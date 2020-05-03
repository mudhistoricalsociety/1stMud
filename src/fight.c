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
#include "magic.h"


Proto (void check_assist, (CharData *, CharData *));

Proto (bool check_dodge, (CharData *, CharData *));
Proto (bool check_parry, (CharData *, CharData *));
Proto (bool check_shield_block, (CharData *, CharData *));
Proto (void dam_message, (CharData *, CharData *, int, int, bool));
Proto (void death_cry, (CharData *));
Proto (void group_gain, (CharData *, CharData *));
Proto (int xp_compute, (CharData *, CharData *, int));
Proto (void make_corpse, (CharData *));
Proto (void one_hit, (CharData *, CharData *, int, bool));
Proto (void mob_hit, (CharData *, CharData *, int));
Proto (void raw_kill, (CharData *, CharData *));
Proto (void update_death, (CharData *, CharData *));
Proto (void set_fighting, (CharData *, CharData *));
Proto (void disarm, (CharData *, CharData *));

Proto (bool check_force_shield, (CharData *, CharData *));
Proto (bool check_static_shield, (CharData *, CharData *));
Proto (bool check_flame_shield, (CharData *, CharData *));



void
violence_update (void)
{
  CharData *ch;
  CharData *ch_next;
  CharData *victim;
  ObjData *obj, *obj_next;
  bool room_trig = false;

  for (ch = char_first; ch != NULL; ch = ch_next)
    {
      ch_next = ch->next;

      if (IsNPC (ch) && ch->fighting == NULL
	  && IsAwake (ch) && ch->hunting != NULL)
	{
	  hunt_victim (ch);
	  continue;
	}

      if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
	continue;

      if (IsAwake (ch) && ch->in_room == victim->in_room)
	multi_hit (ch, victim, TYPE_UNDEFINED);
      else
	stop_fighting (ch, false);

      if ((victim = ch->fighting) == NULL)
	continue;


      check_assist (ch, victim);

      if (IsNPC (ch))
	{
	  if (HasTriggerMob (ch, TRIG_FIGHT))
	    p_percent_trigger (ch, NULL, NULL, victim, NULL, NULL,
			       TRIG_FIGHT);
	  if (HasTriggerMob (ch, TRIG_HPCNT))
	    p_hprct_trigger (ch, victim);
	}
      for (obj = ch->carrying_first; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  if (obj->wear_loc != WEAR_NONE && HasTriggerObj (obj, TRIG_FIGHT))
	    p_percent_trigger (NULL,
			       obj, NULL, victim, NULL, NULL, TRIG_FIGHT);
	}

      if (HasTriggerRoom (ch->in_room, TRIG_FIGHT) && room_trig == false)
	{
	  room_trig = true;
	  p_percent_trigger (NULL, NULL, ch->in_room, victim, NULL, NULL,
			     TRIG_FIGHT);
	}
    }

  return;
}


void
check_assist (CharData * ch, CharData * victim)
{
  CharData *rch, *rch_next;

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch_next)
    {
      rch_next = rch->next_in_room;

      if (IsAwake (rch) && rch->fighting == NULL)
	{


	  if (!IsNPC (ch) && IsNPC (rch) &&
	      IsSet (rch->off_flags, ASSIST_PLAYERS) &&
	      rch->level + 6 > victim->level)
	    {
	      do_function (rch, &do_emote, "screams and attacks!");
	      multi_hit (rch, victim, TYPE_UNDEFINED);
	      continue;
	    }


	  if (!IsNPC (ch) || IsAffected (ch, AFF_CHARM))
	    {
	      if (((!IsNPC (rch) &&
		    IsSet (rch->act, PLR_AUTOASSIST)) ||
		   IsAffected (rch, AFF_CHARM)) &&
		  is_same_group (ch, rch) && !is_safe (rch, victim))
		multi_hit (rch, victim, TYPE_UNDEFINED);

	      continue;
	    }



	  if (IsNPC (ch) && !IsAffected (ch, AFF_CHARM))
	    {
	      if ((IsNPC (rch) &&
		   IsSet (rch->off_flags, ASSIST_ALL)) ||
		  (IsNPC (rch) && rch->group &&
		   rch->group == ch->group) || (IsNPC (rch) &&
						rch->race ==
						ch->race
						&&
						IsSet
						(rch->off_flags,
						 ASSIST_RACE))
		  || (IsNPC (rch) && IsSet (rch->off_flags, ASSIST_ALIGN)
		      && ((IsGood (rch) && IsGood (ch))
			  || (IsEvil (rch) && IsEvil (ch))
			  || (IsNeutral (rch)
			      && IsNeutral (ch))))
		  || (rch->pIndexData == ch->pIndexData
		      && IsSet (rch->off_flags, ASSIST_VNUM)))
		{
		  CharData *vch;
		  CharData *target;
		  int number;

		  if (number_bits (1) == 0)
		    continue;

		  target = NULL;
		  number = 0;
		  for (vch = ch->in_room->person_first; vch; vch = vch->next)
		    {
		      if (can_see (rch, vch) && is_same_group (vch, victim)
			  && number_range (0, number) == 0)
			{
			  target = vch;
			  number++;
			}
		    }

		  if (target != NULL)
		    {
		      do_function (rch, &do_emote, "screams and attacks!");
		      multi_hit (rch, target, TYPE_UNDEFINED);
		    }
		}
	    }
	}
    }
}

