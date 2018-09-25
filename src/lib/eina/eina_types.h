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

/**
 * @addtogroup Eina_Core_Group Core
 *
 * @{
 */

/**
 * @defgroup Eina_Types_Group Types
 *
 * @{
 */

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
# define EAPI_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK __attribute__ ((weak))
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# else
/**
 * @def EAPI
 * @brief Used to export functions (by changing visibility).
 */
#  define EAPI
#  define EAPI_WEAK
#  endif
#endif

/* Weak symbols part of EFL API - Note: not weak on all platforms */
#define EWAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EXPORTAPI __declspec(dllexport)
# else
#  define EXPORTAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EXPORTAPI __attribute__ ((visibility("default")))
#  else
#   define EXPORTAPI
#  endif
# else
/**
 * @def EAPI
 * @brief Used to export functions (by changing visibility).
 */
#  define EXPORTAPI
# endif
#endif

#include "eina_config.h"

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

#ifdef EINA_UNUSED
# undef EINA_UNUSED
#endif
#ifdef EINA_WARN_UNUSED_RESULT
# undef EINA_WARN_UNUSED_RESULT
#endif
#ifdef EINA_ARG_NONNULL
# undef EINA_ARG_NONNULL
#endif
#ifdef EINA_DEPRECATED
# undef EINA_DEPRECATED
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
#ifdef EINA_NOINSTRUMENT
# undef EINA_NOINSTRUMENT
#endif
#ifdef EINA_UNLIKELY
# undef EINA_UNLIKELY
#endif
#ifdef EINA_LIKELY
# undef EINA_LIKELY
#endif
#ifdef EINA_SENTINEL
# undef EINA_SENTINEL
#endif
#ifdef EINA_FALLTHROUGH
# undef EINA_FALLTHROUGH
#endif
#ifdef EINA_PREFETCH
# undef EINA_PREFETCH
#endif
#ifdef EINA_PREFETCH_WRITE
# undef EINA_PREFETCH_WRITE
#endif
#ifdef EINA_PREFETCH_NOCACHE
# undef EINA_PREFETCH_NOCACHE
#endif
#ifdef EINA_PREFETCH_NOCACHE_WRITE
# undef EINA_PREFETCH_NOCACHE_WRITE
#endif

#ifdef __GNUC__

# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define EINA_UNUSED __attribute__ ((__unused__))
# else
#  define EINA_UNUSED
# endif

# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  define EINA_WARN_UNUSED_RESULT __attribute__ ((__warn_unused_result__))
# else
#  define EINA_WARN_UNUSED_RESULT
# endif

# if (!defined(EINA_SAFETY_CHECKS)) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#  define EINA_ARG_NONNULL(...) __attribute__ ((__nonnull__(__VA_ARGS__)))
# else
#  define EINA_ARG_NONNULL(...)
# endif

# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define EINA_DEPRECATED __attribute__ ((__deprecated__))
# else
#  define EINA_DEPRECATED
# endif

# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#  define EINA_MALLOC __attribute__ ((__malloc__))
#  define EINA_PURE   __attribute__ ((__pure__))
# else
#  define EINA_MALLOC
#  define EINA_PURE
# endif

# if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#  define EINA_HOT __attribute__ ((hot))
#  define EINA_COLD __attribute__ ((cold))
# else
#  define EINA_HOT
#  define EINA_COLD
# endif

# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3)
#   define EINA_PRINTF(fmt, arg) __attribute__((__format__ (__gnu_printf__, fmt, arg)))
#  else
#   define EINA_PRINTF(fmt, arg) __attribute__((__format__ (__printf__, fmt, arg)))
#  endif
#  define EINA_SCANF(fmt, arg)  __attribute__((__format__ (__scanf__, fmt, arg)))
#  define EINA_FORMAT(fmt)      __attribute__((__format_arg__(fmt)))
#  define EINA_CONST        __attribute__((__const__))
#  define EINA_NOINSTRUMENT __attribute__((__no_instrument_function__))
#  define EINA_UNLIKELY(exp)    __builtin_expect((exp), 0)
#  define EINA_LIKELY(exp)      __builtin_expect((exp), 1)
#  define EINA_SENTINEL __attribute__((__sentinel__))
#  ifndef __clang__
#   if __GNUC__ >= 7
#    define EINA_FALLTHROUGH __attribute__ ((fallthrough));
#   else
#    define EINA_FALLTHROUGH
#   endif
#   define EINA_PREFETCH(arg) (arg ? __builtin_prefetch(arg) : (void) arg)
#   define EINA_PREFETCH_WRITE(arg) (arg ? __builtin_prefetch(arg, 1) : (void) arg)
#   define EINA_PREFETCH_NOCACHE(arg) (arg ? __builtin_prefetch(arg, 0, 0) : (void) arg)
#   define EINA_PREFETCH_NOCACHE_WRITE(arg) (arg ? __builtin_prefetch(arg, 1, 0) : (void) arg)
#  else
/* LLVM Clang workaround (crash on compilation) */
#   define EINA_FALLTHROUGH
#   define EINA_PREFETCH(arg) ((void) (arg))
#   define EINA_PREFETCH_WRITE(arg) ((void) (arg))
#   define EINA_PREFETCH_NOCACHE(arg) ((void) (arg))
#   define EINA_PREFETCH_NOCACHE_WRITE(arg) ((void) (arg))
#  endif
# else
#  define EINA_PRINTF(fmt, arg)
#  define EINA_SCANF(fmt, arg)
#  define EINA_FORMAT(fmt)
#  define EINA_CONST
#  define EINA_NOINSTRUMENT
#  define EINA_UNLIKELY(exp) exp
#  define EINA_LIKELY(exp)   exp
#  define EINA_SENTINEL
#  define EINA_FALLTHROUGH
#  define EINA_PREFETCH(arg) ((void) (arg))
#  define EINA_PREFETCH_WRITE(arg) ((void) (arg))
#  define EINA_PREFETCH_NOCACHE(arg) ((void) (arg))
#  define EINA_PREFETCH_NOCACHE_WRITE(arg) ((void) (arg))
# endif

#elif defined(_MSC_VER)
# define EINA_UNUSED
# define EINA_WARN_UNUSED_RESULT
# define EINA_ARG_NONNULL(...)
# if _MSC_VER >= 1300
#  define EINA_DEPRECATED __declspec(deprecated)
# else
#  define EINA_DEPRECATED
# endif
# define EINA_MALLOC
# define EINA_PURE
# define EINA_PRINTF(fmt, arg)
# define EINA_SCANF(fmt, arg)
# define EINA_FORMAT(fmt)
# define EINA_CONST
# define EINA_NOINSTRUMENT
# define EINA_UNLIKELY(exp) exp
# define EINA_LIKELY(exp)   exp
# define EINA_SENTINEL
# define EINA_FALLTHROUGH
# define EINA_PREFETCH(arg) ((void) (arg))
# define EINA_PREFETCH_WRITE(arg) ((void) (arg))
# define EINA_PREFETCH_NOCACHE(arg) ((void) (arg))
# define EINA_PREFETCH_NOCACHE_WRITE(arg) ((void) (arg))

#elif defined(__SUNPRO_C)
# define EINA_UNUSED
# define EINA_WARN_UNUSED_RESULT
# define EINA_ARG_NONNULL(...)
# define EINA_DEPRECATED
# if __SUNPRO_C >= 0x590
#  define EINA_MALLOC __attribute__ ((malloc))
#  define EINA_PURE   __attribute__ ((pure))
# else
#  define EINA_MALLOC
#  define EINA_PURE
# endif
# define EINA_PRINTF(fmt, arg)
# define EINA_SCANF(fmt, arg)
# define EINA_FORMAT(fmt)
# if __SUNPRO_C >= 0x590
#  define EINA_CONST __attribute__ ((const))
# else
#  define EINA_CONST
# endif
# define EINA_NOINSTRUMENT
# define EINA_UNLIKELY(exp) exp
# define EINA_LIKELY(exp)   exp
# define EINA_SENTINEL
# define EINA_FALLTHROUGH
# define EINA_PREFETCH(arg) ((void) (arg))
# define EINA_PREFETCH_WRITE(arg) ((void) (arg))
# define EINA_PREFETCH_NOCACHE(arg) ((void) (arg))
# define EINA_PREFETCH_NOCACHE_WRITE(arg) ((void) (arg))

