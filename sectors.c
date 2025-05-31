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
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  *
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *    Scripting engine rebuilt by Michael Kurtz (Nibelung)                 *
 *    Used with permission.                                                *
 *                                                                         *
 **************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "scripts.h"
#include "olc.h"

void show_flag_cmds(CHAR_DATA *ch, const struct flag_type *flag_table);

extern const int16_t movement_loss[SECT_MAX];

SECTOR_DATA *get_sector_data(char *name)
{
	ITERATOR it;
	SECTOR_DATA *sector;
	iterator_start(&it, sectors_list);
	while((sector = (SECTOR_DATA *)iterator_nextdata(&it)))
	{
		if (!str_prefix(name, sector->name))
			break;
	}
	iterator_stop(&it);

	return sector;
}

SECTOR_DATA **gsct_from_name(const char *name)
{
	for(int i = 0; global_sector_table[i].name; i++)
	{
		if (!str_prefix(name, global_sector_table[i].name))
			return global_sector_table[i].gsct;
	}

	return NULL;
}

char *gsct_to_name(SECTOR_DATA **gsct)
{
	for(int i = 0; global_sector_table[i].name; i++)
	{
		if (global_sector_table[i].gsct == gsct)
			return global_sector_table[i].name;
	}

	return NULL;
}

char *gsct_to_display(SECTOR_DATA **gsct)
{
	for(int i = 0; global_sector_table[i].name; i++)
	{
		if (global_sector_table[i].gsct == gsct)
			return global_sector_table[i].name;
	}

	return "{D(unset){x";
}

void save_sector(FILE *fp, SECTOR_DATA *sector)
{
	fprintf(fp, "#SECTOR %s~\n", sector->name);
	fprintf(fp, "Class %s~\n", flag_string(sector_classes, sector->sector_class));
	if (sector->gsct)
		fprintf(fp, "GSCT %s~\n", gsct_to_name(sector->gsct));
	fprintf(fp, "Description %s~\n", fix_string(sector->description));
	fprintf(fp, "Comments %s~\n", fix_string(sector->comments));
	fprintf(fp, "Flags %s\n", print_flags(sector->flags));
	fprintf(fp, "MoveCost %d\n", sector->move_cost);
	fprintf(fp, "HPRegen %d\n", sector->hp_regen);
	fprintf(fp, "ManaRegen %d\n", sector->mana_regen);
	fprintf(fp, "MoveRegen %d\n", sector->move_regen);
	fprintf(fp, "Soil %d\n", sector->soil);
	for(int i = 0; i < SECTOR_MAX_AFFINITIES; i++)
	{
		if (sector->affinities[i][0] > CATALYST_NONE && sector->affinities[i][0] < CATALYST_MAX)
		{
			fprintf(fp, "Affinity %s~ %d\n", flag_string(catalyst_types, sector->affinities[i][0]), sector->affinities[i][1]);
		}
	}
	fprintf(fp, "#-SECTOR\n");
}

void save_sectors()
{
	FILE *fp;

	log_string("save_sectors: saving " SECTORS_FILE);
	if ((fp = fopen(SECTORS_FILE, "w")) == NULL)
	{
		bug("save_sectors: fopen", 0);
		perror(SECTORS_FILE);
	}
	else
	{
		ITERATOR it;
		SECTOR_DATA *sector;
		iterator_start(&it, sectors_list);
		while((sector = (SECTOR_DATA *)iterator_nextdata(&it)))
		{
			log_string(formatf("Saving sector '%s'", sector->name));
			save_sector(fp, sector);
		}
		iterator_stop(&it);

		fprintf(fp, "#END\n");
		fclose(fp);
	}
}

void insert_sector(SECTOR_DATA *sector)
{
	ITERATOR it;
	SECTOR_DATA *s;
	iterator_start(&it, sectors_list);
	while((s = (SECTOR_DATA *)iterator_nextdata(&it)))
	{
		int cmp = str_cmp(sector->name, s->name);
		if (cmp < 0)
		{
			iterator_insert_before(&it, sector);
			break;
		}
	}
	iterator_stop(&it);

	if (!s)
		list_appendlink(sectors_list, sector);
}

