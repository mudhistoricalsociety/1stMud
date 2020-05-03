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


#ifndef __PROTO_H_
#define __PROTO_H_    	1


Proto (void check_sex, (CharData *));
Proto (void add_follower, (CharData *, CharData *));
Proto (void stop_follower, (CharData *));
Proto (void nuke_pets, (CharData *));
Proto (void die_follower, (CharData *));

Proto (bool is_same_group, (CharData *, CharData *));
Declare_Lookup_F (stance_lookup);
Declare_Lookup_F (color_lookup);
Proto (void default_color, (CharData *, int));
Proto (int random_color, (int));
Proto (void convert_random, (colatt_t *));
Proto (void show_greeting, (Descriptor *));

Proto (size_t line_count, (const char *));

Proto (bool convert_color_mxp_tags, (Descriptor *));
Proto (void set_col_attr, (char, colatt_t *, CharData *));
Proto (char *char_color, (CharData *, int));
Proto (void clear_screen, (CharData *));

Proto (const char *show_tilde, (const char *));
Proto (const char *hide_tilde, (const char *));

Proto (size_t cstrlen, (const char *));
Proto (const char *draw_line, (CharData *, char *, size_t));
Proto (const char *strip_color, (const char *));
ProtoF (const char *stringf, (CharData *, size_t, align_t, char *,
			      const char *, ...), 5, 6);

Proto (size_t skipcol, (const char *));
Proto (size_t skiprcol, (const char *));
Proto (size_t count_codes, (const char *, char));

Proto (bool is_ansi_printed_char, (char));
Proto (void public_ch, (const char *, CharData *, const char *, int));


Proto (RoomIndex * get_random_room, (CharData *));


Proto (void set_title, (CharData *, const char *));
Proto (char *format_who, (CharData *, CharData *));


Proto (void move_char, (CharData *, int, bool));


Proto (bool can_loot, (CharData *, ObjData *));
Proto (void wear_obj, (CharData *, ObjData *, bool));
Proto (void get_obj, (CharData *, ObjData *, ObjData *));
Proto (void recursive_clone, (CharData *, ObjData *, ObjData *));

Proto (bool obj_check, (CharData *, ObjData *));


ProtoF (void new_wiznet, (CharData *, const void *, flag_t,
			  bool, int, const char *, ...), 6, 7);

Proto (void copyover_recover, (void));
Proto (void copyover, (void));


Proto (void substitute_alias, (Descriptor *, char *));


Proto (bool check_ban, (const char *, int));
Proto (void ban_site, (CharData *, const char *));


Proto (void show_string, (Descriptor *, char *));
Proto (void close_socket, (Descriptor *));

Proto (int d_print, (Descriptor *, const char *));
Proto (int d_println, (Descriptor *, const char *));
ProtoF (int d_printf, (Descriptor *, const char *, ...), 2, 3);
ProtoF (int d_printlnf, (Descriptor *, const char *, ...), 2, 3);
Proto (int chprint, (CharData *, const char *));
Proto (int chprintln, (CharData *, const char *));
Proto (void sendpage, (CharData *, const char *));
Proto (void act_new, (const char *, CharData *, const void *,
		      const void *, flag_t, position_t));
Proto (void perform_act, (const char *, CharData *, const void *,
			  const void *, flag_t, CharData *));
Proto (const char *perform_act_string,
       (const char *, CharData *, const void *, const void *, bool));

ProtoF (int chprintf, (CharData *, const char *, ...), 2, 3);
ProtoF (int chprintlnf, (CharData *, const char *, ...), 2, 3);
ProtoF (void bugf, (const char *, ...), 1, 2);
Proto (void fix_sex, (CharData *));
ProtoF (void logf, (const char *, ...), 1, 2);
ProtoF (void vinterpret, (CharData *, const char *, const char *, ...), 2, 3);
ProtoF (int strswitch, (const char *, ...), 1, 2);


