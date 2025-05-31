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

void blueprint_update_section_ordinals(BLUEPRINT *bp);
void blueprint_update_link_ordinals(BLUEPRINT *bp);
void room_update(ROOM_INDEX_DATA *room);
void save_script_new(FILE *fp, AREA_DATA *area,SCRIPT_DATA *scr,char *type);
SCRIPT_DATA *read_script_new( FILE *fp, AREA_DATA *area, int type);

LLIST *loaded_instances;

void fix_blueprint_section(BLUEPRINT_SECTION *bs)
{
	if (bs->type == BSTYPE_STATIC)
	{
		for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
		{
			if( bl->vnum < bs->lower_vnum || bl->vnum > bs->upper_vnum )
				continue;

			if( bl->vnum > 0 && bl->door >= 0 && bl->door < MAX_DIR )
			{
				bl->room = get_room_index(bs->area, bl->vnum);

				if( bl->room )
					bl->ex = bl->room->exit[bl->door];
			}
		}
	}
	else if (bs->type == BSTYPE_MAZE)
	{
		if (bs->maze_x > 0 && bs->maze_y > 0)
		{
			for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
			{
				int x = (bl->vnum - 1) % bs->maze_x + 1;
				int y = (bl->vnum - 1) / bs->maze_x + 1;

				if (x < 1 || x > bs->maze_x || y < 1 || y > bs->maze_y)
					continue;
				
				ITERATOR it;
				long vnum = 0;
				MAZE_FIXED_ROOM *mfr;
				iterator_start(&it, bs->maze_fixed_rooms);
				while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
				{
					if (mfr->x == x && mfr->y == y)
					{
						vnum = mfr->vnum;
						break;
					}
				}
				iterator_stop(&it);

				if (vnum > 0)
				{
					bl->room = get_room_index(bs->area, vnum);

					if( bl->room )
						bl->ex = bl->room->exit[bl->door];
				}
			}
		}
	}
}

BLUEPRINT_LINK *load_blueprint_link(FILE *fp, AREA_DATA *pArea)
{
	BLUEPRINT_LINK *link;
	char *word;
	bool fMatch;

	link = new_blueprint_link();

	while (str_cmp((word = fread_word(fp)), "#-LINK"))
	{
		fMatch = false;
		switch(word[0])
		{
		case 'D':
			KEY("Door", link->door, fread_number(fp));
			break;

		case 'N':
			KEYS("Name", link->name, fread_string(fp));
			break;

		case 'R':
			KEY("Room", link->vnum, fread_number(fp));
			break;
		}

		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "load_blueprint_link: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	return link;
}

BLUEPRINT_SECTION *load_blueprint_section(FILE *fp, AREA_DATA *pArea)
{
	BLUEPRINT_SECTION *bs;
	char *word;
	bool fMatch;

	bs = new_blueprint_section();
	bs->vnum = fread_number(fp);
	bs->area = pArea;

	pArea->bottom_blueprint_section_vnum = UMIN(pArea->bottom_blueprint_section_vnum, bs->vnum);
	pArea->top_blueprint_section_vnum = UMAX(pArea->top_blueprint_section_vnum, bs->vnum);

	while (str_cmp((word = fread_word(fp)), "#-SECTION"))
	{
		fMatch = false;

		//log_stringf("SECTION: %s", word);

		switch(word[0])
		{
		case '#':
			if( !str_cmp(word, "#LINK") )
			{
				BLUEPRINT_LINK *link = load_blueprint_link(fp, pArea);

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

				fMatch = true;
				break;
			}
			break;

		case 'C':
			KEYS("Comments", bs->comments, fread_string(fp));
			break;

		case 'D':
			KEYS("Description", bs->description, fread_string(fp));
			break;

		case 'F':
			KEY("Flags", bs->flags, fread_number(fp));
			break;

		case 'L':
			KEY("Lower", bs->lower_vnum, fread_number(fp));
			break;

		case 'M':
			if (!str_cmp(word, "MazeFixedRoom"))
			{
				int x = fread_number(fp);
				int y = fread_number(fp);
				long vnum = fread_number(fp);
				bool connected = fread_number(fp) && true;

				MAZE_FIXED_ROOM *mfr = new_maze_fixed_room();
				mfr->x = x;
				mfr->y = y;
				mfr->vnum = vnum;
				mfr->connected = connected;

				list_appendlink(bs->maze_fixed_rooms, mfr);

				fMatch = true;
				break;
			}
			KEY("MazeH", bs->maze_y, fread_number(fp));
			if (!str_cmp(word, "MazeTemplate"))
			{
				int weight = fread_number(fp);
				long vnum = fread_number(fp);

				MAZE_WEIGHTED_ROOM *mwr = new_maze_weighted_room();
				mwr->weight = weight;
				mwr->vnum = vnum;

				list_appendlink(bs->maze_templates, mwr);
				bs->total_maze_weight += weight;

				fMatch = true;
				break;
			}
			KEY("MazeW", bs->maze_x, fread_number(fp));
			break;

		case 'N':
			KEYS("Name", bs->name, fread_string(fp));
			break;

		case 'R':
			KEY("Recall", bs->recall, fread_number(fp));
			break;

		case 'T':
			KEY("Type", bs->type, fread_number(fp));
			break;

		case 'U':
			KEY("Upper", bs->upper_vnum, fread_number(fp));
			break;
		}


		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "load_blueprint_section: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	fix_blueprint_section(bs);
	return bs;
}


BLUEPRINT_LAYOUT_SECTION_DATA *load_blueprint_layout_section(FILE *fp, int mode)
{
	BLUEPRINT_LAYOUT_SECTION_DATA *layout;
	char *word;
	bool fMatch;

	layout = new_blueprint_layout_section_data();
	layout->mode = mode;

	if (mode == SECTIONMODE_STATIC)
		layout->section = fread_number(fp);
	else if (mode == SECTIONMODE_WEIGHTED)
		layout->total_weight = 0;

	while (str_cmp((word = fread_word(fp)), "#-SECTION"))
	{
		fMatch = false;

		//log_stringf("LAYOUT: %s", word);

		switch(word[0])
		{
		case '#':
			if (mode == SECTIONMODE_GROUP)
			{
				if (!str_cmp(word, "#STATICSECTION"))
				{
					BLUEPRINT_LAYOUT_SECTION_DATA *lo = load_blueprint_layout_section(fp, SECTIONMODE_STATIC);

					list_appendlink(layout->group, lo);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#WEIGHTEDSECTION"))
				{
					BLUEPRINT_LAYOUT_SECTION_DATA *lo = load_blueprint_layout_section(fp, SECTIONMODE_WEIGHTED);

					list_appendlink(layout->group, lo);
					fMatch = true;
					break;
				}
			}
			break;

		case 'S':
			if (!str_cmp(word, "Section"))
			{
				if (mode == SECTIONMODE_WEIGHTED)
				{
					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted = new_weighted_random_section();
					weighted->weight = fread_number(fp);
					weighted->section = fread_number(fp);
					list_appendlink(layout->weighted_sections, weighted);

					layout->total_weight += weighted->weight;
				}
				else
				{
					// Complain about getting weighted floor data on a static reference?
					fread_to_eol(fp);
				}

				fMatch = true;
				break;
			}
			break;
		}

		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "load_blueprint_layout_section: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	return layout;
}

BLUEPRINT_LAYOUT_LINK_DATA *load_blueprint_layout_link(FILE *fp, int mode)
{
	BLUEPRINT_LAYOUT_LINK_DATA *link;
	char *word;
	bool fMatch;

	int max_from = 0;
	int max_to = 0;

	link = new_blueprint_layout_link_data();
	link->mode = mode;

	if (mode == LINKMODE_STATIC || mode == LINKMODE_DESTINATION)
		max_from = 1;
	
	if (mode == LINKMODE_STATIC || mode == LINKMODE_SOURCE)
		max_to = 1;

	while (str_cmp((word = fread_word(fp)), "#-LINK"))
	{
		fMatch = false;

		//log_stringf("LINK: %s", word);

		switch(word[0])
		{
		case '#':
			if (mode == LINKMODE_GROUP)
			{
				if (!str_cmp(word, "#STATICLINK"))
				{
					BLUEPRINT_LAYOUT_LINK_DATA *data = load_blueprint_layout_link(fp, LINKMODE_STATIC);

					list_appendlink(link->group, data);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#SOURCELINK"))
				{
					BLUEPRINT_LAYOUT_LINK_DATA *data = load_blueprint_layout_link(fp, LINKMODE_SOURCE);

					list_appendlink(link->group, data);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#DESTLINK"))
				{
					BLUEPRINT_LAYOUT_LINK_DATA *data = load_blueprint_layout_link(fp, LINKMODE_DESTINATION);

					list_appendlink(link->group, data);
					fMatch = true;
					break;
				}

				if (!str_cmp(word, "#WEIGHTEDLINK"))
				{
					BLUEPRINT_LAYOUT_LINK_DATA *data = load_blueprint_layout_link(fp, LINKMODE_WEIGHTED);

					list_appendlink(link->group, data);
					fMatch = true;
					break;
				}
				break;
			}
			break;
		
		case 'F':
			if (!str_cmp(word, "From"))
			{
				if (mode == LINKMODE_GROUP)
				{
					bug("load_blueprint_layout_link: specifying From entry for group link.", 0);
					continue;
				}

				if (max_from > 0 && list_size(link->from) >= max_from)
				{
					bug("load_blueprint_layout_link: too many From entries found for link mode.", 0);
					continue;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = new_weighted_random_link();
				weighted->weight = fread_number(fp);
				weighted->section = fread_number(fp);
				weighted->link = fread_number(fp);
				list_appendlink(link->from, weighted);
				link->total_from += weighted->weight;
				
				fMatch = true;
				break;
			}
			break;

		case 'T':
			if (!str_cmp(word, "To"))
			{
				if (mode == LINKMODE_GROUP)
				{
					bug("load_blueprint_layout_link: specifying To entry for group link.", 0);
					continue;
				}

				if (max_to > 0 && list_size(link->to) >= max_to)
				{
					bug("load_blueprint_layout_link: too many To entries found for link mode.", 0);
					continue;
				}

				BLUEPRINT_WEIGHTED_LINK_DATA *weighted = new_weighted_random_link();
				weighted->weight = fread_number(fp);
				weighted->section = fread_number(fp);
				weighted->link = fread_number(fp);
				list_appendlink(link->to, weighted);
				link->total_to += weighted->weight;
				
				fMatch = true;
				break;
			}
			break;

		default: break;
		}


		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "load_blueprint_layout_link: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	return link;
}

BLUEPRINT *load_blueprint(FILE *fp, AREA_DATA *pArea)
{
	BLUEPRINT *bp;
	char *word;
	bool fMatch;
	char buf[MSL];

	bp = new_blueprint();
	bp->vnum = fread_number(fp);
	bp->area = pArea;

	pArea->bottom_blueprint_vnum = UMAX(pArea->bottom_blueprint_vnum, bp->vnum);
	pArea->top_blueprint_vnum = UMAX(pArea->top_blueprint_vnum, bp->vnum);

	while (str_cmp((word = fread_word(fp)), "#-BLUEPRINT"))
	{
		fMatch = false;

		//log_stringf("BLUEPRINT: %s", word);

		switch(word[0])
		{
		case '#':
			if (!str_cmp(word, "#STATICSECTION"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *layout = load_blueprint_layout_section(fp, SECTIONMODE_STATIC);

				list_appendlink(bp->layout, layout);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#WEIGHTEDSECTION"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *layout = load_blueprint_layout_section(fp, SECTIONMODE_WEIGHTED);

				list_appendlink(bp->layout, layout);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#GROUPSECTION"))
			{
				BLUEPRINT_LAYOUT_SECTION_DATA *layout = load_blueprint_layout_section(fp, SECTIONMODE_GROUP);

				list_appendlink(bp->layout, layout);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#STATICLINK"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = load_blueprint_layout_link(fp, LINKMODE_STATIC);

				list_appendlink(bp->links, link);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#SOURCELINK"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = load_blueprint_layout_link(fp, LINKMODE_SOURCE);

				list_appendlink(bp->links, link);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#DESTLINK"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = load_blueprint_layout_link(fp, LINKMODE_DESTINATION);

				list_appendlink(bp->links, link);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#WEIGHTEDLINK"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = load_blueprint_layout_link(fp, LINKMODE_WEIGHTED);

				list_appendlink(bp->links, link);
				fMatch = true;
				break;
			}

			if (!str_cmp(word, "#GROUPLINK"))
			{
				BLUEPRINT_LAYOUT_LINK_DATA *link = load_blueprint_layout_link(fp, LINKMODE_GROUP);

				list_appendlink(bp->links, link);
				fMatch = true;
				break;
			}
			break;

		case 'A':
			KEY("AreaWho", bp->area_who, fread_number(fp));
			break;

		case 'C':
			KEYS("Comments", bp->comments, fread_string(fp));
			break;

		case 'D':
			KEYS("Description", bp->description, fread_string(fp));
			break;

		case 'E':
			if( !str_cmp(word, "Entry") )
			{
				char *name = fread_string(fp);
				int section = fread_number(fp);
				int link = fread_number(fp);

				BLUEPRINT_EXIT_DATA *ex = new_blueprint_exit_data();
				ex->name = name;
				ex->section = section;
				ex->link = link;

				list_appendlink(bp->entrances, ex);
				fMatch = true;
				break;
			}

			if( !str_cmp(word, "Exit") )
			{
				char *name = fread_string(fp);
				int section = fread_number(fp);
				int link = fread_number(fp);

				BLUEPRINT_EXIT_DATA *ex = new_blueprint_exit_data();
				ex->name = name;
				ex->section = section;
				ex->link = link;

				list_appendlink(bp->exits, ex);
				fMatch = true;
				break;
			}
			break;

		case 'F':
			KEY("Flags", bp->flags, fread_number(fp));
			break;

		case 'I':
			if (!str_cmp(word, "InstanceProg")) {
				char *p;

				WNUM_LOAD wnum_load = fread_widevnum(fp, pArea->uid);
				p = fread_string(fp);

				struct trigger_type *tt = get_trigger_type(p, PRG_IPROG);
				if(!tt) {
					snprintf(buf, sizeof(buf), "load_blueprint: invalid trigger type %s", p);
					bug(buf, 0);
				} else {
					PROG_LIST *ipr = new_trigger();

					ipr->wnum_load = wnum_load;
					ipr->trig_type = tt->type;
					ipr->trig_phrase = fread_string(fp);
					if( tt->type == TRIG_SPELLCAST ) {
						char buf[MIL];
						SKILL_DATA *skill = get_skill_data(ipr->trig_phrase);

						if( !IS_VALID(skill) ) {
							snprintf(buf, sizeof(buf), "load_blueprint: invalid spell '%s' for TRIG_SPELLCAST", ipr->trig_phrase);
							bug(buf, 0);
							free_trigger(ipr);
							fMatch = true;
							break;
						}

						free_string(ipr->trig_phrase);
						sprintf(buf, "%d", skill->uid);
						ipr->trig_phrase = str_dup(buf);
						ipr->trig_number = skill->uid;
						ipr->numeric = true;

					} else {
						ipr->trig_number = atoi(ipr->trig_phrase);
						ipr->numeric = is_number(ipr->trig_phrase);
					}

					if(!bp->progs) bp->progs = new_prog_bank();

					list_appendlink(bp->progs[tt->slot], ipr);

					trigger_type_add_use(tt);
				}
				fMatch = true;
			}
			break;

		case 'N':
			KEYS("Name", bp->name, fread_string(fp));
			break;

		case 'R':
			KEY("Recall", bp->recall, fread_number(fp));
			KEY("Repop", bp->repop, fread_number(fp));
			break;

		case 'S':
			// TODO: How to do this
			if( !str_cmp(word, "SpecialRoom") )
			{
				char *name = fread_string(fp);
				int section = fread_number(fp);
				long offset = fread_number(fp);

				BLUEPRINT_SPECIAL_ROOM *special = new_blueprint_special_room();

				special->name = name;
				special->section = section;
				special->offset = offset;

				list_appendlink(bp->special_rooms, special);
				fMatch = true;
				break;
			}
			if( !str_cmp(word, "Section") )
			{
				long section = fread_number(fp);

				BLUEPRINT_SECTION *bs = get_blueprint_section(pArea, section);
				if( bs )
				{
					list_appendlink(bp->sections, bs);
				}
				fMatch = true;
				break;
			}
			break;

		case 'V':
			// Variables
			if (olc_load_index_vars(fp, word, &bp->index_vars, pArea))
			{
				fMatch = true;
				break;
			}
			break;
		}


		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "load_blueprint: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	blueprint_update_section_ordinals(bp);
	blueprint_update_link_ordinals(bp);

	return bp;
}

int blueprint_update_section_group_ordinals(BLUEPRINT_LAYOUT_SECTION_DATA *data, int ordinal)
{
	BLUEPRINT_LAYOUT_SECTION_DATA *section;
	ITERATOR sit;
	iterator_start(&sit, data->group);
	while( (section = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&sit)) )
	{
		section->ordinal = ordinal++;
	}
	iterator_stop(&sit);
	return ordinal;
}

void blueprint_update_section_ordinals(BLUEPRINT *bp)
{
	int ordinal = 1;
	BLUEPRINT_LAYOUT_SECTION_DATA *section;
	ITERATOR sit;
	iterator_start(&sit, bp->layout);
	while( (section = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&sit)) )
	{
		if (section->mode == SECTIONMODE_GROUP)
			ordinal = blueprint_update_section_group_ordinals(section, ordinal);
		else
			section->ordinal = ordinal++;
	}
	iterator_stop(&sit);
}

int blueprint_update_link_group_ordinals(BLUEPRINT_LAYOUT_LINK_DATA *data, int ordinal)
{
	BLUEPRINT_LAYOUT_LINK_DATA *link;
	ITERATOR lit;
	iterator_start(&lit, data->group);
	while( (link = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&lit)) )
	{
		link->ordinal = ordinal++;
	}
	iterator_stop(&lit);
	return ordinal;
}

void blueprint_update_link_ordinals(BLUEPRINT *bp)
{
	int ordinal = 1;
	BLUEPRINT_LAYOUT_LINK_DATA *link;
	ITERATOR lit;
	iterator_start(&lit, bp->layout);
	while( (link = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&lit)) )
	{
		if (link->mode == LINKMODE_GROUP)
			ordinal = blueprint_update_link_group_ordinals(link, ordinal);
		else
			link->ordinal = ordinal++;
	}
	iterator_stop(&lit);
}

void save_blueprint_section(FILE *fp, BLUEPRINT_SECTION *bs)
{
	fprintf(fp, "#SECTION %ld\n", bs->vnum);
	fprintf(fp, "Name %s~\n", fix_string(bs->name));
	fprintf(fp, "Description %s~\n", fix_string(bs->description));
	fprintf(fp, "Comments %s~\n", fix_string(bs->comments));

	fprintf(fp, "Type %d\n", bs->type);
	fprintf(fp, "Flags %d\n", bs->flags);

	if (bs->type == BSTYPE_STATIC)
	{
		fprintf(fp, "Recall %ld\n", bs->recall);
		fprintf(fp, "Lower %ld\n", bs->lower_vnum);
		fprintf(fp, "Upper %ld\n", bs->upper_vnum);
	}
	else if (bs->type == BSTYPE_MAZE)
	{
		fprintf(fp, "MazeW %ld\n", bs->maze_x);
		fprintf(fp, "MazeH %ld\n", bs->maze_y);
		fprintf(fp, "Recall %ld\n", bs->recall);

		ITERATOR rit;
		MAZE_WEIGHTED_ROOM *mwr;
		iterator_start(&rit, bs->maze_templates);
		while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&rit)))
		{
			fprintf(fp, "MazeRoomTemplate %d %ld\n", mwr->weight, mwr->vnum);
		}
		iterator_stop(&rit);

		MAZE_FIXED_ROOM *mfr;
		iterator_start(&rit, bs->maze_fixed_rooms);
		while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&rit)))
		{
			fprintf(fp, "MazeFixedRoom %d %d %ld %d\n", mfr->x, mfr->y, mfr->vnum, mfr->connected?1:0);
		}
		iterator_stop(&rit);
	}

	for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
	{
		if( valid_section_link(bl) )
		{
			fprintf(fp, "#LINK\n");
			fprintf(fp, "Name %s~\n", fix_string(bl->name));
			fprintf(fp, "Room %ld\n", bl->vnum);
			fprintf(fp, "Door %d\n", bl->door);
			fprintf(fp, "#-LINK\n");
		}
	}

	fprintf(fp, "#-SECTION\n\n");
}

void save_blueprint_layout_section(FILE *fp, BLUEPRINT_LAYOUT_SECTION_DATA *section)
{
	if (section->mode == SECTIONMODE_STATIC)
	{
		fprintf(fp, "#STATICSECTION %d\n", section->section);
		fprintf(fp, "#-SECTION\n\r");
	}
	else if (section->mode == SECTIONMODE_WEIGHTED)
	{
		fprintf(fp, "#WEIGHTEDSECTION\n");
		ITERATOR wit;
		BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
		iterator_start(&wit, section->weighted_sections);
		while( (weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)) )
		{
			fprintf(fp, "Section %d %d\n", weighted->weight, weighted->section);
		}
		iterator_stop(&wit);
		fprintf(fp, "#-SECTION\n\r");
	}
	else if (section->mode == SECTIONMODE_GROUP)
	{
		fprintf(fp, "#GROUPSECTION\n");
		ITERATOR git;
		BLUEPRINT_LAYOUT_SECTION_DATA *data;
		iterator_start(&git, section->group);
		while( (data = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&git)) )
		{
			save_blueprint_layout_section(fp, data);
		}
		iterator_stop(&git);
		fprintf(fp, "#-SECTION\n\r");
	}
}