void
special_move (CharData * ch, CharData * victim)
{
  if (!victim || victim->position == POS_DEAD)
    return;

  switch (number_range (1, 7))
    {
    default:
      return;
    case 1:
      act
	("{RYou pull your hands into your waist then snap them into $N's{R stomach.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R pulls $s{R hands into $s{R waist then snaps them into your stomach.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R pulls $s{R hands into $s{R waist then snaps them into $N's{R stomach.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act
	("{RYou double over in agony, and fall to the ground gasping for breath.{x",
	 victim, NULL, NULL, TO_CHAR);
      act
	("{R$n{R doubles over in agony, and falls to the ground gasping for breath.{x",
	 victim, NULL, NULL, TO_ROOM);
      break;
    case 2:
      act
	("{RYou spin in a low circle, catching $N{R behind $S{R ankle.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R spins in a low circle, catching you behind your ankle.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R spins in a low circle, catching $N{R behind $S{R ankle.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act ("{RYou crash to the ground, stunned.{x", victim, NULL, NULL,
	   TO_CHAR);
      act ("{R$n{R crashes to the ground, stunned.{x", victim, NULL, NULL,
	   TO_ROOM);
      break;
    case 3:
      act ("{RYou roll between $N's{R legs and flip to your feet.{x", ch,
	   NULL, victim, TO_CHAR);
      act ("{R$n{R rolls between your legs and flips to $s{R feet.{x", ch,
	   NULL, victim, TO_VICT);
      act ("{R$n{R rolls between $N's{R legs and flips to $s{R feet.{x",
	   ch, NULL, victim, TO_NOTVICT);
      act
	("{RYou spin around and smash your elbow into the back of $N's{R head.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R spins around and smashes $s{R elbow into the back of your head.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R spins around and smashes $s{R elbow into the back of $N's{R head.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act ("{RYou fall to the ground, stunned.{x", victim, NULL, NULL,
	   TO_CHAR);
      act ("{R$n{R falls to the ground, stunned.{x", victim, NULL, NULL,
	   TO_ROOM);
      break;
    case 4:
      act
	("{RYou somersault over $N's{R head and land lightly on your toes.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R somersaults over your head and lands lightly on $s toes.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R somersaults over $N's{R head and lands lightly on $s toes.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act
	("{RYou roll back onto your shoulders and kick both feet into $N's{R back.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R rolls back onto $s{R shoulders and kicks both feet into your back.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R rolls back onto $s{R shoulders and kicks both feet into $N's{R back.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act ("{RYou fall to the ground, stunned.", victim, NULL, NULL, TO_CHAR);
      act ("{R$n{R falls to the ground, stunned.", victim, NULL, NULL,
	   TO_ROOM);
      act ("{RYou flip back up to your feet.", ch, NULL, NULL, TO_CHAR);
      act ("{R$n{R flips back up to $s feet.", ch, NULL, NULL, TO_ROOM);
      break;
    case 5:
      act
	("{RYou grab $N{R by the waist and hoist $M{R above your head.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R grabs $N{R by the waist and hoists $M{R above $s{R head.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act
	("{R$n{R grabs you by the waist and hoists you above $s{R head.{x",
	 ch, NULL, victim, TO_VICT);
      act ("{RYou crash to the ground, stunned.{x", victim, NULL, NULL,
	   TO_CHAR);
      act ("{R$n{R crashes to the ground, stunned.{x", victim, NULL, NULL,
	   TO_ROOM);
      break;
    case 6:
      act
	("{RYou grab $N{R by the head and slam $S{R face into your knee.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R grabs you by the head and slams your face into $s{R knee.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R grabs $N{R by the head and slams $S{R face into $s{R knee.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act ("{RYou crash to the ground, stunned.{x", victim, NULL, NULL,
	   TO_CHAR);
      act ("{R$n{R crashes to the ground, stunned.{x", victim, NULL, NULL,
	   TO_ROOM);
      act ("{RYou flip back up to your feet.{x", ch, NULL, NULL, TO_CHAR);
      act ("{R$n{R flips back up to $s{R feet.{x", ch, NULL, NULL, TO_ROOM);
      break;
    case 7:
      act
	("{RYou duck under $N's{R attack and pound your fist into $S{R stomach.{x",
	 ch, NULL, victim, TO_CHAR);
      act
	("{R$n{R ducks under your attack and pounds $s{R fist into your stomach.{x",
	 ch, NULL, victim, TO_VICT);
      act
	("{R$n{R ducks under $N's{R attack and pounds $s{R fist into $N's{R stomach.{x",
	 ch, NULL, victim, TO_NOTVICT);
      act ("{RYou double over in agony.{x", victim, NULL, NULL, TO_CHAR);
      act ("{R$n{R doubles over in agony.{x", victim, NULL, NULL, TO_ROOM);
      break;
    }

  stop_fighting (victim, true);
  victim->position = POS_STUNNED;

  return;
}


void
multi_hit (CharData * ch, CharData * victim, int dt)
{
  int chance;


  if (ch->desc == NULL)
    ch->wait = Max (0, ch->wait - PULSE_VIOLENCE);

  if (ch->desc == NULL)
    ch->daze = Max (0, ch->daze - PULSE_VIOLENCE);


  if (ch->position < POS_RESTING)
    return;

  if (IsNPC (ch))
    {
      mob_hit (ch, victim, dt);
      return;
    }

  one_hit (ch, victim, dt, false);

  if (get_eq_char (ch, WEAR_SECONDARY))
    {
      one_hit (ch, victim, dt, true);
      if (ch->fighting != victim)
	return;
    }

  if (ch->fighting != victim)
    return;

  if (IsAffected (ch, AFF_HASTE))
    one_hit (ch, victim, dt, false);

  if (ch->fighting != victim || dt == gsn_backstab)
    return;

  if (ValidStance (GetStance (ch, STANCE_CURRENT))
      && GetStance (ch, STANCE_CURRENT) >= 200 && number_percent () == 50)
    {
      special_move (ch, victim);
      return;
    }

  chance = get_skill (ch, gsn_second_attack) / 2;

  if (IsAffected (ch, AFF_SLOW))
    chance /= 2;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, false);
      check_improve (ch, gsn_second_attack, true, 5);
      if (ch->fighting != victim)
	return;
    }

  chance = get_skill (ch, gsn_third_attack) / 4;

  if (IsAffected (ch, AFF_SLOW))
    chance = 0;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, false);
      check_improve (ch, gsn_third_attack, true, 6);
      if (ch->fighting != victim)
	return;
    }

  if (InStance (ch, STANCE_VIPER)
      && number_percent () < GetStance (ch, STANCE_VIPER) * 0.5)
    {
      one_hit (ch, victim, dt, false);
      if (ch->fighting != victim)
	return;
    }
  else if (InStance (ch, STANCE_MANTIS)
	   && number_percent () < GetStance (ch, STANCE_MANTIS) * 0.5)
    {
      one_hit (ch, victim, dt, false);
      if (ch->fighting != victim)
	return;
    }
  else if (InStance (ch, STANCE_TIGER)
	   && number_percent () < GetStance (ch, STANCE_TIGER) * 0.5)
    {
      one_hit (ch, victim, dt, false);
      if (ch->fighting != victim)
	return;
    }

  return;
}


void
mob_hit (CharData * ch, CharData * victim, int dt)
{
  int chance, number;
  CharData *vch, *vch_next;

  one_hit (ch, victim, dt, false);

  if (ch->fighting != victim)
    return;



  if (IsSet (ch->off_flags, OFF_AREA_ATTACK))
    {
      for (vch = ch->in_room->person_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;
	  if ((vch != victim && vch->fighting == ch))
	    one_hit (ch, vch, dt, false);
	}
    }

  if (IsAffected (ch, AFF_HASTE) ||
      (IsSet (ch->off_flags, OFF_FAST) && !IsAffected (ch, AFF_SLOW)))
    one_hit (ch, victim, dt, false);

  if (ch->fighting != victim || dt == gsn_backstab)
    return;

  chance = get_skill (ch, gsn_second_attack) / 2;

  if (IsAffected (ch, AFF_SLOW) && !IsSet (ch->off_flags, OFF_FAST))
    chance /= 2;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, false);
      if (ch->fighting != victim)
	return;
    }

  chance = get_skill (ch, gsn_third_attack) / 4;

  if (IsAffected (ch, AFF_SLOW) && !IsSet (ch->off_flags, OFF_FAST))
    chance = 0;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, false);
      if (ch->fighting != victim)
	return;
    }



  if (ch->wait > 0)
    return;

  number = number_range (0, 2);

  if (number == 1 && IsSet (ch->act, ACT_MAGE))
    {
      ;
    }

  if (number == 2 && IsSet (ch->act, ACT_CLERIC))
    {
      ;
    }



  number = number_range (0, 8);

  switch (number)
    {
    case (0):
      if (IsSet (ch->off_flags, OFF_BASH))
	do_function (ch, &do_bash, "");
      break;

    case (1):
      if (IsSet (ch->off_flags, OFF_BERSERK) && !IsAffected (ch, AFF_BERSERK))
	do_function (ch, &do_berserk, "");
      break;

    case (2):
      if (IsSet (ch->off_flags, OFF_DISARM) ||
	  (get_weapon_sn (ch) != gsn_hand_to_hand &&
	   (IsSet (ch->act, ACT_WARRIOR) || IsSet (ch->act, ACT_THIEF))))
	do_function (ch, &do_disarm, "");
      break;

    case (3):
      if (IsSet (ch->off_flags, OFF_KICK))
	do_function (ch, &do_kick, "");
      break;

    case (4):
      if (IsSet (ch->off_flags, OFF_KICK_DIRT))
	do_function (ch, &do_dirt, "");
      break;

    case (5):
      if (IsSet (ch->off_flags, OFF_TAIL))
	{
	  ;
	}
      break;

    case (6):
      if (IsSet (ch->off_flags, OFF_TRIP))
	do_function (ch, &do_trip, "");
      break;

    case (7):
      if (IsSet (ch->off_flags, OFF_CRUSH))
	{
	  ;
	}
      break;
    case (8):
      if (IsSet (ch->off_flags, OFF_BACKSTAB))
	{
	  do_function (ch, &do_backstab, "");
	}
    }
}


void
one_hit (CharData * ch, CharData * victim, int dt, bool secondary)
{
  ObjData *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill;
  dam_class dam_type;
  bool result;

  sn = -1;


  if (victim == ch || ch == NULL || victim == NULL)
    return;


  if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;


  if (!secondary)
    wield = get_eq_char (ch, WEAR_WIELD);
  else
    wield = get_eq_char (ch, WEAR_SECONDARY);

  if (dt == TYPE_UNDEFINED)
    {
      dt = TYPE_HIT;
      if (wield != NULL && wield->item_type == ITEM_WEAPON)
	dt += wield->value[3];
      else
	dt += ch->dam_type;
    }

  if (dt < TYPE_HIT)
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;
    else
      dam_type = attack_table[ch->dam_type].damage;
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;

  if (dam_type == -1)
    dam_type = DAM_BASH;


  sn = get_weapon_sn (ch);
  skill = 20 + get_weapon_skill (ch, sn);


  if (IsNPC (ch))
    {
      thac0_00 = 20;
      thac0_32 = -4;
      if (IsSet (ch->act, ACT_WARRIOR))
	thac0_32 = -10;
      else if (IsSet (ch->act, ACT_THIEF))
	thac0_32 = -4;
      else if (IsSet (ch->act, ACT_CLERIC))
	thac0_32 = 2;
      else if (IsSet (ch->act, ACT_MAGE))
	thac0_32 = 6;
    }
  else
    {
      thac0_00 = get_thac00 (ch);
      thac0_32 = get_thac32 (ch);
    }
  thac0 = interpolate (ch->level, thac0_00, thac0_32);

  if (thac0 < 0)
    thac0 = thac0 / 2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  thac0 -= GetHitroll (ch) * skill / 100;
  thac0 += 5 * (100 - skill) / 100;

  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

  switch (dam_type)
    {
    case (DAM_PIERCE):
      victim_ac = GetArmor (victim, AC_PIERCE) / 10;
      break;
    case (DAM_BASH):
      victim_ac = GetArmor (victim, AC_BASH) / 10;
      break;
    case (DAM_SLASH):
      victim_ac = GetArmor (victim, AC_SLASH) / 10;
      break;
    default:
      victim_ac = GetArmor (victim, AC_EXOTIC) / 10;
      break;
    };

  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;

  if (!can_see (ch, victim))
    victim_ac -= 4;

  if (victim->position < POS_FIGHTING)
    victim_ac += 4;

  if (victim->position < POS_RESTING)
    victim_ac += 6;


  while ((diceroll = number_bits (5)) >= 20)
    ;

  if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {

      damage (ch, victim, 0, dt, dam_type, true);
      improve_stance (ch);
      tail_chain ();
      return;
    }


  if (IsNPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
    if (!ch->pIndexData->new_format)
      {
	dam = number_range (ch->level / 2, ch->level * 3 / 2);
	if (wield != NULL)
	  dam += dam / 2;
      }
    else
      dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

  else
    {
      if (sn != -1)
	check_improve (ch, sn, true, 5);
      if (wield != NULL)
	{
	  if (wield->pIndexData->new_format)
	    dam = dice (wield->value[1], wield->value[2]) * skill / 100;
	  else
	    dam =
	      number_range (wield->value[1] * skill / 100,
			    wield->value[2] * skill / 100);

	  if (get_eq_char (ch, WEAR_SHIELD) == NULL)
	    dam = dam * 11 / 10;


	  if (IsWeaponStat (wield, WEAPON_SHARP))
	    {
	      int percent;

	      if ((percent = number_percent ()) <= (skill / 8))
		dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
      else
	dam =
	  number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
    }



  if (InStance (ch, STANCE_NORMAL))
    {
      if (IsNPC (ch))
	dam = dam * 113 / 100;
      else
	dam = dam * 115 / 100;
    }
  else
    dam = dambonus (ch, victim, dam, GetStance (ch, STANCE_CURRENT));

  if (get_skill (ch, gsn_enhanced_damage) > 0)
    {
      diceroll = number_percent ();
      if (diceroll <= get_skill (ch, gsn_enhanced_damage))
	{
	  check_improve (ch, gsn_enhanced_damage, true, 6);
	  dam += 2 * (dam * diceroll / 300);
	}
    }

  if (!IsAwake (victim))
    dam *= 2;
  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;

  if (dt == gsn_backstab && wield != NULL)
    {
      if (wield->value[0] != 2)
	dam *= 2 + (ch->level / 10);
      else
	dam *= 2 + (ch->level / 8);
    }

  dam += GetDamroll (ch) * Min (100, skill) / 100;

  if (dam <= 0)
    dam = 1;

  result = damage (ch, victim, dam, dt, dam_type, true);


  if (result && wield != NULL)
    {
      int pdam;

      if (ch->fighting == victim && IsWeaponStat (wield, WEAPON_POISON))
	{
	  int level;
	  AffectData *poison, af;

	  if ((poison = affect_find (wield->affect_first, gsn_poison)) ==
	      NULL)
	    level = wield->level;
	  else
	    level = poison->level;

	  if (!saves_spell (level / 2, victim, DAM_POISON))
	    {
	      chprintln (victim,
			 "You feel poison coursing through your veins.");
	      act ("$n is poisoned by the venom on $p.", victim, wield,
		   NULL, TO_ROOM);

	      af.where = TO_AFFECTS;
	      af.type = gsn_poison;
	      af.level = level * 3 / 4;
	      af.duration = level / 2;
	      af.location = APPLY_STR;
	      af.modifier = -1;
	      af.bitvector = AFF_POISON;
	      affect_join (victim, &af);
	    }


	  if (poison != NULL)
	    {
	      poison->level = Max (0, poison->level - 2);
	      poison->duration = Max (0, poison->duration - 1);

	      if (poison->level == 0 || poison->duration == 0)
		act ("The poison on $p has worn off.", ch,
		     wield, NULL, TO_CHAR);
	    }
	}

      if (ch->fighting == victim && IsWeaponStat (wield, WEAPON_VAMPIRIC))
	{
	  pdam = number_range (1, wield->level / 5 + 1);
	  act ("$p draws life from $n.", victim, wield, NULL, TO_ROOM);
	  act ("You feel $p drawing your life away.", victim, wield,
	       NULL, TO_CHAR);
	  damage (ch, victim, pdam, 0, DAM_NEGATIVE, false);
	  ch->alignment = Max (-1000, ch->alignment - 1);
	  ch->hit += pdam / 2;
	}

      if (ch->fighting == victim && IsWeaponStat (wield, WEAPON_FLAMING))
	{
	  pdam = number_range (1, wield->level / 4 + 1);
	  act ("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
	  act ("$p sears your flesh.", victim, wield, NULL, TO_CHAR);
	  fire_effect ((void *) victim, wield->level / 2, pdam, TARGET_CHAR);
	  damage (ch, victim, pdam, 0, DAM_FIRE, false);
	}

      if (ch->fighting == victim && IsWeaponStat (wield, WEAPON_FROST))
	{
	  pdam = number_range (1, wield->level / 6 + 2);
	  act ("$p freezes $n.", victim, wield, NULL, TO_ROOM);
	  act ("The cold touch of $p surrounds you with ice.",
	       victim, wield, NULL, TO_CHAR);
	  cold_effect (victim, wield->level / 2, pdam, TARGET_CHAR);
	  damage (ch, victim, pdam, 0, DAM_COLD, false);
	}

      if (ch->fighting == victim && IsWeaponStat (wield, WEAPON_SHOCKING))
	{
	  pdam = number_range (1, wield->level / 5 + 2);
	  act ("$n is struck by lightning from $p.", victim, wield,
	       NULL, TO_ROOM);
	  act ("You are shocked by $p.", victim, wield, NULL, TO_CHAR);
	  shock_effect (victim, wield->level / 2, pdam, TARGET_CHAR);
	  damage (ch, victim, pdam, 0, DAM_LIGHTNING, false);
	}
    }
  tail_chain ();
  return;
}

int
randomize_damage (CharData * ch, int dam, int am)
{
  dam = (dam * (am + 50)) / 100;
  return dam;
}


bool
damage (CharData * ch, CharData * victim, int dam, int dt,
	dam_class dam_type, bool show)
{
  ObjData *corpse;
  bool immune;

  if (victim->position == POS_DEAD)
    return false;


  if (dam > 10000 && dt >= TYPE_HIT)
    {
      bugf ("Damage: %d: more than 10000 points!", dam);
      dam = 10000;
      if (!IsImmortal (ch))
	{
	  ObjData *obj;

	  obj = get_eq_char (ch, WEAR_WIELD);
	  chprintln (ch, "You really shouldn't cheat.");
	  if (obj != NULL)
	    extract_obj (obj);
	}

    }


  if (dam > 35)
    dam = (dam - 35) / 2 + 35;
  if (dam > 80)
    dam = (dam - 80) / 2 + 80;

  if (!IsNPC (ch))
    {
      if (IsNPC (victim))
	dam *= mud_info.pcdam / 100;
    }
  else
    dam *= mud_info.mobdam / 100;

  if (victim != ch)
    {

      if (is_safe (ch, victim))
	return false;
      check_killer (ch, victim);

      if (victim->position > POS_STUNNED)
	{
	  if (victim->fighting == NULL)
	    {
	      set_fighting (victim, ch);
	      if (IsNPC (victim) && HasTriggerMob (victim, TRIG_KILL))
		p_percent_trigger (victim, NULL, NULL, ch, NULL, NULL,
				   TRIG_KILL);
	    }
	  if (victim->timer <= 4)
	    victim->position = POS_FIGHTING;
	}

      if (victim->position > POS_STUNNED)
	{
	  if (ch->fighting == NULL)
	    set_fighting (ch, victim);
	}


      if (victim->master == ch)
	stop_follower (victim);
    }


  if (IsAffected (ch, AFF_INVISIBLE))
    {
      affect_strip (ch, gsn_invis);
      affect_strip (ch, gsn_mass_invis);
      RemBit (ch->affected_by, AFF_INVISIBLE);
      act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }



  if (dam > 1 && !IsNPC (victim) &&
      victim->pcdata->condition[COND_DRUNK] > 10)
    dam = 9 * dam / 10;

  if (dam > 1 && IsAffected (victim, AFF_SANCTUARY))
    dam /= 2;

  if (dam > 1 &&
      ((IsAffected (victim, AFF_PROTECT_EVIL) && IsEvil (ch)) ||
       (IsAffected (victim, AFF_PROTECT_GOOD) && IsGood (ch))))
    dam -= dam / 4;

  if (mud_info.bonus.status == BONUS_DAM)
    dam *= mud_info.bonus.mod;

  immune = false;


  if (dt >= TYPE_HIT && ch != victim)
    {
      if (check_dodge (ch, victim))
	return false;
      if (InStance (victim, STANCE_MONGOOSE)
	  && GetStance (victim, STANCE_MONGOOSE) > 100 && !can_counter (ch)
	  && !can_bypass (ch, victim) && check_dodge (ch, victim))
	return false;
      else if (InStance (victim, STANCE_SWALLOW)
	       && GetStance (victim, STANCE_SWALLOW) > 100
	       && !can_counter (ch) && !can_bypass (ch, victim)
	       && check_dodge (ch, victim))
	return false;
      if (check_parry (ch, victim))
	return false;
      if (InStance (victim, STANCE_CRANE)
	  && GetStance (victim, STANCE_CRANE) > 100 && !can_counter (ch)
	  && !can_bypass (ch, victim) && check_parry (ch, victim))
	return false;
      else if (InStance (victim, STANCE_MANTIS)
	       && GetStance (victim, STANCE_MANTIS) > 100
	       && !can_counter (ch) && !can_bypass (ch, victim)
	       && check_parry (ch, victim))
	return false;
      if (check_shield_block (ch, victim))
	return false;

      if (IsAffected (victim, AFF_FORCE_SHIELD)
	  && check_force_shield (ch, victim))
	return false;
      if (IsAffected (victim, AFF_STATIC_SHIELD)
	  && check_static_shield (ch, victim))
	return false;
    }

  if (IsAffected (victim, AFF_FLAME_SHIELD) && dam_type <= 3)
    check_flame_shield (ch, victim);

  switch (check_immune (victim, dam_type))
    {
    case (IS_IMMUNE):
      immune = true;
      dam = 0;
      break;
    case (IS_RESISTANT):
      dam -= dam / 3;
      break;
    case (IS_VULNERABLE):
      dam += dam / 2;
      break;
    default:
      break;
    }

  randomize_damage (ch, dam, dice (1, 100));

  if (show)
    dam_message (ch, victim, dam, dt, immune);

  if (dam == 0)
    return false;


  victim->hit -= dam;
  if (!IsNPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  update_pos (victim);

  switch (victim->position)
    {
    case POS_MORTAL:
      act ("$n is mortally wounded, and will die soon, if not aided.",
	   victim, NULL, NULL, TO_ROOM);
      chprintln (victim,
		 "You are mortally wounded, and will die soon, if not aided.");
      break;

    case POS_INCAP:
      act ("$n is incapacitated and will slowly die, if not aided.",
	   victim, NULL, NULL, TO_ROOM);
      chprintln (victim,
		 "You are incapacitated and will slowly die, if not aided.");
      break;

    case POS_STUNNED:
      act ("$n is stunned, but will probably recover.", victim, NULL,
	   NULL, TO_ROOM);
      chprintln (victim, "You are stunned, but will probably recover.");
      break;

    case POS_DEAD:
      act ("$n is DEAD!!", victim, 0, 0, TO_ROOM);
      chprintln (victim, "You have been KILLED!!");
      break;

    default:
      if (dam > victim->max_hit / 4)
	chprintln (victim, "That really did HURT!");
      if (victim->hit < victim->max_hit / 4)
	chprintln (victim, "You sure are BLEEDING!");
      break;
    }


  if (!IsAwake (victim))
    stop_fighting (victim, false);


  if (victim->position == POS_DEAD)
    {
      if (IS_IN_ARENA (ch) && IS_IN_ARENA (victim))
	{
	  check_arena (ch, victim);
	  return true;
	}
      if (InWar (ch) && InWar (victim))
	{
	  check_war (ch, victim);
	  return true;
	}
      if (!IsNPC (ch) && !IsNPC (victim) &&
	  IsSet (ch->in_room->room_flags, ROOM_ARENA) &&
	  IsSet (victim->in_room->room_flags, ROOM_ARENA))
	{
	  stop_fighting (victim, true);
	  death_cry (victim);
	  char_from_room (victim);
	  char_to_room (victim, get_room_index (ROOM_VNUM_TEMPLE));
	  victim->hit = Max (1, victim->hit);
	  victim->mana = Max (1, victim->mana);
	  victim->move = Max (1, victim->move);
	  update_pos (victim);
	  do_function (victim, &do_look, "auto");
	  if (ch->in_room->area->nplayer == 1)
	    {
	      chprintln (ch, "You emerge victorious in the arena!");
	      stop_fighting (ch, true);
	      char_from_room (ch);
	      char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
	      ch->hit = ch->max_hit;
	      ch->mana = ch->max_mana;
	      ch->move = ch->max_move;
	      update_pos (ch);
	      do_function (ch, &do_look, "auto");
	    }
	  return true;
	}

      group_gain (ch, victim);

      if (!IsNPC (victim))
	{
	  logf ("%s killed by %s at %ld", victim->name,
		(IsNPC (ch) ? ch->short_descr : ch->name), ch->in_room->vnum);


	  if (!IsQuester (victim) || ch != victim->pcdata->quest.mob)
	    {
	      if (victim->exp >
		  exp_per_level (victim,
				 victim->pcdata->points) * victim->level)
		gain_exp (victim,
			  (2 *
			   (exp_per_level
			    (victim,
			     victim->pcdata->points) *
			    victim->level - victim->exp) / 3) + 50);
	    }
	}

      if (IsNPC (victim))
	new_wiznet (ch, NULL, WIZ_MOBDEATHS, false, 0,
		    "%s got toasted by $N at %s [room %ld]",
		    victim->short_descr,
		    ch->in_room->name, ch->in_room->vnum);
      else
	{
	  new_wiznet (victim, NULL, WIZ_DEATHS, false, 0,
		      "%s killed by $N at %s [room %ld]", victim->name,
		      ch->in_room->name, ch->in_room->vnum);
	  announce (victim, INFO_DEATH, "$n got %s by %s!",
		    chance (50) ? "wasted" : "anihilated",
		    IsNPC (ch) ? ch->short_descr : ch->name);
	}


      if (IsNPC (victim) && HasTriggerMob (victim, TRIG_DEATH))
	{
	  victim->position = POS_STANDING;
	  p_percent_trigger (victim, NULL, NULL, ch, NULL, NULL, TRIG_DEATH);
	}

      update_death (victim, ch);

      raw_kill (victim, ch);


      if (ch != victim && !IsNPC (ch) && !is_same_clan (ch, victim))
	{
	  if (IsSet (victim->act, PLR_KILLER))
	    RemBit (victim->act, PLR_KILLER);
	  else
	    RemBit (victim->act, PLR_THIEF);
	}



      if (!IsNPC (ch) &&
	  (corpse =
	   get_obj_list (ch, "corpse",
			 ch->in_room->content_first)) != NULL
	  && corpse->item_type == ITEM_CORPSE_NPC && can_see_obj (ch, corpse))
	{
	  ObjData *coins;

	  corpse = get_obj_list (ch, "corpse", ch->in_room->content_first);

	  if (IsSet (ch->act, PLR_AUTOLOOT) && corpse
	      && corpse->content_first)
	    {
	      do_function (ch, &do_get, "all corpse");
	    }

	  if (IsSet (ch->act, PLR_AUTOGOLD) && corpse
	      && corpse->content_first && !IsSet (ch->act, PLR_AUTOLOOT))
	    {
	      if ((coins =
		   get_obj_list (ch, "gcash", corpse->content_first)) != NULL)
		{
		  do_function (ch, &do_get, "all.gcash corpse");
		}
	    }

	  if (IsSet (ch->act, PLR_AUTOSAC))
	    {
	      if (IsSet (ch->act, PLR_AUTOLOOT) && corpse
		  && corpse->content_first)
		{
		  return true;
		}
	      else
		{
		  do_function (ch, &do_sacrifice, "corpse");
		}
	    }
	}

      return true;
    }

  if (victim == ch)
    return true;


  if (!IsNPC (victim) && victim->desc == NULL)
    {
      if (number_range (0, victim->wait) == 0)
	{
	  perform_recall (victim, get_room_index (ROOM_VNUM_LIMBO), "recall");
	  return true;
	}
    }


  if (IsNPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
      if ((IsSet (victim->act, ACT_WIMPY) && number_bits (2) == 0 &&
	   victim->hit < victim->max_hit / 5) ||
	  (IsAffected (victim, AFF_CHARM) && victim->master != NULL &&
	   victim->master->in_room != victim->in_room))
	{
	  do_function (victim, &do_flee, "");
	}
    }

  if (!IsNPC (victim) && victim->hit > 0 && victim->hit <= victim->wimpy
      && victim->wait < PULSE_VIOLENCE / 2)
    {
      do_function (victim, &do_flee, "");
    }

  tail_chain ();
  return true;
}

bool
is_safe (CharData * ch, CharData * victim)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return true;

  if (victim->fighting == ch || victim == ch)
    return false;

  if (IsImmortal (ch) && ch->level > LEVEL_IMMORTAL)
    return false;


  if (IsNPC (victim))
    {


      if (IsSet (victim->in_room->room_flags, ROOM_SAFE))
	{
	  chprintln (ch, "Not in this room.");
	  return true;
	}

      if (victim->pIndexData->pShop != NULL)
	{
	  chprintln (ch, "The shopkeeper wouldn't like that.");
	  return true;
	}


      if (IsSet (victim->act, ACT_TRAIN) ||
	  IsSet (victim->act, ACT_PRACTICE) ||
	  IsSet (victim->act, ACT_IS_HEALER) ||
	  IsSet (victim->act, ACT_IS_CHANGER))
	{
	  act ("I don't think $g would approve.", ch, NULL, NULL, TO_CHAR);
	  return true;
	}

      if (!IsNPC (ch))
	{

	  if (IsSet (victim->act, ACT_PET))
	    {
	      act ("But $N looks so cute and cuddly...", ch,
		   NULL, victim, TO_CHAR);
	      return true;
	    }


	  if (IsAffected (victim, AFF_CHARM) && ch != victim->master)
	    {
	      chprintln (ch, "You don't own that monster.");
	      return true;
	    }
	  if (IsQuester (ch) && ch->pcdata->quest.mob == victim &&
	      (ch->pcdata->quest.status == QUEST_DELIVER ||
	       ch->pcdata->quest.status == QUEST_FINDMOB))
	    {
	      act ("You are supposed to deliver $p to $N, not kill $M.",
		   ch, ch->pcdata->quest.obj, victim, TO_CHAR);
	      return true;
	    }
	}
    }

  else
    {

      if (IsNPC (ch))
	{

	  if (IsSet (victim->in_room->room_flags, ROOM_SAFE))
	    {
	      chprintln (ch, "Not in this room.");
	      return true;
	    }


	  if (IsAffected (ch, AFF_CHARM) && ch->master != NULL &&
	      ch->master->fighting != victim)
	    {
	      chprintln (ch, "Players are your friends!");
	      return true;
	    }
	}

      else
	{

	  if (is_safe_war (ch, victim))
	    {
	      chprintln (ch, "They're on YOUR team.");
	      return true;
	    }

	  if (IsSet (victim->in_room->room_flags, ROOM_ARENA))
	    return false;

	  if (!is_clan (ch))
	    {
	      chprintln (ch, "Join a clan if you want to kill players.");
	      return true;
	    }

	  if (IsSet (victim->act, PLR_KILLER) ||
	      IsSet (victim->act, PLR_THIEF))
	    return false;

	  if (!is_clan (victim))
	    {
	      chprintln (ch, "They aren't in a clan, leave them alone.");
	      return true;
	    }

	  if (IsSet (victim->in_room->room_flags, ROOM_SAFE))
	    {
	      chprintln (ch, "Not in this room.");
	      return true;
	    }

	  if (ch->level > victim->level + 8)
	    {
	      chprintln (ch, "Pick on someone your own size.");
	      return true;
	    }
	  if (Gquester (ch))
	    {
	      chprintln (ch, "I don't beleive they are on the target list.");
	      return true;
	    }

	  if (Gquester (victim))
	    {
	      chprintln (ch,
			 "They are to closey involved in something right now.");
	      return true;
	    }

	}
    }
  return false;
}

bool
is_safe_spell (CharData * ch, CharData * victim, bool area)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return true;

  if (victim == ch && area)
    return true;

  if (victim->fighting == ch || victim == ch)
    return false;

  if (IsImmortal (ch) && ch->level > LEVEL_IMMORTAL && !area)
    return false;


  if (IsNPC (victim))
    {

      if (IsSet (victim->in_room->room_flags, ROOM_SAFE))
	return true;

      if (victim->pIndexData->pShop != NULL)
	return true;


      if (IsSet (victim->act, ACT_TRAIN) ||
	  IsSet (victim->act, ACT_PRACTICE) ||
	  IsSet (victim->act, ACT_IS_HEALER) ||
	  IsSet (victim->act, ACT_IS_CHANGER))
	return true;

      if (!IsNPC (ch))
	{

	  if (IsSet (victim->act, ACT_PET))
	    return true;


	  if (IsAffected (victim, AFF_CHARM) &&
	      (area || ch != victim->master))
	    return true;


	  if (victim->fighting != NULL &&
	      !is_same_group (ch, victim->fighting))
	    return true;

	  if (IsQuester (ch) && ch->pcdata->quest.mob == victim &&
	      (ch->pcdata->quest.status == QUEST_DELIVER ||
	       ch->pcdata->quest.status == QUEST_FINDMOB))
	    return true;
	}
      else
	{

	  if (area && !is_same_group (victim, ch->fighting))
	    return true;
	}
    }

  else
    {
      if (area && IsImmortal (victim) && victim->level > LEVEL_IMMORTAL)
	return true;


      if (IsNPC (ch))
	{

	  if (IsAffected (ch, AFF_CHARM) && ch->master != NULL &&
	      ch->master->fighting != victim)
	    return true;


	  if (IsSet (victim->in_room->room_flags, ROOM_SAFE))
	    return true;


	  if (ch->fighting != NULL && !is_same_group (ch->fighting, victim))
	    return true;
	}


      else
	{
	  if (is_safe_war (ch, victim))
	    return true;

	  if (IsSet (victim->in_room->room_flags, ROOM_ARENA))
	    return false;

	  if (!is_clan (ch))
	    return true;

	  if (IsSet (victim->act, PLR_KILLER) ||
	      IsSet (victim->act, PLR_THIEF))
	    return false;

	  if (!is_clan (victim))
	    return true;

	  if (ch->level > victim->level + 8)
	    return true;

	  if (Gquester (ch) || Gquester (victim))
	    return true;

	}

    }
  return false;
}


void
check_killer (CharData * ch, CharData * victim)
{
  char buf[MAX_STRING_LENGTH];

  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    return;

  while (IsAffected (victim, AFF_CHARM) && victim->master != NULL)
    victim = victim->master;


  if (IsNPC (victim) || IsSet (victim->act, PLR_KILLER) ||
      IsSet (victim->act, PLR_THIEF))
    return;


  if (IsSet (ch->affected_by, AFF_CHARM))
    {
      if (ch->master == NULL)
	{
	  sprintf (buf, "Check_killer: %s bad AFF_CHARM",
		   IsNPC (ch) ? ch->short_descr : ch->name);
	  bug (buf);
	  affect_strip (ch, gsn_charm_person);
	  RemBit (ch->affected_by, AFF_CHARM);
	  return;
	}


      stop_follower (ch);
      return;
    }


  if (IsNPC (ch) || ch == victim || ch->level >= LEVEL_IMMORTAL ||
      !is_clan (ch) || IsSet (ch->act, PLR_KILLER) || ch->fighting == victim)
    return;

  chprintln (ch, "*** You are now a KILLER!! ***");
  SetBit (ch->act, PLR_KILLER);
  new_wiznet (ch, NULL, WIZ_FLAGS, true, 0,
	      "$N is attempting to murder %s", GetName (victim));
  save_char_obj (ch);
  return;
}


bool
check_parry (CharData * ch, CharData * victim)
{
  int chance;

  if (!IsAwake (victim))
    return false;

  chance = get_skill (victim, gsn_parry) / 2;

  if (get_eq_char (victim, WEAR_WIELD) == NULL)
    {
      if (IsNPC (victim))
	chance /= 2;
      else
	return false;
    }

  if (!can_see (ch, victim))
    chance /= 2;

  if (InStance (victim, STANCE_CRANE)
      && GetStance (victim, STANCE_CRANE) > 0 && !can_counter (ch)
      && !can_bypass (ch, victim))
    chance += (GetStance (victim, STANCE_CRANE) * 25 / 100);
  else if (InStance (victim, STANCE_MANTIS)
	   && GetStance (victim, STANCE_MANTIS) > 0 && !can_counter (ch)
	   && !can_bypass (ch, victim))
    chance += (GetStance (victim, STANCE_MANTIS) * 25 / 100);

  if (number_percent () >= chance + victim->level - ch->level)
    return false;

  act ("You parry $n's attack.", ch, NULL, victim, TO_VICT);
  act ("$N parries your attack.", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_parry, true, 6);
  return true;
}


bool
check_shield_block (CharData * ch, CharData * victim)
{
  int chance;

  if (!IsAwake (victim))
    return false;

  chance = get_skill (victim, gsn_shield_block) / 5 + 3;

  if (get_eq_char (victim, WEAR_SHIELD) == NULL)
    return false;

  if (number_percent () >= chance + victim->level - ch->level)
    return false;

  act ("You block $n's attack with your shield.", ch, NULL, victim, TO_VICT);
  act ("$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_shield_block, true, 6);
  return true;
}


bool
check_dodge (CharData * ch, CharData * victim)
{
  int chance;

  if (!IsAwake (victim))
    return false;

  chance = get_skill (victim, gsn_dodge) / 2;

  if (!can_see (victim, ch))
    chance /= 2;

  if (InStance (victim, STANCE_MONGOOSE)
      && GetStance (victim, STANCE_MONGOOSE) > 0 && !can_counter (ch)
      && !can_bypass (ch, victim))
    (chance += GetStance (victim, STANCE_MONGOOSE) * 25 / 100);
  if (InStance (victim, STANCE_SWALLOW)
      && GetStance (victim, STANCE_SWALLOW) > 0 && !can_counter (ch)
      && !can_bypass (ch, victim))
    (chance += GetStance (victim, STANCE_SWALLOW) * 25 / 100);

  if (number_percent () >= chance + victim->level - ch->level)
    return false;

  act ("You dodge $n's attack.", ch, NULL, victim, TO_VICT);
  act ("$N dodges your attack.", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_dodge, true, 6);
  return true;
}


void
update_pos (CharData * victim)
{
  if (victim->hit > 0)
    {
      if (victim->position <= POS_STUNNED)
	victim->position = POS_STANDING;
      return;
    }

  if (IsNPC (victim) && victim->hit < 1)
    {
      victim->position = POS_DEAD;
      return;
    }

  if (victim->hit <= -11)
    {
      victim->position = POS_DEAD;
      return;
    }

  if (victim->hit <= -6)
    victim->position = POS_MORTAL;
  else if (victim->hit <= -3)
    victim->position = POS_INCAP;
  else
    victim->position = POS_STUNNED;

  return;
}


void
set_fighting (CharData * ch, CharData * victim)
{
  if (ch->fighting != NULL)
    {
      bug ("Set_fighting: already fighting");
      return;
    }

  if (IsAffected (ch, AFF_SLEEP))
    affect_strip (ch, gsn_sleep);

  ch->fighting = victim;
  ch->position = POS_FIGHTING;
  autodrop (ch);

  return;
}


void
stop_fighting (CharData * ch, bool fBoth)
{
  CharData *fch;

  for (fch = char_first; fch != NULL; fch = fch->next)
    {
      if (fch == ch || (fBoth && fch->fighting == ch))
	{
	  fch->fighting = NULL;
	  fch->position = IsNPC (fch) ? fch->default_pos : POS_STANDING;
	  update_pos (fch);
	  SetStance (fch, STANCE_CURRENT, STANCE_NONE);
	}
    }
  return;
}


void
make_corpse (CharData * ch)
{
  char buf[MAX_STRING_LENGTH];
  ObjData *corpse;
  ObjData *obj;
  ObjData *obj_next;
  const char *name;
  RoomIndex *morgue = NULL;

  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    return;

  if (IsNPC (ch))
    {
      name = ch->short_descr;
      corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
      corpse->timer = number_range (3, 6);
      if (ch->gold > 0)
	{
	  obj_to_obj (create_money (ch->gold, ch->silver), corpse);
	  ch->gold = 0;
	  ch->silver = 0;
	}
      corpse->cost = 0;
    }
  else
    {
      name = ch->name;
      corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
      corpse->timer = number_range (25, 40);
      RemBit (ch->act, PLR_CANLOOT);
      corpse->owner = str_dup (ch->name);
      if (is_clan (ch))
	{
	  if (ch->gold > 1 || ch->silver > 1)
	    {
	      obj_to_obj (create_money (ch->gold / 2, ch->silver / 2),
			  corpse);
	      ch->gold -= ch->gold / 2;
	      ch->silver -= ch->silver / 2;
	    }
	}

      corpse->cost = 0;
    }

  corpse->level = ch->level;

  sprintf (buf, corpse->short_descr, name);
  replace_str (&corpse->short_descr, buf);

  sprintf (buf, corpse->description, name);
  replace_str (&corpse->description, buf);

  for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
    {
      bool floating = false;

      obj_next = obj->next_content;
      if (obj->wear_loc == WEAR_FLOAT)
	floating = true;
      obj_from_char (obj);
      if (obj->item_type == ITEM_POTION)
	obj->timer = number_range (500, 1000);
      if (obj->item_type == ITEM_SCROLL)
	obj->timer = number_range (1000, 2500);
      if (IsSet (obj->extra_flags, ITEM_ROT_DEATH) && !floating)
	{
	  obj->timer = number_range (5, 10);
	  RemBit (obj->extra_flags, ITEM_ROT_DEATH);
	}
      RemBit (obj->extra_flags, ITEM_VIS_DEATH);

      if (IsSet (obj->extra_flags, ITEM_INVENTORY))
	extract_obj (obj);
      else if (floating)
	{
	  if (IsObjStat (obj, ITEM_ROT_DEATH))
	    {
	      if (obj->content_first != NULL)
		{
		  ObjData *in, *in_next;

		  act ("$p evaporates,scattering its contents.",
		       ch, obj, NULL, TO_ROOM);
		  for (in = obj->content_first; in != NULL; in = in_next)
		    {
		      in_next = in->next_content;
		      obj_from_obj (in);
		      obj_to_room (in, ch->in_room);
		    }
		}
	      else
		act ("$p evaporates.", ch, obj, NULL, TO_ROOM);
	      extract_obj (obj);
	    }
	  else
	    {
	      act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
	      obj_to_room (obj, ch->in_room);
	    }
	}
      else
	obj_to_obj (obj, corpse);
    }

  if (!is_clan (ch) || IsNPC (ch))
    {
      if (IsNPC (ch) || get_trust (ch) >= 20)
	morgue = ch->in_room;
      else
	morgue = get_room_index (ROOM_VNUM_MORGUE);
    }
  else if ((morgue =
	    get_room_index (CharClan (ch)->rooms[CLAN_ROOM_MORGUE])) == NULL)
    morgue = get_room_index (ROOM_VNUM_MORGUE);

  obj_to_room (corpse, ch->in_room);
  return;
}


void
death_cry (CharData * ch)
{
  RoomIndex *was_in_room;
  char *msg;
  int door;
  vnum_t vnum;

  vnum = 0;
  msg = "You hear $n's death cry.";

  switch (number_bits (4))
    {
    case 0:
      msg = "$n hits the ground ... DEAD.";
      break;
    case 1:
      if (ch->material == 0)
	{
	  msg = "$n splatters blood on your armor.";
	  break;
	}
    case 2:
      if (IsSet (ch->parts, PART_GUTS))
	{
	  msg = "$n spills $s guts all over the floor.";
	  vnum = OBJ_VNUM_GUTS;
	}
      break;
    case 3:
      if (IsSet (ch->parts, PART_HEAD))
	{
	  msg = "$n's severed head plops on the ground.";
	  vnum = OBJ_VNUM_SEVERED_HEAD;
	}
      break;
    case 4:
      if (IsSet (ch->parts, PART_HEART))
	{
	  msg = "$n's heart is torn from $s chest.";
	  vnum = OBJ_VNUM_TORN_HEART;
	}
      break;
    case 5:
      if (IsSet (ch->parts, PART_ARMS))
	{
	  msg = "$n's arm is sliced from $s dead body.";
	  vnum = OBJ_VNUM_SLICED_ARM;
	}
      break;
    case 6:
      if (IsSet (ch->parts, PART_LEGS))
	{
	  msg = "$n's leg is sliced from $s dead body.";
	  vnum = OBJ_VNUM_SLICED_LEG;
	}
      break;
    case 7:
      if (IsSet (ch->parts, PART_BRAINS))
	{
	  msg = "$n's head is shattered, and $s brains splash all over you.";
	  vnum = OBJ_VNUM_BRAINS;
	}
    }

  act (msg, ch, NULL, NULL, TO_ROOM);

  if (vnum != 0)
    {
      char buf[MAX_STRING_LENGTH];
      ObjData *obj;
      const char *name;

      name = IsNPC (ch) ? ch->short_descr : ch->name;
      obj = create_object (get_obj_index (vnum), 0);
      obj->timer = number_range (4, 7);

      sprintf (buf, obj->short_descr, name);
      replace_str (&obj->short_descr, buf);

      sprintf (buf, obj->description, name);
      replace_str (&obj->description, buf);

      if (obj->item_type == ITEM_FOOD)
	{
	  if (IsSet (ch->form, FORM_POISON))
	    obj->value[3] = 1;
	  else if (!IsSet (ch->form, FORM_EDIBLE))
	    obj->item_type = ITEM_TRASH;
	}

      obj_to_room (obj, ch->in_room);
    }

  if (IsNPC (ch))
    msg = "You hear something's death cry.";
  else
    msg = "You hear someone's death cry.";

  was_in_room = ch->in_room;
  for (door = 0; door <= 5; door++)
    {
      ExitData *pexit;

      if ((pexit = was_in_room->exit[door]) != NULL &&
	  pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
	{
	  ch->in_room = pexit->u1.to_room;
	  act (msg, ch, NULL, NULL, TO_ROOM);
	}
    }
  ch->in_room = was_in_room;

  return;
}


void
update_death (CharData * victim, CharData * killer)
{
  MspData *snd;

  if (victim == killer)
    return;

  snd = new_msp ();
  snd->type = MSP_COMBAT;

  if (!IsNPC (victim))
    {
      if (IsNPC (killer))
	{
	  kill_table[Range (0, killer->level, MAX_LEVEL - 1)].kills++;
	  killer->pIndexData->kills++;
	  TouchArea (killer->pIndexData->area);
	  AddStat (victim, MOB_DEATHS, 1);
	  mud_info.stats.pdied++;
	}
      else
	{
	  if (!IsImmortal (killer))
	    {
	      AddStat (victim, PK_DEATHS, 1);
	    }
	  AddStat (killer, PK_KILLS, 1);
	  mud_info.stats.pkill++;
	}
      snd->file = str_dup ("deathpc*");
      act_sound (snd, victim, NULL, TO_ZONE, POS_RESTING);
      victim->in_room->area->kills++;
      TouchArea (victim->in_room->area);
    }
  else
    {
      if (!IsNPC (killer))
	{
	  AddStat (killer, MOB_KILLS, 1);
	  snd->file = str_dup ("deathmob*");
	  act_sound (snd, victim, NULL, TO_ROOM, POS_RESTING);
	}
      else
	{
	  kill_table[Range (0, killer->level, MAX_LEVEL - 1)].kills++;
	  killer->pIndexData->kills++;
	  TouchArea (killer->pIndexData->area);
	}
      victim->pIndexData->deaths++;
      TouchArea (victim->pIndexData->area);
      kill_table[Range (0, victim->level, MAX_LEVEL - 1)].deaths++;
      mud_info.stats.mobdeaths++;
      if (mud_info.stats.mobdeaths % 1000000 == 0)
	{
	  set_bonus (BONUS_XP, 2, 50, "in honour of %s killing the %s mob",
		     GetName (killer),
		     ordinal_string (mud_info.stats.mobdeaths));
	}
      victim->in_room->area->deaths++;
      TouchArea (victim->in_room->area);
    }
  free_msp (snd);
}

void
raw_kill (CharData * victim, CharData * killer)
{
  int i;

  stop_fighting (victim, true);
  death_cry (victim);
  make_corpse (victim);

  if (IsNPC (victim))
    {
      extract_char (victim, true);
      return;
    }

  extract_char (victim, false);
  while (victim->affect_first)
    affect_remove (victim, victim->affect_first);
  victim->affected_by = victim->race->aff;
  for (i = 0; i < MAX_AC; i++)
    victim->armor[i] = 100;
  victim->position = POS_RESTING;
  victim->hit = Max (1, victim->hit);
  victim->mana = Max (1, victim->mana);
  victim->move = Max (1, victim->move);

  update_all_qobjs (victim);
  return;
}

void
group_gain (CharData * ch, CharData * victim)
{
  CharData *gch;
  CharData *lch;
  int xp;
  int members;
  int group_levels;
  int i;
  int highestlevel = 0;


  if (victim == ch)
    return;

  members = 0;
  group_levels = 0;
  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      if (is_same_group (gch, ch))
	{
	  members++;
	  group_levels += IsNPC (gch) ? gch->level / 2 : gch->level;
	}
    }

  if (members == 0)
    {
      bug ("Group_gain: members == 0.");
      members = 1;
      group_levels = ch->level;
    }

  for (lch = ch->in_room->person_first; lch != NULL; lch = lch->next_in_room)
    {
      if (!is_same_group (lch, ch))
	continue;
      if (lch->level > highestlevel)
	highestlevel = lch->level;
    }

  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      ObjData *obj;
      ObjData *obj_next;

      if (!is_same_group (gch, ch) || IsNPC (gch))
	continue;


      if (highestlevel - gch->level >= mud_info.group_lvl_limit ||
	  highestlevel - gch->level <= (mud_info.group_lvl_limit * -1))
	{
	  chprintln
	    (gch,
	     "Your powers are useless to such an advanced group of adventurers.");
	  if (IsNPC (gch) && gch->master != NULL)
	    act
	      ("$n's powers are useless to such an advanced group of adventurers.",
	       gch, NULL, gch->master, TO_VICT);
	  continue;
	}

      xp = xp_compute (gch, victim, group_levels);


      if (mud_info.bonus.status == BONUS_XP)
	{
	  xp = xp * mud_info.bonus.mod;
	  if (!NullStr (mud_info.bonus.msg))
	    {
	      chprintlnf
		(gch, "{GYou receive %d %dx exp %s!{x",
		 xp, mud_info.bonus.mod, mud_info.bonus.msg);
	    }
	  else
	    chprintlnf (gch, "{GYou receive %d %dx exp!{x", xp,
			mud_info.bonus.mod);
	}
      else
	{
	  xp = xp * 1;
	  chprintlnf (gch, "You receive %d experience points.", xp);
	}
      gain_exp (gch, xp);

      if (IsQuester (gch) && gch->pcdata->quest.mob == victim)
	{
	  if (ch->pcdata->quest.status == QUEST_DELIVER)
	    {
	      act
		("{rOOPS! Now you did it! You were supposed to deliver $p to $N!",
		 ch, ch->pcdata->quest.obj, victim, TO_CHAR);
	      act
		("You just lost {R50{r questpoints and $N is very mad!{x",
		 ch, NULL, ch->pcdata->quest.giver, TO_CHAR);
	      end_quest (ch, QUEST_TIME + 10);
	      ch->pcdata->quest.points =
		Max (0, ch->pcdata->quest.points - 50);
	    }
	  else if (ch->pcdata->quest.status == QUEST_KILL)
	    {
	      chprintln (gch, "{5+RYou have almost completed your QUEST!{x");
	      act ("{RReturn to $N before your time runs out!", gch, NULL,
		   gch->pcdata->quest.giver, TO_CHAR);
	      gch->pcdata->quest.status = QUEST_RETURN_KILL;
	    }
	}

      if (IsNPC (victim) &&
	  gquest_info.running == GQUEST_RUNNING && Gquester (gch) &&
	  (i = is_gqmob (gch->gquest, victim->pIndexData->vnum)) != -1)
	{
	  gch->gquest->gq_mobs[i] = -1;
	  chprintln
	    (gch,
	     "Congratulations, that that mob was part of your global quest!");
	  chprint (gch, "You receive an extra 3 Quest Points");
	  gch->pcdata->quest.points += 3;
	  if (chance (Range (5, gquest_info.mob_count, 95)))
	    {
	      chprintln (gch, " and a Trivia Point!");
	      gch->pcdata->trivia += 1;
	    }
	  else
	    chprintln (gch, ".");

	  new_wiznet (gch, victim->short_descr, 0, false, 0,
		      "$N has killed $t, a global questmob.");

	  if (count_gqmobs (gch->gquest) == gquest_info.mob_count)
	    chprintln (gch,
		       "You are now ready to complete the global quest. Type 'Gquest COMPLETE' to finish.");
	}

      for (obj = gch->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (obj->wear_loc == WEAR_NONE)
	    continue;

	  if ((IsObjStat (obj, ITEM_ANTI_EVIL) && IsEvil (ch)) ||
	      (IsObjStat (obj, ITEM_ANTI_GOOD) && IsGood (ch)) ||
	      (IsObjStat (obj, ITEM_ANTI_NEUTRAL) && IsNeutral (ch)))
	    {
	      act ("You are zapped by $p.", gch, obj, NULL, TO_CHAR);
	      act ("$n is zapped by $p.", gch, obj, NULL, TO_ROOM);
	      obj_from_char (obj);
	      obj_to_room (obj, gch->in_room);
	    }
	}
    }

  return;
}


int
xp_compute (CharData * gch, CharData * victim, int total_levels)
{
  int xp, base_exp;
  int align, level_range;
  int change;
  int time_per_level;

  level_range = victim->level - gch->level;


  switch (level_range)
    {
    default:
      base_exp = 0;
      break;
    case -9:
      base_exp = 1;
      break;
    case -8:
      base_exp = 2;
      break;
    case -7:
      base_exp = 5;
      break;
    case -6:
      base_exp = 9;
      break;
    case -5:
      base_exp = 11;
      break;
    case -4:
      base_exp = 22;
      break;
    case -3:
      base_exp = 33;
      break;
    case -2:
      base_exp = 50;
      break;
    case -1:
      base_exp = 66;
      break;
    case 0:
      base_exp = 83;
      break;
    case 1:
      base_exp = 99;
      break;
    case 2:
      base_exp = 121;
      break;
    case 3:
      base_exp = 143;
      break;
    case 4:
      base_exp = 165;
      break;
    }

  if (level_range > 4)
    base_exp = 160 + 20 * (level_range - 4);



  align = victim->alignment - gch->alignment;

  if (IsSet (victim->act, ACT_NOALIGN))
    {

    }
  else if (align > 500)
    {
      change = (align - 500) * base_exp / 500 * gch->level / total_levels;
      change = Max (1, change);
      gch->alignment = Max (-1000, gch->alignment - change);
    }
  else if (align < -500)
    {
      change =
	(-1 * align - 500) * base_exp / 500 * gch->level / total_levels;
      change = Max (1, change);
      gch->alignment = Min (1000, gch->alignment + change);
    }
  else
    {

      change = gch->alignment * base_exp / 500 * gch->level / total_levels;
      gch->alignment -= change;
    }


  if (IsSet (victim->act, ACT_NOALIGN))
    xp = base_exp;

  else if (gch->alignment > 500)
    {
      if (victim->alignment < -750)
	xp = (base_exp * 4) / 3;

      else if (victim->alignment < -500)
	xp = (base_exp * 5) / 4;

      else if (victim->alignment > 750)
	xp = base_exp / 4;

      else if (victim->alignment > 500)
	xp = base_exp / 2;

      else if (victim->alignment > 250)
	xp = (base_exp * 3) / 4;

      else
	xp = base_exp;
    }
  else if (gch->alignment < -500)
    {
      if (victim->alignment > 750)
	xp = (base_exp * 5) / 4;

      else if (victim->alignment > 500)
	xp = (base_exp * 11) / 10;

      else if (victim->alignment < -750)
	xp = base_exp / 2;

      else if (victim->alignment < -500)
	xp = (base_exp * 3) / 4;

      else if (victim->alignment < -250)
	xp = (base_exp * 9) / 10;

      else
	xp = base_exp;
    }
  else if (gch->alignment > 200)
    {

      if (victim->alignment < -500)
	xp = (base_exp * 6) / 5;

      else if (victim->alignment > 750)
	xp = base_exp / 2;

      else if (victim->alignment > 0)
	xp = (base_exp * 3) / 4;

      else
	xp = base_exp;
    }
  else if (gch->alignment < -200)
    {
      if (victim->alignment > 500)
	xp = (base_exp * 6) / 5;

      else if (victim->alignment < -750)
	xp = base_exp / 2;

      else if (victim->alignment < 0)
	xp = (base_exp * 3) / 4;

      else
	xp = base_exp;
    }
  else
    {


      if (victim->alignment > 500 || victim->alignment < -500)
	xp = (base_exp * 4) / 3;

      else if (victim->alignment < 200 && victim->alignment > -200)
	xp = base_exp / 2;

      else
	xp = base_exp;
    }


  if (gch->level < 6)
    xp = 10 * xp / (gch->level + 4);


  if (gch->level > 35)
    xp = 15 * xp / (gch->level - 25);



  {

    time_per_level =
      4 * (gch->pcdata->played +
	   (int) (current_time - gch->logon)) / HOUR / gch->level;

    time_per_level = Range (2, time_per_level, 12);
    if (gch->level < 15)
      time_per_level = Max (time_per_level, (15 - gch->level));
    xp = xp * time_per_level / 12;
  }


  xp = number_range (xp * 3 / 4, xp * 5 / 4);


  xp = xp * gch->level / (Max (1, total_levels - 1));

  return xp;
}

void
dam_message (CharData * ch, CharData * victim, int dam, int dt, bool immune)
{
  char buf1[256], buf2[256], buf3[256], chmesg[256], vmesg[256], omesg[256];
  const char *vs;
  const char *vp;
  const char *attack;
  const char *punct;

  if (ch == NULL || victim == NULL)
    return;

  sprintf (chmesg, "{W [{R%d{W]{x", dam);
  sprintf (vmesg, "{W [{R%d{W]{x", dam);
  sprintf (omesg, "{W [{R%d{W]{x", dam);

  if (dam == 0)
    {
      vs = "miss";
      vp = "misses";
    }
  else if (dam <= 4)
    {
      vs = "scratch";
      vp = "scratches";
    }
  else if (dam <= 8)
    {
      vs = "graze";
      vp = "grazes";
    }
  else if (dam <= 12)
    {
      vs = "hit";
      vp = "hits";
    }
  else if (dam <= 16)
    {
      vs = "injure";
      vp = "injures";
    }
  else if (dam <= 20)
    {
      vs = "wound";
      vp = "wounds";
    }
  else if (dam <= 24)
    {
      vs = "maul";
      vp = "mauls";
    }
  else if (dam <= 28)
    {
      vs = "decimate";
      vp = "decimates";
    }
  else if (dam <= 32)
    {
      vs = "devastate";
      vp = "devastates";
    }
  else if (dam <= 36)
    {
      vs = "maim";
      vp = "maims";
    }
  else if (dam <= 40)
    {
      vs = "MUTILATE";
      vp = "MUTILATES";
    }
  else if (dam <= 44)
    {
      vs = "DISEMBOWEL";
      vp = "DISEMBOWELS";
    }
  else if (dam <= 48)
    {
      vs = "DISMEMBER";
      vp = "DISMEMBERS";
    }
  else if (dam <= 52)
    {
      vs = "MASSACRE";
      vp = "MASSACRES";
    }
  else if (dam <= 56)
    {
      vs = "MANGLE";
      vp = "MANGLES";
    }
  else if (dam <= 60)
    {
      vs = "{b*** {BDEMOLISH {b***{x";
      vp = "{b*** {BDEMOLISHES {b***{x";
    }
  else if (dam <= 75)
    {
      vs = "{m*** {MDEVASTATE {m***{x";
      vp = "{m*** {MDEVASTATES {m***{x";
    }
  else if (dam <= 100)
    {
      vs = "{c=== {COBLITERATE {c==={x";
      vp = "{c=== {COBLITERATES {c==={x";
    }
  else if (dam <= 125)
    {
      vs = "{R>>> {YANNIHILATE {R<<<{x";
      vp = "{R>>> {YANNIHILATES {R<<<{x";
    }
  else if (dam <= 150)
    {
      vs = "{Y<<< {RERADICATE {Y>>>{x";
      vp = "{Y<<< {RERADICATES {Y>>>{x";
    }
  else if (dam <= 185)
    {
      vs = "{W***** {CPULVERIZE {W*****{x";
      vp = "{W***** {CPULVERIZES {W*****{x";
    }
  else if (dam <= 220)
    {
      vs = "{B-=- VAPORIZE -=-{x";
      vp = "{B-=- VAPORIZES -=-{x";
    }
  else if (dam <= 275)
    {
      vs = "{M<-==-> {CATOMIZE {M<-==->{x";
      vp = "{M<-==-> {CATOMIZES {M<-==->{x";
    }
  else if (dam <= 315)
    {
      vs = "{C<{W-:-{C>{W ASPHYXIATE {C<{W-:-{C>{x";
      vp = "{C<{W-:-{C>{W ASPHYXIATES {C<{W-:-{C>{x";
    }
  else if (dam <= 390)
    {
      vs = "{W<-*-> {CRAVAGE {W<-*->{x";
      vp = "{W<-*-> {CRAVAGES {W<-*->{x";
    }
  else if (dam <= 435)
    {
      vs = "{M<>*<> {CFISSURE {M<>*<>{x";
      vp = "{M<>*<> {CFISSURES {M<>*<>{x";
    }
  else if (dam <= 500)
    {
      vs = "{Y<*>{R<*> {bLIQUIDATE {R<*>{Y<*>{x";
      vp = "{Y<*>{R<*> {bLIQUIDATES {R<*>{Y<*>{x";
    }
  else if (dam <= 590)
    {
      vs = "{b<*>{Y<*>{R<*>{G EVAPORATE {R<*>{Y<*>{b<*>{x";
      vp = "{b<*>{Y<*>{R<*>{G EVAPORATES {R<*>{Y<*>{b<*>{x";
    }
  else if (dam <= 650)
    {
      vs = "{Y<-=-> {RSUNDER {Y<-=->{x";
      vp = "{Y<-=-> {RSUNDERS {Y<-=->{x";
    }
  else if (dam <= 790)
    {
      vs = "{W<=-=><=-=> {GTEAR INTO {W<=-=><=-=>{x";
      vp = "{W<=-=><=-=> {GTEARS INTO {W<=-=><=-=>{x";
    }
  else if (dam <= 880)
    {
      vs = "{Y<->*<=> {bWASTE {Y<=>*<->{x";
      vp = "{Y<->*<=> {bWASTES {Y<=>*<->{x";
    }
  else if (dam <= 960)
    {
      vs = "{R<-+-><-*-> {WCREMATE {R<-*-><-+->{x";
      vp = "{R<-+-><-*-> {WCREMATES {R<-*-><-+->{x";
    }
  else if (dam <= 1040)
    {
      vs = "{M<*><*>{R<*><*> ANNIHILATE <*><*>{M<*><*>{x";
      vp = "{M<*><*>{R<*><*> ANNIHILATES <*><*>{M<*><*>{x";
    }
  else if (dam <= 3000)
    {
      vs = "{rinflict {f{RUNSPEAKABLE PAIN{r on{x";
      vp = "{rinflicts {f{RUNSPEAKABLE PAIN{r on{x";
    }
  else if (dam <= 6000)
    {
      vs = "{rinflict {f{RUNTHINKABLE PAIN{r on{x";
      vp = "{rinflicts {f{RUNTHINKABLE PAIN{r on{x";
    }
  else if (dam <= 9000)
    {
      vs = "{rinflict {f{RUNIMAGINABLE PAIN{r on{x";
      vp = "{rinflicts {f{RUNIMAGINABLE PAIN{r on{x";
    }
  else if (dam <= 12000)
    {
      vs = "{rinflict {f{RUNBELIEVABLE PAIN{r on{x";
      vp = "{rinflicts {f{RUNBELIEVABLE PAIN{r on{x";
    }
  else
    {
      vs =
	"does {mTOTALLY{x, {mUTTERLY{x, and in all other ways {m{fINCONCEIVABLE{w{x things to";
      vp =
	"does {mTOTALLY{x, {mUTTERLY{x, and in all other ways {m{fINCONCEIVABLE{w{x things to";
    }

  punct =
    (dam < 0) ? "?" : (dam <= 250) ? "." : (dam <=
					    1000) ? "!" : (dam <=
							   3000) ? "!!"
    : (dam <= 5000) ? "!!!" : "!!!!";

#define SEE_DAMAGE(ch) (!IsNPC(ch) && IsSet(ch->act, PLR_AUTODAMAGE))

  if (dt == TYPE_HIT)
    {
      if (ch == victim)
	{
	  sprintf (buf1, CTAG (_OHIT) "$n %s" CTAG (_OHIT) " $melf%s$t", vp,
		   punct);
	  sprintf (buf2, CTAG (_YHIT) "You %s" CTAG (_YHIT) " yourself%s%s",
		   vs, punct, SEE_DAMAGE (ch) ? chmesg : "{x");
	}
      else
	{
	  sprintf (buf1, CTAG (_OHIT) "$n %s" CTAG (_OHIT) " $N%s$t", vp,
		   punct);
	  sprintf (buf2, CTAG (_YHIT) "You %s" CTAG (_YHIT) " $N%s%s", vs,
		   punct, SEE_DAMAGE (ch) ? chmesg : "{x");
	  sprintf (buf3, CTAG (_VHIT) "$n %s" CTAG (_VHIT) " you%s%s", vp,
		   punct, SEE_DAMAGE (victim) ? vmesg : "{x");
	}
    }
  else
    {
      if (dt >= 0 && dt < top_skill)
	attack = skill_table[dt].noun_damage;
      else if (dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
	attack = attack_table[dt - TYPE_HIT].noun;
      else
	{
	  bugf ("Dam_message: bad dt %d.", dt);
	  dt = TYPE_HIT;
	  attack = attack_table[0].name;
	}

      if (immune)
	{
	  if (ch == victim)
	    {
	      sprintf (buf1,
		       CTAG (_OHIT) "$n is unaffected by $s own %s.{x",
		       attack);
	      sprintf (buf2,
		       CTAG (_YHIT) "Luckily, you are immune to that.{x");
	    }
	  else
	    {
	      sprintf (buf1, CTAG (_OHIT) "$N is unaffected by $n's %s!{x",
		       attack);
	      sprintf (buf2, CTAG (_YHIT) "$N is unaffected by your %s!{x",
		       attack);
	      sprintf (buf3,
		       CTAG (_VHIT) "$n's %s is powerless against you.{x",
		       attack);
	    }
	}
      else
	{
	  if (ch == victim)
	    {
	      sprintf (buf1,
		       CTAG (_OHIT) "$n's %s %s" CTAG (_OHIT) " $m%s$t",
		       attack, vp, punct);
	      sprintf (buf2,
		       CTAG (_YHIT) "Your %s %s" CTAG (_YHIT) " you%s%s",
		       attack, vp, punct, SEE_DAMAGE (ch) ? chmesg : "{x");
	    }
	  else
	    {
	      sprintf (buf1,
		       CTAG (_OHIT) "$n's %s %s" CTAG (_OHIT) " $N%s$t",
		       attack, vp, punct);
	      sprintf (buf2,
		       CTAG (_YHIT) "Your %s %s" CTAG (_YHIT) " $N%s%s",
		       attack, vp, punct, SEE_DAMAGE (ch) ? chmesg : "{x");
	      sprintf (buf3,
		       CTAG (_VHIT) "$n's %s %s" CTAG (_VHIT) " you%s%s",
		       attack, vp, punct, SEE_DAMAGE (victim) ? vmesg : "{x");
	    }
	}
    }

  if (ch == victim)
    {
      act (buf1, ch, omesg, NULL, TO_ROOM | TO_DAMAGE);
      act (buf2, ch, NULL, NULL, TO_CHAR);
    }
  else
    {
      act (buf1, ch, omesg, victim, TO_NOTVICT | TO_DAMAGE);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }

  return;
}


void
disarm (CharData * ch, CharData * victim)
{
  ObjData *obj;

  if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    return;

  if (IsObjStat (obj, ITEM_NOREMOVE))
    {
      act ("$S weapon won't budge!", ch, NULL, victim, TO_CHAR);
      act ("$n tries to disarm you, but your weapon won't budge!", ch,
	   NULL, victim, TO_VICT);
      act ("$n tries to disarm $N, but fails.", ch, NULL, victim, TO_NOTVICT);
      return;
    }

  act ("$n DISARMS you and sends your weapon flying!", ch, NULL, victim,
       TO_VICT);
  act ("You disarm $N!", ch, NULL, victim, TO_CHAR);
  act ("$n disarms $N!", ch, NULL, victim, TO_NOTVICT);

  obj_from_char (obj);
  if (IsObjStat (obj, ITEM_NODROP) || IsObjStat (obj, ITEM_INVENTORY) ||
      IsSet (victim->in_room->room_flags, ROOM_ARENA))
    obj_to_char (obj, victim);
  else
    {
      obj_to_room (obj, victim->in_room);
      if (IsNPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
	get_obj (victim, obj, NULL);
    }

  return;
}

Do_Fun (do_berserk)
{
  int chance, hp_percent;

  if ((chance = get_skill (ch, gsn_berserk)) == 0 ||
      (IsNPC (ch) && !IsSet (ch->off_flags, OFF_BERSERK)) ||
      (!IsNPC (ch) && !can_use_skpell (ch, gsn_berserk)))
    {
      chprintln (ch, "You turn red in the face, but nothing happens.");
      return;
    }

  if (IsAffected (ch, AFF_BERSERK) || IsAffected (ch, gsn_berserk) ||
      IsAffected (ch, skill_lookup ("frenzy")))
    {
      chprintln (ch, "You get a little madder.");
      return;
    }

  if (IsAffected (ch, AFF_CALM))
    {
      chprintln (ch, "You're feeling to mellow to berserk.");
      return;
    }

  if (ch->mana < 50)
    {
      chprintln (ch, "You can't get up enough energy.");
      return;
    }




  if (ch->position == POS_FIGHTING)
    chance += 10;


  hp_percent = 100 * ch->hit / ch->max_hit;
  chance += 25 - hp_percent / 2;

  if (number_percent () < chance)
    {
      AffectData af;

      WaitState (ch, PULSE_VIOLENCE);
      ch->mana -= 50;
      ch->move /= 2;


      ch->hit += ch->level * 2;
      ch->hit = Min (ch->hit, ch->max_hit);

      chprintln (ch, "Your pulse races as you are consumed by rage!");
      act ("$n gets a wild look in $s eyes.", ch, NULL, NULL, TO_ROOM);
      check_improve (ch, gsn_berserk, true, 2);

      af.where = TO_AFFECTS;
      af.type = gsn_berserk;
      af.level = ch->level;
      af.duration = number_fuzzy (ch->level / 8);
      af.modifier = Max (1, ch->level / 5);
      af.bitvector = AFF_BERSERK;

      af.location = APPLY_HITROLL;
      affect_to_char (ch, &af);

      af.location = APPLY_DAMROLL;
      affect_to_char (ch, &af);

      af.modifier = Max (10, 10 * (ch->level / 5));
      af.location = APPLY_AC;
      affect_to_char (ch, &af);
      if (skill_table[gsn_berserk].sound)
	act_sound (skill_table[gsn_berserk].sound, ch, NULL,
		   skill_table[gsn_berserk].sound->to, POS_RESTING);
    }
  else
    {
      WaitState (ch, 3 * PULSE_VIOLENCE);
      ch->mana -= 25;
      ch->move /= 2;

      chprintln (ch, "Your pulse speeds up, but nothing happens.");
      check_improve (ch, gsn_berserk, false, 2);
    }
}

Do_Fun (do_bash)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_bash)) == 0 ||
      (IsNPC (ch) && !IsSet (ch->off_flags, OFF_BASH)) ||
      (!IsNPC (ch) && !can_use_skpell (ch, gsn_bash)))
    {
      chprintln (ch, "Bashing? What's that?");
      return;
    }

  if (NullStr (arg))
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  chprintln (ch, "But you aren't fighting anyone!");
	  return;
	}
    }
  else if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim->position < POS_FIGHTING)
    {
      act ("You'll have to let $M get back up first.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "You try to bash your brains out, but fail.");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsNPC (victim) && victim->fighting != NULL &&
      !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }




  chance += ch->carry_weight / 250;
  chance -= victim->carry_weight / 200;

  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 15;
  else
    chance += (ch->size - victim->size) * 10;


  chance += get_curr_stat (ch, STAT_STR);
  chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
  chance -= GetArmor (victim, AC_BASH) / 25;

  if (IsSet (ch->off_flags, OFF_FAST) || IsAffected (ch, AFF_HASTE))
    chance += 10;
  if (IsSet (victim->off_flags, OFF_FAST) || IsAffected (victim, AFF_HASTE))
    chance -= 30;


  chance += (ch->level - victim->level);

  if (!IsNPC (victim) && chance < get_skill (victim, gsn_dodge))
    {
      chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
    }


  if (number_percent () < chance)
    {

      act ("$n sends you sprawling with a powerful bash!", ch, NULL,
	   victim, TO_VICT);
      act ("You slam into $N, and send $M flying!", ch, NULL, victim,
	   TO_CHAR);
      act ("$n sends $N sprawling with a powerful bash.", ch, NULL,
	   victim, TO_NOTVICT);
      check_improve (ch, gsn_bash, true, 1);

      DazeState (victim, 3 * PULSE_VIOLENCE);
      WaitState (ch, skill_table[gsn_bash].beats);
      victim->position = POS_RESTING;
      damage (ch, victim,
	      number_range (2, 2 + 2 * ch->size + chance / 20),
	      gsn_bash, DAM_BASH, false);
      if (skill_table[gsn_bash].sound)
	act_sound (skill_table[gsn_bash].sound, ch, victim,
		   skill_table[gsn_bash].sound->to, POS_RESTING);

    }
  else
    {
      damage (ch, victim, 0, gsn_bash, DAM_BASH, false);
      act ("You fall flat on your face!", ch, NULL, victim, TO_CHAR);
      act ("$n falls flat on $s face.", ch, NULL, victim, TO_NOTVICT);
      act ("You evade $n's bash, causing $m to fall flat on $s face.",
	   ch, NULL, victim, TO_VICT);
      check_improve (ch, gsn_bash, false, 1);
      ch->position = POS_RESTING;
      WaitState (ch, skill_table[gsn_bash].beats * 3 / 2);
    }
  check_killer (ch, victim);
}

Do_Fun (do_dirt)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_dirt)) == 0 ||
      (IsNPC (ch) && !IsSet (ch->off_flags, OFF_KICK_DIRT)) ||
      (!IsNPC (ch) && !can_use_skpell (ch, gsn_dirt)))
    {
      chprintln (ch, "You get your feet dirty.");
      return;
    }

  if (NullStr (arg))
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  chprintln (ch, "But you aren't in combat!");
	  return;
	}
    }
  else if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsAffected (victim, AFF_BLIND))
    {
      act ("$E's already been blinded.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "Very funny.");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsNPC (victim) && victim->fighting != NULL &&
      !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
      return;
    }




  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);


  if (IsSet (ch->off_flags, OFF_FAST) || IsAffected (ch, AFF_HASTE))
    chance += 10;
  if (IsSet (victim->off_flags, OFF_FAST) || IsAffected (victim, AFF_HASTE))
    chance -= 25;


  chance += (ch->level - victim->level) * 2;


  if (chance % 5 == 0)
    chance += 1;



  switch (ch->in_room->sector_type)
    {
    case (SECT_INSIDE):
      chance -= 20;
      break;
    case (SECT_CITY):
      chance -= 10;
      break;
    case (SECT_FIELD):
      chance += 5;
      break;
    case (SECT_MOUNTAIN):
      chance -= 10;
      break;
    case (SECT_WATER_SWIM):
      chance = 0;
      break;
    case (SECT_WATER_NOSWIM):
      chance = 0;
      break;
    case (SECT_AIR):
      chance = 0;
      break;
    case (SECT_DESERT):
      chance += 10;
      break;
    case (SECT_PATH):
      chance += 5;
      break;
    case (SECT_SWAMP):
      chance -= 10;
      break;
    default:
      break;
    }

  if (chance == 0)
    {
      chprintln (ch, "There isn't any dirt to kick.");
      return;
    }


  if (number_percent () < chance)
    {
      AffectData af;

      act ("$n is blinded by the dirt in $s eyes!", victim, NULL, NULL,
	   TO_ROOM);
      act ("$n kicks dirt in your eyes!", ch, NULL, victim, TO_VICT);
      damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, false);
      chprintln (victim, "You can't see a thing!");
      check_improve (ch, gsn_dirt, true, 2);
      WaitState (ch, skill_table[gsn_dirt].beats);

      af.where = TO_AFFECTS;
      af.type = gsn_dirt;
      af.level = ch->level;
      af.duration = 0;
      af.location = APPLY_HITROLL;
      af.modifier = -4;
      af.bitvector = AFF_BLIND;

      affect_to_char (victim, &af);
      if (skill_table[gsn_dirt].sound)
	act_sound (skill_table[gsn_dirt].sound, ch, victim,
		   skill_table[gsn_dirt].sound->to, POS_RESTING);
    }
  else
    {
      damage (ch, victim, 0, gsn_dirt, DAM_NONE, true);
      check_improve (ch, gsn_dirt, false, 2);
      WaitState (ch, skill_table[gsn_dirt].beats);
    }
  check_killer (ch, victim);
}

