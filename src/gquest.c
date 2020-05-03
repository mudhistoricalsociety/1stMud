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
#include "recycle.h"
#include "special.h"
#include "vnums.h"

typedef struct gquest_hist GQUEST_HIST;

struct gquest_hist
{
  LinkNext (GQUEST_HIST);
  const char *short_descr;
  const char *text;
};

GQUEST_HIST *gqhist_first, *gqhist_last;

bool
start_gquest (const char *n_fun, CharData * ch, const char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CharData *registar = NULL;
  int mobs, blevel, elevel, cost, max;

  for (registar = ch->in_room->person_first; registar != NULL;
       registar = registar->next_in_room)
    {
      if (!IsNPC (registar))
	continue;
      if (registar->spec_fun == spec_registar)
	break;
    }

  if (!IsImmortal (ch))
    {
      if (registar == NULL)
	{
	  chprintln (ch, "You can't do that here.");
	  return false;
	}

      if (registar->fighting != NULL)
	{
	  chprintln (ch, "Wait until the fighting stops.");
	  return false;
	}
      if (!str_cmp (gquest_info.who, GetName (ch)))
	{
	  chprintln (ch, "Let someone else have a chance.");
	  return false;
	}
    }
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (NullStr (arg1) || NullStr (arg2) || NullStr (arg3))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "start <min level> <max level> <#mobs>", NULL);
      return false;
    }

  blevel = atoi (arg1);
  elevel = atoi (arg2);
  mobs = atoi (arg3);

  if (blevel <= 0 || blevel > MAX_LEVEL)
    {
      chprintlnf (ch, "Level must be between 1 and %d.", MAX_LEVEL);
      return false;
    }

  if (blevel <= 0 || elevel > MAX_LEVEL)
    {
      chprintlnf (ch, "Level must be between 1 and %d.", MAX_LEVEL);
      return false;
    }

  if (elevel <= blevel)
    {
      chprintln (ch, "Max level must be greater than the min level.");
      return false;
    }

  if (elevel - blevel < 10)
    {
      chprintln (ch, "Level difference must 10 levels or higher.");
      return false;
    }

  max = (IsImmortal (ch) ? (mobile_count / 2) : 25);
  if (mobs < 5 || mobs > max)
    {
      chprintlnf (ch, "Number of mobs must be between 5 and %d.", max);
      return false;
    }

  if (gquest_info.running != GQUEST_OFF)
    {
      chprintln (ch, "There is already a global quest running!");
      return false;
    }

  cost = 5 + (mobs / 5);

  if (!IsImmortal (ch))
    {
      if (ch->pcdata->trivia < cost)
	{
	  sprintf (buf,
		   "$N tells you 'It costs %d Trivia Points to start a global quest with %d mobs.'",
		   cost, mobs);
	  act (buf, ch, NULL, registar, TO_CHAR);
	  return false;
	}
      else
	{
	  sprintf (buf,
		   "$N tells you '%d mobs have cost you %d trivia points.'",
		   mobs, cost);
	  act (buf, ch, NULL, registar, TO_CHAR);
	  ch->pcdata->trivia -= cost;
	}
    }

  gquest_info.running = GQUEST_WAITING;
  gquest_info.minlevel = blevel;
  gquest_info.maxlevel = elevel;
  gquest_info.mob_count = mobs;
  gquest_info.cost = cost;
  if (!generate_gquest (ch))
    {
      if (!IsImmortal (ch))
	{
	  chprintlnf (ch,
		      "Failed to start Gquest, you are being reimbursed %d TP.",
		      cost);
	  ch->pcdata->trivia += cost;
	}
      else
	chprintln (ch, "Failed to start a gquest, not enogh mobs found.");
      return false;
    }
  return true;
}

