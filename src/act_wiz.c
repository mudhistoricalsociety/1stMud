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
#include "recycle.h"
#include "tables.h"
#include "telnet.h"
#include "magic.h"
#include "data_table.h"
#include "vnums.h"
#include "special.h"


Proto (RoomIndex * find_location, (CharData *, const char *));

Do_Fun (do_wiznet)
{
  int flag;
  char arg[MIL];
  Column c;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, stringf
		 (ch, 0, Center, "-",
		  "{W[ " CTAG (_WIZNET) "WIZNET STATUS {W]{x"));
      set_cols (&c, ch, 4, COLS_CHAR, ch);
      for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	  if (wiznet_table[flag].level <= get_trust (ch))
	    {
	      print_on_off (&c,
			    IsSet (ch->wiznet, wiznet_table[flag].flag),
			    wiznet_table[flag].name, NULL);
	    }
	}
      cols_nl (&c);
      chprintln (ch, draw_line (ch, NULL, 0));
      chprintlnf (ch, "Type '%s all <on/off>' to toggle everything.", n_fun);
      return;
    }

  if (!str_prefix (arg, "on"))
    {
      chprintln (ch, CTAG (_WIZNET) "Welcome to Wiznet!{x");
      SetBit (ch->wiznet, WIZ_ON);
      return;
    }

  if (!str_prefix (arg, "off"))
    {
      chprintln (ch, CTAG (_WIZNET) "Signing off of Wiznet.{x");
      RemBit (ch->wiznet, WIZ_ON);
      return;
    }

  if (!str_prefix (arg, "all"))
    {
      if (!str_cmp (argument, "on"))
	{
	  for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    {
	      if (get_trust (ch) >= wiznet_table[flag].level
		  && !IsSet (ch->wiznet, wiznet_table[flag].flag))
		SetBit (ch->wiznet, wiznet_table[flag].flag);
	    }
	  chprintln (ch, CTAG (_WIZNET)
		     "All available wiznet flags turned on.{x");
	  return;
	}
      if (!str_cmp (argument, "off"))
	{
	  for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    {
	      if (get_trust (ch) >= wiznet_table[flag].level
		  && IsSet (ch->wiznet, wiznet_table[flag].flag))
		RemBit (ch->wiznet, wiznet_table[flag].flag);
	    }
	  chprintln (ch, CTAG (_WIZNET)
		     "All available wiznet flags turned off.{x");
	  return;
	}
      cmd_syntax (ch, NULL, n_fun, "all on/off", NULL);
      return;
    }
  flag = wiznet_lookup (arg);

  if (flag == -1 || get_trust (ch) < wiznet_table[flag].level)
    {
      chprintln (ch, "No such option.");
      return;
    }

  set_on_off (ch, &ch->wiznet, wiznet_table[flag].flag,
	      FORMATF (CTAG (_WIZNET) "You will now see {W%s" CTAG (_WIZNET)
		       " on wiznet.{x", wiznet_table[flag].name),
	      FORMATF (CTAG (_WIZNET) "You will no longer see {W%s"
		       CTAG (_WIZNET) " on wiznet.{x",
		       wiznet_table[flag].name));
}

void
new_wiznet (CharData * ch, const void *arg1, flag_t flag,
	    bool logit, int min_level, const char *format, ...)
{
  Descriptor *d;
  CharData *vch;
  va_list args;
  char string[MPL];
  int i;
  const char *chan;

  if (NullStr (format))
    return;

  va_start (args, format);
  vsnprintf (string, sizeof (string), format, args);
  va_end (args);

  for (i = 0; wiznet_table[i].name != NULL; i++)
    if (wiznet_table[i].flag == flag)
      break;

  chan = wiznet_table[i].name == NULL ? "all" : wiznet_table[i].name;

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING && (vch = CH (d)) && IsImmortal (vch)
	  && IsSet (vch->wiznet, WIZ_ON) && (!flag
					     || IsSet (vch->wiznet, flag))
	  && get_trust (vch) >= min_level && vch != ch)
	{
	  if (IsSet (vch->wiznet, WIZ_PREFIX))
	    d_printf (d, CTAG (_WIZNET) "-%s-%s-> ",
		      str_time (-1, GetTzone (vch), "%I:%M:%S"), chan);
	  else
	    d_print (d, CTAG (_WIZNET));
	  act_new (string, vch, arg1, ch, TO_CHAR, POS_DEAD);
	}
    }

  if (logit)
    log_string (strip_color
		(perform_act_string (string, ch, arg1, ch, false)));
  return;
}

Do_Fun (do_guild)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  ClanData *clan;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <cln name>", NULL);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL || IsNPC (victim))
    {
      chprintln (ch, "They aren't playing.");
      return;
    }

  if (!str_prefix (arg2, "none"))
    {
      chprintln (ch, "They are now clanless.");
      chprintln (victim, "You are now a member of no clan!");
      victim->pcdata->clan = NULL;
      victim->rank = 0;
      update_members (victim, true);
      return;
    }

  if ((clan = clan_lookup (arg2)) == NULL)
    {
      chprintln (ch, "No such clan exists.");
      return;
    }

  if (IsSet (clan->flags, CLAN_INDEPENDENT))
    {
      chprintlnf (ch, "They are now a %s.", clan->name);
      chprintlnf (victim, "You are now a %s.", clan->name);
    }
  else
    {
      chprintlnf (ch, "They are now a member of clan %s.",
		  capitalize (clan->name));
      chprintlnf (victim, "You are now a member of clan %s.",
		  capitalize (clan->name));
    }

  if (is_clan (victim))
    {
      update_members (victim, true);
    }

  victim->pcdata->clan = clan;
  victim->rank = 0;
  update_members (victim, false);
}


Do_Fun (do_outfit)
{
  ObjData *obj;
  int i, sn;
  vnum_t vnum;

  if (ch->level > 5 || IsNPC (ch))
    {
      chprintln (ch, "Find it yourself!");
      return;
    }

  if ((obj = get_eq_char (ch, WEAR_LIGHT)) == NULL)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_LIGHT);
    }

  if ((obj = get_eq_char (ch, WEAR_BODY)) == NULL)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_BODY);
    }


  if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      sn = 0;
      vnum = OBJ_VNUM_SCHOOL_SWORD;

      for (i = 0; weapon_table[i].name != NULL; i++)
	{
	  if (ch->pcdata->learned[sn] <
	      ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	      sn = *weapon_table[i].gsn;
	      vnum = weapon_table[i].vnum;
	    }
	}

      obj = create_object (get_obj_index (vnum), 0);
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_WIELD);
    }

  if (((obj = get_eq_char (ch, WEAR_WIELD)) == NULL ||
       !IsWeaponStat (obj, WEAPON_TWO_HANDS)) &&
      (obj = get_eq_char (ch, WEAR_SHIELD)) == NULL)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_SHIELD);
    }

  act ("You have been equipped by $g.", ch, NULL, NULL, TO_CHAR);
}


Do_Fun (do_nochannels)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Nochannel whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (IsSet (victim->comm, COMM_NOCHANNELS))
    {
      RemBit (victim->comm, COMM_NOCHANNELS);
      chprintln (victim, "The gods have restored your channel priviliges.");
      chprintln (ch, "NOCHANNELS removed.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N restores channels to %s", GetName (victim));
    }
  else
    {
      SetBit (victim->comm, COMM_NOCHANNELS);
      chprintln (victim, "The gods have revoked your channel priviliges.");
      chprintln (ch, "NOCHANNELS set.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N revokes %s's channels.", GetName (victim));
    }

  return;
}

Do_Fun (do_smote)
{
  CharData *vch;
  const char *letter, *name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  unsigned int matches = 0;

  if (!IsNPC (ch) && IsSet (ch->comm, COMM_NOEMOTE))
    {
      chprintln (ch, "You can't show your emotions.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Emote what?");
      return;
    }

  if (strstr (argument, ch->name) == NULL)
    {
      chprintln (ch, "You must include your name in an smote.");
      return;
    }

  chprintln (ch, argument);

  for (vch = ch->in_room->person_first; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->desc == NULL || vch == ch)
	continue;

      if ((letter = strstr (argument, vch->name)) == NULL)
	{
	  chprintln (vch, argument);
	  continue;
	}

      strcpy (temp, argument);
      temp[strlen (argument) - strlen (letter)] = '\0';
      last[0] = '\0';
      name = vch->name;

      for (; *letter != '\0'; letter++)
	{
	  if (*letter == '\'' && matches == strlen (vch->name))
	    {
	      strcat (temp, "r");
	      continue;
	    }

	  if (*letter == 's' && matches == strlen (vch->name))
	    {
	      matches = 0;
	      continue;
	    }

	  if (matches == strlen (vch->name))
	    {
	      matches = 0;
	    }

	  if (*letter == *name)
	    {
	      matches++;
	      name++;
	      if (matches == strlen (vch->name))
		{
		  strcat (temp, "you");
		  last[0] = '\0';
		  name = vch->name;
		  continue;
		}
	      strncat (last, letter, 1);
	      continue;
	    }

	  matches = 0;
	  strcat (temp, last);
	  strncat (temp, letter, 1);
	  last[0] = '\0';
	  name = vch->name;
	}

      chprintln (vch, temp);
    }

  return;
}

Do_Fun (do_bamfin)
{

  if (!IsNPC (ch))
    {
      if (NullStr (argument))
	{
	  chprintlnf (ch, "Your poofin is %s", ch->pcdata->bamfin);
	  return;
	}

      if (strstr (argument, ch->name) == NULL)
	{
	  chprintln (ch, "You must include your name.");
	  return;
	}

      replace_str (&ch->pcdata->bamfin, argument);

      chprintlnf (ch, "Your poofin is now %s", ch->pcdata->bamfin);
    }
  return;
}

Do_Fun (do_bamfout)
{

  if (!IsNPC (ch))
    {
      if (NullStr (argument))
	{
	  chprintlnf (ch, "Your poofout is %s", ch->pcdata->bamfout);
	  return;
	}

      if (strstr (argument, ch->name) == NULL)
	{
	  chprintln (ch, "You must include your name.");
	  return;
	}

      replace_str (&ch->pcdata->bamfout, argument);

      chprintlnf (ch, "Your poofout is now %s", ch->pcdata->bamfout);
    }
  return;
}

Do_Fun (do_deny)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Deny whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  SetBit (victim->act, PLR_DENY);
  chprintln (victim, "You are denied access!");
  new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0, "$N denies access to %s",
	      GetName (victim));
  chprintln (ch, "OK.");
  save_char_obj (victim);
  stop_fighting (victim, true);
  do_function (victim, &do_quit, "");

  return;
}

Do_Fun (do_disconnect)
{
  char arg[MAX_INPUT_LENGTH];
  Descriptor *d;
  CharData *victim;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Disconnect whom?");
      return;
    }

  if (is_number (arg))
    {
      SOCKET desc;

      desc = atoi (arg);
      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  if (d->descriptor == desc)
	    {
	      close_socket (d);
	      chprintln (ch, "Ok.");
	      return;
	    }
	}
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim->desc == NULL)
    {
      act ("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
      return;
    }

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if (d == victim->desc)
	{
	  close_socket (d);
	  chprintln (ch, "Ok.");
	  return;
	}
    }

  bug ("Do_disconnect: desc not found.");
  chprintln (ch, "Descriptor not found!");
  return;
}

