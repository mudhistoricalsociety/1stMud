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
#include "tables.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "olc.h"



EXTERN AreaData *current_area;
int nAllocPerm;
long sAllocPerm;


Boot_Fun (load_mobiles)
{
  CharIndex *pMobIndex;

  if (!current_area)
    {
      bug ("Load_mobiles: no #AREA seen yet.");
      exit (1);
    }

  while (true)
    {
      vnum_t vnum;
      char letter;
      int iHash = -1;
      const char *tmp;

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
      pMobIndex->new_format = true;
      newmobs++;
      read_strfree (fp, &pMobIndex->player_name);
      read_strfree (fp, &pMobIndex->short_descr);
      read_strfree (fp, &pMobIndex->long_descr);
      read_strfree (fp, &pMobIndex->description);
      tmp = read_string (fp);
      pMobIndex->race = race_lookup (tmp);
      free_string (tmp);
      if (pMobIndex->race == NULL)
	pMobIndex->race = default_race;

      pMobIndex->act = read_flag (fp) | ACT_IS_NPC | pMobIndex->race->act;
      pMobIndex->affected_by = read_flag (fp) | pMobIndex->race->aff;
      pMobIndex->pShop = NULL;
      pMobIndex->alignment = read_number (fp);
      pMobIndex->group = read_number (fp);

      pMobIndex->level = read_number (fp);

      if (current_area->version >= 4)
	{
	  pMobIndex->random = read_number (fp);
	  pMobIndex->autoset = read_number (fp);
	}

      pMobIndex->hitroll = read_number (fp);


      read_dice (fp, pMobIndex->hit);


      read_dice (fp, pMobIndex->mana);


      read_dice (fp, pMobIndex->damage);

      pMobIndex->dam_type = attack_lookup (read_word (fp));


      pMobIndex->ac[AC_PIERCE] = read_number (fp) * 10;
      pMobIndex->ac[AC_BASH] = read_number (fp) * 10;
      pMobIndex->ac[AC_SLASH] = read_number (fp) * 10;
      pMobIndex->ac[AC_EXOTIC] = read_number (fp) * 10;


      pMobIndex->off_flags = read_flag (fp) | pMobIndex->race->off;
      pMobIndex->imm_flags = read_flag (fp) | pMobIndex->race->imm;
      pMobIndex->res_flags = read_flag (fp) | pMobIndex->race->res;
      pMobIndex->vuln_flags = read_flag (fp) | pMobIndex->race->vuln;


      pMobIndex->start_pos =
	(position_t) flag_value (position_flags, read_word (fp));
      pMobIndex->default_pos =
	(position_t) flag_value (position_flags, read_word (fp));
      pMobIndex->sex = (sex_t) flag_value (sex_flags, read_word (fp));

      pMobIndex->wealth = read_number (fp);

      pMobIndex->form = read_flag (fp) | pMobIndex->race->form;
      pMobIndex->parts = read_flag (fp) | pMobIndex->race->parts;

      CheckPos (pMobIndex->size,
		(size_type) flag_value (size_flags, read_word (fp)), "size");
      pMobIndex->material = str_dup (read_word (fp));

      while (iHash == -1)
	{
	  letter = read_letter (fp);

	  switch (letter)
	    {
	    case 'F':
	      {
		char *word;
		flag_t vector;

		word = read_word (fp);
		vector = read_flag (fp);

		if (!str_prefix (word, "act"))
		  RemBit (pMobIndex->act, vector);
		else if (!str_prefix (word, "aff"))
		  RemBit (pMobIndex->affected_by, vector);
		else if (!str_prefix (word, "off"))
		  RemBit (pMobIndex->off_flags, vector);
		else if (!str_prefix (word, "imm"))
		  RemBit (pMobIndex->imm_flags, vector);
		else if (!str_prefix (word, "res"))
		  RemBit (pMobIndex->res_flags, vector);
		else if (!str_prefix (word, "vul"))
		  RemBit (pMobIndex->vuln_flags, vector);
		else if (!str_prefix (word, "for"))
		  RemBit (pMobIndex->form, vector);
		else if (!str_prefix (word, "par"))
		  RemBit (pMobIndex->parts, vector);
		else
		  {
		    bug ("Flag remove: flag not found.");
		    exit (1);
		  }
	      }
	      break;
	    case 'M':
	      {
		ProgList *pMprog;
		char *word;
		int trigger = 0;

		pMprog = new_prog_list ();
		word = read_word (fp);
		if ((trigger = flag_value (mprog_flags, word)) == NO_FLAG)
		  {
		    bug ("MOBprogs: invalid trigger.");
		    exit (1);
		  }
		SetBit (pMobIndex->mprog_flags, trigger);
		pMprog->trig_type = trigger;
		pMprog->prog = (ProgCode *) read_long (fp);
		read_strfree (fp, &pMprog->trig_phrase);
		Link (pMprog, pMobIndex->mprog, next, prev);
	      }
	      break;
	    case 'S':
	      pMobIndex->kills = read_long (fp);
	      pMobIndex->deaths = read_long (fp);
	      break;
	    default:
	      iHash = vnum % MAX_KEY_HASH;
	      f_ungetc (letter, fp);
	      break;
	    }
	}

      LinkSingle (pMobIndex, char_index_hash[iHash], next);
      top_vnum_mob = Max (top_vnum_mob, vnum);
      assign_area_vnum (vnum);
      kill_table[Range (0, pMobIndex->level, MAX_LEVEL - 1)].number++;
    }

  return;
}


