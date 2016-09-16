#ifndef EINA_CPU_PRIVATE_H_
#define EINA_CPU_PRIVATE_H_

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EINA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EINA_BUILD */
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
 * @brief Used to export functions(by changing visibility).
 */
#  define EAPI
# endif
#endif

EAPI int _eina_cpu_fast_core_get(void);

#undef EAPI
#define EAPI

#endif
