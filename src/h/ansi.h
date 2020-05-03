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

#ifndef __ANSI_H_
#define __ANSI_H_    	1

#define    	COLORCODE   '{'
#define  CCSTR    	    	    	"{"

#define FG_BLACK    	    	30
#define FG_RED    	    	    	31
#define FG_GREEN    	    	32
#define FG_YELLOW    	    	33
#define FG_BLUE    	    	    	34
#define FG_MAGENTA    	    	35
#define FG_CYAN    	    	    	36
#define FG_WHITE    	    	37
#define FG_RANDOM       CL_RANDOM
#define FG_NONE         CL_NONE
#define VALID_FG(fg)    ((fg) >= FG_BLACK && (fg) <= FG_WHITE)

#define BG_BLACK    	    	40
#define BG_RED    	    	    	41
#define BG_GREEN    	    	42
#define BG_YELLOW    	    	43
#define BG_BLUE    	    	    	44
#define BG_MAGENTA    	    	45
#define BG_CYAN    	    	    	46
#define BG_WHITE    	    	47
#define BG_RANDOM       CL_RANDOM
#define BG_NONE         CL_NONE
#define VALID_BG(bg)    ((bg) >= BG_BLACK && (bg) <= BG_WHITE)

#define CL_MOD    	    	    	50

#define CL_NONE    	    	     0
#define CL_BRIGHT    	    	 1
#define CL_DIM    	    	    	 2
#define CL_STANDOUT      3
#define CL_UNDER    	    	 4
#define CL_BLINK    	    	 5
#define CL_ITALIC        6
#define CL_REVERSE    	    	 7
#define CL_HIDDEN    	    	 8
#define CL_RANDOM        -1
#define VALID_CL(cl)     ((cl) >= CL_NONE && (cl) <= CL_HIDDEN)

#define CT_ATTR    	    	    	0
#define CT_FORE    	    	    	1
#define CT_BACK    	    	    	2
#define CT_MAX    	    	    	3

#define ESC    	    	"\033"
#define ESCc    	'\033'

#define CL_DEFAULT    	    	ESC "[0m"

#define _DEFAULT    	0
#define _GOSSIP    	    1
#define _MUSIC    	    2
#define _QA    	        3
#define _QUOTE    	    4
#define _GRATS    	    5
#define _SHOUT1    	    6
#define _SHOUT2    	    7
#define _IMMTALK    	8
#define _TELLS1    	    9
#define _TELLS2    	    10
#define _SAY1       11
#define _SAY2       12
#define _SKILL    	    13
#define _YHIT    	    14
#define _OHIT    	    15
#define _VHIT    	    16
#define _WRACE    	    17
#define _WCLASS    	    18
#define _WLEVEL    	    19
#define _RTITLE    	    20
#define _SCORE1    	    21
#define _SCORE2    	    22
#define _SCORE3    	    23
#define _SCOREB    	    24
#define _WIZNET    	    25
#define _GTELL1     26
#define _GTELL2     27
#define _BTALK    	    28
#define _WSEX       29
#define MAX_CUSTOM_COLOR 30


#define CUSTOMSTART      '\x11'
#define CUSTOMEND    	    '\x12'
#define CSSTR        	    	"\x11"
#define CESTR    	    "\x12"


#define cstr(s) CSSTR #s CESTR
#define CTAG(s) cstr(s)





#define MXP_BEG "\x03"
#define MXP_END "\x04"
#define MXP_ENT "\x06"


#define MXP_BEGc '\x03'
#define MXP_ENDc '\x04'
#define MXP_ENTc '\x06'

#define HTML_LT   "&lt;"
#define HTML_GT   "&gt;"
#define HTML_QUOTE "&quote;"
#define HTML_AMP     "&amp;"

#define HTML_LTc     '<'
#define HTML_GTc     '>'
#define HTML_QUOTEc  '"'
#define HTML_AMPc    '&'

#define MXPMODE(arg) ESC "[" #arg "z"


#define MXPTAG(arg)     MXP_BEG arg MXP_END

#define LF   "\n"
#define CR   "\r"
#define NEWLINE     LF CR


#define CL_SEND_SOUND    	    	"AAA"
#define CL_SEND_IMAGE    	    	"AAB"
#define CL_SEND_REBOOT    	    	"AAC"
#define CL_SEND_MUSIC    	    	"AAD"
#define CL_SEND_UPTIME    	    	"AAF"
#define CL_SEND_AVI    	    	    "AAG"
#define CL_DOWNLOAD_MEDIA    	"AAH"
#define CL_SEND_SPECIAL    	    	"BAA"
#define CL_SEND_SPECIAL2    	"BAC"
#define CL_SEND_TELL    	    	"BAB"
#define CL_SEND_ROOM    	    	"BAD"
#define CL_SEND_MudLAG    	    	"BAE"
#define CL_SEND_EDIT    	    	"BAF"
#define CL_GP1_MASK    	    	    "BBA"
#define CL_GP2_MASK    	    	    "BBB"
#define CL_HP_MASK    	    	    "BBC"
#define CL_SP_MASK    	    	    "BBD"
#define CL_SEND_CAPTION    	    	"CAP"
#define CL_SEND_BEGIN_FILE    	"CDF"
#define CL_SEND_CONT_FILE    	"CCF"
#define CL_SEND_END_FILE    	"CEF"
#define CL_SEND_CHAT    	    	"CAA"
#define CL_SEND_ROOMCODE    "DDD"
#define CL_SEND_COMPOSITE    	"FFF"

#define  CL_SEND_HP    	    	    "A"
#define  CL_SEND_MAXHP    	    	"B"
#define  CL_SEND_SP    	    	    "C"
#define  CL_SEND_MAXSP    	    	"D"
#define  CL_SEND_GP1    	    	"E"
#define  CL_SEND_MAXGP1    	    	"F"
#define  CL_SEND_GP2    	    	"G"
#define  CL_SEND_MAXGP2    	    	"H"
#define  CL_SEND_GLINE1    	    	"I"
#define  CL_SEND_GLINE2    	    	"J"
#define  CL_SEND_ATTACKER    	"K"
#define  CL_SEND_ATTCOND    	"L"
#define  CL_SEND_ATTIMG    	    	"M"

#endif
