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
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
*  This code was freely distributed with the The Isles 1.1 source code,   *
*  and has been used here for OLC - OLC would not be what it is without   *
*  all the previous coders who released their source code.                *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/



#include "merc.h"
#include "tables.h"
#include "olc.h"
#include "interp.h"
#include "recycle.h"
#include "data_table.h"

#define ED_TABLE(a, b, c, d, e, f, g) {a, b, c, &e##_zero, do_##d, e##_data_table, \
                                        d##_create, d##_delete, d##_list, rw_##e##_data, \
                                        &e##_zero.f, g}
#define ED_LIST(a, b, c, d, e, f, g) {a, b, c, &e##_zero, do_##d, e##_data_table, \
                                        d##_create, d##_delete, d##_list, NULL, &e##_zero.f, g}
#define ED_BASIC(a, b, c, d, e, f, g) {a, b, c, &e##_zero, do_##d, e##_data_table, NULL, NULL, NULL, rw_##e##_data, &e##_zero.f, g}

OlcTable olc_ed_table[ED_MAX] = {
  ED_LIST ("AEdit", "area", ED_AREA, aedit, area, vnum, FIELD_VNUM),
  ED_LIST ("REdit", "room", ED_ROOM, redit, room_index, vnum, FIELD_VNUM),
  ED_LIST ("OEdit", "object", ED_OBJECT, oedit, obj_index, vnum,
	   FIELD_VNUM),
  ED_LIST ("MEdit", "mobile", ED_MOBILE, medit, char_index, vnum,
	   FIELD_VNUM),
  ED_LIST ("HEdit", "help", ED_HELP, hedit, help, keyword, FIELD_STRING),
  ED_LIST ("MPEdit", "mob-program", ED_MPCODE, mpedit, mprog, vnum,
	   FIELD_VNUM),
  ED_LIST ("OPEdit", "obj-program", ED_OPCODE, opedit, oprog, vnum,
	   FIELD_VNUM),
  ED_LIST ("RPEdit", "room-program", ED_RPCODE, rpedit, rprog, vnum,
	   FIELD_VNUM),
  ED_TABLE ("CEdit", "clan", ED_CLAN, cedit, clan, name, FIELD_STRING),
  ED_TABLE ("RAEdit", "race", ED_RACE, raedit, race, name, FIELD_STRING),
  ED_TABLE ("SEdit", "social", ED_SOCIAL, sedit, social, name,
	    FIELD_STRING),
  ED_TABLE ("SKEdit", "skill", ED_SKILL, skedit, skill, name,
	    FIELD_STRING),
  ED_TABLE ("GREdit", "group", ED_GROUP, gredit, group, name,
	    FIELD_STRING),
  ED_TABLE ("DEdit", "deity", ED_DEITY, dedit, deity, name, FIELD_STRING),
  ED_BASIC ("MudEdit", "mud", ED_Mud, mudedit, mud, name, FIELD_STRING),
  ED_TABLE ("CmdEdit", "command", ED_CMD, cmdedit, cmd, name,
	    FIELD_STRING),
  ED_TABLE ("ClEdit", "class", ED_CLASS, cledit, class, name,
	    FIELD_STRING),
  ED_TABLE ("ChanEdit", "channel", ED_CHAN, chanedit, channel, name,
	    FIELD_STRING),
  ED_TABLE ("SongEdit", "song", ED_SONG, songedit, song, name,
	    FIELD_STRING),
};

OlcTable *
olc_lookup (ed_t ed)
{
  int i;

  if (ed == ED_NONE)
    return NULL;

  for (i = 0; i < ED_MAX; i++)
    if (olc_ed_table[i].ed == ed)
      return &olc_ed_table[i];

  return NULL;
}

bool
olc_show (CharData * ch, OlcTable * ed)
{
  Buffer *output;

  if (!ch->desc || !ch->desc->pEdit || !ed->table || ch->desc->pString)
    return false;

  output = new_buf ();
  bprintln (output, stringf (ch, 0, Center, "=",
			     "[ %s : %s ]", ed->name,
			     olc_ed_vnum (ch->desc)));
  show_struct (ch, output, ed->type_zero, ed->table, ch->desc->pEdit, 0);
  bprintln (output, draw_line (ch, "=", 0));
  sendpage (ch, buf_string (output));
  free_buf (output);
  return true;
}

Olc_Fun (olced_show)
{
  OlcTable *ed;

  if (!ch->desc || ch->desc->editor == ED_NONE
      || (ed = olc_lookup (ch->desc->editor)) == NULL)
    return false;

  return olc_show (ch, ed);
}

Olc_Fun (olced_create)
{
  OlcTable *ed;

  if (!ch->desc || ch->desc->editor == ED_NONE
      || (ed = olc_lookup (ch->desc->editor)) == NULL || !ed->create_fun)
    return false;

  return (*ed->create_fun) (n_fun, ch, argument);
}

Olc_Fun (olced_delete)
{
  OlcTable *ed;

  if (!ch->desc || ch->desc->editor == ED_NONE
      || (ed = olc_lookup (ch->desc->editor)) == NULL || !ed->delete_fun)
    return false;

  return (*ed->delete_fun) (n_fun, ch, argument);
}

Olc_Fun (olced_list)
{
  OlcTable *ed;

  if (!ch->desc || ch->desc->editor == ED_NONE
      || (ed = olc_lookup (ch->desc->editor)) == NULL || !ed->list_fun)
    return false;

  return (*ed->list_fun) (n_fun, ch, argument);
}


AreaData *
get_olc_area (Descriptor * d)
{
  if (d->editor == ED_NONE || d->pEdit == NULL)
    return NULL;

  switch (d->editor)
    {
    case ED_AREA:
      return (AreaData *) d->pEdit;
    case ED_ROOM:
      return ((RoomIndex *) d->pEdit)->area;
    case ED_OBJECT:
      return ((ObjIndex *) d->pEdit)->area;
    case ED_MOBILE:
      return ((CharIndex *) d->pEdit)->area;
    case ED_MPCODE:
    case ED_OPCODE:
    case ED_RPCODE:
      return ((ProgCode *) d->pEdit)->area;
    default:
      return NULL;
    }
}

