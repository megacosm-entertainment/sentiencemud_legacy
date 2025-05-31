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

#include "oedit.h"



OEDIT(oedit_show)
{
    OBJ_INDEX_DATA *pObj;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;


    SPELL_DATA *spell;
    int cnt;

    EDIT_OBJ(ch, pObj);

    buffer = new_buf();

    sprintf(buf, "Name:         {B[{x%s{B]{x\n\rArea:         {B[{x%7ld{B] {x%s\n\r",
	pObj->name,
	!pObj->area ? -1        : pObj->area->uid,
	!pObj->area ? "No Area" : pObj->area->name);
    add_buf(buffer, buf);

    sprintf(buf, "Vnum:         {B[{x%7ld{B]{x\n\rType:         {B[{x%s{B]{x\n\r",
	pObj->vnum,
	flag_string(type_flags, pObj->item_type));
    add_buf(buffer, buf);

	sprintf(buf, "Immortal:     {B[%s{B]{x\n\r", (pObj->immortal ? "{WON" : "{Doff"));
	add_buf(buffer, buf);

    sprintf(buf, "Persist:      {B[%s{B]{x\n\r", (pObj->persist ? "{WON" : "{Doff"));
    add_buf(buffer, buf);

    sprintf(buf, "Level:        {B[{x%7d{B]{x\n\r", pObj->level);
    add_buf(buffer, buf);

    sprintf(buf, "Wear flags:   {B[{x%s{B]{x\n\r",
	flag_string(wear_flags, pObj->wear_flags));
    add_buf(buffer, buf);

    sprintf(buf, "Imp sig:      {B[{x%s{B]{x\n\r",
		    pObj->imp_sig);
    add_buf(buffer, buf);

    sprintf(buf, "Creator sig:  {B[{x%s{B]{x\n\r",
		    pObj->creator_sig);
    add_buf(buffer, buf);

    sprintf(buf, "Script Kwds:  {B[{x%s{B]{x\n\r",
    		pObj->skeywds);
    add_buf(buffer, buf);

	sprintf(buf, "Extra flags:  {B[{x%s{B]{x\n\r", bitvector_string(4,
		pObj->extra[0], extra_flags,
		pObj->extra[1], extra2_flags,
		pObj->extra[2], extra3_flags,
		pObj->extra[3], extra4_flags));
	add_buf(buffer, buf);

	sprintf(buf, "Class:        {B[{x%s{B]{x\n\r", IS_VALID(pObj->clazz) ? pObj->clazz->name : "none");
	add_buf(buffer, buf);

	sprintf(buf, "Class Type:   {B[{x%s{B]{x\n\r", flag_string(class_types, pObj->clazz_type));
	add_buf(buffer, buf);

	add_buf(buffer, "Race:\n\r");
	if (list_size(pObj->race) > 0)
	{
		int i = 0;
		ITERATOR rit;
		RACE_DATA *race;
		iterator_start(&rit, pObj->race);
		while((race = (RACE_DATA *)iterator_nextdata(&rit)))
		{
			sprintf(buf, " %-2d %s\n\r", ++i, race->name);
			add_buf(buffer, buf);
		}
		iterator_stop(&rit);
	}
	else
		add_buf(buffer, "  None\n\r");

    sprintf(buf, "OUpdate:      {B[{x%s{B]{x\n\r",
    	pObj->update == true ? "Yes" : "No");
    add_buf(buffer, buf);

    sprintf(buf, "Timer:        {B[{x%d{B]{x\n\r",
        pObj->timer);
    add_buf(buffer, buf);

	if (IS_VALID(pObj->material))
	    sprintf(buf, "Material:     {B[{x%s{B]{x\n\r", pObj->material->name);
	else
		sprintf(buf, "Material:     {B[{Dnothing{B]{x\n\r");
    add_buf(buffer, buf);

    sprintf(buf, "Condition:    {B[{x%7d{B]{x\n\r",               /* ROM */
	pObj->condition);
    add_buf(buffer, buf);

    sprintf(buf, "Fragility:    {B[{x%7s{B]{x\n\r",               /* ROM */
	fragile_table[pObj->fragility].name);

    add_buf(buffer, buf);

    sprintf(buf, "Allwd Fixed:  {B[{x%7d{B]{x\n\r",               /* ROM */
	pObj->times_allowed_fixed);
    add_buf(buffer, buf);

    sprintf(buf, "Weight:       {B[{x%7d{B]{x\n\r"
		 "Cost:         {B[{x%7ld{B]{x\n\r",
	pObj->weight, pObj->cost);
    add_buf(buffer, buf);

    sprintf(buf, "Points:       {B[{x%7d{B]{x\n\r",
         pObj->points);
    add_buf(buffer, buf);

	/*
    if( pObj->lock )
    {
		OBJ_INDEX_DATA *lock_key = get_obj_index(pObj->lock->key_wnum.pArea, pObj->lock->key_wnum.vnum);

	    sprintf(buf,"Lock State:\n\r"
	    			"  Key:         {B[{x%ld#%ld{B]{x %s\n\r"
	    			"  Flags:       {B[{x%s{B]{x\n\r"
	    			"  Pick Chance: {B[{x%d%%{B]{x\n\r",
	    			pObj->lock->key_wnum.pArea ? pObj->lock->key_wnum.pArea->uid : 0,
					pObj->lock->key_wnum.vnum,
	    			lock_key ? lock_key->short_descr : "none",
	    			flag_string(lock_flags, pObj->lock->flags),
	    			pObj->lock->pick_chance);
	    add_buf(buffer, buf);
	}
	*/

    if (pObj->extra_descr)
    {
	EXTRA_DESCR_DATA *ed;

	add_buf(buffer, "Ex desc kwd: ");

	for (ed = pObj->extra_descr; ed; ed = ed->next)
	{
	    add_buf(buffer, "[");
	    sprintf(buf, "%s", ed->keyword);
	    add_buf(buffer, buf);
	    add_buf(buffer, "]");
	}

	add_buf(buffer, "\n\r");
    }

    sprintf(buf, "Short desc:{x   %s\n\rLong desc:{x\n\r     %s\n\r",
	pObj->short_descr, pObj->description);
    add_buf(buffer, buf);

    add_buf(buffer, "Description:{x\n\r");
    sprintf(buf, "%s", pObj->full_description);
    add_buf(buffer, buf);

	sprintf(buf, "\n\r-----\n\r{WBuilders' Comments:{X\n\r%s\n\r-----\n\r", pObj->comments);
	add_buf(buffer, buf);

    for (cnt = 0, paf = pObj->affected; paf; paf = paf->next)
    {
		if( paf->where == TO_OBJECT )
		{
			if (cnt == 0)
			{
				sprintf(buf, "{Y%-6s %-20s %-10s %-10s{x\n\r", "Number", "Affects", "Modifier", "Random");
				add_buf(buffer, buf);

				sprintf(buf, "{Y%-6s %-20s %-10s %-10s{x\n\r", "------", "-------", "--------", "------");
				add_buf(buffer, buf);
			}

			sprintf(buf, "{B[{W%4d{B] {%c%-20s{x %-20d %d%%\n\r",
				cnt,
				(paf->location >= APPLY_SKILL)?'Y':'G',
				affect_loc_name(paf->location),
				paf->modifier,
				paf->random);

			add_buf(buffer, buf);
			cnt++;
		}
    }

    for (cnt = 0, paf = pObj->affected; paf; paf = paf->next)
    {
		if( paf->where == TO_IMMUNE || paf->where == TO_RESIST || paf->where == TO_VULN )
		{
			char* irv;

			if(paf->where == TO_IMMUNE)
				irv = "Wimmunity";
			else if(paf->where == TO_VULN)
				irv = "Rvulnerability";
			else
				irv = "Gresistance";

			if (cnt == 0)
			{
				sprintf(buf, "{C%-6s %-15s %-15s %-10s{x\n\r", "Number", "Adds", "Modifier", "Random");
				add_buf(buffer, buf);

				sprintf(buf, "{C%-6s %-15s %-15s %-10s{x\n\r", "------", "-------", "--------", "------");
				add_buf(buffer, buf);
			}

			sprintf(buf, "{B[{W%4d{B] {%-16s{x %-15s %d%%\n\r",
				cnt,
				irv,
				imm_bit_name(paf->bitvector),
				paf->random);

			add_buf(buffer, buf);
			cnt++;
		}
    }

    if (pObj->spells)
    {
		cnt = 0;

		sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
		add_buf(buffer, buf);

		sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
		add_buf(buffer, buf);

		for (spell = pObj->spells; spell != NULL; spell = spell->next, cnt++)
		{
			sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
				cnt,
				spell->skill->name, spell->level, spell->repop);
			buf[0] = UPPER(buf[0]);
			add_buf(buffer, buf);
		}
    }

    if (pObj->catalyst)
    {
		cnt = 0;
		char line_colour = 'x';

		sprintf(buf, "{m%-6s %-20s %-6s %-6s %-11s{x\n\r", "Number", "Type", "Amount", "Random", "Script Name");
		add_buf(buffer, buf);

		sprintf(buf, "{m%-6s %-20s %-6s %-6s %-11s{x\n\r", "------", "----", "------", "------", "-----------");
		add_buf(buffer, buf);

		for (paf = pObj->catalyst; paf; paf = paf->next, cnt++) {
			line_colour = ( paf->where == TO_CATALYST_ACTIVE ) ? 'W' : 'x';

			char *name = (IS_NULLSTR(paf->custom_name)) ? "---" : paf->custom_name;

			if(paf->modifier < 0)
				sprintf(buf, "{M[{W%4d{M]{%c %-20s {Wsource{%c %d%% %s{x\n\r", cnt, line_colour,
					flag_string(catalyst_types,paf->catalyst_type),line_colour,paf->random, name);
			else
				sprintf(buf, "{M[{W%4d{M]{%c %-20s %-6d %d%% %s{x\n\r", cnt, line_colour,
					flag_string(catalyst_types,paf->catalyst_type),paf->modifier,paf->random, name);
			buf[0] = UPPER(buf[0]);
			add_buf(buffer, buf);
		}
    }

	if (pObj->script_visible)
		sprintf(buf, "Visibility: %s (%s)\n\r", widevnum_string_script(pObj->script_visible, NULL), pObj->script_visible->name);
	else
		sprintf(buf, "Visibility: none\n\r");
	add_buf(buffer, buf);

    if (pObj->progs)
		olc_show_progs(buffer, pObj->progs, PRG_OPROG, "ObjProg Vnum");

	olc_show_index_vars(buffer, pObj->index_vars);


    print_obj_values(pObj, buffer);

    page_to_char(buf_string(buffer), ch);
    free_buf(buffer);
    return false;
}


OEDIT(oedit_addaffect)
{
    long value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf, *pAf_tmp;
    int pMod;
    bool pNeg = false;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char randm[MAX_STRING_LENGTH];
    char buf[MSL];

    EDIT_OBJ(ch, pObj);

    argument = one_argument(argument, loc);

    if (loc[0] == '\0'
    || mod[0] == '\0'
    || randm[0] == '\0'
    || !is_number(randm)
    || !is_number(mod))
    {
	send_to_char("Syntax:  addaffect [location] [#xmod] [#rand]\n\r", ch);
	send_to_char("Syntax:  addaffect saves [damageclass] [#xmod] [#rand]\n\r", ch);
	return false;
    }

    if ((value = flag_value(apply_flags, loc)) == NO_FLAG) /* Hugin */
    {
        send_to_char("Valid affects are:\n\r", ch);
	show_help(ch, "apply");
	return false;
    }

	if (value == APPLY_SAVES)
	{
		char dt[MIL];

		argument = one_argument(argument, dt);
		int dam_type = stat_lookup(dt, damage_classes, NO_FLAG);

		if (dam_type != NO_FLAG)
		{
			if (dam_type == DAM_NONE)
			{
				send_to_char("{Wnone{x damage class is not allowed.  Try a different damage class.\n\r", ch);
				show_flag_cmds(ch, damage_classes);
				return false;
			}

			value += dam_type;
		}
		else
		{
			send_to_char("Invalid damage class.  Use '? damageclass' for list of classes.\n\r", ch);
			show_flag_cmds(ch, damage_classes);
			return false;
		}
	}

    argument = one_argument(argument, mod);
    argument = one_argument(argument, randm);

    for (pAf = pObj->affected; pAf != NULL; pAf = pAf->next)
    {
	if (pAf->where == TO_OBJECT && pAf->location == value)
	{
	    sprintf(buf, "There's already a %s modifier on that item.\n\r",
	        flag_string(apply_flags, value));
	    send_to_char(buf, ch);
	    return false;
	}
    }

    switch(value)
    {
        case APPLY_HIT:
	case APPLY_MANA:
		pMod = (int) atoi(mod)/10;
		if (pMod == 0) pMod = 1;
		if (atoi(mod) < 0) pNeg = true;
		break;
	case APPLY_MOVE:
		pMod = (int) atoi(mod)/20;
		if (pMod == 0) pMod = 1;
		if (atoi(mod) < 0) pNeg = true;
		break;
	case APPLY_DEX:
	case APPLY_WIS:
	case APPLY_INT:
	case APPLY_STR:
	case APPLY_CON:
		pMod = atoi(mod);
		if (atoi(mod) < 0) pNeg = true;
		break;
	case APPLY_AC:
		pMod = (int) atoi(mod)/10;
		if (pMod == 0) pMod = 1;
		if (atoi(mod) < 0) pNeg = true;
		break;
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
		pMod = (int) atoi(mod)/2;
		if (pMod == 0) pMod = 1;
		if (atoi(mod) < 0) pNeg = true;
		break;
	case APPLY_XPBOOST:
		pMod = atoi(mod);
		if (pMod == 0) pMod = 1;
		if (atoi(mod) > 0) pNeg = true;
		break;
	default:
		if (value >= APPLY_SAVES && value < APPLY_SAVES_MAX)
		{
			pMod = atoi(mod);
			if (pMod == 0) pMod = 1;
			if (atoi(mod) < 0) pNeg = true;
		}
		else
		{
			pMod = 1;
			pNeg = false;
		}
		break;
    }

    /*
     * Modify based on random. This prevents people adding 123123
     * negative affects which dont ever actually repop on the item.
     */
    if (pNeg)
    {
        if (atoi(randm) < 10) pMod = 0;
        else if (atoi(randm) < 25) pMod = (int) pMod/4;
        else if (atoi(randm) < 50) pMod = (int) pMod/2;
        else if (atoi(randm) < 80) pMod = (int) 4*pMod/5;
    }

    pMod = abs(pMod);

    if (!pNeg && (pObj->points - pMod) < 0)
    {
        send_to_char("You've already added enough positive affects.\n\r", ch);
        return false;
    }

    if (pNeg)
        pObj->points += pMod;
    else
        pObj->points -= pMod;

    pAf             =   new_affect();
    pAf->next	    =   NULL;
    pAf->location   =   value;
    pAf->modifier   =   atoi(mod);
    pAf->where	    =   TO_OBJECT;
    pAf->catalyst_type =   -1;
	pAf->skill		= NULL;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->random	    =   atoi(randm);

    if (!pObj->affected)
	pObj->affected = pAf;
    else
    {
	for (pAf_tmp = pObj->affected; pAf_tmp->next != NULL; pAf_tmp = pAf_tmp->next)
	    ;

        pAf_tmp->next = pAf;
    }

    send_to_char("Affect added.\n\r", ch);
    return true;
}

OEDIT(oedit_addimmune)
{
    long value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf, *pAf_tmp;
    int pMod;
    int where;
    bool pAdd = false;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char randm[MAX_STRING_LENGTH];
    char buf[MSL];

    EDIT_OBJ(ch, pObj);

    argument = one_argument(argument, loc);
    argument = one_argument(argument, mod);
    argument = one_argument(argument, randm);

    if (loc[0] == '\0'
    || mod[0] == '\0'
    || randm[0] == '\0'
    || !is_number(randm))
    {
		send_to_char("Syntax:  addimmune [immune|resist|vuln] [bit] [#rand]\n\r", ch);
		return false;
    }

    where = flag_value(apply_types, loc);

    if( where != TO_IMMUNE && where != TO_RESIST && where != TO_VULN )
    {
		send_to_char("Syntax:  addimmune [immune|resist|vuln] [bit] [#rand]\n\r", ch);
		return false;
	}

	if( where == TO_IMMUNE )
	{
	    if (!str_cmp(pObj->imp_sig, "none") && !IS_IMPLEMENTOR(ch))
	    {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }
	}

	value = flag_value(imm_flags, mod);
	if( value == NO_FLAG || value == 0 )
	{
	    send_to_char("Invalid bit flag\n\r"
			  "Type '? imm' for a list of flags.\n\r", ch);
		return false;
	}

	if ( (value & (~value + 1)) != value )
	{
		send_to_char("You can only put one flag per immunity modifier.\n\r", ch);
		return false;
	}


    for (pAf = pObj->affected; pAf != NULL; pAf = pAf->next)
    {
		if ((pAf->where == TO_IMMUNE || pAf->where == TO_RESIST || pAf->where == TO_VULN) && ((pAf->bitvector & value) != 0))
		{
			sprintf(buf, "There's already an immunity modifier for %s on that item.\n\r",
				flag_string(imm_flags, value));
			send_to_char(buf, ch);
			return false;
		}
    }

    pMod = atoi(randm);

	switch(where)
	{
		case TO_IMMUNE:
			pMod = 5 * pMod / 2;
			break;
		case TO_RESIST:
			break;
		case TO_VULN:
			pAdd = true;
			break;
	}

	pMod = (pMod + 9) / 10;

    if (!pAdd && (pObj->points - pMod) < 0)
    {
        send_to_char("You've already added enough positive affects.\n\r", ch);
        return false;
    }

    if (!pAdd)
        pObj->points -= pMod;
    else
        pObj->points += pMod;

    pAf             =   new_affect();
    pAf->next	    =   NULL;
    pAf->location   =   APPLY_NONE;
    pAf->modifier   =   0;
    pAf->where	    =   where;
    pAf->catalyst_type =   -1;
	pAf->skill = NULL;
    pAf->duration   =   -1;
    pAf->bitvector  =   value;
    pAf->level      =	pObj->level;
    pAf->random	    =   atoi(randm);

    if (!pObj->affected)
	pObj->affected = pAf;
    else
    {
	for (pAf_tmp = pObj->affected; pAf_tmp->next != NULL; pAf_tmp = pAf_tmp->next)
	    ;

        pAf_tmp->next = pAf;
    }

    send_to_char("Immunity modifier added.\n\r", ch);
    return true;
}


OEDIT(oedit_addspell)
{
#if 0
    OBJ_INDEX_DATA *pObj;
    char buf[MSL];
    char name[MSL];
    char level[MSL];
    char rand[MSL];
    SPELL_DATA *spell, *spell_tmp;
    int sn = 0, i;
	WNUM wnum;
	TOKEN_INDEX_DATA *token = NULL;
    bool restricted = true;
    bool spell_restricted = true;

    EDIT_OBJ(ch, pObj);

    if( ch->tot_level == MAX_LEVEL || has_imp_sig(NULL, pObj) )
    	restricted = false;

    if( ch->tot_level == MAX_LEVEL )
    	spell_restricted = false;

    if (restricted &&
    	!(pObj->item_type == ITEM_SCROLL ||
    	pObj->item_type == ITEM_WAND ||
    	pObj->item_type == ITEM_STAFF ||
    	IS_FLUID_CON(pObj) ||
    	pObj->item_type == ITEM_PILL ||
    	pObj->item_type == ITEM_TATTOO ||
    	pObj->item_type == ITEM_PORTAL))
    {
		send_to_char("You can't do this without an IMP's permission.\n\r", ch);
		return false;
    }

    argument = one_argument(argument, name);
    argument = one_argument(argument, level);
    argument = one_argument(argument, rand);

    if (name[0] == '\0' || level[0] == '\0' || rand[0] == '\0'
    ||  !is_number(level) || !is_number(rand))
    {
	send_to_char("Syntax: addspell [spell name/token widevnum] [spell level] [random]\n\r", ch);
	return false;
    }

	if (parse_widevnum(name, NULL, &wnum))
	{
		sn = 0;

		token = get_token_index_wnum(wnum);

		if( !token )
		{
			send_to_char("No such token exists.\n\r", ch);
			return false;
		}

		if (token->type != TOKEN_SPELL)
		{
			send_to_char("That token is not a SPELL token.\n\r", ch);
			return false;
		}
	}
	else if ((sn = skill_lookup(name)) == -1 || (spell_restricted && (skill_table[sn].spell_fun == spell_null)))
    {
		send_to_char("That's not a spell.\n\r", ch);
		return false;
    }

    if (pObj->item_type != ITEM_SCROLL
    &&  pObj->item_type != ITEM_PILL
    &&  pObj->item_type != ITEM_POTION
    &&  pObj->item_type != ITEM_TATTOO
    &&  pObj->item_type != ITEM_STAFF
    &&  pObj->item_type != ITEM_WAND)
    {
	for (spell_tmp = pObj->spells; spell_tmp != NULL; spell_tmp = spell_tmp->next)
	{
		if ((token && spell_tmp->token == token) || (sn > 0 && spell_tmp->sn == sn))
	    {
			send_to_char("That spell is already on the object.\n\r", ch);
			return false;
	    }
	}
    }

    if ((i = atoi(level)) < 1 || i > get_trust(ch))
    {
	sprintf(buf, "Level range is 1-%d.\n\r", get_trust(ch));
	send_to_char(buf, ch);
	return false;
    }

    if ((i = atoi(rand)) < 1 || i > 100)
    {
	send_to_char("Random repop must be a percentage 1-100.\n\r", ch);
	return false;
    }

    spell 		= new_spell();
    spell->sn		= sn;
	spell->token	= token;
    spell->level	= atoi(level);
    spell->repop	= atoi(rand);
    spell->next = NULL;

    // Add to end of list
    if (pObj->spells == NULL)
	pObj->spells = spell;
    else
    {
	for (spell_tmp = pObj->spells; spell_tmp->next != NULL; spell_tmp = spell_tmp->next)
	    ;

        spell_tmp->next = spell;
    }

    sprintf(buf, "Added spell %s, level %d, random %d.\n\r",
        get_spell_data_name(spell), spell->level, spell->repop);
    send_to_char(buf, ch);
    return true;
#else
	send_to_char("ADDSPELL deprecated.\n\r", ch);
	return false;
#endif
}

OEDIT(oedit_addskill)
{
    OBJ_INDEX_DATA *pObj;
    char buf[MSL];
    char name[MSL];
    char mod[MSL];
    char random[MSL];
    AFFECT_DATA *pAf, *pAf_tmp;
	SKILL_DATA *skill;
    int i;

    EDIT_OBJ(ch, pObj);

    if (!IS_IMPLEMENTOR(ch) && !has_imp_sig(NULL, pObj))
    {
	send_to_char("You can't do this without an IMP's permission.\n\r", ch);
	return false;
    }

    argument = one_argument(argument, name);
    argument = one_argument(argument, mod);
    argument = one_argument(argument, random);

    if (name[0] == '\0' || mod[0] == '\0' || random[0] == '\0' ||  !is_number(mod) || !is_number(random))
    {
	send_to_char("Syntax: addskill [skill name] [#modifier] [random]\n\r", ch);
	return false;
    }

	skill = get_skill_data(name);
    if (!IS_VALID(skill))
    {
	send_to_char("That's not a skill.\n\r", ch);
	return false;
    }

    if ((i = atoi(mod)) < -100 || i > 100 || !i)
    {
	send_to_char("Skill modifier must be a positive (1 to 100) or negative (-1 to -100) percentage.\n\r", ch);
	return false;
    }

    if ((i = atoi(random)) < 1 || i > 100)
    {
	send_to_char("Random repop must be a percentage 1-100.\n\r", ch);
	return false;
    }

    pAf             =   new_affect();
    pAf->next	    =   NULL;
    pAf->location   =   APPLY_SKILL+skill->uid;
    pAf->modifier   =   atoi(mod);
    pAf->where	    =   TO_OBJECT;
    pAf->catalyst_type       =   -1;
	pAf->skill		= NULL;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->random	    =   atoi(random);

    if (!pObj->affected)
	pObj->affected = pAf;
    else
    {
	for (pAf_tmp = pObj->affected; pAf_tmp->next != NULL; pAf_tmp = pAf_tmp->next)
	    ;

        pAf_tmp->next = pAf;
    }

    sprintf(buf, "Added skill %s, percent mod %d%%, random %d.\n\r",
        skill->name, pAf->modifier, pAf->random);
    send_to_char(buf, ch);
    return true;
}


OEDIT(oedit_addcatalyst)
{
    OBJ_INDEX_DATA *pObj;
    char type[MSL];
    char charges[MSL];
    char chance[MIL];
    char where[MIL];
    AFFECT_DATA *cat, *pCat;
    int t, c, n, w;

    EDIT_OBJ(ch, pObj);

    if (!IS_IMPLEMENTOR(ch) && !has_imp_sig(NULL, pObj))
    {
	send_to_char("You can't do this without an IMP's permission.\n\r", ch);
	return false;
    }

    argument = one_argument(argument, type);
    argument = one_argument(argument, charges);
    argument = one_argument(argument, chance);
    argument = one_argument(argument, where);

    if (!type[0] || !charges[0] || !chance[0]
    || (!is_number(charges) && str_prefix(charges,"source")) || !is_number(chance))
    {
	send_to_char("Syntax: addcatalyst [type] [charges] [chance] [active] [name]\n\r", ch);
	return false;
    }

    if ((t = flag_value(catalyst_types,type)) == NO_FLAG)
    {
	send_to_char("That's not a catalyst type.\n\r", ch);
	return false;
    }

    c = atoi(chance);
    w = (where[0] && !str_cmp(where, "active")) ? TO_CATALYST_ACTIVE : TO_CATALYST_DORMANT;

	if(!str_prefix(charges,"source"))
		n = -1;
	else if ((n = atoi(charges)) < 1) {
		send_to_char("Invalid charges.\n\r", ch);
		return false;
	}

	c = URANGE(1,c,100);

    for(cat = pObj->catalyst; cat; cat = cat->next) {
	    if(cat->where == w && cat->catalyst_type == t && cat->random == c) {
		    if(cat->modifier < 0 || n < 0)
			    cat->modifier = -1;
		    else
			    cat->modifier += n;
		    break;
	    }
    }

	if(!cat) {
		pCat = new_affect();
		pCat->next = NULL;
		pCat->where = w;
		pCat->modifier = n;
		pCat->catalyst_type = t;
		pCat->random = c;

		if( !IS_NULLSTR(argument) )
			pCat->custom_name = str_dup(argument);

		// Add to end of list
		if (!pObj->catalyst)
			pObj->catalyst = pCat;
		else {
			for (cat = pObj->catalyst; cat->next != NULL; cat = cat->next);
			cat->next = pCat;
		}
	}

    send_to_char("Added catalyst.\n\r", ch);
    return true;
}