void
auto_gquest (void)
{
  CharData *wch = NULL;
  int middle = MAX_MORTAL_LEVEL / 2, maxlvl = 0, heros = 0;
  int minlvl = MAX_LEVEL, count = 0, lbonus = 0, half = 0;

  if (gquest_info.running != GQUEST_OFF)
    return;

  for (wch = player_first; wch != NULL; wch = wch->next_player)
    {
      if (!LinkDead (wch) && !IsImmortal (wch))
	{
	  count++;
	  maxlvl = Max (maxlvl, wch->level);
	  minlvl = Min (minlvl, wch->level);
	  if (wch->level >= LEVEL_HERO)
	    heros++;
	}
    }
  if (count < 1)
    {
      end_gquest ();
      return;
    }


  lbonus = number_range (10, 20);
  minlvl = Max (1, minlvl - lbonus);
  maxlvl = Min (MAX_MORTAL_LEVEL, maxlvl + lbonus);
  half = ((maxlvl - minlvl) / 2);
  middle = Range (minlvl, maxlvl - half, maxlvl);
  minlvl = Max (1, number_range (minlvl, (middle * 2) / 3));
  if (heros > 2 && number_percent () < 40)
    maxlvl = MAX_MORTAL_LEVEL;
  else
    maxlvl = Min (MAX_MORTAL_LEVEL, number_range ((middle * 3) / 2, maxlvl));
  gquest_info.running = GQUEST_WAITING;
  gquest_info.mob_count = number_range (5, 30 - lbonus);
  gquest_info.minlevel = Max (1, minlvl);
  gquest_info.maxlevel = Min (MAX_MORTAL_LEVEL, maxlvl);
  generate_gquest (get_char_vnum (MOB_VNUM_REGISTAR));
  return;
}

void
post_gquest (CharData * ch)
{
  Buffer *output;
  GqData *gql;
  CharIndex *mob;
  int i;
  GQUEST_HIST *hist;
  char shortd[MAX_INPUT_LENGTH];

  if (gquest_info.running == GQUEST_OFF || gquest_info.involved == 0)
    return;

  alloc_mem (hist, GQUEST_HIST, 1);
  sprintf (shortd, "%24s %3d %3d %4d %12s" NEWLINE,
	   str_time (current_time, -1, NULL), gquest_info.minlevel,
	   gquest_info.maxlevel, gquest_info.mob_count, ch->name);
  hist->short_descr = str_dup (shortd);
  output = new_buf ();
  bprintln (output, "GLOBAL QUEST INFO" NEWLINE "-----------------");
  bprintlnf (output, "Started by  : %s",
	     NullStr (gquest_info.who) ? "Unknown" : gquest_info.who);
  bprintlnf (output, "Levels      : %d - %d", gquest_info.minlevel,
	     gquest_info.maxlevel);
  bprintln (output, "Those Playing" NEWLINE "-------------");
  for (gql = gqlist_first; gql != NULL; gql = gql->next)
    if (gql->ch != ch)
      bprintlnf (output, "%s [%d mobs left]", gql->ch->name,
		 gquest_info.mob_count - count_gqmobs (gql));
  bprintlnf (output, "%s won the GQuest.", ch->name);
  bprintln (output, "Quest Rewards" NEWLINE "-------------");
  bprintlnf (output, "Qp Reward   : %d + 3 QPs for each target.",
	     gquest_info.qpoints);
  bprintlnf (output, "Gold Reward : %d", gquest_info.gold);
  bprintln (output, "Quest Targets" NEWLINE "-------------");
  for (i = 0; i < gquest_info.mob_count; i++)
    {
      if (gquest_info.mobs[i] < 0)
	continue;

      if ((mob = get_char_index (gquest_info.mobs[i])) != NULL)
	{
	  bprintlnf (output, "%2d) [%-20s] %-30s (level %3d)", i + 1,
		     mob->area->name, mob->short_descr, mob->level);
	}
    }
  hist->text = str_dup (buf_string (output));
  Link (hist, gqhist, next, prev);
  make_note ("Games",
	     GetStr (gquest_info.who, "Unknown"),
	     "all", FORMATF ("Global Quest Completed (%s)",
			     str_time (-1, -1, "%D")), 10,
	     buf_string (output));
  free_buf (output);
  return;
}

