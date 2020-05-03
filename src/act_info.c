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
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "vnums.h"


Proto (char *format_obj_to_char, (ObjData *, CharData *, bool));
Proto (void show_list_to_char, (ObjData *, CharData *, bool, bool));
Proto (void show_char_to_char_0, (CharData *, CharData *));
Proto (void show_char_to_char_1, (CharData *, CharData *));
Proto (void show_char_to_char, (CharData *, CharData *));

char *
format_obj_to_char (ObjData * obj, CharData * ch, bool fShort)
{
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if ((fShort && NullStr (obj->short_descr)) || NullStr (obj->description))
    return buf;

  if (IsObjStat (obj, ITEM_INVIS))
    strcat (buf, "({cInvis{x) ");
  if (IsAffected (ch, AFF_DETECT_EVIL) && IsObjStat (obj, ITEM_EVIL))
    strcat (buf, "({RRed Aura{x) ");
  if (IsAffected (ch, AFF_DETECT_GOOD) && IsObjStat (obj, ITEM_BLESS))
    strcat (buf, "({BBlue Aura{x) ");
  if (IsAffected (ch, AFF_DETECT_MAGIC) && IsObjStat (obj, ITEM_MAGIC))
    strcat (buf, "({MMagical{x) ");
  if (IsObjStat (obj, ITEM_GLOW))
    strcat (buf, "({YGlowing{x) ");
  if (IsObjStat (obj, ITEM_HUM))
    strcat (buf, "({CHumming{x) ");

  if (IsQuester (ch) && obj == ch->pcdata->quest.obj)
    strcat (buf, "{r[{RTARGET{r] ");

  strcat (buf, "{x");
  if (fShort)
    {
      if (obj->short_descr != NULL)
	strcat (buf, obj->short_descr);
    }
  else
    {
      if (obj->description != NULL)
	strcat (buf, obj->description);
      if (is_donate_room (ch->in_room->vnum))
	sprintf (buf + strlen (buf), " {g[{w%d{g]{x", obj->level);
    }

  return buf;
}


void
show_list_to_char (ObjData * list, CharData * ch, bool fShort,
		   bool fShowNothing)
{
  Buffer *output;
  const char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  ObjData *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;

  if (ch->desc == NULL)
    return;


  output = new_buf ();

  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    count++;
  alloc_mem (prgpstrShow, const char *, count);
  alloc_mem (prgnShow, int, count);

  nShow = 0;


  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
	{
	  pstrShow = format_obj_to_char (obj, ch, fShort);

	  fCombine = false;

	  if (IsNPC (ch) || IsSet (ch->comm, COMM_COMBINE))
	    {

	      for (iShow = nShow - 1; iShow >= 0; iShow--)
		{
		  if (!str_cmp (prgpstrShow[iShow], pstrShow))
		    {
		      prgnShow[iShow]++;
		      fCombine = true;
		      break;
		    }
		}
	    }


	  if (!fCombine)
	    {
	      prgpstrShow[nShow] = str_dup (pstrShow);
	      prgnShow[nShow] = 1;
	      nShow++;
	    }
	}
    }


  for (iShow = 0; iShow < nShow; iShow++)
    {
      if (prgpstrShow[iShow][0] == '\0')
	{
	  free_string (prgpstrShow[iShow]);
	  continue;
	}

      if (IsNPC (ch) || IsSet (ch->comm, COMM_COMBINE))
	{
	  if (prgnShow[iShow] != 1)
	    {
	      bprintf (output, "(%2d) ", prgnShow[iShow]);
	    }
	  else
	    {
	      bprint (output, "     ");
	    }
	}
      bprintln (output, prgpstrShow[iShow]);
      free_string (prgpstrShow[iShow]);
    }

  if (fShowNothing && nShow == 0)
    {
      if (IsNPC (ch) || IsSet (ch->comm, COMM_COMBINE))
	chprint (ch, "     ");
      chprintln (ch, "Nothing.");
    }
  sendpage (ch, buf_string (output));


  free_buf (output);
  free_mem (prgpstrShow);
  free_mem (prgnShow);

  return;
}

void
show_char_to_char_0 (CharData * victim, CharData * ch)
{
  char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];
  size_t z;

  buf[0] = '\0';

  if (IsSet (victim->comm, COMM_AFK))
    strcat (buf, "{R[{YAFK{R] ");
  if (IsAffected (victim, AFF_INVISIBLE))
    strcat (buf, "({cInvis{x) ");
  if (victim->invis_level >= LEVEL_HERO)
    strcat (buf, "{c({WWizi{c) ");
  if (IsAffected (victim, AFF_HIDE))
    strcat (buf, "({DHide{x) ");
  if (IsAffected (victim, AFF_CHARM))
    strcat (buf, "({MCharmed{x) ");
  if (IsAffected (victim, AFF_PASS_DOOR))
    strcat (buf, "({cTranslucent{x) ");
  if (IsAffected (victim, AFF_FAERIE_FIRE))
    strcat (buf, "({MPink Aura{x) ");
  if (IsEvil (victim) && IsAffected (ch, AFF_DETECT_EVIL))
    strcat (buf, "({RRed Aura{x) ");
  if (IsGood (victim) && IsAffected (ch, AFF_DETECT_GOOD))
    strcat (buf, "({YGolden Aura{x) ");
  if (IsAffected (ch, AFF_FORCE_SHIELD))
    strcat (buf, "({BEnergy{x) ");
  if (IsAffected (ch, AFF_STATIC_SHIELD))
    strcat (buf, "({CStatic{x) ");
  if (IsAffected (ch, AFF_FLAME_SHIELD))
    strcat (buf, "({RFlames{x) ");
  if (IsAffected (victim, AFF_SANCTUARY))
    strcat (buf, "({WWhite Aura{x) ");
  if (InWar (victim))
    strcat (buf, "({RWar{x) ");
  if (!IsNPC (victim) && IsSet (victim->act, PLR_KILLER))
    strcat (buf, "({rKILLER{x) ");
  if (!IsNPC (victim) && IsSet (victim->act, PLR_THIEF))
    strcat (buf, "({rTHIEF{x) ");
  if (IsQuester (ch) && victim == ch->pcdata->quest.mob)
    strcat (buf, "{r[{RTARGET{r] ");

  if (Gquester (ch) && IsNPC (victim)
      && is_gqmob (ch->gquest, victim->pIndexData->vnum) != -1)
    {
      strcat (buf, "{Y({RGquest{Y) ");
    }

  if (LinkDead (victim))
    strcat (buf, "{c[{CLD{c] ");

  strcat (buf, "{x");

  if (victim->position == victim->start_pos && !NullStr (victim->long_descr))
    {
      strcat (buf, victim->long_descr);
      chprint (ch, buf);
      return;
    }

  strcat (buf, Pers (victim, ch));
  if (!IsNPC (victim) && !IsSet (ch->comm, COMM_BRIEF) &&
      victim->position == POS_STANDING && ch->on == NULL)
    strcat (buf, victim->pcdata->title);

  switch (victim->position)
    {
    case POS_DEAD:
      strcat (buf, " is DEAD!!");
      break;
    case POS_MORTAL:
      strcat (buf, " is mortally wounded.");
      break;
    case POS_INCAP:
      strcat (buf, " is incapacitated.");
      break;
    case POS_STUNNED:
      strcat (buf, " is lying here stunned.");
      break;
    case POS_SLEEPING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], SLEEP_AT))
	    {
	      sprintf (message, " is sleeping at %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else if (IsSet (victim->on->value[2], SLEEP_ON))
	    {
	      sprintf (message, " is sleeping on %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else
	    {
	      sprintf (message, " is sleeping in %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	}
      else
	strcat (buf, " is sleeping here.");
      break;
    case POS_RESTING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], REST_AT))
	    {
	      sprintf (message, " is resting at %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else if (IsSet (victim->on->value[2], REST_ON))
	    {
	      sprintf (message, " is resting on %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else
	    {
	      sprintf (message, " is resting in %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	}
      else
	strcat (buf, " is resting here.");
      break;
    case POS_SITTING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], SIT_AT))
	    {
	      sprintf (message, " is sitting at %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else if (IsSet (victim->on->value[2], SIT_ON))
	    {
	      sprintf (message, " is sitting on %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else
	    {
	      sprintf (message, " is sitting in %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	}
      else
	strcat (buf, " is sitting here.");
      break;
    case POS_STANDING:
      if (victim->on != NULL)
	{
	  if (IsSet (victim->on->value[2], STAND_AT))
	    {
	      sprintf (message, " is standing at %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else if (IsSet (victim->on->value[2], STAND_ON))
	    {
	      sprintf (message, " is standing on %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	  else
	    {
	      sprintf (message, " is standing in %s.",
		       victim->on->short_descr);
	      strcat (buf, message);
	    }
	}
      else
	strcat (buf, " is here.");
      break;
    case POS_FIGHTING:
      strcat (buf, " is here, fighting ");
      if (victim->fighting == NULL)
	strcat (buf, "thin air??");
      else if (victim->fighting == ch)
	strcat (buf, "YOU!");
      else if (victim->in_room == victim->fighting->in_room)
	{
	  strcat (buf, Pers (victim->fighting, ch));
	  strcat (buf, ".");
	}
      else
	strcat (buf, "someone who left??");
      break;
    default:
      break;
    }

  z = skipcol (buf);
  buf[z] = toupper (buf[z]);
  chprintln (ch, buf);
  return;
}

void
show_char_to_char_1 (CharData * victim, CharData * ch)
{
  char buf[MAX_STRING_LENGTH];
  ObjData *obj;
  int iWear;
  int percent;
  bool found;
  size_t z;

  if (can_see (victim, ch))
    {
      if (ch == victim)
	act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
      else
	{
	  act ("$n looks at you.", ch, NULL, victim, TO_VICT);
	  act ("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
	}
    }

  if (!NullStr (victim->description))
    {
      chprint (ch, victim->description);
    }
  else
    {
      act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
    }

  if (victim->max_hit > 0)
    percent = (100 * victim->hit) / victim->max_hit;
  else
    percent = -1;

  strcpy (buf, Pers (victim, ch));

  if (percent >= 100)
    strcat (buf, " is in excellent condition.");
  else if (percent >= 90)
    strcat (buf, " has a few scratches.");
  else if (percent >= 75)
    strcat (buf, " has some small wounds and bruises.");
  else if (percent >= 50)
    strcat (buf, " has quite a few wounds.");
  else if (percent >= 30)
    strcat (buf, " has some big nasty wounds and scratches.");
  else if (percent >= 15)
    strcat (buf, " looks pretty hurt.");
  else if (percent >= 0)
    strcat (buf, " is in awful condition.");
  else
    strcat (buf, " is bleeding to death.");

  z = skipcol (buf);
  buf[z] = toupper (buf[z]);
  chprintln (ch, buf);

  found = false;
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
      if ((obj = get_eq_char (victim, (wloc_t) iWear)) != NULL
	  && can_see_obj (ch, obj))
	{
	  if (!found)
	    {
	      chprintln (ch, NULL);
	      act ("$N is using:", ch, NULL, victim, TO_CHAR);
	      found = true;
	    }
	  chprint (ch, where_name[iWear]);
	  chprintln (ch, format_obj_to_char (obj, ch, true));
	}
    }

  if (victim != ch && !IsNPC (ch) &&
      number_percent () < get_skill (ch, gsn_peek))
    {
      chprintln (ch, NEWLINE "You peek at the inventory:");
      check_improve (ch, gsn_peek, true, 4);
      show_list_to_char (victim->carrying_first, ch, true, true);
    }

  return;
}

void
show_char_to_char (CharData * list, CharData * ch)
{
  CharData *rch;

  for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
      if (rch == ch)
	continue;

      if (get_trust (ch) < rch->invis_level)
	continue;

      if (can_see (ch, rch))
	{
	  show_char_to_char_0 (rch, ch);
	}
      else if (room_is_dark (ch->in_room) && IsAffected (rch, AFF_INFRARED))
	{
	  chprintln (ch, "You see glowing red eyes watching YOU!");
	}
    }

  return;
}

bool
check_blind (CharData * ch)
{

  if (!IsNPC (ch) && IsSet (ch->act, PLR_HOLYLIGHT))
    return true;

  if (IsAffected (ch, AFF_BLIND))
    {
      chprintln (ch, "You can't see a thing!");
      return false;
    }

  return true;
}

int
get_scr_cols (CharData * ch)
{
  unsigned int len;

  if (!ch)
    len = DEFAULT_SCR_WIDTH;
  else if (ch->columns <= 10)
    len = ScrWidth (ch->desc);
  else
    len = ch->columns;

  return len - 2;
}

int
get_scr_lines (CharData * ch)
{
  unsigned int len;

  if (!ch)
    len = DEFAULT_SCR_HEIGHT;
  else if (!ch->lines)
    len = ScrHeight (ch->desc);
  else
    len = ch->lines;

  return len - 2;
}

Do_Fun (do_screen)
{
  char arg1[MIL], arg2[MIL];
  unsigned int lines;
  int *plines;
  char *func;
  unsigned int def_lines;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (NullStr (arg1) || NullStr (arg2))
    {
      if (get_scr_lines (ch) < 0)
	chprintln (ch, "You do not page long messages.");
      else
	{
	  chprintlnf (ch, "You currently display %s per page.",
		      intstr (get_scr_lines (ch), "line"));
	}
      chprintlnf (ch,
		  NEWLINE "You currently display %s per line." NEWLINE,
		  intstr (get_scr_cols (ch), "column"));
      cmd_syntax (ch, NULL, n_fun, "lines|columns <#val|default>", NULL);
      chprintlnf (ch,
		  "        (type '%s lines none' to disable scrolling)",
		  n_fun);
      return;
    }
  if (!str_prefix (arg1, "lines"))
    {
      if (!str_cmp (arg2, "none") || atoi (arg2) < 0)
	{
	  ch->lines = -1;
	  chprintln
	    (ch,
	     "Scrolling disabled.  This may cause you to disconnect on long outputs.");
	  return;
	}
      plines = &ch->lines;
      func = "lines";
      def_lines = ScrHeight (ch->desc);
    }
  else if (!str_prefix (arg1, "columns"))
    {
      plines = &ch->columns;
      func = "columns";
      def_lines = ScrWidth (ch->desc);
    }
  else
    {
      do_screen (n_fun, ch, "");
      return;
    }

  if (!str_cmp (arg2, "default"))
    {
      *plines = def_lines;
      chprintlnf (ch, "You're screen now displays %d %s.", def_lines, func);
      return;
    }
  else if (!is_number (arg2))
    {
      chprintln (ch, "You must provide a number.");
      return;
    }

  lines = atoi (arg2);

  if (lines < 10 || lines > 250)
    {
      chprintln (ch, "You must provide a reasonable number.");
      return;
    }

  chprintlnf (ch, "Your screen now displays %d %s.", lines, func);
  if (ch->desc && IsSet (ch->desc->desc_flags, DESC_TELOPT_NAWS)
      && lines != def_lines)
    chprintlnf
      (ch, "The Mud has detected that you have %d screen %s.",
       def_lines, func);
  *plines = lines;
}


Do_Fun (do_socials)
{
  SocialData *iSocial;
  Column Cd;
  Buffer *b;

  b = new_buf ();
  set_cols (&Cd, ch, 6, COLS_BUF, b);

  for (iSocial = social_first; iSocial != NULL; iSocial = iSocial->next)
    {
      print_cols (&Cd, iSocial->name);
    }

  cols_nl (&Cd);
  print_cols (&Cd, "Use the '%s' command to display a social's text.",
	      cmd_name (do_sshow));
  sendpage (ch, buf_string (b));
  free_buf (b);
  return;
}

Do_Fun (do_sshow)
{
  char arg[MIL];
  CharData *victim;
  SocialData *soc;

#ifndef HAVE_STRREV
  Proto (char *strrev, (char *));
#endif

  argument = one_argument (argument, arg);

  if (!(soc = find_social (arg)))
    {
      chprintln (ch, "No such social. Type 'socials' for a list.");
      return;
    }

  victim = create_mobile (get_char_index (MOB_VNUM_DUMMY));

  if (victim)
    {
      replace_str (&victim->short_descr,
		   capitalize (strrev ((char *) ch->name)));
      char_to_room (victim, get_room_index (ROOM_VNUM_LIMBO));
    }

  chprintlnf (ch, "Social: %s", soc->name);
  if (!NullStr (soc->char_no_arg))
    act (soc->char_no_arg, ch, NULL, NULL, TO_CHAR);
  if (!NullStr (soc->others_no_arg))
    act (soc->others_no_arg, ch, NULL, NULL, TO_CHAR);

  if (victim)
    {
      if (!NullStr (soc->char_found))
	act (soc->char_found, ch, NULL, victim, TO_CHAR);
      if (!NullStr (soc->others_found))
	act (soc->others_found, ch, NULL, victim, TO_CHAR);
      if (!NullStr (soc->vict_found))
	act (soc->vict_found, ch, NULL, victim, TO_CHAR);
    }

  if (!NullStr (soc->char_auto))
    act (soc->char_auto, ch, NULL, ch, TO_CHAR);
  if (!NullStr (soc->others_auto))
    act (soc->others_auto, ch, NULL, ch, TO_CHAR);

  if (victim)
    extract_char (victim, true);
  return;
}



Do_Fun (do_motd)
{
  do_function (ch, &do_oldhelp, "motd");
}

Do_Fun (do_imotd)
{
  do_function (ch, &do_oldhelp, "imotd");
}

Do_Fun (do_rules)
{
  do_function (ch, &do_oldhelp, "rules");
}

Do_Fun (do_story)
{
  do_function (ch, &do_oldhelp, "story");
}



Do_Fun (do_autolist)
{
  Buffer *buf;


  if (IsNPC (ch))
    return;

  buf = new_buf ();
  bprintlnf (buf, " %-9s %-6s{w %s", "Command", "Status", "Description");
  bprintln (buf, draw_line (ch, NULL, 0));

  bprintlnf (buf, "{G%-11s %-6s{w %s{x", OnOff (IsSet (ch->act, PLR_AUTOMAP)),
	     cmd_name (do_automap), "Map in Room Descriptions");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTODAMAGE)),
	     cmd_name (do_autodamage), "Displays damage amounts in combat.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOASSIST)),
	     cmd_name (do_autoassist),
	     "Automatically assists group members.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOEXIT)), cmd_name (do_autoexit),
	     "Displays exits in room descriptions.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOGOLD)), cmd_name (do_autogold),
	     "Automatically loots gold from corpses.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOLOOT)), cmd_name (do_autoloot),
	     "Automatically loots objects from corpses.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x", OnOff (IsSet (ch->act, PLR_AUTOSAC)),
	     cmd_name (do_autosac), "Automatically sacrifices corpses.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOSPLIT)), cmd_name (do_autosplit),
	     "Automatically splits gold between group members.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_AUTOPROMPT)),
	     cmd_name (do_autoprompt), "Selectivly displays your prompt.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->comm, COMM_COMPACT)), cmd_name (do_compact),
	     "Compacts mud output.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->comm, COMM_PROMPT)), cmd_name (do_prompt),
	     "Displays prompt information.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->comm, COMM_GPROMPT)), cmd_name (do_gprompt),
	     "Displays group information in a prompt.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->comm, COMM_COMBINE)), cmd_name (do_combine),
	     "Combines duplicate objects in display.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     !OnOff (IsSet (ch->act, PLR_CANLOOT)), cmd_name (do_noloot),
	     "Sets players unable to loot your corpse.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_NOSUMMON)), cmd_name (do_nosummon),
	     "Sets you unable to be summoned.");
  bprintlnf (buf, "{G%-11s %-6s{w %s{x",
	     OnOff (IsSet (ch->act, PLR_NOFOLLOW)), cmd_name (do_nofollow),
	     "Stops others from following you.");

  bprintln (buf, draw_line (ch, NULL, 0));
  sendpage (ch, buf_string (buf));
  free_buf (buf);
}

Do_Fun (do_autoassist)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOASSIST,
	      "You now assist group members in combat.",
	      "You no longer assist group members in combat.");
}

Do_Fun (do_autodamage)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTODAMAGE,
	      "You now see damage amounts in combat.",
	      "You no longer see damage amounts in combat.");
}

Do_Fun (do_autoexit)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOEXIT, "Exits will now be displayed.",
	      "Exits will no longer be displayed.");
}

Do_Fun (do_autogold)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOGOLD,
	      "You now loot gold from corpses automatically.",
	      "You no longer loot gold from corpses automatically.");
}

Do_Fun (do_autoloot)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOLOOT,
	      "You now loot objects from corpses automatically.",
	      "You no longer loot objects from corpses automatically.");
}