Do_Fun (do_trip)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_trip)) == 0 ||
      (IsNPC (ch) && !IsSet (ch->off_flags, OFF_TRIP)) ||
      (!IsNPC (ch) && !can_use_skpell (ch, gsn_trip)))
    {
      chprintln (ch, "Tripping?  What's that?");
      return;
    }

  if (NullStr (arg))
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  chprintln (ch, "But you aren't fighting anyone!");
	  return;
	}
    }
  else if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsNPC (victim) && victim->fighting != NULL &&
      !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if (IsAffected (victim, AFF_FLYING))
    {
      act ("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->position < POS_FIGHTING)
    {
      act ("$N is already down.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "You fall flat on your face!");
      WaitState (ch, 2 * skill_table[gsn_trip].beats);
      act ("$n trips over $s own feet!", ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }




  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 10;


  chance += get_curr_stat (ch, STAT_DEX);
  chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;


  if (IsSet (ch->off_flags, OFF_FAST) || IsAffected (ch, AFF_HASTE))
    chance += 10;
  if (IsSet (victim->off_flags, OFF_FAST) || IsAffected (victim, AFF_HASTE))
    chance -= 20;


  chance += (ch->level - victim->level) * 2;


  if (number_percent () < chance)
    {
      act ("$n trips you and you go down!", ch, NULL, victim, TO_VICT);
      act ("You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR);
      act ("$n trips $N, sending $M to the ground.", ch, NULL, victim,
	   TO_NOTVICT);
      check_improve (ch, gsn_trip, true, 1);

      DazeState (victim, 2 * PULSE_VIOLENCE);
      WaitState (ch, skill_table[gsn_trip].beats);
      victim->position = POS_RESTING;
      damage (ch, victim, number_range (2, 2 + 2 * victim->size),
	      gsn_trip, DAM_BASH, true);
      if (number_percent () < chance - 5
	  && ValidStance (GetStance (victim, STANCE_CURRENT)))
	{
	  SetStance (victim, STANCE_CURRENT, STANCE_NONE);
	  act ("You trip up $N's stance!", ch, NULL, victim, TO_CHAR);
	  act ("$n trips up $N's stance!", ch, NULL, victim, TO_NOTVICT);
	  act ("$n trips up your stance!", ch, NULL, victim, TO_VICT);
	}
      if (skill_table[gsn_trip].sound)
	act_sound (skill_table[gsn_trip].sound, ch, victim,
		   skill_table[gsn_trip].sound->to, POS_RESTING);
    }
  else
    {
      damage (ch, victim, 0, gsn_trip, DAM_BASH, true);
      WaitState (ch, skill_table[gsn_trip].beats * 2 / 3);
      check_improve (ch, gsn_trip, false, 1);
    }
  check_killer (ch, victim);
}

Do_Fun (do_kill)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Kill whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "You hit yourself.  Ouch!");
      multi_hit (ch, ch, TYPE_UNDEFINED);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      chprintln (ch, "You do the best you can!");
      return;
    }

  WaitState (ch, 1 * PULSE_VIOLENCE);
  check_killer (ch, victim);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}

Do_Fun (do_murder)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Murder whom?");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) || (IsNPC (ch) && IsSet (ch->act, ACT_PET)))
    return;

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "Suicide is a mortal sin.");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsNPC (victim) && victim->fighting != NULL &&
      !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      chprintln (ch, "You do the best you can!");
      return;
    }

  WaitState (ch, 1 * PULSE_VIOLENCE);
  if (IsNPC (ch))
    sprintf (buf, "Help! I am being attacked by %s!", ch->short_descr);
  else
    sprintf (buf, "Help!  I am being attacked by %s!", ch->name);
  do_function (victim, &do_yell, buf);
  check_killer (ch, victim);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}

