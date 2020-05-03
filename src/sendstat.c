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
* The Dawn of Time v1.69q (c)1997-2002 Michael Garratt                    *
* >> A number of people have contributed to the Dawn codebase, with the   *
*    majority of code written by Michael Garratt - www.dawnoftime.org     *
* >> To use this source code, you must fully comply with the dawn license *
*    in licenses.txt... In particular, you may not remove this copyright  *
*    notice.                                                              *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

// About: The code within this module sends to firstmud.com statistical
// information.  Using this information we are able to
// get an idea of the number of 1stmud based muds running and
// hopefully the number of players.
//
// The statistical information submitted to firstmud.com is a
// summary of mudstats, combined with a few other things specific to 
// your mud environment (such as the name of the mud).  You are free 
// to remove/add info as you please, but please leave the unique_id, 
// name, and version if possible.
//
// By default the mud after about 10 minutes of running will
// send the stats to http://www.firstmud.com/cgi-bin/1stmud/post.cgi
// This does not lag the mud in anyway, (unless your dns resolver
// is broken - use sockets to determine this), if the dns resolver
// is broken there may be a one off small delay (ordinarily less
// than 5 seconds) while the mud resolves the ip address of
// firstmud.com in order to know where to send the
// stats to.
//

#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "olc.h"

#ifndef DISABLE_SENDSTAT

// #define SENDSTAT_LOG_PROGRESS                1
#define SENDSTAT_SUBMIT_DOMAIN  "firstmud.com"
#define SENDSTAT_SUBMIT_URL     "/scripts/post.php"

void
sendstat_logf (const char *fmt, ...)
{
  char buf[MSL];
  va_list args;

#ifndef SENDSTAT_LOG_PROGRESS

  return;
#endif

  if (NullStr (fmt))
    return;

  va_start (args, fmt);
  vsnprintf (buf, MSL, fmt, args);
  va_end (args);

  logf ("sendstat: %s", buf);
}

const char *url_encode_table[] = {
  "%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07",
  "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
  "%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17",
  "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
  "+", "!", "%22", "%23", "$", "%25", "%26", "%27",
  "(", ")", "*", "%2B", ",", "-", ".", "%2F",
  "0", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
  "%40", "A", "B", "C", "D", "E", "F", "G",
  "H", "I", "J", "K", "L", "M", "N", "O",
  "P", "Q", "R", "S", "T", "U", "V", "W",
  "X", "Y", "Z", "%5B", "%5C", "%5D", "%5E", "_",
  "%60", "a", "b", "c", "d", "e", "f", "g",
  "h", "i", "j", "k", "l", "m", "n", "o",
  "p", "q", "r", "s", "t", "u", "v", "w",
  "x", "y", "z", "%7B", "%7C", "%7D", "%7E", "%7F",
  "%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87",
  "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
  "%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97",
  "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
  "%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7",
  "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
  "%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7",
  "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
  "%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7",
  "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
  "%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7",
  "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
  "%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7",
  "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
  "%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7",
  "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
};

char *
url_encode_post_data (const char *postdata)
{
  static char *result;
  unsigned char *s;
  const char *t;
  char *d;


  if (result)
    {
      free_mem (result);
    }
  alloc_mem (result, char, strlen (postdata) * 3 + 1);

  d = result;

  for (s = (unsigned char *) postdata; *s; s++)
    {
      t = url_encode_table[*s];
      while (*t)
	{
	  *d++ = *t++;
	}
    }
  *d = '\0';
  return result;
}

