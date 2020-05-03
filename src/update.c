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
#include "magic.h"
#include "vnums.h"


Proto (int hit_gain, (CharData *));
Proto (int mana_gain, (CharData *));
Proto (int move_gain, (CharData *));
Proto (void mobile_update, (void));
Proto (void weather_update, (void));
Proto (void time_update, (void));
Proto (void char_update, (void));
Proto (void obj_update, (void));
Proto (void aggr_update, (void));
Proto (void quest_update, (void));
Proto (void sendstat_update, (void));



int save_number = 0;


void
advance_level (CharData * ch, bool hide)
{
  int add_hp;
  int add_mana;
  int add_move;
  int add_prac;

  if (!IsNPC (ch))
    ch->pcdata->last_level =
      (ch->pcdata->played + (int) (current_time - ch->logon)) / HOUR;

  add_hp = con_app[get_curr_stat (ch, STAT_CON)].hitp + get_hp_gain (ch);
  add_mana =
    number_range (2,
		  (2 * get_curr_stat (ch, STAT_INT) +
		   get_curr_stat (ch, STAT_WIS)) / 5);
  if (!has_spells (ch))
    add_mana /= 2;
  add_move =
    number_range (1,
		  (get_curr_stat (ch, STAT_CON) +
		   get_curr_stat (ch, STAT_DEX)) / 6);
  add_prac = wis_app[get_curr_stat (ch, STAT_WIS)].practice;

  add_hp = add_hp * 9 / 10;
  add_mana = add_mana * 9 / 10;
  add_move = add_move * 9 / 10;

  add_hp = Max (2, add_hp);
  add_mana = Max (2, add_mana);
  add_move = Max (6, add_move);

  ch->max_hit += add_hp;
  ch->max_mana += add_mana;
  ch->max_move += add_move;
  ch->practice += add_prac;
  ch->train += 1;

  ch->pcdata->perm_hit += add_hp;
  ch->pcdata->perm_mana += add_mana;
  ch->pcdata->perm_move += add_move;

  update_all_qobjs (ch);

  if (!hide)
    {
      chprintlnf (ch,
		  "You gain %d hit point%s, %d mana, %d move, and %d practice%s.",
		  add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
		  add_prac, add_prac == 1 ? "" : "s");

      if (!IsNPC (ch) && ch->level < LEVEL_IMMORTAL)
	{
	  int sn;

	  for (sn = 0; sn < top_skill; sn++)
	    {
	      if (skill_level (ch, sn) == ch->level)
		{
		  if (ch->pcdata->learned[sn] == 1)
		    chprintlnf (ch,
				"{MYou can now learn the {W%s {M%s.{x",
				skill_table[sn].name,
				skill_table[sn].spell_fun ==
				spell_null ? "skill" : "spell");
		  else
		    chprintlnf (ch,
				"{MYou can now use the {W%s {M%s.{x",
				skill_table[sn].name,
				skill_table[sn].spell_fun ==
				spell_null ? "skill" : "spell");
		}
	    }
	}
    }
  return;
}

void
gain_exp (CharData * ch, int gain)
{
  int maxl;

  if (IsNPC (ch) || ch->level >= (maxl = calc_max_level (ch)))
    return;

  ch->exp = Max (exp_per_level (ch, ch->pcdata->points), ch->exp + gain);
  while (ch->level < maxl &&
	 ch->exp >= exp_per_level (ch, ch->pcdata->points) * (ch->level + 1))
    {
      chprintln (ch, "You raise a level!!");
      ch->level += 1;
      mud_info.stats.levels++;
      new_wiznet (ch, NULL, WIZ_LEVELS, true, 0,
		  "$N has attained level %d!", ch->level);

      if (ch->level >= LEVEL_HERO)
	{
	  chprintlnf (ch, "Congratulations, you are now a %s!",
		      high_level_name (ch->level, true));
	  announce (ch, INFO_LEVEL, "$n is now a %s!",
		    high_level_name (ch->level, true));
	}
      else
	announce (ch, INFO_LEVEL, "$n has attained level %d!", ch->level);
      advance_level (ch, false);
      save_char_obj (ch);
    }

  return;
}


