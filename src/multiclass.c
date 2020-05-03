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
#include "vnums.h"
#include "interp.h"

Do_Fun (do_remort)
{
  Descriptor *d;
  CharData *mob;
  void send_race_info (Descriptor *);
  void send_class_info (Descriptor *);

  if (IsNPC (ch) || (d = ch->desc) == NULL)
    return;

  if (ch->in_room->guild == -1 || !is_class (ch, ch->in_room->guild))
    {
      chprintlnf (ch, "You must be at your class%s guild to do that.",
		  IsRemort (ch) ? "(s)" : "");
      return;
    }


  for (mob = ch->in_room->person_first; mob; mob = mob->next_in_room)
    {
      if (IsNPC (mob) && IsSet (mob->act, ACT_TRAIN | ACT_GAIN))
	break;
    }

  if (mob == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  if (ch->level < calc_max_level (ch))
    {
      chprintlnf (ch, "You must be a %s to remort.",
		  high_level_name (calc_max_level (ch), true));
      return;
    }

  if (ch->Class[CLASS_COUNT] == MAX_REMORT
      || ch->Class[CLASS_COUNT] == top_class)
    {
      chprintln (ch, "You can't remort any more!");
      return;
    }

  if (IsQuester (ch) || Gquester (ch))
    {
      chprintln (ch, "Don't you want to finish your quest first?");
      return;
    }

  if (!check_worth (ch, 500000, VALUE_GOLD) || ch->pcdata->quest.points < 500)
    {
      chprintln (ch, "You need 500,000 gold and 500 quest points to remort.");
      return;
    }

  if (ch->pcdata->confirm_remort)
    {
      if (!NullStr (argument))
	{
	  chprintln (ch, "Remort status removed.");
	  ch->pcdata->confirm_remort = false;
	  return;
	}
      else
	{
	  dwraplnf (ch->desc,
		    "You have chosen to remort.  You will now be dropped in at the %s"
		    " selection section of character creation, and will be allowed recreate"
		    " your character with an additional {Cclass{x%s and {Gbonuses{x."
		    NEWLINE
		    "In the unlikely event that you are disconnected or the Mud"
		    " crashes while you are creating your character, log back on and write a"
		    " note to an immortal who will retrieve your backup.",
		    !ch->pcdata->stay_race ? "{BRACE{x" : "{CCLASS{x",
		    !ch->pcdata->stay_race ? ", {brace{x" : "");
	  wiznet ("$N has remorted.", ch, NULL, 0, true, 0);
	  char_from_room (ch);
	  char_to_room (ch, get_room_index (ROOM_VNUM_LIMBO));
	  UnLink (ch, char, next, prev);

	  UnLink (ch, player, next_player, prev_player);


	  chprintln (ch, NEWLINE "Now beginning the remorting process.");

	  if (!ch->pcdata->stay_race)
	    {
	      send_race_info (ch->desc);
	      chprint
		(ch,
		 NEWLINE "What is your race (help for more information)?");
	      d->connected = CON_GET_NEW_RACE;
	    }
	  else
	    {
	      ch->pcdata->points = ch->race->points;

	      send_class_info (ch->desc);
	      chprintln (ch, "What is your next class? ");
	      d->connected = CON_GET_NEW_CLASS;
	    }
	  return;
	}
    }

  if (!NullStr (argument))
    {
      chprintln (ch, "Just type remort.  No argument.");
      return;
    }
  dwraplnf (ch->desc,
	    "{RTyping {Gremort{R with an argument will undo remort status."
	    "  Remorting is {Wnot reversable{R, make sure you read help REMORT"
	    " and have an idea of what {Cclass%s{R you want to remort into."
	    "  Type {Gremort{R again to confirm this command.{x",
	    !ch->pcdata->stay_race ? "and {Brace" : "");
  ch->pcdata->confirm_remort = true;
  if (!ch->pcdata->stay_race)
    chprintln (ch,
	       "{f{WWARNING{x{R: IF YOU CHOOSE A RACE DIFFERENT FROM YOUR RACE NOW YOU WILL BE THAT RACE {fFOREVER{x{R.{x");
  wiznet ("$N is contemplating remorting.", ch, NULL, 0, false,
	  get_trust (ch));
}

void
finish_remort (CharData * ch)
{
  int sn, b = lvl_bonus (ch);
  ObjData *obj, *obj_next;
  AffectData *af, *af_next;

  d_println (ch->desc, "Sucessful Remort!");
  wiznet ("Sucessful Remort! $N sighted.", ch, NULL, 0, false, 0);

  Link (ch, char, next, prev);

  Link (ch, player, next_player, prev_player);

  for (af = ch->affect_first; af != NULL; af = af_next)
    {
      af_next = af->next;
      affect_remove (ch, af);
    }
  for (obj = ch->carrying_first; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      if (obj->wear_loc != WEAR_NONE)
	{
	  unequip_char (ch, obj);
	}
    }
  ch->trust = ch->level;
  ch->level = 1;
  ch->exp = 0;
  ch->pcdata->points = 0;
  deduct_cost (ch, 500000, VALUE_GOLD);
  ch->pcdata->quest.points -= 500;
  ch->max_hit = 100 * b;
  ch->max_mana = 100 * b;
  ch->max_move = 100 * b;
  ch->hit = ch->max_hit;
  ch->mana = ch->max_move;
  ch->move = ch->max_mana;
  ch->pcdata->perm_hit = ch->max_hit;
  ch->pcdata->perm_mana = ch->max_mana;
  ch->pcdata->perm_move = ch->max_move;
  ch->wimpy = ch->max_hit / 5;
  ch->train = 5 * b;
  ch->practice = 7 * b;
  ch->exp = exp_per_level (ch, ch->pcdata->points);
  reset_char (ch);

  if (ch->pet != NULL)
    {
      nuke_pets (ch);
      ch->pet = NULL;
    }

  for (sn = 0; sn < top_skill; sn++)
    {
      if (ch->pcdata->learned[sn] > 0 && ch->pcdata->learned[sn] < 100)
	{
	  if (is_race_skill (ch, sn) && !ch->pcdata->stay_race)
	    ch->pcdata->learned[sn] = 0;
	  else
	    ch->pcdata->learned[sn] = 1;
	}
    }
  char_from_room (ch);
  char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
  ch->desc->connected = CON_PLAYING;
  act
    ("You are brought back to reality, and you feel quite different now...",
     ch, NULL, NULL, TO_CHAR);
  do_function (ch, &do_outfit, "");
  save_char_obj (ch);
  mud_info.stats.remorts++;
}


bool
can_use_skpell (CharData * ch, int sn)
{
  int i;

  if (sn < 0 || sn >= top_skill)
    return false;

  if (IsNPC (ch))
    return true;

  if (is_race_skill (ch, sn))
    return true;

  if (is_deity_skill (ch, sn))
    return true;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    if (ch->level >= skill_table[sn].skill_level[ch->Class[i]])
      return true;

  return false;
}


bool
has_spells (CharData * ch)
{
  int i;

  if (IsNPC (ch))
    return false;
  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    if (class_table[i].fMana)
      return true;
  return false;
}


bool
is_class (CharData * ch, int Class)
{
  int i;

  if (IsNPC (ch))
    return false;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (ch->Class[i] == Class)
	return true;
    }

  return false;
}


