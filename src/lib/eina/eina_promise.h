
#ifdef EFL_BETA_API_SUPPORT

struct _Eina_Promise;
struct _Eina_Promise_Default;

/*
 * @def Eina_Promise
 */
typedef struct _Eina_Promise Eina_Promise;

/*
 * @def Eina_Promise
 */
typedef struct _Eina_Promise_Owner Eina_Promise_Owner;

/*
 * @brief Callback type for freeing Promise value and void* pointer data for callbacks
 */
typedef void(*Eina_Promise_Free_Cb)(void* value);

/*
 * @brief Function callback type for when using eina_promise_then
 */
typedef void(*Eina_Promise_Cb)(void* data, void* value);

/*
 * @brief Function callback type for when using eina_promise_then
 */
typedef void(*Eina_Promise_Error_Cb)(void* data, Eina_Error const* error);

/*
 * @brief Function callback type for progress information
 */
typedef void(*Eina_Promise_Progress_Cb)(void* data, void* value);

/*
 * @brief Function callback type for then function override
 */
typedef void(*Eina_Promise_Then_Cb)(Eina_Promise* promise, Eina_Promise_Cb callback,
                                    Eina_Promise_Error_Cb error_cb, void* data);

#define EINA_FUNC_PROMISE_THEN(Function) ((Eina_Promise_Then_Cb)Function)

/*
 * @brief Function callback type for promise's value_get function override
 */
typedef void*(*Eina_Promise_Value_Get_Cb)(Eina_Promise const* promise);

#define EINA_FUNC_PROMISE_VALUE_GET(Function) ((Eina_Promise_Value_Get_Cb)Function)

/*
 * @brief Function callback type for promise's error_get function override
 */
typedef Eina_Error(*Eina_Promise_Error_Get_Cb)(Eina_Promise const* promise);

#define EINA_FUNC_PROMISE_ERROR_GET(Function) ((Eina_Promise_Error_Get_Cb)Function)

/*
 * @brief Function callback type for promise's pending function override
 */
typedef Eina_Bool(*Eina_Promise_Pending_Is_Cb)(Eina_Promise const* promise);

#define EINA_FUNC_PROMISE_PENDING_IS(Function) ((Eina_Promise_Pending_Is_Cb)Function)

/*
 * @brief Function callback type for promise's progress add function override
 */
typedef void(*Eina_Promise_Progress_Cb_Add_Cb)(Eina_Promise* promise, Eina_Promise_Progress_Cb callback, void* data);

#define EINA_FUNC_PROMISE_PROGRESS_CB_ADD(Function) ((Eina_Promise_Progress_Cb_Add_Cb)Function)

/*
 * @brief Function callback type for promise's cancel function override
 */
typedef void(*Eina_Promise_Cancel_Cb)(Eina_Promise* promise);

#define EINA_FUNC_PROMISE_CANCEL(Function) ((Eina_Promise_Cancel_Cb)Function)

/*
 * @brief Function callback type for promise's ref function override
 */
typedef void(*Eina_Promise_Ref_Cb)(Eina_Promise* promise);

#define EINA_FUNC_PROMISE_REF(Function) ((Eina_Promise_Ref_Cb)Function)

/*
 * @brief Function callback type for promise's unref function override
 */
typedef void(*Eina_Promise_Unref_Cb)(Eina_Promise* promise);

#define EINA_FUNC_PROMISE_UNREF(Function) ((Eina_Promise_Unref_Cb)Function)

/*
 * @brief Function callback type for promise's buffer_get function override
 */
typedef void*(*Eina_Promise_Buffer_Get_Cb)(Eina_Promise* promise);

#define EINA_FUNC_PROMISE_BUFFER_GET(Function) ((Eina_Promise_Buffer_Get_Cb)Function)

/*
 * @brief Function callback type for promise's value_size_get function override
 */
typedef size_t(*Eina_Promise_Value_Size_Get_Cb)(Eina_Promise const* promise);

