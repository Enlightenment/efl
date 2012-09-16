/* EINA - EFL data type library
 * Copyright (C) 2009 Rafael Antognolli
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

#include "eina_suite.h"
#include "Eina.h"

struct test_rect
{
   unsigned long col, row;
   int x, y, w, h;
   Eina_Bool full;
};

static void
check_iterator(Eina_Iterator *it, struct test_rect *cur_test)
{
   unsigned int i = 0;
   struct Eina_Tile_Grid_Info *tile;

      EINA_ITERATOR_FOREACH(it, tile) {
      fail_if(cur_test[i].col != tile->col ||
              cur_test[i].row != tile->row ||
              cur_test[i].x != tile->rect.x ||
              cur_test[i].y != tile->rect.y ||
              cur_test[i].w != tile->rect.w ||
              cur_test[i].h != tile->rect.h ||
              cur_test[i].full != tile->full);
      i++;
   }

      fail_if(i == 0);
}

      START_TEST(eina_test_tile_grid_slicer_iterator)
{
   Eina_Iterator *it;
   struct test_rect *cur_test;
   struct test_rect test1[] = {{1, 1, 72, 82, 10, 15, 0}};
   struct test_rect test2[] =
   {{1, 1,  72,  82,  56,  15,  0},
    {2, 1,   0,  82, 128,  15,  0},
    {3, 1,   0,  82, 116,  15,  0}};
   struct test_rect test3[] =
   {{1, 1,  72,  82,  10,  46,  0},
    {1, 2,  72,   0,  10, 128,  0},
    {1, 3,  72,   0,  10, 126,  0}};
   struct test_rect test4[] =
   {{1, 1,  72,  82,  56,  46,  0},
    {2, 1,   0,  82, 128,  46,  0},
    {3, 1,   0,  82, 128,  46,  0},
    {4, 1,   0,  82,  88,  46,  0},
    {1, 2,  72,   0,  56, 128,  0},
    {2, 2,   0,   0, 128, 128,  1},
    {3, 2,   0,   0, 128, 128,  1},
    {4, 2,   0,   0,  88, 128,  0},
    {1, 3,  72,   0,  56, 126,  0},
    {2, 3,   0,   0, 128, 126,  0},
    {3, 3,   0,   0, 128, 126,  0},
    {4, 3,   0,   0,  88, 126,  0}};
   struct test_rect test5[] = {{1, 1, 0, 0, 128, 128, 1}};
   struct test_rect test6[] = {{1, 1, 0, 0, 1, 1, 0}};
   struct test_rect test7[] =
   {{1, 1,   0,   0, 128, 128,  1},
    {2, 1,   0,   0,   1, 128,  0},
    {1, 2,   0,   0, 128,   1,  0},
    {2, 2,   0,   0,   1,   1,  0}};

   eina_init();

   cur_test = test1;
   it = eina_tile_grid_slicer_iterator_new(200, 210, 10, 15, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test2;
   it = eina_tile_grid_slicer_iterator_new(200, 210, 300, 15, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test3;
   it = eina_tile_grid_slicer_iterator_new(200, 210, 10, 300, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test4;
   it = eina_tile_grid_slicer_iterator_new(200, 210, 400, 300, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test5;
   it = eina_tile_grid_slicer_iterator_new(128, 128, 128, 128, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test6;
   it = eina_tile_grid_slicer_iterator_new(128, 128, 1, 1, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   cur_test = test7;
   it = eina_tile_grid_slicer_iterator_new(128, 128, 129, 129, 128, 128);
   check_iterator(it, cur_test);
   eina_iterator_free(it);

   eina_shutdown();
}
END_TEST

START_TEST(eina_test_tiler_all)
{
   Eina_Tiler *tl;
   Eina_Iterator *it;
   Eina_Rectangle *rp;
   Eina_Rectangle r;
   int i = 0;

   eina_init();

   tl = eina_tiler_new(640, 480);

   eina_tiler_tile_size_set(tl, 32, 32);

   EINA_RECTANGLE_SET(&r, 50, 50, 20, 20);
   fail_if(!eina_tiler_rect_add(tl, &r));

   EINA_RECTANGLE_SET(&r, -10, -10, 5, 5);
   fail_if(eina_tiler_rect_add(tl, &r));

   EINA_RECTANGLE_SET(&r, 40, 40, 20, 20);
   eina_tiler_rect_del(tl, &r);

   it = eina_tiler_iterator_new(tl);
   fail_if(!it);

   EINA_ITERATOR_FOREACH(it, rp)
   {
      fail_if(rp->w <= 0);
      fail_if(rp->h <= 0);
      fail_if(rp->x < 0 || rp->x + rp->w > 640);
      fail_if(rp->y < 0 || rp->y + rp->h > 480);
      ++i;
   }

      fail_if(eina_iterator_container_get(it) != tl);

      eina_iterator_free(it);

   fail_if(i == 0);

   eina_tiler_clear(tl);

   eina_tiler_free(tl);

   eina_shutdown();
}
END_TEST

void
eina_test_tiler(TCase *tc)
{
   tcase_add_test(tc, eina_test_tile_grid_slicer_iterator);
   tcase_add_test(tc, eina_test_tiler_all);
}
