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
#include "recycle.h"
#include "vnums.h"
#include "tables.h"


Proto (int find_door, (CharData *, char *));
Proto (void quest_room_check, (CharData *));

Proto (bool has_key, (CharData *, vnum_t));

void
move_char (CharData * ch, int door, bool follow)
{
  CharData *fch;
  CharData *fch_next;
  RoomIndex *in_room;
  RoomIndex *to_room;
  ExitData *pexit;

  if (door < 0 || door > 5)
    {
      bugf ("Do_move: bad door %d.", door);
      return;
    }


  if (!IsNPC (ch)
      && (p_exit_trigger (ch, door, PRG_MPROG)
	  || p_exit_trigger (ch, door, PRG_OPROG)
	  || p_exit_trigger (ch, door, PRG_RPROG)))
    return;

  in_room = ch->in_room;
  if ((pexit = in_room->exit[door]) == NULL ||
      (to_room = pexit->u1.to_room) == NULL ||
      !can_see_room (ch, pexit->u1.to_room))
    {
      chprintln (ch, "Alas, you cannot go that way.");
      return;
    }

  if (IsSet (pexit->exit_info, EX_CLOSED) &&
      (!IsAffected (ch, AFF_PASS_DOOR) ||
       IsSet (pexit->exit_info, EX_NOPASS)) && !IsTrusted (ch, ANGEL))
    {
      act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master != NULL &&
      in_room == ch->master->in_room)
    {
      chprintln (ch, "What?  And leave your beloved master?");
      return;
    }

  if (!is_room_owner (ch, to_room) && room_is_private (to_room))
    {
      chprintln (ch, "That room is private right now.");
      return;
    }

  if (IsSet (to_room->area->area_flags, AREA_CLOSED))
    {
      chprintln (ch, "That area is closed to players.");
      return;
    }
  if (!IsNPC (ch))
    {
      int move;

      if (to_room->guild > -1 && to_room->guild < top_class &&
	  !is_class (ch, to_room->guild) && !IsImmortal (ch))
	{
	  chprintln (ch, "You aren't allowed in there.");
	  return;
	}

      if (in_room->sector_type == SECT_AIR ||
	  to_room->sector_type == SECT_AIR)
	{
	  if (!IsAffected (ch, AFF_FLYING) && !IsImmortal (ch))
	    {
	      chprintln (ch, "You can't fly.");
	      return;
	    }
	}

      if ((in_room->sector_type == SECT_WATER_NOSWIM ||
	   to_room->sector_type == SECT_WATER_NOSWIM) &&
	  !IsAffected (ch, AFF_FLYING))
	{
	  ObjData *obj;
	  bool found;


	  found = false;

	  if (IsImmortal (ch))
	    found = true;

	  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
	    {
	      if (obj->item_type == ITEM_BOAT)
		{
		  found = true;
		  break;
		}
	    }
	  if (!found)
	    {
	      chprintln (ch, "You need a boat to go there.");
	      return;
	    }
	}

      move =
	movement_loss[Min (SECT_MAX - 1, in_room->sector_type)] +
	movement_loss[Min (SECT_MAX - 1, to_room->sector_type)];

      move /= 2;


      if (IsAffected (ch, AFF_FLYING) || IsAffected (ch, AFF_HASTE))
	move /= 2;

      if (IsAffected (ch, AFF_SLOW))
	move *= 2;

      if (ch->move < move)
	{
	  chprintln (ch, "You are too exhausted.");
	  return;
	}

      WaitState (ch, 1);
      ch->move -= move;
    }

  if (ValidStance (GetStance (ch, STANCE_CURRENT)))
    do_function (ch, &do_stance, "");

  if (!IsAffected (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
    {
      char leave[MIL];

      if (IsAffected (ch, AFF_FLYING))
	strcpy (leave, "flies");
      else if (ch->desc && ch->desc->run_buf)
	strcpy (leave, "runs");
      else if (ch->in_room->sector_type == SECT_WATER_SWIM)
	strcpy (leave, "swims");
      else if (ch->hit < (ch->max_hit / 4))
	strcpy (leave, "crawls");
      else if (ch->hit < (ch->max_hit / 3))
	strcpy (leave, "limps");
      else if (ch->hit < (ch->max_hit / 2))
	strcpy (leave, "staggers");
      else if (IsDrunk (ch))
	strcpy (leave, "stumbles");
      else
	strcpy (leave, "leaves");

      act ("$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM);
    }

  char_from_room (ch);
  char_to_room (ch, to_room);

  if (!IsAffected (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO)
    {
      char enter[MIL];

      if (IsAffected (ch, AFF_FLYING))
	strcpy (enter, "flies in");
      else if (ch->desc && ch->desc->run_buf)
	strcpy (enter, "runs by");
      else if (ch->in_room->sector_type == SECT_WATER_SWIM)
	strcpy (enter, "swims in");
      else if (ch->hit < (ch->max_hit / 4))
	strcpy (enter, "crawls in");
      else if (ch->hit < (ch->max_hit / 3))
	strcpy (enter, "limps in");
      else if (ch->hit < (ch->max_hit / 2))
	strcpy (enter, "staggers in");
      else if (IsDrunk (ch))
	strcpy (enter, "stumbles in");
      else
	strcpy (enter, "arrives");
      act ("$n has arrived.", ch, NULL, NULL, TO_ROOM);
    }

  do_function (ch, &do_look, "auto");

  if (in_room == to_room)
    return;
  else if (in_room->area != to_room->area)
    {
      if (to_room->area->sound)
	send_sound (ch, to_room->area->sound);
    }

  for (fch = in_room->person_first; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;

      if (fch->master == ch && IsAffected (fch, AFF_CHARM) &&
	  fch->position < POS_STANDING)
	do_function (fch, &do_stand, "");

      if (fch->master == ch && fch->position == POS_STANDING &&
	  can_see_room (fch, to_room))
	{

	  if (IsSet (ch->in_room->room_flags, ROOM_LAW) &&
	      (IsNPC (fch) && IsSet (fch->act, ACT_AGGRESSIVE)))
	    {
	      act ("You can't bring $N into the city.", ch,
		   NULL, fch, TO_CHAR);
	      act ("You aren't allowed in the city.", fch, NULL,
		   NULL, TO_CHAR);
	      continue;
	    }

	  act ("You follow $N.", fch, NULL, ch, TO_CHAR);
	  move_char (fch, door, true);
	}
    }


  if (IsNPC (ch) && HasTriggerMob (ch, TRIG_ENTRY))
    p_percent_trigger (ch, NULL, NULL, NULL, NULL, NULL, TRIG_ENTRY);
  if (!IsNPC (ch))
    {
      p_greet_trigger (ch, PRG_MPROG);
      p_greet_trigger (ch, PRG_OPROG);
      p_greet_trigger (ch, PRG_RPROG);
      quest_room_check (ch);
    }

  return;
}

Do_Fun (do_north)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_NORTH, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

Do_Fun (do_east)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_EAST, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

Do_Fun (do_south)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_SOUTH, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

Do_Fun (do_west)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_WEST, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

Do_Fun (do_up)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_UP, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

Do_Fun (do_down)
{
  RoomIndex *was_room;

  was_room = ch->in_room;
  move_char (ch, DIR_DOWN, false);
  if (ch->desc && was_room == ch->in_room)
    free_runbuf (ch->desc);
  return;
}

int
find_door (CharData * ch, char *arg)
{
  ExitData *pexit;
  int door;

  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    door = 0;
  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    door = 1;
  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    door = 2;
  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    door = 3;
  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    door = 4;
  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    door = 5;
  else
    {
      for (door = 0; door < MAX_DIR; door++)
	{
	  if ((pexit = ch->in_room->exit[door]) != NULL &&
	      IsSet (pexit->exit_info, EX_ISDOOR) &&
	      pexit->keyword != NULL && is_name (arg, pexit->keyword))
	    return door;
	}
      act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
      return -1;
    }

  if ((pexit = ch->in_room->exit[door]) == NULL)
    {
      act ("I see no door $T here.", ch, NULL, arg, TO_CHAR);
      return -1;
    }

  if (!IsSet (pexit->exit_info, EX_ISDOOR))
    {
      chprintln (ch, "You can't do that.");
      return -1;
    }

  return door;
}

Do_Fun (do_open)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int door;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Open what?");
      return;
    }

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {

      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IsSet (obj->value[1], EX_ISDOOR))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }

	  if (!IsSet (obj->value[1], EX_CLOSED))
	    {
	      chprintln (ch, "It's already open.");
	      return;
	    }

	  if (IsSet (obj->value[1], EX_LOCKED))
	    {
	      chprintln (ch, "It's locked.");
	      return;
	    }

	  RemBit (obj->value[1], EX_CLOSED);
	  act ("You open $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}


      if (obj->item_type != ITEM_CONTAINER)
	{
	  chprintln (ch, "That's not a container.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSED))
	{
	  chprintln (ch, "It's already open.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSEABLE))
	{
	  chprintln (ch, "You can't do that.");
	  return;
	}
      if (IsSet (obj->value[1], CONT_LOCKED))
	{
	  chprintln (ch, "It's locked.");
	  return;
	}

      RemBit (obj->value[1], CONT_CLOSED);
      act ("You open $p.", ch, obj, NULL, TO_CHAR);
      act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
      return;
    }

  if ((door = find_door (ch, arg)) >= 0)
    {

      RoomIndex *to_room;
      ExitData *pexit;
      ExitData *pexit_rev;

      pexit = ch->in_room->exit[door];
      if (!IsSet (pexit->exit_info, EX_CLOSED))
	{
	  chprintln (ch, "It's already open.");
	  return;
	}
      if (IsSet (pexit->exit_info, EX_LOCKED))
	{
	  chprintln (ch, "It's locked.");
	  return;
	}

      RemBit (pexit->exit_info, EX_CLOSED);
      act ("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      chprintln (ch, "Ok.");


      if ((to_room = pexit->u1.to_room) != NULL &&
	  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL &&
	  pexit_rev->u1.to_room == ch->in_room)
	{
	  CharData *rch;

	  RemBit (pexit_rev->exit_info, EX_CLOSED);
	  for (rch = to_room->person_first; rch != NULL;
	       rch = rch->next_in_room)
	    act ("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
	}
    }

  return;
}

Do_Fun (do_close)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int door;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Close what?");
      return;
    }

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {

      if (obj->item_type == ITEM_PORTAL)
	{

	  if (!IsSet (obj->value[1], EX_ISDOOR) ||
	      IsSet (obj->value[1], EX_NOCLOSE))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }

	  if (IsSet (obj->value[1], EX_CLOSED))
	    {
	      chprintln (ch, "It's already closed.");
	      return;
	    }

	  SetBit (obj->value[1], EX_CLOSED);
	  act ("You close $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}


      if (obj->item_type != ITEM_CONTAINER)
	{
	  chprintln (ch, "That's not a container.");
	  return;
	}
      if (IsSet (obj->value[1], CONT_CLOSED))
	{
	  chprintln (ch, "It's already closed.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSEABLE))
	{
	  chprintln (ch, "You can't do that.");
	  return;
	}

      SetBit (obj->value[1], CONT_CLOSED);
      act ("You close $p.", ch, obj, NULL, TO_CHAR);
      act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
      return;
    }

  if ((door = find_door (ch, arg)) >= 0)
    {

      RoomIndex *to_room;
      ExitData *pexit;
      ExitData *pexit_rev;

      pexit = ch->in_room->exit[door];
      if (IsSet (pexit->exit_info, EX_CLOSED))
	{
	  chprintln (ch, "It's already closed.");
	  return;
	}

      SetBit (pexit->exit_info, EX_CLOSED);
      act ("$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      chprintln (ch, "Ok.");


      if ((to_room = pexit->u1.to_room) != NULL &&
	  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL &&
	  pexit_rev->u1.to_room == ch->in_room)
	{
	  CharData *rch;

	  SetBit (pexit_rev->exit_info, EX_CLOSED);
	  for (rch = to_room->person_first; rch != NULL;
	       rch = rch->next_in_room)
	    act ("The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR);
	}
    }

  return;
}

bool
has_key (CharData * ch, vnum_t key)
{
  ObjData *obj;

  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData->vnum == key)
	return true;
    }

  return false;
}

Do_Fun (do_lock)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int door;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Lock what?");
      return;
    }

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {

      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IsSet (obj->value[1], EX_ISDOOR) ||
	      IsSet (obj->value[1], EX_NOCLOSE))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }
	  if (!IsSet (obj->value[1], EX_CLOSED))
	    {
	      chprintln (ch, "It's not closed.");
	      return;
	    }

	  if (obj->value[4] < 0 || IsSet (obj->value[1], EX_NOLOCK))
	    {
	      chprintln (ch, "It can't be locked.");
	      return;
	    }

	  if (!has_key (ch, obj->value[4]))
	    {
	      chprintln (ch, "You lack the key.");
	      return;
	    }

	  if (IsSet (obj->value[1], EX_LOCKED))
	    {
	      chprintln (ch, "It's already locked.");
	      return;
	    }

	  SetBit (obj->value[1], EX_LOCKED);
	  act ("You lock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}


      if (obj->item_type != ITEM_CONTAINER)
	{
	  chprintln (ch, "That's not a container.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSED))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (obj->value[2] < 0)
	{
	  chprintln (ch, "It can't be locked.");
	  return;
	}
      if (!has_key (ch, obj->value[2]))
	{
	  chprintln (ch, "You lack the key.");
	  return;
	}
      if (IsSet (obj->value[1], CONT_LOCKED))
	{
	  chprintln (ch, "It's already locked.");
	  return;
	}

      SetBit (obj->value[1], CONT_LOCKED);
      act ("You lock $p.", ch, obj, NULL, TO_CHAR);
      act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
      return;
    }

  if ((door = find_door (ch, arg)) >= 0)
    {

      RoomIndex *to_room;
      ExitData *pexit;
      ExitData *pexit_rev;

      pexit = ch->in_room->exit[door];
      if (!IsSet (pexit->exit_info, EX_CLOSED))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (pexit->key < 0)
	{
	  chprintln (ch, "It can't be locked.");
	  return;
	}
      if (!has_key (ch, pexit->key))
	{
	  chprintln (ch, "You lack the key.");
	  return;
	}
      if (IsSet (pexit->exit_info, EX_LOCKED))
	{
	  chprintln (ch, "It's already locked.");
	  return;
	}

      SetBit (pexit->exit_info, EX_LOCKED);
      chprintln (ch, "*Click*");
      act ("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);


      if ((to_room = pexit->u1.to_room) != NULL &&
	  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL &&
	  pexit_rev->u1.to_room == ch->in_room)
	{
	  SetBit (pexit_rev->exit_info, EX_LOCKED);
	}
    }

  return;
}

Do_Fun (do_unlock)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int door;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Unlock what?");
      return;
    }

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {

      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IsSet (obj->value[1], EX_ISDOOR))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }

	  if (!IsSet (obj->value[1], EX_CLOSED))
	    {
	      chprintln (ch, "It's not closed.");
	      return;
	    }

	  if (obj->value[4] < 0)
	    {
	      chprintln (ch, "It can't be unlocked.");
	      return;
	    }

	  if (!has_key (ch, obj->value[4]))
	    {
	      chprintln (ch, "You lack the key.");
	      return;
	    }

	  if (!IsSet (obj->value[1], EX_LOCKED))
	    {
	      chprintln (ch, "It's already unlocked.");
	      return;
	    }

	  RemBit (obj->value[1], EX_LOCKED);
	  act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}


      if (obj->item_type != ITEM_CONTAINER)
	{
	  chprintln (ch, "That's not a container.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSED))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (obj->value[2] < 0)
	{
	  chprintln (ch, "It can't be unlocked.");
	  return;
	}
      if (!has_key (ch, obj->value[2]))
	{
	  chprintln (ch, "You lack the key.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_LOCKED))
	{
	  chprintln (ch, "It's already unlocked.");
	  return;
	}

      RemBit (obj->value[1], CONT_LOCKED);
      act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
      act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
      return;
    }

  if ((door = find_door (ch, arg)) >= 0)
    {

      RoomIndex *to_room;
      ExitData *pexit;
      ExitData *pexit_rev;

      pexit = ch->in_room->exit[door];
      if (!IsSet (pexit->exit_info, EX_CLOSED))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (pexit->key < 0)
	{
	  chprintln (ch, "It can't be unlocked.");
	  return;
	}
      if (!has_key (ch, pexit->key))
	{
	  chprintln (ch, "You lack the key.");
	  return;
	}
      if (!IsSet (pexit->exit_info, EX_LOCKED))
	{
	  chprintln (ch, "It's already unlocked.");
	  return;
	}

      RemBit (pexit->exit_info, EX_LOCKED);
      chprintln (ch, "*Click*");
      act ("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);


      if ((to_room = pexit->u1.to_room) != NULL &&
	  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL &&
	  pexit_rev->u1.to_room == ch->in_room)
	{
	  RemBit (pexit_rev->exit_info, EX_LOCKED);
	}
    }

  return;
}

Do_Fun (do_pick)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *gch;
  ObjData *obj;
  int door;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Pick what?");
      return;
    }

  WaitState (ch, skill_table[gsn_pick_lock].beats);


  for (gch = ch->in_room->person_first; gch; gch = gch->next_in_room)
    {
      if (IsNPC (gch) && IsAwake (gch) && ch->level + 5 < gch->level)
	{
	  act ("$N is standing too close to the lock.", ch, NULL,
	       gch, TO_CHAR);
	  return;
	}
    }

  if (!IsNPC (ch) && number_percent () > get_skill (ch, gsn_pick_lock))
    {
      chprintln (ch, "You failed.");
      check_improve (ch, gsn_pick_lock, false, 2);
      return;
    }

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {

      if (obj->item_type == ITEM_PORTAL)
	{
	  if (!IsSet (obj->value[1], EX_ISDOOR))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }

	  if (!IsSet (obj->value[1], EX_CLOSED))
	    {
	      chprintln (ch, "It's not closed.");
	      return;
	    }

	  if (obj->value[4] < 0)
	    {
	      chprintln (ch, "It can't be unlocked.");
	      return;
	    }

	  if (IsSet (obj->value[1], EX_PICKPROOF))
	    {
	      chprintln (ch, "You failed.");
	      return;
	    }

	  RemBit (obj->value[1], EX_LOCKED);
	  act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
	  check_improve (ch, gsn_pick_lock, true, 2);
	  return;
	}


      if (obj->item_type != ITEM_CONTAINER)
	{
	  chprintln (ch, "That's not a container.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_CLOSED))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (obj->value[2] < 0)
	{
	  chprintln (ch, "It can't be unlocked.");
	  return;
	}
      if (!IsSet (obj->value[1], CONT_LOCKED))
	{
	  chprintln (ch, "It's already unlocked.");
	  return;
	}
      if (IsSet (obj->value[1], CONT_PICKPROOF))
	{
	  chprintln (ch, "You failed.");
	  return;
	}

      RemBit (obj->value[1], CONT_LOCKED);
      act ("You pick the lock on $p.", ch, obj, NULL, TO_CHAR);
      act ("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
      check_improve (ch, gsn_pick_lock, true, 2);
      return;
    }

  if ((door = find_door (ch, arg)) >= 0)
    {

      RoomIndex *to_room;
      ExitData *pexit;
      ExitData *pexit_rev;

      pexit = ch->in_room->exit[door];
      if (!IsSet (pexit->exit_info, EX_CLOSED) && !IsImmortal (ch))
	{
	  chprintln (ch, "It's not closed.");
	  return;
	}
      if (pexit->key < 0 && !IsImmortal (ch))
	{
	  chprintln (ch, "It can't be picked.");
	  return;
	}
      if (!IsSet (pexit->exit_info, EX_LOCKED))
	{
	  chprintln (ch, "It's already unlocked.");
	  return;
	}
      if (IsSet (pexit->exit_info, EX_PICKPROOF) && !IsImmortal (ch))
	{
	  chprintln (ch, "You failed.");
	  return;
	}

      RemBit (pexit->exit_info, EX_LOCKED);
      chprintln (ch, "*Click*");
      act ("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      check_improve (ch, gsn_pick_lock, true, 2);


      if ((to_room = pexit->u1.to_room) != NULL &&
	  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL &&
	  pexit_rev->u1.to_room == ch->in_room)
	{
	  RemBit (pexit_rev->exit_info, EX_LOCKED);
	}
    }

  return;
}

Do_Fun (do_stand)
{
  ObjData *obj = NULL;

  if (!NullStr (argument))
    {
      if (ch->position == POS_FIGHTING)
	{
	  chprintln (ch, "Maybe you should finish fighting first?");
	  return;
	}
      obj = get_obj_list (ch, argument, ch->in_room->content_first);
      if (obj == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
      if (obj->item_type != ITEM_FURNITURE ||
	  (!IsSet (obj->value[2], STAND_AT) &&
	   !IsSet (obj->value[2], STAND_ON) &&
	   !IsSet (obj->value[2], STAND_IN)))
	{
	  chprintln (ch, "You can't seem to find a place to stand.");
	  return;
	}
      if (ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no room to stand on $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  return;
	}
      ch->on = obj;
      if (HasTriggerObj (obj, TRIG_SIT))
	p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT);
    }

  switch (ch->position)
    {
    case POS_SLEEPING:
      if (IsAffected (ch, AFF_SLEEP))
	{
	  chprintln (ch, "You can't wake up!");
	  return;
	}

      if (obj == NULL)
	{
	  chprintln (ch, "You wake and stand up.");
	  act ("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
	  ch->on = NULL;
	}
      else if (IsSet (obj->value[2], STAND_AT))
	{
	  act_new ("You wake and stand at $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], STAND_ON))
	{
	  act_new ("You wake and stand on $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act_new ("You wake and stand in $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_STANDING;
      do_function (ch, &do_look, "auto");
      break;

    case POS_RESTING:
    case POS_SITTING:
      if (obj == NULL)
	{
	  chprintln (ch, "You stand up.");
	  act ("$n stands up.", ch, NULL, NULL, TO_ROOM);
	  ch->on = NULL;
	}
      else if (IsSet (obj->value[2], STAND_AT))
	{
	  act ("You stand at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n stands at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], STAND_ON))
	{
	  act ("You stand on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act ("You stand in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n stands on $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_STANDING;
      break;

    case POS_STANDING:
      chprintln (ch, "You are already standing.");
      break;

    case POS_FIGHTING:
      chprintln (ch, "You are already fighting!");
      break;
    default:
      break;
    }

  return;
}

Do_Fun (do_rest)
{
  ObjData *obj = NULL;

  if (ch->position == POS_FIGHTING)
    {
      chprintln (ch, "You are already fighting!");
      return;
    }


  if (!NullStr (argument))
    {
      obj = get_obj_list (ch, argument, ch->in_room->content_first);
      if (obj == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
    }
  else
    obj = ch->on;

  if (obj != NULL)
    {
      if (obj->item_type != ITEM_FURNITURE ||
	  (!IsSet (obj->value[2], REST_ON) &&
	   !IsSet (obj->value[2], REST_IN) &&
	   !IsSet (obj->value[2], REST_AT)))
	{
	  chprintln (ch, "You can't rest on that.");
	  return;
	}

      if (obj != NULL && ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  return;
	}

      ch->on = obj;
      if (HasTriggerObj (obj, TRIG_SIT))
	p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT);

    }

  switch (ch->position)
    {
    case POS_SLEEPING:
      if (IsAffected (ch, AFF_SLEEP))
	{
	  chprintln (ch, "You can't wake up!");
	  return;
	}

      if (obj == NULL)
	{
	  chprintln (ch, "You wake up and start resting.");
	  act ("$n wakes up and starts resting.", ch, NULL, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_AT))
	{
	  act_new ("You wake up and rest at $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_ON))
	{
	  act_new ("You wake up and rest on $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests on $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act_new ("You wake up and rest in $p.", ch, obj, NULL,
		   TO_CHAR, POS_SLEEPING);
	  act ("$n wakes up and rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;

    case POS_RESTING:
      chprintln (ch, "You are already resting.");
      break;

    case POS_STANDING:
      if (obj == NULL)
	{
	  chprintln (ch, "You rest.");
	  act ("$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_AT))
	{
	  act ("You sit down at $p and rest.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_ON))
	{
	  act ("You sit on $p and rest.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p and rests.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;

    case POS_SITTING:
      if (obj == NULL)
	{
	  chprintln (ch, "You rest.");
	  act ("$n rests.", ch, NULL, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_AT))
	{
	  act ("You rest at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], REST_ON))
	{
	  act ("You rest on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests on $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act ("You rest in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n rests in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_RESTING;
      break;
    default:
      break;
    }

  return;
}

Do_Fun (do_sit)
{
  ObjData *obj = NULL;

  if (ch->position == POS_FIGHTING)
    {
      chprintln (ch, "Maybe you should finish this fight first?");
      return;
    }


  if (!NullStr (argument))
    {
      obj = get_obj_list (ch, argument, ch->in_room->content_first);
      if (obj == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
    }
  else
    obj = ch->on;

  if (obj != NULL)
    {
      if (obj->item_type != ITEM_FURNITURE ||
	  (!IsSet (obj->value[2], SIT_ON) && !IsSet (obj->value[2], SIT_IN)
	   && !IsSet (obj->value[2], SIT_AT)))
	{
	  chprintln (ch, "You can't sit on that.");
	  return;
	}

      if (obj != NULL && ch->on != obj && count_users (obj) >= obj->value[0])
	{
	  act_new ("There's no more room on $p.", ch, obj, NULL, TO_CHAR,
		   POS_DEAD);
	  return;
	}

      ch->on = obj;
      if (HasTriggerObj (obj, TRIG_SIT))
	p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT);
    }
  switch (ch->position)
    {
    case POS_SLEEPING:
      if (IsAffected (ch, AFF_SLEEP))
	{
	  chprintln (ch, "You can't wake up!");
	  return;
	}

      if (obj == NULL)
	{
	  chprintln (ch, "You wake and sit up.");
	  act ("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], SIT_AT))
	{
	  act_new ("You wake and sit at $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], SIT_ON))
	{
	  act_new ("You wake and sit on $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act_new ("You wake and sit in $p.", ch, obj, NULL,
		   TO_CHAR, POS_DEAD);
	  act ("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
	}

      ch->position = POS_SITTING;
      break;
    case POS_RESTING:
      if (obj == NULL)
	chprintln (ch, "You stop resting.");
      else if (IsSet (obj->value[2], SIT_AT))
	{
	  act ("You sit at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], SIT_ON))
	{
	  act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_SITTING;
      break;
    case POS_SITTING:
      chprintln (ch, "You are already sitting down.");
      break;
    case POS_STANDING:
      if (obj == NULL)
	{
	  chprintln (ch, "You sit down.");
	  act ("$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], SIT_AT))
	{
	  act ("You sit down at $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
	}
      else if (IsSet (obj->value[2], SIT_ON))
	{
	  act ("You sit on $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits on $p.", ch, obj, NULL, TO_ROOM);
	}
      else
	{
	  act ("You sit down in $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
	}
      ch->position = POS_SITTING;
      break;
    default:
      break;
    }
  return;
}

Do_Fun (do_sleep)
{
  ObjData *obj = NULL;

  switch (ch->position)
    {
    case POS_SLEEPING:
      chprintln (ch, "You are already sleeping.");
      break;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
      if (NullStr (argument) && ch->on == NULL)
	{
	  chprintln (ch, "You go to sleep.");
	  act ("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
	  ch->position = POS_SLEEPING;
	}
      else
	{

	  if (NullStr (argument))
	    obj = ch->on;
	  else
	    obj = get_obj_list (ch, argument, ch->in_room->content_first);

	  if (obj == NULL)
	    {
	      chprintln (ch, "You don't see that here.");
	      return;
	    }
	  if (obj->item_type != ITEM_FURNITURE ||
	      (!IsSet (obj->value[2], SLEEP_ON) &&
	       !IsSet (obj->value[2], SLEEP_IN) &&
	       !IsSet (obj->value[2], SLEEP_AT)))
	    {
	      chprintln (ch, "You can't sleep on that!");
	      return;
	    }

	  if (ch->on != obj && count_users (obj) >= obj->value[0])
	    {
	      act_new ("There is no room on $p for you.", ch,
		       obj, NULL, TO_CHAR, POS_DEAD);
	      return;
	    }

	  ch->on = obj;
	  if (HasTriggerObj (obj, TRIG_SIT))
	    p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL, TRIG_SIT);

	  if (IsSet (obj->value[2], SLEEP_AT))
	    {
	      act ("You go to sleep at $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep at $p.", ch, obj, NULL, TO_ROOM);
	    }
	  else if (IsSet (obj->value[2], SLEEP_ON))
	    {
	      act ("You go to sleep on $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
	    }
	  else
	    {
	      act ("You go to sleep in $p.", ch, obj, NULL, TO_CHAR);
	      act ("$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
	    }
	  ch->position = POS_SLEEPING;
	}
      break;

    case POS_FIGHTING:
      chprintln (ch, "You are already fighting!");
      break;
    default:
      break;
    }

  return;
}

Do_Fun (do_wake)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      do_function (ch, &do_stand, "");
      return;
    }

  if (!IsAwake (ch))
    {
      chprintln (ch, "You are asleep yourself!");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsAwake (victim))
    {
      act ("$N is already awake.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (IsAffected (victim, AFF_SLEEP))
    {
      act ("You can't wake $M!", ch, NULL, victim, TO_CHAR);
      return;
    }

  act_new ("$n wakes you.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
  do_function (ch, &do_stand, "");
  return;
}

Do_Fun (do_sneak)
{
  AffectData af;

  chprintln (ch, "You attempt to move silently.");
  affect_strip (ch, gsn_sneak);

  if (IsAffected (ch, AFF_SNEAK))
    return;

  if (number_percent () < get_skill (ch, gsn_sneak))
    {
      check_improve (ch, gsn_sneak, true, 3);
      af.where = TO_AFFECTS;
      af.type = gsn_sneak;
      af.level = ch->level;
      af.duration = ch->level;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_SNEAK;
      affect_to_char (ch, &af);
    }
  else
    check_improve (ch, gsn_sneak, false, 3);

  return;
}

Do_Fun (do_hide)
{
  chprintln (ch, "You attempt to hide.");

  if (IsAffected (ch, AFF_HIDE))
    RemBit (ch->affected_by, AFF_HIDE);

  if (number_percent () < get_skill (ch, gsn_hide))
    {
      SetBit (ch->affected_by, AFF_HIDE);
      check_improve (ch, gsn_hide, true, 3);
    }
  else
    check_improve (ch, gsn_hide, false, 3);

  return;
}


Do_Fun (do_visible)
{
  affect_strip (ch, gsn_invis);
  affect_strip (ch, gsn_mass_invis);
  affect_strip (ch, gsn_sneak);
  RemBit (ch->affected_by, AFF_HIDE);
  RemBit (ch->affected_by, AFF_INVISIBLE);
  RemBit (ch->affected_by, AFF_SNEAK);
  chprintln (ch, "Ok.");
  return;
}

void
perform_recall (CharData * ch, RoomIndex * location, const char *what)
{
  CharData *victim;

  if (IsNPC (ch) && !IsSet (ch->act, ACT_PET))
    {
      chprintlnf (ch, "Only players can %s.", what);
      return;
    }
  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    {
      chprintlnf (ch, "You can't %s while in the arena!", what);
      return;
    }

  act ("$n prays for transportation!", ch, 0, 0, TO_ROOM);

  if (location == NULL)
    {
      chprintln (ch, "You are completely lost.");
      return;
    }

  if (ch->in_room == location)
    return;

  if (IsSet (ch->in_room->room_flags, ROOM_NO_RECALL) ||
      IsAffected (ch, AFF_CURSE))
    {
      act ("$g has forsaken you.", ch, NULL, NULL, TO_CHAR);
      return;
    }

  if ((victim = ch->fighting) != NULL)
    {
      int lose, skill;

      skill = get_skill (ch, gsn_recall);

      if (number_percent () < 80 * skill / 100)
	{
	  check_improve (ch, gsn_recall, false, 6);
	  WaitState (ch, 4);
	  chprintln (ch, "You failed!.");
	  return;
	}

      lose = (ch->desc != NULL) ? 25 : 50;
      gain_exp (ch, 0 - lose);
      check_improve (ch, gsn_recall, true, 4);
      chprintlnf (ch, "You %s from combat!  You lose %d exps.", what, lose);
      stop_fighting (ch, true);

    }

  ch->move /= 2;
  act ("$n disappears.", ch, NULL, NULL, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, location);
  act ("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
  do_function (ch, &do_look, "auto");

  if (ch->pet != NULL)
    {
      act ("$n disappears.", ch, NULL, NULL, TO_ROOM);
      char_from_room (ch->pet);
      char_to_room (ch->pet, location);
      act ("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
    }
}

Do_Fun (do_recall)
{
  RoomIndex *location;

  if (!ch->in_room || !ch->in_room->area->recall
      || (location = get_room_index (ch->in_room->area->recall)) == NULL)
    location = get_room_index (ROOM_VNUM_TEMPLE);

  perform_recall (ch, location, "recall");
  return;
}

Do_Fun (do_train)
{
  char buf[MAX_STRING_LENGTH];
  CharData *mob;
  int stat = -1;
  int cost;

  if (IsNPC (ch))
    return;


  for (mob = ch->in_room->person_first; mob; mob = mob->next_in_room)
    {
      if (IsNPC (mob) && IsSet (mob->act, ACT_TRAIN))
	break;
    }

  if (mob == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  if (NullStr (argument))
    {
      chprintlnf (ch, "You have %d training sessions.", ch->train);
      argument = "foo";
    }

  cost = 1;

  if ((stat = (int) flag_value (stat_types, argument)) != NO_FLAG)
    {
      if (is_prime_stat (ch, stat))
	cost = 1;
    }
  else if (!str_cmp (argument, "hp"))
    cost = 1;

  else if (!str_cmp (argument, "mana"))
    cost = 1;

  else
    {
      strcpy (buf, "You can train:");

      for (stat = 0; stat < STAT_MAX; stat++)
	{
	  if (ch->perm_stat[stat] < get_max_train (ch, stat))
	    sprintf (buf + strlen (buf), " %.3s", stat_types[stat].name);
	}
      strcat (buf, " hp mana.");

      chprintln (ch, buf);
      return;
    }

  if (!str_cmp ("hp", argument))
    {
      if (cost > ch->train)
	{
	  chprintln (ch, "You don't have enough training sessions.");
	  return;
	}

      ch->train -= cost;
      ch->pcdata->perm_hit += 10;
      ch->max_hit += 10;
      ch->hit += 10;
      act ("Your durability increases!", ch, NULL, NULL, TO_CHAR);
      act ("$n's durability increases!", ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (!str_cmp ("mana", argument))
    {
      if (cost > ch->train)
	{
	  chprintln (ch, "You don't have enough training sessions.");
	  return;
	}

      ch->train -= cost;
      ch->pcdata->perm_mana += 10;
      ch->max_mana += 10;
      ch->mana += 10;
      act ("Your power increases!", ch, NULL, NULL, TO_CHAR);
      act ("$n's power increases!", ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (ch->perm_stat[stat] >= get_max_train (ch, stat))
    {
      act ("Your $T is already at maximum.", ch, NULL,
	   stat_types[stat].name, TO_CHAR);
      return;
    }

  if (cost > ch->train)
    {
      chprintln (ch, "You don't have enough training sessions.");
      return;
    }

  ch->train -= cost;

  ch->perm_stat[stat] += 1;
  act ("Your $T increases!", ch, NULL, stat_types[stat].name, TO_CHAR);
  act ("$n's $T increases!", ch, NULL, stat_types[stat].name, TO_ROOM);
  return;
}

Do_Fun (do_run)
{
  char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
  char *p;
  bool dFound = false;

  if (!ch->desc || *argument == '\0')
    {
      chprintln (ch, "You run in place!");
      return;
    }

  buf[0] = '\0';

  while (*argument != '\0')
    {
      argument = one_argument (argument, arg);
      strcat (buf, arg);
    }

  for (p = buf + strlen (buf) - 1; p >= buf; p--)
    {
      if (!isdigit (*p))
	{
	  switch (*p)
	    {
	    case 'n':
	    case 's':
	    case 'e':
	    case 'w':
	    case 'u':
	    case 'd':
	      dFound = true;
	      break;

	    case 'o':
	      break;

	    default:
	      chprintln (ch, "Invalid direction!");
	      return;
	    }
	}
      else if (!dFound)
	*p = '\0';
    }

  if (!dFound)
    {
      chprintln (ch, "No directions specified!");
      return;
    }

  replace_str (&ch->desc->run_buf, buf);
  ch->desc->run_head = ch->desc->run_buf;
  chprintln (ch, "You start running...");
  return;
}
