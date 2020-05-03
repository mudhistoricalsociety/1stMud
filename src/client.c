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
#include "interp.h"
#include "tables.h"



#define IMAGE_URL FORMATF("http://www.%s/~%s/images/", HOSTNAME, UNAME)

#define SOUND_URL FORMATF("http://www.%s/~%s/msp/", HOSTNAME, UNAME)

void
setup_mxp (Descriptor * d)
{
  if (!IsMXP (d))
    return;

  d_print (d, MXPTAG ("!ELEMENT RName FLAG=\"RoomName\""));
  d_print (d, MXPTAG ("!ELEMENT RDesc FLAG='RoomDesc' OPEN"));
  d_print (d, MXPTAG ("!ELEMENT RExits FLAG='RoomExit' OPEN"));
  d_print (d, MXPTAG ("!ELEMENT Ex '<SEND>;'"));
  d_print (d, MXPTAG ("!ELEMENT Prompt FLAG=\"Prompt\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT Hp FLAG=\"Set hp\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT MaxHp FLAG=\"Set maxhp\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT Mana FLAG=\"Set mana\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT MaxMana FLAG=\"Set maxmana\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT Move FLAG=\"Set move\" OPEN"));
  d_print (d, MXPTAG ("!ELEMENT MaxMove FLAG=\"Set maxmove\" OPEN"));

  d_print (d, MXPTAG
	   ("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
	    "hint='Bid for &desc;' prompt>\" ATT='name desc'"));
  d_print (d, MXPTAG
	   ("!ELEMENT Help \"<send href='help &#39;&name;&#39;' "
	    "hint='Help on &desc;'>\" ATT='name desc'"));

  d_print (d, MXPTAG
	   ("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
	    "hint='Buy &desc;'>\" ATT='name desc'"));

  d_print (d, MXPTAG
	   ("!ELEMENT Tell \"<send href='tell &#39;&name;&#39; ' "
	    "hint='print a message to &name;' prompt>\" ATT='name'"));
  d_print (d, MXPTAG
	   ("!ELEMENT Fwho \"<send href='whois &#39;&name;&#39;|"
	    "buddy &#39;&name;&#39;' "
	    "hint='Right mouse click to use this object|Whois &desc;|"
	    "Buddy &desc;'>\" ATT='name desc'"));
  d_print (d, MXPTAG
	   ("!ELEMENT Olc \"<send href='commands|show|done' "
	    "hint='Show commands|Show|Finish editing'>\""));
  d_print (d, MXPTAG
	   ("!ELEMENT Pager \"<send href='help|refresh|back|continue' "
	    "hint='Help|Refresh page|Go back a page|Continue to next page'>\""));

  return;
}

void
init_mxp (Descriptor * d)
{
  d_print (d, MXPTAG ("VERSION"));
  d_print (d, MXPTAG ("SUPPORT"));
}

Do_Fun (do_mxp)
{
  if (!ch->desc || IsNPC (ch))
    {
      chprintln (ch, "No descriptor.");
      return;
    }

  set_on_off (ch, &ch->desc->desc_flags, DESC_MXP,
	      "Mud eXtension Protocol enabled.",
	      "Mud eXtension Protocol disabled.");
  if (IsSet (ch->desc->desc_flags, DESC_MXP))
    {
      if ((ch->desc->mxp.mxp_ver * 10) < 4)
	init_mxp (ch->desc);
      else
	setup_mxp (ch->desc);
    }
}



