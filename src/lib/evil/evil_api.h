#ifndef EVIL_API_H
#define EVIL_API_H

#ifdef EVIL_API
#error EVIL_API cant be already defined
#endif

#ifdef _MSC_VER
# ifdef EVIL_BUILD
#  define EVIL_API __declspec(dllexport)
# else
#  define EVIL_API __declspec(dllimport)
# endif
#else
# if __GNUC__ >= 4
#  define EVIL_API __attribute__ ((visibility("default")))
# else
#  define EVIL_API
# endif
#endif

#endif
