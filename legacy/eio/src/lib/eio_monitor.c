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
EAPI int EIO_MONITOR_FILE_CLOSED;
EAPI int EIO_MONITOR_DIRECTORY_CREATED;
EAPI int EIO_MONITOR_DIRECTORY_DELETED;
EAPI int EIO_MONITOR_DIRECTORY_MODIFIED;
EAPI int EIO_MONITOR_DIRECTORY_CLOSED;
EAPI int EIO_MONITOR_SELF_RENAME;
EAPI int EIO_MONITOR_SELF_DELETED;

static Eina_Hash *_eio_monitors = NULL;
static pid_t _monitor_pid = -1;

static void
_eio_monitor_free(Eio_Monitor *monitor)
{
   eina_hash_del(_eio_monitors, monitor->path, monitor);

   if (monitor->exist) eio_file_cancel(monitor->exist);

   if (monitor->backend)
     {
        if (!monitor->fallback)
          eio_monitor_backend_del(monitor);
        else
          eio_monitor_fallback_del(monitor);
     }

   eina_stringshare_del(monitor->path);
   free(monitor);
}

static void
_eio_monitor_error_cleanup_cb(void *user_data, __UNUSED__ void *func_data)
{
   Eio_Monitor_Error *ev = user_data;

   EINA_REFCOUNT_UNREF(ev->monitor)
     _eio_monitor_free(ev->monitor);
   free(ev);
}

static void
_eio_monitor_event_cleanup_cb(void *user_data, __UNUSED__ void *func_data)
{
   Eio_Monitor_Event *ev = user_data;

   EINA_REFCOUNT_UNREF(ev->monitor)
     _eio_monitor_free(ev->monitor);
   eina_stringshare_del(ev->filename);
   free(ev);
}

static void
_eio_monitor_stat_cb(void *data, __UNUSED__ Eio_File *handler, __UNUSED__ const struct stat *st)
{
   Eio_Monitor *monitor = data;

   monitor->exist = NULL;

   if (EINA_REFCOUNT_GET(monitor) > 1)
     eio_monitor_backend_add(monitor);

   EINA_REFCOUNT_UNREF(monitor)
     _eio_monitor_free(monitor);
}

static void
_eio_monitor_error(Eio_Monitor *monitor, int error)
{
   Eio_Monitor_Error *ev;

   ev = calloc(1, sizeof (Eio_Monitor_Error));
   if (!ev) return ;

   ev->monitor = monitor;
   EINA_REFCOUNT_REF(ev->monitor);
   ev->error = error;

   ecore_event_add(EIO_MONITOR_ERROR, ev, _eio_monitor_error_cleanup_cb, NULL);
}

static void
_eio_monitor_error_cb(void *data, Eio_File *handler, int error)
{
   Eio_Monitor *monitor = data;

   monitor->error = error;
   monitor->exist = NULL;

   if (EINA_REFCOUNT_GET(monitor) > 1)
     _eio_monitor_error(monitor, error);

   EINA_REFCOUNT_UNREF(monitor)
     _eio_monitor_free(monitor);

   return ;
}

void
eio_monitor_init(void)
{
   EIO_MONITOR_ERROR = ecore_event_type_new();
   EIO_MONITOR_SELF_RENAME = ecore_event_type_new();
   EIO_MONITOR_SELF_DELETED = ecore_event_type_new();
   EIO_MONITOR_FILE_CREATED = ecore_event_type_new();
   EIO_MONITOR_FILE_DELETED = ecore_event_type_new();
   EIO_MONITOR_FILE_MODIFIED = ecore_event_type_new();
   EIO_MONITOR_FILE_CLOSED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_CREATED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_DELETED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_MODIFIED = ecore_event_type_new();
   EIO_MONITOR_DIRECTORY_CLOSED = ecore_event_type_new();

   eio_monitor_backend_init();
   eio_monitor_fallback_init();

   _eio_monitors = eina_hash_stringshared_new(NULL);

   _monitor_pid = getpid();
}

void
eio_monitor_shutdown(void)
{
   /* FIXME: Need to cancel all request... */
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

   if (monitor)
     {
        EINA_REFCOUNT_REF(monitor);
        return monitor;
     }

   monitor = malloc(sizeof (Eio_Monitor));
   if (!monitor) return NULL;

   monitor->backend = NULL; // This is needed to avoid race condition
   monitor->path = eina_stringshare_ref(path);
   monitor->fallback = EINA_FALSE;
   monitor->rename = EINA_FALSE;

   EINA_REFCOUNT_INIT(monitor);

   monitor->exist = eio_file_direct_stat(monitor->path,
                                         _eio_monitor_stat_cb,
                                         _eio_monitor_error_cb,
                                         monitor);

   eina_hash_direct_add(_eio_monitors, path, monitor);

   return monitor;
}

EAPI void
eio_monitor_del(Eio_Monitor *monitor)
{
   EINA_REFCOUNT_UNREF(monitor)
     _eio_monitor_free(monitor);
}

EAPI const char *
eio_monitor_path_get(Eio_Monitor *monitor)
{
   return monitor->path;
}

void
_eio_monitor_send(Eio_Monitor *monitor, const char *filename, int event_code)
{
   Eio_Monitor_Event *ev;

   ev = calloc(1, sizeof (Eio_Monitor_Event));
   if (!ev) return ;

   ev->monitor = monitor;
   EINA_REFCOUNT_REF(ev->monitor);
   ev->filename = eina_stringshare_add(filename);

   ecore_event_add(event_code, ev, _eio_monitor_event_cleanup_cb, NULL);
}

void
_eio_monitor_rename(Eio_Monitor *monitor, const char *newpath)
{
  const char *tmp;

  /* destroy old state */
  if (monitor->exist) eio_file_cancel(monitor->exist);

  if (monitor->backend)
    {
       if (!monitor->fallback)
         eio_monitor_backend_del(monitor);
       else
         eio_monitor_fallback_del(monitor);
    }

  /* rename */
  tmp = monitor->path;
  monitor->path = eina_stringshare_add(newpath);
  eina_hash_move(_eio_monitors, tmp, monitor->path);
  eina_stringshare_del(tmp);

  /* That means death (cmp pointer and not content) */
  if (tmp == monitor->path)
    {
      _eio_monitor_error(monitor, -1);
      return ;
    }

  /* restart */
  monitor->rename = EINA_TRUE;
  monitor->exist = eio_file_direct_stat(monitor->path,
                                        _eio_monitor_stat_cb,
                                        _eio_monitor_error_cb,
                                        monitor);

  /* and notify the app */
  _eio_monitor_send(monitor, newpath, EIO_MONITOR_SELF_RENAME);
}