Do_Fun (do_gquest)
{
  char arg1[MAX_INPUT_LENGTH];
  CharData *wch;
  CharIndex *mob;
  int i = 0;
  Buffer *output;

  if (IsNPC (ch))
    {
      chprintln (ch, "Your the victim not the player.");
      return;
    }

  argument = one_argument (argument, arg1);

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "join     - join a global quest",
		  "quit     - quit the global quest",
		  "info     - show global quest info",
		  "time     - show global quest time",
		  "check    - show what targets you have left",
		  "progress - show progress of other players",
		  "complete - completes the current quest",
		  "hist     - shows gquest history since last reboot",
		  "start    - starts a gquest", NULL);
      if (IsImmortal (ch))
	{
	  cmd_syntax (ch, NULL, n_fun, "end      - ends the gquest (IMM)",
		      "next     - sets time to next gquest.", NULL);
	}

      return;
    }
  else if (!str_prefix (arg1, "start"))
    {
      start_gquest (n_fun, ch, argument);
      return;
    }
  else if (!str_prefix (arg1, "next") && IsImmortal (ch))
    {
      if (gquest_info.running != GQUEST_OFF)
	{
	  chprintln (ch, "Not while a gquest is running.");
	  return;
	}

      i = is_number (argument) ? atoi (argument) : number_range (30, 100);
      gquest_info.timer = i;
      chprintlnf (ch, "The next gquest will start in %s.",
		  intstr (gquest_info.timer, "minute"));
      return;
    }
  else if (!str_prefix (arg1, "end") && IsImmortal (ch))
    {
      end_gquest ();
      announce (ch, INFO_GQUEST | INFO_PRIVATE,
		"You end the global quest. Next autoquest in %s.",
		intstr (gquest_info.timer, "minute"));
      announce (ch, INFO_GQUEST,
		"$n has ended the global quest. Next gquest in %s.",
		intstr (gquest_info.timer, "minute"));
      return;
    }
  else if (!str_prefix (arg1, "hist"))
    {
      GQUEST_HIST *hist;
      int count = 0;

      if (!gqhist_first)
	{
	  chprintln (ch, "No global quests completed yet.");
	  return;
	}

      output = new_buf ();

      if (NullStr (argument))
	{
	  bprintln (output,
		    "Num Finished Time            Levels  Mobs Completed by"
		    NEWLINE
		    "--- ------------------------ ------- ---- ------------");
	  for (hist = gqhist_first; hist != NULL; hist = hist->next)
	    {
	      bprintf (output, "%2d) ", ++count);
	      bprint (output, hist->short_descr);
	    }
	  bprintlnf (output, "Type '%s hist #' to view details.", n_fun);
	}
      else
	{
	  bool found = false;

	  if (!is_number (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "hist #", NULL);
	      return;
	    }

	  for (hist = gqhist_first; hist != NULL; hist = hist->next)
	    if (++count == atoi (argument))
	      {
		bprint (output, hist->text);
		found = true;
	      }

	  if (!found)
	    bprintln (output, "History data not found.");
	}
      sendpage (ch, buf_string (output));
      free_buf (output);
      return;
    }
  else if (gquest_info.running == GQUEST_OFF)
    {
      chprintlnf (ch,
		  "There is no global quest running.  The next Gquest will start in %s.",
		  intstr (gquest_info.timer, "minute"));
      return;
    }
  else if (!str_prefix (arg1, "join"))
    {
      if (ch->fighting != NULL)
	{
	  chprintln (ch, "You're a little busy right now.");
	  return;
	}
      if (IsQuester (ch))
	{
	  chprintln (ch, "Why don't you finish your other quest first.");
	  return;
	}
      if (Gquester (ch))
	{
	  chprintln (ch, "Your allready in the global quest.");
	  return;
	}

      if (gquest_info.minlevel > ch->level ||
	  gquest_info.maxlevel < ch->level)
	{
	  chprintln (ch, "This gquest is not in your level range.");
	  return;
	}

      if (ch->war)
	{
	  chprintlnf (ch, "Your %s combat right now.",
		      war_info.status == WAR_WAITING ? "waiting for" : "in");
	  return;
	}

      ch->gquest = new_gqlist ();
      alloc_mem (ch->gquest->gq_mobs, vnum_t, gquest_info.mob_count);
      for (i = 0; i < gquest_info.mob_count; i++)
	ch->gquest->gq_mobs[i] = gquest_info.mobs[i];
      ch->gquest->ch = ch;
      Link (ch->gquest, gqlist, next, prev);
      gquest_info.involved++;
      announce
	(ch, INFO_GQUEST | INFO_PRIVATE,
	 "Your global quest flag is now on. Use '%s info' to see the quest(s).",
	 n_fun);
      announce (ch, INFO_GQUEST, "$n has joined the global quest.");
      return;
    }
  else if (!str_prefix (arg1, "quit"))
    {
      if (ch->gquest)
	{
	  UnLink (ch->gquest, gqlist, next, prev);
	  free_gqlist (ch->gquest);
	}
      if (!Gquester (ch))
	{
	  chprintln (ch, "Your not in a global quest.");
	}
      else
	{
	  gquest_info.involved--;
	  chprintln (ch,
		     "Your global quest flag is now off. Sorry you couldn't complete it.");
	  announce (ch, INFO_GQUEST,
		    "$n has quit the global quest, what a sore loser.");
	}
      return;
    }
  else if (!str_prefix (arg1, "info"))
    {
      output = new_buf ();

      bprintln (output, "[ GLOBAL QUEST INFO ]");
      bprintlnf (output, "Started by  : %s",
		 GetStr (gquest_info.who, "Unknown"));
      bprintlnf (output, "Playing     : %s.",
		 intstr (gquest_info.involved, "minute"));
      bprintlnf (output, "Levels      : %d - %d", gquest_info.minlevel,
		 gquest_info.maxlevel);
      bprintlnf (output, "Status      : %s for %s.",
		 gquest_info.running ==
		 GQUEST_WAITING ? "Waiting" : "Running",
		 intstr (gquest_info.timer, "minute"));
      bprintln (output, "[ Quest Rewards ]");
      bprintlnf (output, "Qp Reward   : %d", gquest_info.qpoints);
      bprintlnf (output, "Gold Reward : %d", gquest_info.gold);
      bprintln (output, "[ Quest Targets ]");
      for (i = 0; i < gquest_info.mob_count; i++)
	{
	  if (gquest_info.mobs[i] < 0)
	    continue;

	  if ((mob = get_char_index (gquest_info.mobs[i])) != NULL)
	    {
	      bprintlnf (output,
			 "%2d) [%-20s] %-30s (level %3d)",
			 i + 1, mob->area->name,
			 strip_color (mob->short_descr), mob->level);
	    }
	}
      sendpage (ch, buf_string (output));
      free_buf (output);
      return;
    }
  else if (!str_prefix (arg1, "time"))
    {
      if (gquest_info.running == GQUEST_OFF)
	chprintlnf (ch,
		    "The next Global Quest will start in %s.",
		    intstr (gquest_info.timer, "minute"));
      else
	chprintlnf (ch, "The Global Quest is %s for %s.",
		    gquest_info.running ==
		    GQUEST_WAITING ? "Waiting" : "Running",
		    intstr (gquest_info.timer, "minute"));
      return;
    }
  else if (!str_prefix (arg1, "progress"))
    {
      GqData *gql;

      if (gquest_info.running != GQUEST_RUNNING)
	{
	  chprintln (ch, "The global quest hasn't started yet.");
	  return;
	}
      for (gql = gqlist_first; gql != NULL; gql = gql->next)
	{
	  if (gql->ch != ch)
	    {
	      chprintlnf (ch, "%-12s has %d of %d mobs left.",
			  gql->ch->name,
			  gquest_info.mob_count -
			  count_gqmobs (gql), gquest_info.mob_count);
	    }
	}
      return;
    }
  else if (!str_prefix (arg1, "check"))
    {
      int count = 0;

      if (IsImmortal (ch) && !NullStr (argument))
	{
	  if ((wch = get_char_world (ch, argument)) == NULL || IsNPC (wch))
	    {
	      chprintln (ch, "That player is not here.");
	      return;
	    }
	}
      else
	wch = ch;

      if (!Gquester (wch))
	{
	  chprintlnf (ch, "%s aren't on a global quest.",
		      wch == ch ? "You" : wch->name);
	  return;
	}

      output = new_buf ();
      bprintlnf (output, "[ %s have %d of %d mobs left ]",
		 wch == ch ? "You" : wch->name,
		 gquest_info.mob_count - count_gqmobs (wch->gquest),
		 gquest_info.mob_count);
      for (i = 0; i < gquest_info.mob_count; i++)
	{
	  if (wch->gquest->gq_mobs[i] == -1)
	    continue;

	  if ((mob = get_char_index (wch->gquest->gq_mobs[i])) != NULL)
	    {
	      count++;
	      bprintlnf (output,
			 "%2d) [%-20s] %-30s (level %3d)",
			 count, mob->area->name, mob->short_descr,
			 mob->level);
	    }
	}
      sendpage (ch, buf_string (output));
      free_buf (output);
      return;
    }
  else if (!str_prefix (arg1, "complete"))
    {
      int mobs;

      if (!Gquester (ch))
	{
	  chprintln (ch, "Your not in a global quest.");
	  return;
	}

      if ((mobs = count_gqmobs (ch->gquest)) != gquest_info.mob_count)
	{
	  chprintlnf (ch,
		      "You haven't finished just yet, theres still %s to kill.",
		      intstr (gquest_info.mob_count - mobs, "minute"));
	  return;
	}
      chprintln (ch, "YES! You have completed the global quest.");
      post_gquest (ch);
      gquest_info.qpoints = add_qp (ch, gquest_info.qpoints);
      add_cost (ch, gquest_info.gold, VALUE_GOLD);
      chprintlnf (ch, "You receive %d gold and %d quest points.",
		  gquest_info.gold, gquest_info.qpoints);
      end_gquest ();
      announce (ch, INFO_GQUEST,
		"$n has completed the global quest, next gquest in %s.",
		intstr (gquest_info.timer, "minute"));
      return;
    }
  else
    do_gquest (n_fun, ch, "");
  return;
}

