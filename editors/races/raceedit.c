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

#include "raceedit.h"

RACEEDIT( raceedit_show )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);
	BUFFER *buffer = new_buf();

	add_buf(buffer, formatf("%s Race: %s (%d)\n\r", (race->playable ? "Player" : "NPC"), race->name, race->uid));

	if (race->pgr)
		add_buf(buffer, formatf("GR: %s\n", gr_to_name(race->pgr)));
	else
		add_buf(buffer, "GR: {Dnone{x\n");

	add_buf(buffer, "Description:\n\r");
	add_buf(buffer, string_indent(race->description, 3));
	add_buf(buffer, "{x\n\r");

	add_buf(buffer, "Builder Comments:\n\r");
	add_buf(buffer, string_indent(race->comments, 3));
	add_buf(buffer, "{x\n\r");

	// TODO: Update when race flags are added
	add_buf(buffer, "Flags: none\n");

	add_buf(buffer, formatf("Act:           %s\n", bitmatrix_string(act_flagbank, race->act)));
	add_buf(buffer, formatf("Affected:      %s\n", bitmatrix_string(affect_flagbank, race->aff)));
	add_buf(buffer, formatf("Off:           %s\n", flag_string(off_flags, race->off)));
	add_buf(buffer, formatf("Immunity:      %s\n", flag_string(imm_flags, race->imm)));
	add_buf(buffer, formatf("Reistence:     %s\n", flag_string(imm_flags, race->res)));
	add_buf(buffer, formatf("Vulnerability: %s\n", flag_string(imm_flags, race->vuln)));
	add_buf(buffer, formatf("Form:          %s\n", flag_string(form_flags, race->form)));
	add_buf(buffer, formatf("Parts:         %s\n", flag_string(part_flags, race->parts)));

	if (race->playable)
	{
		add_buf(buffer, formatf("Starting:      %s{x\n", (race->starting ? "{WYES" : "{DNO")));
		add_buf(buffer, formatf("Remort:        %s{x\n", (race->remort ? "{WYES" : "{DNO")));

		if (IS_VALID(race->premort))
			add_buf(buffer, formatf("Remort Race:   %s\n", race->premort->name));
		else
			add_buf(buffer, "Remort Race:   {Dnone{x\n");

		add_buf(buffer, formatf("Who: %s{x\n\r", race->who));

		add_buf(buffer, "Stats:\n\r");
		for(int i = 0; i < MAX_STATS; i++)
		{
			add_buf(buffer, formatf(" - %-20s  %2d / %-2d\n\r", formatf("%s:", flag_string(stat_types, i)), race->stats[i], race->max_stats[i]));
		}

		add_buf(buffer, "Vitals:\n\r");
		for(int i = 0; i < 3; i++)
		{
			add_buf(buffer, formatf(" - %-10s  %d\n\r", formatf("%s:", flag_string(vital_types, i)), race->max_vitals[i]));
		}

		add_buf(buffer, formatf("Size: %s to %s\n\r", size_table[race->min_size].name, size_table[race->max_size].name));

		add_buf(buffer, formatf("Default Alignment: %d\n\r", race->default_alignment));

		if (list_size(race->skills) > 0)
		{
			add_buf(buffer, "Skills:\n\r");
			ITERATOR sit;
			SKILL_DATA *skill;
			iterator_start(&sit, race->skills);
			while((skill = (SKILL_DATA *)iterator_nextdata(&sit)))
			{
				add_buf(buffer, formatf(" - %s\n\r", skill->name));
			}
			iterator_stop(&sit);
			add_buf(buffer, "\n\r");
		}
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

RACEEDIT( raceedit_create )
{
	RACE_DATA *race;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  raceedit create <pc|npc> <name>\n\r", ch);
		send_to_char("Please specify {Wpc{x or {Wnpc{x.\n\r", ch);
		return false;
	}

	bool playable;

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "pc"))
	{
		playable = true;
	}
	else if (!str_prefix(arg, "npc"))
	{
		playable = false;
	}
	else
	{
		send_to_char("Syntax:  raceedit create <pc|npc> <name>\n\r", ch);
		send_to_char("Please specify {Wpc{x or {Wnpc{x.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  raceedit create <pc|npc> <name>\n\r", ch);
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	if ((race = get_race_data(argument)))
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	race = new_race_data();
	race->playable = playable;
	smash_tilde(argument);
	race->name = str_dup(argument);
	insert_race(race);
	save_races();

	olc_set_editor(ch, ED_RACEEDIT, race);

	send_to_char("Race created.\n\r", ch);
	return false;
}