#else /* ! __GNUC__ && ! _MSC_VER && ! __SUNPRO_C */

/**
 * @def EINA_UNUSED
 * Used to warn when an argument of the function is not used.
 */
# define EINA_UNUSED

/**
 * @def EINA_WARN_UNUSED_RESULT
 * Used to warn when the returned value of the function is not used.
 */
# define EINA_WARN_UNUSED_RESULT

/**
 * @def EINA_ARG_NONNULL
 * Used to warn when the specified arguments of the function are @c NULL.
 *
 * @param[in] ... Ordinals of the parameters to check for nullity (1..n)
 *
 * @return Nothing, but Doxygen will complain if it's not documented :-P
 *
 */
# define EINA_ARG_NONNULL(...)

/**
 * @def EINA_DEPRECATED
 * Used to warn when the function is considered as deprecated.
 */
# define EINA_DEPRECATED

/**
 * @def EINA_MALLOC
 * @brief EINA_MALLOC is used to tell the compiler that a function may be treated
 * as if any non-NULL pointer it returns cannot alias any other pointer valid when
 * the function returns and that the memory has undefined content.
 */
# define EINA_MALLOC

/**
 * @def EINA_PURE
 * @brief EINA_PURE is used to tell the compiler this function has no effect
 * except the return value and its return value depends only on the parameters
 * and/or global variables.
 */
# define EINA_PURE

/**
 * @def EINA_PRINTF
 * @param[in] fmt The format to be used.
 * @param[in] arg The argument to be used.
 */
# define EINA_PRINTF(fmt, arg)

/**
 * @def EINA_SCANF
 * @param[in] fmt The format to be used.
 * @param[in] arg The argument to be used.
 */
# define EINA_SCANF(fmt, arg)

/**
 * @def EINA_FORMAT
 * @param[in] fmt The format to be used.
 */
# define EINA_FORMAT(fmt)

/**
 * @def EINA_CONST
 * @brief Attribute from gcc to prevent the function to read/modify any global memory.
 */
# define EINA_CONST

/**
 * @def EINA_NOINSTRUMENT
 * @brief Attribute from gcc to disable instrumentation for a specific function.
 */
# define EINA_NOINSTRUMENT

/**
 * @def EINA_UNLIKELY
 * @param[in] exp The expression to be used.
 */
# define EINA_UNLIKELY(exp) exp

/**
 * @def EINA_LIKELY
 * @param[in] exp The expression to be used.
 */
# define EINA_LIKELY(exp)   exp

/**
 * @def EINA_SENTINEL
 * @brief Attribute from gcc to prevent calls without the necessary NULL
 * sentinel in certain variadic functions
 *
 * @since 1.7
 */
# define EINA_SENTINEL

/**
 * @def EINA_FALLTHROUGH
 * @brief Attribute from gcc to prevent warning and indicate that we expect
 * to actually go to the next switch statement
 *
 * @since 1.20
 */
# define EINA_FALLTHROUGH

/**
 * @def EINA_PREFETCH
 * @brief Hints that the pointer @parg needs to be pre-fetched into cache
 *
 * This hints to the compiler to probably issue a prefetch command for the
 * memory address @p arg and ensure it goes into all levels of cache. For
 * just writing to an address look at EINA_PREFETCH_WRITE().
 *
 * Note that the pointer @p arg does not have to be a valid pointer and
 * will not cause any exceptions (like segfaults) if it is invalid.
 *
 * @since 1.19
 */
# define EINA_PREFETCH(arg)

/**
 * @def EINA_PREFETCH_WRITE
 * @brief Hints that the pointer @parg needs to be pre-fetched into cache
 *
 * This hints to the compiler to probably issue a prefetch command for the
 * memory address @p arg and ensure it goes into all levels of cache. This
 * specifically indicates that the address is going to be written to as
 * opposed to being read from as with EINA_PREFETCH().
 *
 * Note that the pointer @p arg does not have to be a valid pointer and
 * will not cause any exceptions (like segfaults) if it is invalid.
 *
 * @since 1.19
 */
# define EINA_PREFETCH_WRITE(arg)

