#define _GNU_SOURCE

#include "config.h"

#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_File.h"

/* ecore_file_monitor */
int ecore_file_monitor_init(void);
int ecore_file_monitor_shutdown(void);

#define ECORE_FILE_MONITOR(x) ((Ecore_File_Monitor *)(x))

struct _Ecore_File_Monitor
{
   Ecore_List2 __list_data;
   void (*func) (void *data,
		 Ecore_File_Monitor *ecore_file_monitor,
		 Ecore_File_Event event,
		 const char *path);

   char               *path;
   void               *data;
   Ecore_List2      *files;
};

#ifdef HAVE_INOTIFY
EAPI int                 ecore_file_monitor_inotify_init(void);
EAPI int                 ecore_file_monitor_inotify_shutdown(void);
EAPI Ecore_File_Monitor *ecore_file_monitor_inotify_add(const char *path,
							void (*func) (void *data,
								      Ecore_File_Monitor *ecore_file_monitor,
								      Ecore_File_Event event,
								      const char *path),
							void *data);
EAPI void                ecore_file_monitor_inotify_del(Ecore_File_Monitor *ecore_file_monitor);
#endif

#ifdef HAVE_FAM
EAPI int                 ecore_file_monitor_fam_init(void);
EAPI int                 ecore_file_monitor_fam_shutdown(void);
EAPI Ecore_File_Monitor *ecore_file_monitor_fam_add(const char *path,
						    void (*func) (void *data,
								  Ecore_File_Monitor *ecore_file_monitor,
								  Ecore_File_Event event,
								  const char *path),
						    void *data);
EAPI void                ecore_file_monitor_fam_del(Ecore_File_Monitor *ecore_file_monitor);
#endif

#ifdef HAVE_POLL
EAPI int                 ecore_file_monitor_poll_init(void);
EAPI int                 ecore_file_monitor_poll_shutdown(void);
EAPI Ecore_File_Monitor *ecore_file_monitor_poll_add(const char *path,
						     void (*func) (void *data,
								   Ecore_File_Monitor *ecore_file_monitor,
								   Ecore_File_Event event,
								   const char *path),
						     void *data);
EAPI void                ecore_file_monitor_poll_del(Ecore_File_Monitor *ecore_file_monitor);

/* ecore_file_path */
int ecore_file_path_init(void);
int ecore_file_path_shutdown(void);

/* ecore_file_download */
int ecore_file_download_init(void);
int ecore_file_download_shutdown(void);

#endif