Do_Fun (do_backstab)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Backstab whom?");
      return;
    }

  if (ch->fighting != NULL)
    {
      chprintln (ch, "You're facing the wrong end.");
      return;
    }
  else if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "How can you sneak up on yourself?");
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IsNPC (victim) && victim->fighting != NULL &&
      !is_same_group (ch, victim->fighting))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      chprintln (ch, "You need to wield a weapon to backstab.");
      return;
    }

  if (victim->hit < victim->max_hit / 3)
    {
      act ("$N is hurt and suspicious ... you can't sneak up.", ch,
	   NULL, victim, TO_CHAR);
      return;
    }

  check_killer (ch, victim);
  WaitState (ch, skill_table[gsn_backstab].beats);
  if (number_percent () < get_skill (ch, gsn_backstab) ||
      (get_skill (ch, gsn_backstab) >= 2 && !IsAwake (victim)))
    {
      check_improve (ch, gsn_backstab, true, 1);
      if (skill_table[gsn_backstab].sound)
	act_sound (skill_table[gsn_backstab].sound, ch, victim,
		   skill_table[gsn_backstab].sound->to, POS_RESTING);
      multi_hit (ch, victim, gsn_backstab);
    }
  else
    {
      check_improve (ch, gsn_backstab, false, 1);
      damage (ch, victim, 0, gsn_backstab, DAM_NONE, true);
    }

  return;
}

