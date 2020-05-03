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
#include "tables.h"
#include "recycle.h"
#include "data_table.h"
#include "vnums.h"

TableSave_Fun (rw_mbr_data)
{
  rw_list (type, MBR_FILE, ClanMember, mbr);
}

TableSave_Fun (rw_clan_data)
{
  rw_list (type, CLAN_FILE, ClanData, clan);
}

bool
is_leader (CharData * ch)
{
  return ch->rank == (MAX_RANK - 1);
}

Do_Fun (do_promote)
{
  char arg1[MIL];
  CharData *victim;
  int rank;

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg1);

  if ((!is_clan (ch) || !is_leader (ch)) && !IsImmortal (ch))
    {
      chprintln (ch, "You must be a clan Leader to promote someone.");
      return;
    }

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <rank #>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL || IsNPC (victim))
    {
      chprintln (ch, "They aren't playing.");
      return;
    }

  if (!is_clan (victim) || is_leader (victim)
      || (!is_same_clan (ch, victim) && !IsImmortal (ch)))
    {
      chprintln (ch, "You can't promote them.");
      return;
    }

  if (!is_number (argument) || (atoi (argument) > MAX_RANK)
      || (atoi (argument) < 1))
    {
      chprintlnf (ch, "Rank must be between 1 and %d.", MAX_RANK);
      return;
    }

  if ((rank = (atoi (argument) - 1)) == MAX_RANK - 1)
    {
      if (get_trust (victim) < LEVEL_HERO / 4)
	{
	  chprintlnf (ch, "They must reach level %d first.", LEVEL_HERO / 4);
	  return;
	}
    }

  if (victim->rank > rank)
    {
      chprintlnf (ch, "They have been demoted to %s.",
		  CharClan (victim)->rank[rank].rankname);
      chprintlnf (victim, "You have been demoted to %s, by %s.",
		  CharClan (victim)->rank[rank].rankname, ch->name);
      victim->rank = rank;
      update_members (victim, false);
    }
  else
    {
      chprintlnf (ch, "They are now a %s of clan %s.",
		  CharClan (victim)->rank[rank].rankname,
		  capitalize (CharClan (victim)->name));
      chprintlnf (victim, "You are now a %s of clan %s.",
		  CharClan (victim)->rank[rank].rankname,
		  capitalize (CharClan (victim)->name));
      victim->rank = rank;
      update_members (victim, false);
    }
}

Do_Fun (do_clist)
{
  ClanData *i;
  ClanMember *pmbr;
  char buf[MSL];
  char buf2[MSL];
  int e;

  chprintln (ch, "Clans available:");
  chprintln (ch, draw_line (ch, NULL, 0));
  for (e = ETHOS_LAWFUL_GOOD; e != ETHOS_CHAOTIC_EVIL; e--)
    for (i = clan_first; i; i = i->next)
      {
	if (i->ethos != (ethos_t) e)
	  continue;

	chprintf (ch, "%s %s", str_width (18, i->who_name), i->description);
	buf[0] = '\0';
	buf2[0] = '\0';
	for (pmbr = mbr_first; pmbr != NULL; pmbr = pmbr->next)
	  {
	    if (pmbr->clan != i || pmbr->rank != (MAX_RANK - 1))
	      continue;
	    sprintf (buf2, " %s,", pmbr->name);
	    strcat (buf, buf2);
	  }
	if (!NullStr (buf))
	  {
	    buf[strlen (buf) - 1] = '\0';
	    chprintf (ch, " (Leaders:%s)", buf);
	  }
	chprintln (ch, "{x");
      }
  chprintln (ch, draw_line (ch, NULL, 0));
  chprintlnf (ch, "For more info use '%s <clan>'.", cmd_name (do_cinfo));
}

