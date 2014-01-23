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

#ifdef _XOPEN_SOURCE
# define EINA_XOPEN_SOURCE _XOPEN_SOURCE
# undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 600

#ifdef EINA_HAVE_POSIX_SPINLOCK
# include <sched.h>
#endif

#include <errno.h>
#ifndef __USE_UNIX98
# define __USE_UNIX98
# include <pthread.h>
# undef __USE_UNIX98
#else
# include <pthread.h>
#endif

#include <semaphore.h>

#include <sys/time.h>
#include <stdio.h>

#ifdef EINA_HAVE_DEBUG_THREADS
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <execinfo.h>
#define EINA_LOCK_DEBUG_BT_NUM 64
typedef void (*Eina_Lock_Bt_Func) ();

#include "eina_inlist.h"
#endif

typedef struct _Eina_Lock Eina_Lock;
typedef struct _Eina_RWLock Eina_RWLock;
typedef struct _Eina_Condition Eina_Condition;
typedef pthread_key_t Eina_TLS;
typedef sem_t Eina_Semaphore;
#ifdef EINA_HAVE_POSIX_SPINLOCK
typedef pthread_spinlock_t Eina_Spinlock;
#else
typedef Eina_Lock Eina_Spinlock;
#endif

struct _Eina_Lock
{
#ifdef EINA_HAVE_DEBUG_THREADS
   EINA_INLIST;
#endif
   pthread_mutex_t   mutex;
#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_t         lock_thread_id;
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM];
   int               lock_bt_num;
   Eina_Bool         locked : 1;
#endif
};

struct _Eina_Condition
{
   Eina_Lock      *lock;
   pthread_cond_t  condition;
};

struct _Eina_RWLock
{
   pthread_rwlock_t mutex;
#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_t        lock_thread_wid;
#endif
};

EAPI extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
EAPI extern int _eina_threads_debug;
EAPI extern pthread_t _eina_main_loop;
EAPI extern pthread_mutex_t _eina_tracking_lock;
EAPI extern Eina_Inlist *_eina_tracking;
#endif

static inline void
eina_lock_debug(const Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   printf("lock %p, locked: %i, by %i\n",
          mutex, (int)mutex->locked, (int)mutex->lock_thread_id);
   backtrace_symbols_fd((void **)mutex->lock_bt, mutex->lock_bt_num, 1);
#else
   (void) mutex;
#endif
}

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   pthread_mutexattr_t attr;

#ifdef EINA_HAVE_DEBUG_THREADS
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   if (pthread_mutexattr_init(&attr) != 0)
     return EINA_FALSE;
   /* NOTE: PTHREAD_MUTEX_RECURSIVE is not allowed at all, you will break on/off
      feature for sure with that change. */
#ifdef EINA_HAVE_DEBUG_THREADS
   if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
     return EINA_FALSE;
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
   if (!_eina_threads_activated)
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
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   if (_eina_threads_debug)
     {
        struct timeval t0, t1;
        int dt;

        gettimeofday(&t0, NULL);
        ok = pthread_mutex_lock(&(mutex->mutex));
        gettimeofday(&t1, NULL);

        dt = (t1.tv_sec - t0.tv_sec) * 1000000;
        if (t1.tv_usec > t0.tv_usec)
           dt += (t1.tv_usec - t0.tv_usec);
        else
           dt -= t0.tv_usec - t1.tv_usec;
        dt /= 1000;

        if (dt > _eina_threads_debug) abort();
     }
   else
     {
#endif
        ok = pthread_mutex_lock(&(mutex->mutex));
#ifdef EINA_HAVE_DEBUG_THREADS
     }
#endif

   if (ok == 0) ret = EINA_LOCK_SUCCEED;
   else if (ok == EDEADLK)
     {
        printf("ERROR ERROR: DEADLOCK on lock %p\n", mutex);
        eina_lock_debug(mutex);
        ret = EINA_LOCK_DEADLOCK; // magic
#ifdef EINA_HAVE_DEBUG_THREADS
        if (_eina_threads_debug) abort();
#endif
     }

