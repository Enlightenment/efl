/* EINA - EFL data type library
 * Copyright (C) 2013 Gustavo Sverzut Barbieri
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

#ifndef EINA_ALLOCA_H
#define EINA_ALLOCA_H

/**
 * @file eina_alloca.h
 * simplifies usage of alloca() by including the proper file
 * depending on the platform.
 */

#ifdef EINA_HAVE_ALLOCA_H
# include <alloca.h>
#elif !defined alloca
# ifdef __GNUC__
#  define alloca __builtin_alloca
# elif defined _AIX
#  define alloca __alloca
# elif defined _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# elif !defined EINA_HAVE_ALLOCA
#  ifdef  __cplusplus
extern "C"
#  endif
void *alloca (long);
# endif
#endif

#endif /* EINA_HAVE_ALLOCA_H */
