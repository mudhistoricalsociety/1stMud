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
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
*  This code was freely distributed with the The Isles 1.1 source code,   *
*  and has been used here for OLC - OLC would not be what it is without   *
*  all the previous coders who released their source code.                *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#ifndef __OLC_H_
#define __OLC_H_    	1


#define MAX_MOB    	1



EXTERN OlcTable olc_ed_table[ED_MAX];


Proto (AreaData * get_vnum_area, (vnum_t));
Proto (AreaData * get_area_data, (int));
Proto (ChannelData * get_chan_data, (int));
Proto (void add_reset, (RoomIndex *, ResetData *, int));
Proto (void display_resets, (CharData *));

Proto (bool oedit_values, (CharData *, const char *, int));
Proto (bool set_value, (CharData *, ObjIndex *, const char *, int));
Proto (bool set_obj_values, (CharData *, ObjIndex *, int, const char *));
Proto (const char *show_obj_values, (ObjIndex *));

Proto (bool change_exit, (const char *, CharData *, const char *, int));
Proto (bool check_range, (vnum_t, vnum_t));
Proto (void show_spec_cmds, (CharData *));
Proto (void show_skill_cmds, (CharData *, tar_t));

Proto (flag_t wear_bit, (wloc_t));
Proto (wloc_t wear_loc, (flag_t, int));
Proto (void save_area, (AreaData *));
Proto (void save_shops, (FileData *, AreaData *));
Proto (void save_resets, (FileData *, AreaData *));
Proto (void save_door_resets, (FileData *, AreaData *));
Proto (void save_specials, (FileData *, AreaData *));
Proto (void save_rooms, (FileData *, AreaData *));
Proto (void save_objects, (FileData *, AreaData *));
Proto (void save_object, (FileData *, ObjIndex *));
Proto (void save_mobiles, (FileData *, AreaData *));
Proto (void save_mobile, (FileData *, CharIndex *));
Proto (void save_mobprogs, (FileData *, AreaData *));
Proto (void save_area_list, (void));
Proto (void stop_editing, (void *));
Proto (int calc_avedam, (int, int));
Proto (void edit_start, (CharData *, void *, ed_t));

Proto (AreaData * get_olc_area, (Descriptor *));
Proto (bool olc_show, (CharData *, OlcTable *));
ProtoF (void olc_msg, (CharData *, const char *, const char *, ...), 3, 4);

Proto (OlcTable * olc_lookup, (ed_t));

struct olc_comm_type
{
  const char *name;
  Olc_F *fun;
};

EXTERN const struct olc_comm_type olc_comm_table[];


Declare_Olc_F (show_olc_cmds);
Declare_Olc_F (show_olc_help);
Proto (bool edit_done, (CharData *));
Declare_Olc_F (show_olc_version);

Declare_Ed_F (olced_str);
Declare_Ed_F (olced_desc);
Declare_Ed_F (olced_bool);
Declare_Ed_F (olced_flag);
Declare_Ed_F (olced_int);
Declare_Ed_F (olced_long);
Declare_Ed_F (olced_shop);
Declare_Ed_F (olced_spec);
Declare_Ed_F (olced_ac);
Declare_Ed_F (olced_dice);
Declare_Ed_F (olced_race);
Declare_Ed_F (olced_olded);
Declare_Ed_F (olced_ed);
Declare_Ed_F (olced_direction);
Declare_Ed_F (olced_docomm);
Declare_Ed_F (olced_value);
Declare_Ed_F (olced_plookup);
Declare_Ed_F (olced_nlookup);
Declare_Ed_F (olced_vnum);
Declare_Ed_F (olced_addprog);
Declare_Ed_F (olced_delprog);
Declare_Ed_F (olced_array);
Declare_Ed_F (olced_larray);
Declare_Ed_F (olced_sarray);
Declare_Ed_F (olced_farray);
Declare_Ed_F (olced_mclass);
Declare_Ed_F (olced_stance);
Declare_Ed_F (olced_stats);
Declare_Ed_F (olced_deity);
Declare_Ed_F (olced_getchar);
Declare_Ed_F (olced_time);
Declare_Ed_F (olced_clan);
Declare_Ed_F (olced_addaffect);
Declare_Ed_F (olced_delaffect);
Declare_Ed_F (olced_addapply);
Declare_Ed_F (olced_sound);

Declare_Validate_F (validate_align);
Declare_Validate_F (validate_level);
Declare_Validate_F (validate_autoset);
Declare_Validate_F (validate_groupname);

Declare_Validate_F (validate_weapon);
Declare_Validate_F (validate_adept);
Declare_Validate_F (validate_hmv);
Declare_Validate_F (validate_group);
Declare_Validate_F (validate_roomvnum);
Declare_Validate_F (validate_ip);
Declare_Validate_F (validate_port);
Declare_Validate_F (validate_keyword);


Declare_Olc_F (aedit_create);
Declare_Olc_F (aedit_delete);
Declare_Olc_F (aedit_list);
Declare_Olc_F (aedit_file);
Declare_Olc_F (aedit_reset);
Declare_Olc_F (aedit_builder);
Declare_Olc_F (aedit_vnum);
Declare_Olc_F (aedit_lvnum);
Declare_Olc_F (aedit_uvnum);
Declare_Olc_F (aedit_climate);
Declare_Olc_F (aedit_copy);


