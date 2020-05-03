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

void
cleanup_char (CharData * ch)
{
  CorpseData *c, *c_next;
  int hash;
  RoomIndex *pRoom;
  bool found = false;
  Proto (void save_corpses, (void));

  if (IsNPC (ch))
    return;

  update_statlist (ch, true);
  if (is_clan (ch))
    {
      update_members (ch, true);
    }
  update_webpasses (ch, true);
  if (IsImmortal (ch))
    update_wizlist (ch, true);
  for (c = corpse_first; c; c = c_next)
    {
      c_next = c->next;
      if (c->corpse && is_name (ch->name, c->corpse->owner))
	{
	  UnLink (c, corpse, next, prev);
	  free_corpse (c);
	  found = true;
	}
    }
  if (found)
    {
      save_corpses ();
      found = false;
    }
  if (ch->pcdata->backup)
    unlink (pfilename (ch->name, PFILE_BACKUP));

  delete_home (ch);

  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pRoom = room_index_hash[hash]; pRoom != NULL; pRoom = pRoom->next)
	{
	  if (!NullStr (pRoom->owner) && !str_cmp (pRoom->owner, ch->name))
	    {
	      replace_str (&pRoom->owner, "");
	      SetBit (pRoom->area->area_flags, AREA_CHANGED);
	    }
	}
    }
  for (hash = 0; hash < pfiles.count; hash++)
    {
      if (found)
	{
	  replace_str (&pfiles.names[hash - 1], pfiles.names[hash]);
	  free_string (pfiles.names[hash]);
	  continue;
	}

      if (!str_cmp (pfiles.names[hash], ch->name))
	{
	  found = true;
	  free_string (pfiles.names[hash]);
	}
    }
  if (found)
    {
      pfiles.count--;
    }
}


Do_Fun (do_delete)
{
  if (IsNPC (ch))
    return;

  if (ch->pcdata->confirm_delete)
    {
      if (!NullStr (argument))
	{
	  chprintln (ch, "Delete status removed.");
	  ch->pcdata->confirm_delete = false;
	  return;
	}
      else
	{
	  char strsave[MAX_INPUT_LENGTH];

	  cleanup_char (ch);
	  sprintf (strsave, pfilename (ch->name, PFILE_NORMAL));
	  if (get_trust (ch) >= mud_info.min_save_lvl)
	    {
	      char strdel[MAX_INPUT_LENGTH];

	      sprintf (strdel, DELETE_DIR "%s", pfile_filename (ch->name));
	      save_char_obj_to_filename (ch, strdel);
	    }
	  wiznet ("$N turns $Mself into line noise.", ch, NULL, 0, true, 0);
	  stop_fighting (ch, true);
	  do_function (ch, &do_quit, "");
	  unlink (strsave);
	  mud_info.stats.deletions++;
	  return;
	}
    }

  if (!NullStr (argument))
    {
      chprintln (ch, "Just type delete. No argument.");
      return;
    }

  chprintln (ch, "Type delete again to confirm this command.");
  chprintln (ch, "{f{RWARNING{x: this command is irreversible.");
  chprintln (ch, "Typing delete with an argument will undo delete status.");
  ch->pcdata->confirm_delete = true;
  wiznet ("$N is contemplating deletion.", ch, NULL, 0, false,
	  get_trust (ch));
}

Do_Fun (do_nogocial)
{
  set_on_off (ch, &ch->comm, COMM_NOGOCIAL,
	      "You no longer see socials/emotes over channels.",
	      "You now see socials/emotes over channels.");
}



Do_Fun (do_deaf)
{
  set_on_off (ch, &ch->comm, COMM_DEAF,
	      "From now on, you won't hear tells.",
	      "You can now hear tells again.");
}



Do_Fun (do_quiet)
{
  set_on_off (ch, &ch->comm, COMM_QUIET,
	      "From now on, you will only hear says and emotes.",
	      "Quiet mode removed.");
}



Do_Fun (do_afk)
{
  if (IsSet (ch->comm, COMM_AFK))
    {
      chprintln (ch, "AFK mode removed.");
      RemBit (ch->comm, COMM_AFK);
      if (!IsNPC (ch) && ch->pcdata->tells)
	{
	  chprintlnf (ch,
		      "You have %d tells waiting.  Type '%s' to view them.",
		      ch->pcdata->tells, cmd_name (do_replay));
	}
    }
  else
    {
      chprint (ch, "You are now in AFK mode. ");
      SetBit (ch->comm, COMM_AFK);
      if (!IsNPC (ch))
	{
	  replace_strf (&ch->pcdata->afk_msg, "[%s] %s",
			str_time (-1, GetTzone (ch), "%I:%M:%S %p"),
			GetStr (argument,
				"I'm away, please leave a message!"));
	  chprintln (ch, ch->pcdata->afk_msg);
	}
      else
	chprintln (ch, NULL);
    }
}

Do_Fun (do_replay)
{
  if (IsNPC (ch))
    {
      chprintln (ch, "You can't replay.");
      return;
    }

  if (buf_string (ch->pcdata->buffer)[0] == '\0')
    {
      chprintln (ch, "You have no tells to replay.");
      return;
    }

  sendpage (ch, buf_string (ch->pcdata->buffer));
  clear_buf (ch->pcdata->buffer);
  ch->pcdata->tells = 0;
}


