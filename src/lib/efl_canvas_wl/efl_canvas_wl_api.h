#ifndef _EFL_EFL_CANVAS_WL_API_H
#define _EFL_EFL_CANVAS_WL_API_H

#ifdef EFL_CANVAS_WL_API
#error EFL_CANVAS_WL_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EFL_CANVAS_WL_STATIC
#  ifdef EFL_CANVAS_WL_BUILD
#   define EFL_CANVAS_WL_API __declspec(dllexport)
#  else
#   define EFL_CANVAS_WL_API __declspec(dllimport)
#  endif
# else
#  define EFL_CANVAS_WL_API
# endif
# define EFL_CANVAS_WL_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EFL_CANVAS_WL_API __attribute__ ((visibility("default")))
#   define EFL_CANVAS_WL_API_WEAK __attribute__ ((weak))
#  else
#   define EFL_CANVAS_WL_API
#   define EFL_CANVAS_WL_API_WEAK
#  endif
# else
#  define EFL_CANVAS_WL_API
#  define EFL_CANVAS_WL_API_WEAK
# endif
#endif

#endif
