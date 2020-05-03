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


#ifndef __OLC_DELETE_H_
#define __OLC_DELETE_H_    	1

void
unlink_reset (RoomIndex * pRoom, ResetData * pReset)
{
  ResetData *prev, *wReset;

  for (wReset = pRoom->reset_first; wReset; wReset = prev)
    {
      prev = wReset->next;

      if (wReset == pReset)
	{
	  UnLink (pReset, pRoom->reset, next, prev);
	}
    }
}

void
unlink_obj_index (ObjIndex * pObj)
{
  int iHash;

  iHash = pObj->vnum % MAX_KEY_HASH;

  UnlinkSingle (pObj, ObjIndex, obj_index_hash[iHash], next);
}

void
unlink_room_index (RoomIndex * pRoom)
{
  int iHash;

  iHash = pRoom->vnum % MAX_KEY_HASH;

  UnlinkSingle (pRoom, RoomIndex, room_index_hash[iHash], next);
}

void
unlink_char_index (CharIndex * pMob)
{
  int iHash;

  iHash = pMob->vnum % MAX_KEY_HASH;

  UnlinkSingle (pMob, CharIndex, char_index_hash[iHash], next);
}


void
unlink_mprog (vnum_t pnum)
{
  ProgList *list, *list_next;
  CharIndex *pMob;
  int hash, count;

  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pMob = char_index_hash[hash]; pMob; pMob = pMob->next)
	{
	  count = -1;
	  for (list = pMob->mprog_first; list != NULL; list = list_next)
	    {
	      list_next = list->next;
	      count++;

	      if (list->prog->vnum != pnum)
		continue;

	      RemBit (pMob->mprog_flags, list->trig_type);
	      UnLink (list, pMob->mprog, next, prev);
	      free_prog_list (list);
	    }
	}
    }
}

void
unlink_oprog (vnum_t pnum)
{
  ProgList *list, *list_next;
  ObjIndex *pObj;
  int hash, count;

  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pObj = obj_index_hash[hash]; pObj; pObj = pObj->next)
	{
	  count = -1;
	  for (list = pObj->oprog_first; list != NULL; list = list_next)
	    {
	      list_next = list->next;
	      count++;

	      if (list->prog->vnum != pnum)
		continue;

	      RemBit (pObj->oprog_flags, list->trig_type);
	      UnLink (list, pObj->oprog, next, prev);
	      free_prog_list (list);
	    }
	}
    }
}

void
unlink_rprog (vnum_t pnum)
{
  ProgList *list, *list_next;
  RoomIndex *pRoom;
  int hash, count;

  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pRoom = room_index_hash[hash]; pRoom; pRoom = pRoom->next)
	{
	  count = -1;
	  for (list = pRoom->rprog_first; list != NULL; list = list_next)
	    {
	      list_next = list->next;
	      count++;

	      if (list->prog->vnum != pnum)
		continue;

	      RemBit (pRoom->rprog_flags, list->trig_type);
	      UnLink (list, pRoom->rprog, next, prev);
	      free_prog_list (list);
	    }
	}
    }
}


void
clean_area_links (AreaData * target)
{
  vnum_t vnum;
  char buf[20];

  for (vnum = target->min_vnum; vnum <= target->max_vnum; vnum++)
    {
      if (get_prog_index (vnum, PRG_MPROG) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!mpedit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Mprog %ld", vnum);
	}
      if (get_prog_index (vnum, PRG_OPROG) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!opedit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Oprog %ld", vnum);
	}
      if (get_prog_index (vnum, PRG_RPROG) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!rpedit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Rprog %ld", vnum);
	}
      if (get_room_index (vnum) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!redit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Room %ld", vnum);
	}
      if (get_obj_index (vnum) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!oedit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Object %ld", vnum);
	}
      if (get_char_index (vnum) != NULL)
	{
	  sprintf (buf, "%ld", vnum);
	  if (!medit_delete (NULL, NULL, buf))
	    bugf ("Unable to delete Mobile %ld", vnum);
	}
    }
}

