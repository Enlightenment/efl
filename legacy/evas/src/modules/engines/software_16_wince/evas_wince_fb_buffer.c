#include "evas_common.h"
#include "evas_engine.h"


#define GETGXINFO 0x00020000

typedef struct GXDeviceInfo
{
    long Version;               //00 (should filled with 100 before calling ExtEscape)
    void *pvFrameBuffer;        //04
    unsigned long cbStride;     //08
    unsigned long cxWidth;      //0c
    unsigned long cyHeight;     //10
    unsigned long cBPP;         //14
    unsigned long ffFormat;     //18
    char Unused[0x84 - 7 * 4];
} GXDeviceInfo;


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

static int
_evas_software_wince_gxinfo_init(HDC dc, int *width, int *height, void **buffer)
{
   GXDeviceInfo gxInfo = { 0 };
   int          result;

   gxInfo.Version = 100;
   result = ExtEscape(dc, GETGXINFO, 0, NULL, sizeof(gxInfo),
                      (char *) &gxInfo);
   if (result <= 0)
     {
        fprintf (stderr, "[Evas] [Engine] [WinCE FB] ExtEscape() with GETGXINFO failed\n");
        return 0;
     }

   *width = gxInfo.cyHeight;
   *height = gxInfo.cxWidth;
   *buffer = gxInfo.pvFrameBuffer;

   return 1;
}

void *
evas_software_wince_fb_init(HWND window,
                            int  width,
                            int  height)
{
   WCHAR                      oemstr[100];
   RawFrameBufferInfo         rfbi;
   HDC                        dc;
   Evas_Engine_WinCE_FB_Priv *priv;

   priv = (Evas_Engine_WinCE_FB_Priv *)malloc(sizeof(Evas_Engine_WinCE_FB_Priv));
   if (!priv)
     return NULL;

   dc = GetDC (window);
   if (!dc)
     {
        fprintf (stderr, "[Evas] [Engine] [WinCE FB] Can not get DC\n");
        free(priv);
        return NULL;
     }

   SystemParametersInfo (SPI_GETOEMINFO, sizeof (oemstr), oemstr, 0);
   if (((oemstr[12] == 'H') &&
        (oemstr[13] == '3') &&
        (oemstr[14] == '8')) ||
       ((oemstr[12] == 'H') &&
        (oemstr[13] == '3') &&
        (oemstr[14] == '9')))
     {
       if (!_evas_software_wince_gxinfo_init(dc, &priv->width, &priv->height, &priv->buffer))
          {
             ReleaseDC(window, dc);
             free(priv);
             return NULL;
          }

       if ((priv->width != width) ||
           (priv->height != height))
         {
            fprintf (stderr, "[Evas] [Engine] [WinCE FB] Size mismatch\n");
            fprintf (stderr, "[Evas] [Engine] [WinCE FB] asked: %dx%d\n", width, height);
            fprintf (stderr, "[Evas] [Engine] [WinCE FB] got  : %dx%d\n", priv->width, priv->height);
            ReleaseDC(window, dc);
            free(priv);
            return NULL;
         }

        ReleaseDC(window, dc);

        return priv;
     }

   if (!ExtEscape(dc, GETRAWFRAMEBUFFER, 0, 0, sizeof(rfbi), (char *) &rfbi)||
       (rfbi.wBPP != 16) ||
       (rfbi.wFormat != 1))
     {
        fprintf (stderr, "[Evas] [Engine] [WinCE FB] ExtEscape() with GETRAWFRAMEBUFFER failed\n");
        fprintf (stderr, "[Evas] [Engine] [WinCE FB] trying ExtEscape() with GETGXINFO\n");
        if (!_evas_software_wince_gxinfo_init(dc, &priv->width, &priv->height, &priv->buffer))
          {
             ReleaseDC(window, dc);
             free(priv);
             return NULL;
          }

        ReleaseDC(window, dc);
        return priv;
     }

  priv->width = rfbi.cxPixels;
  priv->height = rfbi.cyPixels;
  priv->buffer = rfbi.pFramePointer;

  if ((priv->width != width) ||
      (priv->height != height))
    {
       fprintf (stderr, "[Evas] [Engine] [WinCE FB] Size mismatch\n");
       fprintf (stderr, "[Evas] [Engine] [WinCE FB] asked: %dx%d\n", width, height);
       fprintf (stderr, "[Evas] [Engine] [WinCE FB] got  : %dx%d\n", priv->width, priv->height);
       ReleaseDC(window, dc);
       free(priv);
       return NULL;
    }

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
evas_software_wince_fb_surface_resize(FB_Output_Buffer *fbob)
{
}