void
olcedit (CharData * ch, const char *argument, OlcTable * table)
{
  AreaData *pArea;

  pArea = get_olc_area (ch->desc);

  if (pArea && !IsBuilder (ch, pArea))
    {
      chprintlnf (ch, "%s:  Insufficient security to modify %s.",
		  table->name, table->longname);
      edit_done (ch);
      return;
    }

  if (!str_cmp (argument, "done"))
    {
      edit_done (ch);
      return;
    }

  if (emptystring (argument))
    {
      olc_show (ch, table);
      return;
    }

  if (!table->table
      || !edit_struct (ch, argument, table->type_zero, table->table,
		       ch->desc->pEdit))
    interpret (ch, argument);
  else if (table->save_fun)
    {
      (*table->save_fun) (act_write);
      if (pArea)
	SetBit (pArea->area_flags, AREA_CHANGED);
    }
  return;
}

bool
run_olc_editor (Descriptor * d, char *incomm)
{
  OlcTable *ed;

  if ((ed = olc_lookup (d->editor)) == NULL)
    return false;

  olcedit (CH (d), incomm, ed);
  return true;
}

const char *
olc_ed_name (Descriptor * d)
{
  OlcTable *ed;

  if (!d)
    return &str_empty[0];

  if ((ed = olc_lookup (d->editor)) == NULL)
    return &str_empty[0];

  return ed->name;
}

const char *
olc_ed_name_long (CharData * ch)
{
  OlcTable *ed;

  if (!ch->desc)
    return "Switched";

  if (IsNPC (ch))
    return &str_empty[0];

  if ((ed = olc_lookup (ch->desc->editor)) == NULL)
    return &str_empty[0];

  return ed->longname;
}

char *
olc_ed_vnum (Descriptor * d)
{
  OlcTable *ed;
  const char **str;
  vnum_t *vnum;
  long *l;
  int *i;

  if (!d || (ed = olc_lookup (d->editor)) == NULL)
    return &str_empty[0];

  switch (ed->field)
    {
    case FIELD_STRING:
      rwgetdata (str, const char *, ed->vnum, ed->type_zero, d->pEdit);

      return FORMATF ("%.10s", *str);
    case FIELD_INT:
    default:
      rwgetdata (i, int, ed->vnum, ed->type_zero, d->pEdit);

      return FORMATF ("%d", *i);
    case FIELD_LONG:
      rwgetdata (l, long, ed->vnum, ed->type_zero, d->pEdit);

      return FORMATF ("%ld", *l);
    case FIELD_VNUM:
      rwgetdata (vnum, vnum_t, ed->vnum, ed->type_zero, d->pEdit);
      return FORMATF ("%ld", *vnum);
    }
}

void
stop_editing (void *OLC)
{
  Descriptor *d;

  for (d = descriptor_first; d != NULL; d = d->next)
    {
      if (d->pEdit == NULL || d->character == NULL)
	continue;

      if (d->pEdit == OLC)
	edit_done (d->character);
    }
  return;
}

void
olc_msg (CharData * ch, const char *n_fun, const char *fmt, ...)
{
  char buf[MPL];
  va_list args;

  if (!ch || !ch->desc || NullStr (fmt))
    return;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  if (ch->desc->editor != ED_NONE)
    chprintlnf (ch, "%s: %s: %s", olc_ed_name (ch->desc), n_fun, buf);
  else
    chprintlnf (ch, "%s: %s", n_fun, buf);
}



Olc_Fun (show_olc_cmds)
{
  int cmd;
  OlcTable *ed;
  Column Cd;
  Buffer *buf;

  if ((ed = olc_lookup (ch->desc->editor)) == NULL || ed->table == NULL)
    {
      bugf ("table NULL, editor %d", ch->desc->editor);
      return false;
    }

  buf = new_buf ();
  set_cols (&Cd, ch, 4, COLS_BUF, buf);

  for (cmd = 0; ed->table[cmd].field != NULL; cmd++)
    {
      if (ed->table[cmd].function)
	print_cols (&Cd, ed->table[cmd].field);
    }

  for (cmd = 0; olc_comm_table[cmd].name != NULL; cmd++)
    {
      print_cols (&Cd, olc_comm_table[cmd].name);
    }

  cols_nl (&Cd);
  sendpage (ch, buf_string (buf));
  free_buf (buf);
  return false;
}

const struct olc_comm_type olc_comm_table[] = {
  {"create", olced_create},
  {"delete", olced_delete},
  {"list", olced_list},
  {"show", olced_show},
  {"commands", show_olc_cmds},
  {"?", show_olc_help},
  {"version", show_olc_version},
  {NULL, NULL}
};


AreaData *
get_area_data (int vnum)
{
  AreaData *pArea;

  for (pArea = area_first; pArea; pArea = pArea->next)
    {
      if (pArea->vnum == vnum)
	return pArea;
    }

  return 0;
}


bool
edit_done (CharData * ch)
{
  if (ch->desc->editor != ED_NONE)
    {
      act ("$n stop using the $t editor.", ch, olc_ed_name_long (ch), NULL,
	   TO_ROOM);
      act ("You stop using the $t editor.", ch, olc_ed_name_long (ch),
	   NULL, TO_CHAR);
    }

  ch->desc->pEdit = NULL;
  ch->desc->editor = ED_NONE;
  return false;
}

ChannelData *
get_chan_data (int chan)
{
  if (chan >= 0 && chan < top_channel)
    return &channel_table[chan];
  else
    return NULL;
}

SongData *
get_song_data (int song)
{
  if (song >= 0 && song < top_song)
    return &song_table[song];
  else
    return NULL;
}

SkillData *
get_skill_data (int skill)
{
  if (skill > -1 && skill < top_skill)
    return &skill_table[skill];

  return &skill_table[0];
}

