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

#include <stdio.h>
#include <string.h>

#include "eina_config.h"
#include "eina_types.h"
#include "eina_hamster.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

const char *_eina_hamster = VTIME;
static int _eina_hamsters = -1;

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI int
eina_hamster_count(void)
{
   if (_eina_hamsters < 0)
     {
        int hrs = 0, min = 0, sec = 0;
        int monnum = 0, day = 0, year = 0;
        int zone = 0;
        int fields;

        fields = sscanf(_eina_hamster, "%04d-%02d-%02d %02d:%02d:%02d %d",
                        &year, &monnum, &day, &hrs, &min, &sec, &zone);
        if (fields == 7)
          {
             _eina_hamsters = (hrs * 60) + min;
             // alloc 60 for mins, 24 for hrs
             // alloc 1-31 (32) for days, 1-12 (13) for months
             // use year as-is, for 31 bits (signed) this gives us up to
             // 3584 years, which is good enough imho. - 1500 years from
             // now or so. :)
             _eina_hamsters +=
               (day + (monnum * 32) + (13 * 32 * year)) * (24 * 60);
          }
     }

   // format: [rest - year][0-12 - month][0-31 - day][0-23 - hrs][0-59 - sec]
   return _eina_hamsters;
}

/**
 * @}
 */
