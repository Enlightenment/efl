#ifndef EINA_LALLOC_H_
#define EINA_LALLOC_H_

#include "eina_types.h"

/**
 * @defgroup Lalloc_Group Lazy allocator
 * @{
 */

typedef Eina_Bool (*Eina_Lalloc_Alloc) (void *user_data, int num);
#define EINA_LALLOC_ALLOC(function) ((Eina_Lalloc_Alloc)function)
typedef void (*Eina_Lalloc_Free) (void *user_data);
#define EINA_LALLOC_FREE(function) ((Eina_Lalloc_Free)function)

typedef struct _Eina_Lalloc Eina_Lalloc;
EAPI void eina_lalloc_delete(Eina_Lalloc *a);
EAPI Eina_Lalloc *eina_lalloc_new(void *data, Eina_Lalloc_Alloc alloc_cb, Eina_Lalloc_Free free_cb, int num_init);
EAPI Eina_Bool eina_lalloc_elements_add(Eina_Lalloc *a, int num);
EAPI Eina_Bool eina_lalloc_element_add(Eina_Lalloc *a);

/** @} */

#endif
