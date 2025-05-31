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

#include "sectoredit.h"


SECTOREDIT( sectoredit_show )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	BUFFER *buffer = new_buf();

	add_buf(buffer, formatf("Name:          %s\n\r", sector->name));
	add_buf(buffer, formatf("GSCT:          %s\n\r", gsct_to_display(sector->gsct)));
	add_buf(buffer, formatf("Class:         %s\n\r", flag_string(sector_classes, sector->sector_class)));
	add_buf(buffer, formatf("Flags:         %s\n\r", flag_string(sector_flags, sector->flags)));
	add_buf(buffer, formatf("Move Cost:     %d\n\r", sector->move_cost));
	add_buf(buffer, formatf("Health Regen:  %d%%\n\r", sector->hp_regen));
	add_buf(buffer, formatf("Mana Regen:    %d%%\n\r", sector->mana_regen));
	add_buf(buffer, formatf("Move Regen:    %d%%\n\r", sector->move_regen));
	add_buf(buffer, formatf("Soil Chance:   %d%%\n\r", sector->soil));

	add_buf(buffer, "\n\rAffinities:\n\r");
	for(int i = 0; i < SECTOR_MAX_AFFINITIES; i++)
	{
		if (sector->affinities[i][0] > CATALYST_NONE && sector->affinities[i][0] < CATALYST_MAX)
			add_buf(buffer, formatf(" %d)  %-20s %5d\n\r", i+1, flag_string(catalyst_types, sector->affinities[i][0]), sector->affinities[i][1]));
		else
			add_buf(buffer, formatf(" %d)  {Dnone{x\n\r", i+1));
	}

	if (list_size(sector->hide_msgs) > 0)
	{
		add_buf(buffer, "\n\rHide Messages:\n\r");
		ITERATOR mit;
		int m = 0;
		char *message;
		iterator_start(&mit, sector->hide_msgs);
		while((message = (char *)iterator_nextdata(&mit)))
		{
			add_buf(buffer, formatf("%2d) ...%s\n\r", m+1, message));
		}
		iterator_stop(&mit);
	}

	add_buf(buffer, formatf("\n\rDescription:\n\r%s\n\r", string_indent(sector->description, 3)));

	add_buf(buffer, "\n\r-----\n\r{WBuilders' Comments:{X\n\r");
	add_buf(buffer, sector->comments);
	add_buf(buffer, "\n\r-----\n\r");

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

