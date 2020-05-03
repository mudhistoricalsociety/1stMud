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

#ifndef __DEFINES_H_
#define __DEFINES_H_    	1

#define EXTERN extern

#define Declare_Do_F( fun )    	    	Do_F    fun
#define Declare_Spec_F( fun )    	    	Spec_F  fun
#define Declare_Spell_F( fun )    	    	Spell_F fun
#define Declare_Lookup_F( fun )    	    	Lookup_F fun
#define Declare_Ed_F( fun )    	    	Ed_F    	fun
#define Declare_Validate_F(fun)    	    	Validate_F fun
#define Declare_Obj_F( fun )    	    	Obj_F    	fun
#define Declare_Room_F( fun )    	    	Room_F  fun
#define Declare_TableRW_F(fun)    	    	TableRW_F fun
#define Declare_TableSave_F(fun)    	TableSave_F fun
#define Declare_TableShow_F(fun)    	TableShow_F fun
#define Declare_Olc_F( fun )    	    	Olc_F    fun
#define Declare_Boot_F( fun)    	    	Boot_F   fun
#define Declare_Nanny_F(fun)    	    	Nanny_F  fun

#define    	Do_Fun(name)            void name(const char *n_fun, CharData * ch, \
    	    	    	    	    	const char *argument)
#define Obj_Fun(name)    	    	void name(ObjData * obj, const char *argument)
#define Room_Fun(name)    	    	void name(RoomIndex * room, const char *argument)
#define Spec_Fun(fun)    	    	bool fun(CharData * ch)
#define Spell_Fun(fun)    	    	bool fun( int sn, int level, CharData * ch, \
    	    	    	    	    	void *vo, int target )
#define Ed_Fun(blah)    	    	bool blah ( DataTable *table, CharData *ch, void *base, \
    	    	    	    	    	const char *argument, void *data )
#define Validate_Fun(fun)    	bool fun(CharData *ch, const void *arg)
#define TableRW_Fun(fun)    	bool fun(action_t type, void *temp, const char **arg)
#define TableSave_Fun(fun)    	void fun(action_t type)
#define TableShow_Fun(fun)    	const char *fun(CharData *pch, void *temp)
#define Olc_Fun(fun)    	    	bool fun(const char *n_fun, CharData *ch, \
    	    	    	    	    	const char *argument)
#define Lookup_Fun(fun)    	    	int fun(const char *name)
#define Boot_Fun(fun)    	    	void fun(FileData *fp)
#define Nanny_Fun(fun)    	    	void fun(Descriptor *d, const char *argument)

#define    KILOBYTE                 (1024)
#define    MEGABYTE                 (1048576)
#define    GIGABYTE                 (1073741824)


#define    	MAX_KEY_HASH    	    	(KILOBYTE)
#define MAX_STRING_LENGTH    	(KILOBYTE * 8)
#define MAX_INPUT_LENGTH    	(KILOBYTE)
#define MAX_PRINTF_LENGTH    	(KILOBYTE * 25)
#define MAX_EXPLORE_HASH    	(KILOBYTE * 8)
#define MAX_SOCIAL_HASH    	    	('z' + 1)
#define MAX_CMD_HASH    	    	(126)

#define MSL    	    	    	MAX_STRING_LENGTH
#define MIL    	    	    	MAX_INPUT_LENGTH
#define MPL    	    	    	MAX_PRINTF_LENGTH


#ifndef PACKAGE_NAME
#define MUDNAME    	    	    	"1stMud"
#else
#define MUDNAME         PACKAGE_NAME
#endif

#ifndef PACKAGE_VERSION
#define MUDVERSION    	    	"4.5"
#else
#define MUDVERSION      PACKAGE_VERSION
#endif

#ifndef PACKAGE_STRING
#define MUDSTRING       MUDNAME MUDVERSION
#else
#define MUDSTRING       PACKAGE_STRING
#endif

#define MUDCREDITS    	    	"Markanth [markanth@firstmud.com]"

#ifndef TIME_T_FMT
#define TIME_T_FMT  "%ld"
#endif

#define SECOND    	    	    	   1L
#define MINUTE    	    	    	  (60L * SECOND)
#define HOUR    	    	    	  (60L * MINUTE)
#define DAY    	    	    	      (24L * HOUR)
#define WEEK    	    	    	  (7L * DAY)