Proto (void reset_area, (AreaData *));
Proto (void reset_room, (RoomIndex *));
Proto (char *write_flags, (flag_t));
Proto (void boot_db, (void));
Proto (void area_update, (void));

Proto (CharData * create_mobile, (CharIndex *));
Proto (void clone_mobile, (CharData *, CharData *));

Proto (ObjData * create_object, (ObjIndex *, int));
Proto (void clone_object, (ObjData *, ObjData *));
Proto (const char *get_ed, (const char *, ExDescrData *));

Proto (CharIndex * get_char_index, (vnum_t));
Proto (ObjIndex * get_obj_index, (vnum_t));
Proto (RoomIndex * get_room_index, (vnum_t));
Proto (ProgCode * get_prog_index, (vnum_t, prog_t));

Proto (flag_t flag_convert, (char));
Proto (const char *str_dup, (const char *));
Proto (void free_string, (const char *));
Proto (int number_fuzzy, (int));
Proto (int number_fuzzier, (int));
Proto (int number_range, (int, int));
Proto (int number_percent, (void));
Proto (int number_door, (void));
Proto (int number_bits, (int));
Proto (long number_mm, (void));
Proto (int dice, (int, int));
Proto (int interpolate, (int, int, int));
Proto (int str_cmp, (const char *, const char *));
Proto (int str_casecmp, (const char *, const char *));
Proto (int str_ncmp, (const char *, const char *, size_t));
Proto (int str_ncasecmp, (const char *, const char *, size_t));

Proto (bool str_prefix, (const char *, const char *));
Proto (bool str_infix, (const char *, const char *));
Proto (bool str_suffix, (const char *, const char *));
Proto (char *capitalize, (const char *));
Proto (void append_file, (const char *, const char *, bool));
Proto (void bug, (const char *));
Proto (void log_string, (const char *));
Proto (void tail_chain, (void));

Proto (void fulltime_log_string, (const char *));
Proto (void log_bar, (void));
Proto (void log_note, (const char *));
ProtoF (void log_notef, (const char *, ...), 1, 2);

ProtoF (char *strcatf, (char *, const char *, ...), 2, 3);
ProtoF (char *strncatf, (char *, size_t, const char *, ...), 3, 4);
Proto (const char *print_area_levels, (AreaData *));
Proto (char *ordinal_string, (long));
ProtoF (void set_bonus, (bonus_t, int, int, const char *, ...), 4, 5);


Declare_Boot_F (load_mobiles);
Declare_Boot_F (load_objects);

Proto (void free_runbuf, (Descriptor *));

Proto (void convert_mob, (CharIndex *));
Proto (void convert_obj, (ObjIndex *));
Proto (void assign_area_vnum, (vnum_t));

Proto (void convert_mobile, (CharIndex *));
Proto (void convert_objects, (void));
Proto (void convert_object, (ObjIndex *));


Proto (void acid_effect, (void *, int, int, int));
Proto (void cold_effect, (void *, int, int, int));
Proto (void fire_effect, (void *, int, int, int));
Proto (void poison_effect, (void *, int, int, int));
Proto (void shock_effect, (void *, int, int, int));


Proto (bool is_safe, (CharData *, CharData *));
Proto (bool is_safe_spell, (CharData *, CharData *, bool));
Proto (void violence_update, (void));
Proto (void multi_hit, (CharData *, CharData *, int));

Proto (bool damage, (CharData *, CharData *, int, int, dam_class, bool));
Proto (void update_pos, (CharData *));
Proto (void stop_fighting, (CharData *, bool));
Proto (void check_killer, (CharData *, CharData *));


Proto (AffectData * affect_find, (AffectData *, int));
Proto (void affect_check, (CharData *, where_t, flag_t));
Proto (int count_users, (ObjData *));
Proto (void deduct_cost, (CharData *, money_t, currency_t));
Proto (void add_cost, (CharData *, money_t, currency_t));
Proto (char *cost_str, (money_t, currency_t));

Proto (bool check_worth, (CharData *, money_t, currency_t));
Proto (void affect_enchant, (ObjData *));

