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


#ifndef __OLC_LIST_H_
#define __OLC_LIST_H_  1

Olc_Fun (oedit_list)
{
  return redit_olist (n_fun, ch, argument);
}

Olc_Fun (medit_list)
{
  return redit_mlist (n_fun, ch, argument);
}


Olc_Fun (cedit_list)
{
  do_clist (n_fun, ch, argument);
  return false;
}

Olc_Fun (aedit_list)
{
  do_alist (n_fun, ch, argument);
  return false;
}


Olc_Fun (redit_list)
{
  RoomIndex *pRoomIndex;
  AreaData *pArea;
  Buffer *buf1;
  char arg[MAX_INPUT_LENGTH];
  bool found;
  vnum_t vnum;
  Column *Cd;

  one_argument (argument, arg);

  pArea = ch->in_room->area;
  buf1 = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, buf1);
  found = false;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
      if ((pRoomIndex = get_room_index (vnum)))
	{
	  found = true;
	  print_cols (Cd, "[%5ld] %s", vnum, capitalize (pRoomIndex->name));
	}
    }

  if (!found)
    bprintln (buf1, "Room(s) not found in this area.");
  else
    cols_nl (Cd);

  sendpage (ch, buf_string (buf1));
  free_buf (buf1);
  free_column (Cd);
  return false;
}


Olc_Fun (redit_mlist)
{
  CharIndex *pMobIndex;
  AreaData *pArea;
  Buffer *buf1;
  char arg[MAX_INPUT_LENGTH];
  bool fAll, found;
  vnum_t vnum;
  Column *Cd;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<all/name>", NULL);
      return false;
    }

  buf1 = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, buf1);
  pArea = ch->in_room->area;
  fAll = !str_cmp (arg, "all");
  found = false;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
      if ((pMobIndex = get_char_index (vnum)) != NULL)
	{
	  if (fAll || is_name (arg, pMobIndex->player_name))
	    {
	      found = true;
	      print_cols (Cd, "[%5ld] %s", pMobIndex->vnum,
			  capitalize (pMobIndex->short_descr));
	    }
	}
    }

  if (!found)
    {
      chprintln (ch, "Mobile(s) not found in this area.");
      free_buf (buf1);
      free_column (Cd);
      return false;
    }
  else
    cols_nl (Cd);

  sendpage (ch, buf_string (buf1));
  free_buf (buf1);
  free_column (Cd);
  return false;
}

Olc_Fun (redit_olist)
{
  ObjIndex *pObjIndex;
  AreaData *pArea;
  Buffer *buf1;
  char arg[MAX_INPUT_LENGTH];
  bool fAll, found;
  vnum_t vnum;
  Column *Cd;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<all/name/item_type>", NULL);
      return false;
    }

  pArea = ch->in_room->area;
  buf1 = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, buf1);
  fAll = !str_cmp (arg, "all");
  found = false;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
      if ((pObjIndex = get_obj_index (vnum)))
	{
	  if (fAll || is_name (arg, pObjIndex->name) ||
	      (int) flag_value (type_flags, arg) == pObjIndex->item_type)
	    {
	      found = true;
	      print_cols (Cd, "[%5ld] %s", pObjIndex->vnum,
			  capitalize (pObjIndex->short_descr));
	    }
	}
    }

  if (!found)
    {
      bprintln (buf1, "Object(s) not found in this area.");
    }
  else
    cols_nl (Cd);

  sendpage (ch, buf_string (buf1));
  free_buf (buf1);
  free_column (Cd);
  return false;
}

Olc_Fun (cledit_list)
{
  int i;

  for (i = 0; i < top_class; i++)
    {
      chprintlnf (ch, "%d) %-12s", i + 1, class_table[i].name[0]);
    }
  return false;
}

Olc_Fun (cmdedit_list)
{
  CmdData *c;
  Buffer *b;
  Column *Cd;

  b = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 6, COLS_BUF, b);

  for (c = cmd_first; c; c = c->next)
    {
      print_cols (Cd, "[%2d] %s", c->level, c->name);
    }
  cols_nl (Cd);
  sendpage (ch, buf_string (b));
  free_buf (b);
  free_column (Cd);
  return false;
}

Olc_Fun (dedit_list)
{
  DeityData *i;
  bool found = false;

  for (i = deity_first; i; i = i->next)
    {
      found = true;
      chprintlnf (ch, "\t%-12s (%s) : %s", i->name,
		  flag_string (ethos_types, i->ethos), GetStr (i->desc,
							       "No description."));
    }
  if (!found)
    chprintln (ch, "No deities.");
  return true;
}


Olc_Fun (gredit_list)
{
  int i, x;
  Column *Cd;
  Buffer *b;

  Cd = new_column ();
  b = new_buf ();
  set_cols (Cd, ch, 2, COLS_BUF, b);

  for (i = 0; i < top_group; i++)
    {
      for (x = 0; group_table[i].spells[x] != NULL; x++)
	;

      print_cols (Cd, "%s (%d spells)", group_table[i].name, x);
    }
  cols_nl (Cd);
  sendpage (ch, buf_string (b));
  free_column (Cd);
  free_buf (b);
  return false;
}

