/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdarg.h>
#include "merc.h"
#include "recycle.h"
#include "olc.h"
#include "tables.h"
#include "scripts.h"

#define DEFAULT_READY_CHECK		300			// 5 minutes

INSTANCE *instance_load(FILE *fp);
void update_instance(INSTANCE *instance);
void reset_instance(INSTANCE *instance);
void save_script_new(FILE *fp, AREA_DATA *area,SCRIPT_DATA *scr,char *type);
SCRIPT_DATA *read_script_new( FILE *fp, AREA_DATA *area, int type);
CHAR_DATA *get_player_leader(CHAR_DATA *ch);
int get_groupsize_in_dungeon(DUNGEON *dng, CHAR_DATA *leader);


extern LLIST *loaded_instances;

LLIST *loaded_dungeons;

DUNGEON_INDEX_LEVEL_DATA *load_dungeon_index_level(FILE *fp, int mode)
{
	DUNGEON_INDEX_LEVEL_DATA *level;
	char *word;
	bool fMatch;
	//char buf[MSL];
	//int floor;

	level = new_dungeon_index_level();
	level->mode = mode;

	if (mode == LEVELMODE_STATIC)
		level->floor = fread_number(fp);
	else if(mode == LEVELMODE_WEIGHTED)
	{
		level->total_weight = 0;
	}
	else if(mode == LEVELMODE_GROUP)
	{

	}

	while (str_cmp((word = fread_word(fp)), "#-LEVEL"))
	{
		fMatch = false;

		//log_stringf("LEVEL: %s", word);

		switch(word[0])
		{
		case '#':
			if (mode == LEVELMODE_GROUP)
			{
				if (!str_cmp(word, "#STATICLEVEL"))
				{
					DUNGEON_INDEX_LEVEL_DATA *lvl = load_dungeon_index_level(fp, LEVELMODE_STATIC);

					list_appendlink(level->group, lvl);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#WEIGHTEDLEVEL"))
				{
					DUNGEON_INDEX_LEVEL_DATA *lvl = load_dungeon_index_level(fp, LEVELMODE_WEIGHTED);

					list_appendlink(level->group, lvl);
					fMatch = true;
					break;
				}
			}
			break;

		case 'F':
			if (!str_cmp(word, "Floor"))
			{
				if (mode == LEVELMODE_WEIGHTED)
				{
					DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted = new_weighted_random_floor();
					weighted->weight = fread_number(fp);
					weighted->floor = fread_number(fp);
					list_appendlink(level->weighted_floors, weighted);

					level->total_weight += weighted->weight;
				}
				else
				{
					// Complain about getting weighted floor data on a static reference?
					fread_to_eol(fp);
				}

				fMatch = true;
			}
			break;
		}

		if (!fMatch) {
			char buf[MSL-1];
			snprintf(buf, sizeof(buf), "load_dungeon_index_level: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	return level;
}

DUNGEON_INDEX_SPECIAL_EXIT *load_dungeon_index_special_exit(FILE *fp, int mode)
{
	DUNGEON_INDEX_SPECIAL_EXIT *ex = new_dungeon_index_special_exit();
	char *word;
	bool fMatch;
	char buf[MSL];

	int max_from = 0;
	int max_to = 0;

	ex->mode = mode;
	ex->name = fread_string(fp);

	if (mode == EXITMODE_STATIC || mode == EXITMODE_WEIGHTED_DEST)
		max_from = 1;
	
	if (mode == EXITMODE_STATIC || mode == EXITMODE_WEIGHTED_SOURCE)
		max_to = 1;

	while (str_cmp((word = fread_word(fp)), "#-EXIT"))
	{
		fMatch = false;

		//log_stringf("EXIT: %s", word);

		switch(word[0])
		{
		case '#':
			if (mode == EXITMODE_GROUP)
			{
				if (!str_cmp(word, "#STATICEXIT"))
				{
					DUNGEON_INDEX_SPECIAL_EXIT *gex = load_dungeon_index_special_exit(fp, EXITMODE_STATIC);

					list_appendlink(ex->group, gex);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#SOURCEEXIT"))
				{
					DUNGEON_INDEX_SPECIAL_EXIT *gex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED_SOURCE);

					list_appendlink(ex->group, gex);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#DESTEXIT"))
				{
					DUNGEON_INDEX_SPECIAL_EXIT *gex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED_DEST);

					list_appendlink(ex->group, gex);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#WEIGHTEDEXIT"))
				{
					DUNGEON_INDEX_SPECIAL_EXIT *gex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED);

					list_appendlink(ex->group, gex);
					fMatch = true;
					break;
				}
			}
			break;

		case 'F':
			if (!str_cmp(word, "From"))
			{
				if (mode == EXITMODE_GROUP)
				{
					bug("load_dungeon_index_special_exit: specifying From entry on a group exit.", 0);
					continue;
				}

				if (max_from > 0 && list_size(ex->from) >= max_from)
				{
					bug("load_dungeon_index_special_exit: too many From entries found for exit mode.", 0);
					continue;
				}

				DUNGEON_INDEX_WEIGHTED_EXIT_DATA *weighted = new_weighted_random_exit();
				weighted->weight = fread_number(fp);
				weighted->level = fread_number(fp);
				weighted->door = fread_number(fp);
				list_appendlink(ex->from, weighted);
				ex->total_from += weighted->weight;
				
				fMatch = true;
				break;
			}
			break;

		case 'T':
			if (!str_cmp(word, "To"))
			{
				if (mode == EXITMODE_GROUP)
				{
					bug("load_dungeon_index_special_exit: specifying To entry on a group exit.", 0);
					continue;
				}

				if (max_to > 0 && list_size(ex->to) >= max_to)
				{
					bug("load_dungeon_index_special_exit: too many To entries found for exit mode.", 0);
					continue;
				}

				DUNGEON_INDEX_WEIGHTED_EXIT_DATA *weighted = new_weighted_random_exit();
				weighted->weight = fread_number(fp);
				weighted->level = fread_number(fp);
				weighted->door = fread_number(fp);
				list_appendlink(ex->to, weighted);
				ex->total_to += weighted->weight;

				fMatch = true;
				break;
			}
			break;
		}

		if (!fMatch) {
			snprintf(buf, sizeof(buf), "load_dungeon_index_special_exit: no match for word %.50s", word);
			bug(buf, 0);
		}
	}
	
	return ex;
}

DUNGEON_INDEX_DATA *load_dungeon_index(FILE *fp, AREA_DATA *area)
{
	DUNGEON_INDEX_DATA *dng;
	char *word;
	bool fMatch;
	char buf[MSL];

	dng = new_dungeon_index();
	dng->vnum = fread_number(fp);

	area->bottom_dungeon_vnum = UMIN(area->bottom_dungeon_vnum, dng->vnum);
	area->top_dungeon_vnum = UMAX(area->top_dungeon_vnum, dng->vnum);

	while (str_cmp((word = fread_word(fp)), "#-DUNGEON"))
	{
		fMatch = false;

		//log_stringf("DUNGEON: %s", word);

		switch(word[0])
		{
		case '#':
			if (!str_cmp(word, "#STATICLEVEL"))
			{
				DUNGEON_INDEX_LEVEL_DATA *level = load_dungeon_index_level(fp, LEVELMODE_STATIC);

				list_appendlink(dng->levels, level);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#WEIGHTEDLEVEL"))
			{
				DUNGEON_INDEX_LEVEL_DATA *level = load_dungeon_index_level(fp, LEVELMODE_WEIGHTED);

				list_appendlink(dng->levels, level);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#GROUPLEVEL"))
			{
				DUNGEON_INDEX_LEVEL_DATA *level = load_dungeon_index_level(fp, LEVELMODE_GROUP);

				list_appendlink(dng->levels, level);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#STATICEXIT"))
			{
				DUNGEON_INDEX_SPECIAL_EXIT *ex = load_dungeon_index_special_exit(fp, EXITMODE_STATIC);

				list_appendlink(dng->special_exits, ex);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#SOURCEEXIT"))
			{
				DUNGEON_INDEX_SPECIAL_EXIT *ex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED_SOURCE);

				list_appendlink(dng->special_exits, ex);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#DESTEXIT"))
			{
				DUNGEON_INDEX_SPECIAL_EXIT *ex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED_DEST);

				list_appendlink(dng->special_exits, ex);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#WEIGHTEDEXIT"))
			{
				DUNGEON_INDEX_SPECIAL_EXIT *ex = load_dungeon_index_special_exit(fp, EXITMODE_WEIGHTED);

				list_appendlink(dng->special_exits, ex);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#GROUPEXIT"))
			{
				DUNGEON_INDEX_SPECIAL_EXIT *ex = load_dungeon_index_special_exit(fp, EXITMODE_GROUP);

				list_appendlink(dng->special_exits, ex);
				fMatch = true;
				break;
			}
			break;

		case 'A':
			KEY("AreaWho", dng->area_who, fread_number(fp));
			break;

		case 'C':
			KEYS("Comments", dng->comments, fread_string(fp));
			break;

		case 'D':
			if (!str_cmp(word, "DeathRelease"))
			{
				dng->death_release = stat_lookup(fread_string(fp), death_release_modes, DEATH_RELEASE_NORMAL);

				fMatch = true;
				break;
			}
			KEYS("Description", dng->description, fread_string(fp));
			if (!str_cmp(word, "DungeonProg")) {
				char *p;


				WNUM_LOAD wnum = fread_widevnum(fp, area->uid);
				p = fread_string(fp);

				struct trigger_type *tt = get_trigger_type(p, PRG_DPROG);
				if(!tt) {
					snprintf(buf, sizeof(buf), "load_dungeon_index: invalid trigger type %s", p);
					bug(buf, 0);
				} else {
					PROG_LIST *dpr = new_trigger();

					dpr->wnum_load = wnum;
					dpr->trig_type = tt->type;
					dpr->trig_phrase = fread_string(fp);
					if( tt->type == TRIG_SPELLCAST ) {
						char buf[MIL];
						SKILL_DATA *skill = get_skill_data(dpr->trig_phrase);

						if( !is_skill_spell(skill) ) {
							snprintf(buf, sizeof(buf), "load_dungeon_index: invalid spell '%s' for TRIG_SPELLCAST", dpr->trig_phrase);
							bug(buf, 0);
							free_trigger(dpr);
							fMatch = true;
							break;
						}

						free_string(dpr->trig_phrase);
						sprintf(buf, "%d", skill->uid);
						dpr->trig_phrase = str_dup(buf);
						dpr->trig_number = skill->uid;
						dpr->numeric = true;

					} else {
						dpr->trig_number = atoi(dpr->trig_phrase);
						dpr->numeric = is_number(dpr->trig_phrase);
					}

					if(!dng->progs) dng->progs = new_prog_bank();

					list_appendlink(dng->progs[tt->slot], dpr);
					trigger_type_add_use(tt);
				}
				fMatch = true;
			}
			break;

		case 'E':
			KEY("Entry", dng->entry_room, fread_number(fp));
			KEY("Exit", dng->exit_room, fread_number(fp));
			break;

		case 'F':
			KEY("Flags", dng->flags, fread_number(fp));
			if( !str_cmp(word, "Floor") )
			{
				WNUM_LOAD *wnum = fread_widevnumptr(fp, area->uid);
				
				if( wnum )
				{
					list_appendlink(dng->floors, wnum);
				}

				fMatch = true;
				break;
			}
			break;

		case 'M':
			KEYS("MountOut", dng->zone_out_mount, fread_string(fp));
			break;

		case 'N':
			KEYS("Name", dng->name, fread_string(fp));
			break;

		case 'P':
			KEYS("PortalOut", dng->zone_out_portal, fread_string(fp));
			break;

		case 'R':
			KEY("Repop", dng->repop, fread_number(fp));
			break;

		case 'S':
			if( !str_cmp(word, "SpecialRoom") )
			{
				DUNGEON_INDEX_SPECIAL_ROOM *special = new_dungeon_index_special_room();

				special->name = fread_string(fp);
				special->level = fread_number(fp);
				special->room = fread_number(fp);

				list_appendlink(dng->special_rooms, special);
				fMatch = true;
				break;
			}
			break;

		case 'V':
			if (olc_load_index_vars(fp, word, &dng->index_vars, area))
			{
				fMatch = true;
				break;
			}

			break;

		case 'Z':
			KEYS("ZoneOut", dng->zone_out, fread_string(fp));
			break;

		}

		if (!fMatch) {
			snprintf(buf, sizeof(buf), "load_dungeon_index: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	/*
	ITERATOR fit;
	BLUEPRINT *bp;
	iterator_start(&fit, dng->floors);
	while((bp = (BLUEPRINT *)iterator_nextdata(&fit)))
	{
		sprintf(buf, "load_dungeon_index: floor - %s", bp->name);
		bug(buf, 0);
	}
	iterator_stop(&fit);
	*/

	return dng;

}


void fix_dungeon_index(DUNGEON_INDEX_DATA *dng)
{
	LLIST *floors = list_create(false);

	WNUM_LOAD *wnum;
	ITERATOR it;
	iterator_start(&it, dng->floors);
	while( (wnum = (WNUM_LOAD *)iterator_nextdata(&it)) )
	{
		//log_stringf("fix_dungeon_index: WNUM: %ld#%ld", wnum->auid, wnum->vnum);
		BLUEPRINT *bp = get_blueprint_auid(wnum->auid, wnum->vnum);

		if (bp)
		{
			//log_stringf("fix_dungeon_index: Floor - %s", bp->name);
			list_appendlink(floors, bp);
		}
	}
	iterator_stop(&it);

	list_destroy(dng->floors);

	dng->floors = floors;
}

void fix_dungeon_indexes()
{
	AREA_DATA *area;
	DUNGEON_INDEX_DATA *dng;
	int iHash;

	for(area = area_first; area; area = area->next)
	{
		for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		{
			for(dng = area->dungeon_index_hash[iHash]; dng; dng = dng->next)
				fix_dungeon_index(dng);
		}
	}
}

/*
void load_dungeons()
{
	FILE *fp = fopen(DUNGEONS_FILE, "r");
	if (fp == NULL)
	{
		bug("Couldn't load dungeons.dat", 0);
		return;
	}
	char *word;
	bool fMatch;

	top_dprog_index = 0;

	while (str_cmp((word = fread_word(fp)), "#END"))
	{
		fMatch = false;

		if( !str_cmp(word, "#DUNGEON") )
		{
			DUNGEON_INDEX_DATA *dng = load_dungeon_index(fp);
			int iHash = dng->vnum % MAX_KEY_HASH;

			dng->next = dungeon_index_hash[iHash];
			dungeon_index_hash[iHash] = dng;

			fMatch = true;
		}

		if (!str_cmp(word, "#DUNGEONPROG"))
		{
		    SCRIPT_DATA *pr = read_script_new(fp, NULL, IFC_D);
		    if(pr) {
		    	pr->next = dprog_list;
		    	dprog_list = pr;

		    	if( pr->vnum > top_dprog_index )
		    		top_dprog_index = pr->vnum;
		    }

		    fMatch = true;
		}


		if (!fMatch) {
			char buf[MSL];
			sprintf(buf, "load_dungeons: no match for word %.50s", word);
			bug(buf, 0);
		}

	}

	fclose(fp);
}
*/

void save_dungeon_index_level(FILE *fp, DUNGEON_INDEX_LEVEL_DATA *level, bool allow_groups)
{
	switch(level->mode)
	{
		case LEVELMODE_STATIC:
			fprintf(fp, "#STATICLEVEL %d\n", level->floor);
			break;

		case LEVELMODE_WEIGHTED:
			fprintf(fp, "#WEIGHTEDLEVEL\n");
			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *floor;
			ITERATOR wit;
			iterator_start(&wit, level->weighted_floors);
			while((floor = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
			{
				fprintf(fp, "Floor %d %d\n", floor->weight, floor->floor);
			}
			iterator_stop(&wit);
			break;

		case LEVELMODE_GROUP:
			fprintf(fp, "#GROUPLEVEL\n");
			DUNGEON_INDEX_LEVEL_DATA *group;
			ITERATOR git;
			iterator_start(&git, level->group);
			while( (group = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)) )
			{
				save_dungeon_index_level(fp, group, false);
			}

			iterator_stop(&git);
			break;
		
		default:
			return;
	}
	fprintf(fp, "#-LEVEL\n");
}

void save_dungeon_index_special_exit(FILE *fp, DUNGEON_INDEX_SPECIAL_EXIT *special, bool allow_groups)
{
	ITERATOR it;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to;

	switch(special->mode)
	{
		case EXITMODE_STATIC:
			fprintf(fp, "#STATICEXIT %s~\n", fix_string(special->name));
			from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(special->from, 1);
			fprintf(fp, "From 1 %d %d\n", from->level, from->door);
			to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(special->to, 1);
			fprintf(fp, "To 1 %d %d\n", to->level, to->door);
			break;

		case EXITMODE_WEIGHTED_SOURCE:
			fprintf(fp, "#SOURCEEXIT %s~\n", fix_string(special->name));
			iterator_start(&it, special->from);
			while( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&it)) )
			{
				fprintf(fp, "From %d %d %d\n", from->weight, from->level, from->door);
			}
			iterator_stop(&it);
			to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(special->to, 1);
			fprintf(fp, "To 1 %d %d\n", to->level, to->door);
			break;

		case EXITMODE_WEIGHTED_DEST:
			fprintf(fp, "#DESTEXIT %s~\n", fix_string(special->name));
			from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(special->from, 1);
			fprintf(fp, "From 1 %d %d\n", from->level, from->door);
			iterator_start(&it, special->to);
			while( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&it)) )
			{
				fprintf(fp, "To %d %d %d\n", to->weight, to->level, to->door);
			}
			iterator_stop(&it);
			break;

		case EXITMODE_WEIGHTED:
			fprintf(fp, "#WEIGHTEDEXIT %s~\n", fix_string(special->name));
			iterator_start(&it, special->from);
			while( (from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&it)) )
			{
				fprintf(fp, "From %d %d %d\n", from->weight, from->level, from->door);
			}
			iterator_stop(&it);
			iterator_start(&it, special->to);
			while( (to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&it)) )
			{
				fprintf(fp, "To %d %d %d\n", to->weight, to->level, to->door);
			}
			iterator_stop(&it);
			break;
		
		case EXITMODE_GROUP:
			if (allow_groups)
			{
				DUNGEON_INDEX_SPECIAL_EXIT *gex;
				fprintf(fp, "#GROUPEXIT %s~\n", fix_string(special->name));
				iterator_start(&it, special->group);
				while( (gex = (DUNGEON_INDEX_SPECIAL_EXIT *)iterator_nextdata(&it)) )
				{
					save_dungeon_index_special_exit(fp, gex, false);
				}	
				iterator_stop(&it);
			}
			break;
	}
	fprintf(fp, "#-EXIT\n");
}

void save_dungeon_index(FILE *fp, DUNGEON_INDEX_DATA *dng)
{
	ITERATOR it;

	fprintf(fp, "#DUNGEON %ld\n", dng->vnum);
	fprintf(fp, "Name %s~\n", fix_string(dng->name));
	fprintf(fp, "Description %s~\n", fix_string(dng->description));
	fprintf(fp, "Comments %s~\n", fix_string(dng->comments));
	fprintf(fp, "AreaWho %d\n", dng->area_who);
	fprintf(fp, "Repop %d\n", dng->repop);

	fprintf(fp, "Flags %d\n", dng->flags);

	if( dng->entry_room > 0 )
		fprintf(fp, "Entry %ld\n", dng->entry_room);

	if( dng->exit_room > 0 )
		fprintf(fp, "Exit %ld\n", dng->exit_room);

	fprintf(fp, "ZoneOut %s~\n", fix_string(dng->zone_out));
	fprintf(fp, "PortalOut %s~\n", fix_string(dng->zone_out_portal));
	fprintf(fp, "MountOut %s~\n", fix_string(dng->zone_out_mount));

	fprintf(fp, "DeathRelease %s~\n", flag_string(death_release_modes, dng->death_release));

	BLUEPRINT *bp;
	iterator_start(&it, dng->floors);
	while((bp = (BLUEPRINT *)iterator_nextdata(&it)))
	{
		fprintf(fp, "Floor %s\n", widevnum_string(bp->area, bp->vnum, dng->area));
	}
	iterator_stop(&it);

	// Only save the level design if the dungeon is set to manual mode
	if (!IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		DUNGEON_INDEX_LEVEL_DATA *level;
		ITERATOR lit;
		iterator_start(&lit, dng->levels);
		while((level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&lit)))
		{
			save_dungeon_index_level(fp, level, true);
		}
		iterator_stop(&lit);

		DUNGEON_INDEX_SPECIAL_ROOM *special;
		iterator_start(&it, dng->special_rooms);
		while( (special = (DUNGEON_INDEX_SPECIAL_ROOM *)iterator_nextdata(&it)) )
		{
			fprintf(fp, "SpecialRoom %s~ %d %d\n", fix_string(special->name), special->level, special->room);
		}
		iterator_stop(&it);

		DUNGEON_INDEX_SPECIAL_EXIT *ex;
		iterator_start(&it, dng->special_exits);
		while( (ex = (DUNGEON_INDEX_SPECIAL_EXIT *)iterator_nextdata(&it)) )
		{
			save_dungeon_index_special_exit(fp, ex, true);

		}
		iterator_stop(&it);
	}

    if(dng->progs) {
		ITERATOR it;
		PROG_LIST *trigger;
		for(int i = 0; i < TRIGSLOT_MAX; i++) if(list_size(dng->progs[i]) > 0) {
			iterator_start(&it, dng->progs[i]);
			while((trigger = (PROG_LIST *)iterator_nextdata(&it)))
				fprintf(fp, "DungeonProg %s %s~ %s~\n",
					widevnum_string_wnum(trigger->wnum, dng->area),
					trigger_name(trigger->trig_type), trigger_phrase(trigger->trig_type,trigger->trig_phrase));
			iterator_stop(&it);
		}
	}

	olc_save_index_vars(fp, dng->index_vars, dng->area);

	fprintf(fp, "#-DUNGEON\n\n");
}

void save_dungeons(FILE *fp, AREA_DATA *area)
{
	int iHash;
	for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(DUNGEON_INDEX_DATA *dng = area->dungeon_index_hash[iHash]; dng; dng = dng->next)
		{
			save_dungeon_index(fp, dng);
		}
	}
}

bool can_edit_dungeons(CHAR_DATA *ch)
{
	// Add dungeoneering certification for imms
	return !IS_NPC(ch) && (ch->pcdata->security >= 9) && (IS_IMPLEMENTOR(ch));
}

DUNGEON_INDEX_DATA *get_dungeon_index_wnum(WNUM wuid)
{
	return get_dungeon_index(wuid.pArea, wuid.vnum);
}

DUNGEON_INDEX_DATA *get_dungeon_index_auid(long auid, long vnum)
{
	return get_dungeon_index(get_area_from_uid(auid), vnum);
}

DUNGEON_INDEX_DATA *get_dungeon_index(AREA_DATA *pArea, long vnum)
{
	if (!pArea) return NULL;

	for(int iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(DUNGEON_INDEX_DATA *dng = pArea->dungeon_index_hash[iHash]; dng; dng = dng->next)
		{
			if( dng->vnum == vnum )
				return dng;
		}
	}

	return NULL;
}

int dungeon_commence(DUNGEON *dng)
{
	if (!IS_VALID(dng)) return PRET_NOSCRIPT;
	if (IS_SET(dng->flags, DUNGEON_COMMENCED)) return PRET_NOSCRIPT;

	// LALI-HO!
	SET_BIT(dng->flags, DUNGEON_COMMENCED);

	return p_percent2_trigger(NULL, NULL, dng, NULL, NULL, NULL, NULL, NULL, TRIG_DUNGEON_COMMENCED, NULL,0,0,0,0,0);
}

int dungeon_completed(DUNGEON *dng)
{
	if (!IS_VALID(dng)) return PRET_NOSCRIPT;
	if (IS_SET(dng->flags, (DUNGEON_COMPLETED|DUNGEON_FAILED))) return PRET_NOSCRIPT;

	SET_BIT(dng->flags, DUNGEON_COMPLETED);

	return p_percent2_trigger(NULL, NULL, dng, NULL, NULL, NULL, NULL, NULL, TRIG_COMPLETED, NULL,0,0,0,0,0);
}

int dungeon_failed(DUNGEON *dng)
{
	if (!IS_VALID(dng)) return PRET_NOSCRIPT;
	if (IS_SET(dng->flags, (DUNGEON_COMPLETED|DUNGEON_FAILED))) return PRET_NOSCRIPT;

	SET_BIT(dng->flags, DUNGEON_FAILED);

	return p_percent2_trigger(NULL, NULL, dng, NULL, NULL, NULL, NULL, NULL, TRIG_FAILED, NULL,0,0,0,0,0);
}

void dungeon_check_commence(DUNGEON *dng, CHAR_DATA *ch)
{
	//char buf[MSL];
	if (!IS_VALID(dng)) return;
	if (IS_SET(dng->flags, DUNGEON_COMMENCED)) return;

	if (IS_SET(dng->flags, DUNGEON_GROUP_COMMENCE))
	{
		CHAR_DATA *leader = get_player_leader(ch);
		
		int size = get_groupsize_in_dungeon(dng, leader);

		//sprintf(buf, "dungeon_check_commence: %d ?= %d\n\r", size, dng->index->min_group);
		//send_to_char(buf, ch);

		if (size >= dng->index->min_group)
		{
			/*int ret = */dungeon_commence(dng);

			//sprintf(buf, "dungeon_commence -> ret = %d\n\r", ret);
			//send_to_char(buf, ch);
		}
	}
}


static bool add_dungeon_instance(DUNGEON *dng, BLUEPRINT *bp)
{
//	char buf[MSL];

//	sprintf(buf, "add_dungeon_instance: blueprint %s", 
//		(bp) ? (bp->valid ? widevnum_string(bp->area, bp->vnum, NULL) : "invalid") : "null"); 
//	wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

	// Complain
	if (!IS_VALID(bp))
		return true;

	INSTANCE *instance = create_instance(bp);

	if( !instance )
	{
//		wiznet("add_dungeon_instance: failed to create instance",NULL,NULL,WIZ_TESTING,0,0);
		return true;
	}

	instance->dungeon = dng;
	list_appendlink(dng->floors, instance);
	instance->floor = list_size(dng->floors);
	list_appendlist(dng->rooms, instance->rooms);
	list_appendlink(loaded_instances, instance);
//	wiznet("add_dungeon_instance: instance created",NULL,NULL,WIZ_TESTING,0,0);
	return false;
}

static bool add_dungeon_level(DUNGEON *dng, DUNGEON_INDEX_LEVEL_DATA *level)
{
//	char buf[MSL];
	BLUEPRINT *bp;
	switch(level->mode)
	{
		case LEVELMODE_STATIC:
//			sprintf(buf, "add_dungeon_level: adding static floor %d", level->floor);
//			wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

			bp = (BLUEPRINT *)list_nthdata(dng->index->floors, level->floor);
			return add_dungeon_instance(dng, bp);

		case LEVELMODE_WEIGHTED:
		{
//			wiznet("add_dungeon_level: adding weighted level",NULL,NULL,WIZ_TESTING,0,0);
			int w = number_range(1, level->total_weight);
			bp = NULL;	// Should NEVER get this!

			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
			ITERATOR wit;
			iterator_start(&wit, level->weighted_floors);
			while((weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
			{
				if (w <= weighted->weight)
				{
					bp = (BLUEPRINT *)list_nthdata(dng->index->floors, weighted->floor);
					break;
				}

				w -= weighted->weight;
			}
			iterator_stop(&wit);

			if (!IS_VALID(bp))
			{
				// Complain about an impossible situation
				return true;
			}

			return add_dungeon_instance(dng, bp);
		}

		case LEVELMODE_GROUP:
		{
//			wiznet("add_dungeon_level: adding group level",NULL,NULL,WIZ_TESTING,0,0);
			bool error = false;
			DUNGEON_INDEX_LEVEL_DATA *lvl;

			int count = list_size(level->group);

			int *source = (int *)alloc_mem(sizeof(int) * count);
			for(int i = 0; i < count; i++)
				source[i] = i + 1;

			for(int i = count - 1; i >= 0; i--)
			{
				int ilevel = number_range(0, i);
				int nlevel = source[ilevel];
				source[ilevel] = source[i];

				lvl = (DUNGEON_INDEX_LEVEL_DATA *)list_nthdata(level->group, nlevel);

				if (add_dungeon_level(dng, lvl))
				{
					error = true;
					break;
				}
			}

			free_mem(source, sizeof(int) * count);
			return error;
		}
	}

	return true;
}

/*
static bool add_dungeon_levels(DUNGEON *dng)
{
	bool error = false;
	DUNGEON_INDEX_LEVEL_DATA *level;
	ITERATOR lit;

	list_clear(dng->floors);
	list_clear(dng->rooms);
	iterator_start(&lit, dng->index->levels);
	while( (level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&lit)) )
	{
		if (add_dungeon_level(dng, level))
		{
			error = true;
			break;
		}
	}
	iterator_stop(&lit);

	return error;
}
*/

static DUNGEON_INDEX_WEIGHTED_EXIT_DATA *get_weighted_random_exit(LLIST *list, int total)
{
	if (list_size(list) == 1) return (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(list, 1);

	int w = number_range(1, total);

	ITERATOR it;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *weighted;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *selected = NULL;
	iterator_start(&it, list);
	while( (weighted = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)iterator_nextdata(&it)) )
	{
		if (w <= weighted->weight)
		{
			selected = weighted;
			break;
		}
		else
			w -= weighted->weight;
	}
	iterator_stop(&it);

	return selected;
}

static EXIT_DATA *clone_dungeon_exit(ROOM_INDEX_DATA *room, int door)
{
	EXIT_DATA *ex = room->exit[door];

	if (!IS_VALID(ex))
	{
		EXIT_DATA *index = room->source->exit[door];

		room->exit[door] = ex = new_exit();
		ex->orig_door = door;
		ex->from_room = room;

		if (IS_VALID(index))
		{
			ex->rs_flags = index->rs_flags;
			REMOVE_BIT(ex->rs_flags, EX_ENVIRONMENT);
			ex->door.rs_lock = index->door.rs_lock;
		}
	}

	return ex;
}

INSTANCE *dungeon_get_instance_level(DUNGEON *dng, int level_no)
{
	if (!IS_VALID(dng)) return NULL;

	ITERATOR it;
	if (level_no < 0)
	{
		int ordinal = -level_no;
		INSTANCE *instance;

		iterator_start(&it, dng->floors);
		while((instance = (INSTANCE *)iterator_nextdata(&it)))
		{
			if (instance->ordinal == ordinal)
				break;
		}
		iterator_stop(&it);
		
		return instance;
	}
	else
		return (INSTANCE *)list_nthdata(dng->floors, level_no);
}

static bool add_dungeon_special_exit_from_to(DUNGEON *dng, DUNGEON_INDEX_SPECIAL_EXIT *dsex,
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from, DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to)
{
	if (!from || !to)
	{
		// Failed to get an exit reference
		return true;
	}

	INSTANCE *from_level = dungeon_get_instance_level(dng, from->level);
	if (!IS_VALID(from_level))
	{
		return true;
	}

	INSTANCE *to_level = dungeon_get_instance_level(dng, to->level);
	if (!IS_VALID(to_level))
	{
		return true;
	}

	BLUEPRINT_EXIT_DATA *from_ex = get_blueprint_exit(from_level->blueprint, from->door);
	BLUEPRINT_EXIT_DATA *to_ex = get_blueprint_entrance(to_level->blueprint, to->door);

	ROOM_INDEX_DATA *from_room = NULL;
	int from_door = -1;
	EXIT_DATA *from_exit = NULL;
	EXIT_DATA *fromClone = NULL;

	if (from_ex)
	{
		INSTANCE_SECTION *from_section = instance_get_section(from_level, from_ex->section);

		if (from_section)
		{
			BLUEPRINT_LINK *from_link = get_section_link(from_section->section, from_ex->link);

			if (from_link)
			{
				from_room = instance_section_get_room_byvnum(from_section, from_link->vnum);
				from_door = from_link->door;

				if (from_room && from_door >= 0 && from_door < MAX_DIR)
				{
					from_exit = from_room->source->exit[from_door];
					fromClone = from_room->exit[from_door];
				}
			}
		}
	}

	ROOM_INDEX_DATA *to_room = NULL;
	int to_door = -1;
	EXIT_DATA *to_exit = NULL;
	EXIT_DATA *toClone = NULL;

	if (to_ex)
	{
		INSTANCE_SECTION *to_section = instance_get_section(to_level, to_ex->section);

		if (to_section)
		{
			BLUEPRINT_LINK *to_link = get_section_link(to_section->section, to_ex->link);

			if (to_link)
			{
				to_room = instance_section_get_room_byvnum(to_section, to_link->vnum);
				to_door = to_link->door;

				if (to_room && to_door >= 0 && to_door < MAX_DIR)
				{
					to_exit = to_room->source->exit[to_door];
					toClone = to_room->exit[to_door];
				}
			}
		}
	}

	// Must have a source room, and either no source exit or an unlinked exit
	if (from_room && (!IS_VALID(from_exit) || !from_exit->u1.to_room))
	{
		// Cannot link up an exit that is already linked up somewhere else
		//  Or the remote index exit exists and has a destination already
		if ((!IS_VALID(fromClone) || !fromClone->u1.to_room) &&
			(!IS_VALID(to_exit) || !to_exit->u1.to_room))
		{

			// Deal with the remote exit, first
			if (to_room)
			{
				// If the from exit doesn't have a clone or isn't already linked
				if (!IS_VALID(fromClone) || !fromClone->u1.to_room)
				{
					if (IS_VALID(to_exit))
					{
						// Only connect if we can make a two-way exit?
						if (dsex->connect_if_twoway)
						{
							if (!IS_VALID(toClone))
								toClone = clone_dungeon_exit(to_room, to_door);
						}
					}
					else
					{
						// The target room doesn't have a remote exit, so make it
						toClone = clone_dungeon_exit(to_room, to_door);
					}
				}

				if (IS_VALID(toClone))
				{
					REMOVE_BIT(toClone->rs_flags, EX_ENVIRONMENT);

					if (!toClone->u1.to_room)
					{
						toClone->u1.to_room = from_room;
						
						// We are creating a two-way exit
						//  Are the two exits reverses of each other
						//  If so, set the reset data on the remote exit from the source exit
						if (from_door == rev_dir[to_door])
						{
							// Only do this when they are reverses as the exit code doesn't account for exits
							//    linked to each other not being this way, such as one going north, the other
							//    going west.
							if (IS_VALID(from_exit))
							{
								toClone->rs_flags = from_exit->rs_flags;
								toClone->door.rs_lock.flags = from_exit->door.rs_lock.flags;
								toClone->door.rs_lock.key_wnum = from_exit->door.rs_lock.key_wnum;
								toClone->door.rs_lock.special_keys = from_exit->door.rs_lock.special_keys;	// This will not be destroyed when the exit is freed
								toClone->door.rs_lock.pick_chance = from_exit->door.rs_lock.pick_chance;
							}
							else
							{
								toClone->rs_flags = 0;
								toClone->door.rs_lock.flags = 0;
								toClone->door.rs_lock.key_wnum.pArea = NULL;
								toClone->door.rs_lock.key_wnum.vnum = 0;
								toClone->door.rs_lock.special_keys = NULL;
								toClone->door.rs_lock.pick_chance = 0;
							}
						}
					}
				}
			}
		}

		if (!dsex->create_if_exists ||									// Creates unlinked exit
			(to_room && !dsex->connect_if_twoway) ||			// Creates one-way exit
			(IS_VALID(toClone) && toClone->u1.to_room == from_room))	// Creates two-exit
		{
			if (!IS_VALID(fromClone))
			{
				fromClone = clone_dungeon_exit(from_room, from_door);

				// Explicit remote exit, no source exit and directions are reverses.
				//   Put all exit settings from remote exit onto source exit to make them symmetric
				if (!IS_VALID(from_exit) && IS_VALID(to_exit) && from_door == rev_dir[to_door])
				{
					fromClone->rs_flags = to_exit->rs_flags;
					fromClone->door.rs_lock.flags = to_exit->door.rs_lock.flags;
					fromClone->door.rs_lock.key_wnum = to_exit->door.rs_lock.key_wnum;
					fromClone->door.rs_lock.special_keys = to_exit->door.rs_lock.special_keys;
					fromClone->door.rs_lock.pick_chance = to_exit->door.rs_lock.pick_chance;
				}
			}

			fromClone->u1.to_room = to_room;
		}
	}

	NAMED_SPECIAL_EXIT *special = new_named_special_exit();
	special->name = str_dup(dsex->name);
	if(from_room)
	{
		special->room = from_room;
		if (IS_VALID(fromClone))
			special->ex = fromClone;
	}
	list_appendlink(dng->special_exits, special);

	return false;	
}

static bool add_dungeon_special_exit(DUNGEON *dng, DUNGEON_INDEX_SPECIAL_EXIT *dsex)
{
	bool error = false;
	//	DUNGEON_INDEX_DATA *index = dng->index;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *from = NULL;
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *to = NULL;

	switch(dsex->mode)
	{
	// Fixed source and Fixed destination
	case EXITMODE_STATIC:
		from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->from, 1);	// Get the first entries
		to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->to, 1);
		break;

	// Weight Random source and Fixed destination
	case EXITMODE_WEIGHTED_SOURCE:
		from = get_weighted_random_exit(dsex->from, dsex->total_from);
		to = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->to, 1);
		break;

	// Fixed source and Weighted Random destination
	case EXITMODE_WEIGHTED_DEST:
		from = (DUNGEON_INDEX_WEIGHTED_EXIT_DATA *)list_nthdata(dsex->from, 1);
		to = get_weighted_random_exit(dsex->to, dsex->total_to);
		break;

	// Weighted Random source and Weighted Random destination
	case EXITMODE_WEIGHTED:
		from = get_weighted_random_exit(dsex->from, dsex->total_from);
		to = get_weighted_random_exit(dsex->to, dsex->total_to);
		break;

	case EXITMODE_GROUP:
		{
			int count = list_size(dsex->group);

			int *dest = (int *)alloc_mem(sizeof(int) * count);

			for(int i = 0; i < count; i++)
				dest[i] = i + 1;

			for(int i = count - 1; i >= 0; i--)
			{
				int iexit = number_range(0, i);
				int ndest = dest[iexit];
				dest[iexit] = dest[i];

				DUNGEON_INDEX_SPECIAL_EXIT *fex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(dsex->group, i + 1);
				DUNGEON_INDEX_SPECIAL_EXIT *tex = (DUNGEON_INDEX_SPECIAL_EXIT *)list_nthdata(dsex->group, ndest);

				from = get_weighted_random_exit(fex->from, fex->total_from);
				to = get_weighted_random_exit(tex->to, tex->total_to);

				if (add_dungeon_special_exit_from_to(dng, dsex, from, to))
				{
					error = true;
					break;
				}
			}

			free_mem(dest, sizeof(int) * count);
			return error;
		}
	}

	return add_dungeon_special_exit_from_to(dng, dsex, from, to);
}

inline static void __dungeon_set_portal_room(OBJ_DATA *obj, ROOM_INDEX_DATA *room)
{
	if (!IS_PORTAL(obj)) return;

	PORTAL_DATA *portal = PORTAL(obj);
	portal->type = GATETYPE_NORMAL;

	if (room)
	{
		if (room->source)
		{
			portal->params[0] = room->source->area->uid;
			portal->params[1] = room->source->vnum;
			portal->params[2] = room->id[0];
			portal->params[3] = room->id[1];
			portal->params[4] = 0;
		}
		else
		{
			portal->params[0] = room->area->uid;
			portal->params[1] = room->vnum;
			portal->params[2] = 0;
			portal->params[3] = 0;
			portal->params[4] = 0;
		}
	}
	else
	{
		portal->params[0] = 0;
		portal->params[1] = 0;
		portal->params[2] = 0;
		portal->params[3] = 0;
		portal->params[4] = 0;
	}
}


static void __dungeon_correct_portals(DUNGEON *dng)
{
	ROOM_INDEX_DATA *room;
	ITERATOR rit;

	iterator_start(&rit, dng->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
	{
		for(OBJ_DATA *obj = room->contents; obj; obj = obj->next_content)
		{
			if (IS_PORTAL(obj))
			{
				switch(PORTAL(obj)->type)
				{
					case GATETYPE_DUNGEON_SPECIAL:
					{
						ROOM_INDEX_DATA *room = NULL;

						if (PORTAL(obj)->params[0] > 0)
						{
							room = get_dungeon_special_room(dng, PORTAL(obj)->params[0]);
						}

						__dungeon_set_portal_room(obj, room);
						break;
					}
				}
			}
		}
	}
	iterator_stop(&rit);
}

DUNGEON *create_dungeon(AREA_DATA *pArea, long vnum)
{
	//char buf[MSL];
	ITERATOR it;

	DUNGEON_INDEX_DATA *index = get_dungeon_index(pArea, vnum);

	if( !IS_VALID(index) )
	{
		return NULL;
	}

	// Dungeon is SHARED and already loaded, cannot create it again
	if (IS_SET(index->flags, DUNGEON_SHARED) && list_size(index->loaded) > 0)
	{
		// Get the existing dungeon?
		return (DUNGEON *)list_nthdata(index->loaded, 1);

		// Or should this return NULL?
	}

	//wiznet("create_dungeon: new_dungeon",NULL,NULL,WIZ_TESTING,0,0);

	DUNGEON *dng = new_dungeon();
	dng->index = index;
	dng->flags = index->flags;

	//wiznet("create_dungeon: variables",NULL,NULL,WIZ_TESTING,0,0);

	/*
	ITERATOR fit;
	BLUEPRINT *bp;
	iterator_start(&fit, index->floors);
	while((bp = (BLUEPRINT *)iterator_nextdata(&fit)))
	{
		sprintf(buf, "create_dungeon: floor - %s", bp->name);
		wiznet(buf, NULL, NULL, WIZ_TESTING, 0, 0);
	}
	iterator_stop(&fit);
	*/

	dng->progs			= new_prog_data();
	dng->progs->progs	= index->progs;
	variable_copylist(&index->index_vars,&dng->progs->vars,false);

	//wiznet("create_dungeon: get entry room",NULL,NULL,WIZ_TESTING,0,0);

	dng->entry_room = get_room_index(index->area, index->entry_room);
	if( !dng->entry_room )
	{
		free_dungeon(dng);
		return NULL;
	}

	//wiznet("create_dungeon: get exit room",NULL,NULL,WIZ_TESTING,0,0);

	dng->exit_room = get_room_index(index->area, index->exit_room);
	if( !dng->exit_room )
	{
		free_dungeon(dng);
		return NULL;
	}

	dng->flags = index->flags;

	// Allow a script to create the level definitions, provided it's set to do that.
	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		//wiznet("create_dungeon: dungeon schematic",NULL,NULL,WIZ_TESTING,0,0);
		list_clear(index->levels);
		list_clear(index->special_rooms);
		list_clear(index->special_exits);
		p_percent2_trigger(NULL, NULL, dng, NULL, NULL, NULL, NULL, NULL, TRIG_DUNGEON_SCHEMATIC, NULL,0,0,0,0,0);
	}

	//wiznet("create_dungeon: iterate over levels",NULL,NULL,WIZ_TESTING,0,0);
	bool error = false;
	DUNGEON_INDEX_LEVEL_DATA *level;
	//BLUEPRINT *bp;
	//int level_no = 1;
	INSTANCE *instance;
	iterator_start(&it, index->levels);
	while( (level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)) )
	{
		//sprintf(buf, "create_dungeon: adding level %d", level_no++);
		//wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);
		if (add_dungeon_level(dng, level))
		{
			//wiznet("create_dungeon: failed to add level",NULL,NULL,WIZ_TESTING,0,0);
			error = true;
			break;
		}
	}
	iterator_stop(&it);

	if (!error)
	{
		//wiznet("create_dungeon: correcting portals",NULL,NULL,WIZ_TESTING,0,0);
		__dungeon_correct_portals(dng);

		//wiznet("create_dungeon: resolve special rooms",NULL,NULL,WIZ_TESTING,0,0);
		DUNGEON_INDEX_SPECIAL_ROOM *special;
		iterator_start(&it, index->special_rooms);
		while( (special = (DUNGEON_INDEX_SPECIAL_ROOM *)iterator_nextdata(&it)) )
		{
			// Get the instance for the specified level.
			instance = dungeon_get_instance_level(dng, special->level);

			if( IS_VALID(instance) )
			{
				// Get special room from the instance.
				NAMED_SPECIAL_ROOM *isr = list_nthdata(instance->special_rooms, special->room);

				// Room was found, add to dungeon under new name
				if( isr )
				{
					NAMED_SPECIAL_ROOM *dsr = new_named_special_room();

					free_string(dsr->name);
					dsr->name = str_dup(special->name);
					dsr->room = isr->room;

					list_appendlink(dng->special_rooms, dsr);
				}
			}
		}
		iterator_stop(&it);

		//wiznet("create_dungeon: resolve special exits",NULL,NULL,WIZ_TESTING,0,0);
		DUNGEON_INDEX_SPECIAL_EXIT *dsex;
		iterator_start(&it, index->special_exits);
		while( (dsex = (DUNGEON_INDEX_SPECIAL_EXIT *)iterator_nextdata(&it)) )
		{
			if (add_dungeon_special_exit(dng, dsex))
			{
				error = true;
				break;
			}

		}
		iterator_stop(&it);
	}

	if( error )
	{
		free_dungeon(dng);
		return NULL;
	}

	//wiznet("create_dungeon: getting id",NULL,NULL,WIZ_TESTING,0,0);

	get_dungeon_id(dng);

	list_appendlink(loaded_dungeons, dng);
	list_appendlink(index->loaded, dng);

	if (IS_SET(dng->flags, DUNGEON_SHARED))
	{

	}

	//wiznet("create_dungeon: complete",NULL,NULL,WIZ_TESTING,0,0);
	return dng;
}

void extract_dungeon(DUNGEON *dungeon)
{
	ITERATOR it;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	INSTANCE *instance;

    if(dungeon->progs) {
	    SET_BIT(dungeon->progs->entity_flags,PROG_NODESTRUCT);
	    if(dungeon->progs->script_ref > 0) {
			dungeon->progs->extract_when_done = true;
			return;
		}
    }

	room = dungeon->entry_room;
	if( !room )
		room = room_index_temple;

	// Dump all mobiles
	iterator_start(&it, dungeon->mobiles);
	while( (ch = (CHAR_DATA *)iterator_nextdata(&it)) )
	{
		char_from_room(ch);
		char_to_room(ch, room);
	}
	iterator_stop(&it);

	// Dump objects
	room = dungeon->entry_room;
	if( !room )
		room = room_index_donation;

	iterator_start(&it, dungeon->objects);
	while( (obj = (OBJ_DATA *)iterator_nextdata(&it)) )
	{
		if( obj->in_obj )
			obj_from_obj (obj);
		else if( obj->carried_by )
			obj_from_char(obj);
		else if( obj->in_room)
			obj_from_room(obj);

		obj_to_room(obj, room);
	}
	iterator_stop(&it);


	list_remlink(loaded_dungeons, dungeon, false);

	// Remove instances from loaded list
	iterator_start(&it, dungeon->floors);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		list_remlink(loaded_instances, instance, true);
	}
	iterator_stop(&it);

	free_dungeon(dungeon);
}

DUNGEON *find_dungeon_byplayer(CHAR_DATA *ch, AREA_DATA *pArea, long vnum)
{
	ITERATOR dit;
	DUNGEON *dng;

	if( IS_NPC(ch) ) return NULL;

	iterator_start(&dit, loaded_dungeons);
	while( (dng = (DUNGEON *)iterator_nextdata(&dit)) )
	{
		if( dng->index->area == pArea && dng->index->vnum == vnum && (IS_SET(dng->flags, DUNGEON_SHARED) || dungeon_isowner_player(dng, ch)) )
			break;
	}
	iterator_stop(&dit);

	return dng;
}

CHAR_DATA *get_player_leader(CHAR_DATA *ch)
{
	CHAR_DATA *leader = ch;

	while( (leader->leader != NULL) && (leader->leader != leader) && !IS_NPC(leader->leader) )
	{
		leader = leader->leader;
	}

	return leader;
}

// This may or may not be the same as "leader->num_grouped"
int get_groupsize_in_dungeon(DUNGEON *dng, CHAR_DATA *leader)
{
	if (!IS_VALID(dng)) return 0;
	if (!IS_VALID(leader)) return 0;

	int count = 0;

	ITERATOR it;
	CHAR_DATA *mob;
	iterator_start(&it, dng->mobiles);
	while((mob = (CHAR_DATA *)iterator_nextdata(&it)))
	{
		if (RIDDEN(mob)) continue;	// Skip mounts
		if (mob->master != NULL && mob->master->pet == mob) continue;	// Skip pets

		if (leader == mob || is_same_group(leader, mob))
			count++;
	}
	iterator_stop(&it);

	return count;
}

DUNGEON *spawn_dungeon_player(CHAR_DATA *ch, AREA_DATA *pArea, long vnum)
{
//	char buf[MSL];
	CHAR_DATA *leader = get_player_leader(ch);

	DUNGEON *leader_dng = find_dungeon_byplayer(leader, pArea, vnum);
	DUNGEON *ch_dng = find_dungeon_byplayer(ch, pArea, vnum);

	// Check if the player already has a dungeon
	if( IS_VALID(ch_dng) )
	{
		// Different dungeon?
		if( ch_dng != leader_dng )
		{
			if( !dungeon_canswitch_player(ch_dng, ch) )
			{
				leader_dng = ch_dng;
			}
			else
			{
				dungeon_removeowner_player(ch_dng, ch);
			}
		}
	}

	if( !IS_VALID(leader_dng) )
	{
		//wiznet("spawn_dungeon_player: dungeon not spawned",NULL,NULL,WIZ_TESTING,0,0);

		if( IS_NPC(leader) )
		{
			return NULL;
		}

		//wiznet("spawn_dungeon_player: creating dungeon",NULL,NULL,WIZ_TESTING,0,0);
		leader_dng = create_dungeon(pArea, vnum);

		if( !leader_dng )
		{
			//wiznet("spawn_dungeon_player: failed to create dungeon",NULL,NULL,WIZ_TESTING,0,0);
			return NULL;
		}
		//wiznet("spawn_dungeon_player: dungeon created",NULL,NULL,WIZ_TESTING,0,0);

		// 
		dungeon_addowner_player(leader_dng, leader);

		p_percent2_trigger(NULL, NULL, leader_dng, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL,0,0,0,0,0);
		ITERATOR it;
		INSTANCE *instance;
		iterator_start(&it, leader_dng->floors);
		while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
		{
			p_percent2_trigger(NULL, instance, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_REPOP, NULL,0,0,0,0,0);
		}
		iterator_stop(&it);
	}
	else
	{
		// Check whether the player can enter the dungeon
		if (!IS_SET(leader_dng->flags, DUNGEON_SHARED))
		{
			if (list_size(leader_dng->players) >= leader_dng->index->max_players)
			{
				send_to_char("The dungeon is at capacity.\n\r", ch);
				return NULL;
			}

			int size = get_groupsize_in_dungeon(leader_dng, leader);

			if (size > leader_dng->index->max_group)
			{
				send_to_char("The dungeon is at capacity.\n\r", ch);
				return NULL;
			}
		}
	}

	dungeon_addowner_player(leader_dng, ch);

	return leader_dng;
}

ROOM_INDEX_DATA *spawn_dungeon_player_floor(CHAR_DATA *ch, AREA_DATA *pArea, long vnum, int floor)
{
	DUNGEON *dng = spawn_dungeon_player(ch, pArea, vnum);

	if (!dng) return NULL;

	INSTANCE *instance = (INSTANCE *)list_nthdata(dng->floors, floor);

	if( !IS_VALID(instance) )
	{
		extract_dungeon(dng);
		return NULL;
	}

	return instance->entrance;
}

ROOM_INDEX_DATA *spawn_dungeon_player_special_room(CHAR_DATA *ch, AREA_DATA *pArea, long vnum, int special_room, char *special_room_name)
{
	DUNGEON *dng = spawn_dungeon_player(ch, pArea, vnum);

	if (!dng) return NULL;
	
	if (special_room > 0)
		return get_dungeon_special_room(dng, special_room);
	else if (!IS_NULLSTR(special_room_name))
		return get_dungeon_special_room_byname(dng, special_room_name);

	extract_dungeon(dng);
	return NULL;
}

bool dungeon_can_idle(DUNGEON *dungeon)
{
	return IS_SET(dungeon->flags, DUNGEON_DESTROY) ||
			(!IS_SET(dungeon->flags, DUNGEON_NO_IDLE) &&
				(!IS_SET(dungeon->flags, DUNGEON_IDLE_ON_COMPLETE) ||
				IS_SET(dungeon->flags, DUNGEON_COMPLETED) ||
				IS_SET(dungeon->flags, DUNGEON_FAILED)));
}

void dungeon_check_empty(DUNGEON *dungeon)
{
	if( dungeon->empty )
	{
		if( list_size(dungeon->players) > 0 )
			dungeon->empty = false;
	}
	else if( list_size(dungeon->players) < 1 )
	{
		dungeon->empty = true;
		if( dungeon_can_idle(dungeon) )
			dungeon->idle_timer = UMAX(DUNGEON_IDLE_TIMEOUT, dungeon->idle_timer);
	}

	if( !dungeon->empty && !dungeon_can_idle(dungeon) )
		dungeon->idle_timer = 0;
}

void dungeon_update()
{
	ITERATOR it;
	ITERATOR iit;
	DUNGEON *dungeon;
	INSTANCE *instance;

	iterator_start(&it, loaded_dungeons);
	while( (dungeon = (DUNGEON *)iterator_nextdata(&it)) )
	{
		if( dungeon_isorphaned(dungeon) && list_size(dungeon->players) < 1 )
		{
			// Do NOT keep an empty orphaned dungeon
			extract_dungeon(dungeon);
			continue;
		}

		p_percent2_trigger(NULL, NULL, dungeon, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL,0,0,0,0,0);

		iterator_start(&iit, dungeon->floors);
		while( (instance = (INSTANCE *)iterator_nextdata(&iit)) )
		{
			update_instance(instance);
		}
		iterator_stop(&iit);

		if( IS_SET(dungeon->flags, DUNGEON_DESTROY) || !IS_SET(dungeon->flags, DUNGEON_IDLE_ON_COMPLETE) || IS_SET(dungeon->flags, DUNGEON_COMPLETED) || IS_SET(dungeon->flags, DUNGEON_FAILED) )
		{
			if( dungeon->idle_timer > 0 )
			{
				if( !--dungeon->idle_timer )
				{
					extract_dungeon(dungeon);
					continue;
				}
			}
		}

		dungeon_check_empty(dungeon);

		dungeon->age++;
		if (dungeon->index->repop > 0 && (dungeon->age >= dungeon->index->repop) )
		{
			p_percent2_trigger(NULL, NULL, dungeon, NULL, NULL, NULL, NULL, NULL, TRIG_RESET, NULL,0,0,0,0,0);

			iterator_start(&iit, dungeon->floors);
			while( (instance = (INSTANCE *)iterator_nextdata(&iit)) )
			{
				reset_instance(instance);
			}
			iterator_stop(&iit);

			dungeon->age = 0;
		}
	}
	iterator_stop(&it);
}


/////////////////////////////////////////
//
// DUNGEON EDITOR
//

const struct olc_cmd_type dngedit_table[] =
{
	{ "?",				show_help			},
	{ "adddprog",		dngedit_adddprog	},
	{ "areawho",		dngedit_areawho		},
	{ "commands",		show_commands		},
	{ "comments",		dngedit_comments	},
	{ "create",			dngedit_create		},
	{ "deldprog",		dngedit_deldprog	},
	{ "description",	dngedit_description	},
	{ "entry",			dngedit_entry		},
	{ "exit",			dngedit_exit		},
	{ "flags",			dngedit_flags		},
	{ "floors",			dngedit_floors		},
	{ "groupsize",		dngedit_groupsize	},
	{ "levels",			dngedit_levels		},
	{ "list",			dngedit_list		},
	{ "maxplayers",		dngedit_maxplayers	},
	{ "mountout",		dngedit_mountout	},
	{ "name",			dngedit_name		},
	{ "release",		dngedit_release		},
	{ "portalout",		dngedit_portalout	},
	{ "scripted",		dngedit_scripted	},
	{ "show",			dngedit_show		},
	{ "special",		dngedit_special		},
	{ "varclear",		dngedit_varclear	},
	{ "varset",			dngedit_varset		},
	{ "zoneout",		dngedit_zoneout		},
	{ NULL,				NULL				}

};

void list_dungeons(CHAR_DATA *ch, char *argument)
{
	if(!ch->lines)
		send_to_char("{RWARNING:{W Having scrolling off may limit how many dungeons you can see.{x\n\r", ch);

	AREA_DATA *area = ch->in_room->area;
	int lines = 0;
	bool error = false;
	BUFFER *buffer = new_buf();
	char buf[MSL];

	for(long vnum = 1; vnum <= area->top_dungeon_vnum; vnum++)
	{
		DUNGEON_INDEX_DATA *dng = get_dungeon_index(area, vnum);

		if( dng )
		{
			sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s\n\r",
				vnum,
				dng->name);

			++lines;
			if( !add_buf(buffer, buf) || (!ch->lines && strlen(buf_string(buffer)) > MAX_STRING_LENGTH) )
			{
				error = true;
				break;
			}
		}
	}

	if( error )
	{
		send_to_char("Too many dungeons to list.  Please shorten!\n\r", ch);
	}
	else
	{
		if( !lines )
		{
			add_buf( buffer, "No dungeons to display.\n\r" );
		}
		else
		{
			// Header
			send_to_char("Dungeons in current area.\n\r", ch);
			send_to_char("{Y Vnum   [            Name            ]{x\n\r", ch);
			send_to_char("{Y======================================={x\n\r", ch);

			page_to_char(buffer->string, ch);
		}
	}
	free_buf(buffer);
}

void do_dnglist(CHAR_DATA *ch, char *argument)
{
	list_dungeons(ch, argument);
}

void do_dngedit(CHAR_DATA *ch, char *argument)
{
	DUNGEON_INDEX_DATA *dng;
	WNUM wnum;
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	if (IS_NPC(ch))
		return;

	if (parse_widevnum(arg1, ch->in_room->area, &wnum))
	{
		if (!wnum.pArea || wnum.vnum < 1)
		{
			send_to_char("Widevnum not associated with an area.\n\r", ch);
			return;
		}

	    if (!IS_BUILDER(ch, wnum.pArea))
		{
			send_to_char("DngEdit:  widevnum in an area you cannot build in.\n\r", ch);
			return;
		}

		if (!(dng = get_dungeon_index(wnum.pArea, wnum.vnum)))
		{
			send_to_char("DNGEdit:  That dungeon does not exist.\n\r", ch);
			return;
		}

		ch->pcdata->immortal->last_olc_command = current_time;
		olc_set_editor(ch, ED_DUNGEON, dng);
		return;
	}
	else
	{
		if (!str_cmp(arg1, "create"))
		{
			if (dngedit_create(ch, argument))
			{
				ch->pcdata->immortal->last_olc_command = current_time;
				ch->desc->editor = ED_DUNGEON;

				EDIT_DUNGEON(ch, dng);
				SET_BIT(dng->area->area_flags, AREA_CHANGED);
			}

			return;
		}

	}

	send_to_char("Syntax: dngedit <widevnum>\n\r"
				 "        dngedit create <widevnum>\n\r", ch);
}

void dngedit(CHAR_DATA *ch, char *argument)
{
	DUNGEON_INDEX_DATA *dng;
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  cmd;

	EDIT_DUNGEON(ch, dng);

	smash_tilde(argument);
	strcpy(arg, argument);
	argument = one_argument(argument, command);

	if (!str_cmp(command, "done"))
	{
		edit_done(ch);
		return;
	}

    ch->pcdata->immortal->last_olc_command = current_time;

	if (command[0] == '\0')
	{
		dngedit_show(ch, argument);
		return;
	}

	for (cmd = 0; dngedit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix(command, dngedit_table[cmd].name))
		{
			if ((*dngedit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT(dng->area->area_flags, AREA_CHANGED);
			}

			return;
		}
	}

    interpret(ch, arg);
}


DUNGEON_INDEX_LEVEL_DATA *dungeon_index_get_nth_level(DUNGEON_INDEX_DATA *dng, int level_no, DUNGEON_INDEX_LEVEL_DATA **in_group)
{
	if (!IS_VALID(dng) || !level_no) return NULL;

	if (in_group) *in_group = NULL;

	DUNGEON_INDEX_LEVEL_DATA *level = NULL;
	if (level_no < 0)
	{
		int ordinal = -level_no;
		ITERATOR it;
		DUNGEON_INDEX_LEVEL_DATA *lvl;

		iterator_start(&it, dng->levels);
		while((lvl = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)))
		{
			if (lvl->ordinal == ordinal)
			{
				level = lvl;
				break;
			}
			else if(lvl->mode == LEVELMODE_GROUP)
			{
				ITERATOR git;
				DUNGEON_INDEX_LEVEL_DATA *glvl;

				iterator_start(&git, lvl->group);
				while((glvl = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)))
				{
					if (glvl->ordinal == ordinal)
					{
						level = glvl;
						if(in_group) *in_group = lvl;
						break;
					}
				}
				iterator_stop(&git);

				if (level)
					break;
			}
		}
		iterator_stop(&it);
	}
	else
	{
		ITERATOR it;
		DUNGEON_INDEX_LEVEL_DATA *lvl;

		iterator_start(&it, dng->levels);
		while((lvl = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)))
		{
			if (lvl->mode == LEVELMODE_GROUP)
			{
				ITERATOR git;
				DUNGEON_INDEX_LEVEL_DATA *glvl;

				iterator_start(&git, lvl->group);
				while((glvl = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)))
				{
					if (level_no == 1)
					{
						level = glvl;
						if(in_group) *in_group = lvl;
						break;
					}

					level_no--;
				}
				iterator_stop(&git);

				if(level)
					break;
			}
			else if(level_no == 1)
			{
				level = lvl;
				break;
			}
			else
				level_no--;
		}
		iterator_stop(&it);
	}

	return level;
}

