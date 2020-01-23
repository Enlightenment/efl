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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_range_intersect_union_test)
{
   Eina_Range r1 = EINA_RANGE(0, 10);
   Eina_Range r2 = EINA_RANGE(5, 15);

   Eina_Range r_intersect = eina_range_intersect(&r1, &r2);
   ck_assert_uint_eq(r_intersect.start, 5);
   ck_assert_uint_eq(r_intersect.length, 5);

   Eina_Range r_union = eina_range_union(&r1, &r2);
   ck_assert_uint_eq(r_union.start, 0);
   ck_assert_uint_eq(r_union.length, 20);
}
EFL_END_TEST

EFL_START_TEST(eina_range_contains_test)
{
   Eina_Range r1 = EINA_RANGE(0, 10);

   ck_assert(eina_range_contains(&r1,0));
   ck_assert(eina_range_contains(&r1,9));
   ck_assert(!eina_range_contains(&r1,10));
}
EFL_END_TEST

EFL_START_TEST(eina_range_equal_test)
{
   Eina_Range r1 = EINA_RANGE(0, 10);
   Eina_Range r2 = EINA_RANGE(0, 10);
   Eina_Range r3 = EINA_RANGE(0, 9);

   ck_assert(eina_range_equal(&r1, &r2));
   ck_assert(!eina_range_equal(&r1, &r3));
}
EFL_END_TEST


void
eina_test_range(TCase *tc)
{
   tcase_add_test(tc, eina_range_intersect_union_test);
   tcase_add_test(tc, eina_range_contains_test);
   tcase_add_test(tc, eina_range_equal_test);
}
