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
#include "tables.h"
#include "interp.h"
#include "vnums.h"


FlagTable act_flags[] = {
  {"npc", ACT_IS_NPC, false}
  ,
  {"sentinel", ACT_SENTINEL, true}
  ,
  {"scavenger", ACT_SCAVENGER, true}
  ,
  {"aggressive", ACT_AGGRESSIVE, true}
  ,
  {"stay_area", ACT_STAY_AREA, true}
  ,
  {"wimpy", ACT_WIMPY, true}
  ,
  {"pet", ACT_PET, true}
  ,
  {"train", ACT_TRAIN, true}
  ,
  {"practice", ACT_PRACTICE, true}
  ,
  {"undead", ACT_UNDEAD, true}
  ,
  {"cleric", ACT_CLERIC, true}
  ,
  {"mage", ACT_MAGE, true}
  ,
  {"thief", ACT_THIEF, true}
  ,
  {"warrior", ACT_WARRIOR, true}
  ,
  {"noalign", ACT_NOALIGN, true}
  ,
  {"nopurge", ACT_NOPURGE, true}
  ,
  {"outdoors", ACT_OUTDOORS, true}
  ,
  {"indoors", ACT_INDOORS, true}
  ,
  {"healer", ACT_IS_HEALER, true}
  ,
  {"gain", ACT_GAIN, true}
  ,
  {"update_always", ACT_UPDATE_ALWAYS, true}
  ,
  {"changer", ACT_IS_CHANGER, true}
  ,
  {NULL, 0, false}
};

FlagTable plr_flags[] = {
  {"npc", PLR_IsNPC, false}
  ,
  {"automap", PLR_AUTOMAP, true}
  ,
  {"autoassist", PLR_AUTOASSIST, false}
  ,
  {"autoexit", PLR_AUTOEXIT, false}
  ,
  {"autoloot", PLR_AUTOLOOT, false}
  ,
  {"autosac", PLR_AUTOSAC, false}
  ,
  {"autogold", PLR_AUTOGOLD, false}
  ,
  {"autosplit", PLR_AUTOSPLIT, false}
  ,
  {"holylight", PLR_HOLYLIGHT, false}
  ,
  {"can_loot", PLR_CANLOOT, false}
  ,
  {"nosummon", PLR_NOSUMMON, false}
  ,
  {"nofollow", PLR_NOFOLLOW, false}
  ,
  {"permit", PLR_PERMIT, true}
  ,
  {"log", PLR_LOG, false}
  ,
  {"deny", PLR_DENY, false}
  ,
  {"freeze", PLR_FREEZE, false}
  ,
  {"thief", PLR_THIEF, false}
  ,
  {"killer", PLR_KILLER, false}
  ,
  {"questor", PLR_QUESTER, true}
  ,
  {"autodamage", PLR_AUTODAMAGE, true}
  ,
  {"autoprompt", PLR_AUTOPROMPT, true}
  ,
  {NULL, 0, 0}
};

FlagTable vt100_flags[] = {
  {"showcodes", VT100_SHOW_CODES, true}
  ,
  {"nobeeps", VT100_NO_BEEPS, true}
  ,
  {"nonewlines", VT100_NO_NEWLINE, true}
  ,
  {"darkmod", VT100_DARK_MOD, true}
  ,
  {"noblinking", VT100_NO_BLINKING, true}
  ,
  {"nobackground", VT100_NO_BACKGROUND, true}
  ,
  {"darkcolors", VT100_DARK_COLORS, true}
  ,
  {"brokenansi", VT100_BROKEN_ANSI, true}
  ,
  {NULL, 0, false}
};

FlagTable affect_flags[] = {
  {"blind", AFF_BLIND, true}
  ,
  {"invisible", AFF_INVISIBLE, true}
  ,
  {"detect_evil", AFF_DETECT_EVIL, true}
  ,
  {"detect_invis", AFF_DETECT_INVIS, true}
  ,
  {"detect_magic", AFF_DETECT_MAGIC, true}
  ,
  {"detect_hidden", AFF_DETECT_HIDDEN, true}
  ,
  {"detect_good", AFF_DETECT_GOOD, true}
  ,
  {"sanctuary", AFF_SANCTUARY, true}
  ,
  {"faerie_fire", AFF_FAERIE_FIRE, true}
  ,
  {"infrared", AFF_INFRARED, true}
  ,
  {"curse", AFF_CURSE, true}
  ,
  {"poison", AFF_POISON, true}
  ,
  {"protect_evil", AFF_PROTECT_EVIL, true}
  ,
  {"protect_good", AFF_PROTECT_GOOD, true}
  ,
  {"sneak", AFF_SNEAK, true}
  ,
  {"hide", AFF_HIDE, true}
  ,
  {"sleep", AFF_SLEEP, true}
  ,
  {"charm", AFF_CHARM, true}
  ,
  {"flying", AFF_FLYING, true}
  ,
  {"pass_door", AFF_PASS_DOOR, true}
  ,
  {"haste", AFF_HASTE, true}
  ,
  {"calm", AFF_CALM, true}
  ,
  {"plague", AFF_PLAGUE, true}
  ,
  {"weaken", AFF_WEAKEN, true}
  ,
  {"dark_vision", AFF_DARK_VISION, true}
  ,
  {"berserk", AFF_BERSERK, true}
  ,
  {"swim", AFF_SWIM, true}
  ,
  {"regeneration", AFF_REGENERATION, true}
  ,
  {"slow", AFF_SLOW, true}
  ,
  {"forceshield", AFF_FORCE_SHIELD, true}
  ,
  {"staticshield", AFF_STATIC_SHIELD, true}
  ,
  {"flameshield", AFF_FLAME_SHIELD, true}
  ,
  {NULL, 0, 0}
};

FlagTable off_flags[] = {
  {"area_attack", OFF_AREA_ATTACK, true}
  ,
  {"backstab", OFF_BACKSTAB, true}
  ,
  {"bash", OFF_BASH, true}
  ,
  {"berserk", OFF_BERSERK, true}
  ,
  {"disarm", OFF_DISARM, true}
  ,
  {"dodge", OFF_DODGE, true}
  ,
  {"fade", OFF_FADE, true}
  ,
  {"fast", OFF_FAST, true}
  ,
  {"kick", OFF_KICK, true}
  ,
  {"dirt_kick", OFF_KICK_DIRT, true}
  ,
  {"parry", OFF_PARRY, true}
  ,
  {"rescue", OFF_RESCUE, true}
  ,
  {"tail", OFF_TAIL, true}
  ,
  {"trip", OFF_TRIP, true}
  ,
  {"crush", OFF_CRUSH, true}
  ,
  {"assist_all", ASSIST_ALL, true}
  ,
  {"assist_align", ASSIST_ALIGN, true}
  ,
  {"assist_race", ASSIST_RACE, true}
  ,
  {"assist_players", ASSIST_PLAYERS, true}
  ,
  {"assist_guard", ASSIST_GUARD, true}
  ,
  {"assist_vnum", ASSIST_VNUM, true}
  ,
  {NULL, 0, 0}
};

FlagTable imm_flags[] = {
  {"summon", IMM_SUMMON, true}
  ,
  {"charm", IMM_CHARM, true}
  ,
  {"magic", IMM_MAGIC, true}
  ,
  {"weapon", IMM_WEAPON, true}
  ,
  {"bash", IMM_BASH, true}
  ,
  {"pierce", IMM_PIERCE, true}
  ,
  {"slash", IMM_SLASH, true}
  ,
  {"fire", IMM_FIRE, true}
  ,
  {"cold", IMM_COLD, true}
  ,
  {"lightning", IMM_LIGHTNING, true}
  ,
  {"acid", IMM_ACID, true}
  ,
  {"poison", IMM_POISON, true}
  ,
  {"negative", IMM_NEGATIVE, true}
  ,
  {"holy", IMM_HOLY, true}
  ,
  {"energy", IMM_ENERGY, true}
  ,
  {"mental", IMM_MENTAL, true}
  ,
  {"disease", IMM_DISEASE, true}
  ,
  {"drowning", IMM_DROWNING, true}
  ,
  {"light", IMM_LIGHT, true}
  ,
  {"sound", IMM_SOUND, true}
  ,
  {"wood", IMM_WOOD, true}
  ,
  {"silver", IMM_SILVER, true}
  ,
  {"iron", IMM_IRON, true}
  ,
  {NULL, 0, 0}
};

