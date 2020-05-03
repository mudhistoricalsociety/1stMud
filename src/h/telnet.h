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
* Copyright (c) 1983, 1993                                                *
*      The Regents of the University of California.  All rights reserved. *
*                                                                         *
* Redistribution and use in source and binary forms, with or without      *
* modification, are permitted provided that the following conditions      *
* are met:                                                                *
* 1. Redistributions of source code must retain the above copyright       *
*    notice, this list of conditions and the following disclaimer.        *
* 2. Redistributions in binary form must reproduce the above copyright    *
*    notice, this list of conditions and the following disclaimer in the  *
*    documentation and/or other materials provided with the distribution. *
* 3. All advertising materials mentioning features or use of this software*
*    must display the following acknowledgement:                          *
*      This product includes software developed by the University of      *
*      California, Berkeley and its contributors.                         *
* 4. Neither the name of the University nor the names of its contributors *
*    may be used to endorse or promote products derived from this         *
*    software without specific prior written permission.                  *
*                                                                         *
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND *
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR      *
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS  *
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR  *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF    *
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS*
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN *
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) *
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF  *
* THE POSSIBILITY OF SUCH DAMAGE.                                         *
*                                                                         *
*      @(#)telnet.h            8.2 (Berkeley) 12/15/93                    *
***************************************************************************
* Updated on 19/03/1999 to support current state of Telnet Internet-Drafts*
* by Jeffrey Altman (The Kermit Project at Columbia University)           *
* jaltman@columbia.edu                                                    *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#ifndef __TELNET_H_
#define    	__TELNET_H_

Proto (void init_telnet, (Descriptor *));
Proto (void process_telnet, (Descriptor *, int, unsigned char *));
Proto (void set_desc_flags, (Descriptor *));

EXTERN const char echo_off_str[];
EXTERN const char echo_on_str[];
EXTERN const char echo_dont[];
EXTERN const char echo_do[];


EXTERN const char eor_do[];
EXTERN const char eor_will[];
EXTERN const char eor_dont[];
EXTERN const char eor_wont[];


EXTERN const char naws_will[];
EXTERN const char naws_dont[];
EXTERN const char naws_do[];
EXTERN const char naws_wont[];
EXTERN const char naws_sb[];


EXTERN const char iac_ip[];
EXTERN const char iac_susp[];
EXTERN const char iac_brk[];


EXTERN const char iac_se[];


EXTERN const char go_ahead_str[];
EXTERN const char will_suppress_ga_str[];
EXTERN const char wont_suppress_ga_str[];

#ifndef DISABLE_MCCP

EXTERN const char compress1_will[];
EXTERN const char compress1_do[];
EXTERN const char compress1_dont[];
EXTERN const char compress1_wont[];

EXTERN const char compress2_will[];
EXTERN const char compress2_do[];
EXTERN const char compress2_dont[];
EXTERN const char compress2_wont[];
#endif

EXTERN const char tspd_will[];
EXTERN const char tspd_do[];
EXTERN const char tspd_wont[];
EXTERN const char tspd_dont[];

EXTERN const char msp_will[];
EXTERN const char msp_do[];
EXTERN const char msp_dont[];
EXTERN const char msp_wont[];

EXTERN const char mxp_will[];
EXTERN const char mxp_do[];
EXTERN const char mxp_dont[];
EXTERN const char mxp_wont[];
EXTERN const char start_mxp_str[];

EXTERN const char ttype_do[];
EXTERN const char ttype_dont[];
EXTERN const char ttype_sb[];
EXTERN const char ttype_send[];
EXTERN const char ttype_will[];
EXTERN const char ttype_wont[];

EXTERN const char binary_do[];
EXTERN const char binary_dont[];
EXTERN const char binary_will[];
EXTERN const char binary_wont[];


#define    	IAC    	255
#define    	DONT    	254
#define    	DO    	253
#define    	WONT    	252
#define    	WILL    	251
#define    	SB    	250
#define    	GA    	249
#define    	EL    	248
#define    	EC    	247
#define    	AYT    	246
#define    	AO    	245
#define    	IP    	244
#define    	BREAK    	243
#define    	DM    	242
#define    	NOP    	241
#define    	SE    	240
#define EOR     239
#define    	ABORT    	238
#define    	SUSP    	237
#define    	xEOF    	236

#define SYNCH    	242

#define SEND    	1
#define IS    	0

#ifdef TELCMDS
char *telcmds[] = { "EOF", "SUSP", "ABORT", "EOR", "SE", "NOP", "DMARK",
  "BRK", "IP", "AO", "AYT", "EC", "EL", "GA", "SB", "WILL",
  "WONT", "DO", "DONT", "IAC", 0
};
#else
EXTERN char *telcmds[];
#endif

#define    	TELCMD_FIRST    	xEOF
#define    	TELCMD_LAST    	IAC
#define    	TELCMD_OK(x)    	((unsigned int)(x) <= TELCMD_LAST && \
(unsigned int)(x) >= TELCMD_FIRST)
#define    	TELCMD(x)    	telcmds[(x)-TELCMD_FIRST]


