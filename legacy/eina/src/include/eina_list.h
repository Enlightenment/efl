#ifndef EINA_LIST_H_
#define EINA_LIST_H_

#include "eina_types.h"

/**
 * @defgroup List_Group List
 * @{
 */

typedef struct _Eina_List Eina_List;

EAPI Eina_List *eina_list_append (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_prepend (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_append_relative (Eina_List *list, const void *data, const void *relative);
EAPI Eina_List *eina_list_append_relative_list (Eina_List *list, const void *data, Eina_List *relative);
EAPI Eina_List *eina_list_prepend_relative (Eina_List *list, const void *data, const void *relative);
EAPI Eina_List *eina_list_prepend_relative_list (Eina_List *list, const void *data, Eina_List *relative);
EAPI Eina_List *eina_list_remove (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_remove_list (Eina_List *list, Eina_List *remove_list);
EAPI Eina_List *eina_list_promote_list (Eina_List *list, Eina_List *move_list);
EAPI void *eina_list_find(const Eina_List *list, const void *data);
EAPI Eina_List *eina_list_find_list (const Eina_List *list, const void *data);
EAPI Eina_List *eina_list_free (Eina_List *list);
EAPI Eina_List *eina_list_last (const Eina_List *list);
EAPI Eina_List *eina_list_next (const Eina_List *list);
EAPI Eina_List *eina_list_prev (const Eina_List *list);
EAPI void *eina_list_data(const Eina_List *list);
EAPI int eina_list_count(const Eina_List *list);
EAPI void *eina_list_nth(const Eina_List *list, int n);
EAPI Eina_List *eina_list_nth_list (const Eina_List *list, int n);
EAPI Eina_List *eina_list_reverse (Eina_List *list);
EAPI Eina_List *eina_list_sort (Eina_List *list, int size, int(*func)(void*,void*));
EAPI int eina_list_alloc_error(void);


/** @} */

#endif /* EINA_LIST_H_ */