bool
convert_color_mxp_tags (Descriptor * d)
{
  bool bInTag = false, bInEntity = false;
  bool bMXP = IsSet (d->desc_flags, DESC_MXP);
  char output[MSL];
  register char *result, *ps;
  bool success = true;
  int written;
  colatt_t c;
  CharData *ch = CH (d);
  char buf[MIL];

  memset (output, 0, MSL);
  result = output;
  c.at[CT_ATTR] = CL_NONE;
  c.at[CT_FORE] = FG_NONE;
  c.at[CT_BACK] = BG_NONE;
  buf[0] = NUL;

  for (ps = d->outbuf; *ps != NUL && (int) (ps - d->outbuf) < d->outtop; ps++)
    {
      if ((int) (result - output) >= MSL - 32)
	{
	  *result++ = '\0';

	  written = d_write (d, output, (int) (result - output));
	  if (!(success = written >= 0))
	    break;

	  memset (output, 0, MSL);
	  result = output;
	}

      if (*ps == COLORCODE && !VT100_SET (ch, SHOW_CODES))
	{
	  ps++;

	  if (isdigit (*ps))
	    {
	      int slot = *ps - '0';



	      if (VALID_CL (slot))
		{
		  c.at[CT_ATTR] = slot + CL_MOD;
		}

	      if (*(ps + 1) == '+' && *(ps + 2))
		{
		  ps += 2;
		}
	      else
		continue;
	    }

	  if (*ps == '=')
	    {
	      ps++;
	      c.at[CT_BACK] = c.at[CT_FORE] + CL_MOD;
	    }

	  if (IsSet (d->desc_flags, DESC_COLOR))
	    {

	      switch (*ps)
		{
		case '}':
		  if (!VT100_SET (ch, NO_NEWLINE))
		    strcpy (buf, NEWLINE);
		  break;
		case '-':
		  strcpy (buf, "~");
		  break;
		case 'n':

		  strcpy (buf, mud_info.name);
		  break;
		case 'N':

		  strcpy (buf, strupper (mud_info.name));
		  break;
		case 't':
		case 'T':

		  if (*(ps + 1) == '+' && *(ps + 2) == '"' && *(ps + 3))
		    {
		      char fmt[800];
		      size_t t = 0;

		      ps += 3;

		      do
			{
			  fmt[t++] = *ps++;
			}
		      while (*ps && *ps != '"' && t < sizeof (fmt));

		      fmt[t] = '\0';

		      strcpy (buf,
			      str_time (current_time,
					ch ? GetTzone (ch) : -1, fmt));
		    }
		  else
		    strcpy (buf,
			    str_time (current_time,
				      ch ? GetTzone (ch) : -1, NULL));
		  break;
		case 'P':
		case 'p':
		  if (!VT100_SET (ch, NO_BEEPS))
		    strcpy (buf, "\007");
		  break;
		case COLORCODE:
		  sprintf (buf, "%c", COLORCODE);
		  break;
		case 'X':
		  c.at[CT_ATTR] = CL_NONE;
		  c.at[CT_FORE] = FG_NONE;
		  c.at[CT_BACK] = BG_NONE;
		  strcpy (buf, CL_DEFAULT);
		  break;
		case 'x':
		  c.at[CT_ATTR] = CL_NONE;
		  c.at[CT_FORE] = FG_NONE;
		  c.at[CT_BACK] = BG_NONE;
		  strcpy (buf, char_color (ch, _DEFAULT));
		  break;
		default:
		  set_col_attr (*ps, &c, ch);
		  strcpy (buf, make_color (ch, &c));
		  break;
		}
	      add_text (buf, result);
	    }
	}
      else if (*ps == CUSTOMSTART)
	{
	  int slot = 0;

	  do
	    {
	      ps++;
	      if (isdigit (*ps))
		slot = (slot * 10) + (*ps - '0');
	    }
	  while (*ps && *ps != CUSTOMEND);

	  if (IsSet (d->desc_flags, DESC_COLOR)
	      && !VT100_SET (ch, SHOW_CODES))
	    {
	      if (slot < 0 || slot >= MAX_CUSTOM_COLOR)
		{
		  bug ("get_color(): invalid custom color");
		  strcpy (buf, CL_DEFAULT);
		}
	      else
		{
		  strcpy (buf, char_color (ch, slot));
		}

	      add_text (buf, result);
	    }
	}
      else if (bInTag)
	{
	  if (*ps == MXP_ENDc)
	    {
	      bInTag = false;
	      if (bMXP)
		*result++ = '>';
	      if (buf[0] != NUL)
		add_text (buf, result);
	    }
	  else if (bMXP)
	    *result++ = *ps;
	}
      else if (bInEntity)
	{
	  if (bMXP)
	    *result++ = *ps;
	  if (*ps == ';')
	    bInEntity = false;
	}
      else
	{
	  switch (*ps)
	    {
	    case MXP_BEGc:
	      bInTag = true;
	      if (bMXP)
		{
		  add_text (MXPMODE (1), result);
		  *result++ = '<';
		}
	      break;

	    case MXP_ENDc:
	      bInTag = false;
	      if (bMXP)
		*result++ = '>';
	      break;

	    case MXP_ENTc:
	      bInEntity = true;
	      if (bMXP)
		*result++ = '&';
	      break;

	    default:
	      if (bMXP && !d->pString && d->connected == CON_PLAYING)
		{
		  switch (*ps)
		    {
		    case HTML_LTc:
		      add_text (HTML_LT, result);
		      break;

		    case HTML_GTc:
		      add_text (HTML_GT, result);
		      break;

		    case HTML_AMPc:
		      add_text (HTML_AMP, result);
		      break;

		    case HTML_QUOTEc:
		      add_text (HTML_QUOTE, result);
		      break;

		    default:
		      *result++ = *ps;
		      break;
		    }
		}
	      else
		*result++ = *ps;
	      break;
	    }
	}
    }

  *result = '\0';

  written = d_write (d, output, (int) (result - output));
  success = (success && (written >= 0));

  d->outtop = 0;
  return success;
}


