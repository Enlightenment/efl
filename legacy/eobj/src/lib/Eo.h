#ifndef EO_H
#define EO_H

#include <stdarg.h>
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

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
 * @var _eo_class_creation_lock
 * This variable is used for locking purposes in the class_get function
 * defined in #EO_DEFINE_CLASS.
 * This is just to work around the fact that we need to init locks before
 * using them.
 * Don't touch it if you don't know what you are doing.
 * @internal
 */
EAPI extern Eina_Lock _eo_class_creation_lock;

/**
 * @internal
 * An enum representing the possible types of an Op.
 */
enum _Eo_Op_Type
{
   EO_OP_TYPE_INVALID = -1, /**< Invalid op. */
   EO_OP_TYPE_REGULAR = 0, /**< Regular op. */
   EO_OP_TYPE_CLASS, /**< Class op - a class op. Like static in Java/C++. */
};

/**
 * @internal
 * @typedef Eo_Op_Type
 * A convenience typedef for #_Eo_Op_Type.
 */
typedef enum _Eo_Op_Type Eo_Op_Type;

/**
 * @defgroup Eo Eo Generic Object System
 *
 * The Eo generic object system. It was designed to be the base object
 * system for the EFL.
 *
 * @{
 */

/**
 * @def EO_TYPECHECK(type, x)
 *
 * Checks x is castable to type "type" and casts it to it.
 * @param type The C type to check against.
 * @param x the variable to test and cast.
 */
#define EO_TYPECHECK(type, x) \
   ({ \
    type __x; \
    __x = x; \
    (type) __x; \
    })

/**
 * @typedef Eo
 * The basic Object type.
 */
typedef struct _Eo Eo;
/**
 * @typedef Eo_Op
 * The Eo operation type id.
 */
typedef unsigned int Eo_Op;

/**
 * @typedef Eo_Class
 * The basic Object class type.
 * @ingroup Eo_Class
 */
typedef struct _Eo_Class Eo_Class;

/**
 * @typedef Eo_Class_Id
 * An Id of a class.
 * @ingroup Eo_Class
 */
typedef size_t Eo_Class_Id;

/**
 * @def EO_NOOP
 * A special #Eo_Op meaning "No operation".
 */
#define EO_NOOP ((Eo_Op) 0)

/**
 * @typedef eo_op_func_type
 * The type of the Op functions. This is the type of the functions used by
 * Eo.
 *
 * @see eo_op_func_type_class
 */
typedef void (*eo_op_func_type)(Eo *, void *class_data, va_list *list);

/**
 * @typedef eo_op_func_type_class
 * The type of the class Op functions. This is the same as #eo_op_func_type,\
 * exepct that it's for usage with class functions, and not with object
 * functions.
 *
 * @see eo_op_func_type
 */
typedef void (*eo_op_func_type_class)(const Eo_Class *, va_list *list);

/**
 * @addtogroup Eo_Events Eo's Event Handling
 * @{
 */

/**
 * @struct _Eo_Event_Description
 * This struct holds the description of a specific event.
 */
struct _Eo_Event_Description
{
   const char *name; /**< name of the event. */
   const char *doc; /**< Explanation about the event. */
};

/**
 * @typedef Eo_Event_Description
 * A convenience typedef for #_Eo_Event_Description
 */
typedef struct _Eo_Event_Description Eo_Event_Description;

/**
 * @def EO_EVENT_DESCRIPTION(name, doc)
 * An helper macro to help populating #Eo_Event_Description
 * @param name The name of the event.
 * @param doc Additional doc for the event.
 * @see Eo_Event_Description
 */
#define EO_EVENT_DESCRIPTION(name, doc) { name, doc }

/**
 * @}
 */

/**
 * @addtogroup Eo_Class Eo Class
 * @{
 */

/**
 * @def EO_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...)
 * A convenience macro to be used for creating the class_get function. This
 * macro is fairly simple but should still be used as it'll let us improve
 * things easily.
 * @param class_get_func_name the name of the wanted class_get function name.
 * @param class_desc the class description.
 * @param parent_class The parent class for the function. Look at eo_class_new() for more information.
 * @param ... List of etxensions. Look at eo_class_new() for more information.
 *
 * You must use this macro if you want thread safety in class creation.
 */