char *
sendstat_generate_statistics_text ()
{
  static char result[45000];
  char stats[45000];
  char *encoded;
  int len;

  stats[0] = NUL;

#define ENCODE_INT(field)			strcat(stats, FORMATF("&" # field"=%d", field))
#define ENCODE_INTH(field, header)	strcat(stats, FORMATF("&%s=%d", header, field))
#define ENCODE_BOOL(field)			strcat(stats, FORMATF("&" # field"=%s", !field ? "false" : "true"))
#define ENCODE_LONG(field)			strcat(stats, FORMATF("&" # field"=%ld", field))
#define ENCODE_LONGH(field, header) strcat(stats, FORMATF("&%s=%ld", header, field))
#define ENCODE_LONGHD(field, header) if(field > 0) strcat(stats, FORMATF("&%s=%ld", header, field))
#define ENCODE_STR(field)			strcat(stats, FORMATF("&" # field"=%s", url_encode_post_data(field)))
#define ENCODE_STRH(field, header)	strcat(stats, FORMATF("&%s=%s", header, url_encode_post_data(field)))
#define ENCODE_TIME(field)			strcat(stats, FORMATF("&" # field"=%s", url_encode_post_data(str_time(field, -1, NULL))))
#define ENCODE_TIMEH(field, header) strcat(stats, FORMATF("&%s=%s", header, url_encode_post_data(str_time(field, -1, NULL))))

  ENCODE_INTH (mud_info.unique_id, "unique_id");
  ENCODE_STRH (mud_info.name, "name");
  ENCODE_STRH (MUDVERSION, "version");
  ENCODE_STRH (mud_info.bind_ip_address, "bind_ip");
  ENCODE_INTH (mud_info.default_port, "default_port");
  ENCODE_INT (mainport);
#ifndef DISABLE_WEBSRV
  ENCODE_INT (webport);
#endif
  if (mud_info.disabled_signals)
    ENCODE_STRH (flag_string (signal_flags, mud_info.disabled_signals),
		 "disabled_signals");
  ENCODE_INTH (mud_info.min_save_lvl, "min_save_lvl");
  ENCODE_INTH (mud_info.group_lvl_limit, "group_lvl_range");
  ENCODE_INTH (mud_info.pcdam, "pcdam_mod");
  ENCODE_INTH (mud_info.mobdam, "mobdam_mod");
  ENCODE_INTH (mud_info.max_points, "creation_point_mod");
  if (top_descriptor)
    ENCODE_INTH (top_descriptor, "connections");
  ENCODE_INT (MAX_STATS);
  ENCODE_INTH (top_area, "areas");
  ENCODE_INTH (top_room_index, "rooms");
  ENCODE_INTH (top_shop, "shops");
  ENCODE_INTH (top_explored, "explorable_rooms");
  ENCODE_INTH (top_reset, "resets");
  ENCODE_INTH (top_exit, "exits");
  ENCODE_INTH (top_ed, "extra_descriptions");
  ENCODE_INTH (top_affect, "affects");
  ENCODE_INTH (top_char_index, "mobiles");
  ENCODE_INT (mobile_count);
  ENCODE_INTH (top_obj_index, "objects");
  ENCODE_INTH (top_obj, "object_count");
  if (top_mprog > 0)
    ENCODE_INTH (top_mprog, "mob_programs");
  if (top_oprog > 0)
    ENCODE_INTH (top_oprog, "obj_programs");
  if (top_rprog > 0)
    ENCODE_INTH (top_rprog, "room_programs");
  ENCODE_INTH (top_help, "helps");
  ENCODE_INTH (nAllocString, "str_count");
  ENCODE_INTH (sAllocString, "str_size");
  ENCODE_INTH (mud_info.share_value, "share_value");
  ENCODE_INTH (mud_info.stats.online, "max_online");
  ENCODE_INTH (mud_info.pulsepersec, "pulsepersec");
  ENCODE_LONGHD (mud_info.stats.logins, "logins");
  ENCODE_LONGHD (mud_info.stats.quests, "quests");
  ENCODE_LONGHD (mud_info.stats.qcomplete, "qcomplete");
  ENCODE_LONGHD (mud_info.stats.levels, "levels");
  ENCODE_LONGHD (mud_info.stats.newbies, "newbies");
  ENCODE_LONGHD (mud_info.stats.deletions, "deletions");
  ENCODE_LONGHD (mud_info.stats.mobdeaths, "mobdeaths");
  ENCODE_LONGHD (mud_info.stats.auctions, "auctions");
  ENCODE_LONGHD (mud_info.stats.aucsold, "aucsold");
  ENCODE_LONGHD (mud_info.stats.pdied, "pdied");
  ENCODE_LONGHD (mud_info.stats.pkill, "pkill");
  ENCODE_LONGHD (mud_info.stats.notes, "notes");
  ENCODE_LONGHD (mud_info.stats.remorts, "remorts");
  ENCODE_LONGHD (mud_info.stats.wars, "wars");
  ENCODE_LONGHD (mud_info.stats.gquests, "gquests");
  ENCODE_LONGHD (mud_info.stats.connections, "connections");
  ENCODE_LONGHD (mud_info.stats.boot_connects, "boot_connects");
  if (mud_info.last_copyover > 0)
    ENCODE_STRH (timestr (current_time - mud_info.last_copyover, false),
		 "last_copyover");
  if (mud_info.longest_uptime > 0)
    ENCODE_STRH (timestr (mud_info.longest_uptime, false), "longest_uptime");
  ENCODE_LONGHD (mud_info.stats.web_requests, "web_requests");
  ENCODE_LONGHD (mud_info.stats.chan_msgs, "chan_msgs");
  ENCODE_TIMEH (mud_info.stats.lastupdate, "stats_since");
  ENCODE_INTH (mud_info.stats.version, "stats_version");
  ENCODE_LONG (top_vnum_room);
  ENCODE_LONG (top_vnum_obj);
  ENCODE_LONG (top_vnum_mob);
  ENCODE_INT (LEVEL_HERO);
  ENCODE_INT (LEVEL_IMMORTAL);
  ENCODE_INT (MAX_LEVEL);
  ENCODE_TIME (current_time);
  ENCODE_INTH (top_race, "races");
  ENCODE_INTH (top_class, "classes");
  ENCODE_INTH (top_skill, "skills");
  ENCODE_INTH (top_group, "groups");
  ENCODE_INTH (top_social, "socials");
  ENCODE_INTH (top_clan, "clans");
  ENCODE_INTH (top_cmd, "commands");
  ENCODE_INTH (top_deity, "deities");
  ENCODE_INTH (top_song, "songs");
  ENCODE_STR (HOSTNAME);
  ENCODE_STR (UNAME);
  ENCODE_STR (CWDIR);
  ENCODE_STR (EXE_FILE);
  ENCODE_INTH (top_channel, "channels");
  ENCODE_INTH (pfiles.count, "Pfiles");
  ENCODE_STRH (flag_string (mud_flags, mud_info.mud_flags), "mudflags");
  ENCODE_TIME (boot_time);
#ifdef __cplusplus

  ENCODE_STRH ("yes", "Cplus");
#endif
#ifndef DISABLE_MCCP

  ENCODE_STRH ("yes", "MCCP");
#endif

  ENCODE_INT (MAX_KEY_HASH);
  ENCODE_INT (MAX_STRING_LENGTH);
  ENCODE_INT (MAX_INPUT_LENGTH);
  if (top_ban > 0)
    ENCODE_INTH (top_ban, "Bans");
  if (top_disabled > 0)
    ENCODE_INTH (top_disabled, "disabled_cmds");
  if (top_mbr > 0)
    ENCODE_INTH (top_mbr, "clan_members");
  if (top_msp > 0)
    ENCODE_INTH (top_msp, "msp_sounds");
#ifdef __DATE__

  ENCODE_STRH (__DATE__, "compiled_date");
#endif
#ifdef __TIME__

  ENCODE_STRH (__TIME__, "compiled_time");
#endif
#ifdef __CYGWIN__

  ENCODE_STRH ("cygwin", "compiled_platform");
#elif defined WIN32

  ENCODE_STRH ("Win32", "compiled_platform");
#elif defined unix
#ifdef linux

  ENCODE_STRH ("linux", "compiled_platform");
#elif defined __OpenBSD__

  ENCODE_STRH ("OpenBSD", "compiled_platform");
#elif defined __FreeBSD__

  ENCODE_STRH ("FreeBSD", "compiled_platform");
#elif defined __NetBSD__

  ENCODE_STRH ("NetBSD", "compiled_platform");
#elif defined BSD

  ENCODE_STRH ("BSD", "compiled_platform");
#else

  ENCODE_STRH ("unix", "compiled_platform");
#endif
#else

  ENCODE_STRH ("unknown", "compiled_platform");
#endif


#ifdef __VERSION__

  ENCODE_STRH (__VERSION__, "compiler_version");
#endif

  ENCODE_STRH (get_platform_info (), "platform_info");

  encoded = &stats[1];
  len = strlen (encoded);

  sprintf (result,
	   "POST " SENDSTAT_SUBMIT_URL "  HTTP/1.1\r\n"
	   "Content-Type: application/x-www-form-urlencoded\r\n"
	   "Host: " SENDSTAT_SUBMIT_DOMAIN "\r\n"
	   "User-Agent: Mozilla/4.0 (compatible; " MUDNAME
	   "SendStat/1.0;)\r\n" "Content-Length: %d\r\n"
	   "Cache-Control: no-cache\r\n" "\r\n%s", len, encoded);

  return result;
}

