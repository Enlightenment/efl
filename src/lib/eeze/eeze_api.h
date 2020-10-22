#ifndef _EFL_EEZE_API_H
#define _EFL_EEZE_API_H

#ifdef EEZE_API
#error EEZE_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EEZE_STATIC
#  ifdef EEZE_BUILD
#   define EEZE_API __declspec(dllexport)
#  else
#   define EEZE_API __declspec(dllimport)
#  endif
# else
#  define EEZE_API
# endif
# define EEZE_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EEZE_API __attribute__ ((visibility("default")))
#   define EEZE_API_WEAK __attribute__ ((weak))
#  else
#   define EEZE_API
#   define EEZE_API_WEAK
#  endif
# else
#  define EEZE_API
#  define EEZE_API_WEAK
# endif
#endif

#endif
