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

#define TELOPTS    	1
#define TELCMDS    	1

#include "merc.h"
#include "telnet.h"

const char echo_off_str[] = {
  IAC, WILL, TELOPT_ECHO, NUL
};
const char echo_on_str[] = {
  IAC, WONT, TELOPT_ECHO, NUL
};
const char echo_dont[] = {
  IAC, DONT, TELOPT_ECHO, NUL
};
const char echo_do[] = {
  IAC, DO, TELOPT_ECHO, NUL
};

const char eor_do[] = {
  IAC, DO, TELOPT_EOR, NUL
};
const char eor_will[] = {
  IAC, WILL, TELOPT_EOR, NUL
};
const char eor_dont[] = {
  IAC, DONT, TELOPT_EOR, NUL
};
const char eor_wont[] = {
  IAC, WONT, TELOPT_EOR, NUL
};

const char iac_logout[] = {
  IAC, DO, TELOPT_LOGOUT, NUL
};


const char tspd_will[] = {
  IAC, WILL, TELOPT_TSPEED, NUL
};
const char tspd_do[] = {
  IAC, DO, TELOPT_TSPEED, NUL
};
const char tspd_wont[] = {
  IAC, WONT, TELOPT_TSPEED, NUL
};
const char tspd_dont[] = {
  IAC, DONT, TELOPT_TSPEED, NUL
};

const char naws_will[] = {
  IAC, WILL, TELOPT_NAWS, NUL
};
const char naws_dont[] = {
  IAC, DONT, TELOPT_NAWS, NUL
};
const char naws_do[] = {
  IAC, DO, TELOPT_NAWS, NUL
};
const char naws_wont[] = {
  IAC, WONT, TELOPT_NAWS, NUL
};
const char naws_sb[] = {
  IAC, SB, TELOPT_NAWS, NUL
};

const char iac_ip[] = {
  IAC, IP, NUL
};
const char iac_susp[] = {
  IAC, SUSP, NUL
};
const char iac_brk[] = {
  IAC, BREAK, NUL
};

const char iac_se[] = {
  IAC, SE, NUL
};

const char go_ahead_str[] = {
  IAC, GA, NUL
};
const char will_suppress_ga_str[] = {
  IAC, WILL, TELOPT_SGA, NUL
};
const char wont_suppress_ga_str[] = {
  IAC, WONT, TELOPT_SGA, NUL
};

#ifndef DISABLE_MCCP
const char compress1_will[] = {
  IAC, WILL, TELOPT_COMPRESS, NUL
};
const char compress1_do[] = {
  IAC, DO, TELOPT_COMPRESS, NUL
};
const char compress1_dont[] = {
  IAC, DONT, TELOPT_COMPRESS, NUL
};
const char compress1_wont[] = {
  IAC, WONT, TELOPT_COMPRESS, NUL
};

const char compress2_will[] = {
  IAC, WILL, TELOPT_COMPRESS2, NUL
};
const char compress2_do[] = {
  IAC, DO, TELOPT_COMPRESS2, NUL
};
const char compress2_dont[] = {
  IAC, DONT, TELOPT_COMPRESS2, NUL
};
const char compress2_wont[] = {
  IAC, WONT, TELOPT_COMPRESS2, NUL
};
#endif

const char msp_will[] = {
  IAC, WILL, TELOPT_MSP, NUL
};
const char msp_do[] = {
  IAC, DO, TELOPT_MSP, NUL
};
const char msp_dont[] = {
  IAC, DONT, TELOPT_MSP, NUL
};
const char msp_wont[] = {
  IAC, WONT, TELOPT_MSP, NUL
};

const char mxp_will[] = {
  IAC, WILL, TELOPT_MXP, NUL
};
const char mxp_do[] = {
  IAC, DO, TELOPT_MXP, NUL
};
const char mxp_dont[] = {
  IAC, DONT, TELOPT_MXP, NUL
};
const char mxp_wont[] = {
  IAC, WONT, TELOPT_MXP, NUL
};

const char s_mxp_supports[] = {
  MXPMODE (1) "<SUPPORTS"
};
const char s_mxp_version[] = {
  MXPMODE (1) "<VERSION"
};



const char pueblo_str[] = { "PUEBLOCLIENT "
};
const char ptype_3klient[] = { "3klient "
};
const char imp_str[] = { "v1."
};