bool
is_same_class (CharData * ch, CharData * victim)
{
  int i, jClass;

  if (IsNPC (ch) || IsNPC (victim))
    return false;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      for (jClass = 0; victim->Class[jClass] != -1; jClass++)
	if (ch->Class[i] == victim->Class[jClass])
	  return true;
    }

  return false;
}


int
prime_class (CharData * ch)
{
  int slot = 0;

  if (!IsNPC (ch))
    slot = Range (0, ch->pcdata->prime_class, ch->Class[CLASS_COUNT] - 1);

  return ch->Class[slot];
}

int
current_class (CharData * ch)
{
  return ch->Class[ch->Class[CLASS_COUNT] - 1];
}

int
class_slot (CharData * ch, int pclass)
{
  int iClass;

  if (IsNPC (ch))
    return 0;

  for (iClass = 0; ch->Class[iClass] != -1; iClass++)
    {
      if (ch->Class[iClass] == pclass)
	return iClass;
    }

  return -1;
}


int
number_classes (CharData * ch)
{
  int i;

  if (IsNPC (ch))
    return number_range (1, MAX_REMORT);

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    ;

  return i;
}


char *
class_long (CharData * ch)
{
  static char buf[512];
  int i;

  buf[0] = '\0';
  if (IsNPC (ch))
    return "Mobile";
  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      strcat (buf, "/");
      strcat (buf, ClassName (ch, ch->Class[i]));
    }
  return buf + 1;
}


