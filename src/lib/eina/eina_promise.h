#ifndef EINA_PROMISE_H_
#define EINA_PROMISE_H_

#ifdef EFL_BETA_API_SUPPORT

/*
 * @def Eina_Promise
 */
typedef struct _Eina_Promise Eina_Promise;

/*
 * @def Eina_Promise_VTable
 */
typedef struct _Eina_Promise_VTable Eina_Promise_VTable;

/*
 * @def Eina_Promise
 */
typedef struct _Eina_Promise_Owner Eina_Promise_Owner;

/*
 * @def Eina_Promise
 */
typedef struct _Eina_Promise_Owner_VTable Eina_Promise_Owner_VTable;

/*
 * @brief Callback type for freeing Promise value and void* pointer data for callbacks
 */
typedef void(*Eina_Promise_Free_Cb)(void* value);

/*
 * @brief Callback type for Promise_Owner to get notified of when someone registered a progress and/or then callback
 */
typedef void(*Eina_Promise_Progress_Notify_Cb)(void* data, Eina_Promise_Owner* promise);

/*
 * @brief Function callback type for when using eina_promise_then
 */
typedef void(*Eina_Promise_Cb)(void* data, void* value);

/*
 * @brief Function callback type for when using eina_promise_then
 */
typedef void(*Eina_Promise_Error_Cb)(void* data, Eina_Error error);

/*
 * @brief Function callback type for progress information
 */
typedef void(*Eina_Promise_Progress_Cb)(void* data, void* value);

/*
 * @brief Function callback type for then function override
 */
typedef void(*Eina_Promise_Then_Cb)(Eina_Promise_VTable* promise, Eina_Promise_Cb callback,
                                    Eina_Promise_Error_Cb error_cb, void* data);

#define EINA_FUNC_PROMISE_THEN(Function) ((Eina_Promise_Then_Cb)Function)

/*
 * @brief Function callback type for promise's value_get function override
 */
typedef void*(*Eina_Promise_Value_Get_Cb)(Eina_Promise_VTable const* promise);

#define EINA_FUNC_PROMISE_VALUE_GET(Function) ((Eina_Promise_Value_Get_Cb)Function)

/*
 * @brief Function callback type for promise's error_get function override
 */
typedef Eina_Error(*Eina_Promise_Error_Get_Cb)(Eina_Promise_VTable const* promise);

#define EINA_FUNC_PROMISE_ERROR_GET(Function) ((Eina_Promise_Error_Get_Cb)Function)

/*
 * @brief Function callback type for promise's pending function override
 */
typedef Eina_Bool(*Eina_Promise_Pending_Is_Cb)(Eina_Promise_VTable const* promise);

#define EINA_FUNC_PROMISE_PENDING_IS(Function) ((Eina_Promise_Pending_Is_Cb)Function)

/*
 * @brief Function callback type for promise's progress add function override
 */
typedef void(*Eina_Promise_Progress_Cb_Add_Cb)(Eina_Promise_VTable* promise, Eina_Promise_Progress_Cb callback, void* data
                                               , Eina_Promise_Free_Cb free_cb);

#define EINA_FUNC_PROMISE_PROGRESS_CB_ADD(Function) ((Eina_Promise_Progress_Cb_Add_Cb)Function)

/*
 * @brief Function callback type for promise's cancel function override
 */
typedef void(*Eina_Promise_Cancel_Cb)(Eina_Promise_VTable* promise);

#define EINA_FUNC_PROMISE_CANCEL(Function) ((Eina_Promise_Cancel_Cb)Function)

/*
 * @brief Function callback type for promise's ref function override
 */
typedef void(*Eina_Promise_Ref_Cb)(Eina_Promise_VTable* promise);

#define EINA_FUNC_PROMISE_REF(Function) ((Eina_Promise_Ref_Cb)Function)

/*
 * @brief Function callback type for promise's unref function override
 */
typedef void(*Eina_Promise_Unref_Cb)(Eina_Promise_VTable* promise);

