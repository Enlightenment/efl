#ifndef EOBJ_H
#define EOBJ_H

#include <stdarg.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EOBJ_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EOBJ_BUILD */
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

/**
 * @var _eobj_class_creation_lock
 * This variable is used for locking purposes in the class_get function
 * defined in #EOBJ_DEFINE_CLASS.
 * This is just to work around the fact that we need to init locks before
 * using them.
 * Don't touch it if you don't know what you are doing.
 * @internal
 */
EAPI extern Eina_Lock _eobj_class_creation_lock;

/**
 * @defgroup Eobj Eobj Generic Object System
 *
 * The Eobj generic object system. It was designed to be the base object
 * system for the EFL.
 *
 * @{
 */

/**
 * @def EOBJ_TYPECHECK(type, x)
 *
 * Checks x is castable to type "type" and casts it to it.
 * @param type The C type to check against.
 * @param x the variable to test and cast.
 */
#define EOBJ_TYPECHECK(type, x) \
   ({ \
    type __x; \
    __x = x; \
    (void) __x; \
    (type) x; \
    })

/**
 * @typedef Eobj
 * The basic Object type.
 */
typedef struct _Eobj Eobj;
/**
 * @typedef Eobj_Op
 * The Eobj operation type id.
 */
typedef unsigned int Eobj_Op;

/**
 * @def EOBJ_NOOP
 * A special #Eobj_Op meaning "No operation".
 */
#define EOBJ_NOOP ((Eobj_Op) 0)

/**
 * @typedef eobj_op_func_type
 * The type of the Op functions. This is the type of the functions used by
 * Eobj.
 *
 * @see eobj_op_func_type_const
 */
typedef void (*eobj_op_func_type)(Eobj *, void *class_data, va_list *list);

/**
 * @typedef eobj_op_func_type_const
 * The type of the const Op functions. This is the type of the functions used
 * by Eobj. This is the same as #eobj_op_func_type, except that this should
 * be used with functions that don't modify the data.
 *
 * @see eobj_op_func_type
 */
typedef void (*eobj_op_func_type_const)(const Eobj *, const void *class_data, va_list *list);

/**
 * @addtogroup Eobj_Events Eobj's Event Handling
 * @{
 */

/**
 * @struct _Eobj_Event_Description
 * This struct holds the description of a specific event.
 */
struct _Eobj_Event_Description
{
   const char *name; /**< name of the event. */
   const char *type; /**< describes the data passed in event_info */
   const char *doc; /**< Explanation about the event. */
};

/**
 * @typedef Eobj_Event_Description
 * A convenience typedef for #_Eobj_Event_Description
 */
typedef struct _Eobj_Event_Description Eobj_Event_Description;

/**
 * @def EOBJ_EVENT_DESCRIPTION(name, type, doc)
 * An helper macro to help populating #Eobj_Event_Description
 * @param name The name of the event.
 * @param type The type string of the event.
 * @param doc Additional doc for the event.
 * @see Eobj_Event_Description
 */
#define EOBJ_EVENT_DESCRIPTION(name, type, doc) { name, type, doc }

/**
 * @}
 */

/**
 * @addtogroup Eobj_Class Eobj Class
 * @{
 */

/**
 * @typedef Eobj_Class
 * The basic Object class type.
 */
typedef struct _Eobj_Class Eobj_Class;

/**
 * @def EOBJ_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...)
 * A convenience macro to be used for creating the class_get function. This
 * macro is fairly simple but should still be used as it'll let us improve
 * things easily.
 * @param class_get_func_name the name of the wanted class_get function name.
 * @param class_desc the class description.
 * @param parent_class The parent class for the function. Look at eobj_class_new() for more information.
 * @param ... List of etxensions. Look at eobj_class_new() for more information.
 *
 * You must use this macro if you want thread safety in class creation.
 */
