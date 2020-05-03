#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "eina_config.h"
#include "Eina.h"

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

EINA_API Eina_Lock_Result
_eina_spinlock_macos_take(Eina_Spinlock *spinlock)
{
   os_unfair_lock_lock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}

EINA_API Eina_Lock_Result
_eina_spinlock_macos_take_try(Eina_Spinlock *spinlock)
{
   return (os_unfair_lock_trylock(SPINLOCK_GET(spinlock)) == true)
      ? EINA_LOCK_SUCCEED
      : EINA_LOCK_FAIL;
}

EINA_API Eina_Lock_Result
_eina_spinlock_macos_release(Eina_Spinlock *spinlock)
{
   os_unfair_lock_unlock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}
#endif /* EINA_HAVE_OSX_SPINLOCK */


Eina_Bool fork_resetting;

EINA_API void
_eina_lock_debug_abort(int err, const char *fn, const volatile void *ptr)
{
   if (fork_resetting) return;
   fprintf(stderr, "EINA ERROR: '%s' on %s %p\n", strerror(err), fn, ptr);
#ifdef EINA_HAVE_DEBUG_THREADS
   abort();
#endif
}

EINA_API void
_eina_lock_debug_deadlock(const char *fn, const volatile void *ptr)
{
   fprintf(stderr, "EINA ERROR: DEADLOCK on %s %p\n", fn, ptr);
#ifdef EINA_HAVE_DEBUG_THREADS
   abort();
#endif
}

EINA_API void
eina_lock_debug(const Eina_Lock *mutex)
{
#ifdef EINA_HAVE_DEBUG_THREADS
   fprintf(stderr, "EINA ERROR: DEADLOCK on lock %p, locked: %i, by %ti\n",
           mutex, (int)mutex->locked, (ptrdiff_t)mutex->lock_thread_id);
   backtrace_symbols_fd((void **)mutex->lock_bt, mutex->lock_bt_num, 1);
#else
   fprintf(stderr, "EINA ERROR: DEADLOCK on lock %p\n", mutex);
#endif
}

EINA_API Eina_Bool
_eina_lock_new(Eina_Lock *mutex, Eina_Bool recursive)
{
   Eina_Bool ret = _eina_lock_new(mutex, EINA_FALSE);
#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->recursive = EINA_FALSE;
   mutex->lock_thread_id = 0;
   mutex->lock_bt_num = 0;
   mutex->locked = 0;
#endif
   return ret;
}

EAPI Eina_Bool
eina_lock_recursive_new(Eina_Lock *mutex)
{
   Eina_Bool ret = _eina_lock_new(mutex, EINA_TRUE);
#ifdef EINA_HAVE_DEBUG_THREADS
   mutex->recursive = EINA_TRUE;
   mutex->lock_thread_id = 0;
   mutex->lock_bt_num = 0;
   mutex->locked = 0;
#endif
   return ret;
}

EINA_API void
_eina_lock_free(Eina_Lock *mutex)
{
   _eina_lock_free(mutex);
}

EAPI Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
   return _eina_lock_take_try(mutex);
}

EAPI Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex)
{
   return _eina_lock_take(mutex);
}

EAPI Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
   return _eina_lock_release(mutex);
}

EINA_API Eina_Bool
_eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   return _eina_condition_new(cond, mutex);
}

EAPI void
eina_condition_free(Eina_Condition *cond)
{
   _eina_condition_free(cond);
}

EAPI Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   return _eina_condition_wait(cond);
}

EAPI Eina_Bool
eina_condition_timedwait(Eina_Condition *cond, double t)
{
   return _eina_condition_timedwait(cond, t);
}

EAPI Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
   return _eina_condition_broadcast(cond);
}

EAPI Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
   return _eina_condition_signal(cond);
}

EAPI Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex)
{
   return _eina_rwlock_new(mutex);
}

EAPI void
eina_rwlock_free(Eina_RWLock *mutex)
{
   _eina_rwlock_free(mutex);
}

EAPI Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex)
{
   return _eina_rwlock_take_read(mutex);
}

EAPI Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex)
{
   return _eina_rwlock_take_write(mutex);
}

EAPI Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex)
{
   return _eina_rwlock_release(mutex);
}

EAPI Eina_Bool
eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb)
{
   return _eina_tls_cb_new(key, delete_cb);
}

EAPI Eina_Bool
eina_tls_new(Eina_TLS *key)
{
   return _eina_tls_cb_new(key, NULL);
}

EAPI void
eina_tls_free(Eina_TLS key)
{
   _eina_tls_free(key);
}

EAPI void *
eina_tls_get(Eina_TLS key)
{
   return _eina_tls_get(key);
}

EAPI Eina_Bool
eina_tls_set(Eina_TLS key, const void *data)
{
   return _eina_tls_set(key, data);
}

EAPI Eina_Bool
eina_barrier_new(Eina_Barrier *barrier, int needed)
{
   return _eina_barrier_new(barrier, needed);
}

EINA_API Eina_Bool
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

EINA_API void
eina_barrier_free(Eina_Barrier *barrier)
{
   _eina_barrier_free(barrier);
}

EAPI Eina_Bool
eina_barrier_wait(Eina_Barrier *barrier)
{
   return _eina_barrier_wait(barrier);
}

EAPI Eina_Bool
eina_spinlock_new(Eina_Spinlock *spinlock)
{
   return _eina_spinlock_new(spinlock);
}

EAPI void
eina_spinlock_free(Eina_Spinlock *spinlock)
{
   _eina_spinlock_free(spinlock);
}

EAPI Eina_Lock_Result
eina_spinlock_take_try(Eina_Spinlock *spinlock)
{
   return _eina_spinlock_take_try(spinlock);
}

EAPI Eina_Lock_Result
eina_spinlock_take(Eina_Spinlock *spinlock)
{
   return _eina_spinlock_take(spinlock);
}

EAPI Eina_Lock_Result
eina_spinlock_release(Eina_Spinlock *spinlock)
{
   return _eina_spinlock_release(spinlock);
}

EINA_API Eina_Bool
_eina_spinlock_new(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_POSIX_SPINLOCK)
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

EINA_API void
_eina_spinlock_free(Eina_Spinlock *spinlock)
{
#if defined(EINA_HAVE_POSIX_SPINLOCK)
   int ok = pthread_spin_destroy(spinlock);
   if (ok != 0) EINA_LOCK_ABORT_DEBUG(ok, spin_destroy, spinlock);
#elif defined(EINA_HAVE_OSX_SPINLOCK)
   /* Not applicable */
   (void) spinlock;
#else
   eina_lock_free(spinlock);
#endif
}

EINA_API Eina_Bool
eina_semaphore_new(Eina_Semaphore *sem, int count_init)
{
   return _eina_semaphore_new(sem, count_init);
}

EAPI Eina_Bool
eina_semaphore_free(Eina_Semaphore *sem)
{
   return _eina_semaphore_free(sem);
}

EAPI Eina_Bool
eina_semaphore_lock(Eina_Semaphore *sem)
{
   return _eina_semaphore_lock(sem);
}

EAPI Eina_Bool
eina_semaphore_release(Eina_Semaphore *sem, int count_release EINA_UNUSED)
{
   return _eina_semaphore_release(sem, count_release);
}

