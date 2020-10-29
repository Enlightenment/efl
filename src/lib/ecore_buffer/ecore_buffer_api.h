#ifndef _EFL_ECORE_BUFFER_API_H
#define _EFL_ECORE_BUFFER_API_H

#ifdef ECORE_BUFFER_API
#error ECORE_BUFFER_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_BUFFER_STATIC
#  ifdef ECORE_BUFFER_BUILD
#   define ECORE_BUFFER_API __declspec(dllexport)
#  else
#   define ECORE_BUFFER_API __declspec(dllimport)
#  endif
# else
#  define ECORE_BUFFER_API
# endif
# define ECORE_BUFFER_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_BUFFER_API __attribute__ ((visibility("default")))
#   define ECORE_BUFFER_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_BUFFER_API
#   define ECORE_BUFFER_API_WEAK
#  endif
# else
#  define ECORE_BUFFER_API
#  define ECORE_BUFFER_API_WEAK
# endif
#endif

#endif
