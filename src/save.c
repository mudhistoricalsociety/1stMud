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
#include "recycle.h"
#include "tables.h"
#include "interp.h"
#include "vnums.h"

static ObjData *rgObjNest[MAX_NEST];


Proto (void write_char, (CharData *, FileData *));
Proto (void write_pet, (CharData *, FileData *));
Proto (void read_pet, (CharData *, FileData *));
Proto (void read_descriptor, (Descriptor *, FileData *));
Proto (void write_descriptor, (Descriptor *, FileData *));

#define TAB_SIZE    8

char *
format_tabs (int len)
{
  if (len < TAB_SIZE)
    return "\t\t\t";
  else if (len < TAB_SIZE * 2)
    return "\t\t";
  else if (len < TAB_SIZE * 3)
    return "\t";
  else
    return FORMATF ("%*s", Max (1, len - (TAB_SIZE)), " ");
}

const char *
pfile_filename (const char *name)
{
  static int i;
  static char rbuf[5][MSL];
  char buf[MSL], first_name[MSL];


  ++i, i %= 5;

  strcpy (buf, name);


  one_argument (buf, first_name);

  sprintf (rbuf[i], "%s.plr", first_name);

  return (rbuf[i]);
}

char *
pfilename (const char *name, pfile_t type)
{
  static int i;
  static char rbuf[5][MSL];
  char filename[MSL];


  ++i, i %= 5;

  strcpy (filename, pfile_filename (name));

  switch (type)
    {
    case PFILE_NORMAL:
#if defined(NO_INITIAL_ALPHA_PFILEDIRS)
      sprintf (rbuf[i], PLAYER_DIR "%s", filename);
#else
      sprintf (rbuf[i], PLAYER_DIR "%c" DIR_SYM "%s", filename[0], filename);
#endif
      break;
    case PFILE_BACKUP:
#if defined(NO_INITIAL_ALPHA_PFILEDIRS)
      sprintf (rbuf[i], PLAYER_BACKUP "%s", filename);
#else
      sprintf (rbuf[i], PLAYER_BACKUP "%c" DIR_SYM "%s", filename[0],
	       filename);
#endif
      break;
    case PFILE_DELETED:
      sprintf (rbuf[i], DELETE_DIR "%s", filename);
      break;
    default:
      sprintf (rbuf[i], "ERROR_PFILENAME_%s_INCORRECT_TYPE_%d",
	       filename, (int) type);
      bugf ("ERROR_PFILENAME_INCORRECT_TYPE %d - %s", (int) type, filename);
      break;
    }
  return (rbuf[i]);
}

bool
save_char_obj_to_filename (CharData * ch, const char *filename)
{
  FileData *fp;

  if ((fp = f_open (filename, "w")) == NULL)
    {
      bugf ("Save_char_obj: file open on %s", ch->name);
      log_error (filename);
      return false;
    }
  else
    {
      write_char (ch, fp);
      if (ch->carrying_first != NULL)
	write_obj (ch, ch->carrying_last, fp, 0, 0, SAVE_CHAR);

      if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
	{
	  write_pet (ch->pet, fp);
	  write_obj (ch->pet, ch->pet->carrying_last, fp, 0, 0, SAVE_PET);
	}
      if (ch->desc && (crs_info.status == CRS_COPYOVER || crash_info.crashed))
	write_descriptor (ch->desc, fp);
      f_printf (fp, "#%s" LF, END_MARK);
      f_close (fp);
      return true;
    }
}


void
save_char_obj (CharData * ch)
{
  if (IsNPC (ch))
    return;

  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;

  if (get_trust (ch) < mud_info.min_save_lvl)
    return;

  save_char_obj_to_filename (ch, pfilename (ch->name, PFILE_NORMAL));
  return;
}

void
backup_char_obj (CharData * ch)
{
  char buf[MIL];

  if (IsNPC (ch))
    return;

  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;

  if (get_trust (ch) < mud_info.min_save_lvl * 3)
    return;

  strcpy (buf, pfilename (ch->name, PFILE_BACKUP));
  if (save_char_obj_to_filename (ch, buf))
    {
      ch->pcdata->backup = ch->pcdata->played;
#ifdef unix
      system (FORMATF ("gzip -fq %s", buf));
#endif
    }
  return;
}