GroupData *
get_group_data (int group)
{
  if (group > -1 && group < top_group)
    return &group_table[group];

  return &group_table[0];
}

ClassData *
get_class_data (int Class)
{
  if (Class > -1 && Class < top_class)
    return &class_table[Class];

  return &class_table[0];
}

bool
vnum_OK (vnum_t lnum, vnum_t hnum)
{
  int i;

  for (i = 0; vnum_table[i].vnum != -1; i++)
    {
      if (vnum_table[i].vnum >= lnum && vnum_table[i].vnum <= hnum)
	return false;
    }
  return true;
}

bool
check_vnum (vnum_t vnum, int type)
{
  int i;

  for (i = 0; vnum_table[i].vnum != -1; i++)
    {
      if (vnum_table[i].type != type)
	continue;

      if (vnum_table[i].vnum == vnum)
	return false;
    }
  return true;
}

void
display_resets (CharData * ch)
{
  RoomIndex *pRoom;
  ResetData *pReset;
  CharIndex *pMob = NULL;
  int iReset = 0;

  EditRoom (ch, pRoom);

  chprintln (ch,
	     " No.  Loads    Description       Location         Vnum   Mx Mn Description"
	     NEWLINE
	     "==== ======== ============= =================== ======== ===== ===========");

  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
    {
      ObjIndex *pObj;
      CharIndex *pMobIndex;
      ObjIndex *pObjIndex;
      ObjIndex *pObjToIndex;
      RoomIndex *pRoomIndex;

      chprintf (ch, "[%2d] ", ++iReset);

      switch (pReset->command)
	{
	default:
	  chprintlnf (ch, "Bad reset command: %c.", pReset->command);
	  break;

	case 'M':
	  if (!(pMobIndex = get_char_index (pReset->arg1)))
	    {
	      chprintlnf (ch, "Load Mobile - Bad Mob %ld", pReset->arg1);
	      continue;
	    }

	  if (!(pRoomIndex = get_room_index (pReset->arg3)))
	    {
	      chprintlnf (ch, "Load Mobile - Bad Room %ld", pReset->arg3);
	      continue;
	    }

	  pMob = pMobIndex;


	  {
	    RoomIndex *pRoomIndexPrev;

	    pRoomIndexPrev = get_room_index (pRoomIndex->vnum - 1);
	    if (pRoomIndexPrev
		&& IsSet (pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
	      chprintlnf (ch,
			  "P[%5ld] %-13.13s in room             R[%5ld] %2d-%2d %-15.15s",
			  pReset->arg1, pMob->short_descr,
			  pReset->arg3, pReset->arg2,
			  pReset->arg4, pRoomIndex->name);
	    else
	      chprintlnf (ch,
			  "M[%5ld] %-13.13s in room             R[%5ld] %2d-%2d %-15.15s",
			  pReset->arg1, pMob->short_descr,
			  pReset->arg3, pReset->arg2, pReset->arg4,
			  pRoomIndex->name);
	  }

	  break;

	case 'O':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      chprintlnf (ch, "Load Object - Bad Object %ld", pReset->arg1);
	      continue;
	    }

	  pObj = pObjIndex;

	  if (!(pRoomIndex = get_room_index (pReset->arg3)))
	    {
	      chprintlnf (ch, "Load Object - Bad Room %ld", pReset->arg3);
	      continue;
	    }

	  chprintlnf (ch,
		      "O[%5ld] %-13.13s in room             "
		      "R[%5ld]       %-15.15s", pReset->arg1,
		      pObj->short_descr, pReset->arg3, pRoomIndex->name);

	  break;

	case 'P':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      chprintlnf (ch, "Put Object - Bad Object %ld", pReset->arg1);
	      continue;
	    }

	  pObj = pObjIndex;

	  if (!(pObjToIndex = get_obj_index (pReset->arg3)))
	    {
	      chprintlnf (ch, "Put Object - Bad To Object %ld", pReset->arg3);
	      continue;
	    }

	  chprintlnf (ch,
		      "O[%5ld] %-13.13s inside              O[%5ld] %2d-%2d %-15.15s",
		      pReset->arg1, pObj->short_descr, pReset->arg3,
		      pReset->arg2, pReset->arg4, pObjToIndex->short_descr);

	  break;

	case 'G':
	case 'E':
	  if (!(pObjIndex = get_obj_index (pReset->arg1)))
	    {
	      chprintlnf (ch,
			  "Give/Equip Object - Bad Object %ld", pReset->arg1);
	      continue;
	    }

	  pObj = pObjIndex;

	  if (!pMob)
	    {
	      chprintln (ch, "Give/Equip Object - No Previous Mobile");
	      break;
	    }

	  if (pMob->pShop)
	    {
	      chprintlnf (ch,
			  "O[%5ld] %-13.13s in the inventory of S[%5ld]       %-15.15s",
			  pReset->arg1, pObj->short_descr,
			  pMob->vnum, pMob->short_descr);
	    }
	  else
	    chprintlnf (ch,
			"O[%5ld] %-13.13s %-19.19s M[%5ld]       %-15.15s",
			pReset->arg1, pObj->short_descr,
			(pReset->command ==
			 'G') ? "in the inventory" :
			flag_string (wear_loc_strings, pReset->arg3),
			pMob->vnum, pMob->short_descr);

	  break;


	case 'F':
	  pRoomIndex = get_room_index (pReset->arg1);
	  chprintlnf (ch,
		      "R[%5ld] %s door of %-19.19s reset to %s",
		      pReset->arg1,
		      capitalize (dir_name[pReset->arg2]),
		      pRoomIndex->name, flag_string (exit_flags,
						     pRoomIndex->exit
						     [pReset->
						      arg2]->rs_flags));
	  break;

	case 'R':
	  if (!(pRoomIndex = get_room_index (pReset->arg1)))
	    {
	      chprintlnf (ch, "Randomize Exits - Bad Room %ld", pReset->arg1);
	      continue;
	    }

	  chprintlnf (ch, "R[%5ld] Set to randomize %s %s.",
		      pReset->arg1,
		      pReset->arg3 == 0 ? "exits" : pReset->arg2 ==
		      0 ? "any available exit" : dir_name[pReset->arg2 -
							  1],
		      pReset->arg3 == 0 ? "in room" : pReset->arg3 ==
		      1 ? "to this area" : "to the world");

	  break;
	}
    }

  return;
}


