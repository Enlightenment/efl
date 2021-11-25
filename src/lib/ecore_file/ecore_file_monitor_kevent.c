#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ecore_file_private.h"

#define KEVENT_NUM_EVENTS 5

typedef struct _Ecore_File_Monitor_Kevent Ecore_File_Monitor_Kevent;

#define ECORE_FILE_MONITOR_KEVENT(x) ((Ecore_File_Monitor_Kevent *)(x))

struct _Ecore_File_Monitor_Kevent
{
   Ecore_File_Monitor  monitor;
   Eina_List          *prev;
   int                 fd;
};

typedef struct _File_Info File_Info;
struct _File_Info
{
   const char *path;
   Eina_Stat   st;
};

static Ecore_Fd_Handler   *_kevent_fdh = NULL;
static Eina_Hash          *_kevent_monitors = NULL;

static Eina_Bool           _ecore_file_monitor_kevent_handler(void *data, Ecore_Fd_Handler *fdh);
static int                 _ecore_file_monitor_kevent_monitor(Ecore_File_Monitor *em, const char *path);
static void                _ecore_file_monitor_kevent_find(Ecore_File_Monitor *em);
static void                _ecore_file_monitor_kevent_hash_del_cb(void *data);
static Eina_List *         _ecore_file_monitor_kevent_ls(const char *directory);
static void                _ecore_file_monitor_kevent_ls_free(Eina_List *);

int
ecore_file_monitor_backend_init(void)
{
   int fd;

   if (_kevent_fdh != NULL) return 0;

   fd = kqueue();
   if (fd < 0)
     return 0;

   _kevent_fdh = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _ecore_file_monitor_kevent_handler,
                                           NULL, NULL, NULL);
   if (!_kevent_fdh)
     {
        close(fd);
        return 0;
     }

   _kevent_monitors = eina_hash_int32_new(_ecore_file_monitor_kevent_hash_del_cb);
   return 1;
}

int
ecore_file_monitor_backend_shutdown(void)
{
   int fd;

   if (!_kevent_fdh) return 1;

   eina_hash_free(_kevent_monitors);

   fd = ecore_main_fd_handler_fd_get(_kevent_fdh);
   ecore_main_fd_handler_del(_kevent_fdh);
   _kevent_fdh = NULL;

   if (fd != -1)
     close(fd);

   return 1;
}

Ecore_File_Monitor *
ecore_file_monitor_backend_add(const char *path,
                               void (*func) (void *data, Ecore_File_Monitor *em,
                                             Ecore_File_Event event,
                                             const char *path),
                               void *data)
{
   Ecore_File_Monitor *em;
   char *path2;
   size_t len;

   if (!path) return NULL;
   if (!func) return NULL;

   em = (Ecore_File_Monitor *)calloc(1, sizeof(Ecore_File_Monitor_Kevent));
   if (!em) return NULL;

   em->func = func;
   em->data = data;

   len = strlen(path);
   path2 = alloca(len + 1);
   strcpy(path2, path);
   if (path2[len - 1] == '/' && strcmp(path2, "/")) path2[len - 1] = 0;
   em->path = eina_stringshare_add(path2);

   if (!_ecore_file_monitor_kevent_monitor(em, em->path))
     return NULL;

   return em;
}

static Eina_List *
_ecore_file_monitor_kevent_ls(const char *directory)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;
   Eina_List *files = NULL;

   it = eina_file_direct_ls(directory);
   if (!it) return NULL;
   EINA_ITERATOR_FOREACH(it, info)
     {
        File_Info *file = malloc(sizeof(File_Info));
        if (eina_file_statat(eina_iterator_container_get(it), info, &file->st))
          {
             free(file);
             continue;
          }
        file->path = eina_stringshare_add(info->path);
        files = eina_list_append(files, file);
     }

   eina_iterator_free(it);

   return files;
}

static void
_ecore_file_monitor_kevent_ls_free(Eina_List *list)
{
   File_Info *file;

   EINA_LIST_FREE(list, file)
     {
        eina_stringshare_del(file->path);
        free(file);
     }
}

static void
_ecore_file_monitor_kevent_hash_del_cb(void *data)
{
   Ecore_File_Monitor *em = data;

   if (ECORE_FILE_MONITOR_KEVENT(em)->fd >= 0)
     close(ECORE_FILE_MONITOR_KEVENT(em)->fd);
   eina_stringshare_del(em->path);
   _ecore_file_monitor_kevent_ls_free(ECORE_FILE_MONITOR_KEVENT(em)->prev);

   free(em);
}

