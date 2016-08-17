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
 * @li @ref Eo_Events
 * @li @ref Eo_Composite_Objects
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
EAPI extern Eina_Spinlock _efl_class_creation_lock;

/**
 * @var _efl_object_init_generation
 * This variable stores the current eo init generation. That is, how many times
 * we have completed full init/shutdown cycles. Starts at 1 and incremeted on
 * every call to shutdown that actually shuts down eo.
 * @internal
 */
EAPI extern unsigned int _efl_object_init_generation;

/**
 * @internal
 * An enum representing the possible types of an Op.
 */
enum _Efl_Object_Op_Type
{
   EFL_OBJECT_OP_TYPE_INVALID = -1, /**< Invalid op. */
   EFL_OBJECT_OP_TYPE_REGULAR = 0, /**< Regular op. */
   EFL_OBJECT_OP_TYPE_CLASS, /**< Class op - a class op. Like static in Java/C++. */
   EFL_OBJECT_OP_TYPE_REGULAR_OVERRIDE, /**< Regular op override (previously defined) */
   EFL_OBJECT_OP_TYPE_CLASS_OVERRIDE, /**< Class op override (previously defined) */
};

/**
 * @internal
 * @typedef Efl_Object_Op_Type
 * A convenience typedef for #_Efl_Object_Op_Type.
 */
typedef enum _Efl_Object_Op_Type Efl_Object_Op_Type;

/**
 * @typedef Efl_Del_Intercept
 *
 * A function to be called on object deletion/destruction instead of normal
 * destruction taking place.
 *
 * @param obj_id The object needing destruction
 */
typedef void (*Efl_Del_Intercept) (Eo *obj_id);

#include "efl_object_override.eo.h"
#include "efl_object.eo.h"
#include "efl_interface.eo.h"
#define EO_CLASS EFL_OBJECT_CLASS

/**
 * @addtogroup Eo_Debug_Information Eo's Debug information helper.
 * @{
 */

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
 * @addtogroup Eo_Events Eo's Event Handling
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
   static volatile unsigned char lk_init = 0; \
   static Eina_Spinlock _my_lock; \
   static const Efl_Class * volatile _my_class = NULL; \
   static unsigned int _my_init_generation = 1; \
   if (EINA_UNLIKELY(_efl_object_init_generation != _my_init_generation)) \
     { \
        _my_class = NULL; /* It's freed in efl_object_shutdown(). */ \
        lk_init = 0; \
     } \
   if (EINA_LIKELY(!!_my_class)) return _my_class; \
   \
   eina_spinlock_take(&_efl_class_creation_lock); \
   if (!lk_init) \
      eina_spinlock_new(&_my_lock); \
   if (lk_init < 2) eina_spinlock_take(&_my_lock); \
   if (!lk_init) \
      lk_init = 1; \
   else \
     { \
        if (lk_init < 2) eina_spinlock_release(&_my_lock); \
        eina_spinlock_release(&_efl_class_creation_lock); \
        return _my_class; \
     } \
   eina_spinlock_release(&_efl_class_creation_lock); \
   _tmp_parent_class = parent_class; \
   _my_class = efl_class_new(class_desc, _tmp_parent_class, __VA_ARGS__); \
   _my_init_generation = _efl_object_init_generation; \
   eina_spinlock_release(&_my_lock); \
   \
   eina_spinlock_take(&_efl_class_creation_lock); \
   eina_spinlock_free(&_my_lock); \
   lk_init = 2; \
   eina_spinlock_release(&_efl_class_creation_lock); \
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
   Efl_Object_Op_Type op_type;     /**< The type of the Op. */
} Efl_Op_Description;

/**
 * @struct _Efl_Object_Ops
 *
 * This struct holds the ops and the size of the ops.
 * Please use the #EFL_CLASS_DESCRIPTION_OPS macro when populating it.
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
 * Please use the #EFL_CLASS_DESCRIPTION_OPS macro when populating it.
 */
