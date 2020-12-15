/* EINA - EFL data type library
 * Copyright (C) 2012 Cedric Bail
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

#include <stddef.h>

#ifdef HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "eina_types.h"
#include "eina_config.h"
#include "eina_thread.h"

EINA_API void *
eina_thread_cancellable_run(Eina_Thread_Cancellable_Run_Cb cb, Eina_Free_Cb cleanup_cb, void *data)
{
   Eina_Bool old = EINA_FALSE;
   void *ret;

   EINA_THREAD_CLEANUP_PUSH(cleanup_cb, data);
   eina_thread_cancellable_set(EINA_TRUE, &old); // is a cancellation point
   ret = cb(data); // may not run if was previously canceled
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   eina_thread_cancellable_set(old, NULL);
   return ret;
}
