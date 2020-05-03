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
* Copyright (c) 2000 Fatal Dimensions                                     *
*                                                                         *
* See the file "LICENSE" or information on usage and redistribution       *
* of this file, and for a DISCLAIMER OF ALL WARRANTIES.                   *
***************************************************************************
* Ported to Smaug 1.4a by Samson of Alsherok. Consolidated for            *
* cross-codebase compatibility by Samson of Alsherok. Modifications and   *
* enhancements to the code Copyright (c) 2001-2003 Roger Libiez           *
* ( Samson ) Registered with the United States Copyright Office           *
* TX 5-562-404.  I condensed the 14 or so Fatal Dimensions source code    *
* files into this one file, because I for one find it far easier to       *
* maintain when all of the functions are right here in one file.          *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "tables.h"
#include "data_table.h"
#include "olc.h"
#include "recycle.h"
#include "vnums.h"

#ifndef DISABLE_I3


char I3_input_buffer[IPS];
char I3_output_buffer[OPS];
char I3_currentpacket[IPS];
bool packetdebug = false;
long I3_input_pointer = 0;
long I3_output_pointer = 4;
const char *I3_THISMUD;
const char *I3_ROUTER_NAME;
const char *manual_router;
SOCKET I3_control = INVALID_SOCKET;
int i3wait;
int i3timeout;
time_t ucache_clock;
long bytes_received;
long bytes_sent;

I3_MUD i3mud_info;

Proto (I3_HEADER * I3_get_header, (char **pps));
Proto (void I3_send_channel_listen, (I3_CHANNEL *, bool));

Proto (bool i3check_permissions, (CharData *, int, int, bool));
Proto (const char *i3_funcname, (Do_F *));

Proto (Do_F * i3_function, (const char *));

char *
rankbuffer (CharData * ch)
{
  static char rbuf[MSL];

  if (ch->pcdata->i3chardata->i3perm >= I3PERM_IMM)
    {
      strlcpy (rbuf, "{YStaff", MSL);

      if (!NullStr (CH_I3RANK (ch)))
	snprintf (rbuf, sizeof (rbuf), "{Y%s", CH_I3RANK (ch));
    }
  else
    {
      strlcpy (rbuf, "{BPlayer", MSL);

      if (!NullStr (CH_I3RANK (ch)))
	snprintf (rbuf, sizeof (rbuf), "{B%s", CH_I3RANK (ch));
    }
  return rbuf;
}

FlagTable i3perm_names[] = {
  {"Notset", I3PERM_NOTSET, true}
  ,
  {"None", I3PERM_NONE, true}
  ,
  {"Mort", I3PERM_MORT, true}
  ,
  {"Imm", I3PERM_IMM, true}
  ,
  {"Admin", I3PERM_ADMIN, true}
  ,
  {"Imp", I3PERM_IMP, true}
  ,
  {NULL, 0, false}
};

int
get_permvalue (const char *flag)
{
  unsigned int x;

  for (x = 0; i3perm_names[x].name != NULL; x++)
    if (!str_cmp (flag, i3perm_names[x].name))
      return x;

  return -1;
}



char *
i3strrep (const char *src, const char *sch, const char *rep)
{
  int lensrc = strlen (src), lensch = strlen (sch), lenrep =
    strlen (rep), x, y, in_p;
  static char newsrc[MSL];
  bool searching = false;

  newsrc[0] = '\0';
  for (x = 0, in_p = 0; x < lensrc; x++, in_p++)
    {
      if (src[x] == sch[0])
	{
	  searching = true;
	  for (y = 0; y < lensch; y++)
	    if (src[x + y] != sch[y])
	      searching = false;

	  if (searching)
	    {
	      for (y = 0; y < lenrep; y++, in_p++)
		{
		  if (in_p == (MSL - 1))
		    {
		      newsrc[in_p] = '\0';
		      return newsrc;
		    }
		  if (src[x - 1] == sch[0])
		    {
		      if (rep[0] == '\033')
			{
			  if (y < lensch)
			    {
			      if (y == 0)
				newsrc[in_p - 1] = sch[y];
			      else
				newsrc[in_p] = sch[y];
			    }
			  else
			    y = lenrep;
			}
		      else
			{
			  if (y == 0)
			    newsrc[in_p - 1] = rep[y];
			  newsrc[in_p] = rep[y];
			}
		    }
		  else
		    newsrc[in_p] = rep[y];
		}
	      x += lensch - 1;
	      in_p--;
	      searching = false;
	      continue;
	    }
	}
      if (in_p == (MSL - 1))
	{
	  newsrc[in_p] = '\0';
	  return newsrc;
	}
      newsrc[in_p] = src[x];
    }
  newsrc[in_p] = '\0';
  return newsrc;
}

char *
i3_strip_colors (const char *txt)
{
  I3_COLOR *color;
  static char tbuf[MSL];

  strlcpy (tbuf, txt, MSL);

  for (color = i3color_first; color; color = color->next)
    strlcpy (tbuf, i3strrep (tbuf, color->i3fish, ""), MSL);

  for (color = i3color_first; color; color = color->next)
    strlcpy (tbuf, i3strrep (tbuf, color->i3tag, ""), MSL);

  for (color = i3color_first; color; color = color->next)
    strlcpy (tbuf, i3strrep (tbuf, color->mudtag, ""), MSL);

  return tbuf;
}

char *
I3_mudtofish (const char *txt)
{
  I3_COLOR *color;
  static char tbuf[MSL];

  if (!txt || *txt == '\0')
    return "";

  strlcpy (tbuf, txt, MSL);
  for (color = i3color_first; color; color = color->next)
    strlcpy (tbuf, i3strrep (tbuf, color->mudtag, color->i3fish), MSL);

  return tbuf;
}

char *
I3_codetofish (const char *txt)
{
  I3_COLOR *color;
  static char tbuf[MSL];

  if (!txt || *txt == '\0')
    return "";

  strlcpy (tbuf, txt, MSL);
  for (color = i3color_first; color; color = color->next)
    strlcpy (tbuf, i3strrep (tbuf, color->i3tag, color->i3fish), MSL);

  return tbuf;
}

char *
I3_codetomud (CharData * ch, const char *txt)
{
  I3_COLOR *color;
  static char tbuf[MSL];

  if (!txt || *txt == '\0')
    return "";

  if (!ch || IsSet (ch->pcdata->i3chardata->i3flags, I3_COLORFLAG))
    {
      strlcpy (tbuf, txt, MSL);
      for (color = i3color_first; color; color = color->next)
	strlcpy (tbuf, i3strrep (tbuf, color->i3tag, color->mudtag), MSL);
    }
  else
    strlcpy (tbuf, i3_strip_colors (txt), MSL);

  return tbuf;
}

char *
I3_fishtomud (CharData * ch, const char *txt)
{
  I3_COLOR *color;
  static char tbuf[MSL];

  if (NullStr (txt))
    return "";

  if (!ch || IsSet (ch->pcdata->i3chardata->i3flags, I3_COLORFLAG))
    {
      strlcpy (tbuf, txt, MSL);
      for (color = i3color_first; color; color = color->next)
	strlcpy (tbuf, i3strrep (tbuf, color->i3fish, color->mudtag), MSL);
    }
  else
    strlcpy (tbuf, i3_strip_colors (txt), MSL);

  return tbuf;
}


int
i3print (CharData * ch, const char *txt)
{
  char buf[MPL * 3];

  if (!ch || NullStr (txt) || !ch->desc)
    {
      return 0;
    }

  snprintf (buf, sizeof (buf), "%s{x", I3_fishtomud (ch, txt));
  return d_print (ch->desc, buf);
}

int
i3printf (CharData * ch, const char *fmt, ...)
{
  va_list args;
  char buf[MPL * 3];

  if (!ch || NullStr (fmt) || !ch->desc)
    return 0;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return i3print (ch, buf);
}

int
i3println (CharData * ch, const char *txt)
{
  if (!ch || !ch->desc)
    return 0;

  return i3print (ch, txt) + d_print (ch->desc, NEWLINE);
}

int
i3printlnf (CharData * ch, const char *fmt, ...)
{
  va_list args;
  char buf[MPL * 3];

  if (!ch || !ch->desc)
    return 0;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return i3println (ch, buf);
}

int
i3bprint (Buffer * b, const char *txt)
{
  char buf[MPL * 3];

  if (!b || NullStr (txt))
    return 0;

  snprintf (buf, sizeof (buf), I3_fishtomud (NULL, txt));
  return bprint (b, buf);
}

int
i3bprintf (Buffer * b, const char *fmt, ...)
{
  va_list args;
  char buf[MPL * 3];

  if (!b || NullStr (fmt))
    return 0;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return i3bprint (b, buf);
}

int
i3bprintln (Buffer * b, const char *txt)
{
  if (!b)
    return 0;

  return i3bprint (b, txt) + bprint (b, NEWLINE);
}

int
i3bprintlnf (Buffer * b, const char *fmt, ...)
{
  va_list args;
  char buf[MPL * 3];

  if (!b)
    return 0;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return i3bprintln (b, buf);
}

bool
verify_i3layout (const char *fmt, int number)
{
  const char *c;
  int i = 0;

  c = fmt;
  while ((c = strchr (c, '%')) != NULL)
    {
      if (*(c + 1) == '%')
	{
	  c += 2;
	  continue;
	}

      if (*(c + 1) != 's')
	return false;

      c++;
      i++;
    }

  if (i != number)
    return false;

  return true;
}


bool
I3_is_connected (void)
{
  return (I3_control != INVALID_SOCKET);
}


char *
I3_escape (const char *ps)
{
  static char xnew[MSL];
  char *pnew = xnew;

  while (ps[0])
    {
      if (ps[0] == '"')
	{
	  pnew[0] = '\\';
	  pnew++;
	}
      if (ps[0] == '\\')
	{
	  pnew[0] = '\\';
	  pnew++;
	}
      pnew[0] = ps[0];
      pnew++;
      ps++;
    }
  pnew[0] = '\0';
  return xnew;
}


void
I3_remove_quotes (char **ps)
{
  char *ps1, *ps2;

  if (*ps[0] == '"')
    (*ps)++;
  if ((*ps)[strlen (*ps) - 1] == '"')
    (*ps)[strlen (*ps) - 1] = 0;

  ps1 = ps2 = *ps;
  while (ps2[0])
    {
      if (ps2[0] == '\\')
	{
	  ps2++;
	}
      ps1[0] = ps2[0];
      ps1++;
      ps2++;
    }
  ps1[0] = '\0';
}


I3_CHANNEL *
find_I3_channel_by_localname (const char *name)
{
  I3_CHANNEL *channel = NULL;

  for (channel = I3chan_first; channel; channel = channel->next)
    {
      if (!channel->local_name)
	continue;

      if (!str_cmp (channel->local_name, name))
	return channel;
    }
  return NULL;
}


I3_CHANNEL *
find_I3_channel_by_name (const char *name)
{
  I3_CHANNEL *channel = NULL;

  for (channel = I3chan_first; channel; channel = channel->next)
    {
      if (!str_cmp (channel->I3_name, name))
	return channel;
    }
  return NULL;
}


I3_CHANNEL *
new_I3_channel (void)
{
  I3_CHANNEL *cnew;

  alloc_mem (cnew, I3_CHANNEL, 1);
  return cnew;
}


void
destroy_I3_channel (I3_CHANNEL * channel)
{
  int x;

  if (channel == NULL)
    {
      bug ("destroy_I3_channel: Null parameter");
      return;
    }

  free_string (channel->local_name);
  free_string (channel->host_mud);
  free_string (channel->I3_name);
  free_string (channel->layout_m);
  free_string (channel->layout_e);

  for (x = 0; x < MAX_I3HISTORY; x++)
    {
      if (channel->history[x] && channel->history[x] != '\0')
	free_string (channel->history[x]);
    }

  UnLink (channel, I3chan, next, prev);
  free_mem (channel);
}


I3_MUD *
find_I3_mud_by_name (const char *name)
{
  I3_MUD *mud;

  for (mud = i3mud_first; mud; mud = mud->next)
    {
      if (!str_cmp (mud->name, name))
	return mud;
    }
  return NULL;
}

I3_MUD *
new_I3_mud (const char *name)
{
  I3_MUD *cnew, *mud_prev;

  alloc_mem (cnew, I3_MUD, 1);
  cnew->name = str_dup (name);

  for (mud_prev = i3mud_first; mud_prev; mud_prev = mud_prev->next)
    if (str_cmp (mud_prev->name, name) >= 0)
      break;

  if (!mud_prev)
    Link (cnew, i3mud, next, prev);
  else
    Insert (cnew, mud_prev, i3mud, next, prev);

  return cnew;
}

void
destroy_I3_mud (I3_MUD * mud)
{
  if (mud == NULL)
    {
      bug ("destroy_I3_mud: Null parameter");
      return;
    }

  free_string (mud->name);
  free_string (mud->ipaddress);
  free_string (mud->mudlib);
  free_string (mud->base_mudlib);
  free_string (mud->driver);
  free_string (mud->mud_type);
  free_string (mud->open_status);
  free_string (mud->admin_email);
  free_string (mud->telnet);
  free_string (mud->web_wrong);
  free_string (mud->banner);
  free_string (mud->web);
  free_string (mud->time);
  free_string (mud->daemon);
  if (mud != &i3mud_info)
    {
      UnLink (mud, i3mud, next, prev);
      free_mem (mud);
    }
}


void
I3_connection_close (bool reconnect)
{
  ROUTER_DATA *router = NULL;
  bool rfound = false;

  for (router = router_first; router; router = router->next)
    if (!str_cmp (router->name, I3_ROUTER_NAME))
      {
	rfound = true;
	break;
      }

  if (!rfound)
    {
      socket_error ("Disconnecting from router.");
      closesocket (I3_control);
      I3_control = INVALID_SOCKET;
      return;
    }

  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
	      "Closing connection to Intermud-3 router %s", router->name);
  closesocket (I3_control);
  I3_control = INVALID_SOCKET;
  if (reconnect)
    {
      if (router->reconattempts <= 3)
	{
	  i3wait = 100;
	  wiznet
	    ("Will attempt to reconnect in approximately 15 seconds.",
	     NULL, NULL, WIZ_I3, true, 1);
	}
      else if (router->next != NULL)
	{
	  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
		      "Unable to reach %s. Abandoning connection.",
		      router->name);
	  logf ("Bytes sent: %ld. Bytes received: %ld.", bytes_sent,
		bytes_received);
	  bytes_sent = 0;
	  bytes_received = 0;
	  i3wait = 100;
	  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
		      "Will attempt new connection to %s in approximately 15 seconds.",
		      router->next->name);
	}
      else
	{
	  bytes_sent = 0;
	  bytes_received = 0;
	  i3wait = -2;
	  wiznet ("Unable to reconnect. No routers responding.", NULL,
		  NULL, WIZ_I3, true, 1);
	  ;
	  return;
	}
    }
  logf ("Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received);
  bytes_sent = 0;
  bytes_received = 0;
  return;
}


void
I3_write_buffer (const char *msg)
{
  long newsize = I3_output_pointer + strlen (msg);

  if (newsize > OPS - 1)
    {
      bugf ("I3_write_buffer: buffer too large (would become %ld)", newsize);
      return;
    }
  strlcpy (I3_output_buffer + I3_output_pointer, msg, newsize);
  I3_output_pointer = newsize;
}


void
send_to_i3 (const char *text)
{
  char buf[MSL * 3];

  snprintf (buf, sizeof (buf), "%s", I3_codetofish (text));
  I3_write_buffer (buf);
}


void
I3_write_header (const char *identifier,
		 const char *originator_mudname,
		 const char *originator_username,
		 const char *target_mudname, const char *target_username)
{
  I3_write_buffer ("({\"");
  I3_write_buffer (identifier);
  I3_write_buffer ("\",5,");
  if (originator_mudname)
    {
      I3_write_buffer ("\"");
      I3_write_buffer (originator_mudname);
      I3_write_buffer ("\",");
    }
  else
    I3_write_buffer ("0,");

  if (originator_username)
    {
      I3_write_buffer ("\"");
      I3_write_buffer (originator_username);
      I3_write_buffer ("\",");
    }
  else
    I3_write_buffer ("0,");

  if (target_mudname)
    {
      I3_write_buffer ("\"");
      I3_write_buffer (target_mudname);
      I3_write_buffer ("\",");
    }
  else
    I3_write_buffer ("0,");

  if (target_username)
    {
      I3_write_buffer ("\"");
      I3_write_buffer (target_username);
      I3_write_buffer ("\",");
    }
  else
    I3_write_buffer ("0,");
}


char *
I3_get_field (char *packet, char **ps)
{
  int count[256];
  char has_apostrophe = 0, has_backslash = 0;
  char foundit = 0;

  memset (count, 0, sizeof (count));

  *ps = packet;
  while (1)
    {
      switch (*ps[0])
	{
	case '{':
	  if (!has_apostrophe)
	    count[(int) '{']++;
	  break;
	case '}':
	  if (!has_apostrophe)
	    count[(int) '}']++;
	  break;
	case '[':
	  if (!has_apostrophe)
	    count[(int) '[']++;
	  break;
	case ']':
	  if (!has_apostrophe)
	    count[(int) ']']++;
	  break;
	case '(':
	  if (!has_apostrophe)
	    count[(int) '(']++;
	  break;
	case ')':
	  if (!has_apostrophe)
	    count[(int) ')']++;
	  break;
	case '\\':
	  if (has_backslash)
	    has_backslash = 0;
	  else
	    has_backslash = 1;
	  break;
	case '"':
	  if (has_backslash)
	    {
	      has_backslash = 0;
	    }
	  else
	    {
	      if (has_apostrophe)
		has_apostrophe = 0;
	      else
		has_apostrophe = 1;
	    }
	  break;
	case ',':
	case ':':
	  if (has_apostrophe)
	    break;
	  if (has_backslash)
	    break;
	  if (count[(int) '{'] != count[(int) '}'])
	    break;
	  if (count[(int) '['] != count[(int) ']'])
	    break;
	  if (count[(int) '('] != count[(int) ')'])
	    break;
	  foundit = 1;
	  break;
	}
      if (foundit)
	break;
      (*ps)++;
    }
  *ps[0] = '\0';
  (*ps)++;
  return *ps;
}


CharData *
I3_find_user (const char *name)
{
  Descriptor *d;

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->character && !str_cmp (GetName (d->character), name))
	return d->character;
    }
  return NULL;
}


bool
i3ignoring (CharData * ch, const char *ignore)
{
  I3_IGNORE *temp;
  I3_MUD *mud;
  char *ps;
  char ipbuf[512], mudname[MIL];


  for (temp = ch->pcdata->i3chardata->i3ignore_first; temp; temp = temp->next)
    {
      if (!fnmatch (temp->name, ignore, 0))
	return true;
    }


  ps = strchr (ignore, '@');

  if (NullStr (ignore) || ps == NULL)
    return false;

  ps[0] = '\0';
  strlcpy (mudname, ps + 1, MIL);

  for (mud = i3mud_first; mud; mud = mud->next)
    {
      if (!str_cmp (mud->name, mudname))
	{
	  snprintf (ipbuf, sizeof (ipbuf), "%s:%d", mud->ipaddress,
		    mud->player_port);
	  for (temp = ch->pcdata->i3chardata->i3ignore_first; temp;
	       temp = temp->next)
	    {
	      if (!str_cmp (temp->name, ipbuf))
		return true;
	    }
	}
    }
  return false;
}


bool
I3_write_packet (char *msg)
{
  int oldsize, size, check, x;
  char *s = I3_output_buffer;

  oldsize = size = strlen (msg + 4);
  s[3] = size % 256;
  size >>= 8;
  s[2] = size % 256;
  size >>= 8;
  s[1] = size % 256;
  size >>= 8;
  s[0] = size % 256;


  for (x = 0; x < oldsize + 4; x++)
    {
      if (msg[x] == '\r' && x > 3)
	msg[x] = '\0';
    }
  check = send (I3_control, msg, oldsize + 4, 0);

  if (!check || (check < 0 && !check_errno (EWOULDBLOCK)
#ifndef WIN32
		 && !check_errno (EAGAIN)
#endif
      ))
    {
      if (check < 0)
	socket_error ("Write error on socket.");
      else
	socket_error ("EOF encountered on socket write.");
      I3_connection_close (true);
      return false;
    }

  if (check < 0)
    return true;

  bytes_sent += check;
  if (packetdebug)
    {
      logf ("Size: %d. Bytes Sent: %d.", oldsize, check);
      logf ("Packet Sent: %s", msg + 4);
    }
  I3_output_pointer = 4;
  return true;
}

void
I3_send_packet (void)
{
  I3_write_packet (I3_output_buffer);
  return;
}


void
I3_startup_packet (void)
{
  char s[MIL];
  char *strtime;

  if (!I3_is_connected ())
    return;

  I3_output_pointer = 4;
  I3_output_buffer[0] = '\0';

  new_wiznet (NULL, NULL, WIZ_I3, true, 1, "Sending startup_packet to %s",
	      I3_ROUTER_NAME);

  I3_write_header ("startup-req-3", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL);

  snprintf (s, sizeof (s), "%d", i3mud_info.password);
  I3_write_buffer (s);
  I3_write_buffer (",");
  snprintf (s, sizeof (s), "%d", i3mud_info.mudlist_id);
  I3_write_buffer (s);
  I3_write_buffer (",");
  snprintf (s, sizeof (s), "%d", i3mud_info.chanlist_id);
  I3_write_buffer (s);
  I3_write_buffer (",");
  snprintf (s, sizeof (s), "%d", i3mud_info.player_port);
  I3_write_buffer (s);
  I3_write_buffer (",0,0,\"");

  I3_write_buffer (i3mud_info.mudlib);
  I3_write_buffer ("\",\"");
  I3_write_buffer (i3mud_info.base_mudlib);
  I3_write_buffer ("\",\"");
  I3_write_buffer (i3mud_info.driver);
  I3_write_buffer ("\",\"");
  I3_write_buffer (i3mud_info.mud_type);
  I3_write_buffer ("\",\"");
  I3_write_buffer (i3mud_info.open_status);
  I3_write_buffer ("\",\"");
  I3_write_buffer (i3mud_info.admin_email);
  I3_write_buffer ("\",");


  I3_write_buffer ("([");
  if (i3mud_info.emoteto)
    I3_write_buffer ("\"emoteto\":1,");
  if (i3mud_info.news)
    I3_write_buffer ("\"news\":1,");
  if (i3mud_info.ucache)
    I3_write_buffer ("\"ucache\":1,");
  if (i3mud_info.auth)
    I3_write_buffer ("\"auth\":1,");
  if (i3mud_info.locate)
    I3_write_buffer ("\"locate\":1,");
  if (i3mud_info.finger)
    I3_write_buffer ("\"finger\":1,");
  if (i3mud_info.channel)
    I3_write_buffer ("\"channel\":1,");
  if (i3mud_info.who)
    I3_write_buffer ("\"who\":1,");
  if (i3mud_info.tell)
    I3_write_buffer ("\"tell\":1,");
  if (i3mud_info.beep)
    I3_write_buffer ("\"beep\":1,");
  if (i3mud_info.mail)
    I3_write_buffer ("\"mail\":1,");
  if (i3mud_info.file)
    I3_write_buffer ("\"file\":1,");
  if (i3mud_info.http)
    {
      snprintf (s, sizeof (s), "\"http\":%d,", i3mud_info.http);
      I3_write_buffer (s);
    }
  if (i3mud_info.smtp)
    {
      snprintf (s, sizeof (s), "\"smtp\":%d,", i3mud_info.smtp);
      I3_write_buffer (s);
    }
  if (i3mud_info.pop3)
    {
      snprintf (s, sizeof (s), "\"pop3\":%d,", i3mud_info.pop3);
      I3_write_buffer (s);
    }
  if (i3mud_info.ftp)
    {
      snprintf (s, sizeof (s), "\"ftp\":%d,", i3mud_info.ftp);
      I3_write_buffer (s);
    }
  if (i3mud_info.nntp)
    {
      snprintf (s, sizeof (s), "\"nntp\":%d,", i3mud_info.nntp);
      I3_write_buffer (s);
    }
  if (i3mud_info.rcp)
    {
      snprintf (s, sizeof (s), "\"rcp\":%d,", i3mud_info.rcp);
      I3_write_buffer (s);
    }
  if (i3mud_info.amrcp)
    {
      snprintf (s, sizeof (s), "\"amrcp\":%d,", i3mud_info.amrcp);
      I3_write_buffer (s);
    }
  I3_write_buffer ("]),([");


  if (!NullStr (i3mud_info.web))
    {
#ifndef DISABLE_WEBSRV
      if (web_is_connected ())
	snprintf (s, sizeof (s), "\"url\":\"%s:%d\",", i3mud_info.web,
		  webport);
      else
#endif
	snprintf (s, sizeof (s), "\"url\":\"%s\",", i3mud_info.web);
      I3_write_buffer (s);
    }
  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  snprintf (s, sizeof (s), "\"time\":\"%s\",", strtime);
  I3_write_buffer (s);

  I3_write_buffer ("]),})\r");
  I3_send_packet ();
}

