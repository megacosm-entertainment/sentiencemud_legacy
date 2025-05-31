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

/* BPEditor Functions */
BPEDIT( bpedit_list )
{
	list_blueprints(ch, argument);
	return false;
}

BPEDIT( bpedit_show )
{
	BLUEPRINT *bp;
	BUFFER *buffer;
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	buffer = new_buf();

	sprintf(buf, "{xName:        [%5ld] %s{x\n\r", bp->vnum, bp->name);
	add_buf(buffer, buf);

	if( bp->repop > 0)
		sprintf(buf, "Repop:       %d minutes\n\r", bp->repop);
	else
		sprintf(buf, "Repop:       {Dnever{x\n\r");
	add_buf(buffer, buf);

	sprintf(buf, "{xAreaWho:     [%s] [%s]{x\n\r", flag_string(area_who_titles, bp->area_who), flag_string(area_who_display, bp->area_who));
	add_buf(buffer, buf);

	sprintf(buf, "{xFlags:       %s{x\n\r", flag_string(instance_flags, bp->flags));
	add_buf(buffer, buf);

	add_buf(buffer, "Description:\n\r");
	add_buf(buffer, bp->description);
	add_buf(buffer, "\n\r");

	add_buf(buffer, "\n\r-----\n\r{WBuilders' Comments:{X\n\r");
	add_buf(buffer, bp->comments);
	add_buf(buffer, "\n\r-----\n\r");

	if( list_size(bp->sections) > 0 )
	{
		int line = 0;
		BLUEPRINT_SECTION * bs;
		ITERATOR sit;

		add_buf(buffer, "{YSections:{x\n\r");
		add_buf(buffer, "     [  Vnum  ] [             Name             ]\n\r");
		add_buf(buffer, "------------------------------------------------\n\r");

		iterator_start(&sit, bp->sections);
		while( (bs = (BLUEPRINT_SECTION *)iterator_nextdata(&sit)) )
		{
			sprintf(buf, "{W%4d  {G%8ld{x   %-30.30s{x\n\r", ++line, bs->vnum, bs->name);
			add_buf(buffer, buf);
		}

		iterator_stop(&sit);
		add_buf(buffer, "------------------------------------------------\n\r\n\r");
	}
	else
	{
		add_buf(buffer, "{YSections:{x\n\r   None\n\r\n\r");
	}

	bpedit_buffer_layout(buffer, bp);
	
	bpedit_buffer_links(buffer, bp);

	add_buf(buffer, "Special Rooms:\n\r");
	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		add_buf(buffer, "   {WSCRIPTED{x\n\r");
	}
	else if (list_size(bp->special_rooms) > 0)
	{
		BLUEPRINT_SPECIAL_ROOM *special;

		char buf[MSL];
		int line = 0;

		ITERATOR sit;

		add_buf(buffer, "     [             Name             ] [             Room             ]\n\r");
		add_buf(buffer, "---------------------------------------------------------------------------------\n\r");

		bool approx_msg = false;
		iterator_start(&sit, bp->special_rooms);
		while( (special = (BLUEPRINT_SPECIAL_ROOM *)iterator_nextdata(&sit)) )
		{
			bool exact = false;
			BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, special->section, &exact);

			ROOM_INDEX_DATA *room = NULL;

			if(IS_VALID(bs) )
			{
				room = blueprint_section_get_room_byoffset(bs, special->offset);
			}

			if( !IS_VALID(bs) || !room)
			{
				snprintf(buf, MSL-1, "{W%4d  %-30.30s   {D-{Winvalid{D-{x\n\r", ++line, special->name);
			}
			else
			{
				snprintf(buf, MSL-1, "{W%4d  %-30.30s   (%ld#%ld) {Y%s{x in (%ld#%ld) {Y%s{x%s\n\r", ++line, special->name, room->area->uid, room->vnum, room->name, bs->area->uid, bs->vnum, bs->name, exact?"":" {M**{x");
				if (!exact) approx_msg = true;
			}
			add_buf(buffer, buf);
		}
		iterator_stop(&sit);
		add_buf(buffer, "---------------------------------------------------------------------------------\n\r");

		if (approx_msg)
			add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
	}
	else
	{
		add_buf(buffer, "   None\n\r");
	}
	add_buf(buffer, "\n\r");

	if( IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		add_buf(buffer, "{xRecall:     {WScripted{x\n\r");
	}
	else if(bp->recall)
	{
		bool exact = false;
		BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, bp->recall, &exact);

		// Get the recall room from section
		ROOM_INDEX_DATA *room = IS_VALID(bs) ? get_room_index(bs->area, bs->recall) : NULL;

		sprintf(buf, "{xRecall:     %s{x (#{W%ld{x) in %s{x ({W%d{x) [{Y%s{x]\n\r",
			(room ? room->name : "???"),
			(room ? room->vnum : 0),
			bs->name, abs(bp->recall), (bp->recall < 0 ? "ORDINAL" : "GENERATED"));
		add_buf(buffer, buf);

		if(room && !exact)
			add_buf(buffer, "            {M({WLocation is the most likely location due to section entry.{M}){x\n\r");
	}
	else
	{
		add_buf(buffer, "{xRecall:     none\n\r");
	}
	add_buf(buffer, "\n\r");

	add_buf(buffer, "{xEntrances:\n\r");
	if(list_size(bp->entrances) > 0)
	{
		add_buf(buffer, "      [         Name         ] [ Section ] [               Room               ]\n\r");
		add_buf(buffer, "================================================================================\n\r");
		ITERATOR bxit;
		BLUEPRINT_EXIT_DATA *bex;
		int bxindex = 1;
		bool approx_msg = false;
		iterator_start(&bxit, bp->entrances);
		while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
		{
			bool exact = false;
			BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, bex->section, &exact);

			BLUEPRINT_LINK *link = get_section_link(bs, bex->link);

			ROOM_INDEX_DATA *room = IS_VALID(bs) && valid_section_link(link) ? get_room_index(bs->area, link->vnum) : NULL;

			if (room)
				sprintf(buf, "%4d    %-20.20s     {%c%7d{x     (%-4ld) %s (%s) %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), room->vnum, room->name, dir_name[link->door], (exact ? "" : "{M**{x"));
			else
				sprintf(buf, "%4d    %-20.20s     {%c%7d{x     %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), "???");
			add_buf(buffer, buf);

			if (room && !exact)
				approx_msg = true;
		}
		iterator_stop(&bxit);
		add_buf(buffer, "--------------------------------------------------------------------------------\n\r");

		if (approx_msg)
			add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
	}
	else
	{
		add_buf(buffer, "    none\n\r");
	}
	add_buf(buffer, "\n\r");

	add_buf(buffer, "{xExits:\n\r");
	if(list_size(bp->exits) > 0)
	{
		add_buf(buffer, "      [         Name         ] [ Section ] [               Room               ]\n\r");
		add_buf(buffer, "================================================================================\n\r");
		ITERATOR bxit;
		BLUEPRINT_EXIT_DATA *bex;
		int bxindex = 1;
		bool approx_msg = false;
		iterator_start(&bxit, bp->exits);
		while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
		{
			bool exact = false;
			BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, bex->section, &exact);

			BLUEPRINT_LINK *link = get_section_link(bs, bex->link);

			ROOM_INDEX_DATA *room = IS_VALID(bs) && valid_section_link(link) ? get_room_index(bs->area, link->vnum) : NULL;

			if (room)
				sprintf(buf, "%4d    %-20.20s     {%c%7d{x     (%-4ld) %s (%s) %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), room->vnum, room->name, dir_name[link->door], (exact ? "" : "{M**{x"));
			else
				sprintf(buf, "%4d    %-20.20s     {%c%7d{x     %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), "???");
			add_buf(buffer, buf);

			if (room && !exact)
				approx_msg = true;
		}
		iterator_stop(&bxit);
		add_buf(buffer, "--------------------------------------------------------------------------------\n\r");

		if (approx_msg)
			add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
	}
	else
	{
		add_buf(buffer, "    none\n\r");
	}
	add_buf(buffer, "\n\r");

	
    if (bp->progs)
		olc_show_progs(buffer, bp->progs, PRG_IPROG, "InstProg Vnum");

	olc_show_index_vars(buffer, bp->index_vars);

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

BPEDIT( bpedit_create )
{
	AREA_DATA *area = ch->in_room->area;
	BLUEPRINT *bp;
	WNUM wnum;
	int  iHash;

	if (argument[0] == '\0' || !parse_widevnum(argument, area, &wnum) || !wnum.pArea || wnum.vnum < 1)
	{
		long last_vnum = 0;
		long value = area->top_blueprint_vnum + 1;
		for(last_vnum = 1; last_vnum <= area->top_blueprint_vnum; last_vnum++)
		{
			if( !get_blueprint(area, last_vnum) )
			{
				value = last_vnum;
				break;
			}
		}

		wnum.pArea = area;
		wnum.vnum = value;
	}

	if( get_blueprint(wnum.pArea, wnum.vnum) )
	{
		send_to_char("That vnum already exists.\n\r", ch);
		return false;
	}

    if (!IS_BUILDER(ch, wnum.pArea))
    {
		send_to_char("BpEdit:  widevnum in an area you cannot build in.\n\r", ch);
		return false;
    }

	bp = new_blueprint();
	bp->vnum = wnum.vnum;
	bp->area = wnum.pArea;

	iHash							= bp->vnum % MAX_KEY_HASH;
	bp->next						= wnum.pArea->blueprint_hash[iHash];
	wnum.pArea->blueprint_hash[iHash]			= bp;
	olc_set_editor(ch, ED_BLUEPRINT, bp);

	wnum.pArea->bottom_blueprint_vnum = UMIN(wnum.pArea->bottom_blueprint_vnum, bp->vnum);
	wnum.pArea->top_blueprint_vnum = UMAX(wnum.pArea->top_blueprint_vnum, bp->vnum);

    return true;

}


BPEDIT( bpedit_name )
{
	BLUEPRINT *bp;

	EDIT_BLUEPRINT(ch, bp);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name [string]\n\r", ch);
		return false;
	}

	free_string(bp->name);
	bp->name = str_dup(argument);
	send_to_char("Name changed.\n\r", ch);
	return true;
}

BPEDIT( bpedit_repop )
{
	BLUEPRINT *bp;

	EDIT_BLUEPRINT(ch, bp);

	if( !is_number(argument) )
	{
		send_to_char("Syntax:  repop [age]\n\r", ch);
		return false;
	}

	int repop = atoi(argument);
	bp->repop = UMAX(0, repop);
	send_to_char("Repop changed.\n\r", ch);
	return true;
}

BPEDIT( bpedit_flags )
{
	BLUEPRINT *bp;
	int value;

	EDIT_BLUEPRINT(ch, bp);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  flags <flags>\n\r", ch);
		send_to_char("'? instance' for list of flags.\n\r", ch);
		return false;
	}

	if( (value = flag_value(instance_flags, argument)) != NO_FLAG )
	{
		bp->flags ^= value;
		send_to_char("Instance flags changed.\n\r", ch);
		return true;
	}

	bpedit_flags(ch, "");
	return false;

}


BPEDIT( bpedit_description )
{
	BLUEPRINT *bp;

	EDIT_BLUEPRINT(ch, bp);

	if (argument[0] == '\0')
	{
		string_append(ch, &bp->description);
		return true;
	}

	send_to_char("Syntax:  description\n\r", ch);
	return false;
}

BPEDIT( bpedit_comments )
{
	BLUEPRINT *bp;

	EDIT_BLUEPRINT(ch, bp);

	if (argument[0] == '\0')
	{
		string_append(ch, &bp->comments);
		return true;
	}

	send_to_char("Syntax:  comments\n\r", ch);
	return false;
}

BPEDIT( bpedit_areawho )
{
	BLUEPRINT *bp;
	int value;

	EDIT_BLUEPRINT(ch, bp);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  areawho <value>\n\r", ch);
		send_to_char("See '? areawho' for list\n\r", ch);
		return false;
	}

	if ( !str_prefix(argument, "blank") )
	{
	    bp->area_who = AREA_BLANK;

	    send_to_char("Area who title cleared.\n\r", ch);
	    return true;
	}


	if ((value = flag_value(area_who_titles, argument)) != NO_FLAG)
	{
		bp->area_who = value;

		send_to_char("Area who title set.\n\r", ch);
		return true;
	}

	bpedit_areawho(ch, "");
	return false;
}

BPEDIT( bpedit_section )
{
	BLUEPRINT *bp;
	BLUEPRINT_SECTION *bs;
	char arg[MIL];

	EDIT_BLUEPRINT(ch, bp);

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  section add <widevnum>\n\r", ch);
		send_to_char("         section delete <#>\n\r", ch);
		send_to_char("         section list\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if( !str_prefix(arg, "add") )
	{
		WNUM wnum;
		if(!parse_widevnum(argument, ch->in_room->area, &wnum))
		{
			send_to_char("Please specify a widevnum.\n\r", ch);
			return false;
		}

		bs = get_blueprint_section(wnum.pArea, wnum.vnum);
		if( !bs )
		{
			send_to_char("That blueprint section does not exist.\n\r", ch);
			return false;
		}

		// Make sure the section is well defined
		if (bs->type == BSTYPE_STATIC)
		{
			if (bs->lower_vnum < 1 || bs->upper_vnum < 1 || bs->lower_vnum > bs->upper_vnum)
			{
				send_to_char("That blueprint section does not have any rooms defined.\n\r", ch);
				return false;
			}
		}
		else if (bs->type == BSTYPE_MAZE)
		{
			if (bs->maze_x < 1 || bs->maze_y < 1)
			{
				send_to_char("That blueprint section does not have its maze dimensions set.\n\r", ch);
				return false;
			}
		}
		else
		{
			send_to_char("That blueprint section needs a type.\n\r", ch);
			return false;
		}

		if( !list_appendlink(bp->sections, bs) )
		{
			send_to_char("{WError adding blueprint section to blueprint.{x\n\r", ch);
			return false;
		}

		send_to_char("Blueprint section added.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "delete") )
	{
		if(!is_number(argument))
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int index = atoi(argument);

		if( index < 1 || index > list_size(bp->sections) )
		{
			send_to_char("Index out of range.\n\r", ch);
			return false;
		}

		list_remnthlink(bp->sections, index, true);

		send_to_char("Blueprint section removed.\n\r", ch);
#if 0
		if( bp->mode == BLUEPRINT_MODE_STATIC )
		{
			// Remove any invalid layout definitions since the section has been removed
			STATIC_BLUEPRINT_LINK *prev, *cur, *next;

			prev = NULL;
			for(cur = bp->_static.layout; cur; cur = next)
			{
				next = cur->next;

				// Link references deleted section
				if( cur->section1 == index || cur->section2 == index )
				{
					if( !prev )
						bp->_static.layout = next;
					else
						prev->next = next;
					free_static_blueprint_link(cur);
					continue;
				}

				// If link references a section AFTER the specified index, shift down by one
				if( cur->section1 > index )
					cur->section1--;

				if( cur->section2 > index )
					cur->section2--;

				prev = cur;
			}

			// Check RECALL
			if( bp->_static.recall == index )
				bp->_static.recall = -1;
			else if( bp->_static.recall > index )
				bp->_static.recall--;

			ITERATOR bxit;
			BLUEPRINT_EXIT_DATA *bex;

			iterator_start(&bxit, bp->_static.entries);
			while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
			{
				if (bex->section == index)
				{
					iterator_remcurrent(&bxit);
				}
				else if (bex->section > index)
				{
					bex->section--;
				}
			}
			iterator_stop(&bxit);

			iterator_start(&bxit, bp->_static.exits);
			while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
			{
				if (bex->section == index)
				{
					iterator_remcurrent(&bxit);
				}
				else if (bex->section > index)
				{
					bex->section--;
				}
			}
			iterator_stop(&bxit);
		}
#endif
		return true;
	}

	if( !str_prefix(arg, "list") )
	{
		if( list_size(bp->sections) > 0 )
		{
			BUFFER *buffer = new_buf();

			char buf[MSL];
			int line = 0;

			ITERATOR sit;

			add_buf(buffer, "     [  Vnum  ] [             Name             ]\n\r");
			add_buf(buffer, "------------------------------------------------\n\r");

			iterator_start(&sit, bp->sections);
			while( (bs = (BLUEPRINT_SECTION *)iterator_nextdata(&sit)) )
			{
				sprintf(buf, "{W%4d  {G%8ld{x   %-30.30s{x\n\r", ++line, bs->vnum, bs->name);
				add_buf(buffer, buf);
			}

			iterator_stop(&sit);
			add_buf(buffer, "------------------------------------------------\n\r");

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
			send_to_char("Blueprint has no blueprint sections assigned.\n\r", ch);
		}

		return false;
	}

	bpedit_section(ch, "");
	return false;
}

BPEDIT( bpedit_scripted )
{
	BLUEPRINT *bp;

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		REMOVE_BIT(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT);
		send_to_char("Scripted Layout Mode disabled.\n\r", ch);
	}
	else
	{
		SET_BIT(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT);
		// Clear out all data.
		list_clear(bp->layout);
		list_clear(bp->links);
		list_clear(bp->entrances);
		list_clear(bp->exits);
		bp->recall = 0;
		send_to_char("Scripted Layout Mode enabled.\n\r", ch);
	}

	return false;
}

BPEDIT( bpedit_layout )
{
	BLUEPRINT *bp;
	char buf[MSL];
	char arg[MIL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit blueprint layout in OLC.\n\r", ch);
		return false;
	}

	if (argument[0] != '\0')
	{
		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "list"))
		{
			BUFFER *buffer = new_buf();

			bpedit_buffer_layout(buffer, bp);

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
		else if (!str_prefix(arg, "clear"))
		{
			if (list_size(bp->layout) < 1)
			{
				send_to_char("The layout is empty.\n\r", ch);
				return false;
			}

			list_clear(bp->layout);
			send_to_char("Layout cleared.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "add"))
		{
			char arg2[MIL];

			argument = one_argument(argument, arg2);

			if (!str_prefix(arg2, "static"))
			{
				if (list_size(bp->sections) < 1)
				{
					send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout add static {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				int section_no = atoi(argument);
				if (section_no < 1 || section_no > list_size(bp->sections))
				{
					send_to_char("Syntax:  layout add static {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

				BLUEPRINT_LAYOUT_SECTION_DATA *ls = new_blueprint_layout_section_data();
				ls->mode = SECTIONMODE_STATIC;
				ls->section = section_no;
				list_appendlink(bp->layout, ls);

				blueprint_update_section_ordinals(bp);

				sprintf(buf, "Static Section %d (%s - %ld#%ld) added to Layout.\n\r", list_size(bp->layout), section->name, section->area->uid, section->vnum);
				send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg2, "weighted"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = new_blueprint_layout_section_data();
				ls->mode = SECTIONMODE_WEIGHTED;
				ls->total_weight = 0;
				list_appendlink(bp->layout, ls);

				blueprint_update_section_ordinals(bp);

				sprintf(buf, "Weighted Section %d added to Layout.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
			}
			else if (!str_prefix(arg2, "group"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = new_blueprint_layout_section_data();
				ls->mode = SECTIONMODE_GROUP;
				list_appendlink(bp->layout, ls);

				// No need to update the ordinals

				sprintf(buf, "Group Section %d added to Layout.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
			}

			send_to_char("Syntax:  layout add {Rstatic{x <section#>\n\r", ch);
			send_to_char("         layout add {Rweighted{x\n\r", ch);
			send_to_char("         layout add {Rgroup{x\n\r", ch);
			return false;
		}
		else if (!str_prefix(arg, "static"))
		{
			if (list_size(bp->sections) < 1)
			{
				send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
				return false;
			}

			char arg2[MIL];

			argument = one_argument(argument, arg2);

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  layout static {R#{x <section#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bp->layout))
			{
				send_to_char("Syntax:  layout static {R#{x <section#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  layout static # {R<section#>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(argument);
			if (section_no < 1 || section_no > list_size(bp->sections))
			{
				send_to_char("Syntax:  layout static # {R<section#>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

			BLUEPRINT_LAYOUT_SECTION_DATA *ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(bp->layout, index);
			if (ls->mode != SECTIONMODE_STATIC)
			{
				send_to_char("Syntax:  layout static # {R<section#>{x\n\r", ch);
				send_to_char("That section is not a {YSTATIC{x section entry.\n\r", ch);
				return false;
			}

			ls->section = section_no;

			sprintf(buf, "Static section %d changed to %s (%ld#%ld)\n\r", index, section->name, section->area->uid, section->vnum);
			send_to_char(buf, ch);
			return true;
		}
		else if (!str_prefix(arg, "weighted"))
		{
			char arg2[MIL];
			char arg3[MIL];

			argument = one_argument(argument, arg2);

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  layout weighted {R#{x list\n\r", ch);
				send_to_char("         layout weighted {R#{x clear\n\r", ch);
				send_to_char("         layout weighted {R#{x add <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x set # <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x remove #\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg2);
			if (index < 1 || index > list_size(bp->layout))
			{
				send_to_char("Syntax:  layout weighted {R#{x list\n\r", ch);
				send_to_char("         layout weighted {R#{x clear\n\r", ch);
				send_to_char("         layout weighted {R#{x add <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x set # <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x remove #\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(bp->layout, index);
			if (ls->mode != SECTIONMODE_WEIGHTED)
			{
				send_to_char("Syntax:  layout weighted {R#{x list\n\r", ch);
				send_to_char("         layout weighted {R#{x clear\n\r", ch);
				send_to_char("         layout weighted {R#{x add <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x set # <weight> <section#>\n\r", ch);
				send_to_char("         layout weighted {R#{x remove #\n\r", ch);
				send_to_char("That section is not a {YWEIGHTED{x section entry.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);

			if (!str_prefix(arg3, "list"))
			{
				ITERATOR it;
				BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;

				BUFFER *buffer = new_buf();

				sprintf(buf, "Weighted %d Section Table:\n\r", index);
				add_buf(buffer, buf);

				add_buf(buffer, "      [ Weight ] [           Section           ]\n\r");
				add_buf(buffer, "=================================================\n\r");

				int weight_no = 0;
				iterator_start(&it, ls->weighted_sections);
				while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&it)))
				{
					BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
					sprintf(buf, "%4d    %7d     (%4d) %s (%ld#%ld)\n\r", weight_no++, weighted->weight, weighted->section, section->name, section->area->uid, section->vnum);
					add_buf(buffer, buf);
				}
				iterator_stop(&it);

				add_buf(buffer, "-------------------------------------------------\n\r");
				sprintf(buf, "Total   %7d\n\r", ls->total_weight);
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
			else if (!str_prefix(arg3, "clear"))
			{
				if (list_size(ls->weighted_sections) < 1)
				{
					sprintf(buf, "Weighted %d Section entry is empty.\n\r", index);
					send_to_char(buf, ch);
					return false;
				}

				list_clear(ls->weighted_sections);
				sprintf(buf, "Weighted %d Section entry cleared.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg3, "add"))
			{
				if (list_size(bp->sections) < 1)
				{
					send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
					return false;
				}

				char arg4[MIL];

				argument = one_argument(argument, arg4);
				if (!is_number(arg4))
				{
					send_to_char("Syntax:  layout weighted # add {R<weight>{x <section#>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg4);
				if (weight < 0)
				{
					send_to_char("Syntax:  layout weighted # add {R<weight>{x <section#>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout weighted # add <weight> {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				int section_no = atoi(argument);
				if (section_no < 1 || section_no > list_size(bp->sections))
				{
					send_to_char("Syntax:  layout weighted # add <weight> {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

				if (list_size(ls->weighted_sections) > 0)
				{
					int required_count = blueprint_layout_links_count(bp, ls, 0);
					int link_count = blueprint_section_link_count(section);

					if (required_count > link_count)
					{
						send_to_char("Syntax:  layout weighted # add <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "You can only add a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
						send_to_char(buf, ch);
						return false;
					}
				}

				BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = new_weighted_random_section();
				weighted->weight = weight;
				weighted->section = section_no;

				list_appendlink(ls->weighted_sections, weighted);
				ls->total_weight += weight;

				sprintf(buf, "Added section %s (%ld#%ld) with weight %d to Weighted %d Section Layout entry.\n\r", section->name, section->area->uid, section->vnum, weight, index);
				send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg3, "set"))
			{
				if (list_size(bp->sections) < 1)
				{
					send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
					return false;
				}

				char arg4[MIL];
				char arg5[MIL];

				argument = one_argument(argument, arg4);
				if (!is_number(arg4))
				{
					send_to_char("Syntax:  layout weighted # set {R#{x <weight> <section#>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->weighted_sections));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(arg4);
				if (windex < 1 || windex > list_size(ls->weighted_sections))
				{
					send_to_char("Syntax:  layout weighted # set {R#{x <weight> <section#>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->weighted_sections));
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  layout weighted # set # {R<weight>{x <section#>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg5);
				if (weight < 0)
				{
					send_to_char("Syntax:  layout weighted # set # {R<weight>{x <section#>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout weighted # set # <weight> {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				int section_no = atoi(argument);
				if (section_no < 1 || section_no > list_size(bp->sections))
				{
					send_to_char("Syntax:  layout weighted # set # <weight> {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

				if (list_size(ls->weighted_sections) > 1)
				{
					int required_count = blueprint_layout_links_count(bp, ls, windex);
					int link_count = blueprint_section_link_count(section);

					if (required_count > link_count)
					{
						send_to_char("Syntax:  layout weighted # set # <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "You can only use a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
						send_to_char(buf, ch);
						return false;
					}
				}

				BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)list_nthdata(ls->weighted_sections, windex);
				ls->total_weight -= weighted->weight;
				weighted->weight = weight;
				weighted->section = section_no;

				ls->total_weight += weight;

				sprintf(buf, "Updated Weighted %d Section Layout entry to section %s (%ld#%ld) with weight %d.\n\r", index, section->name, section->area->uid, section->vnum, weight);
				send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg3, "remove"))
			{
				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout weighted # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->weighted_sections));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(argument);
				if (windex < 1 || windex > list_size(ls->weighted_sections))
				{
					send_to_char("Syntax:  layout weighted # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->weighted_sections));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)list_nthdata(ls->weighted_sections, windex);

				ls->total_weight -= weighted->weight;

				list_remnthlink(ls->weighted_sections, windex, true);

				sprintf(buf, "Removed table entry %d from Weighted %d Section entry.\n\r", windex, index);
				send_to_char(buf, ch);
				return true;
			}
			
			send_to_char("Syntax:  layout weighted # {Rlist{x\n\r", ch);
			send_to_char("         layout weighted # {Rclear{x\n\r", ch);
			send_to_char("         layout weighted # {Radd{x <weight> <section#>\n\r", ch);
			send_to_char("         layout weighted # {Rset{x # <weight> <section#>\n\r", ch);
			send_to_char("         layout weighted # {Rremove{x #\n\r", ch);
			return false;
		}
		else if (!str_prefix(arg, "group"))
		{
			char argg[MIL];

			argument = one_argument(argument, argg);

			if (!is_number(argg))
			{
				send_to_char("Syntax:  layout group {R#{x list\n\r", ch);
				send_to_char("         layout group {R#{x clear\n\r", ch);
				send_to_char("         layout group {R#{x add static <section#>\n\r", ch);
				send_to_char("         layout group {R#{x add weighted\n\r", ch);
				send_to_char("         layout group {R#{x static # <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # list\n\r", ch);
				send_to_char("         layout group {R#{x weighted # clear\n\r", ch);
				send_to_char("         layout group {R#{x weighted # add <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # {Rset{x # <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # remove #\n\r", ch);
				send_to_char("         layout group {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			int gindex = atoi(argg);
			if (gindex < 1 || gindex > list_size(bp->layout))
			{
				send_to_char("Syntax:  layout group {R#{x list\n\r", ch);
				send_to_char("         layout group {R#{x clear\n\r", ch);
				send_to_char("         layout group {R#{x add static <section#>\n\r", ch);
				send_to_char("         layout group {R#{x add weighted\n\r", ch);
				send_to_char("         layout group {R#{x static # <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # list\n\r", ch);
				send_to_char("         layout group {R#{x weighted # clear\n\r", ch);
				send_to_char("         layout group {R#{x weighted # add <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # {Rset{x # <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # remove #\n\r", ch);
				send_to_char("         layout group {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(bp->layout, gindex);
			if (ls->mode != SECTIONMODE_GROUP)
			{
				send_to_char("Syntax:  layout group {R#{x list\n\r", ch);
				send_to_char("         layout group {R#{x clear\n\r", ch);
				send_to_char("         layout group {R#{x add static <section#>\n\r", ch);
				send_to_char("         layout group {R#{x add weighted\n\r", ch);
				send_to_char("         layout group {R#{x static # <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # list\n\r", ch);
				send_to_char("         layout group {R#{x weighted # clear\n\r", ch);
				send_to_char("         layout group {R#{x weighted # add <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # {Rset{x # <weight> <section#>\n\r", ch);
				send_to_char("         layout group {R#{x weighted # remove #\n\r", ch);
				send_to_char("         layout group {R#{x remove #\n\r", ch);

				send_to_char("That section is not a {YGROUP{x section entry.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "list"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *sect;

				if (list_size(ls->group) > 0)
				{
					BUFFER *buffer = new_buf();
					ITERATOR git;

					sprintf(buf, "Group %d Section Table:\n\r", gindex);
					add_buf(buffer, buf);

					add_buf(buffer, "          [  Mode  ]{x\n\r");
					add_buf(buffer, "===================================================={x\n\r");

					int gsection_no = 1;
					iterator_start(&git, ls->group);
					while( (sect = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&git)) )
					{
						if (sect->mode == SECTIONMODE_STATIC)
						{
							BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, sect->section);
							sprintf(buf, "{W%4d        {YSTATIC{x     %4d - {x%23.23s{x\n\r", gsection_no++, sect->section, bs->name);
							add_buf(buffer, buf);	
						}
						else if(sect->mode == SECTIONMODE_WEIGHTED)
						{
							ITERATOR wit;
							BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
							BLUEPRINT_SECTION *bs;

							sprintf(buf, "{W%4d       {CWEIGHTED{x\n\r", gsection_no++);
							add_buf(buffer, buf);
							add_buf(buffer, "{c               [ Weight ] [              Floor             ]{x\n\r");
							add_buf(buffer, "{c          ==================================================={x\n\r");

							int weight_no = 1;
							iterator_start(&wit, sect->weighted_sections);
							while( (weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)) )
							{
								bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
								sprintf(buf, "          {c%4d   {W%6d     {x%4d - %23.23s{x\n\r", weight_no++, weighted->weight, weighted->section, bs->name);
								add_buf(buffer, buf);
							}
							iterator_stop(&wit);
							add_buf(buffer, "{c          ----------------------------------------------------{x\n\r");
						}
					}
					iterator_stop(&git);

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
					send_to_char("There are no sections defined for this group entry.\n\r", ch);
				}

				return false;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(ls->group) < 1)
				{
					sprintf(buf, "Group %d Section entry has no subdefinitions.\n\r", gindex);
					send_to_char(buf, ch);
					return false;
				}

				list_clear(ls->group);

				blueprint_update_section_ordinals(bp);
				sprintf(buf, "Group %d Section entry cleared.\n\r", gindex);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "add"))
			{
				char arg2[MIL];

				argument = one_argument(argument, arg2);

				if (!str_prefix(arg2, "static"))
				{
					if (list_size(bp->sections) < 1)
					{
						send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  layout group # add static {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					int section_no = atoi(argument);
					if (section_no < 1 || section_no > list_size(bp->sections))
					{
						send_to_char("Syntax:  layout group # add static {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

					int required_count = blueprint_layout_links_count(bp, ls, 0);
					if (required_count > 0)
					{
						int link_count = blueprint_section_link_count(section);

						if (required_count > link_count)
						{
							send_to_char("Syntax:  layout group # add static {R<section#>{x\n\r", ch);
							sprintf(buf, "You can only add a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
							send_to_char(buf, ch);
							return false;
						}
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *gls = new_blueprint_layout_section_data();
					gls->mode = SECTIONMODE_STATIC;
					gls->section = section_no;
					list_appendlink(ls->group, gls);

					blueprint_update_section_ordinals(bp);

					sprintf(buf, "Static Section %d (%s - %ld#%ld) added to Group %d Section entry.\n\r", list_size(ls->group), section->name, section->area->uid, section->vnum, gindex);
					send_to_char(buf, ch);
					return true;
				}
				else if (!str_prefix(arg2, "weighted"))
				{
					BLUEPRINT_LAYOUT_SECTION_DATA *gls = new_blueprint_layout_section_data();
					gls->mode = SECTIONMODE_WEIGHTED;
					gls->total_weight = 0;
					list_appendlink(ls->group, gls);

					blueprint_update_section_ordinals(bp);

					sprintf(buf, "Weighted Section %d added to Group %d Section entry.\n\r", list_size(ls->group), gindex);
					send_to_char(buf, ch);
				}

				send_to_char("Syntax:  layout group # add {Rstatic{x <section#>\n\r", ch);
				send_to_char("         layout group # add {Rweighted{x\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg, "static"))
			{
				if (list_size(bp->sections) < 1)
				{
					send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
					return false;
				}

				char arg2[MIL];

				argument = one_argument(argument, arg2);

				if (!is_number(arg2))
				{
					send_to_char("Syntax:  layout group # static {R#{x <section#>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(argument);
				if (index < 1 || index > list_size(ls->group))
				{
					send_to_char("Syntax:  layout group # static {R#{x <section#>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout group # static # {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				int section_no = atoi(argument);
				if (section_no < 1 || section_no > list_size(ls->group))
				{
					send_to_char("Syntax:  layout group # static # {R<section#>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

				int required_count = blueprint_layout_links_count(bp, ls, index);
				if (required_count > 0)
				{
					int link_count = blueprint_section_link_count(section);

					if (required_count > link_count)
					{
						send_to_char("Syntax:  layout group # add static {R<section#>{x\n\r", ch);
						sprintf(buf, "You can only use a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
						send_to_char(buf, ch);
						return false;
					}
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(ls->group, index);
				if (gls->mode != SECTIONMODE_STATIC)
				{
					send_to_char("Syntax:  layout group # static # {R<section#>{x\n\r", ch);
					send_to_char("That section is not a {YSTATIC{x section entry.\n\r", ch);
					return false;
				}

				gls->section = section_no;

				sprintf(buf, "Static section %d in Group %d Entry changed to %s (%ld#%ld)\n\r", index, gindex, section->name, section->area->uid, section->vnum);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "weighted"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);

				if (!is_number(arg2))
				{
					send_to_char("Syntax:  layout group # weighted {R#{x list\n\r", ch);
					send_to_char("         layout group # weighted {R#{x clear\n\r", ch);
					send_to_char("         layout group # weighted {R#{x add <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x set # <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x remove #\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg2);
				if (index < 1 || index > list_size(ls->group))
				{
					send_to_char("Syntax:  layout group # weighted {R#{x list\n\r", ch);
					send_to_char("         layout group # weighted {R#{x clear\n\r", ch);
					send_to_char("         layout group # weighted {R#{x add <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x set # <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x remove #\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(ls->group, index);
				if (gls->mode != SECTIONMODE_WEIGHTED)
				{
					send_to_char("Syntax:  layout group # weighted {R#{x list\n\r", ch);
					send_to_char("         layout group # weighted {R#{x clear\n\r", ch);
					send_to_char("         layout group # weighted {R#{x add <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x set # <weight> <section#>\n\r", ch);
					send_to_char("         layout group # weighted {R#{x remove #\n\r", ch);
					send_to_char("That section is not a {YWEIGHTED{x section entry.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg3);

				if (!str_prefix(arg3, "list"))
				{
					ITERATOR it;
					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;

					BUFFER *buffer = new_buf();

					sprintf(buf, "Weighted %d Section Table:\n\r", index);
					add_buf(buffer, buf);

					add_buf(buffer, "      [ Weight ] [           Section           ]\n\r");
					add_buf(buffer, "=================================================\n\r");

					int weight_no = 0;
					iterator_start(&it, gls->weighted_sections);
					while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&it)))
					{
						BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
						sprintf(buf, "%4d    %7d     (%4d) %s (%ld#%ld)\n\r", weight_no++, weighted->weight, weighted->section, section->name, section->area->uid, section->vnum);
						add_buf(buffer, buf);
					}
					iterator_stop(&it);

					add_buf(buffer, "-------------------------------------------------\n\r");
					sprintf(buf, "Total   %7d\n\r", gls->total_weight);
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
				else if (!str_prefix(arg3, "clear"))
				{
					if (list_size(gls->weighted_sections) < 1)
					{
						sprintf(buf, "Weighted %d Section entry is empty.\n\r", index);
						send_to_char(buf, ch);
						return false;
					}

					list_clear(gls->weighted_sections);
					sprintf(buf, "Weighted %d Section entry cleard in Group %d Section.\n\r", index, gindex);
					send_to_char(buf, ch);
					return true;
				}
				else if (!str_prefix(arg3, "add"))
				{
					if (list_size(bp->sections) < 1)
					{
						send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
						return false;
					}

					char arg4[MIL];

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  layout group # weighted # add {R<weight>{x <section#>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg4);
					if (weight < 0)
					{
						send_to_char("Syntax:  layout group # weighted # add {R<weight>{x <section#>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  layout group # weighted # add <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					int section_no = atoi(argument);
					if (section_no < 1 || section_no > list_size(bp->sections))
					{
						send_to_char("Syntax:  layout group # weighted # add <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);

					int required_count = blueprint_layout_links_count(bp, ls, 0);
					if (required_count > 0)
					{
						int link_count = blueprint_section_link_count(section);

						if (required_count > link_count)
						{
							send_to_char("Syntax:  layout group # weighted # add <weight> {R<section#>{x\n\r", ch);
							sprintf(buf, "You can only add a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
							send_to_char(buf, ch);
							return false;
						}
					}

					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = new_weighted_random_section();
					weighted->weight = weight;
					weighted->section = section_no;

					list_appendlink(gls->weighted_sections, weighted);
					ls->total_weight += weight;

					sprintf(buf, "Added section %s (%ld#%ld) with weight %d to Weighted %d Section entry in Group %d Section.\n\r", section->name, section->area->uid, section->vnum, weight, index, gindex);
					send_to_char(buf, ch);
					return true;
				}
				else if (!str_prefix(arg3, "set"))
				{
					if (list_size(bp->sections) < 1)
					{
						send_to_char("Sections list is empty.  Please use {Ysections add{x command to populate the list.\n\r", ch);
						return false;
					}

					char arg4[MIL];
					char arg5[MIL];

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  layout group # weighted # set {R#{x <weight> <section#>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->weighted_sections));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(arg4);
					if (windex < 1 || windex > list_size(gls->weighted_sections))
					{
						send_to_char("Syntax:  layout group # weighted # set {R#{x <weight> <section#>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->weighted_sections));
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  layout group # weighted # set # {R<weight>{x <section#>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg5);
					if (weight < 0)
					{
						send_to_char("Syntax:  layout group # weighted # set # {R<weight>{x <section#>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  layout group # weighted # set # <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					int section_no = atoi(argument);
					if (section_no < 1 || section_no > list_size(bp->sections))
					{
						send_to_char("Syntax:  layout group # weighted # set # <weight> {R<section#>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->sections));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_SECTION *section = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section_no);
					int link_count = blueprint_section_link_count(section);

					// Check everything in the group but this weighted entry
					int required_count = blueprint_layout_links_count(bp, ls, index);
					if (required_count > 0)
					{
						if (required_count > link_count)
						{
							send_to_char("Syntax:  layout group # weighted # set # <weight> {R<section#>{x\n\r", ch);
							sprintf(buf, "You can only use a section with at least %d link%s defined.\n\r", required_count, (required_count == 1?"":"s"));
							send_to_char(buf, ch);
							return false;
						}
					}

					int weighted_count = blueprint_layout_links_count(bp, gls, windex);
					if (weighted_count > 0)
					{
						if (weighted_count > link_count)
						{
							send_to_char("Syntax:  layout group # weighted # set # <weight> {R<section#>{x\n\r", ch);
							sprintf(buf, "You can only use a section with at least %d link%s defined.\n\r", weighted_count, (weighted_count == 1?"":"s"));
							send_to_char(buf, ch);
							return false;
						}
					}

					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)list_nthdata(gls->weighted_sections, windex);
					ls->total_weight -= weighted->weight;
					weighted->weight = weight;
					weighted->section = section_no;

					ls->total_weight += weight;

					sprintf(buf, "Updated Weighted %d Section entry in Group %d Section to section %s (%ld#%ld) with weight %d.\n\r", index, gindex, section->name, section->area->uid, section->vnum, weight);
					send_to_char(buf, ch);
					return true;
				}
				else if (!str_prefix(arg3, "remove"))
				{
					if (!is_number(argument))
					{
						send_to_char("Syntax:  layout group # weighted # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->weighted_sections));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(argument);
					if (windex < 1 || windex > list_size(gls->weighted_sections))
					{
						send_to_char("Syntax:  layout group # weighted # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->weighted_sections));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)list_nthdata(gls->weighted_sections, windex);

					ls->total_weight -= weighted->weight;

					list_remnthlink(gls->weighted_sections, windex, true);

					sprintf(buf, "Removed table entry %d from Weighted %d Section entry in Group %d Section.\n\r", windex, index, gindex);
					send_to_char(buf, ch);
					return true;
				}
				
				send_to_char("Syntax:  layout group # weighted # {Rlist{x\n\r", ch);
				send_to_char("         layout group # weighted # {Rclear{x\n\r", ch);
				send_to_char("         layout group # weighted # {Radd{x <weight> <section#>\n\r", ch);
				send_to_char("         layout group # weighted # {Rset{x # <weight> <section#>\n\r", ch);
				send_to_char("         layout group # weighted # {Rremove{x #\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg, "remove"))
			{
				if (!is_number(argument))
				{
					send_to_char("Syntax:  layout group # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(argument);
				if (index < 1 || index > list_size(ls->group))
				{
					send_to_char("Syntax:  layout group # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ls->group));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(ls->group, index, true);
				blueprint_update_section_ordinals(bp);

				sprintf(buf, "Layout %d removed from Group %d Section.\n\r", index, gindex);
				send_to_char(buf, ch);
				return true;
			}

			send_to_char("Syntax:  layout group # {Rlist{x\n\r", ch);
			send_to_char("         layout group # {Rclear{x\n\r", ch);
			send_to_char("         layout group # {Radd{x static <section#>\n\r", ch);
			send_to_char("         layout group # {Radd{x weighted\n\r", ch);
			send_to_char("         layout group # {Rstatic{x # <section#>\n\r", ch);
			send_to_char("         layout group # {Rweighted{x # list\n\r", ch);
			send_to_char("         layout group # {Rweighted{x # clear\n\r", ch);
			send_to_char("         layout group # {Rweighted{x # add <weight> <section#>\n\r", ch);
			send_to_char("         layout group # {Rweighted{x # {Rset{x # <weight> <section#>\n\r", ch);
			send_to_char("         layout group # {Rweighted{x # remove #\n\r", ch);
			send_to_char("         layout group # {Rremove{x #\n\r", ch);
			return false;
		}
		else if (!str_prefix(arg, "remove"))
		{
			if (!is_number(argument))
			{
				send_to_char("Syntax:  layout remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bp->layout))
			{
				send_to_char("Syntax:  layout remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->layout));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bp->layout, index, true);
			blueprint_update_section_ordinals(bp);

			sprintf(buf, "Layout %d removed.\n\r", index);
			send_to_char(buf, ch);
			return true;
		}
	}

	send_to_char("Syntax:  layout {Rlist{x\n\r", ch);
	send_to_char("         layout {Rclear{x\n\r", ch);
	send_to_char("         layout {Radd{x static <section#>\n\r", ch);
	send_to_char("         layout {Radd{x weighted\n\r", ch);
	send_to_char("         layout {Radd{x group\n\r", ch);
	send_to_char("         layout {Rstatic{x # <section#>\n\r", ch);
	send_to_char("         layout {Rweighted{x # list\n\r", ch);
	send_to_char("         layout {Rweighted{x # clear\n\r", ch);
	send_to_char("         layout {Rweighted{x # add <weight> <section#>\n\r", ch);
	send_to_char("         layout {Rweighted{x # remove #\n\r", ch);
	send_to_char("         layout {Rgroup{x # list\n\r", ch);
	send_to_char("         layout {Rgroup{x # clear\n\r", ch);
	send_to_char("         layout {Rgroup{x # add static <section#>\n\r", ch);
	send_to_char("         layout {Rgroup{x # add weighted\n\r", ch);
	send_to_char("         layout {Rgroup{x # static # <section#>\n\r", ch);
	send_to_char("         layout {Rgroup{x # weighted # list\n\r", ch);
	send_to_char("         layout {Rgroup{x # weighted # clear\n\r", ch);
	send_to_char("         layout {Rgroup{x # weighted # add <weight> <section#>\n\r", ch);
	send_to_char("         layout {Rgroup{x # weighted # {Rset{x # <weight> <section#>\n\r", ch);
	send_to_char("         layout {Rgroup{x # weighted # remove #\n\r", ch);
	send_to_char("         layout {Rgroup{x # remove #\n\r", ch);
	send_to_char("         layout {Rremove{x #\n\r", ch);
	return false;
}


BPEDIT( bpedit_links )
{
	BLUEPRINT *bp;
	char arg[MIL];
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit blueprint links in OLC.\n\r", ch);
		return false;
	}

	if (argument[0] != '\0')
	{
		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "list"))
		{
			BUFFER *buffer = new_buf();

			bpedit_buffer_links(buffer, bp);

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

		if (!str_prefix(arg, "clear"))
		{
			if (list_size(bp->links) < 1)
			{
				send_to_char("There are no links defined.\n\r", ch);
				return false;
			}

			list_clear(bp->links);
			send_to_char("Links clear.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "add"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  links add {Rstatic{x generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links add {Rsource{x generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links add {Rdestination{x generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links add {Rweighted{x\n\r", ch);
				send_to_char("         links add {Rgroup{x\n\r", ch);
				return false;
			}

			char arg2[MIL];

			argument = one_argument(argument, arg2);
			if (!str_prefix(arg2, "static"))
			{
				int sections = blueprint_generation_count(bp);
				char arg3[MIL];
				char arg4[MIL];
				char arg5[MIL];
				char arg6[MIL];
				char arg7[MIL];
				sent_bool from_mode = TRISTATE_UNDEF;
				sent_bool to_mode = TRISTATE_UNDEF;

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "generated"))
					from_mode = TRISTATE_FALSE;
				else if(!str_prefix(arg3, "ordinal"))
					from_mode = TRISTATE_TRUE;
				else
				{
					send_to_char("Syntax:  links add static {Rgenerated|ordinal{x <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if (!is_number(arg4))
				{
					send_to_char("Syntax:  links add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int from_section = atoi(arg4);
				if (from_section < 1 || from_section > sections)
				{
					send_to_char("Syntax:  links add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

				if (link_count < 1)
				{
					send_to_char("Syntax:  links add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> {R<from-link>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int from_link_no = atoi(arg5);
				if (from_link_no < 1 || from_link_no > link_count)
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> {R<from-link>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}
				
				argument = one_argument(argument, arg6);
				if (!str_prefix(arg6, "generated"))
					to_mode = false;
				else if (!str_prefix(arg6, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg7);
				if (!is_number(arg7))
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int to_section = atoi(arg7);
				if (to_section < 1 || to_section > sections)
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

				// This is the maximum allowed links based upon the configuration
				link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
				if (link_count < 1)
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int to_link_no = atoi(argument);
				if (to_link_no < 1 || to_link_no > link_count)
				{
					send_to_char("Syntax:  links add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
				ll->mode = LINKMODE_STATIC;

				blueprint_add_weighted_link(ll->from, 1, from_mode ? -from_section : from_section, from_link_no);
				ll->total_from = 1;

				blueprint_add_weighted_link(ll->to, 1, to_mode ? -to_section : to_section, to_link_no);
				ll->total_to = 1;

				list_appendlink(bp->links, ll);
				sprintf(buf, "Static Link %d added to Links.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return true;
			}
			if (!str_prefix(arg2, "source"))
			{
				int sections = blueprint_generation_count(bp);
				char arg3[MIL];
				char arg4[MIL];
				sent_bool to_mode = TRISTATE_UNDEF;

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "generated"))
					to_mode = TRISTATE_FALSE;
				else if(!str_prefix(arg3, "ordinal"))
					to_mode = TRISTATE_TRUE;
				else
				{
					send_to_char("Syntax:  links add source {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if (!is_number(arg4))
				{
					send_to_char("Syntax:  links add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int to_section = atoi(arg4);
				if (to_section < 1 || to_section > sections)
				{
					send_to_char("Syntax:  links add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

				if (link_count < 1)
				{
					send_to_char("Syntax:  links add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				int to_link_no = atoi(argument);
				if (to_link_no < 1 || to_link_no > link_count)
				{
					send_to_char("Syntax:  links add source generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
				ll->mode = LINKMODE_SOURCE;

				blueprint_add_weighted_link(ll->to, 1, to_mode ? -to_section : to_section, to_link_no);
				ll->total_to = 1;

				list_appendlink(bp->links, ll);
				sprintf(buf, "Source Link %d added to Links.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return true;
			}
			if (!str_prefix(arg2, "destination"))
			{
				int sections = blueprint_generation_count(bp);
				char arg3[MIL];
				char arg4[MIL];
				sent_bool from_mode = TRISTATE_UNDEF;

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "generated"))
					from_mode = false;
				else if(!str_prefix(arg3, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  links add destination {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if (!is_number(arg4))
				{
					send_to_char("Syntax:  links add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int from_section = atoi(arg4);
				if (from_section < 1 || from_section > sections)
				{
					send_to_char("Syntax:  links add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

				if (link_count < 1)
				{
					send_to_char("Syntax:  links add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  links add destination generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int from_link_no = atoi(argument);
				if (from_link_no < 1 || from_link_no > link_count)
				{
					send_to_char("Syntax:  links add destination generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
				ll->mode = LINKMODE_DESTINATION;

				blueprint_add_weighted_link(ll->from, 1, from_mode ? -from_section : from_section, from_link_no);
				ll->total_from = 1;

				list_appendlink(bp->links, ll);
				sprintf(buf, "Destination Link %d added to Links.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return true;
			}
			if (!str_prefix(arg2, "weighted"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = new_blueprint_layout_link_data();
				link->mode = LINKMODE_WEIGHTED;
				link->total_from = 0;
				link->total_to = 0;

				list_appendlink(bp->links, link);
				sprintf(buf, "Weighted Link %d added to Links.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return true;
			}
			if (!str_prefix(arg2, "group"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = new_blueprint_layout_link_data();
				link->mode = LINKMODE_GROUP;

				list_appendlink(bp->links, link);
				sprintf(buf, "Group Link %d added to Links.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return true;
			}

			send_to_char("Syntax:  links add {Rstatic{x generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links add {Rsource{x generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links add {Rdestination{x generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links add {Rweighted{x\n\r", ch);
			send_to_char("         links add {Rgroup{x\n\r", ch);
			return false;
		}
	
		if (!str_prefix(arg, "from"))
		{
			char arg2[MIL];
			char arg3[MIL];

			argument = one_argument(argument, arg2);
			if(!is_number(arg2))
			{
				send_to_char("Syntax:  links from {R#{x list\n\r", ch);
				send_to_char("         links from {R#{x add <weight> <from-section> <from-link>\n\r", ch);
				send_to_char("         links from {R#{x set # <weight> <from-section> <from-link>\n\r", ch);
				send_to_char("         links from {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg2);
			if(index < 1 || index > list_size(bp->links))
			{
				send_to_char("Syntax:  links from {R#{x list\n\r", ch);
				send_to_char("         links from {R#{x add <weight> <from-section> <from-link>\n\r", ch);
				send_to_char("         links from {R#{x set # <weight> <from-section> <from-link>\n\r", ch);
				send_to_char("         links from {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_LINK_DATA *ll = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(bp->links, index);

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "list"))
			{
				if (list_size(ll->from) > 0)
				{
					BUFFER *buffer = new_buf();

					sprintf(buf, "From Table Entries for Link %d:\n\r", index);
					add_buf(buffer, buf);

					add_buf(buffer, "     [ Weight ] [ Section ] [ Link ]\n\r");
					add_buf(buffer, "=====================================\n\r");

					int from_no = 1;
					ITERATOR it;
					BLUEPRINT_WEIGHTED_LINK_DATA *weighted;
					iterator_start(&it, ll->from);
					while((weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)))
					{
						sprintf(buf, "%4d   %6d     {%c%7d{x      %4d\n\r", from_no++, weighted->weight,
							(weighted->section<0?'G':(weighted->section>0?'Y':'W')),
							abs(weighted->section), weighted->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&it);

					add_buf(buffer, "-------------------------------------\n\r");
					add_buf(buffer, "{YYELLOW{x - Generated section position index for Source/Destination sections\n\r");
					add_buf(buffer, "{GGREEN{x  - Ordinal section position index for Source/Destination sections\n\r");

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
					send_to_char("There are no From table definitions.\n\r", ch);
				}
				return false;
			}

			if (!str_prefix(arg3, "add"))
			{
				int sections = blueprint_generation_count(bp);
				sent_bool from_mode = TRISTATE_UNDEF;
				char arg4[MIL];
				char arg5[MIL];
				char arg6[MIL];

				if (ll->mode != LINKMODE_SOURCE && ll->mode != LINKMODE_WEIGHTED)
				{
					send_to_char("Syntax:  links from {R#{x list\n\r", ch);
					send_to_char("         links from {R#{x add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("         links from {R#{x set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("         links from {R#{x remove #\n\r", ch);

					send_to_char("Only able to change the From table on {YSOURCE{x or {YWEIGHTED{x Links.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if(!is_number(arg4))
				{
					send_to_char("Syntax:  links from # add {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg4);
				if (weight < 1)
				{
					send_to_char("Syntax:  links from # add {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if(!str_prefix(arg5, "generated"))
					from_mode = false;
				else if(!str_prefix(arg5, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  links from # add <weight> {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if(!is_number(arg6))
				{
					send_to_char("Syntax:  links from # add <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int from_section = atoi(arg6);
				if(from_section < 1 || from_section > sections)
				{
					send_to_char("Syntax:  links from # add <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
				if (link_count < 1)
				{
					send_to_char("Syntax:  links from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  links from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int from_link_no = atoi(arg5);
				if (from_link_no < 1 || from_link_no > link_count)
				{
					send_to_char("Syntax:  links from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				blueprint_add_weighted_link(ll->from, weight, from_mode ? -from_section : from_section, from_link_no);
				ll->total_from += weight;

				char *mode = "Link";
				if (ll->mode == LINKMODE_SOURCE)
					mode = "Source Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Add Entry %d to %s %d.\n\r", list_size(ll->from), mode, index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "set"))
			{
				int sections = blueprint_generation_count(bp);
				sent_bool from_mode = TRISTATE_UNDEF;
				char argw[MIL];
				char arg4[MIL];
				char arg5[MIL];
				char arg6[MIL];

				argument = one_argument(argument, argw);
				if(!is_number(argw))
				{
					send_to_char("Syntax:  links from # set {R#{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(argw);
				if(windex < 1 || windex > list_size(ll->from))
				{
					send_to_char("Syntax:  links from # set {R#{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if(!is_number(arg4))
				{
					send_to_char("Syntax:  links from # set # {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg4);
				if (weight < 1)
				{
					send_to_char("Syntax:  links from # set # {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if(!str_prefix(arg5, "generated"))
					from_mode = false;
				else if(!str_prefix(arg5, "ordinal"))
					from_mode = true;
				else
				{
					send_to_char("Syntax:  links from # set # <weight> {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if(!is_number(arg6))
				{
					send_to_char("Syntax:  links from # set # <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int from_section = atoi(arg6);
				if(from_section < 1 || from_section > sections)
				{
					send_to_char("Syntax:  links from # set # <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
				if (link_count < 1)
				{
					send_to_char("Syntax:  links from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  links from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int from_link_no = atoi(arg5);
				if (from_link_no < 1 || from_link_no > link_count)
				{
					send_to_char("Syntax:  links from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->from, windex);
				ll->total_from -= weighted->weight;
				weighted->weight = weight;
				weighted->section = from_section;
				weighted->link = from_link_no;
				ll->total_from += weight;

				char *mode = "Link";
				if (ll->mode == LINKMODE_SOURCE)
					mode = "Source Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Updated Entry %d to %s %d.\n\r", windex, mode, index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "remove"))
			{
				if (ll->mode != LINKMODE_SOURCE && ll->mode != LINKMODE_WEIGHTED)
				{
					send_to_char("Syntax:  links from {R#{x list\n\r", ch);
					send_to_char("         links from {R#{x add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("         links from {R#{x set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("         links from {R#{x remove #\n\r", ch);

					send_to_char("Only able to change the From table on {YSOURCE{x or {YWEIGHTED{x Links.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  links from # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(argument);
				if (windex < 1 || windex > list_size(ll->from))
				{
					send_to_char("Syntax:  links from # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->from, windex);
				ll->total_from -= weighted->weight;
				list_remnthlink(ll->from, windex, true);

				char *mode = "Link";
				if (ll->mode == LINKMODE_SOURCE)
					mode = "Source Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Removed Entry %d from %s %d.\n\r", windex, mode, index);
				send_to_char(buf, ch);
				return true;
			}

			send_to_char("Syntax:  links from # {Rlist{x\n\r", ch);
			send_to_char("         links from # {Radd{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links from # {Rset{x # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links from # {Rremove{x #\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "to"))
		{
			char arg2[MIL];
			char arg3[MIL];

			argument = one_argument(argument, arg2);
			if(!is_number(arg2))
			{
				send_to_char("Syntax:  links to {R#{x list\n\r", ch);
				send_to_char("         links to {R#{x add <weight> <to-section> <to-link>\n\r", ch);
				send_to_char("         links to {R#{x set # <weight> <to-section> <to-link>\n\r", ch);
				send_to_char("         links to {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(arg2);
			if(index < 1 || index > list_size(bp->links))
			{
				send_to_char("Syntax:  links to {R#{x list\n\r", ch);
				send_to_char("         links to {R#{x add <weight> <to-section> <to-link>\n\r", ch);
				send_to_char("         links to {R#{x set # <weight> <to-section> <to-link>\n\r", ch);
				send_to_char("         links to {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_LINK_DATA *ll = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(bp->links, index);

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "list"))
			{
				if (list_size(ll->to) > 0)
				{
					BUFFER *buffer = new_buf();

					sprintf(buf, "To Table Entries for Link %d:\n\r", index);
					add_buf(buffer, buf);

					add_buf(buffer, "     [ Weight ] [ Section ] [ Link ]\n\r");
					add_buf(buffer, "=====================================\n\r");

					int to_no = 1;
					ITERATOR it;
					BLUEPRINT_WEIGHTED_LINK_DATA *weighted;
					iterator_start(&it, ll->to);
					while((weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)))
					{
						sprintf(buf, "%4d   %6d     {%c%7d{x      %4d\n\r", to_no++, weighted->weight,
							(weighted->section<0?'G':(weighted->section>0?'Y':'W')),
							abs(weighted->section), weighted->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&it);

					add_buf(buffer, "-------------------------------------\n\r");
					add_buf(buffer, "{YYELLOW{x - Generated section position index for Source/Destination sections\n\r");
					add_buf(buffer, "{GGREEN{x  - Ordinal section position index for Source/Destination sections\n\r");

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
					send_to_char("There are no To table definitions.\n\r", ch);
				}
				return false;
			}

			if (!str_prefix(arg3, "add"))
			{
				int sections = blueprint_generation_count(bp);
				sent_bool to_mode = TRISTATE_UNDEF;
				char arg4[MIL];
				char arg5[MIL];
				char arg6[MIL];

				if (ll->mode != LINKMODE_DESTINATION && ll->mode != LINKMODE_WEIGHTED)
				{
					send_to_char("Syntax:  links to {R#{x list\n\r", ch);
					send_to_char("         links to {R#{x add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links to {R#{x set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links to {R#{x remove #\n\r", ch);

					send_to_char("Only able to change the To table on {YDESTINATION{x or {YWEIGHTED{x Links.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if(!is_number(arg4))
				{
					send_to_char("Syntax:  links to # add {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg4);
				if (weight < 1)
				{
					send_to_char("Syntax:  links to # add {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if(!str_prefix(arg5, "generated"))
					to_mode = false;
				else if(!str_prefix(arg5, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  links to # add <weight> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if(!is_number(arg6))
				{
					send_to_char("Syntax:  links to # add <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int to_section = atoi(arg6);
				if(to_section < 1 || to_section > sections)
				{
					send_to_char("Syntax:  links to # add <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
				if (link_count < 1)
				{
					send_to_char("Syntax:  links to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  links to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int to_link_no = atoi(arg5);
				if (to_link_no < 1 || to_link_no > link_count)
				{
					send_to_char("Syntax:  links to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				blueprint_add_weighted_link(ll->to, weight, to_mode ? -to_section : to_section, to_link_no);
				ll->total_to += weight;

				char *mode = "Link";
				if (ll->mode == LINKMODE_DESTINATION)
					mode = "Destination Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Added To Entry %d to %s %d.\n\r", list_size(ll->to), mode, index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "set"))
			{
				int sections = blueprint_generation_count(bp);
				sent_bool to_mode = TRISTATE_UNDEF;
				char argw[MIL];
				char arg4[MIL];
				char arg5[MIL];
				char arg6[MIL];

				argument = one_argument(argument, argw);
				if(!is_number(argw))
				{
					send_to_char("Syntax:  links to # set {R#{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(argw);
				if(windex < 1 || windex > list_size(ll->to))
				{
					send_to_char("Syntax:  links to # set {R#{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
					send_to_char(buf, ch);
					return false;
				}

				argument = one_argument(argument, arg4);
				if(!is_number(arg4))
				{
					send_to_char("Syntax:  links to # set # {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				int weight = atoi(arg4);
				if (weight < 1)
				{
					send_to_char("Syntax:  links to # set # {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if(!str_prefix(arg5, "generated"))
					to_mode = false;
				else if(!str_prefix(arg5, "ordinal"))
					to_mode = true;
				else
				{
					send_to_char("Syntax:  links to # set # <weight> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
					send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg6);
				if(!is_number(arg6))
				{
					send_to_char("Syntax:  links to # set # <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				int to_section = atoi(arg6);
				if(to_section < 1 || to_section > sections)
				{
					send_to_char("Syntax:  links to # set # <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_SECTION_DATA *group;
				BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

				// This is the maximum allowed links based upon the configuration
				int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
				if (link_count < 1)
				{
					send_to_char("Syntax:  links to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					send_to_char("No links defined for this section reference.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg5);
				if (!is_number(arg5))
				{
					send_to_char("Syntax:  links to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				int to_link_no = atoi(arg5);
				if (to_link_no < 1 || to_link_no > link_count)
				{
					send_to_char("Syntax:  links to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->to, windex);
				ll->total_to -= weighted->weight;
				weighted->weight = weight;
				weighted->section = to_section;
				weighted->link = to_link_no;
				ll->total_to += weight;

				char *mode = "Link";
				if (ll->mode == LINKMODE_SOURCE)
					mode = "Source Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Updated Entry %d to %s %d.\n\r", windex, mode, index);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg3, "remove"))
			{
				if (ll->mode != LINKMODE_DESTINATION && ll->mode != LINKMODE_WEIGHTED)
				{
					send_to_char("Syntax:  links to {R#{x list\n\r", ch);
					send_to_char("         links to {R#{x add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links to {R#{x set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links to {R#{x remove #\n\r", ch);

					send_to_char("Only able to change the To table on {YDESTINATION{x or {YWEIGHTED{x Links.\n\r", ch);
					return false;
				}

				if (!is_number(argument))
				{
					send_to_char("Syntax:  links to # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
					send_to_char(buf, ch);
					return false;
				}

				int windex = atoi(argument);
				if (windex < 1 || windex > list_size(ll->to))
				{
					send_to_char("Syntax:  links to # remove {R#{x\n\r", ch);
					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->to, windex);
				ll->total_to -= weighted->weight;
				list_remnthlink(ll->to, windex, true);

				char *mode = "Link";
				if (ll->mode == LINKMODE_DESTINATION)
					mode = "Destination Link";
				else if (ll->mode == LINKMODE_WEIGHTED)
					mode = "Weighted Link";
				sprintf(buf, "Removed To Entry %d from %s %d.\n\r", windex, mode, index);
				send_to_char(buf, ch);
				return true;
			}

			send_to_char("Syntax:  links to # {Rlist{x\n\r", ch);
			send_to_char("         links to # {Radd{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links to # {Rset{x # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links to # {Rremove{x #\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "group"))
		{
			char argg[MIL];

			argument = one_argument(argument, argg);
			if(!is_number(argg))
			{
				send_to_char("Syntax:  links group {R#{x add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add source generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add destination generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x add weighted\n\r", ch);
				send_to_char("         links group {R#{x add group\n\r", ch);
				send_to_char("         links group {R#{x from # list\n\r", ch);
				send_to_char("         links group {R#{x from # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # remove #\n\r", ch);
				send_to_char("         links group {R#{x to # list\n\r", ch);
				send_to_char("         links group {R#{x to # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # remove #\n\r", ch);
				send_to_char("         links group {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			int gindex = atoi(argg);
			if(gindex < 1 || list_size(bp->links))
			{
				send_to_char("Syntax:  links group {R#{x add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add source generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add destination generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x add weighted\n\r", ch);
				send_to_char("         links group {R#{x add group\n\r", ch);
				send_to_char("         links group {R#{x from # list\n\r", ch);
				send_to_char("         links group {R#{x from # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # remove #\n\r", ch);
				send_to_char("         links group {R#{x to # list\n\r", ch);
				send_to_char("         links group {R#{x to # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # remove #\n\r", ch);
				send_to_char("         links group {R#{x remove #\n\r", ch);

				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->links));
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_LINK_DATA *gls = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(bp->links, gindex);
			if(gls->mode != LINKMODE_GROUP)
			{
				send_to_char("Syntax:  links group {R#{x add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add source generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x add destination generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x add weighted\n\r", ch);
				send_to_char("         links group {R#{x add group\n\r", ch);
				send_to_char("         links group {R#{x from # list\n\r", ch);
				send_to_char("         links group {R#{x from # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group {R#{x from # remove #\n\r", ch);
				send_to_char("         links group {R#{x to # list\n\r", ch);
				send_to_char("         links group {R#{x to # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group {R#{x to # remove #\n\r", ch);
				send_to_char("         links group {R#{x remove #\n\r", ch);

				send_to_char("Please select a {YGROUP{x Link entry.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "list"))
			{
				int count = list_size(gls->group);
				if(count > 0)
				{
					BUFFER *buffer = new_buf();

					ITERATOR git;
					BLUEPRINT_LAYOUT_LINK_DATA *glink;
					BLUEPRINT_WEIGHTED_LINK_DATA *from;
					BLUEPRINT_WEIGHTED_LINK_DATA *to;

					sprintf(buf, "Group Definitions for Group Entry %d:\n\r", gindex);
					add_buf(buffer, buf);
					add_buf(buffer, "{x     [    Mode    ]{x\n\r");
					add_buf(buffer, "{x========================================================{x\n\r");

					int glink_no = 1;
					iterator_start(&git, gls->group);
					while( (glink = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&git)) )
					{
						switch(glink->mode)
						{
							case LINKMODE_STATIC:
							{
								from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->from, 1);
								to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->to, 1);
								sprintf(buf, "%4d  {Y   STATIC   {x\n\r", glink_no++);
								sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
								add_buf(buffer, buf);
								sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
								add_buf(buffer, buf);
								break;
							}

							case LINKMODE_SOURCE:
							{
								ITERATOR wit;
								to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->to, 1);
								sprintf(buf, "%4d  {C   SOURCE   {x\n\r", glink_no++);
								add_buf(buffer, buf);

								int fromlink_no = 1;
								add_buf(buffer, "          Source:\n\r");
								add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
								add_buf(buffer, "          ===================================={x\n\r");
								iterator_start(&wit, glink->from);
								while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
								{
									sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "          ----------------------------------------------------{x\n\r");

								sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
								add_buf(buffer, buf);
								break;
							}

							case LINKMODE_DESTINATION:
							{
								ITERATOR wit;
								from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->from, 1);
								sprintf(buf, "%4d  {C DESTINATION{x\n\r", glink_no++);
								add_buf(buffer, buf);
								sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
								add_buf(buffer, buf);

								int tolink_no = 1;
								add_buf(buffer, "          Destination:\n\r");
								add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
								add_buf(buffer, "          ===================================={x\n\r");
								iterator_start(&wit, glink->to);
								while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
								{
									sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "          ----------------------------------------------------{x\n\r");
								break;
							}

							case LINKMODE_WEIGHTED:
							{
								ITERATOR wit;
								sprintf(buf, "%4d  {C  WEIGHTED  {x\n\r", glink_no++);
								add_buf(buffer, buf);

								int fromlink_no = 1;
								add_buf(buffer, "          Source:\n\r");
								add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
								add_buf(buffer, "          ===================================={x\n\r");
								iterator_start(&wit, glink->from);
								while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
								{
									sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "          ----------------------------------------------------{x\n\r");

								int tolink_no = 1;
								add_buf(buffer, "          Destination:\n\r");
								add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
								add_buf(buffer, "          ===================================={x\n\r");
								iterator_start(&wit, glink->to);
								while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
								{
									sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "          ----------------------------------------------------{x\n\r");
								break;
							}
						}
					}
					iterator_stop(&git);

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
					sprintf(buf, "There are no group definitions defined for Group Entry %d.\n\r", gindex);
					send_to_char(buf, ch);
				}
				return false;
			}

			if (!str_prefix(arg, "clear"))
			{
				if (list_size(gls->group) < 1)
				{
					send_to_char("There are no definitions in the Group Entry.\n\r", ch);
					return false;
				}

				list_clear(gls->group);
				send_to_char("Group definitions clear.\n\r", ch);
				return true;
			}
			
			if (!str_prefix(arg, "add"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  links group # add {Rstatic{x generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # add {Rsource{x generated|ordinal <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # add {Rdestination{x generated|ordinal <from-section> <from-link>\n\r", ch);
					send_to_char("         links group # add {Rweighted{x\n\r", ch);
					return false;
				}

				char arg2[MIL];

				argument = one_argument(argument, arg2);
				if (!str_prefix(arg2, "static"))
				{
					int sections = blueprint_generation_count(bp);
					char arg3[MIL];
					char arg4[MIL];
					char arg5[MIL];
					char arg6[MIL];
					char arg7[MIL];
					sent_bool from_mode = TRISTATE_UNDEF;
					sent_bool to_mode = TRISTATE_UNDEF;

					argument = one_argument(argument, arg3);
					if (!str_prefix(arg3, "generated"))
						from_mode = TRISTATE_FALSE;
					else if(!str_prefix(arg3, "ordinal"))
						from_mode = TRISTATE_TRUE;
					else
					{
						send_to_char("Syntax:  links group # add static {Rgenerated|ordinal{x <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  links group # add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int from_section = atoi(arg4);
					if (from_section < 1 || from_section > sections)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal {R<from-section>{x <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> {R<from-link>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int from_link_no = atoi(arg5);
					if (from_link_no < 1 || from_link_no > link_count)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> {R<from-link>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}
					
					argument = one_argument(argument, arg6);
					if (!str_prefix(arg6, "generated"))
						to_mode = false;
					else if (!str_prefix(arg6, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg7);
					if (!is_number(arg7))
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int to_section = atoi(arg7);
					if (to_section < 1 || to_section > sections)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

					// This is the maximum allowed links based upon the configuration
					link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int to_link_no = atoi(argument);
					if (to_link_no < 1 || to_link_no > link_count)
					{
						send_to_char("Syntax:  links group # add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
					ll->mode = LINKMODE_STATIC;

					BLUEPRINT_WEIGHTED_LINK_DATA *from = new_weighted_random_link();
					from->weight = 1;
					from->section = from_mode ? -from_section : from_section;
					from->link = from_link_no;
					list_appendlink(ll->from, from);
					ll->total_from = 1;

					BLUEPRINT_WEIGHTED_LINK_DATA *to = new_weighted_random_link();
					to->weight = 1;
					to->section = to_mode ? -to_section : to_section;
					to->link = to_link_no;
					list_appendlink(ll->to, to);
					ll->total_to = 1;

					list_appendlink(gls->group, ll);
					sprintf(buf, "Static Link %d added to Group Entry %d.\n\r", list_size(bp->links), gindex);
					send_to_char(buf, ch);
					return true;
				}
				if (!str_prefix(arg2, "source"))
				{
					int sections = blueprint_generation_count(bp);
					char arg3[MIL];
					char arg4[MIL];
					sent_bool to_mode = TRISTATE_UNDEF;

					argument = one_argument(argument, arg3);
					if (!str_prefix(arg3, "generated"))
						to_mode = TRISTATE_FALSE;
					else if(!str_prefix(arg3, "ordinal"))
						to_mode = TRISTATE_TRUE;
					else
					{
						send_to_char("Syntax:  links group # add source {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  links group # add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int to_section = atoi(arg4);
					if (to_section < 1 || to_section > sections)
					{
						send_to_char("Syntax:  links group # add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # add source generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					int to_link_no = atoi(argument);
					if (to_link_no < 1 || to_link_no > link_count)
					{
						send_to_char("Syntax:  links group # add source generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
					ll->mode = LINKMODE_SOURCE;

					BLUEPRINT_WEIGHTED_LINK_DATA *to = new_weighted_random_link();
					to->weight = 1;
					to->section = to_mode ? -to_section : to_section;
					to->link = to_link_no;
					list_appendlink(ll->to, to);
					ll->total_to = 1;

					list_appendlink(gls->group, ll);
					sprintf(buf, "Source Link %d added to Group Entry %d.\n\r", list_size(bp->links), gindex);
					send_to_char(buf, ch);
					return true;
				}
				if (!str_prefix(arg2, "destination"))
				{
					int sections = blueprint_generation_count(bp);
					char arg3[MIL];
					char arg4[MIL];
					sent_bool from_mode = TRISTATE_UNDEF;

					argument = one_argument(argument, arg3);
					if (!str_prefix(arg3, "generated"))
						from_mode = TRISTATE_FALSE;
					else if(!str_prefix(arg3, "ordinal"))
						from_mode = TRISTATE_TRUE;
					else
					{
						send_to_char("Syntax:  links group # add destination {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if (!is_number(arg4))
					{
						send_to_char("Syntax:  links group # add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int from_section = atoi(arg4);
					if (from_section < 1 || from_section > sections)
					{
						send_to_char("Syntax:  links group # add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);

					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # add destination generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  links group # add destination generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int from_link_no = atoi(argument);
					if (from_link_no < 1 || from_link_no > link_count)
					{
						send_to_char("Syntax:  links group # add destination generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_LINK_DATA *ll = new_blueprint_layout_link_data();
					ll->mode = LINKMODE_DESTINATION;

					BLUEPRINT_WEIGHTED_LINK_DATA *from = new_weighted_random_link();
					from->weight = 1;
					from->section = from_mode ? -from_section : from_section;
					from->link = from_link_no;
					list_appendlink(ll->from, from);
					ll->total_from = 1;

					list_appendlink(gls->group, ll);
					sprintf(buf, "Destination Link %d added to Group Entry %d.\n\r", list_size(bp->links), gindex);
					send_to_char(buf, ch);
					return true;
				}
				if (!str_prefix(arg2, "weighted"))
				{
					BLUEPRINT_LAYOUT_LINK_DATA *link = new_blueprint_layout_link_data();
					link->mode = LINKMODE_WEIGHTED;
					link->total_from = 0;
					link->total_to = 0;

					list_appendlink(gls->group, link);
					sprintf(buf, "Weighted Link %d added to Group Entry %d.\n\r", list_size(bp->links),gindex);
					send_to_char(buf, ch);
					return true;
				}

				send_to_char("Syntax:  links group # add {Rstatic{x generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group # add {Rsource{x generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group # add {Rdestination{x generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group # add {Rweighted{x\n\r", ch);
				return false;
			}
			
			if (!str_prefix(arg, "from"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);
				if(!is_number(arg2))
				{
					send_to_char("Syntax:  links group # from {R#{x list\n\r", ch);
					send_to_char("         links group # from {R#{x add <weight> <from-section> <from-link>\n\r", ch);
					send_to_char("         links group # from {R#{x set # <weight> <from-section> <from-link>\n\r", ch);
					send_to_char("         links group # from {R#{x remove #\n\r", ch);

					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg2);
				if(index < 1 || index > list_size(gls->group))
				{
					send_to_char("Syntax:  links group # from {R#{x list\n\r", ch);
					send_to_char("         links group # from {R#{x add <weight> <from-section> <from-link>\n\r", ch);
					send_to_char("         links group # from {R#{x set # <weight> <from-section> <from-link>\n\r", ch);
					send_to_char("         links group # from {R#{x remove #\n\r", ch);

					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->group));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_LINK_DATA *ll = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(gls->group, index);

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "list"))
				{
					if (list_size(ll->from) > 0)
					{
						BUFFER *buffer = new_buf();

						sprintf(buf, "From Table Entries for Sublink %d from Group Link %d:\n\r", index, gindex);
						add_buf(buffer, buf);

						add_buf(buffer, "     [ Weight ] [ Section ] [ Link ]\n\r");
						add_buf(buffer, "=====================================\n\r");

						int from_no = 1;
						ITERATOR it;
						BLUEPRINT_WEIGHTED_LINK_DATA *weighted;
						iterator_start(&it, ll->from);
						while((weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)))
						{
							sprintf(buf, "%4d   %6d     {%c%7d{x      %4d\n\r", from_no++, weighted->weight,
								(weighted->section<0?'G':(weighted->section>0?'Y':'W')),
								abs(weighted->section), weighted->link);
							add_buf(buffer, buf);
						}
						iterator_stop(&it);

						add_buf(buffer, "-------------------------------------\n\r");
						add_buf(buffer, "{YYELLOW{x - Generated section position index for Source/Destination sections\n\r");
						add_buf(buffer, "{GGREEN{x  - Ordinal section position index for Source/Destination sections\n\r");

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
						send_to_char("There are no From table definitions.\n\r", ch);
					}
					return false;
				}

				if (!str_prefix(arg3, "add"))
				{
					int sections = blueprint_generation_count(bp);
					sent_bool from_mode = TRISTATE_UNDEF;
					char arg4[MIL];
					char arg5[MIL];
					char arg6[MIL];

					if (ll->mode != LINKMODE_SOURCE && ll->mode != LINKMODE_WEIGHTED)
					{
						send_to_char("Syntax:  links group # from {R#{x list\n\r", ch);
						send_to_char("         links group # from {R#{x add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("         links group # from {R#{x set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("         links group # from {R#{x remove #\n\r", ch);

						send_to_char("Only able to change the From table on {YSOURCE{x or {YWEIGHTED{x Links.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if(!is_number(arg4))
					{
						send_to_char("Syntax:  links group # from # add {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg4);
					if (weight < 1)
					{
						send_to_char("Syntax:  links group # from # add {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if(!str_prefix(arg5, "generated"))
						from_mode = false;
					else if(!str_prefix(arg5, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  links group # from # add <weight> {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if(!is_number(arg6))
					{
						send_to_char("Syntax:  links group # from # add <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int from_section = atoi(arg6);
					if(from_section < 1 || from_section > sections)
					{
						send_to_char("Syntax:  links group # from # add <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  links group # from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int from_link_no = atoi(arg5);
					if (from_link_no < 1 || from_link_no > link_count)
					{
						send_to_char("Syntax:  links group # from # add <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = new_weighted_random_link();
					weighted->weight = weight;
					weighted->section = from_section;
					weighted->link = from_link_no;
					list_appendlink(ll->from, weighted);
					ll->total_from += weight;

					char *mode = "Link";
					if (ll->mode == LINKMODE_SOURCE)
						mode = "Source Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Added From Entry %d to %s %d in Group Entry %d.\n\r", list_size(ll->from), mode, index, gindex);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "set"))
				{
					int sections = blueprint_generation_count(bp);
					sent_bool from_mode = TRISTATE_UNDEF;
					char argw[MIL];
					char arg4[MIL];
					char arg5[MIL];
					char arg6[MIL];

					argument = one_argument(argument, argw);
					if(!is_number(argw))
					{
						send_to_char("Syntax:  links group # from # set {R#{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(argw);
					if(windex < 1 || windex > list_size(ll->from))
					{
						send_to_char("Syntax:  links group # from # set {R#{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if(!is_number(arg4))
					{
						send_to_char("Syntax:  links group # from # set # {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg4);
					if (weight < 1)
					{
						send_to_char("Syntax:  links group # from # set # {R<weight>{x generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if(!str_prefix(arg5, "generated"))
						from_mode = false;
					else if(!str_prefix(arg5, "ordinal"))
						from_mode = true;
					else
					{
						send_to_char("Syntax:  links group # from # set # <weight> {Rgenerated|ordinal{x <from-section> <from-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if(!is_number(arg6))
					{
						send_to_char("Syntax:  links group # from # set # <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int from_section = atoi(arg6);
					if(from_section < 1 || from_section > sections)
					{
						send_to_char("Syntax:  links group # from # set # <weight> generated|ordinal {R<from-section>{x <from-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (from_mode ? -from_section : from_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  links group # from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int from_link_no = atoi(arg5);
					if (from_link_no < 1 || from_link_no > link_count)
					{
						send_to_char("Syntax:  links group # from # set # <weight> generated|ordinal <from-section> {R<from-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->from, windex);
					ll->total_from -= weighted->weight;
					weighted->weight = weight;
					weighted->section = from_section;
					weighted->link = from_link_no;
					ll->total_from += weight;

					char *mode = "Link";
					if (ll->mode == LINKMODE_SOURCE)
						mode = "Source Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Updated Entry %d to %s %d.\n\r", windex, mode, index);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "remove"))
				{
					if (ll->mode != LINKMODE_SOURCE && ll->mode != LINKMODE_WEIGHTED)
					{
						send_to_char("Syntax:  links group # from {R#{x list\n\r", ch);
						send_to_char("         links group # from {R#{x add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("         links group # from {R#{x set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
						send_to_char("         links group # from {R#{x remove #\n\r", ch);

						send_to_char("Only able to change the From table on {YSOURCE{x or {YWEIGHTED{x Links.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  links group # from # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(argument);
					if (windex < 1 || windex > list_size(ll->from))
					{
						send_to_char("Syntax:  links group # from # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->from));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->from, windex);
					ll->total_from -= weighted->weight;
					list_remnthlink(ll->from, windex, true);

					char *mode = "Link";
					if (ll->mode == LINKMODE_SOURCE)
						mode = "Source Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Removed From Entry %d from %s %d in Group Entry %d.\n\r", windex, mode, index, gindex);
					send_to_char(buf, ch);
					return true;
				}

				send_to_char("Syntax:  links group # from # {Rlist{x\n\r", ch);
				send_to_char("         links group # from # {Radd{x <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group # from # {Rset{x # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
				send_to_char("         links group # from # {Rremove{x #\n\r", ch);
				return false;
			}

			if (!str_prefix(arg, "to"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);
				if(!is_number(arg2))
				{
					send_to_char("Syntax:  links group # to {R#{x list\n\r", ch);
					send_to_char("         links group # to {R#{x add <weight> <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # to {R#{x set # <weight> <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # to {R#{x remove #\n\r", ch);

					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->group));
					send_to_char(buf, ch);
					return false;
				}

				int index = atoi(arg2);
				if(index < 1 || index > list_size(gls->group))
				{
					send_to_char("Syntax:  links group # to {R#{x list\n\r", ch);
					send_to_char("         links group # to {R#{x add <weight> <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # to {R#{x set # <weight> <to-section> <to-link>\n\r", ch);
					send_to_char("         links group # to {R#{x remove #\n\r", ch);

					sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(gls->group));
					send_to_char(buf, ch);
					return false;
				}

				BLUEPRINT_LAYOUT_LINK_DATA *ll = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(gls->group, index);

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "list"))
				{
					if (list_size(ll->to) > 0)
					{
						BUFFER *buffer = new_buf();

						sprintf(buf, "To Table Entries for Sublink %d in Group Link %d:\n\r", index, gindex);
						add_buf(buffer, buf);

						add_buf(buffer, "     [ Weight ] [ Section ] [ Link ]\n\r");
						add_buf(buffer, "=====================================\n\r");

						int to_no = 1;
						ITERATOR it;
						BLUEPRINT_WEIGHTED_LINK_DATA *weighted;
						iterator_start(&it, ll->to);
						while((weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)))
						{
							sprintf(buf, "%4d   %6d     {%c%7d{x      %4d\n\r", to_no++, weighted->weight,
								(weighted->section<0?'G':(weighted->section>0?'Y':'W')),
								abs(weighted->section), weighted->link);
							add_buf(buffer, buf);
						}
						iterator_stop(&it);

						add_buf(buffer, "-------------------------------------\n\r");
						add_buf(buffer, "{YYELLOW{x - Generated section position index for Source/Destination sections\n\r");
						add_buf(buffer, "{GGREEN{x  - Ordinal section position index for Source/Destination sections\n\r");

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
						send_to_char("There are no To table definitions.\n\r", ch);
					}
					return false;
				}

				if (!str_prefix(arg3, "add"))
				{
					int sections = blueprint_generation_count(bp);
					sent_bool to_mode = TRISTATE_UNDEF;
					char arg4[MIL];
					char arg5[MIL];
					char arg6[MIL];

					if (ll->mode != LINKMODE_DESTINATION && ll->mode != LINKMODE_WEIGHTED)
					{
						send_to_char("Syntax:  links group # to {R#{x list\n\r", ch);
						send_to_char("         links group # to {R#{x add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("         links group # to {R#{x set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("         links group # to {R#{x remove #\n\r", ch);

						send_to_char("Only able to change the To table on {YDESTINATION{x or {YWEIGHTED{x Links.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if(!is_number(arg4))
					{
						send_to_char("Syntax:  links group # to # add {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg4);
					if (weight < 1)
					{
						send_to_char("Syntax:  links group # to # add {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if(!str_prefix(arg5, "generated"))
						to_mode = false;
					else if(!str_prefix(arg5, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  links group # to # add <weight> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if(!is_number(arg6))
					{
						send_to_char("Syntax:  links group # to # add <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int to_section = atoi(arg6);
					if(to_section < 1 || to_section > sections)
					{
						send_to_char("Syntax:  links group # to # add <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  links group # to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int to_link_no = atoi(arg5);
					if (to_link_no < 1 || to_link_no > link_count)
					{
						send_to_char("Syntax:  links group # to # add <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = new_weighted_random_link();
					weighted->weight = weight;
					weighted->section = to_section;
					weighted->link = to_link_no;
					list_appendlink(ll->to, weighted);
					ll->total_to += weight;

					char *mode = "Link";
					if (ll->mode == LINKMODE_DESTINATION)
						mode = "Destination Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Added To Entry %d to %s %d in Group Entry %d.\n\r", list_size(ll->to), mode, index, gindex);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "set"))
				{
					int sections = blueprint_generation_count(bp);
					sent_bool to_mode = TRISTATE_UNDEF;
					char argw[MIL];
					char arg4[MIL];
					char arg5[MIL];
					char arg6[MIL];

					argument = one_argument(argument, argw);
					if(!is_number(argw))
					{
						send_to_char("Syntax:  links group # to # set {R#{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(argw);
					if(windex < 1 || windex > list_size(ll->to))
					{
						send_to_char("Syntax:  links group # to # set {R#{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
						send_to_char(buf, ch);
						return false;
					}

					argument = one_argument(argument, arg4);
					if(!is_number(arg4))
					{
						send_to_char("Syntax:  links group # to # set # {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					int weight = atoi(arg4);
					if (weight < 1)
					{
						send_to_char("Syntax:  links group # to # set # {R<weight>{x generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if(!str_prefix(arg5, "generated"))
						to_mode = false;
					else if(!str_prefix(arg5, "ordinal"))
						to_mode = true;
					else
					{
						send_to_char("Syntax:  links group # to # set # <weight> {Rgenerated|ordinal{x <to-section> <to-link>\n\r", ch);
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg6);
					if(!is_number(arg6))
					{
						send_to_char("Syntax:  links group # to # set # <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					int to_section = atoi(arg6);
					if(to_section < 1 || to_section > sections)
					{
						send_to_char("Syntax:  links group # to # set # <weight> generated|ordinal {R<to-section>{x <to-link>\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_LAYOUT_SECTION_DATA *group;
					BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (to_mode ? -to_section : to_section), &group);

					// This is the maximum allowed links based upon the configuration
					int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
					if (link_count < 1)
					{
						send_to_char("Syntax:  links group # to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						send_to_char("No links defined for this section reference.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg5);
					if (!is_number(arg5))
					{
						send_to_char("Syntax:  links group # to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					int to_link_no = atoi(arg5);
					if (to_link_no < 1 || to_link_no > link_count)
					{
						send_to_char("Syntax:  links group # to # set # <weight> generated|ordinal <to-section> {R<to-link>{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->to, windex);
					ll->total_to -= weighted->weight;
					weighted->weight = weight;
					weighted->section = to_section;
					weighted->link = to_link_no;
					ll->total_to += weight;

					char *mode = "Link";
					if (ll->mode == LINKMODE_SOURCE)
						mode = "Source Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Updated To Entry %d to %s %d in Group Link %d.\n\r", windex, mode, index, gindex);
					send_to_char(buf, ch);
					return true;
				}

				if (!str_prefix(arg3, "remove"))
				{
					if (ll->mode != LINKMODE_DESTINATION && ll->mode != LINKMODE_WEIGHTED)
					{
						send_to_char("Syntax:  links group # to {R#{x list\n\r", ch);
						send_to_char("         links group # to {R#{x add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("         links group # to {R#{x set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
						send_to_char("         links group # to {R#{x remove #\n\r", ch);

						send_to_char("Only able to change the To table on {YDESTINATION{x or {YWEIGHTED{x Links.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("Syntax:  links group # to # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
						send_to_char(buf, ch);
						return false;
					}

					int windex = atoi(argument);
					if (windex < 1 || windex > list_size(ll->to))
					{
						send_to_char("Syntax:  links group # to # remove {R#{x\n\r", ch);
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(ll->to));
						send_to_char(buf, ch);
						return false;
					}

					BLUEPRINT_WEIGHTED_LINK_DATA *weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(ll->to, windex);
					ll->total_to -= weighted->weight;
					list_remnthlink(ll->to, windex, true);

					char *mode = "Link";
					if (ll->mode == LINKMODE_DESTINATION)
						mode = "Destination Link";
					else if (ll->mode == LINKMODE_WEIGHTED)
						mode = "Weighted Link";
					sprintf(buf, "Removed To Entry %d from %s %d in Group Entry %d.\n\r", windex, mode, index, gindex);
					send_to_char(buf, ch);
					return true;
				}

				send_to_char("Syntax:  links group # to # {Rlist{x\n\r", ch);
				send_to_char("         links group # to # {Radd{x <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group # to # {Rset{x # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
				send_to_char("         links group # to # {Rremove{x #\n\r", ch);
				return false;
			}


			send_to_char("         links group # {Radd{x static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links group # {Radd{x source generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links group # {Radd{x destination generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links group # {Radd{x weighted\n\r", ch);
			send_to_char("         links group # {Rfrom{x # list\n\r", ch);
			send_to_char("         links group # {Rfrom{x # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links group # {Rfrom{x # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
			send_to_char("         links group # {Rfrom{x # remove #\n\r", ch);
			send_to_char("         links group # {Rto{x # list\n\r", ch);
			send_to_char("         links group # {Rto{x # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links group # {Rto{x # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
			send_to_char("         links group # {Rto{x # remove #\n\r", ch);
			send_to_char("         links group # {Rremove{x #\n\r", ch);
			return false;
		}
	}

	send_to_char("Syntax:  links {Rlist{x\n\r", ch);
	send_to_char("         links {Rclear{x\n\r", ch);
	send_to_char("         links {Radd{x static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Radd{x source generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Radd{x destination generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Radd{x weighted\n\r", ch);
	send_to_char("         links {Radd{x group\n\r", ch);
	send_to_char("         links {Rfrom{x # list\n\r", ch);
	send_to_char("         links {Rfrom{x # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Rfrom{x # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Rfrom{x # remove #\n\r", ch);
	send_to_char("         links {Rto{x # list\n\r", ch);
	send_to_char("         links {Rto{x # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rto{x # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rto{x # remove #\n\r", ch);
	send_to_char("         links {Rgroup{x # add static generated|ordinal <from-section> <from-link> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # add source generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # add destination generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # add weighted\n\r", ch);
	send_to_char("         links {Rgroup{x # add group\n\r", ch);
	send_to_char("         links {Rgroup{x # from # list\n\r", ch);
	send_to_char("         links {Rgroup{x # from # add <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # from # set # <weight> generated|ordinal <from-section> <from-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # from # remove #\n\r", ch);
	send_to_char("         links {Rgroup{x # to # list\n\r", ch);
	send_to_char("         links {Rgroup{x # to # add <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # to # set # <weight> generated|ordinal <to-section> <to-link>\n\r", ch);
	send_to_char("         links {Rgroup{x # to # remove #\n\r", ch);
	send_to_char("         links {Rgroup{x # remove #\n\r", ch);
	send_to_char("         links {Rremove{x #\n\r", ch);
	return false;
}

BPEDIT( bpedit_entrances )
{
	BLUEPRINT *bp;
	char arg[MIL];
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit blueprint entrances in OLC.\n\r", ch);
		return false;
	}

	if (argument[0] != '\0')
	{
		argument = one_argument(argument, arg);

		if(!str_prefix(arg, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "{xEntrances:\n\r");
			if(list_size(bp->entrances) > 0)
			{
				add_buf(buffer, "      [         Name         ] [ Section ] [               Room               ]\n\r");
				add_buf(buffer, "================================================================================\n\r");
				ITERATOR bxit;
				BLUEPRINT_EXIT_DATA *bex;
				int bxindex = 1;
				bool approx_msg = false;
				iterator_start(&bxit, bp->entrances);
				while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
				{
					bool exact = false;
					BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, bex->section, &exact);

					BLUEPRINT_LINK *link = get_section_link(bs, bex->link);

					ROOM_INDEX_DATA *room = IS_VALID(bs) && valid_section_link(link) ? get_room_index(bs->area, link->vnum) : NULL;

					if (room)
						sprintf(buf, "%4d    %-20.20s     {%c%7d{x     (%-4ld) %s (%s) %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), room->vnum, room->name, dir_name[link->door], (exact ? "" : "{M**{x"));
					else
						sprintf(buf, "%4d    %-20.20s     {%c%7d{x     %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), "???");
					add_buf(buffer, buf);

					if (room && !exact)
						approx_msg = true;
				}
				iterator_stop(&bxit);
				add_buf(buffer, "--------------------------------------------------------------------------------\n\r");

				if (approx_msg)
					add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
			}
			else
			{
				add_buf(buffer, "    none\n\r");
			}

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

		if(!str_prefix(arg, "clear"))
		{
			if (list_size(bp->entrances) < 1)
			{
				send_to_char("There are no entrances to remove.\n\r", ch);
				return false;
			}

			list_clear(bp->entrances);
			send_to_char("Blueprint entrances cleared.\n\r", ch);
			return true;
		}

		if(!str_prefix(arg, "add"))
		{
			int sections = blueprint_generation_count(bp);
			sent_bool mode = TRISTATE_UNDEF;
			char arg2[MIL];	// name
			char arg3[MIL]; // mode
			char arg4[MIL]; // section
			// argument = link

			argument = one_argument(argument, arg2);
			if (arg2[0] == '\0')
			{
				send_to_char("Syntax:  entrances add {R<name>{x generated|ordinal <section> <link>\n\r", ch);
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "generated"))
				mode = false;
			else if (!str_prefix(arg3, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Syntax:  entrances add <name> {Rgenerated|ordinal{x <section> <link>\n\r", ch);
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Syntax:  entrances add <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(arg4);
			if (section_no < 1 || section_no > sections)
			{
				send_to_char("Syntax:  entrances add <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *group;
			BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);

			// This is the maximum allowed links based upon the configuration
			int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
			if (!is_number(argument))
			{
				send_to_char("Syntax:  entrances add <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			int link_no = atoi(argument);
			if (link_no < 1 || link_no > link_count)
			{
				send_to_char("Syntax:  entrances add <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			if(mode)
				section_no = -section_no;

			// Make sure this section-link is not duplicated in the entrances AND exits
			bool found = false;
			ITERATOR it;
			BLUEPRINT_EXIT_DATA *x;
			iterator_start(&it, bp->entrances);
			while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
			{
				if(x->section == section_no && x->link == link_no)
				{
					found = true;
					break;
				}
			}
			iterator_stop(&it);
			if (!found)
			{
				iterator_start(&it, bp->exits);
				while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
				{
					if(x->section == section_no && x->link == link_no)
					{
						found = true;
						break;
					}
				}
				iterator_stop(&it);
			}

			if(found)
			{
				send_to_char("Section-Link pair already used as an entrance or an exit.\n\r", ch);
				return false;
			}

			BLUEPRINT_EXIT_DATA *bex = new_blueprint_exit_data();
			free_string(bex->name);
			bex->name = str_dup(arg2);
			bex->section = section_no;
			bex->link = link_no;

			list_appendlink(bp->entrances, bex);
			sprintf(buf, "Added Entrance %d to Blueprint Entrances.\n\r", list_size(bp->entrances));
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg, "set"))
		{
			int sections = blueprint_generation_count(bp);
			sent_bool mode = TRISTATE_UNDEF;
			char argn[MIL];	// index
			char arg2[MIL];	// name
			char arg3[MIL]; // mode
			char arg4[MIL]; // section
			// argument = link

			argument = one_argument(argument, argn);
			if (!is_number(argn))
			{
				send_to_char("Syntax:  entrances set {R#{x <name> generated|ordinal <section> <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->entrances));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argn);
			if (index < 1 || index > list_size(bp->entrances))
			{
				send_to_char("Syntax:  entrances set {R#{x <name> generated|ordinal <section> <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->entrances));
				send_to_char(buf, ch);
				return false;
			}

			argument = one_argument(argument, arg2);
			if (arg2[0] == '\0')
			{
				send_to_char("Syntax:  entrances set # {R<name>{x generated|ordinal <section> <link>\n\r", ch);
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "generated"))
				mode = false;
			else if (!str_prefix(arg3, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Syntax:  entrances set # <name> {Rgenerated|ordinal{x <section> <link>\n\r", ch);
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Syntax:  entrances set # <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(arg4);
			if (section_no < 1 || section_no > sections)
			{
				send_to_char("Syntax:  entrances set # <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *group;
			BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);

			// This is the maximum allowed links based upon the configuration
			int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
			if (!is_number(argument))
			{
				send_to_char("Syntax:  entrances set # <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			int link_no = atoi(argument);
			if (link_no < 1 || link_no > link_count)
			{
				send_to_char("Syntax:  entrances set # <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			if(mode)
				section_no = -section_no;

			// Make sure this section-link is not duplicated in the entrances AND exits
			bool found = false;
			int entry_no = 1;
			ITERATOR it;
			BLUEPRINT_EXIT_DATA *x;
			iterator_start(&it, bp->entrances);
			while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
			{
				if(entry_no != index && x->section == section_no && x->link == link_no)
				{
					found = true;
					break;
				}

				entry_no++;
			}
			iterator_stop(&it);
			if (!found)
			{
				iterator_start(&it, bp->exits);
				while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
				{
					if(x->section == section_no && x->link == link_no)
					{
						found = true;
						break;
					}
				}
				iterator_stop(&it);
			}

			if(found)
			{
				send_to_char("Section-Link pair already used as an entrance or an exit.\n\r", ch);
				return false;
			}

			BLUEPRINT_EXIT_DATA *bex = (BLUEPRINT_EXIT_DATA *)list_nthdata(bp->entrances, index);
			free_string(bex->name);
			bex->name = str_dup(arg2);
			bex->section = section_no;
			bex->link = link_no;

			sprintf(buf, "Updated Entrance %d in Blueprint Entrances.\n\r", index);
			send_to_char(buf, ch);
			return true;

			return true;
		}

		if (!str_prefix(arg, "remove"))
		{
			if (list_size(bp->entrances) < 1)
			{
				send_to_char("There are no blueprint entrances defined.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  entrances remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->entrances));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bp->entrances))
			{
				send_to_char("Syntax:  entrances remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->entrances));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bp->entrances, index, true);
			sprintf(buf, "Removed Blueprint Entrance %d.\n\r", index);
			send_to_char(buf, ch);
			return true;
		}
	}

	send_to_char("Syntax:  entrances {Rlist{x\n\r", ch);
	send_to_char("         entrances {Rclear{x\n\r", ch);
	send_to_char("         entrances {Radd{x <name> generated|ordinal <section> <link>\n\r", ch);
	send_to_char("         entrances {Rset{x # <name> generated|ordinal <section> <link>\n\r", ch);
	send_to_char("         entrances {Rremove{x #\n\r", ch);
	return false;
}

BPEDIT( bpedit_exits )
{
	BLUEPRINT *bp;
	char arg[MIL];
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit blueprint exits in OLC.\n\r", ch);
		return false;
	}

	if (argument[0] != '\0')
	{
		argument = one_argument(argument, arg);

		if(!str_prefix(arg, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "{xExits:\n\r");
			if(list_size(bp->exits) > 0)
			{
				add_buf(buffer, "      [         Name         ] [ Section ] [               Room               ]\n\r");
				add_buf(buffer, "================================================================================\n\r");
				ITERATOR bxit;
				BLUEPRINT_EXIT_DATA *bex;
				int bxindex = 1;
				bool approx_msg = false;
				iterator_start(&bxit, bp->exits);
				while( (bex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&bxit)) )
				{
					bool exact = false;
					BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, bex->section, &exact);

					BLUEPRINT_LINK *link = get_section_link(bs, bex->link);

					ROOM_INDEX_DATA *room = IS_VALID(bs) && valid_section_link(link) ? get_room_index(bs->area, link->vnum) : NULL;

					if (room)
						sprintf(buf, "%4d    %-20.20s     {%c%7d{x     (%-4ld) %s (%s) %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), room->vnum, room->name, dir_name[link->door], (exact ? "" : "{M**{x"));
					else
						sprintf(buf, "%4d    %-20.20s     {%c%7d{x     %s\n\r", bxindex++, bex->name, (bex->section<0?'G':(bex->section>0?'Y':'W')), abs(bex->section), "???");
					add_buf(buffer, buf);

					if (room && !exact)
						approx_msg = true;
				}
				iterator_stop(&bxit);
				add_buf(buffer, "--------------------------------------------------------------------------------\n\r");

				if (approx_msg)
					add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
			}
			else
			{
				add_buf(buffer, "    none\n\r");
			}

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

		if(!str_prefix(arg, "clear"))
		{
			if (list_size(bp->exits) < 1)
			{
				send_to_char("There are no exits to remove.\n\r", ch);
				return false;
			}

			list_clear(bp->exits);
			send_to_char("Blueprint exits cleared.\n\r", ch);
			return true;
		}

		if(!str_prefix(arg, "add"))
		{
			int sections = blueprint_generation_count(bp);
			sent_bool mode = TRISTATE_UNDEF;
			char arg2[MIL];	// name
			char arg3[MIL]; // mode
			char arg4[MIL]; // section
			// argument = link

			argument = one_argument(argument, arg2);
			if (arg2[0] == '\0')
			{
				send_to_char("Syntax:  exits add {R<name>{x generated|ordinal <section> <link>\n\r", ch);
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "generated"))
				mode = false;
			else if (!str_prefix(arg3, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Syntax:  exits add <name> {Rgenerated|ordinal{x <section> <link>\n\r", ch);
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Syntax:  exits add <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(arg4);
			if (section_no < 1 || section_no > sections)
			{
				send_to_char("Syntax:  exits add <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *group;
			BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);

			// This is the maximum allowed links based upon the configuration
			int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
			if (!is_number(argument))
			{
				send_to_char("Syntax:  exits add <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			int link_no = atoi(argument);
			if (link_no < 1 || link_no > link_count)
			{
				send_to_char("Syntax:  exits add <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			if(mode)
				section_no = -section_no;

			// Make sure this section-link is not duplicated in the exits AND entrances
			bool found = false;
			ITERATOR it;
			BLUEPRINT_EXIT_DATA *x;
			iterator_start(&it, bp->exits);
			while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
			{
				if(x->section == section_no && x->link == link_no)
				{
					found = true;
					break;
				}
			}
			iterator_stop(&it);
			if (!found)
			{
				iterator_start(&it, bp->entrances);
				while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
				{
					if(x->section == section_no && x->link == link_no)
					{
						found = true;
						break;
					}
				}
				iterator_stop(&it);
			}

			if(found)
			{
				send_to_char("Section-Link pair already used as an entrance or an exit.\n\r", ch);
				return false;
			}

			BLUEPRINT_EXIT_DATA *bex = new_blueprint_exit_data();
			free_string(bex->name);
			bex->name = str_dup(arg2);
			bex->section = section_no;
			bex->link = link_no;

			list_appendlink(bp->exits, bex);
			sprintf(buf, "Added Exit %d to Blueprint Exits.\n\r", list_size(bp->exits));
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg, "set"))
		{
			int sections = blueprint_generation_count(bp);
			sent_bool mode = TRISTATE_UNDEF;
			char argn[MIL];	// index
			char arg2[MIL];	// name
			char arg3[MIL]; // mode
			char arg4[MIL]; // section
			// argument = link

			argument = one_argument(argument, argn);
			if (!is_number(argn))
			{
				send_to_char("Syntax:  exits set {R#{x <name> generated|ordinal <section> <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->exits));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argn);
			if (index < 1 || index > list_size(bp->exits))
			{
				send_to_char("Syntax:  exits set {R#{x <name> generated|ordinal <section> <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->exits));
				send_to_char(buf, ch);
				return false;
			}

			argument = one_argument(argument, arg2);
			if (arg2[0] == '\0')
			{
				send_to_char("Syntax:  exits set # {R<name>{x generated|ordinal <section> <link>\n\r", ch);
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "generated"))
				mode = false;
			else if (!str_prefix(arg3, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Syntax:  exits set # <name> {Rgenerated|ordinal{x <section> <link>\n\r", ch);
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Syntax:  exits set # <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(arg4);
			if (section_no < 1 || section_no > sections)
			{
				send_to_char("Syntax:  exits set # <name> generated|ordinal {R<section>{x <link>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *group;
			BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);

			// This is the maximum allowed links based upon the configuration
			int link_count = blueprint_layout_links_count(bp, group ? group : ls, 0);
			if (!is_number(argument))
			{
				send_to_char("Syntax:  exits set # <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			int link_no = atoi(argument);
			if (link_no < 1 || link_no > link_count)
			{
				send_to_char("Syntax:  exits set # <name> generated|ordinal <section> {R<link>{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", link_count);
				send_to_char(buf, ch);
				return false;
			}

			if(mode)
				section_no = -section_no;

			// Make sure this section-link is not duplicated in the exits AND entrances
			bool found = false;
			int exit_no = 1;
			ITERATOR it;
			BLUEPRINT_EXIT_DATA *x;
			iterator_start(&it, bp->exits);
			while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
			{
				if(exit_no != index && x->section == section_no && x->link == link_no)
				{
					found = true;
					break;
				}

				exit_no++;
			}
			iterator_stop(&it);
			if (!found)
			{
				iterator_start(&it, bp->entrances);
				while((x = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&it)))
				{
					if(x->section == section_no && x->link == link_no)
					{
						found = true;
						break;
					}
				}
				iterator_stop(&it);
			}

			if(found)
			{
				send_to_char("Section-Link pair already used as an entrance or an exit.\n\r", ch);
				return false;
			}

			BLUEPRINT_EXIT_DATA *bex = (BLUEPRINT_EXIT_DATA *)list_nthdata(bp->exits, index);
			free_string(bex->name);
			bex->name = str_dup(arg2);
			bex->section = section_no;
			bex->link = link_no;

			sprintf(buf, "Updated Exit %d in Blueprint Exits.\n\r", index);
			send_to_char(buf, ch);
			return true;

			return true;
		}

		if (!str_prefix(arg, "remove"))
		{
			if (list_size(bp->exits) < 1)
			{
				send_to_char("There are no blueprint exits defined.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  exits remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->exits));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bp->exits))
			{
				send_to_char("Syntax:  exits remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->exits));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bp->exits, index, true);
			sprintf(buf, "Removed Blueprint Exit %d.\n\r", index);
			send_to_char(buf, ch);
			return true;
		}
	}

	send_to_char("Syntax:  exits {Rlist{x\n\r", ch);
	send_to_char("         exits {Rclear{x\n\r", ch);
	send_to_char("         exits {Radd{x <name> generated|ordinal <section> <link>\n\r", ch);
	send_to_char("         exits {Rset{x # <name> generated|ordinal <section> <link>\n\r", ch);
	send_to_char("         exits {Rremove{x #\n\r", ch);
	return false;
}

BPEDIT( bpedit_recall )
{
	BLUEPRINT *bp;
	char arg[MIL];
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit blueprint recall in OLC.\n\r", ch);
		return false;
	}

	if (!str_prefix(argument, "none"))
	{
		bp->recall = 0;
		send_to_char("Blueprint Recall cleared.\n\r", ch);
	}
	else
	{
		argument = one_argument(argument, arg);
		sent_bool mode = TRISTATE_UNDEF;
		if (!str_prefix(arg, "generated"))
			mode = false;
		else if (!str_prefix(arg, "ordinal"))
			mode = true;
		else
		{
			send_to_char("Syntax:  recall {Rgenerated|ordinal{x <section>\n\r", ch);
			send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
			return false;
		}

		int sections = blueprint_generation_count(bp);
		if (!is_number(argument))
		{
			send_to_char("Syntax:  recall generated|ordinal {R<section>{x\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
			send_to_char(buf, ch);
			return false;
		}

		int section_no = atoi(argument);
		if (section_no < 1 || section_no > sections)
		{
			send_to_char("Syntax:  recall generated|ordinal {R<section>{x\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
			send_to_char(buf, ch);
			return false;
		}

		// Check that the desired section (or possible sections) have a defined recall.
		BLUEPRINT_LAYOUT_SECTION_DATA *group;
		BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);
		if (mode)
		{
			// We are ordinal, so we can check the explicit layout section
			// Note: GROUP sections will not come back in 'ls'
			if (!blueprint_layout_has_recall(bp, ls))
			{
				if(ls->mode == SECTIONMODE_WEIGHTED)
				{
					send_to_char("At least one of the sections in the weighted table does not have a recall defined.\n\r", ch);
				}
				else
					send_to_char("That section definition does not have a recall defined.\n\r", ch);

				return false;
			}
		}
		else if (group)
		{
			if (!blueprint_layout_has_recall(bp, group))
			{
				send_to_char("At least one of the section definitions in the group section does not have a recall defined.\n\r", ch);
				return false;
			}
		}
		else if (!blueprint_layout_has_recall(bp, ls))
		{
			if(ls->mode == SECTIONMODE_WEIGHTED)
			{
				send_to_char("At least one of the sections in the weighted table does not have a recall defined.\n\r", ch);
			}
			else
				send_to_char("That section definition does not have a recall defined.\n\r", ch);

			return false;
		}

		bp->recall = (mode ? -section_no : section_no);
		send_to_char("Blueprint Recall set.\n\r", ch);
	}
	return true;
}

BPEDIT( bpedit_rooms )
{
	BLUEPRINT *bp;
	char arg[MIL];
	char buf[MSL];

	EDIT_BLUEPRINT(ch, bp);

	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		send_to_char("Blueprint is in Scripted Layout Mode.  Cannot edit special rooms in OLC.\n\r", ch);
		return false;
	}

	if (argument[0] != '\0')
	{
		argument = one_argument(argument, arg);
		
		if (!str_prefix(arg, "list"))
		{
			BUFFER *buffer = new_buf();

			add_buf(buffer, "Special Rooms:\n\r");
			if (list_size(bp->special_rooms) > 0)
			{
				BLUEPRINT_SPECIAL_ROOM *special;

				char buf[MSL];
				int line = 0;

				ITERATOR sit;

				add_buf(buffer, "     [             Name             ] [             Room             ]\n\r");
				add_buf(buffer, "---------------------------------------------------------------------------------\n\r");

				bool approx_msg = false;
				iterator_start(&sit, bp->special_rooms);
				while( (special = (BLUEPRINT_SPECIAL_ROOM *)iterator_nextdata(&sit)) )
				{
					bool exact = false;
					BLUEPRINT_SECTION *bs = blueprint_get_representative_section(bp, special->section, &exact);

					ROOM_INDEX_DATA *room = NULL;

					if(IS_VALID(bs) )
					{
						room = blueprint_section_get_room_byoffset(bs, special->offset);
					}

					if( !IS_VALID(bs) || !room)
					{
						snprintf(buf, MSL-1, "{W%4d  %-30.30s   {D-{Winvalid{D-{x\n\r", ++line, special->name);
					}
					else
					{
						snprintf(buf, MSL-1, "{W%4d  %-30.30s   (%ld#%ld) {Y%s{x in (%ld#%ld) {Y%s{x%s\n\r", ++line, special->name, room->area->uid, room->vnum, room->name, bs->area->uid, bs->vnum, bs->name, exact?"":" {M**{x");
						if (!exact) approx_msg = true;
					}
					add_buf(buffer, buf);
				}
				iterator_stop(&sit);
				add_buf(buffer, "---------------------------------------------------------------------------------\n\r");

				if (approx_msg)
					add_buf(buffer, "{M**{x - {WLocation is the most likely location due to section entry.{x\n\r");
			}
			else
			{
				add_buf(buffer, "   None\n\r");
			}
			add_buf(buffer, "\n\r");

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

		if (!str_prefix(arg, "clear"))
		{
			if (list_size(bp->special_rooms) < 1)
			{
				send_to_char("There are no special rooms defined.\n\r", ch);
				return false;
			}

			list_clear(bp->special_rooms);
			send_to_char("Special Rooms cleared.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "add"))
		{
			sent_bool mode = TRISTATE_UNDEF;
			char arg2[MIL];	// name
			char arg3[MIL];	// mode
			char arg4[MIL];	// section
			// argument = room offset

			argument = one_argument(argument, arg2);
			if (arg2[0] == '\0')
			{
				send_to_char("Syntax:  rooms add {R<name>{x generated|ordinal <section> <room offset>\n\r", ch);
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg3);
			if (!str_prefix(arg3, "generated"))
				mode = false;
			else if(!str_prefix(arg3, "ordinal"))
				mode = true;
			else
			{
				send_to_char("Syntax:  rooms add <name> {Rgenerated|ordinal{x <section> <room offset>\n\r", ch);
				send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
				return false;
			}

			int sections = blueprint_generation_count(bp);
			argument = one_argument(argument, arg4);
			if (!is_number(arg4))
			{
				send_to_char("Syntax:  rooms add <name> generated|ordinal {R<section>{x <room offset>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			int section_no = atoi(arg4);
			if (section_no < 1 || section_no > sections)
			{
				send_to_char("Syntax:  rooms add <name> generated|ordinal {R<section>{x <room offset>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", sections);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_LAYOUT_SECTION_DATA *group;
			BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, (mode ? -section_no : section_no), &group);
			int rooms_count = blueprint_layout_room_count(bp, (group ? group : ls));

			if (!is_number(argument))
			{
				send_to_char("Syntax:  rooms add <name> generated|ordinal {R<section>{x <room offset>\n\r", ch);
				sprintf(buf, "Please specify a number from 0 to %d.\n\r", rooms_count - 1);
				send_to_char(buf, ch);
				return false;
			}

			int offset = atoi(argument);
			if (offset < 0 || offset >= rooms_count)
			{
				send_to_char("Syntax:  rooms add <name> generated|ordinal {R<section>{x <room offset>\n\r", ch);
				sprintf(buf, "Please specify a number from 0 to %d.\n\r", rooms_count - 1);
				send_to_char(buf, ch);
				return false;
			}

			BLUEPRINT_SPECIAL_ROOM *room = new_blueprint_special_room();
			free_string(room->name);
			room->name = str_dup(arg2);
			room->section = (mode ? -section_no : section_no);
			room->offset = offset;
			list_appendlink(bp->special_rooms, room);

			sprintf(buf, "Special Room \"%s\" (%d) added.\n\r", arg2, list_size(bp->special_rooms));
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg, "remove"))
		{
			if (list_size(bp->special_rooms) < 1)
			{
				send_to_char("There are no special rooms defined.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				send_to_char("Syntax:  rooms remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->special_rooms));
				send_to_char(buf, ch);
				return false;
			}

			int index = atoi(argument);
			if (index < 1 || index > list_size(bp->special_rooms))
			{
				send_to_char("Syntax:  rooms remove {R#{x\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(bp->special_rooms));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(bp->special_rooms, index, true);
			sprintf(buf, "Special Room %d removed.\n\r", index);
			send_to_char(buf, ch);
			return true;
		}
	}

	send_to_char("Syntax:  rooms {Rlist{x\n\r", ch);
	send_to_char("         rooms {Rclear{x\n\r", ch);
	send_to_char("         rooms {Radd{x <name> generated|ordinal <section> <room offset>\n\r", ch);
	send_to_char("         rooms {Rremove{x #\n\r", ch);
	return false;
}


BPEDIT (bpedit_addiprog)
{
	struct trigger_type *tt;
    int slot;
	BLUEPRINT *blueprint;
    PROG_LIST *list;
    SCRIPT_DATA *code;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MIL];

    EDIT_BLUEPRINT(ch, blueprint);
    argument = one_argument(argument, num);
    argument = one_argument(argument, trigger);
    argument = one_argument(argument, phrase);

    if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0')
    {
		send_to_char("Syntax:   addiprog [wnum] [trigger] [phrase]\n\r",ch);
		return false;
    }

    if (!(tt = get_trigger_type(trigger, PRG_IPROG))) {
		send_to_char("Valid flags are:\n\r",ch);
		show_help(ch, "iprog");
		return false;
    }

    slot = tt->slot;
	WNUM wnum;
	if (!parse_widevnum(num, ch->in_room->area, &wnum))
	{
		send_to_char("Syntax:   addiprog [wnum] [trigger] [phrase]\n\r",ch);
		send_to_char("          Invalid widevnum.\n\r", ch);
		return false;
	}

	if (!wnum.pArea)
		wnum.pArea = blueprint->area;

    if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_IPROG)) == NULL)
    {
		send_to_char("No such INSTANCEProgram.\n\r",ch);
		return false;
    }

    // Make sure this has a list of progs!
    if(!blueprint->progs) blueprint->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);
    list->script          = code;

    list_appendlink(blueprint->progs[slot], list);
	trigger_type_add_use(tt);

    send_to_char("Iprog Added.\n\r",ch);
    return true;
}

BPEDIT (bpedit_deliprog)
{
    BLUEPRINT *blueprint;
    char iprog[MAX_STRING_LENGTH];
    int value;

    EDIT_BLUEPRINT(ch, blueprint);

    one_argument(argument, iprog);
    if (!is_number(iprog) || iprog[0] == '\0')
    {
       send_to_char("Syntax:  deliprog [#iprog]\n\r",ch);
       return false;
    }

    value = atol (iprog);

    if (value < 0)
    {
        send_to_char("Only non-negative iprog-numbers allowed.\n\r",ch);
        return false;
    }

    if(!edit_deltrigger(blueprint->progs,value)) {
	send_to_char("No such iprog.\n\r",ch);
	return false;
    }

    send_to_char("Iprog removed.\n\r", ch);
    return true;
}

BPEDIT(bpedit_varset)
{
    BLUEPRINT *blueprint;

	EDIT_BLUEPRINT(ch, blueprint);

	return olc_varset(&blueprint->index_vars, ch, argument, false);
}

BPEDIT(bpedit_varclear)
{
    BLUEPRINT *blueprint;

	EDIT_BLUEPRINT(ch, blueprint);

	return olc_varclear(&blueprint->index_vars, ch, argument, false);
}