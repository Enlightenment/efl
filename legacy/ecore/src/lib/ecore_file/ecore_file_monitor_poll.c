/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"
#include "Ecore_File.h"

#ifdef HAVE_POLL

/*
 * TODO:
 * - Implement recursive as an option!
 * - Keep whole path or just name of file? (Memory or CPU...)
 * - Remove requests without files?
 */

typedef struct _Ecore_File_Monitor_Poll Ecore_File_Monitor_Poll;
typedef struct _Ecore_File             Ecore_File;

#define ECORE_FILE_MONITOR_POLL(x) ((Ecore_File_Monitor_Poll *)(x))

struct _Ecore_File_Monitor_Poll
{
   Ecore_File_Monitor  monitor;
   int                 mtime;
   Evas_List          *files;
   unsigned char       deleted;
};

struct _Ecore_File
{
   char            *name;
   int              mtime;
   Ecore_File_Type  type;
};

static Ecore_Timer *_timer = NULL;
static Evas_List   *_monitors = NULL;
static int          _lock = 0;

static int                         _ecore_file_monitor_handler(void *data);
static void                        _ecore_file_monitor_check(Ecore_File_Monitor *em);
static int                         _ecore_file_monitor_checking(Ecore_File_Monitor *em, char *name);

int
ecore_file_monitor_init(void)
{
   return 1;
}

int
ecore_file_monitor_shutdown(void)
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
ecore_file_monitor_add(const char *path,
		       void (*func) (void *data, Ecore_File_Monitor *em,
				     Ecore_File_Type type,
				     Ecore_File_Event event,
				     const char *path),
		       void *data)
{
   Ecore_File_Monitor *em;
   Ecore_File_Monitor_Poll *emf;
   int len;

   if (!path) return NULL;
   if (!func) return NULL;

   emf = calloc(1, sizeof(Ecore_File_Monitor_Poll));
   em = ECORE_FILE_MONITOR(emf);
   if (!em) return NULL;

   if (!_timer)
     _timer = ecore_timer_add(1.0, _ecore_file_monitor_handler, NULL);

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = '\0';

   em->func = func;
   em->data = data;

   emf->mtime = ecore_file_mod_time(em->path);
   if (ecore_file_exists(em->path))
     {
	em->type = ecore_file_is_dir(em->path) ?
		   ECORE_FILE_TYPE_DIRECTORY :
		   ECORE_FILE_TYPE_FILE;

#if 0
	em->func(em->data, em, em->type, ECORE_FILE_EVENT_EXISTS, em->path);
#endif
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
		  emf->files = evas_list_append(emf->files, f);
	       }
	     evas_list_free(files);
	  }
#if 0
	else
	  em->func(em->data, em, em->type, ECORE_FILE_EVENT_EXISTS, em->path);
#endif
     }
   else
     {
	em->type = ECORE_FILE_TYPE_NONE;
#if 0
	em->func(em->data, em, em->type, ECORE_FILE_EVENT_DELETED, em->path);
#endif
     }

   _monitors = evas_list_append(_monitors, em);

   return em;
}

void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   Ecore_File_Monitor_Poll *emf;
   Evas_List *l;

   emf = ECORE_FILE_MONITOR_POLL(em);
   if (_lock)
     {
	emf->deleted = 1;
	return;
     }

   /* Remove files */
   for (l = emf->files; l; l = l->next)
     {
	Ecore_File *f;
	f = l->data;
	free(f->name);
	free(f);
     }
   evas_list_free(emf->files);

   _monitors = evas_list_remove(_monitors, em);
   free(em->path);
   free(em);

   if ((!_monitors) && (_timer))
     {
	ecore_timer_del(_timer);
	_timer = NULL;
     }
}

static int
_ecore_file_monitor_handler(void *data)
{
   Evas_List *monitor;

   _lock = 1;
   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;

	em = monitor->data;
	monitor = monitor->next;
	_ecore_file_monitor_check(em);
     }
   _lock = 0;
   for (monitor = _monitors; monitor;)
     {
	Ecore_File_Monitor *em;
	Ecore_File_Monitor_Poll *emf;

	em = monitor->data;
	emf = ECORE_FILE_MONITOR_POLL(em);
	monitor = monitor->next;
	if (emf->deleted)
	  ecore_file_monitor_del(em);
     }
   return 1;
}