OEDIT(oedit_delspell)
{
    OBJ_INDEX_DATA *pObj;
    SPELL_DATA *spell, *spell_prev;
    int i, n;

    EDIT_OBJ(ch, pObj);

    if (!is_number(argument))
    {
	send_to_char("Syntax: delspell [#]\n\r", ch);
	return false;
    }

    n = atoi(argument);
    i = 0;
    spell_prev = NULL;
    for (spell = pObj->spells; spell != NULL; spell = spell->next)
    {
	if (i == n)
	    break;

	i++;
	spell_prev = spell;
    }

    if (spell == NULL)
    {
	send_to_char("That spell isn't on the object.\n\r", ch);
	return false;
    }

    // First one on the list
    if (!spell_prev)
    {
	pObj->spells = spell->next;
	free_spell(spell);
    }
    else
    {
	spell_prev->next = spell->next;
	free_spell(spell);
    }

    send_to_char("Spell removed.\n\r", ch);
    return true;
}


OEDIT(oedit_delcatalyst)
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *catalyst, *catalyst_prev;
    int i, n;

    EDIT_OBJ(ch, pObj);

    if (!is_number(argument))
    {
	send_to_char("Syntax: delcatalyst [#]\n\r", ch);
	return false;
    }

    n = atoi(argument);
    i = 0;
    catalyst_prev = NULL;
    for (catalyst = pObj->catalyst; catalyst != NULL; catalyst = catalyst->next)
    {
	if (i == n)
	    break;

	i++;
	catalyst_prev = catalyst;
    }

    if (catalyst == NULL)
    {
	send_to_char("That catalyst isn't on the object.\n\r", ch);
	return false;
    }

    // First one on the list
    if (!catalyst_prev)
    {
	pObj->catalyst = catalyst->next;
	free_affect(catalyst);
    }
    else
    {
	catalyst_prev->next = catalyst->next;
	free_affect(catalyst);
    }

    send_to_char("Catalyst removed.\n\r", ch);
    return true;
}


OEDIT(oedit_next)
{
    OBJ_INDEX_DATA *pObj;
    OBJ_INDEX_DATA *nextObj = NULL;
    long next_vnum;

    EDIT_OBJ(ch, pObj);

    next_vnum = pObj->vnum;

    next_vnum++;
    while (nextObj == NULL && next_vnum > 0)
    {
		nextObj = get_obj_index(pObj->area, next_vnum);
		next_vnum++;
    }

    if (nextObj == NULL)
    {
	send_to_char("No next object in area.\n\r", ch);
    }
    else
    {
	edit_done(ch);

	olc_set_editor(ch, ED_OBJECT, nextObj);
    }
    return false;
}

OEDIT(oedit_lock)
{
	char arg[MIL];
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  lock add\n\r", ch);
		send_to_char("         lock remove\n\r", ch);
		send_to_char("         lock key [wnum]\n\r", ch);
		send_to_char("         lock key clear\n\r", ch);
		send_to_char("         lock flags [flags]\n\r", ch);
		send_to_char("         lock pick [0-100]\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);

	if( !str_prefix(arg, "add") )
	{
		if( pObj->lock )
		{
			send_to_char("Object already has a lock state.\n\r", ch);
			return false;
		}

		// TODO: Add closeability to weapon_containers and drinkcontainers
		if( pObj->item_type != ITEM_CONTAINER &&
			pObj->item_type != ITEM_PORTAL &&
//			pObj->item_type != ITEM_WEAPON_CONTAINER &&
//			pObj->item_type != ITEM_DRINKCONTAINER &&
			pObj->item_type != ITEM_BOOK )
		{
			send_to_char("Invalid object type.\n\r", ch);
			send_to_char("Only the following types may have lock state added:\n\r", ch);
			send_to_char("{Y*{x CONTAINER\n\r", ch);
			send_to_char("{Y*{x PORTAL\n\r", ch);
//			send_to_char("{Y*{x WEAPON_CONTAINER\n\r", ch);
//			send_to_char("{Y*{x DRINKCONTAINER\n\r", ch);
			send_to_char("{Y*{x BOOK\n\r", ch);
			return false;
		}

		pObj->lock = new_lock_state();
		send_to_char("Lock State added.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "remove") )
	{
		if( !pObj->lock )
		{
			send_to_char("Object does not have a lock state.\n\r", ch);
			return false;
		}


		free_lock_state(pObj->lock);
		pObj->lock = NULL;

		send_to_char("Lock State removed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "key") )
	{
		if( !pObj->lock )
		{
			send_to_char("Object does not have a lock state.\n\r", ch);
			return false;
		}

		if( argument[0] == '\0' )
		{
			send_to_char("Syntax:  lock key [vnum]\n\r", ch);
			send_to_char("         lock key clear\n\r", ch);
			return false;
		}

		WNUM wnum;
		if( parse_widevnum(argument, ch->in_room->area, &wnum) )
		{
			OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

			if( !key )
			{
				send_to_char("That object does not exist.\n\r", ch);
				return false;
			}

			if( key->item_type != ITEM_KEY )
			{
				send_to_char("That object is not a key.\n\r", ch);
				return false;
			}

			// TODO: make a list
			pObj->lock->key_wnum = wnum;
			send_to_char("Lock State key set.\n\r", ch);
			return true;
		}
		else if( !str_prefix(argument, "clear") )
		{
			pObj->lock->key_wnum = wnum_zero;
			send_to_char("Lock State key removed.\n\r", ch);
			return true;
		}

		oedit_lock(ch, "lock key");
		return false;
	}

	if( !str_prefix(arg, "flags") )
	{
		if( !pObj->lock )
		{
			send_to_char("Object does not have a lock state.\n\r", ch);
			return false;
		}

		int value = flag_value(lock_flags, argument);

		if( value == NO_FLAG )
		{
			send_to_char("Syntax:  lock flags [flags]\n\r", ch);
			send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
			show_help(ch, "lock");
			return false;
		}

		pObj->lock->flags ^= value;
		send_to_char("Lock State flags changed.\n\r", ch);
		return true;
	}

	if( !str_prefix(arg, "pick") )
	{
		if( !pObj->lock )
		{
			send_to_char("Object does not have a lock state.\n\r", ch);
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
			send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
			return false;
		}

		pObj->lock->pick_chance = value;
		send_to_char("Lock State pick chance set.\n\r", ch);
		return true;
	}

	oedit_lock(ch, "");
	return false;
}

OEDIT(oedit_persist)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);


	if (!str_cmp(argument,"on")) {
	    if (!str_cmp(pObj->imp_sig, "none") && !IS_IMPLEMENTOR(ch)) {
			send_to_char("You can't do this without an IMP's permission.\n\r", ch);
			return false;
	    }

		pObj->persist = true;
	    use_imp_sig(NULL, pObj);
		send_to_char("Persistance enabled.\n\r", ch);
	} else if (!str_cmp(argument,"off")) {
		pObj->persist = false;
		send_to_char("Persistance disabled.\n\r", ch);
	} else {
		send_to_char("Usage: persist on/off\n\r", ch);
		return false;
	}

	return true;
}

OEDIT(oedit_prev)
{
    OBJ_INDEX_DATA *pObj;
    OBJ_INDEX_DATA *prevObj = NULL;
    long prev_vnum;

    EDIT_OBJ(ch, pObj);

    prev_vnum = pObj->vnum;

    prev_vnum--;
    while (prevObj == NULL && prev_vnum > 0)
    {
		prevObj = get_obj_index(pObj->area, prev_vnum);
		prev_vnum--;
    }

    if (prevObj == NULL)
    {
	send_to_char("No previous object in area.\n\r", ch);
    }
    else
    {
	edit_done(ch);
	olc_set_editor(ch, ED_OBJECT, prevObj);
    }
    return false;
}


OEDIT(oedit_delaffect)
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_prev;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    //int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument(argument, affect);

    if (!is_number(affect) || affect[0] == '\0')
    {
	send_to_char("Syntax:  delaffect [#xaffect]\n\r", ch);
	return false;
    }

    value = atoi(affect);

    if (value < 0)
    {
	send_to_char("Only non-negative affect-numbers allowed.\n\r", ch);
	return false;
    }

    if (!(pAf = pObj->affected))
    {
	send_to_char("OEdit:  Non-existant affect.\n\r", ch);
	return false;
    }

	pAf_prev = NULL;
    for(;pAf;pAf_prev = pAf, pAf = pAf_next)
    {
		pAf_next = pAf->next;

		if( pAf->where == TO_OBJECT )
		{
			if( --value < 0 )
			{
				if( pAf_prev == NULL )
					pObj->affected = pAf_next;
				else
					pAf_prev->next = pAf_next;

				free_affect(pAf);
				send_to_char("Affect removed.\n\r", ch);
				return true;
			}
		}

	}

	send_to_char("No such affect.\n\r", ch);
	return false;
}

OEDIT(oedit_delimmune)
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_prev;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    //int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument(argument, affect);

    if (!is_number(affect) || affect[0] == '\0')
    {
	send_to_char("Syntax:  delimmune [#xaffect]\n\r", ch);
	return false;
    }

    value = atoi(affect);

    if (value < 0)
    {
	send_to_char("Only non-negative affect-numbers allowed.\n\r", ch);
	return false;
    }

    if (!(pAf = pObj->affected))
    {
	send_to_char("OEdit:  Non-existant affect.\n\r", ch);
	return false;
    }

	pAf_prev = NULL;
    for(;pAf;pAf_prev = pAf, pAf = pAf_next)
    {
		pAf_next = pAf->next;

		if( pAf->where == TO_IMMUNE || pAf->where == TO_RESIST || pAf->where == TO_VULN )
		{
			if( --value < 0 )
			{
				if( pAf_prev == NULL )
					pObj->affected = pAf_next;
				else
					pAf_prev->next = pAf_next;

				free_affect(pAf);
				send_to_char("Immunity modifier removed.\n\r", ch);
				return true;
			}
		}

	}

	send_to_char("No such immunity modifier.\n\r", ch);
	return false;
}

OEDIT(oedit_name)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  name [string]\n\r", ch);
	return false;
    }

    free_string(pObj->name);
    pObj->name = str_dup(argument);

    send_to_char("Name set.\n\r", ch);
    return true;
}


OEDIT(oedit_sign)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (!IS_IMPLEMENTOR(ch))
    {
	send_to_char("This is not for you to do.\n\r" , ch);
	return false;
    }

    free_string(pObj->imp_sig);
    pObj->imp_sig = str_dup(ch->name);

    send_to_char("Object signed.\n\r", ch);
    return true;
}

OEDIT(oedit_skeywds)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  skwds [string]\n\r", ch);
	return false;
    }

    free_string(pObj->skeywds);
    pObj->skeywds = str_dup(argument);

    send_to_char("Script keywords set.\n\r", ch);
    return true;
}

OEDIT(oedit_varset)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

	return olc_varset(&pObj->index_vars, ch, argument, false);
}

OEDIT(oedit_varclear)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

	return olc_varclear(&pObj->index_vars, ch, argument, false);
}


OEDIT(oedit_short)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  short [string]\n\r", ch);
	return false;
    }

    free_string(pObj->short_descr);
    pObj->short_descr = str_dup(argument);

    send_to_char("Short description set.\n\r", ch);

    if (IS_SET(ch->act[0], PLR_AUTOSETNAME))
    {
	free_string(pObj->name);
	pObj->name = short_to_name(pObj->short_descr);
	send_to_char("Name keywords set.\n\r", ch);
    }
    return true;
}


OEDIT(oedit_long)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  long [string]\n\r", ch);
	return false;
    }

    strcat(argument, "{x");

    free_string(pObj->description);
    pObj->description = str_dup(argument);
    pObj->description[0] = UPPER(pObj->description[0]);

    send_to_char("Long description set.\n\r", ch);
    return true;
}


bool set_value(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value)
{
    if (argument[0] == '\0')
    {
	set_obj_values(ch, pObj, -1, "");
	return false;
    }

    if (set_obj_values(ch, pObj, value, argument))
	return true;

    return false;
}


/* Finds the object and sets its value. */
bool oedit_values(CHAR_DATA *ch, char *argument, int value)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (set_value(ch, pObj, argument, value))
    {
	if (pObj->item_type == ITEM_WEAPON)
	    set_weapon_dice(pObj);

        return true;
    }

    return false;
}


OEDIT(oedit_value0)
{
    if (oedit_values(ch, argument, 0))
        return true;

    return false;
}


OEDIT(oedit_value1)
{
    if (oedit_values(ch, argument, 1))
        return true;

    return false;
}


OEDIT(oedit_value2)
{
    if (oedit_values(ch, argument, 2))
        return true;

    return false;
}


OEDIT(oedit_value3)
{
    if (oedit_values(ch, argument, 3))
        return true;

    return false;
}


OEDIT(oedit_value4)
{
    if (oedit_values(ch, argument, 4))
        return true;

    return false;
}


OEDIT(oedit_value5)
{
    if (oedit_values(ch, argument, 5))
        return true;

    return false;
}


OEDIT(oedit_value6)
{
    if (oedit_values(ch, argument, 6))
        return true;

    return false;
}


OEDIT(oedit_value7)
{
    if (oedit_values(ch, argument, 7))
        return true;

    return false;
}


OEDIT(oedit_weight)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  weight [number]\n\r", ch);
	return false;
    }

    pObj->weight = atoi(argument);

    send_to_char("Weight set.\n\r", ch);
    return true;
}


OEDIT(oedit_cost)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  cost [number]\n\r", ch);
	return false;
    }

    pObj->cost = atoi(argument);

    send_to_char("Cost set.\n\r", ch);
    return true;
}


OEDIT(oedit_create)
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea = ch->in_room->area;
    long auto_vnum = 0;
    int  iHash;
	WNUM wnum;

    if (argument[0] == '\0' || !parse_widevnum(argument, ch->in_room->area, &wnum) || !wnum.pArea || wnum.vnum < 1 )
    {
		//send_to_char("Syntax:  oedit create [vnum]\n\r", ch);
		for( auto_vnum = 1; auto_vnum > 0 && get_obj_index(pArea, auto_vnum); auto_vnum++);

		if (auto_vnum < 1)
		{
			send_to_char("Sorry, this area has no more space left.\n\r", ch);
			return false;
		}

		wnum.pArea = pArea;
		wnum.vnum = auto_vnum;
    }

    if (!IS_BUILDER(ch, wnum.pArea))
    {
	send_to_char("OEdit:  Vnum in an area you cannot build in.\n\r", ch);
	return false;
    }

    if (get_obj_index(wnum.pArea, wnum.vnum))
    {
	send_to_char("OEdit:  Object already exists.\n\r", ch);
	return false;
    }

    pObj              = new_obj_index();
    pObj->vnum	      = wnum.vnum;
    pObj->area	      = wnum.pArea;
    pObj->creator_sig = str_dup(ch->name);

	wnum.pArea->bottom_vnum_obj = UMIN(wnum.pArea->bottom_vnum_obj, wnum.vnum);
	wnum.pArea->top_vnum_obj = UMAX(wnum.pArea->top_vnum_obj, wnum.vnum);

    iHash                 = wnum.vnum % MAX_KEY_HASH;
    pObj->next		  = wnum.pArea->obj_index_hash[iHash];
    wnum.pArea->obj_index_hash[iHash] = pObj;
	olc_set_editor(ch, ED_OBJECT, pObj);

    SET_BIT(pObj->area->area_flags, AREA_CHANGED);
    send_to_char("Object Created.\n\r", ch);
    return true;
}


OEDIT(oedit_ed)
{
	OBJ_INDEX_DATA *pObj;
	EXTRA_DESCR_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char copy_item[MAX_INPUT_LENGTH];
	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, command);
	argument = one_argument(argument, keyword);
	argument = one_argument(argument, copy_item);

	if (command[0] == '\0')
	{
		send_to_char("Syntax:  ed add [keyword]\n\r", ch);
		send_to_char("         ed delete [keyword]\n\r", ch);
		send_to_char("         ed edit [keyword]\n\r", ch);
		send_to_char("         ed format [keyword]\n\r", ch);
		send_to_char("         ed copy old_keyword new_keyword\n\r", ch);
		send_to_char("         ed environment [keyword]\n\r", ch);

		return false;
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
	ed->next		=   pObj->extra_descr;
	pObj->extra_descr	=   ed;

	send_to_char("Enviromental extra description added.\n\r", ch);

	return true;
    }

	if (!str_cmp(command, "copy"))
	{
		EXTRA_DESCR_DATA *ed2;

		if (keyword[0] == '\0' || copy_item[0] == '\0')
		{
			send_to_char("Syntax:  ed copy existing_keyword new_keyword\n\r", ch);
			return false;
		}

		for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
			if (is_name(keyword, ed->keyword))
			break;
		}

		if (!ed)
		{
			send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
			return false;
		}

		ed2					= new_extra_descr();
		ed2->keyword		= str_dup(copy_item);
		ed2->next			= pObj->extra_descr;
		pObj->extra_descr	= ed2;
		ed2->description	= str_dup(ed->description);

		send_to_char("Done.\n\r", ch);

		return true;
	}

	if (!str_cmp(command, "add"))
	{
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed add [keyword]\n\r", ch);
			return false;
		}

		ed					= new_extra_descr();
		ed->keyword			= str_dup(keyword);
		ed->next			= pObj->extra_descr;
		pObj->extra_descr	= ed;

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

		for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
			if (is_name(keyword, ed->keyword))
			break;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
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

		for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
			if (is_name(keyword, ed->keyword))
				break;
			ped = ed;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
			return false;
		}

		if (!ped)
			pObj->extra_descr = ed->next;
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

		for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
			if (is_name(keyword, ed->keyword))
				break;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
			return false;
		}

		if( !ed->description )
		{
			send_to_char("OEdit:  Extra description is an environmental extra description.\n\r", ch);
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

		for (ed = pObj->extra_descr; ed; ed = ed->next)
		{
			if (is_name(keyword, ed->keyword))
				break;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
			return false;
		}

		if (!ed->description)
		{
			send_to_char("OEdit:  Cannot show environmental extra description.\n\r", ch);
			return false;
		}

		page_to_char(ed->description, ch);

		return true;
	}

	oedit_ed(ch, "");
	return false;
}


OEDIT(oedit_extra)
{
    OBJ_INDEX_DATA *pObj;

    if (argument[0] != '\0')
    {
		EDIT_OBJ(ch, pObj);
		
		long extra[4];

		if (!bitvector_lookup(argument, 4, extra, extra_flags, extra2_flags, extra3_flags, extra4_flags))
		{
			send_to_char("Invalid extra flag.\n\r", ch);
			send_to_char("Type '? extra' for a list of flags.\n\r", ch);
			return false;
		}

		TOGGLE_BIT(pObj->extra[0], extra[0]);
		TOGGLE_BIT(pObj->extra[1], extra[1]);
		TOGGLE_BIT(pObj->extra[2], extra[2]);
		TOGGLE_BIT(pObj->extra[3], extra[3]);

	    send_to_char("Extra flag toggled.\n\r", ch);
	    return true;
    }

    send_to_char("Syntax:  extra [flag]\n\r"
		  "Type '? extra' for a list of flags.\n\r", ch);
    return false;
}

OEDIT(oedit_wear)
{
    OBJ_INDEX_DATA *pObj;
    int value, wear;

    if (argument[0] != '\0')
    {
	EDIT_OBJ(ch, pObj);

	value = flag_value(wear_flags, argument);

	wear = (pObj->wear_flags ^ value) & ~(ITEM_TAKE|ITEM_CONCEALS|ITEM_NO_SAC);

	if((wear & -wear) != wear) {
		send_to_char("You can't set an object to be worn in more than one spot at once.\n\r", ch);
		return false;
	}

/*
        if ((flag_value(wear_flags, argument) == ITEM_WEAR_BACK)
	&&     pObj->item_type != ITEM_RANGED_WEAPON)
	{
	    send_to_char("Only ranged weapons can be slung behind the back.\n\r", ch);
	    return false;
	}

	if ((flag_value(wear_flags, argument) == ITEM_WEAR_SHOULDER)
	&&     pObj->item_type != ITEM_WEAPON_CONTAINER)
	{
	    send_to_char("Only weapon containers can be worn on the shoulder.\n\r", ch);
	    return false;
	}
*/
	if ((value = flag_value(wear_flags, argument)) != NO_FLAG)
	{
	    TOGGLE_BIT(pObj->wear_flags, value);

	    send_to_char("Wear flag toggled.\n\r", ch);

	    return true;
	}
    }

    send_to_char("Syntax:  wear [flag]\n\r"
		  "Type '? wear' for a list of flags.\n\r", ch);
    return false;
}

OEDIT(oedit_visibility)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

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
			send_to_char("No such object script with that widevnum.\n\r", ch);
			return false;
		}

		pObj->script_visible = script;
		send_to_char("Visibility Script changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		pObj->script_visible = NULL;
		send_to_char("Visibility Script cleared.\n\r", ch);
		return true;
	}

	oedit_visibility(ch, "");
	return false;
}

OEDIT(oedit_type)
{
	OBJ_INDEX_DATA *pObj;
	int value;
	int i;

	if (argument[0] != '\0')
	{
		EDIT_OBJ(ch, pObj);

		if ((value = flag_value(type_flags, argument)) != NO_FLAG)
		{

			if ((value == ITEM_BANK ||
				 value == ITEM_SHARECERT ||
				 value == ITEM_ROOM_DARKNESS ||
				 //value == ITEM_ROOM_FLAME ||
				 value == ITEM_SMOKE_BOMB ||
				 value == ITEM_MONEY ||
				 //value == ITEM_WITHERING_CLOUD ||
				 value == ITEM_ROOM_ROOMSHIELD ||
				 value == ITEM_CATALYST ||
				 value == ITEM_SHRINE) &&
				!IS_IMPLEMENTOR(ch))
			{
				send_to_char("Sorry, only an IMP can set that item-type.\n\r",ch);
				return false;
			}

			obj_index_set_primarytype(pObj, value);

			// Clear the values.
			for (i = 0; i < MAX_OBJVALUES; i++)
			{
				pObj->value[i] = 0;
			}

			/*
			// Defaults
			if( pObj->item_type == ITEM_TELESCOPE )
			{
				pObj->value[4] = -1;
			}
			*/

			if( pObj->lock )
			{
				free_lock_state(pObj->lock);
				pObj->lock = NULL;
			}

			send_to_char("Primary type set.\n\r", ch);
			return true;
		}
	}

	send_to_char("Syntax:  type [flag]\n\r"
				"Type '? type' for a list of flags.\n\r", ch);
	return false;
}