DataTable ucache_data_table[] = {
  DTE (STRING, ucache, name, NULL, NULL, str, NULL),
  DTE (INT, ucache, gender, NULL, NULL, int, NULL),
  DTE (TIME, ucache, time, NULL, NULL, time, NULL),
  DTE_END
};

DataTable I3ban_data_table[] = {
  DTE (STRING, I3ban, name, NULL, NULL, str, NULL),
  DTE_END
};

DataTable I3chan_data_table[] = {
  DTE (STRING, I3chan, local_name, NULL, NULL, str, NULL),
  DTE (STRING, I3chan, host_mud, NULL, NULL, str, NULL),
  DTE (STRING, I3chan, I3_name, NULL, NULL, str, NULL),
  DTE (STRING, I3chan, layout_m, NULL, NULL, str, NULL),
  DTE (STRING, I3chan, layout_e, NULL, NULL, str, NULL),
  DTE (INT, I3chan, status, NULL, NULL, int, NULL),
  DTE (INT, I3chan, i3perm, NULL, NULL, int, NULL),
  DTE_SHOW (STRING_ARRAY, I3chan, history, MAX_I3HISTORY, NULL, NULL),
  DTE (FLAGVECTOR, I3chan, flags, NULL, NULL, flag, NULL),
  DTE_END
};

DataTable i3mud_data_table[] = {
  DTE (INT, i3mud, status, NULL, NULL, int, NULL),
  DTE (STRING, i3mud, name, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, ipaddress, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, mudlib, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, base_mudlib, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, driver, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, mud_type, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, open_status, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, admin_email, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, telnet, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, web_wrong, NULL, NULL, str, NULL),
  DTE (INT, i3mud, player_port, NULL, NULL, int, NULL),
  DTE (INT, i3mud, imud_tcp_port, NULL, NULL, int, NULL),
  DTE (INT, i3mud, imud_udp_port, NULL, NULL, int, NULL),
  DTE (BOOL, i3mud, tell, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, beep, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, emoteto, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, who, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, finger, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, locate, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, channel, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, news, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, mail, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, file, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, auth, NULL, NULL, bool, NULL),
  DTE (BOOL, i3mud, ucache, NULL, NULL, bool, NULL),
  DTE (INT, i3mud, smtp, NULL, NULL, int, NULL),
  DTE (INT, i3mud, ftp, NULL, NULL, int, NULL),
  DTE (INT, i3mud, nntp, NULL, NULL, int, NULL),
  DTE (INT, i3mud, http, NULL, NULL, int, NULL),
  DTE (INT, i3mud, pop3, NULL, NULL, int, NULL),
  DTE (INT, i3mud, rcp, NULL, NULL, int, NULL),
  DTE (INT, i3mud, amrcp, NULL, NULL, int, NULL),
  DTE (STRING, i3mud, banner, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, web, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, time, NULL, NULL, str, NULL),
  DTE (STRING, i3mud, daemon, NULL, NULL, str, NULL),
  DTE (INT, i3mud, jeamland, NULL, NULL, int, NULL),
  DTE (BOOL, i3mud, autoconnect, NULL, NULL, bool, NULL),
  DTE (INT, i3mud, password, NULL, NULL, int, NULL),
  DTE (INT, i3mud, mudlist_id, NULL, NULL, int, NULL),
  DTE (INT, i3mud, chanlist_id, NULL, NULL, int, NULL),
  DTE (INT, i3mud, minlevel, NULL, NULL, int, NULL),
  DTE (INT, i3mud, immlevel, NULL, NULL, int, NULL),
  DTE (INT, i3mud, adminlevel, NULL, NULL, int, NULL),
  DTE (INT, i3mud, implevel, NULL, NULL, int, NULL),
  DTE_END
};

DataTable router_data_table[] = {
  DTE (STRING, router, name, NULL, NULL, str, NULL),
  DTE (STRING, router, ip, NULL, NULL, str, NULL),
  DTE (INT, router, port, NULL, NULL, int, NULL),
  DTE_END
};

DataTable i3color_data_table[] = {
  DTE (STRING, i3color, name, NULL, NULL, str, NULL),
  DTE (STRING, i3color, mudtag, NULL, NULL, str, NULL),
  DTE (STRING, i3color, i3tag, NULL, NULL, str, NULL),
  DTE (STRING, i3color, i3fish, NULL, NULL, str, NULL),
  DTE_END
};

TableRW_Fun (i3_fun_rw)
{
  Do_F **fun = (Do_F **) temp;

  switch (type)
    {
    case act_write:
      *arg = i3_funcname (*fun);
      return true;
    case act_read:
      *fun = (Do_F *) i3_function (*arg);
      return true;
    default:
      return false;
    }
}

DataTable i3cmd_data_table[] = {
  DTE (STRING, i3cmd, name, NULL, NULL, str, NULL),
  DTE (STRING, i3cmd, args, NULL, NULL, str, NULL),
  DTE (STRING, i3cmd, descr, NULL, NULL, str, NULL),
  DTE_SHOW (FUNCTION_INT_TO_STR, i3cmd, function, i3_fun_rw, NULL, NULL),
  DTE (INT_FLAGSTRING, i3cmd, level, i3perm_names, NULL, flag, NULL),
  DTE (BOOL, i3cmd, connected, NULL, NULL, bool, NULL),
  DTE_SHOW (NAMELIST, i3cmd, alias_first, &i3cmd_zero.alias_last, NULL,
	    NULL),
  DTE_END
};

DataTable i3help_data_table[] = {
  DTE (STRING, i3help, name, NULL, NULL, str, NULL),
  DTE (STRING, i3help, text, 1, NULL, str, NULL),
  DTE (INT_FLAGSTRING, i3help, level, i3perm_names, NULL, flag, NULL),
};


void
I3_process_startup_reply (I3_HEADER * header, char *s)
{
  ROUTER_DATA *router;
  I3_CHANNEL *channel;
  char *ps = s, *next_ps;


  I3_get_field (ps, &next_ps);
  log_string (ps);
  ps = next_ps;


  I3_get_field (ps, &next_ps);
  i3mud_info.password = atoi (ps);

  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
	      "Received startup_reply from %s", header->originator_mudname);

  for (router = router_first; router; router = router->next)
    {
      if (!str_cmp (router->name, header->originator_mudname))
	{
	  router->reconattempts = 0;
	  I3_ROUTER_NAME = router->name;
	  break;
	}
    }
  i3wait = 0;
  i3timeout = 0;
  wiznet ("Intermud-3 Network connection complete.", NULL, NULL, WIZ_I3,
	  true, 1);

  for (channel = I3chan_first; channel; channel = channel->next)
    {
      if (!NullStr (channel->local_name))
	{
	  new_wiznet (NULL, NULL, WIZ_I3, true, 1, "Subscribing to %s",
		      channel->local_name);
	  I3_send_channel_listen (channel, true);
	}
    }
  return;
}

void
I3_process_chanack (I3_HEADER * header, char *s)
{
  CharData *ch;
  char *next_ps, *ps = s;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!(ch = I3_find_user (header->target_username)))
    log_string (ps);
  else
    i3printlnf (ch, "{G%s", ps);
  return;
}

void
I3_send_error (const char *mud, const char *user, const char *code,
	       const char *message)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("error", I3_THISMUD, 0, mud, user);
  I3_write_buffer ("\"");
  I3_write_buffer (code);
  I3_write_buffer ("\",\"");
  I3_write_buffer (I3_escape (message));
  I3_write_buffer ("\",0,})\r");
  I3_send_packet ();
}

void
I3_process_error (I3_HEADER * header, char *s)
{
  CharData *ch;
  char *next_ps, *ps = s;
  char type[MIL], error[MSL];

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (type, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);


  if (!str_cmp (header->originator_mudname, "VargonMUD"))
    return;

  snprintf (error, sizeof (error), "Error: from %s to %s@%s\n\r%s: %s",
	    header->originator_mudname, header->target_username,
	    header->target_mudname, type, ps);

  if (!(ch = I3_find_user (header->target_username)))
    log_string (error);
  else
    i3println (ch, error);
}

sex_t
i3todikugender (sex_t gender)
{
  sex_t sex = SEX_NONE;

  switch (gender)
    {
    case SEX_NEUTRAL:
      sex = SEX_MALE;
      break;

    case SEX_MALE:
      sex = SEX_FEMALE;
      break;

    default:
      sex = SEX_NEUTRAL;
      break;
    }

  return sex;
}

sex_t
dikutoi3gender (sex_t gender)
{
  sex_t sex = SEX_NONE;

  switch (gender)
    {
    default:
      sex = SEX_FEMALE;
      break;

    case SEX_MALE:
      sex = SEX_NEUTRAL;
      break;

    case SEX_FEMALE:
      sex = SEX_MALE;
      break;
    }

  return sex;
}


Do_Fun (I3_show_ucache_contents)
{
  I3_UCACHE *user;
  Buffer *buf;
  int users = 0;

  buf = new_buf ();
  i3bprintln (buf, "Cached user information");
  i3bprintln (buf, "1 = Female, 2 = Neuter, User | Gender ( 0 = Male)");
  i3bprintln (buf, draw_line (ch, NULL, 0));
  for (user = ucache_first; user; user = user->next)
    {
      i3bprintlnf (buf, "%-30s %d", user->name, user->gender);
      users++;
    }
  i3bprintlnf (buf, "%d users being cached.", users);
  sendpage (ch, buf_string (buf));
  free_buf (buf);
  return;
}

sex_t
I3_get_ucache_gender (char *name)
{
  I3_UCACHE *user;

  for (user = ucache_first; user; user = user->next)
    {
      if (!str_cmp (user->name, name))
	return user->gender;
    }


  return SEX_NONE;
}

I3_UCACHE *
new_ucache (void)
{
  I3_UCACHE *Data;

  alloc_mem (Data, I3_UCACHE, 1);

  return Data;
}

TableSave_Fun (rw_i3_ucache)
{
  rw_list (type, I3_UCACHE_FILE, I3_UCACHE, ucache);
}

void
I3_prune_ucache (void)
{
  I3_UCACHE *ucache, *next_ucache;

  for (ucache = ucache_first; ucache; ucache = next_ucache)
    {
      next_ucache = ucache->next;


      if (current_time - ucache->time >= (DAY * 30))
	{
	  free_string (ucache->name);
	  UnLink (ucache, ucache, next, prev);
	  free_mem (ucache);
	}
    }
  rw_i3_ucache (act_write);
  return;
}


void
I3_ucache_update (char *name, sex_t gender)
{
  I3_UCACHE *user;

  for (user = ucache_first; user; user = user->next)
    {
      if (!str_cmp (user->name, name))
	{
	  user->gender = gender;
	  user->time = current_time;
	  return;
	}
    }
  alloc_mem (user, I3_UCACHE, 1);
  user->name = str_dup (name);
  user->gender = gender;
  user->time = current_time;
  Link (user, ucache, next, prev);

  rw_i3_ucache (act_write);
  return;
}

