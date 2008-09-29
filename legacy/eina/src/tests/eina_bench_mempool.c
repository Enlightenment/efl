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
#include "eina_array.h"
#include "eina_mempool.h"

static void
_eina_mempool_bench(Eina_Mempool *mp, int request)
{
   Eina_Array *array;
   int i;
   int j;

   eina_array_init();
   array = eina_array_new(32);

   for (i = 0; i < 100; ++i)
     {
	for (j = 0; j < request; ++j)
	  {
	     eina_array_push(array, eina_mempool_alloc(mp, sizeof (int)));
	  }

	for (j = 0; j < request; ++j)
	  {
	     eina_mempool_free(mp, eina_array_pop(array));
	  }
     }

   eina_array_free(array);
   eina_array_shutdown();
}

static void
eina_mempool_chained_mempool(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_new("chained_mempool", "test", NULL, sizeof (int), 256);
   _eina_mempool_bench(mp, request);
   eina_mempool_delete(mp);
}

static void
eina_mempool_pass_through(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_new("pass_through", "test", NULL, sizeof (int), 8, 0);
   _eina_mempool_bench(mp, request);
   eina_mempool_delete(mp);
}

static void
eina_mempool_fixed_bitmap(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_new("fixed_bitmap", "test", NULL, sizeof (int));
   _eina_mempool_bench(mp, request);
   eina_mempool_delete(mp);
}

#ifdef EINA_EMEMOA_SUPPORT
static void
eina_mempool_ememoa_fixed(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_new("ememoa_fixed", "test", NULL, sizeof (int), 8, 0);
   _eina_mempool_bench(mp, request);
   eina_mempool_delete(mp);
}

static void
eina_mempool_ememoa_unknown(int request)
{
   Eina_Mempool *mp;

   mp = eina_mempool_new("ememoa_unknown", "test", NULL, 0, 2, sizeof (int), 8, sizeof (int) * 2, 8);
   _eina_mempool_bench(mp, request);
   eina_mempool_delete(mp);
}
#endif

void
eina_bench_mempool(Eina_Benchmark *bench)
{
/*    eina_benchmark_register(bench, "chained mempool", EINA_BENCHMARK(eina_mempool_chained_mempool), 10, 100, 10); */
   eina_benchmark_register(bench, "pass through", EINA_BENCHMARK(eina_mempool_pass_through), 10, 10000, 100);
   eina_benchmark_register(bench, "fixed bitmap", EINA_BENCHMARK(eina_mempool_fixed_bitmap), 10, 10000, 100);
#ifdef EINA_EMEMOA_SUPPORT
   eina_benchmark_register(bench, "ememoa fixed", EINA_BENCHMARK(eina_mempool_ememoa_fixed), 10, 10000, 100);
   eina_benchmark_register(bench, "ememoa unknown", EINA_BENCHMARK(eina_mempool_ememoa_unknown), 10, 10000, 100);
#endif
}