Do_Fun (do_clantalk)
{
  if (!is_true_clan (ch))
    {
      chprintln (ch, "You aren't in a clan.");
      return;
    }

  public_ch (n_fun, ch, argument, gcn_clan);
}


char
is_eye (char eye)
{
  char eyes[] = { ':', ';', '=', NUL };
  int i;

  for (i = 0; eyes[i] != NUL; i++)
    if (eyes[i] == eye)
      return eyes[i];

  return NUL;
}

char
is_nose (char nose)
{
  char noses[] = { '-', '\'', 'o', '~', '^', NUL };
  int i;

  for (i = 0; noses[i] != NUL; i++)
    if (noses[i] == nose)
      return noses[i];

  return NUL;
}


Do_Fun (do_nosayverbs)
{

  set_on_off (ch, &ch->comm, COMM_NOSAYVERBS,
	      "You no longer see say verbs on channels.",
	      "You now see say verbs on channels again.");
}

const char *
say_verb (const char *word, CharData * ch, CharData * viewer, int S)
{
  size_t len = strlen (word), i, j;

#define sm(a, b, c) return S == 0 ? a : S == 1 ? b : c

  if ((viewer && IsSet (viewer->comm, COMM_NOSAYVERBS)) || len < 3)
    sm ("say", "says", "said");

  if (ch)
    {
      if (IsDrunk (ch))
	sm ("slur", "slurs", "slured");
    }

  j = i = len - 2;
  if (is_nose (word[i]))
    i = len - 3;

  if (is_eye (word[i]))
    {
      switch (word[len - 1])
	{
	case ')':
	  if (word[i] != ';')
	    sm ("smile", "smiles", "smiled");
	  else
	    sm ("leer", "leers", "leered");
	case '}':
	case ']':
	case '>':
	  sm ("grin", "grins", "grinned");
	case '(':
	case '{':
	case '[':
	case '<':
	  if (word[j] == '\'')
	    sm ("sob", "sobs", "sobbed");
	  else
	    sm ("sulk", "sulks", "sulked");
	case '|':
	case '/':
	case '\\':
	  sm ("stare", "stares", "stared");
	case 'P':
	case 'p':
	  sm ("smirk", "smirks", "smirked");
	case 'o':
	case 'O':
	  sm ("sing", "sings", "sung");
	case '$':
	  sm ("blush", "blushes", "blushed");
	case 's':
	case 'S':
	  sm ("blab", "blabs", "blabbed");
	case 'D':
	  sm ("beam", "beams", "beamed");
	case '@':
	  sm ("shout", "shouts", "shouted");
	}
    }
  else
    {
      switch (word[len - 1])
	{
	case '!':
	  if (word[len - 2] != '!')
	    sm ("exclaim", "exclaims", "exclaimed");
	  else
	    sm ("scream", "screams", "screamed");
	case '?':
	  if (word[len - 2] == '!')
	    sm ("boggle", "boggles", "boggled");
	  else if (word[len - 2] != '?')
	    sm ("ask", "asks", "asked");
	  else
	    sm ("demand", "demands", "demanded");
	case '.':
	  if (word[len - 2] == '.' && word[len - 3] == '.')
	    sm ("mutter", "mutters", "muttered");
	  break;
	}
    }

  sm ("say", "says", "said");
}

Do_Fun (do_say)
{
  if (NullStr (argument))
    {
      chprintln (ch, "Say what?");
      return;
    }

  act (CTAG (_SAY1) "$n $t '" CTAG (_SAY2) "$T" CTAG (_SAY1)
       "'{x", ch, say_verb (argument, ch, NULL, 1), argument, TO_ROOM);
  act (CTAG (_SAY1) "You $t '" CTAG (_SAY2) "$T" CTAG (_SAY1)
       "'{x", ch, say_verb (argument, ch, NULL, 0), argument, TO_CHAR);

  if (!IsNPC (ch))
    {
      CharData *mob, *mob_next;
      ObjData *obj, *obj_next;

      for (mob = ch->in_room->person_first; mob != NULL; mob = mob_next)
	{
	  mob_next = mob->next_in_room;
	  if (IsNPC (mob) && HasTriggerMob (mob, TRIG_SPEECH) &&
	      mob->position == mob->pIndexData->default_pos)
	    p_act_trigger (argument, mob, NULL, NULL, ch, NULL, NULL,
			   TRIG_SPEECH);
	  for (obj = mob->carrying_first; obj; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if (HasTriggerObj (obj, TRIG_SPEECH))
		p_act_trigger (argument, NULL, obj, NULL, ch, NULL,
			       NULL, TRIG_SPEECH);
	    }
	}
      for (obj = ch->in_room->content_first; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (HasTriggerObj (obj, TRIG_SPEECH))
	    p_act_trigger (argument, NULL, obj, NULL, ch, NULL, NULL,
			   TRIG_SPEECH);
	}

      if (HasTriggerRoom (ch->in_room, TRIG_SPEECH))
	p_act_trigger (argument, NULL, NULL, ch->in_room, ch, NULL,
		       NULL, TRIG_SPEECH);

    }

  return;
}

