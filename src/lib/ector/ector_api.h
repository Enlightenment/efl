#ifndef _EFL_ECTOR_API_H
#define _EFL_ECTOR_API_H

#ifdef ECTOR_API
#error ECTOR_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECTOR_STATIC
#  ifdef ECTOR_BUILD
#   define ECTOR_API __declspec(dllexport)
#  else
#   define ECTOR_API __declspec(dllimport)
#  endif
# else
#  define ECTOR_API
# endif
# define ECTOR_API_WEAK
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define ECTOR_API __attribute__ ((visibility("default")))
#  define ECTOR_API_WEAK __attribute__ ((weak))
# else
#  define ECTOR_API
#  define ECTOR_API_WEAK
# endif
#else
# define ECTOR_API
# define ECTOR_API_WEAK
#endif

#endif
