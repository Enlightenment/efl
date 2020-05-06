#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl_private.h"

/*
 * The subsurface protocol was moved into Wayland Core
 * around v1.3.90 (i.e. v1.4.0).
 * Test if subsurface protocol is part of wayland-client.h.
 * If not, we include our own copy of the protocol header.
 */
#include <wayland-client.h>
#ifndef WL_SUBSURFACE_ERROR_ENUM
# include <subsurface-client-protocol.h>
#endif

struct _Ecore_Wl_Subsurf
{
   EINA_INLIST;
   Ecore_Wl_Window *parent_win;
   struct wl_surface *surface;
   struct wl_subsurface *subsurface;

   int x, y;

   Eina_Bool sync : 1;
};

EAPI Ecore_Wl_Subsurf *
ecore_wl_subsurf_create(Ecore_Wl_Window *win)
{
   struct wl_subsurface *subsurface;
   struct wl_surface *surface;
   Ecore_Wl_Subsurf *ess;
   struct wl_subcompositor *subcomp;

   LOGFN;

   if (!win) return NULL;
   if (!win->surface) return NULL;

   subcomp = _ecore_wl_subcompositor_get();
   if (!subcomp) return NULL;

   surface = wl_compositor_create_surface(_ecore_wl_compositor_get());
   if (!surface)
     return NULL;

   subsurface = wl_subcompositor_get_subsurface
      (subcomp, surface, win->surface);
   if (!subsurface)
     {
        wl_surface_destroy(surface);
        return NULL;
     }

   ess = calloc(1, sizeof(*ess));
   ess->surface = surface;
   ess->subsurface = subsurface;
   ess->parent_win = win;

   win->subsurfs = (Ecore_Wl_Subsurf *)eina_inlist_append
      (EINA_INLIST_GET(win->subsurfs), EINA_INLIST_GET(ess));

   return ess;
}

static void
_ecore_wl_subsurf_destroy(Ecore_Wl_Subsurf *ess)
{
   Ecore_Wl_Window *parent;

   if (!ess) return;

   if (ess->subsurface)
      wl_subsurface_destroy(ess->subsurface);

   if (ess->surface)
      wl_surface_destroy(ess->surface);

   parent = ess->parent_win;
   parent->subsurfs = (Ecore_Wl_Subsurf *)eina_inlist_remove
      (EINA_INLIST_GET(parent->subsurfs), EINA_INLIST_GET(ess));

   free(ess);
}

EAPI void
ecore_wl_subsurf_del(Ecore_Wl_Subsurf *ess)
{
   LOGFN;

   if (!ess) return;

   _ecore_wl_subsurf_destroy(ess);
}

void
_ecore_wl_subsurfs_del_all(Ecore_Wl_Window *win)
{
   Ecore_Wl_Subsurf *ess;

   if (!win) return;

   EINA_INLIST_FREE(win->subsurfs, ess)
     {
        _ecore_wl_subsurf_destroy(ess);
     }
}

EAPI struct wl_surface *
ecore_wl_subsurf_surface_get(Ecore_Wl_Subsurf *ess)
{
   LOGFN;

   if (!ess) return NULL;

   return ess->surface;
}

EAPI void
ecore_wl_subsurf_position_set(Ecore_Wl_Subsurf *ess, int x, int y)
{
   LOGFN;

   if (!ess) return;
   if (!ess->subsurface) return;

   if ((x == ess->x) && (y == ess->y))
     return;

   ess->x = x;
   ess->y = y;

   wl_subsurface_set_position(ess->subsurface, x, y);
}

EAPI void
ecore_wl_subsurf_position_get(Ecore_Wl_Subsurf *ess, int *x, int *y)
{
   LOGFN;

   if (!ess) return;

   if (x) *x = ess->x;
   if (y) *y = ess->y;
}

EAPI void
ecore_wl_subsurf_place_above(Ecore_Wl_Subsurf *ess, struct wl_surface *surface)
{
   LOGFN;

   if (!ess) return;
   if (!surface) return;
   if (!ess->subsurface) return;

   wl_subsurface_place_above(ess->subsurface, surface);
}

EAPI void
ecore_wl_subsurf_place_below(Ecore_Wl_Subsurf *ess, struct wl_surface *surface)
{
   LOGFN;

   if (!ess) return;
   if (!surface) return;
   if (!ess->subsurface) return;

   wl_subsurface_place_below(ess->subsurface, surface);
}

EAPI void
ecore_wl_subsurf_sync_set(Ecore_Wl_Subsurf *ess, Eina_Bool val)
{
   LOGFN;

   if (!ess) return;
   if (!ess->subsurface) return;

   val = !!val;
   if (val == ess->sync) return;

   ess->sync = val;

   if (ess->sync)
     wl_subsurface_set_sync(ess->subsurface);
   else
     wl_subsurface_set_desync(ess->subsurface);
}

EAPI void
ecore_wl_subsurf_opaque_region_set(Ecore_Wl_Subsurf *ess, int x, int y, int w, int h)
{
   struct wl_region *region = NULL;

   LOGFN;

   if (!ess) return;
   if (!ess->surface) return;

   if ((w > 0) && (h > 0))
     {
        region = wl_compositor_create_region(_ecore_wl_compositor_get());
        if (!region) return;

        wl_region_add(region, x, y, w, h);
        wl_surface_set_opaque_region(ess->surface, region);
        wl_region_destroy(region);
     }
   else
     wl_surface_set_opaque_region(ess->surface, NULL);
}
