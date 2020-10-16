#ifndef _EFL_ECORE_AVAHI_API_H
#define _EFL_ECORE_AVAHI_API_H

#ifdef ECORE_AVAHI_API
#error ECORE_AVAHI_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_AVAHI_STATIC
#  ifdef ECORE_AVAHI_BUILD
#   define ECORE_AVAHI_API __declspec(dllexport)
#  else
#   define ECORE_AVAHI_API __declspec(dllimport)
#  endif
# else
#  define ECORE_AVAHI_API
# endif
# define ECORE_AVAHI_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_AVAHI_API __attribute__ ((visibility("default")))
#   define ECORE_AVAHI_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_AVAHI_API
#   define ECORE_AVAHI_API_WEAK
#  endif
# else
#  define ECORE_AVAHI_API
#  define ECORE_AVAHI_API_WEAK
# endif
#endif

#endif
