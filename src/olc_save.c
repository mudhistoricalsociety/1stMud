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
#include "interp.h"
#include "special.h"
#include "data_table.h"
#include "recycle.h"

#define DIF(a,b) (~((~a)|(b)))




char *
fix_string (const char *str)
{
  static char strfix[MAX_STRING_LENGTH * 2];
  int i;
  int o;

  if (str == NULL || !str_cmp (str, "(null)"))
    return '\0';

  for (o = i = 0; str[i + o] != '\0'; i++)
    {
      if (str[i + o] == '\r' || str[i + o] == '~')
	o++;
      strfix[i] = str[i + o];
    }
  strfix[i] = '\0';
  return strfix;
}


void
save_area_list (void)
{
  FileData *fp;
  AreaData *pArea;

  if ((fp = f_open (AREA_LIST, "w")) == NULL)
    {
      bug ("Save_area_list: file open");
      log_error ("area.lst");
    }
  else
    {
      for (pArea = area_first; pArea; pArea = pArea->next)
	{
	  f_printf (fp, "%s" LF, pArea->file_name);
	}

      f_printf (fp, "$" LF);
      f_close (fp);
    }

  return;
}

#define    NBUF 5
#define    NBITS 52

char *
write_flags (flag_t flags)
{
  static int cnt;
  static char buf[NBUF][NBITS + 1];
  flag_t count = 0, temp = 1;

  cnt = (cnt + 1) % NBUF;

  buf[cnt][0] = '+';

  do
    {
      if (IsSet (flags, (temp << count)))
	buf[cnt][count + 1] = 'Y';
      else
	buf[cnt][count + 1] = 'n';
      count++;
    }
  while ((temp << count) <= flags && count < 64);

  buf[cnt][count + 1] = '\0';
  return buf[cnt];
}

void
save_mobprogs (FileData * fp, AreaData * pArea)
{
  ProgCode *pMprog;
  vnum_t i;

  f_printf (fp, "#MOBPROGS" LF);

  for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
    {
      if ((pMprog = get_prog_index (i, PRG_MPROG)) != NULL)
	{
	  f_printf (fp, "#%ld" LF, i);
	  f_printf (fp, "%s~" LF, fix_string (pMprog->code));
	}
    }

  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}

void
save_objprogs (FileData * fp, AreaData * pArea)
{
  ProgCode *pOprog;
  vnum_t i;

  f_printf (fp, "#OBJPROGS" LF);

  for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
    {
      if ((pOprog = get_prog_index (i, PRG_OPROG)) != NULL)
	{
	  f_printf (fp, "#%ld" LF, i);
	  f_printf (fp, "%s~" LF, fix_string (pOprog->code));
	}
    }

  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}

void
save_roomprogs (FileData * fp, AreaData * pArea)
{
  ProgCode *pRprog;
  vnum_t i;

  f_printf (fp, "#ROOMPROGS" LF);

  for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
    {
      if ((pRprog = get_prog_index (i, PRG_RPROG)) != NULL)
	{
	  f_printf (fp, "#%ld" LF, i);
	  f_printf (fp, "%s~" LF, fix_string (pRprog->code));
	}
    }

  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}


