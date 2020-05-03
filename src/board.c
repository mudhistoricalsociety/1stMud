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
* Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@andreasen.org  *
* Basically, the notes are split up into several boards. The boards do    *
* not exist physically, they can be read anywhere and in any position.    *
* Each of the note boards has its own file. Each of the boards can have   *
* its own "rights": who can read/write.                                   *
* Each character has an extra field added, namele the timestamp of the    *
* last note read by him/her on a certain board.                           *
* The note entering system is changed too, making it more interactive.    *
* entering a note, a character is put AFK and into a special CON_ state.  *
* Everything typed goes into the note.                                    *
* For the immortals it is possible to purge notes based on age. An        *
* archive options is available which moves the notes older than X days    *
* into a special board. The file of this board should then be moved into  *
* some other directory during e.g. the startup script and perhaps renamed *
* depending on date.                                                      *
* Note that write_level MUST be >= read_level or else there will be       *
* strange output in certain functions.                                    *
* Board DEFAULT_BOARD must be at least readable by *everyone*.			  *
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
#include "data_table.h"



#define L_SUP (MAX_LEVEL - 1)

BoardData boards[MAX_BOARD] = {

  {"Announce", "Announcements from Immortals", 0, L_SUP, "all", DEF_NORMAL,

   60, NULL, NULL, BOARD_NONE}
  ,
  {"General", "General discussion", 0, 2, "all", DEF_INCLUDE, 21, NULL,
   NULL,
   BOARD_NONE}
  ,
  {"Ideas", "Suggestion for improvement", 0, 2, "all", DEF_NORMAL, 60, NULL,

   NULL,
   BOARD_NONE}
  ,

  {"Bugs", "Typos, bugs, errors", 0, 1, "imm", DEF_NORMAL, 60, NULL, NULL,

   BOARD_NONE}
  ,


  {"Games", "Mud Games, Global Quests, ect.", 0, 1, "all", DEF_NORMAL,
   10, NULL, NULL,
   BOARD_NONE}
  ,
  {"Personal", "Personal messages", 0, 1, "all", DEF_EXCLUDE, 28, NULL,
   NULL,
   BOARD_NOWEB}
  ,

  {"Immortal", "Immortals only", LEVEL_IMMORTAL, LEVEL_IMMORTAL, "imm",
   DEF_INCLUDE, 21, NULL, NULL, BOARD_NOWEB}

};


const char *szFinishPrompt =
  "({WC{x)ontinue, ({WV{x)iew, ({WP{x)ost or ({WF{x)orget it?";

long last_note_stamp = 0;


void
finish_note (BoardData * board, NoteData * note)
{


  if (!NullStr (note->reply_text))
    replace_strf (&note->text, "%s" NEWLINE "%s", note->reply_text,
		  note->text);

  if (last_note_stamp >= current_time)
    note->date_stamp = ++last_note_stamp;
  else
    {
      note->date_stamp = current_time;
      last_note_stamp = current_time;
    }

  Link (note, board->note, next, prev);

  SetBit (board->flags, BOARD_CHANGED);
}


int
board_number (const BoardData * board)
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    if (board == &boards[i])
      return i;

  return -1;
}


Lookup_Fun (board_lookup)
{
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    if (!str_cmp (boards[i].short_name, name))
      return i;

  return -1;
}


static void
unlink_note (BoardData * board, NoteData * note)
{
  UnLink (note, board->note, next, prev);
}


static NoteData *
find_note (CharData * ch, BoardData * board, int num)
{
  int count = 0;
  NoteData *p;

  for (p = board->note_first; p; p = p->next)
    if (++count == num)
      break;

  if ((count == num) && is_note_to (ch, p))
    return p;
  else
    return NULL;

}


static void
show_note_to_char (CharData * ch, NoteData * note, int num)
{
  Buffer *buffer;

  buffer = new_buf ();

  bprintlnf
    (buffer,
     NEWLINE "{YBoard{x: %s" NEWLINE
     "[{x%4d{x] {Y%s{x: {g%s{x" NEWLINE
     "{YDate{x:  %s" NEWLINE
     "{YTo{x:    %s",
     ch->pcdata->board->short_name, num, note->sender, note->subject,
     note->date, note->to_list);
  bprintlnf (buffer, "{g%s{x" NEWLINE "%s{g%s{x",
	     draw_line (ch, "{W={w=", 0), note->text, draw_line (ch,
								 "{W={w=",
								 0));

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
}

static void
check_notes (BoardData * b)
{
  NoteData *p, *p_next;

  for (p = b->note_first; p; p = p_next)
    {
      p_next = p->next;
      if (p->expire < current_time)
	{
	  UnLink (p, b->note, next, prev);
	  free_note (p);
	  SetBit (b->flags, BOARD_CHANGED);
	}
    }
}

TableSave_Fun (rw_note_data)
{
  char file[MIL];
  BoardData *board;
  int i;

  for (i = 0; i < MAX_BOARD; i++)
    {
      board = &boards[i];

      if (type == act_write && !IsSet (board->flags, BOARD_CHANGED))
	continue;
      sprintf (file, NOTE_DIR "%s", board->short_name);
      RemBit (board->flags, BOARD_CHANGED);
      logf ("%s %s data...", type == act_read ? "Loading" : "Saving",
	    board->short_name);
      rw_sublist (type, file, NoteData, board, note);
      if (type == act_read)
	check_notes (board);
    }
}