char *
class_who (CharData * ch)
{
  static char buf[512];

  buf[0] = '\0';
  if (IsNPC (ch))
    return "Mob";

  if (IsRemort (ch))
    sprintf (buf, "%2.2s+%d", ClassName (ch, prime_class (ch)),
	     ch->Class[CLASS_COUNT] - 1);
  else
    sprintf (buf, "%4.4s", ClassName (ch, prime_class (ch)));
  return buf;
}


char *
class_short (CharData * ch)
{
  static char buf[512];
  int i;

  buf[0] = '\0';
  if (IsNPC (ch))
    return "Mob";
  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      strcat (buf, "/");
      sprintf (buf + strlen (buf), "%4.4s", ClassName (ch, ch->Class[i]));
    }
  return buf + 1;
}


char *
class_numbers (CharData * ch)
{
  static char buf[512];
  char buf2[10];
  int i;

  buf[0] = '\0';

  if (IsNPC (ch))
    return "0";

  for (i = 0; i < MAX_MCLASS; i++)
    {
      strcat (buf, " ");
      sprintf (buf2, "%d", ch->Class[i]);
      strcat (buf, buf2);
    }
  return buf + 1;
}


int
skill_level (CharData * ch, int sn)
{
  int i;
  int tempskill = 999;

  if (sn < 0 || sn >= top_skill)
    return MAX_LEVEL + 1;

  if (is_race_skill (ch, sn))
    return 1;

  if (is_deity_skill (ch, sn))
    return 1;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    tempskill = Min (tempskill, skill_table[sn].skill_level[ch->Class[i]]);

  return tempskill == 999 ? LEVEL_IMMORTAL : tempskill;
}


int
skill_rating (CharData * ch, int sn)
{
  int i;
  int temprate = 999;

  if (sn < 0 || sn >= top_skill)
    return 0;

  if (is_race_skill (ch, sn))
    return 2;

  if (is_deity_skill (ch, sn))
    return 2;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (skill_table[sn].rating[ch->Class[i]] < 1)
	continue;

      temprate = Min (temprate, skill_table[sn].rating[ch->Class[i]]);
    }
  return temprate == 999 ? 0 : temprate;
}



int
group_rating (CharData * ch, int gn)
{
  int i;
  int temprate = 999;

  if (gn < 0 || gn >= top_group)
    return 0;

  if (is_race_skill (ch, gn))
    return 2;

  if (is_deity_skill (ch, gn))
    return 2;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (group_table[gn].rating[ch->Class[i]] < 1)
	continue;

      temprate = Min (temprate, group_table[gn].rating[ch->Class[i]]);
    }
  return temprate == 999 ? 0 : temprate;
}


int
get_hp_gain (CharData * ch)
{
  int i = 0;
  int gain = 0;
  int count = 0;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      gain =
	Max (gain,
	     number_range (class_table[ch->Class[i]].hp_min,
			   class_table[ch->Class[i]].hp_max));
      count++;
    }
  return number_range (gain, gain + count);
}

int
hp_max (CharData * ch)
{
  int i;
  int tmp = 0;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    tmp = Max (tmp, class_table[ch->Class[i]].hp_max);

  return tmp;
}


bool
is_prime_stat (CharData * ch, int stat)
{
  int i = 0;

  if (IsNPC (ch))
    return true;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (class_table[ch->Class[i]].attr_prime == stat)
	return true;
    }
  return false;
}


void
add_default_groups (CharData * ch)
{
  int i = 0;

  if (IsNPC (ch))
    return;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    if (class_table[ch->Class[i]].default_group != NULL)
      group_add (ch, class_table[ch->Class[i]].default_group, false);

  ch->pcdata->points += i > 0 ? 50 : 40;
}


