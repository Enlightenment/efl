/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Jorge Luis Zapata Muga, Cedric Bail
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>

#include "eina_iterator.h"
#include "eina_private.h"

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI void
eina_iterator_free(Eina_Iterator *iterator)
{
   if (iterator) iterator->free(iterator);
}

EAPI void *
eina_iterator_container_get(Eina_Iterator *iterator)
{
   if (!iterator) return NULL;
   return iterator->get_container(iterator);
}

EAPI Eina_Bool
eina_iterator_next(Eina_Iterator *iterator, void **data)
{
   if (!iterator) return EINA_FALSE;
   return iterator->next(iterator, data);
}

EAPI void
eina_iterator_foreach(Eina_Iterator *iterator,
		      Eina_Each cb,
		      const void *fdata)
{
   void *container;
   void *data;

   if (!iterator) return ;

   container = iterator->get_container(iterator);
   while (iterator->next(iterator, &data) == EINA_TRUE) {
      if (cb(container, data, (void*) fdata) != EINA_TRUE) return ;
   }
}
