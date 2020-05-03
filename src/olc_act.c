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
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
*  This code was freely distributed with the The Isles 1.1 source code,   *
*  and has been used here for OLC - OLC would not be what it is without   *
*  all the previous coders who released their source code.                *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/



#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "interp.h"
#include "magic.h"
#include "vnums.h"
#include "special.h"

#define ALT_FLAGVALUE_SET( _blargh, _table, _arg )    	    	\
    	{    	    	    	    	    	    	    	\
    	    	flag_t blah = flag_value( _table, _arg );    	    	\
    	    	_blargh = Max(0, blah);    	    	\
    	}

#define ALT_FLAGVALUE_TOGGLE( _blargh, _table, _arg )    	    	\
    	{    	    	    	    	    	    	    	\
    	    	flag_t blah = flag_value( _table, _arg );    	    	\
    	    	ToggleBit(_blargh, Max(0, blah));    	\
    	}


const char *OLC_VERSION[] = {

  "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]",
  "Port a ROM 2.4 v1.8",
  "Port to 1stMud v2.7" NEWLINE,

  "By Jason(jdinkel@mines.colorado.edu)",
  "Modified for use with ROM 2.3",
  "By Hans Birkeland (hansbi@ifi.uio.no)",
  "Modificado para uso en ROM 2.4b6",
  "Por Ivan Toledo (itoledo@ctcreuna.cl)",
  "Modified for 1stMud by Markanth (markanth@firstmud.com)" NEWLINE,

  "(Apr. 7, 1995 - ROM mod, Apr 16, 1995)",
  "(Port a ROM 2.4 - Nov 2, 1996)",
  "Version actual : 1.8 - Sep 8, 1998",
  "1stMud version - Nov 6, 2003" NEWLINE,

  "Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)",
  NULL
};

Olc_Fun (show_olc_version)
{
  int i;

  for (i = 0; OLC_VERSION[i] != NULL; i++)
    olc_msg (ch, n_fun, OLC_VERSION[i]);

  return false;
}


void
show_flag_cmds (CharData * ch, FlagTable * flag_table)
{
  int flag;
  Column Cd;

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (flag_table[flag].settable)
	{
	  print_cols (&Cd, flag_table[flag].name);
	}
    }

  cols_nl (&Cd);
  return;
}


void
show_skill_cmds (CharData * ch, tar_t tar)
{
  int sn;
  Column Cd;

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  for (sn = 0; sn < top_skill; sn++)
    {
      if (!skill_table[sn].name)
	break;

      if (!str_cmp (skill_table[sn].name, "reserved") ||
	  skill_table[sn].spell_fun == spell_null)
	continue;

      if (tar == TAR_NONE || skill_table[sn].target == tar)
	{
	  print_cols (&Cd, skill_table[sn].name);
	}
    }

  cols_nl (&Cd);
  return;
}


void
show_spec_cmds (CharData * ch)
{
  int spec;
  Column Cd;

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  chprintln (ch, "Preceed special functions with 'spec_'");
  for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
      print_cols (&Cd, &spec_table[spec].name[5]);
    }

  cols_nl (&Cd);
  return;
}


bool
show_help_structure (CharData * ch, const void *structure,
		     const char *spell, const char *n_fun)
{
  if (structure == spec_table)
    {
      show_spec_cmds (ch);
      return false;
    }
  else if (structure == liq_table)
    {
      show_liqlist (ch);
      return false;
    }
  else if (structure == attack_table)
    {
      show_damlist (ch);
      return false;
    }
  else if (structure == skill_table)
    {
      tar_t type;
      int i;

      if (NullStr (spell))
	{
	  cmd_syntax (ch, NULL, n_fun, "target_type:", NULL);
	  for (i = 0; target_flags[i].name != NULL; i++)
	    chprintlnf (ch, "\t%s", target_flags[i].name);
	  return false;
	}

      type = (tar_t) flag_value (target_flags, spell);

      show_skill_cmds (ch, type);
      return false;
    }
  else if (structure)
    {
      show_flag_cmds (ch, (FlagTable *) structure);
      return false;
    }
  return false;
}


Olc_Fun (show_olc_help)
{
  char arg[MIL];
  char spell[MIL];
  int cnt;

  argument = one_argument (argument, arg);
  one_argument (argument, spell);

  if (NullStr (arg))
    {
      Column Cd;

      set_cols (&Cd, ch, 4, COLS_CHAR, ch);

      cmd_syntax (ch, NULL, n_fun, "<command>", NULL);
      cols_header (&Cd, "[Command]");

      for (cnt = 0; flag_stat_table[cnt].name != NULL; cnt++)
	{
	  print_cols (&Cd, capitalize (flag_stat_table[cnt].name));
	}
      cols_nl (&Cd);
      return false;
    }

  for (cnt = 0; flag_stat_table[cnt].name != NULL; cnt++)
    {
      if (toupper (arg[0]) == toupper (flag_stat_table[cnt].name[0])
	  && !str_prefix (arg, flag_stat_table[cnt].name))
	{
	  show_help_structure (ch, flag_stat_table[cnt].structure, spell,
			       n_fun);
	  return false;
	}
    }

  show_olc_help (n_fun, ch, "");
  return false;
}

Olc_Fun (mudedit_reset)
{
  mud_info.stats.logins = 0;
  mud_info.stats.quests = 0;
  mud_info.stats.qcomplete = 0;
  mud_info.stats.levels = 0;
  mud_info.stats.newbies = 0;
  mud_info.stats.deletions = 0;
  mud_info.stats.mobdeaths = 0;
  mud_info.stats.auctions = 0;
  mud_info.stats.aucsold = 0;
  mud_info.stats.pdied = 0;
  mud_info.stats.pkill = 0;
  mud_info.stats.notes = 0;
  mud_info.stats.remorts = 0;
  mud_info.stats.wars = 0;
  mud_info.stats.gquests = 0;
  mud_info.stats.connections = 0;
  mud_info.stats.web_requests = 0;
  mud_info.stats.chan_msgs = 0;
  mud_info.stats.lastupdate = current_time;
  mud_info.stats.version++;
  chprintln (ch, "Gamestats reset.");
  return true;
}

Olc_Fun (redit_format)
{
  RoomIndex *pRoom;

  EditRoom (ch, pRoom);

  pRoom->description = format_string (pRoom->description);

  olc_msg (ch, n_fun, "String formatted.");
  return true;
}

Olc_Fun (redit_mshow)
{
  CharIndex *pMob;
  vnum_t value;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if (!is_number (argument))
    {
      olc_msg (ch, n_fun, "Is not a number.");
      return false;
    }

  value = atov (argument);
  if (!(pMob = get_char_index (value)))
    {
      olc_msg (ch, n_fun, "That mobile does not exist.");
      return false;
    }

  ch->desc->pEdit = (void *) pMob;

  olc_show (ch, olc_lookup (ED_MOBILE));
  ch->desc->pEdit = (void *) ch->in_room;
  return false;
}

Olc_Fun (redit_oshow)
{
  ObjIndex *pObj;
  vnum_t value;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if (!is_number (argument))
    {
      olc_msg (ch, n_fun, "That is not a number.");
      return false;
    }

  value = atov (argument);
  if (!(pObj = get_obj_index (value)))
    {
      olc_msg (ch, n_fun, "That object does not exist.");
      return false;
    }

  ch->desc->pEdit = (void *) pObj;

  olc_show (ch, olc_lookup (ED_OBJECT));
  ch->desc->pEdit = (void *) ch->in_room;
  return false;
}


bool
check_range (vnum_t lower, vnum_t upper)
{
  AreaData *pArea;
  int cnt = 0;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {

      if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper) ||
	  (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
	++cnt;

      if (cnt > 1)
	return false;
    }
  return true;
}

AreaData *
get_vnum_area (vnum_t vnum)
{
  AreaData *pArea;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
	return pArea;
    }

  return 0;
}



Olc_Fun (aedit_reset)
{
  AreaData *pArea;

  GetEdit (ch, AreaData, pArea);

  reset_area (pArea);
  chprintln (ch, "Area reset.");

  return false;
}

Olc_Fun (aedit_file)
{
  AreaData *pArea;
  AreaData *cArea;
  char file[MAX_STRING_LENGTH];
  int i, length;

  GetEdit (ch, AreaData, pArea);

  one_argument (argument, file);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "[$file]", NULL);
      return false;
    }


  length = strlen (argument);
  if (length > 8)
    {
      chprintln (ch, "No more than eight characters allowed.");
      return false;
    }


  for (i = 0; i < length; i++)
    {
      if (!isalnum (file[i]))
	{
	  chprintln (ch, "Only letters and numbers are valid.");
	  return false;
	}
    }

  strcat (file, ".are");

  for (cArea = area_first; cArea; cArea = cArea->next)
    {
      if (cArea == pArea)
	continue;

      if (!str_cmp (cArea->file_name, file))
	{
	  chprintln (ch, "There is a file with the same name!!");
	  return false;
	}
    }

  unlink (pArea->file_name);

  replace_str (&pArea->file_name, file);

  chprintln (ch, "Filename set.");
  return true;
}