const char *
mxp_obj (ObjData * obj, CharData * ch, bool fShort)
{
  if (!obj)
    return "!bug!";

  if (!ch || !IsMXP (ch->desc))
    return fShort ? obj->short_descr : obj->description;

  if (obj->in_room)
    {
    }
  else if (obj->in_obj)
    {
    }
  else if (obj->in_room)
    {
    }
  else
    {
    }
  return fShort ? obj->short_descr : obj->description;
}


const char *
mxp_char (CharData * mch, CharData * ch, bool fShort)
{

  if (!mch)
    return "!bug!";

  if (!ch || !IsMXP (ch->desc))
    return fShort ? mch->short_descr : mch->long_descr;

  if (mch->in_room == ch->in_room)
    {
    }
  return fShort ? mch->short_descr : mch->long_descr;
}


const char *
create_tag (CharData * ch, const char *text, const char *hint,
	    const char *command, ...)
{
  va_list args;
  char format[MSL];
  static char buf_new[5][MSL];
  static int i;
  char *result;

  if (!ch || !ch->desc || NullStr (command))
    return text;


  ++i;
  i %= 5;
  result = buf_new[i];

  va_start (args, command);
  vsnprintf (format, sizeof (format), command, args);
  va_end (args);

  if (IsMXP (ch->desc))
    {
      if (!NullStr (hint))
	sprintf (result,
		 MXPTAG ("send href='%s' hint='%s'") "%s"
		 MXPTAG ("/send"), format, hint, text);
      else
	sprintf (result, MXPTAG ("send href='%s'") "%s" MXPTAG ("/send"),
		 format, text);
    }
  else if (IsPueblo (ch->desc))
    {
      if (!NullStr (hint))
	sprintf (result,
		 "</xch_mudtext><img xch_mode=html><a xch_cmd=\"%s\" xch_hint=\"%s\">"
		 "%s</a><br><img xch_mode=text>", format, hint, text);
      else
	sprintf (result,
		 "</xch_mudtext><img xch_mode=html><a xch_cmd=\"%s\">"
		 "%s</a><br><img xch_mode=text>", format, text);
    }
  else
    return text;

  return (result);
}


FlagTable mxp_support_flags[] = {
  {"a", BIT_A, false}
  ,
  {"a.href", BIT_B, false}
  ,
  {"a.xch_cmd", BIT_C, false}
  ,
  {"a.xch_hint", BIT_D, false}
  ,
  {"b", BIT_E, false}
  ,
  {"body", BIT_F, false}
  ,
  {"bold", BIT_G, false}
  ,
  {"br", BIT_H, false}
  ,
  {"c", BIT_I, false}
  ,
  {"c.back", BIT_J, false}
  ,
  {"c.fore", BIT_K, false}
  ,
  {"color", BIT_L, false}
  ,
  {"color.back", BIT_M, false}
  ,
  {"color.fore", BIT_N, false}
  ,
  {"em", BIT_O, false}
  ,
  {"expire", BIT_P, false}
  ,
  {"font", BIT_Q, false}
  ,
  {"font.back", BIT_R, false}
  ,
  {"font.bgcolor", BIT_S, false}
  ,
  {"font.color", BIT_T, false}
  ,
  {"font.fgcolor", BIT_U, false}
  ,
  {"gauge", BIT_V, false}
  ,
  {"h", BIT_W, false}
  ,
  {"head", BIT_X, false}
  ,
  {"high", BIT_Y, false}
  ,
  {"hr", BIT_Z, false}
  ,
  {"html", BIT_a, false}
  ,
  {"i", BIT_b, false}
  ,
  {"image", BIT_c, false}
  ,
  {"image.url", BIT_d, false}
  ,
  {"img", BIT_e, false}
  ,
  {"img.src", BIT_f, false}
  ,
  {"img.xch_mode", BIT_Ax, false}
  ,
  {"italic", BIT_Bx, false}
  ,
  {"li", BIT_Cx, false}
  ,
  {"music", BIT_Dx, false}
  ,
  {"mxp", BIT_Ex, false}
  ,
  {"mxp.off", BIT_Fx, false}
  ,
  {"nobr", BIT_Gx, false}
  ,
  {"ol", BIT_Hx, false}
  ,
  {"option", BIT_Ix, false}
  ,
  {"p", BIT_Jx, false}
  ,
  {"pass", BIT_Kx, false}
  ,
  {"password", BIT_Lx, false}
  ,
  {"pre", BIT_Mx, false}
  ,
  {"relocate", BIT_Nx, false}
  ,
  {"reset", BIT_Ox, false}
  ,
  {"s", BIT_Px, false}
  ,
  {"samp", BIT_Qx, false}
  ,
  {"sbr", BIT_Rx, false}
  ,
  {"send", BIT_Sx, false}
  ,
  {"send.hint", BIT_Tx, false}
  ,
  {"send.href", BIT_Ux, false}
  ,
  {"send.prompt", BIT_Vx, false}
  ,
  {"send.xch_cmd", BIT_Wx, false}
  ,
  {"send.xch_hint", BIT_Xx, false}
  ,
  {"sound", BIT_Yx, false}
  ,
  {"stat", BIT_Zx, false}
  ,
  {NULL, 0, false}
};

