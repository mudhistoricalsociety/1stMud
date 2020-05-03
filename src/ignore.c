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

bool
is_ignoring (CharData * ch, const char *name, flag_t bit)
{
  int pos;

  if (ch == NULL || NullStr (name))
    return false;

  if (IsNPC (ch))
    return false;

  for (pos = 0; pos < MAX_IGNORE; pos++)
    {
      if (NullStr (ch->pcdata->ignore[pos]))
	break;

      if (is_name (ch->pcdata->ignore[pos], name))
	{
	  if (bit == 0
	      || IsSet (ch->pcdata->ignore_flags[pos], bit | IGNORE_ALL))
	    return true;
	}
    }

  return false;
}

Do_Fun (do_ignore)
{
  CharData *victim;
  char arg[MIL];
  int pos;
  bool found = false;
  CharData *wch;
  flag_t iValue;

  argument = one_argument (argument, arg);

  if (IsNPC (ch))
    return;

  if (NullStr (arg))
    {
      Buffer *output = new_buf ();

      bprintln (output, "People you are ignoring:");
      bprintln (output, draw_line (ch, "-=", 0));
      for (pos = 0; pos < MAX_IGNORE; pos++)
	{
	  if (NullStr (ch->pcdata->ignore[pos]))
	    break;
	  found = true;
	  bprintlnf (output, "[%02d] %-12s", pos + 1,
		     ch->pcdata->ignore[pos]);
	}
      if (!found)
	bprintln (output, "No one.");
      found = false;
      pos = 0;
      bprintln (output, NULL);
      bprintln (output, "People online who are ignoring you:");
      bprintln (output, draw_line (ch, "-=", 0));
      for (wch = player_first; wch != NULL; wch = wch->next_player)
	{
	  if (!can_see (ch, wch) || !is_ignoring (wch, ch->name, 0))
	    continue;
	  pos++;
	  found = true;
	  bprintlnf (output, "[%02d] %s", pos, wch->name);
	}
      if (!found)
	bprintln (output, "No one.");
      bprintln (output, NULL);
      bprintlnf
	(output,
	 "Use '%s list' to show possible flags, use '%s <name> <flag>' to set a flags.",
	 n_fun, n_fun);
      sendpage (ch, buf_string (output));
      free_buf (output);
      return;
    }

  if (NullStr (argument))
    {
      if (!str_cmp (arg, "list"))
	{
	  show_flags (ch, ignore_flags);
	  return;
	}
      found = false;

      for (pos = 0; pos < MAX_IGNORE; pos++)
	{
	  if (NullStr (ch->pcdata->ignore[pos]))
	    break;

	  if (found)
	    {
	      replace_str (&ch->pcdata->ignore[pos - 1],
			   ch->pcdata->ignore[pos]);
	      free_string (ch->pcdata->ignore[pos]);
	      ch->pcdata->ignore[pos] = NULL;
	      ch->pcdata->ignore_flags[pos - 1] =
		ch->pcdata->ignore_flags[pos];
	      continue;
	    }

	  if (!str_cmp (arg, ch->pcdata->ignore[pos]))
	    {
	      free_string (ch->pcdata->ignore[pos]);
	      ch->pcdata->ignore[pos] = NULL;
	      ch->pcdata->ignore_flags[pos] = 0;
	      found = true;
	    }
	}
      if (found)
	{
	  chprintlnf (ch, "You stop ignoring %s.", capitalize (arg));
	  return;
	}
    }
  else
    {
      for (pos = 0; pos < MAX_IGNORE; pos++)
	{
	  if (NullStr (ch->pcdata->ignore[pos]))
	    break;

	  if (!str_cmp (arg, ch->pcdata->ignore[pos]))
	    {
	      found = true;
	      break;
	    }
	}
      if (!found)
	{
	  chprintln (ch, "They are not on your ignore list.");
	  return;
	}
    }

  if (pos >= MAX_IGNORE)
    {
      chprintln (ch, "You can't ignore anymore people");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "That character isn't online.");
      return;
    }
  else
    {
      if (IsNPC (victim))
	{
	  chprintln (ch, "Ignore a mob?  I don't think so.");
	  return;
	}

      if (ch == victim)
	{
	  chprintln (ch, "I don't think you really want to ignore yourself.");
	  return;
	}

      if (!is_exact_name (victim->name, arg))
	{
	  chprintln (ch, "You must spell out their entire name.");
	  return;
	}

      if (IsImmortal (victim))
	{
	  chprintln (ch, "You're not going to ignore us that easily!");
	  return;
	}
    }
  if (NullStr (argument))
    {
      replace_str (&ch->pcdata->ignore[pos], capitalize (arg));
      ch->pcdata->ignore_flags[pos] = IGNORE_CHANNELS;
      chprintlnf (ch, "You now ignore %s.", capitalize (arg));
      chprintlnf
	(ch,
	 "Use '%s list' to show possible flags, use '%s <name> <flag>' to set a flags.",
	 n_fun, n_fun);
    }
  else if ((iValue = flag_value (ignore_flags, argument)) != NO_FLAG)
    {
      set_on_off (ch, &ch->pcdata->ignore_flags[pos], iValue,
		  FORMATF ("%s ignore flag set for %s.",
			   flag_string (ignore_flags, iValue),
			   ch->pcdata->ignore[pos]),
		  FORMATF ("%s ignore flag removed for %s.",
			   flag_string (ignore_flags, iValue),
			   ch->pcdata->ignore[pos]));
      return;
    }
  else
    {
      chprintlnf (ch, "Valid flags for %s are:", arg);
      print_all_on_off (ch, ignore_flags, ch->pcdata->ignore_flags[pos]);
    }
  return;

}
