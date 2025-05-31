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

#include "redit.h"



REDIT(redit_show)
{
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    BUFFER *buf1;
	ROOM_INDEX_DATA *recall;


    int door;
    CONDITIONAL_DESCR_DATA *cd;
    int i;
    bool found=false;

    EDIT_ROOM(ch, pRoom);

    buf1 = new_buf();
    sprintf(buf, "Base Description:\n\r%s\n\r", pRoom->description);
    add_buf(buf1, buf);

    sprintf(buf, "Name:         {r[{x%s{r]{x\n\r"
                 "Area:         {r[{x%5ld{r]{x %s\n\r",
	    pRoom->name, pRoom->area->uid, pRoom->area->name);
    add_buf(buf1, buf);

    if (IS_SET(pRoom->rs_room_flag[1], ROOM_VIRTUAL_ROOM))
        sprintf (buf, "VRoom at ({W%ld{x, {W%ld{x), in wilds uid ({W%ld{x) '{W%s{x'\n\r",
                 pRoom->x, pRoom->y, pRoom->wilds->uid, pRoom->wilds->name);
    else if(pRoom->viewwilds)
        sprintf(buf, "Vnum:         {r[{x%5ld{r]{x\n\r"
                     "Sector:       {r[{x%s{r]{x\n\r"
                     "Map Coordinate at ({W%ld{x, {W%ld{x, {W%ld{x), in wilds uid ({W%ld{x) '{W%s{x'\n\r",
	        pRoom->vnum, pRoom->rs_sector->name,
	        pRoom->x, pRoom->y, pRoom->z, pRoom->viewwilds->uid, pRoom->viewwilds->name);
    else
		if (pRoom->rs_sector != NULL) {
			sprintf(buf, "Vnum:         {r[{x%5ld{r]{x\n\r"
						 "Sector:       {r[{x%s{r]{x\n\r",
					pRoom->vnum, pRoom->rs_sector->name);
		} else {
			sprintf(buf, "Vnum:         {r[{x%5ld{r]{x\n\r"
						 "Sector:       {r[NULL]{x\n\r",
					pRoom->vnum);
		}

    add_buf(buf1, buf);

    sprintf(buf, "Persist:      {r[%s{r]{x\n\r", (pRoom->persist ? "{WON" : "{Doff"));
    add_buf(buf1, buf);

    sprintf(buf, "Room flags:   {r[{x%s{r]{x\n\r",
		bitmatrix_string(room_flagbank, pRoom->rs_room_flag));
    add_buf(buf1, buf);

    if (pRoom->rs_heal_rate != 100 || pRoom->rs_mana_rate != 100 || pRoom->rs_move_rate != 100)
    {
	sprintf(buf,
	         "Health rec:   {r[{x%d{r]{x\n\r"
		 "Mana rec:     {r[{x%d{r]{x\n\r"
		 "Move rec:     {r[{x%d{r]{x\n\r",
		pRoom->rs_heal_rate , pRoom->rs_mana_rate, pRoom->rs_move_rate);
        add_buf(buf1, buf);
    }
	if (rs_location_isset(&pRoom->rs_recall))
	{
		if(pRoom->rs_recall.wuid) {
			WILDS_DATA *wilds = get_wilds_from_uid(NULL,pRoom->rs_recall.wuid);
			if(wilds)
				sprintf(buf, "{WRecall:      Wilds {X%s {R[{X%lu{R]{X at {R<{X%lu,%lu,%lu{R>{X\n\r", wilds->name, pRoom->rs_recall.wuid,
					pRoom->rs_recall.id[0],pRoom->rs_recall.id[1],pRoom->rs_recall.id[2]);
			else
				sprintf(buf, "{WRecall:      Wilds {X??? {R[{X%lu{R]{X\n\r", pRoom->rs_recall.wuid);
		} else if(pRoom->rs_recall.id[0] > 0 && (recall = get_room_index(get_area_from_uid(pRoom->rs_recall.auid), pRoom->rs_recall.id[0]))) {
				sprintf(buf, "{WRecall:      Room {R[{X%5ld{R]{X {X%s\n\r", pRoom->rs_recall.id[0], recall->name);
		} else
				sprintf(buf, "{WRecall:      {R[{X%lu{R]{X none\n\r", pRoom->rs_recall.id[0]);
		add_buf(buf1, buf);
	}

	if (pRoom->rs_savage_level < 0)
		sprintf(buf, "Savagery:     {r[{Y-area-{r]{x\n\r");
	else
		sprintf(buf, "Savagery:     {r[{x%d{r]{x\n\r", pRoom->rs_savage_level);
	add_buf(buf1, buf);

	if (IS_VALID(pRoom->region))
	{
		sprintf(buf, "Region:       {r[{x%s{r]{x\n\r", pRoom->region->name);
		add_buf(buf1, buf);
	}

    if (pRoom->locale) {
		sprintf(buf, "Locale:       {r[{x%ld{r]{x\n\r", pRoom->locale);
		add_buf(buf1, buf);
    }

    if (!IS_NULLSTR(pRoom->owner))
    {
	sprintf(buf,
	         "Owner:        {r[{x%s{r]{x\n\r", pRoom->owner);
        add_buf(buf1, buf);
    }

    if (pRoom->home_owner != NULL && pRoom->home_owner[0] != '\0')
    {
	sprintf(buf,
	         "Home owner:   {r[{x%s{r]{x\n\r", pRoom->home_owner);
        add_buf(buf1, buf);
    }

	sprintf(buf, "\n\r-----\n\r{WBuilders' Comments:{X\n\r%s\n\r-----\n\r", pRoom->comments);
	add_buf(buf1, buf);


    if (pRoom->extra_descr)
    {
	EXTRA_DESCR_DATA *ed;

	add_buf(buf1,
	         "Desc Kwds:    {r[{x");

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    add_buf(buf1, ed->keyword);

	    if (ed->next)
		add_buf(buf1, " ");
	}

	add_buf(buf1, "{r]{x\n\r");
    }

    found=0;
    for (door = 0; door < MAX_DIR; door++)
    {
	EXIT_DATA *pexit;

	if ((pexit = pRoom->exit[door]))
	{
	    AREA_DATA *pArea = NULL;
	    WILDS_DATA *pWilds = NULL;
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;
            bool ffound = false;


            if (pRoom->wilds)
            {
                if (IS_SET(pexit->exit_info, EX_VLINK))
				{
                    sprintf (buf, "-{W%-9s{x to {W%6ld{x, Area Uid ({W%ld{x), '{W%s{x'.\n\r",
						capitalize (dir_name[door]),
						pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
						pexit->u1.to_room ? pexit->u1.to_room->area->uid : 0,
						pexit->u1.to_room ? pexit->u1.to_room->area->name : "{RERROR");
				}
                else
                    sprintf (buf, "-{W%-9s{x to ({W%d{x,{W%d{x).\n\r",
                             capitalize (dir_name[door]),
                             pexit->wilds.x, pexit->wilds.y);
            }
            else
            {
                if (IS_SET(pexit->exit_info, EX_VLINK))
				{
					pArea = get_area_from_uid(pexit->wilds.area_uid);
					pWilds = get_wilds_from_uid(pArea, pexit->wilds.wilds_uid);
                    sprintf (buf, "-{W%-9s{x to ({W%d{x,{W%d{x), Wilds Uid ({W%ld{x), '{W%s{x'.\n\r",
						capitalize (dir_name[door]),
						pexit->wilds.x, pexit->wilds.y,
						pWilds ? pWilds->uid : 0,
						pWilds ? pWilds->name : "(null)");
					add_buf(buf1, buf);

					sprintf (buf, "                         Area Uid ({W%ld{x), '{W%s{x'.\n\r",
					pArea ? pArea->uid : 0,
					pArea ? pArea->name : "(null)");
				}
                else
                    sprintf (buf, "-{W%-9s{x to {W%ld#%ld{x %s\n\r",
                             capitalize (dir_name[door]),
							 pexit->u1.to_room ? pexit->u1.to_room->area->uid : 0,
                             pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
							 pexit->u1.to_room ? pexit->u1.to_room->name : "(null)");
            }

		    add_buf(buf1, buf);

            /*
             * Format up the exit info.
             * Capitalize all flags that are not part of the reset info.
             */
            strcpy (reset_state, flag_string (exit_flags, pexit->rs_flags));
            state = flag_string (exit_flags, pexit->exit_info);
            add_buf(buf1, "    Exit flags: [{W");
            ffound = false;
            for (;;)
            {
                state = one_argument (state, word);

                if (word[0] == '\0')
                {
                    add_buf(buf1, "{x]\n\r");
                    break;
                }

		if (str_infix(word, reset_state))
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = UPPER(word[i]);
		}

                if (ffound == true)
                    add_buf(buf1, " ");

                add_buf(buf1, word);
                ffound = true;
            }

            if (pexit->long_desc && pexit->long_desc[0] != '\0')
            {
                sprintf (buf, "    Exit Description:\n\r    {W%s{x\n\r", pexit->long_desc);
                add_buf(buf1, buf);
            }

            sprintf (buf, "    Keywords: [{W%s{x]\n\r"
                          "    Short Description: '{W%s{x'\n\r",
                     pexit->keyword && pexit->keyword[0] != '\0' ? pexit->keyword : "(Not set)",
                     pexit->short_desc && pexit->short_desc[0] != '\0' ? pexit->short_desc : "(Not set)");
            add_buf(buf1, buf);

            if (IS_SET(pexit->rs_flags, EX_ISDOOR))
            {
                sprintf (buf, "    -Door Material: [{W%s{x] Strength: [{W%d{x]  Lock Flags: [{W%s{x]  Key wnum: [{W%ld#%ld{x] Pick chance: [{W%d%%{x]\n\r",
                         IS_VALID(pexit->door.material) ? pexit->door.material->name : "{Dnone",
                         pexit->door.strength,
                         flag_string(lock_flags, pexit->door.lock.flags),
                         pexit->door.lock.key_wnum.pArea ? pexit->door.lock.key_wnum.pArea->uid : 0,
						 pexit->door.lock.key_wnum.vnum,
                         pexit->door.lock.pick_chance);
                add_buf(buf1, buf);
            }
            else
                add_buf(buf1, "\n\r");

            found = true;
	}
    }

    if (found == false)
        add_buf(buf1, "    {W(None set){x\n\r");

	
    if (pRoom->progs->progs)
		olc_show_progs(buf1, pRoom->progs->progs, PRG_RPROG, "RoomProg Vnum");

	olc_show_index_vars(buf1, pRoom->index_vars);

    if (pRoom->conditional_descr)
    {
		char phrase[MIL];

		sprintf(buf, "\n\rConditional Descriptions for {r[{x%5ld{r]{x:\n\r", pRoom->vnum);

		add_buf(buf1, buf);

		for (i = 0, cd = pRoom->conditional_descr; cd != NULL; cd = cd->next)
		{
			if (i == 0)
			{
				add_buf(buf1, "{Y Num  Condition Phrase{x\n\r");
				add_buf(buf1, "{Y ---  --------- ------{x\n\r");
			}

			if (cd->condition == CONDITION_HOUR || cd->condition == CONDITION_SCRIPT)
				sprintf(phrase, "%d", cd->phrase);
			else {
				strncpy(phrase, condition_phrase_to_name(cd->condition, cd->phrase), MIL-1);
				phrase[MIL-1] = '\0';
			}


			sprintf(buf, "{r[{x%3d{r]{x %-9s %s\n\r", i, condition_type_to_name(cd->condition), phrase );

			add_buf(buf1, buf);
			i++;
		}
    }

    page_to_char (buf_string(buf1), ch);
    free_buf(buf1);


	if (ch->in_room->reset_first)
	{
	    send_to_char(
		"\n\rResets: M = mobile, R = room, O = object, "
		"P = pet, S = shopkeeper\n\r", ch);
	    display_resets(ch);
	}

    return false;
}


