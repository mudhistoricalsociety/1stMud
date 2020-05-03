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

#ifndef __COMM_PROMPT_H_
#define __COMM_PROMPT_H_  1





















void
bust_a_group_prompt (CharData * ch)
{
  int group_count = 0;
  int charmies_count = 0;
  bool pet_found = false;
  bool complete;
  CharData *gch;
  CharData *gvictim;
  const char *group_prompt;
  const char *src;
  char *dest;
  char *i;
  char result[MSL * 3];
  char buf[MSL];
  int lowest_percent;

  if (IsSet (ch->comm, COMM_AFK))
    return;


  for (gch = ch->in_room->person_first; gch; gch = gch->next_in_room)
    {
      if (ch != gch && is_same_group (gch, ch))
	{
	  if (ch->pet == gch)
	    pet_found = true;
	  else if (!IsNPC (gch))
	    group_count++;
	  else
	    charmies_count++;
	}
    }

  if (!pet_found && !group_count)
    return;




  if (NullStr (ch->gprompt))
    group_prompt =
      "%g[grp {R%hhp {B%mm {M%vmv{x]%G%p[pet {r%qhp {b%rm {m%smv{x>%P%C";
  else
    group_prompt = ch->gprompt;

  dest = result;

  for (src = group_prompt; !NullStr (src); src++)
    {
      if (*src != '%')
	{
	  *dest++ = *src;
	  continue;
	}

      src++;

      if (*src == '\0')
	{

	  chprintln (ch, "Your group prompt can't end with a single %");
	  return;
	}

      lowest_percent = 101;
      i = "";
      switch (*src)
	{

	default:
	  *dest++ = ' ';
	  continue;
	  break;

	case '%':
	  *dest++ = '%';
	  continue;
	  break;

	case 'c':
	  i = NEWLINE;
	  break;

	case 'C':

	  *dest = '\0';
	  if (strlen (result) > 0)
	    i = NEWLINE;
	  break;

	case 'g':
	  if (!group_count)
	    {


	      complete = false;
	      for (src++; !complete; src++)
		{
		  if (*src == '\0')
		    {



		      chprintln (ch,
				 "Your group prompt needs a %G after the %g before it will be displayed.");
		      return;
		    }
		  if (*src == '%')
		    {

		      src++;
		      if (*src == 'G')
			{
			  complete = true;
			  continue;
			}
		      if (*src == '\0')
			{



			  chprintln (ch,
				     "Your group prompt needs a %G after the %g before it will be displayed.");
			  return;
			}

		    }
		}
	      continue;
	    }
	  break;

	case 'G':

	  break;

	case 'p':
	  if (!pet_found)
	    {


	      complete = false;
	      for (src++; !complete; src++)
		{
		  if (*src == '\0')
		    {



		      chprintln (ch,
				 "Your group prompt needs a %P after the %p before it will be displayed.");
		      return;
		    }
		  if (*src == '%')
		    {

		      src++;
		      if (*src == 'P')
			{
			  complete = true;
			  continue;
			}
		      if (*src == '\0')
			{



			  chprintln (ch,
				     "Your group prompt needs a %P after the %p before it will be displayed.");
			  return;
			}

		    }
		}
	      continue;
	    }
	  break;

	case 'P':

	  break;

	case 'h':

	  if (group_count)
	    {

	      for (gch = ch->in_room->person_first; gch;
		   gch = gch->next_in_room)
		{
		  if (ch != gch && !IsNPC (gch) && ch->pet != gch
		      && is_same_group (gch, ch) && gch->max_hit != 0)
		    {
		      if (Percent (gch->hit, gch->max_hit) < lowest_percent)
			{
			  gvictim = gch;
			  lowest_percent = Percent (gch->hit, gch->max_hit);
			}
		    }
		}
	      if (lowest_percent <= 100)
		{
		  sprintf (buf, "%03d%%", lowest_percent);
		  i = buf;
		}
	    }
	  break;


	case 'm':

	  if (group_count)
	    {

	      for (gch = ch->in_room->person_first; gch;
		   gch = gch->next_in_room)
		{
		  if (ch != gch && !IsNPC (gch) && ch->pet != gch
		      && is_same_group (gch, ch) && gch->max_mana != 0)
		    {
		      if (Percent (gch->mana, gch->max_mana) < lowest_percent)
			{
			  gvictim = gch;
			  lowest_percent = Percent (gch->mana, gch->max_mana);
			}
		    }
		}
	      if (lowest_percent <= 100)
		{
		  sprintf (buf, "%03d%%", lowest_percent);
		  i = buf;
		}
	    }
	  break;

	case 'v':

	  if (group_count)
	    {


	      for (gch = ch->in_room->person_first; gch;
		   gch = gch->next_in_room)
		{
		  if (ch != gch && !IsNPC (gch) && ch->pet != gch
		      && is_same_group (gch, ch) && gch->max_move != 0)
		    {
		      if (Percent (gch->move, gch->max_move) < lowest_percent)
			{
			  gvictim = gch;
			  lowest_percent = Percent (gch->move, gch->max_move);
			}
		    }
		}
	      if (lowest_percent <= 100)
		{
		  sprintf (buf, "%03d%%", lowest_percent);
		  i = buf;
		}
	    }
	  break;

	case 'q':
	  if (pet_found && ch->pet->max_hit)
	    {
	      sprintf (buf, "%03ld%%",
		       Percent (ch->pet->hit, ch->pet->max_hit));
	      i = buf;
	    }
	  break;

	case 'r':
	  if (pet_found && ch->pet->max_mana)
	    {
	      sprintf (buf, "%03ld%%",
		       Percent (ch->pet->mana, ch->pet->max_mana));
	      i = buf;
	    }
	  break;

	case 's':
	  if (pet_found && ch->pet->max_move)
	    {
	      sprintf (buf, "%03ld%%",
		       Percent (ch->pet->move, ch->pet->max_move));
	      i = buf;
	    }
	  break;

	case 'N':
	  sprintf (buf, "%d", group_count);
	  i = buf;
	  break;

	case 'x':
	  sprintf (buf, "%d", charmies_count);
	  i = buf;
	  break;
	}

      while ((*dest = *i) != '\0')
	{
	  dest++;
	  i++;
	}
    }
  *dest = '\0';


  d_print (ch->desc, result);
}


