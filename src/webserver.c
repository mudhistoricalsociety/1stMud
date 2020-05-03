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
* ROM 2.4 Integrated Web Server - Version 1.0.   This is my first major   *
* snippet... Please be kind. ;-)  Copyright 1998 -- Defiant -- Rob        *
* Siemborski -- mud@towers.crusoe.net   Many thanks to Russ and the rest  *
* of the developers of ROM for creating such an excellent codebase to     *
* program on.   If you use this code on your mud, I simply ask that you   *
* place my name someplace in the credits.  You can put it where you feel  *
* it is appropriate.  I offer no guarantee that this will work on any mud *
* except my own, and if you can't get it to work, please don't bother me. *
* I wrote and tested this only on a Linux 2.0.30 system.  Comments about  *
* bugs, are, however, appreciated.                                        *
***************************************************************************
* Insanity v0.9a pre-release Modifications                                *
* By Chris Fewtrell (Trax) <C.J.Fewtrell@bcs.org.uk>                      *
*                                                                         *
* - Added functionailiy for Secure Web server pages, using standard HTTP  *
*   Basic authentication, comparing with pass list generated with command *
*   from within the Mud itself.                                           *
* - Seperated out the HTTP codes and content type to seperate functions   *
*   (intending to allow more than HTML to be served via this)             *
* - Adjusted the descriptor handling to prevent anyone from prematurely   *
*   stopping a transfer causing a fd exception and the system to exit()   *
***************************************************************************
* Additional Modifications based upon with with Insanity Codebase         *
* By Chris Fewtrell (Trax) <C.J.Fewtrell@bcs.org.uk>                      *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "interp.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"
#include "magic.h"
#include "telnet.h"
#include "data_table.h"
#include "vnums.h"

DataTable wpwd_data_table[] = {
  {"name", FIELD_STRING, (void *) &wpwd_zero.name, NULL, NULL},
  {"pwd", FIELD_STRING, (void *) &wpwd_zero.passw, NULL, NULL},
  {"level", FIELD_INT, (void *) &wpwd_zero.level, NULL, NULL},
  {NULL, (field_t) - 1, NULL, NULL, NULL}
};

TableSave_Fun (rw_wpwd_data)
{
  rw_list (type, WPWD_FILE, WebpassData, wpwd);
}

void
update_webpasses (CharData * ch, bool pDelete)
{
  WebpassData *c_next;
  WebpassData *curr;

  if (IsNPC (ch))
    return;

  for (curr = wpwd_first; curr != NULL; curr = c_next)
    {
      c_next = curr->next;

      if (!str_cmp (ch->name, curr->name))
	{
	  UnLink (curr, wpwd, next, prev);
	  free_wpwd (curr);
	}
    }
  if (pDelete || NullStr (ch->pcdata->webpass))
    {
      rw_wpwd_data (act_write);
      return;
    }

  curr = new_wpwd ();
  replace_str (&curr->name, ch->name);
  replace_str (&curr->passw, ch->pcdata->webpass);
  curr->level = get_trust (ch);
  Link (curr, wpwd, next, prev);
  rw_wpwd_data (act_write);
  return;
}

Do_Fun (do_webpass)
{
  char arg1[MIL];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;

  if (!ch || IsNPC (ch))
    return;

  if (!IsImmortal (ch))
    {
      chprintln (ch, "This feature is only available to immortals, sorry.");
      return;
    }

  pArg = arg1;
  while (isspace (*argument))
    argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;

  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *pArg++ = *argument++;
    }
  *pArg = '\0';

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "<new>", "show", NULL);
      return;
    }

  d_write (ch->desc, echo_off_str, 0);

  if (!str_cmp (arg1, "show"))
    {
      chprintlnf (ch, "Your password for the website is: %s",
		  GetStr (ch->pcdata->webpass, "Not Set."));
      return;
    }

  if (strlen (arg1) < 5)
    {
      chprintln (ch, "New password must be at least five characters long.");
      d_write (ch->desc, echo_on_str, 0);
      return;
    }

  pwdnew = crypt (arg1, ch->name);
  for (p = pwdnew; *p != '\0'; p++)
    {
      if (*p == '~')
	{
	  chprintln (ch, "New password not acceptable, try again.");
	  d_write (ch->desc, echo_on_str, 0);
	  return;
	}
    }

  replace_str (&ch->pcdata->webpass, pwdnew);
  save_char_obj (ch);
  update_webpasses (ch, false);
  chprintln (ch, "Ok.");
  d_write (ch->desc, echo_on_str, 0);
  return;
}

WebpassData *
check_web_pass (const char *username, const char *password, int level)
{
  WebpassData *current;

  for (current = wpwd_first; current; current = current->next)
    if (current->level >= level && !str_casecmp (current->name, username))
      if (!str_casecmp (current->passw, crypt (password, username)))
	return current;

  return NULL;
}

#ifndef DISABLE_WEBSRV

Proto (bool bind_ip, (SOCKET, int));
Proto (void handle_web_request, (WebDescriptor *));
Proto (int w_print, (WebDescriptor *, const char *));
ProtoF (int w_printf, (WebDescriptor *, const char *, ...), 2, 3);
ProtoF (int w_printlnf, (WebDescriptor *, const char *, ...), 2, 3);
Proto (int w_println, (WebDescriptor *, const char *));
Proto (void handle_web_request, (WebDescriptor *));


const int pr2six[256] = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,
  1,
  2,
  3, 4, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64,
  64,
  64, 64, 64, 26, 27,
  28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
  46,
  47, 48, 49, 50, 51,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64,
  64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};


void
Base64Decode (char *bufcoded, unsigned char *bufplain, int outbufsize)
{
  int nbytesdecoded;
  int nprbytes;
  char *bufin = bufcoded;
  unsigned char *bufout = bufplain;


  while (*bufcoded == ' ' || *bufcoded == '\t')
    ++bufcoded;


  bufin = bufcoded;
  while (pr2six[(int) *(bufin++)] <= 63)
    ;
  nprbytes = bufin - bufcoded - 1;
  nbytesdecoded = ((nprbytes + 3) / 4) * 3;
  if (nbytesdecoded > outbufsize)
    nprbytes = (outbufsize * 4) / 3;

  bufin = bufcoded;
  while (nprbytes > 0)
    {
      *(bufout++) =
	(unsigned char) (pr2six[(int) *bufin] << 2 |
			 pr2six[(int) bufin[1]] >> 4);
      *(bufout++) =
	(unsigned char) (pr2six[(int) bufin[1]] << 4 |
			 pr2six[(int) bufin[2]] >> 2);
      *(bufout++) =
	(unsigned char) (pr2six[(int) bufin[2]] << 6 |
			 pr2six[(int) bufin[3]]);
      bufin += 4;
      nprbytes -= 4;
    }

  if (nprbytes & 03)
    {
      if (pr2six[(int) bufin[-2]] > 63)
	nbytesdecoded -= 2;
      else
	nbytesdecoded -= 1;
    }
  bufplain[nbytesdecoded] = '\0';
}


struct request_type
{
  char *req;
  char *name;
  Web_F *fun;
  bool secure;
  bool dir;
};

EXTERN const struct request_type request_table[];


#define CONTENT_HTML    1
#define CONTENT_TEXT    2
#define CONTENT_GIF     3
#define CONTENT_JPEG    4
#define CONTENT_GZIP    5
#define CONTENT_WAV     6
#define CONTENT_VRML    7
#define CONTENT_CLASS   8

struct type_data
{
  char *suffix;
  int type;
};

const struct type_data status_types[] = {
  {"Continue", 100},
  {"Switching Protocols", 101},
  {"OK", 200},
  {"Created", 201},
  {"Accepted", 202},
  {"Non-Authoritative Information", 203},
  {"No Content", 204},
  {"Reset Content", 205},
  {"Partial Content", 206},
  {"Multiple Choices", 300},
  {"Moved Permanently", 301},
  {"Found", 302},
  {"See Other", 303},
  {"Not Modified", 304},
  {"Use Proxy", 305},
  {"Temporary Redirect", 307},
  {"Bad Request", 400},
  {"Unauthorized", 401},
  {"Payment Required", 402},
  {"Forbidden", 403},
  {"Not Found", 404},
  {"Method Not Allowed", 405},
  {"Not Acceptable", 406},
  {"Proxy Authentication Required", 407},
  {"Request Time-out", 408},
  {"Conflict", 409},
  {"Gone", 410},
  {"Length Required", 411},
  {"Precondition Failed", 412},
  {"Request Entity Too Large", 413},
  {"Request-URI Too Large", 414},
  {"Unsupported Media Type", 415},
  {"Requested range not satisfiable", 416},
  {"Expectation Failed", 417},
  {"Internal Server Error", 500},
  {"Not Implemented", 501},
  {"Bad Gateway", 502},
  {"Service Unavailable", 503},
  {"Gateway Time-out", 504},
  {"HTTP Version not supported", 505},
  {NULL, 0}
};
const struct type_data content_types[] = {
  {
   ".html", CONTENT_HTML}
  ,
  {
   ".htm", CONTENT_HTML}
  ,
  {
   ".gif", CONTENT_GIF}
  ,
  {
   ".txt", CONTENT_TEXT}
  ,
  {
   ".text", CONTENT_TEXT}
  ,
  {
   ".jpg", CONTENT_JPEG}
  ,
  {
   ".jpeg", CONTENT_JPEG}
  ,
  {
   ".gz", CONTENT_GZIP}
  ,
  {
   ".gzip", CONTENT_GZIP}
  ,
  {
   ".wav", CONTENT_WAV}
  ,
  {
   ".wrl", CONTENT_VRML}
  ,
  {
   ".class", CONTENT_CLASS}
  ,
  {
   NULL, CONTENT_TEXT}
};


int
determine_type (char *path)
{
  int i;

  for (i = 0; content_types[i].suffix != NULL; i++)
    {
      if (!str_suffix (content_types[i].suffix, path))
	return content_types[i].type;
    }


  return CONTENT_TEXT;
}

int
determine_status (int status)
{
  int i;

  for (i = 0; status_types[i].type > 0; i++)
    {
      if (status_types[i].type == status)
	return i;
    }
  return -1;
}

int http_error_code = 200;

void
http_status (WebDescriptor * wdesc, int status)
{
  int i = determine_status (status);

  if (i != -1)
    {
      w_printlnf (wdesc, "HTTP/1.1 %d %s", status_types[i].type,
		  status_types[i].suffix);
      if (status_types[i].type == 401)
	w_println (wdesc,
		   "WWW-Authenticate: Basic realm='User/Password are Case Sensitive'");
      http_error_code = 200;
    }
}

void
send_content (WebDescriptor * wdesc, int type)
{
  switch (type)
    {
    case CONTENT_HTML:
      w_println (wdesc, "Content-type: text/html\n\n");
      break;
    default:
    case CONTENT_TEXT:
      w_println (wdesc, "Content-type: text/plain\n\n");
      break;
    case CONTENT_GIF:
      w_println (wdesc, "Content-type: image/gif\n\n");
      break;
    case CONTENT_WAV:
      w_println (wdesc, "Content-type: audio/x-wav\n\n");
      break;
    case CONTENT_GZIP:
      w_println (wdesc, "Content-type: application/x-zip-compressed\n\n");
      break;
    case CONTENT_VRML:
      w_println (wdesc, "Content-type: x-world/x-vrml\n\n");
      break;
    case CONTENT_CLASS:
      w_println (wdesc, "Content-type: application/octet-stream\n\n");
      break;
    }
}


int
w_print (WebDescriptor * wdesc, const char *txt)
{
  if (!wdesc || NullStr (txt))
    return 0;

  return write_to_socket (wdesc->fd, txt, strlen (txt));
}

