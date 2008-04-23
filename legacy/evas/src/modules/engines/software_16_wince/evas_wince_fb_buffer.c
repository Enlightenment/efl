#include "evas_common.h"
#include "evas_engine.h"


#define GETRAWFRAMEBUFFER 0x00020001

typedef struct _RawFrameBufferInfo
{
   WORD  wFormat;
   WORD  wBPP;
   VOID *pFramePointer;
   int   cxStride;
   int   cyStride;
   int   cxPixels;
   int   cyPixels;
} RawFrameBufferInfo;


typedef struct Evas_Engine_WinCE_FB_Priv Evas_Engine_WinCE_FB_Priv;

struct Evas_Engine_WinCE_FB_Priv
{
   int   width;
   int   height;
   void *buffer;
};


void *
evas_software_wince_fb_init (HWND   window)
{
   RawFrameBufferInfo         rfbi;
   HDC                        dc;
   Evas_Engine_WinCE_FB_Priv *priv;

   priv = (Evas_Engine_WinCE_FB_Priv *)malloc(sizeof(Evas_Engine_WinCE_FB_Priv));
   if (!priv)
     return NULL;

   dc = GetDC (window);
   if (!dc)
     {
        free(priv);
        return NULL;
     }

   if (!ExtEscape(dc, GETRAWFRAMEBUFFER, 0, 0, sizeof(rfbi), (char *) &rfbi)||
       (rfbi.wBPP != 16) ||
       (rfbi.wFormat != 1))
     {
        ReleaseDC(window, dc);
        free(priv);
        return NULL;
     }

  priv->width = rfbi.cxPixels;
  priv->height = rfbi.cyPixels;
  priv->buffer = rfbi.pFramePointer;

  ReleaseDC(window, dc);

  return priv;
}

void
evas_software_wince_fb_shutdown(void *priv)
{
   free(priv);
}


FB_Output_Buffer *
evas_software_wince_fb_output_buffer_new(void *priv,
                                         int   width,
                                         int   height)
{
   FB_Output_Buffer *fbob;
   void             *buffer;

   fbob = calloc(1, sizeof(FB_Output_Buffer));
   if (!fbob) return NULL;

   buffer = malloc (width * height * 2); /* we are sure to have 16bpp */
   if (!buffer)
     {
        free(fbob);
        return NULL;
     }

   fbob->priv = priv;

   fbob->im = (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), width, height, (DATA32 *)buffer, 0, EVAS_COLORSPACE_RGB565_A5P);
   if (fbob->im)
     fbob->im->stride = width;

   return fbob;
}

void
evas_software_wince_fb_output_buffer_free(FB_Output_Buffer *fbob)
{
   free(fbob->im->pixels);
   free(fbob);
}

void
evas_software_wince_fb_output_buffer_paste(FB_Output_Buffer *fbob)
{
   Evas_Engine_WinCE_FB_Priv *priv;

   priv = (Evas_Engine_WinCE_FB_Priv *)fbob->priv;

   if ((fbob->im->cache_entry.w == priv->width) &&
       (fbob->im->cache_entry.h == priv->height))
     memcpy(priv->buffer, fbob->im->pixels,
            priv->width * priv->height * 2);
}

void
evas_software_wince_fb_surface_resize(FB_Output_Buffer *ddob)
{
}