int
hit_gain (CharData * ch)
{
  int gain;
  int number;

  if (ch->in_room == NULL)
    return 0;

  if (IsNPC (ch))
    {
      gain = 5 + ch->level;
      if (IsAffected (ch, AFF_REGENERATION))
	gain *= 2;

      switch (ch->position)
	{
	default:
	  gain /= 2;
	  break;
	case POS_SLEEPING:
	  gain = 3 * gain / 2;
	  break;
	case POS_RESTING:
	  break;
	case POS_FIGHTING:
	  gain /= 3;
	  break;
	}

    }
  else
    {
      gain = Max (3, get_curr_stat (ch, STAT_CON) - 3 + ch->level / 2);
      gain += hp_max (ch) - 10;
      number = number_percent ();
      if (number < get_skill (ch, gsn_fast_healing))
	{
	  gain += number * gain / 100;
	  if (ch->hit < ch->max_hit)
	    check_improve (ch, gsn_fast_healing, true, 8);
	}

      switch (ch->position)
	{
	default:
	  gain /= 4;
	  break;
	case POS_SLEEPING:
	  break;
	case POS_RESTING:
	  gain /= 2;
	  break;
	case POS_FIGHTING:
	  gain /= 6;
	  break;
	}

      if (ch->pcdata->condition[COND_HUNGER] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;

    }

  gain = gain * ch->in_room->heal_rate / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if (IsAffected (ch, AFF_POISON))
    gain /= 4;

  if (IsAffected (ch, AFF_PLAGUE))
    gain /= 8;

  if (IsAffected (ch, AFF_HASTE) || IsAffected (ch, AFF_SLOW))
    gain /= 2;

  return Min (gain, ch->max_hit - ch->hit);
}

int
mana_gain (CharData * ch)
{
  int gain;
  int number;

  if (ch->in_room == NULL)
    return 0;

  if (IsNPC (ch))
    {
      gain = 5 + ch->level;
      switch (ch->position)
	{
	default:
	  gain /= 2;
	  break;
	case POS_SLEEPING:
	  gain = 3 * gain / 2;
	  break;
	case POS_RESTING:
	  break;
	case POS_FIGHTING:
	  gain /= 3;
	  break;
	}
    }
  else
    {
      gain =
	(get_curr_stat (ch, STAT_WIS) +
	 get_curr_stat (ch, STAT_INT) + ch->level) / 2;
      number = number_percent ();
      if (number < get_skill (ch, gsn_meditation))
	{
	  gain += number * gain / 100;
	  if (ch->mana < ch->max_mana)
	    check_improve (ch, gsn_meditation, true, 8);
	}
      if (!has_spells (ch))
	gain /= 2;

      switch (ch->position)
	{
	default:
	  gain /= 4;
	  break;
	case POS_SLEEPING:
	  break;
	case POS_RESTING:
	  gain /= 2;
	  break;
	case POS_FIGHTING:
	  gain /= 6;
	  break;
	}

      if (ch->pcdata->condition[COND_HUNGER] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;

    }

  gain = gain * ch->in_room->mana_rate / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[4] / 100;

  if (IsAffected (ch, AFF_POISON))
    gain /= 4;

  if (IsAffected (ch, AFF_PLAGUE))
    gain /= 8;

  if (IsAffected (ch, AFF_HASTE) || IsAffected (ch, AFF_SLOW))
    gain /= 2;

  return Min (gain, ch->max_mana - ch->mana);
}

int
move_gain (CharData * ch)
{
  int gain;

  if (ch->in_room == NULL)
    return 0;

  if (IsNPC (ch))
    {
      gain = ch->level;
    }
  else
    {
      gain = Max (15, ch->level);

      switch (ch->position)
	{
	case POS_SLEEPING:
	  gain += get_curr_stat (ch, STAT_DEX);
	  break;
	case POS_RESTING:
	  gain += get_curr_stat (ch, STAT_DEX) / 2;
	  break;
	default:
	  break;
	}

      if (ch->pcdata->condition[COND_HUNGER] == 0)
	gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
	gain /= 2;
    }

  gain = gain * ch->in_room->heal_rate / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if (IsAffected (ch, AFF_POISON))
    gain /= 4;

  if (IsAffected (ch, AFF_PLAGUE))
    gain /= 8;

  if (IsAffected (ch, AFF_HASTE) || IsAffected (ch, AFF_SLOW))
    gain /= 2;

  return Min (gain, ch->max_move - ch->move);
}

void
gain_condition (CharData * ch, int iCond, int value)
{
  int condition;

  if (value == 0 || IsNPC (ch) || ch->level >= LEVEL_IMMORTAL)
    return;

  condition = ch->pcdata->condition[iCond];
  if (condition == -1)
    return;
  ch->pcdata->condition[iCond] = Range (0, condition + value, 48);

  if (ch->pcdata->condition[iCond] == 0)
    {
      switch (iCond)
	{
	case COND_HUNGER:
	  chprintln (ch, "You are hungry.");
	  break;

	case COND_THIRST:
	  chprintln (ch, "You are thirsty.");
	  break;

	case COND_DRUNK:
	  if (condition != 0)
	    chprintln (ch, "You are sober.");
	  break;
	}
    }

  return;
}


void
mobile_update (void)
{
  CharData *ch;
  CharData *ch_next;
  ExitData *pexit;
  int door;


  for (ch = char_first; ch != NULL; ch = ch_next)
    {
      ch_next = ch->next;

      if (!IsNPC (ch) || ch->in_room == NULL || IsAffected (ch, AFF_CHARM))
	continue;

      if (ch->hunting)
	hunt_victim (ch);

      if (ch->in_room->area->empty && !IsSet (ch->act, ACT_UPDATE_ALWAYS))
	continue;


      if (ch->spec_fun != 0)
	{
	  if ((*ch->spec_fun) (ch))
	    continue;
	}

      if (ch->pIndexData->pShop != NULL)
	if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth)
	  {
	    ch->gold +=
	      ch->pIndexData->wealth * number_range (1, 20) / 5000000;
	    ch->silver +=
	      ch->pIndexData->wealth * number_range (1, 20) / 50000;
	  }


      if (ch->position == ch->pIndexData->default_pos)
	{

	  if (HasTriggerMob (ch, TRIG_DELAY) && ch->mprog_delay > 0)
	    {
	      if (--ch->mprog_delay <= 0)
		{
		  p_percent_trigger (ch, NULL, NULL, NULL, NULL, NULL,
				     TRIG_DELAY);
		  continue;
		}
	    }
	  if (HasTriggerMob (ch, TRIG_RANDOM))
	    {
	      if (p_percent_trigger
		  (ch, NULL, NULL, NULL, NULL, NULL, TRIG_RANDOM))
		continue;
	    }
	}


      if (ch->position != POS_STANDING)
	continue;


      if (IsSet (ch->act, ACT_SCAVENGER) &&
	  ch->in_room->content_first != NULL && number_bits (6) == 0)
	{
	  ObjData *obj;
	  ObjData *obj_best;
	  money_t max;

	  max = 1;
	  obj_best = 0;
	  for (obj = ch->in_room->content_first; obj; obj = obj->next_content)
	    {
	      if (CanWear (obj, ITEM_TAKE)
		  && can_loot (ch, obj) && obj->cost > max && obj->cost > 0)
		{
		  obj_best = obj;
		  max = obj->cost;
		}
	    }

	  if (obj_best)
	    {
	      obj_from_room (obj_best);
	      obj_to_char (obj_best, ch);
	      act ("$n gets $p.", ch, obj_best, NULL, TO_ROOM);
	    }
	}


      if (!IsSet (ch->act, ACT_SENTINEL) && number_bits (3) == 0 &&
	  (door = number_door ()) < MAX_DIR &&
	  (pexit = ch->in_room->exit[door]) != NULL &&
	  pexit->u1.to_room != NULL &&
	  !IsSet (pexit->exit_info, EX_CLOSED) &&
	  !IsSet (pexit->u1.to_room->room_flags, ROOM_NO_MOB) &&
	  (!IsSet (ch->act, ACT_STAY_AREA) ||
	   pexit->u1.to_room->area == ch->in_room->area) &&
	  (!IsSet (ch->act, ACT_OUTDOORS) ||
	   !IsSet (pexit->u1.to_room->room_flags, ROOM_INDOORS)) &&
	  (!IsSet (ch->act, ACT_INDOORS) ||
	   IsSet (pexit->u1.to_room->room_flags, ROOM_INDOORS)))
	{
	  move_char (ch, door, false);
	}
    }

  return;
}


void
char_update (void)
{
  CharData *ch;
  CharData *ch_next;
  CharData *ch_quit;

  ch_quit = NULL;


  save_number++;

  if (save_number > 29)
    save_number = 0;

  for (ch = char_first; ch != NULL; ch = ch_next)
    {
      AffectData *paf;
      AffectData *paf_next;

      ch_next = ch->next;

      if (ch->timer > 30)
	ch_quit = ch;

      if (ch->position >= POS_STUNNED)
	{

	  if (IsNPC (ch) && ch->zone != NULL &&
	      ch->zone != ch->in_room->area && ch->desc == NULL &&
	      ch->war == NULL &&
	      ch->fighting == NULL && !IsAffected (ch, AFF_CHARM)
	      && number_percent () < 5)
	    {
	      act ("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
	      extract_char (ch, true);
	      continue;
	    }

	  if (ch->hit < ch->max_hit)
	    ch->hit += hit_gain (ch);
	  else
	    ch->hit = ch->max_hit;

	  if (ch->mana < ch->max_mana)
	    ch->mana += mana_gain (ch);
	  else
	    ch->mana = ch->max_mana;

	  if (ch->move < ch->max_move)
	    ch->move += move_gain (ch);
	  else
	    ch->move = ch->max_move;
	}

      if (ch->position == POS_STUNNED)
	update_pos (ch);

      if (!IsNPC (ch) && ch->level < LEVEL_IMMORTAL)
	{
	  ObjData *obj;

	  if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL &&
	      obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
	    {
	      if (--obj->value[2] == 0 && ch->in_room != NULL)
		{
		  --ch->in_room->light;
		  act ("$p goes out.", ch, obj, NULL, TO_ROOM);
		  act ("$p flickers and goes out.", ch, obj, NULL, TO_CHAR);
		  extract_obj (obj);
		}
	      else if (obj->value[2] <= 5 && ch->in_room != NULL)
		act ("$p flickers.", ch, obj, NULL, TO_CHAR);
	    }

	  if (IsImmortal (ch))
	    ch->timer = 0;

	  if (++ch->timer >= 12)
	    {
	      if (ch->was_in_room == NULL && ch->in_room != NULL)
		{
		  ch->was_in_room = ch->in_room;
		  if (ch->fighting != NULL)
		    stop_fighting (ch, true);
		  act ("$n disappears into the void.", ch,
		       NULL, NULL, TO_ROOM);
		  chprintln (ch, "You disappear into the void.");
		  if (ch->level > 1)
		    save_char_obj (ch);
		  char_from_room (ch);
		  char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
		}
	    }

	  gain_condition (ch, COND_DRUNK, -1);
	  gain_condition (ch, COND_FULL, ch->size > SIZE_MEDIUM ? -4 : -2);
	  gain_condition (ch, COND_THIRST, -1);
	  gain_condition (ch, COND_HUNGER, ch->size > SIZE_MEDIUM ? -2 : -1);
	}

      for (paf = ch->affect_first; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  if (paf->duration > 0)
	    {
	      paf->duration--;
	      if (number_range (0, 4) == 0 && paf->level > 0)
		paf->level--;
	    }
	  else if (paf->duration < 0)
	    ;
	  else
	    {
	      if (paf_next == NULL
		  || paf_next->type != paf->type || paf_next->duration > 0)
		{
		  if (paf->type > 0 && skill_table[paf->type].msg_off)
		    {
		      chprintln (ch, skill_table[paf->type].msg_off);
		    }
		}

	      affect_remove (ch, paf);
	    }
	}



      if (is_affected (ch, gsn_plague) && ch != NULL)
	{
	  AffectData *af, plague;
	  CharData *vch;
	  int dam;

	  if (ch->in_room == NULL)
	    continue;

	  act ("$n writhes in agony as plague sores erupt from $s skin.",
	       ch, NULL, NULL, TO_ROOM);
	  chprintln (ch, "You writhe in agony from the plague.");
	  for (af = ch->affect_first; af != NULL; af = af->next)
	    {
	      if (af->type == gsn_plague)
		break;
	    }

	  if (af == NULL)
	    {
	      RemBit (ch->affected_by, AFF_PLAGUE);
	      continue;
	    }

	  if (af->level == 1)
	    continue;

	  plague.where = TO_AFFECTS;
	  plague.type = gsn_plague;
	  plague.level = af->level - 1;
	  plague.duration = number_range (1, 2 * plague.level);
	  plague.location = APPLY_STR;
	  plague.modifier = -5;
	  plague.bitvector = AFF_PLAGUE;

	  for (vch = ch->in_room->person_first; vch != NULL;
	       vch = vch->next_in_room)
	    {
	      if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
		  && !IsImmortal (vch)
		  && !IsAffected (vch, AFF_PLAGUE) && number_bits (4) == 0)
		{
		  chprintln (vch, "You feel hot and feverish.");
		  act ("$n shivers and looks very ill.",
		       vch, NULL, NULL, TO_ROOM);
		  affect_join (vch, &plague);
		}
	    }

	  dam = Min (ch->level, af->level / 5 + 1);
	  ch->mana -= dam;
	  ch->move -= dam;
	  damage (ch, ch, dam, gsn_plague, DAM_DISEASE, false);
	}
      else if (IsAffected (ch, AFF_POISON) && ch != NULL &&
	       !IsAffected (ch, AFF_SLOW))
	{
	  AffectData *poison;

	  poison = affect_find (ch->affect_first, gsn_poison);

	  if (poison != NULL)
	    {
	      act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
	      chprintln (ch, "You shiver and suffer.");
	      damage (ch, ch, poison->level / 10 + 1,
		      gsn_poison, DAM_POISON, false);
	    }
	}
      else if (ch->position == POS_INCAP && number_range (0, 1) == 0)
	{
	  damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, false);
	}
      else if (ch->position == POS_MORTAL)
	{
	  damage (ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, false);
	}
    }


  for (ch = player_first; ch != NULL; ch = ch_next)
    {
      ch_next = ch->next_player;

      if (ch->desc != NULL && (int) ch->desc->descriptor % 30 == save_number)
	{
	  save_char_obj (ch);
	}

      if (ch == ch_quit)
	{
	  do_function (ch, &do_quit, "");
	}
    }

  return;
}


