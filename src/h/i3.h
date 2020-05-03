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
* Ported to Smaug 1.4a by Samson of Alsherok. Consolidated for            *
* cross-codebase compatibility by Samson of Alsherok. Modifications and   *
* enhancements to the code Copyright (c) 2001-2003 Roger Libiez           *
* ( Samson ) Registered with the United States Copyright Office           *
* TX 5-562-404.  I condensed the 14 or so Fatal Dimensions source code    *
* files into this one file, because I for one find it far easier to       *
* maintain when all of the functions are right here in one file.          *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#ifndef __I3_H_
#define __I3_H_

#ifndef DISABLE_I3


#define MAX_I3HISTORY 20
#define MAX_I3TELLHISTORY 20

#define I3DIR   DATA_DIR"i3"DIR_SYM


#define I3_CONFIG_FILE   I3DIR "i3.config"
#define I3_CHANNEL_FILE  I3DIR "i3.channels"
#define I3_ROUTER_FILE   I3DIR "i3.routers"
#define I3_UCACHE_FILE   I3DIR "i3.ucache"
#define I3_MUDLIST_FILE  I3DIR "i3.mudlist"
#define I3_CHANLIST_FILE I3DIR "i3.chanlist"
#define I3_BAN_FILE      I3DIR "i3.bans"
#define I3_COLOR_FILE    I3DIR "i3.color"
#define I3_HELP_FILE     I3DIR "i3.help"
#define I3_CMD_FILE      I3DIR "i3.commands"

typedef enum
{
  I3PERM_NOTSET, I3PERM_NONE, I3PERM_MORT, I3PERM_IMM, I3PERM_ADMIN,
  I3PERM_IMP
}
i3perm_t;


#define I3CHAN_LOG      (BIT_A)


#define I3_TELL         (BIT_A)
#define I3_DENYTELL     (BIT_B)
#define I3_BEEP         (BIT_C)
#define I3_DENYBEEP     (BIT_D)
#define I3_INVIS        (BIT_E)
#define I3_PRIVACY      (BIT_F)
#define I3_DENYFINGER   (BIT_G)
#define I3_AFK          (BIT_H)
#define I3_COLORFLAG    (BIT_I)
#define I3_PERMOVERRIDE (BIT_J)




#define I3DRIVER MUDNAME " I3 Driver 2.40"


#define IPS 131072
#define OPS 65536

#define I3INVIS(ch)           ( IsSet( (ch)->pcdata->i3chardata->i3flags, I3_INVIS ) )
#define I3AFK(ch)             ( IsSet( (ch)->pcdata->i3chardata->i3flags, I3_AFK ) )
#define I3ISINVIS(ch)         ( I3WIZINVIS(ch) || I3INVIS(ch) )
#define CH_I3RANK(ch)         ( is_clan(ch) ? (ch)->pcdata->clan->rank[(ch)->rank].rankname : ClassName(ch, prime_class(ch)) )
#define I3WIZINVIS(ch)        ( IsImmortal(ch) && (ch)->invis_level > 0 )


typedef struct I3_channel I3_CHANNEL;
typedef struct I3_mud I3_MUD;
typedef struct I3_header I3_HEADER;
typedef struct I3_ignore I3_IGNORE;
typedef struct I3_ban I3_BAN;
typedef struct ucache_data I3_UCACHE;
typedef struct i3_chardata I3_CHARDATA;
typedef struct router_data ROUTER_DATA;
typedef struct i3_color I3_COLOR;
typedef struct i3_command I3_CMD_DATA;
typedef struct i3_help I3_HELP_DATA;

EXTERN SOCKET I3_control;

EXTERN I3_MUD i3mud_info;

EXTERN I3_CMD_DATA i3cmd_table[];

struct i3_command
{
  I3_CMD_DATA *next;
  I3_CMD_DATA *prev;
  NameList *alias_first, *alias_last;
  Do_F *function;
  const char *name, *args, *descr;
  i3perm_t level;
  bool connected;
};