#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->locked = 1;
   mutex->lock_thread_id = pthread_self();
   mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);

   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking,
                                       EINA_INLIST_GET(mutex));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   return ret;
}

static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   ok = pthread_mutex_trylock(&(mutex->mutex));
   if (ok == 0) ret = EINA_LOCK_SUCCEED;
   else if (ok == EDEADLK)
     {
        printf("ERROR ERROR: DEADLOCK on trylock %p\n", mutex);
        ret = EINA_LOCK_DEADLOCK; // magic
     }
#ifdef EINA_HAVE_DEBUG_THREADS
   if (ret == EINA_LOCK_SUCCEED)
     {
        mutex->locked = 1;
        mutex->lock_thread_id = pthread_self();
        mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);

        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_append(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        pthread_mutex_unlock(&_eina_tracking_lock);
     }
#endif
   return ret;
}

static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
   Eina_Lock_Result ret;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_remove(_eina_tracking,
                                       EINA_INLIST_GET(mutex));
   pthread_mutex_unlock(&_eina_tracking_lock);

   mutex->locked = 0;
   mutex->lock_thread_id = 0;
   memset(mutex->lock_bt, 0, EINA_LOCK_DEBUG_BT_NUM * sizeof(Eina_Lock_Bt_Func));
   mutex->lock_bt_num = 0;
#endif
   ret = (pthread_mutex_unlock(&(mutex->mutex)) == 0) ?
      EINA_LOCK_SUCCEED : EINA_LOCK_FAIL;
   return ret;
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(mutex != NULL);
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
   memset(cond, 0, sizeof (Eina_Condition));
#endif

   cond->lock = mutex;
   if (pthread_cond_init(&cond->condition, NULL) != 0)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        if (errno == EBUSY)
          printf("eina_condition_new on already initialized Eina_Condition\n");
#endif
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static inline void
eina_condition_free(Eina_Condition *cond)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   pthread_cond_destroy(&(cond->condition));
#ifdef EINA_HAVE_DEBUG_THREADS
   memset(cond, 0, sizeof (Eina_Condition));
#endif
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   Eina_Bool r;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(_eina_threads_activated);
   assert(cond->lock != NULL);

   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_remove(_eina_tracking,
				       EINA_INLIST_GET(cond->lock));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   r = pthread_cond_wait(&(cond->condition),
			 &(cond->lock->mutex)) == 0 ? EINA_TRUE : EINA_FALSE;

#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking,
				       EINA_INLIST_GET(cond->lock));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   return r;
}

static inline Eina_Bool
eina_condition_timedwait(Eina_Condition *cond, double t)
{
   struct timespec tv;
   Eina_Bool r;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(_eina_threads_activated);
   assert(cond->lock != NULL);

   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_remove(_eina_tracking,
				       EINA_INLIST_GET(cond->lock));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   tv.tv_sec = t;
   tv.tv_nsec = (t - (double) tv.tv_sec) * 1000000000;

   r = pthread_cond_timedwait(&(cond->condition),
			      &(cond->lock->mutex),
			      &tv) == 0 ?
     EINA_TRUE : EINA_FALSE;

#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking,
				       EINA_INLIST_GET(cond->lock));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   return r;
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   return pthread_cond_broadcast(&(cond->condition)) == 0 ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   return pthread_cond_signal(&(cond->condition)) == 0 ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   if (pthread_rwlock_init(&(mutex->mutex), NULL) != 0)
     return EINA_FALSE;
   return EINA_TRUE;
}

static inline void
eina_rwlock_free(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (!_eina_threads_activated)
     assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif

   pthread_rwlock_destroy(&(mutex->mutex));
}

static inline Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

   if (pthread_rwlock_rdlock(&(mutex->mutex)) != 0)
     return EINA_LOCK_FAIL;
   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

   if (pthread_rwlock_wrlock(&(mutex->mutex)) != 0)
     return EINA_LOCK_FAIL;
   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(pthread_equal(_eina_main_loop, pthread_self()));
#endif
        return EINA_LOCK_SUCCEED;
     }
