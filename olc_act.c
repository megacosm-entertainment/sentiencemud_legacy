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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "strings.h"
#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "interp.h"
#include "scripts.h"
#include "wilds.h"

extern GLOBAL_DATA         gconfig;
/* Return true if area changed, false if not. */
AREA_DATA *get_area_data args ((long anum));
AREA_DATA *get_area_from_uid args ((long uid));

void obj_index_reset_multitype(OBJ_INDEX_DATA *pObjIndex);
void obj_index_set_primarytype(OBJ_INDEX_DATA *pObjIndex, int item_type);

void insert_liquid(LIQUID *liquid);
void save_liquids();

MATERIAL **gm_from_name(char *name);
char *gm_to_name(MATERIAL **material);
void insert_material(MATERIAL *material);
void save_materials();

bool redit_blueprint_oncreate = false;

char *get_spell_data_name(SPELL_DATA *spell);

struct olc_help_type
{
    char *command;
	int structure_type;
    const void *structure;
    char *desc;
};

#define STRUCT_FLAGS	0
#define STRUCT_FLAGBANK	1
#define STRUCT_TRIGGERS	2
#define STRUCT_SPEC		3
#define STRUCT_LIQUID	4
#define STRUCT_ATTACK	5
#define STRUCT_MATERIAL	6
#define STRUCT_SKILL	7
#define STRUCT_SPELLFUNC	8
#define STRUCT_GSN		9
#define STRUCT_CLASSES	10
#define STRUCT_PREBREWFUNC		13
#define STRUCT_BREWFUNC			14
#define STRUCT_QUAFFFUNC		15
#define STRUCT_PRESCRIBEFUNC	16
#define STRUCT_SCRIBEFUNC		17
#define STRUCT_RECITEFUNC		18
#define STRUCT_PREINKFUNC		19
#define STRUCT_INKFUNC			20
#define STRUCT_TOUCHFUNC		21
#define STRUCT_PREIMBUEFUNC		22
#define STRUCT_IMBUEFUNC		23
#define STRUCT_BRANDISHFUNC		24
#define STRUCT_EQUIPFUNC		25
#define STRUCT_ZAPFUNC			26

#define STRUCT_ARTIFICING		27
#define STRUCT_GCL				28
#define STRUCT_GR				29
#define STRUCT_GSCT				30
#define STRUCT_DOFUNC			31

struct trigger_type dummy_triggers[1];


// This table contains help commands and a brief description of each.
const struct olc_help_type help_table[] =
{
	{	"ac",					STRUCT_FLAGS,		ac_type,					"Ac for different attacks."	},
	{	"act",					STRUCT_FLAGBANK,	act_flagbank,				"Mobile	attributes."	},
	{	"adornments", 			STRUCT_FLAGS,		adornment_types,			"Adornment Types."	},
	{	"affect",				STRUCT_FLAGBANK,	affect_flagbank,			"Mobile	affects."	},
	{	"ammo",					STRUCT_FLAGS,		ammo_types,					"Ammo types."	},
	{	"apply",				STRUCT_FLAGS,		apply_flags,				"Apply flags"	},
	{	"apptype",				STRUCT_FLAGS,		apply_types,				"Apply types."	},
	{	"aprog",				STRUCT_TRIGGERS,	dummy_triggers,				"AreaProgram types."	},
	{	"area",					STRUCT_FLAGS,		area_flags,					"Area attributes."	},
	{	"arearegion",			STRUCT_FLAGS,		area_region_flags,			"Area Region attributes."	},
	{	"areawho",				STRUCT_FLAGS,		area_who_titles,			"Type of area for who."	},
	{	"armour",				STRUCT_FLAGS,		armour_types,				"Types of Armor"	},
	{	"armourstrength",		STRUCT_FLAGS,		armour_strength_table,		"Armor strength types"	},
	{	"blueprint",			STRUCT_FLAGS,		blueprint_flags,			"Blueprint flags" },
	{	"book",					STRUCT_FLAGS,		book_flags,					"Book flags."	},
	{	"brandish_func",		STRUCT_ARTIFICING,	brandish_func_table,		"Brandish Functions."	},
	{	"brew_func",			STRUCT_ARTIFICING,	brew_func_table,			"Brew Functions (SkEdit)"},
	{	"cart",					STRUCT_FLAGS,		cart_flags,					"Cart flags."},
	{	"catalyst",				STRUCT_FLAGS,		catalyst_types,				"Catalyst types."	},
	{	"class",				STRUCT_FLAGS,		class_flags,				"Class flags." },
	{	"classes",				STRUCT_CLASSES,		NULL,						"Classes" },
	{	"classtypes",			STRUCT_FLAGS,		class_types,				"Class Types"},
	{	"cmd",					STRUCT_FLAGS,		command_flags,				"Command Flags (CMDEdit)"},
	{ 	"compartment",			STRUCT_FLAGS,		compartment_flags,			"Compartment Flags."},
	{	"condition",			STRUCT_FLAGS,		room_condition_flags,		"Room Condition types."	},
	{	"container",			STRUCT_FLAGS,		container_flags,			"Container status."	},
	{	"corpse",				STRUCT_FLAGS,		corpse_object_flags,		"Corpse flags." },
	{	"corpsetypes",			STRUCT_FLAGS,		corpse_types,				"Corpse types."	},
	{	"damageclass",			STRUCT_FLAGS,		damage_classes,				"Types of damages."},
	{	"dprog",				STRUCT_TRIGGERS,	dummy_triggers,				"DungeonProgram types."	},
	{	"death_release",		STRUCT_FLAGS,		death_release_modes,		"Dungeon Death Release modes."},
	{	"do_func",				STRUCT_DOFUNC,		do_func_table,				"Do_ functions (CMDEdit)"},
	{	"dungeon",				STRUCT_FLAGS,		dungeon_flags,				"Dungeon Flags"	},
	{	"equip_func",			STRUCT_ARTIFICING,	equip_func_table,			"Equip Functions (SkEdit)"},
	{	"exit",					STRUCT_FLAGS,		exit_flags,					"Exit types."	},
	{	"extra",				STRUCT_FLAGBANK,	extra_flagbank,				"Object attributes."	},
	{	"fluid_con",			STRUCT_FLAGS,		fluid_con_flags,			"Fluid Container status."	},
	{   "foodbuffs",			STRUCT_FLAGS,		food_buff_types,			"Food Buff types" },
	{	"form",					STRUCT_FLAGS,		form_flags,					"Mobile body form."	},
	{	"furniture",			STRUCT_FLAGS,		furniture_flags,			"Furniture flags."	},
	{	"furnitureaction",		STRUCT_FLAGS,		furniture_action_flags,		"Furniture action flags."	},
	{	"gcl",					STRUCT_GCL,			NULL,						"Global classes"},
	{	"gsct",					STRUCT_GSCT,		NULL,						"Global Sectors"},
	{	"gsn",					STRUCT_GSN,			NULL,						"Global Skill Numbers."},
	{	"gr",					STRUCT_GR,			NULL,						"Global Races" },
	{	"imbue_func",			STRUCT_ARTIFICING,	imbue_func_table,			"Imbue Functions."	},
	{	"imm",					STRUCT_FLAGS,		imm_flags,					"Mobile immunity."	},
	{	"immortalflags",		STRUCT_FLAGS,		immortal_flags,				"Immortal duties."	},
	{	"ink_func",				STRUCT_ARTIFICING,	ink_func_table,				"Ink Functions."	},
	{	"instance",				STRUCT_FLAGS,		instance_flags,				"Instance Flags"	},
	{	"instrument_flags",		STRUCT_FLAGS,		instrument_flags,			"Instrument Flags"	},
	{	"instrument_types",		STRUCT_FLAGS,		instrument_types,			"Instrument Types"	},
	{	"iprog",				STRUCT_TRIGGERS,	dummy_triggers,				"InstanceProgram types."	},
	{	"light",				STRUCT_FLAGS,		light_flags,				"Light flags."	},
	{	"liquid",				STRUCT_LIQUID,		liq_table,					"Liquid types."	},
	{   "log",					STRUCT_FLAGS,		log_flags,					"Log levels (CMDEdit)"},
	{	"lock",					STRUCT_FLAGS,		lock_flags,					"Lock state types."	},
	{	"matclass",				STRUCT_FLAGS,		material_classes,			"Material classes" },
	{	"material",				STRUCT_FLAGS,		material_flags,				"Material flags."	},
	{	"materials",			STRUCT_MATERIAL,	NULL,						"Object materials."	},
	{	"mprog",				STRUCT_TRIGGERS,	dummy_triggers,				"MobProgram types."	},
	{	"off",					STRUCT_FLAGS,		off_flags,					"Mobile offensive behaviour."	},
	{	"oprog",				STRUCT_TRIGGERS,	dummy_triggers,				"ObjProgram types."	},
	{	"part",					STRUCT_FLAGS,		part_flags,					"Mobile body parts."	},
	{	"placetype",			STRUCT_FLAGS,		place_flags,				"Where is the town/city etc."	},
	{	"portal",				STRUCT_FLAGS,		portal_flags,				"Portal types."	},
	{	"portal_exit",			STRUCT_FLAGS,		portal_exit_flags,			"Exit (Portal) types."	},
	{	"portal_type",			STRUCT_FLAGS,		portal_gatetype,			"Portal gate types"},
	{	"position",				STRUCT_FLAGS,		position_flags,				"Mobile positions."	},
	{	"practice_entry",		STRUCT_FLAGS,		practice_entry_flags,		"Practice Entry attributes."},
	{	"prebrew_func",			STRUCT_ARTIFICING,	prebrew_func_table,			"PreBrew Functions (SkEdit)"},
	{	"preimbue_func",		STRUCT_ARTIFICING,	preimbue_func_table,		"PreImbue Functions (SkEdit)"},
	{	"preink_func",			STRUCT_ARTIFICING,	preink_func_table,			"PreInk Functions (SkEdit)"},
	{	"prescribe_func",		STRUCT_ARTIFICING,	prescribe_func_table,		"PreScribe Functions (SkEdit)"},
	{	"presong_func",			STRUCT_ARTIFICING,	presong_func_table,			"Presong Functions (SongEdit)"},
	{	"prespell_func",		STRUCT_SPELLFUNC,	prespell_func_table,		"Prespell functions (SkEdit)"},
	{	"projectflags",			STRUCT_FLAGS,		project_flags,				"Project flags."	},
	{	"protections",			STRUCT_FLAGS,		armour_protection_types,	"Armor protection types."	},
	{	"quaff_func",			STRUCT_ARTIFICING,	quaff_func_table,			"Quaff Functions (SkEdit)"},
	//{	"race",					STRUCT_FLAGS,		race_flags,					"Race flags." },
	{	"ranged",				STRUCT_FLAGS,		ranged_weapon_class,		"Ranged	weapon types."	},
	{	"recite_func",			STRUCT_ARTIFICING,	recite_func_table,			"Recite Functions (SkEdit)"},
	{	"reputation",			STRUCT_FLAGS,		reputation_flags,			"Reputation flags." },
	{	"reputation_rank",		STRUCT_FLAGS,		reputation_rank_flags,		"Reputation Rank flags." },
	{	"res",					STRUCT_FLAGS,		res_flags,					"Mobile resistance."	},
	{	"room",					STRUCT_FLAGBANK,	room_flagbank,				"Room attributes."	},
	{	"rprog",				STRUCT_TRIGGERS,	dummy_triggers,				"RoomProgram types."	},
	{	"scribe_func",			STRUCT_ARTIFICING,	scribe_func_table,			"Scribe Functions (SkEdit)"},
	{	"scriptflags",			STRUCT_FLAGS,		script_flags,				"Script Flags {D({Wrestricted{D){x."	},
	{   "script_spaces",		STRUCT_FLAGS,		script_spaces,				"Script spaces"	},
	{	"scroll",				STRUCT_FLAGS,		scroll_flags,				"Scroll flags."	},
	{	"section_flags",		STRUCT_FLAGS,		blueprint_section_flags,	"Blueprint Section Flags"	},
	{	"section_type",			STRUCT_FLAGS,		blueprint_section_types,	"Blueprint Section Types"	},
	{	"sector",				STRUCT_FLAGS,		sector_flags,				"Sector flags."	},
	{	"sectorclass",			STRUCT_FLAGS,		sector_classes,				"Sector Classes." },
//	{	"sectortypes",			STRUCT_FLAGS,		sector_types,				"Sector types." },
	{	"sex",					STRUCT_FLAGS,		sex_flags,					"Sexes."	},
	{	"ship",					STRUCT_FLAGS,		ship_flags,					"Ship flags"	},
	{	"shipclass",			STRUCT_FLAGS,		ship_class_types,			"Ship class types"	},
	{	"shop",					STRUCT_FLAGS,		shop_flags,					"Shop flags"	},
	{	"size",					STRUCT_FLAGS,		size_flags,					"Mobile size."	},
	{	"skill",				STRUCT_FLAGS,		skill_flags,				"Skill flags."	},
	{	"skillentry",			STRUCT_FLAGS,		skill_entry_flags,			"Skill entry flags."	},
	{	"song_func",			STRUCT_ARTIFICING,	song_func_table,			"Song Functions (SongEdit)"},
	{	"song_targets",			STRUCT_FLAGS,		song_target_types,			"Song Target Types."	},
	{	"spec",					STRUCT_SPEC,		spec_table,					"Available special programs. {D(DEPRECATED){x"	},
	{	"spell_func",			STRUCT_SPELLFUNC,	spell_func_table,			"Spell functions (SkEdit)"},
	{	"spell_positions",		STRUCT_FLAGS,		spell_position_flags,		"Spell minimum positions."	},
	{	"spell_targets",		STRUCT_FLAGS,		spell_target_types,			"Spell Target Types."	},
	{	"spells",				STRUCT_SKILL,		NULL,						"Names of current spells."	},
	{	"stats",				STRUCT_FLAGS,		stat_types,					"Stat types"},
	{	"tattoo_loc",			STRUCT_FLAGS,		tattoo_loc_flags,			"Tattoo Locations."},
	{	"tokenflags",			STRUCT_FLAGS,		token_flags,				"Token flags."	},
	{	"touch_func",			STRUCT_ARTIFICING,	touch_func_table,			"Touch Functions (SkEdit)"},
	{	"tprog",				STRUCT_TRIGGERS,	dummy_triggers,				"TokenProgram types."	},
	{	"trigger_slots",		STRUCT_FLAGS,		trigger_slots,				"Trigger slots."},
	{	"trigger_types",		STRUCT_FLAGS,		builtin_trigger_types,		"Built-in trigger types."},
	{	"type",					STRUCT_FLAGS,		type_flags,					"Types of objects."	},
	{	"vuln",					STRUCT_FLAGS,		vuln_flags,					"Mobile vulnerability."	},
	{	"wclass",				STRUCT_FLAGS,		weapon_class,				"Weapon class."	},
	{	"weapon",				STRUCT_ATTACK,		attack_table,				"Weapon types."	},
	{	"wear",					STRUCT_FLAGS,		wear_flags,					"Where to wear object."	},
	{	"wear-loc",				STRUCT_FLAGS,		wear_loc_flags,				"Where mobile wears object."	},
	{	"wilderness_regions",	STRUCT_FLAGS,		wilderness_regions,			"wilderness region names"},
	{	"wtype",				STRUCT_FLAGS,		weapon_type2,				"Special weapon type."	},
	{	"zap_func",				STRUCT_ARTIFICING,	zap_func_table,				"Zap Functions (SkEdit)"},
	{	NULL,					STRUCT_FLAGS,		NULL,						NULL									}
};


static void __region_add_room(AREA_REGION *region, ROOM_INDEX_DATA *room)
{
	if (IS_VALID(region) && room)
	{
		list_appendlink(region->rooms, room);
		room->region = region;
	}
}

static void __region_remove_room(ROOM_INDEX_DATA *room)
{
	if (IS_VALID(room->region))
	{
		list_remlink(room->region->rooms, room, false);
	}
	room->region = NULL;
}

// Displays settable flags and stats.
void show_flag_cmds(CHAR_DATA *ch, const struct flag_type *flag_table)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;

    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if (flag_table[flag].settable)
	{
	    sprintf(buf, "%-19.18s", flag_table[flag].name);
	    strcat(buf1, buf);
	    if (++col % 4 == 0)
		strcat(buf1, "\n\r");
	}
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void show_flagbank_cmds(CHAR_DATA *ch, const struct flag_type **flag_bank)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;
	for(int b = 0; flag_bank[b]; b++)
	{
		for (int f = 0; flag_bank[b][f].name != NULL; f++)
		{
			if (flag_bank[b][f].settable)
			{
				sprintf(buf, "%-19.18s", flag_bank[b][f].name);
				strcat(buf1, buf);
				if (++col % 4 == 0)
					strcat(buf1, "\n\r");
			}
		}
	}

    if (col % 4 != 0)
		strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


// Displays all skill functions.
void show_skill_cmds(CHAR_DATA *ch, int tar)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    SKILL_DATA *skill;
    int  col;

    buf1[0] = '\0';
    col = 0;

	ITERATOR it;
	iterator_start(&it, skills_list);
    while((skill = (SKILL_DATA *)iterator_nextdata(&it)))
    {
		if (!is_skill_spell(skill)) continue;

		if (tar == -1 || skill->target == tar)
		{
			sprintf(buf, "{%c%-19.18s{x", (skill->token ? 'W' : 'x'), skill->name);
			strcat(buf1, buf);
			if (++col % 4 == 0)
				strcat(buf1, "\n\r");
		}
    }
	iterator_stop(&it);

    if (col % 4 != 0)
		strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


