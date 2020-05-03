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

#ifndef __STRUCTS_H_
#define __STRUCTS_H_    	1

#define LinkList(type, name) type * name##_first, * name##_last
#define LinkNext(type) type *next, *prev
#define LinkNextName(type, name) type *next_##name, *prev_##name
#define CheckValid    bool valid


struct ban_data
{
  LinkNext (BanData);
  CheckValid;
  flag_t ban_flags;
  int level;
  const char *name;
};

struct buf_type
{
  LinkNext (Buffer);
  CheckValid;
  buf_t state;
  int size;
  char *string;
};

struct auction_data
{
  LinkNext (AuctionData);
  ObjData *item;
  CharData *owner;
  CharData *high_bidder;
  int status;
  int number;
  auc_t type;
  money_t bid;
    CheckValid;
};


struct time_data
{
  int hour;
  int day;
  int month;
  int year;
  sun_t sunlight;
};

struct weather_data
{
  int temp;
  int precip;
  int wind;
  int temp_vector;
  int precip_vector;
  int wind_vector;
  int climate_temp;
  int climate_precip;
  int climate_wind;
  const char *echo;
  const char *echo_color;
};


struct descriptor_data
{
  LinkNext (Descriptor);
  Descriptor *snoop_by;
  CharData *character;
  CharData *original;
    CheckValid;
  const char *host;
  unsigned long ip;
  int port;
  SOCKET descriptor;
  connect_t connected;
  time_t connect_time;
  bool fcommand;
  bool fPrompt;
  char inbuf[4 * MAX_INPUT_LENGTH];
  char incomm[MAX_INPUT_LENGTH];
  char inlast[MAX_INPUT_LENGTH];
  int repeat;
  char *outbuf;
  int outsize;
  int outtop;
  const char *showstr_head;
  const char *showstr_point;
  void *pEdit;
  const char **pString;
  ed_t editor;
  flag_t desc_flags;
  unsigned int scr_width;
  unsigned int scr_height;
#ifndef DISABLE_MCCP

  z_stream *out_compress;
  unsigned char *out_compress_buf;
  size_t bytes_compressed;
  int mccp_version;
#endif

  size_t bytes_normal;
  const char *run_buf;
  const char *run_head;
  char ttype[60];
  struct
  {
    const char *supports;
    float mxp_ver;
    float client_ver;
    float style_ver;
    const char *client;
    int registered;
    flag_t flags;
    flag_t flags2;
  }
  mxp;
  struct
  {
    unsigned int keycode;
    char version[20];
  }
  portal;
  double imp_vers;
  double pueblo_vers;
#ifdef __cplusplus

  int wrap (const char *buf);
#endif

  int wrong_password_count;
};


struct str_app_type
{
  int tohit;
  int todam;
  int carry;
  int wield;
};

struct int_app_type
{
  int learn;
};

struct wis_app_type
{
  int practice;
};

struct dex_app_type
{
  int defensive;
};

struct con_app_type
{
  int hitp;
  int shock;
};


struct help_data
{
  LinkNext (HelpData);
  CheckValid;
  int level;
  const char *keyword;
  const char *text;
  help_t category;
};

struct shop_data
{
  LinkNext (ShopData);
  CheckValid;
  vnum_t keeper;
  int buy_type[MAX_TRADE];
  int profit_buy;
  int profit_sell;
  int open_hour;
  int close_hour;
};

struct class_type
{
  const char *name[MAX_REMORT];
  const char *description;
  int *index;
  int attr_prime;
  vnum_t weapon;
  int skill_adept;
  int thac0_00;
  int thac0_32;
  int hp_min;
  int hp_max;
  bool fMana;
  const char *base_group;
  const char *default_group;
};

struct weapon_type
{
  const char *name;
  vnum_t vnum;
  weapon_t type;
  int *gsn;
};

struct wiznet_type
{
  const char *name;
  flag_t flag;
  int level;
};

struct attack_type
{
  const char *name;
  const char *noun;
  dam_class damage;
};

struct deity_type
{
  const char *name;
  const char *desc;
  const char *skillname;
  ethos_t ethos;
    LinkNext (DeityData);
    CheckValid;
};

struct corpse_data
{
  LinkNext (CorpseData);
  CheckValid;
  ObjData *corpse;
};