/**
 * @def EINA_PREFETCH_NOCACHE
 * @brief Hints that the pointer @parg needs to be pre-fetched into cache
 *
 * This hints to the compiler to probably issue a prefetch command for the
 * memory address @p arg and ensure it goes into just the closest(l1) cache.
 * For just writing to an address look at EINA_PREFETCH_WRITE_NOCACHE().
 *
 * Note that the pointer @p arg does not have to be a valid pointer and
 * will not cause any exceptions (like segfaults) if it is invalid.
 *
 * @since 1.19
 */
# define EINA_PREFETCH_NOCACHE(arg)

/**
 * @def EINA_PREFETCH_WRITE_NOCACHE
 *
 * @brief Hints that the pointer @parg needs to be pre-fetched into cache
 *
 * This hints to the compiler to probably issue a prefetch command for the
 * memory address @p arg and ensure it goes into just the closest(l1) cache.
 * This specifically indicates that the address is going to be written to as
 * opposed to being read from as with EINA_PREFETCH_NOCACHE().
 * Note that the pointer @p arg does not have to be a valid pointer and
 * will not cause any exceptions (like segfaults) if it is invalid.
 *
 * @since 1.19
 */
# define EINA_PREFETCH_NOCACHE_WRITE(arg)

#endif /* ! __GNUC__ && ! _WIN32 && ! __SUNPRO_C */

/**
 * @typedef Eina_Bool
 * Type to mimic a boolean.
 *
 * @note it differs from stdbool.h as this is defined as an unsigned
 *       char to make it usable by bitfields (Eina_Bool name:1) and
 *       also take as few bytes as possible.
 */
typedef unsigned char Eina_Bool;

/**
 * @def EINA_FALSE
 * boolean value FALSE (numerical value 0)
 */
#define EINA_FALSE ((Eina_Bool)0)

/**
 * @def EINA_TRUE
 * boolean value TRUE (numerical value 1)
 */
#define EINA_TRUE  ((Eina_Bool)1)

EAPI extern const unsigned int eina_prime_table[];

/**
 * @typedef Eina_Compare_Cb
 * Function used in functions using sorting. It compares @p data1 and
 * @p data2. If @p data1 is 'less' than @p data2, -1 must be returned,
 * if it is 'greater', 1 must be returned, and if they are equal, 0
 * must be returned.
 */
typedef int (*Eina_Compare_Cb)(const void *data1, const void *data2);

/**
 * @def EINA_COMPARE_CB
 * Macro to cast to Eina_Compare_Cb.
 */
#define EINA_COMPARE_CB(function) ((Eina_Compare_Cb)function)

/**
 * @typedef Eina_Random_Cb
 * Function used in shuffling functions. An integer between min and max
 * inclusive must be returned.
 *
 * @since 1.8
 */
typedef int (*Eina_Random_Cb)(const int min, const int max);

/**
 * @def EINA_RANDOM_CB
 * Macro to cast to Eina_Random_Cb.
 */
#define EINA_RANDOM_CB(function) ((Eina_Random_Cb)function)

/**
 * @typedef Eina_Each_Cb
 * A callback type used when iterating over a container.
 */
typedef Eina_Bool (*Eina_Each_Cb)(const void *container, void *data, void *fdata);

/**
 * @def EINA_EACH_CB
 * Macro to cast to Eina_Each.
 */
#define EINA_EACH_CB(Function) ((Eina_Each_Cb)Function)

/**
 * @typedef Eina_Free_Cb
 * A callback type used to free data when iterating over a container.
 */
typedef void (*Eina_Free_Cb)(void *data);

/**
 * @def EINA_FREE_CB
 * Macro to cast to Eina_Free_Cb.
 */
#define EINA_FREE_CB(Function) ((Eina_Free_Cb)Function)

/**
 * @def EINA_C_ARRAY_LENGTH
 * Macro to return the array length of a standard c array.
 *
 * For example:
 * int foo[] = { 0, 1, 2, 3 };
 * would return 4 and not 4 * sizeof(int).
 *
 * @since 1.2.0
 */
#define EINA_C_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * @def EINA_DOUBLE_EQUAL
 * Macro to compare 2 double floating point values and deal with precision
 * loss issues.
 *
 * @since 1.18
 */
#define EINA_DOUBLE_EQUAL(x, y) \
   (fabs((x) - (y)) <= (2.2204460492503131e-16) * fabs((x)))

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_TYPES_H_ */