#define EOBJ_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...) \
EAPI const Eobj_Class * \
class_get_func_name(void) \
{ \
   static volatile char lk_init = 0; \
   static Eina_Lock _my_lock; \
   static const Eobj_Class * volatile _my_class = NULL; \
   if (EINA_LIKELY(!!_my_class)) return _my_class; \
   \
   eina_lock_take(&_eobj_class_creation_lock); \
   if (!lk_init) \
      eina_lock_new(&_my_lock); \
   if (lk_init < 2) eina_lock_take(&_my_lock); \
   if (!lk_init) \
      lk_init = 1; \
   else \
     { \
        if (lk_init < 2) eina_lock_release(&_my_lock); \
        eina_lock_release(&_eobj_class_creation_lock); \
        return _my_class; \
     } \
   eina_lock_release(&_eobj_class_creation_lock); \
   _my_class = eobj_class_new(class_desc, parent_class, __VA_ARGS__); \
   eina_lock_release(&_my_lock); \
   \
   eina_lock_take(&_eobj_class_creation_lock); \
   eina_lock_free(&_my_lock); \
   lk_init = 2; \
   eina_lock_release(&_eobj_class_creation_lock); \
   return _my_class; \
}


/**
 * An enum representing the possible types of an Eobj class.
 */
enum _Eobj_Class_Type
{
   EOBJ_CLASS_TYPE_REGULAR = 0, /**< Regular class. */
   EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT, /**< Regular non instant-able class. */
   EOBJ_CLASS_TYPE_INTERFACE, /**< Interface */
   EOBJ_CLASS_TYPE_MIXIN /**< Mixin */
};

/**
 * @typedef Eobj_Class_Type
 * A convenience typedef for #_Eobj_Class_Type.
 */
typedef enum _Eobj_Class_Type Eobj_Class_Type;

/**
 * @struct _Eobj_Op_Func_Description
 * Used to associate an Op with a func.
 * @see eobj_class_funcs_set
 */
struct _Eobj_Op_Func_Description
{
   Eobj_Op op; /**< The op */
   eobj_op_func_type func; /**< The function to call for the op. */
   Eina_Bool constant; /**< #EINA_TRUE if this function is a const. */
};

/**
 * @typedef Eobj_Op_Func_Description
 * A convenience typedef for #_Eobj_Op_Func_Description
 */
typedef struct _Eobj_Op_Func_Description Eobj_Op_Func_Description;

/**
 * @def EOBJ_OP_FUNC(op, func)
 * A convenience macro to be used when populating the #Eobj_Op_Func_Description
 * array.
 *
 * @see EOBJ_OP_FUNC_CONST
 */
#define EOBJ_OP_FUNC(op, func) { op, EOBJ_TYPECHECK(eobj_op_func_type, func), EINA_FALSE }

/**
 * @def EOBJ_OP_FUNC_CONST(op, func)
 * A convenience macro to be used when populating the #Eobj_Op_Func_Description
 * array.
 * The same as #EOBJ_OP_FUNC but for const functions.
 *
 * @see EOBJ_OP_FUNC
 */
#define EOBJ_OP_FUNC_CONST(op, func) { op, (eobj_op_func_type) EOBJ_TYPECHECK(eobj_op_func_type_const, func), EINA_TRUE }

/**
 * @def EOBJ_OP_FUNC_SENTINEL
 * A convenience macro to be used when populating the #Eobj_Op_Func_Description
 * array. It must appear at the end of the ARRAY.
 */
#define EOBJ_OP_FUNC_SENTINEL { 0, NULL, EINA_FALSE }

/**
 * @struct _Eobj_Op_Description
 * This struct holds the description of a specific op.
 */
struct _Eobj_Op_Description
{
   Eobj_Op sub_op; /**< The sub_id of the op in it's class. */
   const char *name; /**< The name of the op. */
   const char *type; /**< descripbes the Op's function signature. */
   const char *doc; /**< Explanation about the Op. */
   Eina_Bool constant; /**< #EINA_TRUE if this op's implementation should not change the obj. */
};