#define MAX_IN_GROUP    	    	15
#define MAX_TZONE    	    	30
#define MAX_ALIAS    	    	10
#define MAX_BUDDY    	    	10
#define MAX_IGNORE    	    	10
#define MAX_GREETING    	2
#define MAX_REMORT    	    	2

#define CLASS_COUNT    	    	(MAX_REMORT + 1)

#define MAX_MCLASS    	    	(CLASS_COUNT + 1)
#define MAX_RANK    	    	6
#define MAX_VNUM    	    	60000
#define MAX_LEVEL    	    	60
#define LEVEL_IMMORTAL    	    	(MAX_LEVEL - 8)
#define MAX_MORTAL_LEVEL    	(MAX_LEVEL - 9)
#define LEVEL_HERO    	    	(MAX_MORTAL_LEVEL - MAX_REMORT)

#define    PC_HOME_COUNT              5
#define    PC_HOME_KEY                6
#define    PC_HOME_ROOM               7
#define    MAX_HOME_VNUMS             8
#define    HAS_HOME(ch)               ((ch)->pcdata->home[PC_HOME_COUNT])
#define    HOME_KEY(ch)               ((ch)->pcdata->home[PC_HOME_KEY])
#define    HOME_ROOM(ch)              ((ch)->pcdata->home[PC_HOME_ROOM])

#define PULSE_PER_SECOND    	  (mud_info.pulsepersec)
#define PULSE_VIOLENCE    	    	  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE    	    	  ( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC    	    	  ( 6 * PULSE_PER_SECOND)
#define PULSE_TICK    	    	  (45 * PULSE_PER_SECOND)
#define PULSE_AREA    	    	  (120 * PULSE_PER_SECOND)
#define PULSE_SENDSTAT    	    	  (10 * PULSE_PER_SECOND)
#define AUCTION_LENGTH    	    	  (65 * PULSE_PER_SECOND)


#define MAX_NEST    	100

#define IMPLEMENTOR    	    	  MAX_LEVEL
#define    	CREATOR    	    	    	 (MAX_LEVEL - 1)
#define SUPREME    	    	    	 (MAX_LEVEL - 2)
#define DEITY    	    	    	 (MAX_LEVEL - 3)
#define GOD    	    	    	 (MAX_LEVEL - 4)
#define IMMORTAL    	    	 (MAX_LEVEL - 5)
#define DEMI    	    	    	 (MAX_LEVEL - 6)
#define ANGEL    	    	    	 (MAX_LEVEL - 7)
#define AVATAR    	    	    	 (MAX_LEVEL - 8)
#define HERO    	    	    	 LEVEL_HERO

#define Season          	    	(((time_info.month / 4) > 3) ? 3 : (time_info.month / 4))


#define HOURS_IN_DAY	24
#define DAYS_IN_MONTH	30
#define MONTHS_IN_YEAR	17
#define DAYS_IN_WEEK	7

#define MAX_CLIMATE 5

#define MAX_LINES    	100
#define MAX_GLOBAL    	10


typedef enum
{
  BUFFER_SAFE,
  BUFFER_OVERFLOW,
  BUFFER_FREED
}
buf_t;

typedef enum
{
  COLS_BUF,
  COLS_CHAR,
  COLS_DESC
}
column_t;

typedef enum
{
  SUN_DARK,
  SUN_RISE,
  SUN_LIGHT,
  SUN_SET,
  SUN_NOON
}
sun_t;


typedef enum
{
  CON_GET_NAME,
  CON_GET_OLD_PASSWORD,
  CON_CONFIRM_NEW_NAME,
  CON_GET_NEW_PASSWORD,
  CON_CONFIRM_NEW_PASSWORD,
  CON_GET_NEW_RACE,
  CON_GET_NEW_SEX,
  CON_GET_NEW_CLASS,
  CON_GET_ALIGNMENT,
  CON_GET_DEITY,
  CON_GET_TIMEZONE,
  CON_GET_SCR_WIDTH,
  CON_CONFIRM_SCR_WIDTH,
  CON_GET_SCR_LINES,
  CON_DEFAULT_CHOICE,
  CON_GEN_GROUPS,
  CON_PICK_WEAPON,
  CON_READ_IMOTD,
  CON_READ_MOTD,
  CON_BREAK_CONNECT,
  CON_GET_TERM,
  CON_COPYOVER_RECOVER,
  CON_ROLL_STATS,
  CON_GET_WIZ,
  CON_PLAYING,
  CON_NOTE_TO,
  CON_NOTE_SUBJECT,
  CON_NOTE_EXPIRE,
  CON_NOTE_TEXT,
  CON_NOTE_FINISH
}
connect_t;


