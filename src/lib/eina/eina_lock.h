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

#ifndef EINA_LOCK_H_
#define EINA_LOCK_H_

#include "eina_config.h"
#include "eina_types.h"
#include "eina_error.h"
#include "eina_main.h"

/**
 * @defgroup Eina_Lock_Group Lock
 * @ingroup Eina_Tools_Group
 * @brief This group provides thread locking and synchronization capabilities.
 *
 * Similar to POSIX threads (pthreads), but it takes care of the platform specific
 * details so you don't have to.
 *
 * If you know how @c pthreads work, this library will look familiar to you.
 * If you are not familiar with @c pthreads, a good overview is available
 * <a href="https://computing.llnl.gov/tutorials/pthreads/">here</a>
 *
 * The Eina lock functions are grouped into several categories to handle different
 * thread locking and synchronization methods:
 * @li eina_lock_* - Functions that implement locking.
 * @li eina_condition_* - Functions that implement condition variables.
 * @li eina_rwlock_* - Functions that implement read/write locks.
 * @li eina_tls_* - Functions that implement thread level storage.
 * @li eina_semaphore_* - Functions that implement semaphores.
 * @li eina_barrier_* - Functions that implement barriers.
 * @li eina_spinlock_* - Functions that implement spinlocks if they are available
 * on the platform.  If they are not available, these functions degrade to plain locks.
 *
 *
 *
 * @{
 */

/**
* @typedef Eina_Barrier
* @brief An opaque type for working with barrier locks.
*/
/**
* @typedef Eina_Lock
* @brief An opaque type for working with locks.
*/
/**
* @typedef Eina_Condition
* @brief An opaque type that represents a condition variable.
*/
/**
* @typedef Eina_RWLock
* @brief An opaque type for working with read/write locks.
*/
/**
* @typedef Eina_TLS
* @brief An opaque type for working with thread level storage.
*/
/**
* @typedef Eina_Semaphore
* @brief An opaque type for working with semaphores.
*/
/**
* @typedef Eina_Spinlock
* @brief An opaque type for working with spinlocks.
*/

/**
 * @typedef Eina_Lock_Result
 * @brief Return codes for lock operations.
 */
typedef enum
{
   EINA_LOCK_FAIL     = EINA_FALSE, /**< Indicates that the lock operation failed. */
   EINA_LOCK_SUCCEED  = EINA_TRUE,  /**< Indicates that the lock operation succeeded. */
   EINA_LOCK_DEADLOCK               /**< Indicates that the lock is deadlocked. */
} Eina_Lock_Result;

/**
 * @brief A callback type for deallocation of thread level storage data.
 */
typedef void (*Eina_TLS_Delete_Cb)(void *ptr);

#include "eina_inline_lock_posix.x"

/**
 * @brief A type definition for warning that a function was called from
 *        somewhere other than the EFL main loop.
 */
EAPI extern Eina_Error EINA_ERROR_NOT_MAIN_LOOP;

/**
 * @brief Initializes a new #Eina_Lock.
 *
 * @param[in] mutex The #Eina_Lock structure to be initialized
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise
 *
 * @details This function initializes an #Eina_Lock with appropriate values.
 *          These values are platform dependent as is the structure of the
 *          #Eina_Lock itself.
 *
 * @see eina_lock_free()
 */
static inline Eina_Bool eina_lock_new(Eina_Lock *mutex);

/**
 * @brief Initializes a new #Eina_Lock that is recursive.
 *
 * @param[in] mutex The #Eina_Lock structure to be initialized
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise
 *
 * @details This function initializes an #Eina_Lock with appropriate values.
 *          These values are platform dependent as is the structure of the
 *          #Eina_Lock itself.
 *
 * @see eina_lock_new()
 * @see eina_lock_free()
 * @since 1.19
 */
static inline Eina_Bool eina_lock_recursive_new(Eina_Lock *mutex);

/**
 * @brief Deallocates an #Eina_Lock.
 * @details This function deallocates an #Eina_Lock allocated by eina_lock_new()
 *          and does any platform dependent cleanup that is required.
 *
 * @param[in] mutex The #Eina_Lock structure to be deallocated
 *
 * @see eina_lock_new()
 */
static inline void eina_lock_free(Eina_Lock *mutex);

