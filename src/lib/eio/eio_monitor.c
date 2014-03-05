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

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static Eina_Hash *_eio_monitors = NULL;
static pid_t _monitor_pid = -1;

static void
_eio_monitor_free(Eio_Monitor *monitor)
{
   if (!monitor->delete_me)
     eina_hash_del(_eio_monitors, monitor->path, monitor);

   if (monitor->exist)
     {
        eio_file_cancel(monitor->exist);
        monitor->exist = NULL;
     }

   if (monitor->backend)
     {
        if (!monitor->fallback)
          eio_monitor_backend_del(monitor);
        else
          eio_monitor_fallback_del(monitor);
     }

   INF("Stopping monitor on '%s'.", monitor->path);

   eina_stringshare_del(monitor->path);
   free(monitor);
}

static void
_eio_monitor_error_cleanup_cb(EINA_UNUSED void *user_data, void *func_data)
{
   Eio_Monitor_Error *ev = func_data;

   EINA_REFCOUNT_UNREF(ev->monitor)
     _eio_monitor_free(ev->monitor);
   free(ev);
}

static void
_eio_monitor_event_cleanup_cb(EINA_UNUSED void *user_data, void *func_data)
{
   Eio_Monitor_Event *ev = func_data;

   EINA_REFCOUNT_UNREF(ev->monitor)
     _eio_monitor_free(ev->monitor);
   eina_stringshare_del(ev->filename);
   free(ev);
}

static void
_eio_monitor_stat_cb(void *data, EINA_UNUSED Eio_File *handler, EINA_UNUSED const Eina_Stat *st)
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
   if (!ev) return;

   ev->monitor = monitor;
   EINA_REFCOUNT_REF(ev->monitor);
   ev->error = error;

   ecore_event_add(EIO_MONITOR_ERROR, ev, _eio_monitor_error_cleanup_cb, NULL);
}

static void
_eio_monitor_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Eio_Monitor *monitor = data;

   monitor->error = error;
   monitor->exist = NULL;

   if (EINA_REFCOUNT_GET(monitor) >= 1)
     _eio_monitor_error(monitor, error);

   EINA_REFCOUNT_UNREF(monitor)
     _eio_monitor_free(monitor);

   return;
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
   /* FIXME: this check is optional, but if it is kept then failure should be handled more gracefully */
   if (!_eio_monitors) abort();

   _monitor_pid = getpid();
}

void
eio_monitor_shutdown(void)
{
   Eina_Iterator *it;
   Eio_Monitor *monitor;

   it = eina_hash_iterator_data_new(_eio_monitors);
   EINA_ITERATOR_FOREACH(it, monitor)
     {
        if (monitor->exist)
          {
             eio_file_cancel(monitor->exist);
             monitor->exist = NULL;
          }
        monitor->delete_me = EINA_TRUE;
     }
   eina_iterator_free(it);
   eina_hash_free(_eio_monitors);

   eio_monitor_backend_shutdown();
   eio_monitor_fallback_shutdown();

   _monitor_pid = -1;
}

static const char *
_eio_naming_event(int event_code)
{
#define EVENT_CHECK(Code, Ev) if (Code == Ev) return #Ev;

   EVENT_CHECK(event_code, EIO_MONITOR_ERROR);
   EVENT_CHECK(event_code, EIO_MONITOR_FILE_CREATED);
   EVENT_CHECK(event_code, EIO_MONITOR_FILE_DELETED);
   EVENT_CHECK(event_code, EIO_MONITOR_FILE_MODIFIED);
   EVENT_CHECK(event_code, EIO_MONITOR_FILE_CLOSED);
   EVENT_CHECK(event_code, EIO_MONITOR_DIRECTORY_CREATED);
   EVENT_CHECK(event_code, EIO_MONITOR_DIRECTORY_DELETED);
   EVENT_CHECK(event_code, EIO_MONITOR_DIRECTORY_MODIFIED);
   EVENT_CHECK(event_code, EIO_MONITOR_DIRECTORY_CLOSED);
   EVENT_CHECK(event_code, EIO_MONITOR_SELF_RENAME);
   EVENT_CHECK(event_code, EIO_MONITOR_SELF_DELETED);
   return "Unknown";
}

