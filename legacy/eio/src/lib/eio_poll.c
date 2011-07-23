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
};

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

void
eio_monitor_fallback_init(void)
{
}

void
eio_monitor_fallback_shutdown(void)
{
}

void
eio_monitor_fallback_add(Eio_Monitor *monitor __UNUSED__)
{
}

void
eio_monitor_fallback_del(Eio_Monitor *monitor __UNUSED__)
{
}
