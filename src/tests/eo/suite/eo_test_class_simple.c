#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SIMPLE_CLASS

EAPI const Eo_Event_Description _EV_A_CHANGED =
        EO_EVENT_DESCRIPTION("a,changed");

EAPI const Eo_Event_Description _EV_A_CHANGED2 =
        EO_EVENT_DESCRIPTION("a,changed");

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   pd->a = a;

   eo_do(obj, eo_event_callback_call(obj, EV_A_CHANGED, &pd->a));
}

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;

   return pd->a;
}

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   const Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", eo_class_name_get(MY_CLASS), pd->a);

   return EINA_TRUE;
}

static Eina_Bool
_class_hi_print(Eo_Class *klass, void *data EINA_UNUSED)
{
   printf("Hi Print %s\n", eo_class_name_get(klass));

   return EINA_TRUE;
}

/* Eo* simple_part_get(Eo const* _object, const char* name) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef Eo* (*_Eo_simple_part_get_func)(Eo const*, void *obj_data, const char* name); */

/*    Eo* _ret; */
/*    EO_FUNC_COMMON_OP(simple_part_get, 0); */
/*     _ret = _func_(___call.obj, ___call.data, name); */
/*    _eo_do_end(_eo_stack_get()); */
/*    return _ret; */
/* } */

EO_FUNC_API_DEFINE(simple_part_get, Eo *, NULL, EO_FUNC_CALL(name), const char *name)

static Eo *
_part_get(Eo *obj, void *class_data EINA_UNUSED, const char *name EINA_UNUSED)
{
   /* A normal part get will do something saner, we just create objects. */
   Eo* o;
   eo_add(o, SIMPLE_CLASS, obj);
   return o;
}

/* void simple_recursive(Eo* _object, int n) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef void (*_Eo_simple_recursive_func)(Eo const*, void *obj_data, int n); */

/*    EO_FUNC_COMMON_OP(simple_recursive, ); */
/*    _func_(___call.obj, ___call.data, n); */
/*    _eo_do_end(_eo_stack_get()); */
/* } */

EO_FUNC_VOID_API_DEFINE(simple_recursive, EO_FUNC_CALL(n), int n)

static void
_recursive(Eo *obj, void *class_data EINA_UNUSED, int n)
{
   static int count = 0;

   if (count < n)
     {
        count++;
        eo_do(obj, simple_recursive(obj, n));
     }
   else
     count = 0;
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_super_eo_dbg_info_get(MY_CLASS, eo_obj, root);
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, "Test list");
   EO_DBG_INFO_APPEND(group, "Test", EINA_VALUE_TYPE_INT, 8);
}

/* void simple_a_set(Eo* _object, int a) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef void (*_Eo_simple_a_set_func)(Eo const*, void *obj_data, int a); */
   
/*    EO_FUNC_COMMON_OP(simple_a_set, ); */
/*     _func_(___call.obj, ___call.data, a); */
/*    _eo_do_end(_eo_stack_get()); */
/* } */

/* int simple_a_get(Eo* _object) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef int (*_Eo_simple_a_get_func)(Eo const*, void *obj_data); */

/*    int _ret; */
/*    EO_FUNC_COMMON_OP(simple_a_get, 0); */
/*    _ret = _func_(___call.obj, ___call.data); */
/*    _eo_do_end(_eo_stack_get()); */
/*    return _ret; */
/* } */

/* Eina_Bool simple_a_print(Eo* _object) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef Eina_Bool (*_Eo_simple_a_print_func)(Eo const*, void *obj_data); */

/*    Eina_Bool _ret; */
/*    EO_FUNC_COMMON_OP(simple_a_print, EINA_FALSE); */
/*    _ret = _func_(___call.obj, ___call.data); */
/*    _eo_do_end(_eo_stack_get()); */
/*    return _ret; */
/* } */

/* Eina_Bool simple_class_hi_print(Eo* _object) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef Eina_Bool (*_Eo_simple_class_hi_print_func)(Eo const*, void *obj_data); */

/*    Eina_Bool _ret; */
/*    EO_FUNC_COMMON_OP(simple_class_hi_print, EINA_FALSE); */
/*    _ret = _func_(___call.obj, ___call.data); */
/*    _eo_do_end(_eo_stack_get()); */
/*    return _ret; */
/* } */

/* void simple_pure_virtual(Eo * _object) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef void (*_Eo_simple_pure_virtual_func)(Eo const*, void *obj_data); */
   
/*    EO_FUNC_COMMON_OP(simple_pure_virtual, ); */
/*     _func_(___call.obj, ___call.data); */
/*    _eo_do_end(_eo_stack_get()); */
/* } */

/* void simple_no_implementation(Eo * _object) */
/* { */
/*    _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get()); */
/*    typedef void (*_Eo_simple_no_implementation_func)(Eo const*, void *obj_data); */
   
/*    EO_FUNC_COMMON_OP(simple_no_implementation, ); */
/*     _func_(___call.obj, ___call.data); */
/*    _eo_do_end(_eo_stack_get()); */
/* } */

EO_FUNC_VOID_API_DEFINE(simple_a_set, EO_FUNC_CALL(a), int a)
EO_FUNC_API_DEFINE(simple_a_get, int, 0, )
EO_FUNC_API_DEFINE(simple_a_print, Eina_Bool, EINA_FALSE, )
EO_FUNC_API_DEFINE(simple_class_hi_print, Eina_Bool, EINA_FALSE, )
EO_FUNC_VOID_API_DEFINE(simple_pure_virtual, )
EO_FUNC_VOID_API_DEFINE(simple_no_implementation, )

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_dbg_info_get, _dbg_info_get),
     EO_OP_FUNC(simple_a_set, _a_set),
     EO_OP_FUNC(simple_a_get, _a_get),
     EO_OP_FUNC(simple_a_print, _a_print),
     EO_OP_CLASS_FUNC(simple_class_hi_print, _class_hi_print),
     EO_OP_FUNC(simple_recursive, _recursive),
     EO_OP_FUNC(simple_part_get, _part_get),
     EO_OP_FUNC(simple_pure_virtual, NULL),
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Simple_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL)

