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

#include "clsedit.h"


CLSEDIT( clsedit_show )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	BUFFER *buffer = new_buf();

	add_buf(buffer, formatf("Class: %s (%d)\n\r", clazz->name, clazz->uid));

	if (clazz->gcl)
	{
		char *gcl_name = gcl_to_name(clazz->gcl);
		add_buf(buffer, formatf("GCL: %s\n\r", gcl_name ? gcl_name : "{Rinvalid{x"));
	}
	else
		add_buf(buffer, "GCL: {Dnone{x\n\r");

	add_buf(buffer, formatf("Type: %s\n\r", flag_string(class_types, clazz->type)));
	add_buf(buffer, formatf("Flags: %s\n\r", flag_string(class_flags, clazz->flags)));

	add_buf(buffer, "Description:\n\r");
	add_buf(buffer, string_indent(clazz->description, 3));
	add_buf(buffer, "\n\r");

	add_buf(buffer, "Display:\n\r");
	add_buf(buffer, formatf("  Neuter: %s\n\r", clazz->display[SEX_NEUTRAL]));
	add_buf(buffer, formatf("  Male:   %s\n\r", clazz->display[SEX_MALE]));
	add_buf(buffer, formatf("  Female: %s\n\r", clazz->display[SEX_FEMALE]));
	add_buf(buffer, formatf("  Either: %s\n\r", clazz->display[SEX_EITHER]));
	add_buf(buffer, "\n\r");

	add_buf(buffer, "Who:\n\r");
	add_buf(buffer, formatf("  Neuter: %s\n\r", clazz->who[SEX_NEUTRAL]));
	add_buf(buffer, formatf("  Male:   %s\n\r", clazz->who[SEX_MALE]));
	add_buf(buffer, formatf("  Female: %s\n\r", clazz->who[SEX_FEMALE]));
	add_buf(buffer, formatf("  Either: %s\n\r", clazz->who[SEX_EITHER]));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("Maximum Level: %d\n", clazz->max_level));

	add_buf(buffer, formatf("Primary Stat: %s\n\r", flag_string(stat_types, clazz->primary_stat)));

	add_buf(buffer, "Groups:\n\r");
	if (list_size(clazz->groups) > 0)
	{
		ITERATOR git;
		SKILL_GROUP *group;
		iterator_start(&git, clazz->groups);
		while((group = (SKILL_GROUP *)iterator_nextdata(&git)))
		{
			add_buf(buffer, formatf("  %s\n\r", group->name));
		}
		iterator_stop(&git);
	}
	else
		add_buf(buffer, "  None\n\r");

	// TODO: traits

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

CLSEDIT( clsedit_create )
{
	CLASS_DATA *clazz;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  clsedit create <name>\n\r", ch);
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	if ((clazz = get_class_data(argument)))
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	clazz = new_class_data();
	smash_tilde(argument);
	clazz->name = str_dup(argument);
	insert_class(clazz);
	save_classes(false);

	olc_set_editor(ch, ED_CLSEDIT, clazz);

	send_to_char("Class created.\n\r", ch);
	return true;
}

CLSEDIT( clsedit_name )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	smash_tilde(argument);

	if (argument[0] == '\0')
	{
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	CLASS_DATA *other = get_class_data(argument);
	if (IS_VALID(other) && clazz != other)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(clazz->name);
	clazz->name = str_dup(argument);

	list_remlink(classes_list, clazz, false);
	insert_class(clazz);

	send_to_char("Class Name changed.\n\r", ch);
	return true;
}

CLSEDIT( clsedit_description )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	string_append(ch, &clazz->description);
	return true;
}

CLSEDIT( clsedit_display )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  clsedit display <sex> <who string>\n\r", ch);
		send_to_char("Valid sex: neuter, male, female, either, or all\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Please provide a display string.\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "all"))
	{
		for(int sex = 0; sex < SEX_MAX; sex++)
		{
			free_string(clazz->display[sex]);
			clazz->display[sex] = str_dup(argument);
		}

		send_to_char("DISPLAY string set for all sexes.\n\r", ch);
	}
	else
	{
		int sex;
		if ((sex = sex_lookup(arg)) == -1)
		{
			send_to_char("Syntax:  clsedit display <sex> <who string>\n\r", ch);
			send_to_char("Valid sex: neuter, male, female, either, or all\n\r", ch);
			return false;
		}

		free_string(clazz->display[sex]);
		clazz->display[sex] = str_dup(argument);
		send_to_char(formatf("DISPLAY string set for {+%s.\n\r", flag_string(sex_table, sex)), ch);
	}
	return true;
}

