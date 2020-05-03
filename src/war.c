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
#include "special.h"
#include "vnums.h"
#include "interp.h"

#define WAR_KILLED    	(BIT_A)
#define WAR_DECOY    	(BIT_B)

struct wars_t
{
  const char *name;
  const char *plural;
  war_types type;
};

const struct wars_t war_table[MAX_WAR] = {
  {"none", "none", WAR_NONE},
  {"clan", "clans", WAR_CLAN},
  {"race", "races", WAR_RACE},
  {"class", "classes", WAR_CLASS},
  {"genocide", "people", WAR_GENOCIDE},
  {"deity", "deities", WAR_DEITY},
  {"sex", "sexes", WAR_SEX}
};

Lookup_Fun (war_lookup)
{
  int i;

  for (i = WAR_NONE; i < MAX_WAR; i++)
    {
      if (is_number (name) ? atoi (name) == war_table[i].type
	  : !str_prefix (name, war_table[i].name))
	return i;
    }
  return -1;
}

char *
wartype_name (int type, bool plural)
{
  int i;

  for (i = 0; i < MAX_WAR; i++)
    if (war_table[i].type == type)
      return capitalize (!plural ? war_table[i].name : war_table[i].plural);

  return "Unknown";
}

#define GET_WAR_CLASS(ch) \
    	Range(0, ch->war->Class, top_class)

#define WAR_COST 3

