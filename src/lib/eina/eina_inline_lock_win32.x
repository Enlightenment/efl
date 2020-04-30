/* EINA - EFL data type library
 * Copyright (C) 2020 Lucas Cavalcante de Sousa
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

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <synchapi.h>
#undef WIN32_LEAN_AND_MEAN

#include "unimplemented.h"

#include <errno.h>

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
typedef struct _Eina_Barrier Eina_Barrier;
typedef LPDWORD Eina_TLS;
typedef HANDLE Eina_Semaphore;

#ifdef EINA_HAVE_WIN32_SPINLOCK
typedef PCRITICAL_SECTION Eina_Spinlock;
#else
typedef Eina_Lock Eina_Spinlock;
#endif

EAPI void eina_lock_debug(const Eina_Lock *mutex);

/** @privatesection  @{ */
typedef HANDLE _Eina_Thread;
typedef PCRITICAL_SECTION _Eina_Mutex_t;
typedef CONDITION_VARIABLE _Eina_Condition_t;
typedef PSRWLOCK _Eina_RWLock_t;

struct _Eina_Lock
{
   _Eina_Mutex_t     mutex; /**< The mutex that handles the locking */
#ifdef EINA_HAVE_DEBUG_THREADS
   EINA_INLIST; /**< Keeps track of the threads waiting for the lock */
   _Eina_Thread         lock_thread_id; /**< The ID of the thread that currently has the lock */
   Eina_Lock_Bt_Func lock_bt[EINA_LOCK_DEBUG_BT_NUM]; /**< The function that will produce a backtrace on the thread that has the lock */
   int               lock_bt_num; /**< Number of addresses in the backtrace */
   Eina_Bool         locked : 1;  /**< Indicates locked or not locked */
   Eina_Bool         recursive : 1;  /**< Indicates recursive lock */
#endif /* EINA_HAVE_DEBUG_THREADS */
};

struct _Eina_Condition
{
   Eina_Lock         *lock;      /**< The lock for this condition */
   _Eina_Condition_t condition; /**< The condition variable */
#if defined(__clockid_t_defined)
   clockid_t         clkid;     /**< The attached clock for timedwait */
#endif
};

enum _Eina_RWLock_Mode {
   _Eina_RWLock_Mode_Shared,
   _Eina_RWLock_Mode_Exclusive,
   _Eina_RWLock_Mode_None
};

struct _Eina_RWLock
{
   _Eina_RWLock_t mutex; /**< The mutex that handles the locking */
#ifdef EINA_HAVE_DEBUG_THREADS
   _Eina_Thread   lock_thread_wid; /**< The ID of the thread that currently has the lock */
#endif
   enum _Eina_RWLock_Mode mode; /**< The mode the SRWLock is (Shared, Exclusive or None)*/
};
/** @} privatesection */

EAPI extern Eina_Bool _eina_threads_activated;

#ifdef EINA_HAVE_DEBUG_THREADS
EAPI extern int _eina_threads_debug;
EAPI extern _Eina_Thread _eina_main_loop;
EAPI extern Eina_Lock _eina_tracking_lock;
EAPI extern Eina_Inlist *_eina_tracking;
#endif

EAPI Eina_Bool eina_lock_new(Eina_Lock *mutex);
EAPI void eina_lock_free(Eina_Lock *mutex);
EAPI Eina_Lock_Result eina_lock_take(Eina_Lock *mutex);
EAPI Eina_Lock_Result eina_lock_take_try(Eina_Lock *mutex);
EAPI Eina_Lock_Result eina_lock_release(Eina_Lock *mutex);
EAPI Eina_Bool eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);
EAPI void eina_condition_free(Eina_Condition *cond);
EAPI Eina_Bool eina_condition_wait(Eina_Condition *cond);
EAPI Eina_Bool eina_condition_broadcast(Eina_Condition *cond);

static inline Eina_Bool
_eina_lock_new(Eina_Lock *mutex, Eina_Bool recursive)
{
   InitializeCriticalSection((mutex->mutex));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;
   return EINA_FALSE;
}

