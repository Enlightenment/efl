
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eo.h>

#undef EOAPI
#undef EAPI
#define EOAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_EO_BUILD
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

#include "example_numberwrapper.eo.h"


typedef struct Example_Numberwrapper_Data
{
   int number;
} Example_Numberwrapper_Data;

// ##################### //
// Example.Numberwrapper //
// ##################### //


void _example_numberwrapper_number_set(EINA_UNUSED Eo *obj, Example_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

int _example_numberwrapper_number_get(EINA_UNUSED Eo *obj, Example_Numberwrapper_Data *pd)
{
   return pd->number;
}

#include "example_numberwrapper.eo.c"