const char ttype_do[] = {
  IAC, DO, TELOPT_TTYPE, NUL
};
const char ttype_dont[] = {
  IAC, DONT, TELOPT_TTYPE, NUL
};
const char ttype_sb[] = {
  IAC, SB, TELOPT_TTYPE, NUL
};
const char ttype_send[] = {
  IAC, SB, TELOPT_TTYPE, SEND, IAC, SE, NUL
};
const char ttype_will[] = {
  IAC, WILL, TELOPT_TTYPE, NUL
};
const char ttype_wont[] = {
  IAC, WONT, TELOPT_TTYPE, NUL
};

const char binary_do[] = {
  IAC, DO, TELOPT_BINARY, NUL
};
const char binary_dont[] = {
  IAC, DONT, TELOPT_BINARY, NUL
};
const char binary_will[] = {
  IAC, WILL, TELOPT_BINARY, NUL
};
const char binary_wont[] = {
  IAC, WONT, TELOPT_BINARY, NUL
};

Proto (void init_mxp, (Descriptor *));
Proto (void mxp_support, (Descriptor *, int, unsigned char *));
Proto (void mxp_version, (Descriptor *, int, unsigned char *));

#define MTELOPT(string, command, len)    	    	    	    	    	\
    	    if (!memcmp(&buf[i],(string),strlen((string))))    	    	\
    	    {     	    	    	    	    	    	    	    	\
            telopt_debug(d,&buf[i],strlen((string))+(len)+telopt_lskip,false);\
            i += strlen((string)) - 1;     	    	    	    	\
            (command);     	    	    	    	    	    	\
            i += len;    	    	    	    	    	    	\
            i += telopt_lskip;    	    	    	    	    	\
            telopt_lskip = 0;    	    	    	    	    	\
            continue;    	    	    	    	    	    	\
    	    }

#define MSTRING(string, command, len)    	    	    	    	    	\
    	    if (!memcmp(&buf[i],(string),strlen((string))))    	    	\
    	    {     	    	    	    	    	    	    	    	\
            (command);     	    	    	    	    	    	\
            i += telopt_lskip;    	    	    	    	    	\
            telopt_lskip = 0;    	    	    	    	    	\
            continue;    	    	    	    	    	    	\
    	    }


void
init_telnet (Descriptor * d)
{
  d_write (d, tspd_will, 0);

  return;
}

void
telopt_init (Descriptor * d)
{

  d_write (d, eor_will, 0);


  d_write (d, ttype_do, 0);

  d_write (d, naws_do, 0);

#ifndef DISABLE_MCCP

  d_write (d, compress2_will, 0);

  d_write (d, compress1_will, 0);
#endif

  d_write (d, msp_will, 0);

  d_write (d, mxp_will, 0);

  d_write (d, binary_will, 0);

  return;
}

void
telopt_debug (Descriptor * d, unsigned char *string, int length, bool send)
{
  int i, j;
  static char buf[MSL];
  unsigned int c;
  bool sb = false;

  if (!d)
    return;

  sprintf (buf, "Telopt: [%d][%s][ ", d->descriptor,
	   (send) ? "send" : "recv");

  if (string[0] != IAC)
    return;

  for (i = 0; i < length; i++)
    {
      c = string[i];

      if (TELCMD_OK (c))
	{
	  sprintf (buf + strlen (buf), "%s ", TELCMD (c));

	  if (c == SB)
	    sb = true;
	  if (c == SE)
	    sb = false;

	  continue;
	}
      else if (TELOPT_OK (c))
	{
	  sprintf (buf + strlen (buf), "%s ", TELOPT (c));

	  if (sb && c == TELOPT_NAWS)
	    {
	      for (j = 0; j < 4; j++)
		{
		  i++;
		  c = string[i];
		  sprintf (buf + strlen (buf), "%u ", c);
		}
	    }

	  if (sb && c == TELOPT_TTYPE)
	    {
	      i++;
	      c = string[i];
	      if (c == SEND)
		{
		  strcat (buf, "SEND ");
		}
	      else if (c == IS)
		{
		  strcat (buf, "IS ");
		  do
		    {
		      i++;
		      c = string[i];
		      sprintf (buf + strlen (buf), "%c", c);
		    }
		  while (c != IAC);
		  strcat (buf, " ");
		}
	      else
		{
		  sprintf (buf + strlen (buf), "%u ", c);
		}
	    }

	  continue;
	}
      else
	{
	  sprintf (buf + strlen (buf), "%u ", c);
	}
    }

  sprintf (buf + strlen (buf), "]");
  wiznet (buf, NULL, NULL, WIZ_TELNET, false, MAX_LEVEL - 2);
  return;
}

