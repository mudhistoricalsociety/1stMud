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
#include "olc.h"
#include "tables.h"
#include "interp.h"

Proto (void free_channel_history, (PcData *));
Proto (void init_channel_history, (PcData *));


BanData *
new_ban (void)
{
  BanData *Data;

  if (ban_free != NULL)
    {
      Data = ban_free;
      ban_free = ban_free->next;
      top_ban_free--;
    }
  else
    {
      Data = (BanData *) alloc_perm (sizeof (BanData));
    }
  *Data = ban_zero;
  top_ban++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_ban (BanData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  top_ban--;
  Invalidate (Data);
  LinkSingle (Data, ban_free, next);
  top_ban_free++;
}



Descriptor *
new_descriptor (void)
{
  Descriptor *Data;

  if (descriptor_free != NULL)
    {
      Data = descriptor_free;
      descriptor_free = descriptor_free->next;
      top_descriptor_free--;
    }
  else
    {
      Data = (Descriptor *) alloc_perm (sizeof (Descriptor));
    }
  *Data = descriptor_zero;
  top_descriptor++;
  Data->next = Data->prev = NULL;
  Data->connected = CON_GET_TERM;
  Data->showstr_head = NULL;
  Data->showstr_point = NULL;
  Data->run_buf = NULL;
  Data->run_head = NULL;
  Data->outsize = 2000;
  Data->scr_width = DEFAULT_SCR_WIDTH;
  Data->scr_height = DEFAULT_SCR_HEIGHT;
  Data->bytes_normal = 0;
#ifndef DISABLE_MCCP

  Data->bytes_compressed = 0;
  Data->mccp_version = 0;
#endif

  Data->editor = ED_NONE;
  Data->pEdit = NULL;
  alloc_mem (Data->outbuf, char, Data->outsize);

  Data->mxp.supports = &str_empty[0];
  Data->mxp.mxp_ver = 0.0;
  Data->mxp.client_ver = 0.0;
  Data->mxp.style_ver = 0.0;
  Data->mxp.client = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_descriptor (Descriptor * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->host);
  free_string (Data->showstr_head);
  free_mem (Data->outbuf);
  free_string (Data->mxp.supports);
  free_string (Data->mxp.client);
  top_descriptor--;
  Invalidate (Data);
  LinkSingle (Data, descriptor_free, next);
  top_descriptor_free++;
}

GenData *
new_gen_data (void)
{
  GenData *Data;

  if (gen_data_free != NULL)
    {
      Data = gen_data_free;
      gen_data_free = gen_data_free->next;
      top_gen_data_free--;
    }
  else
    {
      Data = (GenData *) alloc_perm (sizeof (GenData));
    }
  *Data = gen_data_zero;
  top_gen_data++;
  Data->next = Data->prev = NULL;

  alloc_mem (Data->skill_chosen, bool, top_skill);
  alloc_mem (Data->group_chosen, bool, top_group);
  Data->rerolls = 1;
  Validate (Data);
  return Data;
}

void
free_gen_data (GenData * Data)
{
  if (!IsValid (Data))
    return;

  free_mem (Data->skill_chosen);
  free_mem (Data->group_chosen);
  top_gen_data--;
  Invalidate (Data);
  LinkSingle (Data, gen_data_free, next);
  top_gen_data_free++;
}


ExDescrData *
new_ed (void)
{
  ExDescrData *Data;

  if (ed_free != NULL)
    {
      Data = ed_free;
      ed_free = ed_free->next;
      top_ed_free--;
    }
  else
    {
      Data = (ExDescrData *) alloc_perm (sizeof (ExDescrData));
    }
  *Data = ed_zero;
  top_ed++;
  Data->next = Data->prev = NULL;
  Data->keyword = &str_empty[0];
  Data->description = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_ed (ExDescrData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->keyword);
  free_string (Data->description);
  top_ed--;
  Invalidate (Data);
  LinkSingle (Data, ed_free, next);
  top_ed_free++;
}



AffectData *
new_affect (void)
{
  AffectData *Data;

  if (affect_free != NULL)
    {
      Data = affect_free;
      affect_free = affect_free->next;
      top_affect_free--;
    }
  else
    {
      Data = (AffectData *) alloc_perm (sizeof (AffectData));
    }
  *Data = affect_zero;
  top_affect++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_affect (AffectData * Data)
{
  if (!IsValid (Data))
    return;

  top_affect--;
  Invalidate (Data);
  LinkSingle (Data, affect_free, next);
  top_affect_free++;
}



ObjData *
new_obj (void)
{
  ObjData *Data;

  if (obj_free != NULL)
    {
      Data = obj_free;
      obj_free = obj_free->next;
      top_obj_free--;
    }
  else
    {
      Data = (ObjData *) alloc_perm (sizeof (ObjData));
    }
  *Data = obj_zero;
  top_obj++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->short_descr = &str_empty[0];
  Data->description = &str_empty[0];
  Data->owner = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_obj (ObjData * Data)
{
  AffectData *paf, *paf_next;
  ExDescrData *ed, *ed_next;

  if (!IsValid (Data))
    return;

  for (paf = Data->affect_first; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      free_affect (paf);
    }
  Data->affect_first = NULL;

  for (ed = Data->ed_first; ed != NULL; ed = ed_next)
    {
      ed_next = ed->next;
      free_ed (ed);
    }
  Data->ed_first = Data->ed_last = NULL;
  free_string (Data->name);
  free_string (Data->description);
  free_string (Data->short_descr);
  free_string (Data->owner);
  top_obj--;
  Invalidate (Data);
  LinkSingle (Data, obj_free, next);
  top_obj_free++;
}



CharData *
new_char (void)
{
  CharData *Data;
  int i;

  if (char_free != NULL)
    {
      Data = char_free;
      char_free = char_free->next;
      top_char_free--;
    }
  else
    {
      Data = (CharData *) alloc_perm (sizeof (CharData));
    }
  *Data = char_zero;
  top_char++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->short_descr = &str_empty[0];
  Data->long_descr = &str_empty[0];
  Data->description = &str_empty[0];
  Data->prompt = &str_empty[0];
  Data->gprompt = &str_empty[0];
  Data->prefix = &str_empty[0];
  Data->logon = current_time;
  Data->lines = 0;
  Data->columns = 0;
  Data->rank = 0;
  Data->deity = NULL;
  memset (Data->armor, 100, MAX_AC);
  Data->position = POS_STANDING;
  Data->hit = 20;
  Data->max_hit = 20;
  Data->mana = 100;
  Data->max_mana = 100;
  Data->move = 100;
  Data->max_move = 100;
  memset (Data->perm_stat, 13, STAT_MAX);
  memset (Data->mod_stat, 0, STAT_MAX);
  memset (Data->stance, 0, MAX_STANCE);

  for (i = 0; i < MAX_MCLASS; i++)
    Data->Class[i] = -1;
  Data->Class[CLASS_COUNT] = 0;
  Validate (Data);
  return Data;
}

void
free_char (CharData * Data)
{
  ObjData *obj;
  ObjData *obj_next;
  AffectData *paf;
  AffectData *paf_next;

  if (!IsValid (Data))
    return;

  if (IsNPC (Data))
    mobile_count--;

  for (obj = Data->carrying_first; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      extract_obj (obj);
    }

  for (paf = Data->affect_first; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      affect_remove (Data, paf);
    }

  free_string (Data->name);
  free_string (Data->short_descr);
  free_string (Data->long_descr);
  free_string (Data->description);
  free_string (Data->prompt);
  free_string (Data->gprompt);
  free_string (Data->prefix);
  free_pcdata (Data->pcdata);
  top_char--;
  Invalidate (Data);
  LinkSingle (Data, char_free, next);
  top_char_free++;
}

PcData *
new_pcdata (void)
{
  PcData *Data;
  int i;

  if (pcdata_free != NULL)
    {
      Data = pcdata_free;
      pcdata_free = pcdata_free->next;
      top_pcdata_free--;
    }
  else
    {
      Data = (PcData *) alloc_perm (sizeof (PcData));
    }
  *Data = pcdata_zero;
  top_pcdata++;
  Data->next = Data->prev = NULL;
  for (i = 0; i < MAX_ALIAS; i++)
    {
      Data->alias[i] = &str_empty[0];
      Data->alias_sub[i] = &str_empty[0];
    }
  for (i = 0; i < MAX_BUDDY; i++)
    Data->buddies[i] = &str_empty[0];
  for (i = 0; i < MAX_IGNORE; i++)
    {
      Data->ignore[i] = &str_empty[0];
      Data->ignore_flags[i] = 0;
    }
  for (i = 0; i < MAX_BOARD; i++)
    Data->unsubscribed[i] = false;
  init_channel_history (Data);
  Data->buffer = new_buf ();
  alloc_mem (Data->learned, int, top_skill);

  alloc_mem (Data->group_known, bool, top_group);
  Data->str_ed_key = '/';
  Data->clan = NULL;
  Data->invited = NULL;
  Data->confirm_delete = false;
  Data->board = &boards[DEFAULT_BOARD];
  Data->pwd = &str_empty[0];
  Data->webpass = &str_empty[0];
  Data->bamfin = &str_empty[0];
  Data->bamfout = &str_empty[0];
  Data->title = &str_empty[0];
  Data->who_descr = &str_empty[0];
  Data->afk_msg = &str_empty[0];
  memset (Data->condition, 48, COND_MAX);
  Data->condition[COND_DRUNK] = 0;
  memset (Data->gamestat, 0, MAX_GAMESTAT);
  Data->timezone = -1;
  Data->prime_class = 0;
  Validate (Data);
  return Data;
}

void
free_pcdata (PcData * Data)
{
  int alias, pos, ignore;

  if (!IsValid (Data))
    return;

  free_string (Data->pwd);
  free_string (Data->bamfin);
  free_string (Data->bamfout);
  free_string (Data->title);
  free_string (Data->who_descr);
  free_string (Data->webpass);
  free_string (Data->afk_msg);
  free_buf (Data->buffer);
  free_mem (Data->learned);
  free_mem (Data->group_known);
  for (alias = 0; alias < MAX_ALIAS; alias++)
    {
      free_string (Data->alias[alias]);
      free_string (Data->alias_sub[alias]);
    }
  for (pos = 0; pos < MAX_BUDDY; pos++)
    free_string (Data->buddies[pos]);
  for (ignore = 0; ignore < MAX_IGNORE; ignore++)
    free_string (Data->ignore[ignore]);
#ifndef DISABLE_I3

  free_i3chardata (Data);
#endif

  free_channel_history (Data);
  top_pcdata--;
  Invalidate (Data);
  LinkSingle (Data, pcdata_free, next);
  top_pcdata_free++;
}


long last_pc_id;
long last_mob_id;

long
get_pc_id (void)
{
  int val;

  val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
  last_pc_id = val;
  return val;
}

long
get_mob_id (void)
{
  last_mob_id++;
  return last_mob_id;
}

Buffer *
new_buf (void)
{
  Buffer *Data;

  if (buf_free != NULL)
    {
      Data = buf_free;
      buf_free = buf_free->next;
      top_buf_free--;
    }
  else
    {
      Data = (Buffer *) alloc_perm (sizeof (Buffer));
    }
  *Data = buf_zero;
  top_buf++;
  Data->next = Data->prev = NULL;
  Data->state = BUFFER_SAFE;
  Data->size = KILOBYTE;

  alloc_mem (Data->string, char, Data->size);

  Data->string[0] = '\0';
  Validate (Data);
  return Data;
}

void
free_buf (Buffer * Data)
{
  if (!IsValid (Data))
    return;

  free_mem (Data->string);
  Data->string = NULL;
  Data->size = 0;
  Data->state = BUFFER_FREED;
  top_buf--;
  Invalidate (Data);
  LinkSingle (Data, buf_free, next);
  top_buf_free++;
}

int
bprint (Buffer * buffer, const char *string)
{
  int ret_len, len;
  char *oldstr;

  if (!buffer || NullStr (string) || buffer->state == BUFFER_OVERFLOW)
    return 0;

  oldstr = buffer->string;
  len = strlen (buffer->string) + (ret_len = strlen (string)) + 1;

  if (len >= buffer->size)
    {
      alloc_mem (buffer->string, char, len);

      if (buffer->string == NULL)
	{
	  buffer->state = BUFFER_OVERFLOW;
	  bugf ("buffer overflow %d", buffer->size);
	  return -1;
	}

      buffer->size = len;
      strcpy (buffer->string, oldstr);
      free_mem (oldstr);
    }

  strcat (buffer->string, string);
  return ret_len;
}

int
bprintf (Buffer * buffer, const char *messg, ...)
{
  char buf[MPL];
  va_list args;

  if (!buffer || NullStr (messg))
    return 0;

  va_start (args, messg);
  vsnprintf (buf, sizeof (buf), messg, args);
  va_end (args);

  return bprint (buffer, buf);
}

int
bprintlnf (Buffer * buffer, const char *messg, ...)
{
  va_list args;
  char send_buf[MPL];

  if (!buffer)
    return 0;

  send_buf[0] = NUL;
  if (!NullStr (messg))
    {
      va_start (args, messg);
      vsnprintf (send_buf, sizeof (send_buf), messg, args);
      va_end (args);
    }
  strcat (send_buf, NEWLINE);
  return bprint (buffer, send_buf);
}

int
bprintln (Buffer * buffer, const char *messg)
{
  int len = 0;

  if (!buffer)
    return 0;

  if (!NullStr (messg))
    len += bprint (buffer, messg);
  len += bprint (buffer, NEWLINE);
  return len;
}

void
clear_buf (Buffer * buffer)
{
  buffer->string[0] = NUL;
  buffer->state = BUFFER_SAFE;
}

char *
buf_string (Buffer * buffer)
{
  return buffer->string;
}


ProgList *
new_prog_list (void)
{
  ProgList *Data;

  if (prog_list_free != NULL)
    {
      Data = prog_list_free;
      prog_list_free = prog_list_free->next;
      top_prog_list_free--;
    }
  else
    {
      Data = (ProgList *) alloc_perm (sizeof (ProgList));
    }
  *Data = prog_list_zero;
  top_prog_list++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_prog_list (ProgList * Data)
{
  if (!IsValid (Data))
    return;

  top_prog_list--;
  Invalidate (Data);
  LinkSingle (Data, prog_list_free, next);
  top_prog_list_free++;
}

HelpData *
new_help (void)
{
  HelpData *Data;

  if (help_free != NULL)
    {
      Data = help_free;
      help_free = help_free->next;
      top_help_free--;
    }
  else
    {
      Data = (HelpData *) alloc_perm (sizeof (HelpData));
    }
  *Data = help_zero;
  top_help++;
  Data->next = Data->prev = NULL;
  Data->keyword = &str_empty[0];
  Data->text = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_help (HelpData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->keyword);
  free_string (Data->text);
  top_help--;
  Invalidate (Data);
  LinkSingle (Data, help_free, next);
  top_help_free++;
}


StatData *
new_stat (void)
{
  StatData *Data;

  if (stat_free != NULL)
    {
      Data = stat_free;
      stat_free = stat_free->next;
      top_stat_free--;
    }
  else
    {
      Data = (StatData *) alloc_perm (sizeof (StatData));
    }
  *Data = stat_zero;
  top_stat++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_stat (StatData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  top_stat--;
  Invalidate (Data);
  LinkSingle (Data, stat_free, next);
  top_stat_free++;
}

CorpseData *
new_corpse (void)
{
  CorpseData *Data;

  if (corpse_free != NULL)
    {
      Data = corpse_free;
      corpse_free = corpse_free->next;
      top_corpse_free--;
    }
  else
    {
      Data = (CorpseData *) alloc_perm (sizeof (CorpseData));
    }
  *Data = corpse_zero;
  top_corpse++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_corpse (CorpseData * Data)
{
  if (!IsValid (Data))
    return;

  if (Data->corpse != NULL)
    {
      free_obj (Data->corpse);
      Data->corpse = NULL;
    }
  top_corpse--;
  Invalidate (Data);
  LinkSingle (Data, corpse_free, next);
  top_corpse_free++;
}

AuctionData *
new_auction (void)
{
  AuctionData *Data;

  if (auction_free != NULL)
    {
      Data = auction_free;
      auction_free = auction_free->next;
      top_auction_free--;
    }
  else
    {
      Data = (AuctionData *) alloc_perm (sizeof (AuctionData));
    }
  *Data = auction_zero;
  top_auction++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_auction (AuctionData * Data)
{
  if (!IsValid (Data))
    return;

  Data->high_bidder = NULL;
  Data->item = NULL;
  Data->owner = NULL;
  top_auction--;
  Invalidate (Data);
  LinkSingle (Data, auction_free, next);
  top_auction_free++;
}

WebpassData *
new_wpwd (void)
{
  WebpassData *Data;

  if (wpwd_free != NULL)
    {
      Data = wpwd_free;
      wpwd_free = wpwd_free->next;
      top_wpwd_free--;
    }
  else
    {
      Data = (WebpassData *) alloc_perm (sizeof (WebpassData));
    }
  *Data = wpwd_zero;
  top_wpwd++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->passw = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_wpwd (WebpassData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->passw);
  top_wpwd--;
  Invalidate (Data);
  LinkSingle (Data, wpwd_free, next);
  top_wpwd_free++;
}

RaceData *
new_race (void)
{
  RaceData *Data;

  if (race_free != NULL)
    {
      Data = race_free;
      race_free = race_free->next;
      top_race_free--;
    }
  else
    {
      Data = (RaceData *) alloc_perm (sizeof (RaceData));
    }
  *Data = race_zero;
  top_race++;
  Data->next = Data->prev = NULL;

  Data->name = &str_empty[0];
  Data->description = &str_empty[0];
  Data->pc_race = false;
  Data->act = 0;
  Data->aff = 0;
  Data->off = 0;
  Data->imm = 0;
  Data->res = 0;
  Data->vuln = 0;
  Data->form = 0;
  Data->parts = 0;
  set_array (Data->skills, &str_empty[0], MAX_RACE_SKILL);
  memset (Data->stats, 0, STAT_MAX);
  memset (Data->max_stats, 0, STAT_MAX);
  alloc_mem (Data->class_mult, int, top_class);

  Data->points = 0;
  Data->size = SIZE_MEDIUM;
  Validate (Data);
  return Data;
}

void
free_race (RaceData * Data)
{
  int x;

  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->description);
  for (x = 0; x < MAX_RACE_SKILL; x++)
    free_string (Data->skills[x]);
  free_mem (Data->class_mult);
  top_race--;
  Invalidate (Data);
  LinkSingle (Data, race_free, next);
  top_race_free++;
}

ClanMember *
new_mbr (void)
{
  ClanMember *Data;

  if (mbr_free != NULL)
    {
      Data = mbr_free;
      mbr_free = mbr_free->next;
      top_mbr_free--;
    }
  else
    {
      Data = (ClanMember *) alloc_perm (sizeof (ClanMember));
    }
  *Data = mbr_zero;
  top_mbr++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->clan = NULL;
  Data->rank = 0;
  Validate (Data);
  return Data;
}

void
free_mbr (ClanMember * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  top_mbr--;
  Invalidate (Data);
  LinkSingle (Data, mbr_free, next);
  top_mbr_free++;
}

SocialData *
new_social (void)
{
  SocialData *Data;

  if (social_free != NULL)
    {
      Data = social_free;
      social_free = social_free->next;
      top_social_free--;
    }
  else
    {
      Data = (SocialData *) alloc_perm (sizeof (SocialData));
    }
  *Data = social_zero;
  top_social++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->char_no_arg = &str_empty[0];
  Data->others_no_arg = &str_empty[0];
  Data->char_found = &str_empty[0];
  Data->others_found = &str_empty[0];
  Data->vict_found = &str_empty[0];
  Data->char_not_found = &str_empty[0];
  Data->char_auto = &str_empty[0];
  Data->others_auto = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_social (SocialData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->char_no_arg);
  free_string (Data->others_no_arg);
  free_string (Data->char_found);
  free_string (Data->others_found);
  free_string (Data->vict_found);
  free_string (Data->char_not_found);
  free_string (Data->char_auto);
  free_string (Data->others_auto);
  top_social--;
  Invalidate (Data);
  LinkSingle (Data, social_free, next);
  top_social_free++;
}

DeityData *
new_deity (void)
{
  DeityData *Data;

  if (deity_free != NULL)
    {
      Data = deity_free;
      deity_free = deity_free->next;
      top_deity_free--;
    }
  else
    {
      Data = (DeityData *) alloc_perm (sizeof (DeityData));
    }
  *Data = deity_zero;
  top_deity++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->desc = &str_empty[0];
  Data->skillname = &str_empty[0];
  Data->ethos = ETHOS_TRUE_NEUTRAL;
  Validate (Data);
  return Data;
}

void
free_deity (DeityData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->desc);
  free_string (Data->skillname);
  top_deity--;
  Invalidate (Data);
  LinkSingle (Data, deity_free, next);
  top_deity_free++;
}

ClanData *
new_clan (void)
{
  ClanData *Data;
  int x;

  if (clan_free != NULL)
    {
      Data = clan_free;
      clan_free = clan_free->next;
      top_clan_free--;
    }
  else
    {
      Data = (ClanData *) alloc_perm (sizeof (ClanData));
    }
  *Data = clan_zero;
  top_clan++;
  Data->next = Data->prev = NULL;

  Data->name = &str_empty[0];
  Data->who_name = &str_empty[0];
  Data->description = &str_empty[0];
  Data->ethos = ETHOS_TRUE_NEUTRAL;
  for (x = 0; x < MAX_RANK; x++)
    Data->rank[x].rankname = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_clan (ClanData * Data)
{
  int x;

  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->who_name);
  free_string (Data->description);
  for (x = 0; x < MAX_RANK; x++)
    free_string (Data->rank[x].rankname);
  top_clan--;
  Invalidate (Data);
  LinkSingle (Data, clan_free, next);
  top_clan_free++;
}

CmdData *
new_cmd (void)
{
  CmdData *Data;

  if (cmd_free != NULL)
    {
      Data = cmd_free;
      cmd_free = cmd_free->next;
      top_cmd_free--;
    }
  else
    {
      Data = (CmdData *) alloc_perm (sizeof (CmdData));
    }
  *Data = cmd_zero;
  top_cmd++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->do_fun = do_null;
  Data->level = 0;
  Data->position = POS_DEAD;
  Data->category = CMDCAT_NOSHOW;
  Data->log = LOG_NORMAL;
  Validate (Data);
  return Data;
}

void
free_cmd (CmdData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  top_cmd--;
  Invalidate (Data);
  LinkSingle (Data, cmd_free, next);
  top_cmd_free++;
}

ResetData *
new_reset (void)
{
  ResetData *Data;

  if (reset_free != NULL)
    {
      Data = reset_free;
      reset_free = reset_free->next;
      top_reset_free--;
    }
  else
    {
      Data = (ResetData *) alloc_perm (sizeof (ResetData));
    }
  *Data = reset_zero;
  top_reset++;
  Data->next = Data->prev = NULL;
  Data->command = 'X';
  Data->arg1 = 0;
  Data->arg2 = 0;
  Data->arg3 = 0;
  Data->arg4 = 0;
  Validate (Data);
  return Data;
}

void
free_reset (ResetData * Data)
{
  if (!IsValid (Data))
    return;

  top_reset--;
  Invalidate (Data);
  LinkSingle (Data, reset_free, next);
  top_reset_free++;
}

AreaData *
new_area (void)
{
  AreaData *Data;

  if (area_free != NULL)
    {
      Data = area_free;
      area_free = area_free->next;
      top_area_free--;
    }
  else
    {
      Data = (AreaData *) alloc_perm (sizeof (AreaData));
    }
  *Data = area_zero;
  top_area++;
  Data->next = Data->prev = NULL;
  Data->name = str_dup ("New area");
  Data->area_flags = 0;
  Data->security = 9;
  Data->builders = str_dup ("None");
  Data->credits = &str_empty[0];
  Data->lvl_comment = &str_empty[0];
  Data->resetmsg = &str_empty[0];
  Data->min_level = 0;
  Data->max_level = MAX_LEVEL;
  Data->version = 0;
  Data->min_vnum = 0;
  Data->max_vnum = 0;
  Data->weather.temp = 0;
  Data->weather.precip = 0;
  Data->weather.wind = 0;
  Data->weather.temp_vector = 0;
  Data->weather.precip_vector = 0;
  Data->weather.wind_vector = 0;
  Data->weather.climate_temp = 2;
  Data->weather.climate_precip = 2;
  Data->weather.climate_wind = 2;
  Data->weather.echo = &str_empty[0];
  Data->weather.echo_color = &str_empty[0];
  Data->age = 32;
  Data->nplayer = 0;
  Data->vnum = top_area;
  Data->sound = NULL;
  Data->empty = true;
  Data->file_name = str_dupf ("area%d.are", Data->vnum);
  Data->clan = NULL;
  Validate (Data);
  return Data;
}

void
free_area (AreaData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->file_name);
  free_string (Data->builders);
  free_string (Data->lvl_comment);
  free_string (Data->credits);
  free_string (Data->resetmsg);
  top_area--;
  Invalidate (Data);
  LinkSingle (Data, area_free, next);
  top_area_free++;
}

ExitData *
new_exit (void)
{
  ExitData *Data;

  if (exit_free != NULL)
    {
      Data = exit_free;
      exit_free = exit_free->next;
      top_exit_free--;
    }
  else
    {
      Data = (ExitData *) alloc_perm (sizeof (ExitData));
    }
  *Data = exit_zero;
  top_exit++;
  Data->next = Data->prev = NULL;
  Data->u1.to_room = NULL;
  Data->exit_info = 0;
  Data->key = 0;
  Data->keyword = &str_empty[0];
  Data->description = &str_empty[0];
  Data->rs_flags = 0;
  Validate (Data);
  return Data;
}

void
free_exit (ExitData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->keyword);
  free_string (Data->description);
  top_exit--;
  Invalidate (Data);
  LinkSingle (Data, exit_free, next);
  top_exit_free++;
}

RoomIndex *
new_room_index (void)
{
  RoomIndex *Data;
  int door;

  if (room_index_free != NULL)
    {
      Data = room_index_free;
      room_index_free = room_index_free->next;
      top_room_index_free--;
    }
  else
    {
      Data = (RoomIndex *) alloc_perm (sizeof (RoomIndex));
    }
  *Data = room_index_zero;
  top_room_index++;
  Data->next = Data->prev = NULL;

  Data->person_first = NULL;
  Data->content_first = NULL;
  Data->ed_first = NULL;
  Data->area = NULL;

  for (door = 0; door < MAX_DIR; door++)
    Data->exit[door] = NULL;

  Data->name = &str_empty[0];
  Data->description = &str_empty[0];
  Data->owner = &str_empty[0];
  Data->vnum = 0;
  Data->room_flags = 0;
  Data->light = 0;
  Data->sector_type = SECT_NONE;
  Data->heal_rate = 100;
  Data->mana_rate = 100;
  Data->guild = -1;
  Validate (Data);
  return Data;
}

void
free_room_index (RoomIndex * Data)
{
  int door;
  ExDescrData *pExtra;
  ResetData *pReset;
  ProgList *rp;

  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->description);
  free_string (Data->owner);

  for (rp = Data->rprog_first; rp; rp = rp->next)
    free_prog_list (rp);

  for (door = 0; door < MAX_DIR; door++)
    if (Data->exit[door])
      free_exit (Data->exit[door]);

  for (pExtra = Data->ed_first; pExtra; pExtra = pExtra->next)
    free_ed (pExtra);

  for (pReset = Data->reset_first; pReset; pReset = pReset->next)
    free_reset (pReset);
  top_room_index--;
  Invalidate (Data);
  LinkSingle (Data, room_index_free, next);
  top_room_index_free++;
}

ShopData *
new_shop (void)
{
  ShopData *Data;

  if (shop_free != NULL)
    {
      Data = shop_free;
      shop_free = shop_free->next;
      top_shop_free--;
    }
  else
    {
      Data = (ShopData *) alloc_perm (sizeof (ShopData));
    }
  *Data = shop_zero;
  top_shop++;
  Data->next = Data->prev = NULL;
  Data->keeper = 0;
  memset (Data->buy_type, 0, MAX_TRADE);
  Data->profit_buy = 100;
  Data->profit_sell = 100;
  Data->open_hour = 0;
  Data->close_hour = 23;
  Validate (Data);
  return Data;
}

void
free_shop (ShopData * Data)
{
  if (!IsValid (Data))
    return;

  top_shop--;
  Invalidate (Data);
  LinkSingle (Data, shop_free, next);
  top_shop_free++;
}

ObjIndex *
new_obj_index (void)
{
  ObjIndex *Data;

  if (obj_index_free != NULL)
    {
      Data = obj_index_free;
      obj_index_free = obj_index_free->next;
      top_obj_index_free--;
    }
  else
    {
      Data = (ObjIndex *) alloc_perm (sizeof (ObjIndex));
    }
  *Data = obj_index_zero;
  top_obj_index++;
  Data->next = Data->prev = NULL;
  Data->ed_first = NULL;
  Data->affect_first = NULL;
  Data->area = NULL;
  Data->name = str_dup ("no name");
  Data->short_descr = str_dup ("(no short description)");
  Data->description = str_dup ("(no description)");
  Data->vnum = 0;
  Data->item_type = ITEM_TRASH;
  Data->extra_flags = 0;
  Data->wear_flags = 0;
  Data->count = 0;
  Data->weight = 0;
  Data->cost = 0;
  Data->material = str_dup ("unknown");
  Data->condition = 100;
  memset (Data->value, 0, 5);
  Data->new_format = true;
  Validate (Data);
  return Data;
}

void
free_obj_index (ObjIndex * Data)
{
  ExDescrData *pExtra;
  AffectData *pAf;
  ProgList *op;

  if (!IsValid (Data))
    return;

  free_string (Data->name);
  free_string (Data->short_descr);
  free_string (Data->description);

  for (op = Data->oprog_first; op; op = op->next)
    free_prog_list (op);

  for (pAf = Data->affect_first; pAf; pAf = pAf->next)
    free_affect (pAf);

  for (pExtra = Data->ed_first; pExtra; pExtra = pExtra->next)
    free_ed (pExtra);
  top_obj_index--;
  Invalidate (Data);
  LinkSingle (Data, obj_index_free, next);
  top_obj_index_free++;
}

CharIndex *
new_char_index (void)
{
  CharIndex *Data;
  int i;

  if (char_index_free != NULL)
    {
      Data = char_index_free;
      char_index_free = char_index_free->next;
      top_char_index_free--;
    }
  else
    {
      Data = (CharIndex *) alloc_perm (sizeof (CharIndex));
    }
  *Data = char_index_zero;
  top_char_index++;
  Data->next = Data->prev = NULL;

  Data->spec_fun = NULL;
  Data->pShop = NULL;
  Data->area = NULL;
  Data->player_name = str_dup ("no name");
  Data->short_descr = str_dup ("(no short description)");
  Data->long_descr = str_dup ("(no long description)" NEWLINE);
  Data->description = &str_empty[0];
  Data->vnum = 0;
  Data->count = 0;
  Data->kills = 0;
  Data->deaths = 0;
  Data->sex = SEX_NEUTRAL;
  Data->level = 0;
  Data->act = ACT_IS_NPC;
  Data->affected_by = 0;
  Data->alignment = 0;
  Data->hitroll = 0;
  Data->race = default_race;
  Data->form = 0;
  Data->parts = 0;
  Data->imm_flags = 0;
  Data->res_flags = 0;
  Data->vuln_flags = 0;
  Data->material = str_dup ("unknown");
  Data->off_flags = 0;
  Data->size = SIZE_MEDIUM;
  for (i = 0; i < MAX_AC; i++)
    Data->ac[i] = 0;
  for (i = 0; i < DICE_MAX; i++)
    {
      Data->hit[i] = 0;
      Data->mana[i] = 0;
      Data->damage[i] = 0;
    }
  Data->start_pos = POS_STANDING;
  Data->default_pos = POS_STANDING;
  Data->wealth = 0;
  Data->new_format = true;
  Validate (Data);
  return Data;
}

void
free_char_index (CharIndex * Data)
{
  ProgList *mp;

  if (!IsValid (Data))
    return;

  free_string (Data->player_name);
  free_string (Data->short_descr);
  free_string (Data->long_descr);
  free_string (Data->description);

  for (mp = Data->mprog_first; mp; mp = mp->next)
    free_prog_list (mp);

  if (Data->pShop)
    free_shop (Data->pShop);
  top_char_index--;
  Invalidate (Data);
  LinkSingle (Data, char_index_free, next);
  top_char_index_free++;
}

ProgCode *
new_mprog (void)
{
  ProgCode *Data;

  if (mprog_free != NULL)
    {
      Data = mprog_free;
      mprog_free = mprog_free->next;
      top_mprog_free--;
    }
  else
    {
      Data = (ProgCode *) alloc_perm (sizeof (ProgCode));
    }
  *Data = mprog_zero;
  top_mprog++;
  Data->next = Data->prev = NULL;
  Data->vnum = 0;
  Data->code = &str_empty[0];
  Data->area = NULL;
  Validate (Data);
  return Data;
}

void
free_mprog (ProgCode * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->code);
  top_mprog--;
  Invalidate (Data);
  LinkSingle (Data, mprog_free, next);
  top_mprog_free++;
}

ProgCode *
new_oprog (void)
{
  ProgCode *Data;

  if (oprog_free != NULL)
    {
      Data = oprog_free;
      oprog_free = oprog_free->next;
      top_oprog_free--;
    }
  else
    {
      Data = (ProgCode *) alloc_perm (sizeof (ProgCode));
    }
  *Data = oprog_zero;
  top_oprog++;
  Data->next = Data->prev = NULL;
  Data->vnum = 0;
  Data->code = &str_empty[0];
  Data->area = NULL;
  Validate (Data);
  return Data;
}

void
free_oprog (ProgCode * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->code);
  top_oprog--;
  Invalidate (Data);
  LinkSingle (Data, oprog_free, next);
  top_oprog_free++;
}

