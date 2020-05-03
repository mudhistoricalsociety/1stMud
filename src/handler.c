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
#include "vnums.h"


Proto (void affect_modify, (CharData *, AffectData *, bool));


bool
is_friend (CharData * ch, CharData * victim)
{
  if (is_same_group (ch, victim))
    return true;

  if (!IsNPC (ch))
    return false;

  if (!IsNPC (victim))
    {
      if (IsSet (ch->off_flags, ASSIST_PLAYERS))
	return true;
      else
	return false;
    }

  if (IsAffected (ch, AFF_CHARM))
    return false;

  if (IsSet (ch->off_flags, ASSIST_ALL))
    return true;

  if (ch->group && ch->group == victim->group)
    return true;

  if (IsSet (ch->off_flags, ASSIST_VNUM) &&
      ch->pIndexData == victim->pIndexData)
    return true;

  if (IsSet (ch->off_flags, ASSIST_RACE) && ch->race == victim->race)
    return true;

  if (IsSet (ch->off_flags, ASSIST_ALIGN) &&
      !IsSet (ch->act, ACT_NOALIGN) &&
      !IsSet (victim->act, ACT_NOALIGN) &&
      ((IsGood (ch) && IsGood (victim)) ||
       (IsEvil (ch) && IsEvil (victim)) || (IsNeutral (ch) &&
					    IsNeutral (victim))))
    return true;

  return false;
}


int
count_users (ObjData * obj)
{
  CharData *fch;
  int count = 0;

  if (obj->in_room == NULL)
    return 0;

  for (fch = obj->in_room->person_first; fch != NULL; fch = fch->next_in_room)
    if (fch->on == obj)
      count++;

  return count;
}

Lookup_Fun (weapon_lookup)
{
  int type;

  for (type = 0; weapon_table[type].name != NULL; type++)
    {
      if (tolower (name[0]) == tolower (weapon_table[type].name[0]) &&
	  !str_prefix (name, weapon_table[type].name))
	return type;
    }

  return -1;
}

weapon_t
weapon_class (const char *name)
{
  int type;

  for (type = 0; weapon_table[type].name != NULL; type++)
    {
      if (tolower (name[0]) == tolower (weapon_table[type].name[0]) &&
	  !str_prefix (name, weapon_table[type].name))
	return weapon_table[type].type;
    }

  return WEAPON_EXOTIC;
}

const char *
weapon_name (int weapon_ptype)
{
  int type;

  for (type = 0; weapon_table[type].name != NULL; type++)
    if (weapon_ptype == weapon_table[type].type)
      return weapon_table[type].name;
  return "exotic";
}

Lookup_Fun (attack_lookup)
{
  int att;

  for (att = 0; attack_table[att].name != NULL; att++)
    {
      if (tolower (name[0]) == tolower (attack_table[att].name[0]) &&
	  !str_prefix (name, attack_table[att].name))
	return att;
    }

  return 0;
}


Lookup_Fun (wiznet_lookup)
{
  int flag;

  for (flag = 0; wiznet_table[flag].name != NULL; flag++)
    {
      if (tolower (name[0]) == tolower (wiznet_table[flag].name[0]) &&
	  !str_prefix (name, wiznet_table[flag].name))
	return flag;
    }

  return -1;
}


Lookup_Fun (class_lookup)
{
  int Class;

  for (Class = 0; Class < top_class; Class++)
    {
      if (tolower (name[0]) == tolower (class_table[Class].name[0][0]) &&
	  !str_prefix (name, class_table[Class].name[0]))
	return Class;
    }

  return -1;
}



immune_t
check_immune (CharData * ch, dam_class dam_type)
{
  immune_t immune, def;
  flag_t bit;

  immune = IMMUNE_NONE;
  def = IS_NORMAL;

  if (dam_type == DAM_NONE)
    return immune;

  switch (dam_type)
    {
    case DAM_NONE:
    case DAM_BASH:
    case DAM_PIERCE:
    case DAM_SLASH:
      if (IsSet (ch->imm_flags, IMM_WEAPON))
	def = IS_IMMUNE;
      else if (IsSet (ch->res_flags, RES_WEAPON))
	def = IS_RESISTANT;
      else if (IsSet (ch->vuln_flags, VULN_WEAPON))
	def = IS_VULNERABLE;
      break;
    default:
      if (IsSet (ch->imm_flags, IMM_MAGIC))
	def = IS_IMMUNE;
      else if (IsSet (ch->res_flags, RES_MAGIC))
	def = IS_RESISTANT;
      else if (IsSet (ch->vuln_flags, VULN_MAGIC))
	def = IS_VULNERABLE;
      break;
    }


  switch (dam_type)
    {
    case (DAM_BASH):
      bit = IMM_BASH;
      break;
    case (DAM_PIERCE):
      bit = IMM_PIERCE;
      break;
    case (DAM_SLASH):
      bit = IMM_SLASH;
      break;
    case (DAM_FIRE):
      bit = IMM_FIRE;
      break;
    case (DAM_COLD):
      bit = IMM_COLD;
      break;
    case (DAM_LIGHTNING):
      bit = IMM_LIGHTNING;
      break;
    case (DAM_ACID):
      bit = IMM_ACID;
      break;
    case (DAM_POISON):
      bit = IMM_POISON;
      break;
    case (DAM_NEGATIVE):
      bit = IMM_NEGATIVE;
      break;
    case (DAM_HOLY):
      bit = IMM_HOLY;
      break;
    case (DAM_ENERGY):
      bit = IMM_ENERGY;
      break;
    case (DAM_MENTAL):
      bit = IMM_MENTAL;
      break;
    case (DAM_DISEASE):
      bit = IMM_DISEASE;
      break;
    case (DAM_DROWNING):
      bit = IMM_DROWNING;
      break;
    case (DAM_LIGHT):
      bit = IMM_LIGHT;
      break;
    case (DAM_CHARM):
      bit = IMM_CHARM;
      break;
    case (DAM_SOUND):
      bit = IMM_SOUND;
      break;
    default:
      return def;
    }

  if (IsSet (ch->imm_flags, bit))
    immune = IS_IMMUNE;
  else if (IsSet (ch->res_flags, bit) && immune != IS_IMMUNE)
    immune = IS_RESISTANT;
  else if (IsSet (ch->vuln_flags, bit))
    {
      if (immune == IS_IMMUNE)
	immune = IS_RESISTANT;
      else if (immune == IS_RESISTANT)
	immune = IS_NORMAL;
      else
	immune = IS_VULNERABLE;
    }

  if (immune == IMMUNE_NONE)
    return def;
  else
    return immune;
}

bool
is_clan (CharData * ch)
{
  return CharClan (ch) != NULL;
}

bool
is_true_clan (CharData * ch)
{
  return CharClan (ch) != NULL
    && !IsSet (CharClan (ch)->flags, CLAN_INDEPENDENT);
}

bool
is_same_clan (CharData * ch, CharData * victim)
{
  if (!is_true_clan (ch) || !is_true_clan (victim))
    return false;
  else
    return (CharClan (ch) == CharClan (victim));
}


bool
is_old_mob (CharData * ch)
{
  if (ch->pIndexData == NULL)
    return false;
  else if (ch->pIndexData->new_format)
    return false;
  return true;
}


