#ifndef _EINA_PROMISE_H_
#define _EINA_PROMISE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "eina_safety_checks.h"
#include "eina_types.h"
#include "eina_value.h"

/**
 * @ingroup Eina_Promise
 *
 * @{
 */
typedef struct _Eina_Future_Desc Eina_Future_Desc;
typedef struct _Eina_Promise Eina_Promise;
typedef struct _Eina_Future Eina_Future;
typedef struct _Eina_Future_Cb_Easy_Desc Eina_Future_Cb_Easy_Desc;
typedef struct _Eina_Future_Cb_Console_Desc Eina_Future_Cb_Console_Desc;
typedef struct _Eina_Future_Scheduler Eina_Future_Scheduler;
typedef struct _Eina_Future_Schedule_Entry Eina_Future_Schedule_Entry;
typedef struct _Eina_Future_Race_Result Eina_Future_Race_Result;
typedef struct _Eina_Future_Cb_Log_Desc Eina_Future_Cb_Log_Desc;

/**
 * @defgroup Eina_Future_Callbacks Efl Future Callbacks
 * @ingroup eina_future
 * @typedef Eina_Future_Cb Eina_Future_Cb
 *
 * A callback used to inform that a future was resolved.
 * Usually this callback is called from a clean context, that is, from the
 * main loop or some platform defined safe context. However there are
 * 2 exceptions:
 *
 * @li eina_future_cancel() was used, it's called immediately in the
 * context that called cancel using `ECANCELED` as error.
 *
 * @li eina_future_then(), eina_future_then_from_desc(), eina_future_chain(), eina_future_chain_array()
 * or similar failed due invalid pointer or memory allocation. Then the callback is called from the
 * failed context using `EINVAL` or `ENOMEM` as errors and @p dead_future will be @c NULL.
 *
 * @param data The data provided by the user
 *
 * @param value An Eina_Value which contains the operation result. Before using
 * the @p value, its type must be checked in order to avoid errors. This is needed, because
 * if an operation fails the Eina_Value type will be EINA_VALUE_TYPE_ERROR
 * which is a different type than the expected operation result.
 *
 * @param dead_future A pointer to the future that was completed.
 *
 * @return An Eina_Value to pass to the next Eina_Future in the chain (if any).
 * If there is no need to convert the received value, it's @b recommended
 * to pass-thru @p value argument. If you need to convert to a different type
 * or generate a new value, use @c eina_value_setup() on @b another Eina_Value
 * and return it. By returning an promise Eina_Value (eina_promise_as_value()) the
 * whole chain will wait until the promise is resolved in
 * order to continue its execution.
 * Note that the value contents must survive this function scope,
 * that is, do @b not use stack allocated blobs, arrays, structures or types that
 * keeps references to memory you give. Values will be automatically cleaned up
 * using @c eina_value_flush() once they are unused (no more future or futures
 * returned a new value).
 *
 * @note The returned value @b can be an EFL_VALUE_TYPE_PROMISE! (see eina_promise_as_value() and
 * eina_future_as_value()) In this case the future chain will wait until the promise is resolved.
 *
 * @see eina_future_cancel()
 * @see eina_future_then()
 * @see eina_future_then_from_desc()
 * @see eina_future_then_easy()
 * @see eina_future_chain()
 * @see eina_future_chain_array()
 * @see eina_future_as_value()
 * @see eina_promise_as_value()
 * @{
 */
typedef Eina_Value (*Eina_Future_Cb)(void *data, const Eina_Value value, const Eina_Future *dead_future);

/**
 * @struct _Eina_Future_Scheduler
 * @ingroup eina_promise
 *
 * A struct that represents an scheduled event.
 * This struct may be used by Eina to cancel
 * a scheduled future.
 *
 * @see eina_promise_new()
 *
 * @see #Eina_Future_Scheduler
 * @see #Eina_Future_Scheduler_Cb
 */
struct _Eina_Future_Schedule_Entry {
   /**
    * The scheduler used to create the entry.
    * @note This must not be @c NULL.
    */
   Eina_Future_Scheduler *scheduler;
};


/**
 * @typedef Eina_Future_Scheduler_Cb
 * @ingroup eina_promise
 * A callback used by the Eina_Future_Scheduler to deliver
 * the future operation result.
 *
 * @param f The delivered future.
 * @param value The future result
 *
 *
 * @see eina_promise_new()
 *
 * @see #Eina_Future_Schedule_Entry
 * @see #Eina_Future_Scheduler
 */
typedef void (*Eina_Future_Scheduler_Cb)(Eina_Future *f, Eina_Value value);

/**
 * @struct _Eina_Future_Scheduler
 * @ingroup eina_promise
 * This struct is used as a bridge between Eina and the future scheduler.
 * By using the functions provided by #_Eina_Future_Scheduler Eina can
 * schedule futures resolutions, rejections and cancelations to a safe context.
 *
 * @see eina_promise_new()
 * @see #Eina_Future_Schedule_Entry
 * @see #Eina_Future_Scheduler_Cb
 */
struct _Eina_Future_Scheduler {
   /**
    * Called by @p Eina_Future when a delivery must be scheduled to a safe context.
    * ie: after @p eina_promise_resolve()
    *
    * @note Must not be @c NULL
    *
    * Must call back from a safe context using @p cb(f,value)
    * @param scheduler The scheduler to use.
    * @param cb The #Eina_Future_Scheduler_Cb to be called and deliver the @p f and @p value.
    * @param f The future to be delivered to @p cb
    * @param value The value to be delivered to @p cb
    * @return A scheduled entry or @c NULL on error
    */
   Eina_Future_Schedule_Entry *(*schedule)(Eina_Future_Scheduler *scheduler, Eina_Future_Scheduler_Cb cb, Eina_Future *f, Eina_Value value);
   /**
    * Called by @p Eina_Future when a delivery must be cancelled.
    * ie: after @p eina_future_cancel()
    *
    * @note Must not be @c NULL.
    *
    * @param entry The scheduled event to cancel
    */
   void (*recall)(Eina_Future_Schedule_Entry *entry);
};

/**
 * @typedef Eina_Promise_Cancel_Cb Eina_Promise_Cancel_Cb.
 * @ingroup eina_promise
 *
 * A callback used to inform that a promise was canceled.
 *
 * A promise may be canceled by the user calling `eina_future_cancel()`
 * on any Eina_Future that is part of the chain that uses an Eina_Promise,
 * that will cancel the whole chain and then the promise.
 *
 * It should stop all asynchronous operations or at least mark them to be
 * discarded instead of resolved. Actually it can't be resolved once
 * cancelled since the given pointer @c dead_promise is now invalid.
 *
 * @note This callback is @b mandatory for a reason, do not provide an empty
 *       callback as it'll likely result in memory corruption and invalid access.
 *       If impossible to cancel an asynchronous task, then create an
 *       intermediate memory to hold Eina_Promise and make it @c NULL,
 *       in this callback. Then prior to resolution check if the pointer is set.
 *
 * @note This callback is @b not called if eina_promise_resolve() or
 *       eina_promise_reject() are used. If any cleanup is needed, then
 *       call yourself. It's only meant as cancellation, not a general
 *       "free callback".
 *
 * @param data The data provided by the user.
 * @param dead_promise The canceled promise.
 * @see eina_promise_reject()
 * @see eina_promise_resolve()
 * @see eina_future_cancel()
 */
typedef void (*Eina_Promise_Cancel_Cb) (void *data, const Eina_Promise *dead_promise);