bool
start_war (const char *n_fun, CharData * ch, const char *argument)
{
  char arg1[MIL], arg2[MIL];
  char arg3[MIL];
  CharData *warmaster = NULL;
  int blevel, elevel, type;

  for (warmaster = ch->in_room->person_first; warmaster != NULL;
       warmaster = warmaster->next_in_room)
    {
      if (!IsNPC (warmaster))
	continue;
      if (warmaster->spec_fun == spec_warmaster)
	break;
    }

  if (!IsImmortal (ch) && warmaster == NULL)
    {
      chprintln (ch, "You can't do that here.");
      return false;
    }

  if (!IsImmortal (ch) && warmaster->fighting != NULL)
    {
      chprintln (ch, "Wait until the fighting stops.");
      return false;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (NullStr (arg1) || NullStr (arg2) || NullStr (arg3))
    {
      int i;

      cmd_syntax (ch, NULL, n_fun, "start <min_level> <max_level> <type>",
		  NULL);
      chprintln (ch, "where <type> is either:");
      for (i = 1; war_table[i].name != NULL; i++)
	chprintlnf (ch, "%d - %s war", war_table[i].type, war_table[i].name);
      return false;
    }

  blevel = atoi (arg1);
  elevel = atoi (arg2);
  type = war_lookup (arg3);

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

  if (elevel < blevel)
    {
      chprintln (ch, "Max level must be greater than the min level.");
      return false;
    }

  if (elevel - blevel < 5)
    {
      chprintln (ch, "Levels must have a difference of at least 5.");
      return false;
    }

  if (type == -1)
    {
      int i;

      chprintln (ch, "The type either has to be:");
      for (i = 1; war_table[i].name != NULL; i++)
	chprintlnf (ch, "%d (%s)", war_table[i].type, war_table[i].name);
      return false;
    }

  if (war_info.status != WAR_OFF)
    {
      chprintln (ch, "There is already a war going!");
      return false;
    }

  if (!IsImmortal (ch))
    {
      if (ch->pcdata->trivia < WAR_COST)
	{

	  mob_tell (ch, warmaster,
		    "It costs %d Trivia Points to start a %s war.",
		    WAR_COST, wartype_name (type, false));
	  return false;
	}
      else
	{

	  mob_tell (ch, warmaster,
		    "Thank you %s, %s war started, you are %d trivia points lighter.",
		    ch->name, wartype_name (type, false), WAR_COST);
	}
    }

  war_info.status = WAR_WAITING;
  replace_str (&war_info.who, ch->name);
  war_info.min_level = blevel;
  war_info.max_level = elevel;
  war_info.wartype = war_table[type].type;
  announce (ch, INFO_WAR,
	    "$n announces a %s war for levels %d to %d.  Type 'WAR JOIN' to kill or be killed.",
	    wartype_name (war_info.wartype, false), war_info.min_level,
	    war_info.max_level);
  announce (ch, INFO_WAR | INFO_PRIVATE,
	    "You announce a %s war for levels %d to %d.  Type 'WAR JOIN' to kill or be killed.",
	    wartype_name (war_info.wartype, false), war_info.min_level,
	    war_info.max_level);
  war_info.timer = 3;
  mud_info.stats.wars++;
  return true;
}

void
auto_war (void)
{
  CharData *wch, *wch_last;
  static CharData *warmaster = NULL;
  int maxlvl = 0, minlvl = MAX_LEVEL, middle = MAX_MORTAL_LEVEL / 2;
  int clan = 0, count = 0, lbonus = 0, half = 0;
  int heros = 0;

  if (war_info.status != WAR_OFF)
    return;

  for (wch = player_first; wch != NULL; wch = wch->next_player)
    {
      if (!wch->desc)
	continue;

      if (!IsNPC (wch) && !IsImmortal (wch))
	{
	  count++;
	  maxlvl = Max (maxlvl, wch->level);
	  minlvl = Min (minlvl, wch->level);
	  if (wch->level >= LEVEL_HERO && wch->level <= MAX_MORTAL_LEVEL)
	    heros++;
	  if (is_clan (wch))
	    {
	      for (wch_last = player_first; wch_last != wch;
		   wch_last = wch_last->next_player)
		{
		  if (!IsNPC (wch_last)
		      && !IsImmortal (wch_last)
		      && is_clan (wch_last) && !is_same_clan (wch, wch_last))
		    clan++;
		}
	    }
	}
    }
  if (count < 2)
    {
      end_war ();
      return;
    }

  lbonus = number_range (15, 30);
  minlvl = Max (1, minlvl - lbonus);
  maxlvl = Min (MAX_MORTAL_LEVEL, maxlvl + lbonus);
  half = ((maxlvl - minlvl) / 2);
  middle = Range (minlvl, maxlvl - half, maxlvl);
  minlvl = Max (1, number_range (minlvl, (middle * 2) / 3));
  if (heros > 2 && number_percent () < 25)
    maxlvl = MAX_MORTAL_LEVEL;
  else
    maxlvl = Min (MAX_MORTAL_LEVEL, number_range ((middle * 3) / 2, maxlvl));
  if (warmaster == NULL)
    {
      for (warmaster = char_first; warmaster != NULL;
	   warmaster = warmaster->next)
	if (warmaster->pIndexData
	    && warmaster->pIndexData->vnum == MOB_VNUM_WARMASTER)
	  break;
    }
  war_info.status = WAR_WAITING;
  replace_str (&war_info.who,
	       (!warmaster ? "AutoWar" : warmaster->short_descr));
  war_info.min_level = minlvl;
  war_info.max_level = maxlvl;

  if (clan >= 2)
    war_info.wartype = (war_types) number_range (WAR_NONE + 1, MAX_WAR - 1);
  else
    war_info.wartype = (war_types) number_range (WAR_CLAN + 1, MAX_WAR - 1);

  if (war_info.wartype == WAR_NONE)
    war_info.wartype = WAR_GENOCIDE;

  announce (warmaster, INFO_WAR,
	    "%s %s war for levels %d to %d%s.  Type 'WAR JOIN' to kill or be killed.",
	    !warmaster ? "A" : "$n announces a",
	    wartype_name (war_info.wartype, false), war_info.min_level,
	    war_info.max_level, !warmaster ? " has started" : "");
  announce (warmaster, INFO_WAR | INFO_PRIVATE,
	    "You announce a %s war for levels %d"
	    " to %d.  Type 'WAR JOIN' to kill or be killed.",
	    wartype_name (war_info.wartype, false), war_info.min_level,
	    war_info.max_level);
  war_info.timer = 3;
  mud_info.stats.wars++;
}

void
end_war (void)
{
  WarData *wl, *wl_next;

  for (wl = warlist_first; wl != NULL; wl = wl_next)
    {
      wl_next = wl->next;

      if (wl->ch)
	{
	  stop_fighting (wl->ch, true);
	  if (!IsSet (wl->flags, WAR_DECOY))
	    {
	      if (!IsSet (wl->flags, WAR_KILLED))
		{
		  char_from_room (wl->ch);
		  char_to_room (wl->ch, get_room_index (ROOM_VNUM_TEMPLE));
		}
	      wl->ch->hit = wl->hit;
	      wl->ch->mana = wl->mana;
	      wl->ch->move = wl->move;
	      update_pos (wl->ch);
	      do_function (wl->ch, &do_look, "auto");
	    }
	}
      UnLink (wl, warlist, next, prev);
      free_warlist (wl);
    }
  if (war_info.status == WAR_RUNNING && !NullStr (war_info.who))
    {
      CharData *vch;

      for (vch = player_first; vch; vch = vch->next_player)
	{
	  if (!IsImmortal (vch) && !str_cmp (GetName (vch), war_info.who))
	    {
	      chprintlnf (vch,
			  "War not started, you are being refunded %d TP.",
			  WAR_COST);
	      vch->pcdata->trivia += WAR_COST;
	      break;
	    }
	}
    }
  war_info.wartype = WAR_NONE;
  war_info.min_level = 0;
  war_info.max_level = 0;
  war_info.status = WAR_OFF;
  war_info.inwar = 0;
  war_info.timer = number_range (100, 200);
  warlist_first = warlist_last = NULL;
}

const char *
wartype_info (CharData * wch)
{
  CharData *ch;

  ch = (wch->war->owner ? wch->war->owner : wch);

  switch (war_info.wartype)
    {
    default:
      return "";
    case WAR_RACE:
      return ch->race->name;
    case WAR_CLASS:
      return ClassName (ch, GET_WAR_CLASS (ch));
    case WAR_GENOCIDE:
      return ch->name;
    case WAR_CLAN:
      return CharClan (ch)->who_name;
    case WAR_DEITY:
      return ch->deity->name;
    case WAR_SEX:
      return flag_string (sex_flags, ch->sex);
    }
}

bool
check_wartype_data (CharData * wch, CharData * vict)
{
  CharData *ch = (wch->war->owner ? wch->war->owner : wch);
  CharData *victim = (vict->war->owner ? vict->war->owner : vict);

  switch (war_info.wartype)
    {
    default:
      return false;
    case WAR_RACE:
      return (ch->race == victim->race);
    case WAR_CLASS:
      return (GET_WAR_CLASS (ch) == GET_WAR_CLASS (victim));
    case WAR_GENOCIDE:
      return (ch->id == victim->id);
    case WAR_CLAN:
      return (CharClan (ch) == CharClan (victim));
    case WAR_DEITY:
      return (ch->deity == victim->deity);
    case WAR_SEX:
      return (ch->sex == victim->sex);
    }
}

char *
warrior_status (CharData * ch)
{
  if (war_info.wartype != WAR_GENOCIDE)
    return FORMATF ("%s (%s, Lvl %d)", ch->name, wartype_info (ch),
		    ch->level);
  else
    return FORMATF ("%s (Lvl %d)", ch->name, ch->level);
}

int
count_type_in_war (CharData * join)
{
  WarData *wl;
  int count = 0;

  for (wl = warlist_first; wl != NULL; wl = wl->next)
    {
      if (IsSet (wl->flags, WAR_DECOY) || !wl->ch || wl->ch == join)
	continue;

      if (check_wartype_data (wl->ch, join))
	count++;
    }
  return count;
}

Do_Fun (do_war)
{
  char arg[MIL];
  RoomIndex *location;
  int i = 0;

  if (IsNPC (ch))
    {
      chprintln (ch, "Mobiles not supported yet.");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "start <minlev> <maxlev> <#type>",
		  "talk <msg>", "status", "info", "join", "decoy", NULL);
      if (IsImmortal (ch))
	cmd_syntax (ch, NULL, n_fun, "next", "end", NULL);
      return;
    }
  else if (!str_cmp (arg, "start"))
    {
      if (ch->pcdata->trivia < WAR_COST && !IsImmortal (ch))
	{
	  chprintlnf (ch,
		      "It costs %d Trivia Points to start a war.", WAR_COST);
	  return;
	}
      if (start_war (n_fun, ch, argument) && !IsImmortal (ch))
	ch->pcdata->trivia -= WAR_COST;
      return;
    }
  else if (!str_cmp (arg, "talk"))
    {
      war_talk (ch, argument);
      return;
    }
  else if (!str_cmp (arg, "next") && IsImmortal (ch))
    {
      if (war_info.status != WAR_OFF)
	{
	  chprintln (ch, "Not while a war is running.");
	  return;
	}

      i = is_number (argument) ? atoi (argument) : number_range (30, 100);
      war_info.timer = i;
      chprintlnf (ch, "The next war will start in %s.",
		  intstr (war_info.timer, "minute"));
      return;
    }

  if (war_info.status == WAR_OFF)
    {
      chprintf (ch,
		"There is no war going! The next war will start in %s.",
		intstr (war_info.timer, "minute"));
      return;
    }

  if (!str_cmp (arg, "end") && IsImmortal (ch))
    {
      end_war ();
      announce (ch, INFO_WAR,
		"$n has ended the war. The next autowar will start in %s.",
		intstr (war_info.timer, "minute"));
      announce (ch, INFO_WAR | INFO_PRIVATE,
		"You have ended the war. The next autowar will start in %s.",
		intstr (war_info.timer, "minute"));
      return;
    }
  else if (!str_cmp (arg, "info"))
    {
      chprintln (ch, stringf (ch, 0, Center, "{g-{G-", "{W[ WAR INFO ]"));
      chprintlnf (ch, "{RStarted by  : {W%s",
		  GetStr (war_info.who, "Unknown"));
      chprintlnf (ch, "{RFighting    : {W%s.",
		  intstr (war_info.inwar, "player"));
      chprintlnf (ch, "{RLevels      : {W%d - %d{x", war_info.min_level,
		  war_info.max_level);
      chprintlnf (ch, "{RStatus      : {W%s for %s.{x",
		  war_info.status == WAR_WAITING ? "Waiting" : "Running",
		  intstr (war_info.timer, "minute"));
      chprintlnf (ch, "{RType        : {W%s war.{x",
		  wartype_name (war_info.wartype, false));
      chprintln (ch, draw_line (ch, "{g-{G-", 0));
      return;
    }
  else if (!str_cmp (arg, "status"))
    {
      WarData *wl;
      bool found = false;

      chprintln (ch, stringf (ch, 0, Center, "{g-{G-",
			      "{W[ WAR COMBATENTS ]"));
      for (wl = warlist_first; wl != NULL; wl = wl->next)
	{
	  if (!IsSet (wl->flags, WAR_DECOY) && wl->ch)
	    {
	      if (!IsSet (wl->flags, WAR_KILLED))
		chprintlnf (ch,
			    "{W%-25s : [{R%ld%% hit{W] [{M%ld%% mana{W] [Pos: {G%s{W]{x",
			    warrior_status (wl->ch),
			    Percent (wl->ch->hit, wl->ch->max_hit),
			    Percent (wl->ch->mana, wl->ch->max_mana),
			    position_flags[wl->ch->position].name);
	      else
		chprintlnf (ch, "{W%-25s [{RKILLED{W]{x",
			    warrior_status (wl->ch));
	      found = true;
	    }
	}
      if (!found)
	chprintln (ch, "No one in the war yet.");
      chprintln (ch, draw_line (ch, "{g-{G-", 0));
      return;
    }
  else if (!str_cmp (arg, "decoy"))
    {
      WarData *wl;
      CharData *dc;
      int count = 0;

      if (war_info.status != WAR_RUNNING)
	{
	  chprintln (ch, "Wait untill the war starts.");
	  return;
	}

      if (!InWar (ch))
	{
	  chprintln (ch, "You aren't in the war.");
	  return;
	}

      for (wl = warlist_first; wl; wl = wl->next)
	{
	  if (!IsSet (wl->flags, WAR_DECOY))
	    continue;

	  if (wl->owner != ch)
	    continue;

	  count++;
	}

      if (count >= 5)
	{
	  chprintln (ch,
		     "I'm sorry you are only allowed to deploy 5 decoys.");
	  return;
	}

      if ((dc = create_mobile (get_char_index (MOB_VNUM_DUMMY))) == NULL)
	{
	  chprintln (ch, "Opps, seems there was a problem creating a decoy!");
	  return;
	}

      replace_str (&dc->name, ch->name);
      replace_strf (&dc->short_descr, "%s's Decoy", ch->name);
      replace_strf (&dc->long_descr, "%s%s is here." NEWLINE, ch->name,
		    ch->pcdata->title);
      replace_str (&dc->description, ch->description);
      dc->affected_by = ch->affected_by;
      dc->level = ch->level;
      dc->sex = ch->sex;
      dc->race = ch->race;
      memcpy (dc->Class, ch->Class, MAX_MCLASS);
      dc->deity = ch->deity;
      dc->hit = ch->hit;
      dc->max_hit = ch->max_hit;
      char_to_room (dc, ch->in_room);
      dc->war = new_warlist ();
      dc->war->hit = ch->hit;
      dc->war->mana = ch->mana;
      dc->war->move = ch->move;
      dc->war->flags = WAR_DECOY;
      dc->war->ch = dc;
      dc->war->owner = ch;
      Link (dc->war, warlist, next, prev);
      chprintln (ch, "A decoy of yourself suddenly appears in the room.");
      return;
    }
  else if (!str_cmp (arg, "join"))
    {
      int iClass = -1;

      if (ch->fighting != NULL)
	{
	  chprintln (ch, "You're a little busy right now.");
	  return;
	}

      if (war_info.status == WAR_RUNNING)
	{
	  chprintln (ch, "The war has already started, your too late.");
	  return;
	}

      if (ch->level < war_info.min_level || ch->level > war_info.max_level)
	{
	  chprintln (ch, "Sorry, you can't join this war.");
	  return;
	}

      if (ch->war != NULL)
	{
	  chprintln (ch, "You are already in the war.");
	  return;
	}
      if (IsQuester (ch) || Gquester (ch))
	{
	  chprintln (ch, "What? And leave your quest?");
	  return;
	}

      if (IsSet (ch->in_room->room_flags, ROOM_NO_RECALL))
	{
	  chprintln (ch, "Something prevents you from leaving.");
	  return;
	}

      if (war_info.wartype == WAR_CLAN && !is_clan (ch))
	{
	  chprintln (ch, "You aren't in a clan, you can't join this war.");
	  return;
	}

      if (war_info.wartype == WAR_CLASS)
	{
	  if (IsRemort (ch))
	    {
	      if (NullStr (argument))
		{
		  cmd_syntax (ch, NULL, n_fun, "join <class>", NULL);
		  chprintln (ch, "Which class do you want to go to war as?");
		  return;
		}

	      if ((iClass = class_lookup (argument)) == -1)
		{
		  chprintln (ch, "That is not a class.");
		  return;
		}

	      if (!is_class (ch, iClass))
		{
		  chprintlnf (ch, "You are not part %s!",
			      ClassName (ch, iClass));
		  return;
		}
	    }
	}

      ch->war = new_warlist ();
      ch->war->hit = ch->hit;
      ch->war->mana = ch->mana;
      ch->war->move = ch->move;
      if (IsNPC (ch))
	ch->war->flags = WAR_DECOY;
      else
	ch->war->flags = 0;
      ch->war->ch = ch;
      Link (ch->war, warlist, next, prev);
      ch->war->Class = iClass;

      if (war_info.inwar > 2 && war_info.wartype != WAR_GENOCIDE)
	{
	  if (count_type_in_war (ch) > war_info.inwar - 2)
	    {
	      chprintlnf
		(ch,
		 "%s is already represented well enough in this war. Try again later.",
		 wartype_info (ch));
	      UnLink (ch->war, warlist, next, prev);
	      free_warlist (ch->war);
	      return;
	    }
	}

      if ((location = get_room_index (ROOM_VNUM_WAITROOM)) == NULL)
	{
	  chprintln (ch, "Arena is not yet completed, sorry.");
	  UnLink (ch->war, warlist, next, prev);
	  free_warlist (ch->war);
	  return;
	}
      else
	{
	  act ("$n goes to get $s ass whipped in war!", ch, NULL,
	       NULL, TO_ROOM);
	  char_from_room (ch);
	  char_to_room (ch, location);
	  war_info.inwar++;
	  announce (NULL, INFO_WAR, "%s joins the war!", warrior_status (ch));
	  act ("$n arrives to get $s ass whipped!", ch, NULL, NULL, TO_ROOM);
	  do_function (ch, &do_look, "auto");
	}
      return;
    }
  do_war (n_fun, ch, "");
  return;
}