Olc_Fun (aedit_builder)
{
  AreaData *pArea;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  GetEdit (ch, AreaData, pArea);

  one_argument (argument, name);

  if (NullStr (name))
    {
      cmd_syntax (ch, NULL, n_fun, "<name>", "All", NULL);
      return false;
    }

  name[0] = toupper (name[0]);

  if (strstr (pArea->builders, name) != '\0')
    {
      pArea->builders = string_replace (pArea->builders, name, "\0");
      pArea->builders = string_unpad (pArea->builders);

      if (NullStr (pArea->builders))
	{
	  replace_str (&pArea->builders, "None");
	}
      olc_msg (ch, n_fun, "Builder removed.");
      return true;
    }
  else
    {
      buf[0] = '\0';
      if (strstr (pArea->builders, "None") != '\0')
	{
	  pArea->builders = string_replace (pArea->builders, "None", "\0");
	  pArea->builders = string_unpad (pArea->builders);
	}

      if (!NullStr (pArea->builders))
	{
	  strcat (buf, pArea->builders);
	  strcat (buf, " ");
	}
      strcat (buf, name);
      free_string (pArea->builders);
      pArea->builders = string_proper (str_dup (buf));

      olc_msg (ch, n_fun, "Builder added.");
      return true;
    }

  return false;
}

Olc_Fun (aedit_vnum)
{
  AreaData *pArea;
  char lower[MAX_STRING_LENGTH];
  char upper[MAX_STRING_LENGTH];
  vnum_t ilower;
  vnum_t iupper;

  GetEdit (ch, AreaData, pArea);

  argument = one_argument (argument, lower);
  one_argument (argument, upper);

  if (!is_number (lower) || NullStr (lower) || !is_number (upper) ||
      NullStr (upper))
    {
      cmd_syntax (ch, NULL, n_fun, "<lower> <upper>", NULL);
      return false;
    }

  if ((ilower = atov (lower)) > (iupper = atov (upper)))
    {
      olc_msg (ch, n_fun, "Upper must be larger then lower.");
      return false;
    }

  if (!check_range (ilower, iupper))
    {
      olc_msg (ch, n_fun, "Range must include only this area.");
      return false;
    }

  if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
      olc_msg (ch, n_fun, "Lower vnum already assigned.");
      return false;
    }

  pArea->min_vnum = ilower;
  olc_msg (ch, n_fun, "Lower vnum set.");

  if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
      olc_msg (ch, n_fun, "Upper vnum already assigned.");
      return true;
    }

  if (iupper > MAX_VNUM)
    {
      olc_msg (ch, n_fun, "Vnum can't be higher than %d.", MAX_VNUM);
      return false;
    }

  pArea->max_vnum = iupper;
  olc_msg (ch, n_fun, "Upper vnum set.");

  return true;
}

Olc_Fun (aedit_lvnum)
{
  AreaData *pArea;
  char lower[MAX_STRING_LENGTH];
  vnum_t ilower;
  vnum_t iupper;

  GetEdit (ch, AreaData, pArea);

  one_argument (argument, lower);

  if (!is_number (lower) || NullStr (lower))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if ((ilower = atov (lower)) > (iupper = pArea->max_vnum))
    {
      olc_msg (ch, n_fun, "Value must be less than the max_vnum.");
      return false;
    }

  if (!check_range (ilower, iupper))
    {
      olc_msg (ch, n_fun, "Range must include only this area.");
      return false;
    }

  if (get_vnum_area (ilower) && get_vnum_area (ilower) != pArea)
    {
      olc_msg (ch, n_fun, "Lower vnum already assigned.");
      return false;
    }

  if (ilower > MAX_VNUM)
    {
      olc_msg (ch, n_fun, "Vnum can't be higher than %d", MAX_VNUM);
      return false;
    }

  pArea->min_vnum = ilower;
  olc_msg (ch, n_fun, "Lower vnum set.");
  return true;
}

Olc_Fun (aedit_uvnum)
{
  AreaData *pArea;
  char upper[MAX_STRING_LENGTH];
  vnum_t ilower;
  vnum_t iupper;

  GetEdit (ch, AreaData, pArea);

  one_argument (argument, upper);

  if (!is_number (upper) || NullStr (upper))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  if ((ilower = pArea->min_vnum) > (iupper = atov (upper)))
    {
      olc_msg (ch, n_fun, "Upper must be larger then lower.");
      return false;
    }

  if (!check_range (ilower, iupper))
    {
      olc_msg (ch, n_fun, "Range must include only this area.");
      return false;
    }

  if (get_vnum_area (iupper) && get_vnum_area (iupper) != pArea)
    {
      olc_msg (ch, n_fun, "Upper vnum already assigned.");
      return false;
    }

  pArea->max_vnum = iupper;
  olc_msg (ch, n_fun, "Upper vnum set.");

  return true;
}


bool
check_reset_exit (RoomIndex * pRoom, int door)
{
  ResetData *pReset;

  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
      if (reset_door (pReset, false) == door)
	return true;
    }
  return false;
}


