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

EXTERN const char *target_name;

Spell_Fun (spell_farsight)
{
  if (IsAffected (ch, AFF_BLIND))
    {
      chprintln (ch, "Maybe it would help if you could see?");
      return false;
    }

  do_function (ch, &do_scan, target_name);
  return true;
}

Spell_Fun (spell_portal)
{
  CharData *victim;
  ObjData *portal, *stone;

  if ((victim = get_char_world (ch, target_name)) == NULL || victim == ch
      || victim->in_room == NULL || !can_see_room (ch, victim->in_room)
      || IsSet (victim->in_room->room_flags, ROOM_SAFE)
      || IsSet (victim->in_room->room_flags, ROOM_PRIVATE)
      || IsSet (victim->in_room->room_flags, ROOM_SOLITARY)
      || IsSet (victim->in_room->area->area_flags, AREA_CLOSED)
      || IsSet (victim->in_room->room_flags, ROOM_ARENA)
      || IsSet (ch->in_room->room_flags, ROOM_ARENA)
      || IsSet (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IsSet (ch->in_room->room_flags, ROOM_NO_RECALL) || (IsNPC (victim)
							     &&
							     is_gqmob (NULL,
								       victim->
								       pIndexData->
								       vnum)
							     != -1)
      || (IsNPC (victim) && IsQuester (ch) && ch->pcdata->quest.mob == victim)
      || victim->level >= level + 3 || (!IsNPC (victim)
					&& victim->level >= MAX_MORTAL_LEVEL)
      || (IsNPC (victim) && IsSet (victim->imm_flags, IMM_SUMMON))
      || (IsNPC (victim) && saves_spell (level, victim, DAM_NONE))
      || (is_clan (victim) && !is_same_clan (ch, victim)))
    {
      chprintln (ch, "You failed.");
      return false;
    }

  stone = get_eq_char (ch, WEAR_HOLD);
  if (!IsImmortal (ch) &&
      (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
      chprintln (ch, "You lack the proper component for this spell.");
      return false;
    }

  if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
      act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
      act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
      extract_obj (stone);
    }

  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 2 + level / 25;
  portal->value[3] = victim->in_room->vnum;

  obj_to_room (portal, ch->in_room);

  act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
  act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);
  return true;
}

Spell_Fun (spell_nexus)
{
  CharData *victim;
  ObjData *portal, *stone;
  RoomIndex *to_room, *from_room;

  from_room = ch->in_room;

  if ((victim = get_char_world (ch, target_name)) == NULL || victim == ch
      || (to_room = victim->in_room) == NULL || !can_see_room (ch, to_room)
      || !can_see_room (ch, from_room)
      || IsSet (to_room->room_flags, ROOM_SAFE)
      || IsSet (from_room->room_flags, ROOM_SAFE)
      || IsSet (to_room->room_flags, ROOM_PRIVATE)
      || IsSet (to_room->room_flags, ROOM_SOLITARY)
      || IsSet (to_room->room_flags, ROOM_NO_RECALL)
      || IsSet (from_room->room_flags, ROOM_NO_RECALL)
      || IsSet (to_room->area->area_flags, AREA_CLOSED)
      || IsSet (to_room->room_flags, ROOM_ARENA)
      || IsSet (from_room->room_flags, ROOM_ARENA) || (IsNPC (victim)
						       && is_gqmob (NULL,
								    victim->
								    pIndexData->
								    vnum) !=
						       -1) || (IsQuester (ch)
							       && ch->pcdata->
							       quest.mob ==
							       victim)
      || victim->level >= level + 3 || (!IsNPC (victim)
					&& victim->level >= MAX_MORTAL_LEVEL)
      || (IsNPC (victim) && IsSet (victim->imm_flags, IMM_SUMMON))
      || (IsNPC (victim) && saves_spell (level, victim, DAM_NONE))
      || (is_clan (victim) && !is_same_clan (ch, victim)))
    {
      chprintln (ch, "You failed.");
      return false;
    }

  stone = get_eq_char (ch, WEAR_HOLD);
  if (!IsImmortal (ch) &&
      (stone == NULL || stone->item_type != ITEM_WARP_STONE))
    {
      chprintln (ch, "You lack the proper component for this spell.");
      return false;
    }

  if (stone != NULL && stone->item_type == ITEM_WARP_STONE)
    {
      act ("You draw upon the power of $p.", ch, stone, NULL, TO_CHAR);
      act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
      extract_obj (stone);
    }


  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 10;
  portal->value[3] = to_room->vnum;

  obj_to_room (portal, from_room);

  act ("$p rises up from the ground.", ch, portal, NULL, TO_ROOM);
  act ("$p rises up before you.", ch, portal, NULL, TO_CHAR);


  if (to_room == from_room)
    return true;


  portal = create_object (get_obj_index (OBJ_VNUM_PORTAL), 0);
  portal->timer = 1 + level / 10;
  portal->value[3] = from_room->vnum;

  obj_to_room (portal, to_room);

  if (to_room->person_first != NULL)
    {
      act ("$p rises up from the ground.", to_room->person_first, portal,
	   NULL, TO_ROOM);
      act ("$p rises up from the ground.", to_room->person_first, portal,
	   NULL, TO_CHAR);
    }
  return true;
}

