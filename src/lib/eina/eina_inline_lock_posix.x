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

#ifdef EINA_HAVE_OSX_SPINLOCK
/*
 * macOS 10.12 introduced the os_unfair_lock API which
 * deprecates OSSpinLock, while keeping compatible.
 *
 * The Spinlock API is not inlined because it would imply including
 * stdbool.h, which is not wanted: it would introduce new macros,
 * and break compilation of existing programs.
 */
# if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
#  include <os/lock.h>
#  define SPINLOCK_GET(LCK) ((os_unfair_lock_t)(LCK))
# else
#  include <libkern/OSAtomic.h>
#  define SPINLOCK_GET(LCK) ((OSSpinLock *)(LCK))
#  define os_unfair_lock_lock(LCK) OSSpinLockLock(LCK)
#  define os_unfair_lock_unlock(LCK) OSSpinLockUnlock(LCK)
#  define os_unfair_lock_trylock(LCK) OSSpinLockTry(LCK)
# endif
#elif defined(EINA_HAVE_PTHREAD_SPINLOCK)
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

EINA_API void _eina_lock_debug_abort(int err, const char *fn, const volatile void *ptr);
EINA_API void _eina_lock_debug_deadlock(const char *fn, const volatile void *ptr);

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

#ifdef EINA_HAVE_PTHREAD_SPINLOCK
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

EINA_API void eina_lock_debug(const Eina_Lock *mutex);

/** @privatesection  @{ */
struct _Eina_Lock
{
   pthread_mutex_t   mutex; /**< The mutex that handles the locking */
#ifdef EINA_HAVE_DEBUG_THREADS
   EINA_INLIST; /**< Keeps track of the threads waiting for the lock */
   pthread_t         lock_thread_id; /**< The ID of the thread that currently has the lock */
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM]; /**< The function that will produce a backtrace on the thread that has the lock */
   int               lock_bt_num; /**< Number of addresses in the backtrace */
   Eina_Bool         locked : 1;  /**< Indicates locked or not locked */
   Eina_Bool         recursive : 1;  /**< Indicates recursive lock */
#endif /* EINA_HAVE_DEBUG_THREADS */
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

EINA_API extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
EINA_API extern int _eina_threads_debug;
EINA_API extern pthread_t _eina_main_loop;
EINA_API extern pthread_mutex_t _eina_tracking_lock;
EINA_API extern Eina_Inlist *_eina_tracking;
#endif


EINA_API Eina_Bool _eina_lock_new(Eina_Lock *mutex, Eina_Bool recursive);
EINA_API void      _eina_lock_free(Eina_Lock *mutex);
EINA_API Eina_Bool _eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);
EINA_API void      _eina_condition_free(Eina_Condition *cond);
EINA_API Eina_Bool _eina_rwlock_new(Eina_RWLock *mutex);
EINA_API void      _eina_rwlock_free(Eina_RWLock *mutex);
EINA_API Eina_Bool _eina_spinlock_new(Eina_Spinlock *spinlock);
EINA_API void      _eina_spinlock_free(Eina_Spinlock *spinlock);
EINA_API Eina_Bool _eina_semaphore_new(Eina_Semaphore *sem, int count_init);
EINA_API Eina_Bool _eina_semaphore_free(Eina_Semaphore *sem);
#ifdef EINA_HAVE_OSX_SPINLOCK
EINA_API Eina_Lock_Result _eina_spinlock_macos_take(Eina_Spinlock *spinlock);
EINA_API Eina_Lock_Result _eina_spinlock_macos_take_try(Eina_Spinlock *spinlock);
EINA_API Eina_Lock_Result _eina_spinlock_macos_release(Eina_Spinlock *spinlock);
#endif

static inline Eina_Bool
_eina_lock_new(Eina_Lock *mutex, Eina_Bool recursive)
{
   pthread_mutexattr_t attr;
   Eina_Bool ok = EINA_FALSE;

   if (pthread_mutexattr_init(&attr) != 0) return EINA_FALSE;
   if (recursive)
     {
        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) goto fail_release;
     }
#ifdef EINA_HAVE_DEBUG_THREADS
   else if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto fail_release;
#endif
   if (pthread_mutex_init(&(mutex->mutex), &attr) != 0) goto fail_release;
   ok = EINA_TRUE;
fail_release:
   pthread_mutexattr_destroy(&attr);
   return ok;
}

static inline void
_eina_lock_free(Eina_Lock *mutex)
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

   int ok;
   ok = pthread_mutex_destroy(&(mutex->mutex));
   if (ok != 0) EINA_LOCK_ABORT_DEBUG(ok, mutex_destroy, mutex);
}


static inline Eina_Lock_Result
_eina_lock_take_try(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
        return EINA_LOCK_SUCCEED;
     }
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
        /* backtrace() can somehow generate EINVAL even though this is not documented anywhere? */
        int err = errno;
        mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
        errno = err;

        pthread_mutex_lock(&_eina_tracking_lock);
        _eina_tracking = eina_inlist_append(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        pthread_mutex_unlock(&_eina_tracking_lock);
     }
