/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"
#include "Ecore_File.h"

/*
 * TODO:
 * - Check the return value from fcntl.
 * - Try different realtime numbers if one fail.
 * - Doesn't work right!
 *   - Misses fileupdates
 *   - Misses filedeletions
 */

#ifdef HAVE_DNOTIFY

#include <fcntl.h>

typedef struct _Ecore_File_Monitor_Dnotify Ecore_File_Monitor_Dnotify;
typedef struct _Ecore_File                 Ecore_File;

#define ECORE_FILE_MONITOR_DNOTIFY(x) ((Ecore_File_Monitor_Dnotify *)(x))

struct _Ecore_File_Monitor_Dnotify
{
   Ecore_File_Monitor  monitor;
   int                 fd;
   Evas_List          *files;
   unsigned char       deleted;
};

struct _Ecore_File
{
   char            *name;
   int              mtime;
   Ecore_File_Type  type;
};

static Ecore_Event_Handler *_eh;
static Evas_List           *_monitors = NULL;
static int                  _lock = 0;

static int                         _ecore_file_monitor_handler(void *data, int type, void *event);
static void                        _ecore_file_monitor_check(Ecore_File_Monitor *em);
static int                         _ecore_file_monitor_checking(Ecore_File_Monitor *em, char *path);

int
ecore_file_monitor_init(void)
{
   _eh = ecore_event_handler_add(ECORE_EVENT_SIGNAL_REALTIME, _ecore_file_monitor_handler, NULL);
   return 1;
}

int
ecore_file_monitor_shutdown(void)
{
   Evas_List *l;

   if (_eh) ecore_event_handler_del(_eh);
   for (l = _monitors; l;)
     {
	Ecore_File_Monitor *em;

	em = l->data;
	l = l->next;
	ecore_file_monitor_del(em);
     }
   evas_list_free(_monitors);
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_add(const char *path,
		       void (*func) (void *data, Ecore_File_Monitor *em,
				     Ecore_File_Type type,
				     Ecore_File_Event event,
				     const char *path),
		       void *data)
{
   Ecore_File_Monitor *em;
   Ecore_File_Monitor_Dnotify *emd;
   int len;

   if (!path) return NULL;
   if (!func) return NULL;

   emd = calloc(1, sizeof(Ecore_File_Monitor_Dnotify));
   em = ECORE_FILE_MONITOR(emd);
   if (!em) return NULL;

   em->func = func;
   em->data = data;

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = '\0';

   if (ecore_file_exists(em->path))
     {
	em->type = ecore_file_is_dir(em->path) ?
		   ECORE_FILE_TYPE_DIRECTORY :
		   ECORE_FILE_TYPE_FILE;

	if (em->type == ECORE_FILE_TYPE_DIRECTORY)
	  {
	     /* Check for subdirs */
	     Evas_List *files, *l;

	     files = ecore_file_ls(em->path);
	     for (l = files; l; l = l->next)
	       {
		  Ecore_File *f;
		  char *file;
		  char buf[PATH_MAX];

		  file = l->data;
		  f = calloc(1, sizeof(Ecore_File));
		  if (!f)
		    {
		       free(file);
		       continue;
		    }

		  snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
		  f->name = file;
		  f->mtime = ecore_file_mod_time(buf);
		  f->type = ecore_file_is_dir(buf) ?
			    ECORE_FILE_TYPE_DIRECTORY :
			    ECORE_FILE_TYPE_FILE;
		  em->func(em->data, em, f->type, ECORE_FILE_EVENT_EXISTS, buf);
		  emd->files = evas_list_append(emd->files, f);
	       }
	     evas_list_free(files);

	     emd->fd = open(em->path, O_RDONLY);
	     if (fcntl(emd->fd, F_SETSIG, SIGRTMIN + 1))
	       printf("ERROR: F_SETSIG\n");
	     if (fcntl(emd->fd, F_NOTIFY, DN_ACCESS|DN_MODIFY|DN_CREATE|DN_DELETE|DN_MULTISHOT))
	       printf("ERROR: F_NOTIFY\n");
	  }
	else
	  {
	     /* TODO: We do not support monitoring files! */
	     free(em);
	     return NULL;
	  }
     }
   else
     {
	em->type = ECORE_FILE_TYPE_NONE;
	em->func(em->data, em, em->type, ECORE_FILE_EVENT_DELETED, em->path);
     }

   _monitors = evas_list_append(_monitors, em);

   return em;
}

void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   Ecore_File_Monitor_Dnotify *emd;
   Evas_List *l;

   emd = ECORE_FILE_MONITOR_DNOTIFY(em);
   if (_lock)
     {
	emd->deleted = 1;
	return;
     }
   close(emd->fd);
   /* Remove files */
   for (l = emd->files; l; l = l->next)
     {
	Ecore_File *f;
	f = l->data;
	free(f->name);
	free(f);
     }
   evas_list_free(emd->files);

   _monitors = evas_list_remove(_monitors, em);
   free(em->path);
   free(em);
}

