//#define ENABLE_LOG_PRINTF

#include <string.h>

#include "evas_direct3d_object_image.h"
#include "evas_direct3d_image_cache.h"
#include "evas_direct3d_device.h"
#include "evas_direct3d_shader_pack.h"
#include "evas_direct3d_vertex_buffer_cache.h"

TArray<D3DObjectImage *> D3DObjectImage::_cache;
bool D3DObjectImage::_cache_enabled = false;

D3DObjectImage::D3DObjectImage()
{
   _x = _y = _w = _h = 0;
   _sx = _sy = _sw = _sh = 0;
   _u = _v = _du = _dv = 0;
   _image_id = -1;
   _width = _height = 0;
   _source[0] = 0;
   _color = 0xffffffff;
   _cache_i = 0;
   _border = D3DXVECTOR4(0, 0, 0, 0);
   _uvborder = D3DXVECTOR4(0, 0, 0, 0);
   _with_border = false;
   _dirty = false;
   _image_data_updated = false;
}

D3DObjectImage::~D3DObjectImage()
{
   D3DImageCache::Current()->RemoveImageUser(_image_id);
}

void D3DObjectImage::CopyTo(D3DObjectImage *image)
{
   assert(image != NULL);
   image->_u = _u;
   image->_v = _v;
   image->_du = _du;
   image->_dv = _dv;
   image->_image_id = _image_id;
   image->_width = _width;
   image->_height = _height;
   CopyMemory(image->_source, _source, sizeof(_source));
   D3DImageCache::Current()->AddImageUser(image->_image_id);
}

void D3DObjectImage::BeginCache()
{
   _cache.Allocate(0);
   _cache_enabled = true;
}

void D3DObjectImage::EndCache(D3DDevice *d3d)
{
   if (!_cache_enabled || _cache.Length() == 0)
      return;
   D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYUVC);
   D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_UV_COLOR);
   D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_TEX_COLOR_FILTER);

   static TArray<Vertex> sorted;
   static TArray<GroupDesc> groups;
   sorted.Allocate(0);
   groups.Allocate(0);

   bool found = true;
   while (found)
   {
      found = false;
      int cur_id = -1;
      int num = 0;
      for (int i = 0; i < _cache.Length(); i++)
      {
         // We have processed this
         if (_cache[i]->_image_id < 0)
            continue;
         found = true;
         if (cur_id < 0)
            cur_id = _cache[i]->_image_id;
         if (_cache[i]->_image_id == cur_id)
         {
            if (!_cache[i]->_with_border)
            {
               Vertex *data = _cache[i]->MakeData();
               sorted.Add(data[0]);
               sorted.Add(data[1]);
               sorted.Add(data[2]);
               sorted.Add(data[3]);
               sorted.Add(data[4]);
               sorted.Add(data[5]);
               _cache[i]->_image_id = -_cache[i]->_image_id - 1;
               num++;
            }
            else
            {
               Vertex *data = _cache[i]->MakeDataBorder();
               int last_len = sorted.Length();
               sorted.Allocate(last_len + 6 * 9);
               CopyMemory(&sorted[last_len], data, sizeof(Vertex) * 6 * 9);
               _cache[i]->_image_id = -_cache[i]->_image_id - 1;
               num += 9;
            }
         }
      }
      if (num > 0)
      {
         GroupDesc gd = {num, cur_id};
         groups.Add(gd);
      }
   }

   // Restore ids
   for (int i = 0; i < _cache.Length(); i++)
      _cache[i]->_image_id = -_cache[i]->_image_id - 1;

   D3DVertexBufferCache::CacheEntryInfo ce_info;
   if (!D3DVertexBufferCache::Current()->InitBuffer(d3d, (BYTE *)sorted.Data(),
      sorted.Length() * sizeof(Vertex), ce_info))
   {
      return;
   }
   D3DVertexBufferCache::Current()->SelectBufferToDevice(d3d, ce_info.id, sizeof(Vertex));

   HRESULT hr;
   for (int i = 0, cur = 0; i < groups.Length(); i++)
   {
      if (FAILED(hr = D3DImageCache::Current()->SelectImageToDevice(d3d, groups[i].id)))
      {
         Log("Failed to select texture: %X", (DWORD)hr);
      }
//      d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, groups[i].num * 2,
//         &sorted[cur], sizeof(Vertex));
      d3d->GetDevice()->DrawPrimitive(D3DPT_TRIANGLELIST, cur, groups[i].num * 2);
      cur += groups[i].num * 6;
   }

   Log("Image cache drawn: %d items, %d groups", _cache.Length(), groups.Length());
   _cache_enabled = false;
}

void D3DObjectImage::Draw(D3DDevice *d3d)
{
   _dirty = false;

   Log("Image draw: (%.3f, %.3f, %.3f, %.3f)", _x, _y, _w, _h);

   if (_cache_enabled)
   {
      _cache.Add(this);
      _cache_i = _cache.Length() - 1;
      return;
   }

   D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYUVC);
   D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_UV_COLOR);
   D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_TEX_COLOR_FILTER);
   D3DImageCache::Current()->SelectImageToDevice(d3d, _image_id);

   if (!_with_border)
      d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, MakeData(), sizeof(Vertex));
   else
      d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 18, MakeDataBorder(), sizeof(Vertex));
}

void D3DObjectImage::Init(FLOAT u, FLOAT v, FLOAT du, FLOAT dv,
   int image_id, int width, int height, const char *source)
{
   _u = u;
   _v = v;
   _du = du;
   _dv = dv;
   _image_id = image_id;
   _width = width;
   _height = height;
#ifdef __MINGW32__
   strncpy(_source, source, sizeof(_source) - 1);
#else
   strncpy_s(_source, sizeof(_source), source, sizeof(_source) - 1);
#endif // ! __MINGW32__
}