#define EINA_FUNC_PROMISE_UNREF(Function) ((Eina_Promise_Unref_Cb)Function)

/*
 * @brief Function callback type for promise's ref_get function override
 */
typedef int(*Eina_Promise_Ref_Get_Cb)(Eina_Promise_VTable* promise);

#define EINA_FUNC_PROMISE_REF_GET(Function) ((Eina_Promise_Ref_Get_Cb)Function)

/*
 * @brief Function callback type for promise's buffer_get function override
 */
typedef void*(*Eina_Promise_Buffer_Get_Cb)(Eina_Promise_VTable* promise);

#define EINA_FUNC_PROMISE_BUFFER_GET(Function) ((Eina_Promise_Buffer_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's buffer_get function override
 */
typedef void*(*Eina_Promise_Owner_Buffer_Get_Cb)(Eina_Promise_Owner_VTable* promise);

#define EINA_FUNC_PROMISE_OWNER_BUFFER_GET(Function) ((Eina_Promise_Owner_Buffer_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's promise_get function override
 */
typedef void*(*Eina_Promise_Owner_Promise_Get_Cb)(Eina_Promise_Owner_VTable const* promise);

#define EINA_FUNC_PROMISE_OWNER_PROMISE_GET(Function) ((Eina_Promise_Owner_Promise_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's value_set function override
 */
typedef void(*Eina_Promise_Owner_Value_Set_Cb)(Eina_Promise_Owner_VTable* promise, const void* data, Eina_Promise_Free_Cb free_fun);

#define EINA_FUNC_PROMISE_OWNER_VALUE_SET(Function) ((Eina_Promise_Owner_Value_Set_Cb)Function)

/*
 * @brief Function callback type for promise owner's error_set function override
 */
typedef void(*Eina_Promise_Owner_Error_Set_Cb)(Eina_Promise_Owner_VTable* promise, Eina_Error error);

#define EINA_FUNC_PROMISE_OWNER_ERROR_SET(Function) ((Eina_Promise_Owner_Error_Set_Cb)Function)

/*
 * @brief Function callback type for promise owner's pending function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Pending_Is_Cb)(Eina_Promise_Owner_VTable const* promise);

#define EINA_FUNC_PROMISE_OWNER_PENDING_IS(Function) ((Eina_Promise_Owner_Pending_Is_Cb)Function)

/*
 * @brief Function callback type for promise owner's cancelled function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Cancelled_Is_Cb)(Eina_Promise_Owner_VTable const* promise);

#define EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(Function) ((Eina_Promise_Owner_Cancelled_Is_Cb)Function)

/*
 * @brief Function callback type for promise owner's progress function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Progress_Cb)(Eina_Promise_Owner_VTable const* promise, void* progress);

#define EINA_FUNC_PROMISE_OWNER_PROGRESS(Function) ((Eina_Promise_Owner_Progress_Cb)Function)

/*
 * @brief Function callback type for promise owner's progress notify registration function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Progress_Notify_Cb)(Eina_Promise_Owner_VTable* promise,
   Eina_Promise_Progress_Notify_Cb progress_cb, void* data, Eina_Promise_Free_Cb free_cb);

#define EINA_FUNC_PROMISE_OWNER_PROGRESS_NOTIFY(Function) ((Eina_Promise_Owner_Progress_Notify_Cb)Function)


#define EINA_PROMISE_VERSION 1

struct _Eina_Promise_VTable
{
  int version;
  Eina_Promise_Then_Cb then;
  Eina_Promise_Value_Get_Cb value_get;
  Eina_Promise_Error_Get_Cb error_get;
  Eina_Promise_Pending_Is_Cb pending_is;
  Eina_Promise_Progress_Cb_Add_Cb progress_cb_add;
  Eina_Promise_Cancel_Cb cancel;
  Eina_Promise_Ref_Cb ref;
  Eina_Promise_Unref_Cb unref;
  Eina_Promise_Ref_Get_Cb ref_get;
#define EINA_MAGIC_PROMISE 0x07932A5B
  EINA_MAGIC;
};

struct _Eina_Promise_Owner_VTable
{
  int version;
  Eina_Promise_Owner_Value_Set_Cb value_set;
  Eina_Promise_Owner_Error_Set_Cb error_set;
  Eina_Promise_Owner_Promise_Get_Cb promise_get;
  Eina_Promise_Owner_Pending_Is_Cb pending_is;
  Eina_Promise_Owner_Cancelled_Is_Cb cancelled_is;
  Eina_Promise_Owner_Progress_Cb progress;
  Eina_Promise_Owner_Progress_Notify_Cb progress_notify;
#define EINA_MAGIC_PROMISE_OWNER 0x07932A5C
  EINA_MAGIC;
};

EAPI Eina_Promise_Owner* eina_promise_owner_override(Eina_Promise_Owner_VTable* owner);

EAPI Eina_Promise* eina_promise_override(Eina_Promise_VTable* owner);

/*
 * @brief Appends a callback to be called when the Eina_Promise is
 * finished.
 *
 * @param promise The Eina_Promise to wait for
 * @param callback Callback to be called when Eina_Promise is finished
 * @param data Private data passed to the callback
 */