void
write_char (CharData * ch, FileData * fp)
{
  AffectData *paf;
  int sn, gn, pos;
  int i, j;

  f_printf (fp, "#%s" LF,
	    get_char_save_header (IsNPC (ch) ? SAVE_MOB : SAVE_CHAR));

  write_string (fp, "Name", ch->name, NULL);
  write_int (fp, "Id", "%ld", ch->id, 0);
  write_time (fp, "LogO", current_time, false);
  write_int (fp, "Vers", "%d", PFILE_VERSION, 0);
  write_string (fp, "ShD", ch->short_descr, NULL);
  write_string (fp, "LnD", ch->long_descr, NULL);
  write_string (fp, "Desc", ch->description, NULL);
  write_string (fp, "Prom", ch->prompt, NULL);
  write_string (fp, "GProm", ch->gprompt, NULL);
  write_string (fp, "Race", ch->race->name, NULL);
  if (CharClan (ch) != NULL)
    {
      write_string (fp, "Clan", CharClan (ch)->name, NULL);
      write_int (fp, "Rank", "%d", ch->rank, -1);
    }
  write_int (fp, "Sex", "%d", ch->sex, -1);
  write_array (fp, "Cla", "%d", ch->Class, MAX_MCLASS);
  write_int (fp, "PrClass", "%d", ch->pcdata->prime_class, 0);
  if (ch->pcdata->stay_race)
    f_printf (fp, "StayRace" LF);
  write_int (fp, "Levl", "%d", ch->level, 0);
  write_int (fp, "Tru", "%d", ch->trust, 0);
  write_int (fp, "Sec", "%d", ch->pcdata->security, 0);
  write_int (fp, "Plyd", "%d",
	     ch->pcdata->played + (int) (current_time - ch->logon), 0);
  write_int (fp, "Scro", "%d", ch->lines, -1);
  write_int (fp, "Cols", "%d", ch->columns, -1);
  write_int (fp, "Room", "%ld",
	     (ch->in_room == get_room_index (ROOM_VNUM_LIMBO) &&
	      ch->was_in_room !=
	      NULL) ? ch->was_in_room->vnum : ch->in_room ==
	     NULL ? 3001 : ch->in_room->vnum, 0);

  f_writef (fp, "HMV", "%ld %ld %ld %ld %ld %ld" LF, ch->hit,
	    ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
  write_int (fp, "Gold", "%ld", ch->gold, 0);
  write_int (fp, "Silv", "%ld", ch->silver, 0);
  write_int (fp, "Exp", "%d", ch->exp, 0);
  write_bit (fp, "Act", ch->act, 0);
  write_bit (fp, "AfBy", ch->affected_by, 0);
  write_bit (fp, "Comm", ch->comm, 0);
  write_bit (fp, "Wizn", ch->wiznet, 0);
  write_int (fp, "Invi", "%d", ch->invis_level, 0);
  write_int (fp, "Inco", "%d", ch->incog_level, 0);
  write_int (fp, "Pos", "%d",
	     ch->position == POS_FIGHTING ? POS_STANDING : ch->position, -1);
  write_int (fp, "Prac", "%d", ch->practice, 0);
  write_int (fp, "Trai", "%d", ch->train, 0);
  write_int (fp, "Save", "%d", ch->saving_throw, 0);
  write_int (fp, "Alig", "%d", ch->alignment, 0);
  write_int (fp, "Hit", "%d", ch->hitroll, 0);
  write_int (fp, "Dam", "%d", ch->damroll, 0);
  write_array (fp, "ACs", "%d", ch->armor, MAX_AC);
  write_array (fp, "Stance", "%d", ch->stance, MAX_STANCE);
  write_int (fp, "Wimp", "%d", ch->wimpy, 0);
  write_array (fp, "Attr", "%d", ch->perm_stat, STAT_MAX);
  write_array (fp, "AMod", "%d", ch->mod_stat, STAT_MAX);
  if (IsNPC (ch))
    {
      write_int (fp, "Vnum", "%ld", ch->pIndexData->vnum, 0);
    }
  else
    {
      write_string (fp, "Pass", ch->pcdata->pwd, NULL);
      write_string (fp, "Bin", ch->pcdata->bamfin, NULL);
      write_string (fp, "Bout", ch->pcdata->bamfout, NULL);
      write_string (fp, "Titl", ch->pcdata->title, NULL);
      write_int (fp, "Pnts", "%d", ch->pcdata->points, 0);
      write_int (fp, "TSex", "%d", ch->pcdata->true_sex, -1);
      write_int (fp, "LLev", "%d", ch->pcdata->last_level, 0);
      f_writef (fp, "HMVP", "%ld %ld %ld" LF, ch->pcdata->perm_hit,
		ch->pcdata->perm_mana, ch->pcdata->perm_move);
      write_array (fp, "Cnd", "%d", ch->pcdata->condition, 4);
      write_int (fp, "QuestPnts", "%d", ch->pcdata->quest.points, 0);
      write_int (fp, "QuestTime", "%d", ch->pcdata->quest.time, 0);
      if (ch->pcdata->quest.giver)
	write_int (fp, "QuestGiver", "%ld",
		   ch->pcdata->quest.giver->pIndexData->vnum, 0);
      if (ch->pcdata->quest.room)
	write_int (fp, "QuestLoc", "%ld", ch->pcdata->quest.room->vnum, 0);
      if (ch->pcdata->quest.obj)
	write_int (fp, "QuestObj", "%ld",
		   ch->pcdata->quest.obj->pIndexData->vnum, 0);
      if (ch->pcdata->quest.mob)
	write_int (fp, "QuestMob", "%ld",
		   ch->pcdata->quest.mob->pIndexData->vnum, 0);
      write_int (fp, "Trivia", "%d", ch->pcdata->trivia, 0);
      if (ch->pcdata->str_ed_key != '.' && ch->pcdata->str_ed_key != ' ')
	f_writef (fp, "StrEdKey", "%c" LF, ch->pcdata->str_ed_key);
      write_int (fp, "TimeZone", "%d", ch->pcdata->timezone, -1);
      write_int (fp, "AWins", "%d", ch->pcdata->awins, 0);
      write_int (fp, "ALosses", "%d", ch->pcdata->alosses, 0);
      write_int (fp, "BankG", "%ld", ch->pcdata->gold_bank, 0);
      write_int (fp, "Shares", "%d", ch->pcdata->shares, 0);
      if (ch->deity != NULL)
	write_string (fp, "Deity", ch->deity->name, NULL);
      if (Gquester (ch))
	{
	  write_array (fp, "GQmobs", "%ld", ch->gquest->gq_mobs,
		       gquest_info.mob_count);
	}
      if (war_info.status != WAR_OFF && ch->war != NULL)
	{
	  f_writef (fp, "WarInfo", "%ld %ld %ld %s" LF, ch->war->hit,
		    ch->war->mana, ch->war->move,
		    write_flags (ch->war->flags));
	}
      if (HAS_HOME (ch))
	{
	  write_array (fp, "Homes", "%ld", ch->pcdata->home, MAX_HOME_VNUMS);
	}
      f_writef (fp, "Colo", "%d", MAX_CUSTOM_COLOR);
      for (i = 0; i < MAX_CUSTOM_COLOR; i++)
	for (j = 0; j < CT_MAX; j++)
	  f_printf (fp, " %d", ch->pcdata->color[i].at[j]);
      f_printf (fp, LF);
      write_string (fp, "WhoD", ch->pcdata->who_descr, NULL);
      write_array (fp, "GStats", "%ld", ch->pcdata->gamestat, MAX_GAMESTAT);

      write_bit (fp, "Vt100", ch->pcdata->vt100, 0);

      for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	  if (NullStr (ch->pcdata->alias[pos]) ||
	      NullStr (ch->pcdata->alias_sub[pos]))
	    break;

	  f_writef (fp, "Alias", "%s %s~" LF, ch->pcdata->alias[pos],
		    ch->pcdata->alias_sub[pos]);
	}
      for (pos = 0; pos < MAX_BUDDY; pos++)
	{
	  if (NullStr (ch->pcdata->buddies[pos]))
	    break;

	  write_string (fp, "Buddy", ch->pcdata->buddies[pos], NULL);
	}
      for (pos = 0; pos < MAX_IGNORE; pos++)
	{
	  if (NullStr (ch->pcdata->ignore[pos]))
	    break;

	  f_writef (fp, "Ignore", "%s~ %s" LF, ch->pcdata->ignore[pos],
		    write_flags (ch->pcdata->ignore_flags[pos]));
	}


      f_writef (fp, "Boards", "%d", MAX_BOARD);
      for (i = 0; i < MAX_BOARD; i++)
	f_printf (fp, " %s " TIME_T_FMT, boards[i].short_name,
		  ch->pcdata->last_note[i]);
      f_printf (fp, LF);

      f_writef (fp, "Subscribe", "%d", MAX_BOARD);
      for (i = 0; i < MAX_BOARD; i++)
	f_printf (fp, " %s %d", boards[i].short_name,
		  ch->pcdata->unsubscribed[i]);
      f_printf (fp, LF);

      for (sn = 0; sn < top_skill; sn++)
	{
	  if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
	    {
	      f_writef (fp, "Sk", "%d '%s'" LF, ch->pcdata->learned[sn],
			skill_table[sn].name);
	    }
	}

      for (gn = 0; gn < top_group; gn++)
	{
	  if (group_table[gn].name != NULL && ch->pcdata->group_known[gn])
	    {
	      f_writef (fp, "Gr", "'%s'" LF, group_table[gn].name);
	    }
	}
    }

  for (paf = ch->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= top_skill)
	continue;

      f_writef (fp, "Affc", "'%s' %3d %3d %3d %3d %3d %s" LF,
		skill_table[paf->type].name, paf->where, paf->level,
		paf->duration, paf->modifier, paf->location,
		write_flags (paf->bitvector));
    }
  write_rle (ch->pcdata->explored, fp);

#ifndef DISABLE_I3

  i3save_char (ch, fp);
#endif

  f_printf (fp, END_MARK LF);
  return;
}


void
write_pet (CharData * pet, FileData * fp)
{
  AffectData *paf;

  f_printf (fp, "#%s" LF, get_char_save_header (SAVE_PET));

  write_int (fp, "Vnum", "%ld", pet->pIndexData->vnum, 0);

  write_string (fp, "Name", pet->name, NULL);
  write_time (fp, "LogO", current_time, false);
  write_string (fp, "ShD", pet->short_descr, pet->pIndexData->short_descr);
  write_string (fp, "LnD", pet->long_descr, pet->pIndexData->long_descr);
  write_string (fp, "Desc", pet->description, pet->pIndexData->description);
  if (pet->race != pet->pIndexData->race)
    write_string (fp, "Race", pet->race->name, NULL);
  write_int (fp, "Sex", "%d", pet->sex, -1);
  write_int (fp, "Levl", "%d", pet->level, pet->pIndexData->level);
  f_writef (fp, "HMV", "%ld %ld %ld %ld %ld %ld" LF, pet->hit,
	    pet->max_hit, pet->mana, pet->max_mana, pet->move, pet->max_move);
  write_int (fp, "Gold", "%ld", pet->gold, 0);
  write_int (fp, "Silv", "%ld", pet->silver, 0);
  write_int (fp, "Exp", "%d", pet->exp, 0);
  write_bit (fp, "Act", pet->act, pet->pIndexData->act);
  write_bit (fp, "AfBy", pet->affected_by, pet->pIndexData->affected_by);
  write_bit (fp, "Comm", pet->comm, 0);
  write_int (fp, "Pos", "%d", pet->position =
	     POS_FIGHTING ? POS_STANDING : pet->position, -1);
  write_int (fp, "Save", "%d", pet->saving_throw, 0);
  write_int (fp, "Alig", "%d", pet->alignment, pet->pIndexData->alignment);
  write_int (fp, "Hit", "%d", pet->hitroll, pet->pIndexData->hitroll);
  write_int (fp, "Dam", "%d", pet->damroll,
	     pet->pIndexData->damage[DICE_BONUS]);

  write_array (fp, "ACs", "%d", pet->armor, MAX_AC);

  write_array (fp, "Attr", "%d", pet->perm_stat, STAT_MAX);

  write_array (fp, "AMod", "%d", pet->mod_stat, STAT_MAX);

  for (paf = pet->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= top_skill)
	continue;

      f_writef (fp, "Affc", "'%s' %3d %3d %3d %3d %3d %s" LF,
		skill_table[paf->type].name, paf->where, paf->level,
		paf->duration, paf->modifier, paf->location,
		write_flags (paf->bitvector));
    }

  f_printf (fp, END_MARK LF);
  return;
}