Olc_Fun (mpedit_list)
{
  int count = 1;
  ProgCode *mprg;
  Buffer *buffer;
  bool fAll = !str_cmp (argument, "all");
  char blah;
  AreaData *ad;

  buffer = new_buf ();

  for (mprg = mprog_first; mprg != NULL; mprg = mprg->next)
    if (fAll
	|| Entre (ch->in_room->area->min_vnum - 1, mprg->vnum,
		  ch->in_room->area->max_vnum + 1))
      {
	ad = mprg->area;

	if (ad == NULL)
	  blah = '?';
	else if (IsBuilder (ch, ad))
	  blah = '*';
	else
	  blah = ' ';

	bprintlnf (buffer, "[%3d] (%c) %5ld", count, blah, mprg->vnum);

	count++;
      }

  if (count == 1)
    {
      if (fAll)
	bprintln (buffer, "No existing MobPrograms.");
      else
	bprintln (buffer, "No existing MobPrograms in this area.");
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);

  return false;
}

Olc_Fun (opedit_list)
{
  int count = 1;
  ProgCode *oprg;
  Buffer *buffer;
  bool fAll = !str_cmp (argument, "all");
  char blah;
  AreaData *ad;

  buffer = new_buf ();

  for (oprg = oprog_first; oprg != NULL; oprg = oprg->next)
    if (fAll
	|| Entre (ch->in_room->area->min_vnum - 1, oprg->vnum,
		  ch->in_room->area->max_vnum + 1))
      {
	ad = oprg->area;

	if (ad == NULL)
	  blah = '?';
	else if (IsBuilder (ch, ad))
	  blah = '*';
	else
	  blah = ' ';

	bprintlnf (buffer, "[%3d] (%c) %5ld", count, blah, oprg->vnum);

	count++;
      }

  if (count == 1)
    {
      if (fAll)
	bprintln (buffer, "No existing ObjPrograms.");
      else
	bprintln (buffer, "No existing ObjPrograms in this area.");
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);

  return false;
}

Olc_Fun (rpedit_list)
{
  int count = 1;
  ProgCode *rprg;
  Buffer *buffer;
  bool fAll = !str_cmp (argument, "all");
  char blah;
  AreaData *ad;

  buffer = new_buf ();

  for (rprg = rprog_first; rprg != NULL; rprg = rprg->next)
    if (fAll
	|| Entre (ch->in_room->area->min_vnum - 1, rprg->vnum,
		  ch->in_room->area->max_vnum + 1))
      {
	ad = rprg->area;

	if (ad == NULL)
	  blah = '?';
	else if (IsBuilder (ch, ad))
	  blah = '*';
	else
	  blah = ' ';

	bprintlnf (buffer, "[%3d] (%c) %5ld", count, blah, rprg->vnum);
	count++;
      }

  if (count == 1)
    {
      if (fAll)
	bprintln (buffer, "No existing RoomPrograms.");
      else
	bprintln (buffer, "No existing RoomPrograms in this area.");
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);

  return false;
}

Olc_Fun (raedit_list)
{
  RaceData *i;
  int count = 0;

  chprintln (ch, "Num  Race Name");
  chprintln (ch, draw_line (ch, NULL, 0));

  for (i = race_first; i; i = i->next)
    {
      if (!NullStr (i->name))
	{
	  count++;
	  chprintlnf (ch, "[%2d]  %-30s", count, i->name);
	}
    }

  chprintln (ch, draw_line (ch, NULL, 0));

  return false;
}


Olc_Fun (skedit_list)
{
  int i;
  Column *Cd;
  Buffer *b;

  b = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, b);

  for (i = 0; i < top_skill; i++)
    {
      print_cols (Cd, "%s '%s'",
		  skill_table[i].spell_fun ==
		  spell_null ? "skill" : "spell", skill_table[i].name);
    }
  cols_nl (Cd);
  sendpage (ch, buf_string (b));
  free_buf (b);
  free_column (Cd);
  return false;
}

Olc_Fun (sedit_list)
{
  SocialData *s;
  Column *Cd;
  Buffer *b;

  b = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 6, COLS_BUF, b);

  for (s = social_first; s; s = s->next)
    print_cols (Cd, s->name);

  cols_nl (Cd);
  sendpage (ch, buf_string (b));
  free_buf (b);
  free_column (Cd);
  return false;
}


Olc_Fun (songedit_list)
{
  int i;
  Column *Cd;
  Buffer *b;

  b = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, b);

  for (i = 0; i < top_song; i++)
    {
      print_cols (Cd, "%s by %s (%d lines)", song_table[i].name,
		  song_table[i].group, song_table[i].lines);
    }
  cols_nl (Cd);
  sendpage (ch, buf_string (b));
  free_buf (b);
  free_column (Cd);
  return false;
}

Olc_Fun (hedit_list)
{
  HelpData *pHelp;

  GetEdit (ch, HelpData, pHelp);

  if (!str_cmp (argument, "all"))
    {
      Column *Cd;
      int cnt;
      Buffer *buffer;

      buffer = new_buf ();
      Cd = new_column ();
      set_cols (Cd, ch, 6, COLS_BUF, buffer);

      for (cnt = 0, pHelp = help_first; pHelp; pHelp = pHelp->next, cnt++)
	{
	  print_cols (Cd, "%3d. %s", cnt + 1, pHelp->keyword);
	}

      cols_nl (Cd);

      sendpage (ch, buf_string (buffer));
      free_buf (buffer);
      free_column (Cd);
      return false;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "all", NULL);
      return false;
    }

  return false;
}
#endif
