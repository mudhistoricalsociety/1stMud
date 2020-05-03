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
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "olc.h"
#include "telnet.h"
#include "data_table.h"
#include "vnums.h"

Proto (void game_loop, (SOCKET));
Proto (SOCKET init_socket, (int));
Proto (void init_descriptor, (int));


Proto (int main, (int, char **));
Proto (void nanny, (Descriptor *, const char *));

Proto (bool process_output, (Descriptor *, bool));
Proto (void read_from_buffer, (Descriptor *));
Proto (void stop_idling, (CharData *));
Proto (void set_game_levels, (int, int));

Proto (bool check_directories, (const char *));
Proto (bool create_directories, (void));
Proto (int print_stripped_client_code, (Descriptor *, const char *, int));

Proto (void log_release_held_logs, (void));
Proto (void log_hold_till_commandline_options_parsed, (void));

#ifdef WIN32
void
gettimeofday (struct timeval *t, void *tz)
{
  struct timeb timebuffer;

  ftime (&timebuffer);
  t->tv_sec = timebuffer.time;
  t->tv_usec = timebuffer.millitm * 1000;
}
#endif

const char *
whoami (void)
{
#ifdef WIN32
  static char username[UNLEN + 1];
  unsigned long szusername = UNLEN + 1;

  if (GetUserName (username, &szusername))
    {
      return (username);
    }
#elif defined HAVE_GETUID && defined HAVE_GETPWUID
  struct passwd *pwd;
  uid_t uid;

  uid = getuid ();

  if ((pwd = getpwuid (uid)))
    {
      return (pwd->pw_name);
    }
#endif
  return "unknown";
}

char *
get_platform_info (void)
{
  static char buf[2048];

#ifdef WIN32

  OSVERSIONINFO osvi;

  osvi.dwOSVersionInfoSize = sizeof (osvi);
  GetVersionEx (&osvi);

  sprintf (buf,
	   "%s v%d.%d.%d [%s]",
	   osvi.dwPlatformId == VER_PLATFORM_WIN32s ? "Win32s" :
	   osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ? "Windows9x" :
	   osvi.dwPlatformId == VER_PLATFORM_WIN32_NT ? "WindowsNT" :
	   FORMATF ("Unknown(%d)", osvi.dwPlatformId),
	   osvi.dwMajorVersion,
	   osvi.dwMinorVersion, osvi.dwBuildNumber, osvi.szCSDVersion);
#elif defined HAVE_SYS_UTSNAME_H

  struct utsname name;

  if (uname (&name) == -1)
    {
      sprintf (buf, "Unknown-uname_error%d", errno);
    }
  else
    {
      sprintf (buf, "sysname='%s' nodename='%s' "
	       "release='%s' version='%s' machine='%s'",
	       name.sysname,
	       name.nodename, name.release, name.version, name.machine);
    }
#if defined HAVE_SYS_SYSCTL_H && defined CTL_KERN && defined KERN_VERSION

  int mib[2];
  char kernver[512];
  size_t len;

  mib[0] = CTL_KERN;
  mib[1] = KERN_VERSION;
  len = sizeof (kernver);
  if (sysctl (mib, 2, &kernver, &len, NULL, 0) == 0)
    {
      kernver[sizeof (kernver) - 1] = '\0';
      strcat (buf, " kernver='");
      strcat (buf, strip_cr (kernver));
      strcat (buf, "'");
    }
#endif
#elif defined __CYGWIN__
  strcpy (buf, "Cygwin.");
#else

  strcpy (buf, "Unknown.");
#endif

  return buf;
}

time_t
getcurrenttime (void)
{
  struct timeval last_time;

  gettimeofday (&last_time, NULL);
  current_time = (time_t) last_time.tv_sec;
  return current_time;
}

void
sleep_seconds (int seconds)
{
#ifdef WIN32
  Sleep (seconds * 1000);
#else
  sleep (seconds);
#endif
}

#ifdef WIN32
int winsock_error = SOCKET_ERROR;

#define WIN32_USE_WINSOCK2
#endif


void
init_network (void)
{
#ifdef WIN32

#ifdef WIN32_USE_WINSOCK2
  WORD wVersionRequested = MAKEWORD (2, 0);
#else
  WORD wVersionRequested = MAKEWORD (1, 1);
#endif
  WSADATA wsaData;

  if (winsock_error == NO_ERROR)
    return;


  if ((winsock_error = WSAStartup (wVersionRequested, &wsaData)) != NO_ERROR)
    {
      log_string ("Couldn't initialize winsock.");
      exit (1);
    }
#endif
}


void
close_network (void)
{
#ifdef WIN32
  if (winsock_error == NO_ERROR && crs_info.status != CRS_COPYOVER)
    {
      WSACleanup ();
      winsock_error = SOCKET_ERROR;
    }
#endif
}

void
init_globals (char *exename)
{

  if (gethostname (HOSTNAME, sizeof (HOSTNAME)) == -1 || NullStr (HOSTNAME))
    strcpy (HOSTNAME, "localhost");

  if (!getcwd (CWDIR, sizeof (CWDIR)) || NullStr (CWDIR))
    strcpy (CWDIR, AREA_DIR);

#if defined WIN32 || defined __CYGWIN__
  do
    {
      char *p, sym[2] = DIR_SYM;

      if ((p = strrchr (exename, sym[0])) != NULL)
	{
	  p++;
	  strcpy (exename, p);
	}

      if (str_suffix (".exe", exename))
	strcat (exename, ".exe");

#ifdef NOT_WINDOWS_NT
/* Use this part if your not using windows NT/XP */
      do
	{
	  struct stat chk;
	  int i;

	  if (!str_suffix ("2.exe", exename))
	    strcpy (EXE_FILE, str_rep (exename, "2.exe", ".exe"));
	  else
	    strcpy (EXE_FILE, str_rep (exename, ".exe", "2.exe"));

	  if (stat (EXE_FILE, &chk) == -1)
	    {
	      char *shell = getenv ("ComSpec");	/* hopefully command.com ! */
	      if (shell)
		{
		  system (FORMATF
			  ("%s /c copy %s %s", shell, exename, EXE_FILE));
		  break;
		}
	    }

	  logf ("%s not found! Copyover unavailable!", EXE_FILE);
	}
      while (0);
#endif
    }
  while (0);
#endif

  strcpy (EXE_FILE, exename);

  strcpy (UNAME, whoami ());
}

EXTERN bool log_hold_log_string_core_stdout_restore_value;
int parsed_mainport;

void
cmdline_help (char **argv)
{
  log_string ("Commands:");
  log_string ("-v, --version                    displays version info");
  log_string
    ("-r, --relvl [old max] [new max]  change max level on game data");
  log_string
    ("-s, --signal <all|#sig>          disables signal handling on #sig number");
  log_string ("-nb, --nobackground              don't run in daemon mode");
  log_string ("-nl, --nologfile                 don't write to a logfile.");
  log_string
    ("-lc, --logconsole                always log to the console (win32 default).");
  log_string ("--createdirs                     create missing directories");
  log_string ("Startup Syntax:");
  logf ("%s [#port]                       sets the port number", argv[0]);
  log_string ("Where the port number is in the range 1024 to 65535");
  log_string ("Default port can be configured in " MUD_FILE ".");
}


