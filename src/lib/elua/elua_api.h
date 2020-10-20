#ifndef _EFL_ELUA_API_H
#define _EFL_ELUA_API_H

#ifdef ELUA_API
#error ELUA_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ELUA_STATIC
#  ifdef ELUA_BUILD
#   define ELUA_API __declspec(dllexport)
#  else
#   define ELUA_API __declspec(dllimport)
#  endif
# else
#  define ELUA_API
# endif
# define ELUA_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ELUA_API __attribute__ ((visibility("default")))
#   define ELUA_API_WEAK __attribute__ ((weak))
#  else
#   define ELUA_API
#   define ELUA_API_WEAK
#  endif
# else
#  define ELUA_API
#  define ELUA_API_WEAK
# endif
#endif

#endif
