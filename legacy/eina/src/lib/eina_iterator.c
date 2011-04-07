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

#include "eina_config.h"
#include "eina_private.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_iterator.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static const char EINA_MAGIC_ITERATOR_STR[] = "Eina Iterator";

#define EINA_MAGIC_CHECK_ITERATOR(d)                            \
   do {                                                          \
        if (!EINA_MAGIC_CHECK(d, EINA_MAGIC_ITERATOR)) {              \
             EINA_MAGIC_FAIL(d, EINA_MAGIC_ITERATOR); }                  \
     } while(0)

/**
 * @endcond
 */


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the iterator module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the iterator module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_iterator_init(void)
{
   return eina_magic_string_set(EINA_MAGIC_ITERATOR, EINA_MAGIC_ITERATOR_STR);
}

/**
 * @internal
 * @brief Shut down the iterator module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the iterator module set up by
 * eina_iterator_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_iterator_shutdown(void)
{
   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI void
eina_iterator_free(Eina_Iterator *iterator)
{
   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN(iterator);
   EINA_SAFETY_ON_NULL_RETURN(iterator->free);
   iterator->free(iterator);
}

EAPI void *
eina_iterator_container_get(Eina_Iterator *iterator)
{
   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator,                NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator->get_container, NULL);
   return iterator->get_container(iterator);
}

EAPI Eina_Bool
eina_iterator_next(Eina_Iterator *iterator, void **data)
{
   if (!iterator)
      return EINA_FALSE;

   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator,       EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator->next, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data,           EINA_FALSE);
   return iterator->next(iterator, data);
}

EAPI void
eina_iterator_foreach(Eina_Iterator *iterator,
                      Eina_Each_Cb cb,
                      const void *fdata)
{
   const void *container;
   void *data;

   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN(iterator);
   EINA_SAFETY_ON_NULL_RETURN(iterator->get_container);
   EINA_SAFETY_ON_NULL_RETURN(iterator->next);
   EINA_SAFETY_ON_NULL_RETURN(cb);

   if (!eina_iterator_lock(iterator)) return ;

   container = iterator->get_container(iterator);
   while (iterator->next(iterator, &data) == EINA_TRUE) {
        if (cb(container, data, (void *)fdata) != EINA_TRUE)
	   goto on_exit;
     }

 on_exit:
   (void) eina_iterator_unlock(iterator);
}

EAPI Eina_Bool
eina_iterator_lock(Eina_Iterator *iterator)
{
   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator, EINA_FALSE);

   if (iterator->lock)
      return iterator->lock(iterator);
   return EINA_TRUE;
}

EAPI Eina_Bool
eina_iterator_unlock(Eina_Iterator *iterator)
{
   EINA_MAGIC_CHECK_ITERATOR(iterator);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iterator, EINA_FALSE);

   if (iterator->unlock)
      return iterator->unlock(iterator);
   return EINA_TRUE;
}