int
w_printf (WebDescriptor * wdesc, const char *fmt, ...)
{
  char buf[MPL];
  va_list args;
  int len;

  if (!wdesc || NullStr (fmt))
    return 0;

  va_start (args, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return write_to_socket (wdesc->fd, buf, len);
}

int
w_println (WebDescriptor * wdesc, const char *buf)
{
  if (!wdesc)
    return 0;

  return (w_print (wdesc, buf) + write_to_socket (wdesc->fd, LF, 1));
}

int
w_printlnf (WebDescriptor * wdesc, const char *fmt, ...)
{
  char buf[MPL];

  if (!wdesc)
    return 0;

  if (!NullStr (fmt))
    {
      va_list args;

      va_start (args, fmt);
      vsnprintf (buf, sizeof (buf), fmt, args);
      va_end (args);
    }
  else
    buf[0] = NUL;

  return w_println (wdesc, buf);
}

char html_colorbuf[MSL * 10];

bool lastcolor = false;

const struct htmlcolor_type
{
  int at;
  char *norm;
  char *bryte;
}
htmlcolor_table[] =
{
  {
  0, "black", "gray"}
  ,
  {
  1, "maroon", "red"}
  ,
  {
  2, "green", "lime"}
  ,
  {
  3, "orange", "yellow"}
  ,
  {
  4, "navy", "blue"}
  ,
  {
  5, "magenta", "purple"}
  ,
  {
  6, "teal", "cyan"}
  ,
  {
  7, "silver", "white"}
  ,
  {
  -1, NULL, NULL}
};

char *
htmlcolor_lookup (int at, bool bryte)
{
  int i;

  for (i = 0; htmlcolor_table[i].at != -1; i++)
    if (htmlcolor_table[i].at == at)
      return bryte ? htmlcolor_table[i].bryte : htmlcolor_table[i].norm;
  return NULL;
}

char *
colortohtml (colatt_t * a)
{
  bool b, script = false;
  char buf[MSL];
  static char out[MSL];
  char *col;

  convert_random (a);

  b = (a->at[CT_ATTR] == CL_BRIGHT);

  if (VALID_BG (a->at[CT_BACK]))
    {
      if ((col = htmlcolor_lookup (a->at[CT_BACK] - 40, b)) == NULL)
	{
	  script = true;
	  col = "' + document.bgColor + '";
	}
      sprintf (buf, " background-color: %s;", col);

    }

  if (VALID_FG (a->at[CT_FORE]))
    {
      if ((col = htmlcolor_lookup (a->at[CT_FORE] - 30, b)) == NULL)
	{
	  script = true;
	  col = "' + document.fgColor + '";
	}

      sprintf (buf, " color: %s;", col);
    }

  if (buf[0] != '\0')
    {
      if (script)
	sprintf (out,
		 "%s<script type='text/javascript'>document.write('<span style=\"%s\">');</script>",
		 lastcolor ? "</span>" : "", buf + 1);
      else
	sprintf (out, "%s<span style=\"%s\">",
		 lastcolor ? "</span>" : "", buf + 1);
      lastcolor = true;
    }
  else
    out[0] = '\0';

  return out;
}

int
color_slot_lookup (int slot)
{
  int i;

  for (i = 0; i < MAX_CUSTOM_COLOR; i++)
    if (color_table[i].slot == slot)
      return i;

  return -1;
}

char *
html_colorconv (const char *txt)
{
  const char *point;
  char *output;
  colatt_t c;
  char buf[MIL];

  lastcolor = false;
  memset (html_colorbuf, 0, sizeof (html_colorbuf));
  output = html_colorbuf;

  c.at[CT_ATTR] = CL_NONE;
  c.at[CT_FORE] = FG_NONE;
  c.at[CT_BACK] = BG_NONE;

  for (point = txt; *point; point++)
    {
      if (*point == COLORCODE)
	{
	  point++;

	  if (isdigit (*point))
	    {
	      int slot = *point - '0';



	      if (VALID_CL (slot))
		{
		  c.at[CT_ATTR] = slot + CL_MOD;
		}

	      if (*(point + 1) == '+' && *(point + 2))
		{
		  point += 2;
		}
	    }

	  if (*point == '=')
	    {
	      point++;
	      c.at[CT_BACK] = c.at[CT_FORE] + CL_MOD;
	    }

	  switch (*point)
	    {
	    case '}':
	      strcpy (buf, "<br>");
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

	      if (*(point + 1) == '+' && *(point + 2) == '"' && *(point + 3))
		{
		  char fmt[800];
		  size_t t = 0;

		  point += 3;

		  do
		    {
		      fmt[t++] = *point++;
		    }
		  while (*point && *point != '"' && t < sizeof (fmt));

		  fmt[t] = '\0';

		  strcpy (buf, str_time (current_time, -1, fmt));
		}
	      else
		strcpy (buf, str_time (current_time, -1, NULL));
	      break;
	    case 'P':
	    case 'p':
	      break;
	    case COLORCODE:
	      sprintf (buf, "%c", COLORCODE);
	      break;
	    case 'X':
	    case 'x':
	      c.at[CT_ATTR] = CL_NONE;
	      c.at[CT_FORE] = FG_NONE;
	      c.at[CT_BACK] = BG_NONE;
	      sprintf (buf,
		       "%s<script type='text/javascript'>document.write('<span style=\"color: ' + document.fgColor + ';\">');</script>",
		       lastcolor ? "</span>" : "");
	      break;
	    default:
	      set_col_attr (*point, &c, NULL);
	      strcpy (buf, colortohtml (&c));
	      break;
	    }
	  add_text (buf, output);
	}
      else if (*point == CUSTOMSTART)
	{
	  int slot = 0, pos;

	  do
	    {
	      point++;
	      if (isdigit (*point))
		slot = (slot * 10) + (*point - '0');
	    }
	  while (*point && *point != CUSTOMEND);

	  if (slot < 0 || slot >= MAX_CUSTOM_COLOR
	      || (pos = color_slot_lookup (slot)) == -1)
	    {
	      bug ("invalid custom color");
	      strcpy (buf, " ");
	    }
	  else
	    {
	      copy_array (c.at, color_table[pos].col_attr, CT_MAX);
	      strcpy (buf, colortohtml (&c));
	    }
	  add_text (buf, output);
	}
      else if (*point == '<')
	{
	  strcpy (buf, HTML_LT);
	  add_text (buf, output);
	}
      else if (*point == '>')
	{
	  strcpy (buf, HTML_GT);
	  add_text (buf, output);
	}
      else if (*point == '"')
	{
	  strcpy (buf, HTML_QUOTE);
	  add_text (buf, output);
	}
      else if (*point == '&')
	{
	  strcpy (buf, HTML_AMP);
	  add_text (buf, output);
	}
      else if (*point == '\n')
	{
	  strcpy (buf, "<br>");
	  add_text (buf, output);
	}
      else
	{
	  *output++ = *point;
	}
    }

  if (lastcolor == true)
    {
      char *end = "</span>";

      add_text (end, output);
    }

  *output = '\0';
  return html_colorbuf;
}

#define CHAR_URL_PREFIX		"chars"
#define OBJ_URL_PREFIX		"objs"
#define ROOM_URL_PREFIX		"rooms"
#define HELP_URL_PREFIX		"helps"
#define NOTE_URL_PREFIX		"notes"
#define RACE_URL_PREFIX		"races"
#define DEITY_URL_PREFIX	"deity"
#define CMD_URL_PREFIX	    "cmds"
#define CLAN_URL_PREFIX		"clans"
#define CLASS_URL_PREFIX	"class"
#define STAT_URL_PREFIX		"stats"
#define ADMIN_URL_PREFIX	"admin"
#define LOGIN_URL_PREFIX	"login"
#define NAMEGEN_URL_PREFIX  "namegen"
#define SONG_URL_PREFIX		"songs"

const char *
HTTP_URL (const char *fmt, ...)
{
  static char buf[5][MIL];
  char path[MIL];
  char *result;
  static int i;

  path[0] = NUL;
  if (!NullStr (fmt))
    {
      va_list args;

      va_start (args, fmt);
      vsnprintf (path, sizeof (path), fmt, args);
      va_end (args);
    }

  ++i;
  i %= 5;
  result = buf[i];
  sprintf (result, "http://%s:%d/%s", HOSTNAME, webport, path);
  return result;
}

const char *
html_link (const char *desc, const char *fmt, ...)
{
  static char buf[5][MIL];
  char path[MIL];
  char *result, url[MIL];
  static int i;

  path[0] = NUL;
  if (!NullStr (fmt))
    {
      va_list args;

      va_start (args, fmt);
      vsnprintf (path, sizeof (path), fmt, args);
      va_end (args);
    }

  ++i;
  i %= 5;
  result = buf[i];
  sprintf (url, "http://%s:%d/%s", HOSTNAME, webport, path);
  sprintf (result, "<a href=\"%s\">%s</a>", url,
	   !NullStr (desc) ? desc : !NullStr (path) ? path : url);
  return result;
}

const char *
get_next (const char *path, char *base)
{
  static char *buf[5];
  static int i;
  int x;

  if (NullStr (path))
    return path;

  ++i, i %= 5;

  buf[i] = strchr (path, '/');

  base[0] = NUL;
  if (!NullStr (buf[i]))
    {
      for (x = path[0] == '/' ? 1 : 0; path[x] != NUL; x++)
	{
	  if (path[x] == buf[i][0] && !str_cmp (&path[x], buf[i]))
	    {
	      base[x] = NUL;
	      break;
	    }
	  base[x] = path[x];
	}
      if (buf[i][0] == '/')
	buf[i]++;
    }

  return buf[i];
}

const char *
get_prev (const char *path, char *base)
{
  static char *buf[5];
  static int i;
  int x;

  if (NullStr (path))
    return path;

  ++i, i %= 5;

  buf[i] = strrchr (path, '/');

  base[0] = NUL;
  if (!NullStr (buf[i]))
    {
      for (x = path[0] == '/' ? 1 : 0; path[x] != NUL; x++)
	{
	  if (path[x] == buf[i][0] && !str_cmp (&path[x], buf[i]))
	    {
	      base[x] = NUL;
	      break;
	    }
	  base[x] = path[x];
	}
      if (buf[i][0] == '/')
	buf[i]++;
    }

  return buf[i];
}

WebpassData *
get_name_password (WebDescriptor * wdesc, int level)
{
  char *where;
  char encoded[MIL];
  char username[MIL];
  char *password = &str_empty[0];

  username[0] = '\0';
  encoded[0] = '\0';

  where = strstr (wdesc->stuff, "Authorization: Basic");

  if (!where)
    {
      http_status (wdesc, 401);
    }
  else
    {
      where += strlen ("Authorization: Basic");

      where++;
      for (password = encoded; *where && !isspace (*where);
	   where++, password++)
	*password = *where;

      *password = '\0';

      Base64Decode (encoded, (unsigned char *) username, MIL);

      for (password = username; *password && *password != ':'; password++)
	;
      {
	if (*password == ':')
	  {
	    *password = '\0';
	    password++;
	  }
      }
    }

  return check_web_pass (username, password, level);
}

void
under_line (char *under_lined, const char *spaced_out)
{
  char *point;

  strcpy (under_lined, spaced_out);

  for (point = under_lined; *point; point++)
    {
      if (*point == ' ')
	{
	  *point = '_';
	}
      else
	{
	  *point = tolower (*point);
	}
    }
  return;
}

int
min_class_level (int sn)
{
  int min_so_far = MAX_LEVEL;
  int iClass;

  for (iClass = 0; iClass < top_class; iClass++)
    {
      if (skill_table[sn].skill_level[iClass] < min_so_far)
	min_so_far = skill_table[sn].skill_level[iClass];
    }
  return min_so_far;
}

int meta_refresh = -1;
const char *meta_location = NULL;

void
print_header (WebDescriptor * wdesc, const char *title)
{
  int i;

  http_status (wdesc, http_error_code);
  send_content (wdesc, CONTENT_HTML);
  w_println (wdesc, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">");
  w_println (wdesc, "<html><head>");
  w_println (wdesc,
	     "<meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1'>");
  if (meta_refresh > -1)
    {
      w_printlnf (wdesc, "<meta http-equiv='refresh' content='%d;url=%s'",
		  meta_refresh, meta_location);
      meta_refresh = -1;
    }
  w_printlnf (wdesc, "<title>");
  if (!NullStr (title))
    w_printlnf (wdesc, "%s @ %s", Upper (title), mud_info.name);
  else
    w_printlnf (wdesc, "%s Homepage", mud_info.name);
  w_println (wdesc, "</title>");
  w_println (wdesc,
	     "<style type=\"text/css\" title=\"websrv.css\" media=\"screen\">");
  w_println (wdesc,
	     "* { background-color: black; color: silver; font: small sans-serif; letter-spacing: 1px;}");
  w_println (wdesc, "a { text-decoration: none; color: #FFFFCC; }");
  w_println (wdesc,
	     "a:hover { text-decoration: underline; cursor: crosshair; color: #FFFF66;}");
  w_println (wdesc,
	     "hr { color: maroon; }  .header { font-size: larger; letter-spacing: 7px; ");
  w_println (wdesc,
	     "text-decoration: underline; text-indent: 50px; } .above { color: gray; ");
  w_println (wdesc,
	     "font-size: xx-large; text-indent: 50px; letter-spacing: 7px; }");
  w_println (wdesc,
	     ".small { font-size: x-small; }  .footer { font-size: x-small; padding-top: 1cm; position: relative; }");
  w_println (wdesc,
	     ".main { position: relative; left: 140px; top: 20px; width: 80%; } ");
  w_println (wdesc, ".main * td { padding: 4px 4px 4px 4px; }");
  w_println (wdesc,
	     ".menu { border: 1px solid #333333; position: absolute; top: 20px; width: 130px; }");
  w_println (wdesc,
	     ".top { border: 1px solid; font-size: x-large; padding: 4px 4px 4px 4px; }");
  w_println (wdesc, "img { border: none; } ul { list-style-type: square; } ");
  w_println (wdesc,
	     "th { text-align: left; text-indent: 10px; text-decoration: underline; letter-spacing: 5px; }");
  w_println (wdesc,
	     "dt { font-weight: bold; } .pre { white-space: pre; font-family: monospace; }");
  w_println (wdesc, "</style></head><body>");
  w_println (wdesc, "<div class=\"menu\"><table><tr><th>Contents</th></tr>");
  for (i = 0; request_table[i].req != NULL; i++)
    {
      if (!request_table[i].name)
	continue;

      w_printlnf (wdesc, "<tr><td><a %shref='%s'>%s</a></td></tr>",
		  request_table[i].secure ? "class=\"small\" " : "",
		  HTTP_URL (request_table[i].req), request_table[i].name);
    }
  w_println (wdesc,
	     "<tr><td><a href=\"http://validator.w3.org/check/referer\">"
	     "<img src=\"http://www.w3.org/Icons/valid-html401\" alt=\"Valid HTML 4.0.1!\" height=\"31\" width=\"88\">");
  w_printlnf (wdesc,
	      "</a></td></tr><tr><td><a href=\"http://jigsaw.w3.org/css-validator/validator?uri=%s\">",
	      HTTP_URL (NULL));
  w_println (wdesc,
	     "<img width=\"88\" height=\"31\" src=\"http://jigsaw.w3.org/css-validator/images/vcss\" alt=\"Valid CSS!\"></a></td></tr>");
  w_println (wdesc,
	     "<tr><td><a class=\"small\" href=\"http://www.mudconnect.com/\">mudconnect.com</a></td></tr>");
  w_println (wdesc,
	     "<tr><td><a class=\"small\" href=\"javascript:void(0);\" onclick=\"window.open('http://www.topmudsites.com/cgi-bin/topmuds/rankem.cgi?id=markanth');\">");
  w_println (wdesc, "topmudsites.com</a></td></tr>");
  w_println (wdesc, "</table>");
  w_println (wdesc, "</div><div class=\"main\">");
  if (!NullStr (title))
    w_printlnf (wdesc, "<div class=\"top\">%s @ %s</div>",
		Upper (title), mud_info.name);
  else
    w_printlnf (wdesc, "<div class=\"top\">%s Homepage</div>", mud_info.name);

}

void
print_footer (WebDescriptor * wdesc)
{
  w_println (wdesc, "<div class=\"footer\">");
  w_println (wdesc, "<br><address>");
  w_printlnf (wdesc,
	      "This page is automatically generated by %s (%s).",
	      mud_info.name, str_time (-1, -1, NULL));
  w_println (wdesc, "</address>");
  w_println (wdesc,
	     "<table width=\"55%\"><tr><td><hr><table><tr><td></td><td><table><tr><td><b>1stMud Server Project Ring </b></td></tr><tr><td> [ <a href=\"http://C.webring.com/wrman?ring=1stmudserverproj;addsite\">Join</a> | <a href=\"http://C.webring.com/hub?ring=1stmudserverproj;id=3;hub\">List</a> | <a href=\"http://C.webring.com/go?ring=1stmudserverproj;id=3;random\">Random</a> | <b><a href=\"http://C.webring.com/go?ring=1stmudserverproj;id=3;prev\">&lt;&lt; Prev</a> | <a href=\"http://C.webring.com/go?ring=1stmudserverproj;id=3;next\">Next &gt;&gt;</a></b> ] </td></tr></table></td></tr></table></td></tr></table>");
  w_println (wdesc, "</div></div></body>");
  w_println (wdesc, "</html>");
}

void
print_file (WebDescriptor * wdesc, char *filename)
{
  FileData *fp;
  char buf[MSL];
  int c;
  int num = 0;

  if ((fp = f_open (filename, "r")) != NULL)
    {
      while (!f_eof (fp))
	{
	  while ((buf[num] = f_getc (fp)) != EOF
		 && buf[num] != '\n' && buf[num] != '\r' && num < (MSL - 2))
	    num++;
	  c = f_getc (fp);
	  if ((c != '\n' && c != '\r') || c == buf[num])
	    f_ungetc (c, fp);
	  buf[num] = '\0';
	  w_println (wdesc, buf);
	  num = 0;
	}
      f_close (fp);
    }
}

char *
format_obj_to_html (ObjData * obj)
{
  AffectData *paf;
  static char output[MSL * 5];
  char buf[MSL * 5];

  output[0] = '\0';

  if (NullStr (obj->description))
    return output;

  if (IsObjStat (obj, ITEM_INVIS))
    strcat (output, "(Invis) ");
  if (IsObjStat (obj, ITEM_DARK))
    strcat (output, "(Dark) ");
  if (IsObjStat (obj, ITEM_EVIL))
    strcat (output, "(Red Aura) ");
  if (IsObjStat (obj, ITEM_BLESS))
    strcat (output, "(Blue Aura) ");
  if (IsObjStat (obj, ITEM_MAGIC))
    strcat (output, "(Magical) ");
  if (IsObjStat (obj, ITEM_GLOW))
    strcat (output, "(Glowing) ");
  if (IsObjStat (obj, ITEM_HUM))
    strcat (output, "(Humming) ");

  if (obj->item_type == ITEM_WEAPON)
    {
      for (paf = obj->affect_first; paf; paf = paf->next)
	{
	  if (IsSet (paf->bitvector, WEAPON_FLAMING))
	    strcat (output, "(Flaming) ");
	  if (IsSet (paf->bitvector, WEAPON_FROST))
	    strcat (output, "(Frost) ");
	  if (IsSet (paf->bitvector, WEAPON_VAMPIRIC))
	    strcat (output, "(Vampiric) ");
	  if (IsSet (paf->bitvector, WEAPON_SHOCKING))
	    strcat (output, "(Shocking) ");
	  if (IsSet (paf->bitvector, WEAPON_POISON))
	    strcat (output, "(Poison) ");
	  if (IsSet (paf->bitvector, WEAPON_SHARP))
	    strcat (output, "(Sharp) ");
	  if (IsSet (paf->bitvector, WEAPON_VORPAL))
	    strcat (output, "(Vorpal) ");
	}
    }

  if (IsObjStat (obj, ITEM_AUCTIONED))
    strcat (output, "(Auctioned) ");

  if (obj->condition <= 9 && obj->condition >= 0)
    strcat (output, "(Ruined) ");
  else if (obj->condition >= 10 && obj->condition <= 24)
    strcat (output, "(Broken) ");

  if (obj->description != NULL)
    {
      sprintf (buf, "<a href='%s'>%s</a><br>",
	       HTTP_URL (OBJ_URL_PREFIX "/%ld", obj->pIndexData->vnum),
	       html_colorconv (obj->description));
      strcat (output, buf);
    }

  return output;
}

char *
show_list_to_html (ObjData * list)
{
  static char output[MSL * 6];
  const char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  ObjData *obj;
  int nShow;
  int iShow;
  int count;
  char temp[MSL];
  bool found;

  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    count++;

  alloc_mem (prgpstrShow, const char *, count);
  alloc_mem (prgnShow, int, count);

  nShow = 0;

  output[0] = '\0';

  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE)
	{
	  pstrShow = format_obj_to_html (obj);
	  found = false;

	  for (iShow = nShow - 1; iShow >= 0; iShow--)
	    {
	      if (!str_cmp (prgpstrShow[iShow], pstrShow))
		{
		  prgnShow[iShow]++;
		  found = true;
		  break;
		}
	    }
	  if (!found)
	    {
	      prgpstrShow[nShow] = str_dup (pstrShow);
	      prgnShow[nShow] = 1;
	      nShow++;
	    }
	}
    }

  for (iShow = 0; iShow < nShow; iShow++)
    {
      if (NullStr (prgpstrShow[iShow]))
	{
	  free_string (prgpstrShow[iShow]);
	  continue;
	}

      if (prgnShow[iShow] != 1)
	{
	  sprintf (temp, "(%2d) ", prgnShow[iShow]);
	  strcat (output, temp);
	}
      else
	strcat (output, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

      sprintf (temp, "%s<br>", prgpstrShow[iShow]);
      strcat (output, temp);

      free_string (prgpstrShow[iShow]);

      if (strlen (output) > 5500)
	{
	  strcat (output, "     (More stuff not shown)<br>");
	  break;
	}
    }

  free_mem (prgpstrShow);
  free_mem (prgnShow);
  return output;
}

