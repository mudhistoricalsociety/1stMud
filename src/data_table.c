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
#include "olc.h"
#include "recycle.h"
#include "magic.h"
#include "data_table.h"
#include "index.h"
#include "special.h"

TableShow_Fun (ts_class)
{
  int *cl = (int *) temp;

  if (*cl < 0 || *cl >= top_class)
    return "";
  else
    return FORMATF ("%.3s", class_table[*cl].name[0]);
}

TableShow_Fun (ts_charclass)
{
  CharData *ch = (CharData *) temp;

  if (!ch)
    return "";
  else
    return class_long (ch);
}

TableShow_Fun (ts_stance)
{
  CharData *ch = (CharData *) temp;
  static char buf[MSL];
  int i;

  buf[0] = NUL;
  for (i = 0; i < MAX_STANCE; i++)
    {
      if (stance_table[i].prereq[0] == STANCE_NONE)
	continue;

      if (stance_table[i].prereq[0] == STANCE_NORMAL)
	{
	  sprintf (buf + strlen (buf), " %s-%d%%", stance_table[i].name,
		   GetStance (ch, stance_table[i].stance));
	  continue;
	}
      else if (stance_table[i].prereq[0] == STANCE_CURRENT)
	{
	  sprintf (buf + strlen (buf), " %s-%s", stance_table[i].name,
		   stance_name (GetStance (ch, stance_table[i].stance)));
	  continue;
	}
      else if (GetStance (ch, stance_table[i].prereq[0]) >= 200
	       && GetStance (ch, stance_table[i].prereq[1]) >= 200)
	{
	  sprintf (buf + strlen (buf), " %s-%d%%", stance_table[i].name,
		   GetStance (ch, stance_table[i].stance));
	}
    }
  return buf[0] ? (buf + 1) : "";
}

TableShow_Fun (ts_objvals)
{
  ObjIndex *pObj = (ObjIndex *) temp;

  return show_obj_values (pObj);
}

TableShow_Fun (ts_editor)
{
  Descriptor *d = (Descriptor *) temp;

  if (d->editor != ED_NONE)
    return FORMATF ("%s [%s]", olc_ed_name (d), olc_ed_vnum (d));
  else
    return "";
}

TableShow_Fun (ts_stats)
{
  CharData *ch = (CharData *) temp;
  static char buf[MSL];
  int i;

  buf[0] = NUL;
  for (i = 0; i < STAT_MAX; i++)
    {
      sprintf (buf + strlen (buf), " %.3s [%d/%d]",
	       capitalize (stat_types[i].name), ch->perm_stat[i],
	       get_curr_stat (ch, i));
    }

  return buf[0] ? (buf + 1) : "";
}

TableShow_Fun (ts_weather)
{
  AreaData *pArea = (AreaData *) temp;

  return FORMATF ("Temp [%s], Precip [%s], Wind [%s]",
		  temp_settings[pArea->weather.climate_temp],
		  precip_settings[pArea->weather.climate_precip],
		  wind_settings[pArea->weather.climate_wind]);
}

TableShow_Fun (ts_hours)
{
  int *h = (int *) temp;

  return FORMATF ("%d", *h / HOUR);
}

TableRW_Fun (pgcn_rw)
{
  int **pgcn = (int **) temp;

  switch (type)
    {
    case act_read:
      *pgcn = (int *) index_lookup (*arg, channel_index, (void *) &gcn_null);
      return *pgcn != NULL;
    case act_write:
      *arg = index_name (*pgcn, channel_index, "gcn_null");
      return *pgcn != NULL;
    default:
      return false;
    }
}


TableRW_Fun (class_index_rw)
{
  int **pgci = (int **) temp;

  switch (type)
    {
    case act_read:
      *pgci = (int *) index_lookup (*arg, class_index, (void *) &class_null);
      return *pgci != NULL;
    case act_write:
      *arg = index_name (*pgci, class_index, "class_null");
      return *pgci != NULL;
    default:
      return false;
    }
}

TableRW_Fun (do_fun_rw)
{
  Do_F **fun = (Do_F **) temp;

  switch (type)
    {
    case act_write:
      *arg = index_name ((void *) (*fun), dofun_index, "do_null");
      return true;
    case act_read:
      *fun = (Do_F *) index_lookup (*arg, dofun_index, (void *) do_null);
      return true;
    default:
      return false;
    }
}

TableRW_Fun (cmd_name_rw)
{
  CmdData **cmd = (CmdData **) temp;

  switch (type)
    {
    case act_write:
      *arg = (*cmd)->name;
      return true;
    case act_read:
      *cmd = cmd_lookup (*arg);
      return true;
    default:
      return false;
    }
}


TableRW_Fun (attack_rw)
{
  int *r = (int *) temp;

  switch (type)
    {
    case act_read:
      {
	int ffg = attack_lookup (*arg);

	*r = Max (0, ffg);

	return (ffg != -1);
      }
    case act_write:
      {
	*arg = attack_table[Max (0, *r)].name;
	return (*r != -1);
      }
    default:
      return false;
    }
}

TableRW_Fun (pgsn_rw)
{
  int **pgsn = (int **) temp;

  switch (type)
    {
    case act_read:
      *pgsn = (int *) index_lookup (*arg, gsn_index, (void *) &gsn_null);
      return *pgsn != NULL;
    case act_write:
      *arg = index_name (*pgsn, gsn_index, "gsn_null");
      return *pgsn != NULL;
    default:
      return false;
    }
}

TableRW_Fun (spell_fun_rw)
{
  Spell_F **spfun = (Spell_F **) temp;

  switch (type)
    {
    case act_read:
      *spfun = (Spell_F *) index_lookup (*arg, spell_index,
					 (void *) spell_null);
      return *spfun != NULL;
    case act_write:
      *arg = index_name ((void *) (*spfun), spell_index, "spell_null");
      return *spfun != NULL;
    default:
      return false;
    }
}

TableRW_Fun (clan_rw)
{
  ClanData **c = (ClanData **) temp;

  switch (type)
    {
    case act_read:
      *c = clan_lookup (*arg);
      return *c != NULL;
    case act_write:
      *arg = *c == NULL ? "unknown" : (*c)->name;
      return *c != NULL;
    default:
      return false;
    }
}

DataTable cmd_data_table[] = {
  DTE (STRING, cmd, name, NULL, NULL, olded, cmdedit_name),
  DTE (FUNCTION_INT_TO_STR, cmd, do_fun, do_fun_rw, NULL, olded,
       cmdedit_dofun),
  DTE (INT_FLAGSTRING, cmd, position, position_flags, NULL, flag, NULL),
  DTE (INT, cmd, level, NULL, NULL, olded, cmdedit_level),
  DTE (FLAGSTRING, cmd, flags, cmd_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, cmd, log, log_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, cmd, category, cmd_categories, NULL, flag, NULL),
  DTE_OLC2 ("rearrange", NULL, olded, cmdedit_rearrange),
  DTE_END
};

DataTable skill_data_table[] = {
  DTE (STRING, skill, name, NULL, NULL, str, NULL),
  DTE (INT_ALLOC_ARRAY, skill, skill_level, &top_class, ts_class, olded,
       skedit_levels),
  DTE (INT_ALLOC_ARRAY, skill, rating, &top_class, ts_class, olded,
       skedit_ratings),
  DTE (FUNCTION_INT_TO_STR, skill, spell_fun, spell_fun_rw, NULL,
       olded, skedit_spellfun),
  DTE (INT_FLAGSTRING, skill, target, target_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, skill, minimum_position, position_flags, NULL,
       flag,
       NULL),
  DTE (FUNCTION_INT_TO_STR, skill, pgsn, pgsn_rw,
       &gsn_null, olded, skedit_gsn),
  DTE (INT, skill, min_mana, NULL, NULL, int, NULL),
  DTE (INT, skill, beats, NULL, NULL, int, NULL),
  DTE (STRING, skill, noun_damage, NULL, NULL, str, NULL),
  DTE (STRING, skill, msg_off, NULL, NULL, str, NULL),
  DTE (STRING, skill, msg_obj, NULL, NULL, str, NULL),
  DTE (MSP, skill, sound, NULL, NULL, sound, NULL),
  DTE_END
};

DataTable race_data_table[] = {
  DTE (STRING, race, name, NULL, NULL, olded, raedit_name),
  DTE (STRING, race, description, NULL, NULL, str, NULL),
  DTE (BOOL, race, pc_race, NULL, NULL, bool, NULL),
  DTE (FLAGVECTOR, race, act, act_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, aff, affect_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, off, off_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, imm, imm_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, res, res_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, vuln, vuln_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, form, form_flags, NULL, flag, NULL),
  DTE (FLAGVECTOR, race, parts, part_flags, NULL, flag, NULL),
  DTE (INT, race, points, NULL, NULL, int, NULL),
  DTE (INT_ALLOC_ARRAY, race, class_mult, &top_class, ts_class,
       olded, raedit_classx),
  DTE (STRING_ARRAY_NULL, race, skills, 5, NULL, olded,
       raedit_skills),
  DTE (INT_ARRAY, race, stats, STAT_MAX, 0, stats, NULL),
  DTE (INT_ARRAY, race, max_stats, STAT_MAX, 0, stats, NULL),
  DTE (INT_FLAGSTRING, race, size, size_flags, NULL, flag, NULL),
  DTE_OLC2 ("copy", NULL, olded, raedit_copy),
  DTE_END
};

DataTable group_data_table[] = {
  DTE (STRING, group, name, NULL, NULL, str, validate_groupname),
  DTE (INT_ALLOC_ARRAY, group, rating, &top_class, ts_class, olded,
       gredit_ratings),
  DTE (STRING_ARRAY_NULL, group, spells, MAX_IN_GROUP, NULL, olded,
       gredit_spells),
  DTE_END
};

DataTable class_data_table[] = {
  DTE (STRING_ARRAY, class, name, MAX_REMORT, NULL, olded, NULL),
  DTE (STRING, class, description, NULL, NULL, str, NULL),
  DTE (FUNCTION_INT_TO_STR, class, index, class_index_rw,
       &class_null, olded, cledit_index),
  DTE (INT, class, attr_prime, stat_types, NULL, flag, NULL),
  DTE (LONG, class, weapon, NULL, NULL, vnum, validate_weapon),
  DTE (INT, class, skill_adept, NULL, NULL, int, validate_adept),
  DTE (INT, class, thac0_00, NULL, NULL, int, NULL),
  DTE (INT, class, thac0_32, NULL, NULL, int, NULL),
  DTE (INT, class, hp_min, NULL, NULL, int, NULL),
  DTE (INT, class, hp_max, NULL, NULL, int, NULL),
  DTE (BOOL, class, fMana, NULL, NULL, bool, NULL),
  DTE (STRING, class, base_group, NULL, NULL, desc, validate_groupname),
  DTE (STRING, class, default_group, NULL, NULL, desc,
       validate_groupname),
  DTE_END
};

