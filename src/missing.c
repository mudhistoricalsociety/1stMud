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
* Copyright (C) 1991, 92, 93, 96, 97, 98, 99 Free Software Foundation,    *
* Inc.  Parts of this file are from the GNU C Library.                    *
* This library is free software; you can redistribute it and/or modify it *
* under the terms of the GNU Library General Public License as published  *
* by the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                     *
* This library is distributed in the hope that it will be useful, but     *
* WITHOUT ANY WARRANTY; without even the implied warranty of              * 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
* Library General Public License for more details.                        *
* You should have received a copy of the GNU Library General Public       *
* License along with this library; see the file COPYING.LIB.  If not,     *
* write to the Free Software Foundation, Inc., 59 Temple Place - Suite    *
* 330, Boston, MA 02111-1307, USA.                                        *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"

#ifndef HAVE_FNMATCH

#define FNM_PATHNAME    (1 << 0)
#define FNM_NOESCAPE    (1 << 1)
#define FNM_PERIOD      (1 << 2)

#if !defined _POSIX_C_SOURCE || _POSIX_C_SOURCE < 2 || defined _GNU_SOURCE
# define FNM_FILE_NAME   FNM_PATHNAME
# define FNM_LEADING_DIR (1 << 3)
# define FNM_CASEFOLD    (1 << 4)
#endif


#define FNM_NOMATCH     1

# if defined STDC_HEADERS || !defined isascii
#  define ISASCII(c) 1
# else
#  define ISASCII(c) isascii(c)
# endif

#ifdef isblank
# define ISBLANK(c) (ISASCII (c) && isblank (c))
#else
# define ISBLANK(c) ((c) == ' ' || (c) == '\t')
#endif
#ifdef isgraph
# define ISGRAPH(c) (ISASCII (c) && isgraph (c))
#else
# define ISGRAPH(c) (ISASCII (c) && isprint (c) && !isspace (c))
#endif

#define ISPRINT(c) (ISASCII (c) && isprint (c))
#define ISDIGIT(c) (ISASCII (c) && isdigit (c))
#define ISALNUM(c) (ISASCII (c) && isalnum (c))
#define ISALPHA(c) (ISASCII (c) && isalpha (c))
#define ISCNTRL(c) (ISASCII (c) && iscntrl (c))
#define ISLOWER(c) (ISASCII (c) && islower (c))
#define ISPUNCT(c) (ISASCII (c) && ispunct (c))
#define ISSPACE(c) (ISASCII (c) && isspace (c))
#define ISUPPER(c) (ISASCII (c) && isupper (c))
#define ISXDIGIT(c) (ISASCII (c) && isxdigit (c))

# if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H

#  if defined CHARCLASS_NAME_MAX
#   define CHAR_CLASS_MAX_LENGTH CHARCLASS_NAME_MAX
#  else

#   define CHAR_CLASS_MAX_LENGTH 256
#  endif

# define IS_CHAR_CLASS(string) wctype (string)
# else
#  define CHAR_CLASS_MAX_LENGTH  6

#  define IS_CHAR_CLASS(string)    	    	    	    	    	    	      \
(!str_cmp (string, "alpha") || !str_cmp (string, "upper")    	    	    	      \
|| !str_cmp (string, "lower") || !str_cmp (string, "digit")    	    	      \
|| !str_cmp (string, "alnum") || !str_cmp (string, "xdigit")    	    	      \
|| !str_cmp (string, "space") || !str_cmp (string, "print")    	    	      \
|| !str_cmp (string, "punct") || !str_cmp (string, "graph")    	    	      \
|| !str_cmp (string, "cntrl") || !str_cmp (string, "blank"))
# endif