void
obj_update (void)
{
  ObjData *obj;
  ObjData *obj_next;
  AffectData *paf, *paf_next;

  for (obj = obj_first; obj != NULL; obj = obj_next)
    {
      CharData *rch;
      char *message;

      obj_next = obj->next;


      for (paf = obj->affect_first; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  if (paf->duration > 0)
	    {
	      paf->duration--;
	      if (number_range (0, 4) == 0 && paf->level > 0)
		paf->level--;
	    }
	  else if (paf->duration < 0)
	    ;
	  else
	    {
	      if (paf_next == NULL
		  || paf_next->type != paf->type || paf_next->duration > 0)
		{
		  if (paf->type > 0 && skill_table[paf->type].msg_obj)
		    {
		      if (obj->carried_by != NULL)
			{
			  rch = obj->carried_by;
			  act (skill_table
			       [paf->type].msg_obj, rch, obj, NULL, TO_CHAR);
			}
		      if (obj->in_room != NULL
			  && obj->in_room->person_first != NULL)
			{
			  rch = obj->in_room->person_first;
			  act (skill_table
			       [paf->type].msg_obj, rch, obj, NULL, TO_ALL);
			}
		    }
		}

	      affect_remove_obj (obj, paf);
	    }
	}

      if (obj->timer <= 0 || --obj->timer > 0)
	continue;


      if (obj->in_room || (obj->carried_by && obj->carried_by->in_room))
	{
	  if (HasTriggerObj (obj, TRIG_DELAY) && obj->oprog_delay > 0)
	    {
	      if (--obj->oprog_delay <= 0)
		p_percent_trigger (NULL, obj, NULL, NULL, NULL, NULL,
				   TRIG_DELAY);
	    }
	  else if (((obj->in_room && !obj->in_room->area->empty)
		    || obj->carried_by) && HasTriggerObj (obj, TRIG_RANDOM))
	    p_percent_trigger (NULL, obj, NULL, NULL, NULL, NULL,
			       TRIG_RANDOM);
	}

      if (!obj)
	continue;

      switch (obj->item_type)
	{
	default:
	  message = "$p crumbles into dust.";
	  break;
	case ITEM_FOUNTAIN:
	  message = "$p dries up.";
	  break;
	case ITEM_CORPSE_NPC:
	  message = "$p decays into dust.";
	  break;
	case ITEM_CORPSE_PC:
	  message = "$p decays into dust.";
	  break;
	case ITEM_FOOD:
	  message = "$p decomposes.";
	  break;
	case ITEM_POTION:
	  message = "$p has evaporated from disuse.";
	  break;
	case ITEM_PORTAL:
	  message = "$p fades out of existence.";
	  break;
	case ITEM_CONTAINER:
	  if (CanWear (obj, ITEM_WEAR_FLOAT))
	    if (obj->content_first)
	      message =
		"$p flickers and vanishes, spilling its contents on the floor.";
	    else
	      message = "$p flickers and vanishes.";
	  else
	    message = "$p crumbles into dust.";
	  break;
	}

      if (obj->carried_by != NULL)
	{
	  if (IsNPC (obj->carried_by) &&
	      obj->carried_by->pIndexData->pShop != NULL)
	    obj->carried_by->silver += obj->cost / 5;
	  else
	    {
	      act (message, obj->carried_by, obj, NULL, TO_CHAR);
	      if (obj->wear_loc == WEAR_FLOAT)
		act (message, obj->carried_by, obj, NULL, TO_ROOM);
	    }
	}
      else if (obj->in_room != NULL
	       && (rch = obj->in_room->person_first) != NULL)
	{
	  act (message, rch, obj, NULL, TO_ROOM);
	  act (message, rch, obj, NULL, TO_CHAR);
	}

      if ((obj->item_type == ITEM_CORPSE_PC ||
	   obj->wear_loc == WEAR_FLOAT) && obj->content_first)
	{
	  ObjData *t_obj, *next_obj;

	  for (t_obj = obj->content_first; t_obj != NULL; t_obj = next_obj)
	    {
	      next_obj = t_obj->next_content;
	      obj_from_obj (t_obj);

	      if (obj->in_obj)
		obj_to_obj (t_obj, obj->in_obj);

	      else if (obj->carried_by)
		if (obj->wear_loc == WEAR_FLOAT)
		  if (obj->carried_by->in_room == NULL)
		    extract_obj (t_obj);
		  else
		    obj_to_room (t_obj, obj->carried_by->in_room);
		else
		  obj_to_char (t_obj, obj->carried_by);

	      else if (obj->in_room == NULL)
		extract_obj (t_obj);

	      else
		obj_to_room (t_obj, obj->in_room);
	    }
	}

      extract_obj (obj);
    }

  return;
}


