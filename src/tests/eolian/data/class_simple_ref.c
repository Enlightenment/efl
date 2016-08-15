
Eina_Bool _class_simple_a_set(Eo *obj, Evas_Simple_Data *pd, int value);

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_a_set, Eina_Bool, EINA_TRUE, EFL_FUNC_CALL(value), int value);

int _class_simple_a_get(Eo *obj, Evas_Simple_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(efl_canvas_object_simple_a_get, int, 100);

void _class_simple_b_set(Eo *obj, Evas_Simple_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(efl_canvas_object_simple_b_set);

char * _class_simple_foo(Eo *obj, Evas_Simple_Data *pd, int a, char *b, double *c, int *d);

static char * __eolian_class_simple_foo(Eo *obj, Evas_Simple_Data *pd, int a, char *b, double *c, int *d)
{
   if (c) *c = 1337.600000;
   return _class_simple_foo(obj, pd, a, b, c, d);
}

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_foo, char *, NULL, EFL_FUNC_CALL(a, b, c, d), int a, char *b, double *c, int *d);

int * _class_simple_bar(Eo *obj, Evas_Simple_Data *pd, int x);

EOAPI EFL_FUNC_BODYV(efl_canvas_object_simple_bar, int *, 0, EFL_FUNC_CALL(x), int x);

static const Efl_Op_Description _class_simple_op_desc[] = {
     EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_a_set, _class_simple_a_set),
     EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_a_get, _class_simple_a_get),
     EFL_OBJECT_OP_FUNC(efl_canvas_object_simple_foo, __eolian_class_simple_foo),
};

static const Efl_Class_Description _class_simple_class_desc = {
     EO_VERSION,
     "Class_Simple",
     EFL_CLASS_TYPE_REGULAR,
     EFL_CLASS_DESCRIPTION_OPS(_class_simple_op_desc),
     NULL,
     sizeof(Evas_Simple_Data),
     NULL,
     NULL
};

EFL_DEFINE_CLASS(class_simple_class_get, &_class_simple_class_desc, NULL, NULL);
