#ifndef _EFL_EOLIAN_API_H
#define _EFL_EOLIAN_API_H

#ifdef EOLIAN_API
#error EOLIAN_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EOLIAN_STATIC
#  ifdef EOLIAN_BUILD
#   define EOLIAN_API __declspec(dllexport)
#  else
#   define EOLIAN_API __declspec(dllimport)
#  endif
# else
#  define EOLIAN_API
# endif
# define EOLIAN_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EOLIAN_API __attribute__ ((visibility("default")))
#   define EOLIAN_API_WEAK __attribute__ ((weak))
#  else
#   define EOLIAN_API
#   define EOLIAN_API_WEAK
#  endif
# else
#  define EOLIAN_API
#  define EOLIAN_API_WEAK
# endif
#endif

#endif