Do_Fun (do_sooc)
{
  if (NullStr (argument))
    {
      chprintln (ch, "Say what Out of Character?");
      return;
    }

  act ("$n says out of character '$T'", ch, NULL, argument, TO_ROOM);
  act ("You say out of character '$T'", ch, NULL, argument, TO_CHAR);
  return;
}

Do_Fun (do_whisper)
{
  char arg[MIL];
  CharData *victim;

  argument = first_arg (argument, arg, false);

  if (NullStr (arg))
    {
      chprintln (ch, "Whisper to who?");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Whisper what?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  act ("$n whispers to you '{M$t{x'", ch, argument, victim, TO_VICT);
  act ("You whisper to $N '{M$t{x'", ch, argument, victim, TO_CHAR);
  act ("{M$n{M whispers something to {M$N{M.{x", ch, NULL, victim,
       TO_NOTVICT);
  return;
}


Do_Fun (do_think)
{
  if (NullStr (argument))
    {
      chprintln (ch, "You don't knwo what to think.");
      return;
    }


  act ("{C$n{C . o O ( $T{C ){x", ch, NULL, argument, TO_ROOM);
  act ("{CYou . o O ( $T{C ){x", ch, NULL, argument, TO_CHAR);
  return;
}

Do_Fun (do_shout)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      set_on_off (ch, &ch->comm, COMM_SHOUTSOFF,
		  "You can hear shouts again.",
		  "You will no longer hear shouts.");
      return;
    }

  if (IsSet (ch->comm, COMM_NOSHOUT))
    {
      chprintln (ch, "You can't shout.");
      return;
    }

  RemBit (ch->comm, COMM_SHOUTSOFF);

  WaitState (ch, 12);

  act (CTAG (_SHOUT1) "You shout '" CTAG (_SHOUT2) "$T"
       CTAG (_SHOUT1) "'{x", ch, NULL, swearcheck (argument, ch), TO_CHAR);
  for (d = descriptor_first; d != NULL; d = d->next)
    {
      CharData *victim;

      victim = d->original ? d->original : d->character;

      if (d->connected == CON_PLAYING && d->character != ch &&
	  !IsSet (victim->comm, COMM_SHOUTSOFF) &&
	  !is_ignoring (victim, ch->name, IGNORE_CHANNELS) &&
	  !IsSet (victim->comm, COMM_QUIET))
	{
	  act (CTAG (_SHOUT1) "$n shouts '" CTAG (_SHOUT2) "$t"
	       CTAG (_SHOUT1) "'{x", ch, swearcheck (argument, d->character),
	       d->character, TO_VICT);
	}
    }

  return;
}

#define TELLMSG1    CTAG(_TELLS1) "You tell %s '" CTAG(_TELLS2) "%s" \
    	    	    	CTAG(_TELLS1) "'{x"

#define TELLMSG2    CTAG(_TELLS1) MXPTAG("Tell '%s'") "%s" MXPTAG("/Tell") \
    	    	    	" tells you '" CTAG(_TELLS2) "%s" CTAG(_TELLS1) "'{x"

Do_Fun (do_tell)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  if (IsSet (ch->comm, COMM_NOTELL) || IsSet (ch->comm, COMM_DEAF))
    {
      chprintln (ch, "Your message didn't get through.");
      return;
    }

  if (IsSet (ch->comm, COMM_QUIET))
    {
      chprintln (ch, "You must turn off quiet mode first.");
      return;
    }

  if (IsSet (ch->comm, COMM_DEAF))
    {
      chprintln (ch, "You must turn off deaf mode first.");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      chprintln (ch, "Tell whom what?");
      return;
    }


  if ((victim = get_char_world (ch, arg)) == NULL ||
      (IsNPC (victim) && victim->in_room != ch->in_room))
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (ch == victim)
    {
      chprintln (ch, "You tell yourself to get a life.");
      return;
    }

  if (is_ignoring (victim, ch->name, IGNORE_TELLS))
    {
      act_new ("$N doesn't seem to be listening to you.", ch, NULL,
	       victim, TO_CHAR, POS_DEAD);
      return;
    }

  if (!(IsImmortal (ch) && ch->level > LEVEL_IMMORTAL) && !IsAwake (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }

  if ((IsSet (victim->comm, COMM_QUIET) ||
       IsSet (victim->comm, COMM_DEAF)) && !IsImmortal (ch))
    {
      act ("$E is not receiving tells.", ch, 0, victim, TO_CHAR);
      return;
    }

  if (victim->desc == NULL && !IsNPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      bprintlnf (victim->pcdata->buffer, TELLMSG2, argument,
		 Pers (ch, victim), argument);
      victim->pcdata->tells++;
      return;
    }


  if (IsSet (victim->comm, COMM_AFK))
    {
      if (IsNPC (victim))
	{
	  act ("$E is AFK, and not receiving tells.", ch, NULL,
	       victim, TO_CHAR);
	  return;
	}

      act
	("$E is AFK ($t), but your tell will go through when $E returns.",
	 ch, victim->pcdata->afk_msg, victim, TO_CHAR);
      bprintlnf (victim->pcdata->buffer, TELLMSG2, argument,
		 Pers (ch, victim), argument);
      victim->pcdata->tells++;
      return;
    }

  chprintlnf (ch, TELLMSG1, Pers (victim, ch), argument);
  chprintlnf (victim, TELLMSG2, argument, Pers (ch, victim), argument);

  victim->reply = ch;

  if (!IsNPC (ch) && IsNPC (victim) && HasTriggerMob (victim, TRIG_SPEECH))
    p_act_trigger (argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH);

  return;
}