struct race_type
{
  const char *name;
  const char *description;
  bool pc_race;
  flag_t act;
  flag_t aff;
  flag_t off;
  flag_t imm;
  flag_t res;
  flag_t vuln;
  flag_t form;
  flag_t parts;
  int points;
  int *class_mult;
  const char *skills[MAX_RACE_SKILL];
  int stats[STAT_MAX];
  int max_stats[STAT_MAX];
  size_type size;
    LinkNext (RaceData);
    CheckValid;
};

struct spec_type
{
  const char *name;
  Spec_F *function;
};



struct board_data
{
  const char *short_name;
  const char *long_name;

  int read_level;
  int write_level;

  const char *names;
  board_def force_type;

  int purge_days;


    LinkList (NoteData, note);

  flag_t flags;

};


struct note_data
{
  LinkNext (NoteData);
  CheckValid;
  int type;
  const char *sender;
  const char *date;
  const char *to_list;
  const char *subject;
  const char *text;
  const char *reply_text;
  time_t date_stamp;
  time_t expire;
};

struct channel_type
{
  int *index;
  channel_types spec_flag;
  flag_t bit;
  const char *format;
  const char *color;
  int custom_color;
  int page_length;
  const char *name;
  const char *description;
};

struct gquest_data
{
  LinkNext (GqData);
  CheckValid;
  CharData *ch;
  vnum_t *gq_mobs;
};

struct gquest_type
{
  vnum_t *mobs;
  const char *who;
  int mob_count;
  int timer;
  int involved;
  int qpoints;
  money_t gold;
  int minlevel;
  int maxlevel;
  gquest_t running;
  int cost;
};


struct affect_data
{
  LinkNext (AffectData);
  CheckValid;
  where_t where;
  int type;
  int level;
  int duration;
  apply_t location;
  int modifier;
  flag_t bitvector;
};


struct crs_data
{
  crs_t status;
  long timer;
  const char *who;
  const char *reason;
};


struct kill_data
{
  int number;
  int kills;
  int deaths;
};

struct mud_stats
{
  long logins;
  long quests;
  long qcomplete;
  long levels;
  long newbies;
  long deletions;
  long mobdeaths;
  long auctions;
  long aucsold;
  long pdied;
  long pkill;
  long notes;
  long remorts;
  long wars;
  long gquests;
  long connections;
  long boot_connects;
  int online;
  long web_requests;
  long chan_msgs;
  time_t lastupdate;
  int version;
};

struct bonus_data
{
  int mod;
  int time;
  const char *msg;
  bonus_t status;
};

struct mud_data
{
  time_t last_copyover;
  time_t longest_uptime;
  struct mud_stats stats;
  flag_t mud_flags;
  arena_t arena;
  int share_value;
  int pulsepersec;

  int rand_factor;
  int weath_unit;
  int max_vector;
  int climate_factor;
  const char *name;
  int unique_id;
  int default_port, webport_offset;
  const char *login_prompt;
  const char *bind_ip_address;
  int min_save_lvl;
  struct bonus_data bonus;
  int group_lvl_limit;
  int mobdam;
  int pcdam;
  flag_t disabled_signals;
  int max_points;
  flag_t cmdline_options;
};

struct war_data
{
  LinkNext (WarData);
  CheckValid;
  CharData *ch, *owner;
  long hit, mana, move;
  flag_t flags;
  int Class;
};

struct war_type
{
  WarData *first, *last;
  const char *who;
  int min_level;
  int max_level;
  int inwar;
  war_types wartype;
  int timer;
  war_t status;
};

struct wpwd_data
{
  LinkNext (WebpassData);
  CheckValid;
  const char *name;
  const char *passw;
  int level;
};


struct char_index
{
  LinkNext (CharIndex);
  CheckValid;
  Spec_F *spec_fun;
  ShopData *pShop;
    LinkList (ProgList, mprog);
  AreaData *area;
  vnum_t vnum;
  vnum_t group;
  bool new_format;
  int count;
  long kills;
  long deaths;
  const char *player_name;
  const char *short_descr;
  const char *long_descr;
  const char *description;
  flag_t act;
  flag_t affected_by;
  int alignment;
  int level;
  int hitroll;
  int hit[DICE_MAX];
  int mana[DICE_MAX];
  int damage[DICE_MAX];
  int ac[MAX_AC];
  int dam_type;
  int autoset;
  int random;
  flag_t off_flags;
  flag_t imm_flags;
  flag_t res_flags;
  flag_t vuln_flags;
  position_t start_pos;
  position_t default_pos;
  sex_t sex;
  RaceData *race;
  money_t wealth;
  flag_t form;
  flag_t parts;
  size_type size;
  const char *material;
  flag_t mprog_flags;
};