Spell_Fun (spell_forceshield)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already force-shielded.");
      else
	act ("$N is already force-shielded.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 4;
  af.location = APPLY_AC;
  af.modifier = (level / 5) * -1;
  af.bitvector = AFF_FORCE_SHIELD;
  affect_to_char (victim, &af);
  act ("A sparkling force-shield encircles $n.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "You are encircled by a sparkling force-shield.");
  return true;
}

Spell_Fun (spell_staticshield)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are surrounded by static charge.");
      else
	act ("$N is already surrounded by static charge.", ch, NULL,
	     victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.location = APPLY_AC;
  af.modifier = (level / 4) * -1;
  af.bitvector = AFF_STATIC_SHIELD;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a pulse of static charge.", victim, NULL, NULL,
       TO_ROOM);
  chprintln (victim, "You are surrounded by a pulse of static charge.");
  return true;
}

Spell_Fun (spell_flameshield)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already protected by fire.");
      else
	act ("$N is already protected by fire.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = (level / 10);
  af.location = APPLY_AC;
  af.modifier = (level / 2) * -1;
  af.bitvector = AFF_FLAME_SHIELD;
  affect_to_char (victim, &af);
  act ("$n is shielded by red walls of flame.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "You are shielded by red walls of flame.");
  return true;
}



Spell_Fun (spell_channel)
{
  CharData *victim = (CharData *) vo;
  int heal;

  heal = dice (3, 3) + (level / 3) * 2;
  if (ch == victim)
    {
      chprintln (ch, "You cannot channel energy into yourself.");
      return false;
    }
  victim->mana = Min (victim->mana + heal, victim->max_mana);
  update_pos (victim);
  chprintln (victim, "A swirling cloud of energy engulfs you!");
  if (ch != victim)
    chprintln (ch, "A swirling cloud of energy slips from your fingertips.");
  return true;
}


Spell_Fun (spell_investiture)
{
  CharData *victim = (CharData *) vo;
  int heal;

  heal = ch->move;
  victim->mana = Min (victim->mana + heal, victim->max_mana);
  victim->move = 0;
  update_pos (victim);
  chprintln (victim, "{cThe forces of the earth fill you with energy!{x");
  act ("$n draws magic from the very earth!", ch, NULL, NULL, TO_ROOM);
  return true;
}


Spell_Fun (spell_powerstorm)
{
  CharData *vch;
  CharData *vch_next;
  bool found = false;

  act ("$n makes a firey blaze of magic engulf the room!", ch, NULL, NULL,
       TO_ROOM);
  for (vch = char_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room)
	{
	  if (vch != ch && !is_safe_spell (ch, vch, true))
	    {
	      damage (ch, vch, level / 3 * 2 + dice (20, 20), sn, DAM_FIRE,
		      true);
	      found = true;
	    }
	  continue;
	}
      if (vch->in_room->area == ch->in_room->area)
	found = true;
      chprintln (vch, "A blazing storm of energy rumbles through the area.");
    }
  return found;
}


