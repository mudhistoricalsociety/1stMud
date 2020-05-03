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

#ifndef __COMM_DESCRIPTOR_H_
#define __COMM_DESCRIPTOR_H_  1


int
d_print (Descriptor * d, const char *txt)
{
  int length;

  if (!d || NullStr (txt))
    return 0;

  length = strlen (txt);


  if (d->outtop == 0)
    {
      if (!d->fcommand)
	{
	  d->outbuf[d->outtop++] = '\n';
	  d->outbuf[d->outtop++] = '\r';
	}
      if (d->fPrompt)
	{
	  d->outbuf[d->outtop++] = '\n';
	  d->outbuf[d->outtop++] = '\r';
	  d->fPrompt = false;
	}
    }


  while (d->outtop + length >= d->outsize)
    {
      char *tmpbuf;

      if (d->outsize >= 32000)
	{
	  bug ("Buffer overflow. Closing.");
	  close_socket (d);
	  return -1;
	}
      d->outsize *= 2;
      alloc_mem (tmpbuf, char, d->outsize);
      strncpy (tmpbuf, d->outbuf, d->outtop);
      free_mem (d->outbuf);
      d->outbuf = tmpbuf;
    }


  strncpy (d->outbuf + d->outtop, txt, length);
  d->outtop += length;
  return length;
}

int
d_println (Descriptor * d, const char *txt)
{
  if (!d)
    return 0;

  return (d_print (d, txt) + d_print (d, NEWLINE));
}

int
d_printf (Descriptor * d, const char *txt, ...)
{
  va_list args;
  char buf[MPL];

  if (!d || NullStr (txt))
    return 0;

  va_start (args, txt);
  vsnprintf (buf, sizeof (buf), txt, args);
  va_end (args);

  return d_print (d, buf);
}

int
d_printlnf (Descriptor * d, const char *txt, ...)
{
  char buf[MPL];

  if (!d)
    return 0;

  buf[0] = NUL;
  if (!NullStr (txt))
    {
      va_list args;

      va_start (args, txt);
      vsnprintf (buf, sizeof (buf), txt, args);
      va_end (args);
    }
  return d_println (d, buf);
}

int
write_to_socket (SOCKET desc, const char *txt, int length)
{
  int iStart, nWrite = 0, nBlock;

  if (length <= 0)
    {
      length = strlen (txt);
    }

  for (iStart = 0; iStart < length; iStart += nWrite)
    {
      nBlock = Min (length - iStart, 4096);
      nWrite = send (desc, txt + iStart, nBlock, 0);
      if (nWrite < 0)
	{
#ifdef WIN32
	  if (WSAGetLastError () == WSAEWOULDBLOCK)
	    break;
#else
#ifndef EAGAIN
#define    	EAGAIN    	11
#endif
#ifndef ENOSR
#define    	ENOSR    	63
#endif

	  if (errno == EAGAIN || errno == ENOSR)
	    break;
#endif

	  socket_error ("write_to_socket()");
	  return -1;
	}
    }
  return iStart + Min (0, nWrite);
}


int
d_write (Descriptor * d, const char *txt, int length)
{
  if (!d)
    return 0;

  if (length <= 0)
    length = strlen (txt);

  d->bytes_normal += length;
#ifndef DISABLE_MCCP

  if (d->out_compress)
    {
      z_stream *s = d->out_compress;
      int bad_write_loop = 0, totalwritten = 0, written = 0, len;

      s->next_in = (unsigned char *) txt;
      s->avail_in = length;
      while (s->avail_in && bad_write_loop < 5)
	{
	  s->avail_out =
	    COMPRESS_BUF_SIZE - (s->next_out - d->out_compress_buf);

	  if (s->avail_out)
	    {
	      if (deflate (s, Z_SYNC_FLUSH) != Z_OK)
		{
		  logf ("d_write() - compression error.");
		  return -1;
		}
	    }

	  len = d->out_compress->next_out - d->out_compress_buf;
	  written =
	    write_to_socket (d->descriptor,
			     (char *) d->out_compress_buf, len);
	  if (written > 0)
	    {
	      d->bytes_compressed += written;
	      if (written < len)
		{
		  memmove (d->out_compress_buf,
			   d->out_compress_buf + written, len - written);
		}
	      d->out_compress->next_out = d->out_compress_buf + len - written;
	      totalwritten += written;
	    }
	  else
	    {
	      bad_write_loop++;
	    }
	}
      if (bad_write_loop == 5)
	{


	  if (totalwritten)
	    {

	      return totalwritten;
	    }
	  return -1;
	}

      return length;
    }
  else
#endif

    return write_to_socket (d->descriptor, txt, length);
}



#ifdef __cplusplus

int
Descriptor::wrap (const char *buf)
{
  Descriptor *d = this;
#else
int
dwrap (Descriptor * d, const char *buf)
{
#endif
  static char out[MSL * 5];
  int width;
  int pos;
  char *p;

  if (!d)
    return 0;

  width = ScrWidth (d);

  p = reformat_desc ((char *) buf);
  pos = 0;

  out[0] = NUL;

  do
    {
      pos = get_line_len (p, width);
      if (pos > 0)
	{
	  strncat (out, p, pos);
	  p += pos;
	  strcat (out, NEWLINE);
	}
      else
	{
	  strcat (out, p);
	  break;
	}
    }
  while (true);

  return d_print (d, out);
}

int
dwrapln (Descriptor * d, const char *buf)
{
  if (!d)
    return 0;

  return (dwrap (d, buf) + d_print (d, NEWLINE));
}

int
dwrapf (Descriptor * d, const char *buf, ...)
{
  va_list args;
  char format[MPL];

  if (!d || NullStr (buf))
    return 0;

  va_start (args, buf);
  vsnprintf (format, sizeof (format), buf, args);
  va_end (args);

  return dwrap (d, format);
}

int
dwraplnf (Descriptor * d, const char *buf, ...)
{
  char format[MPL];

  if (!d)
    return 0;

  format[0] = NUL;
  if (!NullStr (buf))
    {
      va_list args;

      va_start (args, buf);
      vsnprintf (format, sizeof (format), buf, args);
      va_end (args);
    }
  return d_println (d, format);
}
#endif
