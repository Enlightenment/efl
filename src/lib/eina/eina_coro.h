/* EINA - EFL data type library
 * Copyright (C) 2017 ProFUSION embedded systems
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

#ifndef EINA_CORO_H_
#define EINA_CORO_H_

#include "eina_config.h"
#include "eina_types.h"
#include "eina_error.h"

typedef struct _Eina_Future Eina_Future;
typedef struct _Eina_Value Eina_Value;

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Coro_Group Co-routines
 *
 * Co-routines are cooperative threads, that is, their execution will
 * stop the caller's thread, execute the coroutine until it finishes
 * or yield, then give back control to the caller thread.
 *
 * The purpose of this primitive is to allow two functions to run with
 * their own stack and registers and be sure that the caller thread
 * won't run meanwhile, then shared context (variables, pointers) do
 * not need locks as this is done implicitly by the design: if one
 * thread is running you can be sure the other is not. The coroutine
 * must explicitly give back control to the caller thread, either by
 * eina_coro_yield() or by return.
 *
 * Multiple coroutines may exist at a given time, however if they are
 * managed by the same caller then it's guaranteed that they will
 * cooperate among themselves.
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such or Efl_Object's efl_domain_data_adopt()
 *       (done automatically from efl_object_init()).
 *
 * @see @ref Eina_Thread_Group for regular concurrent threads.
 *
 * @since 1.21
 * @{
 */

typedef struct _Eina_Coro Eina_Coro;

/**
 * @typedef Eina_Coro_Hook_Coro_Enter_Cb
 *
 * @brief Type for the definition of a coroutine hook.
 *
 * The pointer will be called back with the given @c data and the
 * coroutine that will be entered or exited.
 *
 * The coroutine "enters" when eina_coro_run() is called and "exits"
 * when the provided function returns or calls eina_coro_yield().
 *
 * If the callback returns #EINA_FALSE, then eina_coro_yield() will
 * return that value, meaning the coroutine should voluntarily exit.
 *
 * All hooks are called, and if any of them returns #EINA_FALSE,
 * eina_coro_yield() will return the same.
 *
 * If #EINA_FALSE is returned prior the first execution of the
 * coroutine, then the coroutine will get a canceled #EINA_TRUE as
 * parameter. This allows coroutine to cleanup whatever is needed
 * and return.
 *
 * @since 1.21
 */
typedef Eina_Bool (*Eina_Coro_Hook_Coro_Enter_Cb)(void *data, const Eina_Coro *coro);

/**
 * @typedef Eina_Coro_Hook_Coro_Exit_Cb
 *
 * @brief Type for the definition of a coroutine exit hook.
 *
 * The pointer will be called back with the given @c data and the
 * coroutine that exited.
 *
 * The coroutine "enters" when eina_coro_run() is called and "exits"
 * when the provided function returns or calls eina_coro_yield().
 *
 * Exit hooks are always called in the reverse order they were added,
 * that is, the last added hook will run first (stack).
 *
 * @since 1.21
 */
typedef void (*Eina_Coro_Hook_Coro_Exit_Cb)(void *data, const Eina_Coro *coro);

/**
 * @typedef Eina_Coro_Hook_Main_Enter_Cb
 *
 * @brief Type for the definition of a main routine hook.
 *
 * The pointer will be called back with the given @c data and the
 * coroutine that will be entered or exited.
 *
 * The coroutine "enters" when eina_coro_run() is called and "exits"
 * when the provided function returns or calls eina_coro_yield().
 *
 * Unlike the coroutine exit hooks, the main routine Exit hooks are
 * always called in the reverse order they were added, that is, the
 * last added hook will run first (stack). This is because they match
 * Eina_Coro_Hook_Coro_Exit_Cb, a coroutine exits so the main routine
 * can enter.
 *
 * @since 1.21
 */
typedef void (*Eina_Coro_Hook_Main_Enter_Cb)(void *data, const Eina_Coro *coro);

