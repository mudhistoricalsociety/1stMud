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
#include "tables.h"
#include "olc.h"

const char *
get_chan_color (ChannelData * chan)
{
  if (chan->custom_color != -1)
    return FORMATF (CTAG (%d), chan->custom_color);
  else
    return chan->color;
}

const char *
format_channel (ChannelData * chan, CharData * ch)
{
  static char buf[MSL];

  strcpy (buf, chan->format);

  switch (chan->spec_flag)
    {
    case spec_clan_flag:
      sprintf (buf, chan->format, CharClan (ch)->name,
	       CharClan (ch)->rank[ch->rank].rankname);
      break;
    default:
      break;
    }
  strcat (buf, get_chan_color (chan));
  return (buf);
}

bool
display_channel (CharData * ch, CharData * victim, ChannelData * chan,
		 chanarg_t type, bool fShow)
{

  if (!ch || !victim)
    return false;

  if (IsSet (victim->wiznet, WIZ_CHANSNOOP))
    return true;

  if (!fShow)
    {
      if (chan->bit > 0 && IsSet (victim->comm, chan->bit))
	return false;

      if ((type == CHANNEL_EMOTE || type == CHANNEL_SOCIAL)
	  && IsSet (victim->comm, COMM_NOGOCIAL))
	return false;

      if (IsSet (victim->comm, COMM_QUIET))
	return false;

      if (is_ignoring (victim, ch->name, IGNORE_CHANNELS))
	return false;
    }

  switch (chan->spec_flag)
    {
    case spec_clan_flag:
      if (!is_same_clan (ch, victim))
	return false;
      break;
    case spec_imm_flag:
      if (!IsImmortal (victim))
	return false;
      break;
    case spec_buddy_flag:
      if (victim != ch
	  && (check_buddy (ch, victim) == -1
	      || check_buddy (victim, ch) == -1))
	return false;
      break;
    case spec_public_flag:
      return true;
      break;
    case spec_none:
      return false;
    }

  return true;
}

#ifndef DISABLE_WEBSRV

void
init_www_history (void)
{
  int x;

  for (x = 0; x < 20; x++)
    {
      www_history[x] = &str_empty[0];
    }
}
#endif

void
init_channel_history (PcData * pcdata)
{
  int i, x;

  alloc_mem (pcdata->history, const char **, top_channel);
  alloc_mem (pcdata->history_index, int, top_channel);

  for (i = 0; i < top_channel; i++)
    {
      pcdata->history_index[i] = 0;

      if (channel_table[i].page_length <= 0)
	continue;

      alloc_mem (pcdata->history[i], const char *,
		 channel_table[i].page_length);
      for (x = 0; x < channel_table[i].page_length; x++)
	pcdata->history[i][x] = &str_empty[0];
    }
}

void
realloc_channel_history (PcData * pcdata, int gcn, int val, int oval)
{
  int i, x;

  if (gcn >= 0 && gcn < top_channel)
    {
      if (oval <= 0)
	{
	  if (val > 0)
	    {
	      alloc_mem (pcdata->history[gcn], const char *, val);
	    }
	}
      else
	{
	  if (val <= 0)
	    {
	      free_mem (pcdata->history[gcn]);
	    }
	  else
	    {
	      realloc_mem (pcdata->history[gcn], const char *, val);

	      for (x = oval; x < val; x++)
		pcdata->history[gcn][x] = &str_empty[0];
	    }
	}
    }
  else
    {
      realloc_mem (pcdata->history, const char **, val);
      realloc_mem (pcdata->history_index, int, val);

      for (i = oval; i < val; i++)
	{
	  pcdata->history_index[i] = 0;

	  if (channel_table[i].page_length <= 0)
	    continue;

	  alloc_mem (pcdata->history[i], const char *,
		     channel_table[i].page_length);
	  for (x = 0; x < channel_table[i].page_length; x++)
	    pcdata->history[i][x] = &str_empty[0];
	}
    }
}

void
free_channel_history (PcData * pcdata)
{
  int i, x;

  for (i = 0; i < top_channel; i++)
    {
      if (channel_table[i].page_length <= 0)
	continue;

      for (x = 0; x < channel_table[i].page_length; x++)
	free_string (pcdata->history[i][x]);
      free_mem (pcdata->history[i]);
    }
  free_mem (pcdata->history);
  free_mem (pcdata->history_index);
}

#ifndef DISABLE_WEBSRV