Proto (immune_t check_immune, (CharData *, dam_class));

Declare_Lookup_F (weapon_lookup);
Proto (weapon_t weapon_class, (const char *));
Proto (const char *weapon_name, (int));
Proto (const char *item_name, (int));

Declare_Lookup_F (attack_lookup);
Declare_Lookup_F (wiznet_lookup);
Declare_Lookup_F (class_lookup);
Declare_Lookup_F (channel_lookup);

Proto (SocialData * social_lookup, (const char *));

Proto (bool is_clan, (CharData *));
Proto (bool is_true_clan, (CharData *));
Proto (bool is_same_clan, (CharData *, CharData *));
Proto (bool is_old_mob, (CharData *));
Proto (int get_skill, (CharData *, int));
Proto (int get_weapon_sn, (CharData *));
Proto (int get_weapon_skill, (CharData *, int));
Proto (int get_age, (CharData *));
Proto (void reset_char, (CharData *));
Proto (int get_trust, (CharData *));
Proto (int get_curr_stat, (CharData *, int));
Proto (int get_max_train, (CharData *, int));
Proto (int can_carry_n, (CharData *));
Proto (int can_carry_w, (CharData *));

Proto (bool is_name, (const char *, const char *));
Proto (bool is_exact_name, (const char *, const char *));
Proto (void affect_to_char, (CharData *, AffectData *));
Proto (void affect_to_obj, (ObjData *, AffectData *));
Proto (void affect_remove, (CharData *, AffectData *));
Proto (void affect_remove_obj, (ObjData *, AffectData *));
Proto (void affect_strip, (CharData *, int));

Proto (bool is_affected, (CharData *, int));
Proto (void affect_join, (CharData *, AffectData *));
Proto (void char_from_room, (CharData *));
Proto (void char_to_room, (CharData *, RoomIndex *));
Proto (void obj_to_char, (ObjData *, CharData *));
Proto (void obj_from_char, (ObjData *));
Proto (int apply_ac, (ObjData *, wloc_t, int));

Proto (ObjData * get_eq_char, (CharData *, wloc_t));
Proto (void equip_char, (CharData *, ObjData *, wloc_t));
Proto (void unequip_char, (CharData *, ObjData *));
Proto (int count_obj_list, (ObjIndex *, ObjData *));
Proto (void obj_from_room, (ObjData *));
Proto (void obj_to_room, (ObjData *, RoomIndex *));
Proto (void obj_to_obj, (ObjData *, ObjData *));
Proto (void obj_from_obj, (ObjData *));
Proto (void extract_obj, (ObjData *));
Proto (void extract_char, (CharData *, bool));

Proto (CharData * get_char_room, (CharData *, RoomIndex *, const char *));
Proto (CharData * get_char_world, (CharData *, const char *));
Proto (ObjData * get_obj_type, (ObjIndex *));
Proto (ObjData * get_obj_list, (CharData *, const char *, ObjData *));
Proto (ObjData * get_obj_carry, (CharData *, const char *, CharData *));
Proto (ObjData * get_obj_wear, (CharData *, const char *, bool));
Proto (ObjData * get_obj_here, (CharData *, RoomIndex *, const char *));
Proto (ObjData * get_obj_world, (CharData *, const char *));
Proto (ObjData * create_money, (money_t, money_t));
Proto (int get_obj_number, (ObjData *));
Proto (int get_obj_weight, (ObjData *));
Proto (int get_true_weight, (ObjData *));

Proto (bool room_is_dark, (RoomIndex *));
Proto (bool is_room_owner, (CharData *, RoomIndex *));
Proto (bool room_is_private, (RoomIndex *));
Proto (bool can_see, (CharData *, CharData *));
Proto (bool can_see_obj, (CharData *, ObjData *));
Proto (bool can_see_room, (CharData *, RoomIndex *));
Proto (bool can_drop_obj, (CharData *, ObjData *));
Proto (bool is_friend, (CharData *, CharData *));
Proto (bool is_full_name, (const char *, const char *));
Proto (bool is_player_name, (const char *));
Proto (bool is_exact_player_name, (const char *));
Proto (const char *get_player_name, (const char *));