FlagTable mxp_support_flags2[] = {
  {"strike", BIT_A, false}
  ,
  {"strong", BIT_B, false}
  ,
  {"support", BIT_C, false}
  ,
  {"title", BIT_D, false}
  ,
  {"u", BIT_E, false}
  ,
  {"ul", BIT_F, false}
  ,
  {"underline", BIT_G, false}
  ,
  {"user", BIT_H, false}
  ,
  {"username", BIT_I, false}
  ,
  {"v", BIT_J, false}
  ,
  {"var", BIT_K, false}
  ,
  {"version", BIT_L, false}
  ,
  {"xch_page", BIT_M, false}
  ,
  {"dd", BIT_N, false}
  ,
  {"dest", BIT_O, false}
  ,
  {"dl", BIT_P, false}
  ,
  {"dt", BIT_Q, false}
  ,
  {"frame", BIT_R, false}
  ,
  {"h1", BIT_S, false}
  ,
  {"h2", BIT_T, false}
  ,
  {"h3", BIT_U, false}
  ,
  {"h4", BIT_V, false}
  ,
  {"h5", BIT_W, false}
  ,
  {"h6", BIT_X, false}
  ,
  {"small", BIT_Y, false}
  ,
  {"tt", BIT_Z, false}
  ,
  {"xch_mudtext", BIT_a, false}
  ,
  {"xch_pane", BIT_b, false}
  ,
  {NULL, 0, false}
};

flag_t
mxp_lookup (const char *name, FlagTable * flag_table)
{
  int flag;

  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (tolower (name[0]) == tolower (flag_table[flag].name[0])
	  && !str_cmp (name, flag_table[flag].name))
	return flag_table[flag].bit;
    }

  return 0;
}


void
mxp_support (Descriptor * d, int i, unsigned char *inbuf)
{
  unsigned char *buf = &inbuf[i];
  const char *supports;
  char arg[MIL];
  const char *argument;
  static char tbuf[MSL];
  int n = 10;
  flag_t bit;

  do
    {
      n++;
    }
  while (buf[n] != '>');
  buf[n] = NUL;

  sprintf (tbuf, "%s", buf + 10);

  buf[n] = '>';
  n++;

  supports = str_dup (tbuf);

  for (;;)
    {
      switch (buf[n])
	{
	default:
	case NUL:
	  break;

	case '\n':
	case '\r':
	  n++;
	  continue;
	  break;
	}
      break;
    }

  telopt_lskip = n + 9;

  if (supports)
    {
      argument = supports;
      tbuf[0] = NUL;

      do
	{
	  argument = one_argument (argument, arg);
	  if (arg[0] != NUL)
	    {
	      if ((bit = mxp_lookup (arg + 1, mxp_support_flags)) > 0)
		{
		  SetBit (d->mxp.flags, bit);
		}
	      else if ((bit = mxp_lookup (arg + 1, mxp_support_flags2)) > 0)
		{
		  SetBit (d->mxp.flags2, bit);
		}
	      else
		sprintf (tbuf + strlen (tbuf), " %s", arg);
	    }
	}
      while (arg[0] != NUL);

      free_string (supports);
      replace_str (&d->mxp.supports, tbuf);

    }
  return;

}


void
mxp_version (Descriptor * d, int i, unsigned char *inbuf)
{
  unsigned char *buf = &inbuf[i];
  char cbuf[MIL];
  char rbuf[MIL];
  char *arg;
  int n = 0;
  int c = 0;

  cbuf[0] = NUL;
  rbuf[0] = NUL;

  do
    {
      buf[n] = toupper (buf[n]);
      n++;
    }
  while (buf[n] != '>');

  c = n;
  buf[c] = NUL;

  arg = (char *) &buf[5];

  do
    {
      do
	{
	  arg++;
	}
      while (*arg != ' ' && *arg != NUL);

      if (*arg == NUL)
	break;

      switch (toupper (*(arg + 1)))
	{
	default:
	  break;
	case 'C':
	  sscanf (arg, " CLIENT=%s", cbuf);
	  break;
	case 'M':
	  sscanf (arg, " MXP=%f", &d->mxp.mxp_ver);
	  break;
	case 'R':
	  sscanf (arg, " REGISTERED=%s", rbuf);
	  break;
	case 'S':
	  sscanf (arg, " STYLE=%f", &d->mxp.style_ver);
	  break;
	case 'V':
	  sscanf (arg, " VERSION=%f", &d->mxp.client_ver);
	  break;
	}
    }
  while (*arg != NUL);

  replace_str (&d->mxp.client, cbuf);

  d->mxp.registered = NullStr (rbuf) ? 0 : (toupper (rbuf[0]) == 'Y') ? 2 : 1;

  n = c + 1;
  buf[c] = '>';

  for (;;)
    {
      switch (buf[n])
	{
	default:
	case NUL:
	  break;

	case '\n':
	case '\r':
	  n++;
	  continue;
	  break;
	}
      break;
    }

  telopt_lskip = n - 1;

  if ((d->mxp.mxp_ver * 10) >= 4)
    {
      setup_mxp (d);
    }
  else
    RemBit (d->desc_flags, DESC_MXP);

}