void
save_mobile (FileData * fp, CharIndex * pMobIndex)
{
  RaceData *race = pMobIndex->race;
  ProgList *pMprog;
  flag_t temp;

  f_printf (fp, "#%ld" LF, pMobIndex->vnum);
  f_printf (fp, "%s~" LF, pMobIndex->player_name);
  f_printf (fp, "%s~" LF, pMobIndex->short_descr);
  f_printf (fp, "%s~" LF, Upper (fix_string (pMobIndex->long_descr)));
  f_printf (fp, "%s~" LF, Upper (fix_string (pMobIndex->description)));
  f_printf (fp, "%s~" LF, race->name);
  f_printf (fp, "%s ", write_flags (pMobIndex->act));
  f_printf (fp, "%s ", write_flags (pMobIndex->affected_by));
  f_printf (fp, "%d %ld" LF, pMobIndex->alignment, pMobIndex->group);
  f_printf (fp, "%d ", pMobIndex->level);
  f_printf (fp, "%d %d ", pMobIndex->random, pMobIndex->autoset);
  f_printf (fp, "%d" LF, pMobIndex->hitroll);
  f_printf (fp, "%dd%d+%d ", pMobIndex->hit[DICE_NUMBER],
	    pMobIndex->hit[DICE_TYPE], pMobIndex->hit[DICE_BONUS]);
  f_printf (fp, "%dd%d+%d ", pMobIndex->mana[DICE_NUMBER],
	    pMobIndex->mana[DICE_TYPE], pMobIndex->mana[DICE_BONUS]);
  f_printf (fp, "%dd%d+%d ", pMobIndex->damage[DICE_NUMBER],
	    pMobIndex->damage[DICE_TYPE], pMobIndex->damage[DICE_BONUS]);
  f_printf (fp, "'%s'" LF, attack_table[pMobIndex->dam_type].name);
  f_printf (fp, "%d %d %d %d" LF, pMobIndex->ac[AC_PIERCE] / 10,
	    pMobIndex->ac[AC_BASH] / 10, pMobIndex->ac[AC_SLASH] / 10,
	    pMobIndex->ac[AC_EXOTIC] / 10);
  f_printf (fp, "%s ", write_flags (pMobIndex->off_flags));
  f_printf (fp, "%s ", write_flags (pMobIndex->imm_flags));
  f_printf (fp, "%s ", write_flags (pMobIndex->res_flags));
  f_printf (fp, "%s" LF, write_flags (pMobIndex->vuln_flags));
  f_printf (fp, "%s %s %s %ld" LF,
	    flag_string (position_flags, pMobIndex->start_pos),
	    flag_string (position_flags, pMobIndex->default_pos),
	    flag_string (sex_flags, pMobIndex->sex), pMobIndex->wealth);
  f_printf (fp, "%s ", write_flags (pMobIndex->form));
  f_printf (fp, "%s ", write_flags (pMobIndex->parts));

  f_printf (fp, "%s ", flag_string (size_flags, pMobIndex->size));
  f_printf (fp, "'%s'" LF, GetStr (pMobIndex->material, "unknown"));

  if ((temp = DIF (race->act, pMobIndex->act)))
    f_printf (fp, "F act %s" LF, write_flags (temp));

  if ((temp = DIF (race->aff, pMobIndex->affected_by)))
    f_printf (fp, "F aff %s" LF, write_flags (temp));

  if ((temp = DIF (race->off, pMobIndex->off_flags)))
    f_printf (fp, "F off %s" LF, write_flags (temp));

  if ((temp = DIF (race->imm, pMobIndex->imm_flags)))
    f_printf (fp, "F imm %s" LF, write_flags (temp));

  if ((temp = DIF (race->res, pMobIndex->res_flags)))
    f_printf (fp, "F res %s" LF, write_flags (temp));

  if ((temp = DIF (race->vuln, pMobIndex->vuln_flags)))
    f_printf (fp, "F vul %s" LF, write_flags (temp));

  if ((temp = DIF (race->form, pMobIndex->form)))
    f_printf (fp, "F for %s" LF, write_flags (temp));

  if ((temp = DIF (race->parts, pMobIndex->parts)))
    f_printf (fp, "F par %s" LF, write_flags (temp));

  for (pMprog = pMobIndex->mprog_first; pMprog; pMprog = pMprog->next)
    {
      f_printf (fp, "M %s %ld %s~" LF,
		prog_type_to_name (pMprog->trig_type), pMprog->prog->vnum,
		pMprog->trig_phrase);
    }

  if (pMobIndex->kills > 2 || pMobIndex->deaths > 2)
    f_printf (fp, "S %ld %ld" LF, pMobIndex->kills, pMobIndex->deaths);

  return;
}


void
save_mobiles (FileData * fp, AreaData * pArea)
{
  vnum_t i;
  CharIndex *pMob;

  f_printf (fp, "#MOBILES" LF);

  for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
    {
      if ((pMob = get_char_index (i)))
	save_mobile (fp, pMob);
    }

  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}