void save_blueprint_weighted_link(FILE *fp, LLIST *list, char *field)
{
	BLUEPRINT_WEIGHTED_LINK_DATA *weighted;

	ITERATOR it;
	iterator_start(&it, list);
	while( (weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)) )
	{
		fprintf(fp, "%s %d %d %d\n", field, weighted->weight, weighted->section, weighted->link);
	}
	iterator_stop(&it);
}

void save_blueprint_layout_link(FILE *fp, BLUEPRINT_LAYOUT_LINK_DATA *link)
{
	if (link->mode == LINKMODE_STATIC)
	{
		fprintf(fp, "#STATICLINK\n");
		save_blueprint_weighted_link(fp, link->from, "From");
		save_blueprint_weighted_link(fp, link->to, "To");
		fprintf(fp, "#-LINK\n");
	}
	else if (link->mode == LINKMODE_SOURCE)
	{
		fprintf(fp, "#SOURCELINK\n");
		save_blueprint_weighted_link(fp, link->from, "From");
		save_blueprint_weighted_link(fp, link->to, "To");
		fprintf(fp, "#-LINK\n");
	}
	else if (link->mode == LINKMODE_DESTINATION)
	{
		fprintf(fp, "#DESTLINK\n");
		save_blueprint_weighted_link(fp, link->from, "From");
		save_blueprint_weighted_link(fp, link->to, "To");
		fprintf(fp, "#-LINK\n");
	}
	else if (link->mode == LINKMODE_WEIGHTED)
	{
		fprintf(fp, "#WEIGHTEDLINK\n");
		save_blueprint_weighted_link(fp, link->from, "From");
		save_blueprint_weighted_link(fp, link->to, "To");
		fprintf(fp, "#-LINK\n");
	}
	else if (link->mode == LINKMODE_GROUP)
	{
		fprintf(fp, "#GROUPLINK\n");
		ITERATOR git;
		BLUEPRINT_LAYOUT_LINK_DATA *data;
		iterator_start(&git, link->group);
		while( (data = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&git)) )
		{
			save_blueprint_layout_link(fp, data);
		}
		iterator_stop(&git);
		fprintf(fp, "#-LINK\n");
	}
}

void save_blueprint(FILE *fp, BLUEPRINT *bp)
{
	fprintf(fp, "#BLUEPRINT %ld\n", bp->vnum);
	fprintf(fp, "Name %s~\n", fix_string(bp->name));
	fprintf(fp, "Description %s~\n", fix_string(bp->description));
	fprintf(fp, "Comments %s~\n", fix_string(bp->comments));
	fprintf(fp, "AreaWho %d\n", bp->area_who);
	fprintf(fp, "Repop %d\n", bp->repop);
	fprintf(fp, "Flags %d\n", bp->flags);

	// Master list of sections
	ITERATOR sit;
	BLUEPRINT_SECTION *bs;
	iterator_start(&sit, bp->sections);
	while( (bs = (BLUEPRINT_SECTION *)iterator_nextdata(&sit)) )
	{
		fprintf(fp, "Section %ld\n", bs->vnum);
	}
	iterator_stop(&sit);

	// Actual layout of the sections
	ITERATOR lsit;
	BLUEPRINT_LAYOUT_SECTION_DATA *ls;
	iterator_start(&lsit, bp->layout);
	while( (ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&lsit)) )
	{
		save_blueprint_layout_section(fp, ls);
	}
	iterator_stop(&lsit);

	// List of links between the layout sections
	ITERATOR llit;
	BLUEPRINT_LAYOUT_LINK_DATA *ll;
	iterator_start(&llit, bp->links);
	while( (ll = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&llit)) )
	{
		save_blueprint_layout_link(fp, ll);
	}
	iterator_stop(&llit);

	fprintf(fp, "Recall %d\n", bp->recall);

	ITERATOR xit;
	BLUEPRINT_EXIT_DATA *ex;
	iterator_start(&xit, bp->entrances);
	while( (ex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&xit)) )
	{
		fprintf(fp, "Entry %s~ %d %d\n", fix_string(ex->name), ex->section, ex->link);
	}
	iterator_stop(&xit);

	iterator_start(&xit, bp->exits);
	while( (ex = (BLUEPRINT_EXIT_DATA *)iterator_nextdata(&xit)) )
	{
		fprintf(fp, "Exit %s~ %d %d\n", fix_string(ex->name), ex->section, ex->link);
	}
	iterator_stop(&xit);

   if(bp->progs) {
		ITERATOR it;
		PROG_LIST *trigger;
		for(int i = 0; i < TRIGSLOT_MAX; i++) if(list_size(bp->progs[i]) > 0) {
			iterator_start(&it, bp->progs[i]);
			while((trigger = (PROG_LIST *)iterator_nextdata(&it)))
				fprintf(fp, "InstanceProg %s %s~ %s~\n", widevnum_string_wnum(trigger->wnum, bp->area), trigger_name(trigger->trig_type), trigger_phrase(trigger->trig_type,trigger->trig_phrase));
			iterator_stop(&it);
		}
	}

	olc_save_index_vars(fp, bp->index_vars, bp->area);

	fprintf(fp, "#-BLUEPRINT\n\n");
}

// save blueprints
void save_blueprints(FILE *fp, AREA_DATA *area)
{
	int iHash;

	for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(BLUEPRINT_SECTION *bs = area->blueprint_section_hash[iHash]; bs; bs = bs->next)
		{
			save_blueprint_section(fp, bs);
		}
	}

	for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(BLUEPRINT *bp = area->blueprint_hash[iHash]; bp; bp = bp->next)
		{
			save_blueprint(fp, bp);
		}
	}
}






ROOM_INDEX_DATA *blueprint_section_get_room_byoffset(BLUEPRINT_SECTION *bs, int offset)
{
	long vnum;
	if (bs->type == BSTYPE_STATIC)
	{
		vnum = bs->lower_vnum + offset;
		if (vnum < bs->lower_vnum || vnum > bs->upper_vnum) return NULL;

		return get_room_index(bs->area, vnum);
	}

	if (bs->type == BSTYPE_MAZE)
	{
		if (bs->maze_x < 1 || bs->maze_y < 1) return NULL;
		// Convert zero-based offset to coordinate
		int x = offset % bs->maze_x + 1;
		int y = offset / bs->maze_x + 1;

		// Out of bounds
		if (x < 1 || x > bs->maze_x ||
			y < 1 || y > bs->maze_y)
			return NULL;

		ITERATOR it;
		MAZE_FIXED_ROOM *mfr;
		iterator_start(&it, bs->maze_fixed_rooms);
		while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
		{
			if (mfr->x == x && mfr->y == y)
				break;
		}
		iterator_stop(&it);

		return mfr ? get_room_index(bs->area, mfr->vnum) : NULL;
	}

	return NULL;
}

bool valid_section_link(BLUEPRINT_LINK *bl)
{
	if( !IS_VALID(bl) ) return false;

	if( bl->vnum <= 0 ) return false;

	if( bl->door < 0 || bl->door >= MAX_DIR ) return false;

	if( !bl->room ) return false;

	if( !bl->ex ) return false;

	// Only environment exits can be used as links
	if( !IS_SET(bl->ex->exit_info, EX_ENVIRONMENT) ) return false;

	return true;
}

BLUEPRINT_LINK *get_section_link(BLUEPRINT_SECTION *bs, int link)
{
	if( !IS_VALID(bs) ) return NULL;

	if( link < 1 ) return NULL;

	for(BLUEPRINT_LINK *bl = bs->links; bl; bl = bl->next)
	{
		if( !--link )
			return bl;
	}

	return NULL;
}

bool valid_static_link(STATIC_BLUEPRINT_LINK *sbl)
{
	if( !IS_VALID(sbl) ) return false;
	if( !IS_VALID(sbl->blueprint) ) return false;

	BLUEPRINT_SECTION *section1 = (BLUEPRINT_SECTION *)list_nthdata(sbl->blueprint->sections, sbl->section1);
	if( !IS_VALID(section1) ) return false;

	BLUEPRINT_SECTION *section2 = (BLUEPRINT_SECTION *)list_nthdata(sbl->blueprint->sections, sbl->section2);
	if( !IS_VALID(section2) ) return false;

	BLUEPRINT_LINK *link1 = get_section_link(section1, sbl->link1);
	if( !valid_section_link(link1) ) return false;

	BLUEPRINT_LINK *link2 = get_section_link(section2, sbl->link2);
	if( !valid_section_link(link2) ) return false;

	// Only allow links that are reverse directions to link
	if( rev_dir[link1->door] != link2->door ) return false;

	return true;
}

ROOM_INDEX_DATA *blueprint_get_special_room(BLUEPRINT *bp, BLUEPRINT_SPECIAL_ROOM *special)
{
	BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, special->section, NULL);
	if (!IS_VALID(ls)) return NULL;

	BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);
	if (!IS_VALID(bs)) return NULL;

	if (bs->type == BSTYPE_STATIC)
	{
		long vnum = bs->lower_vnum + special->offset;
		if (vnum < bs->lower_vnum || vnum > bs->upper_vnum)
			return NULL;

		return get_room_index(bs->area, vnum);
	}

	if (bs->type == BSTYPE_MAZE)
	{
		// special->offset is the coordinate encoded as an index
		if (bs->maze_x < 1 || bs->maze_y < 1)
			return NULL;

		if (special->offset < 0)
			return NULL;

		int x = special->offset % bs->maze_x + 1;
		int y = special->offset / bs->maze_x + 1;

		// Out of bounds
		if (x < 1 || x > bs->maze_x ||
			y < 1 || y > bs->maze_y)
			return NULL;

		ITERATOR it;
		long vnum = 0;
		MAZE_FIXED_ROOM *mfr;
		iterator_start(&it, bs->maze_fixed_rooms);
		while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
		{
			if (mfr->x == x && mfr->y == y)
			{
				vnum = mfr->vnum;
				break;
			}
		}
		iterator_stop(&it);

		if (vnum > 0)
			return get_room_index(bs->area, vnum);
		else
			return NULL;
	}

	// Can only resolve special rooms at this point for static sections
	return NULL;
}

bool is_blueprint_static(BLUEPRINT *bp)
{
	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
		return false;

	bool valid = true;
	ITERATOR it;
	BLUEPRINT_LAYOUT_SECTION_DATA *ls;
	iterator_start(&it, bp->layout);
	while((ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)))
	{
		if (ls->mode != SECTIONMODE_STATIC)
		{
			// Only need to check if we have non-static section definitions
			valid = false;
			break;
		}

		BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);
		if (bs->type != BSTYPE_STATIC)
		{
			// Only sections with a static layout can be used
			valid = false;
			break;
		}
	}
	iterator_stop(&it);

	if (valid)
	{
		BLUEPRINT_LAYOUT_LINK_DATA *ll;
		// Make sure all of the links are static links
		iterator_start(&it, bp->links);
		while((ll = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&it)))
		{
			if (ll->mode != LINKMODE_STATIC)
			{
				valid = false;
				break;
			}
		}
		iterator_stop(&it);
	}

	return valid;
}


BLUEPRINT_SECTION *get_blueprint_section_wnum(WNUM wnum)
{
	return get_blueprint_section(wnum.pArea, wnum.vnum);
}

BLUEPRINT_SECTION *get_blueprint_section_auid(long auid, long vnum)
{
	return get_blueprint_section(get_area_from_uid(auid), vnum);
}

BLUEPRINT_SECTION *get_blueprint_section(AREA_DATA *pArea, long vnum)
{
	int iHash = vnum % MAX_KEY_HASH;

	if (!pArea) return NULL;

	for(BLUEPRINT_SECTION *bs = pArea->blueprint_section_hash[iHash]; bs; bs = bs->next)
	{
		if( bs->vnum == vnum )
			return bs;
	}

	return NULL;
}

BLUEPRINT_SECTION *get_blueprint_section_byroom(AREA_DATA *pArea, long vnum)
{
	for(int iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(BLUEPRINT_SECTION *bs = pArea->blueprint_section_hash[iHash]; bs; bs = bs->next)
		{
			if (bs->type == BSTYPE_STATIC)
			{
				if( vnum >= bs->lower_vnum && vnum <= bs->upper_vnum )
					return bs;
			}
			else if(bs->type == BSTYPE_MAZE)
			{
				ITERATOR it;

				// Check the fixed rooms
				MAZE_FIXED_ROOM *mfr;
				iterator_start(&it, bs->maze_fixed_rooms);
				while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
				{
					if (mfr->vnum == vnum)
						break;
				}
				iterator_stop(&it);

				if (mfr)
					return bs;

				// Check the templates to see if it matches one of those
				MAZE_WEIGHTED_ROOM *mwr;
				iterator_start(&it, bs->maze_templates);
				while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&it)))
				{
					if (mwr->vnum == vnum)
						break;
				}
				iterator_stop(&it);

				if (mwr)
					return bs;
			}
		}
	}

	return NULL;
}

