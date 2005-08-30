/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

/*
 * TODO:
 * - When several subdirectories are created really fast, the code
 *   doesn't keep up!
 * - Same for deletion of files in deleted directories!
 */

#ifdef HAVE_FAM

#include <fam.h>

typedef struct _Ecore_File_Monitor_Fam Ecore_File_Monitor_Fam;
typedef struct _Ecore_File             Ecore_File;

#define ECORE_FILE_MONITOR_FAM(x) ((Ecore_File_Monitor_Fam *)(x))

struct _Ecore_File_Monitor_Fam
{
   Ecore_File_Monitor  monitor;
   FAMRequest         *request;
};

struct _Ecore_File
{
   Ecore_List2   __list_data;
   char            *name;
};

static Ecore_Fd_Handler *_fdh = NULL;
static FAMConnection    *_fc = NULL;
static Ecore_List2    *_monitors = NULL;

static int               _ecore_file_monitor_fam_handler(void *data, Ecore_Fd_Handler *fdh);
static Ecore_File       *_ecore_file_monitor_fam_file_find(Ecore_File_Monitor *em, char *name);
static Ecore_File_Event  _ecore_file_monitor_fam_event_get(FAMCodes code, int self, int is_dir);

int
ecore_file_monitor_fam_init(void)
{
   _fc = calloc(1, sizeof(FAMConnection));
   if (!_fc) return 0;

   FAMOpen(_fc);
   _fdh = ecore_main_fd_handler_add(FAMCONNECTION_GETFD(_fc), ECORE_FD_READ,
				    _ecore_file_monitor_fam_handler, NULL, NULL, NULL);

   return 1;
}