Proto (int add_qp, (CharData *, int));


Proto (void interpret, (CharData *, const char *));

Proto (bool is_number, (const char *));
Proto (unsigned int number_argument, (const char *, char *));
Proto (unsigned int mult_argument, (const char *, char *));
Proto (const char *one_argument, (const char *, char *));


Proto (int find_spell, (CharData *, const char *));
Proto (int mana_cost, (CharData *, int, int));

Declare_Lookup_F (skill_lookup);
Proto (int spell_lookup, (Spell_F *));

Proto (bool saves_spell, (int, CharData *, dam_class));
Proto (void obj_cast_spell, (int, int, CharData *, CharData *, ObjData *));

Proto (bool check_dispel, (int, CharData *, int));
Proto (bool saves_dispel, (int, int, int));


Proto (bool can_use_skpell, (CharData *, int));
Proto (bool has_spells, (CharData *));
Proto (bool is_class, (CharData *, int));
Proto (bool is_same_class, (CharData *, CharData *));
Proto (int number_classes, (CharData *));
Proto (char *class_long, (CharData *));
Proto (char *class_who, (CharData *));
Proto (char *class_short, (CharData *));
Proto (char *class_numbers, (CharData *));
Proto (int skill_level, (CharData *, int));
Proto (int skill_rating, (CharData *, int));
Proto (int group_rating, (CharData *, int));

Proto (bool check_base_group, (CharData *, int));
Proto (bool is_base_skill, (CharData *, int));
Proto (int get_hp_gain, (CharData *));
Proto (int get_mana_gain, (CharData *));

Proto (bool is_prime_stat, (CharData *, int));
Proto (void add_default_groups, (CharData *));
Proto (void add_base_groups, (CharData *));
Proto (int get_stat_bonus, (CharData *, int));
Proto (int get_thac00, (CharData *));
Proto (int get_thac32, (CharData *));
Proto (int get_hp_max, (CharData *));
Proto (int lvl_bonus, (CharData *));
Proto (int class_mult, (CharData *));

Proto (bool is_race_skill, (CharData *, int));
Proto (int hp_max, (CharData *));
Proto (int prime_class, (CharData *));


Proto (bool chance, (int));
ProtoF (void mob_tell, (CharData *, CharData *, const char *, ...), 3, 4);
Proto (void generate_quest, (CharData *, CharData *, quest_t));
Proto (void quest_update, (void));

Proto (bool quest_level_diff, (CharData *, CharData *));
Proto (void end_quest, (CharData *, int));

Proto (void update_questobj, (CharData *, ObjData *));

Proto (money_t obj_cost, (ObjData *));
Proto (int is_qobj, (ObjData *));
Proto (void add_apply,
       (ObjData *, apply_t, int, where_t, int, int, flag_t, int));
Proto (void update_all_qobjs, (CharData *));
Proto (void unfinished_quest, (CharData *));

Proto (CharData * find_quest_char, (CharData *, vnum_t));
Proto (ObjData * create_quest_obj, (CharData *, vnum_t));
Proto (void extract_quest, (CharData *));


Proto (void save_char_obj, (CharData *));
Proto (void backup_char_obj, (CharData *));
Proto (char *pfilename, (const char *, pfile_t));
Proto (const char *pfile_filename, (const char *));

Proto (bool save_char_obj_to_filename, (CharData *, const char *));
Proto (bool load_char_obj, (Descriptor *, const char *));
Proto (void update_corpses, (ObjData *, bool));
Proto (void load_corpses, (void));
Proto (void checkcorpse, (CharData *));


Proto (bool parse_gen_groups, (CharData *, const char *));
Proto (void list_group_costs, (CharData *));
Proto (void list_group_known, (CharData *));
Proto (int exp_per_level, (CharData *, int));
Proto (void check_improve, (CharData *, int, bool, int));

