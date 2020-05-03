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
#include "data_table.h"
#include "recycle.h"
#include "tables.h"

#ifdef HAVE_SETITIMER
struct itimerval vtimer;

void
set_vtimer (long sec)
{
  struct itimerval otimer;

  if (IsSet (mud_info.disabled_signals, MakeBit (SIGVTALRM)))
    return;

  vtimer.it_value.tv_sec = sec <= 0 ? (MINUTE * 3) : sec;
  vtimer.it_value.tv_usec = 0;

#ifdef __CYGWIN__

  if (setitimer (ITIMER_REAL, &vtimer, &otimer) == -1)
#else

  if (setitimer (ITIMER_VIRTUAL, &vtimer, &otimer) == -1)
#endif

    {
      log_error ("Failed to set vtimer.");
      exit (1);
    }

  vtimer.it_interval = otimer.it_value;
}

RETSIGTYPE
sigalarm (int sig)
{
  static int safe_check = 0;
  char crash_message_a[] =
    "The mud has been looping for the past 60 seconds.";
  char crash_message_b[] = "Initiating reboot...";
  char crash_message_c[] =
    "The mud failed to inform the players of the above.";

  if (crash_info.status == CRASH_LOOPING)
    return;

  switch (safe_check)
    {
    case 0:
      safe_check = 1;
      bug (crash_message_a);
      bug (crash_message_b);
      break;

    case 1:
      safe_check = 2;
      log_string (crash_message_a);
      log_string (crash_message_b);
      log_string (crash_message_c);
      break;

    case 2:
      break;
    }



  set_vtimer (-1);

  halt_mud (sig);

  exit (0);
}
#elif defined HAVE_ALARM
RETSIGTYPE
sigalarm (int sig)
{
  static int attempt = 0;
  static bool boredom = false;
  time_t tm;

  if (boredom)
    log_string ("TOCK!");
  else
    log_string ("TICK!");

  boredom = !boredom;

  if (crash_info.status == CRASH_LOOPING)
    return;

  time (&tm);
  if ((tm - current_time) > 120 || crash_info.crashed)
    {
      if (attempt != 1)
	{
	  attempt = 1;
	  halt_mud (sig);
	}
      raise (SIGSEGV);
      exit (0);
    }

  alarm (MINUTE * 3);
}

#endif

char *
crash_cmd_info (void)
{
  switch (crash_info.status)
    {
    case CRASH_UNLIKELY:
      return "It is very UNlikely that this command caused the crash.";

    case CRASH_LIKELY:
      return "It is VERY likely that this command caused the crash.";

    case CRASH_PRE_PROCESSING:
      return "This crash occured during the command preprocessing.";

    case CRASH_POST_PROCESSING:
      return "This crash occured during the command postprocessing.";

    case CRASH_UPDATING:
      return "This crash occured while updating the above.";

    case CRASH_UNKNOWN:
      return
	"This crash occured after all updates were complete.  Unknown cause.";

    case CRASH_BOOT:
      return "This crash occured during boot.  Check log for cause.";

    default:
      return "Unknown cause.";
    }
}

void
send_crash_info (void)
{

  if (crash_info.shrt_cmd[0] && crash_info.desc != NULL
      && crash_info.status == CRASH_LIKELY)
    {
      d_write (crash_info.desc, NEWLINE "The last command you typed, '", 0);
      d_write (crash_info.desc, crash_info.shrt_cmd, 0);
      d_write (crash_info.desc,
	       "', might have caused this crash." NEWLINE
	       "Please note any unusual circumstances to IMP and avoid using that command."
	       NEWLINE, 0);
    }
}