static inline void
_eina_lock_free(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   if (mutex->locked)
     {
        EnterCriticalSection(_eina_tracking_lock);
        _eina_tracking = eina_inlist_remove(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        LeaveCriticalSection(_eina_tracking_lock);
     }
#endif
   DWORD ok;

   DeleteCriticalSection((mutex->mutex));
   ok = GetLastError();
   if (ok != ERROR_SUCCESS) EINA_LOCK_ABORT_DEBUG((int)ok, mutex_destroy
                                                  , mutex);
}

static inline Eina_Lock_Result
_eina_lock_take_try(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   int ok = TryEnterCriticalSection((mutex->mutex));
   DWORD err = GetLastError();
   if (ok != 0) ret = EINA_LOCK_SUCCEED;
   else if (err == ERROR_POSSIBLE_DEADLOCK)
     {
        eina_lock_debug(mutex);
        ret = EINA_LOCK_DEADLOCK;
     }
   else if (err != ERROR_TIMEOUT) EINA_LOCK_ABORT_DEBUG((int)err, trylock, mutex);
#ifdef EINA_HAVE_DEBUG_THREADS
   if (ret == EINA_LOCK_SUCCEED)
     {
        /* recursive locks can't make use of any of this */
        if (mutex->recursive) return ret;
        mutex->locked = 1;
        mutex->lock_thread_id = GetCurrentThreadId();
        /* backtrace() can somehow generate EINVAL even though this is not documented anywhere? */
        int err = errno;
        mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
        errno = err;

        EnterCriticalSection(_eina_tracking_lock);
        _eina_tracking = eina_inlist_append(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        LeaveCriticalSection(_eina_tracking_lock);
     }
#endif
   return ret;
}

static inline Eina_Lock_Result
_eina_lock_take(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;
   DWORD ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   if (eina_lock_take_try(mutex) == EINA_LOCK_SUCCEED) return EINA_LOCK_SUCCEED;

   if (_eina_threads_debug >= 100)
     {
        struct timeval t0, t1;
        int dt;

        gettimeofday(&t0, NULL);
        ok = EnterCriticalSection((mutex->mutex));
        gettimeofday(&t1, NULL);

        dt = (t1.tv_sec - t0.tv_sec) * 1000000;
        if (t1.tv_usec > t0.tv_usec) dt += (t1.tv_usec - t0.tv_usec);
        else dt -= t0.tv_usec - t1.tv_usec;
        if (dt > _eina_threads_debug) abort();
     }
   else
     {
#endif
        EnterCriticalSection((mutex->mutex));
        ok = GetLastError();
#ifdef EINA_HAVE_DEBUG_THREADS
     }
#endif

   if (ok == ERROR_SUCCESS) ret = EINA_LOCK_SUCCEED;
   else if (ok == ERROR_POSSIBLE_DEADLOCK)
     {
        eina_lock_debug(mutex);
        ret = EINA_LOCK_DEADLOCK;
#ifdef EINA_HAVE_DEBUG_THREADS
        if (_eina_threads_debug) abort();
#endif
     }
   else if (ok != ERROR_TIMEOUT) EINA_LOCK_ABORT_DEBUG((int)ok, lock, mutex);

#ifdef EINA_HAVE_DEBUG_THREADS
   /* recursive locks can't make use of any of this */
   if (mutex->recursive) return ret;
   mutex->locked = 1;
   mutex->lock_thread_id = GetCurrentThreadId();
   /* backtrace() can somehow generate EINVAL even though this is not documented anywhere? */
   int err = errno;
   mutex->lock_bt_num = backtrace((void **)(mutex->lock_bt), EINA_LOCK_DEBUG_BT_NUM);
   errno = err;

   EnterCriticalSection(_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking,
                                       EINA_INLIST_GET(mutex));
   LeaveCriticalSection(_eina_tracking_lock);
#endif

   return ret;
}

static inline Eina_Lock_Result
_eina_lock_release(Eina_Lock *mutex)
{
   Eina_Lock_Result ret = EINA_LOCK_FAIL;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
/* recursive locks can't make use of any of this */
   if (!mutex->recursive)
     {
        mutex->locked = 0;
        mutex->lock_thread_id = 0;
        memset(mutex->lock_bt, 0, EINA_LOCK_DEBUG_BT_NUM * sizeof(Eina_Lock_Bt_Func));
        mutex->lock_bt_num = 0;
        EnterCriticalSection(_eina_tracking_lock);
        _eina_tracking = eina_inlist_remove(_eina_tracking,
                                            EINA_INLIST_GET(mutex));
        LeaveCriticalSection(_eina_tracking_lock);
     }
#endif
   LeaveCriticalSection((mutex->mutex));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) ret = EINA_LOCK_SUCCEED;
   else if (ok != ERROR_ACCESS_DENIED) ret = EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG((int)ok, unlock, mutex);
   return ret;
}

UNIMPLEMENTED static inline Eina_Bool
_eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   // TODO
   #warning eina_condition_new is not implemented
   return EINA_TRUE;
}