/**
 * @brief Attempts to take a lock.
 * @details This function attempts to gain a lock on the indicated #Eina_Lock. If the
 *          underlying #Eina_Lock is locked already, this call can be blocked until
 *          the lock is released. This is appropriate in many cases, but consider using
 *          eina_lock_take_try() if you don't need to block.
 *
 * @param[in] mutex The #Eina_Lock to take
 *
 * @return Returns #EINA_LOCK_SUCCEED on success. If the operation fails because
 *         a deadlock condition exists, it returns #EINA_LOCK_DEADLOCK. If some other
 *         condition causes the take to fail, #EINA_LOCK_FAIL is returned.
 *
 * @see eina_lock_take_try()
 * @see eina_lock_release()
 */
static inline Eina_Lock_Result eina_lock_take(Eina_Lock *mutex);

/**
 * @brief Attempts to take a lock if possible.
 * @details This function attempts to gain a lock on the indicated #Eina_Lock.
            Identical eina_lock_take(), but returns immediately if the lock is already taken.
 *
 * @param[in] mutex The #Eina_Lock to take
 *
 * @return Returns #EINA_LOCK_SUCCEED on success. If the operation fails because
 *         a deadlock condition exists, it returns #EINA_LOCK_DEADLOCK. If some other
 *         condition causes the take to fail, #EINA_LOCK_FAIL is returned.
 *
 * @see eina_lock_take()
 * @see eina_lock_release()
 */
static inline Eina_Lock_Result eina_lock_take_try(Eina_Lock *mutex);

/**
 * @brief Releases a lock.
 * @details This function releases the lock on the indicated #Eina_Lock. If successful,
 *          and @c EINA_HAVE_DEBUG_THREADS is defined, @p mutex is updated and information
 *          about the locking process is removed (e.g. thread number and backtrace for POSIX).
 *
 * @param[in] mutex The #Eina_Lock to release
 *
 * @return Returns #EINA_LOCK_SUCCEED on success,  If it fails, #EINA_LOCK_FAIL is
 *         returned.
 *
 * @see eina_lock_take()
 * @see eina_lock_take_try()
 */
static inline Eina_Lock_Result eina_lock_release(Eina_Lock *mutex);

/**
 * @brief Prints debug information about a lock.
 * @details This function prints debug information for @p mutex.  The information is
 *          platform dependent.  On POSIX systems it prints the address of @p mutex,
 *          lock state, thread number and a backtrace.
 *
 * @param[in] mutex The #Eina_Lock to print debug info for.
 *
 * @note If @c EINA_HAVE_DEBUG_THREADS is not defined, this function does nothing.
 *
 * @since 1.19
 */
EAPI void eina_lock_debug(const Eina_Lock *mutex);

/**
 * @brief Initializes a new condition variable.
 * @details This function initializes an #Eina_Condition structure and associates it with
 *          an existing lock.
 *
 * @param[in] cond The condition variable to create
 * @param[in] mutex The #Eina_Lock structure that controls access to this condition variable
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Condition variables are used to coordinate actions between threads.  See
 * <a href="https://computing.llnl.gov/tutorials/pthreads/#ConVarOverview"> Condition Variable Overview </a>
 * for an introduction to condition variables and their use.
 *
 * @see eina_condition_free()
 */
static inline Eina_Bool eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);

/**
 * @brief Deallocates a condition variable.
 * @details This function deallocates a condition variable and does any platform dependent
 *          cleanup that is required.
 *
 * @param[in] cond The condition variable to be deallocated.
 *
 * @see eina_condition_new()
 */
static inline void eina_condition_free(Eina_Condition *cond);

/**
 * @brief Causes a thread to wait until signaled by the condition.
 * @details This function makes a thread block until a signal is sent to it via @p cond.
 *
 * @param[in] cond The #Eina_Condition upon which the thread waits.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_condition_timedwait()
 */
static inline Eina_Bool eina_condition_wait(Eina_Condition *cond);

/**
 * @brief Causes a thread to wait until signaled by the condition or a
 *        timeout is reached.
 * @details This function makes a thread block until either a signal is sent to it via
 *          @p cond or @p t seconds have passed.
 *
 * @param[in] cond The #Eina_Condition upon which the thread waits.
 * @param[in] t The maximum amount of time to wait, in seconds.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise. If the operation
 *         timed out, eina error will be set to #ETIMEDOUT.
 *
 * @see eina_condition_wait()
 */
static inline Eina_Bool eina_condition_timedwait(Eina_Condition *cond, double t);