/**
 * @typedef Eobj_Op_Description
 * A convenience typedef for #_Eobj_Op_Description
 */
typedef struct _Eobj_Op_Description Eobj_Op_Description;

/**
 * @struct _Eobj_Class_Description
 * This struct holds the description of a class.
 * This description should be passed to eobj_class_new.
 * Please use the #EOBJ_CLASS_DESCRIPTION_OPS macro when populating it.
 */
struct _Eobj_Class_Description
{
   const char *name; /**< The name of the class. */
   Eobj_Class_Type type; /**< The type of the class. */
   struct {
        Eobj_Op *base_op_id;
        const Eobj_Op_Description *descs;
        size_t count;
   } ops; /**< The ops description, should be filled using #EOBJ_CLASS_DESCRIPTION_OPS */
   const Eobj_Event_Description **events; /**< The event descriptions for this class. */
   size_t data_size; /**< The size of data (private + protected + public) this class needs per object. */
   void (*constructor)(Eobj *obj, void *class_data); /**< The constructor of the object. */
   void (*destructor)(Eobj *obj, void *class_data); /**< The destructor of the object. */
   void (*class_constructor)(Eobj_Class *klass); /**< The constructor of the class. */
   void (*class_destructor)(Eobj_Class *klass); /**< The destructor of the class. */
};

/**
 * @typedef Eobj_Class_Description
 * A convenience typedef for #_Eobj_Class_Description
 */
typedef struct _Eobj_Class_Description Eobj_Class_Description;

/**
 * @def EOBJ_CLASS_DESCRIPTION_OPS(base_op_id, op_descs, count)
 * An helper macro to help populating #Eobj_Class_Description.
 * @param base_op_id A pointer to the base op id of the class.
 * @param op_descs the op descriptions array.
 * @param count the number of ops in the op descriptions array.
 */
#define EOBJ_CLASS_DESCRIPTION_OPS(base_op_id, op_descs, count) { base_op_id, op_descs, count }

/**
 * @def EOBJ_OP_DESCRIPTION(op, type, doc)
 * An helper macro to help populating #Eobj_Op_Description
 * @param sub_id The sub id of the op being described.
 * @param type The type string for the op.
 * @param doc Additional doc for the op.
 * @see Eobj_Op_Description
 * @see EOBJ_OP_DESCRIPTION_CONST
 * @see EOBJ_OP_DESCRIPTION_SENTINEL
 */
#define EOBJ_OP_DESCRIPTION(sub_id, type, doc) { sub_id, #sub_id, type, doc, EINA_FALSE }

/**
 * @def EOBJ_OP_DESCRIPTION_CONST(op, type, doc)
 * An helper macro to help populating #Eobj_Op_Description
 * This macro is the same as EOBJ_OP_DESCRIPTION but indicates that the op's
 * implementation should not change the object.
 * @param sub_id The sub id of the op being described.
 * @param type The type string for the op.
 * @param doc Additional doc for the op.
 * @see Eobj_Op_Description
 * @see EOBJ_OP_DESCRIPTION
 * @see EOBJ_OP_DESCRIPTION_SENTINEL
 */
#define EOBJ_OP_DESCRIPTION_CONST(sub_id, type, doc) { sub_id, #sub_id, type, doc, EINA_TRUE }

/**
 * @def EOBJ_OP_DESCRIPTION_SENTINEL
 * An helper macro to help populating #Eobj_Op_Description
 * Should be placed at the end of the array.
 * @see Eobj_Op_Description
 * @see EOBJ_OP_DESCRIPTION
 */
#define EOBJ_OP_DESCRIPTION_SENTINEL { 0, NULL, NULL, NULL, EINA_FALSE }

/**
 * @brief Create a new class.
 * @param desc the class description to create the class with.
 * @param parent the class to inherit from.
 * @param ... A NULL terminated list of extensions (interfaces, mixins and the classes of any composite objects).
 * @return The new class's handle on success, or NULL otherwise.
 *
 * You should use #EOBJ_DEFINE_CLASS. It'll provide thread safety and other
 * features easily.
 *
 * @see #EOBJ_DEFINE_CLASS
 */
