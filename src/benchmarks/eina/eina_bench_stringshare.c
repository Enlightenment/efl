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

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#include "Evas_Data.h"
#include "Ecore_Data.h"

#include "eina_stringshare.h"
#include "eina_bench.h"
#include "eina_convert.h"
#include "eina_main.h"

static void
eina_bench_stringshare_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   eina_init();

   for (i = 0; i < request; ++i)
     {
        char build[64] = "string_";

        eina_convert_xtoa(i, build + 7);
        tmp = eina_stringshare_add(build);
     }

   for (j = 0; j < 200; ++j)
      for (i = 0; i < request; ++i)
        {
           char build[64] = "string_";

           eina_convert_xtoa(rand() % request, build + 7);
           tmp = eina_stringshare_add(build);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;
   eina_shutdown();
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

static void
eina_bench_evas_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   for (i = 0; i < request; ++i)
     {
        char build[64] = "string_";

        eina_convert_xtoa(i, build + 7);
        tmp = evas_stringshare_add(build);
     }

   for (j = 0; j < 200; ++j)
      for (i = 0; i < request; ++i)
        {
           char build[64] = "string_";

           eina_convert_xtoa(rand() % request, build + 7);
           tmp = evas_stringshare_add(build);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;
}

static void
eina_bench_ecore_job(int request)
{
   const char *tmp;
   unsigned int j;
   int i;

   //ecore_string_init();

   for (i = 0; i < request; ++i)
     {
        char build[64] = "string_";

        eina_convert_xtoa(i, build + 7);
        //tmp = ecore_string_instance(build);
     }

   for (j = 0; j < 200; ++j)
      for (i = 0; i < request; ++i)
        {
           char build[64] = "string_";

           eina_convert_xtoa(rand() % request, build + 7);
           //tmp = ecore_string_instance(build);
        }

   /* Suppress warnings as we really don't want to do anything. */
   (void) tmp;

   //ecore_string_shutdown();
}

void eina_bench_stringshare(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "stringshare",
                           EINA_BENCHMARK(
                              eina_bench_stringshare_job), 100, 20100, 500);
#ifdef EINA_BENCH_HAVE_GLIB
   eina_benchmark_register(bench, "stringchunk (glib)",
                           EINA_BENCHMARK(
                              eina_bench_stringchunk_job), 100, 20100, 500);
#endif
   eina_benchmark_register(bench, "stringshare (evas)",
                           EINA_BENCHMARK(
                              eina_bench_evas_job),        100, 20100, 500);
   eina_benchmark_register(bench, "stringshare (ecore)",
                           EINA_BENCHMARK(
                              eina_bench_ecore_job),       100, 20100, 500);
}