void
telopt_ignore (void)
{
  return;
}


void
telopt_send (Descriptor * d, const char *string, int len)
{
  d_write (d, string, len);
  return;
}


void
telopt_close (Descriptor * d)
{
  wiznet ("IAC IP/SUSP received, closing link to $N.", CH (d), NULL,
	  WIZ_LINKS, true, 0);
  close_socket (d);
  return;
}


void
telopt_eor (Descriptor * d, bool state)
{
  if (state)
    SetBit (d->desc_flags, DESC_TELOPT_EOR);
  else
    RemBit (d->desc_flags, DESC_TELOPT_EOR);
  return;
}

void
telopt_msp (Descriptor * d, bool state)
{
  if (state)
    SetBit (d->desc_flags, DESC_MSP);
  else
    RemBit (d->desc_flags, DESC_MSP);
  return;
}

void
telopt_mxp (Descriptor * d, bool state)
{
  if (state)
    {
      SetBit (d->desc_flags, DESC_MXP);
      init_mxp (d);
    }
  else
    {
      RemBit (d->desc_flags, DESC_MXP);
    }
  return;
}

#ifndef DISABLE_MCCP
void
telopt_compress (Descriptor * d, bool state, int version)
{
  if (d->out_compress)
    return;

  if (state)
    compressStart (d, version);
  else
    compressEnd (d, version);
  return;
}
#endif

void
telopt_echo (Descriptor * d, bool state)
{
  if (state)
    SetBit (d->desc_flags, DESC_TELOPT_ECHO);
  else
    RemBit (d->desc_flags, DESC_TELOPT_ECHO);
  return;
}


int
telopt_unknown (Descriptor * d, unsigned int c, char t, bool quiet)
{
  char buf[MSL];
  char cmd[MIL];
  char opt[MIL];
  char rev;
  char len = 1;

  if (c == '\n' || c == '\r' || c == NUL)
    return 0;

  if (TELCMD_OK (c))
    {
      sprintf (cmd, "%s", TELCMD (c));
      if (c == IAC)
	len = 1;

      else if (c >= SB)
	len = 2;

      else
	len = 1;

    }

  if (!quiet)
    {
      if (TELCMD_OK (c))
	sprintf (cmd, "%s", TELCMD (c));
      else
	sprintf (cmd, "[%u]", c);

      if (TELOPT_OK (t))
	sprintf (opt, "%s", TELOPT (t));
      else if (TELCMD_OK (t))
	sprintf (opt, "%s", TELCMD (t));
#ifndef DISABLE_MCCP

      else if (t == TELOPT_COMPRESS)
	sprintf (opt, "COMPRESS-1");
      else if (t == TELOPT_COMPRESS2)
	sprintf (opt, "COMPRESS-2");
#endif

      else if (t == TELOPT_MSP)
	sprintf (opt, "MSP");
      else if (t == TELOPT_MXP)
	sprintf (opt, "MXP");
      else
	sprintf (opt, "[%u]", t);

      switch (c)
	{
	case WILL:
	  rev = WONT;
	  break;
	case WONT:
	  rev = DONT;
	  break;
	case DONT:
	  rev = WONT;
	  break;
	case DO:
	  rev = DONT;
	  break;
	default:
	  rev = c;
	  break;
	}
      sprintf (buf, "%c%c%c", IAC, rev, t);
      d_write (d, buf, 0);
      logf ("Unknown to client: IAC %s %s", cmd, opt);
    }
  return len;
}


void
telopt_naws_do (Descriptor * d)
{
  if (IsSet (d->desc_flags, DESC_TELOPT_NAWS))
    return;

  d_write (d, naws_do, 0);
  return;
}


void
telopt_naws (Descriptor * d, int i, unsigned char *inbuf)
{
  int x = 0, y = 0, t1, t2;

  SetBit (d->desc_flags, DESC_TELOPT_NAWS);

  t1 = inbuf[i + 1];
  t2 = inbuf[i + 2];
  x = t2 + (t1 * 16);

  t1 = inbuf[i + 3];
  t2 = inbuf[i + 4];
  y = t2 + (t1 * 16);

  d->scr_width = Range (10, x, 250);
  d->scr_height = Range (10, y, 250);

  return;
}

