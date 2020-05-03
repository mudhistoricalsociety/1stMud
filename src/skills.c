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


Do_Fun (do_gain)
{
  char arg[MAX_INPUT_LENGTH];
  CharData *trainer;
  int gn = 0, sn = 0, val;
  int mult;

  if (IsNPC (ch))
    return;


  for (trainer = ch->in_room->person_first; trainer != NULL;
       trainer = trainer->next_in_room)
    if (IsNPC (trainer) && IsSet (trainer->act, ACT_GAIN))
      break;

  if (trainer == NULL || !can_see (ch, trainer))
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  mult = mult_argument (argument, arg);

  if (NullStr (arg))
    {
      do_function (trainer, &do_say, "Pardon me?");
      return;
    }

  if (!str_prefix (arg, "list"))
    {
      Buffer *output;
      Column *Cd;

      Cd = new_column ();
      output = new_buf ();

      set_cols (Cd, ch, 3, COLS_BUF, output);

      cols_header (Cd, "{w%-18s Cost ", "Group");

      for (gn = 0; gn < top_group; gn++)
	{
	  if (!ch->pcdata->group_known[gn]
	      && (val = group_rating (ch, gn)) > 0)
	    {
	      print_cols (Cd, "%-18s %-4d ", group_table[gn].name, val);
	    }
	}
      cols_nl (Cd);

      bprintln (output, NULL);

      set_cols (Cd, ch, 3, COLS_BUF, output);

      cols_header (Cd, "{w%-15s %-4s Lev ", "Skill", "Cost");

      for (sn = 0; sn < top_skill; sn++)
	{
	  if (!ch->pcdata->learned[sn] &&
	      (val = skill_rating (ch, sn)) > 0 &&
	      skill_table[sn].spell_fun == spell_null)
	    {
	      print_cols (Cd, "%-16s %-3d %-3d ",
			  skill_table[sn].name, val, skill_level (ch, sn));
	    }
	}
      cols_nl (Cd);
      bprintlnf (output, "You have %s left.", intstr (ch->train, "train"));
      bprintlnf (output,
		 "Use 'grlist' to see what skills are within a group.");

      sendpage (ch, buf_string (output));
      free_buf (output);
      free_column (Cd);
      return;
    }

  if (!str_prefix (arg, "convert"))
    {
      if (ch->practice < 10 * mult)
	{
	  act ("$N tells you 'You are not yet ready.'", ch, NULL,
	       trainer, TO_CHAR);
	  return;
	}

      act ("$N helps you apply your practice to training", ch, NULL,
	   trainer, TO_CHAR);
      ch->practice -= 10 * mult;
      ch->train += mult;
      return;
    }

  if (!str_prefix (arg, "points"))
    {
      int xp;

      if (ch->train < 2)
	{
	  act ("$N tells you 'You are not yet ready.'", ch, NULL,
	       trainer, TO_CHAR);
	  return;
	}

      if (ch->pcdata->points <= 40)
	{
	  act ("$N tells you 'There would be no point in that.'",
	       ch, NULL, trainer, TO_CHAR);
	  return;
	}

      act ("$N trains you, and you feel more at ease with your skills.",
	   ch, NULL, trainer, TO_CHAR);

      ch->train -= 2;
      xp = ch->exp - (ch->level * exp_per_level (ch, ch->pcdata->points));
      ch->pcdata->points -= 1;
      ch->exp = exp_per_level (ch, ch->pcdata->points) * ch->level;
      ch->exp += xp;
      return;
    }



  gn = group_lookup (argument);
  if (gn >= 0 && gn < top_group)
    {
      if (ch->pcdata->group_known[gn])
	{
	  act ("$N tells you 'You already know that group!'", ch,
	       NULL, trainer, TO_CHAR);
	  return;
	}

      if ((val = group_rating (ch, gn)) < 1)
	{
	  act ("$N tells you 'That group is beyond your powers.'",
	       ch, NULL, trainer, TO_CHAR);
	  return;
	}

      if (ch->train < val)
	{
	  act ("$N tells you 'You are not yet ready for that group.'",
	       ch, NULL, trainer, TO_CHAR);
	  return;
	}


      gn_add (ch, gn);
      act ("$N trains you in the art of $t", ch, group_table[gn].name,
	   trainer, TO_CHAR);
      ch->train -= val;
      return;
    }

  sn = skill_lookup (argument);
  if (sn > -1 && sn < top_skill)
    {
      if (skill_table[sn].spell_fun != spell_null)
	{
	  act ("$N tells you 'You must learn the full group.'", ch,
	       NULL, trainer, TO_CHAR);
	  return;
	}

      if (ch->pcdata->learned[sn])
	{
	  act ("$N tells you 'You already know that skill!'", ch,
	       NULL, trainer, TO_CHAR);
	  return;
	}

      if ((val = skill_rating (ch, sn)) < 1)
	{
	  act ("$N tells you 'That skill is beyond your powers.'",
	       ch, NULL, trainer, TO_CHAR);
	  return;
	}

      if (ch->train < skill_rating (ch, sn))
	{
	  act ("$N tells you 'You are not yet ready for that skill.'",
	       ch, NULL, trainer, TO_CHAR);
	  return;
	}


      ch->pcdata->learned[sn] = 1;
      act ("$N trains you in the art of $t", ch, skill_table[sn].name,
	   trainer, TO_CHAR);
      ch->train -= val;
      return;
    }

  act ("$N tells you 'I do not understand...'", ch, NULL, trainer, TO_CHAR);
}



