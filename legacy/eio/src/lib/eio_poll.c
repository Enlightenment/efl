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

struct _Eio_Monitor_Backend
{
   Eio_Monitor *parent;

   Ecore_Timer *timer;
   Ecore_Idler *idler;
   Ecore_Thread *work;

   Eina_Bool delete_me : 1;
};

static Eina_Bool _eio_monitor_fallback_timer_cb(void *data);

#if !defined HAVE_INOTIFY && !defined HAVE_NOTIFY_WIN32
void eio_monitor_backend_init(void)
{
}

void eio_monitor_backend_shutdown(void)
{
}

void eio_monitor_backend_add(Eio_Monitor *monitor)
{
  eio_monitor_fallback_add(monitor);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
  eio_monitor_fallback_del(monitor);
}
#endif

static void
_eio_monitor_fallback_heavy_cb(void *data, Ecore_Thread *thread)
{
  /* FIXME : copy ecore_file_monitor_poll here */
}

static void
_eio_monitor_fallback_end_cb(void *data, Ecore_Thread *thread)
{
   Eio_Monitor_Backend *backend = data;

   backend->work = NULL;
   backend->timer = ecore_timer_add(60.0, _eio_monitor_fallback_timer_cb, backend);
}

static void
_eio_monitor_fallback_cancel_cb(void *data, Ecore_Thread *thread)
{
   Eio_Monitor_Backend *backend = data;

   backend->work = NULL;
   if (backend->delete_me)
     {
        free(backend);
        return ;
     }
   backend->timer = ecore_timer_add(60.0, _eio_monitor_fallback_timer_cb, backend);
}

static Eina_Bool
_eio_monitor_fallback_idler_cb(void *data)
{
   Eio_Monitor_Backend *backend = data;

   backend->idler = NULL;
   backend->work = ecore_thread_run(_eio_monitor_fallback_heavy_cb,
                                    _eio_monitor_fallback_end_cb,
                                    _eio_monitor_fallback_cancel_cb,
                                    backend);
   return EINA_FALSE;
}

static Eina_Bool
_eio_monitor_fallback_timer_cb(void *data)
{
   Eio_Monitor_Backend *backend = data;

   backend->timer = NULL;
   backend->idler = ecore_idler_add(_eio_monitor_fallback_idler_cb, backend);

   return EINA_FALSE;
}

void
eio_monitor_fallback_init(void)
{
}

void
eio_monitor_fallback_shutdown(void)
{
}

void
eio_monitor_fallback_add(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   monitor->backend = NULL;

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend) return ;

   backend->parent = monitor;
   monitor->backend = backend;
   backend->timer = ecore_timer_add(60.0, _eio_monitor_fallback_timer_cb, backend);
}

void
eio_monitor_fallback_del(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   backend = monitor->backend;
   monitor->backend = NULL;

   if (!backend) return ;

   backend->parent = NULL;
   if (backend->timer) ecore_timer_del(backend->timer);
   backend->timer = NULL;
   if (backend->idler) ecore_idler_del(backend->idler);
   backend->idler = NULL;
   if (backend->work)
     {
        backend->delete_me = EINA_TRUE;
        ecore_thread_cancel(backend->work);
        return ;
     }
   free(backend);
}
