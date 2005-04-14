/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_private.h"
#include "ecore_file_private.h"

#ifdef HAVE_POLL

/*
 * TODO:
 * - Implement recursive as an option!
 * - Keep whole path or just name of file? (Memory or CPU...)
 * - Remove requests without files?
 * - Change poll time
 */

typedef struct _Ecore_File_Monitor_Poll Ecore_File_Monitor_Poll;
typedef struct _Ecore_File              Ecore_File;

#define ECORE_FILE_MONITOR_POLL(x) ((Ecore_File_Monitor_Poll *)(x))

struct _Ecore_File_Monitor_Poll
{
   Ecore_File_Monitor  monitor;
   int                 mtime;
   unsigned char       deleted;
};

struct _Ecore_File
{
   char          *name;
   int            mtime;
   unsigned char  is_dir;
};

#define ECORE_FILE_INTERVAL_MIN  1.0
#define ECORE_FILE_INTERVAL_STEP 0.5
#define ECORE_FILE_INTERVAL_MAX  5.0

static double       _interval = ECORE_FILE_INTERVAL_MIN;
static Ecore_Timer *_timer = NULL;
static Evas_List   *_monitors = NULL;
static int          _lock = 0;

static int         _ecore_file_monitor_poll_handler(void *data);
static void        _ecore_file_monitor_poll_check(Ecore_File_Monitor *em);
static int         _ecore_file_monitor_poll_checking(Ecore_File_Monitor *em, char *name);

int
ecore_file_monitor_poll_init(void)
{
   return 1;
}

int
ecore_file_monitor_poll_shutdown(void)
{
   Evas_List *l;
   for (l = _monitors; l;)
     {
	Ecore_File_Monitor *em;

	em = l->data;
	l = l->next;
	ecore_file_monitor_del(em);
     }
   evas_list_free(_monitors);
   if (_timer)
     {
       	ecore_timer_del(_timer);
	_timer = NULL;
     }
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_poll_add(const char *path,
			    void (*func) (void *data, Ecore_File_Monitor *em,
					  Ecore_File_Event event,
					  const char *path),
			    void *data)
{
   Ecore_File_Monitor *em;
   int len;

   if (!path) return NULL;
   if (!func) return NULL;

   em = calloc(1, sizeof(Ecore_File_Monitor_Poll));
   if (!em) return NULL;

   if (!_timer)
     _timer = ecore_timer_add(_interval, _ecore_file_monitor_poll_handler, NULL);
   else
     ecore_timer_interval_set(_timer, ECORE_FILE_INTERVAL_MIN);

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = 0;

   em->func = func;
   em->data = data;

   ECORE_FILE_MONITOR_POLL(em)->mtime = ecore_file_mod_time(em->path);
   if (ecore_file_exists(em->path))
     {
	if (ecore_file_is_dir(em->path))
	  {
	     /* Check for subdirs */
	     Ecore_List *files;
	     char *file;

	     files = ecore_file_ls(em->path);
	     while ((file = ecore_list_next(files)))
	       {
		  Ecore_File *f;
		  char buf[PATH_MAX];

		  f = calloc(1, sizeof(Ecore_File));
		  if (!f)
		    {
		       free(file);
		       continue;
		    }

		  snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
		  f->name = file;
		  f->mtime = ecore_file_mod_time(buf);
		  f->is_dir = ecore_file_is_dir(buf);
		  em->files = evas_list_append(em->files, f);
	       }
	     ecore_list_destroy(files);
	  }
     }
   else
     {
	ecore_file_monitor_poll_del(em);
	return NULL;
     }

   _monitors = evas_list_append(_monitors, em);

   return em;
}

void
ecore_file_monitor_poll_del(Ecore_File_Monitor *em)
{
   Evas_List *l;

   if (_lock)
     {
	ECORE_FILE_MONITOR_POLL(em)->deleted = 1;
	return;
     }

   /* Remove files */
   for (l = em->files; l; l = l->next)
     {
	Ecore_File *f;
	f = l->data;
	free(f->name);
	free(f);
     }
   evas_list_free(em->files);

   _monitors = evas_list_remove(_monitors, em);
   free(em->path);
   free(em);

   if ((!_monitors) && (_timer))
     {
	ecore_timer_del(_timer);
	_timer = NULL;
     }
   else
     ecore_timer_interval_set(_timer, ECORE_FILE_INTERVAL_MIN);
}

static int
_ecore_file_monitor_poll_handler(void *data __UNUSED__)
{
   Evas_List *monitor;

   _interval += ECORE_FILE_INTERVAL_STEP;
   _lock = 1;
   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;

	em = monitor->data;
	monitor = monitor->next;
	_ecore_file_monitor_poll_check(em);
     }
   _lock = 0;
   if (_interval > ECORE_FILE_INTERVAL_MAX)
     _interval = ECORE_FILE_INTERVAL_MAX;
   ecore_timer_interval_set(_timer, _interval);

   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;

	em = monitor->data;
	monitor = monitor->next;
	if (ECORE_FILE_MONITOR_POLL(em)->deleted)
	  ecore_file_monitor_del(em);
     }
   return 1;
}

