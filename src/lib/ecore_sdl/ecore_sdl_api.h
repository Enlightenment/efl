#ifndef _EFL_ECORE_SDL_API_H
#define _EFL_ECORE_SDL_API_H

#ifdef ECORE_SDL_API
#error ECORE_SDL_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_SDL_STATIC
#  ifdef ECORE_SDL_BUILD
#   define ECORE_SDL_API __declspec(dllexport)
#  else
#   define ECORE_SDL_API __declspec(dllimport)
#  endif
# else
#  define ECORE_SDL_API
# endif
# define ECORE_SDL_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_SDL_API __attribute__ ((visibility("default")))
#   define ECORE_SDL_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_SDL_API
#   define ECORE_SDL_API_WEAK
#  endif
# else
#  define ECORE_SDL_API
#  define ECORE_SDL_API_WEAK
# endif
#endif

#endif
