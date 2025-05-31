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

#include "medit.h"


MEDIT(medit_show)
{
	MOB_INDEX_DATA *pMob;
	char buf[MAX_STRING_LENGTH];


	BUFFER *buffer;

	EDIT_MOB(ch, pMob);

	buffer = new_buf();

	sprintf(buf, "Name:         {C[{x%s{C]{x\n\rArea:         {C[{x%5ld{C]{x %s\n\r",
		pMob->player_name,
		!pMob->area ? -1        : pMob->area->uid,
		!pMob->area ? "No Area" : pMob->area->name);
	add_buf(buffer, buf);
	sprintf(buf, "Loaded:       {C[{x%d{C]{x\n\r", pMob->count);
	add_buf(buffer, buf);

	sprintf(buf, "Sig:          {C[{x%s{C]{x   Creator: {C[{x%s{C]{x\n\r",
		pMob->sig, pMob->creator_sig);
	add_buf(buffer, buf);

	sprintf(buf, "Act:          {C[{x%s{C]{x\n\r",
		bitmatrix_string(act_flagbank, pMob->act));
	add_buf(buffer, buf);

	sprintf(buf, "Vnum:         {C[{x%6ld{C]{x  Sex: {C[{x%7s{C]{x  Race: {C[{x%s{C]{x\n\r",
		pMob->vnum,
		pMob->sex == SEX_MALE    ? "male   " :
		pMob->sex == SEX_FEMALE  ? "female " :
		pMob->sex == SEX_EITHER  ? "random " : "neutral",
		pMob->race->name);
	add_buf(buffer, buf);

    sprintf(buf, "Boss:         {C[%s{C]{x\n\r", (pMob->boss ? "{RYES" : "{gno"));
    add_buf(buffer, buf);

    sprintf(buf, "Persist:      {C[%s{C]{x\n\r", (pMob->persist ? "{WON" : "{Doff"));
    add_buf(buffer, buf);

	if(pMob->attacks < 0) {
		sprintf(buf,
			"Level:        {C[{x%6d{C]{x  Align: {C[{x%6d{C]{x   Owner: {C[{x%s{C]{x\n\r"
			"Hitroll:      {C[{x%6d{C]{x  DamType: {C[{x%s{C]{x\n\r"
			"Movement:     {C[{x%6ld{C]{x  Number of attacks: {C[{Yscripted{C]{x\n\r",
			pMob->level,	pMob->alignment, pMob->owner,
			pMob->hitroll,	attack_table[pMob->dam_type].name,
			pMob->move);
	} else {
		sprintf(buf,
			"Level:        {C[{x%6d{C]{x  Align: {C[{x%6d{C]{x   Owner: {C[{x%s{C]{x\n\r"
			"Hitroll:      {C[{x%6d{C]{x  DamType: {C[{x%s{C]{x\n\r"
			"Movement:     {C[{x%6ld{C]{x  Number of attacks: {C[{x%d{C]{x\n\r",
			pMob->level,	pMob->alignment, pMob->owner,
			pMob->hitroll,	attack_table[pMob->dam_type].name,
			pMob->move, pMob->attacks );
	}
	add_buf(buffer, buf);

	sprintf(buf, "Hit dice:     {C[{x%2dd%-3d+%4d{C]{x ",
		pMob->hit.number,
		pMob->hit.size,
		pMob->hit.bonus);
	add_buf(buffer, buf);

	sprintf(buf, "Damage dice:  {C[{x%2dd%-3d+%4d{C]{x ",
		pMob->damage.number,
		pMob->damage.size,
		pMob->damage.bonus);
	add_buf(buffer, buf);

	sprintf(buf, "Mana dice:    {C[{x%2dd%-3d+%4d{C]{x\n\r",
		pMob->mana.number,
		pMob->mana.size,
		pMob->mana.bonus);
	add_buf(buffer, buf);

	sprintf(buf, "Affected by:  {C[{x%s{C]{x\n\r",
		bitvector_string(2, pMob->affected_by[0], affect_flags, pMob->affected_by[1], affect2_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Armour:        {C[{xpierce: %d  bash: %d  slash: %d  magic: %d{C]{x\n\r",
		pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
		pMob->ac[AC_SLASH],  pMob->ac[AC_EXOTIC]);
	add_buf(buffer, buf);

	sprintf(buf, "Parts:        {C[{x%s{C]{x\n\r", flag_string(part_flags, pMob->parts));
	add_buf(buffer, buf);

	sprintf(buf, "Imm:          {C[{x%s{C]{x\n\r", flag_string(imm_flags, pMob->imm_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Res:          {C[{x%s{C]{x\n\r", flag_string(res_flags, pMob->res_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Vuln:         {C[{x%s{C]{x\n\r", flag_string(vuln_flags, pMob->vuln_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Off:          {C[{x%s{C]{x\n\r", flag_string(off_flags,  pMob->off_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Size:         {C[{x%s{C]{x\n\r", flag_string(size_flags, pMob->size));
	add_buf(buffer, buf);

	if (IS_VALID(pMob->material))
		sprintf(buf, "Material:     {C[{x%s{C]{x\n\r", pMob->material->name);
	else
		sprintf(buf, "Material:     {C[{Dnothing{C]{x\n\r");
	add_buf(buffer, buf);

	sprintf(buf, "Start pos:    {C[{x%s{C]{x\n\r", flag_string(position_flags, pMob->start_pos));
	add_buf(buffer, buf);

	sprintf(buf, "Default pos:  {C[{x%s{C]{x\n\r", flag_string(position_flags, pMob->default_pos));
	add_buf(buffer, buf);

	sprintf(buf, "Wealth:       {C[{x%8ld{C]{x\n\r", pMob->wealth);
	add_buf(buffer, buf);

	sprintf(buf, "Script Kwds:  {C[{x%s{C]{x\n\r", pMob->skeywds);
	add_buf(buffer, buf);

	if (pMob->spec_fun) {
		sprintf(buf, "Spec fun:     {C[{x%s{C]{x\n\r",  spec_name(pMob->spec_fun));
		add_buf(buffer, buf);
	}

	if (IS_VALID(pMob->corpse_type))
		sprintf(buf, "Corpse Type:  {C[{x%s{C]{x\n\r", pMob->corpse_type->name);
	else
		sprintf(buf, "Corpse Type:  {C[{Dnothing{C]{x\n\r");
	add_buf(buffer, buf);

	if (pMob->corpse.auid > 0 && pMob->corpse.vnum > 0) {
		OBJ_INDEX_DATA *obj = get_obj_index_auid(pMob->corpse.auid, pMob->corpse.vnum);
		sprintf(buf, "Corpse Obj:   {C[{x%s{C]{x\n\r",  obj->short_descr);
		add_buf(buffer, buf);
	}

	if (pMob->zombie.auid > 0 && pMob->zombie.vnum > 0) {
		OBJ_INDEX_DATA *obj = get_obj_index_auid(pMob->zombie.auid, pMob->zombie.vnum);
		sprintf(buf, "Zombie Obj:   {C[{x%s{C]{x\n\r",  obj->short_descr);
		add_buf(buffer, buf);
	}


	sprintf(buf, "Short descr: %s\n\rLong descr:\n\r     %s", pMob->short_descr, pMob->long_descr);
	add_buf(buffer, buf);

	sprintf(buf, "Description:\n\r%s", pMob->description);
	add_buf(buffer, buf);

	sprintf(buf, "\n\r-----\n\r{WBuilders' Comments:{X\n\r%s\n\r-----\n\r", pMob->comments);
	add_buf(buffer, buf);

	add_buf(buffer, "Factions:\n\r");
	if (list_size(pMob->factions) > 0)
	{
		int f = 0;
		ITERATOR fit;
		REPUTATION_INDEX_DATA *faction;
		iterator_start(&fit, pMob->factions);
		while((faction = (REPUTATION_INDEX_DATA *)iterator_nextdata(&fit)))
		{
			sprintf(buf, "{x  [%3d] %s {C({x%ld{C#{x%ld{C){x\n\r",
				f++,
				faction->name,
				faction->area->uid,
				faction->vnum);
			add_buf(buffer, buf);
		}
		iterator_stop(&fit);
	}
	else
		add_buf(buffer, "{x  None\n\r");
	add_buf(buffer, "\n\r");

	if (pMob->reputations)
	{
		add_buf(buffer, "Reputation Table:\n\r");
		MOB_REPUTATION_DATA *rep;
		int iRep;
		for(iRep = 0, rep = pMob->reputations; rep; iRep++, rep = rep->next)
		{
			char repName[MIL];

			if (IS_VALID(rep->reputation))
				sprintf(repName, "%s {W({x%ld{W#{x%ld{W)",
					rep->reputation->name, rep->reputation->area->uid, rep->reputation->vnum);
			else
				strcpy(repName, "{D(invalid)");

			// Pad the text so the plain text fits within 20 characters
			int plain_len = strlen_no_colours(repName);
			if (plain_len < 20)
			{
				strcat(repName, formatf("%*.*s", 20 - plain_len, 20 - plain_len, " "));
			}
		
			REPUTATION_INDEX_RANK_DATA *min_rank =
				(IS_VALID(rep->reputation) && rep->minimum_rank > 0) ?
					(REPUTATION_INDEX_RANK_DATA *)list_nthdata(rep->reputation->ranks, rep->minimum_rank) :
					NULL;
			
			REPUTATION_INDEX_RANK_DATA *max_rank =
				(IS_VALID(rep->reputation) && rep->maximum_rank > 0) ?
					(REPUTATION_INDEX_RANK_DATA *)list_nthdata(rep->reputation->ranks, rep->maximum_rank) :
					NULL;

			add_buf(buffer, formatf("[%4d] %s {W({x%3d{W) {%c%-20s {W({x%3d{W) {%c%-20s {%c%d{x\n\r",
				iRep, repName,
				rep->minimum_rank,
				IS_VALID(min_rank) ? min_rank->color : 'x',
				IS_VALID(min_rank) ? min_rank->name : "none",
				rep->maximum_rank,
				IS_VALID(max_rank) ? max_rank->color : 'x',
				IS_VALID(max_rank) ? max_rank->name : "none",
				(rep->points < 0) ? 'R' : 'G',
				rep->points));
		}
	}

	if (pMob->pPractice)
	{
		PRACTICE_DATA *pPractice = pMob->pPractice;

		add_buf(buffer, "Practice Teaching data:\n\r");

		if(pPractice->standard)
			add_buf(buffer, "Mode: {YStandard{x\n\r");
		else
		{
			add_buf(buffer, "Mode: {GCustom{x\n\r");

			if (list_size(pPractice->entries) > 0)
			{
				add_buf(buffer, " ###  [ Type ] [        Name        ] [Max Rating] [    Rating Cost Points    ]\n\r");
				add_buf(buffer, "================================================================================\n\r");

				int iEntry = 0;
				ITERATOR peit;
				ITERATOR pcit;
				PRACTICE_ENTRY_DATA *entry;
				PRACTICE_COST_DATA *cost;
				iterator_start(&peit, pPractice->entries);
				while((entry = (PRACTICE_ENTRY_DATA *)iterator_nextdata(&peit)))
				{
					char *type;
					// This needs to be an abbreviated list.. there can be way too much
					char *name;
					char maxrating[MIL];
					char ratings[MIL * 2];

					if (entry->song)
					{
						type = "Song";
						name = entry->song->name;
					}
					else if(entry->skill)
					{
						if (entry->skill->isspell)
							type = "Spell";
						else
							type = "Skill";
						name = entry->skill->name;

					}
					else
						continue;

					int pr = 0;
					sprintf(maxrating, "%3d{W%%{x", entry->max_rating);

					// Tally up the rating points.
					iterator_start(&pcit, entry->costs);
					while((cost = (PRACTICE_COST_DATA *)iterator_nextdata(&pcit)))
					{
						if (pr > 0)
						{
							ratings[pr++] = ',';
							ratings[pr++] = ' ';
						}
						if (cost->min_rating > 0)
							pr += sprintf(&ratings[pr], "%d{W%%{x", cost->min_rating);
						else
							pr += sprintf(&ratings[pr], "{Wacquire{x");
					}
					ratings[pr] = '\0';
					iterator_stop(&pcit);

					sprintf(buf, "[%3d]   %-5s   %-20s      %s      %s\n\r", ++iEntry,
						type, name,
						maxrating,
						ratings);

					add_buf(buffer, buf);
				}
				iterator_stop(&peit);
				add_buf(buffer, "\n\r");
			}
		}
	}

	if (pMob->pShop) {
		SHOP_DATA *pShop;
		int iTrade;

		pShop = pMob->pShop;

		sprintf(buf,
			"Shop data:\n\r"
			"  Markup for purchaser: %d%%\n\r"
			"  Markdown for seller:  %d%%\n\r",
			pShop->profit_buy, pShop->profit_sell);
		add_buf(buffer, buf);
		sprintf(buf, "  Hours: %d to %d.\n\r", pShop->open_hour, pShop->close_hour);
		add_buf(buffer, buf);

		if( pShop->restock_interval > 0 )
			sprintf(buf, "  Restocking: %d (minutes)\n\r", pShop->restock_interval);
		else
			sprintf(buf, "  Restocking: disabled\n\r");
		add_buf(buffer, buf);

		sprintf(buf, "  Discount Rate: %d%%\n\r", pShop->discount);
		add_buf(buffer, buf);

		sprintf(buf, "  Flags: %s\n\r", flag_string(shop_flags, pShop->flags));
		add_buf(buffer, buf);

		if (IS_VALID(pShop->reputation))
		{
			if (pShop->min_reputation_rank > 0 && pShop->min_reputation_rank <= list_size(pShop->reputation->ranks))
			{
				REPUTATION_INDEX_RANK_DATA *shopRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(pShop->reputation->ranks, pShop->min_reputation_rank);

				if (IS_VALID(shopRank))
					sprintf(buf, "  Interaction Reputation: %s, at least {%c%s{x\n\r", pShop->reputation->name, shopRank->color ? shopRank->color : 'Y', shopRank->name);
				else
					sprintf(buf, "  Interaction Reputation: %s\n\r", pShop->reputation->name);
			}
			else
				sprintf(buf, "  Interaction Reputation: %s\n\r", pShop->reputation->name);
			
			add_buf(buffer, buf);
		}
		else
			add_buf(buffer, "  Interaction Reputation: {D(none){x\n\r");

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
			if (pShop->buy_type[iTrade]) {
				if (!iTrade) {
					add_buf(buffer, "  Number Trades Type\n\r");
					add_buf(buffer, "  ------ -----------\n\r");
				}
				sprintf(buf, "  {C[{x%4d{C]{x %s\n\r", iTrade, flag_string(type_flags, pShop->buy_type[iTrade]));
				add_buf(buffer, buf);
			}
		}

		if(pShop->shipyard > 0)
		{
			WILDS_DATA *wilds = get_wilds_from_uid(NULL, pShop->shipyard);

			sprintf(buf, "  Shipyard: %s (%ld) at (%d,%d) to (%d,%d)\n\r",
				wilds?wilds->name:"(null)", pShop->shipyard,
				pShop->shipyard_region[0][0], pShop->shipyard_region[0][1],
				pShop->shipyard_region[1][0], pShop->shipyard_region[1][1]);
			add_buf(buffer, buf);

			sprintf(buf, "            %s\n\r", pShop->shipyard_description);
			add_buf(buffer, buf);
		}

		if(pShop->stock != NULL)
		{
			SHOP_STOCK_DATA *pStock;
			int iStock;
			char lvl[MIL];
			char qty[32];
			char pricing[MIL];
			char typ[MIL];
			char hours[20];
			char item[MIL];
			char disc[MIL];
			char rep[MIL * 2];
			int hwidth, qwidth, pwidth;

			for(iStock = 1, pStock = pShop->stock;pStock;pStock = pStock->next, iStock++)
			{
				if(iStock == 1)
				{
					add_buf(buffer, "{G  Stock# Level Quantity Sng Hours    Price(s)    Disc           Reputation                            Item{x\n\r");
					add_buf(buffer, "{G  ------ ----- -------- --- ----- -------------- ---- ------------------------------ --------------------------------------{x\n\r");
				}

				if( pStock->level > 0 )
				{
					sprintf(lvl, "{Y%d{x", pStock->level);
				}
				else
				{
					strcpy(lvl, "{GAuto{x");
				}

				if (IS_VALID(pStock->reputation))
				{
					REPUTATION_INDEX_RANK_DATA *pStockMinRank, *pStockMaxRank;
					REPUTATION_INDEX_RANK_DATA *showMinRank, *showMaxRank;

					if (pStock->min_reputation_rank > 0 && pStock->min_reputation_rank <= list_size(pStock->reputation->ranks))
						pStockMinRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(pStock->reputation->ranks, pStock->min_reputation_rank);
					else
						pStockMinRank = NULL;

					if (pStock->max_reputation_rank > 0 && pStock->max_reputation_rank <= list_size(pStock->reputation->ranks))
						pStockMaxRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(pStock->reputation->ranks, pStock->max_reputation_rank);
					else
						pStockMaxRank = NULL;

					if (pStock->min_show_rank > 0 && pStock->min_show_rank <= list_size(pStock->reputation->ranks))
						showMinRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(pStock->reputation->ranks, pStock->min_show_rank);
					else
						showMinRank = NULL;

					if (pStock->max_show_rank > 0 && pStock->max_show_rank <= list_size(pStock->reputation->ranks))
						showMaxRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(pStock->reputation->ranks, pStock->max_show_rank);
					else
						showMaxRank = NULL;

					char buy[MIL];
					char see[MIL];

					if (IS_VALID(pStockMinRank))
					{
						if (IS_VALID(pStockMaxRank))
						{
							sprintf(buy, " ({%c%s{x to {%c%s{x)",
								pStockMinRank->color ? pStockMinRank->color : 'Y',
								pStockMinRank->name,
								pStockMaxRank->color ? pStockMaxRank->color : 'Y',
								pStockMaxRank->name);
						}
						else
						{
							sprintf(buy, " ({%c%s{x)",
								pStockMinRank->color ? pStockMinRank->color : 'Y',
								pStockMinRank->name);
						}
					}
					else if(pStockMaxRank)
					{
						sprintf(buy, " (to {%c%s{x)",
							pStockMaxRank->color ? pStockMaxRank->color : 'Y',
							pStockMaxRank->name);
					}
					else
						buy[0] = '\0';

					if (IS_VALID(showMinRank))
					{
						if (IS_VALID(showMaxRank))
						{
							sprintf(see, " <{%c%s{x to {%c%s{x>",
								showMinRank->color ? showMinRank->color : 'Y',
								showMinRank->name,
								showMaxRank->color ? showMaxRank->color : 'Y',
								showMaxRank->name);
						}
						else
						{
							sprintf(see, " <{%c%s{x>",
								showMinRank->color ? showMinRank->color : 'Y',
								showMinRank->name);
						}
					}
					else if(showMaxRank)
					{
						sprintf(see, " <to {%c%s{x>",
							showMaxRank->color ? showMaxRank->color : 'Y',
							showMaxRank->name);
					}
					else
						see[0] = '\0';

					sprintf(rep, "%s%s%s", pStock->reputation->name, buy, see);
				}
				else
				{
					rep[0] = '\0';
				}

				// Pad, left justified, upto 30 characters
				int repLen = strlen_no_colours(rep);
				if (repLen < 30)
				{
					strcat(rep, formatf("%*s", 30 - repLen, ""));
				}

				if( pStock->quantity > 0 )
				{
					if( pStock->restock_rate > 0 )
					{
						sprintf(qty, "{W%d{x / {W%d{x", pStock->quantity, pStock->restock_rate);
					}
					else
					{
						sprintf(qty, "{W%d{x / {D--{x", pStock->quantity);
					}
				}
				else
				{
					strcpy(qty, "{D   {D--{x   {x");
				}
				qwidth = 16;

				if( pStock->duration > 0 )
				{
					sprintf(hours, "{G%d{x", pStock->duration);
				}
				else
				{
					strcpy(hours, " {D---{x ");
				}
				hwidth = 9;

				if( !IS_NULLSTR(pStock->custom_price) )
				{
					strncpy(pricing, pStock->custom_price, sizeof(pricing)-3);
					strcat(pricing, "{x");
					strcpy(disc, " {D--{x ");
				}
				else
				{
					pricing[0] = '\0';
					int pj = 0;

					if( pStock->silver > 0)
					{
						long silver = pStock->silver % 100;
						long gold = pStock->silver / 100;

						if( gold > 0 )
						{
							if( silver > 0 )
							{
								pj = sprintf(pricing, "{x%ld{Yg{x%ld{Ws{x", gold, silver);
							}
							else
							{
								pj = sprintf(pricing, "{x%ld{Yg{x", gold);
							}
						}
						else
						{
							pj = sprintf(pricing, "{x%ld{Ws{x", silver);
						}
					}

					if( pStock->mp > 0 )
					{
						if( pj > 0 )
						{
							pricing[pj++] = ',';
							pricing[pj++] = ' ';
						}

						pj += sprintf(pricing+pj, "{x%ld{Gmp{x", pStock->mp);
					}

					if( pStock->dp > 0 )
					{
						if( pj > 0 )
						{
							pricing[pj++] = ',';
							pricing[pj++] = ' ';
						}

						pj += sprintf(pricing+pj, "{x%ld{Mdp{x", pStock->dp);
					}

					if( pStock->pneuma > 0 )
					{
						if( pj > 0 )
						{
							pricing[pj++] = ',';
							pricing[pj++] = ' ';
						}

						pj += sprintf(pricing+pj, "{x%ld{Cpn{x", pStock->pneuma);
					}

					if( pStock->rep_points > 0)
					{
						if( pj > 0 )
						{
							pricing[pj++] = ',';
							pricing[pj++] = ' ';
						}

						pj += sprintf(pricing+pj, "{x%ld{Brep{x", pStock->rep_points);
					}

					if( pStock->paragon_levels > 0)
					{
						if( pj > 0 )
						{
							pricing[pj++] = ',';
							pricing[pj++] = ' ';
						}

						pj += sprintf(pricing+pj, "{x%ld{Y*{x", pStock->paragon_levels);
					}

					pricing[pj] = '\0';
					sprintf(disc, "%3d%%", pStock->discount);
				}
				pwidth = get_colour_width(pricing) + 14;

				switch(pStock->type)
				{
				case STOCK_OBJECT:
					strcpy(typ,"{GOBJECT{x  ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 ) {

						OBJ_INDEX_DATA *obj = get_obj_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !obj ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", obj->short_descr, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}
					}
					else
						strcpy(item, "-invalid-");

					break;
				case STOCK_PET:
					strcpy(typ,"{GPET{x     ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 ) {

						MOB_INDEX_DATA *mob = get_mob_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !mob ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", mob->short_descr, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}
					}
					else
						strcpy(item, "-invalid-");
					break;
				case STOCK_MOUNT:
					strcpy(typ,"{GMOUNT{x   ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 ) {

						MOB_INDEX_DATA *mob = get_mob_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !mob ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", mob->short_descr, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}
					}
					else
						strcpy(item, "-invalid-");
					break;
				case STOCK_GUARD:
					strcpy(typ,"{GGUARD{x   ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 ) {

						MOB_INDEX_DATA *mob = get_mob_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !mob ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", mob->short_descr, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}
					}
					else
						strcpy(item, "-invalid-");
					break;

				case STOCK_CREW:
					strcpy(typ,"{GCREW{x    ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 ) {

						MOB_INDEX_DATA *mob = get_mob_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !mob || !mob->pCrew ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", mob->short_descr, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}
					}
					else
						strcpy(item, "-invalid-");
					break;

				case STOCK_SHIP:
					strcpy(typ,"{GSHIP{x    ");
					if( pStock->wnum.pArea && pStock->wnum.vnum > 0 )
					{
						SHIP_INDEX_DATA *ship_index = get_ship_index(pStock->wnum.pArea, pStock->wnum.vnum);

						if( !ship_index ) {
							strcpy(item, "-invalid-");
						}
						else
						{
							sprintf(item, "%s (%ld#%ld)", ship_index->name, pStock->wnum.pArea->uid, pStock->wnum.vnum);
						}

					}
					else
						strcpy(item, "-invalid-");
					break;
				case STOCK_CUSTOM:
					strcpy(typ,"{GCUSTOM{x  ");
					if(IS_NULLSTR(pStock->custom_keyword))
					{
						strcpy(item, "-invalid stock item-");
					}
					else
					{
						strcpy(item, pStock->custom_keyword);
					}
					break;
				}

				snprintf(buf, sizeof(buf) - 1, "  {G[{x%4d{G]{x %-9s %*s  %s  %*s %-*s %s %s %s%s\n\r", iStock, lvl, qwidth, qty, (pStock->singular?"{RY{x":"{GN{x"), hwidth, hours, pwidth, pricing, disc, rep, typ, item);
				add_buf(buffer,buf);

				if( !IS_NULLSTR(pStock->custom_descr) )
				{
					sprintf(buf, "                                                              - %s\n\r", pStock->custom_descr);
					add_buf(buffer, buf);
				}
			}
		}
	}

	if ( IS_VALID(pMob->pCrew) )
	{
		add_buf(buffer, "{CShip Crew Data:{x\n\r");
		add_buf(buffer, "{C================================{x\n\r");

		sprintf(buf, "{CMinimum Rank{c:      {WNYI{x\n\r");
		add_buf(buffer, buf);

		sprintf(buf, "{CScouting Rating{c:   {C[{x%d%%{C]{x\n\r", pMob->pCrew->scouting);
		add_buf(buffer, buf);

		sprintf(buf, "{CGunning Rating{c:    {C[{x%d%%{C]{x\n\r", pMob->pCrew->gunning);
		add_buf(buffer, buf);

		sprintf(buf, "{COarring Rating{c:    {C[{x%d%%{C]{x\n\r", pMob->pCrew->oarring);
		add_buf(buffer, buf);

		sprintf(buf, "{CMechanics Rating{c:  {C[{x%d%%{C]{x\n\r", pMob->pCrew->mechanics);
		add_buf(buffer, buf);

		sprintf(buf, "{CNavigation Rating{c: {C[{x%d%%{C]{x\n\r", pMob->pCrew->navigation);
		add_buf(buffer, buf);

		sprintf(buf, "{CLeadership Rating{c: {C[{x%d%%{C]{x\n\r", pMob->pCrew->leadership);
		add_buf(buffer, buf);

		add_buf(buffer, "\n\r");
	}

	if (pMob->pMissionary)
	{
		MISSIONARY_DATA *missionary = pMob->pMissionary;

		add_buf(buffer, "{YMissionary data:\n\r");

		sprintf(buf, "  {YScroll: %ld#%ld\n\r", missionary->scroll.auid, missionary->scroll.vnum);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->keywords))
			sprintf(buf, "  {YKeywords: (empty){x\n\r");
		else
			sprintf(buf, "  {YKeywords: %s{x\n\r", missionary->keywords);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->short_descr))
			sprintf(buf, "  {YShort Description: (empty){x\n\r");
		else
			sprintf(buf, "  {YShort Description: %s{x\n\r", missionary->short_descr);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->long_descr))
			sprintf(buf, "  {YDescription: (empty){x\n\r");
		else
			sprintf(buf, "  {YDescription: %s{x\n\r", missionary->long_descr);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->header))
			sprintf(buf, "  {YHeader: (empty){x\n\r");
		else
			sprintf(buf, "  {YHeader:\n\r%s{x\n\r", missionary->header);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->footer))
			sprintf(buf, "  {YFooter: (empty){x\n\r");
		else
			sprintf(buf, "  {YFooter:\n\r%s{x\n\r", missionary->footer);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->prefix))
			sprintf(buf, "  {YPrefix: (empty){x\n\r");
		else
			sprintf(buf, "  {YPrefix: %s{x\n\r", missionary->prefix);
		add_buf(buffer, buf);

		if(IS_NULLSTR(missionary->suffix))
			sprintf(buf, "  {YSuffix: (empty){x\n\r");
		else
			sprintf(buf, "  {YSuffix: %s{x\n\r", missionary->suffix);
		add_buf(buffer, buf);

		if( missionary->line_width > 0 )
			sprintf(buf, "  {YWidth:  %d{x\n\r", missionary->line_width);
		else
			sprintf(buf, "  {YWidth:  disabled{x\n\r");
		add_buf(buffer, buf);
		add_buf(buffer, "\n\r");
	}

	if (pMob->script_visible)
		sprintf(buf, "Visibility: %s (%s)\n\r", widevnum_string_script(pMob->script_visible, NULL), pMob->script_visible->name);
	else
		sprintf(buf, "Visibility: none\n\r");
	add_buf(buffer, buf);

    if (pMob->progs)
		olc_show_progs(buffer, pMob->progs, PRG_MPROG, "MobProg Vnum");

	olc_show_index_vars(buffer, pMob->index_vars);

	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);
	return false;
}


MEDIT(medit_next)
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *nextMob = NULL;
    long next_vnum;

    EDIT_MOB(ch, pMob);

    next_vnum = pMob->vnum;

    next_vnum++;
    while (nextMob == NULL && next_vnum > 0)
    {
	nextMob = get_mob_index(pMob->area, next_vnum);
	next_vnum++;
    }

    if (nextMob == NULL)
    {
	send_to_char("No next mob in area.\n\r", ch);
    }
    else
    {
	edit_done(ch);
	olc_set_editor(ch, ED_MOBILE, nextMob);
    }
    return false;
}

