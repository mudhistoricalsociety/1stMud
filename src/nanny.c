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
#include "interp.h"
#include "telnet.h"
#include "recycle.h"
#include "tables.h"
#include "vnums.h"
#include "special.h"

Proto (bool check_playing, (Descriptor *, const char *));
Proto (bool check_reconnect, (Descriptor *, const char *, bool));

Do_Fun (nanny_help)
{
  HelpData *pHelp;
  Buffer *output;
  char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
  int level;

  output = new_buf ();

  if (NullStr (argument))
    argument = "summary";


  argall[0] = '\0';
  while (!NullStr (argument))
    {
      argument = one_argument (argument, argone);
      if (!NullStr (argall))
	strcat (argall, " ");
      strcat (argall, argone);
    }

  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
      level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

      if (level > get_trust (ch) || pHelp->category != HELP_CREATION)
	continue;

      if (is_name (argall, pHelp->keyword))
	{

	  bprint (output, help_text (pHelp->text));
	  break;
	}
    }

  if (!pHelp)
    chprintlnf (ch, "{cNo help on %s.{x", argall);
  else
    sendpage (ch, buf_string (output));
  free_buf (output);
}

Nanny_Fun (HANDLE_CON_GET_TERM)
{
  switch (argument[0])
    {
    case 'y':
    case 'Y':
      SetBit (d->desc_flags, DESC_COLOR);
      d_println (d, "{?Color {cEnabled...{x");
      break;
    case 'n':
    case 'N':
      RemBit (d->desc_flags, DESC_COLOR);
      d_println (d, "Color Disabled...");
      break;
    case 't':
    case 'T':
      d_printlnf (d, NEWLINE "This text should be " ESC
		  "[%d;%dmGREEN" CL_DEFAULT "." NEWLINE
		  "This text should be " ESC "[%d;%dmRED"
		  CL_DEFAULT ".", CL_BRIGHT, FG_GREEN, CL_BRIGHT, FG_RED);
      d_println (d, NEWLINE "Did you see color? (Y)es, (N)o, (T)est: ");
      return;
    default:
      d_println (d, "Please answer (Y)es, (N)o, (T)est: ");
      return;
    }
  show_greeting (d);
  d->connected = CON_GET_NAME;
}

Nanny_Fun (HANDLE_CON_GET_NAME)
{
  CharData *ch;
  bool fOld;
  NameProfile *nl;

  Proto (NameProfile * profile_lookup, (int));
  Proto (char *genname, (NameProfile *));

  if (NullStr (argument))
    {
      d_println (d,
		 "You can't have a blank name, type 'list' to list name generator profiles,"
		 NEWLINE
		 "<#> to generate a list of names based on profile number '#'"
		 NEWLINE
		 "'quit' to disconnect, or type your characters name - try again.");
      d_print (d, mud_info.login_prompt);
      return;
    }
  else if (!str_cmp (argument, "quit"))
    {
      close_socket (d);
      return;
    }
  else if (!str_cmp (argument, "list"))
    {
      int count = 0;

      d_println (d, "{RNAME GENERATOR PROFILES{x");
      for (nl = name_profile_first; nl; nl = nl->next)
	{
	  d_printlnf (d, "{W%2d{c> %s{x", ++count, nl->title);
	}
      d_println (d, "{cEnter a profile number to view generated names:{x");
      return;
    }
  else if (is_number (argument))
    {
      int count = 0, i;
      int profile_num = atoi (argument);
      Column Cd;

      if (profile_num < 1 || profile_num > top_name_profile
	  || (nl = profile_lookup (profile_num)) == NULL)
	{
	  d_printlnf (d, "{cInvalid profile number {W%d{c.{x", profile_num);
	  d_println (d, "{RNAME GENERATOR PROFILES{x");
	  for (nl = name_profile_first; nl; nl = nl->next)
	    {
	      d_printlnf (d, "{W%2d{c> %s{x", ++count, nl->title);
	    }
	  d_println (d,
		     "{cEnter a profile number to view generated names:{x");
	  return;
	}

      set_cols (&Cd, NULL, 4, COLS_DESC, d);
      d_printlnf (d, "{RNAME GENERATOR - %s{x", strupper (nl->title));
      for (i = 0; i < 60; i++)
	{
	  print_cols (&Cd, capitalize (genname (nl)));
	}
      cols_nl (&Cd);

      d_println (d, "{cPlease enter your name:{x");
      d_println
	(d,
	 "{c(to view a list of profiles, type 'list' or enter a profile number){x");
      return;
    }
  else if (!check_parse_name (argument))
    {
      d_println (d, "{cIllegal name, try another.{x" NEWLINE "Name: {x");
      return;
    }

  fOld = load_char_obj (d, argument);
  ch = d->character;

  if (IsSet (ch->act, PLR_DENY))
    {
      logf ("Denying access to %s@%s.", argument, d->host);
      d_println (d, "{cYou are {Rdenied{c access.{x");
      close_socket (d);
      return;
    }

  if (check_ban (d->host, BAN_PERMIT) && !IsSet (ch->act, PLR_PERMIT))
    {
      d_println (d, "{cYour site has been {Rbanned{c from this mud.{x");
      close_socket (d);
      return;
    }

  if (check_reconnect (d, argument, false))
    {
      fOld = true;
    }
  else
    {
      if (IsSet (mud_info.mud_flags, MUD_WIZLOCK) && !IsImmortal (ch))
	{
	  d_println (d, "{cThe game is {Rwizlocked{c.{x");
	  close_socket (d);
	  return;
	}
    }

  if (fOld)
    {

      d_write (d, echo_off_str, 0);
      d_println (d, "{cPassword: {x");
      d->connected = CON_GET_OLD_PASSWORD;
      d->wrong_password_count = 0;
      return;
    }
  else
    {

      if (IsSet (mud_info.mud_flags, MUD_NEWLOCK))
	{
	  d_println (d, "{cThe game is {Rnewlocked{c.{x");
	  close_socket (d);
	  return;
	}

      if (check_ban (d->host, BAN_NEWBIES))
	{
	  d_println (d, "{cNew players are not allowed from your site.{x");
	  close_socket (d);
	  return;
	}

      d_printlnf (d, "{cDid I get that right, %s ({WY{c/{WN{c)? {x",
		  capitalize (argument));
      d->connected = CON_CONFIRM_NEW_NAME;
      return;
    }
}