char *
show_char_to_html_0 (CharData * victim)
{
  static char buf[MSL * 6];
  char temp[MSL];

  buf[0] = '\0';

  if (IsAffected (victim, AFF_INVISIBLE))
    strcat (buf, "(Invis) ");
  if (victim->invis_level >= LEVEL_IMMORTAL)
    strcat (buf, "(Wizi) ");
  if (IsAffected (victim, AFF_HIDE))
    strcat (buf, "(Hide) ");
  if (IsAffected (victim, AFF_CHARM))
    strcat (buf, "(Charmed) ");
  if (IsAffected (victim, AFF_PASS_DOOR))
    strcat (buf, "(Translucent) ");
  if (IsAffected (victim, AFF_FAERIE_FIRE))
    strcat (buf, "(Pink Aura) ");
  if (IsEvil (victim))
    strcat (buf, "(Red Aura) ");
  if (IsGood (victim))
    strcat (buf, "(Golden Aura) ");
  if (IsAffected (victim, AFF_SANCTUARY))
    strcat (buf, "(White Aura) ");
  if (!IsNPC (victim) && !victim->desc)
    strcat (buf, "(Linkdead) ");

  if (!IsNPC (victim))
    {
      if (IsSet (victim->comm, COMM_AFK))
	strcat (buf, "*AFK* ");
      if (victim->war)
	strcat (buf, "(War) ");
      if (!IsNPC (victim) && IsSet (victim->act, PLR_KILLER))
	strcat (buf, "(KILLER) ");
      if (!IsNPC (victim) && IsSet (victim->act, PLR_THIEF))
	strcat (buf, "(THIEF) ");
      if (IsSet (victim->comm, COMM_QUIET))
	strcat (buf, "[QUIET] ");
      if (victim->desc && victim->desc->editor != ED_NONE)
	strcat (buf, "[OLC] ");
      if (victim->pcdata->in_progress != NULL)
	strcat (buf, "[Note] ");
      if (IsQuester (victim))
	strcat (buf, "[Q] ");
    }

  if (victim->level > 0)
    {
      long vict_condition;

      if (victim->max_hit > 0)
	vict_condition = victim->hit * 100 / victim->max_hit;
      else
	vict_condition = -1;

      if (vict_condition < 0)
	strcat (buf, "(DEAD) ");
      else if (vict_condition < 33)
	strcat (buf, "(Wounded) ");
    }

  if (victim->position ==
      (IsNPC (victim) ? victim->start_pos : POS_STANDING)
      && !NullStr (victim->long_descr))
    {
      if (IsNPC (victim))
	{
	  sprintf (temp, "<a href='%s'>%s</a>",
		   HTTP_URL (CHAR_URL_PREFIX "/%ld",
			     victim->pIndexData->vnum),
		   html_colorconv (victim->long_descr));
	  strcat (buf, temp);
	}
      return (buf);
    }
  if (IsNPC (victim))
    {
      sprintf (temp, "<a href='%s'>%s</a>", HTTP_URL (CHAR_URL_PREFIX "/%ld",
						      victim->
						      pIndexData->vnum),
	       html_colorconv (victim->short_descr));
      strcat (buf, temp);
    }
  else
    {
      strcat (buf, victim->name);
    }

  switch (victim->position)
    {
    case POS_DEAD:
      strcat (buf, " is DEAD!!");
      break;
    case POS_MORTAL:
      strcat (buf, " is mortally wounded.");
      break;
    case POS_INCAP:
      strcat (buf, " is incapacitated.");
      break;
    case POS_STUNNED:
      strcat (buf, " is lying here stunned.");
      break;
    case POS_SLEEPING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], SLEEP_AT))
	    {
	      sprintf (temp, " is sleeping at %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else if (IsSet (victim->on->value[2], SLEEP_ON))
	    {
	      sprintf (temp, " is sleeping on %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else
	    {
	      sprintf (temp, " is sleeping in %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	}
      else
	strcat (buf, " is sleeping here.");
      break;
    case POS_RESTING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], REST_AT))
	    {
	      sprintf (temp, " is resting at %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else if (IsSet (victim->on->value[2], REST_ON))
	    {
	      sprintf (temp, " is resting on %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else
	    {
	      sprintf (temp, " is resting in %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	}
      else
	strcat (buf, " is resting here.");
      break;
    case POS_SITTING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], SIT_AT))
	    {
	      sprintf (temp, " is sitting at %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else if (IsSet (victim->on->value[2], SIT_ON))
	    {
	      sprintf (temp, " is sitting on %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else
	    {
	      sprintf (temp, " is sitting in %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	}
      else
	strcat (buf, " is sitting here.");
      break;
    case POS_STANDING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], STAND_AT))
	    {
	      sprintf (temp, " is standing at %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else if (IsSet (victim->on->value[2], STAND_ON))
	    {
	      sprintf (temp, " is standing on %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	  else
	    {
	      sprintf (temp, " is standing in %s.", victim->on->short_descr);
	      strcat (buf, temp);
	    }
	}
      strcat (buf, " is here.");
      break;
    case POS_FIGHTING:
      strcat (buf, " is here, fighting ");
      if (victim->fighting == NULL)
	strcat (buf, "thin air??");
      else if (victim->in_room == victim->fighting->in_room)
	{
	  strcat (buf,
		  IsNPC (victim) ? victim->fighting->
		  short_descr : victim->fighting->name);
	  strcat (buf, ".");
	}
      else
	strcat (buf, "someone who left??");
      break;
    default:
      break;
    }
  return (buf);
}

char *
show_char_to_html (CharData * list)
{
  CharData *rch;
  static char output[MSL * 10];

  output[0] = '\0';
  for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->invis_level >= LEVEL_IMMORTAL)
	continue;

      strcat (output, show_char_to_html_0 (rch));
      strcat (output, "<br>");
    }
  return (output);
}

Web_Fun (HandleMemoryRequest)
{
  int sn, count_spell = 0, count_skill = 0;

  for (sn = 0; sn < top_skill; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;

      if (skill_table[sn].spell_fun != spell_null)
	count_spell += 1;
      else
	count_skill += 1;
    }
  print_header (wdesc, "Technical Info");
  w_println (wdesc, "<table><tr>");
  w_printlnf (wdesc, "<table><tr><td>Version</td><td>%s</td></tr>",
	      MUDSTRING);
  w_println (wdesc, "<tr><td>Compiled on</td><td>" __DATE__ "</td><tr>");
  w_printlnf (wdesc, "<td>Affects</td>  <td>%d</td></tr>", top_affect);
  w_printlnf (wdesc, "<tr><td>Areas</td> <td>%d</td></tr>", top_area);
  w_printlnf (wdesc, "<tr><td>Extra Descriptions</td> <td>%d</td></tr>",
	      top_ed);
  w_printlnf (wdesc, "<tr><td>Exits</td> <td>%d</td></tr>", top_exit);
  w_printlnf (wdesc, "<tr><td>Helps</td> <td>%d</td></tr>", top_help);
  w_printlnf (wdesc, "<tr><td>Socials</td> <td>%d</td></tr>", top_social);
  w_printlnf (wdesc, "<tr><td>Spells</td> <td>%d</td></tr>", count_spell);
  w_printlnf (wdesc, "<tr><td>Spell Groups</td><td>%d</td></tr>", top_group);
  w_printlnf (wdesc, "<tr><td>Skills</td> <td>%d</td></tr>", count_skill);
  w_printlnf (wdesc, "<tr><td>Mobiles</td> <td>%d (%d in use)</td></tr>",
	      top_char_index, mobile_count);
  w_printlnf (wdesc, "<tr><td>Objects</td> <td>%d (%d in use)</td></tr>",
	      top_obj_index, top_obj);
  w_printlnf (wdesc, "<tr><td>Resets</td> <td>%d</td></tr>", top_reset);
  w_printlnf (wdesc, "<tr><td>Rooms</td> <td>%d (%d explorable)</td></tr>",
	      top_room_index, top_explored);
  w_printlnf (wdesc, "<tr><td>Shops</td> <td>%d</td></tr>", top_shop);
  w_printlnf (wdesc, "<tr><td>Number of Players</td><td>%d</td></tr>",
	      pfiles.count);
  w_printlnf (wdesc, "<tr><td>Races</td><td>%d</td></tr>", top_race);
  w_printlnf (wdesc, "<tr><td>Programs (mob,obj,room)</td><td>%d</td></tr>",
	      top_mprog + top_oprog + top_rprog);
  w_printlnf (wdesc, "<tr><td>Bans</td><td>%d</td></tr>", top_ban);
  w_printlnf (wdesc, "<tr><td>Deities</td><td>%d</td></tr>", top_deity);
  w_printlnf (wdesc, "<tr><td>Clans</td><td>%d (%d members)</td></tr>",
	      top_clan, top_mbr);
  w_printlnf (wdesc, "<tr><td>Commands</td><td>%d</td></tr>", top_cmd);
  w_printlnf (wdesc, "<tr><td>Classes</td><td>%d</td></tr>", top_class);
  w_printlnf (wdesc, "<tr><td>Channels</td><td>%d</td></tr>", top_channel);
  w_printlnf (wdesc, "<tr><td>Notes</td><td>%d</td></tr>", top_note);
  w_printlnf (wdesc, "<tr><td>MSP Sounds</td><td>%d</td></tr>", top_msp);
  w_printlnf (wdesc, "<tr><td>String Space</td><td>%d (%s)</td></tr>",
	      nAllocString, fsize (sAllocString));
  w_println (wdesc, "</table>");
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleRulesRequest)
{
  HelpData *pHelp;

  print_header (wdesc, "Rules");
  w_println (wdesc, "<p>");
  if ((pHelp = help_lookup ("RULES")) != NULL)
    {
      w_println (wdesc, html_colorconv (help_text (pHelp->text)));
    }
  else
    w_println (wdesc, "There are no rules!! Anarchy!!");
  w_println (wdesc, "</p>");
  print_footer (wdesc);
  return true;
}


Web_Fun (HandleCreditsRequest)
{
  HelpData *pHelp;

  print_header (wdesc, "Credits");
  if ((pHelp = help_lookup ("DIKU")) != NULL)
    {
      w_println (wdesc, "<hr><p>");
      w_println (wdesc, html_colorconv (help_text (pHelp->text)));
      w_println (wdesc, "</p>");
    }
  if ((pHelp = help_lookup ("MERC")) != NULL)
    {
      w_println (wdesc, "<hr><p>");
      w_println (wdesc, html_colorconv (help_text (pHelp->text)));
      w_println (wdesc, "</p>");
    }
  if ((pHelp = help_lookup ("ROM CREDITS")) != NULL)
    {
      w_println (wdesc, "<hr><p>");
      w_println (wdesc, html_colorconv (help_text (pHelp->text)));
      w_println (wdesc, "</p>");
    }
  if ((pHelp = help_lookup ("1STMUD CREDITS")) != NULL)
    {
      w_println (wdesc, "<hr><p>");
      w_println (wdesc, html_colorconv (help_text (pHelp->text)));
      w_println (wdesc, "</p>");
    }
  else
    {
      w_printlnf
	(wdesc, "<hr><table><tr><td>%s (%s) design by:</td>",
	 mud_info.name, MUDSTRING);
      w_printlnf (wdesc,
		  "<td>Markanth</td><td>&lt;markanth@firstmud.com&gt;</td></tr>");
    }
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleAreaRequest)
{
  AreaData *pArea;
  int count = 0;

  print_header (wdesc, "Areas");
  w_println (wdesc, "<table>");
  w_println (wdesc, "<tr>");
  w_println (wdesc, "<th>Low/High Level</th>");
  w_println (wdesc, "<th>Author</th>");
  w_println (wdesc, "<th>Area Name</th></tr>");
  for (pArea = area_first_sorted; pArea; pArea = pArea->next_sort)
    {
      if (!IsSet (pArea->area_flags, AREA_CLOSED | AREA_PLAYER_HOMES))
	{
	  w_println (wdesc, "<tr>");
	  if (!NullStr (pArea->lvl_comment))
	    w_printlnf (wdesc, "<td>%s</td>", pArea->lvl_comment);
	  else
	    w_printlnf (wdesc, "<td>%03d %03d</td>", pArea->min_level,
			pArea->max_level);
	  w_printlnf (wdesc, "<td>%s</td>", pArea->credits);
	  w_printlnf (wdesc, "<td>%s</td></tr>", pArea->name);
	  count++;
	}
    }
  w_println (wdesc, "</table>");
  w_printlnf (wdesc, "<p>Areas Found: %d.</p>", count);
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleSpellsRequest)
{
  int gn;
  int sn, i;
  int tn;
  RaceData *race;
  bool *displayed;
  int past_default = true;
  char buf[MSL];
  char buf2[MSL];
  char buf3[MSL];

  alloc_mem (displayed, bool, top_skill);
  memset (displayed, false, top_skill);

  print_header (wdesc, "Spells and Skills");

  for (gn = 0; gn < top_group; gn++)
    {
      if (group_table[gn].name == NULL)
	break;

      if (!str_cmp (group_table[gn].name, "rom basics"))
	{
	  w_println (wdesc, "<H3>Basic Skills</H3>");
	  w_println (wdesc,
		     "All players receive the <I>basic</I> skills for their class.<br>");
	  w_println (wdesc, "<table><tr>");
	  w_println (wdesc, "<td>Basic Group</td>");
	  w_println (wdesc, "<td>Skills Included</td></tr>");
	}

      if (!str_cmp (group_table[gn].name, "mage default"))
	{

	  past_default = false;
	  w_println (wdesc, "</table>");
	  w_println (wdesc, "<H3>Default Skill/Spell Groups</H3>");
	  w_println (wdesc,
		     "Players receive the <I>default</I> skills and spell groups if they bypass customization.<br>");
	  w_println (wdesc, "<table><tr>");
	  w_println (wdesc, "<td>Default Group</td>");
	  w_println (wdesc, "<td>Skills Included</td>");
	  w_println (wdesc, "<td>Skill/Spell Groups Included</td></tr>");
	}

      if (!str_cmp (group_table[gn].name, "weaponsmaster"))
	{
	  past_default = true;
	  w_println (wdesc, "</table>");
	  w_println (wdesc, "<H3>Other Skill/Spell Groups</H3>");
	  w_println (wdesc, "Players may gain these skills and spells.<br>");
	  w_println (wdesc, "<table><tr>");
	  w_println (wdesc, "<td>Skill/Spell Group</td>");
	  w_println (wdesc, "<td>Skills/Spells Included</td></tr>");
	}

      under_line (buf3, group_table[gn].name);
      w_printlnf (wdesc, "<tr><td><a name='%s'>%s</a></td><td>",
		  buf3, group_table[gn].name);

      buf[0] = '\0';
      for (sn = 0; sn < MAX_IN_GROUP; sn++)
	{
	  if (group_table[gn].spells[sn] == NULL)
	    break;
	  tn = skill_lookup (group_table[gn].spells[sn]);
	  if (tn != -1)
	    {
	      sprintf (buf2, "%s, ", group_table[gn].spells[sn]);
	      strcat (buf, buf2);

	      displayed[tn] = true;
	    }
	}
      if (!NullStr (buf) && (strlen (buf) > 2))
	buf[(strlen (buf) - 2)] = '\0';

      else
	strcpy (buf, "None");

      if (!past_default)
	{
	  w_printlnf (wdesc, "%s</td><td>", buf);

	  buf[0] = '\0';
	  for (sn = 0; sn < MAX_IN_GROUP; sn++)
	    {
	      if (group_table[gn].spells[sn] == NULL)
		break;
	      tn = skill_lookup (group_table[gn].spells[sn]);
	      if (tn == -1)
		{
		  under_line (buf3, group_table[gn].spells[sn]);
		  sprintf (buf2, "<a href='#%s'>%s</a>, ", buf3,
			   group_table[gn].spells[sn]);
		  strcat (buf, buf2);
		}
	      else
		{

		  displayed[tn] = true;
		}
	    }
	  if (!NullStr (buf) && (strlen (buf) > 2))
	    buf[(strlen (buf) - 2)] = '\0';

	  else
	    strcpy (buf, "None");
	}
      w_printlnf (wdesc, "%s</td></tr>", buf);
    }
  w_println (wdesc, "</table>");

  w_println (wdesc, "<H3>Other Skills and Spells</H3>");
  w_println (wdesc,
	     "The following skills and spells are available to various mortals.<br>");
  w_println (wdesc, "<table><tr><td>");
  buf[0] = '\0';

  for (sn = 1; sn < top_skill; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;

      if (!displayed[sn] && (min_class_level (sn) < ANGEL))
	{
	  sprintf (buf2, "%s, ", skill_table[sn].name);
	  strcat (buf, buf2);
	  displayed[sn] = true;
	}
    }
  if (!NullStr (buf) && (strlen (buf) > 2))
    buf[(strlen (buf) - 2)] = '\0';

  else
    strcpy (buf, "None");
  w_printlnf (wdesc, "%s</td></tr>", buf);
  w_println (wdesc, "</table>");

  w_printlnf (wdesc,
	      "<p>To view skills in class specific tables, visit the <a href='%s'>Classes</a> section.</p>",
	      HTTP_URL (CLASS_URL_PREFIX));

  w_println (wdesc, "<H3><a NAME='RaceSkills'>Race Skills</a></H3>");
  w_println (wdesc, "The following skills and spells are race specific.");

  w_println (wdesc, "<table><tr>");

  w_println (wdesc, "<td>Race</td>");
  w_println (wdesc, "<td>Skills</td></tr>");

  for (race = race_first; race; race = race->next)
    {
      if (!race->pc_race)
	continue;
      w_printlnf (wdesc, "<tr><td>%s</td><td>", race->name);
      buf[0] = '\0';
      for (sn = 1; sn < top_skill; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  for (i = 0; i < MAX_RACE_SKILL; i++)
	    {
	      if (race->skills[i] == NULL)
		break;
	      if (skill_lookup (race->skills[i]) == sn)
		{
		  sprintf (buf2, "%s, ", skill_table[sn].name);
		  strcat (buf, buf2);
		  displayed[sn] = true;
		}
	    }
	}
      if (!NullStr (buf) && (strlen (buf) > 2))
	buf[(strlen (buf) - 2)] = '\0';

      else
	strcpy (buf, "None");
      w_printlnf (wdesc, "%s</td></tr>", buf);
    }
  w_println (wdesc, "</table>");

  w_println (wdesc, "<H3>Immortal Skills and Spells</H3>");
  w_println (wdesc,
	     "The following skills and spells are available to various immortals.");

  w_println (wdesc, "This list includes some spells under development.<br>");
  w_println (wdesc, "<table><tr>");

  w_println (wdesc, "<td>Spells</td></tr>");
  w_println (wdesc, "<tr><td>");
  buf[0] = '\0';
  for (sn = 1; sn < top_skill; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;
      if (!displayed[sn])
	{
	  sprintf (buf2, "%s, ", skill_table[sn].name);
	  strcat (buf, buf2);
	  displayed[sn] = true;
	}
    }
  if (!NullStr (buf) && (strlen (buf) > 2))
    buf[(strlen (buf) - 2)] = '\0';

  else
    strcpy (buf, "None");
  w_printlnf (wdesc, "%s</td></tr>", buf);
  w_println (wdesc, "</table>");
  print_footer (wdesc);
  free_mem (displayed);
  return true;
}

Web_Fun (HandleWhoRequest)
{
  CharData *wch;

  print_header (wdesc, "Players currently");
  w_println (wdesc, "<table><tr>");
  w_println (wdesc, "<th>Level</th><th>Race</th><th>Class</th><th>Clan</th>");
  w_println (wdesc, "<th>Name</th></tr>");
  for (wch = player_first; wch != NULL; wch = wch->next_player)
    {
      if (wch->invis_level >= LEVEL_IMMORTAL
	  || wch->incog_level >= LEVEL_IMMORTAL)
	continue;

      w_println (wdesc, "<tr>");
      if (NullStr (wch->pcdata->who_descr))
	{
	  w_printlnf (wdesc, "<td>%d</td>", wch->level);
	  w_printlnf (wdesc,
		      "<td><a href='%s'>%s</a></td>",
		      HTTP_URL (RACE_URL_PREFIX "/%s", wch->race->name),
		      wch->race->name);
	  w_printlnf (wdesc, "<td>%s</td>", class_short (wch));
	}
      else
	{
	  w_printlnf (wdesc, "<td colspan=3>%s</td>",
		      html_colorconv (wch->pcdata->who_descr));
	}

      if (is_clan (wch))
	{
	  w_printlnf (wdesc,
		      "<td><a href='%s'>%s</a></td>",
		      HTTP_URL (CLAN_URL_PREFIX "/%s", CharClan (wch)->name),
		      html_colorconv (CharClan (wch)->who_name));
	}
      else
	w_println (wdesc, "<td></td>");

      w_println (wdesc, "<td>");
      w_println (wdesc, wch->name);

      w_println (wdesc, html_colorconv (wch->pcdata->title));
      w_println (wdesc, "</td></tr>");

    }
  w_println (wdesc, "</table><br>");
  print_footer (wdesc);
  return true;
}

const char *stat_type_name[MAX_GAMESTAT + 1][2] = {

  {"GENERAL STATS", "general"},
  {"PLAYER KILLERS", "pkill"},
  {"MOB KILLERS", "mkill"},
  {"PK DEATHS", "pkdead"},
  {"MOB DEATHS", "mdead"}
};

Web_Fun (HandleStatsRequest)
{
  int pos;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Stats");
      w_println (wdesc, "<ul>");
      for (pos = 0; pos <= MAX_GAMESTAT; pos++)
	{
	  w_printlnf (wdesc,
		      "<li><a href='%s'>%s</a>",
		      HTTP_URL (STAT_URL_PREFIX "/%s",
				stat_type_name[pos][1]),
		      stat_type_name[pos][0]);
	}
      w_println (wdesc, "</ul>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      if (!str_cmp ("general", wdesc->buf))
	{
	  print_header (wdesc, "General Stats");
	  w_println (wdesc, "<table><tr>");
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Logins",
		      mud_info.stats.logins);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Quests",
		      mud_info.stats.quests);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Quests Complete", mud_info.stats.qcomplete);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Levels",
		      mud_info.stats.levels);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>", "Newbies",
		      mud_info.stats.newbies);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Deletions",
		      mud_info.stats.deletions);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Mob Deaths", mud_info.stats.mobdeaths);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Auctions",
		      mud_info.stats.auctions);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Auctions Sold", mud_info.stats.aucsold);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Player Deaths",
		      mud_info.stats.pdied);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Player Kills", mud_info.stats.pkill);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Notes",
		      mud_info.stats.notes);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>", "Remorts",
		      mud_info.stats.remorts);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Wars",
		      mud_info.stats.wars);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Global Quests", mud_info.stats.gquests);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Connections",
		      mud_info.stats.connections);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>",
		      "Connects this Boot", mud_info.stats.boot_connects);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Online Record",
		      mud_info.stats.online);
	  w_printlnf (wdesc, "</tr><tr><td>%s</td><td>%ld</td>", "Web Hits",
		      mud_info.stats.web_requests);
	  w_printlnf (wdesc, "<td>%s</td><td>%ld</td>", "Channel Messages",
		      mud_info.stats.chan_msgs);
	  w_println (wdesc, "</tr></table>");
	  w_printlnf (wdesc, "<p>Stats since: %s</p>",
		      timestr (current_time - mud_info.stats.lastupdate,
			       false));
	  print_footer (wdesc);
	  return true;
	}
      for (pos = 1; pos <= MAX_GAMESTAT; pos++)
	{
	  if (!str_cmp (stat_type_name[pos][1], wdesc->buf))
	    {
	      Proto (int compare_stats, (const void *, const void *));
	      char temp[MSL];
	      StatData *curr;
	      StatData **top;
	      int count, loop;
	      bool found = false;
	      EXTERN int compare_type;

	      sprintf (temp, "Ranking of %s", stat_type_name[pos][0]);
	      print_header (wdesc, temp);
	      w_println (wdesc, "<table>");

	      alloc_mem (top, StatData *, top_stat);

	      count = 0;
	      compare_type = pos;
	      loop = 0;
	      pos = 0;
	      for (curr = stat_first; curr != NULL; curr = curr->next)
		{
		  top[count] = curr;
		  count++;
		  found = true;
		}

	      qsort (top, count, sizeof (*top), compare_stats);

	      w_println (wdesc,
			 "<tr><th>Rank</th><th>Name</th><th>Number</th>"
			 "<th>Rank</th><th>Name</th><th>Number</th></tr>");
	      for (loop = 0; loop < count; loop++)
		{
		  if (loop >= 50)
		    break;

		  w_printlnf (wdesc,
			      "%s<td>%2d)</td><td>%-20s</td><td>%ld</td>",
			      pos == 0 ? "<tr>" : "", loop + 1,
			      top[loop]->name,
			      top[loop]->gamestat[compare_type]);

		  if (++pos % 2 == 0)
		    {
		      w_println (wdesc, "</tr>");
		      pos = 0;
		    }

		}
	      if (!found)
		w_println (wdesc,
			   "<tr><td colspan=3>No one found yet.</td></tr>");

	      w_println (wdesc, "</table>");

	      print_footer (wdesc);
	      free_mem (top);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleCommandsRequest)
{
  CmdData *i;
  int pos = 0;
  HelpData *pHelp;
  int count = 0;

  print_header (wdesc, "Commands");
  w_println (wdesc, "<table>");

  for (i = cmd_first_sorted; i; i = i->next_sort)
    {
      if (i->level >= LEVEL_IMMORTAL || i->category == CMDCAT_NOSHOW)
	continue;
      count = 0;
      for (pHelp = help_first; pHelp; pHelp = pHelp->next)
	{
	  count++;
	  if (is_name (i->name, pHelp->keyword))
	    break;
	}
      if (pHelp)
	w_printlnf (wdesc, "%s<td><a href='%s'>%s</td>",
		    (pos == 0) ? "<tr>" : "",
		    HTTP_URL (HELP_URL_PREFIX "/%d", count), i->name);
      else
	w_printlnf (wdesc, "%s<td>%s</td>", (pos == 0) ? "<tr>" : "",
		    i->name);
      if (++pos % 5 == 0)
	{
	  w_println (wdesc, "</tr>");
	  pos = 0;
	}
    }
  w_println (wdesc, "</table>");
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleHelpsRequest)
{
  HelpData *pHelp;
  int pos = 0;

  if (NullStr (wdesc->buf))
    {
      int count = 0;

      print_header (wdesc, "Help Files");
      w_println (wdesc, "<table>");
      for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	{
	  count++;
	  if (pHelp->level <= MAX_MORTAL_LEVEL && pHelp->level >= 0)
	    {
	      const char *temp;
	      char wordkey[MSL];

	      temp = pHelp->keyword;
	      while (!NullStr (temp))
		{
		  wordkey[0] = '\0';
		  temp = one_argument (temp, wordkey);
		  w_printlnf (wdesc,
			      "%s<td><a href='%s'>%s</a></td>",
			      (pos == 0) ? "<tr>" : "",
			      HTTP_URL (HELP_URL_PREFIX "/%d", count),
			      wordkey);
		  if (++pos % 5 == 0)
		    {
		      w_println (wdesc, "</tr>");
		      pos = 0;
		    }
		}
	    }
	}
      w_println (wdesc, "</table>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      char temp[MIL];

      pos = 0;
      for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	{
	  pos++;
	  sprintf (temp, "%d", pos);
	  if (!str_cmp (wdesc->buf, temp))
	    {
	      print_header (wdesc, pHelp->keyword);
	      w_println (wdesc, "<table>");
	      w_printlnf (wdesc,
			  "<tr><td>[%d] %s<td><td>Category: %s</td></tr>",
			  pHelp->level, pHelp->keyword,
			  flag_string (help_types, pHelp->category));

	      w_printlnf (wdesc, "<tr><td colspan=\"2\">%s</td></tr>",
			  html_colorconv (fix_string (pHelp->text)));
	      w_println (wdesc, "</table>");

	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  print_header (wdesc, "Can't find help");
  w_printlnf (wdesc, "<p>Can't find help %s</p>", wdesc->buf);
  print_footer (wdesc);
  return true;

  return false;
}


Web_Fun (HandleDeityRequest)
{
  DeityData *d;
  int e;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Deities");
      w_println (wdesc,
		 "<table><tr><th>Name</th><th>Ethos</th><th>Description</th></tr>");
      for (e = ETHOS_LAWFUL_GOOD; e != ETHOS_CHAOTIC_EVIL; e--)
	for (d = deity_first; d; d = d->next)
	  {
	    if (d->ethos != (ethos_t) e)
	      continue;

	    w_printlnf (wdesc,
			"<tr><td>%s</td><td>%s</td><td>%s</td></tr>",
			d->name, flag_string (ethos_types, d->ethos),
			d->desc);
	  }
      w_println (wdesc, "</table>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (d = deity_first; d; d = d->next)
	{
	  if (!str_cmp (d->name, wdesc->buf))
	    {
	      HelpData *pHelp;

	      print_header (wdesc, d->name);
	      if ((pHelp = help_lookup (d->name)) != NULL)
		{
		  w_printlnf (wdesc, "<p>%s</p><br>",
			      html_colorconv (pHelp->text));
		}
	      else
		w_println (wdesc, "<p>No Info Available</p><br>");

	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleRaceRequest)
{
  RaceData *race;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Races");
      w_println (wdesc, "<table><tr>");
      w_println (wdesc, "<th>Race</th>");
      w_printlnf (wdesc,
		  "<th>Str</th><th>Int</th><th>Wis</th><th>Dex</th><th>Con</th>");
      w_println (wdesc, "<th>Creation<br>Points</th></tr>");
      for (race = race_first; race != NULL; race = race->next)
	{
	  if (!race->pc_race)
	    continue;

	  w_println (wdesc, "<tr>");
	  w_printlnf (wdesc,
		      "<td><a href='%s"
		      "'>%s</a></td>" LF "<td>%d</td>" LF "<td>%d</td>" LF
		      "<td>%d</td>" LF "<td>%d</td>" LF
		      "<td>%d</td><td align='center'>%d</td></tr>",
		      HTTP_URL (RACE_URL_PREFIX "/%s", race->name),
		      race->name, race->max_stats[STAT_STR],
		      race->max_stats[STAT_INT], race->max_stats[STAT_WIS],
		      race->max_stats[STAT_DEX], race->max_stats[STAT_CON],
		      race->points);
	}
      w_println (wdesc, "</table>");
      w_printlnf (wdesc,
		  "<p>Creation points increase the amount of experience it takes to gain a level. Maximum a stat can go is 30.</p>");
      w_printlnf (wdesc,
		  "<p>To view skills and spells available to each race, visit the <a href='%s'>Skill/Spell</a> section.</p>",
		  HTTP_URL ("spells#RaceSkills"));
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (race = race_first; race; race = race->next)
	{
	  if (!str_cmp (race->name, wdesc->buf))
	    {
	      HelpData *pHelp;

	      print_header (wdesc, race->name);
	      if ((pHelp = help_lookup (race->name)) != NULL)
		{
		  w_printlnf (wdesc, "<p>%s</p><br>",
			      html_colorconv (pHelp->text));
		}
	      else
		w_println (wdesc, "<p>No Info Available</p><br>");

	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleClanRequest)
{
  ClanData *clan;
  char i[MSL], j[MSL];
  ClanMember *pmbr;
  int e;

  print_header (wdesc, "Clans");
  w_println (wdesc, "<table>");
  w_println (wdesc,
	     "<tr><th>Name</th><th>Ethos</th><th>Description</th><th>Leaders</th></tr>");
  for (e = ETHOS_LAWFUL_GOOD; e != ETHOS_CHAOTIC_EVIL; e--)
    for (clan = clan_first; clan; clan = clan->next)
      {
	if (clan->ethos != (ethos_t) e)
	  continue;

	i[0] = '\0';
	j[0] = '\0';
	for (pmbr = mbr_first; pmbr != NULL; pmbr = pmbr->next)
	  {
	    if (pmbr->clan != clan || pmbr->rank != (MAX_RANK - 1))
	      continue;
	    sprintf (j, " %s,", pmbr->name);
	    strcat (i, j);
	  }
	if (!NullStr (i))
	  {
	    i[strlen (i) - 1] = '\0';
	    w_printlnf (wdesc,
			"<tr><td>%s%s</td><td>%s</td><td>%s</td><td>%s</td></tr>",
			html_colorconv (clan->who_name),
			flag_string (ethos_types, clan->ethos),
			html_colorconv (clan->description), i);
	  }
	else
	  w_printlnf (wdesc,
		      "<tr><td>%s</td><td>%s</td><td>%s</td><td>None</td></tr>",
		      html_colorconv (clan->who_name),
		      flag_string (ethos_types, clan->ethos),
		      html_colorconv (clan->description));
      }
  w_println (wdesc, "</table>");
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleClassRequest)
{
  int i;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Classes");
      w_println (wdesc, "<table>");
      w_println (wdesc, "<tr><th>Name</th><th>Description</th></tr>");
      for (i = 0; i < top_class; i++)
	{
	  w_printlnf (wdesc,
		      "<tr><td><a href='%s'>%s</a>"
		      "</td><td>%s</td></tr>",
		      HTTP_URL (CLASS_URL_PREFIX "/%s",
				class_table[i].name[0]),
		      class_table[i].name[0], class_table[i].description);
	}
      w_println (wdesc, "</table>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (i = 0; i < top_class; i++)
	{
	  if (!str_cmp (class_table[i].name[0], wdesc->buf))
	    {
	      char buf3[MSL];
	      char skill_list[MAX_MORTAL_LEVEL + 1][MSL];
	      int snc, lev;
	      HelpData *pHelp;

	      print_header (wdesc, class_table[i].name[0]);
	      if ((pHelp = help_lookup (class_table[i].name[0])) != NULL)
		{
		  w_printlnf (wdesc,
			      "<table><tr><td>%s</td></tr></table>",
			      html_colorconv (pHelp->text));
		}
	      w_println (wdesc, "<table>");
	      for (lev = 0; lev < MAX_MORTAL_LEVEL + 1; lev++)
		skill_list[lev][0] = '\0';

	      for (snc = 0; snc < top_skill; snc++)
		{
		  if (skill_table[snc].name == NULL)
		    break;

		  if ((lev = skill_table[snc].skill_level[i]) <=
		      MAX_MORTAL_LEVEL)
		    {
		      sprintf (buf3, "%s, ", skill_table[snc].name);
		      if (NullStr (skill_list[lev]))
			sprintf (skill_list[lev],
				 "<tr><td>Level %d</td><td>%s", lev, buf3);
		      else
			strcat (skill_list[lev], buf3);
		    }
		}
	      for (lev = 0; lev < MAX_MORTAL_LEVEL + 1; lev++)
		{
		  if (skill_list[lev][0] != '\0')
		    {
		      if (strlen (skill_list[lev]) > 2)
			skill_list[lev][(strlen (skill_list[lev]) - 2)]
			  = '\0';

		      w_println (wdesc, skill_list[lev]);
		      w_println (wdesc, "</td></tr>");
		    }

		}
	      w_println (wdesc, "</table>");
	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleNotesRequest)
{
  int pos = 0;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Notes");
      w_println (wdesc, "<ul>");
      for (pos = 0; pos < MAX_BOARD; pos++)
	{
	  if (IsSet (boards[pos].flags, BOARD_NOWEB))
	    continue;

	  w_printlnf (wdesc,
		      "<li><a href='%s'>%s</a>",
		      HTTP_URL (NOTE_URL_PREFIX "/%s",
				boards[pos].short_name),
		      boards[pos].short_name);
	}
      w_println (wdesc, "</ul>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (pos = 0; pos < MAX_BOARD; pos++)
	{
	  if (IsSet (boards[pos].flags, BOARD_NOWEB))
	    continue;
	  if (!str_cmp (boards[pos].short_name, wdesc->buf))
	    {
	      BoardData *board = &boards[pos];
	      NoteData *pnote;
	      char buf2[MSL * 5];

	      sprintf (buf2, "Notes on %s Board", board->short_name);
	      print_header (wdesc, buf2);
	      for (pnote = board->note_first; pnote != NULL;
		   pnote = pnote->next)
		{
		  if (!is_name ("all", pnote->to_list))
		    continue;

		  w_printlnf (wdesc,
			      "<table><tr><td>From: %s</td><td>To: %s</td></tr><tr><td>Subject: %s</td>"
			      "<td>Date: %s</td></tr>",
			      pnote->sender, pnote->to_list,
			      pnote->subject, pnote->date);
		  w_printlnf (wdesc,
			      "<tr><td colspan=2><hr>%s<hr></td><tr></table><br>",
			      html_colorconv (pnote->text));
		}

	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleObjsRequest)
{
  ObjIndex *pObj;

  if (!NullStr (wdesc->buf))
    {
      if (!is_number (wdesc->buf))
	{
	  return false;
	}
      if ((pObj = get_obj_index (atov (wdesc->buf))) != NULL)
	{
	  print_header (wdesc, strip_color (pObj->short_descr));
	  w_printlnf (wdesc, "<p>%s<br>", html_colorconv (pObj->short_descr));
	  w_printlnf (wdesc, "%s<br>", html_colorconv (pObj->description));
	  w_printlnf (wdesc, "Material: %s<br>", pObj->material);
	  w_printlnf (wdesc, "Type: %s<br></p>",
		      flag_string (type_flags, pObj->item_type));
	  print_footer (wdesc);
	  return true;
	}
    }
  return false;
}

Web_Fun (HandleMobsRequest)
{
  CharIndex *pMob;

  if (!NullStr (wdesc->buf))
    {
      if (!is_number (wdesc->buf))
	{
	  return false;
	}
      if ((pMob = get_char_index (atov (wdesc->buf))) != NULL)
	{
	  print_header (wdesc, strip_color (pMob->short_descr));
	  w_printlnf (wdesc, "<p>%s<br>", html_colorconv (pMob->short_descr));
	  w_printlnf (wdesc, "%s<br>", html_colorconv (pMob->description));
	  w_printlnf (wdesc, "Race: %s<br>", pMob->race->name);
	  w_printlnf (wdesc, "Sex: %s<br>",
		      flag_string (sex_flags, pMob->sex));
	  w_printlnf (wdesc, "Alignment: %d<br>", pMob->alignment);
	  w_println (wdesc, "</p>");
	  print_footer (wdesc);
	  return true;
	}
    }
  return false;
}

Web_Fun (HandleRoomsRequest)
{
  RoomIndex *pRoom;
  vnum_t vnum;

  if (NullStr (wdesc->buf))
    {
      vnum = ROOM_VNUM_TEMPLE;
    }
  else
    {
      if (!is_number (wdesc->buf))
	{
	  return false;
	}
      vnum = atov (wdesc->buf);
    }
  if ((pRoom = get_room_index (vnum)) != NULL)
    {
      if (!IsSet (pRoom->area->area_flags, AREA_CLOSED | AREA_PLAYER_HOMES))
	{
	  ExitData *pexit;
	  int door;

	  print_header (wdesc, pRoom->name);
	  w_println (wdesc, "<table><tr><td colspan=3>");
	  w_println (wdesc, html_colorconv (pRoom->name));
	  w_printlnf (wdesc, "</td><td colspan=3>%s", pRoom->area->name);
	  w_println (wdesc, "</td></tr><tr><td colspan=6>");
	  w_println (wdesc, html_colorconv (pRoom->description));
	  w_println (wdesc, "</td></tr><tr>");
	  for (door = 0; door < MAX_DIR; door++)
	    {
	      if ((pexit = pRoom->exit[door]) != NULL
		  && pexit->u1.to_room != NULL)
		{
		  w_printlnf (wdesc,
			      "<td><a href='%s'>%s</a></td>",
			      HTTP_URL (ROOM_URL_PREFIX "/%ld",
					pexit->u1.to_room->vnum),
			      dir_name[door]);
		}
	      else
		w_println (wdesc, "<td></td>");
	    }
	  w_printlnf (wdesc, "</tr><tr><td colspan=6>%s</td>",
		      show_list_to_html (pRoom->content_first));
	  w_printlnf (wdesc, "</tr><tr><td colspan=6>%s</td>",
		      show_char_to_html (pRoom->person_first));

	  w_println (wdesc, "</tr></table>");
	  print_footer (wdesc);
	  return true;
	}
    }
  return false;
}

Web_Fun (HandleSocialsRequest)
{
  SocialData *iSocial;
  int i = 0;

  print_header (wdesc, "Socials");
  w_println (wdesc, "<table><tr>");
  for (iSocial = social_first; iSocial; iSocial = iSocial->next)
    w_printlnf (wdesc, "%s<td>%s</td>",
		i++ % 5 == 0 ? "</tr><tr>" : "", iSocial->name);
  w_printlnf (wdesc, "%s</table>", i % 5 != 0 ? "</tr>" : "");
  print_footer (wdesc);
  return true;
}

crs_t
crs_lookup (const char *name)
{
  switch (strswitch (name, "copyover", "reboot", "shutdown", NULL))
    {
    default:
      return CRS_NONE;
    case 1:
      return CRS_COPYOVER;
    case 2:
      return CRS_REBOOT;
    case 3:
      return CRS_SHUTDOWN;
    }
}

void
web_login_menu (WebDescriptor * wdesc, WebpassData * info)
{
  w_printlnf (wdesc,
	      "<hr><p>Welcome %s! You security level is %d.</p><ul><li><a href=\"%s\">Change password</a>",
	      info->name, info->level,
	      HTTP_URL (LOGIN_URL_PREFIX "/changepass"));
  w_println (wdesc, "</ul><hr>");
}

Web_Fun (HandleImmRequest)
{
  WebpassData *info;

  if ((info = get_name_password (wdesc, LEVEL_IMMORTAL)) != NULL)
    {
      if (NullStr (wdesc->buf))
	{
	  print_header (wdesc, "Immortal Info Page");
	  web_login_menu (wdesc, info);
	  w_printlnf (wdesc,
		      "<ul><li><a href='%s'>Skill Debug List</a></li>",
		      HTTP_URL (ADMIN_URL_PREFIX "/skdebug"));
	  w_printlnf (wdesc,
		      "<li><a href='%s'>Log Files</a></li>",
		      HTTP_URL (ADMIN_URL_PREFIX "/log"));
	  w_printlnf (wdesc,
		      "<li><a href=\"%s\">Immortal Help Files</a></li>",
		      HTTP_URL (ADMIN_URL_PREFIX "/ihelps"));
	  w_printlnf (wdesc, "<li><a href=\"%s\">Webserver Users</a></li>",
		      HTTP_URL (ADMIN_URL_PREFIX "/users"));
	  w_printlnf (wdesc,
		      "<li>Game Control<ul><li><a href=\"%s\">Copyover</a></li>",
		      HTTP_URL (ADMIN_URL_PREFIX "/copyover"));
	  w_printlnf (wdesc,
		      "<li><a href=\"%s\">Reboot</a></li><li><a href=\"%s\">Shutdown</a></li></ul>",
		      HTTP_URL (ADMIN_URL_PREFIX "/reboot"),
		      HTTP_URL (ADMIN_URL_PREFIX "/shutdown"));
	  w_println (wdesc, "<hr>");
	  print_footer (wdesc);
	  return true;
	}
      else
	{
	  crs_t stat;

	  if (!str_prefix ("skdebug", wdesc->buf))
	    {
	      int i, sn;

	      print_header (wdesc, "Skill List");
	      w_println (wdesc, "<table><tr><td>Skill Name</td>");
	      for (i = 0; i < top_class; i++)
		w_printlnf (wdesc, "<td>%s</td>", class_table[i].name[0]);
	      w_println (wdesc, "</tr>");
	      for (sn = 0; sn < top_skill; sn++)
		{
		  w_printlnf (wdesc, "<tr><td>%s</td>", skill_table[sn].name);
		  for (i = 0; i < top_class; i++)
		    {
		      if (skill_table[sn].skill_level[i] >= LEVEL_IMMORTAL)
			w_println (wdesc, "<td>---</td>");
		      else
			w_printlnf (wdesc, "<td>%d</td>",
				    skill_table[sn].skill_level[i]);
		    }
		  w_println (wdesc, "</tr>");
		}
	      w_println (wdesc, "</table>");
	      print_footer (wdesc);
	      return true;
	    }
	  else if (!str_prefix ("users", wdesc->buf))
	    {
	      WebpassData *pwd;

	      print_header (wdesc, "WebServer Users");
	      if (!wpwd_first)
		w_println (wdesc, "<p><B>No users at this time.</B></p>");
	      else
		{
		  w_printlnf
		    (wdesc, "<table><tr><th>Name</th><th>Level</th></tr>");
		  for (pwd = wpwd_first; pwd; pwd = pwd->next)
		    w_printlnf (wdesc,
				"<tr><td><a href=\"%s\">%s</a></td><td>%d</td></tr>",
				HTTP_URL (LOGIN_URL_PREFIX "/changepass"),
				pwd->name, pwd->level);
		  w_println (wdesc, "</table>");
		}
	      print_footer (wdesc);
	      return true;
	    }
	  else if (!str_prefix ("log", wdesc->buf))
	    {
	      char buf2[MSL];
	      struct dirent **Dir;
	      int o, i;

	      wdesc->buf = get_next (wdesc->buf, wdesc->baseurl);

	      if (NullStr (wdesc->buf))
		{
		  char buff[MSL];
		  int count = 0;

		  print_header (wdesc, "Log Files");
		  w_println (wdesc, "<table><tr>");
		  o = scandir (LOG_DIR, &Dir, 0, alphasort);
		  for (i = 0; i != o; i++)
		    {
		      if (!str_suffix (".log", Dir[i]->d_name))
			{
			  strcpy (buff, Dir[i]->d_name);
			  buff[strlen (buff) - 4] = '\0';
			  w_printlnf (wdesc,
				      "%s<td><a href='%s'>%s</a></td>",
				      count % 5 == 0 ? "</tr><tr>" : "",
				      HTTP_URL (ADMIN_URL_PREFIX "/log/%s",
						buff), Dir[i]->d_name);
			  count++;
			}
		    }
		  w_printlnf (wdesc, "%s</table>",
			      count % 5 != 0 ? "</tr>" : "");
		  print_footer (wdesc);
		  return true;
		}
	      else
		{
		  o = scandir (LOG_DIR, &Dir, 0, alphasort);
		  for (i = 0; i != o; i++)
		    {
		      if (!str_suffix (".log", Dir[i]->d_name))
			{
			  strcpy (buf2, Dir[i]->d_name);
			  buf2[strlen (buf2) - 4] = '\0';
			  if (!str_cmp (buf2, wdesc->buf))
			    {
			      print_header (wdesc, buf2);
			      w_println (wdesc, "<p class=\"pre\">");
			      sprintf (buf2, LOG_DIR "%s", Dir[i]->d_name);
			      print_file (wdesc, buf2);
			      w_println (wdesc, "</p>");
			      print_footer (wdesc);
			      return true;
			    }
			}
		    }
		}
	      return false;
	    }
	  else if (!str_prefix ("ihelps", wdesc->buf))
	    {
	      HelpData *pHelp;
	      int count = 0, check = 0;

	      print_header (wdesc, "Immortal Help Files");
	      w_println (wdesc, "<table>");
	      for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
		{
		  count++;
		  if (pHelp->level >= LEVEL_IMMORTAL || pHelp->level < 0)
		    {
		      const char *temp2;
		      char wordkey[MSL];

		      temp2 = pHelp->keyword;
		      while (!NullStr (temp2))
			{
			  wordkey[0] = NUL;
			  temp2 = one_argument (temp2, wordkey);
			  w_printlnf
			    (wdesc,
			     "%s<td><a href=\"%s\">%s</a></td>\n",
			     (check == 0) ? "<tr>" : "",
			     HTTP_URL (HELP_URL_PREFIX "/%d", count),
			     wordkey);
			  if (++check % 5 == 0)
			    {
			      w_print (wdesc, "</tr>");
			      check = 0;
			    }
			}
		    }
		}
	      w_println (wdesc, "</table>");
	      print_footer (wdesc);
	      return true;
	    }
	  else if ((stat = crs_lookup (wdesc->buf)) != CRS_NONE)
	    {
	      Proto (const char *crs_type_name, (crs_t));
	      ProtoNoR (void crs_fun, (crs_t));

	      meta_refresh = 5;
	      meta_location = HTTP_URL (NULL);
	      print_header (wdesc,
			    FORMATF ("Sending %s Request...",
				     crs_type_name (stat)));
	      w_printlnf (wdesc, "Sending %s Request now... please wait.",
			  crs_type_name (stat));
	      print_footer (wdesc);
	      crs_info.timer = 1;
	      crs_info.status = stat;
	      replace_str (&crs_info.who, "an admin web request");
	      crs_fun (stat);
	    }
	}
      return false;
    }
  else
    {
      print_header (wdesc, "Invalid Username/Password");
      w_printlnf (wdesc, "<p>You're Username and Password should be your,"
		  " Character name on the mud, and the password you set\n"
		  " with the 'webpass' command.  Each field is Case Sensitive.</p>");
      print_footer (wdesc);
      return true;
    }
}

Web_Fun (HandlePlayersRequest)
{
  WebpassData *info;

  if ((info = get_name_password (wdesc, 1)))
    {
      if (NullStr (wdesc->buf))
	{
	  print_header (wdesc, "Players Page");
	  w_printlnf
	    (wdesc,
	     "<blockquote>Welcome %s, I'm glad you were able to logon!"
	     "  In a continuing effort to bring players of %s"
	     " closer together, you"
	     " can now mark your place on our website.  Choose one of the options below."
	     "</blockquote>", info->name, mud_info.name);
	  web_login_menu (wdesc, info);
	  print_footer (wdesc);
	  return true;
	}
      else
	{

	  if (!str_prefix ("changepass", wdesc->buf))
	    {
	      WebpassData *wp;
	      char *opass, *npass, *cpass, *uname;

	      if (!str_cmp (wdesc->buf, "changepass")
		  || !str_cmp (wdesc->buf, "changepass/"))
		{
		  print_header (wdesc,
				FORMATF ("Change password for %s",
					 info->name));
		  w_printlnf (wdesc,
			      "<form method=\"get\" action=\"%s\">",
			      HTTP_URL (LOGIN_URL_PREFIX "/changepass/%s",
					info->name));
		  w_println (wdesc,
			     "<table><tr><td>Old Password</td><td><input maxLength=\"50\" size=\"35\" name=\"oldpwd\"></td></tr>");
		  w_println (wdesc,
			     "<tr><td>New Password</td><td><input maxLength=\"50\" size=\"35\" name=\"newpwd\"></td></tr>");
		  w_println (wdesc,
			     "<tr><td>Confirm Password</td><td><input maxLenght=\"50\" size=\"35\" name=\"confirmpwd\"></td></tr>");
		  w_println (wdesc,
			     "<tr><td><input type=\"submit\" value=\"Submit\" />");
		  w_println (wdesc,
			     "</td><td><input type=\"reset\" value=\"Clear Form\" />");
		  w_println (wdesc, "</td></tr></table>");
		  w_printlnf (wdesc,
			      "<input type=\"hidden\" name=\"player\" value=\"%s\">",
			      info->name);
		  w_println (wdesc, "</form>");
		  print_footer (wdesc);
		  return true;
		}

	      opass = substr (wdesc->buf, "oldpwd=", "&");
	      npass = substr (wdesc->buf, "newpwd=", "&");
	      cpass = substr (wdesc->buf, "confirmpwd=", "&");
	      uname = substr (wdesc->buf, "player=", "&");

	      if (NullStr (opass) || NullStr (npass) || NullStr (cpass)
		  || NullStr (uname))
		{
		  http_status (wdesc, 500);
		  send_content (wdesc, CONTENT_HTML);
		  w_println (wdesc, "Internal Server Error.");
		  return true;
		}

	      for (wp = wpwd_first; wp; wp = wp->next)
		{
		  if (!str_cmp (uname, wp->name))
		    {
		      Base64Decode (opass, (unsigned char *) wp->name, MIL);
		      Base64Decode (npass, (unsigned char *) wp->name, MIL);
		      Base64Decode (cpass, (unsigned char *) wp->name, MIL);
		      meta_refresh = 10;
		      meta_location =
			HTTP_URL (get_prev (wdesc->path, wdesc->baseurl));
		      if (info->level < MAX_LEVEL
			  && str_casecmp (wp->passw, crypt (opass, wp->name)))
			{
			  print_header (wdesc, "Old password doesn't match");
			  w_println (wdesc,
				     "The old password provided does not match.");
			  print_footer (wdesc);
			  return true;
			}
		      if (str_casecmp (npass, cpass))
			{
			  print_header (wdesc, "New passwords don't match");
			  w_println (wdesc,
				     "The new password does not match the confirmed password.");
			  print_footer (wdesc);
			  return true;
			}
		      print_header (wdesc, "Password changed");
		      w_printlnf (wdesc, "<p>Password changed for '%s'.",
				  wp->name);
		      print_footer (wdesc);
		      return true;
		    }
		}
	    }
	}
    }
  else
    {
      print_header (wdesc, "Invalid Username/Password");
      w_printlnf (wdesc, "<p>You're Username and Password should be your,"
		  " Character name on the mud, and the password you set\n"
		  " with the 'webpass' command.  Each field is Case Sensitive.</p>\n");
      print_footer (wdesc);
      return true;
    }
  return false;
}

Web_Fun (HandleHistoryRequest)
{
  int i;
  bool found = false;

  print_header (wdesc, "Channel History");
  w_println (wdesc, "<p class=\"pre\">");
  for (i = (www_index + 1) % 20; i != www_index; i = (i + 1) % 20)
    {
      if (!NullStr (www_history[i]))
	{
	  found = true;
	  w_printlnf (wdesc, "%s", strip_color (www_history[i]));
	}
    }
  if (!NullStr (www_history[www_index]))
    {
      w_printlnf (wdesc, "%s", strip_color (www_history[www_index]));
      found = true;
    }

  if (!found)
    w_println (wdesc, "None.");
  else
    w_printlnf (wdesc, LF "Current time: %s.",
		str_time (current_time, -1, "%I:%M:%S %p"));
  w_println (wdesc, "</p>");
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleIndexRequest)
{

  print_header (wdesc, NULL);

  w_println (wdesc,
	     "<img src=\"http://www.firstmud.com/images/firstmud_greet.jpg\" alt=\"Welcome to 1stMud\">");
  w_printlnf (wdesc, "<p>Current time is %s.</p>",
	      str_time (current_time, -1, NULL));
  w_printlnf (wdesc, "<p>%s started up at %s (%s ago).</p>", mud_info.name,
	      str_time (boot_time, -1, NULL),
	      timestr (current_time - boot_time, false));
  w_printlnf (wdesc,
	      "<p>The in game time is %d o'clock %s, Day of %s, %s the Month of %s, year %d.</p>",
	      (time_info.hour % (HOURS_IN_DAY / 2) ==
	       0) ? (HOURS_IN_DAY / 2) : time_info.hour % (HOURS_IN_DAY / 2),
	      time_info.hour >= (HOURS_IN_DAY / 2) ? "pm" : "am",
	      day_name[(time_info.day + 1) % DAYS_IN_WEEK],
	      ordinal_string (time_info.day + 1), month_name[time_info.month],
	      time_info.year);

  w_printlnf (wdesc,
	      "<p>Connect to %s: <a href='telnet://%s:%d/'>telnet://%s:%d/"
	      "</a></p>", mud_info.name, HOSTNAME, mainport,
	      HOSTNAME, mainport);

  if (crs_info.timer > -1 && crs_info.status != CRS_NONE)
    w_printlnf (wdesc, "<p>%s</p>", crs_sprintf (true, false));
  else if (mud_info.last_copyover)
    w_printlnf (wdesc, "<p>Last copyover was %s ago.</p>",
		timestr (current_time - mud_info.last_copyover, false));

  if (mud_info.longest_uptime)
    w_printlnf (wdesc, "<p>Longest uptime was %s.</p>",
		timestr (mud_info.longest_uptime, false));

  if (war_info.status == WAR_OFF)
    w_printlnf (wdesc, "<p>The next global war starts in %s.",
		intstr (war_info.timer, "minute"));
  else
    w_printlnf (wdesc,
		"<p>There is %s left in the %s war for levels %d to %d.</p>",
		intstr (war_info.timer, "minute"),
		wartype_name (war_info.wartype, false), war_info.min_level,
		war_info.max_level);

  if (gquest_info.running == GQUEST_OFF)
    w_printlnf (wdesc, "<p>The next global quest starts in %s.",
		intstr (gquest_info.timer, "minute"));
  else
    w_printlnf (wdesc,
		"<p>There is %s left in the global quest for levels %s to %d (%d targets).",
		intstr (gquest_info.timer, "minute"), gquest_info.minlevel,
		gquest_info.maxlevel, gquest_info.mob_count);

  if (auction_first)
    {
      AuctionData *auc, *auc_next;

      w_println (wdesc, "<p><h4>Auction - Current List of Inventory</h4>");
      w_println (wdesc,
		 "<table><th>Num</th><th>Seller</th><th>Buyer</th><th>Item Description                   Lvl</th><th>Last Bid</th><th>Time</th></tr>");

      for (auc = auction_first; auc; auc = auc_next)
	{
	  auc_next = auc->next;

	  if (!auc->item)
	    {
	      reset_auc (auc, true);
	      continue;
	    }

	  w_printlnf (wdesc,
		      "<tr><td>%d</td><td>- %s</td><td>%s</td><td>%s</td><td>%d</td><td>%ld%s</td><td>%s</td></tr>",
		      auc->number, GetName (auc->owner),
		      auc->high_bidder ? GetName (auc->high_bidder) : "None",
		      auc->number, html_colorconv (auc->item->short_descr),
		      auc->item->level, auc->bid, auc_type (auc->type, true),
		      format_pulse (auc->status));
	}
      w_println (wdesc, "</table></p>");
    }
  w_printlnf (wdesc,
	      "<p>Visit the " MUDSTRING " Homepage at "
	      "<a href=\"http://www.firstmud.com/\">http://www.firstmud.com/</a>.</p>");
  w_printlnf (wdesc,
	      "<p>%s Admin email: <a href=\"mailto:%s@%s?%s Homepage\">%s@%s</a></p>",
	      mud_info.name, UNAME, HOSTNAME, mud_info.name, UNAME, HOSTNAME);
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleUnknownRequest)
{
  http_error_code = 400;
  print_header (wdesc, "Error");
  w_printlnf (wdesc, "<p>Unknown url '%s'.</p>", wdesc->path);
  w_printf
    (wdesc,
     "<p>For a list of url's this web server handles see the <a href=\"%s\">Index</a>.</p>\n",
     HTTP_URL (NULL));
  print_footer (wdesc);
  return true;
}

Web_Fun (HandleNameGenRequest)
{
  NameProfile *nl;
  int count;

  if (!name_profile_first)
    {
      print_header (wdesc, "Name Generator");
      w_println (wdesc,
		 "<p>Random name generation is currently unavailable.</p>");
      print_footer (wdesc);
      return true;
    }

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Name Generator");
      w_println (wdesc, "<p>Pick a name profile:</p><ul>");
      for (count = 0, nl = name_profile_first; nl; nl = nl->next, count++)
	{
	  w_printlnf (wdesc, "<li><a href=\"%s\">%s</a>",
		      HTTP_URL (NAMEGEN_URL_PREFIX "/%d", count), nl->title);
	}
      w_printlnf (wdesc, "</ul>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (count = 0, nl = name_profile_first; nl; nl = nl->next, count++)
	{
	  if (!str_cmp (FORMATF ("%d", count), wdesc->buf))
	    {
	      int i;

	      print_header (wdesc,
			    FORMATF ("%s Names", capitalize (nl->title)));
	      w_println (wdesc, "<table><tr><td><ul>");
	      for (i = 0; i < 20; i++)
		w_printlnf (wdesc, "<li>%s</li>", capitalize (genname (nl)));
	      w_println (wdesc, "</ul></td><td><ul>");
	      for (i = 0; i < 20; i++)
		w_printlnf (wdesc, "<li>%s</li>", capitalize (genname (nl)));
	      w_println (wdesc, "</ul></td><td><ul>");
	      for (i = 0; i < 20; i++)
		w_printlnf (wdesc, "<li>%s</li>", capitalize (genname (nl)));
	      w_println (wdesc, "</ul></td></tr></table>");
	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

Web_Fun (HandleLyricsRequest)
{
  int i;

  if (NullStr (wdesc->buf))
    {
      print_header (wdesc, "Lyrics");
      w_println (wdesc, "<p>Please choose a song:</p><ul>");
      for (i = 0; i < top_song; i++)
	{
	  w_printlnf (wdesc, "<li><a href=\"%s\">%s</a> by %s</li>",
		      HTTP_URL (SONG_URL_PREFIX "/%d", i), song_table[i].name,
		      song_table[i].group);
	}
      w_println (wdesc, "</ul>");
      print_footer (wdesc);
      return true;
    }
  else
    {
      for (i = 0; i < top_song; i++)
	{
	  if (!str_cmp (FORMATF ("%d", i), wdesc->buf))
	    {
	      int j;

	      print_header (wdesc, FORMATF ("Lyrics for %s by %s",
					    song_table[i].name,
					    song_table[i].group));
	      w_printlnf (wdesc, "<p class=\"pre\">");
	      for (j = 0; j < song_table[i].lines; j++)
		w_println (wdesc, song_table[i].lyrics[j]);
	      w_println (wdesc, "</p>");
	      print_footer (wdesc);
	      return true;
	    }
	}
    }
  return false;
}

#ifndef DISABLE_I3
Web_Fun (HandleI3Request)
{
  I3_MUD *mud;
  int mudcount;

  print_header (wdesc, "InterMUD3");
  if (i3mud_first == NULL)
    {
      w_println (wdesc, "<p>There are no muds to list!?</p>");
      print_footer (wdesc);
      return true;
    }
  w_println (wdesc,
	     "<table><tr><th>Name</th><th>Type</th><th>Mudlib</th><th>Address</th><th>Port</th></tr>");
  for (mudcount = 0, mud = i3mud_first; mud; mud = mud->next)
    {
      if (mud == NULL)
	{
	  bug ("I3_mudlist: NULL mud found in listing!");
	  continue;
	}

      if (mud->name == NULL)
	{
	  bug ("I3_mudlist: NULL mud name found in listing!");
	  continue;
	}

      if (mud->status == 0)
	continue;

      mudcount++;

      switch (mud->status)
	{
	case -1:
	  if (NullStr (mud->web))
	    w_printlnf (wdesc,
			"<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%d</td></tr>",
			mud->name, mud->mud_type, mud->mudlib,
			mud->ipaddress, mud->player_port);
	  else
	    w_printlnf (wdesc,
			"<tr><td><a href=\"%s\">%s</a></td><td>%s</td><td>%s</td><td>%s</td><td>%d</td></tr>",
			mud->web, mud->name, mud->mud_type, mud->mudlib,
			mud->ipaddress, mud->player_port);
	  break;
	case 0:
	  w_printlnf (wdesc, "<tr><td colspan=\"5\">%s (down)</td></tr>",
		      mud->name);
	  break;
	default:
	  w_printlnf (wdesc,
		      "<tr><td colspan=\"5\">%s (rebooting, back in %d seconds)</td></tr>",
		      mud->name, mud->status);
	  break;
	}
    }
  w_printlnf (wdesc,
	      "<tr><td colspan=\"5\"><hr>%d total muds listed.</td></tr></table>",
	      mudcount);
  print_footer (wdesc);
  return true;
}
#endif

bool
web_is_connected (void)
{
  return (web_control != INVALID_SOCKET);
}



void
update_web_server (void)
{
  SOCKET max_fd;
  WebDescriptor *current, *next;
  fd_set readfds;
  static struct timeval ZERO_TIME = {
    0, 0
  };


  const char ENDREQUEST[5] = {
    13, 10, 13, 10, 0
  }
  ;

  if (!web_is_connected ())
    return;

  FD_ZERO (&readfds);
  FD_SET (web_control, &readfds);


  max_fd = web_control;


  for (current = webdesc_first; current; current = current->next)
    {
      FD_SET (current->fd, &readfds);
      if (max_fd < current->fd)
	max_fd = current->fd;
    }


  select (max_fd + 1, &readfds, NULL, NULL, &ZERO_TIME);

  if (FD_ISSET (web_control, &readfds))
    {

      socklen_t size;
      struct sockaddr_in sock;

      current = new_webdesc ();
      size = sizeof (sock);

      current->request[0] = '\0';

      if ((current->fd =
	   accept (web_control, (SOCKADDR *) & sock,
		   &size)) == INVALID_SOCKET)
	{
	  socket_error ("web-accept");
	  free_webdesc (current);
	  FD_CLR (web_control, &readfds);
	  return;
	}

      current->their_addr = ntohl (sock.sin_addr.s_addr);

      Link (current, webdesc, next, prev);

    }


  for (current = webdesc_first; current; current = current->next)
    {
      if (FD_ISSET (current->fd, &readfds))
	{
	  char buf[MIL];
	  int numbytes;

	  if ((numbytes = recv (current->fd, buf, sizeof (buf), 0)) == -1)
	    {
	      socket_error ("web-read");
	      continue;
	    }

	  buf[numbytes] = '\0';

	  strcat (current->request, buf);
	}
    }



  for (current = webdesc_first; current; current = next)
    {
      next = current->next;

      if (strstr (current->request, "HTTP/1.")
	  && strstr (current->request, ENDREQUEST))
	handle_web_request (current);
      else if (!strstr (current->request, "HTTP/1.")
	       && strchr (current->request, '\n'))
	handle_web_request (current);
      else
	{
	  continue;
	}

      closesocket (current->fd);
      UnLink (current, webdesc, next, prev);
      free_webdesc (current);
    }

}

const struct request_type request_table[] = {
  {"unknown", NULL, HandleUnknownRequest, false, false},
  {"index", "Home", HandleIndexRequest, false, false},
  {"online", "Online", HandleWhoRequest, false, false},
  {"tech", "Tech", HandleMemoryRequest, false, false},
  {"areas", "Areas", HandleAreaRequest, false, false},
  {"spells", "Spells", HandleSpellsRequest, false, false},
  {"rules", "Rules", HandleRulesRequest, false, false},
  {"credits", "Credits", HandleCreditsRequest, false, false},
  {"socials", "Socials", HandleSocialsRequest, false, false},
  {HELP_URL_PREFIX, "Helps", HandleHelpsRequest, false, true},
  {NOTE_URL_PREFIX, "Notes", HandleNotesRequest, false, true},
  {RACE_URL_PREFIX, "Races", HandleRaceRequest, false, false},
  {DEITY_URL_PREFIX, "Deities", HandleDeityRequest, false, false},
  {CMD_URL_PREFIX, "Commands", HandleCommandsRequest, false, true},
  {CLAN_URL_PREFIX, "Clans", HandleClanRequest, false, false},
  {CLASS_URL_PREFIX, "Classes", HandleClassRequest, false, true},
  {"history", "History", HandleHistoryRequest, false, false},
  {STAT_URL_PREFIX, "Stats", HandleStatsRequest, false, true},
  {ROOM_URL_PREFIX, "Explore", HandleRoomsRequest, false, true},
  {NAMEGEN_URL_PREFIX, "Name Generator", HandleNameGenRequest, false, true},
  {SONG_URL_PREFIX, "Song Lyrics", HandleLyricsRequest, false, true},
#ifndef DISABLE_I3
  {"i3", "InterMUD3", HandleI3Request, false, true},
#endif
  {OBJ_URL_PREFIX, NULL, HandleObjsRequest, false, true},
  {CHAR_URL_PREFIX, NULL, HandleMobsRequest, false, true},
  {ADMIN_URL_PREFIX, "Admin", HandleImmRequest, true, true},
  {LOGIN_URL_PREFIX, "Login", HandlePlayersRequest, true, true},
  {NULL, NULL, NULL, false, false}
};


void
format_req (WebDescriptor * wdesc)
{
  char temp[MSL];

  wdesc->stuff = str_dup (first_arg (wdesc->request, temp, false));
  first_arg (wdesc->stuff, temp, false);

  if (temp[0] == '/')
    wdesc->path = str_dup (&temp[1]);
  else
    wdesc->path = str_dup (temp);

  wdesc->buf = get_next (wdesc->path, wdesc->baseurl);

  return;
}

void
handle_web_request (WebDescriptor * wdesc)
{
  int i;



  if (!strstr (wdesc->request, "GET"))
    {
      http_status (wdesc, 501);
      send_content (wdesc, CONTENT_HTML);
      w_println (wdesc, "Not Implemented");
      return;
    }

  format_req (wdesc);

  if (NullStr (wdesc->path))
    {
      HandleIndexRequest (wdesc);
      mud_info.stats.web_requests++;
      return;
    }

  for (i = 0; request_table[i].fun != NULL; i++)
    {
      if (request_table[i].secure && !strstr (wdesc->request, "HTTP/1."))
	break;

      if (!str_prefix (request_table[i].req, wdesc->path))
	{
	  if ((*request_table[i].fun) (wdesc))
	    {
	      logf ("WebServer: '%s' requested.", wdesc->path);
	      mud_info.stats.web_requests++;
	      return;
	    }
	  else
	    break;
	}
    }

  logf ("WebServer: unknown request '%s'.", wdesc->path);
  HandleUnknownRequest (wdesc);
}

void
shutdown_web_server (void)
{
  WebDescriptor *current, *next;

  if (!web_is_connected ())
    return;

#ifdef WIN32

  if (crs_info.status == CRS_COPYOVER)
    return;
#endif


  for (current = webdesc_first; current; current = next)
    {
      next = current->next;
      closesocket (current->fd);
      UnLink (current, webdesc, next, prev);
      free_webdesc (current);
    }


  log_string ("Closing webserver...");
  closesocket (web_control);
  web_control = INVALID_SOCKET;
}

#endif
