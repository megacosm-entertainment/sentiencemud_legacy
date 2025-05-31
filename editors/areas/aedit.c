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

#include "aedit.h"


AEDIT(aedit_show)
{
    AREA_DATA *pArea;
    char buf  [MAX_STRING_LENGTH];
//	ITERATOR it;
//	PROG_LIST *trigger;
	BUFFER *buffer;
	buffer = new_buf();

    EDIT_AREA(ch, pArea);

	sprintf(buf, "{X======== {W%s{X ========\n\r", pArea->name);
	add_buf(buffer, buf);

	sprintf(buf, "{WArea: {R[{X%5ld{R]{X %s {R({WID: {X%ld{R){X\n\r", pArea->anum, pArea->name, pArea->uid);
	add_buf(buffer, buf);

	sprintf(buf, "\n\r{WSystem Infomation:{X\n\r");
	add_buf(buffer, buf);

    sprintf(buf, "{WFile:        {R[{X%s{R]{X\n\r", pArea->file_name);
	add_buf(buffer, buf);

    sprintf(buf, "{WAge:         {R[{X%d{R]{X\n\r",	pArea->age);
	add_buf(buffer, buf);

    sprintf(buf, "{WRepop:       {R[{X%d minutes{R]{X\n\r", pArea->repop);
	add_buf(buffer, buf);

    sprintf(buf, "{WPlayers:     {R[{X%d{R]{X\n\r", pArea->nplayer);
	add_buf(buffer, buf);

	sprintf(buf, "{WCredits:     {R[{X%s{R]{X\n\r", pArea->credits);
	add_buf(buffer, buf);

    sprintf(buf, "{WFlags:       {R[{X%s{R]{X\n\r",
		   flag_string(area_flags, pArea->area_flags));
	add_buf(buffer, buf);

    sprintf(buf, "{WOpen:        {R[{X%s{R]{X\n\r", pArea->open ? "Yes" : "No");
	add_buf(buffer, buf);


//
// OLC Data
//

	sprintf(buf, "\n\r{WOLC Info:{X\n\r");
	add_buf(buffer, buf);

	sprintf(buf, "{WRepop:       {R[{X%d minutes{R]{X\n\r", pArea->repop);
	add_buf(buffer, buf);

	sprintf(buf, "{WSecurity:    {R[{X%d{R]{X\n\r", pArea->security);
	add_buf(buffer, buf);

	sprintf(buf, "{WBuilders:    {R[{X%s{R]{X\n\r", pArea->builders);
	add_buf(buffer, buf);

	sprintf(buf, "{WSuggested Levels:  {R[{X%d-%d{R]{X\n\r", pArea->min_level, pArea->max_level);
	add_buf(buffer, buf);

    sprintf(buf, "Security:    [%d]\n\r", pArea->security);
	add_buf(buffer, buf);

    sprintf(buf, "Builders:    [%s]\n\r", pArea->builders);
	add_buf(buffer, buf);

    sprintf(buf, "Credits:     [%s]\n\r", pArea->credits);
	add_buf(buffer, buf);

    sprintf(buf, "Flags:       [%s]\n\r", flag_string(area_flags, pArea->area_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Description:\n\r%s\n\r", pArea->description);
	add_buf(buffer, buf);

	sprintf(buf, "\n\r{WPlayer Notes:{X\n\r%s\n\r", pArea->notes);
	add_buf(buffer, buf);

	if (!IS_NULLSTR(pArea->comments))
	{
		sprintf(buf,"\n\r-----\n\r{WBuilders' Comments:{X\n\r%s{x\n\r-----\n\r", pArea->comments);
		add_buf(buffer, buf);
	}

	sprintf(buf, "Default Region: %s\n\r", pArea->region.name);
	add_buf(buffer, buf);
	__aedit_show_region(buffer, pArea, &pArea->region);

	ITERATOR rit;
	AREA_REGION *region;
	int region_no = 1;
	iterator_start(&rit, pArea->regions);
	while((region = (AREA_REGION *)iterator_nextdata(&rit)))
	{
		sprintf(buf, "Region %d: %s\n\r", region_no++, region->name);
		add_buf(buffer, buf);

		__aedit_show_region(buffer, pArea, region);
	}
	iterator_stop(&rit);

	if (pArea->progs->progs)
		olc_show_progs(buffer, pArea->progs->progs, PRG_APROG, "AreaProg Vnum");

	olc_show_index_vars(buffer, pArea->index_vars);

    // Trade stuff. One trade center per area at most
    if (pArea->trade_list != NULL)
    {
        TRADE_ITEM *temp;
        send_to_char("Trade Items available within this area:\n\r", ch);
	 	send_to_char("{MName               Obj_Vnum Rep.Time Rep.Amount  Max_Qty Min_Price Max_Price{x\n\r", ch);
        temp = pArea->trade_list;

        while(temp != NULL)
		{
			sprintf(buf, "%-18s %ld#%ld %-10ld %-10ld %-10ld %-6ld %ld\n\r", trade_table[temp->trade_type].name, temp->obj_wnum.auid, temp->obj_wnum.vnum, temp->replenish_time, temp->replenish_amount, temp->max_qty, temp->min_price, temp->max_price);
			add_buf(buffer, buf);
			temp = temp->next;
		}
    }

	if (pArea->progs->progs)
		olc_show_progs(buffer, pArea->progs->progs, PRG_APROG, "AreaProg Vnum");

	if (pArea->index_vars)
		olc_show_index_vars(buffer, pArea->index_vars);

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


AEDIT(aedit_flags)
{
    AREA_DATA *area;
    int value;

    EDIT_AREA(ch, area);

    if ((value = flag_value(area_flags, argument)) != NO_FLAG)
    {
	TOGGLE_BIT(area->area_flags, value);

	send_to_char("Flag toggled.\n\r", ch);
	return true;
    }
    else
    {
	send_to_char("No such flag.\n\r", ch);
	return false;
    }

    return false;
}


AEDIT(aedit_x)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
	send_to_char("Syntax:  x [#x coord on map]\n\r", ch);
	return false;
    }

    pArea->region.rs_x = atoi(argument);
    send_to_char("X Coordinate of Area set.\n\r", ch);

    return true;
}


AEDIT(aedit_y)
{
    AREA_DATA *pArea;
    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
	send_to_char("Syntax:  y [#y coord on map]\n\r", ch);
	return false;
    }

    pArea->region.rs_y = atoi(argument);
    send_to_char("Y Coordinate of Area set.\n\r", ch);

    return true;
}


