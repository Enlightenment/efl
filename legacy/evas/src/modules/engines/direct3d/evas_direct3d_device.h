#ifndef __EVAS_DIRECT3D_DEVICE_H__
#define __EVAS_DIRECT3D_DEVICE_H__

#include "evas_engine.h"

#include <assert.h>

#include "ref.h"
#include "array.h"


class D3DDevice : virtual public Referenc
{
public:
   D3DDevice();

   bool Init(HWND window, int depth, bool fullscreen = false);
   bool Reset(int width, int height, int fullscreen);
   void Destroy();
   bool Begin();
   bool End();

   inline LPDIRECT3DDEVICE9 GetDevice();
   inline int GetWidth();
   inline int GetHeight();
   inline HWND GetWindow();
   inline bool GetFullscreen();

   TArray<DWORD> &GetRenderData();

   HRESULT SetTexture(DWORD stage, LPDIRECT3DTEXTURE9 tex);

private:
   HRESULT RestoreDevice();
   HRESULT ResetDevice();
   void ResetParams();

   HRESULT CreateRenderTarget();

private:
   HWND _window;
   LPDIRECT3D9 _object;
   LPDIRECT3DDEVICE9 _device;
   int _width;
   int _height;
   int _rot;
   int _depth;
   bool _device_lost;
   bool _scene_rendering;
   D3DPRESENT_PARAMETERS _d3dpp;
   D3DSURFACE_DESC _backbuffer_desc;
   //LPDIRECT3DSURFACE9 _render_target;
   LPDIRECT3DSURFACE9 _render_target_data;
   //LPDIRECT3DSURFACE9 _original_render_target;
   //bool _render_to_texture;
   TArray<DWORD> _render_data;
   bool _render_data_updated;

   TArray<LPDIRECT3DTEXTURE9> _textures;
};

LPDIRECT3DDEVICE9 D3DDevice::GetDevice()
{
   return _device;
}

int D3DDevice::GetWidth()
{
   return _width;
}

int D3DDevice::GetHeight()
{
   return _height;
}

HWND D3DDevice::GetWindow()
{
   return _window;
}

bool D3DDevice::GetFullscreen()
{
   return (_d3dpp.Windowed == 0);
}

#endif  // __EVAS_DIRECT3D_DEVICE_H__