BLUEPRINT *dungeon_index_get_representative_blueprint(DUNGEON_INDEX_DATA *dng, int level_no, bool *exact)
{
	DUNGEON_INDEX_LEVEL_DATA *in_group = NULL;
	DUNGEON_INDEX_LEVEL_DATA *lvl = dungeon_index_get_nth_level(dng, level_no, &in_group);

	if(IS_VALID(lvl) || lvl->mode == LEVELMODE_GROUP) return NULL;

	if(lvl->mode == LEVELMODE_STATIC)
	{
		if (exact) *exact = !in_group;
		return (BLUEPRINT *)list_nthdata(dng->floors, lvl->floor);
	}
	else
	{
		ITERATOR wit;
		DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
		DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *most_likely = NULL;

		iterator_start(&wit, lvl->weighted_floors);
		while((weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
		{
			if(!most_likely || weighted->weight > most_likely->weight)
			{
				most_likely = weighted;
			}
		}
		iterator_stop(&wit);

		if(!most_likely) return NULL;

		if(exact) *exact = false;
		return (BLUEPRINT *)list_nthdata(dng->floors, most_likely->floor);
	}
}

void dngedit_buffer_floors(BUFFER *buffer, DUNGEON_INDEX_DATA *dng)
{
	char buf[MSL];

	if( list_size(dng->floors) > 0 )
	{
		ITERATOR fit;
		BLUEPRINT *bp;

		add_buf(buffer, "{gFloors:{x\n\r");
		add_buf(buffer, "{g     [  Vnum  ] [             Name             ]\n\r");
		add_buf(buffer, "{g=================================================\n\r");

		int floor = 0;
		iterator_start(&fit, dng->floors);
		while( (bp = (BLUEPRINT *)iterator_nextdata(&fit)) )
		{
			sprintf(buf, "{W%4d  {G%8ld   {x%-.30s{x\n\r", ++floor, bp->vnum, bp->name);
			add_buf(buffer, buf);
		}
		iterator_stop(&fit);
		add_buf(buffer, "=================================================\n\r");
	}
	else
	{
		add_buf(buffer, "{gFloors:{x\n\r");
		add_buf(buffer, "   None\n\r");
	}
}

void dngedit_buffer_levels(BUFFER *buffer, DUNGEON_INDEX_DATA *dng)
{
	char buf[MSL];

	add_buf(buffer, "{yLevels:{x\n\r");
	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		add_buf(buffer, "  {WSCRIPTED{x\n\r");
	}
	else if (list_size(dng->levels) > 0)
	{
		ITERATOR it;
		DUNGEON_INDEX_LEVEL_DATA *level;

		add_buf(buffer, "{y     [  Mode  ] [ Ordinal ]{x\n\r");
		add_buf(buffer, "{y===================================================={x\n\r");

		int levelno = 1;
		iterator_start(&it, dng->levels);
		while( (level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)) )
		{
			switch(level->mode)
			{
				case LEVELMODE_STATIC:
				{
					BLUEPRINT *bp = (BLUEPRINT *)list_nthdata(dng->floors, level->floor);
					sprintf(buf, "{W%4d  {Y STATIC {x   %7d    %4d - {x%23.23s{x\n\r", levelno++, level->ordinal, level->floor, bp->name);
					add_buf(buffer, buf);
					break;
				}

				case LEVELMODE_WEIGHTED:
				{
					ITERATOR wit;
					DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
					BLUEPRINT *bp;

					sprintf(buf, "{W%4d  {CWEIGHTED{x   %7d\n\r", levelno++, level->ordinal);
					add_buf(buffer, buf);
					add_buf(buffer, "{c               [ Weight ] [              Floor             ]{x\n\r");
					add_buf(buffer, "{c          ==================================================={x\n\r");

					int weightno = 1;
					iterator_start(&wit, level->weighted_floors);
					while( (weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)) )
					{
						
						bp = (BLUEPRINT *)list_nthdata(dng->floors, weighted->floor);
						sprintf(buf, "          {c%4d   {W%6d     {x%4d - %23.23s{x\n\r", weightno++, weighted->weight, weighted->floor, bp->name);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "{c          ----------------------------------------------------{x\n\r");
					break;
				}

				case LEVELMODE_GROUP:
				{
					ITERATOR git;
					DUNGEON_INDEX_LEVEL_DATA *lvl;

					sprintf(buf, "{W%4d  {G  GROUP {x\n\r", levelno++);
					add_buf(buffer, buf);

					if (list_size(level->group) > 0)
					{
						add_buf(buffer, "          {y     [  Mode  ]{x\n\r");
						add_buf(buffer, "          {y===================================================={x\n\r");

						int glevelno = 1;
						iterator_start(&git, level->group);
						while( (lvl = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)) )
						{
							if (lvl->mode == LEVELMODE_STATIC)
							{
								BLUEPRINT *bp = (BLUEPRINT *)list_nthdata(dng->floors, lvl->floor);
								sprintf(buf, "          {W%4d  {Y STATIC {x     %4d - {x%23.23s{x\n\r", glevelno++, lvl->floor, bp->name);
								add_buf(buffer, buf);	
							}
							else if(lvl->mode == LEVELMODE_WEIGHTED)
							{
								ITERATOR wit;
								DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
								BLUEPRINT *bp;

								sprintf(buf, "          {W%4d  {CWEIGHTED{x\n\r", glevelno++);
								add_buf(buffer, buf);
								add_buf(buffer, "{c                         [ Weight ] [              Floor             ]{x\n\r");
								add_buf(buffer, "{c                    ==================================================={x\n\r");

								int weightno = 1;
								iterator_start(&wit, level->weighted_floors);
								while( (weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)) )
								{
									
									bp = (BLUEPRINT *)list_nthdata(dng->floors, weighted->floor);
									sprintf(buf, "                    {c%4d   {W%6d     {x%4d - %23.23s{x\n\r", weightno++, weighted->weight, weighted->floor, bp->name);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "{c                    ----------------------------------------------------{x\n\r");
							}
						}
						iterator_stop(&git);
					}
					else
						add_buf(buffer, "          none\n\r");

					break;
				}
			}
		}
		iterator_stop(&it);
		add_buf(buffer, "{y----------------------------------------------------{x\n\r");
	}
	else
	{
		add_buf(buffer, "  None\n\r");
	}
}