REDIT(redit_north)
{
    if (change_exit(ch, argument, DIR_NORTH))
	return true;

    return false;
}


REDIT(redit_west)
{
    if (change_exit(ch, argument, DIR_WEST))
	return true;

    return false;
}



REDIT(redit_south)
{
    if (change_exit(ch, argument, DIR_SOUTH))
	return true;

    return false;
}



REDIT(redit_east)
{
    if (change_exit(ch, argument, DIR_EAST))
	return true;

    return false;
}


REDIT(redit_southeast)
{
    if (change_exit(ch, argument, DIR_SOUTHEAST))
	return true;

    return false;
}


REDIT(redit_southwest)
{
    if (change_exit(ch, argument, DIR_SOUTHWEST))
	return true;

    return false;
}


REDIT(redit_northeast)
{
    if (change_exit(ch, argument, DIR_NORTHEAST))
	return true;

    return false;
}


REDIT(redit_northwest)
{
    if (change_exit(ch, argument, DIR_NORTHWEST))
	return true;

    return false;
}


REDIT(redit_up)
{
    if (change_exit(ch, argument, DIR_UP))
	return true;

    return false;
}


REDIT(redit_down)
{
    if (change_exit(ch, argument, DIR_DOWN))
	return true;

    return false;
}

REDIT(redit_varset)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

	if(olc_varset(&pRoom->index_vars, ch, argument, false))
	{
		// This will *NOT* update cloned rooms...
		olc_varset(&pRoom->progs->vars, ch, argument, true);
		return true;
	}
	return false;
}

