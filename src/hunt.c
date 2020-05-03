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

struct hash_link
{
  int key;
  struct hash_link *next;
  void *data;
};

struct hash_header
{
  int rec_size;
  int table_size;
  int *keylist, klistsize, klistlen;
  struct hash_link **buckets;
};

#define    	HASH_KEY(ht,key)    	((((unsigned int)(key))*17)%(ht)->table_size)

struct hunting_data
{
  char *name;
  CharData **victim;
};

struct room_q
{
  int room_nr;
  struct room_q *next_q;
};

struct nodes
{
  int visited;
  int ancestor;
};

#define    	IS_DIR          (get_room_index(q_head->room_nr)->exit[i])
#define    	GO_OK           (!IsSet( IS_DIR->exit_info, EX_CLOSED ))
#define    	GO_OK_SMARTER   1

Proto (void init_hash_table, (struct hash_header *, int, int));
Proto (void init_world, (RoomIndex * room_db[]));

Proto (CharData * get_char_area_restrict, (CharData * ch, char *argument));
Proto (void destroy_hash_table, (struct hash_header *));
Proto (void _hash_enter, (struct hash_header *, int, void *));

Proto (RoomIndex * room_find, (RoomIndex * room_db[], int));
Proto (void *hash_find, (struct hash_header *, int));
Proto (int room_enter, (RoomIndex * rb[], int, RoomIndex *));
Proto (int hash_enter, (struct hash_header *, int, void *));

Proto (RoomIndex * room_find_or_create, (RoomIndex * rb[], int));
Proto (void *hash_find_or_create, (struct hash_header *, int));
Proto (int room_remove, (RoomIndex * rb[], int));
Proto (void *hash_remove, (struct hash_header *, int));
Proto (int exit_ok, (ExitData *));
Proto (int find_path, (int, int, CharData *, int, int));

void
init_hash_table (struct hash_header *ht, int rec_size, int table_size)
{
  ht->rec_size = rec_size;
  ht->table_size = table_size;

  alloc_mem (ht->buckets, struct hash_link *, table_size);
  alloc_mem (ht->keylist, int, (ht->klistsize = 128));

  ht->klistlen = 0;
}

void
init_world (RoomIndex * room_db[])
{
  memset (room_db, 0, sizeof (RoomIndex *) * top_room_index);
}

CharData *
get_char_area (CharData * ch, char *argument)
{
  char arg[MIL];
  CharData *ach;
  int number;
  int count;

  if (NullStr (argument))
    return NULL;

  if ((ach = get_char_room (ch, NULL, argument)) != NULL)
    return ach;

  number = number_argument (argument, arg);
  count = 0;
  for (ach = char_first; ach != NULL; ach = ach->next)
    {
      if (ach->in_room == NULL
	  || ach->in_room->area != ch->in_room->area
	  || !can_see (ch, ach) || !is_name (arg, ach->name))
	continue;
      if (++count == number)
	return ach;
    }

  return NULL;
}

void
destroy_hash_table (struct hash_header *ht)
{
  int i;
  struct hash_link *scan, *temp;

  for (i = 0; i < ht->table_size; i++)
    {
      for (scan = ht->buckets[i]; scan;)
	{
	  temp = scan->next;
	  free_mem (scan);
	  scan = temp;
	}
    }
  free_mem (ht->buckets);
  free_mem (ht->keylist);
}

void
_hash_enter (struct hash_header *ht, int key, void *data)
{

  struct hash_link *temp;
  int i;

  alloc_mem (temp, struct hash_link, 1);

  temp->key = key;
  temp->next = ht->buckets[HASH_KEY (ht, key)];
  temp->data = data;
  ht->buckets[HASH_KEY (ht, key)] = temp;
  if (ht->klistlen >= ht->klistsize)
    {
      ht->klistsize *= 2;
      realloc_mem (ht->keylist, int, ht->klistsize);
    }
  for (i = ht->klistlen; i >= 0; i--)
    {
      if (ht->keylist[i - 1] < key)
	{
	  ht->keylist[i] = key;
	  break;
	}
      ht->keylist[i] = ht->keylist[i - 1];
    }
  ht->klistlen++;
}

RoomIndex *
room_find (RoomIndex * room_db[], int key)
{
  return ((key < top_room_index && key > -1) ? room_db[key] : 0);
}

