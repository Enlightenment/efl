/* EINA - EFL data type library
 * Copyright (C) 2012 Cedric Bail
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

#ifndef EINA_THREAD_H_
#define EINA_THREAD_H_

#include "eina_config.h"
#include "eina_types.h"
#include "eina_error.h"

#include <stdint.h>

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Thread_Group Thread
 *
 * Abstracts platform threads, providing a uniform API. It's modeled
 * after POSIX THREADS (pthreads), on Linux they are almost 1:1
 * mapping.
 *
 * @see @ref Eina_Lock_Group for mutex/locking abstraction.
 *
 * @since 1.8
 * @{
 */

/**
 * @typedef Eina_Thread
 * Type for a generic thread.
 */
typedef uintptr_t Eina_Thread;

/**
 * @typedef Eina_Thread_Cb
 * Type for the definition of a thread callback function
 */
typedef void *(*Eina_Thread_Cb)(void *data, Eina_Thread t);

/**
 * @typedef Eina_Thread_Priority
 * Type to enumerate different thread priorities
 */
typedef enum _Eina_Thread_Priority
{
  EINA_THREAD_URGENT,
  EINA_THREAD_NORMAL,
  EINA_THREAD_BACKGROUND,
  EINA_THREAD_IDLE
} Eina_Thread_Priority;

/**
 * @brief Returns identifier of the current thread.
 * @return identifier of current thread.
 * @since 1.8
 */