BLUEPRINT *get_blueprint_wnum(WNUM wnum)
{
	return get_blueprint(wnum.pArea, wnum.vnum);
}

BLUEPRINT *get_blueprint_auid(long auid, long vnum)
{
	return get_blueprint(get_area_from_uid(auid), vnum);
}

BLUEPRINT *get_blueprint(AREA_DATA *pArea, long vnum)
{
	int iHash = vnum % MAX_KEY_HASH;

	if (!pArea) return NULL;

	for(BLUEPRINT *bp = pArea->blueprint_hash[iHash]; bp; bp = bp->next)
	{
		if( bp->vnum == vnum )
			return bp;
	}

	return NULL;
}

bool rooms_in_same_section(AREA_DATA *pArea1, long vnum1, AREA_DATA *pArea2, long vnum2)
{
	BLUEPRINT_SECTION *s1 = get_blueprint_section_byroom(pArea1, vnum1);
	BLUEPRINT_SECTION *s2 = get_blueprint_section_byroom(pArea2, vnum2);

	if( !s1 && !s2 ) return true;	// If neither are in a blueprint section, they are considered in the same section

	return s1 && s2 && (s1 == s2);
}

BLUEPRINT_EXIT_DATA *get_blueprint_entrance(BLUEPRINT *bp, int index)
{
	return (BLUEPRINT_EXIT_DATA *)list_nthdata(bp->entrances, index);
}


BLUEPRINT_EXIT_DATA *get_blueprint_exit(BLUEPRINT *bp, int index)
{
	return (BLUEPRINT_EXIT_DATA *)list_nthdata(bp->exits, index);
}


ROOM_INDEX_DATA *instance_section_get_room_byvnum(INSTANCE_SECTION *section, long vnum)
{
	if( !IS_VALID(section) ) return NULL;

	ROOM_INDEX_DATA *room;
	ITERATOR rit;
	iterator_start(&rit, section->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
	{
		if( room->vnum == vnum )
			break;
	}
	iterator_stop(&rit);

	return room;
}

// offset is zero-based
ROOM_INDEX_DATA *instance_section_get_room_byoffset(INSTANCE_SECTION *section, long offset)
{
	if( !IS_VALID(section) ) return NULL;

	if (section->section->type == BSTYPE_STATIC)
	{
		long vnum = section->section->lower_vnum + offset;
		if (vnum < section->section->lower_vnum || vnum > section->section->upper_vnum) return NULL;

		ROOM_INDEX_DATA *room = NULL;
		ITERATOR rit;
		iterator_start(&rit, section->rooms);
		while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
		{
			if( room->vnum == vnum )
				break;
		}
		iterator_stop(&rit);

		return room;
	}
	else if (section->section->type == BSTYPE_MAZE)
	{
		if (offset < 0 || offset >= (section->section->maze_x * section->section->maze_y))
			return NULL;

		// Add one since list indexes are one-based rather than zero-based
		return (ROOM_INDEX_DATA *)list_nthdata(section->rooms, offset + 1);
	}

	return NULL;
}

ROOM_INDEX_DATA *instance_section_get_room(INSTANCE_SECTION *section, ROOM_INDEX_DATA *source)
{
	if( !source ) return NULL;

	return instance_section_get_room_byvnum(section, source->vnum);
}

int instance_section_count_mob(INSTANCE_SECTION *section, MOB_INDEX_DATA *pMobIndex)
{
	if( !IS_VALID(section) ) return 0;

	int count = 0;
	ROOM_INDEX_DATA *room;
	ITERATOR rit;
	iterator_start(&rit, section->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
	{
		for(CHAR_DATA *pMob = room->people; pMob; pMob = pMob->next_in_room)
		{
			if( IS_NPC(pMob) && pMob->pIndexData == pMobIndex && !IS_SET(pMob->act[0], ACT_ANIMATED) )
				++count;
		}
	}
	iterator_stop(&rit);

	return count;
}

int instance_count_mob(INSTANCE *instance, MOB_INDEX_DATA *pMobIndex)
{
	if( !IS_VALID(instance) ) return 0;

	int count = 0;
	ITERATOR it;
	INSTANCE_SECTION *section;
	iterator_start(&it, instance->sections);
	while( (section = (INSTANCE_SECTION *)iterator_nextdata(&it)) )
	{
		count += instance_section_count_mob(section, pMobIndex);
	}
	iterator_stop(&it);

	return count;
}

// create instance
#define MAZE_MAX_DIR	4
typedef struct __maze_room_cell
{
	int x;
	int y;

	ROOM_INDEX_DATA *room;

	int options[MAZE_MAX_DIR];
	int total_options;

	bool visited;
} MAZE_CELL;

static void __purge_maze_cells(MAZE_CELL *cells, int total)
{
	for(int i = 0; i < total; i++)
	{
		ROOM_INDEX_DATA *room = cells[i].room;
		if (room)
			extract_clone_room(room->source,room->id[0],room->id[1],true);
	}

	free_mem(cells, sizeof(MAZE_CELL) * total);
}

inline static void __maze_link_room(ROOM_INDEX_DATA *room, int door, ROOM_INDEX_DATA *dest)
{
	EXIT_DATA *exClone;

	room->exit[door] = exClone = new_exit();
	exClone->exit_info = 0;
	exClone->keyword = str_dup("");
	exClone->short_desc = str_dup("");
	exClone->long_desc = str_dup("");
	exClone->rs_flags = 0;
	exClone->orig_door = door;
	exClone->door.strength = 0;
	exClone->door.material = NULL;
	exClone->from_room = room;
	exClone->u1.to_room = dest;

	door = rev_dir[door];
	dest->exit[door] = exClone = new_exit();
	exClone->exit_info = 0;
	exClone->keyword = str_dup("");
	exClone->short_desc = str_dup("");
	exClone->long_desc = str_dup("");
	exClone->rs_flags = 0;
	exClone->orig_door = door;
	exClone->door.strength = 0;
	exClone->door.material = NULL;
	exClone->from_room = dest;
	exClone->u1.to_room = room;
}

inline static void __maze_remove_option(MAZE_CELL *cell, int door)
{
	if (door < DIR_NORTH || door >= DIR_UP) return;

	for(int i = 0; i < cell->total_options; i++)
	{
		if (cell->options[i] == door)
		{
			--(cell->total_options);
			cell->options[i] = cell->options[cell->total_options];
			return;
		}
	}
}

inline static bool __maze_has_option(MAZE_CELL *cell, int door)
{
	if (door < DIR_NORTH || door >= DIR_UP) return false;

	for(int i = 0; i < cell->total_options; i++)
	{
		if (cell->options[i] == door)
		{
			return true;
		}
	}

	return false;
}

bool blueprint_section_generate_maze(INSTANCE_SECTION *section, BLUEPRINT_SECTION *bs)
{
	static int dir_offsets[MAX_DIR][2] =
	{
		{0, -1},	// NORTH
		{1, 0},		// EAST
		{0, 1},		// SOUTH
		{-1, 0},	// WEST
		{0, 0},		// UP
		{0, 0},		// DOWN
		{1, -1},	// NORTHEAST
		{-1, -1},	// NORTHWEST
		{1, 1},		// SOUTHEAST
		{-1, 1}		// SOUTHWEST
	};

	ITERATOR it;
	if (bs->maze_x < 1 || bs->maze_y < 1) return false;

	int total = bs->maze_x * bs->maze_y;
	MAZE_CELL *cells = (MAZE_CELL *)alloc_mem(sizeof(MAZE_CELL) * total);
	if(!cells) return false;

	for(int i = 0; i < total; i++) {
		cells[i].visited = false;
		cells[i].room = NULL;
		cells[i].options[0] = DIR_NORTH;
		cells[i].options[1] = DIR_EAST;
		cells[i].options[2] = DIR_SOUTH;
		cells[i].options[3] = DIR_WEST;
		cells[i].total_options = MAZE_MAX_DIR;
	}

	int idx = 0;
	for(int _y = 1; _y <= bs->maze_y; _y++)
		for(int _x = 1; _x <= bs->maze_x; _x++, idx++)
		{
			cells[idx].x = _x;
			cells[idx].y = _y;
		}

	for(int i = 0; i < bs->maze_x; i++)
	{
		__maze_remove_option(&cells[i], DIR_NORTH);
		__maze_remove_option(&cells[total - i - 1], DIR_SOUTH);
	}
	for(int i = 0; i < bs->maze_y; i++)
	{
		__maze_remove_option(&cells[i * bs->maze_x], DIR_WEST);
		__maze_remove_option(&cells[(i+1) * bs->maze_x - 1], DIR_EAST);
	}

	// Place fixed rooms first
	MAZE_FIXED_ROOM *mfr;
	iterator_start(&it, bs->maze_fixed_rooms);
	while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
	{
		if (mfr->x >= 1 && mfr->x <= bs->maze_x &&
			mfr->y >= 1 && mfr->y <= bs->maze_y)
		{
			int index = (mfr->y - 1) * bs->maze_x + mfr->x;

			if (cells[index].room)
			{
				__purge_maze_cells(cells, total);
				return false;
			}

			ROOM_INDEX_DATA *source = get_room_index(bs->area, mfr->vnum);

			if( source )
			{
				ROOM_INDEX_DATA *room = create_virtual_room_nouid(source,false,false,true);

				if( !room )
				{
					__purge_maze_cells(cells, total);
					return false;
				}

				get_vroom_id(room);

				cells[index].room = room;
				if (!mfr->connected)
				{
					cells[index].visited = true;
					cells[index].total_options = 0;
				}
			}
		}
	}
	iterator_stop(&it);

	// Generate the rest of the rooms
	for(int i = 0; i < total; i++)
	{
		if (cells[i].room) continue;	// Fixed room already placed

		int w = number_range(1, bs->total_maze_weight);

		MAZE_WEIGHTED_ROOM *mwr;
		long vnum = 0;
		iterator_start(&it, bs->maze_templates);
		while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&it)))
		{
			if (w <= mwr->weight)
			{
				vnum = mwr->vnum;
				break;
			}
			else
				w -= mwr->weight;
		}
		iterator_stop(&it);

		if (vnum < 1)
		{
			__purge_maze_cells(cells, total);
			return false;
		}

		ROOM_INDEX_DATA *source = get_room_index(bs->area, vnum);

		if( source )
		{
			ROOM_INDEX_DATA *room = create_virtual_room_nouid(source,false,false,true);

			if( !room )
			{
				__purge_maze_cells(cells, total);
				return false;
			}

			get_vroom_id(room);

			cells[i].room = room;
		}
	}

	// Place all the exits from the fixed rooms first
	int x = 1;
	int y = 1;
	for(int i = 0; i < total; i++)
	{
		ROOM_INDEX_DATA *room = cells[i].room;
		ROOM_INDEX_DATA *source = room->source;

		for(int j = 0; j < MAX_DIR; j++)
		{
			if (room->exit[j]) continue;	// Already made

			int x1 = x + dir_offsets[j][0];
			int y1 = y + dir_offsets[j][1];

			EXIT_DATA *exParent = source->exit[j];
			if (exParent)
			{
				MAZE_CELL *dest;
				if (!IS_SET(exParent->exit_info, EX_ENVIRONMENT))
				{
					// Out of bounds
					if (x1 < 1 || x1 > bs->maze_x) continue;
					if (y1 < 1 || y1 > bs->maze_y) continue;

					// UP/DOWN exits must be environment
					if (x1 == x && y1 == y) continue;
					int index1 = (y1 - 1) * bs->maze_x + x1;
					dest = &cells[index1];
				}
				else
					dest = NULL;

				EXIT_DATA *exClone;

				room->exit[i] = exClone = new_exit();
				exClone->exit_info = exParent->exit_info;
				exClone->keyword = str_dup(exParent->keyword);
				exClone->short_desc = str_dup(exParent->short_desc);
				exClone->long_desc = str_dup(exParent->long_desc);
				exClone->rs_flags = exParent->rs_flags;
				exClone->orig_door = exParent->orig_door;
				exClone->door.strength = exParent->door.strength;
				exClone->door.material = exParent->door.material;
				exClone->door.lock = exParent->door.rs_lock;
				exClone->door.rs_lock = exParent->door.rs_lock;
				exClone->from_room = room;
				if (dest != NULL)
				{
					exClone->u1.to_room = dest->room;

					// Remove the remote direction's option in the destination room, even if it's a one way exit, so it doesn't make an exit by mistake
					__maze_remove_option(dest, rev_dir[j]);
				}
				else
					exClone->u1.to_room = NULL;

				// Remove this direction as an option
				__maze_remove_option(&cells[i], j);
			}
		}

		x++;
		if (x > bs->maze_x)
		{
			y++;
			x = 1;
		}
	}

	// Add exits to the rest of the rooms
	LLIST *visited = list_create(false);
	for(int i = 0; i < total; i++)
	{
		// This check here accounts for fixed rooms creating pockets or divisions in the entire maze.
		if (cells[i].visited) continue;

		// Start point
		MAZE_CELL *current;
		MAZE_CELL *next;
		list_addlink(visited, &cells[i]);
		do
		{
			current = (MAZE_CELL *)list_nthdata(visited, 1);
			current->visited = true;

			if (current->total_options > 0)
			{
				int opt = number_range(0, current->total_options - 1);
				int dir = current->options[opt];
				// First, remove the option
				__maze_remove_option(current, dir);

				// Get the destination on the map
				int nx = current->x + dir_offsets[dir][0];
				int ny = current->y + dir_offsets[dir][1];
				int nidx = (ny - 1) * bs->maze_x + nx;
				next = &cells[nidx];

				// If we have never visited it (disconnected fixed rooms will be flagged visited)
				//  -and-
				// Destination has the option to connect
				if (!next->visited && __maze_has_option(next, rev_dir[dir]))
				{
					__maze_remove_option(next, rev_dir[dir]);
					__maze_link_room(current->room, dir, next->room);

					list_addlink(visited, next);
				}
			}
			else
				list_remnthlink(visited, 1, false);
		}
		while(list_size(visited) > 0);
	}
	list_destroy(visited);

	for(int i = 0; i < total; i++)
	{
		if (!list_appendlink(section->rooms, cells[i].room))
		{
			__purge_maze_cells(cells, total);
			return false;
		}

		cells[i].room = NULL;
	}

	free_mem(cells, sizeof(MAZE_CELL) * total);
	return true;
}


INSTANCE_SECTION *clone_blueprint_section(BLUEPRINT_SECTION *parent)
{
	ROOM_INDEX_DATA *room;
	ITERATOR rit;

	INSTANCE_SECTION *section = new_instance_section();

	if( !section ) return NULL;

	section->section = parent;

	if (parent->type == BSTYPE_STATIC)
	{
		// Clone rooms
		for(long vnum = parent->lower_vnum; vnum <= parent->upper_vnum; vnum++)
		{
			ROOM_INDEX_DATA *source = get_room_index(parent->area, vnum);

			if( source )
			{
				room = create_virtual_room_nouid(source,false,false,true);

				if( !room )
				{
					free_instance_section(section);
					return NULL;
				}

				get_vroom_id(room);

				if( !list_appendlink(section->rooms, room) )
				{
					extract_clone_room(room->source,room->id[0],room->id[1],true);
					free_instance_section(section);
					return NULL;
				}

				room->instance_section = section;
			}
		}

		iterator_start(&rit, section->rooms);
		while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
		{
			// Clone the exits
			for(int i = 0; i < MAX_DIR; i++)
			{
				EXIT_DATA *exParent = room->source->exit[i];

				if( exParent )
				{
					EXIT_DATA *exClone;

					room->exit[i] = exClone = new_exit();
					exClone->u1.to_room = instance_section_get_room(section, exParent->u1.to_room);
					exClone->exit_info = exParent->exit_info;
					exClone->keyword = str_dup(exParent->keyword);
					exClone->short_desc = str_dup(exParent->short_desc);
					exClone->long_desc = str_dup(exParent->long_desc);
					exClone->rs_flags = exParent->rs_flags;
					exClone->orig_door = exParent->orig_door;
					exClone->door.strength = exParent->door.strength;
					exClone->door.material = exParent->door.material;
					exClone->door.lock = exParent->door.rs_lock;
					exClone->door.rs_lock = exParent->door.rs_lock;
					exClone->from_room = room;
				}
			}
		}
		iterator_stop(&rit);

	}
	else if(parent->type == BSTYPE_MAZE)
	{
		if (!blueprint_section_generate_maze(section, parent))
		{
			free_instance_section(section);
			return NULL;
		}
	}
	else
	{
		free_instance_section(section);
		return NULL;
	}

	return section;
}

INSTANCE_SECTION *instance_get_section(INSTANCE *instance, int section_no)
{
	if (!IS_VALID(instance) || !section_no) return NULL;

	// Get INSTANCE_SECTION by ordinal
	if (section_no < 0)
	{
		int ordinal = -section_no;
		ITERATOR it;
		INSTANCE_SECTION *section = NULL;
		iterator_start(&it, instance->sections);
		while( (section = (INSTANCE_SECTION *)iterator_nextdata(&it)) )
		{
			if (section->ordinal == ordinal)
				break;
		}
		iterator_stop(&it);

		return section;
	}

	return list_nthdata(instance->sections, section_no);
}