#define TELOPT_BINARY    	    	  0
#define TELOPT_ECHO    	    	  1
#define    	TELOPT_RCP    	    	  2
#define    	TELOPT_SGA    	    	  3
#define    	TELOPT_NAMS    	    	  4
#define    	TELOPT_STATUS    	    	  5
#define    	TELOPT_TM    	    	  6
#define    	TELOPT_RCTE    	    	  7
#define TELOPT_NAOL     	    	  8
#define TELOPT_NAOP     	    	  9
#define TELOPT_NAOCRD    	    	 10
#define TELOPT_NAOHTS    	    	 11
#define TELOPT_NAOHTD    	    	 12
#define TELOPT_NAOFFD    	    	 13
#define TELOPT_NAOVTS    	    	 14
#define TELOPT_NAOVTD    	    	 15
#define TELOPT_NAOLFD    	    	 16
#define TELOPT_XASCII    	    	 17
#define    	TELOPT_LOGOUT    	    	 18
#define    	TELOPT_BM    	    	 19
#define    	TELOPT_DET    	    	 20
#define    	TELOPT_SUPDUP    	    	 21
#define    	TELOPT_SUPDUPOUTPUT     	 22
#define    	TELOPT_SNDLOC    	    	 23
#define    	TELOPT_TTYPE    	    	 24
#define    	TELOPT_EOR    	    	 25
#define    	TELOPT_TUID    	    	 26
#define    	TELOPT_OUTMRK    	    	 27
#define    	TELOPT_TTYLOC    	    	 28
#define    	TELOPT_3270REGIME    	 29
#define    	TELOPT_X3PAD    	    	 30
#define    	TELOPT_NAWS    	    	 31
#define    	TELOPT_TSPEED    	    	 32
#define    	TELOPT_LFLOW    	    	 33
#define TELOPT_LINEMODE    	    	 34
#define TELOPT_XDISPLOC    	    	 35
#define TELOPT_OLD_ENVIRON    	 36
#define    	TELOPT_AUTHENTICATION    	 37
#define    	TELOPT_ENCRYPT    	    	 38
#define TELOPT_NEW_ENVIRON    	 39
#define TELOPT_3270E        	 40
#define TELOPT_XAUTH        	 41
#define TELOPT_CHARSET      	 42
#define TELOPT_RSP          	 43
#define TELOPT_COM_PORT     	 44
#define TELOPT_SLE          	 45
#define TELOPT_START_TLS    	 46
#define TELOPT_KERMIT       	 47
#define TELOPT_SEND_URL     	 48

#ifndef DISABLE_MCCP
#define COMPRESS_BUF_SIZE   (16 * KILOBYTE)
#define TELOPT_COMPRESS    	    	 85
#define TELOPT_COMPRESS2    	 86
#endif
#define    TELOPT_MSP       90
#define    TELOPT_MXP       91

#define TELOPT_PRAGMA_LOGON    	138
#define TELOPT_SSPI_LOGON    	139
#define TELOPT_PRAGMA_HEARTBEAT 140
#define TELOPT_IBM_SAK    	    	200

#define    	TELOPT_EXOPL    	    	255

#define    	NTELOPTS 141

#ifdef TELOPTS
char *telopts[NTELOPTS + 1] = {
  "BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
  "STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
  "NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
  "NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
  "DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
  "SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
  "TACACS UID", "OUTPUT MARKING", "TTYLOC",
  "3270 REGIME", "X.3 PAD", "NAWS", "TSPEED", "LFLOW",
  "LINEMODE", "XDISPLOC", "OLD-ENVIRON", "AUTHENTICATION",
  "ENCRYPT", "NEW-ENVIRON", "TN3270E", "XAUTH", "CHARSET",
  "REMOTE-SERIAL-PORT", "COM-PORT-CONTROL", "SUPPRESS-LOCAL-ECHO",
  "START-TLS", "KERMIT", "SEND-URL",
  0,
};

