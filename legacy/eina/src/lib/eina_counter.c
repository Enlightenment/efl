/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail, Vincent Torri
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WIN32
# include <time.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif /* _WIN2 */

#include "eina_counter.h"
#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifndef _WIN32
typedef struct timespec Eina_Nano_Time;
#else
typedef LARGE_INTEGER Eina_Nano_Time;
#endif

typedef struct _Eina_Clock Eina_Clock;

struct _Eina_Counter
{
   EINA_INLIST;

   Eina_Inlist *clocks;
   const char *name;
};

struct _Eina_Clock
{
   EINA_INLIST;

   Eina_Nano_Time start;
   Eina_Nano_Time end;
   int specimen;

   Eina_Bool valid;
};

static int _eina_counter_init_count = 0;

#ifndef _WIN32
static inline int
_eina_counter_time_get(Eina_Nano_Time *tp)
{
#if defined(CLOCK_PROCESS_CPUTIME_ID)
   return clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tp);
#elif defined(CLOCK_PROF)
   return clock_gettime(CLOCK_PROF, tp);
#else
   return clock_gettime(CLOCK_REALTIME, tp);
#endif
}
#else
static int EINA_COUNTER_ERROR_WINDOWS = 0;
static LARGE_INTEGER _eina_counter_frequency;

static inline int
_eina_counter_time_get(Eina_Nano_Time *tp)
{
   return QueryPerformanceCounter(tp);
}
#endif /* _WIN2 */

