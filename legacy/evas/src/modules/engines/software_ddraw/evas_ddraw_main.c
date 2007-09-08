#include "evas_engine.h"


int
evas_software_ddraw_masks_get(Outbuf *buf)
{
   DDPIXELFORMAT pixel_format;

   ZeroMemory(&pixel_format, sizeof(pixel_format));
   pixel_format.dwSize = sizeof(pixel_format);

   if (FAILED(IDirectDrawSurface7_GetPixelFormat(buf->priv.dd.surface_primary,
                                                 &pixel_format)))
     return 0;

   buf->priv.mask.r = pixel_format.dwRBitMask;
   buf->priv.mask.g = pixel_format.dwGBitMask;
   buf->priv.mask.b = pixel_format.dwBBitMask;

   return 1;
}

void *
evas_software_ddraw_lock(Outbuf *buf, int *ddraw_width, int *ddraw_height, int *ddraw_pitch, int *ddraw_depth)
{
   DDSURFACEDESC2 surface_desc;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   if (FAILED(IDirectDrawSurface7_Lock(buf->priv.dd.surface_back, NULL,
                                       &surface_desc,
                                       DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                                       NULL)))
     return NULL;

   *ddraw_width = surface_desc.dwWidth;
   *ddraw_height = surface_desc.dwHeight;
   *ddraw_pitch = surface_desc.lPitch;
   *ddraw_depth = surface_desc.ddpfPixelFormat.dwRGBBitCount >> 3;

   return surface_desc.lpSurface;
}

void
evas_software_ddraw_unlock_and_flip(Outbuf *buf)
{
   RECT    dst_rect;
   RECT    src_rect;
   POINT   p;

   if (FAILED(IDirectDrawSurface7_Unlock(buf->priv.dd.surface_back, NULL)))
     return;

   /* we figure out where on the primary surface our window lives */
   p.x = 0;
   p.y = 0;
   ClientToScreen(buf->priv.dd.window, &p);
   GetClientRect(buf->priv.dd.window, &dst_rect);
   OffsetRect(&dst_rect, p.x, p.y);
   SetRect(&src_rect, 0, 0, buf->width, buf->height);

   /* nothing to do if the function fails, so we don't check the result */
   IDirectDrawSurface7_Blt(buf->priv.dd.surface_primary, &dst_rect,
                           buf->priv.dd.surface_back, &src_rect,
                           DDBLT_WAIT, NULL);
}

void
evas_software_ddraw_surface_resize(Outbuf *buf)
{
   DDSURFACEDESC2  surface_desc;
   DDSURFACEDESC2 *sd;

   IDirectDrawSurface7_Release(buf->priv.dd.surface_back);
   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   /* FIXME: that code does not compile. Must know why */
#if 0
   surface_desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.dwWidth = width;
   surface_desc.dwHeight = height;
   IDirectDrawSurface7_SetSurfaceDesc(buf->priv.dd.surface_back, &surface_desc, NULL);
#else
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = buf->width;
   surface_desc.dwHeight = buf->height;
   /* Hack to cleanly remove a warning */
   sd = &surface_desc;
   IDirectDraw7_CreateSurface(buf->priv.dd.object,
                              (DDSURFACEDESC *)sd,
                              &buf->priv.dd.surface_back,
                              NULL);
#endif
}
