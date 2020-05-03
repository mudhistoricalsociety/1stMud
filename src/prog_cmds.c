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
*  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
*  Written and adapted to ROM 2.4 by                                      *
*          Markku Nylander (markku.nylander@uta.fi)                       *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "prog_cmds.h"
#include "interp.h"
#include "olc.h"
#include "tables.h"

Proto (RoomIndex * find_location, (CharData *, const char *));


const struct mob_cmd_type mob_cmd_table[] = {
  {"asound", do_mpasound},
  {"gecho", do_mpgecho},
  {"zecho", do_mpzecho},
  {"kill", do_mpkill},
  {"assist", do_mpassist},
  {"junk", do_mpjunk},
  {"echo", do_mpecho},
  {"echoaround", do_mpechoaround},
  {"echoat", do_mpechoat},
  {"mload", do_mpmload},
  {"oload", do_mpoload},
  {"purge", do_mppurge},
  {"goto", do_mpgoto},
  {"at", do_mpat},
  {"transfer", do_mptransfer},
  {"gtransfer", do_mpgtransfer},
  {"otransfer", do_mpotransfer},
  {"force", do_mpforce},
  {"gforce", do_mpgforce},
  {"vforce", do_mpvforce},
  {"cast", do_mpcast},
  {"damage", do_mpdamage},
  {"remember", do_mpremember},
  {"forget", do_mpforget},
  {"delay", do_mpdelay},
  {"cancel", do_mpcancel},
  {"call", do_mpcall},
  {"flee", do_mpflee},
  {"remove", do_mpremove},
  {"peace", do_mppeace},
  {NULL, NULL}
};


const struct obj_cmd_type obj_cmd_table[] = {
  {"gecho", do_opgecho},
  {"zecho", do_opzecho},
  {"echo", do_opecho},
  {"echoaround", do_opechoaround},
  {"echoat", do_opechoat},
  {"mload", do_opmload},
  {"oload", do_opoload},
  {"purge", do_oppurge},
  {"goto", do_opgoto},
  {"transfer", do_optransfer},
  {"gtransfer", do_opgtransfer},
  {"otransfer", do_opotransfer},
  {"force", do_opforce},
  {"gforce", do_opgforce},
  {"vforce", do_opvforce},
  {"damage", do_opdamage},
  {"remember", do_opremember},
  {"forget", do_opforget},
  {"delay", do_opdelay},
  {"cancel", do_opcancel},
  {"call", do_opcall},
  {"remove", do_opremove},
  {"attrib", do_opattrib},
  {"peace", do_oppeace},
  {NULL, NULL}
};


const struct room_cmd_type room_cmd_table[] = {
  {"asound", do_rpasound},
  {"gecho", do_rpgecho},
  {"zecho", do_rpzecho},
  {"echo", do_rpecho},
  {"echoaround", do_rpechoaround},
  {"echoat", do_rpechoat},
  {"mload", do_rpmload},
  {"oload", do_rpoload},
  {"purge", do_rppurge},
  {"transfer", do_rptransfer},
  {"gtransfer", do_rpgtransfer},
  {"otransfer", do_rpotransfer},
  {"force", do_rpforce},
  {"gforce", do_rpgforce},
  {"vforce", do_rpvforce},
  {"damage", do_rpdamage},
  {"remember", do_rpremember},
  {"forget", do_rpforget},
  {"delay", do_rpdelay},
  {"cancel", do_rpcancel},
  {"call", do_rpcall},
  {"remove", do_rpremove},
  {"peace", do_rppeace},
  {NULL, NULL}
};

char *
prog_type_to_name (flag_t type)
{
  switch (type)
    {
    case TRIG_ACT:
      return "ACT";
    case TRIG_SPEECH:
      return "SPEECH";
    case TRIG_RANDOM:
      return "RANDOM";
    case TRIG_FIGHT:
      return "FIGHT";
    case TRIG_HPCNT:
      return "HPCNT";
    case TRIG_DEATH:
      return "DEATH";
    case TRIG_ENTRY:
      return "ENTRY";
    case TRIG_GREET:
      return "GREET";
    case TRIG_GRALL:
      return "GRALL";
    case TRIG_GIVE:
      return "GIVE";
    case TRIG_BRIBE:
      return "BRIBE";
    case TRIG_KILL:
      return "KILL";
    case TRIG_DELAY:
      return "DELAY";
    case TRIG_SURR:
      return "SURRENDER";
    case TRIG_EXIT:
      return "EXIT";
    case TRIG_EXALL:
      return "EXALL";
    case TRIG_GET:
      return "GET";
    case TRIG_DROP:
      return "DROP";
    case TRIG_SIT:
      return "SIT";
    default:
      return "ERROR";
    }
}


Do_Fun (do_mob)
{

  if (ch->desc != NULL && get_trust (ch) < MAX_LEVEL)
    return;
  mob_interpret (ch, argument);
}


void
mob_interpret (CharData * ch, const char *argument)
{
  char command[MIL];
  int cmd;

  argument = one_argument (argument, command);


  for (cmd = 0; !NullStr (mob_cmd_table[cmd].name); cmd++)
    {
      if (command[0] == mob_cmd_table[cmd].name[0]
	  && !str_prefix (command, mob_cmd_table[cmd].name))
	{
	  (*mob_cmd_table[cmd].do_fun) (mob_cmd_table[cmd].name, ch,
					argument);
	  tail_chain ();
	  return;
	}
    }
  bugf ("invalid cmd from mob %ld: '%s'",
	IsNPC (ch) ? ch->pIndexData->vnum : 0, command);
}



Do_Fun (do_mpgecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING)
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Mob echo> ");
	  chprintln (d->character, argument);
	}
    }
}


Do_Fun (do_mpzecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (ch->in_room == NULL)
    return;

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room != NULL
	  && d->character->in_room->area == ch->in_room->area)
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Mob echo> ");
	  chprintln (d->character, argument);
	}
    }
}


