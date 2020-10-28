#ifndef _EFL_ECORE_FB_API_H
#define _EFL_ECORE_FB_API_H

#ifdef ECORE_FB_API
#error ECORE_FB_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_FB_STATIC
#  ifdef ECORE_FB_BUILD
#   define ECORE_FB_API __declspec(dllexport)
#  else
#   define ECORE_FB_API __declspec(dllimport)
#  endif
# else
#  define ECORE_FB_API
# endif
# define ECORE_FB_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_FB_API __attribute__ ((visibility("default")))
#   define ECORE_FB_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_FB_API
#   define ECORE_FB_API_WEAK
#  endif
# else
#  define ECORE_FB_API
#  define ECORE_FB_API_WEAK
# endif
#endif

#endif