FlagTable form_flags[] = {
  {"edible", FORM_EDIBLE, true}
  ,
  {"poison", FORM_POISON, true}
  ,
  {"magical", FORM_MAGICAL, true}
  ,
  {"instant_decay", FORM_INSTANT_DECAY, true}
  ,
  {"other", FORM_OTHER, true}
  ,
  {"animal", FORM_ANIMAL, true}
  ,
  {"sentient", FORM_SENTIENT, true}
  ,
  {"undead", FORM_UNDEAD, true}
  ,
  {"construct", FORM_CONSTRUCT, true}
  ,
  {"mist", FORM_MIST, true}
  ,
  {"intangible", FORM_INTANGIBLE, true}
  ,
  {"biped", FORM_BIPED, true}
  ,
  {"centaur", FORM_CENTAUR, true}
  ,
  {"insect", FORM_INSECT, true}
  ,
  {"spider", FORM_SPIDER, true}
  ,
  {"crustacean", FORM_CRUSTACEAN, true}
  ,
  {"worm", FORM_WORM, true}
  ,
  {"blob", FORM_BLOB, true}
  ,
  {"mammal", FORM_MAMMAL, true}
  ,
  {"bird", FORM_BIRD, true}
  ,
  {"reptile", FORM_REPTILE, true}
  ,
  {"snake", FORM_SNAKE, true}
  ,
  {"dragon", FORM_DRAGON, true}
  ,
  {"amphibian", FORM_AMPHIBIAN, true}
  ,
  {"fish", FORM_FISH, true}
  ,
  {"cold_blood", FORM_COLD_BLOOD, true}
  ,
  {NULL, 0, 0}
};

FlagTable part_flags[] = {
  {"head", PART_HEAD, true}
  ,
  {"arms", PART_ARMS, true}
  ,
  {"legs", PART_LEGS, true}
  ,
  {"heart", PART_HEART, true}
  ,
  {"brains", PART_BRAINS, true}
  ,
  {"guts", PART_GUTS, true}
  ,
  {"hands", PART_HANDS, true}
  ,
  {"feet", PART_FEET, true}
  ,
  {"fingers", PART_FINGERS, true}
  ,
  {"ear", PART_EAR, true}
  ,
  {"eye", PART_EYE, true}
  ,
  {"long_tongue", PART_LONG_TONGUE, true}
  ,
  {"eyestalks", PART_EYESTALKS, true}
  ,
  {"tentacles", PART_TENTACLES, true}
  ,
  {"fins", PART_FINS, true}
  ,
  {"wings", PART_WINGS, true}
  ,
  {"tail", PART_TAIL, true}
  ,
  {"claws", PART_CLAWS, true}
  ,
  {"fangs", PART_FANGS, true}
  ,
  {"horns", PART_HORNS, true}
  ,
  {"scales", PART_SCALES, true}
  ,
  {"tusks", PART_TUSKS, true}
  ,
  {NULL, 0, 0}
};

FlagTable comm_flags[] = {
  {"quiet", COMM_QUIET, true}
  ,
  {"deaf", COMM_DEAF, true}
  ,
  {"nowiz", COMM_NOWIZ, true}
  ,
  {"noclangossip", COMM_NOAUCTION, true}
  ,
  {"nogossip", COMM_NOGOSSIP, true}
  ,
  {"noquestion", COMM_NOQUESTION, true}
  ,
  {"nomusic", COMM_NOMUSIC, true}
  ,
  {"noclan", COMM_NOCLAN, true}
  ,
  {"noquote", COMM_NOQUOTE, true}
  ,
  {"shoutsoff", COMM_SHOUTSOFF, true}
  ,
  {"compact", COMM_COMPACT, true}
  ,
  {"brief", COMM_BRIEF, true}
  ,
  {"prompt", COMM_PROMPT, true}
  ,
  {"gprompt", COMM_GPROMPT, true}
  ,
  {"combine", COMM_COMBINE, true}
  ,
  {"telnet_ga", COMM_TELNET_GA, true}
  ,
  {"show_affects", COMM_SHOW_AFFECTS, true}
  ,
  {"nograts", COMM_NOGRATS, true}
  ,
  {"noemote", COMM_NOEMOTE, false}
  ,
  {"noshout", COMM_NOSHOUT, false}
  ,
  {"notell", COMM_NOTELL, false}
  ,
  {"nochannels", COMM_NOCHANNELS, false}
  ,
  {"snoop_proof", COMM_SNOOP_PROOF, false}
  ,
  {"afk", COMM_AFK, true}
  ,
  {"nocolor", COMM_NOCOLOR, true}
  ,
  {"telnet_eor", COMM_TELNET_EOR, true}
  ,
  {"nogocial", COMM_NOGOCIAL, true}
  ,
  {"noooc", COMM_NOOOC, true}
  ,
  {"nobuddy", COMM_NOBUDDY, true}
  ,
  {"censor", COMM_CENSOR, true}
  ,
  {"noemoteverbs", COMM_NOSAYVERBS, true}
  ,

  {NULL, 0, 0}
};

FlagTable desc_flags[] = {
  {"color", DESC_COLOR, true}
  ,
  {"eor", DESC_TELOPT_EOR, true}
  ,
  {"echo", DESC_TELOPT_ECHO, true}
  ,
  {"naws", DESC_TELOPT_NAWS, true}
  ,
  {"pueblo", DESC_PUEBLO, true}
  ,
  {"mxp", DESC_MXP, true}
  ,
  {"msp", DESC_MSP, true}
  ,
  {"ttype", DESC_TELOPT_TTYPE, true}
  ,
  {"binary", DESC_TELOPT_BINARY, true}
  ,
  {"portal", DESC_PORTAL, true}
  ,
  {"imp", DESC_IMP, true}
  ,
  {NULL, 0, 0}
};

FlagTable mprog_flags[] = {
  {"act", TRIG_ACT, true}
  ,
  {"bribe", TRIG_BRIBE, true}
  ,
  {"death", TRIG_DEATH, true}
  ,
  {"entry", TRIG_ENTRY, true}
  ,
  {"fight", TRIG_FIGHT, true}
  ,
  {"give", TRIG_GIVE, true}
  ,
  {"greet", TRIG_GREET, true}
  ,
  {"grall", TRIG_GRALL, true}
  ,
  {"kill", TRIG_KILL, true}
  ,
  {"hpcnt", TRIG_HPCNT, true}
  ,
  {"random", TRIG_RANDOM, true}
  ,
  {"speech", TRIG_SPEECH, true}
  ,
  {"exit", TRIG_EXIT, true}
  ,
  {"exall", TRIG_EXALL, true}
  ,
  {"delay", TRIG_DELAY, true}
  ,
  {"surr", TRIG_SURR, true}
  ,
  {NULL, 0, true}
};

