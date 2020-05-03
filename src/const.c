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

#include "merc.h"
#include "magic.h"
#include "interp.h"
#include "vnums.h"

char *const dir_name[MAX_DIR] = {
  "north", "east", "south", "west", "up", "down"
};

const int rev_dir[MAX_DIR] = {
  DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP
};

const int movement_loss[SECT_MAX] = {
  1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 1, 1, 6, 4
};



const char *position_table[] = {
  "dead",
  "mortally wounded",
  "incapacitated",
  "stunned",
  "sleeping",
  "resting",
  "sitting",
  "fighting",
  "standing",
  NULL
};


const struct weapon_type weapon_table[] = {
  {"sword", OBJ_VNUM_SCHOOL_SWORD, WEAPON_SWORD, &gsn_sword},
  {"mace", OBJ_VNUM_SCHOOL_MACE, WEAPON_MACE, &gsn_mace},
  {"dagger", OBJ_VNUM_SCHOOL_DAGGER, WEAPON_DAGGER, &gsn_dagger},
  {"axe", OBJ_VNUM_SCHOOL_AXE, WEAPON_AXE, &gsn_axe},
  {"staff", OBJ_VNUM_SCHOOL_STAFF, WEAPON_SPEAR, &gsn_spear},
  {"flail", OBJ_VNUM_SCHOOL_FLAIL, WEAPON_FLAIL, &gsn_flail},
  {"whip", OBJ_VNUM_SCHOOL_WHIP, WEAPON_WHIP, &gsn_whip},
  {"polearm", OBJ_VNUM_SCHOOL_POLEARM, WEAPON_POLEARM, &gsn_polearm},
  {NULL, 0, (weapon_t) - 1, NULL}
};


const struct wiznet_type wiznet_table[] = {
  {"on", WIZ_ON, LEVEL_IMMORTAL},
  {"prefix", WIZ_PREFIX, LEVEL_IMMORTAL},
  {"ticks", WIZ_TICKS, LEVEL_IMMORTAL},
  {"logins", WIZ_LOGINS, LEVEL_IMMORTAL},
  {"sites", WIZ_SITES, MAX_LEVEL - 4},
  {"links", WIZ_LINKS, MAX_LEVEL - 7},
  {"newbies", WIZ_NEWBIE, LEVEL_IMMORTAL},
  {"spam", WIZ_SPAM, MAX_LEVEL - 5},
  {"deaths", WIZ_DEATHS, LEVEL_IMMORTAL},
  {"resets", WIZ_RESETS, MAX_LEVEL - 4},
  {"mobdeaths", WIZ_MOBDEATHS, MAX_LEVEL - 4},
  {"flags", WIZ_FLAGS, MAX_LEVEL - 5},
  {"penalties", WIZ_PENALTIES, MAX_LEVEL - 5},
  {"saccing", WIZ_SACCING, MAX_LEVEL - 5},
  {"levels", WIZ_LEVELS, LEVEL_IMMORTAL},
  {"load", WIZ_LOAD, MAX_LEVEL - 2},
  {"restore", WIZ_RESTORE, MAX_LEVEL - 2},
  {"snoops", WIZ_SNOOPS, MAX_LEVEL - 2},
  {"switches", WIZ_SWITCHES, MAX_LEVEL - 2},
  {"secure", WIZ_SECURE, MAX_LEVEL - 1},
  {"bugs", WIZ_BUGS, MAX_LEVEL - 3},
  {"telnet", WIZ_TELNET, MAX_LEVEL - 2},
  {"chansnoop", WIZ_CHANSNOOP, MAX_LEVEL - 4},
#ifndef DISABLE_I3
  {"i3", WIZ_I3, LEVEL_IMMORTAL + 2},
#endif
  {NULL, 0, 0}
};


