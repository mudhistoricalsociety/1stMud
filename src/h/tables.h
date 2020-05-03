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


#ifndef __TABLES_H_
#define __TABLES_H_    	1


EXTERN const char *position_table[];


EXTERN FlagTable act_flags[];
EXTERN FlagTable plr_flags[];
EXTERN FlagTable affect_flags[];
EXTERN FlagTable off_flags[];
EXTERN FlagTable imm_flags[];
EXTERN FlagTable form_flags[];
EXTERN FlagTable part_flags[];
EXTERN FlagTable comm_flags[];
EXTERN FlagTable extra_flags[];
EXTERN FlagTable wear_flags[];
EXTERN FlagTable container_flags[];
EXTERN FlagTable portal_flags[];
EXTERN FlagTable room_flags[];
EXTERN FlagTable exit_flags[];
EXTERN FlagTable mprog_flags[];
EXTERN FlagTable oprog_flags[];
EXTERN FlagTable rprog_flags[];
EXTERN FlagTable area_flags[];
EXTERN FlagTable sector_flags[];
EXTERN FlagTable wear_loc_strings[];
EXTERN FlagTable wear_loc_flags[];
EXTERN FlagTable res_flags[];
EXTERN FlagTable vuln_flags[];
EXTERN FlagTable type_flags[];
EXTERN FlagTable apply_flags[];
EXTERN FlagTable sex_flags[];
EXTERN FlagTable furniture_flags[];
EXTERN FlagTable weapon_types[];
EXTERN FlagTable apply_types[];
EXTERN FlagTable weapon_flags[];
EXTERN FlagTable size_flags[];
EXTERN FlagTable position_flags[];
EXTERN FlagTable ac_type[];
EXTERN FlagTable autoset_types[];
EXTERN FlagTable board_flags[];
EXTERN FlagTable cmd_flags[];
EXTERN FlagTable to_flags[];
EXTERN const struct bit_type bitvector_type[];
EXTERN FlagTable color_attributes[];
EXTERN FlagTable color_foregrounds[];
EXTERN FlagTable color_backgrounds[];
EXTERN const struct color_type color_table[MAX_CUSTOM_COLOR];
EXTERN FlagTable desc_flags[];
EXTERN const struct vnum_type vnum_table[];
EXTERN FlagTable info_flags[];
EXTERN FlagTable log_flags[];
EXTERN const struct index_type spell_index[];
EXTERN const struct index_type gsn_index[];
EXTERN const struct index_type dofun_index[];
EXTERN const struct index_type channel_index[];
EXTERN const struct index_type class_index[];
EXTERN FlagTable target_flags[];
EXTERN FlagTable chan_types[];
EXTERN const struct tzone_type tzone_table[MAX_TZONE];
EXTERN FlagTable mud_flags[];
EXTERN FlagTable ignore_flags[];
EXTERN const struct directory_type directories_table[];
EXTERN const struct flag_stat_type flag_stat_table[];
EXTERN FlagTable stat_types[];
EXTERN const struct wear_type wear_table[];
EXTERN const struct sig_type sig_table[];
EXTERN const struct stance_type stance_table[];
EXTERN FlagTable clan_flags[];
EXTERN FlagTable signal_flags[];
EXTERN FlagTable cmd_categories[];
EXTERN FlagTable msp_types[];
EXTERN FlagTable help_types[];
EXTERN FlagTable vt100_flags[];
EXTERN FlagTable cmdline_flags[];
EXTERN FlagTable ethos_types[];

struct flag_type
{
  const char *name;
  flag_t bit;
  bool settable;
};

struct flag_stat_type
{
  const char *name;
  FlagTable *structure;
  bool stat;
};

struct bit_type
{
  FlagTable *table;
  const char *help;
};

struct color_type
{
  const char *name;
  int slot;
  int col_attr[CT_MAX];
};

struct vnum_type
{
  vnum_t vnum;
  int type;
};

struct spfun_type
{
  const char *name;
  Spell_F *fun;
};

struct tzone_type
{
  const char *name;
  const char *zone;
  time_t gmt_offset;
  time_t dst_offset;
};

struct index_type
{
  const char *name;
  void *index;
};

struct directory_type
{
  const char *text;
  const char *directory;
};

struct wear_type
{
  wloc_t wear_loc;
  flag_t wear_bit;
};

struct sig_type
{
  char *name;
  int sig;
    RETSIGTYPE (*sigfun) (int);
  int flags;
};

struct stance_type
{
  char *name;
  int stance;
  int prereq[2];
  char *chdrop;
  char *odrop;
};



#define VNUM_NONE   0
#define OBJ_VNUM    1
#define ROOM_VNUM   2
#define MOB_VNUM    3

Proto (flag_t flag_value, (FlagTable *, const char *));
Proto (FlagTable * flag_lookup, (const char *, FlagTable *));
Proto (const char *flag_string, (FlagTable *, flag_t));
Proto (void show_flag_cmds, (CharData *, FlagTable *));

Proto (int is_stat, (FlagTable *));

Proto (const char *index_name,
       (void *, const struct index_type *, const char *));
Proto (void *index_lookup, (const char *, const struct index_type *, void *));

#endif
