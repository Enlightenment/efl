/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "eina_error.h"
#include "eina_array.h"
#include "eina_inline_array.x"

EAPI int
eina_array_init(void)
{
   return eina_error_init();
}

EAPI int
eina_array_shutdown(void)
{
   return eina_error_shutdown();
}

EAPI void
eina_array_append(Eina_Array *array, void *data)
{
   _eina_array_append(array, data);
}

EAPI void *
eina_array_get(Eina_Array *array, unsigned int index)
{
   return _eina_array_get(array, index);
}

EAPI void
eina_array_clean(Eina_Array *array)
{
   array->count = 0;
}

EAPI void
eina_array_setup(Eina_Array *array, unsigned int step)
{
   array->step = step;
}

EAPI void
eina_array_flush(Eina_Array *array)
{
   array->count = 0;
   array->total = 0;

   if (array->data) free(array->data);
   array->data = NULL;
}

EAPI Eina_Array *
eina_array_new(unsigned int step)
{
   Eina_Array *array;

   eina_error_set(0);
   array = malloc(sizeof (Eina_Array));
   if (!array) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   array->data = NULL;
   array->total = 0;
   array->count = 0;
   array->step = step;

   return array;
}

EAPI void
eina_array_free(Eina_Array *array)
{
   eina_array_flush(array);
   free(array);
}

#include <stdio.h>

EAPI void
eina_array_remove(Eina_Array *array, Eina_Bool (*keep)(void *data, void *gdata), void *gdata)
{
   void **tmp;
   unsigned int total = 0;
   unsigned int limit;
   unsigned int i;

   if (array->total == 0) return ;

   for (i = 0; i < array->count; ++i)
     {
	void *data;

	data = _eina_array_get(array, i);

	if (keep(data, gdata) == EINA_FALSE)
	  break;
     }
   limit = i;
   for (; i < array->count; ++i)
     {
	void *data;

	data = _eina_array_get(array, i);

	if (keep(data, gdata) == EINA_TRUE)
	  break;
     }
   /* Special case all objects that need to stay are at the beginning of the array. */
   if (i == array->count)
     {
	array->count = limit;
	if (array->count == 0)
	  {
	     free(array->data);
	     array->total = 0;
	     array->data = NULL;
	  }

	return ;
     }

   eina_error_set(0);
   tmp = malloc(sizeof (void*) * array->total);
   if (!tmp) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return ;
   }

   memcpy(tmp, array->data, limit * sizeof(void*));
   total = limit;

   for (; i < array->count; ++i)
     {
	void *data;

	data = _eina_array_get(array, i);

	if (keep(data, gdata))
	  {
	     tmp[total] = data;
	     total++;
	  }
     }

   free(array->data);

   /* If we do not keep any object in the array, we should have exited
      earlier in test (i == array->count). */
   assert(total != 0);

   array->data = tmp;
   array->count = total;
}