static void
_ecore_file_monitor_poll_check(Ecore_File_Monitor *em)
{
   int mtime;
   int is_dir;

   mtime = ecore_file_mod_time(em->path);
   is_dir = ecore_file_is_dir(em->path);
   if (mtime < ECORE_FILE_MONITOR_POLL(em)->mtime)
     {
	Evas_List *l;
	Ecore_File_Event event;

	/* Notify all files deleted */
	for (l = em->files; l;)
	  {
	     Ecore_File *f;
	     char buf[PATH_MAX];

	     f = l->data;
	     l = l->next;
	     snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
	     if (f->is_dir)
	       event = ECORE_FILE_EVENT_DELETED_DIRECTORY;
	     else
	       event = ECORE_FILE_EVENT_DELETED_FILE;
	     em->func(em->data, em, event, buf);
	     free(f->name);
	     free(f);
	  }
	em->files = evas_list_free(em->files);
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
	_interval = ECORE_FILE_INTERVAL_MIN;
     }
   else
     {
	Evas_List *l;

	/* Check for changed files */
	for (l = em->files; l;)
	  {
	     Ecore_File *f;
	     char buf[PATH_MAX];
	     int mtime;
	     Ecore_File_Event event;

	     f = l->data;
	     l = l->next;
	     snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
	     mtime = ecore_file_mod_time(buf);
	     if (mtime < f->mtime)
	       {
		  if (f->is_dir)
		    event = ECORE_FILE_EVENT_DELETED_DIRECTORY;
		  else
		    event = ECORE_FILE_EVENT_DELETED_FILE;

		  em->func(em->data, em, event, buf);
		  em->files = evas_list_remove(em->files, f);
		  free(f->name);
		  free(f);
		  _interval = ECORE_FILE_INTERVAL_MIN;
	       }
	     else if ((mtime > f->mtime) && !(f->is_dir))
	       {
		  em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, buf);
		  _interval = ECORE_FILE_INTERVAL_MIN;
	       }
	     f->mtime = mtime;
	  }

	/* Check for new files */
	if (ECORE_FILE_MONITOR_POLL(em)->mtime < mtime)
	  {
	     Ecore_List *files;
	     char *file;

	     /* Files have been added or removed */
	     files = ecore_file_ls(em->path);
	     while ((file = ecore_list_next(files)))
	       {
		  Ecore_File *f;
		  char buf[PATH_MAX];
		  Ecore_File_Event event;

		  if (_ecore_file_monitor_poll_checking(em, file))
		    {
		       free(file);
		       continue;
		    }

		  snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
		  f = calloc(1, sizeof(Ecore_File));
		  if (!f)
		    {
		       free(file);
		       continue;
		    }

		  f->name = file;
		  f->mtime = ecore_file_mod_time(buf);
		  f->is_dir = ecore_file_is_dir(buf);
		  if (f->is_dir)
		    event = ECORE_FILE_EVENT_CREATED_DIRECTORY;
		  else
		    event = ECORE_FILE_EVENT_CREATED_FILE;
		  em->func(em->data, em, event, buf);
		  em->files = evas_list_append(em->files, f);
	       }
	     ecore_list_destroy(files);
	     if (!ecore_file_is_dir(em->path))
	       em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, em->path);
	     _interval = ECORE_FILE_INTERVAL_MIN;
	  }
     }
   ECORE_FILE_MONITOR_POLL(em)->mtime = mtime;
}

static int
_ecore_file_monitor_poll_checking(Ecore_File_Monitor *em, char *name)
{
   Evas_List *l;

   for (l = em->files; l; l = l->next)
     {
	Ecore_File *f;

	f = l->data;
	if (!strcmp(f->name, name))
	  return 1;
     }

   return 0;
}
#endif
