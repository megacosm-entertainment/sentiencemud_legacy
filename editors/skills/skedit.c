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

#include "skedit.h"


SKEDIT( skedit_install )
{
	char buf[MSL];
	SKILL_DATA *skill;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit install {Rsource{x skill|spell <name>\n\r", ch);
		send_to_char("         skedit install {Rtoken{x <widevnum>\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "source"))
	{
		argument = one_argument(argument, arg);
		bool isspell;

		if (!str_prefix(arg, "spell"))
			isspell = true;
		else if (!str_prefix(arg, "skill"))
			isspell = false;
		else
		{
			send_to_char("Syntax:  skedit source {Rskill|spell{x <name>\n\r", ch);
			send_to_char("Please specify whether this is a {Yskill{x or {Gspell{x.\n\r", ch);
			return false;
		}

		// Install a SOURCE skill/spell
		smash_tilde(argument);
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skedit install source skill|spell {R<name>{x\n\r", ch);
			send_to_char("Please specify a name.\n\r", ch);
			return false;
		}

		// Make sure the name doesn't exist already.
		if (skill_exists(argument))
		{
			send_to_char(formatf("The name '{W%s{x' is already in use.\n\r", argument), ch);
			return false;
		}

		skill = new_skill_data();
		skill->uid = ++top_skill_uid;
		skill->name = str_dup(argument);
		skill->isspell = isspell;
		skill->token = NULL;
		
		insert_skill(skill);

		sprintf(buf, "Skill {W%s{x installed.\n\r", skill->name);
		send_to_char(buf, ch);

		olc_set_editor(ch, ED_SKEDIT, skill);
		return true;
	}

	if (!str_prefix(arg, "token"))
	{
		WNUM wnum;

		if (!parse_widevnum(argument, NULL, &wnum))
		{
			send_to_char("Syntax:  skedit install token {R<widevnum>{x\n\r", ch);
			send_to_char("Please provide a valid widevnum.\n\r", ch);
			return false;
		}

		TOKEN_INDEX_DATA *token = get_token_index_wnum(wnum);
		if (!token)
		{
			send_to_char("That token does not exist.\n\r", ch);
			return false;
		}

		bool isspell;
		if (token->type == TOKEN_SPELL)
			isspell = true;
		else if (token->type == TOKEN_SKILL)
			isspell = false;
		else
		{
			send_to_char("That token is neither a {Yskill{x nor {Gspell{x token.\n\r", ch);
			return false;
		}

		if (skill_exists(token->name))
		{
			send_to_char(formatf("The name '{W%s{x' is already in use.\n\r", token->name), ch);
			return false;
		}

		skill = new_skill_data();
		skill->uid = ++top_skill_uid;
		skill->name = str_dup(token->name);
		skill->token = token;
		skill->isspell = isspell;
		
		insert_skill(skill);

		sprintf(buf, "Skill {W%s{x installed.\n\r", skill->name);
		send_to_char(buf, ch);

		olc_set_editor(ch, ED_SKEDIT, skill);
		return true;
	}

	skedit_install(ch, "");
	return false;
}

SKEDIT( skedit_list )
{
	BUFFER *buffer = new_buf();
	char buf[MSL];

	sprintf(buf, "%3s %-20.20s %-20.20s\n\r",
		"###", "Name", "Display");
	add_buf(buffer, buf);
	sprintf(buf, "%3s %-20.20s %-20.20s\n\r",
		"===", "====================", "====================");
	add_buf(buffer, buf);

	int i = 0;
	ITERATOR it;
	SKILL_DATA *skill;
	iterator_start(&it, skills_list);
	while((skill = (SKILL_DATA *)iterator_nextdata(&it)))
	{
		sprintf(buf, "%3d {%c%-20.20s{x %-20.20s\n\r", ++i,
			(skill->token ? 'G' : 'Y'), skill->name, skill->display);
		add_buf(buffer, buf);
	}
	iterator_stop(&it);

	sprintf(buf, "%3s %-20.20s %-20.20s\n\r",
		"---", "--------------------", "--------------------");
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
	return false;
}


SKEDIT( skedit_show )
{
	char buf[MSL];
	BUFFER *buffer;
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	buffer = new_buf();

	// Show Header
	if (is_skill_spell(skill))
		sprintf(buf, "Spell: {W%s {x({W%d{x)\n\r", skill->name, skill->uid);
	else
		sprintf(buf, "Skill: {W%s {x({W%d{x)\n\r", skill->name, skill->uid);
	add_buf(buffer, buf);

	const char **tab_names = skill->token ? skedit_tabs_token : skedit_tabs_source;
	olc_buffer_show_tabs(ch, buffer, tab_names);

	switch(ch->desc->nEditTab)
	{
	case 0:	// General
		olc_buffer_show_string(ch, buffer, skill->display, "display", "Display String:", 20, "xDW");
		olc_buffer_show_string(ch, buffer, gsn_to_name(skill->pgsn), "gsn", "GSN:", 20, "xDW");

		olc_buffer_show_flags_ex(ch, buffer, skill_flags, skill->flags, "flags", "Flags:", 77, 20, 5, "xxYyCcD");

		if (skill->token)
		{
			sprintf(buf, "Token:              {W%s {x({W%ld{x#{W%ld{x)\n\r",
				MXPCreateSend(ch->desc, formatf("tshow %ld#%ld", skill->token->area->uid, skill->token->vnum), skill->token->name),
				skill->token->area->uid, skill->token->vnum);
			add_buf(buffer, buf);
		}

    	if (skill->help_keywords != NULL && lookup_help_exact(skill->help_keywords->string,get_staff_rank(ch),topHelpCat) != NULL)
        	add_buf(buffer, formatf("Help Keywords: '\t<send href=\"help #%d\">{W%s{X\t</send>' ({W#%d{X)\n\r", lookup_help_exact(skill->help_keywords->string, get_staff_rank(ch), topHelpCat)->index, skill->help_keywords->string, lookup_help_exact(skill->help_keywords->string, get_staff_rank(ch), topHelpCat)->index));
    	else if (skill->help_keywords != NULL && lookup_help_exact(skill->help_keywords->string,get_staff_rank(ch),topHelpCat) == NULL)
        	add_buf(buffer, formatf("Help Keywords: {R%s{X\n\r", skill->help_keywords->string));
    	else
        	add_buf(buffer, formatf("Help Keywords: %s\n\r", "(none set)"));
    
    	add_buf(buffer, formatf("Summary:       %s\n\r", skill->summary ? skill->summary : "(none)"));

		if (list_size(skill->levels) > 0)
		{
			add_buf(buffer, "\n\rLevels:\n\r");
			ITERATOR lit;
			SKILL_CLASS_LEVEL *level;
			iterator_start(&lit, skill->levels);
			while((level = (SKILL_CLASS_LEVEL *)iterator_nextdata(&lit)))
			{
				int l = 20 - strlen(level->clazz->name);
				l = UMAX(l, 0);
				sprintf(buf, formatf(" - %%s:%%%ds {W%%d{x\n\r", l), MXPCreateSend(ch->desc, formatf("level %s", level->clazz->name), level->clazz->name), "", level->level);
				add_buf(buffer, buf);
			}
			iterator_stop(&lit);
			add_buf(buffer, "\n\r");
		}
		else
			add_buf(buffer, formatf("%s: %d\n\r", MXPCreateSend(ch->desc, "level default", "Level"), skill->default_level));

		add_buf(buffer, formatf("Difficulty: %d\n\r", skill->difficulty));
		add_buf(buffer, formatf("Primary Stat: %s\n\r", flag_string(stat_types, skill->primary_stat)));

		olc_buffer_show_flags_ex(ch, buffer, spell_target_types, skill->target, "target", "Target:", 77, 20, 5, "xxYyCcD");
		olc_buffer_show_flags_ex(ch, buffer, spell_position_flags, skill->minimum_position, "position", "Minimum Position:", 77, 20, 5, "xxYyCcD");

		add_buf(buffer, "\n\r");

		olc_buffer_show_string(ch, buffer, formatf("%d", skill->beats), "beats", "Beats:", 20, "xDW");
		olc_buffer_show_string(ch, buffer, IS_VALID(skill->race) ? formatf("%s", MXPCreateSend(ch->desc, formatf("raceshow %s", skill->race->name), skill->race->name)) : NULL, "race", "Racial Skill:", 20, "xDW");
		break;
	
	case 1:	// Functions / Triggers
		if (skill->token)
		{
			add_buf(buffer, " {W- {xArtificing:\n\r");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_BREW,		"brew",			"   {W+ {xBrew:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_IMBUE,		"imbue",		"   {W+ {xImbue:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_INK,		"ink",			"   {W+ {xInk:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_PREBREW,	"prebrew",		"   {W+ {xPrebrew:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_PREIMBUE,	"preimbue",		"   {W+ {xPreimbue:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_PREINK,		"preink",		"   {W+ {xPreink:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_PRESCRIBE,	"prescribe",	"   {W+ {xPrescribe:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_SCRIBE,		"scribe",		"   {W+ {xScribe:");

			add_buf(buffer, " {W- {xActions:\n\r");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_BRANDISH,	"brandish",		"   {W+ {xBrandish:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_EQUIP,		"equip",		"   {W+ {xEquip:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_INTERRUPT,	"interrupt",	"   {W+ {xInterrupt:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_PRESPELL,			"prespell",		"   {W+ {xPrespell:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_PULSE,		"pulse",		"   {W+ {xPulse:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_QUAFF,		"quaff",		"   {W+ {xQuaff:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_RECITE,		"recite",		"   {W+ {xRecite:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_SPELL,			"spell",		"   {W+ {xSpell:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_TOUCH,		"touch",		"   {W+ {xTouch:");
			skedit_show_trigger(ch, buffer, skill->token->progs, TRIG_TOKEN_ZAP,		"zap",			"   {W+ {xZap:");
		}
		else
		{
			add_buf(buffer, " {W- {xArtificing:\n\r");
			olc_buffer_show_string(ch, buffer, brew_func_display(skill->brew_fun),			"brew",			"   {W+ {xBrew:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, imbue_func_display(skill->imbue_fun),		"imbue",		"   {W+ {xImbue:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, ink_func_display(skill->ink_fun),			"ink",			"   {W+ {xInk:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, prebrew_func_display(skill->prebrew_fun),	"prebrew",		"   {W+ {xPrebrew:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, preimbue_func_display(skill->preimbue_fun),	"preimbue",		"   {W+ {xPreimbue:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, preink_func_display(skill->preink_fun),		"preink",		"   {W+ {xPreink:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, prescribe_func_display(skill->prescribe_fun),"prescribe",	"   {W+ {xPrescribe:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, scribe_func_display(skill->scribe_fun),		"scribe",		"   {W+ {xScribe:", 20, "XDW");

			add_buf(buffer, "\n\r {w- {xActions:\n\r");
			olc_buffer_show_string(ch, buffer, brandish_func_display(skill->brandish_fun),  "brandish",		"   {W+ {xBrandish:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, equip_func_display(skill->equip_fun),		"equip",		"   {W+ {xEquip:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, interrupt_func_display(skill->interrupt_fun),"interrupt",	"   {W+ {xInterrupt:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, prespell_func_display(skill->prespell_fun),	"prespell",		"   {W+ {xPrespell:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, pulse_func_display(skill->pulse_fun),		"pulse",		"   {W+ {xPulse:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, quaff_func_display(skill->quaff_fun),		"quaff",		"   {W+ {xQuaff:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, recite_func_display(skill->recite_fun),		"recite",		"   {W+ {xRecite:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, spell_func_display(skill->spell_fun),		"spell",		"   {W+ {xSpell:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, touch_func_display(skill->touch_fun),		"touch",		"   {W+ {xTouch:", 20, "XDW");
			olc_buffer_show_string(ch, buffer, zap_func_display(skill->zap_fun),			"zap",			"   {W+ {xZap:", 20, "XDW");
		}
		break;
	case 2: // Magic
		add_buf(buffer, "Inks:\n\r");
		for(int i = 0; i < 3; i++)
		{
			sprintf(buf, " {W%s{x) [{W%2d{x] {Y%s{x\n\r", MXPCreateSend(ch->desc, formatf("inks %d", i + 1), formatf("%d", i + 1)), skill->ink_amounts[i], flag_string(catalyst_types, skill->ink_types[i]));
			add_buf(buffer, buf);
		}

		add_buf(buffer, "\n\r");
		olc_buffer_show_string(ch, buffer, formatf("%d", skill->cast_mana), "mana cast", "Casting Mana:", 20, "xDW");
		olc_buffer_show_string(ch, buffer, formatf("%d", skill->brew_mana), "mana brew", "Brewing Mana:", 20, "xDW");
		olc_buffer_show_string(ch, buffer, formatf("%d", skill->scribe_mana), "mana scribe", "Scribing Mana:", 20, "xDW");
		olc_buffer_show_string(ch, buffer, formatf("%d", skill->imbue_mana), "mana imbue", "Imbuing Mana:", 20, "xDW");
		break;

	case 3:	// Values
		add_buf(buffer, "Values:\n\r");
		for(int i = 0; i < MAX_SKILL_VALUES; i++)
		{
			char name[MIL];
			if (IS_NULLSTR(skill->valuenames[i]))
				sprintf(name, "Value %d:", i+1);
			else
				sprintf(name, "%s:", skill->valuenames[i]);
			olc_buffer_show_string(ch, buffer, formatf("%d", skill->values[i]), formatf("value %d", i + 1),	name, 20, "XDW");
		}
		break;
	
	case 4:	// Messages
		for(int i = 0; msg_handlers[i].verb; i++)
		{
			int j = msg_handlers[i].order;
			char **pptr = (char **)((void *)(msg_handlers[j].field) - (void *)&__static_skill + (void *)skill);

			olc_buffer_show_string(ch, buffer, *pptr, formatf("message %s", msg_handlers[j].verb), formatf("%s:", msg_handlers[j].label), 38, "xDW");	
		}
		break;

	default:
		break;
	}


#if 0

	if (IS_NULLSTR(skill->display))
		sprintf(buf, "Display String:   {D(unset){x\n\r");
	else
		sprintf(buf, "Display String:   {W%s{x\n\r", skill->display);
	add_buf(buffer, buf);

	if (skill->pgsn)
		sprintf(buf, "GSN:              {W%s{x\n\r", gsn_to_name(skill->pgsn));
	else
		sprintf(buf, "GSN:              {D(unset){x\n\r");
	add_buf(buffer, buf);

	sprintf(buf, "Flags:            {x%s\n\r", flag_string(skill_flags, skill->flags));
	add_buf(buffer, buf);


	if (skill->token)
	{
		sprintf(buf, "Token:            {W%s {x({W%ld{x#{W%ld{x)\n\r", skill->token->name, skill->token->area->uid, skill->token->vnum);
		add_buf(buffer, buf);

		add_buf(buffer, " - Artificing:\n\r");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_BREW, "   + Brew:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_IMBUE, "   + Imbue:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_INK, "   + Ink:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_PREBREW, "   + Prebrew:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_PREIMBUE, "   + Preimbue:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_PREINK, "   + Preink:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_PRESCRIBE, "   + Prescribe:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_SCRIBE, "   + Scribe:");

		add_buf(buffer, " - Actions:\n\r");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_BRANDISH, "   + Brandish:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_PRESPELL, "   + Prespell:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_QUAFF, "   + Quaff:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_RECITE, "   + Recite:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_SPELL, "   + Spell:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_TOUCH, "   + Touch:");
		skedit_show_trigger(buffer, skill->token->progs, TRIG_TOKEN_ZAP, "   + Zap:");

	}
	else
	{
		add_buf(buffer, "Functions:\n\r");
		add_buf(buffer, " - Artificing:\n\r");
		skedit_show_function(buffer, brew_func_display(skill->brew_fun), "   + Brew:");
		skedit_show_function(buffer, NULL, "   + Imbue:");
		skedit_show_function(buffer, ink_func_display(skill->ink_fun), "   + Ink:");
		skedit_show_function(buffer, prebrew_func_display(skill->prebrew_fun), "   + Prebrew:");
		skedit_show_function(buffer, NULL, "   + Preimbue:");
		skedit_show_function(buffer, preink_func_display(skill->preink_fun), "   + Preink:");
		skedit_show_function(buffer, prescribe_func_display(skill->prescribe_fun), "   + Prescribe:");
		skedit_show_function(buffer, scribe_func_display(skill->scribe_fun), "   + Scribe:");

		add_buf(buffer, " - Actions:\n\r");
		skedit_show_function(buffer, NULL, "Brandish:");
		skedit_show_function(buffer, prespell_func_display(skill->prespell_fun), "   + Prespell:");
		skedit_show_function(buffer, quaff_func_display(skill->quaff_fun), "   + Quaff:");
		skedit_show_function(buffer, recite_func_display(skill->recite_fun), "   + Recite:");
		skedit_show_function(buffer, spell_func_display(skill->spell_fun), "   + Spell:");
		skedit_show_function(buffer, touch_func_display(skill->touch_fun), "   + Touch:");
		skedit_show_function(buffer, NULL, "   + Zap:");
	}

	add_buf(buffer, "\n\rLevels:\n\r");
	sprintf(buf, " - Cleric:  %-7s{x    - Mage:    %s{x\n\r", skill_level_value(skill,CLASS_CLERIC), skill_level_value(skill,CLASS_MAGE)); add_buf(buffer, buf);
	sprintf(buf, " - Thief:   %-7s{x    - Warrior: %s{x\n\r", skill_level_value(skill,CLASS_THIEF), skill_level_value(skill,CLASS_WARRIOR)); add_buf(buffer, buf);

	add_buf(buffer, "\n\rDifficulties:\n\r");
	sprintf(buf, " - Cleric:  %-12s{x    - Mage:    %s{x\n\r", skill_difficulty_value(skill, CLASS_CLERIC), skill_difficulty_value(skill, CLASS_MAGE)); add_buf(buffer, buf);
	sprintf(buf, " - Thief:   %-12s{x    - Warrior: %s{x\n\r", skill_difficulty_value(skill, CLASS_THIEF), skill_difficulty_value(skill, CLASS_WARRIOR)); add_buf(buffer, buf);

	add_buf(buffer, "\n\rInks:\n\r");
	for(int i = 0; i < 3; i++)
	{
		sprintf(buf, " %d) [%2d] %s\n\r", i + 1, skill->inks[i][1], flag_string(catalyst_types, skill->inks[i][0]));
		add_buf(buffer, buf);
	}

	add_buf(buffer, "\n\rValues:\n\r");
	for(int i = 0; i < MAX_SKILL_VALUES; i++)
	{
		char name[MIL];
		if (IS_NULLSTR(skill->valuenames[i]))
			sprintf(name, "Value %d:", i+1);
		else
			sprintf(name, "%s:", skill->valuenames[i]);
		sprintf(buf, "%-20s %d\n\r", name, skill->values[i]);
		add_buf(buffer, buf);
	}

	sprintf(buf, "\n\rTarget:           %s\n\r", flag_string(spell_target_types, skill->target)); add_buf(buffer, buf);
	sprintf(buf, "Minimum Position: %s\n\r", flag_string(position_flags, skill->minimum_position)); add_buf(buffer, buf);

	if (skill->race > 0)
		sprintf(buf, "Racial Skill:     Yes (%s)\n\r", race_table[skill->race].name);
	else
		sprintf(buf, "Racial Skill:     No\n\r");
	add_buf(buffer, buf);

	sprintf(buf, "Casting Mana:     %d\n\r", skill->cast_mana); add_buf(buffer, buf);
	sprintf(buf, "Brewing Mana:     %d\n\r", skill->brew_mana); add_buf(buffer, buf);
	sprintf(buf, "Scribing Mana:    %d\n\r", skill->scribe_mana); add_buf(buffer, buf);
	sprintf(buf, "Imbuing Mana:     %d\n\r", skill->imbue_mana); add_buf(buffer, buf);
	sprintf(buf, "Beats:            %d\n\r", skill->beats); add_buf(buffer, buf);

	sprintf(buf, "Noun Damage:      %s\n\r", IS_NULLSTR(skill->noun_damage) ? "{D(unset){x" : skill->noun_damage); add_buf(buffer, buf);
	sprintf(buf, "Wear Off Message: %s\n\r", IS_NULLSTR(skill->msg_off) ? "{D(unset){x" : skill->msg_off); add_buf(buffer, buf);
	sprintf(buf, "Object Message:   %s\n\r", IS_NULLSTR(skill->msg_obj) ? "{D(unset){x" : skill->msg_obj); add_buf(buffer, buf);
	sprintf(buf, "Dispel Message:   %s\n\r", IS_NULLSTR(skill->msg_disp) ? "{D(unset){x" : skill->msg_disp); add_buf(buffer, buf);
#endif

	if( !ch->lines && strlen(buffer->string) > MAX_STRING_LENGTH )
	{
		send_to_char("Too much to display.  Please enable scrolling.\n\r", ch);
	}
	else
	{
		send_to_char(buffer->string, ch);
		//page_to_char(buffer->string, ch);
	}

	free_buf(buffer);
	return false;
}

SKEDIT( skedit_flags )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int value;
	if ((value = flag_value(skill_flags, argument)) == NO_FLAG)
	{
		send_to_char("Syntax:  skedit flags <flags>\n\r", ch);
		send_to_char("Invalid skill flags.  Use '? skill' for list of skill flags.\n\r", ch);
		return false;
	}

	TOGGLE_BIT(skill->flags, value);

	send_to_char("Skill flags toggled.\n\r", ch);
	return true;
}


SKEDIT( skedit_name )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit name <name>\n\r", ch);
		send_to_char("Please specify a name.\n\r", ch);
		return false;
	}

	free_string(skill->name);
	skill->name = str_dup(argument);

	send_to_char("Skill name set.\n\r", ch);
	return true;
}

SKEDIT( skedit_display )
{
	char arg[MIL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit display {Rset{x <display string>\n\r", ch);
		send_to_char("Syntax:  skedit display {Rclear{x\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "set"))
	{
		smash_tilde(argument);
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skedit display set {R<display string>{x\n\r", ch);
			send_to_char("Please specify a display string.\n\r", ch);
			return false;
		}

		free_string(skill->display);
		skill->display = str_dup(argument);

		send_to_char("Skill display string set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		free_string(skill->display);
		skill->display = NULL;
		send_to_char("Skill display string cleared.\n\r", ch);
		return true;
	}

	skedit_display(ch, "");
	return false;
}

SKEDIT( skedit_primary )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int stat;
	if ((stat = stat_lookup(argument, stat_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Invalid stat.  Use '? stats' for list of valid stat types.\n\r", ch);
		show_flag_cmds(ch, stat_types);
		return false;
	}

	skill->primary_stat = stat;
	send_to_char("Skill Primary Stat set.\n\r", ch);
	return true;
}

#define SKEDIT_FUNC(f, p, t, n)		\
SKEDIT( skedit_##f##func )	\
{ \
	char arg[MIL]; \
	SKILL_DATA *skill; \
\
	EDIT_SKILL(ch, skill); \
\
	if (skill->token) \
	{ \
		if (argument[0] == '\0') \
		{ \
			send_to_char("Syntax:  skedit " #f " {Rset{x <widevnum>\n\r", ch); \
			send_to_char("         skedit " #f " {Rclear{x\n\r", ch); \
			return false; \
		} \
\
		argument = one_argument(argument, arg); \
\
		if (!str_prefix(arg, "set")) \
		{ \
			/* TOKEN mode */ \
			WNUM wnum; \
\
			/* Allow wnum shortcutting using the token's area */ \
			if (!parse_widevnum(argument, skill->token->area, &wnum)) \
			{ \
				send_to_char("Syntax:  skedit " #f " set {R<widevnum>{x\n\r", ch); \
				send_to_char("Please specify a widevnum for the token " #p " trigger.\n\r", ch); \
				return false; \
			} \
\
			/* Get the script */ \
			SCRIPT_DATA *script = get_script_index(wnum.pArea, wnum.vnum, PRG_TPROG); \
			if (!script) \
			{ \
				send_to_char("No such token script by that widevnum.\n\r", ch); \
				return false; \
			} \
\
			/* Remove any triggers from token. */ \
			__token_remove_trigger(skill->token, TRIG_##p); \
\
			/* Add trigger to token. */ \
			if (!__token_add_trigger(skill->token, TRIG_##p, "100", script)) \
			{ \
				send_to_char("Something went wrong adding " #p " trigger to token.\n\r", ch); \
				return false; \
			} \
\
			/* Mark area as changed. */ \
			SET_BIT(skill->token->area->area_flags, AREA_CHANGED); \
			send_to_char(#p " trigger added to spell token.\n\r", ch); \
			return true; \
		} \
\
		if (!str_prefix(arg, "clear")) \
		{ \
			/* Remove any triggers from token. */ \
			__token_remove_trigger(skill->token, TRIG_##p); \
\
			/* Mark area as changed. */ \
			SET_BIT(skill->token->area->area_flags, AREA_CHANGED); \
			send_to_char(#p " trigger cleared on spell token.\n\r", ch); \
			return true; \
		} \
\
		skedit_##f##func (ch, ""); \
		return false; \
	} \
	else \
	{ \
		if (argument[0] == '\0') \
		{ \
			send_to_char("Syntax:  skedit " #f " {Rset{x <function>\n\r", ch); \
			send_to_char("         skedit " #f " {Rclear{x\n\r", ch); \
			return false; \
		} \
\
		argument = one_argument(argument, arg); \
\
		if (!str_prefix(arg, "set")) \
		{ \
			if (argument[0] == '\0') \
			{ \
				send_to_char("Syntax:  skedit " #f " set {R<function>{x\n\r", ch); \
				send_to_char("Invalid " #f " function.  Use '? " #f "_func' for a list of functions.\n\r", ch); \
				return false; \
			} \
\
			t *func = f##_func_lookup(argument); \
			if(!func) \
			{ \
				send_to_char("Syntax:  skedit " #f " set {R<function>{x\n\r", ch); \
				send_to_char("Invalid " #f " function.  Use '? " #f "_func' for a list of functions.\n\r", ch); \
				return false; \
			} \
\
			skill->f##_fun = func; \
			send_to_char("Skill " #f " function set.\n\r", ch); \
			return true; \
		} \
\
		if (!str_prefix(arg, "clear")) \
		{ \
			skill->f##_fun = n; \
			send_to_char("Skill " #f " function cleared.\n\r", ch); \
			return true; \
		} \
\
		skedit_##f##func (ch, ""); \
		return false; \
	} \
}


SKEDIT_FUNC(prespell,PRESPELL,SPELL_FUN,NULL)
SKEDIT_FUNC(spell,SPELL,SPELL_FUN,NULL)
SKEDIT_FUNC(pulse,TOKEN_PULSE,SPELL_FUN,NULL)
SKEDIT_FUNC(interrupt,TOKEN_INTERRUPT,SPELL_FUN,NULL)

SKEDIT_FUNC(prebrew,TOKEN_PREBREW,PREBREW_FUN,NULL)
SKEDIT_FUNC(brew,TOKEN_BREW,BREW_FUN,NULL)
SKEDIT_FUNC(quaff,TOKEN_QUAFF,QUAFF_FUN,NULL)

SKEDIT_FUNC(prescribe,TOKEN_PRESCRIBE,PRESCRIBE_FUN,NULL)
SKEDIT_FUNC(scribe,TOKEN_SCRIBE,SCRIBE_FUN,NULL)
SKEDIT_FUNC(recite,TOKEN_RECITE,RECITE_FUN,NULL)

SKEDIT_FUNC(preink,TOKEN_PREINK,PREINK_FUN,NULL)
SKEDIT_FUNC(ink,TOKEN_INK,INK_FUN,NULL)
SKEDIT_FUNC(touch,TOKEN_TOUCH,TOUCH_FUN,NULL)

SKEDIT_FUNC(preimbue,TOKEN_PREIMBUE,PREIMBUE_FUN,NULL)
SKEDIT_FUNC(imbue,TOKEN_IMBUE,IMBUE_FUN,NULL)
SKEDIT_FUNC(brandish,TOKEN_BRANDISH,BRANDISH_FUN,NULL)
SKEDIT_FUNC(zap,TOKEN_ZAP,ZAP_FUN,NULL)
SKEDIT_FUNC(equip,TOKEN_EQUIP,EQUIP_FUN,NULL)

#if 0
// Morph based upon whether this is a source or scripted ability
SKEDIT( skedit_prespellfunc )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (skill->token)
	{
		// TOKEN mode
		WNUM wnum;

		// Allow wnum shortcutting using the token's area
		if (!parse_widevnum(argument, skill->token->area, &wnum))
		{
			send_to_char("Syntax:  skedit prespell {R<widevnum>{x\n\r", ch);
			send_to_char("Please specify a widevnum for the token SPELL trigger.\n\r", ch);
			return false;
		}

		// Get the script
		SCRIPT_DATA *script = get_script_index(wnum.pArea, wnum.vnum, PRG_TPROG);
		if (!script)
		{
			send_to_char("No such token script by that widevnum.\n\r", ch);
			return false;
		}

		// Remove any TRIG_PRESPELL triggers from token.
		__token_remove_trigger(skill->token, TRIG_PRESPELL);

		// Add trigger to token.
		if (!__token_add_trigger(skill->token, TRIG_PRESPELL, "100", script))
		{
			send_to_char("Something went wrong adding PRESPELL trigger to token.\n\r", ch);
			return false;
		}

		// Mark area as changed.
		SET_BIT(skill->token->area->area_flags, AREA_CHANGED);
		send_to_char("PRESPELL trigger added to spell token.\n\r", ch);
		return true;
	}
	else
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skedit prespell {R<function>{x\n\r", ch);
			send_to_char("Invalid prespell function.  Use '? prespell_func' for a list of functions.\n\r", ch);
			return false;
		}

		SPELL_FUN *func = prespell_func_lookup(argument);
		if(!func)
		{
			send_to_char("Syntax:  skedit prespell {R<function>{x\n\r", ch);
			send_to_char("Invalid prespell function.  Use '? prespell_func' for a list of functions.\n\r", ch);
			return false;
		}

		skill->prespell_fun = func;
		send_to_char("Skill prespell function set.\n\r", ch);
		return true;
	}
}

// Morph based upon whether this is a source or scripted ability
SKEDIT( skedit_spellfunc )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (skill->token)
	{
		// TOKEN mode
		WNUM wnum;

		// Allow wnum shortcutting using the token's area
		if (!parse_widevnum(argument, skill->token->area, &wnum))
		{
			send_to_char("Syntax:  skedit spell {R<widevnum>{x\n\r", ch);
			send_to_char("Please specify a widevnum for the token SPELL trigger.\n\r", ch);
			return false;
		}

		// Get the script
		SCRIPT_DATA *script = get_script_index(wnum.pArea, wnum.vnum, PRG_TPROG);
		if (!script)
		{
			send_to_char("No such token script by that widevnum.\n\r", ch);
			return false;
		}

		// Remove any TRIG_SPELL triggers from token.
		__token_remove_trigger(skill->token, TRIG_SPELL);

		// Add trigger to token.
		if (!__token_add_trigger(skill->token, TRIG_SPELL, "100", script))
		{
			send_to_char("Something went wrong adding SPELL trigger to token.\n\r", ch);
			return false;
		}

		// Mark area as changed.
		SET_BIT(skill->token->area->area_flags, AREA_CHANGED);
		send_to_char("SPELL trigger added to spell token.\n\r", ch);
		return true;
	}
	else
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skedit spell {R<function>{x\n\r", ch);
			send_to_char("Please specify a spell function name.  Use '? spell_func' for a list of functions.\n\r", ch);
			return false;
		}

		SPELL_FUN *func = spell_func_lookup(argument);
		if(!func)
		{
			send_to_char("Syntax:  skedit spell {R<function>{x\n\r", ch);
			send_to_char("Invalid spell function.  Use '? spell_func' for a list of functions.\n\r", ch);
			return false;
		}

		skill->spell_fun = func;
		send_to_char("Skill spell function set.\n\r", ch);
		return true;
	}
}
#endif


SKEDIT( skedit_gsn )
{
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit gsn {Rset{x <gsn>\n\r", ch);
		send_to_char("Syntax:  skedit gsn {Rclear{x\n\r", ch);
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "set"))
	{
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  skedit gsn set {R<gsn>{x\n\r", ch);
			send_to_char("Please specify a GSN.  Use '? gsn' to see list of available GSNs.\n\r", ch);
			return false;
		}

		int16_t *pgsn = gsn_from_name(argument);
		if (!pgsn)
		{
			send_to_char("Syntax:  skedit gsn set {R<gsn>{x\n\r", ch);
			send_to_char("Invalid GSN.  Use '? gsn' to see list of available GSNs.\n\r", ch);
			return false;
		}

		ITERATOR it;
		SKILL_DATA *sk;
		iterator_start(&it, skills_list);
		while((sk = (SKILL_DATA *)iterator_nextdata(&it)))
		{
			if (sk->pgsn == pgsn && sk != skill)
				break;
		}
		iterator_stop(&it);

		if (sk)
		{
			sprintf(buf, "That GSN is already used by skill {W%s{x.\n\r", sk->name);
			send_to_char(buf, ch);
			return false;
		}

		skill->pgsn = pgsn;
		*pgsn = skill->uid;
		for(int i = 0; gsn_table[i].name; i++)
		{
			if (gsn_table[i].gsn == pgsn && gsn_table[i].gsk)
			{
				*gsn_table[i].gsk = skill;
				break;
			}
		}

		send_to_char("Skill GSN set.\n\r", ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		if (!skill->pgsn)
		{
			send_to_char("There is no GSN set on this skill.\n\r", ch);
			return false;
		}

		for(int i = 0; gsn_table[i].name; i++)
		{
			if (gsn_table[i].gsn == skill->pgsn)
			{
				*gsn_table[i].gsn = -1;
				if(gsn_table[i].gsk)
					*gsn_table[i].gsk = skill;
				break;
			}
		}


		skill->pgsn = NULL;
		send_to_char("Skill GSN clear.\n\r", ch);
		return true;
	}

	skedit_gsn(ch, "");
	return false;
}

SKEDIT( skedit_level )
{
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (argument[0] == '\0')
	{
		if (list_size(skill->levels) > 0)
		{
			send_to_char("Syntax:  skedit level {R<class>{x <level>|none\n\r", ch);
			send_to_char("Please select a class.  Use '? classes' to get a list of classes.\n\r", ch);
		}
		else
		{
			send_to_char("Syntax:  skedit level {R<class>{x <level>|none\n\r", ch);
			send_to_char("         skedit level {Rdefault{x <level>\n\r", ch);
			send_to_char("Please select a class or {Wdefault{x.  Use '? classes' to get a list of classes.\n\r", ch);
		}
		return false;
	}

	char arg[MIL];
	argument = one_argument(argument, arg);

	if (list_size(skill->levels) < 1)
	{
		if(!str_prefix(arg, "default"))
		{
			int default_level;
			if (!is_number(argument) || (default_level = atoi(argument)) < 1)
			{
				send_to_char("Please specify a positive number.\n\r", ch);
				return false;
			}

			skill->default_level = default_level;
			send_to_char("Default level changed.\n\r", ch);
			return true;
		}
	}
	
	CLASS_DATA *clazz = get_class_data(arg);
	if (!IS_VALID(clazz))
	{
		send_to_char("Syntax:  skedit level {R<class>{x <level>|none\n\r", ch);
		send_to_char("Invalid class.  Use '? classes' to get a list of classes.\n\r", ch);
		return false;
	}

	if (argument[0] == '\0')
	{
		sprintf(buf, "Syntax:  skedit level <class> {R1-%d|none{x\n\r", clazz->max_level);
		send_to_char(buf, ch);
		sprintf(buf, "Please specify a number from 1 to %d or {Wnone{x.\n\r", clazz->max_level);
		send_to_char(buf, ch);
		return false;
	}

	int value;
	if (is_number(argument))
	{
		value = atoi(argument);
		if (value < 1 || value > clazz->max_level)
		{
			sprintf(buf, "Syntax:  skedit level <class> {R1-%d{x\n\r", clazz->max_level);
			send_to_char(buf, ch);
			sprintf(buf, "Please specify a number from 1 to %d.\n\r", clazz->max_level);
			send_to_char(buf, ch);
			return false;
		}
	}
	else if (!str_prefix(argument, "none"))
	{
		value = -1;
	}
	else
	{
		send_to_char("Syntax:  skedit level <class> {Rnone{x\n\r", ch);
		sprintf(buf, "Please specify a number from 1 to %d or {Wnone{x.\n\r", clazz->max_level);
		send_to_char(buf, ch);
		return false;
	}

	SKILL_CLASS_LEVEL *level = get_skill_class_level(skill, clazz);
	if (level)
	{
		if (value > 0)
			level->level = value;
		else
			list_remlink(skill->levels, level, true);
	}
	else if(value > 0)
	{
		level = new_skill_class_level();
		level->clazz = clazz;
		level->level = value;
		insert_skill_class_level(skill, level);
	}
	else
	{
		send_to_char("Skill does not have class entry.\n\r", ch);
		return false;
	}

	sprintf(buf, "Skill level set for {+%s.\n\r", clazz->name);
	send_to_char(buf, ch);
	return true;
}

SKEDIT( skedit_difficulty )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int difficulty;
	if (!is_number(argument) || (difficulty = atoi(argument)) < 1)
	{
		send_to_char("Syntax:  skedit difficulty 1+\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	skill->difficulty = difficulty;
	send_to_char("Skill difficulty set.\n\r", ch);
	return true;
}

SKEDIT( skedit_target )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int value;
	if ((value = stat_lookup(argument, spell_target_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  skedit target {R<target>{x\n\r", ch);
		send_to_char("Invalid spell target.  Use '? spell_targets' to see list of targets.\n\r", ch);
		return false;
	}

	skill->target = value;
	send_to_char("Skill spell target set.\n\r", ch);
	return true;
}

SKEDIT( skedit_position )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int value;
	if ((value = stat_lookup(argument, position_flags, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  skedit position {R<position>{x\n\r", ch);
		send_to_char("Invalid position.  Use '? position' to see list of positions.\n\r", ch);
		return false;
	}

	skill->minimum_position = value;
	send_to_char("Skill minimum position set.\n\r", ch);
	return true;
}

SKEDIT( skedit_race )
{
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit race {R<race|none>{x\n\r", ch);
		send_to_char("Please specify a race.  Use '? races' for valid races.\n\r", ch);
		return false;
	}

	if (!str_prefix(argument, "none"))
	{
		skill->race = NULL;
		send_to_char("Skill race cleared.\n\r", ch);
	}
	else
	{
		RACE_DATA *race = get_race_data(argument);
		if (!IS_VALID(race))
		{
			send_to_char("Syntax:  skedit race {R<race>{x\n\r", ch);
			send_to_char("Invalid race.  Use '? races' for valid races.\n\r", ch);
			return false;
		}

		if (argument[0] == '?')
		{
			send_to_char("Available races are:", ch);

			int i = 0;
			ITERATOR rit;
			iterator_start(&rit, race_list);
			while((race = (RACE_DATA *)iterator_nextdata(&rit)))
			{
				if ((i++ % 3) == 0)
					send_to_char("\n\r", ch);
				sprintf(buf, " %-15s", race->name);
				send_to_char(buf, ch);
			}
			iterator_stop(&rit);

			send_to_char("\n\r", ch);
			return false;
		}

		skill->race = race;
		send_to_char("Skill race set.\n\r", ch);
	}

	return true;
}

SKEDIT( skedit_mana )
{
	char buf[MSL];
	char arg[MIL];
	SKILL_DATA *skill;
	int16_t *mana;

	EDIT_SKILL(ch, skill);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  skedit mana {Rcast{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rbrew{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rscribe{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rimbue{x <mana>\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "cast"))
	{
		strcpy(arg, "cast");
		mana = &skill->cast_mana;
	}
	else if (!str_prefix(arg, "brew"))
	{
		strcpy(arg, "brew");
		mana = &skill->brew_mana;
	}
	else if (!str_prefix(arg, "scribe"))
	{
		strcpy(arg, "scribe");
		mana = &skill->scribe_mana;
	}
	else if (!str_prefix(arg, "imbue"))
	{
		strcpy(arg, "imbue");
		mana = &skill->imbue_mana;
	}
	else
	{
		send_to_char("Syntax:  skedit mana {Rcast{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rbrew{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rscribe{x <mana>\n\r", ch);
		send_to_char("Syntax:  skedit mana {Rimbue{x <mana>\n\r", ch);
		return false;
	}

	int value;
	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		sprintf(buf, "Syntax:  skedit mana %s {R<mana>{x\n\r", arg);
		send_to_char(buf, ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	*mana = value;
	sprintf(buf, "Skill {+%s Mana Cost set.\n\r", arg);
	send_to_char(buf, ch);
	return true;
}

SKEDIT( skedit_beats )
{
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int value;
	if (!is_number(argument) || (value = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  skedit beats {R<beats>{x\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	skill->beats = value;
	send_to_char("Skill Beats set.\n\r", ch);
	return true;

	return false;
}
SKEDIT( skedit_message )
{
	char arg[MIL];
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	argument = one_argument(argument, arg);
	if (arg[0] == '\0')
	{
		for(int i = 0; msg_handlers[i].verb; i++)
		{
			if (i > 0)
				sprintf(buf, "         skedit message {R%s{x set <message>\n\r", msg_handlers[i].verb);
			else
				sprintf(buf, "Syntax:  skedit message {R%s{x set <message>\n\r", msg_handlers[i].verb);
			send_to_char(buf, ch);
			sprintf(buf, "         skedit message {R%s{x clear\n\r", msg_handlers[i].verb);
			send_to_char(buf, ch);
		}
		return false;
	}

	for(int i = 0; msg_handlers[i].verb; i++)
	{
		if (!str_prefix(arg, msg_handlers[i].verb))
		{
			argument = one_argument(argument, arg);
			if (arg[0] == '\0')
			{
				sprintf(buf, "Syntax:  skedit message %s {Rset{x <message>\n\r", msg_handlers[i].verb);
				send_to_char(buf, ch);
				sprintf(buf, "         skedit message %s {Rclear{x\n\r", msg_handlers[i].verb);
				send_to_char(buf, ch);
				return false;
			}

			if (!str_prefix(arg, "set"))
			{
				smash_tilde(argument);
				if (argument[0] == '\0')
				{
					sprintf(buf, "Syntax:  skedit message %s set {R<message>{x\n\r", msg_handlers[i].verb);
					send_to_char(buf, ch);
					sprintf(buf, "Please specify a %s string.\n\r", msg_handlers[i].name);
					send_to_char(buf, ch);
					return false;
				}

				// Evil pointer math
				// msg_handlers[i].field - &__static_skill => gets offset
				// <offset> + skill => &skill->FIELD
				char **ptr = (char **)((void *)(msg_handlers[i].field) - (void *)&__static_skill + (void *)skill);
				free_string(*ptr);
				*ptr = str_dup(argument);

				sprintf(buf, "Skill %s set.\n\r", msg_handlers[i].name_caps);
				send_to_char(buf, ch);
				return true;
			}

			if (!str_prefix(arg, "clear"))
			{
				// Evil pointer math
				// msg_handlers[i].field - &__static_skill => gets offset
				// <offset> + skill => &skill->FIELD
				char **ptr = (char **)((void *)(msg_handlers[i].field) - (void *)&__static_skill + (void *)skill);
				free_string(*ptr);
				*ptr = NULL;

				sprintf(buf, "Skill %s cleared.\n\r", msg_handlers[i].name_caps);
				send_to_char(buf, ch);
				return true;
			}

			skedit_message(ch, msg_handlers[i].verb);
			return false;
		}
	}

#if 0
	if (!str_prefix(arg, "noun"))
	{
		argument = one_argument(argument, arg);
		if (arg[0] == '\0')
		{
			send_to_char("Syntax:  skedit message noun {Rset{x <message>\n\r", ch);
			send_to_char("         skedit message noun {Rclear{x\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "set"))
		{
			smash_tilde(argument);
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  skedit message noun set {R<message>{x\n\r", ch);
				send_to_char("Please specify a noun damage string.\n\r", ch);
				return false;
			}

			free_string(skill->noun_damage);
			skill->noun_damage = str_dup(argument);
			send_to_char("Skill Noun Damage set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(skill->noun_damage);
			skill->noun_damage = NULL;
			send_to_char("Skill Noun Damage cleared.\n\r", ch);
			return true;
		}

		skedit_message(ch, "noun");
		return false;
	}

	if (!str_prefix(arg, "wearoff"))
	{
		argument = one_argument(argument, arg);
		if (arg[0] == '\0')
		{
			send_to_char("Syntax:  skedit message wearoff {Rset{x <message>\n\r", ch);
			send_to_char("         skedit message wearoff {Rclear{x\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "set"))
		{
			smash_tilde(argument);
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  skedit message wearoff set {R<message>{x\n\r", ch);
				send_to_char("Please specify a wearoff message string.\n\r", ch);
				return false;
			}

			free_string(skill->msg_off);
			skill->msg_off = str_dup(argument);
			send_to_char("Skill Wear Off Message set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(skill->msg_off);
			skill->msg_off = NULL;
			send_to_char("Skill Wear Off Message cleared.\n\r", ch);
			return true;
		}

		skedit_message(ch, "wearoff");
		return false;
	}

	if (!str_prefix(arg, "object"))
	{
		argument = one_argument(argument, arg);
		if (arg[0] == '\0')
		{
			send_to_char("Syntax:  skedit message object {Rset{x <message>\n\r", ch);
			send_to_char("         skedit message object {Rclear{x\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "set"))
		{
			smash_tilde(argument);
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  skedit message object set {R<message>{x\n\r", ch);
				send_to_char("Please specify a object damage string.\n\r", ch);
				return false;
			}

			free_string(skill->msg_obj);
			skill->msg_obj = str_dup(argument);
			send_to_char("Skill Wear Off (Object) Message set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(skill->msg_obj);
			skill->msg_obj = NULL;
			send_to_char("Skill Wear Off (Object) Message cleared.\n\r", ch);
			return true;
		}

		skedit_message(ch, "object");
		return false;
	}

	if (!str_prefix(arg, "dispel"))
	{
		argument = one_argument(argument, arg);
		if (arg[0] == '\0')
		{
			send_to_char("Syntax:  skedit message dispel {Rset{x <message>\n\r", ch);
			send_to_char("         skedit message dispel {Rclear{x\n\r", ch);
			return false;
		}

		if (!str_prefix(arg, "set"))
		{
			smash_tilde(argument);
			if (argument[0] == '\0')
			{
				send_to_char("Syntax:  skedit message dispel set {R<message>{x\n\r", ch);
				send_to_char("Please specify a dispel damage string.\n\r", ch);
				return false;
			}

			free_string(skill->msg_disp);
			skill->msg_disp = str_dup(argument);
			send_to_char("Skill Dispel Message set.\n\r", ch);
			return true;
		}

		if (!str_prefix(arg, "clear"))
		{
			free_string(skill->msg_disp);
			skill->msg_disp = NULL;
			send_to_char("Skill Dispel Message cleared.\n\r", ch);
			return true;
		}

		skedit_message(ch, "dispel");
		return false;
	}
#endif

	skedit_message(ch, "");
	return false;
}

SKEDIT( skedit_inks )
{
	char buf[MSL];
	char arg[MIL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	int index;
	argument = one_argument(argument, arg);
	if (!is_number(arg) || (index = atoi(arg)) < 1 || index > SKILL_MAX_INKS)
	{
		send_to_char(formatf("Syntax: skedit inks {R<1-%d>{x none\n\r", SKILL_MAX_INKS), ch);
		send_to_char(formatf("        skedit inks {R<1-%d>{x <catalyst>[ <amount+>]\n\r", SKILL_MAX_INKS), ch);
		send_to_char(formatf("Please specify a number from 1 to %d.\n\r", SKILL_MAX_INKS), ch);
		return false;
	}

	int catalyst;
	argument = one_argument(argument, arg);
	if ((catalyst = stat_lookup(arg, catalyst_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char(formatf("Syntax: skedit inks <1-%d> {Rnone{x\n\r", SKILL_MAX_INKS), ch);
		send_to_char(formatf("        skedit inks <1-%d> {R<catalyst>{x[ <amount+>]\n\r", SKILL_MAX_INKS), ch);
		send_to_char("Invalid catalyst type.  Use '? catalyst' for list of catalyst types.\n\r", ch);
		return false;
	}

	int amount = 0;
	if (catalyst != CATALYST_NONE)
	{
		if (!is_number(argument) || (amount = atoi(argument)) < 1)
		{
			send_to_char("Syntax:  skedit inks <1-3> <catalyst> {R<amount+>{x\n\r", ch);
			send_to_char("Please specify a positive number.\n\r", ch);
			return false;
		}
	}

	skill->ink_types[index - 1] = catalyst;
	skill->ink_amounts[index - 1] = amount;
	sprintf(buf, "Skill Ink %d set.\n\r", index);
	send_to_char(buf, ch);
	return true;
}

SKEDIT( skedit_value )
{
	char arg[MIL];
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	argument = one_argument(argument, arg);
	int index;
	if (!is_number(arg) || (index = atoi(arg)) < 1 || index > MAX_SKILL_VALUES)
	{
		sprintf(buf, "Syntax:  skedit value {R1-%d{x <value>\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		sprintf(buf, "Please select an index from 1 to %d.\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		return false;
	}

	if (!is_number(argument))
	{
		sprintf(buf, "Syntax:  skedit value 1-%d {R<value>{x\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		send_to_char("Please provide a number.\n\r", ch);
		return false;
	}

	skill->values[index - 1] = atoi(argument);
	if (IS_NULLSTR(skill->valuenames[index - 1]))
		sprintf(buf, "Skill Value %d set.\n\r", index);
	else
		sprintf(buf, "Skill Value %d (%s) set.\n\r", index, skill->valuenames[index - 1]);
	send_to_char(buf, ch);
	return true;
}

SKEDIT( skedit_valuename )
{
	char arg[MIL];
	char buf[MSL];
	SKILL_DATA *skill;

	EDIT_SKILL(ch, skill);

	argument = one_argument(argument, arg);
	int index;
	if (!is_number(arg) || (index = atoi(arg)) < 1 || index > MAX_SKILL_VALUES)
	{
		sprintf(buf, "Syntax:  skedit valuename {R1-%d{x set <name>\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		sprintf(buf, "         skedit valuename {R1-%d{x clear\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		sprintf(buf, "Please select an index from 1 to %d.\n\r", MAX_SKILL_VALUES);
		send_to_char(buf, ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if(!str_prefix(arg, "set"))
	{
		smash_tilde(argument);
		if (argument[0] == '\0')
		{
			sprintf(buf, "Syntax:  skedit valuename 1-%d set {R<name>{x\n\r", MAX_SKILL_VALUES);
			send_to_char(buf, ch);
			send_to_char("Please provide a name.\n\r", ch);
			return false;
		}

		free_string(skill->valuenames[index - 1]);
		skill->valuenames[index - 1] = str_dup(argument);
		sprintf(buf, "Skill Value Name %d set.\n\r", index);
		send_to_char(buf, ch);
		return true;
	}

	if (!str_prefix(arg, "clear"))
	{
		free_string(skill->valuenames[index - 1]);
		skill->valuenames[index - 1] = NULL;
		sprintf(buf, "Skill Value Name %d cleared.\n\r", index);
		send_to_char(buf, ch);
		return true;
	}

	sprintf(buf, "Syntax:  skedit valuename 1-%d {Rset{x <name>\n\r", MAX_SKILL_VALUES);
	send_to_char(buf, ch);
	sprintf(buf, "         skedit valuename 1-%d {Rclear{x\n\r", MAX_SKILL_VALUES);
	send_to_char(buf, ch);
	return false;
}

SKEDIT ( skedit_sethelp )
{

    SKILL_DATA *skill;
    EDIT_SKILL( ch, skill );
    STRING_DATA *help;
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *pHelp;

    if (argument[0] == '\0')
    {
        send_to_char("Syntax: sethelp [keywords]\n\r",ch);
        return false;
    }

    if (!str_cmp(argument, "clear"))
    {
        free_string_data(skill->help_keywords);
        skill->help_keywords = NULL;
        send_to_char("Help keywords cleared.\n\r", ch);
        return true;
    }

    if (argument[0] == '#')
    {
        argument++;
        int index;
		if ((index = atoi(argument)) < 0 || index > 32000)
        {
			send_to_char("That help index is out of range.\n\r", ch);
			return false;
		} else 
            pHelp = lookup_help_index(index, get_staff_rank(ch), topHelpCat);
        
        if (pHelp == NULL)
        {
            act("There is no helpfile with index $t.", ch, NULL, NULL, NULL, NULL, argument, NULL, TO_CHAR, NULL, NULL);
            return false;            
        }
        
    }
    else
    {
        pHelp = lookup_help_exact(argument, get_staff_rank(ch), topHelpCat);
        if (pHelp == NULL)
        {
	        act("There is no helpfile with keywords $t.", ch, NULL, NULL, NULL, NULL, argument, NULL, TO_CHAR, NULL, NULL);
	        return false;
        }
    }

    int i = 0;
    while (argument[i] != '\0')
    {
	argument[i] = UPPER(argument[i]);
	i++;
    }

    help = new_string_data();
    help->string = str_dup(pHelp->keyword);
    skill->help_keywords = help;
    sprintf(buf, "Help keywords set to %s.\n\r", pHelp->keyword);
    send_to_char(buf, ch);
    return true;
}

SKEDIT ( skedit_summary )
{
    SKILL_DATA *skill;

    EDIT_SKILL(ch, skill);

    if (argument[0] == '\0')
    {
	send_to_char("Syntax:  summary [string]\n\r", ch);
	return false;
    }

    free_string(skill->summary);

    skill->summary = str_dup(argument);
    skill->summary[0] = UPPER(skill->summary[0] );

    send_to_char("Skill summary set.\n\r", ch);
    return true;
}



