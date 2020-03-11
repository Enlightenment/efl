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

#include "eina_config.h"
#include "eina_private.h"
#include "eina_inlist.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_counter.h"

#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

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


static char *
_eina_counter_asiprintf(char *base, int *position, const char *format, ...)
{
   char *tmp, *result;
   int size = 32;
   int n;
   va_list ap;

   tmp = realloc(base, sizeof (char) * (*position + size));
   if (!tmp)
      return base;

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

        if (n > -1)
           size = n + 1;
        else
           size <<= 1;

        tmp = realloc(result, sizeof (char) * (*position + size));
        if (!tmp)
           return result;

        result = tmp;
     }
}

/**
 * @endcond
 */

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Counter *
eina_counter_new(const char *name)
{
   Eina_Counter *counter;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   length = strlen(name) + 1;
   counter = calloc(1, sizeof (Eina_Counter) + length);
   if (!counter) return NULL;

   counter->name = (char *)(counter + 1);
   memcpy((char *)counter->name, name, length);

   return counter;
}

EAPI void
eina_counter_free(Eina_Counter *counter)
{
   EINA_SAFETY_ON_NULL_RETURN(counter);

   while (counter->clocks)
     {
        Eina_Clock *clk = (Eina_Clock *)counter->clocks;

        counter->clocks = eina_inlist_remove(counter->clocks, counter->clocks);
        free(clk);
     }

   free(counter);
}

EAPI void
eina_counter_start(Eina_Counter *counter)
{
   Eina_Clock *clk;
   Eina_Nano_Time tp;

   EINA_SAFETY_ON_NULL_RETURN(counter);
   if (_eina_time_get(&tp) != 0) return;

   clk = calloc(1, sizeof (Eina_Clock));
   if (!clk) return;

   counter->clocks = eina_inlist_prepend(counter->clocks, EINA_INLIST_GET(clk));

   clk->valid = EINA_FALSE;
   clk->start = tp;
}

EAPI void
eina_counter_stop(Eina_Counter *counter, int specimen)
{
   Eina_Clock *clk;
   Eina_Nano_Time tp;

   EINA_SAFETY_ON_NULL_RETURN(counter);
   if (_eina_time_get(&tp) != 0)
      return;

   clk = (Eina_Clock *)counter->clocks;

   if (!clk || clk->valid == EINA_TRUE)
      return;

   clk->end = tp;
   clk->specimen = specimen;
   clk->valid = EINA_TRUE;
}

EAPI char *
eina_counter_dump(Eina_Counter *counter)
{
   Eina_Clock *clk;
   char *result = NULL;
   int position = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(counter, NULL);

   result = _eina_counter_asiprintf(
         result,
         &position,
         "# specimen\texperiment time\tstarting time\tending time\n");
   if (!result)
      return NULL;

   EINA_INLIST_REVERSE_FOREACH(counter->clocks, clk)
   {
      long int start;
      long int end;
      long int diff;

      if (clk->valid == EINA_FALSE)
         continue;

      start = _eina_time_convert(&clk->start);
      end = _eina_time_convert(&clk->end);
      diff = _eina_time_delta(&clk->start, &clk->end);

      result = _eina_counter_asiprintf(result, &position,
                                       "%i\t%li\t%li\t%li\n",
                                       clk->specimen,
                                       diff,
                                       start,
                                       end);
   }

   return result;
}
