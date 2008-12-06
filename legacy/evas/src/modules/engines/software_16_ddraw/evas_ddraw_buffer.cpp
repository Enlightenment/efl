#include "evas_common.h"
#include "evas_engine.h"


DDraw_Output_Buffer *
evas_software_ddraw_output_buffer_new(HWND                window,
                                      LPDIRECTDRAW        object,
                                      LPDIRECTDRAWSURFACE surface_primary,
                                      LPDIRECTDRAWSURFACE surface_back,
                                      LPDIRECTDRAWSURFACE surface_source,
                                      int                 width,
                                      int                 height)
{
   DDSURFACEDESC        surface_desc;
   DDraw_Output_Buffer *ddob;

   ddob = (DDraw_Output_Buffer *)calloc(1, sizeof(DDraw_Output_Buffer));
   if (!ddob) return NULL;

   ddob->dd.window = window;
   ddob->dd.object = object;
   ddob->dd.surface_primary = surface_primary;
   ddob->dd.surface_back = surface_back;
   ddob->dd.surface_source = surface_source;
   ddob->width = width;
   ddob->height = height;
   ddob->pitch = width * 2;

   ZeroMemory(&surface_desc, sizeof(surface_desc));
   surface_desc.dwSize = sizeof(surface_desc);

   if (FAILED(ddob->dd.surface_source->Lock(NULL,
                                            &surface_desc,
                                            DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
                                            NULL)))
     {
        free(ddob);
        return NULL;
     }

   ddob->data = (DATA16 *)surface_desc.lpSurface;

   if (FAILED(ddob->dd.surface_source->Unlock(NULL)))
     {
        free(ddob);
        return NULL;
     }
   if (ddob->im)
     evas_cache_image_drop(&ddob->im->cache_entry);

   ddob->im =  (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), width, height, (DATA32 *) ddob->data, 0, EVAS_COLORSPACE_RGB565_A5P);
   if (ddob->im)
     ddob->im->stride = ddob->pitch;

   return ddob;
}

void
evas_software_ddraw_output_buffer_free(DDraw_Output_Buffer *ddob, int sync)
{
   free(ddob);
}

void
evas_software_ddraw_output_buffer_paste(DDraw_Output_Buffer *ddob)
{
   RECT  dst_rect;
   RECT  src_rect;
   POINT p;

   SetRect(&src_rect, 0, 0, ddob->width, ddob->height);

   if (FAILED(ddob->dd.surface_back->BltFast(0, 0,
                                             ddob->dd.surface_source,
                                             &src_rect,
                                             DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT)))
     return;

   p.x = 0;
   p.y = 0;
   ClientToScreen(ddob->dd.window, &p);
   GetClientRect(ddob->dd.window, &dst_rect);
   OffsetRect(&dst_rect, p.x, p.y);
   ddob->dd.surface_primary->Blt(&dst_rect,
                                 ddob->dd.surface_back, &src_rect,
                                 DDBLT_WAIT, NULL);
}
