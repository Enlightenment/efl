#ifndef _EFL_EFREET_API_H
#define _EFL_EFREET_API_H

#ifdef EFREET_API
#error EFREET_API should not be already defined
#endif

#ifdef EFL_BUILD
# define EFREET_DEPRECATED_API
#else
# define EFREET_DEPRECATED_API EINA_DEPRECATED
#endif

#ifdef _WIN32
# ifndef EFREET_STATIC
#  ifdef EFREET_BUILD
#   define EFREET_API __declspec(dllexport)
#  else
#   define EFREET_API __declspec(dllimport)
#  endif
# else
#  define EFREET_API
# endif
# define EFREET_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EFREET_API __attribute__ ((visibility("default")))
#   define EFREET_API_WEAK __attribute__ ((weak))
#  else
#   define EFREET_API
#   define EFREET_API_WEAK
#  endif
# else
#  define EFREET_API
#  define EFREET_API_WEAK
# endif
#endif

#endif
