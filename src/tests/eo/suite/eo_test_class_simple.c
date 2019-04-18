#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SIMPLE_CLASS

EAPI const Efl_Event_Description _EV_A_CHANGED =
        EFL_EVENT_DESCRIPTION("a,changed");

EAPI const Efl_Event_Description _EV_A_CHANGED2 =
        EFL_EVENT_DESCRIPTION("a,changed");

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", efl_class_name_get(MY_CLASS), a);
   pd->a = a;

   efl_event_callback_legacy_call(obj, EV_A_CHANGED, &pd->a);
}

static Eina_Error
_a_set_reflect(Eo *obj, Eina_Value value)
{
   int a;

   eina_value_int_convert(&value, &a);
   simple_a_set(obj, a);
   eina_value_flush(&value);

   return 0;
}

static int
_a_get(const Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;

   return pd->a;
}

static Eina_Value
_a_get_reflect(const Eo *obj)
{
   int a = simple_a_get(obj);

   return eina_value_int_init(a);
}

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   const Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", efl_class_name_get(MY_CLASS), pd->a);

   return EINA_TRUE;
}

EFL_FUNC_BODYV(simple_part_get, Eo *, NULL, EFL_FUNC_CALL(name), const char *name);

static Eo *
_part_get(Eo *obj, void *class_data EINA_UNUSED, const char *name EINA_UNUSED)
{
   /* A normal part get will do something saner, we just create objects. */
   return efl_add(SIMPLE_CLASS, obj);
}

EFL_VOID_FUNC_BODYV(simple_recursive, EFL_FUNC_CALL(n), int n);

static void
_recursive(Eo *obj, void *class_data EINA_UNUSED, int n)
{
   static int count = 0;

   if (count < n)
     {
        count++;
        simple_recursive(obj, n);
     }
   else
     count = 0;
}

static void
_dbg_info_get(Eo *eo_obj, void *_pd EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, "Test list");
   EFL_DBG_INFO_APPEND(group, "Test", EINA_VALUE_TYPE_INT, 8);
}

EFL_VOID_FUNC_BODYV(simple_a_set, EFL_FUNC_CALL(a), int a);
EFL_FUNC_BODY_CONST(simple_a_get, int, 0);
EFL_FUNC_BODY(simple_a_print, Eina_Bool, EINA_FALSE);
EFL_VOID_FUNC_BODY(simple_pure_virtual);
EFL_VOID_FUNC_BODY(simple_no_implementation);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
         EFL_OBJECT_OP_FUNC(simple_a_print, _a_print),
         EFL_OBJECT_OP_FUNC(simple_recursive, _recursive),
         EFL_OBJECT_OP_FUNC(simple_part_get, _part_get),
         EFL_OBJECT_OP_FUNC(simple_pure_virtual, NULL),
         EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _dbg_info_get),
   );
   static const Efl_Object_Property_Reflection reflection_table[] = {
         {"simple_a", _a_set_reflect, _a_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops ref_ops = {
         reflection_table, EINA_C_ARRAY_LENGTH(reflection_table)
   };

   return efl_class_functions_set(klass, &ops, &ref_ops);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL)
EFL_FUNC_BODY_CONST(simple2_class_beef_get, int, 0);

static Eina_Bool
_class_initializer2(Efl_Class *klass)
{

   return efl_class_functions_set(klass, NULL, NULL);
}

static const Efl_Class_Description class_desc2 = {
     EO_VERSION,
     "Simple2",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer2,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple2_class_get, &class_desc2, EO_CLASS, NULL)


static Eina_Bool
_class_initializer3(Efl_Class *klass)
{
   return efl_class_functions_set(klass, NULL, NULL);
}

static const Efl_Class_Description class_desc3 = {
     EO_VERSION,
     "Simple3",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer3,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple3_class_get, &class_desc3, SIMPLE_CLASS, SIMPLE2_CLASS, NULL)


static Efl_Object*
_interface_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Efl_Object *klass)
{
   if (klass == SEARCHABLE_CLASS) return obj;

   return efl_provider_find(efl_super(obj, SEARCHABLE_CLASS), klass);
}

static Eina_Bool
_searchable_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_provider_find, _interface_get)
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc_searchable = {
     EO_VERSION,
     "Searchable",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _searchable_class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(searchable_class_get, &class_desc_searchable, EO_CLASS, NULL)