/**
 * @typedef Eina_Future_Success_Cb Eina_Future_Success_Cb.
 * @ingroup eina_future
 *
 * A callback used to inform that the future completed with success.
 *
 * Unlike #Eina_Future_Cb this callback only called if the future operation was successful, this is,
 * no errors occurred (@p value type differs from EINA_VALUE_TYPE_ERROR)
 * and the @p value matches #_Eina_Future_Cb_Easy_Desc::success_type (if given).
 * In case #_Eina_Future_Cb_Easy_Desc::success_type was not supplied (it's @c NULL) the @p value type
 * must be checked before using it.
 *
 * @note This function is always called from a safe context (main loop or some platform defined safe context).
 *
 * @param data The data provided by the user.
 * @param value The operation result
 * @return An Eina_Value to pass to the next Eina_Future in the chain (if any).
 * If there is no need to convert the received value, it's @b recommended
 * to pass-thru @p value argument. If you need to convert to a different type
 * or generate a new value, use @c eina_value_setup() on @b another Eina_Value
 * and return it. By returning an promise Eina_Value (eina_promise_as_value()) the
 * whole chain will wait until the promise is resolved in
 * order to continue its execution.
 * Note that the value contents must survive this function scope,
 * that is, do @b not use stack allocated blobs, arrays, structures or types that
 * keeps references to memory you give. Values will be automatically cleaned up
 * using @c eina_value_flush() once they are unused (no more future or futures
 * returned a new value).
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 */
typedef Eina_Value (*Eina_Future_Success_Cb)(void *data, const Eina_Value value);

/**
 * @typedef Eina_Future_Error_Cb Eina_Future_Error_Cb.
 * @ingroup eina_future
 *
 * A callback used to inform that the future completed with failure.
 *
 * Unlike #Eina_Future_Success_Cb this function is only called when an error
 * occurs during the future process or when #_Eina_Future_Cb_Easy_Desc::success_type
 * differs from the future result.
 * On future creation errors and future cancellation this function will be called
 * from the current context with the following errors respectitally: `EINVAL`, `ENOMEM` and  `ECANCELED`.
 * Otherwise this function is called from a safe context.
 *
 * If it was possible to recover from an error this function should return an empty value
 * `return EINA_VALUE_EMPTY;` or any other Eina_Value type that differs from EINA_VALUE_TYPE_ERROR.
 * In this case the error will not be reported by the other futures in the chain (if any), otherwise
 * if an Eina_Value type EINA_VALUE_TYPE_ERROR is returned the error will continue to be reported
 * to the other futures in the chain.
 *
 * @param data The data provided by the user.
 * @param error The operation error
 * @return An Eina_Value to pass to the next Eina_Future in the chain (if any).
 * If you need to convert to a different type or generate a new value,
 * use @c eina_value_setup() on @b another Eina_Value
 * and return it. By returning an promise Eina_Value (eina_promise_as_value()) the
 * whole chain will wait until the promise is resolved in
 * order to continue its execution.
 * Note that the value contents must survive this function scope,
 * that is, do @b not use stack allocated blobs, arrays, structures or types that
 * keeps references to memory you give. Values will be automatically cleaned up
 * using @c eina_value_flush() once they are unused (no more future or futures
 * returned a new value).
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 */
typedef Eina_Value (*Eina_Future_Error_Cb)(void *data, const Eina_Error error);

/**
 * @typedef Eina_Future_Free_Cb Eina_Future_Free_Cb.
 * @ingroup eina_future
 *
 * A callback used to inform that the future was freed and the user should also @c free the @p data.
 * This callback may be called from an unsafe context if the future was canceled or an error
 * occurred.
 *
 * @note This callback is always called, even if #Eina_Future_Error_Cb and/or #Eina_Future_Success_Cb
 * were not provided, which can also be used to monitor when a future ends.
 *
 * @param data The data provided by the user.
 * @param dead_future The future that was freed.
 *
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 */
typedef void (*Eina_Future_Free_Cb)(void *data, const Eina_Future *dead_future);

/**
 * @struct _Eina_Future_Cb_Easy_Desc
 * @ingroup eina_future
 *
 * A struct with callbacks to be used by eina_future_cb_easy_from_desc() and eina_future_cb_easy()
 *
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 */
struct _Eina_Future_Cb_Easy_Desc {
   /**
    * Called on success (value.type is not @c EINA_VALUE_TYPE_ERROR).
    *
    * if @c success_type is not NULL, then the value is guaranteed to be of that type,
    * if it's not, then it will trigger @c error with @c EINVAL.
    *
    * After this function returns, @c free callback is called if provided.
    */
   Eina_Future_Success_Cb success;
   /**
    * Called on error (value.type is @c EINA_VALUE_TYPE_ERROR).
    *
    * This function can return another error, propagating or converting it. However it
    * may also return a non-error, in this case the next future in chain will receive a regular
    * value, which may call its @c success.
    *
    * If this function is not provided, then it will pass thru the error to the next error handler.
    *
    * It may be called with @c EINVAL if @c success_type is provided and doesn't
    * match the received type.
    *
    * It may be called with @c ECANCELED if future was canceled.
    *
    * It may be called with @c ENOMEM if memory allocation failed during callback creation.
    *
    * After this function returns, @c free callback is called if provided.
    */
   Eina_Future_Error_Cb error;
   /**
    * Called on @b all situations to notify future destruction.
    *
    * This is called after @c success or @c error, as well as it's called if none of them are
    * provided. Thus can be used as a "weak ref" mechanism.
    */
   Eina_Future_Free_Cb free;
   /**
    * If provided, then @c success will only be called if the value type matches the given pointer.
    *
    * If provided and doesn't match, then @c error will be called with @c EINVAL. If no @c error,
    * then it will be propagated to the next future in the chain.
    */
   const Eina_Value_Type *success_type;
   /**
    * Context data given to every callback.
    *
    * This must be freed @b only by @c free callback as it's called from every case,
    * otherwise it may lead to memory leaks.
    */
   const void *data;
};

/**
 * @struct _Eina_Future_Cb_Console_Desc
 * @ingroup eina_future
 *
 * A struct used to define the prefix and suffix to be printed
 * along side the a future value. This struct is used by
 * eina_future_cb_console_from_desc()
 *
 * @see eina_future_cb_console_from_desc()
 */
struct _Eina_Future_Cb_Console_Desc {
   /**
    * The prefix to be printed. If @c NULL an empty string ("") is used.
    */
   const char *prefix;
   /**
    * The suffix the be printed. If @c NULL '\n' is used.
    */
   const char *suffix;
};

/**
 * @struct _Eina_Future_Cb_Log_Desc
 * @ingroup eina_future
 *
 * This struct is used by eina_future_cb_log_from_desc() and
 * its contents will be routed to eina_log_print() along side
 * the future value.
 *
 * @see eina_future_cb_log_from_desc()
 */
struct _Eina_Future_Cb_Log_Desc {
   /**
    * The prefix to be printed. If @c NULL an empty string ("") is used.
    */
   const char *prefix;
   /**
    * The suffix the be printed. If @c NULL '\n' is used.
    */
   const char *suffix;
   /**
    * The file name to be passed to eina_log_print(). if @c NULL "Unknown file" is used.
    */
   const char *file;
   /**
    * The file name to be passed to eina_log_print(). if @c NULL "Unknown function" is used.
    */
   const char *func;
   /**
    * The Eina_Log_Level to use.
    */
   Eina_Log_Level level;
   /**
    * The log domain to use.
    */
   int domain;
   /**
    * The line number to be passed to eina_log_print().
    */
   int line;
};

/**
 * @struct _Eina_Future_Desc
 * @ingroup eina_future
 * A struct used to define a callback and data for a future.
 *
 * This struct contains a future completion callback and a data to the future
 * completion callback which is used by eina_future_then(), eina_future_chain()
 * and friends to inform the user about the future result. The #_Eina_Future_Desc::data
 * variable should be freed when #_Eina_Future_Desc::cb is called, otherwise it will leak.
 *
 * @note If eina_future_then(), eina_future_chain() and friends fails to return a valid future
 * (in other words: @c NULL is returned) the #_Eina_Future_Desc::cb will be called
 * report an error like `EINVAL` or `ENOMEM` so #_Eina_Future_Desc::data can be freed.
 *
 * @see eina_future_then()
 * @see eina_future_chain_array()
 * @see eina_future_cb_convert_to()
 * @see eina_future_cb_console_from_desc()
 * @see eina_future_cb_ignore_error()
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_log_from_desc()
 */
