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

#ifndef __OLC_ED_H_
#define __OLC_ED_H_  1

Ed_Fun (olced_str)
{
  const char **string = (const char **) data;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field, "<string>", NULL);
      return false;
    }

  if (!validate_arg (ch, table->arg3, argument))
    return false;

  replace_strf (string, "%s%s", argument, table->arg2 == NULL ? "" : NEWLINE);
  olc_msg (ch, table->field, "'%s': Ok.", argument);
  return true;
}

Ed_Fun (olced_desc)
{
  if (NullStr (argument))
    {
      if (!validate_arg (ch, table->arg3, *(const char **) data))
	return false;

      string_append (ch, (const char **) data);
      return false;
    }

  cmd_syntax (ch, NULL, table->field, "", NULL);

  return false;
}

Ed_Fun (olced_bool)
{
  bool *b = (bool *) data;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field, "<true/false>", NULL);
      return false;
    }

  if (!str_cmp (argument, "true"))
    *b = true;
  else if (!str_cmp (argument, "false"))
    *b = false;
  else
    {
      olc_msg (ch, table->field, "invalid argument.");
      return false;
    }

  if (!validate_arg (ch, table->arg3, b))
    return false;

  olc_msg (ch, table->field, "'%s': Ok.", argument);
  return true;
}

Ed_Fun (olced_olded)
{
  return (*(Olc_F *) table->arg3) (table->field, ch, argument);
}