Olc_Fun (redit_delete)
{
  RoomIndex *pRoom, *pRoom2;
  ResetData *pReset;
  ExitData *ex;
  ObjData *Obj, *obj_next;
  CharData *wch, *wnext;
  ExDescrData *pExtra;
  char arg[MIL];
  vnum_t pIndex, v;
  int i, iHash, rcount, ecount, mcount, ocount, edcount;

  if (NullStr (argument))
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  one_argument (argument, arg);

  if (is_number (arg))
    {
      pIndex = atov (arg);
      pRoom = get_room_index (pIndex);
    }
  else
    {
      if (ch)
	chprintln (ch, "That is not a number.");
      return false;
    }

  if (!check_vnum (pIndex, ROOM_VNUM))
    {
      if (ch)
	chprintln (ch, "That vnum is reserved.");
      return false;
    }

  if (!pRoom)
    {
      if (ch)
	chprintln (ch, "No such room.");
      return false;
    }

  stop_editing (pRoom);

  SetBit (pRoom->area->area_flags, AREA_CHANGED);

  rcount = 0;

  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
      rcount++;
    }

  ocount = 0;
  for (Obj = pRoom->content_first; Obj; Obj = obj_next)
    {
      obj_next = Obj->next_content;

      extract_obj (Obj);
      ocount++;
    }

  mcount = 0;
  for (wch = pRoom->person_first; wch; wch = wnext)
    {
      wnext = wch->next_in_room;
      if (IsNPC (wch))
	{
	  extract_char (wch, true);
	  mcount++;
	}
      else
	{
	  chprintln (wch,
		     "This room is being deleted. Moving you somewhere safe.");
	  if (wch->fighting != NULL)
	    stop_fighting (wch, true);

	  char_from_room (wch);

	  char_to_room (wch, get_room_index (ROOM_VNUM_TEMPLE));
	  wch->was_in_room = wch->in_room;
	}
    }

  ecount = 0;
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 = pRoom2->next)
	{
	  for (i = 0; i < MAX_DIR; i++)
	    {
	      if (!(ex = pRoom2->exit[i]))
		continue;

	      if (pRoom2 == pRoom)
		{

		  ecount++;
		  continue;
		}

	      if (ex->u1.to_room == pRoom)
		{
		  free_exit (pRoom2->exit[i]);
		  pRoom2->exit[i] = NULL;
		  SetBit (pRoom2->area->area_flags, AREA_CHANGED);
		  ecount++;
		}
	    }
	}
    }

  edcount = 0;
  for (pExtra = pRoom->ed_first; pExtra; pExtra = pExtra->next)
    {
      edcount++;
    }

  if (top_vnum_room == pIndex)
    for (v = 1; v < pIndex; v++)
      if (get_room_index (v))
	top_vnum_room = v;

  if (!IsSet (pRoom->room_flags, ROOM_NOEXPLORE))
    top_explored--;

  unlink_room_index (pRoom);

  pRoom->area = NULL;
  pRoom->vnum = 0;

  free_room_index (pRoom);

  if (ch)
    {
      chprintlnf (ch,
		  "Removed room vnum {C%ld{x, %d resets, %d extra "
		  "descriptions and %d exits.", pIndex, rcount,
		  edcount, ecount);
      chprintlnf (ch,
		  "{C%d{x objects and {C%d{x mobiles were extracted "
		  "from the room.", ocount, mcount);
    }
  return true;
}


