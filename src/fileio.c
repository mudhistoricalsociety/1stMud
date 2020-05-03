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



void
write_string (FileData * fp, const char *name, const char *str,
	      const char *def)
{
  if (NullStr (name))
    f_printf (fp, "%s~" LF, fix_string (str));
  else if (!NullStr (str) && (NullStr (def) || str_cmp (str, def)))
    f_printf (fp, "%s%s%s~" LF, name, format_tabs (strlen (name)),
	      fix_string (str));
}


void
write_word (FileData * fp, const char *name, const char *str, const char *def)
{
  if (NullStr (name))
    f_printf (fp, "'%s'" LF, fix_string (str));
  else if (!NullStr (str) && (NullStr (def) || str_cmp (str, def)))
    f_printf (fp, "%s%s'%s'" LF, name, format_tabs (strlen (name)),
	      fix_string (str));
}



void
write_bit (FileData * fp, const char *name, flag_t bit, flag_t def)
{
  if (NullStr (name))
    f_printf (fp, "%s" LF, write_flags (bit));
  else if (bit != def)
    f_printf (fp, "%s%s%s" LF, name, format_tabs (strlen (name)),
	      write_flags (bit));
}

void
write_header (FileData * fp, const char *head)
{
  if (!NullStr (head))
    f_printf (fp, "%s%s", head, format_tabs (strlen (head)));
}

void
f_writef (FileData * fp, const char *name, const char *format, ...)
{
  if (!NullStr (format))
    {
      va_list args;
      char buf[MPL];

      va_start (args, format);

      vsnprintf (buf, sizeof (buf), format, args);
      va_end (args);
      if (!NullStr (name))
	f_printf (fp, name);
      f_printf (fp, "%s%s", format_tabs (strlen (name)), buf);
    }
  else if (!NullStr (name))
    {
      f_printf (fp, name);
    }
}

void
write_time (FileData * fp, const char *name, time_t time, bool fTime)
{
  if (!NullStr (name))
    f_printf (fp, "%s%s", name, format_tabs (strlen (name)));
  else
    f_printf (fp, " ");

  f_printf (fp, TIME_T_FMT, time);

  f_printf (fp, " (%s)" LF,
	    !fTime ? str_time (time, -1,
			       NULL) : timestr (current_time - time, false));
}

void
read_time (FileData * fp, time_t * tptr)
{
  *tptr = read_long (fp);
  read_to_eol (fp);
}

void
read_dice (FileData * fp, int *dice)
{
  dice[DICE_NUMBER] = read_number (fp);
  read_letter (fp);
  dice[DICE_TYPE] = read_number (fp);
  read_letter (fp);
  dice[DICE_BONUS] = read_number (fp);
}

bool
is_space (char c)
{
  if (isspace (c))
    {
      switch (c)
	{
	case CUSTOMSTART:
	case CUSTOMEND:
	case MXP_BEGc:
	case MXP_ENDc:
	  return false;
	default:
	  return true;
	}
    }
  else
    return false;
}

#define MLR (MAX_STRING_LENGTH * 25)

const char *
read_file (FileData * fp)
{
  char buf[MLR + 2];
  char c;
  size_t i = 0;
  bool sFull = false;


  memset (buf, 0, sizeof (buf));

  for (;;)
    {
      if (i >= MLR && !sFull)
	{
	  bugf ("file size exceeded %d maximum.", MLR);
	  sFull = true;
	}

      switch (c = f_getc (fp))
	{
	default:
	  if (!sFull)
	    {
	      buf[i] = c;
	      i++;
	    }
	  break;

	case '\n':
	  if (!sFull)
	    {
	      buf[i] = '\n';
	      i++;
	      buf[i] = '\r';
	      i++;
	    }
	  break;

	case '\r':
	  break;

	case NUL:
	  return str_dup (buf);

	}
    }
}


FILE *
file_open (const char *file, const char *mode)
{
  FILE *fp;

  if (fpReserve != NULL)
    {
      fclose (fpReserve);
      fpReserve = NULL;
    }

  if (!(fp = fopen (file, mode)))
    {
      fpReserve = fopen (NULL_FILE, "r");
      return NULL;
    }
  return fp;
}


void
file_close (FILE * fp)
{
  if (fp != NULL)
    {
      fflush (fp);
      fclose (fp);
    }
  fp = NULL;

  if (fpReserve == NULL)
    fpReserve = fopen (NULL_FILE, "r");
}

char
read_letter (FileData * fp)
{
  char c;

  do
    {
      c = f_getc (fp);
    }
  while (is_space (c));

  return c;
}