void
end_gquest (void)
{
  GqData *gql, *gql_next;

  if (gquest_info.running != GQUEST_RUNNING && !NullStr (gquest_info.who)
      && gquest_info.cost > 0)
    {
      CharData *vch;

      for (vch = player_first; vch; vch = vch->next_player)
	{
	  if (IsImmortal (vch))
	    continue;
	  if (!str_cmp (GetName (vch), gquest_info.who))
	    {
	      vch->pcdata->trivia += gquest_info.cost;
	      chprintlnf
		(vch,
		 "Unable to start global quest, being refunded %d TP.",
		 gquest_info.cost);
	      break;

	    }
	}
    }
  gquest_info.running = GQUEST_OFF;
  gquest_info.mob_count = 0;
  gquest_info.timer = number_range (100, 200);
  gquest_info.involved = 0;
  gquest_info.qpoints = 0;
  gquest_info.gold = 0;
  gquest_info.minlevel = 0;
  gquest_info.maxlevel = 0;
  gquest_info.cost = 0;
  if (gquest_info.mobs)
    free_mem (gquest_info.mobs);

  for (gql = gqlist_first; gql != NULL; gql = gql_next)
    {
      gql_next = gql->next;
      UnLink (gql, gqlist, next, prev);
      free_gqlist (gql);
    }
}

