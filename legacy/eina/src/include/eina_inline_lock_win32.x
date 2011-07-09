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

typedef CRITICAL_SECTION       Eina_Lock;
typedef struct _Eina_Condition Eina_Condition;

#if _WIN32_WINNT >= 0x0600
struct _Eina_Condition
{
   CRITICAL_SECTION  *mutex;
   CONDITION_VARIABLE condition;
};
#else
struct _Eina_Condition
{
   int               waiters_count;
   CRITICAL_SECTION  waiters_count_lock;
   CRITICAL_SECTION *mutex;
   HANDLE            semaphore;
   HANDLE            waiters_done;
   Eina_Bool         was_broadcast;
};
#endif

typedef struct _Eina_Win32_RWLock Eina_RWLock;

struct _Eina_Win32_RWLock
{
   LONG readers_count;
   LONG writers_count;
   int readers;
   int writers;

   Eina_Lock      mutex;
   Eina_Condition cond_read;
   Eina_Condition cond_write;
};

EAPI extern Eina_Bool _eina_threads_activated;

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   InitializeCriticalSection(mutex);

   return EINA_TRUE;
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   DeleteCriticalSection(mutex);
}

static inline Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
  if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   EnterCriticalSection(mutex);

   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   return TryEnterCriticalSection(mutex) == 0 ? EINA_LOCK_FAIL : EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_SUCCEED;
#endif

   LeaveCriticalSection(mutex);

   return EINA_LOCK_SUCCEED;
}

static inline void
eina_lock_debug(const Eina_Lock *mutex)
{
   (void)mutex;
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   cond->mutex = mutex;
#if _WIN32_WINNT >= 0x0600
   InitializeConditionVariable(&cond->condition);
#else
   cond->waiters_count = 0;
   cond->was_broadcast = EINA_FALSE;
   cond->semaphore = CreateSemaphore(NULL,       // no security
                                     0,          // initially 0
                                     0x7fffffff, // max count
                                     NULL);      // unnamed
   if (!cond->semaphore)
     return EINA_FALSE;

   InitializeCriticalSection(&cond->waiters_count_lock);

   cond->waiters_done = CreateEvent(NULL,  // no security
                                    FALSE, // auto-reset
                                    FALSE, // non-signaled initially
                                    NULL); // unnamed
   if (!cond->waiters_done)
     {
        CloseHandle(cond->semaphore);
        return EINA_FALSE;
     }

   return EINA_TRUE;
#endif
}

static inline void
eina_condition_free(Eina_Condition *cond)
{
#if _WIN32_WINNT >= 0x0600
   /* Nothing to do */
#else
   CloseHandle(cond->waiters_done);
   DeleteCriticalSection(&cond->waiters_count_lock);
   CloseHandle(cond->semaphore);
#endif
}

static inline Eina_Bool
_eina_condition_internal_timedwait(Eina_Condition *cond, DWORD t)
{
#if _WIN32_WINNT >= 0x0600
   SleepConditionVariableCS(&cond->condition, cond->mutex, t);
#else
   DWORD ret;
   Eina_Bool last_waiter;

   /* Avoid race conditions. */
   EnterCriticalSection(&cond->waiters_count_lock);
   cond->waiters_count++;
   LeaveCriticalSection(&cond->waiters_count_lock);

   /*
    * This call atomically releases the mutex and waits on the
    * semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
    * are called by another thread.
    */
   ret = SignalObjectAndWait(cond->mutex, cond->semaphore, t, FALSE);
   if (ret == WAIT_FAILED)
     return EINA_FALSE;

   /* Reacquire lock to avoid race conditions. */
   EnterCriticalSection(&cond->waiters_count_lock);

   /* We're no longer waiting... */
   cond->waiters_count--;

   /* Check to see if we're the last waiter after <pthread_cond_broadcast>. */
   last_waiter = (cond->was_broadcast) && (cond->waiters_count == 0);

   LeaveCriticalSection(&cond->waiters_count_lock);

   /*
    * If we're the last waiter thread during this particular broadcast
    * then let all the other threads proceed.
    */
  if (last_waiter)
    {
       /*
        * This call atomically signals the <waiters_done_> event and waits until
        * it can acquire the <external_mutex>.  This is required to ensure fairness.
        */
       ret = SignalObjectAndWait(cond->waiters_done, cond->mutex, t, FALSE);
       if (ret == WAIT_FAILED)
         return EINA_FALSE;
    }
  else
    {
       /*
        * Always regain the external mutex since that's the guarantee we
        * give to our callers.
        */
       ret = WaitForSingleObject(cond->mutex, t);
       if (ret == WAIT_FAILED)
         return EINA_FALSE;
    }
#endif

   return EINA_TRUE;
}

