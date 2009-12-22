#ifndef EDJE_DECC_H
#define EDJE_DECC_H

#include <edje_private.h>

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
typedef struct _Font                  Font;
typedef struct _Font_List             Font_List;
typedef struct _SrcFile               SrcFile;
typedef struct _SrcFile_List          SrcFile_List;

struct _Font
{
   char *file;
   char *name;
};

struct _Font_List
{
   Eina_List *list;
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

void    source_edd(void);
void    source_fetch(void);
int     source_append(Eet_File *ef);
SrcFile_List *source_load(Eet_File *ef);
int     source_fontmap_save(Eet_File *ef, Eina_List *fonts);
Font_List *source_fontmap_load(Eet_File *ef);

void   *mem_alloc(size_t size);
char   *mem_strdup(const char *s);
#define SZ sizeof

#endif
