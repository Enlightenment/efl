#ifndef _EFL_EDJE_API_H
#define _EFL_EDJE_API_H

#ifdef EDJE_API
#error EDJE_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EDJE_STATIC
#  ifdef EDJE_BUILD
#   define EDJE_API __declspec(dllexport)
#  else
#   define EDJE_API __declspec(dllimport)
#  endif
# else
#  define EDJE_API
# endif
# define EDJE_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EDJE_API __attribute__ ((visibility("default")))
#   define EDJE_API_WEAK __attribute__ ((weak))
#  else
#   define EDJE_API
#   define EDJE_API_WEAK
#  endif
# else
#  define EDJE_API
#  define EDJE_API_WEAK
# endif
#endif

#endif
