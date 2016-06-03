#ifndef EJSON_H
#define EJSON_H

#include <Ecore.h>
#include <Efl.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EIO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EIO_BUILD */
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

/**
 * @brief Initialize ejson.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int ejson_init(void);

/**
 * @brief Shutdown ejson.
 *
 * @return 0 if e_dbus shuts down, greater than 0 otherwise.
 */
EAPI int ejson_shutdown(void);

#include <ejson_model.eo.h>

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
