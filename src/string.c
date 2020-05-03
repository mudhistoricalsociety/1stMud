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
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
*  This code was freely distributed with the The Isles 1.1 source code,   *
*  and has been used here for OLC - OLC would not be what it is without   *
*  all the previous coders who released their source code.                *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/



#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "recycle.h"

Proto (const char *string_linedel, (const char *, size_t));
Proto (const char *string_lineadd, (const char *, const char *, size_t));
Proto (const char *numlines, (const char *));
Proto (const char *indent_string, (const char *, int));
Proto (const char *get_line, (const char *, char *));

#define STREP_SIZE (MSL * 5)


void
string_edit (CharData * ch, const char **pString)
{
  chprintln (ch,
	     stringf (ch, 0, Center, "{c-{C-", "{W Entering EDIT Mode ]"));
  chprintln (ch,
	     stringf (ch, 0, Center, NULL,
		      "Type %ch on a new line for help", StrEdKey (ch)));
  chprintln (ch,
	     stringf (ch, 0, Center, NULL,
		      "Terminate with a %cq or @ on a blank line.",
		      StrEdKey (ch)));
  chprintln (ch, draw_line (ch, "{c-{C-", 0));

  if (*pString == NULL)
    {
      *pString = str_dup ("");
    }
  else
    {
      *(char *) *pString = '\0';
    }

  ch->desc->pString = pString;

  return;
}


void
string_append (CharData * ch, const char **pString)
{
  chprintln (ch,
	     stringf (ch, 0, Center, "{c-{C-", "{W Entering APPEND Mode ]"));
  chprintln (ch,
	     stringf (ch, 0, Center, NULL,
		      "Type %ch on a new line for help", StrEdKey (ch)));
  chprintln (ch,
	     stringf (ch, 0, Center, NULL,
		      "Terminate with a %cq or @ on a blank line.",
		      StrEdKey (ch)));
  chprintln (ch, draw_line (ch, "{c-{C-", 0));

  if (*pString == NULL)
    {
      *pString = str_dup ("");
    }
  sendpage (ch, numlines (*pString));

  ch->desc->pString = pString;

  return;
}


const char *
string_replace (const char *orig, const char *old, const char *pnew)
{
  char buf[STREP_SIZE];
  const char *ptr;
  size_t i;

  if ((ptr = strstr (orig, old)) == NULL || !str_cmp (old, pnew) ||
      (i = strlen (orig) - strlen (ptr)) + strlen (pnew) >= (STREP_SIZE - 4))
    return orig;

  strcpy (buf, orig);
  buf[i] = NUL;
  strcat (buf, pnew);
  strcat (buf, &orig[i + strlen (old)]);

  free_string (orig);
  return str_dup (buf);
}

const char *
string_replace_all (const char *orig, const char *old, const char *pnew)
{
  const char *ptr;

  if ((ptr = strstr (orig, old)) == NULL || !str_cmp (old, pnew)
      || (strlen (orig) - strlen (ptr)) + strlen (pnew) >= (STREP_SIZE - 4))
    return orig;

  do
    {
      orig = string_replace (orig, old, pnew);
    }
  while ((ptr = strstr (orig, old)) != NULL && !strstr (pnew, old) &&
	 (strlen (orig) - strlen (ptr)) + strlen (pnew) < (STREP_SIZE - 4));

  return orig;
}

char *
strnzncat (char *dest, const char *src, size_t len, size_t count)
{
  size_t old_len;
  size_t ncopy;

  old_len = strlen (dest);
  if (old_len >= len - 1)
    return dest;

  ncopy = len - old_len - 1;
  if (count < ncopy)
    ncopy = count;

  strncat (dest, src, ncopy);
  return dest;
}


size_t
strlcpy (char *dst, const char *src, size_t siz)
{
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;


  if (n != 0 && --n != 0)
    {
      do
	{
	  if ((*d++ = *s++) == 0)
	    break;
	}
      while (--n != 0);
    }


  if (n == 0)
    {
      if (siz != 0)
	*d = '\0';
      while (*s++)
	;
    }
  return (s - src - 1);
}


size_t
strlcat (char *dst, const char *src, size_t siz)
{
  register char *d = dst;
  register const char *s = src;
  register size_t n = siz;
  size_t dlen;


  while (n-- != 0 && *d != '\0')
    d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
    return (dlen + strlen (s));
  while (*s != '\0')
    {
      if (n != 1)
	{
	  *d++ = *s;
	  n--;
	}
      s++;
    }
  *d = '\0';
  return (dlen + (s - src));
}

#define PARSE_FORMAT    	    	0
#define PARSE_REPLACE    	    	1
#define PARSE_REPLACE_ALL   2
#define PARSE_HELP    	    	    3
#define PARSE_DELETE    	    	4
#define PARSE_INSERT    	    	5
#define PARSE_LIST_NORM    	    	6
#define PARSE_LIST_NUM    	    	7
#define PARSE_EDIT    	    	    8
#define PARSE_INDENT        9
#define PARSE_COLOR        10