static int
_ecore_file_monitor_handler(void *data, int type, void *event)
{
   Ecore_Event_Signal_Realtime *ev;
   Evas_List *monitor;

   ev = event;
   _lock = 1;
   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;
	Ecore_File_Monitor_Dnotify *emd;

	em = monitor->data;
	emd = ECORE_FILE_MONITOR_DNOTIFY(em);
	monitor = monitor->next;

	if (emd->fd == ev->data.si_fd)
	  _ecore_file_monitor_check(em);
     }
   _lock = 0;
   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;
	Ecore_File_Monitor_Dnotify *emd;

	em = monitor->data;
	emd = ECORE_FILE_MONITOR_DNOTIFY(em);
	monitor = monitor->next;

	if (emd->deleted)
	  ecore_file_monitor_del(em);
     }
   return 1;
}

#if 0
static Ecore_File_Monitor_Request *
_ecore_file_monitor_request_find(Ecore_File_Monitor *em, char *path)
{
   Evas_List *l;

   for (l = em->requests; l; l = l->next)
     {
	Ecore_File_Monitor_Request *er;

	er = l->data;
	if (!strcmp(er->path, path))
	  return er;
     }
   return NULL;
}
#endif

static void
_ecore_file_monitor_check(Ecore_File_Monitor *em)
{
   Ecore_File_Monitor_Dnotify *emd;
   Evas_List *files, *l;

   /* Check for changed files */
   emd = ECORE_FILE_MONITOR_DNOTIFY(em);
   for (l = emd->files; l;)
     {
	Ecore_File *f;
	char buf[PATH_MAX];
	int mtime;

	f = l->data;
	l = l->next;
	snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
	mtime = ecore_file_mod_time(buf);
	if (mtime < f->mtime)
	  {
	     em->func(em->data, em, f->type, ECORE_FILE_EVENT_DELETED, buf);
	     emd->files = evas_list_remove(emd->files, f);
	     free(f->name);
	     free(f);
	  }
	else if (mtime > f->mtime)
	  em->func(em->data, em, f->type, ECORE_FILE_EVENT_CHANGED, buf);
	f->mtime = mtime;
     }

   files = ecore_file_ls(em->path);
   for (l = files; l; l = l->next)
     {
	Ecore_File *f;
	char *file;
	char buf[PATH_MAX];

	file = l->data;
	if (_ecore_file_monitor_checking(em, file))
	  {
	    free(file);
	    continue;
	  }

	f = calloc(1, sizeof(Ecore_File));
	if (!f)
	  {
	    free(file);
	    continue;
	  }

	snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
	f->name = file;
	f->mtime = ecore_file_mod_time(buf);
	f->type = ecore_file_is_dir(buf) ?
		  ECORE_FILE_TYPE_DIRECTORY :
		  ECORE_FILE_TYPE_FILE;
	em->func(em->data, em, f->type, ECORE_FILE_EVENT_CREATED, buf);
	emd->files = evas_list_append(emd->files, f);
     }
}

static int
_ecore_file_monitor_checking(Ecore_File_Monitor *em, char *name)
{
   Ecore_File_Monitor_Dnotify *emd;
   Evas_List *l;

   emd = ECORE_FILE_MONITOR_DNOTIFY(em);
   for (l = emd->files; l; l = l->next)
     {
	Ecore_File *f;

	f = l->data;
	if (!strcmp(f->name, name))
	  return 1;
     }

   return 0;
}
#endif