Do_Fun (do_flee)
{
  RoomIndex *was_in;
  RoomIndex *now_in;
  CharData *victim;
  int attempt;

  if ((victim = ch->fighting) == NULL)
    {
      if (ch->position == POS_FIGHTING)
	ch->position = POS_STANDING;
      chprintln (ch, "You aren't fighting anyone.");
      return;
    }

  if (IsSet (ch->in_room->room_flags, ROOM_ARENA))
    return;

  was_in = ch->in_room;
  for (attempt = 0; attempt < 6; attempt++)
    {
      ExitData *pexit;
      int door;

      door = number_door ();
      if ((pexit = was_in->exit[door]) == 0 || pexit->u1.to_room == NULL
	  || IsSet (pexit->exit_info, EX_CLOSED) ||
	  number_range (0, ch->daze) != 0 || (IsNPC (ch) &&
					      IsSet (pexit->u1.
						     to_room->room_flags,
						     ROOM_NO_MOB)))
	continue;

      move_char (ch, door, false);
      if ((now_in = ch->in_room) == was_in)
	continue;

      ch->in_room = was_in;
      act ("$n has fled!", ch, NULL, NULL, TO_ROOM);
      ch->in_room = now_in;

      if (!IsNPC (ch))
	{
	  chprintln (ch, "You flee from combat!");
	  if ((is_class (ch, 2)) && (number_percent () < 3 * (ch->level / 2)))
	    chprintln (ch, "You snuck away safely.");
	  else
	    {
	      chprintln (ch, "You lost 10 exp.");
	      gain_exp (ch, -10);
	    }
	}

      stop_fighting (ch, true);
      return;
    }

  chprintln (ch, "PANIC! You couldn't escape!");
  return;
}

