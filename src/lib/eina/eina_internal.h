#ifndef EINA_INTERNAL_H
# define EINA_INTERNAL_H

/*
 * eina_internal.h
 *
 * Lists public functions that are meant for internal use by EFL only and are
 * not stable API.
 */

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
# define EAPI_WEAK
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK __attribute__ ((weak))
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# else
/**
 * @def EAPI
 * @brief Used to export functions (by changing visibility).
 */
#  define EAPI
# endif
#endif

/**
 * @brief Cancels all pending promise/futures.
 *
 * Internal function. Do not use.
 *
 * @internal
 */
EAPI void __eina_promise_cancel_all(void);

#undef EAPI
#define EAPI

#endif
