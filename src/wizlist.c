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
#include "interp.h"
#include "globals.h"
#include "data_table.h"
#include "olc.h"

DataTable wiz_data_table[] = {
  DTE (STRING, wiz, name, NULL, NULL, str, NULL),
  DTE (STRING, wiz, email, NULL, NULL, str, NULL),
  DTE (STRING, wiz, jobs, NULL, NULL, str, NULL),
  DTE (INT, wiz, level, NULL, NULL, int, NULL),
  DTE_END
};

TableSave_Fun (rw_wiz_data)
{
  rw_list (type, WIZ_FILE, WizData, wiz);
}

void
add_to_wizlist (WizData * wiz)
{
  WizData *tmp;

  for (tmp = wiz_first; tmp; tmp = tmp->next)

    {
      if (wiz->level > tmp->level)

	{
	  Insert (wiz, tmp, wiz, next, prev);
	  break;
	}
    }
  if (!tmp)
    Link (wiz, wiz, next, prev);
}

Do_Fun (do_wizlist)
{
  char arg1[MIL];
  char arg2[MIL];
  WizData *curr;
  WizData *next;
  bool found = false;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (!NullStr (arg1) && (ch->level >= MAX_LEVEL - 1))
    {
      if (!str_prefix (arg1, "add"))
	{
	  int level;

	  if (!is_number (arg2) || NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "add <level> <name>", NULL);
	      return;
	    }
	  level = atoi (arg2);
	  if ((level < LEVEL_IMMORTAL) || (level > MAX_LEVEL))

	    {
	      cmd_syntax (ch, NULL, n_fun, "add <level> <name>", NULL);
	      return;
	    }
	  curr = new_wiz ();
	  replace_str (&curr->name, capitalize (argument));
	  replace_str (&curr->email, "-Use Personal Board-");
	  replace_str (&curr->jobs, "Nothing yet.");
	  curr->level = level;
	  add_to_wizlist (curr);
	  rw_wiz_data (act_write);
	  return;
	}
      if (!str_prefix (arg1, "delete"))
	{
	  if (NullStr (arg2))
	    {
	      cmd_syntax (ch, NULL, n_fun, "delete <name>", NULL);
	      return;
	    }
	  for (curr = wiz_first; curr != NULL; curr = next)
	    {
	      next = curr->next;
	      if (!str_cmp (arg2, curr->name))
		{
		  found = true;
		  UnLink (curr, wiz, next, prev);
		  free_wiz (curr);
		}
	    }
	  if (found)
	    {
	      chprintlnf (ch, "%s deleted.", capitalize (arg2));
	      rw_wiz_data (act_write);
	    }
	  else
	    chprintlnf (ch, "%s not found.", capitalize (arg2));
	  return;
	}
      if (!str_prefix (arg1, "email"))
	{
	  if (NullStr (arg2) || NullStr (argument))
	    {
	      cmd_syntax (NULL, n_fun, "email <name> <email>", NULL);
	      return;
	    }
	  for (curr = wiz_first; curr != NULL; curr = curr->next)
	    {
	      if (!str_cmp (arg2, curr->name))
		{
		  replace_str (&curr->email, argument);
		  rw_wiz_data (act_write);
		  break;
		}
	    }
	  if (curr)
	    chprintlnf (ch, "%s's email changed to %s.", curr->name,
			curr->email);
	  else
	    chprintlnf (ch, "%s not found.", capitalize (arg2));
	  return;
	}
      if (!str_prefix (arg1, "jobs"))
	{
	  if (NullStr (arg2) || NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "jobs <name> <jobs>", NULL);
	      return;
	    }
	  for (curr = wiz_first; curr != NULL; curr = curr->next)
	    {
	      if (!str_cmp (arg2, curr->name))
		{
		  replace_str (&curr->jobs, argument);
		  rw_wiz_data (act_write);
		  break;
		}
	    }
	  if (curr)
	    chprintlnf (ch, "%s's jobs are now %s.", curr->name, curr->jobs);
	  else
	    chprintlnf (ch, "%s not found.", capitalize (arg2));
	  return;
	}
      cmd_syntax (ch, NULL, n_fun, "delete <name>", "add <level> <name>",
		  "email <name <addr>", "jobs <name> <descr>", NULL);
      return;
    }
  if (wiz_first == NULL)
    {
      chprintln (ch, "No immortals listed at this time.");
      return;
    }
  chprintln (ch, "\n\r                      {=Wo{x");
  chprintln (ch, "                     {=W/{x");
  chprintln
    (ch, "                    {=W/{=Y------------------------------------{x");
  chprintln (ch,
	     "  {=W========={=Go{=W[[[{1O{=W][[[{=Y< @ |-|.|`|>,<>`|.  <>|''  < <> |>,|]>>========={x");
  chprintln (ch,
	     "                    {=W\\{=Y------------------------------------{x");
  chprintln (ch, "                     {=W\\{x");
  chprintln (ch, "                      {=Wo{x\n\r");
  chprintln (ch, "                        {CIMMORTALS OF {N{x" NEWLINE);
  for (curr = wiz_first; curr != NULL; curr = curr->next)
    {
      chprintlnf
	(ch,
	 "                  {w[{R%3d{w]{W %s ({w%s{W) in charge of {w%s{W.{x",
	 curr->level, str_width (10, curr->name), curr->email, curr->jobs);
      found = true;
    }
  if (!found)
    chprintln (ch, "                   {WNo Immortals Found At This Time.{x");
  chprintln (ch, NULL);
  return;
}

void
update_wizlist (CharData * ch, int level)
{
  WizData *next;
  WizData *curr;
  char jobs[MIL];
  char email[MIL];

  if (IsNPC (ch))
    return;

  for (curr = wiz_first; curr != NULL; curr = next)
    {
      next = curr->next;
      if (curr->level < LEVEL_IMMORTAL || !str_cmp (ch->name, curr->name))
	{
	  strcpy (jobs, curr->jobs);
	  strcpy (email, curr->email);
	  UnLink (curr, wiz, next, prev);
	  free_wiz (curr);
	  break;
	}
    }
  if (level < LEVEL_IMMORTAL)
    {
      rw_wiz_data (act_write);
      return;
    }
  curr = new_wiz ();
  replace_str (&curr->name, ch->name);
  replace_str (&curr->email, email);
  replace_str (&curr->jobs, jobs);
  curr->level = level;
  add_to_wizlist (curr);
  rw_wiz_data (act_write);
  return;
}