char *sendstat_stattext_to_post;

typedef enum
{
  SENDSTATSTAGE_WAIT,
  SENDSTATSTAGE_DOMAIN_RESOLVED,
  SENDSTATSTAGE_CONNECT_IN_PROGRESS,
  SENDSTATSTAGE_GENERATE_STATS,
  SENDSTATSTAGE_POSTING,
  SENDSTATSTAGE_CLOSE_CONNECT,
  SENDSTATSTAGE_COMPLETED,
  SENDSTATSTAGE_ABORTED
}
sendstat_stages;

sendstat_stages sendstat_stage = SENDSTATSTAGE_WAIT;
time_t sendstat_connect_timeout = 0;

struct in_addr sendstat_address;
static SOCKET sendstat_socket;


void
sendstat_resolve_domain ()
{

  if (!inet_aton (SENDSTAT_SUBMIT_DOMAIN, &sendstat_address))
    {
      if (sendstat_address.s_addr == 0)
	{
	  struct hostent *hostp = gethostbyname (SENDSTAT_SUBMIT_DOMAIN);

	  if (!hostp)
	    {

	      sendstat_logf ("failed to resolve '%s.'",
			     SENDSTAT_SUBMIT_DOMAIN);
	      sendstat_stage = SENDSTATSTAGE_ABORTED;
	      return;
	    }
	  memcpy (&sendstat_address, hostp->h_addr, hostp->h_length);
	}
    }
  sendstat_logf ("resolved '%s' as %s",
		 SENDSTAT_SUBMIT_DOMAIN, inet_ntoa (sendstat_address));
  sendstat_stage = SENDSTATSTAGE_DOMAIN_RESOLVED;
}

