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
#include "magic.h"
#include "interp.h"

const struct healer_type
{
  char *name;
  char *descr;
  Spell_F *spell;
  char *words;
  money_t cost;
}
healer_table[] =
{
  {
  "light", "cure light wounds", spell_cure_light, "judicandus dies", 10}
  ,
  {
  "serious", "cure serious wounds", spell_cure_serious,
      "judicandus gzfuajg", 16}
  ,
  {
  "critical", "cure critical wounds", spell_cure_critical,
      "judicandus qfuhuqar", 25}
  ,
  {
  "heal", "healing spell", spell_heal, "pzar", 50}
  ,
  {
  "blindness", "cure blindness", spell_cure_blindness,
      "judicandus noselacri", 20}
  ,
  {
  "disease", "cure disease", spell_cure_disease, "judicandus eugzagz", 15}
  ,
  {
  "poison", "cure poison", spell_cure_poison, "judicandus sausabru", 25}
  ,
  {
  "uncurse", "remove a curse", spell_remove_curse, "candussido judifgz", 50}
  ,
  {
  "refresh", "restore movement", spell_refresh, "candusima", 5}
  ,
  {
  "mana", "restore mana", NULL, "energizer", 10}
  ,
  {
  NULL, NULL, NULL, 0}
};

Do_Fun (do_heal)
{
  CharData *mob;
  char arg[MAX_INPUT_LENGTH];
  int sn, i;


  for (mob = ch->in_room->person_first; mob; mob = mob->next_in_room)
    {
      if (IsNPC (mob) && IsSet (mob->act, ACT_IS_HEALER))
	break;
    }

  if (mob == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  one_argument (argument, arg);

  if (NullStr (arg))
    {

      act ("$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR);
      for (i = 0; healer_table[i].name != NULL; i++)
	chprintlnf (ch, "\t%12s: %24s %ld gold", healer_table[i].name,
		    healer_table[i].descr, healer_table[i].cost);
      chprintln (ch, "\tType heal <type> to be healed.");
      return;
    }

  for (i = 0; healer_table[i].name != NULL; i++)
    {
      if (!str_prefix (arg, healer_table[i].name))
	break;
    }

  if (healer_table[i].name == NULL)
    {
      act ("$N says 'Type 'heal' for a list of spells.'", ch, NULL, mob,
	   TO_CHAR);
      return;
    }

  if (!check_worth (ch, healer_table[i].cost, VALUE_GOLD))
    {
      act ("$N says 'You do not have enough gold for my services.'", ch,
	   NULL, mob, TO_CHAR);
      return;
    }

  WaitState (ch, PULSE_VIOLENCE);

  deduct_cost (ch, healer_table[i].cost, VALUE_GOLD);
  add_cost (mob, healer_table[i].cost, VALUE_GOLD);
  act ("$n utters the words '$T'.", mob, NULL, healer_table[i].words,
       TO_ROOM);

  if (healer_table[i].spell == NULL)
    {
      ch->mana += dice (2, 8) + mob->level / 3;
      ch->mana = Min (ch->mana, ch->max_mana);
      chprintln (ch, "A warm glow passes through you.");
      return;
    }

  if ((sn = spell_lookup (healer_table[i].spell)) == -1)
    return;

  (*healer_table[i].spell) (sn, mob->level, mob, ch, TARGET_CHAR);
}