int
get_skill (CharData * ch, int sn)
{
  int skill;

  if (sn == -1)
    {
      skill = ch->level * 5 / 2;
    }
  else if (sn < -1 || sn > top_skill)
    {
      bugf ("Bad sn %d in get_skill.", sn);
      skill = 0;
    }
  else if (!IsNPC (ch))
    {
      if (!can_use_skpell (ch, sn))
	skill = 0;
      else
	skill = ch->pcdata->learned[sn];
    }
  else
    {

      if (ch->level > 2)
	skill = ch->level / 2 + ch->level / 3;
      else
	skill = ch->level;
    }

  if (ch->daze > 0)
    {
      if (skill_table[sn].spell_fun != spell_null)
	skill /= 2;
      else
	skill = 2 * skill / 3;
    }

  if (!IsNPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    skill = 9 * skill / 10;

  return Range (0, skill, 100);
}


int
get_weapon_sn (CharData * ch)
{
  ObjData *wield;
  int sn;

  wield = get_eq_char (ch, WEAR_WIELD);
  if (wield == NULL || wield->item_type != ITEM_WEAPON)
    sn = gsn_hand_to_hand;
  else
    switch (wield->value[0])
      {
      default:
	sn = -1;
	break;
      case (WEAPON_SWORD):
	sn = gsn_sword;
	break;
      case (WEAPON_DAGGER):
	sn = gsn_dagger;
	break;
      case (WEAPON_SPEAR):
	sn = gsn_spear;
	break;
      case (WEAPON_MACE):
	sn = gsn_mace;
	break;
      case (WEAPON_AXE):
	sn = gsn_axe;
	break;
      case (WEAPON_FLAIL):
	sn = gsn_flail;
	break;
      case (WEAPON_WHIP):
	sn = gsn_whip;
	break;
      case (WEAPON_POLEARM):
	sn = gsn_polearm;
	break;
      }
  return sn;
}

int
get_weapon_skill (CharData * ch, int sn)
{
  int skill;


  if (IsNPC (ch))
    {
      if (sn == -1)
	skill = 3 * ch->level;
      else if (sn == gsn_hand_to_hand)
	skill = 40 + 2 * ch->level;
      else
	skill = 40 + 5 * ch->level / 2;
    }
  else
    {
      if (sn == -1)
	skill = 3 * ch->level;
      else
	skill = ch->pcdata->learned[sn];
    }

  return Range (0, skill, 100);
}


void
reset_char (CharData * ch)
{
  int loc, mod, stat;
  ObjData *obj;
  AffectData *af;
  int i;

  if (IsNPC (ch))
    return;

  if (ch->pcdata->perm_hit == 0 || ch->pcdata->perm_mana == 0 ||
      ch->pcdata->perm_move == 0 || ch->pcdata->last_level == 0)
    {

      for (loc = 0; loc < MAX_WEAR; loc++)
	{
	  obj = get_eq_char (ch, (wloc_t) loc);
	  if (obj == NULL)
	    continue;
	  if (!obj->enchanted)
	    for (af = obj->pIndexData->affect_first; af != NULL;
		 af = af->next)
	      {
		mod = af->modifier;
		switch (af->location)
		  {
		  case APPLY_SEX:
		    ch->sex = (sex_t) ((int) ch->sex - mod);
		    if (ch->sex < SEX_NEUTRAL || ch->sex > SEX_FEMALE)
		      ch->sex =
			IsNPC (ch) ? SEX_NEUTRAL : ch->pcdata->true_sex;
		    break;
		  case APPLY_MANA:
		    ch->max_mana -= mod;
		    break;
		  case APPLY_HIT:
		    ch->max_hit -= mod;
		    break;
		  case APPLY_MOVE:
		    ch->max_move -= mod;
		    break;
		  default:
		    break;
		  }
	      }

	  for (af = obj->affect_first; af != NULL; af = af->next)
	    {
	      mod = af->modifier;
	      switch (af->location)
		{
		case APPLY_SEX:
		  ch->sex = (sex_t) ((int) ch->sex - mod);
		  break;
		case APPLY_MANA:
		  ch->max_mana -= mod;
		  break;
		case APPLY_HIT:
		  ch->max_hit -= mod;
		  break;
		case APPLY_MOVE:
		  ch->max_move -= mod;
		  break;
		default:
		  break;
		}
	    }
	}

      ch->pcdata->perm_hit = ch->max_hit;
      ch->pcdata->perm_mana = ch->max_mana;
      ch->pcdata->perm_move = ch->max_move;
      ch->pcdata->last_level = ch->pcdata->played / HOUR;
      if (ch->pcdata->true_sex < SEX_NEUTRAL
	  || ch->pcdata->true_sex > SEX_FEMALE)
	{
	  if (ch->sex > SEX_NEUTRAL && ch->sex < SEX_RANDOM)
	    ch->pcdata->true_sex = ch->sex;
	  else
	    ch->pcdata->true_sex = SEX_NEUTRAL;
	}

    }


  for (stat = 0; stat < STAT_MAX; stat++)
    ch->mod_stat[stat] = 0;

  if (ch->pcdata->true_sex < SEX_NEUTRAL || ch->pcdata->true_sex > SEX_FEMALE)
    ch->pcdata->true_sex = SEX_NEUTRAL;
  ch->sex = ch->pcdata->true_sex;
  ch->max_hit = ch->pcdata->perm_hit;
  ch->max_mana = ch->pcdata->perm_mana;
  ch->max_move = ch->pcdata->perm_move;

  for (i = 0; i < MAX_AC; i++)
    ch->armor[i] = 100;

  ch->hitroll = 0;
  ch->damroll = 0;
  ch->saving_throw = 0;


  for (loc = 0; loc < MAX_WEAR; loc++)
    {
      obj = get_eq_char (ch, (wloc_t) loc);
      if (obj == NULL)
	continue;
      for (i = 0; i < MAX_AC; i++)
	ch->armor[i] -= apply_ac (obj, (wloc_t) loc, i);

      if (!obj->enchanted)
	for (af = obj->pIndexData->affect_first; af != NULL; af = af->next)
	  {
	    mod = af->modifier;
	    switch (af->location)
	      {
	      case APPLY_STR:
		ch->mod_stat[STAT_STR] += mod;
		break;
	      case APPLY_DEX:
		ch->mod_stat[STAT_DEX] += mod;
		break;
	      case APPLY_INT:
		ch->mod_stat[STAT_INT] += mod;
		break;
	      case APPLY_WIS:
		ch->mod_stat[STAT_WIS] += mod;
		break;
	      case APPLY_CON:
		ch->mod_stat[STAT_CON] += mod;
		break;

	      case APPLY_SEX:
		ch->sex = (sex_t) ((int) ch->sex + mod);
		break;
	      case APPLY_MANA:
		ch->max_mana += mod;
		break;
	      case APPLY_HIT:
		ch->max_hit += mod;
		break;
	      case APPLY_MOVE:
		ch->max_move += mod;
		break;

	      case APPLY_AC:
		for (i = 0; i < MAX_AC; i++)
		  ch->armor[i] += mod;
		break;
	      case APPLY_HITROLL:
		ch->hitroll += mod;
		break;
	      case APPLY_DAMROLL:
		ch->damroll += mod;
		break;

	      case APPLY_SAVES:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_ROD:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_PETRI:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_BREATH:
		ch->saving_throw += mod;
		break;
	      case APPLY_SAVING_SPELL:
		ch->saving_throw += mod;
		break;
	      default:
		break;
	      }
	  }

      for (af = obj->affect_first; af != NULL; af = af->next)
	{
	  mod = af->modifier;
	  switch (af->location)
	    {
	    case APPLY_STR:
	      ch->mod_stat[STAT_STR] += mod;
	      break;
	    case APPLY_DEX:
	      ch->mod_stat[STAT_DEX] += mod;
	      break;
	    case APPLY_INT:
	      ch->mod_stat[STAT_INT] += mod;
	      break;
	    case APPLY_WIS:
	      ch->mod_stat[STAT_WIS] += mod;
	      break;
	    case APPLY_CON:
	      ch->mod_stat[STAT_CON] += mod;
	      break;

	    case APPLY_SEX:
	      ch->sex = (sex_t) ((int) ch->sex + mod);
	      break;
	    case APPLY_MANA:
	      ch->max_mana += mod;
	      break;
	    case APPLY_HIT:
	      ch->max_hit += mod;
	      break;
	    case APPLY_MOVE:
	      ch->max_move += mod;
	      break;

	    case APPLY_AC:
	      for (i = 0; i < MAX_AC; i++)
		ch->armor[i] += mod;
	      break;
	    case APPLY_HITROLL:
	      ch->hitroll += mod;
	      break;
	    case APPLY_DAMROLL:
	      ch->damroll += mod;
	      break;

	    case APPLY_SAVES:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_ROD:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_PETRI:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_BREATH:
	      ch->saving_throw += mod;
	      break;
	    case APPLY_SAVING_SPELL:
	      ch->saving_throw += mod;
	      break;
	    default:
	      break;
	    }
	}
    }


  for (af = ch->affect_first; af != NULL; af = af->next)
    {
      mod = af->modifier;
      switch (af->location)
	{
	case APPLY_STR:
	  ch->mod_stat[STAT_STR] += mod;
	  break;
	case APPLY_DEX:
	  ch->mod_stat[STAT_DEX] += mod;
	  break;
	case APPLY_INT:
	  ch->mod_stat[STAT_INT] += mod;
	  break;
	case APPLY_WIS:
	  ch->mod_stat[STAT_WIS] += mod;
	  break;
	case APPLY_CON:
	  ch->mod_stat[STAT_CON] += mod;
	  break;

	case APPLY_SEX:
	  ch->sex = (sex_t) ((int) ch->sex + mod);
	  break;
	case APPLY_MANA:
	  ch->max_mana += mod;
	  break;
	case APPLY_HIT:
	  ch->max_hit += mod;
	  break;
	case APPLY_MOVE:
	  ch->max_move += mod;
	  break;

	case APPLY_AC:
	  for (i = 0; i < MAX_AC; i++)
	    ch->armor[i] += mod;
	  break;
	case APPLY_HITROLL:
	  ch->hitroll += mod;
	  break;
	case APPLY_DAMROLL:
	  ch->damroll += mod;
	  break;

	case APPLY_SAVES:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_ROD:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_PETRI:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_BREATH:
	  ch->saving_throw += mod;
	  break;
	case APPLY_SAVING_SPELL:
	  ch->saving_throw += mod;
	  break;
	default:
	  break;
	}
    }


  if (ch->sex < SEX_NEUTRAL || ch->sex > SEX_FEMALE)
    ch->sex = ch->pcdata->true_sex;
}


int
get_trust (CharData * ch)
{
  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;

  if (ch->trust)
    return ch->trust;

  if (IsNPC (ch) && ch->level >= MAX_MORTAL_LEVEL)
    return MAX_MORTAL_LEVEL - 1;
  else
    return ch->level;
}


int
get_age (CharData * ch)
{
  return 17 + (ch->pcdata->played +
	       (int) (current_time - ch->logon)) / (20 * HOUR);
}


int
get_curr_stat (CharData * ch, int stat)
{
  int max;

  if (IsNPC (ch) || ch->level > LEVEL_IMMORTAL)
    max = MAX_STATS;

  else
    {
      max = ch->race->max_stats[stat] + 4;

      if (is_prime_stat (ch, stat))
	max += 2;

      if (ch->race == race_lookup ("human"))
	max += 1;

      max = Min (max, MAX_STATS);
    }

  return Range (3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}


int
get_max_train (CharData * ch, int stat)
{
  int max;

  if (IsNPC (ch) || ch->level > LEVEL_IMMORTAL)
    return MAX_STATS;

  max = ch->race->max_stats[stat];
  if (is_prime_stat (ch, stat))
    {
      if (ch->race == race_lookup ("human"))
	max += 3;
      else
	max += 2;
    }
  return Min (max, MAX_STATS);
}


int
can_carry_n (CharData * ch)
{
  if (!IsNPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return 10000;

  if (IsNPC (ch) && IsSet (ch->act, ACT_PET))
    return 100;

  return MAX_WEAR + 2 * get_curr_stat (ch, STAT_DEX) + ch->level;
}


int
can_carry_w (CharData * ch)
{
  if (!IsNPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return 10000000;

  if (IsNPC (ch) && IsSet (ch->act, ACT_PET))
    return 1000;

  return str_app[get_curr_stat (ch, STAT_STR)].carry * 10 + ch->level * 25;
}



bool
is_name (const char *str, const char *namelist)
{
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  const char *list, *string;


  if (NullStr (namelist))
    return false;


  if (NullStr (str))
    return false;

  string = str;

  for (;;)
    {
      str = one_argument (str, part);

      if (NullStr (part))
	return true;


      list = namelist;
      for (;;)
	{
	  list = one_argument (list, name);
	  if (NullStr (name))
	    return false;

	  if (!str_prefix (string, name))
	    return true;

	  if (!str_prefix (part, name))
	    break;
	}
    }
}

bool
is_exact_name (const char *str, const char *namelist)
{
  char name[MAX_INPUT_LENGTH];

  if (namelist == NULL)
    return false;

  for (;;)
    {
      namelist = one_argument (namelist, name);
      if (NullStr (name))
	return false;
      if (!str_cmp (str, name))
	return true;
    }
}


void
affect_enchant (ObjData * obj)
{

  if (!obj->enchanted)
    {
      AffectData *paf, *af_new;

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
}


void
affect_modify (CharData * ch, AffectData * paf, bool fAdd)
{
  ObjData *wield;
  int mod, i;

  mod = paf->modifier;

  if (fAdd)
    {
      switch (paf->where)
	{
	case TO_AFFECTS:
	  SetBit (ch->affected_by, paf->bitvector);
	  break;
	case TO_IMMUNE:
	  SetBit (ch->imm_flags, paf->bitvector);
	  break;
	case TO_RESIST:
	  SetBit (ch->res_flags, paf->bitvector);
	  break;
	case TO_VULN:
	  SetBit (ch->vuln_flags, paf->bitvector);
	  break;
	default:
	  break;
	}
    }
  else
    {
      switch (paf->where)
	{
	case TO_AFFECTS:
	  RemBit (ch->affected_by, paf->bitvector);
	  break;
	case TO_IMMUNE:
	  RemBit (ch->imm_flags, paf->bitvector);
	  break;
	case TO_RESIST:
	  RemBit (ch->res_flags, paf->bitvector);
	  break;
	case TO_VULN:
	  RemBit (ch->vuln_flags, paf->bitvector);
	  break;
	default:
	  break;
	}
      mod = 0 - mod;
    }

  switch (paf->location)
    {
    default:
      bugf ("Affect_modify: unknown location %d.", paf->location);
      return;

    case APPLY_NONE:
      break;
    case APPLY_STR:
      ch->mod_stat[STAT_STR] += mod;
      break;
    case APPLY_DEX:
      ch->mod_stat[STAT_DEX] += mod;
      break;
    case APPLY_INT:
      ch->mod_stat[STAT_INT] += mod;
      break;
    case APPLY_WIS:
      ch->mod_stat[STAT_WIS] += mod;
      break;
    case APPLY_CON:
      ch->mod_stat[STAT_CON] += mod;
      break;
    case APPLY_SEX:
      ch->sex = (sex_t) ((int) ch->sex + mod);
      break;
    case APPLY_CLASS:
      break;
    case APPLY_LEVEL:
      break;
    case APPLY_AGE:
      break;
    case APPLY_HEIGHT:
      break;
    case APPLY_WEIGHT:
      break;
    case APPLY_MANA:
      ch->max_mana += mod;
      break;
    case APPLY_HIT:
      ch->max_hit += mod;
      break;
    case APPLY_MOVE:
      ch->max_move += mod;
      break;
    case APPLY_GOLD:
      break;
    case APPLY_EXP:
      break;
    case APPLY_AC:
      for (i = 0; i < MAX_AC; i++)
	ch->armor[i] += mod;
      break;
    case APPLY_HITROLL:
      ch->hitroll += mod;
      break;
    case APPLY_DAMROLL:
      ch->damroll += mod;
      break;
    case APPLY_SAVES:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_ROD:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_PETRI:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_BREATH:
      ch->saving_throw += mod;
      break;
    case APPLY_SAVING_SPELL:
      ch->saving_throw += mod;
      break;
    case APPLY_SPELL_AFFECT:
      break;
    }


  if (!IsNPC (ch) && (wield = get_eq_char (ch, WEAR_WIELD)) != NULL
      && get_obj_weight (wield) >
      (str_app[get_curr_stat (ch, STAT_STR)].wield * 10))
    {
      static int depth;

      if (depth == 0)
	{
	  depth++;
	  act ("You drop $p.", ch, wield, NULL, TO_CHAR);
	  act ("$n drops $p.", ch, wield, NULL, TO_ROOM);
	  obj_from_char (wield);
	  obj_to_room (wield, ch->in_room);
	  depth--;
	}
    }

  return;
}


AffectData *
affect_find (AffectData * paf, int sn)
{
  AffectData *paf_find;

  for (paf_find = paf; paf_find != NULL; paf_find = paf_find->next)
    {
      if (paf_find->type == sn)
	return paf_find;
    }

  return NULL;
}


void
affect_check (CharData * ch, where_t where, flag_t vector)
{
  AffectData *paf;
  ObjData *obj;

  if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
    return;

  for (paf = ch->affect_first; paf != NULL; paf = paf->next)
    if (paf->where == where && paf->bitvector == vector)
      {
	switch (where)
	  {
	  case TO_AFFECTS:
	    SetBit (ch->affected_by, vector);
	    break;
	  case TO_IMMUNE:
	    SetBit (ch->imm_flags, vector);
	    break;
	  case TO_RESIST:
	    SetBit (ch->res_flags, vector);
	    break;
	  case TO_VULN:
	    SetBit (ch->vuln_flags, vector);
	    break;
	  default:
	    break;
	  }
	return;
      }

  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == -1)
	continue;

      for (paf = obj->affect_first; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	  {
	    switch (where)
	      {
	      case TO_AFFECTS:
		SetBit (ch->affected_by, vector);
		break;
	      case TO_IMMUNE:
		SetBit (ch->imm_flags, vector);
		break;
	      case TO_RESIST:
		SetBit (ch->res_flags, vector);
		break;
	      case TO_VULN:
		SetBit (ch->vuln_flags, vector);
	      default:
		break;
	      }
	    return;
	  }

      if (obj->enchanted)
	continue;

      for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
	if (paf->where == where && paf->bitvector == vector)
	  {
	    switch (where)
	      {
	      case TO_AFFECTS:
		SetBit (ch->affected_by, vector);
		break;
	      case TO_IMMUNE:
		SetBit (ch->imm_flags, vector);
		break;
	      case TO_RESIST:
		SetBit (ch->res_flags, vector);
		break;
	      case TO_VULN:
		SetBit (ch->vuln_flags, vector);
		break;
	      default:
		break;
	      }
	    return;
	  }
    }
}


void
affect_to_char (CharData * ch, AffectData * paf)
{
  AffectData *paf_new;

  paf_new = new_affect ();

  *paf_new = *paf;

  Validate (paf_new);
  Link (paf_new, ch->affect, next, prev);

  affect_modify (ch, paf_new, true);
  return;
}


void
affect_to_obj (ObjData * obj, AffectData * paf)
{
  AffectData *paf_new;

  paf_new = new_affect ();

  *paf_new = *paf;

  Validate (paf_new);
  Link (paf_new, obj->affect, next, prev);


  if (paf->bitvector)
    switch (paf->where)
      {
      case TO_OBJECT:
	SetBit (obj->extra_flags, paf->bitvector);
	break;
      case TO_WEAPON:
	if (obj->item_type == ITEM_WEAPON)
	  SetBit (obj->value[4], paf->bitvector);
	break;
      default:
	break;
      }

  return;
}


void
affect_remove (CharData * ch, AffectData * paf)
{
  where_t where;
  flag_t vector;

  if (ch->affect_first == NULL)
    {
      bug ("Affect_remove: no affect.");
      return;
    }

  affect_modify (ch, paf, false);
  where = paf->where;
  vector = paf->bitvector;

  UnLink (paf, ch->affect, next, prev);

  free_affect (paf);

  affect_check (ch, where, vector);
  return;
}

void
affect_remove_obj (ObjData * obj, AffectData * paf)
{
  where_t where;
  flag_t vector;

  if (obj->affect_first == NULL)
    {
      bug ("Affect_remove_object: no affect.");
      return;
    }

  if (obj->carried_by != NULL && obj->wear_loc != -1)
    affect_modify (obj->carried_by, paf, false);

  where = paf->where;
  vector = paf->bitvector;


  if (paf->bitvector)
    switch (paf->where)
      {
      case TO_OBJECT:
	RemBit (obj->extra_flags, paf->bitvector);
	break;
      case TO_WEAPON:
	if (obj->item_type == ITEM_WEAPON)
	  RemBit (obj->value[4], paf->bitvector);
	break;
      default:
	break;
      }

  UnLink (paf, obj->affect, next, prev);

  free_affect (paf);

  if (obj->carried_by != NULL && obj->wear_loc != -1)
    affect_check (obj->carried_by, where, vector);
  return;
}


void
affect_strip (CharData * ch, int sn)
{
  AffectData *paf;
  AffectData *paf_next;

  for (paf = ch->affect_first; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      if (paf->type == sn)
	affect_remove (ch, paf);
    }

  return;
}


bool
is_affected (CharData * ch, int sn)
{
  AffectData *paf;

  for (paf = ch->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->type == sn)
	return true;
    }

  return false;
}


void
affect_join (CharData * ch, AffectData * paf)
{
  AffectData *paf_old;
  bool found;

  found = false;
  for (paf_old = ch->affect_first; paf_old != NULL; paf_old = paf_old->next)
    {
      if (paf_old->type == paf->type)
	{
	  paf->level = (paf->level += paf_old->level) / 2;
	  paf->duration += paf_old->duration;
	  paf->modifier += paf_old->modifier;
	  affect_remove (ch, paf_old);
	  break;
	}
    }

  affect_to_char (ch, paf);
  return;
}


void
char_from_room (CharData * ch)
{
  ObjData *obj;

  if (ch->in_room == NULL)
    {
      bug ("Char_from_room: NULL.");
      return;
    }

  if (!IsNPC (ch))
    --ch->in_room->area->nplayer;

  if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL &&
      obj->item_type == ITEM_LIGHT && obj->value[2] != 0 &&
      ch->in_room->light > 0)
    --ch->in_room->light;

  UnLink (ch, ch->in_room->person, next_in_room, prev_in_room);

  ch->in_room = NULL;
  ch->next_in_room = NULL;
  ch->on = NULL;
  return;
}


void
char_to_room (CharData * ch, RoomIndex * pRoomIndex)
{
  ObjData *obj;

  if (pRoomIndex == NULL)
    {
      RoomIndex *room;

      bug ("Char_to_room: NULL.");

      if ((room = get_room_index (ROOM_VNUM_TEMPLE)) != NULL)
	char_to_room (ch, room);

      return;
    }

  ch->in_room = pRoomIndex;
  Link (ch, pRoomIndex->person, next_in_room, prev_in_room);

  if (!IsNPC (ch))
    {
      if (ch->in_room->area->empty)
	{
	  ch->in_room->area->empty = false;
	  ch->in_room->area->age = 0;
	}
      ++ch->in_room->area->nplayer;
      if (!IsSet (ch->in_room->room_flags, ROOM_NOEXPLORE))
	StrSetBit (ch->pcdata->explored, ch->in_room->vnum);
      portal_map (ch, ch->in_room);
      if (ch->in_room->sound != NULL)
	act_sound (ch->in_room->sound, ch, NULL, ch->in_room->sound->to,
		   POS_RESTING);
    }

  if ((obj = get_eq_char (ch, WEAR_LIGHT)) != NULL &&
      obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    ++ch->in_room->light;

  if (IsAffected (ch, AFF_PLAGUE))
    {
      AffectData *af, plague;
      CharData *vch;

      for (af = ch->affect_first; af != NULL; af = af->next)
	{
	  if (af->type == gsn_plague)
	    break;
	}

      if (af == NULL)
	{
	  RemBit (ch->affected_by, AFF_PLAGUE);
	  return;
	}

      if (af->level == 1)
	return;

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
	  if (!saves_spell (plague.level - 2, vch, DAM_DISEASE) &&
	      !IsImmortal (vch) && !IsAffected (vch, AFF_PLAGUE)
	      && number_bits (6) == 0)
	    {
	      chprintln (vch, "You feel hot and feverish.");
	      act ("$n shivers and looks very ill.", vch, NULL, NULL,
		   TO_ROOM);
	      affect_join (vch, &plague);
	    }
	}
    }

  return;
}

void
link_obj_to_char (CharData * ch, ObjData * obj)
{
  ObjData *otmp;

  if (!IsNPC (ch) || !ch->pIndexData->pShop)
    {
      Link (obj, ch->carrying, next_content, prev_content);
      return;
    }

  for (otmp = ch->carrying_first; otmp; otmp = otmp->next_content)
    {
      if (obj->pIndexData == otmp->pIndexData &&
	  !str_cmp (obj->short_descr, otmp->short_descr))
	{

	  if (IsObjStat (otmp, ITEM_INVENTORY))
	    {
	      extract_obj (obj);
	      return;
	    }
	  obj->cost = otmp->cost;
	}
      if (obj->level > otmp->level)
	{
	  Insert (obj, otmp, ch->carrying, next_content, prev_content);
	  break;
	}
      else if (obj->level == otmp->level
	       && !str_cmp (obj->short_descr, otmp->short_descr))
	{
	  Insert (obj, otmp, ch->carrying, next_content, prev_content);
	  break;
	}
    }

  if (!otmp)
    Link (obj, ch->carrying, next_content, prev_content);
}


void
obj_to_char (ObjData * obj, CharData * ch)
{
  link_obj_to_char (ch, obj);
  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;
  ch->carry_number += get_obj_number (obj);
  ch->carry_weight += get_obj_weight (obj);
  if (!IsNPC (ch) && IsObjStat (obj, ITEM_QUEST))
    update_questobj (ch, obj);
  if (obj->item_type == ITEM_CORPSE_PC)
    update_corpses (obj, false);
}


void
obj_from_char (ObjData * obj)
{
  CharData *ch;

  if ((ch = obj->carried_by) == NULL)
    {
      bug ("Obj_from_char: null ch.");
      return;
    }

  if (obj->wear_loc != WEAR_NONE)
    unequip_char (ch, obj);

  UnLink (obj, ch->carrying, next_content, prev_content);

  obj->carried_by = NULL;
  obj->next_content = NULL;
  ch->carry_number -= get_obj_number (obj);
  ch->carry_weight -= get_obj_weight (obj);
  return;
}


int
apply_ac (ObjData * obj, wloc_t iWear, int type)
{
  if (obj->item_type != ITEM_ARMOR)
    return 0;

  switch (iWear)
    {
    case WEAR_BODY:
    case WEAR_HEAD:
    case WEAR_LEGS:
    case WEAR_FEET:
    case WEAR_HANDS:
    case WEAR_ARMS:
    case WEAR_SHIELD:
    case WEAR_NECK_1:
    case WEAR_NECK_2:
    case WEAR_ABOUT:
    case WEAR_WAIST:
    case WEAR_WRIST_L:
    case WEAR_WRIST_R:
    case WEAR_HOLD:
      return obj->value[type];
    default:
      break;
    }

  return 0;
}


ObjData *
get_eq_char (CharData * ch, wloc_t iWear)
{
  ObjData *obj;

  if (ch == NULL)
    return NULL;

  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == iWear)
	return obj;
    }

  return NULL;
}


