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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

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
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @addtogroup Eina_Benchmark_Group Benchmark
 *
 * These functions allow you to add benchmark framework in a project
 * for timing critical part and detect slow parts of code. It is used
 * in Eina to compare the time used by eina, glib, evas and ecore data
 * types.
 *
 * The benchmark module must be initialized with eina_benchmark_init()
 * and shut down with eina_benchmark_shutdown(). A benchmark is
 * created with eina_benchmark_new() and freed with
 * eina_benchmark_free().
 *
 * eina_benchmark_register() adds a test to a benchmark. That test can
 * be run a certain amount of times. Adding more than one test to be
 * executed allows the comparison between several parts of a program,
 * or different implementations.
 *
 * eina_benchmark_run() runs all the tests registered with
 * eina_benchmark_register(). The amount of time of each test is
 * written in a gnuplot file.
 *
 * For more information, you can look at the @ref tutorial_benchmark_page.
 *
 * @{
 */

/**
 * @brief Initialize the benchmark module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the error, array and counter modules or
 * Eina. It is also called by eina_init(). It returns 0 on failure,
 * otherwise it returns the number of times eina_error_init() has
 * already been called.
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

/**
 * @brief Shut down the benchmark module.
 *
 * @return 0 when the error module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shut down the error, array and counter modules set up
 * by eina_array_init(). It is also called by eina_shutdown(). It returns
 * 0 when it is called the same number of times than eina_error_init().
 */
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

/**
 * @brief Create a new array.
 *
 * @param name The name of the benchmark.
 * @param run The name of the run.
 * @return @c NULL on failure, non @c NULL otherwise.
 *
 * This function creates a new benchmark. @p name and @p run are used
 * to name the gnuplot file that eina_benchmark_run() will create.
 *
 * This function return a valid benchmark on success, or @c NULL if
 * memory allocation fails. In that case, the error is set to
 * #EINA_ERROR_OUT_OF_MEMORY.
 */
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

/**
 * @brief Free a benchmark object.
 *
 * @param bench The benchmark to free.
 *
 * This function removes all the benchmark tests that have been
 * registered and frees @p bench. If @p bench is @c NULL, this
 * function returns immediatly.
 */
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

/**
 * @brief Add a test to a benchmark.
 *
 * @param bench The benchmark.
 * @param name The name of the test.
 * @param bench_cb The test function to be called.
 * @param count_start The start data to be passed to @p bench_cb.
 * @param count_end The end data to be passed to @p bench_cb.
 * @param count_step The step data to be passed to @p bench_cb.
 *
 * This function adds the test named @p name to @p benchmark. @p
 * bench_cb is the function called when the test is executed. That
 * test can be executed a certain amount of time. @p start, @p end and
 * @p step define a loop with a step increment. The integer that is
 * increasing by @p step from @p start to @p end is passed to @p
 * bench_cb when eina_benchmark_run() is called.
 *
 * If @p bench is @c NULL, this function returns imediatly. If the
 * allocation of the memory of the test to add fails, the error is set
 * to #EINA_ERROR_OUT_OF_MEMORY.
 */
EAPI Eina_Bool
eina_benchmark_register(Eina_Benchmark *bench, const char *name, Eina_Benchmark_Specimens bench_cb,
			int count_start, int count_end, int count_step)
{
   Eina_Run *run;

   if (!bench) return EINA_FALSE;
   if (count_step == 0) return EINA_FALSE;

   eina_error_set(0);
   run = calloc(1, sizeof (Eina_Run));
   if (!run)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return EINA_FALSE;
     }

   run->cb = bench_cb;
   run->name = name;
   run->start = count_start;
   run->end = count_end;
   run->step = count_step;

   bench->runs = eina_inlist_append(bench->runs, EINA_INLIST_GET(run));

   return EINA_TRUE;
}

/**
 * @brief Run the benchmark tests that have been registered.
 *
 * @param bench The benchmark.
 * @return The list of names of the test files.
 *
 * This function runs all the tests that as been registered with
 * eina_benchmark_register() and save the result in a gnuplot
 * file. The name of the file has the following format:
 *
 * @code
 * bench_[name]_[run]%s.gnuplot
 * @endcode
 *
 * where [name] and [run] are the values passed to
 * eina_benchmark_new().
 *
 * Each registered test is executed and timed. The time is written to
 * the gnuplot file. The number of times each test is executed is
 * controlled by the parameters passed to eina_benchmark_register().
 *
 * If @p bench is @c NULL, this functions returns @c NULL
 * immediatly. Otherwise, it returns the list of the names of each
 * test.
 */
#define EINA_BENCHMARK_FILENAME_MASK "bench_%s_%s.gnuplot"
#define EINA_BENCHMARK_DATA_MASK "bench_%s_%s.%s.data"
EAPI Eina_Array *
eina_benchmark_run(Eina_Benchmark *bench)
{
   FILE *main_script;
   FILE *current_data;
   Eina_Array *ea;
   Eina_Run *run;
   char *buffer;
   Eina_Bool first = EINA_FALSE;
   int length;

   if (!bench) return NULL;

   length = strlen(EINA_BENCHMARK_FILENAME_MASK) + strlen(bench->name) + strlen(bench->run);

   buffer = alloca(sizeof (char) * length);
   if (!buffer) return NULL;

   snprintf(buffer, length, EINA_BENCHMARK_FILENAME_MASK, bench->name, bench->run);

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

   EINA_INLIST_FOREACH(bench->runs, run)
     {
	Eina_Counter *counter;
	char *result;
	int tmp;
	int i;

	tmp = strlen(EINA_BENCHMARK_DATA_MASK) + strlen(bench->name) + strlen(bench->run) + strlen(run->name);
	if (tmp > length)
	  {
	     buffer = alloca(sizeof (char) * tmp);
	     length = tmp;
	  }

	snprintf(buffer, length, EINA_BENCHMARK_DATA_MASK, bench->name, bench->run, run->name);

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

	result = eina_counter_dump(counter);
	if (result)
	  {
	     fprintf(current_data, "%s", result);
	     free(result);
	  }

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