Declare_Lookup_F (group_lookup);
Proto (void gn_add, (CharData *, int));
Proto (void gn_remove, (CharData *, int));
Proto (void group_add, (CharData *, const char *, bool));
Proto (void group_remove, (CharData *, const char *));


Proto (void update_statlist, (CharData *, bool));
Proto (void show_game_stats, (CharData *, int));


Proto (RoomIndex * room_by_name, (char *, int, bool));


Proto (void advance_level, (CharData *, bool));
Proto (void gain_exp, (CharData *, int));
Proto (void gain_condition, (CharData *, int, int));
Proto (void update_handler, (void));


Proto (void string_edit, (CharData *, const char **));
Proto (void string_append, (CharData *, const char **));
Proto (const char *string_replace,
       (const char *, const char *, const char *));
Proto (const char *string_replace_all,
       (const char *, const char *, const char *));
Proto (void string_add, (CharData *, char *));
Proto (const char *format_string, (const char *oldstring));
Proto (const char *first_arg, (const char *, char *, bool));
Proto (const char *string_unpad, (const char *));
Proto (const char *string_proper, (const char *));

Proto (strshow_t parse_string_command,
       (const char **, const char *, CharData *));
Proto (void parse_action, (const char **, int, const char *, CharData *));


Proto (bool run_olc_editor, (Descriptor *, char *));
Proto (const char *olc_ed_name, (Descriptor *));
Proto (char *olc_ed_vnum, (Descriptor *));
Proto (void clean_area_links, (AreaData *));

Proto (bool check_vnum, (vnum_t, int));
Proto (bool vnum_OK, (vnum_t, vnum_t));


Proto (RaceData * race_lookup, (const char *));
Declare_Lookup_F (liq_lookup);

Proto (const char *print_ip, (unsigned long));


#ifndef DISABLE_MCCP
Proto (bool compressStart, (Descriptor *, int));
Proto (bool compressEnd, (Descriptor *, int));
Proto (bool compressContinue, (Descriptor *));
#endif

Proto (void hunt_victim, (CharData *));
ProtoF (void announce, (CharData *, flag_t, const char *, ...), 3, 4);


Proto (void auto_gquest, (void));

Proto (void end_gquest, (void));
Proto (void gquest_update, (void));

Proto (bool generate_gquest, (CharData *));
Proto (int count_gqmobs, (GqData *));
Proto (int is_gqmob, (GqData *, vnum_t));

Proto (bool is_random_gqmob, (vnum_t));


Proto (void read_rle, (char *, FileData *));
Proto (void write_rle, (char *, FileData *));
Proto (int arearooms, (AreaData *));
Proto (void update_explored, (CharData *));
Proto (int bitcount, (char));
Proto (int roomcount, (CharData *));
Proto (int areacount, (CharData *, AreaData *));

Proto (bool emptystring, (const char *));
Proto (void draw_map, (CharData *, const char *));


Proto (void war_channel, (CharData *, char *));
Proto (void war_update, (void));
Proto (void auto_war, (void));
Proto (void check_war, (CharData *, CharData *));

Proto (bool is_safe_war, (CharData *, CharData *));
Proto (void war_talk, (CharData *, const char *));
Proto (void end_war, (void));

Proto (bool abort_race_war, (void));
Proto (bool abort_class_war, (void));
Proto (bool abort_clan_war, (void));
Proto (void extract_war, (CharData *));


Proto (void update_wizlist, (CharData *, int));

Proto (CmdData * cmd_lookup, (const char *));

Proto (void delete_home, (CharData *));

Proto (void extract_auc, (CharData *));

Proto (AuctionData * auction_lookup, (int));
Proto (int get_auc_id, (void));

Proto (bool has_auction, (CharData *));
Proto (void reset_auc, (AuctionData *, bool));
Proto (int count_auc, (CharData *));
Proto (void auction_update, (void));

Proto (money_t advatoi, (const char *));

Proto (const char *get_sector_color, (sector_t));