BLUEPRINT_LINK *instance_get_section_link(INSTANCE_SECTION *section, int link_no)
{
	if (!IS_VALID(section)) return NULL;
	
	return get_section_link(section->section, link_no);
}

static bool add_instance_section(INSTANCE *instance, BLUEPRINT_SECTION *bs, int ordinal)
{
	if (!IS_VALID(bs))
		return true;

	INSTANCE_SECTION *section = clone_blueprint_section(bs);
	section->ordinal = ordinal;
	section->blueprint = instance->blueprint;
	section->instance = instance;

	list_appendlist(instance->rooms, section->rooms);
	list_appendlink(instance->sections, section);
	return false;
}

static bool add_instance_layout(INSTANCE *instance, BLUEPRINT_LAYOUT_SECTION_DATA *ls)
{
	BLUEPRINT_SECTION *bs;
	BLUEPRINT *bp = instance->blueprint;
	switch(ls->mode)
	{
		case SECTIONMODE_STATIC:
			bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);
			return add_instance_section(instance, bs, ls->ordinal);

		case SECTIONMODE_WEIGHTED:
		{
			int w = number_range(1, ls->total_weight);
			bs = NULL;

			BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
			ITERATOR wit;
			iterator_start(&wit, ls->weighted_sections);
			while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)))
			{
				if (w <= weighted->weight)
				{
					bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
					break;
				}

				w -= weighted->weight;
			}
			iterator_stop(&wit);

			if (!IS_VALID(bs))
			{
				return true;
			}

			return add_instance_section(instance, bs, ls->ordinal);
		}

		case SECTIONMODE_GROUP:
		{
			bool error = false;
			BLUEPRINT_LAYOUT_SECTION_DATA *data;

			int count = list_size(ls->group);

			int *source = (int *)alloc_mem(sizeof(int) * count);
			for(int i = 0; i < count; i++)
				source[i] = i + 1;

			for(int i = count - 1; i >= 0; i--)
			{
				int isection = number_range(0, i);
				int nsection = source[isection];
				source[isection] = source[i];

				data = (BLUEPRINT_LAYOUT_SECTION_DATA *)list_nthdata(ls->group, nsection);

				if (add_instance_layout(instance, data))
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

BLUEPRINT_WEIGHTED_LINK_DATA *get_weighted_link(LLIST *list, int total)
{
	int w = number_range(1, total);

	ITERATOR it;
	BLUEPRINT_WEIGHTED_LINK_DATA *weighted = NULL;
	iterator_start(&it, list);
	while((weighted = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&it)))
	{
		if (w <= weighted->weight)
			break;

		w -= weighted->weight;
	}
	iterator_stop(&it);

	return weighted;
}

bool add_instance_link_from_to(INSTANCE *instance, BLUEPRINT_LAYOUT_LINK_DATA *link,
	BLUEPRINT_WEIGHTED_LINK_DATA *from, BLUEPRINT_WEIGHTED_LINK_DATA *to)
{
	if (!from || !to) return true;

	INSTANCE_SECTION *fsection = instance_get_section(instance, from->section);
	if (!IS_VALID(fsection))
		return true;

	INSTANCE_SECTION *tsection = instance_get_section(instance, to->section);
	if (!IS_VALID(tsection))
		return true;



	BLUEPRINT_LINK *flink = get_section_link(fsection->section, from->link);
	BLUEPRINT_LINK *tlink = get_section_link(tsection->section, to->link);

	if( tlink && tlink )
	{
		// TODO: change vnum to offset
		ROOM_INDEX_DATA *froom = instance_section_get_room_byvnum(fsection, flink->vnum);
		ROOM_INDEX_DATA *troom = instance_section_get_room_byvnum(tsection, tlink->vnum);

		if( froom && troom )
		{
			EXIT_DATA *fex = froom->exit[flink->door];
			EXIT_DATA *tex = troom->exit[tlink->door];

			if( !fex )
			{
				fex = new_exit();
				fex->from_room = froom;
				fex->orig_door = flink->door;

				froom->exit[flink->door] = fex;
			}

			if( !tex )
			{
				tex = new_exit();
				tex->from_room = troom;
				tex->orig_door = tlink->door;

				froom->exit[tlink->door] = tex;
			}

			REMOVE_BIT(fex->rs_flags, EX_ENVIRONMENT);
			fex->exit_info = fex->rs_flags;
			fex->door.lock = fex->door.rs_lock;
			fex->u1.to_room = troom;

			REMOVE_BIT(tex->rs_flags, EX_ENVIRONMENT);
			tex->exit_info = tex->rs_flags;
			tex->door.lock = tex->door.rs_lock;
			tex->u1.to_room = froom;
		}
	}

	return false;
}

bool add_instance_link(INSTANCE *instance, BLUEPRINT_LAYOUT_LINK_DATA *link)
{
	bool error = false;
	BLUEPRINT_WEIGHTED_LINK_DATA *from = NULL;
	BLUEPRINT_WEIGHTED_LINK_DATA *to = NULL;

	switch(link->mode)
	{
		case LINKMODE_STATIC:		//   STATIC - STATIC
			from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->from, 1);
			to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->to, 1);
			break;

		case LINKMODE_SOURCE:		// WEIGHTED - STATIC
			from = get_weighted_link(link->from, link->total_from);
			to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->to, 1);
			break;
		
		case LINKMODE_DESTINATION:	//   STATIC - WEIGHTED
			from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->from, 1);
			to = get_weighted_link(link->to, link->total_to);
			break;
		
		case LINKMODE_WEIGHTED:		// WEIGHTED - WEIGHTED
			from = get_weighted_link(link->from, link->total_from);
			to = get_weighted_link(link->to, link->total_to);
			break;

		case LINKMODE_GROUP:
		{
			int count = list_size(link->group);

			int *dest = (int *)alloc_mem(sizeof(int) * count);

			for(int i = 0; i < count; i++)
				dest[i] = i + 1;

			for(int i = count - 1; i >= 0; i--)
			{
				int ilink = number_range(0, i);
				int ndest = dest[ilink];
				dest[ilink] = dest[i];

				BLUEPRINT_LAYOUT_LINK_DATA *flink = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(link->group, i + 1);
				BLUEPRINT_LAYOUT_LINK_DATA *tlink = (BLUEPRINT_LAYOUT_LINK_DATA *)list_nthdata(link->group, ndest);

				BLUEPRINT_WEIGHTED_LINK_DATA *from = get_weighted_link(flink->from, flink->total_from);
				BLUEPRINT_WEIGHTED_LINK_DATA *to = get_weighted_link(tlink->to, tlink->total_to);

				if (add_instance_link_from_to(instance, link, from, to))
				{
					error = true;
					break;
				}
			}

			free_mem(dest, sizeof(int) * count);
			return error;
		}
	}

	return add_instance_link_from_to(instance, link, from, to);
}

bool generate_instance(INSTANCE *instance)
{
	ITERATOR lsit;
	BLUEPRINT *bp = instance->blueprint;

	// Blueprint layout is scripted
	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))	
	{
		// TODO: Add "instance layout" script command - for creating the actual layout
		// TODO: Add "instance link" script command - for linking the sections together
		// TODO: Add "instance special" script command - for designating any special rooms
		list_clear(bp->layout);
		list_clear(bp->links);
		list_clear(bp->special_rooms);
		bp->recall = 0;
		p_percent2_trigger(NULL, instance, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_BLUEPRINT_SCHEMATIC, NULL,0,0,0,0,0);
	}

	// Generate the sections
	bool error = false;
	BLUEPRINT_LAYOUT_SECTION_DATA *ls;
	iterator_start(&lsit, bp->layout);
	while((ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&lsit)))
	{
		if (add_instance_layout(instance, ls))
		{
			error = true;
			break;
		}

	}
	iterator_stop(&lsit);

	if (!error)
	{
		// Link the sections together
		ITERATOR llit;
		BLUEPRINT_LAYOUT_LINK_DATA *link;
		iterator_start(&llit, bp->links);
		while((link = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&llit)))
		{
			if (add_instance_link(instance, link))
			{
				error = true;
				break;
			}	
		}
		iterator_stop(&llit);

		if (error) return true;

		// Assign the entry exit (PREVFLOOR) if defined
		BLUEPRINT_EXIT_DATA *bex = list_nthdata(bp->entrances, 1);
		if( bex )
		{
			INSTANCE_SECTION *section = instance_get_section(instance, bex->section);
			if( section )
			{
				BLUEPRINT_LINK *bl = get_section_link(section->section, bex->link);

				if( bl )
				{
					ROOM_INDEX_DATA *room = instance_section_get_room_byvnum(section, bl->vnum);
					if( room )
					{
						EXIT_DATA *ex = room->exit[bl->door];

						if( !ex )
						{
							ex = new_exit();
							ex->from_room = room;
							ex->orig_door = bl->door;
							room->exit[bl->door] = ex;
						}

						REMOVE_BIT(ex->rs_flags, EX_ENVIRONMENT);
						SET_BIT(ex->rs_flags, EX_PREVFLOOR);
						ex->exit_info = ex->rs_flags;
						ex->door.lock = ex->door.rs_lock;
						ex->u1.to_room = NULL;

						instance->entrance = room;
					}
				}
			}
		}

		// Assign the exit exit (NEXTFLOOR) if defined
		bex = list_nthdata(bp->exits, 1);
		if( bex )
		{
			INSTANCE_SECTION *section = instance_get_section(instance, bex->section);
			if( section )
			{
				BLUEPRINT_LINK *bl = get_section_link(section->section, bex->link);

				if( bl )
				{
					ROOM_INDEX_DATA *room = instance_section_get_room_byvnum(section, bl->vnum);
					if( room )
					{
						EXIT_DATA *ex = room->exit[bl->door];

						if( !ex )
						{
							ex = new_exit();
							ex->from_room = room;
							ex->orig_door = bl->door;
							room->exit[bl->door] = ex;
						}

						REMOVE_BIT(ex->rs_flags, EX_ENVIRONMENT);
						SET_BIT(ex->rs_flags, EX_NEXTFLOOR);
						ex->exit_info = ex->rs_flags;
						ex->door.lock = ex->door.rs_lock;
						ex->u1.to_room = NULL;


						instance->exit = room;
					}
				}
			}
		}

		// Assign the recall point based upon the recall section's recall, if defined
		if( bp->recall != 0 )
		{
			INSTANCE_SECTION *recall_section = instance_get_section(instance, bp->recall);

			if( recall_section && recall_section->section->recall > 0 )
			{
				if (recall_section->section->type == BSTYPE_STATIC)
					instance->recall = instance_section_get_room_byvnum(recall_section, recall_section->section->recall);
				else if (recall_section->section->type == BSTYPE_MAZE)
					instance->recall = list_nthdata(recall_section->rooms, recall_section->section->recall);
			}
		}
	}

	return error;
}

/*
bool generate_static_instance(INSTANCE *instance)
{
	ITERATOR bsit;
	BLUEPRINT_SECTION *bs;
	BLUEPRINT *bp = instance->blueprint;

	bool valid = true;
	iterator_start(&bsit, bp->sections);
	while((bs = (BLUEPRINT_SECTION *)iterator_nextdata(&bsit)))
	{
		INSTANCE_SECTION *section = clone_blueprint_section(bs);

		if( !section )
		{
			valid = false;
			break;
		}

		section->blueprint = bp;
		section->instance = instance;

		list_appendlist(instance->rooms, section->rooms);

		list_appendlink(instance->sections, section);
	}
	iterator_stop(&bsit);

	if( valid )
	{
		// Connect all the sections together
		STATIC_BLUEPRINT_LINK *link;

		for(link = bp->_static.layout; link; link = link->next)
		{
			INSTANCE_SECTION *section1 = instance_get_section(instance, link->section1);
			INSTANCE_SECTION *section2 = instance_get_section(instance, link->section2);

			if( section1 && section2 )
			{
				BLUEPRINT_LINK *link1 = get_section_link(section1->section, link->link1);
				BLUEPRINT_LINK *link2 = get_section_link(section2->section, link->link2);

				if( link1 && link2 )
				{
					ROOM_INDEX_DATA *room1 = instance_section_get_room_byvnum(section1, link1->vnum);
					ROOM_INDEX_DATA *room2 = instance_section_get_room_byvnum(section2, link2->vnum);

					if( room1 && room2 )
					{
						EXIT_DATA *ex1 = room1->exit[link1->door];
						EXIT_DATA *ex2 = room2->exit[link2->door];

						if( !ex1 )
						{
							ex1 = new_exit();
							ex1->from_room = room1;
							ex1->orig_door = link1->door;

							room1->exit[link1->door] = ex1;
						}

						if( !ex2 )
						{
							ex2 = new_exit();
							ex2->from_room = room2;
							ex2->orig_door = link2->door;

							room1->exit[link2->door] = ex2;
						}

						REMOVE_BIT(ex1->rs_flags, EX_ENVIRONMENT);
						ex1->exit_info = ex1->rs_flags;
						ex1->door.lock = ex1->door.rs_lock;
						ex1->u1.to_room = room2;

						REMOVE_BIT(ex2->rs_flags, EX_ENVIRONMENT);
						ex2->exit_info = ex2->rs_flags;
						ex2->door.lock = ex2->door.rs_lock;
						ex2->u1.to_room = room1;
					}
				}
			}
		}

		// Assign the entry exit (PREVFLOOR) if defined
		BLUEPRINT_EXIT_DATA *bex = list_nthdata(bp->_static.entries, 1);
		if( bex )
		{
			INSTANCE_SECTION *section = instance_get_section(instance, bex->section);
			if( section )
			{
				BLUEPRINT_LINK *bl = get_section_link(section->section, bex->link);

				if( bl )
				{
					ROOM_INDEX_DATA *room = instance_section_get_room_byvnum(section, bl->vnum);
					if( room )
					{
						EXIT_DATA *ex = room->exit[bl->door];

						if( !ex )
						{
							ex = new_exit();
							ex->from_room = room;
							ex->orig_door = bl->door;
							room->exit[bl->door] = ex;
						}

						REMOVE_BIT(ex->rs_flags, EX_ENVIRONMENT);
						SET_BIT(ex->rs_flags, EX_PREVFLOOR);
						ex->exit_info = ex->rs_flags;
						ex->door.lock = ex->door.rs_lock;
						ex->u1.to_room = NULL;

						instance->entrance = room;
					}
				}
			}
		}

		// Assign the exit exit (NEXTFLOOR) if defined
		bex = list_nthdata(bp->_static.exits, 1);
		if( bex )
		{
			INSTANCE_SECTION *section = instance_get_section(instance, bex->section);
			if( section )
			{
				BLUEPRINT_LINK *bl = get_section_link(section->section, bex->link);

				if( bl )
				{
					ROOM_INDEX_DATA *room = instance_section_get_room_byvnum(section, bl->vnum);
					if( room )
					{
						EXIT_DATA *ex = room->exit[bl->door];

						if( !ex )
						{
							ex = new_exit();
							ex->from_room = room;
							ex->orig_door = bl->door;
							room->exit[bl->door] = ex;
						}

						REMOVE_BIT(ex->rs_flags, EX_ENVIRONMENT);
						SET_BIT(ex->rs_flags, EX_NEXTFLOOR);
						ex->exit_info = ex->rs_flags;
						ex->door.lock = ex->door.rs_lock;
						ex->u1.to_room = NULL;


						instance->exit = room;
					}
				}
			}
		}

		// Assign the recall point based upon the recall section's recall, if defined
		if( bp->_static.recall > 0 )
		{
			INSTANCE_SECTION *recall_section = instance_get_section(instance, bp->_static.recall);

			if( recall_section )
			{
				instance->recall = instance_section_get_room_byvnum(recall_section, recall_section->section->recall);
			}
		}
	}

	return valid;
}
*/

