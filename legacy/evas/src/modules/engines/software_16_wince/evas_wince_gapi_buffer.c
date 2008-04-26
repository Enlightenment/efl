#include "evas_common.h"
#include "evas_engine.h"


typedef int (*evas_engine_wince_close_display)();
typedef int (*evas_engine_wince_close_input)();

typedef struct Evas_Engine_WinCE_GAPI_Priv Evas_Engine_WinCE_GAPI_Priv;


#define LINK(type,name,import) \
  name = (gapi_##type)GetProcAddress (gapi_lib, import)

#define GX_FULLSCREEN 0x01
#define GX_NORMALKEYS 0x02

#define kfDirect555   0x40
#define kfDirect565   0x80


typedef struct
{
   DWORD cxWidth;
   DWORD cyHeight;
   LONG  cbxPitch;
   LONG  cbyPitch;
   LONG  cBPP;
   DWORD ffFormat;
} _GAPI_Display_Properties;

typedef struct
{
  short vkUp;        // key for up
  POINT ptUp;        // x,y position of key/button.  Not on screen but in screen coordinates.
  short vkDown;
  POINT ptDown;
  short vkLeft;
  POINT ptLeft;
  short vkRight;
  POINT ptRight;
  short vkA;
  POINT ptA;
  short vkB;
  POINT ptB;
  short vkC;
  POINT ptC;
  short vkStart;
  POINT ptStart;
} _GAPI_Key_List;

typedef int                      (*gapi_display_open)(HWND hWnd, DWORD dwFlags);
typedef int                      (*gapi_display_close)();
typedef _GAPI_Display_Properties (*gapi_display_properties_get)(void);
typedef void*                    (*gapi_draw_begin)(void);
typedef int                      (*gapi_draw_end)(void);
typedef int                      (*gapi_input_open)(void);
typedef int                      (*gapi_input_close)(void);
typedef _GAPI_Key_List           (*gapi_default_keys_get)(int iOptions);
typedef int                      (*gapi_suspend)(void);
typedef int                      (*gapi_resume)(void);

gapi_default_keys_get default_keys_get = NULL;
gapi_suspend          suspend = NULL;
gapi_resume           resume = NULL;

int
evas_software_wince_gapi_suspend(void)
{
   return suspend();
}

int
evas_software_wince_gapi_resume(void)
{
   return resume();
}

void *
evas_software_wince_gapi_default_keys(void)
{
   _GAPI_Key_List  key_list;
   _GAPI_Key_List *keys;

   keys = (_GAPI_Key_List *)malloc(sizeof(_GAPI_Key_List));
   if (!keys)
     return NULL;

   key_list = default_keys_get(GX_NORMALKEYS);
   memcpy(keys, &key_list, sizeof(_GAPI_Key_List));

   return keys;
}


struct Evas_Engine_WinCE_GAPI_Priv
{
   HMODULE            lib;
   gapi_display_close close_display;
   gapi_input_close   close_input;
   gapi_draw_begin    draw_begin;
   gapi_draw_end      draw_end;
   int                width;
   int                height;
};

void *
evas_software_wince_gapi_init (HWND window)
{
   _GAPI_Display_Properties     prop;
   _GAPI_Key_List               key_list;
   HMODULE                      gapi_lib;
   Evas_Engine_WinCE_GAPI_Priv *priv;

   gapi_display_open            display_open = NULL;
   gapi_display_close           display_close = NULL;
   gapi_display_properties_get  display_properties_get = NULL;
   gapi_draw_begin              draw_begin = NULL;
   gapi_draw_end                draw_end = NULL;
   gapi_input_open              input_open = NULL;
   gapi_input_close             input_close = NULL;

   priv = (Evas_Engine_WinCE_GAPI_Priv *)malloc(sizeof(Evas_Engine_WinCE_GAPI_Priv));
   if (!priv)
     return NULL;

   gapi_lib = LoadLibrary(L"\\Windows\\gx.dll");
   if (!gapi_lib)
     {
        gapi_lib = LoadLibrary(L"gx.dll");
        if (!gapi_lib) {
           printf ("error : LoadLibrary\n");
           goto free_priv;
        }
     }

   LINK(display_open, display_open, L"?GXOpenDisplay@@YAHPAUHWND__@@K@Z");
   LINK(display_close, display_close, L"?GXCloseDisplay@@YAHXZ");
   LINK(display_properties_get, display_properties_get, L"?GXGetDisplayProperties@@YA?AUGXDisplayProperties@@XZ");
   LINK(draw_begin, draw_begin, L"?GXBeginDraw@@YAPAXXZ");
   LINK(draw_end, draw_end, L"?GXEndDraw@@YAHXZ");
   LINK(input_open, input_open, L"?GXOpenInput@@YAHXZ" );
   LINK(input_close, input_close, L"?GXCloseInput@@YAHXZ" );
   LINK(default_keys_get, default_keys_get, L"?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z");
   LINK(suspend, suspend, L"?GXSuspend@@YAHXZ" );
   LINK(resume, resume, L"?GXResume@@YAHXZ" );

   if (!display_open ||
       !display_close ||
       !display_properties_get ||
       !draw_begin ||
       !draw_end ||
       !input_open ||
       !input_close ||
       !default_keys_get ||
       !suspend ||
       !resume)
     {
        printf ("error : no valid symbols\n");
        goto free_lib;
     }

   if (!display_open(window, GX_FULLSCREEN))
     {
        printf ("error : GXOpenDisplay\n");
        goto free_lib;
     }

   prop = display_properties_get();

   // verify pixel format
   if(!(prop.ffFormat & kfDirect565) || (prop.cBPP != 16))
     {
        printf ("error : GAPI format mismatch\n");
        goto close_display;
     }

   // verify we have a vga device
   if ((GetSystemMetrics(SM_CXSCREEN) != (int)prop.cxWidth) ||
       (GetSystemMetrics(SM_CYSCREEN) != (int)prop.cyHeight))
     {
        printf ("error : GAPI format mismatch\n");
        goto close_display;
     }

   if (!input_open())
     {
        printf ("error : GXOpenInput\n");
        goto close_display;
     }

   priv->lib = gapi_lib;
   priv->close_display = display_close;
   priv->close_input = input_close;
   priv->draw_begin = draw_begin;
   priv->draw_end = draw_end;
   priv->width = prop.cxWidth;
   priv->height = prop.cyHeight;

   return priv;

 close_display:
   display_close();
 free_lib:
   FreeLibrary(gapi_lib);
 free_priv:
   free(priv);
   return NULL;
}

void
evas_software_wince_gapi_shutdown(void *priv)
{
   Evas_Engine_WinCE_GAPI_Priv *p;

   p = (Evas_Engine_WinCE_GAPI_Priv *)priv;
   p->close_input();
   p->close_display();
   FreeLibrary(p->lib);
   free(p);
}


FB_Output_Buffer *
evas_software_wince_gapi_output_buffer_new(void *priv,
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
evas_software_wince_gapi_output_buffer_free(FB_Output_Buffer *fbob)
{
   free(fbob->im->pixels);
   free(fbob);
}

void
evas_software_wince_gapi_output_buffer_paste(FB_Output_Buffer *fbob)
{
   Evas_Engine_WinCE_GAPI_Priv *priv;
   void                        *buffer;

   priv = (Evas_Engine_WinCE_GAPI_Priv *)fbob->priv;

   buffer = priv->draw_begin();
   if (!buffer)
     return;

   if ((fbob->im->cache_entry.w == priv->width) &&
       (fbob->im->cache_entry.h == priv->height))
     memcpy(buffer, fbob->im->pixels,
            priv->width * priv->height * 2);

   priv->draw_end();
}

void
evas_software_wince_gapi_surface_resize(FB_Output_Buffer *ddob)
{
}