DataTable social_data_table[] = {
  DTE (STRING, social, name, NULL, NULL, olded, sedit_name),
  DTE (STRING, social, char_no_arg, NULL, NULL, str, NULL),
  DTE (STRING, social, others_no_arg, NULL, NULL, str, NULL),
  DTE (STRING, social, char_found, NULL, NULL, str, NULL),
  DTE (STRING, social, others_found, NULL, NULL, str, NULL),
  DTE (STRING, social, vict_found, NULL, NULL, str, NULL),
  DTE (STRING, social, char_not_found, NULL, NULL, str, NULL),
  DTE (STRING, social, char_auto, NULL, NULL, str, NULL),
  DTE (STRING, social, others_auto, NULL, NULL, str, NULL),
  DTE_END
};

DataTable gquest_data_table[] = {
  DTE (INT, gquest, mob_count, NULL, NULL, int, NULL),
  DTE (STRING, gquest, who, NULL, NULL, str, NULL),
  DTE (INT, gquest, timer, NULL, NULL, int, NULL),
  DTE (INT, gquest, involved, NULL, NULL, int, NULL),
  DTE (INT, gquest, qpoints, NULL, NULL, int, NULL),
  DTE (INT, gquest, gold, NULL, NULL, int, NULL),
  DTE (INT, gquest, minlevel, NULL, NULL, int, validate_level),
  DTE (INT, gquest, maxlevel, NULL, NULL, int, validate_level),
  DTE (INT, gquest, running, NULL, NULL, int, NULL),
  DTE (INT, gquest, cost, NULL, NULL, int, NULL),
  DTE_SHOW (LONG_ALLOC_ARRAY, gquest, mobs, &gquest_info.mob_count, NULL,
	    NULL),
  DTE_END
};

DataTable deity_data_table[] = {
  DTE (STRING, deity, name, NULL, NULL, str, NULL),
  DTE (STRING, deity, desc, NULL, NULL, str, NULL),
  DTE (STRING, deity, skillname, NULL, NULL, str, NULL),
  DTE (INT_FLAGSTRING, deity, ethos, ethos_types, NULL, flag, NULL),
  DTE_END
};

DataTable war_data_table[] = {
  DTE (STRING, war, who, NULL, NULL, str, NULL),
  DTE (INT, war, min_level, NULL, NULL, int, validate_level),
  DTE (INT, war, max_level, NULL, NULL, int, validate_level),
  DTE (INT, war, inwar, NULL, NULL, int, NULL),
  DTE (INT, war, wartype, NULL, NULL, int, NULL),
  DTE (INT, war, timer, NULL, NULL, int, NULL),
  DTE (INT, war, status, NULL, NULL, int, NULL),
  DTE_END
};

DataTable channel_data_table[] = {
  DTE (FUNCTION_INT_TO_STR, channel, index, pgcn_rw, NULL, olded,
       chanedit_gcn),
  DTE (STRING, channel, color, NULL, NULL, olded, chanedit_color),
  DTE (INT, channel, custom_color, -1, NULL, olded, chanedit_color),
  DTE (STRING, channel, format, NULL, NULL, str, NULL),
  DTE (FLAGSTRING, channel, bit, comm_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, channel, spec_flag, chan_types, NULL, flag, NULL),
  DTE (INT, channel, page_length, NULL, NULL, int, NULL),
  DTE (STRING, channel, name, NULL, NULL, str, NULL),
  DTE (STRING, channel, description, NULL, NULL, str, NULL),
  DTE_END
};

DataTable mud_data_table[] = {
  DTE (STRING, mud, name, NULL, NULL, str, NULL),
  DTE_SHOW (INT, mud, unique_id, NULL, NULL, NULL),
  DTE (STRING, mud, bind_ip_address, NULL, NULL, int, validate_ip),
  DTE (INT, mud, default_port, NULL, NULL, int, validate_port),
  DTE (INT, mud, webport_offset, NULL, NULL, int, validate_port),
  DTE (STRING, mud, login_prompt, NULL, NULL, str, NULL),
  DTE (INT, mud, min_save_lvl, NULL, NULL, int, NULL),
  DTE_OLC2 ("reset", NULL, olded, mudedit_reset),
  DTE (FLAGVECTOR, mud, mud_flags, mud_flags, NULL, flag, NULL),
  DTE (INT, mud, pulsepersec, NULL, NULL, int, NULL),
  DTE (INT, mud, arena, NULL, NULL, int, NULL),
  DTE (INT, mud, share_value, NULL, NULL, int, NULL),
  DTE (INT, mud, rand_factor, NULL, NULL, int, NULL),
  DTE (INT, mud, weath_unit, NULL, NULL, int, NULL),
  DTE (INT, mud, max_vector, NULL, NULL, int, NULL),
  DTE (INT, mud, climate_factor, NULL, NULL, int, NULL),
  DTE_SHOW (TIME, mud, last_copyover, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.logins, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.quests, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.qcomplete, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.levels, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.newbies, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.deletions, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.mobdeaths, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.auctions, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.aucsold, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.pdied, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.pkill, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.remorts, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.wars, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.gquests, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.connections, NULL, NULL, NULL),
  DTE_SHOW (INT, mud, stats.online, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.web_requests, NULL, NULL, NULL),
  DTE_SHOW (LONG, mud, stats.chan_msgs, NULL, NULL, NULL),
  DTE_SHOW (TIME, mud, stats.lastupdate, NULL, NULL, NULL),
  DTE (TIME, mud, longest_uptime, NULL, NULL, time, NULL),
  DTE_SHOW (LONG, mud, stats.version, NULL, NULL, NULL),
  DTE (INT, mud, bonus.mod, NULL, NULL, int, NULL),
  DTE (INT, mud, bonus.time, -1, NULL, int, NULL),
  DTE (INT, mud, bonus.status, BONUS_OFF, NULL, int, NULL),
  DTE (STRING, mud, bonus.msg, NULL, NULL, str, NULL),
  DTE (INT, mud, max_points, NULL, NULL, int, NULL),
  DTE (INT, mud, group_lvl_limit, NULL, NULL, int, NULL),
  DTE (INT, mud, pcdam, NULL, NULL, int, NULL),
  DTE (INT, mud, mobdam, NULL, NULL, int, NULL),
  DTE (FLAGVECTOR, mud, disabled_signals, signal_flags, NULL, flag, NULL),
  DTE_SHOW (FLAGVECTOR, mud, cmdline_options, cmdline_flags, NULL, NULL),
  DTE_END
};

DataTable song_data_table[] = {
  DTE (STRING, song, name, NULL, NULL, str, NULL),
  DTE (STRING, song, group, NULL, NULL, str, NULL),
  DTE_SHOW (INT, song, lines, NULL, NULL, NULL),
  DTE (STRING_ARRAY_NULL, song, lyrics, MAX_LINES, NULL, olded,
       songedit_lyrics),
  DTE_END
};

DataTable disabled_data_table[] = {
  DTE (STRING, disabled, disabled_by, NULL, NULL, str, NULL),
  DTE (STRING, disabled, disabled_for, NULL, NULL, str, NULL),
  DTE (INT, disabled, level, NULL, NULL, int, validate_level),
  DTE_SHOW (FUNCTION_INT_TO_STR, disabled, command, do_fun_rw, NULL,
	    NULL),
  DTE_END
};

DataTable note_data_table[] = {
  DTE (STRING, note, sender, NULL, NULL, str, NULL),
  DTE (STRING, note, date, NULL, NULL, str, NULL),
  DTE (TIME, note, date_stamp, NULL, NULL, time, NULL),
  DTE (TIME, note, expire, NULL, NULL, time, NULL),
  DTE (STRING, note, to_list, NULL, NULL, str, NULL),
  DTE (STRING, note, subject, NULL, NULL, str, NULL),
  DTE (STRING, note, text, NULL, 1, str, NULL),
  DTE_END
};

DataTable char_data_table[] = {
  DTE (STRING, char, name, NULL, NULL, str, NULL),
  DTE (TIME, char, id, NULL, NULL, time, NULL),
  DTE (STRING, char, short_descr, NULL, NULL, str, NULL),
  DTE (STRING, char, long_descr, NULL, 1, str, NULL),
  DTE (STRING, char, description, NULL, 1, str, NULL),
  DTE (STRING, char, prompt, NULL, NULL, str, NULL),
  DTE (STRING, char, prefix, NULL, NULL, str, NULL),
  DTE (INT, char, group, NULL, NULL, int, NULL),
  DTE (INT, char, saving_throw, NULL, NULL, int, NULL),
  DTE (INT_FLAGSTRING, char, sex, sex_flags, -1, flag, NULL),
  DTE (FUNCTION, char, Class, ts_charclass, NULL, mclass, NULL),
  DTE (INT, char, timer, NULL, NULL, int, NULL),
  DTE (INT, char, wait, NULL, NULL, int, NULL),
  DTE (INT, char, daze, NULL, NULL, int, NULL),
  DTE (INT, char, carry_weight, NULL, NULL, int, NULL),
  DTE (INT, char, carry_number, NULL, NULL, int, NULL),
  DTE (DICE, char, damage, NULL, NULL, dice, NULL),
  DTE (INT, char, dam_type, NULL, NULL, plookup, attack_lookup),
  DTE (RACE, char, race, DATA_NAME, NULL, race, NULL),
  DTE (INT, char, level, NULL, NULL, int, validate_level),
  DTE (INT, char, trust, NULL, NULL, int, NULL),
  DTE (INT, char, lines, NULL, NULL, int, NULL),
  DTE (LONG, char, hit, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, max_hit, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, mana, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, max_mana, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, move, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, max_move, NULL, NULL, long, validate_hmv),
  DTE (LONG, char, gold, NULL, NULL, long, NULL),
  DTE (LONG, char, silver, NULL, NULL, long, NULL),
  DTE (CHARDATA, char, reply, DATA_NAME, NULL, getchar, NULL),
  DTE (CHARDATA, char, pet, DATA_NAME, NULL, getchar, NULL),
  DTE (CHARDATA, char, mprog_target, DATA_NAME, NULL, getchar, NULL),
  DTE (INT, char, mprog_delay, NULL, NULL, int, NULL),
  DTE (FUNCTION, char, stance, ts_stance, NULL,
       stance, NULL),
  DTE (FLAGSTRING, char, info_settings, info_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, act, act_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, comm, comm_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, imm_flags, imm_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, res_flags, res_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, vuln_flags, vuln_flags, NULL, flag, NULL),
  DTE (INT, char, invis_level, NULL, NULL, int, validate_level),
  DTE (INT, char, incog_level, NULL, NULL, int, validate_level),
  DTE (FLAGSTRING, char, affected_by, affect_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char, position, position_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char, start_pos, position_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char, default_pos, position_flags, NULL, flag,
       NULL),
  DTE (INT, char, practice, NULL, NULL, int, NULL),
  DTE (INT, char, train, NULL, NULL, int, NULL),
  DTE (INT, char, alignment, NULL, NULL, int, validate_align),
  DTE (INT, char, hitroll, NULL, NULL, int, NULL),
  DTE (INT, char, damroll, NULL, NULL, int, NULL),
  DTE (INT_ARRAY, char, armor, 4, NULL, array, NULL),
  DTE (INT, char, wimpy, NULL, NULL, int, NULL),
  DTE (LONG, char, exp, NULL, NULL, long, NULL),
  DTE (INT, char, rank, NULL, NULL, int, NULL),
  DTE (TIME, char, logon, NULL, NULL, time, NULL),
  DTE (FUNCTION, char, perm_stat, ts_stats, STAT_MAX, stats, NULL),
  DTE (FLAGSTRING, char, form, form_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char, parts, part_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char, size, size_flags, NULL, flag, NULL),
  DTE (STRING, char, material, "unknown", NULL, str, NULL),
  DTE (FLAGSTRING, char, off_flags, off_flags, NULL, flag, NULL),
  DTE (DEITY, char, deity, DATA_NAME, NULL, deity, NULL),
  DTE (CHARDATA, char, master, DATA_NAME, NULL, getchar, NULL),
  DTE (CHARDATA, char, leader, DATA_NAME, NULL, getchar, NULL),
  DTE (CHARDATA, char, fighting, DATA_NAME, NULL, getchar, NULL),
  DTE (CHARDATA, char, hunting, DATA_NAME, NULL, getchar, NULL),
  DTE (SPEC_FUN, char, spec_fun, NULL, NULL, spec, NULL),
  DTE_SHOW (AREA, char, zone, DATA_NAME, NULL, NULL),
  DTE_SHOW (ROOM, char, in_room, DATA_NAME, NULL, NULL),
  DTE_SHOW (MOBINDEX, char, pIndexData, NULL, NULL, NULL),
  DTE_SHOW (PCDATA, char, pcdata, DATA_STAT, NULL, NULL),
  DTE_SHOW (DESC, char, desc, DATA_STAT, NULL, NULL),
  DTE_END
};

