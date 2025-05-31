#ifndef __SOCIALS_EDITOR_H__
#define __SOCIALS_EDITOR_H__
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

#include "../olc_common.h"

// Commands for the social editor
SOCEDIT(socedit_show);
SOCEDIT(socedit_create); // Not standard OLC, but useful for stand-alone creation
SOCEDIT(socedit_name);
SOCEDIT(socedit_enabled);
SOCEDIT(socedit_charnoarg);
SOCEDIT(socedit_othersnoarg);
SOCEDIT(socedit_charfound);
SOCEDIT(socedit_othersfound);
SOCEDIT(socedit_victfound);
SOCEDIT(socedit_charnotfound);
SOCEDIT(socedit_charauto);
SOCEDIT(socedit_othersauto);
// Add socedit_delete if needed

// The command table
extern const struct olc_cmd_type socedit_table[];


bool set_social_string_field(CHAR_DATA *ch, SOCIAL_DATA *social, char **field, const char *argument, const char *fieldName);


#endif /* !def __SOCIALS_EDITOR_H__ */