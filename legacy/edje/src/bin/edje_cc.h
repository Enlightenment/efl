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


/* types */
typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;
typedef struct _External_List         External_List;
typedef struct _External              External;
typedef struct _Font_List             Font_List;
typedef struct _Font                  Font;
typedef struct _Code                  Code;
typedef struct _Code_Program          Code_Program;
typedef struct _SrcFile               SrcFile;
typedef struct _SrcFile_List          SrcFile_List;

typedef struct _Edje_Program_Parser                  Edje_Program_Parser;
typedef struct _Edje_Pack_Element_Parser             Edje_Pack_Element_Parser;
typedef struct _Edje_Part_Parser                     Edje_Part_Parser;

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

struct _External_List
{
   Eina_List *list;
};

struct _External
{
    char *name;
};

struct _Font_List
{
   Eina_List *list;
};

struct _Font
{
   char *name;
   char *file;
};

struct _Code
{
   int       l1, l2;
   char      *shared;
   char      *original;
   Eina_List *programs;
   int		is_lua;
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

/* global fn calls */
void    data_setup(void);
void    data_write(void);
void    data_queue_part_lookup(Edje_Part_Collection *pc, const char *name, int *dest);
void    data_queue_copied_part_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void    data_queue_program_lookup(Edje_Part_Collection *pc, const char *name, int *dest);
void    data_queue_copied_program_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void    data_queue_anonymous_lookup(Edje_Part_Collection *pc, Edje_Program *ep, int *dest);
void    data_queue_copied_anonymous_lookup(Edje_Part_Collection *pc, int *src, int *dest);
void    data_queue_image_lookup(char *name, int *dest, Eina_Bool *set);
void    data_queue_copied_image_lookup(int *src, int *dest, Eina_Bool *set);
void    data_queue_part_slave_lookup(int *master, int *slave);
void    data_queue_image_slave_lookup(int *master, int *slave);
void    data_queue_spectrum_lookup(char *name, int *dest);
void    data_queue_spectrum_slave_lookup(int *master, int *slave);
void    data_process_lookups(void);
void    data_process_scripts(void);
void    data_process_script_lookups(void);


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

int     object_handler_num(void);
int     statement_handler_num(void);

void    reorder_parts(void);
void    source_edd(void);
void    source_fetch(void);
int     source_append(Eet_File *ef);
SrcFile_List *source_load(Eet_File *ef);
int     source_fontmap_save(Eet_File *ef, Eina_List *fonts);
Font_List *source_fontmap_load(Eet_File *ef);

void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

void    error_and_abort(Eet_File *ef, const char *fmt, ...);

/* global vars */
extern Eina_List             *ext_dirs;
extern Eina_List             *img_dirs;
extern Eina_List             *fnt_dirs;
extern Eina_List             *snd_dirs;
extern char                  *file_in;
extern char                  *tmp_dir;
extern char                  *file_out;
extern char                  *progname;
extern int                    verbose;
extern int                    no_lossy;
extern int                    no_comp;
extern int                    no_raw;
extern int                    no_save;
extern int                    min_quality;
extern int                    max_quality;
extern int                    line;
extern Eina_List             *stack;
extern Eina_List             *params;
extern Edje_File             *edje_file;
extern Eina_List             *edje_collections;
extern Eina_List             *externals;
extern Eina_List             *fonts;
extern Eina_List             *codes;
extern Eina_List             *defines;
extern Eina_List             *aliases;
extern New_Object_Handler     object_handlers[];
extern New_Statement_Handler  statement_handlers[];


#endif