Nanny_Fun (HANDLE_CON_GET_OLD_PASSWORD)
{
  CharData *ch = CH (d);

  if (str_casecmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      d_print (d, "{cWrong password");
      if (++d->wrong_password_count < 3)
	{
	  d_println (d, " - try again.");
	  d_println (d, "{cPassword: {x");
	  d_println (d, echo_off_str);
	  return;
	}
      else
	{
	  d_println (d, ".");
	}
      close_socket (d);
      return;
    }

  d_write (d, echo_on_str, 0);

  if (check_playing (d, ch->name))
    return;

  if (check_reconnect (d, ch->name, true))
    return;

  new_wiznet (ch, d->host, WIZ_SITES, true, get_trust (ch),
	      "$N@$t has connected.");

  set_desc_flags (d);
  if (IsImmortal (ch))
    {
      d_print
	(d,
	 "{cWould you like to login ({WW{c)izinvis, ({WI{c)ncognito, "
	 "({WB{c)oth, or ({WN{c)ormal?{x" NEWLINE
	 "{c(Include a level number to specify what level){x");
      d->connected = CON_GET_WIZ;
    }
  else
    {
      do_function (ch, &nanny_help, "motd");
      d->connected = CON_READ_MOTD;
    }
}

Nanny_Fun (HANDLE_CON_GET_WIZ)
{
  CharData *ch = CH (d);
  char arg[MIL];
  int wizlevel = Range (LEVEL_IMMORTAL, ch->level, MAX_LEVEL);

  argument = one_argument (argument, arg);

  if (!NullStr (argument) && is_number (argument))
    wizlevel = Range (LEVEL_IMMORTAL, atoi (argument), MAX_LEVEL);
  switch (tolower (*arg))
    {
    case 'w':
      ch->invis_level = wizlevel;
      ch->incog_level = 0;
      break;
    case 'i':
      ch->incog_level = wizlevel;
      ch->invis_level = 0;
      break;
    case 'b':
      ch->incog_level = wizlevel;
      ch->invis_level = wizlevel;
      break;
    default:
    case 'n':
      ch->incog_level = 0;
      ch->invis_level = 0;
      break;
    }
  do_function (ch, &nanny_help, "imotd");
  d->connected = CON_READ_MOTD;
}

Nanny_Fun (HANDLE_CON_BREAK_CONNECT)
{
  CharData *ch = CH (d);
  Descriptor *d_old, *pd_next;

  switch (*argument)
    {
    case 'y':
    case 'Y':
      for (d_old = descriptor_first; d_old != NULL; d_old = pd_next)
	{
	  pd_next = d_old->next;
	  if (d_old == d || d_old->character == NULL)
	    continue;

	  if (str_cmp (ch->name, CH (d_old)->name))
	    continue;

	  close_socket (d_old);
	}
      if (check_reconnect (d, ch->name, true))
	return;
      d_println (d, "{cReconnect attempt failed.{x" NEWLINE "{cName: ");
      if (d->character != NULL)
	{
	  free_char (d->character);

	  d->character = NULL;
	}
      d->connected = CON_GET_NAME;
      break;

    case 'n':
    case 'N':
      d_println (d, "{cName: {x");
      if (d->character != NULL)
	{
	  free_char (d->character);

	  d->character = NULL;
	}
      d->connected = CON_GET_NAME;
      break;

    default:
      d_println (d, "{cPlease type ({WY{c)es or ({WN{c)o: {x");
      break;
    }
}