#define    	TELOPT_FIRST    	TELOPT_BINARY
#define    	TELOPT_LAST    	TELOPT_SEND_URL
#define    	TELOPT_OK(x)    	((unsigned int)(x) <= TELOPT_LAST)
#define    	TELOPT(x)    	telopts[(x)-TELOPT_FIRST]
#endif


#define    	TELQUAL_IS    	0
#define    	TELQUAL_SEND    	1
#define    	TELQUAL_INFO    	2
#define    	TELQUAL_REPLY    	2
#define    	TELQUAL_NAME    	3

#define    	LFLOW_OFF    	    	0
#define    	LFLOW_ON    	    	1
#define    	LFLOW_RESTART_ANY    	2
#define    	LFLOW_RESTART_XON    	3



#define    	LM_MODE    	    	1
#define    	LM_FORWARDMASK    	2
#define    	LM_SLC    	    	3

#define    	MODE_EDIT    	0x01
#define    	MODE_TRAPSIG    	0x02
#define    	MODE_ACK    	0x04
#define MODE_SOFT_TAB    	0x08
#define MODE_LIT_ECHO    	0x10

#define    	MODE_MASK    	0x1f


#define MODE_FLOW    	    	0x0100
#define MODE_ECHO    	    	0x0200
#define MODE_INBIN    	    	0x0400
#define MODE_OUTBIN    	    	0x0800
#define MODE_FORCE    	    	0x1000

#define    	SLC_SYNCH    	1
#define    	SLC_BRK    	    	2
#define    	SLC_IP    	    	3
#define    	SLC_AO    	    	4
#define    	SLC_AYT    	    	5
#define    	SLC_EOR    	    	6
#define    	SLC_ABORT    	7
#define    	SLC_EOF    	    	8
#define    	SLC_SUSP    	9
#define    	SLC_EC    	    	10
#define    	SLC_EL    	    	11
#define    	SLC_EW    	    	12
#define    	SLC_RP    	    	13
#define    	SLC_LNEXT    	14
#define    	SLC_XON    	    	15
#define    	SLC_XOFF    	16
#define    	SLC_FORW1    	17
#define    	SLC_FORW2    	18

#define    	NSLC    	    	18


#define    	SLC_NAMELIST    	"0", "SYNCH", "BRK", "IP", "AO", "AYT", "EOR", \
"ABORT", "EOF", "SUSP", "EC", "EL", "EW", "RP", \
"LNEXT", "XON", "XOFF", "FORW1", "FORW2", 0,
#ifdef SLC_NAMES
char *slc_names[] = {
  SLC_NAMELIST
};
#else
EXTERN char *slc_names[];

#define    	SLC_NAMES SLC_NAMELIST
#endif

#define    	SLC_NAME_OK(x)    	((unsigned int)(x) <= NSLC)
#define SLC_NAME(x)    	slc_names[x]

#define    	SLC_NOSUPPORT    	0
#define    	SLC_CANTCHANGE    	1
#define    	SLC_VARIABLE    	2
#define    	SLC_DEFAULT    	3
#define    	SLC_LEVELBITS    	0x03

#define    	SLC_FUNC    	0
#define    	SLC_FLAGS    	1
#define    	SLC_VALUE    	2

#define    	SLC_ACK    	    	0x80
#define    	SLC_FLUSHIN    	0x40
#define    	SLC_FLUSHOUT    	0x20

#define    	OLD_ENV_VAR    	1
#define    	OLD_ENV_VALUE    	0
#define    	NEW_ENV_VAR    	0
#define    	NEW_ENV_VALUE    	1
#define    	ENV_ESC    	    	2
#define ENV_USERvar    	3




#define    	AUTH_WHO_CLIENT    	    	0
#define    	AUTH_WHO_SERVER    	    	1
#define    	AUTH_WHO_MASK    	    	1


#define    	AUTH_HOW_ONE_WAY    	0
#define    	AUTH_HOW_MUTUAL    	    	2
#define    	AUTH_HOW_MASK    	    	2


