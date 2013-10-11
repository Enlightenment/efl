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

#ifndef EINA_BENCHMARK_H_
#define EINA_BENCHMARK_H_

#include "eina_array.h"



/**
 * @page tutorial_benchmark_page Benchmark Tutorial
 *
 * The Benchmark module allows you to write easily benchmarks
 * framework in a project for timing critical part and detect slow
 * parts of code. In addition it automatically creates data files of
 * these benchmark, as well as a gnuplot file which can display the
 * comparison curves of the benchmarks.
 *
 * @section tutorial_benchmark_basic_usage Basic Usage
 *
 * To create a basic benchmark, you have to follow these steps:
 *
 * @li Create a new benchmark
 * @li Write the functions that wraps the functions you want to
 * benchmark.
 * @li Register these wrappers functions.
 * @li Run the benchmark.
 * @li Free the memory.
 *
 * Here is a basic example of benchmark which creates two functions
 * that will be run. These functions just print a message.
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <Eina.h>
 *
 * static
 * void work1(int request)
 * {
 *   printf ("work1 in progress... Request: %d\n", request);
 * }
 *
 * static
 * void work2(int request)
 * {
 *   printf ("work2 in progress... Request: %d\n", request);
 * }
 *
 * int main()
 * {
 *   Eina_Benchmark *test;
 *   Eina_Array     *ea;
 *
 *   if (!eina_init())
 *     return EXIT_FAILURE;
 *
 *   test = eina_benchmark_new("test", "run");
 *   if (!test)
 *     goto shutdown_eina;
 *
 *   eina_benchmark_register(test, "work-1", EINA_BENCHMARK(work1), 200, 300, 10);
 *   eina_benchmark_register(test, "work-2", EINA_BENCHMARK(work2), 100, 150, 5);
 *
 *   ea = eina_benchmark_run(test);
 *
 *   eina_benchmark_free(test);
 *   eina_shutdown();
 *
 *   return EXIT_SUCCESS;
 *
 *  shutdown_eina:
 *   eina_shutdown();
 *
 *   return EXIT_FAILURE;
 * }
 * @endcode
 *
 * As "test", "run" are passed to eina_benchmark_new() and as the tests
 * "work-1" and "work-2" are registered, the data files
 * bench_test_run.work-1.data and bench_test_run.work-2.data will be
 * created after the eina_benchmark_run() call. They contain four
 * columns. The file bench_test_run.work-1.data contains for example:
 *
 * @code
 * # specimen      experiment time starting time   ending time
 * 200     23632   2852446 2876078
 * 210     6924    2883046 2889970
 * 220     6467    2895962 2902429
 * 230     6508    2908271 2914779
 * 240     6278    2920610 2926888
 * 250     6342    2932830 2939172
 * 260     6252    2944954 2951206
 * 270     6463    2956978 2963441
 * 280     6347    2969548 2975895
 * 290     6457    2981702 2988159
 * @endcode
 *
 * The first column (specimen) is the integer passed to the work1()
 * function when the test is run. The second column (experiment time)
 * is the time, in nanosecond, that work1() takes. The third and
 * fourth columnd are self-explicit.
 *
 * You can see that the integer passed work1() starts from 200 and
 * finishes at 290, with a step of 10. These values are computed withe
 * last 3 values passed to eina_benchmark_register(). See the document
 * of that function for the detailed behavior.
 *
 * The gnuplot file will be named bench_test_run.gnuplot. Just run:
 *
 * @code
 * gnuplot bench_test_run.gnuplot
 * @endcode
 *
 * to create the graphic of the comparison curves. The image file is
 * named output_test_run.png.
 *
 * @section tutorial_benchmark_advanced_usage More Advanced Usage
 *
 * In this section, several test will be created and run. The idea is
 * exactly the same than in the previous section, but with some basic
 * automatic way to run all the benchmarks. The following code
 * benchmarks some Eina converts functions, and some Eina containers
 * types:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 * #include <time.h>
 *
 * #include <Eina.h>
 *
 * static void bench_convert(Eina_Benchmark *bench);
 * static void bench_container(Eina_Benchmark *bench);
 *
 * typedef struct _Benchmark_Case Benchmark_Case;
 *
 * struct _Benchmark_Case
 * {
 *    const char *bench_case;
 *    void (*build)(Eina_Benchmark *bench);
 * };
 *
 * static const Benchmark_Case benchmarks[] = {
 *   { "Bench 1", bench_convert },
 *   { "Bench 2", bench_container },
 *   { NULL,      NULL }
 * };
 *
 * static
 * void convert1(int request)
 * {
 *   char tmp[128];
 *   int i;
 *
 *   srand(time(NULL));
 *
 *   for (i = 0; i < request; ++i)
 *     eina_convert_itoa(rand(), tmp);
 * }
 *
 * static
 * void convert2(int request)
 * {
 *   char tmp[128];
 *   int i;
 *
 *   srand(time(NULL));
 *
 *   for (i = 0; i < request; ++i)
 *     eina_convert_xtoa(rand(), tmp);
 * }
 *
 * static void
 * bench_convert(Eina_Benchmark *bench)
 * {
 *   eina_benchmark_register(bench, "convert-1", EINA_BENCHMARK(convert1), 200, 400, 10);
 *   eina_benchmark_register(bench, "convert-2", EINA_BENCHMARK(convert2), 200, 400, 10);
 * }
 *
 * static
 * void array(int request)
 * {
 *   Eina_Array *array;
 *   Eina_Array_Iterator it;
 *   int *data;
 *   int i;
 *
 *   srand(time(NULL));
 *
 *   array = eina_array_new(64);
 *
 *   for (i = 0; i < request; ++i)
 *     {
 *       data = (int *)malloc(sizeof(int));
 *       if (!data) continue;
 *       *data = rand();
 *       eina_array_push(array, data);
 *     }
 *
 *   EINA_ARRAY_ITER_NEXT(array, i, data, it)
 *     free(data);
 *
 *   eina_array_free(array);
 * }
 *
 * static
 * void list(int request)
 * {
 *   Eina_List *l = NULL;
 *   int *data;
 *   int i;
 *
 *   srand(time(NULL));
 *
 *   for (i = 0; i < request; ++i)
 *     {
 *       data = (int *)malloc(sizeof(int));
 *       if (!data) continue;
 *       *data = rand();
 *       l = eina_list_prepend(l, data);
 *     }
 *
 *   while (l)
 *     {
 *       free(eina_list_data_get(l));
 *       l = eina_list_remove_list(l, l);
 *     }
 * }
 *
 * static void
 * bench_container(Eina_Benchmark *bench)
 * {
 *   eina_benchmark_register(bench, "array", EINA_BENCHMARK(array), 200, 300, 10);
 *   eina_benchmark_register(bench, "list", EINA_BENCHMARK(list), 200, 300, 10);
 * }
 *
 * int main()
 * {
 *   Eina_Benchmark *test;
 *   Eina_Array     *ea;
 *   unsigned int    i;
 *
 *   if (!eina_init())
 *     return EXIT_FAILURE;
 *
 *   for (i = 0; benchmarks[i].bench_case != NULL; ++i)
 *     {
 *       test = eina_benchmark_new(benchmarks[i].bench_case, "Benchmark example");
 *       if (!test)
 *         continue;
 *
 *       benchmarks[i].build(test);
 *
 *       ea = eina_benchmark_run(test);
 *
 *       eina_benchmark_free(test);
 *     }
 *
 *   eina_shutdown();
 *
 *   return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * gnuplot can be used to see how are performed the convert functions
 * together, as well as how are performed the containers. So it is now
 * easy to see that the hexadecimal convert function is faster than
 * the decimal one, and that arrays are faster than lists.
 *
 * You can improve all that by executing automatically gnuplot in your
 * program, or integrate the Eina benchmark framework in an autotooled
 * project. See that
 * <a href="http://trac.enlightenment.org/e/wiki/AutotoolsIntegration#Benchmark">page</a>
 * for more informations.
 *
 */