struct stat_data
{
  LinkNext (StatData);
  CheckValid;
  const char *name;
  long gamestat[MAX_GAMESTAT];
  int version;
};

struct mbr_data
{
  LinkNext (ClanMember);
  CheckValid;
  const char *name;
  int rank;
  ClanData *clan;
  int level;
  money_t gold_donated;
  int qp_donated;
};


struct char_data
{
  LinkNext (CharData);
  LinkNextName (CharData, player);
  LinkNextName (CharData, in_room);
  CharData *master;
  CharData *leader;
  CharData *fighting;
  CharData *reply;
  CharData *pet;
  CharData *mprog_target;
  Spec_F *spec_fun;
  CharIndex *pIndexData;
  Descriptor *desc;
    LinkList (AffectData, affect);
    LinkList (ObjData, carrying);
  ObjData *on;
  RoomIndex *in_room;
  RoomIndex *was_in_room;
  AreaData *zone;
  PcData *pcdata;
  GenData *gen_data;
  CharData *hunting;
  WarData *war;
  GqData *gquest;
    CheckValid;
  const char *name;
  long id;
  int version;
  const char *short_descr;
  const char *long_descr;
  const char *description;
  const char *prompt;
  const char *gprompt;
  const char *prefix;
  int group;
  sex_t sex;
  int rank;
  int Class[MAX_MCLASS];
  RaceData *race;
  int level;
  DeityData *deity;
  int trust;
  int lines;
  int columns;
  time_t logon;
  int timer;
  int wait;
  int daze;
  long hit;
  long max_hit;
  long mana;
  long max_mana;
  long move;
  long max_move;
  money_t gold;
  money_t silver;
  int exp;
  flag_t act;
  flag_t comm;
  flag_t wiznet;
  flag_t imm_flags;
  flag_t res_flags;
  flag_t vuln_flags;
  int invis_level;
  int incog_level;
  flag_t affected_by;
  position_t position;
  int practice;
  int train;
  int carry_weight;
  int carry_number;
  int saving_throw;
  int alignment;
  int hitroll;
  int damroll;
  int armor[MAX_AC];
  int wimpy;

  int perm_stat[STAT_MAX];
  int mod_stat[STAT_MAX];

  flag_t form;
  flag_t parts;
  size_type size;
  const char *material;

  flag_t off_flags;
  int damage[DICE_MAX];
  int dam_type;
  position_t start_pos;
  position_t default_pos;
  flag_t info_settings;
  int mprog_delay;
  int stance[MAX_STANCE];
};

struct colatt_type
{
  int at[CT_MAX];
};

struct quest_data
{
  CharData *mob, *giver;
  ObjData *obj;
  RoomIndex *room;
  int time;
  int points;
  quest_t status;
};


struct pc_data
{
  LinkNext (PcData);
  Buffer *buffer;
  int tells;
  int played;
  CharData *challenger;
  CharData *challenged;
  CharData *gladiator;
    CheckValid;
  const char *pwd;
  const char *bamfin;
  const char *bamfout;
  const char *title;
  const char *who_descr;
  const char *afk_msg;
  long perm_hit;
  long perm_mana;
  long perm_move;
  sex_t true_sex;
  int last_level;
  int condition[COND_MAX];
  int *learned;
  bool *group_known;
  int points;
  bool confirm_delete;
  BoardData *board;
  time_t last_note[MAX_BOARD];
  NoteData *in_progress, *last_read;
  bool unsubscribed[MAX_BOARD];
  const char *alias[MAX_ALIAS];
  const char *alias_sub[MAX_ALIAS];
  int security;
  colatt_t color[MAX_CUSTOM_COLOR];
  long gamestat[MAX_GAMESTAT];
  QuestData quest;
  int trivia;
  bool confirm_remort;
  bool stay_race;
  char explored[MAX_EXPLORE_HASH];
  long home_invite;
  vnum_t home[MAX_HOME_VNUMS];
  vnum_t home_key;
  vnum_t home_room;
  int awins;
  int alosses;
  money_t plr_wager;
  money_t gold_bank;
  int shares;
  const char *webpass;
  char str_ed_key;
  const char *buddies[MAX_BUDDY];
  const char ***history;
  int *history_index;
  int timezone;
  const char *ignore[MAX_IGNORE];
  flag_t ignore_flags[MAX_IGNORE];
  ClanData *clan, *invited;
  int prime_class;
#ifndef DISABLE_I3

