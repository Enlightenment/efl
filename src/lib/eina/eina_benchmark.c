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
#include <string.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"
#include "eina_log.h"
#include "eina_benchmark.h"
#include "eina_inlist.h"
#include "eina_list.h"
#include "eina_counter.h"
#include "eina_strbuf.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_BENCHMARK_FILENAME_MASK "bench_%s_%s.gnuplot"
#define EINA_BENCHMARK_DATA_MASK "bench_%s_%s.%s.data"

typedef struct _Eina_Run Eina_Run;
struct _Eina_Run
{
   EINA_INLIST;

   Eina_Benchmark_Specimens cb;
   const char *name;
   int start;
   int end;
   int step;
};

struct _Eina_Benchmark
{
   const char *name;
   const char *run;

   Eina_Inlist *runs;
   Eina_List *names;
};

static int _eina_benchmark_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_benchmark_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_benchmark_log_dom, __VA_ARGS__)

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the benchmark module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the benchmark module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_benchmark_init(void)
{
   _eina_benchmark_log_dom = eina_log_domain_register("eina_benchmark",
                                                      EINA_LOG_COLOR_DEFAULT);
   if (_eina_benchmark_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_benchmark");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the benchmark module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the benchmark module set up by
 * eina_benchmark_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_benchmark_shutdown(void)
{
   eina_log_domain_unregister(_eina_benchmark_log_dom);
   _eina_benchmark_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI Eina_Benchmark *
eina_benchmark_new(const char *name, const char *run)
{
   Eina_Benchmark *new;

   new = calloc(1, sizeof (Eina_Benchmark));
   if (!new) return NULL;

   new->name = name;
   new->run = run;

   return new;
}

EAPI void
eina_benchmark_free(Eina_Benchmark *bench)
{
   Eina_Array *names;

   if (!bench)
      return;

   while (bench->runs)
     {
        Eina_Run *run = (Eina_Run *)bench->runs;

        bench->runs = eina_inlist_remove(bench->runs, bench->runs);
        free(run);
     }

   EINA_LIST_FREE(bench->names, names)
   {
      Eina_Array_Iterator it;
      char *tmp;
      unsigned int i;

      EINA_ARRAY_ITER_NEXT(names, i, tmp, it)
	free(tmp);

      eina_array_free(names);
   }

   free(bench);
}

EAPI Eina_Bool
eina_benchmark_register(Eina_Benchmark *bench,
                        const char *name,
                        Eina_Benchmark_Specimens bench_cb,
                        int count_start,
                        int count_end,
                        int count_step)
{
   Eina_Run *run;

   if (!bench)
      return EINA_FALSE;

   if (count_step == 0)
      return EINA_FALSE;

   run = calloc(1, sizeof (Eina_Run));
   if (!run) return EINA_FALSE;

   run->cb = bench_cb;
   run->name = name;
   run->start = count_start;
   run->end = count_end;
   run->step = count_step;

   bench->runs = eina_inlist_append(bench->runs, EINA_INLIST_GET(run));

   return EINA_TRUE;
}

EAPI Eina_Array *
eina_benchmark_run(Eina_Benchmark *bench)
{
   FILE *main_script;
   FILE *current_data;
   Eina_Array *ea;
   Eina_Run *run;
   char *buffer;
   Eina_Bool first = EINA_FALSE;
   size_t length;
   Eina_Strbuf *sbname, *sbrun;

   if (!bench)
      return NULL;

   length = strlen(EINA_BENCHMARK_FILENAME_MASK) + strlen(bench->name) + strlen(
         bench->run);

   buffer = alloca(sizeof (char) * length);
   if (!buffer)
      return NULL;

   snprintf(buffer,
            length,
            EINA_BENCHMARK_FILENAME_MASK,
            bench->name,
            bench->run);

   main_script = fopen(buffer, "wb");
   if (!main_script)
      return NULL;

   ea = eina_array_new(16);
   if (!ea)
     {
        fclose(main_script);
        return NULL;
     }

   eina_array_push(ea, strdup(buffer));

   sbname = eina_strbuf_new();
   eina_strbuf_append(sbname, bench->name);
   eina_strbuf_replace_all(sbname, "_", "\\_");

   sbrun = eina_strbuf_new();
   eina_strbuf_append(sbrun, bench->run);
   eina_strbuf_replace_all(sbrun, "_", "\\_");

   fprintf(
      main_script,
      "set   autoscale                        # scale axes automatically\n"
      "unset log                              # remove any log-scaling\n"
      "unset label                            # remove any previous labels\n"
      "set xtic auto                          # set xtics automatically\n"
      "set ytic auto                          # set ytics automatically\n"
/*     "set logscale y\n" */
      "set terminal png size 1024,768\n"
      "set output \"output_%s_%s.png\"\n"
      "set title '%s %s'\n"
      "set xlabel \"tests\"\n"
      "set ylabel \"time\"\n"
      "plot ",
      bench->name,
      bench->run,
      eina_strbuf_string_get(sbname),
      eina_strbuf_string_get(sbrun));

   eina_strbuf_free(sbname);
   eina_strbuf_free(sbrun);

   EINA_INLIST_FOREACH(bench->runs, run)
   {
      Eina_Counter *counter;
      char *result;
      size_t tmp;
      int i;

      tmp = strlen(EINA_BENCHMARK_DATA_MASK) + strlen(bench->name) + strlen(
            bench->run) + strlen(run->name);
      if (tmp > length)
        {
           buffer = alloca(sizeof (char) * tmp);
           length = tmp;
        }

      snprintf(buffer,
               length,
               EINA_BENCHMARK_DATA_MASK,
               bench->name,
               bench->run,
               run->name);

      current_data = fopen(buffer, "wb");
      if (!current_data)
         continue;

      eina_array_push(ea, strdup(buffer));

      counter = eina_counter_new(run->name);

      for (i = run->start; i < run->end; i += run->step)
        {
           fprintf(stderr, "Run %s: %i\n", run->name, i);
           eina_counter_start(counter);

           run->cb(i);

           eina_counter_stop(counter, i);
        }

      result = eina_counter_dump(counter);
      if (result)
        {
           fprintf(current_data, "%s", result);
           free(result);
        }

      eina_counter_free(counter);

      fclose(current_data);

      if (first == EINA_FALSE)
         first = EINA_TRUE;
      else
         fprintf(main_script, ", \\\n");

      sbname = eina_strbuf_new();
      eina_strbuf_append(sbname, run->name);
      eina_strbuf_replace_all(sbname, "_", "\\_");

      fprintf(main_script,
              "\"%s\" using 1:2 title \'%s\' with line",
              buffer, eina_strbuf_string_get(sbname));

      eina_strbuf_free(sbname);
   }

   fprintf(main_script, "\n");

   fclose(main_script);

   bench->names = eina_list_append(bench->names, ea);

   return ea;
}

/**
 * @}
 */