Olc_Fun (aedit_delete)
{
  AreaData *pArea;
  char filename[512];

  GetEdit (ch, AreaData, pArea);

  if (ch->pcdata->security < 9)
    {
      chprintln (ch, "Insufficient security to delete area.");
      return false;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", "all (carefull!)", NULL);
      return false;
    }
  if (!str_cmp (argument, "all") && get_trust (ch) == MAX_LEVEL)
    {
      AreaData *pArea_next;

#ifdef HAVE_WORKING_FORK

      pid_t pid = fork ();

      if (pid == 0)
	{
#endif
	  crash_info.status = CRASH_LOOPING;
	  for (pArea = area_first; pArea != NULL; pArea = pArea_next)
	    {
	      pArea_next = pArea->next;
	      if (vnum_OK (pArea->min_vnum, pArea->max_vnum) == false)
		continue;
	      stop_editing (pArea);
	      clean_area_links (pArea);
	      sprintf (filename, "%s%s", AREA_DIR, pArea->file_name);
	      unlink (filename);
	      unlink_area (pArea);
	      logf ("Done unlinking area %s.", pArea->name);
	      free_area (pArea);
	    }
	  crash_info.status = CRASH_LIKELY;
#ifdef HAVE_WORKING_FORK

	  _exit (1);
	}
      else if (pid > 0)
	{
#endif
	  do_function (NULL, &do_asave, "changed");
	  chprintln (ch, "All deleteable areas deleted.");
#ifdef HAVE_WORKING_FORK

	}
      else
	{
	  chprintln (ch, "Error deleting world.");
	  log_error ("save world: fork()");
	}
#endif

    }
  else
    {
      if (!is_number (argument) || !(pArea = get_area_data (atoi (argument))))
	{
	  chprintln (ch, "That area vnum does not exist.");
	  return false;
	}

      if (vnum_OK (pArea->min_vnum, pArea->max_vnum) == false)
	{
	  chprintln (ch,
		     "That area contains hard coded vnums and cannot be deleted.");
	  return false;
	}

      clean_area_links (pArea);
      sprintf (filename, "%s%s", AREA_DIR, pArea->file_name);
      unlink (filename);
      unlink_area (pArea);
      free_area (pArea);
      do_function (NULL, &do_asave, "changed");
      chprintln (ch, "Area deleted.");
    }
  return true;
}


Olc_Fun (oedit_delete)
{
  ObjData *obj, *obj_next;
  ObjIndex *pObj;
  ResetData *pReset, *wReset;
  RoomIndex *pRoom;
  char arg[MIL];
  vnum_t pIndex, i;
  int rcount, ocount, iHash;

  if (NullStr (argument))
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  one_argument (argument, arg);

  if (is_number (arg))
    {
      pIndex = atov (arg);
      pObj = get_obj_index (pIndex);
    }
  else
    {
      if (ch)
	chprintln (ch, "That is not a number.");
      return false;
    }

  if (!check_vnum (pIndex, OBJ_VNUM))
    {
      if (ch)
	chprintln (ch, "That vnum is reserved.");
      return false;
    }

  if (!pObj)
    {
      if (ch)
	chprintln (ch, "No such object.");
      return false;
    }

  stop_editing (pObj);

  SetBit (pObj->area->area_flags, AREA_CHANGED);

  if (top_vnum_obj == pIndex)
    for (i = 1; i < pIndex; i++)
      if (get_obj_index (i))
	top_vnum_obj = i;

  ocount = 0;
  for (obj = obj_first; obj; obj = obj_next)
    {
      obj_next = obj->next;

      if (obj->pIndexData == pObj)
	{
	  extract_obj (obj);
	  ocount++;
	}
    }

  rcount = 0;
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next)
	{
	  for (pReset = pRoom->reset_first; pReset; pReset = wReset)
	    {
	      wReset = pReset->next;
	      switch (pReset->command)
		{
		case 'O':
		case 'E':
		case 'P':
		case 'G':
		  if ((pReset->arg1 == pIndex) ||
		      ((pReset->command == 'P') && (pReset->arg3 == pIndex)))
		    {
		      unlink_reset (pRoom, pReset);
		      free_reset (pReset);

		      rcount++;
		      SetBit (pRoom->area->area_flags, AREA_CHANGED);

		    }
		}
	    }
	}
    }

  unlink_obj_index (pObj);

  pObj->area = NULL;
  pObj->vnum = 0;

  free_obj_index (pObj);

  if (ch)
    {
      chprintlnf (ch,
		  "Removed object vnum {C%ld{x and {C%d{x resets.",
		  pIndex, rcount);

      chprintlnf (ch,
		  "{C%d{x occurences of the object"
		  " were extracted from the mud.", ocount);

    }
  return true;
}


