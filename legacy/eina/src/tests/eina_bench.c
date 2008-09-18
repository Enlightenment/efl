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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "eina_bench.h"

typedef struct _Eina_Benchmark_Case Eina_Benchmark_Case;
struct _Eina_Benchmark_Case
{
   const char *bench_case;
   void (*build)(Eina_Benchmark *bench);
};

static const Eina_Benchmark_Case etc[] = {
  { "Hash", eina_bench_hash },
  { "Array vs List vs Inlist", eina_bench_array },
  { "Stringshare", eina_bench_stringshare },
  { "Convert", eina_bench_convert },
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   Eina_Benchmark *test;
   Eina_Array *ea;
   unsigned int i;

   if (argc != 2) return -1;

   eina_benchmark_init();

   for (i = 0; etc[i].bench_case != NULL; ++i)
     {
	test = eina_benchmark_new(etc[i].bench_case, argv[1]);
	if (!test) continue ;

	etc[i].build(test);

	ea = eina_benchmark_run(test);
	if (ea)
	  {
	     Eina_Array_Iterator it;
	     char *tmp;
	     unsigned int i;

	     EINA_ARRAY_ITER_NEXT(ea, i, tmp, it)
	       free(tmp);

	     eina_array_free(ea);
	  }

	eina_benchmark_free(test);
     }

   eina_bench_e17();

   eina_benchmark_shutdown();

   return 0;
}
