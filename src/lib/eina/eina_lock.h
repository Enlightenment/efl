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

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Lock_Group Lock
 *
 * @{
 */

typedef enum
{
   EINA_LOCK_FAIL     = EINA_FALSE,
   EINA_LOCK_SUCCEED  = EINA_TRUE,
   EINA_LOCK_DEADLOCK
} Eina_Lock_Result;

typedef void (*Eina_TLS_Delete_Cb)(void *ptr);

#ifdef _WIN32_WCE
# include "eina_inline_lock_wince.x"
#elif defined(_WIN32)
# include "eina_inline_lock_win32.x"
#else
# include "eina_inline_lock_posix.x"
#endif

EAPI extern Eina_Error EINA_ERROR_NOT_MAIN_LOOP;

/** @relates static Eina_Bool eina_lock_new(_Eina_Lock *mutex) */
static inline Eina_Bool eina_lock_new(Eina_Lock *mutex);
/** @relates static void eina_lock_free(_Eina_Lock *mutex) */
static inline void eina_lock_free(Eina_Lock *mutex);
/** @relates static Eina_Lock_Result eina_lock_take(_Eina_Lock *mutex) */
static inline Eina_Lock_Result eina_lock_take(Eina_Lock *mutex);
/** @relates static Eina_Lock_Result eina_lock_take_try(_Eina_Lock *mutex) */
static inline Eina_Lock_Result eina_lock_take_try(Eina_Lock *mutex);
/** @relates static Eina_Lock_Result eina_lock_release(_Eina_Lock *mutex) */
static inline Eina_Lock_Result eina_lock_release(Eina_Lock *mutex);
/** @relates static void eina_lock_debug(const _Eina_Lock *mutex) */
static inline void eina_lock_debug(const Eina_Lock *mutex);

/** @relates static Eina_Bool eina_condition_new(_Eina_Condition *cond, _Eina_Lock *mutex) */
static inline Eina_Bool eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);
/** @relates static void eina_condition_free(_Eina_Condition *cond) */
static inline void eina_condition_free(Eina_Condition *cond);
/** @relates static Eina_Bool eina_condition_wait(_Eina_Condition *cond) */
static inline Eina_Bool eina_condition_wait(Eina_Condition *cond);
/** @relates static Eina_Bool eina_condition_timedwait(_Eina_Condition *cond, double t) */
static inline Eina_Bool eina_condition_timedwait(Eina_Condition *cond, double t);
/** @relates static Eina_Bool eina_condition_broadcast(_Eina_Condition *cond) */
static inline Eina_Bool eina_condition_broadcast(Eina_Condition *cond);
/** @relates static Eina_Bool eina_condition_signal(_Eina_Condition *cond) */
static inline Eina_Bool eina_condition_signal(Eina_Condition *cond);

/** @relates static Eina_Bool eina_rwlock_new(_Eina_RWLock *mutex) */
static inline Eina_Bool eina_rwlock_new(Eina_RWLock *mutex);
/** @relates static void eina_rwlock_free(_Eina_RWLock *mutex) */
static inline void eina_rwlock_free(Eina_RWLock *mutex);
/** @relates static Eina_Lock_Result eina_rwlock_take_read(_Eina_RWLock *mutex) */
static inline Eina_Lock_Result eina_rwlock_take_read(Eina_RWLock *mutex);
/** @relates static Eina_Lock_Result eina_rwlock_take_write(_Eina_RWLock *mutex) */
static inline Eina_Lock_Result eina_rwlock_take_write(Eina_RWLock *mutex);
/** @relates static Eina_Lock_Result eina_rwlock_release(_Eina_RWLock *mutex) */
static inline Eina_Lock_Result eina_rwlock_release(Eina_RWLock *mutex);

/** @relates static Eina_Bool eina_tls_new(pthread_key_t *key) */
static inline Eina_Bool eina_tls_new(Eina_TLS *key);
/** @relates static Eina_Bool eina_tls_cb_new(pthread_key_t *key, Eina_TLS_Delete_Cb delete_cb) */
static inline Eina_Bool eina_tls_cb_new(Eina_TLS *key, Eina_TLS_Delete_Cb delete_cb);
/** @relates static void eina_tls_free(pthread_key_t key) */
static inline void eina_tls_free(Eina_TLS key);
/** @relates static void eina_tls_get(pthread_key_t key) */
static inline void *eina_tls_get(Eina_TLS key);
/** @relates static Eina_Bool eina_tls_set(pthread_key_t key, const void *data) */
static inline Eina_Bool eina_tls_set(Eina_TLS key, const void *data);
/** @relates static Eina_Bool eina_semaphore_new(sem_t *sem, int count_init) */
static inline Eina_Bool eina_semaphore_new(Eina_Semaphore *sem, int count_init);
/** @relates static Eina_Bool eina_semaphore_free(sem_t *sem) */
static inline Eina_Bool eina_semaphore_free(Eina_Semaphore *sem);
/** @relates static Eina_Bool eina_semaphore_lock(sem_t *sem) */
static inline Eina_Bool eina_semaphore_lock(Eina_Semaphore *sem);
/** @relates static Eina_Bool eina_semaphore_release(sem_t *sem, int count_release) */
static inline Eina_Bool eina_semaphore_release(Eina_Semaphore *sem, int count_release);

/** @relates static Eina_Bool eina_barrier_new(Eina_Barrier *barrier, int needed) @since 1.8 */
static inline Eina_Bool eina_barrier_new(Eina_Barrier *barrier, int needed);
/** @relates static void eina_barrier_free(Eina_Barrier *barrier); @since 1.8 */
static inline void eina_barrier_free(Eina_Barrier *barrier);
/** @relates static Eina_Bool eina_barrier_wait(Eina_Barrier *barrier); @since 1.8 */
static inline Eina_Bool eina_barrier_wait(Eina_Barrier *barrier);

/** @relates static Eina_Bool eina_spinlock_new(Eina_Spinlock *spinlock); @since 1.8 */
static inline Eina_Bool eina_spinlock_new(Eina_Spinlock *spinlock);
/** @relates static Eina_Lock_Result eina_spinlock_take(Eina_Spinlock *spinlock); @since 1.8 */
static inline Eina_Lock_Result eina_spinlock_take(Eina_Spinlock *spinlock);
/** @relates static Eina_Lock_Result eina_spinlock_take(Eina_Spinlock *spinlock); @since 1.8 */
static inline Eina_Lock_Result eina_spinlock_take_try(Eina_Spinlock *spinlock);
/** @relates static Eina_Lock_Result eina_spinlock_release(Eina_Spinlock *spinlock); @since 1.8 */
static inline Eina_Lock_Result eina_spinlock_release(Eina_Spinlock *spinlock);
/** @relates static void eina_spinlock_free(Eina_Spinlock *spinlock); @since 1.8 */
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