int
ecore_file_monitor_fam_shutdown(void)
{
   Ecore_List2 *l;

   for (l = _monitors; l;)
     {
	Ecore_File_Monitor *em;

	em = ECORE_FILE_MONITOR(l);
	l = l->next;
	ecore_file_monitor_fam_del(em);
     }
   if (_fdh) ecore_main_fd_handler_del(_fdh);
   if (_fc)
     {
       	FAMClose(_fc);
	free(_fc);
     }
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_fam_add(const char *path,
			   void (*func) (void *data,
					 Ecore_File_Monitor *em,
					 Ecore_File_Event event,
					 const char *path),
			   void *data)
{
   Ecore_File_Monitor *em;
   int len;

   em = calloc(1, sizeof(Ecore_File_Monitor_Fam));
   if (!em) return NULL;

   em->func = func;
   em->data = data;

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = 0;

   if (ecore_file_exists(em->path))
     {
	ECORE_FILE_MONITOR_FAM(em)->request = calloc(1, sizeof(FAMRequest));
	if (!ECORE_FILE_MONITOR_FAM(em)->request)
	  {
	     ecore_file_monitor_fam_del(em);
	     return NULL;
	  }
	if (ecore_file_is_dir(em->path))
	  {
	     FAMMonitorDirectory(_fc, em->path, ECORE_FILE_MONITOR_FAM(em)->request, em);
	  }
	else
	  {
	     FAMMonitorFile(_fc, em->path, ECORE_FILE_MONITOR_FAM(em)->request, em);
	  }
     }
   else
     {
	ecore_file_monitor_fam_del(em);
	return NULL;
     }

   _monitors = _ecore_list2_append(_monitors, em);

   return em;
}

void
ecore_file_monitor_fam_del(Ecore_File_Monitor *em)
{
   Ecore_List2 *l;

   for (l = em->files; l; l = l->next)
     {
	Ecore_File *f;
	
	f = (Ecore_File *)l;
	free(f->name);
	free(f);
     }

   _monitors = _ecore_list2_remove(_monitors, em);

   if (ECORE_FILE_MONITOR_FAM(em)->request)
     {
	FAMCancelMonitor(_fc, ECORE_FILE_MONITOR_FAM(em)->request);
	free(ECORE_FILE_MONITOR_FAM(em)->request);
     }
   free(em->path);
   free(em);
}

static int
_ecore_file_monitor_fam_handler(void *data, Ecore_Fd_Handler *fdh)
{
   int pending, i;

   while ((pending = FAMPending(_fc)))
     {
	for (i = 0; i < pending; i++)
	  {
	     Ecore_File_Monitor *em;
	     FAMEvent fe;
	     Ecore_File_Event event;
	     char buf[PATH_MAX];
	     int len, self;

	     buf[0] = 0;

	     FAMNextEvent(_fc, &fe);
	     len = strlen(fe.filename);
	     if (fe.filename[len - 1] == '/')
	       fe.filename[len - 1] = 0;
	     self = !strcmp(em->path, fe.filename);
	     if (!self)
	       snprintf(buf, sizeof(buf), "%s/%s", em->path, fe.filename);

	     event = _ecore_file_monitor_fam_event_get(fe.code, self, ecore_file_is_dir(buf));
	     em = fe.userdata;
	     if (!em) continue;
	     if (event == ECORE_FILE_EVENT_NONE) continue;
#if 0
	     if (!strcmp(em->path, fe.filename))
	       {
		  Evas_List *l;

		  if (event == ECORE_FILE_EVENT_DELETED)
		    {
		       /* Notify all files deleted */
		       for (l = em->files; l;)
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
		       em->files = evas_list_free(em->files);
		       em->func(em->data, em, em->type, event, em->path);
		       em->type = ECORE_FILE_TYPE_NONE;
		    }
		  else
		    {
		       em->func(em->data, em, em->type, event, em->path);
		    }
	       }
	     else
	       {
		  Ecore_File *f;

		  switch (event)
		    {
		     case ECORE_FILE_EVENT_NONE:
			break;
		     case ECORE_FILE_EVENT_EXISTS:
			f = _ecore_file_monitor_fam_file_find(em, fe.filename);
			if (f)
			  {
			     em->func(em->data, em, f->type, event, buf);
			     break;
			  }
		     case ECORE_FILE_EVENT_CREATED:
			f = calloc(1, sizeof(Ecore_File));
			if (!f) break;

			f->type = ecore_file_is_dir(buf) ?
				  ECORE_FILE_TYPE_DIRECTORY :
				  ECORE_FILE_TYPE_FILE;
			f->name = strdup(fe.filename);
			em->files = evas_list_append(em->files, f);
			em->func(em->data, em, f->type, event, buf);
			break;
		     case ECORE_FILE_EVENT_DELETED:
			f = _ecore_file_monitor_fam_file_find(em, fe.filename);
			if (f)
			  {
			     em->files = evas_list_remove(em->files, f);
			     em->func(em->data, em, f->type, event, buf);
			     free(f->name);
			     free(f);
			  }
			break;
		     case ECORE_FILE_EVENT_CHANGED:
			em->func(em->data, em, f->type, event, buf);
			break;
		    }
	       }
#endif
	  }
   }
   return 1;
}

static Ecore_File *
_ecore_file_monitor_fam_file_find(Ecore_File_Monitor *em, char *name)
{
   for (l = em->files; l; l = l->next)
     {
	Ecore_File *f;
	f = l->data;
	if (!strcmp(f->name, name))
	  return f;
     }
   return NULL;
}

static Ecore_File_Event
_ecore_file_monitor_fam_event_get(FAMCodes code, int self, int is_dir)
{
   switch (code)
     {
      case FAMCreated:
	 if (self)
	   return ECORE_FILE_EVENT_NONE;
	 else if (is_dir)
	   return ECORE_FILE_EVENT_CREATED_DIRECTORY;
	 else
	   return ECORE_FILE_EVENT_CREATED_FILE;
	 break;
      case FAMDeleted:
	 if (self)
	   return ECORE_FILE_EVENT_DELETED_SELF;
	 else if (is_dir)
	   return ECORE_FILE_EVENT_DELETED_DIRECTORY;
	 else
	   return ECORE_FILE_EVENT_DELETED_FILE;
	 break;
      case FAMChanged:
	 if (!is_dir)
	   return ECORE_FILE_EVENT_MODIFIED;
	 break;
      case FAMExists:
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
