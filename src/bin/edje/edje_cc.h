#ifndef EDJE_CC_H
#define EDJE_CC_H

#include <edje_private.h>

extern Eina_Prefix *pfx;

/*
 * On Windows, if the file is not opened in binary mode,
 * read does not return the correct size, because of
 * CR / LF translation.
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif

/* logging variables */
extern int _edje_cc_log_dom ;
#define EDJE_CC_DEFAULT_LOG_COLOR EINA_COLOR_CYAN

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_edje_cc_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_edje_cc_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_edje_cc_log_dom, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_edje_cc_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_edje_cc_log_dom, __VA_ARGS__)

/* types */
typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;
typedef struct _New_Nested_Handler    New_Nested_Handler;
typedef struct _External_List         External_List;
typedef struct _External              External;
typedef struct _Code                  Code;
typedef struct _Code_Program          Code_Program;
typedef struct _SrcFile               SrcFile;
typedef struct _SrcFile_List          SrcFile_List;

typedef struct _Edje_Program_Parser                  Edje_Program_Parser;
typedef struct _Edje_Pack_Element_Parser             Edje_Pack_Element_Parser;
typedef struct _Edje_Part_Parser                     Edje_Part_Parser;
typedef struct _Edje_Part_Collection_Parser          Edje_Part_Collection_Parser;

struct _New_Object_Handler
{
   const char *type;
   void (*func)(void);
};

struct _New_Statement_Handler
{
   const char *type;
   void (*func)(void);
};

struct _New_Nested_Handler
{
   const char *type;
   const char *token;
   void (*func_push)(void);
   void (*func_pop)(void);
};

struct _External_List
{
   Eina_List *list;
};

struct _External
{
    char *name;
};

struct _Code
{
   int       l1, l2;
   char      *shared;
   char      *original;
   Eina_List *programs;
   Eina_List *vars;
   Eina_List *func;
   Eina_Bool  parsed : 1;
   Eina_Bool  is_lua : 1;
};

struct _Code_Program
{
   int        l1, l2;
   int        id;
   char      *script;
   char      *original;
};

struct _SrcFile
{
   char *name;
   char *file;
};

struct _SrcFile_List
{
   Eina_List *list;
};

struct _Edje_Program_Parser
{
   Edje_Program common;
   Eina_Bool can_override;
};

struct _Edje_Pack_Element_Parser
{
   Edje_Pack_Element common;
   Eina_Bool can_override;
};

struct _Edje_Part_Parser
{
   Edje_Part common;
   struct {
      Eina_Bool           done;
      const char         *insert_before; /* the part name for insertion in front of */
      const char         *insert_after; /* the part name for insertion behind of */
      Edje_Part_Parser   *before;
      Edje_Part_Parser   *after;
      int                 linked_prev; /* the number linked previous part for reorder */
      int                 linked_next; /* the number linked next part for reorder */
   } reorder;
   Eina_Bool can_override;
};

typedef struct Edje_Target_Group
{
   char *name;
   char **targets;
} Edje_Target_Group;

typedef struct Edje_Part_Description_Link
{
   Edje_Program *pr;
   Edje_Part_Description_Common *ed;
   Edje_Part_Parser *epp;
} Edje_Part_Description_Link;

struct _Edje_Part_Collection_Parser
{
   Edje_Part_Collection common;
   char *default_source;
   Eina_List *target_groups;
   Eina_List *links;
   Eina_Hash *link_hash;
   Eina_List *base_codes;
   Eina_Bool default_mouse_events;
   Eina_Bool inherit_only;
   Eina_Bool inherit_script : 1;
};

typedef enum
{
   EDJE_PART_ANCHOR_LINE_RELATIVE = -1,
   EDJE_PART_ANCHOR_LINE_NONE,
   EDJE_PART_ANCHOR_LINE_TOP,
   EDJE_PART_ANCHOR_LINE_BOTTOM,
   EDJE_PART_ANCHOR_LINE_LEFT,
   EDJE_PART_ANCHOR_LINE_RIGHT,
   EDJE_PART_ANCHOR_LINE_VERTICAL_CENTER,
   EDJE_PART_ANCHOR_LINE_HORIZONTAL_CENTER
} Edje_Part_Anchor_Line;

typedef enum
{
   EDJE_PART_ANCHOR_FILL_BOTH,
   EDJE_PART_ANCHOR_FILL_HORIZONTAL,
   EDJE_PART_ANCHOR_FILL_VERTICAL
} Edje_Part_Anchor_Fill;

typedef struct
{
   union {
      Edje_Part_Anchor_Line line;
      Edje_Part_Anchor_Fill fill;
   } base;
   Eina_Bool set : 1;
} Edje_Part_Anchor;

typedef struct
{
   Edje_Part_Anchor top;
   Edje_Part_Anchor bottom;
   Edje_Part_Anchor left;
   Edje_Part_Anchor right;
   Edje_Part_Anchor vertical_center;
   Edje_Part_Anchor horizontal_center;
   Edje_Part_Anchor fill;
} Edje_Part_Description_Anchors;

/* global fn calls */
void    data_setup(void);
void    data_write(void);
void    data_queue_face_group_lookup(const char *name);
void    data_queue_group_lookup(const char *name, Edje_Part *part);
void    data_queue_part_lookup(Edje_Part_Collection *pc, const char *name, int *dest);
void    data_queue_part_nest_lookup(Edje_Part_Collection *pc, const char *name, int *dest, char **dest2);
void    data_queue_copied_part_nest_lookup(Edje_Part_Collection *pc, int *src, int *dest, char **dest2);
void    data_queue_part_reallocated_lookup(Edje_Part_Collection *pc, const char *name,
					   unsigned char **base, int offset);