struct _Eina_Future_Desc {
   /**
    * Called when the future is resolved or rejected.
    *
    * Once a future is resolved or rejected this function is called passing the future result
    * to inform the user that the future operation has ended. Normally this
    * function is called from a safe context (main loop or some platform defined safe context),
    * however in case of a future cancellation (eina_future_cancel()) or if eina_future_then(),
    * eina_future_chain() and friends fails to create a new future,
    * this function is called from the current context.
    *
    * Use this function to free @p data if necessary.
    * @see eina_future_chain()
    * @see eina_future_then()
    * @see eina_future_cancel()
    */
   Eina_Future_Cb cb;
   /**
    * Context data to @p cb. The @p data should be freed inside @p cb, otherwise
    * its memory will leak!
    */
   const void *data;

   /**
    * The storage will be used by Eina to store a pointer to the
    * created future. It can be @c NULL.
    */
   Eina_Future **storage;
};

/**
 * @}
 */

/**
 * @defgroup Eina_Promise Eina_Promise
 * Creates a new promise.
 *
 * This function creates a new promise which can be used to create a future
 * using eina_future_new(). Everytime a promise is created a #Eina_Promise_Cancel_Cb
 * must be provided which is used to free resources that were created.
 *
 * A promise may be cancelled directly by calling
 * @c eina_future_cancel(eina_future_new(eina_promise_new(...)))
 * that is, cancelling any future that is chained to receive the results.
 *
 * However promises can be cancelled indirectly by other entities.
 * These other entities will call `eina_future_cancel()` themselves,
 * however you may not be aware of that. Some common sources
 * of indirect cancellations:
 *
 * @li A subsystem was shutdown, cancelling all pending futures (ie: ecore_shutdown())
 *
 * @li An EO object was linked to the promise or future, then if the object dies (last reference
 * is gone), then the pending promises and futures will be cancelled.
 *
 * @li Some other entity (library provider or library user) chained and cancelled his future,
 * which will result in your future being cancelled.
 *
 * Since a promise may be canceled indirectaly (by code sections that goes beyond your scope)
 * you should always provide a cancel callback, even if you think you'll not need it.
 *
 * Below there's a typical example:
 *
 * @code
 * #include <Ecore.h>
 *
 * static void
 * _promise_cancel(void *data, Eina_Promise *p EINA_UNUSED)
 * {
 *   Ctx *ctx = data;
 *   //In case the promise is canceled we must stop the timer!
 *   ecore_timer_del(ctx->timer);
 *   free(ctx);
 * }
 *
 * static Eina_Bool
 * _promise_resolve(void *data)
 * {
 *    Ctx *ctx = data;
 *    Eina_Value v;
 *    eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
 *    eina_value_set(&v, "Promise resolved");
 *    eina_promise_resolve(ctx->p, v);
 *    free(ctx);
 *    return EINA_FALSE;
 * }
 *
 * Eina_Promise *
 * promise_create(Eina_Future_Scheduler *scheduler)
 * {
 *    Ctx *ctx = malloc(sizeof(Ctx));
 *    //A timer is scheduled in order to resolve the promise
 *    ctx->timer = ecore_timer_add(122, _promise_resolve, ctx);
 *    //The _promise_cancel() will be used to clean ctx if the promise is canceled.
 *    ctx->p = eina_promise_new(scheduler, _promise_cancel, ctx);
 *    return ctx->p;
 * }
 * @endcode
 *
 * @param cancel_cb A callback used to inform that the promise was canceled. Use
 * this callback to @c free @p data. @p cancel_cb must not be @c NULL !
 * @param data Data to @p cancel_cb.
 * @return A promise or @c NULL on error.
 * @see eina_future_cancel()
 * @see eina_future_new()
 * @see eina_promise_resolve()
 * @see eina_promise_reject()
 * @see eina_promise_data_get()
 * @see eina_promise_as_value()
 * @see #Eina_Future_Scheduler
 * @see #Eina_Future_Scheduler_Entry
 * @see #Eina_Future_Scheduler_Cb
 * @{
 */
EAPI Eina_Promise *eina_promise_new(Eina_Future_Scheduler *scheduler, Eina_Promise_Cancel_Cb cancel_cb, const void *data) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * Gets the data attached to the promise.
 *
 * @return The data passed to eina_promise_new() or @c NULL on error.
 * @see eina_promise_new()
 * @see eina_promise_resolve()
 * @see eina_promise_reject()
 * @see eina_promise_as_value()
 */
EAPI void *eina_promise_data_get(const Eina_Promise *p) EINA_ARG_NONNULL(1);

/**
 * Resolves a promise.
 *
 *
 * This function schedules an resolve event in a
 * safe context (main loop or some platform defined safe context),
 * whenever possible the future callbacks will be dispatched.
 *
 * @param p A promise to resolve.
 * @param value The value to be delivered. Note that the value contents must survive this function scope,
 * that is, do @b not use stack allocated blobs, arrays, structures or types that
 * keeps references to memory you give. Values will be automatically cleaned up
 * using @c eina_value_flush() once they are unused (no more future or futures
 * returned a new value).
 *
 * @see eina_promise_new()
 * @see eina_promise_reject()
 * @see eina_promise_data_get()
 * @see eina_promise_as_value()
 */
EAPI void eina_promise_resolve(Eina_Promise *p, Eina_Value value) EINA_ARG_NONNULL(1);
/**
 * Rejects a promise.
 *
 * This function schedules an reject event in a
 * safe context (main loop or some platform defined safe context),
 * whenever possible the future callbacks will be dispatched.
 *
 * @param p A promise to reject.
 * @param err An Eina_Error value
 * @note Internally this function will create an Eina_Value with type #EINA_VALUE_TYPE_ERROR.
 *
 * @see eina_promise_new()
 * @see eina_promise_resolve()
 * @see eina_promise_data_get()
 * @see eina_promise_as_value()
 */
EAPI void eina_promise_reject(Eina_Promise *p, Eina_Error err) EINA_ARG_NONNULL(1);


/**
 * @}
 */

/**
 * @defgroup Eina_Future Eina_Future
 * Cancels a future.
 *
 * This function will cancel the whole future chain immediately (it will not be schedule to the next mainloop pass)
 * and it will also cancel the promise linked against it. The Eina_Future_Cb will be called
 * with an Eina_Value typed as #EINA_VALUE_TYPE_ERROR, which its value will be
 * ECANCELED
 * @param f The future to cancel.
 * @{
 */
EAPI void eina_future_cancel(Eina_Future *f) EINA_ARG_NONNULL(1);

/**
 * Flushes an #Eina_Future_Desc
 *
 * This function is mainly used by bindings to flush a #Eina_Future_Desc.
 * It will call the #Eina_Future_Cb with `ENOMEM` and zero the @p desc contents.
 *
 * @param desc The #Eina_Future_Desc to flush, if @c NULL this is a noop.
 */
EAPI void eina_future_desc_flush(Eina_Future_Desc *desc);

/**
 * Flushes an #Eina_Future_Cb_Easy_Desc
 *
 * This function is mainly used by bindings to flush a #Eina_Future_Cb_Easy_Desc.
 * It will call the #Eina_Future_Error_Cb with `ENOMEM`, the #Eina_Future_Free_Cb and
 * zero the @p desc contents.
 *
 * @param desc The #Eina_Future_Cb_Easy_Desc to flush, if @c NULL this is a noop.
 */