Proto (void read_obj, (CharData *, FileData *, save_t));
Proto (void write_obj, (CharData *, ObjData *, FileData *, int, int, save_t));

Proto (void read_char, (CharData *, FileData *));
Proto (void pload_default, (CharData *));

Proto (void check_arena, (CharData *, CharData *));
Proto (void extract_arena, (CharData *));

Proto (bool IS_IN_ARENA, (CharData *));

Proto (char *prog_type_to_name, (flag_t));
Proto (void program_flow, (ProgList * program, CharData *, ObjData *,
			   RoomIndex *, CharData *,
			   const void *arg1, const void *));
Proto (void p_act_trigger,
       (const char *, CharData *, ObjData *, RoomIndex *,
	CharData *, const void *, const void *, flag_t));

Proto (bool p_percent_trigger,
       (CharData *, ObjData *, RoomIndex *, CharData *,
	const void *, const void *, flag_t));
Proto (void p_bribe_trigger, (CharData *, CharData *, money_t));

Proto (bool p_exit_trigger, (CharData *, int, prog_t));
Proto (void p_give_trigger, (CharData *, ObjData *, RoomIndex *,
			     CharData *, ObjData *, flag_t));
Proto (void p_greet_trigger, (CharData *, prog_t));
Proto (void p_hprct_trigger, (CharData *, CharData *));
Proto (void mob_interpret, (CharData *, const char *));
Proto (void obj_interpret, (ObjData *, const char *));
Proto (void room_interpret, (RoomIndex *, const char *));
Proto (void do_obj, (ObjData *, const char *));
Proto (void do_room, (RoomIndex *, const char *));

Proto (bool is_deity_skill, (CharData *, int));

Proto (void update_webpasses, (CharData *, bool));

Proto (int d_write, (Descriptor *, const char *, int));
Proto (int write_to_socket, (SOCKET, const char *, int));

Proto (int check_buddy, (CharData *, CharData *));

Proto (int srt_skills, (const void *, const void *));

Proto (void add_help, (HelpData *));
Proto (void add_area, (AreaData *));
Proto (void unlink_area, (AreaData *));

ProtoF (char *FORMATF, (const char *, ...), 1, 2);
Proto (void *alloc_perm, (size_t));

Proto (void update_members, (CharData *, bool));

Proto (void unlink_command, (CmdData *));
Proto (void add_command, (CmdData *));
Proto (void unhash_command, (CmdData *));
Proto (void hash_command, (CmdData *));

Proto (void unlink_social, (SocialData *));
Proto (void add_social, (SocialData *));
Proto (void unhash_social, (SocialData *));
Proto (void hash_social, (SocialData *));

Proto (SocialData * find_social, (const char *));

Proto (void set_on_off, (CharData *, flag_t *, flag_t, const char *,
			 const char *));
Proto (void print_on_off, (Column *, bool, const char *, const char *));
Proto (void print_all_on_off, (CharData *, FlagTable *, flag_t));

Proto (char *make_color, (CharData *, colatt_t *));

Proto (char *str_time, (time_t, int, const char *));

Proto (int calc_max_level, (CharData *));
Proto (const char *high_level_name, (int, bool));

Proto (void add_random_exit, (RoomIndex *, ResetData *, bool));

Declare_Lookup_F (get_direction);
Proto (int reset_door, (ResetData *, bool));

Proto (void crs_update, (void));
Proto (const char *format_pulse, (int));
Proto (const char *crs_sprintf, (bool, bool));

Proto (bool is_donate_room, (vnum_t));

Proto (bool read_from_descriptor, (Descriptor *));

Proto (bool bust_a_portal, (CharData *));
Proto (void portal_map, (CharData *, RoomIndex *));
Proto (void send_portal, (Descriptor *, const char *, ...));

Proto (void send_imp, (Descriptor *, char *));

Proto (void image_to_char, (CharData *, const char *));
Proto (void send_sound, (CharData *, MspData *));
Proto (void send_music, (CharData *, MspData *));
Proto (void act_sound, (MspData *, CharData *, const void *,
			flag_t, position_t));