// Displays settable special functions.
void show_spec_cmds(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Preceed special functions with 'spec_'\n\r\n\r", ch);
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
	sprintf(buf, "%-19.18s", &spec_table[spec].name[5]);
	strcat(buf1, buf);
	if (++col % 4 == 0)
	    strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


void show_trigger_types(CHAR_DATA *ch, char *header, int prog)
{
	char buf[MIL];
	int n = 0;
	ITERATOR it;
	struct trigger_type *tt;

	send_to_char(header, ch);

	iterator_start(&it, trigger_list);
	while((tt = (struct trigger_type *)iterator_nextdata(&it)))
	{
		if (IS_SET(tt->progs, prog))
		{
			n++;
			sprintf(buf, "%-20s", tt->name);
			send_to_char(buf, ch);
			if (!(n % 4))
			send_to_char("\n\r", ch);
		}
	}
	iterator_stop(&it);

	if (n % 4)
		send_to_char("\n\r", ch);
}

void show_spell_funcs(CHAR_DATA *ch, const struct spell_func_type *table)
{
    char buf  [ MAX_STRING_LENGTH ];
//    char buf1 [ MAX_STRING_LENGTH ];
    int  col;
	BUFFER *buffer = new_buf();

//    buf1[0] = '\0';
    col = 0;
    add_buf(buffer, "Functions available for use:\n\r");
    for (int i = 0; table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", table[i].name);
		add_buf(buffer, buf);
		if (++col % 4 == 0)
	    	add_buf(buffer, "\n\r");
    }

    if (col % 4 != 0)
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
    return;
}

void show_do_funcs(CHAR_DATA *ch, const struct do_func_type *table)
{
    char buf  [ MAX_STRING_LENGTH ];
//    char buf1 [ MAX_STRING_LENGTH ];
    int  col;
	BUFFER *buffer = new_buf();

//    buf1[0] = '\0';
    col = 0;
    add_buf(buffer, "Functions available for use:\n\r");
    for (int i = 0; table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", table[i].name);
		add_buf(buffer, buf);
		if (++col % 4 == 0)
	    	add_buf(buffer, "\n\r");
    }

    if (col % 4 != 0)
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
    return;
}

void show_gsns(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Global Skill Numbers available for use:\n\r", ch);
    for (int i = 0; gsn_table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", gsn_table[i].name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void show_gcls(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Global Classes available for use:\n\r", ch);
    for (int i = 0; gcl_table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", gcl_table[i].name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void show_grs(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Global Races available for use:\n\r", ch);
    for (int i = 0; gr_table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", gr_table[i].name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void show_gscts(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Global Sectors available for use:\n\r", ch);
    for (int i = 0; global_sector_table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", global_sector_table[i].name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

void show_classes(CHAR_DATA *ch)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;

    buf1[0] = '\0';
    col = 0;

	ITERATOR it;
	CLASS_DATA *clazz;

    send_to_char("Classes available for use:\n\r", ch);
	iterator_start(&it, classes_list);
	while((clazz = (CLASS_DATA *)iterator_nextdata(&it)))
    {
		sprintf(buf, "%-19.18s", clazz->name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }
	iterator_stop(&it);

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


void show_artificing(CHAR_DATA *ch, const void *structure)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  col;
	const struct artifice_func_type *table = (const struct artifice_func_type *)structure;

    buf1[0] = '\0';
    col = 0;
    send_to_char("Functions available for use:\n\r", ch);
    for (int i = 0; table[i].name != NULL; i++)
    {
		sprintf(buf, "%-19.18s", table[i].name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
	    	strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


// Displays help for many tables used in OLC.
bool show_help(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument(argument, arg);
    one_argument(argument, spell);

    // Display syntax.
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:  ? [command]\n\r\n\r", ch);
	send_to_char("[command]  [description]\n\r", ch);
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	{
	    sprintf(buf, "%-10.10s -%s\n\r",
	        capitalize(help_table[cnt].command),
		help_table[cnt].desc);
	    send_to_char(buf, ch);
	}
	return false;
    }

    // Find the command, show changeable data.
    for (cnt = 0; help_table[cnt].command != NULL; cnt++)
    {
        if ( arg[0] == help_table[cnt].command[0]
          && !str_prefix(arg, help_table[cnt].command))
		{
			switch(help_table[cnt].structure_type)
			{
				case STRUCT_SPEC:
					show_spec_cmds(ch);
					break;

				case STRUCT_LIQUID:
					show_liqlist(ch);
					break;

				case STRUCT_ATTACK:
					show_damlist(ch);
					break;

				case STRUCT_MATERIAL:
					show_material_list(ch);
					return false;
				
				case STRUCT_SKILL:
					if (spell[0] == '\0')
					{
						send_to_char("Syntax:  ? spells [ignore/attack/defend/self/object/all]\n\r", ch);
						return false;
					}

					if (!str_prefix(spell, "all"))
						show_skill_cmds(ch, -1);
					else if (!str_prefix(spell, "ignore"))
						show_skill_cmds(ch, TAR_IGNORE);
					else if (!str_prefix(spell, "attack"))
						show_skill_cmds(ch, TAR_CHAR_OFFENSIVE);
					else if (!str_prefix(spell, "defend"))
						show_skill_cmds(ch, TAR_CHAR_DEFENSIVE);
					else if (!str_prefix(spell, "self"))
						show_skill_cmds(ch, TAR_CHAR_SELF);
					else if (!str_prefix(spell, "object"))
						show_skill_cmds(ch, TAR_OBJ_INV);
					else
						send_to_char("Syntax:  ? spell [ignore/attack/defend/self/object/all]\n\r", ch);

					break;

				case STRUCT_SPELLFUNC:
					show_spell_funcs(ch, (const struct spell_func_type *)help_table[cnt].structure);
					break;

				case STRUCT_DOFUNC:
					show_do_funcs(ch, (const struct do_func_type *)help_table[cnt].structure);
					break;

				case STRUCT_GSN:
					show_gsns(ch);
					break;

				case STRUCT_CLASSES:
					show_classes(ch);
					break;

				case STRUCT_ARTIFICING:
					show_artificing(ch, help_table[cnt].structure);
					break;

				case STRUCT_GCL:
					show_gcls(ch);
					break;

				case STRUCT_GR:
					show_grs(ch);
					break;

				case STRUCT_GSCT:
					show_gscts(ch);
					break;

				case STRUCT_FLAGS:
					show_flag_cmds(ch, help_table[cnt].structure);
					break;

				case STRUCT_FLAGBANK:
					show_flagbank_cmds(ch, (const struct flag_type **)help_table[cnt].structure);
					break;
			}
			return false;
		}
    }

    show_help(ch, "");
    return false;
}


// Show the list of object materials to a builder
void show_material_list(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH ];
    char buf1[MAX_STRING_LENGTH ];
    int col;

	ITERATOR it;
	MATERIAL *material;
    buf1[0] = '\0';
    col = 0;
	iterator_start(&it, material_list);
	while((material = (MATERIAL *)iterator_nextdata(&it)))
	{
		sprintf(buf, "%-19.18s", material->name);
		strcat(buf1, buf);
		if (++col % 4 == 0)
			strcat(buf1, "\n\r");
	}
	iterator_stop(&it);

    if (col % 4 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}


// Purpose:	Ensures the range spans only one area.
bool check_range(long lower, long upper)
{
    AREA_DATA *pArea;
    int cnt = 0;

    for (pArea = area_first; pArea; pArea = pArea->next)
    {
	/*
	 * lower < area < upper
	 */
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper)
	||   (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
	    ++cnt;

	if (cnt > 1)
	    return false;
    }
    return true;
}

bool edit_deltrigger(LLIST **list, int index)
{
	PROG_LIST *trigger;
	int slot;
	ITERATOR it;

	if(list) {
		for(slot = 0; slot < TRIGSLOT_MAX; slot++) {
			iterator_start(&it, list[slot]);
			while(( trigger = (PROG_LIST *)iterator_nextdata(&it) )) {
				if(!index--) {
					iterator_remcurrent(&it);
					break;
				}
			}
			iterator_stop(&it);

			if(trigger) {
				struct trigger_type *tt = get_trigger_type_bytype(trigger->trig_type);
				trigger_type_delete_use(tt);

				free_trigger(trigger);
				return true;
			}
		}
	}

	return false;
}


AREA_DATA *get_vnum_area(long vnum)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next)
    {
        if (vnum >= pArea->min_vnum
          && vnum <= pArea->max_vnum)
            return pArea;
    }

    return 0;
}

static void __aedit_show_region(BUFFER *buffer, AREA_DATA *pArea, AREA_REGION *region)
{
    char buf  [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *recall;

	if (!IS_NULLSTR(region->description))
	{
		sprintf(buf, "  Description:\n\r%s\n\r", region->description);
		add_buf(buffer, buf);
	}

	if(region->recall.wuid) {
		WILDS_DATA *wilds = get_wilds_from_uid(NULL,region->recall.wuid);
		if(wilds)
			sprintf(buf, "  Recall:      Wilds %s [%lu] at <%lu,%lu,%lu>\n\r", wilds->name, region->recall.wuid,
				region->recall.id[0],region->recall.id[1],region->recall.id[2]);
		else
			sprintf(buf, "  Recall:      Wilds ??? [%lu]\n\r", region->recall.wuid);
	} else if(region->recall.id[0] > 0 && (recall = get_room_index(pArea, region->recall.id[0]))) {
		sprintf(buf, "  Recall:    Room [%5ld] %s\n\r", region->recall.id[0], recall->name);
	} else
		sprintf(buf, "  Recall:      [%lu] area default\n\r", region->recall.id[0]);
	add_buf(buffer, buf);

	sprintf(buf, "  Flags:       [%s]\n\r", flag_string(area_region_flags, region->flags));
	add_buf(buffer, buf);

    sprintf(buf, "  AreaWho:     [%s] [%s]\n\r", flag_string(area_who_titles, region->area_who), flag_string(area_who_display, region->area_who));
	add_buf(buffer, buf);

    sprintf(buf, "  Savagery:    [%d]\n\r", region->rs_savage_level);
	add_buf(buffer, buf);

    sprintf(buf, "  PlaceType:   [%s]\n\r", flag_string(place_flags, region->rs_place_flags));
	add_buf(buffer, buf);

    sprintf(buf, "  X : Y:       [%d, %d]\n\r", region->rs_x, region->rs_y);
	add_buf(buffer, buf);

    sprintf(buf, "  Land X:Y:    [%d, %d]\n\r", region->rs_land_x, region->rs_land_y);
	add_buf(buffer, buf);

    sprintf(buf, "  AirshipLand: [%s (%ld)]\n\r", get_room_index(pArea, region->rs_airship_land_spot) == NULL ? "None" :
        get_room_index(pArea,region->rs_airship_land_spot)->name, region->rs_airship_land_spot);
	add_buf(buffer, buf);

	sprintf(buf, "  PostOffice:  [%s (%ld)]\n\r",
		get_room_index(pArea,region->post_office) == NULL ? "None" :
		get_room_index(pArea,region->post_office)->name, region->post_office);
	add_buf(buffer, buf);

	if (!IS_NULLSTR(region->comments))
	{
		sprintf(buf,"  -----\n\r  {WBuilders' Comments:{X\n\r  %s{x\n\r  -----\n\r", pArea->comments);
		add_buf(buffer, buf);
	}

}







/* The version to allow rooms to change exits */
bool rp_change_exit(ROOM_INDEX_DATA *pRoom, char *argument, int door)
{
    EXIT_DATA *pExit;
    ROOM_INDEX_DATA *pToRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];


    /*
     * Now parse the arguments.
     */
    argument = one_argument(argument, command);
    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
	bug("Rprog: No vnum to create entrance or delete, on room %d.",
		pRoom->vnum);
	return false;
    }

    if (!str_cmp(arg, "delete"))
    {
	int16_t rev;

	if (!pRoom->exit[door])
	{
	    bug("RProg: Couldn't delete room. %d", pRoom->vnum);
	    return false;
	}

	/*
	 * Remove ToRoom Exit.
	 */
	rev = rev_dir[door];
	pToRoom = pRoom->exit[door]->u1.to_room;

	if (pToRoom->exit[rev])
	{
	    free_exit(pToRoom->exit[rev]);
	    pToRoom->exit[rev] = NULL;
	}

	/*
	 * Remove this exit.
	 */
	free_exit(pRoom->exit[door]);
	pRoom->exit[door] = NULL;

	return true;
    }

	WNUM wnum;
	if (!parse_widevnum(arg, pRoom->area, &wnum))
    {
       bug("Rprog: A link cannot link non-existant room.\n\r",0);
       return false;
    }

    if (!get_room_index(wnum.pArea, wnum.vnum))
    {
       bug("Rprog: A link cannot link non-existant room.\n\r",0);
       return false;
    }

    if (get_room_index(wnum.pArea, wnum.vnum)->exit[rev_dir[door]])
    {
       bug("Rprog: Reverse-side exit to room already exists.", 0);
       return false;
    }

    if (!pRoom->exit[door])
    {
       pRoom->exit[door] = new_exit();
	pRoom->exit[door]->from_room = pRoom;
    }

    pToRoom = pRoom->exit[door]->u1.to_room = get_room_index(wnum.pArea, wnum.vnum);
    pRoom->exit[door]->orig_door = door;

    /*	pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

    door                    = rev_dir[door];
    pExit                   = new_exit();
    pExit->u1.to_room       = pRoom;
    /*	pExit->vnum             = pRoom->vnum;    Can't set vnum in ROM */
    pExit->orig_door	= door;
    pToRoom->exit[door]       = pExit;
    pExit->from_room = pToRoom;

    return true;
}

/* Local function. */
bool change_exit(CHAR_DATA *ch, char *argument, int door)
{
	ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *to_room;
	int count;
	char command[MAX_INPUT_LENGTH];
	char commandn[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char buf[MSL];
	long value;

	EDIT_ROOM_SIMPLE(ch, pRoom);

	/*
	* Now parse the arguments.
	*/
	argument = one_argument_norm(argument, command);
	count = mult_argument(command, commandn);
	one_argument_norm(argument, arg);

	if (command[0] == '\0' && argument[0] == '\0')	/* Move command. */
	{
		move_char(ch, door, true, false);
		return false;
	}

	if (!str_cmp(command, "bothflags"))
	{
		// Set the exit flags
		if (!room_is_clone(pRoom) && (value = flag_value(exit_flags, argument)) != NO_FLAG)
		{
			ROOM_INDEX_DATA *pToRoom;
			int16_t rev;

			if (!pRoom->exit[door])
			{
				send_to_char("Exit doesn't exist.\n\r",ch);
				return false;
			}

			TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
			// Don't toggle exit_info because it can be changed by players.
			pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

			pToRoom = pRoom->exit[door]->u1.to_room;
			rev = rev_dir[door];

			// Set the exit as environment
			if( IS_SET(pRoom->exit[door]->exit_info, EX_ENVIRONMENT) && IS_SET(value, EX_ENVIRONMENT) )
			{
				// Delete remote exit
				if( pToRoom != NULL && pRoom->exit[rev] != NULL && pRoom->exit[rev]->u1.to_room == pRoom)
				{
					free_exit(pRoom->exit[rev]);
					pRoom->exit[rev] = NULL;
				}

				// Remove destination
				pRoom->exit[door]->u1.to_room = NULL;
				send_to_char("Exit flag toggled.\n\rEnvironment exit distination unlinked.\n\r", ch);
			}
			else
			{
				// Only toggle if the exits go to each others' room.
				if (pToRoom != NULL && pToRoom->exit[rev] != NULL && pToRoom->exit[rev]->u1.to_room == pRoom)
				{
					TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
					TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
				}

				send_to_char("Exit flag toggled.\n\r", ch);
			}

			return true;
		}

		send_to_char("Invalid exit flag.  Use '? exit' to see list of valid flags.\n\r", ch);
		return false;
	}

	if (!str_cmp(command, "flags"))
	{
		// Set the exit flags
		if (!room_is_clone(pRoom) && (value = flag_value(exit_flags, argument)) != NO_FLAG)
		{
			ROOM_INDEX_DATA *pToRoom;
			int16_t rev;

			if (!pRoom->exit[door])
			{
				send_to_char("Exit doesn't exist.\n\r",ch);
				return false;
			}

			TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
			// Don't toggle exit_info because it can be changed by players.
			pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

			pToRoom = pRoom->exit[door]->u1.to_room;
			rev = rev_dir[door];

			// Set the exit as environment
			if( IS_SET(pRoom->exit[door]->exit_info, EX_ENVIRONMENT) && IS_SET(value, EX_ENVIRONMENT) )
			{
				// Delete remote exit
				if( pToRoom != NULL && pRoom->exit[rev] != NULL && pRoom->exit[rev]->u1.to_room == pRoom)
				{
					free_exit(pRoom->exit[rev]);
					pRoom->exit[rev] = NULL;
				}

				// Remove destination
				pRoom->exit[door]->u1.to_room = NULL;
				send_to_char("Exit flag toggled.\n\rEnvironment exit distination unlinked.\n\r", ch);
			}
			else
			{
				send_to_char("Exit flag toggled.\n\r", ch);
			}

			return true;
		}

		send_to_char("Invalid exit flag.  Use '? exit' to see list of valid flags.\n\r", ch);
		return false;
	}

	if(room_is_clone(pRoom)) return false;

	if (command[0] == '?')
	{
		send_to_char("You must specify an argument.\n\r", ch);
		return false;
	}

	if (!str_cmp(command, "delete"))
	{
		ROOM_INDEX_DATA *pToRoom;
		int16_t rev;
		bool both = !argument[0] || !str_prefix(argument, "both");

		if (!pRoom->exit[door])
		{
			send_to_char("REdit:  Cannot delete a null exit.\n\r", ch);
			return false;
		}

		/*
		* Remove ToRoom Exit.
		*/
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->u1.to_room;
		if (pToRoom == NULL)
		{
			snprintf(buf, sizeof(buf), "change_exit: pToRoom was null! room is %s (%ld), door is %i",
				pRoom->name, pRoom->vnum, door);
			bug(buf, 0);
			send_to_char("REdit: couldn't delete that exit, probably a bad link. Please report to coder@megacosm.net\n\r", ch);
			return false;
		}

		// Only do it if the exit connects back to *this* room, and we want to delete both sides (which is the default)
		if (both && pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room == pRoom)
		{
			free_exit(pToRoom->exit[rev]);
			pToRoom->exit[rev] = NULL;
		}

		/*
		* Remove this exit.
		*/
		free_exit(pRoom->exit[door]);
		pRoom->exit[door] = NULL;

		send_to_char("Exit unlinked.\n\r", ch);
		return true;
	}

	if (!str_cmp(command, "environment"))
	{
		if (!pRoom->exit[door])
		{
			// Environment exits can be created directly
			pRoom->exit[door] = new_exit();
			pRoom->exit[door]->from_room = pRoom;
			pRoom->exit[door]->u1.to_room = NULL;
			pRoom->exit[door]->orig_door = door;
			SET_BIT(pRoom->exit[door]->rs_flags,  EX_ENVIRONMENT);

			send_to_char("Environment exit created.\n\r",ch);
			return true;
		}

		send_to_char("You must first delete the current one.\n\r", ch);
		return false;
	}

	if (!str_cmp(command, "link"))
	{
		EXIT_DATA *pExit;
		WNUM wnum;

		if (arg[0] == '\0' || !parse_widevnum(arg, ch->in_room->area, &wnum))
		{
			send_to_char("Syntax:  [direction] link [wnum]\n\r", ch);
			return false;
		}

		ROOM_INDEX_DATA *pToRoom = get_room_index(wnum.pArea, wnum.vnum);

		if (!pToRoom)
		{
			send_to_char("REdit:  Cannot link to non-existant room.\n\r", ch);
			return false;
		}

		if (!IS_BUILDER(ch, pToRoom->area))
		{
			send_to_char("REdit:  Cannot link to that area.\n\r", ch);
			return false;
		}

		if( !rooms_in_same_section(pRoom->area, pRoom->vnum, pToRoom->area, pToRoom->vnum) )
		{
			send_to_char("REdit:  Attempting to link outside of a defined blueprint section.\n\r", ch);
			return false;
		}

		if (pToRoom->exit[rev_dir[door]])
		{
			send_to_char("REdit:  Remote side's exit already exists.\n\r", ch);
			return false;
		}

		if (!pRoom->exit[door])
		{
			pRoom->exit[door] = new_exit();
			pRoom->exit[door]->from_room = pRoom;
		}
		else
		{
			if( IS_SET(pRoom->exit[door]->exit_info, EX_ENVIRONMENT) )
			{
				send_to_char("REdit:  Environment exits cannot be linked.\n\r", ch);
				return false;
			}
		}


		pRoom->exit[door]->u1.to_room	= pToRoom;
		pRoom->exit[door]->orig_door	= door;
		door							= rev_dir[door];
		pExit							= new_exit();

		pExit->u1.to_room				= pRoom;
		pExit->orig_door				= door;
		pToRoom->exit[door]				= pExit;
		pExit->from_room				= pToRoom;

		send_to_char("Two-way link established.\n\r", ch);
		return true;
	}

	if (!str_cmp(command, "dig"))
	{
		WNUM wnum;
		if (arg[0] == '\0')
		{
			wnum.pArea = ch->in_room->area;
			for(wnum.vnum = ch->in_room->vnum + 1; wnum.vnum > 0 && get_room_index(wnum.pArea, wnum.vnum); wnum.vnum++);

			if (wnum.vnum < 1)
			{
				send_to_char("Could not find a widevnum to dig.\n\r", ch);
				return false;
			}

			// Create the argument for the link command
			sprintf(arg, "%ld#%ld", wnum.pArea->uid, wnum.vnum);
		}
		else if (!parse_widevnum(arg, ch->in_room->area, &wnum))
		{
			send_to_char("Syntax:  [direction] dig [wnum]\n\r", ch);
			return false;
		}
		else if (count > 1)
		{
			send_to_char("Cannot use multi-dig mode when specifying a room widevnum.\n\r", ch);
			return false;
		}

		if( IS_SET(ch->in_room->room_flag[1], ROOM_BLUEPRINT) ||
			IS_SET(ch->in_room->area->area_flags, ROOM_BLUEPRINT) )
		{
			if( !rooms_in_same_section(pRoom->area, pRoom->vnum, wnum.pArea, wnum.vnum) )
			{
				send_to_char("REdit:  Attempting to dig outside of a defined blueprint section.\n\r", ch);
				return false;
			}

			redit_blueprint_oncreate = (IS_SET(ch->in_room->room_flag[1], ROOM_BLUEPRINT)) && true;
		}

		if( pRoom->exit[door] && IS_SET(pRoom->exit[door]->exit_info, EX_ENVIRONMENT) )
		{
			send_to_char("REdit:  Environment exits cannot be linked.\n\r", ch);
			return false;
		}

		redit_create(ch, arg);
		sprintf(buf, "link %s", arg);
		change_exit(ch, buf, door);
		return true;
	}

	if (!str_cmp(command, "room"))
	{
		ROOM_INDEX_DATA *pToRoom;
		EXIT_DATA *pExit;
		int16_t rev;
		WNUM wnum;

		if (arg[0] == '\0' || !parse_widevnum(arg, ch->in_room->area, &wnum))
		{
			send_to_char("Syntax:  [direction] room [wnum]\n\r", ch);
			return false;
		}

		if (!(pExit = pRoom->exit[door]))
		{
			pExit = pRoom->exit[door] = new_exit();
		}
		else
		{
			if( IS_SET(pRoom->exit[door]->exit_info, EX_ENVIRONMENT) )
			{
				send_to_char("REdit:  Environment exits cannot be linked.\n\r", ch);
				return false;
			}

		}

		pToRoom = get_room_index(wnum.pArea, wnum.vnum);

		if (!pToRoom)
		{
			send_to_char("REdit:  Cannot link to non-existant room.\n\r", ch);
			return false;
		}

		if( !rooms_in_same_section(pRoom->area, pRoom->vnum, wnum.pArea, wnum.vnum) )
		{
			send_to_char("REdit:  Attempting to link outside of a defined blueprint section.\n\r", ch);
			return false;
		}

		rev = rev_dir[door];
		if( pToRoom->exit[rev] && IS_SET(pToRoom->exit[rev]->exit_info, EX_ENVIRONMENT) )
		{
			send_to_char("REdit:  Destination has an environment exit in the reverse direction.\n\r", ch);
			return false;
		}

		pRoom->exit[door]->u1.to_room	= pToRoom;
		pRoom->exit[door]->orig_door	= door;
		pExit->from_room				= pRoom;

		send_to_char("One-way link established.\n\r", ch);
		return true;
	}

	if (!str_cmp(command, "lockflags"))
	{
		if( (value = flag_value(lock_flags, argument)) == NO_FLAG )
		{
			send_to_char("Syntax:  [direction] lockflags [flags]\n\r", ch);
			return false;
		}


		if (!pRoom->exit[door])
		{
			send_to_char("Exit doesn't exist.\n\r",ch);
			return false;
		}

		TOGGLE_BIT(pRoom->exit[door]->door.rs_lock.flags,  value);
		// Don't toggle exit_info because it can be changed by players.
		pRoom->exit[door]->door.lock.flags = pRoom->exit[door]->door.rs_lock.flags;

		ROOM_INDEX_DATA *pToRoom = pRoom->exit[door]->u1.to_room;
		int rev = rev_dir[door];

		// Only toggle if the exits go to each others' room.
		if (pToRoom != NULL && pToRoom->exit[rev] != NULL && pToRoom->exit[rev]->u1.to_room == pRoom)
		{
			TOGGLE_BIT(pToRoom->exit[rev]->door.rs_lock.flags,  value);
			TOGGLE_BIT(pToRoom->exit[rev]->door.lock.flags, value);
		}

		send_to_char("Exit flag toggled.\n\r", ch);
		return true;
	}


	if (!str_cmp(command, "pick_chance"))
	{
		if (arg[0] == '\0' || !is_number(arg))
		{
			send_to_char("Syntax:  [direction] pick_chance [0-100]\n\r", ch);
			return false;
		}

		if (!pRoom->exit[door])
		{
			send_to_char("Exit doesn't exist.\n\r",ch);
			return false;
		}

		value = atoi(arg);

		if( value < 0 || value > 100 )
		{
			send_to_char("Chance between 0% and 100%.\n\r",ch);
			return false;
		}

		pRoom->exit[door]->door.lock.pick_chance =
		pRoom->exit[door]->door.rs_lock.pick_chance = value;

		send_to_char("Exit pick chance set.\n\r", ch);
		return true;
	}

	if (!str_cmp(command, "key"))
	{
		WNUM wnum;
		if (arg[0] == '\0' || !parse_widevnum(arg, ch->in_room->area, &wnum))
		{
			send_to_char("Syntax:  [direction] key [widevnum]\n\r", ch);
			return false;
		}

		if (!pRoom->exit[door])
		{
			send_to_char("Exit doesn't exist.\n\r",ch);
			return false;
		}

		OBJ_INDEX_DATA *key = get_obj_index(wnum.pArea, wnum.vnum);
		if (!key)
		{
			send_to_char("REdit:  Item doesn't exist.\n\r", ch);
			return false;
		}

		if (key->item_type != ITEM_KEY)
		{
			send_to_char("REdit:  Key doesn't exist.\n\r", ch);
			return false;
		}

		pRoom->exit[door]->door.lock.key_wnum = wnum;
		pRoom->exit[door]->door.rs_lock.key_wnum = wnum;

		send_to_char("Exit key set.\n\r", ch);
		return true;
	}

	if (!str_cmp(command, "name"))
	{
	if (arg[0] == '\0')
	{
	send_to_char("Syntax:  [direction] name [string]\n\r", ch);
	send_to_char("         [direction] name none\n\r", ch);
	return false;
	}

	if (!pRoom->exit[door])
	{
	send_to_char("Exit doesn't exist.\n\r",ch);
	return false;
	}

	free_string(pRoom->exit[door]->keyword);
	if (str_cmp(arg,"none"))
	{
		pRoom->exit[door]->keyword = str_dup(arg);
		if ((to_room = pRoom->exit[door]->u1.to_room) != NULL && to_room->exit[rev_dir[door]] != NULL && to_room->exit[rev_dir[door]]->u1.to_room == pRoom)
		{
			free_string(to_room->exit[rev_dir[door]]->keyword);
			to_room->exit[rev_dir[door]]->keyword = str_dup(arg);
		}
	}
	else
	{
		pRoom->exit[door]->keyword = str_dup("");
		if ((to_room = pRoom->exit[door]->u1.to_room) != NULL && to_room->exit[rev_dir[door]] != NULL && to_room->exit[rev_dir[door]]->u1.to_room == pRoom)
		{
			free_string(to_room->exit[rev_dir[door]]->keyword);
			to_room->exit[rev_dir[door]]->keyword = str_dup("");
		}
	}

	send_to_char("Exit name set.\n\r", ch);
	return true;
	}

	if (!str_prefix(command, "description"))
	{
	if (arg[0] == '\0')
	{
	if (!pRoom->exit[door])
	{
	send_to_char("Exit doesn't exist.\n\r",ch);
	return false;
	}

	string_append(ch, &pRoom->exit[door]->short_desc);
	return true;
	}

	send_to_char("Syntax:  [direction] desc\n\r", ch);
	return false;
	}

	return false;
}




struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};


const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {	WEAR_HOLD,	ITEM_HOLD		},
    {   WEAR_RING_FINGER, ITEM_WEAR_RING_FINGER },
    {	NO_FLAG,	NO_FLAG			}
};


// Returns the location of the bit that matches the count.
int wear_loc(long bits, int count)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if (IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }

    return NO_FLAG;
}