void
_eio_monitor_send(Eio_Monitor *monitor, const char *filename, int event_code)
{
   Eio_Monitor_Event *ev;

   if (monitor->delete_me)
     return;

   INF("Event '%s' for monitored path '%s'.",
       _eio_naming_event(event_code), filename);

   ev = calloc(1, sizeof (Eio_Monitor_Event));
   if (!ev) return;

   ev->monitor = monitor;
   EINA_REFCOUNT_REF(ev->monitor);
   ev->filename = eina_stringshare_add(filename);

   ecore_event_add(event_code, ev, _eio_monitor_event_cleanup_cb, NULL);
}

void
_eio_monitor_rename(Eio_Monitor *monitor, const char *newpath)
{
  const char *tmp;

  if (monitor->delete_me)
    return;

  /* destroy old state */
  if (monitor->exist)
    {
       eio_file_cancel(monitor->exist);
       monitor->exist = NULL;
    }

  if (monitor->backend)
    {
       if (!monitor->fallback)
         eio_monitor_backend_del(monitor);
       else
         eio_monitor_fallback_del(monitor);
    }

  INF("Renaming path '%s' to '%s'.",
      monitor->path, newpath);

  /* rename */
  tmp = monitor->path;
  monitor->path = eina_stringshare_add(newpath);
  eina_hash_move(_eio_monitors, tmp, monitor->path);
  eina_stringshare_del(tmp);

  /* That means death (cmp pointer and not content) */
  /* this - i think, is wrong. if the paths are the same, we should just
   * re-stat anyway. imagine the file was renamed and then replaced?
   * disable this as this was part of a possible crash due to eio.
  if (tmp == monitor->path)
    {
      _eio_monitor_error(monitor, -1);
      return;
    }
   */

  EINA_REFCOUNT_REF(monitor); /* as we spawn a thread for this monitor, we need to refcount specifically for it */

  /* restart */
  monitor->rename = EINA_TRUE;
  monitor->exist = eio_file_direct_stat(monitor->path,
                                        _eio_monitor_stat_cb,
                                        _eio_monitor_error_cb,
                                        monitor);

  /* FIXME: probably should handle this more gracefully */
  if (!monitor->exist) abort();
  /* and notify the app */
  _eio_monitor_send(monitor, newpath, EIO_MONITOR_SELF_RENAME);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

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

EAPI Eio_Monitor *
eio_monitor_add(const char *path)
{
   const char *tmp;
   Eio_Monitor *ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   tmp = eina_stringshare_add(path);
   ret = eio_monitor_stringshared_add(tmp);
   eina_stringshare_del(tmp);
   return ret;
}

EAPI Eio_Monitor *
eio_monitor_stringshared_add(const char *path)
{
   Eio_Monitor *monitor;
   struct stat st;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   if (_monitor_pid == -1) return NULL;

   if (_monitor_pid != getpid())
     {
       eio_monitor_shutdown();
       eio_monitor_init();
     }

   if (stat(path, &st) != 0)
     {
        ERR("monitored path not found");
        return NULL;
     }

   monitor = eina_hash_find(_eio_monitors, path);

   if (monitor)
     {
        if (st.st_mtime != monitor->mtime)
          {
             monitor->delete_me = EINA_TRUE;
             eina_hash_del(_eio_monitors, monitor->path, monitor);
          }
        else
          {
             EINA_REFCOUNT_REF(monitor);
             return monitor;
          }
     }

   monitor = malloc(sizeof (Eio_Monitor));
   if (!monitor) return NULL;

   monitor->mtime = st.st_mtime;
   monitor->backend = NULL; // This is needed to avoid race condition
   monitor->path = eina_stringshare_ref(path);
   monitor->fallback = EINA_FALSE;
   monitor->rename = EINA_FALSE;
   monitor->delete_me = EINA_FALSE;

   EINA_REFCOUNT_INIT(monitor);
   EINA_REFCOUNT_REF(monitor); /* as we spawn a thread for this monitor, we need to refcount specifically for it */

   monitor->exist = eio_file_direct_stat(monitor->path,
                                         _eio_monitor_stat_cb,
                                         _eio_monitor_error_cb,
                                         monitor);
   if (!monitor->exist)
     {
        _eio_monitor_free(monitor);
        return NULL;
     }

   eina_hash_direct_add(_eio_monitors, path, monitor);
   INF("New monitor on '%s'.", path);

   return monitor;
}

EAPI void
eio_monitor_del(Eio_Monitor *monitor)
{
   if (!monitor) return;
   EINA_REFCOUNT_UNREF(monitor)
     _eio_monitor_free(monitor);
}

EAPI const char *
eio_monitor_path_get(Eio_Monitor *monitor)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(monitor, NULL);
   return monitor->path;
}