Proto (size_t get_line_len, (char *, size_t));
Proto (char *reformat_desc, (char *));

#ifdef __cplusplus
#define dwrap(d, buf) (d)->wrap((buf))
#else
Proto (int dwrap, (Descriptor *, const char *));
#endif
Proto (int dwrapln, (Descriptor *, const char *));
ProtoF (int dwrapf, (Descriptor *, const char *, ...), 2, 3);
ProtoF (int dwraplnf, (Descriptor *, const char *, ...), 2, 3);

Proto (char *fix_string, (const char *));

Proto (void autodrop, (CharData *));

Proto (bool can_bypass, (CharData *, CharData *));
Proto (int dambonus, (CharData *, CharData *, int, int));

Proto (bool can_counter, (CharData *));
Proto (void improve_stance, (CharData *));
Proto (const char *stance_name, (int));

Proto (bool is_ignoring, (CharData *, const char *, flag_t));
Proto (void write_time, (FileData *, const char *, time_t, bool));
Proto (void log_error, (const char *));
ProtoF (void logf_error, (const char *, ...), 1, 2);
Proto (void sleep_seconds, (int));

Proto (void finish_note, (BoardData *, NoteData *));

Declare_Lookup_F (board_lookup);

Proto (bool is_note_to, (CharData *, NoteData *));
Proto (void personal_message, (const char *, const char *,
			       const char *, const int, const char *));
Proto (void make_note, (const char *, const char *, const char *,
			const char *, const int, const char *));

Proto (char *timestr, (time_t, bool));

Proto (int get_scr_cols, (CharData *));
Proto (int get_scr_lines, (CharData *));

ProtoF (bool file_exists, (const char *,...), 1, 2);
Proto (void cleanup_mud, (void));
Proto (void set_signals, (void));
Proto (void exit_mud, (void));
Proto (void halt_mud, (int));

#ifdef HAVE_SETITIMER
Proto (void set_vtimer, (long));
#endif

Proto (void song_update, (void));
Proto (void load_room_objs, (void));
Proto (void save_room_objs, (void));

Proto (ClanData * clan_lookup, (const char *));
Proto (HelpData * help_lookup, (const char *));
Proto (DeityData * deity_lookup, (const char *));
Declare_Lookup_F (tzone_lookup);
Declare_Lookup_F (song_lookup);
Proto (void show_flags, (CharData *, FlagTable *));

Proto (AreaData * area_lookup, (const char *));
Proto (char *get_obj_save_header, (save_t));
Proto (char *get_char_save_header, (save_t));

Proto (RoomIndex * area_begin, (AreaData *));
Proto (char *format_tabs, (int));

Proto (char *strupper, (const char *));
Proto (char *strlower, (const char *));

Proto (void replace_str, (const char **, const char *));
ProtoF (void replace_strf, (const char **, const char *, ...), 2, 3);
ProtoF (const char *str_dupf, (const char *, ...), 1, 2);
Proto (char *stristr, (const char *, const char *));
Proto (const char *str_rep, (const char *, const char *, const char *));
Proto (const char *stri_rep, (const char *, const char *, const char *));
Proto (const char *replines, (const char *));

Proto (CharData * get_char_vnum, (vnum_t));
Proto (char *aoran, (const char *));

Proto (bool isavowel, (char));
Proto (char *strupper, (const char *));
Proto (char *strlower, (const char *));
Proto (const char *Lower, (const char *));
Proto (const char *Upper, (const char *));

#ifndef HAVE_SCANDIR
Proto (int scandir, (const char *, struct dirent ***,
		     int (*select) (const struct dirent *),
		     int (*dcomp) (const struct dirent **,
				   const struct dirent **)));
Proto (int alphasort, (const struct dirent **, const struct dirent **));
#endif

#ifndef HAVE_FNMATCH
Proto (int fnmatch, (const char *, const char *, int));
#endif