Do_Fun (do_rescue)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  CharData *fch;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Rescue whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "What about fleeing instead?");
      return;
    }

  if (!IsNPC (ch) && IsNPC (victim))
    {
      chprintln (ch, "Doesn't need your help!");
      return;
    }

  if (ch->fighting == victim)
    {
      chprintln (ch, "Too late.");
      return;
    }

  if ((fch = victim->fighting) == NULL)
    {
      chprintln (ch, "That person is not fighting right now.");
      return;
    }

  if (IsNPC (fch) && !is_same_group (ch, victim))
    {
      chprintln (ch, "Kill stealing is not permitted.");
      return;
    }

  WaitState (ch, skill_table[gsn_rescue].beats);
  if (number_percent () > get_skill (ch, gsn_rescue))
    {
      chprintln (ch, "You fail the rescue.");
      check_improve (ch, gsn_rescue, false, 1);
      return;
    }

  act ("You rescue $N!", ch, NULL, victim, TO_CHAR);
  act ("$n rescues you!", ch, NULL, victim, TO_VICT);
  act ("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);
  check_improve (ch, gsn_rescue, true, 1);

  stop_fighting (fch, false);
  stop_fighting (victim, false);

  check_killer (ch, fch);
  set_fighting (ch, fch);
  set_fighting (fch, ch);
  return;
}

