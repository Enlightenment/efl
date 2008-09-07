/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Cedric Bail
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

/**
 * @addtogroup Eina_Content_Access_Group Content Access
 *
 * @{
 */

/**
 * @addtogroup Eina_Iterator_Group Iterators Functions
 *
 * @brief These functions manage iterators on containers.
 *
 * These functions allow to access elements of a container in a
 * generic way, without knowing which container is used (a bit like
 * iterators in the C++ STL). Iterators only allows sequential access
 * (that is, from an element to the next one). For random access, see
 * @ref Eina_Accessor_Group.
 *
 * An iterator is created from container data types, so no creation
 * function is available here. An iterator is deleted with
 * eina_iterator_free(). To get the data and iterate, use
 * eina_iterator_next(). To call a function on all the elements of a
 * container, use eina_iterator_foreach().
 *
 * @{
 */

/**
 * @brief Free an iterator.
 *
 * @param iterator The iterator to free.
 *
 * This function frees @p iterator if it is not @c NULL;
 */
EAPI void
eina_iterator_free(Eina_Iterator *iterator)
{
   if (iterator) iterator->free(iterator);
}

/**
 * @brief Return the container of an iterator.
 *
 * @param iterator The iterator.
 * @return The container which created the iterator.
 *
 * This function returns the container which created @p iterator. If
 * @p iterator is @c NULL, this function returns @c NULL.
 */
EAPI void *
eina_iterator_container_get(Eina_Iterator *iterator)
{
   if (!iterator) return NULL;
   return iterator->get_container(iterator);
}

/**
 * @brief Return the value of the current element and go to the next one.
 *
 * @param iterator The iterator.
 * @param data The data of the element.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns the value of the current element pointed by
 * @p iterator in @p data, then goes to the next element. If @p
 * iterator is @c NULL or if a problem occured, #EINA_FALSE is
 * returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool
eina_iterator_next(Eina_Iterator *iterator, void **data)
{
   if (!iterator) return EINA_FALSE;
   return iterator->next(iterator, data);
}

/**
 * @brief Iterate over the container and execute a callback on each element.
 *
 * @param iterator The iterator.
 * @param cb The callback called on each iteration.
 * @param fdata The data passed to the callback.
 *
 * This function iterates over the elements pointed by @p iterator,
 * beginning from the current element. For Each element, the callback
 * @p cb is called with the data @p fdata.If @p iterator is @c NULL,
 * the function returns immediatly.
 */
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

/**
 * @}
 */

/**
 * @}
 */
