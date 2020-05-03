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

#undef INDEX_NUM

#define INDEX_GSN   1
#define INDEX_GCN   2
#define INDEX_CLASS 3

#ifdef IN_DB_C
#define INDEX_NUM(i) int i;
#elif defined MAKE_INDEX_TABLE
#define INDEX_NUM(i) { #i, &i },
#else
#define INDEX_NUM(i) EXTERN int i;
#endif

// *INDENT-OFF*
#if !defined MAKE_INDEX_TABLE || (MAKE_INDEX_TABLE == INDEX_GSN)
INDEX_NUM(gsn_null)
INDEX_NUM(gsn_backstab)
INDEX_NUM(gsn_dodge)
INDEX_NUM(gsn_envenom)
INDEX_NUM(gsn_hide)
INDEX_NUM(gsn_peek)
INDEX_NUM(gsn_pick_lock)
INDEX_NUM(gsn_sneak)
INDEX_NUM(gsn_steal)
INDEX_NUM(gsn_disarm)
INDEX_NUM(gsn_enhanced_damage)
INDEX_NUM(gsn_kick)
INDEX_NUM(gsn_parry)
INDEX_NUM(gsn_rescue)
INDEX_NUM(gsn_second_attack)
INDEX_NUM(gsn_third_attack)
INDEX_NUM(gsn_blindness)
INDEX_NUM(gsn_charm_person)
INDEX_NUM(gsn_curse)
INDEX_NUM(gsn_invis)
INDEX_NUM(gsn_mass_invis)
INDEX_NUM(gsn_plague)
INDEX_NUM(gsn_poison)
INDEX_NUM(gsn_sleep)
INDEX_NUM(gsn_fly)
INDEX_NUM(gsn_sanctuary)
INDEX_NUM(gsn_axe)
INDEX_NUM(gsn_dagger)
INDEX_NUM(gsn_flail)
INDEX_NUM(gsn_mace)
INDEX_NUM(gsn_polearm)
INDEX_NUM(gsn_shield_block)
INDEX_NUM(gsn_spear)
INDEX_NUM(gsn_sword)
INDEX_NUM(gsn_whip)
INDEX_NUM(gsn_bash)
INDEX_NUM(gsn_berserk)
INDEX_NUM(gsn_dirt)
INDEX_NUM(gsn_hand_to_hand)
INDEX_NUM(gsn_trip)
INDEX_NUM(gsn_fast_healing)
INDEX_NUM(gsn_haggle)
INDEX_NUM(gsn_lore)
INDEX_NUM(gsn_meditation)
INDEX_NUM(gsn_scrolls)
INDEX_NUM(gsn_staves)
INDEX_NUM(gsn_wands) 
INDEX_NUM(gsn_recall) 
INDEX_NUM(gsn_hunt)
#endif
#if !defined MAKE_INDEX_TABLE || (MAKE_INDEX_TABLE == INDEX_GCN)
INDEX_NUM(gcn_null)
INDEX_NUM(gcn_gossip)
INDEX_NUM(gcn_buddy)
INDEX_NUM(gcn_immtalk)
INDEX_NUM(gcn_clan)
INDEX_NUM(gcn_ooc)
INDEX_NUM(gcn_music)
INDEX_NUM(gcn_qa)
INDEX_NUM(gcn_quote) 
INDEX_NUM(gcn_grats) 
INDEX_NUM(gcn_barter)
#endif
#if !defined MAKE_INDEX_TABLE || (MAKE_INDEX_TABLE == INDEX_CLASS)
INDEX_NUM(class_null)
INDEX_NUM(class_mage)
INDEX_NUM(class_cleric)
INDEX_NUM(class_thief)
INDEX_NUM(class_warrior) 
INDEX_NUM(class_paladin) 
INDEX_NUM(class_ranger)
#endif
// *INDENT-ON*