bool
abort_war (void)
{
  WarData *cwl, *vwl;

  for (cwl = warlist_first; cwl != NULL; cwl = cwl->next)
    {
      if (IsSet (cwl->flags, WAR_KILLED | WAR_DECOY))
	continue;

      for (vwl = warlist_first; vwl != NULL; vwl = vwl->next)
	{
	  if (IsSet (vwl->flags, WAR_KILLED | WAR_DECOY))
	    continue;

	  if (!check_wartype_data (cwl->ch, vwl->ch))
	    return false;
	}
    }
  return true;
}

void
note_war (CharData * ch)
{
  Buffer *output;
  char sender[MIL], subject[MIL];
  WarData *wl;

  if (war_info.status != WAR_RUNNING)
    return;

  output = new_buf ();
  bprintln (output, "{WWAR INFO{g" NEWLINE "--------{x");
  bprintlnf (output, "{RStarted by  : {W%s",
	     GetStr (war_info.who, "AutoWar"));
  bprintlnf (output, "{RLevels      : {W%d - %d{x", war_info.min_level,
	     war_info.max_level);
  bprintlnf (output, "{RType        : {W%s war.{x",
	     wartype_name (war_info.wartype, false));
  bprintln (output, "{WWAR COMBATENTS{g" NEWLINE "--------------{x");
  for (wl = warlist_first; wl != NULL; wl = wl->next)
    {
      if (IsSet (wl->flags, WAR_DECOY) || !wl->ch)
	continue;

      bprintlnf (output, "{W%s{x", warrior_status (wl->ch));
    }
  bprintln (output, "{g--------------{x");
  switch (war_info.wartype)
    {
    case WAR_RACE:
    case WAR_CLASS:
      bprintlnf (output, "{WThe {R%s's{W won this war.{x", wartype_info (ch));
      break;
    default:
      bprintlnf (output, "{R%s{W won this war.{x", wartype_info (ch));
      break;
    }

  sprintf (subject, "War Info %s", str_time (current_time, -1, NULL));
  sprintf (sender, "%s", GetStr (war_info.who, "AutoWar"));
  make_note ("Games", sender, "All", subject, 15, buf_string (output));
  free_buf (output);
  return;
}