void instance_section_reset_rooms(INSTANCE_SECTION *section)
{
	ITERATOR rit;
	ROOM_INDEX_DATA *room;

	iterator_start(&rit, section->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
	{
		reset_room(room, false);
	}

	iterator_stop(&rit);
}

void reset_instance(INSTANCE *instance)
{
	ITERATOR it;
	INSTANCE_SECTION *section;
	iterator_start(&it, instance->sections);
	while( (section = (INSTANCE_SECTION *)iterator_nextdata(&it)) )
	{
		instance_section_reset_rooms(section);
	}
	iterator_stop(&it);
}

inline static void __instance_set_portal_room(OBJ_DATA *obj, ROOM_INDEX_DATA *room)
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

static void __instance_correct_portals(INSTANCE *instance)
{
	ROOM_INDEX_DATA *room;
	ITERATOR rit;

	iterator_start(&rit, instance->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
	{
		INSTANCE_SECTION *section = room->instance_section;

		// Correct any portal objects
		for(OBJ_DATA *obj = room->contents; obj; obj = obj->next_content)
		{
			// Make sure portals that lead anywhere within the instance uses the correct room id
			if( IS_PORTAL(obj) )
			{
				switch(PORTAL(obj)->type)
				{
					case GATETYPE_NORMAL:
					{
						long auid = PORTAL(obj)->params[0];
						long vnum = PORTAL(obj)->params[1];
						ROOM_INDEX_DATA *dest;

						if ( auid > 0 && vnum > 0 )
						{
							// If the portal is pointing to a room in the blueprint
							if (room->area->uid == auid && (dest = instance_section_get_room_byvnum(section, vnum)))
							{
								PORTAL(obj)->params[2] = dest->id[0];
								PORTAL(obj)->params[3] = dest->id[1];
							}
							else
							{
								dest = get_room_index_auid(auid, vnum);

								if( !dest ||
									IS_SET(dest->room_flag[1], ROOM_BLUEPRINT) ||
									IS_SET(dest->area->area_flags, AREA_BLUEPRINT) )
								{
									// Nullify destination
									PORTAL(obj)->params[0] = 0;
									PORTAL(obj)->params[1] = 0;
								}

								// Force it to be static
								PORTAL(obj)->params[2] = 0;
								PORTAL(obj)->params[3] = 0;
							}
						}
						break;
					}

					case GATETYPE_BLUEPRINT_SECTION_MAZE:
					{
						ROOM_INDEX_DATA *room = NULL;
						if (PORTAL(obj)->params[0])
						{
							section = instance_get_section(instance, PORTAL(obj)->params[0]);
						}

						// is it a maze section?
						if (IS_VALID(section) && section->section->type == BSTYPE_MAZE)
						{
							if (PORTAL(obj)->params[0] > 0 && PORTAL(obj)->params[0] <= section->section->maze_x &&
								PORTAL(obj)->params[1] > 0 && PORTAL(obj)->params[1] <= section->section->maze_y)
							{
								int index = (PORTAL(obj)->params[1] - 1) * section->section->maze_x + PORTAL(obj)->params[0];

								room = list_nthdata(section->rooms, index);
							}
						}
						
						__instance_set_portal_room(obj, room);
						break;
					}

					case GATETYPE_BLUEPRINT_SPECIAL:
					{
						ROOM_INDEX_DATA *room = get_instance_special_room(instance, PORTAL(obj)->params[0]);

						__instance_set_portal_room(obj, room);
						break;
					}
				}
			}
		}
	}
	iterator_stop(&rit);

}

INSTANCE *create_instance(BLUEPRINT *blueprint)
{
	INSTANCE *instance = new_instance();

	if( instance )
	{
		instance->blueprint = blueprint;
		instance->flags = blueprint->flags;

		instance->progs			= new_prog_data();
		instance->progs->progs	= blueprint->progs;
		variable_copylist(&blueprint->index_vars,&instance->progs->vars,false);

		//wiznet("create_instance: generating instance...",NULL,NULL,WIZ_TESTING,0,0);
		if (generate_instance(instance))
		{
			//wiznet("create_instance: generation failed",NULL,NULL,WIZ_TESTING,0,0);
			free_instance(instance);
			return NULL;
		}

		//wiznet("create_instance: special rooms",NULL,NULL,WIZ_TESTING,0,0);
		ITERATOR it;
		INSTANCE_SECTION *section;
		BLUEPRINT_SPECIAL_ROOM *special;
		iterator_start(&it, blueprint->special_rooms);
		while( (special = (BLUEPRINT_SPECIAL_ROOM *)iterator_nextdata(&it)) )
		{
			section = instance_get_section(instance, special->section);
			if( IS_VALID(section) )
			{
				ROOM_INDEX_DATA *room = instance_section_get_room_byoffset(section, special->offset);

				if( room )
				{
					NAMED_SPECIAL_ROOM *isr = new_named_special_room();

					free_string(isr->name);
					isr->name = str_dup(special->name);
					isr->room = room;

					list_appendlink(instance->special_rooms, isr);
				}
			}

		}
		iterator_stop(&it);

		//wiznet("create_instance: reseting instance",NULL,NULL,WIZ_TESTING,0,0);
		reset_instance(instance);

		get_instance_id(instance);

		//wiznet("create_instance: correcting portals",NULL,NULL,WIZ_TESTING,0,0);
		__instance_correct_portals(instance);

		//wiznet("create_instance: instance created",NULL,NULL,WIZ_TESTING,0,0);
	}

	return instance;
}


void update_instance_section(INSTANCE_SECTION *section)
{
	ITERATOR rit;
	ROOM_INDEX_DATA *room;

	iterator_start(&rit, section->rooms);
	while((room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)))
		room_update(room);
	iterator_stop(&rit);
}

void update_instance(INSTANCE *instance)
{
	p_percent2_trigger(NULL, instance, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM, NULL,0,0,0,0,0);

	ITERATOR sit;
	INSTANCE_SECTION *section;
	iterator_start(&sit, instance->sections);
	while( (section = (INSTANCE_SECTION *)iterator_nextdata(&sit)) )
	{
		update_instance_section(section);
	}
	iterator_stop(&sit);
}

bool instance_can_idle(INSTANCE *instance)
{
	return IS_SET(instance->flags, INSTANCE_DESTROY) ||
			(!IS_SET(instance->flags, INSTANCE_NO_IDLE) &&
				(!IS_SET(instance->flags, INSTANCE_IDLE_ON_COMPLETE) ||
				IS_SET(instance->flags, INSTANCE_COMPLETED) ||
				IS_SET(instance->flags, INSTANCE_FAILED)));
}

void instance_check_empty(INSTANCE *instance)
{
	if( instance->empty )
	{
		if( list_size(instance->players) > 0 )
			instance->empty = false;
	}
	else if( list_size(instance->players) < 1 )
	{
		instance->empty = true;
		if( instance_can_idle(instance) )
			instance->idle_timer = UMAX(15, instance->idle_timer);
	}

	if( !instance->empty && !instance_can_idle(instance) )
		instance->idle_timer = 0;
}


void instance_update()
{
	ITERATOR it;
	INSTANCE *instance;

	iterator_start(&it, loaded_instances);
	while((instance = (INSTANCE *)iterator_nextdata(&it)))
	{
		// Skip instances owned by dungeons
		if( IS_VALID(instance->dungeon) ) continue;

		if( IS_VALID(instance->ship) ) continue;

		if( instance_isorphaned(instance) && list_size(instance->players) < 1 )
		{
			// Do not keep an empty orphaned instance
			extract_instance(instance);
			continue;
		}

		update_instance(instance);

		if( instance_can_idle(instance) )
		{
			if( instance->idle_timer > 0 )
			{
				if( !--instance->idle_timer )
				{
					extract_instance(instance);
					continue;
				}
			}
		}

		instance_check_empty(instance);

		instance->age++;
		if( instance->blueprint->repop > 0 && (instance->age >= instance->blueprint->repop) )
		{
			p_percent2_trigger(NULL, instance, NULL, NULL, NULL, NULL, NULL, NULL, TRIG_RESET, NULL,0,0,0,0,0);

			reset_instance(instance);

			instance->age = 0;
		}
	}
	iterator_stop(&it);
}

void extract_instance(INSTANCE *instance)
{
	ITERATOR it;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *environ = NULL;

    if(instance->progs) {
	    SET_BIT(instance->progs->entity_flags,PROG_NODESTRUCT);
	    if(instance->progs->script_ref > 0) {
			instance->progs->extract_when_done = true;
			return;
		}
    }

	if( IS_VALID(instance->object) )
		environ = obj_room(instance->object);

	else if( IS_VALID(instance->ship) )
		environ = obj_room(instance->ship->ship);

	if( !environ )
		environ = instance->environ;

	room = environ;
	if( !room )
		room = get_room_index_wnum(room_wnum_temple);

	// Dump all mobiles
	iterator_start(&it, instance->mobiles);
	while( (ch = (CHAR_DATA *)iterator_nextdata(&it)) )
	{
		char_from_room(ch);
		char_to_room(ch, room);
	}
	iterator_stop(&it);

	// Dump objects
	room = environ;
	if( !room )
		room = get_room_index_wnum(room_wnum_donation);

	iterator_start(&it, instance->objects);
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

	list_remlink(loaded_instances, instance, true);

	free_instance(instance);
}

void instance_apply_specialkeys(INSTANCE *instance, LLIST *special_keys)
{
	ITERATOR sit, rit;
	INSTANCE_SECTION *section;
	ROOM_INDEX_DATA *room;

	if( !IS_VALID(instance) || !IS_VALID(special_keys) ) return;

	iterator_start(&sit, instance->sections);
	while( (section = (INSTANCE_SECTION *)iterator_nextdata(&sit)) )
	{
		iterator_start(&rit, section->rooms);
		while( (room = (ROOM_INDEX_DATA *)iterator_nextdata(&rit)) )
		{
			for( int i = 0; i < MAX_DIR; i++ )
			{
				EXIT_DATA *ex = room->exit[i];

				if( ex && ex->door.lock.key_wnum.pArea && ex->door.lock.key_wnum.vnum > 0 )
				{
					SPECIAL_KEY_DATA *sk = get_special_key(special_keys, ex->door.lock.key_wnum);

					if( sk )
					{
						ex->door.lock.special_keys = sk->list;
						ex->door.rs_lock.special_keys = sk->list;
					}
				}
			}
		}
		iterator_stop(&rit);
	}
	iterator_stop(&sit);
}

CHAR_DATA *instance_section_find_mobile(INSTANCE_SECTION *section, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	ROOM_INDEX_DATA *room;
	CHAR_DATA *mob = NULL, *ch;

	iterator_start(&it, section->rooms);
	while( !IS_VALID(mob) && (room = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) )
	{
		for( ch = room->people; ch; ch = ch->next_in_room )
		{
			if( ch->id[0] == id1 && ch->id[1] == id2 )
			{
				mob = ch;
				break;
			}
		}
	}
	iterator_stop(&it);

	return mob;
}

CHAR_DATA *instance_find_mobile(INSTANCE *instance, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	INSTANCE_SECTION *section;
	CHAR_DATA *mob = NULL;

	iterator_start(&it, instance->sections);
	while( !IS_VALID(mob) && (section = (INSTANCE_SECTION *)iterator_nextdata(&it)) )
	{
		mob = instance_section_find_mobile(section, id1, id2);
	}
	iterator_stop(&it);

	return mob;
}

//////////////////////////////////////////////////////////////
//
// OLC Editors
//
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
//
// Blueprint Section Edit
//

const struct olc_cmd_type bsedit_table[] =
{
	{ "?",				show_help			},
	{ "commands",		show_commands		},
	{ "list",			bsedit_list			},
	{ "show",			bsedit_show			},
	{ "create",			bsedit_create		},
	{ "name",			bsedit_name			},
	{ "description",	bsedit_description	},
	{ "comments",		bsedit_comments		},
	{ "type",			bsedit_type			},
	{ "flags",			bsedit_flags		},
	{ "recall",			bsedit_recall		},
	{ "rooms",			bsedit_rooms		},
	{ "link",			bsedit_link			},
	{ "maze",			bsedit_maze			},
	{ NULL,				NULL				}

};

bool can_edit_blueprints(CHAR_DATA *ch)
{
	// TODO: Add blueprint certification for imms
	return !IS_NPC(ch) && (ch->pcdata->security >= 9) && IS_IMPLEMENTOR(ch);
}

void list_blueprint_sections(CHAR_DATA *ch, char *argument)
{
	if( !can_edit_blueprints(ch) )
	{
		send_to_char("You do not have access to blueprints.\n\r", ch);
		return;
	}

	if(!ch->lines)
		send_to_char("{RWARNING:{W Having scrolling off may limit how many sections you can see.{x\n\r", ch);

	AREA_DATA *pArea = ch->in_room->area;
	int lines = 0;
	bool error = false;
	BUFFER *buffer = new_buf();
	char buf[MSL];

	for(long vnum = 1; vnum <= pArea->top_blueprint_section_vnum; vnum++)
	{
		BLUEPRINT_SECTION *section = get_blueprint_section(pArea, vnum);

		if( section )
		{
			if (section->type == BSTYPE_STATIC)
				sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s  {G%-16.16s{x   %11ld   %11ld-%-11ld \n\r",
					vnum,
					section->name,
					"static",
					section->recall,
					section->lower_vnum,
					section->upper_vnum);
			else if(section->type == BSTYPE_MAZE)
			{
				if (section->recall > 0)
				{
					int rx = (section->recall - 1) % section->maze_x + 1;
					int ry = (section->recall - 1) / section->maze_x + 1;
					sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s  {G%-16.16s{x   (%4d %4d)    (%10ld %-10ld) \n\r",
						vnum,
						section->name,
						"maze",
						rx, ry,
						section->maze_x,
						section->maze_y);
				}
				else
					sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s  {G%-16.16s{x                  (%10ld %-10ld) \n\r",
						vnum,
						section->name,
						"maze",
						section->maze_x,
						section->maze_y);
			}
			else
					sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s  {G%-16.16s{x\n\r",
						vnum,
						section->name,
						"invalid");

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
		send_to_char("Too many blueprints to list.  Please shorten!\n\r", ch);
	}
	else
	{
		if( !lines )
		{
			add_buf( buffer, "No blueprint sections to display.\n\r" );
		}
		else
		{
			// Header
			send_to_char("{Y Vnum   [            Name            ] [      Type      ] [  Recall   ] [    Vnum Range/Size    ]{x\n\r", ch);
			send_to_char("{Y=================================================================================================={x\n\r", ch);
		}

		page_to_char(buffer->string, ch);
	}
	free_buf(buffer);
}

void do_bslist(CHAR_DATA *ch, char *argument)
{
	list_blueprint_sections(ch, argument);
}

void do_bsedit(CHAR_DATA *ch, char *argument)
{
	BLUEPRINT_SECTION *bs;
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
			send_to_char("BsEdit:  widevnum in an area you cannot build in.\n\r", ch);
			return;
		}

		if (!(bs = get_blueprint_section(wnum.pArea, wnum.vnum)))
		{
			send_to_char("BSEdit:  That vnum does not exist.\n\r", ch);
			return;
		}

		ch->pcdata->immortal->last_olc_command = current_time;
		olc_set_editor(ch, ED_BPSECT, bs);
		return;
	}
	else
	{
		if (!str_cmp(arg1, "create"))
		{
			if (bsedit_create(ch, argument))
			{
				ch->pcdata->immortal->last_olc_command = current_time;
				ch->desc->editor = ED_BPSECT;

				EDIT_BPSECT(ch, bs);
				SET_BIT(bs->area->area_flags, AREA_CHANGED);
			}

			return;
		}
	}

	send_to_char("Syntax: bsedit <vnum>\n\r"
				 "        bsedit create <vnum>\n\r", ch);
}


void bsedit(CHAR_DATA *ch, char *argument)
{
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  cmd;
	BLUEPRINT_SECTION *bs;
	AREA_DATA *pArea;

	EDIT_BPSECT(ch, bs);
	pArea = bs->area;

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
		bsedit_show(ch, argument);
		return;
	}

	for (cmd = 0; bsedit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix(command, bsedit_table[cmd].name))
		{
			if ((*bsedit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT(pArea->area_flags, AREA_CHANGED);
			}

			return;
		}
	}

    interpret(ch, arg);
}




void do_bsshow(CHAR_DATA *ch, char *argument)
{
	BLUEPRINT_SECTION *bs;
	WNUM wnum;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  bsshow <widevnum>\n\r", ch);
		return;
	}

	if (!parse_widevnum(argument, ch->in_room->area, &wnum))
	{
		send_to_char("Please specify a widevnum.\n\r", ch);
		return;
	}

	if (!(bs = get_blueprint_section(wnum.pArea, wnum.vnum)))
	{
		send_to_char("That blueprint section does not exist.\n\r", ch);
		return;
	}

	olc_show_item(ch, bs, bsedit_show, argument);
	return;
}




bool validate_vnum_range(CHAR_DATA *ch, BLUEPRINT_SECTION *section, long lower, long upper)
{
	char buf[MSL];

	// Check that are no overlaps
	BLUEPRINT_SECTION *bs;
	int iHash;
	for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for(bs = section->area->blueprint_section_hash[iHash]; bs; bs = bs->next)
		{
			// Only check against other sections
			if (bs == section) continue;

			if( bs->type == BSTYPE_STATIC )
			{
				if( (lower >= bs->lower_vnum && lower <= bs->upper_vnum ) ||
					(upper >= bs->lower_vnum && upper <= bs->upper_vnum ) ||
					(bs->lower_vnum >= lower && bs->lower_vnum <= upper ) ||
					(bs->upper_vnum >= lower && bs->upper_vnum <= upper ) )
				{
					send_to_char("Blueprint section vnum ranges cannot overlap.\n\r", ch);
					return false;
				}
			}
			else if ( bs->type == BSTYPE_MAZE )
			{
				ITERATOR it;
				MAZE_FIXED_ROOM *mfr;
				iterator_start(&it, bs->maze_fixed_rooms);
				while((mfr = (MAZE_FIXED_ROOM *)iterator_nextdata(&it)))
				{
					if (mfr->vnum >= lower && mfr->vnum <= upper)
						break;
				}
				iterator_stop(&it);

				if (mfr)
				{
					send_to_char("Blueprint section vnum ranges cannot overlap.\n\r", ch);
					return false;
				}

				MAZE_WEIGHTED_ROOM *mwr;
				iterator_start(&it, bs->maze_templates);
				while((mwr = (MAZE_WEIGHTED_ROOM *)iterator_nextdata(&it)))
				{
					if( mwr->vnum >= lower && mwr->vnum <= upper)
						break;
				}
				iterator_stop(&it);

				if (mwr)
				{
					send_to_char("Blueprint section vnum ranges cannot overlap.\n\r", ch);
					return false;
				}
			}
		}
	}

	// Verify there are any rooms in the range
	bool found = false;
	bool valid = true;
	BUFFER *buffer = new_buf();		// This will buffer up ALL the problem rooms

	for(long vnum = lower; vnum <= upper; vnum++)
	{
		ROOM_INDEX_DATA *room = get_room_index(section->area, vnum);

		if( room )
		{
			found = true;

			if( !IS_SET(room->room_flag[1], ROOM_BLUEPRINT) &&
				!IS_SET(room->area->area_flags, AREA_BLUEPRINT) )
			{
				sprintf(buf, "{xRoom {W%ld{x is not allocated for use in blueprints.\n\r", room->vnum);
				add_buf(buffer, buf);
				valid = false;
			}

			if( IS_SET(room->room_flag[1], (ROOM_NOCLONE|ROOM_VIRTUAL_ROOM)) )
			{
				sprintf(buf, "{xRoom {W%ld{x cannot be used in blueprints.\n\r", room->vnum);
				add_buf(buffer, buf);
				valid = false;
			}

			// Verify the room does not have non-environment exits pointing OUT of the range of vnums
			for( int i = 0; i < MAX_DIR; i++ )
			{
				EXIT_DATA *ex = room->exit[i];

				if( (ex != NULL) &&
					!IS_SET(ex->exit_info, EX_ENVIRONMENT) &&
					(ex->u1.to_room != NULL) )
				{
					if( IS_SET(ex->exit_info, EX_VLINK) )
					{
						sprintf(buf, "{xRoom {W%ld{x has an exit ({W%s{x) leading to wilderness.\n\r", room->vnum, dir_name[i]);
						add_buf(buffer, buf);
						valid = false;
					}
					else if( ex->u1.to_room->area != ex->from_room->area )
					{
						sprintf(buf, "{xRoom {W%ld{x has an exit ({W%s{x) leading outside of the area.\n\r", room->vnum, dir_name[i]);
						add_buf(buffer, buf);
						valid = false;
					}
					else if( ex->u1.to_room->vnum < lower || ex->u1.to_room->vnum > upper )
					{
						sprintf(buf, "{xRoom {W%ld{x has an exit ({W%s{x) leading outside of the vnum range.\n\r", room->vnum, dir_name[i]);
						add_buf(buffer, buf);
						valid = false;
					}
				}
			}

		}
	}

	if( !found )
	{
		send_to_char("There are no rooms in that range.\n\r", ch);
	}
	else if( !valid )
	{
		page_to_char(buffer->string, ch);
	}

	free_buf(buffer);
	return found && valid;
}



static bool __verify_exit(ROOM_INDEX_DATA *room, int door, int x, int y, int w, int h)
{
	static bool allowed[4][MAX_DIR] = {
	//		N		E		S		W		U		D		NE		NW		SE		SW
		{	true,	false,	false,	false,	true,	true,	true,	true,	false,	false	}, // North Wall
		{   false,	false,	true,	false,	true,	true,	false,	false,	true,	true	}, // South Wall
		{	false,	true,	false,	false,	true,	true,	true,	false,	true,	false	}, // East Wall
		{	false,	false,	false,	true,	true,	true,	false,	true,	false,	true	}, // West Wall
	};

	EXIT_DATA *ex = room->exit[door];
	if (!ex) return true;		// Nothing there, then it's okay

	if (IS_SET(ex->exit_info, EX_ENVIRONMENT))
	{
		// Up/Down environment exits are allowed
		if (door == DIR_UP || door == DIR_DOWN) return true;

		// Check that the lateral environment exits only lead out of the maze bounds		
		if (x == 1)
		{
			// West wall
			if (allowed[3][door]) return true;
		}
		else if(x == w)
		{
			// East wall
			if (allowed[2][door]) return true;
		}

		if (y == 1)
		{
			// North wall
			if (allowed[0][door]) return true;
		}
		else if (y == h)
		{
			// South wall
			if (allowed[1][door]) return true;
		}
		
		// in the middle, no side exits allowed
		return false;
	}
	else
	{
		// Ordinary exit

		if (door == DIR_UP || door == DIR_DOWN) return true;

		// Check that ordinary exits on the walls do not leave the maze bounds
		if (x == 1)
		{
			// West wall
			if (allowed[3][door]) return false;
		}
		else if(x == w)
		{
			// East wall
			if (allowed[2][door]) return false;
		}

		if (y == 1)
		{
			// North wall
			if (allowed[0][door]) return false;
		}
		else if (y == h)
		{
			// South wall
			if (allowed[1][door]) return false;
		}
		
		// in the middle, can point anywhere other than up or down.
		return true;
	}
}




//////////////////////////////////////////////////////////////
//
// Blueprint Edit
//

const struct olc_cmd_type bpedit_table[] =
{
	{ "?",				show_help			},
	{ "addiprog",		bpedit_addiprog		},
	{ "areawho",		bpedit_areawho		},
	{ "commands",		show_commands		},
	{ "comments",		bpedit_comments		},
	{ "create",			bpedit_create		},
	{ "deliprog",		bpedit_deliprog		},
	{ "description",	bpedit_description	},
	{ "entrances",		bpedit_entrances	},
	{ "exits",			bpedit_exits		},
	{ "flags",			bpedit_flags		},
	{ "layout",			bpedit_layout		},
	{ "links",			bpedit_links		},
	{ "list",			bpedit_list			},
	{ "name",			bpedit_name			},
	{ "repop",			bpedit_repop		},
	{ "rooms",			bpedit_rooms		},
	{ "scripted",		bpedit_scripted		},
	{ "section",		bpedit_section		},
	{ "show",			bpedit_show			},
	{ "varclear",		bpedit_varclear		},
	{ "varset",			bpedit_varset		},
	{ NULL,				NULL				}
};

void list_blueprints(CHAR_DATA *ch, char *argument)
{
	AREA_DATA *area = ch->in_room->area;

	if(!ch->lines)
		send_to_char("{RWARNING:{W Having scrolling off may limit how many blueprints you can see.{x\n\r", ch);

	int lines = 0;
	bool error = false;
	BUFFER *buffer = new_buf();
	char buf[MSL];

	for(long vnum = 1; vnum <= area->top_blueprint_vnum; vnum++)
	{
		BLUEPRINT *blueprint= get_blueprint(area, vnum);

		if( blueprint )
		{
			sprintf(buf, "{Y[{W%5ld{Y] {x%-30.30s{x\n\r",
				vnum,
				blueprint->name);

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
		send_to_char("Too many blueprints to list.  Please shorten!\n\r", ch);
	}
	else
	{
		if( !lines )
		{
			add_buf( buffer, "No blueprint to display.\n\r" );
		}
		else
		{
			// Header
			send_to_char("Blueprints in current area:\n\r", ch);
			send_to_char("{Y Vnum   [            Name            ]{x\n\r", ch);
			send_to_char("{Y======================================={x\n\r", ch);
			page_to_char(buffer->string, ch);
		}

	}
	free_buf(buffer);
}


void do_bplist(CHAR_DATA *ch, char *argument)
{
	list_blueprints(ch, argument);
}

void do_bpedit(CHAR_DATA *ch, char *argument)
{
	BLUEPRINT *bp;
	WNUM wnum;
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	if (IS_NPC(ch))
		return;

	if (parse_widevnum(arg1, ch->in_room->area, &wnum))
	{
		if (!wnum.pArea || wnum.vnum < 1)
		{
			send_to_char("Widevnum is not associated with an area.\n\r", ch);
			return;
		}

	    if (!IS_BUILDER(ch, wnum.pArea))
	    {
			send_to_char("BpEdit:  widevnum in an area you cannot build in.\n\r", ch);
			return;
		}

		if (!(bp = get_blueprint(wnum.pArea, wnum.vnum)))
		{
			send_to_char("BPEdit:  That widevnum does not exist.\n\r", ch);
			return;
		}

		ch->pcdata->immortal->last_olc_command = current_time;
		olc_set_editor(ch, ED_BLUEPRINT, bp);
		return;
	}
	else
	{
		if (!str_cmp(arg1, "create"))
		{
			if (bpedit_create(ch, argument))
			{
				ch->pcdata->immortal->last_olc_command = current_time;
				ch->desc->editor = ED_BLUEPRINT;

				EDIT_BLUEPRINT(ch, bp);
				SET_BIT(bp->area->area_flags, AREA_CHANGED);
			}

			return;
		}
	}

	send_to_char("Syntax: bpedit <widevnum>\n\r"
				 "        bpedit create[ <widevnum>]\n\r", ch);
}


void bpedit(CHAR_DATA *ch, char *argument)
{
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  cmd;
	BLUEPRINT *bp;
	AREA_DATA *pArea;

	EDIT_BLUEPRINT(ch, bp);
	pArea = bp->area;

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
		bpedit_show(ch, argument);
		return;
	}

	for (cmd = 0; bpedit_table[cmd].name != NULL; cmd++)
	{
		if (!str_prefix(command, bpedit_table[cmd].name))
		{
			if ((*bpedit_table[cmd].olc_fun) (ch, argument))
			{
				SET_BIT(pArea->area_flags, AREA_CHANGED);
			}

			return;
		}
	}

    interpret(ch, arg);
}

void bpedit_buffer_layout(BUFFER *buffer, BLUEPRINT *bp)
{
	char buf[MSL];

	add_buf(buffer, "{yLayout:{x\n\r");
	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		add_buf(buffer, "  {WSCRIPTED{x\n\r");
	}
	else if(list_size(bp->layout) > 0)
	{
		ITERATOR it;
		BLUEPRINT_LAYOUT_SECTION_DATA *section;

		add_buf(buffer, "{y     [  Mode  ] [ Ordinal ]{x\n\r");
		add_buf(buffer, "{y===================================================={x\n\r");

		int section_no = 1;
		iterator_start(&it, bp->layout);
		while( (section = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)) )
		{
			switch(section->mode)
			{
				case SECTIONMODE_STATIC:
				{
					BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, section->section);
					sprintf(buf, "{W%4d  {Y STATIC {x   %7d       %4d - {x%23.23s{x\n\r", section_no++, section->ordinal, section->section, bs->name);
					add_buf(buffer, buf);
					break;
				}
				
				case SECTIONMODE_WEIGHTED:
				{
					ITERATOR wit;
					BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
					BLUEPRINT_SECTION *bs;

					sprintf(buf, "{W%4d  {CWEIGHTED{x    %7d\n\r", section_no++, section->ordinal);
					add_buf(buffer, buf);
					add_buf(buffer, "{c               [ Weight ] [             Section            ]{x\n\r");
					add_buf(buffer, "{c          ==================================================={x\n\r");

					int weight_no = 1;
					iterator_start(&wit, section->weighted_sections);
					while( (weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)) )
					{
						bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
						sprintf(buf, "          {c%4d   {W%6d     {x%4d - %23.23s{x\n\r", weight_no++, weighted->weight, weighted->section, bs->name);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "{c          ----------------------------------------------------{x\n\r");
					break;
				}

				case SECTIONMODE_GROUP:
				{
					ITERATOR git;
					BLUEPRINT_LAYOUT_SECTION_DATA *sect;

					sprintf(buf, "{W%4d  {G  GROUP {x\n\r", section_no++);
					add_buf(buffer, buf);
					if (list_size(section->group) > 0)
					{

						add_buf(buffer, "          {y     [  Mode  ] [ Ordinal ]{x\n\r");
						add_buf(buffer, "          {y===================================================={x\n\r");

						int gsection_no = 1;
						iterator_start(&git, section->group);
						while( (sect = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&git)) )
						{
							if (sect->mode == SECTIONMODE_STATIC)
							{
								BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, sect->section);
								sprintf(buf, "          {W%4d  {Y STATIC {x    %7d     %4d - {x%23.23s{x\n\r", gsection_no++, sect->ordinal, sect->section, bs->name);
								add_buf(buffer, buf);	
							}
							else if(sect->mode == SECTIONMODE_WEIGHTED)
							{
								ITERATOR wit;
								BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
								BLUEPRINT_SECTION *bs;

								sprintf(buf, "          {W%4d  {CWEIGHTED{x    %7d\n\r", gsection_no++, sect->ordinal);
								add_buf(buffer, buf);
								add_buf(buffer, "{c                         [ Weight ] [              Floor             ]{x\n\r");
								add_buf(buffer, "{c                    ==================================================={x\n\r");

								int weight_no = 1;
								iterator_start(&wit, sect->weighted_sections);
								while( (weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)) )
								{
									bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);
									sprintf(buf, "                    {c%4d   {W%6d     {x%4d - %23.23s{x\n\r", weight_no++, weighted->weight, weighted->section, bs->name);
									add_buf(buffer, buf);
								}
								iterator_stop(&wit);
								add_buf(buffer, "{c                    ----------------------------------------------------{x\n\r");
							}
						}
						iterator_stop(&git);

					}
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