void
telopt_ttype (Descriptor * d, int i, int len, unsigned char *inbuf)
{
  int n;
  int skip = 0;

  d->ttype[0] = NUL;

  SetBit (d->desc_flags, DESC_TELOPT_TTYPE);

  for (n = 0; n <= len; n++)
    {
      if (inbuf[n + i + 2] == IAC)
	{
	  if (inbuf[n + i + 3] == SE)
	    {
	      telopt_lskip = skip;
	      return;
	    }
	}
      else
	{
	  if (n < 59)
	    {
	      d->ttype[n] = inbuf[n + i + 2];
	      d->ttype[n + 1] = NUL;
	    }
	  skip++;
	}
    }

  return;
}

void
telopt_binary (Descriptor * d, bool state)
{
  if (state)
    SetBit (d->desc_flags, DESC_TELOPT_BINARY);
  else
    RemBit (d->desc_flags, DESC_TELOPT_BINARY);
  return;
}

void
pueblo_client (Descriptor * d, int i, unsigned char *inbuf)
{
  char *buf = (char *) &inbuf[i];
  char send[MSL];

  sscanf (buf, "PUEBLOCLIENT %lf", &d->pueblo_vers);

  telopt_lskip = strlen (buf);

  SetBit (d->desc_flags, DESC_PUEBLO);

  sprintf (send, NEWLINE "Pueblo version %.2f detected...", d->pueblo_vers);
  d_write (d, send, 0);

  return;
}

void
portal_3klient (Descriptor * d, int i, unsigned char *inbuf)
{
  char *buf = (char *) &inbuf[i];
  static char tbuf[MSL];
  int n;

  SetBit (d->desc_flags, DESC_TELOPT_TTYPE);
  SetBit (d->desc_flags, DESC_PORTAL);

  sscanf (buf, "3klient %u~%s\r\n", &d->portal.keycode, tbuf);

  for (n = 0; tbuf != NUL; n++)
    if (isdigit (tbuf[n]))
      break;

  snprintf (d->portal.version, 20, "Portal %s", &tbuf[n]);

  telopt_lskip = strlen (buf);

  d_print (d, NEWLINE "Portal GT Detected...");

  return;

}

void
fire_client (Descriptor * d, int i, unsigned char *inbuf)
{
  char *buf = (char *) &inbuf[i];
  int vers;

  SetBit (d->desc_flags, DESC_IMP);

  sscanf (buf, "v1.%2d", &vers);

  telopt_lskip = strlen (buf);

  d->imp_vers = (double) ((vers * .01) + 1);
  d_printf (d, NEWLINE "Fire Client version v%.2f detected...", d->imp_vers);

  return;
}