bool
is_note_to (CharData * ch, NoteData * note)
{
  if (!str_cmp (ch->name, note->sender))
    return true;

  if (is_ignoring (ch, note->to_list, IGNORE_NOTES))
    return false;

  if (is_full_name ("all", note->to_list))
    return true;

  if (IsImmortal (ch) &&
      (is_full_name ("imm", note->to_list) ||
       is_full_name ("imms", note->to_list) ||
       is_full_name ("immortal", note->to_list) ||
       is_full_name ("god", note->to_list) ||
       is_full_name ("gods", note->to_list) ||
       is_full_name ("immortals", note->to_list)))
    return true;

  if ((get_trust (ch) == MAX_LEVEL) &&
      (is_full_name ("imp", note->to_list) ||
       is_full_name ("imps", note->to_list) ||
       is_full_name ("implementor", note->to_list) ||
       is_full_name ("implementors", note->to_list)))
    return true;

  if (is_clan (ch)
      && (is_exact_name ("clan", note->to_list)
	  || is_exact_name (CharClan (ch)->name, note->to_list)))
    return true;

  if (is_full_name (ch->name, note->to_list))
    return true;


  if (is_number (note->to_list) && get_trust (ch) >= atoi (note->to_list))
    return true;

  return false;
}



int
unread_notes (CharData * ch, BoardData * board)
{
  NoteData *note;
  time_t last_read;
  int count = 0;

  if (board->read_level > get_trust (ch))
    return BOARD_NOACCESS;

  last_read = ch->pcdata->last_note[board_number (board)];

  for (note = board->note_first; note; note = note->next)
    if (is_note_to (ch, note) && ((long) last_read < (long) note->date_stamp))
      count++;

  return count;
}


NoteData *
last_note (CharData * ch, BoardData * board)
{
  NoteData *note;

  if (board->read_level > get_trust (ch))
    return NULL;

  for (note = board->note_last; note; note = note->prev)
    if (is_note_to (ch, note))
      return note;

  return NULL;
}


Do_Fun (do_ncheck)
{
  int i, count = 0, unread = 0;

  for (i = 0; i < MAX_BOARD; i++)
    {
      unread = unread_notes (ch, &boards[i]);
      if (unread != BOARD_NOACCESS && !ch->pcdata->unsubscribed[i])
	count += unread;
    }

  if (count < 1)
    chprintln (ch, "You have no new notes on the board.");
  else
    chprintlnf (ch, "You have %s on the board. Type 'board'.",
		intstr (count, "note"));


  if (ch->pcdata->in_progress)
    chprintln (ch,
	       "You have a note in progress. Type 'NOTE WRITE' to continue it.");
}

const char *
format_recipient (CharData * ch, const char *names)
{

  if (is_exact_name ("clan", names) && is_clan (ch))
    return str_rep (names, "clan", CharClan (ch)->name);

  return names;
}




static
Do_Fun (do_nwrite)
{
  if (IsNPC (ch))
    return;

  if (get_trust (ch) < ch->pcdata->board->write_level)
    {
      chprintln (ch, "You cannot post notes on this board.");
      return;
    }

  if (get_trust (ch) < ch->pcdata->board->write_level ||
      ch->pcdata->board->force_type == DEF_READONLY)
    {
      chprintln (ch, "You cannot post notes on this board.");
      return;
    }


  if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text))
    {
      chprintln
	(ch,
	 "Note in progress cancelled because you did not manage to write any text "
	 NEWLINE "before losing link.");
      free_note (ch->pcdata->in_progress);
      ch->pcdata->in_progress = NULL;
    }

  if (!ch->pcdata->in_progress)
    {
      ch->pcdata->in_progress = new_note ();
      replace_str (&ch->pcdata->in_progress->sender, ch->name);

      replace_str (&ch->pcdata->in_progress->date,
		   str_time (current_time, -1, NULL));
    }

  act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_ROOM);


  chprintlnf (ch,
	      "You are now %s a new note on the {W%s{x board." NEWLINE
	      "If you are using tintin, type #verbose to turn off alias expansion!",
	      ch->pcdata->in_progress->text ? "continuing" : "posting",
	      ch->pcdata->board->short_name);

  chprintlnf (ch, "{YFrom{x:    %s", ch->name);

  if (!ch->pcdata->in_progress->text)
    {
      switch (ch->pcdata->board->force_type)
	{
	case DEF_NORMAL:
	  chprintlnf (ch,
		      "If you press Return, default recipient \"{W%s{x\" will be chosen.",
		      ch->pcdata->board->names);
	  break;
	case DEF_INCLUDE:
	  chprintlnf (ch,
		      "The recipient list MUST include \"{W%s{x\". If not, it will be added automatically.",
		      ch->pcdata->board->names);
	  break;

	case DEF_EXCLUDE:
	  chprintlnf (ch,
		      "The recipient of this note must NOT include: \"{W%s{x\".",
		      ch->pcdata->board->names);

	  break;
	default:
	  break;
	}

      chprintln (ch, "{YTo{x:      ");

      ch->desc->connected = CON_NOTE_TO;


    }
  else
    {

      chprintlnf (ch,
		  "{YTo{x:      %s" NEWLINE
		  "{YExpires{x: %s" NEWLINE
		  "{YSubject{x: %s", ch->pcdata->in_progress->to_list,
		  str_time (ch->pcdata->in_progress->expire, GetTzone (ch),
			    NULL), ch->pcdata->in_progress->subject);
      chprintln (ch, "{GYour note so far:{x");
      chprint (ch, ch->pcdata->in_progress->text);

      chprintlnf
	(ch,
	 "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
	 StrEdKey (ch), StrEdKey (ch));
      chprintln (ch, draw_line (ch, "{W={w=", 0));

      ch->desc->connected = CON_NOTE_TEXT;

    }

}


