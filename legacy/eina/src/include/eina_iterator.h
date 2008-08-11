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
#include "eina_error.h"

typedef struct _Eina_Iterator Eina_Iterator;

EAPI void eina_iterator_free           (Eina_Iterator *iterator);

EAPI void *eina_iterator_container_get (Eina_Iterator *iterator);
EAPI Eina_Bool eina_iterator_next      (Eina_Iterator *iterator, void **data);

EAPI void eina_iterator_foreach        (Eina_Iterator *iterator,
					Eina_Each callback,
					const void *fdata);

#endif