DataTable pcdata_data_table[] = {
  DTE (STRING, pcdata, bamfin, NULL, NULL, str, NULL),
  DTE (STRING, pcdata, bamfout, NULL, NULL, str, NULL),
  DTE (STRING, pcdata, title, NULL, NULL, str, NULL),
  DTE (STRING, pcdata, who_descr, NULL, NULL, str, NULL),
  DTE (STRING, pcdata, afk_msg, NULL, NULL, str, NULL),
  DTE (INT_FUNCTION, pcdata, played, ts_hours, NULL, int, NULL),
  DTE (INT_ARRAY, pcdata, condition, COND_MAX, NULL, array, NULL),
  DTE (INT, pcdata, points, NULL, NULL, int, NULL),
  DTE (LONG_ARRAY, pcdata, home, MAX_HOME_VNUMS, -1, larray, NULL),
  DTE (INT, pcdata, quest.points, NULL, NULL, int, NULL),
  DTE (INT, pcdata, quest.time, NULL, NULL, int, NULL),
  DTE (LONG, pcdata, gold_bank, NULL, NULL, long, NULL),
  DTE (INT, pcdata, shares, NULL, NULL, int, NULL),
  DTE (INT, pcdata, trivia, NULL, NULL, int, NULL),
  DTE (INT, pcdata, security, NULL, NULL, int, NULL),
  DTE (INT, pcdata, timezone, -1, NULL, int, NULL),
  DTE (CLAN, pcdata, clan, DATA_NAME, NULL, clan, NULL),
  DTE (CLAN, pcdata, invited, DATA_NAME, NULL, clan, NULL),
  DTE (FLAGSTRING, pcdata, vt100, vt100_flags, NULL, flag, NULL),
  DTE_END
};

DataTable descriptor_data_table[] = {
  DTE (STRING, descriptor, host, NULL, NULL, str, NULL),
  DTE_SHOW (INT, descriptor, descriptor, NULL, NULL, NULL),
  DTE_SHOW (INT, descriptor, connected, NULL, NULL, NULL),
  DTE_SHOW (TIME, descriptor, connect_time, NULL, NULL, NULL),
  DTE_SHOW (INT, descriptor, repeat, NULL, NULL, NULL),
  DTE_SHOW (FUNCTION, descriptor, editor, ts_editor, NULL, NULL),
  DTE (FLAGSTRING, descriptor, desc_flags, desc_flags, NULL, flag, NULL),
  DTE (INT, descriptor, scr_width, 80, NULL, int, NULL),
  DTE (INT, descriptor, scr_height, 24, NULL, int, NULL),
  DTE_SHOW (STRING_SIZE, descriptor, ttype[0], NULL, 60, NULL),
  DTE_SHOW (DOUBLE, descriptor, imp_vers, NULL, 2, NULL),
  DTE_SHOW (DOUBLE, descriptor, pueblo_vers, NULL, 2, NULL),
#ifndef DISABLE_MCCP
  DTE_SHOW (INT, descriptor, mccp_version, NULL, NULL, NULL),
#endif
  DTE_SHOW (FLOAT, descriptor, mxp.mxp_ver, NULL, 2, NULL),
  DTE (STRING, descriptor, mxp.client, NULL, NULL, str, NULL),
  DTE_SHOW (FLOAT, descriptor, mxp.client_ver, NULL, 2, NULL),
  DTE_SHOW (STRING_SIZE, descriptor, portal.version[0], NULL, 20, NULL),
  DTE_SHOW (INT, descriptor, portal.keycode, NULL, NULL, NULL),
#ifndef DISABLE_MCCP
  DTE (INT, descriptor, bytes_compressed, NULL, NULL, int, NULL),
#endif
  DTE (INT, descriptor, bytes_normal, NULL, NULL, int, NULL),
  DTE_SHOW (CHARDATA, descriptor, original, NULL, NULL, NULL),
  DTE_SHOW (CHARDATA, descriptor, character, NULL, NULL, NULL),
  DTE_END
};

DataTable char_index_data_table[] = {
  DTE_SHOW (AREA, char_index, area, DATA_NAME, NULL, NULL),
  DTE_SHOW (LONG, char_index, vnum, NULL, NULL, NULL),
  DTE (LONG, char_index, group, NULL, NULL, olded, medit_group),
  DTE_SHOW (INT, char_index, count, NULL, NULL, NULL),
  DTE_SHOW (LONG, char_index, kills, NULL, NULL, NULL),
  DTE_SHOW (LONG, char_index, deaths, NULL, NULL, NULL),
  DTE (STRING, char_index, player_name, NULL, NULL, str, NULL),
  DTE (STRING, char_index, short_descr, NULL, NULL, str, NULL),
  DTE (STRING, char_index, long_descr, NULL, 1, str, NULL),
  DTE (STRING, char_index, description, NULL, 1, desc, NULL),
  DTE (FLAGSTRING, char_index, act, act_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char_index, affected_by, affect_flags, NULL, flag,
       NULL),
  DTE (INT, char_index, alignment, NULL, NULL, int, validate_align),
  DTE (INT, char_index, level, NULL, NULL, int, validate_level),
  DTE (DICE, char_index, hit, NULL, NULL, dice, NULL),
  DTE (DICE, char_index, mana, NULL, NULL, dice, NULL),
  DTE (DICE, char_index, damage, NULL, NULL, dice, NULL),
  DTE (INT, char_index, hitroll, NULL, NULL, int, NULL),
  DTE (INT_ARRAY, char_index, ac, MAX_AC, NULL, array, NULL),
  DTE (INT, char_index, dam_type, NULL, NULL, plookup, attack_lookup),
  DTE (FLAGSTRING, char_index, off_flags, off_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char_index, imm_flags, imm_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char_index, res_flags, res_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char_index, vuln_flags, vuln_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char_index, start_pos, position_flags, NULL, flag,
       NULL),
  DTE (INT_FLAGSTRING, char_index, default_pos, position_flags, NULL,
       flag,
       NULL),
  DTE (INT_FLAGSTRING, char_index, sex, sex_flags, NULL, flag, NULL),
  DTE (RACE, char_index, race, NULL, NULL, race, NULL),
  DTE (LONG, char_index, wealth, NULL, NULL, long, NULL),
  DTE (FLAGSTRING, char_index, form, form_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, char_index, parts, part_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, char_index, size, size_flags, NULL, flag, NULL),
  DTE (STRING, char_index, material, "unknown", NULL, str, NULL),
  DTE (FLAGSTRING, char_index, mprog_flags, mprog_flags, NULL, flag,
       NULL),
  DTE (INT, char_index, random, NULL, NULL, int, validate_level),
  DTE (INT_FLAGSTRING, char_index, autoset, autoset_types, NULL, flag,
       validate_autoset),
  DTE (SPEC_FUN, char_index, spec_fun, NULL, NULL, spec, NULL),
  DTE (SHOP, char_index, pShop, NULL, NULL, shop, NULL),
  DTE_LIST2 (PROG_LIST, char_index, mprog_first, prog, FIELD_ROOM, NULL),
  DTE_OLC2 ("autoeasy", NULL, olded, medit_autoeasy),
  DTE_OLC2 ("autoset", NULL, olded, medit_autoset),
  DTE_OLC2 ("autohard", NULL, olded, medit_autohard),
  DTE_OLC2 ("copy", NULL, olded, medit_copy),
  DTE_END
};

DataTable shop_data_table[] = {
  DTE_SHOW (LONG, shop, keeper, NULL, NULL, NULL),
  DTE_SHOW (INT_ARRAY, shop, buy_type, NULL, NULL, NULL),
  DTE (INT, shop, profit_buy, NULL, NULL, int, NULL),
  DTE (INT, shop, profit_sell, NULL, NULL, int, NULL),
  DTE (INT, shop, open_hour, NULL, NULL, int, NULL),
  DTE (INT, shop, close_hour, NULL, NULL, int, NULL),
  DTE_END
};

DataTable room_index_data_table[] = {
  DTE_SHOW (AREA, room_index, area, DATA_NAME, NULL, NULL),
  DTE (STRING, room_index, name, NULL, NULL, str, NULL),
  DTE (STRING, room_index, description, NULL, 1, desc, NULL),
  DTE (STRING, room_index, owner, NULL, NULL, str, NULL),
  DTE_SHOW (LONG, room_index, vnum, NULL, NULL, NULL),
  DTE (FLAGSTRING, room_index, room_flags, room_flags, NULL, flag, NULL),
  DTE (INT, room_index, light, NULL, NULL, int, NULL),
  DTE (INT_FLAGSTRING, room_index, sector_type, sector_flags, NULL, flag,
       NULL),
  DTE (INT_FUNCTION, room_index, guild, ts_class, -1, nlookup,
       class_lookup),
  DTE (INT, room_index, heal_rate, 100, NULL, int, NULL),
  DTE (INT, room_index, mana_rate, 100, NULL, int, NULL),
  DTE (MSP, room_index, sound, NULL, NULL, sound, NULL),
  DTE_LIST (EXDESC, room_index, ed_first, ed, FIELD_ROOM, NULL),
  DTE_SHOW (CHARDATA, room_index, person_first, DATA_LIST, NULL, NULL),
  DTE_SHOW (OBJ, room_index, content_first, DATA_LIST, NULL, NULL),
  DTE_SHOW (RESET_DATA, room_index, reset_first, DATA_LIST, NULL, NULL),
  DTE_OLC2 ("resets", NULL, docomm,
	    do_resets),
  DTE_LIST2 (PROG_LIST, room_index, rprog_first, prog, FIELD_ROOM, NULL),
  DTE (CHARDATA, room_index, rprog_target, DATA_NAME, NULL, getchar,
       NULL),
  DTE (FLAGSTRING, room_index, rprog_flags, rprog_flags, NULL, flag,
       NULL),
  DTE_SHOW (EXIT_DATA, room_index, exit, MAX_DIR, NULL, NULL),
  DTE_OLC2 ("format", NULL, olded, redit_format),
  DTE_OLC ("north", room_index, exit[DIR_NORTH], DIR_NORTH, NULL,
	   direction,
	   NULL),
  DTE_OLC ("south", room_index, exit[DIR_SOUTH], DIR_SOUTH, NULL,
	   direction,
	   NULL),
  DTE_OLC ("east", room_index, exit[DIR_EAST], DIR_EAST, NULL, direction,
	   NULL),
  DTE_OLC ("west", room_index, exit[DIR_WEST], DIR_WEST, NULL, direction,
	   NULL),
  DTE_OLC ("up", room_index, exit[DIR_UP], DIR_UP, NULL, direction, NULL),
  DTE_OLC ("down", room_index, exit[DIR_DOWN], DIR_DOWN, NULL, direction,
	   NULL),
  DTE_OLC2 ("snake", NULL, olded, redit_snake),
  DTE_OLC2 ("olist", NULL, olded, redit_olist),
  DTE_OLC2 ("mlist", NULL, olded, redit_mlist),
  DTE_OLC2 ("mreset", NULL, olded, redit_mreset),
  DTE_OLC2 ("oreset", NULL, olded, redit_oreset),
  DTE_OLC2 ("mshow", NULL, olded, redit_mshow),
  DTE_OLC2 ("oshow", NULL, olded, redit_oshow),
  DTE_OLC2 ("purge", NULL, docomm, do_purge),
  DTE_OLC2 ("copy", NULL, olded, redit_copy),
  DTE_END
};