void
aggr_update (void)
{
  CharData *wch;
  CharData *wch_next;
  CharData *ch;
  CharData *ch_next;
  CharData *vch;
  CharData *vch_next;
  CharData *victim;

  for (wch = player_first; wch != NULL; wch = wch_next)
    {
      if (wch->next == NULL)
	{
	  return;
	}
      wch_next = wch->next_player;
      if (IsNPC (wch) || wch->level >= LEVEL_IMMORTAL ||
	  wch->in_room == NULL || wch->in_room->area->empty ||
	  IsSet (wch->in_room->room_flags, ROOM_SAFE))
	continue;

      for (ch = wch->in_room->person_first; ch != NULL; ch = ch_next)
	{
	  int count;

	  ch_next = ch->next_in_room;

	  if (!IsNPC (ch) || !IsSet (ch->act, ACT_AGGRESSIVE) ||
	      IsAffected (ch, AFF_CALM) || ch->fighting != NULL ||
	      IsAffected (ch, AFF_CHARM) || !IsAwake (ch) ||
	      (IsSet (ch->act, ACT_WIMPY) && IsAwake (wch)) ||
	      !can_see (ch, wch) || number_bits (1) == 0)
	    continue;


	  count = 0;
	  victim = NULL;
	  for (vch = wch->in_room->person_first; vch != NULL; vch = vch_next)
	    {
	      vch_next = vch->next_in_room;

	      if (!IsNPC (vch) && vch->level < LEVEL_IMMORTAL
		  && ch->level >= vch->level - 5
		  && (!IsSet (ch->act, ACT_WIMPY)
		      || !IsAwake (vch)) && can_see (ch, vch))
		{
		  if (number_range (0, count) == 0)
		    victim = vch;
		  count++;
		}
	    }

	  if (victim == NULL)
	    continue;

	  multi_hit (ch, victim, TYPE_UNDEFINED);
	}
    }

  return;
}