REDIT(redit_varclear)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

	if(olc_varclear(&pRoom->index_vars, ch, argument, false))
	{
		// This will *NOT* update cloned rooms...
		olc_varclear(&pRoom->progs->vars, ch, argument, true);
		return true;
	}

	return false;
}


REDIT(redit_ed)
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;

    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];
    char copy_item[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, command);
    argument = one_argument(argument, keyword);
    one_argument(argument, copy_item);

    if (command[0] == '\0' || keyword[0] == '\0')
    {
	send_to_char("Syntax:  ed add [keyword]\n\r", ch);
	send_to_char("         ed edit [keyword]\n\r", ch);
	send_to_char("         ed show [keyword]\n\r", ch);
	send_to_char("         ed delete [keyword]\n\r", ch);
	send_to_char("         ed format [keyword]\n\r", ch);
	send_to_char("         ed copy existing_keyword new_keyword\n\r", ch);
	send_to_char("         ed environment [keyword]\n\r", ch);

	return false;
    }

    if (!str_cmp(command, "copy"))
    {
	EXTRA_DESCR_DATA *ed2;

    	if (keyword[0] == '\0' || copy_item[0] == '\0')
	{
	   send_to_char("Syntax:  ed copy existing_keyword new_keyword\n\r", ch);
	   return false;
        }

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    if (is_name(keyword, ed->keyword))
		break;
	}

	if (!ed)
	{
	    send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
	    return false;
	}

	ed2			=   new_extra_descr();
	ed2->keyword		=   str_dup(copy_item);
	if( ed->description )
		ed2->description		= str_dup(ed->description);
	else
		ed2->description		= NULL;
	ed2->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed2;

	send_to_char("Done.\n\r", ch);

	return true;
    }

    if (!str_cmp(command, "environment"))
    {
	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed environment [keyword]\n\r", ch);
	    return false;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup(keyword);
	ed->description		= NULL;
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	send_to_char("Enviromental extra description added.\n\r", ch);

	return true;
    }

    if (!str_cmp(command, "add"))
    {
	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed add [keyword]\n\r", ch);
	    return false;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup(keyword);
	ed->description		=   str_dup("");
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append(ch, &ed->description);

	return true;
    }


    if (!str_cmp(command, "edit"))
    {
	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed edit [keyword]\n\r", ch);
	    return false;
	}

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    if (is_name(keyword, ed->keyword))
		break;
	}

	if (!ed)
	{
	    send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
	    return false;
	}

	if( !ed->description )
		ed->description = str_dup("");

	string_append(ch, &ed->description);

	return true;
    }


    if (!str_cmp(command, "delete"))
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed delete [keyword]\n\r", ch);
	    return false;
	}

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    if (is_name(keyword, ed->keyword))
		break;
	    ped = ed;
	}

	if (!ed)
	{
	    send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
	    return false;
	}

	if (!ped)
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr(ed);

	send_to_char("Extra description deleted.\n\r", ch);
	return true;
    }


    if (!str_cmp(command, "format"))
    {
	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed format [keyword]\n\r", ch);
	    return false;
	}

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    if (is_name(keyword, ed->keyword))
		break;
	}

	if (!ed)
	{
	    send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
	    return false;
	}

	if( !ed->description )
	{
	    send_to_char("REdit:  Extra description is an environmental extra description.\n\r", ch);
	    return false;
	}

	ed->description = format_string(ed->description);

	send_to_char("Extra description formatted.\n\r", ch);
	return true;
    }

    if (!str_cmp(command, "show"))
    {
	if (keyword[0] == '\0')
	{
	    send_to_char("Syntax:  ed show [keyword]\n\r", ch);
	    return false;
	}

	for (ed = pRoom->extra_descr; ed; ed = ed->next)
	{
	    if (is_name(keyword, ed->keyword))
		break;
	}

	if (!ed)
	{
	    send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
	    return false;
	}

	if (!ed->description)
	{
		send_to_char("REdit:  Cannot show environmental extra description.\n\r", ch);
		return false;
	}

	page_to_char(ed->description, ch);

	return true;
    }

    redit_ed(ch, "");
    return false;
}


