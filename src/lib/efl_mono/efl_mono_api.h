#ifndef _EFL_EFL_MONO_API_H
#define _EFL_EFL_MONO_API_H

#ifdef EFL_MONO_API
#error EFL_MONO_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EFL_MONO_STATIC
#  ifdef EFL_MONO_BUILD
#   define EFL_MONO_API __declspec(dllexport)
#  else
#   define EFL_MONO_API __declspec(dllimport)
#  endif
# else
#  define EFL_MONO_API
# endif
# define EFL_MONO_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EFL_MONO_API __attribute__ ((visibility("default")))
#   define EFL_MONO_API_WEAK __attribute__ ((weak))
#  else
#   define EFL_MONO_API
#   define EFL_MONO_API_WEAK
#  endif
# else
#  define EFL_MONO_API
#  define EFL_MONO_API_WEAK
# endif
#endif

#endif