void dngedit_buffer_special_exits(BUFFER *buffer, DUNGEON_INDEX_DATA *dng)
{
	char buf[MSL];

	add_buf(buffer, "{xSpecial Exits:{x\n\r");
	if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
	{
		add_buf(buffer, "  {WSCRIPTED{x\n\r");
	}
	else if(list_size(dng->special_exits) > 0)
	{
		ITERATOR it;
		DUNGEON_INDEX_SPECIAL_EXIT *dsex;

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
					sprintf(buf, "%4d  {Y   STATIC   {x\n\r", exitno++);
					sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
					add_buf(buffer, buf);
					sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), to->level, to->door);
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

					sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), to->level, to->door);
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
									sprintf(buf, "                    Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), to->level, to->door);
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

									sprintf(buf, "                    Destination:   {%c%4d{x (%d)\n\r", (to->level<0?'G':(to->level>0?'Y':'W')), to->level, to->door);
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

									sprintf(buf, "                    Source:        {%c%4d{x (%d)\n\r", (from->level<0?'G':(from->level>0?'Y':'W')), abs(from->level), from->door);
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
	}
	else
	{
		add_buf(buffer, "  None\n\r");
	}


}



void do_dngshow(CHAR_DATA *ch, char *argument)
{
	DUNGEON_INDEX_DATA *dng;
	WNUM wnum;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  dngshow <widevnum>\n\r", ch);
		return;
	}

	if (!parse_widevnum(argument, ch->in_room->area, &wnum))
	{
		send_to_char("Please specify a widevnum.\n\r", ch);
		return;
	}

	if (!(dng= get_dungeon_index(wnum.pArea, wnum.vnum)))
	{
		send_to_char("That dungeon does not exist.\n\r", ch);
		return;
	}

	olc_show_item(ch, dng, dngedit_show, argument);
	return;
}