const char *
del_last_line (const char *string)
{
  size_t len;
  bool found = false;

  char xbuf[MSL * 5];

  xbuf[0] = '\0';
  if (NullStr (string))
    return (str_dup (xbuf));

  strcpy (xbuf, string);

  for (len = strlen (xbuf); len > 0; len--)
    {
      if (xbuf[len] == '\r')
	{
	  if (!found)
	    {
	      if (len > 0)
		len--;
	      found = true;
	    }
	  else
	    {
	      xbuf[len + 1] = '\0';
	      free_string (string);
	      return (str_dup (xbuf));
	    }
	}
    }
  xbuf[0] = '\0';
  free_string (string);
  return (str_dup (xbuf));
}

const char *
string_nocolor (const char *string)
{
  const char *new_str = strip_color (string);

  free_string (string);
  return str_dup (new_str);
}

void
parse_action (const char **text, int command, const char *string,
	      CharData * ch)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

  switch (command)
    {
    case PARSE_HELP:
      chprintln (ch,
		 "-------------------------------------------------------");
      chprintlnf (ch,
		  "Edit help (commands on blank line):" NEWLINE
		  "%ch               - get help (this info)" NEWLINE
		  "%cs               - show string so far" NEWLINE
		  "%cS               - show string so far without line numbers"
		  NEWLINE "%cf               - formats text" NEWLINE
		  "%cc               - clear string so far" NEWLINE
		  "%cd#              - delete line number <num>" NEWLINE
		  "%cd               - delete last line" NEWLINE
		  "%cn#              - indent a string by <num>" NEWLINE
		  "%ci# <str>        - insert <str> on line <num>" NEWLINE
		  "%ce# <str>        - replace line <num> with <str>"
		  NEWLINE
		  "%cr 'a' 'b'       - replace first occurance of text"
		  NEWLINE
		  "%cR 'a' 'b'       - replace all occurances of text"
		  NEWLINE
		  "                   usage: %cr 'pattern' 'replacement'"
		  NEWLINE
		  "%c%c%c               - strip all color from text"
		  NEWLINE "%c| <command>     - execute a mud command"
		  NEWLINE "%cq               - end string", StrEdKey (ch),
		  StrEdKey (ch), StrEdKey (ch), StrEdKey (ch), StrEdKey (ch),
		  StrEdKey (ch), StrEdKey (ch), StrEdKey (ch), StrEdKey (ch),
		  StrEdKey (ch), StrEdKey (ch), StrEdKey (ch), StrEdKey (ch),
		  StrEdKey (ch), COLORCODE, COLORCODE, StrEdKey (ch),
		  StrEdKey (ch));
      chprintln (ch,
		 "------------------------------------------------------");
      break;
    case PARSE_FORMAT:
      *text = format_string (*text);
      chprintln (ch, "String formatted.");
      break;
    case PARSE_EDIT:
      string = first_arg (string, arg1, false);
      if (NullStr (arg1))
	{
	  chprintln (ch, "You must specify a line number.");
	  return;
	}
      *text = string_linedel (*text, atoi (arg1));
      *text = string_lineadd (*text, string, atoi (arg1));
      chprintln (ch, "Line replaced.");
      break;
    case PARSE_DELETE:
      if (NullStr (string))
	{
	  *text = del_last_line (*text);
	  chprintln (ch, "Last line deleted.");
	}
      else
	{
	  *text = string_linedel (*text, atoi (string));
	  chprintlnf (ch, "Line %d deleted.", atoi (string));
	}
      break;
    case PARSE_INDENT:
      if (NullStr (string))
	{
	  *text = indent_string (*text, 0);
	  chprintln (ch, "String un-indented.");
	}
      else
	{
	  *text = indent_string (*text, atoi (string));
	  chprintlnf (ch, "String indented by %d spaces.", atoi (string));
	}
      break;
    case PARSE_REPLACE:
      string = first_arg (string, arg1, false);
      string = first_arg (string, arg2, false);
      if (NullStr (arg1))
	{
	  chprintlnf (ch, "Usage: %cr 'old string' 'new string'",
		      StrEdKey (ch));
	  return;
	}
      *text = string_replace (*text, arg1, arg2);
      chprintlnf (ch, "'%s' replaced with '%s'.", arg1, arg2);
      break;
    case PARSE_REPLACE_ALL:
      string = first_arg (string, arg1, false);
      string = first_arg (string, arg2, false);
      if (NullStr (arg1))
	{
	  chprintlnf (ch, "Usage: %cR 'old string' 'new string'",
		      StrEdKey (ch));
	  return;
	}
      *text = string_replace_all (*text, arg1, arg2);
      chprintlnf (ch, "All occurances of '%s' replaced with '%s'.",
		  arg1, arg2);
      break;
    case PARSE_INSERT:
      string = first_arg (string, arg1, false);
      *text = string_lineadd (*text, string, atoi (arg1));
      chprintlnf (ch, "Line %d inserted.", atoi (arg1));
      break;
    case PARSE_COLOR:
      *text = string_nocolor (*text);
      chprintln (ch, "color string from string.");
      break;
    case PARSE_LIST_NORM:
      chprintln (ch, "String so far:");
      sendpage (ch, *text);
      break;
    case PARSE_LIST_NUM:
      chprintln (ch, "String so far:");
      sendpage (ch, numlines (*text));
      break;
    default:
      chprintln (ch, "Invalid command.");
      bug ("invalid command passed");
      break;
    }
}