REDIT(redit_create)
{
    ROOM_INDEX_DATA *pRoom;
    int iHash;
	WNUM wnum;

    if (argument[0] == '\0' || !parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
    {
		//send_to_char("Syntax:  create [vnum > 0]\n\r", ch);
		wnum.pArea = ch->in_room->area;
		for(wnum.vnum = 1; wnum.vnum > 0 && get_room_index(wnum.pArea, wnum.vnum); wnum.vnum++);

		// wnum.vnum overflowed.... if it ever reaches this...
		if (wnum.vnum < 1) {
			send_to_char("Sorry, this area has no more space left.\n\r", ch);
			return false;
		}
    }

    if (!IS_BUILDER(ch, wnum.pArea))
    {
		send_to_char("REdit:  widevnum in an area you cannot build in.\n\r", ch);
		return false;
    }

    if (get_room_index(wnum.pArea, wnum.vnum))
    {
		send_to_char("REdit:  Room vnum already exists.\n\r", ch);
		return false;
    }

    pRoom = new_room_index();
    pRoom->area = wnum.pArea;

	list_appendlink(wnum.pArea->room_list, pRoom);	// Add to the area room list
    pRoom->vnum = wnum.vnum;
	wnum.pArea->bottom_vnum_room = UMIN(wnum.pArea->bottom_vnum_room, wnum.vnum);
	wnum.pArea->top_vnum_room = UMAX(wnum.pArea->top_vnum_room, wnum.vnum);

	// Check whether to automatically set the room as blueprint
    if( redit_blueprint_oncreate )
    {
		ROOM_INDEX_DATA *pPrevRoom;

		EDIT_ROOM(ch, pPrevRoom);
		// Only copy if the new room is in the same area as the previous room
		if( pPrevRoom && pPrevRoom->area == wnum.pArea )
		{
			SET_BIT(pRoom->room_flag[1], ROOM_BLUEPRINT);
		}
		redit_blueprint_oncreate = false;
	}

	if (IS_SET(ch->act[0], PLR_AUTOOLC))
	{
		// Have we changed areas?
		if (ch->desc->last_area != pRoom->area)
		{
			ch->desc->last_area = pRoom->area;
			ch->desc->last_area_region = NULL;
			ch->desc->last_room_sector = NULL;
			ch->desc->last_room_flag[0] = 0;
			ch->desc->last_room_flag[0] = 0;
		}
	}

	if (!IS_SET(pRoom->room_flag[1], ROOM_BLUEPRINT))
	{
		AREA_REGION *region = &pRoom->area->region;

		if (IS_SET(ch->act[0], PLR_AUTOOLC))
		{
			// Check if we are in the same area as the last used region
			if (ch->desc->last_area_region)
			{
				if (ch->desc->last_area_region->area != pRoom->area)
				{
					ch->desc->last_area_region = NULL;
				}	
			}

			// If we have a last region, use that
			if (ch->desc->last_area_region)
				region = ch->desc->last_area_region;
		}

		// Only non-blueprint rooms will have this done
		__region_add_room(region, pRoom);
	}

	if (IS_SET(ch->act[0], PLR_AUTOOLC))
	{
		if (ch->desc->last_room_sector != NULL)
		{
			pRoom->sector = ch->desc->last_room_sector;
		}

		pRoom->rs_room_flag[0] = ch->desc->last_room_flag[0];
		pRoom->rs_room_flag[1] = ch->desc->last_room_flag[1];
	}

    iHash = wnum.vnum % MAX_KEY_HASH;
    pRoom->next = wnum.pArea->room_index_hash[iHash];
    wnum.pArea->room_index_hash[iHash] = pRoom;

	olc_set_editor(ch, ED_ROOM, pRoom);

    SET_BIT(pRoom->area->area_flags, AREA_CHANGED);
    send_to_char("Room created.\n\r", ch);
    return true;
}


REDIT(redit_name)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  name [name]\n\r", ch);
	return false;
    }

    argument[0] = UPPER(argument[0]);

    free_string(pRoom->name);
    pRoom->name = str_dup(argument);

    send_to_char("Name set.\n\r", ch);
    return true;
}


REDIT(redit_desc)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0')
    {
	string_append(ch, &pRoom->description);
	return true;
    }

    send_to_char("Syntax:  desc\n\r", ch);
    return false;
}

REDIT(redit_comments)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0')
    {
	string_append(ch, &pRoom->comments);
	return true;
    }

    send_to_char("Syntax:  comment\n\r", ch);
    return false;
}

REDIT(redit_recall)
{
	ROOM_INDEX_DATA *pRoom;
	char arg1[MIL];
	char arg2[MIL];
	char arg3[MIL];
	char arg4[MIL];
	int vnum, x, y, z;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);
	argument = one_argument(argument, arg4);

	if (!is_number(arg1) || !arg1[0]) {
		send_to_char("Syntax:  recall <vnum>\n\r", ch);
		send_to_char("         recall <wuid> <x> <y> <z>\n\r", ch);
		return false;
	}

	vnum = atoi(arg1);

	if(vnum < 1) {
		rs_location_clear(&pRoom->rs_recall);
		send_to_char("Recall cleared.\n\r", ch);
	} else if(!arg2[0]) {
		if(!get_room_index(pRoom->area, vnum)) {
			send_to_char("AEdit:  Room vnum does not exist.\n\r", ch);
			return false;
		}

		rs_location_set(&pRoom->rs_recall,0,vnum,0,0);
		send_to_char("Recall set.\n\r", ch);
	} else if(!arg3[0] || !arg4[0] || !is_number(arg2) || !is_number(arg3) || !is_number(arg4)) {
		send_to_char("Syntax:  recall <vnum>\n\r", ch);
		send_to_char("         recall <wuid> <x> <y> <z>\n\r", ch);
		return false;
	} else if(!get_wilds_from_uid(NULL,vnum)) {
		send_to_char("AEdit:  Wilderness UID does not exist.\n\r", ch);
		return false;
	} else {
		x = atoi(arg2);
		y = atoi(arg3);
		z = atoi(arg4);
		rs_location_set(&pRoom->rs_recall,vnum,x,y,z);
		send_to_char("Recall set.\n\r", ch);
	}

	return true;
}


REDIT(redit_heal)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument))
       {
          pRoom->rs_heal_rate = atoi (argument);
          send_to_char ("Heal rate set.\n\r", ch);
          return true;
       }

    send_to_char ("Syntax : heal <#xnumber>\n\r", ch);
    return false;
}


