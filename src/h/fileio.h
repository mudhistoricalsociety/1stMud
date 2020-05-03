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

#ifndef __FILEIO_H_
#define __FILEIO_H_    	1





#define STFILEIO  1

#if defined DISABLE_MCCP && defined GZFILEIO
#undef GZFILEIO
#endif

#ifdef GZFILEIO

#define FileData    	    	gzFile
#define    	f_eof    	    	    	gzeof
#define f_getc    	    	    	gzgetc
#define f_ungetc(c, fp)    	    	gzseek(fp, -1, SEEK_CUR)
#define f_open    	    	    	(gzFile *)gzopen
#define f_close    	    	    	gzclose
#define f_read(s, z, e, fp)    	gzread(fp, s, z)
#define f_printf    	    	gzprintf
#define f_flush(fp)    	    	gzflush(fp, Z_SYNC_FLUSH)
#define f_tell    	    	    	gztell
#define f_seek    	    	    	gzseek

#elif defined STFILEIO

#define FileData    	    	stFile
#define    	f_eof    	    	    	steof
#define f_getc    	    	    	stgetc
#define f_ungetc    	    	stungetc
#define f_open    	    	    	(stFile *)stopen
#define f_close    	    	    	stclose
#define f_read    	    	    	stread
#define f_printf    	    	stprintf
#define f_flush    	    	    	stflush
#define f_tell(fp)    	    	(fp)->pos
#define f_seek    	    	    	stseek

#else

#define FileData    	    	FILE
#define    	f_eof    	    	    	feof
#define f_getc    	    	    	fgetc
#define f_ungetc    	    	ungetc
#define f_open    	    	    	(FILE *)fopen
#define f_close    	    	    	fclose
#define f_read    	    	    	fread
#define f_printf    	    	fprintf
#define f_flush    	    	    	fflush
#define f_tell    	    	    	ftell
#define f_seek    	    	    	fseek

#endif

typedef void Boot_F (FileData *);

#endif
