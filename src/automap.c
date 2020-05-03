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

char map_chars[5] = "|-|-";
char map_chars_closed[5] = "I=I=";
char lcolor = 'x';
int depth = 0;

#define    MAXDEPTH  4
#define    MAPX     10
#define    MAPY      8
#define    BOUNDARY(x, y) (((x) < 0) || ((y) < 0) || \
((x) > (MAPX * 2)) || ((y) > (MAPY * 2))) \

struct map_type
{

  char symbol;
  int depth;
  RoomIndex *pRoom;
};

typedef struct map_type MAP_DATA;

MAP_DATA map[(MAPX * 2) + 1][(MAPY * 2) + 1];

void
get_exit_dir (int dir, int *x, int *y, int xorig, int yorig)
{
  switch (dir)
    {
    case 0:
      *x = xorig;
      *y = yorig - 1;
      break;
    case 1:
      *x = xorig + 1;
      *y = yorig;
      break;
    case 2:
      *x = xorig;
      *y = yorig + 1;
      break;
    case 3:
      *x = xorig - 1;
      *y = yorig;
      break;
    default:
      *x = -1;
      *y = -1;
      break;
    }
}

void
clear_coord (int x, int y)
{
  map[x][y].symbol = ' ';
  map[x][y].depth = 0;
  map[x][y].pRoom = NULL;
}

struct sector_color_type
{
  sector_t bit;
  const char *display_color;
  char display_symbol;
};

const struct sector_color_type sector_color_table[SECT_MAX] = {
  {
   SECT_INSIDE, "{w", 'o'},
  {SECT_CITY, "{W", 'o'},
  {SECT_FIELD, "{G", '*'},
  {SECT_FOREST, "{g", '*'},
  {SECT_HILLS, "{y", '!'},
  {SECT_MOUNTAIN, "{w", '@'},
  {SECT_WATER_SWIM, "{B", '='},
  {SECT_WATER_NOSWIM, "{b", '='},
  {SECT_ICE, "{C", 'O'},
  {SECT_AIR, "{C", '~'},
  {SECT_DESERT, "{y", '+'},
  {SECT_ROAD, "{m", ':'},
  {SECT_PATH, "{M", ':'},
  {SECT_SWAMP, "{G", '&'},
  {SECT_CAVE, "{w", '#'},
  {SECT_NONE, "{w", '?'}
};

const char *
get_sector_color (sector_t sector)
{
  int looper;

  for (looper = 0; looper < SECT_MAX; looper++)
    if (sector_color_table[looper].bit == sector)
      return (sector_color_table[looper].display_color);

  return "";
}

char
get_sector_symbol (sector_t sector)
{
  int looper;

  for (looper = 0; looper < SECT_MAX; looper++)
    if (sector_color_table[looper].bit == sector)
      return (sector_color_table[looper].display_symbol);

  return '?';
}

void
map_exits (CharData * ch, RoomIndex * pRoom, int x, int y)
{
  int door;
  int exitx = 0, exity = 0;
  int roomx = 0, roomy = 0;
  ExitData *pExit;

  if (!can_see_room (ch, pRoom))
    return;

  map[x][y].symbol = get_sector_symbol (pRoom->sector_type);
  map[x][y].depth = depth;
  map[x][y].pRoom = pRoom;

  if (depth >= MAXDEPTH)
    return;

  for (door = DIR_NORTH; door <= DIR_DOWN; door++)
    {
      if ((pExit = pRoom->exit[door]) == NULL)
	continue;

      if (IsSet (pExit->exit_info, EX_CLOSED))
	continue;

      if (pExit->u1.to_room == NULL)
	continue;

      if (!can_see_room (ch, pExit->u1.to_room))
	continue;

      get_exit_dir (door, &exitx, &exity, x, y);
      get_exit_dir (door, &roomx, &roomy, exitx, exity);

      if (BOUNDARY (exitx, exity) || BOUNDARY (roomx, roomy))
	continue;

      if (depth == MAXDEPTH)
	continue;

      map[exitx][exity].depth = depth;
      if (IsSet (pExit->exit_info, EX_CLOSED))
	map[exitx][exity].symbol = map_chars_closed[door];
      else
	map[exitx][exity].symbol = map_chars[door];
      map[exitx][exity].pRoom = pExit->u1.to_room;

      if ((depth < MAXDEPTH)
	  && ((map[roomx][roomy].pRoom == pExit->u1.to_room)
	      || (map[roomx][roomy].pRoom == NULL)))
	{
	  depth++;
	  map_exits (ch, pExit->u1.to_room, roomx, roomy);
	  depth--;
	}
    }
}