void
bank_update (void)
{
  int value = 0;

  if ((time_info.hour < 6) || (time_info.hour > 18))
    return;

  value = number_range (0, 200);
  value -= 100;
  value /= 10;

  mud_info.share_value = Range (10, mud_info.share_value + value, 1000);
}

void
bonus_update (void)
{
  if (mud_info.bonus.status == BONUS_OFF)
    return;

  switch (mud_info.bonus.time)
    {
    case -1:
      mud_info.bonus.status = BONUS_OFF;
      break;
    case 0:
      announce (NULL, INFO_MISC, "%dx %s has now ended.",
		mud_info.bonus.mod, bonus_name (mud_info.bonus.status));
      mud_info.bonus.status = BONUS_OFF;
      mud_info.bonus.mod = 0;
      replace_str (&mud_info.bonus.msg, "");
      mud_info.bonus.time = -1;
      return;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 10:
    case 15:
      announce (NULL, INFO_MISC, "%s remaining of %dx %s.",
		intstr (mud_info.bonus.time, "minute"),
		mud_info.bonus.mod, bonus_name (mud_info.bonus.status));
    default:
      mud_info.bonus.time--;
      break;
    }
  return;
}



void
update_handler (void)
{
  static int pulse_area;
  static int pulse_mobile;
  static int pulse_violence;
  static int pulse_point;
  static int pulse_music;
  static int pulse_sendstat;

  crash_info.status = CRASH_UPDATING;

  if (--pulse_area <= 0)
    {
      pulse_area = PULSE_AREA;
      strcpy (crash_info.shrt_cmd, "pulse_area");
      area_update ();
      bank_update ();
    }

  if (--pulse_music <= 0)
    {
      pulse_music = PULSE_MUSIC;
      strcpy (crash_info.shrt_cmd, "pulse_music");
      song_update ();
    }

  if (--pulse_mobile <= 0)
    {
      pulse_mobile = PULSE_MOBILE;
      strcpy (crash_info.shrt_cmd, "pulse_mobile");
      mobile_update ();
    }

  if (--pulse_violence <= 0)
    {
      pulse_violence = PULSE_VIOLENCE;
      strcpy (crash_info.shrt_cmd, "pulse_violence");
      violence_update ();
    }

  if (--pulse_point <= 0)
    {
      pulse_point = PULSE_TICK;
      strcpy (crash_info.shrt_cmd, "pulse_point");
      wiznet ("TICK!", NULL, NULL, WIZ_TICKS, false, 0);
      weather_update ();
      time_update ();
      char_update ();
      obj_update ();
      quest_update ();
      gquest_update ();
      war_update ();
      bonus_update ();
    }

  if (--pulse_sendstat <= 0)
    {
      pulse_sendstat = 1;
      sendstat_update ();
    }

  strcpy (crash_info.shrt_cmd, "auction_update");
  auction_update ();
  strcpy (crash_info.shrt_cmd, "crs_update");
  crs_update ();
  strcpy (crash_info.shrt_cmd, "aggr_update");
  aggr_update ();
  strcpy (crash_info.shrt_cmd, "N/A");
  crash_info.status = CRASH_UNKNOWN;
  tail_chain ();
  return;
}