static
Do_Fun (do_nforward)
{
  NoteData *p;
  char buf[MSL];
  char arg[MIL];

  if (IsNPC (ch))

    return;

  if (get_trust (ch) < 2)
    {
      chprintln (ch, "You can't seem to write a note.");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg) || !is_number (arg) || atoi (arg) < 1)
    {
      chprintln (ch, "Forward which note?");
      return;
    }

  p = find_note (ch, ch->pcdata->board, atoi (arg));

  if (!p)
    {
      chprintln (ch, "No such note.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Forward the note to who?");
      return;
    }

  if (get_trust (ch) < ch->pcdata->board->write_level)
    {
      chprintln (ch, "You cannot forward notes on this board.");
      return;
    }

  if (ch->pcdata->in_progress)
    {
      if (!ch->pcdata->in_progress->text)
	{
	  chprintln
	    (ch,
	     "Note in progress cancelled because you did not manage to write any text "
	     NEWLINE "before losing link.");
	  free_note (ch->pcdata->in_progress);
	  ch->pcdata->in_progress = NULL;
	}
      else
	{
	  chprintlnf
	    (ch,
	     "You already have a note in progress. type 'note write' to continue it.");
	  return;
	}
    }

  ch->pcdata->in_progress = new_note ();
  replace_str (&ch->pcdata->in_progress->sender, ch->name);

  replace_str (&ch->pcdata->in_progress->date, str_time (-1, -1, NULL));

  act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_ROOM);

  chprintlnf (ch, "{YFrom{x:    %s", GetName (ch));

  if (ch->pcdata->board->force_type == DEF_INCLUDE)
    sprintf (buf, "%s %s",
	     format_recipient (ch, ch->pcdata->board->names), argument);
  else
    sprintf (buf, "%s", argument);

  replace_str (&ch->pcdata->in_progress->to_list, buf);

  replace_strf (&ch->pcdata->in_progress->subject, "FWD: %s", p->subject);

  ch->pcdata->in_progress->expire =
    current_time + ch->pcdata->board->purge_days * DAY;

  chprintlnf (ch, "{YTo{x:      %s" NEWLINE
	      "{YExpires{x: %s" NEWLINE
	      "{YSubject{x: %s", ch->pcdata->in_progress->to_list,
	      str_time (ch->pcdata->in_progress->expire, GetTzone (ch),
			NULL), ch->pcdata->in_progress->subject);

  replace_str (&ch->pcdata->in_progress->reply_text, replines (p->text));

  sendpage (ch, ch->pcdata->in_progress->reply_text);

  chprintlnf
    (ch,
     "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
     StrEdKey (ch), StrEdKey (ch));
  chprintln (ch, draw_line (ch, "{W={w=", 0));
  ch->desc->connected = CON_NOTE_TEXT;
}

static
Do_Fun (do_nreply)
{
  NoteData *p;
  char buf[MSL];
  char arg[MIL];

  if (IsNPC (ch))
    return;

  if (get_trust (ch) < 2)
    {
      chprintln (ch, "You can't seem to write a note.");
      return;
    }

  argument = one_argument (argument, arg);

  if (NullStr (arg) || !is_number (arg) || atoi (arg) < 1)
    {
      chprintln (ch, "Reply to which note?");
      return;
    }

  p = find_note (ch, ch->pcdata->board, atoi (arg));

  if (!p)
    {
      chprintln (ch, "No such note.");
      return;
    }

  if (get_trust (ch) < ch->pcdata->board->write_level)
    {
      chprintln (ch, "You cannot reply to notes on this board.");
      return;
    }

  if (ch->pcdata->in_progress)
    {
      if (!ch->pcdata->in_progress->text)
	{
	  chprintln
	    (ch,
	     "Note in progress cancelled because you did not manage to write any text "
	     NEWLINE "before losing link.");
	  free_note (ch->pcdata->in_progress);
	  ch->pcdata->in_progress = NULL;
	}
      else
	{
	  chprintln
	    (ch,
	     "You already have a note in progress. type 'note write' to continue it.");
	  return;
	}
    }

  ch->pcdata->in_progress = new_note ();
  replace_str (&ch->pcdata->in_progress->sender, ch->name);

  replace_str (&ch->pcdata->in_progress->date,
	       str_time (-1, GetTzone (ch), NULL));

  act ("{G$n starts writing a note.{x", ch, NULL, NULL, TO_ROOM);

  chprintlnf (ch, "{YFrom{x:    %s", GetName (ch));

  if (ch->pcdata->board->force_type == DEF_INCLUDE
      || !str_cmp (argument, "all"))
    sprintf (buf, "%s %s", p->sender,
	     format_recipient (ch, ch->pcdata->board->names));
  else
    sprintf (buf, "%s", p->sender);

  replace_str (&ch->pcdata->in_progress->to_list, buf);

  replace_strf (&ch->pcdata->in_progress->subject, "RE: %s", p->subject);

  ch->pcdata->in_progress->expire =
    current_time + ch->pcdata->board->purge_days * DAY;

  chprintlnf (ch, "{YTo{x:      %s" NEWLINE
	      "{YExpires{x: %s" NEWLINE
	      "{YSubject{x: %s", ch->pcdata->in_progress->to_list,
	      str_time (ch->pcdata->in_progress->expire, GetTzone (ch),
			NULL), ch->pcdata->in_progress->subject);

  replace_str (&ch->pcdata->in_progress->reply_text, replines (p->text));

  sendpage (ch, ch->pcdata->in_progress->reply_text);

  chprintlnf
    (ch,
     "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
     StrEdKey (ch), StrEdKey (ch));
  chprintln (ch, draw_line (ch, "{W={w=", 0));
  ch->desc->connected = CON_NOTE_TEXT;
}


