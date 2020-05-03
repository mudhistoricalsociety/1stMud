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
#include "recycle.h"
#include "olc.h"
#include "tables.h"
#include "vnums.h"

Declare_Do_F (home_buy);
Declare_Do_F (home_add);
Declare_Do_F (home_furnish);
Declare_Do_F (home_unfurnish);
Declare_Do_F (home_sell);
Declare_Do_F (home_describe);
Declare_Do_F (home_name);
Declare_Do_F (home_evict);
Declare_Do_F (home_invite);
Declare_Do_F (home_accept);
Declare_Do_F (home_deny);
Declare_Do_F (do_get_key);
Declare_Do_F (home_help);
Declare_Do_F (home_link);
Declare_Do_F (home_unlink);
Declare_Do_F (do_gohome);
Declare_Do_F (home_door);
Declare_Do_F (home_recall);

Proto (void unlink_reset, (RoomIndex *, ResetData *));

#define HOUSE_PRICE 5000000
#define ROOM_PRICE  3005000

struct home_type
{
  vnum_t vnum;
  money_t cost;
  int quest;
  int tp;
  char *name;
  char *list;
  int type;
  flag_t room_flags;
};

const struct home_type home_table[] = {
  {
   0, 125000, 10, 0, "healrate", "Up a rooms Heal Rate by 50", ROOM_VNUM,
   0},
  {0, 125000, 10, 0, "manarate", "Up a rooms Mana Rate by 50", ROOM_VNUM,
   0},
  {0, 125000, 10, 0, "saferoom", "Make a safe room", ROOM_VNUM, ROOM_SAFE},

  {
   0, 0, 0, 0, NULL, NULL, 0}
};

bool
is_home_owner (CharData * ch, RoomIndex * pHome)
{
  int i;

  if (pHome == NULL)
    return false;

  if (!ch || IsNPC (ch))
    return false;

  for (i = 0; i < HAS_HOME (ch); i++)
    if (ch->pcdata->home[i] == pHome->vnum)
      return true;

  return false;
}

int
tally_resets (RoomIndex * pRoom, money_t cost, int trivia, int quest)
{
  ResetData *pReset;
  int rcount = 0, i;

  if (pRoom == NULL)
    return 0;

  for (i = 0; home_table[i].name != NULL; i++)
    {
      if (home_table[i].type == VNUM_NONE || home_table[i].vnum <= 0)
	continue;
      for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
	{
	  if (pReset->arg1 == home_table[i].vnum)
	    {
	      cost += home_table[i].cost / 3;
	      trivia += home_table[i].tp / 3;
	      quest += home_table[i].quest / 3;
	      rcount++;
	    }
	}
    }
  return rcount;
}

bool
get_key (CharData * ch)
{
  ObjIndex *keyindex = NULL;
  ObjData *key = NULL;

  if (!ch || IsNPC (ch) || ch->pcdata->home_key <= 0)
    return false;

  if ((keyindex = get_obj_index (ch->pcdata->home_key)) == NULL)
    {
      chprintln (ch,
		 "Could not find your key, please have an immortal fix this.");
      return false;
    }

  if (get_obj_here (ch, NULL, keyindex->name) != NULL)
    {
      chprintln (ch, "You already have a key to your home.");
      return true;
    }

  if ((key = create_object (keyindex, 1)) == NULL)
    return false;

  obj_to_char (key, ch);
  chprintln (ch, "A house key appears in your inventory.");

  return true;
}

Do_Fun (do_get_key)
{
  get_key (ch);
}

Do_Fun (home_help)
{
  if (ch->pcdata->home_invite != 0)
    {
      cmd_syntax (ch, NULL, n_fun,
		  "accept    - accept an invitation to someone's home",
		  "deny      - turn down an invitation to someone's home",
		  NULL);
    }
  if (!HAS_HOME (ch))
    cmd_syntax (ch, NULL, n_fun,
		"buy       - purchase a home in <direction> (must be in the right area)",
		NULL);
  else
    {
      cmd_syntax (ch, NULL, n_fun,
		  "sell      - sells your home completly for 1/3 the price",
		  "add       - adds another room in <direction>, must be in your home",
		  "describe  - describes a room using the string editor",
		  "name      - name or rename's a room",
		  "furnish   - add objects or mobiles to a room in your home",
		  "unfurnish - sells objects or mobiles placed in your home",
		  "key       - gets a replacement key for your home",
		  "door      - sets a door on a room",
		  "recall    - sets your recall room for 'gohome'",
		  "evict     - kick's someone out of your home",
		  "invite    - invite someone to your home",
		  "link      - link a room to a specified area",
		  "unlink    - Unlink an exit", NULL);
    }
}

Do_Fun (do_home)
{
  if (!ch)
    return;

  if (IsNPC (ch))
    {
      chprintln (ch, "Mobiles don't have homes =).");
      return;
    }

  vinterpret (ch, n_fun, argument, "buy", home_buy, "add", home_add,
	      "sell", home_sell, "describe", home_describe, "name",
	      home_name, "furnish", home_furnish, "unfurnish",
	      home_unfurnish, "key", do_get_key, "recall", do_gohome,
	      "evict", home_evict, "link", home_link, "unlink",
	      home_unlink, "list", home_help, "invite", home_invite,
	      "accept", home_accept, "deny", home_deny, NULL, home_help);
}

