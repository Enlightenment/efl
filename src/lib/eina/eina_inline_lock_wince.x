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

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

EAPI extern Eina_Bool _threads_activated;
EAPI extern Eina_Bool _eina_thread_tls_cb_register(Eina_TLS key, Eina_TLS_Delete_Cb cb);
EAPI extern Eina_Bool _eina_thread_tls_cb_unregister(Eina_TLS key);
EAPI extern Eina_Bool _eina_thread_tls_key_add(Eina_TLS key);

/** @privatesection @{ */
typedef HANDLE    Eina_Lock;
typedef Eina_Lock Eina_Spinlock;
typedef Eina_Lock Eina_RWLock;
typedef DWORD     Eina_TLS;
typedef void *    Eina_Semaphore;
/** @} privatesection */

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   Eina_Lock m;

   m = CreateMutex(NULL, FALSE, NULL);
   if (m) *mutex = m;
   return (m != NULL);
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   CloseHandle(*mutex);
}

static inline Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex)
{
   DWORD res;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_FAIL;
#endif

   res = WaitForSingleObject(*mutex, INFINITE);
   if ((res == WAIT_ABANDONED) || (res == WAIT_FAILED))
     return EINA_LOCK_FAIL;

   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
   return eina_lock_take(*mutex);
}

static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return ;
#endif

   return ReleaseMutex(*mutex) ? EINA_LOCK_SUCCEED : EINA_LOCK_FAIL;
}

static inline void
eina_lock_debug(const Eina_Lock *mutex)
{
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   return EINA_FALSE;
}

static inline void
eina_condition_free(Eina_Condition *cond)
{
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_timedwait(Eina_Condition *cond, double t)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex)
{
   return eina_lock_new(mutex);
}

static inline void
eina_rwlock_free(Eina_RWLock *mutex)
{
   return eina_lock_free(mutex);
}

static inline Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex)
{
   return eina_lock_take(mutex);
}

static inline Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex)
{
   return eina_lock_take(mutex);
}

static inline Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex)
{
   return eina_lock_release(mutex);
}

static inline Eina_Bool
eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   if ((*key = TlsAlloc()) == TLS_OUT_OF_INDEXES)
      return EINA_FALSE;
   if (delete_cb)
     {
        if (!_eina_thread_tls_cb_register(*key, delete_cb))
          {
             TlsFree(*key);
             return EINA_FALSE;
          }
     }
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
   _eina_thread_tls_cb_unregister(key);
   TlsFree(key);
}

static inline void *
eina_tls_get(Eina_TLS key)
{
   return (void*)TlsGetValue(key);
}

static inline Eina_Bool 
eina_tls_set(Eina_TLS key, const void *data)
{
   if (TlsSetValue(key, (LPVOID)data) == 0)
      return EINA_FALSE;
   _eina_thread_tls_key_add(key);
   return EINA_TRUE;
}

static inline Eina_Bool
eina_semaphore_new(Eina_Semaphore *sem EINA_UNUSED,
                   int count_init EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_free(Eina_Semaphore *sem EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_lock(Eina_Semaphore *sem EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_semaphore_release(Eina_Semaphore *sem EINA_UNUSED,
                       int count_release EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_spinlock_new(Eina_Spinlock *spinlock)
{
   return eina_lock_new(spinlock);
}

static inline void
eina_spinlock_free(Eina_Spinlock *spinlock)
{
   eina_lock_free(spinlock);
}

static inline Eina_Lock_Result
eina_spinlock_take(Eina_Spinlock *spinlock)
{
   return eina_lock_take(spinlock);
}

static inline Eina_Lock_Result
eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
   return eina_lock_take_try(spinlock);
}

static inline Eina_Lock_Result
eina_spinlock_release(Eina_Spinlock *spinlock)
{
   return eina_lock_release(spinlock);
}

#include "eina_inline_lock_barrier.x"

#endif