  I3_CHARDATA *i3chardata;
#endif
  flag_t vt100;
  time_t backup;
};


struct gen_data
{
  LinkNext (GenData);
  CheckValid;
  bool *skill_chosen;
  bool *group_chosen;
  int points_chosen;
  int rerolls;
};

struct liq_type
{
  const char *liq_name;
  const char *liq_color;
  int liq_affect[5];
};


struct ed_data
{
  LinkNext (ExDescrData);
  CheckValid;
  const char *keyword;
  const char *description;
};


struct obj_index
{
  LinkNext (ObjIndex);
  CheckValid;
  LinkList (ExDescrData, ed);
  LinkList (AffectData, affect);
  LinkList (ProgList, oprog);
  AreaData *area;
  bool new_format;
  const char *name;
  const char *short_descr;
  const char *description;
  vnum_t vnum;
  int reset_num;
  const char *material;
  item_t item_type;
  flag_t extra_flags;
  flag_t wear_flags;
  int level;
  int condition;
  int count;
  int weight;
  money_t cost;
  long value[5];
  flag_t oprog_flags;
};


struct obj_data
{
  LinkNext (ObjData);
  LinkNextName (ObjData, content);
  LinkList (ObjData, content);
  ObjData *in_obj;
  ObjData *on;
  CharData *carried_by;
    LinkList (ExDescrData, ed);
    LinkList (AffectData, affect);
  ObjIndex *pIndexData;
  RoomIndex *in_room;
  CharData *oprog_target;
  int oprog_delay;
    CheckValid;
  bool enchanted;
  const char *owner;
  const char *name;
  const char *short_descr;
  const char *description;
  item_t item_type;
  flag_t extra_flags;
  flag_t wear_flags;
  wloc_t wear_loc;
  int weight;
  money_t cost;
  int level;
  int condition;
  const char *material;
  int timer;
  long value[5];
};


struct exit_data
{
  union
  {
    RoomIndex *to_room;
    vnum_t vnum;
  }
  u1;
  flag_t exit_info;
  vnum_t key;
  const char *keyword;
  const char *description;
    LinkNext (ExitData);
    CheckValid;
  flag_t rs_flags;
  int orig_door;
};




struct reset_data
{
  LinkNext (ResetData);
  CheckValid;
  char command;
  vnum_t arg1;
  int arg2;
  vnum_t arg3;
  int arg4;
};


struct area_data
{
  LinkNext (AreaData);
  AreaData *next_sort;
    CheckValid;
  const char *file_name;
  const char *name;
  const char *credits;
  const char *lvl_comment;
  const char *resetmsg;
  int version;
  int age;
  int nplayer;
  int min_level;
  int max_level;
  vnum_t min_vnum;
  vnum_t max_vnum;
  bool empty;
  const char *builders;
  int vnum;
  flag_t area_flags;
  int security;
  WeatherData weather;
  ClanData *clan;
  vnum_t recall;
  MspData *sound;
  long kills, deaths;
};


struct area_index
{
  AreaData *area;
  double percent;
  long stat;
};


struct room_index
{
  LinkNext (RoomIndex);
  CheckValid;
  LinkList (CharData, person);
  LinkList (ObjData, content);
  LinkList (ExDescrData, ed);
  AreaData *area;
  MspData *sound;
  ExitData *exit[MAX_DIR];
    LinkList (ResetData, reset);
    LinkList (ProgList, rprog);
  CharData *rprog_target;
  flag_t rprog_flags;
  int rprog_delay;
  const char *name;
  const char *description;
  const char *owner;
  vnum_t vnum;
  flag_t room_flags;
  int light;
  sector_t sector_type;
  int heal_rate;
  int mana_rate;
  int guild;
};


struct disabled_data
{
  LinkNext (DisabledData);
  CheckValid;
  CmdData *command;
  const char *disabled_by;
  const char *disabled_for;
  int level;
};