const char *
Pers_WWW (CharData * ch)
{

  if (!ch)
    return "@@@";

  if (IsImmortal (ch) && (ch->invis_level != 0 || ch->incog_level != 0))
    return "an Immortal";
  else if (IsAffected (ch, AFF_INVISIBLE))
    return "someone";
  return IsNPC (ch) ? ch->short_descr : ch->name;
}
#endif

void
update_last_data (CharData * sender, CharData * viewer,
		  ChannelData * channel, const char *str, chanarg_t type)
{
  char *time;
  int gcn, i;
  char buf[MSL];
  const char *chan;

  if (!channel || channel->page_length <= 0)
    return;

  gcn = *channel->index;
  chan = format_channel (channel, sender);

  if (viewer && !IsNPC (viewer))
    {
      ++viewer->pcdata->history_index[gcn];
      viewer->pcdata->history_index[gcn] %= channel->page_length;

      i = viewer->pcdata->history_index[gcn];

      time = str_time (current_time, GetTzone (viewer), "%I:%M:%S %p");
      switch (type)
	{
	case CHANNEL_NORMAL:
	  sprintf (buf, "[%s] %s{x %s %s '%s'{x", time, chan, Pers (sender,
								    viewer),
		   say_verb (str, sender, viewer, 2), str);
	  break;
	case CHANNEL_SOCIAL:
	  sprintf (buf, "[%s] %s{x %s{x", time, chan, str);
	  break;
	case CHANNEL_EMOTE:
	  sprintf (buf, "[%s] %s{x %s %s{x",
		   time, chan, sender == viewer ? "You" : Pers (sender,
								viewer), str);
	  break;
	case CHANNEL_THINK:
	  sprintf (buf,
		   "[%s] %s %s . o O ( %s ){x", time, chan,
		   sender == viewer ? "You" : Pers (sender, viewer), str);
	  break;
	default:
	  bugf ("bad channel type [%d]", type);
	  buf[0] = '\0';
	  break;
	}

      replace_str (&viewer->pcdata->history[gcn][i], buf);
    }

#ifndef DISABLE_WEBSRV

  if (channel->spec_flag != spec_public_flag || sender == viewer)
    return;

  ++www_index;
  www_index %= 20;

  time = str_time (current_time, -1, "%I:%M:%S %p");

  switch (type)
    {
    case CHANNEL_NORMAL:
      sprintf (buf, "[%s] %s %s %s '%s'", time, chan, Pers_WWW (sender),
	       say_verb (str, sender, NULL, 2), str);
      break;
    case CHANNEL_SOCIAL:
      sprintf (buf, "[%s] %s %s", time, chan, str);
      break;
    case CHANNEL_EMOTE:
      sprintf (buf, "[%s] %s %s %s", time, chan, Pers_WWW (sender), str);
      break;
    case CHANNEL_THINK:
      sprintf (buf, "[%s] %s %s . o O ( %s )",
	       time, chan, Pers_WWW (sender), str);
      break;
    default:
      bugf ("bad channel type [%d]", type);
      buf[0] = '\0';
      break;
    }
  replace_str (&www_history[www_index], buf);
#endif

}

void
view_last_data (CharData * ch, ChannelData * chan)
{
  int i, gcn;
  bool found = false;
  Buffer *output;

  if (!chan || chan->page_length <= 0)
    return;

  output = new_buf ();
  gcn = *chan->index;
  for (i = (ch->pcdata->history_index[gcn] + 1) % chan->page_length;
       i != ch->pcdata->history_index[gcn]; i = (i + 1) % chan->page_length)
    {
      if (!NullStr (ch->pcdata->history[gcn][i]))
	{
	  found = true;
	  bprintln (output, ch->pcdata->history[gcn][i]);
	}
    }
  if (!NullStr (ch->pcdata->history[gcn][ch->pcdata->history_index[gcn]]))
    {
      bprintln (output,
		ch->pcdata->history[gcn][ch->pcdata->history_index[gcn]]);
      found = true;
    }

  if (!found)
    bprintln (output, "None.");
  else
    {
      bprintlnf (output, NEWLINE "Current time: %s   Your Login Time: %s",
		 str_time (current_time, GetTzone (ch), "%I:%M:%S %p"),
		 str_time (ch->logon, GetTzone (ch), "%I:%M:%S %p"));
    }
  sendpage (ch, buf_string (output));
  free_buf (output);
}