strshow_t
parse_string_command (const char **text, const char *str, CharData * ch)
{
  int i = 2, j = 0;
  char actions[MAX_INPUT_LENGTH];

  if ((*str == StrEdKey (ch)))
    {
      while (str[i] != '\0')
	{
	  actions[j] = str[i];
	  i++;
	  j++;
	}
      actions[j] = '\0';
      *(char *) str = '\0';
      switch (str[1])
	{
	case 'q':
	  *(char *) str = '\0';
	  return STRING_END;
	case '|':
	  interpret (ch, actions);
	  chprintln (ch, "Command performed.");
	  return STRING_FOUND;
	case 'c':
	  chprintln (ch, "String cleared.");
	  replace_str (text, "");
	  return STRING_FOUND;
	case 's':
	  parse_action (text, PARSE_LIST_NUM, actions, ch);
	  return STRING_FOUND;
	case 'S':
	  parse_action (text, PARSE_LIST_NORM, actions, ch);
	  return STRING_FOUND;
	case 'r':
	  parse_action (text, PARSE_REPLACE, actions, ch);
	  return STRING_FOUND;
	case 'R':
	  parse_action (text, PARSE_REPLACE_ALL, actions, ch);
	  return STRING_FOUND;
	case 'f':
	  parse_action (text, PARSE_FORMAT, actions, ch);
	  return STRING_FOUND;
	case 'd':
	  parse_action (text, PARSE_DELETE, actions, ch);
	  return STRING_FOUND;
	case 'n':
	  parse_action (text, PARSE_INDENT, actions, ch);
	  return STRING_FOUND;
	case COLORCODE:
	  parse_action (text, PARSE_COLOR, actions, ch);
	  return STRING_FOUND;
	case 'i':
	  parse_action (text, PARSE_INSERT, actions, ch);
	  return STRING_FOUND;
	case 'e':
	  parse_action (text, PARSE_EDIT, actions, ch);
	  return STRING_FOUND;
	case 'h':
	  parse_action (text, PARSE_HELP, actions, ch);
	  return STRING_FOUND;
	default:
	  chprintln (ch, "Invalid command.");
	  return STRING_FOUND;
	}
    }
  else if (*str == '@' && *(str + 1) == '\0')
    {
      *(char *) str = '\0';
      return STRING_END;
    }
  return STRING_NONE;
}

void
string_add (CharData * ch, char *argument)
{
  char buf[MSL * 8];
  strshow_t action;

  action = parse_string_command (ch->desc->pString, argument, ch);

  switch (action)
    {
    case STRING_END:
      ch->desc->pString = NULL;
      chprintln (ch, "Done editing.");
      return;
    case STRING_FOUND:
      return;
    default:
    case STRING_NONE:
      strcpy (buf, *ch->desc->pString);


      if (strlen (buf) + strlen (argument) >= ((MSL * 8) - 1000))
	{
	  chprintln (ch, "String too long, last line skipped.");

	  ch->desc->pString = NULL;
	  return;
	}


      strcat (buf, argument);
      strcat (buf, NEWLINE);
      replace_str (ch->desc->pString, buf);
      return;
    }
}