OEDIT( oedit_type_ammo )
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_AMMO(pObj))
		{
			send_to_char("Syntax:  ammo type <type>\n\r", ch);
			send_to_char("         ammo dice <number> <size>[ <bonus>]\n\r", ch);

			if (pObj->item_type != ITEM_AMMO)
			{
				send_to_char("         ammo remove\n\r", ch);
			}
		}
		else
		{
			send_to_char("Syntax:  ammo add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (IS_AMMO(pObj))
	{
		if (!str_prefix(arg, "dice"))
		{
			int number;
			argument = one_argument(argument, arg);
			if (!is_number(arg) || (number = atoi(arg)) < 1)
			{
				send_to_char("Please specify a positive number of dice.\n\r", ch);
				return false;
			}

			int size;
			argument = one_argument(argument, arg);
			if (!is_number(arg) || (size = atoi(arg)) < 1)
			{
				send_to_char("Please specify a positive dice size.\n\r", ch);
				return false;
			}

			int bonus = 0;
			if (argument[0] != '\0')
			{
				if (!is_number(argument) || (bonus = atoi(argument)) < 1)
				{
					send_to_char("Please specify a positive dice bonus.\n\r", ch);
					return false;
				}
			}

			AMMO(pObj)->damage.number = number;
			AMMO(pObj)->damage.size = size;
			AMMO(pObj)->damage.bonus = bonus;
			send_to_char("AMMO Damage dice changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(weapon_type2, argument)) == NO_FLAG)
			{
				send_to_char("Invalid attack flag.  Use '? wtype' for list of valid flags.\n\r", ch);
				show_flag_cmds(ch, weapon_type2);
				return false;
			}

			TOGGLE_BIT(AMMO(pObj)->flags, value);
			send_to_char("AMMO Attack flags toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "remove"))
		{
			if (pObj->item_type != ITEM_AMMO)
			{
				
				free_ammo_data(AMMO(pObj));
				AMMO(pObj) = NULL;
				send_to_char("AMMO data removed from object.\n\r", ch);
				return true;
			}
		}

		if (!str_prefix(arg, "type"))
		{
			int type;
			if ((type = stat_lookup(argument, ammo_types, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid ammo type.  Use '? ammo' for list of valid types.\n\r", ch);
				show_flag_cmds(ch, ammo_types);
				return false;
			}

			AMMO(pObj)->type = type;
			send_to_char("AMMO type changed.\n\r", ch);
			return true;
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_AMMO))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			AMMO(pObj) = new_ammo_data();
			send_to_char("AMMO data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_ammo(ch, "");
	return false;
}


bool olc_can_equip_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_EQUIP, TRIGSLOT_SPELL) != NULL;
	else
		return skill->equip_fun != NULL;
}

OEDIT( oedit_type_armor )
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_ARMOR(pObj))
		{
			send_to_char("Syntax:  armor type <type>\n\r", ch);
			send_to_char("         armor strength <strength>\n\r", ch);
			send_to_char("         armor protection <type> <#rating>\n\r", ch);
			send_to_char(formatf("         armor maxadornments <0-%d>\n\r", MAX_ADORNMENTS), ch);

			send_to_char("         armor adornment add <spell> <level>\n\r", ch);
			send_to_char("         armor adornment remove <#>\n\r", ch);
			send_to_char("         armor adornment clear\n\r", ch);
			send_to_char("         armor adornment <#> name <name>\n\r", ch);
			send_to_char("         armor adornment <#> short <string>\n\r", ch);
			send_to_char("         armor adornment <#> description    - Opens string editor\n\r", ch);
			send_to_char("         armor adornment <#> spell <spell>\n\r", ch);
			send_to_char("         armor adornment <#> level <leve>\n\r", ch);

			if (pObj->item_type != ITEM_ARMOUR)
			{
				send_to_char("         armor remove\n\r", ch);
			}
		}
		else
		{
			send_to_char("Syntax:  armor add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (IS_ARMOR(pObj))
	{
		if (!str_prefix(arg, "type"))
		{
			int type;
			if ((type = stat_lookup(argument, armour_types, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid armor type.  Use '? armour' for valid types.\n\r", ch);
				show_flag_cmds(ch, armour_types);
				return false;
			}

			ARMOR(pObj)->armor_type = type;
			set_armour(pObj);

			send_to_char("ARMOR Type changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "strength"))
		{
			int strength;
			if ((strength = stat_lookup(argument, armour_strength_table, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid armor strength. Use '? armourstrength' for valid strengths.\n\r", ch);
				show_flag_cmds(ch, armour_strength_table);
				return false;
			}

			ARMOR(pObj)->armor_strength = strength;
			set_armour(pObj);

			send_to_char("ARMOR Strength changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "protection"))
		{
			argument = one_argument(argument, arg);
			int type;
			if ((type = stat_lookup(arg, armour_protection_types, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid protection type.  Use '? protections' for valid list of types.\n\r", ch);
				show_flag_cmds(ch, armour_protection_types);
				return false;
			}

			int armour;
			if (!is_number(argument) || (armour = atoi(argument)) < 0)
			{
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			ARMOR(pObj)->protection[type] = armour;
			send_to_char(formatf("ARMOR {+%s Rating changed.\n\r", flag_string(armour_protection_types, type)), ch);
			return true;
		}

		if (!str_prefix(arg, "maxadornments"))
		{
			int max;
			if (!is_number(argument) || (max = atoi(argument)) < 0 || max > MAX_ADORNMENTS)
			{
				send_to_char(formatf("Please provide a number from 0 to %d.\n\r", MAX_ADORNMENTS), ch);
				return false;
			}

			if (ARMOR(pObj)->max_adornments > 0 && ARMOR(pObj)->adornments != NULL)
			{
				for(int i = ARMOR(pObj)->max_adornments; i-- > 0; )
				{
					ADORNMENT_DATA *adorn = ARMOR(pObj)->adornments[i];
					if (IS_VALID(adorn))
						free_adornment_data(adorn);
				}
				free_mem(ARMOR(pObj)->adornments, sizeof(ADORNMENT_DATA *) * ARMOR(pObj)->max_adornments);
				ARMOR(pObj)->adornments = NULL;
			}

			ARMOR(pObj)->max_adornments = max;
			if (max > 0)
			{
				ARMOR(pObj)->adornments = alloc_mem(sizeof(ADORNMENT_DATA *) * max);
				for(int i = 0; i < max; i++)
					ARMOR(pObj)->adornments[i] = NULL;
			}

			send_to_char("ARMOR Max Adornments changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "adornment"))
		{
			if (ARMOR(pObj)->max_adornments < 1 || ARMOR(pObj)->adornments == NULL)
			{
				send_to_char("Armor does not allow adornments.\n\r", ch);
				return false;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  armor adornment add <spell> <level>\n\r", ch);
				send_to_char("         armor adornment remove <#>\n\r", ch);
				send_to_char("         armor adornment clear\n\r", ch);
				send_to_char("         armor adornment <#> name <name>\n\r", ch);
				send_to_char("         armor adornment <#> short <string>\n\r", ch);
				send_to_char("         armor adornment <#> description    - Opens string editor\n\r", ch);
				send_to_char("         armor adornment <#> spell <spell>\n\r", ch);
				send_to_char("         armor adornment <#> level <leve>\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "add"))
			{
				int new_index;
				for(new_index = 0; new_index < ARMOR(pObj)->max_adornments; new_index++)
				{
					ADORNMENT_DATA *adorn = ARMOR(pObj)->adornments[new_index];
					if (!IS_VALID(adorn))
						break;
				}

				if (new_index < 0)
				{
					send_to_char("You cannot add any more adornments.\n\r", ch);
					return false;
				}

				int type;
				switch(ARMOR(pObj)->armor_type)
				{
					case ARMOR_TYPE_CLOTH:
						type = ADORNMENT_EMBROIDERY;
						break;

					case ARMOR_TYPE_LEATHER:
						type = ADORNMENT_RUNE;
						break;

					case ARMOR_TYPE_MAIL:
					case ARMOR_TYPE_PLATE:
						type = ADORNMENT_GEM;
						break;

					default:
						send_to_char("ARMOR Type does not allow adornments.\n\r", ch);
						return false;
				}

				argument = one_argument(argument, arg);
				SKILL_DATA *skill;
				if (IS_NULLSTR(arg))
				{
					send_to_char("Please specify a spell name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(arg);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_equip_spell(skill))
					{
						send_to_char("That spell cannot be equipped.\n\r", ch);
						return false;
					}
				}

				int level;
				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					send_to_char(formatf("Level range is 1-%d.\n\r", MAX_CLASS_LEVEL), ch);
					return false;
				}

				// Spells must be unique on adornments on the armor
				for(int i = ARMOR(pObj)->max_adornments; i-- > 0;)
				{
					ADORNMENT_DATA *adorn = ARMOR(pObj)->adornments[i];
					if (IS_VALID(adorn) && adorn->spell != NULL)
					{
						if (adorn->spell->skill == skill)
						{
							send_to_char("Already have an adornment with that spell.\n\r", ch);
							return false;
						}
					}
				}

				ADORNMENT_DATA *adornment = new_adornment_data();

				SPELL_DATA *spell = new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				adornment->type = type;
				adornment->name = &str_empty[0];
				adornment->short_descr = &str_empty[0];
				adornment->description = &str_empty[0];
				adornment->spell = spell;

				ARMOR(pObj)->adornments[new_index] = adornment;

				send_to_char("Adornment added.\n\r", ch);
				return true;
			}
			
			if (!str_prefix(arg, "remove"))
			{
				int index;
				if (!is_number(argument) || (index = atoi(argument)) < 1 || index > ARMOR(pObj)->max_adornments)
				{
					send_to_char(formatf("Please provide a number from 1 to %d.\n\r", ARMOR(pObj)->max_adornments), ch);
					return false;
				}
				ADORNMENT_DATA *adorn = ARMOR(pObj)->adornments[index - 1];
				if (!IS_VALID(adorn))
				{
					send_to_char("No adornment at that slot.\n\r", ch);
					return false;
				}

				free_adornment_data(adorn);
				ARMOR(pObj)->adornments[index - 1] = NULL;
				send_to_char(formatf("ARMOR Adornment #%d removed.\n\r", index), ch);
				return true;
			}
			
			if (!str_prefix(arg, "clear"))
			{
				for(int i = ARMOR(pObj)->max_adornments; i-- > 0; )
				{
					free_adornment_data(ARMOR(pObj)->adornments[i]);
					ARMOR(pObj)->adornments[i] = NULL;
				}

				send_to_char("ARMOR Adornments cleared.\n\r", ch);
				return true;
			}

			int index;
			if (!is_number(arg) || (index = atoi(arg)) < 1 || index > ARMOR(pObj)->max_adornments)
			{
				send_to_char(formatf("Please provide an index from 1 to %d.\n\r", ARMOR(pObj)->max_adornments), ch);
				return false;
			}

			ADORNMENT_DATA *adornment = ARMOR(pObj)->adornments[index - 1];
			if (!IS_VALID(adornment))
			{
				send_to_char("No such adornment at that slot.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "name"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Please provide a name.\n\r", ch);
					return false;
				}

				smash_tilde(argument);
				free_string(adornment->name);
				adornment->name = str_dup(argument);
				send_to_char("ARMOR Adornment Name changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg, "short"))
			{
				if (argument[0] == '\0')
				{
					send_to_char("Please provide a short descriptiong.\n\r", ch);
					return false;
				}

				smash_tilde(argument);
				free_string(adornment->short_descr);
				adornment->short_descr = str_dup(argument);
				send_to_char("ARMOR Adornment Short Description changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg, "description"))
			{
				if (argument[0] == '\0')
				{
					string_append(ch, &adornment->description);
					return true;
				}

				send_to_char(formatf("Syntax:  adornment %d description\n\r", index), ch);
				return false;
			}

			if (!str_prefix(arg, "spell"))
			{
				SKILL_DATA *skill;
				if (IS_NULLSTR(argument))
				{
					send_to_char("Please specify a spell name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(argument);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_equip_spell(skill))
					{
						send_to_char("That spell cannot be equipped.\n\r", ch);
						return false;
					}
				}

				for(int i = ARMOR(pObj)->max_adornments; i-- > 0; )
				{
					ADORNMENT_DATA *adorn = ARMOR(pObj)->adornments[i];

					if (IS_VALID(adorn) && adornment != adorn && adorn->spell != NULL)
					{
						if (adorn->spell->skill == skill)
						{
							send_to_char("Already have an adornment with that spell.\n\r", ch);
							return false;
						}
					}
				}

				if (adornment->spell != NULL)
				{
					adornment->spell->skill = skill;
				}
				else
				{
					SPELL_DATA *spell = new_spell();
					spell->skill	= skill;
					spell->level	= pObj->level;
					spell->repop	= 100;
					spell->next		= NULL;

					adornment->spell = spell;
				}

				send_to_char(formatf("ARMOR Adornment #%d Spell set.\n\r", index), ch);
				return true;
			}

			if (!str_prefix(arg, "level"))
			{
				int level;
				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					send_to_char(formatf("Level range is 1-%d.\n\r", MAX_CLASS_LEVEL), ch);
					return false;
				}

				if (adornment->spell == NULL)
				{
					send_to_char("Please assign a spell first to the adornment.\n\r", ch);
					return false;
				}

				adornment->spell->level = level;
				send_to_char(formatf("ARMOR Adornment %d spell level set.\n\r", index), ch);
				return true;
			}

			oedit_type_armor(ch, formatf("adornment %d", index));
			return false;
		}


		if (pObj->item_type != ITEM_ARMOUR)
		{
			if (!str_prefix(arg, "remove"))
			{
				free_armor_data(ARMOR(pObj));
				ARMOR(pObj) = NULL;

				send_to_char("Armor data removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_ARMOUR))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			ARMOR(pObj) = new_armor_data();
			send_to_char("ARMOR data added to object.\n\r\n\r", ch);
			return true;
		}

	}

	oedit_type_armor(ch, "");
	return false;
}


OEDIT(oedit_type_body_part)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_BODY_PART(pObj))
		{
			send_to_char("Syntax:  bodypart parts <parts>\n\r", ch);
			send_to_char("         bodypart race <race>\n\r", ch);

			if (pObj->item_type != ITEM_BODY_PART)
				send_to_char("         bodypart remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  bodypart add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_BODY_PART(pObj))
	{
		if (!str_prefix(arg, "parts"))
		{
			long value;
			if ((value = flag_value(part_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid part flags.  Use '? part' for list of valid flags.\n\r", ch);
				show_flag_cmds(ch, part_flags);
				return false;
			}

			TOGGLE_BIT(BODY_PART(pObj)->parts, value);
			send_to_char("BODY_PART parts toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "race"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please specify a race.\n\r", ch);
				return false;
			}

			RACE_DATA *race = get_race_data(argument);
			if (!IS_VALID(race))
			{
				send_to_char("No such race by that name.\n\r", ch);
				return false;
			}

			BODY_PART(pObj)->race = race;
			send_to_char("BODY_PART race changed.\n\r", ch);
			return true;
			
		}

		if (pObj->item_type != ITEM_BODY_PART)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_body_part_data(BODY_PART(pObj));
				BODY_PART(pObj) = NULL;

				send_to_char("BODY_PART type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_BODY_PART))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		BODY_PART(pObj) = new_body_part_data();
		send_to_char("BODY_PART type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_body_part(ch, "");
	return false;
}


OEDIT(oedit_type_page)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_PAGE(pObj))
		{
			send_to_char("Syntax:  page title[ <title>]\n\r", ch);
			send_to_char("         page text (opens string editor)\n\r", ch);
			send_to_char("         page number <#>\n\r", ch);
			send_to_char("         page book <widevnum|none>\n\r", ch);

			if (pObj->item_type != ITEM_PAGE)
			{
				send_to_char("         page remove\n\r", ch);
			}
		}
		else
			send_to_char("Syntax:  page add\n\r", ch);

		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);
	if (IS_PAGE(pObj))
	{
		if (!str_prefix(arg, "title"))
		{
			if (strlen_no_colours(argument) > 70)
			{
				send_to_char("Title too long.  Please limit non-colour length to 70.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(PAGE(pObj)->title);
			PAGE(pObj)->title = str_dup(argument);

			send_to_char("PAGE Title set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "text"))
		{
			string_append(ch, &PAGE(pObj)->text);
			return true;
		}
		else if (!str_prefix(arg, "number"))
		{
			int page_no;
			if (!is_number(argument) || (page_no = atoi(argument)) < 1)
			{
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			PAGE(pObj)->page_no = page_no;
			send_to_char("PAGE Number set.\n\r", ch);
			return true;
		}
		else if(!str_prefix(arg, "book"))
		{
			WNUM wnum;

			if (!str_prefix(argument, "none"))
			{
				wnum.pArea = NULL;
				wnum.vnum = 0;
			}
			else if (!parse_widevnum(argument, pObj->area, &wnum))
			{
				send_to_char("Please specify a widevnum.\n\r", ch);
				return false;
			}
			else
			{
				OBJ_INDEX_DATA *book = get_obj_index(wnum.pArea, wnum.vnum);
				if (!book)
				{
					send_to_char("No object exists at that widevnum.\n\r", ch);
					return false;
				}

				if (!IS_BOOK(book))
				{
					send_to_char("Object has not book definition.\n\r", ch);
					return false;
				}
			}

			PAGE(pObj)->book.auid = wnum.pArea ? wnum.pArea->uid : 0;
			PAGE(pObj)->book.vnum = wnum.vnum;
			send_to_char("PAGE Original Book set.\n\r", ch);
			return true;
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_PAGE))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			PAGE(pObj) = new_book_page();
			send_to_char("PAGE data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_page(ch, "");
	return false;
}

OEDIT(oedit_type_book)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_BOOK(pObj))
		{
			send_to_char("Syntax:  book name <name>\n\r", ch);
			send_to_char("         book short <short description>\n\r", ch);
			send_to_char("         book flags <flags>\n\r", ch);
			send_to_char("         book openpage <page#>\n\r", ch);
			send_to_char("         book page clear\n\r", ch);
			send_to_char("         book page renumber\n\r", ch);
			send_to_char("         book page add[ <page#>] (if omitted, appends to the end)\n\r", ch);
			send_to_char("         book page <page#> title[ <title>]\n\r", ch);
			send_to_char("         book page <page#> text (opens string editor)\n\r", ch);
			send_to_char("         book page <page#> remove\n\r", ch);
			send_to_char("         book lock add\n\r", ch);
			send_to_char("         book lock remove\n\r", ch);
			send_to_char("         book lock key <widevnum>\n\r", ch);
			send_to_char("         book lock key clear\n\r", ch);
			send_to_char("         book lock flags [flags]\n\r", ch);
			send_to_char("         book lock pick [0-100]\n\r", ch);

			if (pObj->item_type != ITEM_BOOK)
			{
				send_to_char("         book remove\n\r", ch);
			}
		}
		else
		{
			send_to_char("Syntax:  book add\n\r", ch);
		}

		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);
	if (IS_BOOK(pObj))
	{
		if (!str_prefix(arg, "name"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please provide a name.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(BOOK(pObj)->name);
			BOOK(pObj)->name = str_dup(argument);

			send_to_char("BOOK Name set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "short"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please provide a short description.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(BOOK(pObj)->short_descr);
			BOOK(pObj)->short_descr = str_dup(argument);

			send_to_char("BOOK Name set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(book_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid book flag.\n\r", ch);
				send_to_char("Use '? book' for list of valid flags.\n\r", ch);
				show_help(ch, "book");
				return false;
			}

			TOGGLE_BIT(BOOK(pObj)->flags, value);
			send_to_char("BOOK Flags toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "openpage"))
		{
			if (list_size(BOOK(pObj)->pages) < 1)
			{
				send_to_char("Please add a page first.\n\r", ch);
				return false;
			}

			int page_no;
			if (!str_prefix(arg, "none"))
			{
				page_no = 0;
			}
			else if (!is_number(argument) || (page_no = atoi(argument)) < 1 || page_no > list_size(BOOK(pObj)->pages))
			{
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(BOOK(pObj)->pages));
				send_to_char(buf, ch);
				return false;
			}

			BOOK(pObj)->open_page = page_no;
			send_to_char("BOOK Open Page set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "lock"))
		{
			argument = one_argument(argument, arg);

			if( !str_prefix(arg, "add") )
			{
				if( BOOK(pObj)->lock )
				{
					send_to_char("BOOK already has a lock state.\n\r", ch);
					return false;
				}

				BOOK(pObj)->lock = new_lock_state();
				send_to_char("Lock State added.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "remove") )
			{
				if( !BOOK(pObj)->lock )
				{
					send_to_char("BOOK does not have a lock state.\n\r", ch);
					return false;
				}


				free_lock_state(BOOK(pObj)->lock);
				BOOK(pObj)->lock = NULL;

				send_to_char("Lock State removed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "key") )
			{
				if( !BOOK(pObj)->lock )
				{
					send_to_char("BOOK does not have a lock state.\n\r", ch);
					return false;
				}

				if( argument[0] == '\0' )
				{
					send_to_char("Syntax:  book lock key <widevnum>\n\r", ch);
					send_to_char("         book lock key clear\n\r", ch);
					return false;
				}

				WNUM wnum;
				if( parse_widevnum(argument, pObj->area, &wnum) )
				{
					OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

					if( !key )
					{
						send_to_char("That object does not exist.\n\r", ch);
						return false;
					}

					if( key->item_type != ITEM_KEY )
					{
						send_to_char("That object is not a key.\n\r", ch);
						return false;
					}

					// TODO: make a list
					BOOK(pObj)->lock->key_wnum = wnum;
					send_to_char("Lock State key set.\n\r", ch);
					return true;
				}
				else if( !str_prefix(argument, "clear") )
				{
					BOOK(pObj)->lock->key_wnum = wnum_zero;
					send_to_char("Lock State key removed.\n\r", ch);
					return true;
				}

				oedit_type_book(ch, "lock key");
				return false;
			}

			if( !str_prefix(arg, "flags") )
			{
				if( !BOOK(pObj)->lock )
				{
					send_to_char("BOOK does not have a lock state.\n\r", ch);
					return false;
				}

				int value = flag_value(lock_flags, argument);

				if( value == NO_FLAG )
				{
					send_to_char("Syntax:  book lock flags [flags]\n\r", ch);
					send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
					show_help(ch, "lock");
					return false;
				}

				BOOK(pObj)->lock->flags ^= value;
				send_to_char("Lock State flags changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "pick") )
			{
				if( !BOOK(pObj)->lock )
				{
					send_to_char("BOOK does not have a lock state.\n\r", ch);
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
					send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
					return false;
				}

				BOOK(pObj)->lock->pick_chance = value;
				send_to_char("Lock State pick chance set.\n\r", ch);
				return true;
			}

			oedit_type_book(ch, "");
			return false;
		}

		if (!str_prefix(arg, "page"))
		{
			char arg2[MIL];

			argument = one_argument(argument, arg2);

			if (!str_prefix(arg2, "add"))
			{
				int page_no = 0;
				if (argument[0] != '\0')
				{
					if (!is_number(argument) || (page_no = atoi(argument)) < 1)
					{
						send_to_char("Please specify a positive number.\n\r", ch);
						return false;
					}
				}

				BOOK_PAGE *page = new_book_page();
				if (page_no > 0)
				{
					page->page_no = page_no;
					if (!book_insert_page(BOOK(pObj), page))
					{
						send_to_char("Attempted to add a duplicate page number.\n\r", ch);
						free_book_page(page);
						return false;
					}
				}
				else
				{
					BOOK_PAGE *last_page = (BOOK_PAGE *)list_nthdata(BOOK(pObj)->pages, -1);

					// Append to the end
					list_appendlink(BOOK(pObj)->pages, page);
					if (last_page)
						page->page_no = last_page->page_no + 1;
					else
						page->page_no = 1;	// No pages in the book, so this will be the first
				}

				sprintf(buf, "BOOK Page %d added.\n\r", page->page_no);
				send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg2, "clear"))
			{
				if (list_size(BOOK(pObj)->pages) < 1)
				{
					send_to_char("The book is empty.\n\r", ch);
					return false;
				}

				list_clear(BOOK(pObj)->pages);
				send_to_char("BOOK Pages cleared.\n\r", ch);
				return true;
			}
			else if (!str_prefix(arg2, "renumber"))
			{
				if (list_size(BOOK(pObj)->pages) < 1)
				{
					send_to_char("The book is empty.\n\r", ch);
					return false;
				}

				ITERATOR it;
				int page_no = 0;
				BOOK_PAGE *page;

				iterator_start(&it, BOOK(pObj)->pages);
				while((page = (BOOK_PAGE *)iterator_nextdata(&it)))
				{
					page->page_no = ++page_no;
				}
				iterator_stop(&it);

				send_to_char("BOOK Pages renumbered.\n\r", ch);
				return true;
			}
			else
			{
				if (list_size(BOOK(pObj)->pages) < 1)
				{
					send_to_char("Please add a page first.\n\r", ch);
					return false;
				}

				int page_no;
				if (!is_number(arg2) || (page_no = atoi(arg2)) < 1 || page_no > list_size(BOOK(pObj)->pages))
				{
					sprintf(buf, "Please specify a page number from 1 to %d.\n\r", list_size(BOOK(pObj)->pages));
					send_to_char(buf, ch);
					return false;
				}

				char arg3[MIL];
				argument = one_argument(argument, arg3);

				if (!str_prefix(arg3, "title"))
				{
					if (strlen_no_colours(argument) > 70)
					{
						send_to_char("Title too long.  Please limit non-colour length to 70.\n\r", ch);
						return false;
					}

					BOOK_PAGE *page = book_get_page(BOOK(pObj), page_no);

					smash_tilde(argument);
					free_string(page->title);
					page->title = str_dup(argument);

					send_to_char("BOOK Page Title set.\n\r", ch);
					return true;
				}
				else if (!str_prefix(arg3, "text"))
				{
					BOOK_PAGE *page = book_get_page(BOOK(pObj), page_no);

					string_append(ch, &page->text);
					return true;
				}
				else if (!str_prefix(arg3, "remove"))
				{
					// Find the page with page number
					ITERATOR it;
					BOOK_PAGE *page;
					iterator_start(&it, BOOK(pObj)->pages);
					while((page = (BOOK_PAGE *)iterator_nextdata(&it)))
					{
						if (page->page_no == page_no)
						{
							iterator_remcurrent(&it);
							break;
						}
					}
					iterator_stop(&it);

					sprintf(buf, "BOOK Page %d removed.\n\r", page_no);
					send_to_char(buf, ch);
					return true;
				}
			}

			oedit_type_book(ch, "");
			return false;
		}

		if (pObj->item_type != ITEM_BOOK)
		{
			if (!str_prefix(arg, "remove"))
			{
				free_book_data(BOOK(pObj));
				BOOK(pObj) = NULL;

				send_to_char("BOOK data removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_BOOK))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			BOOK(pObj) = new_book_data();
			send_to_char("BOOK data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_book(ch, "");
	return false;
}


OEDIT(oedit_type_cart)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_CART(pObj))
		{
			send_to_char("Syntax:  cart flags <flags>\n\r", ch);
			send_to_char("         cart minstrength <strength>\n\r", ch);
			send_to_char("         cart movedelay <delay>\n\r", ch);

			if (pObj->item_type != ITEM_CART)
				send_to_char("         cart remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  cart add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_CART(pObj))
	{
		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(cart_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid cart flags.  Use '? cart' for list of valid flags.\n\r", ch);
				show_flag_cmds(ch, cart_flags);
				return false;
			}

			TOGGLE_BIT(CART(pObj)->flags, value);
			send_to_char("CART Flags toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "minstrength"))
		{
			int minstr;
			if (!is_number(argument) || (minstr = atoi(argument)) < 0)
			{
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			CART(pObj)->min_strength = minstr;
			send_to_char("CART Minimum Strength changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "movedelay"))
		{
			int delay;
			if (!is_number(argument) || (delay = atoi(argument)) < 0)
			{
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			CART(pObj)->move_delay = delay;
			send_to_char("CART Move Delay changed.\n\r", ch);
			return true;
		}

		if (pObj->item_type != ITEM_CART)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_cart_data(CART(pObj));
				CART(pObj) = NULL;

				send_to_char("CART type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_CART))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		CART(pObj) = new_cart_data();
		send_to_char("CART type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_cart(ch, "");
	return false;
}

OEDIT(oedit_type_compass)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_COMPASS(pObj))
		{
			send_to_char("Syntax:  compass accuracy <percent>\n\r", ch);
			send_to_char("         compass location set <wilds uid> <x> <y>\n\r", ch);
			send_to_char("         compass location set here\n\r", ch);
			send_to_char("         compass location clear\n\r", ch);

			if (pObj->item_type != ITEM_COMPASS)
				send_to_char("         compass remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  compass add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_COMPASS(pObj))
	{
		if (!str_prefix(arg, "accuracy"))
		{
			int16_t accuracy;
			if (!is_number(argument) || (accuracy = atoi(argument)) < 0 || accuracy > 100)
			{
				send_to_char("Please provide a percent.\n\r", ch);
				return false;
			}

			COMPASS(pObj)->accuracy = accuracy;
			send_to_char("COMPASS Accuracy changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "location"))
		{
			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "set"))
			{
				argument = one_argument(argument, arg);
	
				if (!str_prefix(arg, "here"))
				{
					if (ch->in_room->wilds)
					{
						COMPASS(pObj)->wuid = ch->in_room->wilds->uid;
						COMPASS(pObj)->x = ch->in_room->x;
						COMPASS(pObj)->y = ch->in_room->y;

						send_to_char(formatf("COMPASS Location set to %s (%ld) at (%ld, %ld).\n\r", ch->in_room->wilds->name, ch->in_room->wilds->uid, ch->in_room->x, ch->in_room->y), ch);
						return true;
					}

					send_to_char("You must be in a wilderness room to do this.\n\r", ch);
					return false;
				}

				long wuid = 0;
				if(!is_number(arg) || (wuid = atol(arg)) < 1)
				{
					send_to_char("Please provide a positive number.\n\r", ch);
					return false;
				}

				WILDS_DATA *wilds = get_wilds_from_uid(NULL, wuid);
				if (!wilds)
				{
					send_to_char("No such wilds exists.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg);

				long x;
				if (!is_number(arg) || (x = atol(arg)) < 0 || x > wilds->map_size_x)
				{
					send_to_char(formatf("Please provide a number from 0 to %d.\n\r", wilds->map_size_x), ch);
					return false;
				}

				long y;
				if (!is_number(arg) || (y = atol(arg)) < 0 || x > wilds->map_size_y)
				{
					send_to_char(formatf("Please provide a number from 0 to %d.\n\r", wilds->map_size_y), ch);
					return false;
				}

				COMPASS(pObj)->wuid = wuid;
				COMPASS(pObj)->x = x;
				COMPASS(pObj)->y = y;
				send_to_char(formatf("COMPASS Location set to %s (%ld) at (%ld, %ld).\n\r", wilds->name, wilds->uid, x, y), ch);
				return true;
			}
			else if (!str_prefix(arg, "clear"))
			{
				COMPASS(pObj)->wuid = 0;
				COMPASS(pObj)->x = 0;
				COMPASS(pObj)->y = 0;
				send_to_char("COMPASS Location cleared.\n\r", ch);
				return true;
			}

			send_to_char("Syntax:  compass location set <wilds uid> <x> <y>\n\r", ch);
			send_to_char("         compass location set here\n\r", ch);
			send_to_char("         compass location clear\n\r", ch);
			return false;
		}

		if (pObj->item_type != ITEM_COMPASS)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_compass_data(COMPASS(pObj));
				COMPASS(pObj) = NULL;

				send_to_char("COMPASS type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_COMPASS))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		COMPASS(pObj) = new_compass_data();
		send_to_char("COMPASS type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_compass(ch, "");
	return false;
}

OEDIT(oedit_type_container)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_CONTAINER(pObj))
		{
			send_to_char("Syntax:  container name <name>\n\r", ch);
			send_to_char("         container short <short description>\n\r", ch);
			send_to_char("         container flags <flags>\n\r", ch);
			send_to_char("         container weight <#max|unlimited>[ <%multiplier>]\n\r", ch);
			send_to_char("         container volume <#max|unlimited>\n\r", ch);
			send_to_char("         container lock add\n\r", ch);
			send_to_char("         container lock remove\n\r", ch);
			send_to_char("         container lock key <widevnum>\n\r", ch);
			send_to_char("         container lock key clear\n\r", ch);
			send_to_char("         container lock flags [flags]\n\r", ch);
			send_to_char("         container lock pick [0-100]\n\r", ch);
			send_to_char("         container whitelist list\n\r", ch);
			send_to_char("         container whitelist clear\n\r", ch);
			send_to_char("         container whitelist add <type>[ <subtype>]\n\r", ch);
			send_to_char("         container whitelist remove <#>\n\r", ch);
			send_to_char("         container blacklist list\n\r", ch);
			send_to_char("         container blacklist clear\n\r", ch);
			send_to_char("         container blacklist add <type>[ <subtype>]\n\r", ch);
			send_to_char("         container blacklist remove <#>\n\r", ch);

			if (pObj->item_type != ITEM_CONTAINER)
				send_to_char("         container remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  container add\n\r", ch);
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_CONTAINER(pObj))
	{
		if (!str_prefix(arg, "name"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please specify a name.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(CONTAINER(pObj)->name);
			CONTAINER(pObj)->name = str_dup(argument);
			send_to_char("CONTAINER Name changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "short"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please specify a short description.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(CONTAINER(pObj)->short_descr);
			CONTAINER(pObj)->short_descr = str_dup(argument);
			send_to_char("CONTAINER Short Description changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(container_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid container flag.\n\r", ch);
				send_to_char("Please use one of the following: ({Y? container{x)\n\r", ch);
				show_help(ch, "container");
				return false;
			}

			TOGGLE_BIT(CONTAINER(pObj)->flags, value);
			send_to_char("CONTAINER flags toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "lock"))
		{
			argument = one_argument(argument, arg);

			if( !str_prefix(arg, "add") )
			{
				if( CONTAINER(pObj)->lock )
				{
					send_to_char("CONTAINER already has a lock state.\n\r", ch);
					return false;
				}

				CONTAINER(pObj)->lock = new_lock_state();
				send_to_char("Lock State added.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "remove") )
			{
				if( !CONTAINER(pObj)->lock )
				{
					send_to_char("CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}


				free_lock_state(CONTAINER(pObj)->lock);
				CONTAINER(pObj)->lock = NULL;

				send_to_char("Lock State removed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "key") )
			{
				if( !CONTAINER(pObj)->lock )
				{
					send_to_char("CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}

				if( argument[0] == '\0' )
				{
					send_to_char("Syntax:  container lock key <widevnum>\n\r", ch);
					send_to_char("         container lock key clear\n\r", ch);
					return false;
				}

				WNUM wnum;
				if( parse_widevnum(argument, pObj->area, &wnum) )
				{
					OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

					if( !key )
					{
						send_to_char("That object does not exist.\n\r", ch);
						return false;
					}

					if( key->item_type != ITEM_KEY )
					{
						send_to_char("That object is not a key.\n\r", ch);
						return false;
					}

					// TODO: make a list
					CONTAINER(pObj)->lock->key_wnum = wnum;
					send_to_char("Lock State key set.\n\r", ch);
					return true;
				}
				else if( !str_prefix(argument, "clear") )
				{
					CONTAINER(pObj)->lock->key_wnum = wnum_zero;
					send_to_char("Lock State key removed.\n\r", ch);
					return true;
				}

				oedit_type_container(ch, "lock key");
				return false;
			}

			if( !str_prefix(arg, "flags") )
			{
				if( !CONTAINER(pObj)->lock )
				{
					send_to_char("CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}

				int value = flag_value(lock_flags, argument);

				if( value == NO_FLAG )
				{
					send_to_char("Syntax:  container lock flags [flags]\n\r", ch);
					send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
					show_help(ch, "lock");
					return false;
				}

				CONTAINER(pObj)->lock->flags ^= value;
				send_to_char("Lock State flags changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "pick") )
			{
				if( !CONTAINER(pObj)->lock )
				{
					send_to_char("CONTAINER does not have a lock state.\n\r", ch);
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
					send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
					return false;
				}

				CONTAINER(pObj)->lock->pick_chance = value;
				send_to_char("Lock State pick chance set.\n\r", ch);
				return true;
			}

			oedit_type_container(ch, "");
			return false;
		}

		if (!str_prefix(arg, "weight"))
		{
			char arg2[MIL];

			argument = one_argument(argument, arg2);

			int max_weight;
			if (!str_prefix(arg2, "unlimited"))
				max_weight = -1;
			else if (!is_number(arg2) || (max_weight = atoi(arg2)) < 1)
			{
				send_to_char("Please provide a positive number.\n\r", ch);
				return false;
			}

			int weight_multiplier = 100;
			if (argument[0] != '\0')
			{
				if (!is_number(argument) || (weight_multiplier = atoi(argument)) < 1 || weight_multiplier > 100)
				{
					send_to_char("Please provide a number from 1 to 100.\n\r", ch);
					return false;
				}
			}

			CONTAINER(pObj)->max_weight = max_weight;
			CONTAINER(pObj)->weight_multiplier = weight_multiplier;
			send_to_char("CONTAINER Weight settinsg changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "volume"))
		{
			int max_volume;
			if (!str_prefix(argument, "unlimited"))
				max_volume = -1;
			else if(!is_number(argument) || (max_volume = atoi(argument)) < 1)
			{
				send_to_char("Please provide a positive number.\n\r", ch);
				return false;
			}

			CONTAINER(pObj)->max_volume = max_volume;
			send_to_char("CONTAINER Volume setting changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "whitelist"))
		{
			if (!str_prefix(argument, "list"))
			{
				oedit_type_container_showlist(ch, CONTAINER(pObj)->whitelist, "Whitelist", 'W');
				return false;
			}

			if (!str_prefix(argument, "clear"))
			{
				if (list_size(CONTAINER(pObj)->whitelist) < 1)
				{
					send_to_char("The whitelist is empty.\n\r", ch);
					return false;
				}

				list_clear(CONTAINER(pObj)->whitelist);
				send_to_char("CONTAINER Whitelist cleared.\n\r", ch);
				return true;
			}

			char arg2[MIL];
			argument = one_argument(argument, arg2);

			if (!str_prefix(arg2, "add"))
			{
				char arg3[MIL];

				argument = one_argument(argument, arg3);

				int type;
				if ((type = stat_lookup(arg3, type_flags, -1)) < 0)
				{
					send_to_char("Invalid item type.\n\r", ch);
					send_to_char("Please use one of the following: ({Y? type{x)\n\r", ch);
					show_help(ch, "type");
					return false;
				}

				int subtype = -1;
				if (type == ITEM_WEAPON)
				{
					if ((subtype = stat_lookup(argument, weapon_class, -1)) < 0)
					{
						send_to_char("Invalid weapon class.\n\r", ch);
						send_to_char("Please use one of the following: ({Y? wclass{x)\n\r", ch);
						show_help(ch, "wclass");
						return false;
					}
				}

				if (__container_is_listed(CONTAINER(pObj)->whitelist, type, subtype))
				{
					send_to_char("That is already in the whitelist.\n\r", ch);
					return false;
				}

				if (__container_is_listed(CONTAINER(pObj)->blacklist, type, subtype))
				{
					send_to_char("That is already in the blacklist.\n\r", ch);
					return false;
				}

				CONTAINER_FILTER *filter = new_container_filter();
				filter->item_type = type;
				filter->sub_type = subtype;
				
				list_appendlink(CONTAINER(pObj)->whitelist, filter);
				send_to_char("CONTAINER Whitelist changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg2, "remove"))
			{
				if (list_size(CONTAINER(pObj)->whitelist) < 1)
				{
					send_to_char("Whitelist is empty.\n\r", ch);
					return false;
				}

				int filter_no;
				if (!is_number(argument) || (filter_no = atoi(argument)) < 1 || filter_no > list_size(CONTAINER(pObj)->whitelist))
				{
					sprintf(buf, "Please select a number from 1 to %d.\n\r", list_size(CONTAINER(pObj)->whitelist));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(CONTAINER(pObj)->whitelist, filter_no, true);
				sprintf(buf, "CONTAINER Whitelist #%d removed.\n\r", filter_no);
				send_to_char(buf, ch);
				return true;
			}
		}

		if (!str_prefix(arg, "blacklist"))
		{
			if (!str_prefix(argument, "list"))
			{
				oedit_type_container_showlist(ch, CONTAINER(pObj)->blacklist, "Blacklist", 'D');
				return false;
			}

			if (!str_prefix(argument, "clear"))
			{
				if (list_size(CONTAINER(pObj)->blacklist) < 1)
				{
					send_to_char("The blacklist is empty.\n\r", ch);
					return false;
				}

				list_clear(CONTAINER(pObj)->blacklist);
				send_to_char("CONTAINER Blacklist cleared.\n\r", ch);
				return true;
			}

			char arg2[MIL];
			argument = one_argument(argument, arg2);

			if (!str_prefix(arg2, "add"))
			{
				char arg3[MIL];

				argument = one_argument(argument, arg3);

				int type;
				if ((type = stat_lookup(arg3, type_flags, -1)) < 0)
				{
					send_to_char("Invalid item type.\n\r", ch);
					send_to_char("Please use one of the following: ({Y? type{x)\n\r", ch);
					show_help(ch, "type");
					return false;
				}

				int subtype = -1;
				if (type == ITEM_WEAPON)
				{
					if ((subtype = stat_lookup(argument, weapon_class, -1)) < 0)
					{
						send_to_char("Invalid weapon class.\n\r", ch);
						send_to_char("Please use one of the following: ({Y? wclass{x)\n\r", ch);
						show_help(ch, "wclass");
						return false;
					}
				}

				if (__container_is_listed(CONTAINER(pObj)->whitelist, type, subtype))
				{
					send_to_char("That is already in the whitelist.\n\r", ch);
					return false;
				}

				if (__container_is_listed(CONTAINER(pObj)->blacklist, type, subtype))
				{
					send_to_char("That is already in the blacklist.\n\r", ch);
					return false;
				}

				CONTAINER_FILTER *filter = new_container_filter();
				filter->item_type = type;
				filter->sub_type = subtype;
				
				list_appendlink(CONTAINER(pObj)->whitelist, filter);
				send_to_char("CONTAINER Blacklist changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg2, "remove"))
			{
				if (list_size(CONTAINER(pObj)->blacklist) < 1)
				{
					send_to_char("Blacklist is empty.\n\r", ch);
					return false;
				}

				int filter_no;
				if (!is_number(argument) || (filter_no = atoi(argument)) < 1 || filter_no > list_size(CONTAINER(pObj)->blacklist))
				{
					sprintf(buf, "Please select a number from 1 to %d.\n\r", list_size(CONTAINER(pObj)->blacklist));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(CONTAINER(pObj)->blacklist, filter_no, true);
				sprintf(buf, "CONTAINER Blacklist #%d removed.\n\r", filter_no);
				send_to_char(buf, ch);
				return true;
			}
		}

		if (pObj->item_type != ITEM_CONTAINER)
		{
			if (!str_prefix(arg, "remove"))
			{
				free_container_data(CONTAINER(pObj));
				CONTAINER(pObj) = NULL;

				send_to_char("CONTAINER data removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_CONTAINER))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			CONTAINER(pObj) = new_container_data();
			send_to_char("CONTAINER data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_container(ch, "");
	return false;
}

OEDIT(oedit_type_corpse)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_CORPSE(pObj))
		{
			send_to_char("Syntax:  corpse type <type>\n\r", ch);
			send_to_char("         corpse race <race>\n\r", ch);
			send_to_char("         corpse flags <flags>\n\r", ch);
			if (IS_IMPLEMENTOR9(ch))
				send_to_char("         corpse player <boolean> (security 9 implementor only)\n\r", ch);
			send_to_char("         corpse animate <chance>\n\r", ch);
			send_to_char("         corpse resurrect <chance>\n\r", ch);
			send_to_char("         corpse parts <parts>\n\r", ch);
			send_to_char("         corpse mobile <widevnum|none>\n\r", ch);

			if (pObj->item_type != ITEM_CORPSE)
				send_to_char("         corpse remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  corpse add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_CORPSE(pObj))
	{
		if (!str_prefix(arg, "type"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please provide a corpse type.\n\r", ch);
				send_to_char("Use 'corpselist' for list of corpse types.\n\r", ch);
				return false;
			}

			CORPSE_TYPE *type = get_corpse_type(argument);
			if (!IS_VALID(type))
			{
				send_to_char("No such corpse type by that name.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->type = type;
			send_to_char("CORPSE type changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "race"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please specify a valid race.\n\r", ch);
				send_to_char("Use 'racelist' for list of valid races.\n\r", ch);
				return false;
			}

			RACE_DATA *race = get_race_data(argument);
			if (!IS_VALID(race))
			{
				send_to_char("No such race by that name.\n\r", ch);
				send_to_char("Use 'racelist' for list of valid races.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->race = race;
			send_to_char("CORPSE Race changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(corpse_object_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid corpse flag.  Use '? corpse' for valid flags.\n\r", ch);
				show_flag_cmds(ch, corpse_object_flags);
				return false;
			}

			TOGGLE_BIT(CORPSE(pObj)->flags, value);
			send_to_char("CORPSE flags changed.\n\r", ch);
			return true;
		}

		// Security 9 Implementor only.  Only time to ever deal with this is when setting the reserved player corpse object
		if (!str_prefix(arg, "player") && IS_IMPLEMENTOR9(ch))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please provide a boolean (true/false, yes/no, on/off) value.\n\r", ch);
				return false;
			}

			bool player;
			if (!str_prefix(argument, "true") || !str_prefix(argument, "yes") || !str_prefix(argument, "on"))
				player = true;
			else if (!str_prefix(argument, "false") || !str_prefix(argument, "no") || !str_prefix(argument, "off"))
				player = false;
			else
			{
				send_to_char("Please provide a boolean (true/false, yes/no, on/off) value.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->player = player;
			send_to_char("CORPSE Player state changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "animate"))
		{
			int chance;
			if (!is_number(argument) || (chance = atoi(argument)) < 0 || chance > 100)
			{
				send_to_char("Please provide a number from 0 to 100.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->animate = chance;
			send_to_char("CORPSE animation chance changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "resurrect"))
		{
			int chance;
			if (!is_number(argument) || (chance = atoi(argument)) < 0 || chance > 100)
			{
				send_to_char("Please provide a number from 0 to 100.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->resurrect = chance;
			send_to_char("CORPSE resurrection chance changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "parts"))
		{
			long value;
			if ((value = flag_value(part_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid part flag.  Use '? parts' for valid flags.\n\r", ch);
				show_flag_cmds(ch, part_flags);
				return false;
			}

			TOGGLE_BIT(CORPSE(pObj)->parts, value);
			send_to_char("CORPSE body parts changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "mobile"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please provide a widevnum or none.\n\r", ch);
				return false;
			}

			MOB_INDEX_DATA *mobile;
			WNUM wnum;
			if (!str_prefix(argument, "none"))
				mobile = NULL;
			else if (!parse_widevnum(argument, ch->in_room->area, &wnum))
			{
				send_to_char("Please provide a widevnum.\n\r", ch);
				return false;
			}
			else if (!(mobile = get_mob_index_wnum(wnum)))
			{
				send_to_char("No such mobile by that widevnum.\n\r", ch);
				return false;
			}

			CORPSE(pObj)->mobile = mobile;
			send_to_char("CORPSE mobile changed.\n\r", ch);
			return true;
		}

		if (pObj->item_type != ITEM_CORPSE)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_corpse_data(CORPSE(pObj));
				CORPSE(pObj) = NULL;

				send_to_char("CORPSE type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_CORPSE))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		CORPSE(pObj) = new_corpse_data();
		send_to_char("CORPSE type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_corpse(ch, "");
	return false;
}

OEDIT(oedit_type_fluid_container)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_FLUID_CON(pObj))
		{
			send_to_char("Syntax:  fluid name <name>\n\r", ch);
			send_to_char("         fluid short <short description>\n\r", ch);
			send_to_char("         fluid flags <flags>\n\r", ch);
			send_to_char("         fluid lock add\n\r", ch);
			send_to_char("         fluid lock remove\n\r", ch);
			send_to_char("         fluid lock key <widevnum>\n\r", ch);
			send_to_char("         fluid lock key clear\n\r", ch);
			send_to_char("         fluid lock flags [flags]\n\r", ch);
			send_to_char("         fluid lock pick <chance %>\n\r", ch);
			send_to_char("         fluid liquid set <liquid>\n\r", ch);
			send_to_char("         fluid liquid clear\n\r", ch);
			send_to_char("         fluid capacity <capacity>|unlimited\n\r", ch);
			send_to_char("         fluid amount <amount>\n\r", ch);
			send_to_char("         fluid poison <poison %>[ <refill rate per tick>]\n\r", ch);
			send_to_char("         fluid refill <rate per tick>\n\r", ch);
			send_to_char("         fluid potion add <spell name> <level>\n\r", ch);
			send_to_char("         fluid potion remove <index>\n\r", ch);
			send_to_char("         fluid potion clear\n\r", ch);

			if (pObj->item_type != ITEM_FLUID_CONTAINER)
				send_to_char("         fluid remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  fluid add\n\r", ch);
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_FLUID_CON(pObj))
	{
		if (!str_prefix(arg, "name"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please specify a name.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(FLUID_CON(pObj)->name);
			FLUID_CON(pObj)->name = str_dup(argument);
			send_to_char("FLUID CONTAINER Name changed.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "short"))
		{
			if (IS_NULLSTR(argument))
			{
				send_to_char("Please specify a short description.\n\r", ch);
				return false;
			}

			smash_tilde(argument);
			free_string(FLUID_CON(pObj)->short_descr);
			FLUID_CON(pObj)->short_descr = str_dup(argument);
			send_to_char("FLUID CONTAINER Short Description changed.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(fluid_con_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid fluid container flag.\n\r", ch);
				send_to_char("Please use one of the following: ({Y? fluid_con{x)\n\r", ch);
				show_help(ch, "fluid_con");
				return false;
			}

			TOGGLE_BIT(FLUID_CON(pObj)->flags, value);
			send_to_char("FLUID CONTAINER flags toggled.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "lock"))
		{
			argument = one_argument(argument, arg);

			if( !str_prefix(arg, "add") )
			{
				if( FLUID_CON(pObj)->lock )
				{
					send_to_char("FLUID CONTAINER already has a lock state.\n\r", ch);
					return false;
				}

				FLUID_CON(pObj)->lock = new_lock_state();
				send_to_char("Lock State added.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "remove") )
			{
				if( !FLUID_CON(pObj)->lock )
				{
					send_to_char("FLUID CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}


				free_lock_state(FLUID_CON(pObj)->lock);
				FLUID_CON(pObj)->lock = NULL;

				send_to_char("Lock State removed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "key") )
			{
				if( !FLUID_CON(pObj)->lock )
				{
					send_to_char("FLUID CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}

				if( argument[0] == '\0' )
				{
					send_to_char("Syntax:  fluid lock key <widevnum>\n\r", ch);
					send_to_char("         fluid lock key clear\n\r", ch);
					return false;
				}

				WNUM wnum;
				if( parse_widevnum(argument, pObj->area, &wnum) )
				{
					OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

					if( !key )
					{
						send_to_char("That object does not exist.\n\r", ch);
						return false;
					}

					if( key->item_type != ITEM_KEY )
					{
						send_to_char("That object is not a key.\n\r", ch);
						return false;
					}

					// TODO: make a list
					FLUID_CON(pObj)->lock->key_wnum = wnum;
					send_to_char("Lock State key set.\n\r", ch);
					return true;
				}
				else if( !str_prefix(argument, "clear") )
				{
					FLUID_CON(pObj)->lock->key_wnum = wnum_zero;
					send_to_char("Lock State key removed.\n\r", ch);
					return true;
				}

				oedit_type_container(ch, "lock key");
				return false;
			}

			if( !str_prefix(arg, "flags") )
			{
				if( !FLUID_CON(pObj)->lock )
				{
					send_to_char("FLUID CONTAINER does not have a lock state.\n\r", ch);
					return false;
				}

				int value = flag_value(lock_flags, argument);

				if( value == NO_FLAG )
				{
					send_to_char("Syntax:  fluid lock flags [flags]\n\r", ch);
					send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
					show_help(ch, "lock");
					return false;
				}

				FLUID_CON(pObj)->lock->flags ^= value;
				send_to_char("Lock State flags changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "pick") )
			{
				if( !FLUID_CON(pObj)->lock )
				{
					send_to_char("FLUID CONTAINER does not have a lock state.\n\r", ch);
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
					send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
					return false;
				}

				FLUID_CON(pObj)->lock->pick_chance = value;
				send_to_char("Lock State pick chance set.\n\r", ch);
				return true;
			}

			oedit_type_fluid_container(ch, "");
			return false;
		}
		else if (!str_prefix(arg, "liquid"))
		{
			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "set"))
			{
				LIQUID *liquid = liquid_lookup(argument);
				if (!IS_VALID(liquid))
				{
					send_to_char("Invalid liquid.\n\r", ch);
					return false;
				}

				FLUID_CON(pObj)->liquid = liquid;
				send_to_char("Liquid set.\n\r", ch);
				return true;
			}
			else if (!str_prefix(arg, "clear"))
			{
				FLUID_CON(pObj)->liquid = NULL;

				send_to_char("Liquid cleared.\n\r", ch);
				return true;
			}

			oedit_type_fluid_container(ch, "");
			return false;
		}
		else if (!str_prefix(arg, "capacity"))
		{
			if (!str_prefix(argument, "unlimited"))
			{
				FLUID_CON(pObj)->capacity = -1;
				FLUID_CON(pObj)->amount = -1;
			}
			else
			{
				int capacity;
				if (is_number(argument) && (capacity = atoi(argument)) >= 0)
				{
					FLUID_CON(pObj)->capacity = capacity;
					if (FLUID_CON(pObj)->amount < 0 || FLUID_CON(pObj)->amount > capacity)
						FLUID_CON(pObj)->amount = capacity;
				}
				else
				{
					send_to_char("Please specify a non-negative number.\n\r", ch);
					return false;
				}
			}

			send_to_char("Fluid Capacity set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "amount"))
		{
			if (FLUID_CON(pObj)->capacity < 0)
			{
				send_to_char("Fluid container has an unlimited capacity.\n\r", ch);
				return false;
			}

			int amount;
			if (is_number(argument) && (amount = atoi(argument)) >= 0)
			{
				if (amount > FLUID_CON(pObj)->capacity)
				{
					if (FLUID_CON(pObj)->capacity > 0)
						sprintf(buf, "Please specify a number from 0 to %d.\n\r", FLUID_CON(pObj)->capacity);
					else
						sprintf(buf, "Amount may only be 0 currently.\n\r");
					
					send_to_char(buf, ch);
					return false;
				}

				FLUID_CON(pObj)->amount = amount;
				send_to_char("Fluid Amount set.\n\r", ch);
				return true;
			}
			else
			{
				if (FLUID_CON(pObj)->capacity > 0)
					sprintf(buf, "Please specify a number from 0 to %d.\n\r", FLUID_CON(pObj)->capacity);
				else
					sprintf(buf, "Amount may only be 0 currently.\n\r");
					
				send_to_char(buf, ch);
				return false;
			}
		}
		else if (!str_prefix(arg, "poison"))
		{
			int poison;

			argument = one_argument(argument, arg);
			if (!is_number(arg) || (poison = atoi(arg)) < 0 || poison > 100)
			{
				send_to_char("Please specify a number from 0 to 100 for poison chance.\n\r", ch);
				return false;
			}

			int refill = 0;
			if (!IS_NULLSTR(argument))
			{
				if (!is_number(argument) || (refill = atoi(argument)) < 0)
				{
					send_to_char("Please specify a non-negative number for poison refill rate.\n\r", ch);
					return false;
				}
			}

			FLUID_CON(pObj)->poison = poison;
			FLUID_CON(pObj)->poison_rate = refill;
			send_to_char("Fluid Container Poison set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "refill"))
		{
			int refill;

			if (!is_number(argument) || (refill = atoi(argument)) < 0)
			{
				send_to_char("Please specify a non-negative number.\n\r", ch);
				return false;
			}

			FLUID_CON(pObj)->refill_rate = refill;
			send_to_char("Fluid Container Refill set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "potion"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  fluid potion {Radd{x <spell name> <level>\n\r", ch);
				send_to_char("         fluid potion {Rremove{x <index>\n\r", ch);
				send_to_char("         fluid potion {Rclear{x\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_quaff_spell(skill))
					{
						send_to_char("That spell cannot be quaffed.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Level range is 1-%d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(FLUID_CON(pObj)->spells, spell);

				sprintf(buf, "Added spell %s, level %d.\n\r",
					get_spell_data_name(spell), spell->level);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "remove"))
			{
				int index;
				if(!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(FLUID_CON(pObj)->spells))
				{
					send_to_char("Syntax:  fluid potion remove {R<index>{x\n\r", ch);
					sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(FLUID_CON(pObj)->spells));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(FLUID_CON(pObj)->spells, index, true);
				sprintf(buf, "FLUID Potion Spell #%d removed.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(FLUID_CON(pObj)->spells) < 1)
				{
					send_to_char("Spell list is empty.\n\r", ch);
					return false;
				}

				list_clear(FLUID_CON(pObj)->spells);
				send_to_char("FLUID Potion Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_fluid_container(ch, "potion");
			return false;
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_FLUID_CONTAINER))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			FLUID_CON(pObj) = new_fluid_container_data();
			send_to_char("FLUID_CONTAINER data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_fluid_container(ch, "");
	return false;
}

OEDIT(oedit_type_food)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (pObj->item_type != ITEM_FOOD || !IS_FOOD(pObj))
	{
		send_to_char("Object's primary type must be FOOD.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  food hunger <hours>\n\r", ch);
		send_to_char("         food fullness <hours>\n\r", ch);
		send_to_char("         food poison <0-100>\n\r", ch);
		send_to_char("         food buff clear\n\r", ch);
		send_to_char("         food buff list\n\r", ch);
		send_to_char("         food buff add <type> <level|auto> <location> <modifier> <duration|auto> <affect bits>\n\r", ch);
		send_to_char("         food buff remove <#>\n\r", ch);
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "hunger"))
	{
		int value;
		if (!is_number(argument) || (value = atoi(argument)) < 0)
		{
			send_to_char("Please specify a non-negative number.\n\r", ch);
			return false;
		}

		FOOD(pObj)->hunger = value;
		send_to_char("FOOD Hunger set.\n\r", ch);
		return true;
	}
	else if (!str_prefix(arg, "fullness"))
	{
		int value;
		if (!is_number(argument) || (value = atoi(argument)) < 0)
		{
			send_to_char("Please specify a non-negative number.\n\r", ch);
			return false;
		}

		FOOD(pObj)->full = value;
		send_to_char("FOOD Fullness set.\n\r", ch);
		return true;
	}
	else if (!str_prefix(arg, "poison"))
	{
		int value;
		if (!is_number(argument) || (value = atoi(argument)) < 0 || value > 100)
		{
			send_to_char("Please specify a number from 0 to 100.\n\r", ch);
			return false;
		}

		FOOD(pObj)->poison = value;
		send_to_char("FOOD Poison set.\n\r", ch);
		return true;
	}
	else if (!str_prefix(arg, "buff"))
	{
		char arg2[MIL];

		argument = one_argument(argument, arg2);

		if (arg2[0] == '\0')
		{
			oedit_type_food(ch, "");
			return false;
		}

		if (!str_prefix(arg2, "list"))
		{
			if (list_size(FOOD(pObj)->buffs) > 0)
			{
				int cnt = 1;
				ITERATOR it;
				FOOD_BUFF_DATA *buff;

				BUFFER *buffer = new_buf();

				sprintf(buf, "{C%-6s %-5s %-15s %-15s %-15s %-10s %s{x\n\r", "Number", "Level", "Where", "Adds", "Modifier", "Duration", "Bits");
				add_buf(buffer, buf);

				sprintf(buf, "{C%-6s %-5s %-15s %-15s %-15s %-10s %s{x\n\r", "------", "-----", "-------", "-------", "--------", "----------", "--------------------");
				add_buf(buffer, buf);

				iterator_start(&it, FOOD(pObj)->buffs);
				while((buff = (FOOD_BUFF_DATA *)iterator_nextdata(&it)))
				{
					char level[MIL];
					char duration[MIL];

					if (buff->level > 0)
						sprintf(level, "%d", buff->level);
					else
						strcpy(level, "auto");

					if (buff->duration > 0)
						sprintf(duration, "%d", buff->duration);
					else
						strcpy(duration, "auto");

					if (buff->where == TO_AFFECTS)
					{
						sprintf(buf, "{C%-6d %-5s %-15s %-15s %-15d %-10s %s{x\n\r", cnt++,
							level,
							flag_string(food_buff_types, buff->where),
							flag_string(apply_flags, buff->location),
							buff->modifier,
							duration,
							bitvector_string(2, buff->bitvector, affect_flags, buff->bitvector2, affect2_flags));
					}
					else
					{
						sprintf(buf, "{C%-6d %-5s %-15s %-15s %-15d %-10s %s{x\n\r", cnt++,
							level,
							flag_string(food_buff_types, buff->where),
							flag_string(apply_flags, buff->location),
							buff->modifier,
							duration,
							imm_bit_name(buff->bitvector));
					}
					add_buf(buffer, buf);
				}
				iterator_stop(&it);

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
				send_to_char("There are no food buffs to list.\n\r", ch);

			return false;
		}

		if (!str_prefix(arg2, "remove"))
		{
			if (list_size(FOOD(pObj)->buffs) < 1)
			{
				send_to_char("There are no food buffs to remove.\n\r", ch);
				return false;
			}

			if (!is_number(argument))
			{
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(FOOD(pObj)->buffs));
				send_to_char(buf, ch);
				return false;
			}

			int buff_no = atoi(argument);
			if (buff_no < 1 || buff_no > list_size(FOOD(pObj)->buffs))
			{
				sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(FOOD(pObj)->buffs));
				send_to_char(buf, ch);
				return false;
			}

			list_remnthlink(FOOD(pObj)->buffs, buff_no, true);

			sprintf(buf, "Food Buff %d removed.\n\r", buff_no);
			send_to_char(buf, ch);
			return true;
		}

		if (!str_prefix(arg2, "clear"))
		{
			if (list_size(FOOD(pObj)->buffs) < 1)
			{
				send_to_char("Food buffs list is empty.\n\r", ch);
				return false;
			}

			list_clear(FOOD(pObj)->buffs);
			send_to_char("Food Buff list cleared.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg2, "add"))
		{
			char arg3[MIL];	// type
			char arg4[MIL]; // level
			char arg5[MIL]; // location
			char arg6[MIL]; // modifier
			char arg7[MIL]; // duration
			//char arg8[MIL]; // bitvector2
			// argument = bitvector

			argument = one_argument(argument, arg3);
			argument = one_argument(argument, arg4);
			argument = one_argument(argument, arg5);
			argument = one_argument(argument, arg6);
			argument = one_argument(argument, arg7);
			
			int type;
			if ((type = stat_lookup(arg3, food_buff_types, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid buff type.\n\r", ch);
				send_to_char("'? food_buff for valid food buff types.\n\r", ch);
				return false;
			}

			int level;
			if (!str_prefix(arg4, "auto"))
				level = 0;
			else if (!is_number(arg4))
			{
				send_to_char("That is not a number.\n\r", ch);
				return false;
			}
			else
				level = atoi(arg4);

			int loc;
			if ((loc = stat_lookup(arg5, apply_flags, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid buff location.\n\r", ch);
				return false;
			}

			if (!is_number(arg6))
			{
				send_to_char("Please specify a number.\n\r", ch);
				return false;
			}

			int mod = atoi(arg6);

			/*
			int bit, bit2;			
			if (type == TO_AFFECTS)
			{
				if (!str_prefix(arg7, "none"))
					bit = 0;
				else if ((bit = flag_value(affect_flags, arg7)) == NO_FLAG)
				{
					send_to_char("Invalid affect flag.\n\r", ch);
					return false;
				}
				
				argument = one_argument(argument, arg8);
				if (!str_prefix(arg8, "none"))
					bit2 = 0;
				else if ((bit2 = flag_value(affect2_flags, arg8)) == NO_FLAG)
				{
					send_to_char("Invalid affect2 flag.\n\r", ch);
					return false;
				}
			}
			else	// Immune, Resist, Vuln
			{
				if (!str_prefix(arg7, "none"))
					bit = 0;

				else if ((bit = flag_value(imm_flags, arg7)) == NO_FLAG)
				{
					send_to_char("Invalid immune flag.\n\r", ch);
					return false;
				}

				bit2 = 0;
			}

			int duration = 0;
			if (!str_prefix(argument, "auto"))
				duration = 0;
			else if (!is_number(argument))
			{
				send_to_char("That is not a number.\n\r", ch);
				return false;
			}
			else
				duration = atoi(argument);
			*/

			int duration = 0;
			if (!str_prefix(arg7, "auto"))
				duration = 0;
			else if (!is_number(arg7))
			{
				send_to_char("That is not a number.\n\r", ch);
				return false;
			}
			else
				duration = atoi(arg7);

			long bits[2];
			if (!bitvector_lookup(argument, 2, bits, affect_flags, affect2_flags))
			{
				send_to_char("Invalid affect flags.\n\r", ch);
				return false;
			}

			FOOD_BUFF_DATA *buff = new_food_buff_data();

			buff->where = type;
			buff->level = level;
			buff->location = loc;
			buff->modifier = mod;
			buff->duration = duration;
			buff->bitvector = bits[0];
			buff->bitvector2 = bits[1];

			list_appendlink(FOOD(pObj)->buffs, buff);
			send_to_char("Food Buff added.\n\r", ch);
			return true;
		}
	}

	oedit_type_food(ch, "");
	return false;	
}

OEDIT(oedit_type_furniture)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_FURNITURE(pObj))
		{
			send_to_char("Syntax:  furniture flags <flags>\n\r", ch);
			send_to_char("         furniture main <compartment#|none>\n\r", ch);
			send_to_char("         furniture compartment list\n\r", ch);
			send_to_char("         furniture compartment clear\n\r", ch);
			send_to_char("         furniture compartment add <name>\n\r", ch);
			send_to_char("         furniture compartment <#> remove\n\r", ch);

			send_to_char("         furniture compartment <#> name <name>\n\r", ch);
			send_to_char("         furniture compartment <#> short <short>\n\r", ch);
			send_to_char("         furniture compartment <#> desc (opens string editor)\n\r", ch);

			send_to_char("         furniture compartment <#> flags <flags>\n\r", ch);
			send_to_char("         furniture compartment <#> maxoccupants <count>\n\r", ch);
			send_to_char("         furniture compartment <#> maxweight <weight>\n\r", ch);

			send_to_char("         furniture compartment <#> standing <flags>\n\r", ch);
			send_to_char("         furniture compartment <#> hanging <flags>\n\r", ch);
			send_to_char("         furniture compartment <#> sitting <flags>\n\r", ch);
			send_to_char("         furniture compartment <#> resting <flags>\n\r", ch);
			send_to_char("         furniture compartment <#> sleeping <flags>\n\r", ch);

			send_to_char("         furniture compartment <#> health <regen rate>\n\r", ch);
			send_to_char("         furniture compartment <#> mana <regen rate>\n\r", ch);
			send_to_char("         furniture compartment <#> move <regen rate>\n\r", ch);

			send_to_char("         furniture compartment <#> lock add\n\r", ch);
			send_to_char("         furniture compartment <#> lock remove\n\r", ch);
			send_to_char("         furniture compartment <#> lock key <widevnum>\n\r", ch);
			send_to_char("         furniture compartment <#> lock key clear\n\r", ch);
			send_to_char("         furniture compartment <#> lock flags [flags]\n\r", ch);
			send_to_char("         furniture compartment <#> lock pick [0-100]\n\r", ch);

			if (pObj->item_type != ITEM_FURNITURE)
				send_to_char("         furniture remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  furniture add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);
	if (IS_FURNITURE(pObj))
	{
		if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_value(furniture_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid furniture flags.\n\r", ch);
				send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
				return false;
			}

			TOGGLE_BIT(FURNITURE(pObj)->flags, value);
			send_to_char("FURNITURE Flags toggled.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "main"))
		{
			if (argument[0] != '\0')
			{
				int mc;
				if (is_number(argument))
				{
					if (list_size(FURNITURE(pObj)->compartments) < 1)
					{
						send_to_char("There are no compartments defined.\n\r", ch);
						return false;
					}

					mc = atoi(argument);
					if (mc < 1 || mc > list_size(FURNITURE(pObj)->compartments))
					{
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(FURNITURE(pObj)->compartments));
						send_to_char(buf, ch);
						return false;
					}
				}
				else if (!str_prefix(argument, "none"))
				{
					mc = 0;
				}
				else
				{
					sprintf(buf, "Please specify a number from 1 to %d, or none.\n\r", list_size(FURNITURE(pObj)->compartments));
					send_to_char(buf, ch);
					return false;
				}

				FURNITURE(pObj)->main_compartment = mc;
				send_to_char("FURNITURE Main Compartment changed.\n\r", ch);
				return true;
			}
			
		}

		if (!str_prefix(arg, "compartment"))
		{
			if (argument[0] != '\0')
			{
				char arg2[MIL];

				argument = one_argument(argument, arg2);
				if(!str_prefix(arg2, "list"))
				{
					if (list_size(FURNITURE(pObj)->compartments) > 0)
					{
						BUFFER *buffer = new_buf();

						add_buf(buffer, "Compartments:\n\r");
						add_buf(buffer, "======================\n\r");

						int cnt = 1;
						ITERATOR it;
						FURNITURE_COMPARTMENT *compartment;
						iterator_start(&it, FURNITURE(pObj)->compartments);
						while((compartment = (FURNITURE_COMPARTMENT *)iterator_nextdata(&it)))
						{
							if (cnt == FURNITURE(pObj)->main_compartment)
								sprintf(buf, " {YCompartment {W%d{Y: {G[MAIN COMPARTMENT]{x\n\r", cnt++);
							else
								sprintf(buf, " {YCompartment {W%d{Y:{x\n\r", cnt++);
							add_buf(buffer, buf);

							sprintf(buf, "   Name: %s\n\r", compartment->name);
							add_buf(buffer, buf);
							sprintf(buf, "   Short Description: %s\n\r", compartment->short_descr);
							add_buf(buffer, buf);
							sprintf(buf, "   Description:\n\r%s\n\r", compartment->description);
							add_buf(buffer, buf);

							sprintf(buf, "   Flags: %s\n\r", flag_string(compartment_flags,compartment->flags));
							add_buf(buffer, buf);
							if (compartment->max_occupants < 0)
								sprintf(buf, "   Max Occupants: Unlimited\n\r");
							else
								sprintf(buf, "   Max Occupants: %d\n\r", compartment->max_occupants);
							add_buf(buffer, buf);
							if (compartment->max_weight < 0)
								sprintf(buf, "   Max Weight: Unlimited\n\r");
							else
								sprintf(buf, "   Max Weight: %d\n\r", compartment->max_weight);
							add_buf(buffer, buf);

							sprintf(buf, "   Standing: %s\n\r", flag_string(furniture_action_flags, compartment->standing));
							add_buf(buffer, buf);
							sprintf(buf, "   Hanging:  %s\n\r", flag_string(furniture_action_flags, compartment->hanging));
							add_buf(buffer, buf);
							sprintf(buf, "   Sitting:  %s\n\r", flag_string(furniture_action_flags, compartment->sitting));
							add_buf(buffer, buf);
							sprintf(buf, "   Resting:  %s\n\r", flag_string(furniture_action_flags, compartment->resting));
							add_buf(buffer, buf);
							sprintf(buf, "   Sleeping: %s\n\r", flag_string(furniture_action_flags, compartment->sleeping));
							add_buf(buffer, buf);

							sprintf(buf, "   Health Regen: %d\n\r", compartment->health_regen);
							add_buf(buffer, buf);
							sprintf(buf, "   Mana Regen:   %d\n\r", compartment->mana_regen);
							add_buf(buffer, buf);
							sprintf(buf, "   Move Regen:   %d\n\r", compartment->move_regen);
							add_buf(buffer, buf);

							if (compartment->lock)
							{
								OBJ_INDEX_DATA *lock_key = get_obj_index(compartment->lock->key_wnum.pArea, compartment->lock->key_wnum.vnum);

								sprintf(buf,"   Lock State:\n\r"
											"    Key:         %s (%ld#%ld)\n\r"
											"    Flags:       %s\n\r"
											"    Pick Chance: %d%%\r",
											lock_key ? lock_key->short_descr : "none",
											compartment->lock->key_wnum.pArea ? compartment->lock->key_wnum.pArea->uid : 0,
											compartment->lock->key_wnum.vnum,
											flag_string(lock_flags, compartment->lock->flags),
											compartment->lock->pick_chance);
								add_buf(buffer, buf);
							}
						}
						iterator_stop(&it);

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
						send_to_char("There are no compartments defined.\n\r", ch);
					return false;
				}

				if(!str_prefix(arg2, "clear"))
				{
					if (list_size(FURNITURE(pObj)->compartments) < 1)
					{
						send_to_char("There are no compartments.\n\r", ch);
						return false;
					}

					list_clear(FURNITURE(pObj)->compartments);
					send_to_char("FURNITURE COmpartment list cleared.\n\r", ch);
					return true;
				}

				if(!str_prefix(arg2, "add"))
				{
					if (argument[0] == '\0')
					{
						send_to_char("Please specify a name.\n\r", ch);
						return false;
					}

					FURNITURE_COMPARTMENT *compartment = new_furniture_compartment();

					free_string(compartment->name);
					compartment->name = str_dup(argument);
					free_string(compartment->short_descr);
					compartment->short_descr = str_dup(argument);

					list_appendlink(FURNITURE(pObj)->compartments, compartment);
					send_to_char("FURNITURE Compartment added.\n\r", ch);
					return true;
				}

				if (is_number(arg2))
				{
					int index = atoi(arg2);
					if (index < 1 || index > list_size(FURNITURE(pObj)->compartments))
					{
						sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(FURNITURE(pObj)->compartments));
						send_to_char(buf, ch);
						return false;
					}

					char arg3[MIL];
					argument = one_argument(argument, arg3);

					if (!str_prefix(arg3, "remove"))
					{
						if (FURNITURE(pObj)->main_compartment == index)
						{
							FURNITURE(pObj)->main_compartment = 0;
						}

						list_remnthlink(FURNITURE(pObj)->compartments, index, true);
						send_to_char("FURNITURE Compartment removed.\n\r", ch);
						return true;
					}

					FURNITURE_COMPARTMENT *compartment = (FURNITURE_COMPARTMENT *)list_nthdata(FURNITURE(pObj)->compartments, index);

					if (!str_prefix(arg3, "name"))
					{
						if (argument[0] == '\0')
						{
							send_to_char("Please specify a name.\n\r", ch);
							return false;
						}

						free_string(compartment->name);
						compartment->name = str_dup(argument);
						send_to_char("FURNITURE Compartment Name set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "short"))
					{
						if (argument[0] == '\0')
						{
							send_to_char("Please specify a short description.\n\r", ch);
							return false;
						}

						free_string(compartment->short_descr);
						compartment->short_descr = str_dup(argument);
						send_to_char("FURNITURE Compartment Short Description set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "description"))
					{
						string_append(ch, &compartment->description);
						return true;
					}

					if (!str_prefix(arg3, "flags"))
					{
						long value;
						if ((value = flag_value(compartment_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid compartment flags.\n\r", ch);
							send_to_char("Type '? compartment' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->flags, value);
						send_to_char("FURNITURE Compartment Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg, "lock"))
					{
						argument = one_argument(argument, arg);

						if( !str_prefix(arg, "add") )
						{
							if( compartment->lock )
							{
								send_to_char("Compartment already has a lock state.\n\r", ch);
								return false;
							}

							compartment->lock = new_lock_state();
							send_to_char("Lock State added.\n\r", ch);
							return true;
						}

						if( !str_prefix(arg, "remove") )
						{
							if( !compartment->lock )
							{
								send_to_char("Compartment does not have a lock state.\n\r", ch);
								return false;
							}

							free_lock_state(compartment->lock);
							compartment->lock = NULL;

							send_to_char("Lock State removed.\n\r", ch);
							return true;
						}

						if( !str_prefix(arg, "key") )
						{
							if( !compartment->lock )
							{
								send_to_char("Compartment does not have a lock state.\n\r", ch);
								return false;
							}

							if( argument[0] == '\0' )
							{
								send_to_char("Syntax:  furniture compartment <#> lock key <widevnum>\n\r", ch);
								send_to_char("         furniture compartment <#> lock key clear\n\r", ch);
								return false;
							}

							WNUM wnum;
							if( parse_widevnum(argument, pObj->area, &wnum) )
							{
								OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

								if( !key )
								{
									send_to_char("That object does not exist.\n\r", ch);
									return false;
								}

								if( key->item_type != ITEM_KEY )
								{
									send_to_char("That object is not a key.\n\r", ch);
									return false;
								}

								// TODO: make a list
								compartment->lock->key_wnum = wnum;
								send_to_char("Lock State key set.\n\r", ch);
								return true;
							}
							else if( !str_prefix(argument, "clear") )
							{
								compartment->lock->key_wnum = wnum_zero;
								send_to_char("Lock State key removed.\n\r", ch);
								return true;
							}

							oedit_type_furniture(ch, "");
							return false;
						}

						if( !str_prefix(arg, "flags") )
						{
							if( !compartment->lock )
							{
								send_to_char("Compartment does not have a lock state.\n\r", ch);
								return false;
							}

							int value = flag_value(lock_flags, argument);

							if( value == NO_FLAG )
							{
								send_to_char("Syntax:  container lock flags [flags]\n\r", ch);
								send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
								show_help(ch, "lock");
								return false;
							}

							compartment->lock->flags ^= value;
							send_to_char("Lock State flags changed.\n\r", ch);
							return true;
						}

						if( !str_prefix(arg, "pick") )
						{
							if( !compartment->lock )
							{
								send_to_char("Compartment does not have a lock state.\n\r", ch);
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
								send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
								return false;
							}

							compartment->lock->pick_chance = value;
							send_to_char("Lock State pick chance set.\n\r", ch);
							return true;
						}

						oedit_type_furniture(ch, "");
						return false;
					}

					if (!str_prefix(arg3, "maxoccupants"))
					{
						int max_occupants;
						if (is_number(argument))
						{
							max_occupants = atoi(argument);
							if (max_occupants < 1)
							{
								send_to_char("Please specify a positive number.\n\r", ch);
								return false;
							}
						}
						else if (!str_prefix(argument, "unlimited"))
						{
							max_occupants = -1;
						}
						else
						{
							send_to_char("Please specify a positive number, or unlimited.\n\r", ch);
							return false;
						}

						compartment->max_occupants = max_occupants;
						send_to_char("FURNITURE Compartment Max Occupants set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "maxweight"))
					{
						int max_weight;
						if (is_number(argument))
						{
							max_weight = atoi(argument);
							if (max_weight < 1)
							{
								send_to_char("Please specify a positive number.\n\r", ch);
								return false;
							}
						}
						else if (!str_prefix(argument, "unlimited"))
						{
							max_weight = -1;
						}
						else
						{
							send_to_char("Please specify a positive number, or unlimited.\n\r", ch);
							return false;
						}

						compartment->max_weight = max_weight;
						send_to_char("FURNITURE Compartment Max Weight set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "standing"))
					{
						long value;
						if ((value = flag_value(furniture_action_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid standing flags.\n\r", ch);
							send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->standing, value);
						send_to_char("FURNITURE Compartment Standing Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "hanging"))
					{
						long value;
						if ((value = flag_value(furniture_action_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid hanging flags.\n\r", ch);
							send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->hanging, value);
						send_to_char("FURNITURE Compartment Hanging Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "sitting"))
					{
						long value;
						if ((value = flag_value(furniture_action_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid sitting flags.\n\r", ch);
							send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->sitting, value);
						send_to_char("FURNITURE Compartment Sitting Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "resting"))
					{
						long value;
						if ((value = flag_value(furniture_action_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid resting flags.\n\r", ch);
							send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->resting, value);
						send_to_char("FURNITURE Compartment Resting Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "sleeping"))
					{
						long value;
						if ((value = flag_value(furniture_action_flags, argument)) == NO_FLAG)
						{
							send_to_char("Invalid sleeping flags.\n\r", ch);
							send_to_char("Type '? furniture' to see list of flags.\n\r", ch);
							return false;
						}

						TOGGLE_BIT(compartment->sleeping, value);
						send_to_char("FURNITURE Compartment Sleeping Flags toggled.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "health"))
					{
						int value;
						if (!is_number(argument) || (value = atoi(argument)) < 0)
						{
							send_to_char("Please specify a non-negative number.\n\r", ch);
							return false;
						}

						compartment->health_regen = value;
						send_to_char("FURNITURE Compartment Health Regen set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "mana"))
					{
						int value;
						if (!is_number(argument) || (value = atoi(argument)) < 0)
						{
							send_to_char("Please specify a non-negative number.\n\r", ch);
							return false;
						}

						compartment->mana_regen = value;
						send_to_char("FURNITURE Compartment Mana Regen set.\n\r", ch);
						return true;
					}

					if (!str_prefix(arg3, "move"))
					{
						int value;
						if (!is_number(argument) || (value = atoi(argument)) < 0)
						{
							send_to_char("Please specify a non-negative number.\n\r", ch);
							return false;
						}

						compartment->move_regen = value;
						send_to_char("FURNITURE Compartment Move Regen set.\n\r", ch);
						return true;
					}
				}
			}
		}

		if (pObj->item_type != ITEM_FURNITURE)
		{
			if (!str_prefix(arg, "remove"))
			{
				free_furniture_data(FURNITURE(pObj));
				FURNITURE(pObj) = NULL;

				send_to_char("FURNITURE removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_FURNITURE))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			FURNITURE(pObj) = new_furniture_data();
			send_to_char("FURNITURE data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_furniture(ch, "");
	return false;
}

OEDIT(oedit_type_ink)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_INK(pObj))
		{
			send_to_char(formatf("Syntax:  ink <1-%d> <type> <amount>\n\r", MAX_INK_TYPES), ch);

			if (pObj->item_type != ITEM_INK)
				send_to_char("         ink remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  ink add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_INK(pObj))
	{
		if (is_number(arg))
		{
			int index = atoi(arg);
			if (index < 1 || index > MAX_INK_TYPES)
			{
				send_to_char(formatf("Syntax:  ink {R<1-%d>{x <type> <amount>\n\r", MAX_INK_TYPES), ch);
				send_to_char(formatf("Please specify a number from 1 to %d.\n\r", MAX_INK_TYPES), ch);
				return false;
			}

			argument = one_argument(argument, arg);

			int type;
			if ((type = stat_lookup(arg, catalyst_types, CATALYST_NONE)) == CATALYST_NONE)
			{
				send_to_char(formatf("Syntax:  ink <1-%d> {R<type>{x <amount>\n\r", MAX_INK_TYPES), ch);
				send_to_char("Invalid catalyst types.  Use '? catalyst_types' to list of valid types.\n\r", ch);
				show_flag_cmds(ch, catalyst_types);
				return false;
			}

			int amount;
			if (!is_number(argument) || (amount = atoi(argument)) < 0)
			{
				send_to_char(formatf("Syntax:  ink <1-%d> <type> {R<amount>{x\n\r", MAX_INK_TYPES), ch);
				send_to_char("Please specify a non-negative number.\n\r", ch);
				return false;
			}

			INK(pObj)->types[index - 1] = type;
			INK(pObj)->amounts[index - 1] = amount;
			send_to_char(formatf("INK Type %d set.\n\r", index), ch);
			return true;
		}
		
		if (pObj->item_type != ITEM_INK)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_ink_data(INK(pObj));
				INK(pObj) = NULL;

				send_to_char("INK type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_INK))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		INK(pObj) = new_ink_data();
		send_to_char("INK type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_ink(ch, "");
	return false;
}

OEDIT(oedit_type_instrument)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_INSTRUMENT(pObj))
		{
			send_to_char("Syntax:  instrument {Rtype{x <type>\n\r", ch);
			send_to_char("         instrument {Rflags{x <flags>\n\r", ch);
			send_to_char("         instrument {Rmana{x <min %> <max %>\n\r", ch);
			send_to_char("         instrument {Rbeats{x <min %> <max %>\n\r", ch);
			send_to_char(formatf("         instrument {Rreservoir{x <1-%d> <type> <capacity>[ <amount>]\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
			
			if (pObj->item_type != ITEM_INSTRUMENT)
				send_to_char("         instrument remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  instrument add\n\r", ch);
		return false;
	}

	if (IS_INSTRUMENT(pObj))
	{
		char arg[MIL];

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "beats"))
		{
			argument = one_argument(argument, arg);

			int min;
			if (!is_number(arg) || (min = atoi(arg)) < 1 || min > 200)
			{
				send_to_char("Syntax:  instrument beats {R<min %>{x <max %>\n\r", ch);
				send_to_char("Please specify a number from 1 to 200.\n\r", ch);
				return false;
			}

			int max;
			if (!is_number(argument) || (max = atoi(argument)) < 1 || max > 200)
			{
				send_to_char("Syntax:  instrument beats <min %> {R<max %>{x\n\r", ch);
				send_to_char("Please specify a number from 1 to 200.\n\r", ch);
				return false;
			}

			// Swap them silently
			if (min > max)
			{
				int m = min;
				min = max;
				max = m;
			}

			// TODO: Add limitations (can't have it too big, can't have it too small)
			INSTRUMENT(pObj)->beats_min = min;
			INSTRUMENT(pObj)->beats_max = max;
			send_to_char("INSTRUMENT Mana range set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;

			if ((value = flag_value(instrument_flags, argument)) == NO_FLAG)
			{
				send_to_char("Syntax:  instrument flags {R<flags>{x\n\r", ch);
				send_to_char("Invalid instrument flags.  Use 'instrument_flags' for list of valid flags.\n\r", ch);
				show_flag_cmds(ch, instrument_flags);
				return false;
			}

			TOGGLE_BIT(INSTRUMENT(pObj)->flags, value);
			send_to_char("INSTRUMENT flags changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "mana"))
		{
			argument = one_argument(argument, arg);

			int min;
			if (!is_number(arg) || (min = atoi(arg)) < 1 || min > 200)
			{
				send_to_char("Syntax:  instrument mana {R<min %>{x <max %>\n\r", ch);
				send_to_char("Please specify a number from 1 to 200.\n\r", ch);
				return false;
			}

			int max;
			if (!is_number(argument) || (max = atoi(argument)) < 1 || max > 200)
			{
				send_to_char("Syntax:  instrument mana <min %> {R<max %>{x\n\r", ch);
				send_to_char("Please specify a number from 1 to 200.\n\r", ch);
				return false;
			}

			// Swap them silently
			if (min > max)
			{
				int m = min;
				min = max;
				max = m;
			}

			// TODO: Add limitations (can't have it too big, can't have it too small)
			INSTRUMENT(pObj)->mana_min = min;
			INSTRUMENT(pObj)->mana_max = max;
			send_to_char("INSTRUMENT Mana range set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "reservoir"))
		{
			argument = one_argument(argument, arg);

			int index;
			if (!is_number(arg) || (index = atoi(arg)) < 1 || index > INSTRUMENT_MAX_CATALYSTS)
			{
				send_to_char(formatf("Syntax: reservoir {R<1-%d>{x <type> <capacity>[ <amount>]\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
				send_to_char(formatf("Please specify a number from 1 to %d.\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
				return false;
			}

			argument = one_argument(argument, arg);

			int type;
			if ((type = stat_lookup(arg, catalyst_types, CATALYST_NONE)) == CATALYST_NONE)
			{
				send_to_char(formatf("Syntax: reservoir <1-%d> {R<type>{x <capacity>[ <amount>]\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
				send_to_char("Invalid catalyst type.  Use '? catalyst_types' to see list of valid types.\n\r", ch);
				show_flag_cmds(ch, catalyst_types);
				return false;
			}

			argument = one_argument(argument, arg);

			int capacity;
			if (!is_number(arg) || (capacity = atoi(arg)) < 0)
			{
				send_to_char(formatf("Syntax: reservoir <1-%d> <type> {R<capacity>{x[ <amount>]\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
				send_to_char("Please specify a non-negative number.\n\r", ch);
				return false;
			}

			int amount = capacity;
			if (capacity > 0)
			{
				if (argument[0])
				{
					if (!is_number(argument) || (amount = atoi(argument)) < 0 || amount > capacity)
					{
						send_to_char(formatf("Syntax: reservoir <1-%d> <type> <capacity> {R<amount>{x\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
						send_to_char(formatf("Please specify a number from 0 to %d.\n\r", capacity), ch);
						return false;
					}
				}
			}
			else if (argument[0])
			{
				send_to_char(formatf("Syntax: reservoir <1-%d> <type> <capacity> {R<amount>{x\n\r", INSTRUMENT_MAX_CATALYSTS), ch);
				send_to_char("Please omit {Wamount{x when {Wcapacity{x is 0.\n\r", ch);
				return false;
			}

			INSTRUMENT(pObj)->reservoirs[index - 1].type = type;
			INSTRUMENT(pObj)->reservoirs[index - 1].capacity = capacity;
			INSTRUMENT(pObj)->reservoirs[index - 1].amount = amount;
			send_to_char(formatf("INSTRUMENT Reservoir %d set.\n\r", index), ch);
			return true;
		}

		if (!str_prefix(arg, "type"))
		{
			int value;

			if ((value = stat_lookup(argument, instrument_types, INSTRUMENT_NONE)) == INSTRUMENT_NONE)
			{
				send_to_char("Syntax:  instrument type {R<type>{x\n\r", ch);
				send_to_char("Invalid instrument type.  Use 'instrument_types' for list of valid types.\n\r", ch);
				show_flag_cmds(ch, instrument_types);
				return false;
			}

			INSTRUMENT(pObj)->type = value;
			send_to_char("INSTRUMENT type changed.\n\r", ch);
			return true;
		}
		

		if (pObj->item_type != ITEM_INSTRUMENT)
		{
			if (!str_prefix(argument, "remove"))
			{
				free_instrument_data(INSTRUMENT(pObj));
				INSTRUMENT(pObj) = NULL;

				send_to_char("INSTRUMENT settings removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(argument, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_INSTRUMENT))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}
			
			INSTRUMENT(pObj) = new_instrument_data();
			send_to_char("INSTRUMENT type added.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_instrument(ch, "");
	return false;
}

OEDIT(oedit_type_jewelry)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_JEWELRY(pObj))
		{
			send_to_char("Syntax:  jewelry maxmana <mana>\n\r", ch);
			send_to_char("         jewelry spell add <name> <level>\n\r", ch);
			send_to_char("         jewelry spell remove <#>\n\r", ch);
			send_to_char("         jewelry spell clear\n\r", ch);
			
			if (pObj->item_type != ITEM_JEWELRY)
				send_to_char("         jewelry remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  jewelry add\n\r", ch);
		return false;
	}

	if (IS_JEWELRY(pObj))
	{
		JEWELRY_DATA *jewelry = JEWELRY(pObj);
		char arg[MIL];

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "maxmana"))
		{
			int mana;
			if (!is_number(argument) || (mana = atoi(argument)) < 0)
			{
				send_to_char("Please specify a non-negative number for maximum mana.\n\r", ch);
				return false;
			}

			JEWELRY(pObj)->max_mana = mana;
			send_to_char("JEWELRY Maximum Mana changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "spell"))
		{

			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_equip_spell(skill))
					{
						send_to_char("That spell cannot be equipped.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					send_to_char(formatf("Level range is 1-%d.\n\r", MAX_CLASS_LEVEL), ch);
					return false;
				}

				// Spells must be unique on jewelry
				if (olc_has_spell(jewelry->spells, skill))
				{
					send_to_char("Spell already on the jewelry.\n\r", ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(jewelry->spells, spell);

				send_to_char(formatf("Added spell %s, level %d.\n\r", get_spell_data_name(spell), spell->level), ch);
				return true;
			}

			if (!str_prefix(arg, "remove"))
			{
				if (list_size(jewelry->spells) < 1)
				{
					send_to_char("There are no spells on the jewelry data.\n\r", ch);
					return false;
				}

				int index;
				if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(jewelry->spells))
				{
					send_to_char(formatf("Please specify an index from 1 to %d.\n\r", list_size(jewelry->spells)), ch);
					return false;
				}

				list_remnthlink(jewelry->spells, index, true);
				send_to_char("JEWELRY spell removed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg, "clear"))
			{
				list_clear(jewelry->spells);
				send_to_char("JEWELRY spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_jewelry(ch, "");
			return false;
		}


		if (pObj->item_type != ITEM_JEWELRY)
		{
			if (!str_prefix(argument, "remove"))
			{
				free_jewelry_data(JEWELRY(pObj));
				JEWELRY(pObj) = NULL;

				send_to_char("JEWELRY settings removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(argument, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_JEWELRY))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}
			
			JEWELRY(pObj) = new_jewelry_data();
			send_to_char("JEWELRY type added.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_jewelry(ch, "");
	return false;
}

OEDIT(oedit_type_light)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_LIGHT(pObj))
		{
			send_to_char("Syntax:  light flags <flags>\n\r", ch);
			send_to_char("         light duration <duration|infinite>\n\r", ch);
			
			if (pObj->item_type != ITEM_LIGHT)
				send_to_char("         light remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  light add\n\r", ch);
		return false;
	}

	if (IS_LIGHT(pObj))
	{
		char arg[MIL];

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "duration"))
		{
			int duration;

			if (!str_prefix(argument, "infinite"))
				duration = -1;
			else if (!is_number(argument) || (duration = atoi(argument)) < 1)
			{
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			LIGHT(pObj)->duration = duration;
			send_to_char("LIGHT duration changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			long value;

			if ((value = flag_value(light_flags, argument)) == NO_FLAG)
			{
				send_to_char("Invalid light flags.\n\r", ch);
				show_help(ch, "light");
				return false;
			}

			TOGGLE_BIT(LIGHT(pObj)->flags, value);
			send_to_char("LIGHT flags changed.\n\r", ch);
			return true;
		}

		if (pObj->item_type != ITEM_LIGHT)
		{
			if (!str_prefix(argument, "remove"))
			{
				free_light_data(LIGHT(pObj));
				LIGHT(pObj) = NULL;

				send_to_char("LIGHT settings removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(argument, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_LIGHT))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}
			
			LIGHT(pObj) = new_light_data();
			send_to_char("LIGHT type added.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_light(ch, "");
	return false;
}


OEDIT(oedit_type_map)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_MAP(pObj))
		{
			send_to_char("Syntax:  map location set <wuid> <x> <y>\n\r", ch);
			send_to_char("         map location set here\n\r", ch);
			send_to_char("         map location clear\n\r", ch);
			send_to_char("         map waypoints list\n\r", ch);
			send_to_char("         map waypoints add <wilds> <south> <east>[ <name>]\n\r", ch);
			send_to_char("         map waypoints delete <#>\n\r", ch);

			if (pObj->item_type != ITEM_MAP)
				send_to_char("         map remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  map add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_MAP(pObj))
	{
		if (!str_prefix(arg, "location"))
		{
			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "set"))
			{
				argument = one_argument(argument, arg);
	
				if (!str_prefix(arg, "here"))
				{
					if (ch->in_room->wilds)
					{
						MAP(pObj)->wuid = ch->in_room->wilds->uid;
						MAP(pObj)->x = ch->in_room->x;
						MAP(pObj)->y = ch->in_room->y;

						send_to_char(formatf("MAP Location set to %s (%ld) at (%ld, %ld).\n\r", ch->in_room->wilds->name, ch->in_room->wilds->uid, ch->in_room->x, ch->in_room->y), ch);
						return true;
					}

					send_to_char("You must be in a wilderness room to do this.\n\r", ch);
					return false;
				}

				long wuid = 0;
				if(!is_number(arg) || (wuid = atol(arg)) < 1)
				{
					send_to_char("Please provide a positive number.\n\r", ch);
					return false;
				}

				WILDS_DATA *wilds = get_wilds_from_uid(NULL, wuid);
				if (!wilds)
				{
					send_to_char("No such wilds exists.\n\r", ch);
					return false;
				}

				argument = one_argument(argument, arg);

				long x;
				if (!is_number(arg) || (x = atol(arg)) < 0 || x > wilds->map_size_x)
				{
					send_to_char(formatf("Please provide a number from 0 to %d.\n\r", wilds->map_size_x), ch);
					return false;
				}

				long y;
				if (!is_number(arg) || (y = atol(arg)) < 0 || x > wilds->map_size_y)
				{
					send_to_char(formatf("Please provide a number from 0 to %d.\n\r", wilds->map_size_y), ch);
					return false;
				}

				MAP(pObj)->wuid = wuid;
				MAP(pObj)->x = x;
				MAP(pObj)->y = y;
				send_to_char(formatf("MAP Location set to %s (%ld) at (%ld, %ld).\n\r", wilds->name, wilds->uid, x, y), ch);
				return true;
			}
			else if (!str_prefix(arg, "clear"))
			{
				MAP(pObj)->wuid = 0;
				MAP(pObj)->x = 0;
				MAP(pObj)->y = 0;
				send_to_char("MAP Location cleared.\n\r", ch);
				return true;
			}

			send_to_char("Syntax:  map location set <wilds uid> <x> <y>\n\r", ch);
			send_to_char("         map location set here\n\r", ch);
			send_to_char("         map location clear\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "waypoints"))
		{
			argument = one_argument(argument, arg);

			if( !str_prefix(arg, "list") )
			{
				if (list_size(MAP(pObj)->waypoints) > 0)
				{
					int cnt = 0;
					ITERATOR wit;
					WAYPOINT_DATA *wp;
					WILDS_DATA *wilds;

					BUFFER *buffer = new_buf();

					add_buf(buffer, "{BCartographer Waypoints:{x\n\r\n\r");
					add_buf(buffer, "{B     [     Wilderness     ] [ South ] [  East ] [        Name        ]{x\n\r");
					add_buf(buffer, "{B======================================================================={x\n\r");

					iterator_start(&wit, MAP(pObj)->waypoints);
					while( (wp = (WAYPOINT_DATA *)iterator_nextdata(&wit)) )
					{
						wilds = get_wilds_from_uid(NULL, wp->w);

						char *wname = wilds ? wilds->name : "{D(null){x";

						int wwidth = get_colour_width(wname) + 20;

						sprintf(buf, "{B%3d{b)  {W%-*.*s    {G%5d     %5d    {Y%s{x\n\r",
							++cnt,
							wwidth, wwidth, wname,
							wp->y, wp->x, wp->name);

						add_buf(buffer, buf);
					}

					iterator_stop(&wit);

					add_buf(buffer, "\n\r");

					page_to_char(buffer->string, ch);

					free_buf(buffer);
				}
				else
					send_to_char("No waypoints to display.\n\r", ch);

				return false;
			}

			if( !str_prefix(arg, "add") )
			{
				char arg2[MIL];
				char arg3[MIL];
				char arg4[MIL];

				long uid;
				WILDS_DATA *wilds;
				int x, y;

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);
				argument = one_argument(argument, arg4);

				if( !is_number(arg2) || !is_number(arg3) || !is_number(arg4) )
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				uid = atol(arg2);
				wilds = get_wilds_from_uid(NULL, uid);
				if( !wilds )
				{
					send_to_char("No such wilderness.\n\r", ch);
					return false;
				}

				y = atoi(arg3);
				x = atoi(arg4);

				if( y < 0 || y >= wilds->map_size_y )
				{
					sprintf(buf, "South coordinate is out of bounds.  Please limit from 0 to %d.\n\r", wilds->map_size_y - 1);
					send_to_char(buf, ch);
					return false;
				}

				if( x < 0 || x >= wilds->map_size_x )
				{
					sprintf(buf, "East coordinate is out of bounds.  Please limit from 0 to %d.\n\r", wilds->map_size_x - 1);
					send_to_char(buf, ch);
					return false;
				}

				WAYPOINT_DATA *wp = new_waypoint();

				free_string(wp->name);
				wp->name = nocolour(argument);
				wp->w = uid;
				wp->x = x;
				wp->y = y;

				list_appendlink(MAP(pObj)->waypoints, wp);
				send_to_char("Waypoint added.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "delete") )
			{
				int value;

				if( !is_number(argument) )
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				if( list_size(MAP(pObj)->waypoints) < 1 )
				{
					send_to_char("There are no waypoints to delete.\n\r", ch);
					return false;
				}

				value = atoi(argument);
				if( value < 1 || value > list_size(MAP(pObj)->waypoints) )
				{
					send_to_char("No such waypoint.\n\r", ch);
					return false;
				}

				list_remnthlink(MAP(pObj)->waypoints, value, true);
				send_to_char("Waypoint deleted.\n\r", ch);
				return true;
			}

			oedit_type_map(ch, "");
			return false;
		}

		if (pObj->item_type != ITEM_MAP)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_map_data(MAP(pObj));
				MAP(pObj) = NULL;

				send_to_char("MAP type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_MAP))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		MAP(pObj) = new_map_data();
		send_to_char("MAP type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_map(ch, "");
	return false;
}


OEDIT(oedit_type_mist)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_MIST(pObj))
		{
			send_to_char("Syntax:  mist obscure <mobiles|objects|room> <0-100>\n\r", ch);
			send_to_char("         mist duration <duration|infinite>\n\r", ch);
			
			if (pObj->item_type != ITEM_MIST)
				send_to_char("         mist remove\n\r", ch);
		}
		else
			send_to_char("Syntax:  mist add\n\r", ch);
		return false;
	}

	if (IS_MIST(pObj))
	{
		char arg[MIL];

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "obscure"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  mist obscure mobiles <0-100>\n\r", ch);
				send_to_char("         mist obscure objects <0-100>\n\r", ch);
				send_to_char("         mist obscure room <0-100>\n\r", ch);
				return false;
			}

			int obscure;

			argument = one_argument(argument, arg);
			if (!is_number(argument) || (obscure = atoi(argument)) < 0 || obscure > 100)
			{
				send_to_char("Please provide a percentage (0 to 100).\n\r", ch);
				return false;
			}

			if (!str_prefix(arg, "mobiles"))
			{
				MIST(pObj)->obscure_mobs = obscure;
				send_to_char("MIST Obscure Mobiles changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg, "objects"))
			{
				MIST(pObj)->obscure_objs = obscure;
				send_to_char("MIST Obscure Objects changed.\n\r", ch);
				return true;
			}

			if (!str_prefix(arg, "room"))
			{
				MIST(pObj)->obscure_room = obscure;
				send_to_char("MIST Obscure Room changed.\n\r", ch);
				return true;
			}

			oedit_type_mist(ch, "obscure");
			return false;
		}

		PARSE_PERCENT(arg,"icy",MIST(pObj)->icy,"MIST Icy Chance Changed.\n\r")
		PARSE_PERCENT(arg,"fiery",MIST(pObj)->fiery,"MIST Fiery Chance Changed.\n\r")
		PARSE_PERCENT(arg,"acidic",MIST(pObj)->acidic,"MIST Acidic Chance Changed.\n\r")
		PARSE_PERCENT(arg,"stink",MIST(pObj)->stink,"MIST Stinking Chance Changed.\n\r")
		PARSE_PERCENT(arg,"wither",MIST(pObj)->wither,"MIST Withering Chance Changed.\n\r")
		PARSE_PERCENT(arg,"toxic",MIST(pObj)->toxic,"MIST Toxic Chance Changed.\n\r")
		PARSE_PERCENT(arg,"shock",MIST(pObj)->shock,"MIST Shock Chance Changed.\n\r")
		PARSE_PERCENT(arg,"fog",MIST(pObj)->fog,"MIST Fog Chance Changed.\n\r")
		PARSE_PERCENT(arg,"sleep",MIST(pObj)->sleep,"MIST Sleep Chance Changed.\n\r")

		if (pObj->item_type != ITEM_MIST)
		{
			if (!str_prefix(argument, "remove"))
			{
				free_mist_data(MIST(pObj));
				MIST(pObj) = NULL;

				send_to_char("MIST settings removed.\n\r", ch);
				return true;
			}
		}
	}
	else
	{
		if (!str_prefix(argument, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_MIST))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}
			
			MIST(pObj) = new_mist_data();
			send_to_char("MIST type added.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_mist(ch, "");
	return false;
}


OEDIT(oedit_type_money)
{
	OBJ_INDEX_DATA *pObj;

	if (argument[0] != '\0')
	{
		EDIT_OBJ(ch, pObj);

		if (pObj->item_type != ITEM_MONEY || !IS_MONEY(pObj))
		{
			send_to_char("Object's primary type must be MONEY.\n\r", ch);
			return false;
		}

		char arg[MIL];

		argument = one_argument(argument, arg);

		if (is_number(arg) && is_number(argument))
		{
			int silver = atoi(arg);
			int gold = atoi(argument);

			if (silver < 0 || gold < 0)
			{
				send_to_char("Values may not be negative.\n\r", ch);
				return false;
			}

			MONEY(pObj)->silver = silver;
			MONEY(pObj)->gold = gold;

			send_to_char("MONEY settings set.\n\r", ch);
			return true;
		}

		send_to_char("That is not a number.\n\r", ch);
		return false;
	}

	send_to_char("Syntax:  money <silver> <gold>\n\r", ch);
	return false;
}


OEDIT(oedit_type_portal)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_PORTAL(pObj))
		{
			send_to_char("Syntax:  portal name <name>\n\r", ch);
			send_to_char("         portal short <description>\n\r", ch);
			send_to_char("         portal charge <#charge|infinite>\n\r", ch);
			send_to_char("         portal flags <flags>\n\r", ch);
			send_to_char("         portal exit <exit flags>\n\r", ch);
			send_to_char("         portal type <type>\n\r", ch);
			switch(PORTAL(pObj)->type)
			{
				case GATETYPE_ENVIRONMENT: break;
				case GATETYPE_NORMAL:
					send_to_char("         portal room <widevnum>\n\r", ch);
					break;
				case GATETYPE_WILDS:
					send_to_char("         portal wilds <wuid> <x> <y>\n\r", ch);
					break;
				case GATETYPE_WILDSRANDOM:
					send_to_char("         portal wildsrandom <wuid> <min-x> <min-y> <max-x> <max-y>\n\r", ch);
					break;
				case GATETYPE_AREARANDOM:
					send_to_char("         portal arearandom <auid|current>\n\r", ch);
					break;
				case GATETYPE_REGIONRANDOM:
					send_to_char("         portal regionrandom <auid|current>[ <region|default>]\n\r", ch);
					break;
				case GATETYPE_SECTIONRANDOM:
					send_to_char("         portal sectionrandom current\n\r", ch);
					send_to_char("         portal sectionrandom generated|ordinal <section#>\n\r", ch);
					break;
				case GATETYPE_INSTANCERANDOM: break;
				case GATETYPE_DUNGEONRANDOM: break;
				case GATETYPE_AREARECALL:
					send_to_char("         portal arearecall <auid|current>\n\r", ch);
					break;
				case GATETYPE_REGIONRECALL:
					send_to_char("         portal regionrecall <auid|current>[ <region#|default>]\n\r", ch);
					break;
				case GATETYPE_DUNGEON:
					send_to_char("         portal dungeon <local vnum> floor <floor#>\n\r", ch);
					send_to_char("         portal dungeon <local vnum> room <special room#>\n\r", ch);
					send_to_char("         portal dungeon <local vnum> default\n\r", ch);
					break;
				case GATETYPE_INSTANCE:
					// TODO: Complete
					break;
				case GATETYPE_RANDOM: break;
				case GATETYPE_DUNGEONFLOOR:
					send_to_char("         portal dungeonfloor <local vnum> <floor#|default>\n\r", ch);
					break;
				case GATETYPE_BLUEPRINT_SECTION_MAZE:
					send_to_char("         portal sectionmaze current <x> <y>\n\r", ch);
					send_to_char("         portal sectionmaze generated|ordinal <section#> <x> <y>\n\r", ch);
					break;
				case GATETYPE_BLUEPRINT_SPECIAL:
					send_to_char("         portal instancespecial <special room#>\n\r", ch);
					break;
				case GATETYPE_DUNGEON_FLOOR_SPECIAL:
					send_to_char("         portal floorspecial current <special room#>\n\r", ch);
					send_to_char("         portal floorspecial generated|ordinal <floor #> <special room#>\n\r", ch);
					break;
				case GATETYPE_DUNGEON_SPECIAL:
					send_to_char("         portal dungeonspecial <special room#>\n\r", ch);
					break;
			}

			send_to_char("         portal lock add\n\r", ch);
			send_to_char("         portal lock remove\n\r", ch);
			send_to_char("         portal lock key <widevnum>\n\r", ch);
			send_to_char("         portal lock key clear\n\r", ch);
			send_to_char("         portal lock flags [flags]\n\r", ch);
			send_to_char("         portal lock pick [0-100]\n\r", ch);

			send_to_char("         portal spell clear\n\r", ch);
			send_to_char("         portal spell add <spell name> <spell level> <random>\n\r", ch);
			send_to_char("         portal spell remove <#>\n\r", ch);

			if (pObj->item_type != ITEM_PORTAL)
				send_to_char("         portal remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  portal add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);
	
	if (IS_PORTAL(pObj))
	{
		if (!str_prefix(arg, "name"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please specify a name.\n\r", ch);
				return false;
			}

			free_string(PORTAL(pObj)->name);
			PORTAL(pObj)->name = str_dup(argument);

			send_to_char("PORTAL NAME set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "short"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Please specify a short description.\n\r", ch);
				return false;
			}

			free_string(PORTAL(pObj)->short_descr);
			PORTAL(pObj)->short_descr = str_dup(argument);

			send_to_char("PORTAL SHORT DESCRIPTION set.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "flags"))
		{
			long value;
			if ((value = flag_lookup(argument, portal_flags)) == NO_FLAG)
			{
				send_to_char("Invalid portal flag.\n\r", ch);
				send_to_char("Please use '? portal' for list of valid flags.\n\r", ch);
				show_help(ch, "portal");
				return false;
			}

			TOGGLE_BIT(PORTAL(pObj)->flags, value);
			send_to_char("PORTAL FLAGS toggled.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "exit"))
		{
			long value;
			if ((value = flag_lookup(argument, portal_exit_flags)) == NO_FLAG)
			{
				send_to_char("Invalid portal exit flag.\n\r", ch);
				send_to_char("Please use '? portal_exit' for list of valid flags.\n\r", ch);
				show_help(ch, "portal_exit");
				return false;
			}

			TOGGLE_BIT(PORTAL(pObj)->exit, value);
			send_to_char("PORTAL EXIT FLAGS toggled.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "type"))
		{
			int type;
			if ((type = stat_lookup(argument, portal_gatetype, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid portal type.\n\r", ch);
				send_to_char("Please use '? portal_type' for list of valid types.\n\r", ch);
				show_help(ch, "portal_type");
				return false;
			}

			PORTAL(pObj)->type = type;
			PORTAL(pObj)->params[0] = 0;
			PORTAL(pObj)->params[1] = 0;
			PORTAL(pObj)->params[2] = 0;
			PORTAL(pObj)->params[3] = 0;
			PORTAL(pObj)->params[4] = 0;

			send_to_char("PORTAL TYPE set.\n\r", ch);

			switch(PORTAL(pObj)->type)
			{
				case GATETYPE_ENVIRONMENT: break;
				case GATETYPE_NORMAL:
					send_to_char("Syntax:  portal room <widevnum>\n\r", ch);
					break;
				case GATETYPE_WILDS:
					send_to_char("Syntax:  portal wilds <wuid> <x> <y>\n\r", ch);
					break;
				case GATETYPE_WILDSRANDOM:
					send_to_char("Syntax:  portal wildsrandom <wuid> <min-x> <min-y> <max-x> <max-y>\n\r", ch);
					break;
				case GATETYPE_AREARANDOM:
					send_to_char("Syntax:  portal arearandom <auid|current>\n\r", ch);
					break;
				case GATETYPE_REGIONRANDOM:
					send_to_char("Syntax:  portal regionrandom <auid|current>[ <region|default>]\n\r", ch);
					break;
				case GATETYPE_SECTIONRANDOM:
					send_to_char("Syntax:  portal sectionrandom current\n\r", ch);
					send_to_char("         portal sectionrandom generated|ordinal <section#>\n\r", ch);
					break;
				case GATETYPE_INSTANCERANDOM: break;
				case GATETYPE_DUNGEONRANDOM: break;
				case GATETYPE_AREARECALL:
					send_to_char("Syntax:  portal arearecall <auid|current>\n\r", ch);
					break;
				case GATETYPE_REGIONRECALL:
					send_to_char("Syntax:  portal regionrecall <auid|current>[ <region#|default>]\n\r", ch);
					break;
				case GATETYPE_DUNGEON:
					send_to_char("Syntax:  portal dungeon <local vnum> floor <floor#>\n\r", ch);
					send_to_char("         portal dungeon <local vnum> room <special room#>\n\r", ch);
					send_to_char("         portal dungeon <local vnum> default\n\r", ch);
					break;
				case GATETYPE_INSTANCE:
					// TODO: Complete
					break;
				case GATETYPE_RANDOM: break;
				case GATETYPE_DUNGEONFLOOR:
					send_to_char("Syntax:  portal dungeonfloor <local vnum> <floor#|default>\n\r", ch);
					break;
				case GATETYPE_BLUEPRINT_SECTION_MAZE:
					send_to_char("Syntax:  portal sectionmaze current <x> <y>\n\r", ch);
					send_to_char("         portal sectionmaze generated|ordinal <section#> <x> <y>\n\r", ch);
					break;
				case GATETYPE_BLUEPRINT_SPECIAL:
					send_to_char("Syntax:  portal instancespecial <special room#>\n\r", ch);
					break;
				case GATETYPE_DUNGEON_FLOOR_SPECIAL:
					send_to_char("Syntax:  portal floorspecial current <special room#>\n\r", ch);
					send_to_char("         portal floorspecial generated|ordinal <floor #> <special room#>\n\r", ch);
					break;
				case GATETYPE_DUNGEON_SPECIAL:
					send_to_char("Syntax:  portal dungeonspecial <special room#>\n\r", ch);
					break;
			}

			return true;
		}
		else if (!str_prefix(arg, "lock"))
		{
			argument = one_argument(argument, arg);

			if( !str_prefix(arg, "add") )
			{
				if( PORTAL(pObj)->lock )
				{
					send_to_char("PORTAL already has a lock state.\n\r", ch);
					return false;
				}

				PORTAL(pObj)->lock = new_lock_state();
				send_to_char("Lock State added.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "remove") )
			{
				if( !PORTAL(pObj)->lock )
				{
					send_to_char("PORTAL does not have a lock state.\n\r", ch);
					return false;
				}


				free_lock_state(PORTAL(pObj)->lock);
				PORTAL(pObj)->lock = NULL;

				send_to_char("Lock State removed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "key") )
			{
				if( !PORTAL(pObj)->lock )
				{
					send_to_char("PORTAL does not have a lock state.\n\r", ch);
					return false;
				}

				if( argument[0] == '\0' )
				{
					send_to_char("Syntax:  portal lock key <widevnum>\n\r", ch);
					send_to_char("         portal lock key clear\n\r", ch);
					return false;
				}

				WNUM wnum;
				if( parse_widevnum(argument, pObj->area, &wnum) )
				{
					OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);

					if( !key )
					{
						send_to_char("That object does not exist.\n\r", ch);
						return false;
					}

					if( key->item_type != ITEM_KEY )
					{
						send_to_char("That object is not a key.\n\r", ch);
						return false;
					}

					// TODO: make a list
					PORTAL(pObj)->lock->key_wnum = wnum;
					send_to_char("Lock State key set.\n\r", ch);
					return true;
				}
				else if( !str_prefix(argument, "clear") )
				{
					PORTAL(pObj)->lock->key_wnum = wnum_zero;
					send_to_char("Lock State key removed.\n\r", ch);
					return true;
				}

				oedit_type_portal(ch, "lock key");
				return false;
			}

			if( !str_prefix(arg, "flags") )
			{
				if( !PORTAL(pObj)->lock )
				{
					send_to_char("PORTAL does not have a lock state.\n\r", ch);
					return false;
				}

				int value = flag_value(lock_flags, argument);

				if( value == NO_FLAG )
				{
					send_to_char("Syntax:  portal lock flags [flags]\n\r", ch);
					send_to_char("See \"? lock\" for list of flags\n\r\n\r", ch);
					show_help(ch, "lock");
					return false;
				}

				PORTAL(pObj)->lock->flags ^= value;
				send_to_char("Lock State flags changed.\n\r", ch);
				return true;
			}

			if( !str_prefix(arg, "pick") )
			{
				if( !PORTAL(pObj)->lock )
				{
					send_to_char("PORTAL does not have a lock state.\n\r", ch);
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
					send_to_char("Pick chance must be from 0 to 100.\n\r", ch);
					return false;
				}

				PORTAL(pObj)->lock->pick_chance = value;
				send_to_char("Lock State pick chance set.\n\r", ch);
				return true;
			}

			oedit_type_portal(ch, "");
			return false;
		}
		else if (!str_prefix(arg, "spell"))
		{
			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "add"))
			{
				char arg2[MIL];
				char arg3[MIL];
				SKILL_DATA *skill;

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);

				skill = get_skill_data(arg2);
				if (!IS_VALID(skill) || !is_skill_spell(skill))
				{
					send_to_char("That's not a spell.\n\r", ch);
					return false;
				}

				ITERATOR sit;
				SPELL_DATA *spell_tmp;
				iterator_start(&sit, PORTAL(pObj)->spells);
				while((spell_tmp = (SPELL_DATA *)iterator_nextdata(&sit)))
				{
					if (spell_tmp->skill == skill)
						break;
				}
				iterator_stop(&sit);

				if (spell_tmp)
				{
					send_to_char("That spell is already on the object.\n\r", ch);
					return false;
				}	

				if (!is_number(arg3) || !is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int level = atoi(arg3);
				if (level < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Please specify a level from 1 to %d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				int chance = atoi(argument);
				if (chance < 1 || chance > 100)
				{
					send_to_char("Please specify a number from 1 to 100.\n\r", ch);
					return false;
				}

				SPELL_DATA *spell = new_spell();
				spell->skill = skill;
				spell->level = level;
				spell->repop = chance;
				spell->next = NULL;

				list_appendlink(PORTAL(pObj)->spells, spell);

			    sprintf(buf, "PORTAL Spell %s, level %d, random %d added.\n\r",
					get_spell_data_name(spell), spell->level, spell->repop);
			    send_to_char(buf, ch);
				return true;
			}
			else if (!str_prefix(arg, "remove"))
			{
				if (!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int n = atoi(argument);
				list_remnthlink(PORTAL(pObj)->spells, n, true);
				send_to_char("PORTAL Spell removed.\n\r", ch);
				return true;
			}
			else if (!str_prefix(arg, "clear"))
			{
				list_clear(PORTAL(pObj)->spells);

				send_to_char("PORTAL Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_portal(ch, "");
			return false;
		}
		else if (pObj->item_type != ITEM_PORTAL)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_portal_data(PORTAL(pObj));
				PORTAL(pObj) = NULL;

				send_to_char("PORTAL type removed.\n\r\n\r", ch);
				return true;
			}
		}

		sent_bool ret = __oedit_type_portal_subtype(ch, pObj, argument);
		if (ret != TRISTATE_UNDEF) return ret;
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_PORTAL))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		PORTAL(pObj) = new_portal_data();
		send_to_char("PORTAL type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_portal(ch, "");
	return false;
}


OEDIT(oedit_type_scroll)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_SCROLL(pObj))
		{
			send_to_char("Syntax:  scroll maxmana <#mana|unlimited>\n\r", ch);
			send_to_char("         scroll flags <flags>\n\r", ch);
			send_to_char("         scroll spell clear\n\r", ch);
			send_to_char("         scroll spell add <spell name> <spell level> <random>\n\r", ch);
			send_to_char("         scroll spell remove <#>\n\r", ch);

			if (pObj->item_type != ITEM_SCROLL)
				send_to_char("         scroll remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  scroll add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_SCROLL(pObj))
	{
		if (!str_prefix(arg, "maxmana"))
		{
			int mana;
			if (!str_prefix(argument, "unlimited"))
			{
				mana = -1;
			}
			else if (!is_number(argument) || (mana = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  scroll maxmana {R<#mana>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			SCROLL(pObj)->max_mana = mana;
			send_to_char("SCROLL Max Mana set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "flags"))
		{
			int value;
			if ((value = flag_value(scroll_flags, argument)) == NO_FLAG)
			{
				send_to_char("Syntax:  scroll flags <flags>\n\r", ch);
				send_to_char("Invalid scroll flags.  Use '? scroll' for list of available flags.\n\r", ch);
				return false;
			}
		}

		if (!str_prefix(arg, "spell"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  scroll spell {Radd{x <spell name> <level>\n\r", ch);
				send_to_char("         scroll spell {Rremove{x <index>\n\r", ch);
				send_to_char("         scroll spell {Rclear{x\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_recite_spell(skill))
					{
						send_to_char("That spell cannot be recited from a scroll.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Level range is 1-%d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(SCROLL(pObj)->spells, spell);

				sprintf(buf, "Added spell %s, level %d.\n\r",
					get_spell_data_name(spell), spell->level);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "remove"))
			{
				int index;
				if(!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(SCROLL(pObj)->spells))
				{
					send_to_char("Syntax:  scroll spell remove {R<index>{x\n\r", ch);
					sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(SCROLL(pObj)->spells));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(SCROLL(pObj)->spells, index, true);
				sprintf(buf, "SCROLL Spell #%d removed.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(SCROLL(pObj)->spells) < 1)
				{
					send_to_char("Spell list is empty.\n\r", ch);
					return false;
				}

				list_clear(SCROLL(pObj)->spells);
				send_to_char("SCROLL Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_scroll(ch, "spell");
			return false;
		}
		
		if (pObj->item_type != ITEM_SCROLL)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_scroll_data(SCROLL(pObj));
				SCROLL(pObj) = NULL;

				send_to_char("SCROLL type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_SCROLL))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		SCROLL(pObj) = new_scroll_data();
		send_to_char("SCROLL type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_scroll(ch, "");
	return false;
}

OEDIT(oedit_type_sextant)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_SEXTANT(pObj))
		{
			send_to_char("Syntax:  sextant accuracy <percent>\n\r", ch);

			if (pObj->item_type != ITEM_SEXTANT)
				send_to_char("         sextant remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  sextant add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_SEXTANT(pObj))
	{
		if (!str_prefix(arg, "accuracy"))
		{
			int16_t accuracy;
			if (!is_number(argument) || (accuracy = atoi(argument)) < 0 || accuracy > 100)
			{
				send_to_char("Please provide a percent.\n\r", ch);
				return false;
			}

			SEXTANT(pObj)->accuracy = accuracy;
			send_to_char("SEXTANT Accuracy changed.\n\r", ch);
			return true;
		}

		if (pObj->item_type != ITEM_SEXTANT)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_sextant_data(SEXTANT(pObj));
				SEXTANT(pObj) = NULL;

				send_to_char("SEXTANT type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_SEXTANT))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		SEXTANT(pObj) = new_sextant_data();
		send_to_char("SEXTANT type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_sextant(ch, "");
	return false;
}


bool olc_can_touch_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_TOUCH, TRIGSLOT_SPELL) != NULL;
	else
		return skill->touch_fun != NULL;
}

OEDIT(oedit_type_tattoo)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_TATTOO(pObj))
		{
			send_to_char("Syntax:  tattoo touches <#touches|unlimited>\n\r", ch);
			send_to_char("         tattoo fading <%chance>[ <#rate>]\n\r", ch);
			send_to_char("         tattoo spell clear\n\r", ch);
			send_to_char("         tattoo spell add <spell name> <spell level> <random>\n\r", ch);
			send_to_char("         tattoo spell remove <#>\n\r", ch);

			if (pObj->item_type != ITEM_TATTOO)
				send_to_char("         tattoo remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  tattoo add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_TATTOO(pObj))
	{
		if (!str_prefix(arg, "touches"))
		{
			int touches;
			if (!str_prefix(argument, "unlimited"))
			{
				touches = -1;
			}
			else if (!is_number(argument) || (touches = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  tattoo touches {R<#touches>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			TATTOO(pObj)->touches = touches;
			send_to_char("TATTOO Touches set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "fading"))
		{
			argument = one_argument(argument, arg);

			int fading_chance;
			if (!is_number(arg) || (fading_chance = atoi(arg)) < 0 || fading_chance > 100)
			{
				send_to_char("Syntax:  tattoo fading {R<%chance>{x[ <#rate>]\n\r", ch);
				send_to_char("Please provide a number from 0 to 100.\n\r", ch);
				return false;
			}

			int fading_rate = 0;
			if (argument[0] != '\0')
			{
				if (!is_number(argument) || (fading_rate = atoi(argument)) < 0)
				{
					send_to_char("Syntax:  tattoo fading <%chance> {R<#rate>{x\n\r", ch);
					send_to_char("Please provide a non-negative number.\n\r", ch);
					return false;
				}
			}

			TATTOO(pObj)->fading_chance = fading_chance;
			TATTOO(pObj)->fading_rate = fading_rate;
			send_to_char("TATTOO Fading set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "spell"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  tattoo spell {Radd{x <spell name> <level>\n\r", ch);
				send_to_char("         tattoo spell {Rremove{x <index>\n\r", ch);
				send_to_char("         tattoo spell {Rclear{x\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if (!olc_can_touch_spell(skill))
					{
						send_to_char("That spell cannot be used in a tattoo.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Level range is 1-%d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(TATTOO(pObj)->spells, spell);

				sprintf(buf, "Added spell %s, level %d.\n\r",
					get_spell_data_name(spell), spell->level);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "remove"))
			{
				int index;
				if(!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(TATTOO(pObj)->spells))
				{
					send_to_char("Syntax:  tattoo spell remove {R<index>{x\n\r", ch);
					sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(TATTOO(pObj)->spells));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(TATTOO(pObj)->spells, index, true);
				sprintf(buf, "TATTOO Spell #%d removed.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(TATTOO(pObj)->spells) < 1)
				{
					send_to_char("Spell list is empty.\n\r", ch);
					return false;
				}

				list_clear(TATTOO(pObj)->spells);
				send_to_char("TATTOO Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_tattoo(ch, "spell");
			return false;
		}
		
		if (pObj->item_type != ITEM_TATTOO)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_tattoo_data(TATTOO(pObj));
				TATTOO(pObj) = NULL;

				send_to_char("TATTOO type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_TATTOO))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		TATTOO(pObj) = new_tattoo_data();
		send_to_char("TATTOO type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_tattoo(ch, "");
	return false;
}

OEDIT(oedit_type_telescope)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_TELESCOPE(pObj))
		{
			send_to_char("Syntax:  telescope distance <distance>\n\r", ch);
			send_to_char("         telescope mindistance <distance>\n\r", ch);
			send_to_char("         telescope maxdistance <distance+>\n\r", ch);
			send_to_char("         telescope bonusview <bonus>\n\r", ch);
			send_to_char("         telescope heading <0-359|none>\n\r", ch);

			if (pObj->item_type != ITEM_TELESCOPE)
				send_to_char("         telescope remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  telescope add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_TELESCOPE(pObj))
	{
		if (!str_prefix(arg, "distance"))
		{
			int16_t distance;
			if (!is_number(argument) || (distance = atoi(argument)) < 0)
			{
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			if (distance < TELESCOPE(pObj)->min_distance)
			{
				send_to_char("Distance must be at least the Minimum Distance.\n\r", ch);
				return false;
			}

			if (distance > TELESCOPE(pObj)->max_distance)
			{
				send_to_char("Distance must be atmost the Maximum Distance.\n\r", ch);
				return false;
			}

			TELESCOPE(pObj)->distance = distance;
			send_to_char("TELESCOPE Distance changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "maxdistance"))
		{
			int max;
			if (!is_number(argument) || (max = atoi(argument)) < 1)
			{
				send_to_char("Please provide a positive number.\n\r", ch);
				return false;
			}

			if (max < TELESCOPE(pObj)->min_distance)
			{
				send_to_char("Minimum Distance cannot exceed Maximum Distance.\n\r", ch);
				return false;
			}

			TELESCOPE(pObj)->max_distance = max;
			if (max < TELESCOPE(pObj)->distance)
				TELESCOPE(pObj)->distance = max;
			send_to_char("TELESCOPE Maximum Distance changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "mindistance"))
		{
			int min;
			if (!is_number(argument) || (min = atoi(argument)) < 0)
			{
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			if (min > TELESCOPE(pObj)->max_distance)
			{
				send_to_char("Minimum Distance cannot exceed Maximum Distance.\n\r", ch);
				return false;
			}

			TELESCOPE(pObj)->min_distance = min;
			if (min > TELESCOPE(pObj)->distance)
				TELESCOPE(pObj)->distance = min;
			send_to_char("TELESCOPE Minimum Distance changed.\n\r", ch);
			return true;
		}


		if (!str_prefix(arg, "bonusview"))
		{
			int bonus;
			if (!is_number(argument) || (bonus = atoi(argument)) < 0)
			{
				send_to_char("Please provide a nonnegative number.\n\r", ch);
				return false;
			}

			TELESCOPE(pObj)->bonus_view = bonus;
			send_to_char("TELESCOPE Bonus View changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "heading"))
		{
			int heading;
			if (!str_prefix(argument, "none"))
				heading = -1;
			else if (!is_number(argument) || (heading = atoi(argument)) < 0 || heading > 359)
			{
				send_to_char("Please provide a number from 0 to 359, or none.\n\r", ch);
				return false;
			}

			TELESCOPE(pObj)->heading = heading;
			send_to_char("TELESCOPE Heading changed.\n\r", ch);
			return true;
		}

		if (pObj->item_type != ITEM_TELESCOPE)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_telescope_data(TELESCOPE(pObj));
				TELESCOPE(pObj) = NULL;

				send_to_char("TELESCOPE type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_TELESCOPE))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		TELESCOPE(pObj) = new_telescope_data();
		send_to_char("TELESCOPE type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_telescope(ch, "");
	return false;
}


OEDIT(oedit_type_wand)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_WAND(pObj))
		{
			send_to_char("Syntax:  wand charges <#charges>\n\r", ch);
			send_to_char("         wand maxcharges <#charges|unlimited>\n\r", ch);
			send_to_char("         wand maxmana <mana>\n\r", ch);
			send_to_char("         wand recharge <#rate>\n\r", ch);
			send_to_char("         wand spell clear\n\r", ch);
			send_to_char("         wand spell add <spell name> <spell level>\n\r", ch);
			send_to_char("         wand spell remove <#>\n\r", ch);

			if (pObj->item_type != ITEM_WAND)
				send_to_char("         wand remove\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  wand add\n\r", ch);
		}
		return false;
	}

	char buf[MSL];
	char arg[MIL];

	argument = one_argument(argument, arg);

	if (IS_WAND(pObj))
	{
		if (!str_prefix(arg, "charges"))
		{
			if (WAND(pObj)->max_charges < 0)
			{
				send_to_char("Wand already has unlimited charges.\n\r", ch);
				return false;
			}

			if (WAND(pObj)->max_charges == 0)
			{
				send_to_char("Wand has no charges.\n\r", ch);
				return false;
			}

			int charges;
			if (!is_number(argument) || (charges = atoi(argument)) < 0 || charges > WAND(pObj)->max_charges)
			{
				send_to_char("Syntax:  wand charges {R<#charges>{x\n\r", ch);
				sprintf(buf, "Please provide a number from 0 to %d.\n\r", WAND(pObj)->max_charges);
				send_to_char(buf, ch);
				return false;
			}

			WAND(pObj)->charges = charges;
			send_to_char("WAND Charges set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "maxcharges"))
		{
			int max;
			if (!str_prefix(argument, "unlimited"))
			{
				max = -1;
			}
			else if (!is_number(argument) || (max = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  wand maxcharges {R<#charges>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			WAND(pObj)->max_charges = max;
			if (max < 0)
				WAND(pObj)->charges = -1;
			else if(WAND(pObj)->charges < 0 || max < WAND(pObj)->charges)
				WAND(pObj)->charges = max;
			send_to_char("WAND Max Charges set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "maxmana"))
		{
			int mana;
			if (!is_number(argument) || (mana = atoi(argument)) < 0)
			{
				send_to_char("Please specify a non-negative number for maximum mana.\n\r", ch);
				return false;
			}

			WAND(pObj)->max_mana = mana;
			send_to_char("WAND Maximum Mana set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "recharge"))
		{
			if (WAND(pObj)->max_charges < 0)
			{
				send_to_char("Wand already has unlimited charges.\n\r", ch);
				return false;
			}

			if (WAND(pObj)->max_charges == 0)
			{
				send_to_char("Wand has no charges.\n\r", ch);
				return false;
			}

			int rate;
			if (!is_number(argument) || (rate = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  wand recharge {R<#rate>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			WAND(pObj)->recharge_time = rate;
			send_to_char("WAND Recharge Rate set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "spell"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  wand spell {Radd{x <spell name> <level>\n\r", ch);
				send_to_char("         wand spell {Rremove{x <index>\n\r", ch);
				send_to_char("         wand spell {Rclear{x\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if(!olc_can_zap_spell(skill))
					{
						send_to_char("That spell cannot be used in wand.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Level range is 1-%d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(WAND(pObj)->spells, spell);

				sprintf(buf, "Added spell %s, level %d.\n\r",
					get_spell_data_name(spell), spell->level);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "remove"))
			{
				int index;
				if(!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(WAND(pObj)->spells))
				{
					send_to_char("Syntax:  wand spell remove {R<index>{x\n\r", ch);
					sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(WAND(pObj)->spells));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(WAND(pObj)->spells, index, true);
				sprintf(buf, "WAND Spell #%d removed.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(WAND(pObj)->spells) < 1)
				{
					send_to_char("Spell list is empty.\n\r", ch);
					return false;
				}

				list_clear(WAND(pObj)->spells);
				send_to_char("WAND Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_wand(ch, "spell");
			return false;
		}
		
		if (pObj->item_type != ITEM_WAND)
		{
			if(!str_prefix(arg, "remove"))
			{
				free_wand_data(WAND(pObj));
				WAND(pObj) = NULL;

				send_to_char("WAND type removed.\n\r\n\r", ch);
				return true;
			}
		}
	}
	else if(!str_prefix(arg, "add"))
	{
		if (!obj_index_can_add_item_type(pObj, ITEM_WAND))
		{
			send_to_char("You cannot add this item type to this object.\n\r", ch);
			return false;
		}
		
		WAND(pObj) = new_wand_data();
		send_to_char("WAND type added.\n\r\n\r", ch);
		return true;
	}

	oedit_type_wand(ch, "");
	return false;
}


OEDIT( oedit_type_weapon )
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		if (IS_WEAPON(pObj))
		{
			send_to_char("Syntax:  weapon class <weapon class>\n\r", ch);
			send_to_char("         weapon attack <#> type <attack type>\n\r", ch);
			send_to_char("         weapon attack <#> dice <number> <size>[ <bonus>]\n\r", ch);
			send_to_char("         weapon attack <#> flags <flags>\n\r", ch);
			send_to_char("         weapon attack <#> reset\n\r", ch);
			send_to_char("         weapon maxmana <mana>   (for imbuing)\n\r", ch);
			send_to_char("         weapon charges <#charges>\n\r", ch);
			send_to_char("         weapon maxcharges <#charges|unlimited>\n\r", ch);
			send_to_char("         weapon recharge <#rate>\n\r", ch);
			send_to_char("         weapon ammo <type>\n\r", ch);
			send_to_char("         weapon range <distance>\n\r", ch);
			send_to_char("         weapon spell add <name> <level>\n\r", ch);
			send_to_char("         weapon spell remove <index>\n\r", ch);
			send_to_char("         weapon spell clear\n\r", ch);

			if (pObj->item_type != ITEM_WEAPON)
			{
				send_to_char("         weapon remove\n\r", ch);
			}
		}
		else
		{
			send_to_char("Syntax:  weapon add\n\r", ch);
		}
		return false;
	}

	char arg[MIL];
	char buf[MSL];
	argument = one_argument(argument, arg);
	if (IS_WEAPON(pObj))
	{
		if (!str_prefix(arg, "ammo"))
		{
			int type;
			if ((type = stat_lookup(argument, ammo_types, NO_FLAG)) == NO_FLAG)
			{
				send_to_char("Invalid ammo type.  Use '? ammo' for list of valid types.\n\r", ch);
				show_flag_cmds(ch, ammo_types);
				return false;
			}

			WEAPON(pObj)->ammo = type;
			send_to_char("WEAPON Ammo type set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "charges"))
		{
			if (WEAPON(pObj)->max_charges < 0)
			{
				send_to_char("Weapon already has unlimited charges.\n\r", ch);
				return false;
			}

			if (WEAPON(pObj)->max_charges == 0)
			{
				send_to_char("Weapon has no charges.\n\r", ch);
				return false;
			}

			int charges;
			if (!is_number(argument) || (charges = atoi(argument)) < 0 || charges > WEAPON(pObj)->max_charges)
			{
				send_to_char("Syntax:  weapon charges {R<#charges>{x\n\r", ch);
				sprintf(buf, "Please provide a number from 0 to %d.\n\r", WEAPON(pObj)->max_charges);
				send_to_char(buf, ch);
				return false;
			}

			WEAPON(pObj)->charges = charges;
			send_to_char("WEAPON Charges set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "class"))
		{
			int clazz;
			if ((clazz = stat_lookup(argument, weapon_class, WEAPON_UNKNOWN)) == WEAPON_UNKNOWN)
			{
				send_to_char("Invalid weapon class.  Use '? wclass' for valid types.\n\r", ch);
				show_flag_cmds(ch, weapon_class);
				return false;
			}

			WEAPON(pObj)->weapon_class = clazz;
			// Reset the Ammo
			WEAPON(pObj)->ammo = AMMO_NONE;
			send_to_char("Weapon Class changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "maxcharges"))
		{
			int max;
			if (!str_prefix(argument, "unlimited"))
			{
				max = -1;
			}
			else if (!is_number(argument) || (max = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  weapon maxcharges {R<#charges>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			WEAPON(pObj)->max_charges = max;
			if (max < 0)
				WEAPON(pObj)->charges = -1;
			else if(WEAPON(pObj)->charges < 0 || max < WEAPON(pObj)->charges)
				WEAPON(pObj)->charges = max;
			send_to_char("WEAPON Max Charges set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "maxmana"))
		{
			int mana;
			if (!is_number(argument) || (mana = atoi(argument)) < 0)
			{
				send_to_char("Please specify a non-negative number for maximum mana.\n\r", ch);
				return false;
			}

			WEAPON(pObj)->max_mana = mana;
			send_to_char("WEAPON Maximum Mana set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "range"))
		{
			int range;
			if (!is_number(argument) || (range = atoi(argument)) < 0)
			{
				send_to_char("Please specify a non-negative number.\n\r", ch);
				return false;
			}

			WEAPON(pObj)->range = range;
			send_to_char("WEAPON Range set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "recharge"))
		{
			if (WEAPON(pObj)->max_charges < 0)
			{
				send_to_char("Weapon already has unlimited charges.\n\r", ch);
				return false;
			}

			if (WEAPON(pObj)->max_charges == 0)
			{
				send_to_char("Weapon has no charges.\n\r", ch);
				return false;
			}

			int rate;
			if (!is_number(argument) || (rate = atoi(argument)) < 0)
			{
				send_to_char("Syntax:  weapon recharge {R<#rate>{x\n\r", ch);
				send_to_char("Please provide a non-negative number.\n\r", ch);
				return false;
			}

			WEAPON(pObj)->recharge_time = rate;
			send_to_char("WEAPON Recharge Rate set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "attack"))
		{
			int index;
			argument = one_argument(argument, arg);
			if (!is_number(arg) || (index = atoi(arg)) < 1 || index > MAX_ATTACK_POINTS)
			{
				send_to_char(formatf("Please specify an index from 1 to %d.\n\r", MAX_ATTACK_POINTS), ch);
				return false;
			}

			WEAPON_ATTACK_POINT *attack = &(WEAPON(pObj)->attacks[index-1]);

			argument = one_argument(argument, arg);

			if (!str_prefix(arg, "dice"))
			{
				int number;
				argument = one_argument(argument, arg);
				if (!is_number(arg) || (number = atoi(arg)) < 1)
				{
					send_to_char("Please specify a positive number of dice.\n\r", ch);
					return false;
				}

				int size;
				argument = one_argument(argument, arg);
				if (!is_number(arg) || (size = atoi(arg)) < 1)
				{
					send_to_char("Please specify a positive dice size.\n\r", ch);
					return false;
				}

				int bonus = 0;
				if (argument[0] != '\0')
				{
					if (!is_number(argument) || (bonus = atoi(argument)) < 1)
					{
						send_to_char("Please specify a positive dice bonus.\n\r", ch);
						return false;
					}
				}

				attack->damage.number = number;
				attack->damage.size = size;
				attack->damage.bonus = bonus;
				send_to_char(formatf("WEAPON Attack #%d dice changed.\n\r", index), ch);
				return true;
			}

			if (!str_prefix(arg, "flags"))
			{
				long value;
				if ((value = flag_value(weapon_type2, argument)) == NO_FLAG)
				{
					send_to_char("Invalid attack flag.  Use '? wtype' for list of valid flags.\n\r", ch);
					show_flag_cmds(ch, weapon_type2);
					return false;
				}

				TOGGLE_BIT(attack->flags, value);
				send_to_char(formatf("WEAPON Attack #%d flags toggled.\n\r", index), ch);
				return true;
			}

			if (!str_prefix(arg, "reset"))
			{
				attack->type = -1;
				attack->damage.number = 0;
				attack->damage.size = 0;
				attack->damage.bonus = 0;
				attack->flags = 0;

				send_to_char(formatf("WEAPON Attack #%d reset.\n\r", index), ch);
				return true;
			}

			if (!str_prefix(arg, "type"))
			{
				int type;
				if ((type = attack_lookup(argument)) == 0)
				{
					send_to_char("Invalid attack type.  Use '? weapon' for list of valid types.\n\r", ch);
					show_damlist(ch);
					return false;
				}

				attack->type = type;
				send_to_char(formatf("WEAPON Attack #%d type changed.\n\r", index), ch);
				return true;
			}

			send_to_char("Syntax:  weapon attack <#> type <attack type>\n\r", ch);
			send_to_char("         weapon attack <#> dice <number> <size>[ <bonus>]\n\r", ch);
			send_to_char("         weapon attack <#> flags <flags>\n\r", ch);
			send_to_char("         weapon attack <#> reset\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "remove"))
		{
			if (pObj->item_type != ITEM_WEAPON)
			{
				
				free_weapon_data(WEAPON(pObj));
				WEAPON(pObj) = NULL;
				send_to_char("WEAPON data removed from object.\n\r", ch);
				return true;
			}
		}

		if (!str_prefix(arg, "spell"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  weapon spell {Radd{x <spell name> <level>\n\r", ch);
				send_to_char("         weapon spell {Rremove{x <index>\n\r", ch);
				send_to_char("         weapon spell {Rclear{x\n\r", ch);
				return false;
			}

			argument = one_argument(argument, arg);
			if (!str_prefix(arg, "add"))
			{
				char name[MIL];
				SPELL_DATA *spell;
				int level;
				SKILL_DATA *skill;

				argument = one_argument(argument, name);

				if (IS_NULLSTR(name))
				{
					send_to_char("Please specify a name.\n\r", ch);
					return false;
				}
				else
				{
					skill = get_skill_data(name);
					if (!IS_VALID(skill) || !is_skill_spell(skill))
					{
						send_to_char("That's not a spell.\n\r", ch);
						return false;
					}

					if(!olc_can_zap_spell(skill))
					{
						send_to_char("That spell cannot be used in weapon.\n\r", ch);
						return false;
					}
				}

				if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
				{
					sprintf(buf, "Level range is 1-%d.\n\r", MAX_CLASS_LEVEL);
					send_to_char(buf, ch);
					return false;
				}

				spell			= new_spell();
				spell->skill	= skill;
				spell->level	= level;
				spell->repop	= 100;
				spell->next		= NULL;

				list_appendlink(WEAPON(pObj)->spells, spell);

				sprintf(buf, "Added spell %s, level %d.\n\r",
					get_spell_data_name(spell), spell->level);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "remove"))
			{
				int index;
				if(!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(WEAPON(pObj)->spells))
				{
					send_to_char("Syntax:  weapon spell remove {R<index>{x\n\r", ch);
					sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(WEAPON(pObj)->spells));
					send_to_char(buf, ch);
					return false;
				}

				list_remnthlink(WEAPON(pObj)->spells, index, true);
				sprintf(buf, "WEAPON Spell #%d removed.\n\r", index);
				send_to_char(buf, ch);
				return true;
			}
			else if(!str_prefix(arg, "clear"))
			{
				if (list_size(WEAPON(pObj)->spells) < 1)
				{
					send_to_char("Spell list is empty.\n\r", ch);
					return false;
				}

				list_clear(WEAPON(pObj)->spells);
				send_to_char("WEAPON Spells cleared.\n\r", ch);
				return true;
			}

			oedit_type_weapon(ch, "spell");
			return false;
		}

	}
	else
	{
		if (!str_prefix(arg, "add"))
		{
			if (!obj_index_can_add_item_type(pObj, ITEM_WEAPON))
			{
				send_to_char("You cannot add this item type to this object.\n\r", ch);
				return false;
			}

			WEAPON(pObj) = new_weapon_data();
			send_to_char("WEAPON data added to object.\n\r\n\r", ch);
			return true;
		}
	}

	oedit_type_weapon(ch, "");
	return false;
}


OEDIT(oedit_material)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

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

    pObj->material = material;
	pObj->fragility = material->fragility;	// Reset the fragility

    send_to_char("Material set.\n\r", ch);
    return true;
}


OEDIT(oedit_level)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
	send_to_char("Syntax:  level [number]\n\r", ch);
	return false;
    }

	int level = atoi(argument);
	if (level < 0 || level > MAX_CLASS_LEVEL)
	{
		send_to_char(formatf("Please specify a level from 0 to %d.\n\r", MAX_CLASS_LEVEL), ch);
		return false;
	}

    pObj->level = level;
	send_to_char("Level set.\n\r", ch);

    pObj->points = (int)3 * pObj->level / 10;
    send_to_char(formatf("This object is now assigned {Y%d{x points.\n\r", pObj->points), ch);

    /* auto setting weapon dice stuff */
    if (IS_WEAPON(pObj))
    {
        set_weapon_dice(pObj);
		send_to_char("Damage dice set.\n\r", ch);
    }


    /* auto setting armour stuff */
    if (IS_ARMOR(pObj))
    {
		set_armour(pObj);
		send_to_char("Armour class set.\n\r", ch);
    }

    return true;
}

OEDIT(oedit_immortal)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (!str_cmp(argument,"on")) {
		pObj->immortal = true;
		send_to_char("Object is now an Immortal object.\n\r", ch);
	} else if (!str_cmp(argument,"off")) {
		if (IS_SET(pObj->area->area_flags, AREA_IMMORTAL))
		{
			send_to_char("Area is flagged as Immortal; cannot unset the immortal state.\n\r", ch);
			return false;
		}
		pObj->immortal = false;
		send_to_char("Object is no longer an immortal object.\n\r", ch);
	} else {
		send_to_char("Syntax:  immortal on|off\n\r", ch);
		return false;
	}

	return true;
}

OEDIT(oedit_class)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  class <class name>\n\r", ch);
		return false;
	}

	if(!str_prefix(argument, "none"))
	{
		pObj->clazz = NULL;
		send_to_char("Class cleared.\n\r", ch);
	}
	else
	{
		CLASS_DATA *clazz = get_class_data(argument);
		if (!IS_VALID(clazz))
		{
			send_to_char("No such class by that name.\n\r", ch);
			return false;
		}

		pObj->clazz = clazz;
		send_to_char("Class changed.\n\r", ch);
	}
	return true;
}

OEDIT(oedit_class_type)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  classtype <class type>\n\r", ch);
		return false;
	}

	if (!str_prefix(argument, "none"))
	{
		pObj->clazz_type = CLASS_NONE;
		send_to_char("Class Type cleared.\n\r", ch);
	}
	else
	{
		int type;
		if ((type = stat_lookup(argument, class_types, NO_FLAG)) == NO_FLAG)
		{
			send_to_char("Invalid class type.  Use '? classtypes' for valid list.\n\r", ch);
			show_flag_cmds(ch, class_types);
			return false;
		}

		pObj->clazz_type = type;
		send_to_char("Class Type changed.\n\r", ch);
	}
	return true;
}

OEDIT(oedit_race)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  race add <race name>\n\r", ch);
		send_to_char("         race remove <#>\n\r", ch);
		send_to_char("Syntax:  race clear\n\r", ch);
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "add"))
	{
		RACE_DATA *race = get_race_data(argument);
		if (!IS_VALID(race))
		{
			send_to_char("No such race by that name.\n\r", ch);
			return false;
		}

		if (!race->playable)
		{
			send_to_char("Only playable races allowed.\n\r", ch);
			return false;
		}

		if (list_contains(pObj->race, race, NULL))
		{
			send_to_char("That race is already in the list.\n\r", ch);
			return false;
		}

		list_appendlink(pObj->race, race);
		send_to_char("Race added.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		if (list_size(pObj->race) < 1)
		{
			send_to_char("Race list is already empty.\n\r", ch);
			return false;
		}

		int index;
		if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(pObj->race))
		{
			send_to_char(formatf("Please provide a number from 1 to %d.\n\r", list_size(pObj->race)), ch);
			return false;
		}

		list_remnthlink(pObj->race, index, false);
		send_to_char("Race removed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		list_clear(pObj->race);
		send_to_char("Race list cleared.\n\r", ch);		
		return true;
	}

	oedit_race(ch, "");
	return false;
}


OEDIT(oedit_condition)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0'
    && (value = atoi (argument)) >= 0
    && (value <= 100))
    {
        EDIT_OBJ(ch, pObj);

        pObj->condition = value;
        send_to_char("Condition set.\n\r", ch);

        return true;
    }

    send_to_char("Syntax:  condition [number]\n\r"
                  "Where number can range from 0 (ruined) to 100 (perfect).\n\r"
,
                  ch);
    return false;
}


OEDIT(oedit_fragility)
{
    OBJ_INDEX_DATA *pObj;
    bool set = false;

    if (argument[0] != '\0')
    {
	EDIT_OBJ(ch, pObj);

	if (!str_cmp(argument, "Solid"))
	{
	    if (!str_cmp(pObj->imp_sig, "none")
	    && !IS_IMPLEMENTOR(ch))
	    {
		send_to_char("You can't do this without an IMP's "
			"permission.\n\r", ch);
		return false;
	    }

	    pObj->fragility = OBJ_FRAGILE_SOLID;
	    set = true;
	    use_imp_sig(NULL, pObj);
	}

	if (!str_cmp(argument, "Strong"))
	{
	    pObj->fragility = OBJ_FRAGILE_STRONG;
	    set = true;
	}

	if (!str_cmp(argument, "Normal"))
	{
	    pObj->fragility = OBJ_FRAGILE_NORMAL;
	    set = true;
	}

	if (!str_cmp(argument, "Weak"))
	{
	    pObj->fragility = OBJ_FRAGILE_WEAK;
	    set = true;
	}

	if (set)
	{
	    send_to_char("Fragility set.\n\r", ch);
	    return true;
	}
    }

    send_to_char("Syntax:  fragility  Solid|Strong|Normal|Weak\n\r"
	    "Fragility.\n\r",
	    ch);
    return false;
}


OEDIT(oedit_allowed_fixed)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0'
    && (value = atoi (argument)) >= 0
    && (value <= 100))
    {
	EDIT_OBJ(ch, pObj);

	pObj->times_allowed_fixed = value;
	send_to_char("Allowed Fixed Set.\n\r", ch);

	return true;
    }

    send_to_char("Syntax:  Allowed_fixed [number]\n\r"
		  "Number of times a person can fix the object.\n\r",
		  ch);
    return false;
}


OEDIT (oedit_addoprog)
{
	struct trigger_type *tt;
    int value, slot;
    PROG_LIST *list;
    SCRIPT_DATA *code;
  OBJ_INDEX_DATA *pObj;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_OBJ(ch, pObj);
  argument=one_argument(argument, num);
  argument=one_argument(argument, trigger);
  argument=one_argument(argument, phrase);

  WNUM wnum;
  if (!parse_widevnum(num, ch->in_room->area, &wnum) || trigger[0] =='\0' || phrase[0] =='\0')
  {
        send_to_char("Syntax:   addoprog [wnum] [trigger] [phrase]\n\r",ch);
        return false;
  }

    if (!(tt = get_trigger_type(trigger, PRG_OPROG))) {
	send_to_char("Valid flags are:\n\r",ch);
	show_help(ch, "oprog");
	return false;
    }

    value = tt->type;
    slot = tt->slot;

	if (!wnum.pArea) wnum.pArea = pObj->area;

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


  if ((code = get_script_index (wnum.pArea, wnum.vnum, PRG_OPROG)) == NULL)
  {
        send_to_char("No such OBJProgram.\n\r",ch);
        return false;
  }

    // Make sure this has a list of progs!
    if(!pObj->progs) pObj->progs = new_prog_bank();

    list                  = new_trigger();
    list->wnum            = wnum;
    list->trig_type       = tt->type;
    list->trig_phrase     = str_dup(phrase);
	list->trig_number		= atoi(list->trig_phrase);
    list->numeric		= is_number(list->trig_phrase);
    list->script          = code;
    //SET_BIT(pMob->mprog_flags,value);

    list_appendlink(pObj->progs[slot], list);
	trigger_type_add_use(tt);

  send_to_char("Oprog Added.\n\r",ch);
  return true;
}

OEDIT (oedit_deloprog)
{
    OBJ_INDEX_DATA *pObj;
    char oprog[MAX_STRING_LENGTH];
    long value;

    EDIT_OBJ(ch, pObj);

    one_argument(argument, oprog);
    if (!is_number(oprog) || oprog[0] == '\0')
    {
	send_to_char("Syntax:  deloprog [#oprog]\n\r",ch);
	return false;
    }

    value = atol (oprog);

    if (value < 0)
    {
	send_to_char("Only non-negative oprog-numbers allowed.\n\r",ch);
	return false;
    }

    if(!edit_deltrigger(pObj->progs,value)) {
	send_to_char("No such oprog.\n\r",ch);
	return false;
    }

    send_to_char("Oprog removed.\n\r", ch);
    return true;
}


OEDIT(oedit_desc)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	string_append(ch, &pObj->full_description);
	return true;
    }

    send_to_char("Syntax:  desc\n\r", ch);
    return false;
}

OEDIT(oedit_comments)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0')
    {
	string_append(ch, &pObj->comments);
	return true;
    }

    send_to_char("Syntax:  comments\n\r", ch);
    return false;
}

OEDIT(oedit_update)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (!IS_STAFF(ch, STAFF_SUPREMACY))
    {
	send_to_char("Insufficient security to toggle update.\n\r", ch);
	return false;
    }

    if (pObj->update == true)
    {
	pObj->update = false;
	send_to_char("Update OFF.\n\r", ch);
    }
    else
    {
	pObj->update = true;
	send_to_char("Update ON.\n\r", ch);
    }

    return true;
}

OEDIT(oedit_timer)
{
    OBJ_INDEX_DATA *pObj;
    char arg[MSL];
    int time;

    EDIT_OBJ(ch, pObj);

    argument = one_argument(argument, arg);
    if (arg[0] == '\0')
    {
	send_to_char("Syntax: timer <#ticks>\n\r", ch);
	return false;
    }

    if (!is_number(arg))
    {
	send_to_char("Argument must be numerical.\n\r", ch);
	return false;
    }

    if ((time = atoi(arg)) < 0 || time > 10000)
    {
	send_to_char("Range is 0 (doesn't decay) to 1000.\n\r", ch);
	return false;
    }

    pObj->timer = time;
    send_to_char("Timer set.\n\r", ch);
    return true;
}