EAPI void eina_promise_then(Eina_Promise* promise, Eina_Promise_Cb callback,
                            Eina_Promise_Error_Cb error_cb, void* data);

/*
 * @brief Creates a new @Eina_Promise from other @Eina_Promise's
 *
 * The new @Eina_Promise is fulfilled when all promises
 * have also been fulfilled with success or when the
 * first one fails
 *
 * @param promises An Eina_Iterator for all Eina_Promises
 * @return Returns a new Eina_Promise
 */
EAPI Eina_Promise* eina_promise_all(Eina_Iterator* promises);

/*
 * @brief Creates a new @Eina_Promise from other @Eina_Promises
 *
 * The new @Eina_Promise is fulfilled when the first promise
 * has been fulfilled with success or when the
 * first one fails
 *
 * @param promises An Eina_Iterator for all Eina_Promises
 * @return Returns a new Eina_Promise
 */
EAPI Eina_Promise* eina_promise_race(Eina_Iterator* promises);

/*
 * @brief Creates a new @Eina_Promise from another @Eina_Promise_Owner which
 * is fulfilled when @promise has a progress callback registered
 *
 * @param promise Promise Owner which to be waited for a progress callback register
 * @return Returns a new Eina_Promise
 */
EAPI Eina_Promise* eina_promise_progress_notification(Eina_Promise_Owner* promise);

/*
 * @brief Sets value for Eina_Promise_Owner
 *
 * This finishes the Promise and calls all eina_promise_then callbacks
 * that have been registered on Eina_Promise. This function must be
 * called only once per Eina_Promise_Owner
 *
 * @param promise The promise owner for which to set the value
 * @param value The pointer to the value that is going to be copied, or NULL.
 * @param free_cb Callback function to be used to free the value copied
 */
EAPI void eina_promise_owner_value_set(Eina_Promise_Owner* promise, const void* value, Eina_Promise_Free_Cb free_cb);

/*
 * @brief Returns the pointer to the value
 *
 * If the Eina_Promise is not pending anymore and has not failed,
 * returns the pointer to the value, otherwise returns NULL.
 *
 * @param promise The promise for which to get the value
 * @return Value pointer or @NULL
 */
EAPI void* eina_promise_value_get(Eina_Promise const* promise);

/*
 * @brief Sets an error to the Eina_Promise
 *
 * Sets an error to the promise, finishing the promise and calling all
 * eina_promise_then callbacks registered.
 *
 * @param promise The promise owner for which to set the error
 * @param error Eina_Error to be set
 */
EAPI void eina_promise_owner_error_set(Eina_Promise_Owner* promise, Eina_Error error);

/*
 * @brief Gets the error of the @Eina_Promise
 * 
 * If the promise is finished and has been set with an error, it
 * retuns the @Eina_Error, otherwise returns 0.
 *
 * @param promise The promise for which to get the error
 * @return @Eina_Error set in @Eina_Promise
 */