Do_Fun (do_pardon)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<character> <killer|thief>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  if (!str_cmp (arg2, "killer"))
    {
      if (IsSet (victim->act, PLR_KILLER))
	{
	  RemBit (victim->act, PLR_KILLER);
	  chprintln (ch, "Killer flag removed.");
	  chprintln (victim, "You are no longer a KILLER.");
	}
      return;
    }

  if (!str_cmp (arg2, "thief"))
    {
      if (IsSet (victim->act, PLR_THIEF))
	{
	  RemBit (victim->act, PLR_THIEF);
	  chprintln (ch, "Thief flag removed.");
	  chprintln (victim, "You are no longer a THIEF.");
	}
      return;
    }

  cmd_syntax (ch, NULL, n_fun, "<character> <killer|thief>", NULL);
  return;
}

Do_Fun (do_echo)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      chprintln (ch, "Global echo what?");
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING)
	{
	  if (get_trust (d->character) >= get_trust (ch))
	    chprint (d->character, "global> ");
	  chprintln (d->character, argument);
	}
    }

  return;
}

Do_Fun (do_recho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      chprintln (ch, "Local echo what?");

      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room)
	{
	  if (get_trust (d->character) >= get_trust (ch))
	    chprint (d->character, "local> ");
	  chprintln (d->character, argument);
	}
    }

  return;
}

Do_Fun (do_zecho)
{
  Descriptor *d;

  if (NullStr (argument))
    {
      chprintln (ch, "Zone echo what?");
      return;
    }

  for (d = descriptor_first; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room != NULL
	  && ch->in_room != NULL
	  && d->character->in_room->area == ch->in_room->area)
	{
	  if (get_trust (d->character) >= get_trust (ch))
	    chprint (d->character, "zone> ");
	  chprintln (d->character, argument);
	}
    }
}

Do_Fun (do_pecho)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  argument = one_argument (argument, arg);

  if (NullStr (argument) || NullStr (arg))
    {
      chprintln (ch, "Personal echo what?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "Target not found.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch) && get_trust (ch) != MAX_LEVEL)
    chprint (victim, "personal> ");

  chprintln (victim, argument);
  chprint (ch, "personal> ");
  chprintln (ch, argument);
}

RoomIndex *
find_location (CharData * ch, const char *arg)
{
  CharData *victim;
  ObjData *obj;
  AreaData *area;

  if (is_number (arg))
    return get_room_index (atov (arg));

  if ((victim = get_char_world (ch, arg)) != NULL)
    return victim->in_room;

  if ((area = area_lookup (arg)) != NULL)
    return area_begin (area);

  if ((obj = get_obj_world (ch, arg)) != NULL)
    return obj->in_room;

  return NULL;
}

Do_Fun (do_transfer)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  RoomIndex *location;
  Descriptor *d;
  CharData *victim;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1))
    {
      chprintln (ch, "Transfer whom (and where)?");
      return;
    }

  if (!str_cmp (arg1, "all"))
    {
      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  if (d->connected == CON_PLAYING && d->character != ch &&
	      d->character->in_room != NULL && can_see (ch, d->character))
	    {
	      char buf[MAX_STRING_LENGTH];

	      sprintf (buf, "%s %s", d->character->name, arg2);
	      do_function (ch, &do_transfer, buf);
	    }
	}
      return;
    }


  if (NullStr (arg2))
    {
      location = ch->in_room;
    }
  else
    {
      if ((location = find_location (ch, arg2)) == NULL)
	{
	  chprintln (ch, "No such location.");
	  return;
	}

      if (!is_room_owner (ch, location) && room_is_private (location)
	  && get_trust (ch) < MAX_LEVEL)
	{
	  chprintln (ch, "That room is private right now.");
	  return;
	}
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim->in_room == NULL)
    {
      chprintln (ch, "They are in limbo.");
      return;
    }

  if (victim->fighting != NULL)
    stop_fighting (victim, true);
  act ("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, location);
  act ("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    act ("$n has transferred you.", ch, NULL, victim, TO_VICT);
  do_function (victim, &do_look, "auto");
  chprintln (ch, "Ok.");
}

Do_Fun (do_at)
{
  char arg[MAX_INPUT_LENGTH];
  RoomIndex *location;
  RoomIndex *original;
  ObjData *on;
  CharData *wch;

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      chprintln (ch, "At where what?");
      return;
    }

  if ((location = find_location (ch, arg)) == NULL)
    {
      chprintln (ch, "No such location.");
      return;
    }

  if (!is_room_owner (ch, location) && room_is_private (location) &&
      get_trust (ch) < MAX_LEVEL)
    {
      chprintln (ch, "That room is private right now.");
      return;
    }

  original = ch->in_room;
  on = ch->on;
  char_from_room (ch);
  char_to_room (ch, location);
  interpret (ch, argument);


  for (wch = char_first; wch != NULL; wch = wch->next)
    {
      if (wch == ch)
	{
	  char_from_room (ch);
	  char_to_room (ch, original);
	  ch->on = on;
	  break;
	}
    }

  return;
}

Do_Fun (do_goto)
{
  RoomIndex *location;
  CharData *rch;
  int count = 0;

  if (NullStr (argument))
    {
      chprintln (ch, "Goto where?");
      return;
    }

  if ((location = find_location (ch, argument)) == NULL)
    {
      chprintln (ch, "No such location.");
      return;
    }

  count = 0;
  for (rch = location->person_first; rch != NULL; rch = rch->next_in_room)
    count++;

  if (!is_room_owner (ch, location) && room_is_private (location) &&
      (count > 1 || get_trust (ch) < MAX_LEVEL))
    {
      chprintln (ch, "That room is private right now.");
      return;
    }

  if (ch->fighting != NULL)
    stop_fighting (ch, true);

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust (rch) >= ch->invis_level)
	{
	  if (ch->pcdata != NULL && !NullStr (ch->pcdata->bamfout))
	    act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	  else
	    act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  char_from_room (ch);
  char_to_room (ch, location);

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust (rch) >= ch->invis_level)
	{
	  if (ch->pcdata != NULL && !NullStr (ch->pcdata->bamfin))
	    act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	  else
	    act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  do_function (ch, &do_look, "auto");
  return;
}

Do_Fun (do_violate)
{
  RoomIndex *location;
  CharData *rch;

  if (NullStr (argument))
    {
      chprintln (ch, "Goto where?");
      return;
    }

  if ((location = find_location (ch, argument)) == NULL)
    {
      chprintln (ch, "No such location.");
      return;
    }

  if (!room_is_private (location))
    {
      chprintln (ch, "That room isn't private, use goto.");
      return;
    }

  if (ch->fighting != NULL)
    stop_fighting (ch, true);

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust (rch) >= ch->invis_level)
	{
	  if (ch->pcdata != NULL && !NullStr (ch->pcdata->bamfout))
	    act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	  else
	    act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  char_from_room (ch);
  char_to_room (ch, location);

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (get_trust (rch) >= ch->invis_level)
	{
	  if (ch->pcdata != NULL && !NullStr (ch->pcdata->bamfin))
	    act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	  else
	    act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  do_function (ch, &do_look, "auto");
  return;
}



Do_Fun (do_vnum)
{
  char arg[MAX_INPUT_LENGTH];
  const char *string;

  string = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "obj <name>",
		  "mob <name>", "skill <skill or spell>", NULL);
      return;
    }

  if (!str_cmp (arg, "obj"))
    {
      do_function (ch, &do_ofind, string);
      return;
    }

  if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
      do_function (ch, &do_mfind, string);
      return;
    }

  if (!str_cmp (arg, "skill") || !str_cmp (arg, "spell"))
    {
      do_function (ch, &do_slookup, string);
      return;
    }

  do_function (ch, &do_mfind, argument);
  do_function (ch, &do_ofind, argument);
}

Do_Fun (do_mfind)
{
  char arg[MAX_INPUT_LENGTH];
  CharIndex *pMobIndex;
  int hash;
  bool fAll;
  bool found;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Find whom?");
      return;
    }

  fAll = false;
  found = false;


  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pMobIndex = char_index_hash[hash]; pMobIndex;
	   pMobIndex = pMobIndex->next)
	{
	  if (fAll || is_name (argument, pMobIndex->player_name))
	    {
	      found = true;
	      chprintlnf (ch, "[%5ld] %s", pMobIndex->vnum,
			  pMobIndex->short_descr);
	    }
	}
    }

  if (!found)
    chprintln (ch, "No mobiles by that name.");

  return;
}

Do_Fun (do_ofind)
{
  char arg[MAX_INPUT_LENGTH];
  ObjIndex *pObjIndex;
  int hash;
  bool fAll;
  bool found;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Find what?");
      return;
    }

  fAll = false;
  found = false;


  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    {
      for (pObjIndex = obj_index_hash[hash]; pObjIndex;
	   pObjIndex = pObjIndex->next)
	{
	  if (fAll || is_name (argument, pObjIndex->name))
	    {
	      found = true;
	      chprintlnf (ch, "[%5ld] %s", pObjIndex->vnum,
			  pObjIndex->short_descr);
	    }
	}
    }

  if (!found)
    chprintln (ch, "No objects by that name.");

  return;
}

Do_Fun (do_owhere)
{
  Buffer *buffer;
  ObjData *obj;
  ObjData *in_obj;
  bool found;
  int number = 0, max_found;

  found = false;
  number = 0;
  max_found = 200;

  buffer = new_buf ();

  if (NullStr (argument))
    {
      chprintln (ch, "Find what?");
      return;
    }

  for (obj = obj_first; obj != NULL; obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !is_name (argument, obj->name) ||
	  ch->level < obj->level)
	continue;

      found = true;
      number++;

      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
	;

      if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by)
	  && in_obj->carried_by->in_room != NULL)
	bprintlnf (buffer, "%3d) %s is carried by %s [Room %ld]",
		   number, obj->short_descr, Pers (in_obj->carried_by,
						   ch),
		   in_obj->carried_by->in_room->vnum);
      else if (in_obj->in_room != NULL && can_see_room (ch, in_obj->in_room))
	bprintlnf (buffer,
		   "%3d) %s is in %s [Room %ld]",
		   number,
		   obj->short_descr,
		   in_obj->in_room->name, in_obj->in_room->vnum);
      else
	bprintlnf (buffer, "%3d) %s is somewhere", number, obj->short_descr);

      if (number >= max_found)
	break;
    }

  if (!found)
    chprintln (ch, "Nothing like that in heaven or earth.");
  else
    sendpage (ch, buf_string (buffer));

  free_buf (buffer);
}

