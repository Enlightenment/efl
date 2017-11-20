#ifndef SCENEGRAPH_H_
#define SCENEGRAPH_H_
#include <Eina.h>
#include <Ecore.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECTOR_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
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

#include "sg_node.h"
#include "utils/sg_region.h"

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif //SCENEGRAPH_H_