EAPI Eina_Error eina_promise_error_get(Eina_Promise const* promise);

/*
 * @brief Returns @EINA_TRUE if the promise is still pending and
 * still waiting on a value to be set and @EINA_FALSE otherwise.
 *
 * @param promise The promise for which to get the pending status
 * @return Returns @EINA_TRUE if the promise is still pending and
 * still waiting on a value to be set and @EINA_FALSE otherwise.
 */
EAPI Eina_Bool eina_promise_pending_is(Eina_Promise const* promise);

/*
 * @brief Adds a progress callback that is called by the asynchronous
 * call to inform of any progress made.
 *
 * @param promise The promise for which to register a progress callback
 * @param progress The callback to be called when progress is made
 * @param data The private data that will be passed to the progress callback
 */
EAPI void eina_promise_progress_cb_add(Eina_Promise* promise, Eina_Promise_Progress_Cb progress, void* data,
                                       Eina_Promise_Free_Cb free_cb);

/*
 * @brief Increments the reference count for the Eina_Promise
 *
 * @param promise The promise for which to increment its reference
 */
EAPI void eina_promise_ref(Eina_Promise* promise);

/*
 * @brief Cancel asynchronous operation, if possible.
 *
 * @param promise The promise to cancel
 */
EAPI void eina_promise_cancel(Eina_Promise* promise);

/*
 * @brief Gets Promise from Promise_Owner
 *
 * This function returns an @Eina_Promise from an
 * @Eina_Promise_Owner. The user of the asynchronous operation can
 * query the value from the @Eina_Promise object, while the
 * asynchronous operation should use the @Eina_Promise_Owner object to
 * set the value, or error, and thus finish the promise.
 *
 * @param promise The promise owner to get promise from.
 * @return Return the @Eina_Promise
 */
EAPI Eina_Promise* eina_promise_owner_promise_get(Eina_Promise_Owner* promise);

/*
 * @brief Returns @EINA_TRUE if the promise is still pending and
 * still waiting on a value to be set and @EINA_FALSE otherwise.
 *
 * @param promise The promise owner for which to get the pending status
 * @return Returns @EINA_TRUE if the promise is still pending and
 * still waiting on a value to be set and @EINA_FALSE otherwise.
 */
EAPI Eina_Bool eina_promise_owner_pending_is(Eina_Promise_Owner const* promise);

/*
 * @brief Returns @EINA_TRUE if the promise was cancelled
 * or @EINA_FALSE otherwise.
 *
 * @param promise The promise owner for which to get the cancelled status
 * @return Returns @EINA_TRUE if the promise was cancelled
 * or @EINA_FALSE otherwise.
 */
EAPI Eina_Bool eina_promise_owner_cancelled_is(Eina_Promise_Owner const* promise);

/*
 * @brief Calls progress callbacks in promise, if any exists, with
 * the data.
 *
 * The progress callbacks registered in @Eina_Promise must not free
 * the progress data pointer. The data pointer ownership must be dealt
 * by the @Eina_Promise_Owner's user.
 *
 * @param promise The promise for which to get the cancelled status
 * @param data    The data to be passed to progress
 */
EAPI void eina_promise_owner_progress(Eina_Promise_Owner const* promise, void* progress);

/*
 * @brief Registers a progress notify callbacks in promise owner.
 *
 * Registers a callback to be called for when a progress callback is
 * registered by the linked @Eina_Promise.
 *
 * @param promise   The promise for which to get the cancelled status
 * @param notify_cb The callback to be called
 * @param data      The data to be passed to progress notify callback
 * @param free_cb   The free function that is called for the data param
 */
EAPI void eina_promise_owner_progress_notify(Eina_Promise_Owner* promise,
    Eina_Promise_Progress_Notify_Cb notify_cb, void* data, Eina_Promise_Free_Cb free_cb);
  