void
list_mxp_flags (CharData * ch, Descriptor * d)
{
  int flag;
  char *color;
  int max_col, col = 0;

  max_col = get_scr_cols (ch) / 13;

  for (flag = 0; mxp_support_flags[flag].name != NULL; flag++)
    {
      if (IsSet (d->mxp.flags, mxp_support_flags[flag].bit))
	color = "{G";
      else
	color = "{r";

      chprintf (ch, "|%s%s{x", color,
		stringf (ch, 12, Center, NULL, mxp_support_flags[flag].name));

      if (++col % max_col == 0)
	chprintln (ch, "|");
    }

  for (flag = 0; mxp_support_flags2[flag].name != NULL; flag++)
    {
      if (IsSet (d->mxp.flags, mxp_support_flags2[flag].bit))
	color = "{G";
      else
	color = "{r";

      chprintf (ch, "|%s%s{x", color,
		stringf (ch, 12, Center, NULL,
			 mxp_support_flags2[flag].name));

      if (++col % max_col == 0)
	chprintln (ch, "|");
    }

  if (col % max_col != 0)
    chprintln (ch, "|");
}

void
mxp_details (CharData * ch, Descriptor * d)
{
  if (IsSet (d->desc_flags, DESC_MXP))
    {
      chprintlnf (ch, "Status: {G%s{x", IsMXP (d) ? "ON" : "OFF");
      chprintlnf (ch, "Client: {G%s %1.2f %s{x", d->mxp.client,
		  d->mxp.client_ver,
		  d->mxp.registered ==
		  1 ? "{D[{RUnregistered{D]" : d->mxp.registered ==
		  2 ? "{D[{GRegistered{D]" : "");
      chprintlnf (ch, "MXP Version: {Y%1.2f{x", d->mxp.mxp_ver);
      if (d->mxp.style_ver > 0)
	chprintlnf (ch, "Style Version: {M%1.2f{x", d->mxp.style_ver);
      if (!NullStr (d->mxp.supports))
	chprintlnf (ch, "MXP Info: %s", d->mxp.supports);

      chprintln (ch, draw_line (ch, NULL, 0));

      if (d->mxp.flags || d->mxp.flags2)
	list_mxp_flags (ch, d);

      chprintln (ch, draw_line (ch, NULL, 0));
    }
}

void
send_portal (Descriptor * d, const char *format, ...)
{
  char out[MSL];
  char buf[MSL];
  va_list args;
  int len;

  if (!IsPortal (d) || NullStr (format))
    return;

  va_start (args, format);
  len = vsnprintf (buf, sizeof (buf), format, args);
  va_end (args);

  sprintf (out, "#K%%%05u%03d%s", d->portal.keycode, len, buf);

  d_write (d, out, 0);
}

void
portal_sound (CharData * ch, MspData * sound)
{
  send_portal (ch->desc, "%s%s", CL_SEND_SOUND, sound->file);
}

void
portal_music (CharData * ch, MspData * sound)
{
  send_portal (ch->desc, "%s%s~%d", CL_SEND_MUSIC, sound->file, sound->loop);
}

void
portal_image (CharData * ch, const char *img)
{
  send_portal (ch->desc, "%s%s~%s", CL_SEND_IMAGE, img, img);
}

void
portal_map (CharData * ch, RoomIndex * pRoom)
{
  int i;
  char buf[MSL];
  bool found = false;

  if (!IsPortal (ch->desc) || !pRoom)
    return;

  buf[0] = NUL;
  for (i = 0; i < MAX_DIR; i++)
    {
      if (pRoom->exit[i] != NULL)
	{
	  strcat (buf, " ");
	  strcat (buf, &dir_name[i][0]);
	  found = true;
	}
    }
  if (found)
    send_portal (ch->desc, "%s%s", CL_SEND_ROOMCODE, buf + 1);
}

