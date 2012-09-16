/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric BAIL
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

#define WIDTH 720
#define HEIGHT 576

#include <stdlib.h>

#include "eina_main.h"
#include "eina_mempool.h"
#include "eina_rectangle.h"
#include "eina_quadtree.h"
#include "eina_list.h"
#include "eina_bench.h"

static void
eina_bench_render_loop(int request)
{
   Eina_List *objects = NULL;
   Eina_Rectangle *r;
   int i;
   int j;

   eina_init();

   for (i = 0; i < request; ++i)
      objects = eina_list_append(objects,
                                 eina_rectangle_new((rand() * WIDTH) / RAND_MAX,
                                                    (rand() *
                                                     HEIGHT) / RAND_MAX,
                                                    (rand() * WIDTH /
                                                     2) / RAND_MAX,
                                                    (rand() * HEIGHT /
                                                     2) / RAND_MAX));

   for (j = 0; j < 100; ++j)
     {
        Eina_Rectangle *collide;
        Eina_List *collided = NULL;
        Eina_List *changed = NULL;
        Eina_List *l;

        /* Delete 25% of all objects */
        i = request * 25 / 100;
        for (; i > 0; --i)
          {
             eina_rectangle_free(eina_list_data_get(objects));
             objects = eina_list_remove_list(objects, objects);
          }

        /* Add them back */
        i = request * 25 / 100;
        for (; i > 0; --i)
          {
             r = eina_rectangle_new((rand() * WIDTH) / RAND_MAX,
                                    (rand() * HEIGHT) / RAND_MAX,
                                    (rand() * WIDTH / 3) / RAND_MAX,
                                    (rand() * HEIGHT / 3) / RAND_MAX);
             objects = eina_list_prepend(objects, r);
             changed = eina_list_append(changed, r);
          }

        /* Do one collide search */
        collide = eina_rectangle_new((rand() * WIDTH) / RAND_MAX,
                                     (rand() * HEIGHT) / RAND_MAX,
                                     (rand() * WIDTH / 4) / RAND_MAX,
                                     (rand() * HEIGHT / 4) / RAND_MAX);
        EINA_LIST_FOREACH(objects, l, r)
        if (eina_rectangles_intersect(r, collide))
           collided = eina_list_append(collided, r);

        collided = eina_list_free(collided);
        eina_rectangle_free(collide);

        /* Modify 50% of all objects */
        i = request * 50 / 100;
        for (; i > 0; --i)
          {
             r = eina_list_data_get(eina_list_last(objects));
             objects = eina_list_remove_list(objects, eina_list_last(objects));

             r->x = (rand() * WIDTH) / RAND_MAX;
             r->y = (rand() * HEIGHT) / RAND_MAX;
             r->w = (rand() * WIDTH / 3) / RAND_MAX;
             r->h = (rand() * HEIGHT / 3) / RAND_MAX;

             objects = eina_list_prepend(objects, r);
             changed = eina_list_append(changed, r);
          }

        /* Emulating the render loop by colliding all modified
           object with all intersecting object */
        EINA_LIST_FREE(changed, r)
        {
           EINA_LIST_FOREACH(objects, l, collide)
           if (r != collide && eina_rectangles_intersect(collide, r))
              collided = eina_list_append(collided, collide);

           collided = eina_list_append(collided, r);
        }

        /* Ok, we compute it, now it's done */
        collided = eina_list_free(collided);
     }

   EINA_LIST_FREE(objects, r)
   eina_rectangle_free(r);

   eina_shutdown();
}

typedef struct _Eina_Bench_Quad Eina_Bench_Quad;
struct _Eina_Bench_Quad
{
   Eina_Rectangle r;
   Eina_QuadTree_Item *item;
};

static Eina_Quad_Direction
_eina_bench_quadtree_vertical(const void *object, size_t middle)
{
   const Eina_Bench_Quad *b = object;
   size_t y;

   y = b->r.y < 0 ? 0 : (size_t)b->r.y;

   if (y + b->r.h < middle)
      return EINA_QUAD_LEFT;

   if (y > middle)
      return EINA_QUAD_RIGHT;

   return EINA_QUAD_BOTH;
}

static Eina_Quad_Direction
_eina_bench_quadtree_horizontal(const void *object, size_t middle)
{
   const Eina_Bench_Quad *b = object;
   size_t x;

   x = b->r.x < 0 ? 0 : (size_t)b->r.x;

   if (x + b->r.w < middle)
      return EINA_QUAD_LEFT;

   if (x > middle)
      return EINA_QUAD_RIGHT;

   return EINA_QUAD_BOTH;
}

