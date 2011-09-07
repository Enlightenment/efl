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

#ifdef EINA_HAVE_THREADS
# ifdef _WIN32_WCE
#  include "eina_inline_lock_wince.x"
# elif defined(_WIN32)
#  include "eina_inline_lock_win32.x"
# else
#  include "eina_inline_lock_posix.x"
# endif
#else
# include "eina_inline_lock_void.x"
#endif

EAPI extern Eina_Error EINA_ERROR_NOT_MAIN_LOOP;

static inline Eina_Bool eina_lock_new(Eina_Lock *mutex);
static inline void eina_lock_free(Eina_Lock *mutex);
static inline Eina_Lock_Result eina_lock_take(Eina_Lock *mutex);
static inline Eina_Lock_Result eina_lock_take_try(Eina_Lock *mutex);
static inline Eina_Lock_Result eina_lock_release(Eina_Lock *mutex);
static inline void eina_lock_debug(const Eina_Lock *mutex);

static inline Eina_Bool eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex);
static inline void eina_condition_free(Eina_Condition *cond);
static inline Eina_Bool eina_condition_wait(Eina_Condition *cond);
static inline Eina_Bool eina_condition_timedwait(Eina_Condition *cond, double t);
static inline Eina_Bool eina_condition_broadcast(Eina_Condition *cond);
static inline Eina_Bool eina_condition_signal(Eina_Condition *cond);

static inline Eina_Bool eina_rwlock_new(Eina_RWLock *mutex);
static inline void eina_rwlock_free(Eina_RWLock *mutex);
static inline Eina_Lock_Result eina_rwlock_take_read(Eina_RWLock *mutex);
static inline Eina_Lock_Result eina_rwlock_take_write(Eina_RWLock *mutex);
static inline Eina_Lock_Result eina_rwlock_release(Eina_RWLock *mutex);

#ifdef EINA_HAVE_DEBUG_THREADS
# define EINA_MAIN_LOOP_CHECK_RETURN_VAL(val)				\
  do {									\
    if (EINA_UNLIKELY(!eina_main_loop_is()))				\
      {									\
	eina_error_set(EINA_ERROR_NOT_MAIN_LOOP);			\
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
	eina_error_set(EINA_ERROR_NOT_MAIN_LOOP);			\
	EINA_LOG_ERR("You are calling %s from outside"			\
		     "of the main loop threads in %s at line %i",	\
		     __FUNCTION__,					\
		     __FILE__,						\
		     __LINE__);						\
	return ;							\
      }									\
  } while (0)
#else
# define EINA_MAIN_LOOP_CHECK_RETURN_VAL(val)
# define EINA_MAIN_LOOP_CHECK_RETURN
#endif

/**
 * @}
 */

/**
 * @}
 */

#endif
