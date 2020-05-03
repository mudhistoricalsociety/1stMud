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


#ifndef __OLC_DOFUN_H_
#define __OLC_DOFUN_H_  1

Do_Fun (do_olc_help)
{
  int i;

  chprintln (ch, "Syntax:");
  for (i = 0; i < ED_MAX; i++)
    {
      chprintlnf (ch, "\t%s %s (%s data)", n_fun,
		  olc_ed_table[i].longname, olc_ed_table[i].name);
    }
  chprintln (ch, "See 'HELP OLC' for more information.");
}


Do_Fun (do_olc)
{
  char command[MAX_INPUT_LENGTH];
  int cmd;

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, command);

  if (NullStr (command))
    {

      do_olc_help (n_fun, ch, "");
      return;
    }


  for (cmd = 0; cmd < ED_MAX; cmd++)
    {
      if (!str_prefix (command, olc_ed_table[cmd].longname))
	{
	  (*olc_ed_table[cmd].do_fun) (olc_ed_table[cmd].name, ch, argument);
	  return;
	}
    }



  do_olc_help (n_fun, ch, "");
  return;
}



Do_Fun (do_aedit)
{
  AreaData *pArea;
  int value;
  char arg[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    return;

  pArea = ch->in_room->area;

  argument = one_argument (argument, arg);

  if (is_number (arg))
    {
      value = atoi (arg);
      if (!(pArea = get_area_data (value)))
	{
	  chprintln (ch, "That area vnum does not exist.");
	  return;
	}
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insufficient security to create area.");
	  return;
	}

      aedit_create (arg, ch, argument);
      ch->desc->editor = ED_AREA;
      return;
    }
  else if (!str_cmp (arg, "delete"))
    {
      aedit_delete (arg, ch, argument);
      return;
    }
  if (!IsBuilder (ch, pArea))
    {
      chprintln (ch, "Insuficient security to edit area.");
      return;
    }

  edit_start (ch, pArea, ED_AREA);
  return;
}


Do_Fun (do_redit)
{
  RoomIndex *pRoom;
  char arg1[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg1);

  pRoom = ch->in_room;

  if (!str_cmp (arg1, "reset"))
    {
      if (!IsBuilder (ch, pRoom->area))
	{
	  chprintln (ch, "Insufficient security to modify rooms.");
	  return;
	}

      reset_room (pRoom);
      chprintln (ch, "Room reset.");

      return;
    }
  else if (!str_cmp (arg1, "create"))
    {
      if (NullStr (argument) || atov (argument) == 0)
	{
	  cmd_syntax (ch, NULL, n_fun, "create <vnum>", NULL);
	  return;
	}

      if (redit_create (arg1, ch, argument))
	{
	  ch->desc->editor = ED_ROOM;
	  char_from_room (ch);
	  char_to_room (ch, (RoomIndex *) ch->desc->pEdit);
	  SetBit (((RoomIndex *) ch->desc->pEdit)->area->area_flags,
		  AREA_CHANGED);
	}
      return;
    }
  else if (!str_cmp (arg1, "delete"))
    {
      redit_delete (arg1, ch, argument);
      return;
    }
  else if (!NullStr (arg1))
    {
      pRoom = get_room_index (atov (arg1));

      if (!pRoom)
	{
	  chprintln (ch, "Room vnum doesn't exist.");
	  return;
	}

      if (!IsBuilder (ch, pRoom->area))
	{
	  chprintln (ch, "Insuficient security to edit room.");
	  return;
	}

      char_from_room (ch);
      char_to_room (ch, pRoom);
    }

  if (!IsBuilder (ch, pRoom->area))
    {
      chprintln (ch, "Insuficient security to edit room.");
      return;
    }

  edit_start (ch, pRoom, ED_ROOM);
  return;
}