Boot_Fun (load_objects)
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
      pObjIndex->new_format = true;
      pObjIndex->reset_num = 0;
      newobjs++;
      read_strfree (fp, &pObjIndex->name);
      str = read_string (fp);
      replace_str (&pObjIndex->short_descr, Upper (str));
      free_string (str);
      str = read_string (fp);
      replace_str (&pObjIndex->description, Upper (str));
      free_string (str);
      read_strfree (fp, &pObjIndex->material);

      CheckPos (pObjIndex->item_type,
		(item_t) flag_value (type_flags, read_word (fp)),
		"item_type");
      pObjIndex->extra_flags = read_flag (fp);
      pObjIndex->wear_flags = read_flag (fp);
      switch (pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	  pObjIndex->value[0] = weapon_class (read_word (fp));
	  pObjIndex->value[1] = read_number (fp);
	  pObjIndex->value[2] = read_number (fp);
	  pObjIndex->value[3] = attack_lookup (read_word (fp));
	  pObjIndex->value[4] = read_flag (fp);
	  break;
	case ITEM_CONTAINER:
	  pObjIndex->value[0] = read_number (fp);
	  pObjIndex->value[1] = read_flag (fp);
	  pObjIndex->value[2] = read_number (fp);
	  pObjIndex->value[3] = read_number (fp);
	  pObjIndex->value[4] = read_number (fp);
	  break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	  pObjIndex->value[0] = read_number (fp);
	  pObjIndex->value[1] = read_number (fp);
	  CheckPos (pObjIndex->value[2],
		    liq_lookup (read_word (fp)), "liq_lookup");
	  pObjIndex->value[3] = read_number (fp);
	  pObjIndex->value[4] = read_number (fp);
	  break;
	case ITEM_WAND:
	case ITEM_STAFF:
	  pObjIndex->value[0] = read_number (fp);
	  pObjIndex->value[1] = read_number (fp);
	  pObjIndex->value[2] = read_number (fp);
	  pObjIndex->value[3] = skill_lookup (read_word (fp));
	  pObjIndex->value[4] = read_number (fp);
	  break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
	  pObjIndex->value[0] = read_number (fp);
	  pObjIndex->value[1] = skill_lookup (read_word (fp));
	  pObjIndex->value[2] = skill_lookup (read_word (fp));
	  pObjIndex->value[3] = skill_lookup (read_word (fp));
	  pObjIndex->value[4] = skill_lookup (read_word (fp));
	  break;
	default:
	  pObjIndex->value[0] = read_flag (fp);
	  pObjIndex->value[1] = read_flag (fp);
	  pObjIndex->value[2] = read_flag (fp);
	  pObjIndex->value[3] = read_flag (fp);
	  pObjIndex->value[4] = read_flag (fp);
	  break;
	}
      pObjIndex->level = read_number (fp);
      pObjIndex->weight = read_number (fp);
      pObjIndex->cost = read_number (fp);


      letter = read_letter (fp);
      switch (letter)
	{
	case ('P'):
	  pObjIndex->condition = 100;
	  break;
	case ('G'):
	  pObjIndex->condition = 90;
	  break;
	case ('A'):
	  pObjIndex->condition = 75;
	  break;
	case ('W'):
	  pObjIndex->condition = 50;
	  break;
	case ('D'):
	  pObjIndex->condition = 25;
	  break;
	case ('B'):
	  pObjIndex->condition = 10;
	  break;
	case ('R'):
	  pObjIndex->condition = 0;
	  break;
	default:
	  pObjIndex->condition = 100;
	  break;
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
		paf->level = pObjIndex->level;
		paf->duration = -1;
		paf->location = read_enum (apply_t, fp);
		paf->modifier = read_number (fp);
		paf->bitvector = 0;
		Link (paf, pObjIndex->affect, next, prev);
	      }
	      break;

	    case 'F':
	      {
		AffectData *paf;

		paf = new_affect ();
		letter = read_letter (fp);
		switch (letter)
		  {
		  case 'A':
		    paf->where = TO_AFFECTS;
		    break;
		  case 'I':
		    paf->where = TO_IMMUNE;
		    break;
		  case 'R':
		    paf->where = TO_RESIST;
		    break;
		  case 'V':
		    paf->where = TO_VULN;
		    break;
		  default:
		    bug ("Load_objects: Bad where on flag set.");
		    exit (1);
		  }
		paf->type = -1;
		paf->level = pObjIndex->level;
		paf->duration = -1;
		paf->location = read_enum (apply_t, fp);
		paf->modifier = read_number (fp);
		paf->bitvector = read_flag (fp);
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
	    case 'O':
	      {
		ProgList *pOprog;
		const char *word;
		flag_t trigger = 0;

		pOprog = new_prog_list ();
		word = read_word (fp);
		if ((trigger = flag_value (oprog_flags, word)) == NO_FLAG)
		  {
		    bug ("OBJprogs: invalid trigger.");
		    exit (1);
		  }
		SetBit (pObjIndex->oprog_flags, trigger);
		pOprog->trig_type = trigger;
		pOprog->prog = (ProgCode *) read_long (fp);
		read_strfree (fp, &pOprog->trig_phrase);
		Link (pOprog, pObjIndex->oprog, next, prev);
	      }
	      break;
	    default:
	      iHash = vnum % MAX_KEY_HASH;
	      f_ungetc (letter, fp);
	      break;
	    }
	}

      LinkSingle (pObjIndex, obj_index_hash[iHash], next);
      top_vnum_obj = Max (top_vnum_obj, vnum);
      assign_area_vnum (vnum);
    }

  return;
}


