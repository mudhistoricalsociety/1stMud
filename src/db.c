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


#define    	IN_DB_C 1
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "interp.h"
#include "data_table.h"
#include "vnums.h"
#include "special.h"
#undef IN_DB_C


AreaData *current_area;


FileData *fpArea;
char strArea[MAX_INPUT_LENGTH];


Proto (void init_mm, (void));

Declare_Boot_F (load_old_area);
Declare_Boot_F (load_area);
Declare_Boot_F (load_old_helps);
Declare_Boot_F (load_old_mob);
Declare_Boot_F (load_old_obj);
Declare_Boot_F (load_resets);
Declare_Boot_F (load_rooms);
Declare_Boot_F (load_shops);
Declare_Boot_F (load_specials);
Declare_Boot_F (load_mobprogs);
Declare_Boot_F (load_objprogs);
Declare_Boot_F (load_roomprogs);
Proto (void fix_objprogs, (void));
Proto (void fix_roomprogs, (void));
Proto (void init_area_weather, (void));
Proto (void fix_exits, (void));
Proto (void fix_mobprogs, (void));
Proto (void init_www_history, (void));
Proto (void build_player_index, (void));

DataTable time_data_table[] = {
  {"hour", FIELD_INT, (void *) &time_zero.hour, NULL, NULL, olced_int,
   NULL},
  {"day", FIELD_INT, (void *) &time_zero.day, NULL, NULL, olced_int, NULL},

  {
   "month", FIELD_INT, (void *) &time_zero.month, NULL, NULL, olced_int,
   NULL},
  {"year", FIELD_INT, (void *) &time_zero.year, NULL, NULL, olced_int,
   NULL},
  {"sunlight", FIELD_INT, (void *) &time_zero.sunlight, NULL, NULL,
   olced_int, NULL},
  {NULL, (field_t) - 1, NULL, NULL, NULL, NULL, NULL}
};

TableSave_Fun (rw_time_data)
{
  rw_single (type, TIME_FILE, time);
}

void
setup_myrlims (void)
{
#ifdef HAVE_SETRLIMIT
#define getsetlimit(res) \
do { \
    	struct rlimit limit; \
    getrlimit(res, &limit); \
    limit.rlim_cur = limit.rlim_max = RLIM_INFINITY; \
    setrlimit(res, &limit); \
} while(0)

  log_string ("Setting user limits...");
  getsetlimit (RLIMIT_CPU);
  getsetlimit (RLIMIT_FSIZE);
  getsetlimit (RLIMIT_DATA);
  getsetlimit (RLIMIT_STACK);
  getsetlimit (RLIMIT_CORE);
#ifdef    	RLIMIT_RSS

  getsetlimit (RLIMIT_RSS);
#endif
#if 0

  getsetlimit (RLIMIT_NPROC);
#endif
#if 0

  getsetlimit (RLIMIT_NOFILE);
#endif
#ifdef    	RLIMIT_MEMLOCK

  getsetlimit (RLIMIT_MEMLOCK);
#endif
#ifdef RLIMIT_AS

  getsetlimit (RLIMIT_AS);
#endif
#endif
}

void
load_area_db (void)
{
  FileData *fpList;
  const struct bootf_type
  {
    const char *header;
    Boot_F *fun;
  }
  bootf_table[] =
  {
    {
    "AREA", load_old_area}
    ,
    {
    "AREADATA", load_area}
    ,
    {
    "HELPS", load_old_helps}
    ,
    {
    "MOBOLD", load_old_mob}
    ,
    {
    "MOBILES", load_mobiles}
    ,
    {
    "MOBPROGS", load_mobprogs}
    ,
    {
    "OBJPROGS", load_objprogs}
    ,
    {
    "ROOMPROGS", load_roomprogs}
    ,
    {
    "OBJOLD", load_old_obj}
    ,
    {
    "OBJECTS", load_objects}
    ,
    {
    "RESETS", load_resets}
    ,
    {
    "ROOMS", load_rooms}
    ,
    {
    "SHOPS", load_shops}
    ,
    {
    "SPECIALS", load_specials}
    ,
    {
    NULL, NULL}
  };

  if ((fpList = f_open (AREA_LIST, "r")) == NULL)
    {
      log_error (AREA_LIST);
      exit (1);
    }

  while (true)
    {
      char buf[MIL];

      strcpy (strArea, read_word (fpList));

      if (strArea[0] == '$')
	break;

      sprintf (buf, AREA_DIR "%s", strArea);
      if ((fpArea = f_open (buf, "r")) == NULL)
	{
	  log_error (strArea);
	  exit (1);
	}

      logf ("Loading %s...", strArea);
      current_area = NULL;

      while (true)
	{
	  char *word;
	  int i;

	  if (read_letter (fpArea) != '#')
	    {
	      bug ("Boot_db: # not found.");
	      exit (1);
	    }

	  word = read_word (fpArea);

	  if (word[0] == '$')
	    break;


	  for (i = 0; bootf_table[i].header != NULL; i++)
	    {
	      if (!str_cmp (word, bootf_table[i].header))
		break;
	    }

	  if (bootf_table[i].fun != NULL)
	    {
	      (*bootf_table[i].fun) (fpArea);
	    }
	  else
	    {
	      bug ("Boot_db: bad section name.");
	      exit (1);
	    }
	}
      f_close (fpArea);
    }
  f_close (fpList);
  fpArea = NULL;
  logf ("Loaded %d areas.", top_area);
}


void
boot_db (void)
{

  run_level = RUNLEVEL_BOOTING;

  setup_myrlims ();

  init_mm ();


  rw_time_data (act_read);
  rw_class_data (act_read);
  rw_skill_data (act_read);
  rw_group_data (act_read);
  rw_cmd_data (act_read);
  rw_race_data (act_read);
  rw_clan_data (act_read);
  rw_deity_data (act_read);


  load_area_db ();


  rw_help_data (act_read);


  fix_exits ();
  fix_mobprogs ();
  fix_objprogs ();
  fix_roomprogs ();
  run_level = RUNLEVEL_SAFE_BOOT;
  convert_objects ();
  area_update ();
  build_player_index ();
  rw_note_data (act_read);
  rw_ban_data (act_read);
  rw_song_data (act_read);
  rw_disabled_data (act_read);
  rw_social_data (act_read);
  rw_gquest_data (act_read);
  rw_war_data (act_read);
  rw_mbr_data (act_read);
  rw_channel_data (act_read);
  rw_name_profile_data (act_read);
#ifndef DISABLE_WEBSRV

  init_www_history ();
#endif

  load_corpses ();
  load_room_objs ();
  rw_wpwd_data (act_read);
  init_area_weather ();
  logf ("Stored %d Areas, %d Rooms, %d Mobs, %d Objects & %d Helps.",
	top_area, top_room_index, top_char_index, top_obj_index, top_help);
  logf ("Finished. Bootup took %s.",
	timestr (getcurrenttime () - boot_time, false));
#ifndef DISABLE_WEBSRV

  if (web_is_connected ())
    logf ("Integrated world wide web server running on port %d.", webport);
#endif

  return;
}

Proto (void convert_area_credits, (AreaData *));


Boot_Fun (load_old_area)
{
  AreaData *pArea;

  pArea = new_area ();
  read_strfree (fp, &pArea->file_name);

  pArea->area_flags = AREA_LOADING;
  read_strfree (fp, &pArea->name);
  read_strfree (fp, &pArea->credits);
  pArea->min_vnum = read_number (fp);
  pArea->max_vnum = read_number (fp);

  add_area (pArea);
  current_area = pArea;
  convert_area_credits (pArea);

  top_area++;
  return;
}


Boot_Fun (load_area)
{
  AreaData *pArea;
  const char *word;
  bool fMatch;

  pArea = new_area ();
  replace_str (&pArea->file_name, strArea);

  while (true)
    {
      word = f_eof (fp) ? "End" : read_word (fp);
      fMatch = false;

      switch (toupper (word[0]))
	{
	case 'L':
	  Key_Str ("LvlComment", pArea->lvl_comment);
	  break;
	case 'M':
	  Key ("MinLevel", pArea->min_level, read_number (fp));
	  Key ("MaxLevel", pArea->max_level, read_number (fp));
	  break;
	case 'N':
	  Key_Str ("Name", pArea->name);
	  break;
	case 'F':
	  Key ("Flags", pArea->area_flags, read_flag (fp));
	  break;
	case 'R':
	  Key ("ResetMsg", pArea->resetmsg, read_string (fp));
	  Key ("Recall", pArea->recall, read_number (fp));
	  break;
	case 'S':
	  Key ("Security", pArea->security, read_number (fp));
	  Key ("Sound", pArea->sound, read_sound (fp));
	  if (!str_cmp (word, "Stats"))
	    {
	      pArea->kills = read_long (fp);
	      pArea->deaths = read_long (fp);
	      fMatch = true;
	      break;
	    }
	  break;
	case 'V':
	  Key ("Version", pArea->version, read_number (fp));
	  if (!str_cmp (word, "VNUMs"))
	    {
	      pArea->min_vnum = read_number (fp);
	      pArea->max_vnum = read_number (fp);
	      fMatch = true;
	      break;
	    }
	  break;
	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      fMatch = true;
	      add_area (pArea);
	      current_area = pArea;
	      convert_area_credits (pArea);

	      return;
	    }
	  break;
	case 'B':
	  Key_Str ("Builders", pArea->builders);
	  break;
	case 'C':
	  Key_Str ("Credits", pArea->credits);
	  if (!str_cmp (word, "Climate"))
	    {
	      pArea->weather.climate_temp = read_number (fp);
	      pArea->weather.climate_precip = read_number (fp);
	      pArea->weather.climate_wind = read_number (fp);
	      fMatch = true;
	      break;
	    }
	  Key_SFun ("Clan", pArea->clan, clan_lookup);
	  break;
	}
    }
}