Ed_Fun (olced_int)
{
  int *value = (int *) data;
  char *endptr;
  int temp;
  char str1[MIL];

  one_argument (argument, str1);
  temp = strtol (str1, &endptr, 0);
  if (NullStr (str1) || !NullStr (endptr))
    {
      cmd_syntax (ch, NULL, table->field, "<number>", NULL);
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  *value = temp;

  olc_msg (ch, table->field, "'%d': Ok.", temp);
  return true;
}

Ed_Fun (olced_long)
{
  long *value = (long *) data;
  char *endptr;
  long temp;
  char str1[MIL];

  one_argument (argument, str1);
  temp = strtol (str1, &endptr, 0);
  if (NullStr (str1) || !NullStr (endptr))
    {
      cmd_syntax (ch, NULL, table->field, "<number>", NULL);
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  *value = temp;
  olc_msg (ch, table->field, "'%ld': Ok.", temp);
  return true;
}

Ed_Fun (olced_vnum)
{
  vnum_t *value = (vnum_t *) data;
  char *endptr;
  vnum_t temp;
  char str1[MIL];

  one_argument (argument, str1);
  temp = strtol (str1, &endptr, 0);
  if (NullStr (str1) || !NullStr (endptr))
    {
      cmd_syntax (ch, NULL, table->field, "<number>", NULL);
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  *value = temp;
  olc_msg (ch, table->field, "'%ld': Ok.", temp);
  return true;
}


void
show_flags (CharData * ch, FlagTable * flag_table)
{
  Buffer *output;
  int flag;
  Column *Cd;
  int count = 0;

  output = new_buf ();
  Cd = new_column ();
  set_cols (Cd, ch, 4, COLS_BUF, output);

  for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
      if (flag_table[flag].settable)
	{
	  print_cols (Cd, flag_table[flag].name);
	  count++;
	}
    }

  cols_nl (Cd);
  if (count > 0)
    {
      bprintln (output, draw_line (ch, NULL, 0));
      chprintln (ch, draw_line (ch, NULL, 0));
      sendpage (ch, buf_string (output));
    }
  free_buf (output);
  free_column (Cd);
}

Ed_Fun (olced_flag)
{
  int stat;
  FlagTable *f, *t = (FlagTable *) table->arg1;
  flag_t marked;

  if ((stat = is_stat (t)) < 0)
    {
      olc_msg (ch, table->field,
	       "Unknown table of values (report it to implementors).");
      return false;
    }

  if (NullStr (argument))
    {
      show_flags (ch, t);
      cmd_syntax (ch, NULL, table->field, "<value>", NULL);
      return false;
    }

  if (argument[0] == '?')
    {
      show_flags (ch, t);
      return false;
    }

  if (stat)
    {
      if ((f = flag_lookup (argument, t)) == NULL)
	{
	  show_flags (ch, t);
	  cmd_syntax (ch, NULL, table->field, "<value>", NULL);
	  return false;
	}
      if (!f->settable)
	{
	  olc_msg (ch, table->field, "'%s': value is not settable.", f->name);
	  return false;
	}
      if (!validate_arg (ch, table->arg3, &f->bit))
	return false;
      *(int *) data = f->bit;
      olc_msg (ch, table->field, "'%s': Ok.", f->name);
      return true;
    }

  marked = 0;


  for (;;)
    {
      char word[MAX_INPUT_LENGTH];

      argument = one_argument (argument, word);

      if (NullStr (word))
	break;

      if ((f = flag_lookup (word, t)) == NULL)
	{
	  show_flags (ch, t);
	  cmd_syntax (ch, NULL, table->field, "<value>", NULL);
	  return false;
	}
      if (!f->settable)
	{
	  olc_msg (ch, table->field, "'%s': flag is not settable.", f->name);
	  continue;
	}
      SetBit (marked, f->bit);
    }

  if (marked)
    {
      if (!validate_arg (ch, table->arg3, &marked))
	return false;
      ToggleBit (*(flag_t *) data, marked);
      olc_msg (ch, table->field, "'%s': flag(s) toggled.",
	       flag_string (t, marked));
      return true;
    }
  return false;
}

Ed_Fun (olced_shop)
{
  char command[MAX_INPUT_LENGTH];
  char str1[MAX_INPUT_LENGTH];
  ShopData **MobShop = (ShopData **) data;

  argument = one_argument (argument, command);
  argument = one_argument (argument, str1);

  if (NullStr (command))
    {
      cmd_syntax (ch, NULL, table->field, "hours <open> <close>",
		  "profit <%% buy> <%% sell>", "type <1-5> <obj_type>",
		  "type <1-5> none", "assign", "remove", NULL);

      return false;
    }

  if (!str_cmp (command, "hours"))
    {
      if (NullStr (str1) || !is_number (str1)
	  || NullStr (argument) || !is_number (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "hours [open] [close]", NULL);
	  return false;
	}

      if (!(*MobShop))
	{
	  olc_msg (ch, table->field, "Must set a shop first (shop assign).");
	  return false;
	}

      (*MobShop)->open_hour = atoi (str1);
      (*MobShop)->close_hour = atoi (argument);

      olc_msg (ch, table->field, "Hours set.");
      return true;
    }

  if (!str_cmp (command, "profit"))
    {
      if (NullStr (str1) || !is_number (str1)
	  || NullStr (argument) || !is_number (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "profit [%% buy] [%% sell]",
		      NULL);
	  return false;
	}

      if (!(*MobShop))
	{
	  olc_msg (ch, table->field,
		   "You must create a shop first (shop assign).");
	  return false;
	}

      (*MobShop)->profit_buy = atoi (str1);
      (*MobShop)->profit_sell = atoi (argument);

      olc_msg (ch, table->field, "Shop profit set.");
      return true;
    }

  if (!str_cmp (command, "type"))
    {
      flag_t value = 0;

      if (NullStr (str1) || !is_number (str1) || NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "type [#x0-4] [item type]",
		      NULL);
	  return false;
	}

      if (atoi (str1) >= MAX_TRADE)
	{
	  olc_msg (ch, table->field, "May sell %d items max.", MAX_TRADE);
	  return false;
	}

      if (!(*MobShop))
	{
	  olc_msg (ch, table->field,
		   "You must create a shop first  (shop assign).");
	  return false;
	}

      if ((value = flag_value (type_flags, argument)) == NO_FLAG)
	{
	  olc_msg (ch, table->field, "That type of item is not known.");
	  return false;
	}

      (*MobShop)->buy_type[atoi (str1)] = value;

      olc_msg (ch, table->field, "Shop type set.");
      return true;
    }


  if (!str_prefix (command, "assign"))
    {
      CharIndex *pMob;

      GetEdit (ch, CharIndex, pMob);

      if ((*MobShop))
	{
	  olc_msg (ch, table->field,
		   "Mob already has a shop assigned to it.");
	  return false;
	}

      (*MobShop = new_shop ());
      Link ((*MobShop), shop, next, prev);
      (*MobShop)->keeper = pMob->vnum;

      olc_msg (ch, table->field, "New shop assigned to mobile.");
      return true;
    }

  if (!str_prefix (command, "remove"))
    {
      ShopData *pShop;

      pShop = (*MobShop);
      (*MobShop) = NULL;

      UnLink (pShop, shop, next, prev);

      free_shop (pShop);

      olc_msg (ch, table->field, "Mobile is no longer a shopkeeper.");
      return true;
    }

  olced_shop (table, ch, base, "", data);

  return false;
}

Ed_Fun (olced_spec)
{
  Spec_F **spec = (Spec_F **) data;
  Spec_F *fun;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field, "<spec_func>", NULL);
      return false;
    }

  if (!str_cmp (argument, "none"))
    {
      *spec = NULL;
      olc_msg (ch, table->field, "Spec removed.");
      return true;
    }

  fun = spec_lookup (argument);

  if (fun)
    {
      if (!validate_arg (ch, table->arg3, (void *) fun))
	return false;
      *spec = fun;
      olc_msg (ch, table->field, "Spec set.");
      return true;
    }
  else
    {
      olc_msg (ch, table->field, "Spec function doesn't exist.");
      return false;
    }

  return false;
}