/**
 * @addtogroup Eina_Benchmark_Group Benchmark
 *
 * These functions allow you to add benchmark framework in a project
 * for timing critical part and detect slow parts of code. It is used
 * in Eina to compare the time used by eina, glib, evas and ecore data
 * types.
 *
 * To use the benchmark module, Eina must be initialized with
 * eina_init() and later shut down with eina_shutdown(). A benchmark
 * is created with eina_benchmark_new() and freed with
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
 */

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Benchmark_Group Benchmark
 *
 * @{
 */

/**
 * @typedef Eina_Benchmark
 * Type for a benchmark.
 */
typedef struct _Eina_Benchmark Eina_Benchmark;

/**
 * @typedef Eina_Benchmark_Specimens
 * Type for a test function to be called when running a benchmark.
 */
typedef void (*Eina_Benchmark_Specimens)(int request);

/**
 * @def EINA_BENCHMARK
 * @brief cast to an #Eina_Benchmark_Specimens.
 *
 * @param function The function to cast.
 *
 * This macro casts @p function to Eina_Benchmark_Specimens.
 */
#define EINA_BENCHMARK(function) ((Eina_Benchmark_Specimens)function)


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
 * memory allocation fails.
 * 
 * When the new module is not needed anymore, use
 * eina_benchmark_free() to free the allocated memory.
 */
EAPI Eina_Benchmark *eina_benchmark_new(const char *name,
                                        const char *run);

/**
 * @brief Free a benchmark object.
 *
 * @param bench The benchmark to free.
 *
 * This function removes all the benchmark tests that have been
 * registered and frees @p bench. If @p bench is @c NULL, this
 * function returns immediately.
 */
EAPI void            eina_benchmark_free(Eina_Benchmark *bench);

/**
 * @brief Add a test to a benchmark.
 *
 * @param bench The benchmark.
 * @param name The name of the test.
 * @param bench_cb The test function to be called.
 * @param count_start The start data to be passed to @p bench_cb.
 * @param count_end The end data to be passed to @p bench_cb.
 * @param count_step The step data to be passed to @p bench_cb.
 * @return #EINA_FALSE on failure, #EINA_TRUE otherwise.
 *
 * This function adds the test named @p name to @p benchmark. @p
 * bench_cb is the function called when the test is executed. That
 * test can be executed a certain amount of time. @p count_start, @p count_end and
 * @p count_step define a loop with a step increment. The integer that is
 * increasing by @p count_step from @p count_start to @p count_end is passed to @p
 * bench_cb when eina_benchmark_run() is called.
 *
 * If @p bench is @c NULL, this function returns immediately.
 * This function returns #EINA_FALSE on failure, #EINA_TRUE otherwise.
 */
EAPI Eina_Bool       eina_benchmark_register(Eina_Benchmark          *bench,
                                             const char              *name,
                                             Eina_Benchmark_Specimens bench_cb,
                                             int                      count_start,
                                             int                      count_end,
                                             int                      count_step);

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
 * immediately. Otherwise, it returns the list of the names of each
 * test.
 */
EAPI Eina_Array *eina_benchmark_run(Eina_Benchmark *bench);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_BENCHMARK_H_ */