EAPI void eina_future_cb_easy_desc_flush(Eina_Future_Cb_Easy_Desc *desc);

/**
 * Creates a new Eina_Value from a promise.
 *
 * This function creates a new Eina_Value that will store a promise
 * in it. This function is useful for dealing with promises inside
 * a #Eina_Future_Cb. By returning an Promise Eina_Value the
 * whole chain will wait until the promise is resolved in
 * order to continue its execution. Example:
 *
 * @code
 * static Eina_Value
 * _file_data_ready(const *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
 * {
 *    const char *file_data;
 *    Eina_Promise *p;
 *    //It was not possible to fetch the file size.
 *    if (v.type == EINA_VALUE_TYPE_ERROR)
 *    {
 *       Eina_Error err;
 *       eina_value_get(&v, &err);
 *       fprintf(stderr, "Could get the file data. Reason: %s\n", eina_error_msg_get(err));
 *       ecore_main_loop_quit();
 *       return v;
 *    }
 *    else if (v.type != EINA_VALUE_TYPE_STRING)
 *    {
 *      fprintf(stderr, "Expecting type '%s' - received '%s'\n", EINA_VALUE_TYPE_STRING->name, v.type->name);
 *      ecore_main_loop_quit();
 *      return v;
 *    }
 *
 *    eina_value_get(&v, &file_data);
 *    //count_words will count the words in the background and resolve the promise once it is over...
 *    p = count_words(file_data);
 *    return eina_promise_as_value(p);
 * }
 *
 * static Eina_Value
 * _word_count_ready(const *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
 * {
 *    //The _word_count_ready will only be called once count_words() resolves/rejects the promise returned by _file_data_ready()
 *    int count;
 *    if (v.type == EINA_VALUE_TYPE_ERROR)
 *    {
 *       Eina_Error err;
 *       eina_value_get(&v, &err);
 *       fprintf(stderr, "Could not count the file words. Reason: %s\n", eina_error_msg_get(err));
 *       ecore_main_loop_quit();
 *       return v;
 *    }
 *    else if (v.type != EINA_VALUE_TYPE_INT)
 *    {
 *      fprintf(stderr, "Expecting type '%s' - received '%s'\n", EINA_VALUE_TYPE_INT->name, v.type->name);
 *      ecore_main_loop_quit();
 *      return v;
 *    }
 *    eina_value_get(&v, &count);
 *    printf("File word count %d\n", count);
 *    return v;
 * }
 *
 * void
 * file_operation(void)
 * {
 *    Eina_Future *f = get_file_data("/MyFile.txt");
 *    eina_future_chain(f, {.cb = _file_data_ready, .data = NULL},
 *                         {.cb = _word_count_ready, .data = NULL});
 * }
 * @endcode
 *
 * @return An Eina_Value. On error the value's type will be @c NULL.
 * @note If an error happens the promise will be CANCELED.
 * @see eina_future_as_value()
 * @see eina_promise_reject()
 * @see eina_promise_resolve()
 */
EAPI Eina_Value eina_promise_as_value(Eina_Promise *p) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * Creates an Eina_Value from a future.
 *
 * This function is used for the same purposes as eina_promise_as_value(),
 * but receives an Eina_Future instead.
 *
 * @param f A future to create a Eina_Value from.
 * @return An Eina_Value. On error the value's type will be @c NULL.
 * @note If an error happens the future @p f will be CANCELED
 * @see eina_promise_as_value()
 */
EAPI Eina_Value eina_future_as_value(Eina_Future *f)EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * Creates a new future.
 *
 * This function creates a new future and can be used to report
 * that an operation has succeded or failed using
 * eina_promise_resolve() or eina_promise_reject().
 *
 * Futures can also be canceled using eina_future_cancel(), which
 * will cause the whole chain to be cancelled alongside with any pending promise.
 *
 * @note A promise can only have one future attached to it, calling
 * eina_future_new() on the same promise twice will
 * result in an error (@c NULL is returned) and the future
 * attached to the promise will be canceled!
 *
 * @param p A promise used to attach a future. May not be @c NULL.
 * @return The future or @c NULL on error.
 * @note If an error happens this function will CANCEL the promise.
 * @see eina_promise_new()
 * @see eina_promise_reject()
 * @see eina_promise_resolve()
 * @see eina_future_cancel()
 */