struct sockaddr_in sockaddress;

void
sendstat_initiate_connection ()
{
  SOCKET nRet;

  sendstat_socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sendstat_socket == INVALID_SOCKET)
    {
      sendstat_logf ("Error creating connection socket");
      return;
    }

  sockaddress.sin_family = AF_INET;
  sockaddress.sin_addr.s_addr = sendstat_address.s_addr;
  sockaddress.sin_port = htons (80);

  if (!socket_cntl (sendstat_socket))
    {
      sendstat_logf ("error setting new socket to nonblocking");
      sendstat_stage = SENDSTATSTAGE_ABORTED;
      return;
    }

  nRet = connect (sendstat_socket, (SOCKADDR *) & sockaddress,
		  sizeof (sockaddress));

  if (nRet == 0)
    {				// successful connection, jump straight to generating the stats to post
      sendstat_stage = SENDSTATSTAGE_GENERATE_STATS;
      return;
    }

#ifdef WIN32
  if (WSAGetLastError () != WSAEWOULDBLOCK)
    {
      sendstat_logf
	("sendstat_initiate_connection(): connect() error %d",
	 WSAGetLastError ());
      sendstat_stage = SENDSTATSTAGE_ABORTED;
      return;
    }
#else
  if (nRet < 0)
    {
      if (errno != EINPROGRESS && errno != EALREADY)
	{
	  sendstat_logf
	    ("sendstat_initiate_connection(): connect() error %d", errno);
	  sendstat_stage = SENDSTATSTAGE_ABORTED;
	  return;
	}
    }
