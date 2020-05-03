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


#undef MAGIC_FUN
#ifdef MAKE_SPELL_TABLE
#define MAGIC_FUN(fun)  { Stringify(spell_##fun), (void *)spell_##fun },
#else
#define MAGIC_FUN(fun)  Declare_Spell_F(spell_##fun);
#endif

// *INDENT-OFF*
MAGIC_FUN(null)
MAGIC_FUN(acid_blast)
MAGIC_FUN(armor)
MAGIC_FUN(bless)
MAGIC_FUN(blindness)
MAGIC_FUN(burning_hands)
MAGIC_FUN(call_lightning)
MAGIC_FUN(calm)
MAGIC_FUN(cancellation)
MAGIC_FUN(cause_critical)
MAGIC_FUN(cause_light)
MAGIC_FUN(cause_serious)
MAGIC_FUN(change_sex)
MAGIC_FUN(chain_lightning)
MAGIC_FUN(charm_person)
MAGIC_FUN(chill_touch)
MAGIC_FUN(color_spray)
MAGIC_FUN(continual_light)
MAGIC_FUN(control_weather)
MAGIC_FUN(create_food)
MAGIC_FUN(create_rose)
MAGIC_FUN(create_spring)
MAGIC_FUN(create_water)
MAGIC_FUN(cure_blindness)
MAGIC_FUN(cure_critical)
MAGIC_FUN(cure_disease)
MAGIC_FUN(cure_light)
MAGIC_FUN(cure_poison)
MAGIC_FUN(cure_serious)
MAGIC_FUN(curse)
MAGIC_FUN(demonfire)
MAGIC_FUN(detect_evil)
MAGIC_FUN(detect_good)
MAGIC_FUN(detect_hidden)
MAGIC_FUN(detect_invis)
MAGIC_FUN(detect_magic)
MAGIC_FUN(detect_poison)
MAGIC_FUN(dispel_evil)
MAGIC_FUN(dispel_good)
MAGIC_FUN(dispel_magic)
MAGIC_FUN(earthquake)
MAGIC_FUN(enchant_armor)
MAGIC_FUN(enchant_weapon)
MAGIC_FUN(energy_drain)
MAGIC_FUN(faerie_fire)
MAGIC_FUN(faerie_fog)
MAGIC_FUN(farsight)
MAGIC_FUN(fireball)
MAGIC_FUN(fireproof)
MAGIC_FUN(flamestrike)
MAGIC_FUN(floating_disc)
MAGIC_FUN(fly)
MAGIC_FUN(frenzy)
MAGIC_FUN(gate)
MAGIC_FUN(giant_strength)
MAGIC_FUN(harm)
MAGIC_FUN(haste)
MAGIC_FUN(heal)
MAGIC_FUN(heat_metal)
MAGIC_FUN(holy_word)
MAGIC_FUN(identify)
MAGIC_FUN(infravision)
MAGIC_FUN(invis)
MAGIC_FUN(know_alignment)
MAGIC_FUN(lightning_bolt)
MAGIC_FUN(locate_object)
MAGIC_FUN(magic_missile)
MAGIC_FUN(mass_healing)
MAGIC_FUN(mass_invis)
MAGIC_FUN(nexus)
MAGIC_FUN(pass_door)
MAGIC_FUN(plague)
MAGIC_FUN(poison)
MAGIC_FUN(portal)
MAGIC_FUN(protection_evil)
MAGIC_FUN(protection_good)
MAGIC_FUN(ray_of_truth)
MAGIC_FUN(recharge)
MAGIC_FUN(refresh)
MAGIC_FUN(remove_curse)
MAGIC_FUN(sanctuary)
MAGIC_FUN(shocking_grasp)
MAGIC_FUN(shield)
MAGIC_FUN(sleep)
MAGIC_FUN(slow)
MAGIC_FUN(stone_skin)
MAGIC_FUN(summon)
MAGIC_FUN(teleport)
MAGIC_FUN(ventriloquate)
MAGIC_FUN(weaken)
MAGIC_FUN(word_of_recall)
MAGIC_FUN(acid_breath)
MAGIC_FUN(fire_breath)
MAGIC_FUN(frost_breath)
MAGIC_FUN(gas_breath)
MAGIC_FUN(lightning_breath)
MAGIC_FUN(general_purpose)
MAGIC_FUN(high_explosive)
MAGIC_FUN(trivia_pill)
MAGIC_FUN(forceshield)
MAGIC_FUN(staticshield)
MAGIC_FUN(flameshield)
MAGIC_FUN(channel)
MAGIC_FUN(investiture)
MAGIC_FUN(powerstorm)
MAGIC_FUN(mana_burn)
MAGIC_FUN(bark_skin)
MAGIC_FUN(spell_mantle)
MAGIC_FUN(animal_instinct) 
MAGIC_FUN(chaos_flare) 
MAGIC_FUN(wild_magic)
// *INDENT-ON*
