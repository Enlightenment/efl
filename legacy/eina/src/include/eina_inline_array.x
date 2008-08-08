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

#ifdef __GNUC__
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define UNLIKELY(x) (x)
#endif

static inline Eina_Bool
eina_array_grow(Eina_Array *array)
{
   void **tmp;
   unsigned int total;

   total = array->total + array->step;
   tmp = realloc(array->data, sizeof (void*) * total);
   if (UNLIKELY(!tmp)) return 0;

   array->total = total;
   array->data = tmp;

   return 1;
}

static inline void
eina_array_append(Eina_Array *array, void *data)
{
   if (UNLIKELY((array->count + array->step) > array->total))
     if (!eina_array_grow(array)) return ;

   array->data[array->count++] = data;
}

static inline void *
eina_array_get(Eina_Array *array, unsigned int index)
{
   return array->data[index];
}

static inline unsigned int
eina_array_count(Eina_Array *array)
{
   return array->count;
}

#endif
