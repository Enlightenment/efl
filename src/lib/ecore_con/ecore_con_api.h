#ifndef _EFL_ECORE_CON_API_H
#define _EFL_ECORE_CON_API_H

#ifdef ECORE_CON_API
#error ECORE_CON_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_CON_STATIC
#  ifdef ECORE_CON_BUILD
#   define ECORE_CON_API __declspec(dllexport)
#  else
#   define ECORE_CON_API __declspec(dllimport)
#  endif
# else
#  define ECORE_CON_API
# endif
# define ECORE_CON_API_WEAK
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define ECORE_CON_API __attribute__ ((visibility("default")))
#  define ECORE_CON_API_WEAK __attribute__ ((weak))
# else
#  define ECORE_CON_API
#  define ECORE_CON_API_WEAK
# endif
#else
# define ECORE_CON_API
# define ECORE_CON_API_WEAK
#endif

#endif