Olc_Fun (medit_delete)
{
  CharData *wch, *wnext;
  CharIndex *pMob;
  ResetData *pReset, *wReset;
  RoomIndex *pRoom;
  char arg[MIL];
  vnum_t pIndex, i;
  int mcount, rcount, iHash;
  bool foundmob = false;
  bool foundobj = false;

  if (NullStr (argument))
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  one_argument (argument, arg);

  if (is_number (arg))
    {
      pIndex = atov (arg);
      pMob = get_char_index (pIndex);
    }
  else
    {
      if (ch)
	chprintln (ch, "That is not a number.");
      return false;
    }

  if (!check_vnum (pIndex, MOB_VNUM))
    {
      if (ch)
	chprintln (ch, "That vnum is reserved.");
      return false;
    }

  if (!pMob)
    {
      if (ch)
	chprintln (ch, "No such mobile.");
      return false;
    }

  stop_editing (pMob);

  SetBit (pMob->area->area_flags, AREA_CHANGED);

  if (top_vnum_mob == pIndex)
    for (i = 1; i < pIndex; i++)
      if (get_char_index (i))
	top_vnum_mob = i;

  rcount = 0;
  mcount = 0;

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next)
	{

	  for (wch = pRoom->person_first; wch; wch = wnext)
	    {
	      wnext = wch->next_in_room;
	      if (wch->pIndexData == pMob)
		{
		  extract_char (wch, true);
		  mcount++;
		}
	    }

	  for (pReset = pRoom->reset_first; pReset; pReset = wReset)
	    {
	      wReset = pReset->next;
	      switch (pReset->command)
		{
		case 'M':
		  if (pReset->arg1 == pIndex)
		    {
		      foundmob = true;

		      unlink_reset (pRoom, pReset);
		      free_reset (pReset);

		      rcount++;
		      SetBit (pRoom->area->area_flags, AREA_CHANGED);

		    }
		  else
		    foundmob = false;

		  break;
		case 'E':
		case 'G':
		  if (foundmob)
		    {
		      foundobj = true;

		      unlink_reset (pRoom, pReset);
		      free_reset (pReset);

		      rcount++;
		      SetBit (pRoom->area->area_flags, AREA_CHANGED);

		    }
		  else
		    foundobj = false;

		  break;
		case '0':
		  foundobj = false;
		  break;
		case 'P':
		  if (foundobj && foundmob)
		    {
		      unlink_reset (pRoom, pReset);
		      free_reset (pReset);

		      rcount++;
		      SetBit (pRoom->area->area_flags, AREA_CHANGED);
		    }
		}
	    }
	}
    }

  unlink_char_index (pMob);

  pMob->area = NULL;
  pMob->vnum = 0;

  free_char_index (pMob);

  if (ch)
    {
      chprintlnf (ch,
		  "Removed mobile vnum {C%ld{x and {C%d{x resets.",
		  pIndex, rcount);
      chprintlnf (ch,
		  "{C%d{x mobiles were extracted" " from the mud.", mcount);
    }
  return true;
}