void
crash_log (const char *msg)
{
  FILE *fp;

#ifdef HAVE_GDB

  char gdb[MPL];
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  struct stat fst;
#endif

  if (crash_info.crashed > 1)
    return;

#ifdef HAVE_GDB
  gdb[0] = NUL;
  sprintf (buf, "%s/core", CWDIR);
  if (stat (buf, &fst) != -1)
    {
      sprintf (buf2, "gdb -batch %s %s", EXE_FILE, buf);
      if ((fp = popen (buf2, "r")) != NULL)
	{
	  fread (gdb, MPL - 1000, 1, fp);
	  pclose (fp);
	}
      sprintf (buf2, "mv -f %s %s/core.%d", buf, BIN_DIR, getpid ());
      system (buf2);
    }
#endif

  if ((fp = fopen (LOG_DIR "crash.log", "w")) != NULL)
    {
      fprintf (fp, "Crash on %s.\n", str_time (-1, -1, NULL));
      fprintf (fp, "%s\n", fix_string (msg));
#ifdef HAVE_GDB

      if (gdb != NULL)
	fprintf (fp, "%s\n", gdb);
#endif

    }
  fclose (fp);

#ifdef HAVE_SENDMAIL

  if ((fp = popen ("sendmail -t", "w")) != NULL)
    {
      fprintf (fp, "To: %s Administrator <%s@%s>" LF, mud_info.name,
	       UNAME, HOSTNAME);
      fprintf (fp, "From: %s <%s@%s>" LF, mud_info.name, UNAME, HOSTNAME);
      fprintf (fp, "Reply-to: %s <%s@%s>" LF, mud_info.name, UNAME, HOSTNAME);
      fprintf (fp, "X-Mailer: %s" LF, mud_info.name);
      fprintf (fp, "Subject: Crash: %s" LF, str_time (-1, -1, NULL));
      fprintf (fp, LF);
      fprintf (fp, "%s" LF, fix_string (msg));
#ifdef HAVE_GDB

      if (gdb != NULL)
	{
	  fprintf (fp, "---GDB OUTPUT---" LF);
	  fprintf (fp, "%s" LF, gdb);
	}
#endif
      pclose (fp);
    }
#endif
  return;
}

RETSIGTYPE
halt_mud (int sig)
{
  Descriptor *d;
  CharData *ch;
  char message[MSL];

#ifdef HAVE_WORKING_FORK

  struct sigaction default_action;
  pid_t forkpid;
  int i;
  int status;

  waitpid (-1, &status, WNOHANG);
  switch (crash_info.crashed)
    {
    case 0:
#endif

      crash_info.crashed++;
#ifdef HAVE_STRSIGNAL

      logf ("GAME CRASHED: %s", strsignal (sig));
#elif defined HAVE_PSIGNAL

      psignal (sig, "GAME CRASHED");
#endif

      send_crash_info ();
      sprintf (message, NEWLINE "---CRASH INFORMATION---" NEWLINE "Signal %d"
#ifdef HAVE_STRSIGNAL
	       " (%s)"
#endif
	       NEWLINE "Last recorded function: %s" NEWLINE "Details: %s"
	       NEWLINE "%s" NEWLINE, sig,
#ifdef HAVE_STRSIGNAL
	       strsignal (sig),
#endif
	       crash_info.shrt_cmd, crash_info.long_cmd, crash_cmd_info ());

      for (d = descriptor_first; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  ch = CH (d);
	  if (!ch)
	    {
	      close_socket (d);
	      continue;
	    }

	  save_char_obj (ch);

	  d_write (d, NEWLINE "\007", 3);
	  d_write (d, mud_info.name, 0);
	  d_write (d, " has CRASHED.\007" NEWLINE, 0);

	  if (IsImmortal (ch))
	    d_write (d, message, 0);
	}

#ifdef HAVE_WORKING_FORK


      if ((forkpid = fork ()) > 0)
	{

	  waitpid (forkpid, &status, WNOHANG);
	  crs_info.status = CRS_COPYOVER;
	  copyover ();
	  exit (0);
	}
      else if (forkpid < 0)
	{
	  exit (1);
	}


      for (i = 255; i >= 0; i--)
	close (i);


      open (NULL_FILE, O_RDWR);
      dup (0);
      dup (0);

      default_action.sa_handler = SIG_DFL;
      sigaction (sig, &default_action, NULL);


      if (!fork ())
	{
	  crash_log (message);
	  exit (1);
	}
      else
	return;
      raise (sig);
      break;

    case 1:
      crash_info.crashed++;

      for (d = descriptor_first; d != NULL; d = d_next)
	{
	  d_next = d->next;
	  ch = d->original ? d->original : d->character;
	  if (ch == NULL)
	    {
	      close_socket (d);
	      continue;
	    }

	  d_write (d,
		   "** Error saving character files; conducting full reboot. **\007"
		   NEWLINE, 0);
	  close_socket (d);
	  continue;
	}
      log_string ("CHARACTERS NOT SAVED.");
      default_action.sa_handler = SIG_DFL;
      sigaction (sig, &default_action, NULL);

      if (!fork ())
	{
	  kill (getppid (), sig);
	  exit (1);
	}
      else
	return;
      raise (sig);
      break;

    case 2:
      crash_info.crashed++;
      log_string ("TOTAL GAME CRASH.");
      default_action.sa_handler = SIG_DFL;
      sigaction (sig, &default_action, NULL);

      if (!fork ())
	{
	  kill (getppid (), sig);
	  exit (1);
	}
      else
	return;
      raise (sig);
      break;

    case 3:
      default_action.sa_handler = SIG_DFL;
      sigaction (sig, &default_action, NULL);

      if (!fork ())
	{
	  kill (getppid (), sig);
	  exit (1);
	}
      else
	return;
      raise (sig);
      break;
    }
#endif
}