const char *
format_string (const char *oldstring)
{
  char xbuf[MSL];
  char xbuf2[MSL];
  const char *rdesc;
  size_t i = 0, end_of_line;
  bool cap = true;
  bool bFormat = true;

  if (NullStr (oldstring))
    return &str_empty[0];

  xbuf[0] = xbuf2[0] = 0;

  for (rdesc = oldstring; *rdesc; rdesc++)
    {
      if (*rdesc != COLORCODE)
	{
	  if (bFormat)
	    {
	      if (*rdesc == '\n')
		{
		  if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' '
		      && *(rdesc + 3) == '\n' && xbuf[i - 1] != '\r')
		    {
		      xbuf[i] = '\n';
		      xbuf[i + 1] = '\r';
		      xbuf[i + 2] = '\n';
		      xbuf[i + 3] = '\r';
		      i += 4;
		      rdesc += 2;
		    }
		  else if (*(rdesc + 1) == '\r'
			   && *(rdesc + 2) == ' '
			   && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
		    {
		      xbuf[i] = '\n';
		      xbuf[i + 1] = '\r';
		      i += 2;
		    }
		  else if (*(rdesc + 1) == '\r'
			   && *(rdesc + 2) == '\n' && xbuf[i - 1] != '\r')
		    {
		      xbuf[i] = '\n';
		      xbuf[i + 1] = '\r';
		      xbuf[i + 2] = '\n';
		      xbuf[i + 3] = '\r';
		      i += 4;
		      rdesc += 1;
		    }
		  else if (*(rdesc + 1) == '\r'
			   && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
		    {
		      xbuf[i] = '\n';
		      xbuf[i + 1] = '\r';
		      i += 2;
		    }
		  else if (xbuf[i - 1] != ' ' && xbuf[i - 1] != '\r')
		    {
		      xbuf[i] = ' ';
		      i++;
		    }
		}
	      else if (*rdesc == '\r')
		;
	      else if (*rdesc == 'i' && *(rdesc + 1) == '.'
		       && *(rdesc + 2) == 'e' && *(rdesc + 3) == '.')
		{
		  xbuf[i] = 'i';
		  xbuf[i + 1] = '.';
		  xbuf[i + 2] = 'e';
		  xbuf[i + 3] = '.';
		  i += 4;
		  rdesc += 3;
		}
	      else if (*rdesc == ' ')
		{
		  if (xbuf[i - 1] != ' ')
		    {
		      xbuf[i] = ' ';
		      i++;
		    }
		}
	      else if (*rdesc == ')')
		{
		  if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
		      && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
			  || xbuf[i - 3] == '!'))
		    {
		      xbuf[i - 2] = *rdesc;
		      xbuf[i - 1] = ' ';
		      xbuf[i] = ' ';
		      i++;
		    }
		  else if (xbuf[i - 1] == ' '
			   && (xbuf[i - 2] == ',' || xbuf[i - 2] == ';'))
		    {
		      xbuf[i - 1] = *rdesc;
		      xbuf[i] = ' ';
		      i++;
		    }
		  else
		    {
		      xbuf[i] = *rdesc;
		      i++;
		    }
		}
	      else if (*rdesc == ',' || *rdesc == ';')
		{
		  if (xbuf[i - 1] == ' ')
		    {
		      xbuf[i - 1] = *rdesc;
		      xbuf[i] = ' ';
		      i++;
		    }
		  else
		    {
		      xbuf[i] = *rdesc;
		      if (*(rdesc + 1) != '\"')
			{
			  xbuf[i + 1] = ' ';
			  i += 2;
			}
		      else
			{
			  xbuf[i + 1] = '\"';
			  xbuf[i + 2] = ' ';
			  i += 3;
			  rdesc++;
			}
		    }

		}
	      else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
		{
		  if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
		      && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
			  || xbuf[i - 3] == '!'))
		    {
		      xbuf[i - 2] = *rdesc;
		      if (*(rdesc + 1) != '\"')
			{
			  xbuf[i - 1] = ' ';
			  xbuf[i] = ' ';
			  i++;
			}
		      else
			{
			  xbuf[i - 1] = '\"';
			  xbuf[i] = ' ';
			  xbuf[i + 1] = ' ';
			  i += 2;
			  rdesc++;
			}
		    }
		  else
		    {
		      xbuf[i] = *rdesc;
		      if (*(rdesc + 1) != '\"')
			{
			  xbuf[i + 1] = ' ';
			  xbuf[i + 2] = ' ';
			  i += 3;
			}
		      else
			{
			  xbuf[i + 1] = '\"';
			  xbuf[i + 2] = ' ';
			  xbuf[i + 3] = ' ';
			  i += 4;
			  rdesc++;
			}
		    }
		  cap = true;
		}
	      else
		{
		  xbuf[i] = *rdesc;
		  if (cap)
		    {
		      cap = false;
		      xbuf[i] = toupper (xbuf[i]);
		    }
		  i++;
		}
	    }
	  else
	    {
	      xbuf[i] = *rdesc;
	      i++;
	    }
	}
      else
	{
	  if (*(rdesc + 1) == 'Z')
	    bFormat = !bFormat;
	  xbuf[i] = *rdesc;
	  i++;
	  rdesc++;
	  xbuf[i] = *rdesc;
	  i++;
	}
    }
  xbuf[i] = 0;
  strncpy (xbuf2, xbuf, sizeof (xbuf2));

  rdesc = xbuf2;

  xbuf[0] = 0;

  for (;;)
    {
      end_of_line = 77;
      for (i = 0; i < end_of_line; i++)
	{
	  switch (*(rdesc + i))
	    {
	    case COLORCODE:
	      {
		int k = ansi_skip ((rdesc + i));

		end_of_line += k;
		i += k;
	      }
	      break;
	    case CUSTOMSTART:
	      do
		{
		  end_of_line++;
		  i++;
		}
	      while (*(rdesc + i) && *(rdesc + i) != CUSTOMEND);
	      break;
	    case MXP_BEGc:
	      do
		{
		  end_of_line++;
		  i++;
		}
	      while (*(rdesc + i) && *(rdesc + i) != MXP_ENDc);
	      break;
	    }

	  if (!*(rdesc + i))
	    break;

	  if (*(rdesc + i) == '\r')
	    end_of_line = i;
	}
      if (i < end_of_line)
	{
	  break;
	}
      if (*(rdesc + i - 1) != '\r')
	{
	  for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)
	    {
	      if (*(rdesc + i) == ' ')
		break;
	    }
	  if (i)
	    {
	      strnzncat (xbuf, rdesc, sizeof (xbuf), i);
	      strcat (xbuf, NEWLINE);
	      rdesc += i + 1;
	      while (*rdesc == ' ')
		rdesc++;
	    }
	  else
	    {
	      bug ("No spaces");
	      strnzncat (xbuf, rdesc, sizeof (xbuf), end_of_line - 2);
	      strcat (xbuf, "Ä" NEWLINE);
	      rdesc += end_of_line - 1;
	    }
	}
      else
	{
	  strnzncat (xbuf, rdesc, sizeof (xbuf), i - 1);
	  strcat (xbuf, "\r");
	  rdesc += i;
	  while (*rdesc == ' ')
	    rdesc++;
	}
    }
  while (*(rdesc + i)
	 && (*(rdesc + i) == ' ' || *(rdesc + i) == '\n'
	     || *(rdesc + i) == '\r'))
    i--;

  strnzncat (xbuf, rdesc, sizeof (xbuf), i + 1);

  if (!has_newline (xbuf))
    strcat (xbuf, NEWLINE);

  free_string (oldstring);
  return (str_dup (xbuf));
}

