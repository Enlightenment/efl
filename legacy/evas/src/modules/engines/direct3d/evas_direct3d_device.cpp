//#define ENABLE_LOG_PRINTF

#include "evas_direct3d_device.h"

#include "evas_direct3d_vertex_buffer_cache.h"

D3DDevice::D3DDevice()
{
   ResetParams();
}

bool D3DDevice::Init(HWND window, int depth, bool fullscreen)
{
   D3DPRESENT_PARAMETERS pp;
   D3DDISPLAYMODE dm;
   D3DCAPS9 caps;
   RECT rect;
   DWORD flag;
   HRESULT hr;

   if (window == NULL)
      return false;

   Destroy();

   _object = Direct3DCreate9(D3D_SDK_VERSION);
   if (_object == NULL)
     return false;

   if (FAILED(hr = _object->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm)))
     {
     Log("GetAdapterDisplayMode failed: %x", hr);
     Destroy();
     return false;
     }

   if (FAILED(hr = _object->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps)))
     {
     Log("GetDeviceCaps failed: %x", hr);
     Destroy();
     return false;
     }

   if (!GetClientRect(window, &rect))
     {
     Log("GetClientRect failed: %x", GetLastError());
     Destroy();
     return false;
     }

   if (SUCCEEDED(_object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
     dm.Format, 0, D3DRTYPE_TEXTURE, (depth == 16) ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8)))
     {
     dm.Format = (depth == 16) ? D3DFMT_R5G6B5 : D3DFMT_A8R8G8B8;
     }

   flag = (caps.VertexProcessingCaps != 0) ?
     (D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE) :
     D3DCREATE_SOFTWARE_VERTEXPROCESSING;

   ZeroMemory(&pp, sizeof(pp));
   if (!fullscreen)
   {
      pp.BackBufferWidth = rect.right - rect.left;
      pp.BackBufferHeight = rect.bottom - rect.top;
   }
   else
   {
      pp.BackBufferWidth = ::GetSystemMetrics(SM_CXSCREEN);
      pp.BackBufferHeight = ::GetSystemMetrics(SM_CYSCREEN);
   }
   pp.BackBufferFormat = dm.Format;
   pp.BackBufferCount = 1;
   pp.MultiSampleType = D3DMULTISAMPLE_NONE;
   pp.MultiSampleQuality = 0;
   pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
   pp.hDeviceWindow = window;
   pp.Windowed  = fullscreen ? FALSE : TRUE;
   //pp.EnableAutoDepthStencil = TRUE;
   //pp.AutoDepthStencilFormat = D3DFMT_D16;
   pp.FullScreen_RefreshRateInHz = 0;
   pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

   if (FAILED(hr = _object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
     window, flag, &pp, &_device)))
     {
     Log("CreateDevice failed: %x", hr);
     Destroy();
     return false;
     }

   LPDIRECT3DSURFACE9 backbuffer = NULL;
   _device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
   backbuffer->GetDesc(&_backbuffer_desc);
   backbuffer->Release();

   switch (dm.Format) {
   case D3DFMT_A8R8G8B8:
   case D3DFMT_X8R8G8B8:
     _depth = 32;
     break;
   case D3DFMT_R5G6B5:
     _depth = 16;
     break;
   default:
     Log("No supported format found");
     Destroy();
     return false;
   }

   //_render_to_texture = false;

   _d3dpp = pp;
   _device_lost = FALSE;
   _scene_rendering = FALSE;
   _width = rect.right - rect.left;
   _height = rect.bottom - rect.top;
   _window = window;

   if (FAILED(CreateRenderTarget()))
   {
      Log("Failed to create render target");
      Destroy();
      return false;
   }

   Log("initialized");
   return true;
}

bool D3DDevice::Reset(int width, int height, int fullscreen)
{
   D3DPRESENT_PARAMETERS pp = _d3dpp;
   _d3dpp.BackBufferWidth = (width > 0) ? width : _d3dpp.BackBufferWidth;
   _d3dpp.BackBufferHeight = (height > 0) ? height : _d3dpp.BackBufferHeight;
   _d3dpp.Windowed = (fullscreen == 1) ? FALSE : ((fullscreen == 0) ? TRUE : _d3dpp.Windowed);
   if (FAILED(ResetDevice()))
   {
      Log("Couldnt restore device");
      _d3dpp = pp;
      return SUCCEEDED(ResetDevice());
   }
   _width = _d3dpp.BackBufferWidth;
   _height = _d3dpp.BackBufferHeight;
   return true;
}

void D3DDevice::Destroy()
{
   //if (_render_target != NULL)
   //{
   //   _render_target->Release();
   //   _render_target = NULL;
   //}
   if (_render_target_data != NULL)
   {
      _render_target_data->Release();
      _render_target_data = NULL;
   }
   if (_device != NULL)
   {
      int num = _device->Release();
      assert(num == 0);
   }
   if (_object != NULL)
      _object->Release();
   ResetParams();

   Log("uninitialized");
}

void D3DDevice::ResetParams()
{
   _window = NULL;
   _object = NULL;
   _device = NULL;
   _width = 0;
   _height = 0;
   _rot = 0;
   _depth = 0;
   _device_lost = false;
   _scene_rendering = false;
   ZeroMemory(&_d3dpp, sizeof(_d3dpp));
   ZeroMemory(&_backbuffer_desc, sizeof(_backbuffer_desc));
   //_render_target = NULL;
   _render_target_data = NULL;
   _render_data_updated = false;
   _render_data.Resize();
   //_original_render_target = NULL;
   //_render_to_texture = false;
}

