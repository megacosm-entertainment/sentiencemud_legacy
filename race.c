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

#define VERSION_RACES_000		0x00000000

#define VERSION_RACES_001		0x00000001
// Change #1: Added versioning
// Change #2: Added 'skull' body part to all playable races

#define VERSION_RACES			VERSION_RACES_001

int version_races;

void show_flag_cmds(CHAR_DATA *ch, const struct flag_type *flag_table);

RACE_DATA *get_race_data(const char *name)
{
	ITERATOR it;
	RACE_DATA *race;
	
	iterator_start(&it, race_list);
	while((race = (RACE_DATA *)iterator_nextdata(&it)))
	{
		if (!str_prefix(name, race->name))
			break;
	}
	iterator_stop(&it);

	return race;
}

RACE_DATA *get_race_uid(const int16_t uid)
{
	ITERATOR it;
	RACE_DATA *race;
	
	iterator_start(&it, race_list);
	while((race = (RACE_DATA *)iterator_nextdata(&it)))
	{
		if (race->uid == uid)
			break;
	}
	iterator_stop(&it);

	return race;
}

void insert_race(RACE_DATA *race)
{
	ITERATOR it;
	RACE_DATA *r;
	iterator_start(&it, race_list);
	while((r = (RACE_DATA *)iterator_nextdata(&it)))
	{
		int cmp = str_cmp(race->name, r->name);
		if (cmp < 0)
		{
			iterator_insert_before(&it, race);
			break;
		}
	}
	iterator_stop(&it);

	if (!r)
	{
		list_appendlink(race_list, race);
	}
}

RACE_DATA **gr_from_name(const char *name)
{
	for(int i = 0; gr_table[i].name; i++)
	{
		if (!str_prefix(name, gr_table[i].name))
			return gr_table[i].gr;
	}
	
	return NULL;
}

char *gr_to_name(RACE_DATA **gr)
{
	for(int i = 0; gr_table[i].name; i++)
	{
		if (gr_table[i].gr == gr)
			return gr_table[i].name;
	}
	
	return NULL;
}


void save_race(FILE *fp, RACE_DATA *race)
{
	if (race->playable)
		fprintf(fp, "#PCRACE %d\n", race->uid);
	else
		fprintf(fp, "#RACE %d\n", race->uid);

	fprintf(fp, "Name %s~\n", fix_string(race->name));
	fprintf(fp, "Description %s~\n", fix_string(race->description));
	fprintf(fp, "Comments %s~\n", fix_string(race->comments));

	if (race->pgr)
		fprintf(fp, "GR %s~\n", gr_to_name(race->pgr));

	fprintf(fp, "Flags %s\n", print_flags(race->flags));

	fprintf(fp, "Act %s %s\n", print_flags(race->act[0]), print_flags(race->act[1]));
	fprintf(fp, "Aff %s %s\n", print_flags(race->aff[0]), print_flags(race->aff[1]));
	fprintf(fp, "Off %s\n", print_flags(race->off));
	fprintf(fp, "Imm %s\n", print_flags(race->imm));
	fprintf(fp, "Res %s\n", print_flags(race->res));
	fprintf(fp, "Vuln %s\n", print_flags(race->vuln));
	fprintf(fp, "Form %s\n", print_flags(race->form));
	fprintf(fp, "Parts %s\n", print_flags(race->parts));

	if (race->playable)
	{
		if (IS_VALID(race->premort))
			fprintf(fp, "GRemort %s~\n", race->premort->name);

		if (race->starting)
			fprintf(fp, "Starting\n");

		if (race->remort)
			fprintf(fp, "Remort\n");

		fprintf(fp, "Who %s~\n", fix_string(race->who));

		ITERATOR sit;
		SKILL_DATA *skill;
		iterator_start(&sit, race->skills);
		while((skill = (SKILL_DATA *)iterator_nextdata(&sit)))
		{
			fprintf(fp, "Skill %s~\n", skill->name);
		}
		iterator_stop(&sit);

		for(int i = 0; i < MAX_STATS; i++)
		{
			fprintf(fp, "Stat %s~ %d\n", flag_string(stat_types, i), race->stats[i]);
			fprintf(fp, "MaxStat %s~ %d\n", flag_string(stat_types, i), race->max_stats[i]);
		}

		for(int i = 0; i < 3; i++)
		{
			fprintf(fp, "MaxVital %s~ %d\n", flag_string(vital_types, i), race->max_vitals[i]);
		}

		fprintf(fp, "Size %d %d\n", race->min_size, race->max_size);
		fprintf(fp, "DefaultAlignment %d\n", race->default_alignment);
	}

	fprintf(fp, "#-RACE\n");
}

