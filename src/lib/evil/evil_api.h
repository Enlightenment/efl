#ifndef EVIL_API_H
#define EVIL_API_H

#ifdef EVIL_API
#error EVIL_API should not be already defined
#endif

#ifdef _WIN32
# ifndef EVIL_STATIC
#  ifdef EVIL_BUILD
#   define EVIL_API __declspec(dllexport)
#  else
#   define EVIL_API __declspec(dllimport)
#  endif
# else
#  define EVIL_API
# endif
# define EVIL_API_WEAK
#elif defined(__GNUC__)
# if __GNUC__ >= 4
#  define EVIL_API __attribute__ ((visibility("default")))
#  define EVIL_API_WEAK __attribute__ ((weak))
# else
#  define EVIL_API
#  define EVIL_API_WEAK
# endif
#else
/**
 * @def EVIL_API
 * @brief Used to export functions (by changing visibility).
 */
# define EVIL_API
/**
 * @def EINA_API_WEAK
 * @brief Weak symbol, primarily useful in defining library functions which
 * can be overridden in user code.
 * Note: Not supported on all platforms.
 */
# define EINA_API_WEAK
#endif

#endif // EVIL_API_H