char *
get_obj_save_header (save_t type)
{
  switch (type)
    {
    case SAVE_CHAR:
      return "OBJECT";
    case SAVE_CORPSE:
      return "CORPSE";
    case SAVE_ROOM:
      return "ROOMOBJ";
    case SAVE_PET:
      return "PETOBJ";
    default:
      bugf ("Bad save type (%d)", type);
      return "OBJECT";
    }
}

char *
get_char_save_header (save_t type)
{
  switch (type)
    {
    case SAVE_CHAR:
      return "PLAYER";
    case SAVE_MOB:
      return "MOB";
    case SAVE_PET:
      return "PET";
    default:
      return "CHAR";
    }
}

#define get_desc_save_header    "DESC"


void
write_obj (CharData * ch, ObjData * obj, FileData * fp, int iNest,
	   int iNext, save_t type)
{
  ExDescrData *ed;
  AffectData *paf;
  vnum_t where = ROOM_VNUM_MORGUE;

  if (!obj)
    return;


  if (obj->prev_content != NULL)
    {
      if (obj->prev_content->in_room == NULL
	  || (type == SAVE_ROOM && iNext < 35))
	write_obj (ch, obj->prev_content, fp, iNest, iNext + 1, type);
    }

  if (type == SAVE_CORPSE || type == SAVE_ROOM)
    {
      if (obj->in_obj != NULL)
	where = 1;
      if (obj->in_room != NULL)
	where = obj->in_room->vnum;
    }


  if (ch && !IsObjStat (obj, ITEM_QUEST) &&
      (type == SAVE_CHAR || type == SAVE_PET) && obj->prev_content == NULL)
    {
      if ((ch->level < obj->level - lvl_bonus (ch)))
	return;

      if ((obj->item_type == ITEM_KEY && (IsNPC (ch)
					  || obj->pIndexData->vnum !=
					  ch->pcdata->home_key))
	  || (obj->item_type == ITEM_MAP && !obj->value[0]))
	return;
    }

  f_printf (fp, "#%s" LF, get_obj_save_header (type));
  write_int (fp, "Vnum", "%ld", obj->pIndexData->vnum, 0);
  if (type == SAVE_CORPSE || type == SAVE_ROOM)
    write_int (fp, "Where", "%ld", where, ROOM_VNUM_MORGUE);
  write_string (fp, "Owner", obj->owner, NULL);
  if (!obj->pIndexData->new_format)
    f_printf (fp, "Oldstyle" LF);
  if (obj->enchanted)
    f_printf (fp, "Enchanted" LF);
  write_int (fp, "Nest", "%d", iNest, -1);



  write_string (fp, "Name", obj->name, obj->pIndexData->name);
  write_string (fp, "ShD", obj->short_descr, obj->pIndexData->short_descr);
  write_string (fp, "Desc", obj->description, obj->pIndexData->description);
  write_bit (fp, "ExtF", obj->extra_flags, obj->pIndexData->extra_flags);
  write_bit (fp, "WeaF", obj->wear_flags, obj->pIndexData->wear_flags);
  write_int (fp, "Ityp", "%d", obj->item_type, obj->pIndexData->item_type);
  write_int (fp, "Wt", "%d", obj->weight, obj->pIndexData->weight);
  write_int (fp, "Cond", "%d", obj->condition, obj->pIndexData->condition);



  write_int (fp, "Wear", "%d", obj->wear_loc, WEAR_NONE);
  write_int (fp, "Lev", "%d", obj->level, obj->pIndexData->level);
  write_int (fp, "Time", "%d", obj->timer, 0);
  write_int (fp, "Cost", "%ld", obj->cost, 0);
  if (obj->enchanted
      || memcmp (obj->value, obj->pIndexData->value, sizeof (obj->value)))
    write_array (fp, "Valu", "%ld", obj->value, 5);

  switch (obj->item_type)
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_PILL:
      if (obj->value[1] > 0)
	{
	  f_writef (fp, "Spell 1", "'%s'" LF,
		    skill_table[obj->value[1]].name);
	}

      if (obj->value[2] > 0)
	{
	  f_writef (fp, "Spell 2", "'%s'" LF,
		    skill_table[obj->value[2]].name);
	}

      if (obj->value[3] > 0)
	{
	  f_writef (fp, "Spell 3", "'%s'" LF,
		    skill_table[obj->value[3]].name);
	}

      break;

    case ITEM_STAFF:
    case ITEM_WAND:
      if (obj->value[3] > 0)
	{
	  f_writef (fp, "Spell 3", "'%s'" LF,
		    skill_table[obj->value[3]].name);
	}

      break;
    default:
      break;
    }

  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
    {
      if (paf->type < 0 || paf->type >= top_skill)
	continue;
      f_writef (fp, "Affc", "'%s' %3d %3d %3d %3d %3d %s" LF,
		skill_table[paf->type].name, paf->where, paf->level,
		paf->duration, paf->modifier, paf->location,
		write_flags (paf->bitvector));
    }

  for (ed = obj->ed_first; ed != NULL; ed = ed->next)
    {
      f_writef (fp, "ExDe", "%s~ %s~" LF, ed->keyword, ed->description);
    }

  f_printf (fp, END_MARK LF);

  if (obj->content_last != NULL)
    write_obj (ch, obj->content_last, fp, iNest + 1, iNext, type);

  return;
}

void
write_descriptor (Descriptor * d, FileData * fp)
{
  f_printf (fp, "#%s" LF, get_desc_save_header);
  write_string (fp, "Host", d->host, NULL);
  write_int (fp, "Descr", "%d", d->descriptor, 0);
  write_int (fp, "Connected", "%d", d->connected, 0);
  write_int (fp, "IP", "%ld", d->ip, 0);
  write_int (fp, "Port", "%d", d->port, 0);
  write_bit (fp, "Flags", d->desc_flags, 0);
  write_int (fp, "ScrW", "%u", d->scr_width, 80);
  write_int (fp, "ScrH", "%u", d->scr_height, 24);
  write_int (fp, "ByteN", "%d", d->bytes_normal, 0);
#ifndef DISABLE_MCCP

  write_int (fp, "ByteC", "%d", d->bytes_compressed, 0);
  write_int (fp, "CVersion", "%d", d->mccp_version, 0);
#endif

  write_string (fp, "TType", d->ttype, NULL);
  if (IsMXP (d))
    {
      write_string (fp, "MXPSup", d->mxp.supports, NULL);
      write_int (fp, "MXPVer", "%.2f", d->mxp.mxp_ver, 0);
      write_int (fp, "MXPClVer", "%.2f", d->mxp.client_ver, 0);
      write_int (fp, "MXPStyl", "%.2f", d->mxp.style_ver, 0);
      write_string (fp, "MXPClien", d->mxp.client, NULL);
      write_int (fp, "MXPReg", "%d", d->mxp.registered, 0);
      write_bit (fp, "MXPFlag1", d->mxp.flags, 0);
      write_bit (fp, "MXPFlag2", d->mxp.flags2, 0);
    }
  if (IsPortal (d))
    {
      write_int (fp, "Keycode", "%u", d->portal.keycode, 0);
      write_string (fp, "PortVer", d->portal.version, NULL);
    }
  if (IsFireCl (d))
    write_int (fp, "IMPver", "%.2f", d->imp_vers, 0);
  if (IsPueblo (d))
    write_int (fp, "Pueblo", "%.2f", d->pueblo_vers, 0);
  f_printf (fp, END_MARK LF);
  return;
}

void
set_player_level (CharData * ch, int Old, int New, int version)
{
  int diff = MAX_LEVEL - LEVEL_IMMORTAL;
  int imm_level = Old - diff;
  int mod = New - Old;

  if (ch->version >= version)
    return;

  if (ch->level >= imm_level)
    ch->level += mod;
  if (ch->trust >= imm_level)
    ch->trust += mod;
  save_char_obj (ch);
}

void
pload_default (CharData * ch)
{
  int stat;

  ch->race = default_race;
  ch->act =
    PLR_NOSUMMON | PLR_AUTOMAP | PLR_AUTOEXIT | PLR_AUTODAMAGE |
    PLR_AUTOASSIST | PLR_AUTOGOLD | PLR_AUTOLOOT | PLR_AUTOSAC |
    PLR_AUTOSPLIT | PLR_AUTOPROMPT;
  ch->comm = COMM_COMBINE | COMM_PROMPT;
  ch->prompt = str_dup (DEFAULT_PROMPT);
  for (stat = 0; stat < STAT_MAX; stat++)
    ch->perm_stat[stat] = 13;
  ch->pcdata->security = 0;
  default_color (ch, -1);
  ch->pcdata->trivia = 0;
  end_quest (ch, 0);
#ifndef DISABLE_I3

  i3init_char (ch);
#endif
}

