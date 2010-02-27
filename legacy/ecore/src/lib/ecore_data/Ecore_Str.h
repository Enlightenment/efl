#ifndef _ECORE_STR_H
# define _ECORE_STR_H

#include <Eina.h>
#warning "this file is deprecated. use Eina.h instead."

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
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
#endif /* ! _WIN32 */

/* we need this for size_t */
#include <stddef.h>
#include <string.h>

/**
 * @file Ecore_Str.h
 * @brief Contains useful C string functions.
 *
 * @deprecated use Eina.h instead
 */

# ifdef __cplusplus
extern "C" {
# endif

# ifdef __sgi
#  define __FUNCTION__ "unknown"
#  ifndef __cplusplus
#   define inline
#  endif
# endif


/* strlcpy implementation for libc's lacking it */
EAPI size_t ecore_strlcpy(char *dst, const char *src, size_t siz) EINA_DEPRECATED;
EAPI size_t ecore_strlcat(char *dst, const char *src, size_t siz) EINA_DEPRECATED;

EAPI int ecore_str_has_prefix(const char *str, const char *prefix) EINA_DEPRECATED;

EAPI int ecore_str_has_suffix(const char *str, const char *suffix) EINA_DEPRECATED;
EAPI int ecore_str_has_extension(const char *str, const char *ext) EINA_DEPRECATED;

EAPI char **ecore_str_split(const char *string, const char *delimiter, int max_tokens) EINA_DEPRECATED;

EAPI size_t ecore_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len) EINA_DEPRECATED;


static inline size_t ecore_str_join(char *dst, size_t size, char sep, const char *a, const char *b) EINA_DEPRECATED;

/**
 * Join two strings and store the result in @a dst buffer.
 *
 * Similar to ecore_str_join_len(), but will compute the length of @a
 * and @a b using strlen().
 *
 * @param dst where to store the result.
 * @param size byte size of dst, will write at most (size - 1)
 *     characters and then the '\0' (null terminator).
 * @param sep separator character to use.
 * @param a first string to use, before @a sep.
 * @param b second string to use, after @a sep.
 *
 * @return the number of characters printed (not including the
 *     trailing '\0' used to end output to strings). Just like
 *     snprintf(), it will not write more than @a size bytes, thus a
 *     return value of @a size or more means that the output was
 *     truncated.
 *
 * @see ecore_str_join_len() and ecore_str_join_static()
 */
static inline size_t ecore_str_join(char *dst, size_t size, char sep, const char *a, const char *b)
{
   return ecore_str_join_len(dst, size, sep, a, strlen(a), b, strlen(b));
}

/**
 * Join two static strings and store the result in static @a dst buffer.
 *
 * Similar to ecore_str_join_len(), but will assume string sizes are
 * know using sizeof(X).
 *
 * @param dst where to store the result.
 * @param sep separator character to use.
 * @param a first string to use, before @a sep.
 * @param b second string to use, after @a sep.
 *
 * @return the number of characters printed (not including the
 *     trailing '\0' used to end output to strings). Just like
 *     snprintf(), it will not write more than @a size bytes, thus a
 *     return value of @a size or more means that the output was
 *     truncated.
 *
 * @see ecore_str_join() and ecore_str_join_static()
 */
#define ecore_str_join_static(dst, sep, a, b) ecore_str_join_len(dst, sizeof(dst), sep, a, (sizeof(a) > 0) ? sizeof(a) - 1 : 0, b, (sizeof(b) > 0) ? sizeof(b) - 1 : 0)


#ifdef __cplusplus
}
#endif

#endif				/* _ECORE_STR_H */
