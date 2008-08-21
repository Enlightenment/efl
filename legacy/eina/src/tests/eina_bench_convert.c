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

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "eina_bench.h"
#include "eina_convert.h"

static void
eina_bench_convert_itoa_10(int request)
{
   char tmp[128];
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	eina_convert_itoa(rand(), tmp);
     }
}

static void
eina_bench_convert_itoa_16(int request)
{
   char tmp[128];
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	eina_convert_xtoa(rand(), tmp);
     }
}

static void
eina_bench_convert_snprintf_10(int request)
{
   char tmp[128];
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	snprintf(tmp, 128, "%i", rand());
     }
}

static void
eina_bench_convert_snprintf_x(int request)
{
   char tmp[128];
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	snprintf(tmp, 128, "%x", rand());
     }
}

void eina_bench_convert(Eina_Bench *bench)
{
   eina_bench_register(bench, "itoa 10", EINA_BENCH(eina_bench_convert_itoa_10), 1000, 200000, 500);
   eina_bench_register(bench, "itoa 16", EINA_BENCH(eina_bench_convert_itoa_16), 1000, 200000, 500);
   eina_bench_register(bench, "snprintf 10", EINA_BENCH(eina_bench_convert_snprintf_10), 1000, 200000, 500);
   eina_bench_register(bench, "snprintf 16", EINA_BENCH(eina_bench_convert_snprintf_x), 1000, 200000, 500);
}




