#ifndef _EOLIAN_FUNCTION_POINTERS_EO_H_
#define _EOLIAN_FUNCTION_POINTERS_EO_H_

#ifndef _FUNCTION_AS_ARGUMENT_EO_CLASS_TYPE
#define _FUNCTION_AS_ARGUMENT_EO_CLASS_TYPE

typedef Eo Function_As_Argument;

#endif

#ifndef _FUNCTION_AS_ARGUMENT_EO_TYPES
#define _FUNCTION_AS_ARGUMENT_EO_TYPES


#endif
/** Docs for class Function_As_Argument.
 *
 * @since 1.66
 *
 * @ingroup Function_As_Argument
 */
#define FUNCTION_AS_ARGUMENT_CLASS function_as_argument_class_get()

EWAPI const Efl_Class *function_as_argument_class_get(void) EINA_CONST;

EOAPI void function_as_argument_set_cb(Eo *obj, void *cb_data, SimpleFunc cb, Eina_Free_Cb cb_free_cb);

EOAPI char *function_as_argument_call_cb(Eo *obj, int a, double b) EFL_TRANSFER_OWNERSHIP EINA_WARN_UNUSED_RESULT;

#endif
