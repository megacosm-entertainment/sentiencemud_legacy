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

#include "bpedit.h"


BSEDIT( bsedit_list )
{
	list_blueprint_sections(ch, argument);
	return false;
}

BSEDIT( bsedit_show )
{
	BLUEPRINT_SECTION *bs;
	BUFFER *buffer;
	char buf[MSL];

	EDIT_BPSECT(ch, bs);

	buffer = new_buf();

	sprintf(buf, "Name:        [%5ld] %s\n\r", bs->vnum, bs->name);
	add_buf(buffer, buf);

	sprintf(buf, "Type:        %s\n\r", flag_string(blueprint_section_types, bs->type));
	add_buf(buffer, buf);

	sprintf(buf, "Flags:       %s\n\r", flag_string(blueprint_section_flags, bs->flags));
	add_buf(buffer, buf);

	if (bs->type == BSTYPE_STATIC)
	{
		if( bs->recall > 0 )
		{
			ROOM_INDEX_DATA *recall_room = get_room_index(bs->area, bs->recall);
			sprintf(buf, "Recall:      [%5ld] %s\n\r", bs->recall, recall_room ? recall_room->name : "-invalid-");
		}
		else
		{
			sprintf(buf, "Recall:      no recall defined\n\r");
		}
		add_buf(buffer, buf);

		sprintf(buf, "Lower Vnum:  %ld\n\r", bs->lower_vnum);
		add_buf(buffer, buf);

		sprintf(buf, "Upper Vnum:  %ld\n\r", bs->upper_vnum);
		add_buf(buffer, buf);
	}
	else if (bs->type == BSTYPE_MAZE)
	{
		sprintf(buf, "Maze Width:  %ld\n\r", bs->maze_x);
		add_buf(buffer, buf);
		sprintf(buf, "Maze Height: %ld\n\r", bs->maze_y);
		add_buf(buffer, buf);

		if (bs->maze_x > 0 && bs->maze_y > 0 && bs->recall > 0)
		{
			int y = (bs->recall - 1) / bs->maze_x + 1;
			int x = (bs->recall - 1) % bs->maze_x + 1;

			if (y >= 1 && y <= bs->maze_y)
				sprintf(buf, "Maze Recall: (%d, %d)\n\r", x, y);
			else
				sprintf(buf, "Maze Recall: {ROut-of-Bounds{x\n\r");
		}
		else
			sprintf(buf, "Maze Recall: {DInvalid{x\n\r");
		add_buf(buffer, buf);

		add_buf(buffer, "Maze Fixed Rooms:\n\r");
		if (list_size(bs->maze_fixed_rooms) > 0)
		{
			add_buf(buffer, "     [  X  ] [  Y  ] [ ] [               Room               ]\n\r");
			add_buf(buffer, "==============================================================\n\r");

			int room_no = 1;
			ITERATOR it;
			MAZE_FIXED_ROOM *mfr;
			iterator_start(&it, bs->maze_fixed_rooms);
			while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
			{
				ROOM_INDEX_DATA *room = get_room_index(bs->area, mfr->vnum);
				sprintf(buf, "%4d  %5d   %5d   %s    %5ld %s{x\n\r", room_no++,
					mfr->x, mfr->y,
					mfr->connected?"{WY{x":"{DN{x",
					mfr->vnum,
					(room ? room->name : "{DInvalid{x"));
				add_buf(buffer, buf);
			}
			iterator_stop(&it);

			add_buf(buffer, "--------------------------------------------------------------\n\r");
		}
		else
			add_buf(buffer, "    None\n\r");

		add_buf(buffer, "Maze Room Templates:\n\r");
		if (list_size(bs->maze_templates) > 0)
		{
			add_buf(buffer, "     [ Weight ] [               Room               ]\n\r");
			add_buf(buffer, "=====================================================\n\r");

			int room_no = 1;
			ITERATOR it;
			MAZE_WEIGHTED_ROOM *mwr;
			iterator_start(&it, bs->maze_templates);
			while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&it)))
			{
				ROOM_INDEX_DATA *room = get_room_index(bs->area, mwr->vnum);
				sprintf(buf, "%4d   %6d     %5ld %s\n\r", room_no++,
					mwr->weight,
					mwr->vnum,
					(room ? room->name : "{DInvalid{x"));
				add_buf(buffer, buf);
			}
			iterator_stop(&it);

			add_buf(buffer, "-----------------------------------------------------\n\r");
		}
		else
			add_buf(buffer, "    None\n\r");
	}
	else
	{
		add_buf(buffer, "{WWARNING: {RSection is ill-defined.{x\n\r");
	}

	add_buf(buffer, "Description:\n\r");
	add_buf(buffer, bs->description);
	add_buf(buffer, "\n\r");

	add_buf(buffer, "\n\r-----\n\r{WBuilders' Comments:{X\n\r");
	add_buf(buffer, bs->comments);
	add_buf(buffer, "\n\r-----\n\r");

	if( bs->links )
	{
		int bli = 0;
		// List links
		add_buf(buffer, "{YSection Links:{x\n\r");
		for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
		{
			++bli;
			ROOM_INDEX_DATA *room = bl->room;

			char *door = (bl->door >= 0 && bl->door < MAX_DIR) ? dir_name[bl->door] : "none";
			char excolor = bl->ex ? 'W' : 'D';

			sprintf(buf, " {Y[{W%3d{Y] {G%-30.30s {%c%-9s{x in {Y[{W%5ld{Y]{x %s\n\r", bli, bl->name, excolor, door, bl->vnum, room ? room->name : "nowhere");
			add_buf(buffer, buf);
		}
	}

	page_to_char(buffer->string, ch);

	free_buf(buffer);
	return false;
}

