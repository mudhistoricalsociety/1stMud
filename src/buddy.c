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

Do_Fun (do_btalk)
{
  if (!ch || IsNPC (ch))
    return;

  public_ch (n_fun, ch, argument, gcn_buddy);
  return;
}

Do_Fun (do_buddy)
{
  CharData *fch;
  char arg[MIL];
  int pos;
  bool found = false;

  if (IsNPC (ch))
    {
      chprintln (ch, "Mobiles don't have buddies =).");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      if (NullStr (ch->pcdata->buddies[0]))
	{
	  chprintln (ch, "You have no buddies!");
	  return;
	}
      chprintln (ch, CTAG (_BTALK1) "Your " CTAG (_BTALK2) "Buddies"
		 CTAG (_BTALK1) ":{x");
      for (pos = 0; pos < MAX_BUDDY; pos++)
	{
	  if (NullStr (ch->pcdata->buddies[pos]))
	    break;
	  fch = get_char_world (NULL, ch->pcdata->buddies[pos]);
	  chprintlnf (ch, CTAG (_BTALK1) "%02d)" CTAG (_BTALK2)
		      " %-12s %s{x", pos + 1,
		      capitalize (ch->pcdata->buddies[pos]), (!fch
							      ||
							      !can_see
							      (ch,
							       fch)) ?
		      "{R(Offline){x" : check_buddy (fch,
						     ch) ==
		      -1 ? "{M(Wannabe){x" : !fch->desc ? "{C(LinkDead){x"
		      : "{G(Online){x");
	}
      return;
    }
  else if (!str_prefix (arg, "clear"))
    {
      for (pos = 0; pos < MAX_BUDDY; pos++)
	{
	  if (NullStr (ch->pcdata->buddies[pos]))
	    break;
	  replace_str (&ch->pcdata->buddies[pos], "");
	}
      chprintln (ch, "No one is your buddy!");
      return;
    }
  else if (!str_prefix (arg, "wannabe"))
    {
      chprintln (ch, CTAG (_BTALK1)
		 "People online who want to be your buddy:{x");
      for (fch = player_first; fch != NULL; fch = fch->next_player)
	{
	  if (check_buddy (fch, ch) != -1 && check_buddy (ch, fch) == -1)
	    {
	      chprintlnf (ch, CTAG (_BTALK1) "- " CTAG (_BTALK2)
			  "%-12s{x", fch->name);
	      found = true;
	    }
	}
      if (!found)
	chprintln (ch, CTAG (_BTALK1) "- " CTAG (_BTALK2) "No one.{x");
      return;
    }
  else if (!str_prefix (arg, "deny"))
    {
      if ((fch = get_char_world (ch, argument)) == NULL || IsNPC (fch))
	{
	  chprintln (ch, "They aren't online.");
	  return;
	}
      if (fch == ch)
	{
	  chprintln (ch, "Deny yourself from being your buddy.... ok then.");
	  return;
	}
      if (check_buddy (fch, ch) != -1 && check_buddy (ch, fch) == -1)
	{
	  for (pos = 0; pos < MAX_BUDDY; pos++)
	    {
	      if (NullStr (fch->pcdata->buddies[pos]))
		break;
	      if (found)
		{
		  replace_str (&fch->pcdata->buddies[pos - 1],
			       fch->pcdata->buddies[pos]);
		  fch->pcdata->buddies[pos] = NULL;
		  continue;
		}
	      if (!str_prefix (ch->name, fch->pcdata->buddies[pos]))
		{
		  chprintlnf (fch, "%s doesn't want to be your buddy.",
			      ch->name);
		  chprintlnf
		    (ch,
		     "You inform %s you don't want to be their buddy.",
		     fch->name);
		  replace_str (&fch->pcdata->buddies[pos], "");
		  found = true;
		}
	    }

	  if (!found)
	    {
	      bugf ("%s isn't a wannabe of %s", ch->name, fch->name);
	      return;
	    }
	}
      else
	chprintln (ch, "They don't want to be your buddy.");
      return;
    }
  else
    {
      for (pos = 0; pos < MAX_BUDDY; pos++)
	{
	  if (NullStr (ch->pcdata->buddies[pos]))
	    break;
	  if (found)
	    {
	      replace_str (&ch->pcdata->buddies[pos - 1],
			   ch->pcdata->buddies[pos]);
	      free_string (ch->pcdata->buddies[pos]);
	      continue;
	    }
	  if (!str_prefix (arg, ch->pcdata->buddies[pos]))
	    {
	      if ((fch = get_char_world (ch, ch->pcdata->buddies[pos]))
		  != NULL)
		{
		  if (check_buddy (fch, ch) != -1)
		    chprintlnf (fch, "%s is no longer your buddy.", ch->name);
		  else
		    chprintlnf (fch,
				"%s no longer wants to be your buddy.",
				ch->name);
		}
	      chprintlnf (ch, "%s is no longer your buddy.",
			  capitalize (arg));
	      replace_str (&ch->pcdata->buddies[pos], "");
	      found = true;
	    }
	}

      if (found)
	return;

      if (pos >= MAX_BUDDY)
	{
	  chprintln (ch, "Too many people, remove a name.");
	  return;
	}
      if ((fch = get_char_world (ch, arg)) == NULL)
	{
	  if (!file_exists (pfilename (arg, PFILE_NORMAL)))
	    {
	      chprintln (ch, "That character doesn't exist.");
	      return;
	    }

	  replace_str (&ch->pcdata->buddies[pos], capitalize (arg));
	  chprintlnf (ch, "You are now buddies with %s, who is offline.",
		      capitalize (arg));
	  return;
	}
      if (fch == ch)
	{
	  chprintln (ch, "Are you that lonely?");
	  return;
	}
      if (IsNPC (fch))
	{
	  chprintln (ch, "Uh, nope.");
	  return;
	}

      replace_str (&ch->pcdata->buddies[pos], capitalize (arg));
      if (check_buddy (fch, ch) == -1)
	chprintlnf (ch, "You are now a wannabe buddy of %s.",
		    capitalize (ch->pcdata->buddies[pos]));
      else
	chprintlnf (ch, "You are now buddies with %s.",
		    capitalize (ch->pcdata->buddies[pos]));
      if (check_buddy (fch, ch) == -1)
	chprintlnf (fch,
		    "%s wants to be your buddy. (see 'buddy wannabe')",
		    ch->name);
      else
	chprintlnf (fch, "%s is now your buddy.", ch->name);
      return;
    }
}

int
check_buddy (CharData * ch, CharData * fch)
{
  int pos;

  if (!ch || !fch)
    return -1;

  if (IsNPC (ch) || IsNPC (fch))
    return -1;

  for (pos = 0; pos < MAX_BUDDY; pos++)
    {
      if (NullStr (ch->pcdata->buddies[pos]))
	break;
      if (is_name (fch->name, ch->pcdata->buddies[pos]))
	return pos;
    }
  return -1;
}
