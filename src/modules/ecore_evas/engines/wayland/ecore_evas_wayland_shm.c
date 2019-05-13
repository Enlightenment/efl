#include "ecore_evas_wayland_private.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
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

/* external functions */
EAPI Ecore_Evas *
ecore_evas_wayland_shm_new_internal(const char *disp_name, Ecore_Window parent, int x, int y, int w, int h, Eina_Bool frame)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_evas_wl_common_new_internal(disp_name, parent,
                                             x, y, w, h, frame,
                                             NULL,
                                             "wayland_shm");
}

#endif
