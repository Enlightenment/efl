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

#include <errno.h>
#ifndef __USE_UNIX98
# define __USE_UNIX98
# include <pthread.h>
# undef __USE_UNIX98
#else
# include <pthread.h>
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
#include <execinfo.h>
#define EINA_LOCK_DEBUG_BT_NUM 64
typedef void (*Eina_Lock_Bt_Func) ();
#endif

typedef struct _Eina_Lock Eina_Lock;

struct _Eina_Lock
{
   pthread_mutex_t  mutex;
#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_t         lock_thread_id;
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM];
   int               lock_bt_num;
   Eina_Bool         locked : 1;
#endif
};

EAPI extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
# include <sys/time.h>

EAPI extern int _eina_threads_debug;
EAPI extern pthread_t _eina_main_loop;
#endif

static inline Eina_Lock_Result
eina_lock_new(Eina_Lock *mutex)
{
   pthread_mutexattr_t attr;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   if (pthread_mutexattr_init(&attr) != 0)
     return EINA_FALSE;
/* use errorcheck locks. detect deadlocks.
#ifdef PTHREAD_MUTEX_RECURSIVE
   if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
     return EINA_FALSE;
#endif
 */
   if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
     return EINA_FALSE;
#ifdef EINA_HAVE_DEBUG_THREADS
   memset(mutex, 0, sizeof(Eina_Lock));
#endif
   if (pthread_mutex_init(&(mutex->mutex), &attr) != 0)
     return EINA_FALSE;

   pthread_mutexattr_destroy(&attr);

   return EINA_TRUE;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   pthread_mutex_destroy(&(mutex->mutex));
#ifdef EINA_HAVE_DEBUG_THREADS
   memset(mutex, 0, sizeof(Eina_Lock));
#endif
}

static inline Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex)
{
   Eina_Bool ret;
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   if (_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));

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
   ok = pthread_mutex_lock(&(mutex->mutex));
   if (ok == 0) ret = EINA_TRUE;
   else if (ok == EDEADLK)
     {
        printf("ERROR ERROR: DEADLOCK on lock %p\n", mutex);
        ret = EINA_LOCK_DEADLOCK; // magic
     }
#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->locked = 1;
   mutex->lock_thread_id = pthread_self();
   mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
#endif
   return ret;
}

static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
   Eina_Bool ret = EINA_FALSE;
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   if (_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   ok = pthread_mutex_trylock(&(mutex->mutex));
   if (ok == 0) ret = EINA_TRUE;
   else if (ok == EDEADLK)
     {
        printf("ERROR ERROR: DEADLOCK on trylock %p\n", mutex);
        ret = EINA_LOCK_DEADLOCK; // magic
     }
#ifdef EINA_HAVE_DEBUG_THREADS
   if (ret == EINA_TRUE)
     {
        mutex->locked = 1;
        mutex->lock_thread_id = pthread_self();
        mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
     }
#endif
   return ret;
}

static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
   Eina_Bool ret;

#ifdef EINA_HAVE_DEBUG_THREADS
   if (_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->locked = 0;
   mutex->lock_thread_id = 0;
   memset(mutex->lock_bt, 0, EINA_LOCK_DEBUG_BT_NUM * sizeof(Eina_Lock_Bt_Func));
   mutex->lock_bt_num = 0;
#endif
   ret = (pthread_mutex_unlock(&(mutex->mutex)) == 0) ?
      EINA_TRUE : EINA_FALSE;
   return ret;
}

static inline void
eina_lock_debug(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   printf("lock %p, locked: %i, by %i\n",
          mutex, (int)mutex->locked, (int)mutex->lock_thread_id);
   backtrace_symbols_fd((void **)mutex->lock_bt, mutex->lock_bt_num, 1);
#else
   mutex = 0;
#endif
}

#endif