bool
parse_cmdline_options (int argc, char **argv)
{
  int i;

  mainport = 0;
  parsed_mainport = 0;

#ifdef WIN32
  mud_info.cmdline_options =
    CMDLINE_NO_BACKGROUND_PROCESS | CMDLINE_LOG_CONSOLE;
#else
  mud_info.cmdline_options = 0;
#endif

  for (i = 1; i < argc; i++)
    {
      if (!str_cmp ("-q", argv[i]) || !str_cmp ("--quiet", argv[i]))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_QUIET);
	  log_hold_log_string_core_stdout_restore_value = false;
	  log_string ("Starting in quiet mode, no logging to stdout.");
	  if (IsSet (mud_info.cmdline_options, CMDLINE_LOG_CONSOLE))
	    {
	      log_string
		("The quiet mode doesn't make much sense if the log console option has been selected.");
	      return false;
	    }
	}
      else if (!str_cmp (argv[i], "-nb")
	       || !str_cmp (argv[i], "--nobackground"))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_NO_BACKGROUND_PROCESS);
	  log_string ("Disabling daemon mode...");
	}
      else if (!str_cmp (argv[i], "-nl") || !str_cmp (argv[i], "--nologfile"))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_NO_LOGFILE);
	  log_string ("Disabling use of a log file...");
	}
      else if (!str_cmp (argv[i], "lc") || !str_cmp (argv[i], "--logconsole"))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_LOG_CONSOLE);
	  log_string ("Logging to console...");
	  if (IsSet (mud_info.cmdline_options, CMDLINE_QUIET))
	    {
	      log_string
		("The log console option doesn't make much sense if the quiet option has been selected.");
	      log_hold_log_string_core_stdout_restore_value = true;
	      return false;
	    }
	}
      else if (!str_cmp ("-nolc", argv[i])
	       || !str_cmp ("--nologconsole", argv[i]))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_LOG_CONSOLE);
	  log_string ("Logging to console disabled...");
	}
      else if (!str_cmp ("-f", argv[i]) || !str_cmp ("--foreground", argv[i]))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_NO_BACKGROUND_PROCESS);
	  SetBit (mud_info.cmdline_options, CMDLINE_LOG_CONSOLE);
	  log_string ("Running mud in foreground...");
	  if (IsSet (mud_info.cmdline_options, CMDLINE_QUIET))
	    {
	      log_string
		("The foreground option doesn't make much sense if the quiet option has been selected.");
	      log_string
		("Try using -q -nb if you want the mud to run in the foreground with no console logging.");
	      log_hold_log_string_core_stdout_restore_value = true;
	      return false;
	    }
	}
      else if (!str_cmp ("--startup-script", argv[i]))
	{
	  SetBit (mud_info.cmdline_options, CMDLINE_NO_BACKGROUND_PROCESS);
	  SetBit (mud_info.cmdline_options, CMDLINE_STARTUP_SCRIPT);
	}
      else if (!str_cmp (argv[i], "-s") || !str_cmp (argv[i], "--signal"))
	{
	  bool found = false, all_sigs = false;
	  int s;
	  char buf[MSL];

	  i++;

	  if (!str_cmp (argv[i], "all"))
	    {
	      all_sigs = true;
	      log_string ("All signal handling disabled...");
	    }
	  buf[0] = NUL;
	  for (s = 0; sig_table[s].name != NULL; s++)
	    {
	      if (all_sigs || !str_prefix (argv[i], sig_table[s].name))
		{
		  SetBit (mud_info.disabled_signals,
			  MakeBit (sig_table[s].sig));
		  found = true;
		  if (!all_sigs)
		    {
		      logf ("Disabling %s signal handling...",
			    sig_table[s].name);
		      break;
		    }
		}
	      strcat (buf, " ");
	      strcat (buf, sig_table[s].name);
	    }
	  if (!found)
	    {
	      logf ("Usage: %s %s <#sig>|all", argv[0], argv[i]);
	      logf ("Available signals:%s", buf);
	      return false;
	    }
	  else
	    SetBit (mud_info.cmdline_options, CMDLINE_DISABLE_SIGNALS);
	}
      else if (!str_cmp (argv[i], "-v") || !str_cmp (argv[i], "--version"))
	{
	  logf (MUDSTRING ": Compiled on " __DATE__ " at " __TIME__ ".");
	  return false;
	}
      else if (!str_cmp (argv[i], "-c") || !str_cmp (argv[i], "--copyover"))
	{
	  crs_info.status = CRS_COPYOVER;
	  mud_control = atoi (argv[++i]);
	  parsed_mainport = atoi (argv[++i]);
#ifndef DISABLE_I3
	  I3_control = atoi (argv[++i]);
#else
	  i++;
#endif
	  SetBit (mud_info.cmdline_options, CMDLINE_COPYOVER);
	}
      else if (!str_cmp (argv[i], "--relvl") || !str_cmp (argv[i], "-r"))
	{
	  int oldlev, newlev;

	  i++;

	  if (NullStr (argv[i])
	      || !is_number (argv[i]) || (oldlev = atoi (argv[i])) <= 0)
	    {
	      logf
		("Usage: %s %s [old max level] [new max level]",
		 argv[0], argv[i]);
	      return false;
	    }

	  i++;

	  if (NullStr (argv[i])
	      || !is_number (argv[i]) || (newlev = atoi (argv[i])) <= 0)
	    {
	      logf
		("Usage: %s %s [old max level] [new max level]",
		 argv[0], argv[i]);
	      return false;
	    }

	  boot_db ();
	  set_game_levels (oldlev, newlev);
	  return false;
	}
      else if (!str_cmp ("--createdirs", argv[i]))
	{
	  if (create_directories ())
	    {
	      log_string
		("There may have been problems creating the directories...\n"
		 "create them manually then try again.");
	    }
	  else
	    {
	      log_string ("Directory creation completed successfully...\n"
			  "Start the mud normally to continue.");
	    }
	  return false;
	}
      else if (!str_cmp (argv[i], "-h") || !str_cmp (argv[i], "--help"))
	{
	  cmdline_help (argv);
	  return false;
	}
      else if (!IsSet (mud_info.cmdline_options, CMDLINE_SET_PORT)
	       && !NullStr (argv[i]) && is_number (argv[i]))
	{
	  if ((parsed_mainport = atoi (argv[i])) <= 1024
	      || parsed_mainport >= 65535)
	    {
	      log_string ("Port number must be between 1024 and 65535.");
	      return false;
	    }
	  SetBit (mud_info.cmdline_options, CMDLINE_SET_PORT);
	}
      else
	{
	  logf ("Invalid option '%s'...", argv[i]);
	  cmdline_help (argv);
	  return false;
	}
    }
  return true;
}

int
main (int argc, char **argv)
{
  log_hold_till_commandline_options_parsed ();


  run_level = RUNLEVEL_INIT;
  tzset ();
  boot_time = getcurrenttime ();

#ifdef HAVE_GETUID

  if (getuid () == 0)
    {
      log_string
	("DO NOT RUN THE MUD AS ROOT!!!  THIS IS A SECURITY RISK!!!");
      exit (1);
    }
#endif

  if (!parse_cmdline_options (argc, argv))
    {
      mainport = parsed_mainport;
      log_release_held_logs ();
      return 0;
    }

  if (check_directories (argv[0]))
    {
      log_release_held_logs ();
      exit (1);
    }
  mainport = parsed_mainport;

  log_release_held_logs ();


  if ((fpReserve = fopen (NULL_FILE, "r")) == NULL)
    {
      log_error (NULL_FILE);
      exit (1);
    }

  rw_mud_data (act_read);


  if (mainport == 0)
    {
      mainport = mud_info.default_port;
      logf
	("no mainport value specified on command line, using default value of %d",
	 mainport);
    }


  init_network ();

  init_globals (argv[0]);


  logf ("Starting up %s...", mud_info.name);

  if (mud_control == INVALID_SOCKET)
    {
      if ((mud_control = init_socket (mainport)) == INVALID_SOCKET)
	exit (1);
    }

#ifndef DISABLE_WEBSRV
  webport = mainport + mud_info.webport_offset;
  if (!web_is_connected ())
    web_control = init_socket (webport);
#endif

#ifndef DISABLE_I3

  I3_main (crs_info.status == CRS_COPYOVER);
#endif

  boot_db ();
  set_signals ();
  logf ("%s is ready to rock on %s, port %d.", mud_info.name, HOSTNAME,
	mainport);
  logf ("Mud is running in the %s with a process id of %d",
	IsSet (mud_info.cmdline_options,
	       CMDLINE_NO_BACKGROUND_PROCESS) ? "foreground" : "background",
	getpid ());
  if (IsSet (mud_info.cmdline_options, CMDLINE_NO_BACKGROUND_PROCESS)
      && !IsSet (mud_info.cmdline_options, CMDLINE_STARTUP_SCRIPT))
    {
      logf
	("Pressing ctrl+c will terminate the mud process (unless you have copyovered)");
    }
  if (crs_info.status == CRS_COPYOVER)
    copyover_recover ();
  game_loop (mud_control);
  closesocket (mud_control);

  log_string ("Normal termination of game.");
  exit (0);
  return 0;
}

bool
bind_ip (SOCKET fd, int port)
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;

  sa = sa_zero;
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = INADDR_ANY;

  if (!NullStr (mud_info.bind_ip_address))
    {
      if (strlen (mud_info.bind_ip_address) < 7)
	{
	  log_string ("The 'bind_ip_address' line in " MUD_FILE
		      " is too short to be a valid ip address");
	  log_string ("A valid example of bind_ip_address might read:");
	  log_string ("bind_ip_address 127.0.0.1~");
	  log_string ("Fix this using a text editor then restart the mud.");
	  return false;
	}
      if (strlen (mud_info.bind_ip_address) > 15)
	{
	  log_string ("The 'bind_ip_address' line in " MUD_FILE
		      " is missing a trailing ~ or is too long to be a valid ip address");
	  log_string ("A valid example of bind_ip_address in might read:");
	  log_string ("bind_ip_address 127.0.0.1~");
	  log_string ("Fix this using a text editor then restart the mud.");
	  return false;
	}

      sa.sin_addr.s_addr = inet_addr (mud_info.bind_ip_address);
      if (str_cmp (mud_info.bind_ip_address, "0.0.0.0"))
	logf ("Binding to '%s' as per bind_ip_address setting in "
	      MUD_FILE, mud_info.bind_ip_address);
    }

  if (bind (fd, (SOCKADDR *) & sa, sizeof (sa)) == SOCKET_ERROR)
    {
      socket_error ("Init socket: bind");

      if (NullStr (mud_info.bind_ip_address)
	  || !str_cmp ("0.0.0.0", mud_info.bind_ip_address))
	{
	  if (!str_cmp ("0.0.0.0", mud_info.bind_ip_address))
	    {
	      log_string
		("NOTE: If you have to bind to a specific IP address (NOT port) "
		 "you can specify it in " MUD_FILE
		 " by changing the keyword " "bind_ip_address entry:");
	      log_string
		("If this relates to you, you can achieve this by editing the "
		 MUD_FILE " file "
		 "and change the '0.0.0.0' in the line 'bind_ip_address 0.0.0.0~' to the "
		 "ip address your mud must listen on.");
	      log_string ("e.g. 'bind_ip_address 127.0.0.1~'");
	      log_string
		("The 127.0.0.1 example will make the mud listen for connections "
		 "only on 127.0.0.1 (localhost).");
	    }
	  else
	    {
	      log_string
		("NOTE: If you have to bind to a specific IP address (NOT port) "
		 "you can specify it in " MUD_FILE
		 " by adding a bind_ip_address entry:");
	      log_string ("e.g. add a line 'bind_ip_address 127.0.0.1~'");
	    }
	}
      else
	{
	  logf
	    ("NOTE: Mud attempted to bind to the specific IP Address '%s' as per the bind_ip_address "
	     "setting in " MUD_FILE
	     ", remove that setting if appropriate.",
	     mud_info.bind_ip_address);
	}
      return false;
    }
  return true;
}

