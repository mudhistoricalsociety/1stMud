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
#include "data_table.h"
#include "recycle.h"
#include "tables.h"

Proto (bool check_social, (CharData *, char *, const char *));
Proto (bool check_disabled, (CharData *, CmdData *));



Do_Fun (do_null)
{
  chprintln (ch, "This command is nullified, please notify the immortals.");
  return;
}

bool
cmd_level_ok (CharData * ch, CmdData * cmd)
{
  return (get_trust (ch) >= cmd->level);
}


void
interpret (CharData * ch, const char *argument)
{
  char command[MAX_INPUT_LENGTH];
  char logline[MAX_INPUT_LENGTH];
  CmdData *cmd;


  while (isspace (*argument))
    argument++;
  if (NullStr (argument))
    return;


  RemBit (ch->affected_by, AFF_HIDE);


  if (!IsNPC (ch) && IsSet (ch->act, PLR_FREEZE))
    {
      chprintln (ch, "You're totally frozen!");
      return;
    }

  crash_info.status = CRASH_PRE_PROCESSING;
  strcpy (logline, argument);
  crash_info.desc = ch->desc;
  strcpy (crash_info.shrt_cmd, logline);
  sprintf (crash_info.long_cmd, "[%5ld] %s in [%5ld] %s: %s",
	   IsNPC (ch) ? ch->pIndexData->vnum : 0, IsNPC (ch) ?
	   ch->short_descr : ch->name, ch->in_room ?
	   ch->in_room->vnum : 0, ch->in_room ? ch->in_room->name :
	   "(not in a room)", logline);


  if (!isalpha (argument[0]) && !isdigit (argument[0]))
    {
      command[0] = argument[0];
      command[1] = '\0';
      argument++;
      while (isspace (*argument))
	argument++;
    }
  else
    {
      argument = one_argument (argument, command);
    }


  for (cmd = command_hash[tolower (command[0]) % MAX_CMD_HASH]; cmd;
       cmd = cmd->next_hash)
    {
      if (IsSet (cmd->flags, CMD_NOPREFIX) ? !str_cmp (command, cmd->name)
	  : !str_prefix (command, cmd->name) && cmd_level_ok (ch, cmd))
	break;
    }


  if (cmd)
    {
      if (ch->master && IsSet (cmd->flags, CMD_NO_ORDER))
	{
	  act ("You can't order $N to do that!", ch->master, NULL, ch,
	       TO_CHAR);
	  return;
	}

      if (cmd->log == LOG_NEVER)
	strcpy (logline, "");

      if ((!IsNPC (ch) && IsSet (ch->act, PLR_LOG))
	  || IsSet (mud_info.mud_flags, MUD_LOGALL) || cmd->log == LOG_ALWAYS)
	{
	  new_wiznet (ch, logline, WIZ_SECURE, true, get_trust (ch),
		      "Log $N: $t");
	}

    }

  if (ch->desc != NULL && ch->desc->snoop_by != NULL)
    {
      d_printlnf (ch->desc->snoop_by, "> %s", ch->name, logline);
    }
  if (!cmd)
    {

      if (!check_social (ch, command, argument)
#ifndef DISABLE_I3
	  && !I3_command_hook (ch, command, argument)
#endif
	)
	{
	  char *const huh_message[] = {
	    "Huh?",
	    "Pardon?",
	    "What is command '%s'?",
	    "Input error.",
	    "Try again.",
	    "I do not understand.",
	    "Type commands for a list of commands."
	  };

	  chprintlnf (ch, huh_message[number_range (0,
						    (sizeof (huh_message) /
						     sizeof (huh_message[0]) -
						     1))], command);
	}
      return;
    }
  else if (check_disabled (ch, cmd))
    {
      chprintlnf (ch, "%s has been temporarily disabled.", Upper (cmd->name));
      return;
    }


  if (ch->position < cmd->position)
    {
      switch (ch->position)
	{
	case POS_DEAD:
	  chprintln (ch, "Lie still; you are DEAD.");
	  break;

	case POS_MORTAL:
	case POS_INCAP:
	  chprintln (ch, "You are hurt far too bad for that.");
	  break;

	case POS_STUNNED:
	  chprintln (ch, "You are too stunned to do that.");
	  break;

	case POS_SLEEPING:
	  chprintln (ch, "In your dreams, or what?");
	  break;

	case POS_RESTING:
	  chprintln (ch, "Nah... You feel too relaxed...");
	  break;

	case POS_SITTING:
	  chprintln (ch, "Better stand up first.");
	  break;

	case POS_FIGHTING:
	  chprintln (ch, "No way!  You are still fighting!");
	  break;

	default:
	  break;
	}
      return;
    }

  crash_info.status = CRASH_LIKELY;


  (*cmd->do_fun) (cmd->name, ch, argument);

  crash_info.status = CRASH_POST_PROCESSING;

  strcat (crash_info.shrt_cmd, " (Finished)");
  strcat (crash_info.long_cmd, " (Finished)");

  crash_info.status = CRASH_UNLIKELY;

  tail_chain ();
  return;
}


