
#include "evas_direct3d_object_line.h"
#include "evas_direct3d_device.h"
#include "evas_direct3d_shader_pack.h"

TArray<D3DObjectLine::Vertex> D3DObjectLine::_cache;
bool D3DObjectLine::_cache_enabled = false;

D3DObjectLine::D3DObjectLine()
{
   _x1 = _y1 = 0;
   _x2 = _y2 = 0;
}

void D3DObjectLine::BeginCache()
{
   _cache.Allocate(0);
   _cache_enabled = true;
}

void D3DObjectLine::EndCache(D3DDevice *d3d)
{
   if (!_cache_enabled || _cache.Length() == 0)
      return;
   D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYC);
   D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_COLOR);
   D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_COLOR);
   d3d->GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, _cache.Length() / 2, 
      _cache.Data(), sizeof(Vertex));

   Log("Line cache drawn: %d items", _cache.Length() / 2);
   _cache_enabled = false;
}

void D3DObjectLine::Draw(D3DDevice *d3d)
{
   Vertex data[2] = {{_x1, _y1, _color}, {_x2, _y2, _color}};

   if (!_cache_enabled)
   {
      D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYC);
      D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_COLOR);
      D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_COLOR);
      d3d->GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 1, data, sizeof(Vertex));
   }
   else
   {
      _cache.Add(data[0]);
      _cache.Add(data[1]);
   }
}

void D3DObjectLine::Setup(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, DWORD color)
{
   _x1 = x1;
   _y1 = y1;
   _x2 = x2;
   _y2 = y2;
   _color = color;
}