int
read_number (FileData * fp)
{
  int number;
  bool sign;
  char c;

  do
    {
      c = f_getc (fp);
    }
  while (is_space (c));

  number = 0;

  sign = false;
  if (c == '+')
    {
      c = f_getc (fp);
    }
  else if (c == '-')
    {
      sign = true;
      c = f_getc (fp);
    }

  if (!isdigit (c))
    {
      bugf ("bad format '%c'... %s.", c,
	    AttemptJump ? "attempting longjmp" : "aborting");
      if (AttemptJump)
	{
	  longjmp (jump_env, 1);
	}
      else
	abort ();
    }

  while (isdigit (c))
    {
      number = number * 10 + c - '0';
      c = f_getc (fp);
    }

  if (sign)
    number = 0 - number;

  if (c == '|')
    number += read_number (fp);
  else if (c != ' ')
    f_ungetc (c, fp);

  return number;
}

long
read_long (FileData * fp)
{
  long number;
  bool sign;
  char c;

  do
    {
      c = f_getc (fp);
    }
  while (is_space (c));

  number = 0;

  sign = false;
  if (c == '+')
    {
      c = f_getc (fp);
    }
  else if (c == '-')
    {
      sign = true;
      c = f_getc (fp);
    }

  if (!isdigit (c))
    {
      bugf ("bad format '%c'... %s.", c,
	    AttemptJump ? "attempting longjmp" : "aborting");
      if (AttemptJump)
	{
	  longjmp (jump_env, 1);
	}
      else
	abort ();
    }

  while (isdigit (c))
    {
      number = number * 10 + c - '0';
      c = f_getc (fp);
    }

  if (sign)
    number = 0 - number;

  if (c == '|')
    number += read_long (fp);
  else if (c != ' ')
    f_ungetc (c, fp);

  return number;
}

flag_t
read_flag (FileData * fp)
{
  flag_t number;
  flag_t flag;
  flag_t temp = 1;
  char c;
  bool negative = false;

  do
    {
      c = f_getc (fp);
    }
  while (is_space (c));

  if (c != '+')
    {
      if (c == '-')
	{
	  negative = true;
	  c = f_getc (fp);
	}

      number = 0;

      if (!isdigit (c))
	{
	  while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
	    {
	      number += flag_convert (c);
	      c = f_getc (fp);
	    }
	}

      while (isdigit (c))
	{
	  number = number * 10 + c - '0';
	  c = f_getc (fp);
	}

      if (c == '|')
	number += read_flag (fp);

      else if (c != ' ')
	f_ungetc (c, fp);

      if (negative)
	return -1 * number;
      return number;
    }
  else
    {
      number = 0;
      flag = 0;
      do
	{
	  c = f_getc (fp);
	  flag += (temp << number) * (c == 'Y');
	  number++;
	}
      while (c == 'Y' || c == 'n');

      if (c == '\n' || c == '\r')
	f_ungetc (c, fp);

      return flag;
    }
}

void
read_to_eol (FileData * fp)
{
  char c;

  do
    {
      c = f_getc (fp);
    }
  while (c != '\n' && c != '\r');

  do
    {
      c = f_getc (fp);
    }
  while (c == '\n' || c == '\r');

  f_ungetc (c, fp);
  return;
}

char *
read_word (FileData * fp)
{
  static char word[MIL];
  char *pword;
  char cEnd;

  do
    {
      if (f_eof (fp))
	{
	  bug ("EOF encountered on read.");
	  if (run_level == RUNLEVEL_BOOTING)
	    exit (1);
	  word[0] = NUL;
	  return word;
	}
      cEnd = f_getc (fp);
    }
  while (is_space (cEnd));

  if (cEnd == '\'' || cEnd == '"')
    {
      pword = word;
    }
  else
    {
      word[0] = cEnd;
      pword = word + 1;
      cEnd = ' ';
    }

  for (; pword < word + MIL; pword++)
    {
      *pword = f_getc (fp);
      if (cEnd == ' ' ? is_space (*pword) : *pword == cEnd)
	{
	  if (cEnd == ' ')
	    f_ungetc (*pword, fp);
	  *pword = NUL;
	  return word;
	}
    }

  bugf ("word too long... %s.",
	AttemptJump ? "attempting longjmp" : "aborting");

  if (AttemptJump)
    {
      longjmp (jump_env, 1);
    }
  else
    abort ();
  return NULL;
}

const char *
read_string (FileData * fp)
{
  static char buf[MLR + 2];
  size_t i = 0;
  register char c;
  bool sFull = false;


  do
    {
      c = f_getc (fp);
    }
  while (is_space (c));


  if (c == '~')
    return str_dup ("");

  buf[i++] = c;

  for (;;)
    {
      if (i >= MLR && !sFull)
	{
	  bugf ("String [%20.20s...] exceeded [%d] MLR", buf, MLR);
	  sFull = true;
	}

      switch (c = f_getc (fp))
	{
	default:
	  if (!sFull)
	    {
	      buf[i++] = c;
	    }
	  break;

	case NUL:

	  bugf ("EOF reached... %s.",
		AttemptJump ? "attempting longjmp" : "aborting");
	  if (AttemptJump)
	    {
	      longjmp (jump_env, 1);
	    }
	  else
	    abort ();
	  break;

	case '\n':
	  if (!sFull)
	    {
	      buf[i++] = '\n';
	      buf[i++] = '\r';
	    }
	  break;

	case '\r':
	  break;

	case '~':
	  buf[i] = NUL;
	  return str_dup (buf);
	  break;
	}
    }
}

