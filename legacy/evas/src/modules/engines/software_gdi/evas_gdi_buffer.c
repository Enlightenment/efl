#include <string.h>

#include "evas_common.h"
#include "evas_engine.h"

Gdi_Output_Buffer *
evas_software_gdi_output_buffer_new(HDC             dc,
                                    BITMAPINFO_GDI *bitmap_info,
                                    int             depth,
                                    int             width,
                                    int             height,
                                    void           *data)
{
   Gdi_Output_Buffer *gdiob;

   gdiob = calloc(1, sizeof(Gdi_Output_Buffer));
   if (!gdiob) return NULL;

   if (!data)
     {
        bitmap_info->bih.biWidth = width;
        bitmap_info->bih.biHeight = -height;
        bitmap_info->bih.biSizeImage = (depth >> 3) * width * height;
        gdiob->bitmap = CreateDIBSection(dc,
                                         (const BITMAPINFO *)bitmap_info,
                                         DIB_RGB_COLORS,
                                         (void **)(&data),
                                         NULL,
                                         0);
        if (!gdiob->bitmap)
          {
             free(gdiob);
             return NULL;
          }
     }

   gdiob->bitmap_info = bitmap_info;
   gdiob->dc = dc;
   gdiob->data = data;
   gdiob->width = width;
   gdiob->height = height;
   gdiob->depth = depth;
   gdiob->pitch = width * (depth >> 3);
/*    gdiob->psize = gdiob->pitch * height; */

   return gdiob;
}

void
evas_software_gdi_output_buffer_free(Gdi_Output_Buffer *gdiob)
{
   DeleteObject(gdiob->bitmap);
   free(gdiob);
}

void
evas_software_gdi_output_buffer_paste(Gdi_Output_Buffer *gdiob,
                                      int                x,
                                      int                y)
{
   HDC     dc;

   dc = CreateCompatibleDC(gdiob->dc);
   if (!dc)
     return;
   SelectObject(dc, gdiob->bitmap);
   BitBlt(gdiob->dc,
          x, y,
          gdiob->width, gdiob->height,
          dc,
          0, 0,
          SRCCOPY);
   DeleteDC(dc);
}

DATA8 *
evas_software_gdi_output_buffer_data(Gdi_Output_Buffer *gdiob,
                                     int               *pitch)
{
   if (pitch) *pitch = gdiob->pitch;
   return gdiob->data;
}

int
evas_software_gdi_output_buffer_depth(Gdi_Output_Buffer *gdiob)
{
   return gdiob->depth;
}