Ed_Fun (olced_race)
{
  RaceData *race, **value = (RaceData **) data;

  if (!NullStr (argument) && (race = race_lookup (argument)) != NULL)
    {
      if (!validate_arg (ch, table->arg3, race))
	return false;

      *value = race;

      if (table->arg2 == NULL)

	{
	  CharIndex *pMob = (CharIndex *) base;

	  pMob->act |= race->act;
	  pMob->affected_by |= race->aff;
	  pMob->off_flags |= race->off;
	  pMob->imm_flags |= race->imm;
	  pMob->res_flags |= race->res;
	  pMob->vuln_flags |= race->vuln;
	  pMob->form |= race->form;
	  pMob->parts |= race->parts;
	}
      else
	{
	  CharData *pChar = (CharData *) base;

	  pChar->act |= race->act;
	  pChar->affected_by |= race->aff;
	  pChar->off_flags |= race->off;
	  pChar->imm_flags |= race->imm;
	  pChar->res_flags |= race->res;
	  pChar->vuln_flags |= race->vuln;
	  pChar->form |= race->form;
	  pChar->parts |= race->parts;
	}
      olc_msg (ch, table->field, "Race set.");
      return true;
    }

  if (argument[0] == '?')
    {
      Column *Cd = new_column ();

      set_cols (Cd, ch, 4, COLS_CHAR, ch);

      chprintln (ch, "Available races are:");

      for (race = race_first; race != NULL; race = race->next)
	{
	  print_cols (Cd, race->name);
	}

      cols_nl (Cd);
      free_column (Cd);
      return false;
    }

  cmd_syntax (ch, NULL, table->field, "[race]", "?", NULL);
  return false;
}

Ed_Fun (olced_ac)
{
  CharIndex *pMob = (CharIndex *) data;
  char blarg[MAX_INPUT_LENGTH];
  int pierce, bash, slash, exotic;

  do
    {
      if (NullStr (argument))
	break;

      argument = one_argument (argument, blarg);

      if (!is_number (blarg))
	break;
      pierce = atoi (blarg);
      argument = one_argument (argument, blarg);

      if (!NullStr (blarg))
	{
	  if (!is_number (blarg))
	    break;
	  bash = atoi (blarg);
	  argument = one_argument (argument, blarg);
	}
      else
	bash = pMob->ac[AC_BASH];

      if (!NullStr (blarg))
	{
	  if (!is_number (blarg))
	    break;
	  slash = atoi (blarg);
	  argument = one_argument (argument, blarg);
	}
      else
	slash = pMob->ac[AC_SLASH];

      if (!NullStr (blarg))
	{
	  if (!is_number (blarg))
	    break;
	  exotic = atoi (blarg);
	}
      else
	exotic = pMob->ac[AC_EXOTIC];

      pMob->ac[AC_PIERCE] = pierce;
      pMob->ac[AC_BASH] = bash;
      pMob->ac[AC_SLASH] = slash;
      pMob->ac[AC_EXOTIC] = exotic;

      olc_msg (ch, table->field, "Ac set.");
      return true;
    }
  while (false);

  cmd_syntax (ch, NULL, table->field,
	      "[ac-pierce [ac-bash [ac-slash [ac-exotic]]]]", NULL);

  return false;
}

Ed_Fun (olced_dice)
{
  static char syntax[] = "<number> d <type> + <bonus>";
  char *numb, *type, *bonus, *cp;
  int *arreglo = (int *) data;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field, syntax);
      return false;
    }

  numb = cp = (char *) argument;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  type = cp;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  bonus = cp;

  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';

  if ((!is_number (numb) || atoi (numb) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      cmd_syntax (ch, NULL, table->field, syntax);
      return false;
    }

  arreglo[DICE_NUMBER] = atoi (numb);
  arreglo[DICE_TYPE] = atoi (type);
  arreglo[DICE_BONUS] = atoi (bonus);

  olc_msg (ch, table->field, "'%s' Ok.", argument);

  return true;
}

Ed_Fun (olced_ed)
{
  ExDescrData *ed, **ed_first, **ed_last;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];

  argument = one_argument (argument, command);
  argument = one_argument (argument, keyword);

  if (NullStr (command))
    {
      cmd_syntax (ch, NULL, table->field, "add [keyword]",
		  "delete [keyword]", "edit [keyword]",
		  "format [keyword]", "rename [keyword]", NULL);
      return false;
    }

  switch ((int) table->arg2)
    {

    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	ed_first = &pObj->ed_first;
	ed_last = &pObj->ed_last;
      }
      break;
    case FIELD_OBJ:
      {
	ObjData *Obj = (ObjData *) base;

	ed_first = &Obj->ed_first;
	ed_last = &Obj->ed_last;
      }
      break;
    case FIELD_ROOM:
      {
	RoomIndex *pRoom = (RoomIndex *) base;

	ed_first = &pRoom->ed_first;
	ed_last = &pRoom->ed_last;
      }
      break;
    default:
      olc_msg (ch, table->field, "bad editor.");
      return false;
    }

  if (!str_cmp (command, "add"))
    {
      if (NullStr (keyword))
	{
	  cmd_syntax (ch, NULL, table->field, "add [keyword]", NULL);
	  return false;
	}

      ed = new_ed ();
      replace_str (&ed->keyword, keyword);
      Link (ed, *ed, next, prev);
      string_append (ch, &ed->description);

      return true;
    }

  if (!str_cmp (command, "edit"))
    {
      if (NullStr (keyword))
	{
	  cmd_syntax (ch, NULL, table->field, "edit [keyword]", NULL);
	  return false;
	}

      for (ed = *ed_first; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}

      if (!ed)
	{
	  olc_msg (ch, table->field, "No such extra description.");
	  return false;
	}

      string_append (ch, &ed->description);

      return true;
    }

  if (!str_cmp (command, "delete"))
    {
      if (NullStr (keyword))
	{
	  cmd_syntax (ch, NULL, table->field, "delete [keyword]", NULL);
	  return false;
	}

      for (ed = *ed_first; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}

      if (!ed)
	{
	  olc_msg (ch, table->field, "No such extra description.");
	  return false;
	}

      UnLink (ed, *ed, next, prev);

      free_ed (ed);

      olc_msg (ch, table->field, "Extra description deleted.");
      return true;
    }

  if (!str_cmp (command, "format"))
    {
      if (NullStr (keyword))
	{
	  cmd_syntax (ch, NULL, table->field, "format [keyword]", NULL);
	  return false;
	}

      for (ed = *ed_first; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}

      if (!ed)
	{
	  olc_msg (ch, table->field, "No Such Extra Description");
	  return false;
	}

      ed->description = format_string (ed->description);

      olc_msg (ch, table->field, "Extra description formatted.");
      return true;
    }

  if (!str_cmp (command, "rename"))
    {
      if (NullStr (keyword))
	{
	  cmd_syntax (ch, NULL, table->field, "rename [old] [new]", NULL);
	  return false;
	}

      for (ed = *ed_first; ed; ed = ed->next)
	{
	  if (is_name (keyword, ed->keyword))
	    break;
	}

      if (!ed)
	{
	  olc_msg (ch, table->field, "No such extra description");
	  return false;
	}

      replace_str (&ed->keyword, argument);

      olc_msg (ch, table->field, "Extra description renamed.");
      return true;
    }

  return olced_ed (table, ch, base, "", data);
}