FlagTable oprog_flags[] = {
  {"act", TRIG_ACT, true}
  ,
  {"fight", TRIG_FIGHT, true}
  ,
  {"give", TRIG_GIVE, true}
  ,
  {"greet", TRIG_GREET, true}
  ,
  {"grall", TRIG_GRALL, true}
  ,
  {"random", TRIG_RANDOM, true}
  ,
  {"speech", TRIG_SPEECH, true}
  ,
  {"exall", TRIG_EXALL, true}
  ,
  {"delay", TRIG_DELAY, true}
  ,
  {"drop", TRIG_DROP, true}
  ,
  {"get", TRIG_GET, true}
  ,
  {"sit", TRIG_SIT, true}
  ,
  {NULL, 0, true}
  ,
};

FlagTable rprog_flags[] = {
  {"act", TRIG_ACT, true}
  ,
  {"fight", TRIG_FIGHT, true}
  ,
  {"drop", TRIG_DROP, true}
  ,
  {"greet", TRIG_GREET, true}
  ,
  {"grall", TRIG_GRALL, true}
  ,
  {"random", TRIG_RANDOM, true}
  ,
  {"speech", TRIG_SPEECH, true}
  ,
  {"exall", TRIG_EXALL, true}
  ,
  {"delay", TRIG_DELAY, true}
  ,
  {NULL, 0, true}
  ,
};

FlagTable area_flags[] = {
  {"none", AREA_NONE, false}
  ,
  {"changed", AREA_CHANGED, true}
  ,
  {"added", AREA_ADDED, true}
  ,
  {"loading", AREA_LOADING, false}
  ,
  {"player_homes", AREA_PLAYER_HOMES, true}
  ,
  {"closed", AREA_CLOSED, true}
  ,
  {NULL, 0, 0}
};

FlagTable sex_flags[] = {
  {"male", SEX_MALE, true}
  ,
  {"female", SEX_FEMALE, true}
  ,
  {"either", SEX_NEUTRAL, true}
  ,
  {"random", SEX_RANDOM, true}
  ,
  {"none", SEX_NEUTRAL, true}
  ,
  {NULL, 0, 0}
};

FlagTable exit_flags[] = {
  {"door", EX_ISDOOR, true}
  ,
  {"closed", EX_CLOSED, true}
  ,
  {"locked", EX_LOCKED, true}
  ,
  {"pickproof", EX_PICKPROOF, true}
  ,
  {"nopass", EX_NOPASS, true}
  ,
  {"easy", EX_EASY, true}
  ,
  {"hard", EX_HARD, true}
  ,
  {"infuriating", EX_INFURIATING, true}
  ,
  {"noclose", EX_NOCLOSE, true}
  ,
  {"nolock", EX_NOLOCK, true}
  ,
  {"doorbell", EX_DOORBELL, true}
  ,
  {NULL, 0, 0}
};

FlagTable room_flags[] = {
  {"dark", ROOM_DARK, true}
  ,
  {"home_entrance", HOME_ENTRANCE, true}
  ,
  {"no_mob", ROOM_NO_MOB, true}
  ,
  {"indoors", ROOM_INDOORS, true}
  ,
  {"private", ROOM_PRIVATE, true}
  ,
  {"safe", ROOM_SAFE, true}
  ,
  {"solitary", ROOM_SOLITARY, true}
  ,
  {"pet_shop", ROOM_PET_SHOP, true}
  ,
  {"no_recall", ROOM_NO_RECALL, true}
  ,
  {"imp_only", ROOM_IMP_ONLY, true}
  ,
  {"gods_only", ROOM_GODS_ONLY, true}
  ,
  {"heroes_only", ROOM_HEROES_ONLY, true}
  ,
  {"newbies_only", ROOM_NEWBIES_ONLY, true}
  ,
  {"law", ROOM_LAW, true}
  ,
  {"nowhere", ROOM_NOWHERE, true}
  ,
  {"noexplore", ROOM_NOEXPLORE, true}
  ,
  {"noautomap", ROOM_NOAUTOMAP, true}
  ,
  {"arena", ROOM_ARENA, true}
  ,
  {"bank", ROOM_BANK, true}
  ,
  {"save_objs", ROOM_SAVE_OBJS, true}
  ,
  {NULL, 0, 0}
};

FlagTable sector_flags[] = {
  {"inside", SECT_INSIDE, true}
  ,
  {"city", SECT_CITY, true}
  ,
  {"field", SECT_FIELD, true}
  ,
  {"forest", SECT_FOREST, true}
  ,
  {"hills", SECT_HILLS, true}
  ,
  {"mountain", SECT_MOUNTAIN, true}
  ,
  {"swim", SECT_WATER_SWIM, true}
  ,
  {"noswim", SECT_WATER_NOSWIM, true}
  ,
  {"ice", SECT_ICE, true}
  ,
  {"air", SECT_AIR, true}
  ,
  {"desert", SECT_DESERT, true}
  ,
  {"road", SECT_ROAD, true}
  ,
  {"path", SECT_PATH, true}
  ,
  {"swamp", SECT_SWAMP, true}
  ,
  {"jungle", SECT_JUNGLE, true}
  ,
  {"cave", SECT_CAVE, true}
  ,
  {NULL, 0, 0}
};

FlagTable type_flags[] = {
  {"none", ITEM_NONE, false}
  ,
  {"light", ITEM_LIGHT, true}
  ,
  {"scroll", ITEM_SCROLL, true}
  ,
  {"wand", ITEM_WAND, true}
  ,
  {"staff", ITEM_STAFF, true}
  ,
  {"weapon", ITEM_WEAPON, true}
  ,
  {"treasure", ITEM_TREASURE, true}
  ,
  {"armor", ITEM_ARMOR, true}
  ,
  {"potion", ITEM_POTION, true}
  ,
  {"furniture", ITEM_FURNITURE, true}
  ,
  {"trash", ITEM_TRASH, true}
  ,
  {"container", ITEM_CONTAINER, true}
  ,
  {"drinkcontainer", ITEM_DRINK_CON, true}
  ,
  {"key", ITEM_KEY, true}
  ,
  {"food", ITEM_FOOD, true}
  ,
  {"money", ITEM_MONEY, true}
  ,
  {"boat", ITEM_BOAT, true}
  ,
  {"npc_corpse", ITEM_CORPSE_NPC, true}
  ,
  {"pc_corpse", ITEM_CORPSE_PC, false}
  ,
  {"fountain", ITEM_FOUNTAIN, true}
  ,
  {"pill", ITEM_PILL, true}
  ,
  {"protect", ITEM_PROTECT, true}
  ,
  {"map", ITEM_MAP, true}
  ,
  {"portal", ITEM_PORTAL, true}
  ,
  {"warpstone", ITEM_WARP_STONE, true}
  ,
  {"room_key", ITEM_ROOM_KEY, true}
  ,
  {"gem", ITEM_GEM, true}
  ,
  {"jewelry", ITEM_JEWELRY, true}
  ,
  {"jukebox", ITEM_JUKEBOX, true}
  ,
  {"clothing", ITEM_CLOTHING, true}
  ,
  {NULL, 0, 0}
};

FlagTable extra_flags[] = {
  {"glow", ITEM_GLOW, true}
  ,
  {"hum", ITEM_HUM, true}
  ,
  {"dark", ITEM_DARK, true}
  ,
  {"lock", ITEM_LOCK, true}
  ,
  {"evil", ITEM_EVIL, true}
  ,
  {"invis", ITEM_INVIS, true}
  ,
  {"magic", ITEM_MAGIC, true}
  ,
  {"nodrop", ITEM_NODROP, true}
  ,
  {"bless", ITEM_BLESS, true}
  ,
  {"antigood", ITEM_ANTI_GOOD, true}
  ,
  {"antievil", ITEM_ANTI_EVIL, true}
  ,
  {"antineutral", ITEM_ANTI_NEUTRAL, true}
  ,
  {"noremove", ITEM_NOREMOVE, true}
  ,
  {"inventory", ITEM_INVENTORY, true}
  ,
  {"nopurge", ITEM_NOPURGE, true}
  ,
  {"rotdeath", ITEM_ROT_DEATH, true}
  ,
  {"visdeath", ITEM_VIS_DEATH, true}
  ,
  {"nonmetal", ITEM_NONMETAL, true}
  ,
  {"meltdrop", ITEM_MELT_DROP, true}
  ,
  {"hadtimer", ITEM_HAD_TIMER, true}
  ,
  {"sellextract", ITEM_SELL_EXTRACT, true}
  ,
  {"burnproof", ITEM_BURN_PROOF, true}
  ,
  {"nouncurse", ITEM_NOUNCURSE, true}
  ,
  {"quest", ITEM_QUEST, true}
  ,
  {"auctioned", ITEM_AUCTIONED, false}
  ,
  {NULL, 0, 0}
};