void
save_object (FileData * fp, ObjIndex * pObjIndex)
{
  char letter;
  AffectData *pAf;
  ExDescrData *pEd;
  ProgList *pOprog;

  f_printf (fp, "#%ld" LF, pObjIndex->vnum);
  f_printf (fp, "%s~" LF, pObjIndex->name);
  f_printf (fp, "%s~" LF, pObjIndex->short_descr);
  f_printf (fp, "%s~" LF, fix_string (pObjIndex->description));
  f_printf (fp, "%s~" LF, pObjIndex->material);
  f_printf (fp, "%s ", flag_string (type_flags, pObjIndex->item_type));
  f_printf (fp, "%s ", write_flags (pObjIndex->extra_flags));
  f_printf (fp, "%s" LF, write_flags (pObjIndex->wear_flags));



  switch (pObjIndex->item_type)
    {
    default:
      f_printf (fp, "%s ", write_flags (pObjIndex->value[0]));
      f_printf (fp, "%s ", write_flags (pObjIndex->value[1]));
      f_printf (fp, "%s ", write_flags (pObjIndex->value[2]));
      f_printf (fp, "%s ", write_flags (pObjIndex->value[3]));
      f_printf (fp, "%s" LF, write_flags (pObjIndex->value[4]));
      break;

    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
      f_printf (fp, "%ld %ld '%s' %ld %ld" LF, pObjIndex->value[0],
		pObjIndex->value[1],
		liq_table[pObjIndex->value[2]].liq_name,
		pObjIndex->value[3], pObjIndex->value[4]);
      break;

    case ITEM_CONTAINER:
      f_printf (fp, "%ld %s %ld %ld %ld" LF, pObjIndex->value[0],
		write_flags (pObjIndex->value[1]),
		pObjIndex->value[2], pObjIndex->value[3],
		pObjIndex->value[4]);
      break;

    case ITEM_WEAPON:
      f_printf (fp, "%s %ld %ld %s %s" LF,
		weapon_name (pObjIndex->value[0]),
		pObjIndex->value[1], pObjIndex->value[2],
		attack_table[pObjIndex->value[3]].name,
		write_flags (pObjIndex->value[4]));
      break;

    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SCROLL:
      f_printf (fp, "%ld '%s' '%s' '%s' '%s'" LF, pObjIndex->value[0] > 0 ?
		pObjIndex->value[0] : 0,
		pObjIndex->value[1] !=
		-1 ? skill_table[pObjIndex->value[1]].name : "",
		pObjIndex->value[2] !=
		-1 ? skill_table[pObjIndex->value[2]].name : "",
		pObjIndex->value[3] !=
		-1 ? skill_table[pObjIndex->value[3]].name : "",
		pObjIndex->value[4] !=
		-1 ? skill_table[pObjIndex->value[4]].name : "");
      break;

    case ITEM_STAFF:
    case ITEM_WAND:
      f_printf (fp, "%ld %ld %ld '%s' %ld" LF, pObjIndex->value[0],
		pObjIndex->value[1], pObjIndex->value[2],
		pObjIndex->value[3] !=
		-1 ? skill_table[pObjIndex->value[3]].name : "",
		pObjIndex->value[4]);
      break;
    }

  f_printf (fp, "%d %d %ld ", pObjIndex->level, pObjIndex->weight,
	    pObjIndex->cost);

  if (pObjIndex->condition > 90)
    letter = 'P';
  else if (pObjIndex->condition > 75)
    letter = 'G';
  else if (pObjIndex->condition > 50)
    letter = 'A';
  else if (pObjIndex->condition > 25)
    letter = 'W';
  else if (pObjIndex->condition > 10)
    letter = 'D';
  else if (pObjIndex->condition > 0)
    letter = 'B';
  else
    letter = 'R';

  f_printf (fp, "%c" LF, letter);

  for (pAf = pObjIndex->affect_first; pAf; pAf = pAf->next)
    {
      if (pAf->where == TO_OBJECT || pAf->bitvector == 0)
	f_printf (fp, "A" LF "%d %d" LF, pAf->location, pAf->modifier);
      else
	{
	  f_printf (fp, "F" LF);

	  switch (pAf->where)
	    {
	    case TO_AFFECTS:
	      letter = 'A';
	      break;
	    case TO_IMMUNE:
	      letter = 'I';
	      break;
	    case TO_RESIST:
	      letter = 'R';
	      break;
	    case TO_VULN:
	      letter = 'V';
	      break;
	    default:
	      bug ("olc_save: Invalid Affect->where");
	      letter = ' ';
	      break;
	    }

	  f_printf (fp, "%c %d %d %s" LF, letter, pAf->location,
		    pAf->modifier, write_flags (pAf->bitvector));
	}
    }

  for (pEd = pObjIndex->ed_first; pEd; pEd = pEd->next)
    {
      f_printf (fp, "E" LF "%s~" LF "%s~" LF, pEd->keyword,
		fix_string (pEd->description));
    }
  for (pOprog = pObjIndex->oprog_first; pOprog; pOprog = pOprog->next)
    {
      f_printf (fp, "O %s %ld %s~" LF,
		prog_type_to_name (pOprog->trig_type), pOprog->prog->vnum,
		pOprog->trig_phrase);
    }

  return;
}


