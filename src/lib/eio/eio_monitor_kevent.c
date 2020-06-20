/* EIO - EFL data type library
 * Copyright (C) 2011 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include "eio_private.h"
#include "Eio.h"
#include "Eina.h"
#include "Ecore_File.h"
#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define KEVENT_NUM_EVENTS 5

struct _Eio_Monitor_Backend
{
   Eio_Monitor *parent;
   Eina_List *prev_list;
   int fd;
};

typedef struct _Eio_File_Info Eio_File_Info;
struct _Eio_File_Info
{
   const char *path;
   Eina_Stat st;
};

static Ecore_Fd_Handler *_kqueue_fd = NULL;
static Eina_Hash *_kevent_monitors = NULL;

static void
_eio_kevent_ls_free(Eina_List *list)
{
   Eio_File_Info *file;

   EINA_LIST_FREE(list, file)
     {
        eina_stringshare_del(file->path);
        free(file);
     }
}

static void
_eio_kevent_del(void *data)
{
   Eio_Monitor_Backend *emb = data;

   _eio_kevent_ls_free(emb->prev_list);

   if (emb->fd)
      close(emb->fd);

   free(emb);
}


static Eina_List *
_eio_kevent_ls(const char *directory)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *info;
   Eina_List *files = NULL;

   it = eina_file_direct_ls(directory);
   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, info)
     {
        Eio_File_Info *file = malloc(sizeof(Eio_File_Info));
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
_eio_kevent_event_find(Eio_Monitor_Backend *backend)
{
   Eina_List *l, *l2;
   Eio_File_Info *file, *file2;
   Eina_List *next_list = _eio_kevent_ls(backend->parent->path);

   EINA_LIST_FOREACH(backend->prev_list, l, file)
     {
        Eina_Bool exists = EINA_FALSE;
        EINA_LIST_FOREACH(next_list, l2, file2)
          {
             if (file->st.ino == file2->st.ino)
               {
                  if (file->path == file2->path)
                    exists = EINA_TRUE;

                  if (file->st.mtime != file2->st.mtime)
                    {
                       if (S_ISDIR(file->st.mode))
                         _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_DIRECTORY_MODIFIED);
                       else
                         _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_FILE_MODIFIED);
                    }
               }
          }

        if (!exists)
          {
             if (S_ISDIR(file->st.mode))
               _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_DIRECTORY_DELETED);
             else
               _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_FILE_DELETED);
          }
     }

   EINA_LIST_FOREACH(next_list, l, file)
     {
        Eina_Bool exists = EINA_FALSE;
        EINA_LIST_FOREACH(backend->prev_list, l2, file2)
          {
             if ((file->path == file2->path) &&
                         (file->st.ino == file2->st.ino))
               {
                  exists = EINA_TRUE;
                  break;
               }
          }

        if (!exists)
          {
             if (S_ISDIR(file->st.mode))
               _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_DIRECTORY_CREATED);
             else
               _eio_monitor_send(backend->parent, file->path, EIO_MONITOR_FILE_CREATED);
          }
     }

   _eio_kevent_ls_free(backend->prev_list);

   backend->prev_list = next_list;
}

static Eina_Bool
_eio_kevent_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   Eio_Monitor_Backend *backend;
   struct kevent evs[KEVENT_NUM_EVENTS];
   int event_code = 0;
   const struct timespec timeout = { 0, 0 };

   int res = kevent(ecore_main_fd_handler_fd_get(fdh), 0, 0, evs, KEVENT_NUM_EVENTS, &timeout);

   for(int i=0; i<res; ++i)
     {
        backend = eina_hash_find(_kevent_monitors, &evs[i].ident);
        if(evs[i].fflags & NOTE_DELETE)
          {
             event_code = EIO_MONITOR_SELF_DELETED;
             _eio_monitor_send(backend->parent, backend->parent->path, event_code);
          }
        if(evs[i].fflags & NOTE_WRITE || evs[i].fflags & NOTE_ATTRIB)
          {
             /* Handle directory/file creation and deletion */
             _eio_kevent_event_find(backend);
             /* Old default behaviour */
             event_code = EIO_MONITOR_FILE_MODIFIED;
             _eio_monitor_send(backend->parent, backend->parent->path, event_code);
          }
     }

   return ECORE_CALLBACK_RENEW;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

/**
 * @endcond
 */

void eio_monitor_backend_init(void)
{
   int fd;

   if (_kqueue_fd != NULL) return; // already initialized

   fd = kqueue();
   if (fd < 0) return;

   _kqueue_fd = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _eio_kevent_handler, NULL, NULL, NULL);
   if (!_kqueue_fd)
     {
        close(fd);
        return;
     }

   _kevent_monitors = eina_hash_int32_new(_eio_kevent_del);
}

void eio_monitor_backend_shutdown(void)
{
   int fd;

   if (!_kqueue_fd) return;

   eina_hash_free(_kevent_monitors);

   fd = ecore_main_fd_handler_fd_get(_kqueue_fd);
   ecore_main_fd_handler_del(_kqueue_fd);
   _kqueue_fd = NULL;

   if (fd < 0)
     return;

   close(fd);
}

void eio_monitor_backend_add(Eio_Monitor *monitor)
{
   struct kevent e;
   struct stat st;
   Eio_Monitor_Backend* backend;
   int fd, res = 0;

   if (!_kqueue_fd)
     {
        return;
     }

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend) return;

   res = stat(monitor->path, &st);
   if (res) goto error;

   fd = open(monitor->path, O_RDONLY);
   if (fd < 0) goto error;

   eina_file_close_on_exec(fd, EINA_TRUE);
   backend->fd = fd;
   backend->parent = monitor;
   monitor->backend = backend;

   backend->prev_list = _eio_kevent_ls(backend->parent->path);

   eina_hash_direct_add(_kevent_monitors, &backend->fd, backend);

   EV_SET(&e, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
          NOTE_DELETE | NOTE_WRITE | NOTE_ATTRIB, 0, NULL);
   res = kevent(ecore_main_fd_handler_fd_get(_kqueue_fd), &e, 1, 0, 0, 0);
   if (res)
     {
        eina_hash_del(_kevent_monitors, &backend->fd, backend);
     }

   return;

error:
   free(backend);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   backend = monitor->backend;
   monitor->backend = NULL;

   eina_hash_del(_kevent_monitors, &backend->fd, backend);
}

Eina_Bool eio_monitor_context_check(const Eio_Monitor *monitor, const char *path)
{
   Eio_Monitor_Backend *backend = monitor->backend;
   Eina_List *l;
   Eio_File_Info *file;

   EINA_LIST_FOREACH(backend->prev_list, l, file)
     {
        if (eina_streq(file->path, path))
          {
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}



/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
