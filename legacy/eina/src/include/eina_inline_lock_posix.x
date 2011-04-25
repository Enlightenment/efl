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

#ifndef __USE_UNIX98
# define __USE_UNIX98
# include <pthread.h>
# undef __USE_UNIX98
#else
# include <pthread.h>
#endif

typedef pthread_mutex_t Eina_Lock;

EAPI extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
# include <sys/time.h>

EAPI extern int _eina_threads_debug;
#endif

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   pthread_mutexattr_t attr;

   if (pthread_mutexattr_init(&attr) != 0)
     return EINA_FALSE;
#ifdef PTHREAD_MUTEX_RECURSIVE
   if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
     return EINA_FALSE;
#endif
   if (pthread_mutex_init(mutex, &attr) != 0)
     return EINA_FALSE;

   pthread_mutexattr_destroy(&attr);

   return EINA_TRUE;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   pthread_mutex_destroy(mutex);
}

static inline Eina_Bool
eina_lock_take(Eina_Lock *mutex)
{
   if (_eina_threads_activated)
      {
#ifdef EINA_HAVE_DEBUG_THREADS
         if (_eina_threads_debug)
           {
              struct timeval t0, t1;
              int dt;

              gettimeofday(&t0, NULL);
              pthread_mutex_lock(&(x));
              gettimeofday(&t1, NULL);

              dt = (t1.tv_sec - t0.tv_sec) * 1000000;
              if (t1.tv_usec > t0.tv_usec)
                dt += (t1.tv_usec - t0.tv_usec);
              else
                dt -= t0.tv_usec - t1.tv_usec;
              dt /= 1000;

              if (dt > _eina_threads_debug) abort();
           }
#endif
         return (pthread_mutex_lock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
      }
   return EINA_FALSE;
}

static inline Eina_Bool
eina_lock_take_try(Eina_Lock *mutex)
{
   if (_eina_threads_activated)
     return (pthread_mutex_trylock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}

static inline Eina_Bool
eina_lock_release(Eina_Lock *mutex)
{
   if (_eina_threads_activated)
     return (pthread_mutex_unlock(mutex) == 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}

#endif