Do_Fun (do_mpasound)
{
  RoomIndex *was_in_room;
  int door;

  if (NullStr (argument))
    return;

  was_in_room = ch->in_room;
  for (door = 0; door < MAX_DIR; door++)
    {
      ExitData *pexit;

      if ((pexit = was_in_room->exit[door]) != NULL
	  && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
	{
	  ch->in_room = pexit->u1.to_room;
	  MOBtrigger = false;
	  act (argument, ch, NULL, NULL, TO_ROOM);
	  MOBtrigger = true;
	}
    }
  ch->in_room = was_in_room;
  return;

}


Do_Fun (do_mpkill)
{
  char arg[MIL];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  if (victim == ch || IsNPC (victim) || ch->position == POS_FIGHTING)
    return;

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      bugf ("Charmed mob attacking master from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}


Do_Fun (do_mpassist)
{
  char arg[MIL];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  if (victim == ch || ch->fighting != NULL || victim->fighting == NULL)
    return;

  multi_hit (ch, victim->fighting, TYPE_UNDEFINED);
  return;
}



Do_Fun (do_mpjunk)
{
  char arg[MIL];
  ObjData *obj;
  ObjData *obj_next;

  one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if (str_cmp (arg, "all") && str_prefix ("all.", arg))
    {
      if ((obj = get_obj_wear (ch, arg, true)) != NULL)
	{
	  unequip_char (ch, obj);
	  extract_obj (obj);
	  return;
	}
      if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
	return;
      extract_obj (obj);
    }
  else
    for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
      {
	obj_next = obj->next_content;
	if (arg[3] == '\0' || is_name (&arg[4], obj->name))
	  {
	    if (obj->wear_loc != WEAR_NONE)
	      unequip_char (ch, obj);
	    extract_obj (obj);
	  }
      }

  return;

}



Do_Fun (do_mpechoaround)
{
  char arg[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  act (argument, ch, NULL, victim, TO_NOTVICT);
}


Do_Fun (do_mpechoat)
{
  char arg[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    return;

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  act (argument, ch, NULL, victim, TO_VICT);
}


Do_Fun (do_mpecho)
{
  if (NullStr (argument))
    return;
  act (argument, ch, NULL, NULL, TO_ROOM);
}


Do_Fun (do_mpmload)
{
  char arg[MIL];
  CharIndex *pMobIndex;
  CharData *victim;
  vnum_t vnum;

  one_argument (argument, arg);

  if (ch->in_room == NULL || NullStr (arg) || !is_number (arg))
    return;

  vnum = atov (arg);
  if ((pMobIndex = get_char_index (vnum)) == NULL)
    {
      bugf ("bad mob index (%ld) from mob %ld", vnum,
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  victim = create_mobile (pMobIndex);
  char_to_room (victim, ch->in_room);
  return;
}


Do_Fun (do_mpoload)
{
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  ObjIndex *pObjIndex;
  ObjData *obj;
  int level;
  bool fToroom = false, fWear = false;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  one_argument (argument, arg3);

  if (NullStr (arg1) || !is_number (arg1))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (NullStr (arg2))
    {
      level = get_trust (ch);
    }
  else
    {

      if (!is_number (arg2))
	{
	  bugf ("Bad syntax from vnum %ld.",
		IsNPC (ch) ? ch->pIndexData->vnum : 0);
	  return;
	}
      level = atoi (arg2);
      if (level < 0 || level > get_trust (ch))
	{
	  bugf ("Bad level from vnum %ld.",
		IsNPC (ch) ? ch->pIndexData->vnum : 0);
	  return;
	}
    }


  if (arg3[0] == 'R' || arg3[0] == 'r')
    fToroom = true;
  else if (arg3[0] == 'W' || arg3[0] == 'w')
    fWear = true;

  if ((pObjIndex = get_obj_index (atov (arg1))) == NULL)
    {
      bugf ("Bad vnum arg from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  obj = create_object (pObjIndex, level);
  if ((fWear || !fToroom) && CanWear (obj, ITEM_TAKE))
    {
      obj_to_char (obj, ch);
      if (fWear)
	wear_obj (ch, obj, true);
    }
  else
    {
      obj_to_room (obj, ch->in_room);
    }

  return;
}


Do_Fun (do_mppurge)
{
  char arg[MIL];
  CharData *victim;
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {

      CharData *vnext;
      ObjData *obj_next;

      for (victim = ch->in_room->person_first; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IsNPC (victim) && victim != ch
	      && (IsNPC (victim) && !IsSet (victim->act, ACT_NOPURGE)))
	    extract_char (victim, true);
	}

      for (obj = ch->in_room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (!IsObjStat (obj, ITEM_NOPURGE))
	    extract_obj (obj);
	}

      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      if ((obj = get_obj_here (ch, NULL, arg)))
	{
	  extract_obj (obj);
	}
      else
	{
	  bugf ("Bad argument from vnum %ld.",
		IsNPC (ch) ? ch->pIndexData->vnum : 0);
	}
      return;
    }

  if (!IsNPC (victim))
    {
      bugf ("Purging a PC from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  extract_char (victim, true);
  return;
}


Do_Fun (do_mpgoto)
{
  char arg[MIL];
  RoomIndex *location;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("No argument from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if ((location = find_location (ch, arg)) == NULL)
    {
      bugf ("No such location from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (ch->fighting != NULL)
    stop_fighting (ch, true);

  char_from_room (ch);
  char_to_room (ch, location);

  return;
}


Do_Fun (do_mpat)
{
  char arg[MIL];
  RoomIndex *location;
  RoomIndex *original;
  CharData *wch;
  ObjData *on, *obj;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad argument from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if ((location = find_location (ch, arg)) == NULL)
    {
      bugf ("No such location from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  original = ch->in_room;
  on = ch->on;
  char_from_room (ch);
  char_to_room (ch, location);
  interpret (ch, argument);


  for (wch = char_first; wch != NULL; wch = wch->next)
    {
      if (wch == ch)
	{
	  char_from_room (ch);
	  char_to_room (ch, original);
	  for (obj = original->content_first; obj; obj = obj->next_content)
	    {
	      if (obj == on)
		{
		  ch->on = on;
		  break;
		}
	    }
	  break;
	}
    }
  return;
}


Do_Fun (do_mptransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  RoomIndex *location;
  CharData *victim;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (!str_cmp (arg1, "all"))
    {
      CharData *victim_next;

      for (victim = ch->in_room->person_first; victim != NULL;
	   victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  if (!IsNPC (victim))
	    {
	      sprintf (buf, "%s %s", victim->name, arg2);
	      do_function (ch, &do_mptransfer, buf);
	    }
	}
      return;
    }


  if (NullStr (arg2))
    {
      location = ch->in_room;
    }
  else
    {
      if ((location = find_location (ch, arg2)) == NULL)
	{
	  bugf ("No such location from vnum %ld.",
		IsNPC (ch) ? ch->pIndexData->vnum : 0);
	  return;
	}

      if (room_is_private (location))
	return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    return;

  if (victim->in_room == NULL)
    return;

  if (victim->fighting != NULL)
    stop_fighting (victim, true);
  char_from_room (victim);
  char_to_room (victim, location);
  do_function (victim, &do_look, "auto");
  if (!IsNPC (victim))
    {
      p_greet_trigger (victim, PRG_MPROG);
      p_greet_trigger (victim, PRG_OPROG);
      p_greet_trigger (victim, PRG_RPROG);
    }
  return;
}


Do_Fun (do_mpgtransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  CharData *who, *victim, *victim_next;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if ((who = get_char_room (ch, NULL, arg1)) == NULL)
    return;

  for (victim = ch->in_room->person_first; victim; victim = victim_next)
    {
      victim_next = victim->next_in_room;
      if (is_same_group (who, victim))
	{
	  sprintf (buf, "%s %s", victim->name, arg2);
	  do_function (ch, &do_mptransfer, buf);
	}
    }
  return;
}


Do_Fun (do_mpforce)
{
  char arg[MIL];

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      CharData *vch;
      CharData *vch_next;

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (vch->in_room == ch->in_room
	      && get_trust (vch) < get_trust (ch) && can_see (ch, vch))
	    {
	      interpret (vch, argument);
	    }
	}
    }
  else
    {
      CharData *victim;

      if ((victim = get_char_room (ch, NULL, arg)) == NULL)
	return;

      if (victim == ch)
	return;

      interpret (victim, argument);
    }

  return;
}


Do_Fun (do_mpgforce)
{
  char arg[MIL];
  CharData *victim, *vch, *vch_next;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  if (victim == ch)
    return;

  for (vch = victim->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_same_group (victim, vch))
	{
	  interpret (vch, argument);
	}
    }
  return;
}


Do_Fun (do_mpvforce)
{
  CharData *victim, *victim_next;
  char arg[MIL];
  vnum_t vnum;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if (!is_number (arg))
    {
      bugf ("Non-number argument vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  vnum = atov (arg);

  for (victim = char_first; victim; victim = victim_next)
    {
      victim_next = victim->next;
      if (IsNPC (victim) && victim->pIndexData->vnum == vnum
	  && ch != victim && victim->fighting == NULL)
	interpret (victim, argument);
    }
  return;
}



Do_Fun (do_mpcast)
{
  CharData *vch;
  ObjData *obj;
  void *victim = NULL;
  char spell[MIL], target[MIL];
  int sn;

  argument = one_argument (argument, spell);
  one_argument (argument, target);

  if (NullStr (spell))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }

  if ((sn = skill_lookup (spell)) == -1)
    {
      bugf ("No such spell from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  vch = get_char_room (ch, NULL, target);
  obj = get_obj_here (ch, NULL, target);
  switch (skill_table[sn].target)
    {
    default:
      return;
    case TAR_IGNORE:
      break;
    case TAR_CHAR_OFFENSIVE:
      if (vch == NULL || vch == ch)
	return;
      victim = (void *) vch;
      break;
    case TAR_CHAR_DEFENSIVE:
      victim = vch == NULL ? (void *) ch : (void *) vch;
      break;
    case TAR_CHAR_SELF:
      victim = (void *) ch;
      break;
    case TAR_OBJ_CHAR_DEF:
    case TAR_OBJ_CHAR_OFF:
    case TAR_OBJ_INV:
      if (obj == NULL)
	return;
      victim = (void *) obj;
    }
  (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim,
				skill_table[sn].target);
  return;
}


Do_Fun (do_mpdamage)
{
  CharData *victim = NULL, *victim_next;
  char target[MIL], min[MIL], max[MIL];
  int low, high;
  bool fAll = false, fKill = false;

  argument = one_argument (argument, target);
  argument = one_argument (argument, min);
  argument = one_argument (argument, max);

  if (NullStr (target))
    {
      bugf ("Bad syntax from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  if (!str_cmp (target, "all"))
    fAll = true;
  else if ((victim = get_char_room (ch, NULL, target)) == NULL)
    return;

  if (is_number (min))
    low = atoi (min);
  else
    {
      bugf ("Bad damage min vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  if (is_number (max))
    high = atoi (max);
  else
    {
      bugf ("Bad damage max vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  one_argument (argument, target);



  if (!NullStr (target))
    fKill = true;
  if (fAll)
    {
      for (victim = ch->in_room->person_first; victim; victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  if (victim != ch)
	    damage (victim, victim,
		    fKill ? number_range (low, high) : Min (victim->hit,
							    number_range
							    (low, high)),
		    TYPE_UNDEFINED, DAM_NONE, false);
	}
    }
  else
    damage (victim, victim,
	    fKill ? number_range (low, high) : Min (victim->hit,
						    number_range (low,
								  high)),
	    TYPE_UNDEFINED, DAM_NONE, false);
  return;
}


Do_Fun (do_mpremember)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!NullStr (arg))
    ch->mprog_target = get_char_world (ch, arg);
  else
    bugf ("missing argument from vnum %ld.",
	  IsNPC (ch) ? ch->pIndexData->vnum : 0);
}


Do_Fun (do_mpforget)
{
  ch->mprog_target = NULL;
}


Do_Fun (do_mpdelay)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!is_number (arg))
    {
      bugf ("invalid arg from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  ch->mprog_delay = atoi (arg);
}


Do_Fun (do_mpcancel)
{
  ch->mprog_delay = -1;
}


Do_Fun (do_mpcall)
{
  char arg[MIL];
  CharData *vch;
  ObjData *obj1, *obj2;
  ProgCode *prg;
  ProgList proglist_item;

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("missing arguments from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  if ((prg = get_prog_index (atov (arg), PRG_MPROG)) == NULL)
    {
      bugf ("invalid prog from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  vch = NULL;
  obj1 = obj2 = NULL;
  proglist_item.prog = prg;
  proglist_item.next = NULL;
  proglist_item.trig_phrase = "";
  proglist_item.trig_type = 0;
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    vch = get_char_room (ch, NULL, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj1 = get_obj_here (ch, NULL, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj2 = get_obj_here (ch, NULL, arg);
  program_flow (&proglist_item, ch, NULL, NULL, vch, (void *) obj1,
		(void *) obj2);
}


Do_Fun (do_mpflee)
{
  RoomIndex *was_in;
  ExitData *pexit;
  int door, attempt;

  if (ch->fighting != NULL)
    return;

  if ((was_in = ch->in_room) == NULL)
    return;

  for (attempt = 0; attempt < MAX_DIR; attempt++)
    {
      door = number_door ();
      if ((pexit = was_in->exit[door]) == 0 || pexit->u1.to_room == NULL
	  || IsSet (pexit->exit_info, EX_CLOSED) || (IsNPC (ch)
						     &&
						     IsSet
						     (pexit->u1.
						      to_room->room_flags,
						      ROOM_NO_MOB)))
	continue;

      move_char (ch, door, true);
      if (ch->in_room != was_in)
	return;
    }
}


Do_Fun (do_mpotransfer)
{
  ObjData *obj;
  RoomIndex *location;
  char arg[MIL];
  char buf[MIL];

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("Missing argument from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  one_argument (argument, buf);
  if ((location = find_location (ch, buf)) == NULL)
    {
      bugf ("No such location from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  if ((obj = get_obj_here (ch, NULL, arg)) == NULL)
    return;
  if (obj->carried_by == NULL)
    obj_from_room (obj);
  else
    {
      if (obj->wear_loc != WEAR_NONE)
	unequip_char (ch, obj);
      obj_from_char (obj);
    }
  obj_to_room (obj, location);
}


Do_Fun (do_mpremove)
{
  CharData *victim;
  ObjData *obj, *obj_next;
  vnum_t vnum = 0;
  bool fAll = false;
  char arg[MIL];

  argument = one_argument (argument, arg);
  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    return;

  one_argument (argument, arg);
  if (!str_cmp (arg, "all"))
    fAll = true;
  else if (!is_number (arg))
    {
      bugf ("Invalid object from vnum %ld.",
	    IsNPC (ch) ? ch->pIndexData->vnum : 0);
      return;
    }
  else
    vnum = atov (arg);

  for (obj = victim->carrying_first; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (fAll || obj->pIndexData->vnum == vnum)
	{
	  unequip_char (victim, obj);
	  obj_from_char (obj);
	  extract_obj (obj);
	}
    }
}

Do_Fun (do_mppeace)
{
  CharData *rch;

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting != NULL)
	stop_fighting (rch, true);
      if (IsNPC (rch) && IsSet (rch->act, ACT_AGGRESSIVE))
	RemBit (rch->act, ACT_AGGRESSIVE);
    }
}


Obj_Fun (do_obj)
{

  if (obj->level < MAX_LEVEL)
    return;
  obj_interpret (obj, argument);
}


Obj_Fun (obj_interpret)
{
  char command[MIL];
  int cmd;

  argument = one_argument (argument, command);


  for (cmd = 0; !NullStr (obj_cmd_table[cmd].name); cmd++)
    {
      if (command[0] == obj_cmd_table[cmd].name[0]
	  && !str_prefix (command, obj_cmd_table[cmd].name))
	{
	  (*obj_cmd_table[cmd].obj_fun) (obj, argument);
	  tail_chain ();
	  return;
	}
    }
  bugf ("invalid cmd from obj %ld: '%s'", obj->pIndexData->vnum, command);
}

Obj_Fun (do_opgecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld", obj->pIndexData->vnum);
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING)
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Obj echo> ");
	  chprintln (d->character, argument);
	}
    }
}

Obj_Fun (do_opzecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld", obj->pIndexData->vnum);
      return;
    }

  if (obj->in_room == NULL
      && (obj->carried_by == NULL || obj->carried_by->in_room == NULL))
    return;

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room != NULL
	  &&
	  ((obj->in_room
	    && d->character->in_room->area == obj->in_room->area)
	   || (obj->carried_by
	       && d->character->in_room->area ==
	       obj->carried_by->in_room->area)))
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Obj echo> ");
	  chprintln (d->character, argument);
	}
    }
}

Obj_Fun (do_opechoaround)
{
  char arg[MIL];
  CharData *victim, *vch;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if ((victim =
       get_char_room (NULL,
		      obj->in_room ? obj->in_room : obj->carried_by->in_room,
		      arg)) == NULL)
    return;

  if (obj->in_room && obj->in_room->person_first)
    vch = obj->in_room->person_first;
  else if (obj->carried_by && obj->carried_by->in_room)
    vch = obj->carried_by->in_room->person_first;
  else
    vch = NULL;

  for (; vch; vch = vch->next_in_room)
    {
      if (vch == victim)
	continue;
      chprint (vch, argument);
    }
}

Obj_Fun (do_opechoat)
{
  char arg[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    return;

  if ((victim =
       get_char_room (NULL,
		      obj->in_room ? obj->in_room : obj->carried_by->in_room,
		      arg)) == NULL)
    return;

  act (argument,
       obj->carried_by ? obj->carried_by : obj->in_room->person_first,
       obj, victim, TO_VICT);
}

Obj_Fun (do_opecho)
{
  if (NullStr (argument))
    return;

  if (!obj->carried_by && !obj->in_room->person_first)
    return;

  act (argument,
       obj->carried_by ? obj->carried_by : obj->in_room->person_first,
       NULL, NULL, TO_ROOM);
  act (argument,
       obj->carried_by ? obj->carried_by : obj->in_room->person_first,
       NULL, NULL, TO_CHAR);
}

Obj_Fun (do_opmload)
{
  char arg[MIL];
  CharIndex *pMobIndex;
  CharData *victim;
  vnum_t vnum;

  one_argument (argument, arg);

  if ((obj->in_room == NULL
       && (obj->carried_by == NULL || obj->carried_by->in_room == NULL))
      || NullStr (arg) || !is_number (arg))
    return;

  vnum = atov (arg);
  if ((pMobIndex = get_char_index (vnum)) == NULL)
    {
      bugf ("bad mob index (%ld) from obj %ld", vnum, obj->pIndexData->vnum);
      return;
    }
  victim = create_mobile (pMobIndex);
  char_to_room (victim,
		obj->in_room ? obj->in_room : obj->carried_by->in_room);
  return;
}

Obj_Fun (do_opoload)
{
  char arg1[MIL];
  char arg2[MIL];
  ObjIndex *pObjIndex;
  ObjData *nobj;
  int level;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || !is_number (arg1))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (NullStr (arg2))
    {
      level = obj->level;
    }
  else
    {

      if (!is_number (arg2))
	{
	  bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
	  return;
	}
      level = atoi (arg2);
      if (level < 0 || level > obj->level)
	{
	  bugf ("Bad level from vnum %ld.", obj->pIndexData->vnum);
	  return;
	}
    }

  if ((pObjIndex = get_obj_index (atov (arg1))) == NULL)
    {
      bugf ("Bad vnum arg from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  nobj = create_object (pObjIndex, level);
  obj_to_room (nobj, obj->in_room ? obj->in_room : obj->carried_by->in_room);

  return;
}

Obj_Fun (do_oppurge)
{
  char arg[MIL];
  CharData *victim;
  ObjData *vobj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {

      CharData *vnext;
      ObjData *obj_next;

      if (obj->in_room && obj->in_room->person_first)
	victim = obj->in_room->person_first;
      else if (obj->carried_by && obj->carried_by->in_room)
	victim = obj->carried_by->in_room->person_first;
      else
	victim = NULL;

      for (; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IsNPC (victim) && !IsSet (victim->act, ACT_NOPURGE))
	    extract_char (victim, true);
	}

      if (obj->in_room)
	vobj = obj->in_room->content_first;
      else
	vobj = obj->carried_by->in_room->content_first;

      for (; vobj != NULL; vobj = obj_next)
	{
	  obj_next = vobj->next_content;
	  if (!IsObjStat (vobj, ITEM_NOPURGE) && vobj != obj)
	    extract_obj (vobj);
	}

      return;
    }

  if ((victim =
       get_char_room (NULL,
		      obj->in_room ? obj->in_room : obj->carried_by->in_room,
		      arg)) == NULL)
    {
      if ((vobj =
	   get_obj_here (NULL,
			 obj->in_room ? obj->in_room : obj->
			 carried_by->in_room, arg)))
	{
	  extract_obj (vobj);
	}
      else if (obj->carried_by
	       && (vobj = get_obj_carry (obj->carried_by, arg, NULL)) != NULL)
	{
	  extract_obj (vobj);
	}
      else if (obj->carried_by
	       && (vobj = get_obj_wear (obj->carried_by, arg, false)) != NULL)
	{
	  unequip_char (vobj->carried_by, vobj);
	  extract_obj (vobj);
	}
      else
	{
	  bugf ("Bad argument from vnum %ld.", obj->pIndexData->vnum);
	}
      return;
    }

  if (!IsNPC (victim))
    {
      bugf ("Purging a PC from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  extract_char (victim, true);
  return;
}

Obj_Fun (do_opgoto)
{
  char arg[MIL];
  RoomIndex *location;
  CharData *victim;
  ObjData *dobj;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("No argument from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (is_number (arg))
    location = get_room_index (atov (arg));
  else if ((victim = get_char_world (NULL, arg)) != NULL)
    location = victim->in_room;
  else if ((dobj = get_obj_world (NULL, arg)) != NULL)
    location = dobj->in_room;
  else
    {
      bugf ("No such location from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (obj->in_room != NULL)
    obj_from_room (obj);
  else if (obj->carried_by != NULL)
    obj_from_char (obj);
  obj_to_room (obj, location);

  return;
}

Obj_Fun (do_optransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  RoomIndex *location;
  CharData *victim;
  ObjData *dobj;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg1, "all"))
    {
      CharData *victim_next;

      if (obj->in_room && obj->in_room->person_first)
	victim = obj->in_room->person_first;
      else if (obj->carried_by)
	victim = obj->carried_by->in_room->person_first;
      else
	victim = NULL;
      for (; victim != NULL; victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  if (!IsNPC (victim))
	    {
	      sprintf (buf, "%s %s", victim->name, arg2);
	      do_optransfer (obj, buf);
	    }
	}
      return;
    }

  if (NullStr (arg2))
    {
      location = obj->in_room ? obj->in_room : obj->carried_by->in_room;
    }
  else
    {
      if (is_number (arg2))
	location = get_room_index (atov (arg2));
      else if ((victim = get_char_world (NULL, arg2)) != NULL)
	location = victim->in_room;
      else if ((dobj = get_obj_world (NULL, arg2)) != NULL)
	location = dobj->in_room;
      else
	{
	  bugf ("No such location from vnum %ld.", obj->pIndexData->vnum);
	  return;
	}

      if (room_is_private (location))
	return;
    }

  if ((victim = get_char_world (NULL, arg1)) == NULL)
    return;

  if (victim->in_room == NULL)
    return;

  if (victim->fighting != NULL)
    stop_fighting (victim, true);
  char_from_room (victim);
  char_to_room (victim, location);
  do_function (victim, &do_look, "auto");
  if (!IsNPC (victim))
    {
      p_greet_trigger (victim, PRG_MPROG);
      p_greet_trigger (victim, PRG_OPROG);
      p_greet_trigger (victim, PRG_RPROG);
    }

  return;
}

Obj_Fun (do_opgtransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  CharData *who, *victim, *victim_next;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if ((who =
       get_char_room (NULL,
		      (obj->in_room) ? obj->in_room : obj->
		      carried_by->in_room, arg1)) == NULL)
    return;

  if (obj->in_room && obj->in_room->person_first)
    victim = obj->in_room->person_first;
  else if (obj->carried_by && obj->carried_by->in_room)
    victim = obj->carried_by->in_room->person_first;
  else
    victim = NULL;

  for (; victim; victim = victim_next)
    {
      victim_next = victim->next_in_room;
      if (is_same_group (who, victim))
	{
	  sprintf (buf, "%s %s", victim->name, arg2);
	  do_optransfer (obj, buf);
	}
    }
  return;
}

Obj_Fun (do_opforce)
{
  char arg[MIL];

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (!obj->in_room && !obj->carried_by)
    return;
  if (obj->in_room && !obj->in_room->person_first)
    return;
  if (obj->carried_by && !obj->carried_by->in_room)
    return;

  if (!str_cmp (arg, "all"))
    {
      CharData *vch;
      CharData *vch_next;

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (((obj->in_room && vch->in_room == obj->in_room)
	       || (obj->carried_by
		   && vch->in_room == obj->carried_by->in_room))
	      && get_trust (vch) < obj->level)
	    {
	      interpret (vch, argument);
	    }
	}
    }
  else
    {
      CharData *victim;

      if ((victim =
	   get_char_room (NULL,
			  (obj->in_room) ? obj->in_room : obj->
			  carried_by->in_room, arg)) == NULL)
	return;

      interpret (victim, argument);
    }

  return;
}

Obj_Fun (do_opgforce)
{
  char arg[MIL];
  CharData *victim, *vch, *vch_next;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if ((victim =
       get_char_room (NULL,
		      obj->in_room ? obj->in_room : obj->carried_by->in_room,
		      arg)) == NULL)
    return;

  for (vch = victim->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_same_group (victim, vch))
	{
	  interpret (vch, argument);
	}
    }
  return;
}

Obj_Fun (do_opvforce)
{
  CharData *victim, *victim_next;
  char arg[MIL];
  vnum_t vnum;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if (!is_number (arg))
    {
      bugf ("Non-number argument vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  vnum = atov (arg);

  for (victim = char_first; victim; victim = victim_next)
    {
      victim_next = victim->next;
      if (IsNPC (victim) && victim->pIndexData->vnum == vnum
	  && victim->fighting == NULL)
	interpret (victim, argument);
    }
  return;
}

Obj_Fun (do_opdamage)
{
  CharData *victim = NULL, *victim_next;
  char target[MIL], min[MIL], max[MIL];
  int low, high;
  bool fAll = false, fKill = false;

  argument = one_argument (argument, target);
  argument = one_argument (argument, min);
  argument = one_argument (argument, max);

  if (NullStr (target))
    {
      bugf ("Bad syntax from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  if (!str_cmp (target, "all"))
    fAll = true;
  else if ((victim =
	    get_char_room (NULL,
			   obj->in_room ? obj->in_room : obj->
			   carried_by->in_room, target)) == NULL)
    return;

  if (is_number (min))
    low = atoi (min);
  else
    {
      bugf ("Bad damage min vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  if (is_number (max))
    high = atoi (max);
  else
    {
      bugf ("Bad damage max vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  one_argument (argument, target);

  if (!NullStr (target))
    fKill = true;
  if (fAll)
    {
      if (obj->in_room && obj->in_room->person_first)
	victim = obj->in_room->person_first;
      else if (obj->carried_by)
	victim = obj->carried_by->in_room->person_first;
      else
	victim = NULL;

      for (; victim; victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  if (obj->carried_by && victim != obj->carried_by)
	    damage (victim, victim,
		    fKill ? number_range (low, high) : Min (victim->hit,
							    number_range
							    (low, high)),
		    TYPE_UNDEFINED, DAM_NONE, false);
	}
    }
  else
    damage (victim, victim,
	    fKill ? number_range (low, high) : Min (victim->hit,
						    number_range (low,
								  high)),
	    TYPE_UNDEFINED, DAM_NONE, false);
  return;
}

Obj_Fun (do_opremember)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!NullStr (arg))
    obj->oprog_target = get_char_world (NULL, arg);
  else
    bugf ("missing argument from vnum %ld.", obj->pIndexData->vnum);
}

Obj_Fun (do_opforget)
{
  obj->oprog_target = NULL;
}

Obj_Fun (do_opdelay)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!is_number (arg))
    {
      bugf ("invalid arg from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  obj->oprog_delay = atoi (arg);
}

Obj_Fun (do_opcancel)
{
  obj->oprog_delay = -1;
}

Obj_Fun (do_opcall)
{
  char arg[MIL];
  CharData *vch;
  ObjData *obj1, *obj2;
  ProgCode *prg;
  ProgList proglist_item;

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("missing arguments from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  if ((prg = get_prog_index (atov (arg), PRG_OPROG)) == NULL)
    {
      bugf ("invalid prog from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  vch = NULL;
  obj1 = obj2 = NULL;
  proglist_item.prog = prg;
  proglist_item.next = NULL;
  proglist_item.trig_phrase = "";
  proglist_item.trig_type = 0;
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    vch =
      get_char_room (NULL,
		     obj->in_room ? obj->in_room : obj->carried_by->
		     in_room, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj1 =
      get_obj_here (NULL,
		    obj->in_room ? obj->in_room : obj->carried_by->
		    in_room, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj2 =
      get_obj_here (NULL,
		    obj->in_room ? obj->in_room : obj->carried_by->
		    in_room, arg);
  program_flow (&proglist_item, NULL, obj, NULL, vch, (void *) obj1,
		(void *) obj2);
}

Obj_Fun (do_opotransfer)
{
  ObjData *obj1, *dobj;
  RoomIndex *location;
  char arg[MIL];
  char buf[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("Missing argument from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  one_argument (argument, buf);
  if (is_number (buf))
    location = get_room_index (atov (buf));
  else if ((victim = get_char_world (NULL, buf)) != NULL)
    location = victim->in_room;
  else if ((dobj = get_obj_world (NULL, arg)) != NULL)
    location = dobj->in_room;
  else
    {
      bugf ("No such location from vnum %ld.", obj->pIndexData->vnum);
      return;
    }

  if ((obj1 =
       get_obj_here (NULL,
		     obj->in_room ? obj->in_room : obj->carried_by->in_room,
		     arg)) == NULL)
    return;
  if (obj1->carried_by == NULL)
    obj_from_room (obj1);
  else
    {
      if (obj1->wear_loc != WEAR_NONE)
	unequip_char (obj1->carried_by, obj1);
      obj_from_char (obj1);
    }
  obj_to_room (obj1, location);
}

Obj_Fun (do_opremove)
{
  CharData *victim;
  ObjData *obj1, *obj_next;
  vnum_t vnum = 0;
  bool fAll = false;
  char arg[MIL];

  argument = one_argument (argument, arg);
  if ((victim =
       get_char_room (NULL,
		      obj->in_room ? obj->in_room : obj->carried_by->in_room,
		      arg)) == NULL)
    return;

  one_argument (argument, arg);
  if (!str_cmp (arg, "all"))
    fAll = true;
  else if (!is_number (arg))
    {
      bugf ("Invalid object from vnum %ld.", obj->pIndexData->vnum);
      return;
    }
  else
    vnum = atov (arg);

  for (obj1 = victim->carrying_first; obj1; obj1 = obj_next)
    {
      obj_next = obj->next_content;
      if (fAll || obj1->pIndexData->vnum == vnum)
	{
	  unequip_char (victim, obj1);
	  obj_from_char (obj1);
	  extract_obj (obj1);
	}
    }
}

Obj_Fun (do_opattrib)
{
  CharData *ch;
  char target[MIL], arg1[MIL], arg2[MIL];
  char arg3[MIL], arg4[MIL], arg5[MIL];
  char arg6[MIL], arg7[MIL];
  char mod, *p;
  int i, level, condition;
  vnum_t value0, value1, value2, value3, value4;

  argument = one_argument (argument, target);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  argument = one_argument (argument, arg4);
  argument = one_argument (argument, arg5);
  argument = one_argument (argument, arg6);
  one_argument (argument, arg7);

  if (!str_cmp (target, "worn"))
    {
      if (!obj->carried_by)
	return;
      else
	ch = obj->carried_by;
    }
  else if ((ch =
	    get_char_room (NULL,
			   obj->in_room ? obj->in_room : obj->
			   carried_by->in_room, target)) == NULL)
    return;

  if (!str_cmp (arg1, "none"))
    level = obj->level;
  else if (!isdigit (arg1[0]))
    {
      p = arg1;
      mod = arg1[0];

      for (i = 1; i; i++)
	{
	  if (arg1[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg1[i];
	  p++;
	}

      if (!is_number (arg1))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  level = ch->level + atoi (arg1);
	  break;
	case '-':
	  level = ch->level - atoi (arg1);
	  break;
	case '*':
	  level = ch->level * atoi (arg1);
	  break;
	case '/':
	  level = ch->level / atoi (arg1);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg1))
    level = atoi (arg1);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg2, "none"))
    condition = obj->condition;
  else if (!isdigit (arg2[0]))
    {
      p = arg2;
      mod = arg2[0];

      for (i = 1; i; i++)
	{
	  if (arg2[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg2[i];
	  p++;
	}

      if (!is_number (arg2))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  condition = ch->level + atoi (arg2);
	  break;
	case '-':
	  condition = ch->level - atoi (arg2);
	  break;
	case '*':
	  condition = ch->level * atoi (arg2);
	  break;
	case '/':
	  condition = ch->level / atoi (arg2);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg2))
    condition = atoi (arg2);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg3, "none"))
    value0 = obj->value[0];
  else if (!isdigit (arg3[0]))
    {
      p = arg3;
      mod = arg3[0];

      for (i = 1; i; i++)
	{
	  if (arg3[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg3[i];
	  p++;
	}

      if (!is_number (arg3))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  value0 = ch->level + atol (arg3);
	  break;
	case '-':
	  value0 = ch->level - atol (arg3);
	  break;
	case '*':
	  value0 = ch->level * atol (arg3);
	  break;
	case '/':
	  value0 = ch->level / atol (arg3);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg3))
    value0 = atol (arg3);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg4, "none"))
    value1 = obj->value[1];
  else if (!isdigit (arg4[0]))
    {
      p = arg4;
      mod = arg4[0];

      for (i = 1; i; i++)
	{
	  if (arg4[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg4[i];
	  p++;
	}

      if (!is_number (arg4))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  value1 = ch->level + atol (arg4);
	  break;
	case '-':
	  value1 = ch->level - atol (arg4);
	  break;
	case '*':
	  value1 = ch->level * atol (arg4);
	  break;
	case '/':
	  value1 = ch->level / atol (arg4);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg4))
    value1 = atol (arg4);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg5, "none"))
    value2 = obj->value[2];
  else if (!isdigit (arg5[0]))
    {
      p = arg5;
      mod = arg5[0];

      for (i = 1; i; i++)
	{
	  if (arg5[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg5[i];
	  p++;
	}

      if (!is_number (arg5))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  value2 = ch->level + atol (arg5);
	  break;
	case '-':
	  value2 = ch->level - atol (arg5);
	  break;
	case '*':
	  value2 = ch->level * atol (arg5);
	  break;
	case '/':
	  value2 = ch->level / atol (arg5);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg5))
    value2 = atol (arg5);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg6, "none"))
    value3 = obj->value[3];
  else if (!isdigit (arg6[0]))
    {
      p = arg6;
      mod = arg6[0];

      for (i = 1; i; i++)
	{
	  if (arg6[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg6[i];
	  p++;
	}

      if (!is_number (arg6))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  value3 = ch->level + atol (arg6);
	  break;
	case '-':
	  value3 = ch->level - atol (arg6);
	  break;
	case '*':
	  value3 = ch->level * atol (arg6);
	  break;
	case '/':
	  value3 = ch->level / atol (arg6);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg6))
    value3 = atol (arg6);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  if (!str_cmp (arg7, "none"))
    value4 = obj->value[4];
  else if (!isdigit (arg7[0]))
    {
      p = arg7;
      mod = arg7[0];

      for (i = 1; i; i++)
	{
	  if (arg7[i] == '\0')
	    {
	      *p = '\0';
	      break;
	    }

	  *p = arg7[i];
	  p++;
	}

      if (!is_number (arg7))
	{
	  bugf ("received non-number argument from vnum %ld",
		obj->pIndexData->vnum);
	  return;
	}

      switch (mod)
	{
	case '+':
	  value4 = ch->level + atol (arg7);
	  break;
	case '-':
	  value4 = ch->level - atol (arg7);
	  break;
	case '*':
	  value4 = ch->level * atol (arg7);
	  break;
	case '/':
	  value4 = ch->level / atol (arg7);
	  break;
	default:
	  bugf ("invalid modifier from vnum %ld", obj->pIndexData->vnum);
	  return;
	}
    }
  else if (is_number (arg7))
    value4 = atol (arg7);
  else
    {
      bugf ("received non-number argument from vnum %ld",
	    obj->pIndexData->vnum);
      return;
    }

  obj->level = level;
  obj->condition = condition;
  obj->value[0] = value0;
  obj->value[1] = value1;
  obj->value[2] = value2;
  obj->value[3] = value3;
  obj->value[4] = value4;
}

Obj_Fun (do_oppeace)
{
  CharData *rch;
  RoomIndex *room = obj->in_room ? obj->in_room : obj->carried_by->in_room;

  for (rch = room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting != NULL)
	stop_fighting (rch, true);
      if (IsNPC (rch) && IsSet (rch->act, ACT_AGGRESSIVE))
	RemBit (rch->act, ACT_AGGRESSIVE);
    }
}

Room_Fun (do_room)
{
  room_interpret (room, argument);
}

Room_Fun (room_interpret)
{
  char command[MIL];
  int cmd;

  argument = one_argument (argument, command);

  for (cmd = 0; !NullStr (room_cmd_table[cmd].name); cmd++)
    {
      if (command[0] == room_cmd_table[cmd].name[0]
	  && !str_prefix (command, room_cmd_table[cmd].name))
	{
	  (*room_cmd_table[cmd].room_fun) (room, argument);
	  tail_chain ();
	  return;
	}
    }
  bugf ("invalid cmd from room %ld: '%s'", room->vnum, command);
}

Room_Fun (do_rpgecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld", room->vnum);
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING)
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Room echo> ");
	  chprintln (d->character, argument);
	}
    }
}

Room_Fun (do_rpzecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      bugf ("missing argument from vnum %ld", room->vnum);
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room != NULL
	  && d->character->in_room->area == room->area)
	{
	  if (IsImmortal (d->character))
	    chprint (d->character, "Room echo> ");
	  chprintln (d->character, argument);
	}
    }
}

Room_Fun (do_rpasound)
{
  int door;

  if (NullStr (argument))
    return;

  for (door = 0; door < MAX_DIR; door++)
    {
      ExitData *pexit;

      if ((pexit = room->exit[door]) != NULL && pexit->u1.to_room != NULL
	  && pexit->u1.to_room != room
	  && pexit->u1.to_room->person_first != NULL)
	{
	  act (argument, pexit->u1.to_room->person_first, NULL, NULL,
	       TO_ROOM);
	  act (argument, pexit->u1.to_room->person_first, NULL, NULL,
	       TO_CHAR);
	}
    }
  return;

}

Room_Fun (do_rpechoaround)
{
  char arg[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    return;

  if ((victim = get_char_room (NULL, room, arg)) == NULL)
    return;

  act (argument, victim, NULL, victim, TO_NOTVICT);
}

Room_Fun (do_rpechoat)
{
  char arg[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    return;

  if ((victim = get_char_room (NULL, room, arg)) == NULL)
    return;

  act (argument, victim, NULL, NULL, TO_CHAR);
}

Room_Fun (do_rpecho)
{
  if (NullStr (argument))
    return;

  if (!room->person_first)
    return;

  act (argument, room->person_first, NULL, NULL, TO_ROOM);
  act (argument, room->person_first, NULL, NULL, TO_CHAR);
}

Room_Fun (do_rpmload)
{
  char arg[MIL];
  CharIndex *pMobIndex;
  CharData *victim;
  vnum_t vnum;

  one_argument (argument, arg);

  if (NullStr (arg) || !is_number (arg))
    return;

  vnum = atov (arg);
  if ((pMobIndex = get_char_index (vnum)) == NULL)
    {
      bugf ("bad mob index (%ld) from room %ld", vnum, room->vnum);
      return;
    }
  victim = create_mobile (pMobIndex);
  char_to_room (victim, room);
  return;
}

Room_Fun (do_rpoload)
{
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  ObjIndex *pObjIndex;
  ObjData *obj;
  int level;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  one_argument (argument, arg3);

  if (NullStr (arg1) || !is_number (arg1) || NullStr (arg2)
      || !is_number (arg2))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  level = atoi (arg2);
  if (level < 0 || level > LEVEL_IMMORTAL)
    {
      bugf ("Bad level from vnum %ld.", room->vnum);
      return;
    }

  if ((pObjIndex = get_obj_index (atov (arg1))) == NULL)
    {
      bugf ("Bad vnum arg from vnum %ld.", room->vnum);
      return;
    }

  obj = create_object (pObjIndex, level);
  obj_to_room (obj, room);

  return;
}

Room_Fun (do_rppurge)
{
  char arg[MIL];
  CharData *victim;
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {

      CharData *vnext;
      ObjData *obj_next;

      for (victim = room->person_first; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IsNPC (victim) && !IsSet (victim->act, ACT_NOPURGE))
	    extract_char (victim, true);
	}

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (!IsObjStat (obj, ITEM_NOPURGE))
	    extract_obj (obj);
	}

      return;
    }

  if ((victim = get_char_room (NULL, room, arg)) == NULL)
    {
      if ((obj = get_obj_here (NULL, room, arg)))
	{
	  extract_obj (obj);
	}
      else
	{
	  bugf ("Bad argument from vnum %ld.", room->vnum);
	}
      return;
    }

  if (!IsNPC (victim))
    {
      bugf ("Purging a PC from vnum %ld.", room->vnum);
      return;
    }
  extract_char (victim, true);
  return;
}

Room_Fun (do_rptransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  RoomIndex *location;
  CharData *victim;
  ObjData *tobj;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  if (!str_cmp (arg1, "all"))
    {
      CharData *victim_next;

      for (victim = room->person_first; victim != NULL; victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  if (!IsNPC (victim))
	    {
	      sprintf (buf, "%s %s", victim->name, arg2);
	      do_rptransfer (room, buf);
	    }
	}
      return;
    }

  if (NullStr (arg2))
    {
      location = room;
    }
  else
    {
      if (is_number (arg2))
	location = get_room_index (atov (arg2));
      else if ((victim = get_char_world (NULL, arg2)) != NULL)
	location = victim->in_room;
      else if ((tobj = get_obj_world (NULL, arg2)) != NULL)
	location = tobj->in_room;
      else
	{
	  bugf ("No such location from vnum %ld.", room->vnum);
	  return;
	}

      if (room_is_private (location))
	return;
    }

  if ((victim = get_char_world (NULL, arg1)) == NULL)
    return;

  if (victim->in_room == NULL)
    return;

  if (victim->fighting != NULL)
    stop_fighting (victim, true);
  char_from_room (victim);
  char_to_room (victim, location);
  do_function (victim, &do_look, "auto");
  if (!IsNPC (victim))
    {
      p_greet_trigger (victim, PRG_MPROG);
      p_greet_trigger (victim, PRG_OPROG);
      p_greet_trigger (victim, PRG_RPROG);
    }

  return;
}

Room_Fun (do_rpgtransfer)
{
  char arg1[MIL];
  char arg2[MIL];
  char buf[MSL];
  CharData *who, *victim, *victim_next;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  if ((who = get_char_room (NULL, room, arg1)) == NULL)
    return;

  for (victim = room->person_first; victim; victim = victim_next)
    {
      victim_next = victim->next_in_room;
      if (is_same_group (who, victim))
	{
	  sprintf (buf, "%s %s", victim->name, arg2);
	  do_rptransfer (room, buf);
	}
    }
  return;
}

Room_Fun (do_rpforce)
{
  char arg[MIL];

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      CharData *vch;
      CharData *vch_next;

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (vch->in_room == room && !IsImmortal (vch))
	    interpret (vch, argument);
	}
    }
  else
    {
      CharData *victim;

      if ((victim = get_char_room (NULL, room, arg)) == NULL)
	return;

      interpret (victim, argument);
    }

  return;
}

Room_Fun (do_rpgforce)
{
  char arg[MIL];
  CharData *victim, *vch, *vch_next;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  if ((victim = get_char_room (NULL, room, arg)) == NULL)
    return;

  for (vch = victim->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_same_group (victim, vch))
	interpret (vch, argument);
    }
  return;
}

Room_Fun (do_rpvforce)
{
  CharData *victim, *victim_next;
  char arg[MIL];
  vnum_t vnum;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }

  if (!is_number (arg))
    {
      bugf ("Non-number argument vnum %ld.", room->vnum);
      return;
    }

  vnum = atov (arg);

  for (victim = char_first; victim; victim = victim_next)
    {
      victim_next = victim->next;
      if (IsNPC (victim) && victim->pIndexData->vnum == vnum
	  && victim->fighting == NULL)
	interpret (victim, argument);
    }
  return;
}

Room_Fun (do_rpdamage)
{
  CharData *victim = NULL, *victim_next;
  char target[MIL], min[MIL], max[MIL];
  int low, high;
  bool fAll = false, fKill = false;

  argument = one_argument (argument, target);
  argument = one_argument (argument, min);
  argument = one_argument (argument, max);

  if (NullStr (target))
    {
      bugf ("Bad syntax from vnum %ld.", room->vnum);
      return;
    }
  if (!str_cmp (target, "all"))
    fAll = true;
  else if ((victim = get_char_room (NULL, room, target)) == NULL)
    return;

  if (is_number (min))
    low = atoi (min);
  else
    {
      bugf ("Bad damage min vnum %ld.", room->vnum);
      return;
    }
  if (is_number (max))
    high = atoi (max);
  else
    {
      bugf ("Bad damage max vnum %ld.", room->vnum);
      return;
    }
  one_argument (argument, target);

  if (!NullStr (target))
    fKill = true;
  if (fAll)
    {
      for (victim = room->person_first; victim; victim = victim_next)
	{
	  victim_next = victim->next_in_room;
	  damage (victim, victim,
		  fKill ? number_range (low, high) : Min (victim->hit,
							  number_range
							  (low, high)),
		  TYPE_UNDEFINED, DAM_NONE, false);
	}
    }
  else
    damage (victim, victim,
	    fKill ? number_range (low, high) : Min (victim->hit,
						    number_range (low,
								  high)),
	    TYPE_UNDEFINED, DAM_NONE, false);
  return;
}

Room_Fun (do_rpremember)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!NullStr (arg))
    room->rprog_target = get_char_world (NULL, arg);
  else
    bugf ("missing argument from vnum %ld.", room->vnum);
}

Room_Fun (do_rpforget)
{
  room->rprog_target = NULL;
}

Room_Fun (do_rpdelay)
{
  char arg[MIL];

  one_argument (argument, arg);
  if (!is_number (arg))
    {
      bugf ("invalid arg from vnum %ld.", room->vnum);
      return;
    }
  room->rprog_delay = atoi (arg);
}

Room_Fun (do_rpcancel)
{
  room->rprog_delay = -1;
}

Room_Fun (do_rpcall)
{
  char arg[MIL];
  CharData *vch;
  ObjData *obj1, *obj2;
  ProgCode *prg;
  ProgList proglist_item;

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("missing arguments from vnum %ld.", room->vnum);
      return;
    }
  if ((prg = get_prog_index (atov (arg), PRG_RPROG)) == NULL)
    {
      bugf ("invalid prog from vnum %ld.", room->vnum);
      return;
    }
  vch = NULL;
  obj1 = obj2 = NULL;
  proglist_item.prog = prg;
  proglist_item.next = NULL;
  proglist_item.trig_phrase = "";
  proglist_item.trig_type = 0;
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    vch = get_char_room (NULL, room, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj1 = get_obj_here (NULL, room, arg);
  argument = one_argument (argument, arg);
  if (!NullStr (arg))
    obj2 = get_obj_here (NULL, room, arg);
  program_flow (&proglist_item, NULL, NULL, room, vch, (void *) obj1,
		(void *) obj2);
}

Room_Fun (do_rpotransfer)
{
  ObjData *obj, *tobj;
  RoomIndex *location;
  char arg[MIL];
  char buf[MIL];
  CharData *victim;

  argument = one_argument (argument, arg);
  if (NullStr (arg))
    {
      bugf ("Missing argument from vnum %ld.", room->vnum);
      return;
    }
  one_argument (argument, buf);

  if (is_number (buf))
    location = get_room_index (atov (buf));
  else if ((victim = get_char_world (NULL, buf)) != NULL)
    location = victim->in_room;
  else if ((tobj = get_obj_world (NULL, arg)) != NULL)
    location = tobj->in_room;
  else
    {
      bugf ("No such location from vnum %ld.", room->vnum);
      return;
    }

  if ((obj = get_obj_here (NULL, room, arg)) == NULL)
    return;

  if (obj->carried_by == NULL)
    obj_from_room (obj);
  else
    {
      if (obj->wear_loc != WEAR_NONE)
	unequip_char (obj->carried_by, obj);
      obj_from_char (obj);
    }
  obj_to_room (obj, location);
}

Room_Fun (do_rpremove)
{
  CharData *victim;
  ObjData *obj, *obj_next;
  vnum_t vnum = 0;
  bool fAll = false;
  char arg[MIL];

  argument = one_argument (argument, arg);
  if ((victim = get_char_room (NULL, room, arg)) == NULL)
    return;

  one_argument (argument, arg);
  if (!str_cmp (arg, "all"))
    fAll = true;
  else if (!is_number (arg))
    {
      bugf ("Invalid object from vnum %ld.", room->vnum);
      return;
    }
  else
    vnum = atov (arg);

  for (obj = victim->carrying_first; obj; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (fAll || obj->pIndexData->vnum == vnum)
	{
	  unequip_char (victim, obj);
	  obj_from_char (obj);
	  extract_obj (obj);
	}
    }
}

Room_Fun (do_rppeace)
{
  CharData *rch;

  for (rch = room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting != NULL)
	stop_fighting (rch, true);
      if (IsNPC (rch) && IsSet (rch->act, ACT_AGGRESSIVE))
	RemBit (rch->act, ACT_AGGRESSIVE);
    }
}
