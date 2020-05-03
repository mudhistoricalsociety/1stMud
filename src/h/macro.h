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

#ifndef __MACRO_H_
#define __MACRO_H_    	1

#ifndef PARAMS
# if defined PROTOTYPES || defined WIN32
#  define PARAMS(protos) protos
# else
#  define PARAMS(protos) ()
#  define __attribute__(x)
# endif
#endif

#ifndef __attribute__
#  define __attribute__(x)
#endif

#define Proto(x,y)     	    	EXTERN x PARAMS(y)
#define ProtoF(x,y,a,b) EXTERN x PARAMS(y) __attribute__((format(printf, a, b)))
#define ProtoNoR(x, y)    	EXTERN x PARAMS(y) __attribute__((noreturn))

#define alloc_mem(result, type, number) \
do { \
    if (!((result) = (type *) calloc ((number), sizeof(type)))) { \
        logf( "Malloc failure @ %s:%d", __FILE__, __LINE__ ); \
        abort(); \
    } \
} while(0)

#define realloc_mem(result,type,number) \
do { \
    if (!((result) = (type *) realloc ((void *)(result), sizeof(type) * (number)))) { \
        logf( "Realloc failure @ %s:%d", __FILE__, __LINE__ ); \
        abort(); \
    } \
} while(0)