static inline void
_eina_condition_free(Eina_Condition *cond)
{
   return;
}

static inline Eina_Bool
_eina_condition_wait(Eina_Condition *cond)
{
   Eina_Bool r = EINA_FALSE;

#ifdef EINA_HAVE_DEBUG_THREADS
   assert(_eina_threads_activated);
   assert(cond->lock != NULL);

   EnterCriticalSection(_eina_tracking_lock);
   _eina_tracking = eina_inlist_remove(_eina_tracking,
                                       EINA_INLIST_GET(cond->lock));
   LeaveCriticalSection(_eina_tracking_lock);
#endif

   int ok = SleepConditionVariableCS(&(cond->condition)
                                , (cond->lock->mutex), INFINITE);
   DWORD err = GetLastError();
   if (ok != 0) r = EINA_TRUE;
   else if (err != ERROR_ACCESS_DENIED) r = EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG((int)ok, cond_wait, cond);

#ifdef EINA_HAVE_DEBUG_THREADS
   EnterCriticalSection(_eina_tracking_lock);
   _eina_tracking = eina_inlist_append(_eina_tracking
                                      , EINA_INLIST_GET(cond->lock));
   LeaveCriticalSection(_eina_tracking_lock);
#endif

   return r;
}

UNIMPLEMENTED static inline Eina_Bool
_eina_condition_timedwait(Eina_Condition *cond, double t)
{
   //** TODO **//
   #warning eina_condition_timedwait is not implemented
   return EINA_TRUE;
}

static inline Eina_Bool
_eina_condition_broadcast(Eina_Condition *cond)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   WakeAllConditionVariable(&(cond->condition));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;

   EINA_LOCK_ABORT_DEBUG((int)ok, cond_broadcast, cond);
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_condition_signal(Eina_Condition *cond)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   assert(cond->lock != NULL);
#endif

   WakeConditionVariable(&(cond->condition));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;

   EINA_LOCK_ABORT_DEBUG((int)ok, cond_signal, cond);
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_rwlock_new(Eina_RWLock *mutex)
{
   InitializeSRWLock((mutex->mutex));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;
   else if ((ok == ERROR_NOT_ENOUGH_MEMORY) || (ok == ERROR_ACCESS_DENIED)
            || (ok == ERROR_OUTOFMEMORY))
     {
        return EINA_FALSE;
     }
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_init, mutex);
   return EINA_FALSE;
}

static inline void
_eina_rwlock_free(Eina_RWLock *mutex)
{
    return;
}