void
gquest_update (void)
{
  switch (gquest_info.running)
    {
    case GQUEST_OFF:
      if (gquest_info.timer > 0 && --gquest_info.timer == 0)
	auto_gquest ();
      break;
    case GQUEST_WAITING:
      if (--gquest_info.timer > 0)
	{
	  announce (NULL, INFO_GQUEST,
		    "%s left to join the global quest. (Levels %d - %d)",
		    intstr (gquest_info.timer, "minute"),
		    gquest_info.minlevel, gquest_info.maxlevel);
	}
      else
	{
	  if (gquest_info.involved == 0)
	    {
	      end_gquest ();
	      announce (NULL, INFO_GQUEST,
			"Not enough people for the global quest. The next quest will start in %s.",
			intstr (gquest_info.timer, "minute"));
	    }
	  else
	    {
	      gquest_info.timer =
		number_range (4 * gquest_info.mob_count,
			      6 * gquest_info.mob_count);
	      gquest_info.running = GQUEST_RUNNING;
	      announce (NULL, INFO_GQUEST,
			"The Global Quest begins! You have %s to complete the task!",
			intstr (gquest_info.timer, "minute"));
	    }
	}
      break;
    case GQUEST_RUNNING:
      if (gquest_info.involved == 0)
	{
	  end_gquest ();
	  announce (NULL, INFO_GQUEST,
		    "No one left in the Global Quest, next quest will start in %s.",
		    intstr (gquest_info.timer, "minute"));
	  return;
	}

      switch (gquest_info.timer)
	{
	case 0:
	  end_gquest ();
	  announce (NULL, INFO_GQUEST,
		    "Time has run out on the Global Quest, next quest will start in %s.",
		    intstr (gquest_info.timer, "minute"));
	  return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
	  announce (NULL, INFO_GQUEST,
		    "%s remaining in the global quest.",
		    intstr (gquest_info.timer, "minute"));
	default:
	  gquest_info.timer--;
	  break;
	}
      break;
    }
}

