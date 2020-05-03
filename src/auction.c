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
#include "olc.h"
#include "magic.h"
#include "recycle.h"

AuctionData *
auction_lookup (int num)
{
  AuctionData *pAuc;

  for (pAuc = auction_first; pAuc != NULL; pAuc = pAuc->next)
    {
      if (pAuc->number == num)
	return pAuc;
    }
  return NULL;
}

int last_auc_id;

int
get_auc_id (void)
{
  last_auc_id++;

  if (last_auc_id > 999)
    last_auc_id = 1;

  return last_auc_id;
}

const char *
auc_type (auc_t type, bool fShort)
{
  switch (type)
    {
    default:
    case AUC_GOLD:
      return fShort ? "g" : " gold";
    case AUC_QUEST:
      return fShort ? "qp" : " quest points";
    case AUC_TRIVIA:
      return fShort ? "tp" : " trivia points";
    }
}

const char *
auc_color (auc_t type)
{
  switch (type)
    {
    default:
    case AUC_GOLD:
      return "{Y";
    case AUC_QUEST:
      return "{G";
    case AUC_TRIVIA:
      return "{R";
    }
}

const char *
auc_price (AuctionData * auc, bool fShort)
{
  return FORMATF ("%s%ld%s{x", auc_color (auc->type), auc->bid,
		  auc_type (auc->type, fShort));
}

bool
check_bid (CharData * ch, money_t bid, auc_t type)
{
  if (!ch)
    return true;

  switch (type)
    {
    default:
    case AUC_GOLD:
      return check_worth (ch, bid, VALUE_DEFAULT);
    case AUC_QUEST:
      return bid < (unsigned int) ch->pcdata->quest.points;
    case AUC_TRIVIA:
      return bid < (unsigned int) ch->pcdata->trivia;
    }
}

void
pay_bid (AuctionData * auc)
{
  CharData *bidder = auc->high_bidder;
  CharData *seller = auc->owner;

  switch (auc->type)
    {
    default:
    case AUC_GOLD:
      add_cost (seller, auc->bid * 9 / 10, VALUE_DEFAULT);
      deduct_cost (bidder, auc->bid, VALUE_DEFAULT);
      break;

    case AUC_QUEST:
      seller->pcdata->quest.points += auc->bid;
      bidder->pcdata->quest.points -= auc->bid;
      break;
    case AUC_TRIVIA:
      seller->pcdata->trivia += auc->bid;
      bidder->pcdata->trivia -= auc->bid;
      break;
    }

  if (auc->item->in_room)
    obj_from_room (auc->item);
  else if (auc->item->in_obj)
    obj_from_obj (auc->item);
  else
    obj_from_char (auc->item);

  obj_to_char (auc->item, bidder);

  announce (bidder, INFO_AUCTION | INFO_PRIVATE,
	    "You are sold %s for %s.", auc->item->short_descr,
	    auc_price (auc, false));

  if (auc->type == AUC_GOLD)
    auc->bid = (auc->bid * 9) / 10;

  chprintlnf (seller, "You receive %s for the sale of %s.",
	      auc_price (auc, false), auc->item->short_descr);
}

money_t
bid_incr (auc_t type)
{
  switch (type)
    {
    default:
      return 10;
    case AUC_TRIVIA:
      return 1;
    }
}