void
equip_char (CharData * ch, ObjData * obj, wloc_t iWear)
{
  AffectData *paf;
  int i;

  if (get_eq_char (ch, iWear) != NULL)
    {
      bugf ("Equip_char: already equipped (%d).", iWear);
      return;
    }

  if ((IsObjStat (obj, ITEM_ANTI_EVIL) && IsEvil (ch)) ||
      (IsObjStat (obj, ITEM_ANTI_GOOD) && IsGood (ch)) ||
      (IsObjStat (obj, ITEM_ANTI_NEUTRAL) && IsNeutral (ch)))
    {

      act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
      act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
      obj_from_char (obj);
      obj_to_room (obj, ch->in_room);
      return;
    }

  for (i = 0; i < MAX_AC; i++)
    ch->armor[i] -= apply_ac (obj, iWear, i);
  obj->wear_loc = iWear;

  if (!obj->enchanted)
    for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
      if (paf->location != APPLY_SPELL_AFFECT)
	affect_modify (ch, paf, true);
  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      affect_to_char (ch, paf);
    else
      affect_modify (ch, paf, true);

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 &&
      ch->in_room != NULL)
    ++ch->in_room->light;

  return;
}


void
unequip_char (CharData * ch, ObjData * obj)
{
  AffectData *paf = NULL;
  AffectData *lpaf = NULL;
  AffectData *lpaf_next = NULL;
  int i;

  if (obj->wear_loc == WEAR_NONE)
    {
      bug ("Unequip_char: already unequipped.");
      return;
    }

  for (i = 0; i < MAX_AC; i++)
    ch->armor[i] += apply_ac (obj, obj->wear_loc, i);
  obj->wear_loc = WEAR_NONE;

  if (!obj->enchanted)
    {
      for (paf = obj->pIndexData->affect_first; paf != NULL; paf = paf->next)
	if (paf->location == APPLY_SPELL_AFFECT)
	  {
	    for (lpaf = ch->affect_first; lpaf != NULL; lpaf = lpaf_next)
	      {
		lpaf_next = lpaf->next;
		if ((lpaf->type == paf->type) &&
		    (lpaf->level == paf->level) &&
		    (lpaf->location == APPLY_SPELL_AFFECT))
		  {
		    affect_remove (ch, lpaf);
		    lpaf_next = NULL;
		  }
	      }
	  }
	else
	  {
	    affect_modify (ch, paf, false);
	    affect_check (ch, paf->where, paf->bitvector);
	  }
    }
  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      {
	bug ("Norm-Apply: %d");
	for (lpaf = ch->affect_first; lpaf != NULL; lpaf = lpaf_next)
	  {
	    lpaf_next = lpaf->next;
	    if ((lpaf->type == paf->type) &&
		(lpaf->level == paf->level) &&
		(lpaf->location == APPLY_SPELL_AFFECT))
	      {
		bugf ("location = %d", lpaf->location);
		bugf ("type = %d", lpaf->type);
		affect_remove (ch, lpaf);
		lpaf_next = NULL;
	      }
	  }
      }
    else
      {
	affect_modify (ch, paf, false);
	affect_check (ch, paf->where, paf->bitvector);
      }

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 &&
      ch->in_room != NULL && ch->in_room->light > 0)
    --ch->in_room->light;

  return;
}


