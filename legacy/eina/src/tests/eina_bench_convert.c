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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

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

static void
eina_bench_convert_snprintf_a(int request)
{
   char tmp[128];
   double r;
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	r = 10000 * (rand() / ((double)RAND_MAX + 1));
	snprintf(tmp, 128, "%a", r);
	sscanf(tmp, "%la", &r);
     }
}

static void
eina_bench_convert_dtoa(int request)
{
   char tmp[128];
   long long m;
   long e;
   double r;
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	r = 10000 * (rand() / ((double)RAND_MAX + 1));
	eina_convert_dtoa(r, tmp);
	eina_convert_atod(tmp, 128, &m, &e);
	r = ldexp((double)m, e);
     }
}

#ifdef EINA_BENCH_HAVE_GLIB
static void
eina_bench_convert_gstrtod(int request)
{
   char tmp[128];
   double r;
   int i;

   srand(time(NULL));

   for (i = 0; i < request; ++i)
     {
	r = 10000 * (rand() / ((double)RAND_MAX + 1));
	g_ascii_dtostr(tmp, 128, r);
	r = g_ascii_strtod(tmp, NULL);
     }
}
#endif

void eina_bench_convert(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "itoa 10", EINA_BENCHMARK(eina_bench_convert_itoa_10), 1000, 200000, 500);
   eina_benchmark_register(bench, "itoa 16", EINA_BENCHMARK(eina_bench_convert_itoa_16), 1000, 200000, 500);
   eina_benchmark_register(bench, "snprintf 10", EINA_BENCHMARK(eina_bench_convert_snprintf_10), 1000, 200000, 500);
   eina_benchmark_register(bench, "snprintf 16", EINA_BENCHMARK(eina_bench_convert_snprintf_x), 1000, 200000, 500);
   eina_benchmark_register(bench, "snprintf a", EINA_BENCHMARK(eina_bench_convert_snprintf_a), 1000, 200000, 500);
   eina_benchmark_register(bench, "dtoa", EINA_BENCHMARK(eina_bench_convert_dtoa), 1000, 200000, 500);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "gstrtod", EINA_BENCHMARK(eina_bench_convert_gstrtod), 1000, 200000, 500);
#endif
}




