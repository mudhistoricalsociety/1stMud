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
#include "recycle.h"

bool
is_ansi_printed_char (char c)
{
  switch (c)
    {
    case ' ':
    case '-':
    case COLORCODE:
      return true;
    default:
      return false;
    }
}


int
ansi_skip (const char *pstr)
{
  int i = 0;
  const char *str = pstr;

  if (*str != COLORCODE)
    return i;

  str++;
  i++;

  if (isdigit (*str))
    {
      str++;
      i++;
      if (*str == '+')
	{
	  str++;
	  i++;
	}
      else
	return i;
    }

  if (*str == '=')
    {
      str++;
      i++;
    }

  switch (*str)
    {
    case 'T':
    case 't':
      str += 3;
      i += 3;
      do
	{
	  str++;
	  i++;
	}
      while (*str && *str != '"');
      break;
    default:
      break;
    }

  return i;
}

int
random_color (int att)
{
  switch (att)
    {
    case CT_ATTR:
      return number_range (CL_NONE, CL_BRIGHT);
    case CT_FORE:
      return number_range (FG_RED, FG_WHITE);
    case CT_BACK:
      return number_range (BG_RED, BG_WHITE);
    default:
      return CL_NONE;
    }
}

void
convert_random (colatt_t * attr)
{
  int i;

  for (i = 0; i < CT_MAX; i++)
    if (attr->at[i] == CL_RANDOM)
      attr->at[i] = random_color (i);



  if (attr->at[CT_BACK] >= CL_MOD)
    {
      int bg = BG_NONE;

      if (VALID_FG (attr->at[CT_FORE]))
	bg = attr->at[CT_FORE] + 10;

      attr->at[CT_FORE] = attr->at[CT_BACK] - CL_MOD;
      attr->at[CT_BACK] = bg;
    }

}


char *
make_color (CharData * ch, colatt_t * c)
{
  int len = 0;
  char code[100];

  convert_random (c);

  if (ch)
    {
      if (c->at[CT_ATTR] == CL_BLINK)
	{
	  if (VT100_SET (ch, NO_BLINKING))
	    c->at[CT_ATTR] = CL_NONE;
	}
      if (c->at[CT_FORE] == FG_BLACK)
	{
	  if (VT100_SET (ch, DARK_MOD))
	    c->at[CT_ATTR] = CL_BRIGHT;
	}
      if (c->at[CT_ATTR] == CL_BRIGHT)
	{
	  if (VT100_SET (ch, DARK_COLORS))
	    c->at[CT_ATTR] = CL_NONE;
	}
    }

  if (!VT100_SET (ch, BROKEN_ANSI))
    {
      if (VALID_CL (c->at[CT_ATTR]))
	len = sprintf (code + len, ";%d", c->at[CT_ATTR]);
      if (VALID_FG (c->at[CT_FORE]))
	len = sprintf (code + len, ";%d", c->at[CT_FORE]);
      if (VALID_BG (c->at[CT_BACK]))
	len = sprintf (code + len, ";%d", c->at[CT_BACK]);

      if (len > 0)
	return FORMATF (ESC "[%sm", code + 1);
      else
	{
	  bug ("make_color(): invalid color value(s)");
	  return "";
	}
    }
  else
    {
      if (!VALID_CL (c->at[CT_ATTR]))
	c->at[CT_ATTR] = CL_NONE;
      if (!VALID_FG (c->at[CT_FORE]))
	c->at[CT_FORE] = FG_WHITE;
      if (!VALID_BG (c->at[CT_BACK]))
	c->at[CT_BACK] = BG_BLACK;
      return FORMATF (ESC "[%d;%d;%dm", c->at[CT_ATTR], c->at[CT_FORE],
		      c->at[CT_BACK]);
    }
}


char *
char_color (CharData * ch, int slot)
{
  colatt_t attr;

  if (!ch || !ch->desc || IsNPC (ch) || slot < 0 || slot >= MAX_CUSTOM_COLOR)
    return CL_DEFAULT;

  attr = ch->pcdata->color[slot];
  return make_color (ch, &attr);
}


