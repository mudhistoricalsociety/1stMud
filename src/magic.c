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
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "vnums.h"


Proto (void say_spell, (CharData *, int));


Proto (bool remove_obj, (CharData *, wloc_t, bool));


Lookup_Fun (skill_lookup)
{
  int sn;

  for (sn = 0; sn < top_skill; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;
      if (tolower (name[0]) == tolower (skill_table[sn].name[0]) &&
	  !str_prefix (name, skill_table[sn].name))
	return sn;
    }

  return -1;
}

int
spell_lookup (Spell_F * fun)
{
  int sn;

  for (sn = 0; sn < top_skill; sn++)
    {
      if (skill_table[sn].spell_fun == NULL)
	continue;

      if (skill_table[sn].spell_fun == fun)
	return sn;
    }
  return -1;
}

int
find_spell (CharData * ch, const char *name)
{

  int sn, found = -1;

  if (IsNPC (ch))
    return skill_lookup (name);

  for (sn = 0; sn < top_skill; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;
      if (tolower (name[0]) == tolower (skill_table[sn].name[0]) &&
	  !str_prefix (name, skill_table[sn].name))
	{
	  if (found == -1)
	    found = sn;
	  if (can_use_skpell (ch, sn) && ch->pcdata->learned[sn] > 0)
	    return sn;
	}
    }
  return found;
}


