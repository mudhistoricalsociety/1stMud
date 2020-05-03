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


void
substitute_alias (Descriptor * d, char *argument)
{
  CharData *ch;
  char buf[MAX_STRING_LENGTH],
    prefix[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH];
  const char *point;
  int alias;

  ch = d->original ? d->original : d->character;


  if (!NullStr (ch->prefix) && str_prefix ("prefix", argument))
    {
      if (strlen (ch->prefix) + strlen (argument) > MAX_INPUT_LENGTH - 2)
	chprintln (ch, "Line to long, prefix not processed.");
      else
	{
	  sprintf (prefix, "%s %s", ch->prefix, argument);
	  argument = prefix;
	}
    }

  if (IsNPC (ch) || ch->pcdata->alias[0] == NULL ||
      !str_prefix ("alias", argument) || !str_prefix ("una", argument)
      || !str_prefix ("prefix", argument))
    {
      if (!run_olc_editor (d, argument))
	interpret (d->character, argument);
      return;
    }

  strcpy (buf, argument);

  for (alias = 0; alias < MAX_ALIAS; alias++)
    {
      if (ch->pcdata->alias[alias] == NULL)
	break;

      if (!str_prefix (ch->pcdata->alias[alias], argument))
	{
	  point = one_argument (argument, name);
	  if (!str_cmp (ch->pcdata->alias[alias], name))
	    {
	      buf[0] = '\0';
	      strcat (buf, ch->pcdata->alias_sub[alias]);
	      if (point[0])
		{
		  strcat (buf, " ");
		  strcat (buf, point);
		}
	      if (strlen (buf) > MAX_INPUT_LENGTH - 1)
		{
		  chprintln (ch, "Alias substitution too long. Truncated.");
		  buf[MAX_INPUT_LENGTH - 1] = '\0';
		}
	      break;
	    }
	}
    }
  if (!run_olc_editor (d, buf))
    interpret (d->character, buf);
}

Do_Fun (do_alias)
{
  CharData *rch;
  char arg[MAX_INPUT_LENGTH];
  int pos;
  CmdData *cmd;

  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;

  if (IsNPC (rch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {

      if (rch->pcdata->alias[0] == NULL)
	{
	  chprintln (ch, "You have no aliases defined.");
	  return;
	}
      chprintln (ch, "Your current aliases are:");

      for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	  if (rch->pcdata->alias[pos] == NULL ||
	      rch->pcdata->alias_sub[pos] == NULL)
	    break;

	  chprintlnf (ch, "    %s:  %s", rch->pcdata->alias[pos],
		      rch->pcdata->alias_sub[pos]);
	}
      return;
    }

  if (!str_prefix ("una", arg) || !str_cmp ("alias", arg))
    {
      chprintln (ch, "Sorry, that word is reserved.");
      return;
    }

  if (NullStr (argument))
    {
      for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	  if (rch->pcdata->alias[pos] == NULL ||
	      rch->pcdata->alias_sub[pos] == NULL)
	    break;

	  if (!str_cmp (arg, rch->pcdata->alias[pos]))
	    {
	      chprintlnf (ch, "%s aliases to '%s'.",
			  rch->pcdata->alias[pos],
			  rch->pcdata->alias_sub[pos]);
	      return;
	    }
	}

      chprintln (ch, "That alias is not defined.");
      return;
    }

  if ((cmd = cmd_lookup (argument)) != NULL
      && IsSet (cmd->flags, CMD_NOALIAS))
    {
      chprintln (ch, "That shall not be done!");
      return;
    }

  for (pos = 0; pos < MAX_ALIAS; pos++)
    {
      if (rch->pcdata->alias[pos] == NULL)
	break;

      if (!str_cmp (arg, rch->pcdata->alias[pos]))
	{
	  replace_str (&rch->pcdata->alias_sub[pos], argument);
	  chprintlnf (ch, "%s is now realiased to '%s'.", arg, argument);
	  return;
	}
    }

  if (pos >= MAX_ALIAS)
    {
      chprintln (ch, "Sorry, you have reached the alias limit.");
      return;
    }


  rch->pcdata->alias[pos] = str_dup (arg);
  rch->pcdata->alias_sub[pos] = str_dup (argument);
  chprintlnf (ch, "%s is now aliased to '%s'.", arg, argument);
}

Do_Fun (do_unalias)
{
  CharData *rch;
  char arg[MAX_INPUT_LENGTH];
  int pos;
  bool found = false;

  if (ch->desc == NULL)
    rch = ch;
  else
    rch = ch->desc->original ? ch->desc->original : ch;

  if (IsNPC (rch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Unalias what?");
      return;
    }

  for (pos = 0; pos < MAX_ALIAS; pos++)
    {
      if (rch->pcdata->alias[pos] == NULL)
	break;

      if (found)
	{
	  rch->pcdata->alias[pos - 1] = rch->pcdata->alias[pos];
	  rch->pcdata->alias_sub[pos - 1] = rch->pcdata->alias_sub[pos];
	  rch->pcdata->alias[pos] = NULL;
	  rch->pcdata->alias_sub[pos] = NULL;
	  continue;
	}

      if (!str_cmp (arg, rch->pcdata->alias[pos]))
	{
	  chprintln (ch, "Alias removed.");
	  free_string (rch->pcdata->alias[pos]);
	  free_string (rch->pcdata->alias_sub[pos]);
	  rch->pcdata->alias[pos] = NULL;
	  rch->pcdata->alias_sub[pos] = NULL;
	  found = true;
	}
    }

  if (!found)
    chprintln (ch, "No alias of that name to remove.");
}