Do_Fun (home_buy)
{
  RoomIndex *pRoom = NULL;
  ObjIndex *pObj;
  AreaData *pArea;
  ExitData *pExit, *toExit;
  int door, rev, iHash;
  vnum_t i;

  if (!ch || IsNPC (ch))
    return;

  pArea = ch->in_room->area;

  if (!pArea || !IsSet (pArea->area_flags, AREA_PLAYER_HOMES))
    {
      chprintln (ch, "Player Homes are not allowed in this area.");
      return;
    }

  if (!check_worth (ch, HOUSE_PRICE, VALUE_GOLD))
    {
      chprintlnf (ch, "{wIt costs %d gold to buy a home.", HOUSE_PRICE);
      return;
    }

  if (ch->pcdata->trivia < 40)
    {
      chprintln (ch, "It costs 40 trivia points to buy a house.");
      return;
    }

  if (HAS_HOME (ch))
    {
      chprintlnf (ch,
		  "{wYou already own a home, use '%s add' or '%s furnish'.",
		  cmd_name (do_home), cmd_name (do_home));
      return;
    }

  if (!IsSet (ch->in_room->room_flags, HOME_ENTRANCE))
    {
      chprintln (ch, "You must be at an entrance to a future home.");
      return;
    }

  if ((door = get_direction (argument)) == -1 || door > 3)
    {
      chprintln (ch,
		 "Which direction from here do you want to start your home?");
      return;
    }

  if (ch->in_room->exit[door])
    {
      chprintln (ch, "A room already exits in that location.");
      return;
    }

  for (i = pArea->min_vnum; i < pArea->max_vnum; i++)
    {
      if ((get_obj_index (i)) == NULL)
	{
	  pObj = new_obj_index ();
	  pObj->vnum = i;
	  pObj->area = pArea;
	  replace_strf (&pObj->name, "key %s home", ch->name);
	  replace_strf (&pObj->short_descr, "%s's key",
			capitalize (ch->name));
	  replace_strf (&pObj->description,
			"The key to %s's home is here.",
			capitalize (ch->name));
	  pObj->item_type = ITEM_KEY;
	  SetBit (pObj->wear_flags, ITEM_TAKE);
	  SetBit (pObj->wear_flags, ITEM_HOLD);
	  pObj->condition = -1;
	  iHash = i % MAX_KEY_HASH;
	  LinkSingle (pObj, obj_index_hash[iHash], next);
	  ch->pcdata->home_key = pObj->vnum;
	  break;
	}
    }

  for (i = pArea->min_vnum; i < pArea->max_vnum; i++)
    {
      if ((get_room_index (i)) == NULL)
	{

	  pRoom = new_room_index ();
	  pRoom->area = pArea;
	  pRoom->vnum = i;
	  pRoom->sector_type = SECT_INSIDE;
	  replace_strf (&pRoom->name, "%s\'s Home", capitalize (ch->name));
	  replace_str (&pRoom->description,
		       "This is your room description. You can edit this with HOME DESCRIBE."
		       NEWLINE);
	  replace_str (&pRoom->owner, ch->name);
	  SetBit (pRoom->room_flags, ROOM_INDOORS | ROOM_SAVE_OBJS);
	  iHash = i % MAX_KEY_HASH;
	  LinkSingle (pRoom, room_index_hash[iHash], next);


	  pExit = new_exit ();
	  pExit->u1.to_room = pRoom;
	  pExit->orig_door = door;
	  SetBit (pExit->rs_flags,
		  EX_ISDOOR | EX_CLOSED | EX_NOPASS | EX_PICKPROOF |
		  EX_DOORBELL);
	  pExit->exit_info = pExit->rs_flags;
	  replace_str (&pExit->keyword, "door");
	  replace_strf (&pExit->description,
			"You see the door to %s's home.",
			capitalize (ch->name));
	  ch->in_room->exit[door] = pExit;

	  rev = rev_dir[door];

	  toExit = new_exit ();
	  toExit->u1.to_room = ch->in_room;
	  toExit->orig_door = rev;
	  toExit->rs_flags = pExit->rs_flags;
	  RemBit (toExit->rs_flags, EX_DOORBELL);
	  toExit->exit_info = toExit->rs_flags;
	  replace_str (&toExit->keyword, "door");
	  replace_strf (&toExit->description,
			"You see the door to %s's home.",
			capitalize (ch->name));
	  pRoom->exit[rev] = toExit;


	  if (get_key (ch) == true)
	    {
	      SetBit (pExit->rs_flags, EX_LOCKED);
	      pExit->exit_info = pExit->rs_flags;
	      pExit->key = ch->pcdata->home_key;
	    }
	  SetBit (pArea->area_flags, AREA_CHANGED);
	  break;
	}
    }

  if (!pRoom)
    {
      chprintln (ch,
		 "A new home could not be created for you. please contact an immortal.");
      return;
    }

  ch->pcdata->home[HAS_HOME (ch)] = pRoom->vnum;
  HAS_HOME (ch) += 1;

  deduct_cost (ch, HOUSE_PRICE, VALUE_GOLD);

  ch->pcdata->trivia -= 40;

  chprintln (ch, "Congratulations, you are now a proud home owner.");
  save_char_obj (ch);
  return;
}

