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

static Eina_Bool
_eio_inotify_handler(void *data, Ecore_Fd_Handler *fdh)
{
   return EINA_TRUE;
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
