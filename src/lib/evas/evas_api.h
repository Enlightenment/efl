#ifndef _EFL_EVAS_API_H
#define _EFL_EVAS_API_H

#ifdef EVAS_API
#error EVAS_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EVAS_STATIC
#  ifdef EVAS_BUILD
#   define EVAS_API __declspec(dllexport)
#  else
#   define EVAS_API __declspec(dllimport)
#  endif
# else
#  define EVAS_API
# endif
# define EVAS_API_WEAK
#elif __GNUC__
# if __GNUC__ >= 4
#  define EVAS_API __attribute__ ((visibility("default")))
#  define EVAS_API_WEAK __attribute__ ((weak))
# else
#  define EVAS_API
#  define EVAS_API_WEAK
# endif
#else
# define EVAS_API
# define EVAS_API_WEAK
#endif

#endif