void dungeon_update_level_ordinals(DUNGEON_INDEX_DATA *dng)
{
	int ordinal = 1;
	ITERATOR it;
	DUNGEON_INDEX_LEVEL_DATA *level;

	iterator_start(&it, dng->levels);
	while((level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)))
	{
		if (level->mode == LEVELMODE_GROUP)
		{
			level->ordinal = 0;	// Groups don't have an ordinal on themselves
			ITERATOR git;
			DUNGEON_INDEX_LEVEL_DATA *glevel;
			iterator_start(&git, level->group);
			while((glevel = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)))
			{
				glevel->ordinal = ordinal++;
			}
			iterator_stop(&git);
		}
		else
			level->ordinal = ordinal++;
	}

	iterator_stop(&it);
}



static int get_blueprint_entrance_count(BLUEPRINT *bp)
{
	return list_size(bp->entrances);
}

static int get_blueprint_exit_count(BLUEPRINT *bp)
{
	return list_size(bp->exits);
}

int dungeon_index_generation_count(DUNGEON_INDEX_DATA *dng)
{
	int count = 0;
	ITERATOR it;
	DUNGEON_INDEX_LEVEL_DATA *level;
	iterator_start(&it, dng->levels);
	while((level = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&it)))
	{
		if (level->mode == LEVELMODE_GROUP)
			count = list_size(level->group);
		else
			count++;
	}
	iterator_stop(&it);
	return count;
}