int
count_obj_list (ObjIndex * pObjIndex, ObjData * list)
{
  ObjData *obj;
  int nMatch;

  nMatch = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (obj->pIndexData == pObjIndex)
	nMatch++;
    }

  return nMatch;
}


void
obj_from_room (ObjData * obj)
{
  RoomIndex *in_room;
  CharData *ch;

  if ((in_room = obj->in_room) == NULL)
    {
      bug ("obj_from_room: NULL.");
      return;
    }

  for (ch = in_room->person_first; ch != NULL; ch = ch->next_in_room)
    if (ch->on == obj)
      ch->on = NULL;

  UnLink (obj, in_room->content, next_content, prev_content);

  obj->in_room = NULL;
  obj->next_content = NULL;
  return;
}


void
obj_to_room (ObjData * obj, RoomIndex * pRoomIndex)
{
  if (IsObjStat (obj, ITEM_AUCTIONED))
    return;
  Link (obj, pRoomIndex->content, next_content, prev_content);
  obj->in_room = pRoomIndex;
  obj->carried_by = NULL;
  obj->in_obj = NULL;
  if (is_donate_room (pRoomIndex->vnum))
    obj->cost = 0;
  if (obj->item_type == ITEM_CORPSE_PC)
    update_corpses (obj, false);
  return;
}