static void
eina_bench_quadtree_render_loop(int request)
{
   Eina_List *objects = NULL;
   Eina_Inlist *possibility;
   Eina_Bench_Quad *b;
   Eina_QuadTree *q;
   Eina_Mempool *mp;
   int i;
   int j;

   eina_init();

   mp = eina_mempool_add("chained_mempool", "bench-quad", NULL,
                         sizeof (Eina_Bench_Quad), 320);

   q = eina_quadtree_new(WIDTH, HEIGHT,
                         _eina_bench_quadtree_vertical,
                         _eina_bench_quadtree_horizontal);

   /* Create requested object */
   for (i = 0; i < request; ++i)
     {
        b = eina_mempool_malloc(mp, sizeof (Eina_Bench_Quad));
        EINA_RECTANGLE_SET(&b->r,
                           (rand() * WIDTH) / RAND_MAX,
                           (rand() * HEIGHT) / RAND_MAX,
                           (rand() * WIDTH / 2) / RAND_MAX,
                           (rand() * HEIGHT / 2) / RAND_MAX);
        b->item = eina_quadtree_add(q, b);

        objects = eina_list_append(objects, b);
     }

   for (j = 0; j < 100; ++j)
     {
        Eina_Bench_Quad *collide;
        Eina_List *changed = NULL;
        Eina_List *collided = NULL;

        /* Delete 25% of all objects */
        i = request * 25 / 100;
        for (; i > 0; --i)
          {
             b = eina_list_data_get(objects);
             eina_quadtree_del(b->item);
             eina_mempool_free(mp, b);

             objects = eina_list_remove_list(objects, objects);
          }

        /* Add them back */
        i = request * 25 / 100;
        for (; i > 0; --i)
          {
             b = eina_mempool_malloc(mp, sizeof (Eina_Bench_Quad));
             EINA_RECTANGLE_SET(&b->r,
                                (rand() * WIDTH) / RAND_MAX,
                                (rand() * HEIGHT) / RAND_MAX,
                                (rand() * WIDTH / 3) / RAND_MAX,
                                (rand() * HEIGHT / 3) / RAND_MAX);
             b->item = eina_quadtree_add(q, b);

             objects = eina_list_prepend(objects, b);
             changed = eina_list_append(changed, b);
          }

        /* Do one collide search */
        collide = eina_mempool_malloc(mp, sizeof (Eina_Bench_Quad));
             EINA_RECTANGLE_SET(&collide->r,
                           (rand() * WIDTH) / RAND_MAX,
                           (rand() * HEIGHT) / RAND_MAX,
                           (rand() * WIDTH / 4) / RAND_MAX,
                           (rand() * HEIGHT / 4) / RAND_MAX);
        possibility = eina_quadtree_collide(q,
                                            collide->r.x, collide->r.y,
                                            collide->r.w, collide->r.h);
        while (possibility)
          {
             b = eina_quadtree_object(possibility);
             possibility = possibility->next;

             if (eina_rectangles_intersect(&b->r, &collide->r))
                collided = eina_list_append(collided, b);
          }

        collided = eina_list_free(collided);
        eina_mempool_free(mp, collide);

        /* Modify 50% of all objects */
        i = request * 50 / 100;
        for (; i > 0; --i)
          {
             b = eina_list_data_get(eina_list_last(objects));
             objects = eina_list_remove_list(objects, eina_list_last(objects));

             b->r.x = (rand() * WIDTH) / RAND_MAX;
             b->r.y = (rand() * HEIGHT) / RAND_MAX;
             b->r.w = (rand() * WIDTH / 3) / RAND_MAX;
             b->r.h = (rand() * HEIGHT / 3) / RAND_MAX;

             eina_quadtree_change(b->item);

             objects = eina_list_prepend(objects, b);
             changed = eina_list_append(changed, b);
          }

        /* Emulating the render loop by colliding all modified
           object with all intersecting object */
        EINA_LIST_FREE(changed, b)
        {
           possibility = eina_quadtree_collide(q,
                                               b->r.x, b->r.y, b->r.w, b->r.h);
           while (possibility)
             {
                collide = eina_quadtree_object(possibility);
                possibility = possibility->next;

                if (collide != b &&
                    eina_rectangles_intersect(&b->r, &collide->r))
                   collided = eina_list_append(collided, collide);
             }

           collided = eina_list_append(collided, b);
        }

        /* Ok, we compute it, now it's done */
        collided = eina_list_free(collided);
     }

   EINA_LIST_FREE(objects, b)
   {
      eina_quadtree_del(b->item);
      eina_mempool_free(mp, b);
   }

   eina_mempool_del(mp);

   eina_quadtree_free(q);

   eina_shutdown();
}

void
eina_bench_quadtree(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "collide-all",
                           EINA_BENCHMARK(eina_bench_render_loop),
                           100, 1500, 50);
   eina_benchmark_register(bench, "collide-quad-tree",
                           EINA_BENCHMARK(eina_bench_quadtree_render_loop),
                           100, 1500, 50);
}