void
pload_found (CharData * ch)
{
  int i;

  if (ch->race == NULL)
    ch->race = default_race;

  ch->size = ch->race->size;
  ch->dam_type = 17;

  for (i = 0; i < MAX_RACE_SKILL; i++)
    {
      if (ch->race->skills[i] == NULL)
	break;
      group_add (ch, ch->race->skills[i], false);
    }
  ch->affected_by = ch->affected_by | ch->race->aff;
  ch->imm_flags = ch->imm_flags | ch->race->imm;
  ch->res_flags = ch->res_flags | ch->race->res;
  ch->vuln_flags = ch->vuln_flags | ch->race->vuln;
  ch->form = ch->race->form;
  ch->parts = ch->race->parts;


  set_player_level (ch, 0, 0, 0);

  ch->Class[CLASS_COUNT] = 0;
  while (ch->Class[ch->Class[CLASS_COUNT]] != -1)
    ch->Class[CLASS_COUNT] += 1;

  if (ch->version < 11)
    {
      ch->pcdata->home[PC_HOME_COUNT] = 0;
      while (get_room_index (ch->pcdata->home[PC_HOME_COUNT]) != NULL)
	ch->pcdata->home[PC_HOME_COUNT] += 1;
    }

  if (ch->version < 12)
    default_color (ch, -1);
}


bool
load_char_obj (Descriptor * d, const char *name)
{
  CharData *ch;
  static FileData *fp;
  static bool found;
  char buf[MIL];

  ch = new_char ();

  ch->pcdata = new_pcdata ();

  d->character = ch;
  ch->desc = d;
  ch->name = str_dup (capitalize (name));
  ch->id = get_pc_id ();
  pload_default (ch);
  found = false;

  sprintf (buf, "%s.gz", pfilename (name, PFILE_NORMAL));


#ifdef GZFILEIO

  if ((fp = f_open (buf, "rb")) == NULL)
#else

  if ((fp = f_open (buf, "r")) != NULL)
    {
      f_close (fp);
#ifdef unix

      system (FORMATF ("gzip -dfq %s", buf));
#else

      bugf ("File '%s' is compressed!", buf);
      AttemptJump = false;
      return found;
#endif

    }
#endif
  sprintf (buf, pfilename (name, PFILE_NORMAL));
  if ((fp = f_open (buf, "r")) != NULL)
    {
      int iNest;

      AttemptJump = true;

      if (setjmp (jump_env) == 1)
	{
	  size_t iLine, iChar;
	  char c;

	  iChar = f_tell (fp);
	  f_seek (fp, 0, SEEK_SET);
	  for (iLine = 0; (size_t) f_tell (fp) < iChar; iLine++)
	    {
	      while ((c = f_getc (fp)) != '\n' && c != EOF)
		;
	    }
	  f_seek (fp, iChar, SEEK_SET);

	  bugf ("[*LCO*] Error in file '%s' line %d", capitalize (name),
		iLine);
	  f_close (fp);
	  return found;
	}

      for (iNest = 0; iNest < MAX_NEST; iNest++)
	rgObjNest[iNest] = NULL;

      found = true;
      for (;;)
	{
	  char letter;
	  char *word;

	  letter = read_letter (fp);
	  if (letter == '*')
	    {
	      read_to_eol (fp);
	      continue;
	    }

	  if (letter != '#')
	    {
	      bugf ("# not found. (%c)", letter);
	      break;
	    }

	  word = read_word (fp);
	  if (!str_cmp (word, get_char_save_header (SAVE_CHAR)))
	    read_char (ch, fp);
	  else if (!str_cmp (word, get_obj_save_header (SAVE_CHAR)))
	    read_obj (ch, fp, SAVE_CHAR);
	  else if (!str_cmp (word, get_char_save_header (SAVE_PET)))
	    read_pet (ch, fp);
	  else if (!str_cmp (word, get_obj_save_header (SAVE_PET)))
	    read_obj (ch, fp, SAVE_PET);
	  else if (!str_cmp (word, get_desc_save_header))
	    read_descriptor (d, fp);
	  else if (!str_cmp (word, END_MARK))
	    break;
	  else
	    {
	      bug ("Load_char_obj: bad section.");
	      break;
	    }
	}
      f_close (fp);
    }

  AttemptJump = false;


  if (found)
    {
      pload_found (ch);
    }

  return found;
}