Do_Fun (do_cinfo)
{
  int r;
  ClanData *i;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<clan name>", NULL);
      return;
    }

  if ((i = clan_lookup (argument)) == NULL)
    {
      chprintln (ch, "That is not a valid clan.");
      return;
    }

  chprintlnf (ch, "Name    : %s", i->name);
  chprintlnf (ch, "WhoName : %s", i->who_name);
  chprintlnf (ch, "Descrip : %s", i->description);
  chprintlnf (ch, "Ethos   : %s", flag_string (ethos_types, i->ethos));
  chprintlnf (ch, "Flags   : %s", flag_string (clan_flags, i->flags));
  if (IsImmortal (ch))
    {
      chprintlnf (ch, "Entrance: %ld", i->rooms[CLAN_ROOM_ENTRANCE]);
      chprintlnf (ch, "Morgue  : %ld", i->rooms[CLAN_ROOM_MORGUE]);
    }

  for (r = 0; r < MAX_RANK; r++)
    {
      chprintlnf (ch, "Rank %d : %s", r + 1, i->rank[r].rankname);
    }
}

Do_Fun (do_clanrecall)
{
  RoomIndex *location;

  if (!is_true_clan (ch))
    {
      chprintln (ch, "You're not in a clan.");
      return;
    }

  location = get_room_index (CharClan (ch)->rooms[CLAN_ROOM_ENTRANCE]);

  perform_recall (ch, location, "recall your clan");
  return;
}

Do_Fun (do_clanadmin)
{
  CharData *victim;
  char arg1[MIL];
  ClanData *clan;

  if (IsNPC (ch))
    return;

  if (!is_true_clan (ch) || (!is_leader (ch) && !IsImmortal (ch)))
    {
      chprintln (ch, "You must be a valid clan leader to use this command.");
      return;
    }

  argument = one_argument (argument, arg1);

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "where                                - show location of all members online.",
		  "promote <char> <rank#>               - promote/demote a character.",
		  "invite <char>                        - invite a character to your clan.",
		  "outcast <char>                       - kick a person out of your clan.",
		  "members                              - list members in clan.",
		  NULL);
      if (IsImmortal (ch))
	cmd_syntax
	  (ch, n_fun,
	   "<clan> <command>                     - do a clanadmin command for a specific clan.",
	   NULL);
      return;
    }
  if (IsImmortal (ch))
    {
      char arg2[MIL];

      if ((clan = clan_lookup (arg1)) == NULL)
	{
	  chprintln (ch, "Invalid clan.");
	  return;
	}
      argument = one_argument (argument, arg2);
      strcpy (arg1, arg2);
    }
  else
    {
      clan = CharClan (ch);
    }

  if (!str_cmp (arg1, "where"))
    {
      bool found = false;

      chprintln (ch, "Name          Position  Area");
      chprintln (ch, "----          --------  ----");
      for (victim = player_first; victim != NULL;
	   victim = victim->next_player)
	{
	  if (!IsNPC (victim) && can_see (ch, victim)
	      && !IsSwitched (victim) && is_clan (victim)
	      && CharClan (victim) == clan)
	    {
	      chprintlnf (ch, "%-12s %-8s %s", victim->name,
			  position_table[victim->position],
			  victim->in_room ? victim->in_room->
			  area->name : "Unknown");
	      found = true;
	    }
	}
      if (!found)
	chprintln (ch, "No members found online.");
      return;
    }
  else if (!str_prefix (arg1, "promote"))
    {
      do_function (ch, &do_promote, argument);
      return;
    }
  else if (!str_prefix (arg1, "members"))
    {
      ClanMember *mbr;
      bool found = false;

      chprintln (ch, "Lev  Name         Rank        Gold Donated QP Donated");
      chprintln (ch, "--- ------------ ------------ ------------ ----------");
      for (mbr = mbr_first; mbr != NULL; mbr = mbr->next)
	{
	  if (mbr->clan != clan)
	    continue;

	  chprintlnf (ch, "%3d %12s %12s %12ld %d", mbr->level, mbr->name,
		      mbr->clan->rank[mbr->rank].rankname,
		      mbr->gold_donated, mbr->qp_donated);
	  found = true;
	}
      if (!found)
	chprintln (ch, "No one in the clan.");
      return;
    }
  else if (!str_prefix (arg1, "invite"))
    {
      if ((victim = get_char_world (ch, argument)) == NULL)
	{
	  chprintln (ch, "They aren't playing.");
	  return;
	}
      if (IsNPC (victim))
	{
	  chprintln (ch, "NPC's cannot join clans.");
	  return;
	}

      if (victim == ch)
	{
	  chprintln (ch, "You're stuck...only a god can help you now!");
	  return;
	}
      if (is_true_clan (victim))
	{
	  chprintln (ch, "They are in a clan already.");
	  return;
	}
      if (victim->pcdata->invited != NULL)
	{
	  chprintln (ch, "They have already been invited to join a clan.");
	  return;
	}
      if (victim->deity->ethos < clan->ethos - 1
	  || victim->deity->ethos > clan->ethos + 1)
	{
	  chprintlnf (ch,
		      "%s's ethos is %s, too different from %s's ethos of %s.",
		      flag_string (ethos_types, victim->deity->ethos),
		      flag_string (ethos_types, clan->ethos));
	  return;
	}
      chprintlnf (ch, "%s has been invited to join your clan.", victim->name);
      chprintlnf (victim, "{RYou have been invited to join {x%s{x",
		  clan->who_name);
      chprintln (victim, "{YUse {Gjoin accept{Y to join this clan,{x");
      chprintln (victim, "{Yor {Gjoin deny{Y to turn down the invitation.{x");
      victim->pcdata->invited = clan;
      return;
    }
  else if (!str_prefix (arg1, "outcast"))
    {
      if ((victim = get_char_world (ch, argument)) == NULL)
	{
	  chprintln (ch, "There is no such player.");
	  return;
	}
      if (IsNPC (victim))
	{
	  chprintln (ch, "NPC's cannot join clans.");
	  return;
	}

      if (victim == ch)
	{
	  chprintln (ch, "You're stuck...only a god can help you now!");
	  return;
	}
      if (!is_clan (victim) || CharClan (victim) != clan)
	{
	  chprintln (ch, "They aren't in your clan.");
	  return;
	}
      if (is_leader (victim) && !IsImmortal (ch))
	{
	  chprintln (ch, "You can't kick out another leader.");
	  return;
	}
      chprintln (ch, "They are now clanless.");
      chprintln (victim, "Your clan leader has kicked you out!");
      update_members (victim, true);
      victim->pcdata->clan = NULL;
      victim->rank = 0;
      char_from_room (victim);
      char_to_room (victim, get_room_index (ROOM_VNUM_TEMPLE));
      do_function (victim, &do_look, "auto");
      return;
    }
  else
    do_function (ch, &do_clanadmin, "");
}