DataTable exit_data_table[] = {
  DTE_SHOW (ROOM, exit, u1.to_room, NULL, NULL, NULL),
  DTE (FLAGSTRING, exit, exit_info, exit_flags, NULL, flag, NULL),
  DTE (LONG, exit, key, NULL, NULL, vnum, NULL),
  DTE (STRING, exit, keyword, NULL, NULL, str, NULL),
  DTE (STRING, exit, description, NULL, 1, desc, NULL),
  DTE_END
};

DataTable obj_data_table[] = {
  DTE_SHOW (OBJ, obj, in_obj, NULL, NULL, NULL),
  DTE_SHOW (OBJ, obj, on, DATA_NAME, NULL, NULL),
  DTE_SHOW (CHARDATA, obj, carried_by, NULL, NULL, NULL),
  DTE_SHOW (ROOM, obj, in_room, NULL, NULL, NULL),
  DTE_SHOW (CHARDATA, obj, oprog_target, NULL, NULL, NULL),
  DTE_SHOW (INT, obj, oprog_delay, NULL, NULL, NULL),
  DTE (BOOL, obj, enchanted, NULL, NULL, bool, NULL),
  DTE (STRING, obj, owner, NULL, NULL, str, NULL),
  DTE (STRING, obj, name, NULL, NULL, str, NULL),
  DTE (STRING, obj, short_descr, NULL, NULL, str, NULL),
  DTE (STRING, obj, description, NULL, NULL, str, NULL),
  DTE (INT_FLAGSTRING, obj, item_type, type_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, obj, extra_flags, extra_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, obj, wear_flags, wear_flags, NULL, flag, NULL),
  DTE (INT_FLAGSTRING, obj, wear_loc, wear_loc_flags, NULL, flag, NULL),
  DTE (INT, obj, weight, NULL, NULL, int, NULL),
  DTE (LONG, obj, cost, NULL, NULL, long, NULL),
  DTE (INT, obj, level, NULL, NULL, int, validate_level),
  DTE (INT, obj, condition, 100, NULL, int, NULL),
  DTE (INT, obj, timer, NULL, NULL, int, NULL),
  DTE (STRING, obj, material, "unknown", NULL, str, NULL),
  DTE_LIST (EXDESC, obj, ed_first, ed, FIELD_OBJ, NULL),
  DTE (LONG_ARRAY, obj, value, 5, NULL, larray, NULL),
  DTE_SHOW (OBJINDEX, obj, pIndexData, NULL, NULL, NULL),
  DTE_OLC2 ("copy", NULL, olded, oedit_copy),
  DTE_END
};

DataTable obj_index_data_table[] = {
  DTE_SHOW (AREA, obj_index, area, NULL, NULL, NULL),
  DTE (FLAGSTRING, obj_index, oprog_flags, oprog_flags, NULL, flag, NULL),
  DTE (STRING, obj_index, name, NULL, NULL, str, NULL),
  DTE (STRING, obj_index, short_descr, NULL, NULL, str, NULL),
  DTE (STRING, obj_index, description, NULL, NULL, str, NULL),
  DTE (LONG, obj_index, vnum, NULL, NULL, vnum, NULL),
  DTE (INT_FLAGSTRING, obj_index, item_type, type_flags, NULL, flag,
       NULL),
  DTE (FLAGSTRING, obj_index, extra_flags, extra_flags, NULL, flag, NULL),
  DTE (FLAGSTRING, obj_index, wear_flags, wear_flags, NULL, flag, NULL),
  DTE (INT, obj_index, level, NULL, NULL, int, validate_level),
  DTE (INT, obj_index, condition, 100, NULL, int, NULL),
  DTE (INT, obj_index, count, NULL, NULL, int, NULL),
  DTE (INT, obj_index, weight, NULL, NULL, int, NULL),
  DTE (LONG, obj_index, cost, NULL, NULL, long, NULL),
  DTE (STRING, obj_index, material, "unknown", NULL, str, NULL),
  DTE_SHOW (FUNCTION, obj_index, value, ts_objvals, NULL, 1),
  DTE_OLC2 ("v0", NULL, value, 0),
  DTE_OLC2 ("v1", NULL, value, 1),
  DTE_OLC2 ("v2", NULL, value, 2),
  DTE_OLC2 ("v3", NULL, value, 3),
  DTE_OLC2 ("v4", NULL, value, 4),
  DTE_LIST (EXDESC, obj_index, ed_first, ed, FIELD_OBJINDEX, NULL),
  DTE_LIST2 (PROG_LIST, obj_index, oprog_first, prog, FIELD_OBJINDEX,
	     NULL),
  DTE_LIST2 (AFFECT, obj_index, affect_first, affect, FIELD_OBJINDEX,
	     NULL),
  DTE_OLC ("addapply", obj_index, affect_first, NULL, FIELD_OBJINDEX,
	   addapply, NULL),
  DTE_OLC2 ("autoweapon", NULL, olded, oedit_autoweapon),
  DTE_OLC2 ("autoarmor", NULL, olded, oedit_autoarmor),
  DTE_END
};

DataTable area_data_table[] = {
  DTE (STRING, area, file_name, NULL, NULL, olded, aedit_file),
  DTE (STRING, area, name, NULL, NULL, str, NULL),
  DTE (STRING, area, credits, NULL, NULL, str, NULL),
  DTE (INT, area, age, NULL, NULL, int, NULL),
  DTE (INT, area, nplayer, NULL, NULL, int, NULL),
  DTE (LONG, area, min_vnum, NULL, NULL, olded, aedit_lvnum),
  DTE (LONG, area, max_vnum, NULL, NULL, olded, aedit_uvnum),
  DTE (BOOL, area, empty, false, NULL, bool, NULL),
  DTE (STRING, area, builders, NULL, NULL, olded, aedit_builder),
  DTE (INT, area, vnum, NULL, NULL, olded, aedit_vnum),
  DTE_OLC2 ("reset", NULL, olded, aedit_reset),
  DTE (FLAGSTRING, area, area_flags, area_flags, NULL, flag, NULL),
  DTE (INT, area, security, NULL, NULL, int, NULL),
  DTE (INT, area, min_level, NULL, NULL, int, validate_level),
  DTE (LONG, area, recall, NULL, NULL, vnum, validate_roomvnum),
  DTE (INT, area, max_level, MAX_LEVEL, NULL, int, validate_level),
  DTE_SHOW (STRING, area, lvl_comment, NULL, NULL, NULL),
  DTE (STRING, area, resetmsg, NULL, NULL, str, NULL),
  DTE_SHOW (INT, area, version, AREA_VERSION, NULL, NULL),
  DTE (FUNCTION, area, weather, ts_weather, NULL, olded, aedit_climate),
  DTE_SHOW (LONG, area, deaths, NULL, NULL, NULL),
  DTE_SHOW (LONG, area, kills, NULL, NULL, NULL),
  DTE (CLAN, area, clan, NULL, NULL, clan, NULL),
  DTE (MSP, area, sound, NULL, NULL, sound, NULL),
  DTE_OLC2 ("copy", NULL, olded, aedit_copy),
  DTE_END
};

DataTable mprog_data_table[] = {
  DTE_SHOW (LONG, mprog, vnum, NULL, NULL, NULL),
  DTE (STRING, mprog, name, NULL, NULL, str, NULL),
  DTE (STRING, mprog, code, NULL, 1, desc, NULL),
  DTE (BOOL, mprog, disabled, NULL, NULL, bool, NULL),
  DTE (STRING, mprog, disabled_text, NULL, NULL, str, NULL),
  DTE_SHOW (AREA, mprog, area, DATA_NAME, NULL, NULL),
  DTE_OLC2 ("copy", NULL, olded, mpedit_copy),
  DTE_END
};

DataTable oprog_data_table[] = {
  DTE_SHOW (LONG, oprog, vnum, NULL, NULL, NULL),
  DTE (STRING, oprog, name, NULL, NULL, str, NULL),
  DTE (STRING, oprog, code, NULL, 1, desc, NULL),
  DTE (BOOL, oprog, disabled, NULL, NULL, bool, NULL),
  DTE (STRING, oprog, disabled_text, NULL, NULL, str, NULL),
  DTE_SHOW (AREA, oprog, area, DATA_NAME, NULL, NULL),
  DTE_OLC2 ("copy", NULL, olded, opedit_copy),
  DTE_END
};

DataTable rprog_data_table[] = {
  DTE_SHOW (LONG, rprog, vnum, NULL, NULL, NULL),
  DTE (STRING, rprog, name, NULL, NULL, str, NULL),
  DTE (STRING, rprog, code, NULL, 1, desc, NULL),
  DTE (BOOL, rprog, disabled, NULL, NULL, bool, NULL),
  DTE (STRING, rprog, disabled_text, NULL, NULL, str, NULL),
  DTE_SHOW (AREA, rprog, area, DATA_NAME, NULL, NULL),
  DTE_OLC2 ("copy", NULL, olded, rpedit_copy),
  DTE_END
};

DataTable help_data_table[] = {
  DTE (INT, help, level, NULL, NULL, int, validate_level),
  DTE (STRING, help, keyword, NULL, NULL, str, validate_keyword),
  DTE_OLC2 ("addkeywd", NULL, olded, hedit_addkeywd),
  DTE (STRING, help, text, NULL, 1, desc, NULL),
  DTE (INT_FLAGSTRING, help, category, help_types, NULL, flag, NULL),
  DTE_OLC2 ("copy", NULL, olded, hedit_copy),
  DTE_END
};

DataTable mbr_data_table[] = {
  DTE (STRING, mbr, name, NULL, NULL, str, NULL),
  DTE (FUNCTION_INT_TO_STR, mbr, clan, clan_rw, NULL, clan, NULL),
  DTE (INT, mbr, rank, NULL, NULL, int, NULL),
  DTE (INT, mbr, level, NULL, NULL, int, validate_level),
  DTE (LONG, mbr, gold_donated, NULL, NULL, long, NULL),
  DTE (INT, mbr, qp_donated, NULL, NULL, int, NULL),
  DTE_END
};