void
do_function (CharData * ch, Do_F * do_fun, const char *argument)
{
  const char *command_string;


  command_string = str_dup (argument);


  (*do_fun) (cmd_name (do_fun), ch, command_string);


  free_string (command_string);
}

SocialData *
find_social (const char *command)
{
  SocialData *social;
  int hash;

  if (tolower (command[0]) < 'a' || tolower (command[0]) > 'z')
    hash = 0;
  else
    hash = (tolower (command[0]) - 'a') + 1;

  for (social = social_hash[hash]; social; social = social->next_hash)
    {
      if (!str_prefix (command, social->name))
	return social;
    }
  return NULL;
}

bool
check_social (CharData * ch, char *command, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  SocialData *cmd;

  if ((cmd = find_social (command)) == NULL)
    return false;

  if (!IsNPC (ch) && IsSet (ch->comm, COMM_NOEMOTE))
    {
      chprintln (ch, "You are anti-social!");
      return true;
    }

  switch (ch->position)
    {
    case POS_DEAD:
      chprintln (ch, "Lie still; you are DEAD.");
      return true;

    case POS_INCAP:
    case POS_MORTAL:
      chprintln (ch, "You are hurt far too bad for that.");
      return true;

    case POS_STUNNED:
      chprintln (ch, "You are too stunned to do that.");
      return true;

    case POS_SLEEPING:

      if (!str_cmp (cmd->name, "snore"))
	break;
      chprintln (ch, "In your dreams, or what?");
      return true;
    default:
      break;
    }

  one_argument (argument, arg);
  victim = NULL;
  if (NullStr (arg))
    {
      act (cmd->others_no_arg, ch, NULL, victim, TO_ROOM);
      act (cmd->char_no_arg, ch, NULL, victim, TO_CHAR);
    }
  else if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
    }
  else if (victim == ch)
    {
      act (cmd->others_auto, ch, NULL, victim, TO_ROOM);
      act (cmd->char_auto, ch, NULL, victim, TO_CHAR);
    }
  else
    {
      if (is_ignoring (victim, ch->name, IGNORE_SOCIALS))
	{
	  act ("$N is ignoring socials from you.", ch, NULL, victim, TO_CHAR);
	  return true;
	}
      act (cmd->others_found, ch, NULL, victim, TO_NOTVICT);
      act (cmd->char_found, ch, NULL, victim, TO_CHAR);
      act (cmd->vict_found, ch, NULL, victim, TO_VICT);

      if (!IsNPC (ch) && IsNPC (victim) &&
	  !IsAffected (victim, AFF_CHARM) && IsAwake (victim) &&
	  victim->desc == NULL)
	{
	  switch (number_bits (4))
	    {
	    case 0:

	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
	      act (cmd->others_found, victim, NULL, ch, TO_NOTVICT);
	      act (cmd->char_found, victim, NULL, ch, TO_CHAR);
	      act (cmd->vict_found, victim, NULL, ch, TO_VICT);
	      break;

	    case 9:
	    case 10:
	    case 11:
	    case 12:
	      act ("$n slaps $N.", victim, NULL, ch, TO_NOTVICT);
	      act ("You slap $N.", victim, NULL, ch, TO_CHAR);
	      act ("$n slaps you.", victim, NULL, ch, TO_VICT);
	      break;
	    }
	}
    }

  return true;
}


bool
is_number (const char *arg)
{

  if (*arg == '\0')
    return false;

  if (*arg == '+' || *arg == '-')
    arg++;

  for (; *arg != '\0'; arg++)
    {
      if (!isdigit (*arg))
	return false;
    }

  return true;
}

static unsigned int
x_argument (const char *argument, char arg[MAX_INPUT_LENGTH], char c)
{
  char *p;
  char *q;
  int number;

  p = strchr (argument, c);
  if (p == NULL)
    {
      strcpy (arg, argument);
      return 1;
    }

  number = strtoul (argument, &q, 0);
  if (q != p)
    number = 0;
  strncpy (arg, p + 1, MAX_INPUT_LENGTH);
  return number;
}


unsigned int
number_argument (const char *argument, char *arg)
{
  return x_argument (argument, arg, '.');
}


unsigned int
mult_argument (const char *argument, char *arg)
{
  return x_argument (argument, arg, '*');
}


const char *
one_argument (const char *argument, char *arg_first)
{
  char cEnd;

  while (isspace (*argument))
    argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;

  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *arg_first = tolower (*argument);
      arg_first++;
      argument++;
    }
  *arg_first = '\0';

  while (isspace (*argument))
    argument++;

  return argument;
}