void
set_col_attr (char c, colatt_t * d, CharData * ch)
{
  int z;
  int At = NO_FLAG;

  if (d->at[CT_ATTR] >= CL_MOD)
    At = d->at[CT_ATTR];

  switch (c)
    {
    case '?':
    case '`':
      d->at[CT_ATTR] = CL_RANDOM;
      d->at[CT_FORE] = FG_RANDOM;
      break;
    case 'z':
      d->at[CT_ATTR] = CL_RANDOM;
      d->at[CT_BACK] = BG_RANDOM;
      break;
    case 'Z':
      for (z = 0; z < CT_MAX; z++)
	d->at[z] = CL_RANDOM;
      break;
    case 'b':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_BLUE;
      break;
    case 'c':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_CYAN;
      break;
    case 'g':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_GREEN;
      break;
    case 'm':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_MAGENTA;
      break;
    case 'd':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_BLACK;
      break;
    case 'r':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_RED;
      break;
    case 'y':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_YELLOW;
      break;
    case 'w':
      d->at[CT_ATTR] = CL_NONE;
      d->at[CT_FORE] = FG_WHITE;
      break;
    case 'B':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_BLUE;
      break;
    case 'C':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_CYAN;
      break;
    case 'G':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_GREEN;
      break;
    case 'M':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_MAGENTA;
      break;
    case 'D':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_BLACK;
      break;
    case 'R':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_RED;
      break;
    case 'W':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_WHITE;
      break;
    case 'Y':
      d->at[CT_ATTR] = CL_BRIGHT;
      d->at[CT_FORE] = FG_YELLOW;
      break;
    default:
      break;
    }
  if (At != NO_FLAG)
    d->at[CT_ATTR] = At - CL_MOD;
}


char *
colorize (const char *str)
{
  static int c;
  static char out[4][MSL];
  char *result;
  size_t a, b = 0;

  if (NullStr (str))
    return "{?";

  ++c, c %= 4;
  result = out[c];

  for (a = 0; str[a] != NUL; a++)
    {
      if (str[a] == COLORCODE)
	{
	  a += ansi_skip (&str[a]);
	  continue;
	}
      else if (str[a] == CUSTOMSTART)
	{
	  do
	    {
	      a++;
	    }
	  while (str[a] != CUSTOMEND);
	  a++;
	  continue;
	}
      else if (str[a] == MXP_BEGc)
	{
	  do
	    {
	      a++;
	    }
	  while (str[a] != MXP_ENDc);
	  a++;
	  continue;
	}
      result[b++] = COLORCODE;
      result[b++] = '?';
      result[b++] = str[a];
    }
  result[b++] = COLORCODE;
  result[b++] = 'x';
  result[b++] = NUL;
  return (result);
}


Do_Fun (do_color)
{
  if (!ch)
    return;

  if (IsNPC (ch) || ch->desc == NULL)
    {
      chprintln (ch, "Color is not ON, Way Moron!");
      return;
    }
  if (!IsSet (ch->desc->desc_flags, DESC_COLOR))
    {
      SetBit (ch->desc->desc_flags, DESC_COLOR);
      RemBit (ch->comm, COMM_NOCOLOR);
      chprintln (ch, colorize ("Color is now ON!"));
    }
  else
    {
      chprintln (ch, casemix ("Color is now OFF, <sigh>"));
      RemBit (ch->desc->desc_flags, DESC_COLOR);
      SetBit (ch->comm, COMM_NOCOLOR);
    }
  return;
}

void
goto_xy (CharData * ch, int col, int row)
{
  chprintf (ch, ESC "[%u;%uH", row, col);
}

void
clear_window (CharData * ch)
{
  chprint (ch, ESC "[r");
}

void
clear_screen (CharData * ch)
{
  chprint (ch, ESC "[2J");
}


void
default_color (CharData * ch, int slot)
{
  int i = 0;

  if (!ch || IsNPC (ch))
    return;

  if (slot == -1)
    {
      for (i = 0; i < MAX_CUSTOM_COLOR; i++)
	{
	  copy_array (ch->pcdata->color[i].at, color_table[i].col_attr,
		      CT_MAX);
	}
    }
  else
    {
      for (i = 0; i < MAX_CUSTOM_COLOR; i++)
	{
	  if (color_table[i].slot == slot)
	    {
	      copy_array (ch->pcdata->color[slot].at,
			  color_table[i].col_attr, CT_MAX);
	      break;
	    }
	}
    }

  return;
}


Lookup_Fun (color_lookup)
{
  int i;

  if (NullStr (name))
    return -1;

  for (i = 0; i < MAX_CUSTOM_COLOR; i++)
    if (!str_prefix (name, color_table[i].name))
      return i;

  return -1;
}