void
assign_area_vnum (vnum_t vnum)
{
  if (current_area->min_vnum == 0 || current_area->max_vnum == 0)
    current_area->min_vnum = current_area->max_vnum = vnum;
  if (vnum != Range (current_area->min_vnum, vnum, current_area->max_vnum))
    {
      if (vnum < current_area->min_vnum)
	current_area->min_vnum = vnum;
      else
	current_area->max_vnum = vnum;
    }
  return;
}


Boot_Fun (load_old_helps)
{
  HelpData *pHelp;
  int level;
  const char *keyword;

  while (true)
    {
      level = read_number (fp);
      keyword = read_string (fp);

      if (keyword[0] == '$')
	break;

      pHelp = new_help ();
      pHelp->level = level;
      pHelp->keyword = keyword;
      read_strfree (fp, &pHelp->text);

      add_help (pHelp);
    }

  return;
}


Boot_Fun (load_old_mob)
{
  CharIndex *pMobIndex;


  RaceData *race;
  char name[MAX_STRING_LENGTH];

  if (!current_area)
    {
      bug ("Load_mobiles: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      vnum_t vnum;
      char letter;
      int iHash;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_mobiles: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_char_index (vnum) != NULL)
	{
	  bugf ("Load_mobiles: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pMobIndex = new_char_index ();
      pMobIndex->vnum = vnum;
      pMobIndex->area = current_area;
      pMobIndex->new_format = false;
      read_strfree (fp, &pMobIndex->player_name);
      read_strfree (fp, &pMobIndex->short_descr);
      read_strfree (fp, &pMobIndex->long_descr);
      read_strfree (fp, &pMobIndex->description);

      pMobIndex->act = read_flag (fp) | ACT_IS_NPC;
      pMobIndex->affected_by = read_flag (fp);
      pMobIndex->pShop = NULL;
      pMobIndex->alignment = read_number (fp);
      letter = read_letter (fp);
      pMobIndex->level = read_number (fp);


      read_number (fp);
      read_number (fp);
      read_number (fp);

      read_letter (fp);

      read_number (fp);

      read_letter (fp);

      read_number (fp);
      read_number (fp);

      read_letter (fp);

      read_number (fp);

      read_letter (fp);

      read_number (fp);
      pMobIndex->wealth = read_number (fp) / 20;

      read_number (fp);

      pMobIndex->start_pos = read_enum (position_t, fp);
      pMobIndex->default_pos = read_enum (position_t, fp);

      if (pMobIndex->start_pos < POS_SLEEPING)
	pMobIndex->start_pos = POS_STANDING;
      if (pMobIndex->default_pos < POS_SLEEPING)
	pMobIndex->default_pos = POS_STANDING;


      pMobIndex->sex = read_enum (sex_t, fp);


      one_argument (pMobIndex->player_name, name);

      if (NullStr (name) || (race = race_lookup (name)) == NULL)
	{

	  pMobIndex->race = default_race;
	  pMobIndex->off_flags =
	    OFF_DODGE | OFF_DISARM | OFF_TRIP | ASSIST_VNUM;
	  pMobIndex->imm_flags = 0;
	  pMobIndex->res_flags = 0;
	  pMobIndex->vuln_flags = 0;
	  pMobIndex->form =
	    FORM_EDIBLE | FORM_SENTIENT | FORM_BIPED | FORM_MAMMAL;
	  pMobIndex->parts =
	    PART_HEAD | PART_ARMS | PART_LEGS | PART_HEART |
	    PART_BRAINS | PART_GUTS;
	}
      else
	{
	  pMobIndex->race = race;
	  pMobIndex->off_flags =
	    OFF_DODGE | OFF_DISARM | OFF_TRIP | ASSIST_RACE | race->off;
	  pMobIndex->imm_flags = race->imm;
	  pMobIndex->res_flags = race->res;
	  pMobIndex->vuln_flags = race->vuln;
	  pMobIndex->form = race->form;
	  pMobIndex->parts = race->parts;
	}

      if (letter != 'S')
	{
	  bugf ("Load_mobiles: vnum %ld non-S.", vnum);
	  exit (1);
	}

      convert_mobile (pMobIndex);

      iHash = vnum % MAX_KEY_HASH;
      LinkSingle (pMobIndex, char_index_hash[iHash], next);
      top_vnum_mob = Max (top_vnum_mob, vnum);
      assign_area_vnum (vnum);
      kill_table[Range (0, pMobIndex->level, MAX_LEVEL - 1)].number++;
    }

  return;
}


Boot_Fun (load_old_obj)
{
  ObjIndex *pObjIndex;

  if (!current_area)
    {
      bug ("Load_objects: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      vnum_t vnum;
      char letter;
      int iHash = -1;
      const char *str;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_objects: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_obj_index (vnum) != NULL)
	{
	  bugf ("Load_objects: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pObjIndex = new_obj_index ();
      pObjIndex->vnum = vnum;
      pObjIndex->area = current_area;
      pObjIndex->new_format = false;
      pObjIndex->reset_num = 0;
      read_strfree (fp, &pObjIndex->name);
      str = read_string (fp);
      replace_str (&pObjIndex->short_descr, Upper (str));
      free_string (str);
      str = read_string (fp);
      replace_str (&pObjIndex->description, Upper (str));
      free_string (str);


      read_string (fp);

      pObjIndex->item_type = read_enum (item_t, fp);
      pObjIndex->extra_flags = read_flag (fp);
      pObjIndex->wear_flags = read_flag (fp);
      pObjIndex->value[0] = read_number (fp);
      pObjIndex->value[1] = read_number (fp);
      pObjIndex->value[2] = read_number (fp);
      pObjIndex->value[3] = read_number (fp);
      pObjIndex->value[4] = 0;
      pObjIndex->level = 0;
      pObjIndex->condition = 100;
      pObjIndex->weight = read_number (fp);
      pObjIndex->cost = read_number (fp);

      read_number (fp);

      if (pObjIndex->item_type == ITEM_WEAPON)
	{
	  if (is_name ("two", pObjIndex->name) ||
	      is_name ("two-handed", pObjIndex->name) ||
	      is_name ("claymore", pObjIndex->name))
	    SetBit (pObjIndex->value[4], WEAPON_TWO_HANDS);
	}

      while (iHash == -1)
	{
	  letter = read_letter (fp);

	  switch (letter)
	    {
	    case 'A':
	      {
		AffectData *paf;

		paf = new_affect ();
		paf->where = TO_OBJECT;
		paf->type = -1;
		paf->level = 20;
		paf->duration = -1;
		paf->location = read_enum (apply_t, fp);
		paf->modifier = read_number (fp);
		paf->bitvector = 0;
		Link (paf, pObjIndex->affect, next, prev);
	      }
	      break;
	    case 'E':
	      {
		ExDescrData *ed;

		ed = new_ed ();
		read_strfree (fp, &ed->keyword);
		read_strfree (fp, &ed->description);
		Link (ed, pObjIndex->ed, next, prev);
	      }
	      break;
	    default:
	      {
		iHash = vnum % MAX_KEY_HASH;
		f_ungetc (letter, fp);
		break;
	      }
	    }
	}


      if (pObjIndex->item_type == ITEM_ARMOR)
	{
	  pObjIndex->value[1] = pObjIndex->value[0];
	  pObjIndex->value[2] = pObjIndex->value[1];
	}

      LinkSingle (pObjIndex, obj_index_hash[iHash], next);
      top_vnum_obj = Max (top_vnum_obj, vnum);
      assign_area_vnum (vnum);
    }

  return;
}


Boot_Fun (load_resets)
{
  ResetData *pReset;
  ExitData *pexit;
  RoomIndex *pRoomIndex;
  vnum_t rVnum = -1;
  flag_t arg5;

  if (!current_area)
    {
      bug ("Load_resets: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      char letter;

      if ((letter = read_letter (fp)) == 'S')
	break;

      if (letter == '*')
	{
	  read_to_eol (fp);
	  continue;
	}

      pReset = new_reset ();
      pReset->command = letter;

      read_number (fp);
      pReset->arg1 = read_number (fp);
      pReset->arg2 = read_number (fp);
      pReset->arg3 = (letter == 'G'
		      || (current_area->version < 3
			  && letter == 'R')) ? 0 : read_number (fp);
      pReset->arg4 = (letter == 'P' || letter == 'M') ? read_number (fp) : 0;
      arg5 = (letter == 'F') ? read_flag (fp) : 0;
      read_to_eol (fp);

      switch (pReset->command)
	{
	case 'M':
	case 'O':
	  rVnum = pReset->arg3;
	  break;

	case 'P':
	case 'G':
	case 'E':
	  break;

	case 'D':
	  pRoomIndex = get_room_index ((rVnum = pReset->arg1));
	  if (pReset->arg2 < 0 || pReset->arg2 >= MAX_DIR ||
	      !pRoomIndex ||
	      !(pexit = pRoomIndex->exit[pReset->arg2]) ||
	      !IsSet (pexit->rs_flags, EX_ISDOOR))
	    {
	      bugf ("Load_resets: 'D': exit %d, room %ld not door.",
		    pReset->arg2, pReset->arg1);
	      exit (1);
	    }

	  switch (pReset->arg3)
	    {
	    default:
	      bugf ("Load_resets: 'D': bad 'locks': %ld.", pReset->arg3);
	      break;
	    case 0:
	      break;
	    case 1:
	      SetBit (pexit->rs_flags, EX_CLOSED);
	      SetBit (pexit->exit_info, EX_CLOSED);
	      break;
	    case 2:
	      SetBit (pexit->rs_flags, EX_CLOSED | EX_LOCKED);
	      SetBit (pexit->exit_info, EX_CLOSED | EX_LOCKED);
	      break;
	    }
	  break;

	case 'F':
	  pRoomIndex = get_room_index ((rVnum = pReset->arg1));

	  if (pReset->arg2 < 0 || pReset->arg2 > (MAX_DIR - 1)
	      || !pRoomIndex || !(pexit = pRoomIndex->exit[pReset->arg2]))
	    {
	      bugf ("'F': Nonexistent exit [%ld:%d].", pRoomIndex->vnum,
		    pReset->arg2);
	      break;
	    }

	  pexit->rs_flags = arg5;
	  pexit->exit_info = arg5;
	  break;

	case 'R':
	  rVnum = pReset->arg1;
	  break;
	}

      if (rVnum == -1)
	{
	  bugf ("load_resets : rVnum == -1");
	  exit (1);
	}

      if (pReset->command != 'D' && pReset->command != 'F')
	add_reset (get_room_index (rVnum), pReset, 0);
      else
	free_reset (pReset);
    }

  return;
}


Boot_Fun (load_rooms)
{
  RoomIndex *pRoomIndex;

  if (current_area == NULL)
    {
      bug ("Load_resets: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      vnum_t vnum;
      char letter;
      int door;
      int iHash = -1;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_rooms: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_room_index (vnum) != NULL)
	{
	  bugf ("Load_rooms: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pRoomIndex = new_room_index ();
      pRoomIndex->person_first = NULL;
      pRoomIndex->content_first = NULL;
      pRoomIndex->ed_first = NULL;
      pRoomIndex->area = current_area;
      pRoomIndex->vnum = vnum;
      read_strfree (fp, &pRoomIndex->name);
      read_strfree (fp, &pRoomIndex->description);

      read_number (fp);
      pRoomIndex->room_flags = read_flag (fp);

      if (3000 <= vnum && vnum < 3400)
	SetBit (pRoomIndex->room_flags, ROOM_LAW);
      pRoomIndex->sector_type = read_enum (sector_t, fp);
      pRoomIndex->light = 0;
      for (door = 0; door <= 5; door++)
	pRoomIndex->exit[door] = NULL;


      pRoomIndex->heal_rate = 100;
      pRoomIndex->mana_rate = 100;

      while (iHash == -1)
	{
	  letter = read_letter (fp);

	  switch (letter)
	    {
	    case 'S':
	      iHash = vnum % MAX_KEY_HASH;
	      break;

	    case 'H':
	      pRoomIndex->heal_rate = read_number (fp);
	      break;

	    case 'M':
	      pRoomIndex->mana_rate = read_number (fp);
	      break;

	    case 'C':
	      {
		const char *tmp = read_string (fp);

		free_string (tmp);
	      }
	      break;

	    case 'G':
	      if (pRoomIndex->guild > -1 && pRoomIndex->guild < top_class)
		{
		  bug ("Duplicate guild.");
		  exit (1);
		}

	      pRoomIndex->guild = read_number (fp);
	      break;
	    case 'D':
	      {
		ExitData *pexit;

		door = read_number (fp);
		if (door < 0 || door > 5)
		  {
		    bugf ("Fread_rooms: vnum %ld has bad door number.", vnum);
		    exit (1);
		  }

		pexit = new_exit ();
		read_strfree (fp, &pexit->description);
		read_strfree (fp, &pexit->keyword);
		pexit->exit_info = 0;
		pexit->rs_flags = 0;
		if (pRoomIndex->area->version < 2)
		  {
		    switch (read_number (fp))
		      {
		      case 1:
			pexit->exit_info = EX_ISDOOR;
			pexit->rs_flags = EX_ISDOOR;
			break;
		      case 2:
			pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
			pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF;
			break;
		      case 3:
			pexit->exit_info = EX_ISDOOR | EX_NOPASS;
			pexit->rs_flags = EX_ISDOOR | EX_NOPASS;
			break;
		      case 4:
			pexit->exit_info =
			  EX_ISDOOR | EX_NOPASS | EX_PICKPROOF;
			pexit->rs_flags =
			  EX_ISDOOR | EX_NOPASS | EX_PICKPROOF;
			break;
		      }
		  }
		else
		  pexit->rs_flags = read_flag (fp);

		if (IsSet (pexit->rs_flags, EX_TEMP))
		  RemBit (pexit->rs_flags, EX_TEMP);

		pexit->exit_info = pexit->rs_flags;
		pexit->key = read_number (fp);
		pexit->u1.vnum = read_number (fp);
		pexit->orig_door = door;

		pRoomIndex->exit[door] = pexit;
	      }
	      break;
	    case 'E':
	      {
		ExDescrData *ed;

		ed = new_ed ();
		read_strfree (fp, &ed->keyword);
		read_strfree (fp, &ed->description);
		Link (ed, pRoomIndex->ed, next, prev);
	      }
	      break;

	    case 'O':
	      if (!NullStr (pRoomIndex->owner))
		{
		  bug ("Load_rooms: duplicate owner.");
		  exit (1);
		}

	      pRoomIndex->owner = read_string (fp);
	      SetBit (pRoomIndex->room_flags, ROOM_NOEXPLORE);
	      break;

	    case 'R':
	      {
		ProgList *pRprog;
		const char *word;
		flag_t trigger = 0;

		pRprog = new_prog_list ();
		word = read_word (fp);
		if ((trigger = flag_value (rprog_flags, word)) == NO_FLAG)
		  {
		    bug ("ROOMprogs: invalid trigger.");
		    exit (1);
		  }
		SetBit (pRoomIndex->rprog_flags, trigger);
		pRprog->trig_type = trigger;
		pRprog->prog = (ProgCode *) read_long (fp);
		read_strfree (fp, &pRprog->trig_phrase);
		Link (pRprog, pRoomIndex->rprog, next, prev);
	      }
	      break;
	    default:
	      bugf ("Load_rooms: vnum %ld has invalid flag %c.", vnum,
		    letter);
	      exit (1);
	      break;
	    }
	}

      LinkSingle (pRoomIndex, room_index_hash[iHash], next);
      if (!IsSet (pRoomIndex->room_flags, ROOM_NOEXPLORE))
	top_explored++;
      top_vnum_room = Max (top_vnum_room, vnum);
      assign_area_vnum (vnum);
    }

  return;
}


Boot_Fun (load_shops)
{
  ShopData *pShop;

  while (true)
    {
      CharIndex *pMobIndex;
      int iTrade;

      pShop = new_shop ();
      pShop->keeper = read_number (fp);
      if (pShop->keeper == 0)
	break;
      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
	pShop->buy_type[iTrade] = read_number (fp);
      pShop->profit_buy = read_number (fp);
      pShop->profit_sell = read_number (fp);
      pShop->open_hour = read_number (fp);
      pShop->close_hour = read_number (fp);
      read_to_eol (fp);
      pMobIndex = get_char_index (pShop->keeper);
      pMobIndex->pShop = pShop;
      Link (pShop, shop, next, prev);
    }

  return;
}


Boot_Fun (load_specials)
{
  while (true)
    {
      CharIndex *pMobIndex;
      char letter = read_letter (fp);

      switch (letter)
	{
	default:
	  bugf ("Load_specials: letter '%c' not *MS.", letter);
	  exit (1);

	case 'S':
	  return;

	case '*':
	  break;

	case 'M':
	  pMobIndex = get_char_index (read_number (fp));
	  pMobIndex->spec_fun = spec_lookup (read_word (fp));
	  if (pMobIndex->spec_fun == 0)
	    {
	      bugf ("Load_specials: 'M': vnum %ld.", pMobIndex->vnum);
	      exit (1);
	    }
	  break;
	}

      read_to_eol (fp);
    }
}


void
fix_exits (void)
{
  RoomIndex *pRoomIndex;


  ExitData *pexit;
  ResetData *pReset;
  RoomIndex *iLastRoom, *iLastObj;
  int iHash;
  int door;

  log_string ("Fixing exits...");
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex != NULL;
	   pRoomIndex = pRoomIndex->next)
	{
	  bool fexit;

	  iLastRoom = iLastObj = NULL;


	  for (pReset = pRoomIndex->reset_first; pReset;
	       pReset = pReset->next)
	    {
	      switch (pReset->command)
		{
		default:
		  bugf ("fix_exits : cuarto %ld con reset cmd %c",
			pRoomIndex->vnum, pReset->command);
		  exit (1);
		  break;

		case 'M':
		  get_char_index (pReset->arg1);
		  iLastRoom = get_room_index (pReset->arg3);
		  break;

		case 'O':
		  get_obj_index (pReset->arg1);
		  iLastObj = get_room_index (pReset->arg3);
		  break;

		case 'P':
		  get_obj_index (pReset->arg1);
		  if (iLastObj == NULL)
		    {
		      bugf
			("fix_exits : reset en cuarto %ld con iLastObj NULL",
			 pRoomIndex->vnum);
		      exit (1);
		    }
		  break;

		case 'G':
		case 'E':
		  get_obj_index (pReset->arg1);
		  if (iLastRoom == NULL)
		    {
		      bugf
			("fix_exits : reset en cuarto %ld con iLastRoom NULL",
			 pRoomIndex->vnum);
		      exit (1);
		    }
		  iLastObj = iLastRoom;
		  break;

		case 'D':
		  bugf ("???");
		  break;

		case 'R':
		  get_room_index (pReset->arg1);
		  if (pReset->arg2 < 0 || pReset->arg2 > MAX_DIR)
		    {
		      bugf
			("fix_exits : reset en cuarto %ld con arg2 %d >= MAX_DIR",
			 pRoomIndex->vnum, pReset->arg2);
		      exit (1);
		    }
		  break;
		}
	    }

	  fexit = false;
	  for (door = 0; door <= 5; door++)
	    {
	      if ((pexit = pRoomIndex->exit[door]) != NULL)
		{
		  if (pexit->u1.vnum <= 0 ||
		      get_room_index (pexit->u1.vnum) == NULL)
		    pexit->u1.to_room = NULL;
		  else
		    {
		      fexit = true;
		      pexit->u1.to_room = get_room_index (pexit->u1.vnum);
		    }
		}
	    }
	  if (!fexit)
	    SetBit (pRoomIndex->room_flags, ROOM_NO_MOB);
	}
    }



  return;
}


Boot_Fun (load_mobprogs)
{
  ProgCode *pMprog;

  if (current_area == NULL)
    {
      bug ("Load_mobprogs: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      vnum_t vnum;
      char letter;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_mobprogs: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_prog_index (vnum, PRG_MPROG) != NULL)
	{
	  bugf ("Load_mobprogs: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pMprog = new_mprog ();
      pMprog->vnum = vnum;
      read_strfree (fp, &pMprog->code);
      pMprog->area = current_area;
      Link (pMprog, mprog, next, prev);
    }
  return;
}


void
fix_mobprogs (void)
{
  CharIndex *pMobIndex;
  ProgList *list;
  ProgCode *prog;
  int iHash;

  log_string ("Fixing mob programs...");
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = char_index_hash[iHash]; pMobIndex != NULL;
	   pMobIndex = pMobIndex->next)
	{
	  for (list = pMobIndex->mprog_first; list != NULL; list = list->next)
	    {
	      if ((prog = get_prog_index ((vnum_t) list->prog, PRG_MPROG))
		  == NULL)
		{
		  bugf ("Fix_roomprogs: code vnum %ld not found.",
			(vnum_t) list->prog);
		  exit (1);
		}
	      else
		list->prog = prog;
	    }
	}
    }
}


void
area_update (void)
{
  AreaData *pArea;
  int hash;
  RoomIndex *room;

  if (run_level == RUNLEVEL_BOOTING)
    log_string ("Updating areas...");

  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {

      if (++pArea->age < 3)
	continue;


      if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
	  || pArea->age >= 31)
	{
	  RoomIndex *pRoomIndex;
	  const char *msg = NULL;

	  reset_area (pArea);
	  new_wiznet (NULL, NULL, WIZ_RESETS, false, 0,
		      "%s has just been reset.", pArea->name);
	  if (IsSet (pArea->area_flags, AREA_CHANGED))
	    {
	      save_area (pArea);
	      new_wiznet (NULL, NULL, WIZ_RESETS, true, 0,
			  "%s has just been saved.", pArea->name);
	    }

	  pArea->age = number_range (0, 3);
	  pRoomIndex = get_room_index (ROOM_VNUM_SCHOOL);
	  if (pRoomIndex != NULL && pArea == pRoomIndex->area)
	    pArea->age = 15 - 2;
	  else if (pArea->nplayer == 0)
	    pArea->empty = true;
	  if (!NullStr (pArea->resetmsg))
	    msg = pArea->resetmsg;
	  else if (!pRoomIndex || pRoomIndex->area != pArea)



	    {
	      switch (number_range (0, 4))
		{
		default:
		  msg = "The area repopulates itself.";
		  break;
		case 1:
		  msg = "You notice a change in the area.";
		  break;
		case 2:
		  msg =
		    "Time completes another cycle bringing life to the area.";
		  break;
		case 3:
		  msg =
		    "You feel a sudden deja-vu bringing change to the area.";
		  break;
		case 4:
		  msg = "You hear noises off in the distance...";
		  break;
		}
	    }
	  if (!NullStr (msg))
	    {
	      CharData *ch;

	      for (ch = player_first; ch != NULL; ch = ch->next_player)
		{
		  if (ch->desc && ch->desc->connected == CON_PLAYING &&
		      IsAwake (ch) && ch->in_room
		      && ch->in_room->area == pArea)
		    chprintlnf (ch, "{?%s{x", msg);
		}
	    }
	}
    }


  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    for (room = room_index_hash[hash]; room; room = room->next)
      {
	if (room->area->empty)
	  continue;

	if (HasTriggerRoom (room, TRIG_DELAY) && room->rprog_delay > 0)
	  {
	    if (--room->rprog_delay <= 0)
	      p_percent_trigger (NULL, NULL, room, NULL, NULL, NULL,
				 TRIG_DELAY);
	  }
	else if (HasTriggerRoom (room, TRIG_RANDOM))
	  p_percent_trigger (NULL, NULL, room, NULL, NULL, NULL, TRIG_RANDOM);
      }
  return;
}


void
reset_room (RoomIndex * pRoom)
{
  ResetData *pReset;
  CharData *pMob;
  CharData *mob;
  ObjData *pObj;
  CharData *LastMob = NULL;
  ObjData *LastObj = NULL;
  int iExit;
  int level = 0;
  bool last;

  if (!pRoom)
    return;

  pMob = NULL;
  last = false;

  for (iExit = 0; iExit < MAX_DIR; iExit++)
    {
      ExitData *pExit;

      if ((pExit = pRoom->exit[iExit]))
	{
	  pExit->exit_info = pExit->rs_flags;
	  if ((pExit->u1.to_room != NULL) &&
	      ((pExit = pExit->u1.to_room->exit[rev_dir[iExit]])))
	    {

	      pExit->exit_info = pExit->rs_flags;
	    }
	}
    }

  for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
    {
      CharIndex *pMobIndex;
      ObjIndex *pObjIndex;
      ObjIndex *pObjToIndex;
      RoomIndex *pRoomIndex;
      char buf[MAX_STRING_LENGTH];
      int count, limit = 0;

      switch (pReset->command)
	{
	default:
	  bugf ("Reset_room: bad command %c.", pReset->command);
	  break;

	case 'M':
	  if (!(pMobIndex = get_char_index (pReset->arg1)))
	    {
	      bugf ("Reset_room: 'M': bad vnum %ld.", pReset->arg1);
	      continue;
	    }

	  if ((pRoomIndex = get_room_index (pReset->arg3)) == NULL)
	    {
	      bugf ("Reset_area: 'R': bad vnum %ld.", pReset->arg3);
	      continue;
	    }
	  if (pMobIndex->count >= pReset->arg2)
	    {
	      last = false;
	      break;
	    }

	  count = 0;
	  for (mob = pRoomIndex->person_first; mob != NULL;
	       mob = mob->next_in_room)
	    if (mob->pIndexData == pMobIndex)
	      {
		count++;
		if (count >= pReset->arg4)
		  {
		    last = false;
		    break;
		  }
	      }

	  if (count >= pReset->arg4)
	    break;



	  pMob = create_mobile (pMobIndex);


	  if (room_is_dark (pRoom))
	    SetBit (pMob->affected_by, AFF_INFRARED);


	  {
	    RoomIndex *pRoomIndexPrev;

	    pRoomIndexPrev = get_room_index (pRoom->vnum - 1);
	    if (pRoomIndexPrev &&
		IsSet (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
	      SetBit (pMob->act, ACT_PET);
	  }

	  char_to_room (pMob, pRoom);

	  LastMob = pMob;
	  level = Range (0, pMob->level - 2, MAX_MORTAL_LEVEL - 1);
	  last = true;
	  break;

	case 'O':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      bugf ("Reset_room: 'O' 1 : bad vnum %ld", pReset->arg1);
	      sprintf (buf, "%ld %d %ld %d", pReset->arg1,
		       pReset->arg2, pReset->arg3, pReset->arg4);
	      bugf (buf, 1);
	      continue;
	    }

	  if (!(pRoomIndex = get_room_index (pReset->arg3)))
	    {
	      bugf ("Reset_room: 'O' 2 : bad vnum %ld.", pReset->arg3);
	      sprintf (buf, "%ld %d %ld %d", pReset->arg1,
		       pReset->arg2, pReset->arg3, pReset->arg4);
	      bugf (buf, 1);
	      continue;
	    }

	  if (pRoom->area->nplayer > 0 ||
	      count_obj_list (pObjIndex, pRoom->content_first) > 0)
	    {
	      last = false;
	      break;
	    }

	  pObj = create_object (pObjIndex,
				Min (number_fuzzy (level),
				     MAX_MORTAL_LEVEL - 1));
	  pObj->cost = 0;
	  obj_to_room (pObj, pRoom);
	  last = true;
	  break;

	case 'P':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      bugf ("Reset_room: 'P': bad vnum %ld.", pReset->arg1);
	      continue;
	    }

	  if (!(pObjToIndex = get_obj_index (pReset->arg3)))
	    {
	      bugf ("Reset_room: 'P': bad vnum %ld.", pReset->arg3);
	      continue;
	    }

	  if (pReset->arg2 > 50)
	    limit = 6;
	  else if (pReset->arg2 == -1)
	    limit = 999;
	  else
	    limit = pReset->arg2;

	  if (pRoom->area->nplayer > 0
	      || (LastObj = get_obj_type (pObjToIndex)) == NULL
	      || (LastObj->in_room == NULL && !last)
	      || (pObjIndex->count >= limit)
	      || (count =
		  count_obj_list (pObjIndex,
				  LastObj->content_first)) > pReset->arg4)
	    {
	      last = false;
	      break;
	    }


	  while (count < pReset->arg4)
	    {
	      pObj = create_object (pObjIndex, number_fuzzy (LastObj->level));
	      obj_to_obj (pObj, LastObj);
	      count++;
	      if (pObjIndex->count >= limit)
		break;
	    }


	  LastObj->value[1] = LastObj->pIndexData->value[1];
	  last = true;
	  break;

	case 'G':
	case 'E':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      bugf ("Reset_room: 'E' or 'G': bad vnum %ld.", pReset->arg1);
	      continue;
	    }

	  if (!last)
	    break;

	  if (!LastMob)
	    {
	      bugf ("Reset_room: 'E' or 'G': null mob for vnum %ld.",
		    pReset->arg1);
	      last = false;
	      break;
	    }

	  if (LastMob->pIndexData->pShop)
	    {
	      int olevel = 0, i, j;

	      if (!pObjIndex->new_format)
		switch (pObjIndex->item_type)
		  {
		  default:
		    olevel = 0;
		    break;
		  case ITEM_PILL:
		  case ITEM_POTION:
		  case ITEM_SCROLL:
		    olevel = ANGEL;
		    for (i = 1; i < 5; i++)
		      {
			if (pObjIndex->value[i] > 0)
			  {
			    for (j = 0; j < top_class; j++)
			      {
				olevel =
				  Min
				  (olevel,
				   skill_table
				   [pObjIndex->value[i]].skill_level[j]);
			      }
			  }
		      }

		    olevel = Max (0, (olevel * 3 / 4) - 2);
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
		    olevel = number_range (5, 15);
		    break;
		  case ITEM_TREASURE:
		    olevel = number_range (10, 20);
		    break;

		  }

	      pObj = create_object (pObjIndex, olevel);
	      SetBit (pObj->extra_flags, ITEM_INVENTORY);

	    }
	  else
	    {

	      int plimit;

	      if (pReset->arg2 > 50)
		plimit = 6;
	      else if (pReset->arg2 == -1 || pReset->arg2 == 0)
		plimit = 999;
	      else
		plimit = pReset->arg2;

	      if (pObjIndex->count < plimit || number_range (0, 4) == 0)
		{
		  pObj =
		    create_object (pObjIndex,
				   Min (number_fuzzy
					(level), MAX_MORTAL_LEVEL - 1));

		}
	      else
		break;
	    }

	  obj_to_char (pObj, LastMob);
	  if (pReset->command == 'E')
	    equip_char (LastMob, pObj, (wloc_t) pReset->arg3);
	  last = true;
	  break;

	case 'D':
	  break;

	case 'R':
	  if (!(pRoomIndex = get_room_index (pReset->arg1)))
	    {
	      bugf ("Reset_room: 'R': bad vnum %ld.", pReset->arg1);
	      continue;
	    }

	  {
	    ExitData *pExit;
	    int d0;
	    int d1;

	    switch (pReset->arg3)
	      {
	      default:
		for (d0 = 0; d0 < pReset->arg2 - 1; d0++)
		  {
		    d1 = number_range (d0, pReset->arg2 - 1);
		    pExit = pRoomIndex->exit[d0];
		    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
		    pRoomIndex->exit[d1] = pExit;
		  }
		break;
	      case 1:
		add_random_exit (pRoomIndex, pReset, true);
		break;
	      case 2:
		add_random_exit (pRoomIndex, pReset, false);
		break;
	      }
	  }
	  break;
	}
    }

  return;
}


void
reset_area (AreaData * pArea)
{
  RoomIndex *pRoom;
  vnum_t vnum;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
      if ((pRoom = get_room_index (vnum)))
	reset_room (pRoom);
    }

  return;
}


CharData *
create_mobile (CharIndex * pMobIndex)
{
  CharData *mob;
  int i;
  AffectData af;

  mobile_count++;

  if (pMobIndex == NULL)
    {
      bug ("Create_mobile: NULL pMobIndex.");
      exit (1);
    }

  mob = new_char ();

  mob->pIndexData = pMobIndex;

  mob->name = str_dup (pMobIndex->player_name);
  mob->short_descr = str_dup (pMobIndex->short_descr);
  mob->long_descr = str_dup (pMobIndex->long_descr);
  mob->description = str_dup (pMobIndex->description);
  mob->id = get_mob_id ();
  mob->spec_fun = pMobIndex->spec_fun;
  mob->prompt = NULL;
  mob->mprog_target = NULL;

  if (pMobIndex->wealth == 0)
    {
      mob->silver = 0;
      mob->gold = 0;
    }
  else
    {
      money_t wealth;

      wealth =
	number_range (pMobIndex->wealth / 2, 3 * pMobIndex->wealth / 2);
      mob->gold = number_range (wealth / 200, wealth / 100);
      mob->silver = wealth - (mob->gold * 100);
    }

  mob->act = pMobIndex->act;
  mob->affected_by = pMobIndex->affected_by;
  mob->alignment = pMobIndex->alignment;
  mob->level = pMobIndex->level;
  mob->hitroll = pMobIndex->hitroll;

  if (pMobIndex->random > 0)
    mob->level =
      number_range (Max (1, pMobIndex->level - pMobIndex->random),
		    pMobIndex->level + pMobIndex->random);
  else
    mob->level = pMobIndex->level;

  mob->race = pMobIndex->race;
  mob->off_flags = pMobIndex->off_flags;
  mob->imm_flags = pMobIndex->imm_flags;
  mob->res_flags = pMobIndex->res_flags;
  mob->vuln_flags = pMobIndex->vuln_flags;
  mob->start_pos = pMobIndex->start_pos;
  mob->default_pos = pMobIndex->default_pos;
  mob->sex = pMobIndex->sex;
  if (mob->sex == SEX_RANDOM)
    mob->sex = (sex_t) number_range (SEX_MALE, SEX_FEMALE);
  mob->form = pMobIndex->form;
  mob->parts = pMobIndex->parts;

  if (pMobIndex->new_format)

    {

      mob->group = pMobIndex->group;
      mob->comm = COMM_NOCHANNELS | COMM_NOSHOUT | COMM_NOTELL;
      mob->damroll = pMobIndex->damage[DICE_BONUS];
      mob->max_hit =
	dice (pMobIndex->hit[DICE_NUMBER],
	      pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
      mob->hit = mob->max_hit;
      mob->max_mana =
	dice (pMobIndex->mana[DICE_NUMBER],
	      pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS];
      mob->mana = mob->max_mana;
      mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
      mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
      mob->dam_type = pMobIndex->dam_type;
      if (mob->dam_type == 0)
	switch (number_range (1, 3))
	  {
	  case (1):
	    mob->dam_type = 3;
	    break;
	  case (2):
	    mob->dam_type = 7;
	    break;
	  case (3):
	    mob->dam_type = 11;
	    break;
	  }
      for (i = 0; i < MAX_AC; i++)
	mob->armor[i] = pMobIndex->ac[i];
      mob->material = str_dup (pMobIndex->material);



      for (i = 0; i < STAT_MAX; i++)
	mob->perm_stat[i] = Min (25, 11 + mob->level / 4);

      {
	int stance = number_range (STANCE_NORMAL, STANCE_SWALLOW);

	SetStance (mob, STANCE_CURRENT, STANCE_NONE);
	SetStance (mob, STANCE_AUTODROP, stance);
      }

      if (IsSet (mob->act, ACT_WARRIOR))
	{
	  mob->perm_stat[STAT_STR] += 3;
	  mob->perm_stat[STAT_INT] -= 1;
	  mob->perm_stat[STAT_CON] += 2;
	}

      if (IsSet (mob->act, ACT_THIEF))
	{
	  mob->perm_stat[STAT_DEX] += 3;
	  mob->perm_stat[STAT_INT] += 1;
	  mob->perm_stat[STAT_WIS] -= 1;
	}

      if (IsSet (mob->act, ACT_CLERIC))
	{
	  mob->perm_stat[STAT_WIS] += 3;
	  mob->perm_stat[STAT_DEX] -= 1;
	  mob->perm_stat[STAT_STR] += 1;
	}

      if (IsSet (mob->act, ACT_MAGE))
	{
	  mob->perm_stat[STAT_INT] += 3;
	  mob->perm_stat[STAT_STR] -= 1;
	  mob->perm_stat[STAT_DEX] += 1;
	}

      if (IsSet (mob->off_flags, OFF_FAST))
	mob->perm_stat[STAT_DEX] += 2;

      mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
      mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;


      if (IsAffected (mob, AFF_SANCTUARY))
	{
	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("sanctuary");
	  af.level = mob->level;
	  af.duration = -1;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = AFF_SANCTUARY;
	  affect_to_char (mob, &af);
	}

      if (IsAffected (mob, AFF_HASTE))
	{
	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("haste");
	  af.level = mob->level;
	  af.duration = -1;
	  af.location = APPLY_DEX;
	  af.modifier =
	    1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32);
	  af.bitvector = AFF_HASTE;
	  affect_to_char (mob, &af);
	}

      if (IsAffected (mob, AFF_PROTECT_EVIL))
	{
	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("protection evil");
	  af.level = mob->level;
	  af.duration = -1;
	  af.location = APPLY_SAVES;
	  af.modifier = -1;
	  af.bitvector = AFF_PROTECT_EVIL;
	  affect_to_char (mob, &af);
	}

      if (IsAffected (mob, AFF_PROTECT_GOOD))
	{
	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("protection good");
	  af.level = mob->level;
	  af.duration = -1;
	  af.location = APPLY_SAVES;
	  af.modifier = -1;
	  af.bitvector = AFF_PROTECT_GOOD;
	  affect_to_char (mob, &af);
	}
    }
  else
    {

      mob->damroll = 0;
      mob->max_hit =
	mob->level * 8 + number_range (mob->level * mob->level / 4,
				       mob->level * mob->level);
      mob->max_hit = mob->max_hit * 9 / 10;
      mob->hit = mob->max_hit;
      mob->max_mana = 100 + dice (mob->level, 10);
      mob->mana = mob->max_mana;
      switch (number_range (1, 3))
	{
	case (1):
	  mob->dam_type = 3;
	  break;
	case (2):
	  mob->dam_type = 7;
	  break;
	case (3):
	  mob->dam_type = 11;
	  break;
	}
      for (i = 0; i < 3; i++)
	mob->armor[i] = interpolate (mob->level, 100, -100);
      mob->armor[3] = interpolate (mob->level, 100, 0);
      mob->size = SIZE_MEDIUM;
      mob->material = "";

      for (i = 0; i < STAT_MAX; i++)
	mob->perm_stat[i] = 11 + mob->level / 4;
    }

  mob->position = mob->start_pos;


  Link (mob, char, next, prev);

  pMobIndex->count++;
  return mob;
}


void
clone_mobile (CharData * parent, CharData * clone)
{
  int i;
  AffectData *paf;

  if (parent == NULL || clone == NULL || !IsNPC (parent))
    return;


  replace_str (&clone->name, parent->name);
  clone->version = parent->version;
  replace_str (&clone->short_descr, parent->short_descr);
  replace_str (&clone->long_descr, parent->long_descr);
  replace_str (&clone->description, parent->description);
  clone->group = parent->group;
  clone->sex = parent->sex;
  for (i = 0; i < MAX_MCLASS; i++)
    clone->Class[i] = parent->Class[i];
  clone->race = parent->race;
  clone->level = parent->level;
  clone->trust = 0;
  clone->timer = parent->timer;
  clone->wait = parent->wait;
  clone->hit = parent->hit;
  clone->max_hit = parent->max_hit;
  clone->mana = parent->mana;
  clone->max_mana = parent->max_mana;
  clone->move = parent->move;
  clone->max_move = parent->max_move;
  clone->gold = parent->gold;
  clone->silver = parent->silver;
  clone->exp = parent->exp;
  clone->act = parent->act;
  clone->comm = parent->comm;
  clone->imm_flags = parent->imm_flags;
  clone->res_flags = parent->res_flags;
  clone->vuln_flags = parent->vuln_flags;
  clone->invis_level = parent->invis_level;
  clone->affected_by = parent->affected_by;
  clone->position = parent->position;
  clone->practice = parent->practice;
  clone->train = parent->train;
  clone->saving_throw = parent->saving_throw;
  clone->alignment = parent->alignment;
  clone->hitroll = parent->hitroll;
  clone->damroll = parent->damroll;
  clone->wimpy = parent->wimpy;
  clone->form = parent->form;
  clone->parts = parent->parts;
  clone->size = parent->size;
  replace_str (&clone->material, parent->material);
  clone->off_flags = parent->off_flags;
  clone->dam_type = parent->dam_type;
  clone->start_pos = parent->start_pos;
  clone->default_pos = parent->default_pos;
  clone->spec_fun = parent->spec_fun;

  for (i = 0; i < MAX_AC; i++)
    clone->armor[i] = parent->armor[i];

  for (i = 0; i < STAT_MAX; i++)
    {
      clone->perm_stat[i] = parent->perm_stat[i];
      clone->mod_stat[i] = parent->mod_stat[i];
    }

  for (i = 0; i < 3; i++)
    clone->damage[i] = parent->damage[i];


  for (paf = parent->affect_first; paf != NULL; paf = paf->next)
    affect_to_char (clone, paf);

}


ObjData *
create_object (ObjIndex * pObjIndex, int level)
{
  AffectData *paf;
  ObjData *obj;
  int i;

  if (pObjIndex == NULL)
    {
      bug ("Create_object: NULL pObjIndex.");
      exit (1);
    }

  obj = new_obj ();

  obj->pIndexData = pObjIndex;
  obj->in_room = NULL;
  obj->enchanted = false;

  if (pObjIndex->new_format)
    obj->level = pObjIndex->level;
  else
    obj->level = Max (0, level);
  obj->wear_loc = WEAR_NONE;

  obj->name = str_dup (pObjIndex->name);
  obj->short_descr = str_dup (pObjIndex->short_descr);
  obj->description = str_dup (pObjIndex->description);
  obj->material = str_dup (pObjIndex->material);
  obj->item_type = pObjIndex->item_type;
  obj->extra_flags = pObjIndex->extra_flags;
  obj->wear_flags = pObjIndex->wear_flags;
  obj->value[0] = pObjIndex->value[0];
  obj->value[1] = pObjIndex->value[1];
  obj->value[2] = pObjIndex->value[2];
  obj->value[3] = pObjIndex->value[3];
  obj->value[4] = pObjIndex->value[4];
  obj->weight = pObjIndex->weight;

  if (level == -1 || pObjIndex->new_format)
    obj->cost = pObjIndex->cost;
  else
    obj->cost =
      number_fuzzy (10) * number_fuzzy (level) * number_fuzzy (level);


  switch (obj->item_type)
    {
    default:
      bugf ("Read_object: vnum %ld bad type.", pObjIndex->vnum);
      break;

    case ITEM_LIGHT:
      if (obj->value[2] == 999)
	obj->value[2] = -1;
      break;

    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_MAP:
    case ITEM_CLOTHING:
    case ITEM_PORTAL:
      if (!pObjIndex->new_format)
	obj->cost /= 5;
      break;

    case ITEM_TREASURE:
    case ITEM_WARP_STONE:
    case ITEM_ROOM_KEY:
    case ITEM_GEM:
    case ITEM_JEWELRY:
      break;

    case ITEM_JUKEBOX:
      for (i = 0; i < 5; i++)
	obj->value[i] = -1;
      break;

    case ITEM_SCROLL:
      if (level != -1 && !pObjIndex->new_format)
	obj->value[0] = number_fuzzy (obj->value[0]);
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      if (level != -1 && !pObjIndex->new_format)
	{
	  obj->value[0] = number_fuzzy (obj->value[0]);
	  obj->value[1] = number_fuzzy (obj->value[1]);
	  obj->value[2] = obj->value[1];
	}
      if (!pObjIndex->new_format)
	obj->cost *= 2;
      break;

    case ITEM_WEAPON:
      if (level != -1 && !pObjIndex->new_format)
	{
	  obj->value[1] = number_fuzzy (number_fuzzy (1 * level / 4 + 2));
	  obj->value[2] = number_fuzzy (number_fuzzy (3 * level / 4 + 6));
	}
      break;

    case ITEM_ARMOR:
      if (level != -1 && !pObjIndex->new_format)
	{
	  obj->value[0] = number_fuzzy (level / 5 + 3);
	  obj->value[1] = number_fuzzy (level / 5 + 3);
	  obj->value[2] = number_fuzzy (level / 5 + 3);
	}
      break;

    case ITEM_POTION:
    case ITEM_PILL:
      if (level != -1 && !pObjIndex->new_format)
	obj->value[0] = number_fuzzy (number_fuzzy (obj->value[0]));
      break;

    case ITEM_MONEY:
      if (!pObjIndex->new_format)
	obj->value[0] = obj->cost;
      break;
    }

  for (paf = pObjIndex->affect_first; paf != NULL; paf = paf->next)
    if (paf->location == APPLY_SPELL_AFFECT)
      affect_to_obj (obj, paf);

  Link (obj, obj, next, prev);
  pObjIndex->count++;

  return obj;
}


void
clone_object (ObjData * parent, ObjData * clone)
{
  int i;
  AffectData *paf;
  ExDescrData *ed, *ed_new;

  if (parent == NULL || clone == NULL)
    return;


  replace_str (&clone->name, parent->name);
  replace_str (&clone->short_descr, parent->short_descr);
  replace_str (&clone->description, parent->description);
  clone->item_type = parent->item_type;
  clone->extra_flags = parent->extra_flags;
  clone->wear_flags = parent->wear_flags;
  clone->weight = parent->weight;
  clone->cost = parent->cost;
  clone->level = parent->level;
  clone->condition = parent->condition;
  replace_str (&clone->material, parent->material);
  clone->timer = parent->timer;

  for (i = 0; i < 5; i++)
    clone->value[i] = parent->value[i];


  clone->enchanted = parent->enchanted;

  for (paf = parent->affect_first; paf != NULL; paf = paf->next)
    affect_to_obj (clone, paf);


  for (ed = parent->ed_first; ed != NULL; ed = ed->next)
    {
      ed_new = new_ed ();
      ed_new->keyword = str_dup (ed->keyword);
      ed_new->description = str_dup (ed->description);
      Link (ed_new, clone->ed, next, prev);
    }

}


const char *
get_ed (const char *name, ExDescrData * ed)
{
  for (; ed != NULL; ed = ed->next)
    {
      if (is_name (name, ed->keyword))
	return ed->description;
    }
  return NULL;
}


CharIndex *
get_char_index (vnum_t vnum)
{
  CharIndex *pMobIndex;

  for (pMobIndex = char_index_hash[vnum % MAX_KEY_HASH];
       pMobIndex != NULL; pMobIndex = pMobIndex->next)
    {
      if (pMobIndex->vnum == vnum)
	return pMobIndex;
    }

  if (run_level == RUNLEVEL_BOOTING)
    {
      bugf ("Get_char_index: bad vnum %ld.", vnum);
      exit (1);
    }

  return NULL;
}


ObjIndex *
get_obj_index (vnum_t vnum)
{
  ObjIndex *pObjIndex;

  for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
       pObjIndex != NULL; pObjIndex = pObjIndex->next)
    {
      if (pObjIndex->vnum == vnum)
	return pObjIndex;
    }

  if (run_level == RUNLEVEL_BOOTING)
    {
      bugf ("Get_obj_index: bad vnum %ld.", vnum);
      exit (1);
    }

  return NULL;
}


RoomIndex *
get_room_index (vnum_t vnum)
{
  RoomIndex *pRoomIndex;

  for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
       pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
    {
      if (pRoomIndex->vnum == vnum)
	return pRoomIndex;
    }

  if (run_level == RUNLEVEL_BOOTING)
    {
      bugf ("Get_room_index: bad vnum %ld.", vnum);
      exit (1);
    }

  return NULL;
}

ProgCode *
get_prog_index (vnum_t vnum, prog_t type)
{
  ProgCode *prg;

  switch (type)
    {
    case PRG_MPROG:
      prg = mprog_first;
      break;
    case PRG_OPROG:
      prg = oprog_first;
      break;
    case PRG_RPROG:
      prg = rprog_first;
      break;
    default:
      return NULL;
    }

  for (; prg; prg = prg->next)
    {
      if (prg->vnum == vnum)
	return (prg);
    }
  return NULL;
}

flag_t
flag_convert (char letter)
{
  flag_t bitsum = 0;
  char i;

  if ('A' <= letter && letter <= 'Z')
    {
      bitsum = ((flag_t) 1 << 0);
      for (i = letter; i > 'A'; i--)
	bitsum <<= 1;
    }
  else if ('a' <= letter && letter <= 'z')
    {
      bitsum = ((flag_t) 1 << 26);
      for (i = letter; i > 'a'; i--)
	bitsum <<= 1;
    }

  return bitsum;
}


const char *
str_dup (const char *pstr)
{
  if (NullStr (pstr))
    return &str_empty[0];

  sAllocString += strlen (pstr);
  nAllocString += 1;
  return strdup (pstr);
}

const char *
str_dupf (const char *pstr, ...)
{
  va_list args;
  char str_new[MPL];

  if (NullStr (pstr))
    return &str_empty[0];

  va_start (args, pstr);
  sAllocString += vsnprintf (str_new, sizeof (str_new), pstr, args);
  va_end (args);
  nAllocString += 1;
  return strdup (str_new);
}

void
replace_str (const char **pstr, const char *nstr)
{
  free_string (*pstr);

  if (NullStr (nstr))
    *pstr = &str_empty[0];
  else
    {
      sAllocString += strlen (nstr);
      nAllocString += 1;
      *pstr = strdup (nstr);
    }
}
void
replace_strf (const char **pstr, const char *nstr, ...)
{
  va_list args;
  char str_new[MPL];

  free_string (*pstr);

  if (NullStr (nstr))
    {
      *pstr = &str_empty[0];
      return;
    }

  va_start (args, nstr);
  sAllocString += vsnprintf (str_new, sizeof (str_new), nstr, args);
  va_end (args);

  nAllocString += 1;
  *pstr = strdup (str_new);
}


void
free_string (const char *pstr)
{
  if (NullStr (pstr))
    return;

  sAllocString -= strlen (pstr);
  nAllocString -= 1;
  free_mem (pstr);
  return;
}

void
newarea_insert_level_sort (AreaData * a)
{
  AreaData *lsort = area_first_sorted;
  AreaData *lsort_prev = NULL;

  if (!area_first_sorted)
    {
      area_first_sorted = a;
      return;
    }


  for (; lsort; lsort_prev = lsort, lsort = lsort->next_sort)
    {
      if (NullStr (a->lvl_comment))
	{
	  if (NullStr (lsort->lvl_comment))
	    {
	      if (lsort->min_level > 0)
		{
		  if (a->min_level <= 0 || a->min_level > lsort->min_level)
		    {
		      continue;
		    }

		  if (a->min_level == lsort->min_level
		      && a->max_level > lsort->max_level)
		    {
		      continue;
		    }
		}
	      else
		{
		  if (a->min_level < lsort->min_level)
		    {
		      continue;
		    }
		}
	    }
	}
      else
	{
	  int i;

	  if (NullStr (lsort->lvl_comment))
	    {
	      continue;
	    }

	  i = str_cmp (a->lvl_comment, lsort->lvl_comment);
	  if (i > 0)
	    {
	      continue;
	    }

	  if (i == 0 && str_cmp (a->name, lsort->name) > 0)
	    {
	      continue;
	    }

	}


      if (lsort_prev)
	{
	  a->next_sort = lsort;
	  lsort_prev->next_sort = a;
	}
      else
	{

	  a->next_sort = area_first_sorted;
	  area_first_sorted = a;
	}
      return;
    }

  lsort_prev->next_sort = a;
}

void
add_area (AreaData * pArea)
{
  newarea_insert_level_sort (pArea);
  Link (pArea, area, next, prev);
}

void
unlink_area (AreaData * pArea)
{
  UnlinkSingle (pArea, AreaData, area_first_sorted, next_sort);
  UnLink (pArea, area, next, prev);
}

const char *
print_area_levels (AreaData * pArea)
{
  static char buf[8];
  char low[4], high[4];

  if (!NullStr (pArea->lvl_comment))
    return str_align (7, Center, pArea->lvl_comment);

  if (pArea->min_level >= LEVEL_HERO && pArea->max_level >= LEVEL_HERO)
    return str_align (7, Center, "HERO+");

  if (pArea->min_level >= LEVEL_HERO)
    strncpy (low, "HRO", sizeof (low));
  else
    snprintf (low, sizeof (low), "%03d", pArea->min_level);

  if (pArea->max_level >= LEVEL_HERO)
    strncpy (high, "HRO", sizeof (high));
  else
    snprintf (high, sizeof (high), "%03d", pArea->max_level);

  snprintf (buf, sizeof (buf), "%s %s", low, high);
  return buf;
}

Do_Fun (do_areas)
{
  AreaData *pArea1;
  AreaData *pArea2;
  int iArea;
  int iAreaHalf;

  if (!NullStr (argument))
    {
      chprintln (ch, "No argument is used with this command.");
      return;
    }

  iAreaHalf = (top_area + 1) / 2;
  pArea1 = area_first_sorted;
  pArea2 = area_first_sorted;
  for (iArea = 0; iArea < iAreaHalf; iArea++)
    pArea2 = pArea2->next_sort;

  for (iArea = 0; iArea < iAreaHalf; iArea++)
    {
      chprintf (ch, "[%-7.7s] %-7.7s %-30.30s ", print_area_levels (pArea1),
		pArea1->credits, pArea1->name);

      pArea1 = pArea1->next_sort;
      if (pArea2 != NULL)
	{
	  chprintlnf (ch, "[%-7.7s] %-7.7s %-30.30s",
		      print_area_levels (pArea2), pArea2->credits,
		      pArea2->name);

	  pArea2 = pArea2->next_sort;
	}
      else
	chprintln (ch, NULL);
    }

  return;
}


int
number_fuzzy (int number)
{
  switch (number_bits (2))
    {
    case 0:
      number -= 1;
      break;
    case 3:
      number += 1;
      break;
    }

  return Max (1, number);
}

int
number_fuzzier (int number)
{
  switch (number_bits (2))
    {
    case 0:
      number -= 2;
      break;
    case 1:
      number -= 1;
      break;
    case 3:
      number += 1;
      break;
    case 4:
      number += 2;
      break;
    }

  return Max (1, number);
}


int
number_range (int from, int to)
{
  int power;
  int number;

  if (from == 0 && to == 0)
    return 0;

  if ((to = to - from + 1) <= 1)
    return from;

  for (power = 2; power < to; power <<= 1)
    ;

  while ((number = number_mm () & (power - 1)) >= to)
    ;

  return from + number;
}


int
number_percent (void)
{
  int percent;

  while ((percent = number_mm () & (128 - 1)) > 99)
    ;

  return 1 + percent;
}

int
level_percent (void)
{
  return number_range (1, MAX_MORTAL_LEVEL);
}


int
number_door (void)
{
  int door;

  while ((door = number_mm () & (8 - 1)) > 5)
    ;

  return door;
}

int
number_bits (int width)
{
  return number_mm () & ((1 << width) - 1);
}





#ifdef OLD_RAND
static int rgiState[2 + 55];
#endif

void
init_mm ()
{
  log_string ("Starting random number generator...");
#ifdef OLD_RAND

  int *piState;
  int iState;

  piState = &rgiState[2];

  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;

  piState[0] = ((int) current_time) & ((1 << 30) - 1);
  piState[1] = 1;
  for (iState = 2; iState < 55; iState++)
    {
      piState[iState] =
	(piState[iState - 1] + piState[iState - 2]) & ((1 << 30) - 1);
    }
#else
  srandom (time (NULL) ^ getpid ());
#endif

  return;
}

long
number_mm (void)
{
#ifdef OLD_RAND
  int *piState;
  int iState1;
  int iState2;
  int iRand;

  piState = &rgiState[2];
  iState1 = piState[-2];
  iState2 = piState[-1];
  iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
  piState[iState1] = iRand;
  if (++iState1 == 55)
    iState1 = 0;
  if (++iState2 == 55)
    iState2 = 0;
  piState[-2] = iState1;
  piState[-1] = iState2;
  return iRand >> 6;
#else

  return random () >> 6;
#endif
}


int
dice (int number, int size)
{
  int idice;
  int sum;

  switch (size)
    {
    case 0:
      return 0;
    case 1:
      return number;
    }

  for (idice = 0, sum = 0; idice < number; idice++)
    sum += number_range (1, size);

  return sum;
}


int
interpolate (int level, int value_00, int value_32)
{
  return value_00 + level * (value_32 - value_00) / 32;
}


int
str_cmp (const char *astr, const char *bstr)
{
  if (astr == NULL)
    return bstr == NULL ? 0 : -1;
  if (bstr == NULL)
    return 1;
  return strcasecmp (astr, bstr);
}


int
str_casecmp (const char *astr, const char *bstr)
{
  if (astr == NULL)
    return bstr == NULL ? 0 : -1;
  if (bstr == NULL)
    return 1;
  return strcmp (astr, bstr);
}


int
str_ncmp (const char *astr, const char *bstr, size_t len)
{
  if (astr == NULL)
    return bstr == NULL ? 0 : -1;
  if (bstr == NULL)
    return 1;
  return strncasecmp (astr, bstr, len);
}


int
str_ncasecmp (const char *astr, const char *bstr, size_t len)
{
  if (astr == NULL)
    return bstr == NULL ? 0 : -1;
  if (bstr == NULL)
    return 1;
  return strncmp (astr, bstr, len);
}


bool
str_prefix (const char *astr, const char *bstr)
{
  if (astr == NULL)
    {
      bug ("Str_prefix: null astr.");
      return true;
    }

  if (bstr == NULL)
    {
      bug ("Str_prefix: null bstr.");
      return true;
    }

  for (; *astr; astr++, bstr++)
    {
      if (tolower (*astr) != tolower (*bstr))
	return true;
    }

  return false;
}


bool
str_infix (const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ((c0 = tolower (astr[0])) == '\0')
    return false;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);

  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
    {
      if (c0 == tolower (bstr[ichar]) && !str_prefix (astr, bstr + ichar))
	return false;
    }

  return true;
}