SOCKET
init_socket (int port)
{
  int x = 1;
  SOCKET fd;

  if ((fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
      socket_error ("Init_socket: socket");
      return INVALID_SOCKET;
    }

  if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof (x)) ==
      SOCKET_ERROR)
    {
      socket_error ("Init_socket: SO_REUSEADDR");
      closesocket (fd);
      return INVALID_SOCKET;
    }
#if defined SO_DONTLINGER && !defined SYSV
  {
    struct linger ld;

    ld.l_onoff = 1;
    ld.l_linger = 1000;

    if (setsockopt
	(fd, SOL_SOCKET, SO_DONTLINGER, (char *) &ld,
	 sizeof (ld)) == SOCKET_ERROR)
      {
	socket_error ("Init_socket: SO_DONTLINGER");
	closesocket (fd);
	return INVALID_SOCKET;
      }
  }
#endif

  if (!bind_ip (fd, port))
    {
      closesocket (fd);
      return INVALID_SOCKET;
    }

  if (listen (fd, 4) == SOCKET_ERROR)
    {
      socket_error ("Init socket: listen");
      closesocket (fd);
      return INVALID_SOCKET;
    }

  return fd;
}

void
show_greeting (Descriptor * dnew)
{
  int num;

  num = number_range (0, MAX_GREETING - 1);

  if (help_greeting[num])
    {
      if (help_greeting[num][0] == '.')
	d_print (dnew, help_greeting[num] + 1);
      else
	d_print (dnew, help_greeting[num]);
    }
  else
    {
      bugf ("NULL Greeting! (%d)", num);
      d_printlnf (dnew,
		  "\t{cWelcome to..." NEWLINE NEWLINE "\t\t{W%s" NEWLINE
		  NEWLINE "\t{cMay Your Stay Be.... {CMercenary{c!{x",
		  mud_info.name);
    }

  d_println (dnew,
	     "{c       Original DikuMUD by Hans Staerfeldt, Katja Nyboe,"
	     NEWLINE
	     "       Tom Madsen, Michael Seifert, and Sebastian Hammer"
	     NEWLINE
	     "       Based on MERC 2.1 code by Hatchet, Furey, and Kahn"
	     NEWLINE
	     "       ROM 2.4 copyright (c) 1993-1998 Russ Taylor. "
	     NEWLINE
	     "       1stMud Server copyright (c) 2001-2004, Markanth.{x"
	     NEWLINE);

  d_printf (dnew,
	    "{c%s (type 'list' to use a name generator){x",
	    mud_info.login_prompt);
}

void
game_loop (SOCKET ctrl)
{
  struct timeval last_time;

  gettimeofday (&last_time, NULL);
  current_time = (time_t) last_time.tv_sec;

  run_level = RUNLEVEL_MAIN_LOOP;


  while (run_level == RUNLEVEL_MAIN_LOOP)
    {
      fd_set in_set;
      fd_set out_set;
      fd_set exc_set;
      Descriptor *d;
      SOCKET maxdesc;
      static struct timeval null_time;

#ifdef HAVE_SETITIMER

      int vt_set = 0;
#endif

      struct timeval now_time;
      long secDelta;
      long usecDelta;

      gettimeofday (&now_time, NULL);

      usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
	+ 1000000 / PULSE_PER_SECOND;
      secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
      while (usecDelta < 0)
	{
	  usecDelta += 1000000;
	  secDelta -= 1;
	}

      while (usecDelta >= 1000000)
	{
	  usecDelta -= 1000000;
	  secDelta += 1;
	}

      if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
	{
	  struct timeval stall_time;

	  stall_time.tv_usec = usecDelta;
	  stall_time.tv_sec = secDelta;
#ifdef WIN32

	  Sleep ((stall_time.tv_sec * 1000) | (stall_time.tv_usec / 1000));
#else

	  if (select (0, NULL, NULL, NULL, &stall_time) == SOCKET_ERROR)
	    {
	      socket_error ("Game_loop: select: stall");
	      exit (1);
	    }
#endif

	}

      gettimeofday (&last_time, NULL);
      current_time = (time_t) last_time.tv_sec;


      FD_ZERO (&in_set);
      FD_ZERO (&out_set);
      FD_ZERO (&exc_set);
      FD_SET (ctrl, &in_set);
      maxdesc = ctrl;
      for (d = descriptor_first; d; d = d->next)
	{
	  maxdesc = Max (maxdesc, d->descriptor);
	  FD_SET (d->descriptor, &in_set);
	  FD_SET (d->descriptor, &out_set);
	  FD_SET (d->descriptor, &exc_set);
	}

      if (select (maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) ==
	  SOCKET_ERROR)
	{
	  if (!check_errno (EINTR))
	    {
	      socket_error ("Game_loop: select: poll");
	      exit (1);
	    }
	}


      if (FD_ISSET (ctrl, &in_set))
	init_descriptor (ctrl);


      for (d = descriptor_first; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  if (FD_ISSET (d->descriptor, &exc_set))
	    {
	      FD_CLR (d->descriptor, &in_set);
	      FD_CLR (d->descriptor, &out_set);
	      if (d->character && d->connected == CON_PLAYING)
		save_char_obj (d->character);
	      d->outtop = 0;
	      socket_error ("game_loop: freaky folk found.");
	      close_socket (d);
	    }
	  if (d->connected < CON_PLAYING)
	    {
	      if ((current_time - d->connect_time) > 15 * MINUTE)
		{
		  d->outtop = 0;
		  close_socket (d);
		}
	    }
	}


      for (d = descriptor_first; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  d->fcommand = false;

	  if (FD_ISSET (d->descriptor, &in_set))
	    {
	      if (d->character != NULL)
		d->character->timer = 0;
	      if (!read_from_descriptor (d))
		{
		  FD_CLR (d->descriptor, &out_set);
		  if (d->character != NULL && d->connected == CON_PLAYING)
		    save_char_obj (d->character);
		  d->outtop = 0;
		  close_socket (d);
		  socket_error ("game_loop: unable to read_from_descriptor");
		  continue;
		}
	    }

	  if (d->character != NULL && d->character->daze > 0)
	    --d->character->daze;

	  if (d->character != NULL && d->character->wait > 0)
	    {
	      --d->character->wait;
	      continue;
	    }

	  read_from_buffer (d);
	  if (!NullStr (d->incomm))
	    {
	      d->fcommand = true;
	      stop_idling (d->character);

#ifdef HAVE_SETITIMER

	      vt_set = 0;
	      set_vtimer (-1);
#endif


	      if (d->showstr_point)
		show_string (d, d->incomm);
	      else if (d->pString)
		string_add (d->character, d->incomm);
	      else
		switch (d->connected)
		  {
		  case CON_PLAYING:
		    substitute_alias (d, d->incomm);
		    break;
		  default:
		    nanny (d, d->incomm);
		    break;
		  }

	      d->incomm[0] = '\0';
	    }
	}

#ifndef DISABLE_I3
      I3_loop ();
#endif


      update_handler ();

#ifndef DISABLE_WEBSRV

      update_web_server ();
#endif


      for (d = descriptor_first; d != NULL; d = d_next)
	{
	  d_next = d->next;

	  if ((d->fcommand || d->outtop > 0)
	      && FD_ISSET (d->descriptor, &out_set))
	    {
	      if (!process_output (d, true))
		{
		  if (d->character != NULL && d->connected == CON_PLAYING)
		    save_char_obj (d->character);
		  d->outtop = 0;
		  close_socket (d);
		  socket_error ("game_loop: unable to process_output");
		}
	    }
	}

#ifdef HAVE_SETITIMER
      if (++vt_set >= 35)
	{
	  vt_set = 0;

	  set_vtimer (-1);
	}
#endif

    }

  return;
}


