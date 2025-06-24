#include <string.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "merc.h"

void init_websocket_descriptor(DESCRIPTOR_DATA *d)
{
    if (!d) return;
    
    d->is_websocket = true;
    d->websocket_state = WS_HANDSHAKE;
    d->websocket_buffer_size = 8192;
    d->websocket_buffer = alloc_mem(d->websocket_buffer_size);
    d->websocket_buffer_len = 0;
}

void cleanup_websocket_descriptor(DESCRIPTOR_DATA *d)
{
    if (!d) return;
    
    if (d->websocket_buffer) {
        free_mem(d->websocket_buffer, d->websocket_buffer_size);
        d->websocket_buffer = NULL;
    }
    d->websocket_buffer_len = 0;
    d->websocket_buffer_size = 0;
}

bool websocket_handshake(DESCRIPTOR_DATA *d, char *request)
{
    char *key_start, *key_end;
    char websocket_key[256];
    char accept_key[256];
    char response[1024];
    unsigned char sha_hash[SHA_DIGEST_LENGTH];
    char combined[256];
    
    // Find the WebSocket-Key header
    key_start = strstr(request, "Sec-WebSocket-Key: ");
    if (!key_start) {
        return false;
    }
    
    key_start += strlen("Sec-WebSocket-Key: ");
    key_end = strstr(key_start, "\r\n");
    if (!key_end) {
        return false;
    }
    
    // Extract the key
    int key_len = key_end - key_start;
    strncpy(websocket_key, key_start, key_len);
    websocket_key[key_len] = '\0';
    
    // Create the accept key
    sprintf(combined, "%s%s", websocket_key, WS_MAGIC_STRING);
    
    // SHA1 hash
    SHA1((unsigned char*)combined, strlen(combined), sha_hash);
    
    // Base64 encode
    EVP_ENCODE_CTX *encode_ctx = EVP_ENCODE_CTX_new();
    EVP_EncodeInit(encode_ctx);
    int outlen;
    EVP_EncodeUpdate(encode_ctx, (unsigned char*)accept_key, &outlen, sha_hash, SHA_DIGEST_LENGTH);
    int final_len;
    EVP_EncodeFinal(encode_ctx, (unsigned char*)(accept_key + outlen), &final_len);
    EVP_ENCODE_CTX_free(encode_ctx);
    accept_key[outlen + final_len - 1] = '\0'; // Remove trailing newline
    
    // Send handshake response
    sprintf(response,
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n", accept_key);
    
    if (d->ssl) {
        SSL_write(d->ssl, response, strlen(response));
    } else {
        write(d->descriptor, response, strlen(response));
    }
    
    d->websocket_state = WS_CONNECTED;
    
    // Now send the normal MUD greeting as WebSocket text frame
    char greeting_buf[MAX_STRING_LENGTH];
    if (help_greeting[0] == '.') {
        strcpy(greeting_buf, help_greeting + 1);
    } else {
        strcpy(greeting_buf, help_greeting);
    }
    
    websocket_send_frame(d, greeting_buf, strlen(greeting_buf), WS_OPCODE_TEXT);
    
    return true;
}

bool websocket_send_frame(DESCRIPTOR_DATA *d, const char *data, int len, int opcode)
{
    unsigned char frame[8192];
    int frame_len = 0;
    
    // First byte: FIN + opcode
    frame[frame_len++] = 0x80 | opcode;
    
    // Payload length
    if (len < 126) {
        frame[frame_len++] = len;
    } else if (len < 65536) {
        frame[frame_len++] = 126;
        frame[frame_len++] = (len >> 8) & 0xFF;
        frame[frame_len++] = len & 0xFF;
    } else {
        frame[frame_len++] = 127;
        // For simplicity, we'll just handle up to 16-bit lengths
        for (int i = 0; i < 6; i++) frame[frame_len++] = 0;
        frame[frame_len++] = (len >> 8) & 0xFF;
        frame[frame_len++] = len & 0xFF;
    }
    
    // Copy payload
    memcpy(frame + frame_len, data, len);
    frame_len += len;
    
    // Send frame
    if (d->ssl) {
        return SSL_write(d->ssl, frame, frame_len) == frame_len;
    } else {
        return write(d->descriptor, frame, frame_len) == frame_len;
    }
}

int websocket_parse_frame(DESCRIPTOR_DATA *d, unsigned char *buffer, int buffer_len, char *output)
{
    if (buffer_len < 2) return 0; // Need at least 2 bytes
    
    unsigned char first_byte = buffer[0];
    unsigned char second_byte = buffer[1];
    
    bool fin = (first_byte & 0x80) != 0;
    int opcode = first_byte & 0x0F;
    bool masked = (second_byte & 0x80) != 0;
    int payload_len = second_byte & 0x7F;
    
    int header_len = 2;
    
    // Extended payload length
    if (payload_len == 126) {
        if (buffer_len < 4) return 0;
        payload_len = (buffer[2] << 8) | buffer[3];
        header_len = 4;
    } else if (payload_len == 127) {
        if (buffer_len < 10) return 0;
        // For simplicity, assume payload fits in int
        payload_len = (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | buffer[9];
        header_len = 10;
    }
    
    // Masking key
    unsigned char mask[4];
    if (masked) {
        if (buffer_len < header_len + 4) return 0;
        memcpy(mask, buffer + header_len, 4);
        header_len += 4;
    }
    
    // Check if we have the full frame
    if (buffer_len < header_len + payload_len) return 0;
    
    // Handle different opcodes
    switch (opcode) {
        case WS_OPCODE_TEXT:
            // Unmask payload if needed
            for (int i = 0; i < payload_len; i++) {
                if (masked) {
                    output[i] = buffer[header_len + i] ^ mask[i % 4];
                } else {
                    output[i] = buffer[header_len + i];
                }
            }
            output[payload_len] = '\0';
            return header_len + payload_len;
            
        case WS_OPCODE_PING:
            // Send pong response
            websocket_send_frame(d, (char*)(buffer + header_len), payload_len, WS_OPCODE_PONG);
            return header_len + payload_len;
            
        case WS_OPCODE_CLOSE:
            // Close connection
            return -1;
            
        default:
            // Skip unknown frames
            return header_len + payload_len;
    }
}