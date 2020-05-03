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
#include "data_table.h"

DataTable stat_data_table[] = {
  {"name", FIELD_STRING, (void *) &stat_zero.name, NULL, NULL},
  {"stats", FIELD_LONG_ARRAY, (void *) &stat_zero.gamestat,
   (const void *) MAX_GAMESTAT, (const void *) 0},
  {"version", FIELD_INT, (void *) &stat_zero.version, NULL, NULL},
  {NULL, (field_t) - 1, NULL, NULL, NULL}
};

TableSave_Fun (rw_stat_data)
{
  rw_list (type, STAT_FILE, StatData, stat);
}

void
update_statlist (CharData * ch, bool pdelete)
{
  StatData *prev;
  StatData *curr;
  int i;

  if (IsNPC (ch) || IsImmortal (ch))
    return;

  prev = NULL;

  for (curr = stat_first; curr != NULL; curr = prev)
    {
      prev = curr->next;

      if (!str_cmp (ch->name, curr->name))
	{
	  if (curr->version < mud_info.stats.version)
	    {
	      memset (curr->gamestat, 0, MAX_GAMESTAT);
	      memset (ch->pcdata->gamestat, 0, MAX_GAMESTAT);
	    }
	  UnLink (curr, stat, next, prev);
	  free_stat (curr);
	}
    }
  if (pdelete || IsNPC (ch) || IsImmortal (ch))
    {
      rw_stat_data (act_write);
      return;
    }

  curr = new_stat ();
  curr->name = str_dup (ch->name);
  curr->version = mud_info.stats.version;
  for (i = 0; i < MAX_GAMESTAT; i++)
    curr->gamestat[i] = GetStat (ch, i);

  Link (curr, stat, next, prev);
  rw_stat_data (act_write);
  return;
}


const char *
print_stat (CharData * ch, const char *head, long value)
{
  return FORMATF ("{W%-19.19s {w[{R%10.10ld{w]", head, value);
}