bool
str_suffix (const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);
  if (sstr1 <= sstr2 && !str_cmp (astr, bstr + sstr2 - sstr1))
    return false;
  else
    return true;
}


const char *
Upper (const char *str)
{
  size_t i;
  static char up[3][MSL * 2];
  static int u;

  if (NullStr (str))
    return "";

  u++;
  u %= 3;

  strcpy (up[u], str);
  i = skipcol (up[u]);
  up[u][i] = toupper (up[u][i]);
  return up[u];
}


const char *
Lower (const char *str)
{
  size_t i;
  static char low[3][MSL * 2];
  static int l;

  if (NullStr (str))
    return "";

  l++;
  l %= 3;
  strcpy (low[l], str);
  i = skipcol (low[l]);
  low[l][i] = tolower (low[l][i]);
  return low[l];
}


char *
capitalize (const char *str)
{
  static char result[5][MSL * 5];
  static int i;
  size_t c;

  if (NullStr (str))
    return "";

  i++;
  i %= 5;

  strcpy (result[i], strlower (str));

  c = skipcol (result[i]);
  result[i][c] = toupper (result[i][c]);
  return result[i];
}


char *
strlower (const char *str)
{
  static char strlow[5][MSL * 5];
  static int s;
  char *result;
  size_t i;

  if (NullStr (str))
    return "";

  s++;
  s %= 5;
  result = strlow[s];


  for (i = 0; str[i] != NUL; i++)
    {
      if (str[i] == COLORCODE)
	{
	  int k = ansi_skip (&str[i]);

	  while (k-- > 0)
	    result[i] = str[i++];
	}
      else if (str[i] == CUSTOMSTART)
	{
	  do
	    {
	      result[i] = str[i];
	      i++;
	    }
	  while (str[i] != NUL && str[i] != CUSTOMEND);
	}
      else if (str[i] == MXP_BEGc)
	{
	  do
	    {
	      result[i] = str[i];
	      i++;
	    }
	  while (str[i] != NUL && str[i] != MXP_ENDc);
	}
      else
	result[i] = isalpha (str[i]) ? tolower (str[i]) : str[i];

    }
  result[i] = NUL;
  return result;
}


