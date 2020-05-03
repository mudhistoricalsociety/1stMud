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
#include "tables.h"
#include "recycle.h"
#include "interp.h"
#include "vnums.h"

Proto (void death_cry, (CharData *));

Do_Fun (do_arena_challenge)
{
  CharData *victim;

  if (IsNPC (ch))
    return;

  if (!str_cmp (argument, "self") || !str_cmp (argument, "drop"))
    {
      extract_arena (ch);
      chprintln (ch, "You retract your challenge.");
      return;
    }

  if (mud_info.arena == FIGHT_START)
    {
      chprintln
	(ch,
	 "Sorry, some one else has already started a challenge, please try later.");
      return;
    }

  if (mud_info.arena == FIGHT_BUSY)
    {
      chprintln
	(ch,
	 "Sorry, there is a fight in progress, please wait a few moments.");
      return;
    }

  if (mud_info.arena == FIGHT_LOCK)
    {
      chprintln (ch, "Sorry, the arena is currently locked from use.");
      return;
    }

  if (ch->pcdata->challenger)
    {
      chprintlnf
	(ch,
	 "You have already been challenged, either ACCEPT or DECLINE %s first.",
	 ch->pcdata->challenger->name);
      return;
    }

  if (ch->hit < ch->max_hit)
    {
      chprintln (ch, "You must be fully healed to fight in the arena.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "You must specify whom you wish to challenge.");
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "They are not playing.");
      return;
    }

  if (IsNPC (victim) || IsImmortal (victim) || victim == ch)
    {
      chprintln (ch, "You cannot challenge NPC's or yourself.");
      return;
    }

  if (!can_see (victim, ch))
    {
      chprintln (ch, "They can't see you.");
      return;
    }

  if (victim->pcdata->challenger)
    {
      chprintln (ch, "They have already challenged someone else.");
      return;
    }

  if (victim->fighting != NULL)
    {
      chprintln (ch, "That person is engaged in battle right now.");
      return;
    }

  if (victim->hit < victim->max_hit)
    {
      chprintln (ch, "That player is not healthy enough to fight right now.");
      return;
    }

  if (victim->desc == NULL)
    {
      chprintln (ch,
		 "That player is linkdead at the moment, try them later.");
      return;
    }

  ch->pcdata->challenged = victim;
  victim->pcdata->challenger = ch;
  mud_info.arena = FIGHT_START;
  chprintlnf
    (ch,
     "Challenge has been sent.  Type '%s drop' to cancel the challenge.",
     n_fun);
  act ("{R$n has challenged you to a death match.", ch, NULL, victim,
       TO_VICT);
  announce (victim, INFO_ARENA, "%s has challenged $n to a duel.", ch->name);
  chprintlnf (victim, "Type: ARENA ACCEPT %s to meet the challenge.",
	      ch->name);
  chprintlnf (victim, "Type: ARENA DECLINE %s to chicken out.{x", ch->name);
  return;
}

Do_Fun (do_arena_accept)
{
  float odd1, odd2;
  float lvl1, lvl2;
  CharData *victim;
  RoomIndex *random1;
  RoomIndex *random2;

  random1 =
    get_room_index ((number_range
		     (ROOM_VNUM_DUEL_START, ROOM_VNUM_DUEL_END)));
  random2 =
    get_room_index ((number_range
		     (ROOM_VNUM_DUEL_START, ROOM_VNUM_DUEL_END)));

  if (IsNPC (ch))
    return;

  if (!ch->pcdata->challenger)
    {
      chprintln (ch, "You have not been challenged.");
      return;
    }

  if (mud_info.arena == FIGHT_BUSY)
    {
      chprintln
	(ch,
	 "Sorry, there is a fight in progress, please wait a few moments.");
      return;
    }

  if (mud_info.arena == FIGHT_LOCK)
    {
      chprintln (ch, "Sorry, the arena is currently locked from use.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "You must specify whose challenge you wish to accept.");
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "They aren't logged in!");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "You haven't challenged yourself!");
      return;
    }

  if (!victim->pcdata->challenged || victim != ch->pcdata->challenger)
    {
      chprintln (ch, "That player hasn't challenged you!");
      return;
    }

  chprintln (ch, "You have accepted the challenge!");
  act ("$n accepts your challenge!", ch, NULL, victim, TO_VICT);

  lvl1 = ch->hit + 1;
  lvl2 = victim->hit + 1;
  odd1 = (lvl1 / lvl2);
  odd2 = (lvl2 / lvl1);
  announce (NULL, INFO_ARENA, "%s (%d wins) (%d losses) Payoff odds %.2f",
	    victim->name, victim->pcdata->awins, victim->pcdata->alosses,
	    odd1);
  announce (NULL, INFO_ARENA, "%s (%d wins) (%d losses) Payoff odds %.2f",
	    ch->name, ch->pcdata->awins, ch->pcdata->alosses, odd2);
  announce (NULL, INFO_ARENA,
	    "To wager on the fight, type: arena bet (amount) (player name)");

  chprintln (ch, "You make your way into the arena.");
  char_from_room (ch);
  char_to_room (ch, random1);
  do_function (ch, &do_look, "auto");
  chprintln (victim, "You make your way to the arena.");
  char_from_room (victim);
  char_to_room (victim, random2);
  do_function (victim, &do_look, "auto");
  mud_info.arena = FIGHT_BUSY;
  return;
}