#define EO_DEFINE_CLASS(class_get_func_name, class_desc, parent_class, ...) \
EAPI const Eo_Class * \
class_get_func_name(void) \
{ \
   static volatile char lk_init = 0; \
   static Eina_Lock _my_lock; \
   static const Eo_Class * volatile _my_class = NULL; \
   if (EINA_LIKELY(!!_my_class)) return _my_class; \
   \
   eina_lock_take(&_eo_class_creation_lock); \
   if (!lk_init) \
      eina_lock_new(&_my_lock); \
   if (lk_init < 2) eina_lock_take(&_my_lock); \
   if (!lk_init) \
      lk_init = 1; \
   else \
     { \
        if (lk_init < 2) eina_lock_release(&_my_lock); \
        eina_lock_release(&_eo_class_creation_lock); \
        return _my_class; \
     } \
   eina_lock_release(&_eo_class_creation_lock); \
   (void) parent_class; \
   _my_class = eo_class_new(class_desc, parent_class, __VA_ARGS__); \
   eina_lock_release(&_my_lock); \
   \
   eina_lock_take(&_eo_class_creation_lock); \
   eina_lock_free(&_my_lock); \
   lk_init = 2; \
   eina_lock_release(&_eo_class_creation_lock); \
   return _my_class; \
}


/**
 * An enum representing the possible types of an Eo class.
 */
enum _Eo_Class_Type
{
   EO_CLASS_TYPE_REGULAR = 0, /**< Regular class. */
   EO_CLASS_TYPE_REGULAR_NO_INSTANT, /**< Regular non instant-able class. */
   EO_CLASS_TYPE_INTERFACE, /**< Interface */
   EO_CLASS_TYPE_MIXIN /**< Mixin */
};

/**
 * @typedef Eo_Class_Type
 * A convenience typedef for #_Eo_Class_Type.
 */
typedef enum _Eo_Class_Type Eo_Class_Type;

/**
 * @struct _Eo_Op_Func_Description
 * Used to associate an Op with a func.
 * @see eo_class_funcs_set
 */
struct _Eo_Op_Func_Description
{
   Eo_Op op; /**< The op */
   eo_op_func_type func; /**< The function to call for the op. */
   Eo_Op_Type op_type; /**< The type of the op */
};

/**
 * @typedef Eo_Op_Func_Description
 * A convenience typedef for #_Eo_Op_Func_Description
 */
typedef struct _Eo_Op_Func_Description Eo_Op_Func_Description;

/**
 * @def EO_OP_FUNC(op, func)
 * A convenience macro to be used when populating the #Eo_Op_Func_Description
 * array.
 */
#define EO_OP_FUNC(op, func) { op, EO_TYPECHECK(eo_op_func_type, func), EO_OP_TYPE_REGULAR }

/**
 * @def EO_OP_FUNC_CLASS(op, func)
 * A convenience macro to be used when populating the #Eo_Op_Func_Description
 * array.
 * The same as #EO_OP_FUNC but for class functions.
 *
 * @see EO_OP_FUNC
 */
#define EO_OP_FUNC_CLASS(op, func) { op, (eo_op_func_type) EO_TYPECHECK(eo_op_func_type_class, func), EO_OP_TYPE_CLASS }

/**
 * @def EO_OP_FUNC_SENTINEL
 * A convenience macro to be used when populating the #Eo_Op_Func_Description
 * array. It must appear at the end of the ARRAY.
 */
#define EO_OP_FUNC_SENTINEL { 0, NULL, EO_OP_TYPE_INVALID }

/**
 * @struct _Eo_Op_Description
 * This struct holds the description of a specific op.
 */
struct _Eo_Op_Description
{
   Eo_Op sub_op; /**< The sub_id of the op in it's class. */
   const char *name; /**< The name of the op. */
   const char *doc; /**< Explanation about the Op. */
   Eo_Op_Type op_type; /**< The type of the Op. */
};

/**
 * @typedef Eo_Op_Description
 * A convenience typedef for #_Eo_Op_Description
 */
typedef struct _Eo_Op_Description Eo_Op_Description;

/**
 * @def EO_VERSION
 * The current version of EO.
 */
#define EO_VERSION 1

/**
 * @struct _Eo_Class_Description
 * This struct holds the description of a class.
 * This description should be passed to eo_class_new.
 * Please use the #EO_CLASS_DESCRIPTION_OPS macro when populating it.
 */
struct _Eo_Class_Description
{
   unsigned int version; /**< The current version of eo, use #EO_VERSION */
   const char *name; /**< The name of the class. */
   Eo_Class_Type type; /**< The type of the class. */
   struct {
        Eo_Op *base_op_id;
        const Eo_Op_Description *descs;
        size_t count;
   } ops; /**< The ops description, should be filled using #EO_CLASS_DESCRIPTION_OPS */
   const Eo_Event_Description **events; /**< The event descriptions for this class. */
   size_t data_size; /**< The size of data (private + protected + public) this class needs per object. */
   void (*class_constructor)(Eo_Class *klass); /**< The constructor of the class. */
   void (*class_destructor)(Eo_Class *klass); /**< The destructor of the class. */
};

