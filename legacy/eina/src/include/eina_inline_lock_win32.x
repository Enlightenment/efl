/* EINA - EFL data type library
 * Copyright (C) 2011 Vincent Torri
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

#ifndef EINA_INLINE_LOCK_WIN32_X_
#define EINA_INLINE_LOCK_WIN32_X_

#include <windows.h>

typedef CRITICAL_SECTION Eina_Lock;

EAPI extern Eina_Bool _threads_activated;

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   InitializeCriticalSection(m);

   return EINA_TRUE;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   DeleteCriticalSection(mutex);
}

static inline Eina_Bool
eina_lock_take(Eina_Lock *mutex)
{
   if (!_threads_activated) return EINA_FALSE;

   EnterCriticalSection(mutex);

   return EINA_TRUE;
}

static inline Eina_Bool
eina_lock_take_try(Eina_Lock *mutex)
{
   if (!_threads_activated) return EINA_FALSE;

   return TryEnterCriticalSection(mutex) == 0 ? EINA_FALSE : EINA_TRUE;
}

static inline Eina_Bool
eina_lock_release(Eina_Lock *mutex)
{
   if (!_threads_activated) return EINA_FALSE;

   LeaveCriticalSection(mutex);

   return EINA_TRUE;
}

#endif