const struct attack_type attack_table[] = {
  {"none", "hit", (dam_class) - 1}
  ,
  {"slice", "slice", DAM_SLASH},
  {"stab", "stab", DAM_PIERCE},
  {"slash", "slash", DAM_SLASH},
  {"whip", "whip", DAM_SLASH},
  {"claw", "claw", DAM_SLASH},
  {"blast", "blast", DAM_BASH},
  {"pound", "pound", DAM_BASH},
  {"crush", "crush", DAM_BASH},
  {"grep", "grep", DAM_SLASH},
  {"bite", "bite", DAM_PIERCE},
  {"pierce", "pierce", DAM_PIERCE},
  {"suction", "suction", DAM_BASH},
  {"beating", "beating", DAM_BASH},
  {"digestion", "digestion", DAM_ACID},
  {"charge", "charge", DAM_BASH},
  {"slap", "slap", DAM_BASH},
  {"punch", "punch", DAM_BASH},
  {"wrath", "wrath", DAM_ENERGY},
  {"magic", "magic", DAM_ENERGY},
  {"divine", "divine power", DAM_HOLY},
  {"cleave", "cleave", DAM_SLASH},
  {"scratch", "scratch", DAM_PIERCE},
  {"peck", "peck", DAM_PIERCE},
  {"peckb", "peck", DAM_BASH},
  {"chop", "chop", DAM_SLASH},
  {"sting", "sting", DAM_PIERCE},
  {"smash", "smash", DAM_BASH},
  {"shbite", "shocking bite", DAM_LIGHTNING},
  {"flbite", "flaming bite", DAM_FIRE},
  {"frbite", "freezing bite", DAM_COLD},
  {"acbite", "acidic bite", DAM_ACID},
  {"chomp", "chomp", DAM_PIERCE},
  {"drain", "life drain", DAM_NEGATIVE},
  {"thrust", "thrust", DAM_PIERCE},
  {"slime", "slime", DAM_ACID},
  {"shock", "shock", DAM_LIGHTNING},
  {"thwack", "thwack", DAM_BASH},
  {"flame", "flame", DAM_FIRE},
  {"chill", "chill", DAM_COLD},
  {"code", "code", DAM_OTHER},
  {"radiation", "radiation", DAM_POISON},
  {NULL, NULL, (dam_class) - 1}
};

int MAX_DAMAGE_MESSAGE =
  (sizeof (attack_table) / sizeof (attack_table[0]) - 1);


const struct str_app_type str_app[MAX_STATS + 1] = {
  {
   -5, -4, 0, 0}
  ,
  {-5, -4, 3, 1},
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},
  {0, 0, 115, 11},
  {0, 0, 130, 12},
  {0, 0, 130, 13},
  {0, 1, 140, 14},
  {1, 1, 150, 15},
  {1, 2, 165, 16},
  {2, 3, 180, 22},
  {2, 3, 200, 25},
  {3, 4, 225, 30},
  {3, 5, 250, 35},
  {4, 6, 300, 40},
  {4, 6, 350, 45},
  {5, 7, 400, 50},
  {5, 8, 450, 55},
  {6, 9, 500, 60},
  {6, 10, 550, 65},
  {7, 11, 600, 70},
  {7, 12, 650, 75},
  {8, 13, 700, 80},
  {8, 14, 750, 85}
};

const struct int_app_type int_app[MAX_STATS + 1] = {
  {
   3}
  ,
  {5},
  {7},
  {8},
  {9},
  {10},
  {11},
  {12},
  {13},
  {15},
  {17},
  {19},
  {22},
  {25},
  {28},
  {31},
  {34},
  {37},
  {40},
  {44},
  {49},
  {55},
  {60},
  {70},
  {80},
  {85},
  {90},
  {95},
  {100},
  {105},
  {110}
};

const struct wis_app_type wis_app[MAX_STATS + 1] = {
  {
   0}
  ,
  {0},
  {0},
  {0},
  {0},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {2},
  {2},
  {2},
  {3},
  {3},
  {3},
  {3},
  {4},
  {4},
  {4},
  {5},
  {5},
  {5},
  {6},
  {6},
  {6}
};

const struct dex_app_type dex_app[MAX_STATS + 1] = {
  {
   60}
  ,
  {50},
  {50},
  {40},
  {30},
  {20},
  {10},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {0},
  {-10},
  {-15},
  {-20},
  {-30},
  {-40},
  {-50},
  {-60},
  {-75},
  {-90},
  {-105},
  {-120},
  {-130},
  {-140},
  {-155},
  {-175},
  {-190}
};

const struct con_app_type con_app[MAX_STATS + 1] = {
  {
   -4, 20}
  ,
  {-3, 25},
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},
  {2, 95},
  {2, 97},
  {3, 99},
  {3, 99},
  {4, 99},
  {4, 99},
  {5, 99},
  {6, 99},
  {7, 99},
  {8, 99},
  {9, 100},
  {10, 102},
  {11, 104},
  {12, 107},
  {13, 110}
};