EAPI Eina_Thread eina_thread_self(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Checks if two thread identifiers are the same.
 * @param[in] t1 first thread identifier to compare.
 * @param[in] t2 second thread identifier to compare.
 * @return #EINA_TRUE if they are equal, #EINA_FALSE otherwise.
 * @since 1.8
 */
EAPI Eina_Bool eina_thread_equal(Eina_Thread t1, Eina_Thread t2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new thread, setting its priority and affinity.
 *
 * @param[out] t where to return the thread identifier. Must @b not be @c NULL.
 * @param[in] prio thread priority to use, usually EINA_THREAD_BACKGROUND
 * @param[in] affinity thread affinity to use. To not set affinity use @c -1.
 * @param[in] func function to run in the thread. Must @b not be @c NULL.
 * @param[in] data context data to provide to @a func as first argument.
 * @return #EINA_TRUE if thread was created, #EINA_FALSE on errors.
 * @since 1.8
 */
EAPI Eina_Bool eina_thread_create(Eina_Thread *t,
                                  Eina_Thread_Priority prio, int affinity,
                                  Eina_Thread_Cb func, const void *data) EINA_ARG_NONNULL(1, 4) EINA_WARN_UNUSED_RESULT;

/**
 * @brief The return value of eina_thread_join() if it was canceled with
 * eina_thread_cancel().
 *
 * A thread must be explicitly flagged as cancellable with
 * eina_thread_cancellable_set(), by default it's not and this value
 * shouldn't be returned.
 *
 * @see eina_thread_join()
 *
 * @since 1.19
 */
EAPI extern const void *EINA_THREAD_JOIN_CANCELED;

/**
 * @brief Joins a currently running thread, waiting until it finishes.
 *
 * This function will block the current thread until @a t
 * finishes. The returned value is the one returned by @a t @c func()
 * and may be @c NULL on errors. See @ref Eina_Error_Group to identify
 * problems.
 *
 * @param[in] t thread identifier to wait.
 * @return value returned by @a t creation function @c func() or
 *         @c NULL on errors. Check error with @ref Eina_Error_Group.
 *         If the thread was canceled, it will return
 *         EINA_THREAD_JOIN_CANCELED.
 * @since 1.8
 */
EAPI void *eina_thread_join(Eina_Thread t);

/**
 * @brief Sets the name of a given thread for debugging purposes.
 *
 * This maps to the pthread_setname_np() GNU extension or similar
 * if available. The name may be limited in size (possibly 16
 * characters including the null byte terminator). This is useful
 * for debugging to name a thread so external tools can display a
 * meaningful name attached to the thread.
 *
 * @param[in] t thread to set the name of
 * @param[in] name a string to name the thread - this cannot be NULL
 * @return EINA_TRUE if it succeeds in setting the name or EINA_FALSE
 *         otherwise.
 * @since 1.16
 */
EAPI Eina_Bool eina_thread_name_set(Eina_Thread t, const char *name);

/**
 * @brief Attempts to cancel a running thread.
 *
 * This function sends a cancellation request to the thread, however
 * that request is only fulfilled if the thread is cancellable
 * (eina_thread_cancellable_set() with EINA_TRUE as first parameter)
 * and it will wait for a cancellation point, be
 * eina_thread_cancel_checkpoint() or some syscall as defined in
 * man:pthreads(7).
 *
 * A thread that was canceled will return EINA_THREAD_JOIN_CANCELED
 * when eina_thread_join() is called.
 *
 * @param[in] t Thread to cancel.
 *
 * @return EINA_FALSE if thread was not running, EINA_TRUE
 *         otherwise. Note that if a thread is not cancellable and it
 *         is running, this function will return EINA_TRUE!
 *
 * @since 1.19
 */
EAPI Eina_Bool eina_thread_cancel(Eina_Thread t);

/**
 * @brief Enables or disables if the current thread can be canceled.
 *
 * By default eina_thread_create() will return a thread with
 * cancellation disabled. One can enable the cancellation by using
 * EINA_TRUE in @a cancellable.
 *
 * Eina threads follow pthread_setcanceltype()
 * PTHREAD_CANCEL_DEFERRED, that is, the actual termination will wait
 * for a cancellation point, usually a syscall defined in
 * man:pthreads(7) or an explicit cancellation point defined with
 * eina_thread_cancel_checkpoint().
 *
 * In order to provide cleanup around critical blocks use
 * EINA_THREAD_CLEANUP_PUSH() and EINA_THREAD_CLEANUP_POP() macros
 * (which maps to pthread_cleanup_push() and pthread_cleanup_pop()),
 * or the helper function eina_thread_cancellable_run() which does the
 * pair for you.
 *
 * @param[in] cancellable If EINA_TRUE, this thread will be accept
 *        cancellation requests. If EINA_FALSE -- the default, it will
 *        ignore cancellation requests.
 * @param[in] was_cancellable If non-NULL, will return the previous state,
 *        shall you want to restore.
 *
 * @return EINA_TRUE if it succeeds in setting the cancellable state
 *        or EINA_FALSE otherwise.
 *
 * @see eina_thread_cancel_checkpoint()
 * @see EINA_THREAD_CLEANUP_PUSH()
 * @see EINA_THREAD_CLEANUP_POP()
 * @see eina_thread_cancellable_run()
 * @see eina_thread_cancel()
 *
 * @since 1.19
 */
EAPI Eina_Bool eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable);

/**
 * If the current thread is cancellable, this introduces a
 * cancellation check point. Otherwise it's a no-operation.
 *
 * Eina threads follow pthread_setcanceltype()
 * PTHREAD_CANCEL_DEFERRED, that is, the actual termination will wait
 * for a cancellation point, usually a syscall defined in
 * man:pthreads(7) or an explicit cancellation point defined with this
 * function.
 *
 * @see eina_thread_cancel_checkpoint()
 *
 * @since 1.19
 */
EAPI void eina_thread_cancel_checkpoint(void);

/**
 * @def EINA_THREAD_CLEANUP_PUSH(cleanup, data)
 *
 * @brief Pushes a cleanup function to be executed when the thread is
 * canceled.
 *
 * This macro will schedule a function cleanup(data) to be executed if
 * the thread is canceled with eina_thread_cancel() and the thread
 * was previously marked as cancellable with
 * eina_thread_cancellable_set().
 *
 * It @b must be paired with EINA_THREAD_CLEANUP_POP() in the same
 * code block as they will expand to do {} while ()!
 *
 * The cleanup function may also be executed if
 * EINA_THREAD_CLEANUP_POP(EINA_TRUE) is used.
 *
 * @note If the block within EINA_THREAD_CLEANUP_PUSH() and
 *       EINA_THREAD_CLEANUP_POP() returns, the cleanup callback will
 *       @b not be executed! To avoid problems prefer to use
 *       eina_thread_cancellable_run()!
 *
 * @param[in] cleanup The function to execute on cancellation.
 * @param[in] data The context to give to cleanup function.
 *
 * @see eina_thread_cancellable_run()
 *
 * @since 1.19
 */
#define EINA_THREAD_CLEANUP_PUSH(cleanup, data) \
  pthread_cleanup_push(cleanup, data)

/**
 * @def EINA_THREAD_CLEANUP_POP(exec_cleanup)
 *
 * @brief Pops a cleanup function to be executed when the thread is
 * canceled.
 *
 * This macro will remove a previously pushed cleanup function, thus
 * if the thread is canceled with eina_thread_cancel() and the thread
 * was previously marked as cancellable with
 * eina_thread_cancellable_set(), that cleanup won't be executed
 * anymore.
 *
 * It @b must be paired with EINA_THREAD_CLEANUP_PUSH() in the same
 * code block as they will expand to do {} while ()!
 *
 * @note If the block within EINA_THREAD_CLEANUP_PUSH() and
 *       EINA_THREAD_CLEANUP_POP() returns, the cleanup callback will
 *       @b not be executed even if exec_cleanup is EINA_TRUE! To
 *       avoid problems prefer to use eina_thread_cancellable_run()!
 *
 * @param[in] exec_cleanup if EINA_TRUE, the function registered with
 *        EINA_THREAD_CLEANUP_PUSH() will be executed.
 *
 * @see eina_thread_cancellable_run()
 *
 * @since 1.19
 */
#define EINA_THREAD_CLEANUP_POP(exec_cleanup) \
  pthread_cleanup_pop(exec_cleanup)

/**
 * @typedef Eina_Thread_Cancellable_Run_Cb
 * Type for the definition of a cancellable callback to run.
 *
 * @since 1.19
 */
typedef void *(*Eina_Thread_Cancellable_Run_Cb)(void *data);

/**
 * This function will setup cleanup callback, turn the thread
 * cancellable, execute the given callback, reset the cancellable
 * state to its old value, run the cleanup callback and then return
 * the callback return value.
 *
 * @note cleanup_cb is configured @b before the thread is made
 *       cancellable, thus it @b will be executed while @a cb may not
 *       in the case the thread was already canceled and that was
 *       pending.
 *
 * This helper does exactly the following code. Should you need a
 * slightly different behavior, use the base calls yourself.
 *
 * @code
 *    Eina_Bool old = EINA_FALSE;
 *    void *ret;
 *
 *    EINA_THREAD_CLEANUP_PUSH(cleanup_cb, data);
 *    eina_thread_cancellable_set(EINA_TRUE, &old); // is a cancellation point
 *    ret = cb(data); // may not run if was previously canceled
 *    EINA_THREAD_CLEANUP_POP(EINA_TRUE);
 *    eina_thread_cancellable_set(old, NULL);
 *    return ret;
 * @endcode
 *
 * @param[in] cb a cancellable callback to possibly run. The callback @b
 *        may not be executed if the thread had a pending cancellation
 *        request. During its execution the callback may be canceled
 *        at explicit cancellation points using
 *        eina_thread_cancel_checkpoint(), as well as some syscalls
 *        defined in man:pthreads(7).
 * @param[in] cleanup_cb a cleanup callback to be executed regardless of
 *        the thread being canceled or not. This function will be
 *        executed even if @a cb wasn't.
 * @param[in] data context to give to both @a cb and @a cleanup_cb.
 *
 * @return the return value of @a cb. If the thread was canceled,
 *         this function will not return.
 *
 * @since 1.19
 */
EAPI void *eina_thread_cancellable_run(Eina_Thread_Cancellable_Run_Cb cb, Eina_Free_Cb cleanup_cb, void *data);

/**
 * @}
 */

/**
 * @}
 */

#endif