const char *
get_chan_soc_string (CharData * ch, CharData * victim,
		     CharData * vch, SocialData * soc)
{
  if (!victim)
    {
      if (ch == vch)
	return soc->char_no_arg;
      else
	return soc->others_no_arg;
    }
  else
    {
      if (victim == ch)
	{
	  if (vch == ch)
	    return soc->char_auto;
	  else
	    return soc->others_auto;
	}
      else
	{
	  if (victim == vch)
	    return soc->vict_found;
	  else if (ch == vch)
	    return soc->char_found;
	  else
	    return soc->others_found;
	}
    }
}

void
channel_social (CharData * ch, CharData * victim,
		SocialData * soc, ChannelData * chan)
{
  Descriptor *d;
  const char *type;

  type = format_channel (chan, ch);
  mud_info.stats.chan_msgs++;

  for (d = descriptor_first; d; d = d->next)
    {
      CharData *vch = CH (d);

      if (vch && (vch == ch || vch == victim ||
		  display_channel (ch, vch, chan, CHANNEL_SOCIAL, false)))
	{
	  const char *string = get_chan_soc_string (ch, victim, vch, soc);

	  string = swearcheck (string, vch);

	  if (d->connected == CON_PLAYING)
	    {
	      perform_act (FORMATF ("%s %s{x", type, string), ch, NULL,
			   victim, TO_CHAR, vch);
	    }
	  update_last_data (ch, vch, chan,
			    perform_act_string (string, ch, NULL, victim,
						false), CHANNEL_SOCIAL);
	}
    }
}