const struct liq_type liq_table[] = {

  {"water", "clear", {0, 1, 10, 0, 16}},
  {"beer", "amber", {12, 1, 8, 1, 12}},
  {"red wine", "burgundy", {30, 1, 8, 1, 5}},
  {"ale", "brown", {15, 1, 8, 1, 12}},
  {"dark ale", "dark", {16, 1, 8, 1, 12}},

  {"whisky", "golden", {120, 1, 5, 0, 2}},
  {"lemonade", "pink", {0, 1, 9, 2, 12}},
  {"firebreather", "boiling", {190, 0, 4, 0, 2}},
  {"local specialty", "clear", {151, 1, 3, 0, 2}},
  {"slime mold juice", "green", {0, 2, -8, 1, 2}},

  {"milk", "white", {0, 2, 9, 3, 12}},
  {"tea", "tan", {0, 1, 8, 0, 6}},
  {"coffee", "black", {0, 1, 8, 0, 6}},
  {"blood", "red", {0, 2, -1, 2, 6}},
  {"salt water", "clear", {0, 1, -2, 0, 1}},

  {"coke", "brown", {0, 2, 9, 2, 12}},
  {"root beer", "brown", {0, 2, 9, 2, 12}},
  {"elvish wine", "green", {35, 2, 8, 1, 5}},
  {"white wine", "golden", {28, 1, 8, 1, 5}},
  {"champagne", "golden", {32, 1, 8, 1, 5}},

  {"mead", "honey-colored", {34, 2, 8, 2, 12}},
  {"rose wine", "pink", {26, 1, 8, 1, 5}},
  {"benedictine wine", "burgundy", {40, 1, 8, 1, 5}},
  {"vodka", "clear", {130, 1, 5, 0, 2}},
  {"cranberry juice", "red", {0, 1, 9, 2, 12}},

  {"orange juice", "orange", {0, 2, 9, 3, 12}},
  {"absinthe", "green", {200, 1, 4, 0, 2}},
  {"brandy", "golden", {80, 1, 5, 0, 4}},
  {"aquavit", "clear", {140, 1, 5, 0, 2}},
  {"schnapps", "clear", {90, 1, 5, 0, 2}},

  {"icewine", "purple", {50, 2, 6, 1, 5}},
  {"amontillado", "burgundy", {35, 2, 8, 1, 5}},
  {"sherry", "red", {38, 2, 7, 1, 5}},
  {"framboise", "red", {50, 1, 7, 1, 5}},
  {"rum", "amber", {151, 1, 4, 0, 2}},

  {"cordial", "clear", {100, 1, 5, 0, 2}},
  {NULL, NULL, {0, 0, 0, 0, 0}}
};

char *const he_she[] = { "it", "he", "she"
};
char *const him_her[] = { "it", "him", "her"
};
char *const his_her[] = { "its", "his", "her"
};

char *const runlvl_name[] = {
  "INIT",
  "BOOTING",
  "SAFE_BOOT",
  "MAIN_LOOP",
  "SHUTDOWN"
};


char *const day_name[DAYS_IN_WEEK] = {
  "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
  "the Great Gods", "the Sun"
};