Nanny_Fun (HANDLE_CON_CONFIRM_NEW_NAME)
{
  CharData *ch = CH (d);

  switch (*argument)
    {
    case 'y':
    case 'Y':
      d_printlnf (d,
		  "{cWelcome to {R%s{c!" NEWLINE
		  "{cGive me a password for {W%s{c: {x", mud_info.name,
		  ch->name);
      d_write (d, echo_off_str, 0);
      d->connected = CON_GET_NEW_PASSWORD;
      break;

    case 'n':
    case 'N':
      d_println (d, "{cOk, what IS it, then? {x");
      free_char (d->character);

      d->character = NULL;
      d->connected = CON_GET_NAME;
      break;

    default:
      d_println (d, "{cPlease type ({WY{c)es or ({WN{c)o: {x");
      break;
    }
}

Nanny_Fun (HANDLE_CON_GET_NEW_PASSWORD)
{
  CharData *ch = CH (d);
  const char *pwdnew;
  const char *p;

  if (strlen (argument) < 5)
    {
      d_println (d,
		 "{cPassword must be at least five characters long."
		 NEWLINE "{cPassword: {x");
      return;
    }

  pwdnew = crypt (argument, ch->name);
  for (p = pwdnew; *p != '\0'; p++)
    {
      if (*p == '~')
	{
	  d_println (d,
		     "{cNew password not acceptable, try again." NEWLINE
		     "{cPassword: {x");
	  return;
	}
    }

  replace_str (&ch->pcdata->pwd, pwdnew);
  d_println (d, "{cPlease retype password: {x");
  d->connected = CON_CONFIRM_NEW_PASSWORD;
}

void
send_race_info (Descriptor * d)
{
  RaceData *race;

  d_println (d, NEWLINE "{cThe following races are available:{x");
  d_printlnf (d, "{W%s{x", draw_line (CH (d), "{w-{W-", 0));
  for (race = race_first; race; race = race->next)
    {
      if (!race->pc_race)
	continue;
      d_printlnf (d, "{c\t%s{x", race->name);
    }
  d_printlnf (d, "{W%s", draw_line (CH (d), "{w-{W-", 0));
  d_printlnf (d, "{cFor more information, use help. (i.e. 'help %s'){x",
	      race_first->name);
}

Nanny_Fun (HANDLE_CON_CONFIRM_NEW_PASSWORD)
{
  CharData *ch = CH (d);

  if (str_casecmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      d_println (d,
		 "{cPasswords don't match.{x" NEWLINE
		 "{cRetype password: {x");
      d->connected = CON_GET_NEW_PASSWORD;
      return;
    }

  d_write (d, echo_on_str, 0);
  send_race_info (d);
  d_println (d, "{cWhat is your race? {x");
  d->connected = CON_GET_NEW_RACE;
}

Nanny_Fun (HANDLE_CON_GET_NEW_RACE)
{
  CharData *ch = CH (d);
  char arg[MAX_INPUT_LENGTH];
  RaceData *race;
  int i;

  one_argument (argument, arg);

  if (!str_cmp (arg, "help"))
    {
      argument = one_argument (argument, arg);
      if (NullStr (argument))
	do_function (ch, &nanny_help, "race help");
      else
	do_function (ch, &nanny_help, argument);
      d_println (d,
		 NEWLINE
		 "{cWhat is your race (help for more information)? {x");
      return;
    }

  race = race_lookup (argument);

  if (race == NULL || !race->pc_race)
    {
      d_println (d, "{cThat is not a valid race.{x");
      send_race_info (d);
      return;
    }

  if (IsRemort (ch) && ch->race != race)
    {
      d_printlnf (d, "{cYou are now a {R%s{c forever.{x", ch->race->name);
      ch->pcdata->stay_race = true;
    }

  ch->race = race;

  for (i = 0; i < STAT_MAX; i++)
    ch->perm_stat[i] = race->stats[i];
  ch->affected_by = ch->affected_by | race->aff;
  ch->imm_flags = ch->imm_flags | race->imm;
  ch->res_flags = ch->res_flags | race->res;
  ch->vuln_flags = ch->vuln_flags | race->vuln;
  ch->form = race->form;
  ch->parts = race->parts;


  for (i = 0; i < MAX_RACE_SKILL; i++)
    {
      if (race->skills[i] == NULL)
	break;
      group_add (ch, race->skills[i], false);
    }

  ch->pcdata->points = race->points;
  ch->size = race->size;

  d_printlnf (d, "{cYou are now a {W%s{c.", race->name);

  d_println (d,
	     NEWLINE
	     "{cWhat is your sex ({WM{c)ale/({WF{c)emale/({WN{c)eutral? {x");
  d->connected = CON_GET_NEW_SEX;
}

void
send_class_info (Descriptor * d)
{
  int iClass;

  d_println (d, NEWLINE "{cThe following classes are available:{x");
  d_printlnf (d, "{W%s{x", draw_line (CH (d), "{w-{W-", 0));
  for (iClass = 0; iClass < top_class; iClass++)
    {
      if (is_class (d->character, iClass))
	continue;
      d_printlnf (d, "{c\t%s{x", ClassName (d->character, iClass));
    }
  d_printlnf (d, "{W%s{x", draw_line (CH (d), "{w-{W-", 0));
  d_printlnf (d, "{cFor more information, use help. (i.e. 'help %s'){x",
	      class_table[0].name[0]);
}

