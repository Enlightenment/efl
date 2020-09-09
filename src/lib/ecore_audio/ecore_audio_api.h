#ifndef _EFL_ECORE_AUDIO_API_H
#define _EFL_ECORE_AUDIO_API_H

#ifdef ECORE_AUDIO_API
#error ECORE_AUDIO_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ECORE_AUDIO_STATIC
#  ifdef ECORE_AUDIO_BUILD
#   define ECORE_AUDIO_API __declspec(dllexport)
#  else
#   define ECORE_AUDIO_API __declspec(dllimport)
#  endif
# else
#  define ECORE_AUDIO_API
# endif
# define ECORE_AUDIO_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ECORE_AUDIO_API __attribute__ ((visibility("default")))
#   define ECORE_AUDIO_API_WEAK __attribute__ ((weak))
#  else
#   define ECORE_AUDIO_API
#   define ECORE_AUDIO_API_WEAK
#  endif
# else
#  define ECORE_AUDIO_API
#  define ECORE_AUDIO_API_WEAK
# endif
#endif

#endif