FlagTable wear_flags[] = {
  {"take", ITEM_TAKE, true}
  ,
  {"finger", ITEM_WEAR_FINGER, true}
  ,
  {"neck", ITEM_WEAR_NECK, true}
  ,
  {"body", ITEM_WEAR_BODY, true}
  ,
  {"head", ITEM_WEAR_HEAD, true}
  ,
  {"legs", ITEM_WEAR_LEGS, true}
  ,
  {"feet", ITEM_WEAR_FEET, true}
  ,
  {"hands", ITEM_WEAR_HANDS, true}
  ,
  {"arms", ITEM_WEAR_ARMS, true}
  ,
  {"shield", ITEM_WEAR_SHIELD, true}
  ,
  {"about", ITEM_WEAR_ABOUT, true}
  ,
  {"waist", ITEM_WEAR_WAIST, true}
  ,
  {"wrist", ITEM_WEAR_WRIST, true}
  ,
  {"wield", ITEM_WIELD, true}
  ,
  {"hold", ITEM_HOLD, true}
  ,
  {"nosac", ITEM_NO_SAC, true}
  ,
  {"wearfloat", ITEM_WEAR_FLOAT, true}
  ,

  {NULL, 0, 0}
};


FlagTable apply_flags[] = {
  {"none", APPLY_NONE, true}
  ,
  {"strength", APPLY_STR, true}
  ,
  {"dexterity", APPLY_DEX, true}
  ,
  {"intelligence", APPLY_INT, true}
  ,
  {"wisdom", APPLY_WIS, true}
  ,
  {"constitution", APPLY_CON, true}
  ,
  {"sex", APPLY_SEX, true}
  ,
  {"class", APPLY_CLASS, true}
  ,
  {"level", APPLY_LEVEL, true}
  ,
  {"age", APPLY_AGE, true}
  ,
  {"height", APPLY_HEIGHT, true}
  ,
  {"weight", APPLY_WEIGHT, true}
  ,
  {"mana", APPLY_MANA, true}
  ,
  {"hp", APPLY_HIT, true}
  ,
  {"move", APPLY_MOVE, true}
  ,
  {"gold", APPLY_GOLD, true}
  ,
  {"experience", APPLY_EXP, true}
  ,
  {"ac", APPLY_AC, true}
  ,
  {"hitroll", APPLY_HITROLL, true}
  ,
  {"damroll", APPLY_DAMROLL, true}
  ,
  {"saves", APPLY_SAVES, true}
  ,
  {"savingpara", APPLY_SAVING_PARA, true}
  ,
  {"savingrod", APPLY_SAVING_ROD, true}
  ,
  {"savingpetri", APPLY_SAVING_PETRI, true}
  ,
  {"savingbreath", APPLY_SAVING_BREATH, true}
  ,
  {"savingspell", APPLY_SAVING_SPELL, true}
  ,
  {"spellaffect", APPLY_SPELL_AFFECT, false}
  ,
  {NULL, 0, 0}
};


FlagTable wear_loc_strings[] = {
  {"none", WEAR_NONE, true}
  ,
  {"as a light", WEAR_LIGHT, true}
  ,
  {"on the left finger", WEAR_FINGER_L, true}
  ,
  {"on the right finger", WEAR_FINGER_R, true}
  ,
  {"around the neck (1)", WEAR_NECK_1, true}
  ,
  {"around the neck (2)", WEAR_NECK_2, true}
  ,
  {"on the body", WEAR_BODY, true}
  ,
  {"over the head", WEAR_HEAD, true}
  ,
  {"on the legs", WEAR_LEGS, true}
  ,
  {"on the feet", WEAR_FEET, true}
  ,
  {"on the hands", WEAR_HANDS, true}
  ,
  {"on the arms", WEAR_ARMS, true}
  ,
  {"as a shield", WEAR_SHIELD, true}
  ,
  {"about the shoulders", WEAR_ABOUT, true}
  ,
  {"around the waist", WEAR_WAIST, true}
  ,
  {"on the left wrist", WEAR_WRIST_L, true}
  ,
  {"on the right wrist", WEAR_WRIST_R, true}
  ,
  {"wielded", WEAR_WIELD, true}
  ,
  {"held in the hands", WEAR_HOLD, true}
  ,
  {"floating nearby", WEAR_FLOAT, true}
  ,
  {"dual wielded", WEAR_SECONDARY}
  ,
  {NULL, 0, 0}
};

FlagTable wear_loc_flags[] = {
  {"none", WEAR_NONE, true}
  ,
  {"light", WEAR_LIGHT, true}
  ,
  {"lfinger", WEAR_FINGER_L, true}
  ,
  {"rfinger", WEAR_FINGER_R, true}
  ,
  {"neck1", WEAR_NECK_1, true}
  ,
  {"neck2", WEAR_NECK_2, true}
  ,
  {"body", WEAR_BODY, true}
  ,
  {"head", WEAR_HEAD, true}
  ,
  {"legs", WEAR_LEGS, true}
  ,
  {"feet", WEAR_FEET, true}
  ,
  {"hands", WEAR_HANDS, true}
  ,
  {"arms", WEAR_ARMS, true}
  ,
  {"shield", WEAR_SHIELD, true}
  ,
  {"about", WEAR_ABOUT, true}
  ,
  {"waist", WEAR_WAIST, true}
  ,
  {"lwrist", WEAR_WRIST_L, true}
  ,
  {"rwrist", WEAR_WRIST_R, true}
  ,
  {"wielded", WEAR_WIELD, true}
  ,
  {"hold", WEAR_HOLD, true}
  ,
  {"floating", WEAR_FLOAT, true}
  ,
  {"secondary", WEAR_SECONDARY, true}
  ,
  {NULL, 0, 0}
};

FlagTable container_flags[] = {
  {"closeable", CONT_CLOSEABLE, true}
  ,
  {"pickproof", CONT_PICKPROOF, true}
  ,
  {"closed", CONT_CLOSED, true}
  ,
  {"locked", CONT_LOCKED, true}
  ,
  {"puton", CONT_PUT_ON, true}
  ,
  {NULL, 0, 0}
};



FlagTable ac_type[] = {
  {"pierce", AC_PIERCE, true}
  ,
  {"bash", AC_BASH, true}
  ,
  {"slash", AC_SLASH, true}
  ,
  {"exotic", AC_EXOTIC, true}
  ,
  {NULL, 0, 0}
};

FlagTable size_flags[] = {
  {"tiny", SIZE_TINY, true}
  ,
  {"small", SIZE_SMALL, true}
  ,
  {"medium", SIZE_MEDIUM, true}
  ,
  {"large", SIZE_LARGE, true}
  ,
  {"huge", SIZE_HUGE, true}
  ,
  {"giant", SIZE_GIANT, true}
  ,
  {NULL, 0, 0}
  ,
};

