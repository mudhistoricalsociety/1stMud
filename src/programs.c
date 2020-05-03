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
*  MOBprograms for ROM 2.4 v0.98g (C) M.Nylander 1996                     *
*  Based on MERC 2.2 MOBprograms concept by N'Atas-ha.                    *
*  Written and adapted to ROM 2.4 by                                      *
*          Markku Nylander (markku.nylander@uta.fi)                       *
*  This code may be copied and distributed as per the ROM license.        *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/




#include "merc.h"
#include "tables.h"
#include "prog_cmds.h"
#include "recycle.h"

#define    CHK_RAND    (0)
#define    CHK_MOBHERE     (1)
#define    CHK_OBJHERE     (2)
#define    CHK_MOBEXISTS   (3)
#define    CHK_OBJEXISTS   (4)
#define    CHK_PEOPLE      (5)
#define    CHK_PLAYERS     (6)
#define    CHK_MOBS        (7)
#define    CHK_CLONES      (8)
#define    CHK_ORDER       (9)
#define    CHK_HOUR        (10)
#define    CHK_ISPC        (11)
#define    CHK_ISNPC       (12)
#define    CHK_ISGOOD      (13)
#define    CHK_ISEVIL      (14)
#define    CHK_ISNEUTRAL   (15)
#define    CHK_ISIMMORT    (16)
#define    CHK_ISCHARM     (17)
#define    CHK_ISFOLLOW    (18)
#define    CHK_ISACTIVE    (19)
#define    CHK_ISDELAY     (20)
#define    CHK_ISVISIBLE   (21)
#define    CHK_HASTARGET   (22)
#define    CHK_ISTARGET    (23)
#define    CHK_AFFECTED    (24)
#define    CHK_ACT         (25)
#define    CHK_OFF         (26)
#define    CHK_IMM         (27)
#define    CHK_CARRIES     (28)
#define    CHK_WEARS       (20)
#define    CHK_HAS         (30)
#define    CHK_USES        (31)
#define    CHK_NAME        (32)
#define    CHK_POS         (33)
#define    CHK_CLAN        (34)
#define    CHK_RACE        (35)
#define    CHK_CLASS       (36)
#define    CHK_OBJTYPE     (37)
#define    CHK_VNUM        (38)
#define    CHK_HPCNT       (39)
#define    CHK_ROOM        (40)
#define    CHK_SEX         (41)
#define    CHK_LEVEL       (42)
#define    CHK_ALIGN       (43)
#define    CHK_MONEY       (44)
#define    CHK_OBJVAL0     (45)
#define    CHK_OBJVAL1     (46)
#define    CHK_OBJVAL2     (47)
#define    CHK_OBJVAL3     (48)
#define    CHK_OBJVAL4     (49)
#define    CHK_GRPSIZE     (50)
#define    CHK_ONQUEST     (52)
#define    CHK_HUNTER      (55)
#define    CHK_PLR         (57)
#define    CHK_SKILL       (58)
#define    CHK_WEIGHT      (60)

#define    EVAL_EQ            0
#define    EVAL_GE            1
#define    EVAL_LE            2
#define    EVAL_GT            3
#define    EVAL_LT            4
#define    EVAL_NE            5

const char *fn_keyword[] = {
  "rand",
  "mobhere",
  "objhere",
  "mobexists",
  "objexists",
  "people",
  "players",
  "mobs",
  "clones",
  "order",
  "hour",
  "ispc",
  "isnpc",
  "isgood",
  "isevil",
  "isneutral",
  "isimmort",
  "ischarm",
  "isfollow",
  "isactive",
  "isdelay",
  "isvisible",
  "hastarget",
  "istarget",
  "affected",
  "act",
  "off",
  "imm",
  "carries",
  "wears",
  "has",
  "uses",
  "name",
  "pos",
  "clan",
  "race",
  "class",
  "objtype",
  "vnum",
  "hpcnt",
  "room",
  "sex",
  "level",
  "align",
  "money",
  "objval0",
  "objval1",
  "objval2",
  "objval3",
  "objval4",
  "grpsize",
  "onquest",
  "hunter",
  "plr",
  "skill",
  "weight",
  "\n"
};

const char *fn_evals[] = {
  "==",
  ">=",
  "<=",
  ">",
  "<",
  "!=",
  "\n"
};

int
keyword_lookup (const char **table, char *keyword)
{
  register int i;

  for (i = 0; table[i][0] != '\n'; i++)
    if (!str_cmp (table[i], keyword))
      return (i);
  return -1;
}

int
num_eval (int lval, int oper, int rval)
{
  switch (oper)
    {
    case EVAL_EQ:
      return (lval == rval);
    case EVAL_GE:
      return (lval >= rval);
    case EVAL_LE:
      return (lval <= rval);
    case EVAL_NE:
      return (lval != rval);
    case EVAL_GT:
      return (lval > rval);
    case EVAL_LT:
      return (lval < rval);
    default:
      bug ("invalid oper");
      return 0;
    }
}

CharData *
get_random_char (CharData * mob, ObjData * obj, RoomIndex * room)
{
  CharData *vch, *victim = NULL;
  int now = 0, highest = 0;

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("received multiple prog types");
      return NULL;
    }

  if (mob)
    vch = mob->in_room->person_first;
  else if (obj)
    {
      if (obj->in_room)
	vch = obj->in_room->person_first;
      else
	vch = obj->carried_by->in_room->person_first;
    }
  else
    vch = room->person_first;

  for (; vch; vch = vch->next_in_room)
    {
      if (mob && mob != vch && !IsNPC (vch) && can_see (mob, vch)
	  && (now = number_percent ()) > highest)
	{
	  victim = vch;
	  highest = now;
	}
      else if ((now = number_percent ()) > highest)
	{
	  victim = vch;
	  highest = now;
	}
    }
  return victim;
}

int
count_people_room (CharData * mob, ObjData * obj, RoomIndex * room, int iFlag)
{
  CharData *vch;
  int count;

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("received multiple prog types");
      return 0;
    }

  if (mob)
    vch = mob->in_room->person_first;
  else if (obj)
    {
      if (obj->in_room)
	vch = obj->in_room->person_first;
      else
	vch = obj->carried_by->in_room->person_first;
    }
  else
    vch = room->person_first;

  for (count = 0; vch; vch = vch->next_in_room)
    {
      if (mob)
	{
	  if (mob != vch && (iFlag == 0 || (iFlag == 1 && !IsNPC (vch))
			     || (iFlag == 2 && IsNPC (vch)) || (iFlag == 3
								&&
								IsNPC (mob)
								&&
								IsNPC (vch)
								&&
								mob->
								pIndexData->
								vnum ==
								vch->
								pIndexData->
								vnum)
			     || (iFlag == 4 && is_same_group (mob, vch)))
	      && can_see (mob, vch))
	    count++;
	}
      else if (obj || room)
	{
	  if (iFlag == 0 || (iFlag == 1 && !IsNPC (vch))
	      || (iFlag == 2 && IsNPC (vch)))
	    count++;
	}
    }

  return (count);
}

int
get_order (CharData * ch, ObjData * obj)
{
  CharData *vch;
  ObjData *vobj;
  int i;

  if (ch && obj)
    {
      bug ("received multiple prog types");
      return 0;
    }

  if (ch && !IsNPC (ch))
    return 0;

  if (ch)
    {
      vch = ch->in_room->person_first;
      vobj = NULL;
    }
  else
    {
      vch = NULL;
      if (obj->in_room)
	vobj = obj->in_room->content_first;
      else if (obj->carried_by->in_room->content_first)
	vobj = obj->carried_by->in_room->content_first;
      else
	vobj = NULL;
    }

  if (ch)
    for (i = 0; vch; vch = vch->next_in_room)
      {
	if (vch == ch)
	  return i;

	if (IsNPC (vch) && vch->pIndexData->vnum == ch->pIndexData->vnum)
	  i++;
      }
  else
    for (i = 0; vobj; vobj = vobj->next_content)
      {
	if (vobj == obj)
	  return i;

	if (vobj->pIndexData->vnum == obj->pIndexData->vnum)
	  i++;
      }

  return 0;
}

bool
has_item (CharData * ch, vnum_t vnum, item_t pitem_type, bool fWear)
{
  ObjData *obj;

  for (obj = ch->carrying_first; obj; obj = obj->next_content)
    if ((vnum < 0 || obj->pIndexData->vnum == vnum)
	&& (pitem_type < 0 || obj->pIndexData->item_type == pitem_type)
	&& (!fWear || obj->wear_loc != WEAR_NONE))
      return true;
  return false;
}

bool
get_mob_vnum_room (CharData * ch, ObjData * obj, RoomIndex * room,
		   vnum_t vnum)
{
  CharData *mob;

  if ((ch && obj) || (ch && room) || (obj && room))
    {
      bug ("received multiple prog types");
      return false;
    }

  if (ch)
    mob = ch->in_room->person_first;
  else if (obj)
    {
      if (obj->in_room)
	mob = obj->in_room->person_first;
      else
	mob = obj->carried_by->in_room->person_first;
    }
  else
    mob = room->person_first;

  for (; mob; mob = mob->next_in_room)
    if (IsNPC (mob) && mob->pIndexData->vnum == vnum)
      return true;
  return false;
}

bool
get_obj_vnum_room (CharData * ch, ObjData * obj, RoomIndex * room,
		   vnum_t vnum)
{
  ObjData *vobj;

  if ((ch && obj) || (ch && room) || (obj && room))
    {
      bug ("received multiple prog types");
      return false;
    }

  if (ch)
    vobj = ch->in_room->content_first;
  else if (obj)
    {
      if (obj->in_room)
	vobj = obj->in_room->content_first;
      else
	vobj = obj->carried_by->in_room->content_first;
    }
  else
    vobj = room->content_first;

  for (; vobj; vobj = vobj->next_content)
    if (vobj->pIndexData->vnum == vnum)
      return true;
  return false;
}

