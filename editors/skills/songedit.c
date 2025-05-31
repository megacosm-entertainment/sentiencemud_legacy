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


SONGEDIT( songedit_show )
{
	char buf[MSL];
	BUFFER *buffer;
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	buffer = new_buf();

	sprintf(buf, "Song: {W%s {x({W%d{x)\n\r", song->name, song->uid);
	add_buf(buffer, buf);

	olc_buffer_show_string(ch, buffer, formatf("%d", song->level), "level", "Level:", 20, "xDW");
	olc_buffer_show_string(ch, buffer, formatf("%d", song->beats), "beats", "Beats:", 20, "xDW");
	olc_buffer_show_string(ch, buffer, formatf("%d", song->mana), "mana", "Mana:", 20, "xDW");
	olc_buffer_show_string(ch, buffer, formatf("%d", song->rating), "difficulty", "Difficulty:", 20, "xDW");

	olc_buffer_show_flags_ex(ch, buffer, song_flags, song->flags, "flags", "Flags:", 77, 20, 5, "xxYyCcD");
	olc_buffer_show_flags_ex(ch, buffer, song_target_types, song->target, "target", "Target:", 77, 20, 5, "xxYyCcD");

	if (song->token)
	{
		sprintf(buf, "Token:              {W%s {x({W%ld{x#{W%ld{x)\n\r",
			MXPCreateSend(ch->desc, formatf("tshow %ld#%ld", song->token->area->uid, song->token->vnum), song->token->name),
			song->token->area->uid, song->token->vnum);
		add_buf(buffer, buf);

		songedit_show_trigger(ch, buffer, song->token->progs, TRIG_TOKEN_PRESONG,	"presong",	"  {W+ {xPreSong:");
		songedit_show_trigger(ch, buffer, song->token->progs, TRIG_TOKEN_SONG,	"song",		"  {W+ {xSong:");
	}
	else
	{
		olc_buffer_show_string(ch, buffer, presong_func_display(song->presong_fun),	"presong",	"PreSong:", 20, "XDW");
		olc_buffer_show_string(ch, buffer, song_func_display(song->song_fun),		"song",		"Song:", 20, "XDW");
	}

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

SONGEDIT( songedit_list )
{
	char buf[MSL];
	BUFFER *buffer = new_buf();

	ITERATOR it;
	SONG_DATA *song;

	add_buf(buffer, "Songs:\n\r");
	add_buf(buffer, "[Uid] [        Name        ]\n\r");
	add_buf(buffer, "=============================\n\r");

	iterator_start(&it, songs_list);
	while((song = (SONG_DATA *)iterator_nextdata(&it)))
	{
		sprintf(buf, " %s   {%c%s{x\n",
			MXPCreateSend(ch->desc, formatf("songedit %s", song->name), formatf("%3d", song->uid)),
			(song->token ? 'G' : 'Y'),
			MXPCreateSend(ch->desc, formatf("songshow %s", song->name), song->name));
		add_buf(buffer, buf);
	}
	iterator_stop(&it);

	add_buf(buffer, "-----------------------------\n\r");
	sprintf(buf, "Total: %d\n\r", list_size(songs_list));
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


// songedit install source <name>
// songedit install token <widevnum>
SONGEDIT( songedit_install )
{
	char arg[MIL];
	SONG_DATA *song;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  songedit install {Rsource{x <name>\n\r", ch);
		send_to_char("         songedit install {Rtoken{x <widevnum>\n\r", ch);
		return false;
	}

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "source"))
	{
		smash_tilde(argument);
		if (argument[0] == '\0')
		{
			send_to_char("Syntax:  songedit source {R<name>{x\n\r", ch);
			send_to_char("Please specify a name.\n\r", ch);
			return false;
		}

		if (song_exists(argument))
		{
			send_to_char(formatf("The name '{W%s{x' is already in use.\n\r", argument), ch);
			return false;
		}

		song = new_song_data();
		song->uid = ++top_song_uid;
		song->name = str_dup(argument);
		song->token = NULL;

		insert_song(song);

		send_to_char(formatf("Song {W%s{x installed.\n\r", song->name), ch);

		olc_set_editor(ch, ED_SONGEDIT, song);
		return true;
	}

	if (!str_prefix(arg, "token"))
	{
		WNUM wnum;

		if (!parse_widevnum(argument, NULL, &wnum))
		{
			send_to_char("Syntax:  songedit install token {R<widevnum>{x\n\r", ch);
			send_to_char("Please specify a widevnum.\n\r", ch);
			return false;
		}

		TOKEN_INDEX_DATA *token = get_token_index(wnum.pArea, wnum.vnum);
		if (!token)
		{
			send_to_char("No such token by that widevnum.\n\r", ch);
			return false;
		}

		if (token->type != TOKEN_SONG)
		{
			send_to_char("Token must be a SONG token.\n\r", ch);
			return false;
		}

		if (song_exists(token->name))
		{
			send_to_char(formatf("The name '{W%s{x' is already in use.\n\r", token->name), ch);
			return false;
		}

		song = new_song_data();
		song->uid = ++top_song_uid;
		song->name = str_dup(token->name);
		song->token = token;

		insert_song(song);

		send_to_char(formatf("Song {W%s{x installed.\n\r", song->name), ch);

		olc_set_editor(ch, ED_SONGEDIT, song);
		return true;
	}

	songedit_install(ch, "");
	return false;
}

#define SONGEDIT_FUNC(f, p, t, n)		\
SONGEDIT( songedit_##f##func )	\
{ \
	char arg[MIL]; \
	SONG_DATA *song; \
\
	EDIT_SONG(ch, song); \
\
	if (song->token) \
	{ \
		if (argument[0] == '\0') \
		{ \
			send_to_char("Syntax:  songedit " #f " {Rset{x <widevnum>\n\r", ch); \
			send_to_char("         songedit " #f " {Rclear{x\n\r", ch); \
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
			if (!parse_widevnum(argument, song->token->area, &wnum)) \
			{ \
				send_to_char("Syntax:  songedit " #f " set {R<widevnum>{x\n\r", ch); \
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
			__token_remove_trigger(song->token, TRIG_##p); \
\
			/* Add trigger to token. */ \
			if (!__token_add_trigger(song->token, TRIG_##p, "100", script)) \
			{ \
				send_to_char("Something went wrong adding " #p " trigger to token.\n\r", ch); \
				return false; \
			} \
\
			/* Mark area as changed. */ \
			SET_BIT(song->token->area->area_flags, AREA_CHANGED); \
			send_to_char(#p " trigger added to spell token.\n\r", ch); \
			return true; \
		} \
\
		if (!str_prefix(arg, "clear")) \
		{ \
			/* Remove any triggers from token. */ \
			__token_remove_trigger(song->token, TRIG_##p); \
\
			/* Mark area as changed. */ \
			SET_BIT(song->token->area->area_flags, AREA_CHANGED); \
			send_to_char(#p " trigger cleared on spell token.\n\r", ch); \
			return true; \
		} \
\
		songedit_##f##func (ch, ""); \
		return false; \
	} \
	else \
	{ \
		if (argument[0] == '\0') \
		{ \
			send_to_char("Syntax:  songedit " #f " {Rset{x <function>\n\r", ch); \
			send_to_char("         songedit " #f " {Rclear{x\n\r", ch); \
			return false; \
		} \
\
		argument = one_argument(argument, arg); \
\
		if (!str_prefix(arg, "set")) \
		{ \
			if (argument[0] == '\0') \
			{ \
				send_to_char("Syntax:  songedit " #f " set {R<function>{x\n\r", ch); \
				send_to_char("Invalid " #f " function.  Use '? " #f "_func' for a list of functions.\n\r", ch); \
				return false; \
			} \
\
			t *func = f##_func_lookup(argument); \
			if(!func) \
			{ \
				send_to_char("Syntax:  songedit " #f " set {R<function>{x\n\r", ch); \
				send_to_char("Invalid " #f " function.  Use '? " #f "_func' for a list of functions.\n\r", ch); \
				return false; \
			} \
\
			song->f##_fun = func; \
			send_to_char("Song " #f " function set.\n\r", ch); \
			return true; \
		} \
\
		if (!str_prefix(arg, "clear")) \
		{ \
			song->f##_fun = n; \
			send_to_char("Song " #f " function cleared.\n\r", ch); \
			return true; \
		} \
\
		songedit_##f##func (ch, ""); \
		return false; \
	} \
}

SONGEDIT_FUNC(presong,TOKEN_PRESONG,SONG_FUN,NULL)
SONGEDIT_FUNC(song,TOKEN_SONG,SONG_FUN,NULL)

SONGEDIT( songedit_flags )
{
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int value;
	if ((value = flag_value(song_flags, argument)) == NO_FLAG)
	{
		send_to_char("Syntax:  songedit flags {R<flags>{x\n\r", ch);
		send_to_char("Invalid song flags.  Use '? song' to see list of valid flags.\n\r", ch);
		show_flag_cmds(ch, song_flags);
		return false;
	}

	// Check the new flags for any problems
	long new_value = song->flags ^ value;

	if (IS_SET(new_value, SONG_INSTRUMENT_ONLY) && IS_SET(new_value, SONG_VOICE_ONLY))
	{
		send_to_char("{Winstrument_only{x and {Wvoice_only{x are mutually exclusive.\n\r", ch);
		return false;
	}

	song->flags = new_value;
	send_to_char("Song target set.\n\r", ch);
	return true;
}

SONGEDIT( songedit_difficulty )
{
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int difficulty;
	if (!is_number(argument) || (difficulty = atoi(argument)) < 1)
	{
		send_to_char("Syntax:  songedit difficulty {R<difficulty>{x\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	song->rating = difficulty;
	send_to_char("Song difficulty set.\n\r", ch);
	return true;
}

SONGEDIT( songedit_level )
{
	char buf[MSL];
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int level;
	if (!is_number(argument) || (level = atoi(argument)) < 1 || level > MAX_CLASS_LEVEL)
	{
		sprintf(buf, "Syntax:  songedit level {R<1-%d>{x\n\r", MAX_CLASS_LEVEL);
		send_to_char(buf, ch);
		sprintf(buf, "Please specify a number from 1 to %d.\n\r", MAX_CLASS_LEVEL);
		send_to_char(buf, ch);
		return false;
	}

	song->level = level;
	send_to_char("Song level set.\n\r", ch);
	return true;
}

SONGEDIT( songedit_mana )
{
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int mana;
	if (!is_number(argument) || (mana = atoi(argument)) < 0)
	{
		send_to_char("Syntax:  songedit mana {R<mana>{x\n\r", ch);
		send_to_char("Please specify a non-negative number.\n\r", ch);
		return false;
	}

	song->mana = mana;
	send_to_char("Song mana set.\n\r", ch);
	return true;
}

SONGEDIT( songedit_beats )
{
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int beats;
	if (!is_number(argument) || (beats = atoi(argument)) < 1)
	{
		send_to_char("Syntax:  songedit beats {R<beats>{x\n\r", ch);
		send_to_char("Please specify a positive number.\n\r", ch);
		return false;
	}

	song->beats = beats;
	send_to_char("Song beats set.\n\r", ch);
	return true;
}

SONGEDIT( songedit_target )
{
	SONG_DATA *song;

	EDIT_SONG(ch, song);

	int value;
	if ((value = stat_lookup(argument, song_target_types, NO_FLAG)) == NO_FLAG)
	{
		send_to_char("Syntax:  songedit target {R<target>{x\n\r", ch);
		send_to_char("Invalid song target.  Use '? song_targets' to see list of valid target types.\n\r", ch);
		show_flag_cmds(ch, song_target_types);
		return false;
	}

	song->target = value;
	send_to_char("Song target set.\n\r", ch);
	return true;
}