/**
 * @typedef Eina_Coro_Hook_Main_Exit_Cb
 *
 * @brief Type for the definition of a main routine exit hook.
 *
 * The pointer will be called back with the given @c data and the
 * coroutine that exited.
 *
 * The coroutine "enters" when eina_coro_run() is called and "exits"
 * when the provided function returns or calls eina_coro_yield().
 *
 * Unlike the coroutine enter hooks, the main routine Enter hooks are
 * called in forward order, that is, the first added hook will run
 * first. This is because they match Eina_Coro_Hook_Coro_Enter_Cb, a
 * main routine exits so the coroutine can enter.
 *
 * If the callback returns #EINA_FALSE, then eina_coro_yield() will
 * return that value, meaning the coroutine should voluntarily exit.
 *
 * All hooks are called, and if any of them returns #EINA_FALSE,
 * eina_coro_yield() will return the same.
 *
 * If #EINA_FALSE is returned prior the first execution of the
 * coroutine, then the coroutine will get a canceled #EINA_TRUE as
 * parameter. This allows coroutine to cleanup whatever is needed
 * and return.
 *
 * @since 1.21
 */
typedef Eina_Bool (*Eina_Coro_Hook_Main_Exit_Cb)(void *data, const Eina_Coro *coro);


/**
 * Adds a hook to the coroutine subsystem.
 *
 * The coroutine "enters" when eina_coro_run() is called and "exits"
 * when the provided function returns or calls eina_coro_yield().
 *
 * The main routine (the caller) is the opposite: when eina_coro_run()
 * it will "exit" and it will "enter" before eina_coro_run() returns.
 *
 * Enter hooks are executed in order, while exit hooks are always
 * called in the reverse order they were added, that is, the last
 * added hook will run first (stack).
 *
 * If any enter hooks fail, then eina_coro_yield() will return
 * #EINA_FALSE or the parameter @c canceled as #EINA_TRUE will be
 * given to the coroutine function. This allows coroutines to cleanup
 * and exit.
 *
 * The flow is the following:
 *
 * @li main_exit is called to notify main routine will be stopped.
 * @li coro_enter is called to notify the coroutine will be started.
 * @li coro_exit is called to notify the coroutine stopped.
 * @li main_enter is called to notify main routine will be resumed.
 *
 * They may be useful to properly setup environment prior to callback
 * user code, like Efl_Object must check if we're running in an actual
 * thread and adopt Efl_Domain_Data so objects are accessible from the
 * coroutine -- this is done automatically by efl_object_init().
 *
 * @note this must @b NOT be called from within the coroutine itself,
 *       usually do this from your "init".
 *
 * @param coro_enter the hook to be called when the coroutine
 *        enters. May be #NULL if no enter hook is needed.
 * @param coro_exit the hook to be called when the coroutine
 *        exits. May be #NULL if no exit hook is needed.
 * @param main_enter the hook to be called when the caller
 *        enters. May be #NULL if no enter hook is needed.
 * @param main_exit the hook to be called when the caller
 *        exits. May be #NULL if no exit hook is needed.
 * @param data the context to pass to hooks.  May be #NULL if no
 *        context is needed.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @since 1.12
 */
EAPI Eina_Bool eina_coro_hook_add(Eina_Coro_Hook_Coro_Enter_Cb coro_enter, Eina_Coro_Hook_Coro_Exit_Cb coro_exit, Eina_Coro_Hook_Main_Enter_Cb main_enter, Eina_Coro_Hook_Main_Exit_Cb main_exit, const void *data);

/**
 * Removes a hook from the coroutine subsystem.
 *
 * @note this must @b NOT be called from within the coroutine itself,
 *       usually do this from your "init".
 *
 * @param coro_enter the pointer that was given to eina_coro_hook_add().
 * @param coro_exit the pointer that was given to eina_coro_hook_add().
 * @param main_enter the pointer that was given to eina_coro_hook_add().
 * @param main_exit the pointer that was given to eina_coro_hook_add().
 * @param data the  pointer that was given to eina_coro_hook_add().
 * @return #EINA_TRUE on success, #EINA_FALSE on failure (ie: not found).
 *
 * @since 1.12
 */