RACEEDIT( raceedit_name )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	RACE_DATA *other = get_race_data(argument);
	if (IS_VALID(other) && race != other)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(race->name);
	race->name = str_dup(argument);

	list_remlink(race_list, race, false);
	insert_race(race);

	send_to_char("Race Name changed.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_description )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	string_append(ch, &race->description);
	return true;
}

RACEEDIT( raceedit_comments )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	string_append(ch, &race->comments);
	return true;
}

RACEEDIT( raceedit_flags )
{
	send_to_char("Not Implemented Yet.\n\r", ch);
	return false;
}

RACEEDIT( raceedit_act )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long bits[2];
	if (!bitmatrix_lookup(argument, act_flagbank, bits))
	{
		send_to_char("Invalid act flags.  Use '? act' for valid flags.\n\r", ch);
		show_help(ch, "act");
		return false;
	}

	TOGGLE_BIT(race->act[0], bits[0]);
	TOGGLE_BIT(race->act[1], bits[1]);
	send_to_char("Race Act toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_aff )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long bits[2];
	if (!bitmatrix_lookup(argument, affect_flagbank, bits))
	{
		send_to_char("Invalid affect flags.  Use '? affect' for valid flags.\n\r", ch);
		show_help(ch, "affect");
		return false;
	}

	TOGGLE_BIT(race->aff[0], bits[0]);
	TOGGLE_BIT(race->aff[1], bits[1]);
	send_to_char("Race Affect toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_off )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(off_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid offensive flags.  Use '? off' for valid list.\n\r", ch);
		show_flag_cmds(ch, off_flags);
		return false;
	}

	TOGGLE_BIT(race->off, value);
	send_to_char("Race Offensive toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_imm )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(imm_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid imm flags.  Use '? imm' for valid list.\n\r", ch);
		show_flag_cmds(ch, imm_flags);
		return false;
	}

	TOGGLE_BIT(race->imm, value);
	send_to_char("Race Immunity toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_res )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(imm_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid res flags.  Use '? res' for valid list.\n\r", ch);
		show_flag_cmds(ch, imm_flags);
		return false;
	}

	TOGGLE_BIT(race->res, value);
	send_to_char("Race Resistence toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_vuln )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(imm_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid vuln flags.  Use '? vuln' for valid list.\n\r", ch);
		show_flag_cmds(ch, imm_flags);
		return false;
	}

	TOGGLE_BIT(race->vuln, value);
	send_to_char("Race Vulnerability toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_form )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(form_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid form flags.  Use '? form' for valid list.\n\r", ch);
		show_flag_cmds(ch, form_flags);
		return false;
	}

	TOGGLE_BIT(race->form, value);
	send_to_char("Race Form toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_parts )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	long value;
	if ((value = flag_value(part_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid part flags.  Use '? parts' for valid list.\n\r", ch);
		show_flag_cmds(ch, part_flags);
		return false;
	}

	TOGGLE_BIT(race->parts, value);
	send_to_char("Race Parts toggled.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_gr )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  gr set <global race>\n\r", ch);
		send_to_char("         gr clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		RACE_DATA **gr = gr_from_name(argument);

		if (!gr)
		{
			send_to_char("Invalid global race.  Use '? gr' for valid races.\n\r", ch);
			show_help(ch, "gr");
			return false;
		}

		if (*gr) (*gr)->pgr = NULL;

		race->pgr = gr;
		*gr = race;

		send_to_char("Race Global Reference set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (!race->pgr)
		{
			send_to_char("Race has no global reference.\n\r", ch);
			return false;
		}

		*(race->pgr) = NULL;
		race->pgr = NULL;

		send_to_char("Race Global Reference cleared.\n\r", ch);
		return true;
	}

	raceedit_gr(ch, "");
	return false;
}

RACEEDIT( raceedit_starting )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (!race->playable)
	{
		send_to_char("May only modify player fields on playable races.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  starting <yes|no>\n\r", ch);
		return false;
	}

	bool starting;
	if (!str_prefix(argument, "yes"))
		starting = true;
	else if (!str_prefix(argument, "no"))
		starting = false;
	else
	{
		send_to_char("Please specify yes or no.\n\r", ch);
		return false;
	}

	if(starting && race->remort)
	{
		send_to_char("Unable to make a remort race as a starting race.\n\r", ch);
		return false;
	}

	race->starting = starting;
	send_to_char("Race Starting changed.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_remort )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (!race->playable)
	{
		send_to_char("May only modify remort fields on playable races.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		if (race->remort)
		{
			send_to_char("Syntax:  remort reset\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  remort assign <race name>\n\r", ch);
			send_to_char("         remort set\n\r", ch);
		}

		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (race->remort)
	{
		if (!str_prefix(arg, "reset"))
		{
			race->remort = false;

			// Find any playable race that remorts into this race and unlink it
			ITERATOR it;
			RACE_DATA *r;
			iterator_start(&it, race_list);
			while((r = (RACE_DATA *)iterator_nextdata(&it)))
			{
				if (r->playable && IS_VALID(r->premort) && r->premort == race)
				{
					r->premort = NULL;
				}
			}
			iterator_stop(&it);

			send_to_char("Race Remort Status reset.\n\r", ch);
			return true;
		}
	}
	else
	{
		if (!str_prefix(arg, "assign"))
		{
			if (race->remort)
			{
				send_to_char("Race is already remort.\n\r", ch);
				return false;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Please specify a race name.\n\r", ch);
				return false;
			}

			RACE_DATA *remort = get_race_data(argument);
			if (!IS_VALID(remort))
			{
				send_to_char("No such race by that name.\n\r", ch);
				return false;
			}

			if (!remort->playable)
			{
				send_to_char("That is not a playable race.\n\r", ch);
				return false;
			}

			if (!remort->remort)
			{
				send_to_char("That is not a remort race.\n\r", ch);
				return false;
			}

			race->premort = remort;
			send_to_char("Race Remort Reference assigned.\n\r", ch);
			return true;
		}
		else if (!str_prefix(arg, "set"))
		{
			if(race->starting)
			{
				send_to_char("Unable to make a starting race as a remort race.\n\r", ch);
				return false;
			}

			// Unassign the remort linkage
			race->premort = NULL;
			race->remort = true;
			send_to_char("Race Remort Status set.\n\r", ch);
			return true;
		}
	}

	raceedit_remort(ch, "");
	return false;
}

RACEEDIT( raceedit_who )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  who <string>\n\r", ch);
		return false;
	}

	if (strlen_no_colours(argument) > 6)
	{
		send_to_char("Who string cannot be longer than 6 plain characters.\n\r", ch);
		return false;
	}

	free_string(race->who);
	race->who = str_dup(argument);

	send_to_char("Race Who set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_stats )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  stats <stat> <starting value>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	int stat;
	if ((stat = stat_lookup(arg, stat_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  stats <stat> <starting value>\n\r", ch);
		send_to_char("Invalid stat.  Use '? stat' for valid types.\n\r", ch);
		show_flag_cmds(ch, stat_types);
		return false;
	}

	int value;
	if (!is_number(argument) || (value = atoi(argument)) < 1)
	{
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	race->stats[stat] = value;
	send_to_char("Race Starting Stat set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_maxstats )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  maxstats <stat> <maximum>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	int stat;
	if ((stat = stat_lookup(arg, stat_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  maxstats <stat> <maximum>\n\r", ch);
		send_to_char("Invalid stat.  Use '? stat' for valid types.\n\r", ch);
		show_flag_cmds(ch, stat_types);
		return false;
	}

	int value;
	if (!is_number(argument) || (value = atoi(argument)) < 1)
	{
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	race->max_stats[stat] = value;
	send_to_char("Race Max Stat set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_maxvitals )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  maxvitals <hp|mana|move> <maximum>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	int vital;
	if ((vital = stat_lookup(arg, vital_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  maxvitals <hp|mana|move> <maximum>\n\r", ch);
		send_to_char("Please specify {Whp{x, {Wmana{x or {Wmove{x.\n\r", ch);
		return false;
	}

	int value;
	if (!is_number(argument) || (value = atoi(argument)) < 1)
	{
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	race->max_vitals[vital] = value;
	send_to_char("Race Max Vital set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_size )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (!race->playable)
	{
		send_to_char("Race must be playable.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  size <minimum> <maximum>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	int min_size;
	if ((min_size = flag_value(size_flags, arg)) == NO_FLAG)
	{
		send_to_char("Invalid size.  Use '? size' for valid list.\n\r", ch);
		show_flag_cmds(ch, size_flags);
		return false;
	}

	int max_size;
	if ((max_size = flag_value(size_flags, arg)) == NO_FLAG)
	{
		send_to_char("Invalid size.  Use '? size' for valid list.\n\r", ch);
		show_flag_cmds(ch, size_flags);
		return false;
	}

	if (min_size > max_size)
	{
		send_to_char("Minimum size is bigger than the maximum.\n\r", ch);
		return false;
	}

	race->min_size = min_size;
	race->max_size = max_size;
	send_to_char("Race Sizing set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_align )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (!race->playable)
	{
		send_to_char("Race must be playable.\n\r", ch);
		return false;
	}

	int alignment;
	if (!is_number(argument) || (alignment = atoi(argument)) < -1000 || alignment > 1000)
	{
		send_to_char("Please specify an alignment from -1000 to 1000.\n\r", ch);
		return false;
	}

	race->default_alignment = alignment;
	send_to_char("Race Default Alignment set.\n\r", ch);
	return true;
}

RACEEDIT( raceedit_skills )
{
	RACE_DATA *race;

	EDIT_RACE(ch, race);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skills add <skill name>\n\r", ch);
		send_to_char("         skills remove <skill name>\n\r", ch);
		send_to_char("         skills clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "add"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skills add <skill name>\n\r", ch);
			return false;
		}

		SKILL_DATA *skill = get_skill_data(argument);
		if (!IS_VALID(skill))
		{
			send_to_char("No such skill by that name.\n\r", ch);
			return false;
		}

		if (list_size(skill->levels) > 0)
		{
			send_to_char("Skill may not have class restrictions on it.\n\r", ch);
			return false;
		}

		if (list_contains(race->skills, skill, NULL))
		{
			send_to_char("Race already has that skill.\n\r", ch);
			return false;
		}

		list_appendlink(race->skills, skill);
		send_to_char("Skill added to race.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		if (list_size(race->skills) < 1)
		{
			send_to_char("Race has no skills.\n\r", ch);
			return false;
		}
		
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skills remove <skill name>\n\r", ch);
			return false;
		}

		SKILL_DATA *skill = get_skill_data(argument);
		if (!IS_VALID(skill))
		{
			send_to_char("No such skill by that name.\n\r", ch);
			return false;
		}

		if (!list_contains(race->skills, skill, NULL))
		{
			send_to_char("Race does not have that skill.\n\r", ch);
			return false;
		}

		list_remlink(race->skills, skill, false);
		send_to_char("Skill removed from race.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (list_size(race->skills) > 0)
		{
			list_clear(race->skills);
			send_to_char("Race skills cleared.\n\r", ch);
			return true;
		}

		send_to_char("Race has no skills.\n\r", ch);
		return false;
	}

	raceedit_skills(ch, "");
	return false;
}