/**
 * @brief Signals all threads waiting for a condition.
 * @details This function sends a signal to all the threads waiting on the condition @p cond.
 *          If you know for sure that there is only one thread waiting, use eina_condition_signal()
 *          instead to gain a little optimization.
 *
 * @param[in] cond The #Eina_Condition that signals all its waiting threads.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_condition_signal()
 */
static inline Eina_Bool eina_condition_broadcast(Eina_Condition *cond);

/**
 * @brief Signals a thread waiting for a condition.
 * @details This function sends a signal to a thread waiting on the condition @p cond.
 *          If you do not know for sure that there is only one thread waiting, use
 *          eina_condition_broadcast() instead.
 *
 * @param[in] cond The #Eina_Condition that signals its waiting thread.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @note If there is more than one thread waiting on this condition, one of them is
 *       signaled, but which one is undefined.
 *
 * @see eina_condition_broadcast()
 */
static inline Eina_Bool eina_condition_signal(Eina_Condition *cond);


/**
 * @brief Initializes a new #Eina_RWLock.
 * @details This function initializes an #Eina_RWLock with appropriate values.
 *          These values are platform dependent as is the structure of the #Eina_RWLock
 *          itself.
 *
 * @param[in] mutex The #Eina_RWLock to be initialized.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_rwlock_free()
 */
static inline Eina_Bool eina_rwlock_new(Eina_RWLock *mutex);

/**
 * @brief Deallocates an #Eina_RWLock.
 * @details This function deallocates an #Eina_RWLock and does any platform dependent
 *          cleanup that is required.
 *
 * @param[in] mutex The #Eina_RWLock structure to be deallocated.
 *
 * @see eina_rwlock_new()
 */
static inline void eina_rwlock_free(Eina_RWLock *mutex);

/**
 * @brief Attempts to take a read lock.
 * @details This function attempts to gain a read lock on the indicated #Eina_RWLock. If
 *          the #Eina_RWLock is write locked, this call can be blocked until
 *          the lock is released.
 *
 * @param[in] mutex The #Eina_RWLock to take.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success, #EINA_LOCK_FAIL on failure.
 *
 * @note This function never returns #EINA_LOCK_DEADLOCK.
 *
 * @see eina_rwlock_release()
 */
static inline Eina_Lock_Result eina_rwlock_take_read(Eina_RWLock *mutex);

/**
 * @brief Attempts to take a write lock.
 * @details This function attempts to gain a write lock on the indicated #Eina_RWLock. If
 *          the #Eina_RWLock is locked for reading or writing, this call can be
 *          blocked until the lock is released.
 *
 * @param[in] mutex The #Eina_RWLock to take.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success, #EINA_LOCK_FAIL on failure.
 *
 * @note This function never returns #EINA_LOCK_DEADLOCK.
 *
 * @see eina_rwlock_release()
 */
static inline Eina_Lock_Result eina_rwlock_take_write(Eina_RWLock *mutex);

/**
 * @brief Releases a lock.
 * @details This function releases the lock on the indicated #Eina_RWLock.
 *
 * @param[in] mutex The #Eina_RWLock to release.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success.  If it fails, #EINA_LOCK_FAIL is
 *         returned.
 *
 * @see eina_rwlock_take_read()
 * @see eina_rwlock_take_write()
 */
static inline Eina_Lock_Result eina_rwlock_release(Eina_RWLock *mutex);

/**
 * @brief Initializes a new #Eina_TLS, or thread level storage, to store thread
 *        specific data.
 * @details This function initializes an #Eina_TLS with @p key but does not set a
 *          callback to deallocate @p key when the thread exits. The implementation
 *          is platform dependent as is the structure of the #Eina_TLS itself.
 *
 * @param[in] key The #Eina_TLS to be initialized.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @note use eina_tls_cb_new() instead to set a callback for deallocating @p key.
 *
 * @see eina_tls_cb_new()
 * @see eina_tls_free()
 */
static inline Eina_Bool eina_tls_new(Eina_TLS *key);

/**
 * @brief Initializes a new #Eina_TLS, or thread level storage, to store thread
 *        specific data.
 * @details This function initializes an #Eina_TLS with @p key and sets a
 *          callback to deallocate @p key when the thread exits. The implementation
 *          is platform dependent as is the structure of the #Eina_TLS itself.
 *
 * @param[in] key The #Eina_TLS to be initialized.
 * @param[in] delete_cb A pointer to a function that deallocates @p key.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_tls_new()
 * @see eina_tls_free()
 */
static inline Eina_Bool eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb);