Do_Fun (do_spells)
{
  Buffer *buffer;
  char arg[MAX_INPUT_LENGTH];
  char spell_list[MAX_MORTAL_LEVEL + 1][MAX_STRING_LENGTH];
  char spell_columns[MAX_MORTAL_LEVEL + 1];
  int sn, level, min_lev = 1, max_lev = MAX_MORTAL_LEVEL, mana;
  bool fAll = false, found = false;
  char buf[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    return;

  if (!NullStr (argument))
    {
      fAll = true;

      if (str_prefix (argument, "all"))
	{
	  argument = one_argument (argument, arg);
	  if (!is_number (arg))
	    {
	      chprintln (ch, "Arguments must be numerical or all.");
	      return;
	    }
	  max_lev = atoi (arg);

	  if (max_lev < 1 || max_lev > MAX_MORTAL_LEVEL)
	    {
	      chprintlnf (ch, "Levels must be between 1 and %d.",
			  MAX_MORTAL_LEVEL);
	      return;
	    }

	  if (!NullStr (argument))
	    {
	      argument = one_argument (argument, arg);
	      if (!is_number (arg))
		{
		  chprintln (ch, "Arguments must be numerical or all.");
		  return;
		}
	      min_lev = max_lev;
	      max_lev = atoi (arg);

	      if (max_lev < 1 || max_lev > MAX_MORTAL_LEVEL)
		{
		  chprintlnf (ch,
			      "Levels must be between 1 and %d.",
			      MAX_MORTAL_LEVEL);
		  return;
		}

	      if (min_lev > max_lev)
		{
		  chprintln (ch, "That would be silly.");
		  return;
		}
	    }
	}
    }


  for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
    {
      spell_columns[level] = 0;
      spell_list[level][0] = '\0';
    }

  for (sn = 0; sn < top_skill; sn++)
    {
      if ((level = skill_level (ch, sn)) < MAX_MORTAL_LEVEL + 1 &&
	  (fAll || level <= ch->level) && level >= min_lev &&
	  level <= max_lev && skill_table[sn].spell_fun != spell_null &&
	  ch->pcdata->learned[sn] > 0)
	{
	  found = true;
	  level = skill_level (ch, sn);
	  if (ch->level < level)
	    sprintf (buf, "%-18s n/a      ", skill_table[sn].name);
	  else
	    {
	      mana =
		Max (skill_table[sn].min_mana, 100 / (2 + ch->level - level));
	      sprintf (buf, "%-18s  %3d mana  ", skill_table[sn].name, mana);
	    }

	  if (spell_list[level][0] == '\0')
	    sprintf (spell_list[level], NEWLINE "Level %2d: %s", level, buf);
	  else
	    {

	      if (++spell_columns[level] % 2 == 0)
		strcat (spell_list[level], NEWLINE "          ");
	      strcat (spell_list[level], buf);
	    }
	}
    }



  if (!found)
    {
      chprintln (ch, "No spells found.");
      return;
    }

  buffer = new_buf ();
  for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
    if (spell_list[level][0] != '\0')
      bprint (buffer, spell_list[level]);
  bprintln (buffer, NULL);
  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
}

Do_Fun (do_skills)
{
  Buffer *buffer;
  char arg[MAX_INPUT_LENGTH];
  char skill_list[MAX_MORTAL_LEVEL + 1][MAX_STRING_LENGTH];
  char skill_columns[MAX_MORTAL_LEVEL + 1];
  int sn, level, min_lev = 1, max_lev = MAX_MORTAL_LEVEL;
  bool fAll = false, found = false;
  char buf[MAX_STRING_LENGTH];

  if (IsNPC (ch))
    return;

  if (!NullStr (argument))
    {
      fAll = true;

      if (str_prefix (argument, "all"))
	{
	  argument = one_argument (argument, arg);
	  if (!is_number (arg))
	    {
	      chprintln (ch, "Arguments must be numerical or all.");
	      return;
	    }
	  max_lev = atoi (arg);

	  if (max_lev < 1 || max_lev > MAX_MORTAL_LEVEL)
	    {
	      chprintlnf (ch, "Levels must be between 1 and %d.",
			  MAX_MORTAL_LEVEL);
	      return;
	    }

	  if (!NullStr (argument))
	    {
	      argument = one_argument (argument, arg);
	      if (!is_number (arg))
		{
		  chprintln (ch, "Arguments must be numerical or all.");
		  return;
		}
	      min_lev = max_lev;
	      max_lev = atoi (arg);

	      if (max_lev < 1 || max_lev > MAX_MORTAL_LEVEL)
		{
		  chprintlnf (ch,
			      "Levels must be between 1 and %d.",
			      MAX_MORTAL_LEVEL);
		  return;
		}

	      if (min_lev > max_lev)
		{
		  chprintln (ch, "That would be silly.");
		  return;
		}
	    }
	}
    }


  for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
    {
      skill_columns[level] = 0;
      skill_list[level][0] = '\0';
    }

  for (sn = 0; sn < top_skill; sn++)
    {
      if ((level = skill_level (ch, sn)) < MAX_MORTAL_LEVEL + 1 &&
	  (fAll || level <= ch->level) && level >= min_lev &&
	  level <= max_lev && skill_table[sn].spell_fun == spell_null &&
	  ch->pcdata->learned[sn] > 0)
	{
	  found = true;
	  level = skill_level (ch, sn);
	  if (ch->level < level)
	    sprintf (buf, "{c%-18s n/a      ", skill_table[sn].name);
	  else
	    sprintf (buf, "{c%-18s {W%3d%%      ",
		     skill_table[sn].name, ch->pcdata->learned[sn]);

	  if (skill_list[level][0] == '\0')
	    sprintf (skill_list[level], NEWLINE "{cLevel {W%2d{c: %s{x",
		     level, buf);
	  else
	    {

	      if (++skill_columns[level] % 2 == 0)
		strcat (skill_list[level], NEWLINE "{x          ");
	      strcat (skill_list[level], buf);
	    }
	}
    }



  if (!found)
    {
      chprintln (ch, "{cNo skills found.{x");
      return;
    }

  buffer = new_buf ();
  for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
    if (skill_list[level][0] != '\0')
      bprint (buffer, skill_list[level]);
  bprintln (buffer, NULL);
  sendpage (ch, buf_string (buffer));
  free_buf (buffer);
}


void
list_group_costs (CharData * ch)
{
  int gn, sn, val;
  Buffer *output;
  Column *Cd;

  if (IsNPC (ch))
    return;

  output = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 3, COLS_BUF, output);

  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  cols_header (Cd, "{w%-18s CP ", "Group");

  for (gn = 0; gn < top_group; gn++)
    {
      if (!ch->gen_data->group_chosen[gn]
	  && !ch->pcdata->group_known[gn]
	  && (val = group_rating (ch, gn)) > 0)
	{
	  print_cols (Cd, "{c%-18s {W%-2d ", group_table[gn].name, val);
	}
    }
  cols_nl (Cd);

  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  set_cols (Cd, ch, 2, COLS_BUF, output);

  cols_header (Cd, "{w%-18s %-2s Lev ", "Skill", "CP");

  for (sn = 0; sn < top_skill; sn++)
    {
      if (!ch->gen_data->skill_chosen[sn]
	  && ch->pcdata->learned[sn] == 0
	  && skill_table[sn].spell_fun == spell_null
	  && (val = skill_rating (ch, sn)) > 0)
	{
	  print_cols (Cd, "{c%-18s {W%-2d %-3d ",
		      skill_table[sn].name, val, skill_level (ch, sn));
	}
    }
  cols_nl (Cd);

  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  bprintln (output, NULL);

  bprintlnf (output, "{cCreation points: {W%d{x", ch->pcdata->points);
  bprintlnf (output, "{cExperience per level: {W%d{x",
	     exp_per_level (ch,
			    Max (ch->gen_data->points_chosen,
				 ch->pcdata->points)));
  sendpage (ch, buf_string (output));
  free_buf (output);
  free_column (Cd);
  return;
}