Do_Fun (do_arena_decline)
{
  CharData *victim;

  if (IsNPC (ch))
    return;

  if (!ch->pcdata->challenger)
    {
      chprintln (ch, "You have not been challenged.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "You must specify whose challenge you wish to decline.");
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "They aren't logged in!");
      return;
    }

  if (!victim->pcdata->challenged || victim != ch->pcdata->challenger)
    {
      chprintln (ch, "That player hasn't challenged you.");
      return;
    }

  if (victim == ch)
    return;

  ch->pcdata->challenged = NULL;
  victim->pcdata->challenged = NULL;
  ch->pcdata->challenger = NULL;
  victim->pcdata->challenger = NULL;
  mud_info.arena = FIGHT_OPEN;
  chprintln (ch, "Challenge declined!");
  act ("$n has declined your challenge.", ch, NULL, victim, TO_VICT);
  announce (ch, INFO_ARENA, "$n has declined %s's challenge.", victim->name);

  return;
}

Do_Fun (do_arena_bet)
{
  char arg[MIL];
  CharData *fighter;
  money_t wager;

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (argument) || !is_number (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "[amount] [player]", NULL);
      return;
    }

  if (ch->pcdata->gladiator != NULL)
    {
      chprintln (ch, "You have already placed a bet on this fight.");
      return;
    }

  if (ch->pcdata->challenger || ch->pcdata->challenged)
    {
      chprintln (ch, "You can't bet on this battle.");
      return;
    }

  fighter = get_char_world (ch, argument);

  if (fighter == NULL)
    {
      chprintln (ch, "That player is not logged in.");
      return;
    }

  if (IsNPC (fighter))
    {
      chprintln (ch, "Why bet on a mob? They aren't fighting...");
      return;
    }

  if (!IsSet (fighter->in_room->room_flags, ROOM_ARENA))
    {
      chprintln (ch, "That player is not in the arena.");
      return;
    }

  if (!str_prefix ("-", arg))
    {
      chprintln (ch, "Error: Invalid argument!");
      return;
    }

  wager = atoi (arg);

  if (wager > 5000 || wager < 1)
    {
      chprintln (ch, "Wager range is between 1 and 5,000");
      return;
    }

  if (!check_worth (ch, wager, VALUE_GOLD))
    {
      chprintln (ch, "You don't have that much gold to wager!");
      return;
    }

  ch->pcdata->gladiator = fighter;
  ch->pcdata->plr_wager = wager;
  chprintlnf (ch, "You have placed a %ld gold wager on %s",
	      wager, fighter->name);
  return;
}

void
check_arena (CharData * ch, CharData * victim)
{
  CharData *wch;
  float odds;
  float lvl1, lvl2;
  money_t payoff = 0;

  if (IsNPC (ch) || IsNPC (victim))
    return;
  announce (victim, INFO_ARENA, "%s has defeated $n!", ch->name);
  ch->pcdata->awins += 1;
  victim->pcdata->alosses += 1;
  lvl1 = ch->hit + 1;
  lvl2 = victim->hit + 1;
  odds = (lvl2 / lvl1);
  for (wch = player_first; wch; wch = wch->next_player)
    {
      if (wch->pcdata->gladiator == ch)
	{
	  payoff = wch->pcdata->plr_wager * (Max ((money_t) odds + 1, 2));
	  chprintlnf (wch, "You won! Your wager: %ld, payoff: %ld",
		      wch->pcdata->plr_wager, payoff);
	  add_cost (wch, Max (0, payoff), VALUE_GOLD);
	  wch->pcdata->gladiator = NULL;
	  wch->pcdata->plr_wager = 0;
	  payoff = 0;
	}
      if (wch->pcdata->gladiator && wch->pcdata->gladiator != ch
	  && wch->pcdata->plr_wager >= 1)
	{
	  chprintlnf (wch, "You lost! Your wager: %ld",
		      wch->pcdata->plr_wager);
	  deduct_cost (wch, wch->pcdata->plr_wager, VALUE_GOLD);

	  wch->pcdata->gladiator = NULL;
	  wch->pcdata->plr_wager = 0;
	}
    }

  stop_fighting (victim, true);
  death_cry (victim);
  char_from_room (victim);
  char_to_room (victim, get_room_index (ROOM_VNUM_DUEL_LOSER));
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  update_pos (victim);
  do_function (victim, &do_look, "auto");
  stop_fighting (ch, true);
  char_from_room (ch);
  char_to_room (ch, get_room_index (ROOM_VNUM_DUEL_WINNER));
  ch->hit = ch->max_hit;
  ch->mana = ch->max_mana;
  ch->move = ch->max_move;
  update_pos (ch);
  do_function (ch, &do_look, "auto");

  ch->pcdata->challenger = NULL;
  ch->pcdata->challenged = NULL;
  victim->pcdata->challenger = NULL;
  victim->pcdata->challenged = NULL;

  mud_info.arena = FIGHT_OPEN;

  return;
}