FlagTable weapon_types[] = {
  {"exotic", WEAPON_EXOTIC, true}
  ,
  {"sword", WEAPON_SWORD, true}
  ,
  {"dagger", WEAPON_DAGGER, true}
  ,
  {"spear", WEAPON_SPEAR, true}
  ,
  {"mace", WEAPON_MACE, true}
  ,
  {"axe", WEAPON_AXE, true}
  ,
  {"flail", WEAPON_FLAIL, true}
  ,
  {"whip", WEAPON_WHIP, true}
  ,
  {"polearm", WEAPON_POLEARM, true}
  ,
  {NULL, 0, 0}
};

FlagTable weapon_flags[] = {
  {"flaming", WEAPON_FLAMING, true}
  ,
  {"frost", WEAPON_FROST, true}
  ,
  {"vampiric", WEAPON_VAMPIRIC, true}
  ,
  {"sharp", WEAPON_SHARP, true}
  ,
  {"vorpal", WEAPON_VORPAL, true}
  ,
  {"twohands", WEAPON_TWO_HANDS, true}
  ,
  {"shocking", WEAPON_SHOCKING, true}
  ,
  {"poison", WEAPON_POISON, true}
  ,
  {NULL, 0, 0}
};

FlagTable res_flags[] = {
  {"summon", RES_SUMMON, true}
  ,
  {"charm", RES_CHARM, true}
  ,
  {"magic", RES_MAGIC, true}
  ,
  {"weapon", RES_WEAPON, true}
  ,
  {"bash", RES_BASH, true}
  ,
  {"pierce", RES_PIERCE, true}
  ,
  {"slash", RES_SLASH, true}
  ,
  {"fire", RES_FIRE, true}
  ,
  {"cold", RES_COLD, true}
  ,
  {"lightning", RES_LIGHTNING, true}
  ,
  {"acid", RES_ACID, true}
  ,
  {"poison", RES_POISON, true}
  ,
  {"negative", RES_NEGATIVE, true}
  ,
  {"holy", RES_HOLY, true}
  ,
  {"energy", RES_ENERGY, true}
  ,
  {"mental", RES_MENTAL, true}
  ,
  {"disease", RES_DISEASE, true}
  ,
  {"drowning", RES_DROWNING, true}
  ,
  {"light", RES_LIGHT, true}
  ,
  {"sound", RES_SOUND, true}
  ,
  {"wood", RES_WOOD, true}
  ,
  {"silver", RES_SILVER, true}
  ,
  {"iron", RES_IRON, true}
  ,
  {NULL, 0, 0}
};

FlagTable vuln_flags[] = {
  {"summon", VULN_SUMMON, true}
  ,
  {"charm", VULN_CHARM, true}
  ,
  {"magic", VULN_MAGIC, true}
  ,
  {"weapon", VULN_WEAPON, true}
  ,
  {"bash", VULN_BASH, true}
  ,
  {"pierce", VULN_PIERCE, true}
  ,
  {"slash", VULN_SLASH, true}
  ,
  {"fire", VULN_FIRE, true}
  ,
  {"cold", VULN_COLD, true}
  ,
  {"lightning", VULN_LIGHTNING, true}
  ,
  {"acid", VULN_ACID, true}
  ,
  {"poison", VULN_POISON, true}
  ,
  {"negative", VULN_NEGATIVE, true}
  ,
  {"holy", VULN_HOLY, true}
  ,
  {"energy", VULN_ENERGY, true}
  ,
  {"mental", VULN_MENTAL, true}
  ,
  {"disease", VULN_DISEASE, true}
  ,
  {"drowning", VULN_DROWNING, true}
  ,
  {"light", VULN_LIGHT, true}
  ,
  {"sound", VULN_SOUND, true}
  ,
  {"wood", VULN_WOOD, true}
  ,
  {"silver", VULN_SILVER, true}
  ,
  {"iron", VULN_IRON, true}
  ,
  {NULL, 0, 0}
};

FlagTable position_flags[] = {
  {"dead", POS_DEAD, true}
  ,
  {"mortal", POS_MORTAL, false}
  ,
  {"incap", POS_INCAP, false}
  ,
  {"stunned", POS_STUNNED, false}
  ,
  {"sleeping", POS_SLEEPING, true}
  ,
  {"resting", POS_RESTING, true}
  ,
  {"sitting", POS_SITTING, true}
  ,
  {"fighting", POS_FIGHTING, true}
  ,
  {"standing", POS_STANDING, true}
  ,
  {NULL, 0, 0}
};

FlagTable portal_flags[] = {
  {"normal_exit", GATE_NORMAL_EXIT, true}
  ,
  {"no_curse", GATE_NOCURSE, true}
  ,
  {"go_with", GATE_GOWITH, true}
  ,
  {"buggy", GATE_BUGGY, true}
  ,
  {"random", GATE_RANDOM, true}
  ,
  {NULL, 0, 0}
};

FlagTable furniture_flags[] = {
  {"stand_at", STAND_AT, true}
  ,
  {"stand_on", STAND_ON, true}
  ,
  {"stand_in", STAND_IN, true}
  ,
  {"sit_at", SIT_AT, true}
  ,
  {"sit_on", SIT_ON, true}
  ,
  {"sit_in", SIT_IN, true}
  ,
  {"rest_at", REST_AT, true}
  ,
  {"rest_on", REST_ON, true}
  ,
  {"rest_in", REST_IN, true}
  ,
  {"sleep_at", SLEEP_AT, true}
  ,
  {"sleep_on", SLEEP_ON, true}
  ,
  {"sleep_in", SLEEP_IN, true}
  ,
  {"put_at", PUT_AT, true}
  ,
  {"put_on", PUT_ON, true}
  ,
  {"put_in", PUT_IN, true}
  ,
  {"put_inside", PUT_INSIDE, true}
  ,
  {NULL, 0, 0}
};

FlagTable apply_types[] = {
  {"affects", TO_AFFECTS, true}
  ,
  {"object", TO_OBJECT, true}
  ,
  {"immune", TO_IMMUNE, true}
  ,
  {"resist", TO_RESIST, true}
  ,
  {"vuln", TO_VULN, true}
  ,
  {"weapon", TO_WEAPON, true}
  ,
  {NULL, 0, true}
};

const struct bit_type bitvector_type[] = {
  {
   affect_flags, "affect"},
  {apply_flags, "apply"},
  {imm_flags, "imm"},
  {res_flags, "res"},
  {vuln_flags, "vuln"},
  {weapon_flags, "weapon"}
};

FlagTable color_attributes[] = {
  {"default", CL_NONE, true},
  {"bright", CL_BRIGHT, true},
  {"dim", CL_DIM, true},
  {"standout", CL_STANDOUT, true},
  {"underscore", CL_UNDER, true},
  {"blink", CL_BLINK, true},
  {"italic", CL_ITALIC, true},
  {"reverse", CL_REVERSE, true},
  {"hidden", CL_REVERSE, true},
  {"random", CL_RANDOM, true},
  {NULL, 0, false}
};

FlagTable color_foregrounds[] = {
  {"black", FG_BLACK, true},
  {"red", FG_RED, true},
  {"green", FG_GREEN, true},
  {"yellow", FG_YELLOW, true},
  {"blue", FG_BLUE, true},
  {"magenta", FG_MAGENTA, true},
  {"cyan", FG_CYAN, true},
  {"white", FG_WHITE, true},
  {"random", FG_RANDOM, true},
  {"none", FG_NONE, true},
  {NULL, 0, false}
};

FlagTable color_backgrounds[] = {
  {"black", BG_BLACK, true},
  {"red", BG_RED, true},
  {"green", BG_GREEN, true},
  {"yellow", BG_YELLOW, true},
  {"blue", BG_BLUE, true},
  {"magenta", BG_MAGENTA, true},
  {"cyan", BG_CYAN, true},
  {"white", BG_WHITE, true},
  {"random", BG_RANDOM, true},
  {"none", BG_NONE, true},
  {NULL, 0, false}
};