#define MAX_TRADE    	 5

#define MAX_RACE_SKILL 5

#define STAT_STR     	0
#define STAT_INT    	1
#define STAT_WIS    	2
#define STAT_DEX    	3
#define STAT_CON    	4
#define STAT_MAX    5

#define MAX_STATS   30

typedef enum
{
  GQUEST_OFF,
  GQUEST_WAITING,
  GQUEST_RUNNING
}
gquest_t;


typedef enum
{
  TO_AFFECTS,
  TO_OBJECT,
  TO_IMMUNE,
  TO_RESIST,
  TO_VULN,
  TO_WEAPON
}
where_t;

typedef enum
{
  CRS_NONE,
  CRS_REBOOT,
  CRS_SHUTDOWN,
  CRS_COPYOVER,
  CRS_COPYOVER_RECOVER
}
crs_t;


typedef enum
{
  DAM_NONE,
  DAM_BASH,
  DAM_PIERCE,
  DAM_SLASH,
  DAM_FIRE,
  DAM_COLD,
  DAM_LIGHTNING,
  DAM_ACID,
  DAM_POISON,
  DAM_NEGATIVE,
  DAM_HOLY,
  DAM_ENERGY,
  DAM_MENTAL,
  DAM_DISEASE,
  DAM_DROWNING,
  DAM_LIGHT,
  DAM_OTHER,
  DAM_HARM,
  DAM_CHARM,
  DAM_SOUND
}
dam_class;


typedef enum
{
  IMMUNE_NONE = -1,
  IS_NORMAL,
  IS_IMMUNE,
  IS_RESISTANT,
  IS_VULNERABLE
}
immune_t;


typedef enum
{
  SEX_NONE = -1,
  SEX_NEUTRAL,
  SEX_MALE,
  SEX_FEMALE,
  SEX_RANDOM
}
sex_t;


#define AC_PIERCE    	    	    	0
#define AC_BASH    	    	    	    	1
#define AC_SLASH    	    	    	2
#define AC_EXOTIC    	    	    	3
#define MAX_AC              4


#define DICE_NUMBER    	    	    	0
#define DICE_TYPE    	    	    	1
#define DICE_BONUS    	    	    	2
#define DICE_MAX            3


typedef enum
{
  SIZE_NONE = -1,
  SIZE_TINY,
  SIZE_SMALL,
  SIZE_MEDIUM,
  SIZE_LARGE,
  SIZE_HUGE,
  SIZE_GIANT
}
size_type;


typedef enum
{
  ITEM_NONE,
  ITEM_LIGHT,
  ITEM_SCROLL,
  ITEM_WAND,
  ITEM_STAFF,
  ITEM_WEAPON,
  ITEM_NONE6,
  ITEM_NONE7,
  ITEM_TREASURE,
  ITEM_ARMOR,
  ITEM_POTION,
  ITEM_CLOTHING,
  ITEM_FURNITURE,
  ITEM_TRASH,
  ITEM_NONE14,
  ITEM_CONTAINER,
  ITEM_NONE16,
  ITEM_DRINK_CON,
  ITEM_KEY,
  ITEM_FOOD,
  ITEM_MONEY,
  ITEM_NONE21,
  ITEM_BOAT,
  ITEM_CORPSE_NPC,
  ITEM_CORPSE_PC,
  ITEM_FOUNTAIN,
  ITEM_PILL,
  ITEM_PROTECT,
  ITEM_MAP,
  ITEM_PORTAL,
  ITEM_WARP_STONE,
  ITEM_ROOM_KEY,
  ITEM_GEM,
  ITEM_JEWELRY,
  ITEM_JUKEBOX
}
item_t;


typedef enum
{
  WEAPON_EXOTIC,
  WEAPON_SWORD,
  WEAPON_DAGGER,
  WEAPON_SPEAR,
  WEAPON_MACE,
  WEAPON_AXE,
  WEAPON_FLAIL,
  WEAPON_WHIP,
  WEAPON_POLEARM
}
weapon_t;


