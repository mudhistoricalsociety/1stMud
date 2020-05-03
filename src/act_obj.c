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
#include "magic.h"


Proto (bool remove_obj, (CharData *, int, bool));
Proto (CharData * find_keeper, (CharData *));
Proto (money_t get_cost, (CharData *, ObjData *, bool));

Proto (ObjData * get_obj_keeper, (CharData *, CharData *, char *));



bool
can_loot (CharData * ch, ObjData * obj)
{
  CharData *owner, *wch;

  if (IsImmortal (ch))
    return true;

  if (!obj->owner || obj->owner == NULL)
    return true;

  owner = NULL;
  for (wch = char_first; wch != NULL; wch = wch->next)
    if (!str_cmp (wch->name, obj->owner))
      owner = wch;

  if (owner == NULL)
    return true;

  if (!str_cmp (ch->name, owner->name))
    return true;

  if (!IsNPC (owner) && IsSet (owner->act, PLR_CANLOOT))
    return true;

  if (is_same_group (ch, owner))
    return true;

  return false;
}

void
get_obj (CharData * ch, ObjData * obj, ObjData * container)
{

  CharData *gch;
  int members;
  char buffer[100];

  if (!CanWear (obj, ITEM_TAKE))
    {
      chprintln (ch, "You can't take that.");
      return;
    }

  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
      act ("$d: you can't carry that many items.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }

  if ((!obj->in_obj || obj->in_obj->carried_by != ch) &&
      (get_carry_weight (ch) + get_obj_weight (obj) > can_carry_w (ch)))
    {
      act ("$d: you can't carry that much weight.", ch, NULL, obj->name,
	   TO_CHAR);
      return;
    }

  if (is_donate_room (ch->in_room->vnum)
      && ch->level < obj->level - lvl_bonus (ch))
    {
      chprintln (ch, "You are not powerful enough to use it.");
      return;
    }

  if (!can_loot (ch, obj))
    {
      act ("Corpse looting is not permitted.", ch, NULL, NULL, TO_CHAR);
      return;
    }

  if (obj->in_room != NULL)
    {
      for (gch = obj->in_room->person_first; gch != NULL;
	   gch = gch->next_in_room)
	if (gch->on == obj)
	  {
	    act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
	    return;
	  }
    }

  if (container != NULL)
    {
      act ("You get $p from $P.", ch, obj, container, TO_CHAR);
      act ("$n gets $p from $P.", ch, obj, container, TO_ROOM);
      RemBit (obj->extra_flags, ITEM_HAD_TIMER);
      obj_from_obj (obj);
    }
  else
    {
      act ("You get $p.", ch, obj, container, TO_CHAR);
      act ("$n gets $p.", ch, obj, container, TO_ROOM);
      obj_from_room (obj);
    }

  if (obj->item_type == ITEM_MONEY)
    {
      ch->silver += obj->value[0];
      ch->gold += obj->value[1];
      if (IsSet (ch->act, PLR_AUTOSPLIT))
	{
	  members = 0;
	  for (gch = ch->in_room->person_first; gch != NULL;
	       gch = gch->next_in_room)
	    {
	      if (!IsAffected (gch, AFF_CHARM) && is_same_group (gch, ch))
		members++;
	    }

	  if (members > 1 && (obj->value[0] > 1 || obj->value[1]))
	    {
	      sprintf (buffer, "%ld %ld", obj->value[0], obj->value[1]);
	      do_function (ch, &do_split, buffer);
	    }
	}

      extract_obj (obj);
    }
  else
    {
      obj_to_char (obj, ch);
      if (HasTriggerObj (obj, TRIG_GET))
	p_give_trigger (NULL, obj, NULL, ch, obj, TRIG_GET);
      if (HasTriggerRoom (ch->in_room, TRIG_GET))
	p_give_trigger (NULL, NULL, ch->in_room, ch, obj, TRIG_GET);
      if (IsQuester (ch) && ch->pcdata->quest.obj == obj)
	{
	  chprintln (ch, "{5+RYou have almost completed your QUEST!{x");
	  act ("{RReturn to $N before your time runs out!", ch, NULL,
	       ch->pcdata->quest.giver, TO_CHAR);
	  ch->pcdata->quest.status = QUEST_RETURN_RETRIEVE;
	}
    }

  return;
}

Do_Fun (do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *obj;
  ObjData *obj_next;
  ObjData *container;
  bool found;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (!str_cmp (arg2, "from"))
    argument = one_argument (argument, arg2);


  if (NullStr (arg1))
    {
      chprintln (ch, "Get what?");
      return;
    }

  if (NullStr (arg2))
    {
      if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
	{

	  obj = get_obj_list (ch, arg1, ch->in_room->content_first);
	  if (obj == NULL)
	    {
	      act ("I see no $T here.", ch, NULL, arg1, TO_CHAR);
	      return;
	    }

	  get_obj (ch, obj, NULL);
	}
      else
	{
	  if (is_donate_room (ch->in_room->vnum))
	    {
	      chprintln (ch, "Don't be so greedy!");
	      return;
	    }

	  found = false;
	  for (obj = ch->in_room->content_first; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
		  && can_see_obj (ch, obj))
		{
		  found = true;
		  get_obj (ch, obj, NULL);
		}
	    }

	  if (!found)
	    {
	      if (arg1[3] == '\0')
		chprintln (ch, "I see nothing here.");
	      else
		act ("I see no $T here.", ch, NULL, &arg1[4], TO_CHAR);
	    }
	}
    }
  else
    {

      if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
	{
	  chprintln (ch, "You can't do that.");
	  return;
	}

      if ((container = get_obj_here (ch, NULL, arg2)) == NULL)
	{
	  act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
	  return;
	}

      switch (container->item_type)
	{
	default:
	  chprintln (ch, "That's not a container.");
	  return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	  break;

	case ITEM_CORPSE_PC:
	  if (!can_loot (ch, container))
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }
	  break;
	}

      if (IsSet (container->value[1], CONT_CLOSED))
	{
	  act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
	  return;
	}

      if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
	{

	  obj = get_obj_list (ch, arg1, container->content_first);
	  if (obj == NULL)
	    {
	      act ("I see nothing like that in the $T.", ch,
		   NULL, arg2, TO_CHAR);
	      return;
	    }
	  get_obj (ch, obj, container);
	}
      else
	{
	  if (is_donate_room (ch->in_room->vnum))
	    {
	      chprintln (ch, "Don't be so greedy!");
	      return;
	    }

	  found = false;
	  for (obj = container->content_first; obj != NULL; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name))
		  && can_see_obj (ch, obj))
		{
		  found = true;
		  get_obj (ch, obj, container);
		}
	    }

	  if (!found)
	    {
	      if (arg1[3] == '\0')
		act ("I see nothing in the $T.", ch, NULL, arg2, TO_CHAR);
	      else
		act ("I see nothing like that in the $T.", ch, NULL,
		     arg2, TO_CHAR);
	    }
	}
    }

  return;
}

Do_Fun (do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *container;
  ObjData *obj;
  ObjData *obj_next;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (!str_cmp (arg2, "in") || !str_cmp (arg2, "on"))
    argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      chprintln (ch, "Put what in what?");
      return;
    }

  if (!str_cmp (arg2, "all") || !str_prefix ("all.", arg2))
    {
      chprintln (ch, "You can't do that.");
      return;
    }

  if ((container = get_obj_here (ch, NULL, arg2)) == NULL)
    {
      act ("I see no $T here.", ch, NULL, arg2, TO_CHAR);
      return;
    }

  if (container->item_type != ITEM_CONTAINER)
    {
      chprintln (ch, "That's not a container.");
      return;
    }

  if (IsSet (container->value[1], CONT_CLOSED))
    {
      act ("The $d is closed.", ch, NULL, container->name, TO_CHAR);
      return;
    }

  if (str_cmp (arg1, "all") && str_prefix ("all.", arg1))
    {

      if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
	{
	  chprintln (ch, "You do not have that item.");
	  return;
	}

      if (obj == container)
	{
	  chprintln (ch, "You can't fold it into itself.");
	  return;
	}

      if (!can_drop_obj (ch, obj))
	{
	  chprintln (ch, "You can't let go of it.");
	  return;
	}

      if (IsObjStat (obj, ITEM_QUEST) && !IsObjStat (container, ITEM_QUEST))
	{
	  chprintln (ch, "You can't put a quest item in something.");
	  return;
	}

      if (WeightMult (obj) != 100)
	{
	  chprintln (ch, "You have a feeling that would be a bad idea.");
	  return;
	}

      if (get_obj_weight (obj) + get_true_weight (container) >
	  (container->value[0] * 10) ||
	  get_obj_weight (obj) > (container->value[3] * 10))
	{
	  chprintln (ch, "It won't fit.");
	  return;
	}

      obj_from_char (obj);
      obj_to_obj (obj, container);

      if (IsSet (container->value[1], CONT_PUT_ON))
	{
	  act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
	  act ("You put $p on $P.", ch, obj, container, TO_CHAR);
	}
      else
	{
	  act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
	  act ("You put $p in $P.", ch, obj, container, TO_CHAR);
	}
    }
  else
    {

      for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  if ((arg1[3] == '\0' || is_name (&arg1[4], obj->name)) &&
	      can_see_obj (ch, obj) && WeightMult (obj) == 100 &&
	      obj->wear_loc == WEAR_NONE && obj != container &&
	      can_drop_obj (ch, obj) && (!IsObjStat (obj, ITEM_QUEST)
					 || IsObjStat (container,
						       ITEM_QUEST))
	      && get_obj_weight (obj) + get_true_weight (container) <=
	      (container->value[0] * 10)
	      && get_obj_weight (obj) < (container->value[3] * 10))
	    {
	      obj_from_char (obj);
	      obj_to_obj (obj, container);

	      if (IsSet (container->value[1], CONT_PUT_ON))
		{
		  act ("$n puts $p on $P.", ch, obj, container, TO_ROOM);
		  act ("You put $p on $P.", ch, obj, container, TO_CHAR);
		}
	      else
		{
		  act ("$n puts $p in $P.", ch, obj, container, TO_ROOM);
		  act ("You put $p in $P.", ch, obj, container, TO_CHAR);
		}
	    }
	}
    }

  return;
}