struct _Efl_Class_Description
{
   unsigned int version; /**< The current version of eo, use #EO_VERSION */
   const char *name; /**< The name of the class. */
   Efl_Class_Type type; /**< The type of the class. */
   Efl_Object_Ops ops;  /**< The ops description, should be filled using #EFL_CLASS_DESCRIPTION_OPS (later sorted by Eo). */
   const Efl_Event_Description * const *events; /**< The event descriptions for this class. */
   size_t data_size; /**< The size of data (private + protected + public) this class needs per object. */
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
 * You should use #EFL_DEFINE_CLASS. It'll provide thread safety and other
 * features easily.
 *
 * @see #EFL_DEFINE_CLASS
 */
EAPI const Efl_Class *efl_class_new(const Efl_Class_Description *desc, const Efl_Class *parent, ...);

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
 */
EAPI Eina_Bool efl_object_override(Eo *obj, const Efl_Object_Ops *ops);

/**
 * @brief Define an array of override functions for @ref efl_object_override
 * @param ops A name for the Efl_Object_Ops local variable to define
 * @param ... A comma separated list of Efl_Object_Op overrides, using
 *            #EFL_OBJECT_OP_FUNC_OVERRIDE or #EFL_OBJECT_OP_CLASS_FUNC_OVERRIDE
 *
 * This can be used as follows:
 * @code
 * EFL_OBJECT_OVERRIDE_OPS_DEFINE(ops, EFL_OBJECT_OP_FUNC_OVERRIDE(public_func, _my_func));
 * efl_object_override(obj, &ops);
 * @endcode
 *
 * @see efl_object_override
 */
#define EFL_OBJECT_OVERRIDE_OPS_DEFINE(ops, ...) \
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

// Helpers macro to help populating #Efl_Class_Description.
#define EFL_CLASS_DESCRIPTION_NOOPS() { NULL, 0}
#define EFL_CLASS_DESCRIPTION_OPS(op_descs) { op_descs, EINA_C_ARRAY_LENGTH(op_descs) }

// to fetch internal function and object data at once
typedef struct _Efl_Object_Op_Call_Data
{
   Eo *eo_id;
   _Eo_Object *obj;
   void     *func;
   void     *data;
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

// cache OP id, get real fct and object data then do the call
#define EFL_FUNC_COMMON_OP(Obj, Name, DefRet)                                 \
     static Efl_Object_Call_Cache ___cache; /* static 0 by default */           \
     Efl_Object_Op_Call_Data ___call;                                           \
     if (EINA_UNLIKELY((___cache.op == EFL_NOOP) ||                      \
                       (___cache.generation != _efl_object_init_generation)))   \
       {                                                                \
          ___cache.op = _efl_object_api_op_id_get(EFL_FUNC_COMMON_OP_FUNC(Name)); \
          if (___cache.op == EFL_NOOP) return DefRet;                    \
          ___cache.generation = _efl_object_init_generation;                    \
       }                                                                \
     if (!_efl_object_call_resolve((Eo *) Obj, #Name, &___call, &___cache,                  \
                           __FILE__, __LINE__)) return DefRet;          \
     _Eo_##Name##_func _func_ = (_Eo_##Name##_func) ___call.func;       \

#define _EFL_OBJECT_API_BEFORE_HOOK
#define _EFL_OBJECT_API_AFTER_HOOK
#define _EFL_OBJECT_API_CALL_HOOK(x) x

// to define an EAPI function
#define _EFL_OBJECT_FUNC_BODY(Name, ObjType, Ret, DefRet) \
  Ret                                                                   \
  Name(ObjType obj)                                                            \
  {                                                                     \
     typedef Ret (*_Eo_##Name##_func)(Eo *, void *obj_data);            \
     Ret _r;                                                            \
     EFL_FUNC_COMMON_OP(obj, Name, DefRet);                                   \
     _EFL_OBJECT_API_BEFORE_HOOK                                                       \
     _r = _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data));     \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK                                                       \
     return _r;                                                         \
  }

#define _EFL_OBJECT_VOID_FUNC_BODY(Name, ObjType)                               \
  void									\
  Name(ObjType obj)                                                            \
  {                                                                     \
     typedef void (*_Eo_##Name##_func)(Eo *, void *obj_data);           \
     EFL_FUNC_COMMON_OP(obj, Name, );                                         \
     _EFL_OBJECT_API_BEFORE_HOOK                                                       \
     _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data));          \
     _efl_object_call_end(&___call);                                            \
     _EFL_OBJECT_API_AFTER_HOOK                                                       \
  }

#define _EFL_OBJECT_FUNC_BODYV(Name, ObjType, Ret, DefRet, Arguments, ...)      \
  Ret                                                                   \
  Name(ObjType obj, __VA_ARGS__)                                                     \
  {                                                                     \
     typedef Ret (*_Eo_##Name##_func)(Eo *, void *obj_data, __VA_ARGS__); \
     Ret _r;                                                            \
     EFL_FUNC_COMMON_OP(obj, Name, DefRet);                                   \
     _EFL_OBJECT_API_BEFORE_HOOK                                                       \
     _r = _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data, Arguments)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK                                                        \
     return _r;                                                         \
  }