typedef enum
{
  APPLY_NONE,
  APPLY_STR,
  APPLY_DEX,
  APPLY_INT,
  APPLY_WIS,
  APPLY_CON,
  APPLY_SEX,
  APPLY_CLASS,
  APPLY_LEVEL,
  APPLY_AGE,
  APPLY_HEIGHT,
  APPLY_WEIGHT,
  APPLY_MANA,
  APPLY_HIT,
  APPLY_MOVE,
  APPLY_GOLD,
  APPLY_EXP,
  APPLY_AC,
  APPLY_HITROLL,
  APPLY_DAMROLL,
  APPLY_SAVES,
  APPLY_SAVING_PARA,
  APPLY_SAVING_ROD,
  APPLY_SAVING_PETRI,
  APPLY_SAVING_BREATH,
  APPLY_SAVING_SPELL,
  APPLY_SPELL_AFFECT
}
apply_t;


#define DIR_NORTH    	    	      0
#define DIR_EAST    	    	      1
#define DIR_SOUTH    	    	      2
#define DIR_WEST    	    	      3
#define DIR_UP    	    	    	      4
#define DIR_DOWN    	    	      5
#define MAX_DIR    	              6


typedef enum
{
  SECT_INSIDE,
  SECT_CITY,
  SECT_FIELD,
  SECT_FOREST,
  SECT_HILLS,
  SECT_MOUNTAIN,
  SECT_WATER_SWIM,
  SECT_WATER_NOSWIM,
  SECT_ICE,
  SECT_AIR,
  SECT_DESERT,
  SECT_ROAD,
  SECT_PATH,
  SECT_SWAMP,
  SECT_JUNGLE,
  SECT_CAVE,
  SECT_NONE,
  SECT_MAX
}
sector_t;


typedef enum
{
  WEAR_NONE = -1,
  WEAR_LIGHT,
  WEAR_FINGER_L,
  WEAR_FINGER_R,
  WEAR_NECK_1,
  WEAR_NECK_2,
  WEAR_BODY,
  WEAR_HEAD,
  WEAR_LEGS,
  WEAR_FEET,
  WEAR_HANDS,
  WEAR_ARMS,
  WEAR_SHIELD,
  WEAR_ABOUT,
  WEAR_WAIST,
  WEAR_WRIST_L,
  WEAR_WRIST_R,
  WEAR_WIELD,
  WEAR_HOLD,
  WEAR_FLOAT,
  WEAR_SECONDARY,
  MAX_WEAR
}
wloc_t;


#define COND_DRUNK    	    	      0
#define COND_FULL    	    	      1
#define COND_THIRST    	    	      2
#define COND_HUNGER    	    	      3
#define COND_MAX              4


typedef enum
{
  POS_DEAD,
  POS_MORTAL,
  POS_INCAP,
  POS_STUNNED,
  POS_SLEEPING,
  POS_RESTING,
  POS_SITTING,
  POS_FIGHTING,
  POS_STANDING,
  POS_NONE = -1
}
position_t;


#define STANCE_NONE                  -1
#define STANCE_NORMAL                 0
#define STANCE_VIPER                  1
#define STANCE_CRANE                  2
#define STANCE_CRAB                   3
#define STANCE_MONGOOSE               4
#define STANCE_BULL                   5
#define STANCE_MANTIS                 6
#define STANCE_DRAGON                 7
#define STANCE_TIGER                  8
#define STANCE_MONKEY                 9
#define STANCE_SWALLOW               10
#define STANCE_CURRENT               11
#define STANCE_AUTODROP              12
#define MAX_STANCE                   13

typedef enum
{
  WAR_OFF,
  WAR_WAITING,
  WAR_RUNNING
}
war_t;

#define PK_KILLS         0
#define MOB_KILLS        1
#define PK_DEATHS        2
#define MOB_DEATHS       3
#define MAX_GAMESTAT     4


typedef enum
{
  FIGHT_OPEN,
  FIGHT_START,
  FIGHT_BUSY,
  FIGHT_LOCK
}
arena_t;

typedef enum
{
  CHANNEL_NORMAL,
  CHANNEL_SOCIAL,
  CHANNEL_EMOTE,
  CHANNEL_THINK,
  CHANNEL_WHO
}
chanarg_t;


#define LIQ_WATER        0


#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000


