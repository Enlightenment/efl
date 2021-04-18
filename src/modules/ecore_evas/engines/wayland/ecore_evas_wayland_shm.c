#include "ecore_evas_wayland_private.h"

#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

/* external functions */
EMODAPI Ecore_Evas *
ecore_evas_wayland_shm_new_internal(const char *disp_name, Ecore_Window parent, int x, int y, int w, int h, Eina_Bool frame)
{
   LOGFN;

   return _ecore_evas_wl_common_new_internal(disp_name, parent, x, y, w, h,
                                             frame, NULL, "wayland_shm");
}

#endif