#ifdef WIN32
const char *
get_winsock_error_text (int errorcode)
{
  static char result[MSL];


#define WEM_CASE(m) case m: pszMsg = #m ; break

  const char *pszMsg;
  int iError = 0;

  if (errorcode != 0)
    {
      iError = errorcode;
    }
  else
    {
      iError = WSAGetLastError ();
    }
  switch (iError)
    {
      WEM_CASE (WSABASEERR);
      WEM_CASE (WSAEINTR);
      WEM_CASE (WSAEBADF);
      WEM_CASE (WSAEACCES);
      WEM_CASE (WSAEFAULT);
      WEM_CASE (WSAEINVAL);
      WEM_CASE (WSAEMFILE);
      WEM_CASE (WSAEWOULDBLOCK);
      WEM_CASE (WSAEINPROGRESS);
      WEM_CASE (WSAEALREADY);
      WEM_CASE (WSAENOTSOCK);
      WEM_CASE (WSAEDESTADDRREQ);
      WEM_CASE (WSAEMSGSIZE);
      WEM_CASE (WSAEPROTOTYPE);
      WEM_CASE (WSAENOPROTOOPT);
      WEM_CASE (WSAEPROTONOSUPPORT);
      WEM_CASE (WSAESOCKTNOSUPPORT);
      WEM_CASE (WSAEOPNOTSUPP);
      WEM_CASE (WSAEPFNOSUPPORT);
      WEM_CASE (WSAEAFNOSUPPORT);
      WEM_CASE (WSAEADDRINUSE);
      WEM_CASE (WSAEADDRNOTAVAIL);
      WEM_CASE (WSAENETDOWN);
      WEM_CASE (WSAENETUNREACH);
      WEM_CASE (WSAENETRESET);
      WEM_CASE (WSAECONNABORTED);
      WEM_CASE (WSAECONNRESET);
      WEM_CASE (WSAENOBUFS);
      WEM_CASE (WSAEISCONN);
      WEM_CASE (WSAENOTCONN);
      WEM_CASE (WSAESHUTDOWN);
      WEM_CASE (WSAETOOMANYREFS);
      WEM_CASE (WSAETIMEDOUT);
      WEM_CASE (WSAECONNREFUSED);
      WEM_CASE (WSAELOOP);
      WEM_CASE (WSAENAMETOOLONG);
      WEM_CASE (WSAEHOSTDOWN);
      WEM_CASE (WSAEHOSTUNREACH);
      WEM_CASE (WSAENOTEMPTY);
      WEM_CASE (WSAEPROCLIM);
      WEM_CASE (WSAEUSERS);
      WEM_CASE (WSAEDQUOT);
      WEM_CASE (WSAESTALE);
      WEM_CASE (WSAEREMOTE);
      WEM_CASE (WSAEDISCON);
      WEM_CASE (WSASYSNOTREADY);
      WEM_CASE (WSAVERNOTSUPPORTED);
      WEM_CASE (WSANOTINITIALISED);
      WEM_CASE (WSAHOST_NOT_FOUND);
      WEM_CASE (WSATRY_AGAIN);
      WEM_CASE (WSANO_RECOVERY);
      WEM_CASE (WSANO_DATA);
    default:
      pszMsg = NULL;
    }


  if (pszMsg)
    {
      sprintf (result, "%s(%d)", pszMsg, iError);
    }
  else
    {
      sprintf (result, "unknown winsock error value %d.", iError);
    }

  return result;
};
#endif

bool
socket_cntl (SOCKET desc)
{
#ifdef WIN32

  unsigned long blockmode = 1;

  if (ioctlsocket (desc, FIONBIO, &blockmode) == SOCKET_ERROR)
    {
      socket_error ("error setting new socket to nonblocking");
      return false;
    }
#else
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

  if (fcntl (desc, F_SETFL, FNDELAY) == SOCKET_ERROR)
    {
      socket_error ("fcntl() FNDELAY");
      return false;
    }
#endif
  return true;
}

void
init_descriptor (int ctrl)
{
  Descriptor *dnew;
  struct sockaddr_in sock;
  struct hostent *from;
  SOCKET desc;
  socklen_t size;

  size = sizeof (sock);

  if (getsockname (ctrl, (SOCKADDR *) & sock, &size) == SOCKET_ERROR)
    {
      socket_error ("init_descriptor: getsockname");
    }

  if ((desc = accept (ctrl, (SOCKADDR *) & sock, &size)) == INVALID_SOCKET)
    {
      socket_error ("New_descriptor: accept");
      return;
    }

  if (!socket_cntl (desc))
    {
      socket_error ("New_descriptor: cannot set desc to nonblocking");
      return;
    }


  dnew = new_descriptor ();
  dnew->descriptor = desc;
  dnew->connect_time = getcurrenttime ();

  size = sizeof (sock);
  if (getpeername (desc, (SOCKADDR *) & sock, &size) == SOCKET_ERROR)
    {
      socket_error ("New_descriptor: getpeername");
      dnew->host = str_dup ("(unknown)");
    }
  else
    {

      dnew->port = ntohs (sock.sin_port);
      replace_str (&dnew->host, inet_ntoa (sock.sin_addr));
      dnew->ip = ntohl (sock.sin_addr.s_addr);

      logf ("Sock.sinaddr:  %s", dnew->host);

      if (!IsSet (mud_info.mud_flags, NO_DNS_LOOKUPS))
	{
	  if ((from =
	       gethostbyaddr ((char *) &sock.sin_addr,
			      sizeof (sock.sin_addr), AF_INET)) != NULL)
	    replace_str (&dnew->host, from->h_name);
	}
    }


  if (check_ban (dnew->host, BAN_ALL))
    {
      d_write (dnew, "Your site has been banned from this mud." NEWLINE, 0);
      closesocket (desc);
      free_descriptor (dnew);
      return;
    }

  Link (dnew, descriptor, next, prev);


  init_telnet (dnew);
  d_println (dnew, "Autodetecting IMP...v1.30");
  d_println (dnew, "This world is Pueblo 1.10 enhanced.");
  d_println (dnew,
	     "Welcome, would you like ANSI color? (Y)es, (N)o, (T)est:");
  mud_info.stats.connections++;
  mud_info.stats.boot_connects++;
  return;
}