void
war_update (void)
{
  switch (war_info.status)
    {
    case WAR_OFF:
      if (war_info.timer > 0 && --war_info.timer == 0)
	auto_war ();
      break;

    case WAR_WAITING:
      if (--war_info.timer > 0)
	{
	  announce (NULL, INFO_WAR,
		    "%s left to join the war. (Levels %d - %d, %s War)",
		    intstr (war_info.timer, "minute"),
		    war_info.min_level, war_info.max_level,
		    wartype_name (war_info.wartype, false));
	}
      else
	{
	  if (war_info.inwar < 2)
	    {
	      end_war ();
	      announce (NULL, INFO_WAR, "Not enough people for war.");
	    }
	  else if (abort_war ())
	    {
	      announce (NULL, INFO_WAR, "Not enough %s for war.",
			wartype_name (war_info.wartype, true));
	      end_war ();
	    }
	  else
	    {
	      WarData *wl;

	      announce (NULL, INFO_WAR,
			"The battle begins! %s are fighting!",
			intstr (war_info.inwar, "player"));
	      war_info.timer =
		number_range (3 * war_info.inwar, 5 * war_info.inwar);
	      war_info.status = WAR_RUNNING;
	      for (wl = warlist_first; wl != NULL; wl = wl->next)
		{
		  vnum_t randm = number_range (ROOM_VNUM_WAR_START,

					       ROOM_VNUM_WAR_END);

		  char_from_room (wl->ch);
		  char_to_room (wl->ch, get_room_index (randm));
		  do_function (wl->ch, &do_look, "auto");
		}
	    }
	}
      break;

    case WAR_RUNNING:
      if (war_info.inwar == 0)
	{
	  end_war ();
	  announce (NULL, INFO_WAR, "No one left in the war");
	  return;
	}

      switch (war_info.timer)
	{
	case 0:
	  end_war ();
	  announce (NULL, INFO_WAR, "Time has run out!");
	  return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
	  announce (NULL, INFO_WAR,
		    "%s remaining in the war.",
		    intstr (war_info.timer, "minute"));
	default:
	  war_info.timer--;
	  break;
	}
      break;
    }
}