const char *
indent_string (const char *string, int indent)
{
  char buf[MSL * 5], tmp[MSL * 5];
  char out[MSL * 5];
  const char *tmpstr = string;

  out[0] = NUL;

  while (*tmpstr)
    {
      if (indent > 0)
	{
	  sprintf (buf, "%*s", indent, " ");
	  strcat (out, buf);
	}
      else
	{
	  while (*tmpstr && *tmpstr == ' ')
	    tmpstr++;

	  if (*tmpstr == NUL)
	    break;
	}
      tmpstr = get_line (tmpstr, tmp);
      strcat (out, tmp);
      strcat (out, NEWLINE);
    }

  free_string (string);
  return (str_dup (out));
}


bool skip_first_arg_spaces = true;


const char *
first_arg (const char *argument, char *arg_first, bool fCase)
{
  char cEnd;

  if (NullStr (argument))
    {
      arg_first[0] = '\0';
      return "";
    }
  while (skip_first_arg_spaces && *argument == ' ')
    argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"' || *argument == '%' ||
      *argument == '(')
    {
      if (*argument == '(')
	{
	  cEnd = ')';
	  argument++;
	}
      else
	cEnd = *argument++;
    }

  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      if (fCase)
	*arg_first = tolower (*argument);
      else
	*arg_first = *argument;
      arg_first++;
      argument++;
    }
  *arg_first = '\0';

  while (*argument == ' ')
    argument++;

  return argument;
}


const char *
string_unpad (const char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char *s;

  s = (char *) argument;

  while (*s == ' ')
    s++;

  strcpy (buf, s);
  s = buf;

  if (*s != '\0')
    {
      while (*s != '\0')
	s++;
      s--;

      while (*s == ' ')
	s--;
      s++;
      *s = '\0';
    }

  free_string (argument);
  return str_dup (buf);
}


const char *
string_proper (const char *argument)
{
  char *s;

  s = (char *) argument;

  while (*s != '\0')
    {
      if (*s != ' ')
	{
	  *s = toupper (*s);
	  while (*s != ' ' && *s != '\0')
	    s++;
	}
      else
	{
	  s++;
	}
    }

  return argument;
}

const char *
string_linedel (const char *string, size_t line)
{
  const char *strtmp = string;
  char buf[MAX_STRING_LENGTH];
  size_t cnt = 1, tmp = 0;

  buf[0] = '\0';

  for (; *strtmp != '\0'; strtmp++)
    {
      if (cnt != line)
	buf[tmp++] = *strtmp;

      if (*strtmp == '\n')
	{
	  if (*(strtmp + 1) == '\r')
	    {
	      if (cnt != line)
		buf[tmp++] = *(++strtmp);
	      else
		++strtmp;
	    }

	  cnt++;
	}
    }

  buf[tmp] = '\0';

  free_string (string);
  return str_dup (buf);
}

const char *
string_lineadd (const char *string, const char *newstr, size_t line)
{
  const char *strtmp = string;
  size_t cnt = 1, tmp = 0;
  bool done = false;
  char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  for (; *strtmp != '\0' || (!done && cnt == line); strtmp++)
    {
      if (cnt == line && !done)
	{
	  strcat (buf, newstr);
	  strcat (buf, NEWLINE);
	  tmp += strlen (newstr) + 2;
	  cnt++;
	  done = true;
	}

      buf[tmp++] = *strtmp;

      if (done && *strtmp == '\0')
	break;

      if (*strtmp == '\n')
	{
	  if (*(strtmp + 1) == '\r')
	    buf[tmp++] = *(++strtmp);

	  cnt++;
	}

      buf[tmp] = '\0';
    }

  free_string (string);
  return str_dup (buf);
}

const char *
get_line (const char *str, char *buf)
{
  size_t tmp = 0;
  bool found = false;

  while (*str)
    {
      if (*str == '\n')
	{
	  found = true;
	  break;
	}

      buf[tmp++] = *(str++);
    }

  if (found)
    {
      if (*(str + 1) == '\r')
	str += 2;
      else
	str += 1;
    }

  buf[tmp] = '\0';

  return str;
}

