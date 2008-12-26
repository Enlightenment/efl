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

#include "eina_rectangle.h"
#include "eina_suite.h"

START_TEST(eina_rectangle_pool)
{
   Eina_Rectangle_Pool *pool;
   Eina_Rectangle *rects[8][8];
   int x;
   int y;
   int w;
   int h;

   pool = eina_rectangle_pool_add(256, 256);
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

   eina_rectangle_pool_delete(pool);
}
END_TEST

void
eina_test_rectangle(TCase *tc)
{
   tcase_add_test(tc, eina_rectangle_pool);
}

