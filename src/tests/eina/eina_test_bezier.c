/* EINA - EFL data type library
 * Copyright (C) 2015 Subhransu Mohanty <sub.mohanty@samsung.com>
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
#include <math.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_bezier_test_values)
{
   Eina_Bezier b;
   double sx, sy, cx1, cy1, cx2, cy2, ex, ey;

   eina_bezier_values_set(&b,
                          1, 2,
                          3, 4,
                          5, 6,
                          7, 8);
   eina_bezier_values_get(&b,
                          &sx, &sy,
                          &cx1, &cy1,
                          &cx2, &cy2,
                          &ex, &ey);
   fail_if(sx != 1 ||
           sy != 2 ||
           cx1 != 3 ||
           cy1 != 4 ||
           cx2 != 5 ||
           cy2 != 6 ||
           ex != 7 ||
           ey != 8);
}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_angle)
{
   Eina_Bezier b;
   double angle;

   eina_bezier_values_set(&b,
                          1, 1,
                          3, 1,
                          5, 1,
                          7, 1);
   angle = eina_bezier_angle_at(&b, 0.5);

   fail_if(angle != 0);

   eina_bezier_values_set(&b,
                          1, 2,
                          1, 4,
                          1, 6,
                          1, 8);
   angle = eina_bezier_angle_at(&b, 0.5);
   fail_if(floor(angle) != 90);

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_length)
{
   Eina_Bezier b;
   double length;

   eina_bezier_values_set(&b,
                          1, 1,
                          3, 1,
                          5, 1,
                          7, 1);
   length = eina_bezier_length_get(&b);
   fail_if(floor(length) != 6);

   eina_bezier_values_set(&b,
                          1, 1,
                          1, 1,
                          1, 1,
                          1, 1);
   length = eina_bezier_length_get(&b);
   fail_if(length != 0);

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_t_at)
{
   Eina_Bezier b;
   double length, t;

   eina_bezier_values_set(&b,
                          1, 1,
                          3, 1,
                          5, 1,
                          7, 1);
   length = eina_bezier_length_get(&b);
   t = eina_bezier_t_at(&b, 0);
   fail_if(floor(t) != 0);

   t = eina_bezier_t_at(&b, length);
   fail_if(t != 1);

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_point_at)
{
   Eina_Bezier b;
   double x, y;

   eina_bezier_values_set(&b,
                          1, 2,
                          3, 4,
                          5, 6,
                          7, 8);
   eina_bezier_point_at(&b, 0, &x , &y);
   fail_if(x != 1 ||
           y != 2);

   eina_bezier_point_at(&b, 1, &x , &y);

   fail_if(x != 7 ||
           y != 8);

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_split_at_length)
{
   Eina_Bezier b, l , r;
   double len, len1, len2;

   eina_bezier_values_set(&b,
                          1, 2,
                          3, 4,
                          5, 6,
                          7, 8);
   len = eina_bezier_length_get(&b);
   eina_bezier_split_at_length(&b, len/3, &l, &r);
   len1 = eina_bezier_length_get(&l);
   len2 = eina_bezier_length_get(&r);

   fail_if(len != (len1 + len2));

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_bounds_get)
{
   Eina_Bezier b;
   double x, y, w, h;

   eina_bezier_values_set(&b,
                          0, 0,
                          100, 0,
                          0, 100,
                          100, 100);

   eina_bezier_bounds_get(&b, &x, &y, &w, &h);

   fail_if(x !=0 || y!=0 || w !=100 || h !=100 );

}
EFL_END_TEST

EFL_START_TEST(eina_bezier_test_on_interval)
{
   Eina_Bezier b, res;
   double px1, px2, py1, py2;

   eina_bezier_values_set(&b,
                          0, 0,
                          10, 20,
                          20, 90,
                          30, 0);
   eina_bezier_point_at(&b, 0.25, &px1, &py1);
   eina_bezier_on_interval(&b, 0.25, 0.5, &res);
   eina_bezier_point_at(&res, 0, &px2, &py2);

   fail_if(px1 != px2 || py1 != py2);

}
EFL_END_TEST

void
eina_test_bezier(TCase *tc)
{
   tcase_add_test(tc, eina_bezier_test_values);
   tcase_add_test(tc, eina_bezier_test_angle);
   tcase_add_test(tc, eina_bezier_test_length);
   tcase_add_test(tc, eina_bezier_test_t_at);
   tcase_add_test(tc, eina_bezier_test_point_at);
   tcase_add_test(tc, eina_bezier_test_split_at_length);
   tcase_add_test(tc, eina_bezier_test_bounds_get);
   tcase_add_test(tc, eina_bezier_test_on_interval);
}