static inline Eina_Bool
eina_condition_timedwait(Eina_Condition *cond, double val)
{
   return _eina_condition_internal_timedwait(cond, (DWORD)(val * 1000));
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   return _eina_condition_internal_timedwait(cond, INFINITE);
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
#if _WIN32_WINNT >= 0x0600
   WakeAllConditionVariable(&cond->condition);
   return EINA_TRUE;
#else
   Eina_Bool have_waiters;

   /*
    * This is needed to ensure that <waiters_count_> and <was_broadcast_> are
    * consistent relative to each other.
    */
   EnterCriticalSection(&cond->waiters_count_lock);
   have_waiters = EINA_FALSE;

   if (cond->waiters_count > 0)
     {
        /*
         * We are broadcasting, even if there is just one waiter...
         * Record that we are broadcasting, which helps optimize
         * <pthread_cond_wait> for the non-broadcast case.
         */
        cond->was_broadcast = EINA_TRUE;
        have_waiters = EINA_TRUE;
     }

   if (have_waiters)
     {
        DWORD ret;

        /* Wake up all the waiters atomically. */
        ret = ReleaseSemaphore(cond->semaphore, cond->waiters_count, 0);
        LeaveCriticalSection(&cond->waiters_count_lock);
        if (!ret) return EINA_FALSE;

        /*
         * Wait for all the awakened threads to acquire the counting
         * semaphore.
         */
        ret = WaitForSingleObject(cond->waiters_done, INFINITE);
        if (ret == WAIT_FAILED)
          return EINA_FALSE;
        /*
         * This assignment is okay, even without the <waiters_count_lock_> held
         * because no other waiter threads can wake up to access it.
         */
        cond->was_broadcast = EINA_FALSE;
     }
   else
     LeaveCriticalSection(&cond->waiters_count_lock);

   return EINA_TRUE;
#endif
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
#if _WIN32_WINNT >= 0x0600
   WakeConditionVariable(&cond->condition);
#else
   Eina_Bool have_waiters;

   EnterCriticalSection(&cond->waiters_count_lock);
   have_waiters = (cond->waiters_count > 0);
   LeaveCriticalSection(&cond->waiters_count_lock);

   /* If there aren't any waiters, then this is a no-op. */
  if (have_waiters)
    {
       if (!ReleaseSemaphore(cond->semaphore, 1, 0))
         return EINA_FALSE;
    }

   return EINA_TRUE;
#endif
}

static inline Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex)
{
   if (!eina_lock_new(&(mutex->mutex))) return EINA_FALSE;
   if (!eina_condition_new(&(mutex->cond_read), &(mutex->mutex)))
     goto on_error1;
   if (!eina_condition_new(&(mutex->cond_write), &(mutex->mutex)))
     goto on_error2;

   return EINA_TRUE;

 on_error2:
   eina_condition_free(&(mutex->cond_read));
 on_error1:
   eina_lock_free(&(mutex->mutex));
   return EINA_FALSE;
}

static inline void
eina_rwlock_free(Eina_RWLock *mutex)
{
   eina_condition_free(&(mutex->cond_read));
   eina_condition_free(&(mutex->cond_write));
   eina_lock_free(&(mutex->mutex));
}

static inline Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex)
{
   DWORD res;

   if (eina_lock_take(&(mutex->mutex)) == EINA_LOCK_FAIL)
     return EINA_LOCK_FAIL;

   if (mutex->writers)
     {
        mutex->readers_count++;
        while (mutex->writers)
          {
             EnterCriticalSection(&mutex->cond_write.waiters_count_lock);
             mutex->cond_read.waiters_count++;
             LeaveCriticalSection(&mutex->cond_write.waiters_count_lock);
             res = WaitForSingleObject(mutex->cond_write.semaphore, INFINITE);
             if (res != WAIT_OBJECT_0) break;
          }
        mutex->readers_count--;
     }
   if (res == 0)
     mutex->readers++;
   eina_lock_release(&(mutex->mutex));

   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex)
{
   DWORD res;

   if (eina_lock_take(&(mutex->mutex)) == EINA_LOCK_FAIL)
     return EINA_LOCK_FAIL;

   if (mutex->writers || mutex->readers > 0)
     {
        mutex->writers_count++;
        while (mutex->writers || mutex->readers > 0)
          {
             EnterCriticalSection(&mutex->cond_write.waiters_count_lock);
             mutex->cond_read.waiters_count++;
             LeaveCriticalSection(&mutex->cond_write.waiters_count_lock);
             res = WaitForSingleObject(mutex->cond_write.semaphore, INFINITE);
             if (res != WAIT_OBJECT_0) break;
          }
        mutex->writers_count--;
     }
   if (res == 0) mutex->writers_count = 1;
   eina_lock_release(&(mutex->mutex));

   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex)
{
   if (eina_lock_take(&(mutex->mutex)) == EINA_LOCK_FAIL)
     return EINA_LOCK_FAIL;

   if (mutex->writers)
     {
        mutex->writers = 0;
        if (mutex->readers_count == 1)
          {
             EnterCriticalSection(&mutex->cond_read.waiters_count_lock);
             if (mutex->cond_read.waiters_count > 0)
               ReleaseSemaphore(mutex->cond_read.semaphore, 1, 0);
             LeaveCriticalSection(&mutex->cond_read.waiters_count_lock);
          }
        else if (mutex->readers_count > 0)
          eina_condition_broadcast(&(mutex->cond_read));
        else if (mutex->writers_count > 0)
          {
             EnterCriticalSection (&mutex->cond_write.waiters_count_lock);
             if (mutex->cond_write.waiters_count > 0)
               ReleaseSemaphore(mutex->cond_write.semaphore, 1, 0);
             LeaveCriticalSection (&mutex->cond_write.waiters_count_lock);
          }
     }
   else if (mutex->readers > 0)
     {
        mutex->readers--;
        if (mutex->readers == 0 && mutex->writers_count > 0)
          {
             EnterCriticalSection (&mutex->cond_write.waiters_count_lock);
             if (mutex->cond_write.waiters_count > 0)
               ReleaseSemaphore(mutex->cond_write.semaphore, 1, 0);
             LeaveCriticalSection (&mutex->cond_write.waiters_count_lock);
          }
     }
   eina_lock_release(&(mutex->mutex));

   return EINA_LOCK_SUCCEED;
}

#endif