MEDIT(medit_persist)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);


	if (!str_cmp(argument,"on")) {
	    if (!str_cmp(pMob->sig, "none") && !IS_IMPLEMENTOR(ch)) {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }

		pMob->persist = true;
	    use_imp_sig(pMob, NULL);
		send_to_char("Persistance enabled.\n\r", ch);
	} else if (!str_cmp(argument,"off")) {
		pMob->persist = false;
		send_to_char("Persistance disabled.\n\r", ch);
	} else {
		send_to_char("Usage: persist on/off\n\r", ch);
		return false;
	}

	return true;
}


MEDIT(medit_boss)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (!str_cmp(argument,"on")) {
	    if (!str_cmp(pMob->sig, "none") && !IS_IMPLEMENTOR(ch)) {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }

		pMob->boss = true;
	    use_imp_sig(pMob, NULL);
		send_to_char("Boss status enabled.\n\r", ch);
	} else if (!str_cmp(argument,"off")) {
		pMob->boss= false;
		send_to_char("Boss status disabled.\n\r", ch);
	} else {
		send_to_char("Usage: boss on/off\n\r", ch);
		return false;
	}

	return true;
}

MEDIT(medit_prev)
{
    MOB_INDEX_DATA *pMob;
    MOB_INDEX_DATA *prevMob = NULL;
    long prev_vnum;

    EDIT_MOB(ch, pMob);

    prev_vnum = pMob->vnum;

    prev_vnum--;
    while (prevMob == NULL && prev_vnum > 0)
    {
	prevMob = get_mob_index(pMob->area, prev_vnum);
	prev_vnum--;
    }

    if (prevMob == NULL)
    {
	send_to_char("No previous mob in area.\n\r", ch);
    }
    else
    {
	edit_done(ch);
	olc_set_editor(ch, ED_MOBILE, prevMob);
    }
    return false;
}


