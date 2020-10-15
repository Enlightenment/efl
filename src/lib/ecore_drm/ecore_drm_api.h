#ifndef _EFL_ECORE_DRM_API_H
#define _EFL_ECORE_DRM_API_H

#ifdef ECORE_DRM_API
#error ECORE_DRM_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_DRM_STATIC
#  ifdef ECORE_DRM_BUILD
#   define ECORE_DRM_API __declspec(dllexport)
#  else
#   define ECORE_DRM_API __declspec(dllimport)
#  endif
# else
#  define ECORE_DRM_API
# endif
# define ECORE_DRM_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_DRM_API __attribute__ ((visibility("default")))
#   define ECORE_DRM_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_DRM_API
#   define ECORE_DRM_API_WEAK
#  endif
# else
#  define ECORE_DRM_API
#  define ECORE_DRM_API_WEAK
# endif
#endif