void
convert_objects (void)
{
  vnum_t vnum;
  AreaData *pArea;
  ResetData *pReset;
  CharIndex *pMob = NULL;
  ObjIndex *pObj;
  RoomIndex *pRoom;

  if (newobjs == top_obj_index)
    return;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
	{
	  if (!(pRoom = get_room_index (vnum)))
	    continue;

	  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
	    {
	      switch (pReset->command)
		{
		case 'M':
		  if (!(pMob = get_char_index (pReset->arg1)))
		    bugf ("Convert_objects: 'M': bad vnum %ld.",
			  pReset->arg1);
		  break;

		case 'O':
		  if (!(pObj = get_obj_index (pReset->arg1)))
		    {
		      bugf ("Convert_objects: 'O': bad vnum %ld.",
			    pReset->arg1);
		      break;
		    }

		  if (pObj->new_format)
		    continue;

		  if (!pMob)
		    {
		      bug ("Convert_objects: 'O': No mob reset yet.");
		      break;
		    }

		  pObj->level =
		    pObj->level <
		    1 ? pMob->level - 2 : Min (pObj->level, pMob->level - 2);
		  break;

		case 'P':
		  {
		    ObjIndex *ppObj, *pObjTo;

		    if (!(ppObj = get_obj_index (pReset->arg1)))
		      {
			bugf ("Convert_objects: 'P': bad vnum %ld.",
			      pReset->arg1);
			break;
		      }

		    if (ppObj->new_format)
		      continue;

		    if (!(pObjTo = get_obj_index (pReset->arg3)))
		      {
			bugf ("Convert_objects: 'P': bad vnum %ld.",
			      pReset->arg3);
			break;
		      }

		    ppObj->level =
		      ppObj->level <
		      1 ? pObjTo->level : Min (ppObj->level, pObjTo->level);
		  }
		  break;

		case 'G':
		case 'E':
		  if (!(pObj = get_obj_index (pReset->arg1)))
		    {
		      bugf ("Convert_objects: 'E' or 'G': bad vnum %ld.",
			    pReset->arg1);
		      break;
		    }

		  if (!pMob)
		    {
		      bugf
			("Convert_objects: 'E' or 'G': null mob for vnum %ld.",
			 pReset->arg1);
		      break;
		    }

		  if (pObj->new_format)
		    continue;

		  if (pMob->pShop)
		    {
		      switch (pObj->item_type)
			{
			default:
			  pObj->level = Max (0, pObj->level);
			  break;
			case ITEM_PILL:
			case ITEM_POTION:
			  pObj->level = Max (5, pObj->level);
			  break;
			case ITEM_SCROLL:
			case ITEM_ARMOR:
			case ITEM_WEAPON:
			  pObj->level = Max (10, pObj->level);
			  break;
			case ITEM_WAND:
			case ITEM_TREASURE:
			  pObj->level = Max (15, pObj->level);
			  break;
			case ITEM_STAFF:
			  pObj->level = Max (20, pObj->level);
			  break;
			}
		    }
		  else
		    pObj->level =
		      pObj->level <
		      1 ? pMob->level : Min (pObj->level, pMob->level);
		  break;
		}
	    }
	}
    }



  for (pArea = area_first; pArea; pArea = pArea->next)
    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
      if ((pObj = get_obj_index (vnum)))
	if (!pObj->new_format)
	  convert_object (pObj);

  return;
}


void
convert_object (ObjIndex * pObjIndex)
{
  int level;
  int number, type;

  if (!pObjIndex || pObjIndex->new_format)
    return;

  level = pObjIndex->level;

  pObjIndex->level = Max (0, pObjIndex->level);
  pObjIndex->cost = 10 * level;

  switch (pObjIndex->item_type)
    {
    default:
      bugf ("Obj_convert: type %d bad type.", pObjIndex->item_type);
      break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
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
    case ITEM_SCROLL:
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      pObjIndex->value[2] = pObjIndex->value[1];
      break;

    case ITEM_WEAPON:



      number = Min (level / 4 + 1, 5);
      type = (level + 7) / number;

      pObjIndex->value[1] = number;
      pObjIndex->value[2] = type;
      break;

    case ITEM_ARMOR:
      pObjIndex->value[0] = level / 5 + 3;
      pObjIndex->value[1] = pObjIndex->value[0];
      pObjIndex->value[2] = pObjIndex->value[0];
      break;

    case ITEM_POTION:
    case ITEM_PILL:
      break;

    case ITEM_MONEY:
      pObjIndex->value[0] = pObjIndex->cost;
      break;
    }

  pObjIndex->new_format = true;
  ++newobjs;

  return;
}


void
convert_mobile (CharIndex * pMobIndex)
{
  int i;
  int type, number, bonus;
  int level;

  if (!pMobIndex || pMobIndex->new_format)
    return;

  level = pMobIndex->level;

  pMobIndex->act |= ACT_WARRIOR;


  type = level * level * 27 / 40;
  number = Min (type / 40 + 1, 10);
  type = Max (2, type / number);
  bonus = Max (0, level * (8 + level) * 9 / 10 - number * type);

  pMobIndex->hit[DICE_NUMBER] = number;
  pMobIndex->hit[DICE_TYPE] = type;
  pMobIndex->hit[DICE_BONUS] = bonus;

  pMobIndex->mana[DICE_NUMBER] = level;
  pMobIndex->mana[DICE_TYPE] = 10;
  pMobIndex->mana[DICE_BONUS] = 100;


  type = level * 7 / 4;
  number = Min (type / 8 + 1, 5);
  type = Max (2, type / number);
  bonus = Max (0, level * 9 / 4 - number * type);

  pMobIndex->damage[DICE_NUMBER] = number;
  pMobIndex->damage[DICE_TYPE] = type;
  pMobIndex->damage[DICE_BONUS] = bonus;

  switch (number_range (1, 3))
    {
    case (1):
      pMobIndex->dam_type = 3;
      break;
    case (2):
      pMobIndex->dam_type = 7;
      break;
    case (3):
      pMobIndex->dam_type = 11;
      break;
    }

  for (i = 0; i < 3; i++)
    pMobIndex->ac[i] = interpolate (level, 100, -100);
  pMobIndex->ac[3] = interpolate (level, 100, 0);

  pMobIndex->wealth /= 100;
  pMobIndex->size = SIZE_MEDIUM;
  pMobIndex->material = str_dup ("none");

  pMobIndex->new_format = true;
  ++newmobs;

  return;
}