void
add_reset (RoomIndex * room, ResetData * pReset, int pIndex)
{
  ResetData *reset;
  int iReset = 0;

  if (!room)
    return;

  if (!room->reset_first)
    {
      Link (pReset, room->reset, next, prev);
      return;
    }

  for (reset = room->reset_first; reset; reset = reset->next)
    {
      if (++iReset == pIndex)
	break;
    }

  if (reset)
    Insert (pReset, reset, room->reset, next, prev);
  else
    Link (pReset, room->reset, next, prev);
  return;
}

Do_Fun (do_resets)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char arg5[MAX_INPUT_LENGTH];
  char arg6[MAX_INPUT_LENGTH];
  char arg7[MAX_INPUT_LENGTH];
  ResetData *pReset = NULL;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  argument = one_argument (argument, arg4);
  argument = one_argument (argument, arg5);
  argument = one_argument (argument, arg6);
  argument = one_argument (argument, arg7);

  if (!IsBuilder (ch, ch->in_room->area))
    {
      chprintln (ch, "Resets: Invalid security for editing this area.");
      return;
    }


  if (NullStr (arg1))
    {
      if (ch->in_room->reset_first)
	{
	  chprintln (ch, "Resets: M = mobile, R = room, O = object, "
		     "P = pet, S = shopkeeper");
	  display_resets (ch);
	}
      else
	chprintln (ch, "No resets in this room.");
    }


  if (is_number (arg1))
    {
      RoomIndex *pRoom = ch->in_room;


      if (!str_cmp (arg2, "delete"))
	{
	  int insert_loc = atoi (arg1);
	  int iReset = -1;

	  if (!ch->in_room->reset_first)
	    {
	      chprintln (ch, "No resets in this area.");
	      return;
	    }
	  for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
	    {
	      if (++iReset == insert_loc - 1)
		break;
	    }

	  if (!pReset)
	    {
	      chprintln (ch, "Reset not found.");
	      return;
	    }
	  UnLink (pReset, pRoom->reset, next, prev);
	  free_reset (pReset);
	  chprintln (ch, "Reset deleted.");
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	}
      else
	if ((!str_cmp (arg2, "mob") && is_number (arg3)) ||
	    (!str_cmp (arg2, "obj") && is_number (arg3)))
	{

	  if (!str_cmp (arg2, "mob"))
	    {
	      if (get_char_index (is_number (arg3) ? atov (arg3) : 1) == NULL)
		{
		  chprintln (ch, "Mob no existe.");
		  return;
		}
	      pReset = new_reset ();
	      pReset->command = 'M';
	      pReset->arg1 = atol (arg3);
	      pReset->arg2 = is_number (arg4) ? atoi (arg4) : 1;
	      pReset->arg3 = ch->in_room->vnum;
	      pReset->arg4 = is_number (arg5) ? atoi (arg5) : 1;
	    }
	  else if (!str_cmp (arg2, "obj"))
	    {
	      pReset = new_reset ();
	      pReset->arg1 = atol (arg3);

	      if (!str_prefix (arg4, "inside"))
		{
		  ObjIndex *temp;

		  temp = get_obj_index (is_number (arg5) ? atov (arg5) : 1);
		  if ((temp->item_type != ITEM_CONTAINER)
		      && (temp->item_type != ITEM_CORPSE_NPC))
		    {
		      chprintln (ch, "Objeto 2 no es container.");
		      return;
		    }
		  pReset->command = 'P';
		  pReset->arg2 = is_number (arg6) ? atoi (arg6) : 1;
		  pReset->arg3 = is_number (arg5) ? atol (arg5) : 1;
		  pReset->arg4 = is_number (arg7) ? atoi (arg7) : 1;
		}
	      else if (!str_cmp (arg4, "room"))
		{
		  if (get_obj_index (atov (arg3)) == NULL)
		    {
		      chprintln (ch, "Vnum no existe.");
		      return;
		    }
		  pReset->command = 'O';
		  pReset->arg2 = 0;
		  pReset->arg3 = ch->in_room->vnum;
		  pReset->arg4 = 0;
		}
	      else

		{
		  int wear;

		  if ((wear = flag_value (wear_loc_flags, arg4)) == NO_FLAG)
		    {
		      chprintln (ch, "Resets: '? wear-loc'");
		      return;
		    }
		  if (get_obj_index (atov (arg3)) == NULL)
		    {
		      chprintln (ch, "Vnum no existe.");
		      return;
		    }
		  pReset->arg1 = atol (arg3);
		  pReset->arg3 = wear;
		  if (pReset->arg3 == WEAR_NONE)
		    pReset->command = 'G';
		  else
		    pReset->command = 'E';
		}
	    }
	  add_reset (ch->in_room, pReset, atol (arg1));
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  chprintln (ch, "Reset added.");
	}
      else if (!str_cmp (arg2, "random"))
	{
	  int door;

	  if (is_number (arg3))
	    {
	      door = atoi (arg3);
	    }
	  else
	    {
	      if ((door = get_direction (arg3)) != -1)
		door++;
	    }

	  if (door < 0 || door > MAX_DIR)
	    {
	      chprintln (ch, "Invalid argument.");
	      return;
	    }

	  if (door == 0
	      && (NullStr (arg4) || !is_exact_name (arg4, "area world")))
	    {
	      chprintln (ch, "Invalid or missing argument.");
	      return;
	    }
	  if (is_exact_name (arg4, "area world") && door > 0
	      && ch->in_room->exit[door - 1] != NULL)
	    {
	      chprintln (ch, "There is already an exit in that direction.");
	      return;
	    }
	  pReset = new_reset ();
	  pReset->command = 'R';
	  pReset->arg1 = ch->in_room->vnum;
	  pReset->arg2 = door;
	  pReset->arg3 =
	    !str_cmp (arg4, "area") ? 1 : !str_cmp (arg4, "world") ? 2 : 0;
	  add_reset (ch->in_room, pReset, atoi (arg1));
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  chprintln (ch, "Random exit reset added.");
	}
      else
	{
	  cmd_syntax (ch, NULL, n_fun, "<number> OBJ <vnum> <wear_loc>",
		      "<number> OBJ <vnum> inside <vnum> [limit] [count]",
		      "<number> OBJ <vnum> room",
		      "<number> MOB <vnum> [max #x area] [max #x room]",
		      "<number> DELETE",
		      "<number> RANDOM [#x exits]",
		      "<number> RANDOM <exit|0> area|world", NULL);
	}
    }

  return;
}

