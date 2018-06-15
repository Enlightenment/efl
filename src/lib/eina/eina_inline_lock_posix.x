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

#ifdef EINA_HAVE_OSX_SEMAPHORE
# include <mach/mach.h>
#endif

#include <semaphore.h>

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#ifdef EINA_HAVE_DEBUG_THREADS
#include <assert.h>
#include <execinfo.h>
#define EINA_LOCK_DEBUG_BT_NUM 64
typedef void (*Eina_Lock_Bt_Func) ();

#include "eina_inlist.h"
#endif

EAPI void _eina_lock_debug_abort(int err, const char *fn, const volatile void *ptr);
EAPI void _eina_lock_debug_deadlock(const char *fn, const volatile void *ptr);

#define EINA_LOCK_ABORT_DEBUG(err, fn, ptr) \
   _eina_lock_debug_abort(err, #fn, ptr)
#define EINA_LOCK_DEADLOCK_DEBUG(fn, ptr) \
   _eina_lock_debug_deadlock(#fn, ptr)

/* For cond_timedwait */
#include <time.h>
#include <sys/time.h>

#include <eina_error.h>

typedef struct _Eina_Lock Eina_Lock;
typedef struct _Eina_RWLock Eina_RWLock;
typedef struct _Eina_Condition Eina_Condition;
typedef pthread_key_t Eina_TLS;

#if defined(EINA_HAVE_POSIX_SPINLOCK)
typedef pthread_spinlock_t Eina_Spinlock;
#elif defined(EINA_HAVE_OSX_SPINLOCK)
typedef uintptr_t Eina_Spinlock;
#else
typedef Eina_Lock Eina_Spinlock;
#endif

#if defined(EINA_HAVE_OSX_SEMAPHORE)
typedef semaphore_t Eina_Semaphore;
#else
typedef sem_t Eina_Semaphore;
#endif

EAPI void eina_lock_debug(const Eina_Lock *mutex);

/** @privatesection  @{ */
struct _Eina_Lock
{
#ifdef EINA_HAVE_DEBUG_THREADS
   EINA_INLIST; /**< Keeps track of the threads waiting for the lock */
#endif
   pthread_mutex_t   mutex; /**< The mutex that handles the locking */
#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_t         lock_thread_id; /**< The ID of the thread that currently has the lock */
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM]; /**< The function that will produce a backtrace on the thread that has the lock */
   int               lock_bt_num; /**< Number of addresses in the backtrace */
   Eina_Bool         locked : 1;  /**< Indicates locked or not locked */
   Eina_Bool         recursive : 1;  /**< Indicates recursive lock */
#endif
};

struct _Eina_Condition
{
   Eina_Lock      *lock;      /**< The lock for this condition */
   pthread_cond_t  condition; /**< The condition variable */
#if defined(__clockid_t_defined)
   clockid_t       clkid;     /**< The attached clock for timedwait */
#endif
};

struct _Eina_RWLock
{
   pthread_rwlock_t mutex; /**< The mutex that handles the locking */
#ifdef EINA_HAVE_DEBUG_THREADS
   pthread_t        lock_thread_wid; /**< The ID of the thread that currently has the lock */
#endif
};
/** @} privatesection */

EAPI extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
EAPI extern int _eina_threads_debug;
EAPI extern pthread_t _eina_main_loop;
EAPI extern pthread_mutex_t _eina_tracking_lock;
EAPI extern Eina_Inlist *_eina_tracking;
#endif


EAPI Eina_Bool _eina_lock_new(Eina_Lock *mutex, Eina_Bool recursive);
EAPI void      _eina_lock_free(Eina_Lock *mutex);
EAPI Eina_Bool _eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);
EAPI void      _eina_condition_free(Eina_Condition *cond);
EAPI Eina_Bool _eina_rwlock_new(Eina_RWLock *mutex);
EAPI void      _eina_rwlock_free(Eina_RWLock *mutex);
EAPI Eina_Bool _eina_spinlock_new(Eina_Spinlock *spinlock);
EAPI void      _eina_spinlock_free(Eina_Spinlock *spinlock);
EAPI Eina_Bool _eina_semaphore_new(Eina_Semaphore *sem, int count_init);
EAPI Eina_Bool _eina_semaphore_free(Eina_Semaphore *sem);
#ifdef EINA_HAVE_OSX_SPINLOCK
EAPI Eina_Lock_Result _eina_spinlock_macos_take(Eina_Spinlock *spinlock);
EAPI Eina_Lock_Result _eina_spinlock_macos_take_try(Eina_Spinlock *spinlock);
EAPI Eina_Lock_Result _eina_spinlock_macos_release(Eina_Spinlock *spinlock);
#endif

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   Eina_Bool ret = _eina_lock_new(mutex, EINA_FALSE);
#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->recursive = EINA_FALSE;
#endif
   return ret;
}