static int
internal_fnmatch (const char *pattern, const char *string,
		  int no_leading_period, int flags)
{
  register const char *p = pattern, *n = string;
  register unsigned char c;


#  define FOLD(c) ((flags & FNM_CASEFOLD) && ISUPPER (c) ? tolower (c) : (c))

  while ((c = *p++) != '\0')
    {
      c = FOLD (c);

      switch (c)
	{
	case '?':
	  if (*n == '\0')
	    return FNM_NOMATCH;
	  else if (*n == '/' && (flags & FNM_FILE_NAME))
	    return FNM_NOMATCH;
	  else if (*n == '.' && no_leading_period
		   && (n == string
		       || (n[-1] == '/' && (flags & FNM_FILE_NAME))))
	    return FNM_NOMATCH;
	  break;

	case '\\':
	  if (!(flags & FNM_NOESCAPE))
	    {
	      c = *p++;
	      if (c == '\0')

		return FNM_NOMATCH;
	      c = FOLD (c);
	    }
	  if (FOLD ((unsigned char) *n) != c)
	    return FNM_NOMATCH;
	  break;

	case '*':
	  if (*n == '.' && no_leading_period
	      && (n == string || (n[-1] == '/' && (flags & FNM_FILE_NAME))))
	    return FNM_NOMATCH;

	  for (c = *p++; c == '?' || c == '*'; c = *p++)
	    {
	      if (*n == '/' && (flags & FNM_FILE_NAME))

		return FNM_NOMATCH;
	      else if (c == '?')
		{

		  if (*n == '\0')

		    return FNM_NOMATCH;
		  else

		    ++n;
		}
	    }

	  if (c == '\0')

	    return ((flags & FNM_FILE_NAME) && strchr (n, '/') != NULL
		    ? FNM_NOMATCH : 0);
	  else
	    {
	      const char *endp;


	      if (!(flags & FNM_FILE_NAME)
		  || ((endp = strchr (n, '/')) == NULL))
		endp = n + strlen (n);

	      if (c == '[')
		{
		  int flags2 = ((flags & FNM_FILE_NAME)
				? flags : (flags & ~FNM_PERIOD));

		  for (--p; n < endp; ++n)
		    if (internal_fnmatch (p, n,
					  (no_leading_period
					   && (n == string
					       || (n[-1] == '/'
						   && (flags
						       &
						       FNM_FILE_NAME)))),
					  flags2) == 0)
		      return 0;
		}
	      else if (c == '/' && (flags & FNM_FILE_NAME))
		{
		  while (*n != '\0' && *n != '/')
		    ++n;
		  if (*n == '/'
		      && (internal_fnmatch (p, n + 1, flags & FNM_PERIOD,
					    flags) == 0))
		    return 0;
		}
	      else
		{
		  int flags2 = ((flags & FNM_FILE_NAME)
				? flags : (flags & ~FNM_PERIOD));

		  if (c == '\\' && !(flags & FNM_NOESCAPE))
		    c = *p;
		  c = FOLD (c);
		  for (--p; n < endp; ++n)
		    if (FOLD ((unsigned char) *n) == c
			&& (internal_fnmatch (p, n,
					      (no_leading_period
					       && (n == string
						   || (n[-1] == '/'
						       && (flags
							   &
							   FNM_FILE_NAME)))),
					      flags2) == 0))
		      return 0;
		}
	    }


	  return FNM_NOMATCH;

	case '[':
	  {

	    static int posixly_correct;
	    register int except;
	    char cold;

	    if (posixly_correct == 0)
	      posixly_correct = getenv ("POSIXLY_CORRECT") != NULL ? 1 : -1;

	    if (*n == '\0')
	      return FNM_NOMATCH;

	    if (*n == '.' && no_leading_period && (n == string
						   || (n[-1] == '/'
						       && (flags
							   & FNM_FILE_NAME))))
	      return FNM_NOMATCH;

	    if (*n == '/' && (flags & FNM_FILE_NAME))

	      return FNM_NOMATCH;

	    except = (*p == '!' || (posixly_correct < 0 && *p == '^'));
	    if (except)
	      ++p;

	    c = *p++;
	    for (;;)
	      {
		unsigned char fn = FOLD ((unsigned char) *n);

		if (!(flags & FNM_NOESCAPE) && c == '\\')
		  {
		    if (*p == '\0')
		      return FNM_NOMATCH;
		    c = FOLD ((unsigned char) *p);
		    ++p;

		    if (c == fn)
		      goto matched;
		  }
		else if (c == '[' && *p == ':')
		  {

		    char str[CHAR_CLASS_MAX_LENGTH + 1];
		    size_t c1 = 0;

# if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H

		    wctype_t wt;
# endif

		    const char *startp = p;

		    for (;;)
		      {
			if (c1 == CHAR_CLASS_MAX_LENGTH)

			  return FNM_NOMATCH;

			c = *++p;
			if (c == ':' && p[1] == ']')
			  {
			    p += 2;
			    break;
			  }
			if (c < 'a' || c >= 'z')
			  {

			    p = startp;
			    c = '[';
			    goto normal_bracket;
			  }
			str[c1++] = c;
		      }
		    str[c1] = '\0';

# if defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H

		    wt = IS_CHAR_CLASS (str);
		    if (wt == 0)

		      return FNM_NOMATCH;

		    if (iswctype (btowc ((unsigned char) *n), wt))
		      goto matched;
# else

		    if ((!str_cmp (str, "alnum")
			 && ISALNUM ((unsigned char) *n))
			|| (!str_cmp (str, "alpha")
			    && ISALPHA ((unsigned char) *n))
			|| (!str_cmp (str, "blank")
			    && ISBLANK ((unsigned char) *n))
			|| (!str_cmp (str, "cntrl")
			    && ISCNTRL ((unsigned char) *n))
			|| (!str_cmp (str, "digit")
			    && ISDIGIT ((unsigned char) *n))
			|| (!str_cmp (str, "graph")
			    && ISGRAPH ((unsigned char) *n))
			|| (!str_cmp (str, "lower")
			    && ISLOWER ((unsigned char) *n))
			|| (!str_cmp (str, "print")
			    && ISPRINT ((unsigned char) *n))
			|| (!str_cmp (str, "punct")
			    && ISPUNCT ((unsigned char) *n))
			|| (!str_cmp (str, "space")
			    && ISSPACE ((unsigned char) *n))
			|| (!str_cmp (str, "upper")
			    && ISUPPER ((unsigned char) *n))
			|| (!str_cmp (str, "xdigit")
			    && ISXDIGIT ((unsigned char) *n)))
		      goto matched;
# endif

		  }
		else if (c == '\0')

		  return FNM_NOMATCH;
		else
		  {
		  normal_bracket:
		    if (FOLD (c) == fn)
		      goto matched;

		    cold = c;
		    c = *p++;

		    if (c == '-' && *p != ']')
		      {

			unsigned char cend = *p++;

			if (!(flags & FNM_NOESCAPE) && cend == '\\')
			  cend = *p++;
			if (cend == '\0')
			  return FNM_NOMATCH;

			if (cold <= fn && fn <= FOLD (cend))
			  goto matched;

			c = *p++;
		      }
		  }

		if (c == ']')
		  break;
	      }

	    if (!except)
	      return FNM_NOMATCH;
	    break;

	  matched:

	    while (c != ']')
	      {
		if (c == '\0')

		  return FNM_NOMATCH;

		c = *p++;
		if (!(flags & FNM_NOESCAPE) && c == '\\')
		  {
		    if (*p == '\0')
		      return FNM_NOMATCH;

		    ++p;
		  }
		else if (c == '[' && *p == ':')
		  {
		    do
		      if (*++p == '\0')
			return FNM_NOMATCH;
		    while (*p != ':' || p[1] == ']')
		    ;
		    p += 2;
		    c = *p;
		  }
	      }
	    if (except)
	      return FNM_NOMATCH;
	  }
	  break;

	default:
	  if (c != FOLD ((unsigned char) *n))
	    return FNM_NOMATCH;
	}

      ++n;
    }

  if (*n == '\0')
    return 0;

  if ((flags & FNM_LEADING_DIR) && *n == '/')

    return 0;

  return FNM_NOMATCH;

# undef FOLD
}

