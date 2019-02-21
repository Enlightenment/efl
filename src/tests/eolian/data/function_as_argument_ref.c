
void _function_as_argument_set_cb(Eo *obj, Function_As_Argument_Data *pd, void *cb_data, SimpleFunc cb, Eina_Free_Cb cb_free_cb);

EOAPI EFL_VOID_FUNC_BODYV(function_as_argument_set_cb, EFL_FUNC_CALL(cb_data, cb, cb_free_cb), void *cb_data, SimpleFunc cb, Eina_Free_Cb cb_free_cb);

void _function_as_argument_set_nonull_cb(Eo *obj, Function_As_Argument_Data *pd, void *cb_data, VoidFunc cb, Eina_Free_Cb cb_free_cb);

EOAPI EFL_VOID_FUNC_BODYV(function_as_argument_set_nonull_cb, EFL_FUNC_CALL(cb_data, cb, cb_free_cb), void *cb_data, VoidFunc cb, Eina_Free_Cb cb_free_cb);

char *_function_as_argument_call_cb(Eo *obj, Function_As_Argument_Data *pd, int a, double b);

EOAPI EFL_FUNC_BODYV(function_as_argument_call_cb, char *, NULL, EFL_FUNC_CALL(a, b), int a, double b);

static Eina_Bool
_function_as_argument_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef FUNCTION_AS_ARGUMENT_EXTRA_OPS
#define FUNCTION_AS_ARGUMENT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(function_as_argument_set_cb, _function_as_argument_set_cb),
      EFL_OBJECT_OP_FUNC(function_as_argument_set_nonull_cb, _function_as_argument_set_nonull_cb),
      EFL_OBJECT_OP_FUNC(function_as_argument_call_cb, _function_as_argument_call_cb),
      FUNCTION_AS_ARGUMENT_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _function_as_argument_class_desc = {
   EO_VERSION,
   "Function_As_Argument",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Function_As_Argument_Data),
   _function_as_argument_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(function_as_argument_class_get, &_function_as_argument_class_desc, NULL, NULL);