void
obj_to_obj (ObjData * obj, ObjData * obj_to)
{

  if (IsObjStat (obj, ITEM_AUCTIONED))
    return;

  Link (obj, obj_to->content, next_content, prev_content);
  obj->in_obj = obj_to;
  obj->in_room = NULL;
  obj->carried_by = NULL;
  if (obj_to->item_type == ITEM_CORPSE_PC)
    update_corpses (obj_to, false);
  if (obj->item_type == ITEM_CORPSE_PC)
    update_corpses (obj, false);

  for (; obj_to != NULL; obj_to = obj_to->in_obj)
    {
      if (obj_to->carried_by != NULL)
	{
	  obj_to->carried_by->carry_number += get_obj_number (obj);
	  obj_to->carried_by->carry_weight +=
	    get_obj_weight (obj) * WeightMult (obj_to) / 100;
	}
    }

  return;
}


void
obj_from_obj (ObjData * obj)
{
  ObjData *obj_from;

  if ((obj_from = obj->in_obj) == NULL)
    {
      bug ("Obj_from_obj: null obj_from.");
      return;
    }

  UnLink (obj, obj_from->content, next_content, prev_content);

  obj->next_content = NULL;
  obj->in_obj = NULL;
  if (obj_from->item_type == ITEM_CORPSE_PC)
    update_corpses (obj_from, false);

  for (; obj_from != NULL; obj_from = obj_from->in_obj)
    {
      if (obj_from->carried_by != NULL)
	{
	  obj_from->carried_by->carry_number -= get_obj_number (obj);
	  obj_from->carried_by->carry_weight -=
	    get_obj_weight (obj) * WeightMult (obj_from) / 100;
	}
    }
  return;
}


void
extract_obj (ObjData * obj)
{
  ObjData *obj_content;
  ObjData *obj_next;

  if (obj->in_room != NULL)
    obj_from_room (obj);
  else if (obj->carried_by != NULL)
    obj_from_char (obj);
  else if (obj->in_obj != NULL)
    obj_from_obj (obj);

  if (IsObjStat (obj, ITEM_AUCTIONED))
    {
      AuctionData *auc, *auc_next;

      for (auc = auction_first; auc; auc = auc_next)
	{
	  auc_next = auc->next;
	  if (auc->item == obj)
	    reset_auc (auc, true);
	}
    }

  for (obj_content = obj->content_first; obj_content; obj_content = obj_next)
    {
      obj_next = obj_content->next_content;
      extract_obj (obj_content);
    }

  UnLink (obj, obj, next, prev);

  if (obj->item_type == ITEM_CORPSE_PC)
    update_corpses (obj, true);

  --obj->pIndexData->count;
  free_obj (obj);
  return;
}