Do_Fun (do_showstats)
{
  int option;
  char arg[MIL];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "      {ROPTIONS AVAILABLE:{x");
      chprintln (ch, "      {G0{x - Show general {n stats. (gstats)");
      chprintln (ch, "      {G1{x - Ranking of Player Killers (pkills)");
      chprintln (ch, "      {G2{x - Ranking of Player Deaths (pdeaths)");
      chprintln (ch, "      {G3{x - Ranking of Mob Kills (mkills)");
      chprintln (ch, "      {G4{x - Ranking of Mob Deaths (mdeaths)");
      chprintln (ch, "      {G5{x - Personal Rankings (personal)");
      if (IsImmortal (ch))
	chprintln (ch, "      {Gdelete <name>{x - deletes from statlist");
      return;
    }
  option = atoi (arg);

  if (!str_cmp (arg, "delete") && IsImmortal (ch))
    {
      StatData *prev = NULL;
      StatData *curr = NULL;
      bool found = false;

      for (curr = stat_first; curr != NULL; curr = prev)
	{
	  prev = curr->next;

	  if (!str_cmp (argument, curr->name))
	    {
	      UnLink (curr, stat, next, prev);
	      free_stat (curr);
	      rw_stat_data (act_write);
	      found = true;
	    }
	}
      if (!found)
	chprintlnf (ch, "Error deleting %s.", argument);
    }
  else if (option == 0 || !str_prefix (arg, "gstats"))
    {
      Column *Cd;
      Buffer *b;

      Cd = new_column ();
      b = new_buf ();
      set_cols (Cd, ch, 2, COLS_BUF, b);

      bprintln (b,
		stringf (ch, 0, Center, "{R-{r=",
			 "{C[ %s Stats ]", mud_info.name));
      print_cols (Cd, print_stat (ch, "Logins", mud_info.stats.logins));
      print_cols (Cd, print_stat (ch, "Quests", mud_info.stats.quests));
      print_cols (Cd,
		  print_stat (ch, "Quests Complete",
			      mud_info.stats.qcomplete));
      print_cols (Cd, print_stat (ch, "Levels", mud_info.stats.levels));
      print_cols (Cd, print_stat (ch, "Newbies", mud_info.stats.newbies));
      print_cols (Cd, print_stat (ch, "Deletions", mud_info.stats.deletions));
      print_cols (Cd,
		  print_stat (ch, "Mob Deaths", mud_info.stats.mobdeaths));
      print_cols (Cd, print_stat (ch, "Auctions", mud_info.stats.auctions));
      print_cols (Cd,
		  print_stat (ch, "Auctions Sold", mud_info.stats.aucsold));
      print_cols (Cd, print_stat (ch, "Player Deaths", mud_info.stats.pdied));
      print_cols (Cd, print_stat (ch, "Player Kills", mud_info.stats.pkill));
      print_cols (Cd, print_stat (ch, "Notes", mud_info.stats.notes));
      print_cols (Cd, print_stat (ch, "Remorts", mud_info.stats.remorts));
      print_cols (Cd, print_stat (ch, "Wars", mud_info.stats.wars));
      print_cols (Cd,
		  print_stat (ch, "Global Quests", mud_info.stats.gquests));
      print_cols (Cd,
		  print_stat (ch, "Connections", mud_info.stats.connections));
      print_cols (Cd,
		  print_stat (ch, "Connects this Boot",
			      mud_info.stats.boot_connects));
      print_cols (Cd,
		  print_stat (ch, "Online Record", mud_info.stats.online));
      print_cols (Cd,
		  print_stat (ch, "Web Hits", mud_info.stats.web_requests));
      print_cols (Cd,
		  print_stat (ch, "Channel Messages",
			      mud_info.stats.chan_msgs));
      cols_nl (Cd);
      bprintln (b,
		stringf (ch, 0, Center, "{R-{r=",
			 "{C[ Stats since: %s ]",
			 str_time (mud_info.stats.lastupdate,
				   GetTzone (ch), NULL)));
      sendpage (ch, buf_string (b));
      free_buf (b);
      free_column (Cd);
      return;
    }
  else if (option == 1 || !str_prefix (arg, "pkills"))
    show_game_stats (ch, PK_KILLS);
  else if (option == 3 || !str_prefix (arg, "mkills"))
    show_game_stats (ch, MOB_KILLS);
  else if (option == 2 || !str_prefix (arg, "pdeaths"))
    show_game_stats (ch, PK_DEATHS);
  else if (option == 4 || !str_prefix (arg, "mdeaths"))
    show_game_stats (ch, MOB_DEATHS);
  else if (option == 5 || !str_prefix (arg, "personal"))
    {
      StatData *curr;
      Column Cd;
      Buffer b;
      int i = 0;
      const char *stat_self[MAX_GAMESTAT] = {
	"PLAYER KILLS",
	"MOB KILLS",
	"PK DEATHS",
	"MOB DEATHS",
      };

      update_statlist (ch, false);

      for (curr = stat_first; curr != NULL; curr = curr->next)
	if (!str_cmp (ch->name, curr->name))
	  break;

      if (!curr)
	{
	  chprintln (ch, "You have no stats yet.");
	  return;
	}

      set_cols (&Cd, ch, 2, COLS_BUF, &b);

      bprintln (&b, stringf (ch, 0, Center, "{r-{R=",
			     "{C[ RANKING OF PERSONAL BESTS ]"));
      for (i = 0; i < MAX_GAMESTAT; i++)
	{
	  print_cols (&Cd, "{G%2d{w){W %s{x", i + 1,
		      print_stat (ch, stat_self[i], curr->gamestat[i]));
	}
      cols_nl (&Cd);
      bprintln (&b, draw_line (ch, "{r-{R=", 0));
      sendpage (ch, buf_string (&b));
      return;
    }
  else
    do_showstats (n_fun, ch, "");

  return;
}

int compare_type;

int
compare_stats (const void *v1, const void *v2)
{
  StatData *stat1 = *(StatData **) v1;
  StatData *stat2 = *(StatData **) v2;

  if (!stat2)
    return 1;
  if (!stat1)
    return 2;

  return stat2->gamestat[compare_type] - stat1->gamestat[compare_type];
}

void
show_game_stats (CharData * ch, int type)
{
  StatData *curr;
  Buffer *output;
  StatData **top;
  Column *Cd, c;
  int count, loop;
  bool found = false;
  const char *stat_name[MAX_GAMESTAT] = {
    "PLAYER KILLERS",
    "MOB KILLERS",
    "PK DEATHS",
    "MOB DEATHS",
  };

  output = new_buf ();
  Cd = &c;
  set_cols (Cd, ch, 2, COLS_BUF, output);
  count = 0;
  compare_type = type;

  alloc_mem (top, StatData *, top_stat);

  bprintln (output, stringf (ch, 0, Center, "{R-{r=",
			     "{C[ RANKING OF %s ]", stat_name[type]));
  loop = 0;
  loop = 0;
  for (curr = stat_first; curr != NULL; curr = curr->next)
    {
      top[count] = curr;
      count++;
      found = true;
    }

  if (found)
    {
      qsort (top, count, sizeof (*top), compare_stats);

      for (loop = 0; loop < count; loop++)
	{
	  if (loop >= 50)
	    break;

	  print_cols (Cd,
		      "{G%2d{w){W %-20s {w[{R%8ld{W]{x", loop + 1,
		      top[loop]->name, top[loop]->gamestat[type]);
	}
    }
  if (!found)
    bprintln (output, "No one found yet.");
  else
    cols_nl (Cd);
  bprintln (output, draw_line (ch, "{R-{r=", 0));
  sendpage (ch, buf_string (output));
  free_mem (top);
  free_buf (output);
  free_column (Cd);
  return;
}