SECTOREDIT( sectoredit_affinity )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  affinity <1-3> <catalyst>[ <value>]\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	int index;
	if (!is_number(arg) || (index = atoi(arg)) < 1 || index > 3)
	{
		send_to_char("Please specify a number from 1 to 3.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	int catalyst;
	if ((catalyst = stat_lookup(arg, catalyst_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid catalyst types.  Use '? catalyst' for valid types.\n\r", ch);
		show_flag_cmds(ch, catalyst_types);
		return false;
	}

	if (catalyst == CATALYST_NONE)
	{
		sector->affinities[index-1][0] = CATALYST_NONE;
		sector->affinities[index-1][1] = 0;
		send_to_char(formatf("Affinity %d cleared.\n\r", index), ch);
	}
	else
	{
		int value;
		if (!is_number(argument) || (value = atoi(argument)) < 1)
		{
			send_to_char("Please provide a positive number.\n\r", ch);
			return false;
		}

		sector->affinities[index-1][0] = catalyst;
		sector->affinities[index-1][1] = value;
		send_to_char(formatf("Affinity %d set.\n\r", index), ch);
	}
	return true;
}

SECTOREDIT( sectoredit_class )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	int16_t clazz;
	if ((clazz = stat_lookup(argument, sector_classes, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid sector class.  Use '? sectorclass' for valid classes.\n\r", ch);
		show_flag_cmds(ch, sector_classes);
		return false;
	}

	sector->sector_class = clazz;
	send_to_char("Sector Class changed.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_comments )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (argument[0] == '\0')
	{
		string_append(ch, &sector->comments);
		return true;
	}

	send_to_char("Syntax:  comments\n\r", ch);
	return false;
}

SECTOREDIT( sectoredit_create )
{
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  create <name>\n\r", ch);
		return false;
	}

	smash_tilde(argument);
	if (get_sector_data(argument) != NULL)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	SECTOR_DATA *sector = new_sector_data();
	sector->name = str_dup(argument);
	insert_sector(sector);

	ch->pcdata->immortal->last_olc_command = current_time;
	olc_set_editor(ch, ED_SECTOREDIT, sector);

	send_to_char("Sector created.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_description )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (argument[0] == '\0')
	{
		string_append(ch, &sector->description);
		return true;
	}

	send_to_char("Syntax:  description\n\r", ch);
	return false;
}

SECTOREDIT( sectoredit_flags )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	long value;
	if ((value = flag_value(sector_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid sector flag.  Use '? sector' for valid list.\n\r", ch);
		show_flag_cmds(ch, sector_flags);
		return false;
	}

	TOGGLE_BIT(sector->flags, value);

	// If turning on 'no_soil', reset the soil setting
	if (IS_SET(sector->flags, SECTOR_NO_SOIL) && IS_SET(value, SECTOR_NO_SOIL))
		sector->soil = 0;
	send_to_char("SECTOR Flags toggled.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_gsct )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  gsct set <global sector name>\n\r", ch);
		send_to_char("         gsct clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		SECTOR_DATA **gsct = gsct_from_name(argument);
		if (!gsct)
		{
			send_to_char("Invalid Globl Sector.  Use '? gsct' for valid list.\n\r", ch);
			show_help(ch, "gsct");
			return false;
		}

		if (*gsct) (*gsct)->gsct = NULL;
		*gsct = sector;
		sector->gsct = gsct;
		send_to_char("Global Sector set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (sector->gsct && sector->gsct == &gsct_inside)
		{
			send_to_char("Unable to unassign {Winside{x without reassigning it in the process.\n\r", ch);
			return false;
		}

		if (sector->gsct)
			*(sector->gsct) = NULL;
		sector->gsct = NULL;
		send_to_char("Global Sector cleared.\n\r", ch);
		return true;
	}

	sectoredit_gsct(ch, "");
	return false;
}

SECTOREDIT( sectoredit_health )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	int regen;
	if (!is_number(argument) || (regen = atoi(argument)) < 0)
	{
		send_to_char("Please provide a non-negative number.\n\r", ch);
		return false;
	}

	sector->hp_regen = regen;
	send_to_char("Sector Health Regen set.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_hidemsgs )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (argument[0] == '\0' )
	{
		send_to_char("Syntax:  hidemsgs add <message>\n\r", ch);
		send_to_char("         hidemsgs remove <#>\n\r", ch);
		send_to_char("         hidemsgs clear\n\r", ch);
		send_to_char("\n\rHide messages are the tail end of the following messages:\n\r", ch);
		send_to_char("{MYou deftly hide {W<OBJECT> {Y<HIDE MESSAGE>{M.{x\n\r", ch);
		send_to_char("{MYou notice {W<HIDER>{M hide {W<OBJECT> {Y<HIDE MESSAGE>{M.{x\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	
	if (!str_prefix(arg, "add"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Please provide a message.\n\r", ch);
			return false;
		}

		smash_tilde(argument);
		list_appendlink(sector->hide_msgs, str_dup(argument));
		send_to_char("Hide Message added.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		int index;
		if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(sector->hide_msgs))
		{
			send_to_char(formatf("Please provide a number from 1 to %d.\n\r", list_size(sector->hide_msgs)), ch);
			return false;
		}

		list_remnthlink(sector->hide_msgs, index, true);
		send_to_char("Hide Message removed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		list_clear(sector->hide_msgs);
		send_to_char("Hide Messages cleared.\n\r", ch);
		return true;
	}

	sectoredit_hidemsgs(ch, "");
	return false;
}

SECTOREDIT( sectoredit_mana )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	int regen;
	if (!is_number(argument) || (regen = atoi(argument)) < 0)
	{
		send_to_char("Please provide a non-negative number.\n\r", ch);
		return false;
	}

	sector->mana_regen = regen;
	send_to_char("Sector Mana Regen set.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_move )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	int regen;
	if (!is_number(argument) || (regen = atoi(argument)) < 0)
	{
		send_to_char("Please provide a non-negative number.\n\r", ch);
		return false;
	}

	sector->move_regen = regen;
	send_to_char("Sector Move Regen set.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_movecost )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	int cost;
	if (!is_number(argument) || (cost = atoi(argument)) < 0)
	{
		send_to_char("Please provide a non-negative number.\n\r", ch);
		return false;
	}

	sector->move_cost = cost;
	send_to_char("Sector Move Cost set.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_name )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name <name>\n\r", ch);
		return false;
	}

	SECTOR_DATA *other = get_sector_data(argument);
	if (other && other != sector)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(sector->name);
	sector->name = str_dup(argument);
	list_remlink(sectors_list, sector, false);
	insert_sector(sector);

	send_to_char("SECTOR Name set.\n\r", ch);
	return true;
}

SECTOREDIT( sectoredit_soil )
{
	SECTOR_DATA *sector;

	EDIT_SECTOR(ch, sector);

	if (IS_SET(sector->flags, SECTOR_NO_SOIL))
	{
		send_to_char("Cannot change the soil chance while sector is {Wno_soil{x.\n\r", ch);
		return false;
	}

	int soil;
	if (!is_number(argument) || (soil = atoi(argument)) < -100 || soil > 100)
	{
		send_to_char("Please provide a number from -100 to 100.\n\r", ch);
		return false;
	}

	sector->soil = soil;
	send_to_char("Sector Soil Chance set.\n\r", ch);
	return true;
}