void
save_objects (FileData * fp, AreaData * pArea)
{
  vnum_t i;
  ObjIndex *pObj;

  f_printf (fp, "#OBJECTS" LF);

  for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
    {
      if ((pObj = get_obj_index (i)))
	save_object (fp, pObj);
    }

  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}


void
save_rooms (FileData * fp, AreaData * pArea)
{
  RoomIndex *pRoomIndex;
  ExDescrData *pEd;
  ExitData *pExit;
  int iHash;
  int door;
  ProgList *pRprog;

  f_printf (fp, "#ROOMS" LF);
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
	   pRoomIndex = pRoomIndex->next)
	{
	  if (pRoomIndex->area == pArea)
	    {
	      f_printf (fp, "#%ld" LF, pRoomIndex->vnum);
	      f_printf (fp, "%s~" LF, pRoomIndex->name);
	      f_printf (fp, "%s~" LF, fix_string (pRoomIndex->description));
	      f_printf (fp, "0 %s %d" LF,
			write_flags (pRoomIndex->room_flags),
			pRoomIndex->sector_type);

	      for (pEd = pRoomIndex->ed_first; pEd; pEd = pEd->next)
		{
		  f_printf (fp, "E" LF "%s~" LF "%s~" LF,
			    pEd->keyword, fix_string (pEd->description));
		}
	      for (door = 0; door < MAX_DIR; door++)
		{
		  if ((pExit = pRoomIndex->exit[door])
		      && pExit->u1.to_room
		      && !IsSet (pExit->rs_flags, EX_TEMP))
		    {

		      if (IsSet (pExit->rs_flags, EX_CLOSED)
			  || IsSet (pExit->rs_flags, EX_LOCKED)
			  || IsSet (pExit->rs_flags, EX_PICKPROOF)
			  || IsSet (pExit->rs_flags, EX_NOPASS)
			  || IsSet (pExit->rs_flags, EX_EASY)
			  || IsSet (pExit->rs_flags, EX_HARD)
			  || IsSet (pExit->rs_flags, EX_INFURIATING)
			  || IsSet (pExit->rs_flags, EX_NOCLOSE)
			  || IsSet (pExit->rs_flags, EX_NOLOCK))
			SetBit (pExit->rs_flags, EX_ISDOOR);
		      else
			RemBit (pExit->rs_flags, EX_ISDOOR);

		      f_printf (fp, "D%d" LF, pExit->orig_door);
		      f_printf (fp, "%s~" LF,
				fix_string (pExit->description));
		      f_printf (fp, "%s~" LF, pExit->keyword);
		      f_printf (fp, "%s %ld %ld" LF,
				write_flags (pExit->rs_flags), pExit->key,
				pExit->u1.to_room->vnum);
		    }
		}
	      if (pRoomIndex->mana_rate != 100 ||
		  pRoomIndex->heal_rate != 100)
		f_printf (fp, "M %d H %d" LF,
			  pRoomIndex->mana_rate, pRoomIndex->heal_rate);

	      if (!NullStr (pRoomIndex->owner))
		f_printf (fp, "O %s~" LF, pRoomIndex->owner);
	      if (pRoomIndex->guild > -1 && pRoomIndex->guild < top_class)
		f_printf (fp, "G %d" LF, pRoomIndex->guild);
	      for (pRprog = pRoomIndex->rprog_first; pRprog;
		   pRprog = pRprog->next)
		{
		  f_printf (fp, "R %s %ld %s~" LF,
			    prog_type_to_name (pRprog->trig_type),
			    pRprog->prog->vnum, pRprog->trig_phrase);
		}
	      f_printf (fp, "S" LF);
	    }
	}
    }
  f_printf (fp, "#0" LF);
  f_printf (fp, "" LF);
  return;
}


