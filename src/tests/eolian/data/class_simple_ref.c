EWAPI float BAR = 10.300000f;

Eina_Bool _class_simple_a_set(Eo *obj, Evas_Simple_Data *pd, int value);


static Eina_Error
__eolian_class_simple_a_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   efl_canvas_object_simple_a_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_a_set, Eina_Bool, EINA_TRUE /* true */, EFL_FUNC_CALL(value), int value);

int _class_simple_a_get(const Eo *obj, Evas_Simple_Data *pd);


static Eina_Value
__eolian_class_simple_a_get_reflect(Eo *obj)
{
   int val = efl_canvas_object_simple_a_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(efl_canvas_object_simple_a_get, int, 100);

void _class_simple_b_set(Eo *obj, Evas_Simple_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(efl_canvas_object_simple_b_set);

char *_class_simple_foo(Eo *obj, Evas_Simple_Data *pd, int a, char *b, double *c, int *d);

static char *__eolian_class_simple_foo(Eo *obj, Evas_Simple_Data *pd, int a, char *b, double *c, int *d)
{
   if (c) *c = 1337.600000;
   return _class_simple_foo(obj, pd, a, b, c, d);
}

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_foo, char *, NULL /* null */, EFL_FUNC_CALL(a, b, c, d), int a, char *b, double *c, int *d);

int *_class_simple_bar(Eo *obj, Evas_Simple_Data *pd, int x);

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_bar, int *, NULL, EFL_FUNC_CALL(x), int x);

static Eina_Bool
_class_simple_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef CLASS_SIMPLE_EXTRA_OPS
#define CLASS_SIMPLE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_a_set, _class_simple_a_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_a_get, _class_simple_a_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_b_set, _class_simple_b_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_foo, __eolian_class_simple_foo),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_bar, _class_simple_bar),
      CLASS_SIMPLE_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"a", __eolian_class_simple_a_set_reflect, __eolian_class_simple_a_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _class_simple_class_desc = {
   EO_VERSION,
   "Class_Simple",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Simple_Data),
   _class_simple_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(class_simple_class_get, &_class_simple_class_desc, NULL, NULL);

#include "eolian_class_simple.eo.legacy.c"