char *
reformat_desc (char *desc)
{
  size_t l, m;
  static char buf[MPL];

  l = 0;
  m = 0;
  buf[0] = '\0';

  if (NullStr (desc))
    return "";


  for (m = 0; m <= strlen (desc); m++)
    if (desc[m] == '\n' || desc[m] == '\r')
      desc[m] = '\x20';


  for (m = 0; desc[m] != '\0'; m++)
    {
      if (desc[m] == '\x20' && desc[m + 1] == '\x20')
	{
	  buf[l++] = ' ';
	  do
	    {
	      m++;
	    }
	  while (desc[m] == '\x20');
	}
      buf[l++] = desc[m];
    }
  buf[l] = '\0';

  return buf;
}

size_t
get_line_len (char *desc, size_t max_len)
{
  size_t m, l;
  char buf[MSL];

  if (strlen (desc) <= max_len)
    return 0;

  buf[0] = '\0';
  l = 0;

  for (m = 0; m <= strlen (desc); m++)
    {
      if (desc[m] == COLORCODE)
	{
	  int k = ansi_skip (&desc[m]);

	  m += k;
	  if (k == 1)
	    lcolor = desc[m];
	}
      else if (desc[m] == CUSTOMSTART)
	{
	  do
	    {
	      m++;
	    }
	  while (desc[m] != CUSTOMEND);
	}
      else if (desc[m] == MXP_BEGc)
	{
	  do
	    {
	      m++;
	    }
	  while (desc[m] != MXP_ENDc);
	}
      else
	{
	  if (++l > max_len)
	    break;
	}
    }

  for (l = m; l > 0; l--)
    if (desc[l] == ' ')
      break;

  return l + 1;
}