Do_Fun (do_oedit)
{
  ObjIndex *pObj;
  AreaData *pArea;
  char arg1[MAX_STRING_LENGTH];
  vnum_t value;

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg1);

  if (is_number (arg1))
    {
      value = atov (arg1);
      if (!(pObj = get_obj_index (value)))
	{
	  chprintln (ch, "That vnum does not exist.");
	  return;
	}

      if (!IsBuilder (ch, pObj->area))
	{
	  chprintln (ch, "Insufficient security to modify objects.");
	  return;
	}

      edit_start (ch, pObj, ED_OBJECT);
      return;
    }
  else
    {
      if (!str_cmp (arg1, "create"))
	{
	  value = atov (argument);
	  if (NullStr (argument) || value == 0)
	    {
	      cmd_syntax (ch, NULL, n_fun, "create <vnum>", NULL);
	      return;
	    }

	  pArea = get_vnum_area (value);

	  if (!pArea)
	    {
	      chprintln (ch, "That vnum is not assigned an area.");
	      return;
	    }

	  if (!IsBuilder (ch, pArea))
	    {
	      chprintln (ch, "Insufficient security to modify objects.");
	      return;
	    }

	  if (oedit_create (arg1, ch, argument))
	    {
	      SetBit (pArea->area_flags, AREA_CHANGED);
	      ch->desc->editor = ED_OBJECT;
	    }
	  return;
	}
      else if (!str_cmp (arg1, "delete"))
	{
	  oedit_delete (arg1, ch, argument);
	  return;
	}
    }

  chprintln (ch, "There is no default object to edit.");
  return;
}


Do_Fun (do_medit)
{
  CharIndex *pMob;
  AreaData *pArea;
  vnum_t value;
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument (argument, arg1);

  if (IsNPC (ch))
    return;

  if (is_number (arg1))
    {
      value = atov (arg1);
      if (!(pMob = get_char_index (value)))
	{
	  chprintln (ch, "That vnum does not exist.");
	  return;
	}

      if (!IsBuilder (ch, pMob->area))
	{
	  chprintln (ch, "Insufficient security to modify mobs.");
	  return;
	}

      edit_start (ch, pMob, ED_MOBILE);
      return;
    }
  else
    {
      if (!str_cmp (arg1, "create"))
	{
	  value = atov (argument);
	  if (NullStr (arg1) || value == 0)
	    {
	      cmd_syntax (ch, NULL, n_fun, "create <vnum>", NULL);
	      return;
	    }

	  pArea = get_vnum_area (value);

	  if (!pArea)
	    {
	      chprintln (ch, "That vnum is not assigned an area.");
	      return;
	    }

	  if (!IsBuilder (ch, pArea))
	    {
	      chprintln (ch, "Insufficient security to modify mobs.");
	      return;
	    }

	  if (medit_create (arg1, ch, argument))
	    {
	      SetBit (pArea->area_flags, AREA_CHANGED);
	      ch->desc->editor = ED_MOBILE;
	    }
	  return;
	}
      else if (!str_cmp (arg1, "delete"))
	{
	  medit_delete (arg1, ch, argument);
	  return;
	}
    }

  chprintln (ch, "There is no default mobile to edit.");
  return;
}

Do_Fun (do_cedit)
{
  ClanData *pClan;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pClan = clan_first;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<clan>", NULL);
      return;
    }

  if (!str_cmp (arg, "save"))
    {
      rw_clan_data (act_write);
      chprintln (ch, "Clan database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new clans.");
	  return;
	}
      cedit_create (arg, ch, argument);

      return;
    }
  else if (!(pClan = clan_lookup (arg)) || NullStr (pClan->name))
    {
      chprintln (ch, "That clan does not exist.");
      return;
    }

  edit_start (ch, pClan, ED_CLAN);
  return;
}

Do_Fun (do_cmdedit)
{
  CmdData *pCmd;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<cmd>", NULL);
      return;
    }

  if (!str_cmp (arg, "save"))
    {
      rw_cmd_data (act_write);
      chprintln (ch, "Command database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new commands.");
	  return;
	}

      cmdedit_create (arg, ch, argument);

      return;
    }
  else if (!(pCmd = cmd_lookup (arg)) || NullStr (pCmd->name))
    {
      chprintln (ch, "That command does not exist.");
      return;
    }

  edit_start (ch, pCmd, ED_CMD);

  return;
}