REDIT(redit_mana)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument))
       {
          pRoom->rs_mana_rate = atoi (argument);
          send_to_char ("Mana rate set.\n\r", ch);
          return true;
       }

    send_to_char ("Syntax : mana <#xnumber>\n\r", ch);
    return false;
}


REDIT(redit_move)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument))
    {
	pRoom->rs_move_rate = atoi (argument);
	send_to_char ("Movement regen rate set.\n\r", ch);
	return true;
    }

    send_to_char ("Syntax: move <#xnumber>\n\r", ch);
    return false;
}


REDIT(redit_mreset)
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*newmob;
    char		arg [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];
	WNUM 		wnum;

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0' || !parse_widevnum(arg, ch->in_room->area, &wnum))
    {
	send_to_char ("Syntax:  mreset <widevnum> <max #x> <mix #x>\n\r", ch);
	return false;
    }

    if (!(pMobIndex = get_mob_index(wnum.pArea, wnum.vnum)))
    {
		send_to_char("REdit: No mobile has that widevnum.\n\r", ch);
		return false;
    }

    /*
     * Create the mobile reset.
     */
    pReset						= new_reset_data();
    pReset->command				= 'M';
	pReset->arg1.wnum.pArea		= pMobIndex->area;
	pReset->arg1.wnum.vnum		= pMobIndex->vnum;
    pReset->arg2				= is_number(arg2) ? atoi(arg2) : MAX_MOB;
	// arg3 is ignored
    pReset->arg4				= is_number(argument) ? atoi (argument) : 1;
    add_reset(pRoom, pReset, 0/* Last slot*/);

    /*
     * Create the mobile.
     */
    newmob = create_mobile(pMobIndex, false);
    char_to_room(newmob, pRoom);
//    if (HAS_TRIGGER_MOB(newmob, TRIG_REPOP))
	p_percent_trigger(newmob, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL,0,0,0,0,0);
    sprintf(output, "%s (%ld) has been loaded and added to resets.\n\r"
	"There will be a maximum of %ld loaded to this room.\n\r",
	capitalize(pMobIndex->short_descr),
	pMobIndex->vnum,
	pReset->arg2);
    send_to_char(output, ch);
    act("$n has created $N!", ch, newmob, NULL, NULL, NULL, NULL, NULL, TO_ROOM, NULL, NULL);
    return true;
}


REDIT(redit_oreset)
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    int			olevel = 0;

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];
	WNUM wnum;

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || !parse_widevnum(arg1, ch->in_room->area, &wnum))
    {
		send_to_char ("Syntax:  oreset <widevnum> <args>\n\r", ch);
		send_to_char ("        -no_args               = into room\n\r", ch);
		send_to_char ("        -<obj_name>            = into obj\n\r", ch);
		send_to_char ("        -<mob_name> <wear_loc> = into mob\n\r", ch);
		return false;
    }

    if (!(pObjIndex = get_obj_index(wnum.pArea, wnum.vnum)))
    {
		send_to_char("REdit: No object has that widevnum.\n\r", ch);
		return false;
    }

    /*
     * Load into room.
     */
    if (arg2[0] == '\0')
    {
	pReset		= new_reset_data();
	pReset->command	= 'O';
	pReset->arg1.wnum.pArea	= pObjIndex->area;
	pReset->arg1.wnum.vnum = pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg4	= 0;
	add_reset(pRoom, pReset, 0/* Last slot*/);

	newobj = create_object(pObjIndex, number_fuzzy(olevel), true);
	obj_to_room(newobj, pRoom);

	sprintf(output, "%s (%ld) has been loaded and added to resets.\n\r",
	    capitalize(pObjIndex->short_descr),
	    pObjIndex->vnum);
	send_to_char(output, ch);
    }
    else
    /*
     * Load into object's inventory.
     */
    if (argument[0] == '\0'
    && ((to_obj = get_obj_list(ch, arg2, pRoom->contents)) != NULL))
    {
		// Need to find this object in the resets and put it AFTER that.
	pReset		= new_reset_data();
	pReset->command	= 'P';
	pReset->arg1.wnum.pArea	= pObjIndex->area;
	pReset->arg1.wnum.vnum = pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3.wnum.pArea	= to_obj->pIndexData->area;
	pReset->arg3.wnum.vnum = to_obj->pIndexData->vnum;
	pReset->arg4	= 1;
	add_reset(pRoom, pReset, 0/* Last slot*/);

	newobj = create_object(pObjIndex, number_fuzzy(olevel), true);
	newobj->cost = 0;
	obj_to_obj(newobj, to_obj);

	sprintf(output, "%s (%ld) has been loaded into "
	    "%s (%ld) and added to resets.\n\r",
	    capitalize(newobj->short_descr),
	    newobj->pIndexData->vnum,
	    to_obj->short_descr,
	    to_obj->pIndexData->vnum);
	send_to_char(output, ch);
    }
    else
    /*
     * Load into mobile's inventory.
     */
    if ((to_mob = get_char_room(ch, NULL, arg2)) != NULL && IS_NPC(to_mob))
    {
	int	wear_loc;

	/*
	 * Make sure the location on mobile is valid.
	 */
	if ((wear_loc = flag_value(wear_loc_flags, argument)) == NO_FLAG)
	{
	    send_to_char("REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch);
	    return false;
	}

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if (!IS_SET(pObjIndex->wear_flags, wear_bit(wear_loc)))
	{
	    sprintf(output,
	        "%s (%ld) has wear flags: [%s]\n\r",
	        capitalize(pObjIndex->short_descr),
	        pObjIndex->vnum,
		flag_string(wear_flags, pObjIndex->wear_flags));
	    send_to_char(output, ch);
	    return false;
	}

	/*
	 * Can't load into same position.
	 */
	if (get_eq_char(to_mob, wear_loc))
	{
	    send_to_char("REdit:  Object already equipped.\n\r", ch);
	    return false;
	}

	pReset		= new_reset_data();
	pReset->arg1.wnum.pArea	= pObjIndex->area;
	pReset->arg1.wnum.vnum = pObjIndex->vnum;
	pReset->arg2	= wear_loc;
	if (pReset->arg2 == WEAR_NONE)
	    pReset->command = 'G';
	else
	    pReset->command = 'E';
	pReset->arg3.wnum.pArea = to_mob->pIndexData->area;
	pReset->arg3.wnum.vnum = to_mob->pIndexData->vnum;
	pReset->arg4	= wear_loc;

	add_reset(pRoom, pReset, 0/* Last slot*/);

	olevel  = UMAX(0, to_mob->tot_level - 2);
	newobj = create_object(pObjIndex, number_fuzzy(olevel), true);


	obj_to_char(newobj, to_mob);
	if (pReset->command == 'E')
	    equip_char(to_mob, newobj, pReset->arg4);

	sprintf(output, "%s (%ld) has been loaded "
	    "%s of %s (%ld) and added to resets.\n\r",
	    capitalize(pObjIndex->short_descr),
	    pObjIndex->vnum,
	    flag_string(wear_loc_strings, pReset->arg4),
	    to_mob->short_descr,
	    to_mob->pIndexData->vnum);
	send_to_char(output, ch);
    }
    else	/* Display Syntax */
    {
	send_to_char("REdit:  That mobile isn't here.\n\r", ch);
	return false;
    }

    act("$n has created $p!", ch, NULL, NULL, newobj, NULL, NULL, NULL, TO_ROOM, NULL, NULL);
    return true;
}