static inline Eina_Lock_Result
_eina_rwlock_take_read(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   AcquireSRWLockShared((mutex->mutex));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS)
     {
        mutex->mode = _Eina_RWLock_Mode_Shared;
        return EINA_LOCK_SUCCEED;
     }
   else if (ok == ERROR_NOT_ENOUGH_MEMORY || ok == ERROR_OUTOFMEMORY)
     {
        return EINA_LOCK_FAIL;
     }
   else if (ok == ERROR_POSSIBLE_DEADLOCK)
     {
        EINA_LOCK_DEADLOCK_DEBUG(rwlock_rdlock, mutex);
     }
   else EINA_LOCK_ABORT_DEBUG((int)ok, rwlock_rdlock, mutex);
   return EINA_LOCK_FAIL;
}
static inline Eina_Lock_Result
_eina_rwlock_take_write(Eina_RWLock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   AcquireSRWLockExclusive((mutex->mutex));
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS)
     {
        mutex->mode = _Eina_RWLock_Mode_Exclusive;
        return EINA_LOCK_SUCCEED;
     }
   else if (ok == ERROR_NOT_ENOUGH_MEMORY || ok == ERROR_OUTOFMEMORY)
     {
        return EINA_LOCK_FAIL;
     }
   else if (ok == ERROR_POSSIBLE_DEADLOCK)
     {
        EINA_LOCK_DEADLOCK_DEBUG(rwlock_rdlock, mutex);
     }
   else EINA_LOCK_ABORT_DEBUG((int)ok, rwlock_rdlock, mutex);
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
_eina_rwlock_release(Eina_RWLock *mutex)
{
   DWORD ok;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   if (mutex->mode == _Eina_RWLock_Mode_Exclusive)
     {
        mutex->mode = _Eina_RWLock_Mode_None;
        ReleaseSRWLockExclusive((mutex->mutex));
        ok = GetLastError();
        if (ok  == ERROR_SUCCESS) return EINA_LOCK_SUCCEED;
        mutex->mode = _Eina_RWLock_Mode_Exclusive;
     }
   else if (mutex->mode == _Eina_RWLock_Mode_Shared)
     {
        mutex->mode = _Eina_RWLock_Mode_None;
        ReleaseSRWLockShared((mutex->mutex));
        ok = GetLastError();
        if (ok  == ERROR_SUCCESS) return EINA_LOCK_SUCCEED;
        mutex->mode = _Eina_RWLock_Mode_Shared;
     }
   if (ok == ERROR_ACCESS_DENIED) return EINA_LOCK_FAIL;

   EINA_LOCK_ABORT_DEBUG((int)ok, rwlock_unlock, mutex);
   return EINA_LOCK_FAIL;
}

static inline Eina_Bool
_eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   * key = TlsAlloc();
   DWORD ok = GetLastError();
   if (key != TLS_OUT_OF_INDEXES || ok != ERROR_SUCCESS) return EINA_TRUE;
   else return EINA_FALSE;
}

static inline void
_eina_tls_free(Eina_TLS key)
{
   TlsFree(key);
}

static inline void *
_eina_tls_get(Eina_TLS key)
{
   return TlsGetValue(key);
}

static inline Eina_Bool
_eina_tls_set(Eina_TLS key, const void *data)
{
   int ok = TlsSetValue(key, (void *) data);
   DWORD err = GetLastError();
   if (ok != 0 && err == ERROR_SUCCESS) return EINA_TRUE;
   else return EINA_TRUE;
}

#ifdef EINA_HAVE_WIN32_BARRIER
struct _Eina_Barrier
{
   LPSYNCHRONIZATION_BARRIER barrier;
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
#ifdef EINA_HAVE_WIN32_BARRIER
   InitializeSynchronizationBarrier(&(barrier->barrier), (LONG) needed, 0);
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;
   else if ((ok == ERROR_NOT_ENOUGH_MEMORY) || (ok == ERROR_ACCESS_DENIED)
            || (ok == ERROR_OUTOFMEMORY))
    {
       return EINA_FALSE;
    }
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
#ifdef EINA_HAVE_WIN32_BARRIER
   DeleteSynchronizationBarrier(&(barrier->barrier));
   DWORD ok = GetLastError();
   if (ok != ERROR_SUCCESS) EINA_LOCK_ABORT_DEBUG(ok, barrier_destroy, barrier);
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
#ifdef EINA_HAVE_WIN32_BARRIER
   EnterSyncronizationBarrier(&(barrier->barrier)
                              , SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_TRUE;
   else EINA_LOCK_ABORT_DEBUG((int)ok, barrier_wait, barrier);
   return EINA_TRUE;
#else
   eina_lock_take(&(barrier->cond_lock));
   barrier->called++;
   if (barrier->called == barrier->needed)
     {
        barrier->called = 0;
        eina_condition_broadcast(&(barrier->cond));
     }
   else eina_condition_wait(&(barrier->cond));
   eina_lock_release(&(barrier->cond_lock));
   return EINA_TRUE;
#endif
}

static inline Eina_Bool
_eina_spinlock_new(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_WIN32_SPINLOCK
   int ok = InitializeCriticalSectionAndSpinCount(spinlock, INFINITE);
   DWORD err = GetLastError();
   if (ok != 0) return EINA_TRUE;
   else if ((err == ERROR_NOT_ENOUGH_MEMORY) || (err == ERROR_ACCESS_DENIED)
            || (err == ERROR_OUTOFMEMORY))
    {
       return EINA_FALSE;
    }
   else EINA_LOCK_ABORT_DEBUG((int)err, spin_init, spinlock);
   return EINA_FALSE;
#else
   return eina_lock_new(spinlock);
#endif
}

static inline void
_eina_spinlock_free(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_WIN32_SPINLOCK
   DeleteCriticalSection(spinlock);
   DWORD ok = GetLastError();
   if (ok != ERROR_SUCCESS) EINA_LOCK_ABORT_DEBUG(ok, spin_destroy, spinlock);
#else
   eina_lock_free(spinlock);
#endif
}

static inline Eina_Lock_Result
_eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_WIN32_SPINLOCK
   int ok = TryEnterCriticalSection(spinlock);
   DWORD err = GetLastError();
   if (err == ERROR_SUCCESS) return EINA_LOCK_SUCCEED;
   else if (ok == 0 || err == ERROR_TIMEOUT) EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG((int)err, trylock, mutex);
   return EINA_LOCK_FAIL
#else
   return eina_lock_take_try(spinlock);
#endif
}