Do_Fun (do_reply)
{
  CharData *victim;

  if (IsSet (ch->comm, COMM_NOTELL))
    {
      chprintln (ch, "Your message didn't get through.");
      return;
    }

  if ((victim = ch->reply) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (ch == victim)
    {
      chprintln (ch, "You tell yourself to get a life.");
      return;
    }

  if (is_ignoring (victim, ch->name, IGNORE_TELLS))
    {
      act_new ("$N doesn't seem to be listening to you.", ch, NULL,
	       victim, TO_CHAR, POS_DEAD);
      return;
    }

  if (!IsImmortal (ch) && !IsAwake (victim))
    {
      act ("$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
    }

  if ((IsSet (victim->comm, COMM_QUIET) ||
       IsSet (victim->comm, COMM_DEAF)) && !IsImmortal (ch) &&
      !IsImmortal (victim))
    {
      act_new ("$E is not receiving tells.", ch, 0, victim, TO_CHAR,
	       POS_DEAD);
      return;
    }

  if (!IsImmortal (victim) && !IsAwake (ch))
    {
      chprintln (ch, "In your dreams, or what?");
      return;
    }

  if (victim->desc == NULL && !IsNPC (victim))
    {
      act ("$N seems to have misplaced $S link...try again later.", ch,
	   NULL, victim, TO_CHAR);
      bprintlnf (victim->pcdata->buffer, TELLMSG2, argument,
		 Pers (ch, victim), argument);
      victim->pcdata->tells++;
      return;
    }

  if (IsSet (victim->comm, COMM_AFK))
    {
      if (IsNPC (victim))
	{
	  act_new ("$E is AFK, and not receiving tells.", ch, NULL,
		   victim, TO_CHAR, POS_DEAD);
	  return;
	}

      act_new
	("$E is AFK ($t), but your tell will go through when $E returns.",
	 ch, victim->pcdata->afk_msg, victim, TO_CHAR, POS_DEAD);
      bprintlnf (victim->pcdata->buffer, TELLMSG2, argument,
		 Pers (ch, victim), argument);
      victim->pcdata->tells++;
      return;
    }

  chprintlnf (ch, TELLMSG1, Pers (victim, ch), argument);
  chprintlnf (victim, TELLMSG2, argument, Pers (ch, victim), argument);
  victim->reply = ch;

  return;
}

Do_Fun (do_yell)
{
  Descriptor *d;

  if (IsSet (ch->comm, COMM_NOSHOUT))
    {
      chprintln (ch, "You can't yell.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Yell what?");
      return;
    }

  act ("You yell '$t'", ch, swearcheck (argument, ch), NULL, TO_CHAR);
  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character != ch &&
	  d->character->in_room != NULL &&
	  d->character->in_room->area == ch->in_room->area &&
	  !is_ignoring (d->character, ch->name, IGNORE_CHANNELS) &&
	  !IsSet (d->character->comm, COMM_QUIET))
	{
	  act ("$n yells '$t'", ch, swearcheck (argument, d->character),
	       d->character, TO_VICT);
	}
    }

  return;
}

Do_Fun (do_emote)
{
  if (!IsNPC (ch) && IsSet (ch->comm, COMM_NOEMOTE))
    {
      chprintln (ch, "You can't show your emotions.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Emote what?");
      return;
    }

  MOBtrigger = false;
  act ("$n $T", ch, NULL, argument, TO_ROOM);
  act ("$n $T", ch, NULL, argument, TO_CHAR);
  MOBtrigger = true;
  return;
}