Do_Fun (do_mwhere)
{
  Buffer *buffer;
  CharData *victim;
  bool found;
  int count = 0;

  if (NullStr (argument))
    {
      Descriptor *d;



      buffer = new_buf ();
      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  if (d->character != NULL && d->connected == CON_PLAYING &&
	      d->character->in_room != NULL && can_see (ch, d->character)
	      && can_see_room (ch, d->character->in_room))
	    {
	      victim = d->character;
	      count++;
	      if (d->original != NULL)
		bprintlnf (buffer,
			   "%3d) %s (in the body of %s) is in %s [%ld]",
			   count, d->original->name,
			   victim->short_descr,
			   victim->in_room->name, victim->in_room->vnum);
	      else
		bprintlnf (buffer, "%3d) %s is in %s [%ld]", count,
			   victim->name, victim->in_room->name,
			   victim->in_room->vnum);
	    }
	}

      sendpage (ch, buf_string (buffer));
      free_buf (buffer);
      return;
    }

  found = false;
  buffer = new_buf ();
  for (victim = char_first; victim != NULL; victim = victim->next)
    {
      if (victim->in_room != NULL && is_name (argument, victim->name))
	{
	  found = true;
	  count++;
	  bprintlnf (buffer, "%3d) [%5ld] %-28s [%5ld] %s", count,
		     IsNPC (victim) ? victim->pIndexData->vnum : 0,
		     IsNPC (victim) ? victim->short_descr : victim->name,
		     victim->in_room->vnum, victim->in_room->name);
	}
    }

  if (!found)
    act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
  else
    sendpage (ch, buf_string (buffer));

  free_buf (buffer);

  return;
}

long
get_seconds (long value, const char *arg)
{
  if (is_number (arg))
    return value * atol (arg);
  else if (!str_prefix (arg, "seconds"))
    return value * SECOND;
  else if (!str_prefix (arg, "minutes"))
    return value * MINUTE;
  else if (!str_prefix (arg, "hours"))
    return value * HOUR;
  else if (!str_prefix (arg, "days"))
    return value * DAY;
  else
    return value;
}

const char *
crs_type_name (crs_t type)
{
  switch (type)
    {
    case CRS_COPYOVER:
      return "Copyover";
    case CRS_REBOOT:
      return "Reboot";
    case CRS_SHUTDOWN:
      return "Shutdown";
    default:
      return "Unknown";
    }
}

Proto (void copyover, (void));
Proto (void reboot_mud, (void));
Proto (void shutdown_mud, (void));

void
crs_fun (crs_t type)
{
  switch (type)
    {
    case CRS_COPYOVER:
      copyover ();
      break;
    case CRS_REBOOT:
      reboot_mud ();
      break;
    case CRS_SHUTDOWN:
      shutdown_mud ();
      break;
    default:
      bug ("crs_fun: bad type");
      break;
    }
}

void
crs_update (void)
{
  if (crs_info.status == CRS_NONE || crs_info.timer <= 0)
    return;

  --crs_info.timer;

  if (crs_info.timer % PULSE_PER_SECOND != 0)
    return;

  switch (crs_info.timer / PULSE_PER_SECOND)
    {
    case 0:
      crs_fun (crs_info.status);
      break;
    case 1:
    case 2:
    case 3:
    case 30:
    case MINUTE:
    case MINUTE * 3:
    case MINUTE * 5:
    case MINUTE * 10:
    case MINUTE * 15:
    case MINUTE * 20:
    case MINUTE * 30:
      announce (NULL, INFO_ALL, crs_sprintf (true, false));
      break;
    }
}

const char *
format_pulse (int time)
{
  static char buf[5][100];
  static int i;
  char temp[100];
  double timer = time / PULSE_PER_SECOND;

  ++i;
  i %= 5;

  sprintf (temp, "%%.%sf %s%%s",
	   (timer < MINUTE
	    || (int) timer % MINUTE == 0) ? "" : "2",
	   timer >= MINUTE ? "minute" : "second");
  sprintf (buf[i], temp, timer >= MINUTE ? timer / MINUTE : timer,
	   (timer == MINUTE || timer == 1) ? "" : "s");
  return buf[i];
}

const char *
crs_sprintf (bool pTime, bool reason)
{
  char buf[MSL];
  const char *prstr;

  if (crs_info.status == CRS_NONE || (pTime && crs_info.timer < 0))
    return "";

  if (reason)
    prstr = crs_info.reason;
  else
    prstr = "";

  if (pTime)
    {
      sprintf (buf, "%s by %s in %s.%s",
	       crs_type_name (crs_info.status),
	       !NullStr (crs_info.who) ? crs_info.who : "System",
	       format_pulse (crs_info.timer), prstr);
    }
  else
    {
      sprintf (buf, "%s by %s.%s%s",
	       crs_type_name (crs_info.status),
	       !NullStr (crs_info.who) ? crs_info.who : "System",
	       prstr,
	       crs_info.status == CRS_COPYOVER ? " Please wait....." : "");
    }
  return strip_color (buf);
}

void
do_crs (const char *n_fun, CharData * ch, const char *argument, crs_t type)
{
  char arg[MIL], arg2[MIL];
  int value = -1;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "now [reason]",
		  "<#value> <#mod> [reason]", "cancel [why]", "status", NULL);
      return;
    }

  if (!str_cmp (arg, "cancel"))
    {
      crs_info.timer = -1;
      replace_str (&crs_info.who, "");
      replace_str (&crs_info.reason, "");
      crs_info.status = CRS_NONE;
      announce (ch, INFO_ALL, "$n has cancelled the %s.", n_fun);
      announce (ch, INFO_ALL | INFO_PRIVATE,
		"You have cancelled the %s.", n_fun);
      return;
    }

  if (!str_cmp (arg, "status"))
    {
      if (crs_info.timer == -1 || crs_info.status != type)
	chprintlnf (ch, "Automatic %s is inactive.", n_fun);
      else
	chprintln (ch, crs_sprintf (true, true));
      return;
    }

  if (!str_cmp (arg, "now"))
    {
      crs_info.status = type;
      replace_str (&crs_info.who,
		   (ch->invis_level > 0 ? "an Immortal" : ch->name));
      if (!NullStr (argument))
	replace_strf (&crs_info.reason, " (%s)", argument);
      crs_fun (crs_info.status);
      return;
    }

  if (!str_cmp (arg, "random"))
    value = number_range (60, 360);
  else if (is_number (arg))
    value = atoi (arg);
  else
    {
      chprintln (ch, "Thats not a valid value.");
      return;
    }

  argument = one_argument (argument, arg2);

  if (NullStr (arg2) || !is_name (arg2, "seconds minutes hours days"))
    {
      cmd_syntax (ch, NULL, n_fun, "<value> [seconds/minutes/hours/days]",
		  NULL);
      return;
    }

  value = get_seconds (value, arg2);
  value *= PULSE_PER_SECOND;
  crs_info.timer = value;

  if (!NullStr (argument))
    replace_strf (&crs_info.reason, " (%s)", argument);

  replace_str (&crs_info.who,
	       (ch->invis_level > 0 ? "an Immortal" : ch->name));
  crs_info.status = type;
  announce (NULL, INFO_ALL, crs_sprintf (true, false));
}

Do_Fun (do_reboot)
{
  do_crs (n_fun, ch, argument, CRS_REBOOT);
  return;
}

void
reboot_mud (void)
{
  Descriptor *d, *d_next;
  CharData *vch;

  logf (crs_sprintf (false, true));

  for (d = descriptor_first; d != NULL; d = d_next)
    {
      d_next = d->next;
      d_write (d, NEWLINE, 2);
      d_write (d, crs_sprintf (false, true), 0);
      d_write (d, NEWLINE, 2);
      vch = CH (d);
      if (vch != NULL)
	{
	  save_char_obj (vch);
	}
      close_socket (d);
    }
  run_level = RUNLEVEL_SHUTDOWN;
  return;
}

Do_Fun (do_shutdown)
{
  do_crs (n_fun, ch, argument, CRS_SHUTDOWN);
  return;
}

void
shutdown_mud ()
{
  Descriptor *d, *d_next;
  CharData *vch;

  append_file (SHUTDOWN_FILE, crs_sprintf (false, true), true);
  for (d = descriptor_first; d != NULL; d = d_next)
    {
      d_next = d->next;
      d_write (d, NEWLINE, 2);
      d_write (d, crs_sprintf (false, true), 0);
      d_write (d, NEWLINE, 2);
      vch = CH (d);
      if (vch != NULL)
	{
	  save_char_obj (vch);
	}
      close_socket (d);
    }
  run_level = RUNLEVEL_SHUTDOWN;
  return;
}

Do_Fun (do_protect)
{
  CharData *victim;

  if (NullStr (argument))
    {
      chprintln (ch, "Protect whom from snooping?");
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "You can't find them.");
      return;
    }

  if (IsSet (victim->comm, COMM_SNOOP_PROOF))
    {
      act_new ("$N is no longer snoop-proof.", ch, NULL, victim,
	       TO_CHAR, POS_DEAD);
      chprintln (victim, "Your snoop-proofing was just removed.");
      RemBit (victim->comm, COMM_SNOOP_PROOF);
    }
  else
    {
      act_new ("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR, POS_DEAD);
      chprintln (victim, "You are now immune to snooping.");
      SetBit (victim->comm, COMM_SNOOP_PROOF);
    }
}

Do_Fun (do_snoop)
{
  char arg[MAX_INPUT_LENGTH];
  Descriptor *d;
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Snoop whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim->desc == NULL)
    {
      chprintln (ch, "No descriptor to snoop.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "Cancelling all snoops.");
      wiznet ("$N stops being such a snoop.", ch, NULL, WIZ_SNOOPS,
	      false, get_trust (ch));
      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  if (d->snoop_by == ch->desc)
	    d->snoop_by = NULL;
	}
      return;
    }

  if (victim->desc->snoop_by != NULL)
    {
      chprintln (ch, "Busy already.");
      return;
    }

  if (!is_room_owner (ch, victim->in_room) &&
      ch->in_room != victim->in_room && room_is_private (victim->in_room)
      && !IsTrusted (ch, IMPLEMENTOR))
    {
      chprintln (ch, "That character is in a private room.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch) ||
      IsSet (victim->comm, COMM_SNOOP_PROOF))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (ch->desc != NULL)
    {
      for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
	{
	  if (d->character == victim || d->original == victim)
	    {
	      chprintln (ch, "No snoop loops.");
	      return;
	    }
	}
    }

  victim->desc->snoop_by = ch->desc;
  new_wiznet (ch, NULL, WIZ_SNOOPS, false, get_trust (ch),
	      "$N starts snooping on %s", GetName (victim));
  chprintln (ch, "Ok.");
  return;
}

Do_Fun (do_switch)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Switch into whom?");
      return;
    }

  if (ch->desc == NULL)
    return;

  if (ch->desc->original != NULL)
    {
      chprintln (ch, "You are already switched.");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "Ok.");
      return;
    }

  if (!IsNPC (victim))
    {
      chprintln (ch, "You can only switch into mobiles.");
      return;
    }

  if (!is_room_owner (ch, victim->in_room) &&
      ch->in_room != victim->in_room && room_is_private (victim->in_room)
      && !IsTrusted (ch, IMPLEMENTOR))
    {
      chprintln (ch, "That character is in a private room.");
      return;
    }

  if (victim->desc != NULL)
    {
      chprintln (ch, "Character in use.");
      return;
    }

  new_wiznet (ch, NULL, WIZ_SWITCHES, false, get_trust (ch),
	      "$N switches into %s", GetName (victim));

  ch->desc->character = victim;
  ch->desc->original = ch;
  victim->desc = ch->desc;
  ch->desc = NULL;

  if (ch->prompt != NULL)
    replace_str (&victim->prompt, ch->prompt);
  victim->comm = ch->comm;
  victim->lines = ch->lines;
  victim->columns = ch->columns;
  chprintln (victim, "Ok.");
  return;
}

