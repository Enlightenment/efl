#ifndef _EFL_ECORE_FILE_API_H
#define _EFL_ECORE_FILE_API_H

#ifdef ECORE_FILE_API
#error ECORE_FILE_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_FILE_STATIC
#  ifdef ECORE_FILE_BUILD
#   define ECORE_FILE_API __declspec(dllexport)
#  else
#   define ECORE_FILE_API __declspec(dllimport)
#  endif
# else
#  define ECORE_FILE_API
# endif
# define ECORE_FILE_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_FILE_API __attribute__ ((visibility("default")))
#   define ECORE_FILE_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_FILE_API
#   define ECORE_FILE_API_WEAK
#  endif
# else
#  define ECORE_FILE_API
#  define ECORE_FILE_API_WEAK
# endif
#endif

#endif