bool
bust_a_portal (CharData * ch)
{
  static char buf[MSL];
  char buf2[MSL];
  Descriptor *d = ch->desc;
  CharData *victim;


  if (!IsPortal (d))
    return false;

  buf[0] = NUL;

  strcat (buf, CL_SEND_COMPOSITE);
  sprintf (buf2, "%s%ld", CL_SEND_HP, ch->hit);
  strcat (buf, buf2);
  sprintf (buf2, "~%s%ld", CL_SEND_MAXHP, ch->max_hit);
  strcat (buf, buf2);
  sprintf (buf2, "~%s%ld", CL_SEND_SP, ch->mana);
  strcat (buf, buf2);
  sprintf (buf2, "~%s%ld", CL_SEND_MAXSP, ch->max_mana);
  strcat (buf, buf2);
  sprintf (buf2, "~%s%ld", CL_SEND_GP1, ch->move);
  strcat (buf, buf2);
  sprintf (buf2, "~%s%ld", CL_SEND_MAXGP1, ch->max_move);
  strcat (buf, buf2);

  victim = ch->fighting;

  if (victim && can_see (ch, victim))
    {
      int percent;

      if (victim->max_hit > 0)
	percent = victim->hit * 100 / victim->max_hit;
      else
	percent = 0;

      sprintf (buf2, "~%s%s", CL_SEND_ATTACKER, victim->short_descr);
      strcat (buf, buf2);
      sprintf (buf2, "~%s%d", CL_SEND_ATTCOND, percent);
      strcat (buf, buf2);
    }
  else
    {
      sprintf (buf2, "~%s0", CL_SEND_ATTCOND);
      strcat (buf, buf2);
    }

  sprintf (buf2,
	   "~%s<r[><sNext Level:> %d<sexp><r][><sPurse:> <y%ld> <sgold> <y%ld> <ssilver><r]>",
	   CL_SEND_GLINE2, (ch->level + 1) * exp_per_level (ch,
							    ch->
							    pcdata->points) -
	   ch->exp, ch->gold, ch->silver);
  strcat (buf, buf2);

  sprintf (buf2, "~%s<r", CL_SEND_GLINE1);

  if (IsSet (ch->comm, COMM_AFK))
    {
      strcat (buf, "[><yAFK><r]");
    }

  strcat (buf, ">");

  strcat (buf, buf2);

  send_portal (d, buf);

  return true;
}

Do_Fun (do_portal)
{
  if (IsNPC (ch) || !ch->desc)
    return;

  set_on_off (ch, &ch->desc->desc_flags, DESC_PORTAL,
	      "You now recieve portal enhancements.",
	      "You no longer recieve portal enhancements.");
}

void
send_imp (Descriptor * d, char *buf)
{
  if (d == NULL)
    return;

  if (!IsFireCl (d))
    return;

  d_write (d, buf, 0);
}

void
imp_sound (CharData * ch, MspData * sound)
{
  char buf[MSL];

  sprintf (buf, "<AUDIO FILE=%s%s>", sound->url, sound->file);
  send_imp (ch->desc, buf);
}

void
imp_image (CharData * ch, const char *img)
{
  char buf[MSL];

  sprintf (buf, "<IMG SRC=%s%s ALT=%s>", IMAGE_URL, img, img);
  send_imp (ch->desc, buf);
}

void
write_sound (FileData * fp, const char *name, MspData * snd)
{
  if (NullStr (name))
    f_printf (fp, "'%s' %s V=%d L=%d P=%d C=%d T=%s~ U=%s~ @" LF,
	      snd->file, write_flags (snd->to), snd->volume, snd->loop,
	      snd->priority, !snd->restart, flag_string (msp_types,
							 snd->type),
	      snd->url);
  else if (snd)
    f_printf (fp, "%s%s'%s' %s V=%d L=%d P=%d C=%d T=%s~ U=%s~ @" LF,
	      name, format_tabs (strlen (name)), snd->file,
	      write_flags (snd->to), snd->volume, snd->loop,
	      snd->priority, !snd->restart, flag_string (msp_types,
							 snd->type),
	      snd->url);
}

MspData *
read_sound (FileData * fp)
{
  MspData *sound = new_msp ();
  char letter;

  sound->file = str_dup (read_word (fp));
  sound->to = read_flag (fp);
  letter = read_letter (fp);
  while (letter != '@')
    {
      read_letter (fp);
      switch (letter)
	{
	case 'V':
	  sound->volume = read_number (fp);
	  break;
	case 'L':
	  sound->loop = read_number (fp);
	  break;
	case 'P':
	  sound->priority = read_number (fp);
	  break;
	case 'C':
	  {
	    bool res = read_number (fp);

	    sound->restart = !res;
	  }
	  break;
	case 'T':
	  {
	    const char *str = read_string (fp);
	    int val = flag_value (msp_types, str);

	    free_string (str);
	    if (val == NO_FLAG)
	      sound->type = MSP_NONE;
	    else
	      sound->type = (msp_t) val;
	  }
	  break;
	case 'U':
	  sound->url = read_string (fp);
	  break;
	default:
	  break;
	}
      letter = read_letter (fp);
    }

  return sound;
}