Do_Fun (do_join)
{
  char arg1[MIL];
  CharData *victim;

  argument = one_argument (argument, arg1);

  if (IsNPC (ch))
    {
      return;
    }

  if (is_true_clan (ch))
    {
      chprintln (ch, "You are already in a clan.");
      return;
    }

  if (ch->pcdata->invited == NULL)
    {
      chprintln (ch, "You have not been invited to join a clan.");
      return;
    }

  if (!str_cmp (arg1, "accept"))
    {
      for (victim = player_first; victim != NULL;
	   victim = victim->next_player)
	if (is_clan (victim) && CharClan (victim) == ch->pcdata->invited)
	  chprintlnf (victim,
		      "%s accepts the invitation to join %s.",
		      Pers (ch, victim), CharClan (victim)->name);

      ch->pcdata->clan = ch->pcdata->invited;
      ch->rank = 0;
      chprintlnf (ch, "{RYou are now a %s of {x%s{x",
		  CharClan (ch)->rank[ch->rank].rankname,
		  CharClan (ch)->who_name);
      ch->pcdata->invited = NULL;
      update_members (ch, false);
      return;
    }
  else if (!str_cmp (arg1, "deny"))
    {
      chprintln (ch, "You turn down the invitation.");
      for (victim = player_first; victim != NULL;
	   victim = victim->next_player)
	if (is_clan (victim)
	    && CharClan (victim) ==
	    ch->pcdata->invited && victim->rank >= MAX_RANK - 2)
	  chprintlnf (victim, "%s denies the invitation to join %s.",
		      Pers (ch, victim), CharClan (victim)->name);

      ch->pcdata->invited = NULL;
      return;
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun, "join <accept|deny>", NULL);
      return;
    }
}