#define _EFL_OBJECT_VOID_FUNC_BODYV(Name, ObjType, Arguments, ...)              \
  void                                                                  \
  Name(ObjType obj, __VA_ARGS__)                                                     \
  {                                                                     \
     typedef void (*_Eo_##Name##_func)(Eo *, void *obj_data, __VA_ARGS__); \
     EFL_FUNC_COMMON_OP(obj, Name, );                                         \
     _EFL_OBJECT_API_BEFORE_HOOK                                                       \
     _EFL_OBJECT_API_CALL_HOOK(_func_(___call.eo_id, ___call.data, Arguments)); \
     _efl_object_call_end(&___call); \
     _EFL_OBJECT_API_AFTER_HOOK                                                        \
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

#define EFL_OBJECT_OP_FUNC(_api, _private) { _EFL_OBJECT_OP_API_ENTRY(_api), (void*)_private, EFL_OBJECT_OP_TYPE_REGULAR }
#define EFL_OBJECT_OP_CLASS_FUNC(_api, _private) { _EFL_OBJECT_OP_API_ENTRY(_api), (void*)_private, EFL_OBJECT_OP_TYPE_CLASS }
#define EFL_OBJECT_OP_FUNC_OVERRIDE(_api, _private) { _EFL_OBJECT_OP_API_ENTRY(_api), (void*)_private, EFL_OBJECT_OP_TYPE_REGULAR_OVERRIDE }
#define EFL_OBJECT_OP_CLASS_FUNC_OVERRIDE(_api, _private) { _EFL_OBJECT_OP_API_ENTRY(_api), (void*)_private, EFL_OBJECT_OP_TYPE_CLASS_OVERRIDE }

// returns the OP id corresponding to the given api_func
EAPI Efl_Object_Op _efl_object_api_op_id_get(const void *api_func);

// gets the real function pointer and the object data
EAPI Eina_Bool _efl_object_call_resolve(Eo *obj, const char *func_name, Efl_Object_Op_Call_Data *call, Efl_Object_Call_Cache *callcache, const char *file, int line);

// end of the eo call barrier, unref the obj
EAPI void _efl_object_call_end(Efl_Object_Op_Call_Data *call);

// end of the efl_add. Calls finalize among others
EAPI Eo * _efl_add_end(Eo *obj, Eina_Bool is_ref, Eina_Bool is_fallback);

EAPI Eo *efl_super(const Eo *obj, const Efl_Class *cur_klass);

/*****************************************************************************/

/**
 * @brief Gets the class of the object.
 * @param obj The object to work on
 * @return The object's class.
 *
 * @see efl_class_name_get()
 */
EAPI const Efl_Class *efl_class_get(const Eo *obj);

EAPI Eo *_efl_self_get(void);

/* Check if GCC compatible (both GCC and clang define this) */
#if defined(__GNUC__) && !defined(_EO_ADD_FALLBACK_FORCE)

# define efl_self __efl_self

# define _efl_add_common(klass, parent, is_ref, ...) \
   ({ \
     Eo * const __efl_self = _efl_add_internal_start(__FILE__, __LINE__, klass, parent, is_ref, EINA_FALSE); \
     (void) ((void)0, ##__VA_ARGS__);                                   \
     (Eo *) _efl_add_end(efl_self, is_ref, EINA_FALSE); \
    })

#else

# define efl_self _efl_self_get()

# define _efl_add_common(klass, parent, is_ref, ...) \
   ( \
     _efl_add_internal_start(__FILE__, __LINE__, klass, parent, is_ref, EINA_TRUE), \
     ##__VA_ARGS__, \
     (Eo *) _efl_add_end(efl_self, is_ref, EINA_TRUE) \
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
 * The data reference count is not incremented. The pointer must be used only
 * in the scope of the function and its callees.
 * @param obj the object to work on.
 * @param klass the klass associated with the data.
 * @return a pointer to the data.
 *
 * @see efl_data_ref()
 * @see efl_data_unref()
 */
EAPI void *efl_data_scope_get(const Eo *obj, const Efl_Class *klass);

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
 * @addtogroup Eo_Events Eo's Event Handling
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
 * Helper for creating global callback arrays.
 * The problem is on windows where you can't declare a static array with
 * external symbols in it, because the addresses are only known at runtime.
 */
#define EFL_CALLBACKS_ARRAY_DEFINE(Name, ...)                            \
  static Efl_Callback_Array_Item *                                       \
  Name(void)                                                            \
  {                                                                     \
     static Efl_Callback_Array_Item internal[sizeof ((Efl_Callback_Array_Item[]) { __VA_ARGS__ }) / \
                                            sizeof (Efl_Callback_Array_Item) + \
                                            1] = { { 0, 0 } };          \
     if (internal[0].desc == NULL)                                      \
       {                                                                \
          memcpy(internal,                                              \
                 ((Efl_Callback_Array_Item[]) { __VA_ARGS__, { NULL, NULL } }), \
                 sizeof (Efl_Callback_Array_Item) +                      \
                 sizeof ((Efl_Callback_Array_Item[]) { __VA_ARGS__ }));  \
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
 * Add a callback array for an event.
 * @param[in] array an #Efl_Callback_Array_Item of events to listen to.
 * @param[in] data additional data to pass to the callback.
 *
 * callbacks of the same priority are called in reverse order of creation.
 *
 * @see efl_event_callback_array_priority_add()
 */
#define efl_event_callback_array_add(obj, array, data) \
   efl_event_callback_array_priority_add(obj, array, \
         EFL_CALLBACK_PRIORITY_DEFAULT, data)

/**
 * @}
 */

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