void
I3_send_ucache_update (const char *visname, sex_t gender)
{
  char buf[10];

  if (!I3_is_connected ())
    return;

  I3_write_header ("ucache-update", I3_THISMUD, NULL, NULL, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (visname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (visname);
  I3_write_buffer ("\",");
  snprintf (buf, sizeof (buf), "%d", gender);
  I3_write_buffer (buf);
  I3_write_buffer (",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_ucache_update (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char username[MIL], visname[MIL], buf[MSL];
  sex_t sex, gender;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (username, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  gender = (sex_t) atoi (ps);

  snprintf (buf, sizeof (buf), "%s@%s", visname, header->originator_mudname);

  sex = I3_get_ucache_gender (buf);

  if (sex == gender)
    return;

  I3_ucache_update (buf, gender);
  return;
}

void
I3_send_chan_user_req (const char *targetmud, const char *targetuser)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("chan-user-req", I3_THISMUD, NULL, targetmud, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (targetuser);
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_chan_user_req (I3_HEADER * header, char *s)
{
  char buf[MSL];
  char *ps = s, *next_ps;
  CharData *ch;
  sex_t sex, gender;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (ps)))
    {
      if (!exists_player (ps))
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "No such player.");
      else
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "That player is offline.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "No such player.");
      return;
    }

  if (I3ISINVIS (ch) || i3ignoring (ch, buf))
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "That player is offline.");
      return;
    }


  gender = ch->sex;

  sex = dikutoi3gender (gender);

  I3_write_header ("chan-user-reply", I3_THISMUD, NULL,
		   header->originator_mudname, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (GetName (ch));
  I3_write_buffer ("\",\"");
  I3_write_buffer (GetName (ch));
  I3_write_buffer ("\",");
  snprintf (buf, sizeof (buf), "%d", sex);
  I3_write_buffer (buf);
  I3_write_buffer (",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_chan_user_reply (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char username[MIL], visname[MIL], buf[MSL];
  sex_t sex, gender;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (username, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  gender = (sex_t) atoi (ps);

  snprintf (buf, sizeof (buf), "%s@%s", visname, header->originator_mudname);

  sex = I3_get_ucache_gender (buf);

  if (sex == gender)
    return;

  I3_ucache_update (buf, gender);
  return;
}

void
I3_process_mudlist (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  I3_MUD *mud = NULL;
  char mud_name[MIL];

  I3_get_field (ps, &next_ps);
  i3mud_info.mudlist_id = atoi (ps);

  ps = next_ps;
  ps += 2;

  while (1)
    {
      char *next_ps2;

      I3_get_field (ps, &next_ps);
      I3_remove_quotes (&ps);
      strlcpy (mud_name, ps, MIL);

      ps = next_ps;
      I3_get_field (ps, &next_ps2);

      if (ps[0] != '0')
	{
	  mud = find_I3_mud_by_name (mud_name);
	  if (!mud)
	    mud = new_I3_mud (mud_name);

	  ps += 2;
	  I3_get_field (ps, &next_ps);
	  mud->status = atoi (ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->ipaddress, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  mud->player_port = atoi (ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  mud->imud_tcp_port = atoi (ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  mud->imud_udp_port = atoi (ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->mudlib, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->base_mudlib, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->driver, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->mud_type, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->open_status, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&mud->admin_email, ps);
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);

	  ps += 2;
	  while (1)
	    {
	      char *next_ps3;
	      char key[MIL];

	      if (ps[0] == ']')
		break;

	      I3_get_field (ps, &next_ps3);
	      I3_remove_quotes (&ps);
	      strlcpy (key, ps, MIL);
	      ps = next_ps3;
	      I3_get_field (ps, &next_ps3);

	      switch (key[0])
		{
		case 'a':
		  if (!str_cmp (key, "auth"))
		    {
		      mud->auth = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  if (!str_cmp (key, "amrcp"))
		    {
		      mud->amrcp = atoi (ps);
		      break;
		    }
		  break;
		case 'b':
		  if (!str_cmp (key, "beep"))
		    {
		      mud->beep = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'c':
		  if (!str_cmp (key, "channel"))
		    {
		      mud->channel = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'e':
		  if (!str_cmp (key, "emoteto"))
		    {
		      mud->emoteto = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'f':
		  if (!str_cmp (key, "file"))
		    {
		      mud->file = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  if (!str_cmp (key, "finger"))
		    {
		      mud->finger = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  if (!str_cmp (key, "ftp"))
		    {
		      mud->ftp = atoi (ps);
		      break;
		    }
		  break;
		case 'h':
		  if (!str_cmp (key, "http"))
		    {
		      mud->http = atoi (ps);
		      break;
		    }
		  break;
		case 'l':
		  if (!str_cmp (key, "locate"))
		    {
		      mud->locate = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'm':
		  if (!str_cmp (key, "mail"))
		    {
		      mud->mail = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'n':
		  if (!str_cmp (key, "news"))
		    {
		      mud->news = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  if (!str_cmp (key, "nntp"))
		    {
		      mud->nntp = atoi (ps);
		      break;
		    }
		  break;
		case 'p':
		  if (!str_cmp (key, "pop3"))
		    {
		      mud->pop3 = atoi (ps);
		      break;
		    }
		  break;
		case 'r':
		  if (!str_cmp (key, "rcp"))
		    {
		      mud->rcp = atoi (ps);
		      break;
		    }
		  break;
		case 's':
		  if (!str_cmp (key, "smtp"))
		    {
		      mud->smtp = atoi (ps);
		      break;
		    }
		  break;
		case 't':
		  if (!str_cmp (key, "tell"))
		    {
		      mud->tell = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		case 'u':
		  if (!str_cmp (key, "ucache"))
		    {
		      mud->ucache = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  if (!str_cmp (key, "url"))
		    {
		      I3_remove_quotes (&ps);
		      replace_str (&mud->web_wrong, ps);
		      break;
		    }
		  break;
		case 'w':
		  if (!str_cmp (key, "who"))
		    {
		      mud->who = ps[0] == '0' ? 0 : 1;
		      break;
		    }
		  break;
		default:
		  break;
		}

	      ps = next_ps3;
	      if (ps[0] == ']')
		break;
	    }
	  ps = next_ps;

	  I3_get_field (ps, &next_ps);
	  ps = next_ps;

	}
      else
	{
	  if ((mud = find_I3_mud_by_name (mud_name)) != NULL)
	    destroy_I3_mud (mud);
	}
      ps = next_ps2;
      if (ps[0] == ']')
	break;
    }
  return;
}


I3_CHANNEL *
new_I3chan (void)
{
  I3_CHANNEL *Data;

  alloc_mem (Data, I3_CHANNEL, 1);

  set_array (Data->history, &str_empty[0], MAX_I3HISTORY);
  return Data;
}

void
init_i3_channels (void)
{
  I3_CHANNEL *channel;

  for (channel = I3chan_first; channel; channel = channel->next)
    {

      if (channel->i3perm > I3PERM_IMP)
	{

	  if (channel->i3perm < i3mud_info.minlevel)
	    channel->i3perm = I3PERM_NONE;
	  else if (channel->i3perm >= i3mud_info.minlevel
		   && channel->i3perm < i3mud_info.immlevel)
	    channel->i3perm = I3PERM_MORT;
	  else if (channel->i3perm >= i3mud_info.immlevel
		   && channel->i3perm < i3mud_info.adminlevel)
	    channel->i3perm = I3PERM_IMM;
	  else if (channel->i3perm >= i3mud_info.adminlevel
		   && channel->i3perm < i3mud_info.implevel)
	    channel->i3perm = I3PERM_ADMIN;
	  else if (channel->i3perm >= i3mud_info.implevel)
	    channel->i3perm = I3PERM_IMP;
	}
    }
}

TableSave_Fun (rw_i3_channels)
{
  rw_list (type, I3_CHANNEL_FILE, I3_CHANNEL, I3chan);
  if (type == act_read)
    {
      init_i3_channels ();
    }
}

void
I3_process_chanlist_reply (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  I3_CHANNEL *channel;
  char chan[MIL];

  I3_get_field (ps, &next_ps);
  i3mud_info.chanlist_id = atoi (ps);

  ps = next_ps;
  ps += 2;

  while (1)
    {
      char *next_ps2;

      I3_get_field (ps, &next_ps);
      I3_remove_quotes (&ps);
      strlcpy (chan, ps, MIL);

      ps = next_ps;
      I3_get_field (ps, &next_ps2);
      if (ps[0] != '0')
	{
	  if (!(channel = find_I3_channel_by_name (chan)))
	    {
	      channel = new_I3_channel ();
	      channel->I3_name = str_dup (chan);
	    }

	  ps += 2;
	  I3_get_field (ps, &next_ps);
	  I3_remove_quotes (&ps);
	  replace_str (&channel->host_mud, ps);
	  ps = next_ps;
	  I3_get_field (ps, &next_ps);
	  channel->status = atoi (ps);
	}
      else
	{
	  if ((channel = find_I3_channel_by_name (chan)) != NULL)
	    {
	      if (!NullStr (channel->local_name))
		new_wiznet (NULL, NULL, WIZ_I3, true, 1,
			    "Locally configured channel %s has been purged from router %s",
			    channel->local_name, I3_ROUTER_NAME);
	      destroy_I3_channel (channel);
	      rw_i3_channels (act_write);
	    }
	}
      ps = next_ps2;
      if (ps[0] == ']')
	break;
    }
  return;
}

void
I3_send_channel_message (I3_CHANNEL * channel, const char *name,
			 const char *message)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-m", I3_THISMUD, name, NULL, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (name);
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (message));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_send_channel_emote (I3_CHANNEL * channel, const char *name,
		       const char *message)
{
  char buf[MSL];

  if (!I3_is_connected ())
    return;

  if (strstr (message, "$N") == NULL)
    snprintf (buf, sizeof (buf), "$N %s", message);
  else
    strlcpy (buf, message, MSL);

  I3_write_header ("channel-e", I3_THISMUD, name, NULL, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (name);
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (buf));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_send_channel_t (I3_CHANNEL * channel, const char *name,
		   const char *tmud, const char *tuser,
		   const char *msg_o, const char *msg_t, const char *tvis)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-t", I3_THISMUD, name, NULL, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (tmud);
  I3_write_buffer ("\",\"");
  I3_write_buffer (tuser);
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (msg_o));
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (msg_t));
  I3_write_buffer ("\",\"");
  I3_write_buffer (name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (tvis);
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

int
I3_token (char type, char *string, char *oname, char *tname)
{
  char code[50];
  char *p = '\0';

  switch (type)
    {
    default:
      code[0] = type;
      code[1] = '\0';
      return 1;
    case '$':
      strlcpy (code, "$", 50);
      break;
    case ' ':
      strlcpy (code, " ", 50);
      break;
    case 'N':
      strlcpy (code, oname, 50);
      break;
    case 'O':
      strlcpy (code, tname, 50);
      break;
    }
  p = code;
  while (*p != '\0')
    {
      *string = *p++;
      *++string = '\0';
    }
  return (strlen (code));
}

void
I3_message_convert (char *buffer, const char *txt, char *oname, char *tname)
{
  const char *point;
  int skip = 0;

  for (point = txt; *point; point++)
    {
      if (*point == '$')
	{
	  point++;
	  if (*point == '\0')
	    point--;
	  else
	    skip = I3_token (*point, buffer, oname, tname);
	  while (skip-- > 0)
	    ++buffer;
	  continue;
	}
      *buffer = *point;
      *++buffer = '\0';
    }
  *buffer = '\0';
  return;
}

char *
I3_convert_channel_message (const char *message, char *sname, char *tname)
{
  static char msgbuf[MSL];


  if (!message)
    {
      bug ("I3_convert_channel_message: NULL message!");
      return "ERROR";
    }

  if (!sname)
    {
      bug ("I3_convert_channel_message: NULL sname!");
      return "ERROR";
    }

  if (!tname)
    {
      bug ("I3_convert_channel_message: NULL tname!");
      return "ERROR";
    }

  I3_message_convert (msgbuf, message, sname, tname);
  return msgbuf;
}

void
update_chanhistory (I3_CHANNEL * channel, const char *message)
{
  char msg[MSL], buf[MSL];
  struct tm *local;
  time_t t;
  int x;

  if (!channel)
    {
      bug ("update_chanhistory: NULL channel received!");
      return;
    }

  if (NullStr (message))
    {
      bug ("update_chanhistory: NULL message received!");
      return;
    }

  strlcpy (msg, message, MSL);
  for (x = 0; x < MAX_I3HISTORY; x++)
    {
      if (channel->history[x] == NULL)
	{
	  t = time (NULL);
	  local = localtime (&t);
	  snprintf (buf, sizeof (buf),
		    "{R[%-2.2d/%-2.2d %-2.2d:%-2.2d] {G%s" NEWLINE,
		    local->tm_mon + 1, local->tm_mday, local->tm_hour,
		    local->tm_min, msg);
	  channel->history[x] = str_dup (buf);

	  if (IsSet (channel->flags, I3CHAN_LOG))
	    {
	      FileData *fp;

	      snprintf (buf, sizeof (buf), I3DIR "%s.log",
			channel->local_name);
	      if (!(fp = f_open (buf, "a")))
		{
		  log_error (buf);
		  bugf ("Could not open file %s!", buf);
		}
	      else
		{
		  f_printf (fp, "%s\n",
			    i3_strip_colors (channel->history[x]));
		  f_close (fp);
		}
	    }
	  break;
	}

      if (x == MAX_I3HISTORY - 1)
	{
	  int y;

	  for (y = 1; y < MAX_I3HISTORY; y++)
	    {
	      int z = y - 1;

	      if (channel->history[z] != NULL)
		{
		  replace_str (&channel->history[z], channel->history[y]);
		}
	    }

	  t = time (NULL);
	  local = localtime (&t);
	  snprintf (buf, sizeof (buf),
		    "{R[%-2.2d/%-2.2d %-2.2d:%-2.2d] {G%s" NEWLINE,
		    local->tm_mon + 1, local->tm_mday, local->tm_hour,
		    local->tm_min, msg);
	  replace_str (&channel->history[x], buf);

	  if (IsSet (channel->flags, I3CHAN_LOG))
	    {
	      FileData *fp;

	      snprintf (buf, sizeof (buf), I3DIR "%s.log",
			channel->local_name);
	      if (!(fp = f_open (buf, "a")))
		{
		  log_error (buf);
		  bugf ("Could not open file %s!", buf);
		}
	      else
		{
		  f_printf (fp, "%s\n",
			    i3_strip_colors (channel->history[x]));
		  f_close (fp);
		}
	    }
	}
    }
  return;
}

void
I3_chan_filter_m (I3_CHANNEL * channel, I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char visname[MIL], newmsg[MSL];

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (newmsg, ps, MSL);
  snprintf (newmsg, sizeof (newmsg), "%s%s", ps, " (filtered M)");

  I3_write_header ("chan-filter-reply", I3_THISMUD, NULL, I3_ROUTER_NAME,
		   NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",({\"channel-m\",5,\"");
  I3_write_buffer (header->originator_mudname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (header->originator_username);
  I3_write_buffer ("\",0,0,\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (visname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (newmsg);
  I3_write_buffer ("\",}),})\r");

  I3_send_packet ();
  return;
}

void
I3_chan_filter_e (I3_CHANNEL * channel, I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char visname[MIL], newmsg[MSL];

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  snprintf (newmsg, sizeof (newmsg), "%s%s", ps, " (filtered E)");

  I3_write_header ("chan-filter-reply", I3_THISMUD, NULL, I3_ROUTER_NAME,
		   NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",({\"channel-e\",5,\"");
  I3_write_buffer (header->originator_mudname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (header->originator_username);
  I3_write_buffer ("\",0,0,\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (visname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (newmsg);
  I3_write_buffer ("\",}),})\r");

  I3_send_packet ();
  return;
}

void
I3_chan_filter_t (I3_CHANNEL * channel, I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char targetmud[MIL], targetuser[MIL], message_o[MSL], message_t[MSL];
  char visname_o[MIL];

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (targetmud, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (targetuser, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  snprintf (message_o, sizeof (message_o), "%s%s", ps, " (filtered T)");

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  snprintf (message_t, sizeof (message_t), "%s%s", ps, " (filtered T)");

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname_o, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  I3_write_header ("chan-filter-reply", I3_THISMUD, NULL, I3_ROUTER_NAME,
		   NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",({\"channel-t\",5,\"");
  I3_write_buffer (header->originator_mudname);
  I3_write_buffer ("\",\"");
  I3_write_buffer (header->originator_username);
  I3_write_buffer ("\",0,0,\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",\"");
  I3_write_buffer (targetmud);
  I3_write_buffer ("\",\"");
  I3_write_buffer (targetuser);
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (message_o));
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (message_t));
  I3_write_buffer ("\",\"");
  I3_write_buffer (visname_o);
  I3_write_buffer ("\",\"");
  I3_write_buffer (ps);
  I3_write_buffer ("\",}),})\r");

  I3_send_packet ();
  return;
}

void
I3_process_channel_filter (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char ptype[MIL];
  I3_CHANNEL *channel = NULL;
  I3_HEADER *second_header;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!(channel = find_I3_channel_by_name (ps)))
    {
      logf ("I3_process_channel_filter: received unknown channel (%s)", ps);
      return;
    }

  if (!channel->local_name)
    return;

  ps = next_ps;
  ps += 2;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (ptype, ps, MIL);

  second_header = I3_get_header (&ps);

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!str_cmp (ptype, "channel-m"))
    I3_chan_filter_m (channel, second_header, next_ps);
  if (!str_cmp (ptype, "channel-e"))
    I3_chan_filter_e (channel, second_header, next_ps);
  if (!str_cmp (ptype, "channel-t"))
    I3_chan_filter_t (channel, second_header, next_ps);

  free_mem (second_header);
  return;
}

void
I3_process_channel_t (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  Descriptor *d;
  CharData *vch = NULL;
  char targetmud[MIL],
    targetuser[MIL], message_o[MSL], message_t[MSL], buf[MSL];
  char visname_o[MIL],
    sname[MIL], tname[MIL], lname[MIL], tmsg[MSL], omsg[MSL];
  I3_CHANNEL *channel = NULL;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!(channel = find_I3_channel_by_name (ps)))
    {
      logf ("I3_process_channel_t: received unknown channel (%s)", ps);
      return;
    }

  if (!channel->local_name)
    return;

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (targetmud, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (targetuser, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (message_o, ps, MSL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (message_t, ps, MSL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname_o, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (sname, sizeof (sname), "%s@%s", visname_o,
	    header->originator_mudname);
  snprintf (tname, sizeof (tname), "%s@%s", ps, targetmud);

  snprintf (omsg, sizeof (omsg), "%s",
	    I3_convert_channel_message (message_o, sname, tname));
  snprintf (tmsg, sizeof (tmsg), "%s",
	    I3_convert_channel_message (message_t, sname, tname));

  for (d = descriptor_first; d; d = d->next)
    {
      if (!d->character)
	continue;

      vch = CH (d);

      if (!hasname
	  (vch->pcdata->i3chardata->i3_listen, channel->local_name)
	  || hasname (vch->pcdata->i3chardata->i3_denied,
		      channel->local_name))
	continue;

      snprintf (lname, sizeof (lname), "%s@%s", GetName (vch), I3_THISMUD);

      if (d->connected == CON_PLAYING && !i3ignoring (vch, sname))
	{
	  if (!str_cmp (lname, tname))
	    {
	      snprintf (buf, sizeof (buf), channel->layout_e,
			channel->local_name, tmsg);
	      i3println (vch, buf);
	    }
	  else
	    {
	      snprintf (buf, sizeof (buf), channel->layout_e,
			channel->local_name, omsg);
	      i3println (vch, buf);
	    }
	}
    }
  update_chanhistory (channel, omsg);
  return;
}

void
I3_process_channel_m (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  Descriptor *d;
  CharData *vch = NULL;
  char visname[MIL], buf[MSL];
  I3_CHANNEL *channel;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!(channel = find_I3_channel_by_name (ps)))
    {
      logf ("channel_m: received unknown channel (%s)", ps);
      return;
    }

  if (!channel->local_name)
    return;

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (visname, ps, MIL);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (buf, sizeof (buf), channel->layout_m, channel->local_name,
	    visname, header->originator_mudname, ps);
  for (d = descriptor_first; d; d = d->next)
    {
      if (!d->character)
	continue;

      vch = CH (d);

      if (!hasname
	  (vch->pcdata->i3chardata->i3_listen, channel->local_name)
	  || hasname (vch->pcdata->i3chardata->i3_denied,
		      channel->local_name))
	continue;

      if (d->connected == CON_PLAYING && !i3ignoring (vch, visname))
	i3println (vch, buf);
    }
  update_chanhistory (channel, buf);
  return;
}

void
I3_process_channel_e (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  Descriptor *d;
  CharData *vch = NULL;
  char visname[MIL], msg[MSL], buf[MSL];
  I3_CHANNEL *channel;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  if (!(channel = find_I3_channel_by_name (ps)))
    {
      logf ("channel_e: received unknown channel (%s)", ps);
      return;
    }

  if (!channel->local_name)
    return;

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  snprintf (visname, sizeof (visname), "%s@%s", ps,
	    header->originator_mudname);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (msg, sizeof (msg), "%s",
	    I3_convert_channel_message (ps, visname, visname));
  snprintf (buf, sizeof (buf), channel->layout_e, channel->local_name, msg);

  for (d = descriptor_first; d; d = d->next)
    {
      if (!d->character)
	continue;

      vch = CH (d);

      if (!hasname
	  (vch->pcdata->i3chardata->i3_listen, channel->local_name)
	  || hasname (vch->pcdata->i3chardata->i3_denied,
		      channel->local_name))
	continue;

      if (d->connected == CON_PLAYING && !i3ignoring (vch, visname))
	i3println (vch, buf);
    }
  update_chanhistory (channel, buf);
  return;
}

void
I3_process_chan_who_req (I3_HEADER * header, char *s)
{
  Descriptor *d;
  char *ps = s, *next_ps;
  char buf[MSL], ibuf[MIL];
  I3_CHANNEL *channel;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (ibuf, sizeof (ibuf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(channel = find_I3_channel_by_name (ps)))
    {
      snprintf (buf, sizeof (buf),
		"The channel you specified (%s) is unknown at %s", ps,
		I3_THISMUD);
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-channel", buf);
      logf ("chan_who_req: received unknown channel (%s)", ps);
      return;
    }

  if (!channel->local_name)
    {
      snprintf (buf, sizeof (buf),
		"The channel you specified (%s) is not registered at %s",
		ps, I3_THISMUD);
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-channel", buf);
      return;
    }

  I3_write_header ("chan-who-reply", I3_THISMUD, NULL,
		   header->originator_mudname, header->originator_username);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",({");

  for (d = descriptor_first; d; d = d->next)
    {
      if (!d->character)
	continue;

      if (I3ISINVIS (d->character))
	continue;

      if (hasname
	  (d->character->pcdata->i3chardata->i3_listen,
	   channel->local_name) && !i3ignoring (d->character, ibuf)
	  && !hasname (d->character->pcdata->i3chardata->i3_denied,
		       channel->local_name))
	{
	  I3_write_buffer ("\"");
	  I3_write_buffer (GetName (d->character));
	  I3_write_buffer ("\",");
	}
    }
  I3_write_buffer ("}),})\r");
  I3_send_packet ();
  return;
}

void
I3_process_chan_who_reply (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  CharData *ch;

  if (!(ch = I3_find_user (header->target_username)))
    {
      bugf ("I3_process_chan_who_reply(): user %s not found.",
	    header->target_username);
      return;
    }

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  i3printlnf (ch, "{WUsers listening to %s on %s:" NEWLINE, ps,
	      header->originator_mudname);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  ps += 2;
  while (1)
    {
      if (ps[0] == '}')
	{
	  i3println (ch, "{cNo information returned or no people listening.");
	  return;
	}
      I3_get_field (ps, &next_ps);
      I3_remove_quotes (&ps);
      i3printlnf (ch, "{c%s", ps);

      ps = next_ps;
      if (ps[0] == '}')
	break;
    }

  return;
}

void
I3_send_chan_who (CharData * ch, I3_CHANNEL * channel, I3_MUD * mud)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("chan-who-req", I3_THISMUD, GetName (ch), mud->name, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_send_beep (CharData * ch, const char *to, I3_MUD * mud)
{
  if (!I3_is_connected ())
    return;

  I3_escape (to);
  I3_write_header ("beep", I3_THISMUD, GetName (ch), mud->name, to);
  I3_write_buffer ("\"");
  I3_write_buffer (GetName (ch));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_beep (I3_HEADER * header, char *s)
{
  char buf[MIL];
  char *ps = s, *next_ps;
  CharData *ch;

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (header->target_username)))
    {
      if (!exists_player (header->target_username))
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "No such player.");
      else
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "That player is offline.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "No such player.");
      return;
    }

  if (I3ISINVIS (ch) || i3ignoring (ch, buf))
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "That player is offline.");
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_BEEP))
    {
      snprintf (buf, sizeof (buf), "%s is not accepting beeps.",
		GetName (ch));
      I3_send_error (header->originator_mudname, header->originator_username,
		     "unk-user", buf);
      return;
    }

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  i3printlnf (ch, "{Y\a%s@%s beeps you.", ps, header->originator_mudname);
  return;
}

Do_Fun (I3_beep)
{
  char *ps;
  char mud[MIL];
  I3_MUD *pmud;

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_DENYBEEP))
    {
      i3println (ch, "You are not allowed to use i3beeps.");
      return;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, "user@mud", "on/off", NULL);
      return;
    }

  if (!str_cmp (argument, "on"))
    {
      RemBit (ch->pcdata->i3chardata->i3flags, I3_BEEP);
      i3println (ch, "You now send and receive i3beeps.");
      return;
    }

  if (!str_cmp (argument, "off"))
    {
      SetBit (ch->pcdata->i3chardata->i3flags, I3_BEEP);
      i3println (ch, "You no longer send and receive i3beeps.");
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_BEEP))
    {
      i3println (ch, "Your i3beeps are turned off.");
      return;
    }

  if (I3ISINVIS (ch))
    {
      i3println (ch, "You are invisible.");
      return;
    }

  ps = strchr (argument, '@');

  if (NullStr (argument) || ps == NULL)
    {
      i3println (ch, "{YYou should specify a person@mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  ps[0] = '\0';
  ps++;
  strlcpy (mud, ps, MIL);

  if (!(pmud = find_I3_mud_by_name (mud)))
    {
      i3println (ch, "{YNo such mud known.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!str_cmp (I3_THISMUD, pmud->name))
    {
      i3println (ch, "Use your mud's own internal system for that.");
      return;
    }

  if (pmud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", pmud->name);
      return;
    }

  if (pmud->beep == 0)
    i3printlnf (ch,
		"%s does not support the 'beep' command. Sending anyway.",
		pmud->name);

  I3_send_beep (ch, argument, pmud);
  i3printlnf (ch, "{YYou beep %s@%s.", capitalize (argument), pmud->name);
}

void
I3_send_tell (CharData * ch, const char *to, I3_MUD * mud,
	      const char *message)
{
  if (!I3_is_connected ())
    return;

  I3_escape (to);
  I3_write_header ("tell", I3_THISMUD, GetName (ch), mud->name, to);
  I3_write_buffer ("\"");
  I3_write_buffer (GetName (ch));
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (message));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
i3_update_tellhistory (CharData * ch, const char *msg)
{
  char new_msg[MSL];
  time_t t = time (NULL);
  struct tm *local = localtime (&t);
  int x;

  snprintf (new_msg, sizeof (new_msg), "{R[%-2.2d:%-2.2d] %s",
	    local->tm_hour, local->tm_min, msg);

  for (x = 0; x < MAX_I3TELLHISTORY; x++)
    {
      if (ch->pcdata->i3chardata->i3_tellhistory[x] == '\0')
	{
	  ch->pcdata->i3chardata->i3_tellhistory[x] = str_dup (new_msg);
	  break;
	}

      if (x == MAX_I3TELLHISTORY - 1)
	{
	  int i;

	  for (i = 1; i < MAX_I3TELLHISTORY; i++)
	    {
	      free_string (ch->pcdata->i3chardata->i3_tellhistory[i - 1]);
	      ch->pcdata->i3chardata->i3_tellhistory[i - 1] =
		str_dup (ch->pcdata->i3chardata->i3_tellhistory[i]);
	    }
	  free_string (ch->pcdata->i3chardata->i3_tellhistory[x]);
	  ch->pcdata->i3chardata->i3_tellhistory[x] = str_dup (new_msg);
	}
    }
  return;
}

void
I3_process_tell (I3_HEADER * header, char *s)
{
  char buf[MIL], usr[MIL];
  char *ps = s, *next_ps;
  CharData *ch;

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (header->target_username)))
    {
      if (!exists_player (header->target_username))
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "No such player.");
      else
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "That player is offline.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "No such player.");
      return;
    }

  if (I3ISINVIS (ch) || i3ignoring (ch, buf))
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "That player is offline.");
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_TELL))
    {
      snprintf (buf, sizeof (buf), "%s is not accepting tells.",
		GetName (ch));
      I3_send_error (header->originator_mudname, header->originator_username,
		     "unk-user", buf);
      return;
    }

  if (I3AFK (ch))
    {
      snprintf (buf, sizeof (buf), "%s is currently AFK. Try back later.",
		GetName (ch));
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user", buf);
      return;
    }

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (usr, sizeof (usr), "%s@%s", ps, header->originator_mudname);
  snprintf (buf, sizeof (buf), "'%s@%s'", ps, header->originator_mudname);

  replace_str (&ch->pcdata->i3chardata->i3_replyname, buf);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (buf, sizeof (buf), "{Y%s i3tells you: {c%s", usr, ps);
  i3println (ch, buf);
  i3_update_tellhistory (ch, buf);
  return;
}

Do_Fun (I3_tell)
{
  char to[MIL], *ps;
  char mud[MIL];
  I3_MUD *pmud;

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_DENYTELL))
    {
      i3println (ch, "You are not allowed to use i3tells.");
      return;
    }

  if (NullStr (argument))
    {
      int x;

      cmd_syntax (ch, NULL, "<user@mud> <message>", "on/off", NULL);
      i3printlnf (ch, "{cThe last %d things you were told:",
		  MAX_I3TELLHISTORY);

      for (x = 0; x < MAX_I3TELLHISTORY; x++)
	{
	  if (ch->pcdata->i3chardata->i3_tellhistory[x] == NULL)
	    break;
	  i3println (ch, ch->pcdata->i3chardata->i3_tellhistory[x]);
	}
      return;
    }

  if (!str_cmp (argument, "on"))
    {
      RemBit (ch->pcdata->i3chardata->i3flags, I3_TELL);
      i3println (ch, "You now send and receive i3tells.");
      return;
    }

  if (!str_cmp (argument, "off"))
    {
      SetBit (ch->pcdata->i3chardata->i3flags, I3_TELL);
      i3println (ch, "You no longer send and receive i3tells.");
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_TELL))
    {
      i3println (ch, "Your i3tells are turned off.");
      return;
    }

  if (I3ISINVIS (ch))
    {
      i3println (ch, "You are invisible.");
      return;
    }

  argument = first_arg (argument, to, false);
  ps = strchr (to, '@');

  if (NullStr (to) || NullStr (argument) || ps == NULL)
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  ps[0] = '\0';
  ps++;
  strlcpy (mud, ps, MIL);

  if (!(pmud = find_I3_mud_by_name (mud)))
    {
      i3print (ch, "{YNo such mud known.\n\r"
	       "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!str_cmp (I3_THISMUD, pmud->name))
    {
      i3println (ch, "Use your mud's own internal system for that.");
      return;
    }

  if (pmud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", pmud->name);
      return;
    }

  if (pmud->tell == 0)
    {
      i3printlnf (ch, "%s does not support the 'tell' command.", pmud->name);
      return;
    }

  I3_send_tell (ch, to, pmud, argument);
  snprintf (mud, sizeof (mud), "{YYou i3tell %s@%s: {c%s", capitalize (to),
	    pmud->name, argument);
  i3println (ch, mud);
  i3_update_tellhistory (ch, mud);
}

Do_Fun (I3_reply)
{
  char buf[MSL];

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_DENYTELL))
    {
      i3println (ch, "You are not allowed to use i3tells.");
      return;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, "<message>", NULL);
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_TELL))
    {
      i3println (ch, "Your i3tells are turned off.");
      return;
    }

  if (I3ISINVIS (ch))
    {
      i3println (ch, "You are invisible.");
      return;
    }

  if (!ch->pcdata->i3chardata->i3_replyname)
    {
      i3println (ch, "You have not yet received an i3tell?!?");
      return;
    }

  snprintf (buf, sizeof (buf), "%s %s",
	    ch->pcdata->i3chardata->i3_replyname, argument);
  I3_tell (n_fun, ch, buf);
  return;
}

void
I3_send_who (CharData * ch, const char *mud)
{
  if (!I3_is_connected ())
    return;

  I3_escape (mud);
  I3_write_header ("who-req", I3_THISMUD, GetName (ch), mud, NULL);
  I3_write_buffer ("})\r");
  I3_send_packet ();

  return;
}


char *
i3centerline (const char *string, int length)
{
  char stripped[300];
  static char outbuf[400];
  int amount;

  strlcpy (stripped, i3_strip_colors (string), 300);
  amount = length - strlen (stripped);

  if (amount < 1)
    amount = 1;


  snprintf (outbuf, sizeof (outbuf), "%*s%s%*s", (amount / 2), "", string,
	    ((amount / 2) * 2) == amount ? (amount / 2) : ((amount / 2) + 1),
	    "");

  return outbuf;
}


#ifndef I3CUSTOMWHO
void
I3_process_who_req (I3_HEADER * header, char *s)
{
  Descriptor *d;
  CharData *person;
  char ibuf[MIL], personbuf[MSL], tailbuf[MSL];
  char smallbuf[50], buf[300], outbuf[400], stats[200], rank[200];
  int pcount = 0, xx, yy;
  long int bogusidle = 9999;

  snprintf (ibuf, sizeof (ibuf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  I3_write_header ("who-reply", I3_THISMUD, NULL,
		   header->originator_mudname, header->originator_username);
  I3_write_buffer ("({");


  I3_write_buffer ("({\"");
  snprintf (buf, 300, "{R-=[ {WPlayers on %s {R]=-", I3_THISMUD);
  strlcpy (outbuf, i3centerline (buf, 78), 400);
  send_to_i3 (I3_escape (outbuf));

  I3_write_buffer ("\",");
  snprintf (smallbuf, 50, "%ld", -1l);
  I3_write_buffer (smallbuf);

  I3_write_buffer (",\" \",}),({\"");
  snprintf (buf, 300, "{Y-=[ {Wtelnet://%s:%d {Y]=-", i3mud_info.telnet,
	    i3mud_info.player_port);
  strlcpy (outbuf, i3centerline (buf, 78), 400);
  send_to_i3 (I3_escape (outbuf));

  I3_write_buffer ("\",");
  snprintf (smallbuf, 50, "%ld", bogusidle);
  I3_write_buffer (smallbuf);

  I3_write_buffer (",\" \",}),");

  xx = 0;
  for (d = descriptor_first; d; d = d->next)
    {
      person = d->original ? d->original : d->character;

      if (person && d->connected >= CON_PLAYING)
	{
	  if (person->pcdata->i3chardata->i3perm < I3PERM_MORT
	      || person->pcdata->i3chardata->i3perm >= I3PERM_IMM
	      || I3ISINVIS (person) || i3ignoring (person, ibuf))
	    continue;

	  pcount++;

	  if (xx == 0)
	    {
	      I3_write_buffer ("({\"");
	      send_to_i3 (I3_escape
			  ("{B--------------------------------=[ {WPlayers              {B]=---------------------------------"));
	      I3_write_buffer ("\",");
	      snprintf (smallbuf, 50, "%ld", bogusidle);
	      I3_write_buffer (smallbuf);
	      I3_write_buffer (",\" \",}),");
	    }

	  I3_write_buffer ("({\"");

	  strlcpy (rank, rankbuffer (person), 200);
	  strlcpy (outbuf, i3centerline (rank, 20), 400);
	  send_to_i3 (I3_escape (outbuf));

	  I3_write_buffer ("\",");
	  snprintf (smallbuf, 50, "%ld", -1l);
	  I3_write_buffer (smallbuf);
	  I3_write_buffer (",\"");

	  strlcpy (stats, "{D[", 200);
	  if (I3AFK (person))
	    strlcat (stats, "AFK", 200);
	  else
	    strlcat (stats, "---", 200);
	  strlcat (stats, "]{G", 200);

	  snprintf (personbuf, sizeof (personbuf), "%s %s%s", stats,
		    GetName (person), person->pcdata->title);
	  send_to_i3 (I3_escape (personbuf));
	  I3_write_buffer ("\",}),");
	  xx++;
	}
    }

  yy = 0;
  for (d = descriptor_first; d; d = d->next)
    {
      person = CH (d);

      if (person && d->connected >= CON_PLAYING)
	{
	  if (person->pcdata->i3chardata->i3perm < I3PERM_IMM
	      || I3ISINVIS (person) || i3ignoring (person, ibuf))
	    continue;

	  pcount++;

	  if (yy == 0)
	    {
	      I3_write_buffer ("({\"");
	      send_to_i3 (I3_escape
			  ("{R-------------------------------=[ {WImmortals              {R]=--------------------------------"));
	      I3_write_buffer ("\",");
	      if (xx > 0)
		snprintf (smallbuf, 50, "%ld", bogusidle * 3);
	      else
		snprintf (smallbuf, 50, "%ld", bogusidle);
	      I3_write_buffer (smallbuf);
	      I3_write_buffer (",\" \",}),");
	    }
	  I3_write_buffer ("({\"");

	  strlcpy (rank, rankbuffer (person), 200);
	  strlcpy (outbuf, i3centerline (rank, 20), 400);
	  send_to_i3 (I3_escape (outbuf));

	  I3_write_buffer ("\",");
	  snprintf (smallbuf, 50, "%ld", -1l);
	  I3_write_buffer (smallbuf);
	  I3_write_buffer (",\"");

	  strlcpy (stats, "{D[", 200);
	  if (I3AFK (person))
	    strlcat (stats, "AFK", 200);
	  else
	    strlcat (stats, "---", 200);
	  strlcat (stats, "]{G", 200);

	  snprintf (personbuf, sizeof (personbuf), "%s %s%s", stats,
		    GetName (person), person->pcdata->title);
	  send_to_i3 (I3_escape (personbuf));
	  I3_write_buffer ("\",}),");
	  yy++;
	}
    }

  I3_write_buffer ("({\"");
  snprintf (tailbuf, sizeof (tailbuf), "{Y[{W%d Player%s{Y]", pcount,
	    pcount == 1 ? "" : "s");
  send_to_i3 (I3_escape (tailbuf));
  I3_write_buffer ("\",");
  snprintf (smallbuf, 50, "%ld", bogusidle * 2);
  I3_write_buffer (smallbuf);
  I3_write_buffer (",\"");
#ifndef DISABLE_WEBSRV
  if (web_is_connected ())
    snprintf (tailbuf, sizeof (tailbuf), "{Y[{WHomepage: http://%s:%d/{Y]",
	      i3mud_info.web, webport);
  else
#endif
    snprintf (tailbuf, sizeof (tailbuf), "{Y[{WHomepage: http://%s/{Y]",
	      i3mud_info.web);
  send_to_i3 (I3_escape (tailbuf));
  I3_write_buffer ("\",}),}),})\r");
  I3_send_packet ();
  return;
}
#endif


void
I3_process_who_reply (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps, *next_ps2;
  CharData *ch;
  char person[MSL], title[MIL];
  int idle;

  if (!(ch = I3_find_user (header->target_username)))
    return;

  ps += 2;

  while (1)
    {
      if (ps[0] == '}')
	{
	  i3println (ch, "{WNo information returned.");
	  return;
	}

      I3_get_field (ps, &next_ps);

      ps += 2;
      I3_get_field (ps, &next_ps2);
      I3_remove_quotes (&ps);
      strlcpy (person, ps, MSL);
      ps = next_ps2;
      I3_get_field (ps, &next_ps2);
      idle = atoi (ps);
      ps = next_ps2;
      I3_get_field (ps, &next_ps2);
      I3_remove_quotes (&ps);
      strlcpy (title, ps, MIL);
      ps = next_ps2;

      if (idle == 9999)
	i3printlnf (ch, "%s %s" NEWLINE, person, title);
      else if (idle == 19998)
	i3printlnf (ch, "\n\r%s %s", person, title);
      else if (idle == 29997)
	i3printlnf (ch, "\n\r%s %s" NEWLINE, person, title);
      else
	i3printlnf (ch, "%s %s", person, title);

      ps = next_ps;
      if (ps[0] == '}')
	break;
    }
  return;
}

void
I3_send_emoteto (CharData * ch, const char *to, I3_MUD * mud,
		 const char *message)
{
  char buf[MSL];

  if (!I3_is_connected ())
    return;

  if (strstr (message, "$N") == NULL)
    snprintf (buf, sizeof (buf), "$N %s", message);
  else
    strlcpy (buf, message, MSL);

  I3_escape (to);
  I3_write_header ("emoteto", I3_THISMUD, GetName (ch), mud->name, to);
  I3_write_buffer ("\"");
  I3_write_buffer (GetName (ch));
  I3_write_buffer ("\",\"");
  send_to_i3 (I3_escape (buf));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_emoteto (I3_HEADER * header, char *s)
{
  CharData *ch;
  char *ps = s, *next_ps;
  char visname[MIL], buf[MIL];

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (header->target_username)))
    {
      if (!exists_player (header->target_username))
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "No such player.");
      else
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "That player is offline.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "No such player.");
      return;
    }

  if (I3ISINVIS (ch) || i3ignoring (ch, buf) || !ch->desc)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "That player is offline.");
      return;
    }

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  snprintf (visname, sizeof (visname), "%s@%s", ps,
	    header->originator_mudname);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  i3printlnf (ch, "{c%s", I3_convert_channel_message (ps, visname, visname));
  return;
}

void
I3_send_finger (CharData * ch, const char *user, const char *mud)
{
  if (!I3_is_connected ())
    return;

  I3_escape (mud);

  I3_write_header ("finger-req", I3_THISMUD, GetName (ch), mud, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (I3_escape (user));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}


void
I3_process_finger_reply (I3_HEADER * header, char *s)
{
  CharData *ch;
  char *ps = s, *next_ps;
  char title[MIL], email[MIL], last[MIL], level[MIL];

  if (!(ch = I3_find_user (header->target_username)))
    return;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  i3printlnf (ch, "{wI3FINGER information for {G%s@%s", ps,
	      header->originator_mudname);
  i3println (ch, "{w-------------------------------------------------");
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (title, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (email, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (last, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (level, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  i3printlnf (ch, "{wTitle: {G%s", title);
  i3printlnf (ch, "{wLevel: {G%s", level);
  i3printlnf (ch, "{wEmail: {G%s", email);
  i3printlnf (ch, "{wHTTP : {G%s", ps);
  i3printlnf (ch, "{wLast on: {G%s", last);

  return;
}

void
I3_process_finger_req (I3_HEADER * header, char *s)
{
  CharData *ch;
  char *ps = s, *next_ps;
  char smallbuf[200], buf[MIL];

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (ps)))
    {
      if (!exists_player (ps))
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "No such player.");
      else
	I3_send_error (header->originator_mudname,
		       header->originator_username, "unk-user",
		       "That player is offline.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "No such player.");
      return;
    }

  if (I3ISINVIS (ch) || i3ignoring (ch, buf))
    {
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user",
		     "That player is offline.");
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_DENYFINGER)
      || IsSet (ch->pcdata->i3chardata->i3flags, I3_PRIVACY))
    {
      snprintf (buf, sizeof (buf), "%s is not accepting fingers.",
		GetName (ch));
      I3_send_error (header->originator_mudname,
		     header->originator_username, "unk-user", buf);
      return;
    }

  i3printlnf (ch, "%s@%s has requested your i3finger information.",
	      header->originator_username, header->originator_mudname);

  I3_write_header ("finger-reply", I3_THISMUD, NULL,
		   header->originator_mudname, header->originator_username);
  I3_write_buffer ("\"");
  I3_write_buffer (I3_escape (GetName (ch)));
  I3_write_buffer ("\",\"");
  I3_write_buffer (I3_escape (GetName (ch)));
  send_to_i3 (I3_escape (ch->pcdata->title));
  I3_write_buffer ("\",\"\",\"");
#ifdef FINGERCODE

  if (ch->pcdata->email)
    {
      if (!IsSet (ch->pcdata->flags, PCFLAG_PRIVACY))
	I3_write_buffer (ch->pcdata->email);
      else
	I3_write_buffer ("[Private]");
    }
#else
  I3_write_buffer ("Not supported");
#endif

  I3_write_buffer ("\",\"");
  strlcpy (smallbuf, "-1", 200);
  I3_write_buffer (smallbuf);
  I3_write_buffer ("\",");
  snprintf (smallbuf, sizeof (smallbuf), "%ld", -1l);
  I3_write_buffer (smallbuf);
  I3_write_buffer (",\"");
  I3_write_buffer ("[PRIVATE]");
  I3_write_buffer ("\",\"");
  snprintf (buf, sizeof (buf), "%s", rankbuffer (ch));
  send_to_i3 (buf);
#ifdef FINGERCODE

  I3_write_buffer ("\",\"");
  if (ch->pcdata->homepage)
    I3_write_buffer (I3_escape (ch->pcdata->homepage));
  else
    I3_write_buffer ("Not Provided");
  I3_write_buffer ("\",})\r");
#else

  I3_write_buffer ("\",\"Not Suported\",})\r");
#endif

  I3_send_packet ();

  return;
}

void
I3_send_locate (CharData * ch, const char *user)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("locate-req", I3_THISMUD, GetName (ch), NULL, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (I3_escape (user));
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_process_locate_reply (I3_HEADER * header, char *s)
{
  char mud_name[MIL], user_name[MIL], status[MIL];
  char *ps = s, *next_ps;
  CharData *ch;

  if (!(ch = I3_find_user (header->target_username)))
    return;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (mud_name, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (user_name, ps, MIL);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  ps = next_ps;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (status, ps, MIL);

  if (!str_cmp (status, "active"))
    strlcpy (status, "Online", MIL);

  if (!str_cmp (status, "exists, but not logged on"))
    strlcpy (status, "Offline", MIL);

  i3printlnf (ch, "{RI3 Locate: {Y%s@%s: {c%s.", user_name, mud_name, status);
  return;
}

void
I3_process_locate_req (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  char smallbuf[50], buf[MIL];
  CharData *ch;
  bool choffline = false;

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);

  snprintf (buf, sizeof (buf), "%s@%s", header->originator_username,
	    header->originator_mudname);

  if (!(ch = I3_find_user (ps)))
    {
      if (exists_player (ps))
	choffline = true;
      else
	return;
    }

  if (ch)
    {
      if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
	return;

      if (I3ISINVIS (ch))
	choffline = true;

      if (i3ignoring (ch, buf))
	choffline = true;
    }

  I3_write_header ("locate-reply", I3_THISMUD, NULL,
		   header->originator_mudname, header->originator_username);
  I3_write_buffer ("\"");
  I3_write_buffer (I3_THISMUD);
  I3_write_buffer ("\",\"");
  if (!choffline)
    I3_write_buffer (GetName (ch));
  else
    I3_write_buffer (capitalize (ps));
  I3_write_buffer ("\",");
  snprintf (smallbuf, sizeof (smallbuf), "%ld", -1l);
  I3_write_buffer (smallbuf);
  if (!choffline)
    I3_write_buffer (",\"Online\",})\r");
  else
    I3_write_buffer (",\"Offline\",})\r");
  I3_send_packet ();

  return;
}

void
I3_send_channel_listen (I3_CHANNEL * channel, bool lconnect)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-listen", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",");
  if (lconnect)
    I3_write_buffer ("1,})\r");
  else
    I3_write_buffer ("0,})\r");
  I3_send_packet ();

  return;
}

Do_Fun (I3_mudlisten)
{
  I3_CHANNEL *channel;
  char arg[MIL];

  if (NullStr (argument))
    {
      i3println (ch, "Usage: i3mudlisten [all/none]");
      i3println (ch, "Usage: i3mudlisten <localchannel> [on/off]");
      return;
    }

  if (!str_cmp (argument, "all"))
    {
      for (channel = I3chan_first; channel; channel = channel->next)
	{
	  if (NullStr (channel->local_name))
	    continue;

	  i3printlnf (ch, "Subscribing to %s.", channel->local_name);
	  I3_send_channel_listen (channel, true);
	}
      i3println (ch,
		 "{YThe mud is now subscribed to all available local I3 channels.");
      return;
    }

  if (!str_cmp (argument, "none"))
    {
      for (channel = I3chan_first; channel; channel = channel->next)
	{
	  if (NullStr (channel->local_name))
	    continue;

	  i3printlnf (ch, "Unsubscribing from %s.", channel->local_name);
	  I3_send_channel_listen (channel, false);
	}
      i3println (ch,
		 "{YThe mud is now unsubscribed from all available local I3 channels.");
      return;
    }

  argument = first_arg (argument, arg, false);
  if (!(channel = find_I3_channel_by_localname (arg)))
    {
      i3println (ch, "No such channel configured locally.");
      return;
    }

  if (!str_cmp (argument, "on"))
    {
      i3printlnf (ch, "Turning %s channel on.", channel->local_name);
      I3_send_channel_listen (channel, true);
      return;
    }

  if (!str_cmp (argument, "off"))
    {
      i3printlnf (ch, "Turning %s channel off.", channel->local_name);
      I3_send_channel_listen (channel, false);
      return;
    }
  I3_mudlisten (n_fun, ch, "");
  return;
}

void
i3init_char (CharData * ch)
{
  if (IsNPC (ch))
    return;

  alloc_mem (ch->pcdata->i3chardata, I3_CHARDATA, 1);
  ch->pcdata->i3chardata->i3_listen = NULL;
  ch->pcdata->i3chardata->i3_denied = NULL;
  ch->pcdata->i3chardata->i3_replyname = NULL;
  ch->pcdata->i3chardata->i3flags = I3_COLORFLAG;
  ch->pcdata->i3chardata->i3ignore_first = NULL;
  ch->pcdata->i3chardata->i3ignore_last = NULL;
  ch->pcdata->i3chardata->i3perm = I3PERM_NOTSET;

  return;
}

void
I3_char_login (CharData * ch)
{
  sex_t gender, sex;
  char buf[MIL];


  if (ch->pcdata->i3chardata->i3perm == I3PERM_NOTSET)
    {
      if (ch->level < i3mud_info.minlevel)
	ch->pcdata->i3chardata->i3perm = I3PERM_NONE;
      else if (ch->level >= i3mud_info.minlevel
	       && ch->level < i3mud_info.immlevel)
	ch->pcdata->i3chardata->i3perm = I3PERM_MORT;
      else if (ch->level >= i3mud_info.immlevel
	       && ch->level < i3mud_info.adminlevel)
	ch->pcdata->i3chardata->i3perm = I3PERM_IMM;
      else if (ch->level >= i3mud_info.adminlevel
	       && ch->level < i3mud_info.implevel)
	ch->pcdata->i3chardata->i3perm = I3PERM_ADMIN;
      else if (ch->level >= i3mud_info.implevel)
	ch->pcdata->i3chardata->i3perm = I3PERM_IMP;
    }

  if (!I3_is_connected ())
    {
      if (ch->pcdata->i3chardata->i3perm >= I3PERM_IMM && i3wait == -2)
	i3println (ch,
		   "{RThe Intermud-3 connection is down. Attempts to reconnect were abandoned due to excessive failures.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm < I3PERM_MORT)
    return;

  if (i3mud_info.ucache == true)
    {
      snprintf (buf, sizeof (buf), "%s@%s", GetName (ch), I3_THISMUD);
      gender = I3_get_ucache_gender (buf);
      sex = dikutoi3gender (ch->sex);

      if (gender == sex)
	return;

      I3_ucache_update (buf, sex);
      if (!IsSet (ch->pcdata->i3chardata->i3flags, I3_INVIS))
	I3_send_ucache_update (GetName (ch), sex);
    }
  return;
}

bool
i3load_char (CharData * ch, FileData * fp, const char *word)
{
  bool fMatch = false;

  if (IsNPC (ch))
    return false;

  switch (toupper (word[0]))
    {
    case 'I':
      Key ("i3perm", ch->pcdata->i3chardata->i3perm,
	   read_enum (i3perm_t, fp));
      if (!str_cmp (word, "i3flags"))
	{
	  ch->pcdata->i3chardata->i3flags = read_number (fp);
	  I3_char_login (ch);
	  read_to_eol (fp);
	  fMatch = true;
	  break;
	}


      if (!str_cmp (word, "i3invis"))
	{
	  bool tempinvis = (bool) read_number (fp);

	  if (tempinvis == true)
	    SetBit (ch->pcdata->i3chardata->i3flags, I3_INVIS);
	  I3_char_login (ch);
	  read_to_eol (fp);
	  fMatch = true;
	  break;
	}
      if (!str_cmp (word, "i3listen"))
	{
	  ch->pcdata->i3chardata->i3_listen = read_string (fp);
	  if (ch->pcdata->i3chardata->i3_listen != NULL && I3_is_connected ())
	    {
	      I3_CHANNEL *channel = NULL;
	      const char *channels = ch->pcdata->i3chardata->i3_listen;
	      char arg[MIL];

	      while (1)
		{
		  if (channels[0] == '\0')
		    break;

		  channels = first_arg (channels, arg, false);

		  if (!(channel = find_I3_channel_by_localname (arg)))
		    unflagname (&ch->pcdata->i3chardata->i3_listen, arg);
		  if (channel
		      && ch->pcdata->i3chardata->i3perm < channel->i3perm)
		    unflagname (&ch->pcdata->i3chardata->i3_listen, arg);
		}
	    }
	  read_to_eol (fp);
	  fMatch = true;
	  break;
	}
      if (!str_cmp (word, "i3deny"))
	{
	  ch->pcdata->i3chardata->i3_denied = read_string (fp);
	  if (ch->pcdata->i3chardata->i3_denied != NULL && I3_is_connected ())
	    {
	      I3_CHANNEL *channel = NULL;
	      const char *channels = ch->pcdata->i3chardata->i3_denied;
	      char arg[MIL];

	      while (1)
		{
		  if (channels[0] == '\0')
		    break;
		  channels = first_arg (channels, arg, false);

		  if (!(channel = find_I3_channel_by_localname (arg)))
		    unflagname (&ch->pcdata->i3chardata->i3_denied, arg);
		  if (channel
		      && ch->pcdata->i3chardata->i3perm < channel->i3perm)
		    unflagname (&ch->pcdata->i3chardata->i3_denied, arg);
		}
	    }
	  read_to_eol (fp);
	  fMatch = true;
	  break;
	}
      if (!str_cmp (word, "i3ignore"))
	{
	  I3_IGNORE *temp;

	  alloc_mem (temp, I3_IGNORE, 1);
	  temp->name = read_string (fp);
	  Link (temp, ch->pcdata->i3chardata->i3ignore, next, prev);
	  read_to_eol (fp);
	  fMatch = true;
	  break;
	}
      break;
    }
  return fMatch;
}

void
i3save_char (CharData * ch, FileData * fp)
{
  I3_IGNORE *temp;

  if (IsNPC (ch))
    return;

  write_int (fp, "i3perm", "%d", ch->pcdata->i3chardata->i3perm,
	     I3PERM_NOTSET);
  write_bit (fp, "i3flags", ch->pcdata->i3chardata->i3flags, I3_COLORFLAG);
  write_string (fp, "i3listen", ch->pcdata->i3chardata->i3_listen, NULL);
  write_string (fp, "i3deny", ch->pcdata->i3chardata->i3_denied, NULL);
  for (temp = ch->pcdata->i3chardata->i3ignore_first; temp; temp = temp->next)
    write_string (fp, "i3ignore", temp->name, NULL);
  return;
}


I3_COLOR *
new_i3color (void)
{
  I3_COLOR *Data;

  alloc_mem (Data, I3_COLOR, 1);
  return Data;
}

TableSave_Fun (rw_i3_colors)
{
  rw_list (type, I3_COLOR_FILE, I3_COLOR, i3color);
}

I3_HELP_DATA *
new_i3help (void)
{
  I3_HELP_DATA *Data;

  alloc_mem (Data, I3_HELP_DATA, 1);
  return Data;
}

TableSave_Fun (rw_i3_helps)
{
  rw_list (type, I3_HELP_FILE, I3_HELP_DATA, i3help);
}

I3_CMD_DATA *
new_i3cmd (void)
{
  I3_CMD_DATA *Data;

  alloc_mem (Data, I3_CMD_DATA, 1);
  return Data;
}

TableSave_Fun (rw_i3_cmds)
{
  rw_list (type, I3_CMD_FILE, I3_CMD_DATA, i3cmd);
}

ROUTER_DATA *
new_router (void)
{
  ROUTER_DATA *Data;

  alloc_mem (Data, ROUTER_DATA, 1);

  Data->reconattempts = 0;

  return Data;
}

TableSave_Fun (rw_i3_config)
{
  rw_single (type, I3_CONFIG_FILE, i3mud);
}

TableSave_Fun (rw_i3_routers)
{
  rw_list (type, I3_ROUTER_FILE, ROUTER_DATA, router);
}

ROUTER_DATA *
i3_find_router (const char *name)
{
  ROUTER_DATA *router;

  for (router = router_first; router; router = router->next)
    {
      if (!str_cmp (router->name, name))
	return router;
    }
  return NULL;
}

bool
I3_read_config (int mudport)
{
  wiznet ("Loading Intermud-3 network data...", NULL, NULL, WIZ_I3, true, 1);

  i3mud_info.status = -1;
  i3mud_info.autoconnect = 0;
  i3mud_info.password = 0;
  i3mud_info.mudlist_id = 0;
  i3mud_info.chanlist_id = 0;
  i3mud_info.minlevel = 10;
  i3mud_info.immlevel = LEVEL_IMMORTAL;
  i3mud_info.adminlevel = MAX_LEVEL - 2;
  i3mud_info.implevel = MAX_LEVEL;

  rw_i3_config (act_read);

  replace_strf (&i3mud_info.name, "%s-%d", mud_info.name, mud_info.unique_id);

  i3mud_info.player_port = mudport;


  replace_str (&i3mud_info.mudlib, MUDSTRING);


  if (str_cmp (i3mud_info.base_mudlib, MUDNAME))
    replace_str (&i3mud_info.base_mudlib, MUDNAME);

  replace_str (&i3mud_info.driver, I3DRIVER);

  if (NullStr (i3mud_info.telnet))
    replace_str (&i3mud_info.telnet, HOSTNAME);
  if (NullStr (i3mud_info.admin_email))
    replace_strf (&i3mud_info.admin_email, "%s@%s", UNAME, HOSTNAME);

  if (NullStr (i3mud_info.web))
    replace_str (&i3mud_info.web, HOSTNAME);

  I3_THISMUD = i3mud_info.name;
  return true;
}

I3_BAN *
new_I3ban (void)
{
  I3_BAN *Data;

  alloc_mem (Data, I3_BAN, 1);

  return Data;
}

TableSave_Fun (rw_i3_bans)
{
  rw_list (type, I3_BAN_FILE, I3_BAN, I3ban);
}

Do_Fun (I3_mudlist)
{
  I3_MUD *mud;
  char filter[MIL];
  int mudcount = 0;
  bool all = false;
  Buffer *output;

  argument = first_arg (argument, filter, false);

  if (!str_cmp (filter, "all"))
    {
      all = true;
      argument = first_arg (argument, filter, false);
    }

  if (i3mud_first == NULL)
    {
      i3println (ch, "There are no muds to list!?");
      return;
    }

  output = new_buf ();

  i3bprintlnf (output, "{W%-30s%-10.10s%-25.25s%-15.15s %s", "Name",
	       "Type", "Mudlib", "Address", "Port");
  for (mud = i3mud_first; mud; mud = mud->next)
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

      if (filter[0] && str_prefix (filter, mud->name) &&
	  (mud->mud_type && str_prefix (filter, mud->mud_type)) &&
	  (mud->mudlib && str_prefix (filter, mud->mudlib)))
	continue;

      if (!all && mud->status == 0)
	continue;

      mudcount++;

      switch (mud->status)
	{
	case -1:
	  i3bprintlnf (output, "{c%-30s%-10.10s%-25.25s%-15.15s %d",
		       mud->name, mud->mud_type, mud->mudlib,
		       mud->ipaddress, mud->player_port);
	  break;
	case 0:
	  i3bprintlnf (output, "{R%-26s (down)", mud->name);
	  break;
	default:
	  i3bprintlnf (output, "{Y%-26s (rebooting, back in %d seconds)",
		       mud->name, mud->status);
	  break;
	}
    }
  i3bprintlnf (output, "{W%d total muds listed.", mudcount);
  sendpage (ch, buf_string (output));
  free_buf (output);
  return;
}

Do_Fun (I3_chanlist)
{
  I3_CHANNEL *channel;
  bool all = false, found = false;
  char filter[MIL];
  Buffer *output;

  argument = first_arg (argument, filter, false);

  if (!str_cmp (filter, "all") && I3_is_connected ())
    {
      all = true;
      argument = first_arg (argument, filter, false);
    }

  output = new_buf ();
  i3bprintln (output,
	      "{cLocal name          Perm    I3 Name             Hosted at           Status");
  i3bprintln (output, draw_line (ch, "{c-{w-", 0));
  for (channel = I3chan_first; channel; channel = channel->next)
    {
      found = false;

      if (!all && !channel->local_name && (NullStr (filter)))
	continue;

      if (ch->pcdata->i3chardata->i3perm < I3PERM_ADMIN
	  && !channel->local_name)
	continue;

      if (ch->pcdata->i3chardata->i3perm < channel->i3perm)
	continue;

      if (!all && !NullStr (filter)
	  && str_prefix (filter, channel->I3_name)
	  && str_prefix (filter, channel->host_mud))
	continue;

      if (channel->local_name
	  && hasname (ch->pcdata->i3chardata->i3_listen, channel->local_name))
	found = true;

      i3bprintlnf (output, "{C%c {W%-18s{Y%-8s{B%-20s{M%-20s%-8s",
		   found ? '*' : ' ',
		   channel->local_name ? channel->
		   local_name : "Not configured",
		   i3perm_names[channel->i3perm].name, channel->I3_name,
		   channel->host_mud,
		   channel->status == 0 ? "{GPublic" : "{RPrivate");
    }
  i3bprintln (output, "{C*: You are listening to these channels.");
  sendpage (ch, buf_string (output));
  free_buf (output);
  return;
}

Do_Fun (I3_setup_channel)
{
  Descriptor *d;
  char localname[MIL], I3_name[MIL];
  I3_CHANNEL *channel, *channel2;
  int permvalue = I3PERM_MORT;

  argument = first_arg (argument, I3_name, false);
  argument = first_arg (argument, localname, false);

  if (!(channel = find_I3_channel_by_name (I3_name)))
    {
      i3printlnf (ch, "{YUnknown channel\n\r"
		  "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  if (NullStr (localname))
    {
      if (!channel->local_name)
	{
	  i3printlnf (ch, "Channel %s@%s isn't configured.",
		      channel->I3_name, channel->host_mud);
	  return;
	}

      if (channel->i3perm > ch->pcdata->i3chardata->i3perm)
	{
	  i3printlnf (ch,
		      "You do not have sufficient permission to remove the %s channel.",
		      channel->local_name);
	  return;
	}

      for (d = descriptor_first; d; d = d->next)
	{
	  if (!d->character)
	    continue;

	  if (hasname
	      (d->character->pcdata->i3chardata->i3_listen,
	       channel->local_name))
	    unflagname (&d->character->pcdata->i3chardata->i3_listen,
			channel->local_name);
	  if (hasname
	      (d->character->pcdata->i3chardata->i3_denied,
	       channel->local_name))
	    unflagname (&d->character->pcdata->i3chardata->i3_denied,
			channel->local_name);
	}
      logf ("setup_channel: removing %s as %s@%s", channel->local_name,
	    channel->I3_name, channel->host_mud);
      I3_send_channel_listen (channel, false);
      free_string (channel->local_name);
      rw_i3_channels (act_write);
    }
  else
    {
      if (channel->local_name)
	{
	  i3printlnf (ch, "Channel %s@%s is already known as %s.",
		      channel->I3_name, channel->host_mud,
		      channel->local_name);
	  return;
	}
      if ((channel2 = find_I3_channel_by_localname (localname)))
	{
	  i3printlnf (ch, "Channel %s@%s is already known as %s.",
		      channel2->I3_name, channel2->host_mud,
		      channel2->local_name);
	  return;
	}

      if (!NullStr (argument))
	{
	  permvalue = get_permvalue (argument);
	  if (permvalue < 0 || permvalue > I3PERM_IMP)
	    {
	      i3println (ch, "Invalid permission setting.");
	      return;
	    }
	  if (permvalue > ch->pcdata->i3chardata->i3perm)
	    {
	      i3println (ch,
			 "You cannot assign a permission value above your own.");
	      return;
	    }
	}
      channel->local_name = str_dup (localname);
      channel->i3perm = (i3perm_t) permvalue;
      channel->layout_m = str_dup ("{R[{W%s{R] {C%s@%s: {c%s");
      channel->layout_e = str_dup ("{R[{W%s{R] {c%s");
      i3printlnf (ch, "%s@%s is now locally known as %s",
		  channel->I3_name, channel->host_mud, channel->local_name);
      logf ("setup_channel: setting up %s@%s as %s", channel->I3_name,
	    channel->host_mud, channel->local_name);
      I3_send_channel_listen (channel, true);
      rw_i3_channels (act_write);
    }
}

Do_Fun (I3_edit_channel)
{
  char localname[MIL];
  char arg2[MIL];
  I3_CHANNEL *channel;

  if (NullStr (argument))
    {
      i3println (ch,
		 "Usage: i3 editchan <localname> localname <new localname>");
      i3println (ch, "Usage: i3 editchan <localname> perm <type>");
      return;
    }

  argument = first_arg (argument, localname, false);

  if ((channel = find_I3_channel_by_localname (localname)) == NULL)
    {
      i3println (ch, "{YUnknown local channel\n\r"
		 "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  argument = first_arg (argument, arg2, false);

  if (channel->i3perm > ch->pcdata->i3chardata->i3perm)
    {
      i3println (ch,
		 "You do not have sufficient permissions to edit this channel.");
      return;
    }

  if (!str_cmp (arg2, "localname"))
    {
      i3printlnf (ch, "Local channel %s renamed to %s.",
		  channel->local_name, argument);
      replace_str (&channel->local_name, argument);
      rw_i3_channels (act_write);
      return;
    }

  if (!str_cmp (arg2, "perm") || !str_cmp (arg2, "permission"))
    {
      int permvalue = get_permvalue (argument);

      if (permvalue < 0 || permvalue > I3PERM_IMP)
	{
	  i3println (ch, "Invalid permission setting.");
	  return;
	}
      if (permvalue > ch->pcdata->i3chardata->i3perm)
	{
	  i3println (ch, "You cannot set a permission higher than your own.");
	  return;
	}
      if (channel->i3perm > ch->pcdata->i3chardata->i3perm)
	{
	  i3println (ch,
		     "You cannot edit a channel above your permission level.");
	  return;
	}
      channel->i3perm = (i3perm_t) permvalue;
      i3printlnf (ch, "Local channel %s permission changed to %s.",
		  channel->local_name, argument);
      rw_i3_channels (act_write);
      return;
    }
  I3_edit_channel (n_fun, ch, "");
  return;
}

Do_Fun (I3_chan_who)
{
  char channel_name[MIL];
  I3_CHANNEL *channel;
  I3_MUD *mud;

  argument = first_arg (argument, channel_name, false);

  if (NullStr (channel_name) || NullStr (argument))
    {
      i3println (ch, "Usage: i3chanwho <local channel> <mud>");
      return;
    }

  if ((channel = find_I3_channel_by_localname (channel_name)) == NULL)
    {
      i3println (ch, "{YUnknown channel.\n\r"
		 "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  if (!(mud = find_I3_mud_by_name (argument)))
    {
      i3println (ch, "{YUnknown mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (mud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", mud->name);
      return;
    }

  I3_send_chan_who (ch, channel, mud);
}

Do_Fun (I3_listen_channel)
{
  I3_CHANNEL *channel;

  if (NullStr (argument))
    {
      i3println (ch, "Usage: i3listen <local channel name>");
      return;
    }

  if (!str_cmp (argument, "all"))
    {
      for (channel = I3chan_first; channel; channel = channel->next)
	{
	  if (NullStr (channel->local_name))
	    continue;

	  if (ch->pcdata->i3chardata->i3perm >= channel->i3perm
	      && !hasname (ch->pcdata->i3chardata->i3_listen,
			   channel->local_name))
	    flagname (&ch->pcdata->i3chardata->i3_listen,
		      channel->local_name);
	}
      i3println (ch, "{YYou are now listening to all available I3 channels.");
      return;
    }

  if (!str_cmp (argument, "none"))
    {
      for (channel = I3chan_first; channel; channel = channel->next)
	{
	  if (NullStr (channel->local_name))
	    continue;

	  if (hasname
	      (ch->pcdata->i3chardata->i3_listen, channel->local_name))
	    unflagname (&ch->pcdata->i3chardata->i3_listen,
			channel->local_name);
	}
      i3println (ch, "{YYou no longer listen to any available I3 channels.");
      return;
    }

  if ((channel = find_I3_channel_by_localname (argument)) == NULL)
    {
      i3println (ch, "{YUnknown channel.\n\r"
		 "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  if (hasname (ch->pcdata->i3chardata->i3_listen, channel->local_name))
    {
      i3printlnf (ch, "You no longer listen to %s", channel->local_name);
      unflagname (&ch->pcdata->i3chardata->i3_listen, channel->local_name);
    }
  else
    {
      if (ch->pcdata->i3chardata->i3perm < channel->i3perm)
	{
	  i3printlnf (ch, "Channel %s is above your permission level.",
		      channel->local_name);
	  return;
	}
      i3printlnf (ch, "You now listen to %s", channel->local_name);
      flagname (&ch->pcdata->i3chardata->i3_listen, channel->local_name);
    }
  return;
}

Do_Fun (I3_deny_channel)
{
  char vic_name[MIL];
  CharData *victim;
  I3_CHANNEL *channel;

  argument = first_arg (argument, vic_name, false);

  if (NullStr (vic_name) || NullStr (argument))
    {
      i3println (ch, "Usage: i3deny <person> <local channel name>");
      i3println (ch, "Usage: i3deny <person> [tell/beep/finger]");
      return;
    }

  if (!(victim = I3_find_user (vic_name)))
    {
      i3println (ch, "No such person is currently online.");
      return;
    }

  if (ch->pcdata->i3chardata->i3perm <= victim->pcdata->i3chardata->i3perm)
    {
      i3println (ch, "You cannot alter their settings.");
      return;
    }

  if (!str_cmp (argument, "tell"))
    {
      if (!IsSet (victim->pcdata->i3chardata->i3flags, I3_DENYTELL))
	{
	  SetBit (victim->pcdata->i3chardata->i3flags, I3_DENYTELL);
	  i3printlnf (ch, "%s can no longer use i3tells.", GetName (victim));
	  return;
	}
      RemBit (victim->pcdata->i3chardata->i3flags, I3_DENYTELL);
      i3printlnf (ch, "%s can use i3tells again.", GetName (victim));
      return;
    }

  if (!str_cmp (argument, "beep"))
    {
      if (!IsSet (victim->pcdata->i3chardata->i3flags, I3_DENYBEEP))
	{
	  SetBit (victim->pcdata->i3chardata->i3flags, I3_DENYBEEP);
	  i3printlnf (ch, "%s can no longer use i3beeps.", GetName (victim));
	  return;
	}
      RemBit (victim->pcdata->i3chardata->i3flags, I3_DENYBEEP);
      i3printlnf (ch, "%s can use i3beeps again.", GetName (victim));
      return;
    }

  if (!str_cmp (argument, "finger"))
    {
      if (!IsSet (victim->pcdata->i3chardata->i3flags, I3_DENYFINGER))
	{
	  SetBit (victim->pcdata->i3chardata->i3flags, I3_DENYFINGER);
	  i3printlnf (ch, "%s can no longer use i3fingers.",
		      GetName (victim));
	  return;
	}
      RemBit (victim->pcdata->i3chardata->i3flags, I3_DENYFINGER);
      i3printlnf (ch, "%s can use i3fingers again.", GetName (victim));
      return;
    }

  if (!(channel = find_I3_channel_by_localname (argument)))
    {
      i3println (ch, "{YUnknown channel.\n\r"
		 "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  if (hasname (ch->pcdata->i3chardata->i3_denied, channel->local_name))
    {
      i3printlnf (ch, "%s can now listen to %s", GetName (victim),
		  channel->local_name);
      unflagname (&ch->pcdata->i3chardata->i3_denied, channel->local_name);
    }
  else
    {
      i3printlnf (ch, "%s can no longer listen to %s", GetName (victim),
		  channel->local_name);
      flagname (&ch->pcdata->i3chardata->i3_denied, channel->local_name);
    }
  return;
}

void
free_i3chardata (PcData * pcdata)
{
  I3_IGNORE *temp, *next;

  if (!pcdata)
    return;

  free_string (pcdata->i3chardata->i3_listen);
  free_string (pcdata->i3chardata->i3_denied);
  free_string (pcdata->i3chardata->i3_replyname);

  if (pcdata->i3chardata->i3ignore_first)
    {
      for (temp = pcdata->i3chardata->i3ignore_first; temp; temp = next)
	{
	  next = temp->next;
	  free_string (temp->name);
	  UnLink (temp, pcdata->i3chardata->i3ignore, next, prev);
	  free_mem (temp);
	}
    }
  free_mem (pcdata->i3chardata);
  return;
}

Do_Fun (I3_mudinfo)
{
  I3_MUD *mud;

  if (NullStr (argument))
    {
      i3println (ch, "{YWhich mud do you want information about?\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!(mud = find_I3_mud_by_name (argument)))
    {
      i3println (ch, "{YUnknown mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  i3printf (ch, "Information about %s\n\r" NEWLINE, mud->name);
  if (mud->status == 0)
    i3println (ch, "Status     : Currently down");
  else if (mud->status > 0)
    i3printlnf (ch,
		"Status     : Currently rebooting, back in %d seconds",
		mud->status);
  i3printlnf (ch, "MUD port   : %s %d", mud->ipaddress, mud->player_port);
  i3printlnf (ch, "Base mudlib: %s", mud->base_mudlib);
  i3printlnf (ch, "Mudlib     : %s", mud->mudlib);
  i3printlnf (ch, "Driver     : %s", mud->driver);
  i3printlnf (ch, "Type       : %s", mud->mud_type);
  i3printlnf (ch, "Open status: %s", mud->open_status);
  i3printlnf (ch, "Admin      : %s", mud->admin_email);
  if (mud->web)
    {
#ifndef DISABLE_WEBSRV
      if (web_is_connected ())
	i3printlnf (ch, "URL        : %s:%d", mud->web, webport);
      else
#endif
	i3printlnf (ch, "URL        : %s", mud->web);
    }
  if (mud->web_wrong && !mud->web)
    i3printlnf (ch, "URL        : %s", mud->web_wrong);
  if (mud->daemon)
    i3printlnf (ch, "Daemon     : %s", mud->daemon);
  if (mud->time)
    i3printlnf (ch, "Time       : %s", mud->time);
  if (mud->banner)
    i3printlnf (ch, "Banner:" NEWLINE "%s", mud->banner);

  i3print (ch, "Supports   : ");
  if (mud->tell)
    i3print (ch, "tell, ");
  if (mud->beep)
    i3print (ch, "beep, ");
  if (mud->emoteto)
    i3print (ch, "emoteto, ");
  if (mud->who)
    i3print (ch, "who, ");
  if (mud->finger)
    i3print (ch, "finger, ");
  if (mud->locate)
    i3print (ch, "locate, ");
  if (mud->channel)
    i3print (ch, "channel, ");
  if (mud->news)
    i3print (ch, "news, ");
  if (mud->mail)
    i3print (ch, "mail, ");
  if (mud->file)
    i3print (ch, "file, ");
  if (mud->auth)
    i3print (ch, "auth, ");
  if (mud->ucache)
    i3print (ch, "ucache, ");
  i3println (ch, "");

  i3print (ch, "Supports   : ");
  if (mud->smtp)
    i3printf (ch, "smtp (port %d), ", mud->smtp);
  if (mud->http)
    i3printf (ch, "http (port %d), ", mud->http);
  if (mud->ftp)
    i3printf (ch, "ftp  (port %d), ", mud->ftp);
  if (mud->pop3)
    i3printf (ch, "pop3 (port %d), ", mud->pop3);
  if (mud->nntp)
    i3printf (ch, "nntp (port %d), ", mud->nntp);
  if (mud->rcp)
    i3printf (ch, "rcp  (port %d), ", mud->rcp);
  if (mud->amrcp)
    i3printf (ch, "amrcp (port %d), ", mud->amrcp);
  i3println (ch, "");
}

Do_Fun (I3_chanlayout)
{
  I3_CHANNEL *channel = NULL;
  char arg1[MIL];
  char arg2[MIL];

  if (NullStr (argument))
    {
      i3println (ch,
		 "Usage: i3chanlayout <localchannel> <layout> <format...>");
      i3println (ch, "Layout can be one of these: layout_e layout_m");
      i3println (ch,
		 "Format can be any way you want it to look, provided you have the proper number of %s tags in it.");
      return;
    }

  argument = first_arg (argument, arg1, false);
  argument = first_arg (argument, arg2, false);

  if (NullStr (arg1))
    {
      I3_chanlayout (n_fun, ch, "");
      return;
    }
  if (NullStr (arg2))
    {
      I3_chanlayout (n_fun, ch, "");
      return;
    }
  if (NullStr (argument))
    {
      I3_chanlayout (n_fun, ch, "");
      return;
    }

  if (!(channel = find_I3_channel_by_localname (arg1)))
    {
      i3println (ch, "{YUnknown channel.\n\r"
		 "(use {Wi3chanlist{Y to get an overview of the channels available)");
      return;
    }

  if (!str_cmp (arg2, "layout_e"))
    {
      if (!verify_i3layout (argument, 2))
	{
	  i3println (ch,
		     "Incorrect format for layout_e. You need exactly 2 %s's.");
	  return;
	}
      replace_str (&channel->layout_e, argument);
      i3println (ch, "Channel layout_e changed.");
      rw_i3_channels (act_write);
      return;
    }

  if (!str_cmp (arg2, "layout_m"))
    {
      if (!verify_i3layout (argument, 4))
	{
	  i3println (ch,
		     "Incorrect format for layout_m. You need exactly 4 %s's.");
	  return;
	}
      replace_str (&channel->layout_m, argument);
      i3println (ch, "Channel layout_m changed.");
      rw_i3_channels (act_write);
      return;
    }
  I3_chanlayout (n_fun, ch, "");
  return;
}


void
free_i3data (bool complete)
{
  I3_MUD *mud, *next_mud;
  I3_CHANNEL *channel, *next_chan;
  I3_BAN *ban, *next_ban;
  I3_UCACHE *ucache, *next_ucache;
  ROUTER_DATA *router, *router_next;
  I3_COLOR *color, *color_next;
  I3_CMD_DATA *cmd, *cmd_next;
  I3_HELP_DATA *help, *help_next;

  if (I3ban_first)
    {
      for (ban = I3ban_first; ban; ban = next_ban)
	{
	  next_ban = ban->next;
	  free_string (ban->name);
	  UnLink (ban, I3ban, next, prev);
	  free_mem (ban);
	}
    }

  if (I3chan_first)
    {
      for (channel = I3chan_first; channel; channel = next_chan)
	{
	  next_chan = channel->next;
	  destroy_I3_channel (channel);
	}
    }

  if (i3mud_first)
    {
      for (mud = i3mud_first; mud; mud = next_mud)
	{
	  next_mud = mud->next;
	  destroy_I3_mud (mud);
	}
    }

  if (ucache_first)
    {
      for (ucache = ucache_first; ucache; ucache = next_ucache)
	{
	  next_ucache = ucache->next;
	  free_string (ucache->name);
	  UnLink (ucache, ucache, next, prev);
	  free_mem (ucache);
	}
    }

  if (complete)
    {
      if (router_first)
	{
	  for (router = router_first; router; router = router_next)
	    {
	      router_next = router->next;
	      free_string (router->name);
	      free_string (router->ip);
	      UnLink (router, router, next, prev);
	      free_mem (router);
	    }
	}

      for (cmd = i3cmd_first; cmd; cmd = cmd_next)
	{
	  NameList *alias, *alias_next;

	  cmd_next = cmd->next;
	  free_string (cmd->name);
	  free_string (cmd->args);
	  free_string (cmd->descr);
	  UnLink (cmd, i3cmd, next, prev);
	  for (alias = cmd->alias_first; alias; alias = alias_next)
	    {
	      alias_next = alias->next;
	      free_namelist (alias);
	    }
	  free_mem (cmd);
	}

      for (help = i3help_first; help; help = help_next)
	{
	  help_next = help->next;
	  free_string (help->name);
	  free_string (help->text);
	  UnLink (help, i3help, next, prev);
	  free_mem (help);
	}

      for (color = i3color_first; color; color = color_next)
	{
	  color_next = color->next;
	  free_string (color->name);
	  free_string (color->mudtag);
	  free_string (color->i3tag);
	  free_string (color->i3fish);
	  UnLink (color, i3color, next, prev);
	  free_mem (color);
	}
    }
  return;
}

Do_Fun (I3_disconnect)
{
  if (!I3_is_connected ())
    {
      i3println (ch, "The MUD isn't connected to the Intermud-3 router.");
      return;
    }

  i3println (ch, "Disconnecting from Intermud-3 router.");

  I3_shutdown (0);
  return;
}

Do_Fun (I3_bancmd)
{
  I3_BAN *temp;

  if (NullStr (argument))
    {
      i3println (ch, "{GThe mud currently has the following ban list:\n\r");

      if (!I3ban_first)
	{
	  i3println (ch, "{YNothing\n\r");
	  i3println (ch,
		     "{YTo add a ban, just specify a target. Suggested targets being user@mud or IP:Port");
	  i3println (ch,
		     "{YUser@mud bans can also have wildcard specifiers, such as *@Mud or User@*");
	  return;
	}
      for (temp = I3ban_first; temp; temp = temp->next)
	i3printlnf (ch, "{Y\t  - %s", temp->name);
      return;
    }

  if (!fnmatch (argument, I3_THISMUD, 0))
    {
      i3println (ch, "{YYou don't really want to do that....");
      return;
    }

  for (temp = I3ban_first; temp; temp = temp->next)
    {
      if (!str_cmp (temp->name, argument))
	{
	  free_string (temp->name);
	  UnLink (temp, I3ban, next, prev);
	  free_mem (temp);
	  rw_i3_bans (act_write);
	  i3printlnf (ch, "{YThe mud no longer bans %s.", argument);
	  return;
	}
    }

  alloc_mem (temp, I3_BAN, 1);
  temp->name = str_dup (argument);
  Link (temp, I3ban, next, prev);
  rw_i3_bans (act_write);
  i3printlnf (ch, "{YThe mud now bans all incoming traffic from %s.",
	      temp->name);
}

Do_Fun (I3_ignorecmd)
{
  I3_IGNORE *temp;
  char buf[MIL];

  if (NullStr (argument))
    {
      i3println (ch, "{GYou are currently ignoring the following:\n\r");

      if (!ch->pcdata->i3chardata->i3ignore_first)
	{
	  i3print (ch, "{YNobody" NEWLINE);
	  i3println (ch,
		     "{YTo add an ignore, just specify a target. Suggested targets being user@mud or IP:Port");
	  i3println (ch,
		     "{YUser@mud ignores can also have wildcard specifiers, such as *@Mud or User@*");
	  return;
	}
      for (temp = ch->pcdata->i3chardata->i3ignore_first; temp;
	   temp = temp->next)
	i3printlnf (ch, "{Y\t  - %s", temp->name);

      return;
    }

  snprintf (buf, sizeof (buf), "%s@%s", GetName (ch), I3_THISMUD);
  if (!str_cmp (buf, argument))
    {
      i3println (ch, "{YYou don't really want to do that....");
      return;
    }

  if (!fnmatch (argument, I3_THISMUD, 0))
    {
      i3println (ch, "{YIgnoring your own mud would be silly.");
      return;
    }

  for (temp = ch->pcdata->i3chardata->i3ignore_first; temp; temp = temp->next)
    {
      if (!str_cmp (temp->name, argument))
	{
	  free_string (temp->name);
	  UnLink (temp, ch->pcdata->i3chardata->i3ignore, next, prev);
	  free_mem (temp);
	  i3printlnf (ch, "{YYou are no longer ignoring %s.", argument);
	  return;
	}
    }

  alloc_mem (temp, I3_IGNORE, 1);
  temp->name = str_dup (argument);
  Link (temp, ch->pcdata->i3chardata->i3ignore, next, prev);
  i3printlnf (ch, "{YYou now ignore %s.", temp->name);
}


Do_Fun (I3_afk)
{
  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_AFK))
    {
      RemBit (ch->pcdata->i3chardata->i3flags, I3_AFK);
      i3println (ch, "You are no longer AFK to I3.");
    }
  else
    {
      SetBit (ch->pcdata->i3chardata->i3flags, I3_AFK);
      i3println (ch, "You are now AFK to I3.");
    }
  return;
}

Do_Fun (I3_color)
{
  if (ch->pcdata->i3chardata->i3perm >= I3PERM_IMM
      && !str_cmp (argument, "save"))
    {
      i3println (ch, "I3 color file saved.");
      rw_i3_colors (act_write);
      return;
    }

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_COLORFLAG))
    {
      RemBit (ch->pcdata->i3chardata->i3flags, I3_COLORFLAG);
      i3println (ch, "I3 color is now off.");
    }
  else
    {
      SetBit (ch->pcdata->i3chardata->i3flags, I3_COLORFLAG);
      i3println (ch, "{RI3 c{Yo{Gl{Bo{Mr {Ris now on.");
    }
  return;
}

Do_Fun (i3_cedit)
{
  I3_CMD_DATA *cmd, *tmp;
  NameList *alias, *alias_next;
  char name[MIL], option[MIL];
  bool found = false, aliasfound = false;

  argument = one_argument (argument, name);
  argument = one_argument (argument, option);

  if (NullStr (name) || NullStr (option))
    {
      i3println (ch,
		 "Usage: i3cedit <command> <create|delete|alias|rename|args|descr|code|permission|connected> <field>.");
      return;
    }

  for (cmd = i3cmd_first; cmd; cmd = cmd->next)
    {
      if (!str_cmp (cmd->name, name))
	{
	  found = true;
	  break;
	}
      for (alias = cmd->alias_first; alias; alias = alias->next)
	{
	  if (!str_cmp (alias->name, name))
	    aliasfound = true;
	}
    }

  if (!str_cmp (option, "create"))
    {
      if (found)
	{
	  i3printlnf (ch, "{gA command named {W%s {galready exists.", name);
	  return;
	}

      if (aliasfound)
	{
	  i3printlnf (ch,
		      "{g%s already exists as an alias for another command.",
		      name);
	  return;
	}

      alloc_mem (cmd, I3_CMD_DATA, 1);
      cmd->name = str_dup (name);
      cmd->args = &str_empty[0];
      cmd->descr = &str_empty[0];
      cmd->level = ch->pcdata->i3chardata->i3perm;
      cmd->connected = false;
      i3printlnf (ch, "{gCommand {W%s {gcreated.", cmd->name);
      if (NullStr (argument))
	{
	  cmd->function = i3_function (argument);
	  if (cmd->function == NULL)
	    i3printlnf (ch,
			"{gFunction {W%s {gdoes not exist - set to NULL.",
			argument);
	}
      else
	{
	  i3println (ch, "{gFunction set to NULL.");
	  cmd->function = NULL;
	}
      Link (cmd, i3cmd, next, prev);
      rw_i3_cmds (act_write);
      return;
    }

  if (!found)
    {
      i3printlnf (ch, "{gNo command named {W%s {gexists.", name);
      return;
    }

  if (!i3check_permissions (ch, cmd->level, cmd->level, false))
    return;

  if (!str_cmp (option, "delete"))
    {
      i3printlnf (ch, "{gCommand {W%s {ghas been deleted.", cmd->name);
      for (alias = cmd->alias_first; alias; alias = alias_next)
	{
	  alias_next = alias->next;

	  free_namelist (alias);
	}
      UnLink (cmd, i3cmd, next, prev);
      free_string (cmd->name);
      free_string (cmd->args);
      free_string (cmd->descr);
      free_mem (cmd);
      rw_i3_cmds (act_write);
      return;
    }


  if (!str_cmp (option, "alias"))
    {
      for (alias = cmd->alias_first; alias; alias = alias_next)
	{
	  alias_next = alias->next;

	  if (!str_cmp (alias->name, argument))
	    {
	      i3printlnf (ch, "{W%s {ghas been removed as an alias for {W%s",
			  argument, cmd->name);
	      UnLink (alias, cmd->alias, next, prev);
	      free_namelist (alias);
	      rw_i3_cmds (act_write);
	      return;
	    }
	}

      for (tmp = i3cmd_first; tmp; tmp = tmp->next)
	{
	  if (!str_cmp (tmp->name, argument))
	    {
	      i3printlnf (ch, "{W%s {gis already a command name.", argument);
	      return;
	    }
	  for (alias = tmp->alias_first; alias; alias = alias->next)
	    {
	      if (!str_cmp (argument, alias->name))
		{
		  i3printlnf (ch, "{W%s {gis already an alias for {W%s",
			      argument, tmp->name);
		  return;
		}
	    }
	}

      alias = new_namelist ();
      alias->name = str_dup (argument);
      Link (alias, cmd->alias, next, prev);
      i3printlnf (ch, "{W%s {ghas been added as an alias for {W%s",
		  alias->name, cmd->name);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "connected"))
    {
      cmd->connected = !cmd->connected;

      if (cmd->connected)
	i3printlnf (ch,
		    "{gCommand {W%s {gwill now require a connection to I3 to use.",
		    cmd->name);
      else
	i3printlnf (ch,
		    "{gCommand {W%s {gwill no longer require a connection to I3 to use.",
		    cmd->name);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "show"))
    {
      char buf[MSL];

      i3printlnf (ch, "{gCommand       : {W%s", cmd->name);
      i3printlnf (ch, "{gArgs          : {W%s", cmd->args);
      i3printlnf (ch, "{gDescr         : {W%s", cmd->descr);
      i3printlnf (ch, "{gPermission    : {W%s",
		  i3perm_names[cmd->level].name);
      i3printlnf (ch, "{gFunction      : {W%s", i3_funcname (cmd->function));
      i3printlnf (ch, "{gConnection Req: {W%s",
		  cmd->connected ? "Yes" : "No");
      if (cmd->alias_first)
	{
	  int col = 0;

	  strlcpy (buf, "{gAliases       : {W", sizeof (buf));
	  for (alias = cmd->alias_first; alias; alias = alias->next)
	    {
	      snprintf (buf + strlen (buf), sizeof (buf) - strlen (buf),
			"%s ", alias->name);
	      if (++col % 10 == 0)
		strlcat (buf, "\n\r", sizeof (buf));
	    }
	  if (col % 10 != 0)
	    strlcat (buf, "\n\r", sizeof (buf));
	  i3print (ch, buf);
	}
      return;
    }

  if (NullStr (argument))
    {
      i3println (ch, "Required argument missing.");
      i3_cedit (n_fun, ch, "");
      return;
    }

  if (!str_cmp (option, "rename"))
    {
      i3printlnf (ch, "{gCommand {W%s {ghas been renamed to {W%s.",
		  cmd->name, argument);
      free_string (cmd->name);
      cmd->name = str_dup (argument);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "args"))
    {
      i3printlnf (ch, "{gCommand {W%s {garg list has been changed to {W%s.",
		  cmd->name, argument);
      replace_str (&cmd->args, argument);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "descr"))
    {
      i3printlnf (ch,
		  "{gCommand {W%s {gdescription has been changed to {W%s.",
		  cmd->name, argument);
      replace_str (&cmd->descr, argument);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "code"))
    {
      cmd->function = i3_function (argument);
      if (cmd->function == NULL)
	i3printlnf (ch, "{gFunction {W%s {gdoes not exist - set to NULL.",
		    argument);
      else
	i3printlnf (ch, "{gFunction set to {W%s.", argument);
      rw_i3_cmds (act_write);
      return;
    }

  if (!str_cmp (option, "perm") || !str_cmp (option, "permission"))
    {
      int permvalue = get_permvalue (argument);

      if (!i3check_permissions (ch, permvalue, cmd->level, false))
	return;

      cmd->level = (i3perm_t) permvalue;
      i3printlnf (ch,
		  "{gCommand {W%s {gpermission level has been changed to {W%s.",
		  cmd->name, i3perm_names[permvalue].name);
      rw_i3_cmds (act_write);
      return;
    }
  i3_cedit (n_fun, ch, "");
  return;
}

Do_Fun (I3_invis)
{
  if (I3INVIS (ch))
    {
      RemBit (ch->pcdata->i3chardata->i3flags, I3_INVIS);
      i3println (ch, "You are now i3visible.");
    }
  else
    {
      SetBit (ch->pcdata->i3chardata->i3flags, I3_INVIS);
      i3println (ch, "You are now i3invisible.");
    }
  return;
}

Do_Fun (I3_debug)
{
  packetdebug = !packetdebug;

  if (packetdebug)
    i3println (ch, "Packet debugging enabled.");
  else
    i3println (ch, "Packet debugging disabled.");

  return;
}

Do_Fun (I3_send_user_req)
{
  char user[MIL], mud[MIL];
  char *ps;
  I3_MUD *pmud;

  if (NullStr (argument))
    {
      i3println (ch, "{YQuery who at which mud?\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }
  if (!(ps = strchr (argument, '@')))
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  ps[0] = '\0';
  strlcpy (user, argument, MIL);
  strlcpy (mud, ps + 1, MIL);

  if (NullStr (user) || NullStr (mud))
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!(pmud = find_I3_mud_by_name (mud)))
    {
      i3println (ch, "{YNo such mud known.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (pmud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", pmud->name);
      return;
    }

  I3_send_chan_user_req (pmud->name, user);
  return;
}

void
I3_process_channel_adminlist_reply (I3_HEADER * header, char *s)
{
  char *ps = s, *next_ps;
  I3_CHANNEL *channel;
  CharData *ch;

  if ((ch = I3_find_user (header->target_username)) == NULL)
    {
      bugf ("I3_process_channel_adminlist_reply(): user %s not found.",
	    header->target_username);
      return;
    }

  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  if (!(channel = find_I3_channel_by_name (ps)))
    {
      bugf
	("I3_process_channel_adminlist_reply(): Invalid local channel %s reply received.",
	 ps);
      return;
    }
  i3printlnf (ch, "{RThe following muds are %s %s:" NEWLINE,
	      channel->status == 0 ? "banned from" : "invited to",
	      channel->local_name);

  ps = next_ps;
  I3_get_field (ps, &next_ps);
  ps += 2;
  while (1)
    {
      if (ps[0] == '}')
	{
	  i3println (ch, "{YNo entries found.");
	  return;
	}

      I3_get_field (ps, &next_ps);
      I3_remove_quotes (&ps);
      i3printlnf (ch, "{Y%s", ps);

      ps = next_ps;
      if (ps[0] == '}')
	break;
    }
  return;
}

void
I3_send_channel_adminlist (CharData * ch, const char *chan_name)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("chan-adminlist", I3_THISMUD, GetName (ch),
		   I3_ROUTER_NAME, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (chan_name);
  I3_write_buffer ("\",})\r");
  I3_send_packet ();

  return;
}

void
I3_send_channel_admin (CharData * ch, const char *chan_name, const char *list)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-admin", I3_THISMUD, GetName (ch),
		   I3_ROUTER_NAME, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (chan_name);
  I3_write_buffer ("\",");
  I3_write_buffer (list);
  I3_write_buffer ("})\r");
  I3_send_packet ();

  return;
}

Do_Fun (I3_admin_channel)
{
  I3_CHANNEL *channel = NULL;
  char arg1[MIL], arg2[MIL], buf[MSL];

  if (NullStr (argument))
    {
      i3println (ch,
		 "Usage: i3adminchan <localchannel> <add|remove> <mudname>");
      i3println (ch, "Usage: i3adminchan <localchannel> list");
      return;
    }
  argument = first_arg (argument, arg1, false);
  argument = first_arg (argument, arg2, false);

  if (NullStr (arg1))
    {
      I3_admin_channel (n_fun, ch, "");
      return;
    }

  if (!(channel = find_I3_channel_by_localname (arg1)))
    {
      i3println (ch, "No such channel with that name here.");
      return;
    }

  if (NullStr (arg2))
    {
      I3_admin_channel (n_fun, ch, "");
      return;
    }

  if (!str_cmp (arg2, "list"))
    {
      I3_send_channel_adminlist (ch, channel->I3_name);
      i3println (ch, "Sending request for administrative list.");
      return;
    }

  if (NullStr (argument))
    {
      I3_admin_channel (n_fun, ch, "");
      return;
    }

  if (!str_cmp (arg2, "add"))
    {
      snprintf (buf, sizeof (buf), "({\"%s\",}),({}),", argument);
      I3_send_channel_admin (ch, channel->I3_name, buf);
      i3println (ch, "Sending administrative list addition.");
      return;
    }

  if (!str_cmp (arg2, "remove"))
    {
      snprintf (buf, sizeof (buf), "({}),({\"%s\",}),", argument);
      I3_send_channel_admin (ch, channel->I3_name, buf);
      i3println (ch, "Sending administrative list removal.");
      return;
    }
  I3_admin_channel (n_fun, ch, "");
  return;
}

void
I3_send_channel_add (CharData * ch, char *arg, int type)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-add", I3_THISMUD, GetName (ch), I3_ROUTER_NAME,
		   NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (arg);
  I3_write_buffer ("\",");
  switch (type)
    {
    default:
      bug ("I3_send_channel_add: Illegal channel type!");
      return;
    case 0:
      I3_write_buffer ("0,})\r");
      break;
    case 1:
      I3_write_buffer ("1,})\r");
      break;
    case 2:
      I3_write_buffer ("2,})\r");
      break;
    }
  I3_send_packet ();
  return;
}

Do_Fun (I3_addchan)
{
  I3_CHANNEL *channel;
  char arg[MIL], arg2[MIL], buf[MSL];
  int type, x;

  argument = first_arg (argument, arg, false);
  argument = first_arg (argument, arg2, false);

  if (NullStr (argument) || NullStr (arg) || NullStr (arg2))
    {
      i3print (ch,
	       "Usage: i3addchan <channelname> <localname> <type>" NEWLINE);
      i3println (ch, "Channelname should be the name seen on 'chanlist all'");
      i3println (ch,
		 "Localname should be the local name you want it listed as.");
      i3print (ch, "Type can be one of the following:" NEWLINE);
      i3println (ch, "0: selectively banned");
      i3println (ch, "1: selectively admitted");
      i3println (ch, "2: filtered - valid for selectively admitted ONLY");
      return;
    }

  if ((channel = find_I3_channel_by_name (arg)) != NULL)
    {
      i3printlnf (ch, "{R%s is already hosted by %s.", channel->I3_name,
		  channel->host_mud);
      return;
    }

  if ((channel = find_I3_channel_by_localname (arg2)) != NULL)
    {
      i3printlnf (ch,
		  "{RChannel %s@%s is already locally configured as %s.",
		  channel->I3_name, channel->host_mud, channel->local_name);
      return;
    }

  if (!isdigit (argument[0]))
    {
      i3println (ch, "{RInvalid type. Must be numerical.");
      I3_addchan (n_fun, ch, "");
      return;
    }

  type = atoi (argument);
  if (type < 0 || type > 2)
    {
      i3println (ch, "{RInvalid channel type.");
      I3_addchan (n_fun, ch, "");
      return;
    }

  i3printlnf (ch, "{GAdding channel to router: {W%s", arg);
  I3_send_channel_add (ch, arg, type);

  channel = new_I3chan ();
  channel->I3_name = str_dup (arg);
  channel->host_mud = str_dup (I3_THISMUD);
  channel->local_name = str_dup (arg2);
  channel->i3perm = I3PERM_ADMIN;
  channel->layout_m = str_dup ("{R[{W%s{R] {C%s@%s: {c%s");
  channel->layout_e = str_dup ("{R[{W%s{R] {c%s");
  for (x = 0; x < MAX_I3HISTORY; x++)
    channel->history[x] = NULL;
  Link (channel, I3chan, next, prev);

  if (type != 0)
    {
      snprintf (buf, sizeof (buf), "({\"%s\",}),({}),", I3_THISMUD);
      I3_send_channel_admin (ch, channel->I3_name, buf);
      i3printlnf (ch, "{GSending command to add %s to the invite list.",
		  I3_THISMUD);
    }

  i3printlnf (ch, "{Y%s@%s {Wis now locally known as {Y%s",
	      channel->I3_name, channel->host_mud, channel->local_name);
  I3_send_channel_listen (channel, true);
  rw_i3_channels (act_write);

  return;
}

void
I3_send_channel_remove (CharData * ch, I3_CHANNEL * channel)
{
  if (!I3_is_connected ())
    return;

  I3_write_header ("channel-remove", I3_THISMUD, GetName (ch),
		   I3_ROUTER_NAME, NULL);
  I3_write_buffer ("\"");
  I3_write_buffer (channel->I3_name);
  I3_write_buffer ("\",})\r");
  I3_send_packet ();
  return;
}

Do_Fun (I3_removechan)
{
  I3_CHANNEL *channel = NULL;

  if (NullStr (argument))
    {
      i3println (ch, "{YUsage: i3removechan <channel>");
      i3println (ch,
		 "{WChannelname should be the name seen on 'chanlist all'");
      return;
    }

  if ((channel = find_I3_channel_by_name (argument)) == NULL)
    {
      i3println (ch, "{RNo channel by that name exists.");
      return;
    }

  if (str_cmp (channel->host_mud, I3_THISMUD))
    {
      i3printlnf (ch,
		  "{R%s does not host this channel and cannot remove it.",
		  I3_THISMUD);
      return;
    }

  i3printlnf (ch, "{YRemoving channel from router: {W%s", channel->I3_name);
  I3_send_channel_remove (ch, channel);

  i3printlnf (ch, "{RDestroying local channel entry for {W%s",
	      channel->I3_name);
  destroy_I3_channel (channel);
  rw_i3_channels (act_write);

  return;
}

Do_Fun (I3_setconfig)
{
  char arg[MIL];

  argument = first_arg (argument, arg, false);

  if (NullStr (arg))
    {
      i3println (ch,
		 "{GConfiguration info for your mud. Changes save when edited.");
      i3print (ch, "{GYou can set the following:" NEWLINE);
      i3println (ch,
		 "{wShow       : {GDisplays your current congfiguration.");
      i3println (ch,
		 "{wAutoconnect: {GA toggle. Either on or off. Your mud will connect automatically with it on.");
      i3println (ch,
		 "{wMudname    : {GThe name you want displayed on I3 for your mud.");
      i3println (ch,
		 "{wTelnet     : {GThe telnet address for your mud. Do not include the port number.");
      i3println (ch,
		 "{wWeb        : {GThe website address for your mud. In the form of: www.address.com");
      i3println (ch,
		 "{wEmail      : {GThe email address of your mud's administrator. Needs to be valid!!");
      i3println (ch,
		 "{wStatus     : {GThe open status of your mud. IE: Public, Development, etc.");
      i3println (ch,
		 "{wMudtype    : {GWhat type of mud you have. Diku, Rom, Smaug, Merc, etc.");
      i3println (ch,
		 "{wBaselib    : {GThe base version of the codebase you have.");
      i3println (ch,
		 "{wMudlib     : {GWhat you call the current version of your codebase.");
      i3println (ch,
		 "{wMinlevel   : {GMinimum level at which I3 will recognize your players.");
      i3println (ch,
		 "{wImmlevel   : {GThe level at which immortal commands become available.");
      i3println (ch,
		 "{wAdminlevel : {GThe level at which administrative commands become available.");
      i3println (ch,
		 "{wImplevel   : {GThe level at which implementor commands become available.");
      return;
    }

  if (!str_cmp (arg, "show"))
    {
      i3printlnf (ch, "{wMudname       {R: {G%s", i3mud_info.name);
      i3printlnf (ch, "{wAutoconnect   : {G%s",
		  i3mud_info.autoconnect == true ? "Enabled" : "Disabled");
      i3printlnf (ch, "{wTelnet        : {G%s:%d", i3mud_info.telnet,
		  i3mud_info.player_port);
#ifndef DISABLE_WEBSRV
      if (web_is_connected ())
	i3printlnf (ch, "{wWeb           : {G%s:%d", i3mud_info.web, webport);
      else
#endif
	i3printlnf (ch, "{wWeb           : {G%s", i3mud_info.web);
      i3printlnf (ch, "{wEmail         : {G%s", i3mud_info.admin_email);
      i3printlnf (ch, "{wStatus        : {G%s", i3mud_info.open_status);
      i3printlnf (ch, "{wMudtype       : {G%s", i3mud_info.mud_type);
      i3printlnf (ch, "{wBaselib       {R: {G%s", i3mud_info.base_mudlib);
      i3printlnf (ch, "{wMudlib        : {G%s", i3mud_info.mudlib);
      i3printlnf (ch, "{wMinlevel      : {G%d", i3mud_info.minlevel);
      i3printlnf (ch, "{wImmlevel      : {G%d", i3mud_info.immlevel);
      i3printlnf (ch, "{wAdminlevel    : {G%d", i3mud_info.adminlevel);
      i3printlnf (ch, "{wImplevel      : {G%d", i3mud_info.implevel);
      return;
    }

  if (!str_cmp (arg, "autoconnect"))
    {
      i3mud_info.autoconnect = !i3mud_info.autoconnect;

      if (i3mud_info.autoconnect)
	i3println (ch, "Autoconnect enabled.");
      else
	i3println (ch, "Autoconnect disabled.");
      rw_i3_config (act_write);
      return;
    }

  if (NullStr (argument))
    {
      I3_setconfig (n_fun, ch, "");
      return;
    }

  if (!str_cmp (arg, "implevel")
      && ch->pcdata->i3chardata->i3perm == I3PERM_IMP)
    {
      int value = atoi (argument);

      i3mud_info.implevel = value;
      rw_i3_config (act_write);
      i3printlnf (ch, "Implementor level changed to %d", value);
      return;
    }

  if (!str_cmp (arg, "adminlevel"))
    {
      int value = atoi (argument);

      i3mud_info.adminlevel = value;
      rw_i3_config (act_write);
      i3printlnf (ch, "Admin level changed to %d", value);
      return;
    }

  if (!str_cmp (arg, "immlevel"))
    {
      int value = atoi (argument);

      i3mud_info.immlevel = value;
      rw_i3_config (act_write);
      i3printlnf (ch, "Immortal level changed to %d", value);
      return;
    }

  if (!str_cmp (arg, "minlevel"))
    {
      int value = atoi (argument);

      i3mud_info.minlevel = value;
      rw_i3_config (act_write);
      i3printlnf (ch, "Minimum level changed to %d", value);
      return;
    }

  if (I3_is_connected ())
    {
      i3printlnf (ch, "%s may not be changed while the mud is connected.",
		  arg);
      return;
    }

  if (!str_cmp (arg, "telnet"))
    {
      replace_str (&i3mud_info.telnet, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Telnet address changed to %s:%d", argument,
		  i3mud_info.player_port);
      return;
    }

  if (!str_cmp (arg, "web"))
    {
      replace_str (&i3mud_info.web, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Website changed to %s", argument);
      return;
    }

  if (!str_cmp (arg, "email"))
    {
      replace_str (&i3mud_info.admin_email, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Admin email changed to %s", argument);
      return;
    }

  if (!str_cmp (arg, "status"))
    {
      replace_str (&i3mud_info.open_status, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Status changed to %s", argument);
      return;
    }

  if (!str_cmp (arg, "mudtype"))
    {
      replace_str (&i3mud_info.mud_type, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Mud type changed to %s", argument);
      return;
    }

  if (!str_cmp (arg, "mudlib"))
    {
      replace_str (&i3mud_info.mudlib, argument);
      rw_i3_config (act_write);
      i3printlnf (ch, "Mudlib changed to %s", argument);
      return;
    }

  I3_setconfig (n_fun, ch, "");
  return;
}

Do_Fun (I3_permstats)
{
  CharData *victim;

  if (NullStr (argument))
    {
      i3println (ch, "Usage: i3perms <user>");
      return;
    }

  if (!(victim = I3_find_user (argument)))
    {
      i3println (ch, "No such person is currently online.");
      return;
    }

  if (victim->pcdata->i3chardata->i3perm < 0
      || victim->pcdata->i3chardata->i3perm > I3PERM_IMP)
    {
      i3printlnf (ch, "{R%s has an invalid permission setting!",
		  GetName (victim));
      return;
    }

  i3printlnf (ch, "{YPermissions for %s: %s", GetName (victim),
	      i3perm_names[victim->pcdata->i3chardata->i3perm].name);
  i3printlnf (ch, "{gThese permissions were obtained %s.",
	      IsSet (victim->pcdata->i3chardata->i3flags,
		     I3_PERMOVERRIDE) ? "manually via i3permset" :
	      "automatically by level");
  return;
}


bool
i3check_permissions (CharData * ch, int checkvalue, int targetvalue,
		     bool enforceequal)
{
  if (checkvalue < 0 || checkvalue > I3PERM_IMP)
    {
      i3println (ch, "Invalid permission setting.");
      return false;
    }

  if (checkvalue > ch->pcdata->i3chardata->i3perm)
    {
      i3println (ch, "You cannot set permissions higher than your own.");
      return false;
    }

  if (checkvalue == ch->pcdata->i3chardata->i3perm
      && ch->pcdata->i3chardata->i3perm != I3PERM_IMP && enforceequal)
    {
      i3println (ch,
		 "You cannot set permissions equal to your own. Someone higher up must do this.");
      return false;
    }

  if (ch->pcdata->i3chardata->i3perm < targetvalue)
    {
      i3println (ch,
		 "You cannot alter the permissions of someone or something above your own.");
      return false;
    }
  return true;
}

Do_Fun (I3_permset)
{
  CharData *victim;
  char arg[MIL];
  int permvalue;

  argument = first_arg (argument, arg, false);

  if (NullStr (arg))
    {
      i3println (ch, "Usage: i3permset <user> <permission>");
      i3println (ch, "Permission can be one of: None, Mort, Imm, Admin, Imp");
      return;
    }

  if (!(victim = I3_find_user (arg)))
    {
      i3println (ch, "No such person is currently online.");
      return;
    }

  if (!str_cmp (argument, "override"))
    permvalue = -1;
  else
    {
      permvalue = get_permvalue (argument);

      if (!i3check_permissions
	  (ch, permvalue, victim->pcdata->i3chardata->i3perm, true))
	return;
    }



  if (victim->pcdata->i3chardata->i3perm == permvalue)
    {
      i3printlnf (ch, "%s already has a permission level of %s.",
		  GetName (victim), i3perm_names[permvalue].name);
      return;
    }

  if (permvalue == -1)
    {
      RemBit (victim->pcdata->i3chardata->i3flags, I3_PERMOVERRIDE);
      i3printlnf (ch, "{YPermission flag override has been removed from %s",
		  GetName (victim));
      return;
    }
  victim->pcdata->i3chardata->i3perm = (i3perm_t) permvalue;
  i3printlnf (ch, "{YPermission level for %s has been changed to %s",
	      GetName (victim), i3perm_names[permvalue].name);



  if (victim->pcdata->i3chardata->i3_listen != NULL)
    {
      I3_CHANNEL *channel = NULL;
      const char *channels = victim->pcdata->i3chardata->i3_listen;

      while (1)
	{
	  if (channels[0] == '\0')
	    break;
	  channels = one_argument (channels, arg);

	  if (!(channel = find_I3_channel_by_localname (arg)))
	    unflagname (&victim->pcdata->i3chardata->i3_listen, arg);
	  if (channel && victim->pcdata->i3chardata->i3perm < channel->i3perm)
	    {
	      unflagname (&victim->pcdata->i3chardata->i3_listen, arg);
	      i3printlnf (ch,
			  "{WRemoving '%s' level channel: '%s', exceeding new permission of '%s'",
			  i3perm_names[channel->i3perm].name,
			  channel->local_name,
			  i3perm_names[victim->pcdata->i3chardata->i3perm].
			  name);
	    }
	}
    }
  return;
}

Do_Fun (I3_who)
{
  I3_MUD *mud;

  if (NullStr (argument))
    {
      i3println (ch, "{YGet an overview of which mud?\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!(mud = find_I3_mud_by_name (argument)))
    {
      i3println (ch, "{YNo such mud known.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (mud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", mud->name);
      return;
    }

  if (mud->who == 0)
    i3printlnf (ch,
		"%s does not support the 'who' command. Sending anyway.",
		mud->name);

  I3_send_who (ch, mud->name);
}

Do_Fun (I3_locate)
{
  if (NullStr (argument))
    {
      i3println (ch, "Locate who?");
      return;
    }
  I3_send_locate (ch, argument);
}

Do_Fun (I3_finger)
{
  char user[MIL], mud[MIL];
  char *ps;
  I3_MUD *pmud;

  if (IsSet (ch->pcdata->i3chardata->i3flags, I3_DENYFINGER))
    {
      i3println (ch, "You are not allowed to use i3finger.");
      return;
    }

  if (NullStr (argument))
    {
      i3println (ch, "Usage: i3finger <user@mud>");
      i3println (ch, "Usage: i3finger privacy");
      return;
    }

  if (!str_cmp (argument, "privacy"))
    {
      if (!IsSet (ch->pcdata->i3chardata->i3flags, I3_PRIVACY))
	{
	  SetBit (ch->pcdata->i3chardata->i3flags, I3_PRIVACY);
	  i3println (ch, "I3 finger privacy flag set.");
	  return;
	}
      RemBit (ch->pcdata->i3chardata->i3flags, I3_PRIVACY);
      i3println (ch, "I3 finger privacy flag removed.");
      return;
    }

  if (I3ISINVIS (ch))
    {
      i3println (ch, "You are invisible.");
      return;
    }

  if ((ps = strchr (argument, '@')) == NULL)
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  ps[0] = '\0';
  strlcpy (user, argument, MIL);
  strlcpy (mud, ps + 1, MIL);

  if (user[0] == '\0' || mud[0] == '\0')
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!(pmud = find_I3_mud_by_name (mud)))
    {
      i3println (ch, "{YNo such mud known.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (!str_cmp (I3_THISMUD, pmud->name))
    {
      i3println (ch, "Use your mud's own internal system for that.");
      return;
    }

  if (pmud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", pmud->name);
      return;
    }

  if (pmud->finger == 0)
    i3printlnf (ch,
		"%s does not support the 'finger' command. Sending anyway.",
		pmud->name);

  I3_send_finger (ch, user, pmud->name);
}

Do_Fun (I3_emote)
{
  char to[MIL], *ps;
  char mud[MIL];
  I3_MUD *pmud;

  if (I3ISINVIS (ch))
    {
      i3println (ch, "You are invisible.");
      return;
    }

  argument = first_arg (argument, to, false);
  ps = strchr (to, '@');

  if (to[0] == '\0' || argument[0] == '\0' || ps == NULL)
    {
      i3println (ch, "{YYou should specify a person and a mud.\n\r"
		 "(use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  ps[0] = '\0';
  ps++;
  strlcpy (mud, ps, MIL);

  if (!(pmud = find_I3_mud_by_name (mud)))
    {
      i3println (ch, "{YNo such mud known.\n\r"
		 "( use {Wi3mudlist{Y to get an overview of the muds available)");
      return;
    }

  if (pmud->status >= 0)
    {
      i3printlnf (ch, "%s is marked as down.", pmud->name);
      return;
    }

  if (pmud->emoteto == 0)
    i3printlnf (ch,
		"%s does not support the 'emoteto' command. Sending anyway.",
		pmud->name);

  I3_send_emoteto (ch, to, pmud, argument);
}


SOCKET
I3_connection_open (ROUTER_DATA * router)
{
  struct sockaddr_in sa;

  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
	      "Attempting connect to %s on port %d", router->ip,
	      router->port);

  I3_control = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (I3_control == INVALID_SOCKET)
    {
      socket_error ("Cannot create socket!");
      I3_connection_close (true);
      return INVALID_SOCKET;
    }

  if (!socket_cntl (I3_control))
    {
      log_string ("I3_connection_open: fcntl(F_SETFL)");
      I3_connection_close (true);
      return INVALID_SOCKET;
    }

  memset (&sa, 0, sizeof (sa));
  sa.sin_family = AF_INET;

  if (!inet_aton (router->ip, &sa.sin_addr))
    {
      struct hostent *hostp = gethostbyname (router->ip);

      if (!hostp)
	{
	  log_string ("I3_connection_open: Cannot resolve router hostname.");
	  I3_connection_close (true);
	  return INVALID_SOCKET;
	}
      memcpy (&sa.sin_addr, hostp->h_addr, hostp->h_length);
    }

  sa.sin_port = htons (router->port);

  if (connect (I3_control, (SOCKADDR *) & sa, sizeof (sa)) == SOCKET_ERROR)
    {
      if (!check_errno (EINPROGRESS))
	{
	  socket_error (FORMATF
			("I3_connection_open: Unable to connect to router %s",
			 router->name));
	  I3_connection_close (true);
	  return INVALID_SOCKET;
	}
    }
  I3_ROUTER_NAME = router->name;
  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
	      "Connected to Intermud-3 router %s", router->name);
  return I3_control;
}


bool
i3banned (const char *ignore)
{
  I3_BAN *temp;
  I3_MUD *mud;
  char *ps;
  char mudname[MIL], ipbuf[512];


  for (temp = I3ban_first; temp; temp = temp->next)
    {
      if (!fnmatch (temp->name, ignore, 0))
	return true;
    }


  ps = strchr (ignore, '@');

  if (NullStr (ignore) || ps == NULL)
    return false;

  ps[0] = '\0';
  strlcpy (mudname, ps + 1, MIL);

  for (mud = i3mud_first; mud; mud = mud->next)
    {
      if (!str_cmp (mud->name, mudname))
	{
	  snprintf (ipbuf, sizeof (ipbuf), "%s:%d", mud->ipaddress,
		    mud->player_port);
	  for (temp = I3ban_first; temp; temp = temp->next)
	    {
	      if (!str_cmp (temp->name, ipbuf))
		return true;
	    }
	}
    }
  return false;
}


I3_HEADER *
I3_get_header (char **pps)
{
  I3_HEADER *header;
  char *ps = *pps, *next_ps;

  alloc_mem (header, I3_HEADER, 1);

  I3_get_field (ps, &next_ps);
  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (header->originator_mudname, ps, 256);
  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (header->originator_username, ps, 256);
  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (header->target_mudname, ps, 256);
  ps = next_ps;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (header->target_username, ps, 256);

  *pps = next_ps;
  return header;
}


void
I3_read_packet (void)
{
  long size;

  memmove (&size, I3_input_buffer, 4);
  size = ntohl (size);

  memmove (I3_currentpacket, I3_input_buffer + 4, size);

  if (I3_currentpacket[size - 1] != ')')
    I3_currentpacket[size - 1] = 0;
  I3_currentpacket[size] = 0;

  memmove (I3_input_buffer, I3_input_buffer + size + 4,
	   I3_input_pointer - size - 4);
  I3_input_pointer -= size + 4;
  return;
}


void
I3_parse_packet (void)
{
  I3_HEADER *header = NULL;
  char *ps, *next_ps;
  char ptype[MIL];

  ps = I3_currentpacket;
  if (ps[0] != '(' || ps[1] != '{')
    return;

  if (packetdebug)
    logf ("Packet received: %s", ps);

  ps += 2;
  I3_get_field (ps, &next_ps);
  I3_remove_quotes (&ps);
  strlcpy (ptype, ps, MIL);

  header = I3_get_header (&ps);


  if (i3banned (header->originator_mudname))
    return;

  if (!str_cmp (ptype, "tell"))
    I3_process_tell (header, ps);

  if (!str_cmp (ptype, "beep"))
    I3_process_beep (header, ps);

  if (!str_cmp (ptype, "emoteto"))
    I3_process_emoteto (header, ps);

  if (!str_cmp (ptype, "channel-m"))
    I3_process_channel_m (header, ps);

  if (!str_cmp (ptype, "channel-e"))
    I3_process_channel_e (header, ps);

  if (!str_cmp (ptype, "chan-filter-req"))
    I3_process_channel_filter (header, ps);

  if (!str_cmp (ptype, "finger-req"))
    I3_process_finger_req (header, ps);

  if (!str_cmp (ptype, "finger-reply"))
    I3_process_finger_reply (header, ps);

  if (!str_cmp (ptype, "locate-req"))
    I3_process_locate_req (header, ps);

  if (!str_cmp (ptype, "locate-reply"))
    I3_process_locate_reply (header, ps);

  if (!str_cmp (ptype, "chan-who-req"))
    I3_process_chan_who_req (header, ps);

  if (!str_cmp (ptype, "chan-who-reply"))
    I3_process_chan_who_reply (header, ps);

  if (!str_cmp (ptype, "chan-adminlist-reply"))
    I3_process_channel_adminlist_reply (header, ps);

  if (!str_cmp (ptype, "ucache-update") && i3mud_info.ucache == true)
    I3_process_ucache_update (header, ps);

  if (!str_cmp (ptype, "who-req"))
    I3_process_who_req (header, ps);

  if (!str_cmp (ptype, "who-reply"))
    I3_process_who_reply (header, ps);

  if (!str_cmp (ptype, "chanlist-reply"))
    I3_process_chanlist_reply (header, ps);

  if (!str_cmp (ptype, "startup-reply"))
    I3_process_startup_reply (header, ps);

  if (!str_cmp (ptype, "mudlist"))
    I3_process_mudlist (header, ps);

  if (!str_cmp (ptype, "error"))
    I3_process_error (header, ps);

  if (!str_cmp (ptype, "chan-ack"))
    I3_process_chanack (header, ps);

  if (!str_cmp (ptype, "channel-t"))
    I3_process_channel_t (header, ps);

  if (!str_cmp (ptype, "chan-user-req"))
    I3_process_chan_user_req (header, ps);

  if (!str_cmp (ptype, "chan-user-reply") && i3mud_info.ucache == true)
    I3_process_chan_user_reply (header, ps);

  if (!str_cmp (ptype, "router-shutdown"))
    {
      int delay;

      I3_get_field (ps, &next_ps);
      delay = atoi (ps);

      if (delay == 0)
	{
	  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
		      "Router %s is shutting down.", I3_ROUTER_NAME);
	  I3_connection_close (false);
	}
      else
	{
	  new_wiznet (NULL, NULL, WIZ_I3, true, 1,
		      "Router %s is rebooting and will be back in %d second%s.",
		      I3_ROUTER_NAME, delay, delay == 1 ? "" : "s");
	  I3_connection_close (true);
	}
    }

  free_mem (header);
  return;
}

I3_MUD *
new_i3mud (void)
{
  I3_MUD *Data;

  alloc_mem (Data, I3_MUD, 1);

  return Data;
}

TableSave_Fun (rw_i3_mudlist)
{
  rw_list (type, I3_MUDLIST_FILE, I3_MUD, i3mud);
  if (type == act_read)
    unlink (I3_MUDLIST_FILE);
}

TableSave_Fun (rw_i3_chanlist)
{
  rw_list (type, I3_CHANLIST_FILE, I3_CHANNEL, I3chan);
  if (type == act_read)
    {
      unlink (I3_CHANLIST_FILE);
      init_i3_channels ();
    }
}


void
I3_loadhistory (void)
{
  char filename[256];
  FileData *tempfile;
  I3_CHANNEL *tempchan = NULL;
  int x;

  for (tempchan = I3chan_first; tempchan; tempchan = tempchan->next)
    {
      if (!tempchan->local_name)
	continue;

      snprintf (filename, sizeof (filename), I3DIR "%s.hist",
		tempchan->local_name);

      if (!(tempfile = f_open (filename, "r")))
	continue;

      for (x = 0; x < MAX_I3HISTORY; x++)
	{
	  if (f_eof (tempfile))
	    tempchan->history[x] = NULL;
	  else
	    tempchan->history[x] = str_dup (read_line (tempfile));
	}
      f_close (tempfile);
      unlink (filename);
    }
}


void
I3_savehistory (void)
{
  char filename[256];
  FileData *tempfile;
  I3_CHANNEL *tempchan = NULL;
  int x;

  for (tempchan = I3chan_first; tempchan; tempchan = tempchan->next)
    {
      if (!tempchan->local_name)
	continue;

      if (!tempchan->history[0])
	continue;

      snprintf (filename, sizeof (filename), I3DIR "%s.hist",
		tempchan->local_name);

      if (!(tempfile = f_open (filename, "w")))
	continue;

      for (x = 0; x < MAX_I3HISTORY; x++)
	{
	  if (tempchan->history[x] != NULL)
	    f_printf (tempfile, "%s", tempchan->history[x]);
	}
      f_close (tempfile);
    }
}


void
router_connect (const char *router_name, bool forced, int mudport,
		bool isconnected)
{
  ROUTER_DATA *router;
  bool rfound = false;

  i3wait = 0;
  i3timeout = 0;
  bytes_sent = 0;
  bytes_received = 0;


  manual_router = router_name;


  if (i3cmd_first == NULL)
    {
      rw_i3_cmds (act_read);
      if (!i3cmd_first)
	{
	  wiznet ("router_connect: Unable to load command table!", NULL,
		  NULL, WIZ_I3, true, 1);
	  I3_control = INVALID_SOCKET;
	  return;
	}
    }

  if (!I3_read_config (mudport))
    {
      I3_control = INVALID_SOCKET;
      return;
    }

  if (router_first == NULL)
    {
      rw_i3_routers (act_read);
      if (!router_first)
	{
	  wiznet ("router_connect: No router configurations were found!",
		  NULL, NULL, WIZ_I3, true, 1);
	  I3_control = INVALID_SOCKET;
	  return;
	}
      I3_ROUTER_NAME = router_first->name;
    }


  if (i3help_first == NULL)
    rw_i3_helps (act_read);


  if (i3color_first == NULL)
    rw_i3_colors (act_read);

  if ((!i3mud_info.autoconnect && !forced && !isconnected)
      || (isconnected && I3_control == INVALID_SOCKET))
    {
      wiznet
	("Intermud-3 network data loaded. Autoconnect not set. Will need to connect manually.",
	 NULL, NULL, WIZ_I3, true, 1);
      I3_control = INVALID_SOCKET;
      return;
    }
  else
    wiznet
      ("Intermud-3 network data loaded. Initialiazing network connection...",
       NULL, NULL, WIZ_I3, true, 1);

  rw_i3_bans (act_read);

  if (i3mud_info.ucache == true)
    {
      rw_i3_ucache (act_read);
      I3_prune_ucache ();
      ucache_clock = current_time + (HOUR * 24);
    }

  if (I3_control == INVALID_SOCKET)
    {
      for (router = router_first; router; router = router->next)
	{
	  if (router_name && str_cmp (router_name, router->name))
	    continue;

	  if (router->reconattempts <= 3)
	    {
	      rfound = true;
	      I3_control = I3_connection_open (router);
	      break;
	    }
	}
    }

  if (!rfound && !isconnected)
    {
      wiznet ("Unable to connect. No available routers found.", NULL,
	      NULL, WIZ_I3, true, 1);
      I3_control = INVALID_SOCKET;
      return;
    }

  if (I3_control == INVALID_SOCKET)
    {
      i3wait = 100;
      return;
    }

  sleep_seconds (1);

  wiznet ("Intermud-3 Network initialized.", NULL, NULL, WIZ_I3, true, 1);

  if (!isconnected)
    {
      rw_i3_channels (act_read);
      I3_startup_packet ();
      i3timeout = 100;
    }
  else
    {
      rw_i3_mudlist (act_read);
      rw_i3_chanlist (act_read);
    }
  I3_loadhistory ();
}

Do_Fun (I3_connect)
{
  ROUTER_DATA *router;

  if (I3_is_connected ())
    {
      i3printlnf (ch,
		  "The MUD is already connected to Intermud-3 router %s",
		  I3_ROUTER_NAME);
      return;
    }

  if (NullStr (argument))
    {
      i3print (ch, "Connecting to Intermud-3... ");
      router_connect (NULL, true, i3mud_info.player_port, false);
      if (I3_is_connected ())
	i3println (ch, "Success.");
      else
	i3println (ch, "Failed.");
      return;
    }

  for (router = router_first; router; router = router->next)
    {
      if (!str_cmp (router->name, argument))
	{
	  router->reconattempts = 0;
	  i3printf (ch, "Connecting to Intermud-3 router %s...", argument);
	  router_connect (argument, true, i3mud_info.player_port, false);
	  if (I3_is_connected ())
	    i3println (ch, "Success.");
	  else
	    i3println (ch, "Failed.");
	  return;
	}
    }

  i3printlnf (ch, "%s is not configured as a router for this mud.", argument);
  i3println (ch,
	     "If you wish to add it, use the i3router command to provide its information.");
  return;
}


void
I3_main (bool isconnected)
{
  router_connect (NULL, false, mainport, isconnected);
}

void
I3_send_shutdown (int delay)
{
  I3_CHANNEL *channel;
  char s[50];

  if (!I3_is_connected ())
    return;

  for (channel = I3chan_first; channel; channel = channel->next)
    {
      if (!NullStr (channel->local_name))
	I3_send_channel_listen (channel, false);
    }

  I3_write_header ("shutdown", I3_THISMUD, NULL, I3_ROUTER_NAME, NULL);
  snprintf (s, sizeof (s), "%d", delay);
  I3_write_buffer (s);
  I3_write_buffer (",})\r");

  if (!I3_write_packet (I3_output_buffer))
    I3_connection_close (false);

  return;
}


void
I3_loop (void)
{
  ROUTER_DATA *router;
  int ret;
  long size;
  fd_set in_set, out_set, exc_set;
  static struct timeval null_time;
  bool rfound = false;

  FD_ZERO (&in_set);
  FD_ZERO (&out_set);
  FD_ZERO (&exc_set);

  if (i3wait > 0)
    i3wait--;

  if (i3timeout > 0)
    {
      i3timeout--;
      if (i3timeout == 0)
	{
	  I3_connection_close (true);
	  return;
	}
    }


  if (i3wait == 1)
    {
      for (router = router_first; router; router = router->next)
	{
	  if (manual_router && str_cmp (router->name, manual_router))
	    continue;

	  if (router->reconattempts <= 3)
	    {
	      rfound = true;
	      break;
	    }
	}

      if (!rfound)
	{
	  i3wait = -2;
	  wiznet ("Unable to reconnect. No routers responding.", NULL,
		  NULL, WIZ_I3, true, 1);
	  ;
	  return;
	}
      I3_control = I3_connection_open (router);
      if (I3_control == INVALID_SOCKET)
	{
	  if (router->reconattempts <= 3)
	    i3wait = 100;
	  return;
	}

      sleep_seconds (1);

      new_wiznet (NULL, NULL, WIZ_I3, true, 1,
		  "Connection to Intermud-3 router %s %s.", router->name,
		  router->reconattempts >
		  0 ? "reestablished" : "established");
      router->reconattempts++;
      I3_startup_packet ();
      i3timeout = 100;
      return;
    }

  if (!I3_is_connected ())
    return;


  if (ucache_clock <= current_time)
    {
      ucache_clock = current_time + (HOUR * 24);
      I3_prune_ucache ();
    }

  FD_SET (I3_control, &in_set);
  FD_SET (I3_control, &out_set);
  FD_SET (I3_control, &exc_set);

  if (select (I3_control + 1, &in_set, &out_set, &exc_set, &null_time) ==
      SOCKET_ERROR)
    {
      socket_error ("I3_loop: select: Unable to poll I3_control!");
      I3_connection_close (true);
      return;
    }

  if (FD_ISSET (I3_control, &exc_set))
    {
      FD_CLR (I3_control, &in_set);
      FD_CLR (I3_control, &out_set);
      log_string ("Exception raised on socket.");
      I3_connection_close (true);
      return;
    }

  if (FD_ISSET (I3_control, &in_set))
    {
      ret = recv (I3_control, I3_input_buffer + I3_input_pointer, MSL, 0);
      if (!ret || (ret < 0 && !check_errno (EWOULDBLOCK)
#ifndef WIN32
		   && !check_errno (EAGAIN)
#endif
	  ))
	{
	  FD_CLR (I3_control, &out_set);
	  if (ret < 0)
	    log_string ("Read error on socket.");
	  else
	    log_string ("EOF encountered on socket read.");
	  I3_connection_close (true);
	  return;
	}
      if (ret < 0)
	return;

      I3_input_pointer += ret;
      bytes_received += ret;
      if (packetdebug)
	logf ("Bytes received: %d", ret);
    }

  memcpy (&size, I3_input_buffer, 4);
  size = ntohl (size);

  if (size <= I3_input_pointer - 4)
    {
      I3_read_packet ();
      I3_parse_packet ();
    }
  return;
}


void
I3_shutdown (int delay)
{
  if (I3_control == INVALID_SOCKET || crs_info.status == CRS_COPYOVER)
    return;

  I3_savehistory ();
  free_i3data (false);


  if (I3_output_pointer != 4)
    I3_write_packet (I3_output_buffer);

  I3_send_shutdown (delay);
  I3_connection_close (false);
  I3_input_pointer = 0;
  I3_output_pointer = 4;
  sleep_seconds (2);
}

char *
I3_find_social (CharData * ch, char *sname, const char *person,
		const char *mud, bool victim)
{
  static char socname[MSL];
  SocialData *social;
  char *c;

  socname[0] = '\0';

  for (c = sname; *c; *c = tolower (*c), c++)
    ;

  if ((social = find_social (sname)) == NULL)
    {
      i3printlnf (ch, "{YSocial {W%s{Y does not exist on this mud.", sname);
      return socname;
    }

  if (NullStr (person) && NullStr (mud))
    {
      if (NullStr (person) && !str_cmp (person, GetName (ch))
	  && NullStr (mud) && !str_cmp (mud, I3_THISMUD))
	{
	  if (!social->others_auto)
	    {
	      i3printlnf (ch, "{YSocial {W%s{Y: Missing others_auto.",
			  social->name);
	      return socname;
	    }
	  strlcpy (socname, social->others_auto, MSL);
	}
      else
	{
	  if (!victim)
	    {
	      if (!social->others_found)
		{
		  i3printlnf (ch,
			      "{YSocial {W%s{Y: Missing others_found.",
			      social->name);
		  return socname;
		}
	      strlcpy (socname, social->others_found, MSL);
	    }
	  else
	    {
	      if (!social->vict_found)
		{
		  i3printlnf (ch, "{YSocial {W%s{Y: Missing vict_found.",
			      social->name);
		  return socname;
		}
	      strlcpy (socname, social->vict_found, MSL);
	    }
	}
    }
  else
    {
      if (!social->others_no_arg)
	{
	  i3printlnf (ch, "{YSocial {W%s{Y: Missing others_no_arg.",
		      social->name);
	  return socname;
	}
      strlcpy (socname, social->others_no_arg, MSL);
    }
  return socname;
}

CharData *
I3_make_skeleton (const char *name)
{
  CharData *skeleton;

  alloc_mem (skeleton, CharData, 1);

  skeleton->name = str_dup (name);
  skeleton->short_descr = str_dup (name);
  skeleton->in_room = get_room_index (ROOM_VNUM_LIMBO);

  return skeleton;
}

void
I3_purge_skeleton (CharData * skeleton)
{
  if (!skeleton)
    return;

  free_string (skeleton->name);
  free_string (skeleton->short_descr);
  free_mem (skeleton);

  return;
}

void
I3_send_social (I3_CHANNEL * channel, CharData * ch, const char *argument)
{
  CharData *skeleton = NULL;
  char *ps;
  char socbuf_o[MSL], socbuf_t[MSL], msg_o[MSL], msg_t[MSL];
  char arg1[MIL], person[MIL], mud[MIL], user[MIL], buf[MSL];
  unsigned int x;

  person[0] = '\0';
  mud[0] = '\0';


  argument = first_arg (argument, arg1, false);

  snprintf (user, sizeof (user), "%s@%s", GetName (ch), I3_THISMUD);
  if (!str_cmp (user, argument))
    {
      i3println (ch,
		 "Cannot target yourself due to the nature of I3 socials.");
      return;
    }

  if (!NullStr (argument))
    {
      if (!(ps = strchr (argument, '@')))
	{
	  i3println (ch, "You need to specify a person@mud for a target.");
	  return;
	}
      else
	{
	  for (x = 0; x < strlen (argument); x++)
	    {
	      person[x] = argument[x];
	      if (person[x] == '@')
		break;
	    }
	  person[x] = '\0';

	  ps[0] = '\0';
	  strlcpy (mud, ps + 1, MIL);
	}
    }

  snprintf (socbuf_o, sizeof (socbuf_o), "%s",
	    I3_find_social (ch, arg1, person, mud, false));

  if (NullStr (socbuf_o))
    snprintf (socbuf_t, sizeof (socbuf_t), "%s",
	      I3_find_social (ch, arg1, person, mud, true));

  if (!NullStr (socbuf_o) && !NullStr (socbuf_t))
    {
      if (!NullStr (argument))
	{
	  sex_t sex;

	  snprintf (buf, sizeof (buf), "%s@%s", person, mud);
	  sex = I3_get_ucache_gender (buf);
	  if (sex == -1)
	    {

	      I3_send_chan_user_req (mud, person);
	      sex = SEX_MALE;
	    }
	  else
	    sex = i3todikugender (sex);

	  skeleton = I3_make_skeleton (buf);
	  skeleton->sex = sex;
	}

      strlcpy (msg_o,
	       perform_act_string (socbuf_o, ch, NULL, skeleton, false), MSL);
      strlcpy (msg_t,
	       perform_act_string (socbuf_t, ch, NULL, skeleton, false), MSL);

      if (!skeleton)
	I3_send_channel_emote (channel, GetName (ch), msg_o);
      else
	{
	  strlcpy (buf, person, MSL);
	  buf[0] = tolower (buf[0]);
	  I3_send_channel_t (channel, GetName (ch), mud, buf, msg_o, msg_t,
			     person);
	}
      if (skeleton)
	I3_purge_skeleton (skeleton);
    }
  return;
}

Do_Fun (I3_router)
{
  ROUTER_DATA *router;
  char cmd[MIL];

  if (NullStr (argument))
    {
      i3println (ch,
		 "Usage: i3router add <router_name> <router_ip> <router_port>");
      i3println (ch, "Usage: i3router remove <router_name>");
      i3println (ch, "Usage: i3router list");
      return;
    }
  argument = first_arg (argument, cmd, false);

  if (!str_cmp (cmd, "list"))
    {
      i3println (ch, "{RThe mud has the following routers configured:");
      i3println (ch,
		 "{WRouter Name     Router IP/DNS                  Router Port");
      for (router = router_first; router; router = router->next)
	i3printlnf (ch, "{c%-15.15s{c %-30.30s %d", router->name,
		    router->ip, router->port);
      return;
    }

  if (NullStr (argument))
    {
      I3_router (n_fun, ch, "");
      return;
    }

  if (!str_cmp (cmd, "remove"))
    {
      for (router = router_first; router; router = router->next)
	{
	  if (!str_cmp (router->name, argument)
	      || !str_cmp (router->ip, argument))
	    {
	      free_string (router->name);
	      free_string (router->ip);
	      UnLink (router, router, next, prev);
	      free_mem (router);
	      i3printlnf (ch,
			  "{YRouter {W%s{Y has been removed from your configuration.",
			  argument);
	      rw_i3_config (act_write);
	      return;
	    }
	}
      i3printlnf (ch,
		  "{YNo router named {W%s{Y exists in your configuration.",
		  argument);
      return;
    }

  if (!str_cmp (cmd, "add"))
    {
      ROUTER_DATA *temp;
      char rtname[MIL];
      char rtip[MIL];
      int rtport;

      argument = first_arg (argument, rtname, false);
      argument = first_arg (argument, rtip, false);

      if (NullStr (rtname) || NullStr (rtip) || NullStr (argument))
	{
	  I3_router (n_fun, ch, "");
	  return;
	}

      if (rtname[0] != '*')
	{
	  i3println (ch,
		     "{YA router name must begin with a {W*{Y to be valid.");
	  return;
	}

      for (temp = router_first; temp; temp = temp->next)
	{
	  if (!str_cmp (temp->name, rtname))
	    {
	      i3printlnf (ch,
			  "{YA router named {W%s{Y is already in your configuration.",
			  rtname);
	      return;
	    }
	}

      if (!is_number (argument))
	{
	  i3println (ch, "{YPort must be a numerical value.");
	  return;
	}

      rtport = atoi (argument);
      if (rtport < 1 || rtport > 65535)
	{
	  i3println (ch, "{YInvalid port value specified.");
	  return;
	}

      alloc_mem (router, ROUTER_DATA, 1);
      Link (router, router, next, prev);
      router->name = str_dup (rtname);
      router->ip = str_dup (rtip);
      router->port = rtport;
      router->reconattempts = 0;

      i3printlnf (ch,
		  "{YRouter: {W%s %s %d{Y has been added to your configuration.",
		  router->name, router->ip, router->port);
      rw_i3_config (act_write);
      return;
    }
  I3_router (n_fun, ch, "");
  return;
}

Do_Fun (I3_stats)
{
  I3_MUD *mud;
  I3_CHANNEL *channel;
  int mud_count = 0, chan_count = 0;

  for (mud = i3mud_first; mud; mud = mud->next)
    mud_count++;

  for (channel = I3chan_first; channel; channel = channel->next)
    chan_count++;

  i3print (ch, "{cGeneral Statistics:" NEWLINE);
  i3printlnf (ch, "{cCurrently connected to: {W%s",
	      I3_is_connected ()? I3_ROUTER_NAME : "Nowhere!");
  i3printlnf (ch, "{cBytes sent    : {W%ld", bytes_sent);
  i3printlnf (ch, "{cBytes received: {W%ld", bytes_received);
  i3printlnf (ch, "{cKnown muds    : {W%d", mud_count);
  i3printlnf (ch, "{cKnown channels: {W%d", chan_count);
  return;
}

Do_Fun (i3_help)
{
  I3_HELP_DATA *help;
  Buffer *buf;
  int col, perm;

  if (NullStr (argument))
    {
      buf = new_buf ();
      i3bprintln (buf, "{gHelp is available for the following commands:");
      i3bprintln (buf, "{G---------------------------------------------");
      for (perm = I3PERM_MORT; perm <= ch->pcdata->i3chardata->i3perm; perm++)
	{
	  col = 0;
	  i3bprintlnf (buf, "\n\r{g%s helps:{G", i3perm_names[perm].name);
	  for (help = i3help_first; help; help = help->next)
	    {
	      if (help->level != perm)
		continue;

	      i3bprintf (buf, "%-15s", help->name);
	      if (++col % 6 == 0)
		i3bprintln (buf, "");
	    }
	  if (col % 6 != 0)
	    i3bprintln (buf, "");
	}
      sendpage (ch, buf_string (buf));
      return;
    }

  for (help = i3help_first; help; help = help->next)
    {
      if (!str_cmp (help->name, argument))
	{
	  if (NullStr (help->text))
	    i3printlnf (ch, "{gNo inforation available for topic {W%s{g.",
			help->name);
	  else
	    i3printlnf (ch, "{g%s", help->text);
	  return;
	}
    }
  i3printlnf (ch, "{gNo help exists for topic {W%s{g.", argument);
  return;
}


Do_Fun (i3_hedit)
{
  I3_HELP_DATA *help;
  char name[MIL], cmd[MIL];
  bool found = false;

  argument = one_argument (argument, name);
  argument = one_argument (argument, cmd);

  if (NullStr (name) || NullStr (cmd) || NullStr (argument))
    {
      i3println (ch, "{wUsage: i3hedit <topic> [name|perm] <field>");
      i3println (ch,
		 "{wWhere <field> can be either name, or permission level.");
      return;
    }

  for (help = i3help_first; help; help = help->next)
    {
      if (!str_cmp (help->name, name))
	{
	  found = true;
	  break;
	}
    }

  if (!found)
    {
      i3printlnf (ch,
		  "{gNo help exists for topic {W%s{g. You will need to add it to the helpfile manually.",
		  name);
      return;
    }

  if (!str_cmp (cmd, "name"))
    {
      i3printlnf (ch, "{W%s {ghas been renamed to {W%s.", help->name,
		  argument);
      replace_str (&help->name, argument);
      rw_help_data (act_write);
      return;
    }

  if (!str_cmp (cmd, "perm"))
    {
      int permvalue = get_permvalue (argument);

      if (!i3check_permissions (ch, permvalue, help->level, false))
	return;

      i3printlnf (ch,
		  "{gPermission level for {W%s {ghas been changed to {W%s.",
		  help->name, i3perm_names[permvalue].name);
      help->level = permvalue;
      rw_help_data (act_write);
      return;
    }
  i3_hedit (n_fun, ch, "");
  return;
}


Do_Fun (I3_other)
{
  I3_CMD_DATA *x;

  i3println (ch, "{GGeneral Usage:");
  i3println (ch, draw_line (ch, "{W-{w-", 0));
  for (x = i3cmd_first; x; x = x->next)
    {
      if (ch->pcdata->i3chardata->i3perm < x->level)
	continue;

      if (x->connected == false || I3_is_connected ())
	{
	  i3printlnf (ch, "{G%12s {g%-32s{w - %s", x->name,
		      x->args, x->descr);
	}
    }
  i3println (ch, draw_line (ch, "{W-{w-", 0));

  return;
}

const char *
i3_funcname (Do_F * func)
{
  if (func == I3_other)
    return ("I3_other");
  if (func == I3_listen_channel)
    return ("I3_listen_channel");
  if (func == I3_chanlist)
    return ("I3_chanlist");
  if (func == I3_mudlist)
    return ("I3_mudlist");
  if (func == I3_invis)
    return ("I3_invis");
  if (func == I3_who)
    return ("I3_who");
  if (func == I3_locate)
    return ("I3_locate");
  if (func == I3_tell)
    return ("I3_tell");
  if (func == I3_reply)
    return ("I3_reply");
  if (func == I3_emote)
    return ("I3_emote");
  if (func == I3_beep)
    return ("I3_beep");
  if (func == I3_ignorecmd)
    return ("I3_ignorecmd");
  if (func == I3_finger)
    return ("I3_finger");
  if (func == I3_mudinfo)
    return ("I3_mudinfo");
  if (func == I3_color)
    return ("I3_color");
  if (func == I3_afk)
    return ("I3_afk");
  if (func == I3_chan_who)
    return ("I3_chan_who");
  if (func == I3_connect)
    return ("I3_connect");
  if (func == I3_disconnect)
    return ("I3_disconnect");
  if (func == I3_send_user_req)
    return ("I3_send_user_req");
  if (func == I3_permstats)
    return ("I3_permstats");
  if (func == I3_deny_channel)
    return ("I3_deny_channel");
  if (func == I3_permset)
    return ("I3_permset");
  if (func == I3_chanlayout)
    return ("I3_chanlayout");
  if (func == I3_admin_channel)
    return ("I3_admin_channel");
  if (func == I3_addchan)
    return ("I3_addchan");
  if (func == I3_removechan)
    return ("I3_removechan");
  if (func == I3_edit_channel)
    return ("I3_edit_channel");
  if (func == I3_mudlisten)
    return ("I3_mudlisten");
  if (func == I3_router)
    return ("I3_router");
  if (func == I3_bancmd)
    return ("I3_bancmd");
  if (func == I3_setconfig)
    return ("I3_setconfig");
  if (func == I3_setup_channel)
    return ("I3_setup_channel");
  if (func == I3_stats)
    return ("I3_stats");
  if (func == I3_show_ucache_contents)
    return ("I3_show_ucache_contents");
  if (func == I3_debug)
    return ("I3_debug");
  if (func == i3_hedit)
    return ("i3_hedit");
  if (func == i3_help)
    return ("i3_help");
  if (func == i3_cedit)
    return ("i3_cedit");

  return "";
}

Do_F *
i3_function (const char *func)
{
  if (!str_cmp (func, "I3_other"))
    return I3_other;
  if (!str_cmp (func, "I3_listen_channel"))
    return I3_listen_channel;
  if (!str_cmp (func, "I3_chanlist"))
    return I3_chanlist;
  if (!str_cmp (func, "I3_mudlist"))
    return I3_mudlist;
  if (!str_cmp (func, "I3_invis"))
    return I3_invis;
  if (!str_cmp (func, "I3_who"))
    return I3_who;
  if (!str_cmp (func, "I3_locate"))
    return I3_locate;
  if (!str_cmp (func, "I3_tell"))
    return I3_tell;
  if (!str_cmp (func, "I3_reply"))
    return I3_reply;
  if (!str_cmp (func, "I3_emote"))
    return I3_emote;
  if (!str_cmp (func, "I3_beep"))
    return I3_beep;
  if (!str_cmp (func, "I3_ignorecmd"))
    return I3_ignorecmd;
  if (!str_cmp (func, "I3_finger"))
    return I3_finger;
  if (!str_cmp (func, "I3_mudinfo"))
    return I3_mudinfo;
  if (!str_cmp (func, "I3_color"))
    return I3_color;
  if (!str_cmp (func, "I3_afk"))
    return I3_afk;
  if (!str_cmp (func, "I3_chan_who"))
    return I3_chan_who;
  if (!str_cmp (func, "I3_connect"))
    return I3_connect;
  if (!str_cmp (func, "I3_disconnect"))
    return I3_disconnect;
  if (!str_cmp (func, "I3_send_user_req"))
    return I3_send_user_req;
  if (!str_cmp (func, "I3_permstats"))
    return I3_permstats;
  if (!str_cmp (func, "I3_deny_channel"))
    return I3_deny_channel;
  if (!str_cmp (func, "I3_permset"))
    return I3_permset;
  if (!str_cmp (func, "I3_admin_channel"))
    return I3_admin_channel;
  if (!str_cmp (func, "I3_bancmd"))
    return I3_bancmd;
  if (!str_cmp (func, "I3_setconfig"))
    return I3_setconfig;
  if (!str_cmp (func, "I3_setup_channel"))
    return I3_setup_channel;
  if (!str_cmp (func, "I3_chanlayout"))
    return I3_chanlayout;
  if (!str_cmp (func, "I3_addchan"))
    return I3_addchan;
  if (!str_cmp (func, "I3_removechan"))
    return I3_removechan;
  if (!str_cmp (func, "I3_edit_channel"))
    return I3_edit_channel;
  if (!str_cmp (func, "I3_mudlisten"))
    return I3_mudlisten;
  if (!str_cmp (func, "I3_router"))
    return I3_router;
  if (!str_cmp (func, "I3_stats"))
    return I3_stats;
  if (!str_cmp (func, "I3_show_ucache_contents"))
    return I3_show_ucache_contents;
  if (!str_cmp (func, "I3_debug"))
    return I3_debug;
  if (!str_cmp (func, "i3_help"))
    return i3_help;
  if (!str_cmp (func, "i3_cedit"))
    return i3_cedit;
  if (!str_cmp (func, "i3_hedit"))
    return i3_hedit;

  return NULL;
}


bool
I3_command_hook (CharData * ch, const char *command, const char *argument)
{
  I3_CMD_DATA *cmd;
  NameList *alias;
  I3_CHANNEL *channel;
  int x;

  if (IsNPC (ch))
    return false;

  if (!ch->desc)
    return false;

  if (i3cmd_first == NULL)
    {
      wiznet ("Dammit! No command data is loaded!", NULL, NULL, WIZ_I3, true,
	      1);
      return false;
    }

  if (ch->pcdata->i3chardata->i3perm <= I3PERM_NONE)
    return false;


  for (cmd = i3cmd_first; cmd; cmd = cmd->next)
    {
      if (ch->pcdata->i3chardata->i3perm < i3cmd_first->level)
	continue;

      for (alias = cmd->alias_first; alias; alias = alias->next)
	{
	  if (!str_cmp (command, alias->name))
	    {
	      command = cmd->name;
	      break;
	    }
	}
      if (!str_prefix (command, cmd->name))
	{
	  if (cmd->connected == true && !I3_is_connected ())
	    {
	      i3println (ch, "The mud is not currently connected to I3.");
	      return true;
	    }

	  (*cmd->function) (cmd->name, ch, argument);
	  return true;
	}
    }



  if ((channel = find_I3_channel_by_localname (command)) == NULL)
    return false;

  if (ch->pcdata->i3chardata->i3perm < channel->i3perm)
    return false;

  if (hasname (ch->pcdata->i3chardata->i3_denied, channel->local_name))
    {
      i3printlnf (ch,
		  "You have been denied the use of %s by the administration.",
		  channel->local_name);
      return true;
    }

  if (NullStr (argument))
    {
      i3printlnf (ch, "{cThe last %d %s messages:", MAX_I3HISTORY,
		  channel->local_name);
      for (x = 0; x < MAX_I3HISTORY; x++)
	{
	  if (channel->history[x] != NULL)
	    i3printf (ch, "%s", channel->history[x]);
	  else
	    break;
	}
      return true;
    }

  if (!I3_is_connected ())
    {
      i3println (ch, "The mud is not currently connected to I3.");
      return true;
    }

  if (ch->pcdata->i3chardata->i3perm >= I3PERM_ADMIN
      && !str_cmp (argument, "log"))
    {
      if (!IsSet (channel->flags, I3CHAN_LOG))
	{
	  SetBit (channel->flags, I3CHAN_LOG);
	  i3printlnf (ch,
		      "{RFile logging enabled for %s, PLEASE don't forget to undo this when it isn't needed!",
		      channel->local_name);
	}
      else
	{
	  RemBit (channel->flags, I3CHAN_LOG);
	  i3printlnf (ch, "{GFile logging disabled for %s.",
		      channel->local_name);
	}
      rw_i3_channels (act_write);
      return true;
    }

  if (!hasname (ch->pcdata->i3chardata->i3_listen, channel->local_name))
    {
      i3printlnf (ch, "{YYou were trying to send something to an I3 "
		  "channel but you're not listening to it.\n\rPlease use the command "
		  "'{Wi3listen %s{Y' to listen to it.", channel->local_name);
      return true;
    }

  switch (argument[0])
    {
    case ',':

      argument++;
      while (isspace (*argument))
	argument++;
      I3_send_channel_emote (channel, GetName (ch), argument);
      break;
    case '@':

      argument++;
      while (isspace (*argument))
	argument++;
      I3_send_social (channel, ch, argument);
      break;
    default:
      I3_send_channel_message (channel, GetName (ch), argument);
      break;
    }
  return true;
}

#endif