static inline Eina_Lock_Result
_eina_spinlock_take(Eina_Spinlock *spinlock)
{
#ifdef EINA_HAVE_WIN32_SPINLOCK
# ifdef EINA_HAVE_DEBUG_THREADS
   if (eina_spinlock_take_try(spinlock) == EINA_LOCK_SUCCEED)
     return EINA_LOCK_SUCCEED;
# endif

   for (;;)
     {
        int ok = EnterCriticalSection(spinlock);
        if (ok != 0) break;
        else {
           DWORD err = GetLastError();
           EINA_LOCK_ABORT_DEBUG((int)err, spin_lock, spinlock);
        }
     }

   return EINA_LOCK_SUCCEED;
#else
   return eina_lock_take(spinlock);
#endif
}

static inline Eina_Lock_Result
_eina_spinlock_release(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_WIN32_SPINLOCK)
   LeaveCriticalSection(spinlock);
   DWORD ok = GetLastError();
   if (ok == ERROR_SUCCESS) return EINA_LOCK_SUCCEED;
   else if (ok == ERROR_ACCESS_DENIED) return EINA_LOCK_FAIL;
   else EINA_LOCK_ABORT_DEBUG((int)ok, spin_unlock, spinlock);
   return EINA_LOCK_FAIL;
#else
   return eina_lock_release(spinlock);
#endif
}

static inline Eina_Bool
_eina_semaphore_new(Eina_Semaphore *sem, int count_init)
{
   if (sem && (count_init >= 0))
     {
        sem = CreateSemaphoreA(NULL, count_init, count_init, NULL);
        DWORD ok = GetLastError();
        if (ok == ERROR_SUCCESS) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_semaphore_free(Eina_Semaphore *sem)
{
   if (sem)
     {
        CloseHandle(sem);
        DWORD ok = GetLastError();
        if (ok == ERROR_SUCCESS) return EINA_TRUE;
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
             WaitForSingleObject(sem, INFINITE);
             DWORD ok = GetLastError();
             if (ok == ERROR_SUCCESS)
                return EINA_TRUE;
             else if (ok == WAIT_OBJECT_0 || ok == WAIT_TIMEOUT)
                continue;
             else if (errno == ERROR_POSSIBLE_DEADLOCK)
              {
                 EINA_LOCK_DEADLOCK_DEBUG(sem_wait, sem);
                 return EINA_FALSE;
              }
          }
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eina_semaphore_release(Eina_Semaphore *sem, int count_release EINA_UNUSED)
{
   if (sem)
     return (ReleaseSemaphore(sem, 1, NULL) != 0) ? EINA_TRUE : EINA_FALSE;
   return EINA_FALSE;
}


#endif
