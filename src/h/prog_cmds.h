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
*  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
*  Written and adapted to ROM 2.4 by                                      *
*          Markku Nylander (markku.nylander@uta.fi)                       *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/




#ifndef __PROG_CMDS_H
#define __PROG_CMDS_H    	1

struct mob_cmd_type
{
  const char *name;
  Do_F *do_fun;
};

struct obj_cmd_type
{
  const char *name;
  Obj_F *obj_fun;
};

struct room_cmd_type
{
  const char *name;
  Room_F *room_fun;
};


extern const struct mob_cmd_type mob_cmd_table[];


Declare_Do_F (do_mpasound);
Declare_Do_F (do_mpgecho);
Declare_Do_F (do_mpzecho);
Declare_Do_F (do_mpkill);
Declare_Do_F (do_mpassist);
Declare_Do_F (do_mpjunk);
Declare_Do_F (do_mpechoaround);
Declare_Do_F (do_mpecho);
Declare_Do_F (do_mpechoat);
Declare_Do_F (do_mpmload);
Declare_Do_F (do_mpoload);
Declare_Do_F (do_mppurge);
Declare_Do_F (do_mpgoto);
Declare_Do_F (do_mpat);
Declare_Do_F (do_mptransfer);
Declare_Do_F (do_mpgtransfer);
Declare_Do_F (do_mpforce);
Declare_Do_F (do_mpgforce);
Declare_Do_F (do_mpvforce);
Declare_Do_F (do_mpcast);
Declare_Do_F (do_mpdamage);
Declare_Do_F (do_mpremember);
Declare_Do_F (do_mpforget);
Declare_Do_F (do_mpdelay);
Declare_Do_F (do_mpcancel);
Declare_Do_F (do_mpcall);
Declare_Do_F (do_mpflee);
Declare_Do_F (do_mpotransfer);
Declare_Do_F (do_mpremove);
Declare_Do_F (do_mppeace);


Declare_Obj_F (do_opgecho);
Declare_Obj_F (do_opzecho);
Declare_Obj_F (do_opecho);
Declare_Obj_F (do_opechoaround);
Declare_Obj_F (do_opechoat);
Declare_Obj_F (do_opmload);
Declare_Obj_F (do_opoload);
Declare_Obj_F (do_oppurge);
Declare_Obj_F (do_opgoto);
Declare_Obj_F (do_optransfer);
Declare_Obj_F (do_opgtransfer);
Declare_Obj_F (do_opotransfer);
Declare_Obj_F (do_opforce);
Declare_Obj_F (do_opgforce);
Declare_Obj_F (do_opvforce);
Declare_Obj_F (do_opdamage);
Declare_Obj_F (do_opremember);
Declare_Obj_F (do_opforget);
Declare_Obj_F (do_opdelay);
Declare_Obj_F (do_opcancel);
Declare_Obj_F (do_opcall);
Declare_Obj_F (do_opremove);
Declare_Obj_F (do_opattrib);
Declare_Obj_F (do_oppeace);


Declare_Room_F (do_rpasound);
Declare_Room_F (do_rpgecho);
Declare_Room_F (do_rpzecho);
Declare_Room_F (do_rpecho);
Declare_Room_F (do_rpechoaround);
Declare_Room_F (do_rpechoat);
Declare_Room_F (do_rpmload);
Declare_Room_F (do_rpoload);
Declare_Room_F (do_rppurge);
Declare_Room_F (do_rptransfer);
Declare_Room_F (do_rpgtransfer);
Declare_Room_F (do_rpotransfer);
Declare_Room_F (do_rpforce);
Declare_Room_F (do_rpgforce);
Declare_Room_F (do_rpvforce);
Declare_Room_F (do_rpdamage);
Declare_Room_F (do_rpremember);
Declare_Room_F (do_rpforget);
Declare_Room_F (do_rpdelay);
Declare_Room_F (do_rpcancel);
Declare_Room_F (do_rpcall);
Declare_Room_F (do_rpremove);
Declare_Room_F (do_rppeace);

#endif
