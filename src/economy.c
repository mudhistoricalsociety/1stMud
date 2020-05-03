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
#include "data_table.h"

#define MAX_SHARES  10000

Do_Fun (do_balance)
{
  chprintlnf (ch, "Balance : %ld gold" NEWLINE
	      "On Hand : %ld gold, %ld silver" NEWLINE
	      "Shares  : %d gold in %d shares", ch->pcdata->gold_bank,
	      ch->gold, ch->silver,
	      (ch->pcdata->shares * mud_info.share_value),
	      ch->pcdata->shares);
}

Do_Fun (do_bank)
{
  char arg1[MIL];
  char arg2[MIL];
  money_t amount;
  ClanData *clan;
  bool QP = false;

  if (!ch)
    return;

  if (IsNPC (ch))
    {
      chprintln (ch, "Banking Services are only available to players!");
      return;
    }

  if (!IsSet (ch->in_room->room_flags, ROOM_BANK))
    {
      chprintln (ch, "You can't do that here.");
      return;
    }

  clan = ch->in_room->area->clan;

  if (!clan && (time_info.hour < 4 || time_info.hour > 20))
    {
      chprintln (ch, "The bank is closed, it is open from 4am to 8pm.");
      return;
    }

  if (NullStr (argument))
    {
      cmd_syntax (ch, NULL, n_fun,
		  "balance  : Displays your balance.",
		  "deposit  : Deposit gold into your account.",
		  "withdraw : Withdraw gold from your account.", NULL);
      if (!clan)
	{
	  cmd_syntax (ch, NULL, n_fun,
		      "transfer : Transfer gold to someones account.",
		      "buy #    : Buy # shares",
		      "sell #   : Sell # shares",
		      "check    : Check the current rates of the shares.",
		      NULL);
	}
      return;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

#define MAX_QP  32000
#define MAX_GOLD 99999999

#define MAX_MONEY    	  (!QP ? (money_t) MAX_GOLD : (money_t) MAX_QP)

#define getbalance (!clan ? ch->pcdata->gold_bank : QP ? (money_t)clan->questpoints : clan->gold)

#define paywith     ((clan  && QP) ? (ch->pcdata->quest.points >= (int)amount) \
    : check_worth(ch, amount, VALUE_DEFAULT))

#define getpay     (!clan ? ch->gold : QP ? ch->pcdata->quest.points \
    : ch->gold)

#define paybank do { if(!clan) ch->gold = Max(0, ch->gold - amount); \
    else if(QP) ch->pcdata->quest.points = Max(0, ch->pcdata->quest.points - (int)amount); \
    else deduct_cost(ch, amount, VALUE_DEFAULT); } while(0)

#define paychar do { if(!QP) ch->gold = Min(MAX_GOLD, ch->gold + amount); \
    else ch->pcdata->quest.points = Min(MAX_QP, ch->pcdata->quest.points + (int)amount); } while(0)