void
say_spell (CharData * ch, int sn)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CharData *rch;
  const char *pName;
  int iSyl;
  int length;

  struct syl_type
  {
    char *old;
    char *pnew;
  };

  static const struct syl_type syl_table[] = {
    {" ", " "},
    {"ar", "abra"},
    {"au", "kada"},
    {"bless", "fido"},
    {"blind", "nose"},
    {"bur", "mosa"},
    {"cu", "judi"},
    {"de", "oculo"},
    {"en", "unso"},
    {"light", "dies"},
    {"lo", "hi"},
    {"mor", "zak"},
    {"move", "sido"},
    {"ness", "lacri"},
    {"ning", "illa"},
    {"per", "duda"},
    {"ra", "gru"},
    {"fresh", "ima"},
    {"re", "candus"},
    {"son", "sabru"},
    {"tect", "infra"},
    {"tri", "cula"},
    {"ven", "nofo"},
    {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
    {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
    {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
    {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
    {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
    {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
    {"y", "l"}, {"z", "k"},
    {"", ""}
  };

  buf[0] = '\0';
  for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      strcat (buf, syl_table[iSyl].pnew);
	      break;
	    }
	}

      if (length == 0)
	length = 1;
    }

  sprintf (buf2, "$n utters the words, '%s'.", buf);
  sprintf (buf, "$n utters the words, '%s'.", skill_table[sn].name);

  for (rch = ch->in_room->person_first; rch; rch = rch->next_in_room)
    {
      if (rch != ch)
	act ((!IsNPC (rch) && is_same_class (ch, rch)) ? buf : buf2,
	     ch, NULL, rch, TO_VICT);
    }

  return;
}


bool
saves_spell (int level, CharData * victim, dam_class dam_type)
{
  int save;

  save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;
  if (IsAffected (victim, AFF_BERSERK))
    save += victim->level / 2;

  switch (check_immune (victim, dam_type))
    {
    case IS_IMMUNE:
      return true;
    case IS_RESISTANT:
      save += 2;
      break;
    case IS_VULNERABLE:
      save -= 2;
      break;
    default:
      break;
    }

  if (!IsNPC (victim) && has_spells (victim))
    save = 9 * save / 10;
  save = Range (5, save, 95);
  return number_percent () < save;
}



bool
saves_dispel (int dis_level, int spell_level, int duration)
{
  int save;

  if (duration == -1)
    spell_level += 5;


  save = 50 + (spell_level - dis_level) * 5;
  save = Range (5, save, 95);
  return number_percent () < save;
}



bool
check_dispel (int dis_level, CharData * victim, int sn)
{
  AffectData *af;

  if (is_affected (victim, sn))
    {
      for (af = victim->affect_first; af != NULL; af = af->next)
	{
	  if (af->type == sn)
	    {
	      if (!saves_dispel (dis_level, af->level, af->duration))
		{
		  affect_strip (victim, sn);
		  if (skill_table[sn].msg_off)
		    {
		      chprintln (victim, skill_table[sn].msg_off);
		    }
		  return true;
		}
	      else
		af->level--;
	    }
	}
    }
  return false;
}


int
mana_cost (CharData * ch, int min_mana, int level)
{
  if (ch->level + 2 == level)
    return 1000;
  return Max (min_mana, (100 / (2 + ch->level - level)));
}


const char *target_name;

Do_Fun (do_cast)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *obj;
  void *vo;
  int mana;
  int sn;
  int target;


  if (IsNPC (ch) && ch->desc == NULL)
    return;

  target_name = one_argument (argument, arg1);
  one_argument (target_name, arg2);

  if (NullStr (arg1))
    {
      chprintln (ch, "Cast which what where?");
      return;
    }

  if ((sn = find_spell (ch, arg1)) < 1 ||
      skill_table[sn].spell_fun == spell_null || (!IsNPC (ch) &&
						  (!can_use_skpell (ch,
								    sn)
						   || ch->pcdata->
						   learned[sn] == 0)))
    {
      chprintln (ch, "You don't know any spells of that name.");
      return;
    }

  if (ch->position < skill_table[sn].minimum_position)
    {
      chprintln (ch, "You can't concentrate enough.");
      return;
    }

  if (ch->level + 2 == skill_level (ch, sn))
    mana = 50;
  else
    mana =
      Max (skill_table[sn].min_mana,
	   100 / (2 + ch->level - skill_level (ch, sn)));


  victim = NULL;
  obj = NULL;
  vo = NULL;
  target = TARGET_NONE;

  switch (skill_table[sn].target)
    {
    default:
      bugf ("Do_cast: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (NullStr (arg2))
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      chprintln (ch, "Cast the spell on whom?");
	      return;
	    }
	}
      else
	{
	  if ((victim = get_char_room (ch, NULL, target_name)) == NULL)
	    {
	      chprintln (ch, "They aren't here.");
	      return;
	    }
	}


      if (!IsNPC (ch))
	{

	  if (is_safe (ch, victim) && victim != ch)
	    {
	      chprintln (ch, "Not on that target.");
	      return;
	    }
	  check_killer (ch, victim);
	}

      if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
	{
	  chprintln (ch, "You can't do that on your own follower.");
	  return;
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_DEFENSIVE:
      if (NullStr (arg2))
	{
	  victim = ch;
	}
      else
	{
	  if ((victim = get_char_room (ch, NULL, target_name)) == NULL)
	    {
	      chprintln (ch, "They aren't here.");
	      return;
	    }
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if (!NullStr (arg2) && !is_name (target_name, ch->name))
	{
	  chprintln (ch, "You cannot cast this spell on another.");
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (NullStr (arg2))
	{
	  chprintln (ch, "What should the spell be cast upon?");
	  return;
	}

      if ((obj = get_obj_carry (ch, target_name, ch)) == NULL)
	{
	  chprintln (ch, "You are not carrying that.");
	  return;
	}

      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if (NullStr (arg2))
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      chprintln (ch, "Cast the spell on whom or what?");
	      return;
	    }

	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, NULL, target_name)) != NULL)
	{
	  target = TARGET_CHAR;
	}

      if (target == TARGET_CHAR)
	{
	  if (is_safe_spell (ch, victim, false) && victim != ch)
	    {
	      chprintln (ch, "Not on that target.");
	      return;
	    }

	  if (IsAffected (ch, AFF_CHARM) && ch->master == victim)
	    {
	      chprintln (ch, "You can't do that on your own follower.");
	      return;
	    }

	  if (!IsNPC (ch))
	    check_killer (ch, victim);

	  vo = (void *) victim;
	}
      else if ((obj = get_obj_here (ch, NULL, target_name)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
      break;

    case TAR_OBJ_CHAR_DEF:
      if (NullStr (arg2))
	{
	  vo = (void *) ch;
	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, NULL, target_name)) != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry (ch, target_name, ch)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
      break;
    }

  if (!IsNPC (ch) && ch->mana < mana)
    {
      chprintln (ch, "You don't have enough mana.");
      return;
    }

  if (str_cmp (skill_table[sn].name, "ventriloquate"))
    say_spell (ch, sn);

  WaitState (ch, skill_table[sn].beats);

  if (number_percent () > get_skill (ch, sn))
    {
      chprintln (ch, "You lost your concentration.");
      check_improve (ch, sn, false, 1);
      ch->mana -= mana / 2;
    }
  else
    {
      bool ret;

      ch->mana -= mana;
      if (IsNPC (ch) || has_spells (ch))

	ret = (*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
      else
	ret =
	  (*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo,
					target);
      check_improve (ch, sn, ret, 1);

      if (ret && skill_table[sn].sound)
	act_sound (skill_table[sn].sound, ch, vo,
		   skill_table[sn].sound->to, POS_RESTING);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
       (skill_table[sn].target == TAR_OBJ_CHAR_OFF &&
	target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CharData *vch;
      CharData *vch_next;

      for (vch = ch->in_room->person_first; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL)
	    {
	      check_killer (victim, ch);
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}


void
obj_cast_spell (int sn, int level, CharData * ch, CharData * victim,
		ObjData * obj)
{
  void *vo;
  int target = TARGET_NONE;
  bool ret;

  if (sn <= 0)
    return;

  if (sn >= top_skill || skill_table[sn].spell_fun == 0)
    {
      bugf ("Obj_cast_spell: bad sn %d.", sn);
      return;
    }

  switch (skill_table[sn].target)
    {
    default:
      bugf ("Obj_cast_spell: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
      vo = NULL;
      break;

    case TAR_CHAR_OFFENSIVE:
      if (victim == NULL)
	victim = ch->fighting;
      if (victim == NULL)
	{
	  chprintln (ch, "You can't do that.");
	  return;
	}
      if (is_safe (ch, victim) && ch != victim)
	{
	  chprintln (ch, "Something isn't right...");
	  return;
	}
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
      if (victim == NULL)
	victim = ch;
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (obj == NULL)
	{
	  chprintln (ch, "You can't do that.");
	  return;
	}
      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if (victim == NULL && obj == NULL)
	{
	  if (ch->fighting != NULL)
	    victim = ch->fighting;
	  else
	    {
	      chprintln (ch, "You can't do that.");
	      return;
	    }
	}
      if (victim != NULL)
	{
	  if (is_safe_spell (ch, victim, false) && ch != victim)
	    {
	      chprintln (ch, "Somehting isn't right...");
	      return;
	    }

	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      break;

    case TAR_OBJ_CHAR_DEF:
      if (victim == NULL && obj == NULL)
	{
	  vo = (void *) ch;
	  target = TARGET_CHAR;
	}
      else if (victim != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}

      break;
    }

  target_name = "";
  ret = (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);

  if (ret && skill_table[sn].sound)
    act_sound (skill_table[sn].sound, ch, vo, skill_table[sn].sound->to,
	       POS_RESTING);

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE ||
       (skill_table[sn].target == TAR_OBJ_CHAR_OFF &&
	target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CharData *vch;
      CharData *vch_next;

      for (vch = ch->in_room->person_first; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL)
	    {
	      check_killer (victim, ch);
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}


Spell_Fun (spell_acid_blast)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = dice (level, 12);
  if (saves_spell (level, victim, DAM_ACID))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_ACID, true);
}

Spell_Fun (spell_armor)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already armored.");
      else
	act ("$N is already armored.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.modifier = -20;
  af.location = APPLY_AC;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel someone protecting you.");
  if (ch != victim)
    act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_bless)
{
  CharData *victim;
  ObjData *obj;
  AffectData af;


  if (target == TARGET_OBJ)
    {
      obj = (ObjData *) vo;
      if (IsObjStat (obj, ITEM_BLESS))
	{
	  act ("$p is already blessed.", ch, obj, NULL, TO_CHAR);
	  return false;
	}

      if (IsObjStat (obj, ITEM_EVIL))
	{
	  AffectData *paf;

	  paf = affect_find (obj->affect_first, gsn_curse);
	  if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
	    {
	      if (paf != NULL)
		affect_remove_obj (obj, paf);
	      act ("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
	      RemBit (obj->extra_flags, ITEM_EVIL);
	      return true;
	    }
	  else
	    {
	      act ("The evil of $p is too powerful for you to overcome.",
		   ch, obj, NULL, TO_CHAR);
	      return false;
	    }
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = 6 + level;
      af.location = APPLY_SAVES;
      af.modifier = -1;
      af.bitvector = ITEM_BLESS;
      affect_to_obj (obj, &af);

      act ("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);

      if (obj->wear_loc != WEAR_NONE)
	ch->saving_throw -= 1;
      return true;
    }


  victim = (CharData *) vo;

  if (victim->position == POS_FIGHTING || is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already blessed.");
      else
	act ("$N already has divine favor.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 6 + level;
  af.location = APPLY_HITROLL;
  af.modifier = level / 8;
  af.bitvector = 0;
  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 0 - level / 8;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel righteous.");
  if (ch != victim)
    act ("You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_blindness)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_BLIND)
      || saves_spell (level, victim, DAM_OTHER))
    {
      chprintln (ch, "You failed.");
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.duration = 1 + level;
  af.bitvector = AFF_BLIND;
  affect_to_char (victim, &af);
  chprintln (victim, "You are blinded!");
  act ("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_burning_hands)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_FIRE, true);
}

Spell_Fun (spell_call_lightning)
{
  CharData *vch;
  CharData *vch_next;
  int dam;

  if (!IsOutside (ch))
    {
      chprintln (ch, "You must be out of doors.");
      return false;
    }

  if (ch->in_room->area->weather.precip <= 0)
    {
      chprintln (ch, "You need bad weather.");
      return false;
    }

  dam = dice (level / 2, 8);

  act ("$g's lightning strikes your foes!", ch, NULL, NULL, TO_CHAR);
  act ("$n calls $g's lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);

  for (vch = char_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room)
	{
	  if (vch != ch && (IsNPC (ch) ? !IsNPC (vch) : IsNPC (vch)))
	    damage (ch, vch,
		    saves_spell (level, vch,
				 DAM_LIGHTNING) ? dam / 2 : dam,
		    sn, DAM_LIGHTNING, true);
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area && IsOutside (vch) &&
	  IsAwake (vch))
	chprintln (vch, "Lightning flashes in the sky.");
    }

  return true;
}



Spell_Fun (spell_calm)
{
  CharData *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;
  int chance;
  AffectData af;
  bool found = false;


  for (vch = ch->in_room->person_first; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING)
	{
	  count++;
	  if (IsNPC (vch))
	    mlevel += vch->level;
	  else
	    mlevel += vch->level / 2;
	  high_level = Max (high_level, vch->level);
	}
    }


  chance = 4 * level - high_level + 2 * count;

  if (IsImmortal (ch))
    mlevel = 0;

  if (number_range (0, chance) >= mlevel)
    {
      for (vch = ch->in_room->person_first; vch != NULL;
	   vch = vch->next_in_room)
	{
	  if (IsNPC (vch) &&
	      (IsSet (vch->imm_flags, IMM_MAGIC) ||
	       IsSet (vch->act, ACT_UNDEAD)))
	    break;

	  if (IsAffected (vch, AFF_CALM) ||
	      IsAffected (vch, AFF_BERSERK) ||
	      is_affected (vch, skill_lookup ("frenzy")))
	    break;

	  found = true;

	  chprintln (vch, "A wave of calm passes over you.");

	  if (vch->fighting || vch->position == POS_FIGHTING)
	    stop_fighting (vch, false);

	  af.where = TO_AFFECTS;
	  af.type = sn;
	  af.level = level;
	  af.duration = level / 4;
	  af.location = APPLY_HITROLL;
	  if (!IsNPC (vch))
	    af.modifier = -5;
	  else
	    af.modifier = -2;
	  af.bitvector = AFF_CALM;
	  affect_to_char (vch, &af);

	  af.location = APPLY_DAMROLL;
	  affect_to_char (vch, &af);
	}
    }
  return found;
}

Spell_Fun (spell_cancellation)
{
  CharData *victim = (CharData *) vo;
  bool found = false;

  level += 2;

  if ((!IsNPC (ch) && IsNPC (victim) &&
       !(IsAffected (ch, AFF_CHARM) && ch->master == victim)) ||
      (IsNPC (ch) && !IsNPC (victim)))
    {
      chprintln (ch, "You failed, try dispel magic.");
      return false;
    }





  if (check_dispel (level, victim, skill_lookup ("armor")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("bless")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
      found = true;
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("calm")))
    {
      found = true;
      act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
      found = true;
      act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
      found = true;
      act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
      found = true;
      act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("curse")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect evil")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect good")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect invis")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect magic")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
      act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("fly")))
    {
      act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
      act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);
      ;
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
      act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("haste")))
    {
      act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("infravision")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("pass door")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("protection evil")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("protection good")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
      act ("The white aura around $n's body vanishes.", victim, NULL,
	   NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("shield")))
    {
      act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("sleep")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("slow")))
    {
      act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
      act ("$n's skin regains its normal texture.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
      act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("force shield")))
    {
      act ("The force-shield encircling $n fades.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }


  if (check_dispel (level, victim, skill_lookup ("static shield")))
    {
      act ("The static energy surrounding $n dissipates.", victim, NULL,
	   NULL, TO_ROOM);
      found = true;
    }


  if (check_dispel (level, victim, skill_lookup ("flame shield")))
    {
      act ("The flames protecting $n sputter and die.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }

  if (found)
    chprintln (ch, "Ok.");
  else
    chprintln (ch, "Spell failed.");

  return found;
}

Spell_Fun (spell_cause_light)
{
  return damage (ch, (CharData *) vo, dice (1, 8) + level / 3, sn,
		 DAM_HARM, true);
}

Spell_Fun (spell_cause_critical)
{
  return damage (ch, (CharData *) vo, dice (3, 8) + level - 6, sn,
		 DAM_HARM, true);
}

Spell_Fun (spell_cause_serious)
{
  return damage (ch, (CharData *) vo, dice (2, 8) + level / 2, sn,
		 DAM_HARM, true);
}

Spell_Fun (spell_chain_lightning)
{
  CharData *victim = (CharData *) vo;
  CharData *tmp_vict, *last_vict, *next_vict;
  bool found = false;
  int dam;



  act ("A lightning bolt leaps from $n's hand and arcs to $N.", ch, NULL,
       victim, TO_ROOM);
  act ("A lightning bolt leaps from your hand and arcs to $N.", ch, NULL,
       victim, TO_CHAR);
  act ("A lightning bolt leaps from $n's hand and hits you!", ch, NULL,
       victim, TO_VICT);

  dam = dice (level, 6);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 3;
  damage (ch, victim, dam, sn, DAM_LIGHTNING, true);
  last_vict = victim;
  level -= 4;


  while (level > 0)
    {
      found = false;
      for (tmp_vict = ch->in_room->person_first; tmp_vict != NULL;
	   tmp_vict = next_vict)
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell (ch, tmp_vict, true) && tmp_vict != last_vict)
	    {
	      found = true;
	      last_vict = tmp_vict;
	      act ("The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM);
	      act ("The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR);
	      dam = dice (level, 6);
	      if (saves_spell (level, tmp_vict, DAM_LIGHTNING))
		dam /= 3;
	      damage (ch, tmp_vict, dam, sn, DAM_LIGHTNING, true);
	      level -= 4;
	    }
	}

      if (!found)
	{
	  if (ch == NULL)
	    return false;

	  if (last_vict == ch)
	    {
	      act ("The bolt seems to have fizzled out.", ch,
		   NULL, NULL, TO_ROOM);
	      act ("The bolt grounds out through your body.",
		   ch, NULL, NULL, TO_CHAR);
	      return false;
	    }

	  last_vict = ch;
	  act ("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
	  chprintln (ch, "You are struck by your own lightning!");
	  dam = dice (level, 6);
	  if (saves_spell (level, ch, DAM_LIGHTNING))
	    dam /= 3;
	  damage (ch, ch, dam, sn, DAM_LIGHTNING, true);
	  level -= 4;
	  if (ch == NULL)
	    return false;
	}

    }
  return found;
}

Spell_Fun (spell_change_sex)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You've already been changed.");
      else
	act ("$N has already had $s(?) sex changed.", ch, NULL,
	     victim, TO_CHAR);
      return false;
    }
  if (saves_spell (level, victim, DAM_OTHER))
    return false;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_SEX;
  do
    {
      af.modifier = number_range (0, 2) - victim->sex;
    }
  while (af.modifier == 0);
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel different.");
  act ("$n doesn't look like $mself anymore...", victim, NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_charm_person)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_safe (ch, victim))
    return false;

  if (victim == ch)
    {
      chprintln (ch, "You like yourself even better!");
      return false;
    }

  if (IsAffected (victim, AFF_CHARM) || IsAffected (ch, AFF_CHARM) ||
      level < victim->level || IsSet (victim->imm_flags, IMM_CHARM) ||
      saves_spell (level, victim, DAM_CHARM))
    return false;

  if (IsSet (victim->in_room->room_flags, ROOM_LAW))
    {
      chprintln (ch, "The mayor does not allow charming in the city limits.");
      return false;
    }

  if (victim->master)
    stop_follower (victim);
  add_follower (victim, ch);
  victim->leader = ch;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char (victim, &af);
  act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
  if (ch != victim)
    act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_chill_touch)
{
  CharData *victim = (CharData *) vo;
  AffectData af;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (!saves_spell (level, victim, DAM_COLD))
    {
      act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = 6;
      af.location = APPLY_STR;
      af.modifier = -1;
      af.bitvector = 0;
      affect_join (victim, &af);
    }
  else
    {
      dam /= 2;
    }

  return damage (ch, victim, dam, sn, DAM_COLD, true);
}

Spell_Fun (spell_color_spray)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_LIGHT))
    dam /= 2;
  else
    spell_blindness (skill_lookup ("blindness"), level / 2, ch,
		     (void *) victim, TARGET_CHAR);

  return damage (ch, victim, dam, sn, DAM_LIGHT, true);
}

Spell_Fun (spell_continual_light)
{
  ObjData *light;

  if (!NullStr (target_name))
    {
      light = get_obj_carry (ch, target_name, ch);

      if (light == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return false;
	}

      if (IsObjStat (light, ITEM_GLOW))
	{
	  act ("$p is already glowing.", ch, light, NULL, TO_CHAR);
	  return false;
	}

      SetBit (light->extra_flags, ITEM_GLOW);
      act ("$p glows with a white light.", ch, light, NULL, TO_ALL);
      return true;
    }

  light = create_object (get_obj_index (OBJ_VNUM_LIGHT_BALL), 0);
  obj_to_room (light, ch->in_room);
  act ("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
  act ("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
  return true;
}

Spell_Fun (spell_control_weather)
{
  WeatherData *weath;
  int change;

  weath = &ch->in_room->area->weather;

  change = number_range (-mud_info.rand_factor, mud_info.rand_factor) +
    (ch->level * 3) / (2 * mud_info.max_vector);

  if (!str_cmp (target_name, "warmer"))
    weath->temp_vector += change;
  else if (!str_cmp (target_name, "colder"))
    weath->temp_vector -= change;
  else if (!str_cmp (target_name, "wetter"))
    weath->precip_vector += change;
  else if (!str_cmp (target_name, "drier"))
    weath->precip_vector -= change;
  else if (!str_cmp (target_name, "windier"))
    weath->wind_vector += change;
  else if (!str_cmp (target_name, "calmer"))
    weath->wind_vector -= change;
  else
    {
      chprintln (ch, "Do you want it to get warmer, colder, wetter, "
		 "drier, windier, or calmer?");
      return false;
    }

  weath->temp_vector = Range (-mud_info.max_vector,
			      weath->temp_vector, mud_info.max_vector);
  weath->precip_vector = Range (-mud_info.max_vector,
				weath->precip_vector, mud_info.max_vector);
  weath->wind_vector =
    Range (-mud_info.max_vector, weath->wind_vector, mud_info.max_vector);

  chprintln (ch, "The weather is altered by your magic.");
  return true;
}

Spell_Fun (spell_create_food)
{
  ObjData *mushroom;

  mushroom = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
  mushroom->value[0] = level / 2;
  mushroom->value[1] = level;
  obj_to_room (mushroom, ch->in_room);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
  return true;
}

Spell_Fun (spell_create_rose)
{
  ObjData *rose;

  rose = create_object (get_obj_index (OBJ_VNUM_ROSE), 0);
  act ("$n has created a beautiful red rose.", ch, rose, NULL, TO_ROOM);
  chprintln (ch, "You create a beautiful red rose.");
  obj_to_char (rose, ch);
  return true;
}

Spell_Fun (spell_create_spring)
{
  ObjData *spring;

  spring = create_object (get_obj_index (OBJ_VNUM_SPRING), 0);
  spring->timer = level;
  obj_to_room (spring, ch->in_room);
  act ("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
  act ("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
  return true;
}

Spell_Fun (spell_create_water)
{
  ObjData *obj = (ObjData *) vo;
  int water;
  WeatherData *weath;

  if (obj->item_type != ITEM_DRINK_CON)
    {
      chprintln (ch, "It is unable to hold water.");
      return false;
    }

  if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
    {
      chprintln (ch, "It contains some other liquid.");
      return false;
    }

  weath = &ch->in_room->area->weather;

  water = Min (level * (weath >= 0 ? 4 : 2), obj->value[0] - obj->value[1]);

  if (water > 0)
    {
      obj->value[2] = LIQ_WATER;
      obj->value[1] += water;
      if (!is_name ("water", obj->name))
	{
	  char buf[MAX_STRING_LENGTH];

	  sprintf (buf, "%s water", obj->name);
	  replace_str (&obj->name, buf);
	}
      act ("$p is filled.", ch, obj, NULL, TO_CHAR);
    }

  return true;
}

Spell_Fun (spell_cure_blindness)
{
  CharData *victim = (CharData *) vo;

  if (!is_affected (victim, gsn_blindness))
    {
      if (victim == ch)
	chprintln (ch, "You aren't blind.");
      else
	act ("$N doesn't appear to be blinded.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (check_dispel (level, victim, gsn_blindness))
    {
      chprintln (victim, "Your vision returns!");
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
      return true;
    }
  else
    {
      chprintln (ch, "Spell failed.");
      return false;
    }
}

Spell_Fun (spell_cure_critical)
{
  CharData *victim = (CharData *) vo;
  int heal;

  heal = dice (3, 8) + level - 6;
  victim->hit = Min (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  chprintln (victim, "You feel better!");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}


Spell_Fun (spell_cure_disease)
{
  CharData *victim = (CharData *) vo;

  if (!is_affected (victim, gsn_plague))
    {
      if (victim == ch)
	chprintln (ch, "You aren't ill.");
      else
	act ("$N doesn't appear to be diseased.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (check_dispel (level, victim, gsn_plague))
    {
      chprintln (victim, "Your sores vanish.");
      act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
	   TO_ROOM);
      return true;
    }
  else
    {
      chprintln (ch, "Spell failed.");
      return false;
    }
}

Spell_Fun (spell_cure_light)
{
  CharData *victim = (CharData *) vo;
  int heal;

  heal = dice (1, 8) + level / 3;
  victim->hit = Min (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  chprintln (victim, "You feel better!");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_cure_poison)
{
  CharData *victim = (CharData *) vo;

  if (!is_affected (victim, gsn_poison))
    {
      if (victim == ch)
	chprintln (ch, "You aren't poisoned.");
      else
	act ("$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (check_dispel (level, victim, gsn_poison))
    {
      chprintln (victim, "A warm feeling runs through your body.");
      act ("$n looks much better.", victim, NULL, NULL, TO_ROOM);
      return true;
    }
  else
    {
      chprintln (ch, "Spell failed.");
      return false;
    }
}

Spell_Fun (spell_cure_serious)
{
  CharData *victim = (CharData *) vo;
  int heal;

  heal = dice (2, 8) + level / 2;
  victim->hit = Min (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  chprintln (victim, "You feel better!");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_curse)
{
  CharData *victim;
  ObjData *obj;
  AffectData af;


  if (target == TARGET_OBJ)
    {
      obj = (ObjData *) vo;
      if (IsObjStat (obj, ITEM_EVIL))
	{
	  act ("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
	  return false;
	}

      if (IsObjStat (obj, ITEM_BLESS))
	{
	  AffectData *paf;

	  paf = affect_find (obj->affect_first, skill_lookup ("bless"));
	  if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
	    {
	      if (paf != NULL)
		affect_remove_obj (obj, paf);
	      act ("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
	      RemBit (obj->extra_flags, ITEM_BLESS);
	      return true;
	    }
	  else
	    {
	      act
		("The holy aura of $p is too powerful for you to overcome.",
		 ch, obj, NULL, TO_CHAR);
	      return false;
	    }
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = 2 * level;
      af.location = APPLY_SAVES;
      af.modifier = +1;
      af.bitvector = ITEM_EVIL;
      affect_to_obj (obj, &af);

      act ("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);

      if (obj->wear_loc != WEAR_NONE)
	ch->saving_throw += 1;
      return true;
    }


  victim = (CharData *) vo;

  if (IsAffected (victim, AFF_CURSE) ||
      saves_spell (level, victim, DAM_NEGATIVE))
    return false;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_HITROLL;
  af.modifier = -1 * (level / 8);
  af.bitvector = AFF_CURSE;
  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = level / 8;
  affect_to_char (victim, &af);

  chprintln (victim, "You feel unclean.");
  if (ch != victim)
    act ("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
  return true;
}



Spell_Fun (spell_demonfire)
{
  CharData *victim = (CharData *) vo;
  int dam;
  bool ret;

  if (!IsNPC (ch) && !IsEvil (ch))
    {
      victim = ch;
      chprintln (ch, "The demons turn upon you!");
    }

  ch->alignment = Max (-1000, ch->alignment - 50);

  if (victim != ch)
    {
      act ("$n calls forth the demons of Hell upon $N!", ch, NULL,
	   victim, TO_ROOM);
      act ("$n has assailed you with the demons of Hell!", ch, NULL,
	   victim, TO_VICT);
      chprintln (ch, "You conjure forth the demons of hell!");
    }
  dam = dice (level, 10);
  if (saves_spell (level, victim, DAM_NEGATIVE))
    dam /= 2;
  ret =
    spell_curse (gsn_curse, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
  return ret || damage (ch, victim, dam, sn, DAM_NEGATIVE, true);
}

Spell_Fun (spell_detect_evil)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_DETECT_EVIL))
    {
      if (victim == ch)
	chprintln (ch, "You can already sense evil.");
      else
	act ("$N can already detect evil.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char (victim, &af);
  chprintln (victim, "Your eyes tingle.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_detect_good)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_DETECT_GOOD))
    {
      if (victim == ch)
	chprintln (ch, "You can already sense good.");
      else
	act ("$N can already detect good.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_GOOD;
  affect_to_char (victim, &af);
  chprintln (victim, "Your eyes tingle.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_detect_hidden)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_DETECT_HIDDEN))
    {
      if (victim == ch)
	chprintln (ch, "You are already as alert as you can be. ");
      else
	act ("$N can already sense hidden lifeforms.", ch, NULL,
	     victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  affect_to_char (victim, &af);
  chprintln (victim, "Your awareness improves.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_detect_invis)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_DETECT_INVIS))
    {
      if (victim == ch)
	chprintln (ch, "You can already see invisible.");
      else
	act ("$N can already see invisible things.", ch, NULL, victim,
	     TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char (victim, &af);
  chprintln (victim, "Your eyes tingle.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_detect_magic)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_DETECT_MAGIC))
    {
      if (victim == ch)
	chprintln (ch, "You can already sense magical auras.");
      else
	act ("$N can already detect magic.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  affect_to_char (victim, &af);
  chprintln (victim, "Your eyes tingle.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_detect_poison)
{
  ObjData *obj = (ObjData *) vo;

  if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD)
    {
      if (obj->value[3] != 0)
	chprintln (ch, "You smell poisonous fumes.");
      else
	chprintln (ch, "It looks delicious.");
    }
  else
    {
      chprintln (ch, "It doesn't look poisoned.");
    }

  return true;
}

Spell_Fun (spell_dispel_evil)
{
  CharData *victim = (CharData *) vo;
  int dam;

  if (!IsNPC (ch) && IsEvil (ch))
    victim = ch;

  if (IsGood (victim))
    {
      act ("$G protects $N.", ch, NULL, victim, TO_ROOM);
      return false;
    }

  if (IsNeutral (victim))
    {
      act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (victim->hit > (ch->level * 4))
    dam = dice (level, 4);
  else
    dam = Max (victim->hit, dice (level, 4));
  if (saves_spell (level, victim, DAM_HOLY))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_HOLY, true);
}

Spell_Fun (spell_dispel_good)
{
  CharData *victim = (CharData *) vo;
  int dam;

  if (!IsNPC (ch) && IsGood (ch))
    victim = ch;

  if (IsEvil (victim))
    {
      act ("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
      return false;
    }

  if (IsNeutral (victim))
    {
      act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (victim->hit > (ch->level * 4))
    dam = dice (level, 4);
  else
    dam = Max (victim->hit, dice (level, 4));
  if (saves_spell (level, victim, DAM_NEGATIVE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_NEGATIVE, true);
}



Spell_Fun (spell_dispel_magic)
{
  CharData *victim = (CharData *) vo;
  bool found = false;

  if (saves_spell (level, victim, DAM_OTHER))
    {
      chprintln (victim, "You feel a brief tingling sensation.");
      chprintln (ch, "You failed.");
      return false;
    }



  if (check_dispel (level, victim, skill_lookup ("armor")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("bless")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
      found = true;
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("calm")))
    {
      found = true;
      act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
      found = true;
      act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
      found = true;
      act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
      found = true;
      act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("curse")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect evil")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect good")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect invis")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("detect magic")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
      act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("fly")))
    {
      act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
      act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);
      ;
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
      act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("haste")))
    {
      act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("infravision")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("pass door")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("protection evil")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("protection good")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
      act ("The white aura around $n's body vanishes.", victim, NULL,
	   NULL, TO_ROOM);
      found = true;
    }

  if (IsAffected (victim, AFF_SANCTUARY) &&
      !saves_dispel (level, victim->level, -1) &&
      !is_affected (victim, skill_lookup ("sanctuary")))
    {
      RemBit (victim->affected_by, AFF_SANCTUARY);
      act ("The white aura around $n's body vanishes.", victim, NULL,
	   NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("shield")))
    {
      act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("sleep")))
    found = true;

  if (check_dispel (level, victim, skill_lookup ("slow")))
    {
      act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
      act ("$n's skin regains its normal texture.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
      act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
      found = true;
    }

  if (check_dispel (level, victim, skill_lookup ("force shield")))
    {
      act ("The force-shield encircling $n fades.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }


  if (check_dispel (level, victim, skill_lookup ("static shield")))
    {
      act ("The static energy surrounding $n dissipates.", victim, NULL,
	   NULL, TO_ROOM);
      found = true;
    }


  if (check_dispel (level, victim, skill_lookup ("flame shield")))
    {
      act ("The flames protecting $n sputter and die.", victim, NULL, NULL,
	   TO_ROOM);
      found = true;
    }

  if (found)
    chprintln (ch, "Ok.");
  else
    chprintln (ch, "Spell failed.");
  return found;
}

Spell_Fun (spell_earthquake)
{
  CharData *vch;
  CharData *vch_next;

  chprintln (ch, "The earth trembles beneath your feet!");
  act ("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

  for (vch = char_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room)
	{
	  if (vch != ch && !is_safe_spell (ch, vch, true))
	    {
	      if (IsAffected (vch, AFF_FLYING))
		damage (ch, vch, 0, sn, DAM_BASH, true);
	      else
		damage (ch, vch, level + dice (2, 8), sn, DAM_BASH, true);
	    }
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area)
	chprintln (vch, "The earth trembles and shivers.");
    }

  return true;
}

Spell_Fun (spell_enchant_armor)
{
  ObjData *obj = (ObjData *) vo;
  AffectData *paf;
  int result, fail;
  int ac_bonus, added;
  bool ac_found = false;

  if (obj->item_type != ITEM_ARMOR)
    {
      chprintln (ch, "That isn't an armor.");
      return false;
    }

  if (obj->wear_loc != WEAR_NONE)
    {
      chprintln (ch, "The item must be carried to be enchanted.");
      return false;
    }

  if (IsObjStat (obj, ITEM_QUEST))
    {
      chprintln (ch, "You can't enchant quest items.");
      return false;
    }


  ac_bonus = 0;
  fail = 25;



  if (!obj->enchanted)
    for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
      {
	if (paf->location == APPLY_AC)
	  {
	    ac_bonus = paf->modifier;
	    ac_found = true;
	    fail += 5 * (ac_bonus * ac_bonus);
	  }
	else
	  fail += 20;
      }

  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->location == APPLY_AC)
	{
	  ac_bonus = paf->modifier;
	  ac_found = true;
	  fail += 5 * (ac_bonus * ac_bonus);
	}
      else
	fail += 20;
    }


  fail -= level;

  if (IsObjStat (obj, ITEM_BLESS))
    fail -= 15;
  if (IsObjStat (obj, ITEM_GLOW))
    fail -= 5;

  fail = Range (5, fail, 85);

  result = number_percent ();


  if (result < (fail / 5))
    {
      act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_CHAR);
      act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
      return false;
    }

  if (result < (fail / 3))
    {
      AffectData *paf_next;

      act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
      obj->enchanted = true;


      for (paf = obj->affect_first; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  free_affect (paf);
	}
      obj->affect_first = NULL;


      obj->extra_flags = 0;
      return false;
    }

  if (result <= fail)
    {
      chprintln (ch, "Nothing seemed to happen.");
      return false;
    }


  if (!obj->enchanted)
    {
      AffectData *af_new;

      obj->enchanted = true;

      for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
	{
	  af_new = new_affect ();

	  Link (af_new, obj->affect, next, prev);

	  af_new->where = paf->where;
	  af_new->type = Max (0, paf->type);
	  af_new->level = paf->level;
	  af_new->duration = paf->duration;
	  af_new->location = paf->location;
	  af_new->modifier = paf->modifier;
	  af_new->bitvector = paf->bitvector;
	}
    }

  if (result <= (90 - level / 5))
    {
      act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
      act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
      SetBit (obj->extra_flags, ITEM_MAGIC);
      added = -1;
    }
  else
    {

      act ("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
      act ("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
      SetBit (obj->extra_flags, ITEM_MAGIC);
      SetBit (obj->extra_flags, ITEM_GLOW);
      added = -2;
    }



  if (obj->level < MAX_MORTAL_LEVEL)
    obj->level = Min (MAX_MORTAL_LEVEL - 1, obj->level + 1);

  if (ac_found)
    {
      for (paf = obj->affect_first; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_AC)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = Max (paf->level, level);
	    }
	}
    }
  else
    {

      paf = new_affect ();

      paf->where = TO_OBJECT;
      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_AC;
      paf->modifier = added;
      paf->bitvector = 0;
      Link (paf, obj->affect, next, prev);
    }
  return true;
}

Spell_Fun (spell_enchant_weapon)
{
  ObjData *obj = (ObjData *) vo;
  AffectData *paf;
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = false, dam_found = false;

  if (obj->item_type != ITEM_WEAPON)
    {
      chprintln (ch, "That isn't a weapon.");
      return false;
    }

  if (obj->wear_loc != WEAR_NONE)
    {
      chprintln (ch, "The item must be carried to be enchanted.");
      return false;
    }

  if (IsObjStat (obj, ITEM_QUEST))
    {
      chprintln (ch, "You can't enchant quest items.");
      return false;
    }


  hit_bonus = 0;
  dam_bonus = 0;
  fail = 25;



  if (!obj->enchanted)
    for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
      {
	if (paf->location == APPLY_HITROLL)
	  {
	    hit_bonus = paf->modifier;
	    hit_found = true;
	    fail += 2 * (hit_bonus * hit_bonus);
	  }
	else if (paf->location == APPLY_DAMROLL)
	  {
	    dam_bonus = paf->modifier;
	    dam_found = true;
	    fail += 2 * (dam_bonus * dam_bonus);
	  }
	else
	  fail += 25;
      }

  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->location == APPLY_HITROLL)
	{
	  hit_bonus = paf->modifier;
	  hit_found = true;
	  fail += 2 * (hit_bonus * hit_bonus);
	}
      else if (paf->location == APPLY_DAMROLL)
	{
	  dam_bonus = paf->modifier;
	  dam_found = true;
	  fail += 2 * (dam_bonus * dam_bonus);
	}
      else
	fail += 25;
    }


  fail -= 3 * level / 2;

  if (IsObjStat (obj, ITEM_BLESS))
    fail -= 15;
  if (IsObjStat (obj, ITEM_GLOW))
    fail -= 5;

  fail = Range (5, fail, 95);

  result = number_percent ();


  if (result < (fail / 5))
    {
      act ("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
      act ("$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
      return false;
    }

  if (result < (fail / 2))
    {
      AffectData *paf_next;

      act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
      obj->enchanted = true;


      for (paf = obj->affect_first; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  free_affect (paf);
	}
      obj->affect_first = NULL;


      obj->extra_flags = 0;
      return false;
    }

  if (result <= fail)
    {
      chprintln (ch, "Nothing seemed to happen.");
      return false;
    }


  if (!obj->enchanted)
    {
      AffectData *af_new;

      obj->enchanted = true;

      for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
	{
	  af_new = new_affect ();

	  Link (af_new, obj->affect, next, prev);

	  af_new->where = paf->where;
	  af_new->type = Max (0, paf->type);
	  af_new->level = paf->level;
	  af_new->duration = paf->duration;
	  af_new->location = paf->location;
	  af_new->modifier = paf->modifier;
	  af_new->bitvector = paf->bitvector;
	}
    }

  if (result <= (100 - level / 5))
    {
      act ("$p glows blue.", ch, obj, NULL, TO_CHAR);
      act ("$p glows blue.", ch, obj, NULL, TO_ROOM);
      SetBit (obj->extra_flags, ITEM_MAGIC);
      added = 1;
    }
  else
    {

      act ("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
      act ("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
      SetBit (obj->extra_flags, ITEM_MAGIC);
      SetBit (obj->extra_flags, ITEM_GLOW);
      added = 2;
    }



  if (obj->level < MAX_MORTAL_LEVEL - 1)
    obj->level = Min (MAX_MORTAL_LEVEL - 1, obj->level + 1);

  if (dam_found)
    {
      for (paf = obj->affect_first; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_DAMROLL)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = Max (paf->level, level);
	      if (paf->modifier > 4)
		SetBit (obj->extra_flags, ITEM_HUM);
	    }
	}
    }
  else
    {

      paf = new_affect ();

      paf->where = TO_OBJECT;
      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_DAMROLL;
      paf->modifier = added;
      paf->bitvector = 0;
      Link (paf, obj->affect, next, prev);
    }

  if (hit_found)
    {
      for (paf = obj->affect_first; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_HITROLL)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = Max (paf->level, level);
	      if (paf->modifier > 4)
		SetBit (obj->extra_flags, ITEM_HUM);
	    }
	}
    }
  else
    {

      paf = new_affect ();

      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_HITROLL;
      paf->modifier = added;
      paf->bitvector = 0;
      Link (paf, obj->affect, next, prev);
    }
  return true;
}


Spell_Fun (spell_energy_drain)
{
  CharData *victim = (CharData *) vo;
  int dam;

  if (victim != ch)
    ch->alignment = Max (-1000, ch->alignment - 50);

  if (saves_spell (level, victim, DAM_NEGATIVE))
    {
      chprintln (victim, "You feel a momentary chill.");
      return false;
    }

  if (victim->level <= 2)
    {
      dam = ch->hit + 1;
    }
  else
    {
      gain_exp (victim, 0 - number_range (level / 2, 3 * level / 2));
      victim->mana /= 2;
      victim->move /= 2;
      dam = dice (1, level);
      ch->hit += dam;
    }

  chprintln (victim, "You feel your life slipping away!");
  chprintln (ch, "Wow....what a rush!");
  damage (ch, victim, dam, sn, DAM_NEGATIVE, true);

  return true;
}

Spell_Fun (spell_fireball)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_FIRE, true);
}

Spell_Fun (spell_fireproof)
{
  ObjData *obj = (ObjData *) vo;
  AffectData af;

  if (IsObjStat (obj, ITEM_BURN_PROOF))
    {
      act ("$p is already protected from burning.", ch, obj, NULL, TO_CHAR);
      return false;
    }

  af.where = TO_OBJECT;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = ITEM_BURN_PROOF;

  affect_to_obj (obj, &af);

  act ("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
  act ("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_flamestrike)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = dice (6 + level / 2, 8);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_FIRE, true);
}

Spell_Fun (spell_faerie_fire)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_FAERIE_FIRE))
    return false;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_AC;
  af.modifier = 2 * level;
  af.bitvector = AFF_FAERIE_FIRE;
  affect_to_char (victim, &af);
  chprintln (victim, "You are surrounded by a pink outline.");
  act ("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_faerie_fog)
{
  CharData *ich;
  bool found = false;

  act ("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
  chprintln (ch, "You conjure a cloud of purple smoke.");

  for (ich = ch->in_room->person_first; ich != NULL; ich = ich->next_in_room)
    {
      if (ich->invis_level > 0)
	continue;

      if (ich == ch || saves_spell (level, ich, DAM_OTHER))
	continue;

      affect_strip (ich, gsn_invis);
      affect_strip (ich, gsn_mass_invis);
      affect_strip (ich, gsn_sneak);
      RemBit (ich->affected_by, AFF_HIDE);
      RemBit (ich->affected_by, AFF_INVISIBLE);
      RemBit (ich->affected_by, AFF_SNEAK);
      act ("$n is revealed!", ich, NULL, NULL, TO_ROOM);
      chprintln (ich, "You are revealed!");
      found = true;
    }

  return found;
}

Spell_Fun (spell_floating_disc)
{
  ObjData *disc, *floating;

  floating = get_eq_char (ch, WEAR_FLOAT);
  if (floating != NULL && IsObjStat (floating, ITEM_NOREMOVE))
    {
      act ("You can't remove $p.", ch, floating, NULL, TO_CHAR);
      return false;
    }

  disc = create_object (get_obj_index (OBJ_VNUM_DISC), 0);
  disc->value[0] = ch->level * 10;
  disc->value[3] = ch->level * 5;
  disc->timer = ch->level * 2 - number_range (0, level / 2);

  act ("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
  chprintln (ch, "You create a floating disc.");
  obj_to_char (disc, ch);
  wear_obj (ch, disc, true);
  return true;
}

Spell_Fun (spell_fly)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_FLYING))
    {
      if (victim == ch)
	chprintln (ch, "You are already airborne.");
      else
	act ("$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level + 3;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char (victim, &af);
  chprintln (victim, "Your feet rise off the ground.");
  act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
  return true;
}



Spell_Fun (spell_frenzy)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn) || IsAffected (victim, AFF_BERSERK))
    {
      if (victim == ch)
	chprintln (ch, "You are already in a frenzy.");
      else
	act ("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (is_affected (victim, skill_lookup ("calm")))
    {
      if (victim == ch)
	chprintln (ch, "Why don't you just relax for a while?");
      else
	act ("$N doesn't look like $e wants to fight anymore.", ch,
	     NULL, victim, TO_CHAR);
      return false;
    }

  if ((IsGood (ch) && !IsGood (victim)) ||
      (IsNeutral (ch) && !IsNeutral (victim)) || (IsEvil (ch)
						  && !IsEvil (victim)))
    {
      act ("Your god doesn't seem to like $N", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.modifier = level / 6;
  af.bitvector = 0;

  af.location = APPLY_HITROLL;
  affect_to_char (victim, &af);

  af.location = APPLY_DAMROLL;
  affect_to_char (victim, &af);

  af.modifier = 10 * (level / 12);
  af.location = APPLY_AC;
  affect_to_char (victim, &af);

  chprintln (victim, "You are filled with holy wrath!");
  act ("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
  return true;
}



Spell_Fun (spell_gate)
{
  CharData *victim;
  bool gate_pet;

  if ((victim = get_char_world (ch, target_name)) == NULL || victim == ch
      || victim->in_room == NULL || !can_see_room (ch, victim->in_room)
      || IsSet (victim->in_room->room_flags, ROOM_SAFE)
      || IsSet (victim->in_room->room_flags, ROOM_ARENA)
      || IsSet (ch->in_room->room_flags, ROOM_ARENA)
      || IsSet (victim->in_room->room_flags, ROOM_PRIVATE)
      || IsSet (victim->in_room->room_flags, ROOM_SOLITARY)
      || IsSet (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IsSet (ch->in_room->room_flags, ROOM_NO_RECALL) || (IsNPC (victim)
							     &&
							     is_gqmob (NULL,
								       victim->
								       pIndexData->
								       vnum)
							     != -1)
      || (IsNPC (victim) && IsQuester (ch) && ch->pcdata->quest.mob == victim)
      || victim->level >= level + 3 || (is_clan (victim)
					&& !is_same_clan (ch, victim))
      || (!IsNPC (victim) && victim->level >= MAX_MORTAL_LEVEL)
      || (IsNPC (victim) && IsSet (victim->imm_flags, IMM_SUMMON))
      || (IsNPC (victim) && saves_spell (level, victim, DAM_OTHER)))
    {
      chprintln (ch, "You failed.");
      return false;
    }
  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = true;
  else
    gate_pet = false;

  act ("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM);
  chprintln (ch, "You step through a gate and vanish.");
  char_from_room (ch);
  char_to_room (ch, victim->in_room);

  act ("$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM);
  do_function (ch, &do_look, "auto");

  if (gate_pet)
    {
      act ("$n steps through a gate and vanishes.", ch->pet, NULL, NULL,
	   TO_ROOM);
      chprintln (ch->pet, "You step through a gate and vanish.");
      char_from_room (ch->pet);
      char_to_room (ch->pet, victim->in_room);
      act ("$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM);
      do_function (ch->pet, &do_look, "auto");
    }
  return true;
}

Spell_Fun (spell_giant_strength)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already as strong as you can get!");
      else
	act ("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = 0;
  affect_to_char (victim, &af);
  chprintln (victim, "Your muscles surge with heightened power!");
  act ("$n's muscles surge with heightened power.", victim, NULL, NULL,
       TO_ROOM);
  return true;
}

Spell_Fun (spell_harm)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = Max (20, victim->hit - dice (1, 4));
  if (saves_spell (level, victim, DAM_HARM))
    dam = Min (50, dam / 2);
  dam = Min (100, dam);
  return damage (ch, victim, dam, sn, DAM_HARM, true);
}



Spell_Fun (spell_haste)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn) || IsAffected (victim, AFF_HASTE) ||
      IsSet (victim->off_flags, OFF_FAST))
    {
      if (victim == ch)
	chprintln (ch, "You can't move any faster!");
      else
	act ("$N is already moving as fast as $E can.", ch, NULL,
	     victim, TO_CHAR);
      return false;
    }

  if (IsAffected (victim, AFF_SLOW))
    {
      if (!check_dispel (level, victim, skill_lookup ("slow")))
	{
	  if (victim != ch)
	    chprintln (ch, "Spell failed.");
	  chprintln (victim, "You feel momentarily faster.");
	  return false;
	}
      act ("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  if (victim == ch)
    af.duration = level / 2;
  else
    af.duration = level / 4;
  af.location = APPLY_DEX;
  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = AFF_HASTE;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel yourself moving more quickly.");
  act ("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_heal)
{
  CharData *victim = (CharData *) vo;

  victim->hit = Min (victim->hit + 100, victim->max_hit);
  update_pos (victim);
  chprintln (victim, "A warm feeling fills your body.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_heat_metal)
{
  CharData *victim = (CharData *) vo;
  ObjData *obj_lose, *obj_next;
  int dam = 0;
  bool fail = true;

  if (!saves_spell (level + 2, victim, DAM_FIRE) &&
      !IsSet (victim->imm_flags, IMM_FIRE))
    {
      for (obj_lose = victim->carrying_first; obj_lose != NULL;
	   obj_lose = obj_next)
	{
	  obj_next = obj_lose->next_content;
	  if (number_range (1, 2 * level) > obj_lose->level &&
	      !saves_spell (level, victim, DAM_FIRE) &&
	      !IsObjStat (obj_lose, ITEM_NONMETAL) &&
	      !IsObjStat (obj_lose, ITEM_BURN_PROOF))
	    {
	      switch (obj_lose->item_type)
		{
		case ITEM_ARMOR:
		  if (obj_lose->wear_loc != WEAR_NONE)
		    {
		      if (can_drop_obj (victim, obj_lose)
			  && (obj_lose->weight / 10) <
			  number_range (1,
					2 *
					get_curr_stat
					(victim,
					 STAT_DEX))
			  && remove_obj (victim, obj_lose->wear_loc, true))
			{
			  act ("$n yelps and throws $p to the ground!",
			       victim, obj_lose, NULL, TO_ROOM);
			  act
			    ("You remove and drop $p before it burns you.",
			     victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 3);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = false;
			}
		      else
			{

			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level));
			  fail = false;
			}

		    }
		  else
		    {

		      if (can_drop_obj (victim, obj_lose))
			{
			  act ("$n yelps and throws $p to the ground!",
			       victim, obj_lose, NULL, TO_ROOM);
			  act ("You and drop $p before it burns you.",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 6);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = false;
			}
		      else
			{

			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 2);
			  fail = false;
			}
		    }
		  break;
		case ITEM_WEAPON:
		  if (obj_lose->wear_loc != WEAR_NONE)
		    {
		      if (IsWeaponStat (obj_lose, WEAPON_FLAMING))
			continue;

		      if (can_drop_obj (victim, obj_lose)
			  && remove_obj (victim, obj_lose->wear_loc, true))
			{
			  act
			    ("$n is burned by $p, and throws it to the ground.",
			     victim, obj_lose, NULL, TO_ROOM);
			  chprintln (victim,
				     "You throw your red-hot weapon to the ground!");
			  dam += 1;
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = false;
			}
		      else
			{

			  chprintln (victim, "Your weapon sears your flesh!");
			  dam += number_range (1, obj_lose->level);
			  fail = false;
			}
		    }
		  else
		    {

		      if (can_drop_obj (victim, obj_lose))
			{
			  act
			    ("$n throws a burning hot $p to the ground!",
			     victim, obj_lose, NULL, TO_ROOM);
			  act ("You and drop $p before it burns you.",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 6);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = false;
			}
		      else
			{

			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 2);
			  fail = false;
			}
		    }
		  break;
		default:
		  break;
		}
	    }
	}
    }
  if (fail)
    {
      chprintln (ch, "Your spell had no effect.");
      chprintln (victim, "You feel momentarily warmer.");
      return false;
    }
  else
    {

      if (saves_spell (level, victim, DAM_FIRE))
	dam = 2 * dam / 3;
      damage (ch, victim, dam, sn, DAM_FIRE, true);
      return true;
    }
}


Spell_Fun (spell_holy_word)
{
  CharData *vch;
  CharData *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;

  bless_num = skill_lookup ("bless");
  curse_num = skill_lookup ("curse");
  frenzy_num = skill_lookup ("frenzy");

  act ("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
  chprintln (ch, "You utter a word of divine power.");

  for (vch = ch->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if ((IsGood (ch) && IsGood (vch)) ||
	  (IsEvil (ch) && IsEvil (vch)) || (IsNeutral (ch)
					    && IsNeutral (vch)))
	{
	  chprintln (vch, "You feel full more powerful.");
	  spell_frenzy (frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
	  spell_bless (bless_num, level, ch, (void *) vch, TARGET_CHAR);
	}
      else if ((IsGood (ch) && IsEvil (vch)) || (IsEvil (ch) && IsGood (vch)))
	{
	  if (!is_safe_spell (ch, vch, true))
	    {
	      spell_curse (curse_num, level, ch, (void *) vch, TARGET_CHAR);
	      chprintln (vch, "You are struck down!");
	      dam = dice (level, 6);
	      damage (ch, vch, dam, sn, DAM_ENERGY, true);
	    }
	}
      else if (IsNeutral (ch))
	{
	  if (!is_safe_spell (ch, vch, true))
	    {
	      spell_curse (curse_num, level / 2, ch,
			   (void *) vch, TARGET_CHAR);
	      chprintln (vch, "You are struck down!");
	      dam = dice (level, 4);
	      damage (ch, vch, dam, sn, DAM_ENERGY, true);
	    }
	}
    }

  chprintln (ch, "You feel drained.");
  ch->move = 0;
  ch->hit /= 2;
  return true;
}

Spell_Fun (spell_identify)
{
  ObjData *obj = (ObjData *) vo;
  char buf[MAX_STRING_LENGTH];
  AffectData *paf;

  chprintlnf (ch,
	      "Object '%s' is type %s, extra flags %s." NEWLINE
	      "Weight is %d, value is %ld, level is %d.",
	      obj->name, flag_string (type_flags, obj->item_type),
	      flag_string (extra_flags, obj->extra_flags),
	      obj->weight / 10, obj->cost, obj->level);

  switch (obj->item_type)
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      sprintf (buf, "Level %ld spells of:", obj->value[0]);
      chprint (ch, buf);

      if (obj->value[1] >= 0 && obj->value[1] < top_skill)
	{
	  chprint (ch, " '");
	  chprint (ch, skill_table[obj->value[1]].name);
	  chprint (ch, "'");
	}

      if (obj->value[2] >= 0 && obj->value[2] < top_skill)
	{
	  chprint (ch, " '");
	  chprint (ch, skill_table[obj->value[2]].name);
	  chprint (ch, "'");
	}

      if (obj->value[3] >= 0 && obj->value[3] < top_skill)
	{
	  chprint (ch, " '");
	  chprint (ch, skill_table[obj->value[3]].name);
	  chprint (ch, "'");
	}

      if (obj->value[4] >= 0 && obj->value[4] < top_skill)
	{
	  chprint (ch, " '");
	  chprint (ch, skill_table[obj->value[4]].name);
	  chprint (ch, "'");
	}

      chprintln (ch, ".");
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      chprintf (ch, "Has %ld charges of level %ld", obj->value[2],
		obj->value[0]);

      if (obj->value[3] >= 0 && obj->value[3] < top_skill)
	{
	  chprint (ch, " '");
	  chprint (ch, skill_table[obj->value[3]].name);
	  chprint (ch, "'");
	}

      chprintln (ch, ".");
      break;

    case ITEM_DRINK_CON:
      chprintlnf (ch, "It holds %s-colored %s.",
		  liq_table[obj->value[2]].liq_color,
		  liq_table[obj->value[2]].liq_name);
      break;

    case ITEM_CONTAINER:
      chprintlnf (ch,
		  "Capacity: %ld#  Maximum weight: %ld#  flags: %s",
		  obj->value[0], obj->value[3],
		  flag_string (container_flags, obj->value[1]));
      if (obj->value[4] != 100)
	{
	  chprintlnf (ch, "Weight multiplier: %ld%%", obj->value[4]);
	}
      break;

    case ITEM_WEAPON:
      chprint (ch, "Weapon type is ");
      switch (obj->value[0])
	{
	case (WEAPON_EXOTIC):
	  chprintln (ch, "exotic.");
	  break;
	case (WEAPON_SWORD):
	  chprintln (ch, "sword.");
	  break;
	case (WEAPON_DAGGER):
	  chprintln (ch, "dagger.");
	  break;
	case (WEAPON_SPEAR):
	  chprintln (ch, "spear/staff.");
	  break;
	case (WEAPON_MACE):
	  chprintln (ch, "mace/club.");
	  break;
	case (WEAPON_AXE):
	  chprintln (ch, "axe.");
	  break;
	case (WEAPON_FLAIL):
	  chprintln (ch, "flail.");
	  break;
	case (WEAPON_WHIP):
	  chprintln (ch, "whip.");
	  break;
	case (WEAPON_POLEARM):
	  chprintln (ch, "polearm.");
	  break;
	default:
	  chprintln (ch, "unknown.");
	  break;
	}
      if (obj->pIndexData->new_format)
	chprintlnf (ch, "Damage is %ldd%ld (average %ld).",
		    obj->value[1], obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
      else
	chprintlnf (ch, "Damage is %ld to %ld (average %ld).",
		    obj->value[1], obj->value[2],
		    (obj->value[1] + obj->value[2]) / 2);
      if (obj->value[4])
	{
	  chprintlnf (ch, "Weapons flags: %s",
		      flag_string (weapon_flags, obj->value[4]));
	}
      break;

    case ITEM_ARMOR:
      chprintlnf (ch,
		  "Armor class is %ld pierce, %ld bash, %ld slash, and %ld vs. magic.",
		  obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
      break;
    default:
      break;
    }

  if (!obj->enchanted)
    for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
      {
	if (paf->location != APPLY_NONE && paf->modifier != 0)
	  {
	    chprintlnf (ch, "Affects %s by %d.",
			flag_string (apply_types, paf->location),
			paf->modifier);
	    if (paf->bitvector)
	      {
		switch (paf->where)
		  {
		  case TO_AFFECTS:
		    chprintlnf (ch, "Adds %s affect.",
				flag_string (affect_flags, paf->bitvector));
		    break;
		  case TO_OBJECT:
		    chprintlnf (ch,
				"Adds %s object flag.",
				flag_string (extra_flags, paf->bitvector));
		    break;
		  case TO_IMMUNE:
		    chprintlnf (ch,
				"Adds immunity to %s.",
				flag_string (imm_flags, paf->bitvector));
		    break;
		  case TO_RESIST:
		    chprintlnf (ch,
				"Adds resistance to %s.",
				flag_string (imm_flags, paf->bitvector));
		    break;
		  case TO_VULN:
		    chprintlnf (ch,
				"Adds vulnerability to %s.",
				flag_string (imm_flags, paf->bitvector));
		    break;
		  default:
		    chprintlnf (ch, "Unknown bit %d.", paf->where);
		    break;
		  }
	      }
	  }
      }

  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->location != APPLY_NONE && paf->modifier != 0)
	{
	  chprintf (ch, "Affects %s by %d",
		    flag_string (apply_types, paf->location), paf->modifier);
	  if (paf->duration > -1)
	    chprintlnf (ch, ", %d hours.", paf->duration);
	  else
	    chprintln (ch, ".");
	  if (paf->bitvector)
	    {
	      switch (paf->where)
		{
		case TO_AFFECTS:
		  chprintlnf (ch, "Adds %s affect.",
			      flag_string (affect_flags, paf->bitvector));
		  break;
		case TO_OBJECT:
		  chprintlnf (ch, "Adds %s object flag.",
			      flag_string (extra_flags, paf->bitvector));
		  break;
		case TO_WEAPON:
		  chprintlnf (ch, "Adds %s weapon flags.",
			      flag_string (weapon_flags, paf->bitvector));
		  break;
		case TO_IMMUNE:
		  chprintlnf (ch, "Adds immunity to %s.",
			      flag_string (imm_flags, paf->bitvector));
		  break;
		case TO_RESIST:
		  chprintlnf (ch,
			      "Adds resistance to %s.",
			      flag_string (imm_flags, paf->bitvector));
		  break;
		case TO_VULN:
		  chprintlnf (ch,
			      "Adds vulnerability to %s.",
			      flag_string (imm_flags, paf->bitvector));
		  break;
		default:
		  chprintlnf (ch, "Unknown bit %d.", paf->where);
		  break;
		}
	    }
	}
    }

  return true;
}

Spell_Fun (spell_infravision)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_INFRARED))
    {
      if (victim == ch)
	chprintln (ch, "You can already see in the dark.");
      else
	act ("$N already has infravision.", ch, NULL, victim, TO_CHAR);
      return false;
    }
  act ("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM);

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INFRARED;
  affect_to_char (victim, &af);
  chprintln (victim, "Your eyes glow red.");
  return true;
}

Spell_Fun (spell_invis)
{
  CharData *victim;
  ObjData *obj;
  AffectData af;


  if (target == TARGET_OBJ)
    {
      obj = (ObjData *) vo;

      if (IsObjStat (obj, ITEM_INVIS))
	{
	  act ("$p is already invisible.", ch, obj, NULL, TO_CHAR);
	  return false;
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = level + 12;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = ITEM_INVIS;
      affect_to_obj (obj, &af);

      act ("$p fades out of sight.", ch, obj, NULL, TO_ALL);
      return true;
    }


  victim = (CharData *) vo;

  if (IsAffected (victim, AFF_INVISIBLE))
    return false;

  act ("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level + 12;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INVISIBLE;
  affect_to_char (victim, &af);
  chprintln (victim, "You fade out of existence.");
  return true;
}

Spell_Fun (spell_know_alignment)
{
  CharData *victim = (CharData *) vo;
  char *msg;
  int ap;

  ap = victim->alignment;

  if (ap > 700)
    msg = "$N has a pure and good aura.";
  else if (ap > 350)
    msg = "$N is of excellent moral character.";
  else if (ap > 100)
    msg = "$N is often kind and thoughtful.";
  else if (ap > -100)
    msg = "$N doesn't have a firm moral commitment.";
  else if (ap > -350)
    msg = "$N lies to $S friends.";
  else if (ap > -700)
    msg = "$N is a black-hearted murderer.";
  else
    msg = "$N is the embodiment of pure evil!.";

  act (msg, ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_lightning_bolt)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_LIGHTNING, true);
}

Spell_Fun (spell_locate_object)
{
  Buffer *buffer;
  ObjData *obj;
  ObjData *in_obj;
  bool found;
  int number = 0, max_found;

  found = false;
  number = 0;
  max_found = IsImmortal (ch) ? 200 : 2 * level;

  buffer = new_buf ();

  for (obj = obj_first; obj != NULL; obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !is_name (target_name, obj->name)
	  || IsObjStat (obj, ITEM_NOLOCATE) ||
	  number_percent () > 2 * level || ch->level < obj->level)
	continue;

      found = true;
      number++;

      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
	;

      if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by))
	{
	  bprintlnf (buffer, "one is carried by %s",
		     Pers (in_obj->carried_by, ch));
	}
      else
	{
	  if (IsImmortal (ch) && in_obj->in_room != NULL)
	    bprintlnf (buffer, "one is in %s [Room %ld]",
		       in_obj->in_room->name, in_obj->in_room->vnum);
	  else
	    bprintlnf (buffer, "one is in %s",
		       in_obj->in_room ==
		       NULL ? "somewhere" : in_obj->in_room->name);
	}

      if (number >= max_found)
	break;
    }

  if (!found)
    chprintln (ch, "Nothing like that in heaven or earth.");
  else
    sendpage (ch, buf_string (buffer));

  free_buf (buffer);

  return found;
}

Spell_Fun (spell_magic_missile)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_ENERGY))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_ENERGY, true);
}

Spell_Fun (spell_mass_healing)
{
  CharData *gch;
  int heal_num, refresh_num;

  heal_num = skill_lookup ("heal");
  refresh_num = skill_lookup ("refresh");

  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      if ((IsNPC (ch) && IsNPC (gch)) || (!IsNPC (ch) && !IsNPC (gch)))
	{
	  spell_heal (heal_num, level, ch, (void *) gch, TARGET_CHAR);
	  spell_refresh (refresh_num, level, ch, (void *) gch, TARGET_CHAR);
	}
    }
  return true;
}

Spell_Fun (spell_mass_invis)
{
  AffectData af;
  CharData *gch;

  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      if (!is_same_group (gch, ch) || IsAffected (gch, AFF_INVISIBLE))
	continue;
      act ("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
      chprintln (gch, "You slowly fade out of existence.");

      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level / 2;
      af.duration = 24;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char (gch, &af);
    }
  chprintln (ch, "Ok.");

  return true;
}

Spell_Fun (spell_null)
{
  chprintln (ch, "That's not a spell!");
  return false;
}

Spell_Fun (spell_pass_door)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_PASS_DOOR))
    {
      if (victim == ch)
	chprintln (ch, "You are already out of phase.");
      else
	act ("$N is already shifted out of phase.", ch, NULL, victim,
	     TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char (victim, &af);
  act ("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "You turn translucent.");
  return true;
}



Spell_Fun (spell_plague)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IsNPC (victim) && IsSet (victim->act, ACT_UNDEAD)))
    {
      if (ch == victim)
	chprintln (ch, "You feel momentarily ill, but it passes.");
      else
	act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level * 3 / 4;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = -5;
  af.bitvector = AFF_PLAGUE;
  affect_join (victim, &af);

  chprintln (victim,
	     "You scream in agony as plague sores erupt from your skin.");
  act ("$n screams in agony as plague sores erupt from $s skin.", victim,
       NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_poison)
{
  CharData *victim;
  ObjData *obj;
  AffectData af;

  if (target == TARGET_OBJ)
    {
      obj = (ObjData *) vo;

      if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	  if (IsObjStat (obj, ITEM_BLESS) || IsObjStat (obj, ITEM_BURN_PROOF))
	    {
	      act ("Your spell fails to corrupt $p.", ch, obj, NULL, TO_CHAR);
	      return false;
	    }
	  obj->value[3] = 1;
	  act ("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
	  return false;
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
	      return false;
	    }

	  if (IsWeaponStat (obj, WEAPON_POISON))
	    {
	      act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
	      return false;
	    }

	  af.where = TO_WEAPON;
	  af.type = sn;
	  af.level = level / 2;
	  af.duration = level / 8;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = WEAPON_POISON;
	  affect_to_obj (obj, &af);

	  act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
	  return true;
	}

      act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
      return false;
    }

  victim = (CharData *) vo;

  if (saves_spell (level, victim, DAM_POISON))
    {
      act ("$n turns slightly green, but it passes.", victim, NULL,
	   NULL, TO_ROOM);
      chprintln (victim, "You feel momentarily ill, but it passes.");
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = -2;
  af.bitvector = AFF_POISON;
  affect_join (victim, &af);
  chprintln (victim, "You feel very sick.");
  act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_protection_evil)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_PROTECT_EVIL) ||
      IsAffected (victim, AFF_PROTECT_GOOD))
    {
      if (victim == ch)
	chprintln (ch, "You are already protected.");
      else
	act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.location = APPLY_SAVING_SPELL;
  af.modifier = -1;
  af.bitvector = AFF_PROTECT_EVIL;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel holy and pure.");
  if (ch != victim)
    act ("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_protection_good)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_PROTECT_GOOD) ||
      IsAffected (victim, AFF_PROTECT_EVIL))
    {
      if (victim == ch)
	chprintln (ch, "You are already protected.");
      else
	act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.location = APPLY_SAVING_SPELL;
  af.modifier = -1;
  af.bitvector = AFF_PROTECT_GOOD;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel aligned with darkness.");
  if (ch != victim)
    act ("$N is protected from good.", ch, NULL, victim, TO_CHAR);
  return true;
}

Spell_Fun (spell_ray_of_truth)
{
  CharData *victim = (CharData *) vo;
  int dam, align;
  bool ret;

  if (IsEvil (ch))
    {
      victim = ch;
      chprintln (ch, "The energy explodes inside you!");
    }

  if (victim != ch)
    {
      act ("$n raises $s hand, and a blinding ray of light shoots forth!",
	   ch, NULL, NULL, TO_ROOM);
      chprintln (ch,
		 "You raise your hand and a blinding ray of light shoots forth!");
    }

  if (IsGood (victim))
    {
      act ("$n seems unharmed by the light.", victim, NULL, victim, TO_ROOM);
      chprintln (victim, "The light seems powerless to affect you.");
      return false;
    }

  dam = dice (level, 10);
  if (saves_spell (level, victim, DAM_HOLY))
    dam /= 2;

  align = victim->alignment;
  align -= 350;

  if (align < -1000)
    align = -1000 + (align + 1000) / 3;

  dam = (dam * align * align) / 1000000;

  ret = damage (ch, victim, dam, sn, DAM_HOLY, true);
  return ret
    || spell_blindness (gsn_blindness, 3 * level / 4, ch,
			(void *) victim, TARGET_CHAR);
}

Spell_Fun (spell_recharge)
{
  ObjData *obj = (ObjData *) vo;
  int chance, percent;

  if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
      chprintln (ch, "That item does not carry charges.");
      return false;
    }

  if (obj->value[3] >= 3 * level / 2)
    {
      chprintln (ch, "Your skills are not great enough for that.");
      return false;
    }

  if (obj->value[1] == 0)
    {
      chprintln (ch, "That item has already been recharged once.");
      return false;
    }

  chance = 40 + 2 * level;

  chance -= obj->value[3];
  chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);

  chance = Max (level / 2, chance);

  percent = number_percent ();

  if (percent < chance / 2)
    {
      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
      act ("$p glows softly.", ch, obj, NULL, TO_ROOM);
      obj->value[2] = Max (obj->value[1], obj->value[2]);
      obj->value[1] = 0;
      return true;
    }
  else if (percent <= chance)
    {
      int chargeback, chargemax;

      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);

      chargemax = obj->value[1] - obj->value[2];

      if (chargemax > 0)
	chargeback = Max (1, chargemax * percent / 100);
      else
	chargeback = 0;

      obj->value[2] += chargeback;
      obj->value[1] = 0;
      return true;
    }
  else if (percent <= Min (95, 3 * chance / 2))
    {
      chprintln (ch, "Nothing seems to happen.");
      if (obj->value[1] > 1)
	obj->value[1]--;
      return false;
    }
  else
    {

      act ("$p glows brightly and explodes!", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly and explodes!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
      return false;
    }
}

Spell_Fun (spell_refresh)
{
  CharData *victim = (CharData *) vo;

  victim->move = Min (victim->move + level, victim->max_move);
  if (victim->max_move == victim->move)
    chprintln (victim, "You feel fully refreshed!");
  else
    chprintln (victim, "You feel less tired.");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}

Spell_Fun (spell_remove_curse)
{
  CharData *victim;
  ObjData *obj;
  bool found = false;


  if (target == TARGET_OBJ)
    {
      obj = (ObjData *) vo;

      if (IsObjStat (obj, ITEM_NODROP) || IsObjStat (obj, ITEM_NOREMOVE))
	{
	  if (!IsObjStat (obj, ITEM_NOUNCURSE) &&
	      !saves_dispel (level + 2, obj->level, 0))
	    {
	      RemBit (obj->extra_flags, ITEM_NODROP);
	      RemBit (obj->extra_flags, ITEM_NOREMOVE);
	      act ("$p glows blue.", ch, obj, NULL, TO_ALL);
	      return true;
	    }

	  act ("The curse on $p is beyond your power.", ch, obj,
	       NULL, TO_CHAR);
	  return false;
	}
      act ("There doesn't seem to be a curse on $p.", ch, obj, NULL, TO_CHAR);
      return false;
    }


  victim = (CharData *) vo;

  if (check_dispel (level, victim, gsn_curse))
    {
      chprintln (victim, "You feel better.");
      act ("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
    }

  for (obj = victim->carrying_first; (obj != NULL && !found);
       obj = obj->next_content)
    {
      if ((IsObjStat (obj, ITEM_NODROP)
	   || IsObjStat (obj, ITEM_NOREMOVE))
	  && !IsObjStat (obj, ITEM_NOUNCURSE))
	{
	  if (!saves_dispel (level, obj->level, 0))
	    {
	      found = true;
	      RemBit (obj->extra_flags, ITEM_NODROP);
	      RemBit (obj->extra_flags, ITEM_NOREMOVE);
	      act ("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
	      act ("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
	    }
	}
    }
  return found;
}

Spell_Fun (spell_sanctuary)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_SANCTUARY))
    {
      if (victim == ch)
	chprintln (ch, "You are already in sanctuary.");
      else
	act ("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 6;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SANCTUARY;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "You are surrounded by a white aura.");
  return true;
}

Spell_Fun (spell_shield)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	chprintln (ch, "You are already shielded from harm.");
      else
	act ("$N is already protected by a shield.", ch, NULL, victim,
	     TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 8 + level;
  af.location = APPLY_AC;
  af.modifier = -20;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "You are surrounded by a force shield.");
  return true;
}

Spell_Fun (spell_shocking_grasp)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range ((level | 50) / 2, (level | 50) * 2);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_LIGHTNING, true);
}

Spell_Fun (spell_sleep)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (IsAffected (victim, AFF_SLEEP) ||
      (IsNPC (victim) && IsSet (victim->act, ACT_UNDEAD)) ||
      (level + 2) < victim->level ||
      saves_spell (level - 4, victim, DAM_CHARM))
    return false;

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 4 + level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SLEEP;
  affect_join (victim, &af);

  if (IsAwake (victim))
    {
      chprintln (victim, "You feel very sleepy ..... zzzzzz.");
      act ("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
      victim->position = POS_SLEEPING;
    }
  return true;
}

Spell_Fun (spell_slow)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn) || IsAffected (victim, AFF_SLOW))
    {
      if (victim == ch)
	chprintln (ch, "You can't move any slower!");
      else
	act ("$N can't get any slower than that.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  if (saves_spell (level, victim, DAM_OTHER) ||
      IsSet (victim->imm_flags, IMM_MAGIC))
    {
      if (victim != ch)
	chprintln (ch, "Nothing seemed to happen.");
      chprintln (victim, "You feel momentarily lethargic.");
      return false;
    }

  if (IsAffected (victim, AFF_HASTE))
    {
      if (!check_dispel (level, victim, skill_lookup ("haste")))
	{
	  if (victim != ch)
	    chprintln (ch, "Spell failed.");
	  chprintln (victim, "You feel momentarily slower.");
	  return false;
	}

      act ("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
      return true;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.location = APPLY_DEX;
  af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
  af.bitvector = AFF_SLOW;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel yourself slowing d o w n...");
  act ("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
  return true;
}

Spell_Fun (spell_stone_skin)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (ch, sn))
    {
      if (victim == ch)
	chprintln (ch, "Your skin is already as hard as a rock.");
      else
	act ("$N is already as hard as can be.", ch, NULL, victim, TO_CHAR);
      return false;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_AC;
  af.modifier = -40;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  act ("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
  chprintln (victim, "Your skin turns to stone.");
  return true;
}

Spell_Fun (spell_summon)
{
  CharData *victim;

  if ((victim = get_char_world (ch, target_name)) == NULL ||
      victim == ch || victim->in_room == NULL ||
      IsSet (ch->in_room->room_flags, ROOM_SAFE) ||
      IsSet (victim->in_room->room_flags, ROOM_SAFE) ||
      IsSet (victim->in_room->room_flags, ROOM_PRIVATE) ||
      IsSet (victim->in_room->room_flags, ROOM_SOLITARY) ||
      IsSet (victim->in_room->room_flags, ROOM_NO_RECALL)
      || IsSet (victim->in_room->area->area_flags, AREA_CLOSED)
      || IsSet (victim->in_room->room_flags, ROOM_ARENA)
      || IsSet (ch->in_room->room_flags, ROOM_ARENA)
      || (IsNPC (victim)
	  && is_gqmob (NULL, victim->pIndexData->vnum) != -1)
      || (IsNPC (victim) && IsQuester (ch)
	  && ch->pcdata->quest.mob == victim)
      || (IsNPC (victim) && IsSet (victim->act, ACT_AGGRESSIVE))
      || victim->level >= level + 3 || (!IsNPC (victim)
					&& victim->level >=
					LEVEL_IMMORTAL)
      || victim->fighting != NULL || (IsNPC (victim)
				      && IsSet (victim->imm_flags,
						IMM_SUMMON))
      || (IsNPC (victim) && victim->pIndexData->pShop != NULL)
      || (!IsNPC (victim) && IsSet (victim->act, PLR_NOSUMMON))
      || (IsNPC (victim) && saves_spell (level, victim, DAM_OTHER)))
    {
      chprintln (ch, "You failed.");
      return false;
    }

  act ("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, ch->in_room);
  act ("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
  act ("$n has summoned you!", ch, NULL, victim, TO_VICT);
  do_function (victim, &do_look, "auto");
  return true;
}

Spell_Fun (spell_teleport)
{
  CharData *victim = (CharData *) vo;
  RoomIndex *pRoomIndex;

  if (victim->in_room == NULL ||
      IsSet (victim->in_room->room_flags, ROOM_NO_RECALL) ||
      (victim != ch && IsSet (victim->imm_flags, IMM_SUMMON)) ||
      (!IsNPC (ch) && victim->fighting != NULL) || (victim != ch &&
						    (saves_spell
						     (level - 5,
						      victim, DAM_OTHER))))
    {
      chprintln (ch, "You failed.");
      return false;
    }

  pRoomIndex = get_random_room (victim);

  if (victim != ch)
    chprintln (victim, "You have been teleported!");

  act ("$n vanishes!", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, pRoomIndex);
  act ("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
  do_function (victim, &do_look, "auto");
  return true;
}

Spell_Fun (spell_ventriloquate)
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  CharData *vch;
  bool found = false;

  target_name = one_argument (target_name, speaker);

  sprintf (buf1, "%s says '%s'.", speaker, target_name);
  sprintf (buf2, "Someone makes %s say '%s'.", speaker, target_name);
  buf1[skipcol (buf1)] = toupper (buf1[skipcol (buf1)]);

  for (vch = ch->in_room->person_first; vch != NULL; vch = vch->next_in_room)
    {
      if (is_name (speaker, vch->name) && IsAwake (vch))
	{
	  chprintln (vch, saves_spell (level, vch, DAM_OTHER) ? buf2 : buf1);
	  found = true;
	}
    }

  return found;
}

Spell_Fun (spell_weaken)
{
  CharData *victim = (CharData *) vo;
  AffectData af;

  if (is_affected (victim, sn) || saves_spell (level, victim, DAM_OTHER))
    return false;

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.location = APPLY_STR;
  af.modifier = -1 * (level / 5);
  af.bitvector = AFF_WEAKEN;
  affect_to_char (victim, &af);
  chprintln (victim, "You feel your strength slip away.");
  act ("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);
  return true;
}



Spell_Fun (spell_word_of_recall)
{
  CharData *victim = (CharData *) vo;
  RoomIndex *location;

  if (IsNPC (victim))
    return false;

  location = get_room_index (ROOM_VNUM_TEMPLE);
  perform_recall (victim, location, "recall");
  return true;
}


Spell_Fun (spell_acid_breath)
{
  CharData *victim = (CharData *) vo;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
  act ("$n spits a stream of corrosive acid at you.", ch, NULL, victim,
       TO_VICT);
  act ("You spit acid at $N.", ch, NULL, victim, TO_CHAR);

  hpch = Max (12, ch->hit);
  hp_dam = number_range (hpch / 11 + 1, hpch / 6);
  dice_dam = dice (level, 16);

  dam = Max (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

  if (saves_spell (level, victim, DAM_ACID))
    {
      acid_effect (victim, level / 2, dam / 4, TARGET_CHAR);
      damage (ch, victim, dam / 2, sn, DAM_ACID, true);
    }
  else
    {
      acid_effect (victim, level, dam, TARGET_CHAR);
      damage (ch, victim, dam, sn, DAM_ACID, true);
    }
  return true;
}

Spell_Fun (spell_fire_breath)
{
  CharData *victim = (CharData *) vo;
  CharData *vch, *vch_next;
  int dam, hp_dam, dice_dam;
  int hpch;
  bool found = false;

  act ("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
  act ("$n breathes a cone of hot fire over you!", ch, NULL, victim, TO_VICT);
  act ("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

  hpch = Max (10, ch->hit);
  hp_dam = number_range (hpch / 9 + 1, hpch / 5);
  dice_dam = dice (level, 20);

  dam = Max (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  fire_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

  for (vch = victim->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, true) ||
	  (IsNPC (vch) && IsNPC (ch) &&
	   (ch->fighting != vch || vch->fighting != ch)))
	continue;

      found = true;
      if (vch == victim)
	{
	  if (saves_spell (level, vch, DAM_FIRE))
	    {
	      fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage (ch, vch, dam / 2, sn, DAM_FIRE, true);
	    }
	  else
	    {
	      fire_effect (vch, level, dam, TARGET_CHAR);
	      damage (ch, vch, dam, sn, DAM_FIRE, true);
	    }
	}
      else
	{

	  if (saves_spell (level - 2, vch, DAM_FIRE))
	    {
	      fire_effect (vch, level / 4, dam / 8, TARGET_CHAR);
	      damage (ch, vch, dam / 4, sn, DAM_FIRE, true);
	    }
	  else
	    {
	      fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage (ch, vch, dam / 2, sn, DAM_FIRE, true);
	    }
	}
    }
  return found;
}

Spell_Fun (spell_frost_breath)
{
  CharData *victim = (CharData *) vo;
  CharData *vch, *vch_next;
  int dam, hp_dam, dice_dam, hpch;
  bool found = false;

  act ("$n breathes out a freezing cone of frost!", ch, NULL, victim,
       TO_NOTVICT);
  act ("$n breathes a freezing cone of frost over you!", ch, NULL, victim,
       TO_VICT);
  act ("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);

  hpch = Max (12, ch->hit);
  hp_dam = number_range (hpch / 11 + 1, hpch / 6);
  dice_dam = dice (level, 16);

  dam = Max (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  cold_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

  for (vch = victim->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, true) ||
	  (IsNPC (vch) && IsNPC (ch) &&
	   (ch->fighting != vch || vch->fighting != ch)))
	continue;

      found = true;
      if (vch == victim)
	{
	  if (saves_spell (level, vch, DAM_COLD))
	    {
	      cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage (ch, vch, dam / 2, sn, DAM_COLD, true);
	    }
	  else
	    {
	      cold_effect (vch, level, dam, TARGET_CHAR);
	      damage (ch, vch, dam, sn, DAM_COLD, true);
	    }
	}
      else
	{
	  if (saves_spell (level - 2, vch, DAM_COLD))
	    {
	      cold_effect (vch, level / 4, dam / 8, TARGET_CHAR);
	      damage (ch, vch, dam / 4, sn, DAM_COLD, true);
	    }
	  else
	    {
	      cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage (ch, vch, dam / 2, sn, DAM_COLD, true);
	    }
	}
    }
  return found;
}

Spell_Fun (spell_gas_breath)
{
  CharData *vch;
  CharData *vch_next;
  int dam, hp_dam, dice_dam, hpch;
  bool found = false;

  act ("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL, TO_ROOM);
  act ("You breath out a cloud of poisonous gas.", ch, NULL, NULL, TO_CHAR);

  hpch = Max (16, ch->hit);
  hp_dam = number_range (hpch / 15 + 1, 8);
  dice_dam = dice (level, 12);

  dam = Max (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  poison_effect (ch->in_room, level, dam, TARGET_ROOM);

  for (vch = ch->in_room->person_first; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, true) ||
	  (IsNPC (ch) && IsNPC (vch) &&
	   (ch->fighting == vch || vch->fighting == ch)))
	continue;

      found = true;
      if (saves_spell (level, vch, DAM_POISON))
	{
	  poison_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	  damage (ch, vch, dam / 2, sn, DAM_POISON, true);
	}
      else
	{
	  poison_effect (vch, level, dam, TARGET_CHAR);
	  damage (ch, vch, dam, sn, DAM_POISON, true);
	}
    }
  return found;
}

Spell_Fun (spell_lightning_breath)
{
  CharData *victim = (CharData *) vo;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n breathes a bolt of lightning at $N.", ch, NULL, victim,
       TO_NOTVICT);
  act ("$n breathes a bolt of lightning at you!", ch, NULL, victim, TO_VICT);
  act ("You breathe a bolt of lightning at $N.", ch, NULL, victim, TO_CHAR);

  hpch = Max (10, ch->hit);
  hp_dam = number_range (hpch / 9 + 1, hpch / 5);
  dice_dam = dice (level, 20);

  dam = Max (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

  if (saves_spell (level, victim, DAM_LIGHTNING))
    {
      shock_effect (victim, level / 2, dam / 4, TARGET_CHAR);
      damage (ch, victim, dam / 2, sn, DAM_LIGHTNING, true);
    }
  else
    {
      shock_effect (victim, level, dam, TARGET_CHAR);
      damage (ch, victim, dam, sn, DAM_LIGHTNING, true);
    }
  return true;
}


Spell_Fun (spell_general_purpose)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range (25, 100);
  if (saves_spell (level, victim, DAM_PIERCE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_PIERCE, true);
}

Spell_Fun (spell_high_explosive)
{
  CharData *victim = (CharData *) vo;
  int dam;

  dam = number_range (30, 120);
  if (saves_spell (level, victim, DAM_PIERCE))
    dam /= 2;
  return damage (ch, victim, dam, sn, DAM_PIERCE, true);
}


Spell_Fun (spell_trivia_pill)
{
  CharData *victim = (CharData *) vo;

  if (victim == NULL)
    victim = ch;

  if (IsNPC (victim))
    return false;

  victim->pcdata->trivia++;
  chprintln (victim, "You've gained a Trivia Point!");
  if (ch != victim)
    chprintln (ch, "Ok.");
  return true;
}
