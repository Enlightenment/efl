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

#include "eina_suite.h"
#include "eina_benchmark.h"

#include <unistd.h>

static int global_test = 0;

static void
_eina_benchmark_specimens(int request)
{
   int i;
   int a = 0;

   for (i = 0; i < request; ++i)
     a += i;

   global_test = a;
}

START_TEST(eina_benchmark_simple)
{
   Eina_Benchmark *eb;
   Eina_Array_Iterator it;
   Eina_Array *ea;
   char *tmp;
   unsigned int i;

   eb = eina_benchmark_new("benchmark", "test");
   fail_if(!eb);

   eina_benchmark_register(eb, "specimens_check", EINA_BENCHMARK(_eina_benchmark_specimens), 1000, 1100, 100);

   ea = eina_benchmark_run(eb);
   fail_if(!ea);

   EINA_ARRAY_ITER_NEXT(ea, i, tmp, it)
     {
	fail_if(!tmp);
	fail_if(unlink(tmp));
     }

   fail_if(global_test != 499500);

   eina_array_free(ea);

   eina_benchmark_free(eb);
}
END_TEST

void
eina_test_benchmark(TCase *tc)
{
   tcase_add_test(tc, eina_benchmark_simple);
}
