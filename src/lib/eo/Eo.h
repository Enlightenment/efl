#ifndef EO_H
#define EO_H

#include <stdarg.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#define EOLIAN

/* When used, it indicates that the function is an Eo API. */
#define EOAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_EO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page eo_main Eo
 *
 * @date 2012 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eo_main_intro
 * @li @ref eo_main_compiling
 * @li @ref eo_main_next_steps
 * @li @ref eo_main_intro_example
 * @li @ref eo_lifecycle_debug
 *
 * @section eo_main_intro Introduction
 *
 * The Eo generic object system. It was designed to be the base object
 * system for the EFL.

 * @section eo_main_compiling How to compile
 *
 * Eo is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eo`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eo`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eo_main_next_steps Next Steps
 *
 * After you understood what Eo is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Efl_Class_Base
 * @li @ref Efl_Class
 * @li @ref Efl_Events
 * @li @ref Eo_Composite_Objects
 *
 * @section eo_lifecycle_debug Debug Object Lifecycle
 *
 * When dealing with objects it's important to investigate the object
 * lifecycle: when it was created, when it was deleted. This is not
 * that trivial since objects can have extra references added with
 * efl_ref() as well as removed with efl_unref(), efl_parent_set() to
 * NULL or efl_del().
 *
 * To aid development process and debug memory leaks and invalid
 * access, we provide eo_debug script helper that will preload
 * libeo_dbg.so, run as:
 *
 * @verbatim
   export EO_LIFECYCLE_DEBUG=1
   export EINA_LOG_LEVELS=eo_lifecycle:4
   eo_debug my_app
 * @endverbatim
 *
 * This will print out all the objects that were created and deleted,
 * as well as keep the stack trace that originated those, if a double
 * free or user-after-free occurs it will print out the backtrace
 * where the object was created and where it was deleted. If only
 * errors should be displayed, decrease the log level to 2:
 *
 * @verbatim
   export EO_LIFECYCLE_DEBUG=1
   export EINA_LOG_LEVELS=eo_lifecycle:2  # just critical, error and warning
   eo_debug my_app
 * @endverbatim
 *
 * Keep in mind that the log will consume memory for all objects and
 * that main loop primitives such as timers, jobs, promises and
 * futures are all objects, being created in large numbers, thus
 * consuming lots of memory.
 *
 * To address that log pollution and memory consumption, one can
 * select just handful classes to be logged using @c
 * EO_LIFECYCLE_DEBUG with a list of comma-separated class names. If
 * @c EO_LIFECYCLE_DEBUG=1 or @c EO_LIFECYCLE_DEBUG=*, then all
 * classes are logged, otherwise just the classes listed will be
 * (whitelist).
 *
 * @verbatim
   # Log only 2 classes: Efl_Loop and Efl_Net_Dialer_Tcp
   export EO_LIFECYCLE_DEBUG=Efl_Loop,Efl_Net_Dialer_Tcp
   export EINA_LOG_LEVELS=eo_lifecycle:4
   eo_debug my_app
 * @endverbatim
 *
 * Another approach is to log all but few classes, also known as
 * blacklist. This is done with another environment variable @c
 * EO_LIFECYCLE_NO_DEBUG=class1,class2.
 *
 * @verbatim
   # Log all but Efl_Future, Efl_Promise and Efl_Loop_Timer
   export EO_LIFECYCLE_NO_DEBUG=Efl_Future,Efl_Promise,Efl_Loop_Timer
   export EINA_LOG_LEVELS=eo_lifecycle:4
   eo_debug my_app
 * @endverbatim
 *
 * @section eo_main_intro_example Introductory Example
 *
 * @ref Eo_Tutorial

 *
 *
 * @addtogroup Eo
 * @{
 */

typedef struct _Eo_Object _Eo_Object;

/**
 * @typedef Eo
 * The basic Object type.
 */
typedef struct _Eo_Opaque Eo;

/**
 * @typedef Efl_Class
 * The basic class type - should be removed, just for compat.
 */
typedef Eo Efl_Class;
#define _EFL_CLASS_EO_CLASS_TYPE

typedef Eo Efl_Object;
#define _EFL_OBJECT_EO_CLASS_TYPE

#ifdef EFL_BETA_API_SUPPORT

/**
 * @var _efl_class_creation_lock
 * This variable is used for locking purposes in the class_get function
 * defined in #EFL_DEFINE_CLASS.
 * This is just to work around the fact that we need to init locks before
 * using them.
 * Don't touch it if you don't know what you are doing.
 * @internal
 */
EAPI extern Eina_Lock _efl_class_creation_lock;

/**
 * @var _efl_object_init_generation
 * This variable stores the current eo init generation. That is, how many times
 * we have completed full init/shutdown cycles. Starts at 1 and incremeted on
 * every call to shutdown that actually shuts down eo.
 * @internal
 */
EAPI extern unsigned int _efl_object_init_generation;

/**
 * @typedef Efl_Del_Intercept
 *
 * A function to be called on object deletion/destruction instead of normal
 * destruction taking place.
 *
 * @param obj_id The object needing destruction
 */
typedef void (*Efl_Del_Intercept) (Eo *obj_id);

/**
 * @typedef Efl_Future
 * The type of Efl Future used in asynchronous operation, the read side of a promise.
 */
typedef Eo Efl_Future;

#include "efl_object_override.eo.h"
#include "efl_object.eo.h"
#include "efl_interface.eo.h"
#define EO_CLASS EFL_OBJECT_CLASS

/** An event callback prototype. */
typedef void (*Efl_Event_Cb)(void *data, const Efl_Event *event);

/**
 * @brief Callback priority value. Range is -32k - 32k. The lower the number,
 * the higher the priority.
 *
 * See @ref EFL_CALLBACK_PRIORITY_AFTER, @ref EFL_CALLBACK_PRIORITY_BEFORE @ref
 * EFL_CALLBACK_PRIORITY_DEFAULT
 */
typedef short Efl_Callback_Priority;

/**
 * @struct _Efl_Callback_Array_Item
 * @brief An item in an array of callback desc/func.
 *
 * See also efl_event_callback_array_add().
 */
typedef struct _Efl_Callback_Array_Item
{
  const Efl_Event_Description *desc; /**< The event description. */
  Efl_Event_Cb func; /**< The callback function. */
} Efl_Callback_Array_Item;

/**
 * @brief Add a callback for an event with a specific priority.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * A callback is only executed on events emitted after this call finished.
 *
 * @param[in] desc The description of the event to listen to
 * @param[in] priority The priority of the callback
 * @param[in] cb the callback to call
 * @param[in] data additional data to pass to the callback
 *
 * @return Return @c true when the callback has been successfully added.
 */
EOAPI Eina_Bool efl_event_callback_priority_add(Eo *obj, const Efl_Event_Description *desc, Efl_Callback_Priority priority, Efl_Event_Cb cb, const void *data);

/**
 * @brief Delete a callback with a specific data associated to it for an event.
 *
 * The callback will never be emitted again after this call, even if a event
 * emission is going on.
 *
 * @param[in] desc The description of the event to listen to
 * @param[in] func The callback to delete
 * @param[in] user_data The data to compare
 *
 * @return Return @c true when the callback has been successfully removed.
 */
EOAPI Eina_Bool efl_event_callback_del(Eo *obj, const Efl_Event_Description *desc, Efl_Event_Cb func, const void *user_data);

/**
 * @brief Add an array of callbacks created by @ref EFL_CALLBACKS_ARRAY_DEFINE
 * for an event with a specific priority. The array need to be sorted with @ref
 * efl_callbacks_cmp if you are not using the @ref EFL_CALLBACKS_ARRAY_DEFINE
 * macro.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * A callback from the array is only executed on events emitted  after this
 * call finished.
 *
 * @param[in] array An #Efl_Callback_Array_Item of events to listen to
 * @param[in] priority The priority of the callback
 * @param[in] data Additional data to pass to the callback
 *
 * @return Return @c true when the callback has been successfully added.
 */
EOAPI Eina_Bool efl_event_callback_array_priority_add(Eo *obj, const Efl_Callback_Array_Item *array, Efl_Callback_Priority priority, const void *data);

/**
 * @brief Del a callback array with a specific data associated to it for an
 * event. The callbacks from the array will never be emitted again after this
 * call, even if a event emission is going on.
 *
 * @param[in] array An #Efl_Callback_Array_Item of events to listen to
 * @param[in] user_data The data to compare
 *
 * @return Return @c true when the callback has been successfully removed.
 */
EOAPI Eina_Bool efl_event_callback_array_del(Eo *obj, const Efl_Callback_Array_Item *array, const void *user_data);

/**
 * @brief Call the callbacks for an event of an object.
 *
 * @param[in] desc The description of the event to call
 * @param[in] event_info Extra event info to pass to the callbacks
 *
 * @return @c false if one of the callbacks aborted the call, @c true otherwise
 */
EOAPI Eina_Bool efl_event_callback_call(Eo *obj, const Efl_Event_Description *desc, void *event_info);

/**
 * @brief Call the callbacks for an event of an object.
 *
 * Like @ref efl_event_callback_call, but also call legacy smart callbacks that
 * have the same name of the given event.
 *
 * @param[in] desc The description of the event to call
 * @param[in] event_info Extra event info to pass to the callbacks
 *
 * @return @c false if one of the callbacks aborted the call, @c true otherwise
 *
 * @since 1.19
 */
EOAPI Eina_Bool efl_event_callback_legacy_call(Eo *obj, const Efl_Event_Description *desc, void *event_info);

/**
 * @brief Track a future life cycle and cancel it if the object dies.
 *
 * @param[in] link The future to link with the object
 *
 * @return @c true if it succeeded on setting up the tracking.
 */
EOAPI Eina_Bool efl_future_link(Eo *obj, Efl_Future *link);


/**
 * @struct _Efl_Future_Cb_Desc
 *
 * A struct with callbacks to be used by efl_future_cb_from_desc() and efl_future_chain_array()
 *
 * @see efl_future_cb_from_desc()
 * @see efl_future_chain_array()
 */
typedef struct _Efl_Future_Cb_Desc {
   /**
    * Called on success (value.type is not @c EINA_VALUE_TYPE_ERROR).
    *
    * if @c success_type is not NULL, then the value is guaranteed to be of that type,
    * if it's not, then it will trigger @c error with @c EINVAL.
    *
    * After this function returns, @c free callback is called if provided.
    *
    * @note This function is always called from a safe context (main loop or some platform defined safe context).
    *
    * @param o The object used to create the link in efl_future_cb_from_desc() or efl_future_chain_array().
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
    */
   Eina_Value (*success)(Eo *o, const Eina_Value value);
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
    *
    * @note On future creation errors and future cancellation this function will be called
    * from the current context with the following errors respectitally: `EINVAL`, `ENOMEM` and  `ECANCELED`.
    * Otherwise this function is called from a safe context.
    *
    *
    * @param o The object used to create the link in efl_future_cb_from_desc() or efl_future_chain_array().
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
    */
   Eina_Value (*error)(Eo *o, Eina_Error error);
   /**
    * Called on @b all situations to notify future destruction.
    *
    * This is called after @c success or @c error, as well as it's called if none of them are
    * provided. Thus can be used as a "weak ref" mechanism.
    *
    * @note On future creation errors and future cancellation this function will be called
    * from the current context with the following errors respectitally: `EINVAL`, `ENOMEM` and  `ECANCELED`.
    * Otherwise this function is called from a safe context.
    *
    * @param o The object used to create the link in efl_future_cb_from_desc() or efl_future_chain_array().
    * @param dead_future The future that was freed.
    */
   void (*free)(Eo *o, const Eina_Future *dead_future);
   /**
    * If provided, then @c success will only be called if the value type matches the given pointer.
    *
    * If provided and doesn't match, then @c error will be called with @c EINVAL. If no @c error,
    * then it will be propagated to the next future in the chain.
    */
   const Eina_Value_Type *success_type;
   /**
    * This is used by Eo to cancel a pending futures in case
    * an Eo object is deleted. It can be @c NULL.
    */
   Eina_Future **storage;
} Efl_Future_Cb_Desc;

/**
 * Creates an Eina_Future_Desc for an EO object.
 *
 * This function creates an Eina_Future_Desc based on an Efl_Future_Cb_Desc.
 * The main purpose of this function is create a "link" between the future
 * and the object. In case the object is deleted before the future is resolved/rejected,
 * the object destructor will cancel the future.
 *
 * @note In case context info are needed for the #Efl_Future_Desc callbacks efl_key_data_set()
 * can be used.
 *
 * The example below shows a file download using an Eo object, if the download
 * lasts more than 30 seconds the Eo object will be deleted, causing the
 * future to also be deleted.
 * Usually this would be done with an eina_future_race() of the download promise and a timeout promise,
 * however we provide the following example to illustrate efl_key_data_set() usage.
 *
 * @code
 *
 * static Eina_Bool
 * _timeout(void *data)
 * {
 *    Eo *downloader = data;
 *    //In case the download is not completed yet.
 *    //Delete the downloader (which in cancel the file download and the future)
 *    efl_key_data_set(downloader, "timer", NULL);
 *    efl_unref(downloader);
 *    return EINA_FALSE;
 * }
 *
 * static Eina_Value
 * _file_ok(Eo *o EINA_UNUSED, const Eina_Value value)
 * {
 *    const char *data;
 *    //There's no need to check the value type since EO infra already did that for us
 *    eina_value_get(&value, &data);
 *    //Deliver the data to the user
 *    data_deliver(data);
 *    return v;
 * }
 *
 * static Eina_Value
 * _file_err(Eo *o EINA_UNUSED, Eina_Error error)
 * {
 *    //In case the downloader is deleted before the future is resolved, the future will be canceled thus this callback will be called.
 *    fprintf(stderr, "Could not download the file. Reason: %s\n", eina_error_msg_get(error));
 *    return EINA_VALUE_EMPTY;
 * }
 *
 * static void
 * _downlader_free(Eo *o, const Eina_Future *dead_future EINA_UNUSED)
 * {
 *    Ecore_Timer *t = efl_key_data_get(o, "timer");
 *    //The download was finished before the timer expired. Cancel it...
 *    if (t)
 *    {
 *      ecore_timer_del(t);
 *      efl_unref(o); //Delete the object
 *    } //else - In this case the future was canceled due efl_unref() in _timeout - No need to call efl_unref()
 * }
 *
 * void download_file(const char *file)
 * {
 *   //This could be rewritten using eina_future_race()
 *   Eo *downloader = efl_add(MY_DOWNLOADER_CLASS, NULL);
 *   Eina_Future *f = downloader_download_file(downloader, file);
 *   timer = ecore_timer_add(30, _timeout, downloader);
 *   //Usually this would be done with an eina_future_race() of the download promise and a timeout promise,
 *   //however we provide the following example to illustrate efl_key_data_set() usage.
 *   efl_key_data_set(downloader, "timer", timer);
 *   eina_future_then_from_desc(f, efl_future_cb(.success = _file_ok, .error = _file_err, .success_type = EINA_VALUE_TYPE_STRING, .free = downloader_free));
 * }
 * @endcode
 *
 * @param obj The object to create the link.
 * @param desc An Efl_Future_Cb_Desc
 * @return An Eina_Future_Desc to be used by eina_future_then(), eina_future_chain() and friends.
 * @see efl_future_chain_array()
 * @see efl_future_cb()
 * @see #Efl_Future_Cb_Desc
 * @see efl_key_data_set()
 */
EOAPI Eina_Future_Desc efl_future_cb_from_desc(Eo *obj, const Efl_Future_Cb_Desc desc);

/**
 * Syntax suger over efl_future_cb_from_desc()
 *
 * Usage:
 * @code
 * eina_future_then_from_desc(future, efl_future_cb(my_object, .succes = success, .success_type = EINA_VALUE_TYPE_INT));
 * @endcode
 *
 * @see efl_future_cb_from_desc()
 */
#define efl_future_cb(_eo, ...) efl_future_cb_from_desc(_eo, (Efl_Future_Cb_Desc){__VA_ARGS__})

/**
 * Creates an Future chain based on #Efl_Future_Cb_Desc
 *
 * This function is an wrapper around efl_future_cb_from_desc() and eina_future_then_from_desc()
 *
 * For more information about them, check their documentations.
 *
 *
 * @param obj An EO object to link against the future
 * @param prev The previous future
 * @param descs An array of Efl_Future_Cb_Desc
 * @return An Eina_Future or @c NULL on error.
 * @note If an error happens the whole future chain will be CANCELED, causing
 * desc.error to be called passing `ENOMEM` or `EINVAL` and desc.free
 * to free the @p obj if necessary.
 *
 * @see efl_future_chain()
 * @see efl_future_cb()
 * @see eina_future_then_from_desc()
 * @see #Efl_Future_Cb_Desc
 */
EOAPI Eina_Future *efl_future_chain_array(Eo *obj, Eina_Future *prev, const Efl_Future_Cb_Desc descs[]);

/**
 * Syntax suger over efl_future_chain_array()
 *
 * Usage:
 * @code
 * Eina_Future *f = efl_future_chain(my_object, prev_future, {}, {});
 * @endcode
 *
 * @see efl_future_chain_array()
 */
#define efl_future_chain(_eo, _prev, ...) efl_future_chain_array(_eo, _prev, (Efl_Future_Cb_Desc []){__VA_ARGS__, {NULL, NULL, NULL, NULL, NULL}})

/**
 * @addtogroup Eo_Debug_Information Eo's Debug information helper.
 * @{
 */

/**
 * @struct _Efl_Dbg_Info
 * The structure for the debug info used by Eo.
 */
typedef struct _Efl_Dbg_Info
{
  Eina_Stringshare *name; /**< The name of the part (stringshare). */
  Eina_Value value; /**< The value. */
} Efl_Dbg_Info;

/**
 * @brief Get debug information from the object.
 *
 * @param[in] root_node Node of the tree
 */
EOAPI void efl_dbg_info_get(Eo *obj, Efl_Dbg_Info *root_node);

/**
 * @var EFL_DBG_INFO_TYPE
 * The Eina_Value_Type for the debug info.
 */
EAPI extern const Eina_Value_Type *EFL_DBG_INFO_TYPE;

/**
 * Creates a list inside debug info list.
 * @param[in] list list where to append
 * @param[in] name name of the list
 * @return the new list
 */
static inline Efl_Dbg_Info *
EFL_DBG_INFO_LIST_APPEND(Efl_Dbg_Info *list, const char *name)
{
   Efl_Dbg_Info *tmp = (Efl_Dbg_Info *)calloc(1, sizeof(*tmp));

   if (!tmp) return NULL;
   tmp->name = eina_stringshare_add(name);
   eina_value_list_setup(&(tmp->value), EFL_DBG_INFO_TYPE);
   if (list)
     {
        eina_value_list_pappend(&(list->value), tmp);
     }
   return tmp;
}

/**
 * @def EFL_DBG_INFO_APPEND
 * Creates a new debug info into a list
 * @param[in] LIST list where to append (Efl_Dbg_Info *)
 * @param[in] NAME name of the parameter (const char *)
 * @param[in] TYPE type of the parameter (Eina_Value_Type *)
 * @param[in] VALUE value of the parameter
 */
#define EFL_DBG_INFO_APPEND(LIST, NAME, TYPE, VALUE) \
do { \
   Efl_Dbg_Info *List = LIST; \
   if (List) \
     { \
        Efl_Dbg_Info *Tmp = calloc(1, sizeof(*Tmp)); \
                                                    \
        if (!Tmp) break; \
        Tmp->name = eina_stringshare_add(NAME); \
        eina_value_setup(&(Tmp->value), TYPE); \
        eina_value_set(&(Tmp->value), VALUE); \
        eina_value_list_pappend(&(List->value), Tmp); \
     } \
} while (0)

/**
 * Frees the Efl_Dbg_Info tree. (The whole tree recursively).
 * @param[in] info The tree to delete.
 */
EAPI void efl_dbg_info_free(Efl_Dbg_Info *info);

/**
 * @}
 */

/**
 * @typedef Efl_Object_Op
 * The Eo operation type id.
 */
typedef unsigned int Efl_Object_Op;

/**
 * @def EFL_NOOP
 * A special #Efl_Object_Op meaning "No operation".
 */
#define EFL_NOOP ((Efl_Object_Op) 0)

/**
 * @addtogroup Efl_Events Eo's Event Handling
 * @{
 */

/**
 * @def EFL_EVENT_DESCRIPTION(name)
 * An helper macro to help populating #Efl_Event_Description
 * @param name The name of the event.
 * @see Efl_Event_Description
 */
#define EFL_EVENT_DESCRIPTION(name) { name, EINA_FALSE, EINA_FALSE, EINA_FALSE }

/**
 * @def EFL_EVENT_DESCRIPTION_HOT(name)
 * An helper macro to help populating #Efl_Event_Description and make
 * the event impossible to freeze.
 * @param name The name of the event.
 * @see Efl_Event_Description
 * @see EFL_EVENT_DESCRIPTION
 */
#define EFL_EVENT_DESCRIPTION_HOT(name) { name, EINA_TRUE, EINA_FALSE, EINA_FALSE }

/**
 * @def EFL_EVENT_DESCRIPTION(name)
 * An helper macro to help populating #Efl_Event_Description
 * @param name The name of the event.
 * @see Efl_Event_Description
 */
#define EFL_EVENT_DESCRIPTION_RESTART(name) { name, EINA_FALSE, EINA_FALSE, EINA_TRUE }

/**
 * @def EFL_EVENT_DESCRIPTION_HOT(name)
 * An helper macro to help populating #Efl_Event_Description and make
 * the event impossible to freeze.
 * @param name The name of the event.
 * @see Efl_Event_Description
 * @see EFL_EVENT_DESCRIPTION
 */
#define EFL_EVENT_DESCRIPTION_HOT_RESTART(name) { name, EINA_TRUE, EINA_FALSE, EINA_TRUE }



/**
 * @}
 */

/**
 * @addtogroup Efl_Class Efl Class
 * @{
 */

/**
 * @def EFL_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...)
 * A convenience macro to be used for creating the class_get function. This
 * macro is fairly simple but should still be used as it'll let us improve
 * things easily.
 * @param class_get_func_name the name of the wanted class_get function name.
 * @param class_desc the class description.
 * @param parent_class The parent class for the function. Look at efl_class_new() for more information.
 * @param ... List of extensions. Look at efl_class_new() for more information.
 *
 * You must use this macro if you want thread safety in class creation.
 */
#define EFL_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...) \
const Efl_Class * \
class_get_func_name(void) \
{ \
   const Efl_Class *_tmp_parent_class; \
   static const Efl_Class * volatile _my_class = NULL; \
   static unsigned int _my_init_generation = 1; \
   if (EINA_UNLIKELY(_efl_object_init_generation != _my_init_generation)) \
     { \
        _my_class = NULL; /* It's freed in efl_object_shutdown(). */ \
     } \
   if (EINA_LIKELY(!!_my_class)) return _my_class; \
   \
   eina_lock_take(&_efl_class_creation_lock); \
   if (!!_my_class) \
     { \
        eina_lock_release(&_efl_class_creation_lock); \
        return _my_class; \
     } \
   _tmp_parent_class = parent_class; \
   _my_class = efl_class_new(class_desc, _tmp_parent_class, __VA_ARGS__); \
   _my_init_generation = _efl_object_init_generation; \
   eina_lock_release(&_efl_class_creation_lock); \
   \
   return _my_class; \
}


/**
 * An enum representing the possible types of an Eo class.
 */
enum _Efl_Class_Type
{
   EFL_CLASS_TYPE_REGULAR = 0, /**< Regular class. */
   EFL_CLASS_TYPE_REGULAR_NO_INSTANT, /**< Regular non instant-able class. */
   EFL_CLASS_TYPE_INTERFACE, /**< Interface */
   EFL_CLASS_TYPE_MIXIN /**< Mixin */
};

/**
 * @typedef Efl_Class_Type
 * A convenience typedef for #_Efl_Class_Type.
 */
typedef enum _Efl_Class_Type Efl_Class_Type;

/**
 * @def EO_VERSION
 * The current version of EO.
 */
#define EO_VERSION 2

typedef struct _Efl_Op_Description
{
   void *api_func;         /**< The EAPI function offering this op. (The name of the func on windows) */
   void *func;             /**< The static function to call for the op. */
} Efl_Op_Description;

/**
 * @struct _Efl_Object_Ops
 *
 * This struct holds the ops and the size of the ops.
 */
typedef struct _Efl_Object_Ops
{
   const Efl_Op_Description *descs; /**< The op descriptions array of size count. */
   size_t count; /**< Number of op descriptions. */
} Efl_Object_Ops;

/**
 * @struct _Efl_Class_Description
 * This struct holds the description of a class.
 * This description should be passed to efl_class_new.
 */
struct _Efl_Class_Description
{
   unsigned int version; /**< The current version of eo, use #EO_VERSION */
   const char *name; /**< The name of the class. */
   Efl_Class_Type type; /**< The type of the class. */
   size_t data_size; /**< The size of data (private + protected + public) this class needs per object. */
   Eina_Bool (*class_initializer)(Efl_Class *klass); /**< The initializer for the class */
   void (*class_constructor)(Efl_Class *klass); /**< The constructor of the class. */
   void (*class_destructor)(Efl_Class *klass); /**< The destructor of the class. */
};

/**
 * @typedef Efl_Class_Description
 * A convenience typedef for #_Efl_Class_Description
 */
typedef struct _Efl_Class_Description Efl_Class_Description;

/**
 * @brief Create a new class.
 * @param desc the class description to create the class with.
 * @param parent the class to inherit from.
 * @param ... A NULL terminated list of extensions (interfaces, mixins and the classes of any composite objects).
 * @return The new class's handle on success, or NULL otherwise.
 *
 * @note There are two types of extensions, mixins and none-mixins.
 *       Mixins are inheriting api AND the implementation.
 *       Non-mixins only inherit the api, so a class which inherits a non-mixin as extension must implement the api.
 *
 * You should use #EFL_DEFINE_CLASS. It'll provide thread safety and other
 * features easily.
 *
 * @see #EFL_DEFINE_CLASS
 */
EAPI const Efl_Class *efl_class_new(const Efl_Class_Description *desc, const Efl_Class *parent, ...);

/**
 * @brief Set the functions of a class
 * @param klass_id the class whose functions we are setting.
 * @param object_ops The function structure we are setting for object functions
 * @param class_ops The function structure we are setting for class functions
 * @return True on success, False otherwise.
 *
 * This should only be called from within the initializer function.
 *
 * @see #EFL_DEFINE_CLASS
 */
EAPI Eina_Bool efl_class_functions_set(const Efl_Class *klass_id, const Efl_Object_Ops *object_ops, const Efl_Object_Ops *class_ops);

/**
 * @brief Override Eo functions of this object.
 * @param ops The op description to override with.
 * @return true on success, false otherwise.
 *
 * This lets you override all of the Eo functions of this object (this
 * one included) and repalce them with ad-hoc implementation.
 * The contents of the array are copied so they can for example reside
 * on the stack.
 *
 * You are only allowed to override functions that are defined in the
 * class or any of its interfaces (that is, efl_isa returning true).
 *
 * If @p ops is #NULL, this will revert the @p obj to its original class
 * without any function overrides.
 *
 * It is not possible to override a function table of an object when it's
 * already been overridden. Call efl_object_override(obj, NULL) first if you really
 * need to do that.
 *
 * @see EFL_OPS_DEFINE
 */
EAPI Eina_Bool efl_object_override(Eo *obj, const Efl_Object_Ops *ops);

/**
 * @brief Define an array of override functions for @ref efl_object_override
 * @param ops A name for the Efl_Object_Ops local variable to define
 * @param ... A comma separated list of Efl_Object_Op overrides, using
 *            #EFL_OBJECT_OP_FUNC or #EFL_OBJECT_OP_CLASS_FUNC
 *
 * This can be used as follows:
 * @code
 * EFL_OPS_DEFINE(ops, EFL_OBJECT_OP_FUNC(public_func, _my_func));
 * efl_object_override(obj, &ops);
 * @endcode
 *
 * @see efl_object_override
 */
#define EFL_OPS_DEFINE(ops, ...) \
   const Efl_Op_Description _##ops##_descs[] = { __VA_ARGS__ }; \
   const Efl_Object_Ops ops = { _##ops##_descs, EINA_C_ARRAY_LENGTH(_##ops##_descs) }

/**
 * @brief Check if an object "is a" klass.
 * @param obj The object to check
 * @param klass The klass to check against.
 * @return @c EINA_TRUE if obj implements klass, @c EINA_FALSE otherwise.
 *
 * Notice: This function does not support composite objects.
 */
EAPI Eina_Bool efl_isa(const Eo *obj, const Efl_Class *klass);

/**
 * @brief Gets the name of the passed class.
 * @param klass the class to work on.
 * @return The class's name.
 *
 * @see efl_class_get()
 */
EAPI const char *efl_class_name_get(const Efl_Class *klass);

/**
 * @brief Gets a debug name for this object
 * @param obj_id The object (or class)
 * @return A name to use in logs and for other debugging purposes
 *
 * Note that subclasses can override Efl.Object "debug_name_override" to
 * provide more meaningful debug strings. The standard format includes the
 * class name, the object ID (this @p obj_id), the reference count and
 * optionally the object name (as defined by Efl.Object.name).
 *
 * This might return a temporary string, as created by eina_slstr, which means
 * that a main loop should probably be running.
 *
 * @since 1.21
 */
EAPI const char *efl_debug_name_get(const Eo *obj_id);

/**
 * @}
 */

/**
 * @brief Init the eo subsystem
 * @return @c EINA_TRUE if eo is init, @c EINA_FALSE otherwise.
 *
 * @see eo_shutfown()
 */
EAPI Eina_Bool efl_object_init(void);

/**
 * @brief Shutdown the eo subsystem
 * @return @c EINA_TRUE if eo is init, @c EINA_FALSE otherwise.
 *
 * @see efl_object_init()
 */
EAPI Eina_Bool efl_object_shutdown(void);


/**
 * The virtual allocation domain where an object lives
 *
 * You cannot mix objects between domains in the object tree or as direct
 * or indirect references unless you explicitly handle it and ensure the
 * other domain is adopted into your local thread space
 */
typedef enum
{
   EFL_ID_DOMAIN_INVALID = -1, /**< Invalid */
   EFL_ID_DOMAIN_MAIN    =  0, /**< The main loop domain where eo_init() is called */
   EFL_ID_DOMAIN_SHARED  =  1, /**< A special shared domain that all threads can see but has extra locking and unlocking costs to access */
   EFL_ID_DOMAIN_THREAD /**< The normal domain for threads so they can adopt the main loop domain at times */
   /* One more slot for future expansion here - maybe fine-grain locked objs */
} Efl_Id_Domain;

/**
 * @typedef Efl_Domain_Data
 * An opaque handle for private domain data
 */
typedef struct _Efl_Domain_Data Efl_Domain_Data;

/**
 * @brief Get the native domain for the current thread
 *
 * @return The native domain
 *
 * This will return the native eo object allocation domain for the current
 * thread. This can only be changed with efl_domain_switch() and this can
 * only be called before any objects are created/allocated on the thread
 * where it is called. Calling it after this point will result in
 * undefined behavior, so be sure to call this immediaetly after a thread
 * begins to execute, before anything else. You must not change the domain
 * of the main thread.
 * 
 * @see efl_domain_switch()
 * @see efl_domain_current_get()
 * @see efl_domain_current_set()
 * @see efl_domain_current_push()
 * @see efl_domain_current_pop()
 * @see efl_domain_data_get()
 * @see efl_domain_data_adopt()
 * @see efl_domain_data_return()
 * @see efl_compatible()
 */
EAPI Efl_Id_Domain    efl_domain_get(void);

/**
 * @brief Switch the native domain for the current thread
 * @param domain The domain to switch to
 * @return EINA_TRUE if the switch succeeds, and EINA_FALSE if it fails
 *
 * Permanently switch the native domain for new objects for the calling
 * thread. All objects created on this thread UNLESS it has switched to a
 * new domain temporarily with efl_domain_current_set(),
 * efl_domain_current_push() or efl_domain_current_pop(),
 * efl_domain_data_adopt() and efl_domain_data_return().
 * 
 * @see efl_domain_get()
 */
EAPI Eina_Bool        efl_domain_switch(Efl_Id_Domain domain);

/**
 * @brief Get the current domain used for allocating new objects
 * @return The current domain
 *
 * Get the currently used domain that is at the top of the domain stack.
 * There is actually a stack of domans to use you can alter via
 * efl_domain_current_push() and efl_domain_current_pop(). This only gets
 * the domain for the current thread.
 * 
 * @see efl_domain_get()
 */
EAPI Efl_Id_Domain    efl_domain_current_get(void);

/**
 * @brief Set the current domain used for allocating new objects
 * @return EINA_TRUE if it succeeds, and EINA_FALSE on failure
 *
 * Temporarily switch the current domain being used for allocation. There
 * is actually a stack of domans to use you can alter via
 * efl_domain_current_push() and efl_domain_current_pop(). The current
 * domain is the one ont he top of the stack, so this entry is altered
 * without pushing or popping. This only applies to the calling thread.
 * 
 * @see efl_domain_get()
 */
EAPI Eina_Bool        efl_domain_current_set(Efl_Id_Domain domain);

/**
 * @brief Push a new domain onto the domain stack
 * @param domain The domain to push
 * @return EINA_TRUE if it succeeds, and EINA_FALSE on failure
 *
 * This pushes a domain on the domain stack that can be popped later with
 * efl_domain_current_pop(). If the stack is full this may fail and return
 * EINA_FALSE in that case. This applies only to the calling thread.
 * 
 * @see efl_domain_get()
 */
EAPI Eina_Bool        efl_domain_current_push(Efl_Id_Domain domain);

/**
 * @brief Pop a previously pushed domain from the domain stack
 *
 * This pops the top domain off the domain stack for the calling thread
 * that was pushed with efl_domain_current_push().
 * 
 * @see efl_domain_get()
 */
EAPI void             efl_domain_current_pop(void);

/**
 * @brief Get an opaque handle to the local native domain eoid data
 * @return A handle to the local native domain data or NULl on failure
 *
 * This gets a handle to the domain data for the current thread, intended
 * to be used by another thread to adopt with efl_domain_data_adopt().
 * Once you use efl_domain_data_adopt() the thread that called
 * efl_domain_data_get() should suspend and not execute anything
 * related to eo or efl objects until the thread that adopted the data
 * called efl_domain_data_return() to return the data to its owner and
 * stop making it available to that thread.
 *
 * @see efl_domain_get()
 */
EAPI Efl_Domain_Data *efl_domain_data_get(void);

/**
 * @brief Adopt a single extra domain to be the current domain
 * @param datas_in The domain data to adopt
 * @return The domain that was adopted or EFL_ID_DOMAIN_INVALID on failure
 *
 * This will adopt the given domain data pointed to by @p data_in
 * as an extra domain locally. The adopted domain must have a domain ID
 * that is not the same as the current thread domain or local domain. You
 * may not adopt a domain that clashes with the current domain. If you
 * set, push or pop domains so these might clash (be the same) then
 * undefined behaviour will occur.
 *
 * This will also push the adopted domain as the current domain so that
 * all newly created objects (unless their parent is of a differing domain)
 * will be part of this adopted domain. You can still access objects from
 * your local domain as well, but be aware that creation will require
 * some switch of domain by push, pop or set. Return the domain with
 * efl_domain_data_return() when done.
 * 
 * @see efl_domain_get()
 */
EAPI Efl_Id_Domain    efl_domain_data_adopt(Efl_Domain_Data *data_in);

/**
 * @brief Return a domain to its original owning thread
 * @param domain The domain to return
 * @return EINA_TRUE on success EINA_FALSE on failure
 *
 * This returns the domain specified by @p domain to the thread it came
 * from, allowing that thread after this to continue execution. This
 * will implicitly pop the current domain from the stack, assuming that
 * the current domain is the same one pushed implicitly by
 * efl_domain_data_adopt(). You cannot return your own native local
 * domain, only the one that was adopted by efl_domain_data_adopt().
 * 
 * @see efl_domain_get()
 */
EAPI Eina_Bool        efl_domain_data_return(Efl_Id_Domain domain);

/**
 * @prief Check if 2 objects are compatible
 * @param obj The basic object
 * @param obj_target The alternat object that may be referenced by @p obj
 * @return EINA_TRUE if compatible, EINA_FALSE if not
 *
 * This checks to see if 2 objects are compatible and could be parent or
 * children of eachother, could reference eachother etc.. There is only a
 * need to call this if you got objects from multiple domains (an
 * adopted domain with efl_domain_data_adopt() or the shared domain
 * EFL_ID_DOMAIN_SHARED where objects may be accessed by any thread).
 * 
 * @see efl_domain_get()
 */
EAPI Eina_Bool        efl_compatible(const Eo *obj, const Eo *obj_target);




// to fetch internal function and object data at once
typedef struct _Efl_Object_Op_Call_Data
{
   Eo           *eo_id;
   _Eo_Object   *obj;
   void         *func;
   void         *data;
   void         *extn1; // for the future to avoid ABI issues
   void         *extn2; // for the future to avoid ABI issues
   void         *extn3; // for the future to avoid ABI issues
   void         *extn4; // for the future to avoid ABI issues
} Efl_Object_Op_Call_Data;

#define EFL_OBJECT_CALL_CACHE_SIZE 1

typedef struct _Efl_Object_Call_Cache_Index
{
   const void       *klass;
} Efl_Object_Call_Cache_Index;

typedef struct _Efl_Object_Call_Cache_Entry
{
   const void       *func;
} Efl_Object_Call_Cache_Entry;

typedef struct _Efl_Object_Call_Cache_Off
{
   int               off;
} Efl_Object_Call_Cache_Off;

typedef struct _Efl_Object_Call_Cache
{
#if EFL_OBJECT_CALL_CACHE_SIZE > 0
   Efl_Object_Call_Cache_Index index[EFL_OBJECT_CALL_CACHE_SIZE];
   Efl_Object_Call_Cache_Entry entry[EFL_OBJECT_CALL_CACHE_SIZE];
   Efl_Object_Call_Cache_Off   off  [EFL_OBJECT_CALL_CACHE_SIZE];
# if EFL_OBJECT_CALL_CACHE_SIZE > 1
   int                 next_slot;
# endif
#endif
   Efl_Object_Op               op;
   unsigned int        generation;
} Efl_Object_Call_Cache;

// to pass the internal function call to EFL_FUNC_BODY (as Func parameter)
#define EFL_FUNC_CALL(...) __VA_ARGS__

#ifndef _WIN32
# define EFL_FUNC_COMMON_OP_FUNC(Name) ((const void *) Name)
#else
# define EFL_FUNC_COMMON_OP_FUNC(Name) ((const void *) #Name)
#endif

#ifdef _MSC_VER
# define EFL_FUNC_TLS __declspec(thread)
#else
# define EFL_FUNC_TLS __thread
#endif

// cache OP id, get real fct and object data then do the call
#define EFL_FUNC_COMMON_OP(Obj, Name, DefRet) \
   static EFL_FUNC_TLS Efl_Object_Call_Cache ___cache; /* static 0 by default */ \
   Efl_Object_Op_Call_Data ___call; \
   _Eo_##Name##_func _func_;                                            \
   if (EINA_UNLIKELY((___cache.op == EFL_NOOP) ||                       \
                     (___cache.generation != _efl_object_init_generation))) \
     goto __##Name##_op_create; /* yes a goto - see below */ \
   __##Name##_op_create_done: \
   if (!_efl_object_call_resolve((Eo *) Obj, #Name, &___call, &___cache, \
                                 __FILE__, __LINE__)) return DefRet; \
   _func_ = (_Eo_##Name##_func) ___call.func;

// yes this looks ugly with gotos BUT it moves rare "init" handling code
// out of the hot path and thus l1 instruction cach prefetch etc. so it
// should provide a micro-speedup. this has been shown to have real big
// measurable effects on very hot code paths as l1 instgruction cache
// does matter and fetching a cacheline of code may fetch a lot of rarely
// used instructions that are skipepd by and if so moving those away out
// of the cacheline that was already fetched should yield better cache
// hits.
#define EFL_FUNC_COMMON_OP_END(Obj, Name, DefRet) \
__##Name##_op_create: \
   ___cache.op = _efl_object_op_api_id_get(EFL_FUNC_COMMON_OP_FUNC(Name), Obj, #Name, __FILE__, __LINE__); \
   if (___cache.op == EFL_NOOP) return DefRet; \
   ___cache.generation = _efl_object_init_generation; \
   goto __##Name##_op_create_done;

#define _EFL_OBJECT_API_BEFORE_HOOK
#define _EFL_OBJECT_API_AFTER_HOOK
#define _EFL_OBJECT_API_CALL_HOOK(x) x

// to define an EAPI function
#define _EFL_OBJECT_FUNC_BODY(Name, ObjType, Ret, DefRet) \
  Ret \
  Name(ObjType obj) \
  { \
     typedef Ret (*_Eo_##Name##_func)(Eo *, void *obj_data); \
     Ret _r; \
     EFL_FUNC_COMMON_OP(obj, Name, DefRet); \
     _EFL_OBJECT_API_BEFORE_HOOK \
     _r = _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK \
     return _r; \
     EFL_FUNC_COMMON_OP_END(obj, Name, DefRet); \
  }

#define _EFL_OBJECT_VOID_FUNC_BODY(Name, ObjType) \
  void \
  Name(ObjType obj) \
  { \
     typedef void (*_Eo_##Name##_func)(Eo *, void *obj_data); \
     EFL_FUNC_COMMON_OP(obj, Name, ); \
     _EFL_OBJECT_API_BEFORE_HOOK \
     _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK \
     return; \
     EFL_FUNC_COMMON_OP_END(obj, Name, ); \
  }

#define _EFL_OBJECT_FUNC_BODYV(Name, ObjType, Ret, DefRet, Arguments, ...) \
  Ret \
  Name(ObjType obj, __VA_ARGS__) \
  { \
     typedef Ret (*_Eo_##Name##_func)(Eo *, void *obj_data, __VA_ARGS__); \
     Ret _r; \
     EFL_FUNC_COMMON_OP(obj, Name, DefRet); \
     _EFL_OBJECT_API_BEFORE_HOOK \
     _r = _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data, Arguments)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK \
     return _r; \
     EFL_FUNC_COMMON_OP_END(obj, Name, DefRet); \
  }

#define _EFL_OBJECT_VOID_FUNC_BODYV(Name, ObjType, Arguments, ...) \
  void \
  Name(ObjType obj, __VA_ARGS__) \
  { \
     typedef void (*_Eo_##Name##_func)(Eo *, void *obj_data, __VA_ARGS__); \
     EFL_FUNC_COMMON_OP(obj, Name, ); \
     _EFL_OBJECT_API_BEFORE_HOOK \
     _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data, Arguments)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK \
     return; \
     EFL_FUNC_COMMON_OP_END(obj, Name, ); \
  }

#define EFL_FUNC_BODY(Name, Ret, DefRet) _EFL_OBJECT_FUNC_BODY(Name, Eo *, Ret, DefRet)
#define EFL_VOID_FUNC_BODY(Name) _EFL_OBJECT_VOID_FUNC_BODY(Name, Eo *)
#define EFL_FUNC_BODYV(Name, Ret, DefRet, Arguments, ...) _EFL_OBJECT_FUNC_BODYV(Name, Eo *, Ret, DefRet, EFL_FUNC_CALL(Arguments), __VA_ARGS__)
#define EFL_VOID_FUNC_BODYV(Name, Arguments, ...) _EFL_OBJECT_VOID_FUNC_BODYV(Name, Eo *, EFL_FUNC_CALL(Arguments), __VA_ARGS__)

#define EFL_FUNC_BODY_CONST(Name, Ret, DefRet) _EFL_OBJECT_FUNC_BODY(Name, const Eo *, Ret, DefRet)
#define EFL_VOID_FUNC_BODY_CONST(Name) _EFL_OBJECT_VOID_FUNC_BODY(Name, const Eo *)
#define EFL_FUNC_BODYV_CONST(Name, Ret, DefRet, Arguments, ...) _EFL_OBJECT_FUNC_BODYV(Name, const Eo *, Ret, DefRet, EFL_FUNC_CALL(Arguments), __VA_ARGS__)
#define EFL_VOID_FUNC_BODYV_CONST(Name, Arguments, ...) _EFL_OBJECT_VOID_FUNC_BODYV(Name, const Eo *, EFL_FUNC_CALL(Arguments), __VA_ARGS__)

#ifndef _WIN32
# define _EFL_OBJECT_OP_API_ENTRY(a) (void*)a
#else
# define _EFL_OBJECT_OP_API_ENTRY(a) #a
#endif

#define EFL_OBJECT_OP_FUNC(_api, _private) { _EFL_OBJECT_OP_API_ENTRY(_api), (void*)_private }

// returns the OP id corresponding to the given api_func
EAPI Efl_Object_Op _efl_object_api_op_id_get(const void *api_func) EINA_DEPRECATED;
EAPI Efl_Object_Op _efl_object_op_api_id_get(const void *api_func, const Eo *obj, const char *api_func_name, const char *file, int line) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT;

// gets the real function pointer and the object data
EAPI Eina_Bool _efl_object_call_resolve(Eo *obj, const char *func_name, Efl_Object_Op_Call_Data *call, Efl_Object_Call_Cache *callcache, const char *file, int line);

// end of the eo call barrier, unref the obj
EAPI void _efl_object_call_end(Efl_Object_Op_Call_Data *call);

// end of the efl_add. Calls finalize among others
EAPI Eo * _efl_add_end(Eo *obj, Eina_Bool is_ref, Eina_Bool is_fallback);

/*****************************************************************************/

/**
 * @brief Prepare a call to the parent class implementation of a function
 *
 * @param obj        The object to call (can be a class)
 * @param cur_klass  The current class
 * @return An EO handle that must be used as part of an EO function call.
 *
 * @warning The returned value must always be used as the first argument (the
 * object) of a method or property function call, and should never be handled
 * in any other way. Do not call any function from this file on the returned
 * value (eg. efl_ref, etc...).
 *
 * Usage:
 * @code
 * // Inside the implementation code for MY_CLASS
 * my_property_set(efl_super(obj, MY_CLASS), value);
 * @endcode
 *
 * A common usage pattern is to forward function calls to the parent function:
 * @code
 * EOLIAN static void
 * _my_class_my_property_set(Eo *obj, My_Class_Data *pd, int value)
 * {
 *   // Do some processing on this class data, or on the value
 *   if (value < 0) value = 0;
 *   pd->last_value = value;
 *   // Pass the call to the parent class
 *   my_property_set(efl_super(obj, MY_CLASS), value);
 *   // Do some more processing
 * }
 * @endcode
 *
 * @p cur_klass must be a valid class in the inheritance hierarchy of @p obj's
 * class. Invalid values will lead to undefined behaviour.
 *
 * @see efl_cast
 */
EAPI Eo *efl_super(const Eo *obj, const Efl_Class *cur_klass);

/**
 * @brief Prepare a call to cast to a parent class implementation of a function
 *
 * @param obj        The object to call (can be a class)
 * @param cur_klass  The class to cast into
 * @return An EO handle that must be used as part of an EO function call.
 *
 * @warning The returned value must always be used as the first argument (the
 * object) of a method or property function call, and should never be handled
 * in any other way. Do not call any function from this file on the returned
 * value (eg. efl_ref, etc...).
 *
 * Usage:
 * @code
 * // Inside the implementation code for MY_CLASS
 * my_property_set(efl_cast(obj, SOME_OTHER_CLASS), value);
 * @endcode
 *
 * In the above example, @p obj is assumed to inherit from @c SOME_OTHER_CLASS
 * as either a mixin or direct class inheritance. If @c SOME_OTHER_CLASS
 * implements @c my_property.set then that implementation shall be called,
 * otherwise the call will be propagated to the parent implementation (if any).
 *
 * @p cur_klass must be a valid class in the inheritance hierarchy of @p obj's
 * class. Invalid values will lead to undefined behaviour.
 *
 * @see efl_cast
 *
 * @since 1.20
 */
EAPI Eo *efl_cast(const Eo *obj, const Efl_Class *cur_klass);

/*****************************************************************************/

/**
 * @brief Gets the class of the object.
 * @param obj The object to work on
 * @return The object's class.
 *
 * @see efl_class_name_get()
 */
EAPI const Efl_Class *efl_class_get(const Eo *obj);

EAPI Eo *_efl_added_get(void);

/* Check if GCC compatible (both GCC and clang define this) */
#if defined(__GNUC__) && !defined(_EO_ADD_FALLBACK_FORCE)

# define efl_added __efl_added

# define _efl_add_common(klass, parent, is_ref, ...) \
   ({ \
     Eo * const __efl_added = _efl_add_internal_start(__FILE__, __LINE__, klass, parent, is_ref, EINA_FALSE); \
     (void) ((void)0, ##__VA_ARGS__);                                   \
     (Eo *) _efl_add_end(efl_added, is_ref, EINA_FALSE); \
    })

#else

# define efl_added _efl_added_get()

# define _efl_add_common(klass, parent, is_ref, ...) \
   ( \
     _efl_add_internal_start(__FILE__, __LINE__, klass, parent, is_ref, EINA_TRUE), \
     ##__VA_ARGS__, \
     (Eo *) _efl_add_end(efl_added, is_ref, EINA_TRUE) \
   )

#endif

/**
 * @def efl_add
 * @brief Create a new object and call its constructor(If it exits).
 *
 * The object returned by this function will always have 1 ref
 * (reference count) irrespective of whether the parent is NULL or
 * not.
 * If the object is created using this function, then it would
 * automatically gets deleted when the parent object is deleted.
 * There is no need to call efl_unref on the child. This is convenient
 * in C.
 *
 * If you want a more "consistent" behaviour, take a look at #efl_add_ref.
 *
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @param ... The ops to run.
 * @return An handle to the new object on success, NULL otherwise.
 */
#define efl_add(klass, parent, ...) _efl_add_common(klass, parent, EINA_FALSE, ##__VA_ARGS__)

/**
 * @def efl_add_ref
 * @brief Create a new object and call its constructor(If it exists).
 *
 * The object returned by this function has 1 ref for itself, 1 ref from the
 * parent (if exists) and possible other refs if were added during construction.
 * If a child object is created using this, then it won't get deleted
 * when the parent object is deleted until you manually remove the ref
 * by calling efl_unref().
 *
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @param ... The ops to run.
 * @return An handle to the new object on success, NULL otherwise.
 */
#define efl_add_ref(klass, parent, ...) _efl_add_common(klass, parent, EINA_TRUE, ##__VA_ARGS__)

EAPI Eo * _efl_add_internal_start(const char *file, int line, const Efl_Class *klass_id, Eo *parent, Eina_Bool ref, Eina_Bool is_fallback);

/**
 * @brief Get a pointer to the data of an object for a specific class.
 *
 * The data reference count is not incremented. The pointer must be used only
 * in the scope of the function and its callees.
 *
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @return a pointer to the data.
 *
 * @see efl_data_ref()
 * @see efl_data_unref()
 * @see efl_data_scope_safe_get()
 */
EAPI void *efl_data_scope_get(const Eo *obj, const Efl_Class *klass);

/**
 * @brief Safely get a pointer to the data of an object for a specific class.
 *
 * This call runs a dynamic check and returns NULL if there is no valid data
 * to return.
 *
 * The data reference count is not incremented. The pointer must be used only
 * in the scope of the function and its callees. This function will return NULL
 * if there is no data for this class, or if this object is not an instance of
 * the given class. The function will return NULL if the data size is 0.
 * Note that objects of class A inheriting from another class C as an
 * interface (like: class A(B, C) {} ) will have no data for class C. This
 * means that efl_isa(a, C) will return true but there is no data for C. This
 * function's behaviour is similar to efl_data_scope_get() when running in
 * debug mode (but this prints less error logs).
 *
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @return a pointer to the data or NULL in case of error or $obj was NULL.
 *
 * @see efl_data_scope_get()
 *
 * @since 1.20
 */
EAPI void *efl_data_scope_safe_get(const Eo *obj, const Efl_Class *klass);

/**
 * @def efl_data_xref(obj, klass, ref_obj)
 * Use this macro if you want to associate a referencer object.
 * Convenience macro around efl_data_xref_internal()
 */
#define efl_data_xref(obj, klass, ref_obj) efl_data_xref_internal(__FILE__, __LINE__, obj, klass, ref_obj)

/**
 * @def efl_data_ref(obj, klass)
 * Use this macro if you don't want to associate a referencer object.
 * Convenience macro around efl_data_xref_internal()
 */
#define efl_data_ref(obj, klass) efl_data_xref_internal(__FILE__, __LINE__, obj, klass, (const Eo *)obj)

/**
 * @brief Get a pointer to the data of an object for a specific class and
 * increment the data reference count.
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @param ref_obj the object that references the data.
 * @param file the call's filename.
 * @param line the call's line number.
 * @return a pointer to the data.
 *
 * @see efl_data_xunref_internal()
 */
EAPI void *efl_data_xref_internal(const char *file, int line, const Eo *obj, const Efl_Class *klass, const Eo *ref_obj);

/**
 * @def efl_data_xunref(obj, data, ref_obj)
 * Use this function if you used efl_data_xref to reference the data.
 * Convenience macro around efl_data_xunref_internal()
 * @see efl_data_xref()
 */
#define efl_data_xunref(obj, data, ref_obj) efl_data_xunref_internal(obj, data, ref_obj)

/**
 * @def efl_data_unref(obj, data)
 * Use this function if you used efl_data_ref to reference the data.
 * Convenience macro around efl_data_unref_internal()
 * @see efl_data_ref()
 */
#define efl_data_unref(obj, data) efl_data_xunref_internal(obj, data, obj)

/**
 * @brief Decrement the object data reference count by 1.
 * @param obj the object to work on.
 * @param data a pointer to the data to unreference
 * @param file the call's filename.
 * @param line the call's line number.
 *
 * @see efl_data_xref_internal()
 */
EAPI void efl_data_xunref_internal(const Eo *obj, void *data, const Eo *ref_obj);

/**
 * @brief Increment the object's reference count by 1.
 * @param obj the object to work on.
 * @return The object passed.
 *
 * It's very easy to get a refcount leak and start leaking memory because
 * of a forgotten unref or an extra ref. That is why there are efl_xref
 * and efl_xunref that will make debugging easier in such a case.
 * Therefor, these functions should only be used in small scopes, i.e at the
 * start of some section in which the object may get freed, or if you know
 * what you are doing.
 *
 * @see efl_unref()
 * @see efl_ref_get()
 */
EAPI Eo *efl_ref(const Eo *obj);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed.
 * @param obj the object to work on.
 *
 * @see efl_ref()
 * @see efl_ref_get()
 */
EAPI void efl_unref(const Eo *obj);

/**
 * @brief Return the ref count of the object passed.
 * @param obj the object to work on.
 * @return the ref count of the object.
 *
 * @see efl_ref()
 * @see efl_unref()
 */
EAPI int efl_ref_get(const Eo *obj);

/**
 * @brief Set a deletion interceptor function
 * @param obj The object to set the interceptor on
 * @param del_intercept_func The interceptor function to call
 *
 * This sets the function @p del_intercept_func to be called when an object
 * is about to go from a reference count of 1 to 0, thus triggering actual
 * destruction of the object. Instead of going to a reference count of 0 and
 * being destroyed, the object will stay alive with a reference count of 1
 * and this intercept function will be called instead. It is the job of
 * this interceptor function to handle any further deletion of of the object
 * from here.
 *
 * Note that by default objects have no interceptor function set, and thus
 * will be destroyed as normal. To return an object to this state, simply
 * set the @p del_intercept_func to NULL which is the default.
 *
 * A good use for this feature is to ensure an object is destroyed by its
 * owning main loop and not in a foreign loop. This makes it possible to
 * safely unrefor delete objects from any loop as an interceptor can be set
 * on an object that will abort destruction and instead queue the object
 * on its owning loop to be destroyed at some time in the future and now
 * set the intercept function to NULL so it is not called again on the next
 * "real deletion".
 * 
 * @see efl_del_intercept_get()
 * @see efl_unref()
 * @see efl_del()
 */
EAPI void efl_del_intercept_set(Eo *obj, Efl_Del_Intercept del_intercept_func);

/**
 * @brief Get the deletion interceptor function
 * @param obj The object to get the interceptor of
 * @return The intercept function or NULL if none is set.
 *
 * This returns the interceptor function set by efl_del_intercept_set(). Note
 * that objects by default have no interceptor (NULL) set, but certain
 * classes may set one up in a constructor, so it is important to be able
 * to get the interceptor function to know if this has happend and
 * if you want to override this interceptor, be sure to call it after your
 * own interceptor function has finished. It would generally be a bad idea
 * though to override these functions.
 * 
 * @see efl_del_intercept_set()
 */
EAPI Efl_Del_Intercept efl_del_intercept_get(const Eo *obj);

/**
 * @brief Clears the object so it can be reused (for example in a cache)
 * @param obj The object to mark for reusal
 *
 * This assumes the destructor has been called on the object, so it
 * should probably only be used from the del intercept.
 *
 * @see efl_del_intercept_set()
 */
EAPI void efl_reuse(const Eo *obj);

/**
 * @def efl_xref(obj, ref_obj)
 * Convenience macro around efl_xref_internal()
 * @see efl_xref()
 */
#define efl_xref(obj, ref_obj) efl_xref_internal(__FILE__, __LINE__, obj, ref_obj)

/**
 * @brief Increment the object's reference count by 1 (and associate the ref with ref_obj)
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 * @param file the call's filename.
 * @param line the call's line number.
 * @return The object passed (obj)
 *
 * People should not use this function, use #efl_xref instead.
 * A compile flag my make it and eobj_xunref() behave the same as eobj_ref()
 * and eobj_unref() respectively. So this should be used wherever possible.
 *
 * @see efl_xunref()
 */
EAPI Eo *efl_xref_internal(const char *file, int line, Eo *obj, const Eo *ref_obj);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed. Will free the ref associated with ref_obj).
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 *
 * This function only enforces the checks for object association. I.e don't rely
 * on it. If such enforces are compiled out, this function behaves the same as
 * efl_unref().
 *
 * @see efl_xref_internal()
 */
EAPI void efl_xunref(Eo *obj, const Eo *ref_obj);

/**
 * @brief Add a new weak reference to obj.
 *
 * This function registers the object handle pointed by wref to obj so when obj
 * is deleted it'll be updated to NULL. This functions should be used when you
 * want to keep track of an object in a safe way, but you don't want to prevent
 * it from being freed.
 *
 * @param[in] wref The weak ref
 */
EOAPI void efl_wref_add(Eo *obj, Efl_Object **wref);

/**
 * @brief Delete the weak reference passed.
 *
 * @param[in] wref The weak ref
 */
EOAPI void efl_wref_del(Eo *obj, Efl_Object **wref);


/**
 * @brief Generic data with string key on an object.
 *
 * The user is in charge of freeing the data.
 *
 * @param[in] key The key associated with the data
 * @param[in] data The data to set
 */
EOAPI void efl_key_data_set(Eo *obj, const char * key, const void *data);

/**
 * @brief Generic data with string key on an object.
 *
 * The user is in charge of freeing the data.
 *
 * @param[in] key The key associated with the data
 *
 * @return The data to set
 */
EOAPI void *efl_key_data_get(const Eo *obj, const char * key);

/**
 * @brief Generic object reference with string key to object.
 *
 * The object will be automatically ref'd when set and unref'd when replaced or
 * deleted or referring object is deleted. If the referenced object is deleted
 * then the key is deleted automatically.
 *
 * This is the same key store used by key_data and key_value so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the object ref
 * @param[in] objdata The object to set
 */
EOAPI void efl_key_ref_set(Eo *obj, const char * key, const Efl_Object *objdata);

/**
 * @brief Generic object reference with string key to object.
 *
 * The object will be automatically ref'd when set and unref'd when replaced or
 * deleted or referring object is deleted. If the referenced object is deleted
 * then the key is deleted automatically.
 *
 * This is the same key store used by key_data and key_value so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the object ref
 *
 * @return The object to set
 */
EOAPI Efl_Object *efl_key_ref_get(const Eo *obj, const char * key);

/**
 * @brief Generic weak object reference with string key to object.
 *
 * The object key will be removed if the object is removed, but will not take
 * or removed references like key_obj.
 *
 * This is the same key store used by key_data and key_value so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the object ref
 * @param[in] objdata The object to set
 */
EOAPI void efl_key_wref_set(Eo *obj, const char * key, const Efl_Object *objdata);

/**
 * @brief Generic weak object reference with string key to object.
 *
 * The object key will be removed if the object is removed, but will not take
 * or removed references like key_obj.
 *
 * This is the same key store used by key_data and key_value so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the object ref
 *
 * @return The object to set
 */
EOAPI Efl_Object *efl_key_wref_get(const Eo *obj, const char * key);

/**
 * @brief Value on with string key on the object.
 *
 * This stores the value with the given string key on the object and it will be
 * freed when replaced or deleted or the referring object is deleted.
 *
 * This is the same key store used by key_data and key_obj so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the value
 * @param[in] value The value to set
 */
EOAPI void efl_key_value_set(Eo *obj, const char * key, Eina_Value *value);

/**
 * @brief Value on with string key on the object.
 *
 * This stores the value with the given string key on the object and it will be
 * freed when replaced or deleted or the referring object is deleted.
 *
 * This is the same key store used by key_data and key_obj so keys are shared
 * and can store only one thing
 *
 * @param[in] key The key associated with the value
 *
 * @return The value to set
 */
EOAPI Eina_Value *efl_key_value_get(const Eo *obj, const char * key);

/**
 * @brief Enable or disable the manual free feature.
 * @param obj the object to work on.
 * @param manual_free indicates if the free is manual (EINA_TRUE) or automatic (EINA_FALSE).
 *
 * The developer is in charge to call the function efl_manual_free to free the memory allocated for this object.
 *
 * Do not use, unless you really know what you are doing. It's used by Evas
 * because evas wants to keep its private data available even after the object
 * is deleted. Setting this to true makes Eo destruct the object but not free
 * the private data or the object itself.
 *
 * @see efl_manual_free()
 */
EAPI void efl_manual_free_set(Eo *obj, Eina_Bool manual_free);

/**
 * @brief Frees the object.
 * @param obj the object to work on.
 * This function must be called by the developer if the function
 * efl_manual_free_set has been called before with the parameter EINA_TRUE.
 * An error will be printed if this function is called when the manual
 * free option is not set to EINA_TRUE or the number of refs is not 0.
 * @return EINA_TRUE if successfully freed. EINA_FALSE otherwise.
 *
 * @see efl_manual_free_set()
 */
EAPI Eina_Bool efl_manual_free(Eo *obj);

/**
 * @brief Checks if the object was already descructed (only relevant for manual_free objects).
 * @param obj the object to check.
 * This function checks if the object was already destructed (but not alraedy
 * freed). It should only be used with objects that are supposed to be manually
 * freed, but not yet freed (but possibly destructed).
 *
 * @see efl_manual_free_set()
 */
EAPI Eina_Bool efl_destructed_is(const Eo *obj);

/**
 * @addtogroup Efl_Class_Class Eo's Class class.
 * @{
 */

#include "efl_class.eo.h"

/**
 * @}
 */

/**
 * @addtogroup Efl_Class_Base Eo's Base class.
 * @{
 */

/**
 * @typedef efl_key_data_free_func
 * Data free func prototype.
 * XXX: DO NOT USE, only here for legacy.
 */
typedef void (*efl_key_data_free_func)(void *);

/**
 * @def efl_weak_ref
 * @brief Reference a pointer to an Eo object
 * @param wref the pointer to use for the weak ref
 *
 * @see efl_weak_unref
 * @see efl_wref_add
 */
#define efl_weak_ref(wref)			   \
  do {						   \
    if (*wref) efl_wref_add(*wref, wref);  \
  } while (0)

/**
 * @def efl_weak_unref
 * @brief Unreference a pointer to an Eo object
 * @param wref the pointer to use for the weak unref
 *
 * @see efl_weak_ref
 * @see efl_wref_del
 * @see efl_wref_del_safe
 */
#define efl_weak_unref(wref)			   \
  do {						   \
    if (*wref) efl_wref_del(*wref, wref);  \
  } while (0)

/**
 * @def efl_wref_del_safe
 * @brief Delete the weak reference passed.
 * @param wref the weak reference to free.
 *
 * @see #efl_wref_del
 */
#define efl_wref_del_safe(wref) efl_weak_unref(wref)

/**
 * @addtogroup Efl_Events Eo's Event Handling
 * @{
 */

/**
 * Don't use.
 * The values of the returned event structure are also internal, don't assume
 * anything about them.
 * @internal
 */
EAPI const Efl_Event_Description *efl_object_legacy_only_event_description_get(const char *_event_name);

/**
 * @def EFL_CALLBACK_PRIORITY_BEFORE
 * Slightly more prioritized than default.
 */
#define EFL_CALLBACK_PRIORITY_BEFORE -100
/**
 * @def EFL_CALLBACK_PRIORITY_DEFAULT
 * Default callback priority level
 */
#define EFL_CALLBACK_PRIORITY_DEFAULT 0
/**
 * @def EFL_CALLBACK_PRIORITY_AFTER
 * Slightly less prioritized than default.
 */
#define EFL_CALLBACK_PRIORITY_AFTER 100

/**
 * Helper for sorting callbacks array. Automatically used by
 * @ref EFL_CALLBACKS_ARRAY_DEFINE
 */
EAPI int efl_callbacks_cmp(const Efl_Callback_Array_Item *a, const Efl_Callback_Array_Item *b);

/**
 * Helper for creating global callback arrays.
 * The problem is on windows where you can't declare a static array with
 * external symbols in it, because the addresses are only known at runtime.
 * This also open up the possibility to automatically sort them for better
 * performance.
 */
#define EFL_CALLBACKS_ARRAY_DEFINE(Name, ...)                           \
  static Efl_Callback_Array_Item *                                      \
  Name(void)                                                            \
  {                                                                     \
     Efl_Callback_Array_Item tmp[] = { __VA_ARGS__ }; \
     static Efl_Callback_Array_Item internal[EINA_C_ARRAY_LENGTH(tmp) + 1] = \
       { { 0, 0 } };         \
     if (internal[0].desc == NULL)                                      \
       {                                                                \
          memcpy(internal, tmp, sizeof(tmp)); \
          qsort(internal, EINA_C_ARRAY_LENGTH(internal) - 1, sizeof (internal[0]), \
                (void*) efl_callbacks_cmp);                              \
       }                                                                \
     return internal;                                                   \
  }

/**
 * @def efl_event_callback_add(obj, desc, cb, data)
 * Add a callback for an event.
 * @param[in] desc The description of the event to listen to.
 * @param[in] cb the callback to call.
 * @param[in] data additional data to pass to the callback.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see efl_event_callback_priority_add()
 */
#define efl_event_callback_add(obj, desc, cb, data) \
   efl_event_callback_priority_add(obj, desc, \
         EFL_CALLBACK_PRIORITY_DEFAULT, cb, data)

/**
 * @def efl_event_callback_array_add(obj, desc, cb, data)
 * Add an array of callbacks for an event.
 *
 * @param[in] array an #Efl_Callback_Array_Item of events to listen to.
 * @param[in] data additional data to pass to the callback.
 *
 * Callbacks of the same priority are called in reverse order of creation.
 * The array should have been created by @ref EFL_CALLBACKS_ARRAY_DEFINE. If
 * that wasn't the case, be careful of portability issue and make sure that
 * it is properly sorted with @ref efl_callbacks_cmp.
 *
 * @see efl_event_callback_array_priority_add()
 */
#define efl_event_callback_array_add(obj, array, data) \
   efl_event_callback_array_priority_add(obj, array, \
         EFL_CALLBACK_PRIORITY_DEFAULT, data)

/**
 * @def Replace the previously Eo pointer with new content.
 *
 * @param storage The object to replace the old reference. It can not be @c NULL.
 * @param new_obj The new object. It may be @c NULL.
 *
 * The object pointed by @a storage must be previously an Eo or
 * @c NULL and it will be efl_unref(). The @a new_obj will be passed
 * to efl_ref() and then assigned to @c *storage.
 *
 * @see efl_ref()
 * @see efl_unref()
 */
static inline void
efl_replace(Eo **storage, Eo *new_obj)
{
   if (!storage || *storage == new_obj) return;
   efl_ref(new_obj);
   efl_unref(*storage);
   *storage = new_obj;
}

EOAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_OBJECT;

/**
 * @}
 */

#include "efl_future.h"

/**
 * @addtogroup Eo_Iterators Eo iterators
 * @{
 */

/**
 * @brief Get an iterator on the Eo classes
 *
 * You can use this function to walk over the Eo classes.
 *
 * @return an iterator on success, NULL otherwise
 */
EAPI Eina_Iterator *eo_classes_iterator_new(void);

/**
 * @brief Get an iterator on the Eo objects
 *
 * You can use this function to walk over the Eo objects.
 *
 * @return an iterator on success, NULL otherwise
 */
EAPI Eina_Iterator *eo_objects_iterator_new(void);

/**
 * @}
 */

/**
 * @}
 */


   /* Private for EFL internal use only. Do not use these! */
EAPI int ___efl_ref2_get(const Eo *obj_id);
EAPI void ___efl_ref2_reset(const Eo *obj_id);

#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