void
update_members (CharData * ch, bool pdelete)
{
  ClanMember *curr, *next;
  money_t g_don = 0;
  int qp_don = 0;

  if (IsNPC (ch))
    return;

  for (curr = mbr_first; curr != NULL; curr = next)
    {
      next = curr->next;

      if (!str_cmp (ch->name, curr->name))
	{
	  g_don = curr->gold_donated;
	  qp_don = curr->qp_donated;
	  UnLink (curr, mbr, next, prev);
	  free_mbr (curr);
	  break;
	}
    }
  if (pdelete || !is_clan (ch))
    {
      rw_mbr_data (act_write);
      return;
    }

  curr = new_mbr ();
  replace_str (&curr->name, ch->name);
  curr->rank = ch->rank;
  curr->clan = CharClan (ch);
  curr->level = ch->level;
  curr->gold_donated = g_don;
  curr->qp_donated = qp_don;
  Link (curr, mbr, next, prev);
  rw_mbr_data (act_write);
  return;
}

Do_Fun (do_roster)
{
  int i, count = 0;
  ClanData *clan;
  ClanMember *pmbr;
  char buf[MSL], buf2[MSL];
  char arg[MIL];
  char *rcol[MAX_RANK] = { "{R", "{B", "{Y", "{M", "{G", "{C" };

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<clan name>", NULL);
      if (IsImmortal (ch))
	cmd_syntax (ch, NULL, n_fun, "delete <name>", NULL);
      return;
    }

  if (!str_cmp (arg, "delete") && IsImmortal (ch))
    {
      ClanMember *next = NULL;
      ClanMember *curr = NULL;
      bool found = false;

      for (curr = mbr_first; curr != NULL; curr = next)
	{
	  next = curr->next;

	  if (!str_cmp (argument, curr->name))
	    {
	      UnLink (curr, mbr, next, prev);

	      free_mbr (curr);
	      rw_mbr_data (act_write);
	      found = true;
	    }
	}
      if (!found)
	chprintlnf (ch, "Error deleting %s.", argument);
      else
	chprintlnf (ch, "%s removed from member list.", argument);
      return;
    }

  if ((clan = clan_lookup (arg)) == NULL)
    {
      chprintln (ch, "That clan does not exist.");
      return;
    }

  if (IsSet (clan->flags, CLAN_INDEPENDENT))
    {
      chprintln (ch, "That is not a real clan.");
      return;
    }

  chprintln (ch, stringf (ch, 0, Center, "{W-{w=", "[ %s{W Roster ]",
			  clan->who_name));
  for (i = MAX_RANK - 1; i >= 0; i--)
    {
      chprintf (ch, "%s%12ss {W:%s", rcol[i], clan->rank[i].rankname,
		rcol[i]);
      buf[0] = '\0';
      buf2[0] = '\0';
      count = 0;
      for (pmbr = mbr_first; pmbr != NULL; pmbr = pmbr->next)
	{
	  if (pmbr->rank != i || pmbr->clan != clan || NullStr (pmbr->name))
	    continue;
	  sprintf (buf2, " %s%12s {W({wLvl:{R%3d{W) ", rcol[i],
		   pmbr->name, pmbr->level);
	  strcat (buf, buf2);
	  if (++count % 2 == 0)
	    {
	      sprintf (buf2, NEWLINE "%15s", " ");
	      strcat (buf, buf2);
	    }
	}
      if (NullStr (buf))
	chprintlnf (ch, "  %12s", "No one.");
      else
	{
	  if (count % 2 != 0)
	    strcat (buf, NEWLINE);
	  chprintln (ch, buf);
	}
    }
  chprintln (ch, draw_line (ch, "{w-{W=", 0));
  return;
}