void
save_specials (FileData * fp, AreaData * pArea)
{
  int iHash;
  CharIndex *pMobIndex;

  f_printf (fp, "#SPECIALS" LF);

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = char_index_hash[iHash]; pMobIndex;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun)
	    {
	      if (IsSet (mud_info.mud_flags, VERBOSE_RESETS))
		f_printf (fp, "M %ld %s  * Load to: %s" LF,
			  pMobIndex->vnum,
			  spec_name (pMobIndex->spec_fun),
			  pMobIndex->short_descr);
	      else
		f_printf (fp, "M %ld %s" LF, pMobIndex->vnum,
			  spec_name (pMobIndex->spec_fun));
	    }
	}
    }

  f_printf (fp, "S" LF);
  f_printf (fp, "" LF);
  return;
}


void
save_door_resets (FileData * fp, AreaData * pArea)
{
  int iHash;
  RoomIndex *pRoomIndex;
  ExitData *pExit;
  int door;

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
	   pRoomIndex = pRoomIndex->next)
	{
	  if (pRoomIndex->area == pArea)
	    {
	      for (door = 0; door < MAX_DIR; door++)
		{
		  if ((pExit = pRoomIndex->exit[door]) &&
		      pExit->u1.to_room
		      && !IsSet (pExit->rs_flags, EX_TEMP)
		      && (IsSet (pExit->rs_flags, EX_CLOSED)
			  || IsSet (pExit->rs_flags, EX_LOCKED)))
		    {
		      if (IsSet (mud_info.mud_flags, VERBOSE_RESETS))
			f_printf (fp,
				  "F 0 %ld %d 0 %s  * The %s door of %s (%s)."
				  LF, pRoomIndex->vnum,
				  pExit->orig_door,
				  write_flags (pExit->rs_flags),
				  dir_name[pExit->orig_door],
				  pRoomIndex->name,
				  flag_string (exit_flags, pExit->rs_flags));
		      else
			f_printf (fp, "F 0 %ld %d 0 %s" LF,
				  pRoomIndex->vnum, pExit->orig_door,
				  write_flags (pExit->rs_flags));
		    }
		}
	    }
	}
    }
  return;
}