bool
change_exit (const char *n_fun, CharData * ch, const char *argument, int door)
{
  RoomIndex *pRoom;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  flag_t value;
  int rev;

  EditRoom (ch, pRoom);


  if ((value = flag_value (exit_flags, argument)) != NO_FLAG)
    {
      RoomIndex *pToRoom;

      if (!pRoom->exit[door])
	{
	  olc_msg (ch, n_fun, "Door doesn't exist.");
	  return false;
	}


      ToggleBit (pRoom->exit[door]->rs_flags, value);

      pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;


      pToRoom = pRoom->exit[door]->u1.to_room;
      rev = rev_dir[door];

      if (pToRoom->exit[rev] != NULL)
	{
	  pToRoom->exit[rev]->rs_flags = pRoom->exit[door]->rs_flags;
	  pToRoom->exit[rev]->exit_info = pRoom->exit[door]->exit_info;
	}

      olc_msg (ch, n_fun, "Exit flag toggled.");
      return true;
    }


  argument = one_argument (argument, command);
  one_argument (argument, arg);

  if (NullStr (command) && NullStr (argument))
    {
      move_char (ch, door, true);
      do_function (ch, &do_redit, "");
      return false;
    }

  if (command[0] == '?')
    {
      do_function (ch, &do_oldhelp, "EXIT");
      return false;
    }

  if (!str_cmp (command, "delete"))
    {
      RoomIndex *pToRoom;

      if (!pRoom->exit[door])
	{
	  olc_msg (ch, n_fun, "Cannot delete a null exit.");
	  return false;
	}


      rev = rev_dir[door];
      pToRoom = pRoom->exit[door]->u1.to_room;

      if (pToRoom->exit[rev])
	{
	  free_exit (pToRoom->exit[rev]);
	  pToRoom->exit[rev] = NULL;
	}


      free_exit (pRoom->exit[door]);
      pRoom->exit[door] = NULL;

      if (pToRoom->area != pRoom->area)
	SetBit (pToRoom->area->area_flags, AREA_CHANGED);

      olc_msg (ch, n_fun, "Exit unlinked.");
      return true;
    }

  if (!str_cmp (command, "link"))
    {
      RoomIndex *toRoom;

      if (check_reset_exit (pRoom, door))
	{
	  olc_msg (ch, n_fun,
		   "There is a random exit reset already using that direction.");
	  return false;
	}

      if (NullStr (arg) || !is_number (arg))
	{
	  cmd_syntax (ch, NULL, n_fun, "link [vnum]", NULL);
	  return false;
	}

      value = atov (arg);

      if (!(toRoom = get_room_index (value)))
	{
	  olc_msg (ch, n_fun, "Cannot link to non-existant room.");
	  return false;
	}

      if (!IsBuilder (ch, toRoom->area))
	{
	  olc_msg (ch, n_fun, "Cannot link to that area.");
	  return false;
	}

      rev = rev_dir[door];

      if (toRoom->exit[rev])
	{
	  olc_msg (ch, n_fun, "Remote side's exit already exists.");
	  return false;
	}

      if (!pRoom->exit[door])
	pRoom->exit[door] = new_exit ();

      pRoom->exit[door]->u1.to_room = toRoom;
      pRoom->exit[door]->orig_door = door;

      if (toRoom->area != pRoom->area)
	SetBit (toRoom->area->area_flags, AREA_CHANGED);

      toRoom->exit[rev] = new_exit ();
      toRoom->exit[rev]->u1.to_room = pRoom;
      toRoom->exit[rev]->orig_door = rev;

      olc_msg (ch, n_fun, "Two-way link established.");
      return true;
    }

  if (!str_cmp (command, "dig"))
    {
      char buf[MAX_STRING_LENGTH];

      if (check_reset_exit (pRoom, door))
	{
	  olc_msg (ch, n_fun,
		   "There is a random exit reset already using that direction.");
	  return false;
	}

      if (NullStr (arg))
	{
	  vnum_t newvnum;
	  bool foundopen = false;
	  AreaData *pArea = pRoom->area;

	  newvnum = pArea->min_vnum;

	  while (foundopen != true && newvnum < top_vnum_room)
	    {
	      newvnum++;
	      if (get_room_index (newvnum))
		foundopen = false;
	      else
		foundopen = true;
	    }

	  if (newvnum > pArea->max_vnum)
	    {
	      olc_msg (ch, n_fun, "Dig Error: No more free vnums in area.");
	      return false;
	    }

	  sprintf (buf, "%ld", newvnum);
	  redit_create (command, ch, buf);
	  sprintf (buf, "link %ld", pRoom->vnum);
	  change_exit (n_fun, ch, buf, rev_dir[door]);
	  return true;
	}
      else
	{
	  if (!is_number (arg))
	    {
	      cmd_syntax (ch, NULL, n_fun, "dig <vnum>", NULL);
	      return false;
	    }

	  redit_create (command, ch, arg);
	  sprintf (buf, "link %ld", pRoom->vnum);
	  change_exit (n_fun, ch, buf, rev_dir[door]);
	  return true;
	}
    }

  if (!str_cmp (command, "room"))
    {
      RoomIndex *toRoom;

      if (check_reset_exit (pRoom, door))
	{
	  olc_msg (ch, n_fun,
		   "There is a random exit reset already using that direction.");
	  return false;
	}

      if (NullStr (arg) || !is_number (arg))
	{
	  cmd_syntax (ch, NULL, n_fun, "room [vnum]", NULL);
	  return false;
	}

      value = atov (arg);

      if (!(toRoom = get_room_index (value)))
	{
	  olc_msg (ch, n_fun, "Cannot link to non-existant room.");
	  return false;
	}

      if (!pRoom->exit[door])
	pRoom->exit[door] = new_exit ();

      pRoom->exit[door]->u1.to_room = toRoom;
      pRoom->exit[door]->orig_door = door;

      olc_msg (ch, n_fun, "One-way link established.");
      return true;
    }

  if (!str_cmp (command, "key"))
    {
      ObjIndex *key;

      if (NullStr (arg) || !is_number (arg))
	{
	  cmd_syntax (ch, NULL, n_fun, "key [vnum]", NULL);
	  return false;
	}

      if (!pRoom->exit[door])
	{
	  olc_msg (ch, n_fun, "Door doesn't exist.");
	  return false;
	}

      value = atov (arg);

      if (!(key = get_obj_index (value)))
	{
	  olc_msg (ch, n_fun, "Key doesn't exist.");
	  return false;
	}

      if (key->item_type != ITEM_KEY)
	{
	  olc_msg (ch, n_fun, "Object is not a key.");
	  return false;
	}

      pRoom->exit[door]->key = value;

      olc_msg (ch, n_fun, "Exit key set.");
      return true;
    }

  if (!str_cmp (command, "name"))
    {
      if (NullStr (arg))
	{
	  cmd_syntax (ch, NULL, n_fun, "name [string]", "name none", NULL);
	  return false;
	}

      if (!pRoom->exit[door])
	{
	  olc_msg (ch, n_fun, "Door doesn't exist.");
	  return false;
	}

      if (str_cmp (arg, "none"))
	replace_str (&pRoom->exit[door]->keyword, arg);
      else
	replace_str (&pRoom->exit[door]->keyword, "");

      olc_msg (ch, n_fun, "Exit name set.");
      return true;
    }

  if (!str_prefix (command, "description"))
    {
      if (NullStr (arg))
	{
	  if (!pRoom->exit[door])
	    {
	      olc_msg (ch, n_fun, "Door doesn't exist.");
	      return false;
	    }

	  string_append (ch, &pRoom->exit[door]->description);
	  return true;
	}

      cmd_syntax (ch, NULL, n_fun, "desc", NULL);
      return false;
    }

  return false;
}

Olc_Fun (redit_mreset)
{
  RoomIndex *pRoom;
  CharIndex *pMobIndex;
  CharData *newmob;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  ResetData *pReset;

  EditRoom (ch, pRoom);

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);

  if (NullStr (arg) || !is_number (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum> <max #> <min #>", NULL);
      return false;
    }

  if (!(pMobIndex = get_char_index (atov (arg))))
    {
      olc_msg (ch, n_fun, "No mobile has that vnum.");
      return false;
    }

  if (pMobIndex->area != pRoom->area)
    {
      olc_msg (ch, n_fun, "No such mobile in this area.");
      return false;
    }


  pReset = new_reset ();
  pReset->command = 'M';
  pReset->arg1 = pMobIndex->vnum;
  pReset->arg2 = is_number (arg2) ? atoi (arg2) : MAX_MOB;
  pReset->arg3 = pRoom->vnum;
  pReset->arg4 = is_number (argument) ? atoi (argument) : 1;
  add_reset (pRoom, pReset, 0);


  newmob = create_mobile (pMobIndex);
  char_to_room (newmob, pRoom);

  olc_msg (ch, n_fun,
	   "%s (%ld) has been loaded and added to resets." NEWLINE
	   "There will be a maximum of %d loaded to this room.",
	   capitalize (pMobIndex->short_descr), pMobIndex->vnum,
	   pReset->arg2);
  act ("$n has created $N!", ch, NULL, newmob, TO_ROOM);
  return true;
}



wloc_t
wear_loc (flag_t bits, int count)
{
  int flag;

  for (flag = 0; wear_table[flag].wear_bit != -2; flag++)
    {
      if (IsSet (bits, wear_table[flag].wear_bit) && --count < 1)
	return (wloc_t) wear_table[flag].wear_loc;
    }

  return WEAR_NONE;
}


flag_t
wear_bit (wloc_t loc)
{
  int flag;

  for (flag = 0; wear_table[flag].wear_loc != -2; flag++)
    {
      if (loc == wear_table[flag].wear_loc)
	return wear_table[flag].wear_bit;
    }

  return 0;
}

Olc_Fun (redit_oreset)
{
  RoomIndex *pRoom;
  ObjIndex *pObjIndex;
  ObjData *newobj;
  ObjData *to_obj;
  CharData *to_mob;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int olevel = 0;

  ResetData *pReset;

  EditRoom (ch, pRoom);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || !is_number (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum> <args>",
		  "-no_args = into room", "-<obj> = into obj",
		  "-<mob> <wear_loc> = into mob", NULL);
      return false;
    }

  if (!(pObjIndex = get_obj_index (atov (arg1))))
    {
      olc_msg (ch, n_fun, "No object has that vnum.");
      return false;
    }

  if (pObjIndex->area != pRoom->area)
    {
      olc_msg (ch, n_fun, "No such object in this area.");
      return false;
    }


  if (NullStr (arg2))
    {
      pReset = new_reset ();
      pReset->command = 'O';
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = 0;
      pReset->arg3 = pRoom->vnum;
      pReset->arg4 = 0;
      add_reset (pRoom, pReset, 0);

      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      obj_to_room (newobj, pRoom);

      olc_msg (ch, n_fun,
	       "%s (%ld) has been loaded and added to resets.",
	       capitalize (pObjIndex->short_descr), pObjIndex->vnum);
    }
  else
    if (NullStr (argument) &&
	((to_obj = get_obj_list (ch, arg2, pRoom->content_first)) != NULL))
    {
      pReset = new_reset ();
      pReset->command = 'P';
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = 0;
      pReset->arg3 = to_obj->pIndexData->vnum;
      pReset->arg4 = 1;
      add_reset (pRoom, pReset, 0);

      newobj = create_object (pObjIndex, number_fuzzy (olevel));
      newobj->cost = 0;
      obj_to_obj (newobj, to_obj);

      olc_msg (ch, n_fun,
	       "%s (%ld) has been loaded into "
	       "%s (%ld) and added to resets.",
	       capitalize (newobj->short_descr),
	       newobj->pIndexData->vnum, to_obj->short_descr,
	       to_obj->pIndexData->vnum);
    }
  else if ((to_mob = get_char_room (ch, NULL, arg2)) != NULL)
    {
      wloc_t pwear_loc = WEAR_NONE;


      if ((pwear_loc = (wloc_t) flag_value (wear_loc_flags, argument)) ==
	  NO_FLAG)
	{
	  olc_msg (ch, n_fun, "Invalid wear_loc.");
	  return false;
	}


      if (!IsSet (pObjIndex->wear_flags, wear_bit (pwear_loc)))
	{
	  olc_msg (ch, n_fun, "%s (%ld) has wear flags: [%s]",
		   capitalize (pObjIndex->short_descr),
		   pObjIndex->vnum, flag_string (wear_flags,
						 pObjIndex->wear_flags));
	  return false;
	}


      if (get_eq_char (to_mob, pwear_loc))
	{
	  olc_msg (ch, n_fun, "Object already equipped.");
	  return false;
	}

      pReset = new_reset ();
      pReset->arg1 = pObjIndex->vnum;
      pReset->arg2 = pwear_loc;
      pReset->arg3 = pwear_loc;
      if (pReset->arg2 == WEAR_NONE)
	pReset->command = 'G';
      else
	pReset->command = 'E';

      add_reset (pRoom, pReset, 0);

      olevel = Range (0, to_mob->level - 2, MAX_MORTAL_LEVEL);
      newobj = create_object (pObjIndex, number_fuzzy (olevel));

      if (to_mob->pIndexData->pShop)
	{
	  switch (pObjIndex->item_type)
	    {
	    default:
	      olevel = 0;
	      break;
	    case ITEM_PILL:
	      olevel = number_range (0, 10);
	      break;
	    case ITEM_POTION:
	      olevel = number_range (0, 10);
	      break;
	    case ITEM_SCROLL:
	      olevel = number_range (5, 15);
	      break;
	    case ITEM_WAND:
	      olevel = number_range (10, 20);
	      break;
	    case ITEM_STAFF:
	      olevel = number_range (15, 25);
	      break;
	    case ITEM_ARMOR:
	      olevel = number_range (5, 15);
	      break;
	    case ITEM_WEAPON:
	      if (pReset->command == 'G')
		olevel = number_range (5, 15);
	      else
		olevel = number_fuzzy (olevel);
	      break;
	    }

	  newobj = create_object (pObjIndex, olevel);
	  if (pReset->arg2 == WEAR_NONE)
	    SetBit (newobj->extra_flags, ITEM_INVENTORY);
	}
      else
	newobj = create_object (pObjIndex, number_fuzzy (olevel));

      obj_to_char (newobj, to_mob);
      if (pReset->command == 'E')
	equip_char (to_mob, newobj, (wloc_t) pReset->arg3);

      olc_msg (ch, n_fun,
	       "%s (%ld) has been loaded "
	       "%s of %s (%ld) and added to resets.",
	       capitalize (pObjIndex->short_descr), pObjIndex->vnum,
	       flag_string (wear_loc_strings, pReset->arg3),
	       to_mob->short_descr, to_mob->pIndexData->vnum);
    }
  else
    {

      olc_msg (ch, n_fun, "That mobile isn't here.");
      return false;
    }

  act ("$n has created $p!", ch, newobj, NULL, TO_ROOM);
  return true;
}


