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


#undef VNUM_BEGIN
#undef VNUMBER
#undef VNUM_END

#ifdef MAKE_VNUM_TABLE
#define VNUM_BEGIN
#define VNUMBER(name, num, type) { num, type }
#define VNUM_END    {-1, -1}
#else
#define VNUM_BEGIN  enum {
#define VNUMBER(name, num, type) type##_##name = num
#define VNUM_END    };
#endif

// *INDENT-OFF*
VNUM_BEGIN
VNUMBER(FIDO, 3090, MOB_VNUM),
VNUMBER(CITYGUARD, 3060, MOB_VNUM),
VNUMBER(VAMPIRE, 3404, MOB_VNUM),
VNUMBER(WARMASTER, 17601, MOB_VNUM),
VNUMBER(PATROLMAN, 2106, MOB_VNUM),
VNUMBER(GROUP_TROLLS, 2100, MOB_VNUM),
VNUMBER(GROUP_OGRES, 2101, MOB_VNUM),
VNUMBER(REGISTAR, 202, MOB_VNUM),
VNUMBER(SILVER_ONE, 1, OBJ_VNUM),
VNUMBER(GOLD_ONE, 2, OBJ_VNUM),
VNUMBER(GOLD_SOME, 3, OBJ_VNUM),
VNUMBER(SILVER_SOME, 4, OBJ_VNUM),
VNUMBER(COINS, 5, OBJ_VNUM),
VNUMBER(CORPSE_NPC, 10, OBJ_VNUM),
VNUMBER(CORPSE_PC, 11, OBJ_VNUM),
VNUMBER(SEVERED_HEAD, 12, OBJ_VNUM),
VNUMBER(TORN_HEART, 13, OBJ_VNUM),
VNUMBER(SLICED_ARM, 14, OBJ_VNUM),
VNUMBER(SLICED_LEG, 15, OBJ_VNUM),
VNUMBER(GUTS, 16, OBJ_VNUM),
VNUMBER(BRAINS, 17, OBJ_VNUM),
VNUMBER(MUSHROOM, 20, OBJ_VNUM),
VNUMBER(LIGHT_BALL, 21, OBJ_VNUM),
VNUMBER(SPRING, 22, OBJ_VNUM),
VNUMBER(DISC, 23, OBJ_VNUM),
VNUMBER(PORTAL, 25, OBJ_VNUM),
VNUMBER(ROSE, 1001, OBJ_VNUM),
VNUMBER(SCHOOL_MACE, 3700, OBJ_VNUM),
VNUMBER(SCHOOL_DAGGER, 3701, OBJ_VNUM),
VNUMBER(SCHOOL_SWORD, 3702, OBJ_VNUM),
VNUMBER(SCHOOL_SPEAR, 3717, OBJ_VNUM),
VNUMBER(SCHOOL_STAFF, 3718, OBJ_VNUM),
VNUMBER(SCHOOL_AXE, 3719, OBJ_VNUM),
VNUMBER(SCHOOL_FLAIL, 3720, OBJ_VNUM),
VNUMBER(SCHOOL_WHIP, 3721, OBJ_VNUM),
VNUMBER(SCHOOL_POLEARM, 3722, OBJ_VNUM),
VNUMBER(SCHOOL_VEST, 3703, OBJ_VNUM),
VNUMBER(SCHOOL_SHIELD, 3704, OBJ_VNUM),
VNUMBER(SCHOOL_BANNER, 3716, OBJ_VNUM),
VNUMBER(MAP, 3162, OBJ_VNUM),
VNUMBER(WHISTLE, 2116, OBJ_VNUM),
VNUMBER(TRIVIA_PILL, 200, OBJ_VNUM),
VNUMBER(QUEST_AURA, 201, OBJ_VNUM),
VNUMBER(QUEST_SWORD, 203, OBJ_VNUM),
VNUMBER(QUEST_BPLATE, 204, OBJ_VNUM),
VNUMBER(QUEST_BOOTS, 205, OBJ_VNUM),
VNUMBER(QUEST_GLOVES, 206, OBJ_VNUM),
VNUMBER(QUEST_FLAME, 207, OBJ_VNUM),
VNUMBER(QUEST_HELM, 208, OBJ_VNUM),
VNUMBER(QUEST_BAG, 209, OBJ_VNUM),
VNUMBER(QUEST_SHIELD, 210, OBJ_VNUM),
VNUMBER(QUEST_REGEN, 211, OBJ_VNUM),
VNUMBER(QUEST_INVIS, 212, OBJ_VNUM),
VNUMBER(QUEST_TRIVIA, OBJ_VNUM_TRIVIA_PILL, OBJ_VNUM),
VNUMBER(LIMBO, 2, ROOM_VNUM),
VNUMBER(CHAT, 1200, ROOM_VNUM),
VNUMBER(TEMPLE, 3001, ROOM_VNUM),
VNUMBER(ALTAR, 3054, ROOM_VNUM),
VNUMBER(SCHOOL, 3700, ROOM_VNUM),
VNUMBER(WAITROOM, 17600, ROOM_VNUM),
VNUMBER(MORGUE, 3, ROOM_VNUM),
VNUMBER(DUEL_START, 17596, ROOM_VNUM),
VNUMBER(DUEL_END, 17597, ROOM_VNUM),
VNUMBER(DUEL_WINNER, 17598, ROOM_VNUM),
VNUMBER(DUEL_LOSER, 17599, ROOM_VNUM),
VNUMBER(DUMMY, 30, OBJ_VNUM),
VNUMBER(DUMMY, 30, MOB_VNUM),
VNUMBER(QUEST1, 214, OBJ_VNUM),
VNUMBER(QUEST2, 215, OBJ_VNUM),
VNUMBER(QUEST3, 216, OBJ_VNUM),
VNUMBER(QUEST4, 217, OBJ_VNUM),
VNUMBER(DONATION_WEAPON, 3379, ROOM_VNUM),
VNUMBER(DONATION_ARMOR, 3378, ROOM_VNUM),
VNUMBER(DONATION_OTHER, 3362, ROOM_VNUM),
VNUMBER(WAR_START, 17601, ROOM_VNUM),
VNUMBER(WAR_END, 17636, ROOM_VNUM), 
VNUM_END
// *INDENT-ON*