Nanny_Fun (HANDLE_CON_GET_NEW_SEX)
{
  CharData *ch = CH (d);

  switch (toupper (argument[0]))
    {
    case 'M':
      d_println (d, "{cYou are now {Wmale{c.");
      ch->sex = SEX_MALE;
      ch->pcdata->true_sex = SEX_MALE;
      break;
    case 'F':
      d_println (d, "{cYou are now {Wfemale{c.");
      ch->sex = SEX_FEMALE;
      ch->pcdata->true_sex = SEX_FEMALE;
      break;
    case 'N':
      d_println (d, "{cYou are now {Wneutral{c.");
      ch->sex = SEX_NEUTRAL;
      ch->pcdata->true_sex = SEX_NEUTRAL;
      break;
    default:
      d_println (d, "{cThat's not a sex.{x" NEWLINE "{cWhat IS your sex? {x");
      return;
    }

  send_class_info (d);
  d_printlnf (d, "{cWhat is your %sclass? {x", IsRemort (ch) ? "next " : "");
  d->connected = CON_GET_NEW_CLASS;
}


int
roll_a_stat (CharData * ch, int pStat)
{
  int percent, bonus, low, high;

  if (!ch || !ch->race->pc_race)
    return number_range (13, 22);

  percent = number_percent ();
  if (percent > 99)
    bonus = 2;
  else if (percent > 95)
    bonus = 1;
  else if (percent < 5)
    bonus = -1;
  else if (percent < 1)
    bonus = -2;
  else
    bonus = 0;

  high = ch->race->max_stats[pStat] - (3 - bonus);
  low = ch->race->stats[pStat] - (3 - bonus);
  return number_range (low, high);
}

void
roll_stats (CharData * ch)
{
  int i;

  for (i = 0; i < STAT_MAX; i++)
    ch->perm_stat[i] = roll_a_stat (ch, i);
}

void
init_gen_data (Descriptor * d)
{
  int i;

  CharData *ch = CH (d);

  ch->gen_data = new_gen_data ();
  ch->gen_data->points_chosen = ch->pcdata->points;
  d_println (d, NEWLINE "{cYou may now roll your stats.{x" NEWLINE);
  roll_stats (ch);
  for (i = 0; i < STAT_MAX; i++)
    d_printf (d, "{c%.3s {W%02d {x", capitalize (stat_types[i].name),
	      ch->perm_stat[i]);
  d_println (d, "{c  Keep? ({WY{c/{WN{c) {x");
}

Nanny_Fun (HANDLE_CON_GET_NEW_CLASS)
{
  CharData *ch = CH (d);
  char arg[MIL];
  int iClass;

  one_argument (argument, arg);
  iClass = class_lookup (arg);

  if (iClass == -1)
    {
      if (!str_cmp (arg, "help"))
	{
	  argument = one_argument (argument, arg);
	  if (NullStr (argument))
	    do_function (ch, &nanny_help, "class help");
	  else
	    do_function (ch, &nanny_help, argument);
	  d_printlnf (d, NEWLINE "{cWhat IS your %sclass? {x",
		      IsRemort (ch) ? "next " : "");
	  return;
	}
      d_println (d, "{cInvalid class.{x");
      send_class_info (d);
      d_printlnf (d, "{cWhat IS your %sclass? {x",
		  IsRemort (ch) ? "next " : "");
      return;
    }

  if (is_class (ch, iClass))
    {
      d_printlnf (d, "{cYou are already part {W%s{c. Try another!{x",
		  ClassName (ch, iClass));
      return;
    }


  ch->Class[ch->Class[CLASS_COUNT]] = iClass;
  ch->Class[CLASS_COUNT] += 1;
  ch->Class[ch->Class[CLASS_COUNT]] = -1;


  d_printlnf (d, "{cYou are now a {W%s{c.", class_long (ch));
  if (!IsRemort (ch))
    {
      wiznet ("Newbie alert! $N sighted.", ch, NULL, WIZ_NEWBIE, false, 0);
      new_wiznet (ch, d->host, WIZ_SITES, true, get_trust (ch),
		  "$N@$t new player.");
      mud_info.stats.newbies++;
      d_println (d,
		 NEWLINE
		 "{cYou may now choose a general alignment of your character.{x");
      d_println (d,
		 "{cWhich alignment ({WG{c)ood/({WN{c)eutral/({WE{c)vil? {x");
      d->connected = CON_GET_ALIGNMENT;
    }
  else
    {

      init_gen_data (d);
      d->connected = CON_ROLL_STATS;
    }
}

void
send_deity_info (Descriptor * d)
{
  DeityData *i;
  int e;

  d_println (d, NEWLINE "{cDeities available for worship:{x");
  d_printlnf (d, "{W%s", draw_line (CH (d), "{w-{W-", 0));
  for (e = ETHOS_LAWFUL_GOOD; e != ETHOS_CHAOTIC_EVIL; e--)
    {
      for (i = deity_first; i; i = i->next)
	{
	  if (i->ethos != (ethos_t) e)
	    continue;

	  d_printlnf (d, "{c\t%-12s (%s): %s{x", i->name,
		      flag_string (ethos_types, i->ethos), i->desc);
	}
    }
  d_printlnf (d, "{W%s", draw_line (CH (d), "{w-{W-", 0));
  d_printlnf (d, "{cFor more information, use help. (i.e. 'help %s'){x",
	      deity_first->name);
}