Do_Fun (do_kick)
{
  CharData *victim;

  if (!IsNPC (ch) && !can_use_skpell (ch, gsn_kick))
    {
      chprintln (ch, "You better leave the martial arts to fighters.");
      return;
    }

  if (IsNPC (ch) && !IsSet (ch->off_flags, OFF_KICK))
    return;

  if ((victim = ch->fighting) == NULL)
    {
      chprintln (ch, "You aren't fighting anyone.");
      return;
    }

  WaitState (ch, skill_table[gsn_kick].beats);
  if (get_skill (ch, gsn_kick) > number_percent ())
    {
      if (skill_table[gsn_kick].sound)
	act_sound (skill_table[gsn_kick].sound, ch, victim,
		   skill_table[gsn_kick].sound->to, POS_RESTING);
      damage (ch, victim, number_range (1, ch->level), gsn_kick, DAM_BASH,
	      true);
      check_improve (ch, gsn_kick, true, 1);
    }
  else
    {
      damage (ch, victim, 0, gsn_kick, DAM_BASH, true);
      check_improve (ch, gsn_kick, false, 1);
    }
  check_killer (ch, victim);
  return;
}

Do_Fun (do_disarm)
{
  CharData *victim;
  ObjData *obj;
  int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

  hth = 0;

  if ((chance = get_skill (ch, gsn_disarm)) == 0)
    {
      chprintln (ch, "You don't know how to disarm opponents.");
      return;
    }

  if (get_eq_char (ch, WEAR_WIELD) == NULL &&
      ((hth = get_skill (ch, gsn_hand_to_hand)) == 0 ||
       (IsNPC (ch) && !IsSet (ch->off_flags, OFF_DISARM))))
    {
      chprintln (ch, "You must wield a weapon to disarm.");
      return;
    }

  if ((victim = ch->fighting) == NULL)
    {
      chprintln (ch, "You aren't fighting anyone.");
      return;
    }

  if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    {
      chprintln (ch, "Your opponent is not wielding a weapon.");
      return;
    }


  ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch));
  vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim));
  ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim));




  if (get_eq_char (ch, WEAR_WIELD) == NULL)
    chance = chance * hth / 150;
  else
    chance = chance * ch_weapon / 100;

  chance += (ch_vict_weapon / 2 - vict_weapon) / 2;


  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_STR);


  chance += (ch->level - victim->level) * 2;


  if (number_percent () < chance)
    {
      if (skill_table[gsn_disarm].sound)
	act_sound (skill_table[gsn_disarm].sound, ch, victim,
		   skill_table[gsn_disarm].sound->to, POS_RESTING);
      WaitState (ch, skill_table[gsn_disarm].beats);
      disarm (ch, victim);
      check_improve (ch, gsn_disarm, true, 1);
    }
  else
    {
      WaitState (ch, skill_table[gsn_disarm].beats);
      act ("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
      act ("$n tries to disarm you, but fails.", ch, NULL, victim, TO_VICT);
      act ("$n tries to disarm $N, but fails.", ch, NULL, victim, TO_NOTVICT);
      check_improve (ch, gsn_disarm, false, 1);
    }
  check_killer (ch, victim);
  return;
}

