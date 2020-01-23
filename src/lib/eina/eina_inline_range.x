/* EINA - EFL data type library
 * Copyright (C) 2020 Ali Alzyod
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
#ifndef EINA_INLINE_RANGE_H_
#define EINA_INLINE_RANGE_H_

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

static inline size_t eina_range_end(const Eina_Range *range)
{
   if (range) return range->start + range->length;
   return 0;
}

static inline Eina_Range eina_range_intersect(const Eina_Range *range, const Eina_Range *range2)
{
   Eina_Range ret_range = EINA_RANGE_EMPTY();
   size_t min, max;

   if (!range || !range2)
     return ret_range;

   min = MAX(range->start, range2->start);
   max = MIN(range->start + range->length, range2->start + range2->length);
 
   if (max > min)
     {
       ret_range = eina_range_from_to(min, max);
     }
 
   return ret_range;
}

static inline Eina_Range eina_range_union(const Eina_Range *range, const Eina_Range *range2)
{
   size_t min, max; 

   if (!range || !range2)
     return EINA_RANGE_EMPTY();

   min = MIN(range->start, range2->start);
   max = MAX(range->start + range->length, range2->start + range2->length);

   return eina_range_from_to(min, max);
}

static inline Eina_Bool eina_range_contains(const Eina_Range *range, size_t value)
{
   if (!range) return EINA_FALSE;

   return value >= range->start && value < (range->start + range->length);
}

static inline Eina_Bool eina_range_equal(const Eina_Range *range, const Eina_Range *range2)
{
   if (!range || !range2) return EINA_FALSE;
   return (range->start == range2->start && range->length == range2->length);
}

static inline Eina_Range eina_range_from_to(size_t min, size_t max)
{
   if (min < max) return EINA_RANGE(min, max - min);
   return EINA_RANGE(max, min - max);;
}

#endif // EINA_INLINE_RANGE_H_