Do_Fun (do_autosac)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOSAC,
	      "You now sacrifice corpses automatically.",
	      "You no longer automatically sacrifice corpses.");
}

Do_Fun (do_autosplit)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOSPLIT,
	      "You now split gold with group members.",
	      "You no longer split gold with group members.");
}

Do_Fun (do_brief)
{
  set_on_off (ch, &ch->comm, COMM_BRIEF,
	      "You no longer see room descriptions.",
	      "You now see room descriptions.");
}

Do_Fun (do_compact)
{
  set_on_off (ch, &ch->comm, COMM_COMPACT, "Compact mode set.",
	      "Compact mode removed.");
}

Do_Fun (do_show)
{
  set_on_off (ch, &ch->comm, COMM_SHOW_AFFECTS,
	      "Affects will now be shown in score.",
	      "Affects will no longer be shown in score.");
}

Do_Fun (do_autoprompt)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_AUTOPROMPT,
	      "Prompts will now always show.",
	      "Prompts are now selectivly shown.");
}

struct prompt_type
{
  char *name;
  char *prompt;
  int level;
}
prompt_table[] =
{
  {
  "default", DEFAULT_PROMPT, 0}
  ,
  {
  "leveler",
      DEFAULT_PROMPT
      "<{Y%qq %Qgq {W%ss/%gg {G%x/%Xxp{x> You: {R%b{x Enemy: {R%B{x", 0}
  ,
  {
  "immortal", IMMORTAL_PROMPT, LEVEL_IMMORTAL}
  ,
  {
  NULL, NULL, -1}
};

Do_Fun (do_prompt)
{
  char buf[MAX_STRING_LENGTH];
  int i;

  if (NullStr (argument))
    {
      set_on_off (ch, &ch->comm, COMM_PROMPT,
		  "You will now see prompts. (Type 'prompt list' for a list of default prompts)",
		  "You will no longer see prompts.");
      return;
    }

  if (!str_cmp (argument, "list"))
    {
      for (i = 0; prompt_table[i].name != NULL; i++)
	chprintlnf (ch, "%12s - %s ", prompt_table[i].name,
		    prompt_table[i].prompt);

      cmd_syntax (NULL, n_fun, "<name|str>", NULL);
      return;
    }

  for (i = 0; prompt_table[i].name != NULL; i++)
    if (!str_cmp (argument, prompt_table[i].name))
      break;

  if (prompt_table[i].name != NULL)
    strcpy (buf, prompt_table[i].prompt);
  else
    {
      strcpy (buf, argument);
      if (strlen (buf) > 160)
	buf[160] = '\0';
      if (str_suffix ("%c", buf))
	strcat (buf, " ");
    }

  replace_str (&ch->prompt, buf);
  chprintlnf (ch, "Prompt set to %s", ch->prompt);
  return;
}

Do_Fun (do_gprompt)
{
  char buf[MSL];

  if (IsNPC (ch))
    {
      chprintln (ch, "Players only.");
      return;
    }

  if (NullStr (argument))
    {
      set_on_off (ch, &ch->comm, COMM_GPROMPT,
		  "You will now see group prompts.",
		  "You will no longer see group prompts.");

      chprintlnf (ch,
		  "Type '%s help' for help on the group prompt %% codes.",
		  n_fun);
      return;
    }

  if (!str_cmp (argument, "help"))
    {
      chprintln (ch,
		 stringf (ch, 0, Center, NULL,
			  "GROUP PROMPT PERCENTAGE CODES"));
      chprintln (ch,
		 "  %g - begin group section" "  %G - end group section"
		 "  %h - lowest hitpoints % for group members in the room"
		 "  %m - lowest mana % for group members in the room"
		 "  %v - lowest move % for group members in the room"
		 "  %p - begin pet section" "  %P - end pet section"
		 "  %q - pet hitpoints %" "  %r - pet mana %"
		 "  %s - pet move %"
		 "  %N - number of group members in the current room"
		 "  %c - carriage return"
		 "  %C - carriage return only if there is preceeding text"
		 "  %x - number of charmies in the current room (excluding pet)");
      chprintln (ch,
		 "Anything between %p and %P is 'eaten' when you dont have a pet in the room.");
      chprintln (ch,
		 "Anything between %g and %G is 'eaten' when you dont have a group member in the room.");
      chprintln (ch,
		 "Group prompts only work on pets and other players - not charmies (except %x).");
      chprintln (ch, "The default group prompt is:");
      chprintln (ch,
		 "  '%g[grp {R%hhp {B%mm {M%vmv{x]%G%p[pet {r%qhp {b%rm {m%smv{x>%P%c'");
      chprintln (ch, draw_line (ch, NULL, 0));
      return;
    }


  strcpy (buf, argument);
  if (strlen (buf) > 160)
    {
      buf[160] = '\0';
    }
  if (str_suffix ("%c", buf))
    {
      strcat (buf, " ");
    }

  chprintlnf (ch, "Prompt changed from '%s' to '%s'", ch->gprompt, buf);
  replace_str (&ch->gprompt, buf);
  SetBit (ch->comm, COMM_GPROMPT);
  return;
}

Do_Fun (do_combine)
{
  set_on_off (ch, &ch->comm, COMM_COMBINE, "Combined inventory selected.",
	      "Long inventory selected.");
}

Do_Fun (do_noloot)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_CANLOOT,
	      "Other players can now loot your corpse.",
	      "Other players can no longer loot your corpse.");
}

Do_Fun (do_nofollow)
{
  if (IsNPC (ch))
    return;

  set_on_off (ch, &ch->act, PLR_NOFOLLOW,
	      "You no longer accept followers.",
	      "You accept followers once again.");

  if (IsSet (ch->act, PLR_NOFOLLOW))
    die_follower (ch);
}

Do_Fun (do_nosummon)
{
  if (IsNPC (ch))
    {
      set_on_off (ch, &ch->imm_flags, IMM_SUMMON,
		  "You are now immune to summoning.",
		  "You can be summoned once again.");
    }
  else
    {
      set_on_off (ch, &ch->act, PLR_NOSUMMON,
		  "You are now immune to summoning.",
		  "You can be summoned once again.");
    }
}

bool
is_last_run (CharData * ch)
{
  if (IsNPC (ch))
    return true;

  if (!ch->desc)
    return true;

  if (!ch->desc->run_buf)
    return true;

  if (NullStr (ch->desc->run_head))
    return true;

  if (tolower (ch->desc->run_head[0]) == 'o'
      && strlen (ch->desc->run_head) <= 2)
    return true;

  if (ch->desc->run_head[0] == '0' && strlen (ch->desc->run_head) <= 2)
    return true;

  return false;
}