void
check_war (CharData * ch, CharData * victim)
{
  WarData *wl, *wl_next;

  if (war_info.status == WAR_OFF)
    return;

  if (!InWar (ch) || !InWar (victim))
    return;

  war_info.inwar--;
  stop_fighting (victim, true);
  stop_fighting (ch, true);
  if (IsNPC (victim))
    {
      act ("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
      UnLink (victim->war, warlist, next, prev);
      free_warlist (victim->war);
      extract_char (victim, true);
      return;
    }
  char_from_room (victim);
  char_to_room (victim, get_room_index (ROOM_VNUM_TEMPLE));
  for (wl = warlist_first; wl; wl = wl_next)
    {
      wl_next = wl->next;

      if (!IsSet (wl->flags, WAR_DECOY))
	continue;

      if (wl->owner == victim)
	{
	  act ("$n dissapears suddenly.", wl->ch, NULL, NULL, TO_ROOM);
	  UnLink (wl, warlist, next, prev);
	  free_warlist (wl);
	}
    }
  chprintln (ch, NULL);
  victim->hit = victim->war->hit;
  victim->mana = victim->war->mana;
  victim->move = victim->war->move;
  SetBit (victim->war->flags, WAR_KILLED);
  update_pos (victim);
  do_function (victim, &do_look, "auto");
  announce (NULL, INFO_WAR, "%s was killed in combat by %s!{x",
	    GetName (victim), GetName (ch));

  if (abort_war ())
    {
      int reward;
      int qreward;

      switch (war_info.wartype)
	{
	case WAR_RACE:
	case WAR_CLASS:
	  announce (NULL, INFO_WAR, "The %s's have won the war!",
		    wartype_info (ch));
	  break;
	default:
	  announce (NULL, INFO_WAR, "%s has won the war!", wartype_info (ch));
	  break;
	}
      note_war (ch);
      reward = number_range (500, 1500);
      qreward = number_range (50, 150);
      for (wl = warlist_first; wl != NULL; wl = wl->next)
	{
	  if (IsSet (wl->flags, WAR_DECOY))
	    continue;

	  if (check_wartype_data (wl->ch, ch))
	    {
	      add_cost (wl->ch, reward, VALUE_GOLD);
	      qreward = add_qp (wl->ch, qreward);
	      chprintf (wl->ch,
			"You recieve %d gold and %d questpoints from the war tribunal!",
			reward, qreward);
	    }
	}
      end_war ();
      return;
    }
  return;
}

bool
is_safe_war (CharData * ch, CharData * wch)
{
  if (war_info.status == WAR_OFF)
    return false;

  if (!InWar (ch) || !InWar (wch))
    return false;

  return check_wartype_data (ch, wch);
}

void
war_talk (CharData * ch, const char *argument)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      chprintlnf (ch,
		  "Wartalk about what?" NEWLINE
		  "Use '%s war' to toggle this channel.", cmd_name (do_info));
      return;
    }

  chprintlnf (ch, "{Y({RWarTalk{Y) {gYou drum: %s{x", argument);

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      CharData *victim;

      if (d->connected == CON_PLAYING && (victim = CH (d)) != ch
	  && !IsSet (victim->info_settings, INFO_WAR)
	  && !IsSet (victim->comm, COMM_QUIET)
	  && !is_ignoring (ch, victim->name, IGNORE_CHANNELS))
	{
	  chprintlnf (victim, "{Y({RWarTalk{Y) {g%s drums: %s{x",
		      Pers (ch, victim), argument);
	}
    }
  return;
}

void
extract_war (CharData * ch)
{
  if (war_info.status != WAR_OFF && ch->war != NULL)
    {
      war_info.inwar--;
      if (war_info.status == WAR_RUNNING)
	{
	  if (war_info.inwar == 0 || war_info.inwar == 1)
	    {
	      announce (ch, INFO_WAR, "$n has left. War over.");
	      end_war ();
	    }
	  if (abort_war ())
	    {
	      announce (ch, INFO_WAR, "$n has left. War over.");
	      end_war ();
	    }
	  else
	    {
	      announce (ch, INFO_WAR,
			"$n has left. %s in the war.",
			intstr (war_info.inwar, "player"));
	    }
	}
      char_from_room (ch);
      char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
      UnLink (ch->war, warlist, next, prev);
      free_warlist (ch->war);
    }
}