#endif

  sendstat_connect_timeout = current_time + 200;

  sendstat_logf ("connection initiation successful");
  sendstat_stage = SENDSTATSTAGE_CONNECT_IN_PROGRESS;
  return;
}

void
sendstat_process_connect ()
{
  SOCKET nRet;

  if (sendstat_connect_timeout < current_time)
    {
      sendstat_logf
	("sendstat_process_connect(): pending connection timed out.");
      sendstat_stage = SENDSTATSTAGE_ABORTED;
      return;
    }

  sendstat_logf ("processing connection %s:80", inet_ntoa (sendstat_address));
#ifdef WIN32
  {


    struct timeval select_timeout;
    fd_set fdWrite;
    fd_set fdExcept;

    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 0;


    FD_ZERO (&fdWrite);
    FD_SET (sendstat_socket, &fdWrite);
    FD_ZERO (&fdExcept);
    FD_SET (sendstat_socket, &fdExcept);


    nRet = select (sendstat_socket + 1, NULL, &fdWrite, &fdExcept,
		   &select_timeout);

    if (nRet < 1)
      {
	if (nRet)
	  {
	    sendstat_logf
	      ("sendstat_process_connect(): select() returned error");
	    sendstat_stage = SENDSTATSTAGE_ABORTED;
	  }
	return;
      }

    if (FD_ISSET (sendstat_socket, &fdWrite))
      {

	sendstat_logf ("sendstat_process_connect(): connection established.");
	sendstat_stage = SENDSTATSTAGE_GENERATE_STATS;
	return;
      }

    if (FD_ISSET (sendstat_socket, &fdExcept))
      {

	sendstat_logf ("sendstat_process_connect(): connection failed.");
      }
    else
      {
	sendstat_logf
	  ("sendstat_process_connect(): don't know how we got here!");
      }
    sendstat_stage = SENDSTATSTAGE_ABORTED;
    return;
  }
#else


  nRet = connect (sendstat_socket, (SOCKADDR *) & sockaddress,
		  sizeof (sockaddress));

  if (nRet == 0)
    {

      sendstat_stage = SENDSTATSTAGE_GENERATE_STATS;
      return;
    }
  if (nRet < 0)
    {
      if (errno != EINPROGRESS && errno != EALREADY)
	{
	  sendstat_logf ("sendstat_process_connect(): connect() error %d",
			 errno);
	  sendstat_stage = SENDSTATSTAGE_ABORTED;
	  return;
	}
    }
#endif
  return;
}

void
sendstat_generate_stats ()
{
  sendstat_stattext_to_post = sendstat_generate_statistics_text ();
  if (NullStr (sendstat_stattext_to_post))
    {
      sendstat_logf
	("sendstat_generate_stats(): An error occured generating statistics.");
      sendstat_stage = SENDSTATSTAGE_ABORTED;
    }
  else
    sendstat_stage = SENDSTATSTAGE_POSTING;
}

