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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_bench.h"
#include "Eina.h"

static void
eina_bench_eina_rectangle_pool(int request)
{
   Eina_Rectangle_Pool *pool;
   Eina_Rectangle *rect;
   Eina_List *list = NULL;
   int i;

   eina_init();
   eina_init();

   pool = eina_rectangle_pool_new(2048, 2048);
   if (!pool)
      return;

   for (i = 0; i < request; ++i)
     {
        rect = NULL;

        while (!rect)
          {
             rect = eina_rectangle_pool_request(pool, i & 0xFF, 256 - (i & 0xFF));
             if (!rect)
               {
                  rect = eina_list_data_get(list);
                  list = eina_list_remove_list(list, list);
                  if (rect)
                     eina_rectangle_pool_release(rect);
               }
             else
                list = eina_list_append(list, rect);

             if (!(i & 0xFF))
                break;
          }
     }

   eina_rectangle_pool_free(pool);
   eina_list_free(list);

   eina_shutdown();
}

void eina_bench_rectangle_pool(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "eina",
                           EINA_BENCHMARK(
                              eina_bench_eina_rectangle_pool), 10, 4000, 100);
}


