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

EAPI int EIO_MONITOR_ERROR;
EAPI int EIO_MONITOR_FILE_CREATED;
EAPI int EIO_MONITOR_FILE_DELETED;
EAPI int EIO_MONITOR_FILE_MODIFIED;
EAPI int EIO_MONITOR_FILE_START;
EAPI int EIO_MONITOR_FILE_STOP;
EAPI int EIO_MONITOR_DIRECTORY_CREATED;
EAPI int EIO_MONITOR_DIRECTORY_DELETED;
EAPI int EIO_MONITOR_DIRECTORY_MODIFIED;
EAPI int EIO_MONITOR_DIRECTORY_START;
EAPI int EIO_MONITOR_DIRECTORY_STOP;

static Eina_Hash *_eio_monitors = NULL;
static pid_t _monitor_pid = -1;

static void
_eio_monitor_del(void *data)
{
   Eio_Monitor *monitor = data;

   if (monitor->exist) eio_file_cancel(monitor->exist);

   if (!monitor->fallback)
     eio_monitor_backend_del(monitor);
   else
     eio_monitor_fallback_del(monitor);

   if (monitor->refcount > 0)
     return ;

   eina_stringshare_del(monitor->path);
   free(monitor);
}

static void
_eio_monitor_cleanup_cb(void *user_data, __UNUSED__ void *func_data)
{
   Eio_Monitor_Error *ev = user_data;

   ev->monitor->refcount--;

   _eio_monitor_del(ev->monitor);
   free(ev);
}

static void
_eio_monitor_stat_cb(void *data, __UNUSED__ Eio_File *handler, __UNUSED__ const struct stat *st)
{
   Eio_Monitor *monitor = data;

   monitor->exist = NULL;
   monitor->refcount--;

   if (monitor->refcount > 0)
     {
        eio_monitor_backend_add(monitor);
     }
   else
     {
        eina_hash_del(_eio_monitors, monitor->path, monitor);
     }
}

static void
_eio_monitor_error_cb(void *data, Eio_File *handler, int error)
{
   Eio_Monitor_Error *ev;
   Eio_Monitor *monitor = data;

   monitor->error = error;
   monitor->exist = NULL;
   monitor->refcount--;

   if (monitor->refcount == 0) goto on_empty;

   ev = calloc(1, sizeof (Eio_Monitor_Error));
   if (!ev) return ;

   ev->monitor = monitor;
   ev->monitor->refcount++;
   ev->error = error;

   ecore_event_add(EIO_MONITOR_ERROR, ev, _eio_monitor_cleanup_cb, NULL);

 on_empty:
   eina_hash_del(_eio_monitors, monitor->path, monitor);
}

void
eio_monitor_init(void)
{
   EIO_MONITOR_ERROR = ecore_event_type_new();
   EIO_MONITOR_FILE_CREATED = ecore_event_type_new();
   EIO_MONITOR_FILE_DELETED = ecore_event_type_new();
   EIO_MONITOR_FILE_MODIFIED = ecore_event_type_new();
   EIO_MONITOR_FILE_START = ecore_event_type_new();
   EIO_MONITOR_FILE_STOP = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_CREATED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_DELETED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_MODIFIED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_START = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_STOP = ecore_event_type_new();

   eio_monitor_backend_init();
   eio_monitor_fallback_init();

   _eio_monitors = eina_hash_stringshared_new(_eio_monitor_del);

   _monitor_pid = getpid();
}

void
eio_monitor_shutdown(void)
{
   eina_hash_free(_eio_monitors);

   eio_monitor_backend_shutdown();
   eio_monitor_fallback_shutdown();

   _monitor_pid = -1;
}

EAPI Eio_Monitor *
eio_monitor_add(const char *path)
{
   const char *tmp;

   tmp = eina_stringshare_add(path);
   return eio_monitor_stringshared_add(path);
}

EAPI Eio_Monitor *
eio_monitor_stringshared_add(const char *path)
{
   Eio_Monitor *monitor;

   if (_monitor_pid == -1) return NULL;

   if (_monitor_pid != getpid())
     {
       eio_monitor_shutdown();
       eio_monitor_init();
     }

   monitor = eina_hash_find(_eio_monitors, path);

   if (!monitor)
     {
        monitor = malloc(sizeof (Eio_Monitor));
        if (!monitor) return NULL;

        monitor->backend = NULL; // This is needed to avoid race condition
        monitor->path = eina_stringshare_ref(path);
        monitor->fallback = EINA_FALSE;
        monitor->refcount = 1;

        monitor->exist = eio_file_direct_stat(monitor->path,
                                              _eio_monitor_stat_cb,
                                              _eio_monitor_error_cb,
                                              monitor);

        eina_hash_direct_add(_eio_monitors, path, monitor);
     }

   monitor->refcount++;

   return monitor;
}

EAPI void
eio_monitor_del(Eio_Monitor *monitor)
{
   monitor->refcount--;

   if (monitor->refcount > 0) return ;

   eina_hash_del(_eio_monitors, monitor->path, monitor);
}

EAPI const char *
eio_monitor_path_get(Eio_Monitor *monitor)
{
   return monitor->path;
}