const char *
show_obj_values (ObjIndex * obj)
{
  static char buf_new[5][MSL * 2];
  static int i;
  char *buf;

  i++;
  i %= 5;
  buf = buf_new[i];

  switch (obj->item_type)
    {
    default:
      break;

    case ITEM_LIGHT:
      if (obj->value[2] == -1 || obj->value[2] == 999)
	strcpy (buf, "[v2] Light:  Infinite[-1]");
      else
	sprintf (buf, "[v2] Light:  [%ld]", obj->value[2]);
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf (buf,
	       "[v0] Level:          [%ld]" NEWLINE
	       "[v1] Charges Total:  [%ld]" NEWLINE
	       "[v2] Charges Left:   [%ld]" NEWLINE
	       "[v3] Spell:          %s", obj->value[0],
	       obj->value[1], obj->value[2],
	       obj->value[3] !=
	       -1 ? skill_table[obj->value[3]].name : "none");
      break;

    case ITEM_PORTAL:
      sprintf (buf,
	       "[v0] Charges:        [%ld]" NEWLINE
	       "[v1] Exit Flags:     %s" NEWLINE
	       "[v2] Portal Flags:   %s" NEWLINE
	       "[v3] Goes to (vnum): [%ld]", obj->value[0],
	       flag_string (exit_flags, obj->value[1]),
	       flag_string (portal_flags, obj->value[2]), obj->value[3]);
      break;

    case ITEM_FURNITURE:
      sprintf (buf,
	       "[v0] Max people:      [%ld]" NEWLINE
	       "[v1] Max weight:      [%ld]" NEWLINE
	       "[v2] Furniture Flags: %s" NEWLINE
	       "[v3] Heal bonus:      [%ld]" NEWLINE
	       "[v4] Mana bonus:      [%ld]", obj->value[0],
	       obj->value[1], flag_string (furniture_flags,
					   obj->value[2]),
	       obj->value[3], obj->value[4]);
      break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      sprintf (buf,
	       "[v0] Level:  [%ld]" NEWLINE
	       "[v1] Spell:  %s" NEWLINE
	       "[v2] Spell:  %s" NEWLINE
	       "[v3] Spell:  %s" NEWLINE
	       "[v4] Spell:  %s", obj->value[0],
	       obj->value[1] !=
	       -1 ? skill_table[obj->value[1]].name : "none",
	       obj->value[2] !=
	       -1 ? skill_table[obj->value[2]].name : "none",
	       obj->value[3] !=
	       -1 ? skill_table[obj->value[3]].name : "none",
	       obj->value[4] !=
	       -1 ? skill_table[obj->value[4]].name : "none");
      break;



    case ITEM_ARMOR:
      sprintf (buf,
	       "[v0] Ac pierce       [%ld]" NEWLINE
	       "[v1] Ac bash         [%ld]" NEWLINE
	       "[v2] Ac slash        [%ld]" NEWLINE
	       "[v3] Ac exotic       [%ld]", obj->value[0],
	       obj->value[1], obj->value[2], obj->value[3]);
      break;




    case ITEM_WEAPON:
      sprintf (buf, "[v0] Weapon class:   %s" NEWLINE
	       "[v1] Number of dice: [%ld]" NEWLINE
	       "[v2] Type of dice:   [%ld]" NEWLINE
	       "[v3] Type:           %s" NEWLINE
	       "[v4] Special type:   %s",
	       flag_string (weapon_types, obj->value[0]),
	       obj->value[1], obj->value[2],
	       attack_table[obj->value[3]].name,
	       flag_string (weapon_flags, obj->value[4]));
      break;

    case ITEM_CONTAINER:
      sprintf (buf,
	       "[v0] Weight:     [%ld kg]" NEWLINE
	       "[v1] Flags:      [%s]" NEWLINE
	       "[v2] Key:     %s [%ld]" NEWLINE
	       "[v3] Capacity    [%ld]" NEWLINE
	       "[v4] Weight Mult [%ld]", obj->value[0],
	       flag_string (container_flags, obj->value[1]),
	       get_obj_index (obj->value[2]) ? get_obj_index (obj->value
							      [2])->
	       short_descr : "none", obj->value[2], obj->value[3],
	       obj->value[4]);
      break;

    case ITEM_DRINK_CON:
      sprintf (buf,
	       "[v0] Liquid Total: [%ld]" NEWLINE
	       "[v1] Liquid Left:  [%ld]" NEWLINE
	       "[v2] Liquid:       %s" NEWLINE
	       "[v3] Poisoned:     %s",
	       obj->value[0], obj->value[1],
	       liq_table[obj->value[2]].liq_name,
	       obj->value[3] != 0 ? "Yes" : "No");
      break;

    case ITEM_FOUNTAIN:
      sprintf (buf,
	       "[v0] Liquid Total: [%ld]" NEWLINE
	       "[v1] Liquid Left:  [%ld]" NEWLINE
	       "[v2] Liquid:    	    %s", obj->value[0],
	       obj->value[1], liq_table[obj->value[2]].liq_name);
      break;

    case ITEM_FOOD:
      sprintf (buf,
	       "[v0] Food hours: [%ld]" NEWLINE
	       "[v1] Full hours: [%ld]" NEWLINE
	       "[v3] Poisoned:   %s",
	       obj->value[0], obj->value[1],
	       obj->value[3] != 0 ? "Yes" : "No");
      break;

    case ITEM_MONEY:
      sprintf (buf, "[v0] Gold:   [%ld]", obj->value[0]);
      break;
    }

  return buf;
}