EAPI const Eobj_Class *eobj_class_new(const Eobj_Class_Description *desc, const Eobj_Class *parent, ...);

/**
 * @brief Sets the OP functions for a class.
 * @param klass the class to set the functions to.
 * @param func_descs a NULL terminated array of #Eobj_Op_Func_Description
 *
 * Should be called from within the class constructor.
 */
EAPI void eobj_class_funcs_set(Eobj_Class *klass, const Eobj_Op_Func_Description *func_descs);

/**
 * @brief Gets the name of the passed class.
 * @param klass the class to work on.
 * @return The class's name.
 *
 * @see eobj_class_get()
 */
EAPI const char *eobj_class_name_get(const Eobj_Class *klass);

/**
 * @}
 */

/**
 * @brief Init the eobj subsystem
 * @return #EINA_TRUE on success.
 *
 * @see eobj_shutfown()
 */
EAPI Eina_Bool eobj_init(void);

/**
 * @brief Shutdown the eobj subsystem
 * @return #EINA_TRUE on success.
 *
 * @see eobj_init()
 */
EAPI Eina_Bool eobj_shutdown(void);

/**
 * @def eobj_do
 * A convenience wrapper around eobj_do_internal()
 * @see eobj_do_internal
 */
#define eobj_do(obj, ...) eobj_do_internal(obj, EINA_FALSE, __VA_ARGS__, EOBJ_NOOP)

/**
 * @def eobj_query
 * Same as #eobj_do but only for const ops.
 * @see eobj_do
 */
#define eobj_query(obj, ...) eobj_do_internal((Eobj *) EOBJ_TYPECHECK(const Eobj *, obj), EINA_TRUE, __VA_ARGS__, EOBJ_NOOP)

/**
 * @brief Issues ops on an object.
 * @param obj The object to work on
 * @param constant #EINA_TRUE if this call is on a constant object.
 * @param ... NULL terminated list of OPs and parameters.
 * @return #EINA_TRUE on success.
 *
 * Use the helper macros, don't pass the parameters manually.
 * Use #eobj_do instead of this function.
 *
 * @see #eobj_do
 */
EAPI Eina_Bool eobj_do_internal(Eobj *obj, Eina_Bool constant, ...);

/**
 * @brief Calls the super function for the specific op.
 * @param obj The object to work on
 * @param ... list of parameters.
 * @return #EINA_TRUE on success.
 *
 * Unlike eobj_do() and eobj_query(), this function only accepts one op.
 *
 * Use the helper macros, don't pass the parameters manually.
 *
 * Same as eobj_do_super() just for const objects.
 *
 * @see #eobj_query
 * @see eobj_do_super()
 */
#define eobj_query_super(obj, ...) eobj_do_super_internal((Eobj *) EOBJ_TYPECHECK(const Eobj *, obj), EINA_TRUE, __VA_ARGS__)

/**
 * @brief Calls the super function for the specific op.
 * @param obj The object to work on
 * @param ... list of parameters.
 * @return #EINA_TRUE on success.
 *
 * Unlike eobj_do() and eobj_query(), this function only accepts one op.
 *
 * @see #eobj_query
 * @see eobj_query_super()
 */
#define eobj_do_super(obj, ...) eobj_do_super_internal((Eobj *) EOBJ_TYPECHECK(const Eobj *, obj), EINA_FALSE, __VA_ARGS__)

/**
 * @brief Calls the super function for the specific op.
 * @param obj The object to work on
 * @param constant #EINA_TRUE if this call is on a constant object.
 * @param op The wanted op.
 * @param ... list of parameters.
 * @return #EINA_TRUE on success.
 *
 * Don't use this function, use the wrapping macros instead.
 *
 * @see #eobj_do
 * @see #eobj_do_super
 * @see #eobj_query_super
 */