Spell_Fun (spell_mana_burn)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = dice (level, 13);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  fire_effect (victim, level / 2, dam / 10, TARGET_CHAR);
  damage (ch, victim, dam, sn, DAM_FIRE, true);
  return true;
}


Spell_Fun (spell_bark_skin)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "Your skin is already covered in bark.");
      else
	act ("$N's skin is already bark.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.modifier = -30 - level / 5;
  af.location = APPLY_AC;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "Your skin becomes as tough as bark.");
  if (ch != victim)
    act ("$N's skin becomes as tough as bark.", ch, NULL, victim, TO_CHAR);
  return true;
}


Spell_Fun (spell_spell_mantle)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already protected against magic.");
      else
	act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.modifier = 1 - level / 6;
  af.location = APPLY_SAVES;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "You are surrounded by a glowing spell mantle.");
  if (ch != victim)
    act ("$N is surrounded by a glowing spell mantle.", ch, NULL, victim,
	 TO_CHAR);
  return true;
}


Spell_Fun (spell_animal_instinct)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already animalistic.");
      else
	act ("$N is already animalistic.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.modifier = level / 25;
  af.location = APPLY_STR;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.modifier = level / 20;
  af.location = APPLY_DAMROLL;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "You suddenly look like a wild beast!");
  if (ch != victim)
    act ("$N suddenly grows fangs and claws!", ch, NULL, victim, TO_CHAR);
  return true;

}


Spell_Fun (spell_chaos_flare)
{
  CharData *victim = (CharData *) vo;
  AffectData af;
  int rnum;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already touched by chaos.");
      else
	act ("$N's skin is already touched by chaos.", ch, NULL, victim,
	     TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  rnum = number_percent ();
  if (rnum <= 5)
    {
      af.modifier = -30 - level / 5;
      af.location = APPLY_AC;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "Glinting scales form over your skin!");
      if (ch != victim)
	act ("$N's skin is suddenly covered with metallic scales.", ch,
	     NULL, victim, TO_CHAR);
      return true;
    }
  if (rnum <= 15)
    {
      af.modifier = level / 20;
      af.location = APPLY_DAMROLL;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "Sharp spikes jut out of your skin!");
      if (ch != victim)
	act ("$N's skin is suddenly covered with jagged spikes.", ch, NULL,
	     victim, TO_CHAR);
      return true;
    }
  if (rnum <= 25)
    {
      af.modifier = level / 20;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "Your eyes gleam.");
      if (ch != victim)
	act ("$N's eyes gleam.", ch, NULL, victim, TO_CHAR);
      return true;
    }
  if (rnum <= 35)
    {
      af.modifier = level * 2;
      af.location = APPLY_MOVE;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "You suddenly grow an extra set of legs!");
      if (ch != victim)
	act ("$N suddenly grows an extra set of legs! Yipes!", ch, NULL,
	     victim, TO_CHAR);
      return true;
    }
  if (rnum <= 45)
    {
      af.modifier = level / 20;
      af.location = APPLY_CON;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "You grow much tougher!");
      if (ch != victim)
	act ("$N seems much tougher all of a sudden.", ch, NULL, victim,
	     TO_CHAR);
      return true;
    }
  if (rnum <= 50)
    {
      af.modifier = level / 4;
      af.location = APPLY_DAMROLL;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "{YA blaze of light surrounds you!{x");
      if (ch != victim)
	act ("{YA blazing halo surrounds $N!{x", ch, NULL, victim, TO_CHAR);
      return true;
    }
  if (rnum <= 65)
    {
      af.modifier = 1 - level / 20;
      af.location = APPLY_DEX;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "One of your arms suddenly turns into a flipper.");
      if (ch != victim)
	act ("One of $N's arms turns into a.. dolphin flipper.", ch, NULL,
	     victim, TO_CHAR);
      return true;
    }
  if (rnum <= 75)
    {
      af.modifier = 1 - level / 20;
      af.location = APPLY_INT;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "Me say wah? You suddenly feel very stoopid.");
      if (ch != victim)
	act ("$N is suddenly looking very stupid.", ch, NULL, victim,
	     TO_CHAR);
      return true;
    }
  if (rnum <= 85)
    {
      af.modifier = level * 3;
      af.location = APPLY_HIT;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "You grow two sizes bigger!");
      if (ch != victim)
	act ("$N suddenly gets bigger.. and bigger.. and bigger.", ch,
	     NULL, victim, TO_CHAR);
      return true;
    }
  if (rnum <= 95)
    {
      af.modifier = 1 + level * 2;
      af.location = APPLY_AC;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim,
		 "You suddenly feel quite vulnerable. They're all out to get you!");
      if (ch != victim)
	act ("$N looks might paranoid all of a sudden.", ch, NULL, victim,
	     TO_CHAR);
      return true;
    }
  if (rnum <= 100)
    {
      af.modifier = 1 - level;
      af.location = APPLY_DAMROLL;
      af.bitvector = 0;
      affect_to_char (victim, &af);
      chprintln (victim, "{cAck! You turn into an oozing gelatinous blob!");
      if (ch != victim)
	act ("{c$N's been turned into a green oozing blob!{c", ch, NULL,
	     victim, TO_ROOM);
      return true;
    }

  return true;
}