#define AUTH_ENCRYPT_OFF    	     0
#define AUTH_ENCRYPT_USING_TELOPT    4
#define AUTH_ENCRYPT_ON    	    	     AUTH_ENCRYPT_USING_TELOPT
#define AUTH_ENCRYPT_AFTER_EXCHANGE 16
#define AUTH_ENCRYPT_RESERVED       20
#define AUTH_ENCRYPT_MASK    	    20


#define INI_CREd_FWD_OFF    	0
#define INI_CREd_FWD_ON    	    	8
#define INI_CREd_FWD_MASK    	8

#define    	AUTHTYPE_NULL    	    	0
#define    	AUTHTYPE_KERBEROS_V4    	1
#define    	AUTHTYPE_KERBEROS_V5    	2
#define    	AUTHTYPE_SPX    	    	3
#define    	AUTHTYPE_MINK    	    	4
#define    	AUTHTYPE_SRP    	    	5
#define AUTHTYPE_RSA            6
#define AUTHTYPE_SSL            7
#define AUTHTYPE_LOKI          10
#define AUTHTYPE_SSA           11
#define AUTHTYPE_KEA_SJ        12
#define AUTHTYPE_KEA_INTEG     13
#define AUTHTYPE_DSS           14
#define AUTHTYPE_NTLM          15
#define    	AUTHTYPE_CNT    	       16

#define    	AUTHTYPE_TEST    	    	99

#ifdef AUTH_NAMES
char *authtype_names[] = {
  "NULL",
  "KERBEROS_V4",
  "KERBEROS_V5",
  "SPX",
  "MINK/unassigned_4",
  "SRP",
  "RSA",
  "SSL",
  "IANA_8",
  "IANA_9",
  "LOKI",
  "SSA",
  "KEA_SJ",
  "KEA_SJ_INTEG",
  "DSS",
  "NTLM",
  0
};
#else
EXTERN char *authtype_names[];
#endif

#define    	AUTHTYPE_NAME_OK(x)    	((unsigned int)(x) < AUTHTYPE_CNT)
#define    	AUTHTYPE_NAME(x)    	authtype_names[x]


#define    	ENCRYPT_IS    	    	0
#define    	ENCRYPT_SUPPORT    	    	1
#define    	ENCRYPT_REPLY    	    	2
#define    	ENCRYPT_START    	    	3
#define    	ENCRYPT_END    	    	4
#define    	ENCRYPT_REQSTART    	5
#define    	ENCRYPT_REQEND    	    	6
#define    	ENCRYPT_ENC_KeyID    	7
#define    	ENCRYPT_DEC_KeyID    	8
#define    	ENCRYPT_CNT    	    	9

#define    	ENCTYPE_ANY    	    	0
#define    	ENCTYPE_DES_CFB64    	1
#define    	ENCTYPE_DES_OFB64    	2
#define ENCTYPE_DES3_CFB64      3
#define ENCTYPE_DES3_OFB64      4
#define    	ENCTYPE_CAST5_40_CFB64    	8
#define    	ENCTYPE_CAST5_40_OFB64    	9
#define    	ENCTYPE_CAST128_CFB64    	10
#define    	ENCTYPE_CAST128_OFB64    	11
#define    	ENCTYPE_CNT    	    	12

#ifdef ENCRYPT_NAMES
char *encrypt_names[] = {
  "IS", "SUPPORT", "REPLY", "START", "END",
  "REQUEST-START", "REQUEST-END", "ENC-KeyID", "DEC-KeyID",
  0,
};
char *enctype_names[] = {
  "ANY", "DES_CFB64", "DES_OFB64", "DES3_CFB64", "DES3_OFB64",
  "UNKNOWN", "UNKNOWN", "UNKNOWN", "CAST5_40_CFB64", "CAST5_40_OFB64",
  "CAST128_CFB64", "CAST128_OFB64", 0,
};
#else
EXTERN char *encrypt_names[];
EXTERN char *enctype_names[];
#endif

#define    	ENCRYPT_NAME_OK(x)    	((unsigned int)(x) < ENCRYPT_CNT)
#define    	ENCRYPT_NAME(x)    	    	encrypt_names[x]

#define    	ENCTYPE_NAME_OK(x)    	((unsigned int)(x) < ENCTYPE_CNT)
#define    	ENCTYPE_NAME(x)    	    	enctype_names[x]

#endif