void
public_ch (const char *n_fun, CharData * ch, const char *argument, int gcn)
{
  char command[MIL + 100];
  Descriptor *d;
  chanarg_t chan_type = CHANNEL_NORMAL;
  char arg_left[MSL];
  ChannelData *chan = &channel_table[gcn];
  const char *type;

  if (chan == NULL)
    {
      chprintln (ch, "Channel is currently unavailable.");
      return;
    }

  type = format_channel (chan, ch);

  if (NullStr (argument))
    {
      if (chan->bit <= 0)
	chprintln (ch, "What do you want to say?");
      else
	{
	  set_on_off (ch, &ch->comm, chan->bit,
		      FORMATF ("%s channel is now OFF.{x", n_fun),
		      FORMATF ("%s channel is now ON.{x", n_fun));
	}
    }
  else
    {
      if (IsSet (ch->comm, COMM_QUIET))
	{
	  chprintln (ch, "You must turn off quiet mode first.");
	  return;
	}
      if (chan->bit > 0)
	RemBit (ch->comm, (chan->bit));

      strcpy (arg_left, argument);

      argument = one_argument (argument, command);
      if (command[0] == '+')
	{
	  CharData *victim;
	  SocialData *soc;
	  char argx[MIL];

	  argument = one_argument (argument, command);
	  if (NullStr (command))
	    {
	      cmd_syntax
		(ch, n_fun,
		 "{W%s + <social> is used for channel based socials.{x",
		 n_fun);
	      return;
	    }
	  if (!(soc = find_social (command)))
	    {
	      chprintln (ch, "{WWhat kind of social is that?!?!{x");
	      return;
	    }
	  one_argument (argument, argx);
	  victim = NULL;
	  if (NullStr (argx))
	    {
	      channel_social (ch, NULL, soc, chan);
	    }
	  else if ((victim = get_char_world (ch, argx)) == NULL)
	    {
	      chprintln (ch, "They aren't here.");
	      return;
	    }
	  else
	    {
	      if (is_ignoring (victim, ch->name, IGNORE_SOCIALS))
		{
		  act ("$N is ignoring socials from you.", ch, NULL,
		       victim, TO_CHAR);
		  return;
		}
	      if (!display_channel (ch, victim, chan, CHANNEL_SOCIAL, false))
		{
		  chprintln (ch, "They can't use that channel.");
		  return;
		}
	      channel_social (ch, victim, soc, chan);
	    }
	  return;
	}
      else if (command[0] == '!')
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, "%s ! <argument>", NULL);
	      return;
	    }

	  argument = str_rep (argument, "{x", get_chan_color (chan));
	  argument = swearcheck (argument, ch);
	  chan_type = CHANNEL_EMOTE;
	  chprintlnf (ch, "%s %s %s{x", type,
		      IsNPC (ch) ? ch->short_descr : ch->name, argument);
	  update_last_data (ch, ch, chan, argument, CHANNEL_EMOTE);
	}
      else if (command[0] == '@')
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "%s @ <argument>", NULL);
	      return;
	    }

	  chan_type = CHANNEL_THINK;
	  argument = str_rep (argument, "{x", get_chan_color (chan));
	  argument = swearcheck (argument, ch);
	  chprintlnf (ch, "%s %s . o O ( %s ){x", type,
		      IsNPC (ch) ? ch->short_descr : ch->name, argument);
	  update_last_data (ch, ch, chan, argument, CHANNEL_THINK);
	}
      else if (is_name (command, "-wholist -who") && NullStr (argument))
	{
	  chan_type = CHANNEL_WHO;
	  chprintlnf (ch, "{WPlayers on %s{x", type);
	  chprintln (ch, "{C-------------------{x");
	}
      else if (is_name (command, "-hist -history") && NullStr (argument))
	{
	  if (IsNPC (ch) || chan->page_length <= 0)
	    {
	      chprintln (ch, "Channel history unavailable.");
	      return;
	    }
	  chprintlnf (ch, "{WLast %d messages on %s{x", chan->page_length,
		      type);
	  chprintln (ch, "{C------------------------------{x");
	  view_last_data (ch, chan);
	  return;
	}
      else if (is_name (command, "-help") && NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun,
		      "<message>          - send a message",
		      "                   - toggle channel on/off",
		      "-history           - display channel history",
		      "-wholist           - display who is on channel",
		      "! <emote>          - send an emote over channel",
		      "+ <social> [args]  - do a social over channel",
		      "@ <message>        - enclose a message in 'thought bubbles'",
		      "?                  - this message", NULL);
	  return;
	}
      else
	{
	  chan_type = CHANNEL_NORMAL;
	  argument = str_rep (arg_left, "{x", get_chan_color (chan));
	  argument = swearcheck (argument, ch);
	  chprintlnf (ch, "%s You %s '%s'{x", type,
		      say_verb (argument, ch, ch, 0), argument);
	  update_last_data (ch, ch, chan, argument, CHANNEL_NORMAL);
	}
      mud_info.stats.chan_msgs++;
      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  CharData *victim;

	  if ((victim = d->character) == NULL)
	    continue;
	  if (victim == ch)
	    continue;
	  if (!display_channel (ch, victim, chan, chan_type, false))
	    continue;

	  switch (chan_type)
	    {
	    default:
	    case CHANNEL_NORMAL:
	      argument = swearcheck (arg_left, victim);
	      if (d->connected == CON_PLAYING)
		chprintlnf (victim, "%s %s %s '%s'{x", type,
			    strip_color (Pers (ch, victim)),
			    say_verb (argument, ch, victim, 1), argument);
	      update_last_data (ch, victim, chan, argument, CHANNEL_NORMAL);
	      break;
	    case CHANNEL_EMOTE:
	    case CHANNEL_THINK:
	      argument = swearcheck (argument, victim);
	      if (d->connected == CON_PLAYING)
		{
		  if (chan_type == CHANNEL_THINK)
		    chprintlnf (victim, "%s %s . o O ( %s ){x", type,
				strip_color (Pers (ch, victim)), argument);
		  else
		    chprintlnf (victim, "%s %s %s{x", type,
				strip_color (Pers (ch, victim)), argument);
		}
	      update_last_data (ch, victim, chan, argument, chan_type);
	      break;
	    case CHANNEL_WHO:
	      if (can_see (ch, victim))
		chprintlnf (ch, "{W%s{x", Pers (victim, ch));
	      break;
	    }
	}
    }
}

Do_Fun (do_gossip)
{
  public_ch (n_fun, ch, argument, gcn_gossip);
}

Do_Fun (do_grats)
{
  public_ch (n_fun, ch, argument, gcn_grats);
}

Do_Fun (do_quote)
{
  public_ch (n_fun, ch, argument, gcn_quote);
}

Do_Fun (do_question)
{
  public_ch (n_fun, ch, argument, gcn_qa);
}

Do_Fun (do_answer)
{
  public_ch (n_fun, ch, argument, gcn_qa);
}

Do_Fun (do_music)
{
  public_ch (n_fun, ch, argument, gcn_music);
}

Do_Fun (do_ooc)
{
  public_ch (n_fun, ch, argument, gcn_ooc);
}