void
cleanup_mud (void)
{
  EXTERN FileData *fpArea;

#ifdef HAVE_SETITIMER

  set_vtimer (-1);
#endif

  while (auction_first != NULL)
    reset_auc (auction_first, true);
  rw_gquest_data (act_write);
  rw_war_data (act_write);
  rw_mud_data (act_write);
  rw_time_data (act_write);
  rw_note_data (act_write);
  do_function (NULL, &do_asave, "changed");
  save_room_objs ();
#ifndef DISABLE_WEBSRV

  shutdown_web_server ();
#endif
#ifndef DISABLE_I3

  I3_shutdown (0);
#endif

  close_network ();
  fflush (NULL);

  if (fpArea)
    f_close (fpArea);

  if (fpReserve != NULL)
    fclose (fpReserve);

  log_string ("Mud cleanup successfull.");
  logf ("%s ran for %s.", mud_info.name,
	timestr (getcurrenttime () - boot_time, false));
}

void
exit_mud (void)
{
  Descriptor *d, *d_next;

  logf ("Normal program termination...");
  for (d = descriptor_first; d != NULL; d = d_next)
    {
      d_next = d->next;
      d_write (d, NEWLINE "Normal program termination..." NEWLINE, 0);
      if (CH (d) != NULL)
	{
	  save_char_obj (CH (d));
	  d_write (d, NEWLINE "Saving, and disconnecting..." NEWLINE, 0);
	}
      d->outtop = 0;
      close_socket (d);
    }
  cleanup_mud ();
}


RETSIGTYPE
terminate_mud (int sig)
{
  Descriptor *d;
  CharData *ch;
  char message[MSL];

  crash_info.crashed++;
  log_string ("GAME TERMINATED");

  sprintf (message,
	   NEWLINE "Last recorded function: %s" NEWLINE "Details: %s"
	   NEWLINE "%s" NEWLINE,
	   crash_info.shrt_cmd, crash_info.long_cmd, crash_cmd_info ());

  for (d = descriptor_first; d != NULL; d = d_next)
    {
      d_next = d->next;
      ch = CH (d);
      if (!ch)
	{
	  close_socket (d);
	  continue;
	}

      save_char_obj (ch);

      d_write (d, NEWLINE "\007", 3);
      d_write (d, mud_info.name, 0);
      d_write (d, " has been TERMINATED.\007" NEWLINE, 0);

      if (IsImmortal (ch))
	d_write (d, message, 0);
    }
  exit (1);
}

