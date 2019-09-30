#ifndef _EOLIAN_FUNCTION_POINTERS_EOT_H_
#define _EOLIAN_FUNCTION_POINTERS_EOT_H_

#ifndef _FUNCTION_TYPES_EOT_TYPES
#define _FUNCTION_TYPES_EOT_TYPES

typedef void (*VoidFunc)(void *data);

typedef const char * (*SimpleFunc)(void *data, int a, double b);

typedef double (*ComplexFunc)(void *data, const char *c, char **d EFL_TRANSFER_OWNERSHIP);

typedef void (*FuncAsArgFunc)(void *data, void *cb_data, VoidFunc cb, Eina_Free_Cb cb_free_cb, void *another_cb_data, SimpleFunc another_cb, Eina_Free_Cb another_cb_free_cb);


#endif

#endif