int
cmd_eval_mob (vnum_t vnum, const char *line, int check, CharData * mob,
	      CharData * ch, const void *arg1, const void *arg2,
	      CharData * rch)
{
  CharData *lval_char = mob;
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  ObjData *lval_obj = NULL;
  const char *original;
  char buf[MIL], code;
  int lval = 0, oper = 0, rval = -1;
  flag_t temp;

  original = line;
  line = one_argument (line, buf);
  if (NullStr (buf) || mob == NULL)
    return false;

  if (mob->mprog_target == NULL)
    mob->mprog_target = ch;

  switch (check)
    {

    case CHK_RAND:
      return (number_percent () < atoi (buf));
    case CHK_MOBHERE:
      if (is_number (buf))
	return (get_mob_vnum_room (mob, NULL, NULL, atov (buf)));
      else
	return ((bool) (get_char_room (mob, NULL, buf) != NULL));
    case CHK_OBJHERE:
      if (is_number (buf))
	return (get_obj_vnum_room (mob, NULL, NULL, atov (buf)));
      else
	return ((bool) (get_obj_here (mob, NULL, buf) != NULL));
    case CHK_MOBEXISTS:
      return ((bool) (get_char_world (mob, buf) != NULL));
    case CHK_OBJEXISTS:
      return ((bool) (get_obj_world (mob, buf) != NULL));

    case CHK_PEOPLE:
      rval = count_people_room (mob, NULL, NULL, 0);
      break;
    case CHK_PLAYERS:
      rval = count_people_room (mob, NULL, NULL, 1);
      break;
    case CHK_MOBS:
      rval = count_people_room (mob, NULL, NULL, 2);
      break;
    case CHK_CLONES:
      rval = count_people_room (mob, NULL, NULL, 3);
      break;
    case CHK_ORDER:
      rval = get_order (mob, NULL);
      break;
    case CHK_HOUR:
      rval = time_info.hour;
      break;
    default:
      ;
    }

  if (rval >= 0)
    {
      if ((oper = keyword_lookup (fn_evals, buf)) < 0)
	{
	  bugf ("prog %ld syntax error(2) '%s'", vnum, original);
	  return false;
	}
      one_argument (line, buf);
      lval = rval;
      rval = atoi (buf);
      return (num_eval (lval, oper, rval));
    }

  if (buf[0] != '$' || buf[1] == '\0')
    {
      bugf ("prog %ld syntax error(3) '%s'", vnum, original);
      return false;
    }
  else
    code = buf[1];
  switch (code)
    {
    case 'i':
      lval_char = mob;
      break;
    case 'n':
      lval_char = ch;
      break;
    case 't':
      lval_char = vch;
      break;
    case 'r':
      lval_char = rch == NULL ? get_random_char (mob, NULL, NULL) : rch;
      break;
    case 'o':
      lval_obj = obj1;
      break;
    case 'p':
      lval_obj = obj2;
      break;
    case 'q':
      lval_char = mob->mprog_target;
      break;
    default:
      bugf ("prog %ld syntax error(4) '%s'", vnum, original);
      return false;
    }

  if (lval_char == NULL && lval_obj == NULL)
    return false;

  switch (check)
    {
    case CHK_ISPC:
      return (lval_char != NULL && !IsNPC (lval_char));
    case CHK_ISNPC:
      return (lval_char != NULL && IsNPC (lval_char));
    case CHK_ISGOOD:
      return (lval_char != NULL && IsGood (lval_char));
    case CHK_ISEVIL:
      return (lval_char != NULL && IsEvil (lval_char));
    case CHK_ISNEUTRAL:
      return (lval_char != NULL && IsNeutral (lval_char));
    case CHK_ISIMMORT:
      return (lval_char != NULL && IsImmortal (lval_char));
    case CHK_HUNTER:
      return (lval_char != NULL && lval_char->hunting == mob);
    case CHK_ISCHARM:

      return (lval_char != NULL && IsAffected (lval_char, AFF_CHARM));
    case CHK_ISFOLLOW:
      return (lval_char != NULL && lval_char->master != NULL
	      && lval_char->master->in_room == lval_char->in_room);
    case CHK_ONQUEST:
      return (lval_char != NULL && IsQuester (lval_char));
    case CHK_ISACTIVE:
      return (lval_char != NULL && lval_char->position > POS_SLEEPING);
    case CHK_ISDELAY:
      return (lval_char != NULL && lval_char->mprog_delay > 0);
    case CHK_ISVISIBLE:
      switch (code)
	{
	default:
	case 'i':
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  return (lval_char != NULL && can_see (mob, lval_char));
	case 'o':
	case 'p':
	  return (lval_obj != NULL && can_see_obj (mob, lval_obj));
	}
    case CHK_HASTARGET:
      return (lval_char != NULL && lval_char->mprog_target != NULL
	      && lval_char->in_room == lval_char->mprog_target->in_room);
    case CHK_ISTARGET:
      return (lval_char != NULL && mob->mprog_target == lval_char);
    default:
      ;
    }

  line = one_argument (line, buf);
  switch (check)
    {
    case CHK_AFFECTED:
      if ((temp = flag_value (affect_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsAffected (lval_char, temp));
    case CHK_ACT:
      if ((temp = flag_value (act_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_PLR:
      if ((temp = flag_value (plr_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && !IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_IMM:
      if ((temp = flag_value (imm_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->imm_flags, temp));
    case CHK_OFF:
      if ((temp = flag_value (off_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->off_flags, temp));
    case CHK_CARRIES:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, false));
      else
	return (lval_char != NULL
		&& (get_obj_carry (lval_char, buf, lval_char) != NULL));
    case CHK_WEARS:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, true));
      else
	return (lval_char != NULL
		&& (get_obj_wear (lval_char, buf, true) != NULL));
    case CHK_HAS:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), false));
    case CHK_USES:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), true));
    case CHK_SKILL:
      return (lval_char != NULL && !IsNPC (lval_char)
	      && skill_lookup (buf) != -1
	      && get_skill (lval_char, skill_lookup (buf)) >= atoi (line));
    case CHK_NAME:
      switch (code)
	{
	default:
	case 'i':
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  return (lval_char != NULL && is_name (buf, lval_char->name));
	case 'o':
	case 'p':
	  return (lval_obj != NULL && is_name (buf, lval_obj->name));
	}
    case CHK_POS:
      return (lval_char != NULL
	      && lval_char->position == flag_value (position_flags, buf));
    case CHK_CLAN:
      return (lval_char != NULL && CharClan (lval_char) == clan_lookup (buf));
    case CHK_CLASS:
      return (lval_char != NULL
	      && prime_class (lval_char) == class_lookup (buf));
    case CHK_RACE:
      return (lval_char != NULL && lval_char->race == race_lookup (buf));
    case CHK_OBJTYPE:
      return (lval_obj != NULL
	      && lval_obj->item_type == flag_value (type_flags, buf));
    default:
      ;
    }

  if ((oper = keyword_lookup (fn_evals, buf)) < 0)
    {
      bugf ("prog %ld syntax error(5): '%s'", vnum, original);
      return false;
    }
  one_argument (line, buf);
  rval = atoi (buf);

  switch (check)
    {
    case CHK_VNUM:
      switch (code)
	{
	default:
	case 'i':
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  if (lval_char != NULL && IsNPC (lval_char))
	    lval = lval_char->pIndexData->vnum;
	  break;
	case 'o':
	case 'p':
	  if (lval_obj != NULL)
	    lval = lval_obj->pIndexData->vnum;
	}
      break;
    case CHK_HPCNT:
      if (lval_char != NULL)
	lval = (lval_char->hit * 100) / (Max (1, lval_char->max_hit));
      break;
    case CHK_ROOM:
      if (lval_char != NULL && lval_char->in_room != NULL)
	lval = lval_char->in_room->vnum;
      break;
    case CHK_SEX:
      if (lval_char != NULL)
	lval = lval_char->sex;
      break;
    case CHK_LEVEL:
      if (lval_char != NULL)
	lval = lval_char->level;
      break;
    case CHK_WEIGHT:
      if (lval_char != NULL)
	lval = get_carry_weight (lval_char);
      break;
    case CHK_ALIGN:
      if (lval_char != NULL)
	lval = lval_char->alignment;
      break;
    case CHK_MONEY:
      if (lval_char != NULL)
	lval = lval_char->gold;
      break;
    case CHK_OBJVAL0:
      if (lval_obj != NULL)
	lval = lval_obj->value[0];
      break;
    case CHK_OBJVAL1:
      if (lval_obj != NULL)
	lval = lval_obj->value[1];
      break;
    case CHK_OBJVAL2:
      if (lval_obj != NULL)
	lval = lval_obj->value[2];
      break;
    case CHK_OBJVAL3:
      if (lval_obj != NULL)
	lval = lval_obj->value[3];
      break;
    case CHK_OBJVAL4:
      if (lval_obj != NULL)
	lval = lval_obj->value[4];
      break;
    case CHK_GRPSIZE:
      if (lval_char != NULL)
	lval = count_people_room (lval_char, NULL, NULL, 4);
      break;
    default:
      return false;
    }
  return (num_eval (lval, oper, rval));
}

