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
#include <time.h>
#include <stdio.h>

#ifdef EINA_BENCH_HAVE_GLIB
#include <glib.h>
#endif

#ifdef EINA_BENCH_HAVE_EVAS
#include <Evas.h>
#endif

#ifdef EINA_BENCH_HAVE_ECORE
#include <Ecore_Data.h>
#endif

#include "eina_stringshare.h"
#include "eina_bench.h"
#include "eina_convert.h"

static void
eina_bench_stringshare_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_stringshare_init();

   for (i = 0; i < request; ++i)
     {
	char build[64] = "string_";

	eina_convert_xtoa(i, build + 7);
	tmp = eina_stringshare_add(build);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char build[64] = "string_";

	  eina_convert_xtoa(rand() % request, build + 7);
	  tmp = eina_stringshare_add(build);
       }

   eina_stringshare_shutdown();
}

#ifdef EINA_BENCH_HAVE_GLIB
static void
eina_bench_stringchunk_job(int request)
{
   GStringChunk *chunk;
   unsigned int j;
   int i;

   chunk = g_string_chunk_new(4096);

   for (i = 0; i < request; ++i)
     {
	char build[64] = "string_";

	eina_convert_xtoa(i, build + 7);
	g_string_chunk_insert_const(chunk, build);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char build[64] = "string_";

	  eina_convert_xtoa(rand() % request, build + 7);
	  g_string_chunk_insert_const(chunk, build);
       }

   g_string_chunk_free(chunk);
}
#endif

#ifdef EINA_BENCH_HAVE_EVAS
static void
eina_bench_evas_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   evas_init();
/*    evas_stringshare_init(); */

   for (i = 0; i < request; ++i)
     {
	char build[64] = "string_";

	eina_convert_xtoa(i, build + 7);
	tmp = evas_stringshare_add(build);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char build[64] = "string_";

	  eina_convert_xtoa(rand() % request, build + 7);
	  tmp = evas_stringshare_add(build);
       }

/*    evas_stringshare_shutdown(); */
   evas_shutdown();
}
#endif

#ifdef EINA_BENCH_HAVE_ECORE
static void
eina_bench_ecore_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   ecore_string_init();

   for (i = 0; i < request; ++i)
     {
	char build[64] = "string_";

	eina_convert_xtoa(i, build + 7);
	tmp = ecore_string_instance(build);
     }

   srand(time(NULL));

   for (j = 0; j < 200; ++j)
     for (i = 0; i < request; ++i)
       {
	  char build[64] = "string_";

	  eina_convert_xtoa(rand() % request, build + 7);
	  tmp = ecore_string_instance(build);
       }

   ecore_string_shutdown();
}
#endif

void eina_bench_stringshare(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "stringshare", EINA_BENCHMARK(eina_bench_stringshare_job), 100, 20100, 500);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "stringchunk (glib)", EINA_BENCHMARK(eina_bench_stringchunk_job), 100, 20100, 500);
#endif
#ifdef EINA_BENCH_HAVE_EVAS
   eina_benchmark_register(bench, "stringshare (evas)", EINA_BENCHMARK(eina_bench_evas_job), 100, 20100, 500);
#endif
#ifdef EINA_BENCH_HAVE_ECORE
   eina_benchmark_register(bench, "stringshare (ecore)", EINA_BENCHMARK(eina_bench_ecore_job), 100, 20100, 500);
#endif
}