CLSEDIT( clsedit_type )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	int type;
	if ((type = stat_lookup(argument, class_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid class type.  Use '? classtypes' for valid list.\n\r", ch);
		show_flag_cmds(ch, class_types);
		return false;
	}

	clazz->type = type;
	send_to_char("Class Type changed.\n\r", ch);
	return true;
}

CLSEDIT( clsedit_maxlevel )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	int level;
	if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
	{
		send_to_char(formatf("Please specify a number from 1 to %d.\n", MAX_CLASS_LEVEL), ch);
		return false;
	}

	clazz->max_level = level;
	send_to_char("Class Maximum Level set.\n\r", ch);
	return true;
}

CLSEDIT( clsedit_primary )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	int stat;
	if ((stat = stat_lookup(argument, stat_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid stat type.  Use '? stats' for valid list.\n\r", ch);
		show_flag_cmds(ch, stat_types);
		return false;
	}

	clazz->primary_stat = stat;
	send_to_char("Class Primary Stat changed.\n\r", ch);
	return true;
}

CLSEDIT( clsedit_skills )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skills add <group name>\n\r", ch);
		send_to_char("         skills remove <group name>\n\r", ch);
		send_to_char("         skills clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "add"))
	{
		SKILL_GROUP *group = group_lookup(argument);
		if (!IS_VALID(group))
		{
			send_to_char("No such skill group by that name.  Use 'sglist' for list of skill groups.\n\r", ch);
			return false;
		}

		if (list_contains(clazz->groups, group, NULL))
		{
			send_to_char("Class already has that skill group.\n\r", ch);
			return false;
		}

		list_appendlink(clazz->groups, group);
		send_to_char("Skill group added to class.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		SKILL_GROUP *group = group_lookup(argument);
		if (!IS_VALID(group))
		{
			send_to_char("No such skill group by that name.  Use 'sglist' for list of skill groups.\n\r", ch);
			return false;
		}

		if (!list_contains(clazz->groups, group, NULL))
		{
			send_to_char("Class does not have that skill group.\n\r", ch);
			return false;
		}

		list_remlink(clazz->groups, group, false);
		send_to_char("Skill group removed from class.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (list_size(clazz->groups) < 1)
		{
			send_to_char("Class does not have any skill groups assigned.\n\r", ch);
			return false;
		}

		list_clear(clazz->groups);
		send_to_char("Class skill groups cleared.\n\r", ch);
		return true;
	}

	clsedit_skills(ch, "");
	return true;
}

CLSEDIT( clsedit_who )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  clsedit who <sex> <who string>\n\r", ch);
		send_to_char("Valid sex: neuter, male, female, either, or all.\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Please provide a who string.\n\r", ch);
		return false;
	}

	int len = strlen_no_colours(argument);
	if (len > 12)
	{
		send_to_char("Please limit who string to 12 plain characters.\n\r", ch);
		return false;
	}

	if (!str_prefix(arg, "all"))
	{
		for(int sex = 0; sex < SEX_MAX; sex++)
		{
			free_string(clazz->who[sex]);
			clazz->who[sex] = str_dup(argument);
		}

		send_to_char("WHO string set for all sexes.\n\r", ch);
	}
	else
	{
		int sex;
		if ((sex = sex_lookup(arg)) == -1)
		{
			send_to_char("Syntax:  clsedit who <sex> <who string>\n\r", ch);
			send_to_char("Valid sex: neuter, male, female, either, or all\n\r", ch);
			return false;
		}

		free_string(clazz->who[sex]);
		clazz->who[sex] = str_dup(argument);
		send_to_char(formatf("WHO string set for {+%s.\n\r", flag_string(sex_table, sex)), ch);
	}
	return true;
}

CLSEDIT( clsedit_gcl )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  clsedit gcl set <global class>\n\r", ch);
		send_to_char("         clsedit gcl clear\n\r", ch);
		return false;
	}

	char arg[MIL];

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  clsedit gcl set <global class>\n\r", ch);
			return false;
		}

		CLASS_DATA **gcl = gcl_from_name(argument);
		if (!gcl)
		{
			send_to_char("No such global class by that name.\n\r", ch);
			send_to_char("Use '? gcl' to get a list of valid names.\n\r", ch);
			show_help(ch, "gcl");
			return false;
		}

		if (*gcl) (*gcl)->gcl = NULL;	// Unassign the previous class

		*gcl = clazz;
		clazz->gcl = gcl;
		send_to_char("Class GCL set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (clazz->gcl) *(clazz->gcl) = NULL;
		clazz->gcl = NULL;

		send_to_char("Class GCL cleared.\n\r", ch);
		return true;
	}

	clsedit_gcl(ch, "");
	return true;
}

CLSEDIT( clsedit_flags )
{
	CLASS_DATA *clazz;

	EDIT_CLASS(ch, clazz);

	long value;
	if ((value = flag_value(class_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid class flag.  Use '? class' for list of valid flags:\n\r", ch);
		show_flag_cmds(ch, class_flags);
		return false;
	}

	TOGGLE_BIT(clazz->flags, value);
	send_to_char("Class flags toggled.\n\r", ch);
	return true;
}