Do_Fun (do_return)
{
  if (ch->desc == NULL)
    return;

  if (ch->desc->original == NULL)
    {
      chprintln (ch, "You aren't switched.");
      return;
    }

  chprintln (ch, "You return to your original body.");
  if (!IsNPC (ch) && ch->pcdata->tells)
    {
      chprintlnf (ch,
		  "You have %d tells waiting.  Type '%s' to view them.",
		  ch->pcdata->tells, cmd_name (do_replay));
    }

  if (ch->prompt != NULL)
    {
      free_string (ch->prompt);
      ch->prompt = NULL;
    }

  new_wiznet (ch->desc->original, 0, WIZ_SWITCHES, false,
	      get_trust (ch), "$N returns from %s.", GetName (ch));
  ch->desc->character = ch->desc->original;
  ch->desc->original = NULL;
  ch->desc->character->desc = ch->desc;
  ch->desc = NULL;
  return;
}


bool
obj_check (CharData * ch, ObjData * obj)
{
  if (IsTrusted (ch, GOD) ||
      (IsTrusted (ch, IMMORTAL) && obj->level <= 20 &&
       obj->cost <= 1000) || (IsTrusted (ch, DEMI) &&
			      obj->level <= 10 && obj->cost <= 500) ||
      (IsTrusted (ch, ANGEL) && obj->level <= 5 && obj->cost <= 250)
      || (IsTrusted (ch, AVATAR) && obj->level == 0 && obj->cost <= 100))
    return true;
  else
    return false;
}


void
recursive_clone (CharData * ch, ObjData * obj, ObjData * clone)
{
  ObjData *c_obj, *t_obj;

  for (c_obj = obj->content_first; c_obj != NULL; c_obj = c_obj->next_content)
    {
      if (obj_check (ch, c_obj))
	{
	  t_obj = create_object (c_obj->pIndexData, 0);
	  clone_object (c_obj, t_obj);
	  obj_to_obj (t_obj, clone);
	  recursive_clone (ch, c_obj, t_obj);
	}
    }
}


Do_Fun (do_clone)
{
  char arg[MAX_INPUT_LENGTH];
  const char *rest;
  CharData *mob;
  ObjData *obj;

  rest = one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Clone what?");
      return;
    }

  if (!str_prefix (arg, "object"))
    {
      mob = NULL;
      obj = get_obj_here (ch, NULL, rest);
      if (obj == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
    }
  else if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
    {
      obj = NULL;
      mob = get_char_room (ch, NULL, rest);
      if (mob == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
    }
  else
    {

      mob = get_char_room (ch, NULL, argument);
      obj = get_obj_here (ch, NULL, argument);
      if (mob == NULL && obj == NULL)
	{
	  chprintln (ch, "You don't see that here.");
	  return;
	}
    }


  if (obj != NULL)
    {
      ObjData *clone;

      if (!obj_check (ch, obj))
	{
	  chprintln (ch, "Your powers are not great enough for such a task.");
	  return;
	}

      clone = create_object (obj->pIndexData, 0);
      clone_object (obj, clone);
      if (obj->carried_by != NULL)
	obj_to_char (clone, ch);
      else
	obj_to_room (clone, ch->in_room);
      recursive_clone (ch, obj, clone);

      act ("$n has created $p.", ch, clone, NULL, TO_ROOM);
      act ("You clone $p.", ch, clone, NULL, TO_CHAR);
      wiznet ("$N clones $p.", ch, clone, WIZ_LOAD, false, get_trust (ch));
      return;
    }
  else if (mob != NULL)
    {
      CharData *clone;
      ObjData *new_pobj;

      if (!IsNPC (mob))
	{
	  chprintln (ch, "You can only clone mobiles.");
	  return;
	}

      if ((mob->level > 20 && !IsTrusted (ch, GOD)) ||
	  (mob->level > 10 && !IsTrusted (ch, IMMORTAL)) ||
	  (mob->level > 5 && !IsTrusted (ch, DEMI)) ||
	  (mob->level > 0 && !IsTrusted (ch, ANGEL)) ||
	  !IsTrusted (ch, AVATAR))
	{
	  chprintln (ch, "Your powers are not great enough for such a task.");
	  return;
	}

      clone = create_mobile (mob->pIndexData);
      clone_mobile (mob, clone);

      for (obj = mob->carrying_first; obj != NULL; obj = obj->next_content)
	{
	  if (obj_check (ch, obj))
	    {
	      new_pobj = create_object (obj->pIndexData, 0);
	      clone_object (obj, new_pobj);
	      recursive_clone (ch, obj, new_pobj);
	      obj_to_char (new_pobj, clone);
	      new_pobj->wear_loc = obj->wear_loc;
	    }
	}
      char_to_room (clone, ch->in_room);
      act ("$n has created $N.", ch, NULL, clone, TO_ROOM);
      act ("You clone $N.", ch, NULL, clone, TO_CHAR);
      new_wiznet (ch, NULL, WIZ_LOAD, false, get_trust (ch),
		  "$N clones %s.", GetName (clone));
      return;
    }
}



Do_Fun (do_load)
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "mob <vnum>", "obj <vnum>", NULL);
      return;
    }

  if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
      do_function (ch, &do_mload, argument);
      return;
    }

  if (!str_cmp (arg, "obj"))
    {
      do_function (ch, &do_oload, argument);
      return;
    }

  do_function (ch, &do_load, "");
}

Do_Fun (do_mload)
{
  char arg[MAX_INPUT_LENGTH];
  CharIndex *pMobIndex;
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg) || !is_number (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum>", NULL);
      return;
    }

  if ((pMobIndex = get_char_index (atov (arg))) == NULL)
    {
      chprintln (ch, "No mob has that vnum.");
      return;
    }

  victim = create_mobile (pMobIndex);
  char_to_room (victim, ch->in_room);
  act ("$n has created $N!", ch, NULL, victim, TO_ROOM);
  new_wiznet (ch, NULL, WIZ_LOAD, false, get_trust (ch), "$N loads %s.",
	      GetName (victim));
  chprintln (ch, "Ok.");
  return;
}

Do_Fun (do_oload)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  ObjIndex *pObjIndex;
  ObjData *obj;
  int level;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1) || !is_number (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "<vnum> <level>", NULL);
      return;
    }

  level = get_trust (ch);

  if (!NullStr (arg2))
    {
      if (!is_number (arg2))
	{
	  cmd_syntax (ch, NULL, n_fun, "<vnum> <level>", NULL);
	  return;
	}
      level = atoi (arg2);
      if (level < 0 || level > get_trust (ch))
	{
	  chprintln (ch, "Level must be be between 0 and your level.");
	  return;
	}
    }

  if ((pObjIndex = get_obj_index (atov (arg1))) == NULL)
    {
      chprintln (ch, "No object has that vnum.");
      return;
    }

  obj = create_object (pObjIndex, level);
  if (CanWear (obj, ITEM_TAKE))
    obj_to_char (obj, ch);
  else
    obj_to_room (obj, ch->in_room);
  act ("$n has created $p!", ch, obj, NULL, TO_ROOM);
  wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, false, get_trust (ch));
  chprintln (ch, "Ok.");
  return;
}

Do_Fun (do_purge)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *obj;
  Descriptor *d;

  one_argument (argument, arg);

  if (NullStr (arg))
    {

      CharData *vnext;
      ObjData *obj_next;

      for (victim = ch->in_room->person_first; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IsNPC (victim) && !IsSet (victim->act, ACT_NOPURGE)
	      && victim != ch)
	    extract_char (victim, true);
	}

      for (obj = ch->in_room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (!IsObjStat (obj, ITEM_NOPURGE))
	    extract_obj (obj);
	}

      act ("$n purges the room!", ch, NULL, NULL, TO_ROOM);
      chprintln (ch, "Ok.");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (!IsNPC (victim))
    {

      if (ch == victim)
	{
	  chprintln (ch, "Ho ho ho.");
	  return;
	}

      if (get_trust (ch) <= get_trust (victim))
	{
	  chprintln (ch, "Maybe that wasn't a good idea...");
	  chprintlnf (victim, "%s tried to purge you!", ch->name);
	  return;
	}

      act ("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

      if (victim->level > 1)
	save_char_obj (victim);
      d = victim->desc;
      extract_char (victim, true);
      if (d != NULL)
	close_socket (d);

      return;
    }

  act ("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
  extract_char (victim, true);
  return;
}

Do_Fun (do_advance)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  int level;
  int iLevel;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2) || !is_number (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <level>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      chprintln (ch, "That player is not here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  if ((level = atoi (arg2)) < 1 || level > MAX_LEVEL)
    {
      chprintlnf (ch, "Level must be 1 to %d.", MAX_LEVEL);
      return;
    }

  if (level > get_trust (ch))
    {
      chprintln (ch, "Limited to your trust level.");
      return;
    }


  if (level <= victim->level)
    {
      int temp_prac;

      chprintln (ch, "Lowering a player's level!");
      chprintln (victim, "**** OOOOHHHHHHHHHH  NNNNOOOO ****");
      temp_prac = victim->practice;
      if (!IsNPC (victim) && (victim->level >= LEVEL_IMMORTAL)
	  || (level >= LEVEL_IMMORTAL))
	{
	  update_wizlist (victim, level);
	  if (IsSet (victim->act, PLR_HOLYLIGHT))
	    RemBit (victim->act, PLR_HOLYLIGHT);
	}
      victim->level = 1;
      victim->exp = exp_per_level (victim, victim->pcdata->points);
      victim->max_hit = 10;
      victim->max_mana = 100;
      victim->max_move = 100;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level (victim, true);
      victim->practice = temp_prac;
    }
  else
    {
      chprintln (ch, "Raising a player's level!");
      chprintln (victim, "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****");
      if (!IsNPC (victim) && (victim->level >= LEVEL_IMMORTAL)
	  || (level >= LEVEL_IMMORTAL))
	{
	  update_wizlist (victim, level);
	}
    }

  for (iLevel = victim->level; iLevel < level; iLevel++)
    {
      victim->level += 1;
      advance_level (victim, true);
    }
  chprintlnf (victim, "You are now level %d.", victim->level);
  victim->exp =
    exp_per_level (victim, victim->pcdata->points) * Max (1, victim->level);
  victim->trust = 0;
  save_char_obj (victim);
  return;
}

Do_Fun (do_trust)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CharData *victim;
  int level;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2) || !is_number (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <level>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      chprintln (ch, "That player is not here.");
      return;
    }

  if ((level = atoi (arg2)) < 0 || level > MAX_LEVEL)
    {
      chprintlnf (ch, "Level must be 0 (reset) or 1 to %d.", MAX_LEVEL);
      return;
    }

  if (level > get_trust (ch))
    {
      chprintln (ch, "Limited to your trust.");
      return;
    }

  victim->trust = level;
  return;
}