int get_dungeon_index_level_special_entrances(DUNGEON_INDEX_DATA *dng, DUNGEON_INDEX_LEVEL_DATA *level)
{
	BLUEPRINT *bp;
	switch(level->mode)
	{
	case LEVELMODE_STATIC:
		bp = (BLUEPRINT *)list_nthdata(dng->floors, level->floor);

		return get_blueprint_entrance_count(bp);

	case LEVELMODE_WEIGHTED:
		{
			int min_count = -1;

			ITERATOR wit;
			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
			iterator_start(&wit, level->weighted_floors);
			while( (weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
			{
				bp = (BLUEPRINT *)list_nthdata(dng->floors, weighted->floor);

				int count = get_blueprint_entrance_count(bp);

				if (min_count < 0 || count < min_count)
					min_count = count;
			}
			iterator_stop(&wit);
			return UMAX(0, min_count);
		}

	case LEVELMODE_GROUP:
		{
			int min_count = -1;

			ITERATOR git;
			DUNGEON_INDEX_LEVEL_DATA *glevel;
			iterator_start(&git, level->group);
			while( (glevel = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)))
			{
				int count = get_dungeon_index_level_special_entrances(dng, glevel);

				if (min_count < 0 || count < min_count)
					min_count = count;
			}
			iterator_stop(&git);
			return UMAX(0, min_count);
		}
	}

	return 0;
}


int get_dungeon_index_level_special_exits(DUNGEON_INDEX_DATA *dng, DUNGEON_INDEX_LEVEL_DATA *level)
{
	BLUEPRINT *bp;
	switch(level->mode)
	{
	case LEVELMODE_STATIC:
		bp = (BLUEPRINT *)list_nthdata(dng->floors, level->floor);

		return get_blueprint_exit_count(bp);

	case LEVELMODE_WEIGHTED:
		{
			int min_count = -1;

			ITERATOR wit;
			DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *weighted;
			iterator_start(&wit, level->weighted_floors);
			while( (weighted = (DUNGEON_INDEX_WEIGHTED_FLOOR_DATA *)iterator_nextdata(&wit)))
			{
				bp = (BLUEPRINT *)list_nthdata(dng->floors, weighted->floor);

				int count = get_blueprint_exit_count(bp);

				if (min_count < 0 || count < min_count)
					min_count = count;
			}
			iterator_stop(&wit);
			return UMAX(0, min_count);
		}

	case LEVELMODE_GROUP:
		{
			int min_count = -1;

			ITERATOR git;
			DUNGEON_INDEX_LEVEL_DATA *glevel;
			iterator_start(&git, level->group);
			while( (glevel = (DUNGEON_INDEX_LEVEL_DATA *)iterator_nextdata(&git)))
			{
				int count = get_dungeon_index_level_special_exits(dng, glevel);

				if (min_count < 0 || count < min_count)
					min_count = count;
			}
			iterator_stop(&git);
			return UMAX(0, min_count);
		}
	}

	return 0;
}

void add_dungeon_index_weighted_exit_data(LLIST *list, int weight, int level, int door)
{
	DUNGEON_INDEX_WEIGHTED_EXIT_DATA *weighted = new_weighted_random_exit();

	weighted->weight = weight;
	weighted->level = level;
	weighted->door = door;

	list_appendlink(list, weighted);
}




//////////////////////////////////////////////////////////////
//
// Commands
//
void do_dungeon(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL];

	if( IS_NPC(ch) ) return;

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  dungeon leave\n\r", ch);
		if (IS_DEAD(ch))
			send_to_char("         dungeon release\n\r", ch);
		if( !IS_IMPLEMENTOR(ch) )
		{
			send_to_char("         dungeon list\n\r", ch);
			send_to_char("         dungeon unload #\n\r", ch);
		}
		return;
	}

	argument = one_argument(argument, arg1);

	if( !str_prefix(arg1, "leave") )
	{
		DUNGEON *dungeon = get_room_dungeon(ch->in_room);

		if( !IS_VALID(dungeon) )
		{
			send_to_char("You are not in a dungeon.\n\r", ch);
			return;
		}

		CHAR_DATA *leader = get_player_leader(ch);
		if (leader->pcdata->last_ready_check > 0)
		{
			send_to_char("You may not leave yet until the ready check is finished.\n\r", ch);
			return;
		}

		// Prevent them from just up and leaving the dungeon mid... anything
		if( ch->position != POS_STANDING )
		{
			switch( ch->position )
			{
			case POS_DEAD:
				send_to_char( "Lie still; you are DEAD.\n\r", ch );
				break;

			case POS_MORTAL:
			case POS_INCAP:
				send_to_char( "You are far too hurt for that.\n\r", ch );
				break;

			case POS_STUNNED:
				send_to_char( "You are too stunned to do that.\n\r", ch );
				break;

			case POS_SLEEPING:
				send_to_char( "In your dreams, or what?\n\r", ch );
				break;

			case POS_RESTING:
				send_to_char( "You are resting at the moment.\n\r", ch);
				break;

			case POS_SITTING:
				send_to_char( "Better stand up first.\n\r",ch);
				break;

			case POS_FIGHTING:
				send_to_char( "No way!  You are still fighting!\n\r", ch);
				break;
			}

			return;
		}

		ROOM_INDEX_DATA *room = NULL;

		if (location_isset(&ch->before_dungeon))
		{
			room = location_to_room(&ch->before_dungeon);
			location_clear(&ch->before_dungeon);
		}

		// No previous room, so exit the dungeon
		if (!room) room = dungeon->entry_room;

		// No room, get the TEMPLE
		if( !room ) room = room_index_temple;

		// Should deal with their mount and pet if they have one
		char_from_room(ch);
		char_to_room(ch, room);

		if (ch->pet != NULL)
		{
			char_from_room (ch->pet);
			char_to_room(ch->pet, room);
		}

		act("{Y$n leaves $T.{x", ch, NULL, NULL, NULL, NULL, NULL, dungeon->index->name, TO_ROOM, NULL, NULL);
		act("{YYou leave $T.{x", ch, NULL, NULL, NULL, NULL, NULL, dungeon->index->name, TO_CHAR, NULL, NULL);
		do_function(ch, &do_look, "auto");
		return;
	}

	if (!str_prefix(arg1, "release"))
	{
		INSTANCE *floor;

		if (!IS_DEAD(ch))
		{
			send_to_char("Huh?\n\r", ch);
			return;
		}

		if (!ch->can_release)
		{
			send_to_char("You cannot release at this time.\n\r", ch);
			return;
		}

		DUNGEON *dungeon = get_room_dungeon(ch->in_room);
		if (!IS_VALID(dungeon))
		{
			send_to_char("You are not in a dungeon.\n\r", ch);
			return;
		}

		switch(dungeon->index->death_release)
		{
			// Standard death.  Either respawn as a newbie or go to the death plane
			case DEATH_RELEASE_NORMAL:

				// Newbie death release
				if ((ch->tot_level < 10) && !IS_REMORT(ch))
				{
					// Set the recall point
					location_from_room(&ch->recall,room_index_newbie_death);
					// Resurrect to take care of any aftermath of being dead
					resurrect_pc(ch);

					send_to_char("\n\r{yYou wake up in a dazed state... maybe you weren't dead after all.\n\r{x", ch);
					send_to_char("You notice that you are safe and healthy once again.\n\r", ch);

					ch->position = POS_RESTING;
					ch->hit  = ch->max_hit;
					ch->mana = ch->max_mana;
					ch->move = ch->max_move;
				}
				else
				{
					// Standard death timer
					ch->time_left_death = MINS_PER_DEATH + 1;

					ch->hit = ch->max_hit;
					ch->mana = ch->max_mana;
					ch->move = ch->max_move;
					char_from_room(ch);
					char_to_room(ch, room_index_death);
					
					send_to_char("You release your spirit to the plane of death.\n\r", ch);
				}
				return;
			
			case DEATH_RELEASE_TO_START:
				floor = (INSTANCE *)list_nthdata(dungeon->floors, 1);

				location_from_room(&ch->recall, floor->entrance);
				break;

			case DEATH_RELEASE_TO_FLOOR:
				// This has to be valid for the "dungeon" to be valid
				floor = ch->in_room->instance_section->instance;

				location_from_room(&ch->recall, floor->entrance);
				break;
			
			// If there is no checkpoint, go to the start
			case DEATH_RELEASE_TO_CHECKPOINT:
				if (ch->checkpoint)
				{
					location_from_room(&ch->recall, ch->checkpoint);
				}
				else
				{
					floor = (INSTANCE *)list_nthdata(dungeon->floors, 1);

					location_from_room(&ch->recall, floor->entrance);
				}
				break;
		}

		// Restore the player
		resurrect_pc(ch);

		// Auto look
		do_function(ch, &do_look, "auto");

		return;
	}


	if( IS_IMPLEMENTOR(ch) )
	{
		if( !str_prefix(arg1, "list") )
		{

			if(!ch->lines)
				send_to_char("{RWARNING:{W Having scrolling off may limit how many dungeons you can see.{x\n\r", ch);

			int lines = 0;
			bool error = false;
			BUFFER *buffer = new_buf();
			char buf[MSL];


			ITERATOR it;
			DUNGEON *dungeon;

			iterator_start(&it, loaded_dungeons);
			while((dungeon = (DUNGEON *)iterator_nextdata(&it)))
			{
				//sprintf(buf, "dungeon list: %ld, %s", dungeon->index->vnum, dungeon->index->name);
				//wiznet(buf,NULL,NULL,WIZ_TESTING,0,0);

				char plr_str[21];
				char idle_str[21];
				++lines;

				int players = list_size(dungeon->players);

				if( players > 0 )
				{
					snprintf(plr_str, 20, "{W%d", players);
					plr_str[20] = '\0';
				}
				else
				{
					strcpy(plr_str, "{Dempty");
				}

				if( (IS_SET(dungeon->flags, DUNGEON_DESTROY) || !IS_SET(dungeon->flags, DUNGEON_IDLE_ON_COMPLETE) || IS_SET(dungeon->flags, DUNGEON_COMPLETED|DUNGEON_FAILED)) &&
					dungeon->idle_timer > 0 )
				{
					snprintf(idle_str, 20, "{G%d", dungeon->idle_timer);
					idle_str[20] = '\0';
				}
				else
				{
					strcpy(idle_str, "{YActive");
				}

				char color = 'G';

				if( IS_SET(dungeon->flags, DUNGEON_DESTROY) )
					color = 'D';
				else if( IS_SET(dungeon->flags, DUNGEON_FAILED) )
					color = 'R';
				else if( IS_SET(dungeon->flags, DUNGEON_COMPLETED) )
					color = 'W';

				sprintf(buf, "%4d {Y[{W%5ld{Y] {%c%-30.30s   %13.13s   %8.8s{x\n\r",
					lines,
					dungeon->index->vnum,
					color, dungeon->index->name,
					plr_str, idle_str);

				if( !add_buf(buffer, buf) || (!ch->lines && strlen(buf_string(buffer)) > MAX_STRING_LENGTH) )
				{
					error = true;
					break;
				}
			}
			iterator_stop(&it);

			if( error )
			{
				send_to_char("Too many dungeons to list.  Please shorten!\n\r", ch);
			}
			else
			{
				if( !lines )
				{
					add_buf( buffer, "No dungeons to display.\n\r" );
				}
				else
				{
					// Header
					send_to_char("{Y      Vnum   [            Name            ] [  Players  ] [ Idle ]{x\n\r", ch);
					send_to_char("{Y==================================================================={x\n\r", ch);
				}

				page_to_char(buffer->string, ch);
			}
			free_buf(buffer);

			return;
		}

		if( !str_prefix(arg1, "unload") )
		{
			char buf[MSL];

			if( !can_edit_dungeons(ch) )
			{
				send_to_char("Insufficient access to unload dungeons.\n\r", ch);
				return;
			}

			if( !is_number(argument) )
			{
				send_to_char("That is not a number.\n\r", ch);
				return;
			}

			int index = atoi(argument);

			if( list_size(loaded_dungeons) < index )
			{
				send_to_char("No dungeon at that index.\n\r", ch);
				return;
			}

			// Set a flag on the dungeon and set the idle timer
			DUNGEON *dungeon = (DUNGEON *)list_nthdata(loaded_dungeons, index);

			if( list_size(dungeon->players) > 0 )
			{
				if( IS_SET(dungeon->flags, DUNGEON_DESTROY) )
				{
					send_to_char("Dungeon is already flagged for unloading.\n\r", ch);
					return;
				}

				SET_BIT(dungeon->flags, DUNGEON_DESTROY);
				if( dungeon->idle_timer > 0 )
					dungeon->idle_timer = UMIN(DUNGEON_DESTROY_TIMEOUT, dungeon->idle_timer);
				else
					dungeon->idle_timer = DUNGEON_DESTROY_TIMEOUT;

				sprintf(buf, "{RWARNING: Dungeon is being forcibly unloaded.  You have %d minutes to escape before the end!{x\n\r", dungeon->idle_timer);
				dungeon_echo(dungeon, buf);

				send_to_char("Dungeon flagged for unloading.\n\r", ch);
			}
			else
			{
				extract_dungeon(dungeon);
				send_to_char("Dungeon unloaded.\n\r", ch);
			}
			return;
		}
	}

	do_dungeon(ch, "");
	return;
}