DataTable clan_data_table[] = {
  DTE (STRING, clan, name, NULL, NULL, str, NULL),
  DTE (STRING, clan, who_name, NULL, NULL, str, NULL),
  DTE (STRING, clan, description, NULL, NULL, str, NULL),
  DTE (LONG_ARRAY, clan, rooms, MAX_CLAN_ROOM, NULL, larray, NULL),
  DTE (FLAGSTRING, clan, flags, clan_flags, NULL, flag, NULL),
  DTE (LONG, clan, gold, NULL, NULL, long, NULL),
  DTE (INT, clan, questpoints, NULL, NULL, int, NULL),
  DTE (INT_FLAGSTRING, clan, ethos, ethos_types, NULL, flag, NULL),
  DTE (CLAN_RANK, clan, rank, MAX_RANK, NULL, olded, cedit_rank),
  DTE_END
};

void
load_struct (FileData * fp, void *typebase, DataTable * table, void *puntero)
{
  const char *word;
  DataTable *temp;
  flag_t *pentero, ftemp;
  const char **pstring, *string;
  int *pint, **array;
  long *plong, **larray;
  TableRW_F *function;
  FlagTable *flagtable;
  bool found = false, *pbool;
  int cnt = 0, i;
  ClanRank *rdata;
  time_t *ptime;
  MspData **sound;
  NameList **nl_first, **nl_last, *nl;

  while (str_cmp ((word = read_word (fp)), "#END"))
    {
      for (temp = table; !NullStr (temp->field); temp++)
	{
	  if (!str_cmp (word, temp->field))
	    {
	      switch (temp->type)
		{
		case FIELD_STRING:
		  rwgetdata (pstring, const char *, temp->argument,
			     typebase, puntero);

		  *pstring = read_string (fp);
		  found = true, cnt++;
		  break;

		case FIELD_INT:
		  rwgetdata (pint, int, temp->argument, typebase, puntero);

		  *pint = read_number (fp);
		  found = true, cnt++;
		  break;

		case FIELD_LONG:
		  rwgetdata (plong, long, temp->argument, typebase, puntero);

		  *plong = read_number (fp);
		  found = true, cnt++;
		  break;

		case FIELD_TIME:
		  rwgetdata (ptime, time_t, temp->argument, typebase,
			     puntero);
		  *ptime = read_number (fp);
		  read_to_eol (fp);
		  found = true, cnt++;
		  break;

		case FIELD_FUNCTION_INT_TO_STR:
		  rwgetdata (pint, int, temp->argument, typebase, puntero);

		  function = (TableRW_F *) temp->arg1;
		  string = read_string (fp);
		  if ((*function) (act_read, pint, &string) == false)
		    bugf ("field %s invalid, string %s", temp->field, string);
		  free_string (string);
		  found = true, cnt++;
		  break;

		case FIELD_FLAGSTRING:
		  rwgetdata (pentero, flag_t, temp->argument, typebase,
			     puntero);
		  flagtable = (FlagTable *) temp->arg1;
		  string = read_string (fp);
		  ftemp = flag_value (flagtable, string);
		  *pentero = Max (0, ftemp);
		  free_string (string);
		  found = true, cnt++;
		  break;

		case FIELD_INT_FLAGSTRING:
		  rwgetdata (pint, int, temp->argument, typebase, puntero);

		  flagtable = (FlagTable *) temp->arg1;
		  string = read_string (fp);
		  ftemp = flag_value (flagtable, string);
		  *pint = Max (0, ftemp);
		  free_string (string);
		  found = true, cnt++;
		  break;

		case FIELD_FLAGVECTOR:
		  rwgetdata (pentero, flag_t, temp->argument, typebase,
			     puntero);
		  *pentero = read_flag (fp);
		  found = true, cnt++;
		  break;

		case FIELD_BOOL:
		  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
		  string = read_word (fp);
		  *pbool = str_cmp (string, "false") ? true : false;
		  found = true, cnt++;
		  break;

		case FIELD_INT_ARRAY:
		  rwgetdata (pint, int, temp->argument, typebase, puntero);

		  i = 0;
		  while (str_cmp ((string = read_word (fp)), "@"))
		    {
		      if (i == (int) temp->arg1)
			bugf ("%s array has excess elements", temp->field);
		      else
			pint[i++] = (int) atoi (string);
		    }
		  while (i < (int) temp->arg1)
		    pint[i++] = (int) temp->arg2;
		  found = true, cnt++;
		  break;

		case FIELD_INT_ALLOC_ARRAY:
		  rwgetdata (array, int *, temp->argument, typebase, puntero);

		  i = 0;
		  alloc_mem (*array, int, *(int *) temp->arg1);

		  while (str_cmp ((string = read_word (fp)), "@"))
		    {
		      if (i == *(int *) temp->arg1)
			bugf ("%s array has excess elements", temp->field);
		      else
			(*array)[i++] = (int) atoi (string);
		    }
		  while (i < *(int *) temp->arg1)
		    (*array)[i++] = (int) temp->arg2;
		  found = true, cnt++;
		  break;

		case FIELD_LONG_ARRAY:
		  rwgetdata (plong, long, temp->argument, typebase, puntero);

		  i = 0;
		  while (str_cmp ((string = read_word (fp)), "@"))
		    {
		      if (i == (int) temp->arg1)
			bugf ("%s array has excess elements", temp->field);
		      else
			plong[i++] = (long) atol (string);
		    }
		  while (i < (int) temp->arg1)
		    plong[i++] = (long) temp->arg2;
		  found = true, cnt++;
		  break;

		case FIELD_LONG_ALLOC_ARRAY:
		  rwgetdata (larray, long *, temp->argument, typebase,
			     puntero);

		  i = 0;
		  alloc_mem (*larray, long, *(int *) temp->arg1);

		  while (str_cmp ((string = read_word (fp)), "@"))
		    {
		      if (i == *(int *) temp->arg1)
			bugf ("%s array has excess elements", temp->field);
		      else
			(*larray)[i++] = (long) atol (string);
		    }
		  while (i < *(int *) temp->arg1)
		    (*larray)[i++] = (long) temp->arg2;
		  found = true, cnt++;
		  break;

		case FIELD_STRING_ARRAY:
		case FIELD_STRING_ARRAY_NULL:
		  rwgetdata (pstring, const char *, temp->argument,
			     typebase, puntero);

		  i = 0;
		  while (str_cmp ((string = read_string (fp)), "@"))
		    {
		      if (i == (int) temp->arg1)
			bugf ("%s array has excess elements.", temp->field);
		      else
			pstring[i++] = string;
		    }
		  while (i < (int) temp->arg1)
		    pstring[i++] = NULL;
		  found = true, cnt++;
		  break;

		case FIELD_CLAN_RANK:
		  rwgetdata (rdata, ClanRank, temp->argument, typebase,
			     puntero);
		  i = read_number (fp);
		  rdata[i - 1].rankname = read_string (fp);
		  found = true, cnt++;
		  break;

		case FIELD_MSP:
		  rwgetdata (sound, MspData *, temp->argument, typebase,
			     puntero);
		  *sound = read_sound (fp);
		  found = true, cnt++;
		  break;

		case FIELD_NAMELIST:
		  rwgetdata (nl_first, NameList *, temp->argument, typebase,
			     puntero);
		  rwgetdata (nl_last, NameList *, temp->arg1, typebase,
			     puntero);
		  nl = new_namelist ();
		  nl->name = read_string (fp);
		  Link (nl, *nl, next, prev);
		  found = true, cnt++;
		  break;

		case FIELD_INUTIL:
		  read_to_eol (fp);
		  found = true, cnt++;
		  break;

		case FIELD_BOOL_ARRAY:
		  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
		  i = 0;
		  while (str_cmp ((string = read_word (fp)), "@"))
		    {
		      if (temp->arg1 != NULL && i == (int) temp->arg1)
			bugf ("field_bool_array %s has excess elements",
			      temp->field);
		      else
			pbool[i++] = (bool) atoi (string);
		    }
		  found = true, cnt++;
		  break;
		default:
		  bugf ("type_field %d invalid, field %s", temp->type,
			temp->field);
		  break;
		}
	      if (found == true)
		break;
	    }
	}

      if (found == false)
	{
	  bugf ("word %s not found", word);
	  read_to_eol (fp);
	}
      else
	found = false;
    }
}