#define free_mem(point) \
do { \
  if (!(point)) \
    bugf("Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
  else free((void *)(point)); \
    (point) = NULL; \
} while(0)


#define IsValid(data)                   ((data) != NULL && (data)->valid)
#define Validate(data)                  ((data)->valid = true)
#define Invalidate(data)                ((data)->valid = false)
#define Min(a, b)                               ((a) < (b) ? (a) : (b))
#define Max(a, b)                               ((a) > (b) ? (a) : (b))
#define Diff(a, b)                      ((a > b) ? (a - b) : (b - a))

#define Range(a, b, c)                  ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define Percent(a, b)                   Min(100 * (a) / (b), 100)

#define IsSet(flag, bit)                ((flag) & (bit))
#define SetBit(var, bit)                ((var) |= (bit))
#define RemBit(var, bit)                ((var) &= ~(bit))
#define DiffBit(flag, bit)              ((flag) & ~(bit))
#define ToggleBit(var, bit)             ((var) ^= (bit))
#define GetUnset(flag1,flag2)           (~(flag1)&((flag1)|(flag2)))
#define GetDiff(a,b)					(~((~a)|(b)))
#define DiffBit(flag, bit)          ((flag) & ~(bit))
#define RepBit(flag, a, b)          (((flag) & ~(a)) | (b))


#define StrIsSet(var, bit)              ((((char *)(var))[((bit)/8)]) &   ((1<<((bit)%8))))
#define StrSetBit(var, bit)             ((((char *)(var))[((bit)/8)]) |=  ((1<<((bit)%8))))
#define StrRemBit(var, bit)             ((((char *)(var))[((bit)/8)]) &= ~((1<<((bit)%8))))
#define StrToggleBit(var, bit)          ((((char *)(var))[((bit)/8)]) ^=  ((1<<((bit)%8))))

#define NullStr(str)                    (!(str) || !*(str))
#define Entre(min,num,max)              ( ((min) < (num)) && ((num) < (max)) )

#define CheckPos(a, b, c) \
do { \
        (a) = (b); \
        if ( (a) < 0 ) \
                bug( "CheckPos : " c " == < 0"); \
} while(0)

#define SetStat(ch, pos, val)           ((ch)->pcdata->gamestat[pos] = val)
#define GetStat(ch, pos)                ((ch)->pcdata->gamestat[pos])
#define AddStat(ch, pos, val)           ((ch)->pcdata->gamestat[pos] += val)

#define ValidStance(st)                 ((st) > STANCE_NONE && (st) < STANCE_CURRENT)
#define GetStance(ch, st)               ((ch)->stance[(st)])
#define InStance(ch, sn)                (GetStance((ch), STANCE_CURRENT) == (sn))
#define SetStance(ch, pos, st)          ((ch)->stance[(pos)] = (st))

#define Str(x)                          #x
#define Stringify(x)                    Str(x)

#define UnlinkSingle(point,type,list,next) \
do { \
        if (list == point) \
                list = point->next; \
        else { \
                type *prev; \
                for (prev = list; prev != NULL; prev = prev->next) { \
                        if (prev->next == point) { \
                                prev->next = point->next; \
                                break; \
                        } \
                } \
                if (prev == NULL) \
                        bugf (#point " not found in " #list "."); \
        } \
} while(0)

#define LinkSingle(point,list,next) \
do { \
                point->next = list; \
                list = point; \
} \
while (0)

#define LinkLast(type,point,list,next) \
do { \
    type *tmp; \
    if((tmp = list) == NULL) { \
                point->next = list; \
                list = point; \
        break; \
    } \
    for(; tmp; tmp = tmp->next) { \
        if(!tmp->next) { \
            tmp->next = point; \
            point->next = NULL; \
            break; \
        } \
    } \
} \
while (0)

#define Link(link, name, next, prev) \
do { \
   if ( !(name##_first) ) { \
      (name##_first) = (link); \
      (name##_last) = (link); \
   } \
   else \
      (name##_last)->next = (link); \
   (link)->next = NULL; \
   if (name##_first == link) \
      (link)->prev = NULL; \
   else \
      (link)->prev = (name##_last); \
   (name##_last) = (link); \
} while(0)

#define Insert(link, insert, name, next, prev) \
do { \
   (link)->prev = (insert)->prev; \
   if ( !(insert)->prev ) \
      (name##_first) = (link); \
   else \
      (insert)->prev->next = (link); \
   (insert)->prev = (link); \
   (link)->next = (insert); \
} while(0)

#define UnLink(link, name, next, prev) \
do { \
        if ( !(link)->prev ) { \
                (name##_first) = (link)->next; \
                if ((name##_first)) \
                        (name##_first)->prev = NULL; \
        } \
        else { \
         (link)->prev->next = (link)->next; \
        } \
        if ( !(link)->next ) { \
        (name##_last) = (link)->prev; \
                if ((name##_last)) \
                        (name##_last)->next = NULL; \
        } \
        else { \
         (link)->next->prev = (link)->prev; \
        } \
} while(0)

#define CheckLinks(type, name, next, prev) \
do { \
  type *ptr, *pptr = NULL; \
  if ( !(name##_first) && !(name##_last) ) \
    break; \
  if ( !(name##_first) ) { \
    bugf( "CheckLinks: name##_last with NULL name##_first!  %s.", \
        #name##_first ); \
    for ( ptr = (name##_last); ptr->prev; ptr = ptr->prev ); \
    (name##_first) = ptr; \
  } \
  else if ( !(name##_last) ) { \
    bugf( "CheckLinks: name##_first with NULL name##_last!  %s.", \
        #name##_first ); \
    for ( ptr = (name##_first); ptr->next; ptr = ptr->next ); \
    (name##_last) = ptr; \
  } \
  if ( (name##_first) ) { \
    for ( ptr = (name##_first); ptr; ptr = ptr->next ) { \
      if ( ptr->prev != pptr ) { \
        bugf( "CheckLinks(%s): %p:->prev != %p.  Fixing.", \
            #name##_first, ptr, pptr ); \
        ptr->prev = pptr; \
      } \
      if ( ptr->prev && ptr->prev->next != ptr ) { \
        bugf( "CheckLinks(%s): %p:->prev->next != %p.  Fixing.", \
            #name##_first, ptr, ptr ); \
        ptr->prev->next = ptr; \
      } \
      pptr = ptr; \
    } \
    pptr = NULL; \
  } \
  if ( (name##_last) ) { \
    for ( ptr = (name##_last); ptr; ptr = ptr->prev ) { \
      if ( ptr->next != pptr ) { \
        bugf( "CheckLinks (%s): %p:->next != %p.  Fixing.", \
            #name##_first, ptr, pptr ); \
        ptr->next = pptr; \
      } \
      if ( ptr->next && ptr->next->prev != ptr ) { \
        bugf( "CheckLinks(%s): %p:->next->prev != %p.  Fixing.", \
            #name##_first, ptr, ptr ); \
        ptr->next->prev = ptr; \
      } \
      pptr = ptr; \
    } \
  } \
} while(0)

#define NUL '\0'

#define GetStr(str, msg)                (NullStr(str) ? (msg) : (str))


#define IsNPC(ch)                       (IsSet((ch)->act, ACT_IS_NPC))
#define IsImmortal(ch)                  (get_trust(ch) >= LEVEL_IMMORTAL)
#define IsPlayer(ch)                    (!IsNPC(ch) && !IsImmortal(ch))
#define IsHero(ch)                      (get_trust(ch) >= LEVEL_HERO)
#define IsTrusted(ch,level)             (get_trust((ch)) >= (level))
#define IsAffected(ch, sn)              (IsSet((ch)->affected_by, (sn)))
#define IsDrunk(ch)						(!IsNPC(ch) && (ch)->pcdata->condition[COND_DRUNK] > 10)
#define IsGood(ch)                      (ch->alignment >= 350)
#define IsEvil(ch)                      (ch->alignment <= -350)
#define IsNeutral(ch)                   (!IsGood(ch) && !IsEvil(ch))
#define IsAway(ch)						(IsSet((ch)->comm,COMM_AFK))
#define IsAwake(ch)                     (ch->position > POS_SLEEPING)
#define GetArmor(ch,type)               ((ch)->armor[type] + ( IsAwake(ch) \
                                        ? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define TrueSex(ch)                     (!IsNPC(ch) ? (ch)->pcdata->true_sex : (ch)->sex)

#define GetHitroll(ch)                  ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GetDamroll(ch)                  ((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IsOutside(ch)                   (!IsSet((ch)->in_room->room_flags, ROOM_INDOORS))
#define InWar(ch)                       (ch->war != NULL && IsSet((ch)->in_room->room_flags, ROOM_ARENA))

#define WaitState(ch, npulse)           ((ch)->wait = Max((ch)->wait, (npulse)))
#define DazeState(ch, npulse)           ((ch)->daze = Max((ch)->daze, (npulse)))
#define get_carry_weight(ch)            ((ch)->carry_weight + (int)((ch)->silver/10 + \
                                                (ch)->gold * 2 / 5))

#define act(format,ch,arg1,arg2,type) \
        act_new((format),(ch),(arg1),(arg2),(type),POS_RESTING)

#define log_wiznet(fmt, ch, obj, flag, level) \
    new_wiznet((ch), (obj),(flag),true,(level), (fmt))

#define wiznet(string, ch, obj, flag, logit, level) \
    new_wiznet((ch),(obj),(flag),(logit),(level),(string))

#define HasTriggerMob(ch,trig)          (IsSet((ch)->pIndexData->mprog_flags,(trig)))
#define HasTriggerObj(obj,trig)         (IsSet((obj)->pIndexData->oprog_flags,(trig)))
#define HasTriggerRoom(room,trig)       (IsSet((room)->rprog_flags,(trig)))

#define IsSwitched( ch )                ( ch->desc && ch->desc->original )
#define IsBuilder(ch, Area)             ( !IsNPC(ch) && !IsSwitched( ch ) && \
                                        ( ch->pcdata->security >= Area->security \
                                        || strstr( Area->builders, ch->name ) \
                                        || strstr( Area->builders, "All" ) ) )

#define CH(descriptor)                  ((descriptor)->original ? \
                                                (descriptor)->original : (descriptor)->character)

#define IsRemort(ch)                    ((ch)->Class[CLASS_COUNT] > 1)
#define GetRemort(ch)                   (Max(0, (ch)->Class[CLASS_COUNT] - 1))
#define ClassName(ch, i)                (class_table[(i)].name[GetRemort(ch)])
#define LinkDead(ch)    	    	    	        (!IsNPC(ch) && (ch)->desc == NULL)
#define Gquester(ch)                    (gquest_info.running != GQUEST_OFF && (ch)->gquest)

#define StrEdKey(ch)                    (IsNPC(ch) ? '.' : (ch)->pcdata->str_ed_key)

#define GetTzone(ch)                    (IsNPC(ch) ? -1 : (ch)->pcdata->timezone)

#define AreaClan(area)                  ((area)->clan)
#define CharClan(ch)                    (IsNPC(ch) ? AreaClan((ch)->pIndexData->area) : (ch)->pcdata->clan)
#define ObjClan(obj)                    (AreaClan((obj)->pIndexData->area))
#define RoomClan(room)                  (AreaClan((room)->area))

#define IsPortal(d)                     ((d) && IsSet((d)->desc_flags, DESC_PORTAL))
#define IsFireCl(d)                     ((d) && IsSet((d)->desc_flags, DESC_IMP))
#define IsMXP(d)                        ((d) && IsSet((d)->desc_flags, DESC_MXP) \
                                                && ((d)->mxp.mxp_ver * 10) >= 4)
#define IsMSP(d)                        ((d) && IsSet((d)->desc_flags, DESC_MSP))
#define IsCompressed(d)                 (d && d->out_compress)
#define IsPueblo(d)                     ((d) && IsSet((d)->desc_flags, DESC_PUEBLO))

#define ScrWidth(d)                     (!(d) ? DEFAULT_SCR_WIDTH - 2 : (d)->scr_width)
#define ScrHeight(d)                    (!(d) ? DEFAULT_SCR_HEIGHT - 2 : (d)->scr_height)

#define VT100_SET(ch, fl)    	((ch) && (ch)->pcdata && \
    	    	    	    	    	    	    	    	    	    	IsSet((ch)->pcdata->vt100, VT100_##fl))

#define ObjFlag(obj, flag)      (IsSet((obj)->extra_flags, (flag)))
#define PlrFlag(ch, flag)       (!IsNPC(ch) && IsSet((ch)->act, (flag)))
#define DescrFlag(d, flag)      (IsSet((d)->desc_flags, (flag)))
#define MobFlag(ch, flag)       (IsNPC(ch) && IsSet((ch)->act, (flag)))
#define CommFlag(ch, flag)      (IsSet((ch)->comm, (flag)))
#define RoomFlag(loc, flag)     (IsSet((loc)->room_flags, (flag)))
#define ExitFlag(ex, flag)      (IsSet((ex)->exit_info, (flag)))
#define AreaFlag(loc, flag)     (IsSet((loc)->area_flags, (flag)))
#define ClanFlag(clan, flag)    (IsSet((clan)->clan_flags, (flag)))
#define RaceFlag(race, flag)    (IsSet((race)->race_flags, (flag)))
#define SkillFlag(skill, flag)  (IsSet(skill_table[(skill)].skill_flags, (flag)))
#define GroupFlag(group, flag)  (IsSet(group_table[(group)].group_flags, (flag)))
#define MudFlag(flag)           (IsSet(mud_info.mud_flags, (flag)))
#define VideoFlag(ch, flag)		((ch) && !IsNPC(ch) && IsSet((ch)->pcdata->video, (flag)))

#define TouchArea(area)         (SetBit((area)->area_flags, AREA_CHANGED))
#define QuestObj(obj)           (IsSet(obj->extra_flags, ITEM_QUEST))


#define OnOff(a)                ((a) ? "{WON" : "{ROFF")
#define YesNo(a)                ((a) ? "{WYES" : "{RNO")


#define CanWear(obj, part)              (IsSet((obj)->wear_flags,  (part)))
#define IsObjStat(obj, stat)            (IsSet((obj)->extra_flags, (stat)))
#define IsWeaponStat(obj,stat)          (IsSet((obj)->value[4],(stat)))
#define WeightMult(obj)                 ((obj)->item_type == ITEM_CONTAINER ? \
                                                (obj)->value[4] : 100)

#define IsQuester(ch)                   (!IsNPC(ch) && (ch)->pcdata->quest.status != QUEST_NONE )

#define Season                          (((time_info.month / 4) > 3) ? 3 : (time_info.month / 4))

#define IsAlternate(ch,victim)          (!IsNPC(ch) && !IsNPC(victim) \
                                                && ch->desc && victim->desc \
                                                && ch != victim && !str_cmp(ch->desc->host, \
                                                victim->desc->host))

#define GetName(ch)                     (IsNPC(ch) ? (ch)->short_descr : (ch)->name)


#define Pers(ch, looker)                ( can_see( looker, (ch) ) ? \
                                                ( IsNPC(ch) ? (ch)->short_descr \
                                                : (ch)->name ) : IsImmortal(ch) ? \
                                                "an Immortal" : "someone" )

#define VnumID(ch)              (IsNPC(ch) ? (ch)->pIndexData->vnum : (ch)->id)


#define Key_Array( literal, array, max, def ) \
if ( !str_cmp( word, literal ) ) { \
    read_array(fp, array, max, def); \
    fMatch = true; \
    break; \
}

#define Key_Ignore(literal) \
if ( !str_cmp(word, literal ) ) { \
        read_to_eol(fp); \
        fMatch = true; \
        break; \
}

#define Key_Do(literal, dothis) \
if (!str_cmp(word, literal ) ) { \
    dothis; \
    read_to_eol(fp); \
    fMatch = true; \
    break; \
}

#define Key_Time( literal, field) \
if ( !str_cmp( word, literal ) ) { \
        read_time(fp, &field); \
        fMatch = true; \
        break; \
}

#define Key( literal, field, value ) \
if ( !str_cmp( word, literal ) ) { \
    field  = value; \
    read_to_eol(fp); \
    fMatch = true; \
    break; \
}

#define Key_Str( literal, field ) \
if ( !str_cmp( word, literal ) ) { \
    read_strfree(fp, &field); \
    read_to_eol(fp); \
    fMatch = true; \
    break; \
}

#define Key_SFun( literal, field, strfun ) \
if ( !str_cmp( word, literal ) ) { \
    const char *tmp = read_string(fp); \
    field = strfun(tmp); \
    free_string(tmp); \
    fMatch = true; \
    break; \
}

#define Key_StrCpy( literal, field ) \
if ( !str_cmp( word, literal ) ) { \
    const char *tmp = read_string(fp); \
    strncpy(field, tmp, sizeof(field)); \
    free_string(tmp); \
    read_to_eol(fp); \
    fMatch = true; \
    break; \
}

#define write_array(fp, name, format, array, max) \
do { \
        int tmp; \
        if(NullStr(name)) \
                f_printf(fp, "%d", max); \
        else \
                f_writef(fp, name, "%d", max); \
        for(tmp = 0; tmp < max; tmp++) \
                f_printf(fp, " " format, array[tmp]); \
        if(!NullStr(name)) \
                f_printf(fp, LF); \
} while(0)

#define read_array(fp, array, max, def) \
do { \
        int tmp = 0, tmpmax = read_number(fp); \
        while(tmp < Min(max, tmpmax)) \
                array[tmp++] = read_number(fp); \
        read_to_eol(fp); \
        while(tmp < max) \
                array[tmp++] = def; \
} while(0)

#define write_int(fp, name, format, var, def) \
do { \
    if(NullStr(name)) \
        f_printf((fp), format LF, (var)); \
    else if ((var) != (def)) \
        f_writef((fp), (name), format LF, (var)); \
} while(0)

#define copy_array(array1, array2, aMax) \
do { int a; \
    for(a = 0; a < aMax; a++) \
        array1[a] = array2[a]; \
} while(0)

#define set_array(ar, v, mx) \
do { int _a; \
     for(_a = 0; _a < mx; _a++) \
        ar[_a] = v; \
} while(0)

#define read_enum(type, fp)             (type)read_number(fp)

#define atov      atol
#define read_vnum                   (vnum_t) read_long
#define atom                        (money_t) atol
#define read_money                  (money_t) read_long

#define titlebar(ch, str)               stringf(ch, 0, Center, "-=", str)
#define titlewidth(len, str)            stringf(NULL, len, Center, "-=", str)
#define titlefill(len, fill, str)       stringf(NULL, len, Center, fill, str)
#define str_width(width, str)           stringf(NULL, width, Left, NULL, str)
#define str_align(len, align, str)      stringf(NULL, len, align, NULL, str)
#define str_fill(len, fill)             FORMATF("%-*c", len, fill)

#define help_text(txt)    	    	    	    	    	    	 ((txt)[0] == '.' ? (txt) + 1 : (txt))

#ifdef WIN32
#define vsnprintf               _vsnprintf
#define snprintf                _snprintf
#define getpid                  _getpid
#define random()                rand()
#define srandom( x )            srand( x )
#define STDERR_FILENO           2
#define S_ISREG(mode)           ((mode) & S_IFREG)
#define S_ISDIR(mode)           ((mode) & S_IFDIR)
#define strcasecmp              stricmp
#define strncasecmp             strnicmp
#define socket_error(text)      logf("%s: %s", get_winsock_error_text(0), text)
#define makedir(dir)            mkdir(dir)
#define check_errno(err)        (WSAGetLastError() == WSA##err)
#define sockerrno               WSAGetLastError()
#define popen					_popen
#define pclose					_pclose
#else
#define closesocket             close
#define socket_error            log_error
#define sockerrno               errno
#define check_errno(err)        (errno == err)
#define INVALID_SOCKET          (SOCKET)(~0)
#define SOCKET_ERROR            -1
#define makedir(dir)            mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define SOCKADDR                struct sockaddr
#ifndef HAVE_ATEXIT
#define exit(err) \
        do { \
            if(run_level == RUNLEVEL_MAIN_LOOP) \
                exit_mud(); \
            _exit(err); \
        } while(0)
#endif
#endif

#define add_text(text, to) \
do { \
    char *code = text; \
    while (*code != '\0') { \
        *to = *code++; \
        *++to = '\0'; \
    } \
} while(0)

#ifdef IN_DB_C
#define GLOBAL(str) str
#define GLOBAL_DEF(str, def) str = def
#define GLOBAL_INFO(type, str)  \
        type str##_info, str##_zero; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_STRUCT(type, str) \
        type *str##_free = NULL, str##_zero; \
        int top_##str = 0, top_##str##_free = 0; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_TABLE(type, str, table) \
        type *str##_free = NULL, str##_zero, *str##_##table; \
        int top_##str = 0, top_##str##_free = 0; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_LIST(type, str) \
        type * str##_first = NULL, * str##_last = NULL, str##_zero, *str##_free = NULL; \
        int top_##str = 0, top_##str##_free = 0; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#else
#define GLOBAL(str) EXTERN str
#define GLOBAL_DEF(str, def) EXTERN str
#define GLOBAL_INFO(type, str)  \
        EXTERN type str##_info, str##_zero; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_STRUCT(type, str) \
        EXTERN type *str##_free, str##_zero; \
        EXTERN int top_##str, top_##str##_free; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_TABLE(type, str, table) \
        EXTERN type *str##_free, str##_zero, *str##_##table; \
        EXTERN int top_##str, top_##str##_free; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#define GLOBAL_LIST(type, str) \
        EXTERN type *str##_first, *str##_last, str##_zero, *str##_free; \
        EXTERN int top_##str, top_##str##_free; \
        EXTERN TableSave_F rw_##str##_data; \
        EXTERN DataTable str##_data_table[]
#endif

#endif