void
list_group_chosen (CharData * ch)
{
  int gn, sn, val;
  Buffer *output;
  Column *Cd;

  if (IsNPC (ch))
    return;

  output = new_buf ();

  Cd = new_column ();
  set_cols (Cd, ch, 3, COLS_BUF, output);


  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  print_cols (Cd, "{w%-18s CP ", "Group");

  for (gn = 0; gn < top_group; gn++)
    {
      if (ch->gen_data->group_chosen[gn] && (val = group_rating (ch, gn)) > 0)
	{
	  print_cols (Cd, "{c%-18s {W%-2d ", group_table[gn].name, val);
	}
    }
  cols_nl (Cd);

  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  set_cols (Cd, ch, 2, COLS_BUF, output);

  cols_header (Cd, "{w%-18s %-2s Lev", "Skill", "CP");

  for (sn = 0; sn < top_skill; sn++)
    {
      if (ch->gen_data->skill_chosen[sn] && (val = skill_rating (ch, sn)) > 0)
	{
	  print_cols (Cd, "{c%-18s {W%-2d %-3d ", skill_table[sn].name,
		      val, skill_level (ch, sn));
	}
    }
  cols_nl (Cd);

  bprintlnf (output, "{c%s", draw_line (ch, NULL, 0));

  bprintlnf (output, "{cCreation points: {W%d{x",
	     ch->gen_data->points_chosen);
  bprintlnf (output, "{cExperience per level: {W%d{x",
	     exp_per_level (ch, ch->gen_data->points_chosen));
  sendpage (ch, buf_string (output));
  free_buf (output);
  free_column (Cd);
  return;
}