EAPI Eina_Future *eina_future_new(Eina_Promise *p) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * Register an Eina_Future_Desc to be used when the future is resolve/rejected.
 *
 * With this function a callback and data is attached to the future and then
 * once it's resolved or rejected the callback will be informed.
 *
 * If during the future creation an error happens this function will return @c NULL,
 * and the #Eina_Future_Desc::cb will be called reporting an error (`EINVAL` or `ENOMEM`),
 * so the user has a chance to free #Eina_Future_Desc::data.
 *
 * In case a future in the chain is canceled, the whole chain will be canceled immediately
 * and the error `ECANCELED` will be reported.
 *
 * Below there's a simple usage of this function.
 *
 * @code
 * static Eina_Value
 * _file_ready(const *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
 * {
 *    Ctx *ctx = data;
 *    //It was not possible to fetch the file size.
 *    if (v.type == EINA_VALUE_TYPE_ERROR)
 *    {
 *       Eina_Error err;
 *       eina_value_get(&v, &err);
 *       fprintf(stderr, "Could not read the file size. Reason: %s\n", eina_error_msg_get(err));
 *       ecore_main_loop_quit();
 *       return v;
 *    }
 *    else if (v.type != EINA_VALUE_TYPE_INT)
 *    {
 *      fprintf(stderr, "Expecting type '%s' - received '%s'\n", EINA_VALUE_TYPE_INT->name, v.type->name);
 *      ecore_main_loop_quit();
 *      return v;
 *    }
 *    eina_value_get(&v, &ctx->size);
 *    printf("File size is %d bytes\n", ctx->size);
 *    return v;
 * }
 *
 * void
 * file_operation(void)
 * {
 *    Eina_Future *f = get_file_size_async("/MyFile.txt");
 *    eina_future_then_from_desc(f, (const Eina_Future_Desc){.cb = _size_ready, .data = NULL});
 *    //There's a helper macro called eina_future_then() which simplifies the usage.
 *    //The code below has the same effect.
 *    //eina_future_then(f, _size_ready, NULL);
 * }
 * @endcode
 *
 * Although the code presented at `_size_ready()` is very simple, most of it
 * is just used to check the Eina_Value type. In order
 * to avoid this type of code the function eina_future_cb_easy_from_desc()
 * was created. Please, check its documentation for more information.
 *
 * This function can also be used to create a future chain, making
 * it possible to execute the future result in a cascade order. When
 * using a future chain the Eina_Value returned by a #Eina_Future_Desc::cb
 * will be delivered to the next #Eina_Future_Desc::cb in the chain.
 *
 * Here's an example:
 *
 * static Eina_Value
 * _future_cb1(const *data EINA_UNUSED, const Eina_Value v)
 * {
 *    Eina_Value new_v;
 *    int i;
 *
 *    //There's no need to check the Eina_Value type since we're using eina_future_cb_easy()
 *    eina_value_get(&v, &i);
 *    printf("File size as int: %d\n", i);
 *    eina_value_setup(&new_v, EINA_VALUE_TYPE_STRING);
 *    //Convert the file size to string
 *    eina_value_convert(&v, &new_v);
 *    return new_v;
 * }
 *
 * static Eina_Value
 * _future_cb2(const *data EINA_UNUSED, const Eina_Value v)
 * {
 *    Eina_Value new_v;
 *    const char *file_size_str;
 *
 *    //There's no need to check the Eina_Value type since we're using eina_future_cb_easy()
 *    eina_value_get(&v, &file_size_str);
 *    printf("File size as string: %s\n", file_size_str);
 *    eina_value_setup(&new_v, EINA_VALUE_TYPE_DOUBLE);
 *    eina_value_convert(&v, &new_v);
 *    return new_v;
 * }
 *
 * static Eina_Value
 * _future_cb3(const *data EINA_UNUSED, const Eina_Value v)
 * {
 *    double d;
 *
 *    //There's no need to check the Eina_Value type since we're using eina_future_cb_easy()
 *    eina_value_get(&v, &d);
 *    printf("File size as double: %g\n", d);
 *    return v;
 * }
 *
 * static Eina_Value
 * _future_err(void *data EINA_UNUSED, Eina_Error err)
 * {
 *    //This function is called if future result type does not match or another error occurred
 *    Eina_Value new_v;
 *    eina_value_setup(&new_v, EINA_VALUE_TYPE_ERROR);
 *    eina_valuse_set(&new_v, err);
 *    fprintf(stderr, "Error during future process. Reason: %s\n", eina_error_msg_get(err));
 *    //Pass the error to the next future in the chain..
 *    return new_v;
 * }
 *
 * @code
 * void chain(void)
 * {
 *   Eina_Future *f = get_file_size_async("/MyFile.txt");
 *   f = eina_future_then_easy(f, .success = _future_cb1, .success_type = EINA_VALUE_TYPE_INT);
 *   //_future_cb2 will be executed after _future_cb1()
 *   f = eina_future_then_easy(f, .success = _future_cb2, .success_type = EINA_VALUE_TYPE_STRING);
 *   //_future_cb2 will be executed after _future_cb2()
 *   f = eina_future_then_easy(f, .success = _future_cb3, .success_type = EINA_VALUE_TYPE_DOUBLE);
 *   //If an error happens _future_err will be called
 *   eina_future_then_easy(f, .error = _future_err);
 * }
 * @endcode
 *
 * Although it's possible to create a future chain using eina_future_then()/eina_future_then_from_desc()
 * there's a function that makes this task much easier, please check eina_future_chain_array() for more
 * information.
 * @note This example does manual conversion and print, however we offer
 * eina_future_cb_convert_to() and eina_future_cb_console_from_desc() and to make those common case easier.
 *
 * @param prev A future to link against
 * @param desc A description struct contaning the callback and data.
 * @return A new future or @c NULL on error.
 * @note If an error happens the whole future chain will CANCELED and
 * desc.cb will be called in order to free desc.data.
 * @see eina_future_new()
 * @see eina_future_then()
 * @see #Eina_Future_Desc
 * @see eina_future_chain_array()
 * @see eina_future_chain()
 * @see eina_future_cb_console_from_desc()
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 * @see eina_future_cb_convert_to()
 * @see eina_future_cancel()
 * @see eina_future_then_easy()
 * @see eina_future_cb_log_from_desc()
 */
EAPI Eina_Future *eina_future_then_from_desc(Eina_Future *prev, const Eina_Future_Desc desc) EINA_ARG_NONNULL(1);


/**
 * Creates an Eina_Future_Desc that will log the previous future resolved value.
 *
 * This function can be used to quickly log the value that an #Eina_Future_Desc::cb
 * is returning. The returned value will be passed to the next future in the chain without
 * modifications.
 *
 * There're some helper macros like eina_future_cb_log_dbg() which will automatically
 * fill the following fields:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_DBG will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 *
 * @param desc The description data to be used.
 * @return An #Eina_Future_Desc
 *
 * @see #Eina_Future_Cb_Log_Desc
 * @see efl_future_then()
 * @see efl_future_chain()
 * @see eina_future_cb_log_dbg()
 * @see eina_future_cb_log_crit()
 * @see eina_future_cb_log_err()
 * @see eina_future_cb_log_info()
 * @see eina_future_cb_log_warn()
 * @see eina_future_cb_console_from_desc()
 */
EAPI Eina_Future_Desc eina_future_cb_log_from_desc(const Eina_Future_Cb_Log_Desc desc) EINA_WARN_UNUSED_RESULT;

/**
 * Creates a future chain.
 *
 *
 * This behaves exactly like eina_future_then_from_desc(), but makes it easier to create future chains.
 *
 * If during the future chain creation an error happens this function will return @c NULL,
 * and the #Eina_Future_Desc::cb from the @p descs array will be called reporting an error (`EINVAL` or `ENOMEM`),
 * so the user has a chance to free #Eina_Future_Desc::data.
 *
 * Just like eina_future_then_from_desc(), the value returned by a #Eina_Future_Desc::cb callback will
 * be delivered to the next #Eina_Future_Desc::cb in the chain.
 *
 * In case a future in the chain is canceled, the whole chain will be canceled immediately
 * and the error `ECANCELED` will be reported.
 *
 * Here's an example:
 *
 * @code
 *
 * //callbacks code....
 *
 * Eina_Future* chain(void)
 * {
 *   Eina_Future *f = get_file_size_async("/MyFile.txt");
 *   return eina_future_chain(f, eina_future_cb_easy(_future_cb1, _future_err, NULL, EINA_VALUE_TYPE_INT, NULL),
 *                               eina_future_cb_easy(_future_cb2, _future_err, NULL, EINA_VALUE_TYPE_STRING, NULL),
 *                               eina_future_cb_easy(_future_cb3, _future_err, NULL, EINA_VALUE_TYPE_DOUBLE, NULL),
 *                               {.cb = _future_cb4, .data = NULL });
 * }
 * @endcode
 *
 * @param prev The previous future
 * @param descs An array of descriptions. The last element of the array must have the #Eina_Future_Desc::cb set to @c NULL
 * @return A future or @c NULL on error.
 * @note If an error happens the whole future chain will CANCELED and
 * desc.cb will be called in order to free desc.data.
 * @see eina_future_new()
 * @see eina_future_then()
 * @see #Eina_Future_Desc
 * @see eina_future_chain()
 * @see eina_future_cb_ignore_error()
 * @see eina_future_cb_console_from_desc()
 * @see eina_future_cb_log_from_desc()
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 * @see eina_future_then_from_desc()
 * @see eina_future_then_easy()
 * @see eina_future_cb_convert_to()
 */
EAPI Eina_Future *eina_future_chain_array(Eina_Future *prev, const Eina_Future_Desc descs[]) EINA_ARG_NONNULL(1, 2);


/**
 *
 * Wrappper around eina_future_chain_array() and eina_future_cb_easy_from_desc()
 *
 * This functions makes it easier to use  eina_future_chain_array() with eina_future_cb_easy_from_desc(),
 * check the macro eina_future_chain_easy() for an syntax sugar.
 *
 *
 * @param prev The previous future
 * @param descs An array of descriptions. The last element of the array must have the #Eina_Future_Desc::cb set to @c NULL
 * @return A future or @c NULL on error.
 * @note If an error happens the whole future chain will CANCELED and
 * desc.cb will be called in order to free desc.data.
 * @see eina_future_chain_easy()
 * @see eina_future_chain()
 * @see eina_future_cb_easy()
 * @see eina_future_chain_array()
 * @see eina_future_cb_easy_from_desc()
 */
EAPI Eina_Future *eina_future_chain_easy_array(Eina_Future *prev, const Eina_Future_Cb_Easy_Desc descs[]) EINA_ARG_NONNULL(1, 2);

/**
 * Creates an Eina_Future_Desc that will print the previous future resolved value.
 *
 * This function can be used to quickly inspect the value that an #Eina_Future_Desc::cb
 * is returning. The returned value will be passed to the next future in the chain without
 * modifications.
 *
 * There's also an helper macro called eina_future_cb_console() which makes this
 * fuction easier to use.
 *
 * Example:
 *
 * @code
 *
 * eina_future_chain(a_future, {.cb = cb1, .data = NULL},
 *                             //Inspect the cb1 value and pass to cb2 without modifications
 *                             eina_future_cb_console("cb1 value:"),
 *                             {.cb = cb2, .data = NULL},
 *                             //Inspect the cb2 value
 *                             eina_future_cb_console("cb2 value:", " cb2 value suffix\n"))
 * @endcode
 *
 * @param prefix A Prefix to print, if @c NULL an empty string ("") is used.
 * @param suffix A suffix to print. If @c NULL '\n' will be used. If suffix is provided
 * the '\n' must be provided by suffix otherwise the printed text will not contain
 * a line feed.
 * @return An #Eina_Future_Desc
 * @see eina_future_then()
 * @see #Eina_Future_Desc
 * @see eina_future_chain()
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 * @see eina_future_then_from_desc()
 * @see eina_future_then_easy()
 * @see eina_future_cb_console()
 * @see eina_future_cb_ignore_error()
 * @see eina_future_cb_log_from_desc()
 */
EAPI Eina_Future_Desc eina_future_cb_console_from_desc(const Eina_Future_Cb_Console_Desc desc) EINA_WARN_UNUSED_RESULT;

/**
 * Returns a #Eina_Future_Desc that ignores an error.
 *
 * This function may be used if one wants to ignore an error. If the error
 * specified error happens an EINA_VALUE_EMPTY will be delivered to the
 * next future in the chain.
 *
 * @param err The error to be ignored.
 * @param A future descriptor to be used with eina_future_then() or eina_future_chain()
 */
EAPI Eina_Future_Desc eina_future_cb_ignore_error(Eina_Error err);

/**
 * Creates an #Eina_Future_Desc which will convert the the received eina value to a given type.
 *
 * @param type The Eina_Value_Type to convert to.
 * @return An #Eina_Future_Desc
 * @see eina_future_then()
 * @see #Eina_Future_Desc
 * @see eina_future_chain()
 * @see eina_future_cb_easy_from_desc()
 * @see eina_future_cb_easy()
 * @see eina_future_then_from_desc()
 * @see eina_future_then_easy()
 */
EAPI Eina_Future_Desc eina_future_cb_convert_to(const Eina_Value_Type *type);

/**
 * Creates an #Eina_Future_Desc based on a #Eina_Future_Cb_Easy_Desc
 *
 * This function aims to be used in conjuction with eina_future_chain(),
 * eina_future_then_from_desc() and friends and its main objective is to simplify
 * error handling and Eina_Value type checks.
 * It uses three callbacks to inform the user about the future's
 * result and life cycle. They are:
 *
 * @li #Eina_Future_Cb_Easy_Desc::success: This callback is called when
 * the future execution was successful, this is, no errors occurred and
 * the result type matches #Eina_Future_Cb_Easy_Desc::success_type. In case
 * #Eina_Future_Cb_Easy_Desc::success_type is @c NULL, this function will
 * only be called if the future did not report an error. The value returned
 * by this function will be propagated to the next future in the chain (if any).
 *
 * @li #Eina_Future_Cb_Easy_Desc::error: This callback is called when
 * the future result is an error or #Eina_Future_Cb_Easy_Desc::success_type
 * does not match the future result type. The value returned
 * by this function will be propagated to the next future in the chain (if any).
 *
 * @li #Eina_Future_Cb_Easy_Desc::free: Called after the future was freed and any resources
 * allocated must be freed at this point. This callback is always called.
 *
 * Check the example below for a sample usage:
 *
 * @code
 * static Eina_Value
 * _file_size_ok(void *data, Eina_Value v)
 * {
 *   Ctx *ctx = data;
 *   //Since an Eina_Future_Cb_Easy_Desc::success_type was provided, there's no need to check the value type
 *   int s;
 *   eina_value_get(&v, &s);
 *   printf("File size is %d bytes\n", s);
 *   ctx->file_size = s;
 *   return v;
 * }
 *
 * static Eina_Value
 * _file_size_err(void *data, Eina_Error err)
 * {
 *   fprintf(stderr, "Could not read the file size. Reason: %s\n", eina_error_msg_get(err));
 *   //Stop propagating the error.
 *   return EINA_VALUE_EMPTY;
 * }
 *
 * static void
 * _future_freed(void *data, const Eina_Future dead)
 * {
 *   Ctx *ctx = data;
 *   printf("Future %p deleted\n", dead);
 *   ctx->file_size_handler_cb(ctx->file_size);
 *   free(ctx);
 * }
 *
 * @code
 * void do_work(File_Size_Handler_Cb cb)
 * {
 *   Ctx *ctx = malloc(sizeof(Ctx));
 *   ctx->f = get_file_size("/tmp/todo.txt");
 *   ctx->file_size = -1;
 *   ctx->file_size_handler_cb = cb;
 *   eina_future_then_easy(f, _file_size_ok, _file_size_err, _future_freed, EINA_VALUE_TYPE_INT, ctx);
 * }
 * @endcode
 *
 * @return An #Eina_Future_Desc
 * @see eina_future_chain()
 * @see eina_future_then()
 * @see eina_future_cb_easy()
 */
EAPI Eina_Future_Desc eina_future_cb_easy_from_desc(const Eina_Future_Cb_Easy_Desc desc) EINA_WARN_UNUSED_RESULT;
/**
 * Creates an all promise.
 *
 * Creates a promise that is resolved once all the futures
 * from the @p array are resolved.
 * The promise is resolved with an Eina_Value type array which
 * contains EINA_VALUE_TYPE_VALUE elements. The result array is
 * ordered according to the @p array argument. Example:
 *
 * @code
 *
 * static const char *
 * _get_operation_name_by_index(int idx)
 * {
 *   switch (idx)
 *   {
 *      case 0: return "Get file data";
 *      case 1: return "Get file size";
 *      default: return "sum";
 *   }
 * }
 *
 * static Eina_Value
 * _all_cb(const void *data EINA_UNUSED, const Eina_Value array, const Eina_Future *dead EINA_UNUSED)
 * {
 *    Eina_Error err;
 *    unsined int i, len;
 *
 *    if (v.type == EINA_VALUE_TYPE_ERROR)
 *     {
 *       eina_value_get(&array, &err);
 *       fprintf(stderr, "Could not complete all operations. Reason: %s\n", eina_error_msg_get(err));
 *       return array;
 *     }
 *    len = eina_value_array_count(&array);
 *    for (i = 0; i < len; i++)
 *     {
 *       Eina_Value v;
 *       eina_value_array_get(&array, i, &v);
 *       if (v.type == EINA_VALUE_TYPE_ERROR)
 *        {
 *          eina_value_get(&array, &err);
 *          fprintf(stderr, "Could not complete operation '%s'. Reason: %s\n", _get_operation_name_by_index(i), eina_error_msg_get(err));
 *          continue;
 *        }
 *       if (!i)
 *        {
 *           const char *msg;
 *           if (v.type != EINA_VALUE_TYPE_STRING)
 *           {
 *             fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_STRING->name, v.type->name);
 *             continue;
 *           }
 *           eina_value_get(&v, &msg);
 *           printf("File content:%s\n", msg);
 *        }
 *       else if (i == 1)
 *        {
 *           int i;
 *           if (v.type != EINA_VALUE_TYPE_INT)
 *           {
 *             fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_INT->name, v.type->name);
 *             continue;
 *           }
 *           eina_value_get(&v, &i);
 *           printf("File size: %d\n", i);
 *        }
 *        else
 *        {
 *           double p;
 *           if (v.type != EINA_VALUE_TYPE_DOUBLE)
 *           {
 *             fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_DOUBLE->name, v.type->name);
 *             continue;
 *           }
 *           eina_value_get(&v, &p);
 *           printf("50 places of PI: %f\n", p);
 *        }
 *     }
 *    return array;
 * }
 *
 * void func(void)
 * {
 *   Eina_Future *f1, *f2, *f3, f_all;
 *
 *   f1 = read_file("/tmp/todo.txt");
 *   f2 = get_file_size("/tmp/file.txt");
 *   //calculates 50 places of PI
 *   f3 = calc_pi(50);
 *   f_all = eina_future_all(f1, f2, f3);
 *   eina_future_then(f_all, _all_cb, NULL);
 * }
 * @endcode
 *
 * @param array An array of futures, @c #EINA_FUTURE_SENTINEL terminated.
 * @return A promise or @c NULL on error.
 * @note On error all the futures will be CANCELED.
 * @see eina_future_all_array()
 */
EAPI Eina_Promise *eina_promise_all_array(Eina_Future *array[]) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * Creates a race promise.
 *
 * Creates a promise that resolves when a future from the @p array
 * is completed. The remaining futures will be canceled with the
 * error code `ECANCELED`
 *
 * The resulting value is an EINA_VALUE_TYPE_STRUCT with two fields:
 *
 * @li A field named "value" which contains an Eina_Value with the result itself.
 * @li A field named "index" which is an int that contains the index of the completed
 * function relative to the @p array;
 *
 * Example.
 *
 * @code
 *
 * static const char *
 * _get_operation_name_by_index(int idx)
 * {
 *   switch (idx)
 *   {
 *      case 0: return "Get file data";
 *      case 1: return "Get file size";
 *      default: return "sum";
 *   }
 * }
 *
 * static Eina_Value
 * _race_cb(const void *data EINA_UNUSED, const Eina_Value v)
 * {
 *    unsigned int i;
 *    Eina_Value result;
 *    Eina_Error err;
 *    Eina_Value_Struct *st;
 *
 *     //No need to check for the 'v' type. eina_future_cb_easy() did that for us,
 *     //However we should check if the struct has the correct description
 *     st = eina_value_memory_get(&v);
 *     if (st->desc != EINA_PROMISE_RACE_STRUCT_DESC)
 *      {
 *         fprintf(stderr, "Eina_Value is not a race struct\n");
 *         return v;
 *      }
 *     eina_value_struct_get(&v, "index", &i);
 *     //Get the operation result
 *     eina_value_struct_get(&v, "value", &result);
 *     if (!i)
 *      {
 *        const char *msg;
 *        if (result.type != EINA_VALUE_TYPE_STRING)
 *          fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_STRING->name, result.type->name);
 *        else
 *        {
 *          eina_value_get(&result, &msg);
 *          printf("File content:%s\n", msg);
 *        }
 *      }
 *     else if (i == 1)
 *       {
 *         int i;
 *         if (result.type != EINA_VALUE_TYPE_INT)
 *           fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_INT->name, v.type->name);
 *         else
 *         {
 *           eina_value_get(&result, &i);
 *           printf("File size: %d\n", i);
 *         }
 *       }
 *      else
 *       {
 *         double p;
 *         if (result.type != EINA_VALUE_TYPE_DOUBLE)
 *            fprintf(stderr, "Operation %s expects '%s' - received '%s'\n", _get_operation_name_by_index(i), EINA_VALUE_TYPE_DOUBLE->name, result.type->name);
 *         else
 *          {
 *            eina_value_get(&result, &p);
 *            printf("50 places of PI: %f\n", p);
 *          }
 *       }
 *     eina_value_flush(&result);
 *     return v;
 * }
 *
 * static Eina_Value
 * _race_err(void *data, Eina_Error err)
 * {
 *    fprintf(stderr, "Could not complete the race future. Reason: %s\n", eina_error_msg_get(err));
 *    return EINA_VALUE_EMPTY;
 * }
 *
 * void func(void)
 * {
 *   static const *Eina_Future[] = {NULL, NULL, NULL, NULL};
 *   Eina_List *l = NULL;
 *
 *   futures[0] = read_file("/tmp/todo.txt");
 *   futures[1] = get_file_size("/tmp/file.txt");
 *   //calculates 50 places of PI
 *   futures[2] = calc_pi(50);
 *   f_race = eina_future_race_array(futures);
 *   eina_future_then_easy(f_race, _race_cb, _race_err, NULL, EINA_VALUE_TYPE_STRUCT, NULL);
 * }
 * @endcode
 *
 * @param array An array of futures, @c #EINA_FUTURE_SENTINEL terminated.
 * @return A promise or @c NULL on error.
 * @note On error all the futures will be CANCELED.
 * @see eina_future_race_array()
 * @see #_Eina_Future_Race_Result
 */
