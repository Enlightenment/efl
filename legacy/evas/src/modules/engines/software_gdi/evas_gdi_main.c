#include "evas_common.h"
#include "evas_engine.h"


int
evas_software_gdi_init (HWND         window,
                        HBITMAP      mask,
                        int          depth,
                        unsigned int layered,
                        unsigned int fullscreen __UNUSED__,
                        Outbuf *buf)
{
   RECT window_rect;
   SIZE mask_size;

   if (!window)
     {
        ERR("[Engine] [GDI] Window is NULL");
        return 0;
     }

   buf->priv.gdi.window = window;
   buf->priv.gdi.mask = mask;
   buf->priv.gdi.dc = GetDC(window);
   if (!buf->priv.gdi.dc)
     {
        ERR("[Engine] [GDI] Can not get DC");
        return 0;
     }

   /* FIXME: check depth */
   if (depth != GetDeviceCaps(buf->priv.gdi.dc, BITSPIXEL))
     {
        ERR("[Engine] [GDI] no compatible depth");
        ReleaseDC(window, buf->priv.gdi.dc);
        return 0;
     }
   buf->priv.gdi.depth = depth;

   /* mask */
   if (layered && mask)
     {
        if (GetBitmapDimensionEx(mask, &mask_size) &&
            GetClientRect(window, &window_rect))
          {
             if ((mask_size.cx == window_rect.right) &&
                 (mask_size.cy == window_rect.bottom))
               {
/*                  if (SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_COLORKEY)) */
/*                    { */
/*                    } */
               }
          }
     }

   /* FIXME: support fullscreen */

   buf->priv.gdi.bitmap_info = (BITMAPINFO_GDI *)malloc(sizeof(BITMAPINFO_GDI));
   if (!buf->priv.gdi.bitmap_info)
     {
        ERR("[Engine] [GDI] Can not allocate bitmap info");
        ReleaseDC(window, buf->priv.gdi.dc);
        return 0;
     }

   buf->priv.gdi.bitmap_info->bih.biSize = sizeof(BITMAPINFOHEADER);
   buf->priv.gdi.bitmap_info->bih.biWidth = buf->width;
   buf->priv.gdi.bitmap_info->bih.biHeight = -buf->height;
   buf->priv.gdi.bitmap_info->bih.biPlanes = 1;
   buf->priv.gdi.bitmap_info->bih.biSizeImage = (buf->priv.gdi.depth >> 3) * buf->width * buf->height;
   buf->priv.gdi.bitmap_info->bih.biXPelsPerMeter = 0;
   buf->priv.gdi.bitmap_info->bih.biYPelsPerMeter = 0;
   buf->priv.gdi.bitmap_info->bih.biClrUsed = 0;
   buf->priv.gdi.bitmap_info->bih.biClrImportant = 0;
   buf->priv.gdi.bitmap_info->bih.biBitCount = buf->priv.gdi.depth;
   buf->priv.gdi.bitmap_info->bih.biCompression = BI_BITFIELDS;

   switch (depth)
     {
      case 16:
         buf->priv.gdi.bitmap_info->masks[0] = 0x0000f800;
         buf->priv.gdi.bitmap_info->masks[1] = 0x000007e0;
         buf->priv.gdi.bitmap_info->masks[2] = 0x0000001f;
         break;
      case 32:
         buf->priv.gdi.bitmap_info->masks[0] = 0x00ff0000;
         buf->priv.gdi.bitmap_info->masks[1] = 0x0000ff00;
         buf->priv.gdi.bitmap_info->masks[2] = 0x000000ff;
         break;
      default:
        ERR("[Engine] [GDI] wrong depth");
        free(buf->priv.gdi.bitmap_info);
        ReleaseDC(window, buf->priv.gdi.dc);
        return 0;
     }

   return 1;
}

void
evas_software_gdi_shutdown(Outbuf *buf)
{
   if (!buf)
     return;

   free(buf->priv.gdi.bitmap_info);
   ReleaseDC(buf->priv.gdi.window, buf->priv.gdi.dc);
}

void
evas_software_gdi_bitmap_resize(Outbuf *buf)
{
   buf->priv.gdi.bitmap_info->bih.biWidth = buf->width;
   buf->priv.gdi.bitmap_info->bih.biHeight = -buf->height;
   buf->priv.gdi.bitmap_info->bih.biSizeImage = (buf->priv.gdi.depth >> 3) * buf->width * buf->height;
}