const char *
numlines (const char *string)
{
  int cnt = 1;
  static char buf[MAX_STRING_LENGTH * 2];
  char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

  buf[0] = '\0';

  while (*string)
    {
      string = get_line (string, tmpb);
      sprintf (buf2, "%2d. %s" NEWLINE, cnt++, tmpb);
      strcat (buf, buf2);
    }

  return buf;
}


const char *
replines (const char *string)
{
  static char buf[MSL * 2];
  char buf2[MSL], tmpb[MSL];

  buf[0] = NUL;

  while (*string)
    {
      string = get_line (string, tmpb);
      sprintf (buf2, "> %s" NEWLINE, tmpb);
      strcat (buf, buf2);
    }

  return buf;
}


const char *
hide_tilde (const char *str)
{
  static char buf_new[5][MSL];
  static int p;
  size_t i = 0, j;
  char *result;

  if (NullStr (str))
    return &str_empty[0];


  p++;
  p %= 5;
  result = buf_new[p];

  for (j = 0; str[j] != NUL; j++)
    {
      if (str[j] == '~')
	{
	  result[i++] = COLORCODE;
	  result[i++] = '-';
	}
      else
	{
	  result[i++] = str[j];
	}
    }
  result[i] = NUL;
  return (result);
}

const char *
strip_color (const char *str)
{
  static char buf_new[5][MSL];
  static int p;
  size_t i = 0, j;
  char *result;

  if (NullStr (str))
    return "";

  ++p, p %= 5;
  result = buf_new[p];

  for (j = 0; str[j] != NUL; j++)
    {
      if (str[j] == COLORCODE)
	{
	  j += ansi_skip (&str[j]);
	  if (str[j] == '-')
	    result[i++] = '~';
	}
      else if (str[j] == CUSTOMSTART)
	{
	  do
	    {
	      j++;
	    }
	  while (str[j] != NUL && str[j] != CUSTOMEND);
	}
      else
	{
	  result[i++] = str[j];
	}
    }
  result[i] = NUL;
  return (result);
}

const char *
show_tilde (const char *str)
{
  static char buf_new[5][MSL];
  static int p;
  size_t i = 0, j;
  char *result;

  if (NullStr (str))
    return &str_empty[0];


  p++;
  p %= 5;
  result = buf_new[p];

  for (j = 0; str[j] != NUL; j++)
    {
      if (str[j] == COLORCODE)
	{
	  if (str[j + 1] == '-')
	    {
	      result[i++] = '~';
	      j++;
	    }
	  else
	    {
	      result[i++] = str[j];
	    }
	}
      else
	{
	  result[i++] = str[j];
	}
    }
  result[i] = NUL;
  return (result);
}



char *
fill_cstr_len (const char *src, int len)
{
  int count = 0, sz, cz, c, pos, mod;
  static char buf_new[3][MSL];
  static int i;
  char *result;

  ++i, i %= 3;
  result = buf_new[i];
  result[0] = NUL;

  cz = cstrlen (src);
  mod = len % cz;
  len /= cz;

  sz = strlen (src);
  pos = 0;

  for (c = 0; c < len; c++)
    {
      strcat (result, src);
      pos += sz;
    }

  count = 0;

  for (c = 0; c < sz && count < mod; c++, pos++)
    {
      if (src[c] == COLORCODE)
	{
	  int k = ansi_skip (&src[c]);

	  while (k-- > 0)
	    result[pos++] = src[c++];
	  if (is_ansi_printed_char (result[pos]))
	    count++;
	}
      else if (src[c] == CUSTOMSTART)
	{
	  do
	    {
	      result[pos++] = src[c++];
	    }
	  while (src[c] != NUL && src[c] != CUSTOMEND);
	}
      else if (src[c] == MXP_BEGc)
	{
	  do
	    {
	      result[pos++] = src[c++];
	    }
	  while (src[c] != NUL && src[c] != MXP_ENDc);
	}
      else
	{
	  result[pos] = src[c];
	  count++;
	}
    }
  result[pos] = NUL;
  return buf_new[i];
}

const char *
stringf (CharData * ch, size_t length, align_t align,
	 char *fill, const char *format, ...)
{
  size_t nCount = 0, sz, cnt = 0;
  char *result, str[MPL];
  static char buf_new[5][MSL];
  static int i;

  ++i, i %= 5;
  result = buf_new[i];
  result[0] = NUL;

  if (NullStr (fill))
    fill = " ";

  if (length <= 0)
    length = get_scr_cols (ch);

  str[0] = NUL;
  if (!NullStr (format))
    {
      va_list args;

      va_start (args, format);
      vsnprintf (str, sizeof (str), format, args);
      va_end (args);
    }

  sz = cstrlen (str);
  nCount = Min (sz, length);

  if (align == Right)
    {
      cnt = (length - ++nCount);
      strcat (result, fill_cstr_len (fill, length - nCount));
    }

  if (align == Center)
    {
      nCount = (length - nCount) / 2;
      cnt = nCount;
      strcat (result, fill_cstr_len (fill, nCount));
    }

  strcat (result, str);
  cnt += sz;

  strcat (result, fill_cstr_len (fill, length - cnt));

  sz = strlen (result);

  if (result[sz - 1] == COLORCODE && result[sz] == NUL)
    strcat (result, "x");
  else
    strcat (result, "{x");

  return (result);
}