Do_Fun (home_add)
{
  RoomIndex *pRoom = NULL;
  AreaData *pArea;
  ExitData *pExit;
  int door, rev, iHash;
  vnum_t i;

  if (!ch || IsNPC (ch))
    return;

  pArea = ch->in_room->area;

  if (!pArea || !IsSet (pArea->area_flags, AREA_PLAYER_HOMES))
    {
      chprintln (ch, "Player Homes are not allowed in this area.");
      return;
    }

  if (!HAS_HOME (ch))
    {
      chprintlnf (ch, "Use '%s buy' to purchase a home first.",
		  cmd_name (do_home));
      return;
    }

  if (!check_worth (ch, ROOM_PRICE, VALUE_GOLD))
    {
      chprintln (ch, "{wYou don't have enough to buy a another room.");
      return;
    }

  if (ch->pcdata->quest.points < 200)
    {
      chprintln (ch, "{wYou need 200 questpoints to buy another room.");
      return;
    }

  if (HAS_HOME (ch) == PC_HOME_COUNT)
    {
      chprintln (ch, "{wYou have the max allowable rooms in your house.");
      return;
    }

  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "You must add only to your home!");
      return;
    }

  if ((door = get_direction (argument)) == -1 || door > 3)
    {
      chprintln (ch, "Which direction from here do you want the new room?");
      return;
    }

  if (ch->in_room->exit[door])
    {
      chprintln (ch, "A room already exits in that location.");
      return;
    }

  for (i = pArea->min_vnum; i < pArea->max_vnum; i++)
    {
      if ((get_room_index (i)) == NULL)
	{

	  pRoom = new_room_index ();
	  pRoom->area = pArea;
	  pRoom->vnum = i;
	  pRoom->sector_type = SECT_INSIDE;
	  replace_strf (&pRoom->name, "%s\'s %d Room",
			capitalize (ch->name), HAS_HOME (ch) + 1);
	  replace_str (&pRoom->description,
		       "This is your room description. You can edit this with HOME DESCRIBE."
		       NEWLINE);
	  replace_str (&pRoom->owner, ch->name);
	  SetBit (pRoom->room_flags, ROOM_INDOORS | ROOM_SAVE_OBJS);
	  iHash = i % MAX_KEY_HASH;
	  LinkSingle (pRoom, room_index_hash[iHash], next);

	  ch->in_room->exit[door] = new_exit ();
	  ch->in_room->exit[door]->u1.to_room = pRoom;
	  ch->in_room->exit[door]->orig_door = door;
	  rev = rev_dir[door];
	  pExit = new_exit ();
	  pExit->u1.to_room = ch->in_room;
	  pExit->orig_door = rev;
	  pRoom->exit[rev] = pExit;
	  break;
	}
    }

  if (!pRoom)
    {
      chprintln (ch,
		 "A new room could not be created for you. please contact an immortal.");
      return;
    }

  ch->pcdata->home[HAS_HOME (ch)] = pRoom->vnum;
  HAS_HOME (ch) += 1;

  deduct_cost (ch, ROOM_PRICE, VALUE_GOLD);
  ch->pcdata->quest.points -= 200;

  chprintln
    (ch, "The mayors hired contractors come and do additions to your home.");
  chprintlnf (ch, "Your home now contains %d rooms.", HAS_HOME (ch));
  save_char_obj (ch);
  SetBit (pArea->area_flags, AREA_CHANGED);
  return;
}

Do_Fun (home_sell)
{
  RoomIndex *pRoom = NULL;
  int home, i = 0, rooms = 0;
  money_t cost = 0;
  int quest = 0;
  int tp = 0;
  char buf[MSL];

  if (!ch || IsNPC (ch))
    return;

  if (!(rooms = HAS_HOME (ch)))
    {
      chprintln (ch, "You don't have a home yet.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch,
		 "This command allows you to sell your home.  The Mayors");
      chprintln (ch,
		 "contractors  will completly demolish your home when it is sold,");
      chprintln (ch,
		 "and you will be compensated half the cost of the home,");
      chprintln (ch, "including any items you have purchased.");
      chprintln (ch,
		 "{ROnce your home is sold, it cannot be brought back!!!{x");
      cmd_syntax (ch, NULL, n_fun, "confirm", NULL);
      return;
    }
  else if (!str_cmp (argument, "confirm"))
    {
      bool found = false;

      sprintf (buf, "%ld", HOME_KEY (ch));
      oedit_delete (NULL, NULL, buf);
      for (home = 0; home < PC_HOME_COUNT; home++)
	{
	  if (!found && ch->pcdata->home[home] == HOME_ROOM (ch))
	    found = true;
	  if ((pRoom = get_room_index (ch->pcdata->home[home])) != NULL)
	    {
	      SetBit (pRoom->area->area_flags, AREA_CHANGED);
	      i += tally_resets (pRoom, cost, tp, quest);
	      sprintf (buf, "%ld", ch->pcdata->home[home]);
	      if (redit_delete (NULL, NULL, buf))
		cost += ROOM_PRICE / 3;
	    }
	}
      if (!found && HOME_ROOM (ch)
	  && (pRoom = get_room_index (HOME_ROOM (ch))) != NULL)
	{
	  SetBit (pRoom->area->area_flags, AREA_CHANGED);
	  i += tally_resets (pRoom, cost, tp, quest);
	  sprintf (buf, "%ld", HOME_ROOM (ch));
	  if (redit_delete (NULL, NULL, buf))
	    cost += ROOM_PRICE / 3;
	}
      add_cost (ch, cost, VALUE_GOLD);
      ch->pcdata->trivia += tp;
      ch->pcdata->quest.points += quest;
      chprintlnf
	(ch,
	 "Your home {R(%d rooms) {G(%d furnishings){x has been sold and you are now {Y%ld{x gold coins richer!",
	 rooms, i, cost);
      memset (ch->pcdata->home, 0, MAX_HOME_VNUMS);
      save_char_obj (ch);
      return;
    }
  else
    {
      home_sell (n_fun, ch, "");
      return;
    }
}