void
add_base_groups (CharData * ch)
{
  int i = 0;

  if (IsNPC (ch))
    return;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    if (class_table[ch->Class[i]].base_group != NULL)
      group_add (ch, class_table[ch->Class[i]].base_group, false);
}


bool
check_base_group (CharData * ch, int gn)
{
  int i = 0;

  if (IsNPC (ch))
    return false;

  if (gn < 0 || gn >= top_group)
    return false;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (class_table[ch->Class[i]].base_group == NULL)
	continue;

      if (group_lookup (class_table[ch->Class[i]].base_group) == gn)
	return true;
    }
  return false;
}


bool
is_base_skill (CharData * ch, int sn)
{
  int i = 0;
  int gn, x;

  if (IsNPC (ch))
    return false;

  if (sn < 0 || sn >= top_skill)
    return false;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    {
      if (class_table[ch->Class[i]].base_group == NULL)
	continue;

      if ((gn = group_lookup (class_table[ch->Class[i]].base_group)) != -1)
	{
	  for (x = 0; x < MAX_IN_GROUP; x++)
	    {
	      if (group_table[gn].spells[x] == NULL)
		break;

	      if (skill_lookup (group_table[gn].spells[x]) == sn)
		return true;
	    }
	}
    }
  return false;
}


int
get_thac00 (CharData * ch)
{
  int temp = 0, i = 0;

  if (IsNPC (ch))
    return 0;
  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    temp = Max (temp, class_table[ch->Class[i]].thac0_00);

  return temp;
}


int
get_thac32 (CharData * ch)
{
  int temp = 999, i = 0;

  if (IsNPC (ch))
    return 0;
  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    temp = Min (temp, class_table[ch->Class[i]].thac0_32);

  return temp;
}


int
class_mult (CharData * ch)
{
  int temp = 999, i = 0;

  if (IsNPC (ch))
    return 0;

  for (i = 0; i < ch->Class[CLASS_COUNT]; i++)
    temp = Min (temp, ch->race->class_mult[ch->Class[i]]);

  return temp;
}


int
lvl_bonus (CharData * ch)
{
  float adlev, inclev;
  int cntr;

  adlev = ch->Class[CLASS_COUNT];
  inclev = .09;
  for (cntr = 1; cntr < ch->level; cntr++)
    {
      adlev += .9;
      adlev += inclev;
      inclev += .009;
    }
  return (int) (adlev + inclev);
}


bool
is_race_skill (CharData * ch, int sn)
{
  int i;

  if (sn < 0 || sn >= top_skill)
    return false;

  for (i = 0; i < MAX_RACE_SKILL; i++)
    {
      if (ch->race->skills[i] == NULL)
	continue;
      if (skill_lookup (ch->race->skills[i]) == sn)
	return true;
      if (group_lookup (ch->race->skills[i]) == sn)
	return true;
    }
  return false;
}

bool
is_deity_skill (CharData * ch, int sn)
{
  if (sn < 0 || sn >= top_skill)
    return false;

  if (ch->deity == NULL || IsNPC (ch))
    return false;

  if (NullStr (ch->deity->skillname))
    return false;

  if (skill_lookup (ch->deity->skillname) == sn)
    return true;

  if (group_lookup (ch->deity->skillname) == sn)
    return true;

  return false;
}


Do_Fun (do_prime)
{
  int iClass, iSlot;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<class>", NULL);
      chprintln
	(ch, "It costs {R5{x trivia points to change your prime class.");
    }

  if ((iClass = class_lookup (argument)) == -1)
    {
      chprintln (ch, "No such class!");
      return;
    }

  if ((iSlot = class_slot (ch, iClass)) == -1)
    {
      chprintlnf (ch, "You aren't part %s!", class_table[iClass].name);
      return;
    }

  if (iSlot == ch->pcdata->prime_class)
    {
      chprintlnf (ch, "Your prime class is already %s.",
		  class_table[iClass].name);
      return;
    }

  if (ch->pcdata->trivia < 5)
    {
      chprintln
	(ch, "It costs {R5{x trivia points to change your prime class.");
      return;
    }

  ch->pcdata->prime_class = iSlot;
  ch->pcdata->trivia -= 5;
  chprintlnf
    (ch,
     "Your prime class is now %s, and are {R5{x trivia points lighter.",
     class_table[iClass].name);
  return;
}