static char *
_eina_counter_asiprintf(char *base, int *position, const char *format, ...)
{
   char *tmp, *result;
   int size = 32;
   int n;
   va_list ap;

   tmp = realloc(base, sizeof (char) * (*position + size));
   if (!tmp) return base;
   result = tmp;

   while (1)
     {
	va_start(ap, format);
	n = vsnprintf(result + *position, size, format, ap);
	va_end(ap);

	if (n > -1 && n < size)
	  {
	     /* If we always have glibc > 2.2, we could just return *position += n. */
	     *position += strlen(result + *position);
	     return result;
	  }

	if (n > -1) size = n + 1;
	else size <<= 1;

	tmp = realloc(result, sizeof (char) * (*position + size));
	if (!tmp) return result;
	result = tmp;
     }
}

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
 * @addtogroup Eina_Counter_Group Counter
 *
 * @brief These functions allow you to get the time spent in a part of a code.
 *
 * The counter system must be initialized with eina_counter_init() and
 * shut down with eina_counter_shutdown(). The create a counter, use
 * eina_counter_add(). To free it, use eina_counter_delete().
 *
 * To time a part of a code, call eina_counter_start() just before it,
 * and eina_counter_stop() just after it. Each time you start to time
 * a code, a clock is added to a list. You can give a number of that
 * clock with the second argument of eina_counter_stop(). To send all
 * the registered clocks to a stream (like stdout, ofr a file), use
 * eina_counter_dump().
 *
 * Here is a straightforward example:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_counter.h>
 *
 * void test_malloc(void)
 * {
 *    int i;
 *
 *    for (i = 0; i < 100000; ++i)
 *    {
 *       void *buf;
 *
 *       buf = malloc(100);
 *       free(buf);
 *    }
 * }
 *
 * int main(void)
 * {
 *    Eina_Counter *counter;
 *
 *    if (!eina_counter_init())
 *    {
 *        printf("Error during the initialization of eina_counter module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    counter = eina_counter_add("malloc");
 *
 *    eina_counter_start(counter);
 *    test_malloc();
 *    eina_counter_stop(counter, 1);
 *
 *    eina_counter_dump(counter, stdout);
 *
 *    eina_counter_delete(counter);
 *    eina_counter_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following commant:
 *
 * @code
 * gcc -Wall -o test_eina_counter test_eina.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 * The result should be something like that:
 *
 * @code
 * # specimen	experiment time	starting time	ending time
 * 1	9794125	783816	10577941
 * @endcode
 *
 * Note that the displayed time is in nanosecond.
 *
 * @{
 */

/**
 * @brief Initialize the eina counter internal structure.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function allocates the memory needed by the counter, which
 * means that it sets up the error module of Eina, and only on Windows
 * it initializes the high precision timer. It also registers the errors
 * #EINA_ERROR_OUT_OF_MEMORY and, if on Windows,
 * #EINA_COUNTER_ERROR_WINDOWS. It is also called by eina_init(). It
 * returns 0 on failure, otherwise it returns the number of times it
 * has already been called.
 */
EAPI int
eina_counter_init(void)
{
   _eina_counter_init_count++;

   if (_eina_counter_init_count == 1)
     {
	eina_error_init();
#ifdef _WIN32
        if (!QueryPerformanceFrequency(&_eina_counter_frequency))
          {
             EINA_COUNTER_ERROR_WINDOWS = eina_error_msg_register("Change your OS, you moron !");
             _eina_counter_init_count--;
             return 0;
          }
#endif /* _WIN2 */
     }

   return _eina_counter_init_count;
}

/**
 * @brief Shut down the eina counter internal structures
 *
 * @return 0 when the counter module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function just shuts down the error module. It is also called by
 * eina_shutdown(). It returns 0 when it is called the same number of
 * times than eina_counter_init().
 */
EAPI int
eina_counter_shutdown(void)
{
   _eina_counter_init_count--;

   if (_eina_counter_init_count == 0) eina_error_shutdown();

   return _eina_counter_init_count;
}

/**
 * @brief Return a counter.
 *
 * @param name The name of the counter.
 *
 * This function returns a new counter. It is characterized by @p
 * name. If @p name is @c NULL, the function returns @c NULL
 * immediatly. If memory allocation fails, @c NULL is returned and the
 * error is set to #EINA_ERROR_OUT_OF_MEMORY.
 */
EAPI Eina_Counter *
eina_counter_add(const char *name)
{
   Eina_Counter *counter;
   int length;

   if (!name) return NULL;

   length = strlen(name) + 1;

   eina_error_set(0);
   counter = calloc(1, sizeof (Eina_Counter) + length);
   if (!counter)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return NULL;
     }

   counter->name = (char*) (counter + 1);
   memcpy((char*) counter->name, name, length);

   return counter;
}

/**
 * @brief Delete a counter.
 *
 * @param counter The counter to delete.
 *
 * This function remove the clock of @p counter from the used clocks
 * (see eina_counter_start()) and frees the memory allocated for
 * @p counter. If @p counter is @c NULL, the functions returns
 * immediatly.
 */
EAPI void
eina_counter_delete(Eina_Counter *counter)
{
   if (!counter) return;

   while (counter->clocks)
     {
	Eina_Clock *clock = (Eina_Clock *) counter->clocks;

	counter->clocks = eina_inlist_remove(counter->clocks, counter->clocks);
	free(clock);
     }

   free(counter);
}

/**
 * @brief Start the time count.
 *
 * @param counter The counter.
 *
 * This function specifies that the part of the code beginning just
 * after its call is being to be timed, using @p counter. If
 * @p counter is @c NULL, this function returns immediatly.
 *
 * This function adds the clock associated to @p counter in a list. If
 * the memory needed by that clock can not be allocated, the function
 * returns and the error is set to #EINA_ERROR_OUT_OF_MEMORY.
 *
 * To stop the timing, eina_counter_stop() must be called with the
 * same counter.
 */
EAPI void
eina_counter_start(Eina_Counter *counter)
{
   Eina_Clock *clk;
   Eina_Nano_Time tp;

   if (!counter) return;
   if (_eina_counter_time_get(&tp) != 0) return;

   eina_error_set(0);
   clk = calloc(1, sizeof (Eina_Clock));
   if (!clk)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return;
     }

   counter->clocks = eina_inlist_prepend(counter->clocks, EINA_INLIST_GET(clk));

   clk->valid = EINA_FALSE;
   clk->start = tp;
}

/**
 * @brief Stop the time count.
 *
 * @param counter The counter.
 * @param specimen The number of the test.
 *
 * This function stop the timing that has been started with
 * eina_counter_start(). @p counter must be the same than the one used
 * with eina_counter_start(). @p specimen is the number of the
 * test. If @p counter or its associated clock are  @c NULL, or if the
 * time can't be retrieved the function exits.
 */
EAPI void
eina_counter_stop(Eina_Counter *counter, int specimen)
{
   Eina_Clock *clk;
   Eina_Nano_Time tp;

   if (!counter) return;
   if (_eina_counter_time_get(&tp) != 0) return;

   clk = (Eina_Clock *) counter->clocks;

   if (!clk || clk->valid == EINA_TRUE) return;

   clk->end = tp;
   clk->specimen = specimen;
   clk->valid = EINA_TRUE;
}

/**
 * @brief Dump the result of all clocks of a counter to a stream.
 *
 * @return A string with a summary of the test.
 * @param counter The counter.
 *
 * This function dump all the valid clocks of @p counter to the stream
 * @p out. If @p counter or @p out are @c NULL, the functions exits
 * immediatly. Otherwise, the output is formattted like that:
 *
 * @code
 * \# specimen    experiment time    starting time    ending time
 * 1              208                120000           120208
 * @endcode
 *
 * The unit of time is the nanosecond.
*/
EAPI char *
eina_counter_dump(Eina_Counter *counter)
{
   Eina_Clock *clk;
   char *result = NULL;
   int position = 0;

   if (!counter) return NULL;

   result = _eina_counter_asiprintf(result, &position, "# specimen\texperiment time\tstarting time\tending time\n");
   if (!result) return NULL;

   EINA_INLIST_REVERSE_FOREACH(counter->clocks, clk)
     {
        long int start;
        long int end;
        long int diff;

	if (clk->valid == EINA_FALSE) continue;

#ifndef _WIN32
        start = clk->start.tv_sec * 1000000000 + clk->start.tv_nsec;
        end = clk->end.tv_sec * 1000000000 + clk->end.tv_nsec;
        diff = (clk->end.tv_sec - clk->start.tv_sec) * 1000000000 + clk->end.tv_nsec - clk->start.tv_nsec;
#else
        start = (long int)(((long long int)clk->start.QuadPart * 1000000000ll) / (long long int)_eina_counter_frequency.QuadPart);
        end = (long int)(((long long int)clk->end.QuadPart * 1000000000LL) / (long long int)_eina_counter_frequency.QuadPart);
        diff = (long int)(((long long int)(clk->end.QuadPart - clk->start.QuadPart) * 1000000000LL) / (long long int)_eina_counter_frequency.QuadPart);
#endif /* _WIN2 */

	result = _eina_counter_asiprintf(result, &position,
					 "%i\t%li\t%li\t%li\n",
					 clk->specimen,
					 diff,
					 start,
					 end);
     }

   return result;
}

/**
 * @}
 */

/**
 * @}
 */