BSEDIT( bsedit_create )
{
	AREA_DATA *pArea = ch->in_room->area;
	BLUEPRINT_SECTION *bs;
	WNUM wnum;
	int  iHash;

	if (argument[0] == '\0' || !parse_widevnum(argument, pArea, &wnum) || !wnum.pArea || wnum.vnum < 1)
	{
		long last_vnum = 0;
		long value = pArea->top_blueprint_section_vnum + 1;
		for(last_vnum = 1; last_vnum <= pArea->top_blueprint_section_vnum; last_vnum++)
		{
			if( !get_blueprint_section(pArea, last_vnum) )
			{
				value = last_vnum;
				break;
			}
		}

		wnum.pArea = pArea;
		wnum.vnum = value;
	}

	if( get_blueprint_section(wnum.pArea, wnum.vnum) )
	{
		send_to_char("That vnum already exists.\n\r", ch);
		return false;
	}
	
    if (!IS_BUILDER(ch, wnum.pArea))
    {
		send_to_char("BsEdit:  widevnum in an area you cannot build in.\n\r", ch);
		return false;
    }

	bs = new_blueprint_section();
	bs->vnum = wnum.vnum;
	bs->area = wnum.pArea;

	iHash							= bs->vnum % MAX_KEY_HASH;
	bs->next						= wnum.pArea->blueprint_section_hash[iHash];
	wnum.pArea->blueprint_section_hash[iHash]	= bs;
	olc_set_editor(ch, ED_BPSECT, bs);

	wnum.pArea->bottom_blueprint_section_vnum = UMIN(wnum.pArea->bottom_blueprint_section_vnum, bs->vnum);
	wnum.pArea->top_blueprint_section_vnum = UMAX(wnum.pArea->top_blueprint_section_vnum, bs->vnum);

    return true;
}

BSEDIT( bsedit_name )
{
	BLUEPRINT_SECTION *bs;

	EDIT_BPSECT(ch, bs);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name [string]\n\r", ch);
		return false;
	}

	free_string(bs->name);
	bs->name = str_dup(argument);
	send_to_char("Name changed.\n\r", ch);
	return true;
}

BSEDIT( bsedit_description )
{
	BLUEPRINT_SECTION *bs;

	EDIT_BPSECT(ch, bs);

	if (argument[0] == '\0')
	{
		string_append(ch, &bs->description);
		return true;
	}

	send_to_char("Syntax:  description - line edit\n\r", ch);
	return false;
}

