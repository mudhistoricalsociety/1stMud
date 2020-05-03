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

int
bitcount (char c)
{
  int count = 0;

  if (c & BIT_A)
    count++;
  if (c & BIT_B)
    count++;
  if (c & BIT_C)
    count++;
  if (c & BIT_D)
    count++;
  if (c & BIT_E)
    count++;
  if (c & BIT_F)
    count++;
  if (c & BIT_G)
    count++;
  if (c & BIT_H)
    count++;

  return count;
}

int
roomcount (CharData * ch)
{
  int pIndex = 0, count = 0;

  if (IsNPC (ch))
    return top_room_index;

  for (pIndex = 0; pIndex < MAX_EXPLORE_HASH; pIndex++)
    {
      count += bitcount (ch->pcdata->explored[pIndex]);
    }

  return count;
}

void
update_explored (CharData * ch)
{
  vnum_t vnum;
  RoomIndex *pRoom;
  int nMatch = 0;

  for (vnum = 0; nMatch < top_room_index; vnum++)
    {
      if ((pRoom = get_room_index (vnum)) != NULL)
	{
	  nMatch++;
	  if (IsSet (pRoom->room_flags, ROOM_NOEXPLORE)
	      && StrIsSet (ch->pcdata->explored, vnum))
	    StrRemBit (ch->pcdata->explored, vnum);
	}
      else
	{
	  if (StrIsSet (ch->pcdata->explored, vnum))
	    StrRemBit (ch->pcdata->explored, vnum);
	}
    }
}

int
areacount (CharData * ch, AreaData * area)
{
  vnum_t pIndex = 0;
  int count = 0;

  if (ch == NULL || area == NULL)
    return 0;

  if (IsNPC (ch))
    return top_room_index;

  for (pIndex = area->min_vnum; pIndex <= area->max_vnum; pIndex++)
    {
      count += StrIsSet (ch->pcdata->explored, pIndex) ? 1 : 0;
    }

  return count;
}

int
arearooms (AreaData * area)
{
  int count = 0;
  vnum_t pIndex = 0;
  RoomIndex *pRoom;

  if (!area)
    return 0;

  for (pIndex = area->min_vnum; pIndex <= area->max_vnum; pIndex++)
    {
      if ((pRoom = get_room_index (pIndex)) != NULL
	  && !IsSet (pRoom->room_flags, ROOM_NOEXPLORE))
	count++;
    }

  return count;
}

void
write_rle (char *explored, FileData * fp)
{
  vnum_t pIndex;
  int bit = 0;
  int count = 0;

  f_writef (fp, "RoomRLE", "%d", bit);

  for (pIndex = 0; pIndex < top_vnum_room; pIndex++)
    {
      if ((StrIsSet (explored, pIndex) ? 1 : 0) == bit)
	count++;
      else
	{
	  f_printf (fp, " %d", count);
	  count = 1;
	  bit = (StrIsSet (explored, pIndex)) ? 1 : 0;
	}
    }
  f_printf (fp, " %d -1" LF, count);
  return;
}

void
read_rle (char *explored, FileData * fp)
{
  vnum_t index;
  int bit = 0;
  int count = 0;
  vnum_t pos = 0;

  index = 0;

  bit = read_number (fp);

  for (;;)
    {
      count = read_number (fp);

      if (count < 0)
	break;
      if (count == 0)
	continue;

      do
	{
	  if (bit == 1)
	    {
	      StrSetBit (explored, index);
	    }
	  index++;
	}
      while (index < pos + count);

      pos = index;
      bit = (bit == 1) ? 0 : 1;
    }
  return;
}

int
compare_area_explored (const void *v1, const void *v2)
{
  AreaIndex area1 = *(AreaIndex *) v1;
  AreaIndex area2 = *(AreaIndex *) v2;

  return (int) (area2.percent - area1.percent);
}

Do_Fun (do_explored)
{
  int i = 0, c = 0;
  double rooms, rcnt, percent;
  AreaData *pArea;
  AreaIndex *list;

  if (!ch || IsNPC (ch))
    return;

  if (NullStr (argument))
    {
      rcnt = (double) roomcount (ch);
      rooms = (double) top_explored;

      chprintlnf (ch, "ROM has {G%d{x explorable rooms.", top_explored);
      chprintlnf (ch, "You have explored {G%.0f (%.2f%%){x of the mud{x",
		  rcnt, Percent (rcnt, rooms));

      rcnt = (double) areacount (ch, ch->in_room->area);
      rooms = (double) (arearooms (ch->in_room->area));

      chprintlnf (ch, "This area has {G%.0f{x explorable rooms.", rooms);
      chprintlnf (ch,
		  "You have explored {G%.0f (%.2f%%){x rooms in this area.{x",
		  rcnt, Percent (rcnt, rooms));
    }
  else if (is_exact_name (argument, "reset"))
    {
      memset (ch->pcdata->explored, 0, MAX_EXPLORE_HASH);
      chprintln (ch, "Your explored rooms were set to 0.");
    }
  else if (!str_prefix (argument, "list"))
    {
      Buffer *output = new_buf ();
      Column *Cd = new_column ();

      set_cols (Cd, ch, 2, COLS_BUF, output);
      alloc_mem (list, AreaIndex, top_area);

      for (pArea = area_first; pArea != NULL; pArea = pArea->next)
	{
	  rcnt = (double) (areacount (ch, pArea));
	  rooms = (double) (arearooms (pArea));
	  list[i].area = pArea;
	  list[i].percent = Percent (rcnt, rooms);
	  i++;
	}
      qsort (list, i, sizeof (AreaIndex), compare_area_explored);

      for (c = 0; c < i; c++)
	{
	  pArea = list[c].area;
	  percent = list[c].percent;

	  print_cols (Cd, "{D[{Y%3.0f{y%%{D]{x %s", percent, pArea->name);
	}
      cols_nl (Cd);
      sendpage (ch, buf_string (output));
      free_buf (output);
      free_column (Cd);
      free_mem (list);
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun,
		  "        - show current area and world.",
		  "list    - list percentages for all areas.",
		  "reset   - reset explored rooms.", NULL);
    }
}