Do_Fun (do_look)
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  ExitData *pexit;
  CharData *victim;
  ObjData *obj;
  const char *pdesc;
  int door;
  int number, count;

  if (ch->desc == NULL)
    return;

  if (ch->position < POS_SLEEPING)
    {
      chprintln (ch, "You can't see anything but stars!");
      return;
    }

  if (ch->position == POS_SLEEPING)
    {
      chprintln (ch, "You can't see anything, you're sleeping!");
      return;
    }

  if (!check_blind (ch))
    return;

  if (!IsNPC (ch) && !IsSet (ch->act, PLR_HOLYLIGHT) &&
      room_is_dark (ch->in_room))
    {
      chprintln (ch, "It is pitch black ... ");
      show_char_to_char (ch->in_room->person_first, ch);
      return;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  number = number_argument (arg1, arg3);
  count = 0;

  if (NullStr (arg1) || !str_cmp (arg1, "auto"))
    {

      chprint (ch, CTAG (_RTITLE) MXPTAG ("RName"));
      chprint (ch, ch->in_room->name);
      chprint (ch, MXPTAG ("/RName"));

      if ((IsImmortal (ch) &&
	   (IsNPC (ch) || IsSet (ch->act, PLR_HOLYLIGHT))) ||
	  IsBuilder (ch, ch->in_room->area))
	{
	  chprintf (ch, " {w[{WRoom {R%ld{w]", ch->in_room->vnum);
	}

      chprintln (ch, "{x");

      if (NullStr (arg1) || (!IsNPC (ch) && !IsSet (ch->comm, COMM_BRIEF)))
	{
	  if (is_last_run (ch))
	    {
	      chprint (ch, MXPTAG ("RDesc"));
	      if (!IsSet (ch->in_room->room_flags, ROOM_NOAUTOMAP)
		  && !IsNPC (ch) && IsSet (ch->act, PLR_AUTOMAP))
		{
		  sprintf (buf, "%s%s{x",
			   get_sector_color (ch->in_room->sector_type),
			   NullStr (ch->in_room->description) ?
			   "No room description!" : ch->in_room->description);
		  draw_map (ch, buf);
		}
	      else
		{
		  dwraplnf (ch->desc, "%s%s{x",
			    get_sector_color (ch->in_room->sector_type),
			    NullStr (ch->in_room->description) ?
			    "No room description!" : ch->
			    in_room->description);
		}
	      chprint (ch, MXPTAG ("/RDesc"));
	    }
	}

      if (!IsNPC (ch) && IsSet (ch->act, PLR_AUTOEXIT))
	{
	  chprintln (ch, NULL);
	  do_function (ch, &do_exits, "auto");
	}

      show_list_to_char (ch->in_room->content_first, ch, false, false);
      show_char_to_char (ch->in_room->person_first, ch);
      return;
    }

  if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in") || !str_cmp (arg1, "on"))
    {

      if (NullStr (arg2))
	{
	  chprintln (ch, "Look in what?");
	  return;
	}

      if ((obj = get_obj_here (ch, NULL, arg2)) == NULL)
	{
	  chprintln (ch, "You do not see that here.");
	  return;
	}

      switch (obj->item_type)
	{
	default:
	  chprintln (ch, "That is not a container.");
	  break;

	case ITEM_DRINK_CON:
	  if (obj->value[1] <= 0)
	    {
	      chprintln (ch, "It is empty.");
	      break;
	    }

	  chprintlnf (ch, "It's %sfilled with  a %s liquid.",
		      obj->value[1] <
		      obj->value[0] /
		      4 ? "less than half-" : obj->value[1] <
		      3 * obj->value[0] /
		      4 ? "about half-" : "more than half-",
		      liq_table[obj->value[2]].liq_color);

	  break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	  if (IsSet (obj->value[1], CONT_CLOSED))
	    {
	      chprintln (ch, "It is closed.");
	      break;
	    }

	  act ("$p holds:", ch, obj, NULL, TO_CHAR);
	  show_list_to_char (obj->content_first, ch, true, true);
	  break;
	}
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg1)) != NULL)
    {
      show_char_to_char_1 (victim, ch);
      return;
    }

  for (obj = ch->carrying_first; obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  pdesc = get_ed (arg3, obj->ed_first);
	  if (pdesc != NULL)
	    {
	      if (++count == number)
		{
		  chprint (ch, pdesc);
		  return;
		}
	      else
		continue;
	    }
	  pdesc = get_ed (arg3, obj->pIndexData->ed_first);
	  if (pdesc != NULL)
	    {
	      if (++count == number)
		{
		  chprint (ch, pdesc);
		  return;
		}
	      else
		continue;
	    }
	  if (is_name (arg3, obj->name))
	    if (++count == number)
	      {
		chprintln (ch, obj->description);
		return;
	      }
	}
    }

  for (obj = ch->in_room->content_first; obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  pdesc = get_ed (arg3, obj->ed_first);
	  if (pdesc != NULL)
	    if (++count == number)
	      {
		chprint (ch, pdesc);
		return;
	      }

	  pdesc = get_ed (arg3, obj->pIndexData->ed_first);
	  if (pdesc != NULL)
	    if (++count == number)
	      {
		chprint (ch, pdesc);
		return;
	      }

	  if (is_name (arg3, obj->name))
	    if (++count == number)
	      {
		chprintln (ch, obj->description);
		return;
	      }
	}
    }

  pdesc = get_ed (arg3, ch->in_room->ed_first);
  if (pdesc != NULL)
    {
      if (++count == number)
	{
	  chprint (ch, pdesc);
	  return;
	}
    }

  if (count > 0 && count != number)
    {
      if (count == 1)
	sprintf (buf, "You only see one %s here.", arg3);
      else
	sprintf (buf, "You only see %d of those here.", count);

      chprintln (ch, buf);
      return;
    }

  if ((door = get_direction (arg1)) == -1)
    {
      chprintln (ch, "You do not see that here.");
      return;
    }


  if ((pexit = ch->in_room->exit[door]) == NULL)
    {
      chprintln (ch, "Nothing special there.");
      return;
    }

  if (IsSet (pexit->exit_info, EX_DOORBELL))
    chprintln (ch, "You see a doorbell, perhaps you should ring it?");

  if (!NullStr (pexit->description))
    chprint (ch, pexit->description);
  else
    chprintln (ch, "Nothing special there.");

  if (!NullStr (pexit->keyword) && pexit->keyword[0] != ' ')
    {
      if (IsSet (pexit->exit_info, EX_CLOSED))
	{
	  act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
	}
      else if (IsSet (pexit->exit_info, EX_ISDOOR))
	{
	  act ("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
	}
    }

  return;
}


Do_Fun (do_read)
{
  do_function (ch, &do_look, argument);
}

Do_Fun (do_examine)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ObjData *obj;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Examine what?");
      return;
    }

  do_function (ch, &do_look, arg);

  if ((obj = get_obj_here (ch, NULL, arg)) != NULL)
    {
      switch (obj->item_type)
	{
	default:
	  break;

	case ITEM_JUKEBOX:
	  do_function (ch, &do_play, "list");
	  break;

	case ITEM_MONEY:
	  if (obj->value[0] == 0)
	    {
	      if (obj->value[1] == 0)
		chprintlnf (ch, "Odd...there's no coins in the pile.");
	      else if (obj->value[1] == 1)
		chprintlnf (ch, "Wow. One gold coin.");
	      else
		chprintlnf (ch,
			    "There are %ld gold coins in the pile.",
			    obj->value[1]);
	    }
	  else if (obj->value[1] == 0)
	    {
	      if (obj->value[0] == 1)
		chprintlnf (ch, "Wow. One silver coin.");
	      else
		chprintlnf (ch,
			    "There are %ld silver coins in the pile.",
			    obj->value[0]);
	    }
	  else
	    chprintlnf (ch,
			"There are %ld gold and %ld silver coins in the pile.",
			obj->value[1], obj->value[0]);
	  break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	  sprintf (buf, "in %s", argument);
	  do_function (ch, &do_look, buf);
	}
    }

  return;
}


Do_Fun (do_exits)
{
  char buf[MAX_STRING_LENGTH];
  ExitData *pexit;
  bool found;
  bool fAuto;
  int door;

  fAuto = !str_cmp (argument, "auto");

  if (!check_blind (ch))
    return;

  if (fAuto)
    sprintf (buf, "[Exits:" MXPTAG ("RExits"));
  else if (IsImmortal (ch))
    sprintf (buf, "Obvious exits from room %ld:" NEWLINE, ch->in_room->vnum);
  else
    sprintf (buf, "Obvious exits:" NEWLINE);

  found = false;
  for (door = 0; door < MAX_DIR; door++)
    {
      if ((pexit = ch->in_room->exit[door]) != NULL &&
	  pexit->u1.to_room != NULL &&
	  can_see_room (ch, pexit->u1.to_room) &&
	  !IsSet (pexit->exit_info, EX_CLOSED))
	{
	  found = true;
	  if (fAuto)
	    {
	      strcat (buf, " " MXPTAG ("Ex"));
	      strcat (buf, dir_name[door]);
	      strcat (buf, MXPTAG ("/Ex"));
	    }
	  else
	    {
	      sprintf (buf + strlen (buf), "%-5s - %s",
		       capitalize (dir_name[door]),
		       room_is_dark (pexit->u1.to_room) ?
		       "Too dark to tell" : pexit->u1.to_room->name);
	      if (IsImmortal (ch))
		sprintf (buf + strlen (buf),
			 " (room %ld)", pexit->u1.to_room->vnum);

	      strcat (buf, NEWLINE);
	    }
	}
    }

  if (!found)
    strcat (buf, fAuto ? " none" : "None." NEWLINE);

  if (fAuto)
    strcat (buf, MXPTAG ("/RExits") "]");

  chprintln (ch, buf);
  return;
}

Do_Fun (do_worth)
{

  if (IsNPC (ch))
    {
      chprintlnf (ch, "You have %ld gold and %ld silver.", ch->gold,
		  ch->silver);
      return;
    }

  chprintlnf (ch,
	      "You have %ld gold, %ld silver, and %d experience (%d exp to level).",
	      ch->gold, ch->silver, ch->exp,
	      (ch->level + 1) * exp_per_level (ch,
					       ch->pcdata->points) - ch->exp);

  chprintlnf (ch, "You have earned %s and %s.",
	      intstr (ch->pcdata->quest.points, "questpoint"),
	      intstr (ch->pcdata->trivia, "trivia point"));

  return;
}


char *
get_char_align (CharData * ch)
{
  if (ch->alignment > 900)
    return "Angelic";
  else if (ch->alignment > 700)
    return "Saintly";
  else if (ch->alignment > 350)
    return "Good";
  else if (ch->alignment > 100)
    return "Kind";
  else if (ch->alignment > -100)
    return "Neutral";
  else if (ch->alignment > -350)
    return "Mean";
  else if (ch->alignment > -700)
    return "Evil";
  else if (ch->alignment > -900)
    return "Demonic";
  else
    return "Satanic";
}


long
get_exp_level (CharData * ch)
{
  if (!ch)
    return 1;

  return (IsNPC (ch) || ch->level >= LEVEL_IMMORTAL
	  || ch->level >= calc_max_level (ch)) ? 1
    : (ch->level + 1) * exp_per_level (ch, ch->pcdata->points) - ch->exp;
}


const char *
get_stance_name (CharData * ch)
{
  int stance = GetStance (ch, STANCE_CURRENT);

  if (!ValidStance (stance))
    stance = GetStance (ch, STANCE_AUTODROP);

  if (ValidStance (stance))
    return stance_table[stance].name;
  else
    return "None";
}

int
get_stance_skill (CharData * ch)
{
  int stance = GetStance (ch, STANCE_CURRENT);

  if (!ValidStance (stance))
    stance = GetStance (ch, STANCE_AUTODROP);

  if (ValidStance (stance))
    return GetStance (ch, stance);
  else
    return 0;
}


char *
condition_name (CharData * ch, int iCond)
{
  int condition = ch->pcdata->condition[iCond];

  switch (iCond)
    {
    case COND_HUNGER:
    case COND_FULL:
      if (condition < 0)
	return "Immune";
      else if (condition < 10)
	return "Faint ";
      else if (condition < 20)
	return "Hungry";
      else
	return " Full ";

    case COND_THIRST:
      if (condition < 0)
	return "Immune";
      else if (condition < 10)
	return "Faint ";
      else if (condition < 20)
	return "Thrsty";
      else
	return " Full ";

    case COND_DRUNK:
      if (condition > 20)
	return "Pissed";
      else if (condition > 10)
	return "Drunk ";
      else
	return "Sober ";
    default:
      bug ("Bad condition");
      return " ????? ";
    }
}


char *
make_percent_bar (int val, int max, int length)
{
  int i, j = 0;
  static char out[MSL];
  char colors[] = { 'r', 'R', 'y', 'Y', 'g', 'G', 'W' };
  size_t cp = 0, cm = sizeof (colors) / sizeof (colors[0]) - 1;
  int mod = max / length;
  int count = 0;

  for (i = 0; i < length; i++)
    {
      if (i % cm == 0)
	{
	  out[j++] = COLORCODE;
	  out[j++] = colors[cp++];
	  if (cp > sizeof (colors) / sizeof (colors[0]) - 1)
	    cp = 0;
	}
      if (val > count)
	out[j++] = '|';
      else
	out[j++] = ' ';
      count += mod;
    }
  out[j] = '\0';
  return out;
}