void
read_char (CharData * ch, FileData * fp)
{
  char buf[MAX_STRING_LENGTH];
  const char *word;
  bool fMatch;
  int count = 0;
  int count2 = 0;
  int lastlogoff = current_time;
  int percent;
  int ignore = 0;

  logf ("Loading %s.", ch->name);

  for (;;)
    {
      word = f_eof (fp) ? END_MARK : read_word (fp);
      fMatch = false;

      switch (toupper (word[0]))
	{
	case '*':
	  fMatch = true;
	  read_to_eol (fp);
	  break;

	case 'A':
	  Key ("Act", ch->act, read_flag (fp));
	  Key ("AffectedBy", ch->affected_by, read_flag (fp));
	  Key ("AfBy", ch->affected_by, read_flag (fp));
	  Key ("Alignment", ch->alignment, read_number (fp));
	  Key ("Alig", ch->alignment, read_number (fp));
	  Key ("AWins", ch->pcdata->awins, read_number (fp));
	  Key ("ALosses", ch->pcdata->alosses, read_number (fp));

	  if (!str_cmp (word, "Alia"))
	    {
	      if (count >= MAX_ALIAS)
		{
		  read_to_eol (fp);
		  fMatch = true;
		  break;
		}

	      ch->pcdata->alias[count] = str_dup (read_word (fp));
	      ch->pcdata->alias_sub[count] = str_dup (read_word (fp));
	      count++;
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Alias"))
	    {
	      if (count >= MAX_ALIAS)
		{
		  read_to_eol (fp);
		  fMatch = true;
		  break;
		}

	      ch->pcdata->alias[count] = str_dup (read_word (fp));
	      ch->pcdata->alias_sub[count] = read_string (fp);
	      count++;
	      fMatch = true;
	      break;
	    }
	  Key_Ignore ("AC");
	  Key_Ignore ("Armor");

	  if (!str_cmp (word, "ACs"))
	    {
	      if (ch->version < 9)
		{
		  int i;

		  for (i = 0; i < 4; i++)
		    ch->armor[i] = read_number (fp);
		}
	      else
		read_array (fp, ch->armor, MAX_AC, 100);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "AffD"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("read_char: unknown skill.");
	      else
		paf->type = sn;

	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_number (fp);
	      Link (paf, ch->affect, next, prev);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Affc"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("read_char: unknown skill.");
	      else
		paf->type = sn;

	      paf->where = read_enum (where_t, fp);
	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_flag (fp);
	      Link (paf, ch->affect, next, prev);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "AttrMod") || !str_cmp (word, "AMod"))
	    {
	      if (ch->version < 9)
		{
		  int stat;

		  for (stat = 0; stat < STAT_MAX; stat++)
		    ch->mod_stat[stat] = read_number (fp);
		}
	      else
		read_array (fp, ch->mod_stat, STAT_MAX, 3);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "AttrPerm") || !str_cmp (word, "Attr"))
	    {
	      if (ch->version < 9)
		{
		  int stat;

		  for (stat = 0; stat < STAT_MAX; stat++)
		    ch->perm_stat[stat] = read_number (fp);
		}
	      else
		read_array (fp, ch->perm_stat, STAT_MAX, 3);
	      fMatch = true;
	      break;
	    }
	  break;

	case 'B':
	  Key_Str ("Bamfin", ch->pcdata->bamfin);
	  Key_Str ("Bamfout", ch->pcdata->bamfout);
	  Key_Str ("Bin", ch->pcdata->bamfin);
	  Key_Str ("Bout", ch->pcdata->bamfout);
	  Key ("BankG", ch->pcdata->gold_bank, read_number (fp));
	  if (!str_cmp (word, "Buddy"))
	    {
	      if (count2 >= MAX_BUDDY)
		{
		  read_to_eol (fp);
		  fMatch = true;
		  break;
		}
	      ch->pcdata->buddies[count2] = read_string (fp);
	      count2++;
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Boards"))
	    {
	      int i, num = read_number (fp);
	      char *boardname;

	      for (; num; num--)
		{
		  boardname = read_word (fp);
		  i = board_lookup (boardname);

		  if (i == BOARD_NOTFOUND)
		    {
		      sprintf (buf,
			       "read_char: %s had unknown board name: %s. Skipped.",
			       ch->name, boardname);
		      log_string (buf);
		      read_number (fp);
		    }
		  else
		    ch->pcdata->last_note[i] = read_number (fp);
		}

	      fMatch = true;
	      break;
	    }
	  break;

	case 'C':
	  if (!str_cmp (word, "Cla") || !str_cmp (word, "Class"))
	    {
	      if (ch->version < 10)
		{
		  int i, j;

		  for (i = 0; i < MAX_MCLASS; i++)
		    {
		      ch->Class[i] = read_number (fp);
		      if (ch->Class[i] == -1)
			break;
		    }
		  for (i = j = 0; i < MAX_MCLASS; i++)
		    {
		      if (ch->Class[i] < 0 || ch->Class[i] >= top_class)
			{
			  ch->Class[i] = -1;
			  j++;
			  continue;
			}
		      ch->Class[i - j] = ch->Class[i];
		      ch->Class[i - j + 1] = -1;
		    }
		}
	      else
		{
		  read_array (fp, ch->Class, MAX_MCLASS, -1);
		}
	      fMatch = true;
	      break;
	    }
	  Key_SFun ("Clan", ch->pcdata->clan, clan_lookup);
	  if (!str_cmp (word, "Condition") || !str_cmp (word, "Cond"))
	    {
	      ch->pcdata->condition[0] = read_number (fp);
	      ch->pcdata->condition[1] = read_number (fp);
	      ch->pcdata->condition[2] = read_number (fp);
	      fMatch = true;
	      break;
	    }
	  if (!str_cmp (word, "Cnd"))
	    {
	      if (ch->version < 9)
		{
		  ch->pcdata->condition[0] = read_number (fp);
		  ch->pcdata->condition[1] = read_number (fp);
		  ch->pcdata->condition[2] = read_number (fp);
		  ch->pcdata->condition[3] = read_number (fp);
		}
	      else
		read_array (fp, ch->pcdata->condition, 4, 0);
	      fMatch = true;
	      break;
	    }
	  Key ("Comm", ch->comm, read_flag (fp));
	  Key ("Cols", ch->columns, read_number (fp));
	  if (!str_cmp (word, "Colo"))
	    {
	      if (ch->version >= 8)
		{
		  int i, j, num = read_number (fp);

		  for (i = 0; i < Min (num, MAX_CUSTOM_COLOR); i++)
		    {
		      if (i >= MAX_CUSTOM_COLOR)
			break;
		      for (j = 0; j < CT_MAX; j++)
			ch->pcdata->color[i].at[j] = read_number (fp);
		    }
		}
	      read_to_eol (fp);
	      fMatch = true;
	      break;
	    }
	  break;

	case 'D':
	  Key ("Damroll", ch->damroll, read_number (fp));
	  Key ("Dam", ch->damroll, read_number (fp));
	  Key_Str ("Description", ch->description);
	  Key_Str ("Desc", ch->description);
	  Key_SFun ("Deity", ch->deity, deity_lookup);
	  break;

	case 'E':
	  if (!str_cmp (word, END_MARK))
	    {
	      if (ch->in_room == NULL)
		ch->in_room = get_room_index (ROOM_VNUM_LIMBO);


	      percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

	      percent = Min (percent, 100);

	      if (percent > 0 && !IsAffected (ch, AFF_POISON)
		  && !IsAffected (ch, AFF_PLAGUE))
		{
		  ch->hit += (ch->max_hit - ch->hit) * percent / 100;
		  ch->mana += (ch->max_mana - ch->mana) * percent / 100;
		  ch->move += (ch->max_move - ch->move) * percent / 100;
		}
	      return;
	    }
	  Key ("Exp", ch->exp, read_number (fp));
	  break;

	case 'G':
	  Key ("Gold", ch->gold, read_number (fp));
	  if (!str_cmp (word, "GQmobs"))
	    {
	      if (gquest_info.running != GQUEST_OFF)
		{
		  ch->gquest = new_gqlist ();
		  alloc_mem (ch->gquest->gq_mobs, vnum_t,
			     gquest_info.mob_count);
		  read_array (fp, ch->gquest->gq_mobs,
			      gquest_info.mob_count, -1);
		  ch->gquest->ch = ch;
		  Link (ch->gquest, gqlist, next, prev);
		}
	      fMatch = true;
	      break;
	    }
	  if (!str_cmp (word, "Group") || !str_cmp (word, "Gr"))
	    {
	      int gn;
	      char *temp;

	      temp = read_word (fp);
	      gn = group_lookup (temp);

	      if (gn < 0)
		{
		  bugf ("read_char: unknown group (%s). ", temp);
		}
	      else
		gn_add (ch, gn);
	      fMatch = true;
	      break;
	    }
	  Key_Array ("Gstats", ch->pcdata->gamestat, MAX_GAMESTAT, 0);
	  Key_Str ("GProm", ch->gprompt);
	  break;

	case 'H':
	  Key ("Hitroll", ch->hitroll, read_number (fp));
	  Key ("Hit", ch->hitroll, read_number (fp));
	  Key ("HKey", ch->pcdata->home_key, read_number (fp));
	  Key ("HRoom", ch->pcdata->home_room, read_number (fp));
	  Key_Array ("Homes", ch->pcdata->home, MAX_HOME_VNUMS, 0);

	  if (!str_cmp (word, "HpManaMove") || !str_cmp (word, "HMV"))
	    {
	      ch->hit = read_number (fp);
	      ch->max_hit = read_number (fp);
	      ch->mana = read_number (fp);
	      ch->max_mana = read_number (fp);
	      ch->move = read_number (fp);
	      ch->max_move = read_number (fp);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "HpManaMovePerm") || !str_cmp (word, "HMVP"))
	    {
	      ch->pcdata->perm_hit = read_number (fp);
	      ch->pcdata->perm_mana = read_number (fp);
	      ch->pcdata->perm_move = read_number (fp);
	      fMatch = true;
	      break;
	    }

	  break;

	case 'I':
	  Key ("Id", ch->id, read_number (fp));
	  Key ("InvisLevel", ch->invis_level, read_number (fp));
	  Key ("Inco", ch->incog_level, read_number (fp));
	  Key ("Invi", ch->invis_level, read_number (fp));
	  if (!str_cmp (word, "Ignore"))
	    {
	      if (ignore >= MAX_IGNORE)
		{
		  read_to_eol (fp);
		  fMatch = true;
		  break;
		}
	      ch->pcdata->ignore[ignore] = read_string (fp);
	      ch->pcdata->ignore_flags[ignore] = read_flag (fp);
	      ignore++;
	      fMatch = true;
	      break;
	    }
#ifndef DISABLE_I3
	  if ((fMatch = i3load_char (ch, fp, word)))
	    break;
#endif

	  break;

	case 'L':
	  Key ("LastLevel", ch->pcdata->last_level, read_number (fp));
	  Key ("LLev", ch->pcdata->last_level, read_number (fp));
	  Key ("Level", ch->level, read_number (fp));
	  Key ("Lev", ch->level, read_number (fp));
	  Key ("Levl", ch->level, read_number (fp));
	  Key ("LogO", lastlogoff, read_number (fp));
	  Key_Str ("LongDescr", ch->long_descr);
	  Key_Str ("LnD", ch->long_descr);
	  break;

	case 'N':
	  Key_Str ("Name", ch->name);
	  Key_Ignore ("Not");
	  break;

	case 'P':
	  Key_Str ("Password", ch->pcdata->pwd);
	  Key_Str ("Pass", ch->pcdata->pwd);
	  Key ("Played", ch->pcdata->played, read_number (fp));
	  Key ("Plyd", ch->pcdata->played, read_number (fp));
	  Key ("Points", ch->pcdata->points, read_number (fp));
	  Key ("Pnts", ch->pcdata->points, read_number (fp));
	  Key ("Position", ch->position, read_enum (position_t, fp));
	  Key ("Pos", ch->position, read_enum (position_t, fp));
	  Key ("Practice", ch->practice, read_number (fp));
	  Key ("Prac", ch->practice, read_number (fp));
	  Key ("PrClass", ch->pcdata->prime_class, read_number (fp));
	  Key_Str ("Prompt", ch->prompt);
	  Key_Str ("Prom", ch->prompt);
	  break;

	case 'Q':
	  Key ("QuestPnts", ch->pcdata->quest.points, read_number (fp));
	  Key ("QuestTime", ch->pcdata->quest.time, read_number (fp));
	  Key ("QuestStatus", ch->pcdata->quest.status,
	       read_enum (quest_t, fp));
	  Key_Do ("QuestLoc",
		  (ch->pcdata->quest.room =
		   get_room_index (read_number (fp))));
	  Key_Do ("QuestMob",
		  (ch->pcdata->quest.mob =
		   find_quest_char (ch, read_number (fp))));
	  Key_Do ("QuestObj",
		  (ch->pcdata->quest.obj =
		   create_quest_obj (ch, read_number (fp))));
	  Key_Do ("QuestGiver",
		  (ch->pcdata->quest.giver =
		   find_quest_char (ch, read_number (fp))));
	  break;

	case 'R':
	  Key ("Rank", ch->rank, read_number (fp));
	  Key_SFun ("Race", ch->race, race_lookup);
	  Key_Do ("RoomRLE", read_rle (ch->pcdata->explored, fp));
	  Key_Do ("Room", (ch->in_room = get_room_index (read_number (fp))));
	  break;

	case 'S':
	  Key ("SavingThrow", ch->saving_throw, read_number (fp));
	  Key ("Save", ch->saving_throw, read_number (fp));
	  Key ("Scro", ch->lines, read_number (fp));
	  Key ("Sex", ch->sex, read_enum (sex_t, fp));
	  Key_Str ("ShortDescr", ch->short_descr);
	  Key_Str ("ShD", ch->short_descr);
	  Key ("Sec", ch->pcdata->security, read_number (fp));
	  Key ("Silv", ch->silver, read_number (fp));
	  Key ("Shares", ch->pcdata->shares, read_number (fp));
	  Key ("StrEdKey", ch->pcdata->str_ed_key, read_letter (fp));
	  Key_Do ("StayRace", (ch->pcdata->stay_race = true));
	  Key_Array ("Stance", ch->stance, MAX_STANCE, 0);
	  if (!str_cmp (word, "Skill") || !str_cmp (word, "Sk"))
	    {
	      int sn;
	      int value;
	      char *temp;

	      value = read_number (fp);
	      temp = read_word (fp);
	      sn = skill_lookup (temp);

	      if (sn < 0)
		{
		  bugf ("read_char: unknown skill. (%s)", temp);
		}
	      else
		ch->pcdata->learned[sn] = value;
	      fMatch = true;
	      break;
	    }
	  if (!str_cmp (word, "Subscribe"))
	    {
	      int i, num = read_number (fp);

	      char *boardname;

	      for (; num; num--)
		{

		  boardname = read_word (fp);
		  i = board_lookup (boardname);

		  if (i == BOARD_NOTFOUND)
		    {

		      sprintf (buf,
			       "read_char: %s had unknown board name: %s. Skipped.",
			       ch->name, boardname);
		      log_string (buf);
		      read_number (fp);

		    }
		  else

		    ch->pcdata->unsubscribed[i] = read_number (fp);
		}

	      fMatch = true;
	      break;
	    }

	  break;

	case 'T':
	  Key ("TrueSex", ch->pcdata->true_sex, read_enum (sex_t, fp));
	  Key ("TSex", ch->pcdata->true_sex, read_enum (sex_t, fp));
	  Key ("Trai", ch->train, read_number (fp));
	  Key ("Trust", ch->trust, read_number (fp));
	  Key ("Tru", ch->trust, read_number (fp));
	  Key ("Trivia", ch->pcdata->trivia, read_number (fp));
	  Key ("TimeZone", ch->pcdata->timezone, read_number (fp));
	  if (!str_cmp (word, "Title") || !str_cmp (word, "Titl"))
	    {
	      ch->pcdata->title = read_string (fp);
	      if (ch->pcdata->title[0] != '.' &&
		  ch->pcdata->title[0] != ',' &&
		  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
		{
		  sprintf (buf, " %s", ch->pcdata->title);
		  replace_str (&ch->pcdata->title, buf);
		}
	      fMatch = true;
	      break;
	    }

	  break;

	case 'V':
	  Key ("Version", ch->version, read_number (fp));
	  Key ("Vers", ch->version, read_number (fp));
	  Key ("Vt100", ch->pcdata->vt100, read_flag (fp));
	  Key_Do ("Vnum",
		  (ch->pIndexData = get_char_index (read_number (fp))));
	  break;

	case 'W':
	  if (!str_cmp (word, "WarInfo"))
	    {
	      if (war_info.status != WAR_OFF)
		{
		  ch->war = new_warlist ();
		  ch->war->hit = read_number (fp);
		  ch->war->mana = read_number (fp);
		  ch->war->move = read_number (fp);
		  ch->war->flags = read_flag (fp);
		  ch->war->ch = ch;
		  Link (ch->war, warlist, next, prev);
		}
	      fMatch = true;
	      break;
	    }
	  Key ("Wimpy", ch->wimpy, read_number (fp));
	  Key ("Wimp", ch->wimpy, read_number (fp));
	  Key ("Wizn", ch->wiznet, read_flag (fp));
	  Key_Str ("WhoD", ch->pcdata->who_descr);
	  break;
	}

      if (!fMatch)
	{
	  bugf ("read_char: no match for %s->%s.", ch->name, word);
	  read_to_eol (fp);
	}
    }
}