void
extract_char (CharData * ch, bool fPull)
{
  CharData *wch;
  ObjData *obj;
  ObjData *obj_next;



  nuke_pets (ch);
  ch->pet = NULL;

  if (fPull)
    {
      if (InWar (ch))
	return;

      if (has_auction (ch))
	{
	  bugf ("%s has stake in an auction!", ch->name);
	  extract_auc (ch);
	}

      die_follower (ch);

      update_statlist (ch, false);
      if (is_clan (ch))
	update_members (ch, false);
      if (IsImmortal (ch))
	update_wizlist (ch, ch->level);
    }

  stop_fighting (ch, true);

  for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (fPull || !IsObjStat (obj, ITEM_QUEST | ITEM_AUCTIONED))
	extract_obj (obj);
    }

  if (ch->in_room != NULL)
    char_from_room (ch);


  if (!fPull)
    {
      RoomIndex *room = NULL;

      if (is_clan (ch))
	room = get_room_index (CharClan (ch)->rooms[CLAN_ROOM_ENTRANCE]);
      if (room == NULL)
	room = get_room_index (ROOM_VNUM_ALTAR);
      char_to_room (ch, room);
      return;
    }

  if (IsNPC (ch))
    --ch->pIndexData->count;

  if (ch->desc != NULL && ch->desc->original != NULL)
    {
      do_function (ch, &do_return, "");
      ch->desc = NULL;
    }

  for (wch = char_first; wch != NULL; wch = wch->next)
    {
      if (wch->reply == ch)
	wch->reply = NULL;
      if (ch->mprog_target == wch)
	wch->mprog_target = NULL;
    }

  UnLink (ch, char, next, prev);

  if (!IsNPC (ch))
    UnLink (ch, player, next_player, prev_player);

  if (ch->desc != NULL)
    ch->desc->character = NULL;
  free_char (ch);

  return;
}


CharData *
get_char_room (CharData * ch, RoomIndex * room, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *rch;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  if (!str_cmp (arg, "self"))
    return ch;

  if (ch && room)
    {
      bug ("get_char_room received multiple types (ch/room)");
      return NULL;
    }

  if (ch)
    rch = ch->in_room->person_first;
  else
    rch = room->person_first;

  for (; rch != NULL; rch = rch->next_in_room)
    {
      if ((ch && !can_see (ch, rch)) || !is_name (arg, rch->name))
	continue;
      if (++count == number)
	return rch;
    }

  return NULL;
}


CharData *
get_char_world (CharData * ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *wch;
  int number;
  int count;

  if (ch && (wch = get_char_room (ch, NULL, argument)) != NULL)
    return wch;

  number = number_argument (argument, arg);
  count = 0;
  for (wch = char_first; wch != NULL; wch = wch->next)
    {
      if (wch->in_room == NULL || (ch && !can_see (ch, wch))
	  || !is_name (arg, wch->name))
	continue;
      if (++count == number)
	return wch;
    }

  return NULL;
}

CharData *
get_char_vnum (vnum_t vnum)
{
  CharData *wch;

  if (vnum <= 0)
    return NULL;

  for (wch = char_first; wch != NULL; wch = wch->next)
    {
      if (wch->in_room == NULL)
	continue;
      if (vnum == IsNPC (wch) ? wch->pIndexData->vnum : wch->id)
	return wch;
    }

  return NULL;
}


CharData *
get_pc_world (CharData * ch, const char *argument)
{
  CharData *wch;

  if (NullStr (argument))
    return NULL;

  if (ch && !str_cmp (argument, "self"))
    return ch;

  for (wch = player_first; wch != NULL; wch = wch->next_player)
    {
      if (ch && !can_see (ch, wch))
	continue;
      if (wch->in_room != NULL && is_name (argument, wch->name))
	return wch;
    }

  return NULL;
}


ObjData *
get_obj_type (ObjIndex * pObjIndex)
{
  ObjData *obj;

  for (obj = obj_first; obj != NULL; obj = obj->next)
    {
      if (obj->pIndexData == pObjIndex)
	return obj;
    }

  return NULL;
}


ObjData *
get_obj_list (CharData * ch, const char *argument, ObjData * list)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }

  return NULL;
}


ObjData *
get_obj_carry (CharData * ch, const char *argument, CharData * viewer)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE
	  && (viewer ? can_see_obj (viewer, obj) : true)
	  && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }

  return NULL;
}


ObjData *
get_obj_wear (CharData * ch, const char *argument, bool character)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE
	  && (character ? can_see_obj (ch, obj) : true)
	  && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;
	}
    }

  return NULL;
}


ObjData *
get_obj_here (CharData * ch, RoomIndex * room, const char *argument)
{
  ObjData *obj;
  int number, count;
  char arg[MAX_INPUT_LENGTH];

  if (ch && room)
    {
      bug ("get_obj_here received a ch and a room");
      return NULL;
    }

  number = number_argument (argument, arg);
  count = 0;

  if (ch)
    {
      obj = get_obj_list (ch, argument, ch->in_room->content_first);
      if (obj != NULL)
	return obj;

      if ((obj = get_obj_carry (ch, argument, ch)) != NULL)
	return obj;

      if ((obj = get_obj_wear (ch, argument, true)) != NULL)
	return obj;
    }
  else
    {
      for (obj = room->content_first; obj; obj = obj->next_content)
	{
	  if (!is_name (arg, obj->name))
	    continue;
	  if (++count == number)
	    return obj;
	}
    }

  return NULL;
}


ObjData *
get_obj_world (CharData * ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;
  int number;
  int count;

  if (ch && (obj = get_obj_here (ch, NULL, argument)) != NULL)
    return obj;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = obj_first; obj != NULL; obj = obj->next)
    {
      if ((ch && !can_see_obj (ch, obj)) || !is_name (arg, obj->name))
	continue;
      if (++count == number)
	return obj;
    }

  return NULL;
}



void
deduct_cost (CharData * ch, money_t cost, currency_t type)
{
  while (ch->silver >= 100)
    {
      ch->gold++;
      ch->silver -= 100;
    }

  switch (type)
    {
    case VALUE_GOLD:
      {
	ch->gold -= cost;
	if (ch->gold < 0)
	  {
	    bugf ("deduct_cost: gold %ld < 0", ch->gold);
	    ch->gold = 0;
	  }
      }
      break;

    case VALUE_SILVER:
      {
	while (ch->silver < cost)
	  {
	    ch->gold--;
	    ch->silver += 100;
	  }
	ch->silver -= cost;
	if (ch->gold < 0)
	  {
	    bugf ("deduct costs: gold %ld < 0", ch->gold);
	    ch->gold = 0;
	  }
	if (ch->silver < 0)
	  {
	    bugf ("deduct costs: silver %ld < 0", ch->silver);
	    ch->silver = 0;
	  }
      }
      break;
    }
}


void
add_cost (CharData * ch, money_t cost, currency_t value)
{
  while (ch->silver >= 100)
    {
      ch->gold++;
      ch->silver -= 100;
    }
  switch (value)
    {
    case VALUE_GOLD:
      {
	ch->gold += cost;
      }
      break;
    case VALUE_SILVER:
      {
	ch->silver += cost;
	while (ch->silver >= 100)
	  {
	    ch->gold++;
	    ch->silver -= 100;
	  }
      }
      break;
    }
  return;
}

char *
cost_str (money_t cost, currency_t value)
{
  switch (value)
    {
    case VALUE_GOLD:
      return FORMATF ("%ld gold", cost);
    default:
    case VALUE_SILVER:
      {
	if (cost > 100)
	  return FORMATF ("%ld gold, %ld silver", cost / 100,
			  cost - (cost / 100) * 100);
	else
	  return FORMATF ("%ld silver", cost);
      }
    }
}