BSEDIT( bsedit_comments )
{
	BLUEPRINT_SECTION *bs;

	EDIT_BPSECT(ch, bs);

	if (argument[0] == '\0')
	{
		string_append(ch, &bs->comments);
		return true;
	}

	send_to_char("Syntax:  comments - line edit\n\r", ch);
	return false;
}

BSEDIT( bsedit_type )
{
	BLUEPRINT_SECTION *bs;
	int value;

	EDIT_BPSECT(ch, bs);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  type <type>\n\r", ch);
		send_to_char("'? section_types' for list of types.\n\r", ch);
		return false;
	}

	if( (value = flag_value(blueprint_section_types, argument)) == NO_FLAG )
	{
		send_to_char("That is not a valid type.\n\r", ch);
		send_to_char("'? section_types' for list of types.\n\r", ch);
		return false;
	}

	bs->type = value;
	// Reset parameters
	bs->maze_x = 0;
	bs->maze_y = 0;
	bs->total_maze_weight = 0;
	list_clear(bs->maze_templates);
	bs->lower_vnum = 0;
	bs->upper_vnum = 0;
	bs->recall = 0;
	send_to_char("Section type changed.\n\r", ch);
	return true;
}

BSEDIT( bsedit_flags )
{
	BLUEPRINT_SECTION *bs;
	int value;

	EDIT_BPSECT(ch, bs);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  flags <flags>\n\r", ch);
		send_to_char("'? section_flags' for list of flags.\n\r", ch);
		return false;
	}

	if( (value = flag_value(blueprint_section_flags, argument)) == NO_FLAG )
	{
		send_to_char("That is not a valid flag.\n\r", ch);
		send_to_char("'? section_flags' for list of flags.\n\r", ch);
		return false;
	}

	bs->flags ^= value;
	send_to_char("Section flags changed.\n\r", ch);
	return true;
}


BSEDIT( bsedit_recall )
{
	BLUEPRINT_SECTION *bs;
	ROOM_INDEX_DATA *room;
	long vnum;
	char buf[MSL];

	EDIT_BPSECT(ch, bs);

	if (bs->type != BSTYPE_STATIC)
	{
		send_to_char("You may only set the static recall on a static section.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  recall [vnum]\n\r", ch);
		send_to_char("         recall none\n\r", ch);
		return false;
	}

	if( !str_cmp(argument, "none") )
	{
		if( bs->recall < 1 )
		{
			send_to_char("Recall was not defined.\n\r", ch);
			return false;
		}

		bs->recall = 0;

		send_to_char("Recall cleared.\n\r", ch);
		return true;
	}

	if( bs->lower_vnum < 1 || bs->upper_vnum < 1 )
	{
		send_to_char("Vnum range must be set first.\n\r", ch);
		return false;
	}

	if (!is_number(argument))
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	vnum = atol(argument);
	if( vnum <= 0 )
	{
		send_to_char("That room does not exist.\n\r", ch);
		return false;
	}

	if( vnum < bs->lower_vnum || vnum > bs->upper_vnum )
	{
		sprintf(buf, "Value must be a number from %ld to %ld.\n\r", bs->lower_vnum, bs->upper_vnum);
		send_to_char(buf, ch);
		return false;
	}

	room = get_room_index(bs->area, vnum);
	if( room == NULL )
	{
		send_to_char("That room does not exist.\n\r", ch);
		return false;
	}

	bs->recall = vnum;
	sprintf(buf, "Recall set to %.30s (%ld)\n\r", room->name, vnum);
	send_to_char(buf, ch);
	return true;
}

