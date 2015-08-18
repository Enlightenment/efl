#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

EAPI struct wl_display *
ecore_wl2_display_create(void)
{
   return wl_display_create();
}

EAPI struct wl_display *
ecore_wl2_display_connect(const char *name)
{
   struct wl_display *disp;

   /* try to connect to wayland display with this name */
   disp = wl_display_connect(name);
   if (!disp)
     {
        ERR("Could not connect to display %s: %m", name);
        return NULL;
     }

   return disp;
}
