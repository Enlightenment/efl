/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Cedric BAIL, Carsten Haitzler
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

EFL_START_TEST(eina_rectangle_pool)
{
   Eina_Rectangle_Pool *pool;
   Eina_Rectangle *rects[8][8];
   int x;
   int y;
   int w;
   int h;


   pool = eina_rectangle_pool_new(256, 256);
   fail_if(pool == NULL);

   eina_rectangle_pool_data_set(pool, rects);
   fail_if(eina_rectangle_pool_data_get(pool) != rects);

   fail_if(eina_rectangle_pool_request(pool, 1024, 1024) != NULL);

   for (x = 0; x < 8; x++)
      for (y = 0; y < 8; y++)
        {
           rects[x][y] = eina_rectangle_pool_request(pool, 32, 32);
           fail_if(rects[x][y] == NULL);
        }

   fail_if(eina_rectangle_pool_count(pool) != 64);

   fail_if(eina_rectangle_pool_get(rects[0][0]) != pool);

   fail_if(eina_rectangle_pool_geometry_get(pool, &w, &h) != EINA_TRUE);
   fail_if(w != 256 || h != 256);

   fail_if(eina_rectangle_pool_request(pool, 32, 32) != NULL);
   fail_if(eina_rectangle_pool_request(pool, 1024, 1024) != NULL);

   for (x = 0; x < 8; x++)
     eina_rectangle_pool_release(rects[0][x]);

   fail_if(eina_rectangle_pool_request(pool, 16, 16) == NULL);

   eina_rectangle_pool_free(pool);

}
EFL_END_TEST

EFL_START_TEST(eina_rectangle_pool_skyline)
{
   Eina_Rectangle_Pool *pool;
   Eina_Rectangle *rects[8][8];
   int x;
   int y;
   int w;
   int h;


   pool = eina_rectangle_pool_new(256, 256);
   fail_if(pool == NULL);

   eina_rectangle_pool_packing_set(pool, Eina_Packing_Bottom_Left_Skyline_Improved);

   eina_rectangle_pool_data_set(pool, rects);
   fail_if(eina_rectangle_pool_data_get(pool) != rects);

   fail_if(eina_rectangle_pool_request(pool, 1024, 1024) != NULL);

   for (x = 0; x < 8; x++)
      for (y = 0; y < 8; y++)
        {
           rects[x][y] = eina_rectangle_pool_request(pool, 32, 32);
           fail_if(rects[x][y] == NULL);
        }

   fail_if(eina_rectangle_pool_count(pool) != 64);

   fail_if(eina_rectangle_pool_get(rects[0][0]) != pool);

   fail_if(eina_rectangle_pool_geometry_get(pool, &w, &h) != EINA_TRUE);
   fail_if(w != 256 || h != 256);

   fail_if(eina_rectangle_pool_request(pool, 32, 32) != NULL);
   fail_if(eina_rectangle_pool_request(pool, 1024, 1024) != NULL);

   for (x = 0; x < 8; x++)
     eina_rectangle_pool_release(rects[0][x]);

   eina_rectangle_pool_free(pool);

}
EFL_END_TEST

EFL_START_TEST(eina_rectangle_union_intersect)
{
   Eina_Rectangle r1, r2, r3, r4, r5, r6, r7, r8, rd;


   EINA_RECTANGLE_SET(&r1, 10, 10, 50, 50);
   EINA_RECTANGLE_SET(&r2, 20, 20, 20, 20);
   EINA_RECTANGLE_SET(&r3, 0,   0, 10, 10);
   EINA_RECTANGLE_SET(&r4, 30, 30, 50, 50);
   EINA_RECTANGLE_SET(&r5, 10, 10, 0, 0);
   EINA_RECTANGLE_SET(&r6, 30, 30, 0, 0);
   EINA_RECTANGLE_SET(&r7, 10, 10, 5, 0);
   EINA_RECTANGLE_SET(&r8, 10, 10, 0, 5);


   rd = r1;

   fail_if(eina_rectangle_intersection(&rd, &r3));
   fail_if(!eina_rectangle_intersection(&rd, &r2));

   fail_if(rd.x != r2.x
           || rd.y != r2.y
           || rd.w != r2.w
           || rd.h != r2.h);

   rd = r1;

   fail_if(!eina_rectangle_intersection(&rd, &r4));

   fail_if(rd.x != 30
           || rd.y != 30
           || rd.w != 30
           || rd.h != 30);

   rd = r1;
   eina_rectangle_union(&rd, &r2);
   fail_if(rd.x != r1.x
           || rd.y != r1.y
           || rd.w != r1.w
           || rd.h != r1.h);

   rd = r6;
   fail_if(eina_rectangle_intersection(&rd, &r5));

   rd = r7;
   fail_if(eina_rectangle_intersection(&rd, &r3));

   rd = r8;
   fail_if(eina_rectangle_intersection(&rd, &r3));

   rd = r1;
   eina_rectangle_union(&rd, &r3);
   fail_if(rd.x != 0
           || rd.y != 0
           || rd.w != 60
           || rd.h != 60);

   rd = r3;
   eina_rectangle_union(&rd, &r4);
   fail_if(rd.x != 0
           || rd.y != 0
           || rd.w != 80
           || rd.h != 80);

   rd = r5;
   eina_rectangle_union(&rd, &r6);
   fail_if(rd.x != 10
           || rd.y != 10
           || rd.w != 20
           || rd.h != 20);

}
EFL_END_TEST

EFL_START_TEST(eina_rectangle_position_test)
{
    Eina_Rectangle middle, top, down, right, left;
    EINA_RECTANGLE_SET(&middle, -1, -1, 2.0, 2.0);
    EINA_RECTANGLE_SET(&top,    -1, -2, 2.0, 2.0);
    EINA_RECTANGLE_SET(&right,   0, -1, 2.0, 2.0);
    EINA_RECTANGLE_SET(&left,   -2, -1, 2.0, 2.0);
    EINA_RECTANGLE_SET(&down,   -1,  0, 2.0, 2.0);

    ck_assert_int_eq(eina_rectangle_outside_position(&middle, &top), EINA_RECTANGLE_OUTSIDE_TOP) ;
    ck_assert_int_eq(eina_rectangle_outside_position(&middle, &down), EINA_RECTANGLE_OUTSIDE_BOTTOM) ;
    ck_assert_int_eq(eina_rectangle_outside_position(&middle, &right), EINA_RECTANGLE_OUTSIDE_RIGHT) ;
    ck_assert_int_eq(eina_rectangle_outside_position(&middle, &left), EINA_RECTANGLE_OUTSIDE_LEFT) ;

}
EFL_END_TEST

void
eina_test_rectangle(TCase *tc)
{
   tcase_add_test(tc, eina_rectangle_pool);
   tcase_add_test(tc, eina_rectangle_pool_skyline);
   tcase_add_test(tc, eina_rectangle_union_intersect);
   tcase_add_test(tc, eina_rectangle_position_test);
}