const struct color_type color_table[MAX_CUSTOM_COLOR] = {
  {"Clear", _DEFAULT, {CL_NONE, FG_NONE, BG_NONE}
   },
  {"Gossip", _GOSSIP, {CL_BRIGHT, FG_MAGENTA, BG_NONE}},
  {"Music", _MUSIC, {CL_BRIGHT, FG_RED, BG_NONE}},
  {"Q/A", _QA, {CL_NONE, FG_YELLOW, BG_NONE}},
  {"Quote", _QUOTE, {CL_BRIGHT, FG_WHITE, BG_NONE}},
  {"Gratz", _GRATS, {CL_NONE, FG_GREEN, BG_NONE}},
  {"Shout1", _SHOUT1, {CL_NONE, FG_WHITE, BG_NONE}},
  {"Shout2", _SHOUT2, {CL_NONE, FG_MAGENTA, BG_NONE}},
  {"ImmTalk", _IMMTALK, {CL_NONE, FG_CYAN, BG_NONE}},
  {"Tells1", _TELLS1, {CL_NONE, FG_CYAN, BG_NONE}},
  {"Tells2", _TELLS2, {CL_BRIGHT, FG_CYAN, BG_NONE}},
  {"Say1", _SAY1, {CL_NONE, FG_GREEN, BG_NONE}},
  {"Say2", _SAY2, {CL_BRIGHT, FG_GREEN, BG_NONE}},
  {"Skill", _SKILL, {CL_BRIGHT, FG_YELLOW, BG_NONE}},
  {"YHit", _YHIT, {CL_NONE, FG_GREEN, BG_NONE}},
  {"OHit", _OHIT, {CL_NONE, FG_BLUE, BG_NONE}},
  {"VHit", _VHIT, {CL_NONE, FG_RED, BG_NONE}},
  {"WhoRace", _WRACE, {CL_BRIGHT, FG_RED, BG_NONE}},
  {"WhoClass", _WCLASS, {CL_BRIGHT, FG_CYAN, BG_NONE}},
  {"WhoLevel", _WLEVEL, {CL_BRIGHT, FG_BLUE, BG_NONE}},
  {"RoomTitle", _RTITLE, {CL_BRIGHT, FG_GREEN, BG_NONE}},
  {"Score1", _SCORE1, {CL_NONE, FG_CYAN, BG_NONE}},
  {"Score2", _SCORE2, {CL_BRIGHT, FG_CYAN, BG_NONE}},
  {"Score3", _SCORE3, {CL_BRIGHT, FG_WHITE, BG_NONE}},
  {"Score4", _SCOREB, {CL_NONE, FG_NONE, BG_NONE}},
  {"Wiznet", _WIZNET, {CL_NONE, FG_GREEN, BG_NONE}},
  {"Gtell1", _GTELL1, {CL_BRIGHT, FG_BLUE, BG_NONE}},
  {"Gtell2", _GTELL2, {CL_BRIGHT, FG_MAGENTA, BG_NONE}},
  {"Btalk", _BTALK, {CL_BRIGHT, FG_BLUE, BG_NONE}},
  {"WhoSex", _WSEX, {CL_NONE, FG_GREEN, BG_NONE}}
};

const struct vnum_type vnum_table[] = {
#define MAKE_VNUM_TABLE    	1
#include "vnums.h"
#undef MAKE_VNUM_TABLE
};

FlagTable info_flags[] = {
  {"Notes", INFO_NOTE, true},
  {"Levels", INFO_LEVEL, true},
  {"Deaths", INFO_DEATH, true},
  {"Logins", INFO_LOGIN, true},
  {"Logouts", INFO_LOGOUT, true},
  {"War", INFO_WAR, true},
  {"Gquest", INFO_GQUEST, true},
  {"Auction", INFO_AUCTION, true},
  {"Misc", INFO_MISC, true},
  {NULL, 0, 0}
};

FlagTable clan_flags[] = {
  {"independent", CLAN_INDEPENDENT, true},
  {"no_pk", CLAN_NO_PK, true},
  {"immortal", CLAN_IMMORTAL, true},
  {"favoured", CLAN_FAVOURED, true},
  {"changed", CLAN_CHANGED, true},
  {"deleted", CLAN_DELETED, true},
  {NULL, 0, 0}
};

FlagTable log_flags[] = {
  {"normal", LOG_NORMAL, true},
  {"always", LOG_ALWAYS, true},
  {"never", LOG_NEVER, true},
  {NULL, 0, false}
};

FlagTable target_flags[] = {
  {"ignore", TAR_IGNORE, true},
  {"char_offensive", TAR_CHAR_OFFENSIVE, true},
  {"char_defensive", TAR_CHAR_DEFENSIVE, true},
  {"char_self", TAR_CHAR_SELF, true},
  {"obj_inventory", TAR_OBJ_INV, true},
  {"obj_char_defensive", TAR_OBJ_CHAR_DEF, true},
  {"obj_char_offensive", TAR_OBJ_CHAR_OFF, true},
  {NULL, 0, 0}
};

const struct tzone_type tzone_table[MAX_TZONE] = {
  {"IDLW", "International Date Line West", -(12 * HOUR), 0},
  {"NT", "Nome", -(11 * HOUR), 0},
  {"HST", "Hawaii Standard", -(10 * HOUR), -9},
  {"AKST", "Alaska Standard", -(HOUR * 9), -(HOUR * 8)},
  {"PST", "Pacific Standard", -(HOUR * 8), -(HOUR * 7)},
  {"MST", "Mountain Standard", -(HOUR * 7), -(HOUR * 6)},
  {"CST", "Central Standard", -(HOUR * 6), -(HOUR * 5)},
  {"EST", "Eastern Standard", -(HOUR * 5), -(HOUR * 4)},
  {"AST", "Atlantic Standard", -(HOUR * 4), -(HOUR * 3)},

  {"NFT", "Newfoundland", -(HOUR * 2 + (MINUTE * 30)),
   -(HOUR * 2 + (MINUTE * 30))},
  {"BRST", "Brazil Standard", -(HOUR * 3), -(HOUR * 2)},
  {"AT", "Mid-Atlantic", -(HOUR * 2), -(HOUR)},
  {"WAT", "West Africa", -(HOUR), 0},
  {"WET", "Western European", 0, 0},
  {"CET", "Central European", (HOUR * 1), 0},
  {"EET", "Eastern Europe, USSR Zone 1", (HOUR * 2), (HOUR * 1)},
  {"BT", "Baghdad, USSR Zone 2", (HOUR * 3), (HOUR * 3)},
  {"IT", "Iran", (HOUR * 3 + (MINUTE * 30)), (HOUR * 3 + (MINUTE * 30))},
  {"ZP4", "USSR Zone 3", (HOUR * 4), (HOUR * 4)},
  {"ZP5", "USSR Zone 4", (HOUR * 5), (HOUR * 5)},

  {"IST", "Indian Standard", (HOUR * 5 + (MINUTE * 30)),
   (HOUR * 5 + (MINUTE * 30))},
  {"ZP6", "AUSSR Zone 5", (HOUR * 6), (HOUR * 6)},

  {"NST", "North Sumatra", (HOUR * 6 + (MINUTE * 30)),
   (HOUR * 6 + (MINUTE * 30))},
  {"JAVT", "Java", (HOUR * 7), (HOUR * 7)},
  {"AWST", "Australian Western Standard", (HOUR * 8), (HOUR * 8)},
  {"JST", "Japan Standard, USSR Zone 8", (HOUR * 9), (HOUR * 9)},

  {"ACST", "Australian Central Standard", (HOUR * 9 + (MINUTE * 30)),
   (HOUR * 9 + (MINUTE * 30))},
  {"AEST", "Australian Eastern Standard", (HOUR * 10), (HOUR * 11)},
  {"AEDT", "Australian Eastern Daylight", (HOUR * 11), (HOUR * 11)},
  {"NZST", "New Zealand Standard", (HOUR * 12), (HOUR * 12)}
};