int
fnmatch (const char *pattern, const char *string, int flags)
{
  return internal_fnmatch (pattern, string, flags & FNM_PERIOD, flags);
}

#else
void
fnmatch_dummy (void)
{
}
#endif

#ifndef HAVE_SCANDIR

int
scandir (const char *dirname, struct dirent ***namelist,
	 int (*select) (const struct dirent *),
	 int (*dcomp) (const struct dirent **, const struct dirent **))
{
#ifdef WIN32
  int len;
  char *findIn, *d;
  WIN32_FIND_DATA find;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;

  len = strlen (dirname);

  alloc_mem (findIn, char, len + 5);

  strcpy (findIn, dirname);

  for (d = findIn; *d; d++)
    if (*d == '/')
      *d = '\\';

  if ((len == 0))
    strcpy (findIn, ".\\*");

  if ((len == 1) && (d[-1] == '.'))
    strcpy (findIn, ".\\*");

  if ((len > 0) && (d[-1] == '\\'))
    {
      *d++ = '*';
      *d = 0;
    }

  if ((len > 1) && (d[-1] == '.') && (d[-2] == '\\'))
    d[-1] = '*';

  if ((h = FindFirstFile (findIn, &find)) == INVALID_HANDLE_VALUE)
    {
      LPVOID lpMsgBuf;

      ret = GetLastError ();

      if (ret != ERROR_NO_MORE_FILES)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			 FORMAT_MESSAGE_FROM_SYSTEM, NULL, ret, 0,
			 (LPTSTR) & lpMsgBuf, 0, NULL);

	  MessageBox (NULL, (LPTSTR) lpMsgBuf, "GetLastError",
		      MB_OK | MB_ICONINFORMATION);


	  LocalFree (lpMsgBuf);
	}

      *namelist = dir;
      return nDir;
    }

  do
    {
      alloc_mem (selectDir, struct dirent, strlen (find.cFileName));

      strcpy (selectDir->d_name, find.cFileName);

      if (!select || (*select) (selectDir))
	{
	  if (nDir == NDir)
	    {
	      struct dirent **tempDir;

	      alloc_mem (tempDir, struct dirent *, NDir + 33);

	      if (NDir)
		memcpy (tempDir, dir, sizeof (struct dirent *) * NDir);

	      if (dir)
		free_mem (dir);

	      dir = tempDir;
	      NDir += 32;
	    }

	  dir[nDir++] = selectDir;
	  dir[nDir] = 0;
	}
      else
	free_mem (selectDir);
    }
  while (FindNextFile (h, &find));

  ret = GetLastError ();

  if (ret != ERROR_NO_MORE_FILES)
    {
      LPVOID lpMsgBuf;

      ret = GetLastError ();

      if (ret != ERROR_NO_MORE_FILES)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			 FORMAT_MESSAGE_FROM_SYSTEM, NULL, ret, 0,
			 (LPTSTR) & lpMsgBuf, 0, NULL);

	  MessageBox (NULL, (LPTSTR) lpMsgBuf, "GetLastError",
		      MB_OK | MB_ICONINFORMATION);


	  LocalFree (lpMsgBuf);
	}
    }

  FindClose (h);
  free_mem (findIn);

  if (dcomp)
    qsort (dir, nDir, sizeof (*dir),
	   (int (*)(const void *, const void *)) dcomp);

  *namelist = dir;
  return nDir;
