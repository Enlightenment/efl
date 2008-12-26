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

#ifdef EINA_WARN_UNUSED_RESULT
# undef EINA_WARN_UNUSED_RESULT
#endif
#ifdef EINA_ARG_NONNULL
# undef EINA_ARG_NONNULL
#endif
#ifdef EINA_MALLOC
# undef EINA_MALLOC
#endif
#ifdef EINA_PURE
# undef EINA_PURE
#endif
#ifdef EINA_PRINTF
# undef EINA_PRINTF
#endif
#ifdef EINA_SCANF
# undef EINA_SCANF
#endif
#ifdef EINA_FORMAT
# undef EINA_FORMAT
#endif
#ifdef EINA_CONST
# undef EINA_CONST
#endif
#ifdef EINA_UNLIKELY
# undef EINA_UNLIKELY
#endif
#ifdef EINA_LIKELY
# undef EINA_LIKELY
#endif

#ifdef HAVE___ATTRIBUTE__
# ifdef __GNUC__
#  if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#   define EINA_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#  else
#   define EINA_WARN_UNUSED_RESULT
#  endif

#  if (!defined(EINA_SAFETY_CHECKS)) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#   define EINA_ARG_NONNULL(idx, ...) __attribute__ ((nonnull(idx, ## __VA_ARGS__)))
#  else
#   define EINA_ARG_NONNULL(idx, ...)
#  endif

#  if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#   define EINA_MALLOC __attribute__ ((malloc))
#   define EINA_PURE __attribute__ ((pure))
#  else
#   define EINA_MALLOC
#   define EINA_PURE
#  endif

#  if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#   define EINA_PRINTF(fmt, arg)  __attribute__((format (printf, fmt, arg)))
#   define EINA_SCANF(fmt, arg)  __attribute__((format (scanf, fmt, arg)))
#   define EINA_FORMAT(fmt) __attribute__((format_arg(fmt)))
#   define EINA_CONST __attribute__((const))
#   define EINA_UNLIKELY(exp) __builtin_expect((exp), 0)
#   define EINA_LIKELY(exp) __builtin_expect((exp), 1)
#  else
#   define EINA_PRINTF(fmt, arg)
#   define EINA_SCANF(fmt, arg)
#   define EINA_FORMAT(fmt)
#   define EINA_CONST
#   define EINA_UNLIKELY(exp)
#   define EINA_LIKELY(exp)
#  endif
# else /* no __GNUC__ */
#  define EINA_WARN_UNUSED_RESULT
#  define EINA_ARG_NONNULL(idx, ...)
#  define EINA_MALLOC
#  define EINA_PURE
#  define EINA_PRINTF(fmt, arg)
#  define EINA_SCANF(fmt, arg)
#  define EINA_FORMAT(fmt)
#  define EINA_CONST
#  define EINA_UNLIKELY(exp)
#  define EINA_LIKELY(exp)
# endif
#else /* no HAVE___ATTRIBUTE__ */
# define EINA_WARN_UNUSED_RESULT
# define EINA_ARG_NONNULL(idx, ...)
# define EINA_MALLOC
# define EINA_PURE
# define EINA_PRINTF(fmt, arg)
# define EINA_SCANF(fmt, arg)
# define EINA_FORMAT(fmt)
# define EINA_CONST
# define EINA_UNLIKELY(exp)
# define EINA_LIKELY(exp)
#endif


/* remove this TRUE/FALSE redifinitions */

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

typedef unsigned char Eina_Bool;
#define EINA_FALSE ((Eina_Bool)0)
#define EINA_TRUE ((Eina_Bool)1)

EAPI extern const unsigned int eina_prime_table[];

#define EINA_SORT_MIN 0
#define EINA_SORT_MAX 1

typedef int (*Eina_Compare_Cb) (const void *data1, const void *data2);
#define EINA_COMPARE_CB(function) ((Eina_Compare_Cb)function)

typedef Eina_Bool (*Eina_Each)(const void *container,
			       void *data,
			       void *fdata);
#define EINA_EACH(Function) ((Eina_Each)Function)

typedef void (*Eina_Free_Cb)(void *data);
#define EINA_FREE_CB(Function) ((Eina_Free_Cb)Function)

#endif /* EINA_TYPES_H_ */