//////////////////////////////////////////////////////////
//
// Dungeon Save/Load
//
void dungeon_save(FILE *fp, DUNGEON *dungeon)
{
	ITERATOR it;
	INSTANCE *instance;
	LLIST_UID_DATA *luid;

	fprintf(fp, "#DUNGEON %ld#%ld\n\r", dungeon->index->area->uid, dungeon->index->vnum);
	fprintf(fp, "Uid %ld %ld\n\r", dungeon->uid[0], dungeon->uid[1]);
	// ->entry_room - not saved... resolved on load
	// ->exit_room - not saved...  resolved on load

	fprintf(fp, "Flags %d\n\r", dungeon->flags);

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		fprintf(fp, "Player %lu %lu\n\r", luid->id[0], luid->id[1]);
	}
	iterator_stop(&it);

	if( dungeon->idle_timer > 0 )
	{
		fprintf(fp, "IdleTimer %d\n\r", dungeon->idle_timer);
	}

	iterator_start(&it, dungeon->floors);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		instance_save(fp, instance);
	}
	iterator_stop(&it);


	fprintf(fp, "#-DUNGEON\n\r");
}

void dungeon_tallyentities(DUNGEON *dungeon, INSTANCE *instance)
{
	list_appendlist(dungeon->mobiles, instance->mobiles);
	list_appendlist(dungeon->objects, instance->objects);
	list_appendlist(dungeon->rooms, instance->rooms);
	list_appendlist(dungeon->bosses, instance->bosses);
}

