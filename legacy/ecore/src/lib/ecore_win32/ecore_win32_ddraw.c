/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "config.h"
#include "ecore_win32_private.h"
#include "Ecore_Win32.h"


EAPI int
ecore_win32_ddraw_init(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   DDSURFACEDESC2              surface_desc;
   DDPIXELFORMAT               pixel_format;
   RECT                        rect;
   DDSURFACEDESC2             *sd;
   struct _Ecore_Win32_Window *w;

   if (!window)
     return 0;

   w = (struct _Ecore_Win32_Window *)window;

   if (FAILED(DirectDrawCreateEx(0, (void **)&w->ddraw.object,
                                 &IID_IDirectDraw7, NULL)))
     return 0;

   if (FAILED(IDirectDraw7_SetCooperativeLevel(w->ddraw.object,
                                               w->window, DDSCL_NORMAL)))
     goto no_coop_level;

   if (FAILED(IDirectDraw7_CreateClipper(w->ddraw.object, 0,
                                    &w->ddraw.clipper, NULL)))
     goto no_clipper;

   if (FAILED(IDirectDrawClipper_SetHWnd(w->ddraw.clipper, 0, w->window)))
     goto no_clipper_set_window;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

   /* Hack to cleanly remove a warning */
   sd = &surface_desc;
   if (FAILED(IDirectDraw7_CreateSurface(w->ddraw.object,
                                         (DDSURFACEDESC *)sd,
                                         &w->ddraw.surface_primary,
                                         NULL)))
     goto no_primary_surf;

   if (FAILED(IDirectDrawSurface7_SetClipper(w->ddraw.surface_primary,
                                             w->ddraw.clipper)))
     goto no_primary_surf_set_clipper;

   if (!GetClientRect(w->window, &rect))
     goto no_get_client;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = rect.right - rect.left;
   surface_desc.dwHeight = rect.bottom - rect.top;

   /* Hack to cleanly remove a warning */
   sd = &surface_desc;
   if (FAILED(IDirectDraw7_CreateSurface(w->ddraw.object,
                                         (DDSURFACEDESC *)sd,
                                         &w->ddraw.surface_back,
                                         NULL)))
     goto no_back_surf;

   ZeroMemory(&pixel_format, sizeof(pixel_format));
   pixel_format.dwSize = sizeof(pixel_format);

   if (FAILED(IDirectDrawSurface7_GetPixelFormat(w->ddraw.surface_primary,
                                                 &pixel_format)))
     goto no_get_pix_fmt;

   w->ddraw.depth = pixel_format.dwRGBBitCount;

   return 1;

 no_get_pix_fmt:
   /* no need to release the back surface. the next call free its memory */
 no_back_surf:
 no_get_client:
 no_primary_surf_set_clipper:
   IDirectDrawSurface7_Release(w->ddraw.surface_primary);
 no_primary_surf:
 no_clipper_set_window:
   IDirectDrawClipper_Release(w->ddraw.clipper);
 no_clipper:
 no_coop_level:
   IDirectDraw7_Release(w->ddraw.object);
#endif /* HAVE_DIRECTDRAW */

   return 0;
}

EAPI void
ecore_win32_ddraw_shutdown(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   struct _Ecore_Win32_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_Win32_Window *)window;

   if (w->ddraw.surface_primary)
     IDirectDrawSurface7_Release(w->ddraw.surface_primary);

   /* no need to release the back surface. the previous call free its memory */

   if (w->ddraw.clipper)
     IDirectDrawClipper_Release(w->ddraw.clipper);

   if (w->ddraw.object)
     IDirectDraw7_Release(w->ddraw.object);
#endif /* HAVE_DIRECTDRAW */
}

EAPI Ecore_Win32_DDraw_Object *
ecore_win32_ddraw_object_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   return ((struct _Ecore_Win32_Window *)window)->ddraw.object;
#else
   return NULL;
#endif /* HAVE_DIRECTDRAW */
}

EAPI Ecore_Win32_DDraw_Surface *
ecore_win32_ddraw_surface_primary_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   return ((struct _Ecore_Win32_Window *)window)->ddraw.surface_primary;
#else
   return NULL;
#endif /* HAVE_DIRECTDRAW */
}

EAPI Ecore_Win32_DDraw_Surface *
ecore_win32_ddraw_surface_back_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   return ((struct _Ecore_Win32_Window *)window)->ddraw.surface_back;
#else
   return NULL;
#endif /* HAVE_DIRECTDRAW */
}

EAPI int
ecore_win32_ddraw_depth_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   return ((struct _Ecore_Win32_Window *)window)->ddraw.depth;
#else
   return 0;
#endif /* HAVE_DIRECTDRAW */
}
