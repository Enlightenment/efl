/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @internal
 * @defgroup Eina_Benchmark_Group Benchmark
 * @ingroup Eina_Tools_Group
 *
 * @brief This group discusses the functions that allow you to add the benchmark framework in a project
 *        for timing critical part and detecting slow parts of a code. It is used
 *        in Eina to compare the time used by eina, glib, evas, and ecore data
 *        types.
 *
 * To use the benchmark module, Eina must be initialized with
 * eina_init() and later shut down with eina_shutdown(). A benchmark
 * is created with eina_benchmark_new() and freed with
 * eina_benchmark_free().
 *
 * eina_benchmark_register() adds a test to a benchmark. That test can
 * be run a certain number of times. Adding more than one test to be
 * executed allows the comparison between several parts of a program,
 * or different implementations.
 *
 * eina_benchmark_run() runs all the tests registered with
 * eina_benchmark_register(). The amount of time for each test is
 * written in a gnuplot file.
 *
 * @{
 */

/**
 * @typedef Eina_Benchmark
 * @brief The structure type for a benchmark.
 */
typedef struct _Eina_Benchmark Eina_Benchmark;

/**
 * @typedef Eina_Benchmark_Specimens
 * @brief The structure type for a test function to be called when running a benchmark.
 */
typedef void (*Eina_Benchmark_Specimens)(int request);

/**
 * @def EINA_BENCHMARK
 * @brief Definition to cast to an #Eina_Benchmark_Specimens.
 *
 * @details This macro casts @a function to Eina_Benchmark_Specimens.
 *
 * @param function The function to cast
 *
 */
#define EINA_BENCHMARK(function) ((Eina_Benchmark_Specimens)function)

/**
 * @brief Creates a new array.
 *
 * @details This function creates a new benchmark. @a name and @a run are used
 *          to name the gnuplot file that eina_benchmark_run() creates.
 *
 * @since_tizen 2.3
 *
 * @remarks This function returns a valid benchmark on success, or @c NULL if
 *          memory allocation fails. In that case, the error is set
 *          to #EINA_ERROR_OUT_OF_MEMORY.
 *
 * @remarks When the new module is not needed anymore, use
 *          eina_benchmark_free() to free the allocated memory.
 *
 * @param[in] name The name of the benchmark
 * @param[in] run The name of the run
 * @return @c NULL on failure, otherwise a non @c NULL value
 *
 */
EAPI Eina_Benchmark *eina_benchmark_new(const char *name,
                                        const char *run);

/**
 * @brief Frees a benchmark object.
 *
 * @details This function removes all the benchmark tests that have been
 *          registered and frees @a bench. If @a bench is @c NULL, this
 *          function returns immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] bench The benchmark to free
 *
 */
EAPI void            eina_benchmark_free(Eina_Benchmark *bench);

/**
 * @brief Adds a test to a benchmark.
 *
 * @details This function adds the test named @a name to @a benchmark. @a
 *          bench_cb is the function called when the test is executed. That
 *          test can be executed for a certain amount of time. @a count_start, @a count_end, and
 *          @a count_step define a loop with a step increment. The integer that is
 *          increasing by @a count_step from @a count_start to @a count_end is passed to @a
 *          bench_cb when eina_benchmark_run() is called.
 *
 * @since_tizen 2.3
 *
 * @remarks If @a bench is @c NULL, this function returns immediately. If the
 *          allocation of the memory of the test to add fails, the error is set
 *          to #EINA_ERROR_OUT_OF_MEMORY. This function returns @c EINA_FALSE
 *          on failure, otherwise it returns @c EINA_TRUE.
 *
 * @param[in] bench The benchmark
 * @param[in] name The name of the test
 * @param[in] bench_cb The test function to be called
 * @param[in] count_start The start data to be passed to @a bench_cb
 * @param[in] count_end The end data to be passed to @a bench_cb
 * @param[in] count_step The step data to be passed to @a bench_cb
 * @return @c EINA_FALSE on failure, otherwise @c EINA_TRUE
 *
 */
EAPI Eina_Bool       eina_benchmark_register(Eina_Benchmark          *bench,
                                             const char              *name,
                                             Eina_Benchmark_Specimens bench_cb,
                                             int                      count_start,
                                             int                      count_end,
                                             int                      count_step);

/**
 * @brief Runs the benchmark tests that have been registered.
 *
 * @details This function runs all the tests that have been registered with
 *          eina_benchmark_register() and saves the result in a gnuplot
 *          file. The name of the file has the following format:
 *
 * @code
 * bench_[name]_[run]%s.gnuplot
 * @endcode
 *
 *          where [name] and [run] are the values passed to
 *          eina_benchmark_new().
 *
 * @since_tizen 2.3
 *
 * @remarks Each registered test is executed and timed. The time is written to
 *          the gnuplot file. The number of times each test is executed is
 *          controlled by the parameters passed to eina_benchmark_register().
 *
 * @remarks If @a bench is @c NULL, this functions returns @c NULL
 *          immediately. Otherwise, it returns the list of names of each
 *          test.
 *
 * @param[in] bench The benchmark
 * @return The list of names of the test files
 *
 */
EAPI Eina_Array *eina_benchmark_run(Eina_Benchmark *bench);

/**
 * @}
 */

#endif /* EINA_BENCHMARK_H_ */