Ed_Fun (olced_direction)
{
  return change_exit (table->field, ch, argument, (int) table->arg1);
}

Ed_Fun (olced_docomm)
{
  (*(Do_F *) table->arg3) (table->field, ch, argument);
  return false;
}

Ed_Fun (olced_value)
{
  return oedit_values (ch, argument, (int) table->arg3);
}

bool
templookup (DataTable * table, CharData * ch, const char *argument,
	    void *data, int temp)
{
  int value;
  Lookup_F *blah = (Lookup_F *) table->arg3;

  if (!emptystring (argument))
    {
      if ((value = ((*blah) (argument))) > temp)
	{
	  *(int *) data = value;
	  olc_msg (ch, table->field, "'%s' Ok.", argument);
	  return true;
	}
      else
	{
	  olc_msg (ch, table->field, "'%s' doesn't exist.", argument);
	  return false;
	}
    }

  cmd_syntax (ch, NULL, table->field, "<name>", NULL);

  return false;
}

Ed_Fun (olced_plookup)
{
  return templookup (table, ch, argument, data, 0);
}

Ed_Fun (olced_nlookup)
{
  return templookup (table, ch, argument, data, -1);
}

void
reset_pflags (flag_t * pflag, ProgList * pl)
{
  flag_t bitv = 0;

  for (; pl; pl = pl->next)
    SetBit (bitv, pl->trig_type);

  *pflag = bitv;
}

Ed_Fun (olced_addprog)
{
  flag_t value, *flags;
  FlagTable *flagtable;
  ProgList *list;
  ProgList **progs_first;
  ProgList **progs_last;
  ProgCode *code;
  char trigger[MSL];
  char phrase[MSL];
  char num[MSL];
  prog_t type;

  argument = one_argument (argument, num);
  argument = one_argument (argument, trigger);
  strcpy (phrase, argument);

  if (!is_number (num) || NullStr (trigger) || NullStr (phrase))
    {
      cmd_syntax (ch, NULL, table->field, "<vnum> <trigger> <value>", NULL);
      return false;
    }

  switch ((int) table->arg2)
    {
    case FIELD_MOBINDEX:
      {
	CharIndex *pMob = (CharIndex *) base;

	flagtable = mprog_flags;
	type = PRG_MPROG;
	progs_first = &pMob->mprog_first;
	progs_last = &pMob->mprog_last;
	flags = &pMob->mprog_flags;
      }
      break;
    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	flagtable = oprog_flags;
	type = PRG_OPROG;
	progs_first = &pObj->oprog_first;
	progs_last = &pObj->oprog_last;
	flags = &pObj->oprog_flags;
      }
      break;
    case FIELD_ROOM:
      {
	RoomIndex *pRoom = (RoomIndex *) base;

	flagtable = rprog_flags;
	type = PRG_RPROG;
	progs_first = &pRoom->rprog_first;
	progs_last = &pRoom->rprog_last;
	flags = &pRoom->rprog_flags;
      }
      break;
    default:
      olc_msg (ch, table->field, "invalid editor.");
      return false;
    }

  if ((value = flag_value (flagtable, trigger)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "Valid flags are:");
      show_olc_help (table->field, ch, "ptriggers");
      return false;
    }

  if ((code = get_prog_index (atov (num), type)) == NULL)
    {
      olc_msg (ch, table->field, "No such Program.");
      return false;
    }

  list = new_prog_list ();
  list->trig_type = value;
  list->prog = code;
  replace_str (&list->trig_phrase, phrase);
  Link (list, *progs, next, prev);

  reset_pflags (flags, *progs_first);

  olc_msg (ch, table->field, "program added.");
  return true;
}

