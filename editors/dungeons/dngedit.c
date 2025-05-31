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

#include "dngedit.h"


DNGEDIT( dngedit_list )
{
	list_dungeons(ch, argument);
	return false;
}

DNGEDIT( dngedit_release )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  release <death release mode>\n\r", ch);
		send_to_char("Please enter '? death_release' to see a list of modes.\n\r",ch);
		return false;
	}

	int value;
	if ((value = flag_lookup(argument, death_release_modes)) == NO_FLAG)
	{
		send_to_char("Syntax:  release <death release mode>\n\r", ch);
		send_to_char("Please enter '? death_release' to see a list of modes.\n\r",ch);
		return false;
	}

	dng->death_release = value;
	send_to_char("Death release changed.\n\r", ch);
	return true;	
}

DNGEDIT( dngedit_groupsize )
{
	DUNGEON_INDEX_DATA *dng;
	char arg[MIL];

	EDIT_DUNGEON(ch, dng);

	argument = one_argument(argument, arg);

	if (!is_number(arg))
	{
		send_to_char("Syntax:  groupsize <min players> <max players>\n\r", ch);
		send_to_char("         groupsize <fixed size>\n\r", ch);
		return false;
	}

	int min = atoi(arg);
	if (min < 1)
	{
		send_to_char("Syntax:  groupsize <min players> <max players>\n\r", ch);
		send_to_char("         groupsize <fixed size>\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	int max = min;
	if (argument[0] != '\0')
	{
		if (!is_number(argument))
		{
			send_to_char("Syntax:  groupsize <min players> <max players>\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}

		max = atoi(argument);
		if (max < min)
		{
			send_to_char("Syntax:  groupsize <min players> <max players>\n\r", ch);
			send_to_char("The maximum player count must not be smaller than the minimum.\n\r", ch);
			return false;
		}
	}

	dng->min_group = min;
	dng->max_group = max;
	send_to_char("Dungeon Group Size set.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_maxplayers )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if (!is_number(argument))
	{
		send_to_char("Syntax:  maxplayers <#players>\n\r", ch);
		return false;
	}

	int max = atoi(argument);
	if (max < 1)
	{
		send_to_char("Syntax:  maxplayers <#players>\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	dng->max_players = max;
	send_to_char("Max players set.\n\r", ch);
	return true;
}


DNGEDIT( dngedit_scripted )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		REMOVE_BIT(dng->flags, DUNGEON_SCRIPTED_LEVELS);
		send_to_char("Scripted Levels Mode disabled.\n\r", ch);
	}
	else
	{
		SET_BIT(dng->flags, DUNGEON_SCRIPTED_LEVELS);
		list_clear(dng->levels);
		list_clear(dng->special_rooms);
		list_clear(dng->special_exits);
		send_to_char("Scripted Levels Mode enabled.\n\r", ch);
	}

	return true;
}

DNGEDIT( dngedit_show )
{
	DUNGEON_INDEX_DATA *dng;
	ROOM_INDEX_DATA *room;
	BUFFER *buffer;
	char buf[MSL];

	EDIT_DUNGEON(ch, dng);

	buffer = new_buf();

	sprintf(buf, "Name:        [%5ld] %s\n\r", dng->vnum, dng->name);
	add_buf(buffer, buf);

	sprintf(buf, "Flags:       %s\n\r", flag_string(dungeon_flags, dng->flags));
	add_buf(buffer, buf);

	sprintf(buf, "AreaWho:     %s\n\r", flag_string(area_who_titles, dng->area_who));
	add_buf(buffer, buf);

	if (dng->max_group > dng->min_group)
		sprintf(buf, "Group Size:  %d to %d member%s (excluding pets and mounts)\n\r", dng->min_group, dng->max_group, (dng->max_group == 1) ? "" : "s");
	else
		sprintf(buf, "Group Size:  %d member%s (excluding pets and mounts)\n\r", dng->min_group, (dng->min_group == 1) ? "" : "s");
	add_buf(buffer, buf);

	sprintf(buf, "Max Players: %d\n\r", dng->max_players);
	add_buf(buffer, buf);

	if( dng->repop > 0)
		sprintf(buf, "Repop:       %d minutes\n\r", dng->repop);
	else
		sprintf(buf, "Repop:       {Dnever{X\n\r");
	add_buf(buffer, buf);

	sprintf(buf,     "Death Release: %s\n\r", flag_string(death_release_modes, dng->death_release));
	add_buf(buffer, buf);

	room = get_room_index(dng->area, dng->entry_room);
	if( room )
	{
		sprintf(buf, "Entry:       [%ld] %-.30s\n\r", room->vnum, room->name);
		add_buf(buffer, buf);
	}
	else
		add_buf(buffer, "Entry:       {Dinvalid{x\n\r");

	room = get_room_index(dng->area, dng->exit_room);
	if( room )
	{
		sprintf(buf, "Exit:        [%ld] %-.30s\n\r", room->vnum, room->name);
		add_buf(buffer, buf);
	}
	else
		add_buf(buffer, "Exit:        {Dinvalid{x\n\r");

	add_buf(buffer, "ZoneOut:     ");
	add_buf(buffer, dng->zone_out);
	add_buf(buffer, "{x\n\r");

	add_buf(buffer, "PortalOut:     ");
	add_buf(buffer, dng->zone_out_portal);
	add_buf(buffer, "{x\n\r");

	add_buf(buffer, "MountOut:     ");
	add_buf(buffer, dng->zone_out_mount);
	add_buf(buffer, "{x\n\r");

	add_buf(buffer, "Description:\n\r");
	add_buf(buffer, dng->description);
	add_buf(buffer, "\n\r");

	dngedit_buffer_floors(buffer, dng);

	dngedit_buffer_levels(buffer, dng);

	add_buf(buffer, "Special Rooms:\n\r");
	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		add_buf(buffer, "   {WSCRIPTED{x\n\r");
	}
	else if( list_size(dng->special_rooms) > 0 )
	{
		DUNGEON_INDEX_SPECIAL_ROOM *special;

		char buf[MSL];
		int line = 0;

		ITERATOR sit;

		add_buf(buffer, "     [             Name             ] [ Level ] [ Room ]\n\r");
		add_buf(buffer, "---------------------------------------------------------\n\r");

		iterator_start(&sit, dng->special_rooms);
		while( (special = (DUNGEON_INDEX_SPECIAL_ROOM *)iterator_nextdata(&sit)) )
		{
			sprintf(buf, "{W%4d  %-30.30s   {%c%7d{x     %4d\n\r", ++line, special->name, (special->level<0?'G':(special->level>0?'Y':'W')), abs(special->level), special->room);
			add_buf(buffer, buf);
		}

		iterator_stop(&sit);
		add_buf(buffer, "---------------------------------------------------------\n\r");
		add_buf(buffer, "{YYELLOW{x - Generated level position index\n\r");
		add_buf(buffer, "{GGREEN{x  - Ordinal level position index\n\r");
	}
	else
	{
		add_buf(buffer, "   None\n\r");
	}
	add_buf(buffer, "\n\r");

	dngedit_buffer_special_exits(buffer, dng);

	add_buf(buffer, "\n\r-----\n\r{WBuilders' Comments:{X\n\r");
	add_buf(buffer, dng->comments);
	add_buf(buffer, "\n\r-----\n\r");



    if (dng->progs)
		olc_show_progs(buffer, dng->progs, PRG_DPROG, "DngProg Vnum");

	olc_show_index_vars(buffer, dng->index_vars);

	if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
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

DNGEDIT( dngedit_create )
{
	AREA_DATA *area = ch->in_room->area;
	DUNGEON_INDEX_DATA *dng;
	WNUM wnum;
	int  iHash;

	if (argument[0] == '\0' || !parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
	{
		long last_vnum = 0;
		long value = area->top_dungeon_vnum + 1;
		for(last_vnum = 1; last_vnum <= area->top_dungeon_vnum; last_vnum++)
		{
			if( !get_dungeon_index(area, last_vnum) )
			{
				value = last_vnum;
				break;
			}
		}

		wnum.pArea = area;
		wnum.vnum = value;
	}

	if( get_dungeon_index(wnum.pArea, wnum.vnum) )
	{
		send_to_char("That dungeon already exists.\n\r", ch);
		return false;
	}

    if (!IS_BUILDER(ch, wnum.pArea))
    {
		send_to_char("BpEdit:  widevnum in an area you cannot build in.\n\r", ch);
		return false;
    }

	dng = new_dungeon_index();
	dng->area = wnum.pArea;
	dng->vnum = wnum.vnum;

	iHash							= dng->vnum % MAX_KEY_HASH;
	dng->next						= wnum.pArea->dungeon_index_hash[iHash];
	wnum.pArea->dungeon_index_hash[iHash]	= dng;
	olc_set_editor(ch, ED_DUNGEON, dng);

	wnum.pArea->bottom_dungeon_vnum = UMIN(wnum.pArea->bottom_dungeon_vnum, dng->vnum);
	wnum.pArea->top_dungeon_vnum = UMAX(wnum.pArea->top_dungeon_vnum, dng->vnum);

    return true;
}

DNGEDIT( dngedit_name )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name [string]\n\r", ch);
		return false;
	}

	free_string(dng->name);
	dng->name = str_dup(argument);
	send_to_char("Name changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_repop )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if( !is_number(argument) )
	{
		send_to_char("Syntax:  repop [age]\n\r", ch);
		return false;
	}

	int repop = atoi(argument);
	dng->repop = UMAX(0, repop);
	send_to_char("Repop changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_description )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		string_append(ch, &dng->description);
		return true;
	}

	send_to_char("Syntax:  description - line edit\n\r", ch);
	return false;
}

DNGEDIT( dngedit_comments )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		string_append(ch, &dng->comments);
		return true;
	}

	send_to_char("Syntax:  comments - line edit\n\r", ch);
	return false;
}

DNGEDIT( dngedit_areawho )
{
	DUNGEON_INDEX_DATA *dng;
	int value;

	EDIT_DUNGEON(ch, dng);

    if (argument[0] != '\0')
    {
		if ( !str_prefix(argument, "blank") )
		{
			dng->area_who = AREA_BLANK;

			send_to_char("Area who title cleared.\n\r", ch);
			return true;
		}

		if ((value = flag_value(area_who_titles, argument)) != NO_FLAG)
		{
			if( value == AREA_INSTANCE || value == AREA_DUTY )
			{
				send_to_char("Area who title only allowed in blueprints.\n\r", ch);
				return false;
			}

			dng->area_who = value;

			send_to_char("Area who title set.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax:  areawho [title]\n\r"
				"Type '? areawho' for a list of who titles.\n\r", ch);
    return false;

}

DNGEDIT( dngedit_floors )
{
	DUNGEON_INDEX_DATA *dng;
	char arg[MIL];

	EDIT_DUNGEON(ch, dng);

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  floors add <widevnum>\n\r", ch);
		send_to_char("         floors remove #\n\r", ch);
		send_to_char("         floors list\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if( !str_prefix(arg, "list") )
	{
		BUFFER *buffer = new_buf();

		dngedit_buffer_floors(buffer, dng);

		page_to_char(buffer->string, ch);
		free_buf(buffer);
		return false;
	}

	if( !str_prefix(arg, "add") )
	{
		WNUM wnum;

		if( !parse_widevnum(argument, ch->in_room->area, &wnum) )
		{
			send_to_char("Please specify a widevnum.\n\r", ch);
			return false;
		}

		BLUEPRINT *bp = get_blueprint(wnum.pArea, wnum.vnum);

		if( !bp )
		{
			send_to_char("That blueprint does not exist.\n\r", ch);
			return false;
		}

		if (list_size(bp->entrances) < 1)
		{
			send_to_char("WARNING: Blueprint is missing default entrance.\n\r", ch);
		}

		if (list_size(bp->exits) < 1)
		{
			send_to_char("WARNING: Blueprint is missing default exit.\n\r", ch);
		}

		/*
		// Disabling this type of check because there will be ways to exit a dungeon without using exits
		// This also wouldn't allow single level dungeons with this type of thing
		if( bp->mode == BLUEPRINT_MODE_STATIC )
		{


			if( bp->static_entry_section < 1 || bp->static_entry_link < 1 ||
				bp->static_exit_section < 1 || bp->static_exit_link < 1 )
			{
				send_to_char("Blueprint must have an entrance and exit specified.\n\r", ch);
				return false;
			}
		}
		else
		{
			send_to_char("Blueprint mode not supported yet.\n\r", ch);
			return false;
		}
		*/

		list_appendlink(dng->floors, bp);
		send_to_char("Floor added.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "remove") || !str_prefix(arg, "delete") )
	{
		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int index = atoi(argument);

		if( index < 1 || index > list_size(dng->floors) )
		{
			send_to_char("Index out of range.\n\r", ch);
			return false;
		}

		list_remnthlink(dng->floors, index, false);

		// TODO: Need to go through everything to make sure the floor is no longer referenced

		// Iterate over Level definitions to remove all references to this floor.

		send_to_char("Floor removed.\n\r", ch);
		return true;
	}

	dngedit_floors(ch, "");
	return false;
}

DNGEDIT( dngedit_levels )
{
	char buf[MSL];
	DUNGEON_INDEX_DATA *dng;
	char arg[MIL];
	char arg2[MIL];
	int floor;
	
	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  levels {Radd{x static <floor>\n\r", ch);
		send_to_char("         levels {Radd{x weighted\n\r", ch);
		send_to_char("         levels {Radd{x grouped\n\r", ch);
		send_to_char("         levels {Rmove{x <#> up|down|top|bottom|first|last\n\r", ch);
		send_to_char("         levels {Rmove{x <from> <to>\n\r", ch);
		send_to_char("         levels {Rweight{x <#> list\n\r", ch);
		send_to_char("         levels {Rweight{x <#> add <weight> <floor>\n\r", ch);
		send_to_char("         levels {Rweight{x <#> set <#> <weight> <floor>\n\r", ch);
		send_to_char("         levels {Rweight{x <#> remove <#>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> add static <floor>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> add weighted\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> move <#> up|down|top|bottom|first|last\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> move <from> <to>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> weight <#> list\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> weight <#> add <weight> <floor>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> weight <#> set <#> <weight> <floor>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> weight <#> remove <#>\n\r", ch);
		send_to_char("         levels {Rgroup{x <#> remove <#>\n\r", ch);
		send_to_char("         levels {Rremove{x <#>\n\r", ch);
		return false;
	}

	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		send_to_char("Please turn off scripted levels to edit levels manually.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	// levels add static <floor>
	// levels add weighted
	// levels add grouped
	if (!str_prefix(arg, "add"))
	{
		argument = one_argument(argument, arg2);

		if (!str_prefix(arg2, "static"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax: levels add static <floor>\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Please specify a number for a floor.", ch);
				return false;
			}

			if (list_size(dng->floors) < 1)
			{
				send_to_char("Please create floors first.\n\r", ch);
				return false;
			}

			floor = atoi(argument);
			if ( floor <= 0 || floor > list_size(dng->floors))
			{
				sprintf(buf, "Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_LEVEL_DATA *level = new_dungeon_index_level();
			level->mode = LEVELMODE_STATIC;
			level->floor = floor;
			list_appendlink(dng->levels, level);
			dungeon_update_level_ordinals(dng);

			sprintf(buf, "Static level %d added.\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg2, "weighted"))
		{
			DUNGEON_INDEX_LEVEL_DATA *level = new_dungeon_index_level();
			level->mode = LEVELMODE_WEIGHTED;
			level->floor = 0;
			list_appendlink(dng->levels, level);
			dungeon_update_level_ordinals(dng);

			sprintf(buf, "Weighted Random level %d added.\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg2, "grouped"))
		{
			DUNGEON_INDEX_LEVEL_DATA *level = new_dungeon_index_level();
			level->mode = LEVELMODE_GROUP;
			level->floor = 0;
			list_appendlink(dng->levels, level);
			dungeon_update_level_ordinals(dng);

			sprintf(buf, "Group level %d added.\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return true;
		}

		send_to_char("Invalid type of level.  Please specify either static, weighted floors or grouped levels.\n\r", ch);
		return false;
	}

	// levels move <#> up|down|top|first|bottom|last
	// levels move <from> <to>
	if (!str_prefix(arg, "move"))
	{
		char arg3[MIL];

		argument = one_argument(argument, arg2);
		if (!is_number(arg2))
		{
			send_to_char("Please specify a valid level number.", ch);
			return false;
		}

		int index = atoi(arg2);
		if (index <= 0 || index > list_size(dng->levels))
		{
			sprintf(buf, "Please specify a level number from 1 to %d.\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return false;
		}

		int to_index = -1;
		argument = one_argument(argument, arg3);
		if (is_number(arg3))
		{
			// levels move <from> <to>
			to_index = atoi(arg3);
			if (to_index < 1 || to_index > list_size(dng->levels))
			{
				sprintf(buf, "Please specify a level number from 1 to %d.\n\r", list_size(dng->levels));
				send_to_char(buf, ch);
				return false;
			}
		}
		else if (!str_prefix(arg3, "up"))
		{
			if (index <= 1)
			{
				send_to_char("That level cannot move up any further.\n\r", ch);
				return false;
			}

			to_index = index - 1;
		}
		else if (!str_prefix(arg3, "down"))
		{
			if (index >= list_size(dng->levels))
			{
				send_to_char("That level cannot move down any further.\n\r", ch);
				return false;
			}

			to_index = index + 1;
		}
		else if (!str_prefix(arg3, "top") || !str_prefix(arg3, "first"))
		{
			if (index <= 1)
			{
				send_to_char("That level is already up as far as it can go.\n\r", ch);
				return false;
			}

			to_index = 1;
		}
		else if (!str_prefix(arg3, "bottom") || !str_prefix(arg3, "last"))
		{
			if (index >= list_size(dng->levels))
			{
				send_to_char("That level is already down ass far as it can go.\n\r", ch);
				return false;
			}

			to_index = list_size(dng->levels);
		}
		else
		{
			send_to_char("Syntax:  levels move <#> up|down|top|bottom|first|last\n\r", ch);
			send_to_char("         levels move <from> <to>\n\r", ch);
			return false;
		}
		
		if (index == to_index)
		{
			send_to_char("You shove the level as hard as possible, barely moving.\n\r", ch);
			return false;
		}

		list_movelink(dng->levels, index, to_index);
		dungeon_update_level_ordinals(dng);
		send_to_char("Level moved.\n\r", ch);
		return true;
	}

	// levels weight <#> list
	// levels weight <#> add <weight> <floor>
	// levels weight <#> set <#> <weight> <floor>
	// levels weight <#> remove <#>
	if (!str_prefix(arg, "weight"))
	{
		char arg3[MIL];
		//char arg4[MIL];
		int index;
		DUNGEON_INDEX_LEVEL_DATA *level;
		//DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;

		argument = one_argument(argument, arg2);
		if (!is_number(arg2))
		{
			send_to_char("Please specify a valid level number.", ch);
			return false;
		}

		index = atoi(arg2);
		if (index <= 0 || index > list_size(dng->levels))
		{
			sprintf(buf, "Please specify a level number from 1 to %d.\n\r", list_size(dng->levels));
			return false;
		}

		level = (DUNGEON_INDEX_LEVEL_DATA *)list_nthdata(dng->levels, index);
		if (!IS_VALID(level))
		{
			send_to_char("Failed to retrieve level information.\n\r", ch);
			return false;
		}

		if (level->mode != LEVELMODE_WEIGHTED)
		{
			send_to_char("That level is not a weighted random level.  Please specify a weighted random level.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg3);

		if (!str_prefix(arg3, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "     [ Weight ] [ Floor ] [   Vnum   ] [             Name             ]\n\r");
			add_buf(buffer, "------------------------------------------------------------------------\n\r");

			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
			ITERATOR wit;
			iterator_start(&wit, level->weighted_floors);

			int row = 0;
			while((weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
			{
				BLUEPRINT *bp = list_nthdata(dng->floors, weighted->floor);

				snprintf(buf, MSL-1, "%4d   %6d     %5d     %8ld    %30.30s\n\r", ++row, weighted->weight, weighted->floor, bp->vnum, bp->name);
				add_buf(buffer, buf);
			}

			iterator_stop(&wit);

			if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
			{
				send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
			}
			else
			{
				page_to_char(buffer->string, ch);
			}
			
			free_buf(buffer);
		}
		else if (!str_prefix(arg3, "add"))
		{
			// levels weight <#> add <weight> <floor>
			char arg4[MIL];

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  levels weight <#> add <weight> <floor>\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);

			if (!is_number(arg4))
			{
				send_to_char("Please specify a positive number for the weight.\n\r", ch);
				return false;
			}

			int weight = atoi(arg4);
			if (weight < 1)
			{
				send_to_char("Please specify a positive number for the weight.\n\r", ch);
				return false;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  levels weight <#> add <weight> <floor>\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				sprintf(buf, "Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
				send_to_char(buf, ch);
				return false;
			}

			int floor = atoi(argument);
			if (floor < 1 || floor > list_size(dng->floors))
			{
				sprintf(buf, "Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = new_weighted_random_floor();
			weighted->weight = weight;
			weighted->floor = floor;

			list_appendlink(level->weighted_floors, weighted);
			level->total_weight += weight;

			send_to_char("Weighted Random entry added.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg3, "set"))
		{
			// levels weight <#> set <#> <weight> <floor>
			char arg4[MIL];
			char arg5[MIL];

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  levels weight <#> set <#> <weight> <floor>\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);
			argument = one_argument(argument, arg5);

			if (!is_number(arg4))
			{
				sprintf(buf, "Please specify a weighted random entry number from 1 to %d.\n\r", list_size(level->weighted_floors));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg4);
			if (index < 1 || index > list_size(level->weighted_floors))
			{
				sprintf(buf, "Please specify a weighted random entry number from 1 to %d.\n\r", list_size(level->weighted_floors));
				send_to_char(buf, ch);
				return false;
			}

			if (!is_number(arg5))
			{
				send_to_char("Please specify a positive number for the weight.\n\r", ch);
				return false;
			}

			int weight = atoi(arg5);
			if (weight < 1)
			{
				send_to_char("Please specify a positive number for the weight.\n\r", ch);
				return false;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  levels weight <#> set <#> <weight> <floor>\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				sprintf(buf, "Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
				send_to_char(buf, ch);
				return false;
			}

			int floor = atoi(argument);
			if (floor < 1 || floor > list_size(dng->floors))
			{
				sprintf(buf, "Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
				send_to_char(buf, ch);
				return false;
			}


			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)list_nthdata(level->weighted_floors, index);

			level->total_weight -= weighted->weight;

			weighted->weight = weight;
			weighted->floor = floor;

			level->total_weight += weight;

			send_to_char("Weighted Random entry set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg3, "remove"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  levels weight <#> remove <#>\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Please specify a number.\n\r", ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(level->weighted_floors))
			{
				sprintf(buf, "Invalid weight entry index.  Please specify a value from 1 to %d.\n\r", list_size(level->weighted_floors));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)list_nthdata(level->weighted_floors, index);
			
			// Sanity check
			if (weighted)
			{
				level->total_weight -= weighted->weight;
			}

			list_remnthlink(level->weighted_floors, index, true);

			send_to_char("Weight Random entry removed.\n\r", ch);
			return false;
		}
	}

	// levels group <#> add static <floor>\n\r", ch);
	// levels group <#> add weighted\n\r", ch);
	// levels group <#> move <#> up|down|top|bottom|first|last\n\r", ch);
	// levels group <#> move <from> <to>\n\r", ch);
	// levels group <#> weight <#> list\n\r", ch);
	// levels group <#> weight <#> add <weight> <floor>\n\r", ch);
	// levels group <#> weight <#> set <#> <weight> <floor>\n\r", ch);
	// levels group <#> weight <#> remove <#>\n\r", ch);
	// levels group <#> remove <#>\n\r", ch);
	if (!str_prefix(arg, "group"))
	{
		if (!is_number(arg2))
		{
			send_to_char("Syntax:  levels group {R<#>{x <command>\n\r", ch);
			send_to_char("         Please specify a number.\n\r", ch);
			return false;
		}

		int index = atoi(arg2);
		if (index < 1 || index > list_size(dng->levels))
		{
			send_to_char("Syntax:  levels group {R<#>{x <command>\n\r", ch);
			sprintf(buf, "         Please specify a group number from 1 to %d.\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return false;
		}

		DUNGEON_INDEX_LEVEL_DATA *level = list_nthdata(dng->levels, index);
		if (!IS_VALID(level))
		{
			send_to_char("No such level exists.\n\r", ch);
			return false;
		}

		if (level->mode != LEVELMODE_GROUP)
		{
			sprintf(buf, "Level %d is not a group level set.\n\r", index);
			send_to_char(buf, ch);
			return false;
		}

		if (IS_NULLSTR(argument))
		{
			send_to_char("Syntax:  levels group <#> {Radd{x static <floor>\n\r", ch);
			send_to_char("         levels group <#> {Radd{x weighted\n\r", ch);
			send_to_char("         levels group <#> {Rmove{x <#> up|down|top|bottom|first|last\n\r", ch);
			send_to_char("         levels group <#> {Rmove{x <from> <to>\n\r", ch);
			send_to_char("         levels group <#> {Rweight{x <#> list\n\r", ch);
			send_to_char("         levels group <#> {Rweight{x <#> add <weight> <floor>\n\r", ch);
			send_to_char("         levels group <#> {Rweight{x <#> set <#> <weight> <floor>\n\r", ch);
			send_to_char("         levels group <#> {Rweight{x <#> remove <#>\n\r", ch);
			send_to_char("         levels group <#> {Rremove{x <#>\n\r", ch);
			return false;
		}

		char arg3[MIL];

		argument = one_argument(argument, arg3);

		// levels group <#> add static <floor>
		// levels group <#> add weighted
		if (!str_prefix(arg3, "add"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  levels group <#> add {Rstatic{x <floor>\n\r", ch);
				send_to_char("         levels group <#> add {Rweighted{x\n\r", ch);
				return false;
			}

			char arg4[MIL];
			argument = one_argument(argument, arg4);

			if (!str_prefix(arg4, "static"))
			{
				if (IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  levels group <#> add static <floor>\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  levels group <#> add static {R<floor>{x\n\r", ch);
					send_to_char("         Please specify a number.\n\r", ch);
					return false;
				}

				int floor = atoi(argument);
				if (floor < 1 || floor > list_size(dng->floors))
				{
					send_to_char("Syntax:  levels group <#> add static {R<floor>{x\n\r", ch);
					sprintf(buf, "         Please specify a floor number between 1 and %d\n\r", list_size(dng->floors));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *lvl = new_dungeon_index_level();
				lvl->mode = LEVELMODE_STATIC;
				lvl->floor = floor;
				list_appendlink(level->group, lvl);
				dungeon_update_level_ordinals(dng);

				sprintf(buf, "Static level added to Group Level %d.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg4, "weighted"))
			{
				DUNGEON_INDEX_LEVEL_DATA *lvl = new_dungeon_index_level();
				lvl->mode = LEVELMODE_WEIGHTED;
				lvl->floor = 0;
				list_appendlink(level->group, lvl);
				dungeon_update_level_ordinals(dng);

				sprintf(buf, "Weighted Random level added to Group Level %d.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

		}

		// levels group <#> move <#> up|down|top|first|bottom|last
		// levels group <#> move <from> <to>
		if (!str_prefix(arg3, "move"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  levels group <#> move <#> up|down|top|first|bottom|last\n\r", ch);
				send_to_char("         levels group <#> move <from> <to>\n\r", ch);
				return false;
			}

			char arg4[MIL];
			//char arg5[MIL];

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Please specify a valid level number.", ch);
				return false;
			}

			int entry = atoi(arg4);
			if (entry <= 0 || entry > list_size(level->group))
			{
				send_to_char("Syntax:  levels group <#> move {R<#>{x up|down|top|first|bottom|last\n\r", ch);
				send_to_char("         levels group <#> move {R<from>{x <to>\n\r", ch);
				sprintf(buf, "         Please specify a level number from 1 to %d.\n\r", list_size(level->group));
				send_to_char(buf, ch);
				return false;
			}

			int to_entry = -1;
			if (is_number(argument))
			{
				// levels move <from> <to>
				to_entry = atoi(argument);
				if (to_entry <= 0 || to_entry > list_size(dng->levels))
				{
					send_to_char("Syntax:  levels group <#> move {R<#>{x up|down|top|first|bottom|last\n\r", ch);
					send_to_char("         levels group <#> move {R<from>{x <to>\n\r", ch);
					sprintf(buf, "         Please specify a level number from 1 to %d.\n\r", list_size(level->group));
					return false;
				}
			}
			else if (!str_prefix(argument, "up"))
			{
				if (entry <= 1)
				{
					send_to_char("That level cannot move up any further.\n\r", ch);
					return false;
				}

				to_entry = entry - 1;
			}
			else if (!str_prefix(argument, "down"))
			{
				if (entry >= list_size(level->group))
				{
					send_to_char("That level cannot move down any further.\n\r", ch);
					return false;
				}

				to_entry = entry + 1;
			}
			else if (!str_prefix(argument, "top") || !str_prefix(argument, "first"))
			{
				if (entry <= 1)
				{
					send_to_char("That level is already up as far as it can go.\n\r", ch);
					return false;
				}

				to_entry = 1;
			}
			else if (!str_prefix(argument, "bottom") || !str_prefix(argument, "last"))
			{
				if (entry >= list_size(level->group))
				{
					send_to_char("That level is already down ass far as it can go.\n\r", ch);
					return false;
				}

				to_entry = list_size(level->group);
			}
			else
			{
				send_to_char("Syntax:  levels group <#> move <#> up|down|top|bottom|first|last\n\r", ch);
				send_to_char("         levels group <#>s move <from> <to>\n\r", ch);
				return false;
			}
			
			if (entry == to_entry)
			{
				send_to_char("You shove the level as hard as possible, barely moving.\n\r", ch);
				return false;
			}

			list_movelink(level->group, entry, to_entry);
			dungeon_update_level_ordinals(dng);

			send_to_char("Level moved.\n\r", ch);
			return true;
		}

		// levels group <#> weight <#> list
		// levels group <#> weight <#> add <weight> <floor>
		// levels group <#> weight <#> set <#> <weight> <floor>
		// levels group <#> weight <#> remove <#>
		if (!str_prefix(arg3, "weight"))
		{
			char arg4[MIL];
			char arg5[MIL];
			//char arg6[MIL];
			DUNGEON_INDEX_LEVEL_DATA *lvl;
			//DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Please specify a valid level number.", ch);
				return false;
			}

			int entry = atoi(arg4);
			if (entry < 1 || entry > list_size(level->group))
			{
				sprintf(buf, "Please specify a level number from 1 to %d.\n\r", list_size(level->group));
				return false;
			}

			lvl = (DUNGEON_INDEX_LEVEL_DATA *)list_nthdata(level->group, entry);
			if (!IS_VALID(lvl))
			{
				send_to_char("Failed to retrieve level information.\n\r", ch);
				return false;
			}

			if (lvl->mode != LEVELMODE_WEIGHTED)
			{
				send_to_char("That level is not a weighted random level.  Please specify a weighted random level.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg5);

			if (!str_prefix(arg5, "list"))
			{
				BUFFER *buffer = new_buf();

				add_buf(buffer, "     [ Weight ] [ Floor ] [   Vnum   ] [             Name             ]\n\r");
				add_buf(buffer, "------------------------------------------------------------------------\n\r");

				DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
				ITERATOR wit;
				iterator_start(&wit, lvl->weighted_floors);

				int row = 0;
				while((weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
				{
					BLUEPRINT *bp = list_nthdata(dng->floors, weighted->floor);

					snprintf(buf, MSL-1, "%4d   %6d     %5d     %8ld    %30.30s\n\r", ++row, weighted->weight, weighted->floor, bp->vnum, bp->name);
					add_buf(buffer, buf);
				}

				iterator_stop(&wit);

				if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
				{
					send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
				}
				else
				{
					page_to_char(buffer->string, ch);
				}
				
				free_buf(buffer);
			}
			else if (!str_prefix(arg5, "add"))
			{
				// levels group <#> weight <#> add <weight> <floor>
				char arg6[MIL];

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  levels group <#> weight <#> add <weight> <floor>\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);

				if (!is_number(arg6))
				{
					send_to_char("Syntax:  levels group <#> weight <#> add {R<weight>{x <floor>\n\r", ch);
					send_to_char("         Please specify a positive number for the weight.\n\r", ch);
					return false;
				}

				int weight = atoi(arg6);
				if (weight < 1)
				{
					send_to_char("Syntax:  levels group <#> weight <#> add {R<weight>{x <floor>\n\r", ch);
					send_to_char("         Please specify a positive number for the weight.\n\r", ch);
					return false;
				}

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  levels group <#> weight <#> add <weight> <floor>\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  levels group <#> weight <#> add <weight> {R<floor>{x\n\r", ch);
					sprintf(buf, "         Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
					send_to_char(buf, ch);
					return false;
				}

				int floor = atoi(argument);
				if (floor < 1 || floor > list_size(dng->floors))
				{
					send_to_char("Syntax:  levels group <#> weight <#> add <weight> {R<floor>{x\n\r", ch);
					sprintf(buf, "         Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = new_weighted_random_floor();
				weighted->weight = weight;
				weighted->floor = floor;

				list_appendlink(lvl->weighted_floors, weighted);
				lvl->total_weight += weight;

				send_to_char("Weighted Random entry added.\n\r", ch);
				return true;
			}
			else if (!str_prefix(arg5, "set"))
			{
				// levels weight <#> set <#> <weight> <floor>
				char arg6[MIL];
				char arg7[MIL];

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> {R<weight> <floor>{x\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				argument = one_argument(argument, arg7);

				if (!is_number(arg6))
				{
					send_to_char("Syntax:  levels group <#> weight <#> set {R<#>{x <weight> <floor>\n\r", ch);
					sprintf(buf, "         Please specify a weighted random entry number from 1 to %d.\n\r", list_size(lvl->weighted_floors));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg6);
				if (index < 1 || index > list_size(lvl->weighted_floors))
				{
					send_to_char("Syntax:  levels group <#> weight <#> set {R<#>{x <weight> <floor>\n\r", ch);
					sprintf(buf, "         Please specify a weighted random entry number from 1 to %d.\n\r", list_size(lvl->weighted_floors));
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(arg7))
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> {R<weight>{x <floor>\n\r", ch);
					send_to_char("         Please specify a positive number for the weight.\n\r", ch);
					return false;
				}

				int weight = atoi(arg7);
				if (weight < 1)
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> {R<weight>{x <floor>\n\r", ch);
					send_to_char("         Please specify a positive number for the weight.\n\r", ch);
					return false;
				}

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> <weight> {R<floor>{x\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> <weight> {R<floor>{x\n\r", ch);
					sprintf(buf, "         Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
					send_to_char(buf, ch);
					return false;
				}

				int floor = atoi(argument);
				if (floor < 1 || floor > list_size(dng->floors))
				{
					send_to_char("Syntax:  levels group <#> weight <#> set <#> <weight> {R<floor>{x\n\r", ch);
					sprintf(buf, "         Please specify a floor number from 1 to %d.\n\r", list_size(dng->floors));
					send_to_char(buf, ch);
					return false;
				}


				DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)list_nthdata(level->weighted_floors, index);

				lvl->total_weight -= weighted->weight;

				weighted->weight = weight;
				weighted->floor = floor;

				lvl->total_weight += weight;

				send_to_char("Weighted Random entry set.\n\r", ch);
				return true;
			}
			else if (!str_prefix(arg5, "remove"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  levels group <#> weight <#> remove {R<#>{x\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  levels group <#> weight <#> remove {R<#>{x\n\r", ch);
					send_to_char("         Please specify a number.\n\r", ch);
					return false;
				}

				int index = atoi(argument);
				if (index < 1 || index > list_size(lvl->weighted_floors))
				{
					sprintf(buf, "Invalid weight entry index.  Please specify a value from 1 to %d.\n\r", list_size(lvl->weighted_floors));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)list_nthdata(lvl->weighted_floors, index);
				
				// Sanity check
				if (weighted)
				{
					lvl->total_weight -= weighted->weight;
				}

				list_remnthlink(lvl->weighted_floors, index, true);

				send_to_char("Weight Random entry removed.\n\r", ch);
				return false;
			}
		}

		// levels group <#> remove <#>
		if (!str_prefix(arg3, "remove"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  levels group <#> remove {R<#>{x\n\r", ch);
				send_to_char("         Please give a number.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  levels group <#> remove {R<#>{x\n\r", ch);
				send_to_char("         Please give a number.\n\r", ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(level->group))
			{
				send_to_char("Syntax:  levels group <#> remove {R<#>{x\n\r", ch);
				sprintf(buf, "         Please give a number between 1 and %d\n\r", list_size(level->group));
				send_to_char(buf, ch);
				return false;
			}
		
			list_remnthlink(level->group, index, true);
			dungeon_update_level_ordinals(dng);

			send_to_char("Level removed.\n\r", ch);
			return true;
		}

		dngedit_levels(ch, "group");
	}

	// levels remove <#>
	if (!str_prefix(arg, "remove"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("Syntax:  levels remove {R<#>{x\n\r", ch);
			send_to_char("         Please give a number.\n\r", ch);
			return false;
		}

		if (!is_number(argument))
		{
			send_to_char("Syntax:  levels remove {R<#>{x\n\r", ch);
			send_to_char("         Please give a number.\n\r", ch);
			return false;
		}

		int index = atoi(argument);
		if (index < 1 || index > list_size(dng->levels))
		{
			send_to_char("Syntax:  levels remove {R<#>{x\n\r", ch);
			sprintf(buf, "         Please give a number between 1 and %d\n\r", list_size(dng->levels));
			send_to_char(buf, ch);
			return false;
		}

		DUNGEON_INDEX_LEVEL_DATA *level = list_nthdata(dng->levels, index);
		if (!IS_VALID(level))
		{
			send_to_char("Failed to retrieve level information.\n\r", ch);
			return false;
		}
	
		list_remnthlink(dng->levels, index, true);
		dungeon_update_level_ordinals(dng);

		send_to_char("Level removed.\n\r", ch);
		return true;
	}

	dngedit_levels(ch, "");
	return false;
}

DNGEDIT( dngedit_entry )
{
	DUNGEON_INDEX_DATA *dng;
	long value;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  entry <local vnum>\n\r", ch);
		return false;
	}

	if( !is_number(argument) )
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	value = atol(argument);

	if( !get_room_index(dng->area, value) )
	{
		send_to_char("That room does not exist.\n\r", ch);
		return false;
	}

	dng->entry_room = value;
	send_to_char("Entry room changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_exit )
{
	DUNGEON_INDEX_DATA *dng;
	long value;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  exit <local vnum>\n\r", ch);
		return false;
	}

	if( !is_number(argument) )
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	value = atol(argument);

	if( !get_room_index(dng->area, value) )
	{
		send_to_char("That room does not exist.\n\r", ch);
		return false;
	}

	dng->exit_room = value;
	send_to_char("Exit room changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_flags )
{
	DUNGEON_INDEX_DATA *dng;
	int value;

	EDIT_DUNGEON(ch, dng);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  flags <flags>\n\r", ch);
		send_to_char("'? dungeon' for list of flags.\n\r", ch);
		return false;
	}

	if( (value = flag_value(dungeon_flags, argument)) != NO_FLAG )
	{
		if (IS_SET(value, DUNGEON_SHARED) && list_size(dng->loaded) > 0)
		{
			send_to_char("Cannot toggle {YSHARED{x at this time.  The dungeon is currently loaded.\n\r", ch);
			return false;
		}

		dng->flags ^= value;
		send_to_char("Dungeon flags changed.\n\r", ch);
		return true;
	}

	dngedit_flags(ch, "");
	return false;

}

DNGEDIT( dngedit_zoneout )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  zoneout [string]\n\r", ch);
		return false;
	}

	free_string(dng->zone_out);
	dng->zone_out = str_dup(argument);
	send_to_char("ZoneOut changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_portalout )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  portalout [string]\n\r", ch);
		return false;
	}

	free_string(dng->zone_out_portal);
	dng->zone_out_portal = str_dup(argument);
	send_to_char("PortalOut changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_mountout )
{
	DUNGEON_INDEX_DATA *dng;

	EDIT_DUNGEON(ch, dng);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  mountout [string]\n\r", ch);
		return false;
	}

	free_string(dng->zone_out_mount);
	dng->zone_out_mount = str_dup(argument);
	send_to_char("MountOut changed.\n\r", ch);
	return true;
}

DNGEDIT( dngedit_special )
{
	DUNGEON_INDEX_DATA *dng;
	char arg[MIL];
	char arg2[MIL];
	char arg3[MIL];
	char arg4[MIL];
	char arg5[MIL];
	char arg6[MIL];
	char arg7[MIL];

	EDIT_DUNGEON(ch, dng);

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char("Syntax:  special room list\n\r", ch);
		send_to_char("         special room add generated|ordinal <level> <special room> <name>\n\r", ch);
		send_to_char("         special room # remove\n\r", ch);
		send_to_char("         special room # name <name>\n\r", ch);
		send_to_char("         special room # level generated|ordinal <level>\n\r", ch);
		send_to_char("         special room # room <special room>\n\r", ch);
		send_to_char("         special exit list\n\r", ch);
		send_to_char("         special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit add source generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit add destination generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit add weighted\n\r", ch);
		send_to_char("         special exit add group\n\r", ch);
		send_to_char("         special exit from # list\n\r", ch);
		send_to_char("         special exit from # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit from # set # <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit from # remove #\n\r", ch);
		send_to_char("         special exit to # list\n\r", ch);
		send_to_char("         special exit to # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit to # set # <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit to # remove #\n\r", ch);
		send_to_char("         special exit list\n\r", ch);
		send_to_char("         special exit group # add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit group # add source generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit group # add destination generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit group # add weighted\n\r", ch);
		send_to_char("         special exit group # add group\n\r", ch);
		send_to_char("         special exit group # from # list\n\r", ch);
		send_to_char("         special exit group # from # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit group # from # set # <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit group # from # remove #\n\r", ch);
		send_to_char("         special exit group # to # list\n\r", ch);
		send_to_char("         special exit group # to # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit group # to # set # <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit group # to # remove #\n\r", ch);
		send_to_char("         special exit group # remove #\n\r", ch);
		send_to_char("         special exit remove #\n\r", ch);
		return false;
	}

	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		send_to_char("Please turn off scripted levels to edit special rooms/exits manually.\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "room"))
	{
		argument = one_argument(argument, arg2);

		if( !str_prefix(arg2, "list") )
		{
			if( list_size(dng->special_rooms) > 0 )
			{
				BUFFER *buffer = new_buf();
				DUNGEON_INDEX_SPECIAL_ROOM *special;

				char buf[MSL];
				int line = 0;

				ITERATOR sit;

				add_buf(buffer, "     [             Name             ] [ Level ] [ Room ]\n\r");
				add_buf(buffer, "---------------------------------------------------------\n\r");

				iterator_start(&sit, dng->special_rooms);
				while( (special = (DUNGEON_INDEX_SPECIAL_ROOM *)iterator_nextdata(&sit)) )
				{
					snprintf(buf, MSL - 1, "%4d %30.30s{x    {%c%5d{x     %4d\n\r", ++line, special->name, (special->level<0?'G':(special->level>0?'Y':'W')), special->level, special->room);
					buf[MSL-1] = '\0';
					add_buf(buffer, buf);
				}

				iterator_stop(&sit);
				add_buf(buffer, "---------------------------------------------------------d\n\r");
				add_buf(buffer, "{YYELLOW{x - Generated level position index\n\r");
				add_buf(buffer, "{GGREEN{x  - Ordinal level position index\n\r");

				if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH )
				{
					send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
				}
				else
				{
					page_to_char(buffer->string, ch);
				}

				free_buf(buffer);
			}
			else
			{
				send_to_char("Dungeon has no special rooms defined.\n\r", ch);
			}

			return false;
		}

		if( is_number(arg2) )
		{
			int index = atoi(arg2);

			DUNGEON_INDEX_SPECIAL_ROOM *special = list_nthdata(dng->special_rooms, index);

			if( !IS_VALID(special) )
			{
				send_to_char("No such special room.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if( arg3[0] == '\0' )
			{
				dngedit_special(ch, "");
				return false;
			}

			if( !str_prefix(arg3, "remove") || !str_prefix(arg3, "delete") )
			{
				list_remnthlink(dng->special_rooms, index, true);

				send_to_char("Special room deleted.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg3, "level") )
			{
				sent_bool mode = TRISTATE_UNDEF;

				int levels = dungeon_index_generation_count(dng);

				argument = one_argument(argument, arg4);
				if (!str_prefix(arg4, "generated"))
					mode = false;
				else if (!str_prefix(arg4, "ordinal"))
					mode = true;
				else
				{
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				if( !is_number(argument) )
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int level = atoi(argument);
				if( level < 1 || level > levels )
				{
					send_to_char("Level out of range.\n\r", ch);
					return false;
				}

				special->level = mode ? -level : level;
				special->room = -1;

				send_to_char("Level changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg3, "name") )
			{
				argument = one_argument(argument, arg4);

				if( IS_NULLSTR(arg4) )
				{
					send_to_char("Syntax:  special room # name [name]\n\r", ch);
					return false;
				}


				smash_tilde(arg4);
				free_string(special->name);
				special->name = str_dup(arg4);

				send_to_char("Special room name changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg3, "room") )
			{
				argument = one_argument(argument, arg4);
				if( !is_number(arg4))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int room = atol(arg4);

				special->room = room;

				send_to_char("Special room changed.\n\r", ch);
				return true;
			}
		}
		else if( !str_prefix(arg2, "add") )
		{
			sent_bool mode = TRISTATE_UNDEF;
			char argm[MIL];
			argument = one_argument(argument, argm);
			argument = one_argument(argument, arg3);
			argument = one_argument(argument, arg4);

			if( argument[0] == '\0' )
			{
				send_to_char("Syntax:  special room add generated|ordinal [level] [special room] [name]\n\r", ch);
				return false;
			}

			if (!str_prefix(argm, "generated"))
				mode = false;
			else if (!str_prefix(argm, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			if( !is_number(arg3) || !is_number(arg4) )
			{
				send_to_char("That is not a number.\n\r", ch);
				return false;
			}

			int level = atoi(arg3);
			int room = atoi(arg4);
			int levels = dungeon_index_generation_count(dng);

			if( level < 1 || level > levels )
			{
				send_to_char("Level out of range.\n\r", ch);
				return false;
			}

			char name[MIL+1];
			strncpy(name, argument, MIL);
			name[MIL] = '\0';
			smash_tilde(name);

			DUNGEON_INDEX_SPECIAL_ROOM *special = new_dungeon_index_special_room();

			free_string(special->name);
			special->name = str_dup(name);
			special->level = mode ? -level : level;
			special->room = room;

			list_appendlink(dng->special_rooms, special);

			send_to_char("Special Room added.\n\r", ch);
			return true;
		}


		send_to_char("Syntax:  special {Wroom{x list\n\r", ch);
		send_to_char("         special {Wroom{x add generated|ordinal <level> <special room> <name>\n\r", ch);
		send_to_char("         special {Wroom{x # remove\n\r", ch);
		send_to_char("         special {Wroom{x # name <name>\n\r", ch);
		send_to_char("         special {Wroom{x # level <level>\n\r", ch);
		send_to_char("         special {Wroom{x # room <special room>\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "exit"))
	{
		argument = one_argument(argument, arg2);

		if (!str_prefix(arg2, "list"))
		{
			if (list_size(dng->special_exits) > 0)
			{
				char buf[MSL];
				ITERATOR it;
				DUNGEON_INDEX_SPECIAL_EXIT *dsex;
				BUFFER *buffer = new_buf();

				add_buf(buffer, "{x     [    Mode    ]{x\n\r");
				add_buf(buffer, "{x========================================================{x\n\r");

				int exitno = 1;
				iterator_start(&it, dng->special_exits);
				while ( (dsex = (DUNGEON_INDEX_SPECIAL_EXIT *)iterator_nextdata(&it)))
				{
					switch(dsex->mode)
					{
						case EXITMODE_STATIC:
						{
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->from, 1);
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->to, 1);
							sprintf(buf, "%4d  {Y   STATIC  {x\n\r", exitno++);
							add_buf(buffer, buf);
							sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
							add_buf(buffer, buf);
							sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
							add_buf(buffer, buf);
							break;	
						}

						case EXITMODE_WEIGHTED_SOURCE:
						{
							ITERATOR wit;
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->to, 1);
							sprintf(buf, "%4d  {C   SOURCE   {x\n\r", exitno++);
							add_buf(buffer, buf);

							int fromexitno = 1;
							add_buf(buffer, "          Source:\n\r");
							add_buf(buffer, "               [ Weight ] [ Level ] [ Exit# ]{x\n\r");
							add_buf(buffer, "          ===================================={x\n\r");
							iterator_start(&wit, dsex->from);
							while ( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
							{
								sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromexitno++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
								add_buf(buffer, buf);
							}
							iterator_stop(&wit);
							add_buf(buffer, "          ----------------------------------------------------{x\n\r");

							sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
							add_buf(buffer, buf);
							break;
						}

						case EXITMODE_WEIGHTED_DEST:
						{
							ITERATOR wit;
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->from, 1);
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
							sprintf(buf, "%4d  {C DESTINATION{x\n\r", exitno++);
							add_buf(buffer, buf);

							sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
							add_buf(buffer, buf);

							int toexitno = 1;
							add_buf(buffer, "          Destination:\n\r");
							add_buf(buffer, "               [ Weight ] [ Level ] [ Exit# ]{x\n\r");
							add_buf(buffer, "          ===================================={x\n\r");
							iterator_start(&wit, dsex->to);
							while ( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
							{
								sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", toexitno++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
								add_buf(buffer, buf);
							}
							iterator_stop(&wit);
							add_buf(buffer, "          ----------------------------------------------------{x\n\r");
							break;
						}

						case EXITMODE_WEIGHTED:
						{
							ITERATOR wit;
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
							DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
							sprintf(buf, "%4d  {C  WEIGHTED  {x\n\r", exitno++);
							add_buf(buffer, buf);

							int fromexitno = 1;
							add_buf(buffer, "          Source:\n\r");
							add_buf(buffer, "               [ Weight ] [ Level ] [ Exit# ]{x\n\r");
							add_buf(buffer, "          ===================================={x\n\r");
							iterator_start(&wit, dsex->from);
							while ( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
							{
								sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromexitno++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
								add_buf(buffer, buf);
							}
							iterator_stop(&wit);
							add_buf(buffer, "          ----------------------------------------------------{x\n\r");

							int toexitno = 1;
							add_buf(buffer, "          Destination:\n\r");
							add_buf(buffer, "               [ Weight ] [ Level ] [ Exit# ]{x\n\r");
							add_buf(buffer, "          ===================================={x\n\r");
							iterator_start(&wit, dsex->to);
							while ( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
							{
								sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", toexitno++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
								add_buf(buffer, buf);
							}
							iterator_stop(&wit);
							add_buf(buffer, "          ----------------------------------------------------{x\n\r");
							break;
						}

						case EXITMODE_GROUP:
						{
							int count = list_size(dsex->group);
							sprintf(buf, "%4d  {G    GROUP   {x\n\r", exitno++);
							add_buf(buffer, buf);
							if (count > 0)
							{
								ITERATOR git;
								DUNGEON_INDEX_SPECIAL_EXIT *gex;

								add_buf(buffer, "{x               [    Mode    ]{x\n\r");
								add_buf(buffer, "{x          ========================================================{x\n\r");

								int gexitno = 1;
								iterator_start(&git, dsex->group);
								while ( (gex = (DUNGEON_INDEX_SPECIAL_EXIT *)iterator_nextdata(&git)) )
								{
									switch(gex->mode)
									{
										case EXITMODE_STATIC:
										{
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(gex->from, 1);
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(gex->to, 1);
											sprintf(buf, "          %4d  {Y   STATIC   {x\n\r", gexitno++);
											sprintf(buf, "                    Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
											add_buf(buffer, buf);
											sprintf(buf, "                    Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
											add_buf(buffer, buf);
											break;
										}

										case EXITMODE_WEIGHTED_SOURCE:
										{
											ITERATOR wit;
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(gex->to, 1);
											sprintf(buf, "          %4d  {C   SOURCE   {x\n\r", gexitno++);
											add_buf(buffer, buf);

											int fromexitno = 1;
											add_buf(buffer, "                    Source:\n\r");
											add_buf(buffer, "                         [ Weight ] [ Level ] [ Exit# ]{x\n\r");
											add_buf(buffer, "                    ===================================={x\n\r");
											iterator_start(&wit, gex->from);
											while ( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
											{
												sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", fromexitno++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
												add_buf(buffer, buf);
											}
											iterator_stop(&wit);
											add_buf(buffer, "                    ----------------------------------------------------{x\n\r");

											sprintf(buf,    "                    Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
											add_buf(buffer, buf);
											break;
										}

										case EXITMODE_WEIGHTED_DEST:
										{
											ITERATOR wit;
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(gex->from, 1);
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
											sprintf(buf, "          %4d  {C DESTINATION{x\n\r", gexitno++);
											add_buf(buffer, buf);

											sprintf(buf,    "                    Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
											add_buf(buffer, buf);

											int toexitno = 1;
											add_buf(buffer, "                    Destination:\n\r");
											add_buf(buffer, "                         [ Weight ] [ Level ] [ Exit# ]{x\n\r");
											add_buf(buffer, "                    ===================================={x\n\r");
											iterator_start(&wit, gex->to);
											while ( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
											{
												sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", toexitno++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
												add_buf(buffer, buf);
											}
											iterator_stop(&wit);
											add_buf(buffer, "                    ----------------------------------------------------{x\n\r");
											break;
										}

										case EXITMODE_WEIGHTED:
										{
											ITERATOR wit;
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
											DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
											sprintf(buf, "          %4d  {C  WEIGHTED  {x\n\r", gexitno++);
											add_buf(buffer, buf);

											int fromexitno = 1;
											add_buf(buffer, "                    Source:\n\r");
											add_buf(buffer, "                         [ Weight ] [ Level ] [ Exit# ]{x\n\r");
											add_buf(buffer, "                    ===================================={x\n\r");
											iterator_start(&wit, gex->from);
											while ( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
											{
												sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", fromexitno++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
												add_buf(buffer, buf);
											}
											iterator_stop(&wit);
											add_buf(buffer, "                    ----------------------------------------------------{x\n\r");

											int toexitno = 1;
											add_buf(buffer, "                    Destination:\n\r");
											add_buf(buffer, "                         [ Weight ] [ Level ] [ Exit# ]{x\n\r");
											add_buf(buffer, "                    ===================================={x\n\r");
											iterator_start(&wit, gex->to);
											while ( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&wit)) )
											{
												sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", toexitno++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
												add_buf(buffer, buf);
											}
											iterator_stop(&wit);
											add_buf(buffer, "                    ----------------------------------------------------{x\n\r");
											break;
										}
									}

								}
								iterator_stop(&git);
								add_buf(buffer, "{x          --------------------------------------------------------{x\n\r");
							}
							else
							{
								add_buf(buffer, "          None\n\r");
							}
							break;
						}
					}

				}

				iterator_stop(&it);
				add_buf(buffer, "{x----------------------------------------------------{x\n\r");
				add_buf(buffer, "{YYELLOW{x - Generated level position index\n\r");
				add_buf(buffer, "{GGREEN{x  - Ordinal level position index\n\r");

				if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
				{
					send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
				}
				else
				{
					page_to_char(buffer->string, ch);
				}

				free_buf(buffer);
			}
			else
				send_to_char("Dungeon has no special exits defined.\n\r", ch);
			return false;
		}

		if (!str_prefix(arg2, "add"))
		{
			int levels = dungeon_index_generation_count(dng);
			char buf[MSL];

			if (list_size(dng->levels) < 1)
			{
				send_to_char("Please add level definitions before adding special exits.\n\r", ch);
				return false;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  special exit {Wadd{x static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit {Wadd{x source generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit {Wadd{x destination generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit {Wadd{x weighted\n\r", ch);
				send_to_char("         special exit {Wadd{x group\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);

			// Add Static Special Exit
			// special exit add static <from-level> <from-exit> <to-level> <to-entrance>
			if (!str_prefix(arg3, "static"))
			{
				sent_bool from_mode = TRISTATE_UNDEF;
				sent_bool to_mode = TRISTATE_UNDEF;
				char argmf[MIL];
				char argmt[MIL];

				argument = one_argument(argument, argmf);
				if (!str_prefix(argmf, "generated"))
					from_mode = false;
				else if(!str_prefix(argmf, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  special exit add static {Rgenerated|ordinal{x <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				if (IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal {R<from-level>{x <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", list_size(dng->levels));
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg4);

				if (!is_number(arg4))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal {R<from-level>{x <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				int from_level = atoi(arg4);
				if (from_level < 1 || from_level > levels)
				{
					send_to_char("Syntax:  special exit add static generated|ordinal {R<from-level>{x <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *group;
				DUNGEON_INDEX_LEVEL_DATA *from_level_data = dungeon_index_get_nth_level(dng, (from_mode?-from_level:from_level),&group);

				int from_exits = get_dungeon_index_level_special_exits(dng, group?group:from_level_data);

				if (IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> {R<from-exit>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg5);

				if (!is_number(arg5))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> {R<from-exit>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
					send_to_char(buf, ch);
					return false;
				}

				int from_exit = atoi(arg5);
				if (from_exit < 1 || from_exit > from_exits)
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> {R<from-exit>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, argmt);
				if (!str_prefix(argmt, "generated"))
					to_mode = false;
				else if (!str_prefix(argmt, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				if (IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg6);

				if (!is_number(arg6))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				int to_level = atoi(arg6);
				if (to_level < 1 || to_level > levels)
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *to_level_data = dungeon_index_get_nth_level(dng, (to_mode?-to_level:to_level), &group);

				int to_entries = get_dungeon_index_level_special_entrances(dng, group?group:to_level_data);

				if (IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
					send_to_char(buf, ch);
					return false;
				}

				int to_entry = atoi(argument);
				if (to_entry < 1 || to_entry > to_entries)
				{
					send_to_char("Syntax:  special exit add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
					send_to_char(buf, ch);
					return false;
				}

				if (from_mode == to_mode && from_level == to_level && from_exit == to_entry)
				{
					send_to_char("Both exits are the same.  Unable to connect them.\n\r", ch);
					return false;
				}

				DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
				ex->mode = EXITMODE_STATIC;
				
				add_dungeon_index_weighted_exit_data(ex->from, 1, from_mode?-from_level:from_level, from_exit);
				add_dungeon_index_weighted_exit_data(ex->to, 1, to_mode?-to_level:to_level, to_entry);

				list_appendlink(dng->special_exits, ex);

				send_to_char("Static special exit added.\n\r", ch);
				return true;
			}

			// Add Source Special Exit
			// special exit add source <to-level> <to-entrance>
			if (!str_prefix(arg3, "source"))
			{
				sent_bool to_mode = TRISTATE_UNDEF;
				char argmt[MIL];

				argument = one_argument(argument, argmt);
				if (!str_prefix(argmt, "generated"))
					to_mode = false;
				else if (!str_prefix(argmt, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  special exit add source {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;

				}

				argument = one_argument(argument, arg6);

				if (!is_number(arg6))
				{
					send_to_char("Syntax:  special exit add source generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				int to_level = atoi(arg6);
				if (to_level < 1 || to_level > levels)
				{
					send_to_char("Syntax:  special exit add source generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *group;
				DUNGEON_INDEX_LEVEL_DATA *to_level_data = dungeon_index_get_nth_level(dng, (to_mode?-to_level:to_level), &group);

				int to_entries = get_dungeon_index_level_special_entrances(dng, group?group:to_level_data);

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit add source generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
					send_to_char(buf, ch);
					return false;
				}

				int to_entry = atoi(argument);
				if (to_entry < 1 || to_entry > to_entries)
				{
					send_to_char("Syntax:  special exit add source generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
				ex->mode = EXITMODE_WEIGHTED_SOURCE;
				
				add_dungeon_index_weighted_exit_data(ex->to, 1, (to_mode?-to_level:to_level), to_entry);

				list_appendlink(dng->special_exits, ex);

				int index = list_size(dng->special_exits);

				send_to_char("Source special exit added.\n\r", ch);
				sprintf(buf, "Please add source exits using {Wspecial exit from {Y%d{W add ...{x\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "destination"))
			{
				sent_bool from_mode = TRISTATE_UNDEF;
				char argmf[MIL];

				argument = one_argument(argument, argmf);
				if (!str_prefix(argmf, "generated"))
					from_mode = false;
				else if (!str_prefix(argmf, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  special exit add destination {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);

				if (!is_number(arg4))
				{
					send_to_char("Syntax:  special exit add destination generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
					sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", list_size(dng->levels));
					send_to_char(buf, ch);
					return false;
				}

				int from_level = atoi(arg4);
				if (from_level < 1 || from_level > list_size(dng->levels))
				{
					send_to_char("Syntax:  special exit add destination generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
					sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", list_size(dng->levels));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *group;
				DUNGEON_INDEX_LEVEL_DATA *from_level_data = dungeon_index_get_nth_level(dng, (from_mode?-from_level:from_level), &group);

				int from_exits = get_dungeon_index_level_special_exits(dng, group?group:from_level_data);

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit add destination generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
					sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
					send_to_char(buf, ch);
					return false;
				}

				int from_exit = atoi(argument);
				if (from_exit < 1 || from_exit > from_exits)
				{
					send_to_char("Syntax:  special exit add destination generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
					sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
				ex->mode = EXITMODE_WEIGHTED_DEST;
				
				add_dungeon_index_weighted_exit_data(ex->from, 1, (from_mode?-from_level:from_level), from_exit);

				list_appendlink(dng->special_exits, ex);

				int index = list_size(dng->special_exits);

				send_to_char("Destination special exit added.\n\r", ch);
				sprintf(buf, "Please add target entrances using {Wspecial exit to {Y%d{W add ...{x\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "weighted"))
			{
				if (!IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add weighted\n\r", ch);
					return false;
				}
				
				DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
				ex->mode = EXITMODE_WEIGHTED;

				list_appendlink(dng->special_exits, ex);

				int index = list_size(dng->special_exits);

				send_to_char("Weighted special exit added.\n\r", ch);
				sprintf(buf, "Please add source exits using {Wspecial exit from {Y%d{W add ...{x\n\r", index);
				send_to_char(buf, ch);
				sprintf(buf, "Please add target entrances using {Wspecial exit to {Y%d{W add ...{x\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "group"))
			{
				if (!IS_NULLSTR(argument))
				{
					send_to_char("Syntax:  special exit add group\n\r", ch);
					return false;
				}
				
				DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
				ex->mode = EXITMODE_GROUP;
				
				list_appendlink(dng->special_exits, ex);

				int index = list_size(dng->special_exits);

				send_to_char("Group special exit added.\n\r", ch);
				sprintf(buf, "Please add exit definitions using {Wspecial exit group {Y%d{W add ...{x\n\r", index);
				send_to_char(buf, ch);
				return true;
			}

			send_to_char("Syntax:  special exit add {Wstatic{x generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit add {Wsource{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit add {Wdestination{x generated|ordinal <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit add {Wweighted{x\n\r", ch);
			send_to_char("         special exit add {Wgroup{x\n\r", ch);
			return false;
		}

		if (!str_prefix(arg2, "group"))
		{
			int levels = dungeon_index_generation_count(dng);
			char buf[MSL];
			char argg[MIL];			// used for group #
			char argg2[MIL];		// used for subcommand

			argument = one_argument(argument, argg);
			if (!is_number(argg))
			{
				send_to_char("Syntax:  special exit group {R#{x add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x add source generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x add destinationgenerated|ordinal  <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x add weighted\n\r", ch);
				send_to_char("         special exit group {R#{x from # list\n\r", ch);
				send_to_char("         special exit group {R#{x from # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x from # set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x from # remove #\n\r", ch);
				send_to_char("         special exit group {R#{x to # list\n\r", ch);
				send_to_char("         special exit group {R#{x to # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x to # set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x to # remove #\n\r", ch);
				send_to_char("         special exit group {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			int gindex = atoi(argg);
			if (gindex < 1 || gindex > list_size(dng->special_exits))
			{
				send_to_char("Syntax:  special exit group {R#{x add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x add source generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x add destinationgenerated|ordinal  <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x add weighted\n\r", ch);
				send_to_char("         special exit group {R#{x from # list\n\r", ch);
				send_to_char("         special exit group {R#{x from # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x from # set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group {R#{x from # remove #\n\r", ch);
				send_to_char("         special exit group {R#{x to # list\n\r", ch);
				send_to_char("         special exit group {R#{x to # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x to # set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group {R#{x to # remove #\n\r", ch);
				send_to_char("         special exit group {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_SPECIAL_EXIT *gex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(dng->special_exits, gindex);
			if (!IS_VALID(gex))
			{
				sprintf(buf, "Not such special exit %d found.\n\r", gindex);
				send_to_char(buf, ch);
				return false;
			}

			if (gex->mode != EXITMODE_GROUP)
			{
				sprintf(buf, "Special exit %d is not a GROUP exit.\n\r", gindex);
				send_to_char(buf, ch);
				return false;
			}

			argument = one_argument(argument, argg2);			

			if (!str_prefix(argg2, "add"))
			{
				if (list_size(dng->levels) < 1)
				{
					send_to_char("Please add level definitions before adding special exits.\n\r", ch);
					return false;
				}

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  special exit group # {Radd{x static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # {Radd{x source generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # {Radd{x destination generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         special exit group # {Radd{x weighted\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg3);

				// Add Static Special Exit
				// special exit group # add static <from-level> <from-exit> <to-level> <to-entrance>
				if (!str_prefix(arg3, "static"))
				{
					sent_bool from_mode = TRISTATE_UNDEF;
					sent_bool to_mode = TRISTATE_UNDEF;
					char argmf[MIL];
					char argmt[MIL];

					argument = one_argument(argument, argmf);
					if (!str_prefix(argmf, "generated"))
						from_mode = false;
					else if (!str_prefix(argmf, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # add static {Rgenerated|ordinal{x <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal {R<from-level>{x <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int from_level = atoi(arg4);
					if (from_level < 1 || from_level > levels)
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal {R<from-level>{x <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *from_level_data = dungeon_index_get_nth_level(dng, (from_mode?-from_level:from_level), &group);

					int from_exits = get_dungeon_index_level_special_exits(dng, group?group:from_level_data);

					argument = one_argument(argument, arg5);

					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> {R<from-exit>{x <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
						send_to_char(buf, ch);
						return false;
					}

					int from_exit = atoi(arg5);
					if (from_exit < 1 || from_exit > from_exits)
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> {R<from-exit>{x <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, argmt);
					if (!str_prefix(argmt, "generated"))
						to_mode = false;
					else if (!str_prefix(argmt, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> <from-exit> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);

					if (!is_number(arg6))
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> <from-exit> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int to_level = atoi(arg6);
					if (to_level < 1 || to_level > levels)
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> <from-exit> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *to_level_data = dungeon_index_get_nth_level(dng, (to_mode?-to_level:to_level), &group);

					int to_entries = get_dungeon_index_level_special_entrances(dng, to_level_data);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
						send_to_char(buf, ch);
						return false;
					}

					int to_entry = atoi(argument);
					if (to_entry < 1 || to_entry > to_entries)
					{
						send_to_char("Syntax:  special exit group # add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
						send_to_char(buf, ch);
						return false;
					}

					if (from_mode == to_mode && from_level == to_level && from_exit == to_entry)
					{
						send_to_char("Both exits are the same.  Unable to connect them.\n\r", ch);
						return false;
					}

					DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
					ex->mode = EXITMODE_STATIC;
					
					add_dungeon_index_weighted_exit_data(ex->from, 1, (from_mode?-from_level:from_level), from_exit);
					add_dungeon_index_weighted_exit_data(ex->to, 1, (to_mode?-to_level:to_level), to_entry);

					list_appendlink(gex->group, ex);

					send_to_char("Static special exit added to group.\n\r", ch);
					return true;
				}

				// Add Source Special Exit
				// special exit group # add source <to-level> <to-entrance>
				if (!str_prefix(arg3, "source"))
				{
					sent_bool to_mode = TRISTATE_UNDEF;
					char argmt[MIL];

					argument = one_argument(argument, argmt);
					if (!str_prefix(argmt, "generated"))
						to_mode = false;
					else if (!str_prefix(argmt, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # add source {Rgenerate|ordinal{x <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);

					if (!is_number(arg6))
					{
						send_to_char("Syntax:  special exit group # add source generate|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int to_level = atoi(arg6);
					if (to_level < 1 || to_level > levels)
					{
						send_to_char("Syntax:  special exit group # add source generate|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a target level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *to_level_data = dungeon_index_get_nth_level(dng, (to_mode?-to_level:to_level), &group);

					int to_entries = get_dungeon_index_level_special_entrances(dng, group?group:to_level_data);

					if (IS_NULLSTR(argument))
					{
						send_to_char("Syntax:  special exit group # add source <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
						send_to_char(buf, ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # add source <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
						send_to_char(buf, ch);
						return false;
					}

					int to_entry = atoi(argument);
					if (to_entry < 1 || to_entry > to_entries)
					{
						send_to_char("Syntax:  special exit group # add source <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a target entrance number from 1 to %d.\n\r", to_entries);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
					ex->mode = EXITMODE_WEIGHTED_SOURCE;
					
					add_dungeon_index_weighted_exit_data(ex->to, 1, (to_mode?-to_level:to_level), to_entry);

					list_appendlink(gex->group, ex);

					int index = list_size(gex->group);

					send_to_char("Source special exit group # added.\n\r", ch);
					sprintf(buf, "Please add source exits using {Wspecial exit group {Y%d{W from {Y%d{W add ...{x\n\r", gindex, index);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "destination"))
				{
					sent_bool from_mode = TRISTATE_UNDEF;
					char argmf[MIL];

					argument = one_argument(argument, argmf);
					if (!str_prefix(argmf, "generated"))
						from_mode = false;
					else if (!str_prefix(argmf, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # add destination {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);

					if (!is_number(arg4))
					{
						send_to_char("Syntax:  special exit group # add destination generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int from_level = atoi(arg4);
					if (from_level < 1 || from_level > levels)
					{
						send_to_char("Syntax:  special exit group # add destination generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a source level number from 1 to %d.\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *from_level_data = dungeon_index_get_nth_level(dng, (from_mode?-from_level:from_level), &group);

					int from_exits = get_dungeon_index_level_special_exits(dng, group?group:from_level_data);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # add destination generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
						send_to_char(buf, ch);
						return false;
					}

					int from_exit = atoi(argument);
					if (from_exit < 1 || from_exit > from_exits)
					{
						send_to_char("Syntax:  special exit group # add destination generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a source exit number from 1 to %d.\n\r", from_exits);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
					ex->mode = EXITMODE_WEIGHTED_DEST;
					
					add_dungeon_index_weighted_exit_data(ex->from, 1, (from_mode?-from_level:from_level), from_exit);

					list_appendlink(gex->group, ex);

					int index = list_size(gex->group);

					send_to_char("Destination special exit added to group.\n\r", ch);
					sprintf(buf, "Please add target entrances using {Wspecial exit group {Y%d{W to {Y%d{W add ...{x\n\r", gindex, index);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "weighted"))
				{
					if (!IS_NULLSTR(argument))
					{
						send_to_char("Syntax:  special exit group # add weighted\n\r", ch);
						return false;
					}
					
					DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
					ex->mode = EXITMODE_WEIGHTED;

					list_appendlink(gex->group, ex);

					int index = list_size(gex->group);

					send_to_char("Weighted special exit group # added.\n\r", ch);
					sprintf(buf, "Please add source exits using {Wspecial exit group {Y%d{W from {Y%d{W add ...{x\n\r", gindex, index);
					send_to_char(buf, ch);
					sprintf(buf, "Please add target entrances using {Wspecial exit group {Y%d{W to {Y%d{W add ...{x\n\r", gindex, index);
					send_to_char(buf, ch);
					return true;
				}

				send_to_char("Syntax:  special exit group # add {Wstatic{x <from-level> <from-exit> <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group # add {Wsource{x <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group # add {Wdestination{x <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group # add {Wweighted{x\n\r", ch);
				return false;
			}

			if (!str_prefix(arg2, "from"))
			{
				if (list_size(gex->group) < 1)
				{
					send_to_char("Please add a special exit definition first.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg3);
				if (!is_number(arg3))
				{
					send_to_char("Syntax:  special exit group # from {R#{x list\n\r", ch);
					send_to_char("         special exit group # from {R#{x add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         special exit group # from {R#{x set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         special exit group # from {R#{x remove #\n\r", ch);
					sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg3);
				if (index < 1 || index > list_size(gex->group))
				{
					send_to_char("Syntax:  special exit group # from {R#{x list\n\r", ch);
					send_to_char("         special exit group # from {R#{x add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         special exit group # from {R#{x set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         special exit group # from {R#{x remove #\n\r", ch);
					sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_SPECIAL_EXIT *ex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(gex->group, index);

				if (ex->mode == EXITMODE_GROUP)
				{
					send_to_char("Cannot alter the From definitions on a GROUP exit.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);

				if (!str_prefix(arg4, "list"))
				{
					if (list_size(ex->from) > 0)
					{
						BUFFER *buffer = new_buf();
						ITERATOR fit;

						add_buf(buffer, "     [ Weight ] [ Level ] [ Exit ]\n\r");
						add_buf(buffer, "===================================\n\r");
						
						int findex = 1;
						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
						iterator_start(&fit, ex->from);
						while( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&fit)) )
						{
							sprintf(buf, "%4d   %6d     {%c%5d{x     %4d\n\r", findex++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
							add_buf(buffer, buf);
						}
						iterator_stop(&fit);

						add_buf(buffer, "-----------------------------------\n\r");

						if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
						{
							send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
						}
						else
						{
							page_to_char(buffer->string, ch);
						}
						free_buf(buffer);
					}
					else
					{
						send_to_char("There are no From definitions on this special exit.\n\r", ch);
					}
					return false;
				}

				if (!str_prefix(arg4, "add"))
				{
					if (ex->mode == EXITMODE_STATIC)
					{
						sprintf(buf, "Special exit %d is a STATIC exit.  Cannot add any new From definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (ex->mode == EXITMODE_WEIGHTED_DEST)
					{
						sprintf(buf, "Special exit %d is a DESTINATION exit.  Cannot add any new From definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit group # from # add {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit group # from # add {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool from_mode = TRISTATE_UNDEF;
					char argmf[MIL];

					argument = one_argument(argument, argmf);
					if (!str_prefix(argmf, "generated"))
						from_mode = false;
					else if (!str_prefix(argmf, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # from # add <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit group # from # add <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int flindex = atoi(arg7);
					if (flindex < 1 || flindex > levels)
					{
						send_to_char("Syntax:  special exit group # from # add <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
					int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # from # add <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					int fexit = atoi(argument);
					if (fexit < 1 || fexit > fexits)
					{
						send_to_char("Syntax:  special exit group # from # add <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					add_dungeon_index_weighted_exit_data(ex->from, weight, (from_mode?-flindex:flindex), fexit);
					ex->total_from += weight;

					sprintf(buf, "From definition added to special exit %d.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (!str_prefix(arg4, "set"))
				{
					if (ex->mode == EXITMODE_STATIC || ex->mode == EXITMODE_WEIGHTED_DEST)
					{
						if (list_size(ex->from) < 1)
						{
							send_to_char("Special exit appears to be missing necessary From definition.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg6);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # from # set {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						int weight = atoi(arg6);
						if (weight < 1)
						{
							send_to_char("Syntax:  special exit group # from # set {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						sent_bool from_mode = TRISTATE_UNDEF;
						char argmf[MIL];

						if (!str_prefix(argmf, "generated"))
							from_mode = false;
						else if (!str_prefix(argmf, "ordinal"))
							from_mode = true;
						else
						{
							send_to_char("Syntax:  special exit group # from # set <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
							send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg7);
						if (!is_number(arg7))
						{
							send_to_char("Syntax:  special exit group # from # set <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						int flindex = atoi(arg7);
						if (flindex < 1 || flindex > levels)
						{
							send_to_char("Syntax:  special exit group # from # set <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_LEVEL_DATA *group;
						DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
						int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

						if (!is_number(argument))
						{
							send_to_char("Syntax:  special exit group # from # set <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
							send_to_char(buf, ch);
							return false;
						}

						int fexit = atoi(argument);
						if (fexit < 1 || fexit > fexits)
						{
							send_to_char("Syntax:  special exit group # from # set <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->from, 1);
						ex->total_from -= fex->weight;
						ex->total_from += weight;

						fex->weight = weight;
						fex->level = from_mode?-flindex:flindex;
						fex->door = fexit;

						sprintf(buf, "From definition set on special exit %d.\n\r", index);
						send_to_char(buf, ch);
						return true;
					}
					else
					{
						if (list_size(ex->from) < 1)
						{
							send_to_char("Special exit has no From definition.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg5);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # from # set {R#{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
							send_to_char(buf, ch);
							return false;
						}

						int findex = atoi(arg5);
						if (findex < 1 || findex > list_size(ex->from))
						{
							send_to_char("Syntax:  special exit group # from # set {R#{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
							send_to_char(buf, ch);
							return false;
						}

						argument = one_argument(argument, arg6);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # from # set # {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						int weight = atoi(arg6);
						if (weight < 1)
						{
							send_to_char("Syntax:  special exit group # from # set # {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						sent_bool from_mode = TRISTATE_UNDEF;
						char argmf[MIL];

						if (!str_prefix(argmf, "generated"))
							from_mode = false;
						else if (!str_prefix(argmf, "ordinal"))
							from_mode = true;
						else
						{
							send_to_char("Syntax:  special exit group # from # set # <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
							send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg7);
						if (!is_number(arg7))
						{
							send_to_char("Syntax:  special exit group # from # set # <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						int flindex = atoi(arg7);
						if (flindex < 1 || flindex > levels)
						{
							send_to_char("Syntax:  special exit group # from # set # <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_LEVEL_DATA *group;
						DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
						int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

						if (!is_number(argument))
						{
							send_to_char("Syntax:  special exit group # from # set # <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
							send_to_char(buf, ch);
							return false;
						}

						int fexit = atoi(argument);
						if (fexit < 1 || fexit > fexits)
						{
							send_to_char("Syntax:  special exit group # from # set # <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
							sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->from, findex);
						ex->total_from -= fex->weight;
						ex->total_from += weight;

						fex->weight = weight;
						fex->level = from_mode?-flindex:flindex;
						fex->door = fexit;

						sprintf(buf, "From definition %d set on special exit %d.\n\r", findex, index);
						send_to_char(buf, ch);
						return true;
					}
				}

				if (!str_prefix(arg4, "remove"))
				{
					if (ex->mode == EXITMODE_STATIC)
					{
						sprintf(buf, "Special exit %d is a STATIC exit.  Cannot remove the From definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (ex->mode == EXITMODE_WEIGHTED_DEST)
					{
						sprintf(buf, "Special exit %d is a DESTINATION exit.  Cannot remove the From definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # from # remove {R#{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
						send_to_char(buf, ch);
						return false;
					}

					int findex = atoi(argument);
					if (findex < 1 || findex > list_size(ex->from))
					{
						send_to_char("Syntax:  special exit group # from # remove {R#{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
						send_to_char(buf, ch);
						return false;
					}

					list_remnthlink(ex->from, findex, true);
					send_to_char("From definition removed from special exit.\n\r", ch);
					if (list_size(ex->from) < 1)
						send_to_char("{RWarning:{x Please add a from definition for this exit to work.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  special exit group # from # {Rlist{x\n\r", ch);
				send_to_char("         special exit group # from # {Radd{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group # from # {Rset{x[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit group # from # {Rremove{x #\n\r", ch);
				return false;
			}

			if (!str_prefix(arg2, "to"))
			{
				if (list_size(gex->group) < 1)
				{
					send_to_char("Please add a special exit definition first.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg3);
				if (!is_number(arg3))
				{
					send_to_char("Syntax:  special exit group # to {R#{x list\n\r", ch);
					send_to_char("         special exit group # to {R#{x add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # to {R#{x set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # to {R#{x remove #\n\r", ch);
					sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg3);
				if (index < 1 || index > list_size(gex->group))
				{
					send_to_char("Syntax:  special exit group # to {R#{x list\n\r", ch);
					send_to_char("         special exit group # to {R#{x add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # to {R#{x set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         special exit group # to {R#{x remove #\n\r", ch);
					sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_SPECIAL_EXIT *ex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(gex->group, index);

				if (ex->mode == EXITMODE_GROUP)
				{
					send_to_char("Cannot alter the From definitions on a GROUP exit.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);

				if (!str_prefix(arg4, "list"))
				{
					if (list_size(ex->to) > 0)
					{
						BUFFER *buffer = new_buf();
						ITERATOR fit;

						add_buf(buffer, "     [ Weight ] [ Level ] [ Exit ]\n\r");
						add_buf(buffer, "===================================\n\r");
						
						int tindex = 1;
						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
						iterator_start(&fit, ex->to);
						while( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&fit)) )
						{
							sprintf(buf, "%4d   %6d     {%c%5d{x     %4d\n\r", tindex++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
							add_buf(buffer, buf);
						}
						iterator_stop(&fit);

						add_buf(buffer, "-----------------------------------\n\r");

						if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
						{
							send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
						}
						else
						{
							page_to_char(buffer->string, ch);
						}
						free_buf(buffer);
					}
					else
					{
						send_to_char("There are no To definitions on this special exit.\n\r", ch);
					}
					return false;
				}

				if (!str_prefix(arg4, "add"))
				{
					if (ex->mode == EXITMODE_STATIC)
					{
						sprintf(buf, "Special exit %d is a STATIC exit.  Cannot add any new To definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (ex->mode == EXITMODE_WEIGHTED_SOURCE)
					{
						sprintf(buf, "Special exit %d is a SOURCE exit.  Cannot add any new To definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit group # to # add {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit group # to # add {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool to_mode = TRISTATE_UNDEF;
					char argmt[MIL];

					argument = one_argument(argument, argmt);
					if (!str_prefix(argmt, "generated"))
						to_mode = false;
					else if (!str_prefix(argmt, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  special exit group # to # add <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit group # to # add <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int tlindex = atoi(arg7);
					if (tlindex < 1 || tlindex > levels)
					{
						send_to_char("Syntax:  special exit group # to # add <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
					int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # to # add <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					int tentry = atoi(argument);
					if (tentry < 1 || tentry > tentries)
					{
						send_to_char("Syntax:  special exit group # to # add <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					add_dungeon_index_weighted_exit_data(ex->to, weight, (to_mode?-tlindex:tlindex), tentry);
					ex->total_to += weight;

					sprintf(buf, "To definition added to special exit %d.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (!str_prefix(arg4, "set"))
				{
					if (ex->mode == EXITMODE_STATIC || ex->mode == EXITMODE_WEIGHTED_SOURCE)
					{
						if (list_size(ex->to) < 1)
						{
							send_to_char("Special exit appears to be missing necessary To definition.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg6);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # to # set {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						int weight = atoi(arg6);
						if (weight < 1)
						{
							send_to_char("Syntax:  special exit group # to # set {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						sent_bool to_mode = TRISTATE_UNDEF;
						char argmf[MIL];

						if (!str_prefix(argmf, "generated"))
							to_mode = false;
						else if (!str_prefix(argmf, "ordinal"))
							to_mode = true;
						else
						{
							send_to_char("Syntax:  special exit group # to # set <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
							send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg7);
						if (!is_number(arg7))
						{
							send_to_char("Syntax:  special exit group # to # set <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						int tlindex = atoi(arg7);
						if (tlindex < 1 || tlindex > levels)
						{
							send_to_char("Syntax:  special exit group # to # set <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_LEVEL_DATA *group;
						DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
						int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

						if (!is_number(argument))
						{
							send_to_char("Syntax:  special exit group # to # set <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
							send_to_char(buf, ch);
							return false;
						}

						int tentry = atoi(argument);
						if (tentry < 1 || tentry > tentries)
						{
							send_to_char("Syntax:  special exit group # to # set <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *tex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->to, 1);
						ex->total_to -= tex->weight;
						ex->total_to += weight;

						tex->weight = weight;
						tex->level = to_mode?-tlindex:tlindex;
						tex->door = tentry;

						sprintf(buf, "To definition set on special exit %d.\n\r", index);
						send_to_char(buf, ch);
						return true;
					}
					else
					{
						if (list_size(ex->to) < 1)
						{
							send_to_char("Special exit has no From definition.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg5);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # to # set {R#{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
							send_to_char(buf, ch);
							return false;
						}

						int tindex = atoi(arg5);
						if (tindex < 1 || tindex > list_size(ex->to))
						{
							send_to_char("Syntax:  special exit group # to # set {R#{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
							send_to_char(buf, ch);
							return false;
						}

						argument = one_argument(argument, arg6);
						if (!is_number(arg5))
						{
							send_to_char("Syntax:  special exit group # to # set # {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						int weight = atoi(arg6);
						if (weight < 1)
						{
							send_to_char("Syntax:  special exit group # to # set # {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
							send_to_char("         Please specify a positive number.\n\r", ch);
							return false;
						}

						sent_bool to_mode = TRISTATE_UNDEF;
						char argmf[MIL];

						if (!str_prefix(argmf, "generated"))
							to_mode = false;
						else if (!str_prefix(argmf, "ordinal"))
							to_mode = true;
						else
						{
							send_to_char("Syntax:  special exit group # to # set # <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
							send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
							return false;
						}

						argument = one_argument(argument, arg7);
						if (!is_number(arg7))
						{
							send_to_char("Syntax:  special exit group # to # set # <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						int tlindex = atoi(arg7);
						if (tlindex < 1 || tlindex > levels)
						{
							send_to_char("Syntax:  special exit group # to # set # <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_LEVEL_DATA *group;
						DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
						int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

						if (!is_number(argument))
						{
							send_to_char("Syntax:  special exit group # to # set # <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
							send_to_char(buf, ch);
							return false;
						}

						int tentry = atoi(argument);
						if (tentry < 1 || tentry > tentries)
						{
							send_to_char("Syntax:  special exit group # to # set # <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
							sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
							send_to_char(buf, ch);
							return false;
						}

						DUNGEON_INDEX_WEIGHTED_EXIT_DATA *tex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->to, tindex);
						ex->total_to -= tex->weight;
						ex->total_to += weight;

						tex->weight = weight;
						tex->level = to_mode?-tlindex:tlindex;
						tex->door = tentry;

						sprintf(buf, "To definition %d set on special exit %d.\n\r", tindex, index);
						send_to_char(buf, ch);
						return true;
					}
				}

				if (!str_prefix(arg4, "remove"))
				{
					if (ex->mode == EXITMODE_STATIC)
					{
						sprintf(buf, "Special exit %d is a STATIC exit.  Cannot remove the To definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (ex->mode == EXITMODE_WEIGHTED_SOURCE)
					{
						sprintf(buf, "Special exit %d is a SOURCE exit.  Cannot remove the To definition.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit group # to # remove {R#{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
						send_to_char(buf, ch);
						return false;
					}

					int tindex = atoi(argument);
					if (tindex < 1 || tindex > list_size(ex->to))
					{
						send_to_char("Syntax:  special exit group # to # remove {R#{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
						send_to_char(buf, ch);
						return false;
					}

					list_remnthlink(ex->to, tindex, true);
					send_to_char("To definition removed to special exit.\n\r", ch);
					if (list_size(ex->to) < 1)
						send_to_char("{RWarning:{x Please add a To definition for this exit to work.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  special exit group # to # {Rlist{x\n\r", ch);
				send_to_char("         special exit group # to # {Radd{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group # to # {Rset{x[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit group # to # {Rremove{x #\n\r", ch);
				return false;
			}

			if (!str_prefix(arg2, "remove"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  special exit group # remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				if (list_size(dng->special_exits) < 1)
				{
					send_to_char("There are no special exits.\n\r", ch);
					return false;
				}

				int index = atoi(argument);
				if (index < 1 || index > list_size(gex->group))
				{
					send_to_char("Syntax:  special exit remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(gex->group));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(gex->group, index, true);
				sprintf(buf, "Special exit %d removed from group %d.\n\r", index, gindex);
				send_to_char(buf, ch);
				return true;
			}


			send_to_char("Syntax:  special exit group # {Radd{x static <from-level> <from-exit> <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit group # {Radd{x source <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit group # {Radd{x destination <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit group # {Radd{x weighted\n\r", ch);
			send_to_char("         special exit group # {Rfrom{x # list\n\r", ch);
			send_to_char("         special exit group # {Rfrom{x # add <weight> <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit group # {Rfrom{x # set[ #] <weight> <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit group # {Rfrom{x # remove #\n\r", ch);
			send_to_char("         special exit group # {Rto{x # list\n\r", ch);
			send_to_char("         special exit group # {Rto{x # add <weight> <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit group # {Rto{x # set[ #] <weight> <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit group # {Rto{x # remove #\n\r", ch);
			send_to_char("         special exit group # {Rremove{x #\n\r", ch);
			return false;
		}

		if (!str_prefix(arg2, "from"))
		{
			int levels = dungeon_index_generation_count(dng);
			char buf[MSL];
			if (list_size(dng->special_exits) < 1)
			{
				send_to_char("Please add a special exit definition first.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!is_number(arg3))
			{
				send_to_char("Syntax:  special exit from {R#{x list\n\r", ch);
				send_to_char("         special exit from {R#{x add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from {R#{x set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg3);
			if (index < 1 || index > list_size(dng->special_exits))
			{
				send_to_char("Syntax:  special exit from {R#{x list\n\r", ch);
				send_to_char("         special exit from {R#{x add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from {R#{x set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_SPECIAL_EXIT *ex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(dng->special_exits, index);

			if (ex->mode == EXITMODE_GROUP)
			{
				send_to_char("Cannot alter the From definitions on a GROUP exit.\n\r", ch);
				return false;
			}

			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  special exit from # {Rlist{x\n\r", ch);
				send_to_char("         special exit from # {Radd{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from # {Rset{x[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
				send_to_char("         special exit from # {Rremove{x #\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);

			if (!str_prefix(arg4, "list"))
			{
				if (list_size(ex->from) > 0)
				{
					BUFFER *buffer = new_buf();
					ITERATOR fit;

					add_buf(buffer, "     [ Weight ] [ Level ] [ Exit ]\n\r");
					add_buf(buffer, "===================================\n\r");
					
					int findex = 1;
					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
					iterator_start(&fit, ex->from);
					while( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&fit)) )
					{
						sprintf(buf, "%4d   %6d     {%c%5d{x     %4d\n\r", findex++, from->weight, (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
						add_buf(buffer, buf);
					}
					iterator_stop(&fit);

					add_buf(buffer, "-----------------------------------\n\r");

					if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
					{
						send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
					}
					else
					{
						page_to_char(buffer->string, ch);
					}
					free_buf(buffer);
				}
				else
				{
					send_to_char("There are no From definitions on this special exit.\n\r", ch);
				}
				return false;
			}

			if (!str_prefix(arg4, "add"))
			{
				if (ex->mode == EXITMODE_STATIC)
				{
					sprintf(buf, "Special exit %d is a STATIC exit.  Cannot add any new From definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (ex->mode == EXITMODE_WEIGHTED_DEST)
				{
					sprintf(buf, "Special exit %d is a DESTINATION exit.  Cannot add any new From definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  special exit from # add {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg6);
				if (weight < 1)
				{
					send_to_char("Syntax:  special exit from # add {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
					send_to_char("         Please specify a positive number.\n\r", ch);
					return false;
				}

				sent_bool from_mode = TRISTATE_UNDEF;
				char argm[MIL];
				
				argument = one_argument(argument, argm);
				if(!str_prefix(argm, "generated"))
					from_mode = false;
				else if(!str_prefix(argm, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  special exit from # add <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg7);
				if (!is_number(arg7))
				{
					send_to_char("Syntax:  special exit from # add <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				int flindex = atoi(arg7);
				if (flindex < 1 || flindex > levels)
				{
					send_to_char("Syntax:  special exit from # add <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *group;
				DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
				int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit from # add <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
					send_to_char(buf, ch);
					return false;
				}

				int fexit = atoi(argument);
				if (fexit < 1 || fexit > fexits)
				{
					send_to_char("Syntax:  special exit from # add <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
					send_to_char(buf, ch);
					return false;
				}

				add_dungeon_index_weighted_exit_data(ex->from, weight, flindex, fexit);
				ex->total_from += weight;

				sprintf(buf, "From definition added to special exit %d.\n\r", index);
				send_to_char(buf, ch);
				return false;
			}

			if (!str_prefix(arg4, "set"))
			{
				if (ex->mode == EXITMODE_STATIC || ex->mode == EXITMODE_WEIGHTED_DEST)
				{
					if (list_size(ex->from) < 1)
					{
						send_to_char("Special exit appears to be missing necessary From definition.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit from # set {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit from # set {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool from_mode = TRISTATE_UNDEF;
					char argm[MIL];

					argument = one_argument(argument, argm);
					if (!str_prefix(argm, "generated"))
						from_mode = false;
					else if (!str_prefix(argm, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  special exit from # set <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit from # set <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int flindex = atoi(arg7);
					if (flindex < 1 || flindex > levels)
					{
						send_to_char("Syntax:  special exit from # set <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
					int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit from # set <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					int fexit = atoi(argument);
					if (fexit < 1 || fexit > fexits)
					{
						send_to_char("Syntax:  special exit from # set <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->from, 1);
					ex->total_from -= fex->weight;
					ex->total_from += weight;

					fex->weight = weight;
					fex->level = from_mode?-flindex:flindex;
					fex->door = fexit;

					sprintf(buf, "From definition set on special exit %d.\n\r", index);
					send_to_char(buf, ch);
					return true;
				}
				else
				{
					if (list_size(ex->from) < 1)
					{
						send_to_char("Special exit has no From definition.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit from # set {R#{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
						send_to_char(buf, ch);
						return false;
					}

					int findex = atoi(arg5);
					if (findex < 1 || findex > list_size(ex->from))
					{
						send_to_char("Syntax:  special exit from # set {R#{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit from # set # {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit from # set # {R<weight>{x generated|ordinal <from-level> <from-exit>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool from_mode = TRISTATE_UNDEF;
					char argm[MIL];

					argument = one_argument(argument, argm);
					if (!str_prefix(argm, "generated"))
						from_mode = false;
					else if (!str_prefix(argm, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  special exit from # set # <weight> {Rgenerated|ordinal{x <from-level> <from-exit>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit from # set # <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", list_size(dng->levels));
						send_to_char(buf, ch);
						return false;
					}

					int flindex = atoi(arg7);
					if (flindex < 1 || flindex > list_size(dng->levels))
					{
						send_to_char("Syntax:  special exit from # set # <weight> generated|ordinal {R<from-level>{x <from-exit>\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", list_size(dng->levels));
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (from_mode?-flindex:flindex), &group);
					int fexits = get_dungeon_index_level_special_exits(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit from # set # <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					int fexit = atoi(argument);
					if (fexit < 1 || fexit > fexits)
					{
						send_to_char("Syntax:  special exit from # set # <weight> generated|ordinal <from-level> {R<from-exit>{x\n\r", ch);
						sprintf(buf, "         Please specify a number from 1 to %d\n\r", fexits);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->from, findex);
					ex->total_from -= fex->weight;
					ex->total_from += weight;

					fex->weight = weight;
					fex->level = from_mode?-flindex:flindex;
					fex->door = fexit;

					sprintf(buf, "From definition %d set on special exit %d.\n\r", findex, index);
					send_to_char(buf, ch);
					return true;

				}
			}

			if (!str_prefix(arg4, "remove"))
			{
				char buf[MSL];
				if (ex->mode == EXITMODE_STATIC)
				{
					sprintf(buf, "Special exit %d is a STATIC exit.  Cannot remove the From definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (ex->mode == EXITMODE_WEIGHTED_DEST)
				{
					sprintf(buf, "Special exit %d is a DESTINATION exit.  Cannot remove the From definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit from # remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
					send_to_char(buf, ch);
					return false;
				}

				int findex = atoi(argument);
				if (findex < 1 || findex > list_size(ex->from))
				{
					send_to_char("Syntax:  special exit from # remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(ex->from));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(ex->from, findex, true);
				send_to_char("From definition removed from special exit.\n\r", ch);
				if (list_size(ex->from) < 1)
					send_to_char("{RWarning:{x Please add a from definition for this exit to work.\n\r", ch);
				return true;
			}

			send_to_char("Syntax:  special exit from # {Rlist{x\n\r", ch);
			send_to_char("         special exit from # {Radd{x <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit from # {Rset{x[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
			send_to_char("         special exit from # {Rremove{x #\n\r", ch);
			return false;
		}

		if (!str_prefix(arg2, "to"))
		{
			int levels = dungeon_index_generation_count(dng);
			char buf[MSL];
			if (list_size(dng->special_exits) < 1)
			{
				send_to_char("Please add a special exit definition first.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!is_number(arg3))
			{
				send_to_char("Syntax:  special exit to {R#{x list\n\r", ch);
				send_to_char("         special exit to {R#{x add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to {R#{x set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg3);
			if (index < 1 || index > list_size(dng->special_exits))
			{
				send_to_char("Syntax:  special exit to {R#{x list\n\r", ch);
				send_to_char("         special exit to {R#{x add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to {R#{x set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to {R#{x remove #\n\r", ch);
				sprintf(buf, "         Please specify a number between 1 and {Y%d{x.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			DUNGEON_INDEX_SPECIAL_EXIT *ex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(dng->special_exits, index);

			if (ex->mode == EXITMODE_GROUP)
			{
				send_to_char("Cannot alter the To definitions on a GROUP exit.\n\r", ch);
				return false;
			}

			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  special exit to # {Rlist{x\n\r", ch);
				send_to_char("         special exit to # {Radd{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to # {Rset{x[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
				send_to_char("         special exit to # {Rremove{x #\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);

			if (!str_prefix(arg4, "list"))
			{
				if (list_size(ex->to) > 0)
				{
					BUFFER *buffer = new_buf();
					ITERATOR fit;

					add_buf(buffer, "     [ Weight ] [ Level ] [ Exit ]\n\r");
					add_buf(buffer, "===================================\n\r");
					
					int tindex = 1;
					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;
					iterator_start(&fit, ex->to);
					while( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&fit)) )
					{
						sprintf(buf, "%4d   %6d     {%c%5d{x     %4d\n\r", tindex++, to->weight, (to->level<0?'G':(to->level>0?'Y':'W')), abs(to->level), to->door);
						add_buf(buffer, buf);
					}
					iterator_stop(&fit);

					add_buf(buffer, "-----------------------------------\n\r");

					if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH)
					{
						send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
					}
					else
					{
						page_to_char(buffer->string, ch);
					}
					free_buf(buffer);
				}
				else
				{
					send_to_char("There are no To definitions on this special exit.\n\r", ch);
				}
				return false;
			}

			if (!str_prefix(arg4, "add"))
			{
				if (ex->mode == EXITMODE_STATIC)
				{
					sprintf(buf, "Special exit %d is a STATIC exit.  Cannot add any new To definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (ex->mode == EXITMODE_WEIGHTED_SOURCE)
				{
					sprintf(buf, "Special exit %d is a SOURCE exit.  Cannot add any new To definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  special exit to # add {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg6);
				if (weight < 1)
				{
					send_to_char("Syntax:  special exit to # add {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
					send_to_char("         Please specify a positive number.\n\r", ch);
					return false;
				}

				sent_bool to_mode = TRISTATE_UNDEF;
				char argm[MIL];

				argument = one_argument(argument, argm);
				if (!str_prefix(argm, "generated"))
					to_mode = false;
				else if (!str_prefix(argm, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  special exit to # add <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg7);
				if (!is_number(arg7))
				{
					send_to_char("Syntax:  special exit to # add <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				int tlindex = atoi(arg7);
				if (tlindex < 1 || tlindex > levels)
				{
					send_to_char("Syntax:  special exit to # add <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
					send_to_char(buf, ch);
					return false;
				}

				DUNGEON_INDEX_LEVEL_DATA *group;
				DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
				int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit to # add <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
					send_to_char(buf, ch);
					return false;
				}

				int tentry = atoi(argument);
				if (tentry < 1 || tentry > tentries)
				{
					send_to_char("Syntax:  special exit to # add <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
					send_to_char(buf, ch);
					return false;
				}

				add_dungeon_index_weighted_exit_data(ex->to, weight, to_mode?-tlindex:tlindex, tentry);
				ex->total_to += weight;

				sprintf(buf, "To definition added to special exit %d.\n\r", index);
				send_to_char(buf, ch);
				return false;
			}

			if (!str_prefix(arg4, "set"))
			{
				if (ex->mode == EXITMODE_STATIC || ex->mode == EXITMODE_WEIGHTED_SOURCE)
				{
					if (list_size(ex->to) < 1)
					{
						send_to_char("Special exit appears to be missing necessary To definition.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit to # set {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit to # set {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool to_mode = TRISTATE_UNDEF;
					char argm[MIL];

					argument = one_argument(argument, argm);
					if (!str_prefix(argm, "generated"))
						to_mode = false;
					else if (!str_prefix(argm, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  special exit to # set <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit to # set <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					int tlindex = atoi(arg7);
					if (tlindex < 1 || tlindex > levels)
					{
						send_to_char("Syntax:  special exit to # set <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", levels);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
					int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit to # set <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					int tentry = atoi(argument);
					if (tentry < 1 || tentry > tentries)
					{
						send_to_char("Syntax:  special exit to # set <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->to, 1);
					ex->total_to -= fex->weight;
					ex->total_to += weight;

					fex->weight = weight;
					fex->level = to_mode?-tlindex:tlindex;
					fex->door = tentry;

					sprintf(buf, "To definition set on special exit %d.\n\r", index);
					send_to_char(buf, ch);
					return true;
				}
				else
				{
					if (list_size(ex->to) < 1)
					{
						send_to_char("Special exit has no To definition.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit to # set {R#{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
						send_to_char(buf, ch);
						return false;
					}

					int tindex = atoi(arg5);
					if (tindex < 1 || tindex > list_size(ex->to))
					{
						send_to_char("Syntax:  special exit to # set {R#{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  special exit to # set # {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg6);
					if (weight < 1)
					{
						send_to_char("Syntax:  special exit to # set # {R<weight>{x generated|ordinal <to-level> <to-entrance>\n\r", ch);
						send_to_char("         Please specify a positive number.\n\r", ch);
						return false;
					}

					sent_bool to_mode = TRISTATE_UNDEF;
					char argm[MIL];

					argument = one_argument(argument, argm);
					if (!str_prefix(argm, "generated"))
						to_mode = false;
					else if (!str_prefix(argm, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  special exit to # set # <weight> {Rgenerated|ordinal{x <to-level> <to-entrance>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  special exit to # set # <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", list_size(dng->levels));
						send_to_char(buf, ch);
						return false;
					}

					int tlindex = atoi(arg7);
					if (tlindex < 1 || tlindex > list_size(dng->levels))
					{
						send_to_char("Syntax:  special exit to # set # <weight> generated|ordinal {R<to-level>{x <to-entrance>\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", list_size(dng->levels));
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_LEVEL_DATA *group;
					DUNGEON_INDEX_LEVEL_DATA *flevel = dungeon_index_get_nth_level(dng, (to_mode?-tlindex:tlindex), &group);
					int tentries = get_dungeon_index_level_special_entrances(dng, group?group:flevel);

					if (!is_number(argument))
					{
						send_to_char("Syntax:  special exit to # set # <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					int tentry = atoi(argument);
					if (tentry < 1 || tentry > tentries)
					{
						send_to_char("Syntax:  special exit to # set # <weight> generated|ordinal <to-level> {R<to-entrance>{x\n\r", ch);
						sprintf(buf, "         Please specify a number to 1 to %d\n\r", tentries);
						send_to_char(buf, ch);
						return false;
					}

					DUNGEON_INDEX_WEIGHTED_EXIT_DATA *fex = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(ex->to, tindex);
					ex->total_to -= fex->weight;
					ex->total_to += weight;

					fex->weight = weight;
					fex->level = to_mode?-tlindex:tlindex;
					fex->door = tentry;

					sprintf(buf, "To definition %d set on special exit %d.\n\r", tindex, index);
					send_to_char(buf, ch);
					return true;

				}
			}

			if (!str_prefix(arg4, "remove"))
			{
				char buf[MSL];
				if (ex->mode == EXITMODE_STATIC)
				{
					sprintf(buf, "Special exit %d is a STATIC exit.  Cannot remove the To definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (ex->mode == EXITMODE_WEIGHTED_SOURCE)
				{
					sprintf(buf, "Special exit %d is a SOURCE exit.  Cannot remove the To definition.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  special exit to # remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
					send_to_char(buf, ch);
					return false;
				}

				int tindex = atoi(argument);
				if (tindex < 1 || tindex > list_size(ex->to))
				{
					send_to_char("Syntax:  special exit to # remove {R#{x\n\r", ch);
					sprintf(buf, "         Please specify a number to 1 to %d.\n\r", list_size(ex->to));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(ex->to, tindex, true);
				send_to_char("To definition removed to special exit.\n\r", ch);
				if (list_size(ex->to) < 1)
					send_to_char("{RWarning:{x Please add a to definition for this exit to work.\n\r", ch);
				return true;
			}

			send_to_char("Syntax:  special exit to # {Rlist{x\n\r", ch);
			send_to_char("         special exit to # {Radd{x <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit to # {Rset{x[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
			send_to_char("         special exit to # {Rremove{x #\n\r", ch);
			return false;
		}

		if (!str_prefix(arg2, "remove"))
		{
			char buf[MSL];
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  special exit remove {R#{x\n\r", ch);
				sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  special exit remove {R#{x\n\r", ch);
				sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			if (list_size(dng->special_exits) < 1)
			{
				send_to_char("There are no special exits.\n\r", ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(dng->special_exits))
			{
				send_to_char("Syntax:  special exit remove {R#{x\n\r", ch);
				sprintf(buf, "         Please specify a number from 1 to %d.\n\r", list_size(dng->special_exits));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(dng->special_exits, index, true);
			send_to_char("Special exit removed.\n\r", ch);
			return true;
		}

		send_to_char("Syntax:  special exit {Rlist{x\n\r", ch);
		send_to_char("         special exit {Radd{x static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Radd{x source generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Radd{x destination generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Radd{x weighted\n\r", ch);
		send_to_char("         special exit {Radd{x group\n\r", ch);
		send_to_char("         special exit {Rfrom{x # list\n\r", ch);
		send_to_char("         special exit {Rfrom{x # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Rfrom{x # set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Rfrom{x # remove #\n\r", ch);
		send_to_char("         special exit {Rto{x # list\n\r", ch);
		send_to_char("         special exit {Rto{x # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rto{x # set[ #] <weight> <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rto{x # remove #\n\r", ch);
		send_to_char("         special exit {Rgroup{x # add static generated|ordinal <from-level> <from-exit> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # add source generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # add destination generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # add weighted\n\r", ch);
		send_to_char("         special exit {Rgroup{x # add group\n\r", ch);
		send_to_char("         special exit {Rgroup{x # from # list\n\r", ch);
		send_to_char("         special exit {Rgroup{x # from # add <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # from # set[ #] <weight> generated|ordinal <from-level> <from-exit>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # from # remove #\n\r", ch);
		send_to_char("         special exit {Rgroup{x # to # list\n\r", ch);
		send_to_char("         special exit {Rgroup{x # to # add <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # to # set[ #] <weight> generated|ordinal <to-level> <to-entrance>\n\r", ch);
		send_to_char("         special exit {Rgroup{x # to # remove #\n\r", ch);
		send_to_char("         special exit {Rgroup{x # remove #\n\r", ch);
		send_to_char("         special exit {Rremove{x #\n\r", ch);
		return false;
	}

	dngedit_special(ch, "");
	return false;
}

DNGEDIT (dngedit_adddprog)
{
	struct trigger_type *tt;
    int slot;
	DUNGEON_INDEX_DATA *dungeon;
    PROG_LIST *list;
    SCRIPT_DATA *code;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_DUNGEON(ch, dungeon);
    argument = one_argument(argument, num);
    argument = one_argument(argument, trigger);
    argument = one_argument(argument, phrase);

	WNUM wnum;

    if (!parse_widevnum(num, ch->in_room->area, &wnum) || trigger[0] =='\0' || phrase[0] =='\0')
    {
		send_to_char("Syntax:   adddprog [wnum] [trigger] [phrase]\n\r",ch);
		return false;
    }

    if (!(tt = get_trigger_type(trigger, PRG_DPROG))) {
	send_to_char("Valid flags are:\n\r",ch);
	show_help(ch, "dprog");
	return false;
    }

    slot = tt->slot;
	if(!wnum.pArea) wnum.pArea = dungeon->area;

    if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_DPROG)) == NULL)
    {
	send_to_char("No such DUNGEONProgram.\n\r",ch);
	return false;
    }

    // Make sure this has a list of progs!
    if(!dungeon->progs) dungeon->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);
    list->script          = code;

    list_appendlink(dungeon->progs[slot], list);
	trigger_type_add_use(tt);

    send_to_char("Dprog Added.\n\r",ch);
    return true;
}

DNGEDIT (dngedit_deldprog)
{
    DUNGEON_INDEX_DATA *dungeon;
    char dprog[MAX_STRING_LENGTH];
    int value;

    EDIT_DUNGEON(ch, dungeon);

    one_argument(argument, dprog);
    if (!is_number(dprog) || dprog[0] == '\0')
    {
       send_to_char("Syntax:  deldprog [#dprog]\n\r",ch);
       return false;
    }

    value = atol (dprog);

    if (value < 0)
    {
        send_to_char("Only non-negative dprog-numbers allowed.\n\r",ch);
        return false;
    }

    if(!edit_deltrigger(dungeon->progs,value)) {
	send_to_char("No such dprog.\n\r",ch);
	return false;
    }

    send_to_char("Dprog removed.\n\r", ch);
    return true;
}

DNGEDIT(dngedit_varset)
{
    DUNGEON_INDEX_DATA *dungeon;

	EDIT_DUNGEON(ch, dungeon);

	return olc_varset(&dungeon->index_vars, ch, argument, false);
}

DNGEDIT(dngedit_varclear)
{
    DUNGEON_INDEX_DATA *dungeon;

	EDIT_DUNGEON(ch, dungeon);

	return olc_varclear(&dungeon->index_vars, ch, argument, false);
}