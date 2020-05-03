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
#include "olc.h"
#include "interp.h"

FlagTable *
flag_lookup (const char *name, FlagTable * f)
{
  if (NullStr (name))
    return NULL;

  while (f->name != NULL)
    {
      if (!str_prefix (name, f->name))
	return f;
      f++;
    }
  return NULL;
}

ClanData *
clan_lookup (const char *name)
{
  ClanData *clan;

  for (clan = clan_first; clan; clan = clan->next)
    {
      if (tolower (name[0]) == tolower (clan->name[0]) &&
	  !str_prefix (name, clan->name))
	return clan;
    }

  return NULL;
}


RaceData *
race_lookup (const char *name)
{
  RaceData *race;

  for (race = race_first; race != NULL; race = race->next)
    {
      if (tolower (name[0]) == tolower (race->name[0]) &&
	  !str_prefix (name, race->name))
	return race;
    }

  return NULL;
}

Lookup_Fun (liq_lookup)
{
  int liq;

  for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
      if (tolower (name[0]) == tolower (liq_table[liq].liq_name[0]) &&
	  !str_prefix (name, liq_table[liq].liq_name))
	return liq;
    }

  return -1;
}

Lookup_Fun (stance_lookup)
{
  int st;

  for (st = 0; st < MAX_STANCE; st++)
    {
      if (!str_prefix (name, stance_table[st].name))
	return st;
    }
  return -1;
}

HelpData *
help_lookup (const char *keyword)
{
  HelpData *pHelp;
  char temp[MIL], argall[MIL];

  argall[0] = '\0';

  while (!NullStr (keyword))
    {
      keyword = one_argument (keyword, temp);
      if (!NullStr (argall))
	strcat (argall, " ");
      strcat (argall, temp);
    }

  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    if (is_name (argall, pHelp->keyword))
      return pHelp;

  return NULL;
}

CmdData *
cmd_lookup (const char *name)
{
  int hash;
  CmdData *pCmd;

  if (NullStr (name))
    return NULL;

  hash = tolower (name[0]) % MAX_CMD_HASH;

  for (pCmd = command_hash[hash]; pCmd; pCmd = pCmd->next_hash)
    if (!str_prefix (name, pCmd->name))
      return pCmd;

  return NULL;
}

DeityData *
deity_lookup (const char *arg)
{
  DeityData *i;

  for (i = deity_first; i; i = i->next)
    if (!str_prefix (arg, i->name))
      return i;

  return NULL;
}

Lookup_Fun (tzone_lookup)
{
  int i;

  for (i = 0; i < MAX_TZONE; i++)
    {
      if (!str_cmp (name, tzone_table[i].name))
	return i;
    }

  for (i = 0; i < MAX_TZONE; i++)
    {
      if (is_name (name, tzone_table[i].zone))
	return i;
    }

  for (i = 0; i < MAX_TZONE; i++)
    {
      if (atoi (name) - 1 == i)
	return i;
    }

  return -1;
}

AreaData *
area_lookup (const char *arg)
{
  AreaData *pArea;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (is_number (arg) && atoi (arg) == pArea->vnum)
	return pArea;

      else if (tolower (arg[0]) == tolower (pArea->name[0])
	       && !str_prefix (arg, pArea->name))
	return pArea;

      else if (tolower (arg[0]) == tolower (pArea->file_name[0])
	       && !str_prefix (arg, pArea->file_name))
	return pArea;

    }
  return NULL;
}

RoomIndex *
area_begin (AreaData * pArea)
{
  RoomIndex *pRoom;
  vnum_t vnum;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    if (vnum > 0 && (pRoom = get_room_index (vnum)) != NULL)
      return pRoom;

  return NULL;
}

Lookup_Fun (channel_lookup)
{
  int i;

  for (i = 0; i < top_channel; i++)
    {
      if (!str_prefix (name, channel_table[i].name))
	return i;
    }
  return -1;
}


SocialData *
social_lookup (const char *name)
{
  SocialData *i;

  for (i = social_first; i; i = i->next)
    if (!str_cmp (name, i->name))
      return i;

  return NULL;
}

Lookup_Fun (get_direction)
{
  if (!str_cmp (name, "n") || !str_cmp (name, "north"))
    return DIR_NORTH;
  if (!str_cmp (name, "e") || !str_cmp (name, "east"))
    return DIR_EAST;
  if (!str_cmp (name, "s") || !str_cmp (name, "south"))
    return DIR_SOUTH;
  if (!str_cmp (name, "w") || !str_cmp (name, "west"))
    return DIR_WEST;
  if (!str_cmp (name, "u") || !str_cmp (name, "up"))
    return DIR_UP;
  if (!str_cmp (name, "d") || !str_cmp (name, "down"))
    return DIR_DOWN;

  return -1;
}