Do_Fun (do_skedit)
{
  SkillData *pSkill;
  int value;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pSkill = &skill_table[0];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<#|skill>",
		  NULL);
      return;
    }

  if (is_number (arg))
    value = atoi (arg);
  else
    value = skill_lookup (arg);

  if (value > -1)
    {
      pSkill = get_skill_data (value);

      if (!pSkill || NullStr (pSkill->name))
	{
	  chprintln (ch, "That skill does not exist.");
	  return;
	}

    }
  else if (!str_cmp (arg, "save"))
    {
      rw_skill_data (act_write);
      chprintln (ch, "Skill database saved.");
      return;
    }
  else if (!str_cmp (arg, "make") || !str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new skills.");
	  return;
	}

      skedit_create (arg, ch, argument);

      return;
    }

  edit_start (ch, pSkill, ED_SKILL);
  return;
}

Do_Fun (do_gredit)
{
  GroupData *pGroup;
  int value;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pGroup = &group_table[0];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<#|group>",
		  NULL);
      return;
    }

  if (is_number (arg))
    value = atoi (arg);
  else
    value = group_lookup (arg);

  if (value > -1)
    {
      pGroup = get_group_data (value);

      if (!pGroup || NullStr (pGroup->name))
	{
	  chprintln (ch, "That skill does not exist.");
	  return;
	}

    }
  else if (!str_cmp (arg, "save"))
    {
      rw_group_data (act_write);
      chprintln (ch, "Group database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln
	    (ch, "grEdit : Insuffecient security to create new groups.");
	  return;
	}

      gredit_create (arg, ch, argument);

      return;
    }

  edit_start (ch, pGroup, ED_GROUP);
  return;
}

Do_Fun (do_raedit)
{
  RaceData *pRace;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pRace = race_first;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<name>", NULL);
      return;
    }
  else if (!str_cmp (arg, "save"))
    {
      rw_race_data (act_write);
      chprintln (ch, "Race database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new races.");
	  return;
	}

      raedit_create (arg, ch, argument);

      return;
    }
  else if ((pRace = race_lookup (arg)) == NULL)
    {
      chprintln (ch, "That race does not exist.");
      return;
    }

  edit_start (ch, pRace, ED_RACE);
  return;
}

Do_Fun (do_cledit)
{
  ClassData *pClass;
  int value;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pClass = &class_table[0];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<#|class>",
		  NULL);
      return;
    }

  if (is_number (arg))
    value = atoi (arg);
  else
    value = class_lookup (arg);

  if (value > -1)
    {
      pClass = get_class_data (value);

      if (!pClass || NullStr (pClass->name))
	{
	  chprintln (ch, "That class does not exist.");
	  return;
	}

    }
  else if (!str_cmp (arg, "save"))
    {
      rw_class_data (act_write);
      chprintln (ch, "Class database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln
	    (ch, "clEdit : Insuffecient security to create new class.");
	  return;
	}

      cledit_create (arg, ch, argument);

      return;
    }
  edit_start (ch, pClass, ED_CLASS);
  return;
}


Do_Fun (do_chanedit)
{
  ChannelData *pChan;
  int value;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pChan = &channel_table[0];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<#|name>", NULL);
      return;
    }

  if (is_number (arg))
    value = atoi (arg);
  else
    value = channel_lookup (arg);

  if (value > -1)
    {
      pChan = get_chan_data (value);

      if (!pChan)
	{
	  chprintln (ch, "That channel does not exist.");
	  return;
	}

    }
  else if (!str_cmp (arg, "save"))
    {
      rw_channel_data (act_write);
      chprintln (ch, "Channel database saved.");
      return;
    }
  else if (!str_cmp (arg, "make") || !str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new channels.");
	  return;
	}

      chanedit_create (arg, ch, argument);

      return;
    }

  edit_start (ch, pChan, ED_CHAN);
  return;
}