REDIT(redit_persist)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);


	if (!str_cmp(argument,"on")) {
	    if (!IS_STAFF(ch, STAFF_CREATOR)) {
			send_to_char("Insufficient security.  Department of Homeland Security has been notified.\n\r", ch);
			return false;
	    }

		persist_addroom(pRoom);
		send_to_char("Persistance enabled.\n\r", ch);
	} else if (!str_cmp(argument,"off")) {
		persist_removeroom(pRoom);
		send_to_char("Persistance disabled.\n\r", ch);
	} else {
		send_to_char("Usage: persist on/off\n\r", ch);
		return false;
	}

	return true;
}


REDIT(redit_owner)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  owner [owner]\n\r", ch);
	send_to_char("         owner none\n\r", ch);
	return false;
    }

    free_string(pRoom->owner);
    if (!str_cmp(argument, "none"))
    	pRoom->owner = str_dup("");
    else
	pRoom->owner = str_dup(argument);

    send_to_char("Owner set.\n\r", ch);
    return true;
}


REDIT(redit_room)
{
    ROOM_INDEX_DATA *room;

    EDIT_ROOM(ch, room);

	long bits[2];
	if (!bitmatrix_lookup(argument, room_flagbank, bits))
	{
		send_to_char("Syntax:  room <flags>\n\r", ch);
		send_to_char("Type '? room' for list of flags.\n\r", ch);
		return false;
	}


    if( IS_SET(bits[1], ROOM_BLUEPRINT) )
    {
		// Only those that can edit blueprints can toggle this flag
		/*
		// TODO: Readd this when the role access system is added
		if( !can_edit_blueprints(ch) )
		{
			value &= ~ROOM_BLUEPRINT;

			if( !value )
			{
				send_to_char("Syntax: room2 [flags]\n\r", ch);
				return false;
			}
		}
		else*/ if( !IS_SET(bits[1], ROOM_NOCLONE) && IS_SET(room->rs_room_flag[1], ROOM_NOCLONE) )
		{
			send_to_char("No-clone room cannot be used in blueprints.\n\r", ch);
			return false;
		}
		else if( IS_SET(bits[1], ROOM_NOCLONE) && !IS_SET(room->rs_room_flag[1], ROOM_NOCLONE) )
		{
			send_to_char("BLUEPRINT and NO_CLONE cannot mix.\n\r", ch);
			return false;
		}
	}

	if( IS_SET(bits[1], ROOM_NOCLONE) )
	{
		if( !IS_SET(bits[1], ROOM_BLUEPRINT) && IS_SET(room->rs_room_flag[1], ROOM_BLUEPRINT) )
		{
			send_to_char("Blueprint rooms cannot be no-clone.\n\r", ch);
			return false;
		}

		// Check if room is already used in a section
		if( get_blueprint_section_byroom(room->area, room->vnum) )
		{
			send_to_char("Room is currently used in a blueprint.\n\r", ch);
			// Clear it out, JIC
			if( IS_SET(room->rs_room_flag[1], ROOM_NOCLONE) )
			{
			    REMOVE_BIT(room->rs_room_flag[1], ROOM_NOCLONE);
			    return true;
			}

			return false;
		}
	}


	for(int i = 0; i < 2; i++)
    	TOGGLE_BIT(room->rs_room_flag[i], bits[i]);

	// Now that we've gotten passed the validation:
	// Check for toggling blueprints on and off
	if (IS_SET(bits[1], ROOM_BLUEPRINT))
	{
		// Turned on
		if (IS_SET(room->rs_room_flag[1], ROOM_BLUEPRINT))
		{
			// Blueprint rooms have *no* regions whatsoever
			__region_remove_room(room);
		}
		// Turned off
		else
		{
			// Place room into the area's default region
			__region_add_room(&room->area->region, room);
		}
	}

	if (IS_SET(ch->act[0], PLR_AUTOOLC))
	{
		ch->desc->last_room_flag[0] = room->rs_room_flag[0];
		ch->desc->last_room_flag[1] = room->rs_room_flag[1];
	}

    send_to_char("Room flags toggled.\n\r", ch);
    return true;
}


REDIT(redit_sector)
{
    ROOM_INDEX_DATA *room;

    EDIT_ROOM(ch, room);

	SECTOR_DATA *sector = get_sector_data(argument);
	if (!sector)
	{
		send_to_char("Syntax:  sector <sector>\n\r", ch);
		send_to_char("Invalid sector.  Use 'sectorlist' for list of valid sectors.\n\r", ch);
		return false;
	}

    room->rs_sector = sector;

	if (IS_SET(ch->act[0], PLR_AUTOOLC))
	{
		ch->desc->last_room_sector = sector;
	}

    send_to_char("Sector type set.\n\r", ch);
    return true;
}