bool
set_obj_values (CharData * ch, ObjIndex * pObj, int value_num,
		const char *argument)
{
  switch (pObj->item_type)
    {
    default:
      break;

    case ITEM_LIGHT:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_LIGHT");
	  return false;
	case 2:
	  chprintln (ch, "HOURS OF LIGHT SET.");
	  pObj->value[2] = atol (argument);
	  break;
	}
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_STAFF_WAND");
	  return false;
	case 0:
	  chprintln (ch, "SPELL LEVEL SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "TOTAL NUMBER OF CHARGES SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "CURRENT NUMBER OF CHARGES SET.");
	  pObj->value[2] = atol (argument);
	  break;
	case 3:
	  chprintln (ch, "SPELL TYPE SET.");
	  pObj->value[3] = skill_lookup (argument);
	  break;
	}
      break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_SCROLL_POTION_PILL");
	  return false;
	case 0:
	  chprintln (ch, "SPELL LEVEL SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "SPELL TYPE 1 SET.");
	  pObj->value[1] = skill_lookup (argument);
	  break;
	case 2:
	  chprintln (ch, "SPELL TYPE 2 SET.");
	  pObj->value[2] = skill_lookup (argument);
	  break;
	case 3:
	  chprintln (ch, "SPELL TYPE 3 SET.");
	  pObj->value[3] = skill_lookup (argument);
	  break;
	case 4:
	  chprintln (ch, "SPELL TYPE 4 SET.");
	  pObj->value[4] = skill_lookup (argument);
	  break;
	}
      break;



    case ITEM_ARMOR:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_ARMOR");
	  return false;
	case 0:
	  chprintln (ch, "AC PIERCE SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "AC BASH SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "AC SLASH SET.");
	  pObj->value[2] = atol (argument);
	  break;
	case 3:
	  chprintln (ch, "AC EXOTIC SET.");
	  pObj->value[3] = atol (argument);
	  break;
	}
      break;



    case ITEM_WEAPON:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_WEAPON");
	  return false;
	case 0:
	  chprintln (ch, "WEAPON CLASS SET.");
	  ALT_FLAGVALUE_SET (pObj->value[0], weapon_types, argument);
	  break;
	case 1:
	  chprintln (ch, "NUMBER OF DICE SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "TYPE OF DICE SET.");
	  pObj->value[2] = atol (argument);
	  break;
	case 3:
	  chprintln (ch, "WEAPON TYPE SET.");
	  pObj->value[3] = attack_lookup (argument);
	  break;
	case 4:
	  chprintln (ch, "SPECIAL WEAPON TYPE TOGGLED.");
	  ALT_FLAGVALUE_TOGGLE (pObj->value[4], weapon_flags, argument);
	  break;
	}
      break;

    case ITEM_PORTAL:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_PORTAL");
	  return false;

	case 0:
	  chprintln (ch, "CHARGES SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "EXIT FLAGS SET.");
	  ALT_FLAGVALUE_SET (pObj->value[1], exit_flags, argument);
	  break;
	case 2:
	  chprintln (ch, "PORTAL FLAGS SET.");
	  ALT_FLAGVALUE_SET (pObj->value[2], portal_flags, argument);
	  break;
	case 3:
	  chprintln (ch, "EXIT vnum_t SET.");
	  pObj->value[3] = atol (argument);
	  break;
	}
      break;

    case ITEM_FURNITURE:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_FURNITURE");
	  return false;

	case 0:
	  chprintln (ch, "NUMBER OF PEOPLE SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "MAX WEIGHT SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "FURNITURE FLAGS TOGGLED.");
	  ALT_FLAGVALUE_TOGGLE (pObj->value[2], furniture_flags, argument);
	  break;
	case 3:
	  chprintln (ch, "HEAL BONUS SET.");
	  pObj->value[3] = atol (argument);
	  break;
	case 4:
	  chprintln (ch, "MANA BONUS SET.");
	  pObj->value[4] = atol (argument);
	  break;
	}
      break;

    case ITEM_CONTAINER:
      switch (value_num)
	{
	  flag_t value;

	default:
	  do_function (ch, &do_oldhelp, "ITEM_CONTAINER");
	  return false;
	case 0:
	  chprintln (ch, "WEIGHT CAPACITY SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  if ((value = flag_value (container_flags, argument)) != NO_FLAG)
	    ToggleBit (pObj->value[1], value);
	  else
	    {
	      do_function (ch, &do_oldhelp, "ITEM_CONTAINER");
	      return false;
	    }
	  chprintln (ch, "CONTAINER TYPE SET.");
	  break;
	case 2:
	  if (atol (argument) != 0)
	    {
	      if (!get_obj_index (atov (argument)))
		{
		  chprintln (ch, "THERE IS NO SUCH ITEM.");
		  return false;
		}

	      if (get_obj_index (atov (argument))->item_type != ITEM_KEY)
		{
		  chprintln (ch, "THAT ITEM IS NOT A Key.");
		  return false;
		}
	    }
	  chprintln (ch, "CONTAINER Key SET.");
	  pObj->value[2] = atol (argument);
	  break;
	case 3:
	  chprintln (ch, "CONTAINER MAX WEIGHT SET.");
	  pObj->value[3] = atol (argument);
	  break;
	case 4:
	  chprintln (ch, "WEIGHT MULTIPLIER SET.");
	  pObj->value[4] = atol (argument);
	  break;
	}
      break;

    case ITEM_DRINK_CON:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_DRINK");

	  return false;
	case 0:
	  chprintln (ch, "MAXIMUM AMOUT OF LIQUID HOURS SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "CURRENT AMOUNT OF LIQUID HOURS SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "LIQUID TYPE SET.");
	  pObj->value[2] =
	    (liq_lookup (argument) != -1 ? liq_lookup (argument) : 0);
	  break;
	case 3:
	  chprintln (ch, "POISON VALUE TOGGLED.");
	  pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
	  break;
	}
      break;

    case ITEM_FOUNTAIN:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_FOUNTAIN");

	  return false;
	case 0:
	  chprintln (ch, "MAXIMUM AMOUT OF LIQUID HOURS SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "CURRENT AMOUNT OF LIQUID HOURS SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 2:
	  chprintln (ch, "LIQUID TYPE SET.");
	  pObj->value[2] =
	    (liq_lookup (argument) != -1 ? liq_lookup (argument) : 0);
	  break;
	}
      break;

    case ITEM_FOOD:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_FOOD");
	  return false;
	case 0:
	  chprintln (ch, "HOURS OF FOOD SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "HOURS OF FULL SET.");
	  pObj->value[1] = atol (argument);
	  break;
	case 3:
	  chprintln (ch, "POISON VALUE TOGGLED.");
	  pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
	  break;
	}
      break;

    case ITEM_MONEY:
      switch (value_num)
	{
	default:
	  do_function (ch, &do_oldhelp, "ITEM_MONEY");
	  return false;
	case 0:
	  chprintln (ch, "GOLD AMOUNT SET.");
	  pObj->value[0] = atol (argument);
	  break;
	case 1:
	  chprintln (ch, "SILVER AMOUNT SET.");
	  pObj->value[1] = atol (argument);
	  break;
	}
      break;
    }

  chprintln (ch, show_obj_values (pObj));

  return true;
}

bool
set_value (CharData * ch, ObjIndex * pObj, const char *argument, int value)
{
  if (NullStr (argument))
    {
      set_obj_values (ch, pObj, -1, "");
      return false;
    }

  if (set_obj_values (ch, pObj, value, argument))
    return true;

  return false;
}


bool
oedit_values (CharData * ch, const char *argument, int value)
{
  ObjIndex *pObj;

  GetEdit (ch, ObjIndex, pObj);

  if (set_value (ch, pObj, argument, value))
    return true;

  return false;
}



void
show_liqlist (CharData * ch)
{
  int liq;
  Buffer *buffer;

  buffer = new_buf ();

  for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
      if ((liq % 21) == 0)
	bprintln (buffer,
		  "Name                 Color          Proof Full Thirst Food Ssize");

      bprintlnf (buffer, "%-20s %-14s %5d %4d %6d %4d %5d",
		 liq_table[liq].liq_name, liq_table[liq].liq_color,
		 liq_table[liq].liq_affect[0],
		 liq_table[liq].liq_affect[1],
		 liq_table[liq].liq_affect[2],
		 liq_table[liq].liq_affect[3], liq_table[liq].liq_affect[4]);
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);

  return;
}

void
show_damlist (CharData * ch)
{
  int att;
  Buffer *buffer;

  buffer = new_buf ();

  for (att = 0; attack_table[att].name != NULL; att++)
    {
      if ((att % 21) == 0)
	bprintln (buffer, "Name                 Noun");

      bprintlnf (buffer, "%-20s %-20s", attack_table[att].name,
		 attack_table[att].noun);
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);

  return;
}

Olc_Fun (medit_group)
{
  CharIndex *pMob;
  CharIndex *pMTemp;
  char arg[MAX_STRING_LENGTH];
  int temp;
  Buffer *buffer;
  bool found = false;

  GetEdit (ch, CharIndex, pMob);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<number>", "show <number>", NULL);
      return false;
    }

  if (is_number (argument))
    {
      pMob->group = atov (argument);
      olc_msg (ch, n_fun, "Group set.");
      return true;
    }

  argument = one_argument (argument, arg);

  if (!str_cmp (arg, "show") && is_number (argument))
    {
      if (atov (argument) == 0)
	{
	  olc_msg (ch, n_fun, "Are you crazy?");
	  return false;
	}

      buffer = new_buf ();

      for (temp = 0; temp < MAX_KEY_HASH; temp++)
	{
	  for (pMTemp = char_index_hash[temp]; pMTemp; pMTemp = pMTemp->next)
	    if (pMTemp && (pMTemp->group == atov (argument)))
	      {
		found = true;
		bprintlnf (buffer, "[%5ld] %s", pMTemp->vnum,
			   pMTemp->player_name);
	      }
	}

      if (found)
	sendpage (ch, buf_string (buffer));
      else
	olc_msg (ch, n_fun, "No mobs in that group.");

      free_buf (buffer);
      return false;
    }

  return false;
}


