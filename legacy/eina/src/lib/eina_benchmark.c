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


/**
 * @page tutorial_benchmark_page Benchmark Tutorial
 *
 * to be written...
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "eina_benchmark.h"
#include "eina_inlist.h"
#include "eina_counter.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Run Eina_Run;
struct _Eina_Run
{
   Eina_Inlist __list;

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
};

static int _eina_benchmark_count = 0;

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Tools_Group Tools Modules
 *
 * @{
 */

/**
 * @addtogroup Eina_Benchmark_Group Benchmark Functions
 *
 * These functions allow you to add benchmark framework in a project.
 *
 * For more information, you can look at the @ref tutorial_benchmark_page.
 *
 * @{
 */

EAPI int
eina_benchmark_init(void)
{
   _eina_benchmark_count++;

   if (_eina_benchmark_count > 1) return _eina_benchmark_count;

   eina_error_init();
   eina_array_init();
   eina_counter_init();

   return _eina_benchmark_count;
}

EAPI int
eina_benchmark_shutdown(void)
{
   _eina_benchmark_count--;

   if (_eina_benchmark_count != 0) return _eina_benchmark_count;

   eina_counter_shutdown();
   eina_array_shutdown();
   eina_error_shutdown();

   return 0;
}

EAPI Eina_Benchmark *
eina_benchmark_new(const char *name, const char *run)
{
   Eina_Benchmark *new;

   eina_error_set(0);
   new = calloc(1, sizeof (Eina_Benchmark));
   if (!new)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return NULL;
     }

   new->name = name;
   new->run = run;

   return new;
}

EAPI void
eina_benchmark_free(Eina_Benchmark *bench)
{
   if (!bench) return ;

   while (bench->runs)
     {
	Eina_Run *run = (Eina_Run *) bench->runs;

	bench->runs = eina_inlist_remove(bench->runs, bench->runs);
	free(run);
     }

   free(bench);
}

EAPI void
eina_benchmark_register(Eina_Benchmark *bench, const char *name, Eina_Benchmark_Specimens bench_cb,
			int count_start, int count_end, int count_step)
{
   Eina_Run *run;

   if (!bench) return ;

   eina_error_set(0);
   run = calloc(1, sizeof (Eina_Run));
   if (!run)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return ;
     }

   run->cb = bench_cb;
   run->name = name;
   run->start = count_start;
   run->end = count_end;
   run->step = count_step;

   bench->runs = eina_inlist_append(bench->runs, run);
}

EAPI Eina_Array *
eina_benchmark_run(Eina_Benchmark *bench)
{
   FILE *main_script;
   FILE *current_data;
   Eina_Array *ea;
   Eina_Run *run;
   char buffer[PATH_MAX];
   Eina_Bool first = EINA_FALSE;

   if (!bench) return NULL;

   snprintf(buffer, PATH_MAX, "bench_%s_%s.gnuplot", bench->name, bench->run);

   main_script = fopen(buffer, "w");
   if (!main_script) return NULL;

   ea = eina_array_new(16);
   if (!ea)
     {
	fclose(main_script);
	return NULL;
     }

   eina_array_push(ea, strdup(buffer));

   fprintf(main_script,
	   "set   autoscale                        # scale axes automatically\n"
	   "unset log                              # remove any log-scaling\n"
	   "unset label                            # remove any previous labels\n"
	   "set xtic auto                          # set xtics automatically\n"
	   "set ytic auto                          # set ytics automatically\n"
/* 	   "set logscale y\n" */
	   "set terminal png size 1024,768\n"
	   "set output \"output_%s_%s.png\"\n"
	   "set title \"%s %s\n"
	   "set xlabel \"tests\"\n"
	   "set ylabel \"time\"\n"
	   "plot ", bench->name, bench->run, bench->name, bench->run);

   EINA_INLIST_ITER_NEXT(bench->runs, run)
     {
	Eina_Counter *counter;
	int i;

	snprintf(buffer, PATH_MAX, "bench_%s_%s.%s.data", bench->name, bench->run, run->name);

	current_data = fopen(buffer, "w");
	if (!current_data) continue ;

	eina_array_push(ea, strdup(buffer));

	counter = eina_counter_add(run->name);

	for (i = run->start; i < run->end; i += run->step)
	  {
	     fprintf(stderr, "Run %s: %i\n", run->name, i);
	     eina_counter_start(counter);

	     run->cb(i);

	     eina_counter_stop(counter, i);
	  }

	eina_counter_dump(counter, current_data);

	eina_counter_delete(counter);

	fclose(current_data);

	if (first == EINA_FALSE) first = EINA_TRUE;
	else fprintf(main_script, ", \\\n");

	fprintf(main_script,
		"\"%s\" using 1:2 title \'%s\' with line",
		buffer, run->name);
     }

   fprintf(main_script, "\n");

   fclose(main_script);

   return ea;
}

/**
 * @}
 */

/**
 * @}
 */