AEDIT(aedit_land_x)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
	send_to_char("Syntax:  landx [#x coord on map]\n\r", ch);
	return false;
    }

    pArea->region.rs_land_x = atoi(argument);
    send_to_char("X Coordinate set.\n\r", ch);

    return true;
}


AEDIT(aedit_land_y)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
	send_to_char("Syntax:  landy [#y coord on map]\n\r", ch);
	return false;
    }

    pArea->region.rs_land_y = atoi(argument);
    send_to_char("Y Coordinate set.\n\r", ch);

    return true;
}

AEDIT(aedit_wilds)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
		send_to_char("Syntax:  wilds [map uid]\n\r", ch);
		return false;
    }

    long wuid = atol(argument);

    if( !get_wilds_from_uid(NULL, wuid) )
    {
		send_to_char("Invalid wilds map.\n\r", ch);
		return false;
	}

    pArea->wilds_uid = wuid;
    send_to_char("Wilderness Map UID set set.\n\r", ch);

    return true;
}


AEDIT(aedit_airshipland)
{
    AREA_DATA *pArea;
    char buf[MSL];

    EDIT_AREA(ch, pArea);

    if (!is_number(argument))
    {
	send_to_char("Syntax:  airshipland [vnum]\n\r", ch);
	return false;
    }

	long vnum = atol(argument);
	ROOM_INDEX_DATA *room = get_room_index(pArea, vnum);

    if (!room) {
		send_to_char("That room doesn't exist.\n\r", ch);
		return false;
    }

	if (room->region != &pArea->region)
	{
		send_to_char("Room is not in the default region.\n\r", ch);
		return false;
	}

    pArea->region.rs_airship_land_spot = atol(argument);
    sprintf(buf, "Set airship land spot of %s to %ld - %s\n\r",
        pArea->name, atol(argument), get_room_index(pArea,atol(argument))->name);
    send_to_char(buf, ch);
    return true;
}

AEDIT( aedit_add_trade )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;

    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char arg4[MAX_STRING_LENGTH];
    char arg5[MAX_STRING_LENGTH];
    char arg6[MAX_STRING_LENGTH];

    long replenish_time;
    long replenish_amount;
    long max_qty;
    long min_price;
    long max_price;
	long obj_vnum;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);
    argument = one_argument( argument, arg3);
    argument = one_argument( argument, arg4);
    argument = one_argument( argument, arg5);
    argument = one_argument( argument, arg6);

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' ||
			arg5[0] == '\0' || arg6[0] == '\0' )
    {
	send_to_char("addtrade obj_vnum replenish_time replenish_amount max_qty min_price max_price\n\r", ch);
	return false;
    }

	obj_vnum = atoi( arg1 );
	replenish_time = atoi( arg2 );
	replenish_amount = atoi( arg3 );
	max_qty = atoi( arg4 );
	min_price = atoi( arg5 );
	max_price = atoi( arg6 );

	if ( ( pObj = get_obj_index( pArea, obj_vnum ) ) == NULL )
	{
	    send_to_char( "That object does not exist!\n\r", ch );
	    return false;
	}

	if ( pObj->value[0] == TRADE_NONE )
	{
		send_to_char( "That is not a valid trade item.\n\r", ch );
		return false;
	}

    new_trade_item(pArea, pObj->value[0], replenish_time, replenish_amount, max_qty, min_price, max_price, obj_vnum);
    send_to_char("Trade item added.\n\r", ch);

    return true;
}