Do_Fun (home_door)
{
  int door;
  char arg1[MIL], arg2[MIL];
  ExitData *pexit;

  if (!ch || IsNPC (ch))
    return;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You have to BUY a home before you can describe it.");
      return;
    }
  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "But you do not own this room!");
      return;
    }

  argument = first_arg (argument, arg1, false);
  argument = first_arg (argument, arg2, false);

  if (NullStr (arg1))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "<exit> <flag>          - flag an exit.",
		  "<exit> list            - list exit flags.",
		  "<exit> key <object>    - make a key for exit.",
		  "<exit> name <string>   - name an exit.",
		  "<exit> desc <string>   - describe an exit.", NULL);
      return;
    }

  if ((door = get_direction (arg1)) != -1
      && (pexit = ch->in_room->exit[door]) != NULL)
    {
      flag_t flag;

      if (!str_prefix (arg2, "list"))
	{
	  show_flags (ch, exit_flags);
	  return;
	}
      else if ((flag = flag_value (exit_flags, arg2)) != NO_FLAG)
	{
	  ExitData *toExit;

	  ToggleBit (pexit->rs_flags, flag);

	  if ((IsSet (pexit->rs_flags, EX_PICKPROOF)
	       || IsSet (pexit->rs_flags, EX_EASY)
	       || IsSet (pexit->rs_flags, EX_HARD)
	       || IsSet (pexit->rs_flags, EX_INFURIATING))
	      && !IsSet (pexit->rs_flags, EX_LOCKED))
	    {
	      ToggleBit (pexit->rs_flags, EX_LOCKED);
	    }

	  if ((IsSet (pexit->rs_flags, EX_LOCKED)
	       || IsSet (pexit->rs_flags, EX_NOPASS))
	      && !IsSet (pexit->rs_flags, EX_CLOSED))
	    {
	      ToggleBit (pexit->rs_flags, EX_CLOSED);
	    }
	  if (IsSet (pexit->rs_flags, EX_CLOSED)
	      && !IsSet (pexit->rs_flags, EX_ISDOOR))
	    {
	      ToggleBit (pexit->rs_flags, EX_ISDOOR);
	    }
	  pexit->exit_info = pexit->rs_flags;

	  if (pexit->u1.to_room
	      && (toExit = pexit->u1.to_room->exit[rev_dir[door]]))
	    {

	      toExit->rs_flags = pexit->rs_flags;

	      if ((IsSet (toExit->rs_flags, EX_PICKPROOF)
		   || IsSet (toExit->rs_flags, EX_EASY)
		   || IsSet (toExit->rs_flags, EX_HARD)
		   || IsSet (toExit->rs_flags, EX_INFURIATING))
		  && !IsSet (toExit->rs_flags, EX_LOCKED))
		{
		  ToggleBit (toExit->rs_flags, EX_LOCKED);
		}
	      if ((IsSet (toExit->rs_flags, EX_LOCKED)
		   || IsSet (toExit->rs_flags, EX_NOPASS))
		  && !IsSet (toExit->rs_flags, EX_CLOSED))
		{
		  ToggleBit (toExit->rs_flags, EX_CLOSED);
		}

	      if (IsSet (toExit->rs_flags, EX_CLOSED)
		  && !IsSet (toExit->rs_flags, EX_ISDOOR))
		{
		  ToggleBit (toExit->rs_flags, EX_ISDOOR);
		}
	      toExit->exit_info = toExit->rs_flags;
	    }
	  act ("Exit is now $t.", ch, arg2, pexit, TO_CHAR);
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  return;
	}
      else if (!str_prefix (arg2, "key"))
	{
	  ObjData *key;

	  if ((key = get_obj_here (ch, NULL, argument)) == NULL
	      || key->item_type != ITEM_KEY)
	    {
	      chprintln (ch, "No such key!");
	      return;
	    }

	  pexit->key = key->pIndexData->vnum;
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  act ("Exit now uses $p as a key.", ch, key, pexit->keyword,
	       TO_CHAR);
	  return;
	}
      else if (!str_prefix (arg2, "name"))
	{
	  replace_str (&pexit->keyword, argument);
	  if (NullStr (argument))
	    chprintln (ch, "Exit name removed.");
	  else
	    act ("Exit is now named $T.", ch, dir_name[door], argument,
		 TO_CHAR);
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  return;
	}
      else if (!str_prefix (arg2, "describe"))
	{
	  replace_str (&pexit->description, argument);
	  if (NullStr (argument))
	    chprintln (ch, "Exit description removed.");
	  else
	    act ("Exit is now described as $T.", ch, dir_name[door],
		 argument, TO_CHAR);
	  SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
	  return;
	}
    }
  home_door (n_fun, ch, "");
}