Nanny_Fun (HANDLE_CON_GET_ALIGNMENT)
{
  CharData *ch = CH (d);

  switch (toupper (argument[0]))
    {
    case 'G':
      d_println (d, "{cYou are now {Wgood{c.{x");
      ch->alignment = 750;
      break;
    case 'N':
      d_println (d, "{cYou are now {Yneutral{c.{x");
      ch->alignment = 0;
      break;
    case 'E':
      d_println (d, "{cYou are now {Revil{c.{x");
      ch->alignment = -750;
      break;
    default:
      d_println (d, "{cThat's not a valid alignment.{x");
      d_println (d,
		 "{cWhich alignment ({WG{c)ood/({WN{c)eutral/({WE{c)vil? {x");
      return;
    }

  send_deity_info (d);
  d_println (d, "{cWhat deity would you like to worship?{x");
  d->connected = CON_GET_DEITY;
  return;
}

void
send_timezone_info (Descriptor * d)
{
  int i;

  d_println (d,
	     NEWLINE
	     "{cPlease choose a timezone closest to where you live:{x");
  d_printlnf (d, "{W%s", draw_line (CH (d), "{w-{W-", 0));
  for (i = 0; i < MAX_TZONE; i++)
    {
      d_printlnf (d, "{c\t{W%5s{c - %9s - %s{x", tzone_table[i].name,
		  str_time (current_time, i, "%I:%M:%S %p"),
		  tzone_table[i].zone);
    }
  d_printlnf (d, "{W%s", draw_line (CH (d), "{w-{W-", 0));
  d_printlnf (d, "{cFor more information, use help. (i.e. 'help %s'){x",
	      tzone_table[0].name);
}

Nanny_Fun (HANDLE_CON_GET_DEITY)
{
  char arg[MIL];
  DeityData *i;
  CharData *ch = CH (d);

  one_argument (argument, arg);

  if (!str_cmp (arg, "help"))
    {
      argument = one_argument (argument, arg);
      if (NullStr (argument))
	{
	  send_deity_info (d);
	  d_println (d, "{cWhat deity would you like to worship? {x");
	}
      else
	do_function (ch, &do_help, argument);
      d_println (d, "{cWhat deity would you like to worship? {x");
      return;
    }
  i = deity_lookup (argument);

  if (i == NULL)
    {
      send_deity_info (d);
      d_println (d, "{cWhat deity would you like to worship? {x");
      return;
    }
  ch->deity = i;
  d_printlnf (d, "{cYou now worship {W%s{c.{x", ch->deity->name);

  send_timezone_info (d);
  d_println (d, "{cWhat time zone do you live in? {x");
  d->connected = CON_GET_TIMEZONE;
  return;
}

Nanny_Fun (HANDLE_CON_GET_TIMEZONE)
{
  int i;
  char arg[MIL];
  CharData *ch = CH (d);

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      send_timezone_info (d);
      d_println (d, "{cWhat time zone do you live in? {x");
      return;
    }
  if (!str_cmp (arg, "help"))
    {
      d_printlnf (d,
		  "{cTime zones around the world are split up into %d areas,",
		  MAX_TZONE);
      d_printlnf (d,
		  "{ceach relating to Greenwich Mean Time (GMT) which is %s.{x",
		  str_time (-1, tzone_lookup ("GMT"), "%I:%M:%S %p"));
      d_println (d, "{cWhat time zone do you live in? {x");
      return;
    }
  i = tzone_lookup (argument);

  if (i == -1)
    {
      d_println (d,
		 "{cThat is not a valid time zone, please enter the full time zone name. (ex. GMT-5)");
      d_println (d, "{cWhat time zone do you live in? {x");
      return;
    }

  ch->pcdata->timezone = i;
  d_printlnf (d, "{cYour time zone is now %s %s (%s) {x",
	      tzone_table[i].zone, tzone_table[i].name,
	      str_time (current_time, i, "%I:%M:%S %p"));
  d_println (d, NULL);

  if (IsSet (d->desc_flags, DESC_TELOPT_NAWS))
    {
      init_gen_data (d);
      d->connected = CON_ROLL_STATS;
    }
  else
    {
      d_println (d,
		 "{cThis mud makes use of formating text to your screen size, your client does not");
      d_println (d,
		 "support auto-negotation so you may now enter a screen size manually.  The default");
      d_printlnf (d, "screen is %d columns by %d lines.{x" NEWLINE,
		  DEFAULT_SCR_WIDTH, DEFAULT_SCR_HEIGHT);
      d_printlnf (d,
		  "{cHow many columns (width) does your clients screen have roughly? (%d){x",
		  DEFAULT_SCR_WIDTH);
      d->connected = CON_GET_SCR_WIDTH;
    }
}

