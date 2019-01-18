#ifndef ELM_PREFS_CC_H
#define ELM_PREFS_CC_H

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_prefs.h"

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
extern int _elm_prefs_cc_log_dom;
#define ELM_PREFS_CC_DEFAULT_LOG_COLOR EINA_COLOR_CYAN

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_elm_prefs_cc_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_elm_prefs_cc_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_elm_prefs_cc_log_dom, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_elm_prefs_cc_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_elm_prefs_cc_log_dom, __VA_ARGS__)

/* types */
typedef struct _Elm_Prefs_File        Elm_Prefs_File;
typedef struct _New_Object_Handler    New_Object_Handler;
typedef struct _New_Statement_Handler New_Statement_Handler;

struct _Elm_Prefs_File
{
   const char        *compiler;

   Eina_List         *pages;
};

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

/* global fn calls */
void    compile(void);
char   *parse_str(int n);
int     parse_enum(int n, ...);
int     parse_int(int n);
int     parse_int_range(int n, int f, int t);
int     parse_bool(int n);
double  parse_float(int n);
void    check_arg_count(int n);
void    check_regex(const char *regex);
void    set_verbatim(char *s, int l1, int l2);

void    data_init(void);
void    data_write(void);
void    data_shutdown(void);

int     object_handler_num(void);
int     statement_handler_num(void);

void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

/* global vars */
extern char                  *file_in;
extern const char            *tmp_dir;
extern char                  *file_out;
extern int                    line;
extern Eina_List             *stack;
extern Eina_List             *params;

extern Elm_Prefs_File        *elm_prefs_file;

extern Eina_List             *elm_prefs_pages;
extern New_Object_Handler     object_handlers[];
extern New_Statement_Handler  statement_handlers[];

#endif