Do_Fun (home_describe)
{
  RoomIndex *loc;
  char buf[MSL];
  int len;
  bool found = false;

  loc = ch->in_room;

  if (!ch || IsNPC (ch))
    return;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You have to BUY a home before you can describe it.");
      return;
    }
  if (!is_home_owner (ch, loc))
    {
      chprintln (ch, "But you do not own this room!");
      return;
    }
  if (NullStr (argument))
    {
      chprintln (ch, "This command allows you to describe your home.");
      chprintln (ch, "You should not describe items that are in the room,");
      chprintln (ch,
		 "rather allowing the furnishing of the home to do that.");
      chprintln (ch,
		 "If you currently own this room, you will be placed into.");
      chprintln (ch,
		 "the room editor. Be warned that while in the room editor,");
      chprintln (ch,
		 "you are only allowed to type the description. If you are");
      chprintln (ch,
		 "unsure or hesitant about this, please note the Immortals,");
      chprintln (ch, "or better, discuss the how-to's with a Builder.");
      chprintlnf (ch,
		  "If Using the string editor bothers you type /q on a new line"
		  NEWLINE "and use the syntax: %s +/- <string>{x", n_fun);
      string_append (ch, &loc->description);
      SetBit (loc->area->area_flags, AREA_CHANGED);
      return;
    }
  else
    {
      buf[0] = '\0';

      if (argument[0] == '-')
	{
	  if (NullStr (loc->description))
	    {
	      chprintln (ch, "No lines left to remove.");
	      return;
	    }

	  strcpy (buf, loc->description);

	  for (len = strlen (buf); len > 0; len--)
	    {
	      if (buf[len] == '\r')
		{
		  if (!found)
		    {

		      if (len > 0)
			len--;
		      found = true;
		    }
		  else
		    {

		      buf[len + 1] = '\0';
		      replace_str (&loc->description, buf);
		      chprintln (ch, "OK.");
		      return;
		    }
		}
	    }
	  buf[0] = '\0';
	  replace_str (&loc->description, buf);
	  chprintln (ch, "Description cleared.");
	  return;
	}
      if (argument[0] == '+')
	{
	  if (loc->description != NULL)
	    strcat (buf, loc->description);
	  argument++;
	  while (isspace (*argument))
	    argument++;
	}

      if (strlen (buf) + strlen (argument) >= MSL - 2)
	{
	  chprintln (ch, "Description too long.");
	  return;
	}

      strcat (buf, argument);
      strcat (buf, NEWLINE);
      replace_str (&loc->description, buf);
      SetBit (loc->area->area_flags, AREA_CHANGED);
    }
  return;
}

Do_Fun (home_name)
{
  RoomIndex *loc;

  loc = ch->in_room;

  if (!ch || IsNPC (ch))
    return;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You have to BUY a home before you can rename it.");
      return;
    }
  if (!is_home_owner (ch, loc))
    {
      chprintln (ch, "But you do not own this room!");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Change the this room title to what?");
      return;
    }

  if (cstrlen (argument) > 25)
    {
      chprintln (ch, "This is the name, not the description.");
      return;
    }

  replace_str (&loc->name, argument);
  SetBit (loc->area->area_flags, AREA_CHANGED);
  return;
}

Lookup_Fun (furnish_lookup)
{
  int i;

  for (i = 0; home_table[i].name != NULL; i++)
    {
      if (is_name (name, home_table[i].name))
	return i;
    }
  return -1;
}

