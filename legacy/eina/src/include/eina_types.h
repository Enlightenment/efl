/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_TYPES_H_
#define EINA_TYPES_H_

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EINA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EINA_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif

/* remove this TRUE/FALSE redifinitions */

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

typedef enum {
	EINA_FALSE = 0,
	EINA_TRUE = 1
} Eina_Bool;

EAPI extern const unsigned int eina_prime_table[];

#define EINA_SORT_MIN 0
#define EINA_SORT_MAX 1

typedef void (*Eina_For_Each) (void *value, void *user_data);
#define EINA_FOR_EACH(function) ((Eina_For_Each)function)

typedef void (*Eina_Free_Cb) (void *data);
#define EINA_FREE_CB(func) ((Eina_Free_Cb)func)

typedef unsigned int (*Eina_Hash_Cb) (const void *key);
#define EINA_HASH_CB(function) ((Eina_Hash_Cb)function)

typedef int (*Eina_Compare_Cb) (const void *data1, const void *data2);
#define EINA_COMPARE_CB(function) ((Eina_Compare_Cb)function)

#endif /*EINA_TYPES_H_*/