EAPI Eina_Bool eobj_do_super_internal(Eobj *obj, Eina_Bool constant, Eobj_Op op, ...);

/**
 * @brief Gets the class of the object.
 * @param obj The object to work on
 * @return The object's class.
 *
 * @see eobj_class_name_get()
 */
EAPI const Eobj_Class *eobj_class_get(const Eobj *obj);

/**
 * @brief Calls the super constructor of the object passed.
 * @param obj the object to work on.
 *
 * @see eobj_destructor_super()
 */
EAPI void eobj_constructor_super(Eobj *obj);

/**
 * @brief Calls the super destructor of the object passed.
 * @param obj the object to work on.
 *
 * @see eobj_constructor_super()
 */
EAPI void eobj_destructor_super(Eobj *obj);

/**
 * @brief Notify eobj that there was an error when constructing the object.
 * @param obj the object to work on.
 *
 * (Should only be called from within a constructor/destructor).
 *
 * @see eobj_constructor_error_get()
 */
EAPI void eobj_constructor_error_set(Eobj *obj);

/**
 * @brief Check if there was an error constructing obj
 * @param obj the object to work on.
 * @return #EINA_TRUE if there was an error.
 *
 * (Should only be called from within a constructor/destructor).
 *
 * @see eobj_constructor_error_set()
 */
EAPI Eina_Bool eobj_constructor_error_get(const Eobj *obj);

/**
 * @brief Create a new object.
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @return An handle to the new object on success, NULL otherwise.
 */
EAPI Eobj *eobj_add(const Eobj_Class *klass, Eobj *parent);

/**
 * @brief Get the parent of an object
 * @param obj the object to get the parent of.
 * @return a pointer to the parent object.
 */
EAPI Eobj *eobj_parent_get(Eobj *obj);

/**
 * @brief Get a pointer to the data of an object for a specific class.
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @return a pointer to the data.
 */
EAPI void *eobj_data_get(const Eobj *obj, const Eobj_Class *klass);

/**
 * @brief Increment the object's reference count by 1.
 * @param obj the object to work on.
 * @return The object passed.
 *
 * It's very easy to get a refcount leak and start leaking memory because
 * of a forgotten unref or an extra ref. That is why there are eobj_xref
 * and eobj_xunref that will make debugging easier in such a case.
 * Therefor, these functions should only be used in small scopes, i.e at the
 * start of some section in which the object may get freed, or if you know
 * what you are doing.
 *
 * @see eobj_unref()
 * @see eobj_ref_get()
 */
EAPI Eobj *eobj_ref(Eobj *obj);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed.
 * @param obj the object to work on.
 *
 * @see eobj_ref()
 * @see eobj_ref_get()
 */
EAPI void eobj_unref(Eobj *obj);

/**
 * @brief Return the ref count of the object passed.
 * @param obj the object to work on.
 * @return the ref count of the object.
 *
 * @see eobj_ref()
 * @see eobj_unref()
 */
EAPI int eobj_ref_get(const Eobj *obj);

/**
 * @def eobj_xref(obj, ref_obj)
 * Convenience macro around eobj_xref()
 * @see eobj_xref()
 */
#define eobj_xref(obj, ref_obj) eobj_xref_internal(obj, ref_obj, __FILE__, __LINE__)

/**
 * @brief Increment the object's reference count by 1 (and associate the ref with ref_obj)
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 * @param file the call's filename.
 * @param line the call's line number.
 * @return The object passed (obj)
 *
 * People should not use this function, use #eobj_xref instead.
 *
 * @see eobj_xunref()
 */
EAPI Eobj *eobj_xref_internal(Eobj *obj, const Eobj *ref_obj, const char *file, int line);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed. Will free the ref associated with ref_obj).
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 *
 * This function only enforces the checks for object association. I.e don't rely
 * on it. If such enforces are compiled out, this function behaves the same as
 * eobj_unref().
 *
 * @see eobj_xref_internal()
 */