#ifndef HAVE_INET_ATON
Proto (int inet_aton, (const char *, struct in_addr *));
#endif

#ifdef WIN32
Proto (const char *get_winsock_error_text, (int));
#endif

Proto (bool is_leader, (CharData *));
Proto (void perform_recall, (CharData *, RoomIndex *, const char *));
Proto (char *intstr, (long, const char *));
ProtoF (void cmd_syntax, (CharData *, const char *, const char *, ...), 2, 3);
Proto (const char *cmd_name, (Do_F *));

Proto (bool has_newline, (const char *));
Proto (time_t getcurrenttime, (void));
Proto (char *strip_cr, (const char *));

ProtoF (int print_cols, (Column *, const char *, ...), 3, 4);

Proto (bool cols_nl, (Column *));
Proto (void set_cols, (Column *, CharData *, int, column_t, void *));
Proto (int col_print, (Column *, const char *));

ProtoF (int cols_header, (Column *, const char *, ...), 3, 4);
Proto (char *get_platform_info, (void));

Proto (bool check_blind, (CharData *));
Proto (char *casemix, (const char *));
Proto (char *colorize, (const char *));
Proto (void cycle_log, (void));

#ifdef STFILEIO
Proto (int steof, (stFile *));
Proto (int stgetc, (stFile *));
Proto (int stungetc, (int, stFile *));
Proto (int stseek, (stFile *, long, int));

Proto (stFile * stopen, (const char *, const char *));
Proto (int stclose, (stFile *));
ProtoF (int stprintf, (stFile *, const char *, ...), 2, 3);
#endif

Proto (long read_long, (FileData *));

Proto (char read_letter, (FileData *));
Proto (int read_number, (FileData *));

Proto (flag_t read_flag, (FileData *));
Proto (const char *read_string, (FileData *));
Proto (void read_strfree, (FileData *, const char **));

Proto (const char *read_line, (FileData *));
Proto (void read_to_eol, (FileData *));
Proto (char *read_word, (FileData *));
Proto (void read_dice, (FileData *, int *));
Proto (void write_string,
       (FileData *, const char *, const char *, const char *));
Proto (void write_word,
       (FileData *, const char *, const char *, const char *));
Proto (void write_bit, (FileData *, const char *, flag_t, flag_t));
ProtoF (void f_writef, (FileData *, const char *, const char *, ...), 3, 4);
Proto (void write_header, (FileData *, const char *));

Proto (FILE * file_open, (const char *, const char *));
Proto (void file_close, (FILE *));
Proto (char *flagtable_names, (FlagTable *));

Proto (bool is_home_owner, (CharData *, RoomIndex *));
Proto (void write_sound, (FileData *, const char *, MspData *));

Proto (MspData * read_sound, (FileData *));
Proto (const char *bonus_name, (bonus_t));

Proto (void close_network, (void));

Proto (size_t strlcpy, (char *, const char *, size_t));
Proto (size_t strlcat, (char *, const char *, size_t));
Proto (const char *getarg, (const char *, char *, int));

Proto (bool hasname, (const char *, const char *));
Proto (void flagname, (const char **, const char *));
Proto (void unflagname, (const char **, const char *));

Proto (bool exists_player, (const char *));

Proto (void *rpl_malloc, (size_t));
Proto (void *rpl_realloc, (void *, size_t));

Proto (int ansi_skip, (const char *));
Proto (char *substr, (const char *, const char *, const char *));

Proto (bool socket_cntl, (SOCKET));

#ifndef DISABLE_WEBSRV
Proto (bool web_is_connected, (void));
Proto (void update_web_server, (void));
Proto (void shutdown_web_server, (void));
#endif

Proto (const char *say_verb, (const char *, CharData *, CharData *, int));
Proto (const char *swearcheck, (const char *, CharData *));

Proto (bool check_parse_name, (const char *));
Proto (const char *fsize, (size_t));
Proto (char *wartype_name, (int, bool));
Proto (const char *auc_type, (auc_t, bool));
Proto (char *genname, (NameProfile *));

#endif