int
compare_area (const void *v1, const void *v2)
{
  return (*(AreaData * *)v2)->min_vnum - (*(AreaData * *)v1)->min_vnum;
}


Do_Fun (do_alist)
{
  Buffer *output;
  AreaData **areas;
  AreaData *pArea;
  int i = 0;

  output = new_buf ();

  alloc_mem (areas, AreaData *, top_area);

  bprintlnf (output, "[%3s] %-25s [%-5s-%5s] [%-11s] %3s [%-10s] [%-10s]",
	     "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec",
	     "Builders", "Flags");

  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    areas[i++] = pArea;

  qsort (areas, i, sizeof (pArea), compare_area);

  while (--i >= 0)
    {
      bprintlnf
	(output,
	 "[%3d] %-25s [%-5ld-%5ld] %-13.13s [%1d] [%-10s] [%-10s]",
	 areas[i]->vnum, areas[i]->name, areas[i]->min_vnum,
	 areas[i]->max_vnum, areas[i]->file_name, areas[i]->security,
	 areas[i]->builders, flag_string (area_flags, areas[i]->area_flags));
    }
  bprintlnf (output, NEWLINE "Total areas: %d", top_area);
  sendpage (ch, buf_string (output));
  free_buf (output);
  free_mem (areas);
  return;
}

int
calc_avedam (int num_dice, int dam_dice)
{
  return ((1 + dam_dice) * num_dice / 2);
}

Do_Fun (do_avedam)
{
  int i, j;
  int k = 0;
  int lookingfor;

  if (NullStr (argument))
    {
      chprintln (ch, "What average dam are you looking for?");
      return;
    }

  lookingfor = atoi (argument);
  chprintlnf (ch, "Possible dice combinations for ave dam: %d", lookingfor);
  chprintln (ch, "T_Dice D_Dice   T_Dice D_Dice");
  chprintln (ch, "------ ------   ------ ------");
  for (i = 1; i <= lookingfor; i++)
    {
      for (j = 1; j <= lookingfor; j++)
	{
	  if (lookingfor == calc_avedam (i, j))
	    {
	      k++;
	      chprintf (ch, "%6d %6d   ", i, j);
	      if (k % 2 == 0)
		chprintln (ch, NULL);
	    }
	}
    }
  chprintln (ch, NULL);
}

void
edit_start (CharData * ch, void *OLC, ed_t Ed)
{
  ed_t old_ed;
  OlcTable *t;

  if (!ch || !ch->desc || !(t = olc_lookup (Ed)))
    return;

  old_ed = ch->desc->editor;

  ch->desc->pEdit = OLC;
  ch->desc->editor = Ed;
  if (old_ed != Ed)
    {
      act ("$n has entered the $t editor.", ch, t->longname, NULL, TO_ROOM);
      act ("You are now entering the $t editor, type 'done' to finish.",
	   ch, t->longname, NULL, TO_CHAR);
    }
  if (Ed == ED_ROOM && OLC != NULL)
    {
      char_from_room (ch);
      char_to_room (ch, (RoomIndex *) OLC);
    }
  olc_show (ch, t);
}

void
autoset (CharIndex * pMob)
{
  double roll, size, bonus, avg;
  double ac_n, ac_x;
  int level;

  if (!pMob || pMob->level < 1)
    return;

  level = pMob->level;

  bonus = Max (1, (number_fuzzier (level) - 5));
  ac_n =
    80 - (level * (7.77 + (level / 90))) - ((level / 9) ^ 2) - (bonus * 2);
  ac_x =
    80 - (level * (6.67 + (level / 90))) - ((level / 9) ^ 2) - (bonus * 2);
  pMob->ac[AC_PIERCE] = (int) ac_n;
  pMob->ac[AC_BASH] = (int) ac_n;
  pMob->ac[AC_SLASH] = (int) ac_n;
  pMob->ac[AC_EXOTIC] = (int) ac_x;

  pMob->mana[DICE_NUMBER] = number_fuzzier (level);
  pMob->mana[DICE_TYPE] = number_fuzzier (10);
  pMob->mana[DICE_BONUS] = number_fuzzier (85);

  avg = (number_fuzzier (level) * .84);
  roll = (number_fuzzier (level) / 9 + 1);
  size = (number_fuzzier ((int) roll) / 3);

  for (size = roll / 3; roll * (size + 2) / 2 < avg; size++)
    ;

  pMob->damage[DICE_NUMBER] = (int) Max (1, roll);
  pMob->damage[DICE_TYPE] = (int) Max (2, size);
  pMob->damage[DICE_BONUS] = (int) Max (1, number_fuzzier (level) * .89);

  bonus =
    Max (1,
	 (level / 53 + 1) * ((level * 10) + (level / 10)) - Min (20,
								 (2 *
								  level))
	 + 1);

  size = number_fuzzier (level) * 1.2 - number_fuzzier (1);

  roll = number_fuzzier (level) * 1.4 - number_fuzzier (2);

  pMob->hit[DICE_NUMBER] = (int) Max (1, roll);
  pMob->hit[DICE_TYPE] = (int) Max (2, size);
  pMob->hit[DICE_BONUS] = (int) Max (10, bonus);

  pMob->hitroll = number_range (level * 6 / 10, level);
}