void
dlm_score (CharData * ch)
{
  Buffer *buf;

  buf = new_buf ();

  bprintln (buf, CTAG (_SCOREB)
	    "+-------------------------------------------------------------------------+");


  bprintlnf (buf, "|" CTAG (_SCORE1) "%s" CTAG (_SCOREB) "|",
	     str_align (73, Center,
			FORMATF (CTAG (_SCORE3) "%s" CTAG (_SCORE1)
				 "%s", ch->name, ch->pcdata->title)));

  if (is_clan (ch))
    {
      bprintlnf (buf, "|%s" CTAG (_SCOREB) "|",
		 str_align (73, Center,
			    FORMATF (CTAG (_SCORE1) "Clan : "
				     CTAG (_SCORE3) "%s      " CTAG (_SCORE1)
				     "Rank  : " CTAG (_SCORE3) "%s",
				     ch->pcdata->clan->name,
				     ch->pcdata->clan->rank[ch->
							    rank].rankname)));
    }

  if (IsRemort (ch))
    {
      bprintlnf (buf, "|%s" CTAG (_SCOREB) "|",
		 str_align (73, Center,
			    FORMATF (CTAG (_SCORE1) "Multiclasses : "
				     CTAG (_SCORE3) "%s", class_short (ch))));
    }
  bprintln (buf, CTAG (_SCOREB)
	    "+------------------------+---------------------+--------------------------+");
  bprintlnf (buf, CTAG (_SCOREB) "|" CTAG (_SCORE1)
	     " Strength     : [%s%-2d/%2d" CTAG (_SCORE1) "]"
	     CTAG (_SCOREB) " |" CTAG (_SCORE1) " Race   :"
	     CTAG (_SCORE3) "%11s" CTAG (_SCOREB) " |" CTAG (_SCORE1)
	     " Practices    : [" CTAG (_SCORE3) "%6d " CTAG (_SCORE1) "]"
	     CTAG (_SCOREB) " |", CTAG (_SCORE3), ch->perm_stat[STAT_STR],
	     get_curr_stat (ch, STAT_STR), ch->race->name, ch->practice);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Intelligence : ["
	     CTAG (_SCORE3) "%-2d/%2d" CTAG (_SCORE1) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE1) " Class  : " CTAG (_SCORE3) "%10s" CTAG (_SCOREB)
	     " |" CTAG (_SCORE1) " Trains       : [" CTAG (_SCORE3) "%6d "
	     CTAG (_SCORE1) "]" CTAG (_SCOREB) " |", ch->perm_stat[STAT_INT],
	     get_curr_stat (ch, STAT_INT), class_who (ch), ch->train);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Wisdom       : ["
	     CTAG (_SCORE3) "%-2d/%2d" CTAG (_SCORE1) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE1) " Sex    : " CTAG (_SCORE3) "%10s" CTAG (_SCOREB)
	     " |" CTAG (_SCORE1) " Trivia       : [" CTAG (_SCORE3) "%6d "
	     CTAG (_SCORE1) "]" CTAG (_SCOREB) " |", ch->perm_stat[STAT_WIS],
	     get_curr_stat (ch, STAT_WIS),
	     ch->sex == SEX_NEUTRAL ? "Sexless" : ch->sex ==
	     SEX_MALE ? "Male" : "Female", CTAG (_SCORE3),
	     ch->pcdata->trivia);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Dexterity    : ["
	     CTAG (_SCORE3) "%-2d/%2d" CTAG (_SCORE1) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE1) " Thac0  : " CTAG (_SCORE3) "%10d" CTAG (_SCOREB)
	     " |" CTAG (_SCORE1) " Quest Points : [" CTAG (_SCORE3) "%6d "
	     CTAG (_SCORE1) "]" CTAG (_SCOREB) " |", ch->perm_stat[STAT_DEX],
	     get_curr_stat (ch, STAT_DEX), interpolate (ch->level,
							get_thac00 (ch),
							get_thac32 (ch)),
	     ch->pcdata->quest.points);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Constitution : ["
	     CTAG (_SCORE3) "%-2d/%2d" CTAG (_SCORE1) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE1) " Level  : " CTAG (_SCORE3) "%10d" CTAG (_SCOREB)
	     " |" CTAG (_SCORE1) " Quest Time   : [" CTAG (_SCORE3) "%6d "
	     CTAG (_SCORE1) "]" CTAG (_SCOREB) " |", ch->perm_stat[STAT_CON],
	     get_curr_stat (ch, STAT_CON), ch->level, CTAG (_SCORE3),
	     ch->pcdata->quest.time);

  bprintln (buf, CTAG (_SCOREB)
	    "+------------------------+---------------------+--------------------------+");
  bprintlnf (buf, CTAG (_SCOREB) "|" CTAG (_SCORE2) " Hit    : [{R%5ld"
	     CTAG (_SCORE3) "/{R%5ld" CTAG (_SCORE2) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE2) " Hitroll  : [" CTAG (_SCORE3) "%5d "
	     CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Weight : " CTAG (_SCORE3) "%4d of %-7d" CTAG (_SCOREB) " |",
	     ch->hit, ch->max_hit, GetHitroll (ch),
	     get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE2) " Mana   : [{M%5ld"
	     CTAG (_SCORE3) "/{M%5ld" CTAG (_SCORE2) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE2) " Damroll  : [" CTAG (_SCORE3) "%5d "
	     CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Items  : " CTAG (_SCORE3) "%4d of %-7d" CTAG (_SCOREB) " |",
	     ch->mana, ch->max_mana, GetDamroll (ch), ch->carry_number,
	     can_carry_n (ch));
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE2) " Moves  : [{B%5ld"
	     CTAG (_SCORE3) "/{B%5ld" CTAG (_SCORE2) "]" CTAG (_SCOREB) " |"
	     CTAG (_SCORE2) " Saves    : [" CTAG (_SCORE3) "%5d "
	     CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Align  : " CTAG (_SCORE3) "%5d (%7s)" CTAG (_SCOREB) " |",
	     ch->move, ch->max_move, ch->saving_throw, ch->alignment,
	     get_char_align (ch));
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE2) " Exp    : [" CTAG (_SCORE3)
	     "%10ld " CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Age      : [" CTAG (_SCORE3) "%5d " CTAG (_SCORE2) "]"
	     CTAG (_SCOREB) " |" CTAG (_SCORE2) " Deity  : " CTAG (_SCORE3)
	     "%12s   " CTAG (_SCOREB) " |", ch->exp, get_age (ch),
	     ch->deity->name);
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE2) " To Lvl : [" CTAG (_SCORE3)
	     "%10ld " CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Hours    : [" CTAG (_SCORE3) "%5d " CTAG (_SCORE2) "]"
	     CTAG (_SCOREB) " |" CTAG (_SCORE2) " Stance : " CTAG (_SCORE3)
	     "%-9s (%3d)" CTAG (_SCOREB) " |", get_exp_level (ch),
	     ((ch->pcdata->played + (int) (current_time - ch->logon)) / HOUR),
	     get_stance_name (ch), get_stance_skill (ch));

  bprintf (buf,
	   CTAG (_SCOREB) "|" CTAG (_SCORE2) " Silver : [" CTAG (_SCORE3)
	   "%10ld " CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	   " Trust    : [" CTAG (_SCORE3) "%5d " CTAG (_SCORE2) "]"
	   CTAG (_SCOREB), ch->silver, get_trust (ch));
  bprintlnf (buf,
	     " |" CTAG (_SCORE2) " Hunger :  " CTAG (_SCORE3) " %3d  (%-6s)"
	     CTAG (_SCOREB) " |", ch->pcdata->condition[COND_HUNGER],
	     condition_name (ch, COND_HUNGER));

  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE2) " Gold   : [" CTAG (_SCORE3)
	     "%10ld " CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	     " Pkills   : [" CTAG (_SCORE3) "%5ld " CTAG (_SCORE2) "]"
	     CTAG (_SCOREB) " |" CTAG (_SCORE2) " Thirst :  " CTAG (_SCORE3)
	     " %3d  (%-6s)" CTAG (_SCOREB) " |", ch->gold, GetStat (ch,
								    PK_KILLS),
	     ch->pcdata->condition[COND_THIRST], condition_name (ch,
								 COND_THIRST));
  bprintf (buf,
	   CTAG (_SCOREB) "|" CTAG (_SCORE2) "Position: [" CTAG (_SCORE3)
	   "%10s " CTAG (_SCORE2) "]" CTAG (_SCOREB) " |" CTAG (_SCORE2)
	   " Pdeaths  : [" CTAG (_SCORE3) "%5ld " CTAG (_SCORE2) "]"
	   CTAG (_SCOREB), flag_string (position_flags, ch->position),
	   GetStat (ch, PK_DEATHS));

  bprintlnf (buf, " |" CTAG (_SCORE2) " Drunk  :  %s %3d  (%-6s)"
	     CTAG (_SCOREB) " |", CTAG (_SCORE3),
	     ch->pcdata->condition[COND_DRUNK], condition_name (ch,
								COND_DRUNK));

  bprintln (buf, CTAG (_SCOREB)
	    "+------------------------+---------------------+--------------------------+");

  bprintlnf (buf, CTAG (_SCOREB) "|" CTAG (_SCORE1) " Pierce :"
	     CTAG (_SCORE3) " %-4d " CTAG (_SCORE1) "[%-s" CTAG (_SCORE1)
	     "] " CTAG (_SCOREB) "|", GetArmor (ch, AC_PIERCE),
	     make_percent_bar (GetArmor (ch, AC_PIERCE) * -1, 1000, 55));
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Bash   :" CTAG (_SCORE3)
	     " %-4d " CTAG (_SCORE1) "[%-s" CTAG (_SCORE1) "] "
	     CTAG (_SCOREB) "|", GetArmor (ch, AC_BASH),
	     make_percent_bar (GetArmor (ch, AC_BASH) * -1, 1000, 55));
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Slash  :" CTAG (_SCORE3)
	     " %-4d " CTAG (_SCORE1) "[%-s" CTAG (_SCORE1) "] "
	     CTAG (_SCOREB) "|", GetArmor (ch, AC_SLASH),
	     make_percent_bar (GetArmor (ch, AC_SLASH) * -1, 1000, 55));
  bprintlnf (buf,
	     CTAG (_SCOREB) "|" CTAG (_SCORE1) " Exotic :" CTAG (_SCORE3)
	     " %-4d " CTAG (_SCORE1) "[%-s" CTAG (_SCORE1) "] "
	     CTAG (_SCOREB) "|", GetArmor (ch, AC_EXOTIC),
	     make_percent_bar (GetArmor (ch, AC_EXOTIC) * -1, 1000, 55));

  bprintln (buf, CTAG (_SCOREB)
	    "+-------------------------------------------------------------------------+");
  {
    double rcnt = roomcount (ch);
    double rooms = top_explored;

    bprintlnf (buf, CTAG (_SCOREB) "|%s" CTAG (_SCOREB) "|",
	       str_align (73, Center,
			  FORMATF (CTAG (_SCORE1) "Explored :"
				   CTAG (_SCORE3) " %.0f " CTAG (_SCORE1)
				   "of" CTAG (_SCORE3) " %d "
				   CTAG (_SCORE1) "rooms (" CTAG (_SCORE3)
				   "%5.2f%%" CTAG (_SCORE1)
				   " of the world)", rcnt,
				   top_explored, Percent (rcnt, rooms))));
  }

  if (ch->pcdata->gold_bank || ch->pcdata->shares)
    bprintlnf (buf, CTAG (_SCOREB) "|%s" CTAG (_SCOREB) "|",
	       str_align (73, Center,
			  FORMATF (CTAG (_SCORE1) "You have" CTAG (_SCORE3)
				   " %ld " CTAG (_SCORE1)
				   "gold in the bank and " CTAG (_SCORE3)
				   "%d " CTAG (_SCORE1) "shares ("
				   CTAG (_SCORE3) "%ld gold at %ld value"
				   CTAG (_SCORE1) ").", ch->pcdata->gold_bank,
				   ch->pcdata->shares,
				   (money_t) (ch->pcdata->shares *
					      mud_info.share_value),
				   ch->pcdata->shares,
				   mud_info.share_value)));

  if (IsImmortal (ch))
    {
      bprintlnf (buf, CTAG (_SCOREB) "|%s" CTAG (_SCOREB) "|",
		 str_align (73, Center,
			    FORMATF (CTAG (_SCORE1) "HolyLight : [ "
				     CTAG (_SCORE3) "%3s " CTAG (_SCORE1)
				     "]  Invis : [" CTAG (_SCORE3)
				     "%3d " CTAG (_SCORE1)
				     "]  Incog : [" CTAG (_SCORE3)
				     "%3d " CTAG (_SCORE1) "]  Security: ["
				     CTAG (_SCORE3) "%d" CTAG (_SCORE1) "]",
				     IsSet (ch->act,
					    PLR_HOLYLIGHT) ? "ON" :
				     "OFF", ch->invis_level,
				     ch->incog_level, ch->pcdata->security)));
    }

  bprintln (buf, CTAG (_SCOREB)
	    "+-------------------------------------------------------------------------+{x");

  sendpage (ch, buf_string (buf));
  free_buf (buf);
  return;
}