ObjData *
create_money (money_t gold, money_t silver)
{
  char buf[MAX_STRING_LENGTH];
  ObjData *obj;

  if (gold < 0 || silver < 0 || (gold == 0 && silver == 0))
    {
      bug ("Create_money: zero or negative money.");
      gold = Max (1, gold);
      silver = Max (1, silver);
    }

  if (gold == 0 && silver == 1)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SILVER_ONE), 0);
    }
  else if (gold == 1 && silver == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_GOLD_ONE), 0);
    }
  else if (silver == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_GOLD_SOME), 0);
      sprintf (buf, obj->short_descr, gold);
      replace_str (&obj->short_descr, buf);
      obj->value[1] = gold;
      obj->cost = gold;
      obj->weight = gold / 5;
    }
  else if (gold == 0)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SILVER_SOME), 0);
      sprintf (buf, obj->short_descr, silver);
      replace_str (&obj->short_descr, buf);
      obj->value[0] = silver;
      obj->cost = silver;
      obj->weight = silver / 20;
    }
  else
    {
      obj = create_object (get_obj_index (OBJ_VNUM_COINS), 0);
      sprintf (buf, obj->short_descr, silver, gold);
      replace_str (&obj->short_descr, buf);
      obj->value[0] = silver;
      obj->value[1] = gold;
      obj->cost = 100 * gold + silver;
      obj->weight = gold / 5 + silver / 20;
    }

  return obj;
}


int
get_obj_number (ObjData * obj)
{
  int number;

  if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY ||
      obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
    number = 0;
  else
    number = 1;

  for (obj = obj->content_first; obj != NULL; obj = obj->next_content)
    number += get_obj_number (obj);

  return number;
}


int
get_obj_weight (ObjData * obj)
{
  int weight;
  ObjData *tobj;

  weight = obj->weight;
  for (tobj = obj->content_first; tobj != NULL; tobj = tobj->next_content)
    weight += get_obj_weight (tobj) * WeightMult (obj) / 100;

  return weight;
}

int
get_true_weight (ObjData * obj)
{
  int weight;

  weight = obj->weight;
  for (obj = obj->content_first; obj != NULL; obj = obj->next_content)
    weight += get_obj_weight (obj);

  return weight;
}


bool
room_is_dark (RoomIndex * pRoomIndex)
{
  if (pRoomIndex->light > 0)
    return false;

  if (IsSet (pRoomIndex->room_flags, ROOM_DARK))
    return true;

  if (pRoomIndex->sector_type == SECT_INSIDE ||
      pRoomIndex->sector_type == SECT_CITY)
    return false;

  if (time_info.sunlight == SUN_SET || time_info.sunlight == SUN_DARK)
    return true;

  return false;
}

bool
is_room_owner (CharData * ch, RoomIndex * room)
{
  if (NullStr (room->owner))
    return false;

  if (IsSet (room->area->area_flags, AREA_PLAYER_HOMES))
    return true;

  return is_name (ch->name, room->owner);
}


