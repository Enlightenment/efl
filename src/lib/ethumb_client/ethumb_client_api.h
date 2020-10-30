#ifndef _EFL_ETHUMB_CLIENT_API_H
#define _EFL_ETHUMB_CLIENT_API_H

#ifdef ETHUMB_CLIENT_API
#error ETHUMB_CLIENT_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ETHUMB_CLIENT_STATIC
#  ifdef ETHUMB_CLIENT_BUILD
#   define ETHUMB_CLIENT_API __declspec(dllexport)
#  else
#   define ETHUMB_CLIENT_API __declspec(dllimport)
#  endif
# else
#  define ETHUMB_CLIENT_API
# endif
# define ETHUMB_CLIENT_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ETHUMB_CLIENT_API __attribute__ ((visibility("default")))
#   define ETHUMB_CLIENT_API_WEAK __attribute__ ((weak))
#  else
#   define ETHUMB_CLIENT_API
#   define ETHUMB_CLIENT_API_WEAK
#  endif
# else
#  define ETHUMB_CLIENT_API
#  define ETHUMB_CLIENT_API_WEAK
# endif
#endif

#endif
