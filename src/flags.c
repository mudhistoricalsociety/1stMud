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

Do_Fun (do_flag)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
  char word[MAX_INPUT_LENGTH];
  CharData *victim;
  ObjData *obj;
  flag_t *flag, old = 0, pnew = 0, marked = 0, pos;
  char type;
  FlagTable *flag_table;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  type = argument[0];

  if (type == '=' || type == '-' || type == '+')
    argument = one_argument (argument, word);

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun, "mob <name> <field> <flags>",
		  "char <name> <field> <flags>",
		  "obj <name> <field> <flags>", NULL);

      chprintln (ch, "  mob flags: act,aff,off,imm,res,vuln,form,part");
      chprintln (ch, "  char flags: plr,comm,aff,imm,res,vuln");
      chprintln (ch, "  obj flags: extra,wear");
      chprintln (ch, "  +: add flag, -: remove flag, = set equal to");
      chprintln (ch, "  otherwise flag toggles the flags listed.");
      return;
    }

  if (NullStr (arg2))
    {
      chprintln (ch, "What do you wish to set flags on?");
      return;
    }

  if (NullStr (arg3))
    {
      chprintln (ch, "You need to specify a flag to set.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Which flags do you wish to change?");
      return;
    }

  if (!str_prefix (arg1, "mob") || !str_prefix (arg1, "char"))
    {
      victim = get_char_world (ch, arg2);
      if (victim == NULL)
	{
	  chprintln (ch, "You can't find them.");
	  return;
	}


      if (!str_prefix (arg3, "act"))
	{
	  if (!IsNPC (victim))
	    {
	      chprintln (ch, "Use plr for PCs.");
	      return;
	    }

	  flag = &victim->act;
	  flag_table = act_flags;
	}
      else if (!str_prefix (arg3, "plr"))
	{
	  if (IsNPC (victim))
	    {
	      chprintln (ch, "Use act for NPCs.");
	      return;
	    }

	  flag = &victim->act;
	  flag_table = plr_flags;
	}
      else if (!str_prefix (arg3, "aff"))
	{
	  flag = &victim->affected_by;
	  flag_table = affect_flags;
	}
      else if (!str_prefix (arg3, "immunity"))
	{
	  flag = &victim->imm_flags;
	  flag_table = imm_flags;
	}
      else if (!str_prefix (arg3, "resist"))
	{
	  flag = &victim->res_flags;
	  flag_table = imm_flags;
	}
      else if (!str_prefix (arg3, "vuln"))
	{
	  flag = &victim->vuln_flags;
	  flag_table = imm_flags;
	}
      else if (!str_prefix (arg3, "form"))
	{
	  if (!IsNPC (victim))
	    {
	      chprintln (ch, "Form can't be set on PCs.");
	      return;
	    }

	  flag = &victim->form;
	  flag_table = form_flags;
	}
      else if (!str_prefix (arg3, "parts"))
	{
	  if (!IsNPC (victim))
	    {
	      chprintln (ch, "Parts can't be set on PCs.");
	      return;
	    }

	  flag = &victim->parts;
	  flag_table = part_flags;
	}
      else if (!str_prefix (arg3, "comm"))
	{
	  if (IsNPC (victim))
	    {
	      chprintln (ch, "Comm can't be set on NPCs.");
	      return;
	    }

	  flag = &victim->comm;
	  flag_table = comm_flags;
	}
      else
	{
	  chprintln (ch, "That's not an acceptable flag.");
	  return;
	}
    }
  else if (!str_prefix (arg1, "object"))
    {
      obj = get_obj_world (ch, arg2);
      if (obj == NULL)
	{
	  chprintln (ch, "You can't find that object.");
	  return;
	}


      if (!str_prefix (arg3, "extra"))
	{
	  flag = &obj->extra_flags;
	  flag_table = extra_flags;
	}
      else if (!str_prefix (arg3, "wear"))
	{
	  flag = &obj->wear_flags;
	  flag_table = wear_flags;
	}
      else
	{
	  chprintln (ch, "That's not an acceptable flag.");
	  return;
	}
    }
  else
    {
      do_flag (n_fun, ch, "");
      return;
    }

  old = *flag;

  if (type != '=')
    pnew = old;


  for (;;)
    {
      argument = one_argument (argument, word);

      if (NullStr (word))
	break;

      pos = flag_value (flag_table, word);

      if (pos == NO_FLAG)
	{
	  chprintln (ch, "That flag doesn't exist!");
	  return;
	}
      else
	SetBit (marked, pos);
    }

  for (pos = 0; flag_table[pos].name != NULL; pos++)
    {
      if (!flag_table[pos].settable && IsSet (old, flag_table[pos].bit))
	{
	  SetBit (pnew, flag_table[pos].bit);
	  continue;
	}

      if (IsSet (marked, flag_table[pos].bit))
	{
	  switch (type)
	    {
	    case '=':
	    case '+':
	      SetBit (pnew, flag_table[pos].bit);
	      break;
	    case '-':
	      RemBit (pnew, flag_table[pos].bit);
	      break;
	    default:
	      if (IsSet (pnew, flag_table[pos].bit))
		RemBit (pnew, flag_table[pos].bit);
	      else
		SetBit (pnew, flag_table[pos].bit);
	    }
	}
    }
  *flag = pnew;
  return;
}



