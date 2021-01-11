#ifndef _EFL_EMOTION_API_H
#define _EFL_EMOTION_API_H

#ifdef EMOTION_API
#error EMOTION_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EMOTION_STATIC
#  ifdef EMOTION_BUILD
#   define EMOTION_API __declspec(dllexport)
#  else
#   define EMOTION_API __declspec(dllimport)
#  endif
# else
#  define EMOTION_API
# endif
# define EMOTION_API_WEAK
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define EMOTION_API __attribute__ ((visibility("default")))
#  define EMOTION_API_WEAK __attribute__ ((weak))
# else
#  define EMOTION_API
#  define EMOTION_API_WEAK
# endif
#else
# define EMOTION_API
# define EMOTION_API_WEAK
#endif

#endif
