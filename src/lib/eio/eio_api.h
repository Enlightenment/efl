#ifndef _EFL_EIO_API_H
#define _EFL_EIO_API_H

#ifdef EIO_API
#error EIO_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EIO_STATIC
#  ifdef EIO_BUILD
#   define EIO_API __declspec(dllexport)
#  else
#   define EIO_API __declspec(dllimport)
#  endif
# else
#  define EIO_API
# endif
# define EIO_API_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EIO_API __attribute__ ((visibility("default")))
#   define EIO_API_WEAK __attribute__ ((weak))
#  else
#   define EIO_API
#   define EIO_API_WEAK
#  endif
# else
#  define EIO_API
#  define EIO_API_WEAK
# endif
#endif

#endif
