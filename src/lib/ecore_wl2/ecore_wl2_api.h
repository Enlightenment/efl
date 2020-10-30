#ifndef _EFL_ECORE_WL2_API_H
#define _EFL_ECORE_WL2_API_H

#ifdef ECORE_WL2_API
#error ECORE_WL2_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_WL2_STATIC
#  ifdef ECORE_WL2_BUILD
#   define ECORE_WL2_API __declspec(dllexport)
#  else
#   define ECORE_WL2_API __declspec(dllimport)
#  endif
# else
#  define ECORE_WL2_API
# endif
# define ECORE_WL2_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_WL2_API __attribute__ ((visibility("default")))
#   define ECORE_WL2_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_WL2_API
#   define ECORE_WL2_API_WEAK
#  endif
# else
#  define ECORE_WL2_API
#  define ECORE_WL2_API_WEAK
# endif
#endif

#endif