/*
 * @brief Decrement the reference count for the Eina_Promise.

 * The Eina_Promise, if its reference count drops to zero and is not
 * pending, will be free'd immediately.
 *
 * @param promise The promise for which to decrement its reference
 */
EAPI void eina_promise_unref(Eina_Promise* promise);

/*
 * @brief Get the reference count for the Eina_Promise.

 * @param promise The promise for which to get its reference
 */
EAPI int eina_promise_ref_get(Eina_Promise* promise);

/*
 * @brief Function callback type when promise is canceled
 */
typedef void(*Eina_Promise_Default_Cancel_Cb)(void* data, Eina_Promise_Owner* promise);

/*
 * @brief Creates a @Eina_Promise_Owner
 *
 * Create a @Eina_Promise_Owner for a pointer-type. Which is a promise
 * with ownership of the pointer when its value is set.  The Promise
 * itself, returned by eina_promise_owner_promise_get, represents the
 * asynchronicity of the value itself and is used solely to get the
 * value and to handle users of the asynchronous value. That's why
 * Promises have a reference count while Promise Owners do not, the
 * eina_promise_owner_value_set must be done only once, and
 * consequently, has a unique ownership of the owner lifetime, while
 * the promise can be queried and used by multiple users.
 *
 * @return @Eina_Promise_Owner just instantiated
 */
EAPI Eina_Promise_Owner* eina_promise_add();

/*
 * @brief Adds a cancel callback to be called when the promise is
 * canceled by the usuer
 *
 * @param promise The promise for which to register a cancel callback
 */
EAPI void eina_promise_owner_default_cancel_cb_add(
            Eina_Promise_Owner* promise, Eina_Promise_Default_Cancel_Cb cancel, void* data,
            Eina_Promise_Free_Cb free_cb);

/*
 * @brief Commands the @Eina_Promise_Owner not to call the then
 * callbacks automatically
 *
 * @param promise The promise for which to register a cancel callback
 */
EAPI void eina_promise_owner_default_manual_then_set(Eina_Promise_Owner* promise, Eina_Bool is_manual);

/*
 * @brief Calls the @Eina_Promise_Owner's then callbacks already registered
 *
 * @param promise The promise for which to register a cancel callback
 */
EAPI void eina_promise_owner_default_call_then(Eina_Promise_Owner* promise);

/**
 * @var EINA_ERROR_PROMISE_NO_NOTIFY
 *
 * @brief The error identifier corresponding to when a promise was
 * free'd before any progress callback was registered
 */
EAPI extern Eina_Error EINA_ERROR_PROMISE_NO_NOTIFY;

/**
 * @var EINA_ERROR_PROMISE_CANCEL
 *
 * @brief The error identifier corresponding to when a promise was
 * cancelled before the callback can be called
 */
EAPI extern Eina_Error EINA_ERROR_PROMISE_CANCEL;

/**
 * @var EINA_ERROR_PROMISE_NULL
 *
 * @brief The error identifier corresponding to when a promise function
 * is called on a NULL promise
 */
EAPI extern Eina_Error EINA_ERROR_PROMISE_NULL;

/*
 * @internal
 */
#define _EINA_PROMISE_BEFORE_HOOK(PromiseValue, Ret, ...)               \
  Eina_Promise_Owner* const __eo_promise = eina_promise_add();          \
  typedef Ret (*_Eo_Promise_func_t_)(Eo*, void *obj_data, ##__VA_ARGS__); \
  _Eo_Promise_func_t_ const _eo_promise_func_ = (_Eo_Promise_func_t_)_func_;

/*
 * @internal
 */
#define _EINA_PROMISE_CALL_HOOK(Arguments)                      \
  _eo_promise_func_(___call.eo_id, ___call.data, Arguments);


/*
 * @internal
 */
#define _EINA_PROMISE_AFTER_HOOK(Promise)                               \
  if(Promise)                                                           \
    *Promise = eina_promise_owner_promise_get(__eo_promise);

#endif

#endif /* ! EINA_PROMISE_H_ */