/**
 * @typedef Eo_Class_Description
 * A convenience typedef for #_Eo_Class_Description
 */
typedef struct _Eo_Class_Description Eo_Class_Description;

/**
 * @def EO_CLASS_DESCRIPTION_OPS(base_op_id, op_descs, count)
 * An helper macro to help populating #Eo_Class_Description.
 * @param base_op_id A pointer to the base op id of the class.
 * @param op_descs the op descriptions array.
 * @param count the number of ops in the op descriptions array.
 */
#define EO_CLASS_DESCRIPTION_OPS(base_op_id, op_descs, count) { base_op_id, op_descs, count }

/**
 * @def EO_OP_DESCRIPTION(op, doc)
 * An helper macro to help populating #Eo_Op_Description
 * @param sub_id The sub id of the op being described.
 * @param doc Additional doc for the op.
 * @see Eo_Op_Description
 * @see EO_OP_DESCRIPTION_CLASS
 * @see EO_OP_DESCRIPTION_SENTINEL
 */
#define EO_OP_DESCRIPTION(sub_id, doc) { sub_id, #sub_id, doc, EO_OP_TYPE_REGULAR }

/**
 * @def EO_OP_DESCRIPTION_CLASS(op, doc)
 * An helper macro to help populating #Eo_Op_Description
 * This macro is the same as EO_OP_DESCRIPTION but indicates that the op's
 * implementation is of type CLASS.
 * @param sub_id The sub id of the op being described.
 * @param doc Additional doc for the op.
 * @see Eo_Op_Description
 * @see EO_OP_DESCRIPTION
 * @see EO_OP_DESCRIPTION_SENTINEL
 */
#define EO_OP_DESCRIPTION_CLASS(sub_id, doc) { sub_id, #sub_id, doc, EO_OP_TYPE_CLASS }

/**
 * @def EO_OP_DESCRIPTION_SENTINEL
 * An helper macro to help populating #Eo_Op_Description
 * Should be placed at the end of the array.
 * @see Eo_Op_Description
 * @see EO_OP_DESCRIPTION
 */
#define EO_OP_DESCRIPTION_SENTINEL { 0, NULL, NULL, EO_OP_TYPE_INVALID }

/**
 * @brief Create a new class.
 * @param desc the class description to create the class with.
 * @param parent the class to inherit from.
 * @param ... A NULL terminated list of extensions (interfaces, mixins and the classes of any composite objects).
 * @return The new class's handle on success, or NULL otherwise.
 *
 * You should use #EO_DEFINE_CLASS. It'll provide thread safety and other
 * features easily.
 *
 * @see #EO_DEFINE_CLASS
 */
EAPI const Eo_Class *eo_class_new(const Eo_Class_Description *desc, const Eo_Class *parent, ...);

/**
 * @brief Check if an object "is a" klass.
 * @param obj The object to check
 * @param klass The klass to check against.
 * @return @c EINA_TRUE if obj implements klass, @c EINA_FALSE otherwise.
 *
 * Notice: This function does not support composite objects.
 */
EAPI Eina_Bool eo_isa(const Eo *obj, const Eo_Class *klass);

/**
 * @brief Sets the OP functions for a class.
 * @param klass the class to set the functions to.
 * @param func_descs a NULL terminated array of #Eo_Op_Func_Description
 *
 * Should be called from within the class constructor.
 */
EAPI void eo_class_funcs_set(Eo_Class *klass, const Eo_Op_Func_Description *func_descs);

/**
 * @brief Gets the name of the passed class.
 * @param klass the class to work on.
 * @return The class's name.
 *
 * @see eo_class_get()
 */
EAPI const char *eo_class_name_get(const Eo_Class *klass);

/**
 * @}
 */

/**
 * @brief Init the eo subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see eo_shutfown()
 */
EAPI Eina_Bool eo_init(void);

/**
 * @brief Shutdown the eo subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see eo_init()
 */
EAPI Eina_Bool eo_shutdown(void);

/**
 * @def eo_do
 * A convenience wrapper around eo_do_internal()
 * @see eo_do_internal
 */
#define eo_do(obj, ...) eo_do_internal(obj, EO_OP_TYPE_REGULAR, __VA_ARGS__, EO_NOOP)

/**
 * @def eo_class_do
 * A convenience wrapper around eo_class_do_internal()
 * @see eo_class_do_internal
 */
#define eo_class_do(klass, ...) eo_class_do_internal(klass, __VA_ARGS__, EO_NOOP)

/**
 * @brief Calls op functions of an object
 * @param obj The object to work on
 * @param op_type The type of the ops that are passed.
 * @param ... NULL terminated list of OPs and parameters.
 * @return @c EINA_TRUE on success.
 *
 * Use the helper macros, don't pass the parameters manually.
 * Use #eo_do instead of this function.
 *
 * @see #eo_do
 */
