#ifndef EINA_PREAMBLE_H
# define EINA_PREAMBLE_H

#ifdef EINA_API
# undef EINA_API
#endif

#ifdef _MSC_VER
# ifndef EINA_DLL
#  define EINA_API
# elif defined(EINA_BUILD)
#  define EINA_API __declspec(dllexport)
# else
#  define EINA_API __declspec(dllimport)
# endif
# define EINA_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EINA_API __attribute__ ((visibility("default")))
#   define EINA_API_WEAK __attribute__ ((weak))
#  else
#   define EINA_API
#   define EINA_API_WEAK
#  endif
# else
#  define EINA_API
# endif
#endif

#endif
