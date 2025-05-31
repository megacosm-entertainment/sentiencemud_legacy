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

// Define the command table for the social editor
const struct olc_cmd_type socedit_table[] = {
    { "commands",       show_commands       }, // Standard OLC command
    { "show",           socedit_show        },
    { "create",         socedit_create      }, // Note: Create is usually an admin command, not part of typical OLC edit session
    { "name",           socedit_name        },
    { "enabled",        socedit_enabled     },
    { "charnoarg",      socedit_charnoarg   },
    { "othersnoarg",    socedit_othersnoarg },
    { "charfound",      socedit_charfound   },
    { "othersfound",    socedit_othersfound },
    { "victfound",      socedit_victfound   },
    { "charnotfound",   socedit_charnotfound},
    { "charauto",       socedit_charauto    },
    { "othersauto",     socedit_othersauto  },
    { "?",              show_help           }, // Standard OLC command
    { "version",        show_version        }, // Standard OLC command
    { NULL,             0,                  }
};

#define EDIT_SOCIAL(ch, social)    (social = (SOCIAL_DATA *)ch->desc->pEdit)




SOCEDIT(socedit_show) {
    SOCIAL_DATA *social;
    char buf[MAX_STRING_LENGTH * 2]; // Increased buffer size

    EDIT_SOCIAL(ch, social);

    sprintf(buf, "{WName:        {Y%s\n\r", social->name);
    send_to_char(buf, ch);
    sprintf(buf, "{WEnabled:     {Y%s\n\r", social->enabled ? "Yes" : "No");
    send_to_char(buf, ch);

    send_to_char("{c---------------------------------------------------------------------------{x\n\r", ch);
    sprintf(buf, "{WCharNoArg:   {G%s\n\r", social->char_no_arg ? social->char_no_arg : "{D(none){x");
    send_to_char(buf, ch);
    sprintf(buf, "{WOthersNoArg: {G%s\n\r", social->others_no_arg ? social->others_no_arg : "{D(none){x");
    send_to_char(buf, ch);
    send_to_char("{c---------------------------------------------------------------------------{x\n\r", ch);
    sprintf(buf, "{WCharFound:   {G%s\n\r", social->char_found ? social->char_found : "{D(none){x");
    send_to_char(buf, ch);
    sprintf(buf, "{WOthersFound: {G%s\n\r", social->others_found ? social->others_found : "{D(none){x");
    send_to_char(buf, ch);
    sprintf(buf, "{WVictFound:   {G%s\n\r", social->vict_found ? social->vict_found : "{D(none){x");
    send_to_char(buf, ch);
    send_to_char("{c---------------------------------------------------------------------------{x\n\r", ch);
    sprintf(buf, "{WCharNotFound:{G%s\n\r", social->char_not_found ? social->char_not_found : "{D(none){x");
    send_to_char(buf, ch);
    send_to_char("{c---------------------------------------------------------------------------{x\n\r", ch);
    sprintf(buf, "{WCharAuto:    {G%s\n\r", social->char_auto ? social->char_auto : "{D(none){x");
    send_to_char(buf, ch);
    sprintf(buf, "{WOthersAuto:  {G%s\n\r", social->others_auto ? social->others_auto : "{D(none){x");
    send_to_char(buf, ch);
    send_to_char("{c---------------------------------------------------------------------------{x\n\r", ch);

    return false;
}

SOCEDIT(socedit_create) {
    SOCIAL_DATA *social;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        send_to_char("Syntax: socedit create <social_name>\n\r", ch);
        return false;
    }

    if (get_social(arg)) {
        send_to_char("A social with that name already exists.\n\r", ch);
        return false;
    }

    if (!olc_can_edit_name(ch, arg)) {
         send_to_char("Invalid social name.\n\r", ch);
         return false;
    }
    smash_tilde(arg); // Ensure name is clean

    social = new_social_data();
    social->name = str_dup(arg);
    // social->enabled is true by default from new_social_data()

    insert_social_sorted(social); // Add to the global list

    ch->desc->pEdit = (void *)social;
    ch->desc->editor = ED_SOCEDIT; // Make sure ED_SOCEDIT is defined in olc.h
    SET_BIT(ch->pcdata->immortal->olc_flags, OLC_CHANGED);


    send_to_char(formatf("Social '%s' created.\n\r", social->name), ch);
    socedit_show(ch, ""); // Show the new social
    return true; // Mark as changed
}

SOCEDIT(socedit_name) {
    SOCIAL_DATA *social;
    char new_name[MAX_INPUT_LENGTH];

    EDIT_SOCIAL(ch, social);
    one_argument(argument, new_name);

    if (new_name[0] == '\0') {
        send_to_char("Syntax: name <new_name>\n\r", ch);
        return false;
    }

    if (get_social(new_name) && str_cmp(new_name, social->name)) { // Check if different social has this name
        send_to_char("That name is already in use by another social.\n\r", ch);
        return false;
    }

    if (!olc_can_edit_name(ch, new_name)) {
         send_to_char("Invalid social name.\n\r", ch);
         return false;
    }
    smash_tilde(new_name);

    // Remove from list, change name, re-insert to maintain sort order
    list_remlink(social_list, social, false); // Don't free it
    free_string(social->name);
    social->name = str_dup(new_name);
    insert_social_sorted(social);

    send_to_char(formatf("Social name changed to '%s'.\n\r", social->name), ch);
    return true;
}

SOCEDIT(socedit_enabled) {
    SOCIAL_DATA *social;
    EDIT_SOCIAL(ch, social);

    if (!str_cmp(argument, "yes") || !str_cmp(argument, "on") || !str_cmp(argument, "true")) {
        social->enabled = true;
        send_to_char("Social enabled.\n\r", ch);
    } else if (!str_cmp(argument, "no") || !str_cmp(argument, "off") || !str_cmp(argument, "false")) {
        social->enabled = false;
        send_to_char("Social disabled.\n\r", ch);
    } else {
        send_to_char("Syntax: enabled <yes|no>\n\r", ch);
        return false;
    }
    return true;
}

SOCEDIT(socedit_charnoarg) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->char_no_arg, argument, "CharNoArg");
}
SOCEDIT(socedit_othersnoarg) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->others_no_arg, argument, "OthersNoArg");
}
SOCEDIT(socedit_charfound) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->char_found, argument, "CharFound");
}
SOCEDIT(socedit_othersfound) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->others_found, argument, "OthersFound");
}
SOCEDIT(socedit_victfound) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->vict_found, argument, "VictFound");
}
SOCEDIT(socedit_charnotfound) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->char_not_found, argument, "CharNotFound");
}
SOCEDIT(socedit_charauto) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->char_auto, argument, "CharAuto");
}
SOCEDIT(socedit_othersauto) {
    SOCIAL_DATA *social; EDIT_SOCIAL(ch, social);
    return set_social_string_field(ch, social, &social->others_auto, argument, "OthersAuto");
}