Do_Fun (do_drop)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  ObjData *obj_next;
  bool found;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Drop what?");
      return;
    }

  if (is_number (arg))
    {

      money_t amount, gold = 0, silver = 0;

      amount = atol (arg);
      argument = one_argument (argument, arg);
      if (amount <= 0 ||
	  (str_cmp (arg, "coins") && str_cmp (arg, "coin") &&
	   str_cmp (arg, "gold") && str_cmp (arg, "silver")))
	{
	  chprintln (ch, "Sorry, you can't do that.");
	  return;
	}

      if (!str_cmp (arg, "coins") || !str_cmp (arg, "coin") ||
	  !str_cmp (arg, "silver"))
	{
	  if (ch->silver < amount)
	    {
	      chprintln (ch, "You don't have that much silver.");
	      return;
	    }

	  ch->silver -= amount;
	  silver = amount;
	}
      else
	{
	  if (ch->gold < amount)
	    {
	      chprintln (ch, "You don't have that much gold.");
	      return;
	    }

	  ch->gold -= amount;
	  gold = amount;
	}

      for (obj = ch->in_room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  switch (obj->pIndexData->vnum)
	    {
	    case OBJ_VNUM_SILVER_ONE:
	      silver += 1;
	      extract_obj (obj);
	      break;

	    case OBJ_VNUM_GOLD_ONE:
	      gold += 1;
	      extract_obj (obj);
	      break;

	    case OBJ_VNUM_SILVER_SOME:
	      silver += obj->value[0];
	      extract_obj (obj);
	      break;

	    case OBJ_VNUM_GOLD_SOME:
	      gold += obj->value[1];
	      extract_obj (obj);
	      break;

	    case OBJ_VNUM_COINS:
	      silver += obj->value[0];
	      gold += obj->value[1];
	      extract_obj (obj);
	      break;
	    }
	}

      obj_to_room (create_money (gold, silver), ch->in_room);
      act ("$n drops some coins.", ch, NULL, NULL, TO_ROOM);
      chprintln (ch, "OK.");
      return;
    }

  if (str_cmp (arg, "all") && str_prefix ("all.", arg))
    {

      if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
	{
	  chprintln (ch, "You do not have that item.");
	  return;
	}

      if (!can_drop_obj (ch, obj))
	{
	  chprintln (ch, "You can't let go of it.");
	  return;
	}

      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
      act ("You drop $p.", ch, obj, NULL, TO_CHAR);
      if (HasTriggerObj (obj, TRIG_DROP))
	p_give_trigger (NULL, obj, NULL, ch, obj, TRIG_DROP);
      if (HasTriggerRoom (ch->in_room, TRIG_DROP))
	p_give_trigger (NULL, NULL, ch->in_room, ch, obj, TRIG_DROP);

      if (obj && IsObjStat (obj, ITEM_MELT_DROP))
	{
	  act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
	  act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
	  extract_obj (obj);
	}
    }
  else
    {

      found = false;
      for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  if ((arg[3] == '\0' || is_name (&arg[4], obj->name)) &&
	      can_see_obj (ch, obj) && obj->wear_loc == WEAR_NONE &&
	      can_drop_obj (ch, obj))
	    {
	      found = true;
	      obj_from_char (obj);
	      obj_to_room (obj, ch->in_room);
	      act ("$n drops $p.", ch, obj, NULL, TO_ROOM);
	      act ("You drop $p.", ch, obj, NULL, TO_CHAR);
	      if (HasTriggerObj (obj, TRIG_DROP))
		p_give_trigger (NULL, obj, NULL, ch, obj, TRIG_DROP);
	      if (HasTriggerRoom (ch->in_room, TRIG_DROP))
		p_give_trigger (NULL, NULL, ch->in_room, ch, obj, TRIG_DROP);

	      if (obj && IsObjStat (obj, ITEM_MELT_DROP))
		{
		  act ("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
		  act ("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
		  extract_obj (obj);
		}
	    }
	}

      if (!found)
	{
	  if (arg[3] == '\0')
	    act ("You are not carrying anything.", ch, NULL, arg, TO_CHAR);
	  else
	    act ("You are not carrying any $T.", ch, NULL, &arg[4], TO_CHAR);
	}
    }

  return;
}