#define paytype     (!QP ? "gold" : "quest points")

  if (!str_prefix (arg1, "balance"))
    {
      if (!clan)
	do_balance (n_fun, ch, "");
      else
	chprintlnf (ch, "%s balance: %ld gold, %ld questpoints.",
		    clan->name, clan->gold, clan->questpoints);
      return;
    }

  if (!str_prefix (arg1, "deposit"))
    {
      if (NullStr (arg2))
	{
	  chprintln (ch, "Deposit how much?");
	  return;
	}

      if (clan)
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "deposit <amount> qp|gold", NULL);
	      return;
	    }

	  QP = !str_cmp (argument, "qp");
	}

      if (str_cmp (arg2, "all"))
	amount = advatoi (arg2);
      else
	amount = getpay;

      if (!paywith)
	{
	  chprintlnf (ch,
		      "How can you deposit %ld %s when you only have %ld %s?",
		      amount, paytype, getpay, paytype);
	  return;
	}
      if (amount <= 0)
	{
	  chprintlnf (ch, "Only positive figures are allowed.");
	  return;
	}

      if (amount >= MAX_MONEY)
	{
	  chprintlnf (ch,
		      "I'm sorry, our accounts can only hold up to %d %s!",
		      MAX_MONEY, paytype);
	  return;
	}

      paybank;

      if (!clan)
	ch->pcdata->gold_bank = Max (0, ch->pcdata->gold_bank + amount);
      else if (QP)
	clan->questpoints = Max (0, clan->questpoints + amount);
      else
	clan->gold = Max (0, clan->gold + amount);

      chprintlnf (ch,
		  "You deposit %ld %s.  Your new %sbalance is %ld %s.",
		  amount, paytype, clan ? "clan " : "", getbalance, paytype);
      if (clan)
	{
	  rw_clan_data (act_write);
	  if (is_clan (ch) && clan == CharClan (ch))
	    {
	      ClanMember *curr;

	      for (curr = mbr_first; curr != NULL; curr = curr->next)
		{
		  if (!str_cmp (ch->name, curr->name))
		    {
		      if (!QP)
			curr->gold_donated =
			  Range (0, curr->gold_donated + amount, MAX_MONEY);
		      else
			curr->qp_donated += amount;
		      rw_mbr_data (act_write);
		      break;
		    }
		}
	    }
	}
      return;
    }

  if (!str_prefix (arg1, "withdraw"))
    {
      if (clan && (clan != CharClan (ch) || !is_leader (ch))
	  && !IsImmortal (ch))
	{
	  chprintln
	    (ch,
	     "You have to be a clan leader to withdraw from the clan bank.");
	  return;
	}

      if (NullStr (arg2))
	{
	  chprintln (ch, "Withdraw how much?");
	  return;
	}

      if (clan)
	{
	  if (NullStr (argument))
	    {
	      cmd_syntax (ch, NULL, n_fun, "withdraw <amount> qp|gold", NULL);
	      return;
	    }

	  QP = !str_cmp (argument, "qp");
	}

      if (str_cmp (arg2, "all"))
	amount = advatoi (arg2);
      else
	amount = getbalance;

      if (getbalance < amount)
	{
	  chprintlnf (ch,
		      "How can you withdraw %ld %s when your %sbalance is %ld %s?",
		      amount, paytype,
		      clan ? "clan " : "", getbalance, paytype);
	  return;
	}
      if (amount <= 0)
	{
	  chprintlnf (ch, "Only positive figures are allowed.");
	  return;
	}

      if (getpay + amount >= MAX_MONEY)
	{
	  chprintlnf (ch, "I'm sorry you can only carry %d %s!",
		      MAX_MONEY, paytype);
	  return;
	}

      if (!clan)
	ch->pcdata->gold_bank = Max (0, ch->pcdata->gold_bank - amount);
      else if (QP)
	clan->questpoints = Max (0, clan->questpoints - amount);
      else
	clan->gold = Max (0, clan->gold - amount);

      paychar;

      chprintlnf (ch,
		  "You withdraw %ld %s.  Your new %sbalance is %ld %s.",
		  amount, paytype, clan ? "clan " : "", getbalance, paytype);
      return;
    }

  if (clan)
    {
      do_bank (n_fun, ch, "");
      return;
    }

  if (!str_prefix (arg1, "transfer"))
    {
      CharData *victim;

      if (NullStr (arg2))
	{
	  chprintln (ch, "Transfer how much?");
	  return;
	}

      if (str_cmp (arg2, "all"))
	amount = advatoi (arg2);
      else
	amount = ch->pcdata->gold_bank;

      if (amount > ch->pcdata->gold_bank)
	{
	  chprintlnf (ch,
		      "How can you transfer %ld gold when your balance is %ld?",
		      amount, ch->pcdata->gold_bank);
	  return;
	}
      if (amount <= 0)
	{
	  chprintlnf (ch, "Only positive figures are allowed.");
	  return;
	}

      if (!(victim = get_char_world (ch, argument)))
	{
	  chprintlnf (ch, "%s doesn't have a bank account.",
		      capitalize (argument));
	  return;
	}

      if (IsNPC (victim))
	{
	  chprintln (ch, "You can only transfer money to players.");
	  return;
	}

      if (victim->pcdata->gold_bank + amount >= MAX_MONEY)
	{
	  chprintlnf (ch,
		      "Sorry %s's account can't hold that much!",
		      GetName (victim));
	  return;
	}

      ch->pcdata->gold_bank = Max (0, ch->pcdata->gold_bank - amount);
      victim->pcdata->gold_bank = Max (0, victim->pcdata->gold_bank + amount);
      chprintlnf (ch, "You transfer %ld gold. Your new balance is %ld gold.",
		  amount, ch->pcdata->gold_bank);
      chprintlnf (victim,
		  "{g[{GBANK{g]{W %s has transferred {Y%ld{W gold to your account.{x",
		  Pers (ch, victim), amount);
      return;
    }

  if (!str_prefix (arg1, "buy"))
    {
      unsigned int shares;

      if (NullStr (arg2))
	{
	  chprintln (ch, "Buy how many?");
	  return;
	}

      if (str_cmp (arg2, "all"))
	shares = atoi (arg2);
      else
	shares = MAX_SHARES - ch->pcdata->shares;

      if (ch->pcdata->shares + shares > MAX_SHARES)
	{
	  chprintlnf (ch, "You can't buy more than %d shares.", MAX_SHARES);
	  return;
	}
      if ((shares * mud_info.share_value) > ch->pcdata->gold_bank)
	{
	  chprintlnf (ch,
		      "%d shares will cost you %d, deposit more money.",
		      shares, (shares * mud_info.share_value));
	  return;
	}
      if (shares <= 0)
	{
	  chprintlnf (ch, "If you want to sell shares you have to say so...");
	  return;
	}
      ch->pcdata->gold_bank =
	Max (0, ch->pcdata->gold_bank - (shares * mud_info.share_value));
      ch->pcdata->shares = Range (0, ch->pcdata->shares + shares, MAX_SHARES);
      chprintlnf (ch,
		  "You buy %d shares for %d gold, you now have %d shares.",
		  shares, (shares * mud_info.share_value),
		  ch->pcdata->shares);
      return;
    }

  if (!str_prefix (arg1, "sell"))
    {
      int shares;

      if (NullStr (arg2))
	{
	  chprintln (ch, "Sell how many shares?");
	  return;
	}

      if (str_cmp (arg2, "all"))
	shares = atoi (arg2);
      else
	shares = ch->pcdata->shares;

      if (shares > ch->pcdata->shares)
	{
	  chprintlnf (ch, "You only have %d shares.", ch->pcdata->shares);
	  return;
	}
      if (shares <= 0)
	{
	  chprintlnf (ch, "If you want to buy shares you have to say so...");
	  return;
	}

      if (ch->pcdata->gold_bank + (shares * mud_info.share_value) >=
	  MAX_MONEY)
	ch->pcdata->gold_bank = MAX_MONEY;
      else
	ch->pcdata->gold_bank =
	  Max (0, ch->pcdata->gold_bank + (shares * mud_info.share_value));
      ch->pcdata->shares = Max (0, ch->pcdata->shares - shares);
      chprintlnf (ch,
		  "You sell %d shares for %d gold, you now have %d shares.",
		  shares, (shares * mud_info.share_value),
		  ch->pcdata->shares);
      return;
    }

  if (!str_prefix (arg1, "check"))
    {
      chprintlnf (ch, "The current shareprice is %d.", mud_info.share_value);
      if (ch->pcdata->shares)
	{
	  chprintlnf (ch,
		      "You currently have %d shares, (%d a share) worth a total of %d gold.",
		      ch->pcdata->shares, mud_info.share_value,
		      (ch->pcdata->shares * mud_info.share_value));
	}
      return;
    }

  do_bank (n_fun, ch, "");

  return;
}