void save_races()
{
	FILE *fp;

	log_string("save_races: saving " RACES_FILE);
	if ((fp = fopen(RACES_FILE, "w")) == NULL)
	{
		bug("save_races: fopen", 0);
		perror(RACES_FILE);
		return;
	}

	fprintf(fp, "Version %d\n", VERSION_RACES);

	ITERATOR it;
	RACE_DATA *race;
	iterator_start(&it, race_list);
	while((race = (RACE_DATA *)iterator_nextdata(&it)))
	{
		save_race(fp, race);
	}	
	iterator_stop(&it);

	fprintf(fp, "End\n");
	fclose(fp);

}

RACE_DATA *load_race(FILE *fp, bool playable)
{
	RACE_DATA *data = new_race_data();

	char buf[MSL];
	char *word;
	bool fMatch;

	data->uid = fread_number(fp);
	data->playable = playable;
	data->load_remort = NULL;

    while (str_cmp((word = fread_word(fp)), "#-RACE"))
	{
		fMatch = false;
		switch(word[0])
		{
			case 'A':
				if (!str_cmp(word, "Act"))
				{
					data->act[0] = fread_flag(fp);
					data->act[1] = fread_flag(fp);
					fMatch = true;
					break;
				}
				if (!str_cmp(word, "Aff"))
				{
					data->aff[0] = fread_flag(fp);
					data->aff[1] = fread_flag(fp);
					fMatch = true;
					break;
				}
				break;

			case 'C':
				KEYS("Comments", data->comments, fread_string(fp));
				break;

			case 'D':
				KEYS("Description", data->description, fread_string(fp));
				break;

			case 'F':
				KEY("Flags", data->flags, fread_flag(fp));
				KEY("Form", data->form, fread_flag(fp));
				break;

			case 'G':
				KEY("GR", data->pgr, gr_from_name(fread_string(fp)));
				break;

			case 'I':
				KEY("Imm", data->imm, fread_flag(fp));
				break;

			case 'N':
				KEYS("Name", data->name, fread_string(fp));
				break;

			case 'O':
				KEY("Off", data->off, fread_flag(fp));
				break;

			case 'P':
				KEY("Parts", data->parts, fread_flag(fp));
				break;

			case 'R':
				KEY("Res", data->res, fread_flag(fp));
				break;

			case 'V':
				KEY("Vuln", data->vuln, fread_flag(fp));
				break;
		}

		if (!fMatch && data->playable)
		{
			switch(word[0])
			{
				case 'D':
					KEY("DefaultAlignment", data->default_alignment, fread_number(fp));
					break;

				case 'G':
					KEY("GRemort", data->load_remort, fread_string(fp));
					break;

				case 'M':
					if (!str_cmp(word, "MaxStat"))
					{
						char *name = fread_string(fp);
						int value = fread_number(fp);

						int stat = stat_lookup(name, stat_types, NO_FLAG);
						if (stat != NO_FLAG)
						{
							data->max_stats[stat] = value;
						}

						fMatch = true;
						break;
					}
					if (!str_cmp(word, "MaxVital"))
					{
						char *name = fread_string(fp);
						int value = fread_number(fp);

						int vital = stat_lookup(name, vital_types, NO_FLAG);
						if (vital != NO_FLAG)
						{
							data->max_vitals[vital] = value;
						}

						fMatch = true;
						break;
					}
					break;

				case 'R':
					KEY("Remort", data->remort, true);
					break;

				case 'S':
					if (!str_cmp(word, "Size"))
					{
						int min_size = fread_number(fp);
						int max_size = fread_number(fp);

						data->min_size = min_size;
						data->max_size = max_size;

						fMatch = true;
						break;
					}

					if (!str_cmp(word, "Skill"))
					{
						SKILL_DATA *skill = get_skill_data(fread_string(fp));

						if (IS_VALID(skill))
						{
							list_appendlink(data->skills, skill);
						}
						fMatch = true;
						break;
					}

					KEY("Starting", data->starting, true);

					if (!str_cmp(word, "Stat"))
					{
						char *name = fread_string(fp);
						int value = fread_number(fp);

						int stat = stat_lookup(name, stat_types, NO_FLAG);
						if (stat != NO_FLAG)
						{
							data->stats[stat] = value;
						}

						fMatch = true;
						break;
					}
					break;

				case 'W':
					KEYS("Who", data->who, fread_string(fp));
					break;
			}
		}

		if (!fMatch)
		{
			snprintf(buf, sizeof(buf), "load_race: no match for word %s", word);
			bug(buf, 0);
		}
	}

	return data;
}

