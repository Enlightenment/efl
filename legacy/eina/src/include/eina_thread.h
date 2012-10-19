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

#ifndef EINA_THREAD_H_
#define EINA_THREAD_H_

#include "eina_config.h"
#include "eina_types.h"
#include "eina_error.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Thread_Group Thread
 *
 * @{
 */

#ifdef EINA_HAVE_THREADS
# ifdef _WIN32_WCE

typedef unsigned long int Eina_Thread;

# elif defined(_WIN32)

typedef unsigned long int Eina_Thread;

# else
#  include <pthread.h>

typedef pthread_t Eina_Thread;

# endif
#else
# error "Build without thread is not supported any more"
#endif

typedef void *(*Eina_Thread_Cb)(void *data, Eina_Thread t);

typedef enum _Eina_Thread_Priority
{
  EINA_THREAD_URGENT,
  EINA_THREAD_NORMAL,
  EINA_THREAD_BACKGROUND,
  EINA_THREAD_IDLE
} Eina_Thread_Priority;

EAPI Eina_Thread eina_thread_self(void);
EAPI Eina_Bool eina_thread_equal(Eina_Thread t1, Eina_Thread t2);
EAPI Eina_Bool eina_thread_create(Eina_Thread *t,
                                  Eina_Thread_Priority prio, int affinity,
                                  Eina_Thread_Cb func, const void *data);
EAPI void *eina_thread_join(Eina_Thread t);

/**
 * @}
 */

/**
 * @}
 */

#endif