void
save_struct (FileData * fp, void *typebase, DataTable * table, void *puntero)
{
  DataTable *temp;
  const char **pstring, *string;
  int *pint, **array;
  long *plong, **larray;
  TableRW_F *function;
  flag_t *pentero;
  bool *pbool;
  FlagTable *flagtable;
  int cnt = 0, i;
  ClanRank *rdata;
  MspData **sound;
  time_t *ptime;
  NameList **namelst, *nl;

  for (temp = table; !NullStr (temp->field); temp++)
    {
      switch (temp->type)
	{
	default:
	  bugf ("type_field %d invalid, field %s", temp->type, temp->field);
	  break;

	case FIELD_STRING:
	  rwgetdata (pstring, const char *, temp->argument, typebase,
		     puntero);

	  f_printf (fp, "%s%s%s~" LF, temp->field,
		    format_tabs (strlen (temp->field)),
		    NullStr (*pstring) ? "" : fix_string (*pstring));
	  break;

	case FIELD_INT:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  f_printf (fp, "%s%s%d" LF, temp->field,
		    format_tabs (strlen (temp->field)), *pint);
	  break;

	case FIELD_LONG:
	  rwgetdata (plong, long, temp->argument, typebase, puntero);

	  f_printf (fp, "%s%s%ld" LF, temp->field,
		    format_tabs (strlen (temp->field)), *plong);
	  break;

	case FIELD_TIME:
	  rwgetdata (ptime, time_t, temp->argument, typebase, puntero);
	  f_printf (fp, "%s%s" TIME_T_FMT " (%s)" LF, temp->field,
		    format_tabs (strlen (temp->field)), *ptime,
		    temp->arg1 != NULL ? timestr (*ptime, false) :
		    str_time (*ptime, -1, NULL));
	  break;

	case FIELD_FUNCTION_INT_TO_STR:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  function = (TableRW_F *) temp->arg1;
	  if ((*function) (act_write, (void *) pint, &string) == false)
	    bugf ("field %s invalid, string %s", temp->field, string);
	  f_printf (fp, "%s%s%s~" LF, temp->field,
		    format_tabs (strlen (temp->field)), string);
	  break;

	case FIELD_FLAGSTRING:
	  rwgetdata (pentero, flag_t, temp->argument, typebase, puntero);
	  flagtable = (FlagTable *) temp->arg1;
	  f_printf (fp, "%s%s%s~" LF, temp->field,
		    format_tabs (strlen (temp->field)),
		    flag_string (flagtable, *pentero));
	  break;

	case FIELD_INT_FLAGSTRING:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  flagtable = (FlagTable *) temp->arg1;
	  f_printf (fp, "%s%s%s~" LF, temp->field,
		    format_tabs (strlen (temp->field)),
		    flag_string (flagtable, *pint));
	  break;

	case FIELD_FLAGVECTOR:
	  rwgetdata (pentero, flag_t, temp->argument, typebase, puntero);
	  f_printf (fp, "%s%s%s" LF, temp->field,
		    format_tabs (strlen (temp->field)),
		    write_flags (*pentero));
	  break;

	case FIELD_BOOL:
	  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
	  f_printf (fp, "%s%s%s" LF, temp->field,
		    format_tabs (strlen (temp->field)),
		    (*pbool == true) ? "true" : "false");
	  break;

	case FIELD_INT_ARRAY:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < (int) temp->arg1; i++)
	    f_printf (fp, "%d ", pint[i]);
	  f_printf (fp, "@" LF);
	  break;

	case FIELD_INT_ALLOC_ARRAY:
	  rwgetdata (array, int *, temp->argument, typebase, puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < *(int *) temp->arg1; i++)
	    f_printf (fp, "%d ", (*array)[i]);
	  f_printf (fp, "@" LF);
	  break;

	case FIELD_LONG_ARRAY:
	  rwgetdata (plong, long, temp->argument, typebase, puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < (int) temp->arg1; i++)
	    f_printf (fp, "%ld ", plong[i]);
	  f_printf (fp, "@" LF);
	  break;

	case FIELD_LONG_ALLOC_ARRAY:
	  rwgetdata (larray, long *, temp->argument, typebase, puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < *(int *) temp->arg1; i++)
	    f_printf (fp, "%ld ", (*larray)[i]);
	  f_printf (fp, "@" LF);
	  break;

	case FIELD_STRING_ARRAY:
	  rwgetdata (pstring, const char *, temp->argument, typebase,
		     puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < (int) temp->arg1; i++)
	    f_printf (fp, "%s~ ",
		      !NullStr (pstring[i]) ? fix_string (pstring[i]) : "");
	  f_printf (fp, "@~" LF);
	  break;

	case FIELD_STRING_ARRAY_NULL:
	  rwgetdata (pstring, const char *, temp->argument, typebase,
		     puntero);

	  write_header (fp, temp->field);
	  for (i = 0; i < (int) temp->arg1 && !NullStr (pstring[i]); i++)
	    f_printf (fp, "%s~ ",
		      !NullStr (pstring[i]) ? fix_string (pstring[i]) : "");
	  f_printf (fp, "@~" LF);
	  break;

	case FIELD_BOOL_ARRAY:
	  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
	  write_header (fp, temp->field);
	  for (i = 0; i < (int) temp->arg1; i++)
	    f_printf (fp, "%d ", pbool[i] == true ? 1 : 0);
	  f_printf (fp, "@" LF);
	  break;

	case FIELD_CLAN_RANK:
	  rwgetdata (rdata, ClanRank, temp->argument, typebase, puntero);
	  for (i = 0; i < (int) temp->arg1; i++)
	    {
	      f_writef (fp, temp->field,
			"%d %s~", i + 1, fix_string (rdata[i].rankname));
	    }
	  break;

	case FIELD_MSP:
	  rwgetdata (sound, MspData *, temp->argument, typebase, puntero);
	  write_sound (fp, temp->field, *sound);
	  break;

	case FIELD_NAMELIST:
	  rwgetdata (namelst, NameList *, temp->argument, typebase, puntero);
	  for (nl = *namelst; nl != NULL; nl = nl->next)
	    f_printf (fp, "%s%s%s~" LF, temp->field,
		      format_tabs (strlen (temp->field)),
		      NullStr (nl->name) ? "" : fix_string (nl->name));
	  break;

	case FIELD_INUTIL:
	  break;
	}

      cnt++;
    }
};


char *
format_field (DataTable * table, Column * cd)
{
  return FORMATF ("{W%-*s{w", Percent (5, cd->col_len), table->field);
}