Do_Fun (do_imp)
{
  if (IsNPC (ch) || !ch->desc)
    return;

  set_on_off (ch, &ch->desc->desc_flags, DESC_IMP,
	      "Interactive Mudding Protocol on.",
	      "Interactive Mudding Protocol off.");
}

Do_Fun (do_pueblo)
{
  if (IsNPC (ch) || !ch->desc)
    return;

  set_on_off (ch, &ch->desc->desc_flags, DESC_PUEBLO,
	      "You now recieve pueblo enhancements.",
	      "You no longer recieve pueblo enhancements.");
}

void
image_to_char (CharData * ch, const char *image)
{
  if (!ch || !image)
    return;

  if (IsPueblo (ch->desc))
    {
      chprint (ch, "</xch_mudtext><img xch_mode=html>");
      chprintf (ch, "<img src=\"%s%s\">", IMAGE_URL, image);
      chprint (ch, "<br><br><img xch_mode=text>");
    }
  else if (IsPortal (ch->desc))
    {
      portal_image (ch, image);
    }
  else if (IsFireCl (ch->desc))
    imp_image (ch, image);
  else if (IsMXP (ch->desc))
    chprintf (ch, MXPTAG ("IMAGE %s %s"), image, IMAGE_URL);
}

Do_Fun (do_msp)
{
  if (IsNPC (ch) || !ch->desc)
    return;

  set_on_off (ch, &ch->desc->desc_flags, DESC_MSP,
	      "Mud Sound Protocol on.", "Mud Sound Protocol off.");
}

void
send_sound (CharData * ch, MspData * snd)
{
  if (!ch || !ch->desc || !snd || NullStr (snd->file))
    return;

  if (IsMXP (ch->desc))
    d_printf (ch->desc,
	      MXPTAG ("SOUND %s V=%d L=%d P=%d C=%d T=%s U=%s"),
	      snd->file, snd->volume, snd->loop, snd->priority,
	      !snd->restart, flag_string (msp_types, snd->type),
	      GetStr (snd->url, SOUND_URL));
  else if (IsMSP (ch->desc))
    d_printf (ch->desc, "!!SOUND(%s V=%d L=%d P=%d C=%d T=%s U=%s)",
	      snd->file, snd->volume, snd->loop, snd->priority,
	      !snd->restart, flag_string (msp_types, snd->type),
	      GetStr (snd->url, SOUND_URL));
  else if (IsPueblo (ch->desc))
    d_printf (ch->desc,
	      "</xch_mudtext><img xch_mode=html>"
	      "<img xch_sound=play xch_volume=%d src=\"%s%s\">"
	      "<br><img xch_mode=text>", GetStr (snd->url, SOUND_URL),
	      snd->file);
  else if (IsPortal (ch->desc))
    portal_sound (ch, snd);
  else if (IsFireCl (ch->desc))
    imp_sound (ch, snd);
}

void
send_music (CharData * ch, MspData * snd)
{
  if (!ch || !ch->desc || !snd || NullStr (snd->file))
    return;

  if (IsMXP (ch->desc))
    d_printf (ch->desc,
	      MXPTAG ("MUSIC %s V=%d L=%d P=%d C=%d T=%s U=%s"),
	      snd->file, snd->volume, snd->loop, snd->priority,
	      !snd->restart, flag_string (msp_types, snd->type),
	      GetStr (snd->url, SOUND_URL));
  else if (IsMSP (ch->desc))
    d_printf (ch->desc, "!!MUSIC(%s V=%d L=%d P=%d C=%d T=%s U=%s)",
	      snd->file, snd->volume, snd->loop, snd->priority,
	      !snd->restart, flag_string (msp_types, snd->type),
	      GetStr (snd->url, SOUND_URL));
  else if (IsPueblo (ch->desc))
    d_printf (ch->desc,
	      "</xch_mudtext><img xch_mode=html>"
	      "<img xch_sound=loop xch_volume=%d src=\"%s%s\">"
	      "<br><img xch_mode=text>", snd->volume, GetStr (snd->url,
							      SOUND_URL));
  else if (IsPortal (ch->desc))
    portal_music (ch, snd);
  else if (IsFireCl (ch->desc))
    imp_sound (ch, snd);
}

void
music_off (CharData * ch)
{
  if (IsMXP (ch->desc))
    chprintf (ch, MXPTAG ("MUSIC Off %s"), SOUND_URL);
  else if (IsMSP (ch->desc))
    chprintf (ch, "!!MUSIC(Off %s)", SOUND_URL);
  else if (IsPueblo (ch->desc))
    chprint (ch,
	     "</xch_mudtext><img xch_mode=html>" "<img xch_sound=stop>"
	     "<br><img xch_mode=text>");
  else if (IsPortal (ch->desc))
    send_portal (ch->desc, "%s", CL_SEND_MUSIC);
  else if (IsFireCl (ch->desc))
    send_imp (ch->desc, "<STOPAUDIO>");
}

