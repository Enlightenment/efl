#include "evas_common.h"
#include "evas_engine.h"

int
evas_software_ddraw_init (HWND    window,
                          int     depth,
                          int     fullscreen,
                          Outbuf *buf)
{
   DDSURFACEDESC  surface_desc;
   DDPIXELFORMAT  pixel_format;
   HRESULT        res;
   int            width;
   int            height;

   if (!buf)
     return 0;

   buf->priv.dd.window = window;

   res = DirectDrawCreate(NULL, &buf->priv.dd.object, NULL);
   if (FAILED(res))
     return 0;

   if (buf->priv.dd.fullscreen)
     {
        DDSCAPS caps;

        res = buf->priv.dd.object->SetCooperativeLevel(window,
                                                       DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        if (FAILED(res))
          goto release_object;

        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);

        ZeroMemory(&pixel_format, sizeof(pixel_format));
        pixel_format.dwSize = sizeof(pixel_format);
        buf->priv.dd.surface_primary->GetPixelFormat(&pixel_format);

        if (pixel_format.dwRGBBitCount != depth)
          goto release_object;

        buf->priv.dd.depth = depth;

        res = buf->priv.dd.object->SetDisplayMode(width, height, depth);
        if (FAILED(res))
          goto release_object;

        memset(&surface_desc, 0, sizeof(surface_desc));
        surface_desc.dwSize = sizeof(surface_desc);
        surface_desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        surface_desc.dwBackBufferCount = 1;

        res = buf->priv.dd.object->CreateSurface(&surface_desc,
                                                 &buf->priv.dd.surface_primary, NULL);
        if (FAILED(res))
          goto release_object;

        caps.dwCaps = DDSCAPS_BACKBUFFER;
        res = buf->priv.dd.surface_primary->GetAttachedSurface(&caps,
                                                               &buf->priv.dd.surface_back);
        if (FAILED(res))
          goto release_surface_primary;
     }
   else
     {
        RECT           rect;

        if (!GetClientRect(window, &rect))
          goto release_object;

        width = rect.right - rect.left;
        height = rect.bottom - rect.top;

        res = buf->priv.dd.object->SetCooperativeLevel(window, DDSCL_NORMAL);
        if (FAILED(res))
          goto release_object;

        res = buf->priv.dd.object->CreateClipper(0, &buf->priv.dd.clipper, NULL);
        if (FAILED(res))
          goto release_object;

        res = buf->priv.dd.clipper->SetHWnd(0, window);
        if (FAILED(res))
          goto release_clipper;

        memset(&surface_desc, 0, sizeof(surface_desc));
        surface_desc.dwSize = sizeof(surface_desc);
        surface_desc.dwFlags = DDSD_CAPS;
        surface_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        res = buf->priv.dd.object->CreateSurface(&surface_desc, &buf->priv.dd.surface_primary, NULL);
        if (FAILED(res))
          goto release_clipper;

        res = buf->priv.dd.surface_primary->SetClipper(buf->priv.dd.clipper);
        if (FAILED(res))
          goto release_surface_primary;

        memset (&surface_desc, 0, sizeof(surface_desc));
        surface_desc.dwSize = sizeof(surface_desc);
        surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
        surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        surface_desc.dwWidth = width;
        surface_desc.dwHeight = height;

        res = buf->priv.dd.object->CreateSurface(&surface_desc, &buf->priv.dd.surface_back, NULL);
        if (FAILED(res))
          goto release_surface_primary;

        ZeroMemory(&pixel_format, sizeof(pixel_format));
        pixel_format.dwSize = sizeof(pixel_format);
        buf->priv.dd.surface_primary->GetPixelFormat(&pixel_format);

        if (pixel_format.dwRGBBitCount != depth)
          goto release_surface_back;

        buf->priv.dd.depth = depth;
     }

   return 1;

 release_surface_back:
   buf->priv.dd.surface_back->Release();
 release_surface_primary:
   buf->priv.dd.surface_primary->Release();
 release_clipper:
   if (buf->priv.dd.fullscreen)
     buf->priv.dd.clipper->Release();
 release_object:
   buf->priv.dd.object->Release();

   return 0;
}

void
evas_software_ddraw_shutdown(Outbuf *buf)
{
   if (!buf)
     return;

   if (buf->priv.dd.fullscreen)
     if (buf->priv.dd.surface_back)
       buf->priv.dd.surface_back->Release();
   if (buf->priv.dd.surface_primary)
     buf->priv.dd.surface_primary->Release();
   if (buf->priv.dd.fullscreen)
     if (buf->priv.dd.clipper)
       buf->priv.dd.clipper->Release();
   if (buf->priv.dd.object)
     buf->priv.dd.object->Release();
}

int
evas_software_ddraw_masks_get(Outbuf *buf)
{
   DDPIXELFORMAT pixel_format;

   ZeroMemory(&pixel_format, sizeof(pixel_format));
   pixel_format.dwSize = sizeof(pixel_format);

   if (FAILED(buf->priv.dd.surface_primary->GetPixelFormat(&pixel_format)))
     return 0;

   buf->priv.mask.r = pixel_format.dwRBitMask;
   buf->priv.mask.g = pixel_format.dwGBitMask;
   buf->priv.mask.b = pixel_format.dwBBitMask;

   return 1;
}

void *
evas_software_ddraw_lock(Outbuf *buf,
                         int    *ddraw_width,
                         int    *ddraw_height,
                         int    *ddraw_pitch,
                         int    *ddraw_depth)
{
   DDSURFACEDESC surface_desc;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   if (FAILED(buf->priv.dd.surface_back->Lock(NULL,
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

   if (FAILED(buf->priv.dd.surface_back->Unlock(NULL)))
     return;

   /* we figure out where on the primary surface our window lives */
   p.x = 0;
   p.y = 0;
   ClientToScreen(buf->priv.dd.window, &p);
   GetClientRect(buf->priv.dd.window, &dst_rect);
   OffsetRect(&dst_rect, p.x, p.y);
   SetRect(&src_rect, 0, 0, buf->width, buf->height);

   /* nothing to do if the function fails, so we don't check the result */
   buf->priv.dd.surface_primary->Blt(&dst_rect,
                                     buf->priv.dd.surface_back,
                                     &src_rect,
                                     DDBLT_WAIT, NULL);
}

void
evas_software_ddraw_surface_resize(Outbuf *buf)
{
   DDSURFACEDESC surface_desc;

   buf->priv.dd.surface_back->Release();
   memset (&surface_desc, 0, sizeof (surface_desc));
   surface_desc.dwSize = sizeof (surface_desc);
   /* FIXME: that code does not compile. Must know why */
#if 0
   surface_desc.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.dwWidth = width;
   surface_desc.dwHeight = height;
   buf->priv.dd.surface_back->SetSurfaceDesc(&surface_desc, NULL);
#else
   surface_desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   surface_desc.dwWidth = buf->width;
   surface_desc.dwHeight = buf->height;
   buf->priv.dd.object->CreateSurface(&surface_desc,
                                      &buf->priv.dd.surface_back,
                                      NULL);
#endif
}