static void
next_board (CharData * ch)
{
  int i = board_number (ch->pcdata->board) + 1;

  while ((i < MAX_BOARD)
	 && (unread_notes (ch, &boards[i]) == BOARD_NOACCESS
	     || ch->pcdata->unsubscribed[i]))
    i++;

  if (i == MAX_BOARD)
    {
      chprint (ch, "End of Boards. ");
      i = 0;
    }

  ch->pcdata->board = &boards[i];
  return;
}



static
Do_Fun (do_nread)
{
  NoteData *p;
  int count = 0, number;
  time_t *last_note =
    &ch->pcdata->last_note[board_number (ch->pcdata->board)];

  if (!str_cmp (argument, "again"))
    {
      count = 1;
      for (p = ch->pcdata->board->note_first; p; p = p->next, count++)
	if (p == ch->pcdata->last_read)
	  break;

      if (!p || !is_note_to (ch, p))
	{
	  chprintln (ch, "No such note.");
	}
      else
	show_note_to_char (ch, p, count);
    }
  else if (is_number (argument))
    {
      number = atoi (argument);

      for (p = ch->pcdata->board->note_first; p; p = p->next)
	if (++count == number)
	  break;

      if (!p || !is_note_to (ch, p))
	chprintln (ch, "No such note.");
      else
	{
	  show_note_to_char (ch, p, count);
	  *last_note = Max (*last_note, p->date_stamp);
	  ch->pcdata->last_read = p;
	}
    }
  else
    {

      count = 1;
      for (p = ch->pcdata->board->note_first; p; p = p->next, count++)
	if ((p->date_stamp > *last_note) && is_note_to (ch, p))
	  {
	    show_note_to_char (ch, p, count);

	    *last_note = Max (*last_note, p->date_stamp);
	    ch->pcdata->last_read = p;
	    return;
	  }

      chprintln (ch, "No new notes in this board.");

      next_board (ch);

      chprintlnf (ch, "Changed to next subscribed board, %s.",
		  ch->pcdata->board->short_name);
    }
}


static
Do_Fun (do_nremove)
{
  NoteData *p;

  if (!str_cmp (argument, "all") && IsImmortal (ch))
    {
      NoteData *p_next;

      for (p = ch->pcdata->board->note_first; p; p = p_next)
	{
	  p_next = p->next;

	  if (str_cmp (ch->name, p->sender) && (get_trust (ch) < MAX_LEVEL))
	    continue;

	  unlink_note (ch->pcdata->board, p);
	  free_note (p);
	}
      chprintln (ch, "ALL Notes removed!");
    }
  else
    {

      if (!is_number (argument))
	{
	  chprintln (ch, "Remove which note?");
	  return;
	}

      p = find_note (ch, ch->pcdata->board, atoi (argument));
      if (!p)
	{
	  chprintln (ch, "No such note.");
	  return;
	}

      if (str_cmp (ch->name, p->sender) && (get_trust (ch) < MAX_LEVEL))
	{
	  chprintln (ch, "You are not authorized to remove this note.");
	  return;
	}

      unlink_note (ch->pcdata->board, p);
      free_note (p);
      chprintln (ch, "Note removed!");
    }

  SetBit (ch->pcdata->board->flags, BOARD_CHANGED);
}



static
Do_Fun (do_nlist)
{
  int count = 0, show = 0, num = 0, has_shown = 0;
  time_t last_note;
  NoteData *p;
  Column Cd;
  Buffer *buf;

  if (is_number (argument))
    {

      show = atoi (argument);

      for (p = ch->pcdata->board->note_first; p; p = p->next)
	if (is_note_to (ch, p))
	  count++;
    }

  buf = new_buf ();
  set_cols (&Cd, ch, 2, COLS_BUF, buf);
  bprintln (buf, "{WNotes on this board:{x");
  cols_header (&Cd, "{rNum> Author       Subject");

  last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];

  for (p = ch->pcdata->board->note_first; p; p = p->next)
    {
      num++;
      if (is_note_to (ch, p))
	{
	  has_shown++;

	  if (!show || ((count - show) < has_shown))
	    {
	      print_cols (&Cd, "{W%3d{x>{B%c{Y%-12.12s{Y %s{x ", num,
			  last_note < p->date_stamp ? '*' : ' ',
			  p->sender, p->subject);
	    }
	}

    }
  cols_nl (&Cd);
  sendpage (ch, buf_string (buf));
  free_buf (buf);
}