EAPI Eina_Bool eo_do_internal(Eo *obj, Eo_Op_Type op_type, ...);

/**
 * @brief Calls op functions of a class.
 * @param klass The class to work on
 * @param ... NULL terminated list of OPs and parameters.
 * @return @c EINA_TRUE on success.
 *
 * Use the helper macros, don't pass the parameters manually.
 * Use #eo_do instead of this function.
 *
 * @see #eo_class_do
 */
EAPI Eina_Bool eo_class_do_internal(const Eo_Class *klass, ...);

/**
 * @brief Calls the super function for the specific op.
 * @param obj The object to work on
 * @param ... list of parameters.
 * @return @c EINA_TRUE on success.
 *
 * Unlike eo_do(), this function only accepts one op.
 *
 * @see #eo_do
 */
#define eo_do_super(obj, ...) eo_do_super_internal(obj, EO_OP_TYPE_REGULAR, __VA_ARGS__)

/**
 * @brief Calls the super function for the specific op.
 * @param klass The klass to work on
 * @param ... list of parameters.
 * @return @c EINA_TRUE on success.
 *
 * Unlike eo_class_do(), this function only accepts one op.
 *
 * @see #eo_class_do
 */
#define eo_class_do_super(klass, ...) eo_class_do_super_internal(klass, __VA_ARGS__)

/**
 * @brief Calls the super function for the specific op.
 * @param obj The object to work on
 * @param op_type The type of the ops that are passed.
 * @param op The wanted op.
 * @param ... list of parameters.
 * @return @c EINA_TRUE on success.
 *
 * Don't use this function, use the wrapping macros instead.
 *
 * @see #eo_do
 * @see #eo_do_super
 */
EAPI Eina_Bool eo_do_super_internal(Eo *obj, Eo_Op_Type op_type, Eo_Op op, ...);

/**
 * @brief Calls the super function for the specific op.
 * @param klass The klass to work on
 * @param op The wanted op.
 * @param ... list of parameters.
 * @return @c EINA_TRUE on success.
 *
 * Don't use this function, use the wrapping macros instead.
 *
 * @see #eo_class_do
 * @see #eo_class_do_super
 */
EAPI Eina_Bool eo_class_do_super_internal(const Eo_Class *klass, Eo_Op op, ...);

/**
 * @brief Gets the class of the object.
 * @param obj The object to work on
 * @return The object's class.
 *
 * @see eo_class_name_get()
 */
EAPI const Eo_Class *eo_class_get(const Eo *obj);

/**
 * @def eo_error_set
 * @brief Notify eo that there was an error when constructing, destructing or calling a function of the object.
 * @param obj the object to work on.
 *
 * @see eo_error_get()
 */
#define eo_error_set(obj) eo_error_set_internal(obj, __FILE__, __LINE__)

/* @cond 0 */
EAPI void eo_error_set_internal(const Eo *obj, const char *file, int line);
/* @endcond */

/**
 * @def eo_add
 * @brief Create a new object with the default constructor.
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @param ... The ops to run.
 * @return An handle to the new object on success, NULL otherwise.
 */