#define EINA_FUNC_PROMISE_VALUE_SIZE_GET(Function) ((Eina_Promise_Value_Size_Get_Cb)Function)


/*
 * @brief Function callback type for promise owner's buffer_get function override
 */
typedef void*(*Eina_Promise_Owner_Buffer_Get_Cb)(Eina_Promise_Owner* promise);

#define EINA_FUNC_PROMISE_OWNER_BUFFER_GET(Function) ((Eina_Promise_Owner_Buffer_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's value_size_get function override
 */
typedef size_t(*Eina_Promise_Owner_Value_Size_Get_Cb)(Eina_Promise_Owner const* promise);

#define EINA_FUNC_PROMISE_OWNER_VALUE_SIZE_GET(Function) ((Eina_Promise_Owner_Value_Size_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's promise_get function override
 */
typedef void*(*Eina_Promise_Owner_Promise_Get_Cb)(Eina_Promise_Owner const* promise);

#define EINA_FUNC_PROMISE_OWNER_PROMISE_GET(Function) ((Eina_Promise_Owner_Promise_Get_Cb)Function)

/*
 * @brief Function callback type for promise owner's value_set function override
 */
typedef void(*Eina_Promise_Owner_Value_Set_Cb)(Eina_Promise_Owner* promise, void* data, Eina_Promise_Free_Cb free_fun);

#define EINA_FUNC_PROMISE_OWNER_VALUE_SET(Function) ((Eina_Promise_Owner_Value_Set_Cb)Function)

/*
 * @brief Function callback type for promise owner's error_set function override
 */
typedef void(*Eina_Promise_Owner_Error_Set_Cb)(Eina_Promise_Owner* promise, Eina_Error error);

#define EINA_FUNC_PROMISE_OWNER_ERROR_SET(Function) ((Eina_Promise_Owner_Error_Set_Cb)Function)

/*
 * @brief Function callback type for promise owner's pending function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Pending_Is_Cb)(Eina_Promise_Owner const* promise);

#define EINA_FUNC_PROMISE_OWNER_PENDING_IS(Function) ((Eina_Promise_Owner_Pending_Is_Cb)Function)

/*
 * @brief Function callback type for promise owner's cancelled function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Cancelled_Is_Cb)(Eina_Promise_Owner const* promise);

#define EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(Function) ((Eina_Promise_Owner_Cancelled_Is_Cb)Function)

/*
 * @brief Function callback type for promise owner's progress function override
 */
typedef Eina_Bool(*Eina_Promise_Owner_Progress_Cb)(Eina_Promise_Owner const* promise, void* progress);

#define EINA_FUNC_PROMISE_OWNER_PROGRESS(Function) ((Eina_Promise_Owner_Progress_Cb)Function)


#define EINA_PROMISE_VERSION 1

struct _Eina_Promise
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
  Eina_Promise_Value_Size_Get_Cb value_size_get;
  Eina_Promise_Buffer_Get_Cb buffer_get;
#define EINA_MAGIC_PROMISE 0x07932A5B
  EINA_MAGIC;
};

struct _Eina_Promise_Owner
{
  int version;
  Eina_Promise_Owner_Value_Set_Cb value_set;
  Eina_Promise_Owner_Error_Set_Cb error_set;
  Eina_Promise_Owner_Buffer_Get_Cb buffer_get;
  Eina_Promise_Owner_Value_Size_Get_Cb value_size_get;
  Eina_Promise_Owner_Promise_Get_Cb promise_get;
  Eina_Promise_Owner_Pending_Is_Cb pending_is;
  Eina_Promise_Owner_Cancelled_Is_Cb cancelled_is;
  Eina_Promise_Owner_Progress_Cb progress;
#define EINA_MAGIC_PROMISE_OWNER 0x07932A5C
  EINA_MAGIC;
};

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
 * @brief Creates a new Eina_Promise from other Eina_Promises
 *
 * @param promises An Eina_Iterator for all Eina_Promises
 * @return Returns a new Eina_Promise
 */