void
save_resets (FileData * fp, AreaData * pArea)
{
  ResetData *pReset;
  CharIndex *pLastMob = NULL;
  ObjIndex *pLastObj;
  RoomIndex *pRoom;
  int iHash;

  f_printf (fp, "#RESETS" LF);

  save_door_resets (fp, pArea);

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next)
	{
	  if (pRoom->area == pArea)
	    {
	      for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
		{
		  if (IsSet (mud_info.mud_flags, VERBOSE_RESETS))
		    switch (pReset->command)
		      {
		      default:
			bugf ("Save_resets: bad command %c.",
			      pReset->command);
			break;

		      case 'M':
			pLastMob = get_char_index (pReset->arg1);
			f_printf (fp,
				  "M 0 %ld %d %ld %d  * Load %s" LF,
				  pReset->arg1,
				  pReset->arg2,
				  pReset->arg3,
				  pReset->arg4, pLastMob->short_descr);
			break;

		      case 'O':
			pLastObj = get_obj_index (pReset->arg1);
			pRoom = get_room_index (pReset->arg3);
			f_printf (fp,
				  "O 0 %ld 0 %ld  * %s loaded to %s" LF,
				  pReset->arg1,
				  pReset->arg3,
				  capitalize (pLastObj->short_descr),
				  pRoom->name);
			break;

		      case 'P':
			pLastObj = get_obj_index (pReset->arg1);
			f_printf (fp,
				  "P 0 %ld %d %ld %d  * %s put inside %s"
				  LF, pReset->arg1, pReset->arg2,
				  pReset->arg3, pReset->arg4,
				  capitalize (get_obj_index
					      (pReset->arg1)->short_descr),
				  pLastObj->short_descr);
			break;

		      case 'G':
			f_printf (fp,
				  "G 0 %ld 0  * %s is given to %s" LF,
				  pReset->arg1,
				  capitalize (get_obj_index
					      (pReset->arg1)->short_descr),
				  pLastMob ? pLastMob->short_descr :
				  "!NO_MOB!");
			if (!pLastMob)
			  {
			    bugf ("Save_resets: !NO_MOB! in [%s]",
				  pArea->file_name);
			  }
			break;

		      case 'E':
			f_printf (fp,
				  "E 0 %ld 0 %ld  * %s is loaded %s of %s"
				  LF, pReset->arg1, pReset->arg3,
				  capitalize (get_obj_index
					      (pReset->arg1)->short_descr),
				  flag_string (wear_loc_strings,
					       pReset->arg3),
				  pLastMob ? pLastMob->short_descr :
				  "!NO_MOB!");
			if (!pLastMob)
			  {
			    bugf ("Save_resets: !NO_MOB! in [%s]",
				  pArea->file_name);
			  }
			break;

		      case 'D':
			break;

		      case 'R':
			pRoom = get_room_index (pReset->arg1);
			f_printf (fp,
				  "R 0 %ld %d %ld  * Randomize %s" LF,
				  pReset->arg1, pReset->arg2,
				  pReset->arg3, pRoom->name);
			break;
		      }
		  else
		    switch (pReset->command)
		      {
		      default:
			bugf ("Save_resets: bad command %c.",
			      pReset->command);
			break;

		      case 'M':
			pLastMob = get_char_index (pReset->arg1);
			f_printf (fp,
				  "M 0 %ld %d %ld %d" LF,
				  pReset->arg1,
				  pReset->arg2, pReset->arg3, pReset->arg4);
			break;

		      case 'O':
			pLastObj = get_obj_index (pReset->arg1);
			pRoom = get_room_index (pReset->arg3);
			f_printf (fp, "O 0 %ld 0 %ld" LF,
				  pReset->arg1, pReset->arg3);
			break;

		      case 'P':
			pLastObj = get_obj_index (pReset->arg1);
			f_printf (fp,
				  "P 0 %ld %d %ld %d" LF,
				  pReset->arg1,
				  pReset->arg2, pReset->arg3, pReset->arg4);
			break;

		      case 'G':
			f_printf (fp, "G 0 %ld 0" LF, pReset->arg1);
			if (!pLastMob)
			  {
			    bugf ("Save_resets: !NO_MOB! in [%s]",
				  pArea->file_name);
			  }
			break;

		      case 'E':
			f_printf (fp, "E 0 %ld 0 %ld" LF,
				  pReset->arg1, pReset->arg3);
			if (!pLastMob)
			  {
			    bugf ("Save_resets: !NO_MOB! in [%s]",
				  pArea->file_name);
			  }
			break;

		      case 'D':
			break;

		      case 'R':
			pRoom = get_room_index (pReset->arg1);
			f_printf (fp, "R 0 %ld %d %ld" LF, pReset->arg1,
				  pReset->arg2, pReset->arg3);
			break;
		      }
		}
	    }
	}
    }
  f_printf (fp, "S" LF LF);
  return;
}


void
save_shops (FileData * fp, AreaData * pArea)
{
  ShopData *pShopIndex;
  CharIndex *pMobIndex;
  int iTrade;
  int iHash;

  f_printf (fp, "#SHOPS" LF);

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = char_index_hash[iHash]; pMobIndex;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop)
	    {
	      pShopIndex = pMobIndex->pShop;

	      f_printf (fp, "%ld ", pShopIndex->keeper);
	      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
		{
		  if (pShopIndex->buy_type[iTrade] != 0)
		    {
		      f_printf (fp, "%d ", pShopIndex->buy_type[iTrade]);
		    }
		  else
		    f_printf (fp, "0 ");
		}
	      f_printf (fp, "%d %d %d %d" LF, pShopIndex->profit_buy,
			pShopIndex->profit_sell, pShopIndex->open_hour,
			pShopIndex->close_hour);
	    }
	}
    }

  f_printf (fp, "0" LF LF);
  return;
}