/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;

    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if (loc == wear_table[flag].wear_loc)
            return wear_table[flag].wear_bit;
    }

    return 0;
}



void print_obj_portal_values(OBJ_INDEX_DATA *obj, BUFFER *buffer)
{
	char buf[MSL];
	PORTAL_DATA *portal = PORTAL(obj);

	add_buf(buffer, "\n\r{GPortal:{x\n\r");
	sprintf(buf, "{B[{WName             {B]:  {x%s\n\r", portal->name);
	add_buf(buffer, buf);
	sprintf(buf, "{B[{WShort Description{B]:  {x%s\n\r", portal->short_descr);
	add_buf(buffer, buf);
	if (portal->charges < 0)
		sprintf(buf, "{B[{WCharges          {B]:  {xInfinite\n\r");
	else
		sprintf(buf, "{B[{WCharges          {B]:  {x%d\n\r", portal->charges);
	add_buf(buffer, buf);
	sprintf(buf, "{B[{WExit Flags       {B]:  {x%s\n\r", flag_string(portal_exit_flags, portal->exit));
	add_buf(buffer, buf);
	sprintf(buf, "{B[{WPortal Flags     {B]:  {x%s\n\r", flag_string(portal_exit_flags, portal->flags));
	add_buf(buffer, buf);
	sprintf(buf, "{B[{WPortal Type      {B]:  {x%s\n\r", flag_string(portal_gatetype, portal->type));
	add_buf(buffer, buf);

	AREA_DATA *area;
	WILDS_DATA *wilds;
	ROOM_INDEX_DATA *room;
	DUNGEON_INDEX_DATA *dungeon;
	BLUEPRINT *blueprint;
	switch(portal->type)
	{
		case GATETYPE_ENVIRONMENT:
			// Nothing gets set on environment portals
			sprintf(buf, "{B[{WDestination      {B]:  Current Environment{x\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_NORMAL:
			area = get_area_from_uid(portal->params[0]);
			room = get_room_index(area,portal->params[1]);
			if (room)
				sprintf(buf,
					"{B[{WDestination      {B]:  {x%s{B ({x%ld{B) in {x%s {B({x%ld{B){x\n\r",
					room->name, portal->params[1],
					area->name, portal->params[0]);
			else
				sprintf(buf, "{B[{WDestination      {B]:  {xnone\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_WILDS:
			wilds = get_wilds_from_uid(NULL, portal->params[0]);
			sprintf(buf,
				"{B[{WDestination      {B]:  {x%s{B ({x%ld{B) at ({x%ld{B, {x%ld{B){x\n\r",
				wilds ? wilds->name : "none", portal->params[0],
				portal->params[1],
				portal->params[2]);
			add_buf(buffer, buf);
			break;

		case GATETYPE_WILDSRANDOM:
			wilds = get_wilds_from_uid(NULL, portal->params[0]);
			sprintf(buf,
				"{B[{WDestination      {B]:  {x%s{B ({x%ld{B) at ({x%ld{B, {x%ld{B) to ({x%ld{B, {x%ld{B){x\n\r",
				wilds ? wilds->name : "none", portal->params[0],
				UMIN(portal->params[1],portal->params[3]),
				UMIN(portal->params[2],portal->params[4]),
				UMAX(portal->params[1],portal->params[3]),
				UMAX(portal->params[2],portal->params[4]));
			add_buf(buffer, buf);
			break;

		case GATETYPE_AREARANDOM:
			if (portal->params[0] > 0)
			{
				area = get_area_from_uid(portal->params[0]);
				sprintf(buf,
					"{B[{WDestination      {B]:  {x%s{B ({x%ld{B){x\n\r",
					area ? area->name : "none",
					portal->params[0]);
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  {Y-current area or wilderness-{B ({x%ld{B){x\n\r",
					portal->params[0]);
			}
			add_buf(buffer, buf);
			break;
		
		case GATETYPE_REGIONRANDOM:
		{
			char buf2[MIL];
			if (portal->params[0] > 0)
			{
				area = get_area_from_uid(portal->params[0]);
				sprintf(buf2,
					"{x%s{B ({x%ld{B)",
					area ? area->name : "none",
					portal->params[0]);
			}
			else
			{
				sprintf(buf2,
					"{Y-current area or wilderness-{B");
			}

			if (portal->params[1] > 0)
			{
				if (portal->params[0] > 0)
				{
					area = get_area_from_uid(portal->params[0]);

					AREA_REGION *region = NULL;

					if (area)
					{
						region = (AREA_REGION *)list_nthdata(area->regions, portal->params[1]);
					}

					sprintf(buf,
						"{B[{WDestination      {B]:  %s in Region {x%s{B ({x%ld{B){x\n\r",
						buf2,
						region ? region->name : "{D-invalid-",
						portal->params[1]);
				}
				else
				{
					sprintf(buf,
						"{B[{WDestination      {B]:  %s in Region ({x%ld{B){x\n\r",
						buf2,
						portal->params[1]);
				}
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  %s in {YDefault{B Region{x\n\r",
					buf2);
			}
			add_buf(buffer, buf);
			break;
		}

		case GATETYPE_SECTIONRANDOM:
			if (portal->params[0])
				sprintf(buf,
					"{B[{WDestination      {B]:  %s{B Section {x%d\n\r",
					((portal->params[0] > 0)?"{YGenerated":((portal->params[0] < 0)?"{GOrdinal":"{WCurrent")),
					abs(portal->params[0]));
			else
				sprintf(buf,
					"{B[{WDestination      {B]:  {WCurrent{B Section {x\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_INSTANCERANDOM:
			// No extra values - target is based upon current location
			sprintf(buf, "{B[{WDestination      {B]:  Random Room in {YCurrent{B Instance{x\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_DUNGEONRANDOM:
			// No extra values - target is based upon current location
			sprintf(buf, "{B[{WDestination      {B]:  Random Room in {YCurrent{B Dungeon{x\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_AREARECALL:
			if (portal->params[0] > 0)
			{
				area = get_area_from_uid(portal->params[0]);
				sprintf(buf,
					"{B[{WDestination      {B]:  Recall of %s{B ({x%ld{B){x\n\r",
					area ? area->name : "none",
					portal->params[0]);
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Recall of {YCurrent{B Area{x\n\r");
			}
			add_buf(buffer, buf);
			break;

		case GATETYPE_REGIONRECALL:
		{
			char buf2[MIL];
			if (portal->params[0] > 0)
			{
				area = get_area_from_uid(portal->params[0]);
				sprintf(buf2,
					"{x%s{B ({x%ld{B})",
					area ? area->name : "none",
					portal->params[0]);
			}
			else
			{
				sprintf(buf2,
					"{Y-current area-{x");
			}
			add_buf(buffer, buf);

			if (portal->params[1] > 0)
			{
				if (portal->params[0] > 0)
				{
					area = get_area_from_uid(portal->params[0]);

					AREA_REGION *region = NULL;

					if (area)
					{
						region = (AREA_REGION *)list_nthdata(area->regions, portal->params[1]);
					}

					sprintf(buf,
						"{B[{WDestination      {B]:  Recall of %s in {x%s{B ({x%ld{B){x\n\r",
						buf2,
						region ? region->name : "{D-invalid-",
						portal->params[1]);
				}
				else
				{
					sprintf(buf,
						"{B[{WDestination      {B]:  Recall of %s in Region ({x%ld{B){x\n\r",
						buf2,
						portal->params[1]);
				}
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Recall of %s in {YDefault{B Region{x\n\r",
					buf2);
			}
			add_buf(buffer, buf);
			break;
		}

		case GATETYPE_DUNGEON:
			dungeon = get_dungeon_index(obj->area, portal->params[0]);
			if (portal->params[1] > 0)
				sprintf(buf,
					"{B[{WDestination      {B]:  Floor %ld in {x%s{B ({x%ld{B){x\n\r",
					portal->params[1],
					dungeon ? dungeon->name : "none", portal->params[0]);
			else if (portal->params[2] > 0)
				sprintf(buf,
					"{B[{WDestination      {B]:  Special Room %ld in {x%s{B ({x%ld{B){x\n\r",
					portal->params[2],
					dungeon ? dungeon->name : "none", portal->params[0]);
			else
				sprintf(buf,
					"{B[{WDestination      {B]:  {YDefault{B Entrance in {x%s{B ({x%ld{B){x\n\r",
					dungeon ? dungeon->name : "none", portal->params[0]);
			add_buf(buffer, buf);
			break;

		case GATETYPE_INSTANCE:
		{
			char buf2[MIL];
			if (portal->params[1] > 0 || portal->params[2] > 0)
				sprintf(buf2, "{C({W%ld{C:{W%ld{C)", portal->params[1], portal->params[2]);
			else
				sprintf(buf2, "{C(Spawned)");

			blueprint = get_blueprint(obj->area, portal->params[0]);
			if (portal->params[3] > 0)
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Special Room {x%ld{B in {x%s{B ({x%ld{B) %s{x\n\r",
					portal->params[3],
					blueprint ? blueprint->name : "none", portal->params[0],
					buf2);
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  {YDefault{B Entrance in {x%s{B ({x%ld{B) %s{x\n\r",
					blueprint ? blueprint->name : "none", portal->params[0],
					buf2);
			}
			add_buf(buffer, buf);
			break;
		}

		case GATETYPE_RANDOM:
			sprintf(buf, "{B[{WDestination      {B]:  Random Room{x\n\r");
			add_buf(buffer, buf);
			break;

		case GATETYPE_DUNGEONFLOOR:
			dungeon = get_dungeon_index(obj->area, portal->params[0]);
			if (portal->params[1] > 0)
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Floor {x%ld{B in {x%s{B ({x%ld{B){x\n\r",
					portal->params[1],
					dungeon ? dungeon->name : "none", portal->params[0]);
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Previous/Next Floor in {x%s{B ({x%ld{B){x\n\r",
					dungeon ? dungeon->name : "none", portal->params[0]);
			}
			add_buf(buffer, buf);
			break;
		
		case GATETYPE_BLUEPRINT_SECTION_MAZE:
			sprintf(buf,
				"{B[{WDestination      {B]:  ({x%ld{B, {x%ld{B) in {x%s{x %d{B Maze Section{x\n\r",
				portal->params[1], portal->params[2],
				((portal->params[0] > 0)?"{YGenerated":((portal->params[0] < 0)?"{GOrdinal":"{WCurrent")), abs(portal->params[0]));
			add_buf(buffer, buf);
			break;
		
		case GATETYPE_BLUEPRINT_SPECIAL:
			sprintf(buf,
				"{B[{WDestination      {B]:  Special Room {x%ld{B in {YCurrent{B Instance{x\n\r",
				portal->params[0]);
			add_buf(buffer, buf);
			break;

		case GATETYPE_DUNGEON_FLOOR_SPECIAL:
			if (portal->params[1] > 0)
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  Special Room {x%ld{B on Floor %s %d{B in {YCurrent{B Dungeon{x\n\r",
					portal->params[1],
					((portal->params[0] > 0)?"{YGenerated":((portal->params[0] < 0)?"{GOrdinal":"{WCurrent")), abs(portal->params[0]));
			}
			else
			{
				sprintf(buf,
					"{B[{WDestination      {B]:  {YDefault{B Entrance on Floor %s %d{B in {YCurrent{B Dungeon{x\n\r",
					((portal->params[0] > 0)?"{YGenerated":((portal->params[0] < 0)?"{GOrdinal":"{WCurrent")), abs(portal->params[0]));
			}
			add_buf(buffer, buf);
			break;

		case GATETYPE_DUNGEON_SPECIAL:
			sprintf(buf,
				"{B[{WDestination      {B]:  Special Room {x%ld{B in {YCurrent{B Dungeon{x\n\r",
				portal->params[0]);
			add_buf(buffer, buf);
			break;

		case GATETYPE_DUNGEON_RANDOM_FLOOR:
			sprintf(buf,
				"{B[{WDestination      {B]:  Floors {x%ld{B to {x%ld{B in {YCurrent{B Dungeon{x\n\r",
				portal->params[0], portal->params[1]);
			add_buf(buffer, buf);
			break;

	}
}

void print_lock_state(LOCK_STATE *lock, BUFFER *buffer, char *indent)
{
	char buf[MSL];
	OBJ_INDEX_DATA *lock_key = get_obj_index(lock->key_wnum.pArea, lock->key_wnum.vnum);

	sprintf(buf,"%s {x[{YLock State:{x]\n\r"
				"%s   Key:         {B[{x%ld#%ld{B]{x %s\n\r"
				"%s   Flags:       {B[{x%s{B]{x\n\r"
				"%s   Pick Chance: {B[{x%d%%{B]{x\n\r",
				indent,
				indent, lock->key_wnum.pArea ? lock->key_wnum.pArea->uid : 0,
					lock->key_wnum.vnum,
					lock_key ? lock_key->short_descr : "none",
				indent, flag_string(lock_flags, lock->flags),
				indent, lock->pick_chance);
	add_buf(buffer, buf);
}

// send obj values to a buffer
void print_obj_values(OBJ_INDEX_DATA *obj, BUFFER *buffer)
{
    char buf[MAX_STRING_LENGTH];
	ITERATOR it;

    add_buf(buffer, "\n\r");

	// MULTI-TYPING
	if (IS_AMMO(obj))
	{
		AMMO_DATA *ammo = AMMO(obj);
		add_buf(buffer, "\n\r{GAmmo:{x\n\r");
		sprintf(buf, "{B[{WType         {B]:  {x%s\n\r", flag_string(ammo_types, ammo->type));
		add_buf(buffer, buf);

		if (ammo->damage_type >= 0)
			sprintf(buf, "{B[{WDamage Type  {B]:  {x%s\n\r", attack_table[ammo->damage_type].noun);
		else
			sprintf(buf, "{B[{WDamage Type  {B]:  none{x\n\r");
		add_buf(buffer, buf);

		DICE_DATA dice = ammo->damage;
		sprintf(buf, "{B[{WDamage       {B]:  {x%s\n\r",
			((dice.bonus > 0) ?
				formatf("{x%d{Bd{x%d{B+{x%d", dice.number, dice.size, dice.bonus) :
				formatf("{x%d{Bd{x%d", dice.number, dice.size)));
		add_buf(buffer, buf);

		sprintf(buf, "{B[{WFlags        {B]:  {x%s\n\r", flag_string(weapon_type2, ammo->flags));
		add_buf(buffer, buf);
	}

	if (IS_ARMOR(obj))
	{
		ARMOR_DATA *armor = ARMOR(obj);
		add_buf(buffer, "\n\r{GArmor:{x\n\r");
		sprintf(buf, "{B[{WType             {B]:  {x%s\n\r", flag_string(armour_types, armor->armor_type));
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WStrength         {B]:  {x%s\n\r", flag_string(armour_strength_table, armor->armor_strength));
		add_buf(buffer, buf);
		for(int i = 0; i < ARMOR_MAX; i++)
		{
			sprintf(buf, "{B[{Y{+%-17s{B]:  {x%d\n\r", flag_string(armour_protection_types, i), armor->protection[i]);
			add_buf(buffer, buf);
		}
		sprintf(buf, "{B[{WMax Adornments   {B]:  {x%d\n\r", armor->max_adornments);
		add_buf(buffer, buf);

		if (armor->adornments != NULL)
		{
			add_buf(buffer, "\n\rAdornments:\n\r");
			add_buf(buffer, "#  [   Type   ] [     Spell Name     ] [ Level ]\n\r");
			add_buf(buffer, "=================================================\n\r");
			for(int i = 0; i < armor->max_adornments; i++)
			{
				ADORNMENT_DATA *adorn = armor->adornments[i];
				if (IS_VALID(adorn))
				{
					if (adorn->spell != NULL)
						sprintf(buf, "%d)  %-10s   %-20s    %5d\n\r", i + 1,
							flag_string(adornment_types, adorn->type),
							adorn->spell->skill->name,
							adorn->spell->level);
					else
						sprintf(buf, "%d)  %-10s   {D-no spell-{x\n\r", i + 1,
							flag_string(adornment_types, adorn->type));
				}
				else
					sprintf(buf, "%d)  %-10s{x\n\r", i+1, "---");

				add_buf(buffer, buf);
			}
		}
	}

	if (IS_BODY_PART(obj))
	{
		BODY_PART_DATA *bp = BODY_PART(obj);

		add_buf(buffer, "\n\r{GBody Part:{x\n\r");
		if (IS_VALID(bp->race))
			sprintf(buf, "{B[{WRace             {B]:  {x%s\n\r", bp->race->name);
		else
			sprintf(buf, "{B[{WRace             {B]:  {Dnone{x\n\r");
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WParts            {B]:  {x%s\n\r", flag_string(part_flags, bp->parts));
		add_buf(buffer, buf);
	}

	if (IS_BOOK(obj))
	{
		add_buf(buffer, "\n\r{GBook:{x\n\r");
		sprintf(buf, "{B[{WName             {B]:  {x%s\n\r", BOOK(obj)->name);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WShort Description{B]:  {x%s\n\r", BOOK(obj)->short_descr);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFlags            {B]:  {x%s\n\r", flag_string(book_flags, BOOK(obj)->flags));
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WOpen Page        {B]:  {x%d\n\r", BOOK(obj)->open_page);
		add_buf(buffer, buf);

		sprintf(buf, "{B[{WPages            {B]:  {x%d\n\r", list_size(BOOK(obj)->pages));
		add_buf(buffer, buf);
		if (list_size(BOOK(obj)->pages) > 0)
		{
			BOOK_PAGE *page;
			iterator_start(&it, BOOK(obj)->pages);
			while((page = (BOOK_PAGE *)iterator_nextdata(&it)))
			{
				sprintf(buf, "  {BPage: {x%d\n\r", page->page_no);
				add_buf(buffer, buf);

				sprintf(buf, "    {CTitle: {x%s\n\r", page->title);
				add_buf(buffer, buf);

				sprintf(buf, "    {CText:{x\n\r%s\n\r", string_indent(page->text, 5));
				add_buf(buffer, buf);
			}
			iterator_stop(&it);
		}

		if( BOOK(obj)->lock )
			print_lock_state(BOOK(obj)->lock, buffer, "");
	}

	if (IS_PAGE(obj))
	{
		sprintf(buf, "\n\r{GPage: {x%d\n\r", PAGE(obj)->page_no);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WTitle            {B]:  {x%s\n\r", PAGE(obj)->title);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WText             {B]:{x\n\r%s\n\r", PAGE(obj)->text);
		add_buf(buffer, buf);

		OBJ_INDEX_DATA *book = get_obj_index_auid(PAGE(obj)->book.auid, PAGE(obj)->book.vnum);
		sprintf(buf, "{B[{WOriginal Book    {B]:  {x%s\n\r", book ? book->short_descr : "none");
		add_buf(buffer, buf);
	}

	if (IS_CART(obj))
	{
		CART_DATA *cart = CART(obj);
		add_buf(buffer, "\n\r{GCart:{x\n\r");
		sprintf(buf, "{B[{WFlags            {B]:  {x%s\n\r", flag_string(cart_flags, cart->flags));
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WMinimum Strength {B]:  {x%d\n\r", cart->min_strength);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WMove Delay       {B]:  {x%d\n\r", cart->move_delay);
		add_buf(buffer, buf);
	}

	if (IS_COMPASS(obj))
	{
		COMPASS_DATA *compass = COMPASS(obj);

		add_buf(buffer, "\n\r{GCompass:{x\n\r");
		sprintf(buf, "{B[{WAccuracy         {B]:  {x%d{B%%{x\n\r", compass->accuracy);
		add_buf(buffer, buf);

		if (compass->wuid > 0 && compass->x >= 0 && compass->y >= 0)
		{
			WILDS_DATA *wilds = get_wilds_from_uid(NULL, compass->wuid);

			if (IS_VALID(wilds))
				sprintf(buf, "{B[{WTarget           {B]:  {x%s{B ({x%ld{B) at ({x%ld{B,{x%ld{B){x\n\r", wilds->name, wilds->uid, compass->x, compass->y);
			else
				sprintf(buf, "{B[{WTarget           {B]:  {D-invalid-{B ({D???{B) at ({x%ld{B,{x%ld{B){x\n\r", compass->x, compass->y);
			add_buf(buffer, buf);
		}
	}

	if (IS_CONTAINER(obj))
	{
		add_buf(buffer, "\n\r{GContainer:{x\n\r");
		sprintf(buf, "{B[{WName             {B]:  {x%s\n\r", CONTAINER(obj)->name);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WShort Description{B]:  {x%s\n\r", CONTAINER(obj)->short_descr);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFlags            {B]:  {x%s\n\r", flag_string(container_flags, CONTAINER(obj)->flags));
		add_buf(buffer, buf);
		if (CONTAINER(obj)->max_weight < 0)
			sprintf(buf, "{B[{WMax Weight       {B]:  {xUnlimited\n\r");
		else
			sprintf(buf, "{B[{WMax Weight       {B]:  {x%d\n\r", CONTAINER(obj)->max_weight);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WWeight Multiplier{B]:  {x%d\n\r", CONTAINER(obj)->weight_multiplier);
		add_buf(buffer, buf);

		if (CONTAINER(obj)->max_volume < 0)
			sprintf(buf, "{B[{WMax Volume       {B]:  {xUnlimited\n\r");
		else
			sprintf(buf, "{B[{WMax Volume       {B]:  {x%d\n\r", CONTAINER(obj)->max_volume);
		add_buf(buffer, buf);

		if (list_size(CONTAINER(obj)->whitelist) > 0)
		{
			add_buf(buffer, " {x[{WItem Type Whitelist:{x]\n\r");
			CONTAINER_FILTER *filter;

			sprintf(buf, "   {x%-6s %-15s %-15s\n\r", "Number", "Item Type", "Sub Type");
			add_buf(buffer, buf);
			sprintf(buf, "   {x%-6s %-15s %-15s\n\r", "------", "---------", "--------");
			add_buf(buffer, buf);

			int cnt = 0;
			iterator_start(&it, CONTAINER(obj)->whitelist);
			while((filter = (CONTAINER_FILTER *)iterator_nextdata(&it)))
			{
				char *subtype = "none";
				if (filter->item_type == ITEM_WEAPON && filter->sub_type >= 0)
				{
					subtype = flag_string(weapon_class, filter->sub_type);
				}

				sprintf(buf, "   {W%-6d %-15s %-15s{x\n\r", ++cnt,
					flag_string(type_flags, filter->item_type),
					subtype);
				add_buf(buffer, buf);
			}
			iterator_stop(&it);
		}

		if (list_size(CONTAINER(obj)->blacklist) > 0)
		{
			add_buf(buffer, " {x[{DItem Type Blacklist:{x]\n\r");
			CONTAINER_FILTER *filter;

			sprintf(buf, "   {x%-6s %-15s %-15s\n\r", "Number", "Item Type", "Sub Type");
			add_buf(buffer, buf);
			sprintf(buf, "   {x%-6s %-15s %-15s\n\r", "------", "---------", "--------");
			add_buf(buffer, buf);

			int cnt = 0;
			iterator_start(&it, CONTAINER(obj)->blacklist);
			while((filter = (CONTAINER_FILTER *)iterator_nextdata(&it)))
			{
				char *subtype = "none";
				if (filter->item_type == ITEM_WEAPON && filter->sub_type >= 0)
				{
					subtype = flag_string(weapon_class, filter->sub_type);
				}

				sprintf(buf, "   {D%-6d %-15s %-15s{x\n\r", ++cnt,
					flag_string(type_flags, filter->item_type),
					subtype);
				add_buf(buffer, buf);
			}
			iterator_stop(&it);
		}

		if( CONTAINER(obj)->lock )
			print_lock_state(CONTAINER(obj)->lock, buffer, "");
	}

	if (IS_CORPSE(obj))
	{
		CORPSE_DATA *corpse = CORPSE(obj);
		sprintf(buf, "\n\r{G%s Corpse:{x\n\r", (corpse->player?"Player":"Mobile"));
		add_buf(buffer, buf);

		if (IS_VALID(corpse->type))
			sprintf(buf, "{B[{WType             {B]:  {x%s\n\r", corpse->type->name);
		else
			sprintf(buf, "{B[{WType             {B]:  {Dnone{x\n\r");
		add_buf(buffer, buf);
		if (IS_VALID(corpse->race))
			sprintf(buf, "{B[{WRace             {B]:  {x%s\n\r", corpse->race->name);
		else
			sprintf(buf, "{B[{WRace             {B]:  {Dnone{x\n\r");
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFlags            {B]:  {x%s\n\r", flag_string(corpse_object_flags, corpse->flags));
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WResurrect Chance {B]:  {x%d%%\n\r", corpse->resurrect);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WAnimate Chance   {B]:  {x%d%%\n\r", corpse->animate);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WParts            {B]:  {x%s\n\r", flag_string(part_flags, corpse->parts));
		add_buf(buffer, buf);

		if (corpse->mobile)
			sprintf(buf, "{B[{WMobile           {B]:  {x%s\n\r", widevnum_string_mobile(corpse->mobile, obj->area));
		else
			sprintf(buf, "{B[{WMobile           {B]:  {Dnone{x\n\r");
		add_buf(buffer, buf);
	}

	if (IS_FLUID_CON(obj))
	{
		FLUID_CONTAINER_DATA *fluid = FLUID_CON(obj);
		add_buf(buffer, "\n\r{GFluid Container:{x\n\r");

		sprintf(buf, "{B[{WName             {B]:  {x%s\n\r", fluid->name);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WShort Description{B]:  {x%s\n\r", fluid->short_descr);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFlags            {B]:  {x%s\n\r", flag_string(fluid_con_flags, fluid->flags));
		add_buf(buffer, buf);

		if (IS_VALID(fluid->liquid))
		{
			LIQUID *liquid = fluid->liquid;
			sprintf(buf, "{B[{wLiquid           {B]:  {x%s\n\r", liquid->name);
			add_buf(buffer, buf);

			// Add liquid stats for convenience
			sprintf(buf, "   {CColor:     {x%s\n\r", liquid->color);
			add_buf(buffer, buf);
			sprintf(buf, "   {CFlammable: %s{x\n\r", liquid->flammable ? "{WYes" : "{DNo");
			add_buf(buffer, buf);
			sprintf(buf, "   {CProof:     {x%d\n\r", liquid->proof);
			add_buf(buffer, buf);
			sprintf(buf, "   {CFullness:  {x%d\n\r", liquid->full);
			add_buf(buffer, buf);
			sprintf(buf, "   {CThirst:    {x%d\n\r", liquid->thirst);
			add_buf(buffer, buf);
			sprintf(buf, "   {CHunger:    {x%d\n\r", liquid->hunger);
			add_buf(buffer, buf);
			sprintf(buf, "   {CFuel:      {x%d{C unit%s per {x%d{C tick%s\n\r", liquid->fuel_unit, ((liquid->fuel_unit == 1) ? "" : "s"), liquid->fuel_duration, ((liquid->fuel_duration == 1) ? "" : "s"));
			add_buf(buffer, buf);
			sprintf(buf, "   {CMax Mana:  {x%d{C\n\r", liquid->max_mana);
			add_buf(buffer, buf);
		}
		else
			add_buf(buffer, "{B[{WLiquid           {B]:  {xnone\n\r");

		if (fluid->capacity < 0)
			add_buf(buffer, "{B[{WCapacity         {B]:  {Wunlimited{x\n\r");
		else
		{
			sprintf(buf, "{B[{WCapacity         {B]:  {x%d{B / {x%d\n\r", fluid->amount, fluid->capacity);
			add_buf(buffer, buf);
		}

		// Leave this "none" (0) to make this a normal drink container.
		if (fluid->refill_rate > 0)
		{
			sprintf(buf, "{B[{wRefill Rate      {B]:  {x%d{B per tick{x\n\r", fluid->refill_rate);
			add_buf(buffer, buf);
		}
		else
			add_buf(buffer, "{B[{WRefill Rate      {B]:  {xnone\n\r");

		if (fluid->poison > 0)
		{
			sprintf(buf, "{B[{wPoison           {B]:  {x%d{B%% per drink{x\n\r", fluid->poison);
			add_buf(buffer, buf);
		}
		else
			add_buf(buffer, "{B[{WPoison           {B]:  {xnone\n\r");

		if (fluid->poison_rate > 0)
		{
			sprintf(buf,    "{B[{wPoison Rate      {B]:  {x%d{B per tick{x\n\r", fluid->poison_rate);
			add_buf(buffer, buf);
		}
		else
			add_buf(buffer, "{B[{WPoison Rate      {B]:  {xnone\n\r");

		if( fluid->lock )
			print_lock_state(fluid->lock, buffer, "");

		if (list_size(fluid->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, fluid->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_FOOD(obj))
	{
		add_buf(buffer, "\n\r{GFood:{x\n\r");
		sprintf(buf, "{B[{WHunger  {B]:  {x%d\n\r", FOOD(obj)->hunger);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFullness{B]:  {x%d\n\r", FOOD(obj)->full);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WPoisoned{B]:  {x%d%%\n\r", FOOD(obj)->poison);
		add_buf(buffer, buf);

		if (list_size(FOOD(obj)->buffs) > 0)
		{
			int cnt = 1;
			FOOD_BUFF_DATA *buff;

			add_buf(buffer, " {c[{CFood Buffs:{c]{x\n\r");

			sprintf(buf, "  {C%-6s %-5s %-15s %-15s %-15s %-10s %s{x\n\r", "Number", "Level", "Where", "Adds", "Modifier", "Duration", "Bits");
			add_buf(buffer, buf);

			sprintf(buf, "  {c%-6s %-5s %-15s %-15s %-15s %-10s %s{x\n\r", "------", "-----", "-------", "-------", "--------", "----------", "--------------------");
			add_buf(buffer, buf);

			iterator_start(&it, FOOD(obj)->buffs);
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
					sprintf(buf, "  {C%-6d %-5s %-15s %-15s %-15d %-10s %s{x\n\r", cnt++,
						level,
						flag_string(food_buff_types, buff->where),
						flag_string(apply_flags, buff->location),
						buff->modifier,
						duration,
						bitvector_string(2, buff->bitvector, affect_flags, buff->bitvector2, affect2_flags));
				}
				else
				{
					sprintf(buf, "  {C%-6d %-5s %-15s %-15s %-15d %-10s %s{x\n\r", cnt++,
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
		}
	}

	if (IS_FURNITURE(obj))
	{
		add_buf(buffer, "\n\r{GFurniture:{x\n\r");
		sprintf(buf, "{B[{WFlags           {B]:  {x%s\n\r", flag_string(furniture_flags, FURNITURE(obj)->flags));
		add_buf(buffer, buf);

		if (FURNITURE(obj)->main_compartment > 0)
		{
			FURNITURE_COMPARTMENT *cmpt = (FURNITURE_COMPARTMENT *)list_nthdata(FURNITURE(obj)->compartments, FURNITURE(obj)->main_compartment);

			sprintf(buf, "{B[{WMain Compartment{B]:  {x%d ({W%s{x)\n\r", FURNITURE(obj)->main_compartment, cmpt->short_descr);
		}
		else
			sprintf(buf, "{B[{WMain Compartment{B]:  {xinvalid\n\r");
		add_buf(buffer, buf);
	
		add_buf(buffer, " {CCompartments:{x\n\r");
		if (list_size(FURNITURE(obj)->compartments) > 0)
		{
			FURNITURE_COMPARTMENT *compartment;
			int cnt = 1;

			iterator_start(&it, FURNITURE(obj)->compartments);
			while((compartment = (FURNITURE_COMPARTMENT *)iterator_nextdata(&it)))
			{
				if (cnt == FURNITURE(obj)->main_compartment)
					sprintf(buf, " {c[{C%4d{c] {x%s {G[MAIN COMPARTMENT]{x\n\r", cnt, fix_string(compartment->name));
				else
					sprintf(buf, " {c[{C%4d{c] {x%s\n\r", cnt, fix_string(compartment->name));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WShort Desc  {C]:  {x%s\n\r", fix_string(compartment->short_descr));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WDescription {C]:{x\n\r%s\n\r", compartment->description);
				add_buf(buffer, buf);

				sprintf(buf, "    {C[{WFlags       {C]:  {x%s\n\r", flag_string(compartment_flags, compartment->flags));
				add_buf(buffer, buf);
				if (compartment->max_occupants < 0)
					sprintf(buf, "    {C[{WOccupants   {C]:  {xUnlimited\n\r");
				else
					sprintf(buf, "    {C[{WOccupants   {C]:  {x%d maximum\n\r", compartment->max_occupants);
				add_buf(buffer, buf);
				if (compartment->max_weight < 0)
					sprintf(buf, "    {C[{WWeight      {C]:  {xUnlimited\n\r");
				else
					sprintf(buf, "    {C[{WWeight      {C]:  {x%dkg maximum\n\r", compartment->max_weight);
				add_buf(buffer, buf);

				sprintf(buf, "    {C[{WStanding    {C]:  {x%s\n\r", flag_string(furniture_action_flags, compartment->standing));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WHanging     {C]:  {x%s\n\r", flag_string(furniture_action_flags, compartment->hanging));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WSitting     {C]:  {x%s\n\r", flag_string(furniture_action_flags, compartment->sitting));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WResting     {C]:  {x%s\n\r", flag_string(furniture_action_flags, compartment->resting));
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WSleeping    {C]:  {x%s\n\r", flag_string(furniture_action_flags, compartment->sleeping));
				add_buf(buffer, buf);

				sprintf(buf, "    {C[{WHealth Regen{C]:  {x%d\n\r", compartment->health_regen);
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WMana Regen  {C]:  {x%d\n\r", compartment->mana_regen);
				add_buf(buffer, buf);
				sprintf(buf, "    {C[{WMove Regen  {C]:  {x%d\n\r", compartment->move_regen);
				add_buf(buffer, buf);

				if( compartment->lock)
					print_lock_state(compartment->lock, buffer, "   ");

				cnt++;
			}
			iterator_stop(&it);
		}
		else
		{
			add_buf(buffer, "    {Cnone{x\n\r");
		}
	}

	if (IS_INK(obj))
	{
		add_buf(buffer, "\n\r{GInk:{x\n\r");

		for(int i = 0; i < MAX_INK_TYPES; i++)
		{
			sprintf(buf, "{B[{WType %d  {B]:  {x%s{B ({x%d{B){x\n\r", i + 1,
				flag_string(catalyst_types, INK(obj)->types[i]),
				INK(obj)->amounts[i]);
			add_buf(buffer, buf);
		}
	}

	if (IS_INSTRUMENT(obj))
	{
		add_buf(buffer, "\n\r{GInstrument:{x\n\r");

		sprintf(buf, "{B[{WType    {B]:  {x%s\n\r", flag_string(instrument_types, INSTRUMENT(obj)->type));
		add_buf(buffer, buf);

		sprintf(buf, "{B[{WFlags   {B]:  {x%s\n\r", flag_string(instrument_flags, INSTRUMENT(obj)->flags));
		add_buf(buffer, buf);

		if (INSTRUMENT(obj)->beats_min < INSTRUMENT(obj)->beats_max)
			sprintf(buf, "{B[{WBeats   {B]:  {x%d{B%% to {x%d{B%%{x\n\r", INSTRUMENT(obj)->beats_min, INSTRUMENT(obj)->beats_max);
		else
			sprintf(buf, "{B[{WBeats   {B]:  {x%d{B%%{x\n\r", INSTRUMENT(obj)->beats_min);
		add_buf(buffer, buf);

		if (INSTRUMENT(obj)->mana_min < INSTRUMENT(obj)->mana_max)
			sprintf(buf, "{B[{WMana    {B]:  {x%d{B%% to {x%d{B%%{x\n\r", INSTRUMENT(obj)->mana_min, INSTRUMENT(obj)->mana_max);
		else
			sprintf(buf, "{B[{WMana    {B]:  {x%d{B%%{x\n\r", INSTRUMENT(obj)->mana_min);
		add_buf(buffer, buf);

		add_buf(buffer, "{BReservoirs:{x\n\r");
		for(int i = 0; i < INSTRUMENT_MAX_CATALYSTS; i++)
		{
			if (INSTRUMENT(obj)->reservoirs[i].type != CATALYST_NONE)
			{
				sprintf(buf, "  {W%d{B) {x%s {B({x%s{B}){x\n\r", i + 1,
					flag_string(catalyst_types, INSTRUMENT(obj)->reservoirs[i].type),
					(INSTRUMENT(obj)->reservoirs[i].capacity > 0) ?
						formatf("%d {B/{x %d", INSTRUMENT(obj)->reservoirs[i].amount, INSTRUMENT(obj)->reservoirs[i].capacity) :
						"none");
			}
			else
				sprintf(buf, "  {W%d{B) {xnone\n\r", i + 1);
			add_buf(buffer, buf);
		}
	}

	if (IS_JEWELRY(obj))
	{
		add_buf(buffer, "\n\r{GJewelry:{x\n\r");

		sprintf(buf, "{B[{WMaximum Mana {B]:  {x%d\n\r", JEWELRY(obj)->max_mana);
		add_buf(buffer, buf);

		if (list_size(JEWELRY(obj)->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, JEWELRY(obj)->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_LIGHT(obj))
	{
		add_buf(buffer, "\n\r{GLight:{x\n\r");
		sprintf(buf, "{B[{WFlags   {B]:  {x%s\n\r", flag_string(light_flags, LIGHT(obj)->flags));
		add_buf(buffer, buf);
		if (LIGHT(obj)->duration < 0)
			sprintf(buf, "{B[{WDuration{B]:  {WInfinite{x\n\r");
		else
			sprintf(buf, "{B[{WDuration{B]:  {x%d\n\r", LIGHT(obj)->duration);
		add_buf(buffer, buf);
	}

	if (IS_MAP(obj))
	{
		MAP_DATA *map = MAP(obj);

		add_buf(buffer, "\n\r{GMap:{x\n\r");

		if (map->wuid > 0 && map->x >= 0 && map->y >= 0)
		{
			WILDS_DATA *wilds = get_wilds_from_uid(NULL, map->wuid);

			if (IS_VALID(wilds))
				sprintf(buf, "{B[{WTarget           {B]:  {x%s{B ({x%ld{B) at ({x%ld{B,{x%ld{B){x\n\r", wilds->name, wilds->uid, map->x, map->y);
			else
				sprintf(buf, "{B[{WTarget           {B]:  {D-invalid-{B ({D???{B) at ({x%ld{B,{x%ld{B){x\n\r", map->x, map->y);
		}
		else
			sprintf(buf, "{B[{WTarget           {B]:  {xnone\n\r");
		add_buf(buffer, buf);

		if (list_size(map->waypoints) > 0)
		{
			int cnt = 0;
			ITERATOR wit;
			WAYPOINT_DATA *wp;
			WILDS_DATA *wilds;

			add_buf(buffer, "{BCartographer Waypoints:{x\n\r\n\r");
			add_buf(buffer, "{B     [     Wilderness     ] [ South ] [  East ] [        Name        ]{x\n\r");
			add_buf(buffer, "{B======================================================================={x\n\r");

			iterator_start(&wit, map->waypoints);
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
		}
	}

	if (IS_MIST(obj))
	{
		MIST_DATA *mist = MIST(obj);
		add_buf(buffer, "\n\r{GMist:{x\n\r");

	    sprintf(buf, "{B[{WObscure Mobiles  {B]:  {x%d{B%%{x\n\r", mist->obscure_mobs);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{WObscure Objects  {B]:  {x%d{B%%{x\n\r", mist->obscure_objs);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{WObscure Room     {B]:  {x%d{B%%{x\n\r", mist->obscure_room);
	    add_buf(buffer, buf);

	    sprintf(buf, "{B[{CIcy              {B]:  {x%d{B%%{x\n\r", mist->icy);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{RFiery            {B]:  {x%d{B%%{x\n\r", mist->fiery);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{GAcidic           {B]:  {x%d{B%%{x\n\r", mist->acidic);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{YStink            {B]:  {x%d{B%%{x\n\r", mist->stink);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{DWither           {B]:  {x%d{B%%{x\n\r", mist->wither);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{GToxic Fumes      {B]:  {x%d{B%%{x\n\r", mist->toxic);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{YShocking         {B]:  {x%d{B%%{x\n\r", mist->shock);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{WFog              {B]:  {x%d{B%%{x\n\r", mist->fog);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[{WSleep            {B]:  {x%d{B%%{x\n\r", mist->sleep);
	    add_buf(buffer, buf);
	}

	if (IS_MONEY(obj))
	{
		add_buf(buffer, "\n\r{GMoney:{x\n\r");
		sprintf(buf, "{B[{WSilver  {B]:  {x%d\n\r", MONEY(obj)->silver);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WGold    {B]:  {x%d\n\r", MONEY(obj)->gold);
		add_buf(buffer, buf);
	}

	if (IS_PORTAL(obj))
	{
		// This morphs due to the portal type
		print_obj_portal_values(obj, buffer);

		if( PORTAL(obj)->lock )
			print_lock_state(PORTAL(obj)->lock, buffer, "");


		if (list_size(PORTAL(obj)->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, PORTAL(obj)->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_SCROLL(obj))
	{
		SCROLL_DATA *scroll = SCROLL(obj);

		add_buf(buffer, "\n\r{GScroll:{x\n\r");

		sprintf(buf, "{B[{WMaximum Mana {B]:  {x%d\n\r", scroll->max_mana);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFlags        {B]:  {x%s\n\r", flag_string(scroll_flags, scroll->flags));
		add_buf(buffer, buf);

		if (list_size(scroll->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, scroll->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_SEXTANT(obj))
	{
		SEXTANT_DATA *sextant = SEXTANT(obj);

		add_buf(buffer, "\n\r{GSextant:{x\n\r");
		sprintf(buf, "{B[{WAccuracy         {B]:  {x%d{B%%{x\n\r", sextant->accuracy);
		add_buf(buffer, buf);
	}

	if (IS_TATTOO(obj))
	{
		TATTOO_DATA *tattoo = TATTOO(obj);

		add_buf(buffer, "\n\r{GTattoo:{x\n\r");
		if (tattoo->touches < 0)
			sprintf(buf, "{B[{WTouches      {B]:  {Wunlimited{x\n\r");
		else
			sprintf(buf, "{B[{WTouches      {B]:  {x%d\n\r", tattoo->touches);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFading Chance{B]:  {x%d%%\n\r", tattoo->fading_chance);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WFading Rate  {B]:  {x+%d%% per touch\n\r", tattoo->fading_rate);
		add_buf(buffer, buf);

		if (list_size(tattoo->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, tattoo->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_TELESCOPE(obj))
	{
		TELESCOPE_DATA *telescope = TELESCOPE(obj);

		add_buf(buffer, "\n\r{GTelescope:{x\n\r");
		sprintf(buf, "{B[{WDistance     {B]:  {x%d\n\r", telescope->distance);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WMin Distance {B]:  {x%d\n\r", telescope->min_distance);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WMax Distance {B]:  {x%d\n\r", telescope->max_distance);
		add_buf(buffer, buf);
		sprintf(buf, "{B[{WBonus View   {B]:  {x%d\n\r", telescope->bonus_view);
		add_buf(buffer, buf);
		if (telescope->heading < 0)
			sprintf(buf, "{B[{WHeading      {B]:  {xnone\n\r");
		else
			sprintf(buf, "{B[{WHeading      {B]:  {x%d{B degrees (when stationary)\n\r", telescope->heading);
		add_buf(buffer, buf);
	}

	if (IS_WAND(obj))
	{
		WAND_DATA *wand = WAND(obj);

		add_buf(buffer, "\n\r{GWand:{x\n\r");
		sprintf(buf, "{B[{WMaximum Mana {B]:  {x%d{B (used for imbuing){x\n\r", wand->max_mana);
		add_buf(buffer, buf);

		if (wand->max_charges < 0)
			sprintf(buf, "{B[{WCharges      {B]:  {Wunlimited{x\n\r");
		else
			sprintf(buf, "{B[{WCharges      {B]:  {x%d{B / {x%d\n\r", wand->charges, wand->max_charges);
		add_buf(buffer, buf);

		if (wand->recharge_time > 0)
			sprintf(buf, "{B[{WRecharging   {B]:  1 charge per {x%d{B tick%s{x\n\r", wand->recharge_time, ((wand->recharge_time == 1)?"":"s"));
		else
			sprintf(buf, "{B[{WRecharging   {B]:  none{x\n\r");
		add_buf(buffer, buf);

		if (list_size(wand->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, wand->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

	if (IS_WEAPON(obj))
	{
		WEAPON_DATA *weapon = WEAPON(obj);

		add_buf(buffer, "\n\r{GWeapon:{x\n\r");
		sprintf(buf, "{B[{WClass        {B]:  {x%s\n\r", flag_string(weapon_class, weapon->weapon_class));
		add_buf(buffer, buf);

		for(int a = 0; a < MAX_ATTACK_POINTS; a++)
		{
			if (weapon->attacks[a].type >= 0)
			{
				int dt = weapon->attacks[a].type;
				DICE_DATA dice = weapon->attacks[a].damage;
				sprintf(buf, "{B[{WAttack %-2d    {B]:  {x%s {B({x%s{B), Damage: %s{B, Flags: {x%s\n\r", a+1,
					attack_table[dt].noun,
					flag_string(damage_classes, attack_table[dt].damage),
					((dice.bonus > 0) ?
						formatf("{x%d{Bd{x%d{B+{x%d", dice.number, dice.size, dice.bonus) :
						formatf("{x%d{Bd{x%d", dice.number, dice.size)),
					flag_string(weapon_type2, weapon->attacks[a].flags));
			}
			else
				sprintf(buf, "{B[{WAttack %-2d    {B]:  {xnone\n\r", a+1);
			add_buf(buffer, buf);
		}

		sprintf(buf, "{B[{WMaximum Mana {B]:  {x%d{B (used for imbuing){x\n\r", weapon->max_mana);
		add_buf(buffer, buf);

		if (weapon->max_charges < 0)
			sprintf(buf, "{B[{WCharges      {B]:  {Wunlimited{x\n\r");
		else
			sprintf(buf, "{B[{WCharges      {B]:  {x%d{B / {x%d\n\r", weapon->charges, weapon->max_charges);
		add_buf(buffer, buf);

		if (weapon->recharge_time > 0)
			sprintf(buf, "{B[{WRecharging   {B]:  1 charge per {x%d{B tick%s{x\n\r", weapon->recharge_time, ((weapon->recharge_time == 1)?"":"s"));
		else
			sprintf(buf, "{B[{WRecharging   {B]:  none{x\n\r");
		add_buf(buffer, buf);

		sprintf(buf, "{B[{WRange        {B]:  {x%d\n\r", weapon->range);
		add_buf(buffer, buf);

		sprintf(buf, "{B[{WAmmo         {B]:  {x%s\n\r", flag_string(ammo_types, weapon->ammo));
		add_buf(buffer, buf);

		if (list_size(weapon->spells) > 0)
		{
			int cnt = 0;

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "Number", "Spell", "Level", "Random");
			add_buf(buffer, buf);

			sprintf(buf, "{g%-6s %-20s %-10s %-6s{x\n\r", "------", "-----", "-----", "------");
			add_buf(buffer, buf);

			ITERATOR sit;
			SPELL_DATA *spell;
			iterator_start(&sit, weapon->spells);
			while((spell = (SPELL_DATA *)iterator_nextdata(&sit)))
			{
				sprintf(buf, "{B[{W%4d{B]{x %-20s %-10d %d%%\n\r",
					cnt,
					spell->skill->name, spell->level, spell->repop);
				buf[0] = UPPER(buf[0]);
				add_buf(buffer, buf);

				cnt++;
			}
		}
	}

    add_buf(buffer, "\n\r");

    switch(obj->item_type)
    {
	default:	// No values
	    break;

	/*
	case ITEM_WAND:
	case ITEM_STAFF:
            sprintf(buf,
		"{B[  {Wv0{B]{G Level:{x          [%ld]\n\r"
		"{B[  {Wv1{B]{G Charges Total:{x  [%ld]\n\r"
		"{B[  {Wv2{B]{G Charges Left:{x   [%ld]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2]);
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_FURNITURE:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Max people:{x      [%ld]\n\r"
	        "{B[  {Wv1{B]{G Max weight:{x      [%ld]\n\r"
	        "{B[  {Wv2{B]{G Furniture Flags:{x %s\n\r"
	        "{B[  {Wv3{B]{G Heal bonus:{x      [%ld]\n\r"
	        "{B[  {Wv4{B]{G Mana bonus:{x      [%ld]\n\r"
		"{B[  {Wv5{B]{G Move bonus:{x      [%ld]\n\r",
	        obj->value[0],
	        obj->value[1],
	        flag_string(furniture_flags, obj->value[2]),
	        obj->value[3],
	        obj->value[4],
		obj->value[5]);
	    add_buf(buffer, buf);
	    break;
		*/

		/*
	case ITEM_HERB:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Type:{x            [%s]\n\r"
		"{B[  {Wv1{B]{G Healing:{x         [%ld%%]\n\r"
		"{B[  {Wv2{B]{G Regenerative:{x    [%ld%%]\n\r"
		"{B[  {Wv3{B]{G Refreshing:{x      [%ld%%]\n\r"
		"{B[  {Wv4{B]{G Immunity:{x        [%s]\n\r"
		"{B[  {Wv5{B]{G Resistance:{x      [%s]\n\r"
		"{B[  {Wv6{B]{G Vulnerability:{x   [%s]\n\r"
		"{B[  {Wv7{B]{G Spell:{x           [%s]\n\r",
		herb_table[obj->value[0]].name,
		obj->value[1],
		obj->value[2],
		obj->value[3],
		flag_string(imm_flags, obj->value[4]),
		flag_string(res_flags, obj->value[5]),
		flag_string(vuln_flags, obj->value[6]),
		skill_table[obj->value[7]].name);

	    add_buf(buffer, buf);
	    break;
		*/

	/*
	case ITEM_BLANK_SCROLL:
		sprintf(buf,
				"{B[  {Wv0{B]{G Maximum Mana:{x           [{%c%ld{x]\n\r",
				((obj->value[0] > 0) ? 'x' : 'Y'),
				((obj->value[0] > 0) ? obj->value[0] : 200));
		add_buf(buffer, buf);
		break;
	*/

	//case ITEM_SCROLL:
	//case ITEM_POTION:
	//case ITEM_PILL:
	//    break;

	/*
	case ITEM_TATTOO:
            sprintf(buf,
            		"{B[  {Wv0{B]{G Touches:{x                [%ld]\n\r"
            		"{B[  {Wv1{B]{G Chance of Fading:{x       [%ld]\n\r",
            		obj->value[0],obj->value[1]);
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_INK:
            sprintf(buf, "{B[  {Wv0{B]{G Type 1:{x                 [%s]\n\r", flag_string(catalyst_types, obj->value[0]));
	    add_buf(buffer, buf);
            sprintf(buf, "{B[  {Wv1{B]{G Type 2:{x                 [%s]\n\r", flag_string(catalyst_types, obj->value[1]));
	    add_buf(buffer, buf);
            sprintf(buf, "{B[  {Wv2{B]{G Type 3:{x                 [%s]\n\r", flag_string(catalyst_types, obj->value[2]));
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_SEXTANT:
            sprintf(buf,
		"{B[  {Wv0{B]{G Percentage of working:{x  [%ld]\n\r",
		obj->value[0]);
	    add_buf(buffer, buf);
	    break;
		*/

	case ITEM_SEED:
            sprintf(buf,
		"{B[  {Wv0{B]{G Time before growth:{x     [%ld]\n\r"
		"{B[  {Wv1{B]{G Turns into object vnum:{x [%ld]\n\r",
		obj->value[0],
		obj->value[1]);
	    add_buf(buffer, buf);
	    break;

	/*
	case ITEM_ARMOUR:
	    sprintf(buf,
		"{B[  {Wv0{B] {GAc pierce       {x[%ld]\n\r"
		"{B[  {Wv1{B] {GAc bash         {x[%ld]\n\r"
		"{B[  {Wv2{B] {GAc slash        {x[%ld]\n\r"
		"{B[  {Wv3{B] {GAc exotic       {x[%ld]\n\r"
		"{B[  {Wv4{B] {GArmour strength  {x%s\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
		obj->value[3],
		armour_strength_table[obj->value[4]].name);
	    add_buf(buffer, buf);
	    break;
	*/
	case ITEM_ARTIFACT:
	    break;

	/*
	case ITEM_RANGED_WEAPON:
            sprintf(buf, "{B[  {Wv0{B]{G Ranged Weapon class:{x   %s\n\r",
		     flag_string(ranged_weapon_class, obj->value[0]));
	    add_buf(buffer, buf);

	    sprintf(buf, "{B[  {Wv1{B]{G Number of dice:{x [%ld]\n\r", obj->value[1]);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[  {Wv2{B]{G Type of dice:{x   [%ld]\n\r", obj->value[2]);
	    add_buf(buffer, buf);

	    sprintf(buf, "{B[  {Wv3{B]{G Projectile Distance:{x [%ld]\n\r", obj->value[3]);
	    add_buf(buffer, buf);
	    break;
	*/
/*
	case ITEM_WEAPON:
            sprintf(buf, "{B[  {Wv0{B]{G Weapon class:{x   %s\n\r",
		     flag_string(weapon_class, obj->value[0]));
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[  {Wv1{B]{G Number of dice:{x [%ld]\n\r", obj->value[1]);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[  {Wv2{B]{G Type of dice:{x   [%ld]\n\r", obj->value[2]);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[  {Wv3{B]{G Type:{x           %s\n\r",
		    attack_table[obj->value[3]].name);
	    add_buf(buffer, buf);
 	    sprintf(buf, "{B[  {Wv4{B]{G Special type:{x   %s\n\r",
		     flag_string(weapon_type2,  obj->value[4]));
	    add_buf(buffer, buf);
	    break;
*/
	case ITEM_SHIP:
	    sprintf(buf,
		"{B[  {Wv0{B]{G Weight:{x     [%ld kg]\n\r"
		"{B[  {Wv1{B]{G Move delay:{x [%ld]\n\r"
		"{B[  {Wv2{B]{G Min Crew:{x   [%ld]\n\r"
		"{B[  {Wv3{B]{G Capacity:{x   [%ld]\n\r"
		"{B[  {Wv4{B]{G Max Crew:{x   [%ld]\n\r"
		"{B[  {Wv5{B]{G First Room:{x [%ld]\n\r"
		"{B[  {Wv6{B]{G Hit Points:{x [%ld]\n\r"
		"{B[  {Wv7{B]{G Max Guns:{x   [%ld]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
                obj->value[3],
                obj->value[4],
                obj->value[5],
                obj->value[6],
                obj->value[7]);
	    add_buf(buffer, buf);
	    break;

	/*
	case ITEM_CART:
	    sprintf(buf,
		"{B[  {Wv0{B]{G Weight:{x     [%ld kg]\n\r"
		"{B[  {Wv1{B]{G Move delay:{x [%ld]\n\r"
		"{B[  {Wv2{B]{G Strength:{x   [%ld]\n\r"
		"{B[  {Wv3{B]{G Capacity:{x    [%ld]\n\r"
		"{B[  {Wv4{B]{G Weight Mult:{x [%ld]\n\r",
		obj->value[0],
		obj->value[1],
		obj->value[2],
                obj->value[3],
                obj->value[4]);
	    add_buf(buffer, buf);
	    break;
	*/

	case ITEM_TRADE_TYPE:
	    sprintf(buf,
		"{B[  {Wv0{B]{G Trade Type:{x     [%s]\n\r",
		trade_table[ obj->value[0] ].name);
	    add_buf(buffer, buf);
	    break;

	/*
	case ITEM_CONTAINER:
	    sprintf(buf,
		"{B[  {Wv0{B]{G Weight:{x     [%ld kg]\n\r"
		"{B[  {Wv1{B]{G Flags:{x      [%s]\n\r"
		"{B[  {Wv3{B]{G Capacity:{x    [%ld]\n\r"
		"{B[  {Wv4{B]{G Weight Mult:{x [%ld]\n\r",
		obj->value[0],
		flag_string(container_flags, obj->value[1]),
                obj->value[3],
                obj->value[4]);
	    add_buf(buffer, buf);
	    break;

	case ITEM_WEAPON_CONTAINER:
	    sprintf(buf,
		"{B[  {Wv0{B]{G Weight:{x     [%ld kg]\n\r"
		"{B[  {Wv1{B]{G Weapon Type:{x [%s]\n\r"
		"{B[  {Wv3{B]{G Capacity:{x   [%ld]\n\r"
		"{B[  {Wv4{B]{G Weight Mult:{x[%ld]\n\r",
		obj->value[0],
		flag_string(weapon_class, obj->value[1]),
                obj->value[3],
                obj->value[4]);
	    add_buf(buffer, buf);
	    break;

	case ITEM_DRINK_CON:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Liquid Total:{x [%ld]\n\r"
	        "{B[  {Wv1{B]{G Liquid Left:{x  [%ld]\n\r"
	        "{B[  {Wv2{B]{G Liquid:{x       %s\n\r"
	        "{B[  {Wv3{B]{G Poisoned:{x     %s\n\r",
	        obj->value[0],
	        obj->value[1],
	        liq_table[obj->value[2]].liq_name,
	        obj->value[3] != 0 ? "Yes" : "No");
	    add_buf(buffer, buf);
	    break;

	case ITEM_FOUNTAIN:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Liquid Total:{x [%ld]\n\r"
	        "{B[  {Wv1{B]{G Liquid Left:{x  [%ld]\n\r"
	        "{B[  {Wv2{B]{G Liquid:{x       %s\n\r"
	        "{B[  {Wv3{B]{G Poisoned:{x     %s\n\r"
			"{B[  {Wv4{B]{G Fill Rate:{x    [%ld]\n\r"
			"{B[  {Wv5{B]{G Poison Rate:{x  [%ld]\n\r",
	        obj->value[0],
	        obj->value[1],
	        liq_table[obj->value[2]].liq_name,
	        obj->value[3] != 0 ? "Yes" : "No",
			obj->value[4],
			obj->value[5]);
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_MIST:
	    sprintf(buf, "{B[  {Wv0{B]{G %%HideObjects:{x    [%ld]\n\r", obj->value[0]);
	    add_buf(buffer, buf);
	    sprintf(buf, "{B[  {Wv1{B]{G %%HideCharacters:{x [%ld]\n\r", obj->value[1]);
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_CORPSE_NPC:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Type:{x           %s\n\r"
	        "{B[  {Wv1{B]{G Resurrection:{x   %d%%\n\r"
	        "{B[  {Wv2{B]{G Animation:{x      %d%%\n\r"
	        "{B[  {Wv3{B]{G Body Parts:{x     %s\n\r"
	        "{B[  {Wv5{B]{G Mobile (vnum):{x  %d\n\r",
	        flag_string(corpse_types,obj->value[0]),
	        (int)obj->value[1],(int)obj->value[2],
	        flag_string(part_flags, obj->value[3]),
	        (int)obj->value[5]);
	    add_buf(buffer, buf);
	    break;
		*/
	/*
	case ITEM_INSTRUMENT:
	    sprintf(buf,
	        "{B[  {Wv0{B]{G Type:{x            %s\n\r"
	        "{B[  {Wv1{B]{G Flags:{x           %s\n\r"
	        "{B[  {Wv2{B]{G Min Time Factor:{x %ld%%\n\r"
	        "{B[  {Wv3{B]{G Max Time Factor:{x %ld%%\n\r",
	        flag_string(instrument_types, obj->value[0]),
	        flag_string(instrument_flags, obj->value[1]),
	        obj->value[2],obj->value[3]);
	    add_buf(buffer, buf);
	    break;
	*/
	/*
	case ITEM_BOOK:
	    sprintf(buf,
		"{B[  {Wv1{B]{G Flags:{x      [%s]\n\r",
		flag_string(container_flags, obj->value[1]));
	    add_buf(buffer, buf);
	    break;
	*/

	/*
	case ITEM_TELESCOPE:
		if( obj->value[4] < 0 )
			sprintf(buf,
				"{B[  {Wv0{B]{G Current Distance:{x  [%ld]\n\r"
				"{B[  {Wv1{B]{G Minimum Distance:{x  [%ld]\n\r"
				"{B[  {Wv2{B]{G Maximum Distance:{x  [%ld]\n\r"
				"{B[  {Wv3{B]{G Bonusview Size:{x    [%ld]\n\r"
				"{B[  {Wv4{B]{G Current Heading:{x   [none]\n\r",
					obj->value[0],
					obj->value[1],
					obj->value[2],
					obj->value[3]);
		else
			sprintf(buf,
				"{B[  {Wv0{B]{G Current Distance:{x  [%ld]\n\r"
				"{B[  {Wv1{B]{G Minimum Distance:{x  [%ld]\n\r"
				"{B[  {Wv2{B]{G Maximum Distance:{x  [%ld]\n\r"
				"{B[  {Wv3{B]{G Bonusview Size:{x    [%ld]\n\r"
				"{B[  {Wv4{B]{G Current Heading:{x   [%ld]\n\r",
					obj->value[0],
					obj->value[1],
					obj->value[2],
					obj->value[3],
					obj->value[4]);
	    add_buf(buffer, buf);
	    break;

	case ITEM_COMPASS:
		if( obj->value[1] > 0 )
		{
			WILDS_DATA *pWilds = get_wilds_from_uid(NULL,obj->value[1]);

			sprintf(buf,
				"{B[  {Wv0{B]{G Accuracy:{x      [%ld]\n\r"
				"{B[  {Wv1{B]{G Wilderness:{x    [%ld] %s\n\r"
				"{B[  {Wv2{B]{G X Coordinate:{x  [%ld]\n\r"
				"{B[  {Wv3{B]{G Y Coordinate:{x  [%ld]\n\r",
					obj->value[0],
					obj->value[1], (pWilds?pWilds->name:"???"),
					obj->value[2],
					obj->value[3]);
		}
		else
		{
			sprintf(buf,
				"{B[  {Wv0{B]{G Accuracy:{x      [%ld]\n\r"
				"{B[  {Wv1{B]{G Wilderness:{x    [none]\n\r",
					obj->value[0]);
		}
	    add_buf(buffer, buf);
		break;
		*/

	/*
	case ITEM_BODY_PART:
	{
		RACE_DATA *race = get_race_uid(obj->value[1]);
		sprintf(buf,
				"{B[  {Wv0{B]{G Body Parts:{x    %s\n\r"
				"{B[  {Wv1{B]{G Race:{x          %s\n\r",
				flag_string(part_flags, obj->value[0]),
				(IS_VALID(race) ? race->name : "{Dnone{x"));

		add_buf(buffer, buf);
		break;
	}
	*/
    }
}

bool set_portal_params(CHAR_DATA *ch, OBJ_INDEX_DATA *obj, int value_num, char *argument)
{
	PORTAL_DATA *portal = PORTAL(obj);
	char buf[MSL];
	WILDS_DATA *wilds;
	long vnum;
	switch(portal->type)
	{
		case GATETYPE_ENVIRONMENT:
			// Nothing gets set on environment portals
			break;

		case GATETYPE_NORMAL:
			switch(value_num)
			{
				case 0:		// AUID
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 1)
					{
						send_to_char("There is no area with that UID.\n\r", ch);
						return false;
					}

					if (!get_area_from_uid(vnum))
					{
						send_to_char("There is no area with that UID.\n\r", ch);
						return false;
					}

					portal->params[0] = vnum;
					portal->params[1] = 0;
					portal->params[2] = 0;
					portal->params[3] = 0;
					send_to_char("Area UID set.\n\r", ch);
					return true;

				case 1:		// VNUM
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (!get_room_index_auid(portal->params[0], vnum))
					{
						send_to_char("There is no such room.\n\r", ch);
						return false;
					}

					portal->params[1] = vnum;
					portal->params[2] = 0;
					portal->params[3] = 0;
					send_to_char("Room set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_WILDS:
			switch(value_num)
			{
				case 0:		// WUID
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (!get_wilds_from_uid(NULL, vnum))
					{
						send_to_char("There is no WILDS with that UID.\n\r", ch);
						return false;
					}

					portal->params[0] = vnum;
					portal->params[1] = 0;
					portal->params[2] = 0;
					send_to_char("WILDS UiD set.\n\r", ch);
					return true;

				case 1:		// X
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_x)
					{
						sprintf(buf, "X coordinate out of range (0 - %d)\n\r", wilds->map_size_x - 1);
						send_to_char(buf, ch);
						return false;
					}

					portal->params[1] = vnum;
					send_to_char("X coordinate set.\n\r", ch);
					return true;

				case 2:		// Y
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_y)
					{
						sprintf(buf, "Y coordinate out of range (0 - %d)\n\r", wilds->map_size_y - 1);
						send_to_char(buf, ch);
						return false;
					}

					portal->params[2] = vnum;
					send_to_char("Y coordinate set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_WILDSRANDOM:
			switch(value_num)
			{
				case 0:		// WUID
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					wilds = get_wilds_from_uid(NULL, vnum);
					if (!wilds)
					{
						send_to_char("There is no WILDS with that UID.\n\r", ch);
						return false;
					}

					portal->params[0] = vnum;
					portal->params[1] = 0;
					portal->params[2] = 0;
					portal->params[3] = wilds->map_size_x - 1;
					portal->params[4] = wilds->map_size_y - 1;
					send_to_char("WILDS UiD set.\n\r", ch);
					return true;

				case 1:		// Min X
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_x)
					{
						sprintf(buf, "Minimum X coordinate out of range (0 - %d)\n\r", wilds->map_size_x - 1);
						send_to_char(buf, ch);
						return false;
					}

					if (vnum > portal->params[3])
					{
						send_to_char("Minimum X coordinate cannot be greater than the Maximum X coordinate.\n\r", ch);
						return false;
					}

					portal->params[1] = vnum;
					send_to_char("Minimum X coordinate set.\n\r", ch);
					return true;

				case 2:		// Min Y
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_y)
					{
						sprintf(buf, "Minimum Y coordinate out of range (0 - %d)\n\r", wilds->map_size_y - 1);
						send_to_char(buf, ch);
						return false;
					}

					if (vnum > portal->params[4])
					{
						send_to_char("Minimum Y coordinate cannot be greater than the Maximum Y coordinate.\n\r", ch);
						return false;
					}

					portal->params[2] = vnum;
					send_to_char("Minimum Y coordinate set.\n\r", ch);
					return true;

				case 3:		// Max X
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_x)
					{
						sprintf(buf, "Maximum X coordinate out of range (0 - %d)\n\r", wilds->map_size_x - 1);
						send_to_char(buf, ch);
						return false;
					}

					if (vnum < portal->params[1])
					{
						send_to_char("Maximum X coordinate cannot be less than the Minimum X coordinate.\n\r", ch);
						return false;
					}

					portal->params[3] = vnum;
					send_to_char("Maximum X coordinate set.\n\r", ch);
					return true;

				case 4:		// Max Y
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					wilds = get_wilds_from_uid(NULL, portal->params[0]);
					if (!wilds)
					{
						send_to_char("WILDS UID not set.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum < 0 || vnum >= wilds->map_size_y)
					{
						sprintf(buf, "Maximum Y coordinate out of range (0 - %d)\n\r", wilds->map_size_y - 1);
						send_to_char(buf, ch);
						return false;
					}

					if (vnum < portal->params[2])
					{
						send_to_char("Maximum Y coordinate cannot be less than the Minimum Y coordinate.\n\r", ch);
						return false;
					}

					portal->params[4] = vnum;
					send_to_char("Maximum Y coordinate set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_AREARANDOM:
			switch(value_num)
			{
				case 0:		// AUID (0 == current area or wilds)
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum > 0 && !get_area_from_uid(vnum))
					{
						send_to_char("There is no area with that UID.\n\r", ch);
						return false;
					}

					portal->params[0] = UMAX(0, vnum);
					send_to_char("Area UID set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_REGIONRANDOM:
			switch(value_num)
			{
				case 0:		// AUID (0 == current area)
					if (!str_prefix(argument, "current"))
					{
						portal->params[0] = 0;
					}
					else
					{
						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						vnum = atol(argument);
						if (vnum < 1 || !get_area_from_uid(vnum))
						{
							send_to_char("There is no area with that UID.\n\r", ch);
							return false;
						}

						portal->params[0] = vnum;
					}
					send_to_char("Area UID set.\n\r", ch);
					return true;

				case 1:
					if (!str_prefix(argument, "default"))
					{
						portal->params[1] = 0;
					}
					else
					{
						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						vnum = atol(argument);
						if (vnum < 1)
						{
							send_to_char("Region index must be positive.\n\r", ch);
							return false;
						}

						portal->params[1] = vnum;
					}
					send_to_char("Region Index set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_SECTIONRANDOM:
			switch(value_num)
			{
				case 0:
					if (!str_prefix(argument, "current"))
					{
						portal->params[0] = 0;
						send_to_char("Section index set to {WCURRENT{x.\n\r", ch);
						return true;
					}
					else
					{
						char argsr[MIL];
						sent_bool mode = TRISTATE_UNDEF;

						argument = one_argument(argument, argsr);

						if (!str_prefix(argsr, "generated"))
							mode = false;
						else if (!str_prefix(argsr, "ordinal"))
							mode = true;
						else
						{
							send_to_char("Please specify either {WCURRENT{x, {YGENERATED{x or {GORDINAL{x.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						int index = atoi(argument);
						if (index < 1)
						{
							send_to_char("Index must be positive.\n\r", ch);
							return false;
						}

						portal->params[0] = mode ? -index : index;
						sprintf(buf, "Section index set to %s %d.\n\r",
							mode ? "{GOrdinal{x" : "{YGenerated{x",
							abs(index));
						send_to_char(buf, ch);
						return true;
					}
					break;
			}
			break;

		case GATETYPE_INSTANCERANDOM:
			// No extra values - target is based upon current location
			break;

		case GATETYPE_DUNGEONRANDOM:
			// No extra values - target is based upon current location
			break;

		case GATETYPE_AREARECALL:
			switch(value_num)
			{
				case 0:		// AUID
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					if (vnum > 0 && !get_area_from_uid(vnum))
					{
						send_to_char("There is no area with that UID.\n\r", ch);
						return false;
					}

					portal->params[0] = UMAX(0, vnum);
					send_to_char("Area UID set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_REGIONRECALL:
			switch(value_num)
			{
				case 0:		// AUID (0 == current area)
					if (!str_prefix(argument, "current"))
					{
						portal->params[0] = 0;
					}
					else
					{
						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						vnum = atol(argument);
						if (vnum < 1 || !get_area_from_uid(vnum))
						{
							send_to_char("There is no area with that UID.\n\r", ch);
							return false;
						}

						portal->params[0] = vnum;
					}
					send_to_char("Area UID set.\n\r", ch);
					return true;

				case 1:
					if (!str_prefix(argument, "default"))
					{
						portal->params[1] = 0;
					}
					else
					{
						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						vnum = atol(argument);
						if (vnum < 1)
						{
							send_to_char("Region index must be positive.\n\r", ch);
							return false;
						}

						portal->params[1] = vnum;
					}
					send_to_char("Region Index set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_DUNGEON:
			switch(value_num)
			{
				case 0:	// Dungeon VNUM.  Dungeon must exist in the same area as the portal
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					DUNGEON_INDEX_DATA *dng = get_dungeon_index(obj->area, vnum);
					if (!dng)
					{
						send_to_char("There is no such dungeon.\n\r", ch);
						return false;
					}

					portal->params[0] = vnum;
					portal->params[1] = 0;
					portal->params[2] = 0;
					send_to_char("Dungeon Vnum set.  Target location is default entrance.\n\r", ch);

					if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
					{
						send_to_char("{RWarning: Dungeon is set to use scripted level design.{x\n\r", ch);
					}
					return true;

				case 1:	// Target Floor (priority #1)
					{
						DUNGEON_INDEX_DATA *dng = get_dungeon_index(obj->area, portal->params[0]);
						if (!dng)
						{
							send_to_char("Please select Dungeon first.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						portal->params[1] = atol(argument);
						send_to_char("Target Floor set.\n\r", ch);
						return true;
					}

				case 2: // Target special room (priority #2, v6 must be invalid)
					{
						DUNGEON_INDEX_DATA *dng = get_dungeon_index(obj->area, portal->params[0]);
						if (!dng)
						{
							send_to_char("Please select Dungeon first.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						portal->params[2] = atol(argument);
						send_to_char("Target Special Room set.\n\r", ch);
						return true;
					}

				// If both v6 and v7 are invalid, the destination is the dungeon's default entry room
			}
			break;

		case GATETYPE_INSTANCE:
			// TODO: Complete
			break;

		case GATETYPE_RANDOM:
			// Nothing
			break;

		case GATETYPE_DUNGEONFLOOR:
			switch(value_num)
			{
				case 0:	// Dungeon VNUM.  Dungeon must exist in the same area as the portal
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					vnum = atol(argument);
					DUNGEON_INDEX_DATA *dng = get_dungeon_index(obj->area, vnum);
					if (!dng)
					{
						send_to_char("There is no such dungeon.\n\r", ch);
						return false;
					}

					portal->params[0] = vnum;
					portal->params[1] = 0;
					portal->params[2] = 0;
					send_to_char("Dungeon Vnum set.  Target location is default entrance.\n\r", ch);

					if (IS_SET(dng->flags, DUNGEON_SCRIPTED_LEVELS))
					{
						send_to_char("{RWarning: Dungeon is set to use scripted level design.{x\n\r", ch);
					}
					return true;

				case 1:	// Target Floor (0 = use PREVFLOOR and NEXTFLOOR flags)
					{
						DUNGEON_INDEX_DATA *dng = get_dungeon_index(obj->area, portal->params[0]);
						if (!dng)
						{
							send_to_char("Please select Dungeon first.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						portal->params[1] = atol(argument);
						send_to_char("Target Floor set.\n\r", ch);
						return true;
					}
			}
			break;

		case GATETYPE_BLUEPRINT_SECTION_MAZE:
			switch(value_num)
			{
				case 0:
					if (!str_prefix(argument, "current"))
					{
						portal->params[0] = 0;
						send_to_char("Section index set to {WCURRENT{x.\n\r", ch);
						return true;
					}
					else
					{
						char argsr[MIL];
						sent_bool mode = TRISTATE_UNDEF;

						argument = one_argument(argument, argsr);

						if (!str_prefix(argsr, "generated"))
							mode = false;
						else if (!str_prefix(argsr, "ordinal"))
							mode = true;
						else
						{
							send_to_char("Please specify either {WCURRENT{x, {YGENERATED{x or {GORDINAL{x.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						int index = atoi(argument);
						if (index < 1)
						{
							send_to_char("Index must be positive.\n\r", ch);
							return false;
						}

						portal->params[0] = mode ? -index : index;
						sprintf(buf, "Section index set to %s %d.\n\r",
							mode ? "{GOrdinal{x" : "{YGenerated{x",
							abs(index));
						send_to_char(buf, ch);
						return true;
					}
					break;

				case 1:
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int x = atoi(argument);
					if (x < 1)
					{
						send_to_char("X-Coordinate must be positive.\n\r", ch);
						return false;
					}

					portal->params[1] = x;
					send_to_char("X-coordinate set.\n\r", ch);
					return true;
				
				case 2:
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int y = atoi(argument);
					if (y < 1)
					{
						send_to_char("X-Coordinate must be positive.\n\r", ch);
						return false;
					}

					portal->params[2] = y;
					send_to_char("Y-coordinate set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_BLUEPRINT_SPECIAL:
			switch(value_num)
			{
				case 0:
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int index = atoi(argument);
					if (index < 1)
					{
						send_to_char("Special Room index must be positive.\n\r", ch);
						return false;
					}

					portal->params[0] = index;
					send_to_char("Special Room index set.\n\r", ch);
					return true;
			}
			break;
		
		case GATETYPE_DUNGEON_FLOOR_SPECIAL:
			switch(value_num)
			{
				case 0:
					if (!str_prefix(argument, "current"))
					{
						portal->params[0] = 0;
						send_to_char("FLoor index set to {WCURRENT{x.\n\r", ch);
						return true;
					}
					else
					{
						char argsr[MIL];
						sent_bool mode = TRISTATE_UNDEF;

						argument = one_argument(argument, argsr);

						if (!str_prefix(argsr, "generated"))
							mode = false;
						else if (!str_prefix(argsr, "ordinal"))
							mode = true;
						else
						{
							send_to_char("Please specify either {WCURRENT{x, {YGENERATED{x or {GORDINAL{x.\n\r", ch);
							return false;
						}

						if (!is_number(argument))
						{
							send_to_char("That is not a number.\n\r", ch);
							return false;
						}

						int index = atoi(argument);
						if (index < 1)
						{
							send_to_char("Index must be positive.\n\r", ch);
							return false;
						}

						portal->params[0] = mode ? -index : index;
						sprintf(buf, "Floor index set to %s %d.\n\r",
							mode ? "{GOrdinal{x" : "{YGenerated{x",
							abs(index));
						send_to_char(buf, ch);
						return true;
					}
					break;

				case 1:
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int index = atoi(argument);
					if (index < 1)
					{
						send_to_char("Special Room index must be positive.\n\r", ch);
						return false;
					}

					portal->params[1] = index;
					send_to_char("Special Room index set.\n\r", ch);
					return true;
			}
			break;

		case GATETYPE_DUNGEON_SPECIAL:
			switch(value_num)
			{
				case 0:
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int index = atoi(argument);
					if (index < 1)
					{
						send_to_char("Special Room index must be positive.\n\r", ch);
						return false;
					}

					portal->params[0] = index;
					send_to_char("Special Room index set.\n\r", ch);
					return true;
			}
			break;
	}
	return false;
}

bool set_obj_values(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
	long i = 0;
	BUFFER *buffer;

	switch(pObj->item_type)
	{
	default:
		break;

	/*
	case ITEM_BLANK_SCROLL:
		switch(value_num)
		{
		default:
			do_help(ch, "ITEM_BLANK_SCROLL");
			return false;
		case 0:
			{
				int mana = atoi(argument);
				if (mana < 0 || mana > 1000)
				{
					send_to_char("Maximum mana must be between 0 and 1000.\n\r", ch);
					return false;
				}

				send_to_char("MAXIMUM MANA SET.\n\r\n\r", ch);
				pObj->value[0] = mana;
			}
			break;
		}
		break;
		*/

	//case ITEM_WAND:
	//case ITEM_STAFF:
		/*
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_STAFF_WAND");
			return false;
		case 0:
			send_to_char("SPELL LEVEL SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			//send_to_char("SPELL TYPE SET.\n\r", ch);
			//pObj->value[3] = skill_lookup(argument);
			break;
		}
		break;
		*/

	//case ITEM_SCROLL:
	//case ITEM_POTION:
	case ITEM_PILL:
		break;

	/*
	case ITEM_TATTOO:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_TATTOO");
			return false;
		case 0:
			send_to_char("TOUCHES SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("FADING CHANCE SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		}
		break;

	case ITEM_INK:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_INK");
			return false;
		case 0:
			send_to_char("TYPE 1 SET.\n\r\n\r", ch);
			pObj->value[0] = flag_lookup(argument,catalyst_types);
			break;
		case 1:
			send_to_char("TYPE 2 SET.\n\r\n\r", ch);
			pObj->value[1] = flag_lookup(argument,catalyst_types);
			break;
		case 2:
			send_to_char("TYPE 3 SET.\n\r\n\r", ch);
			pObj->value[2] = flag_lookup(argument,catalyst_types);
			break;
		}
		break;
	*/

	case ITEM_SEXTANT:
		switch(value_num)
		{
		default:
			do_help(ch, "ITEM_SEXTANT");
			return false;
		case 0:
			send_to_char("Accuracy set.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		}
		break;

	case ITEM_SEED:
		switch(value_num)
		{
		default:
			do_help(ch, "ITEM_SEED");
			return false;
		case 0:
			send_to_char("Time set.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			if (atoi(argument) != 0)
			{
				if (!get_obj_index(pObj->area, atoi(argument)))
				{
					send_to_char("No such object exists.\n\r\n\r", ch);
					return false;
				}
			}
			send_to_char("Vnum set.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		}
		break;

	/*
	case ITEM_ARMOUR:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_ARMOUR");
			return false;
		case 0:
			send_to_char("AC PIERCE SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("AC BASH SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("AC SLASH SET.\n\r\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			send_to_char("AC EXOTIC SET.\n\r\n\r", ch);
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			send_to_char("ARMOUR STRENGTH SET.\n\r", ch);
			send_to_char("ARMOUR CLASS SET.\n\r\n\r", ch);

			pObj->value[4] = get_armour_strength(argument);

			set_armour(pObj);

			break;
		}
		break;
	*/

	/*
	case ITEM_RANGED_WEAPON:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_RANGED_WEAPON");
			return false;
		case 0:
			send_to_char("RANGED WEAPON CLASS SET.\n\r\n\r", ch);
			pObj->value[0] = flag_value(ranged_weapon_class, argument);
			break;
		case 1:
			send_to_char("NUMBER OF DICE SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("TYPE OF DICE SET.\n\r\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			send_to_char("PROJECTILE DISTANCE SET.\n\r\n\r", ch);
			pObj->value[3] = atoi(argument);
			break;
		}
		break;
	*/

	case ITEM_HERB:
		/*
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_HERB");
			return false;
		case 0:
			for (i = 0; i < MAX_HERB; i++)
			{
				if (!str_prefix(argument, herb_table[i].name))
				break;
			}

			if (i < MAX_HERB)
			{
				pObj->value[0] = i;
				send_to_char("HERB TYPE SET.\n\r", ch);
			}
			else
				send_to_char("Invalid herb type.\n\r", ch);
			break;
		case 1:
			send_to_char("HEALING RATE SET.\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("REGENERATIVE RATE SET.\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			send_to_char("REFRESHING RATE SET.\n\r", ch);
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			if ((i = flag_value(imm_flags, argument)) != NO_FLAG)
			{
				pObj->value[4] ^= i;
				send_to_char("IMMUNITY SET.\n\r", ch);
			}
			else
				send_to_char("Invalid immunity.\n\r", ch);
			break;
		case 5:
			if ((i = flag_value(res_flags, argument)) != NO_FLAG)
			{
				pObj->value[5] ^= i;
				send_to_char("RESISTANCE SET.\n\r", ch);
			}
			else
			send_to_char("Invalid resistance.\n\r", ch);
			break;
		case 6:
			if ((i = flag_value(vuln_flags, argument)) != NO_FLAG)
			{
				pObj->value[6] ^= i;
				send_to_char("VULNERABILITY SET.\n\r", ch);
			}
			else
				send_to_char("Invalid vulnerability.\n\r", ch);
			break;
		case 7:
			// TODO: UNUSED?
			if ((i = skill_lookup(argument)) > 0 && skill_table[i].spell_fun != spell_null)
			{
				send_to_char("SPELL SET.\n\r", ch);
				pObj->value[7] = i;
			}
			else if (i == 0)
			{
				send_to_char("SPELL RESET.\n\r", ch);
				pObj->value[7] = 0;
			}
			else
				send_to_char("INVALID ARGUMENT.\n\r", ch);

			break;
		}
		*/

		break;

	/*
	case ITEM_WEAPON:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_WEAPON");
			return false;
		case 0:
			send_to_char("WEAPON CLASS SET.\n\r\n\r", ch);
			pObj->value[0] = flag_value(weapon_class, argument);
			break;
		case 1:
			send_to_char("NUMBER OF DICE SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("TYPE OF DICE SET.\n\r\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			send_to_char("WEAPON TYPE SET.\n\r\n\r", ch);
			pObj->value[3] = attack_lookup(argument);
			break;
		case 4:
			send_to_char("SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch);
			pObj->value[4] ^= (flag_value(weapon_type2, argument) != NO_FLAG ? flag_value(weapon_type2, argument) : 0);
			break;
		}
		break;
		*/

	/*
	case ITEM_CART:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_CART");
			return false;
		case 0:
			send_to_char("WEIGHT CAPACITY SET.\n\r\n\r", ch);
			pObj->value[0] = atol(argument);
			break;
		case 1:
			send_to_char("DELAY SET.\n\r\n\r", ch);
			pObj->value[1] = atol(argument);
			break;
		case 2:
			send_to_char("STRENGTH SET.\n\r\n\r", ch);
			pObj->value[2] = atol(argument);
			break;
		case 3:
			send_to_char("CART MAX WEIGHT SET.\n\r", ch);
			pObj->value[3] = atol(argument);
			break;
		case 4:
			send_to_char("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
			pObj->value[4] = atol (argument);
			break;
		case 5:
			send_to_char("VANISH TIME SET.\n\r\n\r", ch);
			pObj->value[5] = atol (argument);
			break;
		}
		break;
	*/

	case ITEM_TRADE_TYPE:
		switch(value_num)
		{
		default:
			do_help(ch, "ITEM_TRADE_TYPE");
			return false;

		case 0:
			if ((argument[0] == '\0') || ((i = get_trade_item(argument)) == 0))
			{
				send_to_char("Trade Types:\n\r", ch);

				while(trade_table[ i ].trade_type != -1)
				{
					send_to_char(trade_table[ i ].name, ch);
					send_to_char("\n\r", ch);
					i++;
				}
				break;
			}

			pObj->value[0] = i;
			send_to_char("Trade type set.\n\r", ch);
			break;
		}
		break;

	/*
	case ITEM_WEAPON_CONTAINER:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_WEAPON_CONTAINER");
			return false;
		case 0:
			send_to_char("WEIGHT CAPACITY SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			pObj->value[1] = flag_value(weapon_class, argument);
			send_to_char("WEAPON TYPE SET.\n\r\n\r", ch);
			break;
		case 3:
			send_to_char("CONTAINER MAX ITEMS SET.\n\r", ch);
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			send_to_char("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
			pObj->value[4] = atoi (argument);
			break;
		}
		break;
	*/

	/*
	case ITEM_CONTAINER:
		switch (value_num)
		{
		int value;

		default:
			do_help(ch, "ITEM_CONTAINER");
			return false;
		case 0:
			send_to_char("WEIGHT CAPACITY SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			if ((value = flag_value(container_flags, argument)) != NO_FLAG)
				TOGGLE_BIT(pObj->value[1], value);
			else
			{
				do_help (ch, "ITEM_CONTAINER");
				return false;
			}
			send_to_char("CONTAINER TYPE SET.\n\r\n\r", ch);
			break;
		case 3:
			if (atoi (argument) > 225 && ch->tot_level < MAX_LEVEL)
			{
				send_to_char("Sorry, that value is out of range.\n\r", ch);
				return false;
			}

			send_to_char("CONTAINER MAX ITEMS SET.\n\r", ch);
			pObj->value[3] = atoi(argument);
			break;

		case 4:
			if(atoi(argument) <= 0 || atoi(argument) > 1000)
			{
				send_to_char("Weight multiplier must be between 1 and 1000.\n\r",  ch);
				return false;
			}

			if (atoi(argument) < 1000 && !has_imp_sig(NULL, pObj) && ch->tot_level < MAX_LEVEL) {
				send_to_char("An imp sig is required to set the weight multiplier below 100%.\n\r", ch);
				return false;
			}

			if (has_imp_sig(NULL, pObj))
				use_imp_sig(NULL, pObj);

			send_to_char("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
			pObj->value[4] = atoi (argument);
			break;
		}
		break;
	*/

	/*
	case ITEM_DRINK_CON:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_DRINK");
			return false;
		case 0:
			send_to_char("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("LIQUID TYPE SET.\n\r\n\r", ch);
			pObj->value[2] = (liq_lookup(argument) != -1 ? liq_lookup(argument) : 0);
			break;
		case 3:
			send_to_char("POISON VALUE TOGGLED.\n\r\n\r", ch);
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		}
		break;

	case ITEM_FOUNTAIN:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_FOUNTAIN");
			return false;
		case 0:
			send_to_char("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("LIQUID TYPE SET.\n\r\n\r", ch);
			pObj->value[2] = (liq_lookup(argument) != -1 ? liq_lookup(argument) : 0);
			break;
		case 3:
			send_to_char("POISON VALUE TOGGLED.\n\r\n\r", ch);
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		case 4:
			send_to_char("FILL RATE SET.\n\r\n\r", ch);
			pObj->value[4] = atoi(argument);
			break;
		case 5:
			send_to_char("POISON RATE SET.\n\r\n\r", ch);
			pObj->value[5] = atoi(argument);
			break;
		}
		break;
		*/

	/*
	case ITEM_MIST:
		switch (value_num)
		{
		default:
			do_help(ch, "ITEM_MIST");
			return false;
		case 0:
			send_to_char("PERCENTAGE TO HIDE OBJECTS SET.\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			send_to_char("PERCENTAGE TO HIDE CHARACTERS SET.\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		}
		break;
	*/

	/*
	case ITEM_CORPSE_NPC:
		switch (value_num)
		{
		int value;
		default:
			do_help(ch, "ITEM_CORPSE_NPC");
			return false;
		case 0:
			if ((value = flag_value(corpse_types, argument)) == NO_FLAG)
				return false;
			send_to_char("CORPSE TYPE SET.\n\r", ch);
			pObj->value[0] = value;
			break;
		case 1:
			send_to_char("RESURRECTION CHANCE SET.\n\r", ch);
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			send_to_char("ANIMATION CHANCE SET.\n\r", ch);
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			if ((value = flag_value(part_flags, argument)) == NO_FLAG)
				return false;
			send_to_char("BODY PARTS SET.\n\r", ch);
			pObj->value[3] = value;
			break;
		case 5:
			send_to_char("MOBILE INDEX VNUM SET.\n\r", ch);
			pObj->value[5] = atoi(argument);
			break;
		}
		break;
		*/

	/*
	case ITEM_INSTRUMENT:
		switch (value_num)
		{
		int value;
		default:
			do_help(ch,"ITEM_INSTRUMENT");
			return false;
		case 0:
			if ((value = flag_value(instrument_types, argument)) == NO_FLAG)
				return false;
			send_to_char("INSTRUMENT TYPE SET.\n\r", ch);
			pObj->value[0] = value;
			break;
		case 1:
			if ((value = flag_value(instrument_flags, argument)) == NO_FLAG)
				return false;
			send_to_char("INSTRUMENT FLAGS TOGGLED.\n\r", ch);
			pObj->value[1] ^= value;
			break;
		case 2:
			value = atoi(argument);
			if( value < 1 || value > 5000)
			{
				send_to_char("Minimum scale factor for playtime can only be between 1% and 5000%.\n\r", ch);
				return false;
			}
			send_to_char("MINIMUM PLAYTIME SCALE FACTOR SET.\n\r", ch);
			pObj->value[2] = value;
			break;
		case 3:
			value = atoi(argument);
			if( value < 1 || value > 5000)
			{
				send_to_char("Maximum scale factor for playtime can only be between 1% and 5000%.\n\r", ch);
				return false;
			}
			send_to_char("MAXIMUM PLAYTIME SCALE FACTOR SET.\n\r", ch);
			pObj->value[3] = value;
			break;
		}
		break;
	*/

	/*
	case ITEM_BOOK:
		switch (value_num)
		{
		int value;

		default:
			do_help(ch, "ITEM_BOOK");
			return false;
		case 1:
			if ((value = flag_value(container_flags, argument)) != NO_FLAG)
				TOGGLE_BIT(pObj->value[1], value);
			else
			{
				do_help (ch, "ITEM_BOOK");
				return false;
			}
			send_to_char("BOOK (CONTAINER) FLAGS SET.\n\r\n\r", ch);
			break;
		}
		break;
		*/

	/*
	case ITEM_TELESCOPE:
		switch (value_num)
		{
		int value;

		default:
			do_help(ch, "ITEM_TELESCOPE");
			return false;
		case 0:
			value = atoi(argument);
			if( value < 0 || (value > 0 && value < pObj->value[1]) || value > pObj->value[2] )
			{
				sprintf(buf, "TELESCOPE DISTANCE must be 0(for collapsed), or from %ld to %ld.\n\r", pObj->value[1], pObj->value[2]);
				send_to_char(buf, ch);
				return false;
			}
			pObj->value[0] = value;
			send_to_char("TELESCOPE DISTANCE SET\n\r", ch);
			break;
		case 1:
			value = atoi(argument);
			if( value <= 0 )
			{
				send_to_char("TELESCOPE MINIMUM DISTANCE must be greater than zero.\n\r", ch);
				return false;
			}
			if( value > pObj->value[2] )
			{
				sprintf(buf, "TELESCOPE MINIMUM DISTANCE must be less than or equal to %ld.\n\r", pObj->value[2]);
				send_to_char(buf, ch);
				return false;
			}
			pObj->value[1] = value;
			send_to_char("TELESCOPE MINIMUM DISTANCE SET\n\r", ch);
			break;
		case 2:
			value = atoi(argument);
			if( value <= 0 )
			{
				send_to_char("TELESCOPE MAXIMUM DISTANCE must be greater than zero.\n\r", ch);
				return false;
			}
			if( value < pObj->value[1] )
			{
				sprintf(buf, "TELESCOPE MAXIMUM DISTANCE must be greater than or equal to %ld.\n\r", pObj->value[1]);
				send_to_char(buf, ch);
				return false;
			}
			if( value > 50 )
			{
				if( !has_imp_sig(NULL, pObj) && !IS_IMPLEMENTOR(ch) )
				{
					send_to_char("An imp sig is required to set the TELESCOPE MAXIMUM DISTANCE greater than 50.\n\r", ch);
					return false;
				}

				if (has_imp_sig(NULL, pObj))
					use_imp_sig(NULL, pObj);
			}
			pObj->value[2] = value;
			send_to_char("TELESCOPE MAXIMUM DISTANCE SET\n\r", ch);
			break;
		case 3:
			value = atoi(argument);
			if( value <= 0 )
			{
				send_to_char("TELESCOPE BONUSVIEW must be greater than zero.\n\r", ch);
				return false;
			}
			if( value > 10 )
			{
				if( !has_imp_sig(NULL, pObj) && !IS_IMPLEMENTOR(ch) )
				{
					send_to_char("An imp sig is required to set the TELESCOPE BONUSVIEW greater than 50.\n\r", ch);
					return false;
				}

				if (has_imp_sig(NULL, pObj))
					use_imp_sig(NULL, pObj);
			}
			pObj->value[3] = value;
			send_to_char("TELESCOPE BONUSVIEW SET\n\r", ch);
			break;
		case 4:
			if( is_number(argument) )
			{
				value = atoi(argument);
				if( value < 0 || value >= 360 )
				{
					send_to_char("TELESCOPE HEADING must be from 0 to 359.\n\r", ch);
					return false;
				}

				pObj->value[4] = value;
				send_to_char("TELESCOPE HEADING SET\n\r", ch);
			}
			else if( !str_cmp(argument, "none") || !str_cmp(argument, "clear") )
			{
				pObj->value[4] = -1;
				send_to_char("TELESCOPE HEADING CLEARED\n\r", ch);
			}
			break;
		}
		break;
	case ITEM_COMPASS:
		switch (value_num)
		{
		int value;
		long wuid;
		WILDS_DATA *pWilds;

		default:
			do_help(ch, "ITEM_COMPASS");
			return false;
		case 0:
			send_to_char("Accuracy set.\n\r\n\r", ch);
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			wuid = atoi(argument);
			if( wuid > 0 )
			{
				pWilds = get_wilds_from_uid(NULL,wuid);
				if( !pWilds )
				{
					send_to_char("Invalid Wilds.\n\r", ch);
					return false;
				}

				pObj->value[1] = wuid;
				pObj->value[2] = pWilds->map_size_x / 2;
				pObj->value[3] = pWilds->map_size_y / 2;

				send_to_char("WILDS set.\n\r", ch);
			}
			else
			{
				pObj->value[1] = 0;
				pObj->value[2] = -1;
				pObj->value[3] = -1;
				send_to_char("WILDS cleared.\n\r", ch);
			}
			break;
		case 2:
			if( !pObj->value[1] )
			{
				send_to_char("Please set the WILDS({Wv1{x) before assigning coordinates.\n\r", ch);
				return false;
			}

			pWilds = get_wilds_from_uid(NULL,pObj->value[1]);
			if( !pWilds )
			{
				send_to_char("Please set the WILDS({Wv1{x) to a valid wilderness before assigning coordinates.\n\r", ch);
				return false;
			}

			value = atoi(argument);
			if( value < 0 || value >= pWilds->map_size_x )
			{
				sprintf(buf, "X COORDINATE must be from 0 to %d.\n\r", pWilds->map_size_x - 1);
				send_to_char(buf, ch);
				return false;
			}

			pObj->value[2] = value;
			send_to_char("X COORDINATE set.\n\r", ch);
			break;
		case 3:
			if( !pObj->value[1] )
			{
				send_to_char("Please set the WILDS({Wv1{x) before assigning coordinates.\n\r", ch);
				return false;
			}

			pWilds = get_wilds_from_uid(NULL,pObj->value[1]);
			if( !pWilds )
			{
				send_to_char("Please set the WILDS({Wv1{x) to a valid wilderness before assigning coordinates.\n\r", ch);
				return false;
			}

			value = atoi(argument);
			if( value < 0 || value >= pWilds->map_size_y )
			{
				sprintf(buf, "Y COORDINATE must be from 0 to %d.\n\r", pWilds->map_size_y - 1);
				send_to_char(buf, ch);
				return false;
			}

			pObj->value[3] = value;
			send_to_char("Y COORDINATE set.\n\r", ch);
			break;
		}
		break;
	*/
	/*
	case ITEM_BODY_PART:
		switch(value_num)
		{
		int value;
		default:
			do_help(ch, "ITEM_BODY_PART");
			break;

		case 0:
			if ((value = flag_value(part_flags, argument)) == NO_FLAG)
				return false;
			send_to_char("BODY PARTS TOGGLED.\n\r", ch);
			pObj->value[0] ^= value;
			break;

		case 1:
			send_to_char("RACE SET\n\r", ch);
			pObj->value[1] = race_lookup(argument);
			break;

		}
		break;
	*/
	}

	buffer = new_buf();
	print_obj_values(pObj, buffer);
	page_to_char(buf_string(buffer), ch);
	free_buf(buffer);

	return true;
}



bool olc_has_spell(LLIST *spells, SKILL_DATA *spell)
{
	ITERATOR it;
	SPELL_DATA *sp;

	iterator_start(&it, spells);
	while((sp = (SPELL_DATA *)iterator_nextdata(&it)))
	{
		if (sp->skill == spell)
			break;
	}
	iterator_stop(&it);

	return (sp != NULL);
}



void __oedit_book_renumber_pages(BOOK_DATA *book)
{
	ITERATOR it;
	int page_no = 0;
	BOOK_PAGE *page;

	iterator_start(&it, book->pages);
	while((page = (BOOK_PAGE *)iterator_nextdata(&it)))
	{
		page->page_no = ++page_no;
	}
	iterator_stop(&it);
}



void oedit_type_container_showlist(CHAR_DATA *ch, LLIST *list, char *name, char color)
{
	if (list_size(list) > 0)
	{
		char buf[MSL];
		CONTAINER_FILTER *filter;
		ITERATOR it;
		BUFFER *buffer = new_buf();

		sprintf(buf, "{x[{%cItem Type %s:{x]\n\r", color, name);
		add_buf(buffer, buf);

		sprintf(buf, " {x%-6s %-15s %-15s\n\r", "Number", "Item Type", "Sub Type");
		add_buf(buffer, buf);
		sprintf(buf, " {x%-6s %-15s %-15s\n\r", "------", "---------", "--------");
		add_buf(buffer, buf);

		int cnt = 0;
		iterator_start(&it, list);
		while((filter = (CONTAINER_FILTER *)iterator_nextdata(&it)))
		{
			char *subtype = "none";
			if (filter->item_type == ITEM_WEAPON && filter->sub_type >= 0)
			{
				subtype = flag_string(weapon_class, filter->sub_type);
			}

			sprintf(buf, " {%c%-6d %-15s %-15s{x\n\r", color, ++cnt,
				flag_string(type_flags, filter->item_type),
				subtype);
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
}

bool __container_is_listed(LLIST *list, int item_type, int sub_type);





bool olc_can_quaff_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_QUAFF, TRIGSLOT_SPELL) != NULL;
	else
		return skill->quaff_fun != NULL;
}




#define PARSE_PERCENT(v,s,f,msg)	\
	if (!str_prefix((v),(s))) \
	{ \
		int percent; \
\
		if (!is_number(argument) || (percent = atoi(argument)) < 0 || percent > 100) \
		{ \
			send_to_char("Please provide a percentage.\n\r", ch); \
			return false; \
		} \
\
		(f) = percent; \
		send_to_char( (msg), ch); \
		return true; \
	}


bool __oedit_type_portal_subtype(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument)
{
	char buf[MSL];
	char arg[MIL];
	PORTAL_DATA *portal = PORTAL(pObj);

	argument = one_argument(argument, arg);

	switch(portal->type)
	{
		default: break;

		case GATETYPE_NORMAL:
			if (!str_prefix(arg, "room"))
			{
				WNUM wnum;
				
				if (!parse_widevnum(argument, pObj->area, &wnum))
				{
					send_to_char("That is not a widevnum.\n\r", ch);
					return false;
				}

				ROOM_INDEX_DATA *room = get_room_index(wnum.pArea, wnum.vnum);
				if (!room)
				{
					send_to_char("No room exists for that widevnum.\n\r", ch);
					return false;
				}

				portal->params[0] = room->area->uid;
				portal->params[1] = room->vnum;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;
				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;

		case GATETYPE_WILDS:
			if (!str_prefix(arg, "wilds"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);

				if (!is_number(arg2) || !is_number(arg3) || !is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				WILDS_DATA *wilds = get_wilds_from_uid(NULL, atol(arg2));
				if (!wilds)
				{
					send_to_char("No such wilds with that uid.\n\r", ch);
					return false;
				}

				long x = atoi(arg3);
				long y = atoi(argument);
				if (x < 1 || x > wilds->map_size_x)
				{
					sprintf(buf, "Please specify an x-coordinate from 1 to %d\n\r", wilds->map_size_x);
					send_to_char(buf, ch);
					return false;
				}
				if (y < 1 || y > wilds->map_size_y)
				{
					sprintf(buf, "Please specify an y-coordinate from 1 to %d\n\r", wilds->map_size_y);
					send_to_char(buf, ch);
					return false;
				}

				portal->params[0] = wilds->uid;
				portal->params[1] = x;
				portal->params[2] = y;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_WILDSRANDOM:
			if (!str_prefix(arg, "wildsrandom"))
			{
				char arg2[MIL];
				char arg3[MIL];
				char arg4[MIL];
				char arg5[MIL];

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);
				argument = one_argument(argument, arg4);
				argument = one_argument(argument, arg5);

				if (!is_number(arg2) || !is_number(arg3) || !is_number(arg4) || !is_number(arg5) || !is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				WILDS_DATA *wilds = get_wilds_from_uid(NULL, atol(arg2));
				if (!wilds)
				{
					send_to_char("No such wilds with that uid.\n\r", ch);
					return false;
				}

				long min_x = atol(arg3);
				long min_y = atol(arg4);
				long max_x = atol(arg5);
				long max_y = atol(argument);
				if (min_x < 1 || min_x > wilds->map_size_x)
				{
					sprintf(buf, "Please specify a minimum x-coordinate from 1 to %d\n\r", wilds->map_size_x);
					send_to_char(buf, ch);
					return false;
				}
				if (min_y < 1 || min_y > wilds->map_size_y)
				{
					sprintf(buf, "Please specify a minimum y-coordinate from 1 to %d\n\r", wilds->map_size_y);
					send_to_char(buf, ch);
					return false;
				}
				if (max_x < 1 || max_x > wilds->map_size_x)
				{
					sprintf(buf, "Please specify a maximum x-coordinate from 1 to %d\n\r", wilds->map_size_x);
					send_to_char(buf, ch);
					return false;
				}
				if (max_y < 1 || max_y > wilds->map_size_y)
				{
					sprintf(buf, "Please specify a maximum y-coordinate from 1 to %d\n\r", wilds->map_size_y);
					send_to_char(buf, ch);
					return false;
				}

				portal->params[0] = wilds->uid;
				portal->params[1] = UMIN(min_x, max_x);
				portal->params[2] = UMIN(min_y, max_y);
				portal->params[3] = UMAX(min_x, max_x);
				portal->params[4] = UMAX(min_y, max_y);

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_AREARANDOM:
			if (!str_prefix(arg, "arearandom"))
			{
				if (!str_prefix(argument, "current"))
				{
					portal->params[0] = 0;
				}
				else if(!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					AREA_DATA *area = get_area_from_uid(atol(argument));
					if (!area)
					{
						send_to_char("No such area exists for that uid.\n\r", ch);
						return false;
					}

					portal->params[0] = area->uid;
				}

				portal->params[1] = 0;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_REGIONRANDOM:
			if (!str_prefix(arg, "regionrandom"))
			{
				char arg2[MIL];
				AREA_DATA *area = NULL;

				argument = one_argument(argument, arg2);

				if (!str_prefix(arg2, "current"))
				{
					portal->params[0] = 0;
				}
				else if(!is_number(arg2))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					area = get_area_from_uid(atol(arg2));
					if (!area)
					{
						send_to_char("No such area exists for that uid.\n\r", ch);
						return false;
					}

					portal->params[0] = area->uid;
				}

				if (!str_prefix(argument, "default"))
				{
					portal->params[1] = 0;
				}
				else if(!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					long region_no = atol(argument);
					if (region_no < 1)
					{
						send_to_char("Please specify a positive region number.\n\r", ch);
						return false;
					}

					if (area && !get_area_region_by_uid(area, region_no))
					{
						send_to_char("No region in the specified area with that UID.\n\r", ch);
						return false;
					}

					portal->params[1] = region_no;
				}

				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_SECTIONRANDOM:
			if (!str_prefix(arg, "sectionrandom"))
			{
				char arg2[MIL];

				argument = one_argument(argument, arg2);

				if (!str_prefix(arg2, "current"))
				{
					portal->params[0] = 0;
				}
				else
				{
					sent_bool mode = TRISTATE_UNDEF;

					if (!str_prefix(arg2, "generated"))
						mode = false;
					else if (!str_prefix(arg2, "ordinal"))
						mode = true;
					else
					{
						send_to_char("Please specify {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}
					int section_no = atoi(argument);
					if (section_no < 1)
					{
						send_to_char("Please specify a positive section number.\n\r", ch);
						return false;
					}

					portal->params[0] = mode ? -section_no : section_no;
				}

				portal->params[1] = 0;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_AREARECALL:
			if (!str_prefix(arg, "arearecall"))
			{
				if (!str_prefix(argument, "current"))
				{
					portal->params[0] = 0;
				}
				else if (!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					AREA_DATA *area = get_area_from_uid(atol(argument));
					if (!area)
					{
						send_to_char("No such area exists for that uid.\n\r", ch);
						return false;
					}

					portal->params[0] = area->uid;
				}

				portal->params[1] = 0;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_REGIONRECALL:
			if (!str_prefix(arg, "regionrecall"))
			{
				char arg2[MIL];
				AREA_DATA *area = NULL;

				argument = one_argument(argument, arg2);

				if (!str_prefix(arg2, "current"))
				{
					portal->params[0] = 0;
				}
				else if (!is_number(arg2))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					area = get_area_from_uid(atol(arg2));
					if (!area)
					{
						send_to_char("No such area exists for that uid.\n\r", ch);
						return false;
					}

					portal->params[0] = area->uid;
				}

				if (!str_prefix(argument, "default"))
				{
					portal->params[1] = 0;
				}
				else if (!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}
				else
				{
					long region_no = atol(argument);

					if (region_no < 1)
					{
						send_to_char("Please specify a positive region number.\n\r", ch);
						return false;
					}

					if (area && !get_area_region_by_uid(area, region_no))
					{
						send_to_char("No region in the specified area with that UID.\n\r", ch);
						return false;
					}

					portal->params[1] = region_no;
				}

				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			send_to_char("         portal regionrecall <auid|current>[ <region#|default>]\n\r", ch);
			break;
		case GATETYPE_DUNGEON:
			if (!str_prefix(arg, "dungeon"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);
				
				if (!is_number(arg2))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				DUNGEON_INDEX_DATA *dungeon = get_dungeon_index(pObj->area, atol(arg2));
				if (!dungeon)
				{
					send_to_char("No such dungeon exists for that vnum.\n\r", ch);
					return false;
				}

				portal->params[0] = dungeon->vnum;

				if (!str_prefix(arg3, "floor"))
				{
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int floor = atoi(argument);
					if (!IS_SET(dungeon->flags, DUNGEON_SCRIPTED_LEVELS))
					{
						if (floor < 1 || floor > list_size(dungeon->levels))
						{
							sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(dungeon->levels));
							send_to_char(buf, ch);
							return false;
						}
					}

					portal->params[1] = floor;
					portal->params[2] = 0;					
				}
				else if(!str_prefix(arg3, "room"))
				{
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int room_no = atoi(argument);
					if (!IS_SET(dungeon->flags, DUNGEON_SCRIPTED_LEVELS))
					{
						if (room_no < 1 || room_no > list_size(dungeon->special_rooms))
						{
							sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(dungeon->special_rooms));
							send_to_char(buf, ch);
							return false;
						}
					}

					portal->params[1] = 0;
					portal->params[2] = room_no;
				}
				else if (!str_prefix(arg3, "default"))
				{
					portal->params[1] = 0;
					portal->params[2] = 0;
				}
				else
				{
					return TRISTATE_UNDEF;
				}

				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_INSTANCE:
			// TODO: Complete
			break;
		case GATETYPE_RANDOM: break;
		case GATETYPE_DUNGEONFLOOR:
			if (!str_prefix(arg, "dungeonfloor"))
			{
				char arg2[MIL];

				argument = one_argument(argument, arg2);
				
				if (!is_number(arg2))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				DUNGEON_INDEX_DATA *dungeon = get_dungeon_index(pObj->area, atol(arg2));
				if (!dungeon)
				{
					send_to_char("No such dungeon exists for that vnum.\n\r", ch);
					return false;
				}

				portal->params[0] = dungeon->vnum;

				if (!str_prefix(argument, "default"))
				{
					portal->params[1] = 0;
				}
				else
				{
					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int floor = atoi(argument);
					if (!IS_SET(dungeon->flags, DUNGEON_SCRIPTED_LEVELS))
					{
						if (floor < 1 || floor > list_size(dungeon->levels))
						{
							sprintf(buf, "Please specify a number from 1 to %d.\n\r", list_size(dungeon->levels));
							send_to_char(buf, ch);
							return false;
						}
					}

					portal->params[1] = floor;
				}

				portal->params[2] = 0;					
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_BLUEPRINT_SECTION_MAZE:
			if (!str_prefix(arg, "sectionmaze"))
			{
				char arg2[MIL];
				char arg3[MIL];
				char arg4[MIL];

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);
				argument = one_argument(argument, arg4);

				int x, y;
				if (!str_prefix(arg2, "current"))
				{
					portal->params[0] = 0;

					if (!is_number(arg3) || !is_number(arg4))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					x = atoi(arg3);
					y = atoi(arg4);
				}
				else
				{
					sent_bool mode = TRISTATE_UNDEF;

					if (!str_prefix(arg2, "generated"))
						mode = false;
					else if (!str_prefix(arg2, "ordinal"))
						mode = true;
					else
					{
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					if (!is_number(arg3))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int section_no = atoi(arg3);

					portal->params[0] = mode ? -section_no : section_no;

					if (!is_number(arg4) || !is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					x = atoi(arg4);
					y = atoi(argument);
				}

				if (x < 1 || y < 1)
				{
					send_to_char("Please specify a positive number.\n\r", ch);
					return false;
				}

				portal->params[1] = x;
				portal->params[2] = y;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_BLUEPRINT_SPECIAL:
			if (!str_prefix(arg, "instancespecial"))
			{
				if (!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int room_no = atoi(argument);
				if (room_no < 1)
				{
					send_to_char("Please specify a positive special room number.\n\r", ch);
					return false;
				}

				portal->params[0] = room_no;
				portal->params[1] = 0;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_DUNGEON_FLOOR_SPECIAL:
			if (!str_prefix(arg, "floorspecial"))
			{
				char arg2[MIL];
				char arg3[MIL];

				argument = one_argument(argument, arg2);
				argument = one_argument(argument, arg3);

				int room_no;
				if (!str_prefix(arg2, "current"))
				{
					portal->params[0] = 0;

					if (!is_number(arg3))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					room_no = atoi(arg3);
				}
				else
				{
					sent_bool mode = TRISTATE_UNDEF;

					if (!str_prefix(arg2, "generated"))
						mode = false;
					else if (!str_prefix(arg2, "ordinal"))
						mode = true;
					else
					{
						send_to_char("Please specify either {Ygenerated{x or {Gordinal{x.\n\r", ch);
						return false;
					}

					if (!is_number(arg3))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					int floor = atoi(arg3);

					portal->params[0] = mode ? -floor : floor;

					if (!is_number(argument))
					{
						send_to_char("That is not a number.\n\r", ch);
						return false;
					}

					room_no = atoi(argument);
				}

				if (room_no < 1)
				{
					send_to_char("Please specify a positive special room number.\n\r", ch);
					return false;
				}

				portal->params[1] = room_no;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
		case GATETYPE_DUNGEON_SPECIAL:
			if (!str_prefix(arg, "dungeonspecial"))
			{
				if (!is_number(argument))
				{
					send_to_char("That is not a number.\n\r", ch);
					return false;
				}

				int room_no = atoi(argument);
				if (room_no < 1)
				{
					send_to_char("Please specify a positive special room number.\n\r", ch);
					return false;
				}

				portal->params[0] = room_no;
				portal->params[1] = 0;
				portal->params[2] = 0;
				portal->params[3] = 0;
				portal->params[4] = 0;

				send_to_char("PORTAL DESTINATION set.\n\r", ch);
				return true;
			}
			break;
	}

	return TRISTATE_UNDEF;
}


bool olc_can_recite_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_RECITE, TRIGSLOT_SPELL) != NULL;
	else
		return skill->recite_fun != NULL;
}



bool olc_can_zap_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_ZAP, TRIGSLOT_SPELL) != NULL;
	else
		return skill->zap_fun != NULL;
}


bool olc_can_brandish_spell(SKILL_DATA *skill)
{
	if (!is_skill_spell(skill)) return false;

	if (skill->token)
		return get_script_token(skill->token, TRIG_TOKEN_BRANDISH, TRIGSLOT_SPELL) != NULL;
	else
		return skill->zap_fun != NULL;
}





int get_armour_strength(char *argument)
{
    char arg[MAX_STRING_LENGTH];
    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "Heavy"))
	return OBJ_ARMOUR_HEAVY;

    else if (!str_cmp(arg, "Strong"))
	return OBJ_ARMOUR_STRONG;

    else if (!str_cmp(arg, "Medium"))
	return OBJ_ARMOUR_MEDIUM;

    else if (!str_cmp(arg, "Light"))
	return OBJ_ARMOUR_LIGHT;

    else if (!str_cmp(arg, "None"))
	return OBJ_ARMOUR_NOSTRENGTH;

    else
	return OBJ_ARMOUR_NOSTRENGTH;
}




void show_liqlist(CHAR_DATA *ch)
{
    int liq;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = new_buf();

    for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
	if ((liq % 21) == 0)
	    add_buf(buffer,"Name                 Colour          Proof Full Thirst Food Ssize\n\r");

	sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
		liq_table[liq].liq_name,liq_table[liq].liq_colour,
		liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
		liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
		liq_table[liq].liq_affect[4]);
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

    return;
}


void show_damlist(CHAR_DATA *ch)
{
    int att;
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];

    buffer = new_buf();

    for (att = 0; attack_table[att].name != NULL; att++)
    {
	if ((att % 21) == 0)
	    add_buf(buffer,"Name                 Noun\n\r");

	sprintf(buf, "%-20s %-20s\n\r",
		attack_table[att].name,attack_table[att].noun);
	add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);

    return;
}





// Inserts based upon entry name
static void __insert_practice_entry(PRACTICE_DATA *data, PRACTICE_ENTRY_DATA *entry)
{
	ITERATOR it;
	PRACTICE_ENTRY_DATA *e;
	char *name = entry->skill ? entry->skill->name : entry->song->name;

	iterator_start(&it, data->entries);
	while((e = (PRACTICE_ENTRY_DATA *)iterator_nextdata(&it)))
	{
		char *nm = e->skill ? e->skill->name : e->song->name;

		if (str_cmp(name, nm) < 0)
		{
			iterator_insert_before(&it, entry);
			break;
		}
	}
	iterator_stop(&it);

	if (!e)
	{
		list_appendlink(data->entries, entry);
	}
}

// Inserts based upon minimum rating
static void __insert_entry_cost(PRACTICE_ENTRY_DATA *entry, PRACTICE_COST_DATA *cost)
{
	ITERATOR it;
	PRACTICE_COST_DATA *c;
	iterator_start(&it, entry->costs);
	while((c = (PRACTICE_COST_DATA *)iterator_nextdata(&it)))
	{
		if (cost->min_rating < c->min_rating)
		{
			iterator_insert_before(&it, cost);
			break;
		}
	}
	iterator_stop(&it);

	if (!c)
	{
		list_appendlink(entry->costs, cost);
	}
}

static void __practice_add_entry(PRACTICE_DATA *data, SKILL_DATA *skill, SONG_DATA *song)
{
	PRACTICE_ENTRY_DATA *entry = new_practice_entry_data();

	entry->skill = skill;
	entry->song = song;
	entry->max_rating = 75;		// Default max rating
	entry->reputation = NULL;

	__insert_practice_entry(data, entry);
	/*

	if (IS_VALID(song))
	{
		PRACTICE_COST_DATA *cost = new_practice_cost_data();
		cost->min_rating = 0;	// Acquire only
		cost->entry = entry;

		__insert_entry_cost(entry, cost);
	}
	*/					
}

static bool __practice_has_entry(PRACTICE_DATA *data, SKILL_DATA *skill, SONG_DATA *song)
{
	ITERATOR it;
	PRACTICE_ENTRY_DATA *entry;
	iterator_start(&it, data->entries);
	while((entry = (PRACTICE_ENTRY_DATA *)iterator_nextdata(&it)))
	{
		if ((skill && entry->skill == skill) ||
			(song && entry->song == song))
			break;
	}
	iterator_stop(&it);

	return entry != NULL;
}

static bool __practice_entry_has_cost(PRACTICE_ENTRY_DATA *entry, int rating)
{
	if (rating < 0 || rating > 100) return false;

	ITERATOR it;
	PRACTICE_COST_DATA *cost;
	iterator_start(&it, entry->costs);
	while((cost = (PRACTICE_COST_DATA *)iterator_nextdata(&it)))
	{
		if (cost->min_rating == rating)
			break;
	}
	iterator_stop(&it);

	return cost != NULL;
}






void correct_vrooms(WILDS_DATA *pWilds, WILDS_TERRAIN *pTerrain)
{
	register ROOM_INDEX_DATA *vroom;
	ITERATOR it;

	iterator_start(&it, pWilds->loaded_vrooms);

	while( (vroom = (ROOM_INDEX_DATA *)iterator_nextdata(&it)) ) {
		if(vroom->parent_template == pTerrain) {
			free_string(vroom->name);
			vroom->name = str_dup(pTerrain->template->name);
			vroom->room_flag[0] = pTerrain->template->room_flag[0];
			vroom->room_flag[1] = pTerrain->template->room_flag[1]|ROOM_VIRTUAL_ROOM;
				vroom->sector = pTerrain->template->sector;
				vroom->sector_flags = pTerrain->template->sector->flags;
		}
	}

	iterator_stop(&it);
}


