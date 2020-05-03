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
* Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
* 4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  *
* code is allowed provided you add a credit line to the effect of: "Quest *
* Code (c) 1996 Ryan Addams" to your logon screen with the rest of the    *
* standard diku/rom credits. If you use this or a modified version of     *
* this code, let me know via email: moongate@moongate.ams.com. Further    *
* updates will be posted to the rom mailing list. If you'd like to get    *
* the latest version of quest.c, please send a request to the above       *
* address. Quest Code v2.03.  Please do not remove this notice from this  *
* file.                                                                   *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/
#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "recycle.h"
#include "vnums.h"
#include "special.h"

Proto (bool quest_complete, (CharData *, CharData *));

struct quest_type
{
  char *name;
  char *descr;
  int vnum;
  int cost;
};


const struct quest_type quest_table[] = {

  {"nohunger", "No Hunger/Thirst (quest buy nohunger)", 0, 3000},
  {"aura", NULL, OBJ_VNUM_QUEST_AURA, 2600},
  {"sword", NULL, OBJ_VNUM_QUEST_SWORD, 2500},
  {"breastplate", NULL, OBJ_VNUM_QUEST_BPLATE, 2500},
  {"boots", NULL, OBJ_VNUM_QUEST_BOOTS, 2500},
  {"gloves", NULL, OBJ_VNUM_QUEST_GLOVES, 2500},
  {"flame", NULL, OBJ_VNUM_QUEST_FLAME, 2500},
  {"helm", NULL, OBJ_VNUM_QUEST_HELM, 2300},
  {"bag", NULL, OBJ_VNUM_QUEST_BAG, 1000},
  {"shield", NULL, OBJ_VNUM_QUEST_SHIELD, 750},
  {"regeneration", NULL, OBJ_VNUM_QUEST_REGEN, 700},
  {"invisibility", NULL, OBJ_VNUM_QUEST_INVIS, 500},
  {"trivia", NULL, OBJ_VNUM_QUEST_TRIVIA, 100},
  {NULL, NULL, 0, 0}
};


bool
chance (int num)
{
  return number_range (1, 100) <= num;
}

int
add_qp (CharData * ch, int qp)
{
  if (!ch || IsNPC (ch))
    return qp;

  if (mud_info.bonus.status == BONUS_QP)
    {
      qp *= mud_info.bonus.mod;
    }

  ch->pcdata->quest.points += qp;
  return qp;
}


int
qobj_lookup (ObjData * obj)
{
  int i;

  if (!obj || !obj->pIndexData)
    return -1;

  for (i = 0; quest_table[i].name != NULL; i++)
    {
      if (obj->pIndexData->vnum == quest_table[i].vnum)
	return i;
    }
  return -1;
}

Lookup_Fun (quest_lookup)
{
  int i;

  if (NullStr (name))
    return -1;

  for (i = 0; quest_table[i].name != NULL; i++)
    {
      if (is_name (name, quest_table[i].name))
	return i;
    }
  return -1;
}

money_t
obj_cost (ObjData * obj)
{
  int i;

  if (!obj || !obj->pIndexData)
    return 0;

  for (i = 0; quest_table[i].name != NULL; i++)
    {
      if (obj->pIndexData->vnum == quest_table[i].vnum)
	return quest_table[i].cost;
    }
  return obj->cost;
}


void
affect_join_obj (ObjData * obj, AffectData * paf)
{
  AffectData *paf_old;
  bool found;

  found = false;
  for (paf_old = obj->affect_first; paf_old != NULL; paf_old = paf_old->next)
    {
      if (paf_old->location == paf->location
	  && paf_old->type == paf->type
	  && paf_old->bitvector == paf->bitvector
	  && paf_old->where == paf->where)
	{
	  paf_old->level = paf->level;
	  paf_old->modifier = paf->modifier;
	  found = true;
	}
    }
  if (!found)
    affect_to_obj (obj, paf);
  return;
}

void
add_apply (ObjData * obj, apply_t loc, int mod, where_t where,
	   int type, int dur, flag_t bit, int level)
{
  AffectData pAf;

  if (obj == NULL)
    return;

  pAf.location = loc;
  pAf.modifier = mod;
  pAf.where = where;
  pAf.type = type;
  pAf.duration = dur;
  pAf.bitvector = bit;
  pAf.level = level;
  affect_join_obj (obj, &pAf);

  return;
}


