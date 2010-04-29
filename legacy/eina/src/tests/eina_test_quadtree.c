/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric Bail
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

#include "eina_suite.h"
#include "Eina.h"

static Eina_Quad_Direction
_eina_quadtree_rectangle_vert(const void *object, size_t middle)
{
   const Eina_Rectangle *r = object;

   if (r->y + r->h < (int) middle)
     return EINA_QUAD_LEFT;
   if (r->y > (int) middle)
     return EINA_QUAD_RIGHT;
   return EINA_QUAD_BOTH;
}

static Eina_Quad_Direction
_eina_quadtree_rectangle_hort(const void *object, size_t middle)
{
   const Eina_Rectangle *r = object;

   if (r->x + r->w < (int) middle)
     return EINA_QUAD_LEFT;
   if (r->x > (int) middle)
     return EINA_QUAD_RIGHT;
   return EINA_QUAD_BOTH;
}

START_TEST(eina_quadtree_collision)
{
   struct {
      Eina_Rectangle r;
      Eina_QuadTree_Item *item;
   } objects[] = {
     { { 10, 10, 30, 30 }, NULL },
     { { 20, 20, 30, 30 }, NULL },
     { { 5, 30, 30, 30 }, NULL },
     { { 70, 130, 100, 100 }, NULL },
     { { 10, 220, 50, 40 }, NULL },
     { { 310, 20, 50, 30 }, NULL },
     { { 300, 220, 40, 40 }, NULL },
     { { 500, 150, 40, 40 }, NULL },
     { { 500, 220, 40, 40 }, NULL },
     { { 330, 250, 40, 40 }, NULL },
     { { 300, 400, 40, 40 }, NULL },
     { { 10, 400, 40, 40 }, NULL },
     { { 0, 0, 0, 0 }, NULL }
     };
   struct {
      Eina_Rectangle r;
      int count;
      int result[20];
   } tests [] = {
     { { 600, 400, 40, 40 }, 4, { 4, 6, 8, 10 } },
     { { 20, 30, 10, 10 }, 7, { 0, 1, 2, 4, 5, 6, 8 } },
     { { 0, 0, 0, 0 }, -1, {} },
   };
   int hidden[] = { 4, 5, 6, 8, 10 };
   int show[] = { 0, 1, 2 };
   Eina_QuadTree *q;
   Eina_Array *result;
   Eina_Rectangle *r;
   Eina_Array_Iterator it;
   unsigned int j;
   int i;

   fail_if(!eina_init());

   q = eina_quadtree_new(640, 480,
			 _eina_quadtree_rectangle_vert,
			 _eina_quadtree_rectangle_hort);

   fail_if(!q);

   for (i = 0; objects[i].r.w != 0 && objects[i].r.h != 0; ++i)
     {
	objects[i].item = eina_quadtree_add(q, &objects[i].r);
	fail_if(!objects[i].item);
	fail_if(!eina_quadtree_show(objects[i].item));
     }

   result = eina_array_new(16);
   fail_if(!result);

   for (i = 0; tests[i].count != -1; ++i)
     {
	eina_quadtree_collide(result, q,
			      tests[i].r.x, tests[i].r.y, tests[i].r.w, tests[i].r.h);
	fail_if(eina_array_count_get(result) != (unsigned int) tests[i].count);

	EINA_ARRAY_ITER_NEXT(result, j, r, it)
	  {
	     int k;

	     for (k = 0; k < tests[i].count; ++k)
	       {
	     	  if (&objects[tests[i].result[k]].r == r)
	     	    break;
	       }
	     fail_if(k == tests[i].count);
	  }

	eina_array_clean(result);
     }

   for (j = 0; j < sizeof (hidden) / sizeof (int); ++j)
     eina_quadtree_hide(objects[hidden[j]].item);
   for (j = 0; j < sizeof (show) / sizeof (int); ++j)
     eina_quadtree_show(objects[show[j]].item);

   eina_quadtree_collide(result, q,
			 tests[1].r.x, tests[1].r.y, tests[1].r.w, tests[1].r.h);
   fail_if(eina_array_count_get(result) != 3);

   EINA_ARRAY_ITER_NEXT(result, j, r, it)
     fail_if(r != &objects[tests[1].result[show[j]]].r);

   eina_array_clean(result);

   eina_quadtree_show(objects[4].item);
   eina_quadtree_show(objects[5].item);
   eina_quadtree_del(objects[5].item);
   eina_quadtree_change(objects[10].item);

   eina_quadtree_collide(result, q,
			 tests[0].r.x, tests[0].r.y, tests[0].r.w, tests[0].r.h);
   fail_if(eina_array_count_get(result) != 1);

   eina_quadtree_free(q);

   eina_shutdown();
}
END_TEST

void
eina_test_quadtree(TCase *tc)
{
   tcase_add_test(tc, eina_quadtree_collision);
}
