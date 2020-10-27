#ifndef _EFL_ECORE_IMF_API_H
#define _EFL_ECORE_IMF_API_H

#ifdef ECORE_IMF_API
#error ECORE_IMF_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_IMF_STATIC
#  ifdef ECORE_IMF_BUILD
#   define ECORE_IMF_API __declspec(dllexport)
#  else
#   define ECORE_IMF_API __declspec(dllimport)
#  endif
# else
#  define ECORE_IMF_API
# endif
# define ECORE_IMF_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_IMF_API __attribute__ ((visibility("default")))
#   define ECORE_IMF_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_IMF_API
#   define ECORE_IMF_API_WEAK
#  endif
# else
#  define ECORE_IMF_API
#  define ECORE_IMF_API_WEAK
# endif
#endif

#endif