EAPI Eina_Promise *eina_promise_race_array(Eina_Future *array[]) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @struct The struct that is used to store the race result.
 *
 * When using eina_promise_race_array() and friends, the future result
 * will be reported as a struct. The values can be obtained using
 * eina_value_struct_get() or one could access the struct directly
 * such as this example:
 *
 * @code
 * static Eina_Value
 * _race_cb(const void *data EINA_UNUSED, const Eina_Value v)
 * {
 *    //code...
 *    Eina_Value_Struct st;
 *    Eina_Future_Race_Result *rr;
 *    eina_value_get(v, &st);
 *    rr = st.memory;
 *    printf("Winning future index: %u\n", rr->index);
 *    //more code..
 *    return v;
 * }
 * @endcode
 *
 * @see eina_promise_race_array()
 * @see eina_future_race_array()
 * @see eina_promise_race()
 * @see eina_future_race()
 * @see #EINA_PROMISE_RACE_STRUCT_DESC
 */
struct _Eina_Future_Race_Result {
   /**
    * The race result.
    */
   Eina_Value value;
   /**
    * The future index that won the race.
    */
   unsigned int index;
};

/**
 * @var EINA_PROMISE_RACE_STRUCT_DESC
 *
 * A pointer to the race struct description, which
 * is used by eina_promise_race_array();
 *
 * This struct contains two members:
 * @li value An EINA_VALUE_TYPE_VALUE that contains the future result that wont the race.
 * @li index An EINA_VALUE_TYPE_UINT that contains the future index that won the race.
 *
 * @see eina_promise_race_array()
 * @see #_Eina_Future_Race_Result
 */