MEDIT(medit_attacks)
{
	MOB_INDEX_DATA *pMob;
	int value;

	EDIT_MOB(ch, pMob);


	if (!str_prefix(argument,"scripted"))
		value = -1;
	else if ((value = atoi(argument)) < 0 || value > 10) {
		send_to_char("Invalid number.\n\r", ch);
		return false;
	}

	pMob->attacks = value;
	send_to_char("Number of attacks set.\n\r", ch);
	return true;
}


MEDIT(medit_owner)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  owner [string]\n\r", ch);
	return false;
    }

    free_string(pMob->owner);
    pMob->owner = str_dup(argument);

    send_to_char("Owner set.\n\r", ch);
    return true;
}


MEDIT(medit_create)
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea = ch->in_room->area;
    int  iHash;
    long auto_vnum = 0;
	WNUM wnum;

    if (argument[0] == '\0' || !parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
    {
	//send_to_char("Syntax:  medit create [vnum]\n\r", ch);
	for(auto_vnum = 1; auto_vnum > 0 && get_mob_index(pArea, auto_vnum); auto_vnum++);

	if (auto_vnum < 1)
	{
	    send_to_char("Sorry, this area has no more space left.\n\r",
		    ch);
	    return false;
	}

		wnum.pArea = pArea;
		wnum.vnum = auto_vnum;
    }

    if (!IS_BUILDER(ch, wnum.pArea))
    {
	send_to_char("MEdit:  Vnum in an area you cannot build in.\n\r", ch);
	return false;
    }

    if (get_mob_index(wnum.pArea, wnum.vnum))
    {
	send_to_char("MEdit:  Mobile already exists.\n\r", ch);
	return false;
    }

    pMob			= new_mob_index();
    pMob->vnum			= wnum.vnum;
    pMob->area			= wnum.pArea;

	wnum.pArea->bottom_vnum_mob = UMIN(wnum.pArea->bottom_vnum_mob, wnum.vnum);
	wnum.pArea->top_vnum_mob = UMAX(wnum.pArea->top_vnum_mob, wnum.vnum);

    pMob->act[0]			= ACT_IS_NPC;
    pMob->act[1]			= 0;
    iHash			= wnum.vnum % MAX_KEY_HASH;
    pMob->next			= wnum.pArea->mob_index_hash[iHash];
    wnum.pArea->mob_index_hash[iHash]	= pMob;
	olc_set_editor(ch, ED_MOBILE, pMob);


    // Make sure to set minimum level to 1.
    pMob->level = 1;
    set_mob_hitdice(pMob);
    set_mob_damdice(pMob);
    if (!IS_SET(pMob->act[0], ACT_MOUNT))
	set_mob_movedice(pMob);

    send_to_char("Mobile Created.\n\r", ch);
    SET_BIT(pMob->area->area_flags, AREA_CHANGED);
    free_string(pMob->creator_sig);
    pMob->creator_sig = str_dup(ch->name);
    return true;
}


MEDIT(medit_spec)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  spec [special function]\n\r", ch);
	return false;
    }


    if (!str_cmp(argument, "none"))
    {
        pMob->spec_fun = NULL;

        send_to_char("Spec removed.\n\r", ch);
        return true;
    }

    if (spec_lookup(argument))
    {
	pMob->spec_fun = spec_lookup(argument);
	send_to_char("Spec set.\n\r", ch);
	return true;
    }

    send_to_char("MEdit: No such special function.\n\r", ch);
    return false;
}


MEDIT(medit_visibility)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  visibility set <widevnum>\n\r", ch);
		send_to_char("         visibility clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		WNUM wnum;

		if (!parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
		{
			send_to_char("Please provide a widevnum.\n\r", ch);
			return false;
		}

		SCRIPT_DATA *script = get_script_index_wnum(wnum, PRG_OPROG);
		if (!script)
		{
			send_to_char("No such mobile script with that widevnum.\n\r", ch);
			return false;
		}

		pMob->script_visible = script;
		send_to_char("Visibility Script changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		pMob->script_visible = NULL;
		send_to_char("Visibility Script cleared.\n\r", ch);
		return true;
	}

	medit_visibility(ch, "");
	return false;
}


MEDIT(medit_damtype)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  damtype [damage message]\n\r", ch);
	send_to_char("For a list of damtypes, type '? weapon'.\n\r", ch);
	return false;
    }

    pMob->dam_type = attack_lookup(argument);
    send_to_char("Damage type set.\n\r", ch);
    return true;
}


MEDIT(medit_align)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  alignment [number]\n\r", ch);
	return false;
    }

    pMob->alignment = atoi(argument);

    send_to_char("Alignment set.\n\r", ch);
    return true;
}


MEDIT(medit_level)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch, pMob);
    char buf[MSL];

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  level [number]\n\r", ch);
	return false;
    }

    if (atoi(argument) == 0) {
	send_to_char("Sorry, mob levels start at 1.\n\r", ch);
	return false;
    }

    if (atoi(argument) > MAX_MOB_SKILL_LEVEL) {
	sprintf(buf, "Sorry, max mob level is %d.\n\r", MAX_MOB_SKILL_LEVEL);
	return false;
    }

    pMob->level = atoi(argument);

    send_to_char("Level set.\n\r", ch);
    set_mob_hitdice(pMob);
    send_to_char("Hit Dice set.\n\r", ch);
    set_mob_damdice(pMob);
    send_to_char("Damage dice set.\n\r", ch);

    if (!IS_SET(pMob->act[0], ACT_MOUNT)) {
	set_mob_movedice(pMob);
	send_to_char("Movement dice set.\n\r", ch);
    }

    if (IS_SET(pMob->off_flags, OFF_MAGIC))
    {
	set_mob_manadice(pMob);
	send_to_char("Mana dice set.\n\r", ch);
    }

    return true;

}


MEDIT(medit_desc)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	string_append(ch, &pMob->description);
	return true;
    }

    send_to_char("Syntax:  desc    - line edit\n\r", ch);
    return false;
}

MEDIT(medit_comments)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	string_append(ch, &pMob->comments);
	return true;
    }

    send_to_char("Syntax:  comments    - line edit\n\r", ch);
    return false;
}


MEDIT(medit_long)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  long [string]\n\r", ch);
	return false;
    }

    free_string(pMob->long_descr);
	if (str_suffix("{x", argument))
	    strcat(argument, "{x");
    pMob->long_descr = str_dup(argument);
    pMob->long_descr[0] = UPPER(pMob->long_descr[0] );

    send_to_char("Long description set.\n\r", ch);
    return true;
}


MEDIT(medit_short)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  short [string]\n\r", ch);
	return false;
    }

    free_string(pMob->short_descr);
    pMob->short_descr = str_dup(argument);

    send_to_char("Short description set.\n\r", ch);
    if (IS_SET(ch->act[0], PLR_AUTOSETNAME))
    {
	free_string(pMob->player_name);
	pMob->player_name = short_to_name(pMob->short_descr);
	send_to_char("Name keywords set.\n\r", ch);
    }
    return true;
}


MEDIT(medit_name)
{
    MOB_INDEX_DATA *pMob;
    char name[MSL];
    FILE *fp;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  name [string]\n\r", ch);
	return false;
    }

    sprintf(name, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
    if ((fp = fopen(name, "r")) == NULL)
    {
	free_string(pMob->player_name);
	pMob->player_name = str_dup(argument);

	send_to_char("Name set.\n\r", ch);
    }
    else
    {
	send_to_char("Sorry, there is a player with that name, so you can't set it on your mob.\n\r", ch);
	fclose(fp);
	return false;
    }

    return true;
}


MEDIT(medit_sign)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (ch->tot_level < 154)
    {
	send_to_char("Sorry, only immortals of level 154 and above can do that.\n\r", ch);
	return false;
    }

    free_string(pMob->sig);
    pMob->sig = str_dup(ch->name);

    send_to_char("Mobile signed.\n\r", ch);

    return true;
}

MEDIT(medit_skeywds)
{
    MOB_INDEX_DATA *pMob;
    char name[MSL];
    FILE *fp;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  skwds [string]\n\r", ch);
	return false;
    }

    sprintf(name, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));
    if ((fp = fopen(name, "r")) == NULL)
    {
	free_string(pMob->skeywds);
	pMob->skeywds = str_dup(argument);

	send_to_char("Script keywords set.\n\r", ch);
    }
    else
    {
	send_to_char("Sorry, there is a player with that name, so you can't set it on your mob.\n\r", ch);
	fclose(fp);
	return false;
    }

    return true;
}


MEDIT(medit_varset)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

	return olc_varset(&pMob->index_vars, ch, argument, false);
}

MEDIT(medit_varclear)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

	return olc_varclear(&pMob->index_vars, ch, argument, false);
}

MEDIT(medit_corpsetype)
{
    MOB_INDEX_DATA *pMob;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch, pMob);


		if (!str_cmp(argument, "none")) {
			pMob->corpse_type = NULL;

			 send_to_char("Corpse type cleared.\n\r", ch);
			return true;
		} else {
			CORPSE_TYPE *corpse = get_corpse_type(argument);
			if (!corpse)
			{
				send_to_char("No corpse data by that name.\n\r", ch);
				return false;
			}
			pMob->corpse_type = corpse;

			 send_to_char("Corpse type set.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax: corpsetype [type]\n\r"
		  "Use 'corpselist' for a list of corpse types.\n\r", ch);
    return false;
}

MEDIT(medit_corpse)
{
    MOB_INDEX_DATA *pMob;
    WNUM wnum;

    if (argument[0] != '\0' && parse_widevnum(argument, ch->in_room->area, &wnum))
    {
		EDIT_MOB(ch, pMob);

		if (!str_prefix(argument, "clear"))
		{
			send_to_char("Corpse object cleared.\n\r",ch);
			pMob->corpse.auid = 0;
			pMob->corpse.vnum = 0;
			return true;
		}
		else if (parse_widevnum(argument, ch->in_room->area, &wnum) && wnum.pArea && wnum.vnum > 0)
		{
			if(!get_obj_index(wnum.pArea, wnum.vnum))
			{
				send_to_char("Object does not exist.\n\r",ch);
				return false;
			}
			else
			{
				send_to_char("Corpse object set.\n\r",ch);
				pMob->corpse.auid = wnum.pArea->uid;
				pMob->corpse.vnum = wnum.vnum;
				return true;
			}
		}
	}

    send_to_char("Syntax: corpse <widevnum>\n\r", ch);
    send_to_char("        corpse clear\n\r", ch);
    return false;
}

MEDIT(medit_zombie)
{
    MOB_INDEX_DATA *pMob;
    WNUM wnum;

    if (argument[0] != '\0' && parse_widevnum(argument, ch->in_room->area, &wnum))
    {
		EDIT_MOB(ch, pMob);

		if (!str_prefix(argument, "clear"))
		{
			send_to_char("Zombie corpse object cleared.\n\r",ch);
			pMob->zombie.auid = 0;
			pMob->zombie.vnum = 0;
			return true;
		}
		else if (parse_widevnum(argument, ch->in_room->area, &wnum) && wnum.pArea && wnum.vnum > 0)
		{
			if(!get_obj_index(wnum.pArea, wnum.vnum))
			{
				send_to_char("Zombie corpse object does not exist.\n\r",ch);
				return false;
			}
			else
			{
				send_to_char("Zombie corpse object set.\n\r",ch);
				pMob->zombie.auid = wnum.pArea->uid;
				pMob->zombie.vnum = wnum.vnum;
				return true;
			}
		}
	}

    send_to_char("Syntax: zombie <widevnum>\n\r", ch);
    send_to_char("        zombie clear\n\r", ch);
    return false;
}