AEDIT( aedit_set_trade)
{
    return false;
}


AEDIT( aedit_view_trade )
{
	char buf[MAX_STRING_LENGTH];
    AREA_DATA *pTArea;
    TRADE_ITEM *pItem;
	int i = 0;

    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

	if ( ( arg[0] == '\0' ) || (( i = get_trade_item( arg )) == 0 ) )
	{
		send_to_char("viewtrade 'trade type'\n\r\n\rAvailable trade types are:\n", ch);

		while( trade_table[ i ].trade_type != TRADE_LAST )
		{
			send_to_char( trade_table[ i ].name, ch );
			send_to_char( "\n\r", ch );
			i++;
		}

		return false;
	}

	sprintf( buf, "Showing all trade types across areas for {G%s{x:\n\r",
			trade_table[i].name );
	send_to_char( buf, ch );
	send_to_char( "\n\rArea            Type        Rep. Amt.   Rep Time.   Qty    MaxQty     Min    Max    Buy    Sell\n\r", ch );
	send_to_char( "----------------------------------------------------------------------------------------------------\n\r", ch );

    for ( pTArea = area_first; pTArea != NULL; pTArea = pTArea->next )
    {
		for ( pItem = pTArea->trade_list; pItem != NULL; pItem = pItem->next )
		{
			if ( pItem->trade_type == i )
			{
				sprintf( buf, "%-16s %-15s %-12ld %-9ld %-7ld %-7ld %-7ld %-7ld %-7ld %-7ld\n\r",
						pTArea->name, ( pItem->replenish_amount > 0 ) ? "{RSupplier{x" : "{GConsumer{x",
						pItem->replenish_amount,
						pItem->replenish_time,
						pItem->qty,
						pItem->max_qty,
						pItem->min_price,
						pItem->max_price,
						pItem->buy_price,
						pItem->sell_price );
				send_to_char( buf, ch );
			}
		}
	}

	return true;
}


AEDIT( aedit_remove_trade)
{
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    TRADE_ITEM *type;
    TRADE_ITEM *temp;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, arg1);
    //sprintf(buf, "%s %s %s %s\n\r", arg1, arg2, arg3, arg4);
    //send_to_char(buf, ch);
    if ( arg1[0] == '\0')// || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0')
    {
	send_to_char("removetrade name\n\r", ch);
	return false;
    }

    type = find_trade_item(pArea, arg1);

    if (type == NULL)
    {
	send_to_char("That is not a valid trade item.\n\r", ch);
	return false;
    }


    if  ( type == pArea->trade_list )
    {
	pArea->trade_list = type->next;
    }
    else
	for ( temp = pArea->trade_list; temp; temp = temp->next )
	{
	    if ( temp->next == type )
	    {
		temp->next = type->next;
		break;
	    }
	}

    free_trade_item(type);
    send_to_char("Trade item removed.\n\r", ch);

    return false;
}

AEDIT(aedit_open)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:   open [Yes/No]\n\r", ch);
	return false;
    }

    if (!str_prefix(argument, "yes"))
    {
	pArea->open = true;
    }
    else
    {
	pArea->open = false;
    }

    send_to_char("Open set.\n\r", ch);
    return true;
}


AEDIT(aedit_create)
{
    AREA_DATA *pArea;

    pArea               =   new_area();
    pArea->uid = gconfig.next_area_uid++;
    gconfig_write();
    area_last->next     =   pArea;
    area_last           =   pArea;      /* Thanks, Walker. */
	olc_set_editor(ch, ED_AREA, pArea);

    SET_BIT(pArea->area_flags, AREA_ADDED);
    send_to_char("Area Created.\n\r", ch);
    return false;
}