Do_Fun (do_restore)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  CharData *vch;
  Descriptor *d;

  one_argument (argument, arg);
  if (NullStr (arg) || !str_cmp (arg, "room"))
    {


      for (vch = ch->in_room->person_first; vch != NULL;
	   vch = vch->next_in_room)
	{
	  affect_strip (vch, gsn_plague);
	  affect_strip (vch, gsn_poison);
	  affect_strip (vch, gsn_blindness);
	  affect_strip (vch, gsn_sleep);
	  affect_strip (vch, gsn_curse);

	  vch->hit = vch->max_hit;
	  vch->mana = vch->max_mana;
	  vch->move = vch->max_move;
	  update_pos (vch);
	  act ("$n has restored you.", ch, NULL, vch, TO_VICT);
	}

      new_wiznet (ch, NULL, WIZ_RESTORE, false, get_trust (ch),
		  "$N restored room %ld.", ch->in_room->vnum);

      chprintln (ch, "Room restored.");
      return;

    }

  if (get_trust (ch) >= MAX_LEVEL - 1 && !str_cmp (arg, "all"))
    {


      for (d = descriptor_first; d != NULL; d = d->next)
	{
	  victim = d->character;

	  if (victim == NULL || IsNPC (victim))
	    continue;

	  affect_strip (victim, gsn_plague);
	  affect_strip (victim, gsn_poison);
	  affect_strip (victim, gsn_blindness);
	  affect_strip (victim, gsn_sleep);
	  affect_strip (victim, gsn_curse);

	  victim->hit = victim->max_hit;
	  victim->mana = victim->max_mana;
	  victim->move = victim->max_move;
	  update_pos (victim);
	  if (victim->in_room != NULL)
	    act ("$n has restored you.", ch, NULL, victim, TO_VICT);
	}
      chprintln (ch, "All active players restored.");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  affect_strip (victim, gsn_plague);
  affect_strip (victim, gsn_poison);
  affect_strip (victim, gsn_blindness);
  affect_strip (victim, gsn_sleep);
  affect_strip (victim, gsn_curse);
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  update_pos (victim);
  act ("$n has restored you.", ch, NULL, victim, TO_VICT);
  new_wiznet (ch, NULL, WIZ_RESTORE, false, get_trust (ch),
	      "$N restored %s", GetName (victim));
  chprintln (ch, "Ok.");
  return;
}

Do_Fun (do_freeze)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Freeze whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (IsSet (victim->act, PLR_FREEZE))
    {
      RemBit (victim->act, PLR_FREEZE);
      chprintln (victim, "You can play again.");
      chprintln (ch, "FREEZE removed.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0, "$N thaws %s.",
		  GetName (victim));
    }
  else
    {
      SetBit (victim->act, PLR_FREEZE);
      chprintln (victim, "You can't do ANYthing!");
      chprintln (ch, "FREEZE set.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N puts %s in the deep freeze.", GetName (victim));
    }

  save_char_obj (victim);

  return;
}

Do_Fun (do_log)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Log whom?");
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      if (IsSet (mud_info.mud_flags, MUD_LOGALL))
	{
	  RemBit (mud_info.mud_flags, MUD_LOGALL);
	  chprintln (ch, "Log ALL off.");
	}
      else
	{
	  SetBit (mud_info.mud_flags, MUD_LOGALL);
	  chprintln (ch, "Log ALL on.");
	}
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }


  if (IsSet (victim->act, PLR_LOG))
    {
      RemBit (victim->act, PLR_LOG);
      chprintln (ch, "LOG removed.");
    }
  else
    {
      SetBit (victim->act, PLR_LOG);
      chprintln (ch, "LOG set.");
    }

  return;
}

Do_Fun (do_noemote)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Noemote whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (IsSet (victim->comm, COMM_NOEMOTE))
    {
      RemBit (victim->comm, COMM_NOEMOTE);
      chprintln (victim, "You can emote again.");
      chprintln (ch, "NOEMOTE removed.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N restores emotes to %s.", GetName (victim));
    }
  else
    {
      SetBit (victim->comm, COMM_NOEMOTE);
      chprintln (victim, "You can't emote!");
      chprintln (ch, "NOEMOTE set.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N revokes %s's emotes.", GetName (victim));
    }

  return;
}

Do_Fun (do_noshout)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Noshout whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (IsSet (victim->comm, COMM_NOSHOUT))
    {
      RemBit (victim->comm, COMM_NOSHOUT);
      chprintln (victim, "You can shout again.");
      chprintln (ch, "NOSHOUT removed.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, false, 0,
		  "$N restores shouts to %s.", GetName (victim));
    }
  else
    {
      SetBit (victim->comm, COMM_NOSHOUT);
      chprintln (victim, "You can't shout!");
      chprintln (ch, "NOSHOUT set.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N revokes %s's shouts.", GetName (victim));
    }

  return;
}

Do_Fun (do_notell)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Notell whom?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      chprintln (ch, "You failed.");
      return;
    }

  if (IsSet (victim->comm, COMM_NOTELL))
    {
      RemBit (victim->comm, COMM_NOTELL);
      chprintln (victim, "You can tell again.");
      chprintln (ch, "NOTELL removed.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N restores tells to %s.", GetName (victim));
    }
  else
    {
      SetBit (victim->comm, COMM_NOTELL);
      chprintln (victim, "You can't tell!");
      chprintln (ch, "NOTELL set.");
      new_wiznet (ch, NULL, WIZ_PENALTIES, true, 0,
		  "$N revokes %s's tells.", GetName (victim));
    }

  return;
}

Do_Fun (do_peace)
{
  CharData *rch;

  for (rch = ch->in_room->person_first; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting != NULL)
	stop_fighting (rch, true);
      if (IsNPC (rch) && IsSet (rch->act, ACT_AGGRESSIVE))
	RemBit (rch->act, ACT_AGGRESSIVE);
    }

  chprintln (ch, "Ok.");
  return;
}

Do_Fun (do_wizlock)
{
  if (!IsSet (mud_info.mud_flags, MUD_WIZLOCK))
    {
      wiznet ("$N has wizlocked the game.", ch, NULL, 0, true, 0);
      chprintln (ch, "Game wizlocked.");
      SetBit (mud_info.mud_flags, MUD_WIZLOCK);
    }
  else
    {
      wiznet ("$N removes wizlock.", ch, NULL, 0, true, 0);
      chprintln (ch, "Game un-wizlocked.");
      RemBit (mud_info.mud_flags, MUD_WIZLOCK);
    }

  return;
}



Do_Fun (do_newlock)
{
  if (!IsSet (mud_info.mud_flags, MUD_NEWLOCK))
    {
      wiznet ("$N locks out new characters.", ch, NULL, 0, true, 0);
      chprintln (ch, "New characters have been locked out.");
      SetBit (mud_info.mud_flags, MUD_NEWLOCK);
    }
  else
    {
      wiznet ("$N allows new characters back in.", ch, NULL, 0, true, 0);
      chprintln (ch, "Newlock removed.");
      RemBit (mud_info.mud_flags, MUD_NEWLOCK);
    }

  return;
}

Do_Fun (do_slookup)
{
  char arg[MAX_INPUT_LENGTH];
  int sn;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Lookup which skill or spell?");
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      for (sn = 0; sn < top_skill; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  chprintlnf (ch, "Sn: %3d  Skill/spell: '%s'",
		      sn, skill_table[sn].name);
	}
    }
  else
    {
      if ((sn = skill_lookup (arg)) < 0)
	{
	  chprintln (ch, "No such skill or spell.");
	  return;
	}

      chprintlnf (ch, "Sn: %3d  Skill/spell: '%s'", sn, skill_table[sn].name);
    }

  return;
}

Do_Fun (do_sset)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CharData *victim;
  int value;
  int sn;
  bool fAll;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (NullStr (arg1) || NullStr (arg2) || NullStr (arg3))
    {
      cmd_syntax (ch, NULL, n_fun, "<name> <spell or skill> <value>",
		  "<name> all <value>", NULL);
      chprintln (ch, "   (use the name of the skill, not the number)");
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on NPC's.");
      return;
    }

  fAll = !str_cmp (arg2, "all");
  sn = 0;
  if (!fAll && (sn = skill_lookup (arg2)) < 0)
    {
      chprintln (ch, "No such skill or spell.");
      return;
    }


  if (!is_number (arg3))
    {
      chprintln (ch, "Value must be numeric.");
      return;
    }

  value = atoi (arg3);
  if (value < 0 || value > 100)
    {
      chprintln (ch, "Value range is 0 to 100.");
      return;
    }

  if (fAll)
    {
      for (sn = 0; sn < top_skill; sn++)
	{
	  if (skill_table[sn].name != NULL)
	    victim->pcdata->learned[sn] = value;
	}
    }
  else
    {
      victim->pcdata->learned[sn] = value;
    }

  return;
}

int
compare_descriptors (const void *v1, const void *v2)
{
  return (*(Descriptor * *)v2)->descriptor -
    (*(Descriptor * *)v1)->descriptor;
}

Do_Fun (do_sockets)
{
  Buffer *buf;
  char arg[MAX_INPUT_LENGTH];
  Descriptor *d, **dlist;
  CharData *vch;
  int count, i, j;

  count = 0;

  one_argument (argument, arg);

  alloc_mem (dlist, Descriptor *, top_descriptor);
  buf = new_buf ();

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if ((vch = CH (d)) != NULL && can_see (ch, vch) &&
	  (NullStr (arg) || is_name (arg, vch->name)))
	{
	  dlist[count++] = d;
	}
    }

  qsort (dlist, count, sizeof (d), compare_descriptors);

  j = count;

  bprintlnf (buf, "Desc %-5s %-12s %-15s %s (Connected)", "Port",
	     "Name", "IP Addr", "Host Name");
  bprintln (buf, draw_line (ch, NULL, 0));
  while (--j >= 0)
    {
      i = nanny_lookup (dlist[j]->connected);

      bprintlnf (buf, "%-4d %-5d %-12s %-15s %s (%s)",
		 dlist[j]->descriptor, dlist[j]->port,
		 CH (dlist[j]) ? CH (dlist[j])->name : "unknown",
		 print_ip (dlist[j]->ip), dlist[j]->host,
		 i == -1 ? "Bad Connection" : nanny_table[i].name);
    }
  bprintln (buf, draw_line (ch, NULL, 0));
  if (count == 0)
    bprintln (buf, "No one by that name is connected.");
  else
    bprintlnf (buf, "%s found.", intstr (count, "user"));
  sendpage (ch, buf_string (buf));
  free_buf (buf);
  free_mem (dlist);
  return;
}