#endif
   return ret;
}

static inline Eina_Lock_Result
_eina_lock_take(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
        return EINA_LOCK_SUCCEED;
     }
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   if (eina_lock_take_try(mutex) == EINA_LOCK_SUCCEED) return EINA_LOCK_SUCCEED;
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   if (_eina_threads_debug >= 100)
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
   /* backtrace() can somehow generate EINVAL even though this is not documented anywhere? */
   int err = errno;
   mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
   errno = err;

   pthread_mutex_lock(&_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking,
                                       EINA_INLIST_GET(mutex));
   pthread_mutex_unlock(&_eina_tracking_lock);
#endif

   return ret;
}

static inline Eina_Lock_Result
_eina_lock_release(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
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
_eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   pthread_condattr_t attr;
   int ok;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(mutex != NULL);
#endif

   cond->lock = mutex;
   pthread_condattr_init(&attr);

   /* OSX doesn't provide clockid_t or clock_gettime. */
#if defined(__clockid_t_defined)
   cond->clkid = (clockid_t) 0;
   /* We try here to chose the best clock for cond_timedwait */
# if defined(CLOCK_MONOTONIC_RAW)
   if (!pthread_condattr_setclock(&attr, CLOCK_MONOTONIC_RAW))
     cond->clkid = CLOCK_MONOTONIC_RAW;
# endif
# if defined(CLOCK_MONOTONIC)
   if (!cond->clkid && !pthread_condattr_setclock(&attr, CLOCK_MONOTONIC))
     cond->clkid = CLOCK_MONOTONIC;
# endif
# if defined(CLOCK_REALTIME)
   if (!cond->clkid && !pthread_condattr_setclock(&attr, CLOCK_REALTIME))
     cond->clkid = CLOCK_REALTIME;
# endif
#endif

   ok = pthread_cond_init(&cond->condition, &attr);
   if (ok != 0)
     {
        pthread_condattr_destroy(&attr);
#ifdef EINA_HAVE_DEBUG_THREADS
        if (ok == EBUSY)
          fprintf(stderr, "EINA ERROR: eina_condition_new on already initialized Eina_Condition\n");
#endif
        return EINA_FALSE;
     }

   pthread_condattr_destroy(&attr);
   return EINA_TRUE;
}

static inline void
_eina_condition_free(Eina_Condition *cond)
{
   pthread_cond_destroy(&(cond->condition));
}

static inline Eina_Bool
_eina_condition_wait(Eina_Condition *cond)
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
_eina_condition_timedwait(Eina_Condition *cond, double t)
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
_eina_condition_broadcast(Eina_Condition *cond)
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
_eina_condition_signal(Eina_Condition *cond)
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
_eina_rwlock_new(Eina_RWLock *mutex)
{
   int ok;

   ok = pthread_rwlock_init(&(mutex->mutex), NULL);
   if (ok == 0) return EINA_TRUE;
   else if ((ok == EAGAIN) || (ok == ENOMEM)) return EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_init, mutex);
   return EINA_FALSE;
}

static inline void
_eina_rwlock_free(Eina_RWLock *mutex)
{
   pthread_rwlock_destroy(&(mutex->mutex));
}