Olc_Fun (cedit_delete)
{
  ClanData *pClan;

  GetEdit (ch, ClanData, pClan);

  if (str_cmp (argument, "confirm"))
    {
      chprintln (ch,
		 "Typing 'delete confirm' again will permanetely remove this clan!");
      return false;
    }
  else
    {
      CharData *pch;
      AreaData *area;
      ClanMember *pmbr, *next = NULL;

      for (pmbr = mbr_first; pmbr != NULL; pmbr = next)
	{
	  next = pmbr->next;

	  if (pmbr->clan == pClan)
	    {
	      UnLink (pmbr, mbr, next, prev);
	      free_mbr (pmbr);
	    }
	}

      for (pch = player_first; pch != NULL; pch = pch->next_player)
	{
	  if (CharClan (pch) == pClan)
	    {
	      pch->pcdata->clan = NULL;
	      pch->rank = 0;
	    }
	}

      for (area = area_first; area; area = area->next)
	{
	  if (area->clan == pClan)
	    {
	      area->clan = NULL;
	      SetBit (area->area_flags, AREA_CHANGED);
	    }
	}

      UnLink (pClan, clan, next, prev);
      free_clan (pClan);
      pClan = clan_first;
      edit_start (ch, pClan, ED_CLAN);
      chprintln (ch, "Clan deleted.");
    }

  return true;
}


Olc_Fun (songedit_delete)
{
  SongData *pSong;

  GetEdit (ch, SongData, pSong);

  if (str_cmp (argument, "confirm"))
    {
      chprintln (ch,
		 "Typing 'delete confirm' will permanetely remove this song!");
      return false;
    }
  else
    {
      int i, j = 0, c;
      struct song_data *new_table;

      alloc_mem (new_table, struct song_data, top_song);

      if (!new_table)
	{
	  chprintln (ch, "Memory Allocation error!!! Unable to delete song.");
	  return false;
	}

      c = song_lookup (pSong->name);

      for (i = 0; i < top_song; i++)
	if (i != c)
	  new_table[j++] = song_table[i];

      free_string (pSong->name);
      free_string (pSong->group);
      for (j = 0; j < pSong->lines; j++)
	free_string (pSong->lyrics[j]);
      free_mem (song_table);
      song_table = new_table;
      top_song--;

      pSong = &song_table[0];
      ch->desc->pEdit = (void *) pSong;

      chprintln (ch, "Song deleted.");
    }

  return true;
}

Olc_Fun (sedit_delete)
{
  SocialData *pSocial;

  if (NullStr (argument))
    GetEdit (ch, SocialData, pSocial);
  else
    pSocial = social_lookup (argument);

  if (pSocial == NULL)
    {
      chprintln (ch, "No such social exists.");
      return false;
    }

  unlink_social (pSocial);
  free_social (pSocial);
  edit_done (ch);
  chprintln (ch, "Social deleted.");
  return true;
}


Olc_Fun (skedit_delete)
{
  SkillData *pSkill;

  GetEdit (ch, SkillData, pSkill);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this skill!");
      return false;
    }
  else
    {
      int i, j = 0, c;
      struct skill_type *new_table;

      alloc_mem (new_table, struct skill_type, top_skill);

      if (!new_table)
	{
	  chprintln (ch,
		     "Memory Allocation error!!! Unable to delete skill.");
	  return false;
	}

      c = skill_lookup (pSkill->name);

      for (i = 0; i < top_skill; i++)
	if (i != c)
	  new_table[j++] = skill_table[i];

      free_string (pSkill->name);
      free_mem (pSkill->skill_level);
      free_mem (pSkill->rating);
      free_string (pSkill->noun_damage);
      free_string (pSkill->msg_off);
      free_string (pSkill->msg_obj);
      free_mem (skill_table);
      skill_table = new_table;
      top_skill--;

      pSkill = &skill_table[0];
      edit_start (ch, pSkill, ED_SKILL);
      chprintln (ch, "Skill deleted.");
    }

  return true;
}


Olc_Fun (raedit_delete)
{
  RaceData *pRace;
  CharData *rch;
  CharIndex *mch;
  int ihash;

  GetEdit (ch, RaceData, pRace);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this race!");
      return false;
    }
  else
    {
      for (ihash = 0; ihash < MAX_KEY_HASH; ihash++)
	{
	  for (mch = char_index_hash[ihash]; mch; mch = mch->next)
	    {
	      if (mch->race == pRace)
		{
		  mch->race = default_race;
		  SetBit (mch->area->area_flags, AREA_CHANGED);
		}
	    }
	}

      for (rch = char_first; rch != NULL; rch = rch->next)
	{
	  if (rch->race == pRace)
	    {
	      if (IsNPC (rch))
		rch->race = default_race;
	      else
		rch->race = default_race;
	    }
	}

      UnLink (pRace, race, next, prev);
      free_race (pRace);
      pRace = race_first;
      edit_start (ch, pRace, ED_RACE);
      chprintln (ch, "Race deleted.");
    }

  return true;
}


