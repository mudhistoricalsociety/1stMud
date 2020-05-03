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
#include "tables.h"


RoomIndex *
get_random_room (CharData * ch)
{
  RoomIndex *room;

  for (;;)
    {
      room = get_room_index (number_range (0, 65535));
      if (room != NULL)
	if (can_see_room (ch, room) && !room_is_private (room) &&
	    !IsSet (room->room_flags, ROOM_PRIVATE) &&
	    !IsSet (room->room_flags, ROOM_SOLITARY) &&
	    !IsSet (room->area->area_flags, AREA_CLOSED) &&
	    !IsSet (room->room_flags, ROOM_ARENA) &&
	    !IsSet (room->room_flags, ROOM_SAFE) && (IsNPC (ch) ||
						     IsSet (ch->act,
							    ACT_AGGRESSIVE)
						     ||
						     !IsSet
						     (room->room_flags,
						      ROOM_LAW)))
	  break;
    }

  return room;
}


Do_Fun (do_enter)
{
  RoomIndex *location;

  if (ch->fighting != NULL)
    return;


  if (!NullStr (argument))
    {
      RoomIndex *old_room;
      ObjData *portal;
      CharData *fch, *fch_next;

      old_room = ch->in_room;

      portal = get_obj_list (ch, argument, ch->in_room->content_first);

      if (portal == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}

      if (portal->item_type != ITEM_PORTAL ||
	  (IsSet (portal->value[1], EX_CLOSED) && !IsTrusted (ch, ANGEL)))
	{
	  chprintln (ch, "You can't seem to find a way in.");
	  return;
	}

      if (!IsTrusted (ch, ANGEL) &&
	  !IsSet (portal->value[2], GATE_NOCURSE) &&
	  (IsAffected (ch, AFF_CURSE) ||
	   IsSet (old_room->room_flags, ROOM_NO_RECALL)))
	{
	  chprintln (ch, "Something prevents you from leaving...");
	  return;
	}

      if (IsSet (portal->value[2], GATE_RANDOM) || portal->value[3] == -1)
	{
	  location = get_random_room (ch);
	  portal->value[3] = location->vnum;
	}
      else if (IsSet (portal->value[2], GATE_BUGGY)
	       && (number_percent () < 5))
	location = get_random_room (ch);
      else
	location = get_room_index (portal->value[3]);

      if (location == NULL || location == old_room ||
	  !can_see_room (ch, location) || (room_is_private (location)
					   && !IsTrusted (ch, IMPLEMENTOR)))
	{
	  act ("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
	  return;
	}

      if (IsNPC (ch) && IsSet (ch->act, ACT_AGGRESSIVE) &&
	  IsSet (location->room_flags, ROOM_LAW))
	{
	  chprintln (ch, "Something prevents you from leaving...");
	  return;
	}

      act ("$n steps into $p.", ch, portal, NULL, TO_ROOM);

      if (IsSet (portal->value[2], GATE_NORMAL_EXIT))
	act ("You enter $p.", ch, portal, NULL, TO_CHAR);
      else
	act ("You walk through $p and find yourself somewhere else...",
	     ch, portal, NULL, TO_CHAR);

      char_from_room (ch);
      char_to_room (ch, location);

      if (IsSet (portal->value[2], GATE_GOWITH))
	{
	  obj_from_room (portal);
	  obj_to_room (portal, location);
	}

      if (IsSet (portal->value[2], GATE_NORMAL_EXIT))
	act ("$n has arrived.", ch, portal, NULL, TO_ROOM);
      else
	act ("$n has arrived through $p.", ch, portal, NULL, TO_ROOM);

      do_function (ch, &do_look, "auto");


      if (portal->value[0] > 0)
	{
	  portal->value[0]--;
	  if (portal->value[0] == 0)
	    portal->value[0] = -1;
	}


      if (old_room == location)
	return;

      for (fch = old_room->person_first; fch != NULL; fch = fch_next)
	{
	  fch_next = fch->next_in_room;

	  if (portal == NULL || portal->value[0] == -1)

	    continue;

	  if (fch->master == ch && IsAffected (fch, AFF_CHARM) &&
	      fch->position < POS_STANDING)
	    do_function (fch, &do_stand, "");

	  if (fch->master == ch && fch->position == POS_STANDING)
	    {

	      if (IsSet (ch->in_room->room_flags, ROOM_LAW) &&
		  (IsNPC (fch) && IsSet (fch->act, ACT_AGGRESSIVE)))
		{
		  act ("You can't bring $N into the city.",
		       ch, NULL, fch, TO_CHAR);
		  act ("You aren't allowed in the city.",
		       fch, NULL, NULL, TO_CHAR);
		  continue;
		}

	      act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	      do_function (fch, &do_enter, argument);
	    }
	}

      if (portal != NULL && portal->value[0] == -1)
	{
	  act ("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
	  if (ch->in_room == old_room)
	    act ("$p fades out of existence.", ch, portal, NULL, TO_ROOM);
	  else if (old_room->person_first != NULL)
	    {
	      act ("$p fades out of existence.",
		   old_room->person_first, portal, NULL, TO_CHAR);
	      act ("$p fades out of existence.",
		   old_room->person_first, portal, NULL, TO_ROOM);
	    }
	  extract_obj (portal);
	}


      if (IsNPC (ch) && HasTriggerMob (ch, TRIG_ENTRY))
	p_percent_trigger (ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY);
      if (!IsNPC (ch))
	{
	  p_greet_trigger (ch, PRG_MPROG);
	  p_greet_trigger (ch, PRG_OPROG);
	  p_greet_trigger (ch, PRG_RPROG);
	}

      return;
    }

  chprintln (ch, "Nope, can't do it.");
  return;
}

Do_Fun (do_worship)
{
  CharData *priest;
  DeityData *i;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<deity>", "list", NULL);
      return;
    }

  if (!str_cmp (argument, "list"))
    {
      int e;

      for (e = ETHOS_LAWFUL_GOOD; e != ETHOS_CHAOTIC_EVIL; e--)
	{
	  for (i = deity_first; i; i = i->next)
	    {
	      if (i->ethos != (ethos_t) e)
		continue;

	      chprintlnf (ch, "\t%-12s (%s): %s", i->name,
			  flag_string (ethos_types, i->ethos), i->desc);
	    }
	}
      return;
    }

  for (priest = ch->in_room->person_first; priest != NULL;
       priest = priest->next_in_room)
    if (IsNPC (priest) && IsSet (priest->act, ACT_IS_HEALER))
      break;

  if (priest == NULL)
    {
      chprintln (ch, "There is no priest here!");
      return;
    }

  if (ch->pcdata->quest.points < 250)
    {
      chprintln (ch, "You need 250 questpoints to change your deity.");
      return;
    }

  i = deity_lookup (argument);

  if (i == NULL)
    {
      chprintln (ch, "That deity doesn't exist.");
      return;
    }

  ch->deity = i;
  chprintlnf (ch, "You now worship %s.", i->name);
  ch->pcdata->quest.points -= 250;
  return;
}


Do_Fun (do_heel)
{
  if (ch->pet == NULL)
    {
      chprintln (ch, "You don't have a pet!");
      return;
    }
  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    {
      act ("$N can't hear your whistle with all the screaming.", ch, NULL,
	   ch->pet, TO_CHAR);
      return;
    }
  char_from_room (ch->pet);
  char_to_room (ch->pet, ch->in_room);
  act ("$n lets out a loud whistle and $N comes running.", ch, NULL,
       ch->pet, TO_ROOM);
  act ("You let out a loud whistle and $N comes running.", ch, NULL,
       ch->pet, TO_CHAR);
}