#define eo_add(klass, parent, ...) \
   ({ \
    (void) klass; \
    eo_add_internal(klass, parent, eo_constructor(), ## __VA_ARGS__, EO_NOOP); \
    })

/**
 * @def eo_add_custom
 * @brief Create a new object with a custom constructor.
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @param ... The ops to run. With the constructor being first.
 * @return An handle to the new object on success, NULL otherwise.
 */
#define eo_add_custom(klass, parent, ...) \
   ({ \
    (void) klass; \
    eo_add_internal(klass, parent, ## __VA_ARGS__, EO_NOOP); \
    })

/**
 * @brief Create a new object.
 * @param klass the class of the object to create.
 * @param parent the parent to set to the object.
 * @param ... The ops to run. With the constructor being first.
 * @return An handle to the new object on success, NULL otherwise.
 *
 * Use the helper macros, don't pass the parameters manually.
 * Use #eo_add or #eo_add_custom instead of this function.
 *
 * @see #eo_add
 */
EAPI Eo *eo_add_internal(const Eo_Class *klass, Eo *parent, ...);

/**
 * @brief Get the parent of an object
 * @param obj the object to get the parent of.
 * @return a pointer to the parent object.
 *
 * @see eo_parent_set()
 */
EAPI Eo *eo_parent_get(const Eo *obj);

/**
 * @brief Set the parent of an object
 * @param obj the object to get the parent of.
 * @param parent the new parent.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 *
 * Parents keep references to their children so in order to delete objects
 * that have parents you need to set parent to NULL or use eo_del() that
 * does that for you (and also unrefs the object).
 *
 * @see eo_del()
 * @see eo_parent_get()
 */
EAPI Eina_Bool eo_parent_set(Eo *obj, const Eo *parent);

/**
 * @brief Get a pointer to the data of an object for a specific class.
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @return a pointer to the data.
 */
EAPI void *eo_data_get(const Eo *obj, const Eo_Class *klass);

/**
 * @brief Increment the object's reference count by 1.
 * @param obj the object to work on.
 * @return The object passed.
 *
 * It's very easy to get a refcount leak and start leaking memory because
 * of a forgotten unref or an extra ref. That is why there are eo_xref
 * and eo_xunref that will make debugging easier in such a case.
 * Therefor, these functions should only be used in small scopes, i.e at the
 * start of some section in which the object may get freed, or if you know
 * what you are doing.
 *
 * @see eo_unref()
 * @see eo_ref_get()
 */
EAPI Eo *eo_ref(const Eo *obj);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed.
 * @param obj the object to work on.
 *
 * @see eo_ref()
 * @see eo_ref_get()
 */
EAPI void eo_unref(const Eo *obj);

/**
 * @brief Return the ref count of the object passed.
 * @param obj the object to work on.
 * @return the ref count of the object.
 *
 * @see eo_ref()
 * @see eo_unref()
 */
EAPI int eo_ref_get(const Eo *obj);

/**
 * @brief Unrefs the object and reparents it to NULL.
 * @param obj the object to work on.
 *
 * Because eo_del() unrefs and reparents to NULL, it doesn't really delete the
 * object.
 *
 * @see eo_unref()
 * @see eo_parent_set()
 */
EAPI void eo_del(const Eo *obj);

/**
 * @def eo_xref(obj, ref_obj)
 * Convenience macro around eo_xref_internal()
 * @see eo_xref()
 */
#define eo_xref(obj, ref_obj) eo_xref_internal(obj, ref_obj, __FILE__, __LINE__)

/**
 * @brief Increment the object's reference count by 1 (and associate the ref with ref_obj)
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 * @param file the call's filename.
 * @param line the call's line number.
 * @return The object passed (obj)
 *
 * People should not use this function, use #eo_xref instead.
 * A compile flag my make it and eobj_xunref() behave the same as eobj_ref()
 * and eobj_unref() respectively. So this should be used wherever possible.
 *
 * @see eo_xunref()
 */
EAPI Eo *eo_xref_internal(Eo *obj, const Eo *ref_obj, const char *file, int line);

/**
 * @brief Decrement the object's reference count by 1 and free it if needed. Will free the ref associated with ref_obj).
 * @param obj the object to work on.
 * @param ref_obj the object that references obj.
 *
 * This function only enforces the checks for object association. I.e don't rely
 * on it. If such enforces are compiled out, this function behaves the same as
 * eo_unref().
 *
 * @see eo_xref_internal()
 */
EAPI void eo_xunref(Eo *obj, const Eo *ref_obj);

/**
 * @brief Enable or disable the manual free feature.
 * @param obj the object to work on.
 * @param manual_free indicates if the free is manual (EINA_TRUE) or automatic (EINA_FALSE).
 *
 * The developer is in charge to call the function eo_manual_free to free the memory allocated for this object.
 *
 * Do not use, unless you really know what you are doing. It's used by Evas
 * because evas wants to keep its private data available even after the object
 * is deleted. Setting this to true makes Eo destruct the object but not free
 * the private data or the object itself.
 *
 * @see eo_manual_free()
 */
EAPI void eo_manual_free_set(Eo *obj, Eina_Bool manual_free);

/**
 * @brief Frees the object.
 * @param obj the object to work on.
 * This function must be called by the developer if the function
 * eo_manual_free_set has been called before with the parameter EINA_TRUE.
 * An error will be printed if this function is called when the manual
 * free option is not set to EINA_TRUE or the number of refs is not 0.
 *
 * @see eo_manual_free_set()
 */
EAPI void eo_manual_free(Eo *obj);

/**
 * @addtogroup Eo_Composite_Objects Composite Objects.
 * @{
 */

/**
 * @brief Make an object a composite object of another.
 * @param comp_obj the object that will be used to composite parent.
 * @param parent the "parent" object.
 *
 * This functions also sets the parent of comp_obj to parent.
 *
 * @see eo_composite_detach()
 * @see eo_composite_is()
 */
EAPI void eo_composite_attach(Eo *comp_obj, Eo *parent);

/**
 * @brief Detach a composite object from another object.
 * @param comp_obj the object attached to parent.
 * @param parent the "parent" object.
 *
 * This functions also sets the parent of comp_obj to @c NULL.
 *
 * @see eo_composite_attach()
 * @see eo_composite_is()
 */
EAPI void eo_composite_detach(Eo *comp_obj, Eo *parent);

/**
 * @brief Check if an object is a composite object.
 * @param comp_obj the object to be checked.
 * @return @c EINA_TRUE if it is, @c EINA_FALSE otherwise.
 *
 * @see eo_composite_attach()
 * @see eo_composite_detach()
 */
EAPI Eina_Bool eo_composite_is(const Eo *comp_obj);

/**
 * @}
 */

/**
 * @addtogroup Eo_Class_Base Eo's Base class.
 * @{
 */

/**
 * @def EO_BASE_CLASS
 * The class type for the Eo base class.
 */
#define EO_BASE_CLASS eo_base_class_get()
/**
 * @brief Use #EO_BASE_CLASS
 * @internal
 * */
EAPI const Eo_Class *eo_base_class_get(void);

/**
 * @typedef eo_base_data_free_func
 * Data free func prototype.
 */
typedef void (*eo_base_data_free_func)(void *);

/**
 * @var EO_BASE_BASE_ID
 * #EO_BASE_CLASS 's base id.
 */
extern EAPI Eo_Op EO_BASE_BASE_ID;

enum {
     EO_BASE_SUB_ID_CONSTRUCTOR,
     EO_BASE_SUB_ID_DESTRUCTOR,
     EO_BASE_SUB_ID_DATA_SET,
     EO_BASE_SUB_ID_DATA_GET,
     EO_BASE_SUB_ID_DATA_DEL,
     EO_BASE_SUB_ID_WREF_ADD,
     EO_BASE_SUB_ID_WREF_DEL,
     EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD,
     EO_BASE_SUB_ID_EVENT_CALLBACK_DEL,
     EO_BASE_SUB_ID_EVENT_CALLBACK_CALL,
     EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD,
     EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL,
     EO_BASE_SUB_ID_EVENT_FREEZE,
     EO_BASE_SUB_ID_EVENT_THAW,
     EO_BASE_SUB_ID_EVENT_FREEZE_GET,
     EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE,
     EO_BASE_SUB_ID_EVENT_GLOBAL_THAW,
     EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET,
     EO_BASE_SUB_ID_LAST
};

/**
 * @def EO_BASE_ID(sub_id)
 * Helper macro to get the full Op ID out of the sub_id for EO_BASE.
 * @param sub_id the sub id inside EO_BASE.
 */
#define EO_BASE_ID(sub_id) (EO_BASE_BASE_ID + (sub_id))

/**
 * @def eo_base_data_set(key, data, free_func)
 * Set generic data to object.
 * @param[in] key the key associated with the data
 * @param[in] data the data to set.
 * @param[in] free_func the func to free data with (NULL means "do nothing").
 *
 * @see #eo_base_data_get
 * @see #eo_base_data_del
 */
#define eo_base_data_set(key, data, free_func) EO_BASE_ID(EO_BASE_SUB_ID_DATA_SET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const void *, data), EO_TYPECHECK(eo_base_data_free_func, free_func)

/**
 * @def eo_base_data_get(key, data)
 * Get generic data from object.
 * @param[in] key the key associated with the data
 * @param[out] data the data for the key
 *
 * @see #eo_base_data_set
 * @see #eo_base_data_del
 */
#define eo_base_data_get(key, data) EO_BASE_ID(EO_BASE_SUB_ID_DATA_GET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(void **, data)

/**
 * @def eo_base_data_del(key)
 * Del generic data from object.
 * @param[in] key the key associated with the data
 *
 * @see #eo_base_data_set
 * @see #eo_base_data_get
 */
#define eo_base_data_del(key) EO_BASE_ID(EO_BASE_SUB_ID_DATA_DEL), EO_TYPECHECK(const char *, key)

/**
 * @def eo_wref_add
 * @brief Add a new weak reference to obj.
 * @param wref The pointer to use for the weak ref.
 *
 * This function registers the object handle pointed by wref to obj so when
 * obj is deleted it'll be updated to NULL. This functions should be used
 * when you want to keep track of an object in a safe way, but you don't want
 * to prevent it from being freed.
 *
 * @see #eo_wref_del
 */
#define eo_wref_add(wref) EO_BASE_ID(EO_BASE_SUB_ID_WREF_ADD), EO_TYPECHECK(Eo **, wref)

/**
 * @def eo_wref_del
 * @brief Delete the weak reference passed.
 * @param wref the weak reference to free.
 *
 * @see #eo_wref_add
 */
#define eo_wref_del(wref) EO_BASE_ID(EO_BASE_SUB_ID_WREF_DEL), EO_TYPECHECK(Eo **, wref)

/**
 * @def eo_wref_del_safe
 * @brief Delete the weak reference passed.
 * @param wref the weak reference to free.
 *
 * Same as eo_wref_del(), with the different that it's not called from eobj_do()
 * so you don't need to check if *wref is not NULL.
 *
 * @see #eo_wref_del
 */
#define eo_wref_del_safe(wref) \
   do { \
        if (*wref) eo_do(*wref, eo_wref_del(wref)); \
   } while (0)

/**
 * @def eo_constructor
 * @brief Call the object's constructor.
 *
 * Should not be used with #eo_do. Only use it with #eo_do_super.
 *
 * @see #eo_destructor
 */
#define eo_constructor() EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR)

/**
 * @def eo_destructor
 * @brief Call the object's destructor.
 *
 * Should not be used with #eo_do. Only use it with #eo_do_super.
 *
 * @see #eo_constructor
 */
#define eo_destructor() EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR)

/**
 * @addtogroup Eo_Events Eo's Event Handling
 * @{
 */

/**
 * @def EO_CALLBACK_PRIORITY_BEFORE
 * Slightly more prioritized than default.
 */
#define EO_CALLBACK_PRIORITY_BEFORE -100
/**
 * @def EO_CALLBACK_PRIORITY_DEFAULT
 * Default callback priority level
 */
#define EO_CALLBACK_PRIORITY_DEFAULT 0
/**
 * @def EO_CALLBACK_PRIORITY_AFTER
 * Slightly less prioritized than default.
 */
#define EO_CALLBACK_PRIORITY_AFTER 100

/**
 * @typedef Eo_Callback_Priority
 *
 * Callback priority value. Range is -32k - 32k. The lower the number, the
 * higher the priority.
 *
 * @see EO_CALLBACK_PRIORITY_AFTER
 * @see EO_CALLBACK_PRIORITY_BEFORE
 * @see EO_CALLBACK_PRIORITY_DEFAULT
 */
typedef short Eo_Callback_Priority;

/**
 * @def EO_CALLBACK_STOP
 * Stop calling callbacks for the even of which the callback was called for.
 * @see EO_CALLBACK_CONTINUE
 */
#define EO_CALLBACK_STOP EINA_FALSE

/**
 * @def EO_CALLBACK_CONTINUE
 * Continue calling callbacks for the even of which the callback was called for.
 * @see EO_CALLBACK_STOP
 */
#define EO_CALLBACK_CONTINUE EINA_TRUE

/**
 * @typedef Eo_Event_Cb
 *
 * An event callback prototype.
 *
 * @param data The user data registered with the callback.
 * @param obj The object which initiated the event.
 * @param desc The event's description.
 * @param event_info additional data passed with the event.
 * @return #EO_CALLBACK_STOP to stop calling additional callbacks for the event, #EO_CALLBACK_CONTINUE to continue.
 */
typedef Eina_Bool (*Eo_Event_Cb)(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info);

/**
 * @def eo_event_callback_forwarder_add
 * @brief Add an event callback forwarder for an event and an object.
 * @param[in] desc The description of the event to listen to.
 * @param[in] new_obj The object to emit events from.
 *
 * @see eo_event_callback_forwarder_del()
 */
#define eo_event_callback_forwarder_add(desc, new_obj) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_ADD), EO_TYPECHECK(const Eo_Event_Description *, desc), EO_TYPECHECK(Eo *, new_obj)

/**
 * @def eo_event_callback_forwarder_del
 * @brief Remove an event callback forwarder for an event and an object.
 * @param[in] desc The description of the event to listen to.
 * @param[in] new_obj The object to emit events from.
 *
 * @see eo_event_callback_forwarder_add()
 */
#define eo_event_callback_forwarder_del(desc, new_obj) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_FORWARDER_DEL), EO_TYPECHECK(const Eo_Event_Description *, desc), EO_TYPECHECK(Eo *, new_obj)

/**
 * @def eo_event_freeze
 * @brief freeze events of object.
 *
 * Prevents event callbacks from being called for the object.
 *
 * @see #eo_event_thaw
 */
#define eo_event_freeze() EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE)

/**
 * @def eo_event_thaw
 * @brief thaw events of object.
 *
 * Lets event callbacks be called for the object.
 *
 * @see #eo_event_freeze
 */
#define eo_event_thaw() EO_BASE_ID(EO_BASE_SUB_ID_EVENT_THAW)

/**
 * @def eo_event_freeze_get
 * @brief return freeze events of object.
 *
 * @param[out] fcount The event freeze count of the object.
 *
 * Return event freeze count.
 *
 * @see #eo_event_freeze
 * @see #eo_event_thaw
 */
#define eo_event_freeze_get(fcount) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_FREEZE_GET), EO_TYPECHECK(int *, fcount)

/**
 * @def eo_event_global_freeze
 * @brief freeze events of object.
 *
 * Prevents event callbacks from being called for the object.
 *
 * @see #eo_event_freeze
 * @see #eo_event_global_thaw
 */
#define eo_event_global_freeze() EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE)

/**
 * @def eo_event_global_thaw
 * @brief thaw events of object.
 *
 * Lets event callbacks be called for the object.
 *
 * @see #eo_event_thaw
 * @see #eo_event_global_freeze
 */
#define eo_event_global_thaw() EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_THAW)

/**
 * @def eo_event_global_freeze_get
 * @brief return freeze events of object.
 *
 * @param[out] fcount The event freeze count of the object.
 *
 * Return event freeze count.
 *
 * @see #eo_event_freeze_get
 * @see #eo_event_global_freeze
 * @see #eo_event_global_thaw
 */
#define eo_event_global_freeze_get(fcount) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_GLOBAL_FREEZE_GET), EO_TYPECHECK(int *, fcount)

/**
 * @def eo_event_callback_add(obj, desc, cb, data)
 * Add a callback for an event.
 * @param[in] desc The description of the event to listen to.
 * @param[in] cb the callback to call.
 * @param[in] data additional data to pass to the callback.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see eo_event_callback_priority_add()
 */
#define eo_event_callback_add(desc, cb, data) \
   eo_event_callback_priority_add(desc, \
         EO_CALLBACK_PRIORITY_DEFAULT, cb, data)

/**
 * @def eo_event_callback_priority_add
 * @brief Add a callback for an event with a specific priority.
 * @param[in] desc The description of the event to listen to.
 * @param[in] priority The priority of the callback.
 * @param[in] cb the callback to call.
 * @param[in] data additional data to pass to the callback.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see #eo_event_callback_add
 */
#define eo_event_callback_priority_add(desc, priority, cb, data) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_PRIORITY_ADD), EO_TYPECHECK(const Eo_Event_Description *, desc), EO_TYPECHECK(Eo_Callback_Priority, priority), EO_TYPECHECK(Eo_Event_Cb, cb), EO_TYPECHECK(const void *, data)


/**
 * @def eo_event_callback_del
 * @brief Del a callback with a specific data associated to it for an event.
 * @param[in] desc The description of the event to listen to.
 * @param[in] func the callback to delete.
 * @param[in] user_data The data to compare.
 *
 */
#define eo_event_callback_del(desc, func, user_data) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_DEL), EO_TYPECHECK(const Eo_Event_Description *, desc), EO_TYPECHECK(Eo_Event_Cb, func), EO_TYPECHECK(const void *, user_data)

/**
 * @def eo_event_callback_call
 * @brief Call the callbacks for an event of an object.
 * @param[in] desc The description of the event to call.
 * @param[in] event_info Extra event info to pass to the callbacks.
 * @param[out] aborted @c EINA_TRUE if one of the callbacks aborted the call, @c EINA_FALSE otherwise.
 */
#define eo_event_callback_call(desc, event_info, aborted) EO_BASE_ID(EO_BASE_SUB_ID_EVENT_CALLBACK_CALL), EO_TYPECHECK(const Eo_Event_Description *, desc), EO_TYPECHECK(const void *, event_info), EO_TYPECHECK(Eina_Bool *, aborted)

/**
 * @}
 */

/**
 * @var _EO_EV_CALLBACK_ADD
 * see EO_EV_CALLBACK_ADD
 */
EAPI extern const Eo_Event_Description _EO_EV_CALLBACK_ADD;

/**
 * @def EO_EV_CALLBACK_ADD
 * The event description (of type #Eo_Event_Description) for
 * The "Callback listener added" event.
 */
#define EO_EV_CALLBACK_ADD (&(_EO_EV_CALLBACK_ADD))

/**
 * @var _EO_EV_CALLBACK_DEL
 * see EO_EV_CALLBACK_DEL
 */
EAPI extern const Eo_Event_Description _EO_EV_CALLBACK_DEL;

/**
 * @def EO_EV_CALLBACK_DEL
 * The event description (of type #Eo_Event_Description) for
 * The "Callback listener deleted" event.
 */
#define EO_EV_CALLBACK_DEL (&(_EO_EV_CALLBACK_DEL))

/**
 * @var _EO_EV_DEL
 * see #EO_EV_DEL
 */
EAPI extern const Eo_Event_Description _EO_EV_DEL;

/**
 * @def EO_EV_DEL
 * Object is being deleted.
 */
#define EO_EV_DEL (&(_EO_EV_DEL))

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
