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

#ifndef EINA_INLINE_LOCK_POSIX_X_
#define EINA_INLINE_LOCK_POSIX_X_

#include <pthread.h>

typedef pthread_mutex_t Eina_Lock;

EAPI extern Eina_Bool _threads_activated;

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   return (pthread_mutex_init(mutex, NULL) == 0) ? EINA_TRUE : EINA_FALSE;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   pthread_mutex_destroy(mutex);
}

static inline Eina_Bool
eina_lock_take(Eina_Lock *mutex)
{
   if (_threads_activated)
     return (pthread_mutex_lock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_lock_take_try(Eina_Lock *mutex)
{
   if (_threads_activated)
     return (pthread_mutex_trylock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_lock_release(Eina_Lock *mutex)
{
   if (_threads_activated)
     return (pthread_mutex_unlock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}

#endif
