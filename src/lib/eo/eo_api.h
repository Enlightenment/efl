#ifndef _EFL_EO_API_H
#define _EFL_EO_API_H

#ifdef EO_API
#error EO_API should not be already defined
#endif

#define EOLIAN

#ifdef _WIN32
# ifndef EO_STATIC
#  ifdef EO_BUILD
#   define EO_API __declspec(dllexport)
#  else
#   define EO_API __declspec(dllimport)
#  endif
# else
#  define EO_API
# endif
# define EO_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EO_API __attribute__ ((visibility("default")))
#   define EO_API_WEAK __attribute__ ((weak))
#  else
#   define EO_API
#   define EO_API_WEAK
#  endif
# else
#  define EO_API
#  define EO_API_WEAK
# endif
#endif

#endif