MEDIT(medit_shop)
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *flag_start;

    argument = one_argument(argument, command);
    flag_start = argument;
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    EDIT_MOB(ch, pMob);

    if (command[0] == '\0')
    {
		send_to_char("Syntax:  shop assign\n\r", ch);
		send_to_char("         shop remove\n\r\n\r", ch);

		send_to_char("         shop discount [0-100] [reset]\n\r", ch);
		send_to_char("         shop flags [flags]\n\r", ch);
		send_to_char("         shop hours [#xopening] [#xclosing]\n\r", ch);
		send_to_char("         shop profit [#xbuying%] [#xselling%]\n\r", ch);
		send_to_char("         shop reputation set <reputation>[ <minimum rank#>]\n\r", ch);
		send_to_char("         shop reputation clear\n\r", ch);
		send_to_char("         shop restock [minutes]\n\r", ch);
		send_to_char("         shop shipyard clear\n\r", ch);
		send_to_char("         shop shipyard <wuid> <x1> <y1> <x2> <y2> <description>\n\r", ch);
		send_to_char("         shop stock add [type] [value]\n\r", ch);
		send_to_char("         shop stock [#] discount [0-100]\n\r", ch);
		send_to_char("         shop stock [#] description [description]\n\r", ch);
		send_to_char("         shop stock [#] duration [#hours|none]\n\r", ch);
		send_to_char("         shop stock [#] level [level]\n\r", ch);
		send_to_char("         shop stock [#] price <silver|qp|dp|pneuma|reputation|paragon|custom>[ <check price script (for custom only)>] <value>\n\r", ch);
		send_to_char("         shop stock [#] quantity unlimited\n\r", ch);
		send_to_char("         shop stock [#] quantity [total] [reset rate]\n\r", ch);
		send_to_char("         shop stock [#] singular\n\r", ch);
		send_to_char("         shop stock [#] remove\n\r", ch);
		send_to_char("         shop stock [#] reputation set <reputation> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
		send_to_char("         shop stock [#] reputation clear\n\r", ch);
		send_to_char("         shop type [#x0-4] [item type]\n\r", ch);
		return false;
    }


    if (!str_prefix(command, "hours"))
    {
		if (arg1[0] == '\0' || !is_number(arg1) ||
			argument[0] == '\0' || !is_number(arg2))
		{
			send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(arg2);

		send_to_char("Shop hours set.\n\r", ch);
		return true;
    }

	if (!str_prefix(command, "reputation"))
	{
		if (arg1[0] == '\0')
		{
			send_to_char("Syntax:  shop reputation set <reputation>[ <minimum rank#>]\n\r", ch);
			send_to_char("         shop reputation clear\n\r", ch);
			return false;
		}

		if (!str_prefix(arg1, "set"))
		{
			WNUM wnum;

			if (!parse_widevnum(arg2, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
			{
				send_to_char("Please provide a widevnum.\n\r", ch);
				return false;
			}

			REPUTATION_INDEX_DATA *repIndex = get_reputation_index_wnum(wnum);
			if (!IS_VALID(repIndex))
			{
				send_to_char("No such reputation by that widevnum.\n\r", ch);
				return false;
			}

			int min_rank = 0;
			if (argument[0] != '\0')
			{
				if (!is_number(argument) || (min_rank = atoi(argument)) < 1 || min_rank > list_size(repIndex->ranks))
				{
					send_to_char(formatf("Please specify a rank number from 1 to %d.\n\r", list_size(repIndex->ranks)), ch);
					return false;
				}
			}

			pMob->pShop->reputation = repIndex;
			pMob->pShop->min_reputation_rank = min_rank;
			send_to_char("Interaction reputation set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg1, "clear"))
		{
			pMob->pShop->reputation = NULL;
			pMob->pShop->min_reputation_rank = 0;
			send_to_char("Interaction reputation cleared.\n\r", ch);
			return true;
		}

		medit_shop(ch, "reputation");
		return false;
	}

    if (!str_prefix(command, "restock"))
    {
		if (arg1[0] == '\0' || !is_number(arg1))
		{
			send_to_char("Syntax:  shop restock [minutes]\n\r", ch);
			send_to_char("   Specify at least 10 minutes, or 0 to disable restocking.\n\r", ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		int interval = atoi(arg1);

		if( interval <= 0 )
		{
			send_to_char("Restocking disabled.\n\r", ch);
			pMob->pShop->restock_interval = 0;
			return true;
		}
		else if( interval < 10 )
		{
			send_to_char("Interval too short.\n\rPlease try at least 10 minutes, or 0 to disable restocking.\n\r", ch);
			return false;
		}
		else
		{
			send_to_char("Restocking changed.\n\r", ch);
			pMob->pShop->restock_interval = interval;
			return true;
		}
    }

    if (!str_prefix(command, "shipyard"))
    {
		char arg3[MIL];
		char arg4[MIL];
		char arg5[MIL];
		argument = one_argument(argument, arg3);
		argument = one_argument(argument, arg4);
		argument = one_argument(argument, arg5);

		if( !str_cmp(arg1, "clear") )
		{
			pMob->pShop->shipyard = 0;
			pMob->pShop->shipyard_region[0][0] = 0;
			pMob->pShop->shipyard_region[0][1] = 0;
			pMob->pShop->shipyard_region[1][0] = 0;
			pMob->pShop->shipyard_region[1][1] = 0;

			free_string(pMob->pShop->shipyard_description);
			pMob->pShop->shipyard_description = &str_empty[0];

			send_to_char("Shipyard cleared.\n\r", ch);
			return true;
		}
		if( !is_number(arg1) || !is_number(arg2) || !is_number(arg3) || !is_number(arg4) || !is_number(arg5) || IS_NULLSTR(argument) )
		{
			send_to_char("Syntax:  shop shipyard <wuid> <x1> <y1> <x2> <y2> <description>\n\r", ch);
			send_to_char("         shop shipyard clear\n\r", ch);
			return false;
		}
		long wuid = atol(arg1);
		int x1 = atoi(arg2);
		int y1 = atoi(arg3);
		int x2 = atoi(arg4);
		int y2 = atoi(arg5);

		if( !is_shipyard_valid(wuid, x1, y1, x2, y2) )
		{
			send_to_char("Shipyard not valid.  Please verify wilderness and coordinates.\n\r", ch);
			send_to_char("Make sure Shipyard has safe harbor water tiles next to non-water tiles.\n\r", ch);
			return false;
		}

		pMob->pShop->shipyard = wuid;
		pMob->pShop->shipyard_region[0][0] = x1;
		pMob->pShop->shipyard_region[0][1] = y1;
		pMob->pShop->shipyard_region[1][0] = x2;
		pMob->pShop->shipyard_region[1][1] = y2;

		smash_tilde(argument);
		free_string(pMob->pShop->shipyard_description);
		pMob->pShop->shipyard_description = str_dup(argument);


		send_to_char("Shipyard set.\n\r", ch);
		return true;
	}

    if (!str_prefix(command, "discount"))
    {
		if (arg1[0] == '\0' || !is_number(arg1))
		{
			send_to_char("Syntax:  shop discount [0-100] [reset]\n\r", ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		int disc = atoi(arg1);

		if( disc < 0 || disc > 100 )
		{
			send_to_char("Discount must be a percentage (0-100).\n\r", ch);
			return false;
		}

		pMob->pShop->discount = disc;

		if( !str_cmp(arg2, "reset") && pMob->pShop->stock != NULL )
		{
			bool updated = false;
			for(SHOP_STOCK_DATA *stock = pMob->pShop->stock; stock; stock = stock->next)
			{
				if( IS_NULLSTR(stock->custom_keyword) )
				{
					stock->discount = pMob->pShop->discount;
					updated = true;
				}
			}

			if( updated )
				send_to_char("Discount changed, and stock updated.\n\r", ch);
			else
				send_to_char("Discount changed.\n\r", ch);
		}
		else
			send_to_char("Discount changed.\n\r", ch);
		return true;
    }


    if (!str_prefix(command, "profit"))
    {
		if (arg1[0] == '\0' || !is_number(arg1) ||
			argument[0] == '\0' || !is_number(arg2))
		{
			send_to_char("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(arg2);

		send_to_char("Shop profit set.\n\r", ch);
		return true;
    }


    if (!str_prefix(command, "type"))
    {
		char buf[MAX_INPUT_LENGTH];
		int value;

		if (arg1[0] == '\0' || !is_number(arg1) || arg2[0] == '\0')
		{
		    send_to_char("Syntax:  shop type [#x0-4] [item type]\n\r", ch);
		    return false;
		}

		if (atoi(arg1) >= MAX_TRADE)
		{
			sprintf(buf, "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
			send_to_char(buf, ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		if ((value = flag_value(type_flags, arg2)) == NO_FLAG)
		{
			send_to_char("MEdit:  That type of item is not known.\n\r", ch);
			return false;
		}

		pMob->pShop->buy_type[atoi(arg1)] = value;

		send_to_char("Shop type set.\n\r", ch);
		return true;
    }

    /* shop assign && shop delete by Phoenix */

    if (!str_prefix(command, "assign"))
    {
    	if (pMob->pShop)
    	{
        	send_to_char("Mob already has a shop assigned to it.\n\r", ch);
        	return false;
		}

		pMob->pShop		= new_shop();
		if (!shop_first)
				shop_first	= pMob->pShop;
		if (shop_last)
			shop_last->next	= pMob->pShop;
		shop_last		= pMob->pShop;

		send_to_char("New shop assigned to mobile.\n\r", ch);
		return true;
    }

    if (!str_prefix(command, "remove"))
    {
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;

		if (pShop == shop_first)
		{
			if (!pShop->next)
			{
				shop_first = NULL;
				shop_last = NULL;
			}
			else
				shop_first = pShop->next;
		}
		else
		{
			SHOP_DATA *ipShop;

			for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
			{
				if (ipShop->next == pShop)
				{
					if (!pShop->next)
					{
						shop_last = ipShop;
						shop_last->next = NULL;
					}
					else
						ipShop->next = pShop->next;
				}
			}
		}

		free_shop(pShop);

		send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
		return true;
    }

    if(!str_prefix(command, "flags"))
    {
		int value;
		if (flag_start[0] != '\0')
		{

			if ((value = flag_value(shop_flags, flag_start)) != NO_FLAG)
			{
				pMob->pShop->flags ^= value;

				send_to_char("Shop flags toggled.\n\r", ch);
				return true;
			}
		}

		send_to_char(	"Syntax: shop flags [flag]\n\r"
						"Type '? shop' for a list of flags.\n\r", ch);

		return false;
	}

	if(!str_prefix(command, "stock"))
	{
		SHOP_STOCK_DATA *stock;

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Please create a shop first (shop assign).\n\r", ch);
			return false;
		}

		if(arg1[0] == '\0')
		{
			send_to_char("Syntax:  shop stock add object [vnum]\n\r", ch);
			send_to_char("         shop stock add pet [vnum]\n\r", ch);
			send_to_char("         shop stock add mount [vnum]\n\r", ch);
			send_to_char("         shop stock add guard [vnum]\n\r", ch);
			send_to_char("         shop stock add crew [vnum]\n\r", ch);
			send_to_char("         shop stock add ship [vnum]\n\r", ch);
			send_to_char("         shop stock add custom [keyword]\n\r", ch);
			send_to_char("         shop stock [#] discount [0-100]\n\r", ch);
			send_to_char("         shop stock [#] description [description]\n\r", ch);
			send_to_char("         shop stock [#] duration [#hours|none]\n\r", ch);
			send_to_char("         shop stock [#] level [level]\n\r", ch);
			send_to_char("         shop stock [#] price <silver|qp|dp|pneuma|reputation|paragon|custom>[ <check price script (for custom only)>] [value]\n\r", ch);
			send_to_char("         shop stock [#] quantity unlimited\n\r", ch);
			send_to_char("         shop stock [#] quantity [total] [reset rate]\n\r", ch);
			send_to_char("         shop stock [#] singular\n\r", ch);
			send_to_char("         shop stock [#] remove\n\r", ch);
			send_to_char("         shop stock [#] reputation set <reputation> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
			send_to_char("         shop stock [#] reputation clear\n\r", ch);
			return false;
		}

		if(!str_prefix(arg1, "add"))
		{
			WNUM wnum;
			if(arg2[0] == '\0' || argument[0] == '\0')
			{
				send_to_char("Syntax:  shop stock add object [wnum]\n\r", ch);
				send_to_char("         shop stock add pet [wnum]\n\r", ch);
				send_to_char("         shop stock add mount [wnum]\n\r", ch);
				send_to_char("         shop stock add guard [wnum]\n\r", ch);
				send_to_char("         shop stock add crew [wnum]\n\r", ch);
				send_to_char("         shop stock add ship [wnum]\n\r", ch);
				send_to_char("         shop stock add custom [keyword]\n\r", ch);
				return false;
			}

			if(!str_prefix(arg2, "object"))
			{
				if(parse_widevnum(argument, ch->in_room->area, &wnum))
				{
					OBJ_INDEX_DATA *item = get_obj_index(wnum.pArea, wnum.vnum);

					if(!item)
					{
						send_to_char("Object does not exist.\n\r", ch);
						return false;
					}

					if(IS_MONEY(item))
					{
						send_to_char("You cannot sell money.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_OBJECT;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = item->cost;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (OBJECT) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add object [wnum]\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg2, "pet"))
			{
				if(parse_widevnum(argument, ch->in_room->area, &wnum))
				{
					MOB_INDEX_DATA *mob = get_mob_index(wnum.pArea, wnum.vnum);

					if(!mob)
					{
						send_to_char("Mobile does not exist.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_PET;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = 10 * mob->level * mob->level;
					stock->level = mob->level;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (PET) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add pet [wnum]\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg2, "mount"))
			{
				if(parse_widevnum(argument, ch->in_room->area, &wnum))
				{
					MOB_INDEX_DATA *mob = get_mob_index(wnum.pArea, wnum.vnum);

					if(!mob)
					{
						send_to_char("Mobile does not exist.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_MOUNT;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = 25 * mob->level * mob->level;
					stock->level = mob->level;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (MOUNT) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add mount [wnum]\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg2, "guard"))
			{
				if(parse_widevnum(argument, ch->in_room->area, &wnum))
				{
					MOB_INDEX_DATA *mob = get_mob_index(wnum.pArea, wnum.vnum);

					if(!mob)
					{
						send_to_char("Mobile does not exist.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_GUARD;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = 50 * mob->level * mob->level;
					stock->level = mob->level;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (GUARD) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add guard [wnum]\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg2, "crew"))
			{
				if(parse_widevnum(argument, ch->in_room->area, &wnum))
				{
					MOB_INDEX_DATA *mob = get_mob_index(wnum.pArea, wnum.vnum);

					if(!mob)
					{
						send_to_char("Mobile does not exist.\n\r", ch);
						return false;
					}

					if(!mob->pCrew)
					{
						send_to_char("Mobile has no Crew definition.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_CREW;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = 50 * mob->level * mob->level;
					stock->level = mob->level;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (CREW) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add crew [wnum]\n\r", ch);
				return false;
			}
			else if(!str_prefix(arg2, "ship"))
			{
				if( !is_shipyard_valid(pMob->pShop->shipyard,
					pMob->pShop->shipyard_region[0][0],
					pMob->pShop->shipyard_region[0][1],
					pMob->pShop->shipyard_region[1][0],
					pMob->pShop->shipyard_region[1][1]) )
				{
					send_to_char("Shopkeeper needs to have a valid shipyard defined first before you can add a ship.\n\r", ch);
					return false;
				}

				if( parse_widevnum(argument, ch->in_room->area, &wnum) )
				{
					SHIP_INDEX_DATA *ship;

					if( !(ship = get_ship_index(wnum.pArea, wnum.vnum)) )
					{
						send_to_char("That ship does not exist.\n\r", ch);
						return false;
					}

					if( !IS_VALID(ship->blueprint) || !get_obj_index(ship->area, ship->ship_object) )
					{
						send_to_char("Ship is incomplete.  Cannot be sold yet.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_SHIP;
					stock->shop = pMob->pShop;
					stock->wnum = wnum;
					stock->silver = 100000;	// Default 1000gold
					stock->level = 1;
					stock->discount = pMob->pShop->discount;

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (SHIP) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add ship [wnum]\n\r", ch);
				return false;

			}
			else if(!str_prefix(arg2, "custom"))
			{
				argument = one_argument(argument, arg2);
				if(!IS_NULLSTR(argument))
				{
					for(stock = pMob->pShop->stock; stock; stock = stock->next)
					{
						if( (stock->type == STOCK_CUSTOM) &&
							!str_cmp(argument, stock->custom_keyword) )
						{
							break;
						}
					}

					if( stock != NULL )
					{
						send_to_char("Keyword already used.\n\r", ch);
						return false;
					}

					stock = new_shop_stock();

					if(!stock)
					{
						send_to_char("{RERROR{W: Unable to create stock item.{x\n\r", ch);
						return false;
					}

					stock->type = STOCK_CUSTOM;
					stock->shop = pMob->pShop;
					stock->custom_keyword = str_dup(argument);
					stock->discount = 0;		// They do not handle discounts.
												// If you wish to do discounts, that has to be scripted.

					stock->next = pMob->pShop->stock;
					pMob->pShop->stock = stock;

					send_to_char("Stock item (CUSTOM) added.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock add custom [keyword]\n\r", ch);
				return false;
			}

			send_to_char("Syntax:  shop stock add object [wnum]\n\r", ch);
			send_to_char("         shop stock add pet [wnum]\n\r", ch);
			send_to_char("         shop stock add mount [wnum]\n\r", ch);
			send_to_char("         shop stock add guard [wnum]\n\r", ch);
			send_to_char("         shop stock add custom [keyword]\n\r", ch);
			return false;
		}

		if(is_number(arg1))
		{
			int idx = atoi(arg1);
			stock = get_shop_stock_bypos(pMob->pShop, idx);

			if(!stock)
			{
				send_to_char("Invalid stock number.\n\r", ch);
				return false;
			}

			if(!str_prefix(arg2, "duration"))
			{
				int duration;
				if (!str_prefix(argument, "none"))
					duration = 0;
				else if (!is_number(argument) || (duration = atoi(argument)) < 1)
				{
					send_to_char("Please provide a positive number or none.\n\r", ch);
					return false;
				}

				stock->duration = duration;
				send_to_char("Stock duration changed.\n\r", ch);
				return true;
			}

			if(!str_prefix(arg2, "price"))
			{
				char arg3[MIL];

				argument = one_argument(argument, arg3);

				if(!str_prefix(arg3, "silver"))
				{
					if(!is_number(argument))
					{
						send_to_char("Silver price must be a number.\n\r", ch);
						return false;
					}

					int silver = atoi(argument);

					stock->silver = UMAX(silver, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock silver price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "mp"))
				{
					if(!is_number(argument))
					{
						send_to_char("Mission point price must be a number.\n\r", ch);
						return false;
					}

					int mp = atoi(argument);

					stock->mp = UMAX(mp, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock mission point price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "dp"))
				{
					if(!is_number(argument))
					{
						send_to_char("Deity point price must be a number.\n\r", ch);
						return false;
					}

					int dp = atoi(argument);

					stock->dp = UMAX(dp, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock deity point price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "pneuma"))
				{
					if(!is_number(argument))
					{
						send_to_char("Pneuma price must be a number.\n\r", ch);
						return false;
					}

					int pneuma = atoi(argument);

					stock->pneuma = UMAX(pneuma, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock pneuma price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "reputation"))
				{
					if(!is_number(argument))
					{
						send_to_char("Reputation price must be a number.\n\r", ch);
						return false;
					}

					int points = atoi(argument);

					if(points > 0 && !IS_VALID(stock->reputation))
					{
						send_to_char("Please assign a reputation to the stock item, first.\n\r", ch);
						return false;
					}

					stock->rep_points = UMAX(points, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock reputation point price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "paragon"))
				{
					if(!is_number(argument))
					{
						send_to_char("Paragon price must be a number.\n\r", ch);
						return false;
					}

					int levels = atoi(argument);

					if(levels > 0 && !IS_VALID(stock->reputation))
					{
						send_to_char("Please assign a reputation to the stock item, first.\n\r", ch);
						return false;
					}

					stock->paragon_levels = UMAX(levels, 0);
					if( !IS_NULLSTR(stock->custom_price) )
					{
						stock->discount = pMob->pShop->discount;
						free_string(stock->custom_price);
						stock->custom_price = &str_empty[0];
						stock->check_price = NULL;
					}
					send_to_char("Stock paragon level price changed.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg3, "custom"))
				{
					WNUM wnum;
					argument = one_argument(argument, arg3);
					if(argument[0] == '\0' || !parse_widevnum(arg3, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
					{
						send_to_char("Please specify a custom price string and check price script.\n\r", ch);
						send_to_char("Syntax:  shop stock [#] price <check price script> custom <value>\n\r\n\r", ch);
						send_to_char("If you wish to clear the custom pricing, select a different pricing type.\n\r", ch);
						return false;
					}

					SCRIPT_DATA *script = get_script_index_wnum(wnum, PRG_MPROG);
					if (!script)
					{
						send_to_char("No such mobprog by that widevnum.\n\r", ch);
						return false;
					}

					stock->silver = 0;
					stock->mp = 0;
					stock->dp = 0;
					stock->pneuma = 0;
					stock->rep_points = 0;
					stock->paragon_levels = 0;
					stock->discount = 0;
					stock->check_price = script;
					free_string(stock->custom_price);
					stock->custom_price = str_dup(argument);
					send_to_char("Stock custom price changed.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock [#] price <silver|qp|dp|pneuma|reputation|paragon|custom>[ <check price script (for custom only)>] [value]\n\r", ch);
				return false;
			}

			if(!str_prefix(arg2, "discount"))
			{
				if( !IS_NULLSTR(stock->custom_price) )
				{
					send_to_char("Stock items with custom pricing do not receive discounts.\n\r", ch);
					send_to_char("Those need to be handled in the CUSTOM_PRICE trigger.\n\r", ch);
					return false;
				}

				if(!is_number(argument))
				{
					send_to_char("Syntax:  shop stock [#] discount [0-100]\n\r", ch);
					return false;
				}

				int disc = atoi(argument);

				if(disc < 0 || disc > 100)
				{
					send_to_char("Discount must be a percentage (0-100).\n\r", ch);
					return false;
				}

				stock->discount = disc;
				send_to_char("Stock discount changed.\n\r", ch);
				return true;
			}

			if(!str_prefix(arg2, "level"))
			{
				if(!is_number(argument))
				{
					send_to_char("Syntax:  shop stock [#] level [level]\n\r", ch);
					return false;
				}

				int lvl = atoi(argument);

				if(lvl < 1)
				{
					stock->level = 0;
					send_to_char("Stock level set to automatic.\n\r", ch);
					return true;
				}

				stock->level = lvl;
				send_to_char("Stock level changed.\n\r", ch);
				return true;
			}

			if(!str_prefix(arg2, "singular"))
			{
				stock->singular = !stock->singular;
				if(stock->singular)
					send_to_char("Stock is now singular.\n\r", ch);
				else
					send_to_char("Stock is no longer singular.\n\r", ch);
				return true;
			}


			if(!str_prefix(arg2, "quantity"))
			{
				if(!str_prefix(argument, "unlimited"))
				{
					stock->quantity = 0;
					stock->restock_rate = 0;
					send_to_char("Stock quantity settings changed.\n\r", ch);
					return true;
				}

				char arg3[MIL];
				argument = one_argument(argument, arg3);
				if(!is_number(arg3) || !is_number(argument))
				{
					send_to_char("Syntax:  shop stock [#] quantity [total] [reset rate]\n\r", ch);
					return false;
				}

				int total = atoi(arg3);
				int rate = atoi(argument);

				if(total < 1)
				{
					send_to_char("Please specify a positive number for limited quantity.\n\r", ch);
					return false;
				}

				stock->quantity = total;
				stock->restock_rate = UMAX(rate, 0);		// A rate of zero means it never restock
				send_to_char("Stock quantity settings changed.\n\r", ch);
				return true;
			}

			if(!str_prefix(arg2, "description"))
			{
				free_string(stock->custom_descr);
				stock->custom_descr = str_dup(argument);

				send_to_char("Stock description changed.\n\r", ch);
				return true;
			}

			if(!str_prefix(arg2, "remove"))
			{
				if( idx < 1 )
				{
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				SHOP_STOCK_DATA *prev = NULL;
				for(stock = pMob->pShop->stock;stock;prev = stock, stock = stock->next)
				{
					if(!--idx)
						break;
				}

				if( !stock )
				{
					send_to_char("Invalid stock number.\n\r", ch);
					return false;
				}

				if( prev != NULL )
				{
					prev->next = stock->next;
				}
				else
				{
					pMob->pShop->stock = stock->next;
				}

				free_shop_stock(stock);
				send_to_char("Stock item removed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg2, "reputation"))
			{
				char arg3[MIL];

				argument = one_argument(argument, arg3);
				if (!str_prefix(arg3, "set"))
				{
					WNUM wnum;

					argument = one_argument(argument, arg3);
					if (!parse_widevnum(arg3, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
					{
						send_to_char("Please provide a valid widevnum,\n\r", ch);
						return false;
					}

					REPUTATION_INDEX_DATA *repIndex = get_reputation_index_wnum(wnum);
					if (!IS_VALID(repIndex))
					{
						send_to_char("No such reputation with that widevnum.\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg3);
					int min_rank;
					if (!str_prefix(arg3, "none"))
						min_rank = 0;
					else
					{
						if (!is_number(arg3) || (min_rank = atoi(arg3)) < 1 || min_rank > list_size(repIndex->ranks))
						{
							send_to_char(formatf("Please provide a rank from 1 to %d.\n\r", list_size(repIndex->ranks)), ch);
							return false;
						}
					}

					argument = one_argument(argument, arg3);
					int max_rank;
					if (!str_prefix(arg3, "none"))
						max_rank = 0;
					else
					{
						if (!is_number(arg3) || (max_rank = atoi(arg3)) < 1 || max_rank > list_size(repIndex->ranks))
						{
							send_to_char(formatf("Please provide a rank from 1 to %d.\n\r", list_size(repIndex->ranks)), ch);
							return false;
						}
					}

					if (min_rank && max_rank && min_rank > max_rank)
					{
						send_to_char("Minimum rank must not be higher than the maximum rank.\n\r", ch);
						return false;
					}


					argument = one_argument(argument, arg3);
					int min_show_rank;
					if (!str_prefix(arg3, "none"))
						min_show_rank = 0;
					else
					{
						if (!is_number(arg3) || (min_show_rank = atoi(arg3)) < 1 || min_show_rank > list_size(repIndex->ranks))
						{
							send_to_char(formatf("Please provide a rank from 1 to %d.\n\r", list_size(repIndex->ranks)), ch);
							return false;
						}
					}

					argument = one_argument(argument, arg3);
					int max_show_rank;
					if (!str_prefix(arg3, "none"))
						max_show_rank = 0;
					else
					{
						if (!is_number(arg3) || (max_show_rank = atoi(arg3)) < 1 || max_show_rank > list_size(repIndex->ranks))
						{
							send_to_char(formatf("Please provide a rank from 1 to %d.\n\r", list_size(repIndex->ranks)), ch);
							return false;
						}
					}

					if (min_show_rank && max_show_rank && min_show_rank > max_show_rank)
					{
						send_to_char("Minimum show rank must not be higher than the maximum show rank.\n\r", ch);
						return false;
					}


					stock->reputation = repIndex;
					stock->min_reputation_rank = min_rank;
					stock->max_reputation_rank = max_rank;
					stock->min_show_rank = min_show_rank;
					stock->max_show_rank = max_show_rank;

					send_to_char("Stock reputation set.\n\r", ch);
					return true;
				}

				if (!str_prefix(arg3, "clear"))
				{
					stock->reputation = NULL;
					stock->min_reputation_rank = 0;
					stock->max_reputation_rank = 0;
					stock->min_show_rank = 0;
					stock->max_show_rank = 0;

					send_to_char("Stock reputation cleared.\n\r", ch);
					return true;
				}

				send_to_char("Syntax:  shop stock [#] reputation set <reputation> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
				send_to_char("         shop stock [#] reputation clear\n\r", ch);
				return false;
			}

			send_to_char("Syntax:  shop stock [#] description [description]\n\r", ch);
			send_to_char("         shop stock [#] discount [0-100]\n\r", ch);
			send_to_char("         shop stock [#] level [level]\n\r", ch);
			send_to_char("         shop stock [#] price <silver|qp|dp|pneuma|custom|reputation|paragon> <value>[ <check_price script (only on custom)>]\n\r", ch);
			send_to_char("         shop stock [#] quantity unlimited\n\r", ch);
			send_to_char("         shop stock [#] quantity [total] [reset rate]\n\r", ch);
			send_to_char("         shop stock [#] singular\n\r", ch);
			send_to_char("         shop stock [#] remove\n\r", ch);
			send_to_char("         shop stock [#] reputation set <reputation> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
			send_to_char("         shop stock [#] reputation clear\n\r", ch);
			return false;
		}

		medit_shop(ch, "stock");
		return false;
	}

    medit_shop(ch, "");
    return false;
}


MEDIT(medit_sex)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(sex_flags, argument)) != NO_FLAG)
	{
	    pMob->sex = value;

	    send_to_char("Sex set.\n\r", ch);
	    return true;
	}
	else
	if (!str_cmp(argument, "neutral")) // hack
	{
	    pMob->sex = SEX_NEUTRAL;
	    send_to_char("Sex set.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: sex [sex]\n\r"
		  "Type '? sex' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_act)
{
    MOB_INDEX_DATA *pMob;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch, pMob);

		long bits[2];
		if (bitvector_lookup(argument, 2, bits, act_flags, act2_flags))
		{
			TOGGLE_BIT(pMob->act[0], bits[0]);
			TOGGLE_BIT(pMob->act[1], bits[1]);
			SET_BIT(pMob->act[0], ACT_IS_NPC);	// Force on, all the time
			REMOVE_BIT(pMob->act[0], ACT_PRACTICE);

			send_to_char("Act flag toggled.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax: act [flag]\n\r"
		  "Type '? act' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_affect)
{
    MOB_INDEX_DATA *pMob;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch, pMob);
		long bits[2];

		if (bitvector_lookup(argument, 2, bits, affect_flags, affect2_flags))
		{
			TOGGLE_BIT(pMob->affected_by[0], bits[0]);
			TOGGLE_BIT(pMob->affected_by[1], bits[1]);

			send_to_char("Affect flag toggled.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax: affect [flag]\n\r"
		  "Type '? affect' for a list of flags.\n\r", ch);
    return false;
}

MEDIT(medit_ac)
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do   /* So that I can use break and send the syntax in one place */
    {
	if (argument[0] == '\0')  break;

	EDIT_MOB(ch, pMob);
	argument = one_argument(argument, arg);

	if (!is_number(arg))  break;
	pierce = atoi(arg);
	argument = one_argument(argument, arg);

	if (arg[0] != '\0')
	{
	    if (!is_number(arg))  break;
	    bash = atoi(arg);
	    argument = one_argument(argument, arg);
	}
	else
	    bash = pMob->ac[AC_BASH];

	if (arg[0] != '\0')
	{
	    if (!is_number(arg))  break;
	    slash = atoi(arg);
	    argument = one_argument(argument, arg);
	}
	else
	    slash = pMob->ac[AC_SLASH];

	if (arg[0] != '\0')
	{
	    if (!is_number(arg))  break;
	    exotic = atoi(arg);
	}
	else
	    exotic = pMob->ac[AC_EXOTIC];

	pMob->ac[AC_PIERCE] = pierce;
	pMob->ac[AC_BASH]   = bash;
	pMob->ac[AC_SLASH]  = slash;
	pMob->ac[AC_EXOTIC] = exotic;

	send_to_char("Ac set.\n\r", ch);
	return true;
    } while (false);    /* Just do it once.. */

    send_to_char("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
    return false;
}


MEDIT(medit_form)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(form_flags, argument)) != NO_FLAG)
	{
	    pMob->form ^= value;
	    send_to_char("Form toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: form [flags]\n\r"
		  "Type '? form' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_part)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(part_flags, argument)) != NO_FLAG)
	{
	    pMob->parts ^= value;
	    send_to_char("Parts toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: part [flags]\n\r"
		  "Type '? part' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_immune)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(imm_flags, argument)) != NO_FLAG)
	{
	    pMob->imm_flags ^= value;
	    send_to_char("Immunity toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: imm [flags]\n\r"
		  "Type '? imm' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_res)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(res_flags, argument)) != NO_FLAG)
	{
	    pMob->res_flags ^= value;
	    send_to_char("Resistance toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: res [flags]\n\r"
		  "Type '? res' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_vuln)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(vuln_flags, argument)) != NO_FLAG)
	{
	    pMob->vuln_flags ^= value;
	    send_to_char("Vulnerability toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: vuln [flags]\n\r"
		  "Type '? vuln' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_material)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  material [string]\n\r", ch);
		return false;
    }


	MATERIAL *material = material_lookup(argument);

    if (!IS_VALID(material))
    {
		send_to_char("Invalid material. Type '? materials.'\n\r", ch);
		return false;
    }

    pMob->material = material;

    send_to_char("Material set.\n\r", ch);
    return true;
}


MEDIT(medit_off)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(off_flags, argument)) != NO_FLAG)
	{
	    pMob->off_flags ^= value;
	    send_to_char("Offensive behaviour toggled.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: off [flags]\n\r"
		  "Type '? off' for a list of flags.\n\r", ch);
    return false;
}


MEDIT(medit_size)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
	EDIT_MOB(ch, pMob);

	if ((value = flag_value(size_flags, argument)) != NO_FLAG)
	{
	    pMob->size = value;
	    send_to_char("Size set.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax: size [size]\n\r"
		  "Type '? size' for a list of sizes.\n\r", ch);
    return false;
}


MEDIT(medit_hitdice)
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (ch->tot_level < 151)
    {
	send_to_char("You do not have permission to edit hit dice.\n\r", ch);
	return false;
    }

    if (argument[0] == '\0')
    {
	send_to_char(syntax, ch);
	return false;
    }

    num = cp = argument;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp))  *(cp++) = '\0';

    type = cp;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp)) *(cp++) = '\0';

    bonus = cp;

    while (ISDIGIT(*cp)) ++cp;
    if (*cp != '\0') *cp = '\0';

    if ((!is_number(num  ) || atoi(num  ) < 1)
    ||   (!is_number(type ) || atoi(type ) < 1)
    ||   (!is_number(bonus) || atoi(bonus) < 0))
    {
	send_to_char(syntax, ch);
	return false;
    }

    pMob->hit.number = atoi(num  );
    pMob->hit.size   = atoi(type );
    pMob->hit.bonus  = atoi(bonus);

    send_to_char("Hitdice set.\n\r", ch);
    return true;
}


MEDIT(medit_manadice)
{
    static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char(syntax, ch);
	return false;
    }

    num = cp = argument;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp))  *(cp++) = '\0';

    type = cp;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp)) *(cp++) = '\0';

    bonus = cp;

    while (ISDIGIT(*cp)) ++cp;
    if (*cp != '\0') *cp = '\0';

    if (!(is_number(num) && is_number(type) && is_number(bonus)))
    {
	send_to_char(syntax, ch);
	return false;
    }

    if ((!is_number(num  ) || atoi(num  ) < 1)
    ||   (!is_number(type ) || atoi(type ) < 1)
    ||   (!is_number(bonus) || atoi(bonus) < 0))
    {
	send_to_char(syntax, ch);
	return false;
    }

    pMob->mana.number = atoi(num  );
    pMob->mana.size   = atoi(type );
    pMob->mana.bonus  = atoi(bonus);

    send_to_char("Manadice set.\n\r", ch);
    return true;
}


MEDIT(medit_damdice)
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0')
    {
	send_to_char(syntax, ch);
	return false;
    }

    num = cp = argument;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp))  *(cp++) = '\0';

    type = cp;

    while (ISDIGIT(*cp)) ++cp;
    while (*cp != '\0' && !ISDIGIT(*cp)) *(cp++) = '\0';

    bonus = cp;

    while (ISDIGIT(*cp)) ++cp;
    if (*cp != '\0') *cp = '\0';

    if (!(is_number(num) && is_number(type) && is_number(bonus)))
    {
	send_to_char(syntax, ch);
	return false;
    }

    if ((!is_number(num  ) || atoi(num  ) < 1)
    ||   (!is_number(type ) || atoi(type ) < 1)
    ||   (!is_number(bonus) || atoi(bonus) < 0))
    {
	send_to_char(syntax, ch);
	return false;
    }

    pMob->damage.number = atoi(num  );
    pMob->damage.size   = atoi(type );
    pMob->damage.bonus  = atoi(bonus);

    send_to_char("Damdice set.\n\r", ch);
    return true;
}


MEDIT(medit_race)
{
    MOB_INDEX_DATA *pMob;
    RACE_DATA *race;

    if (argument[0] != '\0')
	{
		race = get_race_data(argument);
		if (IS_VALID(race))
		{
			EDIT_MOB(ch, pMob);

			pMob->race = race;
			pMob->act[0]	  |= race->act[0];
			pMob->act[1]	  |= race->act[1];
			pMob->affected_by[0] |= race->aff[0];
			pMob->affected_by[1] |= race->aff[1];
			pMob->off_flags   |= race->off;
			pMob->imm_flags   |= race->imm;
			pMob->res_flags   |= race->res;
			pMob->vuln_flags  |= race->vuln;
			pMob->form        |= race->form;
			pMob->parts       |= race->parts;

			send_to_char("Race set.\n\r", ch);
			return true;
		}
    }

    if (argument[0] == '?')
    {
		char buf[MAX_STRING_LENGTH];

		send_to_char("Available races are:", ch);
		int i = 1;
		ITERATOR it;
		iterator_start(&it, race_list);
		while((race = (RACE_DATA *)iterator_nextdata(&it)))
		{
			if ((i++ % 3) == 0)
				send_to_char("\n\r", ch);
			sprintf(buf, " %-15s", race->name);
			send_to_char(buf, ch);
		}
		iterator_stop(&it);
		send_to_char("\n\r", ch);
		return false;
    }

    send_to_char("Syntax:  race [race]\n\r"
		  "Type 'race ?' for a list of races.\n\r", ch);
    return false;
}


MEDIT(medit_position)
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument(argument, arg);

    switch (arg[0])
    {
    default:
	break;

    case 'S':
    case 's':
	if (str_prefix(arg, "start"))
	    break;

	if ((value = flag_value(position_flags, argument)) == NO_FLAG)
	    break;

	EDIT_MOB(ch, pMob);

	pMob->start_pos = value;
	send_to_char("Start position set.\n\r", ch);
	return true;

    case 'D':
    case 'd':
	if (str_prefix(arg, "default"))
	    break;

	if ((value = flag_value(position_flags, argument)) == NO_FLAG)
	    break;

	EDIT_MOB(ch, pMob);

	pMob->default_pos = value;
	send_to_char("Default position set.\n\r", ch);
	return true;
    }

    send_to_char("Syntax:  position [start/default] [position]\n\r"
		  "Type '? position' for a list of positions.\n\r", ch);
    return false;
}


MEDIT(medit_movedice)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  move [number]\n\r", ch);
	return false;
    }

    pMob->move = atoi(argument);

    send_to_char("Movement set.\n\r", ch);
    return true;
}


MEDIT(medit_gold)
{
    MOB_INDEX_DATA *pMob;
    long value;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  wealth [number]\n\r", ch);
	return false;
    }

    value = atol(argument);

    if (value > 1000 && !has_imp_sig(pMob, NULL))
    {
	send_to_char("Sorry, that's too much. Have an IMP sign this mob if you want to set that much gold.\n\r", ch);
	return false;
    }

    pMob->wealth = value;
    use_imp_sig(pMob, NULL);

    send_to_char("Wealth set.\n\r", ch);
    return true;
}


MEDIT(medit_hitroll)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  hitroll [number]\n\r", ch);
	return false;
    }

    pMob->hitroll = atoi(argument);

    send_to_char("Hitroll set.\n\r", ch);
    return true;
}


MEDIT (medit_addmprog)
{
	struct trigger_type *tt;
    int value, slot;
    MOB_INDEX_DATA *pMob;
    PROG_LIST *list;
    SCRIPT_DATA *code;
    char trigger[MAX_STRING_LENGTH];
    char phrase[MAX_STRING_LENGTH];
    char num[MAX_STRING_LENGTH];

    EDIT_MOB(ch, pMob);
    argument = one_argument(argument, num);
    argument = one_argument(argument, trigger);
    argument = one_argument(argument, phrase);

	WNUM wnum;

    if (!parse_widevnum(num, ch->in_room->area, &wnum) || trigger[0] =='\0' || phrase[0] =='\0')
    {
	send_to_char("Syntax:   addmprog [wnum] [trigger] [phrase]\n\r",ch);
	return false;
    }

    if (!(tt = get_trigger_type(trigger, PRG_MPROG))) {
	send_to_char("Valid flags are:\n\r",ch);
	show_help(ch, "mprog");
	return false;
    }

    value = tt->type;
    slot = tt->slot;
	if (!wnum.pArea) wnum.pArea = pMob->area;

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
	else if( value == TRIG_EXIT || value == TRIG_EXALL )
	{
		if( !str_cmp(phrase, "*") )
		{
			strcpy(phrase, "-1");
		}
		else
		{
			int door = parse_door(phrase);
			if( door < 0 ) {
				send_to_char("Invalid direction for exit/exall trigger.\n\r", ch);
				return false;
			}
			sprintf(phrase,"%d",door);
		}
	}

    if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_MPROG)) == NULL)
    {
	send_to_char("No such MOBProgram.\n\r",ch);
	return false;
    }

    // Make sure this has a list of progs!
    if(!pMob->progs) pMob->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);

    list->script          = code;
    //SET_BIT(pMob->mprog_flags,value);

    list_appendlink(pMob->progs[slot], list);
	trigger_type_add_use(tt);

    send_to_char("Mprog Added.\n\r",ch);
    return true;
}


MEDIT (medit_delmprog)
{
    MOB_INDEX_DATA *pMob;
    char mprog[MAX_STRING_LENGTH];
    int value;

    EDIT_MOB(ch, pMob);

    one_argument(argument, mprog);
    if (!is_number(mprog) || mprog[0] == '\0')
    {
       send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
       return false;
    }

    value = atol (mprog);

    if (value < 0)
    {
        send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
        return false;
    }

    if(!edit_deltrigger(pMob->progs,value)) {
	send_to_char("No such mprog.\n\r",ch);
	return false;
    }

    send_to_char("Mprog removed.\n\r", ch);
    return true;
}

MEDIT (medit_faction)
{
	char arg[MIL];
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  faction {Radd{x <widevnum>\n\r", ch);
		send_to_char("         faction {Rclear{x\n\r", ch);
		send_to_char("         faction {Rremove{x <#>\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "add"))
	{
		WNUM wnum;
		if (!parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
		{
			send_to_char("Syntax:  faction add {R<widevnum>{x\n\r", ch);
			send_to_char("Please specify a widevnum.\n\r", ch);
			return false;
		}

		REPUTATION_INDEX_DATA *repIndex = get_reputation_index(wnum.pArea, wnum.vnum);
		if (!IS_VALID(repIndex))
		{
			send_to_char("There is no reputation with that widevnum.\n\r", ch);
			return false;
		}

		if (list_hasdata(pMob->factions, repIndex))
		{
			send_to_char("That mob is already associate with that reputation.\n\r", ch);
			return false;
		}

		list_appendlink(pMob->factions, repIndex);
		send_to_char("Faction added.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (list_size(pMob->factions) < 1)
		{
			send_to_char("MobIndex isn't associated with a reputation.\n\r", ch);
			return false;
		}

		list_clear(pMob->factions);
		send_to_char("Factions cleared.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		if (list_size(pMob->factions) < 1)
		{
			send_to_char("MobIndex isn't associated with a reputation.\n\r", ch);
			return false;
		}

		int index;
		if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(pMob->factions))
		{
			send_to_char(formatf("Please specify a number from 1 to %d.\n\r", list_size(pMob->factions)), ch);
			return false;
		}

		list_remnthlink(pMob->factions, index, true);
		send_to_char("Faction removed.\n\r", ch);
		return true;
	}

	medit_faction(ch, "");
	return false;
}


MEDIT (medit_practice)
{
	char arg[MIL];
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		if (pMob->pPractice)
		{
			if (pMob->pPractice->standard)
			{
				send_to_char("Syntax:  practice unassign\n\r", ch);
			}
			else
			{
				send_to_char("Syntax:  practice add skill <name>\n\r", ch);
				send_to_char("         practice add group <name> (creates individual skill entries)\n\r", ch);
				send_to_char("         practice add song <name>\n\r", ch);
				send_to_char("         practice unassign\n\r", ch);

				// Show extra info about the current entry... as there is just way too much to show in medit_show
				send_to_char("         practice <#> show\n\r", ch);

				send_to_char("         practice <#> cost add <minimum rating|acquire>\n\r", ch);
				send_to_char("         practice <#> cost clear\n\r", ch);
				send_to_char("         practice <#> cost <#> price <silver|practices|trains|qp|dp|pneuma|reputation|paragon|custom>[ <check_price script (custom only)>] <value>\n\r", ch);
				send_to_char("         practice <#> cost <#> remove\n\r", ch);

				send_to_char("         practice <#> flags <flags>\n\r", ch);

				send_to_char("         practice <#> reputation <reputation widevnum> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
				// TODO: maybe this can also be scripted... just need a way to pass which practice entry to work with
				send_to_char("         practice <#> maxrating <1-100>\n\r", ch);
				send_to_char("         practice <#> remove\n\r", ch);
			}
		}
		else
		{
			send_to_char("Syntax:  practice assign standard   -- standard practice set\n\r", ch);
			send_to_char("         practice assign custom     -- allow customizing what can be practiced\n\r", ch);
		}
		return false;
	}

	argument = one_argument(argument, arg);
	
	if (pMob->pPractice)
	{
		if (pMob->pPractice->standard)
		{
			if (!str_prefix(argument, "unassign"))
			{
				free_practice_data(pMob->pPractice);
				pMob->pPractice = NULL;

				send_to_char("Practice data unassigned\n\r", ch);
				return true;
			}
		}
		else
		{
			PRACTICE_DATA *prac = pMob->pPractice;
			if (!str_prefix(arg, "add"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  practice add skill <name>\n\r", ch);
					send_to_char("         practice add group <name>   (creates individual skill entries)\n\r", ch);
					send_to_char("         practice add song <name>\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg);
				if (!str_prefix(arg, "group"))
				{
					if (argument[0] == '\0')
					{
						send_to_char("Syntax:  practice add group <name>   (creates individual skill entries)\n\r", ch);
						send_to_char("Please provide a group name.\n\r", ch);
						return false;
					}

					SKILL_GROUP *group = group_lookup(argument);
					if (!IS_VALID(group))
					{
						send_to_char("No such skill group.\n\r", ch);
						return false;
					}

					if (list_size(group->contents) < 1)
					{
						send_to_char("Group has no skills defined.\n\r", ch);
						return false;
					}

					int added = 0;
					ITERATOR skit;
					char *skill_name;
					iterator_start(&skit, group->contents);
					while((skill_name = (char *)iterator_nextdata(&skit)))
					{
						SKILL_DATA *skill = get_skill_data(skill_name);

						if (skill && !__practice_has_entry(prac, skill, NULL))
						{
							__practice_add_entry(prac, skill, NULL);
							send_to_char(formatf("{W%s{x added.\n\r", skill->name), ch);
							added++;
						}
					}
					iterator_stop(&skit);

					if (added > 0)
					{
						send_to_char(formatf("Added {W%d{x skill%s.\n\r", added, ((added == 1)?"":"s")), ch);
						return true;
					}
					else
					{
						send_to_char("Nothing was added.\n\r", ch);
						return false;
					}
				}

				if (!str_prefix(arg, "skill"))
				{
					if (argument[0] == '\0')
					{
						send_to_char("Syntax:  practice add skill <name>\n\r", ch);
						return false;
					}

					SKILL_DATA *skill = get_skill_data(argument);
					if (!skill)
					{
						send_to_char("No such skill by that name.\n\r", ch);
						return false;
					}

					if (__practice_has_entry(prac, skill, NULL))
					{
						send_to_char("That skill is already in the practice data.\n\r", ch);
						return false;
					}

					__practice_add_entry(prac, skill, NULL);
					send_to_char(formatf("Skill {W%s{x added.\n\r", skill->name), ch);
					return true;
				}

				if (!str_prefix(arg, "song"))
				{
					if (argument[0] == '\0')
					{
						send_to_char("Syntax:  practice add song <name>\n\r", ch);
						return false;
					}

					SONG_DATA *song = get_song_data(argument);
					if (!song)
					{
						send_to_char("No such song by that name.\n\r", ch);
						return false;
					}

					if (__practice_has_entry(prac, NULL, song))
					{
						send_to_char("That song is already in the practice data.\n\r", ch);
						return false;
					}

					__practice_add_entry(prac, NULL, song);
					send_to_char(formatf("Song {W%s{x added.\n\r", song->name), ch);
					return true;
				}

				medit_practice(ch, "add");
				return false;
			}

			if (!str_prefix(arg, "unassign"))
			{
				free_practice_data(pMob->pPractice);
				pMob->pPractice = NULL;

				send_to_char("Practice data unassigned\n\r", ch);
				return true;
			}
			
			if (is_number(arg))
			{
				if (list_size(prac->entries) < 1)
				{
					send_to_char("Please add an entry first.\n\r", ch);
					return false;
				}

				if (argument[0] == '\0')
				{
					send_to_char("Syntax:  practice <#> show\n\r", ch);
					send_to_char("         practice <#> cost add <minimum rating|acquire>\n\r", ch);
					send_to_char("         practice <#> cost clear\n\r", ch);
					send_to_char("         practice <#> cost <#> price <silver|practices|trains|qp|dp|pneuma|reputation|paragon|custom>[ <check_price script (custom only)>] <value>\n\r", ch);
					send_to_char("         practice <#> cost <#> remove\n\r", ch);
					send_to_char("         practice <#> reputation <reputation widevnum> <minimum rank#|none> <maximum rank#|none> <minimum show rank#|none> <maximum show rank#|none>\n\r", ch);
					send_to_char("         practice <#> maxrating <1-100>\n\r", ch);
					send_to_char("         practice <#> remove\n\r", ch);
					return false;
				}

				int index = atoi(arg);
				if (index < 1 || index > list_size(prac->entries))
				{
					send_to_char(formatf("Please provide an index from 1 to %d.\n\r", list_size(prac->entries)), ch);
					return false;
				}

				PRACTICE_ENTRY_DATA *entry = (PRACTICE_ENTRY_DATA *)list_nthdata(prac->entries, index);

				argument = one_argument(argument, arg);
				if (!str_prefix(arg, "cost"))
				{
					if (argument[0] == '\0')
					{
						send_to_char("Syntax:  practice <#> cost add <minimum rating|acquire>\n\r", ch);
						send_to_char("         practice <#> cost clear\n\r", ch);
						send_to_char("         practice <#> cost <#> price <silver|practices|trains|qp|dp|pneuma|reputation|paragon|custom>[ <check_price script (custom only)>] <value>\n\r", ch);
						send_to_char("         practice <#> cost <#> remove\n\r", ch);
						return false;
					}

					argument = one_argument(argument, arg);

					if (!str_prefix(arg, "add"))
					{
						if (argument[0] == '\0')
						{
							send_to_char("Syntax:  practice <#> cost add <minimum rating|acquire>\n\r", ch);
							return false;
						}

						int min_rating;
						if (!str_prefix(argument, "acquire"))
							min_rating = 0;
						else if (!is_number(argument) || (min_rating = atoi(argument)) < 1 || min_rating > 100)
						{
							send_to_char("Please provide a minimum rating from 1 to 100, or {Wacquire{x.\n\r", ch);
							return false;
						}

						if (__practice_entry_has_cost(entry, min_rating))
						{
							send_to_char("There is already a cost point for that rating.\n\r", ch);
							return false;
						}

						PRACTICE_COST_DATA *cost = new_practice_cost_data();
						cost->min_rating = min_rating;
						cost->entry = entry;

						__insert_entry_cost(entry, cost);
						send_to_char("Cost point added to practice entry.\n\r", ch);
						return true;
					}
					else if (!str_prefix(arg, "clear"))
					{
						list_clear(entry->costs);
						send_to_char("Cost points cleared.\n\r", ch);
						return true;
					}
					else if (is_number(arg))
					{
						if (list_size(entry->costs) < 1)
						{
							send_to_char("Please add a cost table entry to the practice entry.", ch);
							return false;
						}

						int cindex = atoi(arg);
						if (cindex < 1 || cindex > list_size(entry->costs))
						{
							send_to_char(formatf("Please specify a cost index from 1 to %d\n\r", list_size(entry->costs)), ch);
							return false;
						}

						if (argument[0] == '\0')
						{
							send_to_char("Syntax:  practice <#> cost <#> price <silver|practices|trains|qp|dp|pneuma|reputation|paragon|custom>[ <check_price script (custom only)>] <value>\n\r", ch);
							send_to_char("         practice <#> cost <#> remove\n\r", ch);
							return false;
						}

						PRACTICE_COST_DATA *cost = (PRACTICE_COST_DATA *)list_nthdata(entry->costs, cindex);

						argument = one_argument(argument, arg);
						if (!str_prefix(arg, "price"))
						{
							if (argument[0] == '\0')
							{
								send_to_char("Please specify silver, practices, trains, qp, dp, pneuma, reputation, paragon or custom.\n\r", ch);
								return false;
							}

							argument = one_argument(argument, arg);
							if (!str_prefix(arg, "silver"))
							{
								int silver;
								if (!is_number(argument) || (silver = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->silver = silver;
								send_to_char("Silver set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "practices"))
							{
								int practices;
								if (!is_number(argument) || (practices = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->practices = practices;
								send_to_char("Practices set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "trains"))
							{
								int trains;
								if (!is_number(argument) || (trains = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->trains = trains;
								send_to_char("Trains set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "mp"))
							{
								int mp;
								if (!is_number(argument) || (mp = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->mp = mp;
								send_to_char("Missionpoint set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "dp"))
							{
								int dp;
								if (!is_number(argument) || (dp = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->dp = dp;
								send_to_char("Deitypoint set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "pneuma"))
							{
								int pneuma;
								if (!is_number(argument) || (pneuma = atoi(argument)) < 0)
								{
									send_to_char("Please specify a non-negative number.\n\r", ch);
									return false;
								}

								cost->pneuma = pneuma;
								send_to_char("Pneuma set on cost point.\n\r", ch);
								return true;
							}
							if (!str_prefix(arg, "reputation"))
							{
								int points;
								if (!is_number(argument) || (points = atoi(argument)) < 0)
								{
									send_to_char("Please provide a non-negative number.\n\r", ch);
									return false;
								}

								if (points > 0)
								{
									// Only care if setting a positive value
									if (!IS_VALID(entry->reputation))
									{
										send_to_char("Please assign a reputation to the entry first.\n\r", ch);
										return false;
									}
								}

								cost->rep_points = points;
								send_to_char("Reputation Points set on cost point.\n\r", ch);
								return true;
							}

							if (!str_prefix(arg, "paragon"))
							{
								int levels;
								if (!is_number(argument) || (levels = atoi(argument)) < 0)
								{
									send_to_char("Please provide a non-negative number.\n\r", ch);
									return false;
								}

								if (levels > 0)
								{
									// Only care if setting a positive value
									if (!IS_VALID(entry->reputation))
									{
										send_to_char("Please assign a reputation to the entry first.\n\r", ch);
										return false;
									}

									REPUTATION_INDEX_RANK_DATA *lastRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(entry->reputation->ranks, -1);
									if (!IS_SET(lastRank->flags, REPUTATION_RANK_PARAGON))
									{
										send_to_char("Reputation is not configured to Paragon.\n\r", ch);
										return false;
									}
								}

								cost->paragon_levels = levels;
								send_to_char("Paragon Levels set on cost point.\n\r", ch);
								return true;
							}

							if (!str_prefix(arg, "custom"))
							{
								WNUM wnum;
								argument = one_argument(argument, arg);
								if(argument[0] == '\0' || !parse_widevnum(arg, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
								{
									send_to_char("Please specify a custom price string and check price script.\n\r", ch);
									send_to_char("Syntax:  shop stock [#] price <check price script> custom <value>\n\r\n\r", ch);
									send_to_char("If you wish to clear the custom pricing, select a different pricing type.\n\r", ch);
									return false;
								}

								SCRIPT_DATA *script = get_script_index_wnum(wnum, PRG_MPROG);
								if (!script)
								{
									send_to_char("No such mobprog by that widevnum.\n\r", ch);
									return false;
								}

								cost->silver = 0;
								cost->practices = 0;
								cost->trains = 0;
								cost->mp = 0;
								cost->dp = 0;
								cost->pneuma = 0;
								cost->rep_points = 0;
								cost->paragon_levels = 0;
								free_string(cost->custom_price);
								cost->custom_price = str_dup(argument);
								cost->check_price = script;

								send_to_char("Custom pricing set.\n\r", ch);
								return true;
							}

							medit_practice(ch, formatf("%d cost %d price", index, cindex));
							return false;
						}

						if (!str_prefix(arg, "remove"))
						{
							list_remnthlink(entry->costs, cindex, true);
							free_practice_cost_data(cost);

							send_to_char(formatf("Cost point %d removed.\n\r", cindex), ch);
							return true;
						}

						medit_practice(ch, formatf("%d cost %d", index, cindex));
						return false;
					}

					medit_practice(ch, formatf("%d cost", index));
					return false;
				}

				if (!str_prefix(arg, "flags"))
				{
					long value;
					if ((value = flag_value(practice_entry_flags, argument)) == NO_FLAG)
					{
						send_to_char("Invalid practice entry flag.  Use '? practice_entry' to list valid flags.\n\r", ch);
						show_flag_cmds(ch, practice_entry_flags);
						return false;
					}

					TOGGLE_BIT(entry->flags, value);
					send_to_char("Practice entry flags toggled.\n\r", ch);
					return true;
				}

				if (!str_prefix(arg, "maxrating"))
				{
					// TODO: Add imp_sig / implementer allowance to get this all the way to 100
					int rating;
					if (!is_number(argument) || (rating = atoi(argument)) < 1 || rating > 95)
					{
						send_to_char("Please specify a rating from 1 to 95.\n\r", ch);
						return false;
					}

					entry->max_rating = rating;
					send_to_char("Maximum rating changed.\n\r", ch);
					return true;
				}

				if (!str_prefix(arg, "remove"))
				{
					list_remnthlink(prac->entries, index, true);
					free_practice_entry_data(entry);

					send_to_char(formatf("Entry #%d removed.\n\r", index), ch);
					return true;
				}

				if (!str_prefix(arg, "reputation"))
				{
					WNUM wnum;
					argument = one_argument(argument, arg);
					if (!parse_widevnum(arg, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
					{
						send_to_char("Please provide a widevnum.\n\r", ch);
						return false;
					}

					REPUTATION_INDEX_DATA *repIndex = get_reputation_index_wnum(wnum);
					if (!IS_VALID(repIndex))
					{
						send_to_char("No such reputation with that widevnum.\n\r", ch);
						return false;
					}

					if (list_size(repIndex->ranks) < 1)
					{
						send_to_char("That reputation has no ranks defined.\n\r", ch);
						return false;
					}

					int min_rank;
					argument = one_argument(argument, arg);
					if (!str_prefix(arg, "none"))
						min_rank = 0;
					else if (!is_number(arg) || (min_rank = atoi(arg)) < 1 || min_rank > list_size(repIndex->ranks))
					{
						send_to_char(formatf("Please specify a minimum rank number from 1 to %d, or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
						return false;
					}

					int max_rank;
					argument = one_argument(argument, arg);
					if (!str_prefix(arg, "none"))
						max_rank = 0;
					else if (!is_number(arg) || (max_rank = atoi(arg)) < 1 || max_rank > list_size(repIndex->ranks))
					{
						send_to_char(formatf("Please specify a maximum rank number from 1 to %d, or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
						return false;
					}

					if (min_rank > 0 && max_rank > 0 && min_rank > max_rank)
					{
						send_to_char("Minimum rank must not be greater than the maximum rank.\n\r", ch);
						return false;
					}


					int min_show_rank;
					argument = one_argument(argument, arg);
					if (!str_prefix(arg, "none"))
						min_show_rank = 0;
					else if (!is_number(arg) || (min_show_rank = atoi(arg)) < 1 || min_show_rank > list_size(repIndex->ranks))
					{
						send_to_char(formatf("Please specify a minimum show rank number from 1 to %d, or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
						return false;
					}

					int max_show_rank;
					argument = one_argument(argument, arg);
					if (!str_prefix(arg, "none"))
						max_show_rank = 0;
					else if (!is_number(arg) || (max_show_rank = atoi(arg)) < 1 || max_show_rank > list_size(repIndex->ranks))
					{
						send_to_char(formatf("Please specify a maximum show rank number from 1 to %d, or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
						return false;
					}

					if (min_show_rank > 0 && max_show_rank > 0 && min_show_rank > max_show_rank)
					{
						send_to_char("Minimum show rank must not be greater than the maximum show rank.\n\r", ch);
						return false;
					}

					entry->reputation = repIndex;
					entry->min_reputation_rank = min_rank;
					entry->max_reputation_rank = max_rank;
					entry->min_show_rank = min_show_rank;
					entry->max_show_rank = max_show_rank;

					return true;
				}

				if (!str_prefix(arg, "show"))
				{
					char buf[MSL];
					ITERATOR pcit;
					PRACTICE_COST_DATA *cost;

					BUFFER *buffer = new_buf();

					if (entry->skill)
						sprintf(buf, "Skill: %s\n\r", entry->skill->name);
					else if (entry->song)
						sprintf(buf, "Song: %s\n\r", entry->song->name);
					else
						sprintf(buf, "Unknown: ???\n\r");
					add_buf(buffer, buf);

					sprintf(buf, "Maximum Rating: %d%%\n\r", entry->max_rating);
					add_buf(buffer, buf);

					if (IS_VALID(entry->reputation))
					{
						add_buf(buffer, "Reputation:\n\r");
						add_buf(buffer, formatf("  %s (%ld#%ld)\n\r",
							entry->reputation->name,
							entry->reputation->area->uid,
							entry->reputation->vnum));

						REPUTATION_INDEX_RANK_DATA *minRank;
						if (entry->min_reputation_rank > 0)
							minRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(entry->reputation->ranks, entry->min_reputation_rank);
						else
							minRank = NULL;

						if (!IS_VALID(minRank))
							add_buf(buffer, formatf("  Minimum Rank: %s (%d)\n\r", minRank->name, entry->min_reputation_rank));
						else
							add_buf(buffer, "  Minimum Rank: none\n\r");

						REPUTATION_INDEX_RANK_DATA *maxRank;
						if (entry->max_reputation_rank > 0)
							maxRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(entry->reputation->ranks, entry->max_reputation_rank);
						else
							maxRank = NULL;

						if (!IS_VALID(maxRank))
							add_buf(buffer, formatf("  Maximum Rank: %s (%d)\n\r", maxRank->name, entry->max_reputation_rank));
						else
							add_buf(buffer, "  Maximum Rank: none\n\r");

						REPUTATION_INDEX_RANK_DATA *minShowRank;
						if (entry->min_show_rank > 0)
							minShowRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(entry->reputation->ranks, entry->min_show_rank);
						else
							minShowRank = NULL;

						if (!IS_VALID(minShowRank))
							add_buf(buffer, formatf("  Minimum Show Rank: %s (%d)\n\r", minShowRank->name, entry->min_show_rank));
						else
							add_buf(buffer, "  Minimum Show Rank: none\n\r");

						REPUTATION_INDEX_RANK_DATA *maxShowRank;
						if (entry->max_show_rank > 0)
							maxShowRank = (REPUTATION_INDEX_RANK_DATA *)list_nthdata(entry->reputation->ranks, entry->max_show_rank);
						else
							maxShowRank = NULL;

						if (!IS_VALID(maxShowRank))
							add_buf(buffer, formatf("  Maximum Show Rank: %s (%d)\n\r", maxShowRank->name, entry->max_show_rank));
						else
							add_buf(buffer, "  Maximum Show Rank: none\n\r");
					}
					else
						add_buf(buffer, "Reputation:\n\r  none\n\r");
					add_buf(buffer, "\n\r");

					if (list_size(entry->costs) > 0)
					{
						add_buf(buffer, "Costs:\n\r");

						add_buf(buffer, "###  Rating    Price\n\r");
						add_buf(buffer, "=== ========= ==========================================\n\r");

						int iCost = 0;
						iterator_start(&pcit, entry->costs);
						while((cost = (PRACTICE_COST_DATA *)iterator_nextdata(&pcit)))
						{
							char rating[MIL];
							if (cost->min_rating > 0)
								sprintf(rating, "%6d{W%%{x", cost->min_rating);
							else
								strcpy(rating, "{Yacquire{x");

							char price[MIL];

							if (IS_NULLSTR(cost->custom_price) || !cost->check_price)
							{
								int pr = 0;
								if (cost->silver > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}
									int s = cost->silver % 100;
									long g = cost->silver / 100;

									if (g > 0)
									{
										if (s > 0)
											pr += sprintf(&price[pr], "%ld{Yg{x %d{Ws{x", g, s);
										else
											pr += sprintf(&price[pr], "%ld{Yg{x", g);
									}
									else
										pr += sprintf(&price[pr], "%d{Ws{x", s);
								}

								if (cost->practices > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Gp{x", cost->practices);
								}

								if (cost->trains > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Gt{x", cost->trains);
								}

								if (cost->mp > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Yqp{x", cost->mp);
								}

								if (cost->dp > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Ydp{x", cost->dp);
								}

								if (cost->pneuma > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Cpn{x", cost->pneuma);
								}

								if (cost->rep_points > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Brep{x", cost->rep_points);
								}

								if (cost->paragon_levels > 0)
								{
									if (pr > 0)
									{
										price[pr++] = ',';
										price[pr++] = ' ';
									}

									pr += sprintf(&price[pr], "%ld{Y*{x", cost->paragon_levels);
								}

								price[pr] = '\0';
							}
							else
							{
								sprintf(price, "%s [%s (%ld#%ld)]\n\r",
									cost->custom_price,
									cost->check_price->name,
									cost->check_price->area->uid,
									cost->check_price->vnum);
							}

							sprintf(buf, "%3d  %s   %s\n\r", ++iCost, rating, price);
							add_buf(buffer, buf);
						}
						iterator_stop(&pcit);
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


				return false;
			}
		}
	}
	else
	{
		if (!str_prefix(arg, "assign"))
		{
			bool standard;
			if (!str_prefix(argument, "standard"))
			{
				send_to_char("Standard practice suite not implemented yet.\n\r", ch);
				return false;
			}
			else if (!str_prefix(argument, "custom"))
			{
				standard = false;
				send_to_char("Adding Custom Practice suite.\n\r", ch);
			}
			else
			{
				medit_practice(ch, "");
				return false;	
			}

			pMob->pPractice = new_practice_data();
			pMob->pPractice->standard = standard;
			return true;
		}
	}

	medit_practice(ch, "");
	return false;
}

MEDIT (medit_addreputation)
{
	char arg[MIL];
	MOB_INDEX_DATA *pMob;
	WNUM wnum;

	EDIT_MOB(ch, pMob);

	argument = one_argument(argument, arg);
	if (!parse_widevnum(arg, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
	{
		send_to_char("Syntax:  addreputation {R<reputation widevnum>{x <minimum rank|none> <maximum rank|none> <points>\n\r", ch);
		send_to_char("Please specify a valid widevnum.\n\r", ch);
		return false;
	}

	REPUTATION_INDEX_DATA *repIndex = get_reputation_index(wnum.pArea, wnum.vnum);
	if (!IS_VALID(repIndex))
	{
		send_to_char("No reputation with that widevnum.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	int min_rank;
	if (is_number(arg))
	{
		min_rank = atoi(arg);
		if (min_rank < 1 || min_rank > list_size(repIndex->ranks))
		{
			send_to_char("Syntax:  addreputation <reputation widevnum> {R<minimum rank|none>{x <maximum rank|none> <points>\n\r", ch);
			send_to_char(formatf("Invalid rank number.  Please specify a number from 1 to %d or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
			return false;
		}
	}
	else if(!str_prefix(arg, "none"))
	{
		min_rank = 0;
	}
	else
	{
		send_to_char("Syntax:  addreputation <reputation widevnum> {R<minimum rank|none>{x <maximum rank|none> <points>\n\r", ch);
		send_to_char(formatf("Invalid rank number.  Please specify a number from 1 to %d or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
		return false;
	}


	argument = one_argument(argument, arg);
	int max_rank;
	if (is_number(arg))
	{
		max_rank = atoi(arg);
		if (max_rank < 1 || max_rank > list_size(repIndex->ranks))
		{
			send_to_char("Syntax:  addreputation <reputation widevnum> <minimum rank|none> {R<maximum rank|none>{x <points>\n\r", ch);
			send_to_char(formatf("Invalid rank number.  Please specify a number from 1 to %d or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
			return false;
		}
	}
	else if(!str_prefix(arg, "none"))
	{
		max_rank = 0;
	}
	else
	{
		send_to_char("Syntax:  addreputation <reputation widevnum> <minimum rank|none> {R<maximum rank|none>{x <points>\n\r", ch);
		send_to_char(formatf("Invalid rank number.  Please specify a number from 1 to %d or {Wnone{x.\n\r", list_size(repIndex->ranks)), ch);
		return false;
	}

	if (min_rank && max_rank && min_rank > max_rank)
	{
		send_to_char("Syntax:  addreputation <reputation widevnum> {R<minimum rank> <maximum rank>{x <points>\n\r", ch);
		send_to_char("Minimum rank must not be greater than maximum rank.\n\r", ch);
		return false;
	}

	long points;
	if (!is_number(argument) || !(points = atol(argument)))
	{
		send_to_char("Syntax:  addreputation <reputation widevnum> <minimum rank|none> <maximum rank|none> {R<points>{x\n\r", ch);
		send_to_char("Please specify a nonzero number.\n\r", ch);
		return false;
	}

	MOB_REPUTATION_DATA *rep, *new_rep;
	new_rep = new_mob_reputation_data();
	new_rep->reputation = repIndex;
	new_rep->minimum_rank = min_rank;
	new_rep->maximum_rank = max_rank;
	new_rep->points = points;
	new_rep->next = NULL;

	for(rep = pMob->reputations; rep && rep->next; rep = rep->next);

	if (rep)
		rep->next = new_rep;
	else
		pMob->reputations = new_rep;

	send_to_char("Reputation added.\n\r", ch);
	return true;
}


MEDIT (medit_delreputation)
{
    MOB_INDEX_DATA *pMob;
    char mprog[MAX_STRING_LENGTH];
    int value;

    EDIT_MOB(ch, pMob);

    one_argument(argument, mprog);
    if (!is_number(mprog) || mprog[0] == '\0')
    {
       send_to_char("Syntax:  delreputation <number>\n\r",ch);
       return false;
    }

    value = atol (mprog);

    if (value < 0)
    {
        send_to_char("Please specify a non-negative number.\n\r",ch);
        return false;
    }

	MOB_REPUTATION_DATA *prev, *rep;
	for(prev = NULL, rep = pMob->reputations; rep && value--; prev = rep, rep = rep->next);

	if (!rep)
	{
		send_to_char("No such reputation.\n\r", ch);
		return false;
	}

	if (prev)
		prev->next = rep->next;
	else
		pMob->reputations = rep->next;
	free_mob_reputation_data(rep);
	
    send_to_char("Reputation removed.\n\r", ch);
    return true;
}


// TODO: COMPLETE
MEDIT(medit_addquest)
{
    MOB_INDEX_DATA *pMob;
    QUEST_LIST *quest;
    QUEST_INDEX_DATA *pQuestIndex;
    WNUM wnum;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1)
    {
	send_to_char("Syntax:  addquest [quest wnum]\n\r", ch);
	return false;
    }

    pQuestIndex = get_quest_index(wnum.pArea, wnum.vnum);
    if (pQuestIndex == NULL)
    {
	send_to_char("That quest doesn't exist.\n\r", ch);
	return false;
    }

    for (quest = pMob->quests; quest != NULL; quest = quest->next)
    {
	if (quest->wnum.pArea == wnum.pArea && quest->wnum.vnum == wnum.vnum)
	{
	    send_to_char("That would be redundant as you've already added that quest.\n\r", ch);
	    return false;
	}
    }

    quest = new_quest_list();
	quest->wnum.pArea = pQuestIndex->area;
    quest->wnum.vnum = pQuestIndex->vnum;

    quest->next = pMob->quests;
    pMob->quests = quest;

    send_to_char("Quest added.\n\r", ch);

    return true;
}


MEDIT(medit_delquest)
{
    MOB_INDEX_DATA *pMob;
    QUEST_LIST *quest_list;
    QUEST_LIST *prev_quest_list = NULL;
    int i;
    int counter;

    EDIT_MOB(ch, pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  delquest [#]\n\r", ch);
	return false;
    }

    i = atoi (argument);
    counter = 0;
    for (quest_list = pMob->quests; quest_list != NULL;
	  quest_list = quest_list->next)
    {
	if (i == counter)
	    break;

	counter++;
	prev_quest_list = quest_list;
    }

    if (quest_list == NULL)
    {
	send_to_char("Number not found.\n\r", ch);
	return false;
    }

    if (prev_quest_list != NULL)
	prev_quest_list->next = quest_list->next;
    else
	pMob->quests = quest_list->next;

    free_quest_list(quest_list);
    send_to_char("Quest removed.\n\r", ch);
    return true;
}


MEDIT(medit_missionary)
{
	MOB_INDEX_DATA *pMob;
	char arg[MIL];

	EDIT_MOB(ch, pMob);

	if(IS_NULLSTR(argument))
	{
		send_to_char("MISSIONARY ADD                 Adds missionary data to mob.\n\r", ch);
		send_to_char("        REMOVE              Removes missionary data from mob.\n\r", ch);
		send_to_char("        SCROLL [wnum]       Sets the scroll object to the specified widevnum.\n\r", ch);
		send_to_char("        KEYWORDS [string]   Sets keywords of scroll.\n\r", ch);
		send_to_char("        SHORT [string]      Sets short description of scroll.\n\r", ch);
		send_to_char("        LONG [string]       Sets long description of scroll.\n\r", ch);
		send_to_char("        HEADER              Edits scroll header.\n\r", ch);
		send_to_char("        FOOTER              Edits scroll footer.\n\r", ch);
		send_to_char("        PREFIX [string]     Edits line prefix.\n\r", ch);
		send_to_char("        SUFFIX [string]     Edits line suffix.\n\r", ch);
		send_to_char("        WIDTH [width]       Sets line width.\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if (!str_prefix(arg,"add"))
	{
	    if (!str_cmp(pMob->sig, "none") && !IS_IMPLEMENTOR(ch))
	    {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }

	    if( pMob->pMissionary != NULL )
	    {
			send_to_char("There is already missionary data.\n\r", ch);
			return false;
		}

		pMob->pMissionary = new_missionary_data();
	    use_imp_sig(pMob, NULL);
		send_to_char("Missionary data added.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"remove")) {
	    if (!str_cmp(pMob->sig, "none") && !IS_IMPLEMENTOR(ch))
	    {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }

	    if( pMob->pMissionary == NULL )
	    {
			send_to_char("There is any missionary data.\n\r", ch);
			return false;
		}

		free_missionary_data(pMob->pMissionary);
		pMob->pMissionary = NULL;
		send_to_char("Missionary data removed.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"scroll")) {
		WNUM wnum;
		if(!parse_widevnum(argument, ch->in_room->area, &wnum))
		{
			send_to_char("That is not a widevnum.\n\r", ch);
			return false;
		}

		if( !get_obj_index(wnum.pArea, wnum.vnum) )
		{
			send_to_char("Object does not exist.\n\r", ch);
			return false;
		}

		pMob->pMissionary->scroll.auid = wnum.pArea->uid;
		pMob->pMissionary->scroll.vnum = wnum.vnum;
		send_to_char("Missionary scroll object changed.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"keywords")) {
		free_string(pMob->pMissionary->keywords);
		pMob->pMissionary->keywords = str_dup(argument);

		send_to_char("Keywords set.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"short")) {
		free_string(pMob->pMissionary->short_descr);
		pMob->pMissionary->short_descr = str_dup(argument);

		send_to_char("Short description set.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"long")) {
		free_string(pMob->pMissionary->long_descr);
		pMob->pMissionary->long_descr = str_dup(argument);

		send_to_char("Long description set.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"header")) {
		send_to_char("Editting the Missionary Header:\n\r", ch);
		send_to_char("  Use {Y$PLAYER${x as a placeholder for the player's name.\n\r", ch);
		send_to_char("  Use {Y$MISSIONARY${x as a placeholder for the questgiver's name.\n\r", ch);
		send_to_char("\n\r", ch);

		string_append(ch, &pMob->pMissionary->header);
		return true;

	} else if (!str_prefix(arg,"footer")) {
		send_to_char("Editting the Missionary Footer:\n\r", ch);
		send_to_char("  Use {Y$PLAYER${x as a placeholder for the player's name.\n\r", ch);
		send_to_char("  Use {Y$MISSIONARY${x as a placeholder for the questgiver's name.\n\r", ch);
		send_to_char("\n\r", ch);

		string_append(ch, &pMob->pMissionary->footer);
		return true;

	} else if (!str_prefix(arg,"prefix")) {
		free_string(pMob->pMissionary->prefix);
		pMob->pMissionary->prefix = str_dup(argument);

		send_to_char("Prefix set.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"suffix")) {
		free_string(pMob->pMissionary->suffix);
		pMob->pMissionary->suffix = str_dup(argument);

		send_to_char("Prefix set.\n\r", ch);
		return true;

	} else if (!str_prefix(arg,"width")) {
		if(!is_number(argument))
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int width = atoi(argument);
		if( width <= 0 )
		{
			pMob->pMissionary->line_width = 0;
			send_to_char("Line width disabled.\n\r", ch);
			return true;

		}
		else if(width > 160)
		{
			send_to_char("Width is out of range.  Please specify a number from 1 to 160, or 0 to disable width.\n\r", ch);
			return false;
		}

		pMob->pMissionary->line_width = width;
		send_to_char("Line width set.\n\r", ch);
		return true;

	} else {
		medit_missionary(ch, "");
		return false;
	}

	return true;

}

MEDIT( medit_crew )
{
	MOB_INDEX_DATA *pMob;
	char arg[MIL];

	EDIT_MOB(ch, pMob);

	if(IS_NULLSTR(argument))
	{
		send_to_char("Syntax:  crew assign\n\r", ch);
		send_to_char("         crew remove\n\r", ch);
		send_to_char("         crew minrank <rank>\n\r", ch);
		send_to_char("         crew scouting <rating>\n\r", ch);
		send_to_char("         crew gunning <rating>\n\r", ch);
		send_to_char("         crew oarring <rating>\n\r", ch);
		send_to_char("         crew mechanics <rating>\n\r", ch);
		send_to_char("         crew navigation <rating>\n\r", ch);
		send_to_char("         crew leadership <rating>\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if( !str_prefix(arg, "assign") )
	{
		if( IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile already has ship crew data.\n\r", ch);
			return false;
		}

		pMob->pCrew = new_ship_crew_index();
		send_to_char("Ship Crew assigned.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "remove") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile has no ship crew data.\n\r", ch);
			return false;
		}

		free_ship_crew_index(pMob->pCrew);
		pMob->pCrew = NULL;
		send_to_char("Ship Crew removed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "minrank") )
	{
		send_to_char("Not implemented yet.\n\r", ch);
		return false;
	}

	if( !str_prefix(arg, "scouting") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->scouting = value;
		send_to_char("Scouting Rating changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "gunning") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->gunning = value;
		send_to_char("Gunning Rating changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "oarring") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->oarring = value;
		send_to_char("Oarring Rating changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "mechanics") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->mechanics = value;
		send_to_char("Mechanics Rating changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "navigation") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->navigation = value;
		send_to_char("Navigation Rating changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "leadership") )
	{
		if( !IS_VALID(pMob->pCrew) )
		{
			send_to_char("Mobile is not assigned as a ship crew.\n\r", ch);
			return false;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return false;
		}

		int value = atoi(argument);
		if( value < 0 || value > 100 )
		{
			send_to_char("Rating out of range.  Please specify a value from 0 to 100.\n\r", ch);
			return false;
		}

		pMob->pCrew->leadership = value;
		send_to_char("Leadership Rating changed.\n\r", ch);
		return true;
	}

	medit_crew(ch, "");
	return false;
}