Olc_Fun (cledit_skill)
{
  int sn;
  ClassData *pClass;
  int Class;
  int value;
  bool pLevel;
  char arg1[MIL], arg2[MIL], arg3[MIL];

  GetEdit (ch, ClassData, pClass);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (NullStr (arg1) || NullStr (arg2) || NullStr (arg3))
    {
      cmd_syntax (ch, NULL, n_fun, "<skill> rating|level <value>", NULL);
      return false;
    }

  if ((sn = skill_lookup (arg1)) < 0 || sn >= top_skill)
    {
      olc_msg (ch, n_fun, "Thats not a valid skill.");
      return false;
    }

  if (!str_cmp (arg2, "rating"))
    pLevel = false;
  else if (!str_cmp (arg2, "level"))
    pLevel = true;
  else
    {
      olc_msg (ch, n_fun,
	       "Please choose either a skill 'rating' or 'level'.");
      return false;
    }

  value = atoi (arg3);
  if (pLevel && (value < 1 || value > MAX_LEVEL))
    {
      olc_msg (ch, n_fun, "Please choose a level between 1 and %d.",
	       MAX_LEVEL);
      return false;
    }

  Class = class_lookup (pClass->name[0]);

  if (pLevel)
    {
      skill_table[sn].skill_level[Class] = value;
      olc_msg (ch, n_fun, "Class skill level set.");
    }
  else
    {
      skill_table[sn].rating[Class] = value;
      olc_msg (ch, n_fun, "Class rating set.");
    }
  return true;
}

Olc_Fun (cledit_index)
{
  ClassData *pClass;
  Column Cd;
  int i;

  GetEdit (ch, ClassData, pClass);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<index name>", NULL);
      return false;
    }

  if (is_name (argument, "clear reset none"))
    {
      pClass->index = &class_null;
      olc_msg (ch, n_fun, "Index cleared.");
      return true;
    }

  for (i = 0; class_index[i].name != NULL; i++)
    {
      if (!str_cmp (argument, class_index[i].name))
	{
	  if (class_index[i].index != &class_null
	      && (!class_index[i].index || !*(int *) class_index[i].index))
	    {
	      int tmp;

	      for (tmp = 0; tmp < top_class; tmp++)
		if (&class_table[tmp] == pClass)
		  break;
	      *(int *) class_index[i].index = tmp;
	    }
	  pClass->index = (int *) class_index[i].index;
	  olc_msg (ch, n_fun, "%s now uses global skill pointer '%s'.",
		   pClass->name, class_index[i].name);
	  return true;
	}
    }

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  for (i = 0; class_index[i].name != NULL; i++)
    print_cols (&Cd, class_index[i].name);
  cols_nl (&Cd);
  olc_msg (ch, n_fun, "That class hasn't been coded in yet.");
  return false;
}


Olc_Fun (cmdedit_dofun)
{
  CmdData *pCmd;
  Do_F *fun;

  GetEdit (ch, CmdData, pCmd);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<do_function>", NULL);
      return false;
    }

  if (is_name (argument, "none clear reset"))
    {
      pCmd->do_fun = do_null;
      olc_msg (ch, n_fun, "Function entry reset.");
      return true;
    }

  if ((fun = (Do_F *) index_lookup (argument, dofun_index, NULL)) == NULL)
    {
      olc_msg (ch, n_fun,
	       "That is not a valid function (probably not coded in yet)");
      return false;
    }

  pCmd->do_fun = fun;
  olc_msg (ch, n_fun, "%s now points to the %s function.", pCmd->name,
	   argument);
  return true;
}

Olc_Fun (cmdedit_rearrange)
{
  CmdData *pCmd, *iCmd, *tmp;
  bool found = false;

  GetEdit (ch, CmdData, pCmd);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<cmd to place before>", NULL);
      return false;
    }

  if ((iCmd = cmd_lookup (argument)) == NULL)
    {
      olc_msg (ch, n_fun, "That is not a command.");
      return false;
    }

  if (iCmd == pCmd)
    {
      olc_msg (ch, n_fun, "Can't rearrange the same command.");
      return false;
    }

  for (tmp = cmd_first; tmp; tmp = tmp->next)
    {
      if (tmp == pCmd)
	{
	  found = true;
	  continue;
	}
      if (tmp == iCmd)
	{
	  if (found)
	    {
	      olc_msg (ch, n_fun,
		       "Error: command %s is already above %s.",
		       pCmd->name, iCmd->name);
	      return false;
	    }
	  else
	    break;
	}
    }

  UnLink (pCmd, cmd, next, prev);
  Insert (pCmd, iCmd, cmd, next, prev);

  for (tmp = cmd_first; tmp; tmp = tmp->next)
    unhash_command (tmp);
  for (tmp = cmd_first; tmp; tmp = tmp->next)
    hash_command (tmp);

  olc_msg (ch, n_fun, "Command moved.");

  return true;
}

Olc_Fun (cmdedit_name)
{
  bool relocate;
  CmdData *pCmd;
  char arg1[MIL];

  GetEdit (ch, CmdData, pCmd);

  one_argument (argument, arg1);
  if (NullStr (arg1))
    {
      chprintln (ch, "Cannot clear name field!");
      return false;
    }
  if (arg1[0] != pCmd->name[0])
    {
      unhash_command (pCmd);
      relocate = true;
    }
  else
    relocate = false;

  replace_str (&pCmd->name, arg1);
  if (relocate)
    hash_command (pCmd);
  chprintln (ch, "Name set.");
  return true;
}


Olc_Fun (gredit_ratings)
{
  GroupData *pGroup;
  int pClass, value;
  char arg1[MIL], arg2[MIL];

  GetEdit (ch, GroupData, pGroup);

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<class> <value>", NULL);
      return false;
    }

  if ((pClass = class_lookup (arg1)) == -1)
    {
      olc_msg (ch, n_fun, "That is not a valid class.");
      return false;
    }

  if (!is_number (arg2))
    {
      olc_msg (ch, n_fun, "That is not a valid value.");
      return false;
    }
  else
    value = atoi (arg2);

  pGroup->rating[pClass] = value;
  olc_msg (ch, n_fun, "%s's rating for %s changed to %d.", pGroup->name,
	   class_table[pClass].name[0], value);
  return true;
}

Olc_Fun (cledit_name)
{
  ClassData *pClass;
  int i, j;
  char arg[MIL], name[MIL];

  GetEdit (ch, ClassData, pClass);

  argument = one_argument (argument, arg);
  one_argument (argument, name);

  if (NullStr (arg) || NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<#remort> <name>", NULL);
      return false;
    }

  if ((i = atoi (arg)) < 1 || i > MAX_REMORT)
    {
      olc_msg (ch, n_fun, "Remort number must be between 1 and %d.",
	       MAX_REMORT);
      return false;
    }
  i--;

  if (strlen (name) > 25)
    {
      olc_msg (ch, n_fun, "Name too long.");
      return false;
    }

  if ((j = class_lookup (name)) != -1)
    {
      olc_msg (ch, n_fun, "A class with that name exists.");
      return false;
    }

  replace_str (&pClass->name[i], Upper (name));
  olc_msg (ch, n_fun, "Name set.");
  return true;
}

Olc_Fun (gredit_spells)
{
  GroupData *pGroup;
  int i;
  bool found = false;

  GetEdit (ch, GroupData, pGroup);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<spell-name>", NULL);
      return false;
    }

  for (i = 0; pGroup->spells[i] != NULL; i++)
    {
      if (found)
	{
	  pGroup->spells[i - 1] = pGroup->spells[i];
	  pGroup->spells[i] = NULL;
	  continue;
	}
      if (!str_cmp (argument, pGroup->spells[i]))
	{
	  olc_msg (ch, n_fun, "%s spell removed.", pGroup->spells[i]);
	  free_string (pGroup->spells[i]);
	  pGroup->spells[i] = NULL;
	  found = true;
	}
    }

  if (found)
    return true;

  if (i >= MAX_IN_GROUP)
    {
      olc_msg (ch, n_fun, "Too many spells already in group.");
      return false;
    }

  if (skill_lookup (argument) == -1)
    chprintln (ch, "WARNING: that skill does not exist.");

  replace_str (&pGroup->spells[i], argument);
  olc_msg (ch, n_fun, "%s skill/spell added to %s.", argument, pGroup->name);

  return true;
}


Olc_Fun (raedit_classx)
{
  char clas[MSL];
  char value[MSL];
  RaceData *pRace;

  GetEdit (ch, RaceData, pRace);

  if (!pRace->pc_race)
    {
      olc_msg (ch, n_fun, "Race must be a PC race first.");
      return false;
    }

  if (!NullStr (argument))
    {

      argument = one_argument (argument, clas);
      argument = one_argument (argument, value);

      if (class_lookup (clas) < 0)
	{
	  olc_msg (ch, n_fun, "That class does not exist.");
	  return false;
	}

      if (!is_number (value))
	{
	  olc_msg (ch, n_fun, "Second argument must be numeric.");
	  return false;
	}

      if (atoi (value) < 75)
	{
	  olc_msg (ch, n_fun,
		   "{RWARNING!{x Class modifier should be at least 75.");
	}
      pRace->class_mult[class_lookup (clas)] = atoi (value);
      olc_msg (ch, n_fun, "Race modifier for class %s is set to %d.",
	       clas, atoi (value));
      return true;
    }
  cmd_syntax (ch, NULL, n_fun, "<class> <value>", NULL);
  return false;
}