Do_Fun (do_auction)
{
  AuctionData *auc;
  ObjData *obj = NULL;
  money_t minbid = 1;
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  auc_t type;

  argument = first_arg (argument, arg1, false);
  argument = first_arg (argument, arg2, false);
  first_arg (argument, arg3, false);

  if (ch == NULL)
    return;

  if (NullStr (arg1))
    {
      if (IsSet (ch->info_settings, INFO_AUCTION))
	{
	  RemBit (ch->info_settings, INFO_AUCTION);
	  chprintln (ch, "{YAUCTION{x channel is now ON.");
	}
      else
	{
	  SetBit (ch->info_settings, INFO_AUCTION);
	  chprintln (ch, "{YAUCTION{x channel is now OFF.");
	}
      return;
    }
  else if (!str_cmp (arg1, "stop") && IsImmortal (ch))
    {

      if (NullStr (arg2) || !is_number (arg2))
	{
	  chprintln (ch, "Stop which auction?");
	  return;
	}
      if ((auc = auction_lookup (atoi (arg2))) == NULL)
	{
	  chprintln (ch, "No such auction.");
	  return;
	}
      if (auc->item)
	{
	  announce (ch, INFO_AUCTION,
		    "$n has stopped the auction and confiscated %s!",
		    auc->item->short_descr);
	  announce (ch, INFO_AUCTION | INFO_PRIVATE,
		    "You have stopped the auction and confiscated %s!",
		    auc->item->short_descr);
	  obj_from_char (auc->item);
	  obj_to_char (auc->item, ch);
	}
      reset_auc (auc, true);
      return;
    }

  if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
    {
      chprintln (ch, "You aren't carrying that item.");
      return;
    }

  if (is_name (arg2, "gold coin"))
    type = AUC_GOLD;
  else if (is_name (arg2, "qp questpoints"))
    type = AUC_QUEST;
  else if (is_name (arg2, "tp triviapoints"))
    type = AUC_TRIVIA;
  else if (IsObjStat (obj, ITEM_QUEST))
    type = AUC_QUEST;
  else
    {
      chprintln
	(ch,
	 "What kind of currency do you want to use? (gold, quest points, or trivia points)");
      return;
    }

  if (IsObjStat (obj, ITEM_AUCTIONED))
    {
      chprintln (ch, "That items is already being auctioned.");
      return;
    }

  if (IsObjStat (obj, ITEM_NODROP) && !IsObjStat (obj, ITEM_QUEST))
    {
      chprintln (ch, "You can't let go of that item.");
      return;
    }

  if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
    {
      chprintln (ch, "Not a good idea....");
      return;
    }

  if (count_auc (ch) >= 3)
    {
      chprintln (ch, "You can only auction up to 3 items at a time!");
      return;
    }

  if (!NullStr (arg3))
    minbid = atol (arg3);
  else if (IsObjStat (obj, ITEM_QUEST))
    minbid = obj_cost (obj) / 3;

  if (minbid > 1000000 || minbid < 1)
    {
      chprintln (ch,
		 "Minumum bids can't be higher than 1000000 or less than 1.");
      return;
    }

  auc = new_auction ();
  Link (auc, auction, next, prev);
  SetBit (obj->extra_flags, ITEM_AUCTIONED);
  auc->owner = ch;
  auc->item = obj;
  auc->type = type;
  auc->bid = minbid;
  auc->number = get_auc_id ();
  auc->status = AUCTION_LENGTH;

  announce (auc->owner, INFO_AUCTION,
	    "$n is auctioning %s (Level %d, Num %d). Current bid is %s.",
	    auc->item->short_descr, auc->item->level, auc->number,
	    auc_price (auc, false));
  announce (auc->owner, INFO_AUCTION | INFO_PRIVATE,
	    "You are auctioning %s (Level %d, Num %d). Current bid is %ld%s.",
	    auc->item->short_descr, auc->item->level, auc->number,
	    auc_price (auc, false));
  mud_info.stats.auctions++;
  return;
}

void
auction_update (void)
{
  AuctionData *auc, *auc_next;

  if (!auction_first)
    return;

  for (auc = auction_first; auc != NULL; auc = auc_next)
    {
      auc_next = auc->next;

      if (auc->status == 0)
	continue;

      auc->status--;

      if (auc->status < 0)
	auc->status = 0;

      if (!auc->item)
	{
	  reset_auc (auc, true);
	  bugf ("Auction with no item, reseting.");
	  continue;
	}
      else if (auc->status % PULSE_PER_SECOND == 0)
	{
	  switch (auc->status / PULSE_PER_SECOND)
	    {
	    case 0:
	      if (auc->high_bidder == NULL)
		{
		  announce (NULL, INFO_AUCTION,
			    "No bids recieved on %s, sale stopped.",
			    auc->item->short_descr);
		  reset_auc (auc, true);
		}
	      else if (!check_bid (auc->high_bidder, auc->bid, auc->type))
		{
		  announce (auc->high_bidder, INFO_AUCTION,
			    "$n can't cover their stake in the auction, sale stopped.");
		  announce (auc->high_bidder, INFO_AUCTION | INFO_PRIVATE,
			    "You can't cover your stake in the auction, sale stopped.");

		  reset_auc (auc, true);
		}
	      else
		{
		  announce (auc->high_bidder, INFO_AUCTION,
			    "%s SOLD to $n for %s.",
			    auc->item->short_descr, auc_price (auc, false));
		  reset_auc (auc, false);
		  mud_info.stats.aucsold++;
		}
	      break;
	    case 32:
	      announce (NULL, INFO_AUCTION,
			"Going once %s (Level %d, Num %d). Current bid is %s.",
			auc->item->short_descr, auc->item->level,
			auc->number, auc_price (auc, false));
	      break;
	    case 15:
	      announce (NULL, INFO_AUCTION,
			"Going twice %s (Level %d, Num %d). Current bid is %s.",
			auc->item->short_descr, auc->item->level,
			auc->number, auc_price (auc, false));
	      break;
	    }
	}
    }
  return;
}