Nanny_Fun (HANDLE_CON_GET_SCR_WIDTH)
{
  int sz;
  char arg[MIL];
  CharData *ch = CH (d);

  argument = one_argument (argument, arg);

  if (!NullStr (arg))
    {
      sz = atoi (arg);

      if (sz < 10 || sz > 250)
	{
	  d_println (d,
		     "{cPlease keep your screen width between 10 and 250 columns.{x");
	  return;
	}
      d->scr_width = sz;
      ch->columns = sz;
    }
  d_println (d, draw_line (ch, "{`-{W=", 0));
  d_println (d, NULL);
  d_println (d,
	     "{cBased on the above line, is your screen width set to your satisfaction? ({WY{c/{WN{c) ");
  d->connected = CON_CONFIRM_SCR_WIDTH;
}

Nanny_Fun (HANDLE_CON_CONFIRM_SCR_WIDTH)
{
  switch (toupper (argument[0]))
    {
    case 'Y':
      d_printlnf (d, "{W%d {cscreen columns set.{x", d->scr_width);
      d_printlnf (d,
		  "{cHow many lines (height) does your clients screen have? (%d){x",
		  DEFAULT_SCR_HEIGHT);
      d->connected = CON_GET_SCR_LINES;
      break;
    case 'N':
      d_printlnf (d,
		  "{cHow many columns (width) does your clients screen have roughly? (%d){x",
		  DEFAULT_SCR_WIDTH);
      d->connected = CON_GET_SCR_WIDTH;
      break;
    default:
      d_println (d, "{cYes or No?{x");
      break;
    }
}

Nanny_Fun (HANDLE_CON_GET_SCR_LINES)
{
  int sz;
  char arg[MIL];
  CharData *ch = CH (d);

  argument = one_argument (argument, arg);

  if (!NullStr (arg))
    {
      sz = atoi (arg);

      if (sz < 10 || sz > 250)
	{
	  d_println (d,
		     "{cPlease keep your screen height between 10 and 250 lines.{x");
	  return;
	}

      d->scr_height = sz;
      ch->lines = sz;
    }
  d_printlnf (d, "{W%d {cscreen lines set.{x", d->scr_height);
  init_gen_data (d);
  d->connected = CON_ROLL_STATS;
}

Nanny_Fun (HANDLE_CON_ROLL_STATS)
{
  CharData *ch = CH (d);
  int i;

  switch (tolower (argument[0]))
    {
    case 'y':
      ch->gen_data->rerolls = 1;
      group_add (ch, "rom basics", false);
      add_base_groups (ch);
      ch->pcdata->learned[gsn_recall] = 50;
      d_println (d, "{cDo you wish to customize this character?{x");
      d_println (d,
		 "{cCustomization takes time, but allows a wider range of skills and abilities.{x");
      d_println (d, "{cCustomize ({WY{c/{WN{c)? {x");
      d->connected = CON_DEFAULT_CHOICE;
      break;
    case 'n':
      if (ch->gen_data->rerolls >= 10)
	{
	  d_println (d, NEWLINE
		     "You cannot reroll forever. Your stuck with what you have.{x");
	  d_println (d, NULL);
	  group_add (ch, "rom basics", false);
	  add_base_groups (ch);
	  ch->pcdata->learned[gsn_recall] = 50;
	  d_println (d, "{cDo you wish to customize this character?{x");
	  d_println (d,
		     "Customization takes time, but allows a wider range of skills and abilities.");
	  d_println (d, "{cCustomize ({WY{c/{WN{c)? {x");
	  d->connected = CON_DEFAULT_CHOICE;
	  break;
	}
      ch->gen_data->rerolls++;

      roll_stats (ch);
      d_printlnf (d, "{cYou have {W%d{c re-rolls remaining.{x",
		  (11 - ch->gen_data->rerolls));
      for (i = 0; i < STAT_MAX; i++)
	d_printf (d, "{c%.3s {W%02d {x", capitalize (stat_types[i].name),
		  ch->perm_stat[i]);
      d_println (d, "{c  Keep? ({WY{c/{WN{c) {x");
      break;
    default:
      d_println (d, "{cYes or No?{x");
      break;
    }
}

void
send_weapon_info (Descriptor * d)
{
  CharData *ch = CH (d);
  int i;

  free_gen_data (ch->gen_data);
  d_println (d, "{cPlease pick a weapon from the following choices:{x");
  for (i = 0; weapon_table[i].name != NULL; i++)
    if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
      d_printlnf (d, "{c\t%s{x", weapon_table[i].name);
  d_println (d, "{cYour choice? {x");
}