#define STNM(flag)  #flag, flag


const struct flag_stat_type flag_stat_table[] = {

  {STNM (area_flags), false},
  {STNM (sex_flags), true},
  {STNM (exit_flags), false},
  {STNM (room_flags), false},
  {STNM (sector_flags), true},
  {STNM (type_flags), true},
  {STNM (extra_flags), false},
  {STNM (wear_flags), false},
  {STNM (act_flags), false},
  {STNM (plr_flags), false},
  {STNM (comm_flags), false},
  {STNM (affect_flags), false},
  {STNM (apply_flags), true},
  {STNM (wear_loc_flags), true},
  {STNM (wear_loc_strings), true},
  {STNM (container_flags), false},
  {STNM (mprog_flags), false},
  {STNM (rprog_flags), false},
  {STNM (oprog_flags), false},
  {STNM (info_flags), false},
  {STNM (form_flags), false},
  {STNM (part_flags), false},
  {STNM (ac_type), true},
  {STNM (size_flags), true},
  {STNM (position_flags), true},
  {STNM (off_flags), false},
  {STNM (imm_flags), false},
  {STNM (res_flags), false},
  {STNM (vuln_flags), false},
  {STNM (weapon_types), true},
  {STNM (weapon_flags), false},
  {STNM (apply_types), true},
  {STNM (desc_flags), false},
  {STNM (log_flags), true},
  {STNM (target_flags), true},
  {STNM (chan_types), true},
  {STNM (mud_flags), false},
  {STNM (autoset_types), true},
  {STNM (board_flags), false},
  {STNM (cmd_flags), false},
  {STNM (stat_types), true},
  {STNM (clan_flags), false},
  {STNM (signal_flags), false},
  {STNM (cmd_categories), true},
  {STNM (msp_types), true},
  {STNM (help_types), true},
  {STNM (to_flags), false},
  {STNM (color_attributes), true},
  {STNM (color_foregrounds), true},
  {STNM (color_backgrounds), true},
  {STNM (vt100_flags), false},
#ifndef DISABLE_I3
  {STNM (i3perm_names), true},
#endif
  {STNM (cmdline_flags), false},
  {STNM (ethos_types), true},
  {NULL, NULL, 0}
};


int
is_stat (FlagTable * flag_table)
{
  int flag;

  for (flag = 0; flag_stat_table[flag].structure; flag++)
    if (flag_stat_table[flag].structure == flag_table)
      return flag_stat_table[flag].stat;

  logf ("flag_table[0] == '%s': not in flag_stat_table", flag_table[0].name);
  return -1;
}


flag_t
flag_value (FlagTable * flag_table, const char *argument)
{
  FlagTable *f;
  flag_t marked;
  bool found = false;

  if (is_stat (flag_table))
    {
      if ((f = flag_lookup (argument, flag_table)) == NULL)
	return NO_FLAG;
      return f->bit;
    }

  marked = 0;


  for (;;)
    {
      char word[MAX_INPUT_LENGTH];

      argument = one_argument (argument, word);

      if (NullStr (word))
	break;

      if ((f = flag_lookup (word, flag_table)) != NULL)
	{
	  SetBit (marked, f->bit);
	  found = true;
	}
    }

  if (found)
    return marked;
  else
    return NO_FLAG;
}


const char *
flag_string (FlagTable * flag_table, flag_t bits)
{
  static char buf[3][MSL];
  static int cnt = 0;
  int flag;
  int stat;

  cnt = (cnt + 1) % 3;
  buf[cnt][0] = '\0';

  stat = is_stat (flag_table);
  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (!stat)
	{
	  if (IsSet (bits, flag_table[flag].bit))
	    {
	      strcat (buf[cnt], " ");
	      strcat (buf[cnt], flag_table[flag].name);
	    }
	}
      else if (flag_table[flag].bit == bits)
	{
	  strcat (buf[cnt], " ");
	  strcat (buf[cnt], flag_table[flag].name);
	  break;
	}
    }
  return (buf[cnt][0] != '\0') ? buf[cnt] + 1 : stat ? "unknown" : "none";
}


char *
flagtable_names (FlagTable * flag_table)
{
  static int i;
  static char buf[5][512];
  int flag;


  ++i, i %= 5;
  buf[i][0] = '\0';

  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      strcat (buf[i], " ");
      strcat (buf[i], flag_table[flag].name);
    }
  return buf[i] + 1;
}