AEDIT(aedit_regions)
{
	AREA_DATA *pArea;
	char buf[MSL];
	char arg1[MIL];
	char arg2[MIL];

	EDIT_AREA(ch, pArea);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  regions add <name>\n\r", ch);
		send_to_char("         regions remove <#>\n\r", ch);
		send_to_char("         regions name <#|default> <name>\n\r", ch);
		send_to_char("         regions description <#|default>\n\r", ch);
		send_to_char("         regions comments <#|default>\n\r", ch);
		send_to_char("         regions airship <#> <vnum or none> {Y**{x\n\r", ch);
		send_to_char("         regions flags <#|default> <flags>\n\r", ch);
		send_to_char("         regions landing <#> <x> <y>\n\r", ch);
		send_to_char("         regions landing <#> none\n\r", ch);
		send_to_char("         regions place <#> <place type>\n\r", ch);
		send_to_char("         regions postoffice <#> <vnum or none> {Y**{x\n\r", ch);
		send_to_char("         regions recall <#> <vnum|none|default> {Y**{x\n\r", ch);
		send_to_char("         regions savage <#> <0-5>\n\r", ch);
		send_to_char("         regions who <#> <area who>\n\r", ch);
		send_to_char("         regions xy <#> <x> <y>\n\r", ch);
		send_to_char("         regions xy <#> none\n\r", ch);
		send_to_char("\n\r", ch);
		send_to_char(" {Y**{x - Room vnum must be assigned to the region.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg1);
	if (!str_prefix(arg1, "add"))
	{
		if (IS_NULLSTR(argument))
		{
			send_to_char("Syntax:  regions add <name>\n\r", ch);
			send_to_char("Please specify a name.\n\r", ch);
			return false;
		}

		AREA_REGION *region = new_area_region();
		region->uid = ++pArea->top_region_uid;
		free_string(region->name);
		region->name = str_dup(argument);

		// Copy some settings from the default region
		region->area_who = pArea->region.area_who;
		region->rs_place_flags = pArea->region.rs_place_flags;
		region->rs_x = pArea->region.rs_x;
		region->rs_y = pArea->region.rs_y;
		region->rs_land_x = pArea->region.rs_land_x;
		region->rs_land_y = pArea->region.rs_land_y;

		list_appendlink(pArea->regions, region);
		sprintf(buf, "Region %d added.\n\r", list_size(pArea->regions));
		send_to_char(buf, ch);
		return true;
	}

	if (!str_prefix(arg1, "remove"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("There are no regions to remove.\n\r", ch);
			return false;
		}

		if (!is_number(argument))
		{
			send_to_char("Syntax:  regions remove <#>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			return false;
		}

		int region_no = atoi(argument);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions remove <#>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			return false;
		}

		AREA_REGION *remove_me = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		// Transfer all rooms from this region to the default region.
		ITERATOR rit;
		ROOM_INDEX_DATA *room;
		iterator_start(&rit, remove_me->rooms);
		while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
		{
			__region_add_room(&pArea->region, room);
		}
		iterator_stop(&rit);

		// Transfer all players from this region to the default region.
		ITERATOR pit;
		CHAR_DATA *player;
		iterator_start(&pit, remove_me->players);
		while((player = (CHAR_DATA *)iterator_nextdata(&pit)))
		{
			list_appendlink(pArea->region.players, player);
		}
		iterator_stop(&pit);
		
		list_remnthlink(pArea->regions, region_no, true);
		sprintf(buf, "Region %d removed.\n\r", region_no);
		send_to_char(buf, ch);
		return true;
	}

	if (!str_prefix(arg1, "name"))
	{
		argument = one_argument(argument, arg2);

		if (IS_NULLSTR(argument))
		{
			send_to_char("Name must not be empty.\n\r", ch);
			return false;
		}

		AREA_REGION *region;
		if (!str_prefix(arg2, "default"))
		{
			region = &pArea->region;
		}
		else
		{
			if (list_size(pArea->regions) < 1)
			{
				send_to_char("Area has no regions defined.\n\r", ch);
				return false;
			}

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  regions name <#|default> <name>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			int region_no = atoi(arg2);
			if (region_no < 1 || region_no > list_size(pArea->regions))
			{
				send_to_char("Syntax:  regions name <#> <name>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			if (IS_NULLSTR(argument))
			{
				send_to_char("Syntax:  regions name <#> <name>\n\r", ch);
				send_to_char("Please specify a name.\n\r", ch);
				return false;
			}

			region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		}

		free_string(region->name);
		region->name = str_dup(argument);

		send_to_char("Region name changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "description"))
	{
		argument = one_argument(argument, arg2);

		AREA_REGION *region;
		if (!str_prefix(arg2, "default"))
		{
			region = &pArea->region;
		}
		else
		{
			if (list_size(pArea->regions) < 1)
			{
				send_to_char("Area has no regions defined.\n\r", ch);
				return false;
			}

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  regions description <#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			int region_no = atoi(arg2);
			if (region_no < 1 || region_no > list_size(pArea->regions))
			{
				send_to_char("Syntax:  regions description <#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		}

		string_append(ch, &region->description);
		return true;
	}

	if (!str_prefix(arg1, "comments"))
	{
		argument = one_argument(argument, arg2);

		AREA_REGION *region;
		if (!str_prefix(arg2, "default"))
		{
			region = &pArea->region;
		}
		else
		{
			if (list_size(pArea->regions) < 1)
			{
				send_to_char("Area has no regions defined.\n\r", ch);
				return false;
			}

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  regions comments <#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			int region_no = atoi(arg2);
			if (region_no < 1 || region_no > list_size(pArea->regions))
			{
				send_to_char("Syntax:  regions comments <#>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		}
		string_append(ch, &region->comments);
		return true;
	}

	if (!str_prefix(arg1, "flags"))
	{
		argument = one_argument(argument, arg2);

		AREA_REGION *region;
		if (!str_prefix(arg2, "default"))
		{
			region = &pArea->region;
		}
		else
		{
			if (list_size(pArea->regions) < 1)
			{
				send_to_char("Area has no regions defined.\n\r", ch);
				return false;
			}

			if (!is_number(arg2))
			{
				send_to_char("Syntax:  regions flags <#> <flags>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			int region_no = atoi(arg2);
			if (region_no < 1 || region_no > list_size(pArea->regions))
			{
				send_to_char("Syntax:  regions flags <#> <flags>\n\r", ch);
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
				send_to_char(buf, ch);
				return false;
			}

			region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		}

		long value;
		if ((value = flag_value(area_region_flags, argument)) == NO_FLAG)
		{
			send_to_char("Syntax:  regions flags <#> <flags>\n\r", ch);
			send_to_char("Please type '? area_region_flags' to see a list of values.\n\r", ch);
			return false;
		}

		TOGGLE_BIT(region->flags, value);
		send_to_char("Region flags toggled.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "recall"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions recall <#> <vnum|default>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions recall <#> <vnum|default>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		long recall;
		if (!str_prefix(argument, "default"))
		{
			// Use the area's default region's recall
			recall = 0L;
		}
		else if (!is_number(argument))
		{
			send_to_char("Syntax:  regions recall <#> <vnum>\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}
		else
		{
			recall = atol(argument);
			if (recall < 1)
			{
				send_to_char("Syntax:  regions recall <#> <vnum>\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			ROOM_INDEX_DATA *room = get_room_index(pArea, recall);
			if (!room)
			{
				send_to_char("That room does not exist.\n\r", ch);
				return false;
			}

			if (room->region != region)
			{
				send_to_char("That room is not assigned to that region.\n\r", ch);
				return false;
			}
		}

		location_set(&region->rs_recall, pArea, 0, recall, 0, 0);
		send_to_char("Region Recall point changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "airship"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions airship <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions airship <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		if (!str_prefix(argument, "none"))
		{
			region->rs_airship_land_spot = 0;	// Clear it
		}
		else if (!is_number(argument))
		{
			send_to_char("Syntax:  regions airship <#> <vnum or none>\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}
		else
		{
			long vnum = atol(argument);
			if (vnum < 1)
			{
				send_to_char("Syntax:  regions airship <#> <vnum or none>\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			ROOM_INDEX_DATA *room = get_room_index(pArea, vnum);
			if (!room)
			{
				send_to_char("That room does not exist.\n\r", ch);
				return false;
			}

			if (room->region != region)
			{
				send_to_char("Room is in a different region.\n\r", ch);
				return false;
			}

			region->rs_airship_land_spot = room->vnum;
		}

		send_to_char("Region Airship landing room changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "landing"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions landing <#> <x> <y>\n\r", ch);
			send_to_char("         regions landing <#> none\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions landing <#> <x> <y>\n\r", ch);
			send_to_char("         regions landing <#> none\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		if (!str_prefix(argument, "none"))
		{
			region->rs_land_x = -1;
			region->rs_land_y = -1;
		}
		else
		{
			char arg3[MIL];
			argument = one_argument(argument, arg3);

			if (!is_number(arg3) || !is_number(argument))
			{
				send_to_char("Syntax:  regions landing <#> <x> <y>\n\r", ch);
				send_to_char("Please specify a number.\n\r", ch);
				return false;
			}

			int x = atoi(arg3);
			int y = atoi(argument);

			if (x < 0 || y < 0)
			{
				send_to_char("Syntax:  regions landing <#> <x> <y>\n\r", ch);
				send_to_char("Number cannot be negative.\n\r", ch);
				return false;
			}

			region->rs_land_x = x;
			region->rs_land_y = y;
		}

		send_to_char("Region landing coordinates changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "place"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions place <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions place <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		int value;

		if(!str_cmp(argument, "none"))
		{
			region->rs_place_flags = PLACE_NOWHERE;

			send_to_char("Region place type cleared.\n\r", ch);
			return true;
		}
		else if ((value = flag_value(place_flags, argument)) != NO_FLAG)
		{
			region->rs_place_flags = value;

			send_to_char("Region place type set.\n\r", ch);
			return true;
		}

		send_to_char("Syntax:  regions place <#> <flag>\n\r", ch);
		send_to_char("Type '? placetype' for a list of possible values.\n\r", ch);
    	return false;
	}

	if (!str_prefix(arg1, "postoffice"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions postoffice <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions postoffice <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		if (!str_prefix(argument, "none"))
		{
			region->post_office = 0;	// Clear it
		}
		else if (!is_number(argument))
		{
			send_to_char("Syntax:  regions postoffice <#> <vnum or none>\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}
		else
		{
			long vnum = atol(argument);
			if (vnum < 1)
			{
				send_to_char("Syntax:  regions postoffice <#> <vnum or none>\n\r", ch);
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			ROOM_INDEX_DATA *room = get_room_index(pArea, vnum);
			if (!room)
			{
				send_to_char("That room does not exist.\n\r", ch);
				return false;
			}

			if (room->region != region)
			{
				send_to_char("Room is in a different region.\n\r", ch);
				return false;
			}

			region->post_office = room->vnum;
		}

		send_to_char("Region Post Office changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "savage"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions savage <#> <0-5>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions savage <#> <0-5>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		if (!is_number(argument))
		{
			send_to_char("Syntax:  regions savage <#> <0-5>\n\r", ch);
			send_to_char("Please specify a number from 0 to 5.\n\r", ch);
			return false;
		}

		int level = atoi(argument);
		if (level < 0 || level > 5)
		{
			send_to_char("Syntax:  regions savage <#> <0-5>\n\r", ch);
			send_to_char("Please specify a number from 0 to 5.\n\r", ch);
			return false;
		}

		region->rs_savage_level = level;

		send_to_char("Region Savagery changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg1, "who"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions who <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions who <#> <vnum or none>\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);
		int value;

		if ( !str_prefix(argument, "blank") )
		{
			region->area_who = AREA_BLANK;

			send_to_char("Region who title cleared.\n\r", ch);
			return true;
		}

		if ((value = flag_value(area_who_titles, argument)) != NO_FLAG)
		{
			if( value == AREA_INSTANCE || value == AREA_DUTY )
			{
				send_to_char("That who title only allowed in blueprints.\n\r", ch);
				return false;
			}

			region->area_who = value;

			send_to_char("Region who title set.\n\r", ch);
			return true;
		}

		send_to_char("Syntax:  regions who <#> <flag>\n\r", ch);
		send_to_char("Type '? areawho' for a list of possible values.\n\r", ch);
    	return false;
	}

	if (!str_prefix(arg1, "xy"))
	{
		if (list_size(pArea->regions) < 1)
		{
			send_to_char("Area has no regions defined.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg2);

		if (!is_number(arg2))
		{
			send_to_char("Syntax:  regions xy <#> <x> <y>\n\r", ch);
			send_to_char("         regions xy <#> none\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		int region_no = atoi(arg2);
		if (region_no < 1 || region_no > list_size(pArea->regions))
		{
			send_to_char("Syntax:  regions xy <#> <x> <y>\n\r", ch);
			send_to_char("         regions xy <#> none\n\r", ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(pArea->regions));
			send_to_char(buf, ch);
			return false;
		}

		AREA_REGION *region = (AREA_REGION *)list_nthdata(pArea->regions, region_no);

		if (!str_prefix(argument, "none"))
		{
			region->rs_x = -1;
			region->rs_y = -1;
		}
		else
		{
			char arg3[MIL];
			argument = one_argument(argument, arg3);

			if (!is_number(arg3) || !is_number(argument))
			{
				send_to_char("Syntax:  regions xy <#> <x> <y>\n\r", ch);
				send_to_char("Please specify a number.\n\r", ch);
				return false;
			}

			int x = atoi(arg3);
			int y = atoi(argument);

			if (x < 0 || y < 0)
			{
				send_to_char("Syntax:  regions xy <#> <x> <y>\n\r", ch);
				send_to_char("Number cannot be negative.\n\r", ch);
				return false;
			}

			region->rs_x = x;
			region->rs_y = y;
		}

		send_to_char("Region coordinates changed.\n\r", ch);
		return true;
	}

	aedit_regions(ch, "");
	return false;
}




AEDIT(aedit_savage)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  savage <0-5>\n\r", ch);
		return false;
	}

	if (!is_number(argument))
	{
		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	int level = atoi(argument);
	if (level < 0 || level > 5)
	{
		send_to_char("Savage level must be from 0 to 5.\n\r", ch);
		return false;
	}

	pArea->region.rs_savage_level = level;
	send_to_char("Savage level set.\n\r", ch);
	return true;
}

AEDIT(aedit_name)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
        send_to_char("Syntax:   name [$name]\n\r", ch);
        return false;
    }

    free_string(pArea->name);
    pArea->name = str_dup(argument);

    send_to_char("Name set.\n\r", ch);
    return true;
}

AEDIT(aedit_desc)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
		string_append(ch, &pArea->description);
		return true;
    }

    send_to_char("Syntax:  desc\n\r", ch);
    return false;
}

AEDIT(aedit_comments)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
		string_append(ch, &pArea->comments);
		return true;
    }

    send_to_char("Syntax:  comment\n\r", ch);
    return false;
}

AEDIT(aedit_notes)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
	string_append(ch, &pArea->notes);
	return TRUE;
    }

    send_to_char("Syntax:  notes\n\r", ch);
    return FALSE;
}

AEDIT(aedit_repop)
{
    AREA_DATA *pArea;
    int value;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
        send_to_char("Syntax: repop [#mins]\n\r", ch);
        return false;
    }

    if (!is_number(argument))
    {
	send_to_char("That's not a number!\n\r", ch);
	return false;
    }

    if ((value = atoi(argument)) < 5 || value > 120)
    {
	send_to_char("Value is out of range. Range is 5-120 minutes.\n\r", ch);
	return false;
    }

    pArea->rs_repop = value;
    send_to_char("Repop time set.\n\r", ch);
    return true;
}


AEDIT(aedit_credits)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:   credits [$credits]\n\r", ch);
	return false;
    }

    free_string(pArea->credits);
    pArea->credits = str_dup(argument);

    send_to_char("Credits set.\n\r", ch);
    return true;
}


AEDIT(aedit_areawho)
{
    AREA_DATA *pArea;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_AREA(ch, pArea);

		if ( !str_prefix(argument, "blank") )
		{
			pArea->region.area_who = AREA_BLANK;

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

			pArea->region.area_who = value;

			send_to_char("Area who title set.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax:  areawho [title]\n\r"
		  "Type '? areawho' for a list of who titles.\n\r", ch);
    return false;
}

AEDIT(aedit_placetype)
{
    AREA_DATA *pArea;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_AREA(ch, pArea);

		if(!str_cmp(argument, "none")) {
			pArea->region.rs_place_flags = PLACE_NOWHERE;

			send_to_char("Area place type cleared.\n\r", ch);
			return true;
		} else if ((value = flag_value(place_flags, argument)) != NO_FLAG) {
			pArea->region.rs_place_flags = value;

			send_to_char("Area place type set.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax:  placetype [flag]\n\r"
		  "Type '? placetype' for a list of possible values.\n\r", ch);
    return false;
}


AEDIT(aedit_file)
{
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument(argument, file);	/* Forces Lowercase */

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  filename [$file]\n\r", ch);
	return false;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen(argument);
    if (length > 12)
    {
	send_to_char("No more than twelve characters allowed.\n\r", ch);
	return false;
    }

    /*
     * Allow only letters and numbers.
     */
    for (i = 0; i < length; i++)
    {
	if (!ISALNUM(file[i]))
	{
	    send_to_char("Only letters and numbers are valid.\n\r", ch);
	    return false;
	}
    }

    free_string(pArea->file_name);
    strcat(file, ".are");
    pArea->file_name = str_dup(file);

    send_to_char("Filename set.\n\r", ch);
    return true;
}


AEDIT(aedit_age)
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument(argument, age);

    if (!is_number(age) || age[0] == '\0')
    {
	send_to_char("Syntax:  age [#xage]\n\r", ch);
	return false;
    }

    pArea->age = atoi(age);

    send_to_char("Age set.\n\r", ch);
    return true;
}


AEDIT(aedit_recall)
{
	AREA_DATA *pArea;
	char arg1[MIL];
	char arg2[MIL];
	char arg3[MIL];
	char arg4[MIL];
	int vnum, x, y, z;

	EDIT_AREA(ch, pArea);

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
		location_clear(&pArea->region.rs_recall);
		send_to_char("Recall cleared.\n\r", ch);
	} else if(!arg2[0]) {
		if(!get_room_index(pArea,vnum)) {
			send_to_char("AEdit:  Room vnum does not exist.\n\r", ch);
			return false;
		}

		location_set(&pArea->region.rs_recall,pArea,0,vnum,0,0);
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
		location_set(&pArea->region.rs_recall,NULL,vnum,x,y,z);
		send_to_char("Recall set.\n\r", ch);
	}

	return true;
}


AEDIT(aedit_security)
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument(argument, sec);

    if (!is_number(sec) || sec[0] == '\0')
    {
	send_to_char("Syntax:  security [#xlevel]\n\r", ch);
	return false;
    }

    value = atoi(sec);

    if (value > ch->pcdata->security || value < 0)
    {
	if (ch->pcdata->security != 0)
	{
	    sprintf(buf, "Security is 0-%d.\n\r", ch->pcdata->security);
	    send_to_char(buf, ch);
	}
	else
	    send_to_char("Security is 0 only.\n\r", ch);
	return false;
    }

    pArea->security = value;

    send_to_char("Security set.\n\r", ch);
    return true;
}


AEDIT(aedit_builder)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name);

	if (name[0] == '\0')
	{
		send_to_char("Syntax:  builder [$name]  -toggles builder\n\r", ch);
		send_to_char("Syntax:  builder All      -allows everyone\n\r", ch);
		return false;
	}

	name[0] = UPPER(name[0]);

	if (strstr(pArea->builders, name) != NULL)
	{
		pArea->builders = string_replace(pArea->builders, name, "\0");
		pArea->builders = string_unpad(pArea->builders);

		if (pArea->builders[0] == '\0')
		{
			free_string(pArea->builders);
			pArea->builders = str_dup("None");
		}
		send_to_char("Builder removed.\n\r", ch);
		return true;
	}
	else
	{
		buf[0] = '\0';

		if (!player_exists(name) && str_cmp(name, "All"))
		{
			act("There is no character by the name of $t.", ch, NULL, NULL, NULL, NULL, name, NULL, TO_CHAR, NULL, NULL);
			return false;
		}

		if (strstr(pArea->builders, "None") != NULL)
		{
			pArea->builders = string_replace(pArea->builders, "None", "\0");
			pArea->builders = string_unpad(pArea->builders);
		}

		if (pArea->builders[0] != '\0')
		{
			strcat(buf, pArea->builders);
			strcat(buf, " ");
		}

		strcat(buf, name);
		free_string(pArea->builders);
		pArea->builders = string_proper(str_dup(buf));

		send_to_char("Builder added.\n\r", ch);
		send_to_char(pArea->builders,ch);
		return true;
	}

	return false;
}

AEDIT(aedit_levels)
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || lower[0] == '\0'
    || !is_number(upper) || upper[0] == '\0')
    {
	send_to_char("Syntax:  levels [#xlower] [#xupper]\n\r", ch);
	return FALSE;
    }

    if ((ilower = atoi(lower)) > (iupper = atoi(upper)))
    {
	send_to_char("AEdit:  Upper must be larger then lower.\n\r", ch);
	return FALSE;
    }

    if ((ilower = atoi(lower)) > 120 || (iupper = atoi(upper)) < 1)
    {
	send_to_char("AEdit:  Range must be between 1 and 120.\n\r", ch);
	return FALSE;
    }

    pArea->min_level = ilower;
    send_to_char("Lower level set.\n\r", ch);

    pArea->max_level = iupper;
    send_to_char("Upper level set.\n\r", ch);

    return TRUE;
}

AEDIT(aedit_postoffice)
{
    AREA_DATA *pArea;
    long vnum;
    char buf[MSL];
    ROOM_INDEX_DATA *room;

    EDIT_AREA(ch, pArea);

    if (argument[0] == '\0') {
	send_to_char("Syntax:   postoffice <vnum in the area>\n\r", ch);
	return false;
    }

	vnum = atol(argument);

    if ((room = get_room_index(pArea, vnum)) == NULL) {
		send_to_char("That room vnum doesn't exist.\n\r", ch);
		return false;
    }

	if (room->region != &pArea->region)
	{
		send_to_char("Room is not in the default region.\n\r", ch);
		return false;
	}

    sprintf(buf, "Set post office of %s to %s(%ld)\n\r", pArea->name, room->name, vnum);
    send_to_char(buf, ch);

    pArea->region.post_office = vnum;
    return true;
}

AEDIT (aedit_addaprog)
{
	struct trigger_type *tt;
    int slot;
    AREA_DATA *pArea;
    PROG_LIST *list;
    SCRIPT_DATA *code;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);
    argument = one_argument(argument, num);
    argument = one_argument(argument, trigger);
    argument = one_argument(argument, phrase);

	WNUM wnum;

    if (!parse_widevnum(num, ch->in_room->area, &wnum) || trigger[0] =='\0' || phrase[0] =='\0')
    {
	send_to_char("Syntax:   addaprog [wnum] [trigger] [phrase]\n\r",ch);
	return false;
    }

    if (!(tt = get_trigger_type(trigger, PRG_APROG))) {
	send_to_char("Valid flags are:\n\r",ch);
	show_help(ch, "aprog");
	return false;
    }

    slot = tt->slot;

	if (!wnum.pArea) wnum.pArea = pArea;


    if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_APROG)) == NULL)
    {
	send_to_char("No such AREAProgram.\n\r",ch);
	return false;
    }

    // Make sure this has a list of progs!
    if(!pArea->progs->progs) pArea->progs->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);

    list->script          = code;
    //SET_BIT(pMob->mprog_flags,value);

    list_appendlink(pArea->progs->progs[slot], list);
	trigger_type_add_use(tt);

    send_to_char("Aprog Added.\n\r",ch);
    return true;
}


AEDIT (aedit_delaprog)
{
    AREA_DATA *pArea;
    char aprog[MAX_STRING_LENGTH];
    int value;

    EDIT_AREA(ch, pArea);

    one_argument(argument, aprog);
    if (!is_number(aprog) || aprog[0] == '\0')
    {
       send_to_char("Syntax:  delaprog [#aprog]\n\r",ch);
       return false;
    }

    value = atol (aprog);

    if (value < 0)
    {
        send_to_char("Only non-negative aprog-numbers allowed.\n\r",ch);
        return false;
    }

    if(!edit_deltrigger(pArea->progs->progs,value)) {
	send_to_char("No such aprog.\n\r",ch);
	return false;
    }

    send_to_char("Aprog removed.\n\r", ch);
    return true;
}

AEDIT(aedit_varset)
{
    AREA_DATA *pArea;
 
    EDIT_AREA(ch, pArea);

	if (olc_varset(&pArea->index_vars, ch, argument, false))
	{
		// Install variable into the "live"
		olc_varset(&pArea->progs->vars, ch, argument, true);
		return true;
	}
	return false;
}

AEDIT(aedit_varclear)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

	if (olc_varclear(&pArea->index_vars, ch, argument, false))
	{
		// Clear variable on "live"
		olc_varclear(&pArea->progs->vars, ch, argument, true);
		return true;
	}

	return false;
}