money_t
net_worth (CharData * ch)
{
  if (!ch)
    return 0;
  else if (IsNPC (ch))
    return ch->gold;
  else
    return (ch->silver + 100 * ch->gold) + ch->pcdata->gold_bank +
      (ch->pcdata->shares * mud_info.share_value);
}


bool
check_worth (CharData * ch, money_t amount, currency_t type)
{
  switch (type)
    {
    default:
    case VALUE_SILVER:
      return (ch->silver + (100 * ch->gold) >= amount);
    case VALUE_GOLD:
      return (ch->gold >= amount);
    }
}

Do_Fun (do_tax)
{
  char arg[MIL];
  CharData *vict;
  int p;
  money_t net, tax;

  argument = one_argument (argument, arg);

  if (NullStr (arg))
    {
      cmd_syntax (ch, NULL, n_fun, "<char> <percentange>", NULL);
      return;
    }

  if ((vict = get_char_world (ch, arg)) == NULL)
    {
      chprintln (ch, "No such character.");
      return;
    }

  if (!is_number (argument) || (p = atoi (argument)) < 1 || p > 100)
    {
      chprintln (ch, "Percentage should be between 1 and 100.");
      return;
    }

  net = (vict->silver + (100 * vict->gold));

  if (!net)
    {
      act ("$N has to money to tax.", ch, NULL, vict, TO_CHAR);
      return;
    }

  tax = net * p / 100;

  deduct_cost (vict, tax, VALUE_DEFAULT);
  chprintlnf
    (vict,
     "A small man holding an elaborite ledger appears in front of you quite suddenly,"
     " and politely informs you the council has taxed you %ld coin, refusal to pay such fee, will be"
     " dealt with quickly.", tax);
  chprintlnf (ch, "You tax %s %s.", GetName (vict),
	      cost_str (tax, VALUE_DEFAULT));
}
