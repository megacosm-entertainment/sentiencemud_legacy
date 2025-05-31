/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "scripts.h"

//bool edit_deltrigger(LLIST **list, int index);




char *condition_type_to_name (int type)
{
    switch (type)
    {
	case CONDITION_SEASON: return "SEASON";
	case CONDITION_SKY: return "SKY";
	case CONDITION_HOUR: return "HOUR";
	case CONDITION_SCRIPT: return "SCRIPT";
	default: return "UNKNOWN";
    }
}


char *condition_phrase_to_name (int type, int phrase)
{
    switch (type)
    {
	case CONDITION_SEASON:
	    switch (phrase)
	    {
		case SEASON_SPRING: return "SPRING";
		case SEASON_SUMMER: return "SUMMER";
		case SEASON_FALL: return "FALL";
		case SEASON_WINTER: return "WINTER";
		default: return "UNKNOWN";
	    }

	case CONDITION_SKY:
	    switch (phrase)
	    {
		case SKY_CLOUDLESS: return "CLOUDLESS";
		case SKY_CLOUDY: return "CLOUDY";
		case SKY_RAINING: return "RAINY";
		case SKY_LIGHTNING: return "STORMY";
		default: return "UNKNOWN";
	    }

	default: return "UNKNOWN";
    }
}


int cd_phrase_lookup(ROOM_INDEX_DATA *room, int condition, char *phrase)
{
    if (condition == CONDITION_SEASON)
    {
	if (!str_cmp(phrase, "winter"))
	    return SEASON_WINTER;
	else
	if (!str_cmp(phrase, "spring"))
	    return SEASON_SPRING;
	else
	if (!str_cmp(phrase, "summer"))
	    return SEASON_SUMMER;
	else
	if (!str_cmp(phrase, "fall"))
	    return SEASON_FALL;
	else
	    return -1;
    }

    if (condition == CONDITION_SKY)
    {
	if (!str_cmp(phrase, "cloudless"))
	    return SKY_CLOUDLESS;
	else
	if (!str_cmp(phrase, "cloudy"))
	    return SKY_CLOUDY;
	else
	if (!str_cmp(phrase, "rainy"))
	    return SKY_RAINING;
	else
	if (!str_cmp(phrase, "stormy"))
	    return SKY_LIGHTNING;
	else
	    return -1;
    }

    if (condition == CONDITION_HOUR)
    {
	int hour;

	hour = atoi(phrase);

	if (hour < 0 || hour > 23)
	    return -1;
	else
	    return hour;
    }

    if (condition == CONDITION_SCRIPT)
    {

	int vnum;

	vnum = atoi(phrase);

	if (!get_script_index(room->area, vnum,PRG_RPROG))
	    return -1;
	else
	    return vnum;
    }

    return -1;
}






/* Help Editor */



char *token_index_getvaluename(TOKEN_INDEX_DATA *token, int v)
{
	/*
	if(token->type == TOKEN_SPELL )
	{
		if( v == TOKVAL_SPELL_RATING ) return "Rating";
		else if( v == TOKVAL_SPELL_DIFFICULTY ) return "Difficulty";
		else if( v == TOKVAL_SPELL_TARGET ) return "Spell Target";
		else if( v == TOKVAL_SPELL_POSITION ) return "Min Position";
		else if( v == TOKVAL_SPELL_MANA ) return "Mana Cost";
		else if( v == TOKVAL_SPELL_LEARN ) return "Learn Cost";
	}
	else if( token->type == TOKEN_SKILL )
	{
		if( v == TOKVAL_SPELL_RATING ) return "Rating";
		else if( v == TOKVAL_SPELL_DIFFICULTY ) return "Difficulty";
		else if( v == TOKVAL_SPELL_LEARN ) return "Learn Cost";
	}
	else if( token->type == TOKEN_SONG )
	{
		if( v == TOKVAL_SPELL_TARGET ) return "Song Target";
		else if( v == TOKVAL_SPELL_MANA ) return "Mana Cost";
		else if( v == TOKVAL_SPELL_LEARN ) return "Learn Cost";
	}
	*/
	return token->value_name[v];
}

