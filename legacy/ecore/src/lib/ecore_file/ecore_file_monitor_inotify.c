/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_file_private.h"

/*
 * TODO:
 *
 * - Listen to these events:
 *   IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_OPEN
 * - Read all events first, then call the callbacks. This will prevent several
 *   callbacks with the typic save cycle (delete file, new file)
 */

#ifdef HAVE_INOTIFY

#ifdef HAVE_SYS_INOTIFY
# include <sys/inotify.h>
#else
# include <asm/unistd.h>
# include <linux/inotify.h>
#endif

#ifndef HAVE_SYS_INOTIFY
static inline int inotify_init(void);
static inline int inotify_add_watch(int fd, const char *name, __u32 mask);
static inline int inotify_rm_watch(int fd, __u32 wd);
#endif


typedef struct _Ecore_File_Monitor_Inotify Ecore_File_Monitor_Inotify;

#define ECORE_FILE_MONITOR_INOTIFY(x) ((Ecore_File_Monitor_Inotify *)(x))

struct _Ecore_File_Monitor_Inotify
{
   Ecore_File_Monitor  monitor;
   int                 wd;
};

static Ecore_Fd_Handler *_fdh = NULL;
static Ecore_List2    *_monitors = NULL;

static int                 _ecore_file_monitor_inotify_handler(void *data, Ecore_Fd_Handler *fdh);
static Ecore_File_Monitor *_ecore_file_monitor_inotify_monitor_find(int wd);
static void                _ecore_file_monitor_inotify_events(Ecore_File_Monitor *em, char *file, int mask);
static int                 _ecore_file_monitor_inotify_monitor(Ecore_File_Monitor *em, const char *path);
#if 0
static void                _ecore_file_monitor_inotify_print(char *file, int mask);
#endif

int
ecore_file_monitor_inotify_init(void)
{
   int fd;

   fd = inotify_init();
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
   Ecore_List2 *l;

   for (l = _monitors; l;)
     {
	Ecore_File_Monitor *em;

	em = ECORE_FILE_MONITOR(l);
	l = l->next;
	ecore_file_monitor_inotify_del(em);
     }

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
   if (em->path[len - 1] == '/' && strcmp(em->path, "/"))
     em->path[len - 1] = 0;

   _monitors = _ecore_list2_append(_monitors, em);

   if (ecore_file_exists(em->path))
     {
	if (!_ecore_file_monitor_inotify_monitor(em, em->path))
	  return NULL;
     }
   else
     {
	ecore_file_monitor_inotify_del(em);
	return NULL;
     }

   return em;
}

void
ecore_file_monitor_inotify_del(Ecore_File_Monitor *em)
{
   int fd;

   _monitors = _ecore_list2_remove(_monitors, em);

   fd = ecore_main_fd_handler_fd_get(_fdh);
   if (ECORE_FILE_MONITOR_INOTIFY(em)->wd)
     inotify_rm_watch(fd, ECORE_FILE_MONITOR_INOTIFY(em)->wd);
   free(em->path);
   free(em);
}

static int
_ecore_file_monitor_inotify_handler(void *data __UNUSED__, Ecore_Fd_Handler *fdh)
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

	_ecore_file_monitor_inotify_events(em, (event->len ? event->name : NULL), event->mask);
     }

   return 1;
}

static Ecore_File_Monitor *
_ecore_file_monitor_inotify_monitor_find(int wd)
{
   Ecore_List2 *l;

   for (l = _monitors; l; l = l->next)
     {
	Ecore_File_Monitor *em;

	em = ECORE_FILE_MONITOR(l);

	if (ECORE_FILE_MONITOR_INOTIFY(em)->wd == wd)
	  return em;
     }
   return NULL;
}

