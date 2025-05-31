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

#include "matedit.h"



MATEDIT( matedit_list )
{
	char buf[MSL];
	BUFFER *buffer = new_buf();

	sprintf(buf, "%-4s %-20s %-20s\n\r",
		"#", "Name", "Class");
	add_buf(buffer, buf);
	sprintf(buf, "%-4s %-20s %-20s\n\r",
		"====", "====================", "====================");
	add_buf(buffer, buf);

	int i = 0;
    ITERATOR it;
    MATERIAL *mat;
    iterator_start(&it, material_list);
    while((mat = (MATERIAL *)iterator_nextdata(&it)))
    {
		sprintf(buf, "%-4d %-20s %-20s\n\r", ++i,
			mat->name, flag_string(material_classes, mat->material_class));
		add_buf(buffer, buf);
    }
    iterator_stop(&it);

	sprintf(buf, "%-4s %-20s %-20s\n\r",
		"====", "====================", "====================");
	add_buf(buffer, buf);

	sprintf(buf, "Total: %d\n\r", i);
	add_buf(buffer, buf);

	if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH )
	{
		send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
	}
	else
	{
		page_to_char(buffer->string, ch);
	}

	free_buf(buffer);
	return false;
}

MATEDIT( matedit_show )
{
	char buf[MSL];
	BUFFER *buffer;
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	buffer = new_buf();

	sprintf(buf, "Material:    %s\n\r", material->name);
	add_buf(buffer, buf);

	sprintf(buf, "GM: %s\n\r", (material->gm) ? gm_to_name(material->gm) : "{Dnone{x");
	add_buf(buffer, buf);

	sprintf(buf, "Class: %s\n\r", flag_string(material_classes, material->material_class));
	add_buf(buffer, buf);

	sprintf(buf, "Flags: %s\n\r", flag_string(material_flags, material->flags));
	add_buf(buffer, buf);

	sprintf(buf, "Flammable: %d%% (%s)\n\r", material->flammable, (IS_VALID(material->burned) ? material->burned->name : "{Dnone{x"));
	add_buf(buffer, buf);

	sprintf(buf, "Corrodable: %d%% (%s)\n\r", material->corrodibility, (IS_VALID(material->corroded) ? material->corroded->name : "{Dnone{x"));
	add_buf(buffer, buf);

	sprintf(buf, "Strength: %d\n\r", material->strength);
	add_buf(buffer, buf);

	sprintf(buf, "Value: %d\n\r", material->value);
	add_buf(buffer, buf);

	if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH )
	{
		send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
	}
	else
	{
		page_to_char(buffer->string, ch);
	}

	free_buf(buffer);
	return false;
}

MATEDIT( matedit_create )
{
	MATERIAL *material;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  matedit create <name>\n\r", ch);
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	if ((material = material_lookup(argument)))
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	material = new_material();
	smash_tilde(argument);
	material->name = str_dup(argument);
	insert_material(material);
	save_materials();

	olc_set_editor(ch, ED_MATEDIT, material);

	send_to_char("Material created.\n\r", ch);
	return true;
}

MATEDIT( matedit_name )
{
	MATERIAL *material, *other;

	EDIT_MATERIAL(ch, material);
	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  matedit name <name>\n\r", ch);
		send_to_char("Please specify a name.\n\r", ch);
		return false;
	}

	other = material_lookup(argument);
	if (IS_VALID(other) && material != other)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(material->name);
	material->name = str_dup(argument);

	// Reposition material
	list_remlink(material_list, material, false);
	insert_material(material);

	send_to_char("Material name set.\n\r", ch);
	return true;
}

MATEDIT( matedit_gm )
{
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  gm set <global material>\n\r", ch);
		send_to_char("         gm clear\n\r", ch);
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		MATERIAL **gm = gm_from_name(argument);

		if(gm == NULL)
		{
			send_to_char("Invalid global material.\n\r", ch);
			return false;
		}

		if (*gm) (*gm)->gm = NULL;	// Unlink from previous assignment
		*gm = material;
		material->gm = gm;
		send_to_char("Material GM set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (!material->gm)
		{
			send_to_char("Material does not reference a global material.\n\r", ch);
			return false;
		}

		*(material->gm) = NULL;
		material->gm = NULL;
		send_to_char("Material GM cleared.\n\r", ch);
		return true;
	}

	matedit_gm(ch, "");
	return false;

}

MATEDIT( matedit_class )
{
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);
	
	int value;
	if ((value = stat_lookup(argument, material_classes, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid material class.  Use '? matclass' for valid classes.\n\r", ch);
		return false;
	}

	material->material_class = value;

	send_to_char("Material class set.\n\r", ch);
	return true;
}

MATEDIT( matedit_flags )
{
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);
	
	long value;
	if ((value = flag_value(material_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid material flags.  Use '? material' for valid flags.\n\r", ch);
		show_flag_cmds(ch, material_flags);
		return false;
	}

	TOGGLE_BIT(material->flags, value);
	
	send_to_char("Material flags toggled.\n\r", ch);
	return true;
}

MATEDIT( matedit_flammable )
{
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  matedit flammable <0-100>[ <burned material>]\n\r", ch);
		return false;
	}

	int percent;
	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!is_number(arg) || (percent = atoi(arg)) < 0 || percent > 100)
	{
		send_to_char("Please provide a value from 0 to 100.\n\r", ch);
		return false;
	}

	MATERIAL *burned = NULL;
	if (argument[0])
	{
		burned = material_lookup(argument);
		if (!IS_VALID(burned))
		{
			send_to_char("Invalid material.  Use '? materials' for valid list.\n\r", ch);
			return false;
		}
	}


	material->flammable = percent;
	material->burned = burned;
	send_to_char("Material flammablity set.\n\r", ch);
	return true;
}

MATEDIT( matedit_corrodibility )
{
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  matedit corrodibility <0-100>[ <corroded material>]\n\r", ch);
		return false;
	}

	int percent;
	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!is_number(arg) || (percent = atoi(arg)) < 0 || percent > 100)
	{
		send_to_char("Please provide a value from 0 to 100.\n\r", ch);
		return false;
	}

	MATERIAL *corroded = NULL;
	if (argument[0])
	{
		corroded = material_lookup(argument);
		if (!IS_VALID(corroded))
		{
			send_to_char("Invalid material.  Use '? materials' for valid list.\n\r", ch);
			return false;
		}
	}

	material->corrodibility = percent;
	material->corroded = corroded;
	send_to_char("Material corrodibility set.\n\r", ch);
	return true;
}


MATEDIT( matedit_strength )
{
	int value;
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	if (!is_number(argument) || (value = atoi(argument)) < 1)
	{
		send_to_char("Syntax:  matedit strength <strength+>\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	material->strength = value;

	send_to_char("Material strength set.\n\r", ch);
	return true;
}

MATEDIT( matedit_value )
{
	int value;
	MATERIAL *material;

	EDIT_MATERIAL(ch, material);

	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  matedit value <value+>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	material->value = value;

	send_to_char("Material value set.\n\r", ch);
	return true;
}