FlagTable chan_types[] = {
  {"public", spec_public_flag, true},
  {"clan", spec_clan_flag, true},
  {"immortal", spec_imm_flag, true},
  {"buddy", spec_buddy_flag, true},
  {"none", spec_none, false},
  {NULL, 0, 0}
};

FlagTable mud_flags[] = {
  {"wizlock", MUD_WIZLOCK, true},
  {"newlock", MUD_NEWLOCK, true},
  {"logall", MUD_LOGALL, true},
  {"nodnslookups", NO_DNS_LOOKUPS, true},
  {"verbose_resets", VERBOSE_RESETS, true},
  {NULL, 0, 0}
};

FlagTable ignore_flags[] = {
  {"none", 0, true},
  {"channels", IGNORE_CHANNELS, true},
  {"notes", IGNORE_NOTES, true},
  {"tells", IGNORE_TELLS, true},
  {"announce", IGNORE_ANNOUNCE, true},
  {"levels", IGNORE_LEVELS, true},
  {"socials", IGNORE_SOCIALS, true},
  {"all", IGNORE_ALL, true},
  {"says", IGNORE_SAY, true},
  {NULL, 0, false}
};

const struct directory_type directories_table[] = {
  {"Data files.", DATA_DIR},
  {"Area files.", AREA_DIR},
  {"Binary files.", BIN_DIR},
  {"Log files.", LOG_DIR},
  {"Note files.", NOTE_DIR},
#ifndef DISABLE_I3
  {"I3 files.", I3DIR},
#endif
#if defined(NO_INITIAL_ALPHA_PFILEDIRS)
  {"player files", PLAYER_DIR},
  {"backup player files", PLAYER_BACKUP},
#else


  {"player files root", PLAYER_DIR},
  {"player files - A", PLAYER_DIR "a" DIR_SYM},
  {"player files - B", PLAYER_DIR "b" DIR_SYM},
  {"player files - C", PLAYER_DIR "c" DIR_SYM},
  {"player files - D", PLAYER_DIR "d" DIR_SYM},
  {"player files - E", PLAYER_DIR "e" DIR_SYM},
  {"player files - F", PLAYER_DIR "f" DIR_SYM},
  {"player files - G", PLAYER_DIR "g" DIR_SYM},
  {"player files - H", PLAYER_DIR "h" DIR_SYM},
  {"player files - I", PLAYER_DIR "i" DIR_SYM},
  {"player files - J", PLAYER_DIR "j" DIR_SYM},
  {"player files - K", PLAYER_DIR "k" DIR_SYM},
  {"player files - L", PLAYER_DIR "l" DIR_SYM},
  {"player files - M", PLAYER_DIR "m" DIR_SYM},
  {"player files - N", PLAYER_DIR "n" DIR_SYM},
  {"player files - O", PLAYER_DIR "o" DIR_SYM},
  {"player files - P", PLAYER_DIR "p" DIR_SYM},
  {"player files - Q", PLAYER_DIR "q" DIR_SYM},
  {"player files - R", PLAYER_DIR "r" DIR_SYM},
  {"player files - S", PLAYER_DIR "s" DIR_SYM},
  {"player files - T", PLAYER_DIR "t" DIR_SYM},
  {"player files - U", PLAYER_DIR "u" DIR_SYM},
  {"player files - V", PLAYER_DIR "v" DIR_SYM},
  {"player files - W", PLAYER_DIR "w" DIR_SYM},
  {"player files - X", PLAYER_DIR "x" DIR_SYM},
  {"player files - Y", PLAYER_DIR "y" DIR_SYM},
  {"player files - Z", PLAYER_DIR "z" DIR_SYM},
  {"backup player files root", PLAYER_BACKUP},
  {"backup player files - A", PLAYER_BACKUP "a" DIR_SYM},
  {"backup player files - B", PLAYER_BACKUP "b" DIR_SYM},
  {"backup player files - C", PLAYER_BACKUP "c" DIR_SYM},
  {"backup player files - D", PLAYER_BACKUP "d" DIR_SYM},
  {"backup player files - E", PLAYER_BACKUP "e" DIR_SYM},
  {"backup player files - F", PLAYER_BACKUP "f" DIR_SYM},
  {"backup player files - G", PLAYER_BACKUP "g" DIR_SYM},
  {"backup player files - H", PLAYER_BACKUP "h" DIR_SYM},
  {"backup player files - I", PLAYER_BACKUP "i" DIR_SYM},
  {"backup player files - J", PLAYER_BACKUP "j" DIR_SYM},
  {"backup player files - K", PLAYER_BACKUP "k" DIR_SYM},
  {"backup player files - L", PLAYER_BACKUP "l" DIR_SYM},
  {"backup player files - M", PLAYER_BACKUP "m" DIR_SYM},
  {"backup player files - N", PLAYER_BACKUP "n" DIR_SYM},
  {"backup player files - O", PLAYER_BACKUP "o" DIR_SYM},
  {"backup player files - P", PLAYER_BACKUP "p" DIR_SYM},
  {"backup player files - Q", PLAYER_BACKUP "q" DIR_SYM},
  {"backup player files - R", PLAYER_BACKUP "r" DIR_SYM},
  {"backup player files - S", PLAYER_BACKUP "s" DIR_SYM},
  {"backup player files - T", PLAYER_BACKUP "t" DIR_SYM},
  {"backup player files - U", PLAYER_BACKUP "u" DIR_SYM},
  {"backup player files - V", PLAYER_BACKUP "v" DIR_SYM},
  {"backup player files - W", PLAYER_BACKUP "w" DIR_SYM},
  {"backup player files - X", PLAYER_BACKUP "x" DIR_SYM},
  {"backup player files - Y", PLAYER_BACKUP "y" DIR_SYM},
  {"backup player files - Z", PLAYER_BACKUP "z" DIR_SYM},
#endif
  {"Pfiles of deleters above lvl 5", DELETE_DIR},
  {"", ""}
};

FlagTable autoset_types[] = {
  {"default", AUTODEFAULT, true},
  {"easy", AUTOEASY, true},
  {"normal", AUTOSET, true},
  {"hard", AUTOHARD, true},
  {"random", AUTORANDOM, true},
  {NULL, 0, 0}
};

FlagTable board_flags[] = {
  {"changed", BOARD_CHANGED, true},
  {"noweb", BOARD_NOWEB, true},
  {NULL, 0, 0}
};

FlagTable cmd_flags[] = {
  {"none", CMD_NONE, true},
  {"deleted", CMD_DELETED, false},
  {"noprefix", CMD_NOPREFIX, true},
  {"no_order", CMD_NO_ORDER, true},
  {"noalias", CMD_NOALIAS, true},
  {NULL, 0, 0}
};

FlagTable stat_types[] = {
  {"strength", STAT_STR, true},
  {"intelligence", STAT_INT, true},
  {"wisdom", STAT_WIS, true},
  {"dexterity", STAT_DEX, true},
  {"constitution", STAT_CON, true},
  {NULL, STAT_MAX, 0}
};

