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

#ifndef __MERC_H_
#define __MERC_H_    	1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <io.h>
#include <process.h>
#include <winsock2.h>
#include <sys\timeb.h>
#include <direct.h>
#include <lmcons.h>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif
#pragma comment( lib, "wsock32.lib")
#pragma warning(disable:4244)
#pragma warning(disable:4018)
#pragma warning(disable:4305)
#pragma warning(disable:4761)
#pragma warning(disable:4800)
#pragma warning(disable:4309)
#pragma warning(disable:4550)
#ifndef __FUNCTION__
#define __FUNCTION__    	__FILE__
#endif
#ifndef NO_INITIAL_ALPHA_PFILEDIRS
#define NO_INITIAL_ALPHA_PFILEDIRS
#endif
#endif
#ifdef __CYGWIN__
#undef HAVE_SETITIMER
#undef HAVE_ALARM
#ifndef NO_INITIAL_ALPHA_PFILEDIRS
#define NO_INITIAL_ALPHA_PFILEDIRS
#endif
#endif
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#include <limits.h>
#include <fcntl.h>
#include <assert.h>
#include <setjmp.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#ifdef HAVE_MYSQL_H
#include <mysql.h>
#endif
#ifndef WIN32
#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#endif
#if !HAVE_WORKING_VFORK
# define vfork fork
#endif
#include <sys/resource.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pwd.h>
#endif
#ifndef __GNUC__
# ifdef HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca
char *alloca ();
#   endif
#  endif
# endif
#endif

#ifndef DISABLE_MCCP
#ifdef WIN32

#pragma comment( lib, "zdll.lib")
#endif
#include <zlib.h>
#endif

#include "ansi.h"

#include "macro.h"

#include "typedef.h"

#include "fileio.h"

#include "defines.h"

#ifndef DISABLE_I3
#include "i3.h"
#endif

#include "structs.h"

#include "bits.h"

#include "index.h"

#include "globals.h"

#include "proto.h"


#if    	defined DISABLE_CRYPT || !defined HAVE_CRYPT
#define crypt(s1, s2)    	(s1)
#elif defined HAVE_CRYPT_H
#include <crypt.h>
#endif



#ifdef WIN32
#define DIR_SYM     "\\"
#define NULL_FILE   "nul"
#define WIN_DIR		".."DIR_SYM"win"DIR_SYM
#else
#define DIR_SYM     "/"
#define NULL_FILE    	"/dev/null"
#endif

#ifndef PLAYER_DIR
#define PLAYER_DIR      ".."DIR_SYM"player"DIR_SYM
#endif
#define PLAYER_BACKUP	PLAYER_DIR"backup"DIR_SYM
#define DELETE_DIR		PLAYER_DIR"deleted"DIR_SYM
#ifndef DATA_DIR
#define DATA_DIR        ".."DIR_SYM"data"DIR_SYM
#endif
#ifndef AREA_DIR
#define AREA_DIR    	    ".."DIR_SYM"area"DIR_SYM
#endif
#ifndef BIN_DIR
#define BIN_DIR         ".."DIR_SYM"bin"DIR_SYM
#endif
#ifndef LOG_DIR
#define LOG_DIR         ".."DIR_SYM"log"DIR_SYM
#endif
#ifndef NOTE_DIR
#define NOTE_DIR      	    ".."DIR_SYM"notes"DIR_SYM
#endif

#define AREA_LIST       AREA_DIR "area.lst"
#define BUG_FILE        DATA_DIR "bugs.txt"
#define TYPO_FILE       DATA_DIR "typos.txt"
#define SHUTDOWN_FILE   DATA_DIR "shutdown.txt"
#define HELP_FILE       DATA_DIR "help.dat"
#define BAN_FILE    	    DATA_DIR "ban.dat"
#define MUSIC_FILE    	    DATA_DIR "music.dat"
#define DISABLED_FILE    	DATA_DIR "disabled.dat"
#define STAT_FILE       DATA_DIR "statlist.dat"
#define    	GQUEST_FILE    	    	DATA_DIR "gquest.dat"
#define WAR_FILE    	    	DATA_DIR "war.dat"
#define CLAN_FILE       DATA_DIR "clans.dat"
#define COMMAND_FILE    DATA_DIR "commands.dat"
#define SKILL_FILE      DATA_DIR "skills.dat"
#define GROUP_FILE      DATA_DIR "groups.dat"
#define RACE_FILE       DATA_DIR "races.dat"
#define CLASS_FILE      DATA_DIR "classes.dat"
#define SOCIAL_FILE     DATA_DIR "social.dat"
#define CORPSE_FILE     DATA_DIR "corpses.dat"
#define DEITY_FILE      DATA_DIR "deity.dat"
#define WPWD_FILE       DATA_DIR "webpass.dat"
#define MBR_FILE        DATA_DIR "mbr.dat"
#define    	CHANNEL_FILE    	DATA_DIR "channels.dat"
#define MUD_FILE        DATA_DIR "mud.dat"
#define TIME_FILE    	    	DATA_DIR "time.dat"
#define COPYOVER_FILE   DATA_DIR "copyover.txt"
#define ROOM_OBJS_FILE  DATA_DIR "room_objs.dat"
#define CRS_FILE        DATA_DIR "crs.dat"
#define PLR_INDEX_FILE  DATA_DIR "plr_index.dat"
#define NAMEGEN_FILE    DATA_DIR "namegen.dat"
#define LOG_INDEX_FILE  LOG_DIR "index.log"
#define WIZ_FILE		DATA_DIR "wizlist.dat"

#endif