static inline Eina_Bool
eina_lock_recursive_new(Eina_Lock *mutex)
{
   Eina_Bool ret = _eina_lock_new(mutex, EINA_TRUE);
#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->recursive = EINA_TRUE;
#endif
   return ret;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (mutex->locked)
     {
        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_remove(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        pthread_mutex_unlock(&_eina_tracking_lock);
     }
#endif
   _eina_lock_free(mutex);
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
        eina_lock_debug(mutex);
        ret = EINA_LOCK_DEADLOCK;
#ifdef EINA_HAVE_DEBUG_THREADS
        if (_eina_threads_debug) abort();
#endif
     }
   else EINA_LOCK_ABORT_DEBUG(ok, lock, mutex);

#ifdef EINA_HAVE_DEBUG_THREADS
   /* recursive locks can't make use of any of this */
   if (mutex->recursive) return ret;
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
        eina_lock_debug(mutex);
        ret = EINA_LOCK_DEADLOCK;
     }
   else if (ok != EBUSY) EINA_LOCK_ABORT_DEBUG(ok, trylock, mutex);
#ifdef EINA_HAVE_DEBUG_THREADS
   if (ret == EINA_LOCK_SUCCEED)
     {
        /* recursive locks can't make use of any of this */
        if (mutex->recursive) return ret;
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
/* recursive locks can't make use of any of this */
   if (!mutex->recursive)
     {
        mutex->locked = 0;
        mutex->lock_thread_id = 0;
        memset(mutex->lock_bt, 0, EINA_LOCK_DEBUG_BT_NUM * sizeof(Eina_Lock_Bt_Func));
        mutex->lock_bt_num = 0;
        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_remove(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        pthread_mutex_unlock(&_eina_tracking_lock);
     }
#endif
   ok = pthread_mutex_unlock(&(mutex->mutex));
   if (ok == 0) ret = EINA_LOCK_SUCCEED;
   else if (ok == EPERM) ret = EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG(ok, unlock, mutex);
   return ret;
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   return _eina_condition_new(cond, mutex);
}

static inline void
eina_condition_free(Eina_Condition *cond)
{
   _eina_condition_free(cond);
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   Eina_Bool r = EINA_FALSE;
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(_eina_threads_activated);
   assert(cond->lock != NULL);

   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_remove(_eina_tracking,
                                       EINA_INLIST_GET(cond->lock));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   ok = pthread_cond_wait(&(cond->condition), &(cond->lock->mutex));
   if (ok == 0) r = EINA_TRUE;
   else if (ok == EPERM) r = EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG(ok, cond_wait, cond);

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
   struct timespec ts;
   time_t sec;
   long nsec;
   int err;
   Eina_Bool r = EINA_FALSE;

   if (t >= 0.0)
     {
#if defined(__clockid_t_defined)
        if (cond->clkid)
          {
             if (clock_gettime(cond->clkid, &ts) != 0) return EINA_FALSE;
          }
        else
#endif
          {
             /* Obsolete for Linux.
              * TODO: use pthread_cond_timedwait_relative_np for OSX. */
             struct timeval tv;
             if (gettimeofday(&tv, NULL) != 0) return EINA_FALSE;
             ts.tv_sec = tv.tv_sec;
             ts.tv_nsec = tv.tv_usec * 1000L;
          }

#ifdef EINA_HAVE_DEBUG_THREADS
        assert(_eina_threads_activated);
        assert(cond->lock != NULL);

        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_remove(_eina_tracking,
                                            EINA_INLIST_GET(cond->lock));
        pthread_mutex_unlock(&_eina_tracking_lock);
#endif

        sec = (time_t)t;
        nsec = (t - (double)sec) * 1000000000L;
        ts.tv_sec += sec;
        ts.tv_nsec += nsec;
        if (ts.tv_nsec > 1000000000L)
          {
             ts.tv_sec++;
             ts.tv_nsec -= 1000000000L;
          }

        err = pthread_cond_timedwait(&(cond->condition),
                                     &(cond->lock->mutex),
                                     &ts);
        if (err == 0) r = EINA_TRUE;
        else if (err == EPERM) eina_error_set(EPERM);
        else if (err == ETIMEDOUT) eina_error_set(ETIMEDOUT);
        else EINA_LOCK_ABORT_DEBUG(err, cond_timedwait, cond);

#ifdef EINA_HAVE_DEBUG_THREADS
        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_append(_eina_tracking,
                                            EINA_INLIST_GET(cond->lock));
        pthread_mutex_unlock(&_eina_tracking_lock);
#endif
     }
   errno = EINVAL;
   return r;
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   ok = pthread_cond_broadcast(&(cond->condition));
   if (ok == 0) return EINA_TRUE;

   EINA_LOCK_ABORT_DEBUG(ok, cond_broadcast, cond);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   ok = pthread_cond_signal(&(cond->condition));
   if (ok == 0) return EINA_TRUE;

   EINA_LOCK_ABORT_DEBUG(ok, cond_signal, cond);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex)
{
   return _eina_rwlock_new(mutex);
}

static inline void
eina_rwlock_free(Eina_RWLock *mutex)
{
   _eina_rwlock_free(mutex);
}

static inline Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex)
{
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

   ok = pthread_rwlock_rdlock(&(mutex->mutex));
   if (ok == 0) return EINA_LOCK_SUCCEED;
   else if (ok == EAGAIN || ok == ENOMEM) return EINA_LOCK_FAIL;
   else if (ok == EDEADLK) EINA_LOCK_DEADLOCK_DEBUG(rwlock_rdlock, mutex);
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_rdlock, mutex);
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex)
{
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

   ok = pthread_rwlock_wrlock(&(mutex->mutex));
   if (ok == 0) return EINA_LOCK_SUCCEED;
   else if (ok == ENOMEM) return EINA_LOCK_FAIL;
   else if (ok == EDEADLK) EINA_LOCK_DEADLOCK_DEBUG(rwlock_wrlock, mutex);
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_wrlock, mutex);
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex)
{
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

   ok = pthread_rwlock_unlock(&(mutex->mutex));
   if (ok == 0) return EINA_LOCK_SUCCEED;
   else if (ok == EPERM) return EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_unlock, mutex);
   return EINA_LOCK_FAIL;
}