Do_Fun (do_immtalk)
{
  public_ch (n_fun, ch, argument, gcn_immtalk);
}

Do_Fun (do_barter)
{
  public_ch (n_fun, ch, argument, gcn_barter);
}



Do_Fun (do_channels)
{
  int i;
  Buffer *buf;

  buf = new_buf ();

  bprintlnf (buf, " %-9s %-6s{w %s", "Command", "Status", "Description");

  bprintln (buf, draw_line (ch, NULL, 0));

  for (i = 0; i < top_channel; i++)
    {
      if (!display_channel (ch, ch, &channel_table[i], CHANNEL_NORMAL, true))
	continue;

      bprintlnf (buf, "{G%-11s %-6s{w %s{x", channel_table[i].name,
		 OnOff (!IsSet (ch->comm, channel_table[i].bit)),
		 channel_table[i].description);
    }

  bprintlnf (buf, "{G%-11s %-6s{w %s{x", "shouts",
	     OnOff (IsSet (ch->comm, COMM_SHOUTSOFF)),
	     "A global channel that transmits with a delay as if there is an echo.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x", "deaf",
	     OnOff (IsSet (ch->comm, COMM_DEAF)),
	     "Prevents you from hearing any tells.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x", "quiet",
	     OnOff (IsSet (ch->comm, COMM_QUIET)),
	     "Toggles whether you receive any channels at all.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x", "afk",
	     OnOff (IsSet (ch->comm, COMM_AFK)),
	     "Sets you Away From Keyboard.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x", "nogocial",
	     OnOff (IsSet (ch->comm, COMM_NOGOCIAL)),
	     "Toggles socials/emotes over public channels.");

  bprintln (buf, draw_line (ch, NULL, 0));

  if (IsSet (ch->comm, COMM_SNOOP_PROOF))
    bprintln (buf, "You are immune to snooping.");

  if (ch->prompt != NULL)
    {
      bprintlnf (buf, "Your current prompt is: %s", ch->prompt);
    }

  if (ch->gprompt != NULL)
    {
      bprintlnf (buf, "Your current gprompt is: %s", ch->gprompt);
    }

  if (IsSet (ch->comm, COMM_NOSHOUT))
    bprintln (buf, "You cannot shout.");

  if (IsSet (ch->comm, COMM_NOTELL))
    bprintln (buf, "You cannot use tell.");

  if (IsSet (ch->comm, COMM_NOCHANNELS))
    bprintln (buf, "You cannot use channels.");

  if (IsSet (ch->comm, COMM_NOEMOTE))
    bprintln (buf, "You cannot show emotions.");

  sendpage (ch, buf_string (buf));
  free_buf (buf);

  do_function (ch, &do_censor, "status");
}


char *const swear[] = {
  "shit", "fuck", "bitch", "bastard",
  "bullshit", "pussy", "dick", "cock",
  "motherfuck", "clit", "damn", "dammit",
  "dumbass", "slut", "whore", "asshole", NULL
};

Do_Fun (do_censor)
{
  if (!str_cmp (argument, "status"))
    {
      chprintlnf (ch, "Channels are currently rated %s.",
		  !IsSet (ch->comm, COMM_CENSOR) ? "{RR{x" : "{GPG{x");
      return;
    }
  if (!str_cmp (argument, "list"))
    {
      Column c;
      int x;

      if (IsSet (ch->comm, COMM_CENSOR))
	{
	  chprintln (ch, "You must be set to rated {RR{x to see the list.");
	  return;
	}

      set_cols (&c, ch, 4, COLS_CHAR, ch);

      for (x = 0; swear[x] != NULL; x++)
	{
	  print_cols (&c, swear[x]);
	}
      cols_nl (&c);
      return;
    }

  set_on_off (ch, &ch->comm, COMM_CENSOR,
	      "Channels are now rated {GPG{x.",
	      "Channels are now rated {RR{x.");
}


const char *
swearcheck (const char *argument, CharData * ch)
{
  int x;

  if (ch && !CommFlag (ch, COMM_CENSOR))
    return argument;

  for (x = 0; swear[x] != NULL; x++)
    {
      if (!str_infix (swear[x], argument))
	{
	  return stri_rep (argument, swear[x],
			   stringf (NULL, strlen (swear[x]), Left, "*", "*"));
	}
    }
  return argument;
}