void bpedit_buffer_links(BUFFER *buffer, BLUEPRINT *bp)
{
	char buf[MSL];

	add_buf(buffer, "{yLinks:{x\n\r");
	if (IS_SET(bp->flags, BLUEPRINT_SCRIPTED_LAYOUT))
	{
		add_buf(buffer, "  {WSCRIPTED{x\n\r");
	}
	else if(list_size(bp->links) > 0)
	{
		ITERATOR it;
		BLUEPRINT_LAYOUT_LINK_DATA *link;

		add_buf(buffer, "{x     [    Mode    ]{x\n\r");
		add_buf(buffer, "{x========================================================{x\n\r");

		int link_no = 1;
		iterator_start(&it, bp->links);
		while( (link = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&it)) )
		{
			BLUEPRINT_WEIGHTED_LINK_DATA *from;
			BLUEPRINT_WEIGHTED_LINK_DATA *to;
			switch(link->mode)
			{
				case LINKMODE_STATIC:
				{
					from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->from, 1);
					to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->to, 1);
					sprintf(buf, "%4d  {Y   STATIC   {x\n\r", link_no++);
					sprintf(buf, "          Source:        {%c%4d{x (%d)\n\r", (from->section<0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
					add_buf(buffer, buf);
					sprintf(buf, "          Destination:   {%c%4d{x (%d)\n\r", (to->section<0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
					add_buf(buffer, buf);
					break;
				}

				case LINKMODE_SOURCE:
				{
					ITERATOR wit;
					to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->to, 1);
					sprintf(buf, "%4d  {C   SOURCE   {x\n\r", link_no++);
					add_buf(buffer, buf);

					int fromlink_no = 1;
					add_buf(buffer, "          Source:\n\r");
					add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
					add_buf(buffer, "          ===================================={x\n\r");
					iterator_start(&wit, link->from);
					while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
					{
						sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section<0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "          ----------------------------------------------------{x\n\r");

					sprintf(buf,    "          Destination:   {%c%4d{x (%d)\n\r", (to->section<0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
					add_buf(buffer, buf);
					break;
				}

				case LINKMODE_DESTINATION:
				{
					ITERATOR wit;
					from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(link->from, 1);
					sprintf(buf, "%4d  {C DESTINATION{x\n\r", link_no++);
					add_buf(buffer, buf);
					sprintf(buf,    "          Source:        {%c%4d{x (%d)\n\r", (from->section<0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
					add_buf(buffer, buf);

					int tolink_no = 1;
					add_buf(buffer, "          Destination:\n\r");
					add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
					add_buf(buffer, "          ===================================={x\n\r");
					iterator_start(&wit, link->to);
					while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
					{
						sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section<0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "          ----------------------------------------------------{x\n\r");
					break;
				}

				case LINKMODE_WEIGHTED:
				{
					ITERATOR wit;
					sprintf(buf, "%4d  {C  WEIGHTED  {x\n\r", link_no++);
					add_buf(buffer, buf);

					int fromlink_no = 1;
					add_buf(buffer, "          Source:\n\r");
					add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
					add_buf(buffer, "          ===================================={x\n\r");
					iterator_start(&wit, link->from);
					while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
					{
						sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section<0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "          ----------------------------------------------------{x\n\r");

					int tolink_no = 1;
					add_buf(buffer, "          Destination:\n\r");
					add_buf(buffer, "               [ Weight ] [Section] [ Exit# ]{x\n\r");
					add_buf(buffer, "          ===================================={x\n\r");
					iterator_start(&wit, link->to);
					while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
					{
						sprintf(buf, "          %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section<0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
						add_buf(buffer, buf);
					}
					iterator_stop(&wit);
					add_buf(buffer, "          ----------------------------------------------------{x\n\r");
					break;
				}

				case LINKMODE_GROUP:
				{
					int count = list_size(link->group);
					sprintf(buf, "%4d  {G    GROUP   {x\n\r", link_no++);
					add_buf(buffer, buf);
					if(count > 0)
					{
						ITERATOR git;
						BLUEPRINT_LAYOUT_LINK_DATA *glink;

						add_buf(buffer, "{x               [    Mode    ]{x\n\r");
						add_buf(buffer, "{x          ========================================================{x\n\r");

						int glink_no = 1;
						iterator_start(&git, link->group);
						while( (glink = (BLUEPRINT_LAYOUT_LINK_DATA *)iterator_nextdata(&git)) )
						{
							switch(glink->mode)
							{
								case LINKMODE_STATIC:
								{
									from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->from, 1);
									to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->to, 1);
									sprintf(buf, "          %4d  {Y   STATIC   {x\n\r", glink_no++);
									sprintf(buf, "                    Source:        {%c%4d{x (%d)\n\r", (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
									add_buf(buffer, buf);
									sprintf(buf, "                    Destination:   {%c%4d{x (%d)\n\r", (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
									add_buf(buffer, buf);
									break;
								}

								case LINKMODE_SOURCE:
								{
									ITERATOR wit;
									to = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->to, 1);
									sprintf(buf, "          %4d  {C   SOURCE   {x\n\r", link_no++);
									add_buf(buffer, buf);

									int fromlink_no = 1;
									add_buf(buffer, "                    Source:\n\r");
									add_buf(buffer, "                         [ Weight ] [Section] [ Exit# ]{x\n\r");
									add_buf(buffer, "                    ===================================={x\n\r");
									iterator_start(&wit, glink->from);
									while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
									{
										sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
										add_buf(buffer, buf);
									}
									iterator_stop(&wit);
									add_buf(buffer, "                    ----------------------------------------------------{x\n\r");

									sprintf(buf, "                    Destination:   {%c%4d{x (%d)\n\r", (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
									add_buf(buffer, buf);
									break;
								}

								case LINKMODE_DESTINATION:
								{
									ITERATOR wit;
									from = (BLUEPRINT_WEIGHTED_LINK_DATA *)list_nthdata(glink->from, 1);
									sprintf(buf, "          %4d  {C DESTINATION{x\n\r", glink_no++);
									add_buf(buffer, buf);
									sprintf(buf, "                    Source:        {%c%4d{x (%d)\n\r", (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
									add_buf(buffer, buf);

									int tolink_no = 1;
									add_buf(buffer, "                    Destination:\n\r");
									add_buf(buffer, "                         [ Weight ] [Section] [ Exit# ]{x\n\r");
									add_buf(buffer, "                    ===================================={x\n\r");
									iterator_start(&wit, glink->to);
									while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
									{
										sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
										add_buf(buffer, buf);
									}
									iterator_stop(&wit);
									add_buf(buffer, "                    ----------------------------------------------------{x\n\r");
									break;
								}

								case LINKMODE_WEIGHTED:
								{
									ITERATOR wit;
									sprintf(buf, "          %4d  {C  WEIGHTED  {x\n\r", glink_no++);
									add_buf(buffer, buf);

									int fromlink_no = 1;
									add_buf(buffer, "                    Source:\n\r");
									add_buf(buffer, "                         [ Weight ] [Section] [ Exit# ]{x\n\r");
									add_buf(buffer, "                    ===================================={x\n\r");
									iterator_start(&wit, glink->from);
									while ( (from = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
									{
										sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", fromlink_no++, from->weight, (from->section < 0?'G':(from->section>0?'Y':'W')), abs(from->section), from->link);
										add_buf(buffer, buf);
									}
									iterator_stop(&wit);
									add_buf(buffer, "                    ----------------------------------------------------{x\n\r");

									int tolink_no = 1;
									add_buf(buffer, "                    Destination:\n\r");
									add_buf(buffer, "                         [ Weight ] [Section] [ Exit# ]{x\n\r");
									add_buf(buffer, "                    ===================================={x\n\r");
									iterator_start(&wit, glink->to);
									while ( (to = (BLUEPRINT_WEIGHTED_LINK_DATA *)iterator_nextdata(&wit)) )
									{
										sprintf(buf, "                    %4d   %6d    {%c%5d{x     %5d\n\r", tolink_no++, to->weight, (to->section < 0?'G':(to->section>0?'Y':'W')), abs(to->section), to->link);
										add_buf(buffer, buf);
									}
									iterator_stop(&wit);
									add_buf(buffer, "                    ----------------------------------------------------{x\n\r");
									break;
								}
							}
						}
						iterator_stop(&git);
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

		add_buf(buffer, "{YYELLOW{x - Generated section position index for Source/Destination sections\n\r");
		add_buf(buffer, "{GGREEN{x  - Ordinal section position index for Source/Destination sections\n\r");
	}
	else
	{
		add_buf(buffer, "  None\n\r");
	}
}

BLUEPRINT_LAYOUT_SECTION_DATA *blueprint_get_nth_section(BLUEPRINT *bp, int section_no, BLUEPRINT_LAYOUT_SECTION_DATA **in_group)
{
	if (!IS_VALID(bp) || !section_no) return NULL;

	if (in_group) *in_group = NULL;
	BLUEPRINT_LAYOUT_SECTION_DATA *section_data = NULL;
	if (section_no < 0)
	{
		int ordinal = -section_no;
		ITERATOR lsit;
		BLUEPRINT_LAYOUT_SECTION_DATA *ls;

		iterator_start(&lsit, bp->layout);
		while((ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&lsit)))
		{
			if (ls->ordinal == ordinal)
			{
				section_data = ls;
				break;
			}
			else if (ls->mode == SECTIONMODE_GROUP)
			{
				ITERATOR git;
				BLUEPRINT_LAYOUT_SECTION_DATA *gls;

				iterator_start(&git, ls->group);
				while((gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&git)))
				{
					if (gls->ordinal == ordinal)
					{
						section_data = gls;
						if (in_group) *in_group = ls;
						break;
					}
				}
				iterator_stop(&git);

				if (section_data)
					break;
			}
		}
		iterator_stop(&lsit);
	}
	else
	{
		ITERATOR lsit;
		BLUEPRINT_LAYOUT_SECTION_DATA *ls;

		iterator_start(&lsit, bp->layout);
		while((ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&lsit)))
		{
			if (ls->mode == SECTIONMODE_GROUP)
			{
				ITERATOR git;
				BLUEPRINT_LAYOUT_SECTION_DATA *gls;

				iterator_start(&git, ls->group);
				while((gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&git)))
				{
					if (section_no == 1)
					{
						section_data = gls;
						if (in_group) *in_group = ls;
						break;
					}

					section_no--;
				}
				iterator_stop(&git);

				if (section_data)
					break;
			}
			else if (section_no == 1)
			{
				section_data = ls;
				break;
			}
			else
				section_no--;
		}
		iterator_stop(&lsit);
	}

	return section_data;
}

BLUEPRINT_SECTION *blueprint_get_representative_section(BLUEPRINT *bp, int section_no, bool *exact)
{
	BLUEPRINT_LAYOUT_SECTION_DATA *in_group = NULL;
	BLUEPRINT_LAYOUT_SECTION_DATA *ls = blueprint_get_nth_section(bp, section_no, &in_group);

	if (!IS_VALID(ls) || ls->mode == SECTIONMODE_GROUP) return NULL;

	if (ls->mode == SECTIONMODE_STATIC)
	{
		// Can get the exact section
		if (exact) *exact = !in_group;
		return (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);
	}
	else
	{
		ITERATOR wit;
		BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
		BLUEPRINT_WEIGHTED_SECTION_DATA *most_likely = NULL;

		iterator_start(&wit, ls->weighted_sections);
		while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&wit)))
		{
			if(!most_likely || weighted->weight > most_likely->weight)
			{
				most_likely = weighted;
			}
		}
		iterator_stop(&wit);

		if (!most_likely) return NULL;

		if (exact) *exact = false;
		return (BLUEPRINT_SECTION *)list_nthdata(bp->sections, most_likely->section);
	}
}



void do_bpshow(CHAR_DATA *ch, char *argument)
{
	BLUEPRINT *bp;
	WNUM wnum;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  bpshow <widevnum>\n\r", ch);
		return;
	}

	if (!parse_widevnum(argument, ch->in_room->area, &wnum))
	{
		send_to_char("Please specify a widevnum.\n\r", ch);
		return;
	}

	if (!(bp = get_blueprint(wnum.pArea, wnum.vnum)))
	{
		send_to_char("That blueprint does not exist.\n\r", ch);
		return;
	}

	olc_show_item(ch, bp, bpedit_show, argument);
	return;
}




int blueprint_generation_count(BLUEPRINT *bp)
{
	int count = 0;
	ITERATOR it;
	BLUEPRINT_LAYOUT_SECTION_DATA *ls;

	iterator_start(&it, bp->layout);
	while((ls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)))
	{
		if (ls->mode == SECTIONMODE_GROUP)
			count += list_size(ls->group);
		else
			count++;
	}
	iterator_stop(&it);

	return count;
}

int blueprint_section_link_count(BLUEPRINT_SECTION *section)
{
	int count = 0;

	if (IS_VALID(section))
		for(BLUEPRINT_LINK *link = section->links; link; link = link->next)
			count++;

	return count;
}

int blueprint_weighted_link_count(BLUEPRINT *bp, BLUEPRINT_WEIGHTED_SECTION_DATA *weighted)
{
	if (weighted)
	{
		BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);

		return blueprint_section_link_count(bs);
	}

	return 0;
}

int blueprint_layout_links_count(BLUEPRINT *bp, BLUEPRINT_LAYOUT_SECTION_DATA *ls, int exclude)
{
	if (IS_VALID(ls))
	{
		switch(ls->mode)
		{
			case SECTIONMODE_STATIC:
			{
				BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);
				return blueprint_section_link_count(bs);
			}

			case SECTIONMODE_WEIGHTED:
			{
				int count = 0;

				// Find the first entry that *has* a link count, as most of them can be empty weighted entries.
				ITERATOR it;
				BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
				int index = 1;
				iterator_start(&it, ls->weighted_sections);
				while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&it)))
				{
					if (index != exclude)
					{
						int c = blueprint_weighted_link_count(bp, weighted);

						if (c > 0 && (count < 1 || c < count))
						{
							count = c;
							break;
						}
					}

					index++;
				}
				iterator_stop(&it);

				return count;
			}

			case SECTIONMODE_GROUP:
			{
				int count = 0;

				// Find the first entry that *has* a link count, as most of them can be empty weighted entries.
				ITERATOR it;
				BLUEPRINT_LAYOUT_SECTION_DATA *gls;
				int index = 1;
				iterator_start(&it, ls->group);
				while((gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)))
				{
					if (index != exclude)
					{
						int c = blueprint_layout_links_count(bp, gls, 0);

						if (c > 0 && (count < 1 || c < count))
						{
							count = c;
						}
					}

					index++;
				}
				iterator_stop(&it);

				return count;
			}
		}
	}
	return 0;
}

void blueprint_add_weighted_link(LLIST *list, int weight, int section_no, int link_no)
{
	BLUEPRINT_WEIGHTED_LINK_DATA *wl = new_weighted_random_link();
	wl->weight = weight;
	wl->section = section_no;
	wl->link = link_no;
	list_appendlink(list, wl);
}



bool blueprint_layout_has_recall(BLUEPRINT *bp, BLUEPRINT_LAYOUT_SECTION_DATA *ls)
{
	BLUEPRINT_SECTION *bs;
	switch(ls->mode)
	{
		case SECTIONMODE_STATIC:
			bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);

			return IS_VALID(bs) && (bs->recall > 0);

		case SECTIONMODE_WEIGHTED:
		{
			bool valid = true;
			ITERATOR it;
			BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
			iterator_start(&it, ls->weighted_sections);
			while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&it)))
			{
				bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);

				if (!IS_VALID(bs) || bs->recall < 1)
				{
					valid = false;
					break;
				}
			}
			iterator_stop(&it);

			return valid;
		}

		case SECTIONMODE_GROUP:
		{
			bool valid = true;
			ITERATOR it;
			BLUEPRINT_LAYOUT_SECTION_DATA *gls;
			iterator_start(&it, ls->group);
			while((gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)))
			{
				if (!blueprint_layout_has_recall(bp, gls))
				{
					valid = false;
					break;
				}
			}
			iterator_stop(&it);

			return valid;
		}
	}

	return false;
}