char *const month_name[MONTHS_IN_YEAR] = {
  "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
  "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
  "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
  "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

char *const where_name[] = {
  "{g<{Wused as light{g>{x     ",
  "{g<{Wworn on finger{g>{x    ",
  "{g<{Wworn on finger{g>{x    ",
  "{g<{Wworn around neck{g>{x  ",
  "{g<{Wworn around neck{g>{x  ",
  "{g<{Wworn on torso{g>{x     ",
  "{g<{Wworn on head{g>{x      ",
  "{g<{Wworn on legs{g>{x      ",
  "{g<{Wworn on feet{g>{x      ",
  "{g<{Wworn on hands{g>{x     ",
  "{g<{Wworn on arms{g>{x      ",
  "{g<{Wworn as shield{g>{x    ",
  "{g<{Wworn about body{g>{x   ",
  "{g<{Wworn about waist{g>{x  ",
  "{g<{Wworn around wrist{g>{x ",
  "{g<{Wworn around wrist{g>{x ",
  "{g<{Wwielded{g>{x           ",
  "{g<{Wheld{g>{x              ",
  "{g<{Wfloating nearby{g>{x   ",
  "{g<{Wsecondary weapon{g>{x  "
};


char *const temp_settings[MAX_CLIMATE] = {
  "cold",
  "cool",
  "normal",
  "warm",
  "hot",
};

char *const precip_settings[MAX_CLIMATE] = {
  "arid",
  "dry",
  "normal",
  "damp",
  "wet",
};

char *const wind_settings[MAX_CLIMATE] = {
  "still",
  "calm",
  "normal",
  "breezy",
  "windy",
};

char *const preciptemp_msg[6][6] = {

  {
   "Frigid temperatures settle over the land",
   "It is bitterly cold",
   "The weather is crisp and dry",
   "A comfortable warmth sets in",
   "A dry heat warms the land",
   "Seething heat bakes the land"},

  {
   "A few flurries drift from the high clouds",
   "Frozen drops of rain fall from the sky",
   "An occasional raindrop falls to the ground",
   "Mild drops of rain seep from the clouds",
   "It is very warm, and the sky is overcast",
   "High humidity intensifies the seering heat"},

  {
   "A brief snow squall dusts the earth",
   "A light flurry dusts the ground",
   "Light snow drifts down from the heavens",
   "A light drizzle mars an otherwise perfect day",
   "A few drops of rain fall to the warm ground",
   "A light rain falls through the sweltering sky"},

  {
   "Snowfall covers the frigid earth",
   "Light snow falls to the ground",
   "A brief shower moistens the crisp air",
   "A pleasant rain falls from the heavens",
   "The warm air is heavy with rain",
   "A refreshing shower eases the oppresive heat"},

  {
   "Sleet falls in sheets through the frosty air",
   "Snow falls quickly, piling upon the cold earth",
   "Rain pelts the ground on this crisp day",
   "Rain drums the ground rythmically",
   "A warm rain drums the ground loudly",
   "Tropical rain showers pelt the seering ground"},

  {
   "A downpour of frozen rain covers the land in ice",
   "A blizzard blankets everything in pristine white",
   "Torrents of rain fall from a cool sky",
   "A drenching downpour obscures the temperate day",
   "Warm rain pours from the sky",
   "A torrent of rain soaks the heated earth"}
};

char *const windtemp_msg[6][6] = {

  {
   "The frigid air is completely still",
   "A cold temperature hangs over the area",
   "The crisp air is eerily calm",
   "The warm air is still",
   "No wind makes the day uncomfortably warm",
   "The stagnant heat is sweltering"},

  {
   "A light breeze makes the frigid air seem colder",
   "A stirring of the air intensifies the cold",
   "A touch of wind makes the day cool",
   "It is a temperate day, with a slight breeze",
   "It is very warm, the air stirs slightly",
   "A faint breeze stirs the feverish air"},

  {
   "A breeze gives the frigid air bite",
   "A breeze swirls the cold air",
   "A lively breeze cools the area",
   "It is a temperate day, with a pleasant breeze",
   "Very warm breezes buffet the area",
   "A breeze ciculates the sweltering air"},

  {
   "Stiff gusts add cold to the frigid air",
   "The cold air is agitated by gusts of wind",
   "Wind blows in from the north, cooling the area",
   "Gusty winds mix the temperate air",
   "Brief gusts of wind punctuate the warm day",
   "Wind attempts to cut the sweltering heat"},

  {
   "The frigid air whirls in gusts of wind",
   "A strong, cold wind blows in from the north",
   "Strong wind makes the cool air nip",
   "It is a pleasant day, with gusty winds",
   "Warm, gusty winds move through the area",
   "Blustering winds punctuate the seering heat"},

  {
   "A frigid gale sets bones shivering",
   "Howling gusts of wind cut the cold air",
   "An angry wind whips the air into a frenzy",
   "Fierce winds tear through the tepid air",
   "Gale-like winds whip up the warm air",
   "Monsoon winds tear the feverish air"}
};

char *const precip_msg[3] = {
  "there is not a cloud in the sky",
  "pristine white clouds are in the sky",
  "thick, grey clouds mask the sun"
};

char *const wind_msg[6] = {
  "there is not a breath of wind in the air",
  "a slight breeze stirs the air",
  "a breeze wafts through the area",
  "brief gusts of wind punctuate the air",
  "angry gusts of wind blow",
  "howling winds whip the air into a frenzy"
};
