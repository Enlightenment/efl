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

#ifdef HAVE_SYS_INOTIFY
# include <sys/inotify.h>
#else
# include <asm/unistd.h>
# include <linux/inotify.h>
#endif

struct _Eio_Monitor_Backend
{
   Eio_Monitor *parent;

   int hwnd;
};

static Ecore_Fd_Handler *_inotify_fdh = NULL;
static Eina_Hash *_inotify_monitors = NULL;

static void
_eio_inotify_del(void *data)
{
   Eio_Monitor_Backend *emb = data;
   int fd;

   if (emb->hwnd)
     {
        fd = ecore_main_fd_handler_fd_get(_inotify_fdh);
        inotify_rm_watch(fd, emb->hwnd);
        emb->hwnd = 0;
     }

   free(emb);
}

typedef struct _Eio_Inotify_Table Eio_Inotify_Table;
struct _Eio_Inotify_Table
{
   int mask;
   int *ev_file_code;
   int *ev_dir_code;
};

#define EIO_INOTIFY_LINE(Ino, Ef, Ed)		\
  { Ino, &EIO_MONITOR_##Ef, &EIO_MONITOR_##Ed }

static const Eio_Inotify_Table match[] = {
  EIO_INOTIFY_LINE(IN_ATTRIB, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_INOTIFY_LINE(IN_CLOSE_WRITE, FILE_CLOSED, DIRECTORY_CLOSED),
  EIO_INOTIFY_LINE(IN_MODIFY, FILE_MODIFIED, DIRECTORY_MODIFIED),
  EIO_INOTIFY_LINE(IN_MOVED_FROM, FILE_DELETED, DIRECTORY_DELETED),
  EIO_INOTIFY_LINE(IN_MOVED_TO, FILE_CREATED, DIRECTORY_CREATED),
  EIO_INOTIFY_LINE(IN_DELETE, FILE_DELETED, DIRECTORY_DELETED),
  EIO_INOTIFY_LINE(IN_CREATE, FILE_CREATED, DIRECTORY_CREATED),
  EIO_INOTIFY_LINE(IN_DELETE_SELF, SELF_DELETED, SELF_DELETED),
  EIO_INOTIFY_LINE(IN_MOVE_SELF, SELF_DELETED, SELF_DELETED),
  EIO_INOTIFY_LINE(IN_UNMOUNT, SELF_DELETED, SELF_DELETED)
};

static void
_eio_inotify_events(Eio_Monitor_Backend *backend, const char *file, int mask)
{
   char *tmp;
   unsigned int length;
   unsigned int tmp_length;
   unsigned int i;
   Eina_Bool is_dir;

   length = file ? strlen(file) : 0;
   tmp_length = eina_stringshare_strlen(backend->parent->path) + length + 2;
   tmp = alloca(sizeof (char) * tmp_length);

   snprintf(tmp, tmp_length, length ? "%s/%s" : "%s",
	    backend->parent->path, file);

   is_dir = !!(mask & IN_ISDIR);

   for (i = 0; i < sizeof (match) / sizeof (Eio_Inotify_Table); ++i)
     if (match[i].mask & mask)
       {
          _eio_monitor_send(backend->parent, tmp, is_dir ? *match[i].ev_dir_code : *match[i].ev_file_code);
       }

   /* special case for IN_IGNORED */
   if (mask & IN_IGNORED)
     {
        _eio_monitor_rename(backend->parent, tmp);
     }
}

static Eina_Bool
_eio_inotify_handler(void *data __UNUSED__, Ecore_Fd_Handler *fdh)
{
   Eio_Monitor_Backend *backend;
   unsigned char buffer[16384];
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

        backend = eina_hash_find(_inotify_monitors, &event->wd);
        if (!backend) continue ;
        if (!backend->parent) continue ;

        _eio_inotify_events(backend, (event->len ? event->name : NULL), event->mask);
     }

   return ECORE_CALLBACK_RENEW;
}

void eio_monitor_backend_init(void)
{
   int fd;

   fd = inotify_init();
   if (fd < 0)
     return ;

   _inotify_fdh = ecore_main_fd_handler_add(fd, ECORE_FD_READ, _eio_inotify_handler, NULL, NULL, NULL);
   if (!_inotify_fdh)
     {
        close(fd);
        return ;
     }

   _inotify_monitors = eina_hash_int32_new(_eio_inotify_del);
}

void eio_monitor_backend_shutdown(void)
{
   int fd;

   if (!_inotify_fdh) return ;

   eina_hash_free(_inotify_monitors);

   fd = ecore_main_fd_handler_fd_get(_inotify_fdh);
   ecore_main_fd_handler_del(_inotify_fdh);
   _inotify_fdh = NULL;

   close(fd);
}

void eio_monitor_backend_add(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;
   int mask =
     IN_ATTRIB |
     IN_CLOSE_WRITE |
     IN_MOVED_FROM |
     IN_MOVED_TO |
     IN_DELETE |
     IN_CREATE |
     IN_MODIFY |
     IN_DELETE_SELF |
     IN_MOVE_SELF |
     IN_UNMOUNT;

   if (!_inotify_fdh)
     eio_monitor_fallback_add(monitor);

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend) return eio_monitor_fallback_add(monitor);

   backend->parent = monitor;
   backend->hwnd = inotify_add_watch(ecore_main_fd_handler_fd_get(_inotify_fdh), monitor->path, mask);
   if (!backend->hwnd)
     return eio_monitor_fallback_add(monitor);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   if (!_inotify_fdh)
     eio_monitor_fallback_del(monitor);

   if (!monitor->backend) return ;

   eina_hash_del(_inotify_monitors, &monitor->backend->hwnd, monitor->backend);
   monitor->backend = NULL;
}