Olc_Fun (mpedit_delete)
{
  ProgCode *curr, *next;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if (get_prog_index (value, PRG_MPROG) == NULL)
    {
      if (ch)
	chprintln (ch, "Mob program not found.");
      return false;
    }

  ad = get_vnum_area (value);
  if (ad == NULL)
    {
      if (ch)
	chprintln (ch, "Mob program not assigned to an area.");
      return false;
    }

  if (ch && !IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create MobProgs.");
      return false;
    }

  unlink_mprog (value);
  for (curr = mprog_first; curr != NULL; curr = next)
    {
      next = curr->next;

      if (curr->vnum != value)
	continue;

      UnLink (curr, mprog, next, prev);

      free_mprog (curr);
    }
  SetBit (ad->area_flags, AREA_CHANGED);
  if (ch)
    chprintln (ch, "MobProgram Code Deleted.");

  return true;
}


Olc_Fun (opedit_delete)
{
  ProgCode *curr, *next;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum", NULL);
      return false;
    }

  if (get_prog_index (value, PRG_OPROG) == NULL)
    {
      if (ch)
	chprintln (ch, "Obj program not found.");
      return false;
    }

  ad = get_vnum_area (value);
  if (ad == NULL)
    {
      if (ch)
	chprintln (ch, "Obj program not assigned to an area.");
      return false;
    }

  if (ch && !IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create ObjProgs.");
      return false;
    }

  unlink_oprog (value);
  for (curr = oprog_first; curr != NULL; curr = next)
    {
      next = curr->next;

      if (curr->vnum != value)
	continue;

      UnLink (curr, oprog, next, prev);

      free_oprog (curr);
    }
  SetBit (ad->area_flags, AREA_CHANGED);
  if (ch)
    chprintln (ch, "Obj Program Code Deleted.");

  return true;
}


Olc_Fun (rpedit_delete)
{
  ProgCode *curr, *next;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      if (ch)
	cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if (get_prog_index (value, PRG_RPROG) == NULL)
    {
      if (ch)
	chprintln (ch, "Room program not found.");
      return false;
    }

  ad = get_vnum_area (value);
  if (ad == NULL)
    {
      if (ch)
	chprintln (ch, "Room program not assigned to an area.");
      return false;
    }

  if (ch && !IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create RoomProgs.");
      return false;
    }

  unlink_rprog (value);
  for (curr = rprog_first; curr != NULL; curr = next)
    {
      next = curr->next;

      if (curr->vnum != value)
	continue;
      UnLink (curr, rprog, next, prev);

      free_rprog (curr);
    }
  SetBit (ad->area_flags, AREA_CHANGED);
  if (ch)
    chprintln (ch, "Room Program Code Deleted.");

  return true;
}


Olc_Fun (gredit_delete)
{
  GroupData *pGroup;

  GetEdit (ch, GroupData, pGroup);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this group!");
      return false;
    }
  else
    {
      int i, j = 0, c;
      struct group_type *new_table;

      alloc_mem (new_table, struct group_type, top_group);

      if (!new_table)
	{
	  chprintln (ch,
		     "Memory Allocation error!!! Unable to delete group.");
	  return false;
	}

      c = group_lookup (pGroup->name);

      for (i = 0; i < top_group; i++)
	if (i != c)
	  new_table[j++] = group_table[i];

      free_string (pGroup->name);
      for (j = 0; j < MAX_IN_GROUP; j++)
	free_string (pGroup->spells[j]);
      free_mem (pGroup->rating);
      free_mem (group_table);
      group_table = new_table;
      top_group--;

      pGroup = &group_table[0];
      edit_start (ch, pGroup, ED_GROUP);
      chprintln (ch, "Group deleted.");
    }

  return true;
}