void
ecore_file_monitor_backend_del(Ecore_File_Monitor *em)
{
   eina_hash_del(_kevent_monitors, &(ECORE_FILE_MONITOR_KEVENT(em)->fd), em);
}

static Eina_Bool
_ecore_file_monitor_kevent_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   Ecore_File_Monitor *em;
   struct kevent evs[KEVENT_NUM_EVENTS];
   int fd;
   const struct timespec timeout = { 0, 0 };

   fd = ecore_main_fd_handler_fd_get(fdh);
   if (fd < 0) return ECORE_CALLBACK_RENEW;

   int res = kevent(fd, 0, 0, evs, KEVENT_NUM_EVENTS, &timeout);
   for (int i = 0; i < res; i++)
     {
        em = eina_hash_find(_kevent_monitors, &evs[i].ident);
        if (evs[i].fflags & NOTE_DELETE)
          {
             em->func(em->data, em, ECORE_FILE_EVENT_DELETED_SELF, em->path);
          }
        if ((evs[i].fflags & NOTE_WRITE) || (evs[i].fflags & NOTE_ATTRIB))
          {
             if (ecore_file_is_dir(em->path))
               _ecore_file_monitor_kevent_find(em);
             else
               em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, em->path);
          }
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_file_monitor_kevent_find(Ecore_File_Monitor *em)
{
   Eina_List *l, *l2;
   File_Info *file, *file2;
   Eina_List *files;

   files = _ecore_file_monitor_kevent_ls(em->path);
   EINA_LIST_FOREACH(ECORE_FILE_MONITOR_KEVENT(em)->prev, l, file)
     {
        Eina_Bool exists = EINA_FALSE;
        EINA_LIST_FOREACH(files, l2, file2)
          {
             if (file->st.ino == file2->st.ino)
               {
                  if (file->path == file2->path)
                    exists = EINA_TRUE;

                  if (file->st.mtime != file2->st.mtime)
                    em->func(em->data, em, ECORE_FILE_EVENT_MODIFIED, file->path);
               }
          }

        if (!exists)
          {
             if (S_ISDIR(file->st.mode))
               em->func(em->data, em, ECORE_FILE_EVENT_DELETED_DIRECTORY, file->path);
             else
               em->func(em->data, em, ECORE_FILE_EVENT_DELETED_FILE, file->path);
          }
     }

   EINA_LIST_FOREACH(files, l, file)
     {
        Eina_Bool exists = EINA_FALSE;
        EINA_LIST_FOREACH(ECORE_FILE_MONITOR_KEVENT(em)->prev, l2, file2)
          {
             if ((file->path == file2->path) && (file->st.ino == file2->st.ino))
               {
                  exists = EINA_TRUE;
                  break;
               }
          }

        if (!exists)
          {
             if (S_ISDIR(file->st.mode))
               em->func(em->data, em, ECORE_FILE_EVENT_CREATED_DIRECTORY, file->path);
             else
               em->func(em->data, em, ECORE_FILE_EVENT_CREATED_FILE, file->path);
          }
     }

   _ecore_file_monitor_kevent_ls_free(ECORE_FILE_MONITOR_KEVENT(em)->prev);
   ECORE_FILE_MONITOR_KEVENT(em)->prev = files;
}

static int
_ecore_file_monitor_kevent_monitor(Ecore_File_Monitor *em, const char *path)
{
   struct kevent ev;
   int fd, res = 0;

   if (!ecore_file_exists(path))
     return 0;

   fd = open(path, O_RDONLY);
   if (fd < 0)
     {
        INF("open failed, %s", strerror(errno));
        ecore_file_monitor_backend_del(em);
        return 0;
     }

   eina_file_close_on_exec(fd, EINA_TRUE);

   ECORE_FILE_MONITOR_KEVENT(em)->fd = fd;
   if (ecore_file_is_dir(em->path))
     ECORE_FILE_MONITOR_KEVENT(em)->prev = _ecore_file_monitor_kevent_ls(em->path);

   eina_hash_direct_add(_kevent_monitors, &(ECORE_FILE_MONITOR_KEVENT(em)->fd), em);

   EV_SET(&ev, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
          NOTE_DELETE | NOTE_WRITE | NOTE_ATTRIB, 0, NULL);
   res = kevent(ecore_main_fd_handler_fd_get(_kevent_fdh), &ev, 1, 0, 0, 0);
   if (res)
     eina_hash_del(_kevent_monitors, &(ECORE_FILE_MONITOR_KEVENT(em)->fd), em);

   return 1;
}