Do_Fun (do_songedit)
{
  SongData *pSong;
  int value;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pSong = &song_table[0];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<#|name>", NULL);
      return;
    }

  if (is_number (arg))
    value = atoi (arg);
  else
    value = song_lookup (arg);

  if (value > -1)
    {
      pSong = get_song_data (value);

      if (!pSong)
	{
	  chprintln (ch, "That song does not exist.");
	  return;
	}

    }
  else if (!str_cmp (arg, "save"))
    {
      rw_song_data (act_write);
      chprintln (ch, "Song database saved.");
      return;
    }
  else if (!str_cmp (arg, "make") || !str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new songs.");
	  return;
	}

      songedit_create (arg, ch, argument);

      return;
    }

  edit_start (ch, pSong, ED_SONG);
  return;
}

Do_Fun (do_mudedit)
{
  if (IsNPC (ch))
    return;

  if (get_trust (ch) < MAX_LEVEL - 1)
    {
      chprintln (ch, "Insufficient security to edit mud data.");
      return;
    }

  edit_start (ch, &mud_info, ED_Mud);
  return;
}


Do_Fun (do_dedit)
{
  DeityData *pDeity;
  char arg[MSL];

  if (IsNPC (ch))
    return;

  pDeity = deity_first;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "create <name>", "save", "<name>", NULL);
      return;
    }

  if (!str_cmp (arg, "save"))
    {
      rw_deity_data (act_write);
      chprintln (ch, "Deity database saved.");
      return;
    }
  else if (!str_cmp (arg, "create"))
    {
      if (ch->pcdata->security < 9)
	{
	  chprintln (ch, "Insuffecient security to create new deities.");
	  return;
	}

      dedit_create (arg, ch, argument);

      return;
    }
  else if (!(pDeity = deity_lookup (arg)) || NullStr (pDeity->name))
    {
      chprintln (ch, "That deity does not exist.");
      return;
    }

  edit_start (ch, pDeity, ED_DEITY);
  return;
}


Do_Fun (do_mpedit)
{
  ProgCode *pMcode;
  char command[MIL];

  argument = one_argument (argument, command);

  if (is_number (command))
    {
      vnum_t vnum = atov (command);
      AreaData *ad;

      if ((pMcode = get_prog_index (vnum, PRG_MPROG)) == NULL)
	{
	  chprintln (ch, "That vnum does not exist.");
	  return;
	}

      ad = pMcode->area;

      if (ad == NULL)
	{
	  chprintln (ch, "Vnum is not assigned an area.");
	  return;
	}

      if (!IsBuilder (ch, ad))
	{
	  chprintln (ch, "Insufficient security to modify area.");
	  return;
	}

      edit_start (ch, pMcode, ED_MPCODE);
      return;
    }

  if (!str_cmp (command, "create"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "create <vnum>", NULL);
	  return;
	}

      mpedit_create (command, ch, argument);
      return;
    }
  if (!str_cmp (command, "delete"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "delete <vnum>", NULL);
	  return;
	}
      mpedit_delete (command, ch, argument);
      return;
    }

  cmd_syntax (ch, NULL, n_fun, "<vnum>", "create <vnum>", "delete <vnum>",
	      NULL);

  return;
}