DUNGEON *dungeon_load(FILE *fp)
{
	char *word;
	bool fMatch;

	DUNGEON *dungeon = new_dungeon();
	WNUM_LOAD wnum = fread_widevnum(fp, 0);

	dungeon->index = get_dungeon_index_auid(wnum.auid, wnum.vnum);

	dungeon->progs			= new_prog_data();
	dungeon->progs->progs	= dungeon->index->progs;
	variable_copylist(&dungeon->index->index_vars,&dungeon->progs->vars,false);


	dungeon->entry_room = get_room_index(dungeon->index->area,dungeon->index->entry_room);
	dungeon->exit_room = get_room_index(dungeon->index->area,dungeon->index->exit_room);

	while (str_cmp((word = fread_word(fp)), "#-DUNGEON"))
	{
		fMatch = false;

		switch(word[0])
		{
		case '#':
			if( !str_cmp(word, "#INSTANCE") )
			{
				INSTANCE *instance = instance_load(fp);

				if( instance )
				{
					instance->dungeon = dungeon;
					list_appendlink(dungeon->floors, instance);

					dungeon_tallyentities(dungeon, instance);
				}

				fMatch = true;
				break;
			}

			break;

		case 'F':
			KEY("Flags", dungeon->flags, fread_number(fp));
			break;

		case 'I':
			KEY("IdleTimer", dungeon->idle_timer, fread_number(fp));
			break;

		case 'P':
			if( !str_cmp(word, "Player") )
			{
				unsigned long id1 = fread_number(fp);
				unsigned long id2 = fread_number(fp);

				dungeon_addowner_playerid(dungeon, id1, id2);

				fMatch = true;
				break;
			}
			break;

		case 'U':
			if( !str_cmp(word, "Uid") )
			{
				dungeon->uid[0] = fread_number(fp);
				dungeon->uid[1] = fread_number(fp);

				fMatch = true;
				break;
			}

			break;
		}

		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "dungeon_load: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	get_dungeon_id(dungeon);

	//log_stringf("dungeon_load: dungeon %ld loaded", dungeon->index->vnum);
	return dungeon;
}

void resolve_dungeon_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == ch->id[0] && luid->id[1] == ch->id[1])
		{
			luid->ptr = ch;
			break;
		}
	}
	iterator_stop(&it);
}

void resolve_dungeons_player(CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	DUNGEON *dungeon;
	iterator_start(&it, loaded_dungeons);
	while( (dungeon = (DUNGEON *)iterator_nextdata(&it)) )
	{
		resolve_dungeon_player(dungeon, ch);
	}
	iterator_stop(&it);

}

void detach_dungeon_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == ch->id[0] && luid->id[1] == ch->id[1])
		{
			luid->ptr = NULL;
			break;
		}
	}
	iterator_stop(&it);
}

void detach_dungeons_player(CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	DUNGEON *dungeon;
	iterator_start(&it, loaded_dungeons);
	while( (dungeon = (DUNGEON *)iterator_nextdata(&it)) )
	{
		detach_dungeon_player(dungeon, ch);

		// Check player quests
	}
	iterator_stop(&it);

}

void dungeon_echo(DUNGEON *dungeon, char *text)
{
	if( !IS_VALID(dungeon) || IS_NULLSTR(text) ) return;

	ITERATOR it;
	CHAR_DATA *ch;

	iterator_start(&it, dungeon->players);
	while( (ch = (CHAR_DATA *)iterator_nextdata(&it)) )
	{
		send_to_char(text, ch);
		send_to_char("\n\r", ch);
	}
	iterator_stop(&it);
}

ROOM_INDEX_DATA *dungeon_random_room(CHAR_DATA *ch, DUNGEON *dungeon)
{
	if( !IS_VALID(dungeon) ) return NULL;

	return get_random_room_list_byflags( ch, dungeon->rooms,
		(ROOM_PRIVATE | ROOM_SOLITARY | ROOM_DEATH_TRAP | ROOM_CHAOTIC),
		ROOM_NO_GET_RANDOM );
}

DUNGEON *get_room_dungeon(ROOM_INDEX_DATA *room)
{
	if( !room ) return NULL;

	if( !IS_VALID(room->instance_section) ) return NULL;

	if( !IS_VALID(room->instance_section->instance) ) return NULL;

	if( !IS_VALID(room->instance_section->instance->dungeon) ) return NULL;

	return room->instance_section->instance->dungeon;
}

OBJ_DATA *get_room_dungeon_portal(ROOM_INDEX_DATA *room, WNUM wnum)
{
	OBJ_DATA *obj;

	if( get_room_dungeon(room) ) return NULL;

	for(obj = room->contents; obj; obj = obj->next_content)
	{
		if( IS_PORTAL(obj) &&
			PORTAL(obj)->type == GATETYPE_DUNGEON &&
			obj->pIndexData->area == wnum.pArea &&
			PORTAL(obj)->params[0] == wnum.vnum)
		{
			return obj;
		}
	}

	return NULL;
}

ROOM_INDEX_DATA *get_dungeon_special_room(DUNGEON *dungeon, int index)
{
	if( !IS_VALID(dungeon) || index < 1) return NULL;

	NAMED_SPECIAL_ROOM *special = list_nthdata(dungeon->special_rooms, index);

	if( IS_VALID(special) )
		return special->room;

	return NULL;
}

ROOM_INDEX_DATA *get_dungeon_special_room_byname(DUNGEON *dungeon, char *name)
{
	int number;
	char arg[MSL];

	if( !IS_VALID(dungeon) ) return NULL;

	number = number_argument(name, arg);

	if( number < 1 ) return NULL;

	ITERATOR it;
	ROOM_INDEX_DATA *room = NULL;
	NAMED_SPECIAL_ROOM *special;
	iterator_start(&it, dungeon->special_rooms);
	while( (special = (NAMED_SPECIAL_ROOM *)iterator_nextdata(&it)) )
	{
		if( is_name(arg, special->name) )
		{
			if( !--number )
			{
				room = special->room;
				break;
			}
		}
	}
	iterator_stop(&it);

	return room;
}