Do_Fun (do_surrender)
{
  CharData *mob;

  if ((mob = ch->fighting) == NULL)
    {
      chprintln (ch, "But you're not fighting!");
      return;
    }
  act ("You surrender to $N!", ch, NULL, mob, TO_CHAR);
  act ("$n surrenders to you!", ch, NULL, mob, TO_VICT);
  act ("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
  stop_fighting (ch, true);

  if (!IsNPC (ch) && IsNPC (mob) &&
      (!HasTriggerMob (mob, TRIG_SURR) ||
       !p_percent_trigger (mob, NULL, NULL, ch, NULL, NULL, TRIG_SURR)))
    {
      act ("$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR);
      multi_hit (mob, ch, TYPE_UNDEFINED);
    }
}

Do_Fun (do_slay)
{
  CharData *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Slay whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (ch == victim)
    {
      chprintln (ch, "Suicide is a mortal sin.");
      return;
    }

  if (!IsNPC (victim) && victim->level >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  act ("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
  act ("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
  act ("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
  raw_kill (victim, ch);
  return;
}

Do_Fun (do_sskill)
{
  int i;

  chprintln (ch, stringf
	     (ch, 0, Center, "{w-{W-", "{R[ Fighting Stances{W ]"));
  for (i = 0; i < MAX_STANCE; i++)
    {
      if (stance_table[i].prereq[0] == STANCE_NONE)
	continue;

      if (stance_table[i].prereq[0] == STANCE_NORMAL)
	{
	  chprintlnf (ch, "%-9s: {Y%d%%{x", stance_table[i].name,
		      GetStance (ch, stance_table[i].stance));
	  continue;
	}

      if (stance_table[i].prereq[0] == STANCE_CURRENT)
	{
	  chprintlnf (ch, "%-9s: {R%s{x", stance_table[i].name,
		      stance_name (GetStance (ch, stance_table[i].stance)));
	  continue;
	}

      if (GetStance (ch, stance_table[i].prereq[0]) >= 200
	  && GetStance (ch, stance_table[i].prereq[1]) >= 200)
	{
	  chprintlnf (ch, "%-9s: {Y%d%%{x", stance_table[i].name,
		      GetStance (ch, stance_table[i].stance));
	}
      else
	{
	  chprintlnf (ch, "%-9s: {yrequires master in %s and %s.{x",
		      stance_table[i].name,
		      stance_name (stance_table[i].prereq[0]),
		      stance_name (stance_table[i].prereq[1]));
	}
    }

  chprintln (ch, draw_line (ch, "{w-{W-", 0));
  return;
}

int
dambonus (CharData * ch, CharData * victim, int dam, int stance)
{
  if (dam < 1)
    return 0;

  if (!ValidStance (stance))
    return dam;

  if (!can_counter (victim))
    {
      if (InStance (ch, STANCE_MONKEY))
	{
	  int mindam = dam * 25 / 100;

	  dam *= (GetStance (ch, STANCE_MONKEY) + 1) / 200;
	  if (dam < mindam)
	    dam = mindam;
	}
      else if (InStance (ch, STANCE_BULL)
	       && GetStance (ch, STANCE_BULL) > 100)
	dam += dam * (GetStance (ch, STANCE_BULL) / 100);
      else if (InStance (ch, STANCE_DRAGON)
	       && GetStance (ch, STANCE_DRAGON) > 100)
	dam += dam * (GetStance (ch, STANCE_DRAGON) / 100);
      else if (InStance (ch, STANCE_TIGER)
	       && GetStance (ch, STANCE_TIGER) > 100)
	dam += dam * (GetStance (ch, STANCE_TIGER) / 100);
      else if (GetStance (ch, STANCE_CURRENT) > 0
	       && GetStance (ch, stance) < 100)
	dam = dam * 5 / 10;
    }
  if (!can_counter (ch))
    {
      if (InStance (victim, STANCE_CRAB)
	  && GetStance (victim, STANCE_CRAB) > 100)
	dam /= GetStance (victim, STANCE_CRAB) / 100;
      else if (InStance (victim, STANCE_DRAGON)
	       && GetStance (victim, STANCE_DRAGON) > 100)
	dam /= GetStance (victim, STANCE_DRAGON) / 100;
      else if (InStance (victim, STANCE_SWALLOW)
	       && GetStance (victim, STANCE_SWALLOW) > 100)
	dam /= GetStance (victim, STANCE_SWALLOW) / 100;
    }
  return dam;
}

bool
can_counter (CharData * ch)
{
  if (InStance (ch, STANCE_MONKEY))
    return true;

  return false;
}

bool
can_bypass (CharData * ch, CharData * victim)
{
  if (InStance (ch, STANCE_VIPER))
    return true;

  if (InStance (ch, STANCE_MANTIS))
    return true;

  if (InStance (ch, STANCE_TIGER))
    return true;

  return false;
}

const char *
stance_name (int stance)
{
  int i;

  for (i = 0; i < MAX_STANCE; i++)
    {
      if (stance_table[i].stance == stance)
	return stance_table[i].name;
    }

  return "unknown";
}

bool
can_use_stance (CharData * ch, int stance)
{
  int pos;

  if (!ValidStance (stance))
    return false;

  for (pos = 0; pos < MAX_STANCE; pos++)
    {
      if (stance_table[pos].stance == stance)
	break;
    }

  if (pos == MAX_STANCE)
    return false;

  if (stance_table[pos].prereq[0] <= STANCE_NORMAL)
    return true;

  if (GetStance (ch, stance_table[pos].prereq[0]) >= 200 &&
      GetStance (ch, stance_table[pos].prereq[1]) >= 200)
    return true;

  return false;
}

void
improve_stance (CharData * ch)
{
  char bufskill[25];
  int dice1;
  int dice2;
  int stance;
  int skill;

  dice1 = number_percent ();
  dice2 = number_percent ();

  stance = GetStance (ch, STANCE_CURRENT);
  if (!ValidStance (stance))
    return;
  skill = GetStance (ch, stance);
  if (skill >= 200)
    {
      SetStance (ch, stance, 200);
      return;
    }
  if ((dice1 > skill && dice2 > skill) || (dice1 == 100 || dice2 == 100))
    ch->stance[stance] += 1;
  else
    return;
  if (skill == GetStance (ch, stance))
    return;

  if (IsNPC (ch))
    return;

  switch (GetStance (ch, stance))
    {

    case 1:
      sprintf (bufskill, "an apprentice of");
      break;
    case 26:
      sprintf (bufskill, "a trainee of");
      break;
    case 51:
      sprintf (bufskill, "a student of");
      break;
    case 76:
      sprintf (bufskill, "fairly experienced in");
      break;
    case 101:
      sprintf (bufskill, "well trained in");
      break;
    case 126:
      sprintf (bufskill, "highly skilled in");
      break;
    case 151:
      sprintf (bufskill, "an expert of");
      break;
    case 176:
      sprintf (bufskill, "a master of");
      break;
    case 200:
      sprintf (bufskill, "a grand master of");
      break;
    default:
      return;
    }

  chprintlnf (ch, "{RYou are now %s the %s stance.{x", bufskill,
	      stance_name (stance));
  return;
}

void
show_available_stances (CharData * ch, const char *n_fun)
{
  int i;
  bool found = false;

  cmd_syntax (ch, NULL, n_fun, "<stance>", NULL);
  chprint (ch, "Valid stances are: ");
  for (i = 0; i < MAX_STANCE; i++)
    {
      if (!ValidStance (stance_table[i].stance))
	continue;

      if (stance_table[i].prereq[0] <= STANCE_NORMAL)
	{
	  found = true;
	  chprintf (ch, " %s", stance_table[i].name);
	}
      else if (GetStance (ch, stance_table[i].prereq[0]) >= 200 &&
	       GetStance (ch, stance_table[i].prereq[1]) >= 200)
	{
	  found = true;
	  chprintf (ch, " %s", stance_table[i].name);
	}
    }
  if (!found)
    chprintln (ch, "none!");
  else
    chprintln (ch, ".");
  return;
}

Do_Fun (do_stance)
{
  char arg[MIL];
  int i;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      if (!ValidStance (GetStance (ch, STANCE_CURRENT)))
	{
	  SetStance (ch, STANCE_CURRENT, STANCE_NORMAL);
	  chprintln (ch, "You drop into a general fighting stance.");
	  act ("$n drops into a general fighting stance.", ch, NULL, NULL,
	       TO_ROOM);
	}
      else
	{
	  SetStance (ch, STANCE_CURRENT, STANCE_NONE);
	  chprintln (ch, "You relax from your fighting stance.");
	  act ("$n relaxes from $s fighting stance.", ch, NULL, NULL,
	       TO_ROOM);
	}
      return;
    }

  if (ValidStance (GetStance (ch, STANCE_CURRENT)))
    {
      chprintln
	(ch,
	 "You cannot change stances until you come up from the one you are currently in.");
      return;
    }

  i = stance_lookup (arg);

  if (i == -1 || !ValidStance (stance_table[i].stance))
    {
      show_available_stances (ch, n_fun);
      return;
    }

  if (!can_use_stance (ch, stance_table[i].stance))
    {
      chprintlnf (ch, "You need to master %s and %s stances to use %s.",
		  stance_name (stance_table[i].prereq[0]),
		  stance_name (stance_table[i].prereq[1]),
		  stance_table[i].name);
      return;
    }

  SetStance (ch, STANCE_CURRENT, stance_table[i].stance);

  chprintln (ch, stance_table[i].chdrop);
  act (stance_table[i].odrop, ch, NULL, NULL, TO_ROOM);

  if (IsNPC (ch))
    SetStance (ch, stance_table[i].stance, Min (ch->level * 4 / 2, 200));
  return;
}

Do_Fun (do_autostance)
{
  char arg[MIL];
  int i;

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (!str_cmp (arg, "none"))
    {
      chprintln (ch, "You no longer autostance.");
      SetStance (ch, STANCE_AUTODROP, STANCE_NONE);
      return;
    }

  i = stance_lookup (arg);

  if (i == -1 || !ValidStance (stance_table[i].stance))
    {
      show_available_stances (ch, n_fun);

      return;
    }

  if (!can_use_stance (ch, stance_table[i].stance))
    {
      chprintlnf (ch, "You need to master %s and %s stances to use %s.",
		  stance_name (stance_table[i].prereq[0]),
		  stance_name (stance_table[i].prereq[1]),
		  stance_table[i].name);
      return;
    }

  SetStance (ch, STANCE_AUTODROP, stance_table[i].stance);

  chprintlnf (ch, "You now autostance to %s.", stance_table[i].name);

  if (IsNPC (ch))
    SetStance (ch, stance_table[i].stance, Min (ch->level * 4 / 2, 200));

}

void
autodrop (CharData * ch)
{
  int stance;

  stance = GetStance (ch, STANCE_AUTODROP);

  if (!ValidStance (stance))
    return;

  if (!ValidStance (GetStance (ch, STANCE_CURRENT)))
    {
      SetStance (ch, STANCE_CURRENT, stance);
      chprintlnf (ch, "You autodrop into the %s stance. (%d%%)",
		  stance_name (stance), GetStance (ch, stance));
      act ("$n autodrops into the $T stance.", ch, NULL,
	   stance_name (stance), TO_ROOM);
    }
}




bool
check_force_shield (CharData * ch, CharData * victim)
{
  int chance;

  if (!IsAffected (victim, AFF_FORCE_SHIELD))
    return false;

  chance = 100 / 15;

  if (victim->level >= ch->level)
    chance += 2;

  if (number_percent () >= chance)
    return false;

  act ("Your force-shield blocks $n's attack!", ch, NULL, victim, TO_VICT);
  act ("$N's force-shield blocks your attack.", ch, NULL, victim, TO_CHAR);

  return true;
}



bool
check_static_shield (CharData * ch, CharData * victim)
{
  int chance, sn;
  AffectData *shock;

  if (!IsAffected (victim, AFF_STATIC_SHIELD))
    return false;

  chance = 10;

  if (victim->level >= ch->level)
    chance += 2;

  if (number_percent () >= chance)
    return false;


  sn = skill_lookup ("static shield");
  shock = affect_find (victim->affect_first, sn);

  if (shock != NULL)
    {
      damage (victim, ch, number_fuzzy (shock->level / 5), sn, DAM_ENERGY,
	      true);
    }

  if (get_eq_char (ch, WEAR_WIELD) == NULL)
    return true;

  act ("Your static shield catches $n!", victim, NULL, ch, TO_VICT);
  act ("$N's static shield catches you!", victim, NULL, ch, TO_CHAR);

  spell_heat_metal (skill_lookup ("heat metal"),
		    victim->level / 2, victim, (void *) ch, TARGET_CHAR);

  return true;
}



bool
check_flame_shield (CharData * ch, CharData * victim)
{
  int chance, sn;
  AffectData *burn;

  if (!IsAffected (victim, AFF_FLAME_SHIELD))
    return false;

  if (get_eq_char (victim, WEAR_WIELD) != NULL)
    return false;

  chance = 100 / 3;

  if (victim->level >= ch->level)
    chance += 2;

  if (number_percent () >= chance)
    return false;

  sn = skill_lookup ("flame shield");
  burn = affect_find (victim->affect_first, sn);

  if (burn != NULL)
    {
      fire_effect (ch, burn->level, number_fuzzy (10), TARGET_CHAR);
      damage (victim, ch, number_fuzzy (burn->level), sn, DAM_FIRE, true);
    }

  return true;
}
