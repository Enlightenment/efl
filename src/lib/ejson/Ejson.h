#ifndef EJSON_H
#define EJSON_H

#include <Ecore.h>
#include <Efl.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EJSON_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EJSON_BUILD */
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

#define EJSON_MODEL_NAME_PROPERTY "name"
#define EJSON_MODEL_VALUE_PROPERTY "value"
#define EJSON_MODEL_JSON_PROPERTY "json"

/**
 * @brief Initialize ejson.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int ejson_init(void);

/**
 * @brief Shutdown ejson.
 *
 * @return 0 if ejson shuts down, greater than 0 otherwise.
 */
EAPI int ejson_shutdown(void);

#ifdef EFL_EO_API_SUPPORT
# include <ejson_model.eo.h>
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