#endif

   if (pthread_rwlock_unlock(&(mutex->mutex)) != 0)
     return EINA_LOCK_FAIL;
   return EINA_LOCK_SUCCEED;
}

static inline Eina_Bool
eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   if (pthread_key_create(key, delete_cb) != 0)
      return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
eina_tls_new(Eina_TLS *key)
{
   return eina_tls_cb_new(key, NULL);
}

static inline void 
eina_tls_free(Eina_TLS key)
{
   pthread_key_delete(key);
}

static inline void * 
eina_tls_get(Eina_TLS key)
{
   return pthread_getspecific(key);
}

static inline Eina_Bool 
eina_tls_set(Eina_TLS key, const void *data)
{
   if (pthread_setspecific(key, data) != 0)
      return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
eina_semaphore_new(Eina_Semaphore *sem, int count_init)
{
   if (!sem || (count_init <= 0))
     return EINA_FALSE;

   return (sem_init(sem, count_init, 1) == 0) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_free(Eina_Semaphore *sem)
{
   if (!sem)
     return EINA_FALSE;

   return (sem_destroy(sem) == 0) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_lock(Eina_Semaphore *sem)
{
   if (!sem)
     return EINA_FALSE;

   return (sem_wait(sem) == 0) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_release(Eina_Semaphore *sem, int count_release EINA_UNUSED)
{
   if (!sem)
     return EINA_FALSE;

   return (sem_post(sem) == 0) ? EINA_TRUE : EINA_FALSE;
}

#ifdef EINA_HAVE_PTHREAD_BARRIER
typedef struct _Eina_Barrier Eina_Barrier;

struct _Eina_Barrier
{
   pthread_barrier_t barrier;
};

static inline Eina_Bool
eina_barrier_new(Eina_Barrier *barrier, int needed)
{
   if (!pthread_barrier_init(&(barrier->barrier), NULL, needed))
     return EINA_TRUE;
   return EINA_FALSE;
}

static inline void
eina_barrier_free(Eina_Barrier *barrier)
{
   pthread_barrier_destroy(&(barrier->barrier));
}

static inline Eina_Bool
eina_barrier_wait(Eina_Barrier *barrier)
{
   pthread_barrier_wait(&(barrier->barrier));
   return EINA_TRUE;
}

#else
#include "eina_inline_lock_barrier.x"
#endif

static inline Eina_Bool
eina_spinlock_new(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_POSIX_SPINLOCK
   return pthread_spin_init(spinlock, PTHREAD_PROCESS_PRIVATE) == 0 ? EINA_TRUE : EINA_FALSE;
#else
   return eina_lock_new(spinlock);
#endif
}

static inline Eina_Lock_Result
eina_spinlock_take(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_POSIX_SPINLOCK
   int t;

   do {
      t = pthread_spin_trylock(spinlock);
      if (t != 0)
        {
           if (errno == EBUSY) sched_yield();
           else if (errno == EDEADLK) return EINA_LOCK_DEADLOCK;
        }
   } while (t != 0);

   return EINA_LOCK_SUCCEED;
#else
   return eina_lock_take(spinlock);
#endif
}

static inline Eina_Lock_Result
eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_POSIX_SPINLOCK
   int t;

   t = pthread_spin_trylock(spinlock);
   return t ? EINA_LOCK_FAIL : EINA_LOCK_SUCCEED;
#else
   return eina_lock_take_try(spinlock);
#endif
}

static inline Eina_Lock_Result
eina_spinlock_release(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_POSIX_SPINLOCK
   return pthread_spin_unlock(spinlock) ? EINA_LOCK_FAIL : EINA_LOCK_SUCCEED;
#else
   return eina_lock_release(spinlock);
#endif
}

static inline void
eina_spinlock_free(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_POSIX_SPINLOCK
   pthread_spin_destroy(spinlock);
#else
   eina_lock_free(spinlock);
#endif
}

#undef _XOPEN_SOURCE
// This is necessary to let third party still define this macro
#ifdef EINA_XOPEN_SOURCE
# define _XOPEN_SOURCE EINA_XOPEN_SOURCE
#endif

#endif