int dungeon_count_mob(DUNGEON *dungeon, MOB_INDEX_DATA *pMobIndex)
{
	ITERATOR it;
	INSTANCE *instance;

	if( !IS_VALID(dungeon) || !pMobIndex ) return 0;

	int count = 0;
	iterator_start(&it, dungeon->floors);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		count += instance_count_mob(instance, pMobIndex);
	}
	iterator_stop(&it);

	return count;
}

void dungeon_addowner_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	// Don't add twice
	if( dungeon_isowner_player(dungeon, ch) ) return;

	LLIST_UID_DATA *luid = new_list_uid_data();
	luid->id[0] = ch->id[0];
	luid->id[1] = ch->id[1];
	luid->ptr = ch;

	list_appendlink(dungeon->player_owners, luid);
}

void dungeon_addowner_playerid(DUNGEON *dungeon, unsigned long id1, unsigned long id2)
{
	// Don't add twice
	if( dungeon_isowner_playerid(dungeon, id1, id2) ) return;

	LLIST_UID_DATA *luid = new_list_uid_data();
	luid->id[0] = id1;
	luid->id[1] = id2;
	luid->ptr = NULL;

	list_appendlink(dungeon->player_owners, luid);
}

void dungeon_removeowner_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == ch->id[0] && luid->id[1] == ch->id[1] )
		{
			iterator_remcurrent(&it);
			break;
		}
	}
	iterator_stop(&it);
}

void dungeon_removeowner_playerid(DUNGEON *dungeon, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == id1 && luid->id[1] == id2 )
		{
			iterator_remcurrent(&it);
			break;
		}
	}
	iterator_stop(&it);
}

bool dungeon_isowner_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return false;

	ITERATOR it;
	LLIST_UID_DATA *luid;
	bool ret = false;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == ch->id[0] && luid->id[1] == ch->id[1] )
		{
			ret = true;
			break;
		}
	}
	iterator_stop(&it);

	return ret;
}

bool dungeon_isowner_playerid(DUNGEON *dungeon, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	LLIST_UID_DATA *luid;
	bool ret = false;

	iterator_start(&it, dungeon->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == id1 && luid->id[1] == id2)
		{
			ret = true;
			break;
		}
	}
	iterator_stop(&it);

	return ret;
}

bool dungeon_canswitch_player(DUNGEON *dungeon, CHAR_DATA *ch)
{
	// TODO: Add lockout system
	return true;
}

bool dungeon_isorphaned(DUNGEON *dungeon)
{
	if( list_size(dungeon->player_owners) > 0 ) return false;

	// Any other owners?

	return true;
}

void dungeon_check_failure(DUNGEON *dungeon)
{
	if (!IS_VALID(dungeon)) return;

	// Already "done"
	if (IS_SET(dungeon->flags, (DUNGEON_COMPLETED|DUNGEON_FAILED))) return;

	if (list_size(dungeon->players) < 1)
	{
		if (IS_SET(dungeon->flags, DUNGEON_FAILURE_ON_EMPTY))
			dungeon_failed(dungeon);

		return;
	}

	if (IS_SET(dungeon->flags, DUNGEON_FAILURE_ON_WIPE))
	{
		bool all = true;
		ITERATOR it;
		CHAR_DATA *ch;

		iterator_start(&it, dungeon->players);
		while((ch = (CHAR_DATA *)iterator_nextdata(&it)))
		{
			if (!IS_DEAD(ch))
			{
				all = false;
				break;
			}
		}
		iterator_stop(&it);

		if (all)
			dungeon_failed(dungeon);
	}
}

// Called after a successful ready check, or you are in there without players.
void readycheck_henchmen(DUNGEON *dungeon, CHAR_DATA *leader)
{
	ITERATOR it;
	CHAR_DATA *vch;

	iterator_start(&it, dungeon->mobiles);
	while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
	{
		bool check = false;		
		if(vch == leader) continue;
		if (vch->master != NULL && vch->master->pet == vch && is_same_group(vch->master, leader)) check = true;	// Pets
		if (vch->rider != NULL && is_same_group(vch->rider, leader)) check = true;								// Mounts
		if(is_same_group(vch, leader)) check = true;															// General Henchmen

		if (check)
			p_percent_trigger(vch, NULL, NULL, NULL, leader, NULL, NULL, NULL, NULL, TRIG_READYCHECK, NULL,0,0,0,0,0);
	}
	iterator_stop(&it);
}

bool is_readycheck_complete(CHAR_DATA *ch)
{
	bool ready = true;
	CHAR_DATA *leader = get_player_leader(ch);

	ITERATOR it;
	CHAR_DATA *vch;
	iterator_start(&it, leader->lgroup);
	while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
	{
		if (vch == leader) continue;
		if (IS_NPC(vch)) continue;

		if (vch->pcdata->readycheck_answer != true)
		{
			ready = false;
		}
	}
	iterator_stop(&it);

	return ready;
}

void do_readycheck(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *leader;
	//char buf[MIL];
	char arg[MIL];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  readycheck info    - show current state of ready check\n\r", ch);
		send_to_char("         readycheck start   - initiate a ready check on group\n\r", ch);
		send_to_char("         readycheck yes     - confirm you are ready\n\r", ch);
		send_to_char("         readycheck no      - confirm you are not ready\n\r", ch);
		return;
	}

	DUNGEON *dungeon = get_room_dungeon(ch->in_room);
	leader = get_player_leader(ch);

	if (!IS_VALID(dungeon))
	{
		send_to_char("You are not in a dungeon.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "info"))
	{
		char buf[MSL];
		bool active = false;

		bool complete = is_readycheck_complete(leader);

		if(complete)
		{
			send_to_char("Ready Check Results:\n\r", ch);
		}
		else if(leader->pcdata->last_ready_check > current_time)
		{
			int seconds = leader->pcdata->last_ready_check - current_time;
			char *unit = "second";
			if (seconds >= 120)
			{
				seconds /= 60;
				unit = "minute";
			}

			sprintf(buf, "Current Ready Check: %d %s%s remaining\n\r", seconds, unit, (seconds == 1)?"":"s");
			send_to_char(buf, ch);

			active = true;
		}
		else if(leader->pcdata->last_ready_check > 0)
		{
			send_to_char("Ready Check Results:\n\r", ch);
		}
		else
		{
			send_to_char("No readycheck has been started.\n\r", ch);
			return;
		}

		send_to_char("    [      PLAYER      ] [READY]\n\r", ch);
		send_to_char("=================================\n\r", ch);

		int plr_no = 1;

		sprintf(buf, "{W%2d{x) {Y%-20s  %s{x\n\r", plr_no++, leader->name,
			((leader->pcdata->readycheck_answer == true) ? "{G YES " :
			((leader->pcdata->readycheck_answer == false) ? "{R NO  " :
			(active ? "{x ??? " : "{D-{xA{WF{xK{D-"))));
		send_to_char(buf, ch);

		int yes = 1;
		int no = 0;
		int afk = 0;

		ITERATOR it;
		CHAR_DATA *vch;
		iterator_start(&it, leader->lgroup);
		while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
		{
			if(vch == leader) continue;
			if(IS_NPC(vch)) continue;

			sprintf(buf, "{W%2d{x) %-20s  %s{x\n\r", plr_no++, vch->name,
				((vch->pcdata->readycheck_answer == true) ? "{G YES " :
				((vch->pcdata->readycheck_answer == false) ? "{R NO  " :
				(active ? "{x ??? " : "{D-{xA{WF{xK{D-"))));
			send_to_char(buf, ch);

			if (vch->pcdata->readycheck_answer == true)
				yes++;
			else if (vch->pcdata->readycheck_answer == false)
				no++;
			else
				afk++;
		}
		iterator_stop(&it);
		send_to_char("---------------------------------\n\r", ch);

		sprintf(buf, "Yes: %s%d\n\rNo:  %s%d\n\r%s %d\n\r",
			(active?"    ":""),yes, 
			(active?"    ":""),no, 
			(active?"Unknown:":"AFK:"), afk);
		send_to_char(buf, ch);
		return;
	}

	if (!str_prefix(arg, "start"))
	{
		CHAR_DATA *leader = get_player_leader(ch);

		if (ch != leader)
		{
			send_to_char("Only the group leader may initiate a readycheck.\n\r", ch);
			return;
		}

		if (ch->pcdata->last_ready_check > 0)
		{
			send_to_char("You have already started a readycheck.\n\r", ch);
			send_to_char("Use {Yreadycheck info{x for current status.\n\r", ch);
			return;
		}

		bool all = true;
		bool found = false;
		ITERATOR it;
		CHAR_DATA *vch;

		iterator_start(&it, ch->lgroup);
		while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
		{
			if (vch == ch) continue;	// Ignore you
			if (IS_NPC(ch)) continue;

			found = true;
			DUNGEON *vch_dungeon = get_room_dungeon(vch->in_room);
			if (!IS_VALID(vch_dungeon) || vch_dungeon != dungeon)
			{
				act("$N is not in the dungeon.", ch, vch, NULL, NULL, NULL, NULL, NULL, TO_CHAR, NULL, NULL);
				all = false;
			}
		}
		iterator_stop(&it);

		if (!found)
		{
			send_to_char("You readycheck your party.\n\r", ch);
			readycheck_henchmen(dungeon, ch);
			return;
		}

		if (all)
		{
			iterator_start(&it, ch->lgroup);
			while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
			{
				if (vch == ch) continue;
				if (IS_NPC(ch)) continue;

				DUNGEON *vch_dungeon = get_room_dungeon(vch->in_room);
				if (IS_VALID(vch_dungeon) && vch_dungeon == dungeon)
				{
					vch->pcdata->last_ready_check = current_time + DEFAULT_READY_CHECK;
					vch->pcdata->readycheck_answer = TRISTATE_UNDEF;
					act("$n has initiated a readycheck.", ch, vch, NULL, NULL, NULL, NULL, NULL, TO_VICT, NULL, NULL);
					send_to_char("Please answer with either {Yreadycheck yes{x or {Yreadycheck no{x.\n\r", vch);
				}
			}
			iterator_stop(&it);

			send_to_char("You have initiated a readycheck.\n\r", ch);
			ch->pcdata->last_ready_check = current_time + DEFAULT_READY_CHECK;
			ch->pcdata->readycheck_answer = true;	// Auto confirm yes
		}

		return;
	}

	if (!str_prefix(arg, "yes"))
	{
		if (ch->pcdata->readycheck_answer != TRISTATE_UNDEF)
		{
			send_to_char("You've already given your ready state.\n\r", ch);
			return;
		}
		ch->pcdata->readycheck_answer = true;
		ch->pcdata->last_ready_check = 0;
		send_to_char("You confirmed your READY CHECK - {GYES{x.\n\r", ch);
		return;
	}
	
	if (!str_prefix(arg, "no"))
	{
		if (ch->pcdata->readycheck_answer != TRISTATE_UNDEF)
		{
			send_to_char("You've already given your ready state.\n\r", ch);
			return;
		}
		ch->pcdata->readycheck_answer = false;
		ch->pcdata->last_ready_check = 0;
		send_to_char("You confirmed your READY CHECK - {RNO{x.\n\r", ch);
		return;
	}

	do_readycheck(ch, "");
}


void readycheck_update(CHAR_DATA *ch)
{
	CHAR_DATA *leader = get_player_leader(ch);

	//send_to_char("readycheck_update: called", ch);
	//if (leader) send_to_char("readycheck_update: you are the leader", leader);

	if (leader != ch) return;	// Only run on a leader player
	if (list_size(ch->lgroup) < 1) return;	// No one else in group

	if (leader->pcdata->last_ready_check <= 0) return;

	bool show = true;
	bool complete = true;
	if (leader->pcdata->last_ready_check >= current_time)
	{
		ITERATOR it;
		CHAR_DATA *vch;
		iterator_start(&it, leader->lgroup);
		while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
		{
			if (vch == leader) continue;
			if (IS_NPC(vch)) continue;

			// Someone voted NO
			if (vch->pcdata->readycheck_answer == false)
				complete = false;

			// Someone hasn't voted yet
			if (vch->pcdata->readycheck_answer == TRISTATE_UNDEF)
			{
				complete = false;
				show = false;
				break;
			}
		}
		iterator_stop(&it);
	}

	// Either it has expired or all the votes are in
	if (show)
	{
		do_readycheck(leader, "info");
		ITERATOR it;
		CHAR_DATA *vch;
		iterator_start(&it, leader->lgroup);
		while((vch = (CHAR_DATA *)iterator_nextdata(&it)))
		{
			if (vch == leader) continue;
			if (IS_NPC(vch)) continue;

			vch->pcdata->last_ready_check = 0;			
			do_readycheck(vch, "info");
		}
		iterator_stop(&it);

		leader->pcdata->last_ready_check = 0;
	}

	if (complete)
	{
		DUNGEON *dungeon = get_room_dungeon(leader->in_room);

		readycheck_henchmen(dungeon, leader);
	}
}