int
exp_per_level (CharData * ch, int points)
{
  int expl, inc;

  if (IsNPC (ch))
    return 1000;

  expl = 1000;
  inc = 500;

  if (points >= mud_info.max_points)
    {

      points -= mud_info.max_points;

      while (points >= 10)
	{
	  expl += inc;
	  points -= 10;
	  if (points >= 10)
	    {
	      expl += inc;
	      inc *= 2;
	      points -= 10;
	    }
	}

      expl += points * inc / 10;
    }

  expl *= class_mult (ch);
  expl /= 100;

  return expl;
}


bool
parse_gen_groups (CharData * ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int gn, sn, i;

  if (NullStr (argument))
    return false;

  argument = one_argument (argument, arg);

  if (!str_prefix (arg, "help"))
    {
      if (NullStr (argument))
	{
	  chprintln (ch, "{cThe following commands are available:{x");
	  chprintln (ch,
		     "{clist          display all groups and skills not yet bought{x");
	  chprintln (ch,
		     "{clearned       show all groups and skills bought {x");
	  chprintln (ch,
		     "{cpremise       brief explanation of creation points and skill groups{x");
	  chprintln (ch, "{cadd <name>    buy a skill or group{x");
	  chprintln (ch, "{cdrop <name>   discard a skill or group{x");
	  chprintln (ch,
		     "{cinfo <name>   list the skills or spells contained within a group{x");
	  chprintln (ch, "{clookup <name> show class levels for skill{x");
	  chprintln (ch,
		     "{chelp <name>   help on skills and groups, or other help topics{x");
	  chprintln (ch,
		     "{cdone          exit the character generation process{x");
	  return true;
	}

      chprint (ch, "{c");
      do_function (ch, &do_oldhelp, argument);
      chprint (ch, "{x");

      return true;
    }

  if (!str_prefix (arg, "add"))
    {
      int val;

      if (NullStr (argument))
	{
	  chprintln (ch, "{cYou must provide a skill name.{x");
	  return true;
	}

      gn = group_lookup (argument);
      if (gn != -1)
	{
	  if (ch->gen_data->group_chosen[gn] || ch->pcdata->group_known[gn])
	    {
	      chprintln (ch, "{cYou already know that group!{x");
	      return true;
	    }

	  if ((val = group_rating (ch, gn)) < 1)
	    {
	      chprintln (ch, "{cThat group is not available.{x");
	      return true;
	    }


	  if (ch->gen_data->points_chosen + val > 300)
	    {
	      chprintln (ch,
			 "{cYou cannot take more than 300 creation points.{x");
	      return true;
	    }

	  chprintlnf (ch, "{W%s{c group added.{x",
		      Upper (group_table[gn].name));
	  ch->gen_data->group_chosen[gn] = true;
	  ch->gen_data->points_chosen += val;
	  gn_add (ch, gn);
	  ch->pcdata->points += val;
	  return true;
	}

      sn = skill_lookup (argument);
      if (sn != -1)
	{
	  if (ch->gen_data->skill_chosen[sn] || ch->pcdata->learned[sn] > 0)
	    {
	      chprintln (ch, "{cYou already know that skill!{x");
	      return true;
	    }

	  if ((val = skill_rating (ch, sn)) < 1 ||
	      skill_table[sn].spell_fun != spell_null)
	    {
	      chprintln (ch, "{cThat skill is not available.{x");
	      return true;
	    }


	  if (ch->gen_data->points_chosen + val > 300)
	    {
	      chprintln (ch,
			 "{cYou cannot take more than 300 creation points.{x");
	      return true;
	    }
	  chprintlnf (ch, "{W%s{c skill added.{x",
		      Upper (skill_table[sn].name));
	  ch->gen_data->skill_chosen[sn] = true;
	  ch->gen_data->points_chosen += val;
	  ch->pcdata->learned[sn] = 1;
	  ch->pcdata->points += val;
	  return true;
	}

      chprintln (ch, "{cNo skills or groups by that name...{x");
      return true;
    }

  if (!str_cmp (arg, "drop"))
    {
      int val;

      if (NullStr (argument))
	{
	  chprintln (ch, "{cYou must provide a skill to drop.{x");
	  return true;
	}

      gn = group_lookup (argument);
      if (gn != -1 && ch->gen_data->group_chosen[gn])
	{
	  chprintlnf (ch, "{W%s{c group dropped.",
		      Upper (group_table[gn].name));
	  ch->gen_data->group_chosen[gn] = false;
	  val = group_rating (ch, gn);
	  ch->gen_data->points_chosen -= val;
	  gn_remove (ch, gn);
	  for (i = 0; i < top_group; i++)
	    {
	      if (ch->gen_data->group_chosen[gn])
		gn_add (ch, gn);
	    }
	  ch->pcdata->points -= val;
	  return true;
	}

      sn = skill_lookup (argument);
      if (sn != -1 && ch->gen_data->skill_chosen[sn])
	{
	  chprintlnf (ch, "{W%s{c skill dropped.",
		      Upper (skill_table[sn].name));
	  ch->gen_data->skill_chosen[sn] = false;
	  val = skill_rating (ch, sn);
	  ch->gen_data->points_chosen -= val;
	  ch->pcdata->learned[sn] = 0;
	  ch->pcdata->points -= val;
	  return true;
	}

      chprintln (ch, "{cYou haven't bought any such skill or group.{x");
      return true;
    }

  if (!str_prefix (arg, "premise"))
    {
      chprint (ch, "{c");
      do_function (ch, &do_oldhelp, "premise");
      chprint (ch, "{x");
      return true;
    }

  if (!str_prefix (arg, "list"))
    {
      list_group_costs (ch);
      return true;
    }

  if (!str_prefix (arg, "learned"))
    {
      list_group_chosen (ch);
      return true;
    }

  if (!str_prefix (arg, "info"))
    {
      do_grlist ("info", ch, argument);
      return true;
    }
  if (!str_prefix (arg, "lookup"))
    {
      do_slist ("lookup", ch, argument);
      return true;
    }
  return false;
}