const struct sig_type sig_table[] = {
#ifndef WIN32
  {"SIGPIPE", SIGPIPE, SIG_IGN, 0},
  {"SIGCHLD", SIGCHLD, SIG_IGN, 0},
  {"SIGHUP", SIGHUP, SIG_IGN, 0},
  {"SIGQUIT", SIGQUIT, halt_mud, SA_NODEFER},
  {"SIGBUS", SIGBUS, halt_mud, SA_NODEFER},
  {"SIGUSR1", SIGUSR1, halt_mud, SA_NODEFER},
  {"SIGUSR2", SIGUSR2, halt_mud, SA_NODEFER},
#else
#define SA_NODEFER 0
#endif
  {"SIGINT", SIGINT, halt_mud, SA_NODEFER},
  {"SIGILL", SIGILL, halt_mud, SA_NODEFER},
  {"SIGFPE", SIGFPE, halt_mud, SA_NODEFER},
  {"SIGSEGV", SIGSEGV, halt_mud, SA_NODEFER},
  {"SIGTERM", SIGTERM, terminate_mud, SA_NODEFER},
  {"SIGABRT", SIGABRT, halt_mud, SA_NODEFER},
#ifdef HAVE_SETITIMER
  {"SIGVTALRM", SIGVTALRM, sigalarm, SA_NODEFER},
#elif defined HAVE_ALARM
  {"SIGALRM", SIGALRM, sigalarm, SA_NODEFER},
#endif
  {NULL, -1, NULL, -1}
};

bool
init_sig (const struct sig_type *tabl)
{

  if (IsSet (mud_info.disabled_signals, MakeBit (tabl->sig)))
    return false;

#ifdef WIN32

  signal (tabl->sig, tabl->sigfun);
#else

  {
    struct sigaction sigact;

    sigact.sa_flags = tabl->flags;
    sigact.sa_handler = (RETSIGTYPE (*)(int)) tabl->sigfun;
    sigemptyset (&sigact.sa_mask);

    sigaction (tabl->sig, &sigact, NULL);
  }
#endif

#ifdef HAVE_SETITIMER
  if (tabl->sig == SIGVTALRM)
    {

      vtimer.it_interval.tv_sec = MINUTE * 3;
      vtimer.it_interval.tv_usec = 0;
      set_vtimer (-1);
    }
#elif defined HAVE_ALARM
  if (tabl->sig == SIGALRM)
    alarm (MINUTE * 3);
#endif

  return true;
}

void
set_signals (void)
{
  int i;

  crash_info.desc = NULL;
  crash_info.shrt_cmd[0] = '\0';
  crash_info.long_cmd[0] = '\0';
  crash_info.status = CRASH_BOOT;
  crash_info.crashed = 0;

  for (i = 0; sig_table[i].name != NULL; i++)
    init_sig (&sig_table[i]);

#if defined HAVE_ATEXIT || defined(WIN32)

  atexit (exit_mud);
#endif

  log_string ("Signals Initialized.");

  crs_info.who = &str_empty[0];
  crs_info.reason = &str_empty[0];
}

Do_Fun (do_crash)
{
  char arg[MIL];
  int i;

  argument = one_argument (argument, arg);

  if (get_trust (ch) < MAX_LEVEL)
    {
      chprintln (ch, "You don't have enough security to use this command.");
      return;
    }

  if (NullStr (arg))
    {
      Column c;

      set_cols (&c, ch, 4, COLS_CHAR, ch);
      cmd_syntax (ch, NULL, n_fun, "<sig>", NULL);
      chprint (ch, "Available signals:");
      for (i = 0; sig_table[i].name != NULL; i++)
	print_cols (&c, " %s", sig_table[i].name);
      cols_nl (&c);
      return;
    }

  for (i = 0; sig_table[i].name != NULL; i++)
    {
      if (!str_prefix (arg, sig_table[i].name))
	{
	  chprintlnf (ch, "Sending %s signal to %s...", sig_table[i].name,
		      mud_info.name);
	  raise (sig_table[i].sig);
	  break;
	}
    }

  do_crash (n_fun, ch, "");
  return;
}