void D3DObjectImage::Setup(FLOAT x, FLOAT y, FLOAT w, FLOAT h,
   int sx, int sy, int sw, int sh)
{
   if (!_dirty)
   {
      _x = 1.f;
      _y = -1.f;
      _w = _h = 0.f;
      _sx = _sy = 1.f;
      _sw = _sh = 0.f;
   }

   if (!_with_border)
   {
      _x = x;
      _y = y;
      _w = w;
      _h = h;
      _sx = FLOAT(sx) / FLOAT(_width);
      _sy = FLOAT(sy) / FLOAT(_height);
      _sw = FLOAT(sw) / FLOAT(_width);
      _sh = FLOAT(sh) / FLOAT(_height);
   }
   else
   {
      _x = min(_x, x);
      _y = max(_y, y);
      _w += w / 3;
      _h += h / 3;
      _sx = min(_sx, FLOAT(sx) / FLOAT(_width));
      _sy = min(_sy, FLOAT(sy) / FLOAT(_height));
      _sw += FLOAT(sw) / (3.f * FLOAT(_width));
      _sh += FLOAT(sh) / (3.f * FLOAT(_height));
   }
   _dirty = true;

}

void D3DObjectImage::SetupColorFilter(DWORD color)
{
   //_color = ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
   _color = color;
}

D3DObjectImage::Vertex *D3DObjectImage::MakeData()
{
   //FLOAT z = (FLOAT(_cache_i) + 0.5f) / _cache.Length();
   Vertex data[6] = {
      {_x, _y,             _u + _sx * _du, _v + _sy * _dv,                 _color},
      {_x + _w, _y,        _u + (_sx + _sw) * _du, _v + _sy * _dv,         _color},
      {_x, _y + _h,        _u + _sx * _du, _v + (_sy + _sh) * _dv,         _color},
      {_x, _y + _h,        _u + _sx * _du, _v + (_sy + _sh) * _dv,         _color},
      {_x + _w, _y,        _u + (_sx + _sw) * _du, _v + _sy * _dv,         _color},
      {_x + _w, _y + _h,   _u + (_sx + _sw) * _du, _v + (_sy + _sh) * _dv, _color}};
   CopyMemory(_data, data, sizeof(data));
   return _data;
}

D3DObjectImage::Vertex *D3DObjectImage::MakeDataBorder()
{
   //FLOAT z = (FLOAT(_cache_i) + 0.5f) / _cache.Length();
   if (_border.x + _border.z > _w)
      _border.x = _border.z = _w / 2;
   if (_border.y + _border.w < _h)
      _border.y = _border.w = _h / 2;

   FLOAT ul, ut, ur, ub;
   ul = _uvborder.x * _du;
   ut = _uvborder.y * _dv;
   ur = _uvborder.z * _du;
   ub = _uvborder.w * _dv;
   FLOAT bl, bt, br, bb;
   bl = _border.x;
   bt = _border.y;
   br = _border.z;
   bb = _border.w;

   const FLOAT half_x = 0.5f * _du / FLOAT(_width);
   const FLOAT half_y = 0.5f * _dv / FLOAT(_height);

   // Diagonal knots
   Vertex data[4] = {
      {_x, _y,                         _u + _sx * _du + half_x,     _v + _sy * _dv + half_y,         _color},
      {_x + bl, _y + bt,               _u + ul + _sx * _du,         _v + ut + _sy * _dv,             _color},
      {_x + _w - br, _y + _h - bb,     _u - ur + (_sx + _sw) * _du, _v - ub + (_sy + _sh) * _dv,     _color},
      {_x + _w, _y + _h,               _u + (_sx + _sw) * _du - half_x, _v + (_sy + _sh) * _dv - half_y, _color}};

   static const int yshift[6] = {0, 0, 1, 1, 0, 1};
   static const int xshift[6] = {0, 1, 0, 0, 1, 1};

   int vi = 0;
   for (int i = 0; i < 3; i++)
   {
      for (int j = 0; j < 3; j++)
      {
         for (int v = 0; v < 6; v++)
         {
            _data[vi].x = data[xshift[v] + j].x;
            _data[vi].y = data[yshift[v] + i].y;
            _data[vi].u = data[xshift[v] + j].u;
            _data[vi].v = data[yshift[v] + i].v;
            _data[vi].col = data[0].col;
            vi++;
         }
      }
   }

   return _data;
}

void D3DObjectImage::SetupBorder(const D3DXVECTOR4 &world_border, const D3DXVECTOR4 &pix_border)
{
   _border = world_border;
   _uvborder = pix_border;
   _with_border = (_border.x > 0.0001f || _border.y > 0.0001f ||
      _border.z > 0.0001f || _border.w > 0.0001f);
}

bool D3DObjectImage::UpdateImageData(DWORD *image_data)
{
   D3DImageCache::CacheEntryInfo info = {_image_id, _width, _height, _u, _v, _du, _dv};
   _image_data_updated = false;
   return D3DImageCache::Current()->UpdateImageData(info, image_data);
}

DWORD *D3DObjectImage::GetImageData()
{
   if (_image_data_updated)
      return _image_data.Data();
   _image_data_updated = true;
   D3DImageCache::CacheEntryInfo info = {_image_id, _width, _height, _u, _v, _du, _dv};
   D3DImageCache::Current()->GetImageData(info, _image_data);
   return _image_data.Data();
}