Do_Fun (do_force)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (NullStr (arg) || NullStr (argument))
    {
      chprintln (ch, "Force whom to do what?");
      return;
    }

  one_argument (argument, arg2);

  if (!str_cmp (arg2, "delete") || !str_prefix (arg2, "mob"))
    {
      chprintln (ch, "That will NOT be done.");
      return;
    }

  sprintf (buf, "$n forces you to '%s'.", argument);

  if (!str_cmp (arg, "all"))
    {
      CharData *vch;
      CharData *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 3)
	{
	  chprintln (ch, "Not at your level!");
	  return;
	}

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IsNPC (vch) && get_trust (vch) < get_trust (ch))
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else if (!str_cmp (arg, "players"))
    {
      CharData *vch;
      CharData *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 2)
	{
	  chprintln (ch, "Not at your level!");
	  return;
	}

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IsNPC (vch) && get_trust (vch) < get_trust (ch) &&
	      vch->level < MAX_MORTAL_LEVEL)
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else if (!str_cmp (arg, "gods"))
    {
      CharData *vch;
      CharData *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 2)
	{
	  chprintln (ch, "Not at your level!");
	  return;
	}

      for (vch = char_first; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IsNPC (vch) && get_trust (vch) < get_trust (ch) &&
	      vch->level >= MAX_MORTAL_LEVEL)
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else
    {
      CharData *victim;

      if ((victim = get_char_world (ch, arg)) == NULL)
	{
	  chprintln (ch, "They aren't here.");
	  return;
	}

      if (victim == ch)
	{
	  chprintln (ch, "Aye aye, right away!");
	  return;
	}

      if (!is_room_owner (ch, victim->in_room) &&
	  ch->in_room != victim->in_room &&
	  room_is_private (victim->in_room) && !IsTrusted (ch, IMPLEMENTOR))
	{
	  chprintln (ch, "That character is in a private room.");
	  return;
	}

      if (get_trust (victim) >= get_trust (ch))
	{
	  chprintln (ch, "Do it yourself!");
	  return;
	}

      if (!IsNPC (victim) && get_trust (ch) < MAX_LEVEL - 3)
	{
	  chprintln (ch, "Not at your level!");
	  return;
	}

      act (buf, ch, NULL, victim, TO_VICT);
      interpret (victim, argument);
    }

  chprintln (ch, "Ok.");
  return;
}


Do_Fun (do_invis)
{
  int level;
  char arg[MAX_STRING_LENGTH];


  one_argument (argument, arg);

  if (NullStr (arg))


    if (ch->invis_level)
      {
	ch->invis_level = 0;
	act ("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
	chprintln (ch, "You slowly fade back into existence.");
      }
    else
      {
	ch->invis_level = get_trust (ch);
	act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	chprintln (ch, "You slowly vanish into thin air.");
      }
  else

    {
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  chprintln (ch, "Invis level must be between 2 and your level.");
	  return;
	}
      else
	{
	  ch->reply = NULL;
	  ch->invis_level = level;
	  act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	  chprintln (ch, "You slowly vanish into thin air.");
	}
    }

  return;
}

Do_Fun (do_incognito)
{
  int level;
  char arg[MAX_STRING_LENGTH];


  one_argument (argument, arg);

  if (NullStr (arg))


    if (ch->incog_level)
      {
	ch->incog_level = 0;
	act ("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
	chprintln (ch, "You are no longer cloaked.");
      }
    else
      {
	ch->incog_level = get_trust (ch);
	act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	chprintln (ch, "You cloak your presence.");
      }
  else

    {
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  chprintln (ch, "Incog level must be between 2 and your level.");
	  return;
	}
      else
	{
	  ch->reply = NULL;
	  ch->incog_level = level;
	  act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	  chprintln (ch, "You cloak your presence.");
	}
    }

  return;
}

Do_Fun (do_holylight)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_HOLYLIGHT, "Holy light mode on.",
	      "Holy light mode off.");

  return;
}



Do_Fun (do_prefix)
{
  if (NullStr (argument))
    {
      if (NullStr (ch->prefix))
	{
	  chprintln (ch, "You have no prefix to clear.");
	  return;
	}

      chprintln (ch, "Prefix removed.");
      replace_str (&ch->prefix, "");
      return;
    }

  if (!NullStr (ch->prefix))
    {
      chprintlnf (ch, "Prefix changed to %s.", argument);
      replace_str (&ch->prefix, argument);
    }
  else
    {
      chprintlnf (ch, "Prefix set to %s.", argument);
      replace_str (&ch->prefix, argument);
    }
}




Do_Fun (do_copyover)
{
  do_crs (n_fun, ch, argument, CRS_COPYOVER);
  return;
}

void
copyover (void)
{
  FileData *fp;
  Descriptor *d, *d_next;
  char exec[1024], arg1[100], arg2[100], arg3[100];
  int pid;

  fp = f_open (COPYOVER_FILE, "w");

  if (!fp)
    {
      logf ("Could not write to copyover file: %s", COPYOVER_FILE);
      log_error ("do_copyover:file open");
      return;
    }

  crs_info.status = CRS_COPYOVER;

  f_printf (fp, TIME_T_FMT LF, boot_time);


  for (d = descriptor_first; d; d = d_next)
    {
      CharData *och = CH (d);

      d_next = d->next;

      if (!och || d->connected > CON_PLAYING)
	{
	  d_write (d, NEWLINE
		   "Sorry, we are rebooting. Come back in a few minutes."
		   NEWLINE, 0);
	  close_socket (d);
	}
      else
	{
	  f_printf (fp, "D %d %s" LF, d->descriptor, och->name);

	  if (och->fighting != NULL)
	    {
	      stop_fighting (och->fighting, true);
	      stop_fighting (och, true);
	    }
	  extract_arena (och);

	  update_pos (och);
	  save_char_obj (och);

	  d_write (d, NEWLINE, 2);
	  d_write (d, crs_sprintf (false, true), 0);
	  d_write (d, NEWLINE, 2);

#ifndef DISABLE_MCCP

	  compressEnd (d, d->mccp_version);
#endif

	}
    }

  f_printf (fp, "#" LF);
  f_close (fp);


#ifndef DISABLE_I3

  if (I3_is_connected ())
    {
      rw_i3_chanlist (act_write);
      rw_i3_mudlist (act_write);
      I3_savehistory ();
    }
#endif

  cleanup_mud ();


  sprintf (exec, "%s" DIR_SYM "%s", CWDIR, EXE_FILE);
  sprintf (arg1, "%d", mud_control);
  sprintf (arg2, "%d", mainport);
#ifndef DISABLE_I3
  sprintf (arg3, "%d", I3_control);
#else

  sprintf (arg3, "-1");
#endif
  pid = execl (exec, EXE_FILE, "-c", arg1, arg2, arg3, (char *) NULL);

  if (pid == -1)
    {

      logf_error ("copyover: execl %s", exec);
      announce (NULL, INFO_ALL, "Copyover FAILED!");

      if (fpReserve == NULL)
	fpReserve = fopen (NULL_FILE, "r");
    }
#ifdef WIN32
  else
    cwait (NULL, pid, WAIT_CHILD);
#endif
}


void
copyover_recover ()
{
  Descriptor *d;
  FileData *fp;
  char *name;
  int desc;
  char letter;
  bool fOld;

  logf ("Copyover recovery initiated");
  crs_info.status = CRS_COPYOVER_RECOVER;

  fp = f_open (COPYOVER_FILE, "r");

  if (!fp)
    {
      log_error ("copyover_recover:file open");
      logf ("Copyover file not found. Exitting.");
      exit (1);
    }

  unlink (COPYOVER_FILE);

  boot_time = read_long (fp);

  mud_info.last_copyover = getcurrenttime ();

  letter = read_letter (fp);

  while (letter != '#')
    {
      switch (letter)
	{
	case 'D':

	  desc = read_number (fp);

	  name = read_word (fp);

	  d = new_descriptor ();
	  d->descriptor = desc;
	  Link (d, descriptor, next, prev);
	  d->connected = CON_COPYOVER_RECOVER;

	  if (!d_write (d, NEWLINE "Restoring from copyover..." NEWLINE, 0))
	    {
	      close_socket (d);
	      break;
	    }



	  fOld = load_char_obj (d, name);

	  if (!fOld)
	    {
	      d_write (d, NEWLINE
		       "Somehow, your character was lost in the copyover. Sorry."
		       NEWLINE, 0);
	      close_socket (d);
	    }
	  else
	    {

	      d_write (d, NEWLINE "Copyover recovery complete." NEWLINE, 0);

#ifndef DISABLE_MCCP

	      if (d->mccp_version)
		compressStart (d, d->mccp_version);
#endif


	      if (!d->character->in_room)
		d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);


	      Link (d->character, char, next, prev);

	      Link (d->character, player, next_player, prev_player);

	      update_explored (d->character);
	      update_all_qobjs (d->character);

	      char_to_room (d->character, d->character->in_room);
	      if (d->character->pet != NULL)
		char_to_room (d->character->pet, d->character->in_room);
	      do_function (d->character, &do_look, "auto");
	      do_function (d->character, &do_ncheck, "");
	      checkcorpse (d->character);
	      d->connected = CON_PLAYING;
	    }
	  break;
	default:
	  break;
	}
      read_to_eol (fp);
      letter = read_letter (fp);
    }
  f_close (fp);
#ifdef unix
  if (IsSet (mud_info.cmdline_options, CMDLINE_STARTUP_SCRIPT))
    {
      char buf[MIL];

      sprintf (buf, "%s.pid", EXE_FILE);
      if ((fp = f_open (buf, "w")) != NULL)
	{
	  f_printf (fp, "%d\n", getpid ());
	  f_close (fp);
	}
    }
#endif
  crs_info.status = CRS_NONE;
}

struct qspell_type
{
  Spell_F *spellf;
};

int
sn_spellfun_lookup (Spell_F * fun)
{
  int sn;

  for (sn = 0; sn < top_skill; sn++)
    {
      if (skill_table[sn].spell_fun == fun)
	return sn;
    }
  return -1;
}

const struct qspell_type qspell_table[] = {
  {
   spell_bless},
  {spell_giant_strength},
  {spell_haste},
  {spell_frenzy},
  {spell_shield},
  {spell_armor},
  {spell_sanctuary},
  {spell_detect_hidden},
  {spell_detect_invis},
  {spell_stone_skin},
  {spell_bark_skin},
  {spell_forceshield},
  {spell_staticshield},
  {spell_flameshield},
  {NULL},
};

Do_Fun (do_spellup)
{
  CharData *vch;
  char arg[MIL];
  Descriptor *tempdesc;
  int i, sn;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "spellup [all, room, <char>]", NULL);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      tempdesc = ch->desc;
      ch->desc = NULL;
      for (vch = player_first; vch != NULL; vch = vch->next_player)
	{
	  if (vch == ch || !can_see (ch, vch))
	    continue;

	  do_function (ch, &do_spellup, vch->name);
	}
      ch->desc = tempdesc;
      chprintln (ch, "OK.");
    }
  else if (!str_cmp (arg, "room"))
    {

      tempdesc = ch->desc;
      ch->desc = NULL;
      for (vch = ch->in_room->person_first; vch; vch = vch->next_in_room)
	{
	  if (vch == ch || IsNPC (vch) || !can_see (ch, vch))
	    continue;

	  do_function (ch, &do_spellup, vch->name);
	}
      ch->desc = tempdesc;
      chprintln (ch, "OK.");
    }
  else if ((vch = get_char_world (ch, arg)) != NULL)
    {

      tempdesc = ch->desc;
      ch->desc = NULL;
      for (i = 0, sn = 0; qspell_table[i].spellf != NULL; i++)
	{
	  sn = sn_spellfun_lookup (qspell_table[i].spellf);
	  if (sn == -1)
	    continue;
	  if (is_affected (vch, sn))
	    continue;
	  qspell_table[i].spellf (sn, get_trust (ch), ch, vch, TARGET_CHAR);
	}
      ch->desc = tempdesc;
      chprintln (ch, "OK.");
    }
  else
    cmd_syntax (ch, NULL, n_fun, "[all, room, <char>]", NULL);

  return;
}