/**
 * @brief Frees an allocated #Eina_TLS.
 * @details This function frees the #Eina_TLS @p key. The implementation
 *          is platform dependent.
 *
 * @param[in] key The #Eina_TLS to be freed.
 *
 * @see eina_tls_new()
 * @see eina_tls_cb_new()
 */
static inline void eina_tls_free(Eina_TLS key);

/**
 * @brief Gets the value in #Eina_TLS for this thread.
 * @details This function gets a pointer to the data associated with #Eina_TLS @p key for
 *          this thread. The implementation is platform dependent.
 *
 * @param[in] key The #Eina_TLS to be retrieved.
 *
 * @return A pointer to the data associated with @p key.
 *
 * @see  eina_tls_set()
 */
static inline void *eina_tls_get(Eina_TLS key);

/**
 * @brief Sets the value in Eina_TLS for this thread.
 * @details This function sets the value associated with @p key to the pointer to the data
 *          @p data. The implementation is platform dependent.
 *
 * @param[in] key The #Eina_TLS to be set.
 * @param[in] data A pointer to the data to be stored.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_tls_get()
 */
static inline Eina_Bool eina_tls_set(Eina_TLS key, const void *data);

/**
 * @brief Initializes a new #Eina_Semaphore.
 * @details This function initializes an unnamed #Eina_Semaphore with appropriate values.
 *          These values are platform dependent.
 *
 * @param[in] sem The #Eina_Semaphore to be initialized.
 * @param[in] count_init Indicates the initial count of threads waiting on this semaphore.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_semaphore_free()
 */
static inline Eina_Bool eina_semaphore_new(Eina_Semaphore *sem, int count_init);

/**
 * @brief Frees an allocated #Eina_Semaphore.
 * @details This function frees the #Eina_Semaphore @p sem. The implementation
 *          is platform dependent.
 *
 * @param[in] sem The #Eina_Semaphore to be freed.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_semaphore_new()
 */
static inline Eina_Bool eina_semaphore_free(Eina_Semaphore *sem);

/**
 * @brief Gets a lock on an #Eina_Semaphore.
 * @details This function locks the #Eina_Semaphore @p sem. The implementation
 *          is platform dependent.
 *
 * @param[in] sem The #Eina_Semaphore to lock.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_semaphore_release()
 */
static inline Eina_Bool eina_semaphore_lock(Eina_Semaphore *sem);

/**
 * @brief Releases a lock on an #Eina_Semaphore.
 * @details This function releases a lock on the #Eina_Semaphore @p sem. The implementation
 *          is platform dependent.
 *
 * @param[in] sem The #Eina_Semaphore to release.
 * @param[in] count_release Not used.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_semaphore_lock()
 */
static inline Eina_Bool eina_semaphore_release(Eina_Semaphore *sem, int count_release);

/**
 * @brief Initializes a new #Eina_Barrier.
 * @details This function initializes a new #Eina_Barrier.  It sets the @c needed flag
 *          to the value of @p needed, sets the barrier's @c count member to 0 and
 *          creates new #Eina_Lock and #Eina_Condition objects for the barrier.
 *
 * @param[in] barrier The #Eina_Barrier to be initialized.
 * @param[in] needed The number of thread waits that causes this barrier to be reset.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eina_barrier_free()
 */
static inline Eina_Bool eina_barrier_new(Eina_Barrier *barrier, int needed);

/**
 * @brief Frees an allocated #Eina_Barrier.
 * @details This function frees the #Eina_Barrier @p barrier.
 *
 * @param[in] barrier The #Eina_Barrier to be freed.
 *
 * @see eina_barrier_new()
 */
static inline void eina_barrier_free(Eina_Barrier *barrier);

/**
 * @brief Increments the count of threads that are waiting on @p barrier.
 * @details When the count of threads reaches the @c needed value for the barrier, all
 *          waiting threads are notified via eina_condition_broadcast().
 * @param[in] barrier The #Eina_Barrier to be incremented.
 *
 * @return #EINA_TRUE on success, else #EINA_FALSE otherwise.
 *
 */
static inline Eina_Bool eina_barrier_wait(Eina_Barrier *barrier);


/**
 * @brief Initializes a new #Eina_Spinlock.
 * @details This function initializes a new #Eina_Spinlock, if spinlocks are available. If
 *          spinlocks are not available, it creates a new #Eina_Lock.
 *
 * @param[in] spinlock The #Eina_Spinlock to be initialized.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @note Spinlocks are only implemented on the POSIX platform and are only available
 *      if #EINA_HAVE_POSIX_SPINLOCK is defined. you get a new #Eina_Lock on All other platforms.
 *
 * @see eina_spinlock_free()
 */