int blueprint_section_room_count(BLUEPRINT_SECTION *bs)
{
	if (bs->type == BSTYPE_STATIC)
	{
		if (bs->upper_vnum < 1 || bs->lower_vnum < 1) return 0;

		return bs->upper_vnum - bs->lower_vnum + 1;
	}
	else if (bs->type == BSTYPE_MAZE)
	{
		return bs->maze_x * bs->maze_y;
	}
	else
		return 0;
}

int blueprint_layout_room_count(BLUEPRINT *bp, BLUEPRINT_LAYOUT_SECTION_DATA *ls)
{
	switch(ls->mode)
	{
		case SECTIONMODE_STATIC:
		{
			BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, ls->section);

			return blueprint_section_room_count(bs);
		}

		case SECTIONMODE_WEIGHTED:
		{
			int min_rooms = -1;
			ITERATOR it;
			BLUEPRINT_WEIGHTED_SECTION_DATA *weighted;
			iterator_start(&it, ls->weighted_sections);
			while((weighted = (BLUEPRINT_WEIGHTED_SECTION_DATA *)iterator_nextdata(&it)))
			{
				BLUEPRINT_SECTION *bs = (BLUEPRINT_SECTION *)list_nthdata(bp->sections, weighted->section);

				int rooms = blueprint_section_room_count(bs);

				if (min_rooms < 0 || rooms < min_rooms)
				{
					min_rooms = rooms;
				}
			}
			iterator_stop(&it);

			return UMAX(min_rooms, 0);
		}

		case SECTIONMODE_GROUP:
		{
			int min_rooms = -1;
			ITERATOR it;
			BLUEPRINT_LAYOUT_SECTION_DATA *gls;
			iterator_start(&it, ls->group);
			while((gls = (BLUEPRINT_LAYOUT_SECTION_DATA *)iterator_nextdata(&it)))
			{
				int rooms = blueprint_layout_room_count(bp, gls);

				if (min_rooms < 0 || rooms < min_rooms)
				{
					min_rooms = rooms;
				}
			}
			iterator_stop(&it);

			return UMAX(min_rooms, 0);
		}
	}

	return 0;
}




//////////////////////////////////////////////////////////////
//
// Immortal Commands
//


