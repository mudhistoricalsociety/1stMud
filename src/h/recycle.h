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


#ifndef __RECYCLE_H_
#define __RECYCLE_H_    	1

#define Declare_Recycle(name, type) \
EXTERN  type * new_##name (void); \
EXTERN void free_##name (type *)

Declare_Recycle (ban, BanData);
Declare_Recycle (descriptor, Descriptor);
Declare_Recycle (gen_data, GenData);
Declare_Recycle (ed, ExDescrData);
Declare_Recycle (affect, AffectData);
Declare_Recycle (obj, ObjData);
Declare_Recycle (char, CharData);

Declare_Recycle (pcdata, PcData);
Declare_Recycle (buf, Buffer);

Proto (int bprint, (Buffer * buffer, const char *string));
ProtoF (int bprintf, (Buffer * buffer, const char *messg, ...), 2, 3);
ProtoF (int bprintlnf, (Buffer * buffer, const char *messg, ...), 2, 3);
Proto (int bprintln, (Buffer * buffer, const char *messg));
Proto (void clear_buf, (Buffer * buffer));
Proto (char *buf_string, (Buffer * buffer));

Proto (long get_mob_id, (void));
Proto (long get_pc_id, (void));

Declare_Recycle (help, HelpData);
Declare_Recycle (stat, StatData);
Declare_Recycle (corpse, CorpseData);
Declare_Recycle (auction, AuctionData);
Declare_Recycle (wpwd, WebpassData);
Declare_Recycle (mbr, ClanMember);
Declare_Recycle (race, RaceData);
Declare_Recycle (social, SocialData);
Declare_Recycle (deity, DeityData);
Declare_Recycle (clan, ClanData);
Declare_Recycle (cmd, CmdData);
Declare_Recycle (reset, ResetData);
Declare_Recycle (area, AreaData);
Declare_Recycle (exit, ExitData);
Declare_Recycle (room_index, RoomIndex);
Declare_Recycle (shop, ShopData);
Declare_Recycle (obj_index, ObjIndex);
Declare_Recycle (char_index, CharIndex);
Declare_Recycle (prog_list, ProgList);
Declare_Recycle (mprog, ProgCode);
Declare_Recycle (oprog, ProgCode);
Declare_Recycle (rprog, ProgCode);
Declare_Recycle (gqlist, GqData);
Declare_Recycle (warlist, WarData);
Declare_Recycle (disabled, DisabledData);
#ifdef STFILEIO
Declare_Recycle (stfile, stFile);
#endif
Declare_Recycle (note, NoteData);
#ifndef DISABLE_WEBSRV
Declare_Recycle (webdesc, WebDescriptor);
#endif
Declare_Recycle (column, Column);
Declare_Recycle (msp, MspData);
Declare_Recycle (name_profile, NameProfile);
Declare_Recycle (namelist, NameList);
Declare_Recycle (wiz, WizData);

#endif
