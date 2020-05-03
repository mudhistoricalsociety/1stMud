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

int channel_songs[MAX_GLOBAL + 1];

int
song_lookup (const char *name)
{
  int i;

  for (i = 0; i < top_song; i++)
    if (is_name (name, song_table[i].name))
      return i;

  return -1;
}

void
song_update (void)
{
  ObjData *obj;
  CharData *victim;
  RoomIndex *room;
  Descriptor *d;
  char buf[MAX_STRING_LENGTH];
  const char *line;
  int i;


  if (channel_songs[1] >= top_song)
    channel_songs[1] = -1;

  if (channel_songs[1] > -1)
    {
      if (channel_songs[0] >= MAX_LINES ||
	  channel_songs[0] >= song_table[channel_songs[1]].lines)
	{
	  channel_songs[0] = -1;


	  for (i = 1; i < MAX_GLOBAL; i++)
	    channel_songs[i] = channel_songs[i + 1];
	  channel_songs[MAX_GLOBAL] = -1;
	}
      else
	{
	  if (channel_songs[0] < 0)
	    {
	      sprintf (buf, "Music: %s, %s",
		       song_table[channel_songs[1]].group,
		       song_table[channel_songs[1]].name);
	      channel_songs[0] = 0;
	    }
	  else
	    {
	      sprintf (buf, "Music: '%s'",
		       song_table[channel_songs[1]].lyrics[channel_songs[0]]);
	      channel_songs[0]++;
	    }

	  for (d = descriptor_first; d != NULL; d = d->next)
	    {
	      victim = d->original ? d->original : d->character;

	      if (d->connected == CON_PLAYING &&
		  !IsSet (victim->comm, COMM_NOMUSIC) &&
		  !IsSet (victim->comm, COMM_QUIET))
		act_new ("$t", d->character, buf, NULL,
			 TO_CHAR, POS_SLEEPING);
	    }
	}
    }

  for (obj = obj_first; obj != NULL; obj = obj->next)
    {
      if (obj->item_type != ITEM_JUKEBOX || obj->value[1] < 0)
	continue;

      if (obj->value[1] >= top_song)
	{
	  obj->value[1] = -1;
	  continue;
	}



      if ((room = obj->in_room) == NULL)
	{
	  if (obj->carried_by == NULL)
	    continue;
	  else if ((room = obj->carried_by->in_room) == NULL)
	    continue;
	}

      if (obj->value[0] < 0)
	{
	  sprintf (buf, "$p starts playing %s, %s.",
		   song_table[obj->value[1]].group,
		   song_table[obj->value[1]].name);
	  if (room->person_first != NULL)
	    act (buf, room->person_first, obj, NULL, TO_ALL);
	  obj->value[0] = 0;
	  continue;
	}
      else
	{
	  if (obj->value[0] >= MAX_LINES ||
	      obj->value[0] >= song_table[obj->value[1]].lines)
	    {

	      obj->value[0] = -1;


	      obj->value[1] = obj->value[2];
	      obj->value[2] = obj->value[3];
	      obj->value[3] = obj->value[4];
	      obj->value[4] = -1;
	      continue;
	    }

	  line = song_table[obj->value[1]].lyrics[obj->value[0]];
	  obj->value[0]++;
	}

      sprintf (buf, "$p bops: '%s'", line);
      if (room->person_first != NULL)
	act (buf, room->person_first, obj, NULL, TO_ALL);
    }
}

Do_Fun (do_play)
{
  ObjData *juke;
  const char *str;
  char arg[MAX_INPUT_LENGTH];
  int song, i;
  bool global = false;

  str = one_argument (argument, arg);

  for (juke = ch->in_room->content_first; juke != NULL;
       juke = juke->next_content)
    if (juke->item_type == ITEM_JUKEBOX && can_see_obj (ch, juke))
      break;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "list [artist]", "[loud] <song>", NULL);
      chprintln (ch, "Play what?");
      return;
    }

  if (juke == NULL)
    {
      chprintln (ch, "You see nothing to play.");
      return;
    }

  if (!str_cmp (arg, "list"))
    {
      Buffer *buffer;
      int col = 0;
      bool artist = false, match = false;

      buffer = new_buf ();
      argument = str;
      argument = one_argument (argument, arg);

      if (!str_cmp (arg, "artist"))
	artist = true;

      if (!NullStr (argument))
	match = true;

      bprintlnf (buffer, "%s has the following songs available:",
		 juke->short_descr);

      for (i = 0; i < top_song; i++)
	{
	  if (artist &&
	      (!match || !str_prefix (argument, song_table[i].group)))
	    bprintlnf (buffer, "%-39s %-39s", song_table[i].group,
		       song_table[i].name);
	  else if (!artist
		   && (!match || !str_prefix (argument, song_table[i].name)))
	    bprintf (buffer, "%-35s ", song_table[i].name);
	  else
	    continue;
	  if (!artist && ++col % 2 == 0)
	    bprintln (buffer, NULL);
	}
      if (!artist && col % 2 != 0)
	bprintln (buffer, NULL);

      sendpage (ch, buf_string (buffer));
      free_buf (buffer);
      return;
    }

  if (!str_cmp (arg, "loud"))
    {
      argument = str;
      global = true;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Play what?");
      return;
    }

  if ((global &&channel_songs[MAX_GLOBAL] >
       -1) ||(!global &&juke->value[4] > -1))
    {
      chprintln (ch, "The jukebox is full up right now.");
      return;
    }

  for (song = 0; song < top_song; song++)
    {
      if (!str_prefix (argument, song_table[song].name))
	break;
    }

  if (song >= top_song)
    {
      chprintln (ch, "That song isn't available.");
      return;
    }

  chprintln (ch, "Coming right up.");

  if (global)
    {
      for (i = 1; i <= MAX_GLOBAL; i++)
	if (channel_songs[i] < 0)
	  {
	    if (i == 1)
	      channel_songs[0] = -1;
	    channel_songs[i] = song;
	    return;
	  }
    }
  else
    {
      for (i = 1; i < 5; i++)
	if (juke->value[i] < 0)
	  {
	    if (i == 1)
	      juke->value[0] = -1;
	    juke->value[i] = song;
	    return;
	  }
    }
}