EAPI extern const Eina_Value_Struct_Desc *EINA_PROMISE_RACE_STRUCT_DESC;

/**
 * Creates a future that will be resolved once all futures from @p array is resolved.
 * This is a helper over eina_promise_all_array()
 *
 * @param array A future array, must be terminated with #EINA_FUTURE_SENTINEL
 * @return A future.
 * @see eina_promise_all_array()
 * @see #EINA_FUTURE_SENTINEL
 */
static inline Eina_Future *
eina_future_all_array(Eina_Future *array[])
{
   Eina_Promise *p = eina_promise_all_array(array);
   if (!p) return NULL;
   return eina_future_new(p);
}

/**
 * Creates a future that will be resolved once a future @p array is resolved.
 * This is a helper over eina_promise_race_array()
 *
 * @param array A future array, must be terminated with #EINA_FUTURE_SENTINEL
 * @return A future.
 * @see eina_promise_race_array()
 * @see #EINA_FUTURE_SENTINEL
 */
static inline Eina_Future *
eina_future_race_array(Eina_Future *array[])
{
   Eina_Promise *p = eina_promise_race_array(array);
   if (!p) return NULL;
   return eina_future_new(p);
}

/**
 * Used by eina_promise_race_array() and eina_promise_all_array() and
 * friends to flag the end of the array.
 *
 * @see eina_promise_race_array()
 * @see eina_promise_all_array()
 */
#define EINA_FUTURE_SENTINEL ((void *)(unsigned long)-1)

/**
 * A syntatic sugar over eina_promise_race_array().
 * Usage:
 * @code
 * promise = eina_promise_race(future1, future2, future3, future4);
 * @endcode
 * @see eina_promise_race_array()
 */
#define eina_promise_race(...) eina_promise_race_array((Eina_Future *[]){__VA_ARGS__, EINA_FUTURE_SENTINEL})
/**
 * A syntatic sugar over eina_future_race_array().
 * Usage:
 * @code
 * future = eina_future_race(future1, future2, future3, future4);
 * @endcode
 * @see eina_future_racec_array()
 */