void
old_score (CharData * ch)
{
  int i;

  chprintlnf (ch,
	      CTAG (_SCORE1) "You are " CTAG (_SCORE2) "%s%s"
	      CTAG (_SCORE1) ", level " CTAG (_SCORE2) "%d"
	      CTAG (_SCORE1) ", " CTAG (_SCORE2) "%d" CTAG (_SCORE1)
	      " years old (%d hours).{x", ch->name,
	      IsNPC (ch) ? "" : ch->pcdata->title, ch->level,
	      get_age (ch),
	      (ch->pcdata->played + (int) (current_time - ch->logon)) / HOUR);

  if (get_trust (ch) != ch->level)
    {
      chprintlnf (ch,
		  CTAG (_SCORE1) "You are trusted at level "
		  CTAG (_SCORE2) "%d" CTAG (_SCORE1) ".{x", get_trust (ch));
    }

  chprintlnf (ch,
	      CTAG (_SCORE1) "Race: " CTAG (_SCORE2) "%s"
	      CTAG (_SCORE1) "  Sex: " CTAG (_SCORE2) "%s"
	      CTAG (_SCORE1) "  Class: " CTAG (_SCORE2) "%s{x",
	      ch->race->name,
	      ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
	      IsNPC (ch) ? "mobile" : class_long (ch));

  chprintlnf (ch, CTAG (_SCORE1) "You have %ld/%ld" CTAG (_SCORE1)
	      " hit, %ld/%ld" CTAG (_SCORE1) " mana, %ld/%ld" CTAG (_SCORE1)
	      " movement.{x", ch->hit, ch->max_hit, ch->mana,
	      ch->max_mana, ch->move, ch->max_move);

  chprintlnf (ch,
	      CTAG (_SCORE1) "You have " CTAG (_SCORE2) "%d"
	      CTAG (_SCORE1) " practices and " CTAG (_SCORE2) "%d"
	      CTAG (_SCORE1) " training sessions.{x", ch->practice,
	      ch->train);

  chprintlnf (ch,
	      CTAG (_SCORE1) "You are carrying " CTAG (_SCORE2) "%d/%d"
	      CTAG (_SCORE1) " items with weight " CTAG (_SCORE2) "%ld/%d"
	      CTAG (_SCORE1) " pounds.{x", ch->carry_number,
	      can_carry_n (ch), get_carry_weight (ch) / 10,
	      can_carry_w (ch) / 10);

  for (i = 0; i < STAT_MAX; i++)
    {
      chprintf (ch, CTAG (_SCORE1) "%.3s: " CTAG (_SCORE2) "%d"
		CTAG (_SCORE3) "(%d) ", capitalize (stat_types[i].name),
		ch->perm_stat[i], get_curr_stat (ch, i));
    }
  chprintln (ch, "{x");

  chprintlnf (ch,
	      CTAG (_SCORE1) "You have scored " CTAG (_SCORE2) "%d"
	      CTAG (_SCORE1) " exp, and have " CTAG (_SCORE2) "%ld"
	      CTAG (_SCORE1) " gold and " CTAG (_SCORE2) "%ld"
	      CTAG (_SCORE1) " silver coins.{x", ch->exp, ch->gold,
	      ch->silver);

  if (!IsNPC (ch))
    {
      chprintlnf (ch,
		  CTAG (_SCORE1) "You have " CTAG (_SCORE2) "%ld"
		  CTAG (_SCORE1) " gold in you bank account.{x",
		  ch->pcdata->gold_bank);

      chprintlnf (ch,
		  CTAG (_SCORE1) "You have earned " CTAG (_SCORE2) "%d"
		  CTAG (_SCORE1) " questpoints and " CTAG (_SCORE2) "%d"
		  CTAG (_SCORE1) " trivia points.{x",
		  ch->pcdata->quest.points, ch->pcdata->trivia);
    }
  chprintlnf (ch,
	      CTAG (_SCORE1) "You worship " CTAG (_SCORE2) "%s, %s"
	      CTAG (_SCORE1) ".{x",
	      ch->deity != NULL ? ch->deity->name : "Mota",
	      ch->deity != NULL ? ch->deity->desc : "the God of Thera");

  {
    double rcnt = roomcount (ch);
    double rooms = top_explored;

    chprintlnf (ch,
		CTAG (_SCORE1) "Explored : " CTAG (_SCORE2) "%.0f"
		CTAG (_SCORE1) " of " CTAG (_SCORE2) "%d" CTAG (_SCORE1)
		" rooms (" CTAG (_SCORE2) "%5.2f%%" CTAG (_SCORE1)
		" of the world){x", rcnt, top_explored, Percent (rcnt,
								 rooms));
  }


  if (!IsNPC (ch) && ch->level < calc_max_level (ch))
    {
      chprintlnf (ch,
		  CTAG (_SCORE1) "You need " CTAG (_SCORE2) "%d"
		  CTAG (_SCORE1) " exp to level.{x",
		  ((ch->level + 1) * exp_per_level (ch,
						    ch->pcdata->points) -
		   ch->exp));
    }

  chprintlnf (ch,
	      CTAG (_SCORE1) "Wimpy set to " CTAG (_SCORE2) "%d"
	      CTAG (_SCORE1) " hit points.{x", ch->wimpy);

  if (!IsNPC (ch) && ch->pcdata->condition[COND_DRUNK] > 10)
    chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "drunk.{x");
  if (!IsNPC (ch) && ch->pcdata->condition[COND_THIRST] == 0)
    chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "thirsty.{x");
  if (!IsNPC (ch) && ch->pcdata->condition[COND_HUNGER] == 0)
    chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "hungry.{x");

  switch (ch->position)
    {
    case POS_DEAD:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "DEAD!!{x");
      break;
    case POS_MORTAL:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3)
		 "mortally wounded.{x");
      break;
    case POS_INCAP:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3)
		 "incapacitated.{x");
      break;
    case POS_STUNNED:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "stunned.{x");
      break;
    case POS_SLEEPING:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "sleeping.{x");
      break;
    case POS_RESTING:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "resting.{x");
      break;
    case POS_SITTING:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "sitting.{x");
      break;
    case POS_STANDING:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "standing.{x");
      break;
    case POS_FIGHTING:
      chprintln (ch, CTAG (_SCORE1) "You are " CTAG (_SCORE3) "fighting.{x");
      break;
    default:
      break;
    }

  if (ValidStance (GetStance (ch, STANCE_AUTODROP)))
    chprintlnf (ch,
		CTAG (_SCORE1) "You auto-drop into the " CTAG (_SCORE3)
		"%s" CTAG (_SCORE1) " stance. (" CTAG (_SCORE3) "%d%%"
		CTAG (_SCORE1) "){x",
		stance_name (GetStance (ch, STANCE_AUTODROP)),
		GetStance (ch, GetStance (ch, STANCE_AUTODROP)));


  if (ch->level >= 25)
    {
      chprintlnf (ch,
		  CTAG (_SCORE1) "Armor: " CTAG (_SCORE2) "pierce: "
		  CTAG (_SCORE3) "%d  " CTAG (_SCORE2) "bash: "
		  CTAG (_SCORE3) "%d  " CTAG (_SCORE2) "slash: "
		  CTAG (_SCORE3) "%d  " CTAG (_SCORE2) "magic: "
		  CTAG (_SCORE3) "%d{x", GetArmor (ch, AC_PIERCE),
		  GetArmor (ch, AC_BASH), GetArmor (ch, AC_SLASH),
		  GetArmor (ch, AC_EXOTIC));
    }

  for (i = 0; i < 4; i++)
    {
      char *temp;

      switch (i)
	{
	case (AC_PIERCE):
	  temp = CTAG (_SCORE2) "piercing";
	  break;
	case (AC_BASH):
	  temp = CTAG (_SCORE2) "bashing";
	  break;
	case (AC_SLASH):
	  temp = CTAG (_SCORE2) "slashing";
	  break;
	case (AC_EXOTIC):
	  temp = CTAG (_SCORE2) "magic";
	  break;
	default:
	  temp = CTAG (_SCORE2) "error";
	  break;
	}

      chprint (ch, CTAG (_SCORE1) "You are ");

      if (GetArmor (ch, i) >= 101)
	chprintlnf (ch, "hopelessly vulnerable to %s.{x", temp);
      else if (GetArmor (ch, i) >= 80)
	chprintlnf (ch, "defenseless against %s.{x", temp);
      else if (GetArmor (ch, i) >= 60)
	chprintlnf (ch, "barely protected from %s.{x", temp);
      else if (GetArmor (ch, i) >= 40)
	chprintlnf (ch, "slightly armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= 20)
	chprintlnf (ch, "somewhat armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= 0)
	chprintlnf (ch, "armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= -20)
	chprintlnf (ch, "well-armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= -40)
	chprintlnf (ch, "very well-armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= -60)
	chprintlnf (ch, "heavily armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= -80)
	chprintlnf (ch, "superbly armored against %s.{x", temp);
      else if (GetArmor (ch, i) >= -100)
	chprintlnf (ch, "almost invulnerable to %s.{x", temp);
      else
	chprintlnf (ch, "divinely armored against %s.{x", temp);
    }


  if (IsImmortal (ch))
    {
      chprint (ch, CTAG (_SCORE1) "Holy Light: ");
      if (IsSet (ch->act, PLR_HOLYLIGHT))
	chprint (ch, CTAG (_SCORE2) "on");
      else
	chprint (ch, CTAG (_SCORE2) "off");

      if (ch->invis_level)
	{
	  chprintf (ch, CTAG (_SCORE1) "  Invisible: " CTAG (_SCORE2)
		    "level " CTAG (_SCORE3) "%d", ch->invis_level);
	}

      if (ch->incog_level)
	{
	  chprintf (ch, CTAG (_SCORE1) "  Incognito: " CTAG (_SCORE2)
		    "level " CTAG (_SCORE3) "%d", ch->incog_level);
	}
      chprintln (ch, "{x");
    }

  if (ch->level >= 15)
    {
      chprintlnf (ch,
		  CTAG (_SCORE1) "Hitroll: " CTAG (_SCORE2) "%d"
		  CTAG (_SCORE1) "  Damroll: " CTAG (_SCORE2) "%d.{x",
		  GetHitroll (ch), GetDamroll (ch));
    }

  if (ch->level >= 10)
    {
      chprintf (ch,
		CTAG (_SCORE1) "Alignment: " CTAG (_SCORE2) "%d.  ",
		ch->alignment);
    }

  chprint (ch, CTAG (_SCORE1) "You are ");
  if (ch->alignment > 900)
    chprintln (ch, CTAG (_SCORE2) "angelic.{x");
  else if (ch->alignment > 700)
    chprintln (ch, CTAG (_SCORE2) "saintly.{x");
  else if (ch->alignment > 350)
    chprintln (ch, CTAG (_SCORE2) "good.{x");
  else if (ch->alignment > 100)
    chprintln (ch, CTAG (_SCORE2) "kind.{x");
  else if (ch->alignment > -100)
    chprintln (ch, CTAG (_SCORE2) "neutral.{x");
  else if (ch->alignment > -350)
    chprintln (ch, CTAG (_SCORE2) "mean.{x");
  else if (ch->alignment > -700)
    chprintln (ch, CTAG (_SCORE2) "evil.{x");
  else if (ch->alignment > -900)
    chprintln (ch, CTAG (_SCORE2) "demonic.{x");
  else
    chprintln (ch, CTAG (_SCORE2) "satanic.{x");

  if (IsSet (ch->comm, COMM_SHOW_AFFECTS))
    do_function (ch, &do_affects, "");
}

Declare_Do_F (do_stat_char);

Do_Fun (do_score)
{
  if (IsNPC (ch))
    do_stat_char (n_fun, ch, ch->name);
  else if (!str_cmp (argument, "old") || !str_cmp (argument, "rom"))
    old_score (ch);
  else
    dlm_score (ch);
}

Do_Fun (do_affects)
{
  AffectData *paf, *paf_last = NULL;
  const char *buf4;
  char buf3[MSL];
  char buf2[MSL];
  bool found = false;
  flag_t filter;
  flag_t printme;
  Buffer *buffer;
  ObjData *obj;
  int iWear;

  buffer = new_buf ();

  if (ch->affect_first != NULL)
    {
      bprintln (buffer, "You are affected by the following spells:{x");

      for (paf = ch->affect_first; paf != NULL; paf = paf->next)
	{
	  if (paf_last != NULL && paf->type == paf_last->type)
	    {
	      if (get_trust (ch) >= 20)
		bprint (buffer, "                          ");
	      else
		continue;
	    }
	  else
	    bprintf (buffer, "{xSpell: {c%-19s{x",
		     (paf->type != -1
		      && !NullStr (skill_table[paf->type].name))
		     ? skill_table[paf->type].name : "unknown");

	  if (get_trust (ch) >= 20)
	    {
	      bprintf (buffer, ": {xmodifies %s by %d{x ",
		       flag_string (apply_flags, paf->location),
		       paf->modifier);
	      if (paf->duration == -1)
		bprint (buffer, "{xpermanently{x");
	      else
		bprintf (buffer, "{xfor %d hours{x", paf->duration);
	    }

	  bprintln (buffer, NULL);
	  paf_last = paf;
	}
      found = true;
      bprintln (buffer, NULL);
    }
  if (ch->race->aff != 0 && IsAffected (ch, ch->race->aff))
    {
      bprintln (buffer,
		"You are affected by the following racial abilities:{x");

      strcpy (buf3, flag_string (affect_flags, ch->race->aff));
      buf4 = buf3;
      buf4 = one_argument (buf4, buf2);
      while (buf2[0])
	{
	  bprintlnf (buffer, "{xSpell: {c%-19s{x", buf2);
	  buf4 = one_argument (buf4, buf2);
	}
      found = true;
      bprintln (buffer, NULL);
    }
  if (ch->affected_by != 0 && (ch->affected_by != ch->race->aff))
    {
      bool print = false;

      for (iWear = 0; iWear < MAX_WEAR; iWear++)
	{
	  if ((obj = get_eq_char (ch, (wloc_t) iWear)) != NULL)
	    {
	      for (paf = obj->affect_first; paf != NULL; paf = paf->next)
		{
		  if (!IsSet (ch->affected_by, paf->bitvector))
		    continue;

		  if (paf->where != TO_AFFECTS)
		    continue;

		  filter = paf->bitvector;
		  filter &= ch->affected_by;
		  printme = filter;
		  if (!print)
		    {
		      bprintln (buffer,
				"You are affected by the following equipment spells:{x");
		      print = true;
		    }

		  strcpy (buf3, flag_string (affect_flags, printme));
		  buf4 = buf3;
		  buf4 = one_argument (buf4, buf2);
		  while (buf2[0])
		    {
		      bprintlnf (buffer, "{xSpell: {c%-19s:{x %s", buf2,
				 obj->short_descr);
		      buf4 = one_argument (buf4, buf2);
		    }
		}
	      if (!obj->enchanted)
		{
		  for (paf = obj->pIndexData->affect_first; paf != NULL;
		       paf = paf->next)
		    {
		      if (!IsSet (ch->affected_by, paf->bitvector))
			continue;
		      if (paf->where != TO_AFFECTS)
			continue;
		      filter = paf->bitvector;
		      filter &= ch->affected_by;
		      printme = filter;
		      if (!print)
			{
			  bprintln (buffer,
				    "You are affected by the following equipment spells:{x");
			  print = true;
			}

		      strcpy (buf3, flag_string (affect_flags, printme));

		      buf4 = buf3;
		      buf4 = one_argument (buf4, buf2);
		      while (buf2[0])
			{
			  bprintlnf (buffer, "{xSpell: {c%-19s:{x %s",
				     buf2, obj->short_descr);
			  buf4 = one_argument (buf4, buf2);
			}
		    }
		}
	    }
	}
      found = true;
      if (print)
	bprintln (buffer, NULL);
    }
  if (!found)
    {
      bprintln (buffer, "You are not affected by any spells.{x");
    }

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
  return;
}

Do_Fun (do_time)
{
  getcurrenttime ();

  if (!NullStr (argument))
    {
      CharData *victim;

      if ((victim = get_char_world (ch, argument)) == NULL || IsNPC (victim))
	{
	  chprintln (ch, "There is no such player.");
	  return;
	}
      if (GetTzone (victim) == -1)
	{
	  act ("$N doesn't have a time zone set.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      act ("{W$N's local time is $t.{x", ch,
	   str_time (current_time, GetTzone (victim), NULL), victim, TO_CHAR);
      return;
    }

  chprintlnf (ch,
	      "It is %d o'clock %s, Day of %s, %d%s the Month of %s, year %d.",
	      (time_info.hour % (HOURS_IN_DAY / 2) ==
	       0) ? (HOURS_IN_DAY / 2) : time_info.hour % (HOURS_IN_DAY / 2),
	      time_info.hour >= (HOURS_IN_DAY / 2) ? "pm" : "am",
	      day_name[(time_info.day + 1) % DAYS_IN_WEEK],
	      ordinal_string (time_info.day + 1), month_name[time_info.month],
	      time_info.year);
  chprintlnf (ch,
	      "%s started up at %s, which was %s ago." NEWLINE
	      "The system time is %s.", mud_info.name, str_time (boot_time,
								 -1, NULL),
	      timestr (current_time - boot_time, false), str_time (-1, -1,
								   NULL));
  if (crs_info.timer > -1 && crs_info.status != CRS_NONE)
    {
      chprintln (ch, crs_sprintf (true, false));
    }
  else if (mud_info.last_copyover)
    chprintlnf (ch, "Last copyover was %s ago.",
		timestr (current_time - mud_info.last_copyover, false));

  if (mud_info.longest_uptime)
    chprintlnf (ch, "Longest uptime was %s.",
		timestr (mud_info.longest_uptime, false));

  if (!IsNPC (ch))
    {
      if (GetTzone (ch) != -1)
	chprintlnf (ch, "Your local time is %s",
		    str_time (-1, GetTzone (ch), NULL));
      else
	chprintln
	  (ch,
	   "Your local time is not set! Use the 'timezone' command to set your time zone.");


      chprintlnf (ch, "You connected at %s, which was %s ago.",
		  str_time (ch->logon, GetTzone (ch), NULL),
		  timestr (current_time - ch->logon, false));

      chprintlnf (ch, "You first created at %s, "
		  "which was %s ago.",
		  str_time (ch->id, GetTzone (ch), NULL),
		  timestr (current_time - ch->id, false));

      chprintlnf (ch, "You have played approximately %d.%02d hours.",
		  (ch->pcdata->played +
		   (int) (current_time - ch->logon)) / HOUR,
		  ((ch->pcdata->played + (int) (current_time - ch->logon)) /
		   36) % 100);

      chprintlnf
	(ch, "Which is %0.03f%% of the time since you created.",
	 Percent ((double)
		  (ch->pcdata->played + (int) (current_time - ch->logon)),
		  (double) (current_time - ch->id)));
    }

  return;
}

Do_Fun (do_timezone)
{
  int i;
  Column Cd;

  if (IsNPC (ch))
    return;

  set_cols (&Cd, ch, 2, COLS_CHAR, ch);

  if (NullStr (argument))
    {
      cols_header (&Cd, "%-11s %-6s (%s)", "Time", "Name",
		   "City/Zone Crosses");
      chprintln (ch, draw_line (ch, NULL, 0));
      for (i = 0; i < MAX_TZONE; i++)
	{
	  print_cols (&Cd, "%-11s %-6s (%s)",
		      str_time (current_time, i,
				"%H:%M:%S %p"),
		      tzone_table[i].name, tzone_table[i].zone);
	}
      cols_nl (&Cd);
      chprintln (ch, draw_line (ch, NULL, 0));
      return;
    }

  i = tzone_lookup (argument);

  if (i == -1)
    {
      chprintln
	(ch,
	 "That time zone does not exists. Make sure to use the exact name.");
      return;
    }

  ch->pcdata->timezone = i;
  chprintlnf (ch, "Your time zone is now %s %s (%s)", tzone_table[i].name,
	      tzone_table[i].zone, str_time (current_time, i, "%I:%M:%S %p"));
}

Do_Fun (do_weather)
{
  char *combo = "", *single = "";
  int temp, precip, wind;

  if (!IsOutside (ch))
    {
      chprintln (ch, "You can't see the sky from here.");
      return;
    }

  temp =
    (ch->in_room->area->weather.temp + 3 * mud_info.weath_unit -
     1) / mud_info.weath_unit;
  precip =
    (ch->in_room->area->weather.precip + 3 * mud_info.weath_unit -
     1) / mud_info.weath_unit;
  wind =
    (ch->in_room->area->weather.wind + 3 * mud_info.weath_unit -
     1) / mud_info.weath_unit;

  if (precip >= 3)
    {
      combo = preciptemp_msg[precip][temp];
      single = wind_msg[wind];
    }
  else
    {
      combo = windtemp_msg[wind][temp];
      single = precip_msg[precip];
    }

  chprintlnf (ch, "{B%s and %s.{x", combo, single);
  return;
}


Do_Fun (do_help)
{
  HelpData *pHelp;
  bool found = false;
  bool list = false;
  char argall[MIL], argone[MIL], argall2[MIL];
  char nohelp[MIL];
  Buffer *buffer;
  Buffer *related;
  Column c;
  int counter = 0, number, count = 0;

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

  buffer = new_buf ();
  related = new_buf ();

  number = number_argument (argall, argall2);
  strcpy (nohelp, argall2);
  set_cols (&c, ch, 3, COLS_BUF, buffer);

  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
      if (pHelp->level > get_trust (ch))
	continue;

      else if (!is_name (argall2, pHelp->keyword))
	continue;

      else if (list || argall2[1] == NUL)
	{
	  counter++;

	  print_cols (&c, "%3d) "
		      MXPTAG ("Help '%s'") "%s" MXPTAG ("/Help"), counter,
		      pHelp->keyword, pHelp->keyword);
	  list = true;
	  found = true;
	  continue;
	}
      else if (++count == number)
	{
	  bprintln (buffer, draw_line (ch, "{c-{C-", 0));
	  bprintlnf (buffer, "Help Keywords : %s", pHelp->keyword);
	  bprintln (buffer, draw_line (ch, "{c-{C-", 0));
	  bprint (buffer, help_text (pHelp->text));
	  bprintln (buffer, draw_line (ch, "{c-{C-", 0));
	  found = true;
	}
      else if (found)
	{
	  bprintf (related, MXPTAG ("Help '%s'") "%s" MXPTAG ("/Help") ", ",
		   pHelp->keyword, pHelp->keyword);
	}
    }
  if (list)
    {
      const char *text;

      cols_nl (&c);
      text = str_dup (buf_string (buffer));

      clear_buf (buffer);
      bprintlnf (buffer, "Help files that start with the letter '%c'.",
		 toupper (argall2[0]));
      bprintln (buffer, draw_line (ch, "{c-{C-", 0));
      bprint (buffer, text);
      bprintln (buffer, draw_line (ch, "{c-{C-", 0));
      bprintlnf (buffer, "%d total help files.", counter);
      free_string (text);
    }
  else if (!found)
    {
      bprintlnf (buffer,
		 "No help found for %s. Try using just the first letter.",
		 nohelp);
      new_wiznet (ch, NULL, 0, true, 0, "Missing Help: %s", nohelp);
    }
  else if (!NullStr (related->string))
    {
      counter = strlen (related->string);
      related->string[counter - 2] = '.';
      related->string[counter - 1] = '\0';
      bprintlnf (buffer, "See Also : %s", buf_string (related));
      bprintln (buffer, draw_line (ch, "{c-{C-", 0));
    }
  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
  free_buf (related);
}

Do_Fun (do_oldhelp)
{
  HelpData *pHelp;
  Buffer *output;
  bool found = false;
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

      if (level > get_trust (ch))
	continue;

      if (is_name (argall, pHelp->keyword))
	{

	  if (found)
	    bprintln (output, draw_line (ch, "{C-{c=", 0));
	  if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
	    {
	      bprintln (output, pHelp->keyword);
	    }



	  bprint (output, help_text (pHelp->text));
	  found = true;

	  if (ch->desc != NULL && ch->desc->connected != CON_PLAYING
	      && ch->desc->connected != CON_GEN_GROUPS)
	    break;
	}
    }

  if (!found)
    chprintln (ch, "No help on that word.");
  else
    sendpage (ch, buf_string (output));
  free_buf (output);
}


int
helps_in_category (help_t cat)
{
  HelpData *pHelp;
  int count = 0;

  for (pHelp = help_first; pHelp; pHelp = pHelp->next)
    if (pHelp->category == cat)
      count++;
  return count;
}

Do_Fun (do_index)
{
  HelpData *pHelp;
  char arg[MIL];
  char arg2[MIL];
  Buffer *output;
  int category;
  int i;
  Column c;

  argument = one_argument (argument, arg);
  one_argument (argument, arg2);

  if (NullStr (arg))
    {
      chprintln (ch, "Help Category not found. Valid args are:");
      for (i = 0; help_types[i].name != NULL; i++)
	chprintlnf (ch, "%2d) %s (%d helps)", i + 1, help_types[i].name,
		    helps_in_category ((help_t) help_types[i].bit));

      return;
    }

  if (is_number (arg))
    category = atoi (arg) - 1;
  else
    category = (help_t) flag_value (help_types, arg);

  if (category < HELP_UNKNOWN || category >= HELP_MAX)
    {
      chprintln (ch, "Unknown category.");
      return;
    }

  if (!NullStr (arg2))
    {
      int c = 0;

      if (!is_number (arg2) || (i = atoi (arg2)) < 1 || i >= top_help)
	{
	  cmd_syntax (ch, NULL, n_fun, "<category> <help number>", NULL);
	  return;
	}

      for (pHelp = help_first; pHelp; pHelp = pHelp->next)
	{
	  if (pHelp->level > get_trust (ch))
	    continue;
	  if (pHelp->category != (help_t) help_types[category].bit)
	    continue;

	  c++;

	  if (c == i)
	    break;
	}

      if (pHelp)
	{
	  output = new_buf ();
	  bprintlnf (output, "{C%s", draw_line (ch, NULL, 0));
	  bprintlnf (output, "{cHelp Keywords {x: %s", pHelp->keyword);
	  bprintlnf (output, "{cHelp Category {x: " MXPTAG ("Index %d") "%s"
		     MXPTAG ("/Index"), pHelp->category,
		     flag_string (help_types, pHelp->category));
	  bprintlnf (output, "{C%s", draw_line (ch, NULL, 0));

	  bprint (output, help_text (pHelp->text));
	  bprintlnf (output, "{C%s", draw_line (ch, NULL, 0));
	  sendpage (ch, buf_string (output));
	  free_buf (output);
	}
      else
	{
	  chprintlnf (ch, "That help not found in %s.",
		      flag_string (help_types, category));
	}
      return;
    }
  output = new_buf ();

  set_cols (&c, ch, 2, COLS_BUF, output);

  bprintlnf (output, "{C%s{x",
	     titlebar (ch, FORMATF ("[ {W%s{C ]",
				    flag_string (help_types, category))));
  i = 0;
  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
    {
      if (pHelp->level > get_trust (ch))
	continue;
      if (pHelp->category != (help_t) help_types[category].bit)
	continue;

      i++;

      print_cols (&c, "{c" MXPTAG ("Help '%s'") "%3d" MXPTAG ("/Help")
		  "{C){W %s {x", pHelp->keyword, i, str_width (35,
							       pHelp->
							       keyword));
    }
  if (!i)
    bprintlnf (output, "No helps found in %s.",
	       flag_string (help_types, category));
  else
    cols_nl (&c);

  bprintlnf (output, "{C%s", draw_line (ch, NULL, 0));

  sendpage (ch, buf_string (output));
  free_buf (output);
  return;
}

char *
format_clan (CharData * ch)
{
  static char buf[MSL];

  if (!is_clan (ch))
    return "";

  sprintf (buf, " %s", CharClan (ch)->who_name);
  return buf;
}

char *
format_who (CharData * ch, CharData * wch)
{
  char block[MIL];
  static char buf[MSL];

  if (!wch)
    return "ERROR";


  if (!NullStr (wch->pcdata->who_descr))
    sprintf (block, "[%s] ",
	     stringf (ch, 15, Center, NULL, wch->pcdata->who_descr));
  else
    sprintf (block,
	     "[" CTAG (_WSEX) "%c " CTAG (_WLEVEL) "%3.3s " CTAG (_WRACE)
	     "%4.4s " CTAG (_WCLASS) "%4.4s{x] ",
	     TrueSex (wch) == SEX_FEMALE ? 'F' : TrueSex (wch) ==
	     SEX_MALE ? 'M' : 'N', high_level_name (wch->level, false),
	     wch->race->name, class_who (wch));

  sprintf (buf, "%s%s%s%s%s%s%s%s%s%s{x"
	   MXPTAG ("Fwho '%s'") "%s" MXPTAG ("/Fwho") "%s%s" NEWLINE, block,
	   wch->incog_level >= LEVEL_HERO ? "{D(Incog) " : "",
	   wch->invis_level >= LEVEL_HERO ? "{W(Wizi) " : "",
	   IsSet (wch->comm, COMM_AFK) ? "{R[{YAFK{R] " : "",
	   IsQuester (wch) ? "{W[{CQ{W] " : "",
	   Gquester (wch) ? "{Y({RGQ{Y) " : "", wch->war ? "{R(War) " :
	   "", IsSet (wch->act, PLR_KILLER) ? "{r(KILLER) " : "",
	   IsSet (wch->act, PLR_THIEF) ? "{r(THIEF) " : "",
	   LinkDead (wch) ? "{c[{CLD{c] " : "", wch->name,
	   GetName (wch), IsNPC (wch) ? "" : wch->pcdata->title,
	   format_clan (wch));
  return (buf);
}


Do_Fun (do_whois)
{
  char arg[MAX_INPUT_LENGTH];
  Buffer *output;
  Descriptor *d;
  bool found = false;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "You must provide a name.");
      return;
    }

  output = new_buf ();

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      CharData *wch;

      if (d->connected != CON_PLAYING || !can_see (ch, d->character))
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (!can_see (ch, wch))
	continue;

      if (!str_prefix (arg, wch->name))
	{
	  found = true;

	  bprint (output, format_who (ch, wch));
	}
    }

  if (!found)
    {
      chprintln (ch, "No one of that name is playing.");
      return;
    }

  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_whowas)
{
  char arg[MIL];
  static CharData *victim;
  bool fOld;
  char buf[MIL];
  FileData *fp;

  one_argument (argument, arg);
  if (NullStr (arg))
    {
      chprintln (ch, "Whowas who?");
      return;
    }

  if ((victim = get_char_world (ch, arg)) != NULL && can_see (ch, victim))
    {
      if (!IsNPC (victim))
	{
	  act ("$N is on right now! Use Whois.", ch, NULL, victim, TO_CHAR);
	  return;
	}
    }

  victim = new_char ();
  victim->pcdata = new_pcdata ();
  pload_default (victim);
  fOld = false;
  sprintf (buf, pfilename (arg, PFILE_NORMAL));
  if ((fp = f_open (buf, "r")) != NULL)
    {
      AttemptJump = true;

      if (setjmp (jump_env) == 1)
	{
	  bugf ("[*WHOWAS*] pFile: '%s' error", capitalize (arg));
	  chprintlnf (ch, "An error occured reading %s's pfile.",
		      capitalize (arg));
	  f_close (fp);
	  free_char (victim);

	  return;
	}

      fOld = true;

      for (;;)
	{
	  char letter;
	  char *word;

	  letter = read_letter (fp);
	  if (letter == '*')
	    {
	      read_to_eol (fp);
	      continue;
	    }

	  if (letter != '#')
	    {
	      bug ("do_whowas: # not found.");
	      break;
	    }

	  word = read_word (fp);
	  if (!str_cmp (word, "PLAYER"))
	    read_char (victim, fp);
	  else if (!str_cmp (word, "PET"))
	    break;
	  else if (!str_cmp (word, "O"))
	    break;
	  else if (!str_cmp (word, "END"))
	    break;
	  else
	    break;
	}
      f_close (fp);
    }

  AttemptJump = false;

  if (!fOld)
    {
      chprintln (ch, "No player by that name exists.");
      free_char (victim);

      return;
    }

  if (IsNPC (victim) || victim->pcdata == NULL)
    {
      chprintln (ch, "Error loading pcdata.  Stop.");
      free_char (victim);

      return;
    }

  chprint (ch, format_who (ch, victim));
  free_char (victim);
  return;
}

struct s_charitem
{
  CharData *pch;
  long levelkey;
};


Do_Fun (do_who)
{
  char buf[MAX_STRING_LENGTH];
  Buffer *output;
  Descriptor *d;
  int iClass;
  RaceData *iRace = NULL;
  ClanData *iClan = NULL;
  int iLevelLower;
  int iLevelUpper;
  int nNumber;
  int nMatch, count = 0, immcount = 0;
  int ndesc, totalcount = 0, imminvis = 0;
  bool *rgfClass;
  bool fClassRestrict = false;
  bool fClanRestrict = false;
  bool fClan = false;
  bool fRaceRestrict = false;
  bool fImmortalOnly = false;
  struct s_charitem *charitems, tmp_charitem;
  bool searched = false;
  int j1, j2;
  CharData *wch;


  iLevelLower = 0;
  iLevelUpper = MAX_LEVEL;
  alloc_mem (rgfClass, bool, top_class);
  for (iClass = 0; iClass < top_class; iClass++)
    rgfClass[iClass] = false;


  nNumber = 0;
  for (;;)
    {
      char arg[MAX_STRING_LENGTH];

      argument = one_argument (argument, arg);
      if (NullStr (arg))
	break;
      else
	searched = true;

      if (is_number (arg))
	{
	  switch (++nNumber)
	    {
	    case 1:
	      iLevelLower = atoi (arg);
	      break;
	    case 2:
	      iLevelUpper = atoi (arg);
	      break;
	    default:
	      chprintln (ch, "Only two level numbers allowed.");
	      free_mem (rgfClass);
	      return;
	    }
	}
      else
	{


	  if (!str_prefix (arg, "immortals"))
	    {
	      fImmortalOnly = true;
	    }
	  else
	    {
	      iClass = class_lookup (arg);
	      if (iClass == -1 || iClass >= top_class)
		{
		  iRace = race_lookup (arg);

		  if (iRace == NULL || !iRace->pc_race)
		    {
		      if (!str_prefix (arg, "clan"))
			fClan = true;
		      else
			{
			  iClan = clan_lookup (arg);
			  if (iClan != NULL)
			    {
			      fClanRestrict = true;
			    }
			  else
			    {
			      chprintln
				(ch,
				 "That's not a valid race, class, or clan.");
			      free_mem (rgfClass);
			      return;
			    }
			}
		    }
		  else
		    {
		      fRaceRestrict = true;
		    }
		}
	      else
		{
		  fClassRestrict = true;
		  rgfClass[iClass] = true;
		}
	    }
	}
    }

  ndesc = 0;
  for (d = descriptor_first; d != NULL; d = d->next)
    ndesc++;

  alloc_mem (charitems, struct s_charitem, ndesc);

  if (ndesc > mud_info.stats.online)
    mud_info.stats.online = ndesc;


  nMatch = 0;
  buf[0] = '\0';
  output = new_buf ();
  bprintln (output, draw_line (ch, "{r-{R-", 0));
  for (d = descriptor_first; d != NULL; d = d->next)
    {

      if (d->connected != CON_PLAYING)
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (get_trust (wch) >= LEVEL_IMMORTAL && wch->level >= LEVEL_IMMORTAL)
	immcount++;
      else
	count++;

      if (!can_see (ch, wch))
	{
	  if (IsImmortal (wch))
	    imminvis++;
	  continue;
	}

      if (wch->level < iLevelLower || wch->level > iLevelUpper ||
	  (fImmortalOnly && wch->level < LEVEL_IMMORTAL) ||
	  (fClassRestrict && !rgfClass[prime_class (wch)])
	  || (fRaceRestrict && iRace != wch->race)
	  || (fClan && !is_clan (wch)) || (fClanRestrict
					   && CharClan (wch) != iClan))
	continue;

      charitems[nMatch].pch = wch;
      charitems[nMatch].levelkey = wch->level + 1;
      nMatch++;
    }

  totalcount = (count + immcount);

  for (j1 = 0; j1 < nMatch - 1; j1++)
    {
      for (j2 = j1 + 1; j2 < nMatch; j2++)
	{
	  if (charitems[j2].levelkey > charitems[j1].levelkey)
	    {
	      tmp_charitem = charitems[j1];
	      charitems[j1] = charitems[j2];
	      charitems[j2] = tmp_charitem;
	    }
	}
    }

  for (j1 = 0; j1 < nMatch; j1++)
    {
      wch = charitems[j1].pch;
      bprint (output, format_who (ch, wch));
      if (j1 == (immcount - imminvis - 1) && j1 != (nMatch - 1)
	  && IsImmortal (wch))
	{
	  bprintln (output, draw_line (ch, "{r-{R-", 0));
	}
    }

  free_mem (charitems);

  bprintln (output, draw_line (ch, "{r-{R-", 0));

  if (searched)
    {
      bprintlnf (output, "{WMatches found: {R%d{x", nMatch);
    }
  else if (nMatch < (totalcount - imminvis))
    {
      bprintlnf (output,
		 "{WPlayers found: {R%d{W  Most on ever: {R%d{W  Invisible: {R%d{x",
		 nMatch, mud_info.stats.online,
		 (totalcount - nMatch - imminvis));
    }
  else
    {
      bprintlnf (output, "{WPlayers found: {R%d{W  Most on ever: {R%d{x",
		 totalcount - imminvis, mud_info.stats.online);
    }

  if (!Gquester (ch) && gquest_info.minlevel < ch->level &&
      gquest_info.maxlevel > ch->level)
    bprintln (output, "{WThere is a Global Quest running you can join.{x");
  if (crs_info.timer > -1 && crs_info.status != CRS_NONE)
    {
      bprintlnf (output, "{W%s{x", crs_sprintf (true, false));
    }

  sendpage (ch, buf_string (output));
  free_buf (output);
  free_mem (rgfClass);
  return;
}

Do_Fun (do_count)
{
  int count;
  Descriptor *d;

  count = 0;

  for (d = descriptor_first; d != NULL; d = d->next)
    if (d->connected == CON_PLAYING && can_see (ch, d->character))
      count++;

  mud_info.stats.online = Max (count, mud_info.stats.online);

  if (mud_info.stats.online == count)
    chprintlnf (ch, "There %s %s on, the most ever.",
		count == 1 ? "is" : "are", intstr (count, "character"));
  else
    chprintlnf (ch, "There %s %s on, the most on ever was %d.",
		count == 1 ? "is" : "are", intstr (count, "character"),
		mud_info.stats.online);
}

Do_Fun (do_inventory)
{
  chprintlnf (ch,
	      "{YYou are carrying {W%d/%d{Y items with {W%ld/%d{Y weight:{x",
	      ch->carry_number, can_carry_n (ch),
	      get_carry_weight (ch) / 10, can_carry_w (ch) / 10);
  show_list_to_char (ch->carrying_first, ch, true, true);
  return;
}

Do_Fun (do_equipment)
{
  ObjData *obj;
  int iWear;

  chprintln (ch, "You are using:");
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
      chprint (ch, where_name[iWear]);
      if ((obj = get_eq_char (ch, (wloc_t) iWear)) == NULL)
	{
	  chprintln (ch, "nothing.");
	}
      else if (can_see_obj (ch, obj))
	{
	  chprintln (ch, format_obj_to_char (obj, ch, true));
	}
      else
	{
	  chprintln (ch, "something.");
	}
    }
  return;
}

Do_Fun (do_compare)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ObjData *obj1;
  ObjData *obj2;
  long value1;
  long value2;
  char *msg;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (NullStr (arg1))
    {
      chprintln (ch, "Compare what to what?");
      return;
    }

  if ((obj1 = get_obj_carry (ch, arg1, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  if (NullStr (arg2))
    {
      for (obj2 = ch->carrying_first; obj2 != NULL; obj2 = obj2->next_content)
	{
	  if (obj2->wear_loc != WEAR_NONE && can_see_obj (ch, obj2)
	      && obj1->item_type == obj2->item_type
	      && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
	    break;
	}

      if (obj2 == NULL)
	{
	  chprintln (ch, "You aren't wearing anything comparable.");
	  return;
	}
    }
  else if ((obj2 = get_obj_carry (ch, arg2, ch)) == NULL)
    {
      chprintln (ch, "You do not have that item.");
      return;
    }

  msg = NULL;
  value1 = 0;
  value2 = 0;

  if (obj1 == obj2)
    {
      msg = "You compare $p to itself.  It looks about the same.";
    }
  else if (obj1->item_type != obj2->item_type)
    {
      msg = "You can't compare $p and $P.";
    }
  else
    {
      switch (obj1->item_type)
	{
	default:
	  msg = "You can't compare $p and $P.";
	  break;

	case ITEM_ARMOR:
	  value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	  value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	  break;

	case ITEM_WEAPON:
	  if (obj1->pIndexData->new_format)
	    value1 = (1 + obj1->value[2]) * obj1->value[1];
	  else
	    value1 = obj1->value[1] + obj1->value[2];

	  if (obj2->pIndexData->new_format)
	    value2 = (1 + obj2->value[2]) * obj2->value[1];
	  else
	    value2 = obj2->value[1] + obj2->value[2];
	  break;
	}
    }

  if (msg == NULL)
    {
      if (value1 == value2)
	msg = "$p and $P look about the same.";
      else if (value1 > value2)
	msg = "$p looks better than $P.";
      else
	msg = "$p looks worse than $P.";
    }

  act (msg, ch, obj1, obj2, TO_CHAR);
  return;
}

Do_Fun (do_credits)
{
  do_function (ch, &do_oldhelp, "diku");
  do_function (ch, &do_oldhelp, "ROM");
  do_function (ch, &do_oldhelp, "1stMud");
  return;
}

Do_Fun (do_where)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  Descriptor *d;
  bool found;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintlnf (ch, "You are in zone  : %s", ch->in_room->area->name);
      if (!NullStr (ch->in_room->area->lvl_comment))
	chprintlnf (ch, "Recomended Levels: [%-7s]",
		    ch->in_room->area->lvl_comment);
      else
	chprintlnf (ch, "Recomended Levels: [%03d %03d]",
		    ch->in_room->area->min_level,
		    ch->in_room->area->max_level);

      chprintlnf (ch, "Author           : [%-7s]",
		  ch->in_room->area->credits);
      if (IsImmortal (ch))
	{
	  chprintlnf (ch, "Vnum Range       : %ld to %ld",
		      ch->in_room->area->min_vnum,
		      ch->in_room->area->max_vnum);
	}
      chprintln (ch, "Players near you:");
      found = false;
      for (d = descriptor_first; d; d = d->next)
	{
	  if (d->connected == CON_PLAYING &&
	      (victim = d->character) != NULL && !IsNPC (victim) &&
	      victim->in_room != NULL &&
	      !IsSet (victim->in_room->room_flags, ROOM_NOWHERE) &&
	      (is_room_owner (ch, victim->in_room) ||
	       !room_is_private (victim->in_room)) &&
	      victim->in_room->area == ch->in_room->area &&
	      can_see (ch, victim))
	    {
	      found = true;
	      chprintlnf (ch, "%-28s %s", victim->name,
			  victim->in_room->name);
	    }
	}
      if (!found)
	chprintln (ch, "None");
    }
  else
    {
      found = false;
      for (victim = char_first; victim != NULL; victim = victim->next)
	{
	  if (victim->in_room != NULL &&
	      victim->in_room->area == ch->in_room->area &&
	      !IsAffected (victim, AFF_HIDE) &&
	      !IsAffected (victim, AFF_SNEAK)
	      && can_see (ch, victim) && is_name (arg, victim->name))
	    {
	      found = true;
	      chprintlnf (ch, "%-28s %s", Pers (victim, ch),
			  victim->in_room->name);
	      break;
	    }
	}
      if (!found)
	act ("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
    }

  return;
}

Do_Fun (do_consider)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *victim;
  char *msg;
  int diff;

  one_argument (argument, arg);

  if (NullStr (arg))
    {
      chprintln (ch, "Consider killing whom?");
      return;
    }

  if ((victim = get_char_room (ch, NULL, arg)) == NULL)
    {
      chprintln (ch, "They're not here.");
      return;
    }

  if (is_safe (ch, victim))
    {
      chprintln (ch, "Don't even think about it.");
      return;
    }

  diff = victim->level - ch->level;

  if (diff <= -10)
    msg = "You can kill $N naked and weaponless.";
  else if (diff <= -5)
    msg = "$N is no match for you.";
  else if (diff <= -2)
    msg = "$N looks like an easy kill.";
  else if (diff <= 1)
    msg = "The perfect match!";
  else if (diff <= 4)
    msg = "$N says 'Do you feel lucky, punk?'.";
  else if (diff <= 9)
    msg = "$N laughs at you mercilessly.";
  else
    msg = "Death will thank you for your gift.";

  act (msg, ch, NULL, victim, TO_CHAR);
  return;
}

void
set_title (CharData * ch, const char *title)
{
  char buf[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    {
      bug ("Set_title: NPC.");
      return;
    }

  if (title[0] != '.' && title[0] != ',' && title[0] != '!' &&
      title[0] != '?')
    {
      buf[0] = ' ';
      strcpy (buf + 1, title);
    }
  else
    {
      strcpy (buf, title);
    }

  replace_str (&ch->pcdata->title, buf);
  return;
}

Do_Fun (do_title)
{
  char buf[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    return;

  if (NullStr (argument))
    {
      chprintln (ch, "Change your title to what?");
      return;
    }

  strcpy (buf, argument);

  if (strlen (buf) > 45)
    buf[45] = '\0';

  set_title (ch, buf);
  chprintln (ch, "Ok.");
}

Do_Fun (do_description)
{
  char buf[MAX_STRING_LENGTH];

  if (!NullStr (argument))
    {
      buf[0] = '\0';

      if (argument[0] == '-')
	{
	  int len;
	  bool found = false;

	  if (NullStr (ch->description))
	    {
	      chprintln (ch, "No lines left to remove.");
	      return;
	    }

	  strcpy (buf, ch->description);

	  for (len = strlen (buf); len > 0; len--)
	    {
	      if (buf[len] == '\r')
		{
		  if (!found)
		    {
		      if (len > 0)
			len--;
		      found = true;
		    }
		  else
		    {

		      buf[len + 1] = '\0';
		      replace_str (&ch->description, buf);
		      chprintln (ch, "Your description is:");
		      chprint (ch, ch->description ?
			       ch->description : "(None)." NEWLINE);
		      return;
		    }
		}
	    }
	  buf[0] = '\0';
	  replace_str (&ch->description, buf);
	  chprintln (ch, "Description cleared.");
	  return;
	}
      if (argument[0] == '+')
	{
	  if (ch->description != NULL)
	    strcat (buf, ch->description);
	  argument++;
	  while (isspace (*argument))
	    argument++;
	}

      if (strlen (buf) >= 1024)
	{
	  chprintln (ch, "Description too long.");
	  return;
	}

      strcat (buf, argument);
      strcat (buf, NEWLINE);
      replace_str (&ch->description, buf);
    }

  chprintln (ch, "Your description is:");
  chprint (ch, ch->description ? ch->description : "(None)." NEWLINE);
  return;
}

Do_Fun (do_report)
{
  char buf[MAX_INPUT_LENGTH];

  chprintlnf (ch,
	      "You say 'I have %ld/%ld hp %ld/%ld mana %ld/%ld mv %d xp.'",
	      ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
	      ch->max_move, ch->exp);

  sprintf (buf,
	   "$n says 'I have %ld/%ld hp %ld/%ld mana %ld/%ld mv %d xp.'",
	   ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
	   ch->max_move, ch->exp);

  act (buf, ch, NULL, NULL, TO_ROOM);

  return;
}

Do_Fun (do_practice)
{
  int sn;

  if (IsNPC (ch))
    return;

  if (NullStr (argument))
    {
      Column Cd;

      set_cols (&Cd, ch, 3, COLS_CHAR, ch);

      for (sn = 0; sn < top_skill; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  if (!can_use_skpell (ch, sn) || ch->pcdata->learned[sn] < 1)
	    continue;

	  print_cols (&Cd, "%-18s %3d%%", skill_table[sn].name,
		      ch->pcdata->learned[sn]);
	}

      cols_nl (&Cd);

      chprintlnf (ch, "You have %d practice sessions left.", ch->practice);
    }
  else
    {
      CharData *mob;
      int adept;

      if (!IsAwake (ch))
	{
	  chprintln (ch, "In your dreams, or what?");
	  return;
	}

      for (mob = ch->in_room->person_first; mob != NULL;
	   mob = mob->next_in_room)
	{
	  if (IsNPC (mob) && IsSet (mob->act, ACT_PRACTICE))
	    break;
	}

      if (mob == NULL)
	{
	  chprintln (ch, "You can't do that here.");
	  return;
	}

      if (ch->practice <= 0)
	{
	  chprintln (ch, "You have no practice sessions left.");
	  return;
	}

      if ((sn = find_spell (ch, argument)) < 0
	  || (!IsNPC (ch)
	      && (!can_use_skpell (ch, sn) || ch->pcdata->learned[sn] < 1
		  || skill_rating (ch, sn) == 0)))
	{
	  chprintln (ch, "You can't practice that.");
	  return;
	}

      adept = IsNPC (ch) ? 100 : class_table[prime_class (ch)].skill_adept;

      if (ch->pcdata->learned[sn] >= adept)
	{
	  chprintlnf (ch, "You are already learned at %s.",
		      skill_table[sn].name);
	}
      else
	{
	  ch->practice--;
	  ch->pcdata->learned[sn] +=
	    int_app[get_curr_stat (ch, STAT_INT)].learn /
	    skill_rating (ch, sn);
	  if (ch->pcdata->learned[sn] < adept)
	    {
	      act ("You practice $T.", ch, NULL,
		   skill_table[sn].name, TO_CHAR);
	      act ("$n practices $T.", ch, NULL,
		   skill_table[sn].name, TO_ROOM);
	    }
	  else
	    {
	      ch->pcdata->learned[sn] = adept;
	      act ("You are now learned at $T.", ch, NULL,
		   skill_table[sn].name, TO_CHAR);
	      act ("$n is now learned at $T.", ch, NULL,
		   skill_table[sn].name, TO_ROOM);
	    }
	}
    }
  return;
}


Do_Fun (do_wimpy)
{
  char arg[MAX_INPUT_LENGTH];
  int wimpy;

  one_argument (argument, arg);

  if (NullStr (arg))
    wimpy = ch->max_hit / 5;
  else
    wimpy = atoi (arg);

  if (wimpy < 0)
    {
      chprintln (ch, "Your courage exceeds your wisdom.");
      return;
    }

  if (wimpy > ch->max_hit / 2)
    {
      chprintln (ch, "Such cowardice ill becomes you.");
      return;
    }

  ch->wimpy = wimpy;
  chprintlnf (ch, "Wimpy set to %d hit points.", wimpy);
  return;
}

Do_Fun (do_password)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;

  if (IsNPC (ch))
    return;


  pArg = arg1;
  while (isspace (*argument))
    argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;

  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *pArg++ = *argument++;
    }
  *pArg = '\0';

  pArg = arg2;
  while (isspace (*argument))
    argument++;

  cEnd = ' ';
  if (*argument == '\'' || *argument == '"')
    cEnd = *argument++;

  while (*argument != '\0')
    {
      if (*argument == cEnd)
	{
	  argument++;
	  break;
	}
      *pArg++ = *argument++;
    }
  *pArg = '\0';

  if (NullStr (arg1) || NullStr (arg2))
    {
      cmd_syntax (ch, NULL, n_fun, "<old> <new>", NULL);
      return;
    }

  if (str_casecmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      WaitState (ch, 40);
      chprintln (ch, "Wrong password.  Wait 10 seconds.");
      return;
    }

  if (strlen (arg2) < 5)
    {
      chprintln (ch, "New password must be at least five characters long.");
      return;
    }


  pwdnew = crypt (arg2, ch->name);
  for (p = pwdnew; *p != '\0'; p++)
    {
      if (*p == '~')
	{
	  chprintln (ch, "New password not acceptable, try again.");
	  return;
	}
    }

  replace_str (&ch->pcdata->pwd, pwdnew);
  save_char_obj (ch);
  chprintln (ch, "Ok.");
  return;
}

void
print_on_off (Column * C, bool is_set, const char *cmd, const char *desc)
{
  if (!NullStr (desc))
    {
      col_print (C,
		 FORMATF (" {G%-11s %-6s{w %s{x" NEWLINE, cmd, OnOff (is_set),
			  desc));
    }
  else
    {
      print_cols (C, " {G%-11s %-6s{x ", cmd, OnOff (is_set));
    }
  return;
}

void
print_all_on_off (CharData * ch, FlagTable * table, flag_t bits)
{
  int i;
  Buffer *b;
  Column c;

  b = new_buf ();
  set_cols (&c, ch, 4, COLS_BUF, b);

  for (i = 0; table[i].name != NULL; i++)
    {
      print_on_off (&c, IsSet (bits, table[i].bit), table[i].name, NULL);
    }
  cols_nl (&c);
  sendpage (ch, buf_string (b));
  free_buf (b);
}

void
set_on_off (CharData * ch, flag_t * flags, flag_t flag, const char *on,
	    const char *off)
{
  if (IsSet (*flags, flag))
    {
      RemBit (*flags, flag);
      chprintln (ch, off);
    }
  else
    {
      SetBit (*flags, flag);
      chprintln (ch, on);
    }
}

Do_Fun (do_clear)
{
  clear_screen (ch);
}


Do_Fun (do_version)
{
  chprintlnf (ch, "{n is running " MUDSTRING ".");
  chprintln (ch, "Compiled at " __DATE__ " at " __TIME__ ".");
}


int
compare_mdeaths (const void *v1, const void *v2)
{
  return (*(CharIndex * *)v2)->deaths - (*(CharIndex * *)v1)->deaths;
}

Do_Fun (do_mobdeaths)
{
  Buffer *buffer;
  CharIndex *pMobIndex;
  CharIndex **mobs;
  int iHash, count, i;

  count = 0;
  i = 0;

  if (IsImmortal (ch) && !NullStr (argument))
    {
      char arg[MIL];

      argument = one_argument (argument, arg);

      if (!str_cmp (arg, "reset"))
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "reset <vnum>", "reset all", NULL);
	    }

	  else if (is_number (argument))
	    {
	      if ((pMobIndex = get_char_index (atov (argument))) != NULL)
		{
		  if (pMobIndex->deaths > 0)
		    TouchArea (pMobIndex->area);
		  pMobIndex->deaths = 0;
		  chprintlnf (ch, "Deaths set to 0 on mob vnum %ld.",
			      pMobIndex->vnum);
		}
	      else
		chprintln (ch, "No such mob.");
	    }
	  else if (!str_cmp (argument, "all"))
	    {
	      for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		{
		  for (pMobIndex = char_index_hash[iHash];
		       pMobIndex != NULL; pMobIndex = pMobIndex->next)
		    {
		      if (pMobIndex->area->deaths > 0)
			TouchArea (pMobIndex->area);
		      pMobIndex->deaths = 0;
		    }
		}
	      chprintln (ch, "All mob deaths set to 0.");
	    }
	  return;
	}
    }

  alloc_mem (mobs, CharIndex *, top_char_index);

  if (mobs == NULL)
    {
      log_error ("do_mobdeaths");
      return;
    }

  buffer = new_buf ();

  bprintln (buffer, str_align (70, Center,
			       "{W[{RThera's Most Popular Mobs{W]{x"));
  bprintln
    (buffer,
     "{GNum  Mob Name                  Level  Area Name            Deaths{x");
  bprintln (buffer,
	    "{m---  ------------------------- -----  -------------------- ------{x");

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = char_index_hash[iHash]; pMobIndex != NULL;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex->deaths > 2)
	    {
	      mobs[count] = pMobIndex;
	      count++;
	    }
	}
    }

  qsort (mobs, count, sizeof (pMobIndex), compare_mdeaths);

  for (i = 0; i < count; i++)
    {
      if (i == 50)
	break;

      bprintlnf (buffer, "%3d) %s %5d  %-20s {R%6ld{x", i + 1,
		 str_width (25,
			    mobs[i]->short_descr), mobs[i]->level,
		 mobs[i]->area->name, mobs[i]->deaths);
    }

  if (!count)
    bprintln (buffer, "\n\rNo Mobs listed yet.");

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
  free_mem (mobs);
  return;
}