void
read_pet (CharData * ch, FileData * fp)
{
  const char *word;
  CharData *pet;
  bool fMatch;
  int lastlogoff = current_time;
  int percent;


  word = f_eof (fp) ? END_MARK : read_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      vnum_t vnum;

      vnum = read_number (fp);
      if (get_char_index (vnum) == NULL)
	{
	  bugf ("Fread_pet: bad vnum %ld.", vnum);
	  pet = create_mobile (get_char_index (MOB_VNUM_FIDO));
	}
      else
	pet = create_mobile (get_char_index (vnum));
    }
  else
    {
      bug ("Fread_pet: no vnum in file.");
      pet = create_mobile (get_char_index (MOB_VNUM_FIDO));
    }

  for (;;)
    {
      word = f_eof (fp) ? END_MARK : read_word (fp);
      fMatch = false;

      switch (toupper (word[0]))
	{
	case '*':
	  fMatch = true;
	  read_to_eol (fp);
	  break;

	case 'A':
	  Key ("Act", pet->act, read_flag (fp));
	  Key ("AfBy", pet->affected_by, read_flag (fp));
	  Key ("Alig", pet->alignment, read_number (fp));

	  if (!str_cmp (word, "ACs"))
	    {
	      if (pet->version < 9)
		{
		  int i;

		  for (i = 0; i < 4; i++)
		    pet->armor[i] = read_number (fp);
		}
	      else
		read_array (fp, pet->armor, MAX_AC, 100);

	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "AffD"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("read_char: unknown skill.");
	      else
		paf->type = sn;

	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_number (fp);
	      Link (paf, pet->affect, next, prev);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Affc"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("read_char: unknown skill.");
	      else
		paf->type = sn;

	      paf->where = read_enum (where_t, fp);
	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_flag (fp);
	      Link (paf, pet->affect, next, prev);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "AMod"))
	    {
	      if (pet->version < 9)
		{
		  int stat;

		  for (stat = 0; stat < STAT_MAX; stat++)
		    pet->mod_stat[stat] = read_number (fp);
		}
	      else
		read_array (fp, pet->mod_stat, STAT_MAX, 0);
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Attr"))
	    {
	      if (ch->version < 9)
		{
		  int stat;

		  for (stat = 0; stat < STAT_MAX; stat++)
		    pet->perm_stat[stat] = read_number (fp);
		}
	      else
		read_array (fp, pet->perm_stat, STAT_MAX, 3);
	      fMatch = true;
	      break;
	    }
	  break;

	case 'C':
	  Key ("Comm", pet->comm, read_flag (fp));
	  break;

	case 'D':
	  Key ("Dam", pet->damroll, read_number (fp));
	  Key_Str ("Desc", pet->description);
	  break;

	case 'E':
	  if (!str_cmp (word, END_MARK))
	    {
	      pet->leader = ch;
	      pet->master = ch;
	      ch->pet = pet;

	      percent = (current_time - lastlogoff) * 25 / (2 * 60 * 60);

	      if (percent > 0 && !IsAffected (ch, AFF_POISON)
		  && !IsAffected (ch, AFF_PLAGUE))
		{
		  percent = Min (percent, 100);
		  pet->hit += (pet->max_hit - pet->hit) * percent / 100;
		  pet->mana += (pet->max_mana - pet->mana) * percent / 100;
		  pet->move += (pet->max_move - pet->move) * percent / 100;
		}
	      return;
	    }
	  Key ("Exp", pet->exp, read_number (fp));
	  break;

	case 'G':
	  Key ("Gold", pet->gold, read_number (fp));
	  break;

	case 'H':
	  Key ("Hit", pet->hitroll, read_number (fp));

	  if (!str_cmp (word, "HMV"))
	    {
	      pet->hit = read_number (fp);
	      pet->max_hit = read_number (fp);
	      pet->mana = read_number (fp);
	      pet->max_mana = read_number (fp);
	      pet->move = read_number (fp);
	      pet->max_move = read_number (fp);
	      fMatch = true;
	      break;
	    }
	  break;

	case 'L':
	  Key ("Levl", pet->level, read_number (fp));
	  Key_Str ("LnD", pet->long_descr);
	  Key ("LogO", lastlogoff, read_number (fp));
	  break;

	case 'N':
	  Key_Str ("Name", pet->name);
	  break;

	case 'P':
	  Key ("Pos", pet->position, read_enum (position_t, fp));
	  break;

	case 'R':
	  Key_SFun ("Race", pet->race, race_lookup);
	  break;

	case 'S':
	  Key ("Save", pet->saving_throw, read_number (fp));
	  Key ("Sex", pet->sex, read_enum (sex_t, fp));
	  Key_Str ("ShD", pet->short_descr);
	  Key ("Silv", pet->silver, read_number (fp));
	  break;

	  if (!fMatch)
	    {
	      bug ("Fread_pet: no match.");
	      read_to_eol (fp);
	    }

	}
    }
}

