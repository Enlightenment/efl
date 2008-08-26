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
#include <time.h>
#include <stdio.h>

#include "eina_stringshare.h"
#include "eina_bench.h"
#include "eina_array.h"

static void
eina_bench_stringshare_job(int request)
{
   const char *tmp;
   Eina_Array *ea;
   Eina_Array_Iterator it;
   unsigned int j;
   int i;

   eina_stringshare_init();
   eina_array_init();

   ea = eina_array_new(request);

   for (i = 0; i < request; ++i)
     {
	char build[64];

	snprintf(build, 64, "string_%i", i);
	tmp = eina_stringshare_add(build);

	eina_array_push(ea, tmp);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char build[64];

	  snprintf(build, 64, "string_%i", rand() % request);
	  tmp = eina_stringshare_add(build);

	  eina_array_push(ea, tmp);
       }

   EINA_ARRAY_ITER_NEXT(ea, j, tmp, it)
     eina_stringshare_del(tmp);

   eina_array_shutdown();
   eina_stringshare_shutdown();
}

void eina_bench_stringshare(Eina_Bench *bench)
{
   eina_bench_register(bench, "stringshare", EINA_BENCH(eina_bench_stringshare_job), 100, 20100, 500);
}