Ed_Fun (olced_delprog)
{
  ProgList *list;
  flag_t *flags;
  ProgList **progs_first;
  ProgList **progs_last;
  char prog[MAX_STRING_LENGTH];
  int value;
  int cnt = -1;

  one_argument (argument, prog);

  if (!is_number (prog) || NullStr (prog))
    {
      cmd_syntax (ch, NULL, table->field, "[#prog]", NULL);
      return false;
    }

  value = atoi (prog);

  if (value < 0)
    {
      olc_msg (ch, table->field, "Only non-negative prog-numbers allowed.");
      return false;
    }

  switch ((int) table->arg2)
    {
    case FIELD_MOBINDEX:
      {
	CharIndex *pMob = (CharIndex *) base;

	progs_first = &pMob->mprog_first;
	progs_last = &pMob->mprog_last;
	flags = &pMob->mprog_flags;
      }
      break;
    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	progs_first = &pObj->oprog_first;
	progs_last = &pObj->oprog_last;
	flags = &pObj->oprog_flags;
      }
      break;
    case ED_ROOM:
      {
	RoomIndex *pRoom = (RoomIndex *) base;

	progs_first = &pRoom->rprog_first;
	progs_last = &pRoom->rprog_last;
	flags = &pRoom->rprog_flags;
      }
      break;
    default:
      olc_msg (ch, table->field, "invalid editor.");
      return false;
    }

  for (list = *progs_first; list != NULL; list = list->next)
    if (++cnt == value)
      break;

  if (!list)
    {
      olc_msg (ch, table->field, "Non existant program.");
      return false;
    }

  UnLink (list, *progs, next, prev);
  free_prog_list (list);

  reset_pflags (flags, *progs_first);

  olc_msg (ch, table->field, "program removed.");
  return true;
}

Ed_Fun (olced_array)
{
  int *value = (int *) data;
  int max = (int) table->arg1;
  int temp, pos;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  one_argument (argument, str2);
  if (NullStr (str1) || NullStr (str2) || !is_number (str1)
      || !is_number (str2))
    {
      cmd_syntax (ch, NULL, table->field, FORMATF ("<1-%d> <value>", max),
		  NULL);
      return false;
    }

  pos = atoi (str1);
  if (pos < 1 || pos > max)
    {
      olc_msg (ch, table->field, "invalid slot.");
      return false;
    }

  temp = atoi (str2);

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos - 1] = temp;
  olc_msg (ch, table->field, "slot %d = '%d': Ok.", pos, temp);
  return true;
}

Ed_Fun (olced_larray)
{
  long *value = (long *) data;
  int max = (int) table->arg1;
  int pos;
  long temp;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  one_argument (argument, str2);
  if (NullStr (str1) || NullStr (str2) || !is_number (str1)
      || !is_number (str2))
    {
      cmd_syntax (ch, NULL, table->field, FORMATF ("<1-%d> <value>", max),
		  NULL);
      return false;
    }

  pos = atoi (str1);
  if (pos < 1 || pos > max)
    {
      olc_msg (ch, table->field, "invalid slot.");
      return false;
    }

  temp = atol (str2);

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos - 1] = temp;
  olc_msg (ch, table->field, "slot %d = '%ld': Ok.", pos, temp);
  return true;
}

Ed_Fun (olced_sarray)
{
  const char **value = (const char **) data;
  int max = (int) table->arg1;
  int pos;
  char str1[MIL];

  argument = one_argument (argument, str1);

  if (NullStr (str1) || NullStr (argument) || !is_number (str1))
    {
      cmd_syntax (ch, NULL, table->field, FORMATF ("<1-%d> <string>", max),
		  NULL);
      return false;
    }

  pos = atoi (str1);
  if (pos < 1 || pos > max)
    {
      olc_msg (ch, table->field, "invalid slot.");
      return false;
    }

  if (!validate_arg (ch, table->arg3, argument))
    return false;

  replace_str (&value[pos - 1], argument);
  olc_msg (ch, table->field, "slot %d = '%s': Ok.", pos, argument);
  return true;
}

Ed_Fun (olced_farray)
{
  flag_t *value = (flag_t *) data;
  int max = (int) table->arg1;
  int pos;
  flag_t temp;
  int f;
  FlagTable *t;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  argument = one_argument (argument, str2);

  if (NullStr (str1) || NullStr (str2) || NullStr (argument)
      || !is_number (str1))
    {
      cmd_syntax (ch, NULL, table->field,
		  FORMATF ("<1-%d> <flags> <string>", max), NULL);
      return false;
    }

  pos = atoi (str1);
  if (pos < 1 || pos > max)
    {
      olc_msg (ch, table->field, "invalid slot.");
      return false;
    }

  t = NULL;
  for (f = 0; flag_stat_table[f].name != NULL; f++)
    {
      if (toupper (flag_stat_table[f].name[0]) == toupper (str2[0])
	  && !str_prefix (str2, flag_stat_table[f].name))
	{
	  t = flag_stat_table[f].structure;
	  break;
	}
    }
  if (!t)
    {
      olc_msg (ch, table->field, "unknown flags");
      return false;
    }

  if ((temp = flag_value (t, argument)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "No such flag in %s.",
	       flag_stat_table[f].name);
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos - 1] = temp;
  olc_msg (ch, table->field, "slot %d = '%s': Ok.", pos, argument);
  return true;
}