int
cmd_eval_obj (vnum_t vnum, const char *line, int check, ObjData * obj,
	      CharData * ch, const void *arg1, const void *arg2,
	      CharData * rch)
{
  CharData *lval_char = NULL;
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  ObjData *lval_obj = obj;
  const char *original;
  char buf[MIL], code;
  int lval = 0, oper = 0, rval = -1;
  flag_t temp;

  original = line;
  line = one_argument (line, buf);
  if (NullStr (buf) || obj == NULL)
    return false;

  if (obj->oprog_target == NULL)
    obj->oprog_target = ch;

  switch (check)
    {

    case CHK_RAND:
      return (number_percent () < atoi (buf));
    case CHK_MOBHERE:
      if (is_number (buf))
	return (get_mob_vnum_room (NULL, obj, NULL, atov (buf)));
      else
	return ((bool)
		(get_char_room
		 (NULL,
		  (obj->in_room ? obj->in_room : obj->carried_by->
		   in_room), buf) != NULL));
    case CHK_OBJHERE:
      if (is_number (buf))
	return (get_obj_vnum_room (NULL, obj, NULL, atov (buf)));
      else
	return ((bool)
		(get_obj_here
		 (NULL,
		  (obj->in_room ? obj->in_room : obj->carried_by->
		   in_room), buf) != NULL));
    case CHK_MOBEXISTS:
      return ((bool) (get_char_world (NULL, buf) != NULL));
    case CHK_OBJEXISTS:
      return ((bool) (get_obj_world (NULL, buf) != NULL));

    case CHK_PEOPLE:
      rval = count_people_room (NULL, obj, NULL, 0);
      break;
    case CHK_PLAYERS:
      rval = count_people_room (NULL, obj, NULL, 1);
      break;
    case CHK_MOBS:
      rval = count_people_room (NULL, obj, NULL, 2);
      break;
    case CHK_CLONES:
      bug ("cmd_eval_obj: received CHK_CLONES.");
      break;
    case CHK_ORDER:
      rval = get_order (NULL, obj);
      break;
    case CHK_HOUR:
      rval = time_info.hour;
      break;
    default:
      ;
    }

  if (rval >= 0)
    {
      if ((oper = keyword_lookup (fn_evals, buf)) < 0)
	{
	  bugf ("Cmd_eval_obj: prog %ld syntax error(2) '%s'", vnum,
		original);
	  return false;
	}
      one_argument (line, buf);
      lval = rval;
      rval = atoi (buf);
      return (num_eval (lval, oper, rval));
    }

  if (buf[0] != '$' || buf[1] == '\0')
    {
      bugf ("Cmd_eval_obj: prog %ld syntax error(3) '%s'", vnum, original);
      return false;
    }
  else
    code = buf[1];
  switch (code)
    {
    case 'i':
      lval_obj = obj;
      break;
    case 'n':
      lval_char = ch;
      break;
    case 't':
      lval_char = vch;
      break;
    case 'r':
      lval_char = rch == NULL ? get_random_char (NULL, obj, NULL) : rch;
      break;
    case 'o':
      lval_obj = obj1;
      break;
    case 'p':
      lval_obj = obj2;
      break;
    case 'q':
      lval_char = obj->oprog_target;
      break;
    default:
      bugf ("Cmd_eval_obj: prog %ld syntax error(4) '%s'", vnum, original);
      return false;
    }

  if (lval_char == NULL && lval_obj == NULL)
    return false;

  switch (check)
    {
    case CHK_ISPC:
      return (lval_char != NULL && !IsNPC (lval_char));
    case CHK_ISNPC:
      return (lval_char != NULL && IsNPC (lval_char));
    case CHK_ISGOOD:
      return (lval_char != NULL && IsGood (lval_char));
    case CHK_ISEVIL:
      return (lval_char != NULL && IsEvil (lval_char));
    case CHK_ISNEUTRAL:
      return (lval_char != NULL && IsNeutral (lval_char));
    case CHK_ISIMMORT:
      return (lval_char != NULL && IsImmortal (lval_char));
    case CHK_ISCHARM:

      return (lval_char != NULL && IsAffected (lval_char, AFF_CHARM));
    case CHK_ISFOLLOW:
      return (lval_char != NULL && lval_char->master != NULL
	      && lval_char->master->in_room == lval_char->in_room);
    case CHK_ISACTIVE:
      return (lval_char != NULL && lval_char->position > POS_SLEEPING);
    case CHK_ISDELAY:
      return (lval_char != NULL && lval_char->mprog_delay > 0);
    case CHK_HASTARGET:
      return (lval_char != NULL && lval_char->mprog_target != NULL
	      && lval_char->in_room == lval_char->mprog_target->in_room);
    case CHK_ISTARGET:
      return (lval_char != NULL && obj->oprog_target == lval_char);
    default:
      ;
    }

  line = one_argument (line, buf);
  switch (check)
    {
    case CHK_AFFECTED:
      if ((temp = flag_value (affect_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsAffected (lval_char, temp));
    case CHK_ACT:
      if ((temp = flag_value (act_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_PLR:
      if ((temp = flag_value (plr_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && !IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_IMM:
      if ((temp = flag_value (imm_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->imm_flags, temp));
    case CHK_OFF:
      if ((temp = flag_value (off_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->off_flags, temp));
    case CHK_CARRIES:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, false));
      else
	return (lval_char != NULL
		&& (get_obj_carry (lval_char, buf, lval_char) != NULL));
    case CHK_WEARS:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, true));
      else
	return (lval_char != NULL
		&& (get_obj_wear (lval_char, buf, false) != NULL));
    case CHK_HAS:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), false));
    case CHK_USES:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), true));
    case CHK_SKILL:
      return (lval_char != NULL && !IsNPC (lval_char)
	      && skill_lookup (buf) != -1
	      && get_skill (lval_char, skill_lookup (buf)) >= atoi (line));
    case CHK_NAME:
      switch (code)
	{
	default:
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  return (lval_char != NULL && is_name (buf, lval_char->name));
	case 'i':
	case 'o':
	case 'p':
	  return (lval_obj != NULL && is_name (buf, lval_obj->name));
	}
    case CHK_POS:
      return (lval_char != NULL
	      && lval_char->position == flag_value (position_flags, buf));
    case CHK_CLAN:
      return (lval_char != NULL && CharClan (lval_char) == clan_lookup (buf));
    case CHK_CLASS:
      return (lval_char != NULL
	      && prime_class (lval_char) == class_lookup (buf));
    case CHK_RACE:
      return (lval_char != NULL && lval_char->race == race_lookup (buf));
    case CHK_OBJTYPE:
      return (lval_obj != NULL
	      && lval_obj->item_type == flag_value (type_flags, buf));
    default:
      ;
    }

  if ((oper = keyword_lookup (fn_evals, buf)) < 0)
    {
      bugf ("Cmd_eval_obj: prog %ld syntax error(5): '%s'", vnum, original);
      return false;
    }
  one_argument (line, buf);
  rval = atoi (buf);

  switch (check)
    {
    case CHK_VNUM:
      switch (code)
	{
	default:
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  if (lval_char != NULL && IsNPC (lval_char))
	    lval = lval_char->pIndexData->vnum;
	  break;
	case 'i':
	case 'o':
	case 'p':
	  if (lval_obj != NULL)
	    lval = lval_obj->pIndexData->vnum;
	}
      break;
    case CHK_HPCNT:
      if (lval_char != NULL)
	lval = (lval_char->hit * 100) / (Max (1, lval_char->max_hit));
      break;
    case CHK_ROOM:
      if (lval_char != NULL && lval_char->in_room != NULL)
	lval = lval_char->in_room->vnum;
      else if (lval_obj != NULL
	       && (lval_obj->in_room != NULL || lval_obj->carried_by != NULL))
	lval =
	  lval_obj->in_room ? lval_obj->in_room->vnum : lval_obj->
	  carried_by->in_room->vnum;
      break;
    case CHK_SEX:
      if (lval_char != NULL)
	lval = lval_char->sex;
      break;
    case CHK_LEVEL:
      if (lval_char != NULL)
	lval = lval_char->level;
      break;
    case CHK_WEIGHT:
      if (lval_char != NULL)
	lval = get_carry_weight (lval_char);
      break;
    case CHK_ALIGN:
      if (lval_char != NULL)
	lval = lval_char->alignment;
      break;
    case CHK_MONEY:
      if (lval_char != NULL)
	lval = lval_char->gold;
      break;
    case CHK_OBJVAL0:
      if (lval_obj != NULL)
	lval = lval_obj->value[0];
      break;
    case CHK_OBJVAL1:
      if (lval_obj != NULL)
	lval = lval_obj->value[1];
      break;
    case CHK_OBJVAL2:
      if (lval_obj != NULL)
	lval = lval_obj->value[2];
      break;
    case CHK_OBJVAL3:
      if (lval_obj != NULL)
	lval = lval_obj->value[3];
      break;
    case CHK_OBJVAL4:
      if (lval_obj != NULL)
	lval = lval_obj->value[4];
      break;
    case CHK_GRPSIZE:
      if (lval_char != NULL)
	lval = count_people_room (lval_char, NULL, NULL, 4);
      break;
    default:
      return false;
    }
  return (num_eval (lval, oper, rval));
}

