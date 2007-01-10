#ifndef _ECORE_STR_H
# define _ECORE_STR_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
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

/**
 * @file Ecore_Data.h
 * @brief Contains threading, list, hash, debugging and tree functions.
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
   EAPI size_t ecore_strlcpy(char *dst, const char *src, size_t siz);

#ifdef __cplusplus
}
#endif
#endif				/* _ECORE_STR_H */