void
bust_a_prompt (CharData * ch)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  const char *str;
  const char *i;
  char *point;
  char doors[MAX_INPUT_LENGTH];
  ExitData *pexit;
  int door;

  point = buf;
  str = ch->prompt;

  bust_a_portal (ch);

  if (NullStr (str))
    {
      d_printf (ch->desc, "<%ldhp %ldm %ldmv> %s", ch->hit, ch->mana,
		ch->move, ch->prefix);
      return;
    }

  if (IsSet (ch->comm, COMM_AFK))
    {
      if (!IsNPC (ch) && !NullStr (ch->pcdata->afk_msg))
	d_printf (ch->desc, "{y<{YAFK{y>{Y (%s{Y){x", ch->pcdata->afk_msg);
      else
	d_print (ch->desc, "{y<{YAFK{y>{x ");
      return;
    }

  d_print (ch->desc, MXPTAG ("Prompt"));

  for (str = ch->prompt; *str != '\0'; str++)
    {
      if (*str != '%')
	{
	  *point++ = *str;
	  continue;
	}
      ++str;
      buf2[0] = '\0';
      i = "";
      switch (*str)
	{
	default:
	  break;
	case 'e':
	  doors[0] = '\0';
	  for (door = 0; door < MAX_DIR; door++)
	    {
	      if ((pexit = ch->in_room->exit[door]) != NULL &&
		  pexit->u1.to_room != NULL &&
		  (can_see_room (ch, pexit->u1.to_room) ||
		   (IsAffected (ch, AFF_INFRARED) &&
		    !IsAffected (ch, AFF_BLIND))) &&
		  !IsSet (pexit->exit_info, EX_CLOSED))
		{
		  sprintf (doors + strlen (doors),
			   MXPTAG ("Ex") "%c" MXPTAG ("/Ex"),
			   toupper (dir_name[door][0]));
		}
	    }
	  sprintf (buf2, MXPTAG ("Rexits") "%s" MXPTAG ("/Rexits"),
		   GetStr (doors, "none"));
	  i = buf2;
	  break;
	case 'c':
	  strcpy (buf2, NEWLINE);
	  i = buf2;
	  break;
	case 'h':
	  sprintf (buf2, MXPTAG ("Hp") "%ld" MXPTAG ("/Hp"), ch->hit);
	  i = buf2;
	  break;
	case 'H':
	  sprintf (buf2, MXPTAG ("MaxHp") "%ld" MXPTAG ("/MaxHp"),
		   ch->max_hit);
	  i = buf2;
	  break;
	case 'm':
	  sprintf (buf2, MXPTAG ("Mana") "%ld" MXPTAG ("/Mana"), ch->mana);
	  i = buf2;
	  break;
	case 'M':
	  sprintf (buf2, MXPTAG ("MaxMana") "%ld" MXPTAG ("/MaxMana"),
		   ch->max_mana);
	  i = buf2;
	  break;
	case 'v':
	  sprintf (buf2, MXPTAG ("Move") "%ld" MXPTAG ("/Move"), ch->move);
	  i = buf2;
	  break;
	case 'V':
	  sprintf (buf2, MXPTAG ("MaxMove") "%ld" MXPTAG ("/MaxMove"),
		   ch->max_move);
	  i = buf2;
	  break;
	case 'x':
	  sprintf (buf2, "%d", ch->exp);
	  i = buf2;
	  break;
	case 'X':
	  sprintf (buf2, "%d", IsNPC (ch) ? 0 : (ch->level + 1) *
		   exp_per_level (ch, ch->pcdata->points) - ch->exp);
	  i = buf2;
	  break;
	case 'g':
	  sprintf (buf2, "%ld", ch->gold);
	  i = buf2;
	  break;
	case 's':
	  sprintf (buf2, "%ld", ch->silver);
	  i = buf2;
	  break;
	case 'b':
	  sprintf (buf2, "%ld%%", Percent (ch->hit, ch->max_hit));
	  i = buf2;
	  break;
	case 'B':
	  if (ch->fighting)
	    sprintf (buf2, "%ld%%",
		     Percent (ch->fighting->hit, ch->fighting->max_hit));
	  else
	    strcpy (buf2, "??");
	  i = buf2;
	  break;
	case 'a':
	  if (ch->level > 9)
	    sprintf (buf2, "%d", ch->alignment);
	  else
	    sprintf (buf2, "%s",
		     IsGood (ch) ? "good" : IsEvil (ch) ? "evil" : "neutral");
	  i = buf2;
	  break;
	case 'r':
	  if (ch->in_room != NULL)
	    sprintf (buf2, "%s", check_blind (ch) &&
		     !room_is_dark (ch->in_room) ? ch->in_room->name
		     : "darkness");
	  i = buf2;
	  break;
	case 'R':
	  if (IsImmortal (ch) && ch->in_room != NULL)
	    sprintf (buf2, "%ld", ch->in_room->vnum);
	  i = buf2;
	  break;
	case 'Q':
	  if (gquest_info.running != GQUEST_OFF)
	    sprintf (buf2, "%d", gquest_info.timer);
	  else
	    sprintf (buf2, "%dn", gquest_info.timer);
	  i = buf2;
	  break;

	case 'z':
	  if (IsImmortal (ch) && ch->in_room != NULL)
	    sprintf (buf2, "%s", ch->in_room->area->name);
	  i = buf2;
	  break;
	case '%':
	  sprintf (buf2, "%%");
	  i = buf2;
	  break;
	case 'o':
	  sprintf (buf2, MXPTAG ("Olc") "%s" MXPTAG ("/Olc"),
		   olc_ed_name (ch->desc));
	  i = buf2;
	  break;
	case 'O':
	  sprintf (buf2, "%s", olc_ed_vnum (ch->desc));
	  i = buf2;
	  break;
	case 'q':
	  if (IsQuester (ch))
	    sprintf (buf2, "%d", ch->pcdata->quest.time);
	  else
	    strcpy (buf2, "0");
	  i = buf2;
	  break;
	}
      while ((*point = *i) != '\0')
	++point, ++i;
    }
  *point = NUL;

  d_print (ch->desc, buf);
  d_print (ch->desc, MXPTAG ("/Prompt"));
  if (!NullStr (ch->prefix))
    d_print (ch->desc, ch->prefix);
  return;
}
#endif
