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

#ifndef EINA_INLINE_LOCK_VOID_X_
#define EINA_INLINE_LOCK_VOID_X_

#ifdef EINA_UNUSED
# undef EINA_UNUSED
#endif
#ifdef __GNUC__
# define EINA_UNUSED __attribute__((unused))
#else
# define EINA_UNUSED
#endif

/**
 * @addtogroup Eina_Lock_Group Lock
 *
 * @brief These functions provide Mutual Exclusion objects management.
 *
 * @note On Windows XP, critical sections are used, while on Windows
 * CE, standard Mutex objects are used.
 *
 * @{
 */

/**
 * @typedef Eina_Lock
 * Abtract type for a mutual exclusive object.
 */
typedef void *Eina_Lock;
typedef void *Eina_RWLock;
typedef void *Eina_Condition;

/**
 * @brief Create a new #Eina_Lock.
 *
 * @param mutex A pointer to the lock object.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function creates a new #Eina_Lock object and stores it in the
 * @p mutex buffer. On success, this function returns #EINA_TRUE and
 * #EINA_FALSE otherwise. To free the resources allocated by this
 * function, use eina_lock_free(). For performance reasons, no check
 * is done on @p mutex.
 */
static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex EINA_UNUSED)
{
   return EINA_FALSE;
}

/**
 * @brief Free the ressources of the given lock object.
 *
 * @param mutex The lock object to free.
 *
 * This function frees the resources of @p mutex allocated by
 * eina_lock_new(). For performance reasons, no check is done on
 * @p mutex.
 */
static inline void
eina_lock_free(Eina_Lock *mutex EINA_UNUSED)
{
}

/**
 * @brief Lock the given mutual exclusion object.
 *
 * @param mutex The lock object to lock.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function locks @p mutex. @p mutex must have been created by
 * eina_lock_new(). On success, this function returns #EINA_TRUE and
 * #EINA_FALSE otherwise. For performance reasons, no check is done on
 * @p mutex.
 */
static inline Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

/**
 * @brief Try to lock the given mutual exclusion object.
 *
 * @param mutex The lock object to try to lock.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function tries to lock @p mutex. @p mutex must have been created by
 * eina_lock_new(). If @p mutex can be locked, this function returns
 * #EINA_TRUE; if @p mutex can not be locked, or is already locked, it
 * returns #EINA_FALSE. This function does not block and returns
 * immediately. For performance reasons, no check is done on
 * @p mutex.
 *
 * @note On Windows CE, this function is actually eina_lock_take().
 */
static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

/**
 * @brief Unlock the given mutual exclusion object.
 *
 * @param mutex The lock object to unlock.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function unlocks @p mutex. @p mutex must have been created by
 * eina_lock_new(). On success, this function returns #EINA_TRUE and
 * #EINA_FALSE otherwise. For performance reasons, no check is done on
 * @p mutex.
 */
static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

static inline void
eina_lock_debug(const Eina_Lock *mutex EINA_UNUSED)
{
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond EINA_UNUSED, Eina_Lock *mutex EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline void
eina_condition_free(Eina_Condition *cond EINA_UNUSED)
{
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_timedwait(Eina_Condition *cond EINA_UNUSED, double val EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_rwlock_new(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_FALSE;
}

static inline void
 eina_rwlock_free(Eina_RWLock *mutex EINA_UNUSED)
{
}

static inline Eina_Lock_Result
eina_rwlock_read_take(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_write_take(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_release(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_take_read(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

static inline Eina_Lock_Result
eina_rwlock_take_write(Eina_RWLock *mutex EINA_UNUSED)
{
   return EINA_LOCK_FAIL;
}

/**
 * @}
 */

#endif
