/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

/*
 * TODO:
 */

#ifdef HAVE_INOTIFY

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/inotify.h>

typedef struct _Ecore_File_Monitor_Inotify Ecore_File_Monitor_Inotify;

#define ECORE_FILE_MONITOR_INOTIFY(x) ((Ecore_File_Monitor_Inotify *)(x))

struct _Ecore_File_Monitor_Inotify
{
   Ecore_File_Monitor  monitor;
   int                 wd;
};

static Ecore_Fd_Handler *_fdh = NULL;
static Evas_List        *_monitors = NULL;

static int                 _ecore_file_monitor_inotify_handler(void *data, Ecore_Fd_Handler *fdh);
static Ecore_File_Monitor *_ecore_file_monitor_inotify_monitor_find(int wd);
static void                _ecore_file_monitor_inotify_events(Ecore_File_Monitor *em,
							      char *file, int mask);

int
ecore_file_monitor_inotify_init(void)
{
   int fd;

   fd = open("/dev/inotify", O_RDONLY);
   if (fd < 0)
     return 0;

   _fdh = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _ecore_file_monitor_inotify_handler,
				    NULL, NULL, NULL);
   if (!_fdh)
     {
	close(fd);
	return 0;
     }
   return 1;
}

int
ecore_file_monitor_inotify_shutdown(void)
{
   int fd;

   if (_fdh)
     {
	fd = ecore_main_fd_handler_fd_get(_fdh);
	ecore_main_fd_handler_del(_fdh);
	close(fd);
     }
   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_inotify_add(const char *path,
			       void (*func) (void *data, Ecore_File_Monitor *em,
					     Ecore_File_Event event,
					     const char *path),
			       void *data)
{
   Ecore_File_Monitor *em;
   int len;

   em = calloc(1, sizeof(Ecore_File_Monitor_Inotify));
   if (!em) return NULL;

   em->func = func;
   em->data = data;

   em->path = strdup(path);
   len = strlen(em->path);
   if (em->path[len - 1] == '/')
     em->path[len - 1] = 0;

   if (ecore_file_exists(em->path))
     {
	struct inotify_watch_request request;

	request.name = em->path;
	request.mask = IN_MODIFY|
		       IN_MOVED_FROM|IN_MOVED_TO|
		       IN_DELETE_SUBDIR|IN_DELETE_FILE|
		       IN_CREATE_SUBDIR|IN_CREATE_FILE|
		       IN_DELETE_SELF|IN_UNMOUNT;
	ECORE_FILE_MONITOR_INOTIFY(em)->wd = ioctl(ecore_main_fd_handler_fd_get(_fdh),
						   INOTIFY_WATCH, &request);
	if (ECORE_FILE_MONITOR_INOTIFY(em)->wd < 0)
	  {
	     printf("ioctl error\n");
	     ecore_file_monitor_inotify_del(em);
	     return NULL;
	  }
     }
   else
     {
	ecore_file_monitor_inotify_del(em);
	return NULL;
     }

   _monitors = evas_list_append(_monitors, em);

   return em;
}

void
ecore_file_monitor_inotify_del(Ecore_File_Monitor *em)
{
   int fd;

   _monitors = evas_list_remove(_monitors, em);

   fd = ecore_main_fd_handler_fd_get(_fdh);
   if (ECORE_FILE_MONITOR_INOTIFY(em)->wd)
     ioctl(fd, INOTIFY_IGNORE, ECORE_FILE_MONITOR_INOTIFY(em)->wd);
   free(em->path);
   free(em);
}

static int
_ecore_file_monitor_inotify_handler(void *data, Ecore_Fd_Handler *fdh)
{
   Ecore_File_Monitor *em;
   char buffer[16384];
   struct inotify_event *event;
   int i = 0;
   int event_size;
   ssize_t size;

   size = read(ecore_main_fd_handler_fd_get(fdh), buffer, sizeof(buffer));
   while (i < size)
     {
	event = (struct inotify_event *)&buffer[i];
	event_size = sizeof(struct inotify_event) + event->len;
	i += event_size;

	em = _ecore_file_monitor_inotify_monitor_find(event->wd);
	if (!em) continue;

	_ecore_file_monitor_inotify_events(em, event->name, event->mask);
     }

   return 1;
}

static Ecore_File_Monitor *
_ecore_file_monitor_inotify_monitor_find(int wd)
{
   Evas_List *l;

   for (l = _monitors; l; l = l->next)
     {
	Ecore_File_Monitor *em;

	em = l->data;
	if (ECORE_FILE_MONITOR_INOTIFY(em)->wd == wd)
	  return em;
     }
   return NULL;
}

static void
_ecore_file_monitor_inotify_events(Ecore_File_Monitor *em, char *file, int mask)
{
   char buf[PATH_MAX];
   if (file)
     snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
   else
     strcpy(buf, em->path);

   if (mask & IN_MODIFY)
     {
	if (!ecore_file_is_dir(buf))
	  em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, buf);
     }
   if (mask & IN_MOVED_FROM)
     {
	printf("MOVE_FROM ");
     }
   if (mask & IN_MOVED_TO)
     {
	printf("MOVE_TO ");
     }
   if (mask & IN_DELETE_SUBDIR)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_DIRECTORY, buf);
     }
   if (mask & IN_DELETE_FILE)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_FILE, buf);
     }
   if (mask & IN_CREATE_SUBDIR)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_CREATED_DIRECTORY, buf);
     }
   if (mask & IN_CREATE_FILE)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_CREATED_FILE, buf);
     }
   if (mask & IN_DELETE_SELF)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
     }
   if (mask & IN_UNMOUNT)
     {
	printf("UNMOUNT ");
     }
}
#endif
