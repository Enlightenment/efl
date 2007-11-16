/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "config.h"
#include "ecore_win32_private.h"
#include "Ecore_Win32.h"
#include <cstdio>


extern "C" {


EAPI int
ecore_win32_direct3d_init(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   D3DPRESENT_PARAMETERS       pp;
   D3DDISPLAYMODE              dm;
   D3DSURFACE_DESC             sd;
   D3DCAPS9                    caps;
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       flag;

   if (!window)
     return 0;

   w = (struct _Ecore_Win32_Window *)window;

   printf ("ecore_win32_direct3d_init debut : %p (%d %d) (%d %d)\n",
           w,
           w->min_width,
           w->min_height,
           w->max_width,
           w->max_height);

   w->d3d.object = Direct3DCreate9 (D3D_SDK_VERSION);
   if (!w->d3d.object)
     return 0;

   if (FAILED (w->d3d.object->GetAdapterDisplayMode (D3DADAPTER_DEFAULT,
                                                     &dm)))
     goto no_get_adapter;

   if (FAILED (w->d3d.object->GetDeviceCaps (D3DADAPTER_DEFAULT,
                                             D3DDEVTYPE_HAL,
                                             &caps)))
     goto no_caps;

   if (!GetClientRect(w->window, &rect))
     goto no_get_client;

   flag = (caps.VertexProcessingCaps != 0)
     ? D3DCREATE_HARDWARE_VERTEXPROCESSING
     : D3DCREATE_SOFTWARE_VERTEXPROCESSING;

   ZeroMemory(&pp, sizeof(pp));
   pp.BackBufferWidth = rect.right - rect.left;
   pp.BackBufferHeight = rect.bottom - rect.top;
   pp.BackBufferFormat = dm.Format;
   pp.BackBufferCount = 1;
   pp.MultiSampleType = D3DMULTISAMPLE_NONE;
   pp.MultiSampleQuality = 0;
   pp.SwapEffect = D3DSWAPEFFECT_FLIP;
   pp.hDeviceWindow = w->window;
   pp.Windowed  = TRUE;
   pp.EnableAutoDepthStencil = FALSE;
   pp.FullScreen_RefreshRateInHz = 0;
   pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

   if (FAILED(w->d3d.object->CreateDevice (D3DADAPTER_DEFAULT,
                                           D3DDEVTYPE_HAL,
                                           w->window,
                                           flag,
                                           &pp,
                                           &w->d3d.device)))
     goto no_device;

   if (FAILED (D3DXCreateSprite (w->d3d.device, &w->d3d.sprite)))
     goto no_sprite;

   if (FAILED (w->d3d.device->CreateTexture (rect.right - rect.left,
                                             rect.bottom - rect.top,
                                             1,
                                             D3DUSAGE_DYNAMIC,
                                             dm.Format,
                                             D3DPOOL_DEFAULT,
                                             &w->d3d.texture, NULL)))
     goto no_texture;

   if (FAILED (w->d3d.texture->GetLevelDesc (0, &sd)))
     goto no_level_desc;

   switch (sd.Format) {
   case D3DFMT_A8R8G8B8:
   case D3DFMT_X8R8G8B8:
     w->d3d.depth = 32;
     break;
   case D3DFMT_R5G6B5:
     w->d3d.depth = 16;
     break;
   default:
     goto no_supported_depth;
   }

   w->backend = ECORE_WIN32_BACKEND_DIRECT3D;

   printf ("ecore_win32_direct3d_init fin : %p (%d %d) (%d %d)\n",
           w,
           w->min_width,
           w->min_height,
           w->max_width,
           w->max_height);

   return 1;

 no_supported_depth:
 no_level_desc:
   w->d3d.texture->Release();
 no_texture:
   w->d3d.sprite->Release();
 no_sprite:
   w->d3d.device->Release();
 no_device:
 no_get_client:
 no_caps:
 no_get_adapter:
   w->d3d.object->Release();
#endif /* HAVE_DIRECT3D */

   return 0;
}

EAPI void
ecore_win32_direct3d_shutdown(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   struct _Ecore_Win32_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_Win32_Window *)window;

   printf ("0\n");
   if (w->d3d.texture)
     w->d3d.texture->Release();

   printf ("1\n");
//    if (w->d3d.sprite)
//      w->d3d.sprite->OnLostDevice();

   printf ("2\n");
//    if (w->d3d.device)
//      w->d3d.device->Release();

   printf ("3\n");
   if (w->d3d.object)
     w->d3d.object->Release();

   printf ("4\n");
   w->backend = ECORE_WIN32_BACKEND_NONE;
#endif /* HAVE_DIRECT3D */
}

EAPI Ecore_Win32_Direct3D_Object *
ecore_win32_direct3d_object_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->d3d.object;
#else
   return NULL;
#endif /* HAVE_DIRECT3D */
}

EAPI Ecore_Win32_Direct3D_Device *
ecore_win32_direct3d_device_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->d3d.device;
#else
   return NULL;
#endif /* HAVE_DIRECT3D */
}

EAPI Ecore_Win32_Direct3D_Sprite *
ecore_win32_direct3d_sprite_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->d3d.sprite;
#else
   return NULL;
#endif /* HAVE_DIRECT3D */
}

EAPI Ecore_Win32_Direct3D_Texture *
ecore_win32_direct3d_texture_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->d3d.texture;
#else
   return NULL;
#endif /* HAVE_DIRECT3D */
}

EAPI int
ecore_win32_direct3d_depth_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->d3d.depth;
#else
   return 0;
#endif /* HAVE_DIRECT3D */
}

}