void do_instance(CHAR_DATA *ch, char *argument)
{
	char arg1[MIL];

	if( argument[0] == '\0' )
	{
		send_to_char("Syntax:  instance list\n\r", ch);
		send_to_char("         instance unload\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);

	if( !str_prefix(arg1, "list") )
	{
		if(!ch->lines)
			send_to_char("{RWARNING:{W Having scrolling off may limit how many instances you can see.{x\n\r", ch);

		int lines = 0;
		bool error = false;
		BUFFER *buffer = new_buf();
		char buf[MSL];


		ITERATOR it;
		INSTANCE *instance;

		iterator_start(&it, loaded_instances);
		while((instance = (INSTANCE *)iterator_nextdata(&it)))
		{
			++lines;

			char *owner = instance_get_ownership(instance);

			char color = 'G';

			if( IS_SET(instance->flags, INSTANCE_DESTROY) )
				color = 'D';
			else if( IS_SET(instance->flags, INSTANCE_FAILED) )
				color = 'R';
			else if( IS_SET(instance->flags, INSTANCE_COMPLETED) )
				color = 'W';

			sprintf(buf, "%4d {Y[{W%5ld{Y] {%c%-30.30s{x  %s{x\n\r",
				lines,
				instance->blueprint->vnum,
				color,
				instance->blueprint->name,
				owner);

			if( !add_buf(buffer, buf) || (!ch->lines && strlen(buf_string(buffer)) > MAX_STRING_LENGTH) )
			{
				error = true;
				break;
			}
		}
		iterator_stop(&it);

		if( error )
		{
			send_to_char("Too many instances to list.  Please shorten!\n\r", ch);
		}
		else
		{
			if( !lines )
			{
				add_buf( buffer, "No instances to display.\n\r" );
			}
			else
			{
				// Header
				send_to_char("{Y      Vnum   [            Name            ] [      Owner     ]{x\n\r", ch);
				send_to_char("{Y==============================================================={x\n\r", ch);
			}

			page_to_char(buffer->string, ch);
		}
		free_buf(buffer);

		return;
	}


	/*
	if( !str_prefix(arg1, "load") )
	{
		if( !can_edit_blueprints(ch) )
		{
			send_to_char("Insufficient access to load blueprints.\n\r", ch);
			return;
		}

		if( list_size(loaded_instances) > 0 )
		{
			send_to_char("TEMPORARY: There is already an instance loaded.\n\r", ch);
			return;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return;
		}

		BLUEPRINT *bp = get_blueprint(atol(argument));

		if( !bp )
		{
			send_to_char("That blueprint does not exist.\n\r", ch);
			return;
		}

		INSTANCE *instance = create_instance(bp);

		if( !instance )
		{
			send_to_char("{WERROR SPAWNING INSTANCE!{x\n\r", ch);
			return;
		}

		list_appendlink(loaded_instances, instance);


		// Get the entry point
		ROOM_INDEX_DATA *room = instance->entrance;

		if( !room )
		{
			// Fallback to the recall if the entry is not defined
			room = instance->recall;
		}

		if( !room )
		{
			send_to_char("{WERROR GETTING ENTRY ROOM IN INSTANCE!{x\n\r", ch);
			return;
		}

		char_from_room(ch);
		char_to_room(ch, room);
		do_function (ch, &do_look, "");

		send_to_char("{YInstance loaded.{x\n\r", ch);
		return;
	}
	*/

	if( !str_prefix(arg1, "unload") )
	{
		if( !can_edit_blueprints(ch) )
		{
			send_to_char("Insufficient access to unload blueprints.\n\r", ch);
			return;
		}

		if( !is_number(argument) )
		{
			send_to_char("That is not a number.\n\r", ch);
			return;
		}

		int index = atoi(argument);

		if( list_size(loaded_instances) < index )
		{
			send_to_char("There is no instance loaded.\n\r", ch);
			return;
		}


		INSTANCE *instance = (INSTANCE *)list_nthdata(loaded_instances, index);

		if( !instance_isorphaned(instance) )
		{
			send_to_char("Instance is not orphaned.\n\r", ch);
			return;
		}

		if( list_size(instance->players) > 0 )
		{
			char buf[MSL];
			if( IS_SET(instance->flags, INSTANCE_DESTROY) )
			{
				send_to_char("Instance is already flagged for unloading.\n\r", ch);
				return;
			}

			SET_BIT(instance->flags, INSTANCE_DESTROY);
			if( instance->idle_timer > 0 )
				instance->idle_timer = UMIN(INSTANCE_DESTROY_TIMEOUT, instance->idle_timer);
			else
				instance->idle_timer = INSTANCE_DESTROY_TIMEOUT;

			sprintf(buf, "{RWARNING: Instance is being forcibly unloaded.  You have %d minutes to escape before the end!{x\n\r", instance->idle_timer);
			instance_echo(instance, buf);

			send_to_char("Instance flagged for unloading.\n\r", ch);
		}
		else
		{
			extract_instance(instance);
			send_to_char("Instance unloaded.\n\r", ch);
		}
		return;
	}

	do_instance(ch, "");
	return;
}

////////////////////////////////////////////////////////
//
// Instance Save/Load
//

void instance_section_save(FILE *fp, INSTANCE_SECTION *section)
{
	fprintf(fp, "#SECTION %ld#%ld\n\r", section->section->area->uid, section->section->vnum);

	ITERATOR it;
	ROOM_INDEX_DATA *room;
	iterator_start(&it, section->rooms);
	while( (room = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) )
	{
		persist_save_room(fp, room);
	}

	iterator_stop(&it);

	fprintf(fp, "#-SECTION\n\r");
}

void instance_save_roominfo(FILE *fp, char *field, ROOM_INDEX_DATA *room)
{
	if( room )
	{
		fprintf(fp, "%s %ld %lu %lu\n\r", field, room->vnum, room->id[0], room->id[1]);
	}
}

void instance_save(FILE *fp, INSTANCE *instance)
{
	fprintf(fp, "#INSTANCE %ld#%ld\n\r", instance->blueprint->area->uid, instance->blueprint->vnum);

	fprintf(fp, "Uid %lu %lu\n\r", instance->uid[0], instance->uid[1]);

	fprintf(fp, "Floor %d\n\r", instance->floor);
	fprintf(fp, "Flags %d\n\r", instance->flags);

	if( instance->object_uid[0] > 0 || instance->object_uid[1] > 0 )
	{
		fprintf(fp, "Object %lu %lu\n\r", instance->object_uid[0], instance->object_uid[1]);
	}

	ITERATOR it;
	LLIST_UID_DATA *luid;
	iterator_start(&it, instance->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		fprintf(fp, "Player %lu %lu\n\r", luid->id[0], luid->id[1]);
	}
	iterator_stop(&it);

	INSTANCE_SECTION *section;
	iterator_start(&it, instance->sections);
	while( (section = (INSTANCE_SECTION *)iterator_nextdata(&it)) )
	{
		instance_section_save(fp, section);
	}
	iterator_stop(&it);

	instance_save_roominfo(fp, "Recall", instance->recall);
	instance_save_roominfo(fp, "Entrance", instance->entrance);
	instance_save_roominfo(fp, "Exit", instance->exit);

	fprintf(fp, "#-INSTANCE\n\r");
}

void instance_section_tallyentities(INSTANCE_SECTION *section)
{
	ITERATOR it;
	ROOM_INDEX_DATA *room;

	iterator_start(&it, section->rooms);
	while( (room = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) )
	{
		for(OBJ_DATA *obj = room->contents; obj; obj = obj->next_content)
		{
			if(!IS_SET(obj->extra[2], ITEM_INSTANCE_OBJ))
			{
				list_appendlink(section->instance->objects, obj);
			}
		}

		for(CHAR_DATA *ch = room->people; ch; ch = ch->next_in_room)
		{
			if( IS_NPC(ch) )
			{
				if( !IS_SET(ch->act[1], ACT2_INSTANCE_MOB) )
					list_appendlink(section->instance->mobiles, ch);
				else if ( IS_BOSS(ch) )
					list_appendlink(section->instance->bosses, ch);
			}
		}

		list_appendlink(section->instance->rooms, room);
	}
	iterator_stop(&it);
}

INSTANCE_SECTION *instance_section_load(FILE *fp)
{
	char *word;
	bool fMatch;
	bool fError = false;

	INSTANCE_SECTION *section = new_instance_section();
	WNUM_LOAD wnum_load = fread_widevnum(fp, 0);

	section->section = get_blueprint_section_auid(wnum_load.auid, wnum_load.vnum);

	while (str_cmp((word = fread_word(fp)), "#-SECTION"))
	{
		fMatch = false;

		switch(word[0])
		{
		case '#':
			if( !str_cmp(word, "#CROOM") )
			{
				ROOM_INDEX_DATA *room = persist_load_room(fp, 'C');
				if(room)
				{
					room->instance_section = section;

					variable_dynamic_fix_clone_room(room);
					list_appendlink(section->rooms, room);
				}
				else
					fError = true;

				fMatch = true;
				break;
			}
			break;
		}

		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "instance_section_load: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	if( fError )
	{
		free_instance_section(section);
		return NULL;
	}

	return section;
}

INSTANCE *instance_load(FILE *fp)
{
	char *word;
	bool fMatch;
	bool fError = false;

	INSTANCE *instance = new_instance();
	WNUM_LOAD wnum_load = fread_widevnum(fp, 0);
	AREA_DATA *area = get_area_from_uid(wnum_load.auid);
	long vnum = wnum_load.vnum;

	instance->blueprint = get_blueprint(area, vnum);

	instance->progs			= new_prog_data();
	instance->progs->progs	= instance->blueprint->progs;
	variable_copylist(&instance->blueprint->index_vars,&instance->progs->vars,false);

	while (str_cmp((word = fread_word(fp)), "#-INSTANCE"))
	{
		fMatch = false;

		switch(word[0])
		{
		case '#':
			if( !str_cmp(word, "#SECTION") )
			{
				INSTANCE_SECTION *section = instance_section_load(fp);

				if( section )
				{
					section->instance = instance;
					section->blueprint = instance->blueprint;

					list_appendlink(instance->sections, section);

					instance_section_tallyentities(section);
				}
				else
					fError = true;

				fMatch = true;
				break;
			}
			break;

		case 'E':
			if( !str_cmp(word, "Entrance") )
			{
				long room_vnum = fread_number(fp);
				unsigned long id1 = fread_number(fp);
				unsigned long id2 = fread_number(fp);

				//log_string("get_clone_room: instance->entrance");
				instance->entrance = get_clone_room(get_room_index(area, room_vnum), id1, id2);

				fMatch = true;
				break;
			}

			if( !str_cmp(word, "Exit") )
			{
				long room_vnum = fread_number(fp);
				unsigned long id1 = fread_number(fp);
				unsigned long id2 = fread_number(fp);

				//log_string("get_clone_room: instance->exit");
				instance->exit = get_clone_room(get_room_index(area, room_vnum), id1, id2);

				fMatch = true;
				break;
			}

			break;

		case 'F':
			KEY("Flags", instance->flags, fread_number(fp));
			KEY("Floor", instance->floor, fread_number(fp));
			break;

		case 'O':
			if( !str_cmp(word, "Object") )
			{
				instance->object_uid[0] = fread_number(fp);
				instance->object_uid[1] = fread_number(fp);

				fMatch = true;
				break;
			}
			break;

		case 'P':
			if( !str_cmp(word, "Player") )
			{
				unsigned long uid[2];
				uid[0] = fread_number(fp);
				uid[1] = fread_number(fp);

				instance_addowner_playerid(instance, uid[0], uid[1]);

				fMatch = true;
				break;
			}
			break;

		case 'R':
			if( !str_cmp(word, "Recall") )
			{
				long room_vnum = fread_number(fp);
				unsigned long id1 = fread_number(fp);
				unsigned long id2 = fread_number(fp);

				//log_string("get_clone_room: instance->recall");
				instance->recall = get_clone_room(get_room_index(area, room_vnum), id1, id2);

				fMatch = true;
				break;
			}
			break;
		case 'U':
			if (!str_cmp(word, "Uid"))
			{
				instance->uid[0] = fread_number(fp);
				instance->uid[1] = fread_number(fp);
				fMatch = true;
				break;
			}
			break;
		}

		if (!fMatch) {
			char buf[MSL];
			snprintf(buf, sizeof(buf), "instance_load: no match for word %.50s", word);
			bug(buf, 0);
		}
	}

	if( fError )
	{
		free_instance(instance);
		return NULL;
	}

	if( instance->object_uid[0] > 0 && instance->object_uid[1] > 0 )
	{
		OBJ_DATA *obj = idfind_object(instance->object_uid[0], instance->object_uid[1]);

		if( IS_VALID(obj) )
			instance->object = obj;
	}

	get_instance_id(instance);
	return instance;
}


INSTANCE *find_instance(unsigned long id0, unsigned long id1)
{
	ITERATOR it;
	INSTANCE *instance = NULL;

	iterator_start(&it, loaded_instances);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		if( instance->uid[0] == id0 && instance->uid[1] == id1 )
			break;
	}
	iterator_stop(&it);
	
	return instance;
}


void resolve_instances()
{
	ITERATOR it;
	INSTANCE *instance;

	iterator_start(&it, loaded_instances);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		if( instance->object_uid[0] > 0 && instance->object_uid[1] > 0 )
		{
			OBJ_DATA *obj = idfind_object(instance->object_uid[0], instance->object_uid[1]);

			if( IS_VALID(obj) )
				instance->object = obj;
		}
	}
	iterator_stop(&it);
}

void resolve_instance_player(INSTANCE *instance, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, instance->player_owners);
	while( (luid = (LLIST_UID_DATA *)iterator_nextdata(&it)) )
	{
		if( luid->id[0] == ch->id[0] &&
			luid->id[1] == ch->id[1])
		{
			luid->ptr = ch;
			continue;
		}
	}
	iterator_stop(&it);
}

void resolve_instances_player(CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	INSTANCE *instance;

	iterator_start(&it, loaded_instances);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		resolve_instance_player(instance, ch);

		// Iterate over the character's quest list
	}
	iterator_stop(&it);
}


void detach_instances_player(CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	INSTANCE *instance;

	iterator_start(&it, loaded_instances);
	while( (instance = (INSTANCE *)iterator_nextdata(&it)) )
	{
		instance_removeowner_player(instance, ch);

		// Iterate over the character's quest list
	}
	iterator_stop(&it);
}

void instance_echo(INSTANCE *instance, char *text)
{
	if( !IS_VALID(instance) || IS_NULLSTR(text) ) return;

	ITERATOR it;
	CHAR_DATA *ch;

	iterator_start(&it, instance->players);
	while( (ch = (CHAR_DATA *)iterator_nextdata(&it)) )
	{
		send_to_char(text, ch);
		send_to_char("\n\r", ch);
	}
	iterator_stop(&it);
}

ROOM_INDEX_DATA *section_random_room(CHAR_DATA *ch, INSTANCE_SECTION *section)
{
	if( !IS_VALID(section) ) return NULL;

	return get_random_room_list_byflags( ch, section->rooms,
		(ROOM_PRIVATE | ROOM_SOLITARY | ROOM_DEATH_TRAP | ROOM_CHAOTIC),
		ROOM_NO_GET_RANDOM );
}

ROOM_INDEX_DATA *instance_random_room(CHAR_DATA *ch, INSTANCE *instance)
{
	if( !IS_VALID(instance) ) return NULL;

	return get_random_room_list_byflags( ch, instance->rooms,
		(ROOM_PRIVATE | ROOM_SOLITARY | ROOM_DEATH_TRAP | ROOM_CHAOTIC),
		ROOM_NO_GET_RANDOM );
}

ROOM_INDEX_DATA *get_instance_special_room(INSTANCE *instance, int index)
{
	if( !IS_VALID(instance) || index < 1) return NULL;

	NAMED_SPECIAL_ROOM *special = list_nthdata(instance->special_rooms, index);

	if( IS_VALID(special) )
		return special->room;

	return NULL;
}

ROOM_INDEX_DATA *get_instance_special_room_byname(INSTANCE *instance, char *name)
{
	int number;
	char arg[MSL];

	if( !IS_VALID(instance) ) return NULL;

	number = number_argument(name, arg);

	if( number < 1 ) return NULL;

	ITERATOR it;
	ROOM_INDEX_DATA *room = NULL;
	NAMED_SPECIAL_ROOM *special;
	iterator_start(&it, instance->special_rooms);
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


void instance_addowner_player(INSTANCE *instance, CHAR_DATA *ch)
{
	// Don't add twice
	if( instance_isowner_player(instance, ch) ) return;

	LLIST_UID_DATA *luid = new_list_uid_data();
	luid->id[0] = ch->id[0];
	luid->id[1] = ch->id[1];
	luid->ptr = ch;

	list_appendlink(instance->player_owners, luid);
}

void instance_addowner_playerid(INSTANCE *instance, unsigned long id1, unsigned long id2)
{
	// Don't add twice
	if( instance_isowner_playerid(instance, id1, id2) ) return;

	LLIST_UID_DATA *luid = new_list_uid_data();
	luid->id[0] = id1;
	luid->id[1] = id2;
	luid->ptr = NULL;

	list_appendlink(instance->player_owners, luid);
}

void instance_removeowner_player(INSTANCE *instance, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return;

	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, instance->player_owners);
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

void instance_removeowner_playerid(INSTANCE *instance, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	LLIST_UID_DATA *luid;

	iterator_start(&it, instance->player_owners);
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

bool instance_isowner_player(INSTANCE *instance, CHAR_DATA *ch)
{
	if( IS_NPC(ch) ) return false;

	ITERATOR it;
	LLIST_UID_DATA *luid;
	bool ret = false;

	iterator_start(&it, instance->player_owners);
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

bool instance_isowner_playerid(INSTANCE *instance, unsigned long id1, unsigned long id2)
{
	ITERATOR it;
	LLIST_UID_DATA *luid;
	bool ret = false;

	iterator_start(&it, instance->player_owners);
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

bool instance_canswitch_player(INSTANCE *instance, CHAR_DATA *ch)
{
	// TODO: Add lockout system
	return true;
}

bool instance_isorphaned(INSTANCE *instance)
{
	if( !IS_VALID(instance) ) return true;

	if( IS_VALID(instance->dungeon) ) return false;

	if( IS_VALID(instance->ship) ) return false;

	// Does it have player owners?
	if( list_size(instance->player_owners) > 0 ) return false;

	// Does it have an object owner?
	if( IS_VALID(instance->object) ||
		instance->object_uid[0] > 0 ||
		instance->object_uid[1] > 0)
		return false;

	// Does it have a quest owner?
//  if( IS_VALID(instance->quest) ) return false;

	// Add other ownership

	return true;
}

char *instance_get_ownership(INSTANCE *instance)
{
	static char buf[4][MSL+1];
	static int idx = 0;

	if (++idx > 3)
		idx = 0;

	char *p = buf[idx];
	if( !IS_VALID(instance) )
	{
		strncpy(p, "{R    ! {WERROR {R!   {x", MSL);
	}
	else if( IS_VALID(instance->dungeon) )
	{
		strncpy(p, "{R     D{rU{RN{rG{RE{rO{RN    {x", MSL);
	}
	else if( IS_VALID(instance->ship) )
	{
		strncpy(p, "{C      S{cH{CI{cP      {x", MSL);
	}
	else if( IS_VALID(instance->object) || instance->object_uid[0] > 0 || instance->object_uid[1] > 0)
	{
		strncpy(p, "{Y     OBJECT     {x", MSL);
	}
//	else if( IS_VALID(instance->quest) )
//	{
//		strncpy(p, "{C      QUEST     {x", MSL);
//	}
	else if( list_size(instance->player_owners) > 0 )
	{
		strncpy(p, "{G     PLAYER     {x", MSL);
	}
	else
	{
		strncpy(p, "{D   - {WORPHAN{D -   {x", MSL);
	}


	p[MSL] = '\0';
	return p;
}


INSTANCE *get_room_instance(ROOM_INDEX_DATA *room)
{
	if( !room ) return NULL;

	if( !IS_VALID(room->instance_section) ) return NULL;

	if( !IS_VALID(room->instance_section->instance) ) return NULL;

	return room->instance_section->instance;
}
