#ifndef EOBJ_H
#define EOBJ_H

#include <stdarg.h>
#include <Eina.h>

/* Check that the types are castable and cast them. */
#define EOBJ_TYPECHECK(type, x) \
   ({ \
    type __x; \
    __x = x; \
    (void) __x; \
    (type) x; \
    })

#define EOBJ_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

typedef struct _Eobj Eobj;
typedef uintptr_t Eobj_Op;

typedef struct _Eobj_Class Eobj_Class;
typedef int Eobj_Class_Id;

typedef enum
{
   EOBJ_CLASS_TYPE_REGULAR = 0,
   EOBJ_CLASS_TYPE_REGULAR_NO_INSTANT,
   EOBJ_CLASS_TYPE_INTERFACE,
   EOBJ_CLASS_TYPE_MIXIN
} Eobj_Class_Type;

typedef void (*eobj_op_func_type)(Eobj *, Eobj_Op, va_list *list);

typedef struct
{
   Eobj_Op op;
   eobj_op_func_type func;
} Eobj_Op_Func_Description;

#define EOBJ_OP_FUNC_DESCRIPTION(op, func) { op, func }
#define EOBJ_OP_FUNC_DESCRIPTION_SENTINEL { 0, NULL }

typedef struct
{
   const char *name; /**< name used for lookups */
   const char *type; /**< used for introspection purposes, documents what goes as callback event information (@c event_info) */
   const char *doc; /**< documentation for introspection purposes */
} Eobj_Event_Description;


typedef struct
{
   Eobj_Op sub_op;
   const char *name;
   /* FIXME: properly define the type so it'll support get/set and etc.
    * Do I even need/want type? If so, do I need/want it here? docs aren't
    * enough? */
   const char *type;
   const char *doc;
} Eobj_Op_Description;

typedef struct
{
   const char *name;
   Eobj_Class_Type type;
   struct {
        Eobj_Op *base_op_id;
        const Eobj_Op_Description *descs;
        size_t count;
   } ops;
   const Eobj_Event_Description **events;
   size_t private_size;
   void (*constructor)(Eobj *obj);
   void (*destructor)(Eobj *obj);
   void (*class_constructor)(Eobj_Class *klass);
   void (*class_destructor)(Eobj_Class *klass);
} Eobj_Class_Description;

#define EOBJ_CLASS_DESCRIPTION_OPS(base_op_id, op_descs, count) { base_op_id, op_descs, count }

#define EOBJ_OP_DESCRIPTION(op, type, doc) { op, #op, type, doc }
#define EOBJ_OP_DESCRIPTION_SENTINEL { 0, NULL, NULL, NULL }

#define EOBJ_EVENT_DESCRIPTION(name, type, doc) { name, type, doc }

EAPI Eina_Bool eobj_init(void);
EAPI Eina_Bool eobj_shutdown(void);

#define eobj_do(object, ...) eobj_do_internal(object, __VA_ARGS__, NULL)
/* FIXME: Change this to SUPER as well. */
#define eobj_class_do(object, klass, ...) eobj_class_do_internal(object, klass, __VA_ARGS__, NULL)
#define eobj_class_parent_do(object, klass, ...) eobj_class_do_internal(object, eobj_class_parent_get(klass), __VA_ARGS__, NULL)

EAPI Eina_Bool eobj_do_internal(Eobj *obj, ...);

EAPI Eina_Bool eobj_class_do_internal(Eobj *obj, const Eobj_Class *klass, ...);

EAPI const Eobj_Class *eobj_class_get(Eobj *obj);
EAPI const Eobj_Class *eobj_class_parent_get(const Eobj_Class *klass);
EAPI const char *eobj_class_name_get(const Eobj_Class *klass);

EAPI void eobj_constructor_super(Eobj *obj);
EAPI void eobj_destructor_super(Eobj *obj);
EAPI void eobj_constructor_error_set(Eobj *obj);
EAPI Eina_Bool eobj_constructor_error_get(const Eobj *obj);

EAPI Eobj_Class *eobj_class_new(const Eobj_Class_Description *desc, const Eobj_Class *parent, ...);
EAPI void eobj_class_free(Eobj_Class *klass);
EAPI void eobj_class_funcs_set(Eobj_Class *klass, const Eobj_Op_Func_Description *func_descs);

EAPI Eobj *eobj_add(const Eobj_Class *klass, Eobj *parent);
EAPI Eobj *eobj_parent_get(Eobj *obj);
EAPI void *eobj_data_get(Eobj *obj, const Eobj_Class *klass);
EAPI Eobj *eobj_ref(Eobj *obj);
EAPI void eobj_unref(Eobj *obj);
EAPI void eobj_del(Eobj *obj);
EAPI void *eobj_generic_data_set(Eobj *obj, const char *key, const void *data);
EAPI void *eobj_generic_data_get(const Eobj *obj, const char *key);
EAPI void *eobj_generic_data_del(Eobj *obj, const char *key);

#define EOBJ_CLASS_BASE eobj_base_class_get()
EAPI const Eobj_Class *eobj_base_class_get(void) EINA_CONST;

EAPI void eobj_composite_object_attach(Eobj *obj, Eobj *emb_obj);
EAPI void eobj_composite_object_detach(Eobj *obj, Eobj *emb_obj);
EAPI Eina_Bool eobj_composite_is(Eobj *emb_obj);

/* Events */
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
typedef short Eobj_Callback_Priority;

/* True meaning continue, False meaning stop callbacks. - Make it an enum? */
typedef Eina_Bool (*Eobj_Event_Cb)(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info);

EAPI Eina_Bool eobj_event_callback_forwarder_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj);
EAPI Eina_Bool eobj_event_callback_forwarder_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj *new_obj);

/* callbacks of the same priority are called in reverse order of creation. */
EAPI Eina_Bool eobj_event_callback_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb cb, const void *data);
EAPI Eina_Bool eobj_event_callback_priority_add(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Callback_Priority priority, Eobj_Event_Cb cb, const void *data);
EAPI void *eobj_event_callback_del(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func);
EAPI void *eobj_event_callback_del_full(Eobj *obj, const Eobj_Event_Description *desc, Eobj_Event_Cb func, const void *user_data);
EAPI Eina_Bool eobj_event_callback_call(Eobj *obj, const Eobj_Event_Description *desc, const void *event_info);

EAPI extern const Eobj_Event_Description _EOBJ_SIG_CALLBACK_ADD;
#define EOBJ_SIG_CALLBACK_ADD (&(_EOBJ_SIG_CALLBACK_ADD))
EAPI extern const Eobj_Event_Description _EOBJ_SIG_CALLBACK_DEL;
#define EOBJ_SIG_CALLBACK_DEL (&(_EOBJ_SIG_CALLBACK_DEL))

#endif