EAPI void eobj_xunref(Eobj *obj, const Eobj *ref_obj);

/**
 * @brief Delete the object passed (disregarding ref count).
 * @param obj the object to work on.
 *
 * @see eobj_unref()
 */
EAPI void eobj_del(Eobj *obj);

/**
 * @addtogroup Eobj_Weak_Ref Weak reference for Eobj objects.
 * @{
 */

/**
 * @brief Add a new weak reference to obj.
 * @param wref The pointer to use for the weak ref.
 * @param obj The object being referenced.
 * @return The object being referenced (obj).
 *
 * This function registers the object handle pointed by wref to obj so when
 * obj is deleted it'll be updated to NULL. This functions should be used
 * when you want to keep track of an object in a safe way, but you don't want
 * to prevent it from being freed.
 *
 * @see eobj_weak_ref_del()
 */
EAPI Eobj *eobj_weak_ref_add(Eobj **wref, const Eobj *obj);

/**
 * @brief Delete the weak reference passed.
 * @param wref the weak reference to free.
 *
 * @see eobj_weak_ref_add()
 */
EAPI void eobj_weak_ref_del(Eobj **wref);

/**
 * @}
 */


/**
 * @addtogroup Eobj_Composite_Objects Composite Objects.
 * @{
 */

/**
 * @brief Make an object a composite object of another.
 * @param obj the "parent" object.
 * @param comp_obj the object that will be used to composite obj.
 *
 * @see eobj_composite_object_detach()
 * @see eobj_composite_is()
 */
EAPI void eobj_composite_object_attach(Eobj *obj, Eobj *comp_obj);

/**
 * @brief Detach a composite object from another object.
 * @param obj the "parent" object.
 * @param comp_obj the object attached to obj.
 *
 * @see eobj_composite_object_attach()
 * @see eobj_composite_is()
 */
EAPI void eobj_composite_object_detach(Eobj *obj, Eobj *comp_obj);

/**
 * @brief Check if an object is a composite object.
 * @param comp_obj the object to be checked.
 * @return #EINA_TRUE if it is, #EINA_FALSE otherwise.
 *
 * @see eobj_composite_object_attach()
 * @see eobj_composite_object_detach()
 */
EAPI Eina_Bool eobj_composite_is(Eobj *comp_obj);

/**
 * @}
 */

/**
 * @addtogroup Eobj_Events Eobj's Event Handling
 * @{
 */

/**
 * @def EOBJ_CALLBACK_PRIORITY_BEFORE
 * Slightly more prioritized than default.
 */
#define EOBJ_CALLBACK_PRIORITY_BEFORE -100
/**
 * @def EOBJ_CALLBACK_PRIORITY_DEFAULT
 * Default callback priority level
 */
#define EOBJ_CALLBACK_PRIORITY_DEFAULT 0
/**
 * @def EOBJ_CALLBACK_PRIORITY_AFTER
 * Slightly less prioritized than default.
 */
#define EOBJ_CALLBACK_PRIORITY_AFTER 100

/**
 * @typedef Eobj_Callback_Priority
 *
 * Callback priority value. Range is -32k - 32k. The lower the number, the
 * higher the priority.
 *
 * @see EOBJ_CALLBACK_PRIORITY_AFTER
 * @see EOBJ_CALLBACK_PRIORITY_BEFORE
 * @see EOBJ_CALLBACK_PRIORITY_DEFAULT
 */
typedef short Eobj_Callback_Priority;

/**
 * @def EOBJ_CALLBACK_STOP
 * Stop calling callbacks for the even of which the callback was called for.
 * @see EOBJ_CALLBACK_CONTINUE
 */
#define EOBJ_CALLBACK_STOP EINA_FALSE

/**
 * @def EOBJ_CALLBACK_CONTINUE
 * Continue calling callbacks for the even of which the callback was called for.
 * @see EOBJ_CALLBACK_STOP
 */
#define EOBJ_CALLBACK_CONTINUE EINA_TRUE