void
check_improve (CharData * ch, int sn, bool success, int multiplier)
{
  int chance;

  if (IsNPC (ch))
    return;

  if (!can_use_skpell (ch, sn)
      || skill_rating (ch, sn) < 1 ||
      ch->pcdata->learned[sn] == 0 || ch->pcdata->learned[sn] == 100)
    return;


  chance = 10 * int_app[get_curr_stat (ch, STAT_INT)].learn;
  chance /= (multiplier * skill_rating (ch, sn) * 4);
  chance += ch->level;

  if (number_range (1, 1000) > chance)
    return;



  if (success)
    {
      chance = Range (5, 100 - ch->pcdata->learned[sn], 95);
      if (number_percent () < chance)
	{
	  chprintlnf (ch, "You have become better at %s!",
		      skill_table[sn].name);
	  ch->pcdata->learned[sn]++;
	  gain_exp (ch, 2 * skill_rating (ch, sn));
	}
    }
  else
    {
      chance = Range (5, ch->pcdata->learned[sn] / 2, 30);
      if (number_percent () < chance)
	{
	  chprintlnf (ch,
		      "You learn from your mistakes, and your %s skill improves.",
		      skill_table[sn].name);
	  ch->pcdata->learned[sn] += number_range (1, 3);
	  ch->pcdata->learned[sn] = Min (ch->pcdata->learned[sn], 100);
	  gain_exp (ch, 2 * skill_rating (ch, sn));
	}
    }
  if (ch->pcdata->learned[sn] == 100)
    {
      chprintlnf (ch, "{GYou have now mastered the '%s' %s!",
		  skill_table[sn].name,
		  skill_table[sn].spell_fun ? "spell" : "skill");
    }
}