EAPI Eina_Bool eina_coro_hook_del(Eina_Coro_Hook_Coro_Enter_Cb coro_enter, Eina_Coro_Hook_Coro_Exit_Cb coro_exit, Eina_Coro_Hook_Main_Enter_Cb main_enter, Eina_Coro_Hook_Main_Exit_Cb main_exit, const void *data);

/**
 * @typedef Eina_Coro_Cb
 * Type for the definition of a coroutine callback function.
 *
 * Once eina_coro_new() returns non-NULL, this function will be always
 * called. However it may receive @c canceled as #EINA_TRUE, in this
 * case it should cleanup and exit as soon as possible.
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such as the Python's GIL or Efl_Object's
 *       efl_domain_data_adopt() (done automatically from
 *       efl_object_init()).
 *
 * @since 1.21
 */
typedef const void *(*Eina_Coro_Cb)(void *data, Eina_Bool canceled, Eina_Coro *coro);

/**
 * @def EINA_CORO_STACK_SIZE_DEFAULT
 *
 * Use the system's default stack size, usually @c PTHREAD_STACK_MIN
 * (16Kb - 16384 bytes).
 *
 * @since 1.21
 */
#define EINA_CORO_STACK_SIZE_DEFAULT 0

/**
 * @brief Creates a new coroutine.
 *
 * Allocates a coroutine environment using the given @a stack_size to
 * execute @a func with the given context @a data. The coroutine must
 * be explicitly executed with eina_coro_run(), that will stop the
 * caller and let the coroutine work.
 *
 * @note The coroutine is @b not executed when it's created, that is
 *       managed by eina_coro_run().
 *
 * @note Currently @a stack_size is ignored, the default thread stack
 *       size will be used.
 *
 * @param func function to run in the coroutine. Must @b not be @c NULL.
 * @param data context data to provide to @a func as first argument.
 * @param stack_size defines the stack size to use to run the function
 *        @a func. Usually must be multiple of @c PAGE_SIZE and most
 *        systems will define a minimum stack limit such as 16Kb -
 *        those nuances are handled automatically for you. Most users
 *        want #EINA_CORO_STACK_SIZE_DEFAULT.
 * @return newly allocated coroutine handle on success, #NULL on failure.
 *
 * @see eina_coro_run()
 *
 * @since 1.21
 */
EAPI Eina_Coro *eina_coro_new(Eina_Coro_Cb func, const void *data, size_t stack_size)  EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Yields control from coroutine to caller.
 *
 * This @b must be called from within the @b coroutine and will pause it,
 * giving back control to the caller of eina_coro_run(). The coroutine
 * will remain stalled until eina_coro_run() is executed again.
 *
 * Yield can return #EINA_FALSE if any hooks fail, see
 * eina_coro_hook_add(), or if the coroutine is canceled with
 * eina_coro_cancel().
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such as the Python's GIL or Efl_Object's
 *       efl_domain_data_adopt() (done automatically from
 *       efl_object_init()).
 *
 * @param coro the coroutine that will yield control. Must not be #NULL.
 *
 * @return #EINA_TRUE if coroutine is allowed to continue,
 *         #EINA_FALSE if the coroutine should cleanup and exit.
 *         Refer to macros eina_coro_yield_or_return() or
 *         eina_coro_yield_or_goto() to aid such tasks.
 *
 * @see eina_coro_run()
 * @see eina_coro_new()
 *
 * @since 1.21
 */
EAPI Eina_Bool eina_coro_yield(Eina_Coro *coro) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * #def eina_coro_yield_or_return(coro, val)
 *
 * This will yield control back to the main thread and wait. If the
 * yield returns #EINA_FALSE then it will return the given value.
 *
 * @param coro the coroutine handle. Must not be #NULL.
 * @param val the value to return (void *) if yield returns #EINA_FALSE.
 *
 * @see eina_coro_yield()
 * @see eina_coro_yield_or_goto()
 *
 * @since 1.21
 */