Do_Fun (do_opedit)
{
  ProgCode *pOcode;
  char command[MIL];

  argument = one_argument (argument, command);

  if (is_number (command))
    {
      vnum_t vnum = atov (command);
      AreaData *ad;

      if ((pOcode = get_prog_index (vnum, PRG_OPROG)) == NULL)
	{
	  chprintln (ch, "That vnum does not exist.");
	  return;
	}

      ad = pOcode->area;

      if (ad == NULL)
	{
	  chprintln (ch, "Vnum is not assigned an area.");
	  return;
	}

      if (!IsBuilder (ch, ad))
	{
	  chprintln (ch, "Insufficient security to modify area.");
	  return;
	}

      edit_start (ch, pOcode, ED_OPCODE);
      return;
    }

  if (!str_cmp (command, "create"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "create <vnum>", NULL);
	  return;
	}

      opedit_create (command, ch, argument);
      return;
    }
  if (!str_cmp (command, "delete"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "delete <vnum>", NULL);
	  return;
	}
      opedit_delete (command, ch, argument);
      return;
    }

  cmd_syntax (ch, NULL, n_fun, "<vnum>", "create <vnum>", "delete <vnum>",
	      NULL);

  return;
}

Do_Fun (do_rpedit)
{
  ProgCode *pRcode;
  char command[MIL];

  argument = one_argument (argument, command);

  if (is_number (command))
    {
      vnum_t vnum = atov (command);
      AreaData *ad;

      if ((pRcode = get_prog_index (vnum, PRG_RPROG)) == NULL)
	{
	  chprintln (ch, "That vnum does not exist.");
	  return;
	}

      ad = pRcode->area;

      if (ad == NULL)
	{
	  chprintln (ch, "Vnum is not assigned an area.");
	  return;
	}

      if (!IsBuilder (ch, ad))
	{
	  chprintln (ch, "Insufficient security to modify area.");
	  return;
	}

      edit_start (ch, pRcode, ED_RPCODE);
      return;
    }

  if (!str_cmp (command, "create"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
	  return;
	}

      rpedit_create (command, ch, argument);
      return;
    }
  if (!str_cmp (command, "delete"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
	  return;
	}
      rpedit_delete (command, ch, argument);
      return;
    }

  cmd_syntax (ch, NULL, n_fun, "<vnum>", "create <vnum>", "delete <vnum>",
	      NULL);

  return;
}


Do_Fun (do_sedit)
{
  SocialData *pSocial;
  char arg1[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg1);

  if (IsNPC (ch))
    return;

  if (ch->pcdata->security < 5)
    {
      chprintln (ch, "Insuficient security to edit socials.");
      return;
    }

  if (!str_cmp (arg1, "create"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "create <name>", NULL);
	  return;
	}
      if (sedit_create (arg1, ch, argument))
	{
	  ch->desc->editor = ED_SOCIAL;
	  act ("$n has entered the social editor.", ch, NULL, NULL, TO_ROOM);
	}
      return;
    }
  if (!str_cmp (arg1, "delete"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, n_fun, "delete <social>", NULL);
	  return;
	}
      if (sedit_delete (arg1, ch, argument))
	{
	  return;
	}
    }

  if ((pSocial = social_lookup (arg1)) != NULL)
    {
      edit_start (ch, pSocial, ED_SOCIAL);
      return;
    }
  if (pSocial == NULL && !NullStr (arg1))
    {
      chprintln (ch, "That social does not exist.");
      return;
    }

  chprintln (ch, "There is no default social to edit.");
  return;
}


