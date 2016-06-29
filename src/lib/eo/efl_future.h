#ifndef EFL_FUTURE_H_
# define EFL_FUTURE_H_

/**
 * @addtogroup Efl_Future Efl future and promise.
 * @{
 */

/**
 * @typedef Efl_Promise
 * The type of Efl Promise used in asynchronous operation, the write side of a promise.
 */
typedef Eo Efl_Promise;

#define EFL_FUTURE_CLASS efl_future_class_get()
EWAPI const Efl_Class *efl_future_class_get(void);

EAPI extern Eina_Error EINA_ERROR_FUTURE_CANCEL;

typedef struct _Efl_Future_Event_Failure Efl_Future_Event_Failure;
struct _Efl_Future_Event_Failure
{
   Efl_Promise *next;
   Eina_Error error;
};

typedef struct _Efl_Future_Event_Success Efl_Future_Event_Success;
struct _Efl_Future_Event_Success
{
   Efl_Promise *next;
   void *value;
};

typedef struct _Efl_Future_Event_Progress Efl_Future_Event_Progress;
struct _Efl_Future_Event_Progress
{
   Efl_Promise *next;
   void *progress;
};

EOAPI extern const Efl_Event_Description _EFL_FUTURE_EVENT_FAILURE;
EOAPI extern const Efl_Event_Description _EFL_FUTURE_EVENT_SUCCESS;
EOAPI extern const Efl_Event_Description _EFL_FUTURE_EVENT_PROGRESS;

  // FIXME: documentation
#define EFL_FUTURE_EVENT_FAILURE (&(_EFL_FUTURE_EVENT_FAILURE))
#define EFL_FUTURE_EVENT_SUCCESS (&(_EFL_FUTURE_EVENT_SUCCESS))
#define EFL_FUTURE_EVENT_PROGRESS (&(_EFL_FUTURE_EVENT_PROGRESS))

/**
 * @brief Add sets of callbacks to handle the progress and the result of a future.
 *
 * callbacks are called depending on the outcome of the promise related to the future.
 *
 * @param[in] success the callback to call in case of a succesful computation from the promise
 * @param[in] failure the callback to call in case of a failure to deliver from the promise
 * @param[in] progress the callback to call during the progression of the the promise, this is optional
 * @param[in] data additional data to pass to the callback
 *
 * @return Return a new future when the callback has been successfully added. This future can be ignored.
 *
 * @note except if you do reference count the Efl.Future object, you can only call once this function.
 *
 * @ingroup Efl_Future
 */
EOAPI Efl_Future *efl_future_then(Eo *obj, Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data);

/**
 * @brief Cancel the need for that specific future.
 *
 * This will trigger the failure of the future and may result in the promise stopping its computation.
 *
 * @see efl_future_use
 *
 * @ingroup Efl_Future
 */
EOAPI void efl_future_cancel(Eo *obj);

/**
 * @brief To be used in conjunction with when you plan to use efl_future_cancel
 *
 * This function will store in *wref, obj and make sure that on failure or success of the future, it
 * will be reset to NULL. This guarantee that the pointer you are using will always be correct and
 * that you do not have to worry about passing a dead pointer to efl_future_cancel.
 *
 * @param[out] storage Will be set to obj and tracked during all the lifetime of the future.
 * @param[in] future The future to remember about.
 *
 * @see efl_future_cancel
 *
 * @ingroup Efl_Future
 */
static inline void
efl_future_use(Efl_Future **storage, Eo *future)
{
   efl_wref_add(future, storage);
}

/**
 * @}
 */

/**
 * @}
 */

#endif