char *
strupper (const char *str)
{
  static char strup[5][MSL * 5];
  static int s;
  char *result;
  size_t i;

  if (NullStr (str))
    return "";

  s++;
  s %= 5;
  result = strup[s];

  for (i = 0; str[i] != NUL; i++)
    {
      if (str[i] == COLORCODE)
	{
	  int k = ansi_skip (&str[i]);

	  while (k-- > 0)
	    result[i] = str[i++];
	}
      else if (str[i] == CUSTOMSTART)
	{
	  do
	    {
	      result[i] = str[i];
	      i++;
	    }
	  while (str[i] != NUL && str[i] != CUSTOMEND);
	}
      else if (str[i] == MXP_BEGc)
	{
	  do
	    {
	      result[i] = str[i];
	      i++;
	    }
	  while (str[i] != NUL && str[i] != MXP_ENDc);

	}
      else
	result[i] = isalpha (str[i]) ? toupper (str[i]) : str[i];
    }
  result[i] = NUL;
  return result;
}


bool
isavowel (char letter)
{
  char c;

  c = tolower (letter);
  if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
    return true;
  else
    return false;
}


char *
aoran (const char *str)
{
  static char temp[MAX_STRING_LENGTH];

  if (NullStr (str))
    return "";

  if (isavowel (str[0])
      || (strlen (str) > 1 && tolower (str[0]) == 'y' && !isavowel (str[1])))
    strcpy (temp, "an ");
  else
    strcpy (temp, "a ");
  strcat (temp, str);
  return temp;
}