Nanny_Fun (HANDLE_CON_DEFAULT_CHOICE)
{
  CharData *ch = CH (d);

  d_println (d, NULL);
  switch (toupper (argument[0]))
    {
    case 'Y':
      d_println (d,
		 "{cThe following skills and groups are available to your character:{x");
      d_println (d, "{c(this list may be seen again by typing list){x");
      list_group_costs (ch);
      if (!IsRemort (ch))
	{
	  d_println (d, "{cYou already have the following skills:{x");
	  do_function (ch, &do_skills, "");
	}
      d_println (d,
		 NEWLINE
		 "{cChoice? (add, drop, list, learned, info, lookup, help, premise, done){x ");
      d->connected = CON_GEN_GROUPS;
      break;
    case 'N':
      d_printlnf (d, "{cDefault skill set for %s used.",
		  ClassName (ch, prime_class (ch)));
      add_default_groups (ch);
      send_weapon_info (d);
      d->connected = CON_PICK_WEAPON;
      break;
    default:
      d_println (d, "{cPlease answer ({WY{c/{WN{c)? {x");
      return;
    }
}

Nanny_Fun (HANDLE_CON_PICK_WEAPON)
{
  CharData *ch = CH (d);
  int i, weapon;
  void finish_remort (CharData *);

  weapon = weapon_lookup (argument);
  if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
    {
      d_println (d, "{cThat's not a valid selection. Choices are:{x");
      for (i = 0; weapon_table[i].name != NULL; i++)
	if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
	  d_printlnf (d, "{c\t%s{x", weapon_table[i].name);
      d_println (d, "{cYour choice? {x");
      return;
    }

  ch->pcdata->learned[*weapon_table[weapon].gsn] =
    Max (40, ch->pcdata->learned[*weapon_table[weapon].gsn]);

  if (IsRemort (ch))
    {
      finish_remort (ch);
    }
  else
    {
      do_function (ch, &nanny_help, "motd");
      d->connected = CON_READ_MOTD;
    }
}

Nanny_Fun (HANDLE_CON_GEN_GROUPS)
{
  CharData *ch = CH (d);

  if (!str_cmp (argument, "done"))
    {
      if (!IsRemort (ch))
	{
	  if (ch->pcdata->points == ch->race->points)
	    {
	      chprintln (ch, "{cYou didn't pick anything.{x");
	      return;
	    }
	}

      ch->pcdata->points = Max (ch->pcdata->points, mud_info.max_points);

      chprintlnf (ch, "{cCreation points: %d{x", ch->pcdata->points);
      chprintlnf (ch, "{cExperience per level: %d{x",
		  exp_per_level (ch, ch->gen_data->points_chosen));
      send_weapon_info (d);
      d->connected = CON_PICK_WEAPON;
      return;
    }

  if (!parse_gen_groups (ch, argument))
    d_println (d,
	       "{cChoices are: (add, drop, list, learned, info, lookup, help, premise, done){x ");
  else
    d_println (d,
	       "{cChoice? (add, drop, list, learned, info, lookup, help, premise, done){x ");
}

Nanny_Fun (HANDLE_CON_READ_IMOTD)
{
  CharData *ch = CH (d);

  do_function (ch, &nanny_help, "motd");
  d->connected = CON_READ_MOTD;
}

Nanny_Fun (HANDLE_CON_READ_MOTD)
{
  CharData *ch = CH (d);
  char buf[MSL];

  if (ch->pcdata == NULL || NullStr (ch->pcdata->pwd))
    {
      d_println (d, "{WWarning! Null password!{x");
      d_println (d, "{WPlease report old password with bug.{x");
      d_println (d, "{WType '{Wpassword null <new password>{c' to fix.{x");
    }

  d_printlnf (d, "{cWelcome to {?%s{c, %s.{x", mud_info.name, ch->name);
  Link (ch, char, next, prev);

  Link (ch, player, next_player, prev_player);
  d->connected = CON_PLAYING;
  reset_char (ch);

  if (ch->level == 0)
    {
      if (ch->Class[0] == -1)
	ch->Class[0] = 0;

      ch->perm_stat[class_table[prime_class (ch)].attr_prime] += 3;

      if (pfiles.count == 0)
	{
	  int sn;

	  ch->level = MAX_LEVEL;
	  ch->pcdata->perm_hit = ch->max_hit = 50000;
	  ch->pcdata->perm_mana = ch->max_mana = 50000;
	  ch->pcdata->perm_move = ch->max_move = 50000;
	  ch->pcdata->security = 9;
	  ch->trust = MAX_LEVEL;
	  for (sn = 0; sn < top_skill; sn++)
	    {
	      if (get_skill (ch, sn) > 0)
		ch->pcdata->learned[sn] = 100;
	    }
#ifndef DISABLE_I3
	  ch->pcdata->i3chardata->i3perm = I3PERM_IMP;
#endif

	  SetBit (ch->act, PLR_HOLYLIGHT);
	  replace_str (&ch->prompt, IMMORTAL_PROMPT);
	  save_char_obj (ch);
	  realloc_mem (pfiles.names, const char *, pfiles.count + 1);

	  pfiles.names[pfiles.count++] = str_dup (ch->name);
	}
      else
	ch->level = 1;

      ch->exp = exp_per_level (ch, ch->pcdata->points);
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
      ch->train = 3;
      ch->practice = 5;
      ch->gold = 10;
      sprintf (buf, "the %s %s", ch->race->name,
	       ClassName (ch, prime_class (ch)));
      set_title (ch, buf);

      if (ch->level >= LEVEL_IMMORTAL)
	{
	  char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
	  chprintlnf (ch,
		      "{RSince you are the first player file on {?%s{R, "
		      "you have been made an {rImplementor{R automatically.",
		      mud_info.name);
	}
      else
	{
	  do_function (ch, &do_outfit, "");
	  obj_to_char (create_object (get_obj_index (OBJ_VNUM_MAP), 0), ch);

	  char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
	  chprintln (ch, NULL);
	  do_function (ch, &nanny_help, "newbie info");
	}
    }
  else if (ch->in_room != NULL)
    {
      char_to_room (ch, ch->in_room);
    }
  else if (IsImmortal (ch))
    {
      char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
    }
  else
    {
      char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));
    }

  announce (ch, INFO_LOGIN, "$n has entered the realms.");
  act ("$n has entered the game.", ch, NULL, NULL, TO_ROOM);

  mud_info.stats.logins++;

  d_println (d, NULL);

  do_function (ch, &do_look, "auto");

  wiznet ("$N has left real life behind.", ch, NULL, WIZ_LOGINS,
	  false, get_trust (ch));

  if (ch->pet != NULL)
    {
      char_to_room (ch->pet, ch->in_room);
      act ("$n has entered the game.", ch->pet, NULL, NULL, TO_ROOM);
    }

  do_function (ch, &do_backup, "check");
  do_function (ch, &do_ncheck, "");
  do_function (ch, &do_count, "");
  unfinished_quest (ch);
  update_explored (ch);
  checkcorpse (ch);
}

