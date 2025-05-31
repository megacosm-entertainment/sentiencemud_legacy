/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include "socedit.h"

// Utility function to set a social string field
bool set_social_string_field(CHAR_DATA *ch, SOCIAL_DATA *social, char **field, const char *argument, const char *fieldName) {
    if (argument[0] == '\0' || !str_cmp(argument, "clear")) {
        free_string(*field);
        *field = NULL;
        send_to_char(formatf("%s cleared.\n\r", fieldName), ch);
        return true;
    }

    if (strlen(argument) >= MAX_STRING_LENGTH - 10) { // Check length, -10 for color codes etc.
        send_to_char("String too long.\n\r", ch);
        return false;
    }

    free_string(*field);
    *field = str_dup(argument);
    smash_tilde(*field); // Ensure no tildes are in the string
    send_to_char(formatf("%s set to: %s\n\r", fieldName, *field), ch);
    return true;
}