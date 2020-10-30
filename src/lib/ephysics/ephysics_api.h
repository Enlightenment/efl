#ifndef _EFL_EPHYSICS_API_H
#define _EFL_EPHYSICS_API_H

#ifdef EPHYSICS_API
#error EPHYSICS_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EPHYSICS_STATIC
#  ifdef EPHYSICS_BUILD
#   define EPHYSICS_API __declspec(dllexport)
#  else
#   define EPHYSICS_API __declspec(dllimport)
#  endif
# else
#  define EPHYSICS_API
# endif
# define EPHYSICS_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EPHYSICS_API __attribute__ ((visibility("default")))
#   define EPHYSICS_API_WEAK __attribute__ ((weak))
#  else
#   define EPHYSICS_API
#   define EPHYSICS_API_WEAK
#  endif
# else
#  define EPHYSICS_API
  define EPHYSICS_API_WEAK
# endif
#endif

#endif