void
autohard (CharIndex * pMob)
{
  double roll, size, bonus, avg;
  double ac_n, ac_x;
  int level;

  if (!pMob || pMob->level < 1)
    return;

  level = pMob->level;

  bonus = Max (1, (number_fuzzier (level) - 5));
  ac_n =
    70 - (level * (8.15 + (level / 90))) - ((level / 8) ^ 2) - (bonus * 2);
  ac_x =
    70 - (level * (7.12 + (level / 90))) - ((level / 8) ^ 2) - (bonus * 2);
  pMob->ac[AC_PIERCE] = (int) ac_n;
  pMob->ac[AC_BASH] = (int) ac_n;
  pMob->ac[AC_SLASH] = (int) ac_n;
  pMob->ac[AC_EXOTIC] = (int) ac_x;

  pMob->mana[DICE_NUMBER] = number_fuzzier (level);
  pMob->mana[DICE_TYPE] = number_fuzzier (12);
  pMob->mana[DICE_BONUS] = number_fuzzier (95);

  avg = (number_fuzzier (level) * .9);
  roll = (number_fuzzier (level) / 9 + 1);
  size = (number_fuzzier ((int) roll) / 3);

  for (size = roll / 3; roll * (size + 2) / 2 < avg; size++)
    ;

  pMob->damage[DICE_NUMBER] = (int) Max (1, roll);
  pMob->damage[DICE_TYPE] = (int) Max (2, size);
  pMob->damage[DICE_BONUS] = (int) Max (1, number_fuzzier (level) * .99);

  bonus = (level / 53 + 1) * ((level * 10) + (level / 10)) + 1;

  size = number_fuzzier (level) * 1.2 - number_fuzzier (1);

  size++;

  roll = number_fuzzier (level) * 1.4 - number_fuzzier (2);

  pMob->hit[DICE_NUMBER] = (int) Max (1, roll);
  pMob->hit[DICE_TYPE] = (int) Max (2, size);
  pMob->hit[DICE_BONUS] = (int) Max (10, bonus);

  pMob->hitroll = number_range (level * 65 / 100, level);
}

void
autoeasy (CharIndex * pMob)
{
  double roll, size, bonus, avg;
  double ac_n, ac_x;
  int level;

  if (!pMob || pMob->level < 1)
    return;

  level = pMob->level;

  bonus = Max (1, (number_fuzzier (level) - 5));
  ac_n =
    90 - (level * (7.47 + (level / 90))) - ((level / 10) ^ 2) - (bonus * 2);
  ac_x =
    90 - (level * (6.37 + (level / 90))) - ((level / 10) ^ 2) - (bonus * 2);
  pMob->ac[AC_PIERCE] = (int) ac_n;
  pMob->ac[AC_BASH] = (int) ac_n;
  pMob->ac[AC_SLASH] = (int) ac_n;
  pMob->ac[AC_EXOTIC] = (int) ac_x;

  pMob->mana[DICE_NUMBER] = number_fuzzier (level);
  pMob->mana[DICE_TYPE] = number_fuzzier (10);
  pMob->mana[DICE_BONUS] = number_fuzzier (60);

  avg = (number_fuzzier (level) * .76);
  roll = (number_fuzzier (level) / 9 + 1);
  size = (number_fuzzier ((int) roll) / 3);

  for (size = roll / 3; roll * (size + 2) / 2 < avg; size++)
    ;

  pMob->damage[DICE_NUMBER] = (int) Max (1, roll);
  pMob->damage[DICE_TYPE] = (int) Max (2, size);
  pMob->damage[DICE_BONUS] = (int) Max (1, number_fuzzier (level) * .82);

  bonus =
    Max (1,
	 (level / 55 + 1) * ((level * 9) + (level / 10)) - Min (20,
								(1.5 *
								 level)) + 1);

  size = number_fuzzier (level) * 1.2 - number_fuzzier (1);

  roll = number_fuzzier (level) * 1.4 - number_fuzzier (2);

  pMob->hit[DICE_NUMBER] = (int) Max (1, roll);
  pMob->hit[DICE_TYPE] = (int) Max (2, size);
  pMob->hit[DICE_BONUS] = (int) Max (10, bonus);

  pMob->hitroll = number_range (level * 59 / 100, level);
}

void
autoweapon (ObjIndex * pObj)
{
  AffectData *pAf, *pObjAf;
  bool bDamroll, bHitroll;
  double pDice, size, bonus;
  int level, mod;

  if (!pObj || pObj->item_type != ITEM_WEAPON)
    return;

  if (pObj->level < 1 || IsObjStat (pObj, ITEM_QUEST))
    return;

  level = pObj->level;

  bonus = Max (1, number_fuzzier (level) / 9 - 1);
  pDice = (number_fuzzier (level) * .22 + 2);
  size = (number_fuzzier ((int) pDice) / 2);

  for (size = pDice / 2; pDice * (size + 2) / 2 < level; size++)
    ;

  pDice = Max (2, pDice);
  size = Max (3, size);

  switch (pObj->value[0])
    {
    default:
    case WEAPON_EXOTIC:
    case WEAPON_SWORD:
      break;
    case WEAPON_DAGGER:
      pDice = Max (2, pDice - 1);
      size = Max (3, size - 1);
      break;
    case WEAPON_SPEAR:
    case WEAPON_POLEARM:
      size++;
      break;
    case WEAPON_MACE:
    case WEAPON_AXE:
      size = Max (3, size - 1);
      break;
    case WEAPON_FLAIL:
    case WEAPON_WHIP:
      pDice = Max (2, pDice - 1);
      break;
    }

  pDice += 1;
  size -= 1;

  pDice = Max (2, number_fuzzier ((int) pDice));
  size = Max (3, number_fuzzier ((int) size));

  pObj->value[1] = (long) pDice;
  pObj->value[2] = (long) size;

  mod = Max (1, number_range (number_fuzzier ((int) bonus) * 6 / 10,
			      number_fuzzier ((int) bonus)));

  if (mod > 0)
    {
      bDamroll = false;
      bHitroll = false;

      for (pObjAf = pObj->affect_first; pObjAf; pObjAf = pObjAf->next)
	{
	  if (pObjAf->location == APPLY_DAMROLL)
	    {
	      pObjAf->level = level;
	      pObjAf->modifier = mod;
	      bDamroll = true;
	    }
	  else if (pObjAf->location == APPLY_HITROLL)
	    {
	      pObjAf->level = level;
	      pObjAf->modifier = mod;
	      bHitroll = true;
	    }
	}

      if (!bDamroll)
	{
	  pAf = new_affect ();
	  pAf->location = APPLY_DAMROLL;
	  pAf->modifier = mod;
	  pAf->where = TO_OBJECT;
	  pAf->type = -1;
	  pAf->duration = -1;
	  pAf->bitvector = 0;
	  pAf->level = level;
	  Link (pAf, pObj->affect, next, prev);
	}

      if (!bHitroll)
	{
	  pAf = new_affect ();
	  pAf->location = APPLY_HITROLL;
	  pAf->modifier = mod;
	  pAf->where = TO_OBJECT;
	  pAf->type = -1;
	  pAf->duration = -1;
	  pAf->bitvector = 0;
	  pAf->level = level;
	  Link (pAf, pObj->affect, next, prev);
	}
    }
}

