#ifndef ECORE_FILE_PRIVATE_H_
#define ECORE_FILE_PRIVATE_H_

#ifdef __linux__
# include <features.h>
#endif

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_File.h"

extern int _ecore_file_log_dom;

#ifdef ECORE_FILE_DEFAULT_LOG_COLOR
#undef ECORE_FILE_DEFAULT_LOG_COLOR
#endif
#define ECORE_FILE_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_file_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_file_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_file_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_file_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_file_log_dom, __VA_ARGS__)

/* ecore_file_monitor */
int  ecore_file_monitor_init(void);
void ecore_file_monitor_shutdown(void);

#define ECORE_FILE_MONITOR(x) ((Ecore_File_Monitor *)(x))

typedef struct _Ecore_File Ecore_File;
struct _Ecore_File
{
   EINA_INLIST;
   char          *name;
   int            mtime;
   unsigned char  is_dir;
};

struct _Ecore_File_Monitor
{
   EINA_INLIST;
   void (*func) (void *data,
                 Ecore_File_Monitor *ecore_file_monitor,
                 Ecore_File_Event event,
                 const char *path);

   const char         *path;
   void               *data;
   Ecore_File         *files;
};

int                 ecore_file_monitor_backend_init(void);
int                 ecore_file_monitor_backend_shutdown(void);
Ecore_File_Monitor *ecore_file_monitor_backend_add(const char *path,
                                                   void (*func) (void *data,
                                                                 Ecore_File_Monitor *ecore_file_monitor,
                                                                 Ecore_File_Event event,
                                                                 const char *path),
                                                   void *data);
void                ecore_file_monitor_backend_del(Ecore_File_Monitor *ecore_file_monitor);

/* ecore_file_path */
void ecore_file_path_init(void);
void ecore_file_path_shutdown(void);

/* ecore_file_download */
int  ecore_file_download_init(void);
void ecore_file_download_shutdown(void);

#endif