SECTOR_DATA *load_sector(FILE *fp)
{
	SECTOR_DATA *sector;
	char *word;
	bool fMatch;

	sector = new_sector_data();
	sector->name = fread_string(fp);

	while(str_cmp((word = fread_word(fp)), "#-SECTOR"))
	{
		fMatch = false;

		switch(word[0])
		{
			case 'A':
				if (!str_cmp(word, "Affinity"))
				{
					int catalyst = stat_lookup(fread_string(fp), catalyst_types, CATALYST_NONE);
					int value = fread_number(fp);

					if (catalyst > CATALYST_NONE && catalyst < CATALYST_MAX)
					{
						int i;
						for(i = 0; i < SECTOR_MAX_AFFINITIES; i++)
						{
							if (sector->affinities[i][0] == CATALYST_NONE)
								break;
						}

						if (i < SECTOR_MAX_AFFINITIES)
						{
							sector->affinities[i][0] = catalyst;
							sector->affinities[i][1] = value;
						}
					}

					fMatch = true;
					break;
				}
				break;

			case 'C':
				KEY("Class", sector->sector_class, stat_lookup(fread_string(fp), sector_classes, SECTCLASS_NONE));
				KEYS("Comments", sector->comments, fread_string(fp));
				break;

			case 'D':
				KEYS("Description", sector->description, fread_string(fp));
				break;

			case 'F':
				KEY("Flags", sector->flags, fread_flag(fp));
				break;

			case 'G':
				KEY("GSCT", sector->gsct, gsct_from_name(fread_string(fp)));
				break;

			case 'H':
				KEY("HPRegen", sector->hp_regen, fread_number(fp));
				break;

			case 'M':
				KEY("ManaRegen", sector->mana_regen, fread_number(fp));
				KEY("MoveCost", sector->move_cost, fread_number(fp));
				KEY("MoveRegen", sector->move_regen, fread_number(fp));
				break;

			case 'S':
				KEY("Soil", sector->soil, fread_number(fp));
				break;
		}

		if (!fMatch)
		{
			bug(formatf("load_sector: encountered unknown word '%s'\n\r", word), 0);
			fread_to_eol(fp);
		}
	}

	return sector;
}

static void delete_sector(void *ptr)
{
	free_sector_data((SECTOR_DATA *)ptr);
}

