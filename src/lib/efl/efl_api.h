#ifndef _EFL_EFL_API_H
#define _EFL_EFL_API_H

#ifdef EFL_API
#error EFL_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EFL_STATIC
#  ifdef EFL_BUILD
#   define EFL_API __declspec(dllexport)
#  else
#   define EFL_API __declspec(dllimport)
#  endif
# else
#  define EFL_API
# endif
# define EFL_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EFL_API __attribute__ ((visibility("default")))
#   define EFL_API_WEAK __attribute__ ((weak))
#  else
#   define EFL_API
#   define EFL_API_WEAK
#  endif
# else
#  define EFL_API
#  define EFL_API_WEAK
# endif
#endif

#endif
