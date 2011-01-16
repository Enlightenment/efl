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

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#include "eina_bench.h"
#include "Eina.h"

static void
_eina_mempool_bench(Eina_Mempool *mp, int request)
{
   Eina_Array *array;
   int i;
   int j;

   eina_init();
   array = eina_array_new(32);

   for (i = 0; i < 100; ++i)
     {
        for (j = 0; j < request; ++j)
          {
             eina_array_push(array, eina_mempool_malloc(mp, sizeof (int)));
          }

        for (j = 0; j < request; ++j)
          {
             eina_mempool_free(mp, eina_array_pop(array));
          }
     }

   eina_array_free(array);
   eina_shutdown();
}

#ifdef EINA_BUILD_CHAINED_POOL
static void
eina_mempool_chained_mempool(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("chained_mempool", "test", NULL, sizeof (int), 256);
   _eina_mempool_bench(mp, request);
   eina_mempool_del(mp);
}
#endif

#ifdef EINA_BUILD_PASS_THROUGH
static void
eina_mempool_pass_through(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("pass_through", "test", NULL, sizeof (int), 8, 0);
   _eina_mempool_bench(mp, request);
   eina_mempool_del(mp);
}
#endif

#ifdef EINA_BUILD_FIXED_BITMAP
static void
eina_mempool_fixed_bitmap(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("fixed_bitmap", "test", NULL, sizeof (int));
   _eina_mempool_bench(mp, request);
   eina_mempool_del(mp);
}
#endif

#ifdef EINA_BUILD_EMEMOA_FIXED
static void
eina_mempool_ememoa_fixed(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("ememoa_fixed", "test", NULL, sizeof (int), 8, 0);
   _eina_mempool_bench(mp, request);
   eina_mempool_del(mp);
}
#endif

#ifdef EINA_BUILD_EMEMOA_UNKNOWN
static void
eina_mempool_ememoa_unknown(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("ememoa_unknown",
                         "test",
                         NULL,
                         0,
                         2,
                         sizeof (int),
                         8,
                         sizeof (int) * 2,
                         8);
   _eina_mempool_bench(mp, request);
   eina_mempool_del(mp);
}
#endif

#ifdef EINA_BENCH_HAVE_GLIB
static void
eina_mempool_glib(int request)
{
   Eina_Array *array;
   int i;
   int j;

   eina_init();
   array = eina_array_new(32);

   for (i = 0; i < 100; ++i)
     {
        for (j = 0; j < request; ++j)
          {
             eina_array_push(array, g_slice_alloc(sizeof (int)));
          }

        for (j = 0; j < request; ++j)
          {
             g_slice_free1(sizeof (int), eina_array_pop(array));
          }
     }

   eina_array_free(array);
   eina_shutdown();

}
#endif

void
eina_bench_mempool(Eina_Benchmark *bench)
{
#ifdef EINA_BUILD_CHAINED_POOL
   eina_benchmark_register(bench, "chained mempool",
                           EINA_BENCHMARK(
                              eina_mempool_chained_mempool), 10, 1000, 10);
#endif
#ifdef EINA_BUILD_PASS_THROUGH
   eina_benchmark_register(bench, "pass through",
                           EINA_BENCHMARK(
                              eina_mempool_pass_through),    10, 1000, 10);
#endif
#ifdef EINA_BUILD_FIXED_BITMAP
   eina_benchmark_register(bench, "fixed bitmap",
                           EINA_BENCHMARK(
                              eina_mempool_fixed_bitmap),    10, 1000, 10);
#endif
#ifdef EINA_BUILD_EMEMOA_FIXED
   eina_benchmark_register(bench, "ememoa fixed",
                           EINA_BENCHMARK(
                              eina_mempool_ememoa_fixed),    10, 1000, 10);
#endif
#ifdef EINA_BUILD_EMEMOA_UNKNOWN
   eina_benchmark_register(bench, "ememoa unknown",
                           EINA_BENCHMARK(
                              eina_mempool_ememoa_unknown),  10, 1000, 10);
#endif
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "gslice",
                           EINA_BENCHMARK(
                              eina_mempool_glib),            10, 1000, 10);
#endif
}