int
compare_mkills (const void *v1, const void *v2)
{
  return (*(CharIndex * *)v2)->kills - (*(CharIndex * *)v1)->kills;
}

Do_Fun (do_mobkills)
{
  Buffer *buffer;
  CharIndex *pMobIndex;
  CharIndex **mobs;
  int iHash, count, i;

  count = 0;
  i = 0;

  if (IsImmortal (ch) && !NullStr (argument))
    {
      char arg[MIL];

      argument = one_argument (argument, arg);

      if (!str_cmp (arg, "reset"))
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "reset <vnum>", "reset all", NULL);
	    }

	  else if (is_number (argument))
	    {
	      if ((pMobIndex = get_char_index (atov (argument))) != NULL)
		{
		  if (pMobIndex->kills > 0)
		    TouchArea (pMobIndex->area);
		  pMobIndex->kills = 0;
		  chprintlnf (ch, "Kills set to 0 on mob vnum %ld.",
			      pMobIndex->vnum);
		}
	      else
		chprintln (ch, "No such mob.");
	    }
	  else if (!str_cmp (argument, "all"))
	    {
	      for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		{
		  for (pMobIndex = char_index_hash[iHash];
		       pMobIndex != NULL; pMobIndex = pMobIndex->next)
		    {
		      if (pMobIndex->kills > 0)
			TouchArea (pMobIndex->area);
		      pMobIndex->kills = 0;
		    }
		}
	      chprintln (ch, "All mob kills set to 0.");
	    }
	  return;
	}
    }

  alloc_mem (mobs, CharIndex *, top_char_index);

  if (mobs == NULL)
    {
      log_error ("do_mobkills");
      return;
    }

  buffer = new_buf ();

  bprintln (buffer, str_align (70, Center,
			       "{W[{RThera's Most Dangerous Monsters{W]{x"));
  bprintln
    (buffer,
     "{GNum  Mob Name                  Level  Area Name            Kills{x");
  bprintln (buffer,
	    "{m---  ------------------------- -----  -------------------- ------{x");

  for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
      for (pMobIndex = char_index_hash[iHash]; pMobIndex != NULL;
	   pMobIndex = pMobIndex->next)
	{
	  if (pMobIndex->kills > 2)
	    {
	      mobs[count] = pMobIndex;
	      count++;
	    }
	}
    }

  qsort (mobs, count, sizeof (pMobIndex), compare_mkills);

  for (i = 0; i < count; i++)
    {
      if (i == 50)
	break;

      bprintlnf (buffer, "%3d) %s %5d  %-20s {R%6ld{x", i + 1,
		 str_width (25,
			    mobs[i]->short_descr), mobs[i]->level,
		 mobs[i]->area->name, mobs[i]->kills);
    }

  if (!count)
    bprintln (buffer, "\n\rNo Mobs listed yet.");

  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
  free_mem (mobs);
  return;
}


