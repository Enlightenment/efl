#ifndef _EFL_ECORE_X_API_H
#define _EFL_ECORE_X_API_H

#ifdef ECORE_X_API
#error ECORE_X_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_X_STATIC
#  ifdef ECORE_X_BUILD
#   define ECORE_X_API __declspec(dllexport)
#  else
#   define ECORE_X_API __declspec(dllimport)
#  endif
# else
#  define ECORE_X_API
# endif
# define ECORE_X_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_X_API __attribute__ ((visibility("default")))
#   define ECORE_X_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_X_API
#   define ECORE_X_API_WEAK
#  endif
# else
#  define ECORE_X_API
#  define ECORE_X_API_WEAK
# endif
#endif

#endif