REDIT(redit_coords)
{
	char arg1[MIL];
	char arg2[MIL];
	char arg3[MIL];
    ROOM_INDEX_DATA *room;
    WILDS_DATA *w;
    int x,y,z;

    EDIT_ROOM(ch, room);

    if(room->wilds) {
	    send_to_char("Wilderness rooms cannot be modified.\n\r",ch);
	    return false;
    }

    if(IS_NULLSTR(argument)) {
	    send_to_char("coords <x> <y> <z>[ <wilds uid>]\n\r",ch);
	    send_to_char("<wilds uid> can be omitted if dealing with a blueprint room.\n\r", ch);
	    return false;
    }

	if(!str_cmp(argument,"none")) {
		room->viewwilds = NULL;
		room->x = 0;
		room->y = 0;
		room->z = 0;
	} else {
		argument = one_argument(argument,arg1);
		argument = one_argument(argument,arg2);
		argument = one_argument(argument,arg3);


		x = atoi(arg1);
		y = atoi(arg2);
		z = atoi(arg3);

		if( !IS_SET(room->room_flag[1], ROOM_BLUEPRINT) )
		{
			w = get_wilds_from_uid(NULL,atoi(argument));
			if(!w) {
				send_to_char("No such wilderness.\n\r",ch);
				return false;
			}

			if(x < 0 || x >= w->map_size_x) {
				send_to_char("Invalid map coordinate.\n\r",ch);
				return false;
			}
			if(y < 0 || y >= w->map_size_y) {
				send_to_char("Invalid map coordinate.\n\r",ch);
				return false;
			}

			room->viewwilds = w;
		}
		else
			room->viewwilds = NULL;

		room->x = x;
		room->y = y;
		room->z = z;

		send_to_char("Coordinate set.\n\r", ch);
	}

	return true;
}

REDIT(redit_locale)
{
    ROOM_INDEX_DATA *room;
    int locale;

    EDIT_ROOM(ch, room);

    if(IS_NULLSTR(argument) || !is_number(argument)) {
	    send_to_char("locale <#locale>\n\r",ch);
	    return false;
    }

    locale = atoi(argument);

    room->locale = locale;

    send_to_char("Locale set.\n\r", ch);

    return true;
}

REDIT (redit_region)
{
    ROOM_INDEX_DATA *room;
	char buf[MSL];

    EDIT_ROOM(ch, room);

	if (list_size(room->area->regions) < 1)
	{
		send_to_char("Room's area does not have any regions to select from.\n\r", ch);
		return false;
	}

	if (IS_NULLSTR(argument))
	{
		send_to_char("Syntax:  region <# or default>\n\r", ch);
		return false;
	}

	AREA_REGION *region = NULL;

	if (!str_prefix(argument, "default"))
	{
		region = &room->area->region;
	}
	else if (!is_number(argument))
	{
		send_to_char("Syntax:  region <# or default>\n\r", ch);
		sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(room->area->regions));
		send_to_char(buf, ch);
		return false;
	}
	else
	{
		int region_no = atoi(argument);
		if (region_no < 1 || region_no > list_size(room->area->regions))
		{
			send_to_char("Syntax:  region <# or default>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(room->area->regions));
			send_to_char(buf, ch);
			return false;
		}

		region = (AREA_REGION *)list_nthdata(room->area->regions, region_no);
	}

	if (!IS_VALID(region))
	{
		send_to_char("That is not a valid region.\n\r", ch);
		return false;
	}

	if (room->region == region)
	{
		send_to_char("The room is already in that region.\n\r", ch);
		return false;
	}
	
	__region_remove_room(room);
	__region_add_room(region, room);

	if (IS_SET(ch->act[0], PLR_AUTOOLC))
	{
		ch->desc->last_area_region = region;	// Save for faster building
	}
	send_to_char("Room region set.\n\r", ch);
	return true;
}

REDIT (redit_savage)
{
    ROOM_INDEX_DATA *room;

    EDIT_ROOM(ch, room);

    if (IS_NULLSTR(argument))
	{
	    send_to_char("savage <#0-5 or auto>\n\r",ch);
	    return false;
    }

	if (!str_prefix(argument, "auto"))
	{
		room->rs_savage_level = -1;
	}
	else if (!is_number(argument))
	{
	    send_to_char("savage <#0-5 or auto>\n\r",ch);
	    return false;
	}
	else
	{
		int level = atoi(argument);
		if (level < 0 || level > 5)
		{
			send_to_char("Savage level can only be {Yauto{x or {W0{x to {W5{x.\n\r", ch);
			return false;
		}

		room->rs_savage_level = level;
	}

	send_to_char("Savage level set.\n\r", ch);
	return true;
}