Do_Fun (do_pmote)
{
  CharData *vch;
  const char *letter, *name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  unsigned int matches = 0;

  if (!IsNPC (ch) && IsSet (ch->comm, COMM_NOEMOTE))
    {
      chprintln (ch, "You can't show your emotions.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Emote what?");
      return;
    }

  act ("$n $t", ch, argument, NULL, TO_CHAR);

  for (vch = ch->in_room->person_first; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->desc == NULL || vch == ch)
	continue;

      if ((letter = strstr (argument, vch->name)) == NULL)
	{
	  MOBtrigger = false;
	  act ("$N $t", vch, argument, ch, TO_CHAR);
	  MOBtrigger = true;
	  continue;
	}

      strcpy (temp, argument);
      temp[strlen (argument) - strlen (letter)] = '\0';
      last[0] = '\0';
      name = vch->name;

      for (; *letter != '\0'; letter++)
	{
	  if (*letter == '\'' && matches == strlen (vch->name))
	    {
	      strcat (temp, "r");
	      continue;
	    }

	  if (*letter == 's' && matches == strlen (vch->name))
	    {
	      matches = 0;
	      continue;
	    }

	  if (matches == strlen (vch->name))
	    {
	      matches = 0;
	    }

	  if (*letter == *name)
	    {
	      matches++;
	      name++;
	      if (matches == strlen (vch->name))
		{
		  strcat (temp, "you");
		  last[0] = '\0';
		  name = vch->name;
		  continue;
		}
	      strncat (last, letter, 1);
	      continue;
	    }

	  matches = 0;
	  strcat (temp, last);
	  strncat (temp, letter, 1);
	  last[0] = '\0';
	  name = vch->name;
	}

      MOBtrigger = false;
      act ("$N $t", vch, temp, ch, TO_CHAR);
      MOBtrigger = true;
    }

  return;
}

Do_Fun (do_bug)
{
  char buf[MSL];

  sprintf (buf, "[%ld] %s: %s", ch->in_room->vnum, ch->name, argument);

  append_file (BUG_FILE, buf, true);

  chprintln (ch, "Bug logged.");
  return;
}

Do_Fun (do_typo)
{
  char buf[MSL];

  sprintf (buf, "[%ld] %s: %s", ch->in_room->vnum, ch->name, argument);

  append_file (TYPO_FILE, buf, true);
  chprintln (ch, "Typo logged.");
  return;
}

Do_Fun (do_rent)
{
  chprintln (ch, "There is no rent here.  Just save and quit.");
  return;
}

Do_Fun (do_quit)
{
  Descriptor *d, *d_next;
  int id;
  char *const quit_message[] = {
    "Alas, all good things must come to an end.",
    "{RYou have been KILLED!!!{x",
    "What???? You're not addicted yet?????",
    "Hope to see you soon!"
  };

  if (IsNPC (ch))
    return;

  if (ch->position == POS_FIGHTING)
    {
      chprintln (ch, "No way! You are fighting.");
      return;
    }

  if (ch->position < POS_STUNNED)
    {
      chprintln (ch, "You're not DEAD yet.");
      return;
    }
  if (has_auction (ch))
    {
      chprintln
	(ch,
	 "{YThe Auctioneer will not let you leave while you have an auction running.{x");
      return;
    }

  chprintln (ch,
	     quit_message[sizeof (quit_message) / sizeof (quit_message[0]) -
			  1]);
  announce (ch, INFO_LOGOUT, "$n has left the realms.");
  act ("$n has left the game.", ch, NULL, NULL, TO_ROOM);
  logf ("%s has quit.", ch->name);
  wiznet ("$N rejoins the real world.", ch, NULL, WIZ_LOGINS, false,
	  get_trust (ch));


  save_char_obj (ch);


  if (ch->pcdata->in_progress)
    free_note (ch->pcdata->in_progress);

  extract_quest (ch);
  extract_war (ch);
  extract_arena (ch);

  id = ch->id;
  d = ch->desc;
  extract_char (ch, true);
  if (d != NULL)
    close_socket (d);


  for (d = descriptor_first; d != NULL; d = d_next)
    {
      CharData *tch;

      d_next = d->next;
      tch = d->original ? d->original : d->character;
      if (tch && tch->id == id)
	{
	  extract_char (tch, true);
	  close_socket (d);
	}
    }

  return;
}

Do_Fun (do_save)
{
  if (IsNPC (ch))
    return;

  if (get_trust (ch) < mud_info.min_save_lvl)
    {
      chprintlnf (ch,
		  "You must be level %d before you can save your character.",
		  mud_info.min_save_lvl);
      return;
    }
  save_char_obj (ch);
  update_statlist (ch, false);
  if (is_clan (ch))
    update_members (ch, false);
  if (IsImmortal (ch))
    update_wizlist (ch, ch->level);
  chprintlnf (ch, "Saving. Remember that %s has automatic saving.",
	      mud_info.name);
  WaitState (ch, PULSE_VIOLENCE / 2);
  return;
}

Do_Fun (do_backup)
{
  if (IsNPC (ch))
    return;

  if (!str_cmp (argument, "check"))
    {
      if (ch->pcdata->played - ch->pcdata->backup >= HOUR)
	{
	  if (ch->pcdata->backup == 0)
	    {
	      chprintln
		(ch, "{RThere is currently no backup for your character.{x");
	    }
	  else if (ch->pcdata->played - ch->pcdata->backup >= (HOUR * 24))
	    {
	      chprintlnf
		(ch, "{RYou have not backed up for {W"
		 "%ld{R hours of gameplay.{x",
		 (ch->pcdata->played - ch->pcdata->backup) / HOUR);
	    }
	  else
	    {
	      chprintlnf
		(ch, "{RYou have not backed up for {W"
		 "%ld{R hour%s of gameplay.{x",
		 (ch->pcdata->played - ch->pcdata->backup) / HOUR,
		 (ch->pcdata->played - ch->pcdata->backup) / HOUR ==
		 1 ? "" : "s");
	    }

	}
      return;
    }
  if (get_trust (ch) < mud_info.min_save_lvl * 3)
    {
      chprintlnf (ch,
		  "You must be level %d before you can backup your character.",
		  mud_info.min_save_lvl * 3);
      return;
    }
  backup_char_obj (ch);
  update_statlist (ch, false);
  if (is_clan (ch))
    update_members (ch, false);
  chprintlnf (ch, "%s has been saved to a backup.", ch->name);
  WaitState (ch, PULSE_VIOLENCE);
  return;
}

