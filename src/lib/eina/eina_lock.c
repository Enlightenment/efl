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

EAPI Eina_Lock_Result
_eina_spinlock_macos_take(Eina_Spinlock *spinlock)
{
   os_unfair_lock_lock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}

EAPI Eina_Lock_Result
_eina_spinlock_macos_take_try(Eina_Spinlock *spinlock)
{
   return (os_unfair_lock_trylock(SPINLOCK_GET(spinlock)) == true)
      ? EINA_LOCK_SUCCEED
      : EINA_LOCK_FAIL;
}

EAPI Eina_Lock_Result
_eina_spinlock_macos_release(Eina_Spinlock *spinlock)
{
   os_unfair_lock_unlock(SPINLOCK_GET(spinlock));
   return EINA_LOCK_SUCCEED;
}
#endif /* EINA_HAVE_OSX_SPINLOCK */


EAPI void
_eina_lock_debug_abort(int err, const char *fn, const volatile void *ptr)
{
   fprintf(stderr, "EINA ERROR: '%s' on %s %p\n", strerror(err), fn, ptr);
#ifdef EINA_HAVE_DEBUG_THREADS
   abort();
#endif
}

EAPI void
_eina_lock_debug_deadlock(const char *fn, const volatile void *ptr)
{
   fprintf(stderr, "EINA ERROR: DEADLOCK on %s %p\n", fn, ptr);
#ifdef EINA_HAVE_DEBUG_THREADS
   abort();
#endif
}

EAPI void
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

EAPI Eina_Bool
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

EAPI void
_eina_lock_free(Eina_Lock *mutex)
{
   int ok;

   ok = pthread_mutex_destroy(&(mutex->mutex));
   if (ok != 0) EINA_LOCK_ABORT_DEBUG(ok, mutex_destroy, mutex);
}

EAPI Eina_Bool
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

EAPI void
_eina_condition_free(Eina_Condition *cond)
{
   pthread_cond_destroy(&(cond->condition));
}

EAPI Eina_Bool
_eina_rwlock_new(Eina_RWLock *mutex)
{
   int ok;

   ok = pthread_rwlock_init(&(mutex->mutex), NULL);
   if (ok == 0) return EINA_TRUE;
   else if ((ok == EAGAIN) || (ok == ENOMEM)) return EINA_FALSE;
   else EINA_LOCK_ABORT_DEBUG(ok, rwlock_init, mutex);
   return EINA_FALSE;
}

EAPI void
_eina_rwlock_free(Eina_RWLock *mutex)
{
   pthread_rwlock_destroy(&(mutex->mutex));
}

EAPI Eina_Bool
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

EAPI void
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

EAPI Eina_Bool
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

EAPI void
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

EAPI Eina_Bool
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

EAPI Eina_Bool
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