typedef enum
{
  TAR_IGNORE,
  TAR_CHAR_OFFENSIVE,
  TAR_CHAR_DEFENSIVE,
  TAR_CHAR_SELF,
  TAR_OBJ_INV,
  TAR_OBJ_CHAR_DEF,
  TAR_OBJ_CHAR_OFF,
  TAR_NONE = -1
}
tar_t;

typedef enum
{
  TARGET_CHAR,
  TARGET_OBJ,
  TARGET_ROOM,
  TARGET_NONE
}
target_t;

typedef enum
{
  PRG_NONE,
  PRG_MPROG,
  PRG_OPROG,
  PRG_RPROG
}
prog_t;

#define NO_FLAG  -99

#define AREA_VERSION    4
#define PFILE_VERSION   12

typedef enum
{
  STRING_NONE,
  STRING_FOUND,
  STRING_END
}
strshow_t;

#define DEFAULT_SCR_WIDTH       80
#define DEFAULT_SCR_HEIGHT      25

typedef enum
{
  RUNLEVEL_INIT,
  RUNLEVEL_BOOTING,
  RUNLEVEL_SAFE_BOOT,
  RUNLEVEL_MAIN_LOOP,
  RUNLEVEL_SHUTDOWN
}
runlevel_t;

#define AUTODEFAULT 0
#define AUTOEASY    1
#define AUTOSET     2
#define AUTOHARD    3
#define AUTORANDOM  4

typedef enum
{ SAVE_NONE, SAVE_CHAR, SAVE_CORPSE, SAVE_ROOM,
  SAVE_PET, SAVE_MOB, SAVE_MAX
}
save_t;

typedef enum
{ AUC_GOLD, AUC_QUEST, AUC_TRIVIA }
auc_t;


typedef enum
{
  FIELD_STRING,
  FIELD_FUNCTION_INT_TO_STR,
  FIELD_LONG_ARRAY,
  FIELD_FLAGSTRING,
  FIELD_INT,
  FIELD_FLAGVECTOR,
  FIELD_BOOL,
  FIELD_INT_ARRAY,
  FIELD_STRING_ARRAY,
  FIELD_INT_FLAGSTRING,
  FIELD_BOOL_ARRAY,
  FIELD_INUTIL,
  FIELD_INT_ALLOC_ARRAY,
  FIELD_LONG,
  FIELD_STRING_ARRAY_NULL,
  FIELD_CLAN_RANK,
  FIELD_LONG_ALLOC_ARRAY,
  FIELD_STRING_SIZE,
  FIELD_CHAR,
  FIELD_INT_FUNCTION,
  FIELD_FLOAT,
  FIELD_DOUBLE,
  FIELD_TIME,
  FIELD_INT_FUNC_ARRAY,
  FIELD_CHARDATA,
  FIELD_SPEC_FUN,
  FIELD_MOBINDEX,
  FIELD_RACE,
  FIELD_DEITY,
  FIELD_AREA,
  FIELD_ROOM,
  FIELD_CLAN,
  FIELD_OBJ,
  FIELD_PCDATA,
  FIELD_DESC,
  FIELD_SHOP,
  FIELD_EXDESC,
  FIELD_RESET_DATA,
  FIELD_EXIT_DATA,
  FIELD_PROG_LIST,
  FIELD_OBJINDEX,
  FIELD_AFFECT,
  FIELD_FUNCTION,
  FIELD_DICE,
  FIELD_VNUM,
  FIELD_MONEY,
  FIELD_MSP,
  FIELD_NAMELIST
}
field_t;



typedef enum
{
  ED_AREA,
  ED_ROOM,
  ED_OBJECT,
  ED_MOBILE,
  ED_MPCODE,
  ED_OPCODE,
  ED_RPCODE,
  ED_HELP,
  ED_SOCIAL,
  ED_CLAN,
  ED_CMD,
  ED_SKILL,
  ED_GROUP,
  ED_RACE,
  ED_CLASS,
  ED_DEITY,
  ED_CHAN,
  ED_Mud,
  ED_SONG,
  ED_MAX,
  ED_NONE = -1,
}
ed_t;


typedef enum
{
  spec_public_flag,
  spec_clan_flag,
  spec_imm_flag,
  spec_buddy_flag,
  spec_none
}
channel_types;

typedef enum
{
  WAR_NONE,
  WAR_CLAN,
  WAR_RACE,
  WAR_CLASS,
  WAR_GENOCIDE,
  WAR_DEITY,
  WAR_SEX,
  MAX_WAR
}
war_types;

