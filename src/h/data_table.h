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

#ifndef __DATA_TABLE_H_
#define __DATA_TABLE_H_    	1

#define DATA_LIST       1
#define DATA_NAME       2
#define DATA_STAT       3

Proto (void load_struct, (FileData *, void *, DataTable *, void *));
Proto (void save_struct, (FileData *, void *, DataTable *, void *));
Proto (void show_struct, (CharData *, Buffer *, void *,
			  DataTable *, void *, int));

Proto (bool edit_struct,
       (CharData *, const char *, void *, DataTable *, void *));
Proto (bool set_struct,
       (CharData *, const char *, void *, DataTable *, void *));

Proto (void show_set_commands, (CharData *, DataTable *));


struct datasave_type
{
  TableSave_F *fun;
  const char *name;
};

EXTERN const struct datasave_type datasave_table[];


#define DTE(field, name, var, arg1, arg2, olc, arg3) \
    {#var, FIELD_##field, (void *)&name##_zero.var, (void *)(arg1), (void *)(arg2), olced_##olc, (void *)(arg3)}

#define DTE_SHOW(field, name, var, arg1, arg2, arg3) \
    {#var, FIELD_##field, (void *)&name##_zero.var, (void *)(arg1), (void *)(arg2), NULL, (void *)(arg3)}

#define DTE_OLC(cmd, name, var, arg1, arg2, olc, arg3) \
    {cmd, FIELD_INUTIL, (void *) &name##_zero.var, (void *)(arg1), NULL, olced_##olc, (void *)(arg3)}

#define DTE_OLC2(cmd, arg1, olc, arg3) \
    {cmd, FIELD_INUTIL, NULL, (void *)(arg1), NULL, olced_##olc, (void *)(arg3)}

#define DTE_LIST(field, name, var, list, arg2, arg3) \
    {#list, FIELD_##field, (void *) &name##_zero.var, (void *) DATA_LIST, (void *) (arg2), NULL, \
        (void *) (arg3)}, \
    {#list, FIELD_INUTIL, (void *) &name##_zero.var, (void *) FIELD_##field, (void *) (arg2), olced_##list, (void *)(arg3)}

#define DTE_LIST2(field, name, var, list, arg2, arg3) \
    {#list, FIELD_##field, (void *) &name##_zero.var, (void *) DATA_LIST, (void *) (arg2), NULL, \
        (void *) (arg3)}, \
    {"add"#list, FIELD_INUTIL, (void *) &name##_zero.var, (void *) FIELD_##field, (void *) (arg2), olced_add##list, (void *)(arg3)}, \
    {"del"#list, FIELD_INUTIL, (void *) &name##_zero.var, (void *) FIELD_##field, (void *) (arg2), olced_del##list, (void *)(arg3)}

#define DTE_END  {NULL, (field_t)-1, NULL, NULL, NULL, NULL, NULL}



#define rwgetdata(var, type, temp, base, data) \
    ((var) = (type *) ((int)(temp) - (int)(base) + (int)(data)))


#define rw_table(action, file, type, name) \
do { \
    	int i = 0; \
    	switch(action) { \
    	    	case act_read: { \
            FileData *fp; \
            const char *word; \
            if ((fp = f_open(file, "r")) == NULL) { \
                bugf("Unable to open %s for reading.", file); \
                if((fp = f_open(file, "w")) != NULL) { \
                    f_printf(fp, "#!" LF); \
                    f_close(fp); \
                } \
                break; \
            } \
            top_##name = read_number(fp); \
            alloc_mem(name##_table, type, top_##name + 1); \
            if (!name##_table) { \
                bugf("Error! " #name "_table == NULL, top_" #name " : %d", top_##name); \
                break; \
            } \
            log_string("Loading " #name " data..."); \
            for (;;) { \
                word = f_eof(fp) ? "#!" : read_word(fp); \
                if (!str_cmp(word, "#!")) \
                    break; \
                if (str_cmp(word, "#" #name)) { \
                    bugf("word doesn't exist (%s)", word); \
                    break; \
                } \
                if (i >= top_##name) { \
                    bugf("%d greater than top_" #name " %d", i, top_##name); \
                    break; \
                } \
                name##_table[i] = name##_zero; \
                load_struct(fp, &name##_zero, name##_data_table, &name##_table[i++]); \
            } \
            f_close(fp); \
    	    	} \
    	    	break; \
    	    	case act_write: { \
    	    	    	FileData *fp; \
    	    	    	int i; \
    	    	    	if ((fp = f_open(file, "w")) == NULL) { \
    	    	    	    	log_error(file); \
    	    	    	    	break; \
    	    	    	} \
    	    	    	f_printf(fp, "%d" LF, top_##name); \
						log_string("Saving " #name " data..."); \
    	    	    	for (i = 0; i < top_##name; ++i) { \
    	    	    	    	f_printf(fp, LF "#%s" LF, strupper(#name)); \
    	    	    	    	save_struct(fp, &name##_zero, name##_data_table, &name##_table[i]); \
    	    	    	    	f_printf(fp, "#" END_MARK LF); \
    	    	    	} \
    	    	    	f_printf(fp, "#!" LF ); \
    	    	    	f_close(fp); \
    	    	}\
    	    	break;\
    	    	default: \
    	    	break; \
    	} \
} while(0)


#define rw_list(action, file, type, name) \
do { \
    	switch(action) {\
    	    	case act_read: { \
            FileData *fp; \
            type *data; \
            const char *word; \
            if ((fp = f_open(file, "r")) == NULL) { \
                bugf("Unable to open %s for reading.", file); \
                if((fp = f_open(file, "w")) != NULL) { \
                    f_printf(fp, "#!" LF); \
                    f_close(fp); \
                } \
                break; \
            } \
            log_string("Loading " #name " data..."); \
            for (;;) { \
                word = f_eof(fp) ? "#!" : read_word(fp); \
                if (!str_cmp(word, "#!")) \
                    break; \
                if (str_cmp(word, "#" #name)) { \
                    bugf("word doesn't exist (%s)", word); \
                    break; \
                } \
                data = new_##name(); \
                load_struct(fp, &name##_zero, name##_data_table, data); \
                if(name##_data_table == social_data_table) \
                    add_social((SocialData *)data); \
                else if(name##_data_table == cmd_data_table) \
                    add_command((CmdData *)data); \
                else if(name##_data_table == help_data_table) \
                    add_help((HelpData *)data); \
                else \
                    Link(data, name, next, prev); \
            } \
            f_close(fp); \
    	    	} \
    	    	break; \
    	    	case act_write: { \
    	    	    	FileData *fp; \
    	    	    	type *data; \
    	    	    	if ((fp = f_open(file, "w")) == NULL) { \
    	    	    	    	log_error(file); \
    	    	    	    	break; \
    	    	    	} \
						log_string("Saving " #name " data..."); \
    	    	    	for (data = name##_first; data != NULL; data = data->next) { \
    	    	    	    	f_printf(fp, LF "#%s" LF, strupper(#name)); \
    	    	    	    	save_struct(fp, &name##_zero, name##_data_table, data); \
    	    	    	    	f_printf(fp, "#" END_MARK LF); \
    	    	    	} \
    	    	    	f_printf(fp, "#!" LF ); \
    	    	    	f_close(fp); \
    	    	} \
    	    	break; \
    	    	default: \
    	    	break; \
    	} \
} while(0)


#define rw_sublist(action, file, type, name, sub) \
do { \
    	switch(action) {\
    	    	case act_read: { \
            FileData *fp; \
            type *data; \
            const char *word; \
            if ((fp = f_open(file, "r")) == NULL) { \
                bugf("Unable to open %s for reading.", file); \
                if((fp = f_open(file, "w")) != NULL) { \
                    f_printf(fp, "#!" LF); \
                    f_close(fp); \
                } \
                break; \
            } \
            for (;;) { \
                word = f_eof(fp) ? "#!" : read_word(fp); \
                if (!str_cmp(word, "#!")) \
                    break; \
                if (str_cmp(word, "#" #name)) { \
                    bugf("word doesn't exist (%s)", word); \
                    break; \
                } \
                data = new_##sub(); \
                load_struct(fp, &sub##_zero, sub##_data_table, data); \
                if(sub##_data_table == social_data_table) \
                    add_social((SocialData *)data); \
                else if(sub##_data_table == cmd_data_table) \
                    add_command((CmdData *)data); \
                else \
                    Link(data, name->sub, next, prev); \
            } \
            f_close(fp); \
    	    	} \
    	    	break; \
    	    	case act_write: { \
    	    	    	FileData *fp; \
    	    	    	type *data; \
    	    	    	if ((fp = f_open(file, "w")) == NULL) { \
    	    	    	    	log_error(file); \
    	    	    	    	break; \
    	    	    	} \
    	    	    	for (data = name->sub##_first; data != NULL; data = data->next) { \
    	    	    	    	f_printf(fp, LF "#%s" LF, strupper(#name)); \
    	    	    	    	save_struct(fp, &sub##_zero, sub##_data_table, data); \
    	    	    	    	f_printf(fp, "#" END_MARK LF); \
    	    	    	} \
    	    	    	f_printf(fp, "#!" LF ); \
    	    	    	f_close(fp); \
    	    	} \
    	    	break; \
    	    	default: \
    	    	break; \
    	} \
} while(0)


#define rw_single(action, file, name) \
do { \
    	switch(action) { \
    	    	case act_read: { \
            FileData *fp; \
            const char *word; \
            if ((fp = f_open(file, "r")) == NULL) { \
                bugf("Could not open file %s for reading.", file); \
                if((fp = f_open(file, "w")) != NULL) { \
                    f_printf(fp, "#!" LF); \
                    f_close(fp); \
                } \
                break; \
            } \
            log_string("Loading " #name " data..."); \
            for (;;) { \
                word = f_eof(fp) ? "#!" : read_word(fp); \
                if (!str_cmp(word, "#!")) \
                    break; \
                if (str_cmp(word, "#" #name)) { \
                    bugf("word doesn't exist (%s)", word); \
                    break; \
                } \
                load_struct(fp, &name##_zero, name##_data_table, &name##_info); \
            } \
            f_close(fp); \
    	    	}\
    	    	break; \
    	    	case act_write: { \
    	    	    	FileData *fp; \
    	    	    	if (!(fp = f_open(file, "w"))) { \
    	    	    	    	log_error(file); \
    	    	    	    	break; \
    	    	    	} \
            log_string("Saving " #name " data..."); \
    	    	    	f_printf(fp, LF "#%s" LF, strupper(#name)); \
    	    	    	save_struct(fp, &name##_zero, name##_data_table, &name##_info); \
    	    	    	f_printf(fp, "#" END_MARK LF); \
    	    	    	f_printf(fp, "#!" LF ); \
    	    	    	f_close(fp); \
    	    	} \
    	    	break; \
    	    	default: \
    	    	break; \
    	} \
} while(0)


#endif
