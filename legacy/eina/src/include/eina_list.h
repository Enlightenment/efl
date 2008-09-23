/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_LIST_H_
#define EINA_LIST_H_

#include <stdlib.h>

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_List_Group List
 *
 * @{
 */

/**
 * @typedef Eina_List
 * Type for a generic single linked list.
 */
typedef struct _Eina_List Eina_List;

typedef struct _Eina_List_Accounting Eina_List_Accounting;

/**
 * @struct _Eina_List
 * Type for a generic single linked list.
 */
struct _Eina_List /** A linked list node */
{
   void      *data; /**< Pointer to list element payload */
   Eina_List *next; /**< Next member in the list */
   Eina_List *prev; /**< Previous member in the list */
   struct _Eina_List_Accounting *accounting; /**< Private list accounting info - don't touch */
};

struct _Eina_List_Accounting
{
   Eina_List *last;
   unsigned int count;
};


EAPI int eina_list_init(void);
EAPI int eina_list_shutdown(void);

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
EAPI void *eina_list_nth(const Eina_List *list, unsigned int n);
EAPI Eina_List *eina_list_nth_list (const Eina_List *list, unsigned int n);
EAPI Eina_List *eina_list_reverse (Eina_List *list);
EAPI Eina_List *eina_list_sort (Eina_List *list, unsigned int size, int(*func)(void*,void*));

static inline Eina_List *eina_list_last (const Eina_List *list);
static inline Eina_List *eina_list_next (const Eina_List *list);
static inline Eina_List *eina_list_prev (const Eina_List *list);
static inline void *eina_list_data(const Eina_List *list);
static inline unsigned int eina_list_count(const Eina_List *list);

EAPI Eina_Iterator *eina_list_iterator_new(const Eina_List *list);
EAPI Eina_Accessor *eina_list_accessor_new(const Eina_List *list);

/**
 * @def EINA_LIST_ITER_NEXT
 * @brief Macro to iterate over a list easily.
 *
 * @param list The list to iterate over.
 * @param l A list that is used as loop index.
 * @param data The data
 *
 * This macro allow the iteration over @p list in an easy way. It
 * iterates from the first element to the last one. @p data is the
 * data of each element of the list. @p l is an #Eina_List that is
 * used as counter.
 *
 * This macro can be used for freeing the data of alist, like in
 * the following example:
 *
 * @code
 * Eina_List *list;
 * Eina_List *l;
 * char       *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings,
 * // EINA_LIST_ITER_NEXT will be used to free those strings
 *
 * EINA_LIST_ITER_NEXT(list, l, data)
 *   free(data);
 * @endcode
 */
#define EINA_LIST_ITER_NEXT(list, l, data) for (l = list, data = eina_list_data(l); l; l = eina_list_next(l), data = eina_list_data(l))

#include "eina_inline_list.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_LIST_H_ */