EAPI Eina_Promise* eina_promise_all(Eina_Iterator* promises);

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
EAPI void eina_promise_owner_value_set(Eina_Promise_Owner* promise, void* value, Eina_Promise_Free_Cb free_cb);

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
 * @brief Returns the pointer to the buffer that holds the value.
 * 
 * If the promise is finished and has not failed, this function is the
 * same as @eina_promise_value_get. Otherwise, instead of returning
 * NULL as @eina_promise_value_get, this function always returns the
 * buffer pointer to where the value will be hold.
 *
 * @param promise The promise for which to get the buffer pointer
 * @return Buffer pointer
 */
EAPI void* eina_promise_buffer_get(Eina_Promise* promise);

/*
 * @brief Returns the pointer to the buffer that holds the value.
 *
 * This function always return the buffer pointer independently if the
 * value has been set or not. This is useful to instantiate the value
 * directly in the correct buffer, without needing to copy. Then the
 * user can @eina_promise_owner_value_set with a NULL pointer for the
 * value to avoid copying over the buffer.
 *
 * @param promise The promise owner for which to get the buffer pointer
 * @return Buffer pointer
 */
EAPI void* eina_promise_owner_buffer_get(Eina_Promise_Owner* promise);

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
 * @brief Gets the size of the value in eina_promise_value_get.
 *
 * @param promise The promise for which to get the value size
 * @return The size of the value in eina_promise_value_get.
 */
EAPI size_t eina_promise_value_size_get(Eina_Promise const* promise);

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
EAPI void eina_promise_progress_cb_add(Eina_Promise* promise, Eina_Promise_Progress_Cb progress, void* data);

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
 * @brief Decrement the reference count for the Eina_Promise.

 * The Eina_Promise, if its reference count drops to zero and is not
 * pending, will be free'd immediately.
 *
 * @param promise The promise for which to decrement its reference
 */
EAPI void eina_promise_unref(Eina_Promise* promise);

/*
 * @brief Function callback type when promise is canceled
 */
typedef void(*Eina_Promise_Default_Cancel_Cb)(void* data, Eina_Promise_Owner* promise);

/*
 * @brief Creates a @Eina_Promise_Owner
 *
 * Create a @Eina_Promise_Owner with a value of size value_size. Which
 * is a promise with ownership of the value to be set.  It is used by
 * the asynchronous operation to set the actual value when it becomes
 * available. The Promise itself, returned by
 * eina_promise_owner_promise_get, represents the asynchronicity of
 * the value itself and is used solely to get the value and to handle
 * users of the asynchronous value. That's why Promises have a
 * reference count while Promise Owners do not, the
 * eina_promise_owner_value_set must be done only once, and
 * consequently, has a unique ownership of the owner lifetime, while
 * the promise can be queried and used by multiple users.
 *
 * @param value_size Size of value-type that Eina_Promise will hold
 * @return @Eina_Promise_Owner just instantiated
 */
EAPI Eina_Promise_Owner* eina_promise_default_add(int value_size);

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


/*
 * @internal
 */
#define _EINA_PROMISE_BEFORE_HOOK(PromiseValue, Ret, ...)               \
  Eina_Promise_Owner* __eo_promise = eina_promise_default_add(sizeof(PromiseValue)); \
  typedef Ret (*_Eo__Promise_func_)(Eo*, void *obj_data, ##__VA_ARGS__);  \
  _Eo__Promise_func_ const _promise_func = (_Eo__Promise_func_)_func_;  \
  {                                                                     \
    _Eo__Promise_func_ const _func_ = _promise_func;


/*
 * @internal
 */
#define _EINA_PROMISE_AFTER_HOOK(Promise)                               \
  }                                                                     \
     if(Promise)                                                        \
       *Promise = eina_promise_owner_promise_get(__eo_promise);

#endif