void
reset_auc (AuctionData * auc, bool forced)
{
  if (auc->item != NULL)
    {
      if (IsObjStat (auc->item, ITEM_AUCTIONED))
	RemBit (auc->item->extra_flags, ITEM_AUCTIONED);
      else
	bug ("item not flagged auction item");

      if (!forced && auc->high_bidder != NULL && auc->bid > 0)
	pay_bid (auc);

      else if (auc->owner != NULL)
	chprintlnf (auc->owner, "Sale of %s has been stopped.",
		    auc->item->short_descr);

    }
  UnLink (auc, auction, next, prev);
  free_auction (auc);
  return;
}

int
count_auc (CharData * ch)
{
  AuctionData *q;
  int count;

  q = auction_first;

  if (!q)
    return 0;

  for (count = 0; q; q = q->next)
    {
      if (q->owner == ch)
	count++;
    }

  return count;
}

money_t
advatoi (const char *s)
{
  const char *sptr;
  money_t number;
  money_t multiplier;

  for (sptr = s, number = 0; *sptr != NUL && isdigit (*sptr); sptr++)
    number = (number * 10) + (*sptr - '0');

  switch (toupper (*sptr))
    {
    case 'K':
      multiplier = 1000;
      number *= multiplier;
      sptr++;
      break;
    case 'M':
      multiplier = 1000 * 1000;
      number *= multiplier;
      sptr++;
      break;
    case '\0':
      multiplier = 0;
      break;
    default:
      return 0;

    }

  while (*sptr != NUL && isdigit (*sptr) && (multiplier > 1))
    {
      multiplier = multiplier / 10;
      number = number + ((*sptr - '0') * multiplier);
      sptr++;
    }

  if (*sptr != '\0' && !isdigit (*sptr))
    return 0;

  return number;
}

money_t
parsebet (const money_t currentbet, const char *argument)
{
  money_t newbet = 0;
  const char *strptr = argument;

  if (*strptr)
    {
      if (isdigit (*strptr))
	newbet = advatoi (strptr);
      else if (*strptr == '+')
	{
	  if (strlen (strptr) == 1)
	    newbet = (currentbet * 125) / 100;
	  else
	    newbet = (currentbet * (100 + atoi (++strptr))) / 100;
	}
      else
	{
	  if ((*strptr == '*') || (*strptr == 'x'))
	    {
	      if (strlen (strptr) == 1)
		newbet = currentbet * 2;
	      else
		newbet = currentbet * atoi (++strptr);
	    }
	}
    }

  return newbet;
}

