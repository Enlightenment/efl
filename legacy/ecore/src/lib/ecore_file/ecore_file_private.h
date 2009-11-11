#ifndef ECORE_FILE_PRIVATE_H_
#define ECORE_FILE_PRIVATE_H_

#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS  64
#endif

#ifdef __linux__
# include <features.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#include "Ecore_File.h"

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

   char               *path;
   void               *data;
   Ecore_File         *files;
};

#ifdef HAVE_INOTIFY
int                 ecore_file_monitor_inotify_init(void);
int                 ecore_file_monitor_inotify_shutdown(void);
Ecore_File_Monitor *ecore_file_monitor_inotify_add(const char *path,
							void (*func) (void *data,
								      Ecore_File_Monitor *ecore_file_monitor,
								      Ecore_File_Event event,
								      const char *path),
							void *data);
void                ecore_file_monitor_inotify_del(Ecore_File_Monitor *ecore_file_monitor);
#endif
 
#ifdef HAVE_NOTIFY_WIN32
int                 ecore_file_monitor_win32_init(void);
int                 ecore_file_monitor_win32_shutdown(void);
Ecore_File_Monitor *ecore_file_monitor_win32_add(const char *path,
                                                 void      (*func) (void               *data,
                                                                    Ecore_File_Monitor *ecore_file_monitor,
                                                                    Ecore_File_Event    event,
                                                                    const char         *path),
                                                 void       *data);
void                ecore_file_monitor_win32_del(Ecore_File_Monitor *ecore_file_monitor);
#endif

#ifdef HAVE_POLL
int                 ecore_file_monitor_poll_init(void);
int                 ecore_file_monitor_poll_shutdown(void);
Ecore_File_Monitor *ecore_file_monitor_poll_add(const char *path,
						void (*func) (void *data,
							      Ecore_File_Monitor *ecore_file_monitor,
							      Ecore_File_Event event,
							      const char *path),
						void *data);
void                ecore_file_monitor_poll_del(Ecore_File_Monitor *ecore_file_monitor);

#endif

/* ecore_file_path */
void ecore_file_path_init(void);
void ecore_file_path_shutdown(void);

/* ecore_file_download */
int  ecore_file_download_init(void);
void ecore_file_download_shutdown(void);

#endif
