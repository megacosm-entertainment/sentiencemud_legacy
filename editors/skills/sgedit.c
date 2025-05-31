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

SGEDIT( sgedit_show )
{
	char buf[MSL];
	SKILL_GROUP *group;

	EDIT_SKILL_GROUP(ch, group);

	BUFFER *buffer = new_buf();

	sprintf(buf, "{BGroup: {C%s{x\n\r", group->name);
	add_buf(buffer, buf);

	add_buf(buffer, "{b=================================={x\n\r");

	ITERATOR it;
	SKILL_DATA *sk;
	SKILL_GROUP *gr;
	char *str;
	int cnt = 0;
	iterator_start(&it, group->contents);
	while((str = (char *)iterator_nextdata(&it)))
	{
		sk = get_skill_data(str);

		++cnt;

		if (IS_VALID(sk))
			sprintf(buf, "{C%3d {b- {%c%s{x\n\r", cnt, (sk->isspell?'G':'Y'), sk->name);
		else
		{
			gr = group_lookup(str);
			if (IS_VALID(gr))
				sprintf(buf, "{C%3d {b- {W%s{x\n\r", cnt, gr->name);
			else
				sprintf(buf, "{C%3d {b- {D%s{x\n\r", cnt, str);
		}

		add_buf(buffer, buf);
	}
	iterator_stop(&it);
	
	add_buf(buffer, "{b----------------------------------{x\n\r");
	sprintf(buf, "{BTotal: {C%d{x\n\r", list_size(group->contents));


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

SGEDIT( sgedit_create )
{
	SKILL_GROUP *group;

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  sgedit create {R<name>{x\n\r", ch);
		send_to_char("Please provide a unique name.\n\r", ch);
		return false;
	}

	group = group_lookup_exact(argument);
	if (IS_VALID(group))
	{
		send_to_char("Syntax:  sgedit create {R<name>{x\n\r", ch);
		send_to_char("Name is already taken.\n\r", ch);
		return false;
	}

	group = new_skill_group_data();
	group->name = str_dup(argument);
	insert_skill_group(group);

	char buf[MSL];
	sprintf(buf, "Group {W%s{x added.\n\r", argument);
	send_to_char(buf, ch);

	olc_set_editor(ch, ED_SGEDIT, group);
	return true;
}

SGEDIT( sgedit_add )
{
	SKILL_GROUP *group;

	EDIT_SKILL_GROUP(ch, group);

	smash_tilde(argument);
	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  sgedit add {R<name>{x\n\r", ch);
		send_to_char("Please provide the name for a skill, spell or group.\n\r", ch);
		return false;
	}

	if(group_has_item_exact(group, argument))
	{
		send_to_char("That group already contains that item.\n\r", ch);
		return false;
	}

	char buf[MSL];
	SKILL_DATA *sk = get_skill_data(argument);
	if (!IS_VALID(sk))
	{
		SKILL_GROUP *gr = group_lookup(argument);

		if (!IS_VALID(gr))
		{
			send_to_char("There is no skill, spell or group by that name.\n\r", ch);
			return false;
		}

		if (gr == group)
		{
			send_to_char("You cannot add a group to itself.\n\r", ch);
			return false;
		}

		// TODO: Check for circular linkages.

		sprintf(buf, "Added group '%s' to group %s.\n\r", gr->name, group->name);
	}
	else
		sprintf(buf, "Added %s '%s' to group %s.\n\r", (sk->isspell ? "spell" : "skill"), sk->name, group->name);

	insert_skill_group_item(group, str_dup(argument));

	send_to_char(buf, ch);
	return true;
}

SGEDIT( sgedit_remove )
{
	char buf[MSL];
	SKILL_GROUP *group;

	EDIT_SKILL_GROUP(ch, group);

	if (list_size(group->contents) < 1)
	{
		send_to_char("This group is already empty.\n\r", ch);
		return false;
	}

	int index;
	if (!is_number(argument) || (index = atoi(argument)) < 1 || index > list_size(group->contents))
	{
		sprintf(buf, "Syntax:  sgedit remove {R<1-%d>{x\n\r", list_size(group->contents));
		send_to_char(buf, ch);
		sprintf(buf, "Please provide a number from 1 to %d.\n\r", list_size(group->contents));
		send_to_char(buf, ch);
		return false;
	}

	list_remnthlink(group->contents, index, true);
	sprintf(buf, "Removed #%d item from group.\n\r", index);
	send_to_char(buf, ch);
	return true;
}

SGEDIT( sgedit_clear )
{
	SKILL_GROUP *group;

	EDIT_SKILL_GROUP(ch, group);

	if (list_size(group->contents) < 1)
	{
		send_to_char("This group is already empty.\n\r", ch);
		return false;
	}

	list_clear(group->contents);
	send_to_char("Group cleared.\n\r", ch);
	return true;
}
