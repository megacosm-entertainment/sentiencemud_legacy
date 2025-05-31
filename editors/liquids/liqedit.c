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

#include "liqedit.h"


LIQEDIT( liqedit_list )
{
	char buf[MSL];
	BUFFER *buffer = new_buf();

	sprintf(buf, "%-4s %-20s %-20s %s %3s %3s %3s %3s %-7s %s\n\r",
		"#", "Name", "Color", "F", "Prf", "Ful", "Thi", "Hun", "Fuel", "Mana");
	add_buf(buffer, buf);
	sprintf(buf, "%-4s %-20s %-20s %s %3s %3s %3s %3s %-7s %s\n\r",
		"====", "====================", "====================", "=", "===", "===", "===", "===", "=======", "=====");
	add_buf(buffer, buf);

	int i = 0;
    ITERATOR it;
    LIQUID *liq;
    iterator_start(&it, liquid_list);
    while((liq = (LIQUID *)iterator_nextdata(&it)))
    {
		sprintf(buf, "%-4d %-20s %-20s %s %3d %3d %3d %3d %3d %3d %5d\n\r", ++i,
			liq->name, liq->color,
			(liq->flammable ? "{RY{x" : "{rn{x"), liq->proof,
			liq->full, liq->thirst, liq->hunger,
			liq->fuel_unit, liq->fuel_duration,
			liq->max_mana);
		add_buf(buffer, buf);
    }
    iterator_stop(&it);

	sprintf(buf, "%-4s %-20s %-20s %s %3s %3s %3s %3s %-7s %s\n\r",
		"====", "====================", "====================", "=", "===", "===", "===", "===", "=======", "=====");
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

LIQEDIT( liqedit_show )
{
	char buf[MSL];
	BUFFER *buffer;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	buffer = new_buf();

	sprintf(buf, "Liquid:    %s (%d)\n\r", liquid->name, liquid->uid);
	add_buf(buffer, buf);

	sprintf(buf, "Color:     %s\n\r", liquid->color);
	add_buf(buffer, buf);

	sprintf(buf, "Flammable: %s\n\r", (liquid->flammable ? "{WYES{x" : "{DNO{x"));
	add_buf(buffer, buf);

	sprintf(buf, "Proof:     %d\n\r", liquid->proof);
	add_buf(buffer, buf);

	sprintf(buf, "Full:      %d hour%s\n\r", liquid->full, ((liquid->full == 1) ? "" : "s"));
	add_buf(buffer, buf);

	sprintf(buf, "Thirst:    %d hour%s\n\r", liquid->thirst, ((liquid->thirst == 1) ? "" : "s"));
	add_buf(buffer, buf);

	sprintf(buf, "Hunger:    %d hour%s\n\r", liquid->hunger, ((liquid->hunger == 1) ? "" : "s"));
	add_buf(buffer, buf);

	sprintf(buf, "Fuel:      %d unit%s per %d hour%s\n\r",
		liquid->fuel_unit, ((liquid->fuel_unit == 1) ? "" : "s"),
		liquid->fuel_duration, ((liquid->fuel_duration == 1) ? "" : "s"));
	add_buf(buffer, buf);

	sprintf(buf, "Max Mana:  %d\n\r", liquid->max_mana);
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

LIQEDIT( liqedit_create )
{
	LIQUID *liquid;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  liqedit create <name>\n\r", ch);
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	if ((liquid = liquid_lookup(argument)))
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	liquid = new_liquid();
	smash_tilde(argument);
	liquid->name = str_dup(argument);
	liquid->uid = ++top_liquid_uid;
	liquid->gln = NULL;
	insert_liquid(liquid);
	save_liquids();

	olc_set_editor(ch, ED_LIQEDIT, liquid);

	send_to_char("Liquid created.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_delete )
{
	send_to_char("Not yet implemented.\n\r", ch);
	return false;
}

LIQEDIT( liqedit_name )
{
	LIQUID *liquid, *other;

	EDIT_LIQUID(ch, liquid);
	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  liqedit name <name>\n\r", ch);
		send_to_char("Please specify a name.\n\r", ch);
		return false;
	}

	other = liquid_lookup(argument);
	if (IS_VALID(other) && liquid != other)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(liquid->name);
	liquid->name = str_dup(argument);
	send_to_char("Liquid name set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_color )
{
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);
	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  liqedit color <color>\n\r", ch);
		send_to_char("Please specify a color.\n\r", ch);
		return false;
	}

	free_string(liquid->color);
	liquid->color = str_dup(argument);	

	send_to_char("Liquid color set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_flammable )
{
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  liqedit flammable <boolean>\n\r", ch);
		send_to_char("Please specify a boolean value.\n\r", ch);
		return false;
	}

	if (!str_prefix(argument, "yes") || !str_prefix(argument, "true") || !str_prefix(argument, "on"))
		liquid->flammable = true;
	else if (!str_prefix(argument, "no") || !str_prefix(argument, "false") || !str_prefix(argument, "off"))
		liquid->flammable = false;
	{
		send_to_char("Syntax:  liqedit flammable <boolean>\n\r", ch);
		send_to_char("Please specify a boolean value.\n\r", ch);
		return false;
	}

	send_to_char("Liquid flammablity set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_proof )
{
	int value;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (!is_number(argument) || (value = atoi(argument)) < 0 || value > 200)
	{
		send_to_char("Syntax:  liqedit proof <0-200>\n\r", ch);
		send_to_char("Please specify a number from 0 to 200.\n\r", ch);
		return false;
	}

	liquid->proof = value;

	send_to_char("Liquid proof set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_full )
{
	int value;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  liqedit full <hours>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	liquid->full = value;

	send_to_char("Liquid fullness hours set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_thirst )
{
	int value;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  liqedit thirst <hours>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	liquid->thirst = value;

	send_to_char("Liquid thirst hours set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_hunger )
{
	int value;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  liqedit hunger <hours>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	liquid->hunger = value;

	send_to_char("Liquid hunger hours set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_maxmana )
{
	int value;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  liqedit maxmana <number>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	liquid->max_mana = value;

	send_to_char("Liquid Maximum Mana set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_fuel )
{
	char arg[MIL];
	int unit, duration;
	LIQUID *liquid;

	EDIT_LIQUID(ch, liquid);

	argument = one_argument(argument, arg);

	if (!is_number(arg) || (unit = atoi(arg)) < 0)
	{
		send_to_char("Syntax:  liqedit fuel {R<unit>{x <duration>\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	if (!is_number(argument) || (duration = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  liqedit fuel <unit> {R<duration>{x\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	liquid->fuel_unit = unit;
	liquid->fuel_duration = duration;

	send_to_char("Liquid fuel usage set.\n\r", ch);
	return true;
}

LIQEDIT( liqedit_gln )
{
	return false;
}