bool load_sectors()
{
	FILE *fp;
	SECTOR_DATA *sector;

	sectors_list = list_createx(false,NULL,delete_sector);
	if (!IS_VALID(sectors_list))
	{
		log_string("load_sectors: failed to create sectors_list.");
		return false;
	}

	log_string("load_sectors: loading " SECTORS_FILE);
	if ((fp = fopen(SECTORS_FILE, "r")) == NULL)
	{
		log_string("load_sectors: bootstrapping " SECTORS_FILE);

		for(int i = 0; sector_types[i].name; i++)
		{
			int type = sector_types[i].bit;

			if (type >= 0 && type < SECT_MAX)
			{
				log_string(formatf("Bootstrapping sector '%s'", sector_types[i].name));

				sector = new_sector_data();
				sector->name = str_dup(sector_types[i].name);

				int16_t clazz = SECTCLASS_NONE;
				long flags = 0;
				int16_t soil = 0;
				switch(type)
				{
					case SECT_INSIDE:
						clazz = SECTCLASS_CITY;
						list_appendlink(sector->hide_msgs, str_dup("in the corner"));
						list_appendlink(sector->hide_msgs, str_dup("amidst the shadows"));
						list_appendlink(sector->hide_msgs, str_dup("beneath some forgotten trash"));
						list_appendlink(sector->hide_msgs, str_dup("in a poorly lit area"));
						list_appendlink(sector->hide_msgs, str_dup("from view"));
						sector->gsct = &gsct_inside;
						SET_BIT(flags, SECTOR_CITY_LIGHTS);
						SET_BIT(flags, SECTOR_INDOORS);
						soil = -20;
						break;

					case SECT_CITY:
						clazz = SECTCLASS_CITY;
						list_appendlink(sector->hide_msgs, str_dup("in the corner"));
						list_appendlink(sector->hide_msgs, str_dup("amidst the shadows"));
						list_appendlink(sector->hide_msgs, str_dup("beneath some forgotten trash"));
						list_appendlink(sector->hide_msgs, str_dup("in a poorly lit area"));
						list_appendlink(sector->hide_msgs, str_dup("from view"));
						sector->gsct = &gsct_city;
						SET_BIT(flags, SECTOR_CITY_LIGHTS);
						soil = -10;
						break;

					case SECT_FIELD:
						clazz = SECTCLASS_PLAINS;
						SET_BIT(flags, SECTOR_NATURE);
						list_appendlink(sector->hide_msgs, str_dup("among the grasses"));
						list_appendlink(sector->hide_msgs, str_dup("in a bed of flowers"));
						list_appendlink(sector->hide_msgs, str_dup("under a pile of stones"));
						list_appendlink(sector->hide_msgs, str_dup("in a small hole"));
						list_appendlink(sector->hide_msgs, str_dup("from sight"));
						soil = 5;
						break;

					case SECT_FOREST:
						clazz = SECTCLASS_FOREST;
						SET_BIT(flags, SECTOR_NATURE);
						list_appendlink(sector->hide_msgs, str_dup("inside a tree"));
						list_appendlink(sector->hide_msgs, str_dup("under a stump"));
						list_appendlink(sector->hide_msgs, str_dup("in the thick vegetation"));
						list_appendlink(sector->hide_msgs, str_dup("in the branches of a tree"));
						list_appendlink(sector->hide_msgs, str_dup("from sight"));
						break;

					case SECT_HILLS:
						clazz = SECTCLASS_HILLS;
						SET_BIT(flags, SECTOR_NATURE);
						list_appendlink(sector->hide_msgs, str_dup("under a large rock"));
						list_appendlink(sector->hide_msgs, str_dup("from sight"));
						break;

					case SECT_MOUNTAIN:
						clazz = SECTCLASS_MOUNTAINS;
						SET_BIT(flags, SECTOR_NATURE);
						list_appendlink(sector->hide_msgs, str_dup("in the deep mountain crags"));
						soil = -10;
						break;

					case SECT_WATER_SWIM:
						clazz = SECTCLASS_WATER;
						list_appendlink(sector->hide_msgs, str_dup("in the sands beneath your feet"));
						sector->gsct = &gsct_water_swim;
						SET_BIT(flags, SECTOR_NO_FADE);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_WATER_NOSWIM:
						clazz = SECTCLASS_WATER;
						sector->gsct = &gsct_water_noswim;
						SET_BIT(flags, SECTOR_DEEP_WATER);
						SET_BIT(flags, SECTOR_NO_FADE);
						SET_BIT(flags, SECTOR_NO_GATE);
						SET_BIT(flags, SECTOR_NO_GOHALL);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_TUNDRA:
						clazz = SECTCLASS_SUBARCTIC;
						list_appendlink(sector->hide_msgs, str_dup("beneath a large pile of snow"));
						SET_BIT(flags, SECTOR_FROZEN);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_AIR:
						clazz = SECTCLASS_AIR;
						SET_BIT(flags, SECTOR_AERIAL);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_DESERT:
						clazz = SECTCLASS_DESERT;
						list_appendlink(sector->hide_msgs, str_dup("under a pile of desert sand"));
						soil = 10;
						break;

					case SECT_NETHERWORLD:
						clazz = SECTCLASS_NETHER;
						list_appendlink(sector->hide_msgs, str_dup("beneath a pile of bones"));
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_DOCK:
						clazz = SECTCLASS_CITY;
						list_appendlink(sector->hide_msgs, str_dup("under a couple of planks"));
						soil = -10;
						break;

					case SECT_ENCHANTED_FOREST:
						clazz = SECTCLASS_FOREST;
						SET_BIT(flags, SECTOR_CRUMBLES);
						SET_BIT(flags, SECTOR_NATURE);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_SLEEP_DRAIN);
						break;

					case SECT_TOXIC_BOG:
						clazz = SECTCLASS_SWAMP;
						SET_BIT(flags, SECTOR_NATURE);
						SET_BIT(flags, SECTOR_NO_SOIL);
						SET_BIT(flags, SECTOR_TOXIC);
						break;

					case SECT_CURSED_SANCTUM:
						clazz = SECTCLASS_DUNGEON;
						SET_BIT(flags, SECTOR_DRAIN_MANA);
						SET_BIT(flags, SECTOR_HARD_MAGIC);
						SET_BIT(flags, SECTOR_NO_GOHALL);
						SET_BIT(flags, SECTOR_SLOW_MAGIC);
						soil = -20;
						break;

					case SECT_BRAMBLE:
						clazz = SECTCLASS_FOREST;
						SET_BIT(flags, SECTOR_BRIARS);
						SET_BIT(flags, SECTOR_NATURE);
						break;

					case SECT_SWAMP:
						clazz = SECTCLASS_SWAMP;
						SET_BIT(flags, SECTOR_NATURE);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_ACID:
						clazz = SECTCLASS_HAZARDOUS;
						SET_BIT(flags, SECTOR_MELTS);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_LAVA:
						clazz = SECTCLASS_VULCAN;
						SET_BIT(flags, SECTOR_FLAME);
						SET_BIT(flags, SECTOR_MELTS);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_SNOW:
						clazz = SECTCLASS_SUBARCTIC;
						list_appendlink(sector->hide_msgs, str_dup("beneath a large pile of snow"));
						SET_BIT(flags, SECTOR_FROZEN);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_ICE:
						clazz = SECTCLASS_SUBARCTIC;
						list_appendlink(sector->hide_msgs, str_dup("beneath a large pile of snow"));
						SET_BIT(flags, SECTOR_FROZEN);
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_CAVE:
						clazz = SECTCLASS_UNDERGROUND;
						list_appendlink(sector->hide_msgs, str_dup("beneath a large pile of rocks"));
						list_appendlink(sector->hide_msgs, str_dup("behind a stalagmite"));
						soil = -5;
						break;

					case SECT_UNDERWATER:
						clazz = SECTCLASS_WATER;
						list_appendlink(sector->hide_msgs, str_dup("behind some submerged vegetation"));
						sector->gsct = &gsct_underwater_swim;
						SET_BIT(flags, SECTOR_NO_FADE);
						SET_BIT(flags, SECTOR_NO_SOIL);
						SET_BIT(flags, SECTOR_UNDERWATER);
						break;

					case SECT_DEEP_UNDERWATER:
						clazz = SECTCLASS_WATER;
						sector->gsct = &gsct_underwater_noswim;
						SET_BIT(flags, SECTOR_DEEP_WATER);
						SET_BIT(flags, SECTOR_NO_FADE);
						SET_BIT(flags, SECTOR_NO_GATE);
						SET_BIT(flags, SECTOR_NO_HIDE_OBJ);
						SET_BIT(flags, SECTOR_NO_SOIL);
						SET_BIT(flags, SECTOR_UNDERWATER);
						break;

					case SECT_JUNGLE:
						clazz = SECTCLASS_JUNGLE;
						SET_BIT(flags, SECTOR_NATURE);
						list_appendlink(sector->hide_msgs, str_dup("inside a tree"));
						list_appendlink(sector->hide_msgs, str_dup("under a stump"));
						list_appendlink(sector->hide_msgs, str_dup("in the thick vegetation"));
						list_appendlink(sector->hide_msgs, str_dup("in the branches of a tree"));
						list_appendlink(sector->hide_msgs, str_dup("from sight"));
						break;

					case SECT_PAVED_ROAD:
						clazz = SECTCLASS_ROADS;
						list_appendlink(sector->hide_msgs, str_dup("behind a rock on the side of the road"));
						SET_BIT(flags, SECTOR_NO_SOIL);
						break;

					case SECT_DIRT_ROAD:
						clazz = SECTCLASS_ROADS;
						list_appendlink(sector->hide_msgs, str_dup("behind a rock on the side of the road"));
						soil = 20;
						break;
				}

				sector->sector_class = clazz;
				sector->flags = flags;
				sector->soil = soil;
				sector->move_cost = movement_loss[type];

				insert_sector(sector);
			}
		}

		save_sectors();
	}
	else
	{
		char *word;
		bool fMatch;

		while(str_cmp((word = fread_word(fp)), "#END"))
		{
			fMatch = true;

			switch(word[0])
			{
				case '#':
					if (!str_cmp(word, "#SECTOR"))
					{
						sector = load_sector(fp);

						insert_sector(sector);
						fMatch = true;
						break;
					}
					break;
			}

			if (!fMatch)
			{
				bug(formatf("load_sectors: encountered unknown word '%s'.\n\r", word), 0);
				fread_to_eol(fp);
			}
		}
	}

	// Global Sector pointers
	for(int i = 0; global_sector_table[i].name; i++)
	{
		if (global_sector_table[i].gsct)
			*global_sector_table[i].gsct = NULL;
	}

	ITERATOR sit;
	iterator_start(&sit, sectors_list);
	while((sector = (SECTOR_DATA *)iterator_nextdata(&sit)))
	{
		if (sector->gsct)
			*(sector->gsct) = sector;
	}
	iterator_stop(&sit);

	// Only mandatory one because the server will assume it *IS* assigned after this point
	if (!gsct_inside)
	{
		log_string("load_sectors: Missing sector for gsct_inside.");
		log_string("load_sectors: Please assign \"GSCT Inside~\" to a sector and restart.");
		list_destroy(sectors_list);
		return false;
	}

	return true;
}