static inline Eina_Bool
eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   if (pthread_key_create(key, delete_cb) == 0) return EINA_TRUE;
   return EINA_FALSE;
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
   if (pthread_setspecific(key, data) == 0) return EINA_TRUE;
   return EINA_FALSE;
}


#ifdef EINA_HAVE_PTHREAD_BARRIER
typedef struct _Eina_Barrier Eina_Barrier;

struct _Eina_Barrier
{
   pthread_barrier_t barrier;
};

static inline Eina_Bool
eina_barrier_wait(Eina_Barrier *barrier)
{
   int ok = pthread_barrier_wait(&(barrier->barrier));
   if (ok == 0) return EINA_TRUE;
   else if (ok == PTHREAD_BARRIER_SERIAL_THREAD) return EINA_TRUE;
   else EINA_LOCK_ABORT_DEBUG(ok, barrier_wait, barrier);
   return EINA_TRUE;
}

#else
#include "eina_inline_lock_barrier.x"
#endif

EAPI Eina_Bool _eina_barrier_new(Eina_Barrier *barrier, int needed);
EAPI void      _eina_barrier_free(Eina_Barrier *barrier);

static inline Eina_Bool
eina_barrier_new(Eina_Barrier *barrier, int needed)
{
   return _eina_barrier_new(barrier, needed);
}