#define MAX_GQUEST_MOB_SEARCH    mobile_count

bool
generate_gquest (CharData * who)
{
  CharData *victim = NULL;
  vnum_t *vnums;
  int mob_count;
  int i;

  mob_count = 0;
  alloc_mem (vnums, vnum_t, MAX_GQUEST_MOB_SEARCH);

  for (victim = char_first; victim; victim = victim->next)
    {
      if (!IsNPC (victim) || victim->gquest
	  || victim->level > (gquest_info.maxlevel + 10)
	  || victim->level < (gquest_info.minlevel - 10)
	  || victim->pIndexData == NULL
	  || victim->in_room == NULL
	  || victim->pIndexData->pShop != NULL
	  || victim->pIndexData->vnum < 100
	  || IsSet (victim->in_room->room_flags, ROOM_PET_SHOP)
	  || victim->in_room->area->clan != NULL
	  || IsSet (victim->imm_flags, IMM_WEAPON | IMM_MAGIC)
	  || IsSet (victim->act,
		    ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER |
		    ACT_PET | ACT_GAIN)
	  || IsSet (victim->affected_by, AFF_CHARM)
	  || (IsSet (victim->act, ACT_SENTINEL)
	      && IsSet (victim->in_room->room_flags,
			ROOM_PRIVATE | ROOM_SOLITARY | ROOM_SAFE)))
	continue;
      vnums[mob_count] = victim->pIndexData->vnum;
      mob_count++;
      if (mob_count >= MAX_GQUEST_MOB_SEARCH)
	break;
    }

  if (mob_count < 5)
    {
      end_gquest ();
      free_mem (vnums);
      return false;
    }
  else if (mob_count < gquest_info.mob_count)
    {
      gquest_info.mob_count = mob_count;
    }

  alloc_mem (gquest_info.mobs, vnum_t, gquest_info.mob_count);

  for (i = 0; i < gquest_info.mob_count; i++)
    {
      do
	{
	  gquest_info.mobs[i] = vnums[number_range (0, mob_count - 1)];
	}
      while (!is_random_gqmob (gquest_info.mobs[i]));
    }

  gquest_info.qpoints = number_range (15, 30) * gquest_info.mob_count;
  gquest_info.gold = number_range (100, 150) * gquest_info.mob_count;
  gquest_info.timer = 3;
  replace_str (&gquest_info.who, (!who ? "AutoQuest" : GetName (who)));
  announce (who, INFO_GQUEST,
	    "%s Global Quest for levels %d to %d%s.  Type '%s info' to see the quest.",
	    !who ? "A" : "$n announces a", gquest_info.minlevel,
	    gquest_info.maxlevel, !who ? " has started" : "",
	    cmd_name (do_gquest));
  announce (who, INFO_GQUEST | INFO_PRIVATE,
	    "You announce a Global Quest for levels %d to %d with %d targets.",
	    gquest_info.minlevel, gquest_info.maxlevel,
	    gquest_info.mob_count);
  mud_info.stats.gquests++;
  free_mem (vnums);
  return true;
}

int
is_gqmob (GqData * gql, vnum_t vnum)
{
  int i;

  if (gquest_info.running == GQUEST_OFF)
    return -1;

  for (i = 0; i < gquest_info.mob_count; i++)
    {
      if (gql)
	{
	  if (gql->gq_mobs[i] == vnum)
	    return i;
	}
      else
	{
	  if (gquest_info.mobs[i] == vnum)
	    return i;
	}
    }

  return -1;
}

int
count_gqmobs (GqData * gql)
{
  int i, count = 0;

  if (gquest_info.running == GQUEST_OFF || !gql)
    return 0;

  for (i = 0; i < gquest_info.mob_count; i++)
    if (gql->gq_mobs[i] == -1)
      count++;

  return count;
}

bool
is_random_gqmob (vnum_t vnum)
{
  int i;

  if (gquest_info.running == GQUEST_OFF)
    return false;

  if (vnum == -1 || get_char_index (vnum) == NULL)
    return false;

  for (i = 0; i < gquest_info.mob_count; i++)
    if (gquest_info.mobs[i] == vnum)
      return false;

  return true;
}