void
save_area (AreaData * pArea)
{
  FileData *fp;
  char filename[512];

  sprintf (filename, "%s%s", AREA_DIR, pArea->file_name);

  if (!(fp = f_open (filename, "w")))
    {
      bug ("Open_area: file open");
      log_error (pArea->file_name);
    }
  else
    {
      RemBit (pArea->area_flags, AREA_CHANGED);
      f_printf (fp, "#AREADATA" LF);
      write_string (fp, "Name", pArea->name, NULL);
      write_string (fp, "Builders", pArea->builders, NULL);
      f_writef (fp, "VNUMs", "%ld %ld" LF, pArea->min_vnum, pArea->max_vnum);
      write_string (fp, "Credits", pArea->credits, NULL);
      write_string (fp, "LvlComment", pArea->lvl_comment, NULL);
      write_string (fp, "ResetMsg", pArea->resetmsg, NULL);
      write_int (fp, "MinLevel", "%d", pArea->min_level, 0);
      write_int (fp, "MaxLevel", "%d", pArea->max_level, MAX_LEVEL);
      write_int (fp, "Version", "%d", AREA_VERSION, 0);
      write_int (fp, "Security", "%d", pArea->security, 0);
      if (pArea->clan != NULL)
	write_string (fp, "Clan", pArea->clan->name, NULL);
      f_writef (fp, "Climate", "%d %d %d" LF,
		pArea->weather.climate_temp,
		pArea->weather.climate_precip, pArea->weather.climate_wind);
      write_int (fp, "Recall", "%ld", pArea->recall, 0);
      write_bit (fp, "Flags", pArea->area_flags, 0);
      write_sound (fp, "Sound", pArea->sound);
      f_writef (fp, "Stats", "%ld %ld", pArea->kills, pArea->deaths);
      f_printf (fp, END_MARK LF);
      f_printf (fp, "" LF);
      f_printf (fp, "" LF);

      save_mobiles (fp, pArea);
      save_objects (fp, pArea);
      save_rooms (fp, pArea);
      save_specials (fp, pArea);
      save_resets (fp, pArea);
      save_shops (fp, pArea);
      save_mobprogs (fp, pArea);
      save_objprogs (fp, pArea);
      save_roomprogs (fp, pArea);

      f_printf (fp, "#$" LF);

      f_close (fp);
    }
  return;
}