Do_Fun (do_follow)
{

  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Follow whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM) && ch->master != NULL)
    {
      act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      if (ch->master == NULL)
	{
	  chprintln (ch, "You already follow yourself.");
	  return;
	}
      stop_follower (ch);
      return;
    }

  if (!IsNPC (victim) && IsSet (victim->act, PLR_NOFOLLOW)
      && !IsImmortal (ch))
    {
      act ("$N doesn't seem to want any followers.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }

  RemBit (ch->act, PLR_NOFOLLOW);

  if (ch->master != NULL)
    stop_follower (ch);

  add_follower (ch, victim);
  return;
}

Do_Fun (do_ditch)
{
  char arg[MIL];
  CharData *victim;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Ditch whom?");
      return;
    }
  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }
  if (IsAffected (ch, AFF_CHARM) && ch->master != NULL)
    {
      act ("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
    }
  if (victim == ch)
    {
      chprintln (ch, "You try to ditch yourself.... unsuccessfuly.");
      return;
    }
  if (victim->master != ch)
    {
      chprintln (ch, "They aren't following you.");
      return;
    }

  stop_follower (victim);
  return;
}

void
add_follower (CharData * ch, CharData * master)
{
  if (ch->master != NULL)
    {
      bug ("Add_follower: non-null master.");
      return;
    }

  ch->master = master;
  ch->leader = NULL;

  if (can_see (master, ch))
    act ("$n now follows you.", ch, NULL, master, TO_VICT);

  act ("You now follow $N.", ch, NULL, master, TO_CHAR);

  return;
}

void
stop_follower (CharData * ch)
{
  if (ch->master == NULL)
    {
      bug ("Stop_follower: null master.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM))
    {
      RemBit (ch->affected_by, AFF_CHARM);
      affect_strip (ch, gsn_charm_person);
    }

  if (can_see (ch->master, ch) && ch->in_room != NULL)
    {
      act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
      act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);
    }
  if (ch->master->pet == ch)
    ch->master->pet = NULL;

  ch->master = NULL;
  ch->leader = NULL;
  return;
}


void
nuke_pets (CharData * ch)
{
  CharData *pet;

  if ((pet = ch->pet) != NULL)
    {
      stop_follower (pet);
      if (pet->in_room != NULL)
	act ("$N slowly fades away.", ch, NULL, pet, TO_NOTVICT);
      extract_char (pet, true);
    }
  ch->pet = NULL;

  return;
}

void
die_follower (CharData * ch)
{
  CharData *fch;

  if (ch->master != NULL)
    {
      if (ch->master->pet == ch)
	ch->master->pet = NULL;
      stop_follower (ch);
    }

  ch->leader = NULL;

  for (fch = char_first; fch != NULL; fch = fch->next)
    {
      if (fch->master == ch)
	stop_follower (fch);
      if (fch->leader == ch)
	fch->leader = fch;
    }

  return;
}

Do_Fun (do_order)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  CharData *och;
  CharData *och_next;
  bool found;
  bool fAll;

  argument = one_argument (argument, arg);
  one_argument (argument, arg2);

  if (!str_cmp (arg2, "delete") || !str_cmp (arg2, "mob"))
    {
      chprintln (ch, "That will NOT be done.");
      return;
    }

  if (NullStr (arg) || NullStr (argument))
    {
      chprintln (ch, "Order whom to do what?");
      return;
    }

  if (IsAffected (ch, AFF_CHARM))
    {
      chprintln (ch, "You feel like taking, not giving, orders.");
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      fAll = true;
      victim = NULL;
    }
  else
    {
      fAll = false;
      if ((victim = get_char_room (ch, NULL, arg)) == NULL)
	{
	  chprintln (ch, "They aren't here.");
	  return;
	}

      if (victim == ch)
	{
	  chprintln (ch, "Aye aye, right away!");
	  return;
	}

      if (!IsAffected (victim, AFF_CHARM) || victim->master != ch ||
	  (IsImmortal (victim) && victim->trust >= ch->trust))
	{
	  chprintln (ch, "Do it yourself!");
	  return;
	}
    }

  found = false;
  for (och = ch->in_room->person_first; och != NULL; och = och_next)
    {
      och_next = och->next_in_room;

      if (IsAffected (och, AFF_CHARM) && och->master == ch &&
	  (fAll || och == victim))
	{
	  found = true;
	  sprintf (buf, "$n orders you to '%s'.", argument);
	  act (buf, ch, NULL, och, TO_VICT);
	  interpret (och, argument);
	}
    }

  if (found)
    {
      WaitState (ch, PULSE_VIOLENCE);
      chprintln (ch, "Ok.");
    }
  else
    chprintln (ch, "You have no followers here.");
  return;
}