Lookup_Fun (group_lookup)
{
  int gn;

  for (gn = 0; gn < top_group; gn++)
    {
      if (tolower (name[0]) == tolower (group_table[gn].name[0]) &&
	  !str_prefix (name, group_table[gn].name))
	return gn;
    }

  return -1;
}


void
gn_add (CharData * ch, int gn)
{
  int i;

  if (IsNPC (ch) || gn >= top_group || gn < 0)
    return;
  ch->pcdata->group_known[gn] = true;
  for (i = 0; i < MAX_IN_GROUP; i++)
    {
      if (group_table[gn].spells[i] == NULL)
	break;
      group_add (ch, group_table[gn].spells[i], false);
    }
}


void
gn_remove (CharData * ch, int gn)
{
  int i;

  if (IsNPC (ch) || gn >= top_group || gn < 0)
    return;

  if (check_base_group (ch, gn))
    return;

  ch->pcdata->group_known[gn] = false;

  for (i = 0; i < MAX_IN_GROUP; i++)
    {
      if (group_table[gn].spells[i] == NULL)
	break;
      group_remove (ch, group_table[gn].spells[i]);
    }
}


void
group_add (CharData * ch, const char *name, bool deduct)
{
  int sn, gn;

  if (IsNPC (ch))
    return;

  sn = skill_lookup (name);

  if (sn != -1)
    {
      if (ch->pcdata->learned[sn] == 0)
	{
	  ch->pcdata->learned[sn] = 1;
	  if (deduct)
	    ch->pcdata->points += skill_rating (ch, sn);
	}
      return;
    }



  gn = group_lookup (name);

  if (gn != -1)
    {
      if (ch->pcdata->group_known[gn] == false)
	{
	  ch->pcdata->group_known[gn] = true;
	  if (deduct)
	    ch->pcdata->points += group_rating (ch, gn);
	}
      gn_add (ch, gn);
    }
}