Ed_Fun (olced_mclass)
{
  int *value = (int *) data;
  int pos, temp;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  one_argument (argument, str2);
  if (NullStr (str1) || NullStr (str2) || !is_number (str1))
    {
      cmd_syntax (ch, NULL, table->field,
		  FORMATF ("<1-%d> <class>", MAX_REMORT), NULL);
      return false;
    }

  pos = atoi (str1);
  if (pos < 1 || pos > MAX_REMORT)
    {
      olc_msg (ch, table->field, "invalid slot.");
      return false;
    }

  if (pos > value[CLASS_COUNT] + 1)
    {
      olc_msg (ch, table->field, "have to set class %d first.",
	       value[CLASS_COUNT] + 1);
      return false;
    }

  if ((temp = class_lookup (str2)) == -1)
    {
      olc_msg (ch, table->field, "no such class.");
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos - 1] = temp;
  if (pos > value[CLASS_COUNT])
    {
      value[CLASS_COUNT] += 1;
      value[pos] = -1;
    }
  olc_msg (ch, table->field, "slot %d = '%s': Ok.", pos,
	   class_table[temp].name[0]);
  return true;
}

Ed_Fun (olced_stance)
{
  int *value = (int *) data;
  int pos, temp;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  one_argument (argument, str2);
  if (NullStr (str1) || NullStr (str2))
    {
      cmd_syntax (ch, NULL, table->field, "<stance> <value>", NULL);
      return false;
    }

  pos = stance_lookup (str1);

  if (pos == -1 || !ValidStance (stance_table[pos].stance))
    {
      olc_msg (ch, table->field, "invalid stance.");
      return false;
    }

  temp = atoi (str2);
  if (temp < -1 || temp > 200)
    {
      olc_msg (ch, table->field, "value must be between -1 and 200.");
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos] = temp;
  olc_msg (ch, table->field, "%s = '%d': Ok.", stance_table[pos].name, temp);
  return true;
}

Ed_Fun (olced_stats)
{
  int *value = (int *) data;
  int pos, temp;
  char str1[MIL], str2[MIL];

  argument = one_argument (argument, str1);
  one_argument (argument, str2);
  if (NullStr (str1) || NullStr (str2))
    {
      cmd_syntax (ch, NULL, table->field, "<stat> <value>", NULL);
      chprintln (ch, "Valid stats:");
      show_flags (ch, stat_types);
      return false;
    }

  if ((pos = (int) flag_value (stat_types, str1)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "invalid stat.");
      return false;
    }

  temp = atoi (str2);
  if (temp < 0 || temp > MAX_STATS)
    {
      olc_msg (ch, table->field, "value must be between 0 and %d.",
	       MAX_STATS);
      return false;
    }

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  value[pos] = temp;
  olc_msg (ch, table->field, "%s = '%d': Ok.", stat_types[pos].name, temp);
  return true;
}

Ed_Fun (olced_deity)
{
  DeityData **value = (DeityData **) data;
  DeityData *temp;
  char str[MIL];

  one_argument (argument, str);
  if (NullStr (str))
    {
      cmd_syntax (ch, NULL, table->field, "<deity>", NULL);
      return false;
    }

  if ((temp = deity_lookup (str)) == NULL)
    {
      olc_msg (ch, table->field, "invalid deity.");
      return false;
    }

  *value = temp;
  olc_msg (ch, table->field, "'%s': Ok.", temp->name);
  return true;
}

Ed_Fun (olced_getchar)
{
  CharData **value = (CharData **) data;
  CharData *temp;
  char str[MIL];

  one_argument (argument, str);
  if (NullStr (str))
    {
      cmd_syntax (ch, NULL, table->field, "<name>", "none", NULL);
      return false;
    }

  if (!str_cmp (str, "none"))
    {
      *value = NULL;
      olc_msg (ch, table->field, "reset: Ok.");
      return true;
    }

  if ((temp = get_char_world (ch, str)) == NULL)
    {
      olc_msg (ch, table->field, "invalid character.");
      return false;
    }

  *value = temp;
  olc_msg (ch, table->field, "'%s': Ok.",
	   IsNPC (temp) ? temp->short_descr : temp->name);
  return true;
}

Ed_Fun (olced_clan)
{
  ClanData *temp, **value = (ClanData **) data;
  char str[MIL];

  one_argument (argument, str);
  if (NullStr (str))
    {
      cmd_syntax (ch, NULL, table->field, "<clan>", "none", NULL);
      return false;
    }

  if (!str_cmp (str, "none"))
    {
      *value = NULL;
      olc_msg (ch, table->field, "reset: Ok.");
      return true;
    }

  if ((temp = clan_lookup (str)) == NULL)
    {
      olc_msg (ch, table->field, "invalid clan.");
      return false;
    }

  *value = temp;
  olc_msg (ch, table->field, "'%s': Ok.", temp->name);
  return true;
}