Do_Fun (home_furnish)
{
  ResetData *loc_reset, *pReset;
  ObjData *furn = NULL;
  ObjIndex *pObj = NULL;
  CharData *moby = NULL;
  CharIndex *pMob = NULL;
  int i;

  if (!ch || IsNPC (ch))
    return;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You have to BUY a home before you can furnish it.");
      return;
    }

  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "But you do not own this room!");
      return;
    }

  if (NullStr (argument) || !str_cmp (argument, "list"))
    {
      chprintln (ch, "This command allows you to furnish your home.");
      chprintln (ch,
		 "You must be standing in your home. You cannot have more");
      chprintln (ch, "than five items and five mobiles in your home.");
      chprintln (ch,
		 "Other things like heal/mana rate have a limit instead.");
      cmd_syntax (ch, NULL, n_fun, "<option>", NULL);
      for (i = 0; home_table[i].name != NULL; i++)
	{
	  switch (home_table[i].type)
	    {
	    default:
	    case VNUM_NONE:
	    case ROOM_VNUM:
	      break;
	    case MOB_VNUM:
	      pMob = get_char_index (home_table[i].vnum);
	      break;
	    case OBJ_VNUM:
	      pObj = get_obj_index (home_table[i].vnum);
	      break;
	    }
	  chprintlnf (ch, "\t%-15s - %ld gold, %dqp, %dtp (%s)",
		      pMob ? pMob->short_descr : pObj ? pObj->
		      short_descr : home_table[i].list, home_table[i].cost,
		      home_table[i].quest, home_table[i].tp,
		      home_table[i].name);
	  pMob = NULL;
	  pObj = NULL;
	}
      return;
    }
  else
    {
      if ((i = furnish_lookup (argument)) == -1)
	{
	  chprintln (ch, "Furnish what?");
	}

      if (!check_worth (ch, home_table[i].cost, VALUE_GOLD))
	{
	  chprintlnf
	    (ch, "You don't have enough gold for to buy a %s.",
	     home_table[i].name);
	  return;
	}
      if (ch->pcdata->quest.points < home_table[i].quest)
	{
	  chprintlnf (ch, "You need %s to buy a %s.",
		      intstr (home_table[i].quest, "questpoint"),
		      home_table[i].name);
	  return;
	}
      if (ch->pcdata->trivia < home_table[i].tp)
	{
	  chprintlnf (ch, "You need %s to buy a %s.",
		      intstr (home_table[i].quest, "trivia point"),
		      home_table[i].name);
	  return;
	}
      if (home_table[i].type == VNUM_NONE)
	{
	  if (!str_cmp (home_table[i].name, "healrate"))
	    {
	      if (ch->in_room->heal_rate >= 200)
		{
		  chprintln (ch, "This rooms heal rate can't go any higher.");
		  return;
		}
	      ch->in_room->heal_rate = Min (ch->in_room->heal_rate + 50, 200);
	    }
	  if (!str_cmp (home_table[i].name, "manarate"))
	    {
	      if (ch->in_room->mana_rate >= 200)
		{
		  chprintln (ch, "This rooms mana rate can't go any higher.");
		  return;
		}
	      ch->in_room->mana_rate = Min (ch->in_room->mana_rate + 50, 200);
	    }
	}
      else if (home_table[i].type == OBJ_VNUM)
	{
	  for (furn = ch->in_room->content_first; furn;
	       furn = furn->next_content)
	    {
	      if (furn->pIndexData->vnum == home_table[i].vnum)
		{
		  chprintln
		    (ch, "You already have one of those in this room.");
		  return;
		}
	    }
	  if ((pObj = get_obj_index (home_table[i].vnum)) != NULL)
	    {
	      furn = create_object (pObj, pObj->level);
	      obj_to_room (furn, ch->in_room);
	    }
	  else
	    chprintln
	      (ch, "Cannot find that object, please contact an immortal.");
	}
      else if (home_table[i].type == MOB_VNUM)
	{
	  for (pReset = ch->in_room->reset_first; pReset != NULL;
	       pReset = pReset->next)
	    {
	      if (pReset->command == 'M'
		  && pReset->arg1 == home_table[i].vnum)
		{
		  chprintln
		    (ch, "You already have one of those in this room.");
		  return;
		}
	    }

	  if ((pMob = get_char_index (home_table[i].vnum)) != NULL)
	    {
	      moby = create_mobile (pMob);
	      loc_reset = new_reset ();
	      loc_reset->command = 'M';
	      loc_reset->arg1 = pMob->vnum;
	      loc_reset->arg2 =
		ch->in_room->area->max_vnum - ch->in_room->area->min_vnum;
	      loc_reset->arg3 = ch->in_room->vnum;
	      loc_reset->arg4 = 1;
	      add_reset (ch->in_room, loc_reset, 0);
	      char_to_room (moby, ch->in_room);
	    }
	  else
	    chprintln
	      (ch, "Cannot find that mobile, please contact an immortal.");
	}
      else
	{
	  chprintln
	    (ch,
	     "Unable to complete your request.  Please inform an Immortal.");
	  return;
	}

      if (home_table[i].room_flags > 0)
	SetBit (ch->in_room->room_flags, home_table[i].room_flags);

      deduct_cost (ch, home_table[i].cost, VALUE_GOLD);

      ch->pcdata->quest.points -= home_table[i].quest;
      ch->pcdata->trivia -= home_table[i].tp;

      chprintlnf
	(ch,
	 "You have been deducted %ld gold, %dqp and %dtp for your purchase.",
	 home_table[i].cost, home_table[i].quest, home_table[i].tp);
      SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
      return;
    }
}

Do_Fun (home_unfurnish)
{
  ResetData *pReset = NULL, *pResNext;
  ObjData *obj, *obj_next;
  CharData *mob, *mob_next;
  int i = 0;
  bool found = false;

  if (!ch || IsNPC (ch))
    return;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You have to BUY a home before you can furnish it.");
      return;
    }

  if (!ch->in_room || !is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "But you do not own this room!");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Unfurnish which object?.");
      return;
    }
  else
    {
      for (i = 0; home_table[i].name != NULL; i++)
	{
	  if (home_table[i].type == VNUM_NONE || home_table[i].vnum <= 0)
	    continue;

	  if (home_table[i].type == MOB_VNUM)
	    {
	      for (pReset = ch->in_room->reset_first; pReset;
		   pReset = pResNext)
		{
		  pResNext = pReset->next;

		  if (pReset->command == 'M'
		      && pReset->arg1 == home_table[i].vnum)
		    {
		      unlink_reset (ch->in_room, pReset);
		      free_reset (pReset);
		      if (home_table[i].room_flags > 0)
			RemBit (ch->in_room->room_flags,
				home_table[i].room_flags);
		      SetBit (ch->in_room->area->area_flags, AREA_CHANGED);
		      found = true;
		      for (mob = ch->in_room->person_first; mob != NULL;
			   mob = mob_next)
			{
			  mob_next = mob->next_in_room;

			  if (IsNPC (mob)
			      && mob->pIndexData->vnum == home_table[i].vnum)
			    extract_char (mob, true);
			}
		    }
		}
	    }
	  else if (home_table[i].type == OBJ_VNUM)
	    {
	      for (obj = ch->in_room->content_first; obj != NULL;
		   obj = obj_next)
		{
		  obj_next = obj->next_content;

		  if (obj->pIndexData->vnum == home_table[i].vnum)
		    {
		      extract_obj (obj);
		      found = true;
		      if (home_table[i].room_flags > 0)
			RemBit (ch->in_room->room_flags,
				home_table[i].room_flags);
		    }
		}
	    }

	  add_cost (ch, home_table[i].cost / 3, VALUE_GOLD);
	  ch->pcdata->quest.points += home_table[i].quest / 3;
	  ch->pcdata->trivia += home_table[i].tp / 3;
	  chprintlnf
	    (ch,
	     "You sell %s back to the mayor for %ld gold, %dqp and %dtp.",
	     home_table[i].list, home_table[i].cost / 3,
	     home_table[i].quest / 3, home_table[i].tp / 3);
	}
      if (!found)
	chprintln (ch, "Unable to find your request.");
      return;
    }
}