#else

  register struct dirent *d, *p, **names;
  register int nitems;
  struct stat stb;
  long arraysz;
  DIR *dirp;

  if ((dirp = opendir (dirname)) == NULL)
    return (-1);
  if (fstat (dirp->d_fd, &stb) < 0)
    return (-1);


  arraysz = (stb.st_size / 24);
  names = (struct dirent **) malloc (arraysz * sizeof (struct dirent *));

  if (names == NULL)
    return (-1);

  nitems = 0;
  while ((d = readdir (dirp)) != NULL)
    {
      if (select != NULL && !(*select) (d))
	continue;

      p = (struct dirent *) malloc (d->d_reclen);
      if (p == NULL)
	return (-1);
      p->d_ino = d->d_ino;
      p->d_reclen = d->d_reclen;
      p->d_off = d->d_off;
      strcpy (p->d_name, d->d_name);

      if (++nitems >= arraysz)
	{
	  if (fstat (dirp->d_fd, &stb) < 0)
	    return (-1);
	  arraysz = stb.st_size / 12;
	  names = (struct dirent **) realloc ((char *) names,
					      arraysz *
					      sizeof (struct dirent *));
	  if (names == NULL)
	    return (-1);
	}
      names[nitems - 1] = p;
    }
  closedir (dirp);
  if (nitems && dcomp != NULL)
    qsort (names, nitems, sizeof (struct dirent *),
	   (int (*)(const void *, const void *)) dcomp);

  *namelist = names;
  return (nitems);
#endif
}


int
alphasort (const struct dirent **d1, const struct dirent **d2)
{
  return (strcmp ((*d1)->d_name, (*d2)->d_name));
}

#endif


#ifndef HAVE_INET_ATON
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

int
inet_aton (const char *cp, struct in_addr *addr)
{
  addr->s_addr = inet_addr (cp);
  return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}
#endif

#ifndef HAVE_STRREV

char *
strrev (char *str)
{
  char *p1, *p2;

  if (!str || !*str)
    return str;
  for (p1 = str, p2 = str + strlen (str) - 1; p2 > p1; ++p1, --p2)
    {
      *p1 ^= *p2;
      *p2 ^= *p1;
      *p1 ^= *p2;
    }
  return str;
}

#endif