Do_Fun (do_colorset)
{
  char arg[MIL], attr[MIL], fore[MIL], back[MIL];
  int i = 0, slot = 0;
  colatt_t c_attr;
  int pos = 0;

  if (!ch || IsNPC (ch) || !ch->desc)
    return;

  if (!IsSet (ch->desc->desc_flags, DESC_COLOR))
    {
      chprintln (ch, "You must have color on to use colorset.");
      return;
    }

  argument = one_argument (argument, arg);
  argument = one_argument (argument, attr);
  argument = one_argument (argument, fore);
  argument = one_argument (argument, back);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "colors           - lists possible colors",
		  "status           - lists options to colorize",
		  "<option> <color> - sets an option to a color",
		  "default <option> - sets option to default value",
		  "default all      - reset all options to default values",
		  "vt100 <flag>    	    	- toggle a display flag",
		  NULL);
      return;
    }
  else if (!str_prefix (arg, "vt100"))
    {
      flag_t vf;

      if (NullStr (attr) || (vf = flag_value (vt100_flags, attr)) == NO_FLAG)
	{
	  chprintln (ch, "Invalid vt100 flag.");
	  print_all_on_off (ch, vt100_flags, ch->pcdata->vt100);
	  return;
	}

      ToggleBit (ch->pcdata->vt100, vf);
      chprintlnf (ch, "%s %s.", capitalize (flag_string (vt100_flags, vf)),
		  IsSet (ch->pcdata->vt100, vf) ? "ON" : "OFF");
      return;
    }
  else if (!str_prefix (arg, "colors") || !str_prefix (arg, "colors"))
    {
      int j = 0, k = 0;

      chprintln (ch, "Attributes       Foregrounds      Backgrounds");
      chprintln (ch, draw_line (ch, NULL, 58));
      for (i = 0; color_attributes[i].name != NULL; i++)
	{
	  chprintf (ch, "%-15s  ", Upper (color_attributes[i].name));
	  if (color_foregrounds[j].name != NULL)
	    chprintf (ch, "%-15s  ", Upper (color_foregrounds[j++].name));
	  else
	    chprintf (ch, "%-15s  ", " ");
	  if (color_backgrounds[k].name != NULL)
	    chprintlnf (ch, "%s", Upper (color_backgrounds[k++].name));
	  else
	    chprintln (ch, NULL);
	}
      while (color_foregrounds[j].name != NULL)
	{
	  chprintf (ch, "%-15s  %-15s  ", " ",
		    Upper (color_foregrounds[j++].name));
	  if (color_backgrounds[k].name != NULL)
	    chprintlnf (ch, "%s", Upper (color_backgrounds[k++].name));
	  else
	    chprintln (ch, NULL);
	}
      while (color_backgrounds[k].name != NULL)
	chprintlnf (ch, "%-15s  %-15s  %s", " ", " ",
		    Upper (color_backgrounds[k++].name));
      chprintln (ch, draw_line (ch, NULL, 58));
    }
  else if (!str_prefix (arg, "status"))
    {
      Column *Cd = new_column ();

      set_cols (Cd, ch, 2, COLS_CHAR, ch);
      chprintln (ch,
		 "Options that can currently be configured for color are:");
      chprintln (ch, draw_line (ch, NULL, 0));

      for (i = 0; i < MAX_CUSTOM_COLOR; i++)
	{
	  print_cols (Cd, "%-12s - %sLooks like this..{x ",
		      color_table[i].name, char_color (ch,
						       color_table[i].slot));
	}
      cols_nl (Cd);

      chprintln (ch, draw_line (ch, NULL, 0));

      free_column (Cd);
    }
  else if (!str_prefix (arg, "default"))
    {
      if (!str_cmp (attr, "all"))
	{
	  slot = -1;
	  chprintln (ch, "All colors set to default values.");
	}
      else if ((pos = color_lookup (attr)) == -1)
	{
	  do_colorset (n_fun, ch, "");
	  return;
	}
      else
	{
	  slot = color_table[pos].slot;
	  chprintlnf (ch, "%s set to default value.", color_table[pos].name);
	}
      default_color (ch, slot);
      return;
    }
  else if ((pos = color_lookup (arg)) != -1)
    {
      slot = color_table[pos].slot;

      if (NullStr (attr) || NullStr (fore) || NullStr (back))
	{
	  cmd_syntax
	    (ch, NULL, n_fun,
	     "<option> <attribute> <foreground> <background>", NULL);
	  return;
	}

      if ((c_attr.at[CT_ATTR] = flag_value (color_attributes, attr)) ==
	  NO_FLAG)
	{
	  chprintln (ch, "Invalid color Attribute.");
	  return;
	}

      if ((c_attr.at[CT_FORE] =
	   flag_value (color_foregrounds, fore)) == NO_FLAG)
	{
	  chprintln (ch, "Invalid Foreground color.");
	  return;
	}

      if ((c_attr.at[CT_BACK] =
	   flag_value (color_backgrounds, back)) == NO_FLAG)
	{
	  chprintln (ch, "Invalid background color.");
	  return;
	}

      copy_array (ch->pcdata->color[slot].at, c_attr.at, CT_MAX);
      chprintf (ch, "%s set to %s%s",
		color_table[pos].name, char_color (ch, slot),
		flag_string (color_attributes, c_attr.at[CT_ATTR]));
      if (VALID_FG (c_attr.at[CT_FORE]))
	chprintf (ch, " %s",
		  flag_string (color_foregrounds, c_attr.at[CT_FORE]));
      if (VALID_BG (c_attr.at[CT_BACK]))
	chprintf (ch, ", with a %s background",
		  flag_string (color_backgrounds, c_attr.at[CT_BACK]));
      chprintln (ch, "{x.");
      return;
    }
  else
    do_colorset (n_fun, ch, "");
}
