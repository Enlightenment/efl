#ifndef EFLAT_XML_H
#define EFLAT_XML_H

#include <Ecore.h>
#include <Efl.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EFLAT_XML_MODEL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EFLAT_XML_MODEL_BUILD */
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
 * @brief Initialize eflat_xml_model.
 *
 * @return 1 or greater on success, 0 otherwise
 */
EAPI int eflat_xml_model_init(void);
/**
 * @brief Shutdown eflat_xml_model.
 *
 * @return 0 if eflat_xml_model shuts down, greater than 0 otherwise.
 */
EAPI int eflat_xml_model_shutdown(void);

#ifdef EFL_EO_API_SUPPORT
# include <eflat_xml_model.eo.h>
#endif

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