#define eina_coro_yield_or_return(coro, val) \
  do { if (!eina_coro_yield(coro)) return (val); } while (0)

/**
 * #def eina_coro_yield_or_goto(coro, label)
 *
 * This will yield control back to the main thread and wait. If the
 * yield returns #EINA_FALSE then it will goto the given label.
 *
 * @param coro the coroutine handle. Must not be #NULL.
 * @param label the label to jump if yield returns #EINA_FALSE.
 *
 * @see eina_coro_yield()
 * @see eina_coro_yield_or_return()
 *
 * @since 1.21
 */
#define eina_coro_yield_or_goto(coro, label) \
  do { if (!eina_coro_yield(coro)) goto label; } while (0)

/**
 * @brief Yield control from coroutine to the caller and report it's
 * awaiting a future to be resolved.
 *
 * This @b must be called from within the @b coroutine and will pause
 * it, giving back control to the caller of eina_coro_run(). The
 * coroutine will remain stalled until eina_coro_run() is executed
 * again. The caller of eina_coro_run() will get @c p_awaiting set so
 * it can wait for the future to be resolved and improve its
 * scheduling. Note that this will busy wait using eina_coro_yield()
 * until the future resolves, then it's safe to naively call
 * eina_coro_run() if the future is still pending -- but it will keep
 * CPU consumption to its maximum.
 *
 * Await can return #EINA_FALSE if any hooks fail, see
 * eina_coro_hook_add(), or if the coroutine is canceled with
 * eina_coro_cancel().
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such as the Python's GIL or Efl_Object's
 *       efl_domain_data_adopt() (done automatically from
 *       efl_object_init()).
 *
 * @param coro the coroutine that will await control. Must not be #NULL.
 * @param future the future that this will wait. Must not be #NULL.
 * @param[out] p_value the pointer to value containing the future
 *        resolution. If given the value will be owned by caller and
 *        must be released with eina_value_flush(). May be #NULL.
 *
 * @return #EINA_TRUE if coroutine is allowed to continue,
 *         #EINA_FALSE if the coroutine should cleanup and exit.
 *         Refer to macros eina_coro_await_or_return() or
 *         eina_coro_await_or_goto() to aid such tasks.
 *
 * @see eina_coro_run()
 * @see eina_coro_new()
 * @see eina_coro_yield()
 *
 * @since 1.21
 */
EAPI Eina_Bool eina_coro_await(Eina_Coro *coro, Eina_Future *future, Eina_Value *p_value) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * #def eina_coro_await_or_return(coro, future, p_value, val)
 *
 * This will yield control back to the main thread and wait for future
 * to be resolved. If the await returns #EINA_FALSE then it will
 * return the given value.
 *
 * @param coro the coroutine handle. Must not be #NULL.
 * @param future the future that this will wait. Must not be #NULL.
 * @param[out] p_value the pointer to value containing the future
 *        resolution. If given the value will be owned by caller and
 *        must be released with eina_value_flush(). May be #NULL.
 * @param val the value to return (void *) if await returns #EINA_FALSE.
 *
 * @see eina_coro_await()
 * @see eina_coro_await_or_goto()
 *
 * @since 1.21
 */
#define eina_coro_await_or_return(coro, future, p_value, val) \
  do { if (!eina_coro_await(coro, future, p_value)) return (val); } while (0)

/**
 * #def eina_coro_await_or_goto(coro, label)
 *
 * This will yield control back to the main thread and wait for future
 * to be resolved. If the await returns #EINA_FALSE then it will goto
 * the given label.
 *
 * @param coro the coroutine handle. Must not be #NULL.
 * @param future the future that this will wait. Must not be #NULL.
 * @param[out] p_value the pointer to value containing the future
 *        resolution. If given the value will be owned by caller and
 *        must be released with eina_value_flush(). May be #NULL.
 * @param label the label to jump if await returns #EINA_FALSE.
 *
 * @see eina_coro_await()
 * @see eina_coro_await_or_return()
 *
 * @since 1.21
 */