Do_Fun (do_gohome)
{
  RoomIndex *location = NULL;
  int i;

  if (!ch)
    return;

  if (IsNPC (ch))
    {
      chprintln (ch, "Only players can go home.");
      return;
    }

  if (!HOME_ROOM (ch) || (location = get_room_index (HOME_ROOM (ch))) == NULL)
    {
      for (i = 0; i < HAS_HOME (ch); i++)
	if ((location = get_room_index (ch->pcdata->home[i])) != NULL)
	  break;

      if (location == NULL)
	{
	  chprintln (ch, "You don't have a home *pout*.");
	  return;
	}
    }

  perform_recall (ch, location, "go home");
  return;
}

Do_Fun (home_evict)
{
  CharData *victim;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You don't have a home to kick them out of.");
      return;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "evict <char>", NULL);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "They aren't here.");
      return;
    }

  if (IsNPC (victim))
    {
      chprintln (ch, "No such player.");
      return;
    }

  if (victim == ch)
    {
      chprintln (ch, "What would be the point of that?");
      return;
    }

  if (ch->fighting != NULL)
    {
      chprintln (ch, "Your a little busy right now.");
      return;
    }

  if (!is_home_owner (ch, victim->in_room))
    {
      chprintln (ch, "They aren't in your home.");
      return;
    }

  if (victim->fighting != NULL)
    stop_fighting (victim, true);

  char_from_room (victim);
  char_to_room (victim, get_room_index (ROOM_VNUM_TEMPLE));
  act ("$n has someone escort you out of $s home.", ch, NULL, victim,
       TO_VICT);
  do_function (victim, &do_look, "auto");
  act ("You are no longer welcome in $n's home.", ch, NULL, victim, TO_VICT);
  act ("$N is no longer welcome in your home.", ch, NULL, victim, TO_CHAR);
  return;
}

Do_Fun (home_link)
{
  int door;
  AreaData *pArea;
  RoomIndex *pRoom, *toRoom = NULL;
  char arg[MIL];
  vnum_t vnum;

  if (!HAS_HOME (ch))
    {
      chprintlnf (ch, "Use '%s buy' to purchase a home first.",
		  cmd_name (do_home));
      return;
    }

  if (!check_worth (ch, ROOM_PRICE, VALUE_GOLD))
    {
      chprintln (ch, "{wYou don't have enough to buy a another room.");
      return;
    }

  if (ch->pcdata->quest.points < 125)
    {
      chprintln (ch, "It costs 125 quest points to add a link to this room.");
      return;
    }

  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "You must add only to your home!");
      return;
    }

  argument = one_argument (argument, arg);

  if ((door = get_direction (arg)) == -1)
    {
      chprintln (ch, "Which direction from here do you want the new exit?");
      return;
    }

  pRoom = ch->in_room;

  if (pRoom->exit[door])
    {
      chprintln (ch, "An exit already exits in that location.");
      return;
    }

  if (NullStr (argument))
    {
      chprintln (ch, "Invalid area to link to.");
      return;
    }

  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
      if (!str_prefix (argument, pArea->name))
	break;
    }

  if (pArea == NULL)
    {
      chprintln (ch, "Invalid area to link to.");
      return;
    }

  for (vnum = pArea->min_vnum; vnum < pArea->max_vnum; vnum++)
    {
      if ((toRoom = get_room_index (vnum)) != NULL)
	break;
    }

  if (toRoom == NULL)
    {
      chprintln (ch, "Unable to link to that area.");
      return;
    }

  pRoom->exit[door] = new_exit ();

  pRoom->exit[door]->u1.to_room = toRoom;

  pRoom->exit[door]->orig_door = door;

  SetBit (pRoom->area->area_flags, AREA_CHANGED);
  deduct_cost (ch, ROOM_PRICE, VALUE_GOLD);
  ch->pcdata->quest.points -= 125;
  chprintlnf (ch, "Exit to %s added.", pArea->name);
}

Do_Fun (home_unlink)
{
  char arg[MIL];

  RoomIndex *pRoom, *pToRoom;
  int rev, door;

  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You don't own a home.");
      return;
    }

  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "You don't own this room!");
      return;
    }

  argument = one_argument (argument, arg);

  if ((door = get_direction (arg)) == -1)
    {
      chprintln (ch,
		 "Which direction from here do you want to delete the exit?");
      return;
    }

  pRoom = ch->in_room;

  if (!pRoom->exit[door])
    {
      chprintln (ch, "There is no exit in that direction.");
      return;
    }

  rev = rev_dir[door];
  pToRoom = pRoom->exit[door]->u1.to_room;

  if (pToRoom->area == pRoom->area)
    {
      chprintln (ch, "You can't unlink that exit.");
      return;
    }
  free_exit (pRoom->exit[door]);
  pRoom->exit[door] = NULL;

  add_cost (ch, ROOM_PRICE / 3, VALUE_GOLD);
  ch->pcdata->quest.points += 125 / 3;
  SetBit (pRoom->area->area_flags, AREA_CHANGED);
  chprintln (ch, "Exit unlinked.");
  return;
}

