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


/**
 * @page tutorial_counter_page Counter Tutorial
 *
 * to be written...
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
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
   Eina_Inlist __list;

   Eina_Inlist *clocks;
   const char *name;
};

struct _Eina_Clock
{
   Eina_Inlist __list;

   Eina_Nano_Time start;
   Eina_Nano_Time end;
   int specimen;

   Eina_Bool valid;
};

static int _eina_counter_init_count = 0;
static int EINA_COUNTER_ERROR_OUT_OF_MEMORY = 0;

#ifndef _WIN32
static inline int
_eina_counter_time_get(Eina_Nano_Time *tp)
{
   return clock_gettime(CLOCK_PROCESS_CPUTIME_ID, tp);
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
 * @addtogroup Eina_Counter_Group Counter Functions
 *
 * These functions allow you to get the time spent in a part of a code.
 *
 * For more information, you can look at the @ref tutorial_counter_page.
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
 * #EINA_COUNTER_ERROR_OUT_OF_MEMORY and, if on Windows,
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
	EINA_COUNTER_ERROR_OUT_OF_MEMORY  = eina_error_msg_register("Eina_Counter out of memory");
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
 * error is set to #EINA_COUNTER_ERROR_OUT_OF_MEMORY.
 */
EAPI Eina_Counter *
eina_counter_add(const char *name)
{
   Eina_Counter *counter;
   int length;

   if (!name) return NULL;

   length = strlen(name) + 1;

   counter = calloc(1, sizeof (Eina_Counter) + length);
   if (!counter)
     {
	eina_error_set(EINA_COUNTER_ERROR_OUT_OF_MEMORY);
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
 * returns and the error is set to #EINA_COUNTER_ERROR_OUT_OF_MEMORY.
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

   clk = calloc(1, sizeof (Eina_Clock));
   if (!clk)
     {
	eina_error_set(EINA_COUNTER_ERROR_OUT_OF_MEMORY);
	return;
     }

   counter->clocks = eina_inlist_prepend(counter->clocks, clk);

   clk->valid = EINA_FALSE;
   clk->start = tp;
}

/**
 * @brief Stop the time count.
 *
 * @param counter The counter.
 * @param specimen The specimen.
 *
 * This function stop the timing that has been started with
 * eina_counter_start(). @p counter must be the same than the one used
 * with eina_counter_start(). @p specimen is the number of times the
 * test should be done. If @p counter or its associated clock are
 * @c NULL, or if the time can't be retrieved the function exits.
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
 * @param counter The counter.
 * @param out The stream to dump the clocks.
 *
 * This function dump all the valid clocks of @p counter to the stream
 * @p out. If @p counter or @p out are @c NULL, the functions exits
 * immediatly. Otherwise, the output is formattted like that:
 *
 * @code
 * \# specimen    experiment time    starting time    ending time
 * 1              208                120000           120208
 * @endcode
*/
EAPI void
eina_counter_dump(Eina_Counter *counter, FILE *out)
{
   Eina_Clock *clk;

   if (!counter || !out) return;

   fprintf(out, "# specimen\texperiment time\tstarting time\tending time\n");

   EINA_INLIST_ITER_LAST(counter->clocks, clk)
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

	fprintf(out, "%i\t%li\t%li\t%li\n",
		clk->specimen,
		diff,
		start,
		end);
     }
}

/**
 * @}
 */

/**
 * @}
 */