void
append_file (const char *file, const char *str, bool newline)
{
  FILE *fp;

  if (NullStr (str) || NullStr (file))
    return;

  if ((fp = file_open (file, "a")) == NULL)
    {
      log_error (file);
    }
  else
    {
      if (newline)
	fprintf (fp, "%s" LF, str);
      else
	fprintf (fp, "%s", str);
    }
  file_close (fp);
}


void
bug (const char *str)
{
  char buf[MAX_STRING_LENGTH];

  if (fpArea != NULL)
    {
      size_t iLine, iChar;
      char c;

      iChar = f_tell (fpArea);
      f_seek (fpArea, 0, SEEK_SET);
      for (iLine = 0; (size_t) f_tell (fpArea) < iChar; iLine++)
	{
	  while ((c = f_getc (fpArea)) != '\n' && c != EOF)
	    ;
	}
      f_seek (fpArea, iChar, SEEK_SET);

      logf ("[*****] FILE: %s LINE: %d", strArea, iLine);
    }

  sprintf (buf, "[*****] BUG: %s", str);
  if (run_level == RUNLEVEL_MAIN_LOOP)
    {

      wiznet (buf, NULL, NULL, WIZ_BUGS, true, 0);
    }
  else
    log_string (buf);
  return;
}


void
tail_chain (void)
{
  return;
}

char *
casemix (const char *str)
{
  static char out[4][MSL];
  static int o;
  char *result;
  size_t i;
  bool wascap = false;

  if (NullStr (str))
    return "";

  ++o, o %= 4;
  result = out[o];

  for (i = 0; str[i] != NUL; i++)
    {
      if (isalpha (str[i]))
	{
	  if (wascap)
	    result[i] = tolower (str[i]);
	  else
	    result[i] = toupper (str[i]);

	  wascap = !wascap;
	}
      else
	{
	  result[i] = str[i];
	}
    }
  result[i] = NUL;
  return (result);
}