char *
intstr (long i, const char *word)
{
  static char buf[5][MIL];
  static int x;
  size_t len;

  x++;
  x %= 5;
  len = strlen (word);

  if (i == 1)
    {
      sprintf (buf[x], "%ld %s", i, word);
    }
  else if (word[len - 1] == 'y')
    {
      char tmp[MSL];

      strcpy (tmp, word);
      tmp[len - 1] = NUL;
      sprintf (buf[x], "%ld %sies", i, tmp);
    }
  else if (word[len - 1] == 's')
    {
      if (word[len - 2] == 's')
	{
	  sprintf (buf[x], "%ld %ses", i, word);
	}
      else
	{
	  sprintf (buf[x], "%ld %s", i, word);
	}
    }
  else
    sprintf (buf[x], "%ld %ss", i, word);

  return buf[x];
}

const char *
fsize (size_t size)
{

  if (size < 1024)
    return FORMATF ("%3d  b", size);
  else if (size < 1048576)
    return FORMATF ("%3d Kb", size / 1024);
  else if (size < 1073741824)
    return FORMATF ("%3d Mb", size / 1048576);
  else
    return FORMATF ("%3d Gb", size / 1073741824);
}

#define rpt( name, obj ) do { if(top_##name > 0) { bprintlnf(buf, "%20s * %s = %s", \
                         intstr(top_##name, #name), fsize(sizeof(obj)), fsize(top_##name * \
                         sizeof(obj)) ); total += top_##name * \
                         sizeof(obj); } } while(0)

