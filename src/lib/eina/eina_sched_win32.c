/* EINA - EFL data type library
 * Copyright (C) 2020 Carlos Signor
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

#include "eina_sched.h"
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

EINA_API void
eina_sched_prio_drop(void)
{
   Eina_Thread thread_id;
   int sched_priority;

   thread_id = eina_thread_self();

   sched_priority = GetThreadPriority(thread_id);

   if (EINA_UNLIKELY(sched_priority == THREAD_PRIORITY_TIME_CRITICAL))
     {
        sched_priority -= RTNICENESS;

        /* We don't change the policy */
        if (sched_priority < 1)
          {
             EINA_LOG_INFO("RT prio < 1, setting to 1 instead");
             sched_priority = 1;
          }
        if (!SetThreadPriority(thread_id, sched_priority))
          {
             EINA_LOG_ERR("Unable to query sched parameters");
          }
     }
   else
     {
        sched_priority += NICENESS;

        /* We don't change the policy */
        if (sched_priority > THREAD_PRIORITY_TIME_CRITICAL)
          {
             EINA_LOG_INFO("Max niceness reached; keeping max (THREAD_PRIORITY_TIME_CRITICAL)");
             sched_priority = THREAD_PRIORITY_TIME_CRITICAL;
          }
        if (!SetThreadPriority(thread_id, sched_priority))
          {
             EINA_LOG_ERR("Unable to query sched parameters");
          }
     }
}
