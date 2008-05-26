/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstdio>

#include "ecore_win32_private.h"
#include "Ecore_Win32.h"


EAPI int
ecore_win32_ddraw_init(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   DDSURFACEDESC               surface_desc;
   DDPIXELFORMAT               pixel_format;
   RECT                        rect;
   struct _Ecore_Win32_Window *w;

   if (!window)
     return 0;

   w = (struct _Ecore_Win32_Window *)window;
   w->ddraw.surface_source = NULL;
   {
     RECT rect;
     GetClientRect(w->window, &rect);
     printf (" *** ecore_win32_ddraw_init %ld %ld\n",
             rect.right - rect.left, rect.bottom - rect.top);
   }

   if (FAILED(DirectDrawCreate(NULL, &w->ddraw.object, NULL)))
     return 0;

   if (FAILED(w->ddraw.object->SetCooperativeLevel(w->window, DDSCL_NORMAL)))
     goto no_coop_level;

   if (FAILED(w->ddraw.object->CreateClipper(0, &w->ddraw.clipper, NULL)))
     goto no_clipper;

   if (FAILED(w->ddraw.clipper->SetHWnd(0, w->window)))
     goto no_clipper_set_window;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

   if (FAILED(w->ddraw.object->CreateSurface(&surface_desc,
                                             &w->ddraw.surface_primary,
                                             NULL)))
     goto no_primary_surf;

   if (FAILED(w->ddraw.surface_primary->SetClipper(w->ddraw.clipper)))
     goto no_primary_surf_set_clipper;

   if (!GetClientRect(w->window, &rect))
     goto no_get_client;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = rect.right - rect.left;
   surface_desc.dwHeight = rect.bottom - rect.top;

   if (FAILED(w->ddraw.object->CreateSurface(&surface_desc,
                                             &w->ddraw.surface_back,
                                             NULL)))
     goto no_back_surf;

   ZeroMemory(&pixel_format, sizeof(pixel_format));
   pixel_format.dwSize = sizeof(pixel_format);

   if (FAILED(w->ddraw.surface_primary->GetPixelFormat(&pixel_format)))
     goto no_get_pix_fmt;

   w->ddraw.depth = pixel_format.dwRGBBitCount;

   w->backend = ECORE_WIN32_BACKEND_DIRECTDRAW;

   return 1;

 no_get_pix_fmt:
   /* no need to release the back surface. the next call free its memory */
 no_back_surf:
 no_get_client:
 no_primary_surf_set_clipper:
   w->ddraw.surface_primary->Release();
 no_primary_surf:
 no_clipper_set_window:
   w->ddraw.clipper->Release();
 no_clipper:
 no_coop_level:
   w->ddraw.object->Release();
#endif /* HAVE_DIRECTDRAW */

   return 0;
}

EAPI int
ecore_win32_ddraw_16_init(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   DDSURFACEDESC               surface_desc;
   DDPIXELFORMAT               pixel_format;
   RECT                        rect;
   void                       *source;
   struct _Ecore_Win32_Window *w;

   if (!window)
     return 0;

   w = (struct _Ecore_Win32_Window *)window;
   {
     RECT rect;
     GetClientRect(w->window, &rect);
     printf (" *** ecore_win32_ddraw_init %ld %ld\n",
             rect.right - rect.left, rect.bottom - rect.top);
   }
   w->ddraw.clipper = NULL;

   if (FAILED(DirectDrawCreate(NULL, &w->ddraw.object, NULL)))
     return 0;

   if (FAILED(w->ddraw.object->SetCooperativeLevel(w->window, DDSCL_NORMAL)))
     goto no_coop_level;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

   if (FAILED(w->ddraw.object->CreateSurface(&surface_desc,
                                             &w->ddraw.surface_primary,
                                             NULL)))
     goto no_primary_surf;

   if (!GetClientRect(w->window, &rect))
     goto no_get_client;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = rect.right - rect.left;
   surface_desc.dwHeight = rect.bottom - rect.top;

   if (FAILED(w->ddraw.object->CreateSurface(&surface_desc,
                                             &w->ddraw.surface_back,
                                             NULL)))
     goto no_back_surf;

   ZeroMemory(&pixel_format, sizeof(pixel_format));
   pixel_format.dwSize = sizeof(pixel_format);

   if (FAILED(w->ddraw.surface_primary->GetPixelFormat(&pixel_format)))
     goto no_get_pix_fmt;

   w->ddraw.depth = pixel_format.dwRGBBitCount;

   source = malloc((rect.right - rect.left) * (rect.bottom - rect.top) * 2);
   if (!source)
     goto no_source;

   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   surface_desc.dwFlags =
     DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
     DDSD_LPSURFACE | DDSD_PITCH | DDSD_PIXELFORMAT;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
   surface_desc.dwWidth = rect.right - rect.left;
   surface_desc.dwHeight = rect.bottom - rect.top;
   surface_desc.lPitch = 2 * surface_desc.dwWidth;
   surface_desc.lpSurface = source;

   surface_desc.ddpfPixelFormat = pixel_format;

   if (FAILED(w->ddraw.object->CreateSurface(&surface_desc,
                                             &w->ddraw.surface_source,
                                             NULL)))
     goto no_source_surf;

   w->backend = ECORE_WIN32_BACKEND_DIRECTDRAW_16;

   return 1;

 no_source_surf:
   free(source);
 no_source:
 no_get_pix_fmt:
   /* no need to release the back surface. the next call free its memory */
 no_back_surf:
 no_get_client:
   w->ddraw.surface_primary->Release();
 no_primary_surf:
 no_coop_level:
   w->ddraw.object->Release();
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
     {
        w->ddraw.surface_primary->Release();
        w->ddraw.surface_primary = NULL;
     }

   if (w->ddraw.surface_source)
     {
        DDSURFACEDESC surface_desc;

        ZeroMemory(&surface_desc, sizeof(surface_desc));
        surface_desc.dwSize = sizeof(surface_desc);

        if (w->ddraw.surface_source->Lock(NULL,
                                          &surface_desc,
                                          DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                                          NULL) == DD_OK)
          {
             free(surface_desc.lpSurface);
             if (w->ddraw.surface_source->Unlock(NULL) == DD_OK)
               {
                  w->ddraw.surface_source->Release();
               }
          }
        w->ddraw.surface_source = NULL;
     }

   /* no need to release the back surface. the previous call free its memory */

   if (w->ddraw.clipper)
     {
        w->ddraw.clipper->Release();
        w->ddraw.clipper = NULL;
     }

   if (w->ddraw.object)
     {
        w->ddraw.object->Release();
        w->ddraw.object = NULL;
     }

   w->backend = ECORE_WIN32_BACKEND_NONE;
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

EAPI Ecore_Win32_DDraw_Surface *
ecore_win32_ddraw_surface_source_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECTDRAW
   return ((struct _Ecore_Win32_Window *)window)->ddraw.surface_source;
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