Do_Fun (do_pload)
{
  Descriptor d;
  bool isChar = false;
  char name[MAX_INPUT_LENGTH];

  if (NullStr (argument))
    {
      chprintln (ch, "Load who?");
      return;
    }

  argument = one_argument (argument, name);


  if (get_char_world (ch, name) != NULL)
    {
      chprintln (ch, "That person is already connected!");
      return;
    }

  isChar = load_char_obj (&d, capitalize (name));

  if (!isChar)
    {
      chprintln (ch, "Load Who? Are you sure? I cant seem to find them.");
      return;
    }

  d.character->desc = NULL;
  Link (d.character, char, next, prev);
  Link (d.character, player, next_player, prev_player);
  d.connected = CON_PLAYING;
  reset_char (d.character);


  if (d.character->in_room != NULL)
    {
      char_to_room (d.character, ch->in_room);
    }

  act ("$n has pulled $N from the pattern!", ch, NULL, d.character, TO_ROOM);

  if (d.character->pet != NULL)
    {
      char_to_room (d.character->pet, d.character->in_room);
      act ("$n has entered the game.", d.character->pet, NULL, NULL, TO_ROOM);
    }

}


Do_Fun (do_punload)
{
  CharData *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument (argument, who);

  if ((victim = get_char_world (ch, who)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }


  if (victim->desc != NULL)
    {
      chprintln (ch, "I dont think that would be a good idea...");
      return;
    }

  if (victim->was_in_room != NULL)
    {
      char_to_room (victim, victim->was_in_room);
      if (victim->pet != NULL)
	char_to_room (victim->pet, victim->was_in_room);
    }

  save_char_obj (victim);
  do_function (victim, &do_quit, "");

  act ("$n has released $N back to the Pattern.", ch, NULL, victim, TO_ROOM);
}


Do_Fun (do_set_char)
{
  Descriptor d;
  bool result = false;
  char name[MIL];
  CharData *victim;

  argument = first_arg (argument, name, false);

  if (NullStr (name))
    {
      show_set_commands (ch, char_data_table);
      cmd_syntax (ch, NULL, n_fun, "char <name> <args...>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, name)) == NULL)
    {
      result = load_char_obj (&d, name);
      if (result)
	{
	  if (get_trust (d.character) > get_trust (ch))
	    {
	      chprintln (ch, "Sorry, you can't do that.");
	      free_char (d.character);

	      return;
	    }
	  victim = d.character;
	}
      else
	{
	  chprintln (ch, "There is no such character.");
	  free_char (d.character);

	  return;
	}
    }
  set_struct (ch, argument, &char_zero, char_data_table, victim);
  if (result)
    free_char (victim);

  return;
}

Do_Fun (do_set_pc)
{
  Descriptor d;
  bool result = false;
  char name[MIL];
  CharData *victim;

  argument = first_arg (argument, name, false);

  if (NullStr (name))
    {
      show_set_commands (ch, pcdata_data_table);
      cmd_syntax (ch, NULL, n_fun, "player <name> <args...>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, name)) == NULL || IsNPC (victim))
    {
      result = load_char_obj (&d, name);
      if (result)
	{
	  if (get_trust (d.character) > get_trust (ch))
	    {
	      chprintln (ch, "Sorry, you can't do that.");
	      free_char (d.character);

	      return;
	    }
	  victim = d.character;
	}
      else
	{
	  chprintln (ch, "There is no such player.");
	  free_char (d.character);

	  return;
	}
    }
  if (!victim->pcdata)
    {
      chprintln (ch, "No such player.");
      if (result)
	free_char (victim);

      return;
    }
  set_struct (ch, argument, &pcdata_zero, pcdata_data_table, victim->pcdata);
  if (result)
    free_char (victim);

  return;
}

Do_Fun (do_set_desc)
{
  char name[MIL];
  CharData *victim;
  Descriptor *d = NULL;

  argument = first_arg (argument, name, false);

  if (NullStr (name))
    {
      show_set_commands (ch, descriptor_data_table);
      cmd_syntax (ch, NULL, n_fun, "descriptor <name> <args...>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, name)) == NULL || !victim->desc)
    {
      if (is_number (name))
	{
	  for (d = descriptor_first; d; d = d->next)
	    if ((SOCKET) atoi (name) == d->descriptor)
	      break;
	}
      if (!d)
	{
	  chprintln (ch, "Player has no descriptor.");
	  return;
	}
    }
  else
    d = victim->desc;
  set_struct (ch, argument, &descriptor_zero, descriptor_data_table, d);
  return;
}

Do_Fun (do_set_obj)
{
  ObjData *obj;
  char name[MIL];

  argument = first_arg (argument, name, false);

  if (NullStr (name))
    {
      show_set_commands (ch, obj_data_table);
      cmd_syntax (ch, NULL, n_fun, "object <name> <args...>", NULL);
      return;
    }

  if ((obj = get_obj_world (ch, name)) == NULL)
    {
      chprintln (ch, "There is no such object.");
      return;
    }
  set_struct (ch, argument, &obj_zero, obj_data_table, obj);
  return;
}

Do_Fun (do_set_help)
{
  cmd_syntax (ch, NULL, n_fun, "char <name> <option> <value>",
	      "obj <name> <option> <value>",
	      "desc <name> <option> <value>",
	      "skill <name> <skill|all> <value>", NULL);
}

Do_Fun (do_set)
{
  vinterpret (ch, n_fun, argument, "character", do_set_char, "mobile",
	      do_set_char, "player", do_set_pc, "object", do_set_obj,
	      "descriptor", do_set_desc, "skill", do_sset, "spell",
	      do_sset, NULL, do_set_help);
}


Do_Fun (do_stat_char)
{
  CharData *victim;
  Buffer *output;

  if (NullStr (argument))
    {
      chprintln (ch, "Stat who?");
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "No such character.");
      return;
    }

  output = new_buf ();

  bprintln (output, draw_line (ch, "<>", 0));

  show_struct (ch, output, &char_zero, char_data_table, victim, 0);

  bprintln (output, draw_line (ch, "<>", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_stat_obj)
{
  ObjData *obj;
  Buffer *output;

  if (NullStr (argument))
    {
      chprintln (ch, "Stat what?");
      return;
    }

  if ((obj = get_obj_world (ch, argument)) == NULL
      || (obj = get_obj_here (ch, NULL, argument)) == NULL)
    {
      chprintln (ch, "No such object.");
      return;
    }

  output = new_buf ();

  bprintln (output, draw_line (ch, "<>", 0));

  show_struct (ch, output, &obj_zero, obj_data_table, obj, 0);

  bprintln (output, draw_line (ch, "<>", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_stat_room)
{
  RoomIndex *room;
  Buffer *output;

  if ((room = get_room_index (atov (argument))) == NULL)
    {
      room = ch->in_room;
    }

  output = new_buf ();

  bprintln (output, draw_line (ch, "<>", 0));

  show_struct (ch, output, &room_index_zero, room_index_data_table, room, 0);

  bprintln (output, draw_line (ch, "<>", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_stat_desc)
{
  Descriptor *d;
  CharData *victim;
  Buffer *output;

  if (NullStr (argument))
    {
      chprintln (ch, "Stat who?");
      return;
    }

  d = NULL;
  if ((victim = get_char_world (ch, argument)) == NULL
      || victim->desc == NULL)
    {
      if (is_number (argument))
	{
	  for (d = descriptor_first; d; d = d->next)
	    if ((SOCKET) atoi (argument) == d->descriptor)
	      break;
	}
      if (!d)
	{
	  chprintln (ch, "No such character.");
	  return;
	}
    }
  else
    d = victim->desc;

  output = new_buf ();

  bprintln (output, draw_line (ch, "<>", 0));

  show_struct (ch, output, &descriptor_zero, descriptor_data_table, d, 0);

  bprintln (output, draw_line (ch, "<>", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_stat_area)
{
  AreaData *area;
  Buffer *output;

  if ((area = area_lookup (argument)) == NULL)
    {
      area = ch->in_room->area;
    }

  output = new_buf ();

  bprintln (output, draw_line (ch, "<>", 0));

  show_struct (ch, output, &area_zero, area_data_table, area, 0);

  bprintln (output, draw_line (ch, "<>", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_stat_help)
{
  cmd_syntax (ch, NULL, n_fun, "char <name>",
	      "desc <desc>",
	      "room <name>",
	      "area <name>", "object <name>", "old <args>", NULL);
}

Do_Fun (do_stat)
{
  vinterpret (ch, n_fun, argument, "char", do_stat_char, "room",
	      do_stat_room, "descriptor", do_stat_desc,
	      "object", do_stat_obj, "area", do_stat_area,
	      NULL, do_stat_help);
}


const char *
bonus_name (bonus_t type)
{
  switch (type)
    {
    case BONUS_XP:
      return "exp";
    case BONUS_QP:
      return "qp";
    case BONUS_DAM:
      return "damage";
    default:
      bugf ("bad bonus type - %d", type);
      return "";
    }
}

void
set_bonus (bonus_t status, int mod, int time, const char *format, ...)
{
  char buf[MPL];
  va_list args;

  va_start (args, format);
  vsnprintf (buf, sizeof (buf), format, args);
  va_end (args);

  mud_info.bonus.status = status;
  mud_info.bonus.mod = Max (mod, mud_info.bonus.mod);
  if (time != -1)
    mud_info.bonus.time = Max (time, mud_info.bonus.time);
  else
    mud_info.bonus.time = time;

  replace_str (&mud_info.bonus.msg, buf);

  if (time != -1)

    announce (NULL, INFO_MISC,
	      "%dx %s has been activated for %s %s.",
	      mud_info.bonus.mod, bonus_name (mud_info.bonus.status),
	      intstr (mud_info.bonus.time, "minute"), mud_info.bonus.msg);

  else

    announce (NULL, INFO_MISC,
	      "%dx %s has been activated infinitely %s.",
	      mud_info.bonus.mod, bonus_name (mud_info.bonus.status),
	      mud_info.bonus.msg);
}

Do_Fun (do_bonus)
{
  char typearg[MIL], modarg[MIL], timearg[MIL], msg[MIL];
  bonus_t type;
  int mod, time;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "<type (xp, qp, dam)> <mod> <time> [msg]", "off", NULL);
      return;
    }

  argument = one_argument (argument, typearg);

  switch (strswitch (typearg, "xp", "qp", "dam", "off", NULL))
    {
    case 1:
      type = BONUS_XP;
      break;
    case 2:
      type = BONUS_QP;
      break;
    case 3:
      type = BONUS_DAM;
      break;
    case 4:
      {
	announce (ch, INFO_MISC, "$n has turned off %dx %s.",
		  mud_info.bonus.mod, bonus_name (mud_info.bonus.status));
	chprintlnf (ch, "You turn off %dx %s.", mud_info.bonus.mod,
		    bonus_name (mud_info.bonus.status));
	mud_info.bonus.mod = 0;
	mud_info.bonus.time = 0;
	mud_info.bonus.status = BONUS_OFF;
	replace_str (&mud_info.bonus.msg, "");
	return;
      }
    default:
      chprintln (ch, "Thats not a valid bonus type. (xp, qp, dam)");
      return;
    }

  argument = one_argument (argument, modarg);

  mod = atoi (modarg);

  if (mod < 1 || mod > 4)
    {
      chprintln (ch, "Modifier should be greater than 0 and less than 5.");
      return;
    }

  argument = one_argument (argument, timearg);

  time = atoi (timearg);

  if (time < 5)
    {
      chprintln (ch, "Time (in game ticks) must be greater than 4.");
      return;
    }

  if (NullStr (argument))
    sprintf (msg, "in honour of %s", GetName (ch));
  else if (is_exact_player_name (argument))
    sprintf (msg, "in honour of %s", capitalize (argument));
  else
    strcpy (msg, argument);

  set_bonus (type, mod, time, msg);
}

Do_Fun (do_system)
{
  char buf[MIL];
  FILE *fp;
  char out[MPL];
  char *shell;

  if (IsNPC (ch))
    {
      chprintln (ch, "Huh?");
      return;
    }

  if (!(IsTrusted (ch, MAX_LEVEL) && ch->level >= MAX_LEVEL - 1))
    {
      chprintln (ch, "Huh?");
      return;
    }
  if (NullStr (argument))
    {
      char sysver[MIL];

#ifdef WIN32
      OSVERSIONINFO osvi;

      osvi.dwOSVersionInfoSize = sizeof (osvi);
      GetVersionEx (&osvi);

      sprintf (sysver,
	       "%s v%d.%d.%d [%s]",
	       osvi.dwPlatformId == VER_PLATFORM_WIN32s ? "Win32s" :
	       osvi.dwPlatformId ==
	       VER_PLATFORM_WIN32_WINDOWS ? "Windows9x" : osvi.dwPlatformId
	       ==
	       VER_PLATFORM_WIN32_NT ? "WindowsNT" : FORMATF ("Unknown(%d)",
							      osvi.
							      dwPlatformId),
	       osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber,
	       osvi.szCSDVersion);
#elif defined HAVE_SYS_UTSNAME_H
      struct utsname name;

      if (uname (&name) == -1)
	sprintf (sysver, "Unknown-uname_error%d", errno);
      else
	sprintf (sysver, "%s %s", name.sysname, name.version);
#elif defined __CYGWIN__
      strcpy (sysver, "Cygwin.");
#else

      strcpy (sysver, "Unknown.");
#endif
      cmd_syntax (ch, NULL, n_fun, "<args...>", NULL);
      chprintlnf (ch,
		  NEWLINE
		  "This command executes a command on the current system (%s) "
		  "by passing a command through '/bin/sh -c'. "
		  "While this seems like fun, its very dangerous, the wrong command could"
		  " harm the mud or the system itself, please be careful!",
		  sysver);
      return;
    }
  chprintlnf (ch, "Performing system command '%s'...", argument);

#ifdef WIN32
  shell = getenv ("ComSpec");
  sprintf (buf, "%s /c %s", shell ? shell : "cmd.exe", argument);
#else
  shell = getenv ("SHELL");
  sprintf (buf, "%s -c %s", shell ? shell : "/bin/sh", argument);
#endif
  if ((fp = popen (buf, "r")) == NULL)
    {
      chprintlnf (ch, "Error processing %s", buf);
      return;
    }
  fread (out, MPL - 1000, 1, fp);
  sendpage (ch, out);
  chprintln (ch, NULL);
  pclose (fp);
  return;
}

Do_Fun (do_helpcheck)
{
  HelpData *pHelp;
  int i;
  Column c;
  bool found;
  Buffer *out;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "commands", "skills", "groups", "classes",
		  "clans", "races", "deities", "areas", "channels", "common",
		  NULL);
      return;
    }

  out = new_buf ();
  set_cols (&c, ch, 4, COLS_BUF, out);
  found = false;

  if (!str_prefix (argument, "commands"))
    {
      CmdData *cmd;

      bprintln (out, "          ----[ MISSING COMMAND HELP FILES ]----");
      for (cmd = cmd_first_sorted; cmd; cmd = cmd->next_sort)
	{
	  if (cmd->category == CMDCAT_NOSHOW)
	    continue;

	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (cmd->name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, cmd->name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All commands have a help file.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "skills"))
    {
      bprintln (out, "          ----[ MISSING SKILL/SPELL HELP FILES ]----");
      for (i = 0; i < top_skill; i++)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (skill_table[i].name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, skill_table[i].name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All skill/spells have a help file.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "groups"))
    {
      bprintln (out, "          ----[ MISSING GROUP HELP FILES ]----");
      for (i = 0; i < top_group; i++)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (group_table[i].name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, group_table[i].name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All spell groups have a help file.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "classes"))
    {
      bprintln (out, "          ----[ MISSING CLASS HELP FILES ]----");
      for (i = 0; i < top_class; i++)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (class_table[i].name[0], pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, class_table[i].name[0]);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All classes have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "clans"))
    {
      ClanData *clan;

      bprintln (out, "          ----[ MISSING CLAN HELP FILES ]----");
      for (clan = clan_first; clan; clan = clan->next)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (clan->name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, clan->name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All clans have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "races"))
    {
      RaceData *race;

      bprintln (out, "          ----[ MISSING RACE HELP FILES ]----");
      for (race = race_first; race; race = race->next)
	{
	  if (!race->pc_race)
	    continue;

	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (race->name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, race->name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All races have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "deities"))
    {
      DeityData *dd;

      bprintln (out, "          ----[ MISSING DEITY HELP FILES ]----");
      for (dd = deity_first; dd; dd = dd->next)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (dd->name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, dd->name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All deities have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "areas"))
    {
      AreaData *area;

      bprintln (out, "          ----[ MISSING AREA HELP FILES ]----");
      for (area = area_first_sorted; area; area = area->next_sort)
	{
	  if (IsSet (area->area_flags, AREA_CLOSED))
	    continue;

	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (area->name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, area->name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All areas have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "channels"))
    {
      bprintln (out, "          ----[ MISSING CHANNEL HELP FILES ]----");
      for (i = 0; i < top_channel; i++)
	{
	  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	    {
	      if (is_name (channel_table[i].name, pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, channel_table[i].name);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All channels have help files.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else if (!str_prefix (argument, "common"))
    {
      char *const keywords[] = {
	"RULES", "DIKU", "MERC", "ROM CREDITS",
	"1STMUD CREDITS", "AUCTION", "GQUEST",
	"WARFARE", "ARENA", "BOARD", "MSP", "GREETING",
	"STRENGTH", "INTELLIGENCE", "WISDOM", "DEXTERITY",
	"CONSTITUTION", "PLAYER KILLING",
	"MOTD", "IMOTD", "OLC", "AUTHORS", "QUEST",
	"COLOR", "GOD RULES", "HOMES", "NEWBIE",
	"REMORT", "SUMMARY", NULL
      };

      bprintln (out, "          ----[ MISSING COMMON HELP FILES ]----");

      for (i = 0; keywords[i] != NULL; i++)
	{
	  for (pHelp = help_first; pHelp; pHelp = pHelp->next)
	    {
	      if (is_name (keywords[i], pHelp->keyword))
		break;
	    }
	  if (!pHelp)
	    {
	      print_cols (&c, keywords[i]);
	      found = true;
	    }
	}
      if (!found)
	print_cols (&c, "All common help files found.");
      cols_nl (&c);
      sendpage (ch, buf_string (out));
      free_buf (out);
      return;
    }
  else
    do_helpcheck (n_fun, ch, "");
}


bool
valid_name (const char *name)
{
  Descriptor *dc;

  for (dc = descriptor_first; dc; dc = dc->next)
    {
      if (CH (dc) && !str_cmp (name, CH (dc)->name))
	{
	  return false;
	}
    }

  if (strchr (name, COLORCODE))
    {
      return false;
    }

  return check_parse_name (name);
}

bool
rename_char (CharData * victim, const char *name)
{
  char old_name[MSL];

  if (valid_name (name))
    {
      ObjIndex *pObj;
      RoomIndex *pRoom;
      int hash;
      char *path;

      sprintf (old_name, victim->name);

      logf ("%s renamed to %s", old_name, capitalize (name));
      chprintln (victim, "You feel like a new person!");
      update_statlist (victim, true);
      update_webpasses (victim, true);
      if (is_clan (victim))
	update_members (victim, true);
      if (IsImmortal (victim))
	update_wizlist (victim, 1);
      new_wiznet (NULL, NULL, 0, true, 0, "%s renamed to %s.",
		  victim->name, capitalize (name));
      replace_str (&victim->name, capitalize (name));
      save_char_obj (victim);
      update_statlist (victim, false);
      update_webpasses (victim, false);
      if (is_clan (victim))
	update_members (victim, false);
      if (IsImmortal (victim))
	update_wizlist (victim, victim->level);
      unlink (pfilename (old_name, PFILE_NORMAL));
      if (file_exists ((path = pfilename (old_name, PFILE_BACKUP))))
	{
	  unlink (path);
	  backup_char_obj (victim);
	}
      for (hash = 0; hash < MAX_KEY_HASH; hash++)
	{
	  for (pRoom = room_index_hash[hash]; pRoom != NULL;
	       pRoom = pRoom->next)
	    {
	      if (!NullStr (pRoom->owner)
		  && !str_cmp (old_name, pRoom->owner))
		{
		  replace_str (&pRoom->owner, victim->name);
		  SetBit (pRoom->area->area_flags, AREA_CHANGED);
		}
	    }
	}
      if (HOME_KEY (victim) != -1
	  && (pObj = get_obj_index (HOME_KEY (victim))) != NULL)
	{
	  replace_strf (&pObj->name, "key %s home", victim->name);
	  replace_strf (&pObj->short_descr, "%s's key",
			capitalize (victim->name));
	  replace_strf (&pObj->description, "The key to %s's home is here.",
			capitalize (victim->name));
	  SetBit (pObj->area->area_flags, AREA_CHANGED);
	}
      return true;
    }
  return false;
}

Do_Fun (do_rename)
{
  CharData *victim;
  char arg1[MSL];
  char arg2[MSL];

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (IsNPC (ch))
    return;

  if ((NullStr (arg1)) || (NullStr (arg2)))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <newname>", NULL);
      return;
    }

  victim = get_char_world (ch, arg1);

  if (victim == NULL)
    {
      chprintln (ch, "They must be playing.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "Not on IsNPC's");
      return;
    }
  if (rename_char (victim, arg2))
    {
      chprintln (ch, "Rename successfull.");
    }
  else
    {
      chprintln (ch, "They cannot be known as that.");
      return;
    }
}