Olc_Fun (raedit_skills)
{
  int x;
  RaceData *pRace;
  bool found = false;

  GetEdit (ch, RaceData, pRace);

  if (!pRace->pc_race)
    {
      olc_msg (ch, n_fun, "Race must be a PC race first.");
      return false;
    }

  if (skill_lookup (argument) != -1)
    {
      for (x = 0; x < MAX_RACE_SKILL; x++)
	{
	  if (found)
	    {
	      replace_str (&pRace->skills[x - 1], pRace->skills[x]);
	      continue;
	    }
	  if (!str_cmp (pRace->skills[x], argument))
	    {
	      free_string (pRace->skills[x]);
	      pRace->skills[x] = NULL;
	      olc_msg (ch, n_fun, "Skill %s deleted.", argument);
	      found = true;
	      continue;
	    }
	  if (pRace->skills[x] == NULL)
	    {
	      replace_str (&pRace->skills[x], argument);
	      olc_msg (ch, n_fun, "Skill %s set.", argument);
	      return true;
	    }
	}

      if (found)
	return true;

      if (x == MAX_RACE_SKILL)
	{
	  chprintln (ch, "No available spaces for skill.");
	  return false;
	}
    }
  cmd_syntax (ch, NULL, n_fun, "<skill>", NULL);
  return false;

}

Olc_Fun (raedit_name)
{
  RaceData *pRace;
  CharIndex *pMob;
  int ihash;
  char arg[MIL];

  GetEdit (ch, RaceData, pRace);

  first_arg (argument, arg, false);

  if (NullStr (arg))
    {
      olc_msg (ch, n_fun, "Change name to what?");
      return false;
    }

  if (race_lookup (arg) != NULL)
    {
      olc_msg (ch, n_fun, "A race with that name already exists.");
      return false;
    }

  if (str_cmp (arg, pRace->name))
    {
      for (ihash = 0; ihash < MAX_KEY_HASH; ihash++)
	{
	  for (pMob = char_index_hash[ihash]; pMob; pMob = pMob->next)
	    {
	      if (pMob->race == pRace)
		SetBit (pMob->area->area_flags, AREA_CHANGED);
	    }
	}
    }
  replace_str (&pRace->name, arg);
  olc_msg (ch, n_fun, "Name set.");
  return true;
}


Olc_Fun (skedit_levels)
{
  SkillData *pSkill;
  char arg1[MIL], arg2[MIL];
  int pClass;
  int lev;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  GetEdit (ch, SkillData, pSkill);

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<class> <level>", NULL);
      return false;
    }

  if ((pClass = class_lookup (arg1)) == -1)
    {
      olc_msg (ch, n_fun, "That is not a valid class.");
      return false;
    }

  if (!is_number (arg2) || (lev = atoi (arg2)) < 0 || lev > MAX_LEVEL)
    {
      olc_msg (ch, n_fun, "That is not a valid level.");
      return false;
    }

  pSkill->skill_level[pClass] = lev;
  olc_msg (ch, n_fun, "Skill level for %s set to %d.",
	   class_table[pClass].name[0], lev);
  return true;
}

Olc_Fun (skedit_ratings)
{
  SkillData *pSkill;
  int pClass, lev;
  char arg1[MIL], arg2[MIL];

  GetEdit (ch, SkillData, pSkill);

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<class> <rating>", NULL);
      return false;
    }

  if ((pClass = class_lookup (arg1)) == -1)
    {
      olc_msg (ch, n_fun, "That is not a valid class.");
      return false;
    }

  if (!is_number (arg2))
    {
      olc_msg (ch, n_fun, "That is not a valid rating.");
      return false;
    }
  else
    lev = atoi (arg2);

  pSkill->rating[pClass] = lev;
  olc_msg (ch, n_fun, "Skill rating for %s set to %d.",
	   class_table[pClass].name[0], lev);
  return true;
}

Olc_Fun (skedit_spellfun)
{
  SkillData *pSkill;
  int i;

  GetEdit (ch, SkillData, pSkill);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<spell_function>", NULL);
      return false;
    }

  if (is_name (argument, "none clear reset"))
    {
      pSkill->spell_fun = spell_null;
      olc_msg (ch, n_fun, "Spell value reset.");
      return true;
    }

  for (i = 0; spell_index[i].name != NULL; i++)
    {
      if (!str_cmp (argument, spell_index[i].name))
	{
	  pSkill->spell_fun = (Spell_F *) spell_index[i].index;
	  olc_msg (ch, n_fun, "%s now uses spell function '%s'.",
		   pSkill->name, spell_index[i].name);
	  return true;
	}
    }

  olc_msg (ch, n_fun, "That spell function hasn't been coded into the mud.");
  return false;
}

Olc_Fun (skedit_gsn)
{
  SkillData *pSkill;
  Column Cd;
  int i;

  GetEdit (ch, SkillData, pSkill);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<gsn-name>", NULL);
      return false;
    }

  if (is_name (argument, "clear reset none"))
    {
      pSkill->pgsn = &gsn_null;
      olc_msg (ch, n_fun, "GSN Entry cleared.");
      return true;
    }

  for (i = 0; gsn_index[i].name != NULL; i++)
    {
      if (!str_cmp (argument, gsn_index[i].name))
	{
	  if (gsn_index[i].index != &gsn_null
	      && (!gsn_index[i].index || !*(int *) gsn_index[i].index))
	    {
	      int tmp;

	      for (tmp = 0; tmp < top_skill; tmp++)
		if (&skill_table[tmp] == pSkill)
		  break;
	      *(int *) gsn_index[i].index = tmp;
	    }
	  pSkill->pgsn = (int *) gsn_index[i].index;
	  olc_msg (ch, n_fun, "%s now uses global skill pointer '%s'.",
		   pSkill->name, gsn_index[i].name);
	  return true;
	}
    }

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  for (i = 0; gsn_index[i].name != NULL; i++)
    print_cols (&Cd, gsn_index[i].name);
  cols_nl (&Cd);
  olc_msg (ch, n_fun, "That GSN hasn't been coded in yet.");
  return false;
}


Olc_Fun (sedit_name)
{
  bool relocate;
  SocialData *psocial;
  char arg1[MIL];

  GetEdit (ch, SocialData, psocial);

  one_argument (argument, arg1);
  if (NullStr (arg1))
    {
      olc_msg (ch, n_fun, "Cannot clear name field!");
      return false;
    }
  if (arg1[0] != psocial->name[0])
    {
      unhash_social (psocial);
      relocate = true;
    }
  else
    relocate = false;

  replace_str (&psocial->name, arg1);
  if (relocate)
    hash_social (psocial);
  olc_msg (ch, n_fun, "Name set.");
  return true;
}


Olc_Fun (songedit_lyrics)
{
  char arg[MIL];
  SongData *pSong;

  GetEdit (ch, SongData, pSong);

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "+ <string>", "-", NULL);
      return false;
    }

  if (!str_cmp (arg, "+"))
    {
      if (NullStr (argument))
	{
	  olc_msg (ch, n_fun, "No Blank Lines.");
	  return false;
	}
      if (pSong->lines >= MAX_LINES)
	{
	  olc_msg (ch, n_fun, "I'm afraid you can't add any more lines.");
	  return false;
	}
      replace_str (&pSong->lyrics[pSong->lines], argument);
      pSong->lines += 1;
      olc_msg (ch, n_fun, "Line added.");
      return true;
    }
  if (!str_cmp (arg, "-"))
    {
      pSong->lines -= 1;
      if (pSong->lines < 0)
	{
	  pSong->lines = 0;
	  olc_msg (ch, n_fun, "Lyrics Cleared.");
	  return false;
	}
      replace_str (&pSong->lyrics[pSong->lines], " ");
      olc_msg (ch, n_fun, "Line deleted.");
      return true;
    }
  cmd_syntax (ch, NULL, n_fun, "+ <string>", "-", NULL);
  return false;
}

Olc_Fun (cedit_rank)
{
  ClanData *pClan;
  char str1[MIL];

  GetEdit (ch, ClanData, pClan);

  argument = one_argument (argument, str1);

  if (is_number (str1) && atoi (str1) <= MAX_RANK && atoi (str1) > 0)
    {
      int value;

      value = atoi (str1) - 1;

      if (!NullStr (argument))
	{
	  replace_str (&pClan->rank[value].rankname, argument);
	  olc_msg (ch, n_fun, "Rank name changed.");
	  return true;
	}

    }
  cmd_syntax (ch, NULL, n_fun, "<#> <rank-name>", NULL);
  return false;
}

