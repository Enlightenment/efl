#ifndef _EOLIAN_FUNCTION_POINTERS_H_
#define _EOLIAN_FUNCTION_POINTERS_H_

#ifndef _FUNCTION_AS_ARGUMENT_EO_CLASS_TYPE
#define _FUNCTION_AS_ARGUMENT_EO_CLASS_TYPE

typedef Eo Function_As_Argument;

#endif

#ifndef _FUNCTION_AS_ARGUMENT_EO_TYPES
#define _FUNCTION_AS_ARGUMENT_EO_TYPES


#endif
#define FUNCTION_AS_ARGUMENT_CLASS function_as_argument_class_get()

EWAPI const Efl_Class *function_as_argument_class_get(void);

EOAPI void function_as_argument_set_cb(Eo *obj, void *cb_data, SimpleFunc cb, Eina_Free_Cb cb_free_cb);

EOAPI void function_as_argument_set_nonull_cb(Eo *obj, void *cb_data, VoidFunc cb, Eina_Free_Cb cb_free_cb) EINA_ARG_NONNULL(3);

EOAPI const char *function_as_argument_call_cb(Eo *obj, int a, double b);

#endif