bool
room_is_private (RoomIndex * pRoomIndex)
{
  CharData *rch;
  int count;

  if (!NullStr (pRoomIndex->owner)
      && !IsSet (pRoomIndex->area->area_flags, AREA_PLAYER_HOMES))
    return true;

  count = 0;
  for (rch = pRoomIndex->person_first; rch != NULL; rch = rch->next_in_room)
    count++;

  if (IsSet (pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
    return true;

  if (IsSet (pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
    return true;

  if (IsSet (pRoomIndex->room_flags, ROOM_IMP_ONLY))
    return true;

  return false;
}


bool
can_see_room (CharData * ch, RoomIndex * pRoomIndex)
{
  if (IsSet (pRoomIndex->room_flags, ROOM_ARENA))
    return true;

  if (IsSet (pRoomIndex->room_flags, ROOM_IMP_ONLY) &&
      get_trust (ch) < MAX_LEVEL)
    return false;

  if (IsSet (pRoomIndex->room_flags, ROOM_GODS_ONLY) && !IsImmortal (ch))
    return false;

  if (IsSet (pRoomIndex->room_flags, ROOM_HEROES_ONLY) && !IsImmortal (ch))
    return false;

  if (IsSet (pRoomIndex->room_flags, ROOM_NEWBIES_ONLY) && ch->level > 5
      && !IsImmortal (ch))
    return false;

  if (!IsImmortal (ch) && pRoomIndex->area->clan != NULL
      && CharClan (ch) != pRoomIndex->area->clan)
    return false;

  if (is_home_owner (ch, pRoomIndex))
    return true;

  return true;
}


bool
can_see (CharData * ch, CharData * victim)
{

  if (ch == victim)
    return true;

  if (get_trust (ch) < victim->invis_level)
    return false;

  if (get_trust (ch) < victim->incog_level && ch->in_room != victim->in_room)
    return false;

  if ((!IsNPC (ch) && IsSet (ch->act, PLR_HOLYLIGHT)) ||
      (IsNPC (ch) && IsImmortal (ch)))
    return true;

  if (IsAffected (ch, AFF_BLIND))
    return false;

  if ((!IsNPC (ch) && ch->in_room &&
       IsSet (ch->in_room->room_flags, ROOM_ARENA)) && (!IsNPC (victim) &&
							victim->in_room
							&&
							IsSet
							(victim->
							 in_room->room_flags,
							 ROOM_ARENA)))
    return true;

  if (IsQuester (ch) && victim == ch->pcdata->quest.mob)
    return true;

  if (gquest_info.running == GQUEST_RUNNING && Gquester (ch) &&
      IsNPC (victim) && is_gqmob (ch->gquest, victim->pIndexData->vnum) != -1)
    return true;

  if (room_is_dark (ch->in_room) && !IsAffected (ch, AFF_INFRARED))
    return false;

  if (IsAffected (victim, AFF_INVISIBLE) &&
      !IsAffected (ch, AFF_DETECT_INVIS))
    return false;


  if (IsAffected (victim, AFF_SNEAK) &&
      !IsAffected (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    {
      int chance;

      chance = get_skill (victim, gsn_sneak);
      chance += get_curr_stat (victim, STAT_DEX) * 3 / 2;
      chance -= get_curr_stat (ch, STAT_INT) * 2;
      chance -= ch->level - victim->level * 3 / 2;

      if (number_percent () < chance)
	return false;
    }

  if (IsAffected (victim, AFF_HIDE) &&
      !IsAffected (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
    return false;

  return true;
}


bool
can_see_obj (CharData * ch, ObjData * obj)
{
  if (!IsNPC (ch) && IsSet (ch->act, PLR_HOLYLIGHT))
    return true;

  if (IsQuester (ch) && ch->pcdata->quest.obj == obj)
    return true;

  if (IsSet (obj->extra_flags, ITEM_VIS_DEATH))
    return false;

  if (IsAffected (ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
    return false;

  if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
    return true;

  if (IsSet (obj->extra_flags, ITEM_INVIS) &&
      !IsAffected (ch, AFF_DETECT_INVIS))
    return false;

  if (IsObjStat (obj, ITEM_GLOW))
    return true;

  if (room_is_dark (ch->in_room) && !IsAffected (ch, AFF_DARK_VISION))
    return false;

  return true;
}


bool
can_drop_obj (CharData * ch, ObjData * obj)
{

  if (IsSet (obj->extra_flags, ITEM_AUCTIONED))
    return false;

  if (!IsSet (obj->extra_flags, ITEM_NODROP))
    return true;

  if (!IsNPC (ch) && ch->level >= LEVEL_IMMORTAL)
    return true;

  return false;
}



bool
is_full_name (const char *str, const char *namelist)
{
  char name[MAX_INPUT_LENGTH];

  for (;;)
    {
      namelist = one_argument (namelist, name);
      if (NullStr (name))
	return false;
      if (!str_cmp (str, name))
	return true;
    }
}

bool
emptystring (const char *str)
{
  int i = 0;

  for (; str[i]; i++)
    if (str[i] != ' ')
      return false;

  return true;
}

char *
str_time (time_t timet, int tz, const char *format)
{
  static char buf_new[5][100];
  static int i;
  char *result;


  ++i;
  i %= 5;
  result = buf_new[i];

  if (timet <= 0)
    {
      timet = current_time;
    }
  if (tz > -1 && tz < MAX_TZONE)
    {
#ifdef __CYGWIN__
      timet += _timezone;
#elif defined HAVE_TIMEZONE_VAR || defined WIN32
      timet += timezone;
#elif defined HAVE_STRUCT_TM_TM_GMTOFF || define HAVE_TM_GMTOFF

      struct tm *ptime;

      ptime = localtime (&timet);
      timet += ptime->tm_gmtoff;
#else
#error "unable to find timezone variable!"
#endif

      timet += tzone_table[tz].gmt_offset;
    }
  strftime (result, 100,
	    !NullStr (format) ? format : "%a %b %d %I:%M:%S %p %Y",
	    localtime (&timet));

  return result;
}

int
calc_max_level (CharData * ch)
{
  if (IsNPC (ch) || IsImmortal (ch))
    return MAX_LEVEL;

  return Min (MAX_MORTAL_LEVEL, (LEVEL_HERO + ch->Class[CLASS_COUNT] - 1));
}

const char *
high_level_name (int level, bool fLong)
{
  static char buf[MSL];
  const char *imm_long[MAX_LEVEL - MAX_MORTAL_LEVEL] =
    { "AVATAR", "ANGEL", "DEMI", "IMMORTAL", "GOD", "DEITY", "SUPREME",
    "CREATOR", "IMPLEMENTOR"
  };
  const char *imm_short[MAX_LEVEL - MAX_MORTAL_LEVEL] =
    { "AVA", "ANG", "DEM", "IMM", "GOD", "DEI", "SUP", "CRE", "IMP"
  };

  if (level > MAX_MORTAL_LEVEL && level <= MAX_LEVEL)
    return fLong ? imm_long[level - MAX_MORTAL_LEVEL -
			    1] : imm_short[level - MAX_MORTAL_LEVEL - 1];

  if (level <= MAX_MORTAL_LEVEL && level > LEVEL_HERO)
    {
      sprintf (buf, "%s+%d", fLong ? "HERO" : "H", level - LEVEL_HERO);
      return buf;
    }

  if (level == LEVEL_HERO)
    {
      return fLong ? "HERO" : "HRO";
    }

  sprintf (buf, "%03d", level);

  return buf;
}

bool
file_exists (const char *path, ...)
{
  va_list args;
  char file[1024];
  struct stat buf;

  if (NullStr (path))
    return false;

  va_start (args, path);
  vsnprintf (file, sizeof (file), path, args);
  va_end (args);

  return (stat (file, &buf) != -1 && S_ISREG (buf.st_mode));
}

bool
exists_player (const char *name)
{
  if (NullStr (name))
    return false;

  return file_exists ("%s%s", pfilename (name, PFILE_NORMAL));
}

bool
is_player_name (const char *name)
{
  int i;
  const char *lwrname;

  if (NullStr (name))
    return false;

  lwrname = strip_color (name);

  for (i = 0; i < pfiles.count; i++)
    {
      if (!str_infix (pfiles.names[i], lwrname))
	return true;
    }
  return false;
}

bool
is_exact_player_name (const char *name)
{
  int i;

  if (NullStr (name))
    return false;

  for (i = 0; i < pfiles.count; i++)
    {
      if (!str_cmp (pfiles.names[i], name))
	return true;
    }
  return false;
}

const char *
get_player_name (const char *name)
{
  int i;
  const char *pname;

  if (NullStr (name))
    return "";

  pname = strip_color (name);

  for (i = 0; i < pfiles.count; i++)
    {
      if (!str_prefix (pname, pfiles.names[i]))
	return pfiles.names[i];
    }
  return "";
}

bool
is_donate_room (vnum_t room)
{
  return (room == ROOM_VNUM_DONATION_WEAPON
	  || room == ROOM_VNUM_DONATION_ARMOR
	  || room == ROOM_VNUM_DONATION_OTHER);
}


char *
timestr (time_t time, bool fShort)
{
  static char timebuf[3][MSL];
  static int pindex;


  long dsec, dmin, dhour, dday;
  long dweek, dmonth, dyear;
  char working[MSL];
  char *trim;



  ++pindex, pindex %= 3;
  timebuf[pindex][0] = NUL;

  if (time <= 0)
    return (char *) (!fShort ? "0 seconds" : "0:0:0");


  dsec = abs (time);

  dday = dsec / (DAY);
  dsec %= DAY;

  dyear = dday / 365;
  dday %= 365;

  dweek = dday / 7;
  dday %= 7;

  dmonth = dweek * 12 / 52;
  dweek -= dmonth * 52 / 12;

  dhour = dsec / (HOUR);
  dsec %= HOUR;

  dmin = dsec / MINUTE;
  dsec = dsec % MINUTE;


  working[0] = NUL;

  if (dyear != 0)
    {
      if (!fShort)
	sprintf (working, "%ld year%s, ", dyear, ((dyear != 1) ? "s" : ""));
      else
	sprintf (working, "%ld/", dyear);
      strcat (timebuf[pindex], working);
    }

  if (dmonth != 0)
    {
      if (!fShort)
	sprintf (working, "%ld month%s, ", dmonth,
		 ((dmonth != 1) ? "s" : ""));
      else
	sprintf (working, "%ld/", dmonth);
      strcat (timebuf[pindex], working);
    }

  if (dweek != 0)
    {
      if (!fShort)
	sprintf (working, "%ld week%s, ", dweek, ((dweek != 1) ? "s" : ""));
      else
	sprintf (working, "%ld:", dweek);
      strcat (timebuf[pindex], working);
    }

  if (dday != 0)
    {
      if (!fShort)
	sprintf (working, "%ld day%s, ", dday, ((dday != 1) ? "s" : ""));
      else
	sprintf (working, "%ld/", dday);
      strcat (timebuf[pindex], working);
    }

  if (dhour != 0)
    {
      if (!fShort)
	sprintf (working, "%ld hour%s, ", dhour, ((dhour != 1) ? "s" : ""));
      else
	sprintf (working, "%ld:", dhour);
      strcat (timebuf[pindex], working);
    }

  if (dmin != 0)
    {
      if (!fShort)
	sprintf (working, "%ld minute%s, ", dmin, ((dmin != 1) ? "s" : ""));
      else
	sprintf (working, "%ld:", dmin);
      strcat (timebuf[pindex], working);
    }

  if (dsec != 0)
    {
      if (!fShort)
	sprintf (working, "%ld second%s", dsec, ((dsec != 1) ? "s" : ""));
      else
	sprintf (working, "%ld:", dsec);
      strcat (timebuf[pindex], working);
    }

  trim = &timebuf[pindex][strlen (timebuf[pindex]) - 1];
  if (*trim == ' ' || *trim == ',' || *trim == ':' || *trim == '/')
    {
      *trim = NUL;
      trim--;
      if (*trim == ' ' || *trim == ',' || *trim == ':' || *trim == '/')
	*trim = NUL;
    }

  return (timebuf[pindex]);
}



const char *
getarg (const char *argument, char *arg, int length)
{
  int len = 0;

  if (NullStr (argument))
    {
      if (arg)
	arg[0] = '\0';

      return argument;
    }

  while (*argument && isspace (*argument))
    argument++;

  if (arg)
    while (*argument && !isspace (*argument) && len < length - 1)
      *arg++ = *argument++, len++;
  else
    while (*argument && !isspace (*argument))
      argument++;

  while (*argument && !isspace (*argument))
    argument++;

  while (*argument && isspace (*argument))
    argument++;

  if (arg)
    *arg = '\0';

  return argument;
}


bool
hasname (const char *list, const char *name)
{
  const char *p;
  char arg[MIL];

  if (!list)
    return false;

  p = getarg (list, arg, MIL);
  while (arg[0])
    {
      if (!str_cmp (name, arg))
	return true;
      p = getarg (p, arg, MIL);
    }
  return false;
}


void
flagname (const char **list, const char *name)
{
  char buf[MSL];

  if (hasname (*list, name))
    return;

  if (!NullStr (*list))
    snprintf (buf, sizeof (buf), "%s %s", *list, name);
  else
    strlcpy (buf, name, MSL);

  replace_str (list, buf);
}


void
unflagname (const char **list, const char *name)
{
  char buf[MSL], arg[MIL];
  const char *p;

  buf[0] = '\0';
  p = getarg (*list, arg, MIL);
  while (arg[0])
    {
      if (str_cmp (arg, name))
	{
	  if (buf[0])
	    strlcat (buf, " ", MSL);
	  strlcat (buf, arg, MSL);
	}
      p = getarg (p, arg, MIL);
    }
  replace_str (list, buf);
}

char *
ordinal_string (long n)
{
  static char buf[5][20];
  static int i;

  ++i, i %= 5;

  if (n == 1 || n == 0)
    strcpy (buf[i], "first");
  else if (n == 2)
    strcpy (buf[i], "second");
  else if (n == 3)
    strcpy (buf[i], "third");
  else if (n % 10 == 1)
    sprintf (buf[i], "%ldst", n);
  else if (n % 10 == 2)
    sprintf (buf[i], "%ldnd", n);
  else if (n % 10 == 3)
    sprintf (buf[i], "%ldrd", n);
  else
    sprintf (buf[i], "%ldth", n);

  return buf[i];
}
