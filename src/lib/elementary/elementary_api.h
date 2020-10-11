#ifndef _EFL_ELM_API_H
#define _EFL_ELM_API_H

#ifdef ELM_API
#error ELM_API should not be already defined
#endif

#ifdef _WIN32
# ifndef ELM_STATIC
#  ifdef ELM_BUILD
#   define ELM_API __declspec(dllexport)
#  else
#   define ELM_API __declspec(dllimport)
#  endif
# else
#  define ELM_API
# endif
# define ELM_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define ELM_API __attribute__ ((visibility("default")))
#   define ELM_API_WEAK __attribute__ ((weak))
#  else
#   define ELM_API
#   define ELM_API_WEAK
#  endif
# else
#  define ELM_API
#  define ELM_API_WEAK
# endif
#endif

#endif