int
compare_areastat (const void *v1, const void *v2)
{
  AreaIndex area1 = *(AreaIndex *) v1;
  AreaIndex area2 = *(AreaIndex *) v2;

  if (area1.stat > area2.stat)
    return -1;
  if (area1.stat < area2.stat)
    return 1;
  return 0;
}

Do_Fun (do_areakills)
{
  AreaData *area;
  AreaIndex *list;
  int i = 0;
  int c = 0;
  Buffer *buf;

  if (!str_cmp (argument, "reset"))
    {
      if (!IsTrusted (ch, MAX_LEVEL - 1))
	{
	  chprintln (ch, "You can't reset area stats.");
	  return;
	}
      for (area = area_first; area != NULL; area = area->next)
	{
	  if (area->kills > 0)
	    TouchArea (area);
	  area->kills = 0;
	}
      return;
    }

  buf = new_buf ();
  alloc_mem (list, AreaIndex, top_area);

  bprintlnf
    (buf, " {D[{Y%3s{D][{G%-25.25s{D][{Y%7s{D][{C%7s{D]{x",
     "Num", "Area Name", "Levels", "Kills");
  bprintlnf (buf, draw_line (ch, NULL, 0));
  for (area = area_first; area != NULL; area = area->next)
    {
      list[i].area = area;
      list[i].stat = area->kills;
      i++;
    }
  qsort (list, i, sizeof (AreaIndex), compare_areastat);

  for (c = 0; c < i; c++)
    {
      AreaData *single = list[c].area;

      if (!IsSet (single->area_flags, AREA_CLOSED) || single->kills <= 0)
	continue;

      bprintlnf
	(buf, " {D[{Y%3d{D][{G%-25.25s{D][{Y%7s{D][{C%7ld{D]{x",
	 c + 1, single->name, print_area_levels (single), single->kills);

    }
  bprintln (buf, draw_line (ch, NULL, 0));
  sendpage (ch, buf_string (buf));
  free_buf (buf);
  free_mem (list);
  return;
}

