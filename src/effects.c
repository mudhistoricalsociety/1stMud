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
#include "recycle.h"

void
acid_effect (void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      RoomIndex *room = (RoomIndex *) vo;
      ObjData *obj, *obj_next;

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  acid_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CharData *victim = (CharData *) vo;
      ObjData *obj, *obj_next;


      for (obj = victim->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  acid_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      ObjData *obj = (ObjData *) vo;
      ObjData *t_obj, *n_obj;
      int chance;
      char *msg;

      if (IsObjStat (obj, ITEM_BURN_PROOF) ||
	  IsObjStat (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IsObjStat (obj, ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_CONTAINER:
	case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
	  msg = "$p fumes and dissolves.";
	  break;
	case ITEM_ARMOR:
	  msg = "$p is pitted and etched.";
	  break;
	case ITEM_CLOTHING:
	  msg = "$p is corroded into scrap.";
	  break;
	case ITEM_STAFF:
	case ITEM_WAND:
	  chance -= 10;
	  msg = "$p corrodes and breaks.";
	  break;
	case ITEM_SCROLL:
	  chance += 10;
	  msg = "$p is burned into waste.";
	  break;
	}

      chance = Range (5, chance, 95);

      if (number_percent () > chance)
	return;

      if (obj->carried_by != NULL)
	act (msg, obj->carried_by, obj, NULL, TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->person_first != NULL)
	act (msg, obj->in_room->person_first, obj, NULL, TO_ALL);

      if (obj->item_type == ITEM_ARMOR)
	{
	  AffectData *paf;
	  bool af_found = false;
	  int i;

	  affect_enchant (obj);

	  for (paf = obj->affect_first; paf != NULL; paf = paf->next)
	    {
	      if (paf->location == APPLY_AC)
		{
		  af_found = true;
		  paf->type = -1;
		  paf->modifier += 1;
		  paf->level = Max (paf->level, level);
		  break;
		}
	    }

	  if (!af_found)

	    {
	      paf = new_affect ();

	      paf->type = -1;
	      paf->level = level;
	      paf->duration = -1;
	      paf->location = APPLY_AC;
	      paf->modifier = 1;
	      paf->bitvector = 0;
	      Link (paf, obj->affect, next, prev);
	    }

	  if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
	    for (i = 0; i < MAX_AC; i++)
	      obj->carried_by->armor[i] += 1;
	  return;
	}


      if (obj->content_first)
	{
	  for (t_obj = obj->content_first; t_obj != NULL; t_obj = n_obj)
	    {
	      n_obj = t_obj->next_content;
	      obj_from_obj (t_obj);
	      if (obj->in_room != NULL)
		obj_to_room (t_obj, obj->in_room);
	      else if (obj->carried_by != NULL)
		obj_to_room (t_obj, obj->carried_by->in_room);
	      else
		{
		  extract_obj (t_obj);
		  continue;
		}

	      acid_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
	    }
	}

      extract_obj (obj);
      return;
    }
}

void
cold_effect (void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      RoomIndex *room = (RoomIndex *) vo;
      ObjData *obj, *obj_next;

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  cold_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CharData *victim = (CharData *) vo;
      ObjData *obj, *obj_next;


      if (!saves_spell (level / 4 + dam / 20, victim, DAM_COLD))
	{
	  AffectData af;

	  act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
	  act ("A chill sinks deep into your bones.", victim, NULL, NULL,
	       TO_CHAR);
	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("chill touch");
	  af.level = level;
	  af.duration = 6;
	  af.location = APPLY_STR;
	  af.modifier = -1;
	  af.bitvector = 0;
	  affect_join (victim, &af);
	}


      if (!IsNPC (victim))
	gain_condition (victim, COND_HUNGER, dam / 20);


      for (obj = victim->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  cold_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      ObjData *obj = (ObjData *) vo;
      int chance;
      char *msg;

      if (IsObjStat (obj, ITEM_BURN_PROOF) ||
	  IsObjStat (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IsObjStat (obj, ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_POTION:
	  msg = "$p freezes and shatters!";
	  chance += 25;
	  break;
	case ITEM_DRINK_CON:
	  msg = "$p freezes and shatters!";
	  chance += 5;
	  break;
	}

      chance = Range (5, chance, 95);

      if (number_percent () > chance)
	return;

      if (obj->carried_by != NULL)
	act (msg, obj->carried_by, obj, NULL, TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->person_first != NULL)
	act (msg, obj->in_room->person_first, obj, NULL, TO_ALL);

      extract_obj (obj);
      return;
    }
}

void
fire_effect (void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      RoomIndex *room = (RoomIndex *) vo;
      ObjData *obj, *obj_next;

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  fire_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CharData *victim = (CharData *) vo;
      ObjData *obj, *obj_next;


      if (!IsAffected (victim, AFF_BLIND) &&
	  !saves_spell (level / 4 + dam / 20, victim, DAM_FIRE))
	{
	  AffectData af;

	  act ("$n is blinded by smoke!", victim, NULL, NULL, TO_ROOM);
	  act ("Your eyes tear up from smoke...you can't see a thing!",
	       victim, NULL, NULL, TO_CHAR);

	  af.where = TO_AFFECTS;
	  af.type = skill_lookup ("fire breath");
	  af.level = level;
	  af.duration = number_range (0, level / 10);
	  af.location = APPLY_HITROLL;
	  af.modifier = -4;
	  af.bitvector = AFF_BLIND;

	  affect_to_char (victim, &af);
	}


      if (!IsNPC (victim))
	gain_condition (victim, COND_THIRST, dam / 20);


      for (obj = victim->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;

	  fire_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      ObjData *obj = (ObjData *) vo;
      ObjData *t_obj, *n_obj;
      int chance;
      char *msg;

      if (IsObjStat (obj, ITEM_BURN_PROOF) ||
	  IsObjStat (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IsObjStat (obj, ITEM_BLESS))
	chance -= 5;
      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_CONTAINER:
	  msg = "$p ignites and burns!";
	  break;
	case ITEM_POTION:
	  chance += 25;
	  msg = "$p bubbles and boils!";
	  break;
	case ITEM_SCROLL:
	  chance += 50;
	  msg = "$p crackles and burns!";
	  break;
	case ITEM_STAFF:
	  chance += 10;
	  msg = "$p smokes and chars!";
	  break;
	case ITEM_WAND:
	  msg = "$p sparks and sputters!";
	  break;
	case ITEM_FOOD:
	  msg = "$p blackens and crisps!";
	  break;
	case ITEM_PILL:
	  msg = "$p melts and drips!";
	  break;
	}

      chance = Range (5, chance, 95);

      if (number_percent () > chance)
	return;

      if (obj->carried_by != NULL)
	act (msg, obj->carried_by, obj, NULL, TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->person_first != NULL)
	act (msg, obj->in_room->person_first, obj, NULL, TO_ALL);

      if (obj->content_first)
	{


	  for (t_obj = obj->content_first; t_obj != NULL; t_obj = n_obj)
	    {
	      n_obj = t_obj->next_content;
	      obj_from_obj (t_obj);
	      if (obj->in_room != NULL)
		obj_to_room (t_obj, obj->in_room);
	      else if (obj->carried_by != NULL)
		obj_to_room (t_obj, obj->carried_by->in_room);
	      else
		{
		  extract_obj (t_obj);
		  continue;
		}
	      fire_effect (t_obj, level / 2, dam / 2, TARGET_OBJ);
	    }
	}

      extract_obj (obj);
      return;
    }
}

void
poison_effect (void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      RoomIndex *room = (RoomIndex *) vo;
      ObjData *obj, *obj_next;

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  poison_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CharData *victim = (CharData *) vo;
      ObjData *obj, *obj_next;


      if (!saves_spell (level / 4 + dam / 20, victim, DAM_POISON))
	{
	  AffectData af;

	  chprintln (victim, "You feel poison coursing through your veins.");
	  act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);

	  af.where = TO_AFFECTS;
	  af.type = gsn_poison;
	  af.level = level;
	  af.duration = level / 2;
	  af.location = APPLY_STR;
	  af.modifier = -1;
	  af.bitvector = AFF_POISON;
	  affect_join (victim, &af);
	}


      for (obj = victim->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  poison_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      ObjData *obj = (ObjData *) vo;
      int chance;

      if (IsObjStat (obj, ITEM_BURN_PROOF) ||
	  IsObjStat (obj, ITEM_BLESS) || number_range (0, 4) == 0)
	return;

      chance = level / 4 + dam / 10;
      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_FOOD:
	  break;
	case ITEM_DRINK_CON:
	  if (obj->value[0] == obj->value[1])
	    return;
	  break;
	}

      chance = Range (5, chance, 95);

      if (number_percent () > chance)
	return;

      obj->value[3] = 1;
      return;
    }
}

void
shock_effect (void *vo, int level, int dam, int target)
{
  if (target == TARGET_ROOM)
    {
      RoomIndex *room = (RoomIndex *) vo;
      ObjData *obj, *obj_next;

      for (obj = room->content_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  shock_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_CHAR)
    {
      CharData *victim = (CharData *) vo;
      ObjData *obj, *obj_next;


      if (!saves_spell (level / 4 + dam / 20, victim, DAM_LIGHTNING))
	{
	  chprintln (victim, "Your muscles stop responding.");
	  DazeState (victim, Max (12, level / 4 + dam / 20));
	}


      for (obj = victim->carrying_first; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  shock_effect (obj, level, dam, TARGET_OBJ);
	}
      return;
    }

  if (target == TARGET_OBJ)
    {
      ObjData *obj = (ObjData *) vo;
      int chance;
      char *msg;

      if (IsObjStat (obj, ITEM_BURN_PROOF) ||
	  IsObjStat (obj, ITEM_NOPURGE) || number_range (0, 4) == 0)
	return;

      chance = level / 4 + dam / 10;

      if (chance > 25)
	chance = (chance - 25) / 2 + 25;
      if (chance > 50)
	chance = (chance - 50) / 2 + 50;

      if (IsObjStat (obj, ITEM_BLESS))
	chance -= 5;

      chance -= obj->level * 2;

      switch (obj->item_type)
	{
	default:
	  return;
	case ITEM_WAND:
	case ITEM_STAFF:
	  chance += 10;
	  msg = "$p overloads and explodes!";
	  break;
	case ITEM_JEWELRY:
	  chance -= 10;
	  msg = "$p is fused into a worthless lump.";
	}

      chance = Range (5, chance, 95);

      if (number_percent () > chance)
	return;

      if (obj->carried_by != NULL)
	act (msg, obj->carried_by, obj, NULL, TO_ALL);
      else if (obj->in_room != NULL && obj->in_room->person_first != NULL)
	act (msg, obj->in_room->person_first, obj, NULL, TO_ALL);

      extract_obj (obj);
      return;
    }
}