HRESULT D3DDevice::RestoreDevice()
{
   Log("restore");
   assert(_device != NULL);

   HRESULT hr = S_OK;

   // Test the cooperative level to see if it's okay to render
   if (SUCCEEDED(hr = _device->TestCooperativeLevel()))
   {
      _device_lost = FALSE;
      Log("render test ok");
      return S_OK;
   }

   // If the device was lost, do not render until we get it back
   if (hr == D3DERR_DEVICELOST)
      return E_FAIL;

   // Check if the device needs to be reset.
   if (hr == D3DERR_DEVICENOTRESET)
   {
      if (FAILED(hr = ResetDevice()))
         return hr;
   }
   return hr;
}

HRESULT D3DDevice::ResetDevice()
{
   Log("reset");
   HRESULT hr = S_OK;

   _scene_rendering = FALSE;

   // Release all video memory objects
   // Bad to call such, make better
   D3DVertexBufferCache::Current()->Uninitialize();

   //if (_render_target != NULL)
   //{
   //   _render_target->Release();
   //   _render_target = NULL;
   //}
   if (_render_target_data != NULL)
   {
      _render_target_data->Release();
      _render_target_data = NULL;
   }

   // Reset the device
   if (FAILED(hr = _device->Reset(&_d3dpp)))
   {
      Log("D3DDevice: Reset of the device failed! Error (%X)", (DWORD)hr);
      return hr;
   }

   // Store render target surface desc
   LPDIRECT3DSURFACE9 backbuffer = NULL;
   _device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
   if (backbuffer != NULL)
   {
      backbuffer->GetDesc(&_backbuffer_desc);
      backbuffer->Release();
   }

   // Initialize the app's device-dependent objects
   hr = CreateRenderTarget();

   if (FAILED(hr))
   {
      Log("Restoration of device objects failed");
      // Invalidate objects

      return E_FAIL;
   }

   Log("Device objects were successfuly restored");
   _textures.Set(NULL);

   //_device_objects_restored = true;
   return S_OK;
}

bool D3DDevice::Begin()
{
   if (FAILED(RestoreDevice()))
      return false;

   //if (_render_to_texture && _render_target != NULL)
   //{
   //   if (FAILED(_device->GetRenderTarget(0, &_original_render_target)))
   //      return false;
   //   if (FAILED(_device->SetRenderTarget(0, _render_target)))
   //      return false;
   //}

   HRESULT hr;
   if (FAILED(hr = _device->BeginScene()))
   {
      Log("Cannot begin scene: %X", (DWORD)hr);
      return false;
   }

   //static const D3DVIEWPORT9 vp = {0, 0, _width, _height, 0.f, 1.f};
   //_device->SetViewport(&vp);
   //_device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

   //_device->Clear(0, NULL, D3DCLEAR_TARGET /*| D3DCLEAR_ZBUFFER*/, 0xff8080ff, 1.f, 0);
   return true;
}

bool D3DDevice::End()
{
   _device->EndScene();
   _device->Present(NULL, NULL, NULL, NULL);

   _render_data_updated = false;

   //if (_render_to_texture && _render_target != NULL && _original_render_target != NULL)
   //{
   //   if (FAILED(_device->SetRenderTarget(0, _original_render_target)))
   //      return false;
   //}

   return true;
}

TArray<DWORD> &D3DDevice::GetRenderData()
{
   if (_render_data_updated)
      return _render_data;
   _render_data.Allocate(0);
   if (_render_target_data == NULL)
      return _render_data;

   LPDIRECT3DSURFACE9 surf = NULL;
   HRESULT hr;
   if (FAILED(_device->GetRenderTarget(0, &surf)))
      return _render_data;
   if (FAILED(hr = _device->GetRenderTargetData(surf, _render_target_data)))
   {
      Log("Failed to get render target data (%X)", (DWORD)hr);
      surf->Release();
      return _render_data;
   }
   D3DLOCKED_RECT lr;
   if (FAILED(_render_target_data->LockRect(&lr, NULL, D3DLOCK_READONLY)))
   {
      surf->Release();
      return _render_data;
   }
   _render_data.Allocate(_width * _height);

   for (int i = 0; i < _height; i++)
   {
      CopyMemory(&_render_data[i * _width], (BYTE *)lr.pBits + i * lr.Pitch,
         _width * sizeof(DWORD));
   }

   _render_target_data->UnlockRect();
   _render_data_updated = true;
   surf->Release();
   return _render_data;
}

HRESULT D3DDevice::SetTexture(DWORD stage, LPDIRECT3DTEXTURE9 tex)
{
   if (stage >= 8)
      return E_FAIL;
   if (_textures.Length() <= (int)stage)
      _textures.Allocate(stage + 1);
   if (_textures[stage] != tex)
   {
      _textures[stage] = tex;
      return _device->SetTexture(stage, tex);
   }
   return S_OK;
}

HRESULT D3DDevice::CreateRenderTarget()
{
   if (_device == NULL)
      return E_FAIL;
   //if (_render_target != NULL &&
   if (_render_target_data != NULL)
      return S_OK;

   //if (FAILED(_device->CreateRenderTarget(_width, _height, _backbuffer_desc.Format,
   //   D3DMULTISAMPLE_NONE, 0, FALSE, &_render_target, NULL)))
   //{
   //   return E_FAIL;
   //}
   if (FAILED(_device->CreateOffscreenPlainSurface(_backbuffer_desc.Width,
      _backbuffer_desc.Height, _backbuffer_desc.Format, D3DPOOL_SYSTEMMEM,
      &_render_target_data, NULL)))
   {
      return E_FAIL;
   }
   return S_OK;
}