size_t
cstrlen (const char *cstr)
{
  size_t cnt = 0, i;

  if (NullStr (cstr))
    return 0;

  for (i = 0; cstr[i] != NUL; i++)
    {
      if (cstr[i] == COLORCODE)
	{
	  i += ansi_skip (&cstr[i]);
	  if (is_ansi_printed_char (cstr[i]))
	    cnt++;
	}
      else if (cstr[i] == CUSTOMSTART)
	{
	  do
	    {
	      i++;
	    }
	  while (cstr[i] != NUL && cstr[i] != CUSTOMEND);
	}
      else if (cstr[i] == MXP_BEGc)
	{
	  do
	    {
	      i++;
	    }
	  while (cstr[i] != NUL && cstr[i] != MXP_ENDc);
	}
      else
	{
	  cnt++;
	}
    }
  return cnt;
}

size_t
skipcol (const char *str)
{
  size_t i;

  if (NullStr (str))
    return 0;

  for (i = 0; str[i] != NUL; i++)
    {
      if (str[i] == COLORCODE)
	{
	  i += ansi_skip (&str[i]);
	}
      else if (str[i] == CUSTOMSTART)
	{
	  do
	    {
	      i++;
	    }
	  while (str[i] != NUL && str[i] != CUSTOMEND);
	}
      else if (str[i] == MXP_BEGc)
	{
	  do
	    {
	      i++;
	    }
	  while (str[i] != NUL && str[i] != MXP_ENDc);
	}
      else
	break;
    }
  return i;
}

size_t
skiprcol (const char *str)
{
  size_t i;

  if (NullStr (str))
    return 0;

  for (i = strlen (str) - 1; i > 0; i--)
    {
      if (str[i - 1] == COLORCODE)
	{
	  i -= ansi_skip (&str[i - 1]);
	  if (str[i - 1] == COLORCODE)
	    i--;
	}
      else if (str[i] == CUSTOMEND)
	{
	  do
	    {
	      i--;
	    }
	  while (str[i] != CUSTOMSTART);
	}
      else if (str[i] == MXP_ENDc)
	{
	  do
	    {
	      i--;
	    }
	  while (str[i] != MXP_BEGc);
	}
      else
	break;
    }
  return i + 1;
}

size_t
count_codes (const char *str, char code)
{
  size_t i, cnt = 0;

  if (NullStr (str))
    return 0;

  for (i = 0; str[i] != NUL; i++)
    {
      if (str[i] == code)
	{
	  cnt++;
	  switch (code)
	    {
	    case COLORCODE:
	      i += ansi_skip (&str[i]);
	      break;
	    case CUSTOMSTART:
	      do
		{
		  i++;
		}
	      while (str[i] != NUL && str[i] != CUSTOMEND);
	      break;
	    case MXP_BEGc:
	      do
		{
		  i++;
		}
	      while (str[i] != NUL && str[i] != MXP_ENDc);
	      break;
	    }
	}
    }
  return cnt;
}

char *
num_punc (long foo)
{
  size_t index, index_new, len;
  static char buf_new[5][MIL];
  static int i;
  char buf[MIL];
  char *result;

  ++i, i %= 5;
  result = buf_new[i];

  sprintf (buf, "%ld", foo);

  len = strlen (buf);

  for (index = index_new = 0; index < len; index++, index_new++)
    {
      if (index != 0 && (len - index) % 3 == 0)
	{
	  result[index_new] = ',';
	  index_new++;
	  result[index_new] = buf[index];
	}
      else
	result[index_new] = buf[index];
    }
  result[index_new] = NUL;
  return (result);
}



const char *
draw_line (CharData * ch, char *fill, size_t len)
{
  static char buf_new[5][MSL];
  static int i;
  char *result;

  ++i, i %= 5;
  result = buf_new[i];
  result[0] = '\0';

  if (NullStr (fill))
    fill = "-";

  if (len == 0)
    len = get_scr_cols (ch);

  strcat (result, fill_cstr_len (fill, len));

  if (result[strlen (result) - 1] == COLORCODE)
    strcat (result, "x");
  else
    strcat (result, "{x");

  return result;
}

Do_Fun (do_strkey)
{
  if (IsNPC (ch))
    return;

  if (NullStr (argument) || strlen (argument) > 1)
    {
      cmd_syntax (ch, NULL, n_fun, "<key>", NULL);
      chprintln (ch, "Where <key> can be any 1 letter you want ");
      chprintln (ch, "to use for string editor commands.");
      return;
    }

  if (!isascii (argument[0]) || argument[0] == ' ' || argument[0] == '\\')
    {
      chprintln (ch, "Invalid string editor key.");
      return;
    }

  ch->pcdata->str_ed_key = argument[0];
  chprintlnf (ch, "The string editor now uses %c for commands.",
	      StrEdKey (ch));
}

