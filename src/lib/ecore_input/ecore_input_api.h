#ifndef _EFL_ECORE_INPUT_API_H
#define _EFL_ECORE_INPUT_API_H

#ifdef ECORE_INPUT_API
#error ECORE_INPUT_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_INPUT_STATIC
#  ifdef ECORE_INPUT_BUILD
#   define ECORE_INPUT_API __declspec(dllexport)
#  else
#   define ECORE_INPUT_API __declspec(dllimport)
#  endif
# else
#  define ECORE_INPUT_API
# endif
# define ECORE_INPUT_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_INPUT_API __attribute__ ((visibility("default")))
#   define ECORE_INPUT_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_INPUT_API
#   define ECORE_INPUT_API_WEAK
#  endif
# else
#  define ECORE_INPUT_API
#  define ECORE_INPUT_API_WEAK
# endif
#endif

#endif
