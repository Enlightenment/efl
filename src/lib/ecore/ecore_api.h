#ifndef _EFL_CORE_API_H
#define _EFL_CORE_API_H

#ifdef ECORE_API
#error ECORE_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_STATIC
#  ifdef ECORE_BUILD
#   define ECORE_API __declspec(dllexport)
#  else
#   define ECORE_API __declspec(dllimport)
#  endif
# else
#  define ECORE_API
# endif
# define ECORE_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_API __attribute__ ((visibility("default")))
#   define ECORE_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_API
#   define ECORE_API_WEAK
#  endif
# else
#  define ECORE_API
#  define ECORE_API_WEAK
# endif
#endif

#endif
