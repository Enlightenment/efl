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

#ifndef EINA_THREAD_WIN32_H
#define EINA_THREAD_WIN32_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "eina_config.h"
#include "eina_cpu.h"
#include "eina_thread.h"
#include "unimplemented.h"
#include <errno.h>
#include <string.h>
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#define EINA_THREAD_CANCEL_ENABLE 0
#define EINA_THREAD_CANCEL_DISABLE 1
#define EINA_THREAD_CANCEL_DEFERRED 0
#define EINA_THREAD_CANCEL_ASYNCHRONOUS 1
#define EINA_THREAD_CANCELED   ((void *) -1)
/**
 * @typedef Eina_Thread
 * Type for a generic thread.
 */
typedef HANDLE Eina_Thread;

EAPI const void *EINA_THREAD_JOIN_CANCELED = EINA_THREAD_CANCELED;

#endif 