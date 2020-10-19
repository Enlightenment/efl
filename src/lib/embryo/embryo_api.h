#ifndef _EFL_EMBRYO_API_H
#define _EFL_EMBRYO_API_H

#ifdef EMBRYO_API
#error EMBRYO_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EMBRYO_STATIC
#  ifdef EMBRYO_BUILD
#   define EMBRYO_API __declspec(dllexport)
#  else
#   define EMBRYO_API __declspec(dllimport)
#  endif
# else
#  define EMBRYO_API
# endif
# define EMBRYO_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMBRYO_API __attribute__ ((visibility("default")))
#   define EMBRYO_API_WEAK __attribute__ ((weak))
#  else
#   define EMBRYO_API
#   define EMBRYO_API_WEAK
#  endif
# else
#  define EMBRYO_API
#  define EMBRYO_API_WEAK
# endif
#endif

#endif