static
Do_Fun (do_ncatchup)
{
  NoteData *p;

  if (argument[0] == '\0')
    {
      for (p = ch->pcdata->board->note_first; p && p->next; p = p->next)
	;

      if (!p)
	chprintln (ch, "Alas, there are no notes in that board.");
      else
	{
	  ch->pcdata->last_note[board_number (ch->pcdata->board)] =
	    p->date_stamp;
	  chprintln (ch, "All messages skipped.");
	}
    }
  else
    {
      if (is_name ("all", argument))
	{
	  int i, c = 0;
	  BoardData *board;

	  for (i = 0; i < MAX_BOARD; i++)
	    {
	      board = &boards[i];

	      if (unread_notes (ch, board) == BOARD_NOACCESS)
		continue;
	      if (unread_notes (ch, board) == 0
		  || ch->pcdata->unsubscribed[i])
		continue;

	      c++;

	      for (p = board->note_first; p && p->next; p = p->next)
		;

	      if (p)
		{
		  ch->pcdata->last_note[board_number (board)] = p->date_stamp;
		  chprintlnf (ch, "All notes in {W%s{x board skipped.",
			      board->short_name);
		}
	    }
	  if (c > 0)
	    chprintlnf (ch,
			"All notes in {W%s{x were skipped.", intstr (c,
								     "minute"));
	  else
	    chprintln (ch, "There is no new notes to skip.");
	}
      else
	{
	  chprintlnf
	    (ch, "Only argument supported after '%s' is 'all'.", n_fun);
	}
    }
}

static
Do_Fun (do_nclear)
{
  if (ch->pcdata->in_progress)
    {
      free_note (ch->pcdata->in_progress);
      chprintln (ch, "Note cleared.");
    }
  else
    chprintln (ch, "You dont have any notes in progress.");
}


static
Do_Fun (do_npurge)
{
  int i;

  if (!IsImmortal (ch))
    return;

  for (i = 0; i < MAX_BOARD; i++)
    {
      check_notes (&boards[i]);
    }
  chprintln (ch, "Old notes cleaned.");

  return;
}

static
Do_Fun (do_nreset)
{
  int pos;

  if (IsNPC (ch))
    return;

  for (pos = 0; pos < MAX_BOARD; pos++)
    ch->pcdata->last_note[pos] = 0;

  chprintln (ch, "All notes marked as unread.");

  return;
}

static
Do_Fun (do_nhelp)
{
  cmd_syntax
    (ch, n_fun,
     "read [again]                - read all notes 1 board at a time.",
     "write                       - write a note on your current board.",
     "list                        - list all notes on your current board.",
     "remove [number]             - remove a note.",
     "print                       - print a note to a usable object.",
     "reply [number]              - reply to a note.",
     "forward [number] [arg]      - forward a note to a specific person.",
     "catchup                     - mark all notes on current board as read.",
     "reset                       - mark all notes on all boards as unread.",
     "check                       - count how many unread notes you have on all boards.",
     "clear                       - clear current note in progress.", NULL);
  if (IsImmortal (ch))
    cmd_syntax (ch, n_fun,
		"purge                       - purges expired notes from current board.",
		NULL);
}

Do_Fun (do_note)
{
  if (IsNPC (ch))
    return;

  vinterpret (ch, n_fun, argument, "read", do_nread, "list", do_nlist,
	      "write", do_nwrite, "to", do_nwrite, "remove", do_nremove,
	      "reply", do_nreply, "forward", do_nforward,
	      "purge", do_npurge, "reset", do_nreset, "clear", do_nclear,
	      "check", do_ncheck, "catchup", do_ncatchup, "help",
	      do_nhelp, NULL, do_nread);
}

Do_Fun (do_subscribe)
{
  int i, count, number;

  if (IsNPC (ch))
    return;

  if (NullStr (argument))
    {
      count = 1;
      chprintln (ch, "{RNum         Name Subscribed Description{x" NEWLINE
		 "{R=== ============ ========== ==========={x");
      for (i = 0; i < MAX_BOARD; i++)
	{
	  if (unread_notes (ch, &boards[i]) == BOARD_NOACCESS)
	    continue;

	  chprintlnf (ch, "{W%2d{x> {g%12s{x [  %-8s{x] %s{x", count,
		      boards[i].short_name,
		      ch->pcdata->unsubscribed[i] ? "{rNO" : "{gYES",
		      boards[i].long_name);
	  count++;
	}
      return;
    }

  if (ch->pcdata->in_progress)
    {
      chprintln (ch, "Please finish your interrupted note first.");
      return;
    }

  if (is_number (argument))
    {
      count = 0;
      number = atoi (argument);
      for (i = 0; i < MAX_BOARD; i++)
	if (unread_notes (ch, &boards[i]) != BOARD_NOACCESS)
	  if (++count == number)
	    break;

      if (i == board_lookup ("Announce") || i == board_lookup ("Personal"))
	{
	  chprintln
	    (ch,
	     "You cannot un-subscribe from the Announce or Personal boards.");
	  return;
	}

      if (count == number)
	{
	  if (ch->pcdata->unsubscribed[i])
	    {
	      ch->pcdata->unsubscribed[i] = false;
	      chprintlnf (ch,
			  "You are now subscribed to the {W%s{x board.",
			  boards[i].short_name);
	    }
	  else
	    {
	      ch->pcdata->unsubscribed[i] = true;
	      chprintlnf
		(ch,
		 "You are no longer subscribed to the {W%s{x board.",
		 boards[i].short_name);
	    }
	}
      else
	chprintln (ch, "No such board.");

      return;
    }

  for (i = 0; i < MAX_BOARD; i++)
    if (!str_prefix (argument, boards[i].short_name))
      break;

  if (i == MAX_BOARD)
    {
      chprintln (ch, "No such board.");
      return;
    }

  if (unread_notes (ch, &boards[i]) == BOARD_NOACCESS)
    {
      chprintln (ch, "No such board.");
      return;
    }

  if (i == board_lookup ("Announce") || i == board_lookup ("Personal"))
    {
      chprintln
	(ch, "You cannot un-subscribe from the Announce or Personal boards.");
      return;
    }

  if (ch->pcdata->unsubscribed[i])
    {
      ch->pcdata->unsubscribed[i] = false;
      chprintlnf (ch, "You are now subscribed to the {W%s{x board.",
		  boards[i].short_name);
    }
  else
    {
      ch->pcdata->unsubscribed[i] = true;
      chprintlnf (ch, "You are no longer subscribed to the {W%s{x board.",
		  boards[i].short_name);
    }
}