static void delete_race_data(void *ptr)
{
	free_race_data((RACE_DATA *)ptr);
}

bool load_races()
{
	FILE *fp;
	char buf[MSL];
	char *word;
	bool fMatch;
	RACE_DATA *race;
	top_race_uid = 0;

	version_races = VERSION_RACES_000;

	log_string("load_races: creating race_list");
	race_list = list_createx(false, NULL, delete_race_data);
	if (!IS_VALID(race_list))
	{
		log_string("race_list was not created.");
		return false;
	}

	log_string("load_races: loading " RACES_FILE);
	if ((fp = fopen(RACES_FILE, "r")) == NULL)
	{
		log_string("load_races: '" RACES_FILE "' file not found.  Bootstrapping races.");

		for (int i = 0; __race_table[i].name; i++)
		{
			if (__race_table[i].pgrn)
				*__race_table[i].pgrn = i;
		}
		for (int i = 0; __pc_race_table[i].name; i++)
		{
			if (__pc_race_table[i].pgrn)
				*__pc_race_table[i].pgrn = i;
		}

		// Skip the first entry ("none")
		for(int i = 1; __race_table[i].name; i++)
		{
			RACE_DATA *race = new_race_data();
			race->uid = ++top_race_uid;

			race->name = str_dup(__race_table[i].name);
			race->playable = __race_table[i].pc_race;
			race->pgr = gr_from_name(race->name);

			race->act[0] = __race_table[i].act;
			race->act[1] = __race_table[i].act2;
			race->aff[0] = __race_table[i].aff;
			race->aff[1] = __race_table[i].aff2;
			race->off = __race_table[i].off;
			race->imm = __race_table[i].imm;
			race->res = __race_table[i].res;
			race->vuln = __race_table[i].vuln;
			race->form = __race_table[i].form;
			race->parts = __race_table[i].parts;
			if (race->playable && IS_SET(race->parts, PART_HEAD))
				SET_BIT(race->parts, PART_SKULL);

			if (race->playable && __race_table[i].pgprn)
			{
				int16_t j = *(__race_table[i].pgprn);

				race->who = str_dup(__pc_race_table[j].who_name);

				for(int k = 0; k < 9; k++) if (!IS_NULLSTR(__pc_race_table[j].skills[k]))
				{
					SKILL_DATA *skill = get_skill_data(__pc_race_table[j].skills[k]);
					if (IS_VALID(skill))
						list_appendlink(race->skills, skill);
				}

				for(int k = 0; k < MAX_STATS; k++)
				{
					race->stats[k] = __pc_race_table[j].stats[k];
					race->max_stats[k] = __pc_race_table[j].max_stats[k];
				}

				for(int k = 0; k < 3; k++)
				{
					race->max_vitals[k] = __pc_race_table[j].max_vital_stats[k];
				}

				race->min_size = __pc_race_table[j].size;
				race->max_size = __pc_race_table[j].size;
				race->default_alignment = __pc_race_table[j].alignment;

				race->remort = __pc_race_table[j].remort;
			}

			insert_race(race);
		}

		// Link REMORT pointers
		for(int i = 1; __pc_race_table[i].name; i++)
		{
			RACE_DATA *race = get_race_data(__pc_race_table[i].name);

			if (__pc_race_table[i].prgrn && *(__pc_race_table[i].prgrn) != -1)
			{
				int j = *(__pc_race_table[i].prgrn);

				race->premort = get_race_data(__pc_race_table[j].name);
			}
		}
	}
	else
	{
		while(str_cmp((word = fread_word(fp)), "End"))
		{
			fMatch = false;

			switch(word[0])
			{
			case '#':
				if (!str_cmp(word, "#RACE"))
				{
					race = load_race(fp, false);
					if (race)
					{
						insert_race(race);

						if (race->uid > top_race_uid)
							top_race_uid = race->uid;
					}
					else
						log_string("Failed to load a race.");

					fMatch = true;
					break;
				}
				if (!str_cmp(word, "#PCRACE"))
				{
					race = load_race(fp, true);
					if (race)
					{
						insert_race(race);

						if (race->uid > top_race_uid)
							top_race_uid = race->uid;
					}
					else
						log_string("Failed to load a race.");

					fMatch = true;
					break;
				}
				break;

			case 'V':
				KEY("Version", version_races, fread_number(fp));
				break;
			}

			if (!fMatch) {
				snprintf(buf, sizeof(buf), "load_races: no match for word %s", word);
				bug(buf, 0);
			}
		}

		bool save = false;

		for(int i = 0; gr_table[i].name; i++)
			if (gr_table[i].gr)
				*(gr_table[i].gr) = NULL;

		ITERATOR it;
		iterator_start(&it, race_list);
		while((race = (RACE_DATA *)iterator_nextdata(&it)))
		{
			if (race->pgr)
				*(race->pgr) = race;

			if (!race->uid)
			{
				race->uid = ++top_race_uid;
				save = true;
			}

			if (race->load_remort)
			{
				race->premort = get_race_data(race->load_remort);
				free_string(race->load_remort);
				race->load_remort = NULL;
			}

			if (version_races < VERSION_RACES_001 && race->playable && IS_SET(race->parts,PART_HEAD))
			{
				SET_BIT(race->parts, PART_SKULL);
				save = true;
			}
		}
		iterator_stop(&it);

		if (save)
			save_races();
	}

	return true;
}