Do_Fun (do_bid)
{
  char arg1[MIL];
  char arg2[MIL];
  AuctionData *auc, *auc_next;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (auction_first == NULL)
    {
      chprintln (ch, "There's nothing up for auction right now.");
      return;
    }

  if (NullStr (arg1))
    {
      chprintln (ch, stringf (ch, 0, Center, NULL,
			      "{Y[ Auction - Current List of Inventory ]{x"));
      chprintln
	(ch,
	 "{GNum   Seller       Buyer        Item Description                   Lvl   Last Bid    Time{x");
      chprintln (ch,
		 "{W---  ------------ ------------ ----------------------------------- --- ------------- ------------{x");
      for (auc = auction_first; auc; auc = auc_next)
	{
	  auc_next = auc->next;

	  if (!auc->item)
	    {
	      reset_auc (auc, true);
	      continue;
	    }

	  chprintlnf (ch, "{R%3d{x - %-12.12s %-12.12s" MXPTAG ("Bid %d")
		      "%35.35s" MXPTAG ("/Bid") " %3d" " {R%13s{G %s{x",
		      auc->number, GetName (auc->owner),
		      auc->high_bidder ? GetName (auc->high_bidder) : "None",
		      auc->number, stringf (ch, 35, Left, NULL,
					    auc->item->short_descr),
		      auc->item->level, auc_price (auc, true),
		      format_pulse (auc->status));
	}
      chprintln
	(ch,
	 "{W---  ------------ ------------ ----------------------------------- --- ------------- ------------{x");
      chprintlnf (ch,
		  "Type: '%s <num>' to see stats and '%s <num> <amount>' to bid on an item.",
		  n_fun, n_fun);
      return;
    }
  else if ((auc = auction_lookup (atoi (arg1))) != NULL)
    {
      if (!auc->item)
	{
	  reset_auc (auc, true);
	  chprintln (ch, "No such item.");
	  return;
	}

      if (NullStr (arg2))
	{
	  if (ch == auc->owner)
	    chprintlnf (ch, "You're auctioning %s.", auc->item->short_descr);
	  spell_identify (0, ch->level, ch, auc->item, TARGET_OBJ);
	  chprintf (ch, "Current bid is %s", auc_price (auc, false));
	  if (auc->high_bidder)
	    chprintlnf (ch, ", placed by %s.", Pers (auc->high_bidder, ch));
	  else
	    chprintln (ch, ".");
	  return;
	}
      else
	{
	  money_t bid = 0;

	  if (ch == auc->high_bidder)
	    {
	      chprintln (ch, "You already have the highest bid!");
	      return;
	    }

	  if (ch == auc->owner)
	    {
	      chprintln (ch, "You cannot bid on your own items!");
	      return;
	    }

	  bid = parsebet (auction_first->bid, arg2);

	  if (bid < 0 || bid > 200000000)
	    {
	      chprintln (ch, "Invalid bid.");
	      return;
	    }

	  if (!check_bid (ch, bid, auc->type))
	    {
	      chprintln (ch, "You can't cover that bid.");
	      return;
	    }

	  if (bid < auc->bid)
	    {
	      chprintlnf (ch, "The minimum bid is %s.",
			  intstr (auc->bid, auc_type (auc->type, false)));
	      return;
	    }

	  if (bid < (auc->bid + bid_incr (auc->type)))
	    {
	      chprintlnf (ch, "You must outbid %s by at least %ld.",
			  intstr (auc->bid, auc_type (auc->type, false)),
			  bid_incr (auc->type));
	      return;
	    }
	  announce (ch, INFO_AUCTION, "$n has offered %s for %s.",
		    auc_price (auc, false), auc->item->short_descr);
	  announce (ch, INFO_AUCTION | INFO_PRIVATE,
		    "You place a %s bid on %s.", auc_price (auc, false),
		    auc->item->short_descr);
	  auc->high_bidder = ch;
	  auc->bid = bid;
	  auc->status = AUCTION_LENGTH;
	  return;
	}
    }
  else
    chprintln
      (ch, "Bid on what object? (type 'bid', nothing else for a list)");
}

bool
has_auction (CharData * ch)
{
  AuctionData *auc;

  for (auc = auction_first; auc != NULL; auc = auc->next)
    {
      if (auc->owner == ch || auc->high_bidder == ch)
	return true;
    }

  return false;
}

void
extract_auc (CharData * ch)
{
  AuctionData *auc, *auc_next;

  for (auc = auction_first; auc != NULL; auc = auc_next)
    {
      auc_next = auc->next;

      if (auc->owner == ch)
	{
	  reset_auc (auc, true);
	  continue;
	}

      if (auc->high_bidder == ch)
	{
	  reset_auc (auc, true);
	  continue;
	}
    }
}