void
show_board (CharData * ch, bool fAll)
{
  int unread, count, i, last;
  NoteData *p;
  BoardData *b;
  bool found = true;

  count = 0;
  if (IsImmortal (ch))
    chprintln
      (ch,
       "{RNum         Name Flags       Unread Last Description{x"
       NEWLINE "{R=== ============ =========== ====== ==== ==========={x");
  else
    chprintln (ch,
	       "{RNum         Name Unread Last Description{x" NEWLINE
	       "{R=== ============ ====== ==== ==========={x");
  for (i = 0; i < MAX_BOARD; i++)
    {
      unread = unread_notes (ch, &boards[i]);

      if (unread == BOARD_NOACCESS)
	continue;

      count++;

      if (ch->pcdata->unsubscribed[i])
	continue;

      if (unread == 0 && fAll == true)
	continue;

      last = 0;
      b = &boards[i];
      for (p = b->note_first; p; p = p->next)
	if (is_note_to (ch, p))
	  last++;

      found = false;
      chprintlnf (ch, "{W%2d{x> {G%12s{x [{%c%4d{x] {G%4d {Y%s{x", count,
		  boards[i].short_name, unread == 0 ? 'r' : 'R', unread,
		  last, boards[i].long_name);
    }
  if (!found)
    chprintln
      (ch,
       "You have no unread notes on any subscribed board." NEWLINE
       "(Use 'board all' to see a list of boards.)");

  chprintf (ch, NEWLINE "Your current board is {W%s{x",
	    ch->pcdata->board->short_name);
  if ((p = last_note (ch, ch->pcdata->board)) != NULL)
    chprintlnf (ch, ". Last message was from {W%s{x.", p->sender);
  else
    chprintln (ch, ".");

  if (ch->pcdata->board->read_level > get_trust (ch))
    chprintln (ch, "You cannot read nor write notes on this board.");
  else if (ch->pcdata->board->write_level > get_trust (ch))
    chprintln (ch, "You can only read notes from this board.");
  else
    chprintln (ch, "You can both read and write on this board.");

  chprintln (ch, "Use 'board all' to see all subscribed boards.");
  chprintln (ch, "Use 'subscribe' to see what boards you are subscribed to.");
  return;
}

Do_Fun (do_board)
{
  int i, number, count;
  NoteData *p;

  if (IsNPC (ch))
    return;

  if (NullStr (argument))
    {
      show_board (ch, true);
      return;
    }
  else if (!str_cmp (argument, "all"))
    {
      show_board (ch, false);
      return;
    }
  else if (IsImmortal (ch) && !str_cmp (argument, "save"))
    {
      rw_note_data (act_write);
      chprintln (ch, "Notes saved.");
      return;
    }

  if (ch->pcdata->in_progress)
    {
      chprintln (ch, "Please finish your interrupted note first.");
      return;
    }

  if (is_number (argument))
    {
      count = 0;
      number = atoi (argument);
      for (i = 0; i < MAX_BOARD; i++)
	if (unread_notes (ch, &boards[i]) != BOARD_NOACCESS)
	  if (++count == number)
	    break;

      if (count == number)
	{
	  ch->pcdata->board = &boards[i];
	  chprintlnf (ch, "Current board changed to {W%s{x. %s.",
		      boards[i].short_name,
		      (get_trust (ch) <
		       boards[i].write_level) ? "You can only read here" :
		      "You can both read and write here");
	  if ((p = last_note (ch, &boards[i])) != NULL)
	    chprintlnf
	      (ch, "Last message was from {W%s{x concerning {W%s{x.",
	       p->sender, p->subject);
	}
      else

	chprintln (ch, "No such board.");

      return;
    }

  for (i = 0; i < MAX_BOARD; i++)
    if (!str_prefix (argument, boards[i].short_name))
      break;

  if (i == MAX_BOARD)
    {
      chprintln (ch, "No such board.");
      return;
    }

  if (unread_notes (ch, &boards[i]) == BOARD_NOACCESS)
    {
      chprintln (ch, "No such board.");
      return;
    }

  ch->pcdata->board = &boards[i];
  chprintlnf (ch, "Current board changed to {W%s{x. %s.",
	      boards[i].short_name,
	      (get_trust (ch) <
	       boards[i].write_level) ? "You can only read here" :
	      "You can both read and write here");
  if ((p = last_note (ch, &boards[i])) != NULL)
    chprintlnf (ch, "Last message was from {W%s{x concerning {W%s{x.",
		p->sender, p->subject);
}