struct sectorlist_params
{
	char name[MIL];

	bool has_name;
};

static bool __sectorlist_parse_params(CHAR_DATA *ch, char *argument, struct sectorlist_params *params)
{
	params->has_name = false;

	while(argument[0])
	{
		char arg[MIL];
		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "name"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Missing name in filter.\n\r", ch);
				return false;
			}

			argument = one_argument(argument, params->name);
			params->has_name = true;
		}
		else
		{
			send_to_char("Invalid parameters.\n\r", ch);
			send_to_char("  name <name>        - Filter by name\n\r", ch);
			return false;
		}
		
	}

	return true;
}

void do_sectorlist(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer = new_buf();
	char buf[MSL];

	struct sectorlist_params params;
	if (!__sectorlist_parse_params(ch, argument, &params))
		return;

	sprintf(buf, "%-4s %-20s\n\r",
		"#", "Name");
	add_buf(buffer, buf);
	sprintf(buf, "%-4s %-20s\n\r",
		"====", "====================");
	add_buf(buffer, buf);

	int i = 0;
	ITERATOR it;
	SECTOR_DATA *sector;
    iterator_start(&it, sectors_list);
    while((sector = (SECTOR_DATA *)iterator_nextdata(&it)))
    {
		if (params.has_name && str_prefix(params.name, sector->name)) continue;

		sprintf(buf, "%-4d %-20s\n\r", ++i,
			sector->name);
		add_buf(buffer, buf);
    }
    iterator_stop(&it);

	sprintf(buf, "%-4s %-20s\n\r",
		"====", "====================");
	add_buf(buffer, buf);

	sprintf(buf, "Total: %d\n\r", i);
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
}