/**
 * @typedef Eobj_Event_Cb
 *
 * An event callback prototype.
 *
 * @param data The user data registered with the callback.
 * @param obj The object which initiated the event.
 * @param desc The event's description.
 * @param event_info additional data passed with the event.
 * @return #EOBJ_CALLBACK_STOP to stop calling additional callbacks for the event, #EOBJ_CALLBACK_CONTINUE to continue.
 */
typedef Eina_Bool (*Eobj_Event_Cb)(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info);

/**
 * @brief Add an event callback forwarder for an event and an object.
 * @param obj The object to listen to events on.
 * @param desc The description of the event to listen to.
 * @param new_obj The object to emit events from.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eobj_event_callback_forwarder_del()
 */
EAPI Eina_Bool eobj_event_callback_forwarder_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj);

/**
 * @brief Remove an event callback forwarder for an event and an object.
 * @param obj The object to listen to events on.
 * @param desc The description of the event to listen to.
 * @param new_obj The object to emit events from.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @see eobj_event_callback_forwarder_add()
 */
EAPI Eina_Bool eobj_event_callback_forwarder_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj);

/**
 * @def eobj_event_callback_add(obj, desc, cb, data)
 * Add a callback for an event.
 * @param obj The object to listen to events on.
 * @param desc The description of the event to listen to.
 * @param cb the callback to call.
 * @param data additional data to pass to the callback.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see eobj_event_callback_priority_add()
 */
#define eobj_event_callback_add(obj, desc, cb, data) \
   eobj_event_callback_priority_add(obj, desc, \
         EOBJ_CALLBACK_PRIORITY_DEFAULT, cb, data)

/**
 * @brief Add a callback for an event with a specific priority.
 * @param obj The object to listen to events on.
 * @param desc The description of the event to listen to.
 * @param priority The priority of the callback.
 * @param cb the callback to call.
 * @param data additional data to pass to the callback.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see #eobj_event_callback_add
 */
EAPI Eina_Bool eobj_event_callback_priority_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Callback_Priority priority, Eobj_Event_Cb cb, const void *data);

/**
 * @brief Del a callback for an event
 * @param obj The object to listen to delete from.
 * @param desc The description of the event to listen to.
 * @param func the callback to delete.
 * @return The additional data that was set to be passed to the callback.
 *
 * @see eobj_event_callback_del()
 */
EAPI void *eobj_event_callback_del_lazy(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func);

/**
 * @brief Del a callback with a specific data associated to it for an event.
 * @param obj The object to listen to delete from.
 * @param desc The description of the event to listen to.
 * @param func the callback to delete.
 * @param user_data The data to compare.
 * @return The additional data that was set to be passed to the callback.
 *
 * @see eobj_event_callback_del_lazy()
 */
EAPI void *eobj_event_callback_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func, const void *user_data);

/**
 * @brief Call the callbacks for an event of an object.
 * @param obj The object to work on.
 * @param desc The description of the event to call.
 * @param event_info Extra event info to pass to the callbacks.
 * @return #EINA_FALSE if one of the callbacks aborted the callback calls or #EINA_TRUE otherwise.
 */
EAPI Eina_Bool eobj_event_callback_call(Eobj *obj, const Eobj_Event_Description *desc, const void *event_info);

/**
 * @}
 */

/**
 * @addtogroup Eobj_Class_Base Eobj's Base class.
 * @{
 */

/**
 * @def EOBJ_BASE_CLASS
 * The class type for the Eobj base class.
 */
#define EOBJ_BASE_CLASS eobj_base_class_get()
/**
 * @brief Use #EOBJ_BASE_CLASS
 * @internal
 * */
EAPI const Eobj_Class *eobj_base_class_get(void) EINA_CONST;

/**
 * @typedef eobj_base_data_free_func
 * Data free func prototype.
 */
typedef void (*eobj_base_data_free_func)(void *);