Olc_Fun (chanedit_create)
{
  int j, i = top_channel;
  ChannelData *pChan;
  struct channel_type *new_table;
  CharData *pch;

  top_channel++;

  alloc_mem (new_table, struct channel_type, top_channel);

  if (!new_table)
    {
      chprintln (ch, "Memory Allocation Failed!!! Unable to create channel.");
      return false;
    }

  for (j = 0; j < i; j++)
    new_table[j] = channel_table[j];

  free_mem (channel_table);
  channel_table = new_table;

  channel_table[i].index = &gcn_null;
  channel_table[i].bit = 0;
  channel_table[i].spec_flag = spec_none;
  channel_table[i].page_length = 20;
  channel_table[i].format = &str_empty[0];
  channel_table[i].color = &str_empty[0];
  channel_table[i].name = str_dup (argument);
  channel_table[i].description = &str_empty[0];
  channel_table[i].custom_color = -1;

  for (pch = player_first; pch; pch = pch->next_player)
    realloc_channel_history (pch->pcdata, -1, top_channel, i);

  pChan = &channel_table[i];
  edit_start (ch, pChan, ED_CHAN);
  chprintln (ch, "Channel created.");
  return true;
}

Olc_Fun (chanedit_gcn)
{
  ChannelData *pChan;
  Column Cd;
  int i;

  GetEdit (ch, ChannelData, pChan);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<gcn_name>", NULL);
      return false;
    }

  if (is_name (argument, "clear reset none"))
    {
      pChan->index = &gcn_null;
      olc_msg (ch, n_fun, "GCN Entry cleared.");
      return true;
    }

  for (i = 0; channel_index[i].name != NULL; i++)
    {
      if (!str_cmp (argument, channel_index[i].name))
	{
	  if (channel_index[i].index != &gcn_null
	      && (!channel_index[i].index
		  || !*(int *) channel_index[i].index))
	    {
	      int tmp;

	      for (tmp = 0; tmp < top_channel; tmp++)
		if (&channel_table[tmp] == pChan)
		  break;
	      *(int *) channel_index[i].index = tmp;
	    }
	  pChan->index = (int *) channel_index[i].index;
	  olc_msg (ch, n_fun,
		   "Channel now uses global channel pointer '%s'.",
		   channel_index[i].name);
	  return true;
	}
    }

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);
  for (i = 0; channel_index[i].name != NULL; i++)
    print_cols (&Cd, channel_index[i].name);
  cols_nl (&Cd);
  olc_msg (ch, n_fun, "That GCN hasn't been coded in yet.");
  return false;
}

Olc_Fun (chanedit_color)
{
  ChannelData *pChan;
  int slot;

  GetEdit (ch, ChannelData, pChan);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<code>", "<custom code>", NULL);
      return false;
    }

  if ((slot = color_lookup (argument)) != -1)
    {
      pChan->custom_color = color_table[slot].slot;
      replace_str (&pChan->color, "");
    }
  else
    {
      replace_str (&pChan->color, argument);
      pChan->custom_color = -1;
    }

  olc_msg (ch, n_fun, "Channel color changed.");
  return true;
}

Olc_Fun (chanedit_delete)
{
  ChannelData *pChan;

  GetEdit (ch, ChannelData, pChan);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this channel!");
      return false;
    }
  else
    {
      int i, j = 0, c, old = top_channel;
      struct channel_type *new_table;
      CharData *pch;

      top_channel--;

      alloc_mem (new_table, struct channel_type, top_channel);

      if (!new_table)
	{
	  chprintln (ch,
		     "Memory Allocation error!!! Unable to delete channel.");
	  return false;
	}

      c = channel_lookup (pChan->name);
      if (c == -1 && pChan->index != NULL)
	c = *pChan->index;

      for (i = 0; i < old; i++)
	if (i != c)
	  new_table[j++] = channel_table[i];

      free_mem (channel_table);
      channel_table = new_table;

      for (pch = player_first; pch != NULL; pch = pch->next_player)
	realloc_channel_history (pch->pcdata, -1, top_channel, old);

      pChan = &channel_table[0];
      ch->desc->pEdit = (void *) pChan;

      chprintln (ch, "Channel deleted.");
    }

  return true;
}

Olc_Fun (chanedit_list)
{
  int i;

  for (i = 0; i < top_channel; i++)
    chprintlnf (ch, "%2d) %s{x", i, format_channel (&channel_table[i], ch));

  return true;
}