void
show_map (CharData * ch, char *text, bool fSmall)
{
  char buf[MSL * 2];
  size_t x, y, m, n, pos;
  char *p;
  bool alldesc = false;
  double rcnt = (double) (areacount (ch, ch->in_room->area));
  double rooms = (double) (arearooms (ch->in_room->area));
  int maxlen = get_scr_cols (ch);
  int maplen = maxlen - 15;

  if (fSmall)
    {
      m = 4;
      n = 5;
    }
  else
    {
      m = 0;
      n = 0;
    }
  pos = 0;
  p = text;
  buf[0] = '\0';
  lcolor = 'x';

  if (fSmall)
    {

      if (IsNPC (ch) || IsSet (ch->in_room->room_flags, ROOM_NOEXPLORE))
	sprintf (buf, "{R+------------+{%c ", lcolor);
      else
	sprintf (buf, "{R+-----[{x%3.0f%%{R]+{%c ",
		 Percent (rcnt, rooms), lcolor);

      if (!alldesc)
	{
	  pos = get_line_len (p, maplen);
	  if (pos > 0)
	    {
	      strncat (buf, p, pos);
	      p += pos;
	    }
	  else
	    {
	      strcat (buf, p);
	      alldesc = true;
	    }
	}
      strcat (buf, NEWLINE);
    }

  for (y = m; y <= (MAPY * 2) - m; y++)
    {
      if (fSmall)
	strcat (buf, "{R|");
      else
	strcat (buf, "{D");

      for (x = n; x <= (MAPX * 2) - n; x++)
	{
	  if (map[x][y].pRoom)
	    {
	      if (map[x][y].symbol ==
		  get_sector_symbol (map[x][y].pRoom->sector_type)
		  && !IsNPC (ch)
		  && StrIsSet (ch->pcdata->explored, map[x][y].pRoom->vnum))
		{
		  if (map[x][y].pRoom->exit[DIR_UP]
		      && map[x][y].pRoom->exit[DIR_DOWN])
		    map[x][y].symbol = 'B';
		  else if (!map[x][y].pRoom->exit[DIR_UP]
			   && map[x][y].pRoom->exit[DIR_DOWN])
		    map[x][y].symbol = 'D';
		  else if (map[x][y].pRoom->exit[DIR_UP]
			   && !map[x][y].pRoom->exit[DIR_DOWN])
		    map[x][y].symbol = 'U';
		}
	      if (!fSmall)
		sprintf (buf + strlen (buf), " %s%c{D",
			 get_sector_color (map[x][y].pRoom->sector_type),
			 map[x][y].symbol);
	      else
		sprintf (buf + strlen (buf), "%s%c",
			 get_sector_color (map[x][y].pRoom->sector_type),
			 map[x][y].symbol);
	    }
	  else
	    {
	      if (!fSmall)
		strcat (buf, " {D.");
	      else
		strcat (buf, " ");
	    }
	}
      if (!fSmall)
	{
	  switch (y)
	    {
	    case 0:
	      strcat (buf, "   {xX   You are here");
	      break;
	    case 2:
	      strcat (buf, "   {xo   Normal Rooms");
	      break;
	    case 3:
	      strcat (buf, "   {xU   Room with exit up");
	      break;
	    case 4:
	      strcat (buf, "   {xD   Room with exit down");
	      break;
	    case 5:
	      strcat (buf, "   {xB   Room with exits up & down");
	      break;
	    case 6:
	      strcat (buf, "   {x|-  Exits");
	      break;
	    case 7:
	      strcat (buf, "   {x>I< Closed Doors");
	      break;
	    case 8:
	      strcat (buf, "   {x*   Field/Forest");
	      break;
	    case 9:
	      strcat (buf, "   {x!   Hills");
	      break;
	    case 10:
	      strcat (buf, "   {x@   Mountain");
	      break;
	    case 11:
	      strcat (buf, "   {x=   Water");
	      break;
	    case 12:
	      strcat (buf, "   {x~   Air");
	      break;
	    case 13:
	      strcat (buf, "   {x+   Desert");
	      break;
	    case 14:
	      strcat (buf, "   {x:   Road/Path");
	      break;
	    case 15:
	      strcat (buf, "   {x&   Swamp");
	      break;
	    case 16:
	      strcat (buf, "   {x#   Cave");
	      break;
	    case 17:
	      strcat (buf, "   {x?   Unknown");
	      break;
	    default:
	      strcat (buf, "   {x");
	      break;
	    }
	  strcat (buf, NEWLINE);
	}
      else
	{
	  sprintf (buf + strlen (buf), "{R| {%c", lcolor);

	  if (!alldesc)
	    {
	      pos = get_line_len (p, maplen);
	      if (pos > 0)
		{
		  strncat (buf, p, pos);
		  p += pos;
		}
	      else
		{
		  strcat (buf, p);
		  alldesc = true;
		}
	    }
	  strcat (buf, NEWLINE);
	}
    }

  if (!fSmall)
    chprintlnf (ch, "%s" NEWLINE "%s{x%s", draw_line (ch, NULL, 0), buf,
		draw_line (ch, NULL, 0));
  else
    {
      sprintf (buf + strlen (buf), "{R+-----------+{%c ", lcolor);

      if (!alldesc)
	{
	  pos = get_line_len (p, maplen);
	  if (pos > 0)
	    {
	      strncat (buf, p, pos);
	      p += pos;
	    }
	  else
	    {
	      strcat (buf, p);
	      alldesc = true;
	    }
	}

      if (!alldesc)
	{
	  do
	    {
	      pos = get_line_len (p, maxlen);
	      if (pos > 0)
		{
		  strncat (buf, p, pos);
		  p += pos;
		}
	      else
		{
		  strcat (buf, p);
		  alldesc = true;
		}
	    }
	  while (!alldesc);
	}
      strcat (buf, "{x");
      chprint (ch, buf);
    }
}

void
draw_map (CharData * ch, const char *desc)
{
  int x, y;
  char *buf;
  bool fSmall;

  if (NullStr (desc))
    {
      buf = (char *) desc;
      fSmall = false;
    }
  else
    {
      buf = reformat_desc ((char *) desc);
      fSmall = true;
    }

  for (y = 0; y <= MAPY * 2; y++)
    {
      for (x = 0; x <= MAPX * 2; x++)
	{
	  clear_coord (x, y);
	}
    }

  x = MAPX;
  y = MAPY;

  depth = (fSmall) ? 2 : 0;

  map_exits (ch, ch->in_room, x, y);

  map[x][y].symbol = 'X';
  show_map (ch, buf, fSmall);
}

Do_Fun (do_automap)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOMAP,
	      "You now see an automap in room descriptions.",
	      "You no longer see automap room descriptions.");
}

Do_Fun (do_map)
{
  if (IsNPC (ch))
    return;
  if (!ch->in_room)
    return;
  if (!check_blind (ch))
    return;

  draw_map (ch, NULL);

  return;
}
