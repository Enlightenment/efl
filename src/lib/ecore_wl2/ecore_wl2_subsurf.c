#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

void
_ecore_wl2_subsurf_free(Ecore_Wl2_Subsurface *subsurf)
{
   Ecore_Wl2_Window *parent;

   if (subsurf->wl.subsurface) wl_subsurface_destroy(subsurf->wl.subsurface);
   if (subsurf->wl.surface) wl_surface_destroy(subsurf->wl.surface);

   parent = subsurf->parent;
   if (parent)
     {
        parent->subsurfs =
          eina_inlist_remove(parent->subsurfs, EINA_INLIST_GET(subsurf));
     }

   free(subsurf);
}

EAPI Ecore_Wl2_Subsurface *
ecore_wl2_subsurface_new(Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Display *display;
   Ecore_Wl2_Subsurface *subsurf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(window->surface, NULL);

   display = window->display;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display->wl.compositor, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(display->wl.subcompositor, NULL);

   subsurf = calloc(1, sizeof(Ecore_Wl2_Subsurface));
   if (!subsurf) return NULL;

   subsurf->parent = window;

   subsurf->wl.surface = wl_compositor_create_surface(display->wl.compositor);
   if (!subsurf->wl.surface)
     {
        ERR("Failed to create surface: %m");
        goto surf_err;
     }

   subsurf->wl.subsurface =
     wl_subcompositor_get_subsurface(display->wl.subcompositor,
                                     subsurf->wl.surface, window->surface);
   if (!subsurf->wl.subsurface)
     {
        ERR("Could not create subsurface: %m");
        goto sub_surf_err;
     }

   window->subsurfs =
     eina_inlist_append(window->subsurfs, EINA_INLIST_GET(subsurf));

   return subsurf;

sub_surf_err:
   wl_surface_destroy(subsurf->wl.surface);

surf_err:
   free(subsurf);
   return NULL;
}

EAPI void
ecore_wl2_subsurface_del(Ecore_Wl2_Subsurface *subsurface)
{
   EINA_SAFETY_ON_NULL_RETURN(subsurface);

   _ecore_wl2_subsurf_free(subsurface);
}

EAPI struct wl_surface *
ecore_wl2_subsurface_surface_get(Ecore_Wl2_Subsurface *subsurface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(subsurface, NULL);

   return subsurface->wl.surface;
}