Olc_Fun (oedit_autoweapon)
{
  ObjIndex *pObj;

  GetEdit (ch, ObjIndex, pObj);

  if (pObj->item_type != ITEM_WEAPON)
    {
      olc_msg (ch, n_fun, "Only works on weapons...");
      return false;
    }

  if (pObj->level < 1)
    {
      olc_msg (ch, n_fun,
	       "Requires a level to be set on the weapon first(vnum %ld).",
	       pObj->vnum);
      return false;
    }

  if (IsObjStat (pObj, ITEM_QUEST))
    {
      olc_msg (ch, n_fun, "This weapon cannot be autoset.");
      return false;
    }

  autoweapon (pObj);
  olc_msg (ch, n_fun, "Weapon set to default values, check for accuracy.");

  return true;
}

Olc_Fun (oedit_autoarmor)
{
  ObjIndex *pObj;

  GetEdit (ch, ObjIndex, pObj);

  if (pObj->item_type != ITEM_ARMOR)
    {
      olc_msg (ch, n_fun, "Item is not armor.");
      return false;
    }

  if (pObj->level < 1)
    {
      olc_msg (ch, n_fun,
	       "A level must be set on the armor first. (vnum %ld)",
	       pObj->vnum);
      return false;
    }

  if (IsObjStat (pObj, ITEM_QUEST))
    {
      olc_msg (ch, n_fun, "This weapon cannot be autoset.");
      return false;
    }

  autoarmor (pObj);
  olc_msg (ch, n_fun, "Experimental values for AC set.");
  return true;
}

Olc_Fun (medit_autoset)
{
  CharIndex *pMob;

  GetEdit (ch, CharIndex, pMob);

  if (pMob->level < 1)
    {
      olc_msg (ch, n_fun, "Set a level on the mob first!!!(vnum %ld)",
	       pMob->vnum);
      return false;
    }

  autoset (pMob);
  olc_msg (ch, n_fun, "Values set, check for accuracy.");
  return true;
}

Olc_Fun (medit_autohard)
{
  CharIndex *pMob;

  GetEdit (ch, CharIndex, pMob);

  if (pMob->level < 1)
    {
      olc_msg (ch, n_fun, "Set a level on the mob first!!!(vnum %ld)",
	       pMob->vnum);
      return false;
    }

  autohard (pMob);

  olc_msg (ch, n_fun, "Hard values set, check for accuracy.");

  return true;
}

Olc_Fun (medit_autoeasy)
{
  CharIndex *pMob;

  GetEdit (ch, CharIndex, pMob);
  if (pMob->level < 1)
    {
      olc_msg (ch, n_fun, "Set a level on the mob first!!!(vnum %ld)",
	       pMob->vnum);
      return false;
    }
  autoeasy (pMob);

  olc_msg (ch, n_fun, "Easy values set, check for accuracy.");

  return true;
}

Olc_Fun (cmdedit_level)
{
  CmdData *pCmd;
  int lvl;

  GetEdit (ch, CmdData, pCmd);

  if (NullStr (argument))
    {
      cmd_syntax (NULL, n_fun, "<level>", NULL);
      return false;
    }

  if ((lvl = atoi (argument)) < 0 || lvl > MAX_LEVEL)
    {
      olc_msg (ch, n_fun, "Please keep level between 0 and %d.", MAX_LEVEL);
      return false;
    }

  if (pCmd->level < LEVEL_IMMORTAL && lvl >= LEVEL_IMMORTAL)
    pCmd->category = CMDCAT_IMMORTAL;
  else if (pCmd->level >= LEVEL_IMMORTAL && lvl < LEVEL_IMMORTAL)
    pCmd->category = CMDCAT_NONE;

  pCmd->level = lvl;
  olc_msg (ch, n_fun, "Level set.");
  return true;
}

Olc_Fun (hedit_addkeywd)
{
  HelpData *pHelp;
  char buf[MSL];

  GetEdit (ch, HelpData, pHelp);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<keyword>", NULL);
      return false;
    }

  if (is_exact_name (argument, pHelp->keyword))
    {
      olc_msg (ch, n_fun, "Keyword already exists.");
      return false;
    }

  sprintf (buf, "%s %s", pHelp->keyword, strupper (argument));
  replace_str (&pHelp->keyword, buf);
  olc_msg (ch, n_fun, "Keyword added.");
  return true;
}

Olc_Fun (hedit_copy)
{
  HelpData *pHelp, *oHelp;

  GetEdit (ch, HelpData, pHelp);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<keywords>", NULL);
      return false;
    }

  if ((oHelp = help_lookup (argument)) == NULL)
    {
      olc_msg (ch, n_fun, "No such help!");
      return false;
    }

  replace_str (&pHelp->keyword, oHelp->keyword);
  replace_str (&pHelp->text, oHelp->text);
  pHelp->level = oHelp->level;
  pHelp->category = oHelp->category;

  olc_msg (ch, n_fun, "Help info copied.");
  return true;
}

Olc_Fun (redit_copy)
{
  RoomIndex *pRoom, *oRoom;
  ExDescrData *ed;
  vnum_t vn;

  EditRoom (ch, pRoom);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return false;
    }

  vn = atov (argument);
  if ((oRoom = get_room_index (vn)) == NULL)
    {
      olc_msg (ch, n_fun, "No such room!");
      return false;
    }

  replace_str (&pRoom->name, oRoom->name);
  replace_str (&pRoom->description, oRoom->description);
  replace_str (&pRoom->owner, oRoom->owner);
  pRoom->room_flags = oRoom->room_flags;
  pRoom->sector_type = oRoom->sector_type;
  pRoom->guild = oRoom->guild;
  pRoom->heal_rate = oRoom->heal_rate;
  pRoom->mana_rate = oRoom->mana_rate;

  for (ed = oRoom->ed_first; ed; ed = ed->next)
    {
      ExDescrData *nEd;

      nEd = new_ed ();
      nEd->keyword = str_dup (ed->keyword);
      nEd->description = str_dup (ed->description);
      Link (nEd, pRoom->ed, next, prev);
    }

  return true;
}

Olc_Fun (medit_copy)
{
  return false;
}

Olc_Fun (oedit_copy)
{
  return false;
}

Olc_Fun (mpedit_copy)
{
  return false;
}

Olc_Fun (opedit_copy)
{
  return false;
}

Olc_Fun (rpedit_copy)
{
  return false;
}

Olc_Fun (raedit_copy)
{
  return false;
}

Olc_Fun (aedit_copy)
{
  return false;
}

Olc_Fun (redit_snake)
{
  RoomIndex *in_room, *tracer;
  char holder[MAX_DIR], *ptr;
  int count = 0;

  in_room = ch->in_room;
  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<directions>", NULL);
      chprintlnf (ch, NEWLINE "Example: %s 2n2e 2s2w", n_fun);
      chprintln (ch,
		 "This will create a path of rooms 2 north and then 2 east, then create"
		 NEWLINE "a second path 2 south and 2 west.");
      return false;
    }
  tracer = ch->in_room;
  ptr = holder;
  for (; *argument;)
    {
      char_from_room (ch);
      char_to_room (ch, tracer);
      switch (*argument)
	{
	default:
	  if (*argument < '0' || *argument > '9')
	    {
	      olc_msg (ch, n_fun, "Error in syntax.");
	      return false;
	    }
	  while (*argument >= '0' && *argument <= '9')
	    {
	      *ptr = *argument;
	      *(ptr + 1) = 0;
	      ++argument;
	    }
	  count = atoi (holder);
	  break;

	case ' ':
	  tracer = in_room;
	  break;
	case 'n':
	case 'N':
	  change_exit (n_fun, ch, "dig", DIR_NORTH);
	  tracer = ch->in_room->exit[0]->u1.to_room;
	  break;
	case 'e':
	case 'E':
	  change_exit (n_fun, ch, "dig", DIR_EAST);
	  tracer = ch->in_room->exit[1]->u1.to_room;
	  break;
	case 's':
	case 'S':
	  change_exit (n_fun, ch, "dig", DIR_SOUTH);
	  tracer = ch->in_room->exit[2]->u1.to_room;
	  break;
	case 'w':
	case 'W':
	  change_exit (n_fun, ch, "dig", DIR_WEST);
	  tracer = ch->in_room->exit[3]->u1.to_room;
	  break;
	case 'u':
	case 'U':
	  change_exit (n_fun, ch, "dig", DIR_UP);
	  tracer = ch->in_room->exit[4]->u1.to_room;
	  break;
	case 'd':
	case 'D':
	  change_exit (n_fun, ch, "dig", DIR_DOWN);
	  tracer = ch->in_room->exit[5]->u1.to_room;
	  break;
	}
      if (count == 0)
	++argument;
      else
	--count;
    }
  char_from_room (ch);
  char_to_room (ch, in_room);
  return true;
}

#include "olc_delete.h"
#include "olc_create.h"
#include "olc_list.h"
#include "olc_validate.h"
#include "olc_ed.h"