BSEDIT( bsedit_rooms )
{
	BLUEPRINT_SECTION *bs;
	long lvnum, uvnum;
	char buf[MSL];
	char arg[MIL];

	EDIT_BPSECT(ch, bs);

	if (bs->type != BSTYPE_STATIC)
	{
		send_to_char("Blueprint section must be a STATIC type.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char("Syntax:  rooms [lower vnum] [upper vnum]\n\r", ch);
		send_to_char("{YVnums must be in the same area.{x\n\r", ch);
		return false;
	}

	if( !is_number(arg) || !is_number(argument) )
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	lvnum = atol(arg);
	uvnum = atol(argument);

	// Silently swap the bounds if necessary, don't be annoying
	if( uvnum < lvnum )
	{
		long vnum = lvnum;
		lvnum = uvnum;
		uvnum = vnum;
	}

	if( validate_vnum_range(ch, bs, lvnum, uvnum) )
	{
		bs->lower_vnum = lvnum;
		bs->upper_vnum = uvnum;

		send_to_char("Vnum range set.\n\r", ch);

		// Make sure recall point is still inside room range
		if( bs->recall > 0 )
		{
			if( bs->recall < lvnum || bs->recall > uvnum )
			{
				send_to_char("{YRecall room outside of new range.  Clearing.{x\n\r", ch);
				bs->recall = 0;
			}
		}

		// Make sure link are still inside room range
		if( bs->links )
		{
			BLUEPRINT_LINK *prev = NULL, *cur, *next;

			for(cur = bs->links; cur; cur = next)
			{
				next = cur->next;

				if( cur->vnum < lvnum || cur->vnum > uvnum )
				{
					sprintf(buf, "Link %.30s outside of new vnum range.  Removing.\n\r", cur->name);
					send_to_char(buf, ch);

					if( prev )
						prev->next = next;
					else
						bs->links = next;

					free_blueprint_link(cur);
				}
				else
				{
					prev = cur;
				}
			}

		}

		return true;
	}

	return false;
}

BSEDIT( bsedit_maze )
{
	BLUEPRINT_SECTION *bs;
	char buf[MSL];
	char arg[MIL];

	EDIT_BPSECT(ch, bs);

	if (bs->type != BSTYPE_MAZE)
	{
		send_to_char("Blueprint section must be a MAZE type.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "size"))
	{
		char argw[MIL];
		char argh[MIL];

		argument = one_argument(argument, argw);
		argument = one_argument(argument, argh);

		if (!is_number(argw) || !is_number(argh))
		{
			send_to_char("Syntax:  maze size <width> <height>\n\r", ch);
			send_to_char("Please specify a number.\n\r", ch);
			return false;
		}

		long width = atol(argw);
		long height = atol(argh);

		if (width < 1 || height < 1)
		{
			send_to_char("Syntax:  maze size <width> <height>\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}

		bs->maze_x = width;
		bs->maze_y = height;
		bs->recall = 0;
		list_clear(bs->maze_fixed_rooms);

		send_to_char("Maze dimensions set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "templates"))
	{
		char arg2[MIL];

		argument = one_argument(argument, arg2);

		if (!str_prefix(arg2, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "Maze Room Templates:\n\r");
			if (list_size(bs->maze_templates) > 0)
			{
				add_buf(buffer, "     [ Weight ] [               Room               ]\n\r");
				add_buf(buffer, "=====================================================\n\r");

				int room_no = 1;
				ITERATOR it;
				MAZE_WEIGHTED_ROOM *mwr;
				iterator_start(&it, bs->maze_templates);
				while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&it)))
				{
					ROOM_INDEX_DATA *room = get_room_index(bs->area, mwr->vnum);
					sprintf(buf, "%4d   %6d     %5ld %s\n\r", room_no++,
						mwr->weight,
						mwr->vnum,
						(room ? room->name : "{DInvalid{x"));
					add_buf(buffer, buf);
				}
				iterator_stop(&it);

				add_buf(buffer, "-----------------------------------------------------\n\r");
			}
			else
				add_buf(buffer, "    None\n\r");

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
		else if (!str_prefix(arg2, "add"))
		{
			char argw[MIL];

			argument = one_argument(argument, argw);

			if (!is_number(argw))
			{
				send_to_char("Syntax:  maze templates add {R<weight>{x <room vnum>\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			int weight = atoi(argw);
			if (weight < 1)
			{
				send_to_char("Syntax:  maze templates add {R<weight>{x <room vnum>\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  maze templates add <weight> {R<room vnum>{x\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			long vnum = atoi(argument);
			if (vnum < 1)
			{
				send_to_char("Syntax:  maze templates add <weight> {R<room vnum>{x\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			ROOM_INDEX_DATA *room = get_room_index(bs->area, vnum);
			if (!room)
			{
				send_to_char("Syntax:  maze templates add <weight> {R<room vnum>{x\n\r", ch);
				sprintf(buf, "Room does not exist at vnum %ld.\n\r", vnum);
				send_to_char(buf, ch);
				return false;
			}

			for(int i = 0; i < MAX_DIR; i++)
			{
				if (room->exit[i] != NULL)
				{
					send_to_char("Syntax:  maze templates add <weight> {R<room vnum>{x\n\r", ch);
					send_to_char("Only unlinked rooms may be used as a maze room template.\n\r", ch);
					return false;
				}
			}

			SET_BIT(room->room_flag[1], ROOM_BLUEPRINT);
			MAZE_WEIGHTED_ROOM *mwr = new_maze_weighted_room();
			mwr->weight = weight;
			mwr->vnum = vnum;
			list_appendlink(bs->maze_templates, mwr);

			sprintf(buf, "Room %s (%ld) added with weight %d.\n\r", room->name, vnum, weight);
			send_to_char(buf, ch);
			return true;
		}
		else if (!str_prefix(arg2, "remove"))
		{
			if (list_size(bs->maze_templates) < 1)
			{
				send_to_char("There are no maze room templates to remove.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  maze templates remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bs->maze_templates));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bs->maze_templates))
			{
				send_to_char("Syntax:  maze templates remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bs->maze_templates));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bs->maze_templates, index, true);
			sprintf(buf, "Maze room template %d removed.", index);
			send_to_char(buf, ch);
			return true;
		}

		send_to_char("Syntax:  maze templates {Rlist{x\n\r", ch);
		send_to_char("         maze templates {Radd{x <weight> <room vnum>\n\r", ch);
		send_to_char("         maze templates {Rremove{x #\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "fixed"))
	{
		char arg2[MIL];

		argument = one_argument(argument, arg2);
		if (!str_prefix(arg2, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "Maze Fixed Rooms:\n\r");
			if (list_size(bs->maze_fixed_rooms) > 0)
			{
				add_buf(buffer, "     [  X  ] [  Y  ] [ ] [               Room               ]\n\r");
				add_buf(buffer, "==============================================================\n\r");

				int room_no = 1;
				ITERATOR it;
				MAZE_FIXED_ROOM *mfr;
				iterator_start(&it, bs->maze_fixed_rooms);
				while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
				{
					ROOM_INDEX_DATA *room = get_room_index(bs->area, mfr->vnum);
					sprintf(buf, "%4d  %5d   %5d   %s    %5ld %s{x\n\r", room_no++,
						mfr->x, mfr->y,
						mfr->connected?"{WY{x":"{DN{x",
						mfr->vnum,
						(room ? room->name : "{DInvalid{x"));
					add_buf(buffer, buf);
				}
				iterator_stop(&it);

				add_buf(buffer, "--------------------------------------------------------------\n\r");
			}
			else
				add_buf(buffer, "    None\n\r");

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
		
		if(!str_prefix(arg2, "add"))
		{
			char argx[MIL];
			char argy[MIL];
			char argr[MIL];
			long x, y, vnum;
			bool connected;

			argument = one_argument(argument, argx);
			argument = one_argument(argument, argy);
			argument = one_argument(argument, argr);

			if (!is_number(argx) || (x = atol(argx)) < 1 || x > bs->maze_x)
			{
				send_to_char("Syntax:  maze fixed add {R<x>{x <y> <room vnum>[ <connected>]\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %ld.\n\r", bs->maze_x);
				send_to_char(buf, ch);
				return false;
			}

			if (!is_number(argy) || (y = atol(argy)) < 1 || y > bs->maze_y)
			{
				send_to_char("Syntax:  maze fixed add <x> {R<y>{x <room vnum>[ <connected>]\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %ld.\n\r", bs->maze_y);
				send_to_char(buf, ch);
				return false;
			}

			if (!is_number(argr) || (vnum = atol(argr)) < 1)
			{
				send_to_char("Syntax:  maze fixed add <x> <y> {R<room vnum>{[ <connected>]\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			ROOM_INDEX_DATA *room = get_room_index(bs->area, vnum);
			if (!room)
			{
				send_to_char("Syntax:  maze fixed add <x> <y> {R<room vnum>{x[ <connected>]\n\r", ch);
				send_to_char("Room does not exist.\n\r", ch);
				return false;
			}

			// Verify exits are only ENVIRONMENT and only around the edges for lateral directions
			for(int i = 0; i < MAX_DIR; i++)
			{
				if (!__verify_exit(room, i, x, y, bs->maze_x, bs->maze_y))
				{
					send_to_char("Syntax:  maze fixed add <x> <y> {R<room vnum>{x[ <connected>]\n\r", ch);
					send_to_char("Room contains either an ENVIRONMENT exit going inward or an ordinary exit leaving maze bounds.\n\r", ch);
					send_to_char("UP/DOWN exits are allowed anywhere in the maze section.\n\r", ch);
					send_to_char("Lateral exits are only allowed on their respective edges,\n\r",ch);
					send_to_char("    such as NORTH* exits on the north edge.\n\r",ch);
					return false;
				}
			}

			if (argument[0] == '\0')
				connected = true;
			else if (!str_prefix(argument, "yes"))
				connected = true;
			else if (!str_prefix(argument, "no"))
				connected = false;
			else
				connected = false;

			MAZE_FIXED_ROOM *mfr = new_maze_fixed_room();
			mfr->x = x;
			mfr->y = y;
			mfr->vnum = vnum;
			mfr->connected = connected;
			list_appendlink(bs->maze_fixed_rooms, mfr);

			sprintf(buf, "Fixed room %s (%ld) added to maze section at location (%ld %ld).\n\r", room->name, vnum, x, y);
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg2, "remove"))
		{
			if (list_size(bs->maze_fixed_rooms) < 1)
			{
				send_to_char("There are no fixed maze rooms to remove.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  maze fixed remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bs->maze_fixed_rooms));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bs->maze_fixed_rooms))
			{
				send_to_char("Syntax:  maze fixed remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bs->maze_fixed_rooms));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bs->maze_fixed_rooms, index, true);
			sprintf(buf, "Fixed maze room %d removed.\n\r", index);
			send_to_char(buf, ch);
			return true;
		}

		send_to_char("Syntax:  maze fixed {Rlist{x\n\r", ch);	
		send_to_char("         maze fixed {Radd{x <x> <y> <room vnum>\n\r", ch);
		send_to_char("         maze fixed {Rremove{x #\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "recall"))
	{
		char arg2[MIL];

		argument = one_argument(argument, arg2);
		if (!str_prefix(arg2, "clear"))
		{
			bs->recall = 0;
			send_to_char("Section recall cleared.\n\r", ch);
			return true;
		}

		if (!is_number(arg2) || !is_number(argument))
		{
			send_to_char("Syntax:  maze recall clear\n\r", ch);
			send_to_char("         maze recall <x> <y>\n\r", ch);
			return false;
		}

		long x = atol(arg2);
		long y = atol(argument);

		if (x < 1 || x > bs->maze_x || y < 1 || y > bs->maze_y)
		{
			send_to_char("Syntax:  maze recall <x> <y>\n\r", ch);
			sprintf(buf, "Please specify coordinate from {Y1 1{x to {Y%ld %ld{x.\n\r", bs->maze_x, bs->maze_y);
			send_to_char(buf, ch);
			return false;
		}

		bs->recall = (y - 1) * bs->maze_x + x;
		sprintf(buf, "Section recall set to (%ld %ld).\n\r", x, y);
		send_to_char(buf, ch);
		return true;
	}

	send_to_char("Syntax:  maze {Rsize{x <width> <height>\n\r", ch);
	send_to_char("         maze {Rfixed{x list\n\r", ch);	
	send_to_char("         maze {Rfixed{x add <x> <y> <room vnum>\n\r", ch);
	send_to_char("         maze {Rfixed{x remove #\n\r", ch);
	send_to_char("         maze {Rtemplates{x list\n\r", ch);
	send_to_char("         maze {Rtemplates{x add <weight> <room vnum>\n\r", ch);
	send_to_char("         maze {Rtemplates{x remove #\n\r", ch);
	send_to_char("         maze {Rrecall{x clear\n\r", ch);
	send_to_char("         maze {Rrecall{x <x> <y>\n\r", ch);
	return false;
}

BSEDIT( bsedit_link )
{
	BLUEPRINT_SECTION *bs;
	BLUEPRINT_LINK *link;
	char arg[MIL];
	char arg2[MIL];

	EDIT_BPSECT(ch, bs);

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  link list\n\r", ch);
		send_to_char("         link add <vnum> <door>\n\r", ch);
		send_to_char("         link # delete\n\r", ch);
		send_to_char("         link # name <name>\n\r", ch);
		send_to_char("         link # room <vnum>\n\r", ch);
		send_to_char("         link # exit <door>\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if( !str_cmp(arg, "list" ) )
	{
		if( bs->links )
		{
			char buf[MSL];

			int bli = 0;
			// List links
			send_to_char("{YSection Links:{x\n\r", ch);
			for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
			{
				++bli;
				ROOM_INDEX_DATA *room = bl->room;

				char *door = (bl->door >= 0 && bl->door < MAX_DIR) ? dir_name[bl->door] : "none";
				char excolor = bl->ex ? 'W' : 'D';

				sprintf(buf, " {Y[{W%3d{Y] {G%-30.30s {%c%-9s{x in {Y[{W%5ld{Y]{x %s\n\r", bli, bl->name, excolor, door, bl->vnum, room ? room->name : "nowhere");
				send_to_char(buf, ch);
			}
		}
		else
		{
			send_to_char("No links defined.\n\r", ch);
		}

		return false;
	}

	if( !str_cmp(arg, "add") )
	{
		if (bs->type == BSTYPE_STATIC)
		{
			if( bs->lower_vnum < 1 || bs->upper_vnum < 1 )
			{
				send_to_char("Vnum range must be set first.\n\r", ch);
				return false;
			}
		}
		else if (bs->type == BSTYPE_MAZE)
		{
			if( bs->maze_x < 1 || bs->maze_y < 1)
			{
				send_to_char("Maze dimensions must be set first.\n\r", ch);
				return false;
			}
		}

		if( !is_number(arg2) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		long vnum = atol(arg2);
		if (bs->type == BSTYPE_STATIC)
		{
			if( vnum < bs->lower_vnum || vnum > bs->upper_vnum )
			{
				send_to_char("Vnum is out of range of blueprint section.\n\r", ch);
				return false;
			}
		}
		else if(bs->type == BSTYPE_MAZE)
		{
			ITERATOR fit;
			MAZE_FIXED_ROOM *mfr;
			iterator_start(&fit, bs->maze_fixed_rooms);
			while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&fit)))
			{
				if (mfr->vnum == vnum)
					break;
			}
			iterator_stop(&fit);

			if (!mfr)
			{
				send_to_char("Blueprint section does not have a maze fixed room with that vnum.\n\r", ch);
				return false;
			}
		}
		else
		{
			send_to_char("Blueprint section does not have a type defined.\n\r", ch);
			return false;
		}

		ROOM_INDEX_DATA *room = get_room_index(bs->area, vnum);
		if( !room )
		{
			send_to_char("That room does not exist.\n\r", ch);
			return false;
		}

		bool found = false;
		for( int i = 0; i < MAX_DIR; i++ )
		{
			if( room->exit[i] )
				found = true;
		}

		if( !found )
		{
			send_to_char("That room has no exits.\n\r,", ch);
			return false;
		}

		int door = parse_door(argument);
		if( door < 0 )
		{
			send_to_char("That is an invalid exit.\n\r", ch);
			return false;
		}

		EXIT_DATA *ex = room->exit[door];
		if( !ex )
		{
			send_to_char("That is an invalid exit.\n\r", ch);
			return false;
		}

		if( !IS_SET(ex->exit_info, EX_ENVIRONMENT) )
		{
			send_to_char("Exit links must be {YENVIRONMENT{x exits.\n\r", ch);
			return false;
		}

		link = new_blueprint_link();
		link->vnum = vnum;
		link->door = door;
		link->room = room;
		link->ex = ex;

		// Append to the end
		link->next = NULL;
		if( bs->links )
		{
			BLUEPRINT_LINK *cur;

			for(cur = bs->links;cur->next; cur = cur->next)
			{
				;
			}

			cur->next = link;
		}
		else
		{
			bs->links = link;
		}

		send_to_char("Link added.\n\r", ch);
		return true;
	}

	if( !is_number(arg) )
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	int linkno = atoi(arg);
	if( linkno < 1 )
	{
		send_to_char("That is not a section link.\n\r", ch);
		return false;
	}


	if( !str_cmp(arg2, "delete") )
	{
		BLUEPRINT_LINK *prev = NULL;

		for( link = bs->links; link; link = link->next )
		{
			if(!--linkno)
				break;

			prev = link;
		}

		if(!link)
		{
			send_to_char("That is not a section link.\n\r", ch);
			return false;
		}

		if( prev )
			prev->next = link->next;
		else
			bs->links = link->next;

		free_blueprint_link(link);
		send_to_char("Link removed.\n\r", ch);
		return true;
	}

	link = get_section_link(bs, linkno);
	if( !link )
	{
		send_to_char("That is not a section link.\n\r", ch);
		return false;
	}

	if( !str_cmp(arg2, "name") )
	{
		if( argument[0] == '\0' )
		{
			send_to_char("Syntax:  link # name <name>\n\r", ch);
			return false;
		}

		free_string(link->name);
		link->name = str_dup(argument);

		send_to_char("Name changed.\n\r", ch);
		return true;
	}

	if( !str_cmp(arg2, "room") )
	{
		if( argument[0] == '\0' )
		{
			send_to_char("Syntax:  link # room <vnum>\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		long vnum = atol(argument);
		if (bs->type == BSTYPE_STATIC)
		{
			if( vnum < bs->lower_vnum || vnum > bs->upper_vnum )
			{
				send_to_char("Vnum is out of range of blueprint section.\n\r", ch);
				return false;
			}
		}
		else if(bs->type == BSTYPE_MAZE)
		{
			ITERATOR fit;
			MAZE_FIXED_ROOM *mfr;
			iterator_start(&fit, bs->maze_fixed_rooms);
			while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&fit)))
			{
				if (mfr->vnum == vnum)
					break;
			}
			iterator_stop(&fit);

			if (!mfr)
			{
				send_to_char("Blueprint section does not have a maze fixed room with that vnum.\n\r", ch);
				return false;
			}
		}
		else
		{
			send_to_char("Blueprint section does not have a type defined.\n\r", ch);
			return false;
		}

		ROOM_INDEX_DATA *room = get_room_index(bs->area, vnum);
		if( !room )
		{
			send_to_char("That room does not exist.\n\r", ch);
			return false;
		}

		bool found = false;
		for( int i = 0; i < MAX_DIR; i++ )
		{
			if( room->exit[i] )
				found = true;
		}

		if( !found )
		{
			send_to_char("That room has no exits.\n\r,", ch);
			return false;
		}

		link->vnum = vnum;
		link->door = -1;
		link->room = room;
		link->ex = NULL;

		send_to_char("Room changed.\n\r", ch);
		return true;
	}

	if( !str_cmp(arg2, "exit") )
	{
		if( argument[0] == '\0' )
		{
			send_to_char("Syntax:  link # exit <door>\n\r", ch);
			return false;
		}

		int door = parse_door(argument);
		if( door < 0 )
		{
			send_to_char("That is an invalid exit.\n\r", ch);
			return false;
		}

		EXIT_DATA *ex = link->room->exit[door];
		if( !ex )
		{
			send_to_char("That is an invalid exit.\n\r", ch);
			return false;
		}

		if( !IS_SET(ex->exit_info, EX_ENVIRONMENT) )
		{
			send_to_char("Exit links must be {YENVIRONMENT{x exits.\n\r", ch);
			return false;
		}

		link->door = door;
		link->ex = ex;
		send_to_char("Exit changed.\n\r", ch);
		return true;
	}

	bsedit_link(ch, "");
	return false;
}