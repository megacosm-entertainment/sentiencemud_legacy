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
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "wilds.h"

SPELL_FUNC(spell_soul_essence)
{
	char buf[MSL];
	OBJ_DATA *obj, *obj_next;
	char *arg = (char *) vo;
	int souls, i;
	int rating, rating2;
	bool found = false, all;

	if(IS_NPC(ch)) return false;

	if (!arg) return false;

	if (!arg[0] || (!is_number(arg) && str_cmp(arg,"all"))) {
		send_to_char("How much soul essence did you want to absorb?\n\r", ch);
		return false;
	}

	all = !str_cmp(arg,"all");
	souls = atoi(arg);

    // Use the lcarrying LLIST instead of the old carrying linked list
    if (ch->lcarrying) {
        i = 0;
        iterator_start(&it, ch->lcarrying);
        while ((obj = (OBJ_DATA *)iterator_nextdata(&it)) && (all || i < souls)) {
            if (obj->pIndexData->vnum == get_reserved_vnum("obj_pneuma_item")) {
                found = true;
                // Need to remove from list before extracting to prevent invalid list access
                list_remlink(ch->lcarrying, obj, false);
                extract_obj(obj);
                i++;
            }
        }
        iterator_stop(&it);
    }

	if (found) {
		rating = get_skill(ch,gsk_soul_essence); rating = UMAX(0,rating);
		rating2 = get_skill(ch,gsk_soul_essence); rating2 = UMAX(0,rating2);

		i = i * rating * rating2 / 10000;

		// Give boost for avatars and wraiths
		if(ch->race == gr_avatar || ch->race == gr_wraith)
			i = i * ( 240 + ch->tot_level ) / 240;

		// TODO: Account for pneuma boosts

		if(i > 0) {
			sprintf(buf, "{BYou feel {C%d{B soul%s flowing into you!{x\n\r", i, ((i==1)?"":"s"));
			send_to_char(buf,ch);
			act("{B$n glows briefly.{x", ch, NULL, NULL, NULL, NULL, NULL, NULL, TO_ROOM, NULL, NULL);

			if (boost_table[BOOST_PNEUMA].boost != 100)
			{
	    		send_to_char("{WPNEUMA boost!{x\n\r", ch);
	   			ch->pneuma += (i * boost_table[BOOST_PNEUMA].boost)/100;
			}
			else
	    		ch->pneuma += i;
    		
		} else
			send_to_char("You absorb soul essence, but it completely dissipates...\n\r", ch);
	} else
		send_to_char("You lack soul essence to absorb.\n\r", ch);
	return true;
}


