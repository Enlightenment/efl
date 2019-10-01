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

#ifndef EINA_INLINE_ARRAY_X_
#define EINA_INLINE_ARRAY_X_

#include <stddef.h>

#include <stdio.h>

/**
 * @cond LOCAL
 */

EAPI Eina_Bool eina_array_grow(Eina_Array *array);

/**
 * @endcond
 */

/**
 * @addtogroup Eina_Array_Group Array
 *
 * @brief These functions provide array management.
 *
 * @{
 */

static inline Eina_Bool
eina_array_push(Eina_Array *array, const void *data)
{
   if (data)
     {
        if (EINA_UNLIKELY((array->count + 1) > array->total)) goto do_grow;
do_grow_back:

        array->data[array->count++] = (void*) data;

        return EINA_TRUE;
     }
   return EINA_FALSE;
do_grow:
   if (!eina_array_grow(array)) return EINA_FALSE;
   goto do_grow_back;
}

static inline void *
eina_array_pop(Eina_Array *array)
{
   if (array->count > 0) return array->data[--array->count];
   return NULL;
}

static inline void *
eina_array_data_get(const Eina_Array *array, unsigned int idx)
{
   return array->data[idx];
}

static inline void
eina_array_data_set(const Eina_Array *array, unsigned int idx, const void *data)
{
   array->data[idx] = (void*) data;
}


static inline unsigned int
eina_array_count_get(const Eina_Array *array)
{
   return array->count;
}


static inline unsigned int
eina_array_count(const Eina_Array *array)
{
   return array->count;
}

static inline Eina_Bool
eina_array_find(const Eina_Array *array, const void *data, unsigned int *out_idx)
{
   unsigned int i;

   if (!array) return EINA_FALSE;

   for (i = 0; i < array->count; i++)
     {
        if (array->data[i] == data)
          {
             if (out_idx) *out_idx = i;

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static inline Eina_Bool
eina_array_foreach(Eina_Array *array, Eina_Each_Cb cb, void *fdata)
{
   void *data;
   Eina_Array_Iterator iterator;
   unsigned int i;
   Eina_Bool ret = EINA_TRUE;

   EINA_ARRAY_ITER_NEXT(array, i, data, iterator)
     {
        if (cb(array, data, fdata) == EINA_TRUE) continue;
        ret = EINA_FALSE;
        break;
     }
   return ret;
}

static inline void
eina_array_clean(Eina_Array *array)
{
   array->count = 0;
}

/**
 * @}
 */

#endif