Do_Fun (do_group)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      CharData *gch;
      CharData *leader;

      leader = (ch->leader != NULL) ? ch->leader : ch;
      chprintlnf (ch, "%s's group:", Pers (leader, ch));

      for (gch = char_first; gch != NULL; gch = gch->next)
	{
	  if (is_same_group (gch, ch))
	    {
	      chprintlnf (ch,
			  "[%2d %s] %-16s %4ld/%4ld hp %4ld/%4ld mana %4ld/%4ld mv %5d xp",
			  gch->level,
			  IsNPC (gch) ? "Mob" : class_who (gch),
			  capitalize (Pers (gch, ch)), gch->hit,
			  gch->max_hit, gch->mana, gch->max_mana,
			  gch->move, gch->max_move, gch->exp);
	    }
	}
      chprintlnf (ch, "Type '%s where' to view group member locations.",
		  n_fun);
      return;
    }
  else if (!str_cmp (arg, "where"))
    {
      CharData *gch, *leader;

      leader = (ch->leader != NULL) ? ch->leader : ch;
      chprintlnf (ch, "{W%s's group:{x", Pers (leader, ch));
      for (gch = char_first; gch != NULL; gch = gch->next)
	{
	  if (is_same_group (gch, ch))
	    {
	      chprintlnf (ch, "{W%s is in %s the general area of %s.{x",
			  Pers (gch, ch), gch->in_room->name,
			  gch->in_room->area->name);
	    }
	}
      return;
    }
  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
    {
      chprintln (ch, "But you are following someone else!");
      return;
    }

  if (victim->master != ch && ch != victim)
    {
      act_new ("$N isn't following you.", ch, NULL, victim, TO_CHAR,
	       POS_SLEEPING);
      return;
    }

  if (IsAffected (victim, AFF_CHARM))
    {
      chprintln (ch, "You can't remove charmed mobs from your group.");
      return;
    }

  if (IsAffected (ch, AFF_CHARM))
    {
      act_new ("You like your master too much to leave $m!", ch, NULL,
	       victim, TO_VICT, POS_SLEEPING);
      return;
    }

  if (is_same_group (victim, ch) && ch != victim)
    {
      victim->leader = NULL;
      act_new ("$n removes $N from $s group.", ch, NULL, victim,
	       TO_NOTVICT, POS_RESTING);
      act_new ("$n removes you from $s group.", ch, NULL, victim,
	       TO_VICT, POS_SLEEPING);
      act_new ("You remove $N from your group.", ch, NULL, victim,
	       TO_CHAR, POS_SLEEPING);
      return;
    }

  victim->leader = ch;
  act_new ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT, POS_RESTING);
  act_new ("You join $n's group.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
  act_new ("$N joins your group.", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
  return;
}


Do_Fun (do_split)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CharData *gch;
  int members;
  money_t amount_gold = 0, amount_silver = 0;
  money_t share_gold, share_silver;
  money_t extra_gold, extra_silver;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      chprintln (ch, "Split how much?");
      return;
    }

  amount_silver = atoi (arg1);

  if (!NullStr (arg2))
    amount_gold = atoi (arg2);

  if (amount_gold < 0 || amount_silver < 0)
    {
      chprintln (ch, "Your group wouldn't like that.");
      return;
    }

  if (amount_gold == 0 && amount_silver == 0)
    {
      chprintln (ch, "You hand out zero coins, but no one notices.");
      return;
    }

  if (ch->gold < amount_gold || ch->silver < amount_silver)
    {
      chprintln (ch, "You don't have that much to split.");
      return;
    }

  members = 0;
  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      if (is_same_group (gch, ch) && !IsAffected (gch, AFF_CHARM))
	members++;
    }

  if (members < 2)
    {
      chprintln (ch, "Just keep it all.");
      return;
    }

  share_silver = amount_silver / members;
  extra_silver = amount_silver % members;

  share_gold = amount_gold / members;
  extra_gold = amount_gold % members;

  if (share_gold == 0 && share_silver == 0)
    {
      chprintln (ch, "Don't even bother, cheapskate.");
      return;
    }

  ch->silver -= amount_silver;
  ch->silver += share_silver + extra_silver;
  ch->gold -= amount_gold;
  ch->gold += share_gold + extra_gold;

  if (share_silver > 0)
    {
      chprintlnf (ch,
		  "You split %ld silver coins. Your share is %ld silver.",
		  amount_silver, share_silver + extra_silver);
    }

  if (share_gold > 0)
    {
      chprintlnf (ch,
		  "You split %ld gold coins. Your share is %ld gold.",
		  amount_gold, share_gold + extra_gold);
    }

  if (share_gold == 0)
    {
      sprintf (buf,
	       "$n splits %ld silver coins. Your share is %ld silver.",
	       amount_silver, share_silver);
    }
  else if (share_silver == 0)
    {
      sprintf (buf, "$n splits %ld gold coins. Your share is %ld gold.",
	       amount_gold, share_gold);
    }
  else
    {
      sprintf (buf,
	       "$n splits %ld silver and %ld gold coins, giving you %ld silver and %ld gold.",
	       amount_silver, amount_gold, share_silver, share_gold);
    }

  for (gch = ch->in_room->person_first; gch != NULL; gch = gch->next_in_room)
    {
      if (gch != ch && is_same_group (gch, ch)
	  && !IsAffected (gch, AFF_CHARM))
	{
	  act (buf, ch, NULL, gch, TO_VICT);
	  gch->gold += share_gold;
	  gch->silver += share_silver;
	}
    }

  return;
}