Do_Fun (do_give)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CharData *victim;
  ObjData *obj;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      chprintln (ch, "Give what to whom?");
      return;
    }

  if (is_number (arg1))
    {

      money_t amount;
      bool silver;

      amount = atol (arg1);
      if (amount <= 0 ||
	  (str_cmp (arg2, "coins") && str_cmp (arg2, "coin") &&
	   str_cmp (arg2, "gold") && str_cmp (arg2, "silver")))
	{
	  chprintln (ch, "Sorry, you can't do that.");
	  return;
	}

      silver = str_cmp (arg2, "gold");

      argument = one_argument (argument, arg2);
      if (NullStr (arg2))
	{
	  chprintln (ch, "Give what to whom?");
	  return;
	}

      if ((victim = get_char_room (ch, NULL, arg2)) == NULL)
	{
	  chprintln (ch, "They aren't here.");
	  return;
	}

      if ((!silver && ch->gold < amount) || (silver && ch->silver < amount))
	{
	  chprintln (ch, "You haven't got that much.");
	  return;
	}

      if (silver)
	{
	  ch->silver -= amount;
	  victim->silver += amount;
	}
      else
	{
	  ch->gold -= amount;
	  victim->gold += amount;
	}

      sprintf (buf, "$n gives you %ld %s.", amount,
	       silver ? "silver" : "gold");
      act (buf, ch, NULL, victim, TO_VICT);
      act ("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
      sprintf (buf, "You give $N %ld %s.", amount,
	       silver ? "silver" : "gold");
      act (buf, ch, NULL, victim, TO_CHAR);


      if (IsNPC (victim) && HasTriggerMob (victim, TRIG_BRIBE))
	p_bribe_trigger (victim, ch, silver ? amount : amount * 100);

      if (IsNPC (victim) && IsSet (victim->act, ACT_IS_CHANGER))
	{
	  money_t change;

	  change = (silver ? 95 * amount / 100 / 100 : 95 * amount);

	  if (!silver && change > victim->silver)
	    victim->silver += change;

	  if (silver && change > victim->gold)
	    victim->gold += change;

	  if (change < 1 && can_see (victim, ch))
	    {
	      act
		("$n tells you 'I'm sorry, you did not give me enough to change.'",
		 victim, NULL, ch, TO_VICT);
	      ch->reply = victim;
	      sprintf (buf, "%ld %s %s", amount,
		       silver ? "silver" : "gold", ch->name);
	      do_function (victim, &do_give, buf);
	    }
	  else if (can_see (victim, ch))
	    {
	      sprintf (buf, "%ld %s %s", change,
		       silver ? "gold" : "silver", ch->name);
	      do_function (victim, &do_give, buf);
	      if (silver)
		{
		  sprintf (buf, "%ld silver %s",
			   (95 * amount / 100 - change * 100), ch->name);
		  do_function (victim, &do_give, buf);
		}
	      act ("$n tells you 'Thank you, come again.'",
		   victim, NULL, ch, TO_VICT);
	      ch->reply = victim;
	    }
	}
      return;
    }

  if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  if (obj->wear_loc != WEAR_NONE)
    {
      chprintln (ch, "You must remove it first.");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg2)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }


  if (IsQuester (ch)
      && ch->pcdata->quest.status == QUEST_DELIVER
      && ch->pcdata->quest.obj == obj)
    {
      if (ch->pcdata->quest.mob == victim)
	{
	  act ("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
	  act ("$n gives you $p.", ch, obj, victim, TO_VICT);
	  act ("You give $p to $N.", ch, obj, victim, TO_CHAR);

	  chprintln (ch,
		     NEWLINE "{5+RYou have almost completed your QUEST!{x");
	  act ("{RReturn to $N before your time runs out!{x", ch, NULL,
	       ch->pcdata->quest.giver, TO_CHAR);
	  ch->pcdata->quest.status = QUEST_RETURN_DELIVER;

	  obj_from_char (ch->pcdata->quest.obj);
	  extract_obj (ch->pcdata->quest.obj);

	  ch->pcdata->quest.obj = NULL;
	  ch->pcdata->quest.mob = NULL;

	  sprintf (buf, "thank %s", ch->name);
	  interpret (victim, buf);
	  return;
	}
      else
	{
	  act ("That isn't who your supposed to deliver $p too.", ch,
	       ch->pcdata->quest.obj, NULL, TO_CHAR);
	  return;
	}
    }

  if (IsNPC (victim) && victim->pIndexData->pShop != NULL)
    {
      act ("$N tells you 'Sorry, you'll have to sell that.'", ch, NULL,
	   victim, TO_CHAR);
      ch->reply = victim;
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      chprintln (ch, "You can't let go of it.");
      return;
    }

  if (IsObjStat (obj, ITEM_QUEST) && ch->level <= MAX_MORTAL_LEVEL)
    {
      chprintln (ch, "You can't give quest items.");
      return;
    }

  if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
    {
      act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (get_carry_weight (victim) + get_obj_weight (obj) > can_carry_w (victim))
    {
      act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (!can_see_obj (victim, obj))
    {
      act ("$N can't see it.", ch, NULL, victim, TO_CHAR);
      return;
    }

  obj_from_char (obj);
  obj_to_char (obj, victim);
  MOBtrigger = false;
  act ("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
  act ("$n gives you $p.", ch, obj, victim, TO_VICT);
  act ("You give $p to $N.", ch, obj, victim, TO_CHAR);
  MOBtrigger = true;

  if (HasTriggerObj (obj, TRIG_GIVE))
    p_give_trigger (NULL, obj, NULL, ch, obj, TRIG_GIVE);
  if (HasTriggerRoom (ch->in_room, TRIG_GIVE))
    p_give_trigger (NULL, NULL, ch->in_room, ch, obj, TRIG_GIVE);

  if (IsNPC (victim) && HasTriggerMob (victim, TRIG_GIVE))
    p_give_trigger (victim, NULL, NULL, ch, obj, TRIG_GIVE);

  return;
}


Do_Fun (do_envenom)
{
  ObjData *obj;
  AffectData af;
  int percent, skill;


  if (NullStr (argument))
    {
      chprintln (ch, "Envenom what item?");
      return;
    }

  obj = get_obj_list (ch, argument, ch->carrying_first);

  if (obj == NULL)
    {
      chprintln (ch, "You don't have that item.");
      return;
    }

  if ((skill = get_skill (ch, gsn_envenom)) < 1)
    {
      chprintln (ch, "Are you crazy? You'd poison yourself!");
      return;
    }

  if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
      if (IsObjStat (obj, ITEM_BLESS) || IsObjStat (obj, ITEM_BURN_PROOF))
	{
	  act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      if (number_percent () < skill)
	{
	  act ("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
	  act ("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
	  if (!obj->value[3])
	    {
	      obj->value[3] = 1;
	      check_improve (ch, gsn_envenom, true, 4);
	    }
	  WaitState (ch, skill_table[gsn_envenom].beats);
	  return;
	}

      act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
      if (!obj->value[3])
	check_improve (ch, gsn_envenom, false, 4);
      WaitState (ch, skill_table[gsn_envenom].beats);
      return;
    }

  if (obj->item_type == ITEM_WEAPON)
    {
      if (IsWeaponStat (obj, WEAPON_FLAMING) ||
	  IsWeaponStat (obj, WEAPON_FROST) ||
	  IsWeaponStat (obj, WEAPON_VAMPIRIC) ||
	  IsWeaponStat (obj, WEAPON_SHARP) ||
	  IsWeaponStat (obj, WEAPON_VORPAL) ||
	  IsWeaponStat (obj, WEAPON_SHOCKING) ||
	  IsObjStat (obj, ITEM_BLESS) || IsObjStat (obj, ITEM_BURN_PROOF))
	{
	  act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      if (obj->value[3] < 0 || attack_table[obj->value[3]].damage == DAM_BASH)
	{
	  chprintln (ch, "You can only envenom edged weapons.");
	  return;
	}

      if (IsWeaponStat (obj, WEAPON_POISON))
	{
	  act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      percent = number_percent ();
      if (percent < skill)
	{

	  af.where = TO_WEAPON;
	  af.type = gsn_poison;
	  af.level = ch->level * percent / 100;
	  af.duration = ch->level / 2 * percent / 100;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = WEAPON_POISON;
	  affect_to_obj (obj, &af);

	  act ("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
	  act ("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
	  check_improve (ch, gsn_envenom, true, 3);
	  WaitState (ch, skill_table[gsn_envenom].beats);
	  return;
	}
      else
	{
	  act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
	  check_improve (ch, gsn_envenom, false, 3);
	  WaitState (ch, skill_table[gsn_envenom].beats);
	  return;
	}
    }

  act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
  return;
}

Do_Fun (do_fill)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  ObjData *obj;
  ObjData *fountain;
  bool found;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Fill what?");
      return;
    }

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  found = false;
  for (fountain = ch->in_room->content_first; fountain != NULL;
       fountain = fountain->next_content)
    {
      if (fountain->item_type == ITEM_FOUNTAIN)
	{
	  found = true;
	  break;
	}
    }

  if (!found)
    {
      chprintln (ch, "There is no fountain here!");
      return;
    }

  if (obj->item_type != ITEM_DRINK_CON)
    {
      chprintln (ch, "You can't fill that.");
      return;
    }

  if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
    {
      chprintln (ch, "There is already another liquid in it.");
      return;
    }

  if (obj->value[1] >= obj->value[0])
    {
      chprintln (ch, "Your container is full.");
      return;
    }

  sprintf (buf, "You fill $p with %s from $P.",
	   liq_table[fountain->value[2]].liq_name);
  act (buf, ch, obj, fountain, TO_CHAR);
  sprintf (buf, "$n fills $p with %s from $P.",
	   liq_table[fountain->value[2]].liq_name);
  act (buf, ch, obj, fountain, TO_ROOM);
  obj->value[2] = fountain->value[2];
  obj->value[1] = obj->value[0];
  return;
}

Do_Fun (do_pour)
{
  char arg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  ObjData *out, *in;
  CharData *vch = NULL;
  int amount;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      chprintln (ch, "Pour what into what?");
      return;
    }

  if ((out = get_obj_carry (ch, arg, ch)) == NULL)
    {
      chprintln (ch, "You don't have that item.");
      return;
    }

  if (out->item_type != ITEM_DRINK_CON)
    {
      chprintln (ch, "That's not a drink container.");
      return;
    }

  if (!str_cmp (argument, "out"))
    {
      if (out->value[1] == 0)
	{
	  chprintln (ch, "It's already empty.");
	  return;
	}

      out->value[1] = 0;
      out->value[3] = 0;
      sprintf (buf, "You invert $p, spilling %s all over the ground.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, NULL, TO_CHAR);

      sprintf (buf, "$n inverts $p, spilling %s all over the ground.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, NULL, TO_ROOM);
      return;
    }

  if ((in = get_obj_here (ch, NULL, argument)) == NULL)
    {
      vch = get_char_room (ch, NULL, argument);

      if (vch == NULL)
	{
	  chprintln (ch, "Pour into what?");
	  return;
	}

      in = get_eq_char (vch, WEAR_HOLD);

      if (in == NULL)
	{
	  chprintln (ch, "They aren't holding anything.");
	  return;
	}
    }

  if (in->item_type != ITEM_DRINK_CON)
    {
      chprintln (ch, "You can only pour into other drink containers.");
      return;
    }

  if (in == out)
    {
      chprintln (ch, "You cannot change the laws of physics!");
      return;
    }

  if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
      chprintln (ch, "They don't hold the same liquid.");
      return;
    }

  if (out->value[1] == 0)
    {
      act ("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
      return;
    }

  if (in->value[1] >= in->value[0])
    {
      act ("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
      return;
    }

  amount = Min (out->value[1], in->value[0] - in->value[1]);

  in->value[1] += amount;
  out->value[1] -= amount;
  in->value[2] = out->value[2];

  if (vch == NULL)
    {
      sprintf (buf, "You pour %s from $p into $P.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, in, TO_CHAR);
      sprintf (buf, "$n pours %s from $p into $P.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, out, in, TO_ROOM);
    }
  else
    {
      sprintf (buf, "You pour some %s for $N.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_CHAR);
      sprintf (buf, "$n pours you some %s.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_VICT);
      sprintf (buf, "$n pours some %s for $N.",
	       liq_table[out->value[2]].liq_name);
      act (buf, ch, NULL, vch, TO_NOTVICT);

    }
}

Do_Fun (do_drink)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int amount;
  int liquid;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      for (obj = ch->in_room->content_first; obj; obj = obj->next_content)
	{
	  if (obj->item_type == ITEM_FOUNTAIN)
	    break;
	}

      if (obj == NULL)
	{
	  chprintln (ch, "Drink what?");
	  return;
	}
    }
  else
    {
      if ((obj = get_obj_here (ch, NULL, arg)) == NULL)
	{
	  chprintln (ch, "You can't find it.");
	  return;
	}
    }

  if (!IsNPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    {
      chprintln (ch, "You fail to reach your mouth.  *Hic*");
      return;
    }

  switch (obj->item_type)
    {
    default:
      chprintln (ch, "You can't drink from that.");
      return;

    case ITEM_FOUNTAIN:
      if ((liquid = obj->value[2]) < 0)
	{
	  bugf ("Do_drink: bad liquid number %d.", liquid);
	  liquid = obj->value[2] = 0;
	}
      amount = liq_table[liquid].liq_affect[4] * 3;
      break;

    case ITEM_DRINK_CON:
      if (obj->value[1] <= 0)
	{
	  chprintln (ch, "It is already empty.");
	  return;
	}

      if ((liquid = obj->value[2]) < 0)
	{
	  bugf ("Do_drink: bad liquid number %d.", liquid);
	  liquid = obj->value[2] = 0;
	}

      amount = liq_table[liquid].liq_affect[4];
      amount = Min (amount, obj->value[1]);
      break;
    }
  if (!IsNPC (ch) && !IsImmortal (ch) &&
      ch->pcdata->condition[COND_FULL] > 45)
    {
      chprintln (ch, "You're too full to drink more.");
      return;
    }

  act ("$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM);
  act ("You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR);

  gain_condition (ch, COND_DRUNK,
		  amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36);
  gain_condition (ch, COND_FULL,
		  amount * liq_table[liquid].liq_affect[COND_FULL] / 4);
  gain_condition (ch, COND_THIRST,
		  amount * liq_table[liquid].liq_affect[COND_THIRST] / 10);
  gain_condition (ch, COND_HUNGER,
		  amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2);

  if (!IsNPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    chprintln (ch, "You feel drunk.");
  if (!IsNPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
    chprintln (ch, "You are full.");
  if (!IsNPC (ch) && ch->pcdata->condition[COND_THIRST] > 40)
    chprintln (ch, "Your thirst is quenched.");

  if (obj->value[3] != 0)
    {

      AffectData af;

      act ("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
      chprintln (ch, "You choke and gag.");
      af.where = TO_AFFECTS;
      af.type = gsn_poison;
      af.level = number_fuzzy (amount);
      af.duration = 3 * amount;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_POISON;
      affect_join (ch, &af);
    }

  if (obj->value[0] > 0)
    obj->value[1] -= amount;

  return;
}

Do_Fun (do_eat)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Eat what?");
      return;
    }

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  if (!IsImmortal (ch))
    {
      if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL)
	{
	  chprintln (ch, "That's not edible.");
	  return;
	}

      if (!IsNPC (ch) && ch->pcdata->condition[COND_FULL] > 40)
	{
	  chprintln (ch, "You are too full to eat more.");
	  return;
	}
    }

  act ("$n eats $p.", ch, obj, NULL, TO_ROOM);
  act ("You eat $p.", ch, obj, NULL, TO_CHAR);

  switch (obj->item_type)
    {

    case ITEM_FOOD:
      if (!IsNPC (ch))
	{
	  int condition;

	  condition = ch->pcdata->condition[COND_HUNGER];
	  gain_condition (ch, COND_FULL, obj->value[0]);
	  gain_condition (ch, COND_HUNGER, obj->value[1]);
	  if (condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0)
	    chprintln (ch, "You are no longer hungry.");
	  else if (ch->pcdata->condition[COND_FULL] > 40)
	    chprintln (ch, "You are full.");
	}

      if (obj->value[3] != 0)
	{

	  AffectData af;

	  act ("$n chokes and gags.", ch, 0, 0, TO_ROOM);
	  chprintln (ch, "You choke and gag.");

	  af.where = TO_AFFECTS;
	  af.type = gsn_poison;
	  af.level = number_fuzzy (obj->value[0]);
	  af.duration = 2 * obj->value[0];
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = AFF_POISON;
	  affect_join (ch, &af);
	}
      break;

    case ITEM_PILL:
      obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
      obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);
      break;
    default:
      break;

    }

  extract_obj (obj);
  return;
}


bool
remove_obj (CharData * ch, wloc_t iWear, bool fReplace)
{
  ObjData *obj;

  if ((obj = get_eq_char (ch, iWear)) == NULL)
    return true;

  if (!fReplace)
    return false;

  if (IsSet (obj->extra_flags, ITEM_NOREMOVE))
    {
      act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return false;
    }

  unequip_char (ch, obj);
  act ("$n stops using $p.", ch, obj, NULL, TO_ROOM);
  act ("You stop using $p.", ch, obj, NULL, TO_CHAR);
  return true;
}


void
wear_obj (CharData * ch, ObjData * obj, bool fReplace)
{

  if (ch->level < obj->level)
    {
      chprintlnf (ch, "You must be level %d to use this object.", obj->level);
      act ("$n tries to use $p, but is too inexperienced.", ch, obj, NULL,
	   TO_ROOM);
      return;
    }

  if (obj->item_type == ITEM_LIGHT)
    {
      if (!remove_obj (ch, WEAR_LIGHT, fReplace))
	return;
      act ("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
      act ("You light $p and hold it.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_LIGHT);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_FINGER))
    {
      if (get_eq_char (ch, WEAR_FINGER_L) != NULL &&
	  get_eq_char (ch, WEAR_FINGER_R) != NULL &&
	  !remove_obj (ch, WEAR_FINGER_L, fReplace) &&
	  !remove_obj (ch, WEAR_FINGER_R, fReplace))
	return;

      if (get_eq_char (ch, WEAR_FINGER_L) == NULL)
	{
	  act ("$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, WEAR_FINGER_L);
	  return;
	}

      if (get_eq_char (ch, WEAR_FINGER_R) == NULL)
	{
	  act ("$n wears $p on $s right finger.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, WEAR_FINGER_R);
	  return;
	}

      bug ("Wear_obj: no free finger.");
      chprintln (ch, "You already wear two rings.");
      return;
    }

  if (CanWear (obj, ITEM_WEAR_NECK))
    {
      if (get_eq_char (ch, WEAR_NECK_1) != NULL &&
	  get_eq_char (ch, WEAR_NECK_2) != NULL &&
	  !remove_obj (ch, WEAR_NECK_1, fReplace) &&
	  !remove_obj (ch, WEAR_NECK_2, fReplace))
	return;

      if (get_eq_char (ch, WEAR_NECK_1) == NULL)
	{
	  act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, WEAR_NECK_1);
	  return;
	}

      if (get_eq_char (ch, WEAR_NECK_2) == NULL)
	{
	  act ("$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, WEAR_NECK_2);
	  return;
	}

      bug ("Wear_obj: no free neck.");
      chprintln (ch, "You already wear two neck items.");
      return;
    }

  if (CanWear (obj, ITEM_WEAR_BODY))
    {
      if (!remove_obj (ch, WEAR_BODY, fReplace))
	return;
      act ("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your torso.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_BODY);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_HEAD))
    {
      if (!remove_obj (ch, WEAR_HEAD, fReplace))
	return;
      act ("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your head.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_HEAD);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_LEGS))
    {
      if (!remove_obj (ch, WEAR_LEGS, fReplace))
	return;
      act ("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_LEGS);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_FEET))
    {
      if (!remove_obj (ch, WEAR_FEET, fReplace))
	return;
      act ("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your feet.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_FEET);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_HANDS))
    {
      if (!remove_obj (ch, WEAR_HANDS, fReplace))
	return;
      act ("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your hands.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_HANDS);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_ARMS))
    {
      if (!remove_obj (ch, WEAR_ARMS, fReplace))
	return;
      act ("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p on your arms.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_ARMS);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_ABOUT))
    {
      if (!remove_obj (ch, WEAR_ABOUT, fReplace))
	return;
      act ("$n wears $p about $s torso.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p about your torso.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_ABOUT);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_WAIST))
    {
      if (!remove_obj (ch, WEAR_WAIST, fReplace))
	return;
      act ("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p about your waist.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_WAIST);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_WRIST))
    {
      if (get_eq_char (ch, WEAR_WRIST_L) != NULL &&
	  get_eq_char (ch, WEAR_WRIST_R) != NULL &&
	  !remove_obj (ch, WEAR_WRIST_L, fReplace) &&
	  !remove_obj (ch, WEAR_WRIST_R, fReplace))
	return;

      if (get_eq_char (ch, WEAR_WRIST_L) == NULL)
	{
	  act ("$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR);
	  equip_char (ch, obj, WEAR_WRIST_L);
	  return;
	}

      if (get_eq_char (ch, WEAR_WRIST_R) == NULL)
	{
	  act ("$n wears $p around $s right wrist.", ch, obj, NULL, TO_ROOM);
	  act ("You wear $p around your right wrist.", ch, obj, NULL,
	       TO_CHAR);
	  equip_char (ch, obj, WEAR_WRIST_R);
	  return;
	}

      bug ("Wear_obj: no free wrist.");
      chprintln (ch, "You already wear two wrist items.");
      return;
    }

  if (CanWear (obj, ITEM_WEAR_SHIELD))
    {
      ObjData *weapon;

      if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
	{
	  chprintln (ch, "You cannot use a shield while using 2 weapons.");
	  return;
	}

      if (!remove_obj (ch, WEAR_SHIELD, fReplace))
	return;

      weapon = get_eq_char (ch, WEAR_WIELD);
      if (weapon != NULL && ch->size < SIZE_LARGE &&
	  IsWeaponStat (weapon, WEAPON_TWO_HANDS))
	{
	  chprintln (ch, "Your hands are tied up with your weapon!");
	  return;
	}

      act ("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
      act ("You wear $p as a shield.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_SHIELD);
      return;
    }

  if (CanWear (obj, ITEM_WIELD))
    {
      int sn, skill;

      if (!remove_obj (ch, WEAR_WIELD, fReplace))
	return;

      if (!IsNPC (ch) &&
	  get_obj_weight (obj) >
	  (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
	{
	  chprintln (ch, "It is too heavy for you to wield.");
	  return;
	}

      if (!IsNPC (ch) && ch->size < SIZE_LARGE &&
	  IsWeaponStat (obj, WEAPON_TWO_HANDS) &&
	  get_eq_char (ch, WEAR_SHIELD) != NULL)
	{
	  chprintln (ch, "You need two hands free for that weapon.");
	  return;
	}

      act ("$n wields $p.", ch, obj, NULL, TO_ROOM);
      act ("You wield $p.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_WIELD);

      sn = get_weapon_sn (ch);

      if (sn == gsn_hand_to_hand)
	return;

      skill = get_weapon_skill (ch, sn);

      if (skill >= 100)
	act ("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);
      else if (skill > 85)
	act ("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);
      else if (skill > 70)
	act ("You are skilled with $p.", ch, obj, NULL, TO_CHAR);
      else if (skill > 50)
	act ("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);
      else if (skill > 25)
	act ("$p feels a little clumsy in your hands.", ch, obj, NULL,
	     TO_CHAR);
      else if (skill > 1)
	act ("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);
      else
	act ("You don't even know which end is up on $p.", ch, obj,
	     NULL, TO_CHAR);

      return;
    }

  if (CanWear (obj, ITEM_HOLD))
    {

      if (get_eq_char (ch, WEAR_SECONDARY) != NULL)
	{
	  chprintln (ch, "You cannot hold an item while using 2 weapons.");
	  return;
	}

      if (!remove_obj (ch, WEAR_HOLD, fReplace))
	return;
      act ("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
      act ("You hold $p in your hand.", ch, obj, NULL, TO_CHAR);
      equip_char (ch, obj, WEAR_HOLD);
      return;
    }

  if (CanWear (obj, ITEM_WEAR_FLOAT))
    {
      if (!remove_obj (ch, WEAR_FLOAT, fReplace))
	return;
      act ("$n releases $p to float next to $m.", ch, obj, NULL, TO_ROOM);
      act ("You release $p and it floats next to you.", ch, obj, NULL,
	   TO_CHAR);
      equip_char (ch, obj, WEAR_FLOAT);
      return;
    }

  if (fReplace)
    chprintln (ch, "You can't wear, wield, or hold that.");

  return;
}

Do_Fun (do_wear)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Wear, wield, or hold what?");
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      ObjData *obj_next;

      for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
	    wear_obj (ch, obj, false);
	}
      return;
    }
  else
    {
      if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
	{
	  chprintln (ch, "You do not have that item.");
	  return;
	}

      wear_obj (ch, obj, true);
    }

  return;
}

Do_Fun (do_remove)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Remove what?");
      return;
    }

  if (!str_cmp (arg, "all") || !str_prefix ("all.", arg))
    {
      ObjData *obj_next;

      for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (obj->wear_loc != WEAR_NONE && can_see_obj (ch, obj)
	      && (arg[3] == '\0' || is_name (&arg[4], obj->name)))
	    remove_obj (ch, obj->wear_loc, true);
	}
      return;
    }

  if ((obj = get_obj_wear (ch, arg, true)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  remove_obj (ch, obj->wear_loc, true);
  return;
}

Do_Fun (do_sacrifice)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  money_t silver;


  CharData *gch;
  int members;
  char buffer[100];

  one_argument (argument, arg);

  if (NullStr (arg) || !str_cmp (arg, ch->name))
    {
      act ("$n offers $mself to $g, who graciously declines.", ch,
	   NULL, NULL, TO_ROOM);
      act ("$g appreciates your offer and may accept it later.", ch, NULL,
	   NULL, TO_CHAR);
      return;
    }

  if (is_donate_room (ch->in_room->vnum))
    {
      chprintln (ch, "You can't sacrifice items in this room!");
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      ObjData *next;

      for (obj = ch->in_room->content_first; obj; obj = next)
	{
	  next = obj->next_content;
	  do_sacrifice (n_fun, ch, obj->name);
	}
      return;
    }
  obj = get_obj_list (ch, arg, ch->in_room->content_first);
  if (obj == NULL)
    {
      chprintln (ch, "You can't find it.");
      return;
    }

  if (obj->item_type == ITEM_CORPSE_PC)
    {
      if (obj->content_first)
	{
	  act ("$g wouldn't like that.", ch, NULL, NULL, TO_CHAR);
	  return;
	}
    }

  if (!CanWear (obj, ITEM_TAKE) || CanWear (obj, ITEM_NO_SAC))
    {
      act ("$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);
      return;
    }

  if (obj->in_room != NULL)
    {
      for (gch = obj->in_room->person_first; gch != NULL;
	   gch = gch->next_in_room)
	if (gch->on == obj)
	  {
	    act ("$N appears to be using $p.", ch, obj, gch, TO_CHAR);
	    return;
	  }
    }

  silver = Max (1, obj->level * 3);

  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = Min (silver, obj->cost);

  if (silver == 1)
    act ("$g gives you one silver coin for your sacrifice.", ch, NULL,
	 NULL, TO_CHAR);
  else
    {
      chprintlnf (ch,
		  "%s gives you %d silver coins for your sacrifice.",
		  ch->deity == NULL ? "Mota" : ch->deity->name, silver);
    }

  ch->silver += silver;

  if (IsSet (ch->act, PLR_AUTOSPLIT))
    {
      members = 0;
      for (gch = ch->in_room->person_first; gch != NULL;
	   gch = gch->next_in_room)
	{
	  if (is_same_group (gch, ch))
	    members++;
	}

      if (members > 1 && silver > 1)
	{
	  sprintf (buffer, "%ld", silver);
	  do_function (ch, &do_split, buffer);
	}
    }

  act ("$n sacrifices $p to $g.", ch, obj, NULL, TO_ROOM);
  wiznet ("$N sends up $p as a burnt offering.", ch, obj, WIZ_SACCING,
	  false, 0);
  extract_obj (obj);
  return;
}

Do_Fun (do_quaff)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Quaff what?");
      return;
    }


  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    {
      act
	("You try to drink from $p, but feel to many eyes watching you.",
	 ch, NULL, NULL, TO_CHAR);
      return;
    }

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      chprintln (ch, "You do not have that potion.");
      return;
    }

  if (obj->item_type != ITEM_POTION)
    {
      chprintln (ch, "You can quaff only potions.");
      return;
    }

  if (ch->level < obj->level)
    {
      chprintln (ch, "This liquid is too powerful for you to drink.");
      return;
    }

  act ("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
  act ("You quaff $p.", ch, obj, NULL, TO_CHAR);

  obj_cast_spell (obj->value[1], obj->value[0], ch, ch, NULL);
  obj_cast_spell (obj->value[2], obj->value[0], ch, ch, NULL);
  obj_cast_spell (obj->value[3], obj->value[0], ch, ch, NULL);

  extract_obj (obj);
  return;
}

Do_Fun (do_recite)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *scroll;
  ObjData *obj;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if ((scroll = get_obj_carry (ch, arg1, ch)) == NULL)
    {
      chprintln (ch, "You do not have that scroll.");
      return;
    }

  if (scroll->item_type != ITEM_SCROLL)
    {
      chprintln (ch, "You can recite only scrolls.");
      return;
    }

  if (ch->level < scroll->level)
    {
      chprintln (ch, "This scroll is too complex for you to comprehend.");
      return;
    }

  obj = NULL;
  if (NullStr (arg2))
    {
      victim = ch;
    }
  else
    {
      if ((victim = get_char_room (ch, NULL, arg2)) == NULL &&
	  (obj = get_obj_here (ch, NULL, arg2)) == NULL)
	{
	  chprintln (ch, "You can't find it.");
	  return;
	}
    }

  act ("$n recites $p.", ch, scroll, NULL, TO_ROOM);
  act ("You recite $p.", ch, scroll, NULL, TO_CHAR);

  if (number_percent () >= 20 + get_skill (ch, gsn_scrolls) * 4 / 5)
    {
      chprintln (ch, "You mispronounce a syllable.");
      check_improve (ch, gsn_scrolls, false, 2);
    }
  else
    {
      obj_cast_spell (scroll->value[1], scroll->value[0], ch, victim, obj);
      obj_cast_spell (scroll->value[2], scroll->value[0], ch, victim, obj);
      obj_cast_spell (scroll->value[3], scroll->value[0], ch, victim, obj);
      check_improve (ch, gsn_scrolls, true, 2);
    }

  extract_obj (scroll);
  return;
}

Do_Fun (do_brandish)
{
  CharData *vch;
  CharData *vch_next;
  ObjData *staff;
  int sn;

  if ((staff = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
      chprintln (ch, "You hold nothing in your hand.");
      return;
    }

  if (staff->item_type != ITEM_STAFF)
    {
      chprintln (ch, "You can brandish only with a staff.");
      return;
    }

  if ((sn = staff->value[3]) < 0 || sn >= top_skill ||
      skill_table[sn].spell_fun == 0)
    {
      bugf ("Do_brandish: bad sn %d.", sn);
      return;
    }

  WaitState (ch, 2 * PULSE_VIOLENCE);

  if (staff->value[2] > 0)
    {
      act ("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
      act ("You brandish $p.", ch, staff, NULL, TO_CHAR);
      if (ch->level < staff->level ||
	  number_percent () >= 20 + get_skill (ch, gsn_staves) * 4 / 5)
	{
	  act ("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
	  act ("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
	  check_improve (ch, gsn_staves, false, 2);
	}
      else
	for (vch = ch->in_room->person_first; vch; vch = vch_next)
	  {
	    vch_next = vch->next_in_room;

	    switch (skill_table[sn].target)
	      {
	      default:
		bugf ("Do_brandish: bad target for sn %d.", sn);
		return;

	      case TAR_IGNORE:
		if (vch != ch)
		  continue;
		break;

	      case TAR_CHAR_OFFENSIVE:
		if (IsNPC (ch) ? IsNPC (vch) : !IsNPC (vch))
		  continue;
		break;

	      case TAR_CHAR_DEFENSIVE:
		if (IsNPC (ch) ? !IsNPC (vch) : IsNPC (vch))
		  continue;
		break;

	      case TAR_CHAR_SELF:
		if (vch != ch)
		  continue;
		break;
	      }

	    obj_cast_spell (staff->value[3], staff->value[0], ch, vch, NULL);
	    check_improve (ch, gsn_staves, true, 2);
	  }
    }

  if (--staff->value[2] <= 0)
    {
      act ("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
      act ("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
      extract_obj (staff);
    }

  return;
}

Do_Fun (do_zap)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *wand;
  ObjData *obj;

  one_argument (argument, arg);
  if (NullStr (arg) && ch->fighting == NULL)
    {
      chprintln (ch, "Zap whom or what?");
      return;
    }

  if ((wand = get_eq_char (ch, WEAR_HOLD)) == NULL)
    {
      chprintln (ch, "You hold nothing in your hand.");
      return;
    }

  if (wand->item_type != ITEM_WAND)
    {
      chprintln (ch, "You can zap only with a wand.");
      return;
    }

  obj = NULL;
  if (NullStr (arg))
    {
      if (ch->fighting != NULL)
	{
	  victim = ch->fighting;
	}
      else
	{
	  chprintln (ch, "Zap whom or what?");
	  return;
	}
    }
  else
    {
      if ((victim = get_char_room (ch, NULL, arg)) == NULL &&
	  (obj = get_obj_here (ch, NULL, arg)) == NULL)
	{
	  chprintln (ch, "You can't find it.");
	  return;
	}
    }

  WaitState (ch, 2 * PULSE_VIOLENCE);

  if (wand->value[2] > 0)
    {
      if (victim != NULL)
	{
	  act ("$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT);
	  act ("You zap $N with $p.", ch, wand, victim, TO_CHAR);
	  act ("$n zaps you with $p.", ch, wand, victim, TO_VICT);
	}
      else
	{
	  act ("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
	  act ("You zap $P with $p.", ch, wand, obj, TO_CHAR);
	}

      if (ch->level < wand->level ||
	  number_percent () >= 20 + get_skill (ch, gsn_wands) * 4 / 5)
	{
	  act ("Your efforts with $p produce only smoke and sparks.",
	       ch, wand, NULL, TO_CHAR);
	  act ("$n's efforts with $p produce only smoke and sparks.",
	       ch, wand, NULL, TO_ROOM);
	  check_improve (ch, gsn_wands, false, 2);
	}
      else
	{
	  obj_cast_spell (wand->value[3], wand->value[0], ch, victim, obj);
	  check_improve (ch, gsn_wands, true, 2);
	}
    }

  if (--wand->value[2] <= 0)
    {
      act ("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
      act ("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
      extract_obj (wand);
    }

  return;
}

Do_Fun (do_steal)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *obj;
  int percent;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      chprintln (ch, "Steal what from whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg2)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "That's pointless.");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsSet (victim->in_room->room_flags, ROOM_ARENA))
    {
      chprintln (ch, "You are here to do battle, not to steal!");
      return;
    }

  if (IsNPC (victim) && victim->position == POS_FIGHTING)
    {
      chprintlnf (ch, "Kill stealing is not permitted." NEWLINE
		  "You'd better not -- you might get hit.");
      return;
    }

  WaitState (ch, skill_table[gsn_steal].beats);
  percent = number_percent ();

  if (!IsAwake (victim))
    percent -= 10;
  else if (!can_see (victim, ch))
    percent += 25;
  else
    percent += 50;

  if (((ch->level + 7 < victim->level || ch->level - 7 > victim->level)
       && !IsNPC (victim) && !IsNPC (ch)) || (!IsNPC (ch) &&
					      percent >
					      get_skill (ch,
							 gsn_steal))
      || (!IsNPC (ch) && !is_clan (ch)))
    {

      chprintln (ch, "Oops.");
      affect_strip (ch, gsn_sneak);
      RemBit (ch->affected_by, AFF_SNEAK);

      act ("$n tried to steal from you.", ch, NULL, victim, TO_VICT);
      act ("$n tried to steal from $N.", ch, NULL, victim, TO_NOTVICT);
      switch (number_range (0, 3))
	{
	case 0:
	  sprintf (buf, "%s is a lousy thief!", ch->name);
	  break;
	case 1:
	  sprintf (buf,
		   "%s couldn't rob %s way out of a paper bag!",
		   ch->name, (ch->sex == 2) ? "her" : "his");
	  break;
	case 2:
	  sprintf (buf, "%s tried to rob me!", ch->name);
	  break;
	case 3:
	  sprintf (buf, "Keep your hands out of there, %s!", ch->name);
	  break;
	}
      if (!IsAwake (victim))
	do_function (victim, &do_wake, "");
      if (IsAwake (victim))
	do_function (victim, &do_yell, buf);
      if (!IsNPC (ch))
	{
	  if (IsNPC (victim))
	    {
	      check_improve (ch, gsn_steal, false, 2);
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	    }
	  else
	    {
	      new_wiznet (ch, NULL, WIZ_FLAGS, true, 0,
			  "$N tried to steal from %s.", GetName (victim));
	      if (!IsSet (ch->act, PLR_THIEF))
		{
		  SetBit (ch->act, PLR_THIEF);
		  chprintln (ch, "*** You are now a THIEF!! ***");
		  save_char_obj (ch);
		}
	    }
	}

      return;
    }

  if (!str_cmp (arg1, "coin") || !str_cmp (arg1, "coins") ||
      !str_cmp (arg1, "gold") || !str_cmp (arg1, "silver"))
    {
      money_t gold, silver;

      gold = victim->gold * number_range (1, ch->level) / MAX_LEVEL;
      silver = victim->silver * number_range (1, ch->level) / MAX_LEVEL;
      if (gold <= 0 && silver <= 0)
	{
	  chprintln (ch, "You couldn't get any coins.");
	  return;
	}

      add_cost (ch, gold, VALUE_GOLD);
      add_cost (ch, silver, VALUE_SILVER);
      deduct_cost (victim, silver, VALUE_SILVER);
      deduct_cost (victim, gold, VALUE_GOLD);

      if (silver <= 0)
	chprintlnf (ch, "Bingo!  You got %d gold coins.", gold);
      else if (gold <= 0)
	chprintlnf (ch, "Bingo!  You got %d silver coins.", silver);
      else
	chprintlnf (ch,
		    "Bingo!  You got %d silver and %d gold coins.",
		    silver, gold);

      check_improve (ch, gsn_steal, true, 2);
      return;
    }

  if ((obj = get_obj_carry (victim, arg1, ch)) == NULL)
    {
      chprintln (ch, "You can't find it.");
      return;
    }

  if (!can_drop_obj (ch, obj) ||
      IsSet (obj->extra_flags, ITEM_INVENTORY) || obj->level > ch->level)
    {
      chprintln (ch, "You can't pry it away.");
      return;
    }

  if (ch->carry_number + get_obj_number (obj) > can_carry_n (ch))
    {
      chprintln (ch, "You have your hands full.");
      return;
    }

  if (ch->carry_weight + get_obj_weight (obj) > can_carry_w (ch))
    {
      chprintln (ch, "You can't carry that much weight.");
      return;
    }

  obj_from_char (obj);
  obj_to_char (obj, ch);
  act ("You pocket $p.", ch, obj, NULL, TO_CHAR);
  check_improve (ch, gsn_steal, true, 2);
  chprintln (ch, "Got it!");
  return;
}


CharData *
find_keeper (CharData * ch)
{

  CharData *keeper;
  ShopData *pShop;

  pShop = NULL;
  for (keeper = ch->in_room->person_first; keeper;
       keeper = keeper->next_in_room)
    {
      if (IsNPC (keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
	break;
    }

  if (pShop == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return NULL;
    }



  if (time_info.hour < pShop->open_hour)
    {
      do_function (keeper, &do_say, "Sorry, I am closed. Come back later.");
      return NULL;
    }

  if (time_info.hour > pShop->close_hour)
    {
      do_function (keeper, &do_say,
		   "Sorry, I am closed. Come back tomorrow.");
      return NULL;
    }


  if (!can_see (keeper, ch))
    {
      do_function (keeper, &do_say, "I don't trade with folks I can't see.");
      return NULL;
    }

  return keeper;
}


ObjData *
get_obj_keeper (CharData * ch, CharData * keeper, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = keeper->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE && can_see_obj (keeper, obj)
	  && can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;


	  while (obj->next_content != NULL &&
		 obj->pIndexData == obj->next_content->pIndexData &&
		 !str_cmp (obj->short_descr, obj->next_content->short_descr))
	    obj = obj->next_content;
	}
    }

  return NULL;
}

money_t
get_cost (CharData * keeper, ObjData * obj, bool fBuy)
{
  ShopData *pShop;
  money_t cost;

  if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
    return 0;

  if (fBuy)
    {
      cost = obj->cost * pShop->profit_buy / 100;
    }
  else
    {
      ObjData *obj2;
      int itype;

      cost = 0;
      for (itype = 0; itype < MAX_TRADE; itype++)
	{
	  if (obj->item_type == pShop->buy_type[itype])
	    {
	      cost = obj->cost * pShop->profit_sell / 100;
	      break;
	    }
	}

      if (!IsObjStat (obj, ITEM_SELL_EXTRACT))
	{
	  for (obj2 = keeper->carrying_first; obj2; obj2 = obj2->next_content)
	    {
	      if (obj->pIndexData == obj2->pIndexData
		  && !str_cmp (obj->short_descr, obj2->short_descr))
		{
		  if (IsObjStat (obj2, ITEM_INVENTORY))
		    cost /= 2;
		  else
		    cost = cost * 3 / 4;
		}
	    }
	}
    }

  if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
      if (obj->value[1] == 0)
	cost /= 4;
      else
	cost = cost * obj->value[2] / obj->value[1];
    }

  return cost;
}

Do_Fun (do_buy)
{
  char buf[MAX_STRING_LENGTH];
  money_t cost;
  int roll;

  if (NullStr (argument))
    {
      chprintln (ch, "Buy what?");
      return;
    }

  if (IsSet (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      char arg[MAX_INPUT_LENGTH];
      CharData *pet;
      RoomIndex *pRoomIndexNext;
      RoomIndex *in_room;

      if (IsNPC (ch))
	return;

      argument = one_argument (argument, arg);


      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);
      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bugf ("Do_buy: bad pet shop at vnum %ld.", ch->in_room->vnum);
	  chprintln (ch, "Sorry, you can't buy that here.");
	  return;
	}

      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      pet = get_char_room (ch, NULL, arg);
      ch->in_room = in_room;

      if (pet == NULL || !IsSet (pet->act, ACT_PET))
	{
	  chprintln (ch, "Sorry, you can't buy that here.");
	  return;
	}

      if (ch->pet != NULL)
	{
	  chprintln (ch, "You already own a pet.");
	  return;
	}

      cost = 10 * pet->level * pet->level;

      if (!check_worth (ch, cost, VALUE_DEFAULT))
	{
	  chprintln (ch, "You can't afford it.");
	  return;
	}

      if (ch->level < pet->level)
	{
	  chprintln (ch, "You're not powerful enough to master this pet.");
	  return;
	}


      roll = number_percent ();
      if (roll < get_skill (ch, gsn_haggle))
	{
	  cost -= cost / 2 * roll / 100;
	  chprintlnf (ch, "You haggle the price down to %ld coins.", cost);
	  check_improve (ch, gsn_haggle, true, 4);

	}

      deduct_cost (ch, cost, VALUE_DEFAULT);
      pet = create_mobile (pet->pIndexData);
      SetBit (pet->act, ACT_PET);
      SetBit (pet->affected_by, AFF_CHARM);
      pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

      argument = one_argument (argument, arg);
      if (!NullStr (arg))
	{
	  replace_strf (&pet->name, "%s %s", pet->name, arg);
	}

      replace_strf (&pet->description,
		    "%sA neck tag says 'I belong to %s'." NEWLINE,
		    pet->description, ch->name);

      char_to_room (pet, ch->in_room);
      add_follower (pet, ch);
      pet->leader = ch;
      ch->pet = pet;
      chprintln (ch, "Enjoy your pet.");
      act ("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
      return;
    }
  else
    {
      CharData *keeper;
      ObjData *obj, *t_obj;
      char arg[MAX_INPUT_LENGTH];
      int number, count = 1;

      if ((keeper = find_keeper (ch)) == NULL)
	return;

      number = mult_argument (argument, arg);
      obj = get_obj_keeper (ch, keeper, arg);
      cost = get_cost (keeper, obj, true);

      if (number < 1 || number > 99)
	{
	  act ("$n tells you 'Get real!", keeper, NULL, ch, TO_VICT);
	  return;
	}

      if (cost <= 0 || !can_see_obj (ch, obj))
	{
	  act ("$n tells you 'I don't sell that -- try 'list''.",
	       keeper, NULL, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (!IsObjStat (obj, ITEM_INVENTORY))
	{
	  for (t_obj = obj->next_content;
	       count < number && t_obj != NULL; t_obj = t_obj->next_content)
	    {
	      if (t_obj->pIndexData == obj->pIndexData
		  && !str_cmp (t_obj->short_descr, obj->short_descr))
		count++;
	      else
		break;
	    }

	  if (count < number)
	    {
	      act ("$n tells you 'I don't have that many in stock.",
		   keeper, NULL, ch, TO_VICT);
	      ch->reply = keeper;
	      return;
	    }
	}

      if (!check_worth (ch, cost * number, VALUE_DEFAULT))
	{
	  if (number > 1)
	    act ("$n tells you 'You can't afford to buy that many.",
		 keeper, obj, ch, TO_VICT);
	  else
	    act ("$n tells you 'You can't afford to buy $p'.",
		 keeper, obj, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (obj->level > ch->level)
	{
	  act ("$n tells you 'You can't use $p yet'.", keeper, obj,
	       ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (ch->carry_number + number * get_obj_number (obj) > can_carry_n (ch))
	{
	  chprintln (ch, "You can't carry that many items.");
	  return;
	}

      if (ch->carry_weight + number * get_obj_weight (obj) > can_carry_w (ch))
	{
	  chprintln (ch, "You can't carry that much weight.");
	  return;
	}


      roll = number_percent ();
      if (!IsObjStat (obj, ITEM_SELL_EXTRACT) &&
	  roll < get_skill (ch, gsn_haggle))
	{
	  cost -= obj->cost / 2 * roll / 100;
	  act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
	  check_improve (ch, gsn_haggle, true, 4);
	}

      if (number > 1)
	{
	  sprintf (buf, "$n buys $p[%d].", number);
	  act (buf, ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p[%d] for %ld silver.", number,
		   cost * number);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      else
	{
	  act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
	  sprintf (buf, "You buy $p for %ld silver.", cost);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      deduct_cost (ch, cost * number, VALUE_DEFAULT);
      add_cost (keeper, cost * number, VALUE_DEFAULT);

      for (count = 0; count < number; count++)
	{
	  if (IsSet (obj->extra_flags, ITEM_INVENTORY))
	    t_obj = create_object (obj->pIndexData, obj->level);
	  else
	    {
	      t_obj = obj;
	      obj = obj->next_content;
	      obj_from_char (t_obj);
	    }

	  if (t_obj->timer > 0 && !IsObjStat (t_obj, ITEM_HAD_TIMER))
	    t_obj->timer = 0;
	  RemBit (t_obj->extra_flags, ITEM_HAD_TIMER);
	  obj_to_char (t_obj, ch);
	  if (cost < t_obj->cost)
	    t_obj->cost = cost;
	}
    }
}

Do_Fun (do_list)
{

  if (IsSet (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      RoomIndex *pRoomIndexNext;
      CharData *pet;
      bool found;


      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);
      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);

      if (pRoomIndexNext == NULL)
	{
	  bugf ("Do_list: bad pet shop at vnum %ld.", ch->in_room->vnum);
	  chprintln (ch, "You can't do that here.");
	  return;
	}

      found = false;
      for (pet = pRoomIndexNext->person_first; pet; pet = pet->next_in_room)
	{
	  if (IsSet (pet->act, ACT_PET))
	    {
	      if (!found)
		{
		  found = true;
		  chprintln (ch, "Pets for sale:");
		}
	      chprintlnf (ch, "[%2d] %8d - %s", pet->level,
			  10 * pet->level * pet->level, pet->short_descr);
	    }
	}
      if (!found)
	chprintln (ch, "Sorry, we're out of pets right now.");
      return;
    }
  else
    {
      CharData *keeper;
      ObjData *obj;
      money_t cost, count;
      bool found;
      char arg[MAX_INPUT_LENGTH];

      if ((keeper = find_keeper (ch)) == NULL)
	return;
      one_argument (argument, arg);

      found = false;
      for (obj = keeper->carrying_first; obj; obj = obj->next_content)
	{
	  int i = 0;
	  const char *p = obj->name;

	  for (; *p && !isspace (*p); p++, i++)
	    ;
	  if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj) &&
	      (cost = get_cost (keeper, obj, true)) > 0 &&
	      (NullStr (arg) || is_name (arg, obj->name)))
	    {
	      if (!found)
		{
		  found = true;
		  chprintln (ch, "[Lv Price Qty] Item");
		}

	      if (IsObjStat (obj, ITEM_INVENTORY))
		chprintlnf (ch, "[%2d %5ld -- ] "
			    MXPTAG ("List '%.*s' '%s'") "%s"
			    MXPTAG ("/List"), obj->level, cost, i,
			    obj->name, obj->short_descr, obj->short_descr);
	      else
		{
		  count = 1;

		  while (obj->next_content != NULL &&
			 obj->pIndexData ==
			 obj->next_content->pIndexData
			 && !str_cmp (obj->short_descr,
				      obj->next_content->short_descr))
		    {
		      obj = obj->next_content;
		      count++;
		    }
		  chprintlnf (ch, "[%2d %5ld %2d ] "
			      MXPTAG ("List '%.*s' '%s'") "%s"
			      MXPTAG ("/List"), obj->level, cost, count, i,
			      obj->name, obj->short_descr, obj->short_descr);
		}
	    }
	}

      if (!found)
	chprintln (ch, "You can't buy anything here.");
      return;
    }
}

Do_Fun (do_sell)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CharData *keeper;
  ObjData *obj;
  money_t cost;
  int roll;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Sell what?");
      return;
    }

  if ((keeper = find_keeper (ch)) == NULL)
    return;

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      act ("$n tells you 'You don't have that item'.", keeper, NULL, ch,
	   TO_VICT);
      ch->reply = keeper;
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      chprintln (ch, "You can't let go of it.");
      return;
    }

  if (IsObjStat (obj, ITEM_QUEST))
    {
      chprintln (ch, "You should sell that to the questor instead!");
      return;
    }

  if (!can_see_obj (keeper, obj))
    {
      act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	   TO_VICT);
      return;
    }

  if ((cost = get_cost (keeper, obj, false)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }
  if (!check_worth (keeper, cost, VALUE_DEFAULT))
    {
      act
	("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	 keeper, obj, ch, TO_VICT);
      return;
    }

  act ("$n sells $p.", ch, obj, NULL, TO_ROOM);

  roll = number_percent ();
  if (!IsObjStat (obj, ITEM_SELL_EXTRACT) &&
      roll < get_skill (ch, gsn_haggle))
    {
      chprintln (ch, "You haggle with the shopkeeper.");
      cost += obj->cost / 2 * roll / 100;
      cost = Min (cost, 95 * get_cost (keeper, obj, true) / 100);
      cost = Min (cost, (keeper->silver + 100 * keeper->gold));
      check_improve (ch, gsn_haggle, true, 4);
    }
  sprintf (buf, "You sell $p for %ld silver and %ld gold piece%s.",
	   cost - (cost / 100) * 100, cost / 100, cost == 1 ? "" : "s");
  act (buf, ch, obj, NULL, TO_CHAR);
  add_cost (ch, cost, VALUE_DEFAULT);
  deduct_cost (keeper, cost, VALUE_DEFAULT);

  if (obj->item_type == ITEM_TRASH || IsObjStat (obj, ITEM_SELL_EXTRACT))
    {
      extract_obj (obj);
    }
  else
    {
      obj_from_char (obj);
      if (obj->timer)
	SetBit (obj->extra_flags, ITEM_HAD_TIMER);
      else
	obj->timer = number_range (50, 100);
      obj_to_char (obj, keeper);
    }

  return;
}

Do_Fun (do_value)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CharData *keeper;
  ObjData *obj;
  money_t cost;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Value what?");
      return;
    }

  if ((keeper = find_keeper (ch)) == NULL)
    return;

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      act ("$n tells you 'You don't have that item'.", keeper, NULL, ch,
	   TO_VICT);
      ch->reply = keeper;
      return;
    }

  if (!can_see_obj (keeper, obj))
    {
      act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	   TO_VICT);
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      chprintln (ch, "You can't let go of it.");
      return;
    }

  if ((cost = get_cost (keeper, obj, false)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }

  sprintf (buf,
	   "$n tells you 'I'll give you %ld silver and %ld gold coins for $p'.",
	   cost - (cost / 100) * 100, cost / 100);
  act (buf, keeper, obj, ch, TO_VICT);
  ch->reply = keeper;

  return;
}


Do_Fun (do_appraise)
{
  char arg[MIL];
  CharData *keeper;
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Appraise what?");
      return;
    }

  if ((keeper = find_keeper (ch)) == NULL)
    {
      chprintln (ch, "You aren't in a shop.");
      return;
    }

  if ((obj = get_obj_keeper (ch, keeper, arg)) == NULL)
    {
      act ("{c$n{c tells you '{CI don't have that item.{c'{x.", keeper,
	   NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
    }

  spell_identify (0, ch->level, ch, obj, TARGET_OBJ);

  return;
}

Do_Fun (do_second)
{
  ObjData *obj;

  if (NullStr (argument))
    {
      chprintln (ch, "Wear which weapon in your off-hand?");
      return;
    }

  obj = get_obj_carry (ch, argument, ch);

  if (obj == NULL)
    {
      chprintln (ch, "You have no such thing in your backpack.");
      return;
    }



  if ((get_eq_char (ch, WEAR_SHIELD) != NULL) ||
      (get_eq_char (ch, WEAR_HOLD) != NULL))
    {
      chprintln
	(ch,
	 "You cannot use a secondary weapon while using a shield or holding an item");
      return;
    }

  if (ch->level < obj->level)
    {
      chprintlnf (ch, "You must be level %d to use this object.", obj->level);
      act ("$n tries to use $p, but is too inexperienced.", ch, obj, NULL,
	   TO_ROOM);
      return;
    }


  if (get_eq_char (ch, WEAR_WIELD) == NULL)
    {
      chprintln
	(ch,
	 "You need to wield a primary weapon, before using a secondary one!");
      return;
    }


  if (get_obj_weight (obj) >
      (str_app[get_curr_stat (ch, STAT_STR)].wield / 2))
    {
      chprintln
	(ch,
	 "This weapon is too heavy to be used as a secondary weapon by you.");
      return;
    }


  if ((get_obj_weight (obj) * 2) >
      get_obj_weight (get_eq_char (ch, WEAR_WIELD)))
    {
      chprintln
	(ch,
	 "Your secondary weapon has to be considerably lighter than the primary one.");
      return;
    }



  if (!remove_obj (ch, WEAR_SECONDARY, true))
    return;



  act ("$n wields $p in $s off-hand.", ch, obj, NULL, TO_ROOM);
  act ("You wield $p in your off-hand.", ch, obj, NULL, TO_CHAR);
  equip_char (ch, obj, WEAR_SECONDARY);
  return;
}

void
donate_object (ObjData * obj)
{
  RoomIndex *location;

  if (obj == NULL)
    {
      bug ("donate_object: NULL obj");
      return;
    }

  if (obj->carried_by)
    obj_from_char (obj);
  else if (obj->in_room)
    obj_from_room (obj);
  else if (obj->in_obj)
    obj_from_obj (obj);

  obj->cost = 0;

  if (obj->item_type == ITEM_WEAPON
      && (location = get_room_index (ROOM_VNUM_DONATION_WEAPON)) != NULL)
    obj_to_room (obj, location);
  else if (obj->item_type == ITEM_ARMOR
	   && (location = get_room_index (ROOM_VNUM_DONATION_ARMOR)) != NULL)
    obj_to_room (obj, location);
  else if ((location = get_room_index (ROOM_VNUM_DONATION_OTHER)) != NULL)
    obj_to_room (obj, location);
  else
    {
      bug ("donate_obj: NULL donation room");
      extract_obj (obj);
    }
}

Do_Fun (do_donate)
{
  char arg[MIL];
  ObjData *obj;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Donate what?");
      return;
    }
  if (!str_cmp ("all", arg) || !str_prefix ("all.", arg))
    {
      ObjData *obj_next;
      bool found = false;
      int count = 0;

      for (obj = ch->carrying_first; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  if (arg[3] != '\0' && !is_name (&arg[4], obj->name))
	    continue;

	  if (obj->item_type == ITEM_CORPSE_PC
	      || obj->item_type == ITEM_CORPSE_NPC
	      || IsObjStat (obj, ITEM_ROT_DEATH)
	      || IsObjStat (obj, ITEM_QUEST) || obj->wear_loc != WEAR_NONE)
	    continue;

	  if (count++ > 90)
	    {
	      chprintln (ch, "Too many objects, aborting.");
	      return;
	    }

	  found = true;
	  act ("$n donates $p to the needy.", ch, obj, NULL, TO_ROOM);
	  act ("You donate $p to the needy.", ch, obj, NULL, TO_CHAR);

	  donate_object (obj);
	}
      if (!found)
	chprintln (ch, "You have nothing to donate.");
      return;
    }

  if ((obj = get_obj_carry (ch, arg, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      chprintln (ch, "You can't let go of it.");
      return;
    }
  if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
    {
      chprintln (ch, "That would stink up the city.");
      return;
    }
  if (IsObjStat (obj, ITEM_ROT_DEATH))
    {
      chprintln (ch, "Only permanent items may be donated.");
      return;
    }

  if (IsObjStat (obj, ITEM_QUEST))
    {
      chprintln (ch, "You can't donate quest items.");
      return;
    }

  act ("$n donates $p to the needy.", ch, obj, NULL, TO_ROOM);
  act ("You donate $p to the needy.", ch, obj, NULL, TO_CHAR);
  donate_object (obj);

  return;
}


Do_Fun (do_ring)
{
  int door, count, search, depth;
  ExitData *pExit;
  CharData *vch;
  RoomIndex *room;

  if (!ch->in_room)
    return;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<direction>", NULL);
      return;
    }
  else if ((door = get_direction (argument)) == -1)
    {
      chprintln (ch, "Ring in what direction?");
      return;
    }
  else if (ch->in_room->exit[door] == NULL
	   || !IsSet (ch->in_room->exit[door]->exit_info, EX_DOORBELL))
    {
      chprintln (ch, "There is no doorbell in that direction.");
      return;
    }

  room = ch->in_room;
  count = 0;

  for (search = 0; search < MAX_DIR; search++)
    {
      for (depth = 0; depth < 5; depth++)
	{
	  if ((pExit = room->exit[door]) == NULL
	      || (pExit->u1.to_room) == NULL)
	    continue;

	  room = pExit->u1.to_room;
	  for (vch = room->person_first; vch; vch = vch->next_in_room)
	    {
	      if (!IsAwake (vch))
		continue;
	      chprintlnf (vch, "You hear a doorbell ring from the %s.",
			  rev_dir[door]);
	    }
	}
    }

  chprintln (ch, "You ring the doorbell.");
  act ("$n rings the doorbell.", ch, NULL, NULL, TO_ROOM);
}