void    part_lookup_del(Edje_Part_Collection *pc, int *dest);
void    part_lookup_delete(Edje_Part_Collection *pc, const char *name, int *dest, char **dest2);
void    data_queue_copied_part_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void   *data_queue_program_lookup(Edje_Part_Collection *pc, const char *name, int *dest);
void    program_lookup_rename(void *p, const char *name);
void    copied_program_lookup_delete(Edje_Part_Collection *pc, const char *name);
Eina_Bool     data_queue_copied_program_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void    copied_program_anonymous_lookup_delete(Edje_Part_Collection *pc, int *dest);
void    data_queue_anonymous_lookup(Edje_Part_Collection *pc, Edje_Program *ep, int *dest);
void    data_queue_copied_anonymous_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void    data_queue_image_lookup(char *name, int *dest, Eina_Bool *set);
void    data_queue_model_lookup(char *name, int *dest, Eina_Bool *set);
void    data_queue_copied_image_lookup(int *src, int *dest, Eina_Bool *set);
void    data_queue_image_remove(int *dest, Eina_Bool *set);
void    data_queue_copied_model_lookup(int *src, int *dest, Eina_Bool *set);
void    data_queue_model_remove(int *dest, Eina_Bool *set);
void    data_queue_spectrum_lookup(char *name, int *dest);
void    data_queue_spectrum_slave_lookup(int *master, int *slave);
void    data_process_lookups(void);
void    data_process_scripts(void);
void    data_process_script_lookups(void);
void    process_color_tree(char *s, const char *file_in, int line);

void    part_description_image_cleanup(Edje_Part *ep);

int     is_verbatim(void);
void    track_verbatim(int on);
void    set_verbatim(char *s, int l1, int l2);
char   *get_verbatim(void);
int     get_verbatim_line1(void);
int     get_verbatim_line2(void);
void    compile(void);
int     is_param(int n);
int     is_num(int n);
char   *parse_str(int n);
int     parse_enum(int n, ...);
int     parse_flags(int n, ...);
int     parse_int(int n);
int     parse_int_range(int n, int f, int t);
int     parse_bool(int n);
double  parse_float(int n);
double  parse_float_range(int n, double f, double t);
int     get_arg_count(void);
void    check_arg_count(int n);
void    check_min_arg_count(int n);
int     check_range_arg_count(int n, int m);
int     param_had_quote(int n);

int     object_handler_num(void);
int     object_handler_short_num(void);
int     statement_handler_num(void);
int     statement_handler_short_num(void);
int     statement_handler_short_single_num(void);
int     nested_handler_num(void);
int     nested_handler_short_num(void);

void    reorder_parts(void);
void    source_edd(void);
void    source_fetch(void);
int     source_append(Eet_File *ef);
SrcFile_List *source_load(Eet_File *ef);
int     source_fontmap_save(Eet_File *ef, Eina_List *fonts);
Edje_Font_List *source_fontmap_load(Eet_File *ef);

void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

void    using_file(const char *filename, const char type);

void    error_and_abort(Eet_File *ef, const char *fmt, ...);

void stack_push_quick(const char *str);
char *stack_pop_quick(Eina_Bool check_last, Eina_Bool do_free);
void stack_replace_quick(const char *token);
Eina_Bool edje_cc_handlers_wildcard(void);
void edje_cc_handlers_hierarchy_alloc(void);
void edje_cc_handlers_hierarchy_free(void);
void edje_cc_handlers_pop_notify(const char *token);
int get_param_index(char *str);

void color_tree_root_free(void);
void convert_color_code(char *str, int *r, int *g, int *b, int *a);

void script_rewrite(Code *code);

/* global vars */
extern Eina_List             *ext_dirs;
extern Eina_List             *img_dirs;
extern Eina_List             *model_dirs;
extern Eina_List             *fnt_dirs;
extern Eina_List             *snd_dirs;
extern Eina_List             *mo_dirs;
extern Eina_List             *vibration_dirs;
extern Eina_List             *data_dirs;
extern char                  *file_in;
extern char                  *file_out;
extern char                  *watchfile;
extern char                  *depfile;
extern char                  *license;
extern char                  *authors;
extern Eina_List             *licenses;
extern int                    no_lossy;
extern int                    no_comp;
extern int                    no_raw;
extern int                    no_etc1;
extern int                    no_etc2;
extern int                    no_save;
extern int                    min_quality;
extern int                    max_quality;
extern int                    line;
extern Eina_List             *stack;
extern Edje_File             *edje_file;
extern Eina_List             *edje_collections;
extern Eina_Hash             *edje_collections_lookup;
extern Eina_List             *externals;
extern Eina_List             *fonts;
extern Eina_List             *codes;
extern Eina_List             *defines;
extern Eina_List             *aliases;
extern New_Object_Handler     object_handlers[];
extern New_Object_Handler     object_handlers_short[];
extern New_Statement_Handler  statement_handlers[];
extern New_Statement_Handler  statement_handlers_short[];
extern New_Statement_Handler  statement_handlers_short_single[];
extern New_Nested_Handler     nested_handlers[];
extern New_Nested_Handler     nested_handlers_short[];
extern int                    compress_mode;
extern int                    threads;
extern int                    annotate;
extern Eina_Bool current_group_inherit;
extern Eina_List             *color_tree_root;
extern int                    beta;

extern int had_quote;

extern unsigned int max_open_files;
extern Eina_Array *requires;
extern Eina_Bool namespace_verify;
#endif