#define eina_coro_await_or_goto(coro, future, p_value, label) \
  do { if (!eina_coro_await(coro, future, p_value)) goto label; } while (0)


/**
 * @brief Run the coroutine and report if it's still alive.
 *
 * This @b must be called from @b outside the coroutine, what we call
 * "the caller side", also known as "main thread". The caller will be
 * paused and control will be handled to the coroutine until it
 * eina_coro_yield() or exits (returns).
 *
 * If the coroutine exits (returns), then this function will free the
 * coroutine created, make @a p_coro #NULL, set @a p_result to
 * whatever the Eina_Coro_Cb returned and return #EINA_FALSE.
 *
 * If the coroutine yields, then this function will return #EINA_TRUE,
 * meaning it must be called again.
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such as the Python's GIL or Efl_Object's
 *       efl_domain_data_adopt() (done automatically from
 *       efl_object_init()).
 *
 * @param[inout] p_coro pointer to the coroutine that will be
 *        executed.  Must @b not be #NULL.
 * @param[out] p_result if the coroutine exited, then will be set to the
 *        returned value. May be #NULL.
 * @param[out] p_awaiting if the coroutine is awaiting a future to be
 *        resolved, then the handle. The scheduler should consider
 *        eina_future_then() and only call the coroutine once it's
 *        resolved. This happens when coroutine used eina_coro_await()
 *        instead of eina_coro_yield().
 *
 * @return #EINA_TRUE if the coroutine yielded and the user must call
 *         eina_coro_run() again. #EINA_FALSE if the coroutine exited,
 *         then @a p_coro will point to #NULL and if @a p_result is
 *         given it will be set to whatever the coroutine @c func returned.
 *
 * @since 1.21
 */
EAPI Eina_Bool eina_coro_run(Eina_Coro **p_coro, void **p_result, Eina_Future **p_awaiting) EINA_ARG_NONNULL(1);

/**
 * @brief Cancel the coroutine and wait for it to finish.
 *
 * This @b must be called from @b outside the coroutine, what we call
 * "the caller side", also known as "main thread". The caller will be
 * paused and control will be handled to the coroutine until it exits
 * (returns).
 *
 * This function will free the coroutine created, make @a p_coro
 * #NULL, set @a p_result to whatever the Eina_Coro_Cb returned and
 * return #EINA_FALSE.
 *
 * @note This will busy wait on eina_coro_run() until the coroutine @b
 *       voluntarily finishes, it's not aborted in any way. The
 *       primitive eina_coro_yield() will return #EINA_FALSE when the
 *       coroutine is canceled, this gives the opportunity to cleanup
 *       and exit. Likewise, if canceled before it's ever ran, then
 *       it will pass @c canceled as #EINA_TRUE to the Eina_Coro_Cb
 *       given to eina_coro_new()
 *
 * @note The current implementation @b may use real threads with a
 *       lock and condition variable to ensure the behavior, this is
 *       an implementation detail that must not be relied upon.
 *       Depending on the platform it may use ucontext.h (SysV-like)
 *       or custom task save/restore. Libraries can use
 *       eina_coro_hook_add() to be called when the coroutine code
 *       will enter and exit, being able to retrieve context and set
 *       some other locks such as the Python's GIL or Efl_Object's
 *       efl_domain_data_adopt() (done automatically from
 *       efl_object_init()).
 *
 * @param[inout] p_coro pointer to the coroutine that will be
 *        executed.  Must @b not be #NULL.
 *
 * @return The value returned by the coroutine function, same as would
 *         be returned in eina_coro_run()'s @c p_result parameter.
 *
 * @since 1.21
 */
EAPI void *eina_coro_cancel(Eina_Coro **p_coro) EINA_ARG_NONNULL(1);

/**
 * @}
 */

/**
 * @}
 */

#endif