Ed_Fun (olced_time)
{
  time_t *value = (time_t *) data;
  time_t temp;
  char str[MIL];

  argument = one_argument (argument, str);
  temp = atol (str);

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field,
		  "<number> days|hours|minutes|seconds", NULL);
      return false;
    }

  if (!str_prefix (argument, "days"))
    temp *= DAY;
  else if (!str_prefix (argument, "hours"))
    temp *= HOUR;
  else if (!str_prefix (argument, "minutes"))
    temp *= MINUTE;
  else
    temp *= SECOND;

  if (!validate_arg (ch, table->arg3, &temp))
    return false;

  *value = temp;
  olc_msg (ch, table->field, "'%s': Ok.", timestr (temp, false));
  return true;
}

Ed_Fun (olced_addaffect)
{
  apply_t value;
  AffectData *pAf, **af_first, **af_last;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];
  int level;

  switch ((int) table->arg2)
    {
    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	af_first = &pObj->affect_first;
	af_last = &pObj->affect_last;
	level = pObj->level;
      }
      break;
    default:
      olc_msg (ch, table->field, "bad editor");
      return false;
    }

  argument = one_argument (argument, loc);
  one_argument (argument, mod);

  if (NullStr (loc) || NullStr (mod) || !is_number (mod))
    {
      cmd_syntax (ch, NULL, table->field, "<location> <#mod>", NULL);
      return false;
    }

  if ((value = (apply_t) flag_value (apply_flags, loc)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "Valid affects are:");
      show_olc_help (table->field, ch, "apply");
      return false;
    }

  pAf = new_affect ();
  pAf->location = value;
  pAf->modifier = atoi (mod);
  pAf->where = TO_OBJECT;
  pAf->type = -1;
  pAf->duration = -1;
  pAf->bitvector = 0;
  pAf->level = level;
  Link (pAf, *af, next, prev);

  olc_msg (ch, table->field, "Affect added.");
  return true;
}

Ed_Fun (olced_addapply)
{
  apply_t value;
  where_t typ;
  flag_t bv;
  int level;
  AffectData *pAf, **af_first, **af_last;
  char loc[MAX_STRING_LENGTH];
  char mod[MAX_STRING_LENGTH];
  char type[MAX_STRING_LENGTH];
  char bvector[MAX_STRING_LENGTH];

  switch ((int) table->arg2)
    {
    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	af_first = &pObj->affect_first;
	af_last = &pObj->affect_last;
	level = pObj->level;
      }
      break;
    default:
      olc_msg (ch, table->field, "bad editor");
      return false;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, table->field,
		  "<type> <location> <#mod> <bitvector>", NULL);
      return false;
    }
  argument = one_argument (argument, type);
  argument = one_argument (argument, loc);
  argument = one_argument (argument, mod);
  one_argument (argument, bvector);

  if (NullStr (type)
      || (typ = (where_t) flag_value (apply_types, type)) == NO_FLAG)
    {
      olc_msg (ch, table->field,
	       "Invalid apply type. Valid apply types are:");
      show_olc_help (table->field, ch, "apptype");
      return false;
    }

  if (NullStr (loc)
      || (value = (apply_t) flag_value (apply_flags, loc)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "Valid applys are:");
      show_olc_help (table->field, ch, "apply");
      return false;
    }

  if (NullStr (bvector) ||
      (bv = flag_value (bitvector_type[typ].table, bvector)) == NO_FLAG)
    {
      olc_msg (ch, table->field, "Invalid bitvector type.");
      olc_msg (ch, table->field, "Valid bitvector types are:");
      show_olc_help (table->field, ch, bitvector_type[typ].help);
      return false;
    }

  if (NullStr (mod) || !is_number (mod))
    {
      cmd_syntax (ch, NULL, table->field,
		  "[type] [location] [#xmod] [bitvector]", NULL);
      return false;
    }

  pAf = new_affect ();
  pAf->location = value;
  pAf->modifier = atoi (mod);
  pAf->where = typ;
  pAf->type = -1;
  pAf->duration = -1;
  pAf->bitvector = bv;
  pAf->level = level;
  Link (pAf, *af, next, prev);

  olc_msg (ch, table->field, "Apply added.");
  return true;
}


Ed_Fun (olced_delaffect)
{
  AffectData *pAf, **af_first, **af_last;
  char affect[MAX_STRING_LENGTH];
  int value;
  int cnt = -1;

  switch ((int) table->arg2)
    {
    case FIELD_OBJINDEX:
      {
	ObjIndex *pObj = (ObjIndex *) base;

	af_first = &pObj->affect_first;
	af_last = &pObj->affect_last;
      }
      break;
    default:
      olc_msg (ch, table->field, "bad editor");
      return false;
    }

  one_argument (argument, affect);

  if (!is_number (affect) || NullStr (affect))
    {
      cmd_syntax (ch, NULL, table->field, "<number>", NULL);
      return false;
    }

  value = atoi (affect);

  if (value < 0)
    {
      olc_msg (ch, table->field, "Only non-negative affect-numbers allowed.");
      return false;
    }

  for (pAf = *af_first; pAf != NULL; pAf = pAf->next)
    if (++cnt == value)
      break;

  if (!pAf)
    {
      olc_msg (ch, table->field, "Non-existant affect.");
      return false;
    }

  UnLink (pAf, *af, next, prev);
  free_affect (pAf);

  olc_msg (ch, table->field, "Affect removed.");
  return true;
}


