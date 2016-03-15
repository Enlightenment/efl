
#ifndef EFL_ECORE_PROMISE_H
#define EFL_ECORE_PROMISE_H

#ifdef EFL_BETA_API_SUPPORT

struct _Ecore_Promise;

/*
 * @def _Ecore_Promise
 */
typedef struct _Ecore_Promise Ecore_Promise;

/*
 * @brief Function callback type for when using ecore_promise_then
 */
typedef void(*Ecore_Promise_Cb)(void* data, void* value);

/*
 * @brief Function callback type for when creating Ecore_Thread that
 * uses Ecore_Promise for communication
 */
typedef void(*Ecore_Promise_Thread_Cb)(const void* data, Ecore_Promise* promise);

/*
 * @brief Function that instantiates a Ecore_Promise and automatically
 * executes func_blocking callback function in another thread
 */
EAPI Ecore_Promise* ecore_promise_thread_run(Ecore_Promise_Thread_Cb func_blocking, const void* data, size_t value_size);

/*
 * @brief Creates a Ecore_Promise with a value of size value_size.
 *
 * @param value_size Size of value-type that Ecore_Promise will hold
 */
EAPI Ecore_Promise* ecore_promise_add(int value_size);

/*
 * @brief Appends a callback to be called when the Ecore_Promise is
 * finished.
 *
 * @param promise The Ecore_Promise to wait for
 * @param callback Callback to be called when Ecore_Promise is finished
 * @param data Private data passed to the callback
 */
EAPI void ecore_promise_then(Ecore_Promise* promise, Ecore_Promise_Cb callback, void* data);

/*
 * @brief Creates a new Ecore_Promise from other Ecore_Promises
 *
 * @param promises An Eina_Iterator for all Ecore_Promises
 */
EAPI Ecore_Promise* ecore_promise_all(Eina_Iterator* promises);

/*
 * @brief Sets value for Ecore_Promise. This finishes the callback and
 * calls all ecore_promise_then callbacks that have been registered on
 * this Ecore_Promise. This function must be called only once per
 * Ecore_Promise
 *
 * @param promise The promise for which to set the value
 * @param value The pointer to the value that is going to be copied, or NULL.
 */
EAPI void ecore_promise_value_set(Ecore_Promise* promise, void* value);

/*
 * @brief Returns the pointer to the value if the Ecore_Promise is
 * finished. Waits for it to be finished, otherwise.
 *
 * @param promise The promise for which to get the value
 */
EAPI void* ecore_promise_value_get(Ecore_Promise const* promise);

/*
 * @brief Returns the pointer to the buffer that holds the value. This
 * function is useful to instantiate the value directly in the correct
 * buffer, without needing to copy. The ecore_promise_value_set must
 * still be called, possibly with NULL, to finish the Ecore_Promise
 * and call the callbacks registered in it.
 *
 * @param promise The promise for which to get the buffer pointer
 */
EAPI void* ecore_promise_buffer_get(Ecore_Promise* promise);

/*
 * @brief Sets an error to the Ecore_Promise, thus finishing the
 * promise and calling all ecore_promise_then callbacks registered.
 *
 * @param promise The promise for which to set the error
 * @param error Eina_Error to be set
 */
EAPI void ecore_promise_error_set(Ecore_Promise* promise, Eina_Error error);

/*
 * @brief Gets an error to the Ecore_Promise if the promise is
 * finished and has error'ed out. If it hasn't finished, it will wait,
 * and if it has finished but otherwise not error'ed, returns 0.
 *
 * @param promise The promise for which to get the error
 */
EAPI Eina_Error ecore_promise_error_get(Ecore_Promise const* promise);

/*
 * @brief Gets the size of the value in ecore_promise_value_get.
 *
 * @param promise The promise for which to get the value size
 */
EAPI int ecore_promise_value_size_get(Ecore_Promise const* promise);

/*
 * @brief Returns @EINA_TRUE if the promise is ready and won't block
 * on ecore_promise_value_get and @EINA_FALSE otherwise.
 *
 * @param promise The promise for which to get the ready status
 */
EAPI Eina_Bool ecore_promise_ready_is(Ecore_Promise const* promise);

/*
 * @brief Increments the reference count for the Ecore_Promise
 *
 * @param promise The promise for which to increment its reference
 */
EAPI void ecore_promise_ref(Ecore_Promise* promise);

/*
 * @brief Decrement the reference count for the Ecore_Promise and
 * possibly schedule its destruction. The Ecore_Promise, if its
 * reference count drops to zero, will only be free'd when all the
 * current mainloop events have been processed. This allows the user
 * to call ecore_promise_then before that happens so it can increment
 * the reference back to 1 and wait for a value set or error set on
 * the Ecore_Promise.
 *
 * @param promise The promise for which to decrement its reference
 */
EAPI void ecore_promise_unref(Ecore_Promise* promise);

#define _EO_PROMISE_BEFORE_HOOK(PromiseValue)                           \
  typedef Ecore_Promise*(*_eo_ecore_promise_add_t)(int size);           \
  _eo_ecore_promise_add_t _eo_ecore_promise_add_f = (_eo_ecore_promise_add_t)_eo_ecore_promise_add; \
  Ecore_Promise* __eo_promise = _eo_ecore_promise_add_f(sizeof(PromiseValue));

#define _EO_PROMISE_AFTER_HOOK(Promise)                                 \
     if(Promise)                                                        \
       *Promise = __eo_promise;

#endif

#endif