struct i3_help
{
  I3_HELP_DATA *next;
  I3_HELP_DATA *prev;
  const char *name;
  const char *text;
  int level;
};

struct i3_color
{
  I3_COLOR *next;
  I3_COLOR *prev;
  const char *name;
  const char *mudtag;
  const char *i3tag;
  const char *i3fish;
};

struct router_data
{
  ROUTER_DATA *next;
  ROUTER_DATA *prev;
  const char *name;
  const char *ip;
  int port;
  int reconattempts;
};

struct ucache_data
{
  I3_UCACHE *next;
  I3_UCACHE *prev;
  const char *name;
  sex_t gender;
  time_t time;
};

struct I3_ignore
{
  I3_IGNORE *next;
  I3_IGNORE *prev;
  const char *name;
};

struct I3_ban
{
  I3_BAN *next;
  I3_BAN *prev;
  const char *name;
};

struct i3_chardata
{
  const char *i3_replyname;
  const char *i3_listen;
  const char *i3_denied;
  const char *i3_tellhistory[MAX_I3TELLHISTORY];
  flag_t i3flags;
  i3perm_t i3perm;
  I3_IGNORE *i3ignore_first;
  I3_IGNORE *i3ignore_last;
};

struct I3_header
{
  char originator_mudname[256];
  char originator_username[256];
  char target_mudname[256];
  char target_username[256];
};

struct I3_channel
{
  I3_CHANNEL *next;
  I3_CHANNEL *prev;
  const char *local_name;
  const char *host_mud;
  const char *I3_name;
  const char *layout_m;
  const char *layout_e;
  int status;
  i3perm_t i3perm;
  const char *history[MAX_I3HISTORY];
  flag_t flags;
};

struct I3_mud
{
  I3_MUD *next;
  I3_MUD *prev;


  int status;
  const char *name;
  const char *ipaddress;
  const char *mudlib;
  const char *base_mudlib;
  const char *driver;
  const char *mud_type;
  const char *open_status;
  const char *admin_email;
  const char *telnet;
  const char *web_wrong;
  int player_port;
  int imud_tcp_port;
  int imud_udp_port;

  bool tell;
  bool beep;
  bool emoteto;
  bool who;
  bool finger;
  bool locate;
  bool channel;
  bool news;
  bool mail;
  bool file;
  bool auth;
  bool ucache;

  int smtp;
  int ftp;
  int nntp;
  int http;
  int pop3;
  int rcp;
  int amrcp;


  const char *banner;
  const char *web;
  const char *time;
  const char *daemon;
  int jeamland;


  bool autoconnect;
  int password;
  int mudlist_id;
  int chanlist_id;
  int minlevel;
  int immlevel;
  int adminlevel;
  int implevel;
};

GLOBAL_LIST (I3_UCACHE, ucache);
GLOBAL_LIST (ROUTER_DATA, router);
GLOBAL_LIST (I3_BAN, I3ban);
GLOBAL_LIST (I3_CHANNEL, I3chan);
GLOBAL_LIST (I3_MUD, i3mud);
GLOBAL_LIST (I3_COLOR, i3color);
GLOBAL_LIST (I3_HELP_DATA, i3help);
GLOBAL_LIST (I3_CMD_DATA, i3cmd);


Proto (void I3_main, (bool));

Proto (bool I3_is_connected, (void));
Proto (void I3_loop, (void));
Proto (void I3_shutdown, (int));

Proto (bool I3_command_hook, (CharData *, const char *, const char *));
Proto (void i3init_char, (CharData *));
Proto (void i3save_char, (CharData *, FileData *));

Proto (bool i3load_char, (CharData *, FileData *, const char *));
Proto (void free_i3chardata, (PcData *));

Declare_TableSave_F (rw_i3_mudlist);
Declare_TableSave_F (rw_i3_chanlist);
Proto (void I3_savehistory, (void));

EXTERN FlagTable i3perm_names[];

#endif
#endif