REDIT (redit_addrprog)
{
	struct trigger_type *tt;
    int value, slot;
    PROG_LIST *list;
    SCRIPT_DATA *code;
    ROOM_INDEX_DATA *pRoom;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_ROOM(ch, pRoom);
    argument=one_argument(argument, num);
    argument=one_argument(argument, trigger);
    argument=one_argument(argument, phrase);

	WNUM wnum;
    if (!parse_widevnum(num, ch->in_room->area, &wnum) || trigger[0] =='\0' || phrase[0] =='\0')
    {
	send_to_char("Syntax:   addrprog [wnum] [trigger] [phrase]\n\r",ch);
	return false;
    }

    if (!(tt = get_trigger_type(trigger, PRG_RPROG))) {
	send_to_char("Valid flags are:\n\r",ch);
	show_help(ch, "rprog");
	return false;
    }

    value = tt->type;
    slot = tt->slot;
	if (!wnum.pArea) wnum.pArea = pRoom->area;

	if(value == TRIG_SPELLCAST) {
		if( !str_cmp(phrase, "*") )
		{
			strcpy(phrase, "0");
		}
		else
		{
			SKILL_DATA *skill = get_skill_data(phrase);
			if(!IS_VALID(skill) || !is_skill_spell(skill)) {
				send_to_char("Invalid spell for trigger.\n\r",ch);
				return false;
			}
			sprintf(phrase,"%d",skill->uid);
		}
	}
	else if( value == TRIG_EXIT ||
			 value == TRIG_EXALL ||
			 value == TRIG_OPEN ||
			 value == TRIG_CLOSE ||
			 value == TRIG_KNOCK ||
			 value == TRIG_KNOCKING ||
			 value == TRIG_SHOWEXIT ||
			 value == TRIG_LOOK_AT)
	{
		if( !str_cmp(phrase, "*") )
		{
			strcpy(phrase, "-1");
		}
		else
		{
			int door = parse_door(phrase);
			if( door < 0 ) {
				send_to_char("Invalid direction for exit/exall/open/close/knock/knocking/showexit/look_at trigger.\n\r", ch);
				return false;
			}
			sprintf(phrase,"%d",door);
		}
	}

    if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_RPROG)) == NULL)
    {
	send_to_char("No such ROOMProgram.\n\r",ch);
	return false;
    }

    // Make sure this has a list of progs!
    if(!pRoom->progs->progs) pRoom->progs->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);
    list->script          = code;
    //SET_BIT(pMob->mprog_flags,value);
    list_appendlink(pRoom->progs->progs[slot], list);
	trigger_type_add_use(tt);

    send_to_char("Rprog Added.\n\r",ch);
    return true;
}


REDIT (redit_delrprog)
{
    ROOM_INDEX_DATA *pRoom;
    char rprog[MAX_STRING_LENGTH];
    long value;

    EDIT_ROOM(ch, pRoom);

    one_argument(argument, rprog);
    if (!is_number(rprog) || rprog[0] == '\0')
    {
	send_to_char("Syntax:  delrprog [#rprog]\n\r",ch);
	return false;
    }

    value = atol (rprog);

    if (value < 0)
    {
	send_to_char("Only non-negative rprog-numbers allowed.\n\r",ch);
	return false;
    }

    if(!edit_deltrigger(pRoom->progs->progs,value)) {
	send_to_char("No such rprog.\n\r",ch);
	return false;
    }

    send_to_char("Rprog removed.\n\r", ch);
    return true;
}


REDIT(redit_addcdesc)
{
    int value;
    ROOM_INDEX_DATA *pRoom;
    char type[MSL];
    char phrase[MSL];
    CONDITIONAL_DESCR_DATA *cd;

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, type);
    argument = one_argument(argument, phrase);

    if (type[0] == '\0' || phrase[0] == '\0')
    {
	send_to_char("Syntax: addcdesc [type] [phrase]\n\r", ch);
	return false;
    }

    if ((value = flag_value(room_condition_flags, type)) == NO_FLAG)
    {
	send_to_char("Valid condition types are:\n\r", ch);
	show_help(ch, "condition");
	return false;
    }

	int phr = cd_phrase_lookup(pRoom, value, phrase);
    if (phr == -1)
    {
	send_to_char("Invalid phrase.\n\r", ch);
	return false;
    }

    for (cd = pRoom->conditional_descr; cd != NULL; cd = cd->next)
    {
	if (cd->condition == value && cd->phrase == phr)
	{
	    send_to_char("That would be redundant.\n\r", ch);
	    return false;
	}
    }

    cd = new_conditional_descr();
    cd->condition = value;
    cd->phrase = phr;
    cd->next = pRoom->conditional_descr;
    pRoom->conditional_descr = cd;

    string_append(ch, &cd->description);

    return true;
}


REDIT(redit_dislink)
{
    ROOM_INDEX_DATA *pRoom;
    bool changed = false;

    EDIT_ROOM(ch, pRoom);

    if (!str_cmp(argument, "junk")) {
	free_string(pRoom->name);
	pRoom->name = str_dup("NULL");
	changed = true;
    }

    if (dislink_room(pRoom))
    {
	send_to_char("Room dislinked.\n\r", ch);
	changed = true;
    }
    else
	send_to_char("No exits to dislink.\n\r", ch);

    return changed;
}


REDIT(redit_delcdesc)
{
    CONDITIONAL_DESCR_DATA *cd;
    CONDITIONAL_DESCR_DATA *cd_prev;
    int i = 0;
    char cDesc[MSL];
    int value;
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    one_argument(argument, cDesc);
    if (!is_number(cDesc) || cDesc[0] == '\0')
    {
	send_to_char("Syntax: delcdesc [#cdesc]\n\r", ch);
	return false;
    }

    value = atoi(cDesc);
    if (value < 0)
    {
	send_to_char("Invalid value.\n\r", ch);
	return false;
    }

    cd_prev = NULL;
    for (cd = pRoom->conditional_descr; cd != NULL; cd = cd->next)
    {
	if (i == value)
	    break;

	cd_prev = cd;
	i++;
    }

    if (cd == NULL)
    {
	send_to_char("Conditional description not found in list.\n\r", ch);
	return false;
    }

    if (cd_prev == NULL) // head of list
    {
	pRoom->conditional_descr = cd->next;
    }
    else
    {
	cd_prev->next = cd->next;
    }

    free_conditional_descr(cd);

    send_to_char("Conditional description removed.\n\r", ch);
    return true;
}


REDIT(redit_editcdesc)
{
    CONDITIONAL_DESCR_DATA *cd;
    ROOM_INDEX_DATA *pRoom;
    int i;
    char arg[MSL];
    int num;

    EDIT_ROOM(ch, pRoom);

    one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Syntax: editcdesc [#cdesc]\n\r", ch);
	return false;
    }

    num = atoi(arg);
    if (num < 0)
    {
	send_to_char("Invalid argument.\n\r", ch);
	return false;
    }

    i = 0;
    for (cd = pRoom->conditional_descr; cd != NULL; cd = cd->next)
    {
	if (i == num)
	    break;

	i++;
    }

    if (cd == NULL)
    {
	send_to_char("Conditional description not found in list.\n\r", ch);
	return false;
    }

    string_append(ch, &cd->description);

    return true;
}