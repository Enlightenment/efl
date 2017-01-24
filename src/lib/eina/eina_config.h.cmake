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

#ifndef EINA_CONFIG_H_
#define EINA_CONFIG_H_

#ifdef HAVE_EXOTIC
# include <Exotic.h>
#endif

#include "eina_config_gen.h"

#include <limits.h>

#ifndef __WORDSIZE
# if defined(__OPENBSD__) && (INTPTR_MAX == INT32_MAX)
#  define __WORDSIZE 32
# else
#  define __WORDSIZE 64
# endif
#endif

/* Do not turn the following #define as meaning EFL64. We are only
   interested to know if sizeof (void*) == 64bits or not. Those means
   something else.

   defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
 */
#if defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(__ppc64__)
# define EFL64
#endif

#endif /* EINA_CONFIG_H_ */
