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


#ifndef __OLC_CREATE_H_
#define __OLC_CREATE_H_ 1

Olc_Fun (aedit_create)
{
  AreaData *pArea;

  pArea = new_area ();
  add_area (pArea);
  edit_start (ch, pArea, ED_AREA);
  SetBit (pArea->area_flags, AREA_ADDED);
  chprintln (ch, "Area Created.");
  return false;
}


Olc_Fun (redit_create)
{
  AreaData *pArea;
  RoomIndex *pRoom;
  vnum_t value;
  int iHash;

  value = atov (argument);

  if (NullStr (argument) || value <= 0)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum > 0>", NULL);
      return false;
    }

  pArea = get_vnum_area (value);
  if (!pArea)
    {
      chprintln (ch, "That vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, pArea))
    {
      chprintln (ch, "Vnum in an area you cannot build in.");
      return false;
    }

  if (get_room_index (value))
    {
      chprintln (ch, "Room vnum already exists.");
      return false;
    }

  pRoom = new_room_index ();
  pRoom->area = pArea;
  pRoom->vnum = value;

  if (value > top_vnum_room)
    top_vnum_room = value;

  iHash = value % MAX_KEY_HASH;
  LinkSingle (pRoom, room_index_hash[iHash], next);
  edit_start (ch, pRoom, ED_ROOM);
  chprintln (ch, "Room created.");
  return true;
}


Olc_Fun (oedit_create)
{
  ObjIndex *pObj;
  AreaData *pArea;
  vnum_t value;
  int iHash;

  value = atov (argument);
  if (NullStr (argument) || value == 0)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  pArea = get_vnum_area (value);
  if (!pArea)
    {
      chprintln (ch, "That vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, pArea))
    {
      chprintln (ch, "Vnum in an area you cannot build in.");
      return false;
    }

  if (get_obj_index (value))
    {
      chprintln (ch, "Object vnum already exists.");
      return false;
    }

  pObj = new_obj_index ();
  pObj->vnum = value;
  pObj->area = pArea;

  if (value > top_vnum_obj)
    top_vnum_obj = value;

  iHash = value % MAX_KEY_HASH;
  LinkSingle (pObj, obj_index_hash[iHash], next);
  edit_start (ch, pObj, ED_OBJECT);
  chprintln (ch, "Object Created.");
  return true;
}


Olc_Fun (medit_create)
{
  CharIndex *pMob;
  AreaData *pArea;
  vnum_t value;
  int iHash;

  value = atov (argument);
  if (NullStr (argument) || value == 0)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  pArea = get_vnum_area (value);

  if (!pArea)
    {
      chprintln (ch, "That vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, pArea))
    {
      chprintln (ch, "Vnum in an area you cannot build in.");
      return false;
    }

  if (get_char_index (value))
    {
      chprintln (ch, "Mobile vnum already exists.");
      return false;
    }

  pMob = new_char_index ();
  pMob->vnum = value;
  pMob->area = pArea;

  if (value > top_vnum_mob)
    top_vnum_mob = value;

  pMob->act = ACT_IS_NPC;
  iHash = value % MAX_KEY_HASH;
  LinkSingle (pMob, char_index_hash[iHash], next);
  edit_start (ch, pMob, ED_MOBILE);
  chprintln (ch, "Mobile Created.");
  return true;
}


Olc_Fun (cedit_create)
{
  ClanData *pClan;
  char buf[MIL];

  if (!NullStr (argument) && clan_lookup (argument) == NULL)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Clan%d", top_clan + 1);

  pClan = new_clan ();
  replace_str (&pClan->name, buf);
  replace_str (&pClan->who_name, buf);
  Link (pClan, clan, next, prev);

  edit_start (ch, pClan, ED_CLAN);
  chprintln (ch, "Clan created.");
  return true;
}


Olc_Fun (cledit_create)
{
  int j, i = top_class;
  int x = 0;
  ClassData *pClass;
  struct class_type *new_table;
  RaceData *race;
  char buf[MIL];

  if (!NullStr (argument) && class_lookup (argument) == -1)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Class%d", top_class + 1);

  top_class++;

  alloc_mem (new_table, struct class_type, top_class);

  if (!new_table)
    {
      chprintln (ch, "Memory Allocation Failed!!! Unable to create class.");
      return false;
    }

  for (j = 0; j < i; j++)
    new_table[j] = class_table[j];

  for (x = 0; x < top_skill; x++)
    {
      realloc_mem (skill_table[x].rating, int, top_class);

      skill_table[x].rating[i] = 0;
      realloc_mem (skill_table[x].skill_level, int, top_class);

      skill_table[x].skill_level[i] = ANGEL;
    }

  for (race = race_first; race; race = race->next)
    {
      realloc_mem (race->class_mult, int, top_class);

      race->class_mult[i] = 100;
    }

  for (x = 0; x < top_group; x++)
    {
      realloc_mem (group_table[x].rating, int, top_class);

      group_table[x].rating[i] = 0;
    }

  free_mem (class_table);
  class_table = new_table;

  class_table[i].name[0] = str_dup (buf);
  for (x = 1; x < MAX_REMORT; x++)
    class_table[i].name[x] = &str_empty[0];
  class_table[i].description = &str_empty[0];
  class_table[i].index = &class_null;
  pClass = &class_table[i];
  edit_start (ch, pClass, ED_CLASS);
  chprintln (ch, "Class created.");
  return true;
}


Olc_Fun (cmdedit_create)
{
  CmdData *pCmd;
  char buf[MIL];

  if (!NullStr (argument) && cmd_lookup (argument) == NULL)
    sprintf (buf, argument);
  else
    sprintf (buf, "NewCommand%d", top_cmd + 1);

  pCmd = new_cmd ();
  replace_str (&pCmd->name, buf);
  add_command (pCmd);

  edit_start (ch, pCmd, ED_CMD);
  chprintln (ch, "Command created.");
  return true;
}


Olc_Fun (dedit_create)
{
  DeityData *pDeity;
  char buf[MIL];

  if (!NullStr (argument) && deity_lookup (argument) == NULL)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Deity%d", top_deity + 1);

  pDeity = new_deity ();
  replace_str (&pDeity->name, buf);
  Link (pDeity, deity, next, prev);
  edit_start (ch, pDeity, ED_DEITY);
  chprintln (ch, "Deity created.");
  return true;
}


Olc_Fun (gredit_create)
{
  int j, i = top_group;
  int x = 0;
  GroupData *pGroup;
  struct group_type *new_table;
  char buf[MIL];
  CharData *pch;

  if (!NullStr (argument) && group_lookup (argument) == -1)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Group%d", top_group + 1);

  top_group++;

  alloc_mem (new_table, struct group_type, top_group);

  if (!new_table)
    {
      chprintln (ch, "Memory Allocation Failed!!! Unable to create group.");
      return false;
    }

  for (j = 0; j < i; j++)
    new_table[j] = group_table[j];

  free_mem (group_table);
  group_table = new_table;

  group_table[i].name = str_dup (buf);

  for (x = 0; x < MAX_IN_GROUP; x++)
    group_table[i].spells[x] = NULL;

  alloc_mem (group_table[i].rating, int, top_class);

  for (pch = player_first; pch; pch = pch->next_player)
    {
      realloc_mem (pch->pcdata->group_known, bool, top_group);
      if (pch->gen_data != NULL)
	realloc_mem (pch->gen_data->group_chosen, bool, top_group);
    }
  pGroup = &group_table[i];
  chprintln (ch, "Group created.");
  edit_start (ch, pGroup, ED_GROUP);
  return true;
}


Olc_Fun (mpedit_create)
{
  ProgCode *pMcode;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  ad = get_vnum_area (value);

  if (ad == NULL)
    {
      chprintln (ch, "Vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create MobProgs.");
      return false;
    }

  if (get_prog_index (value, PRG_MPROG))
    {
      chprintln (ch, "Code vnum already exists.");
      return false;
    }

  pMcode = new_rprog ();
  pMcode->vnum = value;
  pMcode->area = ad;
  Link (pMcode, mprog, next, prev);
  edit_start (ch, pMcode, ED_MPCODE);
  chprintln (ch, "MobProgram Code Created.");

  return true;
}

Olc_Fun (opedit_create)
{
  ProgCode *pOcode;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  ad = get_vnum_area (value);

  if (ad == NULL)
    {
      chprintln (ch, "Vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create ObjProgs.");
      return false;
    }

  if (get_prog_index (value, PRG_OPROG))
    {
      chprintln (ch, "Code vnum already exists.");
      return false;
    }

  pOcode = new_rprog ();
  pOcode->vnum = value;
  pOcode->area = ad;
  Link (pOcode, oprog, next, prev);
  edit_start (ch, pOcode, ED_OPCODE);
  chprintln (ch, "ObjProgram Code Created.");

  return true;
}

Olc_Fun (rpedit_create)
{
  ProgCode *pRcode;
  vnum_t value = atov (argument);
  AreaData *ad;

  if (NullStr (argument) || value < 1)
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  ad = get_vnum_area (value);

  if (ad == NULL)
    {
      chprintln (ch, "Vnum is not assigned an area.");
      return false;
    }

  if (!IsBuilder (ch, ad))
    {
      chprintln (ch, "Insufficient security to create RoomProgs.");
      return false;
    }

  if (get_prog_index (value, PRG_RPROG))
    {
      chprintln (ch, "Code vnum already exists.");
      return false;
    }

  pRcode = new_rprog ();
  pRcode->vnum = value;
  pRcode->area = ad;
  Link (pRcode, rprog, next, prev);
  edit_start (ch, pRcode, ED_RPCODE);
  chprintln (ch, "RoomProgram Code Created.");

  return true;
}


Olc_Fun (raedit_create)
{
  RaceData *pRace;
  char buf[MIL];

  if (!NullStr (argument) && race_lookup (argument) == NULL)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Race%d", top_race + 1);

  pRace = new_race ();
  replace_str (&pRace->name, buf);
  Link (pRace, race, next, prev);
  edit_start (ch, pRace, ED_RACE);
  chprintln (ch, "Race created.");
  return true;
}


Olc_Fun (skedit_create)
{
  int j, i = top_skill;
  SkillData *pSkill;
  struct skill_type *new_table;
  char buf[MIL];
  CharData *pch;

  if (!NullStr (argument) && skill_lookup (argument) == -1)
    sprintf (buf, argument);
  else
    sprintf (buf, "New_Skill%d", top_skill + 1);

  top_skill++;

  alloc_mem (new_table, struct skill_type, top_skill);

  if (!new_table)
    {
      chprintln (ch, "Memory Allocation Failed!!! Unable to create skill.");
      return false;
    }

  for (j = 0; j < i; j++)
    new_table[j] = skill_table[j];

  free_mem (skill_table);
  skill_table = new_table;

  skill_table[i].name = str_dup (buf);

  skill_table[i].spell_fun = spell_null;
  skill_table[i].pgsn = &gsn_null;
  skill_table[i].min_mana = 0;
  skill_table[i].target = TAR_IGNORE;
  skill_table[i].minimum_position = POS_STANDING;
  skill_table[i].beats = 0;
  skill_table[i].noun_damage = str_empty;
  skill_table[i].msg_off = str_empty;
  skill_table[i].msg_obj = str_empty;
  alloc_mem (skill_table[i].skill_level, int, top_class);
  alloc_mem (skill_table[i].rating, int, top_class);

  for (pch = player_first; pch; pch = pch->next_player)
    {
      realloc_mem (pch->pcdata->learned, int, top_skill);

      if (pch->gen_data != NULL)
	realloc_mem (pch->gen_data->skill_chosen, bool, top_skill);
    }
  pSkill = &skill_table[i];
  edit_start (ch, pSkill, ED_SKILL);
  chprintln (ch, "Skill created.");
  return true;
}


Olc_Fun (songedit_create)
{
  int j, i = top_song;
  SongData *pSong;
  struct song_data *new_table;
  char buf[MIL];

  if (!NullStr (argument) && song_lookup (argument) == -1)
    sprintf (buf, argument);
  else
    sprintf (buf, "New Song%d", top_song + 1);

  top_song++;

  alloc_mem (new_table, struct song_data, top_song);

  if (!new_table)
    {
      chprintln (ch, "Memory Allocation Failed!!! Unable to create song.");
      return false;
    }

  for (j = 0; j < i; j++)
    new_table[j] = song_table[j];

  free_mem (song_table);
  song_table = new_table;

  song_table[i].name = str_dup (buf);
  song_table[i].group = &str_empty[0];
  for (j = 0; j < MAX_LINES; j++)
    song_table[i].lyrics[j] = NULL;

  pSong = &song_table[i];
  song_table[top_song].name = NULL;
  edit_start (ch, pSong, ED_SONG);
  chprintln (ch, "Song created.");
  return true;
}


Olc_Fun (sedit_create)
{
  SocialData *pSocial;
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<name>", NULL);
      return false;
    }
  if (social_lookup (arg) != NULL)
    {
      chprintln (ch, "A social with that name already exists.");
      return false;
    }
  pSocial = new_social ();
  replace_str (&pSocial->name, arg);
  add_social (pSocial);
  edit_start (ch, pSocial, ED_SOCIAL);
  chprintln (ch, "Social created.");
  return true;
}


Olc_Fun (hedit_create)
{
  HelpData *help;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<keywords>", NULL);
      return false;
    }

  if (help_lookup (argument))
    {
      chprintln (ch, "Help already exists.");
      return false;
    }

  help = new_help ();
  help->level = 0;
  help->keyword = str_dup (strupper (argument));
  help->text = str_dup ("");

  add_help (help);

  edit_start (ch, help, ED_HELP);
  chprintln (ch, "Ok.");
  return false;
}

#endif