void
personal_message (const char *sender, const char *to,
		  const char *subject, const int expire_days,
		  const char *text)
{
  make_note ("Personal", sender, to, subject, expire_days, text);
}

void
make_note (const char *board_name, const char *sender, const char *to,
	   const char *subject, const int expire_days, const char *text)
{
  int board_index = board_lookup (board_name);
  BoardData *board;
  NoteData *note;

  if (board_index == BOARD_NOTFOUND)
    {
      bug ("make_note: board not found");
      return;
    }

  if (strlen (text) > MAX_NOTE_TEXT)
    {
      bugf ("make_note: text too long (%d bytes)", strlen (text));
      return;
    }

  board = &boards[board_index];

  note = new_note ();

  note->sender = str_dup (sender);
  note->to_list = str_dup (to);
  note->subject = str_dup (subject);
  note->expire = current_time + expire_days * 60 * 60 * 24;
  note->text = str_dup (text);

  note->date = str_dup (str_time (current_time, -1, NULL));

  finish_note (board, note);

}

void
append_to_note (CharData * ch, const char *board_name,
		const char *sender, const char *to,
		const char *subject, const int expire_days, const char *text)
{
  int board_index = board_lookup (board_name);
  BoardData *board;

  if (board_index == BOARD_NOTFOUND)
    {
      bug ("board not found");
      return;
    }

  if (strlen (text) > (size_t) MAX_NOTE_TEXT)
    {
      bugf ("text too long (%d bytes)", strlen (text));
      return;
    }

  board = &boards[board_index];
  ch->pcdata->in_progress = new_note ();
  replace_str (&ch->pcdata->in_progress->sender, sender);
  replace_str (&ch->pcdata->in_progress->to_list, to);
  replace_str (&ch->pcdata->in_progress->subject, subject);
  ch->pcdata->in_progress->expire =
    current_time + (time_t) (expire_days * DAY);

  replace_str (&ch->pcdata->in_progress->date, str_time (-1, -1, NULL));
  replace_str (&ch->pcdata->in_progress->text, text);

  chprintlnf (ch, "{YTo{x: %s" NEWLINE
	      "{YExpires{x: %s" NEWLINE
	      "{YSubject{x: %s" NEWLINE
	      "%s", ch->pcdata->in_progress->to_list,
	      (ch->pcdata->board->purge_days ==
	       -1) ? "Never" : str_time (ch->pcdata->in_progress->expire,
					 GetTzone (ch), NULL),
	      ch->pcdata->in_progress->subject,
	      ch->pcdata->in_progress->text);

  chprintlnf
    (ch,
     "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
     StrEdKey (ch), StrEdKey (ch));
  chprintln (ch, draw_line (ch, "{W={w=", 0));

  ch->desc->connected = CON_NOTE_TEXT;

}

Nanny_Fun (HANDLE_CON_NOTE_TO)
{
  CharData *ch = d->character;
  const char *names;

  if (!ch->pcdata->in_progress)
    {
      d->connected = CON_PLAYING;
      bugf ("nanny: In CON_NOTE_TO, but no note in progress");
      return;
    }

  names = format_recipient (ch, ch->pcdata->board->names);

  switch (ch->pcdata->board->force_type)
    {
    case DEF_NORMAL:
      if (NullStr (argument))
	{
	  replace_str (&ch->pcdata->in_progress->to_list, names);
	  d_printlnf (d, "Assumed default recipient: {W%s{x", names);
	}
      else
	replace_str (&ch->pcdata->in_progress->to_list, argument);

      break;
    default:
      break;

    case DEF_INCLUDE:
      if (!is_full_name (names, argument))
	{
	  replace_strf (&ch->pcdata->in_progress->to_list, "%s %s",
			argument, names);

	  d_printlnf (d,
		      NEWLINE
		      "You did not specify %s as recipient, so it was automatically added."
		      NEWLINE "{YNew To{x :  %s", names,
		      ch->pcdata->in_progress->to_list);
	}
      else
	replace_str (&ch->pcdata->in_progress->to_list, argument);
      break;

    case DEF_EXCLUDE:
      if (NullStr (argument))
	{
	  d_print (d,
		   "You must specify a recipient." NEWLINE "{YTo{x:      ");
	  return;
	}

      if (is_full_name (names, argument))
	{
	  d_printf (d,
		    "You are not allowed to send notes to %s on this board. Try again."
		    NEWLINE "{YTo{x:      ", names);
	  return;
	}
      else
	replace_str (&ch->pcdata->in_progress->to_list, argument);
      break;

    }

  d_print (d, NEWLINE "{YSubject{x: ");
  d->connected = CON_NOTE_SUBJECT;
}