struct skill_type
{
  const char *name;
  int *skill_level;
  int *rating;
  Spell_F *spell_fun;
  tar_t target;
  position_t minimum_position;
  int *pgsn;
  int min_mana;
  int beats;
  const char *noun_damage;
  const char *msg_off;
  const char *msg_obj;
  MspData *sound;
};

struct group_type
{
  const char *name;
  int *rating;
  const char *spells[MAX_IN_GROUP];
};

#ifdef STFILEIO

struct st_data
{
  LinkNext (stFile);
  CheckValid;
  char *str;
  size_t pos, size;
  FILE *stream;
#if !defined WIN32 && !defined __CYGWIN__
  const char *temp;
#endif

  const char *file, *mode;
};

#endif

struct prog_list
{
  flag_t trig_type;
  const char *trig_phrase;
  ProgCode *prog;
    LinkNext (ProgList);
    CheckValid;
};

struct prog_code
{
  vnum_t vnum;
  const char *name;
  const char *code;
  AreaData *area;
    LinkNext (ProgCode);
    CheckValid;
  bool disabled;
  const char *disabled_text;
};



struct social_type
{
  const char *name;
  const char *char_no_arg;
  const char *others_no_arg;
  const char *char_found;
  const char *others_found;
  const char *vict_found;
  const char *char_not_found;
  const char *char_auto;
  const char *others_auto;
  SocialData *next_hash;
    LinkNext (SocialData);
    CheckValid;
};

struct crash_type
{
  Descriptor *desc;
  char shrt_cmd[MAX_INPUT_LENGTH];
  char long_cmd[MAX_INPUT_LENGTH];

  crash_status status;
  crash_t crashed;
};

struct clan_rank
{
  const char *rankname;
};

struct clan_type
{
  const char *name;
  const char *who_name;
  const char *description;
  vnum_t rooms[MAX_CLAN_ROOM];
  flag_t flags;
  ethos_t ethos;
  ClanRank rank[MAX_RANK];
  money_t gold;
  int questpoints;
    LinkNext (ClanData);
    CheckValid;
};


struct cmd_type
{
  const char *name;
  Do_F *do_fun;
  int position;
  int level;
  log_t log;
  cmd_cat category;
  flag_t flags;
  CmdData *next_hash, *next_sort;
    LinkNext (CmdData);
    CheckValid;
};

struct song_data
{
  const char *group;
  const char *name;
  const char *lyrics[MAX_LINES];
  int lines;
};

struct datatable_type
{
  const char *field;
  field_t type;
  void *argument;
  const void *arg1;
  const void *arg2;
  Ed_F *function;
  const void *arg3;
};

struct olc_ed_type
{
  const char *name;
  const char *longname;
  ed_t ed;
  void *type_zero;
  Do_F *do_fun;
  DataTable *table;
  Olc_F *create_fun;
  Olc_F *delete_fun;
  Olc_F *list_fun;
  TableSave_F *save_fun;
  const void *vnum;
  field_t field;
};

#ifndef DISABLE_WEBSRV
struct web_descriptor
{
  LinkNext (WebDescriptor);
  CheckValid;
  SOCKET fd;
  char request[MIL], baseurl[MIL];
  const char *stuff, *path, *buf;
  int their_addr;
};
#endif

struct pfile_list
{
  const char **names;
  int count;
};

#ifdef WIN32
struct dirent
{
  char d_name[1];
};
#endif

struct column_data
{
  bool newline;
  size_t col_pos;
  size_t line_len;
  size_t line_pos;
  size_t columns;
  size_t col_len;
  column_t type;
  void *to;
    LinkNext (Column);
    CheckValid;
};

struct msp_data
{
  const char *file;
  msp_t type;
  int volume;
  int loop;
  int priority;
  bool restart;
  const char *url;
  flag_t to;
    LinkNext (MspData);
    CheckValid;
};

#define MAX_PARTS       3

struct name_profile
{
  const char *title;
  const char *part[MAX_PARTS];
  int part_count[MAX_PARTS];
  bool valid;
    LinkNext (NameProfile);
};

struct name_list
{
  const char *name;
    LinkNext (NameList);
  bool valid;
};


struct wiz_data
{
  LinkNext (WizData);
  int level;
  const char *name;
  const char *email;
  const char *jobs;
  bool valid;
};

#endif