void
autoarmor (ObjIndex * pObj)
{
  int size;
  int level;

  if (!pObj || pObj->item_type != ITEM_ARMOR)
    return;

  if (pObj->level < 1 || IsObjStat (pObj, ITEM_QUEST))
    return;

  level = pObj->level;

  size = Max (1, number_fuzzier (level) / 2 + 1);

  pObj->value[0] = size;
  pObj->value[1] = size;
  pObj->value[2] = size;
  pObj->value[3] = (size - 1);
}

void
area_set_auto (AreaData * pArea)
{
  ObjIndex *pObjIndex;
  CharIndex *pMobIndex;
  vnum_t vnum;

  if (!pArea)
    return;

  for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
    {
      if ((pMobIndex = get_char_index (vnum)) != NULL
	  && pMobIndex->level >= 1)
	{
	  switch (pMobIndex->autoset)
	    {
	    case AUTOEASY:
	      autoeasy (pMobIndex);
	      break;
	    case AUTOSET:
	      autoset (pMobIndex);
	      break;
	    case AUTOHARD:
	      autohard (pMobIndex);
	      break;
	    case AUTORANDOM:
	      switch (number_range (1, 3))
		{
		case 1:
		  autoeasy (pMobIndex);
		  break;
		case 2:
		  autoset (pMobIndex);
		  break;
		case 3:
		  autohard (pMobIndex);
		  break;
		}
	      break;
	    case AUTODEFAULT:
	      if (pMobIndex->level < MAX_LEVEL / 10)
		autoeasy (pMobIndex);
	      else if (pMobIndex->level < MAX_LEVEL / 2)
		autoset (pMobIndex);
	      else
		autohard (pMobIndex);
	      break;
	    }
	}

      if ((pObjIndex = get_obj_index (vnum)) != NULL
	  && !IsObjStat (pObjIndex, ITEM_QUEST) && pObjIndex->level >= 1)
	{

	  if (pObjIndex->item_type == ITEM_WEAPON)
	    autoweapon (pObjIndex);
	  else if (pObjIndex->item_type == ITEM_ARMOR)
	    autoarmor (pObjIndex);
	}
    }
}

Do_Fun (do_areaset)
{
  AreaData *pArea;

  if (!str_cmp (argument, "world"))
    {
      for (pArea = area_first; pArea; pArea = pArea->next)
	area_set_auto (pArea);
      chprintln (ch, "World set to default values.");
    }
  else if (ch->in_room != NULL && (pArea = ch->in_room->area) != NULL)
    {
      area_set_auto (pArea);
      chprintlnf (ch, "%s set to default values.", pArea->name);
    }
  else
    cmd_syntax (ch, NULL, n_fun,
		"world       - set the whole world.",
		"            - set your current area.", NULL);

  return;
}


void
find_area_levels (CharData * ch, AreaData * pArea, bool set)
{
  int low = MAX_LEVEL, high = 0;

  if (IsSet (pArea->area_flags, AREA_PLAYER_HOMES))
    {
      low = 0;
      high = MAX_LEVEL;
    }
  else
    {
      vnum_t vnum;
      CharIndex *pMob;
      bool found = false;

      for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
	{
	  if ((pMob = get_char_index (vnum)) != NULL)
	    {
	      if (pMob->level > 0 && pMob->level <= MAX_LEVEL
		  && pMob->pShop == NULL
		  && !IsSet (pMob->act,
			     ACT_TRAIN | ACT_PRACTICE | ACT_IS_HEALER |
			     ACT_PET | ACT_PET | ACT_GAIN))
		{
		  if (pMob->level > high)
		    high = pMob->level;
		  if (pMob->level < low)
		    low = pMob->level;
		  found = true;
		}
	    }
	}
      if (!found)
	{
	  low = 0;
	  high = MAX_LEVEL;
	}
    }
  if (!set)
    chprintlnf (ch, "Recomended levels for %s: %03d - %03d",
		pArea->name, low, high);
  else
    {
      if (low != pArea->min_level || high != pArea->max_level)
	SetBit (pArea->area_flags, AREA_CHANGED);
      pArea->min_level = low;
      pArea->max_level = high;
      chprintlnf (ch, "%s: min level set to %d, max level set to %d.",
		  pArea->name, low, high);
    }
}

