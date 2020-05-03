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



#ifndef __SPECIAL_H_
#define __SPECIAL_H_    	1

Declare_Spec_F (spec_breath_any);
Declare_Spec_F (spec_breath_acid);
Declare_Spec_F (spec_breath_fire);
Declare_Spec_F (spec_breath_frost);
Declare_Spec_F (spec_breath_gas);
Declare_Spec_F (spec_breath_lightning);
Declare_Spec_F (spec_cast_adept);
Declare_Spec_F (spec_cast_cleric);
Declare_Spec_F (spec_cast_judge);
Declare_Spec_F (spec_cast_mage);
Declare_Spec_F (spec_cast_undead);
Declare_Spec_F (spec_executioner);
Declare_Spec_F (spec_fido);
Declare_Spec_F (spec_guard);
Declare_Spec_F (spec_janitor);
Declare_Spec_F (spec_mayor);
Declare_Spec_F (spec_poison);
Declare_Spec_F (spec_thief);
Declare_Spec_F (spec_nasty);
Declare_Spec_F (spec_troll_member);
Declare_Spec_F (spec_ogre_member);
Declare_Spec_F (spec_patrolman);
Declare_Spec_F (spec_questmaster);
Declare_Spec_F (spec_triviamob);
Declare_Spec_F (spec_registar);
Declare_Spec_F (spec_warmaster);

Proto (Spec_F * spec_lookup, (const char *));
Proto (const char *spec_name, (Spec_F *));




Declare_Nanny_F (HANDLE_CON_GET_TERM);
Declare_Nanny_F (HANDLE_CON_GET_NAME);
Declare_Nanny_F (HANDLE_CON_GET_OLD_PASSWORD);
Declare_Nanny_F (HANDLE_CON_GET_WIZ);
Declare_Nanny_F (HANDLE_CON_BREAK_CONNECT);
Declare_Nanny_F (HANDLE_CON_CONFIRM_NEW_NAME);
Declare_Nanny_F (HANDLE_CON_GET_NEW_PASSWORD);
Declare_Nanny_F (HANDLE_CON_CONFIRM_NEW_PASSWORD);
Declare_Nanny_F (HANDLE_CON_GET_NEW_RACE);
Declare_Nanny_F (HANDLE_CON_GET_NEW_SEX);
Declare_Nanny_F (HANDLE_CON_GET_NEW_CLASS);
Declare_Nanny_F (HANDLE_CON_GET_ALIGNMENT);
Declare_Nanny_F (HANDLE_CON_GET_DEITY);
Declare_Nanny_F (HANDLE_CON_GET_TIMEZONE);
Declare_Nanny_F (HANDLE_CON_GET_SCR_WIDTH);
Declare_Nanny_F (HANDLE_CON_CONFIRM_SCR_WIDTH);
Declare_Nanny_F (HANDLE_CON_GET_SCR_LINES);
Declare_Nanny_F (HANDLE_CON_ROLL_STATS);
Declare_Nanny_F (HANDLE_CON_DEFAULT_CHOICE);
Declare_Nanny_F (HANDLE_CON_PICK_WEAPON);
Declare_Nanny_F (HANDLE_CON_GEN_GROUPS);
Declare_Nanny_F (HANDLE_CON_READ_IMOTD);
Declare_Nanny_F (HANDLE_CON_NOTE_TO);
Declare_Nanny_F (HANDLE_CON_NOTE_SUBJECT);
Declare_Nanny_F (HANDLE_CON_NOTE_EXPIRE);
Declare_Nanny_F (HANDLE_CON_NOTE_TEXT);
Declare_Nanny_F (HANDLE_CON_NOTE_FINISH);
Declare_Nanny_F (HANDLE_CON_READ_MOTD);

struct nanny_type
{
  const char *name;
  connect_t state;
  Nanny_F *fun;
};

EXTERN const struct nanny_type nanny_table[];
Proto (int nanny_lookup, (connect_t));

#endif
