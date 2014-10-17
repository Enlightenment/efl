#ifndef ELM_CODE_H_
# define ELM_CODE_H_

#include <Elementary.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ELM_CODE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ELM_CODE_BUILD */
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
 * @file
 * @brief These routines are used for interacting with files using Elm Code.
 */

/**
 * @brief Init / shutdown functions.
 * @defgroup Init  Init / Shutdown
 *
 * @{
 *
 * Functions of obligatory usage, handling proper initialization
 * and shutdown routines.
 *
 * Before the usage of any other function, Elm Code should be properly
 * initialized with @ref elm_code_init() and the last call to Elm Code's
 * functions should be @ref elm_code_shutdown(), so everything will
 * be correctly freed.
 *
 * Elm Code logs everything with Eina Log, using the "elm_code" log domain.
 *
 */

/**
 * Initialize Elm Code.
 *
 * Initializes Elm Code, its dependencies and modules. Should be the first
 * function of Elm Code to be called.
 *
 * @return The init counter value.
 *
 * @see elm_code_shutdown().
 *
 * @ingroup Init
 */
EAPI int elm_code_init(void);

/**
 * Shutdown Elm Code
 *
 * Shutdown Elm Code. If init count reaches 0, all the internal structures will
 * be freed. Any Elm Code library call after this point will leads to an error.
 *
 * @return Elm Code's init counter value.
 *
 * @see elm_code_init().
 *
 * @ingroup Init
 */
EAPI int elm_code_shutdown(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ELM_CODE_H_ */