Nanny_Fun (HANDLE_CON_NOTE_SUBJECT)
{
  CharData *ch = d->character;

  if (!ch->pcdata->in_progress)
    {
      d->connected = CON_PLAYING;
      bugf ("nanny: In CON_NOTE_SUBJECT, but no note in progress");
      return;
    }



  if (NullStr (argument))
    {
      d_println (d, "Please find a meaningful subject!");
      d_print (d, "{YSubject{x: ");
    }
  else if (strlen (argument) > 60)
    {
      d_println (d,
		 "No, no. This is just the Subject. You're note writing the note yet. Twit.");
    }
  else

    {
      replace_str (&ch->pcdata->in_progress->subject, argument);
      if (IsImmortal (ch))
	{
	  d_printf (d,
		    NEWLINE
		    "How many days do you want this note to expire in?"
		    NEWLINE
		    "Press Enter for default value for this board, {W%d{x days."
		    NEWLINE "{YExpire{x:  ", ch->pcdata->board->purge_days);
	  d->connected = CON_NOTE_EXPIRE;
	}
      else
	{
	  ch->pcdata->in_progress->expire =
	    current_time + ch->pcdata->board->purge_days * 24L * 3600L;
	  d_printlnf (d, "This note will expire %s",
		      str_time (ch->pcdata->in_progress->expire,
				GetTzone (ch), NULL));
	  d_printlnf (d,
		      NEWLINE
		      "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
		      StrEdKey (ch), StrEdKey (ch));
	  d_println (d, draw_line (ch, "{W={w=", 0));
	  d->connected = CON_NOTE_TEXT;
	}
    }
}

Nanny_Fun (HANDLE_CON_NOTE_EXPIRE)
{
  CharData *ch = d->character;
  time_t expire;
  int days;

  if (!ch->pcdata->in_progress)
    {
      d->connected = CON_PLAYING;
      bugf ("nanny: In CON_NOTE_EXPIRE, but no note in progress");
      return;
    }

  if (NullStr (argument))
    days = ch->pcdata->board->purge_days;
  else if (!is_number (argument))
    {
      d_println (d, "Write the number of days!");
      d_print (d, "{YExpire{x:  ");
      return;
    }
  else
    {
      days = atoi (argument);
      if (days <= 0)
	{
	  d_println (d,
		     "This is a positive Mud. Use positive numbers only! :)");
	  d_print (d, "{YExpire{x:  ");
	  return;
	}
    }

  expire = current_time + (days * DAY);

  ch->pcdata->in_progress->expire = expire;

  d_printlnf (d,
	      NEWLINE
	      "Enter text. Type {W%cq{x or {W@{x on an empty line to end note, or {W%ch{x for help.",
	      StrEdKey (ch), StrEdKey (ch));
  d_println (d, draw_line (ch, "{W={w=", 0));
  d->connected = CON_NOTE_TEXT;
}

Nanny_Fun (HANDLE_CON_NOTE_TEXT)
{
  strshow_t action;
  CharData *ch = d->character;
  char buf[MAX_STRING_LENGTH * 5];

  if (!ch->pcdata->in_progress)
    {
      d->connected = CON_PLAYING;
      bugf ("nanny: In CON_NOTE_TEXT, but no note in progress");
      return;
    }

  action =
    parse_string_command (&ch->pcdata->in_progress->text, argument, ch);

  switch (action)
    {
    case STRING_END:
      d_printf (d, NEWLINE "%s", szFinishPrompt);
      d->connected = CON_NOTE_FINISH;
      return;
    case STRING_FOUND:
      return;
    default:
    case STRING_NONE:
      if (ch->pcdata->in_progress->text)
	{
	  strcpy (buf, ch->pcdata->in_progress->text);
	  replace_str (&ch->pcdata->in_progress->text, "");
	}
      else
	strcpy (buf, "");

      if ((strlen (argument) + strlen (buf)) > MAX_NOTE_TEXT)
	{
	  d_println (d, "Note too long, bailing out!");
	  free_note (ch->pcdata->in_progress);
	  ch->pcdata->in_progress = NULL;
	  d->connected = CON_PLAYING;
	  return;
	}

      strcat (buf, argument);
      strcat (buf, NEWLINE);
      replace_str (&ch->pcdata->in_progress->text, buf);
      return;
    }
}

Nanny_Fun (HANDLE_CON_NOTE_FINISH)
{

  CharData *ch = d->character;

  if (!ch->pcdata->in_progress)
    {
      d->connected = CON_PLAYING;
      bugf ("nanny: In CON_NOTE_FINISH, but no note in progress");
      return;
    }

  switch (tolower (argument[0]))
    {
    case 'c':
      d_println (d, "Continuing note...");
      d->connected = CON_NOTE_TEXT;
      break;

    case 'v':
      if (ch->pcdata->in_progress->text)
	{
	  d_println (d, "{gText of your note so far:{x");
	  d_print (d, ch->pcdata->in_progress->text);
	}
      else
	d_println (d, "You haven't written a thing!" NEWLINE);
      d_println (d, szFinishPrompt);
      break;

    case 'p':
      announce (ch, INFO_NOTE, "New note on %s board from $n. Subj: %s",
		ch->pcdata->board->short_name,
		ch->pcdata->in_progress->subject);
      finish_note (ch->pcdata->board, ch->pcdata->in_progress);
      d_println (d, "Note posted.");
      mud_info.stats.notes++;
      d->connected = CON_PLAYING;
      ch->pcdata->in_progress = NULL;
      act ("{G$n finishes $s note.{x", ch, NULL, NULL, TO_ROOM);
      break;

    case 'f':
      d_println (d, "Note cancelled!");
      free_note (ch->pcdata->in_progress);
      ch->pcdata->in_progress = NULL;
      d->connected = CON_PLAYING;

      break;

    default:
      d_println (d, "Huh? Valid answers are:" NEWLINE);
      d_println (d, szFinishPrompt);
    }
}
