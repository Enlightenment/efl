#ifndef _EFL_EMILE_API_H
#define _EFL_EMILE_API_H

#ifdef EMILE_API
#error EMILE_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EMILE_STATIC
#  ifdef EMILE_BUILD
#   define EMILE_API __declspec(dllexport)
#  else
#   define EMILE_API __declspec(dllimport)
#  endif
# else
#  define EMILE_API
# endif
# define EMILE_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EMILE_API __attribute__ ((visibility("default")))
#   define EMILE_API_WEAK __attribute__ ((weak))
#  else
#   define EMILE_API
#   define EMILE_API_WEAK
#  endif
# else
#  define EMILE_API
#  define EMILE_API_WEAK
# endif
#endif

#endif
