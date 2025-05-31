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

 #include "corpsedit.h"

 CORPSEDIT( corpsedit_create )
{
	CORPSE_TYPE *corpse;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  corpsedit create <name>\n\r", ch);
		send_to_char("Please provide a name.\n\r", ch);
		return false;
	}

	if ((corpse = get_corpse_type(argument)))
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	corpse = new_corpse_type();
	smash_tilde(argument);
	corpse->name = str_dup(argument);
	insert_corpse(corpse);
	save_corpses();

	olc_set_editor(ch, ED_CORPSEDIT, corpse);

	send_to_char("Corpse created.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_show )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	BUFFER *buffer = new_buf();

	add_buf(buffer, formatf("{yCORPSE{Y[{W%s{Y]{y: {W%ld{x\n\r", corpse->name, corpse->uid));
	if (corpse->gcrp)
		add_buf(buffer, formatf("{Y[{WGSCP{Y]{y: {W%s{x\n\r", gcrp_to_display(corpse->gcrp)));
	else
		add_buf(buffer, "{Y[{WGSCP{Y]{y: {D(unset){x\n\r");

	if (!IS_NULLSTR(corpse->comments))
		add_buf(buffer, formatf("  -----\n\r  {WBuilders' Comments:{X\n\r  %s{x\n\r  -----\n\r\n\r", corpse->comments));

	add_buf(buffer, formatf("{Y[{WKeywords{Y]{y: {W%s{x\n\r", corpse->keywords));
	add_buf(buffer, formatf("{Y[{WShort Description{Y]{y: {W%s{x\n\r", corpse->short_descr));
	add_buf(buffer, formatf("{Y[{WLong Description{Y]{y: {W%s{x\n\r", corpse->long_descr));
	add_buf(buffer, formatf("{Y[{WDescription{Y]{y:{x\n\r%s\n\r", string_indent(corpse->full_descr, 3)));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("{Y[{WHeadless?{Y]{y: %s{x\n\r", (corpse->headless ? "{WYES" : "{DNO")));
	add_buf(buffer, formatf("{Y[{WHeadless Can Be Animated?{Y]{y: %s{x\n\r", (corpse->animate_headless ? "{WYES" : "{DNO")));
	add_buf(buffer, formatf("{Y[{WHeadless Short Description{Y]{y: {W%s{x\n\r", corpse->short_headless));
	add_buf(buffer, formatf("{Y[{WHeadless Long Description{Y]{y: {W%s{x\n\r", corpse->long_headless));
	add_buf(buffer, formatf("{Y[{WHeadless Description{Y]{y:{x\n\r%s\n\r", string_indent(corpse->full_headless, 3)));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("{Y[{WResurrection Chance{Y]{y: {W%d%%{x\n\r", corpse->resurrect_chance));
	add_buf(buffer, formatf("{Y[{WAnimation Chance{Y]{y: {W%d%%{x\n\r", corpse->animation_chance));
	add_buf(buffer, formatf("{Y[{WAnimated Name{Y]{y: {W%s{x\n\r", corpse->animate_name));
	add_buf(buffer, formatf("{Y[{WAnimated Long Description{Y]{y: {W%s{x\n\r", corpse->animate_long));
	add_buf(buffer, formatf("{Y[{WAnimated Description{Y]{y:{x\n\r%s\n\r", string_indent(corpse->animate_descr, 3)));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("{Y[{WVictim Death Message{Y]{y: {W%s{x\n\r", corpse->victim_message));
	add_buf(buffer, formatf("{Y[{WRoom Death Message{Y]{y: {W%s{x\n\r", corpse->room_message));
	add_buf(buffer, formatf("{Y[{WDecay Message{Y]{y: {W%s{x\n\r", corpse->decay_message));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("{Y[{WSkulling Chance{Y]{y: {W%d%%{x\n\r", corpse->skulling_chance));
	add_buf(buffer, formatf("{Y[{WSkull Success Message{Y]{y: {W%s{x\n\r", corpse->skull_success));
	add_buf(buffer, formatf("{Y[{WSkull Success Room Message{Y]{y: {W%s{x\n\r", corpse->skull_success_other));
	add_buf(buffer, formatf("{Y[{WSkull Failure Message{Y]{y: {W%s{x\n\r", corpse->skull_fail));
	add_buf(buffer, formatf("{Y[{WSkull Failure Room Message{Y]{y: {W%s{x\n\r", corpse->skull_fail_other));
	add_buf(buffer, "\n\r");

	add_buf(buffer, formatf("{Y[{WOwner Loot{Y]{y: %s{x\n\r", (corpse->owner_loot ? "{WYES" : "{DNO")));
	
	if (IS_VALID(corpse->decay_type))
		add_buf(buffer, formatf("{Y[{WCorpse Decays Into{Y]{y: {W%s{x\n\r", corpse->decay_type->name));
	else
		add_buf(buffer, "{Y[{WCorpse Decays Into{Y]{y: {D(nothing){x\n\r");

	if (corpse->decay_rate > 100)
		add_buf(buffer, formatf("{Y[{WCorpse Degradation Rate{Y]{y: {W%d%%{y per tick with {W%d%%{y chance for an additional percent{x\n\r", (corpse->decay_rate / 100), (corpse->decay_rate % 100)));
	else if (corpse->decay_rate == 100)
		add_buf(buffer, "{Y[{WCorpse Degradation Rate{Y]{y: {W1%%{y per tick{x\n\r");
	else if (corpse->decay_rate > 0)	
		add_buf(buffer, formatf("{Y[{WCorpse Degradation Rate{Y]{y: {W%d%%{y chance to degrade {W1%%{y per tick{x\n\r", corpse->decay_rate));
	else
		add_buf(buffer, "{Y[{WCorpse Degradation Rate{Y]{y: {D(never){x\n\r");

	add_buf(buffer, formatf("{Y[{WNPC Corpse Decay Timer{Y]{y: {W%d{y to {W%d{y ticks{x\n\r", corpse->decay_npctimer_min, corpse->decay_npctimer_max));
	add_buf(buffer, formatf("{Y[{WPC Corpse Decay Timer{Y]{y: {W%d{y to {W%d{y ticks{x\n\r", corpse->decay_pctimer_min, corpse->decay_pctimer_max));
	add_buf(buffer, formatf("{Y[{WDecay Spill Chance{Y]{y: {W%d%%{x\n\r", corpse->decay_spill_chance));
	add_buf(buffer, formatf("{Y[{WBodyparts Lost{Y]{y: {W%s{x\n\r", flag_string(part_flags, corpse->lost_bodyparts)));

	if (list_size(corpse->damage_table) > 0)
	{
		add_buf(buffer, "{Y[{WDamage Table{Y]{y:{x\n\r");
		int d = 0;
		int b;
		ITERATOR dit, bit;
		CORPSE_DAMAGE *damage;
		CORPSE_BLENDING *blend;
		iterator_start(&dit, corpse->damage_table);
		while((damage = (CORPSE_DAMAGE *)iterator_nextdata(&dit)))
		{
			add_buf(buffer, formatf("%3d) %s\n\r", ++d, flag_string(damage_classes, damage->damage_type)));
			
			b = 0;
			iterator_start(&bit, damage->blending);
			while((blend = (CORPSE_BLENDING *)iterator_nextdata(&bit)))
			{
				if (IS_VALID(blend->result))
					add_buf(buffer, formatf("     %3d)  %s\n\r", ++b, MXPCreateSend(ch->desc, formatf("corpseshow %s", blend->result->name), blend->result->name)));
				else
					add_buf(buffer, formatf("     %3d)  {Dnocorpse{x\n\r", ++b));
			}
			iterator_stop(&bit);
		}
		iterator_stop(&dit);
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

CORPSEDIT( corpsedit_gcrp )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  gcrp set <name>\n\r", ch);
		send_to_char("         gcrp clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "set"))
	{
		CORPSE_TYPE **gcrp = gcrp_from_name(argument);

		if (!gcrp)
		{
			send_to_char("No such global corpse pointer by that name.\n\r", ch);
			return false;
		}

		if (*gcrp) (*gcrp)->gcrp = NULL;
		*gcrp = corpse;
		corpse->gcrp = gcrp;

		send_to_char("Global Corpse Pointer updated.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (corpse->gcrp)
		{
			*(corpse->gcrp) = NULL;
			corpse->gcrp = NULL;
		}

		send_to_char("Global Corpse Pointer cleared.\n\r", ch);
		return true;
	}

	corpsedit_gcrp(ch, "");
	return false;
}

CORPSEDIT( corpsedit_comments )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0])
	{
		send_to_char("Syntax:  comments (opens string editor)\n\r", ch);
		return false;
	}

	string_append(ch, &corpse->comments);
	return true;
}

