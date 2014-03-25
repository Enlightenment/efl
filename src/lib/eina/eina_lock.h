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
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Lock_Group Lock
 * 
 * The Eina lock libraries provide thread locking and sychronization capabilities
 * similar to POISIX threads (pthreads), but it takes care of the platform specific  
 * details so you don't have to.
 * 
 * If you know how @c pthreads work, this library will look familiar to you.
 * If you are not familiar with @c pthreads, a good overview is available 
 * <a href="https://computing.llnl.gov/tutorials/pthreads/">here</a> 
 * 
 * The Eina lock functions are grouped into several categories to handle different 
 * thread locking and sychronization methods:
 * @li eina_lock_* - Functions that implement locking.
 * @li eina_condition_* - Functions that implement condition variables.
 * @li eina_rwlock_* - Functions that implement read/write locks.
 * @li eina_tls_* - Functions that implement thread level storage.
 * @li eina_semaphore_* - Functions that implement semaphores.
 * @li eina_barrier_* - Functions that implement barriers.
 * @li eina_spinlock_* - Functions that implement spinlocks if they are available
 * on the platform.  If they are not available, these functuions degrade to plain locks.
 * 
 * 
 *
 * @{
 */

/** 
* @typedef Eina_Barrier
* An opaque type for working with barrier locks.
*   
*/
/** 
* @typedef Eina_Lock
* An opaque type for working with locks.
*   
*/
/** 
* @typedef Eina_Condition
* An opaque type that represents a condition variable.
*   
*/
/** 
* @typedef Eina_RWLock
* An opaque type for working with read/write locks.
*   
*/
/** 
* @typedef Eina_TLS
* An opaque type for working with thread level storage.
*   
*/
/** 
* @typedef Eina_Semaphore
* An opaque type for working with semaphores.
*   
*/
/** 
* @typedef Eina_Spinlock
* An opaque type for working with spinlocks.
*   
*/

/**
 * @typedef Eina_Lock_Result
 * Return codes for lock operations.
 */
typedef enum
{
   EINA_LOCK_FAIL     = EINA_FALSE, /**< Indicates that the lock operation failed. */
   EINA_LOCK_SUCCEED  = EINA_TRUE,  /**< Indicates that the lock operation succeeded. */
   EINA_LOCK_DEADLOCK               /**< Indicates that the lock is deadlocked. */
} Eina_Lock_Result;

/** Type definition for deallocation callbacks for thread level sotrage data.  A simple function pointer. */
typedef void (*Eina_TLS_Delete_Cb)(void *ptr);

#ifdef _WIN32_WCE
# include "eina_inline_lock_wince.x"
#elif defined(_WIN32)
# include "eina_inline_lock_win32.x"
#else
# include "eina_inline_lock_posix.x"
#endif

/** A type definition for warning that a function was called from somewhere other than the EFL main loop. */
EAPI extern Eina_Error EINA_ERROR_NOT_MAIN_LOOP;

/** 
 * @brief Initializes a new Eina_Lock
 * 
 * This function initializes an Eina_Lock with appropriate values.  
 * These values are platform dependent as is the structure of the Eina_Lock 
 * itself. 
 *
 * @param mutex The Eina_Lock structure to be initialized.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_lock_new(Eina_Lock *mutex);

/** 
 * @brief Deallocates an Eina_Lock
 * 
 * This function deallocates an Eina_Lock and does any platform dependent 
 * cleanup that is required. 
 *
 * @param mutex The Eina_Lock structure to be deallocated. 
 * 
 */
static inline void eina_lock_free(Eina_Lock *mutex);

/** 
 * @brief Attempts to take a lock.
 * 
 * This function attempts to gain a lock on the indicated Eina_Lock.  If the 
 * underlying Eina_Lock is locked already, this call will block until 
 * the lock is released.  This is appropriate in many cases, but consider using
 * #eina_lock_take_try if you don't need to block.
 * 
 * @param mutex The Eina_Lock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If the operation fails because 
 * a deadlock condition exists, it will return EINA_LOCK_DEADLOCK.  If some other 
 * condition causes the take to fail, EINA_LOCK_FAIL is returned. 
 * 
 */
static inline Eina_Lock_Result eina_lock_take(Eina_Lock *mutex);

/** 
 * @brief Attempts to take a lock.
 * 
 * This function attempts to gain a lock on the indicated Eina_Lock.  Identical 
 * to #eina_lock_take, but returns immediately if the lock is already taken.
 *
 * @param mutex The Eina_Lock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If the operation fails because 
 * a deadlock condition exists, it will return EINA_LOCK_DEADLOCK.  If some other 
 * condition causes the take to fail, EINA_LOCK_FAIL is returned. 
 * 
 */
static inline Eina_Lock_Result eina_lock_take_try(Eina_Lock *mutex);

/** 
 * @brief Releases a lock.
 * 
 * This function will release the lock on the indicated Eina_Lock.  If successful,
 * and EINA_HAVE_DEBUG_THREADS is defined, @p mutex is updated and information
 * about the locking process is removed (e.g. thread number and backtrace for POSIX).
 *
 * @param mutex The Eina_Lock to release.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If it fails, EINA_LOCK_FAIL is 
 * returned. 
 * 
 */
static inline Eina_Lock_Result eina_lock_release(Eina_Lock *mutex);

/** 
 * @brief Print debug information about a lock.
 * 
 * This function will print debug information for @p mutex.  The information is 
 * platform dependant.  On POSIX systems it will print the address of @p mutex, 
 * lock state, thread number and a backtrace.
 *
 * @param mutex The Eina_Lock to print debug info for.
 *
 * @note If EINA_HAVE_DEBUG_THREADS is not defined, this function does nothing.
 * @note This function is implemented on Win32 or WinCE, but it will
 * not produce any output, regardless of EINA_HAVE_DEBUG_THREADS being set.
 */ 
static inline void eina_lock_debug(const Eina_Lock *mutex);

/** 
 * @brief Initializes a new condition variable
 * 
 * This function initializes an Eina_Condition structure and associates it with
 * an existing lock.  
 * 
 * Condition variables are used to coordinate actions between threads.  See 
 * <a href="https://computing.llnl.gov/tutorials/pthreads/#ConVarOverview"> Condition Varable Overview </a>
 * for an introduction to condition variables and their use.
 *
 * @param cond The condition variable to create.
 * @param mutex The _Eina_Lock structure that controls access to this condition variable.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);

/** 
 * @brief Deallocates a condition variable
 * 
 * This function deallocates a condition variable and does any platform dependent 
 * cleanup that is required. 
 *
 * @param cond The condition variable to be deallocated. 
 * 
 */
static inline void eina_condition_free(Eina_Condition *cond);

/** 
 * @brief Causes a thread to wait until signaled by the condition.
 * 
 * This function makes a thread block until a signal is sent to it via @p cond. 
 *
 * @param cond The Eina_Condition upon which the thread will wait. 
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 */
static inline Eina_Bool eina_condition_wait(Eina_Condition *cond);

/** 
 * @brief Causes a thread to wait until signaled by the condition or a 
 * timeout is reached.
 * 
 * This function makes a thread block until either a signal is sent to it via 
 * @p cond or @p t seconds have passed. 
 *
 * @param cond The Eina_Condition upon which the thread will wait. 
 * @param t The maximum amount of time to wait, in seconds.
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */
static inline Eina_Bool eina_condition_timedwait(Eina_Condition *cond, double t);

/** 
 * @brief Signal all threads waiting for a condition.
 * 
 * This function sends a signal to all the threads waiting on the condition @p cond.
 * If you know for sure that there is only one thread waiting, use  eina_condition_signal
 * instead to gain a little optimization.
 *
 * @param cond The Eina_Condition that will signal all its waiting threads. 
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */
static inline Eina_Bool eina_condition_broadcast(Eina_Condition *cond);

/** 
 * @brief Signal a thread waiting for a condition.
 * 
 * This function sends a signal to a thread waiting on the condition @p cond.
 * If you do not know for sure that there is only one thread waiting, use  
 * eina_condition_broadcast instead.
 * 
 * If there is more than one thread waiting on this condition, one of them will 
 * be signalled, but which one is undefined.
 *
 * @param cond The Eina_Condition that will signal its waiting thread. 
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */
static inline Eina_Bool eina_condition_signal(Eina_Condition *cond);


/** 
 * @brief Initializes a new Eina_RWLock
 * 
 * This function initializes an Eina_RWLock with appropriate values.  
 * These values are platform dependent as is the structure of the Eina_RWLock 
 * itself. 
 *
 * @param mutex The Eina_RWLock to be initialized.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_rwlock_new(Eina_RWLock *mutex);

/** 
 * @brief Deallocates an Eina_RWLock
 * 
 * This function deallocates an Eina_RWLock and does any platform dependent 
 * cleanup that is required. 
 *
 * @param mutex The Eina_RWLock structure to be deallocated. 
 * 
 */
static inline void eina_rwlock_free(Eina_RWLock *mutex);

/** 
 * @brief Attempts to take a read lock.
 * 
 * This function attempts to gain a read lock on the indicated Eina_RWLock.  If 
 * the Eina_RWLock is write locked, this call will block until 
 * the lock is released.
 * 
 * @param mutex The Eina_RWLock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success, EINA_LOCK_FAIL on failure.  
 * @note This function will never return EINA_LOCK_DEADLOCK. 
 * 
 */
static inline Eina_Lock_Result eina_rwlock_take_read(Eina_RWLock *mutex);

/** 
 * @brief Attempts to take a write lock.
 * 
 * This function attempts to gain a write lock on the indicated Eina_RWLock.  If 
 * the Eina_RWLock is locked for reading or writing, this call  
 * will block until the lock is released.
 * 
 * @param mutex The Eina_RWLock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success, EINA_LOCK_FAIL on failure.  
 * @note This function will never return EINA_LOCK_DEADLOCK. 
 * 
 */
static inline Eina_Lock_Result eina_rwlock_take_write(Eina_RWLock *mutex);

/** 
 * @brief Releases a lock.
 * 
 * This function will release the lock on the indicated Eina_RWLock.  
 *
 * @param mutex The Eina_RWLock to release.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If it fails, EINA_LOCK_FAIL is 
 * returned. 
 * 
 */
static inline Eina_Lock_Result eina_rwlock_release(Eina_RWLock *mutex);

/** 
 * @brief Initializes a new Eina_TLS, or thread level storage, to store thread 
 * specific data.
 * 
 * This function initializes an Eina_TLS with @p key but does not set a 
 * callback to deallocate @p key when the thread exits. The implementation 
 * is platform dependent as is the structure of the Eina_TLS itself. 
 *
 * @param key The Eina_TLS to be initialized.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_tls_new(Eina_TLS *key);

/** 
 * @brief Initializes a new Eina_TLS, or thread level storage, to store thread 
 * specific data.
 * 
 * This function initializes an Eina_TLS with @p key and sets a 
 * callback to deallocate @p key when the thread exits. The implementation 
 * is platform dependent as is the structure of the Eina_TLS itself. 
 *
 * @param key The Eina_TLS to be initialized.
 * @param delete_cb A pointer to a function that will deallocate @p key.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb);

/** 
 * @brief Frees an allocated Eina_TLS.
 * 
 * This function frees the Eina_TLS @p key. The implementation 
 * is platform dependent. 
 *
 * @param key The Eina_TLS to be freed.
 * 
 */
static inline void eina_tls_free(Eina_TLS key);

/** 
 * @brief Gets the value in Eina_TLS for this thread.
 * 
 * This function gets a pointer to the data associated with Eina_TLS @p key for 
 * this thread. The implementation is platform dependent. 
 *
 * @param key The Eina_TLS to be retrieved.
 *
 * @return A pointer to the data associated with @p key. 
 * 
 */
static inline void *eina_tls_get(Eina_TLS key);

/** 
 * @brief Sets the value in Eina_TLS for this thread.
 * 
 * This function sets the value associated with @p key to the pointer to the data 
 * @p data. The implementation is platform dependent. 
 *
 * @param key The Eina_TLS to be set.
 * @param data A pointer to the data to be stored.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_tls_set(Eina_TLS key, const void *data);


/** 
 * @brief Initializes a new Eina_Semaphore
 * 
 * This function initializes an unnamed Eina_Semaphore with appropriate values.  
 * These values are platform dependent. 
 * 
 * 
 * Be aware that the behavior of the parameter @p count_init differs by 
 * platform.  
 * 
 * @li POSIX:  Indicates whether this semaphore can be shared by between processes. Greater than 0 == shared. 
 * @li Win32:  Indicates the initial count of threads waiting on this semaphore.
 * 
 * @note Semaphores are not avialable on the WinCE platform.
 *
 * @param sem The Eina_Semaphore to be initialized.
 * @param count_init Behavior is platofrm specific.  See above.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_semaphore_new(Eina_Semaphore *sem, int count_init);

/** 
 * @brief Frees an allocated Eina_Semaphore.
 * 
 * This function frees the Eina_Semaphore @p sem. The implementation 
 * is platform dependent. 
 *
 * @param sem The Eina_Semaphore to be freed.
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */ 
static inline Eina_Bool eina_semaphore_free(Eina_Semaphore *sem);

/** 
 * @brief Gets a lock on an Eina_Semaphore.
 * 
 * This function locks the Eina_Semaphore @p sem. The implementation 
 * is platform dependent. 
 *
 * @param sem The Eina_Semaphore to lock.
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */ 
static inline Eina_Bool eina_semaphore_lock(Eina_Semaphore *sem);

/** 
 * @brief Releases a lock on an Eina_Semaphore.
 * 
 * This function releases a lock on the Eina_Semaphore @p sem. The implementation 
 * is platform dependent. 
 *
 * @param sem The Eina_Semaphore to release.
 * @param count_release Not used.
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */ 
static inline Eina_Bool eina_semaphore_release(Eina_Semaphore *sem, int count_release);


/** 
 * @brief Initializes a new Eina_Barrier
 * 
 * This function initializes a new Eina_Barrier.  It will set the @c needed flag
 * to the value of @p needed, set the barrier's @c count member to 0 and will 
 * create new Eina_Lock and Eina_Condition objects for the barrier.
 *
 * @param barrier The Eina_Barrier to be initialized.
 * @param needed The number of thread waits that will cause this barrier to be reset.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_barrier_new(Eina_Barrier *barrier, int needed);

/** 
 * @brief Frees an allocated Eina_Barrier.
 * 
 * This function frees the Eina_Barrier @p barrier. 
 *
 * @param barrier The Eina_Barrier to be freed.
 * 
 */ 
static inline void eina_barrier_free(Eina_Barrier *barrier);

/** 
 * @brief Increments the count of threads that are waiting on @p barrier.
 * 
 * When the count of threads reaches the @c needed value for the barrier, all 
 * waiting threads will be notified via eina_condition_broadcast.
 *
 * @param barrier The Eina_Barrier to be incremented.
 * 
 * @return EINA_TRUE on success, else EINA_FALSE.
 * 
 */ 
static inline Eina_Bool eina_barrier_wait(Eina_Barrier *barrier);


/** 
 * @brief Initializes a new Eina_Spinlock
 * 
 * This function initializes a new Eina_Spinlock, if spinlocks are available. If
 * spinlocks are not avialable, it will create a new Eina_Lock.
 *
 *@note Spinlocks are only implemented on the POSIX platform and are only available 
 *if EINA_HAVE_POSIX_SPINLOCK is defined. All other platforms will get a new Eina_Lock.
 *
 * @param spinlock The Eina_Spinlock to be initialized.
 *
 * @return EINA_TRUE on success, else EINA_FALSE. 
 * 
 */
static inline Eina_Bool eina_spinlock_new(Eina_Spinlock *spinlock);

/** 
 * @brief Attempts to take a spinlock.
 * 
 * This function attempts to gain a lock on the indicated Eina_Spinlock.  If the 
 * underlying Eina_Spinlock is locked already, this call will block until 
 * the lock is released.  This is appropriate in many cases, but consider using
 * #eina_spinlock_take_try if you don't need to block.
 * 
 * @param spinlock The Eina_Spinlock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If the operation fails because 
 * a deadlock condition exists, it will return EINA_LOCK_DEADLOCK.  If some other 
 * condition causes the take to fail, EINA_LOCK_FAIL is returned. 
 * 
 */
static inline Eina_Lock_Result eina_spinlock_take(Eina_Spinlock *spinlock);

/** 
 * @brief Attempts to take a spinlock.
 * 
 * This function attempts to gain a lock on the indicated Eina_Spinlock.  Identical 
 * to #eina_lock_take, but returns immediately if the lock is already taken.
 *
 * @param spinlock The Eina_Spinlock to take.
 *
 * @return Returns EINA_LOCK_SUCCEED on success.  If the operation fails because 
 * a deadlock condition exists, it will return EINA_LOCK_DEADLOCK.  If some other 
 * condition causes the take to fail, EINA_LOCK_FAIL is returned. 
 * 
 */
static inline Eina_Lock_Result eina_spinlock_take_try(Eina_Spinlock *spinlock);

/** 
 * @brief Releases a spinlock.
 * 
 * This function will release the lock on the indicated Eina_Spinlock.  If successful,
 * and EINA_HAVE_DEBUG_THREADS is defined, @p mutex is updated and information
 * about the locking process is removed (e.g. thread number and backtrace for POSIX).
 *
 * @param spinlock The Eina_Spinlock to release.
 *
 * @return Returns EINA_LOCK_SUCCEED on success, else EINA_LOCK_FAIL. 
 * 
 */
static inline Eina_Lock_Result eina_spinlock_release(Eina_Spinlock *spinlock);

/** 
 * @brief Deallocates an Eina_Spinlock
 * 
 * This function deallocates an Eina_Spinlock and does any platform dependent 
 * cleanup that is required. 
 *
 * @param spinlock The Eina_Spinlock to be deallocated. 
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
 * @brief The macro doesn't do anything unless EINA_HAVE_DEBUG_THREADS is defined.
 * @param val The value to be returned.
 */
# define EINA_MAIN_LOOP_CHECK_RETURN_VAL(val)
/**
 * @def EINA_MAIN_LOOP_CHECK_RETURN
 * @brief The macro doesn't do anything unless EINA_HAVE_DEBUG_THREADS is defined.
 */
# define EINA_MAIN_LOOP_CHECK_RETURN
#endif

/**
 * @}
 */

/**
 * @}
 */

#endif