Do_Fun (do_gtell)
{
  CharData *gch;

  if (NullStr (argument))
    {
      chprintln (ch, "Tell your group what?");
      return;
    }

  if (IsSet (ch->comm, COMM_NOTELL))
    {
      chprintln (ch, "Your message didn't get through!");
      return;
    }

  for (gch = char_first; gch != NULL; gch = gch->next)
    {
      if (is_same_group (gch, ch))
	act_new (CTAG (_GTELL1) "$n tells the group '"
		 CTAG (_GTELL2) "$t" CTAG (_GTELL1) "'{x", ch,
		 argument, gch, TO_VICT, POS_SLEEPING);
    }

  return;
}


bool
is_same_group (CharData * ach, CharData * bch)
{
  if (ach == NULL || bch == NULL)
    return false;

  if (ach->leader != NULL)
    ach = ach->leader;
  if (bch->leader != NULL)
    bch = bch->leader;
  return ach == bch;
}

flag_t
ignore_bit (flag_t info)
{
  switch (info)
    {
    case INFO_LEVEL:
      return IGNORE_LEVELS;
    default:
      return IGNORE_ANNOUNCE;
    }
}

void
announce (CharData * ch, flag_t bit, const char *message, ...)
{
  Descriptor *d;
  char buf[MSL], buf2[MSL];
  char *iType;
  bool Private = false;
  va_list args;

  if (NullStr (message))
    return;

  va_start (args, message);
  vsnprintf (buf2, sizeof (buf2), message, args);
  va_end (args);

  if (IsSet (bit, INFO_PRIVATE))
    {
      if (ch)
	Private = true;
      RemBit (bit, INFO_PRIVATE);
    }

  if (bit == INFO_NOTE)
    iType = "{GBOARD";
  else if (bit == INFO_WAR)
    iType = "{CWar";
  else if (bit == INFO_GQUEST)
    iType = "{MGquest";
  else if (bit == INFO_AUCTION)
    iType = "{YAUCTION";
  else if (bit == INFO_ARENA)
    iType = "{BARENA";
  else
    iType = "{RINFO";

  sprintf (buf, "%s{W:{x %s{x", iType, buf2);

  if (!Private)
    {
      for (d = descriptor_first; d; d = d->next)
	{
	  CharData *och;

	  if ((och = d->character) == NULL)
	    continue;
	  if (d->connected != CON_PLAYING)
	    continue;
	  if (bit == INFO_NOTE && ch &&
	      !is_note_to (och, ch->pcdata->in_progress))
	    continue;
	  if (IsSet (och->info_settings, bit) && bit != INFO_ALL)
	    continue;

	  if (ch == NULL)
	    chprintln (och, buf);
	  else if (!is_ignoring (och, ch->name, ignore_bit (bit)))
	    act_new (buf, ch, NULL, och, TO_VICT, POS_DEAD);
	}
    }
  else
    {
      if (!ch)
	{
	  bug ("NULL ch in private announce");
	  return;
	}
      chprintln (ch, buf);
    }
  return;
}

Do_Fun (do_info)
{
  int i;
  char arg[MIL];
  flag_t iValue;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || !str_cmp (arg, "list"))
    {
      chprintln (ch, "{RInformation settings:");
      chprintln (ch, "{r---------------------{x");
      for (i = 0; info_flags[i].name != NULL; i++)
	{
	  chprintlnf (ch, "%-13.25s [%s]",
		      info_flags[i].name,
		      IsSet (ch->info_settings,
			     info_flags[i].bit) ? "{WOFF{x" : " {RON{x");
	}
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      bool fOn;

      if (!str_cmp (argument, "on"))
	fOn = true;
      else if (!str_cmp (argument, "off"))
	fOn = false;
      else
	{
	  chprintln (ch, "Turn all info channels on or off?");
	  return;
	}

      for (iValue = 0; info_flags[iValue].name != NULL; iValue++)
	{
	  if (fOn)
	    SetBit (ch->info_settings, info_flags[iValue].bit);
	  else
	    RemBit (ch->info_settings, info_flags[iValue].bit);
	}

      chprintlnf (ch, "All info channels turned %s.",
		  fOn ? "{Ron{x" : "{Roff{x");
      return;
    }

  if ((iValue = flag_value (info_flags, arg)) != NO_FLAG)
    {
      ToggleBit (ch->info_settings, iValue);
      chprintln (ch, "Info channel set.");
      return;
    }
  chprintln (ch, "Which option??");
  return;
}

Do_Fun (do_announce)
{
  announce (NULL, INFO_ALL, argument);
}
