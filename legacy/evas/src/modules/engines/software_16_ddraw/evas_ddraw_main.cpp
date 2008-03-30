#include "evas_engine.h"


void *
evas_software_ddraw_lock(DDraw_Output_Buffer *ddob, int *ddraw_width, int *ddraw_height, int *ddraw_pitch, int *ddraw_depth)
{
   DDSURFACEDESC surface_desc;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   if (FAILED(ddob->dd.surface_back->Lock(NULL,
                                          &surface_desc,
                                          DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY,
                                          NULL)))
     return NULL;

   *ddraw_width = surface_desc.dwWidth;
   *ddraw_height = surface_desc.dwHeight;
   *ddraw_pitch = surface_desc.lPitch;
   *ddraw_depth = surface_desc.ddpfPixelFormat.dwRGBBitCount >> 3;

   return surface_desc.lpSurface;
}

void
evas_software_ddraw_unlock_and_flip(DDraw_Output_Buffer *ddob)
{
   RECT    dst_rect;
   RECT    src_rect;
   POINT   p;

   if (FAILED(ddob->dd.surface_back->Unlock(NULL)))
     return;

   /* we figure out where on the primary surface our window lives */
   p.x = 0;
   p.y = 0;
   ClientToScreen(ddob->dd.window, &p);
   GetClientRect(ddob->dd.window, &dst_rect);
   OffsetRect(&dst_rect, p.x, p.y);
   SetRect(&src_rect, 0, 0, ddob->width, ddob->height);

   /* nothing to do if the function fails, so we don't check the result */
   ddob->dd.surface_primary->BltFast(0, 0,
                                     ddob->dd.surface_back, &dst_rect,
                                     DDBLTFAST_WAIT || DDBLTFAST_NOCOLORKEY);
}

void
evas_software_ddraw_surface_resize(DDraw_Output_Buffer *ddob)
{
   DDSURFACEDESC surface_desc;

   ddob->dd.surface_back->Release();
   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   /* FIXME: that code does not compile. Must know why */
#if 0
   surface_desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.dwWidth = width;
   surface_desc.dwHeight = height;
   IDirectDrawSurface7_SetSurfaceDesc(ddob->dd.surface_back, &surface_desc, NULL);
#else
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = ddob->width;
   surface_desc.dwHeight = ddob->height;
   ddob->dd.object->CreateSurface(&surface_desc, &ddob->dd.surface_back, NULL);
#endif
}