static void
_ecore_file_monitor_check(Ecore_File_Monitor *em)
{
   Ecore_File_Monitor_Poll *emf;
   int mtime;

   emf = ECORE_FILE_MONITOR_POLL(em);
   mtime = ecore_file_mod_time(em->path);
   switch (em->type)
     {
      case ECORE_FILE_TYPE_FILE:
	 if (mtime < emf->mtime)
	   {
	      em->func(em->data, em, em->type, ECORE_FILE_EVENT_DELETED, em->path);
	      em->type = ECORE_FILE_TYPE_NONE;
	   }
	 else if (mtime > emf->mtime)
	   em->func(em->data, em, em->type, ECORE_FILE_EVENT_CHANGED, em->path);
	 break;
      case ECORE_FILE_TYPE_DIRECTORY:
	 if (mtime < emf->mtime)
	   {
	      /* Deleted */
	      Evas_List *l;

	      /* Notify all files deleted */
	      for (l = emf->files; l;)
		{
		   Ecore_File *f;
		   char buf[PATH_MAX];

		   f = l->data;
		   l = l->next;
		   snprintf(buf, sizeof(buf), "%s/%s", em->path, f->name);
		   em->func(em->data, em, f->type, ECORE_FILE_EVENT_DELETED, buf);
		   free(f->name);
		   free(f);
		}
	      emf->files = evas_list_free(emf->files);
#if 0
	      em->func(em->data, em, em->type, ECORE_FILE_EVENT_DELETED, em->path);
#endif
	      em->type = ECORE_FILE_TYPE_NONE;
	   }
	 else
	   {
	      Evas_List *l;

	      /* Check for changed files */
	      for (l = emf->files; l;)
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
			emf->files = evas_list_remove(emf->files, f);
			free(f->name);
			free(f);
		     }
		   else if (mtime > f->mtime)
		     em->func(em->data, em, f->type, ECORE_FILE_EVENT_CHANGED, buf);
		   f->mtime = mtime;
		}

	      /* Check for new files */
	      if (emf->mtime < mtime)
		{
		   Evas_List *files;

		   /* Files have been added or removed */
#if 0
		   em->func(em->data, em, em->type, ECORE_FILE_EVENT_CHANGED, em->path);
#endif
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

			snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
			f = calloc(1, sizeof(Ecore_File));
			if (!f)
			  {
			     free(file);
			     continue;
			  }

			f->name = file;
			f->mtime = ecore_file_mod_time(buf);
			f->type = ecore_file_is_dir(buf) ?
				  ECORE_FILE_TYPE_DIRECTORY :
				  ECORE_FILE_TYPE_FILE;
			em->func(em->data, em, f->type, ECORE_FILE_EVENT_CREATED, buf);
			emf->files = evas_list_append(emf->files, f);
		     }
		}
	   }
	 break;
      case ECORE_FILE_TYPE_NONE:
	 if (mtime > emf->mtime)
	   {
	      /* Something has been created! */
	      em->type = ecore_file_is_dir(em->path) ?
			 ECORE_FILE_TYPE_DIRECTORY :
			 ECORE_FILE_TYPE_FILE;

	      em->func(em->data, em, em->type, ECORE_FILE_EVENT_CREATED, em->path);
	      if (em->type == ECORE_FILE_TYPE_DIRECTORY)
		{
		   /* Check for subdirs */
		   Evas_List *files, *l;

		   em->func(em->data, em, em->type, ECORE_FILE_EVENT_CREATED, em->path);
		   files = ecore_file_ls(em->path);
		   for (l = files; l; l = l->next)
		     {
			Ecore_File *f;
			char *file;
			char buf[PATH_MAX];

			file = l->data;
			snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
			f = calloc(1, sizeof(Ecore_File));
			if (!f)
			  {
			     free(file);
			     continue;
			  }

			f->name = file;
			f->mtime = ecore_file_mod_time(buf);
			f->type = ecore_file_is_dir(buf) ?
			   ECORE_FILE_TYPE_DIRECTORY :
			   ECORE_FILE_TYPE_FILE;
			em->func(em->data, em, f->type, ECORE_FILE_EVENT_CREATED, buf);
			emf->files = evas_list_append(emf->files, f);
		     }
		   evas_list_free(files);
		}
	   }
	 break;
     }
   emf->mtime = mtime;
}

static int
_ecore_file_monitor_checking(Ecore_File_Monitor *em, char *name)
{
   Evas_List *l;
   Ecore_File_Monitor_Poll *emf;

   emf = ECORE_FILE_MONITOR_POLL(em);
   for (l = emf->files; l; l = l->next)
     {
	Ecore_File *f;

	f = l->data;
	if (!strcmp(f->name, name))
	  return 1;
     }

   return 0;
}
#endif