Do_Fun (do_asave)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  AreaData *pArea;
  FILE *fp;
  int value, sec;

  fp = NULL;

  sec = !ch ? 9 : IsNPC (ch) ? 0 : ch->pcdata->security;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      if (ch)
	{
	  chprintln (ch, "Usage:");
	  cmd_syntax (ch, "{x", n_fun,
		      "<vnum>     - saves a particular area", NULL);
	  cmd_syntax (ch, "{x", n_fun,
		      "list       - saves the area.lst file", NULL);
	  cmd_syntax (ch, "{x", n_fun,
		      "area       - save the area being edited", NULL);
	  cmd_syntax (ch, "{x", n_fun, "helps      - save help files", NULL);
	  cmd_syntax (ch, "{x", n_fun, "data       - save all data files",
		      NULL);
	  cmd_syntax (ch, "{x", n_fun,
		      "changed    - saves all changed areas", NULL);
	  cmd_syntax (ch, "{x", n_fun,
		      "world      - save the world! (db dump)", NULL);
	  cmd_syntax (ch, "{x", n_fun,
		      "backup     - execute a backup script", NULL);
	  for (value = 0; datasave_table[value].name != NULL; value++)
	    {
	      cmd_syntax (ch, "{x", n_fun,
			  FORMATF ("%-10s - saves %s data",
				   datasave_table[value].name,
				   datasave_table[value].name), NULL);
	    }
	}

      return;
    }



  else if (is_number (arg1))
    {


      value = atoi (arg1);
      if (!(pArea = get_area_data (value)))
	{
	  if (ch)
	    chprintln (ch, "That area does not exist.");
	  return;
	}
      if (ch && !IsBuilder (ch, pArea))
	{
	  chprintln (ch, "You are not a builder for this area.");
	  return;
	}

      save_area_list ();
      save_area (pArea);

      return;
    }



  else if (!str_cmp ("world", arg1))
    {
#ifdef HAVE_WORKING_FORK
      pid_t pid = fork ();

      if (pid == 0)
	{
#endif
	  save_area_list ();
	  for (pArea = area_first; pArea; pArea = pArea->next)
	    {

	      if (ch && !IsBuilder (ch, pArea))
		continue;

	      save_area (pArea);
	      RemBit (pArea->area_flags, AREA_CHANGED);
	    }
	  for (value = 0; datasave_table[value].name != NULL; value++)
	    (*datasave_table[value].fun) (act_write);
#ifdef HAVE_WORKING_FORK

	  _exit (1);
	}
      else if (pid > 0)
	{
#endif
	  if (ch)
	    chprintln (ch, "You saved the world.");
	  log_string ("World saved (db dump).");
#ifdef HAVE_WORKING_FORK

	}
      else
	{
	  chprintln (ch, "Error saving world.");
	  log_error ("save world: fork()");
	}
#endif
      return;
    }
  else if (!str_cmp ("backup", arg1))
    {
      FILE *fp;
      char buf[MIL];
      char p[MPL];

      if ((ch && get_trust (ch) < MAX_LEVEL - 2) || sec < 9)
	{
	  chprintln (ch, "Sorry, not enough security to do that.");
	  return;
	}
#if defined WIN32 || defined CYGWIN
      sprintf (buf, WIN_DIR "backup.bat %s", arg2);
#else
      sprintf (buf, BIN_DIR "backup %s", arg2);
#endif
      if ((fp = popen (buf, "r")) == NULL)
	{
	  chprintlnf (ch, "Error processing %s %s", arg1, arg2);
	  return;
	}

      fread (p, MPL - 1000, 1, fp);
      sendpage (ch, p);
      chprintln (ch, NULL);
      pclose (fp);
      return;
    }

  else if (!str_cmp ("changed", arg1))
    {
      char buf[MAX_INPUT_LENGTH];

      save_area_list ();

      if (ch)
	chprintln (ch, "Saved zones:");
      else
	log_string ("Saved zones:");

      sprintf (buf, "None.");

      for (pArea = area_first; pArea; pArea = pArea->next)
	{

	  if (ch && !IsBuilder (ch, pArea))
	    continue;


	  if (IsSet (pArea->area_flags, AREA_CHANGED))
	    {
	      save_area (pArea);
	      sprintf (buf, "%24s - '%s'", pArea->name, pArea->file_name);
	      if (ch)
		chprintln (ch, buf);
	      else
		log_string (buf);
	      RemBit (pArea->area_flags, AREA_CHANGED);
	    }
	}

      if (!str_cmp (buf, "None."))
	{
	  if (ch)
	    chprintln (ch, buf);
	  else
	    log_string ("None.");
	}
      return;
    }



  else if (!str_cmp (arg1, "list"))
    {
      save_area_list ();
      chprintln (ch, "Area list saved.");
      return;
    }


  else if (!str_cmp (arg1, "area"))
    {
      if (!ch || !ch->desc)
	return;


      if (ch->desc->editor == ED_NONE)
	{
	  chprintln (ch, "You are not editing an area, "
		     "therefore an area vnum is required.");
	  return;
	}


      if ((pArea = get_olc_area (ch->desc)))
	pArea = ch->in_room->area;

      if (!IsBuilder (ch, pArea))
	{
	  chprintln (ch, "You are not a builder for this area.");
	  return;
	}

      save_area_list ();
      save_area (pArea);
      RemBit (pArea->area_flags, AREA_CHANGED);
      chprintln (ch, "Area saved.");
      return;
    }
  else if (!str_cmp (arg1, "data"))
    {
      for (value = 0; datasave_table[value].name != NULL; value++)
	(*datasave_table[value].fun) (act_write);

      chprintln (ch, "All data files saved.");
      return;
    }
  else
    {
      for (value = 0; datasave_table[value].name != NULL; value++)
	{
	  if (!str_cmp (arg1, datasave_table[value].name))
	    {
	      (*datasave_table[value].fun) (act_write);
	      chprintlnf (ch, "%s data saved.",
			  capitalize (datasave_table[value].name));
	      return;
	    }
	}


      if (ch)
	do_function (ch, &do_asave, "");
    }
  return;
}
