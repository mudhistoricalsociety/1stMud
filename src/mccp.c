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
* mccp.c - support functions for mccp (the Mud Client Compression         *
* Protocol).  See http://homepages.ihug.co.nz/~icecube/compress/ and      *
* README.Rom24-mccp.                                                      *
* Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.                 *
* This code may be freely distributed and used if this copyright notice   *
* is retained intact.                                                     *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "telnet.h"

#ifndef DISABLE_MCCP

char compress2_start[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, NUL };
char compress1_start[] = { IAC, SB, TELOPT_COMPRESS, IAC, SE, NUL };

#if defined __FreeBSD__ && !defined ENOSR
#define ENOSR 63
#endif



void *
zlib_alloc (void *opaque, unsigned int items, unsigned int size)
{
  return calloc (items, size);
}

void
zlib_free (void *opaque, void *address)
{
  free (address);
}


bool
compressStart (Descriptor * desc, int version)
{
  z_stream *s;

  if (desc->out_compress)
    return true;


  alloc_mem (s, z_stream, 1);
  alloc_mem (desc->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE);

  s->next_in = NULL;
  s->avail_in = 0;

  s->next_out = desc->out_compress_buf;
  s->avail_out = COMPRESS_BUF_SIZE;

  s->zalloc = zlib_alloc;
  s->zfree = zlib_free;
  s->opaque = NULL;

  if (deflateInit (s, 9) != Z_OK)
    {

      free_mem (desc->out_compress_buf);
      free_mem (s);
      return false;
    }

  switch (version)
    {
    default:
    case 1:
      desc->mccp_version = 1;
      d_write (desc, compress1_start, 0);
      break;
    case 2:
      desc->mccp_version = 2;
      d_write (desc, compress2_start, 0);
      break;
    }

  desc->out_compress = s;
  return true;
}


bool
compressEnd (Descriptor * desc, int version)
{
  unsigned char dummy[1];

  if (!desc->out_compress)
    return true;

  if (desc->mccp_version != version)
    return true;

  desc->out_compress->avail_in = 0;
  desc->out_compress->next_in = dummy;



  if (deflate (desc->out_compress, Z_FINISH) != Z_STREAM_END)
    return false;

  write_to_socket (desc->descriptor, (char *) desc->out_compress_buf,
		   desc->out_compress->next_out - desc->out_compress_buf);

  deflateEnd (desc->out_compress);
  free_mem (desc->out_compress_buf);
  free_mem (desc->out_compress);
  desc->out_compress = NULL;
  desc->out_compress_buf = NULL;

  return true;
}

bool
compressContinue (Descriptor * desc)
{


















  z_stream *s;


  alloc_mem (s, z_stream, 1);
  alloc_mem (desc->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE);

  s->next_in = NULL;
  s->avail_in = 0;
  s->next_out = desc->out_compress_buf;
  s->avail_out = COMPRESS_BUF_SIZE;

  s->zalloc = zlib_alloc;
  s->zfree = zlib_free;
  s->opaque = NULL;

  if (deflateInit (s, 9) != Z_OK)
    {

      free_mem (desc->out_compress_buf);
      free_mem (s);
      return false;
    }



  s->next_in = (unsigned char *) " ";
  s->avail_in = 1;
  deflate (s, Z_FULL_FLUSH);
  s->next_out = desc->out_compress_buf;


  desc->out_compress = s;
  return true;
}

#endif


Do_Fun (do_compress)
{
#ifdef DISABLE_MCCP
  chprintln (ch, "Mud Compression is disabled.");
#else

  if (!ch->desc)
    {
      chprint (ch, "What descriptor?!\n");
      return;
    }

  if (NullStr (argument))
    {
      if (ch->desc->out_compress)
	{
	  double bcomp = ch->desc->bytes_compressed;
	  double bnorm = ch->desc->bytes_normal;

	  chprintlnf (ch, "Bytes Normal    : %d", ch->desc->bytes_normal);
	  chprintlnf (ch, "Bytes Compressed: %d", ch->desc->bytes_compressed);
	  chprintlnf (ch, "MCCP version    : %d", ch->desc->mccp_version);
	  chprintlnf (ch, "Compression Rate: %2.2f%%",
		      100.00 - Percent (bcomp, bnorm));
	}
      else
	{
	  chprintlnf (ch, "Bytes Normal    : %d", ch->desc->bytes_normal);
	  chprintln (ch, "You are not compressing data.");
	}
      cmd_syntax (ch, NULL, n_fun, "on/off", NULL);
      return;
    }

  if (!str_cmp (argument, "on"))
    {
      if (!ch->desc->out_compress)
	{
	  chprintln (ch,
		     "NOTE: If you did not have compression running after logging"
		     NEWLINE
		     "      in, chances are your client does not support it and you"
		     NEWLINE "      will recieve strange output...");
	  if (!compressStart (ch->desc, ch->desc->mccp_version))
	    {
	      chprint (ch, "Failed.\n");
	      return;
	    }
	  chprintln (ch, "Ok, compression enabled.");
	}
      else
	chprintln (ch, "You already have compression enabled.");
      return;
    }
  if (!str_cmp (argument, "off"))
    {
      if (!compressEnd (ch->desc, ch->desc->mccp_version))
	{
	  chprint (ch, "Failed.\n");
	  return;
	}

      chprint (ch, "Ok, compression disabled.\n");
      return;
    }
#endif
}