void *
hash_find (struct hash_header *ht, int key)
{
  struct hash_link *scan;

  scan = ht->buckets[HASH_KEY (ht, key)];

  while (scan && scan->key != key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

int
room_enter (RoomIndex * rb[], int key, RoomIndex * rm)
{
  RoomIndex *temp;

  temp = room_find (rb, key);
  if (temp)
    return (0);

  rb[key] = rm;
  return (1);
}

int
hash_enter (struct hash_header *ht, int key, void *data)
{
  void *temp;

  temp = hash_find (ht, key);
  if (temp)
    return 0;

  _hash_enter (ht, key, data);
  return 1;
}

RoomIndex *
room_find_or_create (RoomIndex * rb[], int key)
{
  RoomIndex *rv;

  rv = room_find (rb, key);
  if (rv)
    return rv;

  alloc_mem (rv, RoomIndex, 1);
  rb[key] = rv;

  return rv;
}

void *
hash_find_or_create (struct hash_header *ht, int key)
{
  void *rval;

  rval = hash_find (ht, key);
  if (rval)
    return rval;

  alloc_mem (rval, void *, ht->rec_size);
  _hash_enter (ht, key, rval);

  return rval;
}

int
room_remove (RoomIndex * rb[], int key)
{
  RoomIndex *tmp;

  tmp = room_find (rb, key);
  if (tmp)
    {
      rb[key] = 0;
      free_mem (tmp);
    }
  return (0);
}

void *
hash_remove (struct hash_header *ht, int key)
{
  struct hash_link **scan;

  scan = ht->buckets + HASH_KEY (ht, key);

  while (*scan && (*scan)->key != key)
    scan = &(*scan)->next;

  if (*scan)
    {
      int i;
      struct hash_link *temp, *aux;

      temp = (struct hash_link *) (*scan)->data;
      aux = *scan;
      *scan = aux->next;
      free_mem (aux);

      for (i = 0; i < ht->klistlen; i++)
	if (ht->keylist[i] == key)
	  break;

      if (i < ht->klistlen)
	{
	  memcpy ((char *) ht->keylist + i + 1,
		  (char *) ht->keylist + i,
		  (ht->klistlen - i) * sizeof (*ht->keylist));
	  ht->klistlen--;
	}

      return temp;
    }

  return NULL;
}

int
exit_ok (ExitData * pexit)
{
  RoomIndex *to_room;

  if ((pexit == NULL) || (to_room = pexit->u1.to_room) == NULL)
    return 0;

  return 1;
}

int
find_path (int in_room_vnum, int out_room_vnum, CharData * ch,
	   int depth, int in_zone)
{
  struct room_q *tmp_q, *q_head, *q_tail;
  struct hash_header x_room;
  int i, tmp_room, count = 0, thru_doors;
  RoomIndex *herep;
  RoomIndex *startp;
  ExitData *exitp;

  if (depth < 0)
    {
      thru_doors = true;
      depth = -depth;
    }
  else
    {
      thru_doors = false;
    }

  startp = get_room_index (in_room_vnum);

  init_hash_table (&x_room, sizeof (int), 2048);

  hash_enter (&x_room, in_room_vnum, (void *) -1);

  alloc_mem (q_head, struct room_q, 1);

  q_tail = q_head;
  q_tail->room_nr = in_room_vnum;
  q_tail->next_q = 0;

  while (q_head)
    {
      herep = get_room_index (q_head->room_nr);

      if (herep->area == startp->area || !in_zone)
	{
	  for (i = 0; i < MAX_DIR; i++)
	    {
	      exitp = herep->exit[i];
	      if (exit_ok (exitp) && (thru_doors ? GO_OK_SMARTER : GO_OK))
		{
		  tmp_room = herep->exit[i]->u1.to_room->vnum;
		  if (tmp_room != out_room_vnum)
		    {
		      if (!hash_find (&x_room, tmp_room) && (count < depth))
			{
			  count++;

			  alloc_mem (tmp_q, struct room_q, 1);

			  tmp_q->room_nr = tmp_room;
			  tmp_q->next_q = 0;
			  q_tail->next_q = tmp_q;
			  q_tail = tmp_q;

			  hash_enter (&x_room, tmp_room, ((int)
							  hash_find
							  (&x_room,
							   q_head->room_nr)
							  ==
							  -1) ? (void *) (i +
									  1)
				      : hash_find (&x_room, q_head->room_nr));
			}
		    }
		  else
		    {

		      tmp_room = q_head->room_nr;
		      for (; q_head; q_head = tmp_q)
			{
			  tmp_q = q_head->next_q;
			  free_mem (q_head);
			}

		      if ((int) hash_find (&x_room, tmp_room) == -1)
			{
			  if (x_room.buckets)
			    {
			      destroy_hash_table (&x_room);
			    }
			  return (i);
			}
		      else
			{
			  i = (int) hash_find (&x_room, tmp_room);
			  if (x_room.buckets)
			    {
			      destroy_hash_table (&x_room);
			    }
			  return (-1 + i);
			}
		    }
		}
	    }
	}

      tmp_q = q_head->next_q;
      free_mem (q_head);
      q_head = tmp_q;
    }

  if (x_room.buckets)
    {
      destroy_hash_table (&x_room);
    }
  return -1;
}

Do_Fun (do_hunt)
{
  char buf[MSL];
  char arg[MSL];
  CharData *victim;
  int direction;
  bool fArea;

  one_argument (argument, arg);

  if (get_skill (ch, gsn_hunt) == 0 || !can_use_skpell (ch, gsn_hunt))
    {
      chprintln (ch, "You don't know how to hunt.");
      return;
    }

  if (NullStr (arg))
    {
      chprintln (ch, "Whom are you trying to hunt?");
      return;
    }

  fArea = (get_trust (ch) < MAX_LEVEL);

  if (IsNPC (ch))
    victim = get_char_world (ch, arg);
  else if (fArea)
    victim = get_char_area (ch, arg);
  else
    victim = get_char_world (ch, arg);

  if (victim == NULL)
    {
      chprintln (ch, "No-one around by that name.");
      return;
    }

  if (ch->in_room == victim->in_room)
    {
      act ("$N is here!", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->move > 2)
    ch->move -= 3;
  else
    {
      chprintln (ch, "You're too exhausted to hunt anyone!");
      return;
    }

  act ("$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM);
  WaitState (ch, skill_table[gsn_hunt].beats);
  direction =
    find_path (ch->in_room->vnum, victim->in_room->vnum, ch, -40000, fArea);

  if (direction == -1)
    {
      act ("You couldn't find a path to $N from here.",
	   ch, NULL, victim, TO_CHAR);
      return;
    }

  if (direction < 0 || direction >= MAX_DIR)
    {
      chprintln (ch, "Hmm... Something seems to be wrong.");
      return;
    }

  if ((IsNPC (ch) && number_percent () > 50) ||
      (!IsNPC (ch) && number_percent () > get_skill (ch, gsn_hunt)))
    {
      do
	{
	  direction = number_door ();
	}
      while ((ch->in_room->exit[direction] == NULL) ||
	     (ch->in_room->exit[direction]->u1.to_room == NULL));
    }

  sprintf (buf, "$N is %s from here.", dir_name[direction]);
  act (buf, ch, NULL, victim, TO_CHAR);
  check_improve (ch, gsn_hunt, true, 1);
  return;
}

void
hunt_victim (CharData * ch)
{
  int dir;
  bool found;
  CharData *tmp;

  if (ch == NULL || ch->hunting == NULL || !IsNPC (ch))
    return;

  for (found = 0, tmp = char_first; tmp && !found; tmp = tmp->next)
    if (ch->hunting == tmp)
      found = 1;

  if (!found || !can_see (ch, ch->hunting))
    {
      do_function (ch, &do_say, "Damn!  My prey is gone!!");
      ch->hunting = NULL;
      return;
    }

  if (ch->in_room == ch->hunting->in_room)
    {
      if (number_percent () < 60)
	{
	  act (CTAG (_SAY1) "$n" CTAG (_SAY1) " glares at $N"
	       CTAG (_SAY1) " and says, '" CTAG (_SAY2)
	       "Ye shall DIE!" CTAG (_SAY1) "'{x", ch, NULL,
	       ch->hunting, TO_NOTVICT);
	  act (CTAG (_SAY1) "$n" CTAG (_SAY1)
	       " glares at you and says, '" CTAG (_SAY2)
	       "Ye shall DIE!" CTAG (_SAY1) "'{x", ch, NULL,
	       ch->hunting, TO_VICT);
	  act (CTAG (_SAY1) "You glare at $N" CTAG (_SAY1)
	       " and say, '" CTAG (_SAY2) "Ye shall DIE!"
	       CTAG (_SAY1) "'{x", ch, NULL, ch->hunting, TO_CHAR);
	}
      else
	{
	  act (CTAG (_SAY1) "$n" CTAG (_SAY1) " glares at $N"
	       CTAG (_SAY1) " and says, '" CTAG (_SAY2)
	       "Hey, I remember you!" CTAG (_SAY1) "'{x", ch, NULL,
	       ch->hunting, TO_NOTVICT);
	  act (CTAG (_SAY1) "$n" CTAG (_SAY1)
	       " glares at you and says, '" CTAG (_SAY2)
	       "Hey, I remember you!" CTAG (_SAY1) "'{x", ch, NULL,
	       ch->hunting, TO_VICT);
	  act (CTAG (_SAY1) "You glare at $N" CTAG (_SAY1)
	       " and say, '" CTAG (_SAY2) "Hey, I remember you!"
	       CTAG (_SAY1) "'{x", ch, NULL, ch->hunting, TO_CHAR);
	}
      multi_hit (ch, ch->hunting, TYPE_UNDEFINED);
      ch->hunting = NULL;
      return;
    }

  WaitState (ch, skill_table[gsn_hunt].beats);
  dir =
    find_path (ch->in_room->vnum, ch->hunting->in_room->vnum, ch,
	       -40000, true);

  if (dir < 0 || dir >= MAX_DIR)
    {
      act (CTAG (_SAY1) "$n" CTAG (_SAY1) " says '" CTAG (_SAY2)
	   "Damn!  Lost $M" CTAG (_SAY1) "!'{x", ch, NULL, ch->hunting,
	   TO_ROOM);
      ch->hunting = NULL;
      return;
    }

  if (number_percent () > 50)
    {
      do
	{
	  dir = number_door ();
	}
      while ((ch->in_room->exit[dir] == NULL) ||
	     (ch->in_room->exit[dir]->u1.to_room == NULL));
    }

  if (IsSet (ch->in_room->exit[dir]->exit_info, EX_CLOSED))
    {
      do_function (ch, &do_open, (char *) dir_name[dir]);
      return;
    }

  move_char (ch, dir, false);
  return;
}
