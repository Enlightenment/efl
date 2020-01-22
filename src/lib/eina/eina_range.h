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

#ifndef EINA_RANGE_H_
#define EINA_RANGE_H_

#include "eina_types.h"

#include "eina_cpu.h"

#define EINA_RANGE(start, length) ((Eina_Range) { (start), (length) })
#define EINA_RANGE_FROM_TO(from, to) ((Eina_Range) { MIN((from),(to)), ABS((to) - (from)) })
#define EINA_RANGE_ZERO() EINA_RANGE(0, 0)
#define EINA_RANGE_EMPTY() ((Eina_Range) EINA_RANGE_ZERO())

/** @brief A Int range */
typedef struct _Eina_Range
{
   size_t start;
   size_t length;
} Eina_Range;


/**
 * @brief convenience macro for comparing two Eina_Range structs
 * @param[in] a An Eina_Range
 * @param[in] b An Eina_Range
 * @return 1 if the structs are equal, 0 if they are not
 * @since 1.24
 */
#define EINA_RANGE_EQ(a, b) \
  (((a).start == (b).start) && ((a).length == (b).length))

/**
 * @brief Get end value in range (not included in range).
 *
 * @param[in] range The Range.
 * @return The sum of end and length of the range.
 *
 * @since 1.24
 * */
static inline size_t eina_range_end(const Eina_Range *range);

/**
 * @brief Intersection between two ranges.
 *
 * @param[in] range The first range.
 * @param[in] range2 The second range.
 * @return The intersection between two ranges, If ranges do not intersect return length will be 0.
 *
 * @since 1.24
 * */
static inline Eina_Range eina_range_intersect(const Eina_Range *range, const Eina_Range *range2);


/**
 * @brief Union between two ranges.
 *
 * @param[in] range The first range.
 * @param[in] range2 The second range.
 * @return The union between two ranges.
 *
 * @since 1.24
 * */
static inline Eina_Range eina_range_union(const Eina_Range *range, const Eina_Range *range2);

/**
 * @brief Check if value is set in a range.
 *
 * @param[in] range The range.
 * @return Wither value set within range.
 *
 * @since 1.24
 * */
static inline Eina_Bool eina_range_contains(const Eina_Range *range, size_t value);

/**
 * @brief Check if two ranges are equal.
 *
 * @param[in] range The first range.
 * @param[in] range2 The second range.
 * @return Wither two ranges are equal.
 *
 * @since 1.24
 * */
static inline Eina_Bool eina_range_equal(const Eina_Range *range, const Eina_Range *range2);

/**
 * @brief Check if two ranges are equal.
 *
 * @param[in] min The min value in range.
 * @param[in] max The max value in range.
 * @return range.
 *
 * @since 1.24
 * */
static inline Eina_Range eina_range_from_to(size_t min, size_t max);

#include "eina_inline_range.x"

#endif /*EINA_RANGE_H_*/