static void
_ecore_file_monitor_inotify_events(Ecore_File_Monitor *em, char *file, int mask)
{
   char buf[PATH_MAX];
   int isdir;

   if ((file) && (file[0]))
     snprintf(buf, sizeof(buf), "%s/%s", em->path, file);
   else
     strcpy(buf, em->path);
   isdir = mask & IN_ISDIR;

#if 0
   _ecore_file_monitor_inotify_print(file, mask);
#endif

   if (mask & IN_MODIFY)
     {
	if (!isdir)
	  em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, buf);
     }
   if (mask & IN_MOVED_FROM)
     {
	if (isdir)
	  em->func(em->data, em, ECORE_FILE_EVENT_DELETED_DIRECTORY, buf);
	else
	  em->func(em->data, em, ECORE_FILE_EVENT_DELETED_FILE, buf);
     }
   if (mask & IN_MOVED_TO)
     {
	if (isdir)
	  em->func(em->data, em, ECORE_FILE_EVENT_CREATED_DIRECTORY, buf);
	else
	  em->func(em->data, em, ECORE_FILE_EVENT_CREATED_FILE, buf);
     }
   if (mask & IN_DELETE)
     {
	if (isdir)
	  em->func(em->data, em, ECORE_FILE_EVENT_DELETED_DIRECTORY, buf);
	else
	  em->func(em->data, em, ECORE_FILE_EVENT_DELETED_FILE, buf);
     }
   if (mask & IN_CREATE)
     {
	if (isdir)
	  em->func(em->data, em, ECORE_FILE_EVENT_CREATED_DIRECTORY, buf);
	else
	  em->func(em->data, em, ECORE_FILE_EVENT_CREATED_FILE, buf);
     }
   if (mask & IN_DELETE_SELF)
     {
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
     }
   if (mask & IN_MOVE_SELF)
     {
	/* We just call delete. The dir is gone... */
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
     }
   if (mask & IN_UNMOUNT)
     {
	/* We just call delete. The dir is gone... */
	em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
     }
   if (mask & IN_IGNORED)
     {
	/* The watch is removed. If the file name still exists monitor the new one,
	 * else delete it */
	if (ecore_file_exists(em->path))
	  {
	     if (!_ecore_file_monitor_inotify_monitor(em, em->path))
	       em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
	  }
	else
	  em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
     }
}

static int
_ecore_file_monitor_inotify_monitor(Ecore_File_Monitor *em, const char *path)
{
   int mask;
   mask = IN_MODIFY|
	  IN_MOVED_FROM|IN_MOVED_TO|
	  IN_DELETE|IN_CREATE|
	  IN_DELETE_SELF|IN_MOVE_SELF|
	  IN_UNMOUNT;
   ECORE_FILE_MONITOR_INOTIFY(em)->wd = inotify_add_watch(ecore_main_fd_handler_fd_get(_fdh),
							  path, mask);
   if (ECORE_FILE_MONITOR_INOTIFY(em)->wd < 0)
     {
	printf("inotify_add_watch error\n");
	ecore_file_monitor_inotify_del(em);
	return 0;
     }
   return 1;
}

#ifndef HAVE_SYS_INOTIFY
static inline int
inotify_init(void)
{
   return syscall(__NR_inotify_init);
}

static inline int
inotify_add_watch(int fd, const char *name, __u32 mask)
{
   return syscall(__NR_inotify_add_watch, fd, name, mask);
}

static inline int
inotify_rm_watch(int fd, __u32 wd)
{
   return syscall(__NR_inotify_rm_watch, fd, wd);
}
#endif

#if 0
static void
_ecore_file_monitor_inotify_print(char *file, int mask)
{
   const char *type;

   if (mask & IN_ISDIR)
     type = "dir";
   else
     type = "file";

   if (mask & IN_MODIFY)
     {
	printf("Inotify modified %s: %s\n", type, file);
     }
   if (mask & IN_MOVED_FROM)
     {
	printf("Inotify moved from %s: %s\n", type, file);
     }
   if (mask & IN_MOVED_TO)
     {
	printf("Inotify moved to %s: %s\n", type, file);
     }
   if (mask & IN_DELETE)
     {
	printf("Inotify delete %s: %s\n", type, file);
     }
   if (mask & IN_CREATE)
     {
	printf("Inotify create %s: %s\n", type, file);
     }
   if (mask & IN_DELETE_SELF)
     {
	printf("Inotify delete self %s: %s\n", type, file);
     }
   if (mask & IN_MOVE_SELF)
     {
	printf("Inotify move self %s: %s\n", type, file);
     }
   if (mask & IN_UNMOUNT)
     {
	printf("Inotify unmount %s: %s\n", type, file);
     }
}
#endif
#endif /* HAVE_INOTIFY */