typedef enum
{
  Justify,
  Left,
  Center,
  Right
}
align_t;



typedef enum
{
  LOG_NORMAL,
  LOG_ALWAYS,
  LOG_NEVER
}
log_t;

typedef enum
{
  SYSLOG_STDERR,
  SYSLOG_FILE,
  SYSLOG_OFF
}
syslog_t;

#define END_MARK    "END"

typedef enum
{
  VALUE_SILVER,
  VALUE_GOLD,
  VALUE_DEFAULT = VALUE_SILVER
}
currency_t;

#define DEFAULT_PROMPT  "<{R%h/%Hhp {M%m/%M/m {B%v/%vmv{x>"

#define IMMORTAL_PROMPT DEFAULT_PROMPT " {m[{M%e{m]{x%c<{yG: {Y%g {wS: {W%s{x> {g[{G%r[{R%R{G], %z{g] {C%Qgq{W %o %O{x"

typedef enum
{
  BONUS_OFF,
  BONUS_XP,
  BONUS_QP,
  BONUS_DAM
}
bonus_t;

#define CLAN_ROOM_ENTRANCE    	0
#define CLAN_ROOM_MORGUE    	1
#define MAX_CLAN_ROOM    	    	2

typedef enum
{
  CMDCAT_NOSHOW,
  CMDCAT_NONE,
  CMDCAT_IMMORTAL,
  CMDCAT_COMBAT,
  CMDCAT_SETTINGS,
  CMDCAT_INFO,
  CMDCAT_MOVEMENT,
  CMDCAT_COMMUNICATION,
  CMDCAT_MISC,
  CMDCAT_OLC,
  CMDCAT_OBJECT,
  CMDCAT_CLAN,
  MAX_CMDCAT
}
cmd_cat;

typedef enum
{
  CRASH_UNLIKELY,
  CRASH_LIKELY,
  CRASH_PRE_PROCESSING,
  CRASH_POST_PROCESSING,
  CRASH_UPDATING,
  CRASH_UNKNOWN,
  CRASH_BOOT,
  CRASH_LOOPING
}
crash_status;

typedef enum
{
  MSP_NONE,
  MSP_COMBAT,
  MSP_WEATHER,
  MSP_SKILL
}
msp_t;

typedef enum
{
  HELP_UNKNOWN,
  HELP_CREATION,
  HELP_SPELLS,
  HELP_COMMANDS,
  HELP_NEWBIE,
  HELP_IMMORTAL,
  HELP_OLC,
  HELP_CLAN,
  HELP_MAX
}
help_t;

typedef enum
{
  QUEST_RETURN_FINDMOB = -5,
  QUEST_RETURN_FINDROOM = -4,
  QUEST_RETURN_DELIVER = -3,
  QUEST_RETURN_RETRIEVE = -2,
  QUEST_RETURN_KILL = -1,
  QUEST_NONE = 0,
  QUEST_KILL = 1,
  QUEST_RETRIEVE = 2,
  QUEST_DELIVER = 3,
  QUEST_FINDROOM = 4,
  QUEST_FINDMOB = 5
}
quest_t;

#define QUEST_TIME  20


typedef enum
{
  DEF_NORMAL,
  DEF_INCLUDE,
  DEF_EXCLUDE,
  DEF_READONLY
}
board_def;

#define MAX_BOARD     	  7

#define DEFAULT_BOARD 0


#define MAX_LINE_LENGTH 80


#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)

#define BOARD_NOTFOUND -1
#define BOARD_NOACCESS -1


typedef enum
{
  PFILE_NONE,
  PFILE_NORMAL,
  PFILE_BACKUP,
  PFILE_DELETED
}
pfile_t;

typedef enum
{
  ETHOS_LAWFUL_GOOD = 4,
  ETHOS_NEUTRAL_GOOD = 3,
  ETHOS_CHAOTIC_GOOD = 2,
  ETHOS_LAWFUL_NEUTRAL = 1,
  ETHOS_TRUE_NEUTRAL = 0,
  ETHOS_EVIL_NEUTRAL = -1,
  ETHOS_LAWFUL_EVIL = -2,
  ETHOS_NEUTRAL_EVIL = -3,
  ETHOS_CHAOTIC_EVIL = -4
}
ethos_t;

#endif