void
read_obj (CharData * ch, FileData * fp, save_t type)
{
  ObjData *obj;
  const char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  bool new_format;
  bool make_new;
  vnum_t where;

  fVnum = false;
  obj = NULL;
  first = true;
  new_format = false;
  make_new = false;

  word = f_eof (fp) ? END_MARK : read_word (fp);
  if (!str_cmp (word, "Vnum"))
    {
      vnum_t vnum;
      ObjIndex *pObj;

      first = false;

      vnum = read_number (fp);
      if ((pObj = get_obj_index (vnum)) == NULL)
	{
	  bugf ("Fread_obj: bad vnum %ld.", vnum);
	}
      else
	{
	  obj = create_object (pObj, -1);
	  new_format = true;
	  fVnum = true;
	}

    }

  if (obj == NULL)
    {
      obj = new_obj ();
      obj->name = str_dup ("");
      obj->short_descr = str_dup ("");
      obj->description = str_dup ("");
    }

  fNest = false;
  fVnum = true;
  iNest = 0;
  where = type == SAVE_CORPSE ? ROOM_VNUM_MORGUE : 0;

  for (;;)
    {
      if (first)
	first = false;
      else
	word = f_eof (fp) ? END_MARK : read_word (fp);
      fMatch = false;

      switch (toupper (word[0]))
	{
	case '*':
	  fMatch = true;
	  read_to_eol (fp);
	  break;

	case 'A':
	  if (!str_cmp (word, "AffD"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("Fread_obj: unknown skill.");
	      else
		paf->type = sn;

	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_number (fp);
	      Link (paf, obj->affect, next, prev);
	      fMatch = true;
	      break;
	    }
	  if (!str_cmp (word, "Affc"))
	    {
	      AffectData *paf;
	      int sn;

	      paf = new_affect ();

	      sn = skill_lookup (read_word (fp));
	      if (sn < 0)
		bug ("Fread_obj: unknown skill.");
	      else
		paf->type = sn;

	      paf->where = read_enum (where_t, fp);
	      paf->level = read_number (fp);
	      paf->duration = read_number (fp);
	      paf->modifier = read_number (fp);
	      paf->location = read_enum (apply_t, fp);
	      paf->bitvector = read_flag (fp);
	      Link (paf, obj->affect, next, prev);
	      fMatch = true;
	      break;
	    }
	  break;

	case 'C':
	  Key ("Cond", obj->condition, read_number (fp));
	  Key ("Cost", obj->cost, read_number (fp));
	  break;

	case 'D':
	  Key_Str ("Description", obj->description);
	  Key_Str ("Desc", obj->description);
	  break;

	case 'E':
	  Key_Do ("Enchanted", (obj->enchanted = true));
	  Key ("ExtraFlags", obj->extra_flags, read_number (fp));
	  Key ("ExtF", obj->extra_flags, read_flag (fp));

	  if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe"))
	    {
	      ExDescrData *ed;

	      ed = new_ed ();

	      ed->keyword = read_string (fp);
	      ed->description = read_string (fp);
	      Link (ed, obj->ed, next, prev);
	      fMatch = true;
	    }

	  if (!str_cmp (word, END_MARK))
	    {
	      if (!fNest || (fVnum && obj->pIndexData == NULL))
		{
		  bug ("Fread_obj: incomplete object.");
		  free_obj (obj);
		  return;
		}
	      else
		{
		  if (!fVnum)
		    {
		      free_obj (obj);
		      obj = create_object (get_obj_index (OBJ_VNUM_DUMMY), 0);
		    }

		  if (!new_format)
		    {
		      Link (obj, obj, next, prev);
		      obj->pIndexData->count++;
		    }

		  if (!obj->pIndexData->new_format &&
		      obj->item_type == ITEM_ARMOR && obj->value[1] == 0)
		    {
		      obj->value[1] = obj->value[0];
		      obj->value[2] = obj->value[0];
		    }
		  if (make_new)
		    {
		      wloc_t wear;

		      wear = obj->wear_loc;
		      extract_obj (obj);

		      obj = create_object (obj->pIndexData, 0);
		      obj->wear_loc = wear;
		    }
		  if (iNest == 0 || rgObjNest[iNest] == NULL)
		    {
		      switch (type)
			{
			case SAVE_CORPSE:
			  {
			    RoomIndex *Room;
			    CorpseData *c;

			    c = new_corpse ();
			    c->corpse = obj;
			    Link (c, corpse, next, prev);
			    if ((Room = get_room_index (where)) == NULL)
			      Room = get_room_index (ROOM_VNUM_MORGUE);
			    obj_to_room (obj, Room);
			  }
			  break;
			case SAVE_ROOM:
			  {
			    RoomIndex *Room;

			    if ((Room = get_room_index (where)) == NULL)
			      extract_obj (obj);
			    else
			      obj_to_room (obj, Room);
			  }
			  break;
			case SAVE_CHAR:
			  obj_to_char (obj, ch);
			  break;
			case SAVE_PET:
			  obj_to_char (obj, ch->pet ? ch->pet : ch);
			  break;
			default:
			  bugf ("Bad save type (%d)", type);
			  break;
			}
		    }
		  else
		    obj_to_obj (obj, rgObjNest[iNest - 1]);
		  return;
		}
	    }
	  break;

	case 'I':
	  Key ("ItemType", obj->item_type, read_enum (item_t, fp));
	  Key ("Ityp", obj->item_type, read_enum (item_t, fp));
	  break;

	case 'L':
	  Key ("Level", obj->level, read_number (fp));
	  Key ("Lev", obj->level, read_number (fp));
	  break;

	case 'N':
	  Key_Str ("Name", obj->name);

	  if (!str_cmp (word, "Nest"))
	    {
	      iNest = read_number (fp);
	      if (iNest < 0 || iNest >= MAX_NEST)
		{
		  bugf ("Fread_obj: bad nest %d.", iNest);
		}
	      else
		{
		  rgObjNest[iNest] = obj;
		  fNest = true;
		}
	      fMatch = true;
	    }
	  break;

	case 'O':
	  if (!str_cmp (word, "Oldstyle"))
	    {
	      if (obj->pIndexData != NULL && obj->pIndexData->new_format)
		make_new = true;
	      fMatch = true;
	      break;
	    }
	  Key_Str ("Owner", obj->owner);
	  break;

	case 'S':
	  Key_Str ("ShortDescr", obj->short_descr);
	  Key_Str ("ShD", obj->short_descr);

	  if (!str_cmp (word, "Spell"))
	    {
	      int iValue;
	      int sn;

	      iValue = read_number (fp);
	      sn = skill_lookup (read_word (fp));
	      if (iValue < 0 || iValue > 3)
		{
		  bugf ("Fread_obj: bad iValue %d.", iValue);
		}
	      else if (sn < 0)
		{
		  bug ("Fread_obj: unknown skill.");
		}
	      else
		{
		  obj->value[iValue] = sn;
		}
	      fMatch = true;
	      break;
	    }

	  break;

	case 'T':
	  Key ("Timer", obj->timer, read_number (fp));
	  Key ("Time", obj->timer, read_number (fp));
	  break;

	case 'V':
	  if (!str_cmp (word, "Values") || !str_cmp (word, "Vals"))
	    {
	      obj->value[0] = read_number (fp);
	      obj->value[1] = read_number (fp);
	      obj->value[2] = read_number (fp);
	      obj->value[3] = read_number (fp);
	      if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
		obj->value[0] = obj->pIndexData->value[0];
	      fMatch = true;
	      break;
	    }

	  if (!str_cmp (word, "Val"))
	    {
	      obj->value[0] = read_number (fp);
	      obj->value[1] = read_number (fp);
	      obj->value[2] = read_number (fp);
	      obj->value[3] = read_number (fp);
	      obj->value[4] = read_number (fp);
	      fMatch = true;
	      break;
	    }

	  Key_Array ("Valu", obj->value, 5, 0);

	  if (!str_cmp (word, "Vnum"))
	    {
	      vnum_t vnum;
	      ObjIndex *pObj;

	      vnum = read_number (fp);
	      if ((pObj = get_obj_index (vnum)) == NULL)
		bugf ("Fread_obj: bad vnum %ld.", vnum);
	      else
		{
		  obj->pIndexData = pObj;
		  fVnum = true;
		}
	      fMatch = true;
	      break;
	    }
	  break;

	case 'W':
	  Key ("WearFlags", obj->wear_flags, read_number (fp));
	  Key ("WeaF", obj->wear_flags, read_flag (fp));
	  Key ("WearLoc", obj->wear_loc, read_enum (wloc_t, fp));
	  Key ("Wear", obj->wear_loc, read_enum (wloc_t, fp));
	  Key ("Weight", obj->weight, read_number (fp));
	  Key ("Wt", obj->weight, read_number (fp));
	  Key ("Where", where, read_number (fp));
	  break;

	}

      if (!fMatch)
	{
	  bug ("Fread_obj: no match.");
	  read_to_eol (fp);
	}
    }
}

void
read_descriptor (Descriptor * d, FileData * fp)
{
  const char *word;
  bool fMatch;

  for (;;)
    {
      word = f_eof (fp) ? END_MARK : read_word (fp);
      fMatch = false;
      if (crs_info.status != CRS_COPYOVER_RECOVER)
	{
	  if (!str_cmp (word, END_MARK))
	    return;
	  else
	    {
	      read_to_eol (fp);
	      continue;
	    }
	}
      switch (toupper (word[0]))
	{
	case '*':
	  fMatch = true;
	  read_to_eol (fp);
	  break;
	case 'B':
	  Key ("ByteN", d->bytes_normal, read_number (fp));
#ifndef DISABLE_MCCP

	  Key ("ByteC", d->bytes_compressed, read_number (fp));
#endif

	  break;
	case 'C':
	  Key ("Connected", d->connected, read_enum (connect_t, fp));
#ifndef DISABLE_MCCP

	  Key ("CVersion", d->mccp_version, read_number (fp));
#endif

	  break;
	case 'D':
	  Key ("Descr", d->descriptor, read_number (fp));
	  break;
	case 'E':
	  if (!str_cmp (word, END_MARK))
	    return;
	  break;
	case 'F':
	  Key ("Flags", d->desc_flags, read_flag (fp));
	  break;
	case 'H':
	  Key_Str ("Host", d->host);
	  break;
	case 'I':
	  Key_Do ("IMPver", d->imp_vers = atof (read_word (fp)));
	  Key ("IP", d->ip, read_number (fp));
	  break;
	case 'K':
	  Key ("Keycode", d->portal.keycode, read_number (fp));
	  break;
	case 'M':
	  Key_Str ("MXPSup", d->mxp.supports);
	  Key_Do ("MXPVer", d->mxp.mxp_ver = atof (read_word (fp)));
	  Key_Do ("MXPClVer", d->mxp.client_ver = atof (read_word (fp)));
	  Key_Do ("MXPStyl", d->mxp.style_ver = atof (read_word (fp)));
	  Key_Str ("MXPClien", d->mxp.client);
	  Key ("MXPReg", d->mxp.registered, read_number (fp));
	  Key ("MXPFlag1", d->mxp.flags, read_flag (fp));
	  Key ("MXPFlag2", d->mxp.flags2, read_flag (fp));
	  break;
	case 'P':
	  Key_StrCpy ("PortVer", d->portal.version);
	  Key_Do ("Pueblo", d->pueblo_vers = atof (read_word (fp)));
	  Key ("Port", d->port, read_number (fp));
	  break;
	case 'S':
	  Key ("ScrW", d->scr_width, read_number (fp));
	  Key ("ScrH", d->scr_height, read_number (fp));
	  break;
	case 'T':
	  Key_StrCpy ("TType", d->ttype);
	  break;
	}

      if (!fMatch)
	{
	  bugf ("no match for %s.", word);
	  read_to_eol (fp);
	}
    }
}

void
save_corpses (void)
{
  FileData *fp;
  CorpseData *c;

  if ((fp = f_open (CORPSE_FILE, "w")) == NULL)
    {
      bug ("save_corpses: " CORPSE_FILE " not found.");
    }
  else
    {
      for (c = corpse_first; c != NULL; c = c->next)
	{
	  if (c->corpse->item_type == ITEM_CORPSE_PC)
	    write_obj (NULL, c->corpse, fp, 0, 0, SAVE_CORPSE);
	  else
	    update_corpses (c->corpse, true);
	}
      f_printf (fp, "#" END_MARK LF);
      f_close (fp);
    }
  return;
}

void
load_corpses (void)
{
  FileData *fp;

  log_string ("Loading corpses...");

  if ((fp = f_open (CORPSE_FILE, "r")) == NULL)
    {
      bug ("load_corpses: " CORPSE_FILE " not found");
    }
  else
    {
      for (;;)
	{
	  char letter;
	  char *word;

	  letter = read_letter (fp);
	  if (letter == '*')
	    {
	      read_to_eol (fp);
	      continue;
	    }

	  if (letter != '#')
	    {
	      bugf ("# not found. (%c)", letter);
	      break;
	    }

	  word = read_word (fp);
	  if (!str_cmp (word, get_obj_save_header (SAVE_CORPSE)))
	    read_obj (NULL, fp, SAVE_CORPSE);
	  else if (!str_cmp (word, END_MARK))
	    break;
	  else
	    {
	      bug ("load_corpses: bad section.");
	      break;
	    }
	}
      f_close (fp);
    }
  return;
}

void
update_corpses (ObjData * obj, bool pdelete)
{
  if (obj && obj->item_type == ITEM_CORPSE_PC)
    {
      CorpseData *c;

      for (c = corpse_first; c != NULL; c = c->next)
	if (c->corpse == obj)
	  break;
      if (c != NULL)
	{
	  if (pdelete)
	    {
	      UnLink (c, corpse, next, prev);
	      free_corpse (c);
	      save_corpses ();
	    }
	}
      else if (obj->content_first != NULL && obj->in_room != NULL)
	{
	  c = new_corpse ();
	  c->corpse = obj;
	  Link (c, corpse, next, prev);
	  save_corpses ();
	}
    }
  return;
}

void
checkcorpse (CharData * ch)
{
  CorpseData *c;
  bool found = false;
  int count = 0;

  if (!ch || IsNPC (ch))
    return;

  for (c = corpse_first; c != NULL; c = c->next)
    {
      if (c->corpse && is_name (ch->name, c->corpse->owner))
	{
	  found = true;
	  count++;
	}
    }
  if (found)
    {
      chprintlnf
	(ch, NEWLINE "{f{RWARNING:{x {WYou have %s in the game.{x",
	 intstr (count, "corpse"));
      new_wiznet (ch, intstr (count, "corpse"), 0, 0, 0,
		  "$N has $t in the game.");
    }
}
