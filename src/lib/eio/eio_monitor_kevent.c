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

   int fd;
};

static Ecore_Fd_Handler *_kqueue_fd = NULL;
static Eina_Hash *_kevent_monitors = NULL;

static void
_eio_kevent_del(void *data)
{
   Eio_Monitor_Backend *emb = data;

   if (emb->fd)
      close(emb->fd);

   free(emb);
}

static Eina_Bool
_eio_kevent_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   Eio_Monitor_Backend *backend;
   struct kevent evs[KEVENT_NUM_EVENTS];
   int event_code = 0;

   int res = kevent(ecore_main_fd_handler_fd_get(fdh), 0, 0, evs, KEVENT_NUM_EVENTS, 0);

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
        eio_monitor_fallback_add(monitor);
        return;
     }

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend) return;
   
   res = stat(monitor->path, &st);
   if (res) goto error;

   if (S_ISDIR(st.st_mode)) // let poller handle directories
     {
        eio_monitor_fallback_add(monitor);
        goto error;
     }

   fd = open(monitor->path, O_RDONLY);
   if (fd < 0) goto error;

   backend->fd = fd;
   backend->parent = monitor;
   monitor->backend = backend;

   eina_hash_direct_add(_kevent_monitors, &backend->fd, backend);

   EV_SET(&e, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
          NOTE_DELETE | NOTE_WRITE | NOTE_ATTRIB, 0, NULL);
   res = kevent(ecore_main_fd_handler_fd_get(_kqueue_fd), &e, 1, 0, 0, 0);
   if (res)
     {
        eina_hash_del(_kevent_monitors, &backend->fd, backend);
        eio_monitor_fallback_add(monitor);
     }

   return;
   
error:
   free(backend);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;
   
   if (monitor->fallback)
     {
        eio_monitor_fallback_del(monitor);
        return;
     }

   backend = monitor->backend;
   monitor->backend = NULL;

   eina_hash_del(_kevent_monitors, &backend->fd, backend);
}


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