Do_Fun (do_commands)
{
  CmdData *cmd;
  int i = 0;
  Buffer *b;
  Column Cd;
  int cat = NO_FLAG;

  if (!NullStr (argument)
      && (cat = (int) flag_value (cmd_categories, argument)) == NO_FLAG)
    {
      cmd_syntax (ch, NULL, n_fun, "<flag>", NULL);
      chprintln (ch, "Valid flags are:");
      show_flags (ch, cmd_categories);
      return;
    }

  b = new_buf ();
  set_cols (&Cd, ch, 6, COLS_BUF, b);

  for (cmd = cmd_first_sorted; cmd; cmd = cmd->next_sort)
    {
      if (cmd->level <= LEVEL_HERO && cmd_level_ok (ch, cmd) &&
	  cmd->category != CMDCAT_NOSHOW && (cat == NO_FLAG ||
					     cmd->category == (cmd_cat) cat))
	{
	  i++;
	  print_cols (&Cd, "%3d. %s", i, cmd->name);
	}
    }

  cols_nl (&Cd);

  if (i == 0)
    {
      if (cat != NO_FLAG)
	bprintlnf (b, "No commands found in the '%s' category.",
		   flag_string (cmd_categories, cat));
      else
	bprintln (b, "No commands found.");
    }

  sendpage (ch, buf_string (b));
  free_buf (b);
  return;
}

Do_Fun (do_wizhelp)
{
  CmdData *cmd;
  int i = 0;
  Column Cd;
  Buffer *b;

  b = new_buf ();
  set_cols (&Cd, ch, 6, COLS_BUF, b);

  for (cmd = cmd_first_sorted; cmd; cmd = cmd->next_sort)
    {
      if (cmd->level >= LEVEL_HERO && cmd_level_ok (ch, cmd) &&
	  cmd->category != CMDCAT_NOSHOW)
	{
	  i++;
	  print_cols (&Cd, "%3d. %s", i, cmd->name);
	}
    }

  cols_nl (&Cd);
  sendpage (ch, buf_string (b));
  free_buf (b);
  return;
}



Do_Fun (do_disable)
{
  CmdData *i;
  DisabledData *p;
  char arg[MIL];

  if (IsNPC (ch))
    {
      chprintln (ch, "RETURN first.");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      if (!disabled_first)
	{
	  chprintln (ch, "There are no commands disabled.");
	  return;
	}

      chprintln (ch, "Disabled commands:" NEWLINE
		 "Command      Level   Disabled by   Disabled for");

      for (p = disabled_first; p; p = p->next)
	{
	  chprintlnf (ch, "%-12s %5d   %-12s  %s", p->command->name,
		      p->level, p->disabled_by, p->disabled_for);
	}
      return;
    }




  for (p = disabled_first; p; p = p->next)
    if (!str_cmp (arg, p->command->name))
      break;

  if (p)
    {


      if (get_trust (ch) < p->level)
	{
	  chprintln (ch, "This command was disabled by a higher power.");
	  return;
	}


      UnLink (p, disabled, next, prev);
      free_disabled (p);
      rw_disabled_data (act_write);
      chprintln (ch, "Command enabled.");
    }
  else
    {



      for (i = command_hash[tolower (arg[0]) % MAX_CMD_HASH]; i;
	   i = i->next_hash)
	if (!str_cmp (i->name, arg))
	  break;

      if (!i)
	{
	  chprintln (ch, "No such command.");
	  return;
	}


      if (i->do_fun == do_disable)
	{
	  chprintln (ch, "You cannot disable the disable command.");
	  return;
	}


      if (i->level > get_trust (ch))
	{
	  chprintln (ch,
		     "You don't have access to that command; you cannot disable it.");
	  return;
	}


      p = new_disabled ();
      p->command = i;
      p->disabled_by = str_dup (ch->name);
      p->disabled_for = str_dup (argument);
      p->level = get_trust (ch);
      Link (p, disabled, next, prev);

      chprintln (ch, "Command disabled.");
      rw_disabled_data (act_write);
    }
}


bool
check_disabled (CharData * ch, CmdData * command)
{
  DisabledData *p;

  for (p = disabled_first; p; p = p->next)
    if (p->command->do_fun == command->do_fun)
      break;

  if (!p)
    return false;

  if (!ch || NullStr (p->disabled_for))
    return true;

  return is_exact_name (ch->name, p->disabled_for);
}


void
cmd_syntax (CharData * ch, const char *title, const char *n_fun, ...)
{
  va_list args;
  char *str;
  size_t i;

  if (NullStr (n_fun) || !ch || !ch->desc)
    return;

  va_start (args, n_fun);

  str = va_arg (args, char *);

  if (str == NULL)
    return;

  if (NullStr (title))
    {
      if (ch->desc->editor != ED_NONE)
	title = olc_ed_name (ch->desc);
      else
	{
	  switch (number_range (1, 3))
	    {
	    default:
	    case 1:
	      title = "Syntax";
	      break;
	    case 2:
	      title = "Usage";
	      break;
	    case 3:
	      title = "Type";
	      break;
	    }
	}
    }

  i = strlen (title) + 1;

  chprintlnf (ch, "{W%s: {w%s %s{x", Upper (title), n_fun, str);

  while ((str = va_arg (args, char *)) != NULL)
      chprintlnf (ch, "{W%*c {w%s %s{x", i, ':', n_fun, str);

  va_end (args);
}

const char *
cmd_name (Do_F * dofun)
{
  CmdData *c;

  for (c = cmd_first; c; c = c->next)
    if (c->do_fun == dofun)
      return c->name;

  return "unknown";
}
