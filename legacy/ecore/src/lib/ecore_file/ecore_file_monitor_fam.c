/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"
#include "Ecore_File.h"

/*
 * TODO:
 * - When several subdirectories are created really fast, the code
 *   doesn't keep up! Putting in a random printf() makes it work..
 * - Same for deletion of files in deleted directories!
 */

#ifdef HAVE_FAM

#include <fam.h>

typedef struct _Ecore_File_Monitor_Fam Ecore_File_Monitor_Fam;
typedef struct _Ecore_File             Ecore_File;

struct _Ecore_File_Monitor_Fam
{
   Ecore_File_Monitor monitor;
   FAMRequest         *request;
   Evas_List          *files;
};

struct _Ecore_File
{
   char *name;
   Ecore_File_Type  type;
};

#define ECORE_FILE_MONITOR_FAM(x) ((Ecore_File_Monitor_Fam *)(x))

static Ecore_Fd_Handler *_fdh = NULL;
static FAMConnection    *_fc = NULL;
static Evas_List        *_monitors = NULL;

static int                         _ecore_file_monitor_handler(void *data, Ecore_Fd_Handler *fdh);
static Ecore_File                * _ecore_file_monitor_file_find(Ecore_File_Monitor *em, char *name);
static Ecore_File_Event            _ecore_file_monitor_event_get(FAMCodes change);

int
ecore_file_monitor_init(void)
{
   _fc = calloc(1, sizeof(FAMConnection));
   if (!_fc) return 0;

   FAMOpen(_fc);
   _fdh = ecore_main_fd_handler_add(FAMCONNECTION_GETFD(_fc), ECORE_FD_READ, _ecore_file_monitor_handler,
				    NULL, NULL, NULL);

   return 1;
}

int
ecore_file_monitor_shutdown(void)
{
   Evas_List *l;
   for (l = _monitors; l; l = l->next)
     ecore_file_monitor_del(ECORE_FILE_MONITOR(l->data));
   evas_list_free(_monitors);
   if (_fdh) ecore_main_fd_handler_del(_fdh);
   if (_fc)
     {
       	FAMClose(_fc);
	free(_fc);
     }
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_add(const char *path,
		       void (*func) (void *data,
				     Ecore_File_Monitor *em,
				     Ecore_File_Type type,
				     Ecore_File_Event event,
				     const char *path),
		       void *data)
{
   Ecore_File_Monitor *em;
   Ecore_File_Monitor_Fam *emf;
   int len;

   emf = calloc(1, sizeof(Ecore_File_Monitor_Fam));
   em = ECORE_FILE_MONITOR(emf);
   if (!em) return NULL;

   em->func = func;
   em->data = data;

   _monitors = evas_list_append(_monitors, em);

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = '\0';

   if (ecore_file_exists(em->path))
     {
	em->type = ecore_file_is_dir(em->path) ?
		   ECORE_FILE_TYPE_DIRECTORY :
		   ECORE_FILE_TYPE_FILE;

	/* TODO: Check if calloc succeded! */
	emf->request = calloc(1, sizeof(FAMRequest));
	if (em->type == ECORE_FILE_TYPE_DIRECTORY)
	  {
	     FAMMonitorDirectory(_fc, em->path, emf->request, em);
	  }
	else
	  {
	     FAMMonitorFile(_fc, em->path, emf->request, em);
	     em->func(em->data, em, em->type, ECORE_FILE_EVENT_EXISTS, em->path);
	  }
     }
   else
     {
	em->type = ECORE_FILE_TYPE_NONE;
	em->func(em->data, em, em->type, ECORE_FILE_EVENT_DELETED, em->path);
     }

   return em;
}

void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   Ecore_File_Monitor_Fam *emf;
   Evas_List *l;

   emf = ECORE_FILE_MONITOR_FAM(em);
   for (l = emf->files; l; l = l->next)
     {
	Ecore_File *f;
	
	f = l->data;
	free(f->name);
	free(f);
     }
   evas_list_free(emf->files);

   _monitors = evas_list_remove(_monitors, em);

   FAMCancelMonitor(_fc, emf->request);
   free(emf->request);
   free(em->path);
   free(em);
}

static int
_ecore_file_monitor_handler(void *data, Ecore_Fd_Handler *fdh)
{
   int pending, i;

   while ((pending = FAMPending(_fc)))
     {
	for (i = 0; i < pending; i++)
	  {
	     Ecore_File_Monitor *em;
	     Ecore_File_Monitor_Fam *emf;
	     FAMEvent fe;
	     Ecore_File_Event event;
	     char buf[PATH_MAX];

	     FAMNextEvent(_fc, &fe);
	     event = _ecore_file_monitor_event_get(fe.code);
	     em = fe.userdata;
	     emf = ECORE_FILE_MONITOR_FAM(em);
	     if (!em) continue;
	     if (event == ECORE_FILE_EVENT_NONE) continue;
	     if ((em->type == ECORE_FILE_TYPE_DIRECTORY)
		 && !strcmp(em->path, fe.filename))
	       continue;
	     /* Create path */
	     snprintf(buf, sizeof(buf), "%s/%s", em->path, fe.filename);
	     if (event == ECORE_FILE_EVENT_DELETED)
	       {
		  Ecore_File *f;

		  f = _ecore_file_monitor_file_find(em, fe.filename);
		  if (f)
		    {
		       emf->files = evas_list_remove(emf->files, f);
		       em->func(em->data, em, f->type, event, buf);
		       free(f->name);
		       free(f);
		    }
	       }
	     else
	       {
		  Ecore_File *f;

		  f = calloc(1, sizeof(Ecore_File));
		  if (!f) continue;

		  f->type = ecore_file_is_dir(buf) ?
			    ECORE_FILE_TYPE_DIRECTORY :
			    ECORE_FILE_TYPE_FILE;
		  f->name = strdup(fe.filename);
		  emf->files = evas_list_append(emf->files, f);
		  em->func(em->data, em, f->type, event, buf);
	       }
	  }
   }
   return 1;
}

static Ecore_File *
_ecore_file_monitor_file_find(Ecore_File_Monitor *em, char *name)
{
   Ecore_File_Monitor_Fam *emf;
   Evas_List *l;

   emf = ECORE_FILE_MONITOR_FAM(em);
   for (l = emf->files; l; l = l->next)
     {
	Ecore_File *f;
	f = l->data;
	if (!strcmp(f->name, name))
	  return f;
     }
   return NULL;
}

static Ecore_File_Event
_ecore_file_monitor_event_get(FAMCodes code)
{
   switch (code)
     {
      case FAMCreated:
	 return ECORE_FILE_EVENT_CREATED;
      case FAMDeleted:
	 return ECORE_FILE_EVENT_DELETED;
      case FAMChanged:
	 return ECORE_FILE_EVENT_CHANGED;
      case FAMExists:
	 return ECORE_FILE_EVENT_EXISTS;
      case FAMStartExecuting:
      case FAMStopExecuting:
      case FAMMoved:
      case FAMAcknowledge:
      case FAMEndExist:
	 return ECORE_FILE_EVENT_NONE;
     }
   return ECORE_FILE_EVENT_NONE;
}
#endif