void
sendstat_post ()
{
  char *msg = sendstat_stattext_to_post;

  int written;
  int msglen = strlen (msg);

  written = write_to_socket (sendstat_socket, msg, msglen);

  if (written < 0)
    {
      sendstat_logf ("sendstat_post(): An error occured posting statistics.");
      sendstat_stage = SENDSTATSTAGE_ABORTED;
      return;
    }

  if (written < msglen)
    {
      sendstat_logf
	("Incomplete write, sent %d bytes of %d, write rest later",
	 written, msglen);
      sendstat_stattext_to_post += written;
      return;
    }

  sendstat_logf ("Submitted %d bytes", written);

  sendstat_stage = SENDSTATSTAGE_CLOSE_CONNECT;
}
#endif

Do_Fun (do_sendstat)
{
#ifdef DISABLE_SENDSTAT
  chprintlnf (ch,
	      "Posting %s statistics to http://" SENDSTAT_SUBMIT_DOMAIN
	      "/muds/%d.php is disabled.", mud_info.name, mud_info.unique_id);
#else
  while (sendstat_stage != SENDSTATSTAGE_ABORTED
	 && sendstat_stage != SENDSTATSTAGE_COMPLETED)
    {
      sendstat_resolve_domain ();
      sendstat_initiate_connection ();
      sendstat_process_connect ();
      sendstat_generate_stats ();
      sendstat_post ();
      closesocket (sendstat_socket);
      sendstat_stage = SENDSTATSTAGE_COMPLETED;
    }

  if (sendstat_stage == SENDSTATSTAGE_COMPLETED)
    chprintlnf (ch,
		"%s statistics posted on http://" SENDSTAT_SUBMIT_DOMAIN
		"/muds/%d.php", mud_info.name, mud_info.unique_id);
  else
    chprintlnf (ch,
		"There was an error posting %s statistics on http://"
		SENDSTAT_SUBMIT_DOMAIN "/muds/%d.php", mud_info.name,
		mud_info.unique_id);
#endif
}

void
sendstat_update (void)
{
#ifndef DISABLE_SENDSTAT
  static time_t wait_until = 0;



  if ((int) sendstat_stage < (int) SENDSTATSTAGE_COMPLETED)
    {
      if (!wait_until || sendstat_stage != SENDSTATSTAGE_WAIT)
	{
	  sendstat_logf ("sendstat_update(%d)", (int) sendstat_stage);
	}
    }

  switch (sendstat_stage)
    {
    case SENDSTATSTAGE_WAIT:
      if (wait_until == 0)
	{
	  wait_until = current_time + MINUTE * 30;
	}
      else if (wait_until < current_time)
	{

	  wait_until = 0;
	  sendstat_logf ("moving on to resolving stage.");
	  sendstat_resolve_domain ();
	}
      break;

    case SENDSTATSTAGE_DOMAIN_RESOLVED:
      sendstat_logf ("initiating connection to '%s:80'",
		     inet_ntoa (sendstat_address));
      sendstat_initiate_connection ();
      break;

    case SENDSTATSTAGE_CONNECT_IN_PROGRESS:
      sendstat_logf ("processing connection.");
      sendstat_process_connect ();
      break;

    case SENDSTATSTAGE_GENERATE_STATS:
      sendstat_logf ("generating statistics.");
      sendstat_generate_stats ();
      break;

    case SENDSTATSTAGE_POSTING:
      sendstat_logf ("posting statistics.");
      sendstat_post ();
      break;

    case SENDSTATSTAGE_CLOSE_CONNECT:
      sendstat_logf ("closing socket.");
      closesocket (sendstat_socket);
      sendstat_stage = SENDSTATSTAGE_COMPLETED;
      break;

    case SENDSTATSTAGE_COMPLETED:
    case SENDSTATSTAGE_ABORTED:

      {
	static time_t redo_in = 0;

	if (redo_in)
	  {
	    if (redo_in < current_time)
	      {
		sendstat_logf ("restarting sendstats");

		sendstat_stage = SENDSTATSTAGE_WAIT;
		redo_in = 0;
	      }
	  }
	else
	  {
	    redo_in = current_time + (24 * HOUR);
	  }

      }
    default:
      break;
    };
#endif
}