void
extract_arena (CharData * ch)
{
  if (mud_info.arena == FIGHT_START && ch->pcdata->challenger != NULL)
    do_function (ch, &do_arena_decline, ch->pcdata->challenger->name);
  if (mud_info.arena == FIGHT_START && ch->pcdata->challenged != NULL)
    {
      ch->pcdata->challenged->pcdata->challenger = NULL;
      ch->pcdata->challenged = NULL;
      mud_info.arena = FIGHT_OPEN;
    }
}

bool
IS_IN_ARENA (CharData * ch)
{
  if (!ch)
    return false;

  if (IsNPC (ch))
    return false;

  if (!ch->pcdata->challenged && !ch->pcdata->challenger)
    return false;

  if (!ch->in_room)
    return false;

  if (!IsSet (ch->in_room->room_flags, ROOM_ARENA))
    return false;

  return true;
}

Do_Fun (do_arena_help)
{
  cmd_syntax (ch, NULL, n_fun, "challenge <name>|drop",
	      "accept <name>", "decline <name>", "bet <name>", NULL);

  if (IsImmortal (ch))
    {
      cmd_syntax (ch, NULL, n_fun, "clear|lock|busy>", NULL);
      switch (mud_info.arena)
	{
	case FIGHT_OPEN:
	  chprintln (ch, "Arena is [CLEAR]");
	  break;

	case FIGHT_START:
	  chprintln (ch, "A challenge has been started.");
	  break;

	case FIGHT_BUSY:
	  chprintln (ch, "Arena is [BUSY]");
	  break;

	case FIGHT_LOCK:
	  chprintln (ch, "Arena is [LOCKED]");
	}
    }
  return;
}

Do_Fun (do_arena_clear)
{
  Descriptor *d;

  if (!IsImmortal (ch))
    {
      do_arena_help (n_fun, ch, "");
      return;
    }
  mud_info.arena = FIGHT_OPEN;
  chprintln (ch, "Arena now set [CLEARED]");
  announce (NULL, INFO_ARENA, "The arena has been opened.");
  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character)
	{
	  d->character->pcdata->challenger = NULL;
	  d->character->pcdata->challenged = NULL;
	  if (IsSet (d->character->in_room->room_flags, ROOM_ARENA))
	    {
	      char_from_room (ch);
	      char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
	      chprintln (d->character,
			 "You re-appear in the Temple of Midgaard.");
	    }
	}
    }
  return;
}

Do_Fun (do_arena_busy)
{
  if (!IsImmortal (ch))
    {
      do_arena_help (n_fun, ch, "");
      return;
    }
  mud_info.arena = FIGHT_BUSY;
  chprintln (ch, "Arena now set [BUSY]");
  announce (NULL, INFO_ARENA, "The arena is now busy.");
  return;

}

Do_Fun (do_arena_lock)
{
  if (!IsImmortal (ch))
    {
      do_arena_help (n_fun, ch, "");
      return;
    }
  mud_info.arena = FIGHT_LOCK;
  chprintln (ch, "Arena now set [LOCKED]");
  announce (NULL, INFO_ARENA, "The arena has been locked.");
  return;
}

Do_Fun (do_arena)
{
  vinterpret (ch, n_fun, argument, "challenge", do_arena_challenge,
	      "decline", do_arena_decline, "accept", do_arena_accept,
	      "bet", do_arena_bet, "clear", do_arena_clear, "busy",
	      do_arena_busy, "lock", do_arena_lock, NULL, do_arena_help);
}