#define eina_future_race(...) eina_future_race_array((Eina_Future *[]){__VA_ARGS__, EINA_FUTURE_SENTINEL})
/**
 * A syntatic sugar over eina_future_all_array().
 * Usage:
 * @code
 * future = eina_future_all(future1, future2, future3, future4);
 * @endcode
 * @see eina_future_all_array()
 */
#define eina_future_all(...) eina_future_all_array((Eina_Future *[]){__VA_ARGS__, EINA_FUTURE_SENTINEL})
/**
 * A syntatic sugar over eina_promise_all_array().
 * Usage:
 * @code
 * promise = eina_promise_all(future1, future2, future3, future4);
 * @endcode
 * @see eina_promise_all_array()
 */
#define eina_promise_all(...) eina_promise_all_array((Eina_Future *[]){__VA_ARGS__, EINA_FUTURE_SENTINEL})
/**
 * A syntatic sugar over eina_future_cb_easy_from_desc().
 * Usage:
 * @code
 * future_desc = eina_future_cb_easy(_success_cb, _error_cb, _free_cb, EINA_VALUE_TYPE_INT, my_data);
 * @endcode
 * @see eina_future_cb_easy_from_desc()
 */
#define eina_future_cb_easy(...) eina_future_cb_easy_from_desc((Eina_Future_Cb_Easy_Desc){__VA_ARGS__})
/**
 * A syntatic sugar over eina_future_chain_array().
 * Usage:
 * @code
 * future = eina_future_chain(future, {.cb = _my_cb, .data = my_data}, {.cb = _my_another_cb, .data = NULL});
 * @endcode
 * @see eina_future_chain_array()
 */
#define eina_future_chain(_prev, ...) eina_future_chain_array(_prev, (Eina_Future_Desc[]){__VA_ARGS__, {.cb = NULL, .data = NULL}})
/**
 * A syntatic sugar over eina_future_then_from_desc().
 * Usage:
 * @code
 * future = eina_future_then(future, _my_cb, my_data);
 * @endcode
 * @see eina_future_then_from_desc()
 * @see eina_future_then_easy()
 */
#define eina_future_then(_prev, ...) eina_future_then_from_desc(_prev, (Eina_Future_Desc){__VA_ARGS__})
/**
 * A syntatic sugar over eina_future_cb_console_from_desc().
 * Usage:
 * @code
 * desc = eina_future_cb_console(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_console_from_desc()
 */
#define eina_future_cb_console(...) eina_future_cb_console_from_desc((Eina_Future_Cb_Console_Desc){__VA_ARGS__})

/**
 * A syntatic sugar over eina_future_cb_log_from_desc().
 *
 * This macro will set the following fields of the #Eina_Future_Cb_Log_Desc:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_DBG will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 * Usage:
 * @code
 * desc = eina_future_cb_log_dbg(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_log_from_desc()
 */
#define eina_future_cb_log_dbg(_prefix, _suffix)                        \
  eina_future_cb_log_from_desc((Eina_Future_Cb_Log_Desc){_prefix, _suffix, __FILE__, \
         __FUNCTION__, EINA_LOG_LEVEL_DBG, EINA_LOG_DOMAIN_DEFAULT, __LINE__})

/**
 * A syntatic sugar over eina_future_cb_log_from_desc().
 *
 * This macro will set the following fields of the #Eina_Future_Cb_Log_Desc:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_CRITICAL will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 * Usage:
 * @code
 * desc = eina_future_cb_log_crit(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_log_from_desc()
 */
#define eina_future_cb_log_crit(_prefix, _suffix)                       \
  eina_future_cb_log_from_desc((Eina_Future_Cb_Log_Desc){_prefix, _suffix, __FILE__, \
         __FUNCTION__, EINA_LOG_LEVEL_CRITICAL, EINA_LOG_DOMAIN_DEFAULT, __LINE__})

/**
 * A syntatic sugar over eina_future_cb_log_from_desc().
 *
 * This macro will set the following fields of the #Eina_Future_Cb_Log_Desc:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_ERR will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 * Usage:
 * @code
 * desc = eina_future_cb_log_err(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_log_from_desc()
 */
#define eina_future_cb_log_err(_prefix, _suffix)                        \
  eina_future_cb_log_from_desc((Eina_Future_Cb_Log_Desc){_prefix, _suffix, __FILE__, \
         __FUNCTION__, EINA_LOG_LEVEL_ERR, EINA_LOG_DOMAIN_DEFAULT, __LINE__})

/**
 * A syntatic sugar over eina_future_cb_log_from_desc().
 *
 * This macro will set the following fields of the #Eina_Future_Cb_Log_Desc:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_INFO will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 *
 * Usage:
 * @code
 * desc = eina_future_cb_log_info(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_log_from_desc()
 */
#define eina_future_cb_log_info(_prefix, _suffix)                       \
  eina_future_cb_log_from_desc((Eina_Future_Cb_Log_Desc){_prefix, _suffix, __FILE__, \
         __FUNCTION__, EINA_LOG_LEVEL_INFO, EINA_LOG_DOMAIN_DEFAULT, __LINE__})

/**
 * A syntatic sugar over eina_future_cb_log_from_desc().
 *
 * This macro will set the following fields of the #Eina_Future_Cb_Log_Desc:
 *
 * @li #Eina_Future_Cb_Log_Desc::file: The __FILE__ function will be used.
 * @li #Eina_Future_Cb_Log_Desc::func: The __FUNCTION__ macro will be used.
 * @li #Eina_Future_Cb_Log_Desc::level: EINA_LOG_LEVEL_WARN will be used.
 * @li #Eina_Future_Cb_Log_Desc::domain: EINA_LOG_DOMAIN_DEFAULT will be used.
 * @li #Eina_Future_Cb_Log_Desc::line: The __LINE__ macro will be used.
 *
 * Usage:
 * @code
 * desc = eina_future_cb_log_warn(.prefix = "prefix", .suffix = "suffix");
 * @endcode
 * @see eina_future_cb_log_from_desc()
 */
#define eina_future_cb_log_warn(_prefix, _suffix)                       \
  eina_future_cb_log_from_desc((Eina_Future_Cb_Log_Desc){_prefix, _suffix, __FILE__, \
         __FUNCTION__, EINA_LOG_LEVEL_WARN, EINA_LOG_DOMAIN_DEFAULT, __LINE__})

/**
 * A syntatic sugar over eina_future_then() and eina_future_cb_easy().
 * Usage:
 * @code
 * f = eina_future_then_easy(f, .success = _success_cb, .success_type = EINA_VALUE_TYPE_DOUBLE, .data = NULL, );
 * @endcode
 * @see eina_future_then_from_desc()
 * @see eina_future_easy()
 * @see eina_future_then()
 * @see eina_future_cb_easy_from_desc()
 */
#define eina_future_then_easy(_prev, ...) eina_future_then_from_desc(_prev, eina_future_cb_easy(__VA_ARGS__))

/**
 * A syntatic sugar over eina_future_chain() and eina_future_cb_easy().
 * Usage:
 * @code
 * f = eina_future_chain_easy(f, {.success = _success_cb, .success_type = EINA_VALUE_TYPE_DOUBLE, .data = NULL},
 *                               { .success = _success2_cb }, {.error = error_cb});
 * @endcode
 * @see eina_future_chain_array()
 * @see eina_future_easy()
 * @see eina_future_chain_easy_array()
 * @see eina_future_cb_easy_from_desc()
 */
#define eina_future_chain_easy(_prev, ...) eina_future_chain_easy_array(_prev, (Eina_Future_Cb_Easy_Desc[]) {__VA_ARGS__, {NULL, NULL, NULL, NULL, NULL}})


/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif
