#ifndef _EFL_ELPUT_API_H
#define _EFL_ELPUT_API_H

#ifdef ELPUT_API
#error ELPUT_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ELPUT_STATIC
#  ifdef ELPUT_BUILD
#   define ELPUT_API __declspec(dllexport)
#  else
#   define ELPUT_API __declspec(dllimport)
#  endif
# else
#  define ELPUT_API
# endif
# define ELPUT_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ELPUT_API __attribute__ ((visibility("default")))
#   define ELPUT_API_WEAK __attribute__ ((weak))
#  else
#   define ELPUT_API
#   define ELPUT_API_WEAK
#  endif
# else
#  define ELPUT_API
#  define ELPUT_API_WEAK
# endif
#endif

#endif