Olc_Fun (dedit_delete)
{
  DeityData *pDeity;

  GetEdit (ch, DeityData, pDeity);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this deity!");
      return false;
    }
  else
    {
      UnLink (pDeity, deity, next, prev);
      free_deity (pDeity);
      pDeity = deity_first;
      ch->desc->pEdit = (void *) pDeity;

      chprintln (ch, "Deity deleted.");
    }

  return true;
}


Olc_Fun (cmdedit_delete)
{
  CmdData *pCmd;

  GetEdit (ch, CmdData, pCmd);

  if (!NullStr (argument))
    {
      if (IsSet (pCmd->flags, CMD_DELETED))
	{
	  chprintln (ch, "Delete cancelled.");
	  return true;
	}
      else
	chprintln (ch, "No deletion was in progress.");

      return false;
    }

  if (!IsSet (pCmd->flags, CMD_DELETED))
    {
      chprintln
	(ch,
	 "Typing 'delete' again will permanetely remove this command!"
	 NEWLINE "-or- 'delete' with any argument will cancel the deletion.");
      SetBit (pCmd->flags, CMD_DELETED);
      return false;
    }
  else
    {
      unlink_command (pCmd);
      free_cmd (pCmd);
      pCmd = cmd_first;
      edit_start (ch, pCmd, ED_CMD);
      chprintln (ch, "Command deleted.");
    }

  return true;
}


Olc_Fun (cledit_delete)
{
  ClassData *pClass;

  GetEdit (ch, ClassData, pClass);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch, "Typing 'delete confirm' will permanetely remove this class!");
      return false;
    }
  else
    {
      int i, j = 0, c;
      struct class_type *new_table;
      RaceData *race;
      CharData *pch;

      alloc_mem (new_table, struct class_type, top_class);

      if (!new_table)
	{
	  chprintln (ch,
		     "Memory Allocation error!!! Unable to delete class.");
	  return false;
	}

      c = class_lookup (pClass->name[0]);

      for (pch = player_first; pch != NULL; pch = pch->next_player)
	{
	  for (i = 0, j = 0; i < MAX_MCLASS; i++)
	    {
	      if (pch->Class[i] == c)
		{
		  pch->Class[i] = -1;
		  j++;
		  continue;
		}
	      pch->Class[i - j] = pch->Class[i];
	      pch->Class[i - j + 1] = -1;
	    }
	}
      for (i = 0; i < top_class; i++)
	if (i != c)
	  new_table[j++] = class_table[i];

      for (i = 0; i < MAX_REMORT; i++)
	free_string (pClass->name[i]);
      free_string (pClass->description);
      free_mem (class_table);
      class_table = new_table;
      top_class--;

      for (i = 0; i < top_skill; i++)
	{
	  realloc_mem (skill_table[i].rating, int, top_class);
	  realloc_mem (skill_table[i].skill_level, int, top_class);
	}

      for (i = 0; i < top_group; i++)
	realloc_mem (group_table[i].rating, int, top_class);

      for (race = race_first; race; race = race->next)
	realloc_mem (race->class_mult, int, top_class);

      pClass = &class_table[0];
      edit_start (ch, pClass, ED_CLASS);
      chprintln (ch, "Class deleted.");
    }

  return true;
}


Olc_Fun (hedit_delete)
{
  HelpData *pHelp;

  GetEdit (ch, HelpData, pHelp);

  if (str_cmp (argument, "confirm"))
    {
      chprintln
	(ch,
	 "Typing 'delete confirm' will permanetely remove this help file!");
      return false;
    }
  else
    {
      stop_editing (pHelp);

      UnLink (pHelp, help, next, prev);

      free_help (pHelp);

      chprintln (ch, "Ok.");
    }
  return true;
}

#endif