static inline Eina_Lock_Result
_eina_rwlock_take_read(Eina_RWLock *mutex)
{
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
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
_eina_rwlock_take_write(Eina_RWLock *mutex)
{
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
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
_eina_rwlock_release(Eina_RWLock *mutex)
{
   int ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated)
     {
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
_eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   if (pthread_key_create(key, delete_cb) == 0) return EINA_TRUE;
   return EINA_FALSE;
}

static inline void
_eina_tls_free(Eina_TLS key)
{
   pthread_key_delete(key);
}

static inline void *
_eina_tls_get(Eina_TLS key)
{
   return pthread_getspecific(key);
}

static inline Eina_Bool
_eina_tls_set(Eina_TLS key, const void *data)
{
   if (pthread_setspecific(key, data) == 0) return EINA_TRUE;
   return EINA_FALSE;
}

typedef struct _Eina_Barrier Eina_Barrier;

#ifdef EINA_HAVE_PTHREAD_BARRIER
struct _Eina_Barrier
{
   pthread_barrier_t barrier;
};
#else
struct _Eina_Barrier
{
   int needed;           /**< The number of waiting threads that will cause the barrier to signal and reset. */
   int called;           /**< The number of threads that are waiting on this barrier. */
   Eina_Lock cond_lock;  /**< The lock for the barrier */
   Eina_Condition cond;  /**< The condition variable for the barrier */
};
#endif

static inline Eina_Bool
_eina_barrier_new(Eina_Barrier *barrier, int needed)
{
#ifdef EINA_HAVE_PTHREAD_BARRIER
   int ok = pthread_barrier_init(&(barrier->barrier), NULL, needed);
   if (ok == 0) return EINA_TRUE;
   else if ((ok == EAGAIN) || (ok == ENOMEM)) return EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG(ok, barrier_init, barrier);
   return EINA_FALSE;
#else
   barrier->needed = needed;
   barrier->called = 0;
   if (eina_lock_new(&(barrier->cond_lock)))
     {
        if (eina_condition_new(&(barrier->cond), &(barrier->cond_lock)))
          return EINA_TRUE;
     }
   return EINA_FALSE;
#endif
}

static inline  void
_eina_barrier_free(Eina_Barrier *barrier)
{
#ifdef EINA_HAVE_PTHREAD_BARRIER
   int ok = pthread_barrier_destroy(&(barrier->barrier));
   if (ok != 0) EINA_LOCK_ABORT_DEBUG(ok, barrier_destroy, barrier);
#else
   eina_condition_free(&(barrier->cond));
   eina_lock_free(&(barrier->cond_lock));
   barrier->needed = 0;
   barrier->called = 0;
#endif
}

static inline Eina_Bool
_eina_barrier_wait(Eina_Barrier *barrier)
{
#ifdef EINA_HAVE_PTHREAD_BARRIER
   int ok = pthread_barrier_wait(&(barrier->barrier));
   if (ok == 0) return EINA_TRUE;
   else if (ok == PTHREAD_BARRIER_SERIAL_THREAD) return EINA_TRUE;
   else EINA_LOCK_ABORT_DEBUG(ok, barrier_wait, barrier);
   return EINA_TRUE;
#else
   eina_lock_take(&(barrier->cond_lock));
   barrier->called++;
   if (barrier->called == barrier->needed)
     {
        barrier->called = 0;
        eina_condition_broadcast(&(barrier->cond));
     }
   else
     eina_condition_wait(&(barrier->cond));
   eina_lock_release(&(barrier->cond_lock));
   return EINA_TRUE;
#endif
}

EINA_API Eina_Bool _eina_barrier_new(Eina_Barrier *barrier, int needed);
EINA_API void      _eina_barrier_free(Eina_Barrier *barrier);

static inline Eina_Bool
_eina_spinlock_new(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_PTHREAD_SPINLOCK)
   int ok = pthread_spin_init(spinlock, PTHREAD_PROCESS_PRIVATE);
   if (ok == 0) return EINA_TRUE;
   else if ((ok == EAGAIN) || (ok == ENOMEM)) return EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG(ok, spin_init, spinlock);
   return EINA_FALSE;
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   *spinlock = 0;
   return EINA_TRUE;
#else
   return eina_lock_new(spinlock);
#endif
}

static inline void
_eina_spinlock_free(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_PTHREAD_SPINLOCK)
   int ok = pthread_spin_destroy(spinlock);
   if (ok != 0) EINA_LOCK_ABORT_DEBUG(ok, spin_destroy, spinlock);
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   /* Not applicable */
   (void) spinlock;
#else
   eina_lock_free(spinlock);
#endif
}

#ifdef EINA_HAVE_OSX_SPINLOCK
static inline Eina_Lock_Result
_eina_spinlock_macos_take(Eina_Spinlock *spinlock)
{
   os_unfair_lock_lock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
_eina_spinlock_macos_take_try(Eina_Spinlock *spinlock)
{
   return (os_unfair_lock_trylock(SPINLOCK_GET(spinlock)) == true)
      ? EINA_LOCK_SUCCEED
      : EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
_eina_spinlock_macos_release(Eina_Spinlock *spinlock)
{
   os_unfair_lock_unlock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}
#endif /* EINA_HAVE_OSX_SPINLOCK */

static inline Eina_Lock_Result
_eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_PTHREAD_SPINLOCK)
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
_eina_spinlock_take(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_PTHREAD_SPINLOCK)
   int t;

#ifdef EINA_HAVE_DEBUG_THREADS
   if (eina_spinlock_take_try(spinlock) == EINA_LOCK_SUCCEED) return EINA_LOCK_SUCCEED;
#endif

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
_eina_spinlock_release(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_PTHREAD_SPINLOCK)
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
_eina_semaphore_new(Eina_Semaphore *sem, int count_init)
{
   if (sem && (count_init >= 0))
     {
#if defined(EINA_HAVE_OSX_SEMAPHORE)
        kern_return_t kr = semaphore_create(mach_task_self(), sem, SYNC_POLICY_FIFO, count_init);
        return (kr == KERN_SUCCESS) ? EINA_TRUE : EINA_FALSE;
#else
        return (sem_init(sem, 0, count_init) == 0) ? EINA_TRUE : EINA_FALSE;
#endif
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_semaphore_free(Eina_Semaphore *sem)
{
   if (sem)
     {
#if defined(EINA_HAVE_OSX_SEMAPHORE)
        return (semaphore_destroy(mach_task_self(), *sem) == KERN_SUCCESS)
        ? EINA_TRUE : EINA_FALSE;
#else
        return (sem_destroy(sem) == 0) ? EINA_TRUE : EINA_FALSE;
#endif
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_semaphore_lock(Eina_Semaphore *sem)
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
_eina_semaphore_release(Eina_Semaphore *sem, int count_release EINA_UNUSED)
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
