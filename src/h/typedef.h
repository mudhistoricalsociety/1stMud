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

#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_    	1

#ifndef __cplusplus
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
typedef unsigned char _Bool;
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif
#endif


#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

typedef signed long int vnum_t;
typedef unsigned long int money_t;

#ifdef HAVE_INT64_T
typedef int64_t flag_t;
#elif defined WIN32
typedef __int64 flag_t;
#elif SIZEOF_LONG_LONG == 8
typedef signed long long int flag_t;
#elif !defined DEPEND
#error "unable to find a 64 bit flag type"
#endif

typedef volatile sig_atomic_t crash_t;

#ifndef WIN32
typedef unsigned int SOCKET;
#endif

#ifndef HAVE_SOCKLEN_T
#ifdef WIN32
typedef int socklen_t;
#elif !defined __CYGWIN__
typedef unsigned int socklen_t;
#endif
#endif


typedef struct affect_data AffectData;
typedef struct area_data AreaData;
typedef struct ban_data BanData;
typedef struct buf_type Buffer;
typedef struct char_data CharData;
typedef struct descriptor_data Descriptor;
typedef struct exit_data ExitData;
typedef struct ed_data ExDescrData;
typedef struct help_data HelpData;
typedef struct kill_data KillData;
typedef struct char_index CharIndex;
typedef struct note_data NoteData;
typedef struct obj_data ObjData;
typedef struct obj_index ObjIndex;
typedef struct pc_data PcData;
typedef struct gen_data GenData;
typedef struct reset_data ResetData;
typedef struct room_index RoomIndex;
typedef struct shop_data ShopData;
typedef struct time_data TimeData;
typedef struct weather_data WeatherData;
typedef struct prog_list ProgList;
typedef struct prog_code ProgCode;
typedef struct disabled_data DisabledData;
typedef struct stat_data StatData;
typedef struct social_type SocialData;
typedef struct gquest_type Gquest;
typedef struct war_type War;
typedef struct clan_type ClanData;
typedef struct cmd_type CmdData;
typedef struct skill_type SkillData;
typedef struct group_type GroupData;
typedef struct race_type RaceData;
typedef struct class_type ClassData;
typedef struct corpse_data CorpseData;
typedef struct auction_data AuctionData;
typedef struct clan_rank ClanRank;
typedef struct deity_type DeityData;
typedef struct wpwd_data WebpassData;
typedef struct mbr_data ClanMember;
typedef struct war_data WarData;
typedef struct gquest_data GqData;
typedef struct channel_type ChannelData;
typedef struct crs_data Crs;
typedef struct mud_data Mud;
typedef struct song_data SongData;
typedef struct st_data stFile;
typedef struct column_data Column;
typedef struct msp_data MspData;
typedef struct name_profile NameProfile;
typedef struct quest_data QuestData;
typedef struct board_data BoardData;
typedef struct name_list NameList;
typedef struct wiz_data WizData;
typedef struct area_index AreaIndex;

typedef enum
{ act_read, act_write }
action_t;

typedef const struct datatable_type DataTable;
typedef const struct olc_ed_type OlcTable;
typedef const struct flag_type FlagTable;
typedef struct colatt_type colatt_t;


typedef void Do_F (const char *, CharData *, const char *);
typedef bool Spec_F (CharData *);
typedef bool Spell_F (int, int, CharData *, void *, int);
typedef int Lookup_F (const char *);
typedef bool Validate_F (CharData *, const void *);
typedef bool Ed_F (DataTable *, CharData *, void *, const char *, void *);
typedef void Obj_F (ObjData *, const char *);
typedef void Room_F (RoomIndex *, const char *);
typedef bool Olc_F (const char *, CharData *, const char *);
typedef void TableSave_F (action_t);
typedef bool TableRW_F (action_t, void *, const char **);
typedef const char *TableShow_F (CharData *, void *);
typedef void Nanny_F (Descriptor *, const char *);


#ifndef DISABLE_WEBSRV
typedef struct web_descriptor WebDescriptor;

typedef bool Web_F (WebDescriptor *);

#define Web_Fun(fun)    	bool fun (WebDescriptor *wdesc)
#endif

#endif