#define SENDOK(ch, type)    ((IsNPC(ch) || ((ch)->desc && (ch->desc->connected == CON_PLAYING))) \
            && (ch)->position >= min_pos)

void
act_sound (MspData * sound, CharData * ch, const void *arg1,
	   flag_t type, position_t min_pos)
{
  CharData *to = (CharData *) arg1;

  if (!sound)
    return;

  if (IsSet (type, TO_CHAR))
    {
      if (ch && SENDOK (ch, type))
	send_sound (ch, sound);
    }

  if (IsSet (type, TO_VICT))
    {
      if (to && SENDOK (to, type) && to != ch)
	send_sound (to, sound);
    }

  if (IsSet (type, TO_ZONE | TO_ALL))
    {
      Descriptor *d;

      for (d = descriptor_first; d; d = d->next)
	{
	  to = CH (d);

	  if (to && SENDOK (to, type) && (to != ch)
	      &&
	      ((IsSet
		(type, TO_ALL) || (to->in_room
				   && to->in_room->area ==
				   ch->in_room->area))))
	    send_sound (to, sound);
	}
    }

  if (IsSet (type, TO_ROOM | TO_NOTVICT))
    {
      RoomIndex *room;
      CharData *vch;

      if (ch && ch->in_room != NULL)
	room = ch->in_room;
      else if (to && to->in_room != NULL)
	room = to->in_room;
      else
	{
	  bugf ("no valid target");
	  return;
	}
      for (vch = room->person_first; vch; vch = vch->next_in_room)
	{
	  if (SENDOK (vch, type) && (vch != ch)
	      && (IsSet (type, TO_ROOM) || (to != vch)))
	    send_sound (vch, sound);
	}
    }
  return;
}


int
print_stripped_client_code (Descriptor * to, const char *txt, int size)
{
  int i, cnt = 0;
  int tmp = 0;
  char *dest;

  alloc_mem (dest, char, size);

  for (i = 0; i < size; i++)
    {
      if (txt[i] == ESCc && sscanf (&txt[i], ESC "[%dz", &tmp) == 1)
	{
	  do
	    {
	      i += 2;
	      do
		{
		  i++;
		}
	      while (isdigit (txt[i]));
	      i++;
	    }
	  while (txt[i] == ESCc && sscanf (&txt[i], ESC "[%dz", &tmp) == 1);

	  if (txt[i] == '<')
	    {
	      do
		{
		  i++;
		}
	      while (txt[i] != '>');
	    }
	}
      else if (txt[i] == MXP_BEGc)
	{
	  do
	    {
	      i++;
	    }
	  while (txt[i] != MXP_ENDc);
	}
      else if (txt[i] == '!' && (!memcmp (&txt[i], "!!SOUND(", 8)
				 || !memcmp (&txt[i], "!!MUSIC(", 8)))
	{
	  i += 8;

	  do
	    {
	      i++;
	    }
	  while (txt[i] != ')');
	}
      else if (txt[i] == '<' && !memcmp (&txt[i], "</xch_mudtext>", 14))
	{
	  i += 14;

	  do
	    {
	      i++;
	    }
	  while (txt[i] != '<'
		 || memcmp (&txt[i], "<img xch_mode=text>", 19));

	  i += 18;
	}
      else
	dest[cnt++] = txt[i];
    }

  dest[cnt] = NUL;

  d_print (to, dest);
  free_mem (dest);
  return cnt;
}


Do_Fun (do_client_list)
{
  chprintlnf (ch, "Type   Command  Status");
  chprintln (ch, "----------------------");
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "MXP", cmd_name (do_mxp),
	      OnOff (IsMXP (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "MSP", cmd_name (do_msp),
	      OnOff (IsMSP (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "PUEBLO",
	      cmd_name (do_pueblo), OnOff (IsPueblo (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "IMP", cmd_name (do_imp),
	      OnOff (IsFireCl (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "MCCP",
	      cmd_name (do_compress), OnOff (IsCompressed (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "PORTAL",
	      cmd_name (do_portal), OnOff (IsPortal (ch->desc)));
  chprintlnf (ch, "{W%-6s {g%-8s %s{x", "COLOR",
	      cmd_name (do_color),
	      OnOff (IsSet (ch->desc->desc_flags, DESC_COLOR)));
  chprintln (ch, "------------------------");
  chprintlnf (ch,
	      "Type '%s mxp|msp|pueblo|imp|portal|mccp|color' to change.",
	      n_fun);
}

Do_Fun (do_client)
{
  vinterpret (ch, n_fun, argument, "mxp", do_mxp, "msp", do_msp, "pueblo",
	      do_pueblo, "imp", do_imp, "portal", do_portal, "mccp",
	      do_compress, "color", do_color, NULL, do_client_list);
}