void
show_struct (CharData * ch, Buffer * output, void *typebase,
	     DataTable * table, void *puntero, int recurse)
{
  DataTable *temp;
  const char **pstring, *string;
  int *pint, **array;
  long *plong, **larray;
  float *floater;
  double *doubler;
  char *pchar, *pstrsz;
  time_t *ptime;
  TableRW_F *function;
  CharData **chdata;
  Spec_F **spdata;
  CharIndex **mobindex;
  RaceData **racedata;
  DeityData **deitydata;
  AreaData **areadata;
  RoomIndex **roomdata;
  ClanData **clandata;
  ObjData **objdata;
  AffectData **affdata;
  flag_t *upentero;
  bool *pbool;
  ClanRank *rdata;
  ShopData **shopdata;
  Descriptor **descdata;
  ObjIndex **objindex;
  ExDescrData **eddata, *edlist;
  ResetData **resdata, *r;
  ExitData ***exdata;
  ProgList **progdata;
  PcData **pcdata;
  MspData **sound;
  FlagTable *flagtable;
  TableShow_F *tsfunc;
  int cnt = 0, i;
  char buf[MSL * 8];
  Column *Cd;

  Cd = new_column ();
  set_cols (Cd, ch, 2, COLS_BUF, output);

  for (temp = table; !NullStr (temp->field); temp++)
    {
      switch (temp->type)
	{
	default:
	  bugf ("type_field %d invalid, field %s", temp->type, temp->field);
	  break;

	case FIELD_STRING:
	  rwgetdata (pstring, const char *, temp->argument, typebase,
		     puntero);

	  if (!NullStr (*pstring)
	      && (!temp->arg1
		  || str_cmp ((const char *) temp->arg1, *pstring)))
	    {
	      print_cols (Cd, "%s: %s%s",
			  format_field (temp, Cd), temp->arg2 != NULL ?
			  NEWLINE : "", *pstring);
	    }
	  break;

	case FIELD_STRING_SIZE:
	  rwgetdata (pstrsz, char, temp->argument, typebase, puntero);

	  string = &(*pstrsz);
	  if (!NullStr (string)
	      && (!temp->arg1 || str_cmp ((const char *) temp->arg1, string)))
	    {
	      print_cols (Cd, "%s: %s", format_field (temp, Cd), string);
	    }
	  break;

	case FIELD_CHAR:
	  rwgetdata (pchar, char, temp->argument, typebase, puntero);

	  if (!temp->arg1 || *pchar != *(char *) temp->arg1)
	    print_cols (Cd, "%s: %c", format_field (temp, Cd), *pchar);
	  break;

	case FIELD_INT:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  if (!temp->arg1 || *pint != (int) temp->arg1)
	    print_cols (Cd, "%s: %d", format_field (temp, Cd), *pint);
	  break;

	case FIELD_INT_FUNCTION:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  tsfunc = (TableShow_F *) temp->arg1;
	  if (!temp->arg2 || *pint != (int) temp->arg2)
	    {
	      if (tsfunc != NULL)
		string = (*tsfunc) (ch, pint);
	      else
		string = FORMATF ("%d", *pint);

	      if (!NullStr (string))
		print_cols (Cd, "%s: %s", format_field (temp, Cd), string);
	    }
	  break;

	case FIELD_FLOAT:
	  rwgetdata (floater, float, temp->argument, typebase, puntero);

	  if (temp->arg1 && *floater != *(float *) temp->arg1)
	    {
	      if (temp->arg2 != NULL)
		print_cols (Cd, "%s: %.*f",
			    format_field
			    (temp, Cd), (int) temp->arg2, *floater);
	      else
		print_cols (Cd, "%s: %f", format_field (temp, Cd), *floater);
	    }
	  break;

	case FIELD_DOUBLE:
	  rwgetdata (doubler, double, temp->argument, typebase, puntero);

	  if (temp->arg1 && *doubler != *(double *) temp->arg1)
	    {
	      if (temp->arg2 != NULL)
		print_cols (Cd, "%s: %.*f",
			    format_field
			    (temp, Cd), (int) temp->arg2, *doubler);
	      else
		print_cols (Cd, "%s: %f", format_field (temp, Cd), *doubler);
	    }
	  break;

	case FIELD_LONG:
	  rwgetdata (plong, long, temp->argument, typebase, puntero);

	  if (!temp->arg1 || *plong != (long) temp->arg1)
	    print_cols (Cd, "%s: %ld", format_field (temp, Cd), *plong);
	  break;

	case FIELD_TIME:
	  rwgetdata (ptime, time_t, temp->argument, typebase, puntero);
	  if (!temp->arg2 || *ptime != (time_t) temp->arg2)
	    print_cols (Cd, "%s: %s",
			format_field (temp, Cd),
			temp->arg1 !=
			NULL ? timestr (*ptime,
					false) : str_time (*ptime, -1, NULL));
	  break;

	case FIELD_FUNCTION_INT_TO_STR:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  if (!temp->arg2 || *pint != (int) temp->arg2)
	    {
	      function = (TableRW_F *) temp->arg1;
	      if (!(*function) (act_write, pint, &string))
		bugf ("field %s invalid, string %s", temp->field, string);
	      print_cols (Cd, "%s: %s", format_field (temp, Cd), string);
	    }
	  break;

	case FIELD_FLAGSTRING:
	  rwgetdata (upentero, flag_t, temp->argument, typebase, puntero);
	  if (!temp->arg2 || *upentero != *(flag_t *) temp->arg2)
	    {
	      flagtable = (FlagTable *) temp->arg1;
	      if (flagtable == act_flags && table == char_data_table
		  && !IsSet (*upentero, ACT_IS_NPC))
		flagtable = plr_flags;
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd),
			  flag_string (flagtable, *upentero));
	    }
	  break;

	case FIELD_INT_FLAGSTRING:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  if (!temp->arg2 || *pint != (int) temp->arg2)
	    {
	      flagtable = (FlagTable *) temp->arg1;
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd),
			  flag_string (flagtable, *pint));
	    }
	  break;

	case FIELD_FLAGVECTOR:
	  rwgetdata (upentero, flag_t, temp->argument, typebase, puntero);
	  if (!temp->arg2 || *upentero != *(flag_t *) temp->arg2)
	    {
	      flagtable = (FlagTable *) temp->arg1;
	      if (flagtable)
		print_cols (Cd, "%s: %s",
			    format_field (temp, Cd),
			    flag_string (flagtable, *upentero));
	      else
		print_cols (Cd, "%s: %s", format_field (temp, Cd),
			    write_flags (*upentero));
	    }
	  break;

	case FIELD_BOOL:
	  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
	  if (!temp->arg1 || *pbool != *(bool *) temp->arg1)
	    print_cols (Cd, "%s: %s",
			format_field (temp, Cd),
			(*pbool == true) ? "true" : "false");
	  break;

	case FIELD_INT_ARRAY:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg1; i++)
	    if (!temp->arg2 || pint[i] != (int) temp->arg2)
	      sprintf (buf + strlen (buf), ", %d", pint[i]);
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_INT_FUNC_ARRAY:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  tsfunc = (TableShow_F *) temp->arg1;
	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg2; i++)
	    {
	      if (tsfunc)
		string = (*tsfunc) (ch, (void *) &pint[i]);
	      else
		string = FORMATF ("%d", pint[i]);
	      if (!NullStr (string))
		sprintf (buf + strlen (buf), ", %s", string);
	    }
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_INT_ALLOC_ARRAY:
	  rwgetdata (array, int *, temp->argument, typebase, puntero);

	  tsfunc = (TableShow_F *) temp->arg2;

	  buf[0] = NUL;
	  for (i = 0; i < *(int *) temp->arg1; i++)
	    {
	      if (tsfunc != NULL && (string = tsfunc (ch, (void *) &i)))
		sprintf (buf + strlen (buf), ", %s:", string);
	      else
		strcat (buf, ", ");

	      sprintf (buf + strlen (buf), "%d", (*array)[i]);
	    }
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_LONG_ALLOC_ARRAY:
	  rwgetdata (larray, long *, temp->argument, typebase, puntero);

	  tsfunc = (TableShow_F *) temp->arg2;

	  buf[0] = NUL;
	  for (i = 0; i < *(int *) temp->arg1; i++)
	    {
	      if (tsfunc != NULL && (string = tsfunc (ch, (void *) &i)))
		sprintf (buf + strlen (buf), ", %s:", string);
	      else
		strcat (buf, ", ");

	      sprintf (buf + strlen (buf), "%ld", (*larray)[i]);
	    }
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_LONG_ARRAY:
	  rwgetdata (plong, long, temp->argument, typebase, puntero);

	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg1; i++)
	    if (!temp->arg2 || plong[i] != (long) temp->arg2)
	      sprintf (buf + strlen (buf), ", %ld", plong[i]);
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_STRING_ARRAY:
	case FIELD_STRING_ARRAY_NULL:
	  rwgetdata (pstring, const char *, temp->argument, typebase,
		     puntero);

	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg1; i++)
	    {
	      if (!NullStr (pstring[i]))
		sprintf (buf + strlen (buf), "%s%d) %s",
			 format_tabs (strlen (temp->field + 3)), i + 1,
			 pstring[i]);
	    }
	  if (buf[0] != NUL)
	    {
	      print_cols (Cd, "%s: %s", format_field (temp, Cd), buf);
	    }
	  break;

	case FIELD_BOOL_ARRAY:
	  rwgetdata (pbool, bool, temp->argument, typebase, puntero);
	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg1; i++)
	    sprintf (buf + strlen (buf), ", %s",
		     pbool[i] == true ? "true" : "false");
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_DICE:
	  rwgetdata (pint, int, temp->argument, typebase, puntero);

	  print_cols (Cd, "%s: %dd%d+%d",
		      format_field (temp, Cd),
		      pint[DICE_NUMBER], pint[DICE_TYPE], pint[DICE_BONUS]);
	  break;

	case FIELD_CLAN_RANK:
	  rwgetdata (rdata, ClanRank, temp->argument, typebase, puntero);
	  buf[0] = NUL;
	  for (i = 0; i < (int) temp->arg1; i++)
	    {
	      sprintf (buf + strlen (buf), "%s%d) %s",
		       format_tabs (strlen (temp->field) + 3), i + 1,
		       rdata[i].rankname);
	      col_print (Cd, buf);
	    }
	  break;

	case FIELD_CHARDATA:
	  rwgetdata (chdata, CharData *, temp->argument, typebase, puntero);
	  if (chdata && *chdata)
	    {
	      if ((int) temp->arg1 == DATA_LIST)
		{
		  CharData *list;

		  buf[0] = NUL;
		  for (list = *chdata; IsValid (list);
		       list = list->next_in_room)
		    sprintf (buf + strlen (buf), ", %s",
			     IsNPC (list) ? list->short_descr : list->name);
		  if (buf[0] != NUL)
		    print_cols (Cd, "%s: %s", format_field (temp, Cd),
				&buf[2]);
		}
	      else if ((int) temp->arg1 == DATA_STAT && recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &char_zero, char_data_table,
			       *chdata, recurse + 1);
		}
	      else
		{
		  if (!IsNPC (*chdata))
		    print_cols (Cd, "%s: %s",
				format_field (temp, Cd), IsNPC (*chdata)
				? (*chdata)->short_descr : (*chdata)->name);
		  else
		    print_cols (Cd, "%s: %s [%ld]", format_field (temp, Cd),
				IsNPC ((*chdata)) ? (*chdata)->short_descr
				: (*chdata)->name,
				(*chdata)->pIndexData->vnum);
		}
	    }
	  break;

	case FIELD_EXDESC:
	  rwgetdata (eddata, ExDescrData *, temp->argument, typebase,
		     puntero);
	  buf[0] = NUL;
	  for (edlist = *eddata; IsValid (edlist); edlist = edlist->next)
	    sprintf (buf + strlen (buf), ", %s", edlist->keyword);
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_FUNCTION:
	  if ((tsfunc = (TableShow_F *) temp->arg1) != NULL)
	    {
	      string = (*tsfunc) (ch, puntero);
	      if (!NullStr (string))
		print_cols (Cd, "%s: %s", format_field (temp, Cd), string);
	    }
	  break;

	case FIELD_RESET_DATA:
	  rwgetdata (resdata, ResetData *, temp->argument, typebase, puntero);
	  buf[0] = NUL;
	  for (r = *resdata; IsValid (r); r = r->next)
	    sprintf (buf + strlen (buf), ", [%c-%ld]", r->command, r->arg1);
	  if (buf[0] != NUL)
	    print_cols (Cd, "%s: %s", format_field (temp, Cd), &buf[2]);
	  break;

	case FIELD_EXIT_DATA:
	  rwgetdata (exdata, ExitData **, temp->argument, typebase, puntero);
	  if ((int) temp->arg2 == DATA_STAT || recurse == 0)
	    {
	      RoomIndex *pRoom = (RoomIndex *) puntero;

	      for (i = 0; i < (int) temp->arg1; i++)
		{
		  if (pRoom->exit[i] != NULL)
		    {
		      cols_nl (Cd);
		      bprintln (output, stringf
				(ch, 0, Center, NULL, "--=[%s]=--",
				 dir_name[i]));
		      show_struct (ch, output, &exit_zero,
				   exit_data_table, pRoom->exit[i],
				   recurse + 1);
		    }
		}
	    }
	  break;

	case FIELD_MSP:
	  rwgetdata (sound, MspData *, temp->argument, typebase, puntero);

	  if (sound && *sound)
	    {
	      cols_nl (Cd);
	      bprintln (output, "Vol Loop Prio Cont File");
	      bprintln (output, "--- ---- ---- ---- --------------------");

	      bprintlnf (output, "%-3d %-4d %-4d %-4s %s%s%s%s",
			 (*sound)->volume, (*sound)->loop,
			 (*sound)->priority,
			 (*sound)->restart ? "false" : "true",
			 (*sound)->url, flag_string (msp_types,
						     (*sound)->type),
			 (*sound)->type == MSP_NONE ? "" : "/",
			 (*sound)->file);
	    }
	  break;

	case FIELD_AFFECT:
	  rwgetdata (affdata, AffectData *, temp->argument, typebase,
		     puntero);
	  if (affdata && *affdata)
	    {
	      AffectData *aff;
	      int cnt;

	      for (cnt = 0, aff = *affdata; IsValid (aff); aff = aff->next)
		{
		  if (cnt == 0)
		    {
		      cols_nl (Cd);
		      bprintln (output,
				"Number Modifier Affects      Special Type");
		      bprintln (output,
				"------ -------- -------      ------------");
		    }
		  if (aff->bitvector)
		    bprintlnf (output, "[%4d] %-8d %-12s %-7s %s", cnt,
			       aff->modifier,
			       flag_string (apply_flags,
					    aff->location),
			       flag_string (apply_types,
					    aff->where),
			       flag_string (bitvector_type
					    [aff->where].table,
					    aff->bitvector));
		  else
		    bprintlnf (output, "[%4d] %-8d %-12s", cnt,
			       aff->modifier,
			       flag_string (apply_flags, aff->location));
		  cnt++;
		}
	    }
	  break;

	case FIELD_PROG_LIST:
	  rwgetdata (progdata, ProgList *, temp->argument, typebase, puntero);
	  if (progdata && *progdata)
	    {
	      ProgList *list;
	      int cnt;

	      for (cnt = 0, list = *progdata; IsValid (list);
		   list = list->next)
		{
		  if (cnt == 0)
		    {
		      cols_nl (Cd);
		      bprintln (output, "Number Vnum Trigger Phrase");
		      bprintln (output, "------ ---- ------- ------");
		    }

		  bprintlnf (output, "[%5d] %4ld %7s %s", cnt,
			     list->prog->vnum,
			     prog_type_to_name (list->trig_type),
			     list->trig_phrase);
		  if (list->prog->disabled)
		    bprintlnf (output,
			       "           Program is disabled. (%.10s)",
			       list->prog->disabled_text);
		  cnt++;
		}
	    }

	  break;

	case FIELD_DESC:
	  rwgetdata (descdata, Descriptor *, temp->argument, typebase,
		     puntero);
	  if (descdata && *descdata)
	    {
	      if ((int) temp->arg1 == DATA_STAT && recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &descriptor_zero,
			       descriptor_data_table, *descdata, recurse + 1);
		}
	      else
		{
		  print_cols (Cd, "%s: %d",
			      format_field
			      (temp, Cd), (*descdata)->descriptor);
		}
	    }
	  break;

	case FIELD_SPEC_FUN:
	  rwgetdata (spdata, Spec_F *, temp->argument, typebase, puntero);
	  if (spdata && *spdata)
	    {
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd), spec_name ((*spdata)));
	    }
	  break;

	case FIELD_SHOP:
	  rwgetdata (shopdata, ShopData *, temp->argument, typebase, puntero);

	  if (shopdata && *shopdata)
	    {
	      if (recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &shop_zero, shop_data_table,
			       *shopdata, recurse + 1);
		}
	    }
	  break;

	case FIELD_PCDATA:
	  rwgetdata (pcdata, PcData *, temp->argument, typebase, puntero);

	  if (pcdata && *pcdata)
	    {
	      if (recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &pcdata_zero,
			       pcdata_data_table, *pcdata, recurse + 1);
		}
	    }
	  break;

	case FIELD_MOBINDEX:
	  rwgetdata (mobindex, CharIndex *, temp->argument, typebase,
		     puntero);

	  if (mobindex && *mobindex)
	    {
	      if (recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &char_index_zero,
			       char_index_data_table, *mobindex, recurse + 1);
		}
	    }
	  break;

	case FIELD_OBJINDEX:
	  rwgetdata (objindex, ObjIndex *, temp->argument, typebase, puntero);

	  if (objindex && *objindex)
	    {
	      if (recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &obj_index_zero,
			       obj_index_data_table, *objindex, recurse + 1);
		}
	    }
	  break;

	case FIELD_RACE:
	  rwgetdata (racedata, RaceData *, temp->argument, typebase, puntero);

	  if (racedata && *racedata)
	    {
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd), (*racedata)->name);
	    }
	  break;

	case FIELD_DEITY:
	  rwgetdata (deitydata, DeityData *, temp->argument, typebase,
		     puntero);

	  if (deitydata && *deitydata)
	    {
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd), (*deitydata)->name);
	    }
	  break;

	case FIELD_AREA:
	  rwgetdata (areadata, AreaData *, temp->argument, typebase, puntero);

	  if (areadata && *areadata)
	    {
	      if ((int) temp->arg1 == DATA_STAT && recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &area_zero, area_data_table,
			       *areadata, recurse + 1);
		}
	      else
		{
		  print_cols (Cd, "%s: %s",
			      format_field (temp, Cd), (*areadata)->name);
		}
	    }
	  break;
	case FIELD_ROOM:
	  rwgetdata (roomdata, RoomIndex *, temp->argument, typebase,
		     puntero);
	  if (roomdata && *roomdata)
	    {
	      if ((int) temp->arg1 == DATA_STAT && recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &room_index_zero,
			       room_index_data_table, *roomdata, recurse + 1);
		}
	      else
		{
		  if ((*roomdata)->area->clan == NULL)
		    print_cols (Cd, "%s: %s [%ld]",
				format_field
				(temp, Cd),
				(*roomdata)->name, (*roomdata)->vnum);
		  else
		    print_cols (Cd,
				"%s: %s [%ld] (owned by %s)",
				format_field (temp, Cd),
				(*roomdata)->name,
				(*roomdata)->vnum,
				(*roomdata)->area->clan->name);
		}
	    }
	  break;

	case FIELD_CLAN:
	  rwgetdata (clandata, ClanData *, temp->argument, typebase, puntero);

	  if (clandata && *clandata)
	    {
	      print_cols (Cd, "%s: %s",
			  format_field (temp, Cd), (*clandata)->who_name);
	    }
	  break;

	case FIELD_OBJ:
	  rwgetdata (objdata, ObjData *, temp->argument, typebase, puntero);

	  if (objdata && *objdata)
	    {
	      if ((int) temp->arg1 == DATA_LIST)
		{
		  ObjData *list;

		  buf[0] = NUL;
		  for (list = *objdata; IsValid (list);
		       list = list->next_content)
		    sprintf (buf + strlen (buf), ", %s", list->short_descr);
		  if (buf[0] != NUL)
		    print_cols (Cd, "%s: %s", format_field (temp, Cd),
				&buf[2]);
		}
	      else if ((int) temp->arg1 == DATA_STAT && recurse == 0)
		{
		  cols_nl (Cd);
		  show_struct (ch, output, &obj_zero, obj_data_table,
			       *objdata, recurse + 1);
		}
	      else
		{
		  print_cols (Cd, "%s: %s [%ld]",
			      format_field
			      (temp, Cd),
			      (*objdata)->short_descr,
			      (*objdata)->pIndexData->vnum);
		}
	    }
	  break;

	case FIELD_INUTIL:
	  break;
	}

      cnt++;
    }
  cols_nl (Cd);
};