void
group_remove (CharData * ch, const char *name)
{
  int sn, gn;

  sn = skill_lookup (name);

  if (sn != -1)
    {
      if (!is_base_skill (ch, sn))
	ch->pcdata->learned[sn] = 0;
      return;
    }



  gn = group_lookup (name);

  if (gn != -1 && ch->pcdata->group_known[gn] == true)
    {
      ch->pcdata->group_known[gn] = false;
      gn_remove (ch, gn);
    }
}

Do_Fun (do_slist)
{
  int sn, clas;
  bool found = false;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "<skill>", "<spell>", "<class>", NULL);
      return;
    }
  else if ((clas = class_lookup (argument)) != -1)
    {
      char buf[MSL];
      Buffer *buffer;
      char skill_list[MAX_MORTAL_LEVEL + 1][5000];
      int skill_columns[MAX_MORTAL_LEVEL + 1];
      int level;

      for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
	{
	  skill_columns[level] = 0;
	  skill_list[level][0] = NUL;
	}

      for (sn = 0; sn < top_skill; sn++)
	{
	  if ((level = skill_table[sn].skill_level[clas]) <= MAX_MORTAL_LEVEL)
	    {
	      found = true;
	      sprintf (buf, "{c%-18s      ", skill_table[sn].name);

	      if (skill_list[level][0] == NUL)
		sprintf (skill_list[level],
			 NEWLINE "{cLevel {W%3d{c: %s{x", level, buf);
	      else
		{

		  if (++skill_columns[level] % 2 == 0)
		    strcat (skill_list[level], NEWLINE "{x           ");

		  strcat (skill_list[level], buf);
		}

	    }

	}

      buffer = new_buf ();
      for (level = 0; level < MAX_MORTAL_LEVEL + 1; level++)
	{
	  if (skill_list[level][0] != NUL)
	    bprint (buffer, skill_list[level]);
	}
      bprintln (buffer, NULL);
      sendpage (ch, buf_string (buffer));
      free_buf (buffer);
      return;
    }
  else if ((sn = skill_lookup (argument)) != -1)
    {
      int x;

      chprintf (ch, NEWLINE "{c%s: [ ", capitalize (skill_table[sn].name));
      for (x = 0; x < top_class; x++)
	{
	  if (skill_table[sn].skill_level[x] > MAX_MORTAL_LEVEL)
	    chprintf (ch, "{W%.3s: %3s{c  ", ClassName (ch, x), "n/a");
	  else
	    chprintf (ch, "{W%.3s: %03d{c  ", ClassName (ch, x),
		      skill_table[sn].skill_level[x]);
	}
      chprintln (ch, "]{x");
      return;
    }
  else
    {
      do_slist (n_fun, ch, "");
      return;
    }
}


int
spell_avail (CharData * ch, const char *name)
{

  int sn;

  sn = skill_lookup (name);

  if (sn != -1 && skill_level (ch, sn) > MAX_MORTAL_LEVEL)
    return -1;

  return sn;
}