static inline void
eina_barrier_free(Eina_Barrier *barrier)
{
   _eina_barrier_free(barrier);
}


static inline Eina_Bool
eina_spinlock_new(Eina_Spinlock *spinlock)
{
   return _eina_spinlock_new(spinlock);
}

static inline void
eina_spinlock_free(Eina_Spinlock *spinlock)
{
   _eina_spinlock_free(spinlock);
}

static inline Eina_Lock_Result
eina_spinlock_take(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_POSIX_SPINLOCK)
   int t;

   for (;;)
     {
        t = pthread_spin_lock(spinlock);
        if (t == 0) break;
        else EINA_LOCK_ABORT_DEBUG(t, spin_lock, spinlock);
     }

   return EINA_LOCK_SUCCEED;
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   return _eina_spinlock_macos_take(spinlock);
#else
   return eina_lock_take(spinlock);
#endif
}

static inline Eina_Lock_Result
eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_POSIX_SPINLOCK)
   int t = pthread_spin_trylock(spinlock);
   if (t == 0) return EINA_LOCK_SUCCEED;
   else if (t == EBUSY) return EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG(t, spin_trylock, spinlock);
   return EINA_LOCK_FAIL;
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   return _eina_spinlock_macos_take_try(spinlock);
#else
   return eina_lock_take_try(spinlock);
#endif
}

static inline Eina_Lock_Result
eina_spinlock_release(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_POSIX_SPINLOCK)
   int ok = pthread_spin_unlock(spinlock);
   if (ok == 0) return EINA_LOCK_SUCCEED;
   else if (ok == EPERM) return EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG(ok, spin_unlock, spinlock);
   return EINA_LOCK_FAIL;
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   return _eina_spinlock_macos_release(spinlock);
#else
   return eina_lock_release(spinlock);
#endif
}

static inline Eina_Bool
eina_semaphore_new(Eina_Semaphore *sem, int count_init)
{
   return _eina_semaphore_new(sem, count_init);
}

static inline Eina_Bool
eina_semaphore_free(Eina_Semaphore *sem)
{
   return _eina_semaphore_free(sem);
}

static inline Eina_Bool
eina_semaphore_lock(Eina_Semaphore *sem)
{
   if (sem)
     {
        for (;;)
          {
             if (
#if defined(EINA_HAVE_OSX_SEMAPHORE)
                 semaphore_wait(*sem)
#else
                 sem_wait(sem)
#endif
                 == 0)
               return EINA_TRUE;
             else if (errno != EINTR) goto err;
          }
     }
   return EINA_FALSE;
err:
   if (errno == EDEADLK) EINA_LOCK_DEADLOCK_DEBUG(sem_wait, sem);
   return EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_release(Eina_Semaphore *sem, int count_release EINA_UNUSED)
{
   if (sem)
#if defined(EINA_HAVE_OSX_SEMAPHORE)
     return (semaphore_signal(*sem) == KERN_SUCCESS) ? EINA_TRUE : EINA_FALSE;
#else
     return (sem_post(sem) == 0) ? EINA_TRUE : EINA_FALSE;
#endif
   return EINA_FALSE;
}

#undef _XOPEN_SOURCE
// This is necessary to let third party still define this macro
#ifdef EINA_XOPEN_SOURCE
# define _XOPEN_SOURCE EINA_XOPEN_SOURCE
#endif

#endif
