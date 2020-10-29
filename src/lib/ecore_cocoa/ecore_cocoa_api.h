#ifndef _EFL_ECORE_COCOA_API_H
#define _EFL_ECORE_COCOA_API_H

#ifdef ECORE_COCOA_API
#error ECORE_COCOA_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_COCOA_STATIC
#  ifdef ECORE_COCOA_BUILD
#   define ECORE_COCOA_API __declspec(dllexport)
#  else
#   define ECORE_COCOA_API __declspec(dllimport)
#  endif
# else
#  define ECORE_COCOA_API
# endif
# define ECORE_COCOA_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_COCOA_API __attribute__ ((visibility("default")))
#   define ECORE_COCOA_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_COCOA_API
#   define ECORE_COCOA_API_WEAK
#  endif
# else
#  define ECORE_COCOA_API
#  define ECORE_COCOA_API_WEAK
# endif
#endif

#endif
