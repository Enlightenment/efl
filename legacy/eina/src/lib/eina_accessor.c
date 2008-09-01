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

#include "eina_accessor.h"
#include "eina_private.h"

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI void
eina_accessor_free(Eina_Accessor *accessor)
{
   if (accessor) accessor->free(accessor);
}

EAPI Eina_Bool
eina_accessor_data_get(Eina_Accessor *accessor, unsigned int position, void **data)
{
   if (!accessor) return EINA_FALSE;
   return accessor->get_at(accessor, position, data);
}

EAPI void *
eina_accessor_container_get(Eina_Accessor *accessor)
{
   if (!accessor) return NULL;
   return accessor->get_container(accessor);
}

EAPI void
eina_accessor_over(Eina_Accessor *accessor,
		   Eina_Each cb,
		   unsigned int start,
		   unsigned int end,
		   const void *fdata)
{
   void *container;
   void *data;
   unsigned int i = start;

   if (!accessor) return ;
   if (!(start < end)) return ;

   container = eina_accessor_container_get(accessor);
   for (i = start; i < end && accessor->get_at(accessor, i, &data) == EINA_TRUE; ++i)
      if (cb(container, data, (void*) fdata) != EINA_TRUE) return ;
}


