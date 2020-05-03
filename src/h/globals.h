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

#ifndef __GLOBALS_H_
#define __GLOBALS_H_    	1


GLOBAL_LIST (RaceData, race);
GLOBAL_LIST (SocialData, social);
GLOBAL_LIST (HelpData, help);
GLOBAL_LIST (ShopData, shop);
GLOBAL_LIST (CharData, char);

GLOBAL_LIST (Descriptor, descriptor);
GLOBAL_LIST (ObjData, obj);
GLOBAL_LIST (CharData, player);
GLOBAL_LIST (ProgCode, mprog);
GLOBAL_LIST (ProgCode, oprog);
GLOBAL_LIST (ProgCode, rprog);
GLOBAL_LIST (AuctionData, auction);
GLOBAL_LIST (StatData, stat);
GLOBAL_LIST (BanData, ban);
GLOBAL_LIST (DisabledData, disabled);
GLOBAL_LIST (DeityData, deity);
GLOBAL_LIST (WebpassData, wpwd);
GLOBAL_LIST (ClanMember, mbr);
GLOBAL_LIST (ClanData, clan);
GLOBAL_LIST (CmdData, cmd);
GLOBAL_LIST (CorpseData, corpse);
GLOBAL_LIST (AreaData, area);
GLOBAL_LIST (GqData, gqlist);
GLOBAL_LIST (WarData, warlist);
#ifdef STFILEIO
GLOBAL_LIST (stFile, stfile);
#endif
#ifndef DISABLE_WEBSRV
GLOBAL_LIST (WebDescriptor, webdesc);
#endif
GLOBAL_LIST (NameProfile, name_profile);
GLOBAL_LIST (WizData, wiz);


GLOBAL_TABLE (ClassData, class, table);
GLOBAL_TABLE (SkillData, skill, table);
GLOBAL_TABLE (GroupData, group, table);
GLOBAL_TABLE (ChannelData, channel, table);
GLOBAL_TABLE (SongData, song, table);
GLOBAL_TABLE (CharIndex, char_index, hash[MAX_KEY_HASH]);
GLOBAL_TABLE (ObjIndex, obj_index, hash[MAX_KEY_HASH]);
GLOBAL_TABLE (RoomIndex, room_index, hash[MAX_KEY_HASH]);


GLOBAL_INFO (TimeData, time);
GLOBAL_INFO (Gquest, gquest);
GLOBAL_INFO (War, war);
GLOBAL_INFO (Crs, crs);
GLOBAL_INFO (Mud, mud);


GLOBAL_STRUCT (PcData, pcdata);
GLOBAL_STRUCT (AffectData, affect);
GLOBAL_STRUCT (ExDescrData, ed);
GLOBAL_STRUCT (ExitData, exit);
GLOBAL_STRUCT (ResetData, reset);
GLOBAL_STRUCT (GenData, gen_data);
GLOBAL_STRUCT (Buffer, buf);
GLOBAL_STRUCT (ProgList, prog_list);
GLOBAL_STRUCT (NoteData, note);
GLOBAL_STRUCT (Column, column);
GLOBAL_STRUCT (MspData, msp);
GLOBAL_STRUCT (NameList, namelist);


GLOBAL_DEF (time_t current_time, 0);
GLOBAL_DEF (FILE * fpReserve, NULL);
GLOBAL_DEF (bool MOBtrigger, true);
GLOBAL_DEF (int newmobs, 0);
GLOBAL_DEF (int newobjs, 0);
GLOBAL_DEF (int nAllocString, 0);

GLOBAL_DEF (size_t sAllocString, 0);
GLOBAL_DEF (bool AttemptJump, false);
GLOBAL_DEF (RaceData * default_race, NULL);
GLOBAL_DEF (time_t boot_time, 0);
GLOBAL_DEF (int top_explored, 0);

GLOBAL_DEF (vnum_t top_vnum_mob, 0);
GLOBAL_DEF (vnum_t top_vnum_obj, 0);
GLOBAL_DEF (vnum_t top_vnum_room, 0);
GLOBAL_DEF (int mobile_count, 0);

GLOBAL_DEF (AreaData * area_first_sorted, NULL);
#ifndef DISABLE_WEBSRV
GLOBAL_DEF (SOCKET web_control, INVALID_SOCKET);
GLOBAL_DEF (int www_index, 0);
#endif
GLOBAL_DEF (SOCKET mud_control, INVALID_SOCKET);
GLOBAL_DEF (int telopt_lskip, 0);

GLOBAL_DEF (Descriptor * d_next, NULL);
GLOBAL_DEF (int mainport, 0);

#ifndef DISABLE_WEBSRV
GLOBAL_DEF (int webport, 0);
#endif

GLOBAL (KillData kill_table[MAX_LEVEL]);
GLOBAL (jmp_buf jump_env);
GLOBAL (char str_empty[1]);

GLOBAL (CmdData * command_hash[MAX_CMD_HASH]);
GLOBAL (SocialData * social_hash[MAX_SOCIAL_HASH]);
GLOBAL_DEF (CmdData * cmd_first_sorted, NULL);
GLOBAL (char HOSTNAME[KILOBYTE]);
GLOBAL (char UNAME[KILOBYTE]);
GLOBAL (char CWDIR[KILOBYTE]);
GLOBAL (char EXE_FILE[KILOBYTE]);
GLOBAL (const char *www_history[20]);
GLOBAL (struct pfile_list pfiles);
GLOBAL (const char *help_greeting[MAX_GREETING]);

GLOBAL (runlevel_t run_level);
GLOBAL (struct crash_type crash_info);


EXTERN char *const temp_settings[];
EXTERN char *const precip_settings[];
EXTERN char *const wind_settings[];
EXTERN char *const preciptemp_msg[6][6];
EXTERN char *const windtemp_msg[6][6];
EXTERN char *const precip_msg[];
EXTERN char *const wind_msg[];
EXTERN char *const dir_name[MAX_DIR];
EXTERN const int rev_dir[MAX_DIR];
EXTERN const int movement_loss[SECT_MAX];
EXTERN char *const he_she[];
EXTERN char *const him_her[];
EXTERN char *const his_her[];

EXTERN const struct str_app_type str_app[MAX_STATS + 1];
EXTERN const struct int_app_type int_app[MAX_STATS + 1];
EXTERN const struct wis_app_type wis_app[MAX_STATS + 1];
EXTERN const struct dex_app_type dex_app[MAX_STATS + 1];
EXTERN const struct con_app_type con_app[MAX_STATS + 1];
EXTERN const struct weapon_type weapon_table[];
EXTERN const struct wiznet_type wiznet_table[];
EXTERN const struct attack_type attack_table[];
EXTERN const struct spec_type spec_table[];
EXTERN const struct liq_type liq_table[];

EXTERN BoardData boards[MAX_BOARD];

EXTERN int channel_songs[MAX_GLOBAL + 1];
EXTERN int MAX_DAMAGE_MESSAGE;
EXTERN char *const runlvl_name[];

EXTERN char *const day_name[DAYS_IN_WEEK];
EXTERN char *const month_name[MONTHS_IN_YEAR];
EXTERN char *const where_name[];

#endif