Do_Fun (do_grlist)
{
  int gn, sn, tn;
  bool found = false;
  Column *Cd;

  if (!ch || IsNPC (ch))
    return;

  Cd = new_column ();
  set_cols (Cd, ch, 4, COLS_CHAR, ch);

  if (NullStr (argument))
    {
      for (gn = 0; gn < top_group; gn++)
	{
	  if (ch->pcdata->group_known[gn])
	    {
	      if (!found)
		{
		  chprintln (ch, "{cGroups you currently have:");
		  chprintln (ch, draw_line (ch, NULL, 0));
		}
	      print_cols (Cd, "{W%s", group_table[gn].name);
	      found = true;
	    }
	}
      if (!found)
	chprintln (ch, "{cYou know no groups.{x");
      else
	cols_nl (Cd);
      chprintlnf (ch, "{cCreation points: {W%d{x", ch->pcdata->points);
    }
  else if (!str_cmp (argument, "all"))
    {

      for (gn = 0; gn < top_group; gn++)
	{
	  if (IsImmortal (ch) || group_rating (ch, gn) > 0)
	    {
	      if (!found)
		{
		  chprintln (ch, "{cGroups available to you:");
		  chprintln (ch, draw_line (ch, NULL, 0));
		}
	      print_cols (Cd, "{W%s", group_table[gn].name);
	      found = true;
	    }
	}
      if (!found)
	chprintln (ch, "{cNo groups are available to you.{x");
      else
	cols_nl (Cd);
    }
  else if ((tn = class_lookup (argument)) != -1)
    {
      for (gn = 0; gn < top_group; gn++)
	{
	  if (group_table[gn].rating[tn] <= 0)
	    continue;

	  if (!found)
	    {
	      chprintlnf (ch, "{cGroups available for the {W%s{c"
			  " class:", class_table[tn].name[0]);
	      chprintln (ch, draw_line (ch, NULL, 0));
	    }
	  print_cols (Cd, "{W%s", group_table[gn].name);
	  found = true;
	}
      if (!found)
	chprintlnf (ch, "{cThere are no groups available to the {W%s{c"
		    " class.{x", class_table[tn].name[0]);
      else
	cols_nl (Cd);
    }
  else if ((gn = group_lookup (argument)) != -1)
    {
      for (sn = 0; sn < MAX_IN_GROUP; sn++)
	{
	  if (group_table[gn].spells[sn] == NULL)
	    break;
	  if ((tn = spell_avail (ch, group_table[gn].spells[sn])) >= 0)
	    {
	      if (!found)
		{
		  chprintlnf (ch, "{cSpells available in {W%s{c:{x",
			      group_table[gn].name);
		  cols_header (Cd, "{cLevel {WSpell{x");

		  chprintlnf (ch, "{c%s{x", draw_line (ch, NULL, 0));
		}
	      print_cols (Cd, "{c%-5d {W%s{x", skill_level (ch, tn),
			  group_table[gn].spells[sn]);
	      found = true;
	    }
	}
      if (!found)
	chprintlnf (ch, "{cNo spells available in the {W%s{c group.{x",
		    group_table[gn].name);
      else
	cols_nl (Cd);
    }
  else if ((sn = skill_lookup (argument)) != -1)
    {
      for (gn = 0; gn < top_group; gn++)
	{
	  for (tn = 0; tn < MAX_IN_GROUP; tn++)
	    {
	      if (group_table[gn].spells[tn] == NULL)
		break;

	      if (skill_lookup (group_table[gn].spells[tn]) != sn)
		continue;

	      if (!found)
		{
		  chprintlnf (ch, "{W%s{c is in the following groups:{x",
			      skill_table[sn].name);
		  chprintlnf (ch, "{c%s{x", draw_line (ch, NULL, 0));
		}
	      print_cols (Cd, "{W%s", group_table[gn].name);
	      found = true;
	    }
	}
      if (!found)
	chprintlnf (ch, "{W%s{c can't be found in any groups.{x",
		    skill_table[sn].name);
      else
	cols_nl (Cd);
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun,
		  "        -list your current groups",
		  "all     -list all available groups",
		  "<group> -list all spells in a group",
		  "<skill> -list all groups a skill is in",
		  "<class> -list all groups available to a class", NULL);
    }
  free_column (Cd);
}
