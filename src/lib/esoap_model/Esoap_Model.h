#ifndef ESOAP_MODEL_H
#define ESOAP_MODEL_H

#include <Ecore.h>
#include <Efl.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ESOAP_MODEL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ESOAP_MODEL_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize esoap_model.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int esoap_model_init(void);

/**
 * @brief Shutdown esoap_model.
 *
 * @return 0 if esoap_model shuts down, greater than 0 otherwise.
 */
EAPI int esoap_model_shutdown(void);

#ifdef EFL_EO_API_SUPPORT
# include <esoap_model.eo.h>
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