void
read_strfree (FileData * fp, const char **pstr)
{
  free_string (*pstr);
  *pstr = read_string (fp);
}


const char *
read_line (FileData * fp)
{
  static char line[MSL];
  char *pline;
  char c;
  int ln;

  pline = line;
  line[0] = NUL;
  ln = 0;


  do
    {
      if (f_eof (fp))
	{
	  bug ("EOF encountered on read.");
	  strcpy (line, "");
	  return line;
	}
      c = f_getc (fp);
    }
  while (is_space (c));

  f_ungetc (c, fp);
  do
    {
      if (f_eof (fp))
	{
	  bug ("EOF encountered on read.");
	  *pline = NUL;
	  return line;
	}
      c = f_getc (fp);
      *pline++ = c;
      ln++;
      if (ln >= (MSL - 1))
	{
	  bug ("line too long");
	  break;
	}
    }
  while (c != '\n' && c != '\r');

  do
    {
      c = f_getc (fp);
    }
  while (c == '\n' || c == '\r');

  f_ungetc (c, fp);
  *pline = NUL;
  return line;
}

#ifdef STFILEIO

stFile *
stopen (const char *file, const char *mode)
{
  struct stat buf;
  FileData *stfp;

  if (NullStr (file))
    return NULL;

  stfp = new_stfile ();
  if (stfp == NULL)
    return NULL;

  stfp->file = str_dup (file);
  stfp->mode = str_dup (mode);

  if (mode[0] == NUL || tolower (mode[0]) == 'r')
    {
      if (stat (file, &buf) == -1 || !S_ISREG (buf.st_mode))
	{
	  free_stfile (stfp);
	  return NULL;
	}

      if ((stfp->stream = file_open (file, mode)) == NULL)
	{
	  free_stfile (stfp);
	  return NULL;
	}

      stfp->pos = 0;
      stfp->size = buf.st_size;
      alloc_mem (stfp->str, char, stfp->size + 1);
      fread (stfp->str, stfp->size + 1, 1, stfp->stream);
      stfp->str[stfp->size] = NUL;
    }
  else
    {
      static int count = 0;

      ++count, count %= 999;

#if !defined WIN32 && !defined __CYGWIN__

      stfp->temp = str_dupf ("%s.tmp%03d", stfp->file, count);
      if ((stfp->stream = file_open (stfp->temp, mode)) == NULL)
#else

      if ((stfp->stream = file_open (stfp->file, mode)) == NULL)
#endif

	{
	  free_stfile (stfp);
	  return NULL;
	}
    }
  Link (stfp, stfile, next, prev);
  return stfp;
}

int
stclose (stFile * fp)
{
#if !defined WIN32 && !defined __CYGWIN__
  if (!NullStr (fp->temp) && rename (fp->temp, fp->file) == -1)
    {
      bugf ("error renaming %s -> %s", fp->temp, fp->file);
    }
#endif
  free_stfile (fp);
  fp = NULL;
  return 0;
}

int
stflush (stFile * fp)
{
  if (fp != NULL)
    {
      if (!NullStr (fp->mode) && fp->mode[0] != 'r')
	{
	  fprintf (fp->stream, "%s\n", fp->str);
	}
      fflush (fp->stream);
    }
  else
    for (fp = stfile_first; fp; fp = fp->next)
      stflush (fp);

  return 0;
}

int
steof (FileData * fp)
{
  return (fp->pos >= fp->size) ? EOF : 0;
}

int
stgetc (FileData * fp)
{
  char c = fp->str[fp->pos];

  if (!steof (fp))
    fp->pos++;

  return c;
}

int
stungetc (int c, stFile * fp)
{
  if (fp->pos > 0)
    fp->pos--;
  return fp->str[fp->pos];
}


size_t
stread (void *ptr, size_t size, size_t nmemb, stFile * fp)
{
  size_t nsize;

  if (!fp->str || steof (fp))
    return 0;

  nsize = Min (fp->pos + size, fp->size);

  memcpy (ptr, &fp->str[fp->pos], nsize);
  return nsize;
}

int
stprintf (stFile * fp, const char *fmt, ...)
{
  va_list args;
  int len;

  if (!fp || NullStr (fmt) || NullStr (fp->mode)
      || tolower (fp->mode[0]) == 'r' || !fp->stream)
    return 0;

  va_start (args, fmt);
  len = vfprintf (fp->stream, fmt, args);
  va_end (args);

  return len;
}

int
stseek (stFile * fp, long offset, int whence)
{
  if (!fp)
    return -1;

  switch (whence)
    {
    case SEEK_SET:
      fp->pos = 0;
      break;
    default:
    case SEEK_CUR:
      break;
    case SEEK_END:
      fp->pos = fp->size;
      break;
    }

  if (fp->pos + offset < 0 || fp->pos + offset >= fp->size)
    return -1;

  fp->pos += offset;
  return 0;
}

#endif