Do_Fun (do_areadeaths)
{
  AreaData *area;
  AreaIndex *list;
  int i = 0;
  int c = 0;
  Buffer *buf;

  if (!str_cmp (argument, "reset"))
    {
      if (!IsTrusted (ch, MAX_LEVEL - 1))
	{
	  chprintln (ch, "You can't reset area stats.");
	  return;
	}
      for (area = area_first; area != NULL; area = area->next)
	{
	  if (area->deaths > 0)
	    TouchArea (area);
	  area->deaths = 0;
	}
      return;
    }

  buf = new_buf ();
  alloc_mem (list, AreaIndex, top_area);

  bprintlnf
    (buf, " {D[{Y%3s{D][{G%-25.25s{D][{Y%7s{D][{C%7s{D]{x",
     "Num", "Area Name", "Levels", "Deaths");
  bprintlnf (buf, draw_line (ch, NULL, 0));
  for (area = area_first; area != NULL; area = area->next)
    {
      list[i].area = area;
      list[i].stat = area->deaths;
      i++;
    }
  qsort (list, i, sizeof (AreaIndex), compare_areastat);

  for (c = 0; c < i; c++)
    {
      AreaData *single = list[c].area;

      if (IsSet (single->area_flags, AREA_CLOSED) || single->deaths <= 0)
	continue;

      bprintlnf
	(buf, " {D[{Y%3d{D][{G%-25.25s{D][{Y%7s{D][{C%7ld{D]{x",
	 c + 1, single->name, print_area_levels (single), single->deaths);

    }
  bprintln (buf, draw_line (ch, NULL, 0));
  sendpage (ch, buf_string (buf));
  free_buf (buf);
  free_mem (list);
  return;
}