CORPSEDIT( corpsedit_name )
{
	CORPSE_TYPE *corpse, *other;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  corpsedit name <name>\n\r", ch);
		send_to_char("Please specify a name.\n\r", ch);
		return false;
	}
	smash_tilde(argument);

	other = get_corpse_type(argument);
	if (IS_VALID(other) && corpse != other)
	{
		send_to_char("That name is already in use.\n\r", ch);
		return false;
	}

	free_string(corpse->name);
	corpse->name = str_dup(argument);

	// Reposition corpse
	list_remlink(corpse_list, corpse, false);
	insert_corpse(corpse);

	send_to_char("Corpse name set.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_keywords )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  keywords <string>\n\r", ch);
		return false;
	}

	smash_tilde(argument);
	free_string(corpse->keywords);
	corpse->keywords = str_dup(argument);
	send_to_char("Corpse Keywords changed.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_short )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  short <string>\n\r", ch);
		return false;
	}

	smash_tilde(argument);
	free_string(corpse->short_descr);
	corpse->short_descr = str_dup(argument);
	send_to_char("Corpse Short Description changed.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_long )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  long <string>\n\r", ch);
		return false;
	}

	smash_tilde(argument);
	free_string(corpse->long_descr);
	corpse->long_descr = str_dup(argument);
	send_to_char("Corpse Long Description changed.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_description )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0])
	{
		send_to_char("Syntax:  description (opens string editor)\n\r", ch);
		return false;
	}

	string_append(ch, &corpse->full_descr);
	return true;
}