static inline Eina_Bool eina_spinlock_new(Eina_Spinlock *spinlock);

/**
 * @brief Attempts to take a spinlock.
 * @details This function attempts to gain a lock on the indicated #Eina_Spinlock. If the
 *          underlying #Eina_Spinlock is locked already, this call can be blocked until
 *          the lock is released. This is appropriate in many cases, but consider using
 *          eina_spinlock_take_try() if you don't need to block.
 *
 * @param[in] spinlock The #Eina_Spinlock to take.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success.  If the operation fails because
 *         a deadlock condition exists, it returns #EINA_LOCK_DEADLOCK. If some other
 *         condition causes the take to fail, #EINA_LOCK_FAIL is returned.
 *
 * @see eina_spinlock_take_try()
 * @see eina_spinlock_release()
 */
static inline Eina_Lock_Result eina_spinlock_take(Eina_Spinlock *spinlock);

/**
 * @brief Attempts to take a spinlock if possible.
 * @details This function attempts to gain a lock on the indicated #Eina_Spinlock. Identical
 *          to eina_lock_take(), but returns immediately if the lock is already taken.
 *
 * @param[in] spinlock The #Eina_Spinlock to take.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success. If the operation fails because
 *         a deadlock condition exists, it returns #EINA_LOCK_DEADLOCK. If some other
 *         condition causes the take to fail, #EINA_LOCK_FAIL is returned.
 *
 * @see eina_spinlock_take_try()
 * @see eina_spinlock_release()
 */
static inline Eina_Lock_Result eina_spinlock_take_try(Eina_Spinlock *spinlock);

/**
 * @brief Releases a spinlock.
 * @details This function will release the lock on the indicated #Eina_Spinlock. If successful,
 *          and @c EINA_HAVE_DEBUG_THREADS is defined, @p mutex is updated and information
 *          about the locking process is removed (e.g. thread number and backtrace for POSIX).
 *
 * @param[in] spinlock The #Eina_Spinlock to release.
 *
 * @return Returns #EINA_LOCK_SUCCEED on success, #EINA_LOCK_FAIL otherwise.
 *
 * @see eina_spinlock_take()
 * @see eina_spinlock_take_try()

 */
static inline Eina_Lock_Result eina_spinlock_release(Eina_Spinlock *spinlock);

/**
 * @brief Deallocates an #Eina_Spinlock.
 * @details This function deallocates an #Eina_Spinlock and does any platform dependent
 *          cleanup that is required.
 *
 * @param[in] spinlock The #Eina_Spinlock to be deallocated.
 *
 */
static inline void eina_spinlock_free(Eina_Spinlock *spinlock);

#ifdef EINA_HAVE_DEBUG_THREADS
# define EINA_MAIN_LOOP_CHECK_RETURN_VAL(val)				\
  do {									\
    if (EINA_UNLIKELY(!eina_main_loop_is()))				\
      {									\
	EINA_LOG_ERR("You are calling %s from outside"			\
		     "of the main loop threads in %s at line %i",	\
		     __FUNCTION__,					\
		     __FILE__,						\
		     __LINE__);						\
	return val;							\
      }									\
  } while (0)
# define EINA_MAIN_LOOP_CHECK_RETURN					\
  do {									\
    if (EINA_UNLIKELY(!eina_main_loop_is()))				\
      {									\
	EINA_LOG_ERR("You are calling %s from outside"			\
		     "of the main loop threads in %s at line %i",	\
		     __FUNCTION__,					\
		     __FILE__,						\
		     __LINE__);						\
	return ;							\
      }									\
  } while (0)
#else
/**
 * @def EINA_MAIN_LOOP_CHECK_RETURN_VAL
 * @brief Definition for the macro that doesn't do anything unless @c EINA_HAVE_DEBUG_THREADS is defined.
 * @param[in] val The value to be returned
 */
# define EINA_MAIN_LOOP_CHECK_RETURN_VAL(val)
/**
 * @def EINA_MAIN_LOOP_CHECK_RETURN
 * @brief Definition for the macro that doesn't do anything unless @c EINA_HAVE_DEBUG_THREADS is defined.
 */
# define EINA_MAIN_LOOP_CHECK_RETURN
#endif

/**
 * @}
 */

#endif