void
close_socket (Descriptor * dclose)
{
  CharData *ch;

  if (dclose->outtop > 0)
    process_output (dclose, false);

  if (dclose->snoop_by != NULL)
    {
      d_println (dclose->snoop_by, "Your victim has left the game.");
    }

  {
    Descriptor *d;

    for (d = descriptor_first; d != NULL; d = d->next)
      {
	if (d->snoop_by == dclose)
	  d->snoop_by = NULL;
      }
  }

  if ((ch = CH (dclose)) != NULL)
    {
      logf ("Closing link to %s.", ch->name);



      if ((dclose->connected == CON_PLAYING
	   && run_level != RUNLEVEL_SHUTDOWN)
	  || ((dclose->connected >= CON_NOTE_TO)
	      && (dclose->connected <= CON_NOTE_FINISH)))
	{
	  extract_quest (ch);
	  extract_war (ch);
	  extract_arena (ch);
	  act ("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
	  wiznet ("Net death has claimed $N.", ch, NULL, WIZ_LINKS, 0, 0);
	  ch->desc = NULL;
	}
      else
	{
	  free_char (ch);
	}
    }

  free_runbuf (dclose);

  if (d_next == dclose)
    d_next = d_next->next;

  UnLink (dclose, descriptor, next, prev);

#ifndef DISABLE_MCCP

  if (dclose->out_compress)
    {
      deflateEnd (dclose->out_compress);
      free_mem (dclose->out_compress_buf);
      free_mem (dclose->out_compress);
    }
#endif

  closesocket (dclose->descriptor);
  free_descriptor (dclose);
  return;
}

bool
read_from_descriptor (Descriptor * d)
{
  unsigned int iStart, index;
  static unsigned char buf[sizeof (d->inbuf)];

  memset (buf, 0, sizeof (buf));


  if (!NullStr (d->incomm))
    return true;


  iStart = strlen (d->inbuf);
  index = 0;

  if (iStart >= sizeof (d->inbuf) - 10)
    {
      logf ("%s input overflow!", d->host);
      d_write (d, NEWLINE "*** PUT A LID ON IT!!! ***" NEWLINE, 0);
      d->inbuf[sizeof (d->inbuf) - 10] = '\0';
      return true;
    }

  for (;;)
    {
      int nRead;


      if (sizeof (buf) - 10 - iStart == 0)
	{
	  break;
	}

      nRead =
	recv (d->descriptor, (char *) buf + index,
	      sizeof (buf) - 10 - index, 0);
      if (nRead > 0)
	{
	  index += nRead;
	  if (buf[index - 1] == '\n' || buf[index - 1] == '\r')
	    break;
	}
      else if (nRead == 0)
	{
	  logf ("EOF encountered on read.");
	  return false;
	}
      else if (check_errno (EWOULDBLOCK))
	break;
      else
	{
	  socket_error ("Read_from_descriptor");
	  return false;
	}
    }


  process_telnet (d, index, buf);


  if (d->connected == CON_GET_TERM &&
      (IsSet
       (d->desc_flags,
	DESC_TELOPT_EOR | DESC_TELOPT_ECHO | DESC_TELOPT_NAWS | DESC_MXP
	| DESC_MSP | DESC_PUEBLO | DESC_TELOPT_TTYPE | DESC_TELOPT_BINARY
	| DESC_PORTAL | DESC_IMP)
#ifndef DISABLE_MCCP
       || d->out_compress
#endif
      ))
    {
      SetBit (d->desc_flags, DESC_COLOR);
      d_printlnf (d, "%s{c enabled Automatically...{x",
		  Upper (colorize ("color")));
      show_greeting (d);
      d->connected = CON_GET_NAME;
    }
  return true;
}


void
read_from_buffer (Descriptor * d)
{
  int i, j, k;


  if (!NullStr (d->incomm))
    return;

  if (d->character && d->character->position == POS_FIGHTING && d->run_buf)
    free_runbuf (d);

  if (d->run_buf)
    {
      while (isdigit (*d->run_head) && *d->run_head != '\0')
	{
	  char *s;
	  char *e;

	  s = (char *) d->run_head;
	  while (isdigit (*s))
	    s++;
	  e = s;
	  while (*(--s) == '0' && s != d->run_head)
	    ;
	  if (isdigit (*s) && *s != '0' && *e != 'o')
	    {
	      d->incomm[0] = *e;
	      d->incomm[1] = '\0';
	      s[0]--;
	      while (isdigit (*(++s)))
		*s = '9';
	      return;
	    }
	  if (*e == 'o')
	    d->run_head = e;
	  else
	    d->run_head = ++e;
	}
      if (*d->run_head != '\0')
	{
	  if (*d->run_head != 'o')
	    {
	      d->incomm[0] = *d->run_head++;
	      d->incomm[1] = '\0';
	      return;
	    }
	  else
	    {
	      char buf[MAX_INPUT_LENGTH];

	      d->run_head++;

	      strcpy (buf, "open ");
	      switch (*d->run_head)
		{
		case 'n':
		  strcat (buf, "north");
		  break;
		case 's':
		  strcat (buf, "south");
		  break;
		case 'e':
		  strcat (buf, "east");
		  break;
		case 'w':
		  strcat (buf, "west");
		  break;
		case 'u':
		  strcat (buf, "up");
		  break;
		case 'd':
		  strcat (buf, "down");
		  break;
		default:
		  return;
		}

	      strcpy (d->incomm, buf);
	      d->run_head++;
	      return;
	    }
	}
      free_runbuf (d);
    }


  for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
      if (d->inbuf[i] == '\0')
	return;
    }


  for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
    {
      if (k >= MAX_INPUT_LENGTH - 32)
	{
	  d_write (d, "Line too long." NEWLINE, 0);


	  for (; d->inbuf[i] != '\0'; i++)
	    {
	      if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		break;
	    }
	  d->inbuf[i] = '\n';
	  d->inbuf[i + 1] = '\0';
	  break;
	}

      if (d->inbuf[i] == '\b' && k > 0)
	--k;
      else if (isascii (d->inbuf[i]) && isprint (d->inbuf[i]))
	d->incomm[k++] = d->inbuf[i];
    }


  if (k == 0)
    d->incomm[k++] = ' ';
  d->incomm[k] = '\0';



  if (k > 1 || d->incomm[0] == '!')
    {
      if (d->incomm[0] != '!' && str_cmp (d->incomm, d->inlast))
	{
	  d->repeat = 0;
	}
      else
	{
	  if (++d->repeat >= 25 && d->character &&
	      d->connected == CON_PLAYING)
	    {
	      logf ("%s input spamming!", d->host);
	      new_wiznet
		(d->character, NULL, WIZ_SPAM, false,
		 get_trust (d->character),
		 "Spam spam spam $N spam spam spam spam spam! (%s)",
		 d->incomm[0] == '!' ? d->inlast : d->incomm);
	      d->repeat = 0;

	    }
	}
    }


  if (d->incomm[0] == '!')
    strcpy (d->incomm, d->inlast);
  else
    strcpy (d->inlast, d->incomm);


  while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
    i++;
  for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
    ;
  return;
}

#define CAN_SHOW_PROMPT    	    	((!IsNPC(ch) && IsSet(ch->act, PLR_AUTOPROMPT)) \
    	    	    	    	    	    	    	|| (d->fcommand && !d->run_buf) || ch->fighting)

#include "comm_descriptor.h"

#include "comm_prompt.h"


bool
process_output (Descriptor * d, bool fPrompt)
{

  if (run_level != RUNLEVEL_SHUTDOWN)
    {
      CharData *ch = d->character;

      d->fPrompt = false;

      if (d->showstr_point)
	{
	  const char *ptr;
	  size_t shown_lines = 0, total_lines = 0;

	  for (ptr = d->showstr_head; ptr != d->showstr_point; ptr++)
	    if (*ptr == '\n')
	      shown_lines++;

	  total_lines = shown_lines + line_count (d->showstr_point);

	  d_printf (d, NEWLINE "(%d%%) Please type " MXPTAG ("Pager")
		    "(H)elp, (R)efresh, (B)ack, or (C)ontinue or hit ENTER"
		    MXPTAG ("/Pager") ".", Percent (shown_lines,
						    total_lines));
	  d->fPrompt = true;
	}
      else if (fPrompt)
	{
	  switch (d->connected)
	    {
	    case CON_NOTE_TEXT:
	      d_print (d, "> ");
	      d->fPrompt = true;
	      break;
	    case CON_PLAYING:
	      if (d->pString)
		{
		  d_print (d, "> ");
		  d->fPrompt = true;
		}
	      else if (ch)
		{
		  CharData *victim;


		  if ((victim = ch->fighting) != NULL && can_see (ch, victim))
		    {
		      int percent;
		      char wound[100];

		      if (victim->max_hit > 0)
			percent = victim->hit * 100 / victim->max_hit;
		      else
			percent = -1;

		      if (percent >= 100)
			sprintf (wound, "is in excellent condition.");
		      else if (percent >= 90)
			sprintf (wound, "has a few scratches.");
		      else if (percent >= 75)
			sprintf (wound, "has some small wounds and bruises.");
		      else if (percent >= 50)
			sprintf (wound, "has quite a few wounds.");
		      else if (percent >= 30)
			sprintf (wound,
				 "has some big nasty wounds and scratches.");
		      else if (percent >= 15)
			sprintf (wound, "looks pretty hurt.");
		      else if (percent >= 0)
			sprintf (wound, "is in awful condition.");
		      else
			sprintf (wound, "is bleeding to death.");

		      d_printlnf (d, "%s %s", Upper (GetName (victim)),
				  wound);
		    }

		  ch = CH (d);

		  if (!IsSet (ch->comm, COMM_COMPACT)
		      && (CAN_SHOW_PROMPT || d->editor != ED_NONE))
		    {
		      d_println (d, NULL);
		    }

		  if (IsSet (ch->comm, COMM_GPROMPT) && CAN_SHOW_PROMPT)
		    {
		      bust_a_group_prompt (d->character);
		      d->fPrompt = true;
		    }

		  if (IsSet (ch->comm, COMM_PROMPT))
		    {
		      if (CAN_SHOW_PROMPT)
			{
			  bust_a_prompt (d->character);
			  d->fPrompt = true;
			}
		      else if (d->editor != ED_NONE)
			{
			  d_printf (d, "{cOLC %s : {W%s{x",
				    olc_ed_name (d), olc_ed_vnum (d));
			  d->fPrompt = true;
			}
		    }
		  if (IsSet (ch->comm, COMM_TELNET_GA))
		    {
		      d_write (d, go_ahead_str, 0);
		    }
		  else if (IsSet (ch->comm, COMM_TELNET_EOR))
		    {
		      const char eor_str[] = {
			IAC, EOR, NUL
		      };

		      d_write (d, eor_str, 0);
		      d->fPrompt = false;
		    }
		}
	      break;
	    default:
	      d->fPrompt = true;
	      break;
	    }
	}
    }


  if (d->outtop == 0)
    return true;


  if (d->snoop_by != NULL)
    {
      if (d->character != NULL)
	d_print (d->snoop_by, d->character->name);
      d_print (d->snoop_by, "> ");
      print_stripped_client_code (d->snoop_by, d->outbuf, d->outtop);
    }


  return convert_color_mxp_tags (d);
}