/**
 * @var EOBJ_BASE_BASE_ID
 * #EOBJ_BASE_CLASS 's base id.
 */
extern EAPI Eobj_Op EOBJ_BASE_BASE_ID;

enum {
     EOBJ_BASE_SUB_ID_DATA_SET,
     EOBJ_BASE_SUB_ID_DATA_GET,
     EOBJ_BASE_SUB_ID_DATA_DEL,
     EOBJ_BASE_SUB_ID_LAST
};

/**
 * @def EOBJ_BASE_ID(sub_id)
 * Helper macro to get the full Op ID out of the sub_id for EOBJ_BASE.
 * @param sub_id the sub id inside EOBJ_BASE.
 */
#define EOBJ_BASE_ID(sub_id) (EOBJ_BASE_BASE_ID + sub_id)

/**
 * @def eobj_base_data_set(key, data, free_func)
 * Set generic data to object.
 * @param[in] key the key associated with the data
 * @param[in] data the data to set.
 * @param[in] free_func the func to free data with (NULL means "do nothing").
 *
 * @see #eobj_base_data_get
 * @see #eobj_base_data_del
 */
#define eobj_base_data_set(key, data, free_func) EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_SET), EOBJ_TYPECHECK(const char *, key), EOBJ_TYPECHECK(const void *, data), EOBJ_TYPECHECK(eobj_base_data_free_func, free_func)

/**
 * @def eobj_base_data_get(key, data)
 * Get generic data from object.
 * @param[in] key the key associated with the data
 * @param[out] data the data for the key
 *
 * @see #eobj_base_data_set
 * @see #eobj_base_data_del
 */
#define eobj_base_data_get(key, data) EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_GET), EOBJ_TYPECHECK(const char *, key), EOBJ_TYPECHECK(void **, data)

/**
 * @def eobj_base_data_del(key)
 * Del generic data from object.
 * @param[in] key the key associated with the data
 *
 * @see #eobj_base_data_set
 * @see #eobj_base_data_get
 */
#define eobj_base_data_del(key) EOBJ_BASE_ID(EOBJ_BASE_SUB_ID_DATA_DEL), EOBJ_TYPECHECK(const char *, key)

/**
 * @var _EOBJ_EV_CALLBACK_ADD
 * see EOBJ_EV_CALLBACK_ADD
 */
EAPI extern const Eobj_Event_Description _EOBJ_EV_CALLBACK_ADD;

/**
 * @def EOBJ_EV_CALLBACK_ADD
 * The event description (of type #Eobj_Event_Description) for
 * The "Callback listener added" event.
 */
#define EOBJ_EV_CALLBACK_ADD (&(_EOBJ_EV_CALLBACK_ADD))

/**
 * @var _EOBJ_EV_CALLBACK_DEL
 * see EOBJ_EV_CALLBACK_DEL
 */
EAPI extern const Eobj_Event_Description _EOBJ_EV_CALLBACK_DEL;

/**
 * @def EOBJ_EV_CALLBACK_DEL
 * The event description (of type #Eobj_Event_Description) for
 * The "Callback listener deleted" event.
 */
#define EOBJ_EV_CALLBACK_DEL (&(_EOBJ_EV_CALLBACK_DEL))

/**
 * @var _EOBJ_EV_FREE
 * see #EOBJ_EV_FREE
 */
EAPI extern const Eobj_Event_Description _EOBJ_EV_FREE;

/**
 * @def EOBJ_EV_FREE
 * Object is being freed.
 */
#define EOBJ_EV_FREE (&(_EOBJ_EV_FREE))

/**
 * @var _EOBJ_EV_DEL
 * see #EOBJ_EV_DEL
 */
EAPI extern const Eobj_Event_Description _EOBJ_EV_DEL;

/**
 * @def EOBJ_EV_DEL
 * Object is being deleted.
 */
#define EOBJ_EV_DEL (&(_EOBJ_EV_DEL))

/**
 * @}
 */

/**
 * @}
 */

#endif