const struct wear_type wear_table[] = {
  {
   WEAR_NONE, ITEM_TAKE},
  {WEAR_LIGHT, ITEM_LIGHT},
  {WEAR_FINGER_L, ITEM_WEAR_FINGER},
  {WEAR_FINGER_R, ITEM_WEAR_FINGER},
  {WEAR_NECK_1, ITEM_WEAR_NECK},
  {WEAR_NECK_2, ITEM_WEAR_NECK},
  {WEAR_BODY, ITEM_WEAR_BODY},
  {WEAR_HEAD, ITEM_WEAR_HEAD},
  {WEAR_LEGS, ITEM_WEAR_LEGS},
  {WEAR_FEET, ITEM_WEAR_FEET},
  {WEAR_HANDS, ITEM_WEAR_HANDS},
  {WEAR_ARMS, ITEM_WEAR_ARMS},
  {WEAR_SHIELD, ITEM_WEAR_SHIELD},
  {WEAR_ABOUT, ITEM_WEAR_ABOUT},
  {WEAR_WAIST, ITEM_WEAR_WAIST},
  {WEAR_WRIST_L, ITEM_WEAR_WRIST},
  {WEAR_WRIST_R, ITEM_WEAR_WRIST},
  {WEAR_WIELD, ITEM_WIELD},
  {WEAR_HOLD, ITEM_HOLD},
  {(wloc_t) - 2, -2}
};

const struct stance_type stance_table[MAX_STANCE] = {
  {"normal", STANCE_NORMAL, {STANCE_NONE, STANCE_NONE},
   "You drop into a general fighting stance.",
   "$n drops into a general fighting stance."},
  {"viper", STANCE_VIPER, {STANCE_NORMAL, STANCE_NORMAL},
   "You arch your body into the viper fighting stance.",
   "$n arches $s body into the viper fighting stance."},
  {"crane", STANCE_CRANE, {STANCE_NORMAL, STANCE_NORMAL},
   "You swing your body into the crane fighting stance.",
   "$n swings $s body into the crane fighting stance."},
  {"crab", STANCE_CRAB, {STANCE_NORMAL, STANCE_NORMAL},
   "You squat down into the crab fighting stance.",
   "$n squats down into the crab fighting stance."},
  {"mongoose", STANCE_MONGOOSE, {STANCE_NORMAL, STANCE_NORMAL},
   "You twist into the mongoose fighting stance.",
   "$n twists into the mongoose fighting stance."},
  {"bull", STANCE_BULL, {STANCE_NORMAL, STANCE_NORMAL},
   "You hunch down into the bull fighting stance.",
   "$n hunches down into the bull fighting stance."},
  {"mantis", STANCE_MANTIS, {STANCE_CRANE, STANCE_VIPER},
   "You spin your body into the mantis fighting stance.",
   "$n spins $s body into the mantis fighting stance."},
  {"dragon", STANCE_DRAGON, {STANCE_BULL, STANCE_CRAB},
   "You coil your body into the dragon fighting stance.",
   "$n coils $s body into the dragon fighting stance."},
  {"tiger", STANCE_TIGER, {STANCE_BULL, STANCE_VIPER},
   "You lunge into the tiger fighting stance.",
   "$n lunges into the tiger fighting stance."},
  {"monkey", STANCE_MONKEY, {STANCE_CRANE, STANCE_MONGOOSE},
   "You rotate your body into the monkey fighting stance.",
   "$n rotates $s body into the monkey fighting stance."},
  {"swallow", STANCE_SWALLOW, {STANCE_CRAB, STANCE_MONGOOSE},
   "You slide into the swallow fighting stance.",
   "$n slides into the swallow fighting stance."},
  {"current", STANCE_CURRENT, {STANCE_CURRENT, STANCE_CURRENT}, "", ""},
  {"autodrop", STANCE_AUTODROP, {STANCE_CURRENT, STANCE_CURRENT}, "", ""}
};

FlagTable signal_flags[] = {
#ifndef WIN32
  {"SIGPIPE", MakeBit (SIGPIPE), true},
  {"SIGCHLD", MakeBit (SIGCHLD), true},
  {"SIGHUP", MakeBit (SIGHUP), true},
  {"SIGQUIT", MakeBit (SIGQUIT), true},
  {"SIGBUS", MakeBit (SIGBUS), true},
  {"SIGUSR1", MakeBit (SIGUSR1), true},
  {"SIGUSR2", MakeBit (SIGUSR2), true},
#else
#define SA_NODEFER 0
#endif
  {"SIGINT", MakeBit (SIGINT), true},
  {"SIGILL", MakeBit (SIGILL), true},
  {"SIGFPE", MakeBit (SIGFPE), true},
  {"SIGSEGV", MakeBit (SIGSEGV), true},
  {"SIGTERM", MakeBit (SIGTERM), true},
  {"SIGABRT", MakeBit (SIGABRT), true},
#ifdef HAVE_SETITIMER
  {"SIGVTALRM", MakeBit (SIGVTALRM), true},
#elif defined HAVE_ALARM
  {"SIGALRM", MakeBit (SIGALRM), true},
#endif
  {NULL, 0, 0}
};

FlagTable cmd_categories[] = {
  {"hidden", CMDCAT_NOSHOW, true},
  {"none", CMDCAT_NONE, true},
  {"immortal", CMDCAT_IMMORTAL, true},
  {"combat", CMDCAT_COMBAT, true},
  {"settings", CMDCAT_SETTINGS, true},
  {"information", CMDCAT_INFO, true},
  {"movement", CMDCAT_MOVEMENT, true},
  {"communication", CMDCAT_COMMUNICATION, true},
  {"miscellaneous", CMDCAT_MISC, true},
  {"olc", CMDCAT_OLC, true},
  {"object", CMDCAT_OBJECT, true},
  {"clan", CMDCAT_CLAN, true},
  {NULL, 0, false}
};

FlagTable msp_types[] = {
  {"", MSP_NONE, false},
  {"none", MSP_NONE, true},
  {"combat", MSP_COMBAT, true},
  {"weather", MSP_WEATHER, true},
  {"skill", MSP_SKILL, true},
  {NULL, 0, false}
};

FlagTable help_types[] = {
  {"unknown", HELP_UNKNOWN, true},
  {"creation", HELP_CREATION, true},
  {"spells", HELP_SPELLS, true},
  {"commands", HELP_COMMANDS, true},
  {"newbie", HELP_NEWBIE, true},
  {"immortal", HELP_IMMORTAL, true},
  {"olc", HELP_OLC, true},
  {"clan", HELP_CLAN, true},
  {NULL, 0, 0}
};

FlagTable to_flags[] = {
  {"room", TO_ROOM, true},
  {"notvict", TO_NOTVICT, true},
  {"vict", TO_VICT, true},
  {"char", TO_CHAR, true},
  {"all", TO_ALL, true},
  {"zone", TO_ZONE, true},
  {NULL, 0, false}
};

FlagTable cmdline_flags[] = {
  {"no_bg_process", CMDLINE_NO_BACKGROUND_PROCESS, true},
  {"no_logfile", CMDLINE_NO_LOGFILE, true},
  {"log_console", CMDLINE_LOG_CONSOLE, true},
  {"quiet", CMDLINE_QUIET, false},
  {"startup_script", CMDLINE_STARTUP_SCRIPT, false},
  {"disable_signals", CMDLINE_DISABLE_SIGNALS, false},
  {"copyover", CMDLINE_COPYOVER, false},
  {"set_port", CMDLINE_SET_PORT, false},
  {NULL, 0, false}
};

FlagTable ethos_types[] = {
  {"lawful-good", ETHOS_LAWFUL_GOOD, true},
  {"neutral-good", ETHOS_NEUTRAL_GOOD, true},
  {"chaotic-good", ETHOS_CHAOTIC_GOOD, true},
  {"lawful-neutral", ETHOS_LAWFUL_NEUTRAL, true},
  {"true-neutral", ETHOS_TRUE_NEUTRAL, true},
  {"chaotic-neutral", ETHOS_EVIL_NEUTRAL, true},
  {"lawful-evil", ETHOS_LAWFUL_EVIL, true},
  {"neutral-evil", ETHOS_NEUTRAL_EVIL, true},
  {"chaotic-evil", ETHOS_CHAOTIC_EVIL, true},
  {NULL, 0, false}
};