Declare_Olc_F (redit_create);
Declare_Olc_F (redit_mreset);
Declare_Olc_F (redit_oreset);
Declare_Olc_F (redit_mlist);
Declare_Olc_F (redit_list);
Declare_Olc_F (redit_olist);
Declare_Olc_F (redit_mshow);
Declare_Olc_F (redit_oshow);
Declare_Olc_F (redit_delete);
Declare_Olc_F (redit_format);
Declare_Olc_F (redit_copy);
Declare_Olc_F (redit_snake);


Declare_Olc_F (oedit_create);
Declare_Olc_F (oedit_delete);
Declare_Olc_F (oedit_autoweapon);
Declare_Olc_F (oedit_autoarmor);
Declare_Olc_F (oedit_list);
Declare_Olc_F (oedit_copy);


Declare_Olc_F (medit_create);
Declare_Olc_F (medit_delete);
Declare_Olc_F (medit_list);
Declare_Olc_F (medit_group);
Declare_Olc_F (medit_autoset);
Declare_Olc_F (medit_autoeasy);
Declare_Olc_F (medit_autohard);
Declare_Olc_F (medit_copy);


Declare_Olc_F (mpedit_create);
Declare_Olc_F (mpedit_list);
Declare_Olc_F (mpedit_delete);
Declare_Olc_F (mpedit_copy);

Declare_Olc_F (opedit_create);
Declare_Olc_F (opedit_list);
Declare_Olc_F (opedit_delete);
Declare_Olc_F (opedit_copy);

Declare_Olc_F (rpedit_create);
Declare_Olc_F (rpedit_list);
Declare_Olc_F (rpedit_delete);
Declare_Olc_F (rpedit_copy);


Declare_Olc_F (hedit_create);
Declare_Olc_F (hedit_delete);
Declare_Olc_F (hedit_list);
Declare_Olc_F (hedit_copy);
Declare_Olc_F (hedit_addkeywd);

Declare_Olc_F (sedit_name);
Declare_Olc_F (sedit_create);
Declare_Olc_F (sedit_delete);
Declare_Olc_F (sedit_list);

Declare_Olc_F (cedit_create);
Declare_Olc_F (cedit_delete);
Declare_Olc_F (cedit_list);
Declare_Olc_F (cedit_rank);

Declare_Olc_F (cmdedit_create);
Declare_Olc_F (cmdedit_delete);
Declare_Olc_F (cmdedit_list);
Declare_Olc_F (cmdedit_dofun);
Declare_Olc_F (cmdedit_rearrange);
Declare_Olc_F (cmdedit_name);
Declare_Olc_F (cmdedit_level);

Declare_Olc_F (gredit_create);
Declare_Olc_F (gredit_ratings);
Declare_Olc_F (gredit_spells);
Declare_Olc_F (gredit_delete);
Declare_Olc_F (gredit_list);

Declare_Olc_F (skedit_create);
Declare_Olc_F (skedit_levels);
Declare_Olc_F (skedit_ratings);
Declare_Olc_F (skedit_spellfun);
Declare_Olc_F (skedit_gsn);
Declare_Olc_F (skedit_delete);
Declare_Olc_F (skedit_list);

Declare_Olc_F (raedit_create);
Declare_Olc_F (raedit_delete);
Declare_Olc_F (raedit_list);
Declare_Olc_F (raedit_skills);
Declare_Olc_F (raedit_classx);
Declare_Olc_F (raedit_name);
Declare_Olc_F (raedit_copy);

Declare_Olc_F (cledit_create);
Declare_Olc_F (cledit_delete);
Declare_Olc_F (cledit_list);
Declare_Olc_F (cledit_prime);
Declare_Olc_F (cledit_weapon);
Declare_Olc_F (cledit_skill);
Declare_Olc_F (cledit_index);
Declare_Olc_F (cledit_name);

Declare_Olc_F (dedit_list);
Declare_Olc_F (dedit_delete);
Declare_Olc_F (dedit_create);

Declare_Olc_F (chanedit_create);
Declare_Olc_F (chanedit_gcn);
Declare_Olc_F (chanedit_delete);
Declare_Olc_F (chanedit_color);
Declare_Olc_F (chanedit_list);

Declare_Olc_F (mudedit_reset);

Declare_Olc_F (songedit_lyrics);
Declare_Olc_F (songedit_create);
Declare_Olc_F (songedit_delete);
Declare_Olc_F (songedit_list);


#define GetEdit(Ch, type, data)  ( (data) = (type *)(Ch)->desc->pEdit )
#define EditRoom(Ch, data) ( (data) = (Ch)->desc->pEdit ? (RoomIndex *) (Ch)->desc->pEdit : (Ch)->in_room)



Proto (void show_liqlist, (CharData *));
Proto (void show_damlist, (CharData *));

Proto (void autoset, (CharIndex *));
Proto (void autohard, (CharIndex *));
Proto (void autoeasy, (CharIndex *));
Proto (void autoarmor, (ObjIndex *));
Proto (void autoweapon, (ObjIndex *));

#endif