void
process_telnet (Descriptor * d, int len, unsigned char *buf)
{
  int i;
  int iStart = strlen (d->inbuf);

  if (len <= 0)
    return;

  for (i = 0; i <= len; i++)
    {
      if (buf[i] == IAC)
	{
	  switch (buf[i + 1])
	    {
	    case IAC:
	      continue;
	      break;

	    case DO:
	      MTELOPT (tspd_do, telopt_init (d), 0);
	      MTELOPT (eor_do, telopt_eor (d, true), 0);
	      MTELOPT (echo_do, telopt_echo (d, true), 0);
#ifndef DISABLE_MCCP

	      MTELOPT (compress2_do, telopt_compress (d, true, 2), 0);
	      MTELOPT (compress1_do, telopt_compress (d, true, 1), 0);
#endif

	      MTELOPT (msp_do, telopt_msp (d, true), 0);
	      MTELOPT (mxp_do, telopt_mxp (d, true), 0);
	      MTELOPT (ttype_do, telopt_ignore (), 0);
	      MTELOPT (binary_do, telopt_binary (d, true), 1);
	      MTELOPT (iac_logout, telopt_ignore (), 0);
	      break;

	    case DONT:
	      MTELOPT (tspd_dont, telopt_init (d), 0);
#ifndef DISABLE_MCCP

	      MTELOPT (compress2_dont, telopt_compress (d, false, 2), 0);
	      MTELOPT (compress1_dont, telopt_compress (d, false, 1), 0);
#endif

	      MTELOPT (msp_dont, telopt_msp (d, false), 0);
	      MTELOPT (mxp_dont, telopt_mxp (d, false), 0);
	      MTELOPT (ttype_dont, telopt_ignore (), 0);
	      MTELOPT (binary_dont, telopt_binary (d, false), 1);
	      MTELOPT (echo_dont, telopt_echo (d, false), 0);
	      MTELOPT (eor_dont, telopt_eor (d, false), 0);
	      break;

	    case WILL:
	      MTELOPT (naws_will, telopt_naws_do (d), 0);
	      MTELOPT (tspd_will, telopt_init (d), 0);
#ifndef DISABLE_MCCP

	      MTELOPT (compress2_will, telopt_compress (d, true, 2), 0);
	      MTELOPT (compress1_will, telopt_compress (d, true, 1), 0);
#endif

	      MTELOPT (eor_will, telopt_eor (d, true), 0);
	      MTELOPT (msp_will, telopt_msp (d, true), 0);
	      MTELOPT (mxp_will, telopt_mxp (d, true), 0);
	      MTELOPT (ttype_will, telopt_send (d, ttype_send, 6), 0);
	      MTELOPT (binary_will, telopt_binary (d, true), 1);
	      break;

	    case WONT:
	      MTELOPT (naws_wont, telopt_ignore (), 0);
	      MTELOPT (tspd_wont, telopt_init (d), 0);
	      MTELOPT (eor_wont, telopt_eor (d, false), 0);
#ifndef DISABLE_MCCP

	      MTELOPT (compress2_wont, telopt_compress (d, false, 2), 0);
	      MTELOPT (compress1_wont, telopt_compress (d, false, 1), 0);
#endif

	      MTELOPT (mxp_wont, telopt_mxp (d, false), 0);
	      MTELOPT (msp_wont, telopt_msp (d, false), 0);
	      MTELOPT (ttype_wont, telopt_ignore (), 0);
	      MTELOPT (binary_wont, telopt_binary (d, false), 1);
	      break;

	    case SB:
	      MTELOPT (naws_sb, telopt_naws (d, i, buf), 6);
	      MTELOPT (ttype_sb, telopt_ttype (d, i, len, buf), 1);
	      break;

	    case IP:

	      MTELOPT (iac_ip, telopt_close (d), 0);
	      break;

	    case SUSP:
	      MTELOPT (iac_susp, telopt_close (d), 0);
	      break;

	    case BREAK:

	      MTELOPT (iac_brk, telopt_ignore (), 0);
	      break;

	    case SE:

	      MTELOPT (iac_se, telopt_ignore (), 0);
	      break;

	    default:

	      i += telopt_unknown (d, buf[i + 1], buf[i + 2], false);
	      break;
	    }
	}
      else
	{
	  if (buf[i] == 'P' && buf[i + 1] == 'U')
	    MSTRING (pueblo_str, pueblo_client (d, i, buf), 0);

	  if (buf[i] == '3' && buf[i + 1] == 'k')
	    MSTRING (ptype_3klient, portal_3klient (d, i, buf), 0);

	  if (buf[i] == 'v' && buf[i + 1] == '1')
	    MSTRING (imp_str, fire_client (d, i, buf), 0);

	  if (buf[i] == ESCc && buf[i + 1] == '[')
	    {
	      MSTRING (s_mxp_version, mxp_version (d, i, buf), 0);
	      MSTRING (s_mxp_supports, mxp_support (d, i + 4, buf), 0);
	    }

	  switch (buf[i])
	    {
	    case '~':
	      d->inbuf[iStart++] = COLORCODE;
	      d->inbuf[iStart++] = '-';
	      break;

	    default:
	      d->inbuf[iStart++] = buf[i];
	      break;
	    }
	}
    }
  return;
}


void
set_desc_flags (Descriptor * d)
{
  CharData *ch = CH (d);

  if (!ch)
    return;

  if (IsSet (d->desc_flags, DESC_TELOPT_EOR))
    SetBit (ch->comm, COMM_TELNET_EOR);
  else
    RemBit (ch->comm, COMM_TELNET_EOR);

  if (IsSet (ch->comm, COMM_NOCOLOR))
    RemBit (d->desc_flags, DESC_COLOR);

  return;
}