bool
check_parse_name (const char *name)
{
  ClanData *clan;


  if (is_exact_name
      (name, "all auto immortal self someone something the you loner none"))
    {
      return false;
    }


  for (clan = clan_first; clan; clan = clan->next)
    {
      if (tolower (name[0]) == tolower (clan->name[0]) &&
	  !str_cmp (name, clan->name))
	return false;
    }

  if (str_cmp (capitalize (name), "Alander") &&
      (!str_prefix ("Alan", name) || !str_suffix ("Alander", name)))
    return false;





  if (strlen (name) < 2)
    return false;

  if (strlen (name) > 12)
    return false;


  {
    const char *pc;
    bool fIll, adjcaps = false, cleancaps = false;
    unsigned int total_caps = 0;

    fIll = true;
    for (pc = name; *pc != '\0'; pc++)
      {
	if (!isalpha (*pc))
	  return false;

	if (isupper (*pc))
	  {
	    if (adjcaps)
	      cleancaps = true;
	    total_caps++;
	    adjcaps = true;
	  }
	else
	  adjcaps = false;

	if (tolower (*pc) != 'i' && tolower (*pc) != 'l')
	  fIll = false;
      }

    if (fIll)
      return false;

    if (cleancaps || (total_caps > (strlen (name)) / 2 && strlen (name) < 3))
      return false;
  }


  {
    CharIndex *pMobIndex;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
      {
	for (pMobIndex = char_index_hash[iHash]; pMobIndex != NULL;
	     pMobIndex = pMobIndex->next)
	  {
	    if (is_name (name, pMobIndex->player_name))
	      return false;
	  }
      }
  }

  return true;
}


bool
check_reconnect (Descriptor * d, const char *name, bool fConn)
{
  CharData *ch;

  for (ch = char_first; ch != NULL; ch = ch->next)
    {
      if (!IsNPC (ch) && (!fConn || ch->desc == NULL) &&
	  !str_cmp (d->character->name, ch->name))
	{
	  if (fConn == false)
	    {
	      replace_str (&d->character->pcdata->pwd, ch->pcdata->pwd);
	    }
	  else
	    {
	      free_char (d->character);

	      d->character = ch;
	      ch->desc = d;
	      ch->timer = 0;
	      chprintlnf (ch,
			  "{cReconnecting%s{x" NEWLINE
			  "Welcome back %s.", colorize ("....."), ch->name);
	      if (!IsNPC (ch) && ch->pcdata->tells)
		{
		  chprintlnf (ch,
			      "You have %d tells waiting.  Type '%s' to view them.",
			      ch->pcdata->tells, cmd_name (do_replay));
		}
	      act ("$n has reconnected.", ch, NULL, NULL, TO_ROOM);

	      logf ("%s@%s reconnected.", ch->name, d->host);
	      wiznet ("$N groks the fullness of $S link.", ch,
		      NULL, WIZ_LINKS, false, 0);
	      d->connected = CON_PLAYING;
	      do_function (ch, &do_ncheck, "");
	    }
	  return true;
	}
    }

  return false;
}


bool
check_playing (Descriptor * d, const char *name)
{
  Descriptor *dold;

  for (dold = descriptor_first; dold; dold = dold->next)
    {
      if (dold != d && dold->character != NULL &&
	  dold->connected != CON_GET_NAME &&
	  dold->connected != CON_GET_OLD_PASSWORD &&
	  !str_cmp (name, CH (dold)->name))
	{
	  d_println (d, "{cThat character is already playing.{x");
	  d_print (d, "{cDo you wish to connect anyway ({WY{c/{WN{c?{x");
	  d->connected = CON_BREAK_CONNECT;
	  return true;
	}
    }

  return false;
}