char *
FORMATF (const char *formatbuf, ...)
{
  static int i;
  static char buf[10][MSL * 3];
  va_list args;

  if (NullStr (formatbuf))
    return "";

  ++i, i %= 10;

  va_start (args, formatbuf);
  vsnprintf (buf[i], sizeof (buf[i]), formatbuf, args);
  va_end (args);

  return buf[i];
}


const char *
fix_name (const char *name)
{
  if (NullStr (name))
    return "";
  if ((name[0] == 'a' || name[0] == 'A') && name[1] == ' ')
    return name + 2;
  if ((name[0] == 't' || name[0] == 'T') && name[1] == 'h'
      && name[2] == 'e' && name[3] == ' ')
    return name + 4;

  return name;
}



const char *
str_rep (const char *orig, const char *old, const char *pnew)
{
  static char buf_new[5][STREP_SIZE];
  static int o;
  char *buf, rest[STREP_SIZE];
  const char *ptr;
  size_t i;

  if ((ptr = strstr (orig, old)) == NULL || !str_cmp (old, pnew) ||
      (i = strlen (orig) - strlen (ptr)) + strlen (pnew) >= (STREP_SIZE - 4))
    return orig;

  ++o, o %= 5;
  buf = buf_new[o];

  strcpy (buf, orig);
  do
    {
      strcpy (rest, &buf[i + strlen (old)]);
      buf[i] = NUL;
      strcat (buf, pnew);
      strcat (buf, rest);
    }
  while ((ptr = strstr (buf, old)) != NULL &&
	 !strstr (pnew, old) &&
	 (i =
	  strlen (buf) - strlen (ptr)) + strlen (pnew) < (STREP_SIZE - 4));

  return buf;
}



char *
stristr (const char *String, const char *Pattern)
{
  char *pptr, *sptr, *start;

  for (start = (char *) String; *start != NUL; start++)
    {

      for (; ((*start != NUL) && (toupper (*start) != toupper (*Pattern)));
	   start++)
	;
      if (NUL == *start)
	return NULL;

      pptr = (char *) Pattern;
      sptr = (char *) start;

      while (toupper (*sptr) == toupper (*pptr))
	{
	  sptr++;
	  pptr++;



	  if (NUL == *pptr)
	    return (start);
	}
    }
  return NULL;
}


const char *
stri_rep (const char *orig, const char *old, const char *pnew)
{
  static char buf_new[5][STREP_SIZE];
  static int o;
  char *buf, rest[STREP_SIZE];
  const char *ptr;
  size_t i;

  if ((ptr = strstr (orig, old)) == NULL || !str_cmp (old, pnew) ||
      (i = strlen (orig) - strlen (ptr)) + strlen (pnew) >= (STREP_SIZE - 4))
    return orig;

  ++o, o %= 5;
  buf = buf_new[o];

  strcpy (buf, orig);
  do
    {
      strcpy (rest, &buf[i + strlen (old)]);
      buf[i] = NUL;
      strcat (buf, pnew);
      strcat (buf, rest);
    }
  while ((ptr = strstr (buf, old)) != NULL &&
	 (i =
	  strlen (buf) - strlen (ptr)) + strlen (pnew) < (STREP_SIZE - 4));

  return buf;
}

bool
has_newline (const char *str)
{
  size_t i, j;

  if (NullStr (str))
    return false;

  i = skiprcol (str);
  j = strlen (NEWLINE);

  if (i < j)
    return false;

  return !str_ncmp (&str[i - j], NEWLINE, j);
}


char *
strip_cr (const char *str)
{
  static char buf_new[5][MSL];
  static int p;
  size_t i = 0, j;
  char *result;

  if (NullStr (str))
    return "";

  ++p, p %= 5;
  result = buf_new[p];

  for (j = 0; str[j] != NUL; j++)
    {
      if (str[j] == '\n' || str[j] == '\r')
	{
	  j++;
	}
      else
	{
	  result[i++] = str[j];
	}
    }
  result[i] = NUL;
  return (result);
}

char *
substr (const char *orig, const char *first, const char *last)
{
  char *a, *b, *result;
  static char buf[5][STREP_SIZE];
  static int i;
  int len;

  if (NullStr (orig))
    return &str_empty[0];

  ++i, i %= 5;
  result = buf[i];

  if (!(a = strstr (first, orig)))
    {
      strcpy (result, orig);
      return result;
    }

  if (!NullStr (last) && (b = strstr (orig, last)))
    len = strlen (a) - strlen (b);
  else
    len = strlen (a);

  strncpy (result, a + strlen (first), len);
  result[len] = NUL;

  return result;
}

char *
strcatf (char *prev, const char *next, ...)
{
  char buf[MPL];

  buf[0] = NUL;

  if (!NullStr (next))
    {
      va_list args;

      va_start (args, next);
      vsnprintf (buf, sizeof (buf), next, args);
      va_end (args);
    }
  return strcat (prev, buf);
}

char *
strncatf (char *prev, size_t len, const char *next, ...)
{
  char buf[MPL];

  buf[0] = NUL;

  if (!NullStr (next))
    {
      va_list args;

      va_start (args, next);
      vsnprintf (buf, sizeof (buf), next, args);
      va_end (args);
    }
  return strncat (prev, buf, len);
}
