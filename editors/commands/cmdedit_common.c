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

#include "cmdedit.h"

void show_flag_cmds(CHAR_DATA *ch, const struct flag_type *flag_table);


CMD_DATA *get_cmd_data(char *name)
{
	ITERATOR it;
	CMD_DATA *command;
	iterator_start(&it, commands_list);
	while((command = (CMD_DATA *)iterator_nextdata(&it)))
	{
		if (!str_prefix(name, command->name))
			break;
	}
	iterator_stop(&it);

	return command;
}

/*
DO_FUN * do_func_lookup(char *name)
{ 
	for (int i = 0; do_func_table[i].name != NULL; i++)
	{
		if (!str_cmp(name, do_func_table[i].name))
			return do_func_table[i].func;
	}

	return NULL;
}

char do_func_name(DO_FUN *func)
{
	for (int i = 0; do_func_table[i].name != NULL; i++)
	{
		if (do_func_table[i].func == func)
			return do_func_table[i].name;
	}

	return NULL;
}

char do_func_display(DO_FUN *func)
{
	if ( !func ) return NULL;

	for (int i = 0; do_func_table[i].name != NULL; i++)
	{
		if (do_func_table[i].func == func)
			return do_func_table[i].name;
	}

	return "(invalid)";
}
*/

#define FUNC_LOOKUPS(f,t,n) \
t * f##_func_lookup(char *name) \
{ \
	for (int i = 0; f##_func_table[i].name != NULL; i++) \
	{ \
		if (!str_cmp(name, f##_func_table[i].name)) \
			return f##_func_table[i].func; \
	} \
 \
	return NULL; \
} \
 \
char *f##_func_name(t *func) \
{ \
	for (int i = 0; f##_func_table[i].name != NULL; i++) \
	{ \
		if (f##_func_table[i].func == func) \
			return f##_func_table[i].name; \
	} \
 \
	return NULL; \
} \
 \
char *f##_func_display(t *func) \
{ \
	if ( n ) return NULL; \
 \
	for (int i = 0; f##_func_table[i].name != NULL; i++) \
	{ \
		if (f##_func_table[i].func == func) \
			return f##_func_table[i].name; \
	} \
 \
	return "(invalid)"; \
} \
 \

FUNC_LOOKUPS(do, DO_FUN,(!func))

// Create a function to load commands from a command file. If the command file does not exist, boostrap one from the cmd_table.

void save_command(FILE *fp, CMD_DATA *command)
{
    fprintf(fp, "#COMMAND %s~\n", command->name);
    fprintf(fp, "Enabled %d\n", command->enabled);
    fprintf(fp, "Function %s~\n", do_func_name(command->function));
    fprintf(fp, "Rank %d\n", command->rank);
    fprintf(fp, "Log %d\n", command->log);
    fprintf(fp, "Position %d\n", command->position);
    fprintf(fp, "Type %d\n", command->type);
    fprintf(fp, "Addl_Types %ld\n", command->addl_types);
    fprintf(fp, "Flags %ld\n", command->command_flags);
    fprintf(fp, "Comments %s~\n", command->comments);
    fprintf(fp, "Description %s~\n", command->description);
    if (command->help_keywords != NULL && !IS_NULLSTR(command->help_keywords->string))
	    fprintf(fp, "HelpKeywords %s~\n", command->help_keywords->string);
    if (!IS_NULLSTR(command->reason))
        fprintf(fp, "Reason %s~\n", command->reason);
    if (!IS_NULLSTR(command->summary))
        fprintf(fp, "Summary %s~\n", command->summary);
    fprintf(fp, "#-COMMAND\n");
}

void save_commands()
{
    FILE *fp;

    log_string("save_commands: saving " COMMANDS_FILE);
    if ((fp = fopen(COMMANDS_FILE, "w")) == NULL)
    {
        bug("save_commands: fopen", 0);
        perror(COMMANDS_FILE);
    }
    else
    {
        log_string(formatf("save_commands: Saving %ld commands", commands_list->size));
        
        ITERATOR it;
        CMD_DATA *command;

        int count = 0;
        iterator_start(&it, commands_list);
        while((command = (CMD_DATA *)iterator_nextdata(&it)))
        {
            count++;
        }
        iterator_stop(&it);
    //log_string(formatf("Found %d commands from iterating. (save_commands)", count));

        iterator_start(&it, commands_list);
        while((command = (CMD_DATA *)iterator_nextdata(&it)))
        {
    //        log_string(formatf("Saving command '%s'", command->name));
            save_command(fp, command);
        }
        iterator_stop(&it);
        fprintf(fp, "#END\n");
        fclose(fp);
    }
}

void insert_command(CMD_DATA *command)
{
    /*
    ITERATOR it;
    CMD_DATA *cmd;
    iterator_start(&it, commands_list);
    while((cmd = (CMD_DATA *)iterator_nextdata(&it)))
    {
        int cmp = str_cmp(command->name, cmd->name);
        if (cmp < 0)
        {
            iterator_insert_before(&it, command);
//            log_string(formatf("DBG2 Inserted command '%s', commands_list is now %ld entries long", command->name, commands_list->size));
            break;
        }
    }
    iterator_stop(&it);

    if (!cmd)
    {*/
        list_appendlink(commands_list, command);
//        log_string(formatf("DBG1 Inserted command '%s', commands_list is now %ld entries long", command->name, commands_list->size));
    //}

    

}

CMD_DATA *load_command(FILE *fp)
{
    CMD_DATA *command;
    char *word;
    bool fMatch;


    command = new_cmd();
    command->name = fread_string(fp);

    while(str_cmp((word = fread_word(fp)), "#-COMMAND"))
    {
        fMatch = true;

        switch(word[0])
        {
            case 'A':
                KEY("Addl_Types", command->addl_types, fread_number(fp));
                break;
            case 'C':
                KEY("Comments", command->comments, fread_string(fp));
                break;
            case 'D':
                KEY("Description", command->description, fread_string(fp));
                break;
            case 'E':
                KEY("Enabled", command->enabled, fread_number(fp));
                break;
            case 'H':
                if (!str_cmp(word, "HelpKeywords"))
                {
		            STRING_DATA *help;

                    help = new_string_data();
		            help->string = fread_string(fp);
			        command->help_keywords = help;
                        fMatch = true;
                        break;
                }
                break;
            case 'F':
                KEY("Flags", command->command_flags, fread_number(fp));
                if (!str_cmp(word, "Function"))
                {
                    char *name = fread_string(fp);
                    command->function = do_func_lookup(name);
                    fMatch = true;
                    break;
                }
                break;
            case 'L':
                KEY("Log", command->log, fread_number(fp));
                break;
            case 'P':
                KEY("Position", command->position, fread_number(fp));
                break;
            case 'R':
                KEY("Rank", command->rank, fread_number(fp));
                KEY("Reason", command->reason, fread_string(fp));
            case 'S':
                KEY("Summary", command->summary, fread_string(fp));
                break;
            case 'T':
                KEY("Type", command->type, fread_number(fp));
                break;
        }

        if (!fMatch)
        {
            bug(formatf("load_command: no match for '%s'\n\r", word), 0);
            fread_to_eol(fp);
        }
    }

    if (command->addl_types == 0 && command->type != 0)
    {
        TOGGLE_BIT(command->addl_types, flag_value(command_addl_types, flag_name(command_types, command->type)));
    }
    /*
    if (!str_cmp(command->help_keywords->string, "(null)"))
    {
        free_string_data(command->help_keywords);
        command->help_keywords = NULL;
    }
    if (!str_cmp(command->reason, "(null)"))
    {
        free_string(command->reason);
        command->reason = NULL;
    }
    */
    return command;
}

static void delete_command(void *ptr)
{
    free_cmd((CMD_DATA *)ptr);
}

bool load_commands()
{
    FILE *fp;
    CMD_DATA *command;

    commands_list = list_createx(false, NULL, delete_command);

    if (!IS_VALID(commands_list))
    {
        log_string("load_commands: commands_list is not valid.");
        return false;
    }

    log_string("load_commands: loading " COMMANDS_FILE);
    if ((fp = fopen(COMMANDS_FILE, "r")) == NULL)
    {
        log_string("load_commands: " COMMANDS_FILE " not found. Bootstrapping from cmd_table.");

        for (int i = 0; !IS_NULLSTR(cmd_table[i].name); i++)
        {
            if (*cmd_table[i].name == '\0')
                continue;
            
            log_string(formatf("Bootstrapping command '%s'", cmd_table[i].name));
            command = new_cmd();
            command->name = str_dup(cmd_table[i].name);
//            command->type = cmd_table[i].cmd_type;
            command->rank = cmd_table[i].rank;
            command->log = cmd_table[i].log;
            command->position = cmd_table[i].position;
            command->function = cmd_table[i].do_fun;
            command->type = cmd_table[i].cmd_type;

            if (!cmd_table[i].show)
                TOGGLE_BIT(command->command_flags, CMD_HIDE_LISTS);

            if (cmd_table[i].is_ooc)
                TOGGLE_BIT(command->command_flags, CMD_IS_OOC);

            command->enabled = true;

            insert_command(command);
//            log_string(formatf("DBG3 Bootstrapped command '%s', commands_list is now %ld entries long", command->name, commands_list->size));
        }
        save_commands();
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
                    if (!str_cmp(word, "#COMMAND"))
                    {
                        command = load_command(fp);
                        
                        insert_command(command);
                        fMatch = true;
                        break;
                    }
                    break;
            }

            if (!fMatch)
            {
                bug(formatf("load_commands: no match for word '%s'", word),0);
                fread_to_eol(fp);
            }
        }
    }
    log_string(formatf("load_commands: Loaded %ld commands", commands_list->size));

    return true;
}

void do_cmdlist(CHAR_DATA *ch, char *argument)
{
    BUFFER *buffer = new_buf();
    char buf[MSL];
    char cmd_colour[3];
    char line_colour[3];
    char helpstatus[15];

        CMD_DATA *command;
        int count = 0;

        add_buf(buffer, "Commands:\n");
        add_buf(buffer, "####  Name               Level  Position    Log    Enabled  Function       Help  \n");
        add_buf(buffer, "----  ----               -----  --------  ------   -------  --------     --------\n");

        ITERATOR it;
        iterator_start(&it, commands_list);
        while((command = (CMD_DATA *)iterator_nextdata(&it)))
        {

            switch(command->type)
            {
                case CMDTYPE_NONE:
                    sprintf(cmd_colour, "{X");
                    break;
                case CMDTYPE_MOVE:
                    sprintf(cmd_colour, "{Y");
                    break;
                case CMDTYPE_COMBAT:
                    sprintf(cmd_colour, "{R");
                    break;
                case CMDTYPE_OBJECT:
                    sprintf(cmd_colour, "{J");
                    break;
                case CMDTYPE_INFO:
                    sprintf(cmd_colour, "{C");
                    break;
                case CMDTYPE_COMM:
                    sprintf(cmd_colour, "{M");
                    break;
                case CMDTYPE_RACIAL:
                    sprintf(cmd_colour, "{B");
                    break;
                case CMDTYPE_OOC:
                    sprintf(cmd_colour, "{G");
                    break;
                case CMDTYPE_IMMORTAL:
                    sprintf(cmd_colour, "{A");
                    break;
                case CMDTYPE_OLC:
                    sprintf(cmd_colour, "{P");
                    break;
                case CMDTYPE_ADMIN:
                    sprintf(cmd_colour, "{O");
                    break;
                default:
                    sprintf(cmd_colour, "{X");
                    break;
            }

            if (!command->enabled)
            {
                cmd_colour[1] = LOWER(cmd_colour[1]);
                sprintf(line_colour, "{D");
            }
            else
            {
                sprintf(line_colour, "{X");
            }

            if ((command->help_keywords == NULL || lookup_help_exact(command->help_keywords->string,get_staff_rank(ch),topHelpCat) == NULL) && command->summary == NULL) 
                sprintf(helpstatus, "{RNone{X");
            else if ((command->help_keywords == NULL || lookup_help_exact(command->help_keywords->string,get_staff_rank(ch),topHelpCat) == NULL) && command->summary != NULL)
                sprintf(helpstatus, "{YSummary{X");
            else if ((command->help_keywords != NULL && lookup_help_exact(command->help_keywords->string,get_staff_rank(ch),topHelpCat) != NULL) && command->summary == NULL)
                sprintf(helpstatus, "{YKeywords{X");
            else
                sprintf(helpstatus, "{GBoth{X");

            sprintf(buf, "{W%3d{X)  \t<send href=\"cmdshow %s|cmdedit %s\" hint=\"Show %s|Edit %s\">%s%s%s\t</send>%s%s %3d  %8s  %6s  %8s  %-12.12s %-12s{X\n\r",
                list_getindex(commands_list, command),
                command->name,
                command->name,
                command->name,
                command->name,
                cmd_colour,
                command->name,
                line_colour,
                pad_string(command->name, 20, NULL, NULL),
                line_colour,
                command->rank,
                position_table[command->position].name,
                log_flags[command->log].name,
                command->enabled ? "Enabled" : "Disabled",
                command->function ? do_func_name(command->function) : "None",
                helpstatus);
            add_buf(buffer, buf);
            count++;
        }
        iterator_stop(&it);

        sprintf(buf, "\n%d commands found.\n", count);
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

