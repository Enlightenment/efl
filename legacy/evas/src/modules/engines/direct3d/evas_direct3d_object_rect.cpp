#include "evas_direct3d_object_rect.h"
#include "evas_direct3d_device.h"
#include "evas_direct3d_shader_pack.h"

TArray<D3DObjectRect::Vertex> D3DObjectRect::_cache;
bool D3DObjectRect::_cache_enabled = false;

D3DObjectRect::D3DObjectRect()
{
   _x = _y = 0;
   _w = _h = 0;
}

void D3DObjectRect::BeginCache()
{
   _cache.Allocate(0);
   _cache_enabled = true;
}

void D3DObjectRect::EndCache(D3DDevice *d3d)
{
   if (!_cache_enabled || _cache.Length() == 0)
      return;
   D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYC);
   D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_COLOR);
   D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_COLOR);
   d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, _cache.Length() / 3,
      _cache.Data(), sizeof(Vertex));

   Log("Rect cache drawn: %d items", _cache.Length() / 6);
   _cache_enabled = false;
}

void D3DObjectRect::Draw(D3DDevice *d3d)
{
   Vertex data[6] = {
      {_x, _y, _color}, {_x + _w, _y, _color}, {_x, _y + _h, _color},
      {_x, _y + _h, _color}, {_x + _w, _y, _color}, {_x + _w, _y + _h, _color}};

   if (!_cache_enabled)
   {
      D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYC);
      D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_COLOR);
      D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_COLOR);
      d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, data, sizeof(Vertex));
   }
   else
   {
      _cache.Add(data[0]);
      _cache.Add(data[1]);
      _cache.Add(data[2]);
      _cache.Add(data[3]);
      _cache.Add(data[4]);
      _cache.Add(data[5]);
   }
}

void D3DObjectRect::Setup(FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD color)
{
   _x = x;
   _y = y;
   _w = w;
   _h = h;
   _color = color;
}