Spell_Fun (spell_wild_magic)
{
  CharData *victim = (CharData *) vo;
  int dam;
  int numba;

  dam = dice (level * 3 / 2, 14);
  numba = number_percent ();
  if (numba <= 10)
    {
      if (saves_spell (level, victim, DAM_ACID))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_ACID, true);
      acid_effect (victim, level, dam, TARGET_CHAR);
      return true;
    }
  if (numba <= 20)
    {
      if (saves_spell (level, victim, DAM_FIRE))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_FIRE, true);
      fire_effect (victim, level, dam, TARGET_CHAR);
      return true;
    }
  if (numba <= 30)
    {
      if (saves_spell (level, victim, DAM_LIGHTNING))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_LIGHTNING, true);
      shock_effect (victim, level, dam, TARGET_CHAR);
      return true;
    }
  if (numba <= 40)
    {
      if (saves_spell (level, victim, DAM_COLD))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_COLD, true);
      cold_effect (victim, level, dam, TARGET_CHAR);
      return true;
    }
  if (numba <= 50)
    {
      if (saves_spell (level, victim, DAM_HOLY))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_HOLY, true);
      return true;
    }
  if (numba <= 60)
    {
      if (saves_spell (level, victim, DAM_LIGHT))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_LIGHT, true);
      return true;
    }
  if (numba <= 70)
    {
      if (saves_spell (level, victim, DAM_DROWNING))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_DROWNING, true);
      return true;
    }
  if (numba <= 80)
    {
      if (saves_spell (level, victim, DAM_DISEASE))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_DISEASE, true);
      return true;
    }
  if (numba <= 90)
    {
      if (saves_spell (level, victim, DAM_SLASH))
	dam /= 2;
      damage (ch, victim, dam, sn, DAM_SLASH, true);
      return true;
    }
  if (numba <= 100)
    {
      if (saves_spell (level, victim, DAM_NEGATIVE))
	dam /= 2;
      dam /= 5;
      damage (ch, victim, dam, sn, DAM_NEGATIVE, true);
      acid_effect (victim, level, dam, TARGET_CHAR);
      fire_effect (victim, level, dam, TARGET_CHAR);
      cold_effect (victim, level, dam, TARGET_CHAR);
      shock_effect (victim, level, dam, TARGET_CHAR);
      return true;
    }
  return true;
}
