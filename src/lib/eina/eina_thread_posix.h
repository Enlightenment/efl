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

#ifndef EINA_THREAD_POSIX_H
#define EINA_THREAD_POSIX_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_thread.h"
#include <pthread.h>
#include "eina_safety_checks.h"
#include <signal.h>

#define EINA_THREAD_CANCEL_ENABLE PTHREAD_CANCEL_ENABLE
#define EINA_THREAD_CANCEL_DISABLE PTHREAD_CANCEL_DISABLE
#define EINA_THREAD_CANCEL_DEFERRED PTHREAD_CANCEL_DEFERRED
#define EINA_THREAD_CANCEL_ASYNCHRONOUS PTHREAD_CANCEL_ASYNCHRONOUS
#define EINA_THREAD_CANCELED   PTHREAD_CANCELED

#if defined(EINA_HAVE_PTHREAD_AFFINITY) || defined(EINA_HAVE_THREAD_SETNAME)
# ifndef __linux__
#  include <pthread_np.h>
#  define cpu_set_t cpuset_t
# endif
#endif
typedef uintptr_t Eina_Thread;
/**
 * @typedef Eina_Thread
 * Type for a generic thread.
 */

#endif