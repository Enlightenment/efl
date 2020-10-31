#ifndef _EFL_ECORE_WAYLAND_API_H
#define _EFL_ECORE_WAYLAND_API_H

#ifdef ECORE_WAYLAND_API
#error ECORE_WAYLAND_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_WAYLAND_STATIC
#  ifdef ECORE_WAYLAND_BUILD
#   define ECORE_WAYLAND_API __declspec(dllexport)
#  else
#   define ECORE_WAYLAND_API __declspec(dllimport)
#  endif
# else
#  define ECORE_WAYLAND_API
# endif
# define ECORE_WAYLAND_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_WAYLAND_API __attribute__ ((visibility("default")))
#   define ECORE_WAYLAND_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_WAYLAND_API
#   define ECORE_WAYLAND_API_WEAK
#  endif
# else
#  define ECORE_WAYLAND_API
#  define ECORE_WAYLAND_API_WEAK
# endif
#endif

#endif