// yes/no, short, long, description
CORPSEDIT( corpsedit_headless )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  corspedit headless <boolean>\n\r", ch);
		send_to_char("         corspedit headless short <string>\n\r", ch);
		send_to_char("         corspedit headless long <string>\n\r", ch);
		send_to_char("         corspedit headless description (opens string editor)\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "short"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  headless short <string>\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->short_headless);
		corpse->short_headless = str_dup(argument);
		send_to_char("Corpse Headless Short description changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "long"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  headless long <string>\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->long_headless);
		corpse->long_headless = str_dup(argument);
		send_to_char("Corpse Headless Long description changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "description"))
	{
		if (argument[0])
		{
			send_to_char("Syntax:  description\n\r", ch);
			return false;
		}

		string_append(ch, &corpse->full_headless);
		return true;
	}

	bool state;
	if (!str_prefix(arg, "true") || !str_prefix(arg, "yes") || !str_prefix(arg, "on"))
		state = true;
	else if (!str_prefix(arg, "false") || !str_prefix(arg, "no") || !str_prefix(arg, "off"))
		state = false;
	else
	{
		send_to_char("Please specify either short, long, description or provide a boolean value.\n\r", ch);
		return false;
	}

	corpse->headless = state;
	send_to_char("Corpse Headless state changed\n\r", ch);
	return true;
}

// name, long, description, headlesss
CORPSEDIT( corpsedit_animate )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  animate name <name>\n\r", ch);
		send_to_char("         animate long <string>\n\r", ch);
		send_to_char("         animate description (opens string editor)\n\r", ch);
		send_to_char("         animate headless <boolean>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "name"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  animate name <name>\n\r", ch);
			send_to_char("Please provide a name.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->animate_name);
		corpse->animate_name = str_dup(argument);
		send_to_char("Corpse Animated Name changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "long"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  animate long <string>\n\r", ch);
			send_to_char("Please provide a room description.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->animate_long);
		corpse->animate_long = str_dup(argument);
		send_to_char("Corpse Animated Long Description changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "description"))
	{
		if (argument[0] != '\0')
		{
			send_to_char("Syntax:  animate description (opens string editor)\n\r", ch);
			return false;
		}

		string_append(ch, &corpse->animate_descr);
		return true;
	}

	if (!str_prefix(arg, "headless"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  animate headless <boolean>\n\r", ch);
			send_to_char("Please provide a boolean value (true/false, yes/no, on/off).\n\r", ch);
			return false;
		}

		bool state;
		if (!str_prefix(argument, "true") || !str_prefix(argument, "yes") || !str_prefix(argument, "on"))
			state = true;
		else if (!str_prefix(argument, "false") || !str_prefix(argument, "no") || !str_prefix(argument, "off"))
			state = false;
		else
		{
			send_to_char("Syntax:  animate headless <boolean>\n\r", ch);
			send_to_char("Please provide a boolean value (true/false, yes/no, on/off).\n\r", ch);
			return false;
		}

		corpse->animate_headless = state;
		send_to_char("Corpse Allow Animated Headless changed.\n\r", ch);
		return true;
	}

	corpsedit_animate(ch, "");
	return false;
}

// decay, room and victim
CORPSEDIT( corpsedit_message )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  message decay set <string>\n\r", ch);
		send_to_char("         message decay clear\n\r", ch);
		send_to_char("         message room set <string>\n\r", ch);
		send_to_char("         message room clear\n\r", ch);
		send_to_char("         message victim set <string>\n\r", ch);
		send_to_char("         message victim clear\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "decay"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  message decay set <string>\n\r", ch);
			send_to_char("         message decay clear\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "set"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  message decay set <string>\n\r", ch);
				send_to_char("Please provide a decay message.\n\r", ch);
				return false;
			}
			smash_tilde(argument);

			free_string(corpse->decay_message);
			corpse->decay_message = str_dup(argument);
			send_to_char("Corpse Decay Message changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(corpse->decay_message);
			corpse->decay_message = NULL;
			send_to_char("Corpse Decay Message cleared.\n\r", ch);
			return true;
		}

		corpsedit_message(ch, "decay");
		return false;
	}

	if (!str_prefix(arg, "room"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  message room set <string>\n\r", ch);
			send_to_char("         message room clear\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "set"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  message room set <string>\n\r", ch);
				send_to_char("Please provide a room death message.\n\r", ch);
				return false;
			}
			smash_tilde(argument);

			free_string(corpse->room_message);
			corpse->room_message = str_dup(argument);
			send_to_char("Corpse Room Death Message changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(corpse->room_message);
			corpse->room_message = NULL;
			send_to_char("Corpse Room Death Message cleared.\n\r", ch);
			return true;
		}

		corpsedit_message(ch, "room");
		return false;
	}

	if (!str_prefix(arg, "victim"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  message victim set <string>\n\r", ch);
			send_to_char("         message victim clear\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "set"))
		{
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  message victim set <string>\n\r", ch);
				send_to_char("Please provide a victim death message.\n\r", ch);
				return false;
			}
			smash_tilde(argument);

			free_string(corpse->victim_message);
			corpse->victim_message = str_dup(argument);
			send_to_char("Corpse Victim Death Message changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(corpse->victim_message);
			corpse->victim_message = NULL;
			send_to_char("Corpse Victim Death Message cleared.\n\r", ch);
			return true;
		}

		corpsedit_message(ch, "victim");
		return false;
	}

	corpsedit_message(ch, "");
	return false;
}

// success, successother, fail, failother
CORPSEDIT( corpsedit_skull )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skull success <message>\n\r", ch);
		send_to_char("         skull successother <message>\n\r", ch);
		send_to_char("         skull fail <message>\n\r", ch);
		send_to_char("         skull failother <message>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "success"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skull success <message>\n\r", ch);
			send_to_char("Please provide a skull success message.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->skull_success);
		corpse->skull_success = str_dup(argument);
		send_to_char("Corpse Skull Success Message changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "successother"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skull successother <message>\n\r", ch);
			send_to_char("Please provide a skull success room message.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->skull_success_other);
		corpse->skull_success_other = str_dup(argument);
		send_to_char("Corpse Skull Success Room Message changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "fail"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skull fail <message>\n\r", ch);
			send_to_char("Please provide a skull fail message.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->skull_fail);
		corpse->skull_fail = str_dup(argument);
		send_to_char("Corpse Skull Failure Message changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "failother"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skull failother <message>\n\r", ch);
			send_to_char("Please provide a skull fail room message.\n\r", ch);
			return false;
		}
		smash_tilde(argument);

		free_string(corpse->skull_fail_other);
		corpse->skull_fail_other = str_dup(argument);
		send_to_char("Corpse Skull Failure Room Message changed.\n\r", ch);
		return true;
	}

	corpsedit_skull(ch, "");
	return false;
}

CORPSEDIT( corpsedit_owner_loot )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		corpse->owner_loot = !corpse->owner_loot;
		send_to_char("Corpse Owner Loot toggled.\n\r", ch);
		return true;
	}

	bool value;
	if (!str_prefix(argument, "true") || !str_prefix(argument, "yes") || !str_prefix(argument, "on"))
		value = true;
	else if (!str_prefix(argument, "false") || !str_prefix(argument, "no") || !str_prefix(argument, "off"))
		value = false;
	else
	{
		send_to_char("Please provide a boolean value (true/false, yes/no or on/off).\n\r", ch);
		return false;
	}

	corpse->owner_loot = value;
	send_to_char("Corpse Owner Loot changed.\n\r", ch);
	return true;
}

// resurrect, animation, skulling
CORPSEDIT( corpsedit_chance )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  chance animation <percent>\n\r", ch);
		send_to_char("         chance resurrect <percent>\n\r", ch);
		send_to_char("         chance skulling <percent>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "animation"))
	{
		int percent;
		if (!is_number(argument) || (percent = atoi(argument)) < 0 || percent > 100)
		{
			send_to_char("Syntax:  chance animation <percent>\n\r", ch);
			send_to_char("Please provide a percentage (0 to 100).\n\r", ch);
			return false;
		}

		corpse->animation_chance = percent;
		send_to_char("Corpse Animation Chance changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "resurrect"))
	{
		int percent;
		if (!is_number(argument) || (percent = atoi(argument)) < 0 || percent > 100)
		{
			send_to_char("Syntax:  chance resurrect <percent>\n\r", ch);
			send_to_char("Please provide a percentage (0 to 100).\n\r", ch);
			return false;
		}

		corpse->resurrect_chance = percent;
		send_to_char("Corpse Resurrection Chance changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "skulling"))
	{
		int percent;
		if (!is_number(argument) || (percent = atoi(argument)) < 0 || percent > 100)
		{
			send_to_char("Syntax:  chance skulling <percent>\n\r", ch);
			send_to_char("Please provide a percentage (0 to 100).\n\r", ch);
			return false;
		}

		corpse->skulling_chance = percent;
		send_to_char("Corpse Skulling Chance changed.\n\r", ch);
		return true;
	}

	corpsedit_chance(ch, "");
	return false;
}

// type, rate, timer, spill
CORPSEDIT( corpsedit_decay )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  decay type <corpse type>\n\r", ch);
		send_to_char("         decay rate <rate>\n\r", ch);
		send_to_char("         decay timer npc|pc <min ticks> <max ticks>\n\r", ch);
		send_to_char("         decay spill <percent>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "type"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  decay type set <corpse type>\n\r", ch);
			send_to_char("Syntax:  decay type clear\n\r", ch);
			send_to_char("For list of corpse types, do 'corpselist'.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		if (!str_prefix(arg, "set"))
		{
			CORPSE_TYPE *decay = get_corpse_type(argument);
			if (!IS_VALID(decay))
			{
				send_to_char("No such corpse type by that name.\n\r", ch);
				return false;
			}

			corpse->decay_type = decay;
			send_to_char("Corpse Decay Type changed.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			corpse->decay_type = NULL;
			send_to_char("Corpse Decay Type cleared.\n\r", ch);
			return true;
		}

		corpsedit_decay(ch, "type");
		return false;
	}

	if (!str_prefix(arg, "rate"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  decay rate <rate>\n\r", ch);
			send_to_char("Every 100% rate causes a 1% of corpse degredation per tick.\n\r", ch);
			send_to_char("All excess results in a percent chance of the final 1% loss.\n\r", ch);
			return false;
		}

		int rate;
		if (!is_number(argument) || (rate = atoi(argument)) < 0)
		{
			send_to_char("Please provide a non-negative number.\n\r", ch);
			send_to_char("Every 100% rate causes a 1% of corpse degredation per tick.\n\r", ch);
			send_to_char("All excess results in a percent chance of the final 1% loss.\n\r", ch);
			return false;
		}

		corpse->decay_rate = rate;
		send_to_char("Corpse Decay Rate changed.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "timer"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  decay npc|pc <min ticks> <max ticks>\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		bool pc;
		if (!str_prefix(arg, "npc"))
			pc = false;
		else if (!str_prefix(arg, "pc"))
			pc = true;
		else
		{
			send_to_char("Syntax:  decay npc|pc <min ticks> <max ticks>\n\r", ch);
			send_to_char("Specify either npc or pc.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);

		int mn;
		if (!is_number(arg) || (mn = atoi(arg)) < 1)
		{
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}

		int mx;
		if (!is_number(argument) || (mx = atoi(argument)) < 1)
		{
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}

		if (pc)
		{
			corpse->decay_pctimer_min = UMIN(mn,mx);
			corpse->decay_pctimer_max = UMAX(mn,mx);
			send_to_char("Corpse Decay Timer (PC Corpse) changed.\n\r", ch);
		}
		else
		{
			corpse->decay_npctimer_min = UMIN(mn,mx);
			corpse->decay_npctimer_max = UMAX(mn,mx);
			send_to_char("Corpse Decay Timer (NPC Corpse) changed.\n\r", ch);
		}

		return true;
	}

	if (!str_prefix(arg, "spill"))
	{
		int spill;
		if (!is_number(argument) || (spill = atoi(argument)) < 0 || spill > 100)
		{
			send_to_char("Syntax:  decay spill <percent>\n\r", ch);
			send_to_char("Please provide a percentage.\n\r", ch);
			return false;
		}

		corpse->decay_spill_chance = spill;
		send_to_char("Corpse Decay Spill Chance changed.\n\r", ch);
		return true;
	}

	corpsedit_decay(ch, "");
	return false;
}

CORPSEDIT( corpsedit_lost )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  lost <parts>\n\r", ch);
		show_flag_cmds(ch, part_flags);
		return false;
	}

	long value;
	if ((value = flag_value(part_flags, argument)) == NO_FLAG)
	{
		send_to_char("Invalid parts flag.\n\r", ch);
		show_flag_cmds(ch, part_flags);
		return false;
	}

	TOGGLE_BIT(corpse->lost_bodyparts, value);
	send_to_char("Lost bodyparts toggled.\n\r", ch);
	return true;
}

CORPSEDIT( corpsedit_damage )
{
	CORPSE_TYPE *corpse;
	EDIT_CORPSE(ch, corpse);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  damage add <damage class>\n\r", ch);
		send_to_char("         damage clear\n\r", ch);
		send_to_char("         damage remove #\n\r", ch);
		send_to_char("         damage # add <corpse type> <weight>\n\r", ch);
		send_to_char("         damage # add <corpse type> <weight>\n\r", ch);
		send_to_char("         damage # clear\n\r", ch);
		send_to_char("         damage # remove #\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	CORPSE_DAMAGE *damage;
	if (!str_prefix(arg, "add"))
	{
		int dc;

		if ((dc = stat_lookup(argument, damage_classes, NO_FLAG)) == NO_FLAG)
		{
			send_to_char("Syntax:  damage add <damage class>\n\r", ch);
			send_to_char("Invalid damage class.  Use '? damageclass' for valid classes.\n\r", ch);
			show_flag_cmds(ch, damage_classes);
			return false;
		}

		damage = get_corpse_damage(corpse, dc);
		if (IS_VALID(damage))
		{
			send_to_char("The corpse already has that damage class in its damage table.\n\r", ch);
			return false;
		}
		
		damage = new_corpse_damage();
		damage->damage_type = dc;

		list_appendlink(corpse->damage_table, damage);
		send_to_char("Damage Class added to Corpse Damage Table.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		list_clear(corpse->damage_table);
		send_to_char("Corpse Damage Table cleared.\n\r", ch);
		return true;
	}

	int index;
	if (!str_prefix(arg, "remove"))
	{
		if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(corpse->damage_table))
		{
			send_to_char(formatf("Please specify a number from 1 to %d.\n\r", list_size(corpse->damage_table)), ch);
			return false;
		}

		list_remnthlink(corpse->damage_table, index, true);
		send_to_char(formatf("Damage Entry #%d removed from Corpse Damage Table.\n\r", index), ch);
		return true;
	}

	if (!is_number(arg) || (index = atoi(arg)) < 1 || index > list_size(corpse->damage_table))
	{
		send_to_char(formatf("Please specify a number from 1 to %d.\n\r", list_size(corpse->damage_table)), ch);
		return false;
	}

	damage = (CORPSE_DAMAGE *)list_nthdata(corpse->damage_table, index);

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "add"))
	{
		if (argument[0] == '\0')
		{
			send_to_char(formatf("Syntax:  damage %d add <corpse type> <weight>\n\r", index), ch);
			send_to_char("Please specify a corpse type.  Use 'corpselist' for list of valid types.\n\r", ch);
			return false;
		}

		argument = one_argument(argument, arg);
		
		CORPSE_TYPE *bc;
		if (!str_prefix(arg, "nocorpse"))
			bc = NULL;
		else
		{
			bc = get_corpse_type(arg);
			if (!IS_VALID(bc))
			{
				send_to_char("No such corpse by that name.\n\r", ch);
				return false;
			}
		}

		int weight;
		if (!is_number(argument) || (weight = atoi(argument)) < 1)
		{
			send_to_char("Please provide a positive number.\n\r", ch);
			return false;
		}

		CORPSE_BLENDING *blend = alloc_mem(sizeof(CORPSE_BLENDING));
		blend->weight = weight;
		blend->result = bc;

		list_appendlink(damage->blending, blend);
		damage->total_weight += weight;
		send_to_char(formatf("Blending added to Damage Entry #%d for Corpse Damage Table.\n\r", index), ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		list_clear(damage->blending);
		damage->total_weight = 0;
		send_to_char(formatf("Blending cleared for Damage Entry #%d of Corpse Damage Table.\n\r", index), ch);
		return true;
	}

	if (!str_prefix(arg, "remove"))
	{
		int blend;
		if (!is_number(argument) || (blend = atoi(argument)) < 1 || blend > list_size(damage->blending))
		{
			send_to_char(formatf("Please specify a number from 1 to %d.\n\r", list_size(damage->blending)), ch);
			return false;
		}

		CORPSE_BLENDING *blending = (CORPSE_BLENDING *)list_nthdata(damage->blending, blend);

		list_remnthlink(damage->blending, blend, true);
		damage->total_weight -= blending->weight;
		send_to_char(formatf("Blending #%d removed from Damage Entry #%d of Corpse Damage Table.\n\r", blend, index), ch);
		return true;
	}

	corpsedit_damage(ch, "");
	return false;
}