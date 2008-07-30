#ifndef EINA_LALLOC_H_
#define EINA_LALLOC_H_

#include "eina_types.h"

/**
 * @defgroup Array_Group Array
 * @{
 */

typedef void (*Eina_Array_Alloc) (void *user_data, int num);
#define EINA_ARRAY_ALLOC(function) ((Eina_Array_Alloc)function)
typedef void (*Eina_Array_Free) (void *user_data);
#define EINA_ARRAY_FREE(function) ((Eina_Array_Free)function)

typedef struct _Eina_Array Eina_Array;
EAPI void eina_array_free(Eina_Array *a);
EAPI Eina_Array *eina_array_new(void *data, Eina_Array_Alloc alloc_cb, Eina_Array_Free free_cb, int num_init);
EAPI void eina_array_elements_add(Eina_Array *a, int num);
EAPI void eina_array_element_add(Eina_Array *a);

/** @} */

#endif
