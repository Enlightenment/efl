#ifndef _EFL_ECORE_WIN32_API_H
#define _EFL_ECORE_WIN32_API_H

#ifdef ECORE_WIN32_API
#error ECORE_WIN32_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_WIN32_STATIC
#  ifdef ECORE_WIN32_BUILD
#   define ECORE_WIN32_API __declspec(dllexport)
#  else
#   define ECORE_WIN32_API __declspec(dllimport)
#  endif
# else
#  define ECORE_WIN32_API
# endif
# define ECORE_WIN32_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_WIN32_API __attribute__ ((visibility("default")))
#   define ECORE_WIN32_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_WIN32_API
#   define ECORE_WIN32_API_WEAK
#  endif
# else
#  define ECORE_WIN32_API
#  define ECORE_WIN32_API_WEAK
# endif
#endif

#endif