Do_Fun (do_cmdcheck)
{
  CmdData *i;
  Do_F *fun;
  bool found = false;
  Buffer *b;
  Column *Cd;
  int cat;

  Cd = new_column ();
  b = new_buf ();
  set_cols (Cd, ch, 4, COLS_BUF, b);

  if (!str_prefix (argument, "null"))
    {
      bprintln (b, "NULL commands:");
      for (i = cmd_first_sorted; i; i = i->next_sort)
	{
	  if (i->do_fun == do_null)
	    {
	      found = true;
	      print_cols (Cd, i->name);
	    }
	}
      if (!found)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else if (!str_prefix (argument, "hidden"))
    {
      bprintln (b, "Hidden commands:");
      for (i = cmd_first_sorted; i; i = i->next_sort)
	{
	  if (!i->category == CMDCAT_NOSHOW)
	    {
	      found = true;
	      print_cols (Cd, i->name);
	    }
	}
      if (!found)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else if (!str_prefix (argument, "missing"))
    {
      int j;
      bool any = false;

      bprintln (b, "Functions missing command entries:");
      for (j = 0; dofun_index[j].name != NULL; j++)
	{
	  for (i = cmd_first_sorted; i; i = i->next_sort)
	    {
	      found = false;
	      if ((Do_F *) dofun_index[j].index == i->do_fun)
		{
		  found = true;
		  break;
		}
	    }
	  if (!found)
	    {
	      any = true;
	      print_cols (Cd, dofun_index[j].name);
	    }
	}
      if (!any)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else if ((cat = flag_value (cmd_categories, argument)) != NO_FLAG)
    {
      bprintlnf (b, "Commands in %s category:",
		 flag_string (cmd_categories, cat));
      for (i = cmd_first_sorted; i; i = i->next_sort)
	{
	  if (i->category == (cmd_cat) cat)
	    {
	      found = true;
	      print_cols (Cd, i->name);
	    }
	}
      if (!found)
	bprintln (b, "none.");
      else
	cols_nl (Cd);
    }
  else if ((fun = (Do_F *) index_lookup (argument, dofun_index, NULL))
	   != NULL)
    {
      bprintlnf (b, "%s commands:", capitalize (argument));
      for (i = cmd_first_sorted; i; i = i->next_sort)
	{
	  if (i->do_fun == fun)
	    {
	      found = true;
	      print_cols (Cd, i->name);
	    }
	}
      if (!found)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun, "null", "missing", "hidden",
		  "<category", "<dofun_name>", NULL);
      free_buf (b);
      free_column (Cd);
      return;
    }
  sendpage (ch, buf_string (b));
  free_buf (b);
  free_column (Cd);
  return;
}

Do_Fun (do_skcheck)
{
  int i, gsn;
  bool found = false, any = false;
  Column *Cd;
  Buffer *b;

  b = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 4, COLS_BUF, b);

  if (!str_prefix (argument, "gsns"))
    {
      bprintln (b, "Missing gsn entries:");

      for (i = 0; gsn_index[i].name != NULL; i++)
	{
	  found = false;
	  for (gsn = 0; gsn < top_skill; gsn++)
	    {
	      if ((int *) gsn_index[i].index == skill_table[gsn].pgsn)
		{
		  found = true;
		  break;
		}
	    }
	  if (!found)
	    {
	      any = true;
	      print_cols (Cd, gsn_index[i].name);
	    }
	}
      if (!any)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else if (!str_prefix (argument, "spells"))
    {
      bprintln (b, "Missing spell entries:");

      for (i = 0; spell_index[i].name != NULL; i++)
	{
	  found = false;
	  for (gsn = 0; gsn < top_skill; gsn++)
	    {
	      if ((Spell_F *) spell_index[i].index ==
		  skill_table[gsn].spell_fun)
		{
		  found = true;
		  break;
		}
	    }
	  if (!found)
	    {
	      any = true;
	      print_cols (Cd, spell_index[i].name);
	    }
	}
      if (!any)
	bprintln (b, "None.");
      else
	cols_nl (Cd);
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun, "gsns/spells", NULL);
      free_buf (b);
      free_column (Cd);
      return;
    }

  sendpage (ch, buf_string (b));
  free_column (Cd);
  free_buf (b);
  return;
}


Do_Fun (do_hedit)
{
  HelpData *pHelp;
  char arg[MIL];

  if (IsNPC (ch))
    return;

  argument = one_argument (argument, arg);

  if (is_name (arg, "create new make"))
    {
      hedit_create (arg, ch, argument);
      return;
    }

  strcat (arg, " ");
  strcat (arg, argument);

  if ((pHelp = help_lookup (arg)) == NULL)
    {
      chprintln (ch, "Help doesn't exist.");
      return;
    }

  edit_start (ch, pHelp, ED_HELP);
  return;
}

#endif
