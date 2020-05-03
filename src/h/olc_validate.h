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

#ifndef __OLC_VALIDATE_H_
#define __OLC_VALIDATE_H_  1

Validate_Fun (validate_roomvnum)
{
  return get_room_index (*(vnum_t *) arg) != NULL;
}

Validate_Fun (validate_port)
{
  return (*(int *) arg > 1024);
}

Validate_Fun (validate_keyword)
{
  const char *str = (const char *) arg;

  while (*str)
    {
      if (isalpha (*str) && !isupper (*str))
	{
	  chprintln (ch, "Keyword letters must be all uppercase.");
	  return false;
	}
      str++;
    }
  return true;
}

Validate_Fun (validate_ip)
{
  int count = 0;
  const char *str = (const char *) arg;

  while (*str != NUL)
    {
      if (*str == '.')
	count++;
      else if (!isdigit (*str))
	{
	  olc_msg (ch, NULL,
		   "IP must be in standard numbers-and-dots notation.");
	  return false;
	}
    }

  if (count != 3)
    {
      olc_msg (ch, NULL, "IP must be in standard numbers-and-dots notation.");
      return false;
    }

  return true;
}

Validate_Fun (validate_autoset)
{
  switch (ch->desc->editor)
    {
    case ED_MOBILE:
      {
	CharIndex *pMob;

	GetEdit (ch, CharIndex, pMob);
	if (pMob->level < 1)
	  {
	    if (pMob->autoset != 0)
	      pMob->autoset = 0;

	    chprintln (ch, "You can't autoset this mob.");
	    return false;
	  }
	break;
      }
    case ED_OBJECT:
      {
	ObjIndex *pObj;

	GetEdit (ch, ObjIndex, pObj);
	if (pObj->level < 1 ||
	    (pObj->item_type != ITEM_ARMOR && pObj->item_type != ITEM_WEAPON))
	  {
	    chprintln (ch, "You can't autoset this obj.");
	    return false;
	  }
	break;
      }
    default:
      break;
    }
  return true;
}

Validate_Fun (validate_level)
{
  int num = *(int *) arg;

  if (num < 0 || num > MAX_LEVEL)
    {
      chprintlnf (ch, "Number must be between 0 and %d.", MAX_LEVEL);
      return false;
    }

  return true;
}

Validate_Fun (validate_align)
{
  int num = *(int *) arg;

  if (num < -1000 || num > 1000)
    {
      chprintln (ch, "Number must be between -1000 and 1000.");
      return false;
    }

  return true;
}


Validate_Fun (validate_weapon)
{
  ObjIndex *obj;

  if ((obj = get_obj_index (*(vnum_t *) arg)) == NULL)
    {
      chprintln (ch, "Invalid vnum.");
      return false;
    }

  if (obj->item_type != ITEM_WEAPON)
    {
      chprintln (ch, "Thats not a vnum of a weapon.");
      return false;
    }
  return true;
}

Validate_Fun (validate_adept)
{
  int value = *(int *) arg;

  if (value < 20 || value > 90)
    {
      chprintln (ch, "Please choose a value between 20 & 90");
      return false;
    }

  return true;
}

Validate_Fun (validate_hmv)
{
  int *value = (int *) arg;

  if (*value < 1 || *value > 100)
    {
      chprintln (ch, "Please choose a value from 1-100.");
      return false;
    }
  return true;
}

Validate_Fun (validate_group)
{
  int g;

  g = group_lookup ((const char *) arg);

  if (g < 0 || g >= top_group)
    {
      chprintln (ch, "That group doesn't exist");
      return false;
    }
  return true;
}

Validate_Fun (validate_groupname)
{
  if (group_lookup ((const char *) arg) != -1)
    {
      chprintln (ch, "That group already exists.");
      return false;
    }

  return true;
}

bool
validate_arg (CharData * ch, const void *val, const void *arg)
{
  Validate_F *fun = (Validate_F *) val;

  return (!fun || fun (ch, arg));
}

#endif
