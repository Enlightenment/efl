#ifndef _EFL_EET_API_H
#define _EFL_EET_API_H

#ifdef EET_API
#error EET_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EET_STATIC
#  ifdef EET_BUILD
#   define EET_API __declspec(dllexport)
#  else
#   define EET_API __declspec(dllimport)
#  endif
# else
#  define EET_API
# endif
# define EET_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EET_API __attribute__ ((visibility("default")))
#   define EET_API_WEAK __attribute__ ((weak))
#  else
#   define EET_API
#   define EET_API_WEAK
#  endif
# else
#  define EET_API
#  define EET_API_WEAK
# endif
#endif

#endif