struct racelist_params
{
	char name[MIL];
	bool playable;
	bool remort;

	bool has_name;
	bool has_playable;
	bool has_remort;
};

static bool __racelist_parse_params(CHAR_DATA *ch, char *argument, struct racelist_params *params)
{
	params->has_name = false;
	params->has_playable = false;
	params->has_remort = false;

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
		else if (!str_prefix(arg, "pc"))
		{
			params->playable = true;
			params->has_playable = true;
		}
		else if (!str_prefix(arg, "npc"))
		{
			if (params->has_remort)
			{
				if (params->remort)
					send_to_char("Combining {Wnpc{x with {Wremort{x filter.\n\r", ch);
				else
					send_to_char("Combining {Wnpc{x with {Wmort{x filter.\n\r", ch);
				return false;
			}

			params->playable = false;
			params->has_playable = true;
		}
		else if (!str_prefix(arg, "remort"))
		{
			if (params->has_playable && !params->playable)
			{
				send_to_char("Combining {Wremort{x with {Wnpc{x filter.\n\r", ch);
				return false;
			}

			params->playable = true;
			params->has_playable = true;
			params->remort = true;
			params->has_remort = true;
		}
		else if (!str_prefix(arg, "mort"))
		{
			if (params->has_playable && !params->playable)
			{
				send_to_char("Combining {Wmort{x with {Wnpc{x filter.\n\r", ch);
				return false;
			}

			params->playable = true;
			params->has_playable = true;
			params->remort = false;
			params->has_remort = true;
		}
		else
		{
			send_to_char("Invalid parameters.\n\r", ch);
			send_to_char("  name <name>        - Filter by name\n\r", ch);
			send_to_char("  npc                - Only show non-playable races\n\r", ch);
			send_to_char("  pc                 - Only show playable races\n\r", ch);
			send_to_char("  mort               - Only show mort races (playable only)\n\r", ch);
			send_to_char("  remort             - Only show remort races (playable only)\n\r", ch);
			return false;
		}
		
	}

	return true;
}

void do_racelist(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer = new_buf();
	char buf[MSL];

	struct racelist_params params;
	if (!__racelist_parse_params(ch, argument, &params))
		return;

	sprintf(buf, "%-4s %-20s %-9s %-7s\n\r",
		"#", "Name", "Playable?", "Remort?");
	add_buf(buffer, buf);
	sprintf(buf, "%-4s %-20s %-9s %-8s\n\r",
		"====", "====================", "=========", "=======");
	add_buf(buffer, buf);

	int i = 0;
	ITERATOR it;
	RACE_DATA *race;
    iterator_start(&it, race_list);
    while((race = (RACE_DATA *)iterator_nextdata(&it)))
    {
		if (params.has_name && str_prefix(params.name, race->name)) continue;
		if (params.has_playable && race->playable != params.playable) continue;
		if (params.has_remort && (!race->playable || race->remort != params.remort)) continue;

		char race_color;
		if (race->playable)
		{
			if (race->remort)
				race_color = 'Y';
			else
				race_color = 'G';
		}
		else
			race_color = 'x';

		sprintf(buf, "%-4d {%c%-20s %s %s\n\r", ++i,
			race_color, race->name,
			(race->playable ? "{W   YES   {x" : "{D    NO   {x"),
			(race->playable && race->remort ? "{W  YES  {x" : "{D   NO  {x"));
		add_buf(buffer, buf);
    }
    iterator_stop(&it);

	sprintf(buf, "%-4s %-20s %-9s %-8s\n\r",
		"====", "====================", "=========", "========");
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

