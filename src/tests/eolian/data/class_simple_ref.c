EWAPI float BAR = 10.300000f;

Eina_Bool _class_simple_a_set(Eo *obj, Evas_Simple_Data *pd, int value);

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_a_set, Eina_Bool, EINA_TRUE /* true */, EFL_FUNC_CALL(value), int value);

int _class_simple_a_get(const Eo *obj, Evas_Simple_Data *pd);

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
   const Efl_Object_Ops *opsp = NULL, *copsp = NULL;

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

#ifdef CLASS_SIMPLE_EXTRA_CLASS_OPS
   EFL_OPS_DEFINE(cops, CLASS_SIMPLE_EXTRA_CLASS_OPS);
   copsp = &cops;
#endif

   return efl_class_functions_set(klass, opsp, copsp);
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

EAPI void
evas_object_simple_b_set(Class_Simple *obj)
{
   efl_canvas_object_simple_b_set(obj);
}

EAPI int *
evas_object_simple_bar(Class_Simple *obj, int x)
{
   return efl_canvas_object_simple_bar(obj, x);
}