Do_Fun (do_arecommend)
{
  AreaData *pArea;
  char arg[MIL];
  bool set = false;

  argument = one_argument (argument, arg);
  set = !str_cmp (argument, "set");

  if (NullStr (arg) || (set = !str_cmp (arg, "set")))
    {
      pArea = ch->in_room->area;

      find_area_levels (ch, pArea, set);
      chprintlnf (ch, "Use '%s all [set]' to see all areas.", n_fun);
      chprintlnf (ch, "Use '%s <area> [set]' to see a specific area.", n_fun);
    }
  else if (!str_cmp (arg, "all"))
    {
      for (pArea = area_first; pArea; pArea = pArea->next)
	find_area_levels (ch, pArea, set);
    }
  else if ((pArea = area_lookup (arg)) != NULL)
    {
      find_area_levels (ch, pArea, set);
    }
  else
    {
      cmd_syntax (ch, NULL, n_fun,
		  "[set]        - recommend levels for current area.",
		  "all [set]    - recommend levels for all areas.",
		  "<area> [set] - recommend levels for a specific area.",
		  NULL);
    }
}

Do_Fun (do_afun_help)
{
  cmd_syntax (ch, NULL, n_fun,
	      "set          - autoset mob/obj values for this area",
	      "links        - find exits in and out of area",
	      "relevel      - change levels on mobs/objs in area",
	      "recommend    - recommend min/max level for area", NULL);
}


Do_Fun (do_relevel)
{
  char arg[MIL], arg2[MIL];
  AreaData *parea;
  int amount;
  bool pAdd = false, pSub = false;
  vnum_t vnum;
  char *relvl_help;
  CharIndex *pmob;
  ObjIndex *pobj;

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);

  relvl_help = "+/- <amount> [area]";
  if (NullStr (arg) || (NullStr (arg2) || !is_number (arg2)))
    {
      cmd_syntax (ch, NULL, n_fun, relvl_help, NULL);
      return;
    }

  if (!str_cmp (arg, "+"))
    pAdd = true;
  else if (!str_cmp (arg, "-"))
    pSub = true;
  else
    {
      cmd_syntax (ch, NULL, n_fun, relvl_help, NULL);
      return;
    }

  if (!NullStr (argument))
    {
      if ((parea = area_lookup (argument)) == NULL)
	{
	  chprintln (ch, "Invalid area to relevel.");
	  return;
	}
    }
  else
    parea = ch->in_room->area;

  amount = atoi (arg2);

  if (amount <= 0 || amount > MAX_LEVEL)
    {
      chprintln (ch, "Invalid amount.");
      return;
    }

  for (vnum = parea->min_vnum; vnum < parea->max_vnum; vnum++)
    {
      if ((pmob = get_char_index (vnum)) != NULL && pmob->level > 0)
	{
	  if (pAdd)
	    pmob->level = Max (1, pmob->level + amount);
	  else if (pSub)
	    pmob->level = Max (1, pmob->level - amount);
	}
      if ((pobj = get_obj_index (vnum)) != NULL && pobj->level > 0)
	{
	  if (pAdd)
	    pobj->level = Range (1, pobj->level + amount, MAX_MORTAL_LEVEL);
	  else if (pSub)
	    pobj->level = Range (1, pobj->level - amount, MAX_MORTAL_LEVEL);
	  switch (pobj->item_type)
	    {
	    case ITEM_POTION:
	    case ITEM_SCROLL:
	    case ITEM_PILL:
	    case ITEM_WAND:
	    case ITEM_STAFF:
	      if (pAdd)
		pobj->value[0] =
		  Range (1, pobj->value[0] + amount, MAX_LEVEL);
	      else if (pSub)
		pobj->value[0] =
		  Range (1, pobj->value[0] - amount, MAX_LEVEL);
	      break;
	    default:
	      break;
	    }
	}
    }
  SetBit (parea->area_flags, AREA_CHANGED);
  area_set_auto (parea);

  chprintln (ch, "Area re-leveled and auto-set.");
  return;
}


Do_Fun (do_alinks)
{
  int i;
  vnum_t vnum;
  RoomIndex *room;
  AreaData *area, *target;

  if (IsNPC (ch) || (ch->in_room == NULL))
    return;
  if (!IsBuilder (ch, ch->in_room->area) && !IsImmortal (ch))
    {
      chprintln (ch, "Huh?");
      return;
    }
  target = ch->in_room->area;

  if (!str_cmp (argument, "in"))
    {
      chprintlnf (ch, NEWLINE "Exits leading IN to %s:",
		  capitalize (ch->in_room->area->name));
      {
	for (area = area_first; area; area = area->next)
	  {
	    if (area != ch->in_room->area)
	      {
		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++)
		  {
		    if ((room = get_room_index (vnum)) != NULL)
		      for (i = 0; i < MAX_DIR; i++)
			{
			  if (room->exit[i] != NULL)
			    if (room->exit[i]->u1.to_room->area == target)
			      {
				chprintlnf (ch,
					    "%-5s from %7ld [%s]",
					    dir_name[i], room->vnum,
					    room->area->name);
			      }
			}
		  }
	      }
	  }
      }
      return;
    }

  if (!str_cmp (argument, "out"))
    {
      chprintlnf (ch, NEWLINE "Exits leading OUT of %s:",
		  capitalize (ch->in_room->area->name));
      for (vnum = target->min_vnum; vnum <= target->max_vnum; vnum++)
	{
	  if ((room = get_room_index (vnum)))
	    {
	      for (i = 0; i < MAX_DIR; i++)
		if (room->exit[i] != NULL)
		  if (room->exit[i]->u1.to_room->area != target)
		    {
		      chprintlnf (ch, "%-5s from %7ld [%s]",
				  dir_name[i], vnum,
				  room->exit[i]->u1.to_room->area->name);
		    }
	    }
	}
      return;
    }

  cmd_syntax (ch, NULL, n_fun, "links <in|out>", NULL);
  return;
}

Do_Fun (do_afun)
{
  vinterpret (ch, n_fun, argument, "links", do_alinks, "set", do_areaset,
	      "relevel", do_relevel, "recommend",
	      do_arecommend, NULL, do_afun_help);
}

#include "olc_dofun.h"