void
update_questobj (CharData * ch, ObjData * obj)
{
  int bonus, pbonus;
  money_t cost;

  if (obj == NULL)
    {
      bug ("update_questobj: NULL obj");
      return;
    }
  if (ch == NULL)
    {
      bug ("update_questobj: NULL ch");
      return;
    }

  if (!IsObjStat (obj, ITEM_QUEST))
    return;

  bonus = Max (5, ch->level / 10);
  pbonus = Max (5, ch->level / 5);
  cost = obj_cost (obj);

  if (obj->level != ch->level)
    obj->level = ch->level;
  if (obj->condition != -1)
    obj->condition = -1;
  if (obj->cost != cost)
    obj->cost = cost;
  if (!CanWear (obj, ITEM_NO_SAC))
    SetBit (obj->wear_flags, ITEM_NO_SAC);
  if (!IsObjStat (obj, ITEM_BURN_PROOF))
    SetBit (obj->extra_flags, ITEM_BURN_PROOF);


  switch (obj->pIndexData->vnum)
    {
    case OBJ_VNUM_QUEST_BPLATE:
      add_apply (obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
      add_apply (obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
      break;

    case OBJ_VNUM_QUEST_SHIELD:
      add_apply (obj, APPLY_DAMROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
      add_apply (obj, APPLY_HITROLL, pbonus, TO_OBJECT, 0, -1, 0, ch->level);
      break;

    case OBJ_VNUM_QUEST_AURA:
      add_apply (obj, APPLY_HIT, Max (50, ch->level), TO_OBJECT, 0, -1,
		 0, ch->level);
      add_apply (obj, APPLY_MANA, Max (50, ch->level), TO_OBJECT, 0,
		 -1, 0, ch->level);
      add_apply (obj, APPLY_MOVE, Max (50, ch->level), TO_OBJECT, 0,
		 -1, 0, ch->level);
      break;
    }

  switch (obj->item_type)
    {
    case ITEM_CONTAINER:

      obj->weight = -1 * (50 + (ch->level * 15 / 10));
      obj->value[0] = 1000 + (20 * ch->level);
      obj->value[3] = 1000 + (20 * ch->level);
      break;

    case ITEM_WEAPON:

      obj->value[1] = Max (15, ch->level);
      obj->value[2] = ch->level < (MAX_LEVEL / (25 / 10)) ? 4 : 5;
      add_apply (obj, APPLY_DAMROLL, bonus, TO_OBJECT, 0, -1, 0, ch->level);
      add_apply (obj, APPLY_HITROLL, bonus, TO_OBJECT, 0, -1, 0, ch->level);
      break;

    case ITEM_ARMOR:

      obj->value[0] = Max (20, ch->level);
      obj->value[1] = Max (20, ch->level);
      obj->value[2] = Max (20, ch->level);
      obj->value[3] = (5 * Max (20, ch->level)) / 6;
      break;
    case ITEM_STAFF:
      obj->value[0] = Max (40, ch->level / 3);
      break;
    case ITEM_LIGHT:
      obj->value[2] = -1;
      break;
    case ITEM_PORTAL:
      if (!IsSet (obj->value[2], GATE_NOCURSE))
	SetBit (obj->value[2], GATE_NOCURSE);
      if (!IsSet (obj->value[2], GATE_GOWITH))
	SetBit (obj->value[2], GATE_GOWITH);
      break;
    default:
      break;
    }

  return;
}


void
update_all_qobjs (CharData * ch)
{
  ObjData *obj;
  wloc_t iWear;

  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (IsObjStat (obj, ITEM_QUEST))
	{
	  update_questobj (ch, obj);
	  if ((iWear = obj->wear_loc) != WEAR_NONE)
	    {
	      unequip_char (ch, obj);
	      equip_char (ch, obj, iWear);
	    }
	}
    }
}

void
unfinished_quest (CharData * ch)
{
  if (!ch->pcdata)
    return;

  if (ch->pcdata->quest.status == QUEST_NONE || !ch->pcdata->quest.giver
      || !ch->pcdata->quest.room)
    {
      end_quest (ch, ch->pcdata->quest.time);
      return;
    }

  if (ch->pcdata->quest.mob)
    {
      ch->pcdata->quest.room = ch->pcdata->quest.mob->in_room;
    }
  if (ch->pcdata->quest.obj)
    {
      if (ch->pcdata->quest.status == QUEST_DELIVER)
	obj_to_char (ch->pcdata->quest.obj, ch);
      else
	obj_to_room (ch->pcdata->quest.obj, ch->pcdata->quest.room);
    }
  do_function (ch, &do_quest, "info");
}

char *const qmob_desc[] =
  { "fiend", "criminal", "monster", "traitor", "outcast"
};
char *const qobj_desc[] = { "treasure", "artifact", "item", "keepsake"
};
int maxqmobdesc = sizeof (qmob_desc) / sizeof (qmob_desc[0]) - 1;
int maxqobjdesc = sizeof (qobj_desc) / sizeof (qobj_desc[0]) - 1;


Do_Fun (do_quest)
{
  CharData *questman;
  ObjData *obj = NULL;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int i = 0;

  if (IsNPC (ch))
    {
      chprintln (ch, "I'm sorry, you can't quest.");
      return;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "info", "request", "complate", "list",
		  "buy", "quit", "sell", "identify", NULL);
      if (IsImmortal (ch))
	cmd_syntax (ch, NULL, n_fun, "reset <player>", NULL);
      chprintln (ch, "For more information, see 'HELP QUEST'.");
      return;
    }
  else if (!str_prefix (arg1, "info"))
    {

      chprintln (ch, NULL);
      switch (ch->pcdata->quest.status)
	{
	default:
	  chprintln (ch, "{RYour quest is {5ALMOST{x{R complete!{x");
	  act
	    ("{RYou have $t to get back to %s{Rbefore your time runs out!{x",
	     ch, intstr (ch->pcdata->quest.time, "minute"),
	     ch->pcdata->quest.giver, TO_CHAR);
	  return;
	case QUEST_NONE:
	  chprintln (ch, "You aren't currently on a quest.");
	  chprintlnf
	    (ch,
	     "There are %s remaining until you can go on another quest.",
	     intstr (ch->pcdata->quest.time, "minute"));
	  chprintlnf (ch, "You have %s.",
		      intstr (ch->pcdata->quest.points, "quest point"));
	  return;

	case QUEST_RETRIEVE:
	  if (ch->pcdata->quest.obj != NULL)
	    {
	      act ("You are on a quest to recover the fabled $p!", ch,
		   ch->pcdata->quest.obj, NULL, TO_CHAR);
	      chprintlnf (ch,
			  "Rumor has it this %s was last seen in the area known as %s, near %s.",
			  qobj_desc[number_range (0, maxqobjdesc)],
			  ch->pcdata->quest.room->area->name,
			  ch->pcdata->quest.room->name);
	      return;
	    }
	  break;

	case QUEST_DELIVER:
	  if (ch->pcdata->quest.obj != NULL && ch->pcdata->quest.mob != NULL)
	    {
	      act ("You are on a quest to deliver $p to $N.", ch,
		   ch->pcdata->quest.obj, ch->pcdata->quest.mob, TO_CHAR);
	      chprintlnf (ch,
			  "Rumor has it %s was last seen in the area known area %s, near %s.",
			  GetName (ch->pcdata->quest.mob),
			  ch->pcdata->quest.room->area->name,
			  ch->pcdata->quest.room->name);
	      return;
	    }
	  break;

	case QUEST_KILL:
	  if (ch->pcdata->quest.mob != NULL)
	    {
	      act ("You are on a quest to slay $N!", ch, NULL,
		   ch->pcdata->quest.mob, TO_CHAR);
	      chprintlnf (ch,
			  "Rumor has it this %s was last seen in the area known as %s, near %s.",
			  qmob_desc[number_range (0, maxqmobdesc)],
			  ch->pcdata->quest.room->area->name,
			  ch->pcdata->quest.room->name);
	      return;
	    }
	  break;

	case QUEST_FINDROOM:
	  if (ch->pcdata->quest.room != NULL)
	    {
	      chprintlnf (ch, "You are on a quest to find %s in %s!",
			  ch->pcdata->quest.room->name,
			  ch->pcdata->quest.room->area->name);
	      return;
	    }
	  break;

	case QUEST_FINDMOB:
	  if (ch->pcdata->quest.room != NULL && ch->pcdata->quest.mob != NULL)
	    {
	      chprintlnf (ch,
			  "You are on a quest to find %s near %s in %s!",
			  GetName (ch->pcdata->quest.mob),
			  ch->pcdata->quest.room->name,
			  ch->pcdata->quest.room->area->name);
	      return;
	    }
	  break;
	}

      return;
    }
  else if (!str_prefix (arg1, "reset") && IsImmortal (ch))
    {
      CharData *victim;

      if (NullStr (arg2))
	{
	  chprintln (ch, "Reset which player?");
	  return;
	}

      if ((victim = get_char_world (ch, arg2)) == NULL)
	{
	  chprintln (ch, "They aren't here.");
	  return;
	}

      if (IsNPC (victim))
	{
	  chprintln (ch, "Mobs dont quest.");
	  return;
	}

      end_quest (victim, 0);

      if (victim == ch)
	chprintln (ch, "You clear your quest.");
      else
	act ("$n has cleared your quest.", ch, NULL, victim, TO_VICT);
      return;
    }



  for (questman = ch->in_room->person_first; questman != NULL;
       questman = questman->next_in_room)
    {
      if (!IsNPC (questman))
	continue;
      if (questman->spec_fun == spec_questmaster)
	break;
    }

  if (questman == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  if (questman->fighting != NULL)
    {
      chprintln (ch, "Wait until the fighting stops.");
      return;
    }



  if (!str_prefix (arg1, "list"))
    {
      ObjIndex *test;

      act ("$n asks $N for a list of quest items.", ch, NULL, questman,
	   TO_ROOM);
      chprintln (ch, "\tCurrent Quest Items available for Purchase:");
      for (i = 0; quest_table[i].name != NULL; i++)
	{
	  test = get_obj_index (quest_table[i].vnum);
	  chprintlnf (ch, "\t%-4dqp ........ %s",
		      quest_table[i].cost,
		      test ? test->short_descr : quest_table[i].descr !=
		      NULL ? quest_table[i].descr : "Unavailable");
	}
      chprintlnf (ch, "\tTo buy an item, type '%s buy <item>'.", n_fun);
      return;
    }
  else if (!str_prefix (arg1, "buy"))
    {
      if (NullStr (arg2))
	{
	  chprintlnf (ch, "To buy an item, type '%s buy <item>'.", n_fun);
	  return;
	}

      if ((i = quest_lookup (arg2)) == -1)
	{
	  mob_tell (ch, questman, "I don't have that item, %s.", ch->name);
	  return;
	}

      if (ch->pcdata->quest.points < quest_table[i].cost)
	{
	  mob_tell (ch, questman,
		    "You need %s for that.",
		    intstr (quest_table[i].cost, "questpoint"));
	  return;
	}


      if (quest_table[i].vnum == 0)
	{
	  ch->pcdata->quest.points -= quest_table[i].cost;

	  ch->pcdata->condition[COND_FULL] = -1;
	  ch->pcdata->condition[COND_HUNGER] = -1;
	  ch->pcdata->condition[COND_THIRST] = -1;
	  act
	    ("$N calls upon the power of the gods to relieve your mortal burdens.",
	     ch, NULL, questman, TO_CHAR);
	  act
	    ("$N calls upon the power of the gods to relieve $n's mortal burdens.",
	     ch, NULL, questman, TO_ROOM);
	  return;
	}
      else if ((obj =
		create_object (get_obj_index
			       (quest_table[i].vnum), ch->level)) == NULL)
	{
	  chprintln (ch,
		     "That object could not be found, contact an immortal.");
	  return;
	}

      ch->pcdata->quest.points -= quest_table[i].cost;

      act ("$N gives $p to $n.", ch, obj, questman, TO_ROOM);
      act ("$N gives you $p.", ch, obj, questman, TO_CHAR);
      obj_to_char (obj, ch);
      save_char_obj (ch);
      return;
    }
  else if (!str_prefix (arg1, "sell"))
    {
      if (NullStr (arg2))
	{
	  chprintlnf (ch, "To sell an item, type '%s sell <item>'.", n_fun);
	  return;
	}
      if ((obj = get_obj_carry (ch, arg2, ch)) == NULL)
	{
	  chprintln (ch, "Which item is that?");
	  return;
	}

      if (!IsObjStat (obj, ITEM_QUEST))
	{
	  mob_tell (ch, questman, "That is not a quest item.");
	  return;
	}

      if ((i = qobj_lookup (obj)) == -1)
	{
	  mob_tell (ch, questman, "I only take items I sell, %s.", ch->name);
	  return;
	}

      ch->pcdata->quest.points += quest_table[i].cost / 3;
      act ("$N takes $p from $n.", ch, obj, questman, TO_ROOM);
      sprintf (buf,
	       "$N takes $p from you for %s.",
	       intstr (quest_table[i].cost / 3, "questpoint"));
      act (buf, ch, obj, questman, TO_CHAR);
      extract_obj (obj);
      save_char_obj (ch);
      return;
    }
  else if (!str_prefix (arg1, "identify"))
    {
      if (NullStr (arg2))
	{
	  chprintlnf (ch,
		      "To identify an item, type '%s identify <item>'.",
		      n_fun);
	  return;
	}

      if ((i = quest_lookup (arg2)) == -1)
	{
	  mob_tell (ch, questman, "I don't have that item.");
	  return;
	}

      if (quest_table[i].vnum == 0)
	{
	  chprintln (ch, "That isn't a quest item.");
	  return;
	}

      if ((obj = create_object (get_obj_index (quest_table[i].vnum),
				ch->level)) == NULL)
	{
	  chprintln (ch,
		     "That object could not be found, contact an immortal.");
	  return;
	}

      obj_to_char (obj, ch);
      act ("$p costs $T.", ch, obj,
	   intstr (quest_table[i].cost, "questpoint"), TO_CHAR);
      spell_identify (0, ch->level, ch, obj, TAR_OBJ_INV);
      extract_obj (obj);
      return;
    }
  else if (!str_prefix (arg1, "request"))
    {
      quest_t type = QUEST_NONE;

      if (IsImmortal (ch))
	{
	  if (NullStr (arg2))
	    {
	      chprintlnf (ch,
			  "{W%s{x" NEWLINE "  {GAvailable Quests" NEWLINE
			  "{W%s{x" NEWLINE
			  "   {Y1{D. {GFind a room in the realm" NEWLINE
			  "   {Y2{D. {GFind a person" NEWLINE
			  "   {Y3{D. {GRetrieve stolen property" NEWLINE
			  "   {Y4{D. {GDeliver priceless artifact" NEWLINE
			  "   {Y5{D. {GHunt down heinous criminal" NEWLINE
			  "   {Y6{D. {GRandom quest 1-5" NEWLINE "{W%s{x"
			  NEWLINE
			  "{xTo get a quest, type {WQUEST REQUEST {D<{Ynumber{D>"
			  NEWLINE "{W%s{x", draw_line (ch, NULL, 58),
			  draw_line (ch, NULL, 58), draw_line (ch, NULL,
							       58),
			  draw_line (ch, NULL, 58));
	      return;
	    }

	  if (!is_number (arg2))
	    {
	      chprintln
		(ch, "{xTo get a quest, type {WQUEST REQUEST {D<{Ynumber{D>");
	      return;
	    }

	  switch (atoi (arg2))
	    {
	    case 1:
	      type = QUEST_FINDROOM;
	      break;
	    case 2:
	      type = QUEST_FINDMOB;
	      break;
	    case 3:
	      type = QUEST_RETRIEVE;
	      break;
	    case 4:
	      type = QUEST_DELIVER;
	      break;
	    case 5:
	      type = QUEST_KILL;
	      break;
	    case 6:
	      break;
	    default:
	      chprintln (ch, "Invalid quest request.");
	      return;
	    }
	}
      act ("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
      act ("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);
      if (IsQuester (ch))
	{
	  act ("$N rejects $n's request.", ch, NULL, questman, TO_ROOM);
	  mob_tell (ch, questman, "But you're already on a quest!");
	  return;
	}

      if (IsImmortal (ch))
	end_quest (ch, 0);

      if (ch->pcdata->quest.time > 0)
	{
	  mob_tell (ch, questman,
		    "You're very brave, %s, but let someone else have a chance.",
		    ch->name);
	  mob_tell (ch, questman, "Come back later.");
	  return;
	}

      mob_tell (ch, questman, "Thank you, brave %s!", ch->name);

      generate_quest (ch, questman, type);
      if (IsQuester (ch))
	mud_info.stats.quests++;
      return;
    }
  else if (!str_prefix (arg1, "complete"))
    {
      if (ch->pcdata->quest.giver != questman)
	{
	  mob_tell (ch, questman,
		    "I never sent you on a quest! Perhaps you're thinking of someone else.");
	  return;
	}

      if (IsQuester (ch))
	{
	  if (quest_complete (ch, questman))
	    return;
	  else if (ch->pcdata->quest.status > QUEST_NONE
		   && ch->pcdata->quest.time > 0)
	    {
	      mob_tell (ch, questman,
			"You haven't completed the quest yet, but there is still time!");
	      return;
	    }
	}
      else
	mob_tell (ch, questman, "You have to REQUEST a quest first.");
      return;
    }
  else if (!str_prefix (arg1, "quit") || !str_prefix (arg1, "fail"))
    {
      act ("$n informs $N $e wishes to quit $s quest.", ch, NULL,
	   questman, TO_ROOM);
      act ("You inform $N you wish to quit $s quest.", ch, NULL,
	   questman, TO_CHAR);
      if (ch->pcdata->quest.giver != questman)
	{
	  mob_tell (ch, questman,
		    "I never sent you on a quest! Perhaps you're thinking of someone else.");
	  return;
	}

      if (IsQuester (ch))
	{
	  end_quest (ch, QUEST_TIME * 3 / 2);
	  mob_tell (ch, questman,
		    "Your quest is over, but for your cowardly behavior, you may not quest again for 15 minutes.");
	  return;
	}
      else
	{
	  chprintln (ch, "You aren't on a quest!");
	  return;
	}
    }

  do_quest (n_fun, ch, "");
  return;
}

#define MAX_QMOB_COUNT mobile_count

CharData *
random_quest_mob (CharData * ch, CharData * questman)
{
  CharData *victim;
  CharData **mobs;
  int mob_count;
  int mrange;


  alloc_mem (mobs, CharData *, MAX_QMOB_COUNT);

  mob_count = 0;
  for (victim = char_first; victim; victim = victim->next)
    {
      if (!IsNPC (victim)
	  || !quest_level_diff (ch, victim)
	  || victim->pIndexData == NULL
	  || victim->in_room == NULL
	  || victim->pIndexData->pShop != NULL
	  || (IsEvil (victim) && IsEvil (ch) && chance (50))
	  || (IsGood (victim) && IsGood (ch) && chance (50))
	  || victim->pIndexData->vnum < 100
	  || victim->in_room->area->clan != NULL
	  || IsSet (victim->imm_flags, IMM_WEAPON | IMM_MAGIC)
	  || IsSet (victim->act,
		    ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER | ACT_PET
		    | ACT_PET | ACT_GAIN)
	  || IsSet (victim->affected_by, AFF_CHARM)
	  || IsSet (victim->in_room->room_flags, ROOM_PET_SHOP)
	  || questman->pIndexData == victim->pIndexData
	  || (IsSet (victim->act, ACT_SENTINEL)
	      && IsSet (victim->in_room->room_flags,
			ROOM_PRIVATE | ROOM_SOLITARY | ROOM_SAFE)))
	continue;
      mobs[mob_count++] = victim;
      if (mob_count >= MAX_QMOB_COUNT)
	break;
    }
  do
    {
      mrange = number_range (0, mob_count - 1);
    }
  while ((victim = mobs[mrange]) == NULL);

  free_mem (mobs);
  return victim;
}

void
generate_quest (CharData * ch, CharData * questman, quest_t type)
{
  CharData *victim;

  if ((victim = random_quest_mob (ch, questman)) == NULL)
    {
      mob_tell (ch, questman,
		"I'm sorry, but I don't have any quests for you at this time.");
      mob_tell (ch, questman, "Try again later.");
      end_quest (ch, QUEST_TIME / 10);
      return;
    }


  ch->pcdata->quest.giver = questman;

  ch->pcdata->quest.room = victim->in_room;


  ch->pcdata->quest.time = number_range (15, 30);

  ch->pcdata->quest.status = type > QUEST_NONE ? type : chance (10) ?
    (chance (50) ? QUEST_FINDMOB : QUEST_FINDROOM)
    : chance (20) ? (chance (50) ? QUEST_RETRIEVE : QUEST_DELIVER) :
    QUEST_KILL;

  switch (ch->pcdata->quest.status)
    {
    case QUEST_RETRIEVE:
      ch->pcdata->quest.obj = create_quest_obj (ch, -1);
      obj_to_room (ch->pcdata->quest.obj, ch->pcdata->quest.room);
      replace_str (&ch->pcdata->quest.obj->owner, ch->name);
      ch->pcdata->quest.obj->cost = 0;
      ch->pcdata->quest.obj->timer = (4 * ch->pcdata->quest.time + 10) / 3;
      ch->pcdata->quest.mob = NULL;

      switch (number_range (0, 1))
	{
	default:
	case 0:
	  mob_tell (ch, questman,
		    "Vile pilferers have stolen %s from the royal treasury!",
		    ch->pcdata->quest.obj->short_descr);
	  mob_tell (ch, questman,
		    "My court wizardess, with her magic mirror, has pinpointed its location.");
	  break;
	case 1:
	  mob_tell (ch, questman,
		    "A powerful wizard has stolen %s for his personal power!",
		    ch->pcdata->quest.obj->short_descr);
	  break;
	}

      mob_tell (ch, questman,
		"This %s was last seen somewhere in the vicinity of %s!",
		qobj_desc[number_range (0, maxqobjdesc)],
		victim->in_room->name);
      break;



    case QUEST_KILL:
      ch->pcdata->quest.mob = victim;
      ch->pcdata->quest.obj = NULL;
      switch (number_range (0, 3))
	{
	default:
	case 0:
	  mob_tell (ch, questman,
		    "An enemy of mine, %s, is making vile threats against the crown.",
		    GetName (victim));
	  mob_tell (ch, questman, "This threat must be eliminated!");
	  break;

	case 1:
	  mob_tell (ch, questman,
		    "{n's most heinous criminal, %s, has escaped from the dungeon!",
		    GetName (victim));
	  mob_tell (ch, questman,
		    "Since the escape, %s has murdered %d civillians!",
		    GetName (victim), number_range (2, 20));
	  mob_tell (ch, questman,
		    "The penalty for this crime is death, and you are to deliver the sentence!");
	  break;

	case 2:
	  mob_tell (ch, questman,
		    "The Mayor of Midgaard has recently been attacked by %s.  This is an act of war!",
		    GetName (victim));
	  mob_tell (ch, questman,
		    "%s must be severly dealt with for this injustice.",
		    GetName (victim));
	  break;

	case 3:

	  mob_tell (ch, questman,
		    "%s has been stealing valuables from the citizens of %s.",
		    GetName (victim), victim->in_room->area->name);
	  mob_tell (ch, questman,
		    "Make sure that %s never has the chance to steal again.",
		    GetName (victim));
	  break;

	}

      mob_tell (ch, questman,
		"Seek this %s out somewhere in the vicinity of %s!",
		qmob_desc[number_range (0, maxqmobdesc)],
		victim->in_room->name);
      break;

    case QUEST_DELIVER:
      RemBit (victim->act, ACT_AGGRESSIVE);
      victim->position = POS_STANDING;
      victim->spec_fun = NULL;

      ch->pcdata->quest.obj = create_quest_obj (ch, -1);
      ch->pcdata->quest.mob = victim;
      ch->pcdata->quest.room = victim->in_room;

      obj_to_char (ch->pcdata->quest.obj, ch);

      mob_tell (ch, questman,
		"Please deliver this %s to my friend - %s. Time is the essence, please hurry.",
		ch->pcdata->quest.obj->short_descr, victim->short_descr);
      mob_tell (ch, questman,
		"Seek %s out somewhere in the vicinity of {W%s{x!",
		victim->short_descr, victim->in_room->name);

      act ("$n gives $p to $N.", questman, ch->pcdata->quest.obj, ch,
	   TO_NOTVICT);
      act ("$n gives you $p.", questman, ch->pcdata->quest.obj, ch, TO_VICT);
      act ("You give $p to $N.", questman, ch->pcdata->quest.obj, ch,
	   TO_CHAR);
      break;

    case QUEST_FINDROOM:
      ch->pcdata->quest.mob = NULL;
      ch->pcdata->quest.obj = NULL;
      ch->pcdata->quest.room = victim->in_room;

      mob_tell (ch, questman,
		"This quest tests your knowledge of {n. Your goal is simple, seek out");
      mob_tell (ch, questman,
		"the location '{W%s{x' and return to me.",
		victim->in_room->name);
      mob_tell (ch, questman,
		"You will be told when you find the right place.");
      break;

    case QUEST_FINDMOB:
      ch->pcdata->quest.mob = victim;
      ch->pcdata->quest.obj = NULL;
      ch->pcdata->quest.room = victim->in_room;

      mob_tell (ch, questman,
		"This quest tests your knowledge of {n. Your goal is simple, seek out");
      mob_tell (ch, questman,
		"'{W%s{x' in vicinity of {W%s{x, and return to me.",
		victim->short_descr, victim->in_room->name);
      mob_tell (ch, questman,
		"You will be told when you find the right person.");
      break;

    default:
      bug ("generate_quest(): bad quest type");
      end_quest (ch, QUEST_TIME / 5);
      return;
    }


  mob_tell (ch, questman,
	    "The location is in the general area of %s.",
	    victim->in_room->area->name);

  mob_tell (ch, questman, "You have %s to complete this quest.",
	    intstr (ch->pcdata->quest.time, "minute"));
  mob_tell (ch, questman, "May %s go with you!", ch->deity->name);

  return;
}

bool
quest_level_diff (CharData * ch, CharData * mob)
{
  int bonus = 10 + lvl_bonus (ch);

  if (IsImmortal (ch))
    return true;
  else if (ch->level > (mob->level + bonus)
	   || ch->level < (mob->level - bonus))
    return false;
  else
    return true;
}

void
quest_update (void)
{
  CharData *ch;

  for (ch = player_first; ch != NULL; ch = ch->next_player)
    {
      if (!ch->desc || ch->desc->connected != CON_PLAYING
	  || ch->pcdata->quest.time <= 0)
	continue;

      if (ch->pcdata->quest.status != QUEST_NONE)
	{
	  if (--ch->pcdata->quest.time <= 0)
	    {
	      end_quest (ch, QUEST_TIME - 2);
	      chprintlnf (ch,
			  "{RYou have run out of time for your quest!"
			  "  You may quest again in %d minutes.{x",
			  ch->pcdata->quest.time);
	    }
	  else if (ch->pcdata->quest.time < 6)
	    {
	      chprintln (ch,
			 "{pBetter hurry, you're almost out of time for your quest!");
	      return;
	    }
	}
      else
	{
	  if (--ch->pcdata->quest.time <= 0)
	    {
	      chprintln (ch, "{WYou may now {?quest{W again.{x");
	      return;
	    }
	}
    }
  return;
}

void
end_quest (CharData * ch, int time)
{
  if (!ch || IsNPC (ch))
    return;

  ch->pcdata->quest.status = QUEST_NONE;
  ch->pcdata->quest.giver = 0;
  ch->pcdata->quest.time = time;
  ch->pcdata->quest.mob = 0;
  ch->pcdata->quest.obj = 0;
  ch->pcdata->quest.room = 0;
}


void
quest_reward (CharData * ch, CharData * questman, quest_t type)
{
  money_t reward = 0;
  int pointreward = 0;
  int time = 0;


  switch (type)
    {
    case QUEST_RETURN_KILL:
      pointreward = 50;
      reward = ch->level * 4;
      break;

    case QUEST_RETURN_DELIVER:
      pointreward = 40;
      reward = ch->level * 3;
      break;

    case QUEST_RETURN_RETRIEVE:
      pointreward = 30;
      reward = ch->level * 2;
      break;

    case QUEST_RETURN_FINDMOB:
      pointreward = 25;
      reward = ch->level * 2;
      time = -4;
      break;

    case QUEST_RETURN_FINDROOM:
      pointreward = 20;
      reward = ch->level * 2;
      time = -5;
      break;

    default:
      pointreward = 20;
      reward = ch->level;
      break;
    }

  pointreward = number_range (pointreward * 4 / 5, pointreward);
  reward = number_range (reward * 4 / 5, reward);

  if (pointreward + ch->pcdata->quest.points > 32000)
    {
      unsigned int t = 0;

      t = (pointreward + ch->pcdata->quest.points) - 32000;
      pointreward -= t;
      reward += t;
    }


  if (ch->pcdata->quest.obj != NULL)
    extract_obj (ch->pcdata->quest.obj);
  if (ch->pcdata->quest.mob != NULL)
    {
      if (IsNPC (ch->pcdata->quest.mob))
	{
	  ch->pcdata->quest.mob->master = NULL;
	  extract_char (ch->pcdata->quest.mob, true);
	}
    }

  end_quest (ch, QUEST_TIME + time);
  ch->gold += reward;
  pointreward = add_qp (ch, pointreward);
  act ("$N congratulates $n.", ch, NULL, questman, TO_ROOM);
  mob_tell (ch, questman,
	    "Congratulations on completing your quest! As a reward, I am giving you {W%s{x, and {Y%ld{x gold.",
	    intstr (pointreward, "quest point"), reward);
  if (chance (pointreward / 5))
    {
      chprintln (ch, "You gain an extra {YTrivia {RPoint{x!");
      ch->pcdata->trivia += 1;
    }
  mud_info.stats.qcomplete++;
  save_char_obj (ch);
  return;
}

bool
quest_complete (CharData * ch, CharData * questman)
{
  ObjData *obj = NULL, *obj_next;

  if (ch->pcdata->quest.time <= 0)
    {
      act ("$N informs $n about returning too late.", ch, NULL,
	   questman, TO_ROOM);
      chprintln (ch, NEWLINE "But you didn't complete your quest in time!");

      end_quest (ch, QUEST_TIME + 5);

      return true;
    }

  switch (ch->pcdata->quest.status)
    {
    case QUEST_NONE:
      act ("$N explains $n how to request quest.", ch, NULL,
	   questman, TO_ROOM);
      chprintln (ch, NEWLINE "You have to REQUEST a quest first.");
      return true;
      break;

    case QUEST_KILL:
    case QUEST_RETRIEVE:
    case QUEST_DELIVER:
    case QUEST_FINDMOB:
    case QUEST_FINDROOM:
      return false;
      break;

    case QUEST_RETURN_KILL:
    case QUEST_RETURN_FINDMOB:
    case QUEST_RETURN_FINDROOM:
    case QUEST_RETURN_DELIVER:
      quest_reward (ch, questman, ch->pcdata->quest.status);
      return true;
      break;

    case QUEST_RETURN_RETRIEVE:
      {
	bool obj_found = false;

	for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
	  {
	    obj_next = obj->next_content;

	    if (obj == ch->pcdata->quest.obj)
	      {
		if (is_name (ch->name, obj->owner))
		  {
		    obj_found = true;
		    break;
		  }
		else
		  {
		    chprintln (ch, "Cheating is not nice, you know?");
		    act ("$N calls $n cheater!", ch, NULL, questman, TO_ROOM);
		    act ("$N calls You cheater!", ch, NULL, questman,
			 TO_CHAR);
		    obj->timer = 1;
		  }
	      }
	  }

	if (obj_found == true)
	  {
	    quest_reward (ch, questman, ch->pcdata->quest.status);
	  }
	else
	  {
	    act ("$N sends $n to complete his quest.", ch, NULL,
		 questman, TO_ROOM);
	    chprintln
	      (ch,
	       NEWLINE
	       "You haven't completed the quest yet, but there is still time!");
	  }
	return true;
      }
      break;

    default:
      return false;
    }
}


void
quest_room_check (CharData * ch)
{
  CharData *victim;

  if (!IsQuester (ch) || ch->in_room != ch->pcdata->quest.room)
    return;

  switch (ch->pcdata->quest.status)
    {
    case QUEST_FINDROOM:
      chprintln (ch, NEWLINE "{5+RYou have almost completed your QUEST!{x");
      act ("{RReturn to $N{R before your time runs out!{x", ch, NULL,
	   ch->pcdata->quest.giver, TO_CHAR);
      ch->pcdata->quest.status = QUEST_RETURN_FINDROOM;
      ch->pcdata->quest.room = NULL;
      break;

    case QUEST_FINDMOB:
      for (victim = ch->in_room->person_first; victim != NULL;
	   victim = victim->next_in_room)
	{
	  if (victim != ch->pcdata->quest.mob)
	    continue;

	  do_function (victim, &do_say,
		       "Excellent! You have found me. Good job!");
	  chprintln (ch, NEWLINE "{5+RYou have almost completed your QUEST!");
	  act ("{RReturn to $N{R before your time runs out!{x", ch, NULL,
	       ch->pcdata->quest.giver, TO_CHAR);
	  ch->pcdata->quest.status = QUEST_RETURN_FINDMOB;
	  ch->pcdata->quest.mob = NULL;
	  break;
	}
      break;

    case QUEST_KILL:
      for (victim = ch->in_room->person_first; victim != NULL;
	   victim = victim->next_in_room)
	{
	  if (victim != ch->pcdata->quest.mob)
	    continue;

	  act ("$N eye's you warily...", ch, NULL, victim, TO_CHAR);
	  act ("$N eye's $n warily...", ch, NULL, victim, TO_ROOM);
	  break;
	}
      break;

    case QUEST_DELIVER:
      for (victim = ch->in_room->person_first; victim != NULL;
	   victim = victim->next_in_room)
	{
	  if (victim != ch->pcdata->quest.mob)
	    continue;

	  act ("$N smiles at you...", ch, NULL, victim, TO_CHAR);
	  act ("$N smiles at $n...", ch, NULL, victim, TO_ROOM);
	  break;
	}
      break;

    default:
      break;
    }
}


void
mob_tell (CharData * ch, CharData * victim, const char *argument, ...)
{
  char buf[MSL];
  va_list args;

  if (!victim || NullStr (argument))
    return;

  va_start (args, argument);
  vsnprintf (buf, sizeof (buf), argument, args);
  va_end (args);

  chprintlnf (ch, CTAG (_TELLS1) "%s " CTAG (_TELLS1) "tells you '"
	      CTAG (_TELLS2) "%s" CTAG (_TELLS1) "'{x",
	      GetName (victim), str_rep (buf, "{x", CTAG (_TELLS2)));
  return;
}

Do_Fun (do_tpspend)
{
  CharData *triviamob;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int sav_trust;

  argument = one_argument (argument, arg1);
  strcpy (arg2, argument);

  if (!ch || IsNPC (ch))
    return;

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "<item>", "list", NULL);
      return;
    }

  if (is_name (arg1, "list"))
    {
      chprintln (ch, "Trivia Point Options");
      chprintln (ch, "corpse retrival......1tp");
      chprintln (ch, "transfer.............1tp");
      chprintln (ch, "restore..............1tp");
      chprintln (ch, "5 trains.............1tp");
      chprintln (ch, "40 practices.........1tp");
      chprintln (ch, "75 questpoints.......1tp");
      chprintln (ch, "1 Trivia Pill........1tp");
      chprintln (ch, "See HELP TRIVIA for important info before buying.");
      chprintln (ch,
		 "For transfers and corpses, you do not have to be at a trivia shop.");
      return;
    }
  else if (is_name (arg1, "corpse"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  ObjData *c;
	  int count = 0;

	  for (c = obj_first; c != NULL; c = c->next)
	    {
	      if (is_name (ch->name, c->owner)
		  && c->item_type == ITEM_CORPSE_PC)
		{
		  if (c->in_room)
		    obj_from_room (c);
		  else if (c->carried_by)
		    obj_from_char (c);
		  else if (c->in_obj)
		    obj_from_obj (c->in_obj);
		  obj_to_room (c, ch->in_room);
		  count++;
		}
	    }
	  if (count == 0)
	    chprintln (ch, "You have no corpses in the game.");
	  else
	    {
	      ch->pcdata->trivia -= 1;
	      if (count == 1)
		{
		  chprintln (ch, "Your corpse appears in the room.");
		  act ("$n's corpse appears in the room.",
		       ch, NULL, NULL, TO_ROOM);
		}
	      else
		{
		  chprintln (ch, "All your corpses appear in the room.");
		  act ("All of $n's corpses appear in the room.",
		       ch, NULL, NULL, TO_ROOM);
		}
	    }
	  return;
	}
      else
	{
	  chprintln (ch, "You don't have enough trivia points for that.");
	  return;
	}
    }
  else if (is_name (arg1, "transfer"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  RoomIndex *oldroom;

	  if (NullStr (arg2))
	    {
	      chprintln (ch,
			 "Transfer you where? [recall/room name/character name]");
	      return;
	    }
	  else if (!str_prefix (arg2, "recall"))
	    sprintf (arg2, "%d", ROOM_VNUM_TEMPLE);

	  oldroom = ch->in_room;
	  sprintf (buf, "self '%s'", arg2);
	  sav_trust = ch->trust;
	  ch->trust = MAX_LEVEL;
	  do_function (ch, &do_transfer, buf);
	  ch->trust = sav_trust;
	  if (oldroom != ch->in_room)
	    ch->pcdata->trivia -= 1;
	  else
	    chprintln (ch, "Whoops! You were not charged for that transfer.");
	  return;
	}
      else
	{
	  chprintln (ch, "You don't have enough trivia points for that.");
	  return;
	}
    }
  for (triviamob = ch->in_room->person_first; triviamob != NULL;
       triviamob = triviamob->next_in_room)
    {
      if (!IsNPC (triviamob))
	continue;
      if (triviamob->spec_fun == spec_triviamob)
	break;
    }

  if (triviamob == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  if (triviamob->fighting != NULL)
    {
      chprintln (ch, "Wait until the fighting stops.");
      return;
    }

  if (is_name (arg1, "practices pracs practice"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  ch->pcdata->trivia -= 1;
	  ch->practice += 40;
	  act ("$N gives 40 practices to $n.", ch, NULL, triviamob, TO_ROOM);
	  act ("$N gives you 40 practices.", ch, NULL, triviamob, TO_CHAR);
	  return;
	}
      else
	{
	  mob_tell (ch, triviamob,
		    "You don't have enough trivia points for that.");
	  return;
	}
    }
  else if (is_name (arg1, "trains train"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  ch->pcdata->trivia -= 1;
	  ch->train += 5;
	  act ("$N gives 5 training sessions to $n.", ch, NULL,
	       triviamob, TO_ROOM);
	  act ("$N gives you 5 training sessions.", ch, NULL,
	       triviamob, TO_CHAR);
	  return;
	}
      else
	{
	  mob_tell (ch, triviamob,
		    "You don't have enough trivia points for that.");
	  return;
	}
    }
  else if (is_name (arg1, "questpoints points"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  ch->pcdata->trivia -= 1;
	  ch->pcdata->quest.points += 75;
	  act ("$N gives 75 questpoints to $n.", ch, NULL, triviamob,
	       TO_ROOM);
	  act ("$N gives you 75 questpoints.", ch, NULL, triviamob, TO_CHAR);
	  return;
	}
      else
	{
	  mob_tell (ch, triviamob,
		    "You don't have enough trivia points for that.");
	  return;
	}
    }
  else if (is_name (arg1, "pill"))
    {
      ObjData *obj = NULL;

      if (ch->pcdata->trivia >= 1)
	{
	  obj = create_object (get_obj_index (OBJ_VNUM_TRIVIA_PILL), 1);
	  if (obj != NULL)
	    {
	      act ("$N gives $p to $n.", ch, obj, triviamob, TO_ROOM);
	      act ("$N gives you $p.", ch, obj, triviamob, TO_CHAR);
	      obj_to_char (obj, ch);
	      ch->pcdata->trivia -= 1;
	      return;
	    }
	  else
	    {
	      mob_tell (ch, triviamob,
			"I don't any more trivia pills to give.");
	    }
	  return;
	}
      else
	{
	  mob_tell (ch, triviamob,
		    "You don't have enough trivia points for that.");
	  return;
	}
    }
  else if (is_name (arg1, "restore"))
    {
      if (ch->pcdata->trivia >= 1)
	{
	  sav_trust = ch->trust;
	  ch->trust = MAX_LEVEL;
	  do_function (ch, &do_restore, "all");
	  ch->trust = sav_trust;
	  ch->pcdata->trivia -= 1;
	  return;
	}
      else
	{
	  mob_tell (ch, triviamob,
		    "You don't have enough trivia points for that.");
	  return;
	}
    }
  else
    do_tpspend (n_fun, ch, "list");
}

Do_Fun (do_qpgive)
{
  CharData *victim;
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  int amount;

  if (!ch || IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (argument) || !is_number (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<amount> <person>", NULL);
      return;
    }

  if ((amount = atoi (arg)) <= 0)
    {
      chprintln (ch, "Give how many questpoints?");
      return;
    }

  if (amount > ch->pcdata->quest.points)
    {
      chprintln (ch, "You don't have that many questpoints to give.");
      return;
    }

  if ((victim = get_char_room (ch, NULL, argument)) == NULL)
    {
      chprintln (ch, "That person is not here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "NPC's don't need quest points.");
      return;
    }

  if (victim == ch)
    {
      chprintlnf (ch,
		  "You give yourself %s..... don't you feel better?",
		  intstr (amount, "questpoint"));
      return;
    }

  ch->pcdata->quest.points -= amount;
  victim->pcdata->quest.points += amount;

  sprintf (buf, "%d", amount);
  act ("$n gives you $t questpoints.", ch, buf, victim, TO_VICT);
  act ("You give $N $t questpoints.", ch, buf, victim, TO_CHAR);
  act ("$n gives $N $t questpoints.", ch, buf, victim, TO_ROOM);

  return;
}

Do_Fun (do_tpgive)
{
  CharData *victim;
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  int amount;

  if (!ch || IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (argument) || !is_number (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<amount> <person>", NULL);
      return;
    }

  if ((amount = atoi (arg)) <= 0)
    {
      chprintln (ch, "Give how many trivia points?");
      return;
    }

  if (amount > ch->pcdata->trivia)
    {
      chprintln (ch, "You don't have that many trivia points to give!!");
      return;
    }

  if ((victim = get_char_room (ch, NULL, argument)) == NULL)
    {
      chprintln (ch, "That person is not here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "NPC's don't need trivia points.");
      return;
    }

  if (victim == ch)
    {
      chprintlnf (ch,
		  "You give yourself %s..... don't you feel better?",
		  intstr (amount, "trivia point"));
      return;
    }

  ch->pcdata->trivia -= amount;
  victim->pcdata->trivia += amount;

  sprintf (buf, "%d", amount);
  act ("$n gives you $t questpoints.", ch, buf, victim, TO_VICT);
  act ("You give $N $t questpoints.", ch, buf, victim, TO_CHAR);
  act ("$n gives $N $t questpoints.", ch, buf, victim, TO_ROOM);

  return;
}


CharData *
find_quest_char (CharData * ch, vnum_t vnum)
{
  CharData *mob, *found = NULL;

  if (ch->pcdata->quest.mob)
    return ch->pcdata->quest.mob;

  for (mob = char_first; mob; mob = mob->next)
    {
      if ((IsNPC (mob) ? mob->pIndexData->vnum : mob->id) != vnum)
	continue;

      found = mob;

      if (!IsNPC (mob)
	  || mob->pIndexData->spec_fun == spec_lookup ("spec_questmaster")
	  || mob->in_room == ch->pcdata->quest.room)
	break;
    }

  if (!found)
    {
      CharIndex *pMob = get_char_index (vnum);

      if (!pMob || (found = create_mobile (pMob)) == NULL)
	{
	  if ((found = random_quest_mob (ch, ch->pcdata->quest.giver)) ==
	      NULL)
	    {
	      end_quest (ch, ch->pcdata->quest.time);
	      return NULL;
	    }
	}
    }
  return found;
}


vnum_t
random_quest_piece (void)
{
  vnum_t objvnum = 0;

  switch (number_range (0, 3))
    {
    case 0:
      objvnum = OBJ_VNUM_QUEST1;
      break;

    case 1:
      objvnum = OBJ_VNUM_QUEST2;
      break;

    case 2:
      objvnum = OBJ_VNUM_QUEST3;
      break;

    case 3:
      objvnum = OBJ_VNUM_QUEST4;
      break;
    }
  return objvnum;
}

ObjData *
create_quest_obj (CharData * ch, vnum_t vnum)
{
  ObjData *obj;
  int check = 0;

  if (ch->pcdata->quest.obj != NULL)
    return ch->pcdata->quest.obj;

  if (vnum <= 0)
    vnum = random_quest_piece ();

  while ((obj = create_object (get_obj_index (vnum), ch->level)) == NULL)
    {
      vnum = random_quest_piece ();
      if (++check >= 10)
	{
	  bug ("Bad vnum");
	  return NULL;
	}
    }
  replace_str (&obj->owner, ch->name);
  obj->cost = 0;
  obj->timer = (4 * ch->pcdata->quest.time + 10) / 3;
  return obj;
}

void
extract_quest (CharData * ch)
{
  if (IsNPC (ch))
    return;

  if (ch->pcdata->quest.obj)
    extract_obj (ch->pcdata->quest.obj);
}