int
cmd_eval_room (vnum_t vnum, const char *line, int check,
	       RoomIndex * room, CharData * ch, const void *arg1,
	       const void *arg2, CharData * rch)
{
  CharData *lval_char = NULL;
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  ObjData *lval_obj = NULL;
  const char *original;
  char buf[MIL], code;
  int lval = 0, oper = 0, rval = -1;
  flag_t temp;

  original = line;
  line = one_argument (line, buf);
  if (NullStr (buf) || room == NULL)
    return false;

  if (room->rprog_target == NULL)
    room->rprog_target = ch;

  switch (check)
    {

    case CHK_RAND:
      return (number_percent () < atoi (buf));
    case CHK_MOBHERE:
      if (is_number (buf))
	return (get_mob_vnum_room (NULL, NULL, room, atov (buf)));
      else
	return ((bool) (get_char_room (NULL, room, buf) != NULL));
    case CHK_OBJHERE:
      if (is_number (buf))
	return (get_obj_vnum_room (NULL, NULL, room, atov (buf)));
      else
	return ((bool) (get_obj_here (NULL, room, buf) != NULL));
    case CHK_MOBEXISTS:
      return ((bool) (get_char_world (NULL, buf) != NULL));
    case CHK_OBJEXISTS:
      return ((bool) (get_obj_world (NULL, buf) != NULL));

    case CHK_PEOPLE:
      rval = count_people_room (NULL, NULL, room, 0);
      break;
    case CHK_PLAYERS:
      rval = count_people_room (NULL, NULL, room, 1);
      break;
    case CHK_MOBS:
      rval = count_people_room (NULL, NULL, room, 2);
      break;
    case CHK_CLONES:
      bug ("received CHK_CLONES.");
      break;
    case CHK_ORDER:
      bug ("received CHK_ORDER.");
      break;
    case CHK_HOUR:
      rval = time_info.hour;
      break;
    default:
      ;
    }

  if (rval >= 0)
    {
      if ((oper = keyword_lookup (fn_evals, buf)) < 0)
	{
	  bugf ("prog %ld syntax error(2) '%s'", vnum, original);
	  return false;
	}
      one_argument (line, buf);
      lval = rval;
      rval = atoi (buf);
      return (num_eval (lval, oper, rval));
    }

  if (buf[0] != '$' || buf[1] == '\0')
    {
      bugf ("prog %ld syntax error(3) '%s'", vnum, original);
      return false;
    }
  else
    code = buf[1];
  switch (code)
    {
    case 'i':
      bug ("received code case 'i'.");
      break;
    case 'n':
      lval_char = ch;
      break;
    case 't':
      lval_char = vch;
      break;
    case 'r':
      lval_char = rch == NULL ? get_random_char (NULL, NULL, room) : rch;
      break;
    case 'o':
      lval_obj = obj1;
      break;
    case 'p':
      lval_obj = obj2;
      break;
    case 'q':
      lval_char = room->rprog_target;
      break;
    default:
      bugf ("prog %ld syntax error(4) '%s'", vnum, original);
      return false;
    }

  if (lval_char == NULL && lval_obj == NULL)
    return false;

  switch (check)
    {
    case CHK_ISPC:
      return (lval_char != NULL && !IsNPC (lval_char));
    case CHK_ISNPC:
      return (lval_char != NULL && IsNPC (lval_char));
    case CHK_ISGOOD:
      return (lval_char != NULL && IsGood (lval_char));
    case CHK_ISEVIL:
      return (lval_char != NULL && IsEvil (lval_char));
    case CHK_ISNEUTRAL:
      return (lval_char != NULL && IsNeutral (lval_char));
    case CHK_ISIMMORT:
      return (lval_char != NULL && IsImmortal (lval_char));
    case CHK_ISCHARM:

      return (lval_char != NULL && IsAffected (lval_char, AFF_CHARM));
    case CHK_ISFOLLOW:
      return (lval_char != NULL && lval_char->master != NULL
	      && lval_char->master->in_room == lval_char->in_room);
    case CHK_ISACTIVE:
      return (lval_char != NULL && lval_char->position > POS_SLEEPING);
    case CHK_ISDELAY:
      return (lval_char != NULL && lval_char->mprog_delay > 0);
    case CHK_HASTARGET:
      return (lval_char != NULL && lval_char->mprog_target != NULL
	      && lval_char->in_room == lval_char->mprog_target->in_room);
    case CHK_ISTARGET:
      return (lval_char != NULL && room->rprog_target == lval_char);
    default:
      ;
    }

  line = one_argument (line, buf);
  switch (check)
    {
    case CHK_AFFECTED:
      if ((temp = flag_value (affect_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsAffected (lval_char, temp));
    case CHK_ACT:
      if ((temp = flag_value (act_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_PLR:
      if ((temp = flag_value (plr_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL
	      && !IsNPC (lval_char) && IsSet (lval_char->act, temp));
    case CHK_IMM:
      if ((temp = flag_value (imm_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->imm_flags, temp));
    case CHK_OFF:
      if ((temp = flag_value (off_flags, buf)) == NO_FLAG)
	return false;
      return (lval_char != NULL && IsSet (lval_char->off_flags, temp));
    case CHK_CARRIES:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, false));
      else
	return (lval_char != NULL
		&& (get_obj_carry (lval_char, buf, lval_char) != NULL));
    case CHK_WEARS:
      if (is_number (buf))
	return (lval_char != NULL
		&& has_item (lval_char, atov (buf), (item_t) - 1, true));
      else
	return (lval_char != NULL
		&& (get_obj_wear (lval_char, buf, false) != NULL));
    case CHK_HAS:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), false));
    case CHK_USES:
      return (lval_char != NULL
	      && has_item (lval_char, -1,
			   (item_t) flag_value (type_flags, buf), true));
    case CHK_SKILL:
      return (lval_char != NULL && !IsNPC (lval_char)
	      && skill_lookup (buf) != -1
	      && get_skill (lval_char, skill_lookup (buf)) >= atoi (line));
    case CHK_NAME:
      switch (code)
	{
	default:
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  return (lval_char != NULL && is_name (buf, lval_char->name));
	case 'i':
	  return false;
	case 'o':
	case 'p':
	  return (lval_obj != NULL && is_name (buf, lval_obj->name));
	}
    case CHK_POS:
      return (lval_char != NULL
	      && lval_char->position == flag_value (position_flags, buf));
    case CHK_CLAN:
      return (lval_char != NULL && CharClan (lval_char) == clan_lookup (buf));
    case CHK_CLASS:
      return (lval_char != NULL
	      && prime_class (lval_char) == class_lookup (buf));
    case CHK_RACE:
      return (lval_char != NULL && lval_char->race == race_lookup (buf));
    case CHK_OBJTYPE:
      return (lval_obj != NULL
	      && lval_obj->item_type == (item_t) flag_value (type_flags,
							     buf));
    default:
      ;
    }

  if ((oper = keyword_lookup (fn_evals, buf)) < 0)
    {
      bugf ("prog %ld syntax error(5): '%s'", vnum, original);
      return false;
    }
  one_argument (line, buf);
  rval = atoi (buf);

  switch (check)
    {
    case CHK_VNUM:
      switch (code)
	{
	default:
	case 'n':
	case 't':
	case 'r':
	case 'q':
	  if (lval_char != NULL && IsNPC (lval_char))
	    lval = lval_char->pIndexData->vnum;
	  break;
	case 'i':
	  return false;
	case 'o':
	case 'p':
	  if (lval_obj != NULL)
	    lval = lval_obj->pIndexData->vnum;
	}
      break;
    case CHK_HPCNT:
      if (lval_char != NULL)
	lval = (lval_char->hit * 100) / (Max (1, lval_char->max_hit));
      break;
    case CHK_ROOM:
      if (lval_char != NULL && lval_char->in_room != NULL)
	lval = lval_char->in_room->vnum;
      else if (lval_obj != NULL
	       && (lval_obj->in_room != NULL || lval_obj->carried_by != NULL))
	lval =
	  lval_obj->in_room ? lval_obj->in_room->vnum : lval_obj->
	  carried_by->in_room->vnum;
      break;
    case CHK_SEX:
      if (lval_char != NULL)
	lval = lval_char->sex;
      break;
    case CHK_LEVEL:
      if (lval_char != NULL)
	lval = lval_char->level;
      break;
    case CHK_WEIGHT:
      if (lval_char != NULL)
	lval = get_carry_weight (lval_char);
      break;
    case CHK_ALIGN:
      if (lval_char != NULL)
	lval = lval_char->alignment;
      break;
    case CHK_MONEY:
      if (lval_char != NULL)
	lval = lval_char->gold;
      break;
    case CHK_OBJVAL0:
      if (lval_obj != NULL)
	lval = lval_obj->value[0];
      break;
    case CHK_OBJVAL1:
      if (lval_obj != NULL)
	lval = lval_obj->value[1];
      break;
    case CHK_OBJVAL2:
      if (lval_obj != NULL)
	lval = lval_obj->value[2];
      break;
    case CHK_OBJVAL3:
      if (lval_obj != NULL)
	lval = lval_obj->value[3];
      break;
    case CHK_OBJVAL4:
      if (lval_obj != NULL)
	lval = lval_obj->value[4];
      break;
    case CHK_GRPSIZE:
      if (lval_char != NULL)
	lval = count_people_room (lval_char, NULL, NULL, 4);
      break;
    default:
      return false;
    }
  return (num_eval (lval, oper, rval));
}

void
expand_arg_mob (char *buf, const char *format, CharData * mob,
		CharData * ch, const void *arg1, const void *arg2,
		CharData * rch)
{
  const char *someone = "someone";
  const char *something = "something";
  const char *someones = "someone's";

  char name[MIL];
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  const char *str;
  const char *i;
  char *point;

  if (NullStr (format))
    return;

  point = buf;
  str = format;
  while (*str != '\0')
    {
      if (*str != '$')
	{
	  *point++ = *str++;
	  continue;
	}
      ++str;

      switch (*str)
	{
	default:
	  bugf ("bad code %c.", *str);
	  i = " <@@@> ";
	  break;
	case 'i':
	  one_argument (mob->name, name);
	  i = name;
	  break;
	case 'I':
	  i = mob->short_descr;
	  break;
	case 'n':
	  i = someone;
	  if (ch != NULL && can_see (mob, ch))
	    {
	      one_argument (ch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'N':
	  i = (ch != NULL
	       && can_see (mob,
			   ch)) ? (IsNPC (ch) ? ch->
				   short_descr : ch->name) : someone;
	  break;
	case 't':
	  i = someone;
	  if (vch != NULL && can_see (mob, vch))
	    {
	      one_argument (vch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'T':
	  i = (vch != NULL
	       && can_see (mob,
			   vch)) ? (IsNPC (vch) ? vch->
				    short_descr : vch->name) : someone;
	  break;
	case 'r':
	  if (rch == NULL)
	    rch = get_random_char (mob, NULL, NULL);
	  i = someone;
	  if (rch != NULL && can_see (mob, rch))
	    {
	      one_argument (rch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'R':
	  if (rch == NULL)
	    rch = get_random_char (mob, NULL, NULL);
	  i = (rch != NULL
	       && can_see (mob,
			   rch)) ? (IsNPC (ch) ? ch->
				    short_descr : ch->name) : someone;
	  break;
	case 'q':
	  i = someone;
	  if (mob->mprog_target != NULL && can_see (mob, mob->mprog_target))
	    {
	      one_argument (mob->mprog_target->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'Q':
	  i = (mob->mprog_target != NULL
	       && can_see (mob,
			   mob->mprog_target)) ? (IsNPC (mob->mprog_target) ?
						  mob->
						  mprog_target->short_descr :
						  mob->mprog_target->
						  name) : someone;
	  break;
	case 'j':
	  i = he_she[Range (0, mob->sex, 2)];
	  break;
	case 'e':
	  i = (ch != NULL
	       && can_see (mob, ch)) ? he_she[Range (0, ch->sex,
						     2)] : someone;
	  break;
	case 'E':
	  i = (vch != NULL
	       && can_see (mob, vch)) ? he_she[Range (0, vch->sex,
						      2)] : someone;
	  break;
	case 'J':
	  i = (rch != NULL
	       && can_see (mob, rch)) ? he_she[Range (0, rch->sex,
						      2)] : someone;
	  break;
	case 'X':
	  i = (mob->mprog_target != NULL
	       && can_see (mob, mob->mprog_target)) ? he_she[Range (0,
								    mob->
								    mprog_target->
								    sex,
								    2)] :
	    someone;
	  break;
	case 'k':
	  i = him_her[Range (0, mob->sex, 2)];
	  break;
	case 'm':
	  i = (ch != NULL
	       && can_see (mob, ch)) ? him_her[Range (0, ch->sex,
						      2)] : someone;
	  break;
	case 'M':
	  i = (vch != NULL
	       && can_see (mob, vch)) ? him_her[Range (0, vch->sex,
						       2)] : someone;
	  break;
	case 'K':
	  if (rch == NULL)
	    rch = get_random_char (mob, NULL, NULL);
	  i = (rch != NULL
	       && can_see (mob, rch)) ? him_her[Range (0, rch->sex,
						       2)] : someone;
	  break;
	case 'Y':
	  i = (mob->mprog_target != NULL
	       && can_see (mob, mob->mprog_target)) ? him_her[Range (0,
								     mob->
								     mprog_target->
								     sex,
								     2)] :
	    someone;
	  break;
	case 'l':
	  i = his_her[Range (0, mob->sex, 2)];
	  break;
	case 's':
	  i = (ch != NULL
	       && can_see (mob, ch)) ? his_her[Range (0, ch->sex,
						      2)] : someones;
	  break;
	case 'S':
	  i = (vch != NULL
	       && can_see (mob, vch)) ? his_her[Range (0, vch->sex,
						       2)] : someones;
	  break;
	case 'L':
	  if (rch == NULL)
	    rch = get_random_char (mob, NULL, NULL);
	  i = (rch != NULL
	       && can_see (mob, rch)) ? his_her[Range (0, rch->sex,
						       2)] : someones;
	  break;
	case 'Z':
	  i = (mob->mprog_target != NULL
	       && can_see (mob, mob->mprog_target)) ? his_her[Range (0,
								     mob->
								     mprog_target->
								     sex,
								     2)] :
	    someones;
	  break;
	case 'o':
	  i = something;
	  if (obj1 != NULL && can_see_obj (mob, obj1))
	    {
	      one_argument (obj1->name, name);
	      i = name;
	    }
	  break;
	case 'O':
	  i = (obj1 != NULL
	       && can_see_obj (mob, obj1)) ? obj1->short_descr : something;
	  break;
	case 'p':
	  i = something;
	  if (obj2 != NULL && can_see_obj (mob, obj2))
	    {
	      one_argument (obj2->name, name);
	      i = name;
	    }
	  break;
	case 'P':
	  i = (obj2 != NULL
	       && can_see_obj (mob, obj2)) ? obj2->short_descr : something;
	  break;
	}

      ++str;
      while ((*point = *i) != '\0')
	++point, ++i;

    }
  *point = '\0';

  return;
}

void
expand_arg_other (char *buf, const char *format, ObjData * obj,
		  RoomIndex * room, CharData * ch,
		  const void *arg1, const void *arg2, CharData * rch)
{
  const char *someone = "someone";
  const char *something = "something";
  const char *someones = "someone's";

  char name[MIL];
  CharData *vch = (CharData *) arg2;
  ObjData *obj1 = (ObjData *) arg1;
  ObjData *obj2 = (ObjData *) arg2;
  const char *str;
  const char *i;
  char *point;

  if (obj && room)
    {
      bug ("received a obj and a room");
      return;
    }

  if (NullStr (format))
    return;

  point = buf;
  str = format;
  while (*str != '\0')
    {
      if (*str != '$')
	{
	  *point++ = *str++;
	  continue;
	}
      ++str;

      switch (*str)
	{
	default:
	  bugf ("bad code %c.", *str);
	  i = " <@@@> ";
	  break;
	case 'i':
	  if (obj)
	    {
	      one_argument (obj->name, name);
	      i = name;
	    }
	  else
	    {
	      bug ("room had an \"i\" case.");
	      i = " <@@@> ";
	    }
	  break;
	case 'I':
	  if (obj)
	    i = obj->short_descr;
	  else
	    {
	      bug ("room had an \"I\" case.");
	      i = " <@@@> ";
	    }
	  break;
	case 'n':
	  i = someone;
	  if (ch != NULL)
	    {
	      one_argument (ch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'N':
	  i = (ch !=
	       NULL) ? (IsNPC (ch) ? ch->short_descr : ch->name) : someone;
	  break;
	case 't':
	  i = someone;
	  if (vch != NULL)
	    {
	      one_argument (vch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'T':
	  i = (vch !=
	       NULL) ? (IsNPC (vch) ? vch->short_descr : vch->name) : someone;
	  break;
	case 'r':
	  if (rch == NULL && obj)
	    rch = get_random_char (NULL, obj, NULL);
	  else if (rch == NULL && room)
	    rch = get_random_char (NULL, NULL, room);
	  i = someone;
	  if (rch != NULL)
	    {
	      one_argument (rch->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'R':
	  if (rch == NULL && obj)
	    rch = get_random_char (NULL, obj, NULL);
	  else if (rch == NULL && room)
	    rch = get_random_char (NULL, NULL, room);
	  i = (rch !=
	       NULL) ? (IsNPC (ch) ? ch->short_descr : ch->name) : someone;
	  break;
	case 'q':
	  i = someone;
	  if (obj && obj->oprog_target != NULL)
	    {
	      one_argument (obj->oprog_target->name, name);
	      i = capitalize (name);
	    }
	  else if (room && room->rprog_target != NULL)
	    {
	      one_argument (room->rprog_target->name, name);
	      i = capitalize (name);
	    }
	  break;
	case 'Q':
	  i = (obj
	       && obj->oprog_target !=
	       NULL) ? (IsNPC (obj->oprog_target) ? obj->
			oprog_target->short_descr : obj->oprog_target->
			name) : (room
				 && room->rprog_target !=
				 NULL) ? (IsNPC (room->rprog_target) ? room->
					  rprog_target->short_descr : room->
					  rprog_target->name) : someone;
	  break;
	case 'j':
	  bug ("Obj/room received case 'j'");
	  i = " <@@@> ";
	  break;
	case 'e':
	  i = (ch != NULL) ? he_she[Range (0, ch->sex, 2)] : someone;
	  break;
	case 'E':
	  i = (vch != NULL) ? he_she[Range (0, vch->sex, 2)] : someone;
	  break;
	case 'J':
	  i = (rch != NULL) ? he_she[Range (0, rch->sex, 2)] : someone;
	  break;
	case 'X':
	  i = (obj
	       && obj->oprog_target != NULL) ? he_she[Range (0,
							     obj->
							     oprog_target->
							     sex, 2)] : (room
									 &&
									 room->
									 rprog_target
									 !=
									 NULL)
	    ? he_she[Range (0, room->rprog_target->sex, 2)] : someone;
	  break;
	case 'k':
	  bug ("received case 'k'.");
	  i = " <@@@> ";
	  break;
	case 'm':
	  i = (ch != NULL) ? him_her[Range (0, ch->sex, 2)] : someone;
	  break;
	case 'M':
	  i = (vch != NULL) ? him_her[Range (0, vch->sex, 2)] : someone;
	  break;
	case 'K':
	  if (obj && rch == NULL)
	    rch = get_random_char (NULL, obj, NULL);
	  else if (room && rch == NULL)
	    rch = get_random_char (NULL, NULL, room);
	  i = (rch != NULL) ? him_her[Range (0, rch->sex, 2)] : someone;
	  break;
	case 'Y':
	  i = (obj
	       && obj->oprog_target != NULL) ? him_her[Range (0,
							      obj->
							      oprog_target->
							      sex,
							      2)] : (room
								     &&
								     room->
								     rprog_target
								     !=
								     NULL) ?
	    him_her[Range (0, room->rprog_target->sex, 2)] : someone;
	  break;
	case 'l':
	  bug ("received case 'l'.");
	  i = " <@@@> ";
	  break;
	case 's':
	  i = (ch != NULL) ? his_her[Range (0, ch->sex, 2)] : someones;
	  break;
	case 'S':
	  i = (vch != NULL) ? his_her[Range (0, vch->sex, 2)] : someones;
	  break;
	case 'L':
	  if (obj && rch == NULL)
	    rch = get_random_char (NULL, obj, NULL);
	  else if (room && rch == NULL)
	    rch = get_random_char (NULL, NULL, room);
	  i = (rch != NULL) ? his_her[Range (0, rch->sex, 2)] : someones;
	  break;
	case 'Z':
	  i = (obj
	       && obj->oprog_target != NULL) ? his_her[Range (0,
							      obj->
							      oprog_target->
							      sex,
							      2)] : (room
								     &&
								     room->
								     rprog_target
								     !=
								     NULL) ?
	    his_her[Range (0, room->rprog_target->sex, 2)] : someones;
	  break;
	case 'o':
	  i = something;
	  if (obj1 != NULL)
	    {
	      one_argument (obj1->name, name);
	      i = name;
	    }
	  break;
	case 'O':
	  i = (obj1 != NULL) ? obj1->short_descr : something;
	  break;
	case 'p':
	  i = something;
	  if (obj2 != NULL)
	    {
	      one_argument (obj2->name, name);
	      i = name;
	    }
	  break;
	case 'P':
	  i = (obj2 != NULL) ? obj2->short_descr : something;
	  break;
	}

      ++str;
      while ((*point = *i) != '\0')
	++point, ++i;

    }
  *point = '\0';

  return;
}


const char *
prog_type (prog_t type)
{
  switch (type)
    {
    case PRG_MPROG:
      return "mobile";
    case PRG_OPROG:
      return "object";
    case PRG_RPROG:
      return "room";
    default:
      return "unknown";
    }
}

#define    MAX_NESTED_LEVEL 12

#define    BEGIN_BLOCK       0

#define    IN_BLOCK         -1

#define    END_BLOCK        -2

#define    MAX_CALL_LEVEL    5


#define MAX_PTRACE    	500
int ptrace_current;
int ptrace_calledby[MAX_PTRACE];
vnum_t callstack_pvnum[MAX_CALL_LEVEL];
vnum_t callstack_mvnum[MAX_CALL_LEVEL];
vnum_t callstack_rvnum[MAX_CALL_LEVEL];
int callstack_line[MAX_CALL_LEVEL];
bool callstack_aborted[MAX_CALL_LEVEL];
prog_t callstack_type[MAX_CALL_LEVEL];
vnum_t ptrace_pvnum[MAX_PTRACE];
vnum_t ptrace_mvnum[MAX_PTRACE];
vnum_t ptrace_rvnum[MAX_PTRACE];
prog_t ptrace_type[MAX_PTRACE];
unsigned char ptrace_calllevel[MAX_PTRACE];
time_t ptrace_time[MAX_PTRACE];
bool ptrace_aborted[MAX_PTRACE];

int call_level = 0;

Do_Fun (do_preset)
{
  int i;

  chprintln (ch, "Progs reseted.  (prog callstack depth to 0)");
  for (i = 0; i < MAX_CALL_LEVEL; i++)
    {
      callstack_pvnum[i] = 0;
      callstack_mvnum[i] = 0;
      callstack_rvnum[i] = 0;
      callstack_type[i] = PRG_NONE;
      callstack_aborted[i] = false;
    }
  call_level = 0;

  logf ("%s reset the prog callstack", ch->name);
  return;
}


Do_Fun (do_pinfo)
{
  int i;

  Buffer *output;

  output = new_buf ();

  bprintlnf (output,
	     "    Displaying program call abort history - calllevel currently is %d\r\n",
	     call_level);


  for (i = 0; i < MAX_CALL_LEVEL; i++)
    {
      if (callstack_aborted[i])
	{
	  bprintlnf
	    (output,
	     "      [%2d] Program %ld on %s %ld (in room %ld) {RABORTED!!! (BUGGY?){X\r\n",
	     i, callstack_pvnum[i], prog_type (callstack_type[i]),
	     callstack_mvnum[i], callstack_rvnum[i]);
	}
      else
	{
	  bprintlnf
	    (output,
	     "      [%2d] Program %ld on %s %ld (in room %ld)\r\n", i,
	     callstack_pvnum[i], prog_type (callstack_type[i]),
	     callstack_mvnum[i], callstack_rvnum[i]);
	}
    }

  {
    ProgCode *prg;

    for (prg = mprog_first; prg; prg = prg->next)
      {
	if (prg->disabled)
	  {
	    bprintlnf
	      (output,
	       "Mobprog %ld is disabled with the following text:{x",
	       prg->vnum);
	    bprintln (output, prg->disabled_text);
	  }
      }
    for (prg = oprog_first; prg; prg = prg->next)
      {
	if (prg->disabled)
	  {
	    bprintlnf
	      (output,
	       "Objprog %ld is disabled with the following text:{x",
	       prg->vnum);
	    bprintln (output, prg->disabled_text);
	  }
      }
    for (prg = rprog_first; prg; prg = prg->next)
      {
	if (prg->disabled)
	  {
	    bprintlnf
	      (output,
	       "Roomprog %ld is disabled with the following text:{x",
	       prg->vnum);
	    bprintln (output, prg->disabled_text);
	  }
      }
  }

  sendpage (ch, buf_string (output));
  free_buf (output);

  return;
}


Do_Fun (do_ptrace)
{
  char arg1[MIL];
  Buffer *output;
  int lines_to_show;
  int line;
  int count;
  int i;
  int lc;

  argument = one_argument (argument, arg1);
  if (NullStr (arg1))
    {
      lines_to_show = get_scr_cols (ch);
      lines_to_show %= MAX_PTRACE;
    }
  else if (is_number (arg1))
    {
      lines_to_show = atoi (arg1);
      if (lines_to_show < 1)
	{
	  chprintln (ch, "Lines to show, increased to 1");
	  lines_to_show = 1;
	}
      else if (lines_to_show > MAX_PTRACE)
	{
	  chprintlnf
	    (ch,
	     "Lines to show, decreased to the number logged in the trace - %d",
	     MAX_PTRACE);
	  lines_to_show = MAX_PTRACE;
	}
    }
  else
    {
      chprintlnf (ch,
		  "{RThe only parameter for ptrace must be a numeric value"
		  NEWLINE
		  "for the number of lines of the trace you wish to see.{x");
      return;
    }
  chprintlnf (ch, "Number of trace lines to show=%d", lines_to_show);

  output = new_buf ();

  bprintln (output, "{xPTrace {G#prog{x, {Mtype{x, {Yvnum{x, {Binroom{x");


  line = ptrace_current + MAX_PTRACE - lines_to_show;
  lc = 0;

  for (count = 0; count < lines_to_show; count++)
    {
      line++;
      line %= MAX_PTRACE;

      if (ptrace_time[line] == 0)
	continue;


      bprintf (output, "[%3d]%s  ", ++lc,
	       str_time (ptrace_time[line], GetTzone (ch), "%H:%M:%S"));


      for (i = 0; i < ptrace_calllevel[line]; i++)
	{
	  bprint (output, "        ");
	}

      bprintlnf (output, "{G%5ld{x,{M%6s,{Y%5ld{x,{B%5ld{x, %s",
		 ptrace_pvnum[line],
		 prog_type (ptrace_type[line]),
		 ptrace_mvnum[line],
		 ptrace_rvnum[line],
		 prog_type_to_name (ptrace_calledby[line]));
    }
  sendpage (ch, buf_string (output));
  free_buf (output);
}

Do_Fun (do_phelp)
{
  cmd_syntax (ch, NULL, n_fun, "trace", "reset", "info", "stat", "dump",
	      NULL);
}

Do_Fun (do_pstat)
{
  char cmd[MIL], arg[MIL];
  ProgList *prg;
  int i;

  argument = one_argument (argument, cmd);
  one_argument (argument, arg);

  if (NullStr (cmd))
    {
      cmd_syntax (ch, NULL, n_fun, "mob <vnum>", "obj <vnum>",
		  "rpstat <vnum>", NULL);
      return;
    }
  else if (!str_prefix (cmd, "room"))
    {
      RoomIndex *room;

      if (NullStr (arg))
	room = ch->in_room;
      else if (!is_number (arg))
	{
	  chprintln (ch, "You must provide a number.");
	  return;
	}
      else if ((room = get_room_index (atov (arg))) == NULL)
	{
	  chprintln (ch, "No such room.");
	  return;
	}

      chprintlnf (ch, "Room #%-6ld [%s]", room->vnum, room->name);

      chprintlnf (ch, "Delay   %-6d [%s]", room->rprog_delay,
		  room->rprog_target ==
		  NULL ? "No target" : room->rprog_target->name);

      if (!room->rprog_flags)
	{
	  chprintln (ch, "[No programs set]");
	  return;
	}

      for (i = 0, prg = room->rprog_first; prg != NULL; prg = prg->next)
	{
	  chprintlnf (ch,
		      "[%2d] Trigger [%-8s] Program [%4ld] Phrase [%s]",
		      ++i, prog_type_to_name (prg->trig_type),
		      prg->prog->vnum, prg->trig_phrase);
	}

      return;
    }
  else if (!str_prefix (cmd, "object"))
    {
      ObjData *obj;

      if (NullStr (arg) || (obj = get_obj_world (ch, arg)) == NULL)
	{
	  chprintln (ch, "No such object.");
	  return;
	}

      chprintlnf (ch, "Object #%-6ld [%s]", obj->pIndexData->vnum,
		  obj->short_descr);

      chprintlnf (ch, "Delay   %-6d [%s]", obj->oprog_delay,
		  obj->oprog_target ==
		  NULL ? "No target" : obj->oprog_target->name);

      if (!obj->pIndexData->oprog_flags)
	{
	  chprintln (ch, "[No programs set]");
	  return;
	}

      for (i = 0, prg = obj->pIndexData->oprog_first; prg != NULL;
	   prg = prg->next)
	{
	  chprintlnf (ch,
		      "[%2d] Trigger [%-8s] Program [%4ld] Phrase [%s]",
		      ++i, prog_type_to_name (prg->trig_type),
		      prg->prog->vnum, prg->trig_phrase);
	}

      return;
    }
  else if (!str_prefix (cmd, "mobile"))
    {
      CharData *victim;

      if (NullStr (arg) || (victim = get_char_world (ch, arg)) == NULL)
	{
	  chprintln (ch, "No such creature.");
	  return;
	}

      if (!IsNPC (victim))
	{
	  chprintln (ch, "That is not a mobile.");
	  return;
	}

      chprintlnf (ch, "Mobile #%-6ld [%s]", victim->pIndexData->vnum,
		  victim->short_descr);

      chprintlnf (ch, "Delay   %-6d [%s]", victim->mprog_delay,
		  victim->mprog_target ==
		  NULL ? "No target" : victim->mprog_target->name);

      if (!victim->pIndexData->mprog_flags)
	{
	  chprintln (ch, "[No programs set]");
	  return;
	}

      for (i = 0, prg = victim->pIndexData->mprog_first; prg != NULL;
	   prg = prg->next)
	{
	  chprintlnf (ch,
		      "[%2d] Trigger [%-8s] Program [%4ld] Phrase [%s]",
		      ++i, prog_type_to_name (prg->trig_type),
		      prg->prog->vnum, prg->trig_phrase);
	}

      return;
    }
  else
    do_pstat (n_fun, ch, "");
}

Do_Fun (do_pdump)
{
  char cmd[MIL], buf[MIL];
  ProgCode *prg;

  argument = one_argument (argument, cmd);
  one_argument (argument, buf);

  if (NullStr (cmd))
    {
      cmd_syntax (ch, NULL, n_fun, "mob <vnum>", "obj <vnum>",
		  "room <vnum>", NULL);
      return;
    }
  else if (!str_prefix (cmd, "mobile"))
    {
      if ((prg = get_prog_index (atov (buf), PRG_MPROG)) == NULL)
	{
	  chprintln (ch, "No such MOBprogram.");
	  return;
	}
      sendpage (ch, prg->code);
    }
  else if (!str_prefix (cmd, "object"))
    {
      if ((prg = get_prog_index (atov (buf), PRG_OPROG)) == NULL)
	{
	  chprintln (ch, "No such OBJprogram.");
	  return;
	}
      sendpage (ch, prg->code);
    }
  else if (!str_prefix (cmd, "room"))
    {
      if ((prg = get_prog_index (atov (buf), PRG_RPROG)) == NULL)
	{
	  chprintln (ch, "No such ROOMprogram.");
	  return;
	}
      sendpage (ch, prg->code);
    }
  else
    do_pdump (n_fun, ch, "");
}

Do_Fun (do_programs)
{
  vinterpret (ch, n_fun, argument, "trace", do_ptrace, "reset", do_preset,
	      "info", do_pinfo, "stat", do_pstat, "dump", do_pdump, NULL,
	      do_phelp);
}

void
buggy_prog (ProgList * trigger, const char *fmt, ...)
{
  char buf[MSL];
  char *pBuf;
  int i;

  assert (trigger->prog->disabled == false);

  trigger->prog->disabled = true;

  sprintf (buf, "#############" NEWLINE
	   "             Disabling prog %ld,%ld,%ld,%d,%d:" NEWLINE
	   "             ",
	   callstack_pvnum[call_level - 1],
	   callstack_mvnum[call_level - 1],
	   callstack_rvnum[call_level - 1],
	   callstack_line[call_level - 1], call_level - 1);

  if (!NullStr (fmt))
    {
      va_list args;

      pBuf = &buf[strlen (buf)];
      va_start (args, fmt);
      vsnprintf (pBuf, sizeof (buf), fmt, args);
      va_end (args);
      bug (pBuf);
    }

  pBuf = &buf[strlen (buf)];
  sprintf (pBuf,
	   NEWLINE "             Trigger type '%s' phrase '%s'" NEWLINE,
	   prog_type_to_name (trigger->trig_type), trigger->trig_phrase);

  strcat (pBuf, "             Call stack info:" NEWLINE);


  for (i = 0; i < call_level; i++)
    {
      if (!callstack_aborted[i] || (i == call_level - 1))
	{
	  sprintf (pBuf + strlen (pBuf),
		   "             Program %ld on %s %ld (in room %ld), line %d"
		   NEWLINE, callstack_pvnum[i],
		   prog_type (callstack_type[i]), callstack_mvnum[i],
		   callstack_rvnum[i], callstack_line[i]);
	}
    }
  strcat (pBuf, "#############" NEWLINE NEWLINE);

  replace_str (&trigger->prog->disabled_text, buf);

}

void
program_flow (ProgList * program, CharData * mob,
	      ObjData * obj, RoomIndex * room, CharData * ch,
	      const void *arg1, const void *arg2)
{
  CharData *rch = NULL;
  const char *source;
  const char *code;
  const char *line;
  char ctrl[MAX_INPUT_LENGTH], data[MAX_STRING_LENGTH];
  static bool init_ptrace = true;
  char buf[MSL];
  prog_t type = PRG_NONE;
  int level, eval, check;
  int state[MAX_NESTED_LEVEL], cond[MAX_NESTED_LEVEL];
  vnum_t mvnum = 0, ovnum = 0, rvnum = 0, pvnum;

  if (init_ptrace)
    {
      memset (&ptrace_time[0], 0, sizeof (time_t));
      init_ptrace = false;
    }

  if (!program)
    {
      bug ("program_flow(): program==NULL!!!");
      return;
    }

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("received multiple prog types.");
      return;
    }
  if (mob)
    {
      type = PRG_MPROG;
      mvnum = mob->pIndexData->vnum;
      rvnum = mob->in_room ? mob->in_room->vnum : 0;
    }
  else if (obj)
    {
      type = PRG_OPROG;
      ovnum = obj->pIndexData->vnum;
      mvnum = 0;
      rvnum = obj->in_room ? obj->in_room->vnum : 0;
    }
  else if (room)
    {
      type = PRG_RPROG;
      mvnum = 0;
      rvnum = room->vnum;
    }
  else
    {
      bug ("did not receive a prog type.");
      return;
    }

  pvnum = program->prog->vnum;

  callstack_pvnum[call_level] = pvnum;
  callstack_mvnum[call_level] = mvnum;
  callstack_rvnum[call_level] = rvnum;
  callstack_aborted[call_level] = true;

  callstack_line[call_level] = 0;
  callstack_type[call_level] = type;

  ptrace_current++;
  ptrace_current %= MAX_PTRACE;
  ptrace_calledby[ptrace_current] = program->trig_type;
  ptrace_pvnum[ptrace_current] = pvnum;
  ptrace_mvnum[ptrace_current] = mvnum;
  ptrace_rvnum[ptrace_current] = rvnum;
  ptrace_time[ptrace_current] = current_time;
  ptrace_calllevel[ptrace_current] = call_level;
  ptrace_type[ptrace_current] = type;

  if (++call_level > MAX_CALL_LEVEL)
    {
      int i;

      if (mob)
	bugf
	  ("Progs: MAX_CALL_LEVEL exceeded, vnum %ld, mprog vnum %ld",
	   mvnum, pvnum);
      else if (obj)
	bugf
	  ("Progs: MAX_CALL_LEVEL exceeded, vnum %ld oprog vnum %ld.",
	   ovnum, pvnum);
      else
	bugf
	  ("Progs: MAX_CALL_LEVEL exceeded, vnum %ld rprog vnum %ld.",
	   rvnum, pvnum);

      log_string ("Program callstack:\n");
      for (i = 0; i < MAX_CALL_LEVEL; i++)
	{
	  logf ("[%2d] Program %ld on %s %ld (in room %ld)", i,
		callstack_pvnum[i], prog_type (callstack_type[i]),
		callstack_mvnum[i], callstack_rvnum[i]);
	}
      return;
    }

  if (program->prog->disabled)
    {
      if (ch)
	{
	  bugf
	    ("Prog %ld triggered (%s) on me by '%s', not run cause prog is disabled.",
	     program->prog->vnum,
	     prog_type_to_name (program->trig_type), ch->name);
	}
      else
	{
	  bugf
	    ("Prog %ld triggered (%s), not run cause prog is disabled.",
	     program->prog->vnum, prog_type_to_name (program->trig_type));
	}
      call_level--;
      callstack_aborted[call_level] = false;
      return;
    }

  for (level = 0; level < MAX_NESTED_LEVEL; level++)
    {
      state[level] = IN_BLOCK;
      cond[level] = true;
    }
  level = 0;

  source = program->prog->code;
  code = source;

  while (*code)
    {
      bool arg_first = true;
      char *b = buf, *c = ctrl, *d = data;

      while (isspace (*code) && *code)
	code++;
      while (*code)
	{
	  if (*code == '\n' || *code == '\r')
	    break;
	  else if (isspace (*code))
	    {
	      if (arg_first)
		arg_first = false;
	      else
		*d++ = *code;
	    }
	  else
	    {
	      if (arg_first)
		*c++ = *code;
	      else
		*d++ = *code;
	    }
	  *b++ = *code++;
	}
      *b = *c = *d = '\0';

      if (NullStr (buf))
	break;
      if (buf[0] == '*')

	continue;

      line = data;

      if (!str_cmp (ctrl, "if"))
	{
	  if (state[level] == BEGIN_BLOCK)
	    {
	      if (mob)
		buggy_prog (program,
			    "misplaced if statement, mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program,
			    "misplaced if statement, obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "misplaced if statement, room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  state[level] = BEGIN_BLOCK;
	  if (++level >= MAX_NESTED_LEVEL)
	    {
	      if (mob)
		buggy_prog (program,
			    "Max nested level exceeded, mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program,
			    "Max nested level exceeded, obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "Max nested level exceeded, room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  if (level && cond[level - 1] == false)
	    {
	      cond[level] = false;
	      continue;
	    }
	  line = one_argument (line, ctrl);
	  if (mob && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      cond[level] =
		cmd_eval_mob (pvnum, line, check, mob, ch, arg1, arg2, rch);
	    }
	  else if (obj && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      cond[level] =
		cmd_eval_obj (pvnum, line, check, obj, ch, arg1, arg2, rch);
	    }
	  else if (room && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      cond[level] =
		cmd_eval_room (pvnum, line, check, room, ch, arg1, arg2, rch);
	    }
	  else
	    {
	      if (mob)
		buggy_prog (program,
			    "invalid if_check (if), mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program,
			    "invalid if_check (if), obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "invalid if_check (if), room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  state[level] = END_BLOCK;
	}
      else if (!str_cmp (ctrl, "or"))
	{
	  if (!level || state[level - 1] != BEGIN_BLOCK)
	    {
	      if (mob)
		buggy_prog (program, "or without if, mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program, "or without if, obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program, "or without if, room %ld prog %ld",
			    rvnum, pvnum);
	      return;
	    }
	  if (level && cond[level - 1] == false)
	    continue;
	  line = one_argument (line, ctrl);
	  if (mob && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_mob (pvnum, line, check, mob, ch, arg1, arg2, rch);
	    }
	  else if (obj && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_obj (pvnum, line, check, obj, ch, arg1, arg2, rch);
	    }
	  else if (room && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_room (pvnum, line, check, room, ch, arg1, arg2, rch);
	    }
	  else
	    {
	      if (mob)
		buggy_prog (program,
			    "invalid if_check (or), mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program,
			    "invalid if_check (or), obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "invalid if_check (or), room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  cond[level] = (eval == (int) true) ? true : cond[level];
	}
      else if (!str_cmp (ctrl, "and"))
	{
	  if (!level || state[level - 1] != BEGIN_BLOCK)
	    {
	      if (mob)
		buggy_prog (program, "and without if, mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program, "and without if, obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "and without if, room %ld prog %ld", rvnum,
			    pvnum);
	      call_level--;
	      return;
	    }
	  if (level && cond[level - 1] == false)
	    continue;
	  line = one_argument (line, ctrl);
	  if (mob && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_mob (pvnum, line, check, mob, ch, arg1, arg2, rch);
	    }
	  else if (obj && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_obj (pvnum, line, check, obj, ch, arg1, arg2, rch);
	    }
	  else if (room && (check = keyword_lookup (fn_keyword, ctrl)) >= 0)
	    {
	      eval =
		cmd_eval_room (pvnum, line, check, room, ch, arg1, arg2, rch);
	    }
	  else
	    {
	      if (mob)
		buggy_prog (program,
			    "invalid if_check (and), mob %ld prog %ld",
			    mvnum, pvnum);
	      else if (obj)
		buggy_prog (program,
			    "invalid if_check (and), obj %ld prog %ld",
			    ovnum, pvnum);
	      else
		buggy_prog (program,
			    "invalid if_check (and), room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  cond[level] = (cond[level] == (int) true)
	    && (eval == (int) true) ? true : false;
	}
      else if (!str_cmp (ctrl, "endif"))
	{
	  if (!level || state[level - 1] != BEGIN_BLOCK)
	    {
	      if (mob)
		buggy_prog (program,
			    "endif without if, mob %ld prog %ld", mvnum,
			    pvnum);
	      else if (obj)
		buggy_prog (program,
			    "endif without if, obj %ld prog %ld", ovnum,
			    pvnum);
	      else
		buggy_prog (program,
			    "endif without if, room %ld prog %ld",
			    rvnum, pvnum);
	      call_level--;
	      return;
	    }
	  cond[level] = true;
	  state[level] = IN_BLOCK;
	  state[--level] = END_BLOCK;
	}
      else if (!str_cmp (ctrl, "else"))
	{
	  if (!level || state[level - 1] != BEGIN_BLOCK)
	    {
	      if (mob)
		buggy_prog (program,
			    "else without if, mob %ld prog %ld", mvnum,
			    pvnum);
	      else if (obj)
		buggy_prog (program,
			    "else without if, obj %ld prog %ld", ovnum,
			    pvnum);
	      else
		buggy_prog (program,
			    "else without if, room %ld prog %ld", rvnum,
			    pvnum);
	      call_level--;
	      return;
	    }
	  if (level && cond[level - 1] == false)
	    continue;
	  state[level] = IN_BLOCK;
	  cond[level] = (cond[level] == (int) true) ? false : true;
	}
      else if (cond[level] == (int) true
	       && (!str_cmp (ctrl, "break") || !str_cmp (ctrl, "end")))
	{
	  call_level--;
	  callstack_aborted[call_level] = false;
	  return;
	}
      else if ((!level || cond[level] == (int) true) && !NullStr (buf))
	{
	  state[level] = IN_BLOCK;

	  if (mob)
	    expand_arg_mob (data, buf, mob, ch, arg1, arg2, rch);
	  else if (obj)
	    expand_arg_other (data, buf, obj, NULL, ch, arg1, arg2, rch);
	  else
	    expand_arg_other (data, buf, NULL, room, ch, arg1, arg2, rch);

	  if (!str_cmp (ctrl, "mob"))
	    {

	      line = one_argument (data, ctrl);
	      if (!mob)
		bug ("mob command in non MOBprog");
	      else
		mob_interpret (mob, line);
	    }
	  else if (!str_cmp (ctrl, "obj"))
	    {

	      line = one_argument (data, ctrl);
	      if (!obj)
		bug ("obj command in non OBJprog");
	      else
		obj_interpret (obj, line);
	    }
	  else if (!str_cmp (ctrl, "room"))
	    {

	      line = one_argument (data, ctrl);
	      if (!room)
		bug ("room command in non ROOMprog");
	      else
		room_interpret (room, line);
	    }
	  else
	    {

	      if (!mob)
		bugf
		  ("Normal Mud command in non-MOBprog, prog vnum %ld", pvnum);
	      else
		interpret (mob, data);
	    }
	}
    }
  call_level--;
  callstack_aborted[call_level] = false;
}

void
p_act_trigger (const char *argument, CharData * mob, ObjData * obj,
	       RoomIndex * room, CharData * ch, const void *arg1,
	       const void *arg2, flag_t type)
{
  ProgList *prg;

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("Multiple program types in ACT trigger.");
      return;
    }

  if (mob)
    {
      if (!IsNPC (mob))
	{
	  bug ("Attempt to access p_act_trigger on a PLAYER.");
	  return;
	}

      for (prg = mob->pIndexData->mprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && strstr (argument, prg->trig_phrase) != NULL)
	    {
	      program_flow (prg, mob, NULL, NULL, ch, arg1, arg2);
	      break;
	    }
	}
    }
  else if (obj)
    {
      for (prg = obj->pIndexData->oprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && strstr (argument, prg->trig_phrase) != NULL)
	    {
	      program_flow (prg, NULL, obj, NULL, ch, arg1, arg2);
	      break;
	    }
	}
    }
  else if (room)
    {
      for (prg = room->rprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && strstr (argument, prg->trig_phrase) != NULL)
	    {
	      program_flow (prg, NULL, NULL, room, ch, arg1, arg2);
	      break;
	    }
	}
    }
  else
    bug ("ACT trigger with no program type.");

  return;
}

bool
p_percent_trigger (CharData * mob, ObjData * obj,
		   RoomIndex * room, CharData * ch,
		   const void *arg1, const void *arg2, flag_t type)
{
  ProgList *prg;

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("Multiple program types in PERCENT trigger.");
      return (false);
    }

  if (mob)
    {
      for (prg = mob->pIndexData->mprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && number_percent () < atoi (prg->trig_phrase))
	    {
	      program_flow (prg, mob, NULL, NULL, ch, arg1, arg2);
	      return (true);
	    }
	}
    }
  else if (obj)
    {
      for (prg = obj->pIndexData->oprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && number_percent () < atoi (prg->trig_phrase))
	    {
	      program_flow (prg, NULL, obj, NULL, ch, arg1, arg2);
	      return (true);
	    }
	}
    }
  else if (room)
    {
      for (prg = room->rprog_first; prg != NULL; prg = prg->next)
	{
	  if (prg->trig_type == type
	      && number_percent () < atoi (prg->trig_phrase))
	    {
	      program_flow (prg, NULL, NULL, room, ch, arg1, arg2);
	      return (true);
	    }
	}
    }
  else
    bug ("PERCENT trigger missing program type.");

  return (false);
}

void
p_bribe_trigger (CharData * mob, CharData * ch, money_t amount)
{
  ProgList *prg;

  for (prg = mob->pIndexData->mprog_first; prg; prg = prg->next)
    {
      if (prg->trig_type == TRIG_BRIBE
	  && amount >= strtoul (prg->trig_phrase, (char **) NULL, 10))
	{
	  program_flow (prg, mob, NULL, NULL, ch, NULL, NULL);
	  break;
	}
    }
  return;
}

bool
p_exit_trigger (CharData * ch, int dir, prog_t type)
{
  CharData *mob;
  ObjData *obj;
  RoomIndex *room;
  ProgList *prg;

  if (type == PRG_MPROG)
    {
      for (mob = ch->in_room->person_first; mob != NULL;
	   mob = mob->next_in_room)
	{
	  if (IsNPC (mob)
	      && (HasTriggerMob (mob, TRIG_EXIT)
		  || HasTriggerMob (mob, TRIG_EXALL)))
	    {
	      for (prg = mob->pIndexData->mprog_first; prg; prg = prg->next)
		{

		  if (prg->trig_type == TRIG_EXIT
		      && dir == atoi (prg->trig_phrase)
		      && mob->position == mob->pIndexData->default_pos
		      && can_see (mob, ch))
		    {
		      program_flow (prg, mob, NULL, NULL, ch, NULL, NULL);
		      return true;
		    }
		  else if (prg->trig_type == TRIG_EXALL
			   && dir == atoi (prg->trig_phrase))
		    {
		      program_flow (prg, mob, NULL, NULL, ch, NULL, NULL);
		      return true;
		    }
		}
	    }
	}
    }
  else if (type == PRG_OPROG)
    {
      for (obj = ch->in_room->content_first; obj != NULL;
	   obj = obj->next_content)
	{
	  if (HasTriggerObj (obj, TRIG_EXALL))
	    {
	      for (prg = obj->pIndexData->oprog_first; prg; prg = prg->next)
		{
		  if (prg->trig_type == TRIG_EXALL
		      && dir == atoi (prg->trig_phrase))
		    {
		      program_flow (prg, NULL, obj, NULL, ch, NULL, NULL);
		      return true;
		    }
		}
	    }
	}

      for (mob = ch->in_room->person_first; mob; mob = mob->next_in_room)
	{
	  for (obj = mob->carrying_first; obj; obj = obj->next_content)
	    {
	      if (HasTriggerObj (obj, TRIG_EXALL))
		{
		  for (prg = obj->pIndexData->oprog_first; prg;
		       prg = prg->next)
		    {
		      if (prg->trig_type == TRIG_EXALL
			  && dir == atoi (prg->trig_phrase))
			{
			  program_flow (prg, NULL, obj, NULL, ch, NULL, NULL);
			  return true;
			}
		    }
		}
	    }
	}
    }
  else if (type == PRG_RPROG)
    {
      room = ch->in_room;

      if (HasTriggerRoom (room, TRIG_EXALL))
	{
	  for (prg = room->rprog_first; prg; prg = prg->next)
	    {
	      if (prg->trig_type == TRIG_EXALL
		  && dir == atoi (prg->trig_phrase))
		{
		  program_flow (prg, NULL, NULL, room, ch, NULL, NULL);
		  return true;
		}
	    }
	}
    }

  return false;
}

void
p_give_trigger (CharData * mob, ObjData * obj, RoomIndex * room,
		CharData * ch, ObjData * dropped, flag_t type)
{

  char buf[MIL];
  const char *p;
  ProgList *prg;

  if ((mob && obj) || (mob && room) || (obj && room))
    {
      bug ("Multiple program types in GIVE trigger.");
      return;
    }

  if (mob)
    {
      for (prg = mob->pIndexData->mprog_first; prg; prg = prg->next)
	if (prg->trig_type == TRIG_GIVE)
	  {
	    p = prg->trig_phrase;

	    if (is_number (p))
	      {
		if (dropped->pIndexData->vnum == atov (p))
		  {
		    program_flow (prg, mob, NULL, NULL,
				  ch, (void *) dropped, NULL);
		    return;
		  }
	      }
	    else
	      {
		while (*p)
		  {
		    p = one_argument (p, buf);

		    if (is_name (buf, dropped->name) || !str_cmp ("all", buf))
		      {
			program_flow (prg, mob, NULL, NULL, ch,
				      (void *) dropped, NULL);
			return;
		      }
		  }
	      }
	  }
    }
  else if (obj)
    {
      for (prg = obj->pIndexData->oprog_first; prg; prg = prg->next)
	if (prg->trig_type == type)
	  {
	    program_flow (prg, NULL, obj, NULL, ch, (void *) obj, NULL);
	    return;
	  }
    }
  else if (room)
    {
      for (prg = room->rprog_first; prg; prg = prg->next)
	if (prg->trig_type == type)
	  {
	    p = prg->trig_phrase;

	    if (is_number (p))
	      {
		if (dropped->pIndexData->vnum == atov (p))
		  {
		    program_flow (prg, NULL, NULL, room,
				  ch, (void *) dropped, NULL);
		    return;
		  }
	      }
	    else
	      {
		while (*p)
		  {
		    p = one_argument (p, buf);

		    if (is_name (buf, dropped->name) || !str_cmp ("all", buf))
		      {
			program_flow (prg, NULL, NULL, room, ch,
				      (void *) dropped, NULL);
			return;
		      }
		  }
	      }
	  }
    }
}

void
p_greet_trigger (CharData * ch, prog_t type)
{
  CharData *mob;
  ObjData *obj;
  RoomIndex *room;

  if (type == PRG_MPROG)
    {
      for (mob = ch->in_room->person_first; mob != NULL;
	   mob = mob->next_in_room)
	{
	  if (IsNPC (mob)
	      && (HasTriggerMob (mob, TRIG_GREET)
		  || HasTriggerMob (mob, TRIG_GRALL)))
	    {

	      if (HasTriggerMob (mob, TRIG_GREET)
		  && mob->position == mob->pIndexData->default_pos
		  && can_see (mob, ch))
		p_percent_trigger (mob, NULL, NULL, ch, NULL, NULL,
				   TRIG_GREET);
	      else if (HasTriggerMob (mob, TRIG_GRALL))
		p_percent_trigger (mob, NULL, NULL, ch, NULL, NULL,
				   TRIG_GRALL);
	    }
	}
    }
  else if (type == PRG_OPROG)
    {
      for (obj = ch->in_room->content_first; obj != NULL;
	   obj = obj->next_content)
	{
	  if (HasTriggerObj (obj, TRIG_GRALL))
	    {
	      p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL, TRIG_GRALL);
	      return;
	    }
	}

      for (mob = ch->in_room->person_first; mob; mob = mob->next_in_room)
	{
	  for (obj = mob->carrying_first; obj; obj = obj->next_content)
	    {
	      if (HasTriggerObj (obj, TRIG_GRALL))
		{
		  p_percent_trigger (NULL, obj, NULL, ch, NULL, NULL,
				     TRIG_GRALL);
		  return;
		}
	    }
	}
    }
  else if (type == PRG_RPROG)
    {
      room = ch->in_room;

      if (HasTriggerRoom (room, TRIG_GRALL))
	p_percent_trigger (NULL, NULL, room, ch, NULL, NULL, TRIG_GRALL);
    }

  return;
}

void
p_hprct_trigger (CharData * mob, CharData * ch)
{
  ProgList *prg;

  for (prg = mob->pIndexData->mprog_first; prg != NULL; prg = prg->next)
    if ((prg->trig_type == TRIG_HPCNT)
	&& ((100 * mob->hit / mob->max_hit) < atoi (prg->trig_phrase)))
      {
	program_flow (prg, mob, NULL, NULL, ch, NULL, NULL);
	break;
      }
}