Do_Fun (home_invite)
{
  CharData *vch;

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun, "invite <person>", NULL);
      return;
    }

  if ((vch = get_char_world (ch, argument)) == NULL)
    {
      chprintln (ch, "They aren't online.");
      return;
    }

  if (IsNPC (vch))
    {
      chprintln (ch, "You can't invite them.");
      return;
    }

  if (vch == ch)
    {
      chprintln (ch, "You can't invite yourself.");
      return;
    }
  if (vch->pcdata->home_invite != 0)
    {
      chprintln (ch, "They have already been invited somewhere.");
      return;
    }
  if (vch->fighting != NULL)
    {
      chprintln (ch, "They're a little busy right now.");
      return;
    }

  vch->pcdata->home_invite = ch->id;

  act
    ("$n has invited you to $s home. Type '$t accept' to accept the invitation"
     NEWLINE
     "and be transfered to $s home, or type '$t deny' to cancel the invitation.",
     ch, cmd_name (do_home), vch, TO_VICT);

  act ("Invitation sent to $N.", ch, NULL, vch, TO_CHAR);
}

Do_Fun (home_accept)
{
  CharData *owner;
  RoomIndex *pRoom;
  int i;

  if (ch->pcdata->home_invite == 0)
    {
      chprintln (ch, "You haven't been invited anywhere.");
      return;
    }

  if (ch->fighting != NULL || InWar (ch))
    {
      chprintln (ch, "You're a little busy right now.");
      return;
    }

  if ((owner = get_char_vnum (ch->pcdata->home_invite)) == NULL)
    {
      chprintln (ch,
		 "Hmm, seems the person who invited you isn't around anymore.");
      return;
    }

  pRoom = NULL;

  if ((pRoom = get_room_index (HOME_ROOM (owner))) == NULL)
    {
      for (i = 0; i < HAS_HOME (ch); i++)
	if ((pRoom = get_room_index (owner->pcdata->home[i])) != NULL)
	  break;
    }

  if (!pRoom)
    {
      chprintln (ch, "BUG: inviter doesn't own a home!");
      return;
    }

  char_from_room (ch);
  char_to_room (ch, pRoom);
  act ("$n arrives.", ch, NULL, NULL, TO_ROOM);
  do_function (ch, &do_look, "auto");
  ch->pcdata->home_invite = 0;
  return;
}

Do_Fun (home_deny)
{
  CharData *owner;

  if (ch->pcdata->home_invite == 0)
    {
      chprintln (ch, "You haven't been invited anywhere.");
      return;
    }

  owner = get_char_vnum (ch->pcdata->home_invite);

  if (owner)
    {
      act ("You turn down $N's invitation.", ch, NULL, owner, TO_CHAR);
      act ("$n turns down your invitation.", ch, NULL, owner, TO_VICT);
    }
  else
    chprintln (ch, "You turn down the invitation.");

  ch->pcdata->home_invite = 0;
}


Do_Fun (home_recall)
{
  if (!HAS_HOME (ch))
    {
      chprintln (ch, "You don't own a home.");
      return;
    }

  if (!is_home_owner (ch, ch->in_room))
    {
      chprintln (ch, "This isn't your home!");
      return;
    }

  ch->pcdata->home_room = ch->in_room->vnum;
  chprintlnf (ch, "You can use '%s' to come back here.",
	      cmd_name (do_gohome));
}

void
delete_home (CharData * ch)
{
  int i;
  char buf[MSL];

  if (HAS_HOME (ch))
    {
      RoomIndex *pRoom;
      AreaData *pArea = NULL;

      for (i = 0; i < PC_HOME_COUNT; i++)
	{
	  if ((pRoom = get_room_index (ch->pcdata->home[i])) != NULL)
	    {
	      pArea = pRoom->area;
	      sprintf (buf, "%ld", ch->pcdata->home[i]);
	      redit_delete (NULL, NULL, buf);
	    }
	}
      sprintf (buf, "%ld", HOME_KEY (ch));
      oedit_delete (NULL, NULL, buf);
      if (pArea)
	SetBit (pArea->area_flags, AREA_CHANGED);
    }
}


void
write_room_objs (RoomIndex * pRoom, FileData * fp)
{
  if (!pRoom || !pRoom->content_last || !fp)
    return;

  write_obj (NULL, pRoom->content_last, fp, 0, 0, SAVE_ROOM);
}

void
save_room_objs (void)
{
  FileData *fp;
  RoomIndex *pRoom;
  int iHash;

  if ((fp = f_open (ROOM_OBJS_FILE, "w")) != NULL)
    {
      for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
	  for (pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next)
	    {
	      if (!IsSet (pRoom->room_flags, ROOM_SAVE_OBJS)
		  && (!IsSet (pRoom->area->area_flags, AREA_PLAYER_HOMES)
		      || NullStr (pRoom->owner)))
		continue;

	      write_room_objs (pRoom, fp);
	    }
	}
      f_printf (fp, "#" END_MARK LF);
      f_close (fp);
    }
}

void
load_room_objs (void)
{
  FileData *fp;

  log_string ("Loading saved objects...");

  if ((fp = f_open (ROOM_OBJS_FILE, "r")) == NULL)
    {
      bug (ROOM_OBJS_FILE " not found");
    }
  else
    {
      for (;;)
	{
	  char letter;
	  char *word;

	  letter = read_letter (fp);
	  if (letter == '*')
	    {
	      read_to_eol (fp);
	      continue;
	    }

	  if (letter != '#')
	    {
	      bug ("# not found.");
	      break;
	    }

	  word = read_word (fp);
	  if (!str_cmp (word, get_obj_save_header (SAVE_ROOM)))
	    read_obj (NULL, fp, SAVE_ROOM);
	  else if (!str_cmp (word, "END"))
	    break;
	  else
	    {
	      bug ("bad section.");
	      break;
	    }
	}
      f_close (fp);
    }

  return;
}