ProgCode *
new_rprog (void)
{
  ProgCode *Data;

  if (rprog_free != NULL)
    {
      Data = rprog_free;
      rprog_free = rprog_free->next;
      top_rprog_free--;
    }
  else
    {
      Data = (ProgCode *) alloc_perm (sizeof (ProgCode));
    }
  *Data = rprog_zero;
  top_rprog++;
  Data->next = Data->prev = NULL;
  Data->vnum = 0;
  Data->code = &str_empty[0];
  Data->area = NULL;
  Validate (Data);
  return Data;
}

void
free_rprog (ProgCode * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->code);
  top_rprog--;
  Invalidate (Data);
  LinkSingle (Data, rprog_free, next);
  top_rprog_free++;
}

GqData *
new_gqlist (void)
{
  GqData *Data;

  if (gqlist_free != NULL)
    {
      Data = gqlist_free;
      gqlist_free = gqlist_free->next;
      top_gqlist_free--;
    }
  else
    {
      Data = (GqData *) alloc_perm (sizeof (GqData));
    }
  *Data = gqlist_zero;
  top_gqlist++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_gqlist (GqData * Data)
{
  if (!IsValid (Data))
    return;

  if (Data->gq_mobs)
    free_mem (Data->gq_mobs);
  if (Data->ch)
    Data->ch->gquest = NULL;
  top_gqlist--;
  Invalidate (Data);
  LinkSingle (Data, gqlist_free, next);
  top_gqlist_free++;
}

WarData *
new_warlist (void)
{
  WarData *Data;

  if (warlist_free != NULL)
    {
      Data = warlist_free;
      warlist_free = warlist_free->next;
      top_warlist_free--;
    }
  else
    {
      Data = (WarData *) alloc_perm (sizeof (WarData));
    }
  *Data = warlist_zero;
  top_warlist++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_warlist (WarData * Data)
{
  if (!IsValid (Data))
    return;

  if (Data->ch)
    {
      if (!IsNPC (Data->ch))
	Data->ch->war = NULL;
      else
	extract_char (Data->ch, true);
    }
  top_warlist--;
  Invalidate (Data);
  LinkSingle (Data, warlist_free, next);
  top_warlist_free++;
}

DisabledData *
new_disabled (void)
{
  DisabledData *Data;

  if (disabled_free != NULL)
    {
      Data = disabled_free;
      disabled_free = disabled_free->next;
      top_disabled_free--;
    }
  else
    {
      Data = (DisabledData *) alloc_perm (sizeof (DisabledData));
    }
  *Data = disabled_zero;
  top_disabled++;
  Data->next = Data->prev = NULL;
  Data->disabled_by = &str_empty[0];
  Data->disabled_for = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_disabled (DisabledData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->disabled_by);
  free_string (Data->disabled_for);
  top_disabled--;
  Invalidate (Data);
  LinkSingle (Data, disabled_free, next);
  top_disabled_free++;
}

#ifdef STFILEIO

stFile *
new_stfile (void)
{
  stFile *Data;

  if (stfile_free != NULL)
    {
      Data = stfile_free;
      stfile_free = stfile_free->next;
      top_stfile_free--;
    }
  else
    {
      Data = (stFile *) alloc_perm (sizeof (stFile));
    }
  *Data = stfile_zero;
  top_stfile++;
  Data->next = Data->prev = NULL;
  Data->str = NULL;
  Data->file = &str_empty[0];
#ifndef WIN32

  Data->temp = &str_empty[0];
#endif

  Data->stream = NULL;
  Data->mode = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_stfile (stFile * Data)
{
  if (!IsValid (Data))
    return;
#ifndef WIN32

  free_string (Data->temp);
#endif

  free_string (Data->mode);
  if (Data->str != NULL)
    free_mem (Data->str);
  if (Data->stream)
    file_close (Data->stream);
  free_string (Data->file);
  top_stfile--;
  Invalidate (Data);
  LinkSingle (Data, stfile_free, next);
  top_stfile_free++;
}

#endif


void
free_note (NoteData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->sender);
  free_string (Data->to_list);
  free_string (Data->subject);
  free_string (Data->date);
  free_string (Data->text);
  top_note--;
  Invalidate (Data);
  LinkSingle (Data, note_free, next);
  top_note_free++;
}


NoteData *
new_note (void)
{
  NoteData *Data;

  if (note_free != NULL)
    {
      Data = note_free;
      note_free = note_free->next;
      top_note_free--;
    }
  else
    {
      Data = (NoteData *) alloc_perm (sizeof (NoteData));
    }
  *Data = note_zero;
  top_note++;
  Data->next = Data->prev = NULL;
  Data->sender = NULL;
  Data->expire = 0;
  Data->to_list = NULL;
  Data->subject = NULL;
  Data->date = NULL;
  Data->date_stamp = 0;
  Data->text = NULL;
  Validate (Data);
  return Data;
}

#ifndef DISABLE_WEBSRV
WebDescriptor *
new_webdesc (void)
{
  WebDescriptor *Data;

  if (webdesc_free != NULL)
    {
      Data = webdesc_free;
      webdesc_free = webdesc_free->next;
      top_webdesc_free--;
    }
  else
    {
      Data = (WebDescriptor *) alloc_perm (sizeof (WebDescriptor));
    }
  *Data = webdesc_zero;
  top_webdesc++;
  Data->next = Data->prev = NULL;
  Data->request[0] = NUL;
  Validate (Data);
  return Data;
}

void
free_webdesc (WebDescriptor * Data)
{
  if (!IsValid (Data))
    return;

  Data->request[0] = NUL;
  top_webdesc--;
  Invalidate (Data);
  LinkSingle (Data, webdesc_free, next);
  top_webdesc_free++;
}
#endif

MspData *
new_msp (void)
{
  MspData *Data;

  if (msp_free != NULL)
    {
      Data = msp_free;
      msp_free = msp_free->next;
      top_msp_free--;
    }
  else
    {
      Data = (MspData *) alloc_perm (sizeof (MspData));
    }
  *Data = msp_zero;
  top_msp++;
  Data->next = Data->prev = NULL;
  Data->file = &str_empty[0];
  Data->url = &str_empty[0];
  Data->type = MSP_NONE;
  Data->volume = 100;
  Data->loop = 1;
  Data->priority = 50;
  Data->restart = false;
  Validate (Data);
  return Data;
}

void
free_msp (MspData * Data)
{
  if (!IsValid (Data))
    return;

  free_string (Data->file);
  free_string (Data->url);
  top_msp--;
  Invalidate (Data);
  LinkSingle (Data, msp_free, next);
  top_msp_free++;
}

Column *
new_column (void)
{
  Column *Data;

  if (column_free != NULL)
    {
      Data = column_free;
      column_free = column_free->next;
      top_column_free--;
    }
  else
    {
      Data = (Column *) alloc_perm (sizeof (Column));
    }
  *Data = column_zero;
  top_column++;
  Data->next = Data->prev = NULL;
  Validate (Data);
  return Data;
}

void
free_column (Column * Data)
{
  if (!IsValid (Data))
    return;

  top_column--;
  Invalidate (Data);
  LinkSingle (Data, column_free, next);
  top_column_free++;
}

NameProfile *
new_name_profile (void)
{
  NameProfile *Data;
  int i;

  if (name_profile_free != NULL)
    {
      Data = name_profile_free;
      name_profile_free = name_profile_free->next;
      top_name_profile_free--;
    }
  else
    {
      Data = (NameProfile *) alloc_perm (sizeof (NameProfile));
    }
  *Data = name_profile_zero;
  top_name_profile++;
  Data->next = Data->prev = NULL;
  Data->title = &str_empty[0];
  for (i = 0; i < MAX_PARTS; i++)
    {
      Data->part[i] = &str_empty[0];
      Data->part_count[i] = 0;
    }
  Validate (Data);
  return Data;
}

void
free_name_profile (NameProfile * Data)
{
  int i;

  if (!IsValid (Data))
    return;

  top_name_profile--;
  free_string (Data->title);
  for (i = 0; i < MAX_PARTS; i++)
    free_string (Data->part[i]);
  Invalidate (Data);
  LinkSingle (Data, name_profile_free, next);
  top_name_profile_free++;
}

NameList *
new_namelist (void)
{
  NameList *Data;

  if (namelist_free != NULL)
    {
      Data = namelist_free;
      namelist_free = namelist_free->next;
      top_namelist_free--;
    }
  else
    {
      Data = (NameList *) alloc_perm (sizeof (NameList));
    }
  *Data = namelist_zero;
  top_namelist++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_namelist (NameList * Data)
{
  if (!IsValid (Data))
    return;

  top_namelist--;
  free_string (Data->name);
  Invalidate (Data);
  LinkSingle (Data, namelist_free, next);
  top_namelist_free++;
}



WizData *
new_wiz (void)
{
  WizData *Data;

  if (wiz_free != NULL)
    {
      Data = wiz_free;
      wiz_free = wiz_free->next;
      top_wiz_free--;
    }
  else
    {
      Data = (WizData *) alloc_perm (sizeof (WizData));
    }
  *Data = wiz_zero;
  top_wiz++;
  Data->next = Data->prev = NULL;
  Data->name = &str_empty[0];
  Data->email = &str_empty[0];
  Data->jobs = &str_empty[0];
  Validate (Data);
  return Data;
}

void
free_wiz (WizData * Data)
{
  if (!IsValid (Data))
    return;

  top_wiz--;
  free_string (Data->name);
  free_string (Data->email);
  free_string (Data->jobs);
  Invalidate (Data);
  LinkSingle (Data, wiz_free, next);
  top_wiz_free++;
}
