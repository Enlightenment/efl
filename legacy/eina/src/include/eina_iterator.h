/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_ITERATOR_H__
#define EINA_ITERATOR_H__

#include "eina_types.h"
#include "eina_magic.h"

/**
 * @defgroup Eina_Iterator_Group Iterator Functions
 *
 * @{
 */

/**
 * @typedef Eina_Iterator
 * Type for iterators.
 */
typedef struct _Eina_Iterator Eina_Iterator;

typedef Eina_Bool (*Eina_Iterator_Next_Callback)(Eina_Iterator *it, void **data);
typedef void *(*Eina_Iterator_Get_Container_Callback)(Eina_Iterator *it);
typedef void (*Eina_Iterator_Free_Callback)(Eina_Iterator *it);

struct _Eina_Iterator
{
   Eina_Iterator_Next_Callback          next;
   Eina_Iterator_Get_Container_Callback get_container;
   Eina_Iterator_Free_Callback          free;

#define EINA_MAGIC_ITERATOR 0x98761233
   EINA_MAGIC;
};


#define FUNC_ITERATOR_NEXT(Function) ((Eina_Iterator_Next_Callback)Function)
#define FUNC_ITERATOR_GET_CONTAINER(Function) ((Eina_Iterator_Get_Container_Callback)Function)
#define FUNC_ITERATOR_FREE(Function) ((Eina_Iterator_Free_Callback)Function)

EAPI void eina_iterator_free           (Eina_Iterator *iterator);

EAPI void *eina_iterator_container_get (Eina_Iterator *iterator);
EAPI Eina_Bool eina_iterator_next      (Eina_Iterator *iterator, void **data);

EAPI void eina_iterator_foreach        (Eina_Iterator *iterator,
					Eina_Each callback,
					const void *fdata);

/**
 * @}
 */

#endif
