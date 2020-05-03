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
* The Dawn of Time v1.69q (c)1997-2002 Michael Garratt                    *
* >> A number of people have contributed to the Dawn codebase, with the   *
*    majority of code written by Michael Garratt - www.dawnoftime.org     *
* >> To use this source code, you must fully comply with the dawn license *
*    in licenses.txt... In particular, you may not remove this copyright  *
*    notice.                                                              *
***************************************************************************
*          1stMud ROM Derivative (c) 2001-2004 by Markanth                *
*            http://www.firstmud.com/  <markanth@firstmud.com>            *
*         By using this code you have agreed to follow the term of        *
*             the 1stMud license in ../doc/1stMud/LICENSE                 *
***************************************************************************/

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "data_table.h"
#include "olc.h"

void
init_name_profiles (void)
{
  const char *word2;
  char buf[MSL];
  int i = 0, count;
  NameProfile *nl;

  top_name_profile = 0;
  if (!name_profile_first)
    {
      name_profile_first = new_name_profile ();
      name_profile_first->title = str_dup ("mixed fantasy names");
      name_profile_first->part[0] =
	str_dup ("A Ab Ac Ad Af Agr Ast As Al Adw Adr "
		 "Ar B Br C C C Cr Ch Cad D Dr Dw Ed Eth Et Er El Eow F Fr G Gr Gw Gw "
		 "Gal Gl H Ha Ib Jer K Ka Ked L Loth Lar Leg M Mir N Nyd Ol Oc On P "
		 "Pr R Rh S Sev T Tr Th Th V Y Yb Z W W Wic");
      name_profile_first->part[1] =
	str_dup ("a ae ae au ao are ale ali ay ardo e "
		 "ei ea ea eri era ela eli enda erra i ia ie ire ira ila ili ira igo "
		 "o oa oi oe ore u y");
      name_profile_first->part[2] =
	str_dup ("_ _ _ _ _ _ a and b bwyn baen bard "
		 "c ctred cred ch can d dan don der dric dfrid dus f g gord gan l li "
		 "lgrin lin lith lath loth ld ldric ldan m mas mos mar mond n nydd "
		 "nidd nnon nwan nyth nad nn nnor nd p r ron rd s sh seth sean t th "
		 "th tha tlan trem tram v vudd w wan win win wyn wyn wyr wyr wyth");
      name_profile_first->next = NULL;
    };

  for (nl = name_profile_first; nl != NULL; nl = nl->next)
    {
      top_name_profile++;
      for (i = 0; i < MAX_PARTS; i++)
	{
	  count = 0;
	  word2 = nl->part[i];
	  while (true)
	    {
	      word2 = one_argument (word2, buf);
	      if (NullStr (word2))
		{
		  break;
		}
	      count++;
	    }
	  nl->part_count[i] = count;
	}
    }
}

DataTable name_profile_data_table[] = {
  DTE (STRING, name_profile, title, NULL, NULL, str, NULL),
  DTE (STRING_ARRAY, name_profile, part, MAX_PARTS, NULL, sarray, NULL),
  DTE_END
};

TableSave_Fun (rw_name_profile_data)
{
  rw_list (type, NAMEGEN_FILE, NameProfile, name_profile);
  if (type == act_read)
    init_name_profiles ();
}

NameProfile *
profile_lookup (int which)
{
  NameProfile *nl;
  int i = 0;

  for (nl = name_profile_first; nl; nl = nl->next)
    {
      if (++i >= which)
	break;
    }
  return nl;
}


char *
genname (NameProfile * profile)
{
  static int i;
  static char result[5][100];
  const char *word;
  char buf[MIL];
  int wordnum;
  int count, j;


  ++i, i %= 5;
  result[i][0] = NUL;

  for (j = 0; j < MAX_PARTS; j++)
    {
      word = profile->part[j];
      wordnum = number_range (0, profile->part_count[j]);
      count = 0;
      while (true)
	{
	  word = one_argument (word, buf);
	  if (NullStr (buf))
	    {
	      bugf
		("char * genname(%s) - no word found!!! j=%d, wn=%d, count=%d.",
		 profile->title, j, wordnum, count);
	      break;
	    }
	  if (count == wordnum)
	    {
	      break;
	    }
	  count++;
	}
      if (!NullStr (buf) && buf[0] != '_')
	{
	  strcat (result[i], buf);
	}
    }
  return result[i];
}

Do_Fun (do_genname)
{
  int count, i, profile_num;
  NameProfile *nl;
  char arg[MIL];
  Column Cd;

  if (!name_profile_first)
    {
      chprintln (ch, "Random name generation is currently unavailable.");
      return;
    }

  if (NullStr (argument) || !is_number (argument))
    {
      chprintlnf (ch, "{R%s NAME GENERATOR{x", strupper (mud_info.name));
      chprintlnf (ch, "Syntax: %s <1-%d> <amount>", n_fun, top_name_profile);
      chprintln (ch,
		 "Notes: <profile> relates to the name profile you want to use.");

      for (count = 0, nl = name_profile_first; nl; nl = nl->next)
	{
	  chprintlnf (ch, "%2d> %s", ++count, nl->title);
	}
      return;
    }

  argument = one_argument (argument, arg);

  profile_num = atoi (arg);

  if (!NullStr (argument))
    count = atoi (argument);
  else
    count = 60;

  if (profile_num < 1 || profile_num > top_name_profile
      || (nl = profile_lookup (profile_num)) == NULL)
    {
      chprintln (ch, "Invalid profile number.");
      do_genname (n_fun, ch, "");
      return;
    }

  if (count < 1 || count > 100)
    {
      chprintln (ch,
		 "Please choose a reasonable amount of names to display.");
      return;
    }

  chprintlnf (ch, "{R%s NAME GENERATOR - %s{x", strupper (mud_info.name),
	      strupper (nl->title));

  set_cols (&Cd, ch, 4, COLS_CHAR, ch);

  for (i = 0; i < count; i++)
    {
      print_cols (&Cd, capitalize (genname (nl)));
    }
  cols_nl (&Cd);
}

const char *
get_random_name (void)
{
  NameProfile *nl;

  do
    {
      nl = profile_lookup (number_range (1, top_name_profile));
    }
  while (nl == NULL);

  return capitalize (genname (nl));
}
