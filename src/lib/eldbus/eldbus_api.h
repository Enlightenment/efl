#ifndef _EFL_ELDBUS_API_H
#define _EFL_ELDBUS_API_H

#ifdef ELDBUS_API
#error ELDBUS_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ELDBUS_STATIC
#  ifdef ELDBUS_BUILD
#   define ELDBUS_API __declspec(dllexport)
#  else
#   define ELDBUS_API __declspec(dllimport)
#  endif
# else
#  define ELDBUS_API
# endif
# define ELDBUS_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ELDBUS_API __attribute__ ((visibility("default")))
#   define ELDBUS_API_WEAK __attribute__ ((weak))
#  else
#   define ELDBUS_API
#   define ELDBUS_API_WEAK
#  endif
# else
#  define ELDBUS_API
#  define ELDBUS_API_WEAK
# endif
#endif

#endif