#define NANNY_CASE(state, fun) \
    case state : \
        fun (d, argument); \
        break

void
nanny (Descriptor * d, const char *argument)
{


  if (d->connected != CON_NOTE_TEXT)
    {
      while (isspace (*argument))
	argument++;
    }

  switch (d->connected)
    {

    default:
      bugf ("Nanny: bad d->connected %d.", d->connected);
      close_socket (d);
      return;

      NANNY_CASE (CON_GET_TERM, HANDLE_CON_GET_TERM);

      NANNY_CASE (CON_GET_NAME, HANDLE_CON_GET_NAME);

      NANNY_CASE (CON_GET_OLD_PASSWORD, HANDLE_CON_GET_OLD_PASSWORD);

      NANNY_CASE (CON_GET_WIZ, HANDLE_CON_GET_WIZ);

      NANNY_CASE (CON_BREAK_CONNECT, HANDLE_CON_BREAK_CONNECT);

      NANNY_CASE (CON_CONFIRM_NEW_NAME, HANDLE_CON_CONFIRM_NEW_NAME);

      NANNY_CASE (CON_GET_NEW_PASSWORD, HANDLE_CON_GET_NEW_PASSWORD);

      NANNY_CASE (CON_CONFIRM_NEW_PASSWORD, HANDLE_CON_CONFIRM_NEW_PASSWORD);

      NANNY_CASE (CON_GET_NEW_RACE, HANDLE_CON_GET_NEW_RACE);

      NANNY_CASE (CON_GET_NEW_SEX, HANDLE_CON_GET_NEW_SEX);

      NANNY_CASE (CON_GET_NEW_CLASS, HANDLE_CON_GET_NEW_CLASS);

      NANNY_CASE (CON_GET_ALIGNMENT, HANDLE_CON_GET_ALIGNMENT);

      NANNY_CASE (CON_GET_DEITY, HANDLE_CON_GET_DEITY);

      NANNY_CASE (CON_GET_TIMEZONE, HANDLE_CON_GET_TIMEZONE);

      NANNY_CASE (CON_GET_SCR_WIDTH, HANDLE_CON_GET_SCR_WIDTH);

      NANNY_CASE (CON_CONFIRM_SCR_WIDTH, HANDLE_CON_CONFIRM_SCR_WIDTH);

      NANNY_CASE (CON_GET_SCR_LINES, HANDLE_CON_GET_SCR_LINES);

      NANNY_CASE (CON_ROLL_STATS, HANDLE_CON_ROLL_STATS);

      NANNY_CASE (CON_DEFAULT_CHOICE, HANDLE_CON_DEFAULT_CHOICE);

      NANNY_CASE (CON_PICK_WEAPON, HANDLE_CON_PICK_WEAPON);

      NANNY_CASE (CON_GEN_GROUPS, HANDLE_CON_GEN_GROUPS);

      NANNY_CASE (CON_READ_IMOTD, HANDLE_CON_READ_IMOTD);

      NANNY_CASE (CON_NOTE_TO, HANDLE_CON_NOTE_TO);

      NANNY_CASE (CON_NOTE_SUBJECT, HANDLE_CON_NOTE_SUBJECT);

      NANNY_CASE (CON_NOTE_EXPIRE, HANDLE_CON_NOTE_EXPIRE);

      NANNY_CASE (CON_NOTE_TEXT, HANDLE_CON_NOTE_TEXT);

      NANNY_CASE (CON_NOTE_FINISH, HANDLE_CON_NOTE_FINISH);

      NANNY_CASE (CON_READ_MOTD, HANDLE_CON_READ_MOTD);
    }

  return;
}
