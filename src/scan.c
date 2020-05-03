/**************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefiting.  We hope that you share your changes too.  What goes       *
*  around, comes around.                                                  *
***************************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                        *
*       ROM has been brought to you by the ROM consortium                 *
*           Russ Taylor (rtaylor@hypercube.org)                           *
*           Gabrielle Taylor (gtaylor@hypercube.org)                      *
*           Brian Moore (zump@rom.org)                                    *
*       By using this code, you have agreed to follow the terms of the    *
*       ROM license, in the file Rom24/doc/rom.license                    *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/


#include "merc.h"
#include "interp.h"

char *const distance[4] = {
  "right here.", "nearby to the %s.", "not far %s.",
  "off in the distance %s."
};

Proto (void scan_list, (RoomIndex *, CharData *, int, int));
Proto (void scan_char, (CharData *, CharData *, int, int));

Do_Fun (do_scan)
{
  char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  RoomIndex *scan_room;
  ExitData *pExit;
  int door, depth;

  argument = one_argument (argument, arg1);

  if (NullStr (arg1))
    {
      act ("$n looks all around.", ch, NULL, NULL, TO_ROOM);
      chprintln (ch, "Looking around you see:");
      scan_list (ch->in_room, ch, 0, -1);

      for (door = 0; door < MAX_DIR; door++)
	{
	  if ((pExit = ch->in_room->exit[door]) != NULL)
	    scan_list (pExit->u1.to_room, ch, 1, door);
	}
      return;
    }
  else if ((door = get_direction (arg1)) == -1)
    {
      chprintln (ch, "Which way do you want to scan?");
      return;
    }

  act ("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
  act ("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
  sprintf (buf, "Looking %s you see:" NEWLINE, dir_name[door]);

  scan_room = ch->in_room;

  for (depth = 1; depth < 4; depth++)
    {
      if ((pExit = scan_room->exit[door]) != NULL)
	{
	  scan_room = pExit->u1.to_room;
	  scan_list (pExit->u1.to_room, ch, depth, door);
	}
    }
  return;
}

void
scan_list (RoomIndex * scan_room, CharData * ch, int depth, int door)
{
  CharData *rch;

  if (scan_room == NULL)
    return;
  for (rch = scan_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (rch == ch)
	continue;
      if (!IsNPC (rch) && rch->invis_level > get_trust (ch))
	continue;
      if (can_see (ch, rch))
	scan_char (rch, ch, depth, door);
    }
  return;
}

void
scan_char (CharData * victim, CharData * ch, int depth, int door)
{
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  buf[0] = '\0';

  if (IsQuester (ch) && victim == ch->pcdata->quest.mob)
    strcat (buf, "{r[{RTARGET{r]{x ");
  if (Gquester (ch) && IsNPC (victim) &&
      is_gqmob (ch->gquest, victim->pIndexData->vnum) != -1)
    strcat (buf, "(Gquest) ");
  strcat (buf, Pers (victim, ch));
  strcat (buf, ", ");
  sprintf (buf2, distance[depth], dir_name[door]);
  strcat (buf, buf2);

  chprintln (ch, buf);
  return;
}
