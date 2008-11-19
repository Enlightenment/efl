#include "evas_common.h"
#include "evas_engine.h"


typedef struct BITMAPINFO_16bpp BITMAPINFO_16bpp;
typedef struct Evas_Engine_WinCE_GDI_Priv Evas_Engine_WinCE_GDI_Priv;

struct BITMAPINFO_16bpp
{
   BITMAPINFOHEADER bih;
   DWORD            masks[3];
};

struct Evas_Engine_WinCE_GDI_Priv
{
   HWND              window;
   HDC               dc;
   BITMAPINFO_16bpp *bitmap_info;
   HBITMAP           bitmap;
   int               width;
   int               height;
};

void *
evas_software_wince_gdi_init(HWND window,
                             int  width,
                             int  height)
{
   Evas_Engine_WinCE_GDI_Priv *priv;

   priv = (Evas_Engine_WinCE_GDI_Priv *)malloc(sizeof(Evas_Engine_WinCE_GDI_Priv));
   if (!priv)
     return NULL;

   priv->window = window;
   priv->dc = GetDC(window);
   if (!priv->dc)
     {
        fprintf (stderr, "[Evas] [Engine] [WinCE GDI] Can not get DC\n");
        free(priv);
        return NULL;
     }

   priv->width = GetSystemMetrics(SM_CXSCREEN);
   priv->height = GetSystemMetrics(SM_CYSCREEN);

   if ((priv->width != width) ||
       (priv->height != height))
     {
        fprintf (stderr, "[Evas] [Engine] [WinCE GDI] Size mismatch\n");
        fprintf (stderr, "[Evas] [Engine] [WinCE GDI] asked: %dx%d\n", width, height);
        fprintf (stderr, "[Evas] [Engine] [WinCE GDI] got  : %dx%d\n", priv->width, priv->height);
        ReleaseDC(window, priv->dc);
        free(priv);
        return NULL;
     }

   priv->bitmap_info = (BITMAPINFO_16bpp *)malloc(sizeof(BITMAPINFO_16bpp));
   if (!priv->bitmap_info)
     {
        ReleaseDC(window, priv->dc);
        free(priv);
        return NULL;
     }

   priv->bitmap_info->bih.biSize = sizeof(BITMAPINFOHEADER);
   priv->bitmap_info->bih.biWidth = priv->width;
   priv->bitmap_info->bih.biHeight = -priv->height;
   priv->bitmap_info->bih.biPlanes = 1;
   priv->bitmap_info->bih.biSizeImage = 2 * priv->width * priv->height;
   priv->bitmap_info->bih.biXPelsPerMeter = 0;
   priv->bitmap_info->bih.biYPelsPerMeter = 0;
   priv->bitmap_info->bih.biClrUsed = 0;
   priv->bitmap_info->bih.biClrImportant = 0;
   priv->bitmap_info->bih.biBitCount = 16;
   priv->bitmap_info->bih.biCompression = BI_BITFIELDS;
   priv->bitmap_info->masks[0] = 0x0000f800;
   priv->bitmap_info->masks[1] = 0x000007e0;
   priv->bitmap_info->masks[2] = 0x0000001f;

   return priv;
}

void
evas_software_wince_gdi_shutdown(void *priv)
{
   free(((Evas_Engine_WinCE_GDI_Priv *)priv)->bitmap_info);
   ReleaseDC(((Evas_Engine_WinCE_GDI_Priv *)priv)->window, ((Evas_Engine_WinCE_GDI_Priv *)priv)->dc);
   free(priv);
}


FB_Output_Buffer *
evas_software_wince_gdi_output_buffer_new(void *priv,
                                          int   width,
                                          int   height)
{
   Evas_Engine_WinCE_GDI_Priv *priv2;
   FB_Output_Buffer           *fbob;
   void                       *buffer;

   fbob = calloc(1, sizeof(FB_Output_Buffer));
   if (!fbob) return NULL;

   fbob->priv = priv;

   priv2 = (Evas_Engine_WinCE_GDI_Priv *)fbob->priv;

   priv2->bitmap = CreateDIBSection(priv2->dc,
                                    (const BITMAPINFO *)priv2->bitmap_info,
                                    DIB_RGB_COLORS,
                                    (void **)(&buffer),
                                    NULL,
                                    0);
   if (!priv2->bitmap)
     {
        free(fbob);
        return NULL;
     }

   fbob->im = (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), width, height, (DATA32 *)buffer, 0, EVAS_COLORSPACE_RGB565_A5P);
   if (fbob->im)
     fbob->im->stride = width;

   return fbob;
}

void
evas_software_wince_gdi_output_buffer_free(FB_Output_Buffer *fbob)
{
   Evas_Engine_WinCE_GDI_Priv *priv;

   priv = (Evas_Engine_WinCE_GDI_Priv *)fbob->priv;
   DeleteObject(priv->bitmap);
   free(fbob);
}

void
evas_software_wince_gdi_output_buffer_paste(FB_Output_Buffer *fbob)
{
   Evas_Engine_WinCE_GDI_Priv *priv;

   priv = (Evas_Engine_WinCE_GDI_Priv *)fbob->priv;

   if ((fbob->im->cache_entry.w == priv->width) &&
       (fbob->im->cache_entry.h == priv->height))
     {
        HDC     dc;

        dc = CreateCompatibleDC(priv->dc);
        SelectObject(dc, priv->bitmap);
        BitBlt(priv->dc,
               0, 0,
               priv->width, priv->height,
               dc,
               0, 0,
               SRCCOPY);
        DeleteDC(dc);

     }
}

void
evas_software_wince_gdi_surface_resize(FB_Output_Buffer *fbob)
{
}