void
stop_idling (CharData * ch)
{
  if (ch == NULL || ch->desc == NULL
      || ch->desc->connected != CON_PLAYING || ch->was_in_room == NULL
      || ch->in_room != get_room_index (ROOM_VNUM_LIMBO))
    return;

  ch->timer = 0;
  char_from_room (ch);
  char_to_room (ch, ch->was_in_room);
  ch->was_in_room = NULL;
  act ("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
  return;
}


int
chprint (CharData * ch, const char *txt)
{
  if (!NullStr (txt) && ch && ch->desc != NULL)
    return d_print (ch->desc, txt);

  return 0;
}

int
chprintln (CharData * ch, const char *txt)
{
  if (ch && ch->desc != NULL)
    {
      return (d_print (ch->desc, txt) + d_print (ch->desc, NEWLINE));
    }
  return 0;
}


void
sendpage (CharData * ch, const char *txt)
{
  Descriptor *d;

  if (NullStr (txt) || (d = ch->desc) == NULL)
    return;

  if (ch->lines <= 0)
    {
      d_print (d, txt);
      return;
    }


  if (!NullStr (d->showstr_head))
    {
      char *fub;
      size_t i, size_new = strlen (txt) + strlen (d->showstr_head) + 2;

      alloc_mem (fub, char, size_new);

      fub[0] = '\0';
      strncat (fub, d->showstr_head, size_new);
      i = strlen (fub) - strlen (d->showstr_point);
      strncat (fub, txt, size_new);

      replace_str (&d->showstr_head, fub);
      d->showstr_point = d->showstr_head + i;
      free_mem (fub);
      return;
    }

  replace_str (&d->showstr_head, txt);
  d->showstr_point = d->showstr_head;
  show_string (d, "");
  return;
}


void
show_string (Descriptor * d, char *input)
{
  char buffer[MAX_STRING_LENGTH * 3];
  char buf[MAX_INPUT_LENGTH];
  register const char *scan;
  register char *scan2;
  register const char *chk;
  int lines = 0;
  int maxlines = get_scr_lines (d->character);
  int toggle = 1;

  one_argument (input, buf);

  switch (toupper (buf[0]))
    {
    case '\0':
    case 'C':
      lines = 0;
      break;

    case 'R':
      lines = -1 - maxlines;
      break;

    case 'B':
      lines = -(2 * maxlines);
      break;

    case '?':
    case 'H':
      d_println (d,
		 "Pager help:" NEWLINE "C or Enter     next page" NEWLINE
		 "R              refresh this page");
      d_println (d,
		 "B              previous page" NEWLINE
		 "H or ?         help" NEWLINE "Any other keys exit.");
      return;

    default:
      if (d->showstr_head)
	{
	  replace_str (&d->showstr_head, "");
	}
      d->showstr_point = NULL;
      return;

    }


  if (lines < 0)
    {
      for (scan = d->showstr_point; scan > d->showstr_head; scan--)
	if ((*scan == '\n') || (*scan == '\r'))
	  {
	    toggle = -toggle;
	    if (toggle < 0)
	      if (!(++lines))
		break;
	  }
      d->showstr_point = scan;
    }


  lines = 0;
  toggle = 1;
  for (scan2 = buffer;; scan2++, d->showstr_point++)
    if (((*scan2 = *d->showstr_point) == '\n' || *scan2 == '\r')
	&& (toggle = -toggle) < 0)
      lines++;
    else if (!*scan2 || (d->character && !IsNPC (d->character)
			 && lines >= maxlines))
      {

	*scan2 = '\0';
	d_print (d, buffer);


	for (chk = d->showstr_point; isspace (*chk); chk++)
	  ;
	if (!*chk)
	  {
	    if (d->showstr_head)
	      {
		replace_str (&d->showstr_head, "");
	      }
	    d->showstr_point = 0;
	  }
	return;
      }

  return;
}

char *
fname (const char *namelist)
{
  static char holder[256];
  char *point;

  for (point = holder; isalpha (*namelist); namelist++, point++)
    *point = *namelist;

  *point = '\0';

  return (holder);
}

void
perform_act (const char *orig, CharData * ch, const void *arg1,
	     const void *arg2, flag_t type, CharData * to)
{
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  char buf[MSL * 5];
  register const char *str, *i = NULL;
  register char *point;
  size_t z;

  for (str = orig, point = buf; *str != '\0'; str++)
    {
      if (*str != '$')
	{
	  *point++ = *str;
	  continue;
	}

      str++;
      i = "<@@@>";
      if (!arg2 && isupper (*str))
	{
	  logf ("perform_act:missing arg2 for code %c.", *str);
	  i = " <@@@> ";
	}
      else
	{
	  switch (*str)
	    {
	    default:
	      logf ("perform_act:bad code %c.", *str);
	      i = " <@@@> ";
	      break;

	    case '$':
	      i = "$";
	      break;
	    case 't':
	      if (arg1)
		{
		  if (IsSet (type, TO_DAMAGE)
		      && (IsNPC (to) || !IsSet (to->act, PLR_AUTODAMAGE)))
		    i = "";
		  else
		    i = (const char *) arg1;
		}
	      else
		log_string ("perform_act:bad code $t for 'arg1'");
	      break;
	    case 'T':
	      if (arg2)
		i = (const char *) arg2;
	      else
		log_string ("perform_act:bad code $T for 'arg2'");
	      break;
	    case 'n':
	      if (ch && to)
		i = Pers (ch, to);
	      else
		log_string ("perform_act:bad code $n for 'ch' or 'to'");
	      break;
	    case 'N':
	      if (vch && to)
		i = Pers (vch, to);
	      else
		log_string ("perform_act:bad code $N for 'ch' or 'to'");
	      break;
	    case 'e':
	      if (ch)
		i = he_she[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $e for 'ch'");
	      break;
	    case 'E':
	      if (vch)
		i = he_she[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $E for 'vch'");
	      break;
	    case 'm':
	      if (ch)
		i = him_her[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $m for 'ch'");
	      break;
	    case 'M':
	      if (vch)
		i = him_her[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $M for 'vch'");
	      break;
	    case 's':
	      if (ch)
		i = his_her[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $s for 'ch'");
	      break;
	    case 'S':
	      if (vch)
		i = his_her[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $S for 'vch'");
	      break;
	    case 'g':
	      if (ch && ch->deity != NULL)
		i = ch->deity->name;
	      else
		log_string ("perform_act:bad code $g for 'ch'");
	      break;
	    case 'G':
	      if (vch && vch->deity != NULL)
		i = vch->deity->name;
	      else
		log_string ("perform_act:bad code $G for 'vch'");
	      break;
	    case 'c':
	      if (ch && is_clan (ch))
		i = CharClan (ch)->name;
	      else
		log_string ("perform_act:bad code $c for 'ch'");
	      break;
	    case 'C':
	      if (vch && is_clan (vch))
		i = CharClan (vch)->name;
	      else
		log_string ("perform_act:bad code $C for 'vch'");
	      break;
	    case 'o':
	      if (to && obj1)
		i = can_see_obj (to, obj1) ? fname (obj1->name) : "something";
	      else
		log_string ("perform_act:bad code $o for 'to' and 'obj1'");
	      break;

	    case 'O':
	      if (to && obj2)
		i = can_see_obj (to, obj2) ? fname (obj2->name) : "something";
	      else
		log_string ("perform_act:bad code $O for 'to' and 'obj2'");
	      break;

	    case 'p':
	      if (to && obj1)
		i = can_see_obj (to, obj1) ? obj1->short_descr : "something";
	      else
		log_string ("perform_act:bad code $p for 'to' and 'obj1'");
	      break;

	    case 'P':
	      if (to && obj2)
		i = can_see_obj (to, obj2) ? obj2->short_descr : "something";
	      else
		log_string ("perform_act:bad code $P for 'to' and 'obj2'");
	      break;

	    case 'd':
	      if (arg2 == NULL || ((const char *) arg2)[0] == '\0')
		{
		  i = "door";
		}
	      else
		{
		  char name[MIL];

		  one_argument ((const char *) arg2, name);
		  i = name;
		}
	      break;
	    }
	}

      while ((*point = *i) != '\0')
	point++, i++;
    }

  *point++ = '{';
  *point++ = 'x';
  *point++ = '\n';
  *point++ = '\r';
  *point = '\0';

  z = skipcol (buf);
  buf[z] = toupper (buf[z]);

  if (to->desc)
    {
      if (to->desc->connected == CON_PLAYING)
	d_print (to->desc, buf);
    }
  else if (IsNPC (to) && MOBtrigger && HasTriggerMob (to, TRIG_ACT))
    p_act_trigger (buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT);

  if (ch && ch->in_room && IsSet (type, TO_ROOM | TO_NOTVICT))
    {
      ObjData *obj, *obj_next;
      CharData *tch, *tch_next;

      for (obj = ch->in_room->content_first; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (HasTriggerObj (obj, TRIG_ACT))
	    p_act_trigger (orig, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT);
	}

      for (tch = ch; tch; tch = tch_next)
	{
	  tch_next = tch->next_in_room;

	  for (obj = tch->carrying_first; obj; obj = obj_next)
	    {
	      obj_next = obj->next_content;
	      if (HasTriggerObj (obj, TRIG_ACT))
		p_act_trigger (orig, NULL, obj, NULL, ch, NULL, NULL,
			       TRIG_ACT);
	    }
	}

      if (HasTriggerRoom (ch->in_room, TRIG_ACT))
	p_act_trigger (orig, NULL, NULL, ch->in_room, ch, NULL, NULL,
		       TRIG_ACT);
    }

  return;
}

const char *
perform_act_string (const char *orig, CharData * ch,
		    const void *arg1, const void *arg2, bool cReturn)
{
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  char buf[MSL * 5];
  register const char *str, *i = NULL;
  register char *point;

  for (str = orig, point = buf; *str != '\0'; str++)
    {
      if (*str != '$')
	{
	  *point++ = *str;
	  continue;
	}

      str++;
      i = "<@@@>";
      if (!arg2 && isupper (*str))
	{
	  logf ("perform_act:missing arg2 for code %c.", *str);
	  i = " <@@@> ";
	}
      else
	{
	  switch (*str)
	    {
	    default:
	      logf ("perform_act:bad code %c.", *str);
	      i = " <@@@> ";
	      break;

	    case '$':
	      i = "$";
	      break;
	    case 't':
	      if (arg1)
		{
		  i = (const char *) arg1;
		}
	      else
		log_string ("perform_act:bad code $t for 'arg1'");
	      break;
	    case 'T':
	      if (arg2)
		i = (const char *) arg2;
	      else
		log_string ("perform_act:bad code $T for 'arg2'");
	      break;
	    case 'n':
	      if (ch)
		i = IsNPC (ch) ? ch->short_descr : ch->name;
	      else
		log_string ("perform_act:bad code $n for 'ch'");
	      break;
	    case 'N':
	      if (vch)
		i = IsNPC (vch) ? vch->short_descr : vch->name;
	      else
		log_string ("perform_act:bad code $N for 'vch'");
	      break;
	    case 'e':
	      if (ch)
		i = he_she[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $e for 'ch'");
	      break;
	    case 'E':
	      if (vch)
		i = he_she[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $E for 'vch'");
	      break;
	    case 'm':
	      if (ch)
		i = him_her[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $m for 'ch'");
	      break;
	    case 'M':
	      if (vch)
		i = him_her[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $M for 'vch'");
	      break;
	    case 's':
	      if (ch)
		i = his_her[Range (0, ch->sex, 2)];
	      else
		log_string ("perform_act:bad code $s for 'ch'");
	      break;
	    case 'S':
	      if (vch)
		i = his_her[Range (0, vch->sex, 2)];
	      else
		log_string ("perform_act:bad code $S for 'vch'");
	      break;
	    case 'g':
	      if (ch && ch->deity != NULL)
		i = ch->deity->name;
	      else
		log_string ("perform_act:bad code $g for 'ch'");
	      break;
	    case 'G':
	      if (vch && vch->deity != NULL)
		i = vch->deity->name;
	      else
		log_string ("perform_act:bad code $G for 'vch'");
	      break;
	    case 'c':
	      if (ch && is_clan (ch))
		i = CharClan (ch)->name;
	      else
		log_string ("perform_act:bad code $c for 'ch'");
	      break;
	    case 'C':
	      if (vch && is_clan (vch))
		i = CharClan (vch)->name;
	      else
		log_string ("perform_act:bad code $C for 'vch'");
	      break;
	    case 'o':
	      if (obj1)
		i = fname (obj1->name);
	      else
		log_string ("perform_act:bad code $o for 'to' and 'obj1'");
	      break;

	    case 'O':
	      if (obj2)
		i = fname (obj2->name);
	      else
		log_string ("perform_act:bad code $O for 'obj2'");
	      break;

	    case 'p':
	      if (obj1)
		i = obj1->short_descr;
	      else
		log_string ("perform_act:bad code $p for 'obj1'");
	      break;

	    case 'P':
	      if (obj2)
		i = obj2->short_descr;
	      else
		log_string ("perform_act:bad code $P for 'obj2'");
	      break;

	    case 'd':
	      if (arg2 == NULL || ((const char *) arg2)[0] == '\0')
		{
		  i = "door";
		}
	      else
		{
		  char name[MIL];

		  one_argument ((const char *) arg2, name);
		  i = name;
		}
	      break;
	    }
	}

      while ((*point = *i) != '\0')
	point++, i++;
    }

  *point++ = '{';
  *point++ = 'x';
  if (cReturn)
    {
      *point++ = '\n';
      *point++ = '\r';
    }
  *point = '\0';

  return Upper (buf);
}

#define SENDOK(ch, type)    ((IsNPC(ch) || ((ch)->desc && (ch->desc->connected == CON_PLAYING))) \
            && (ch)->position >= min_pos)

void
act_new (const char *format, CharData * ch, const void *arg1,
	 const void *arg2, flag_t type, position_t min_pos)
{
  Descriptor *d;
  RoomIndex *room;
  CharData *to = (CharData *) arg2;

  if (NullStr (format))
    return;

  if (IsSet (type, TO_CHAR))
    {
      if (ch && SENDOK (ch, type))
	perform_act (format, ch, arg1, arg2, type, ch);
    }

  if (IsSet (type, TO_VICT))
    {
      if (to && SENDOK (to, type) && to != ch)
	perform_act (format, ch, arg1, arg2, type, to);
    }

  if (IsSet (type, TO_ZONE | TO_ALL))
    {
      for (d = descriptor_first; d; d = d->next)
	{
	  CharData *vch = CH (d);

	  if (vch && SENDOK (vch, type) && (vch != ch)
	      &&
	      ((IsSet
		(type, TO_ALL) || (vch->in_room
				   && vch->in_room->area ==
				   ch->in_room->area))))
	    perform_act (format, ch, arg1, arg2, type, vch);
	}
    }

  if (IsSet (type, TO_ROOM | TO_NOTVICT))
    {
      ObjData *obj1 = (ObjData *) arg1;
      ObjData *obj2 = (ObjData *) arg2;

      if (ch && ch->in_room != NULL)
	room = ch->in_room;
      else if (obj1 && obj1->in_room != NULL)
	room = obj1->in_room;
      else if (obj2 && obj2->in_room != NULL)
	room = obj2->in_room;
      else
	{
	  bugf ("no valid target '%s'", format);
	  return;
	}
      for (to = room->person_first; to; to = to->next_in_room)
	{
	  if (SENDOK (to, type) && (to != ch)
	      && (IsSet (type, TO_ROOM) || (to != (CharData *) arg2)))
	    perform_act (format, ch, arg1, arg2, type, to);
	}
    }
  return;
}

int
chprintf (CharData * ch, const char *fmt, ...)
{
  char buf[MPL];
  va_list args;

  if (NullStr (fmt) || !ch->desc)
    return 0;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  return d_print (ch->desc, buf);
}

int
chprintlnf (CharData * ch, const char *fmt, ...)
{
  char buf[MPL];

  if (!ch || !ch->desc)
    return 0;

  buf[0] = NUL;
  if (!NullStr (fmt))
    {
      va_list args;

      va_start (args, fmt);
      vsnprintf (buf, sizeof (buf), fmt, args);
      va_end (args);
    }
  return d_println (ch->desc, buf);
}

void
bugf (const char *fmt, ...)
{
  char buf[2 * MSL];
  va_list args;

  if (NullStr (fmt))
    return;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  bug (buf);
}

void
logf (const char *fmt, ...)
{
  char buf[2 * MSL];
  va_list args;

  if (NullStr (fmt))
    return;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  log_string (buf);
}

void
set_game_levels (int Old, int New)
{
  CmdData *cmd;
  HelpData *pHelp;
  ObjIndex *pObj;
  CharIndex *pMob;
  BanData *ban;
  DisabledData *d;
  ClanMember *mbr;
  WebpassData *wpass;
  int hash, sn, x;
  int diff = MAX_LEVEL - LEVEL_IMMORTAL;
  int imm_level = Old - diff;
  int mod = New - Old;

  logf ("Old Imm Level = %d, Old Max Level = %d.", imm_level, Old);
  logf ("New Imm Level = %d, New Max Level = %d.", New - diff, New);

  for (cmd = cmd_first; cmd; cmd = cmd->next)
    {
      if (cmd->level >= imm_level)
	cmd->level += mod;
    }
  rw_cmd_data (act_write);
  for (pHelp = help_first; pHelp; pHelp = pHelp->next)
    {
      if (pHelp->level >= imm_level)
	pHelp->level += mod;
    }
  rw_help_data (act_write);
  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pMob = char_index_hash[hash]; pMob; pMob = pMob->next)
	{
	  if (pMob->level >= imm_level)
	    {
	      pMob->level += mod;
	      SetBit (pMob->area->area_flags, AREA_CHANGED);
	    }
	}
      for (pObj = obj_index_hash[hash]; pObj; pObj = pObj->next)
	{
	  AffectData *paf;

	  if (pObj->level >= imm_level)
	    {
	      pObj->level += mod;
	      SetBit (pObj->area->area_flags, AREA_CHANGED);
	    }
	  switch (pObj->item_type)
	    {
	    case ITEM_POTION:
	    case ITEM_SCROLL:
	    case ITEM_PILL:
	    case ITEM_WAND:
	    case ITEM_STAFF:
	      if (pObj->value[0] >= imm_level)
		{
		  pObj->value[0] += mod;
		  SetBit (pObj->area->area_flags, AREA_CHANGED);
		}
	      break;
	    default:
	      break;
	    }
	  for (paf = pObj->affect_first; paf; paf = paf->next)
	    {
	      if (paf->level >= imm_level)
		{
		  paf->level += mod;
		  SetBit (pObj->area->area_flags, AREA_CHANGED);
		}
	    }
	}
    }
  do_function (NULL, &do_asave, "changed");
  for (ban = ban_first; ban; ban = ban->next)
    {
      if (ban->level >= imm_level)
	ban->level += mod;
    }
  rw_ban_data (act_write);
  for (d = disabled_first; d; d = d->next)
    {
      if (d->level >= imm_level)
	d->level += mod;
    }
  rw_disabled_data (act_read);
  for (sn = 0; sn < top_skill; sn++)
    {
      for (x = 0; x < top_class; x++)
	{
	  if (skill_table[sn].skill_level[x] >= imm_level)
	    skill_table[sn].skill_level[x] += mod;
	}
    }
  rw_skill_data (act_write);
  for (mbr = mbr_first; mbr; mbr = mbr->next)
    {
      if (mbr->level >= imm_level)
	mbr->level += mod;
    }
  rw_mbr_data (act_write);
  for (wpass = wpwd_first; wpass; wpass = wpass->next)
    {
      if (wpass->level >= imm_level)
	wpass->level += mod;
    }
  rw_wpwd_data (act_write);
}

const char *
print_ip (unsigned long ip)
{
  struct sockaddr_in sock;
  static char buf[5][100];
  static int i;

  i++;
  i %= 5;

  sock.sin_addr.s_addr = ip;
  strcpy (buf[i], inet_ntoa (sock.sin_addr));
  return buf[i];
}

size_t
line_count (const char *s)
{
  size_t count = 0;

  for (; *s; s++)
    if (*s == '\n')
      count++;

  return count;
}

int
col_print (Column * c, const char *txt)
{
  switch (c->type)
    {
    case COLS_BUF:
      return bprint ((Buffer *) c->to, txt);
    case COLS_CHAR:
      return chprint ((CharData *) c->to, txt);
    case COLS_DESC:
      return d_print ((Descriptor *) c->to, txt);
    default:
      bugf ("void_print: invalid type");
      return 0;
    }
}

int
print_cols (Column * c, const char *format, ...)
{
  int out = 0;
  size_t buf_len = 0;
  bool has_nl = false;

  if (!c)
    return 0;

  if (!NullStr (format))
    {
      va_list args;
      char buf[MPL];

      va_start (args, format);
      vsnprintf (buf, sizeof (buf), format, args);
      va_end (args);

      buf_len = cstrlen (buf);
      c->line_pos += buf_len;

      if (c->line_pos >= c->line_len)
	{
	  if (!c->newline)
	    out += col_print (c, NEWLINE);
	  c->col_pos = 0;
	  c->newline = true;
	  c->line_pos = buf_len;
	}
      has_nl = has_newline (buf);
      out += col_print (c, buf);
    }
  if (has_nl)
    {
      c->newline = true;
      c->col_pos = c->line_pos = 0;
    }
  else if (++c->col_pos >= c->columns)
    {
      if (!c->newline)
	col_print (c, NEWLINE);
      c->newline = true;
      c->col_pos = c->line_pos = 0;
    }
  else
    {
      int col_diff = 0;

      if (c->col_len > buf_len)
	{
	  col_diff = c->col_len - buf_len;
	}
      else if (buf_len > c->col_len)
	{
	  int diff_count = 0;

	  while (buf_len > c->col_len)
	    {
	      ++diff_count;
	      buf_len -= c->col_len;
	    }

	  col_diff = (c->col_len * diff_count) - buf_len;
	  c->col_pos += diff_count;
	}
      if (col_diff > 0)
	{
	  c->line_pos += col_diff;
	  out += col_print (c, FORMATF ("%*s", col_diff, " "));
	}
      c->newline = false;
    }
  return out;
}

bool
cols_nl (Column * c)
{
  if (!c)
    return false;

  if (!c->newline)
    {
      col_print (c, NEWLINE);
      c->newline = true;
      c->line_pos = 0;
      c->col_pos = 0;
      return true;
    }

  return false;
}

void
set_cols (Column * c, CharData * ch, int columns, column_t type, void *to)
{
  if (!c)
    return;

  c->newline = false;
  c->line_len = get_scr_cols (ch);
  c->col_pos = 0;
  c->line_pos = 0;
  c->columns = columns;
  c->type = type;
  c->to = to;
  c->col_len = c->line_len / c->columns;
}

int
cols_header (Column * c, const char *format, ...)
{
  size_t i;
  int out;
  char head[MPL];
  va_list args;

  if (NullStr (format) || !c)
    return 0;

  va_start (args, format);
  vsnprintf (head, sizeof (head), format, args);
  va_end (args);

  for (i = out = 0; i < c->columns && !c->newline; i++)
    out += print_cols (c, head);

  return out;
}