#define rptf( name, type ) do { if (top_##name##_free > 0) { bprintlnf(buf,  "%20s * %s = %s", \
                           intstr(top_##name##_free, #name), fsize(sizeof(type)), fsize(top_##name##_free * sizeof(type))); \
                           total += top_##name##_free * sizeof(type); } } while(0)

#define rptsd(n,st,sz)    do { if(n > 0) { bprintlnf(buf,  "%6d %-29s %s", n, st, fsize(sz)); total += sz; } } while(0)

#define rpt_end  do { bprintlnf(buf, "Total memory used: %s", fsize(total)); bprintln(buf, separator); sendpage(ch, buf_string(buf)); free_buf(buf); } while(0)


const char separator[] =
  "----------------------------------------------------------------------";

Do_Fun (do_memory_heap)
{
  size_t total = 0;
  Buffer *buf = new_buf ();

  bprintln (buf, separator);
  rptsd (nAllocString, "Strings in string space", sAllocString);
  rptsd (nAllocPerm, "Permanent memory objects", sAllocPerm);
  bprintln (buf, separator);
  rpt_end;
}

Do_Fun (do_memory_perms)
{
  Buffer *buf = new_buf ();
  size_t total = 0;

  bprintln (buf, separator);
  bprintln (buf, "Memory objects in use");
  bprintln (buf, separator);
  rpt (char, CharData);

  rpt (pcdata, PcData);
  rpt (descriptor, Descriptor);
  rpt (char_index, CharIndex);
  rpt (obj, ObjData);
  rpt (obj_index, ObjIndex);
  rpt (ed, ExDescrData);
  rpt (affect, AffectData);
  rpt (room_index, RoomIndex);
  rpt (exit, ExitData);
  rpt (shop, ShopData);
  rpt (area, AreaData);
  rpt (reset, ResetData);
  rpt (area, AreaData);
  rpt (help, HelpData);
  rpt (corpse, CorpseData);
  rpt (note, NoteData);
  rpt (stat, StatData);
  rpt (mbr, ClanMember);
  rpt (auction, AuctionData);
  rpt (ban, BanData);
  rpt (social, SocialData);
  rpt (skill, SkillData);
  rpt (group, GroupData);
  rpt (race, RaceData);
  rpt (cmd, CmdData);
  rpt (class, ClassData);
  rpt (deity, DeityData);
  rpt (clan, ClanData);
  rpt (song, SongData);
  rpt (mprog, ProgCode);
  rpt (oprog, ProgCode);
  rpt (rprog, ProgCode);
  rpt (channel, ChannelData);
  rpt (wpwd, WebpassData);
  rpt (prog_list, ProgList);
  rpt (buf, Buffer);
  rpt (gen_data, GenData);
#ifdef STFILEIO

  rpt (stfile, stFile);
#endif

  bprintln (buf, separator);
  rpt_end;
}

Do_Fun (do_memory_freelists)
{
  Buffer *buf = new_buf ();
  size_t total = 0;

  bprintln (buf, separator);
  bprintln (buf, "Memory objects waiting to be recycled");
  bprintln (buf, separator);
  rptf (char, CharData);

  rptf (pcdata, PcData);
  rptf (descriptor, Descriptor);
  rptf (char_index, CharIndex);
  rptf (obj, ObjData);
  rptf (obj_index, ObjIndex);
  rptf (ed, ExDescrData);
  rptf (affect, AffectData);
  rptf (room_index, RoomIndex);
  rptf (exit, ExitData);
  rptf (shop, ShopData);
  rptf (area, AreaData);
  rptf (reset, ResetData);
  rptf (area, AreaData);
  rptf (help, HelpData);
  rptf (corpse, CorpseData);
  rptf (note, NoteData);
  rptf (stat, StatData);
  rptf (mbr, ClanMember);
  rptf (auction, AuctionData);
  rptf (ban, BanData);
  rptf (social, SocialData);
  rptf (skill, SkillData);
  rptf (group, GroupData);
  rptf (race, RaceData);
  rptf (cmd, CmdData);
  rptf (class, ClassData);
  rptf (deity, DeityData);
  rptf (clan, ClanData);
  rptf (song, SongData);
  rptf (mprog, ProgCode);
  rptf (oprog, ProgCode);
  rptf (rprog, ProgCode);
  rptf (channel, ChannelData);
  rptf (wpwd, WebpassData);
  rptf (prog_list, ProgList);
  rptf (buf, Buffer);
  rptf (gen_data, GenData);
#ifdef STFILEIO

  rptf (stfile, stFile);
#endif

  bprintln (buf, separator);
  rpt_end;
}

Do_Fun (do_memory_formats)
{
  size_t total = 0;
  Buffer *buf = new_buf ();

  bprintln (buf, separator);
  rptsd (top_char_index - newmobs, "Old format mobs",
	 (top_char_index - newmobs) * sizeof (CharIndex));
  rptsd (top_obj_index - newobjs, "Old format objects",
	 (top_obj_index - newobjs) * sizeof (ObjIndex));
  bprintln (buf, separator);
  rpt_end;
}


#undef rpt
#undef rptsd
#undef rptf

Do_Fun (do_memory_help)
{
  cmd_syntax (ch, NULL, n_fun,
	      "heap                Report heap memory allocated by the memory manager",
	      "perms               Report managed memory objects in use by other modules",
	      "freelists           Report memory objects waiting to be recycled",
	      "formats             Report old format prototypes",
	      "stats               Report machine stats",
	      "?                   This message", NULL);
}

Do_Fun (do_memory_stats)
{
  chprintlnf (ch, " HostName = %s.", HOSTNAME);
  chprintlnf (ch, " UserName = %s.", UNAME);
  chprintlnf (ch, " Running = %s.", EXE_FILE);
  chprintlnf (ch, " From = %s.", CWDIR);
  chprintlnf (ch, " Port = %d.", mainport);
#ifndef DISABLE_WEBSRV

  if (web_is_connected ())
    chprintlnf (ch, " Web Server Port = %d", webport);
  else
    chprintlnf (ch, " The Web Server on port %d is down.", webport);
#endif
  chprintlnf (ch, " Current process ID is %d.", getpid ());
  chprintln (ch, " Compiled on " __DATE__ ", " __TIME__ ".");
}

Do_Fun (do_memory)
{
  vinterpret (ch, n_fun, argument, "perms", do_memory_perms, "heap",
	      do_memory_heap, "freelists", do_memory_freelists, "formats",
	      do_memory_formats, "stats", do_memory_stats, NULL,
	      do_memory_help);
}

int
strswitch (const char *arg, ...)
{
  int i = 0;
  char *p;
  va_list caselist;

  if (arg[0])
    {
      va_start (caselist, arg);
      while ((p = va_arg (caselist, char *)) != NULL)
	{
	  i++;
	  if (!str_prefix (arg, p))
	    return i;
	}
      va_end (caselist);
    }
  return 0;
}

void
vinterpret (CharData * ch, const char *n_fun, const char *argument, ...)
{
  char arg[MAX_INPUT_LENGTH];
  char *iStr;
  Do_F *iFun;
  va_list caselist;

  va_start (caselist, argument);
  argument = one_argument (argument, arg);
  do
    {
      iStr = va_arg (caselist, char *);

      iFun = va_arg (caselist, Do_F *);
    }
  while (iStr != NULL && (!arg[0] || str_prefix (arg, iStr)));
  va_end (caselist);

  if (iFun != NULL)
    {
      char buf[MIL];

      if (iStr != NULL)
	sprintf (buf, "%s %s", n_fun, iStr);
      else
	strcpy (buf, n_fun);

      (*iFun) (n_fun, ch, argument);
    }
  else
    {
      int count = 0;

      va_start (caselist, argument);
      do
	{
	  iStr = va_arg (caselist, char *);

	  if (iStr == NULL)
	    break;

	  if (count == 0)
	    chprintlnf (ch, "Syntax: %s %s", n_fun, iStr);
	  else
	    chprintlnf (ch, "      : %s %s", n_fun, iStr);

	  iFun = va_arg (caselist, Do_F *);
	}
      while (iStr != NULL && (!arg[0] || str_prefix (arg, iStr)));
      va_end (caselist);
    }
}

const struct index_type dofun_index[] = {
#define MAKE_COMMAND_TABLE    	1
#include "dofun.h"
#undef MAKE_COMMAND_TABLE
  {NULL, NULL}
};

const struct index_type gsn_index[] = {
#define MAKE_INDEX_TABLE    INDEX_GSN
#include "index.h"
#undef MAKE_INDEX_TABLE
  {NULL, NULL}
};

const struct index_type spell_index[] = {
#define MAKE_SPELL_TABLE    	1
#include "magic.h"
#undef MAKE_SPELL_TABLE
  {NULL, NULL}
};

const struct index_type class_index[] = {
#define MAKE_INDEX_TABLE INDEX_CLASS
#include "index.h"
#undef MAKE_INDEX_TABLE
  {NULL, NULL}
};


const struct index_type channel_index[] = {
#define MAKE_INDEX_TABLE    INDEX_GCN
#include "index.h"
#undef MAKE_INDEX_TABLE
  {NULL, NULL}
};

void *
index_lookup (const char *name, const struct index_type *table, void *def)
{
  int i;

  for (i = 0; table[i].name != NULL; i++)
    {
      if (!str_cmp (name, table[i].name))
	return table[i].index;
    }
  return def;
}

const char *
index_name (void *data, const struct index_type *table, const char *def)
{
  int i;

  for (i = 0; table[i].name != NULL; i++)
    {
      if (data == table[i].index)
	return table[i].name;
    }
  return def;
}

void
free_runbuf (Descriptor * d)
{
  if (d && d->run_buf)
    {
      free_string (d->run_buf);
      d->run_buf = NULL;
      d->run_head = NULL;
    }
  return;
}

Boot_Fun (load_objprogs)
{
  ProgCode *pOprog;

  if (current_area == NULL)
    {
      bug ("Load_objprogs: no #AREA seen yet.");
      exit (1);
    }

  for (;;)
    {
      vnum_t vnum;
      char letter;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_objprogs: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_prog_index (vnum, PRG_OPROG) != NULL)
	{
	  bugf ("Load_objprogs: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pOprog = new_oprog ();
      pOprog->vnum = vnum;
      read_strfree (fp, &pOprog->code);
      pOprog->area = current_area;
      Link (pOprog, oprog, next, prev);
    }
  return;
}

Boot_Fun (load_roomprogs)
{
  ProgCode *pRprog;

  if (current_area == NULL)
    {
      bug ("Load_roomprogs: no #AREA seen yet.");
      exit (1);
    }

  for (;;)
    {
      vnum_t vnum;
      char letter;

      letter = read_letter (fp);
      if (letter != '#')
	{
	  bug ("Load_roomprogs: # not found.");
	  exit (1);
	}

      vnum = read_number (fp);
      if (vnum == 0)
	break;

      run_level = RUNLEVEL_SAFE_BOOT;
      if (get_prog_index (vnum, PRG_RPROG) != NULL)
	{
	  bugf ("Load_roomprogs: vnum %ld duplicated.", vnum);
	  exit (1);
	}
      run_level = RUNLEVEL_BOOTING;

      pRprog = new_oprog ();
      pRprog->vnum = vnum;
      read_strfree (fp, &pRprog->code);
      pRprog->area = current_area;
      Link (pRprog, rprog, next, prev);
    }
  return;
}

void
fix_objprogs (void)
{
  ObjIndex *pObjIndex;
  ProgList *list;
  ProgCode *prog;
  int iHash;

  log_string ("Fixing obj programs...");
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pObjIndex = obj_index_hash[iHash];
	   pObjIndex != NULL; pObjIndex = pObjIndex->next)
	{
	  for (list = pObjIndex->oprog_first; list != NULL; list = list->next)
	    {
	      if ((prog = get_prog_index ((vnum_t) list->prog, PRG_OPROG))
		  == NULL)
		{
		  bugf ("Fix_objprogs: code vnum %ld not found.",
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
fix_roomprogs (void)
{
  RoomIndex *pRoomIndex;
  ProgList *list;
  ProgCode *prog;
  int iHash;

  log_string ("Fixing room programs...");
  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pRoomIndex = room_index_hash[iHash];
	   pRoomIndex != NULL; pRoomIndex = pRoomIndex->next)
	{
	  for (list = pRoomIndex->rprog_first; list != NULL;
	       list = list->next)
	    {
	      if ((prog = get_prog_index ((vnum_t) list->prog, PRG_RPROG))
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

int
srt_skills (const void *p1, const void *p2)
{
  struct skill_type sk1;
  struct skill_type sk2;

  sk1 = *(struct skill_type *) p1;
  sk2 = *(struct skill_type *) p2;

  return str_cmp (sk1.name, sk2.name);
}

void
add_social (SocialData * social)
{
  SocialData *tmp;

  hash_social (social);

  for (tmp = social_first; tmp; tmp = tmp->next)
    {
      if (str_cmp (social->name, tmp->name) < 0)
	{
	  Insert (social, tmp, social, next, prev);
	  break;
	}
    }

  if (!tmp)
    Link (social, social, next, prev);
}

void
unlink_social (SocialData * social)
{
  unhash_social (social);
  UnLink (social, social, next, prev);
}

void
add_help (HelpData * help)
{
  HelpData *tmp;

  if (!help)
    return;

  if (!str_prefix ("greeting", help->keyword))
    {
      const char *pstr = help->keyword;
      int number = 0;

      while (*pstr)
	{
	  if (isdigit (*pstr))
	    number = number * 10 + *pstr - '0';
	  pstr++;
	}

      if (number > 0)
	help_greeting[number - 1] = help->text;
      if (number == 0 && help_greeting[0] == NULL)
	help_greeting[0] = help->text;
    }

  for (tmp = help_first; tmp; tmp = tmp->next)
    {
      if (str_cmp (help->keyword, tmp->keyword) < 0)
	{
	  Insert (help, tmp, help, next, prev);
	  break;
	}
    }

  if (!tmp)
    Link (help, help, next, prev);
}

void
newcmd_insert_name_sort (CmdData * a)
{
  CmdData *lsort = cmd_first_sorted;
  CmdData *lsort_prev = NULL;

  if (!cmd_first_sorted)
    {
      cmd_first_sorted = a;
      return;
    }

  for (; lsort; lsort_prev = lsort, lsort = lsort->next_sort)
    {
      if (str_cmp (a->name, lsort->name) > 0)
	{
	  continue;
	}

      if (lsort_prev)
	{
	  a->next_sort = lsort;
	  lsort_prev->next_sort = a;
	}
      else
	{

	  a->next_sort = cmd_first_sorted;
	  cmd_first_sorted = a;
	}
      return;
    }

  lsort_prev->next_sort = a;
}

void
add_command (CmdData * command)
{
  hash_command (command);
  newcmd_insert_name_sort (command);
  Link (command, cmd, next, prev);
}

void
unlink_command (CmdData * command)
{
  unhash_command (command);
  UnlinkSingle (command, CmdData, cmd_first_sorted, next_sort);
  UnLink (command, cmd, next, prev);
}

#define                 MAX_PERM_BLOCK  131072

void *
alloc_perm (size_t sMem)
{
  static char *pMemPerm;
  static size_t iMemPerm;
  void *pMem;

  while (sMem % sizeof (long) != 0)
    sMem++;

  if (sMem > MAX_PERM_BLOCK)
    {
      bugf ("%d too large.", sMem);
      exit (1);
    }

  if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK)
    {
      iMemPerm = 0;
      if ((pMemPerm = (char *) calloc (1, MAX_PERM_BLOCK)) == NULL)
	{
	  log_error ("Alloc_perm");
	  exit (1);
	}
    }

  pMem = pMemPerm + iMemPerm;
  iMemPerm += sMem;
  nAllocPerm += 1;
  sAllocPerm += sMem;

  return pMem;
}

void
hash_command (CmdData * command)
{
  int hash;

  if (!command)
    {
      bug ("add_command: NULL command");
      return;
    }

  if (!command->name)
    {
      bug ("add_command: NULL command->name");
      return;
    }

  if (!command->do_fun)
    {
      bug ("add_command: NULL command->do_fun");
      return;
    }

  hash = tolower (command->name[0]) % MAX_CMD_HASH;

  LinkLast (CmdData, command, command_hash[hash], next_hash);

  return;
}

void
unhash_command (CmdData * command)
{
  int hash;

  if (!command)
    {
      bug ("unlink_command: NULL command");
      return;
    }

  hash = tolower (command->name[0]) % MAX_CMD_HASH;

  UnlinkSingle (command, CmdData, command_hash[hash], next_hash);
}

void
unhash_social (SocialData * social)
{
  int hash;

  if (!social)
    {
      bug ("unlink_social: NULL social");
      return;
    }

  if (tolower (social->name[0]) < 'a' || tolower (social->name[0]) > 'z')
    hash = 0;
  else
    hash = (tolower (social->name[0]) - 'a') + 1;

  UnlinkSingle (social, SocialData, social_hash[hash], next_hash);
}

void
hash_social (SocialData * social)
{
  int hash;

  if (!social)
    {
      bug ("add_social: NULL social");
      return;
    }

  if (!social->name)
    {
      bug ("add_social: NULL social->name");
      return;
    }

  if (tolower (social->name[0]) < 'a' || tolower (social->name[0]) > 'z')
    hash = 0;
  else
    hash = (tolower (social->name[0]) - 'a') + 1;

  LinkLast (SocialData, social, social_hash[hash], next_hash);

  return;
}

int
convert_level (char *arg)
{
  if (NullStr (arg))
    return 0;
  else if (is_number (arg))
    return atoi (arg);
  else if (is_name ("IMM", arg))
    return LEVEL_IMMORTAL;
  else if (is_name ("HERO", arg) || is_name ("HRO", arg))
    return MAX_MORTAL_LEVEL;
  else
    return 0;
}

void
convert_area_credits (AreaData * pArea)
{
  char high[MAX_STRING_LENGTH], low[MAX_STRING_LENGTH], builder[MSL];

  if (!pArea || pArea->version > 0)
    return;

  if (3 !=
      sscanf (pArea->credits, "{ %[^} ] %[^} ] } %s", low, high, builder)
      && 3 != sscanf (pArea->credits, "{ %[^} ]-%[^} ] } %s", low, high,
		      builder)
      && 3 != sscanf (pArea->credits, "[ %[^] ] %[^] ] ] %s", low, high,
		      builder)
      && 3 != sscanf (pArea->credits, "[ %[^] ]-%[^] ] ] %s", low, high,
		      builder))
    {
      if (2 == sscanf (pArea->credits, "{ %[^} ] } %s", low, builder)
	  || 2 == sscanf (pArea->credits, "[ %[^] ] ] %s", low, builder))
	{
	  replace_str (&pArea->lvl_comment, low);
	  replace_str (&pArea->credits, builder);
	}
      return;
    }
  replace_str (&pArea->lvl_comment, "");
  replace_str (&pArea->credits, builder);
  pArea->min_level = convert_level (low);
  pArea->max_level = convert_level (high);
}

int
reset_door (ResetData * pReset, bool fRandom)
{
  if (pReset && pReset->command == 'R')
    {
      if (pReset->arg2 > 0 && pReset->arg2 <= MAX_DIR)
	{
	  return pReset->arg2 - 1;
	}
      else if (fRandom && pReset->arg4 > 0 && pReset->arg4 <= MAX_DIR)
	{
	  return pReset->arg4 - 1;
	}
    }
  return -1;
}

int
open_exit (RoomIndex * toRoom, RoomIndex * pRoom, ResetData * pReset)
{
  int door;

  if ((door = reset_door (pReset, false)) != -1)
    {
      if (toRoom->exit[rev_dir[door]] != NULL)
	return -1;

      return door;
    }
  for (door = 0; door < MAX_DIR; door++)
    {
      if (!toRoom->exit[rev_dir[door]] && !pRoom->exit[door])
	return door;
    }
  return -1;
}

void
clean_temp_exit (RoomIndex * pRoom, int door)
{
  RoomIndex *pToRoom;

  if (!pRoom || (door < 0 && door >= MAX_DIR) ||
      pRoom->exit[door] == NULL
      || !IsSet (pRoom->exit[door]->exit_info, EX_TEMP))
    return;

  if ((pToRoom = pRoom->exit[door]->u1.to_room) != NULL)
    {
      int rev = rev_dir[door];

      if (pToRoom->exit[rev]
	  && IsSet (pToRoom->exit[rev]->exit_info, EX_TEMP))
	{
	  free_exit (pToRoom->exit[rev]);
	  pToRoom->exit[rev] = NULL;
	}
    }

  free_exit (pRoom->exit[door]);
  pRoom->exit[door] = NULL;
}

void
clean_entrance (RoomIndex * pRoom, ResetData * pReset)
{
  int door;

  if (!pRoom)
    return;

  if ((door = reset_door (pReset, true)) != -1)
    {
      clean_temp_exit (pRoom, door);
      return;
    }
  for (door = 0; door < MAX_DIR; door++)
    {
      clean_temp_exit (pRoom, door);
    }
}

void
add_random_exit (RoomIndex * pRoom, ResetData * pReset, bool Area)
{
  ExitData *pExit, *toExit;
  RoomIndex *toRoom;
  int door, rev;
  vnum_t minvn, maxvn;

  if (!pRoom)
    return;

  if (Area)
    {
      minvn = pRoom->area->min_vnum;
      maxvn = pRoom->area->max_vnum;
    }
  else
    {
      minvn = 2;
      maxvn = top_vnum_room;
    }

  clean_entrance (pRoom, pReset);

  for (;;)
    {
      toRoom = get_room_index (number_range (minvn, maxvn));
      if (toRoom != NULL && !IsSet (toRoom->room_flags, ROOM_PRIVATE)
	  && !IsSet (toRoom->room_flags, ROOM_SOLITARY)
	  && !IsSet (toRoom->room_flags, ROOM_ARENA)
	  && !IsSet (toRoom->room_flags, ROOM_BANK)
	  && !IsSet (toRoom->room_flags, ROOM_IMP_ONLY)
	  && !IsSet (toRoom->room_flags, ROOM_GODS_ONLY)
	  && !IsSet (toRoom->room_flags, ROOM_HEROES_ONLY)
	  && !IsSet (toRoom->room_flags, ROOM_NEWBIES_ONLY)
	  && (Area
	      || !IsSet (toRoom->area->area_flags,
			 AREA_CLOSED | AREA_PLAYER_HOMES))
	  && NullStr (toRoom->owner) && (Area
					 || toRoom->area->clan == NULL)
	  && (door = open_exit (toRoom, pRoom, pReset)) != -1)
	break;
    }

  pExit = new_exit ();
  pExit->u1.to_room = toRoom;
  pExit->orig_door = door;
  SetBit (pExit->rs_flags, EX_TEMP);
  SetBit (pExit->exit_info, EX_TEMP);
  pRoom->exit[door] = pExit;

  rev = rev_dir[door];
  toExit = new_exit ();
  toExit->u1.to_room = pRoom;
  toExit->orig_door = rev;
  SetBit (toExit->rs_flags, EX_TEMP);
  SetBit (toExit->exit_info, EX_TEMP);
  toRoom->exit[rev] = toExit;



  if (pReset && pReset->arg4 != -1)
    pReset->arg4 = door + 1;
}

bool
check_directories (const char *exename)
{
  char current_dir[MSL];
  bool problem = false;
  int i;


  if (getcwd (current_dir, MSL) == NULL)
    {
      bugf
	("check_directories(): can't get current directory - error %d (%s)",
	 errno, strerror (errno));
    }

  for (i = 0; strlen (directories_table[i].directory) +
       strlen (directories_table[i].text) > 0; i++)
    {
      if (chdir (directories_table[i].directory))
	{
	  if (strlen (directories_table[i].directory) > 0)
	    {
	      logf ("no directory %-16s - used for '%s'",
		    directories_table[i].directory,
		    directories_table[i].text);
	      problem = true;
	    }
	}
      if (chdir (current_dir))
	{
	  logf ("UNABLE TO CHANGE BACK TO THE DEFAULT DIRECTORY!!!\n%s\n",
		current_dir);
	  problem = true;
	}
    }

  if (problem)
    {
      logf ("\nTo create all these directories start the mud as:\n"
	    "'%s --createdirs'\nfrom within '%s'\n\n", exename, current_dir);
    }

  return problem;
}


bool
create_directories (void)
{
  char current_dir[MSL];
  bool problem = false;
  int i;

  log_string ("=== starting in create directories mode:");

  if (getcwd (current_dir, MSL) == NULL)
    {
      bugf
	("create_directories(): can't get current directory - error %d (%s)",
	 errno, strerror (errno));
    }

  for (i = 0; strlen (directories_table[i].directory) +
       strlen (directories_table[i].text) > 0; i++)
    {
      if (chdir (directories_table[i].directory))
	{
	  if (strlen (directories_table[i].directory) > 0)
	    {
	      logf ("creating dir %-15s - used for '%s'",
		    directories_table[i].directory,
		    directories_table[i].text);

	      if (makedir (directories_table[i].directory))
		{
		  logf ("could not create dir %-8s  - used for '%s'",
			directories_table[i].directory,
			directories_table[i].text);
		  problem = true;
		}
	    }
	}
      if (chdir (current_dir))
	{
	  bugf ("UNABLE TO CHANGE BACK TO THE DEFAULT DIRECTORY!!!\n%s\n",
		current_dir);
	  problem = true;
	}
    }

  if (problem)
    {
      logf
	("\nThe code couldn't create the above directory/directories for you...\n"
	 "see if you can fix the problem manually.\n");
    }

  return problem;
}


void
build_player_index (void)
{
  int i, o;
  struct dirent **alist;
  struct stat fst;
  char buf[1024];
  int j, p;
  struct dirent **blist;

  if ((o = scandir (PLAYER_DIR, &alist, 0, alphasort)) == 0)
    return;

  alloc_mem (pfiles.names, const char *, o);

  for (i = 0; i != o; i++)
    {
      if (alist[i]->d_name[0] == '.' || !str_cmp ("CVS", alist[i]->d_name))
	continue;

      sprintf (buf, PLAYER_DIR "%s", alist[i]->d_name);

      if (stat (buf, &fst) != -1)
	{
	  if (S_ISDIR (fst.st_mode))
	    {
	      sprintf (buf, PLAYER_DIR "%s", alist[i]->d_name);
	      p = scandir (buf, &blist, 0, alphasort);
	      if (p > 0)
		{
		  realloc_mem (pfiles.names, const char *, o + p);

		  for (j = 0; j != p; j++)
		    {
		      sprintf (buf, PLAYER_DIR "%s" DIR_SYM "%s",
			       alist[i]->d_name, blist[j]->d_name);
		      if (stat (buf, &fst) != -1 && S_ISREG (fst.st_mode))
			{
			  pfiles.names[pfiles.count++] =
			    str_dup (blist[j]->d_name);
			}
		    }
		}
	    }
	  else if (S_ISREG (fst.st_mode))
	    {
	      pfiles.names[pfiles.count++] = str_dup (alist[i]->d_name);
	    }
	}
    }

  if (pfiles.count == 0)
    logf
      ("No players in database, first character created will be immortal!");
  else
    {
      FileData *fp;

      logf ("%d players in database.", pfiles.count);
      if ((fp = f_open (PLR_INDEX_FILE, "w")) != NULL)
	{
	  for (i = 0; i < pfiles.count; i++)
	    f_printf (fp, "%s" LF, pfiles.names[i]);
	  f_close (fp);
	}
    }
  return;
}

void *
rpl_malloc (size_t n)
{
  if (n == 0)
    n = 1;
  return malloc (n);
}

void *
rpl_realloc (void *ptr, size_t n)
{
  if (n == 0)
    n = 1;
  return realloc (ptr, n);
}