const struct datasave_type datasave_table[] = {
  {
   rw_help_data, "helps"},
  {rw_cmd_data, "commands"},
  {rw_skill_data, "skills"},
  {rw_race_data, "races"},
  {rw_group_data, "groups"},
  {rw_class_data, "classes"},
  {rw_social_data, "socials"},
  {rw_clan_data, "clans"},
  {rw_stat_data, "stats"},
  {rw_ban_data, "bans"},
  {rw_gquest_data, "gquest"},
  {rw_deity_data, "deities"},
  {rw_wpwd_data, "webpasswds"},
  {rw_mbr_data, "members"},
  {rw_war_data, "war"},
  {rw_channel_data, "channels"},
  {rw_mud_data, "mud"},
  {rw_song_data, "music"},
  {rw_disabled_data, "disabled"},
  {rw_time_data, "time"},
  {NULL, NULL}
};

TableSave_Fun (rw_cmd_data)
{
  rw_list (type, COMMAND_FILE, CmdData, cmd);
}

TableSave_Fun (rw_skill_data)
{
  rw_table (type, SKILL_FILE, SkillData, skill);

  if (type == act_read)
    {
      int sn;

      skill_table[top_skill].name = NULL;
      for (sn = 0; sn < top_skill; sn++)
	{
	  if (skill_table[sn].pgsn != NULL
	      && skill_table[sn].pgsn != &gsn_null)
	    *skill_table[sn].pgsn = sn;
	}
    }

}

TableSave_Fun (rw_race_data)
{
  rw_list (type, RACE_FILE, RaceData, race);
  if (type == act_read)
    {
      default_race = new_race ();
      replace_str (&default_race->name, "Unique");
    }
}

TableSave_Fun (rw_group_data)
{
  rw_table (type, GROUP_FILE, GroupData, group);

  if (type == act_read)
    group_table[top_group].name = NULL;
}

TableSave_Fun (rw_class_data)
{
  rw_table (type, CLASS_FILE, ClassData, class);

  if (MAX_REMORT > top_class)
    {
      logf ("MAX_REMORT(%d) is higher than top_class(%d).", MAX_REMORT,
	    top_class);
      logf ("You can fix this by changing MAX_REMORT to a lower number.");
      exit (1);
    }

  if (type == act_read)
    {
      int sn;

      class_table[top_class].name[0] = NULL;
      for (sn = 0; sn < top_class; sn++)
	{
	  if (class_table[sn].index != NULL
	      && class_table[sn].index != &class_null)
	    *class_table[sn].index = sn;
	}
    }
}

TableSave_Fun (rw_social_data)
{
  rw_list (type, SOCIAL_FILE, SocialData, social);
}

TableSave_Fun (rw_disabled_data)
{
  rw_list (type, DISABLED_FILE, DisabledData, disabled);
}

TableSave_Fun (rw_gquest_data)
{
  rw_single (type, GQUEST_FILE, gquest);
}

TableSave_Fun (rw_deity_data)
{
  rw_list (type, DEITY_FILE, DeityData, deity);
}

TableSave_Fun (rw_war_data)
{
  rw_single (type, WAR_FILE, war);
}

TableSave_Fun (rw_channel_data)
{
  rw_table (type, CHANNEL_FILE, ChannelData, channel);
  if (type == act_read)
    {
      int sn;

      channel_table[top_channel].name = NULL;
      for (sn = 0; sn < top_channel; sn++)
	{
	  if (channel_table[sn].index != NULL
	      && channel_table[sn].index != &gcn_null)
	    *channel_table[sn].index = sn;
	}
    }
}

TableSave_Fun (rw_help_data)
{
  rw_list (type, HELP_FILE, HelpData, help);
}

void
init_mud_data (void)
{
  mud_info.pulsepersec = 4;
  mud_info.default_port = 4000;
  mud_info.webport_offset = 1;
  mud_info.mud_flags = 0;
  mud_info.share_value = 100;
  memset (&mud_info.stats, 0, sizeof (mud_info.stats));
  mud_info.last_copyover = 0;
  mud_info.weath_unit = 10;
  mud_info.rand_factor = 2;
  mud_info.climate_factor = 1;
  mud_info.max_vector = mud_info.weath_unit * 3;
  mud_info.longest_uptime = current_time - boot_time;
  mud_info.stats.lastupdate = current_time;
  mud_info.bind_ip_address = str_dup ("0.0.0.0");
  mud_info.bonus.mod = 0;
  mud_info.bonus.time = -1;
  mud_info.bonus.status = BONUS_OFF;
  mud_info.group_lvl_limit = 20;
  mud_info.pcdam = 100;
  mud_info.mobdam = 100;
  mud_info.disabled_signals = 0;
  mud_info.max_points = 40;
}

TableSave_Fun (rw_mud_data)
{
  flag_t saved_cmdline = 0;

  if (type == act_read)
    {
      if (crs_info.status != CRS_COPYOVER)
	saved_cmdline = mud_info.cmdline_options;
      init_mud_data ();
    }
  rw_single (type, MUD_FILE, mud);
  if (type == act_read)
    {
      if (crs_info.status != CRS_COPYOVER)
	mud_info.cmdline_options = saved_cmdline;
      if (!mud_info.unique_id)
	mud_info.unique_id = getcurrenttime () * getpid () & 0x7fffffff;
      if (NullStr (mud_info.name))
	replace_str (&mud_info.name, MUDNAME);
      if (NullStr (mud_info.login_prompt))
	replace_str (&mud_info.login_prompt,
		     "By What Name Shall You Be Known? ");
    }
}

TableSave_Fun (rw_song_data)
{

  rw_table (type, MUSIC_FILE, SongData, song);
  if (type == act_read)
    {
      int i;

      song_table[top_song].name = NULL;
      for (i = 0; i <= MAX_GLOBAL; i++)
	channel_songs[i] = -1;
    }
}

void
show_set_commands (CharData * ch, DataTable * table)
{
  int i;
  Buffer *buf = new_buf ();
  Column *Cd = new_column ();

  set_cols (Cd, ch, 5, COLS_BUF, buf);

  for (i = 0; table[i].field != NULL; i++)
    {
      if (table[i].function)
	print_cols (Cd, table[i].field);
    }
  cols_nl (Cd);

  sendpage (ch, buf_string (buf));
  free_buf (buf);
  if (table == char_data_table)
    {
      show_set_commands (ch, pcdata_data_table);
      show_set_commands (ch, descriptor_data_table);
    }
  free_column (Cd);
}

bool
set_struct (CharData * ch, const char *argument, void *typebase,
	    DataTable * table, void *data)
{
  char arg[MIL], logline[MIL];
  DataTable *temp;
  bool found = false;

  strcpy (logline, argument);
  argument = one_argument (argument, arg);

  for (temp = table; !NullStr (temp->field) && !found; temp++)
    {
      if (!temp->function)
	continue;

      if (tolower (arg[0]) == tolower (temp->field[0])
	  && !str_prefix (arg, temp->field))
	{
	  void *puntero;

	  rwgetdata (puntero, void, temp->argument, typebase, data);

	  (*temp->function) (temp, ch, data, argument, puntero);
	  found = true;

	}
    }
  if (!found && table == char_data_table)
    {
      CharData *vict = (CharData *) data;

      if (vict->pcdata)
	{
	  found =
	    set_struct (ch, logline, &pcdata_zero, pcdata_data_table,
			vict->pcdata);
	}
      if (!found && vict->desc)
	{
	  found =
	    set_struct (ch, logline, &descriptor_zero,
			descriptor_data_table, vict->desc);
	}
    }
  if (!found)
    show_set_commands (ch, table);
  return found;
}

bool
check_common_olc (CharData * ch, const char *arg, const char *argument)
{
  int temp;

  for (temp = 0; olc_comm_table[temp].name != NULL; temp++)
    {
      if (tolower (arg[0]) == tolower (olc_comm_table[temp].name[0])
	  && !str_prefix (arg, olc_comm_table[temp].name))
	{
	  (*olc_comm_table[temp].fun) (olc_comm_table[temp].name, ch,
				       argument);
	  return true;
	}
    }
  return false;
}

bool
edit_struct (CharData * ch, const char *argument, void *typebase,
	     DataTable * table, void *data)
{
  char arg[MIL];
  DataTable *temp;

  argument = one_argument (argument, arg);

  if (check_common_olc (ch, arg, argument))
    return true;

  for (temp = table; !NullStr (temp->field); temp++)
    {
      if (!temp->function)
	continue;

      if (tolower (arg[0]) == tolower (temp->field[0])
	  && !str_prefix (arg, temp->field))
	{
	  void *puntero;

	  rwgetdata (puntero, void, temp->argument, typebase, data);

	  if ((*temp->function) (temp, ch, data, argument, puntero))
	    {
	      olc_show (ch, olc_lookup (ch->desc->editor));
	    }
	  return true;
	}
    }

  return false;
}