Ed_Fun (olced_sound)
{
  MspData **msp = (MspData **) data;
  char command[MAX_INPUT_LENGTH];

  argument = first_arg (argument, command, false);

  if (NullStr (command))
    {
      cmd_syntax (ch, NULL, table->field, "add [file]",
		  "remove", "rename [new]", "type [cat]", "to [flag]",
		  "volume [#]", "loop [#]", "priority [#]",
		  "restart [true/false]", "url [addr]", NULL);
      return false;
    }

  if (!str_cmp (command, "add"))
    {
      char file[MIL];
      int pos;

      if (msp && *msp)
	{
	  olc_msg (ch, table->field,
		   "You have to remove the current sound first.");
	  return false;
	}

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "add [file]", NULL);
	  return false;
	}

      for (file[0] = NUL, pos = 0; *argument; argument++)
	{

	  if (isdigit (*argument))
	    {
	      do
		{
		  argument++;
		}
	      while (isdigit (*argument));
	      file[pos++] = '*';
	    }
	  else
	    file[pos++] = *argument;
	}
      file[pos] = NUL;

      *msp = new_msp ();
      replace_str (&(*msp)->file, file);
      olc_msg (ch, table->field, "Sound added.");
      return true;
    }

  if (!msp || !*msp)
    {
      olc_msg (ch, table->field, "You have to add a sound first.");
      return false;
    }

  if (!str_cmp (command, "type"))
    {
      int cat;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "type [cat]", NULL);
	  chprintln (ch, "Valid types are:");
	  show_flags (ch, msp_types);
	  return false;
	}

      if ((cat = flag_value (msp_types, argument)) == NO_FLAG)
	{
	  olc_msg (ch, table->field, "No such sound type.");
	  chprintln (ch, "Valid types are:");
	  show_flags (ch, msp_types);
	  return false;
	}

      (*msp)->type = (msp_t) cat;
      olc_msg (ch, table->field, "Sound type set.");
      return true;
    }

  if (!str_cmp (command, "to"))
    {
      flag_t to;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "to [flag]", NULL);
	  chprintln (ch, "Valid flags are:");
	  show_flags (ch, to_flags);
	  return false;
	}

      if ((to = flag_value (to_flags, argument)) == NO_FLAG)
	{
	  olc_msg (ch, table->field, "No such flag.");
	  chprintln (ch, "Valid flags are:");
	  show_flags (ch, to_flags);
	  return false;
	}

      ToggleBit ((*msp)->to, to);
      olc_msg (ch, table->field, "%s flag(s) toggled.",
	       flag_string (to_flags, to));
      return true;
    }

  if (!str_cmp (command, "delete"))
    {
      free_msp (*msp);

      olc_msg (ch, table->field, "sound deleted.");
      return true;
    }

  if (!str_cmp (command, "volume"))
    {
      int v;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "volume [#]", NULL);
	  chprintln (ch, "Use -1 for infinite loop.");
	  return false;
	}

      v = atoi (argument);
      if (v < 0 || v > 100)
	{
	  olc_msg (ch, table->field, "Please keep volume between 0 and 100.");
	  return false;
	}

      (*msp)->volume = v;
      olc_msg (ch, table->field, "Sound volume set.");
      return true;
    }

  if (!str_cmp (command, "rename"))
    {
      replace_str (&(*msp)->file, argument);

      olc_msg (ch, table->field, "Sound file renamed.");
      return true;
    }

  if (!str_cmp (command, "loop"))
    {
      int l;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "loop [#]", NULL);
	  chprintln (ch, "Use -1 for infinite loop.");
	  return false;
	}

      l = atoi (argument);
      if (l < -1 || l > 100)
	{
	  olc_msg (ch, table->field, "Please keep loops between -1 and 100.");
	  return false;
	}

      (*msp)->loop = l;
      olc_msg (ch, table->field, "Sound loops set.");
      return true;
    }
  if (!str_cmp (command, "priority"))
    {
      int p;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "priority [#]", NULL);
	  return false;
	}

      p = atoi (argument);
      if (p < 0 || p > 100)
	{
	  olc_msg (ch, table->field,
		   "Please keep priority between 0 and 100.");
	  return false;
	}

      (*msp)->priority = p;
      olc_msg (ch, table->field, "Sound priority set.");
      return true;
    }
  if (!str_cmp (command, "restart"))
    {
      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "restart [yes/no]", NULL);
	  chprintln (ch, "Restart sound after being interupted?");
	  return false;
	}

      (*msp)->restart = !str_cmp (argument, "yes");
      olc_msg (ch, table->field, "Sound restarting set.");
      return true;
    }
  if (!str_cmp (command, "url"))
    {
      char url[MIL];
      int len;

      if (NullStr (argument))
	{
	  cmd_syntax (ch, NULL, table->field, "url [addr]", NULL);
	  return false;
	}

      if (!str_prefix (argument, "http://"))
	strcpy (url, argument);
      else
	sprintf (url, "http://%s", argument);

      len = strlen (url);
      if (url[len - 1] != '/')
	strcat (url, "/");

      replace_str (&(*msp)->url, url);
      olc_msg (ch, table->field, "Sound url set.");
      return true;
    }

  return olced_sound (table, ch, base, "", data);
}

#endif
