#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"

#include "Ecore.h"
#include "Ecore_File.h"

int ecore_file_monitor_init(void);
int ecore_file_monitor_shutdown(void);

struct _Ecore_File_Monitor
{
   void (*func) (void *data,
		 Ecore_File_Monitor *ecore_file_monitor,
		 Ecore_File_Type type,
		 Ecore_File_Event event,
		 const char *path);

   char               *path;
   Ecore_File_Type     type;
   void               *data;
   Evas_List          *files;
#ifdef HAVE_POLL
   int                 mtime;
   unsigned char       deleted;
#endif
};

/*
#define HAVE_POLL
#define HAVE_FAM
#define HAVE_DNOTIFY
#define HAVE_INOTIFY
*/
