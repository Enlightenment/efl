//#define ENABLE_LOG_PRINTF

#include <string.h>

#include "evas_direct3d_object_font.h"
#include "evas_direct3d_image_cache.h"
#include "evas_direct3d_device.h"
#include "evas_direct3d_shader_pack.h"
#include "evas_direct3d_vertex_buffer_cache.h"

D3DObjectFont::Cache D3DObjectFont::_cache;

D3DObjectFont::D3DObjectFont(void *source, int image_id)
{
   _image_id = image_id;
   _color = 0xff000000;
   _source = source;
   D3DImageCache::Current()->AddImageUser(_image_id);
}

D3DObjectFont::~D3DObjectFont()
{
   D3DImageCache::Current()->RemoveImageUser(_image_id);
}

void D3DObjectFont::CopyTo(D3DObjectFont *font)
{
   assert(font != NULL);
   font->_image_id = _image_id;
   font->_source = _source;
   D3DImageCache::Current()->AddImageUser(font->_image_id);
   _glyphs.CopyTo(font->_glyphs);
}

void D3DObjectFont::BeginCache(int image_id)
{
   if (_cache.enabled)
      return;
   int w = D3DImageCache::Current()->GetImageWidth(image_id);
   int h = D3DImageCache::Current()->GetImageHeight(image_id);
   _cache.enabled = true;
   _cache.image_id = image_id;
   _cache.data.Allocate(w * h);

   if (_cache.dirty.Length() != h)
   {
      _cache.dirty.Allocate(h);
      memset(_cache.dirty.Data(), 0xff, sizeof(POINT) * _cache.dirty.Length());
   }

   ZeroMemory(_cache.data.Data(), sizeof(DWORD) * _cache.data.Length());
   _cache.width = w;
   _cache.height = h;

   _cache.valid_rect.left = w;
   _cache.valid_rect.right = 0;
   _cache.valid_rect.top = h;
   _cache.valid_rect.bottom = 0;
}

void D3DObjectFont::Draw(D3DDevice *d3d)
{
   assert(_cache.image_id == _image_id);
}

void D3DObjectFont::EndCache(D3DDevice *d3d)
{
   if (!_cache.enabled)
      return;
   _cache.enabled = false;

   if (_cache.data.Length() == 0)
      return;

   D3DImageCache::CacheEntryInfo info;
   ZeroMemory(&info, sizeof(info));
   info.id = _cache.image_id;
   info.width = _cache.width;
   info.height = _cache.height;
   if (!D3DImageCache::Current()->UpdateImageDataWithDirtyInfo(info,
      _cache.data.Data(), _cache.dirty.Data()))
   //if (!D3DImageCache::Current()->UpdateImageDataDiscard(info, _cache.data.Data()));
   {
      return;
   }

   D3DShaderPack::Current()->SetVDecl(d3d, D3DShaderPack::VDECL_XYUV);
   D3DShaderPack::Current()->SetVS(d3d, D3DShaderPack::VS_COPY_UV);
   D3DShaderPack::Current()->SetPS(d3d, D3DShaderPack::PS_TEX_2);  // This image is in s1
   D3DImageCache::Current()->SelectImageToDevice(d3d, _cache.image_id);

   const FLOAT half_x = 0.5f / FLOAT(_cache.width);
   const FLOAT half_y = 0.5f / FLOAT(_cache.height);
   FLOAT left = FLOAT(_cache.valid_rect.left - 5) / FLOAT(_cache.width),
      top = FLOAT(_cache.valid_rect.top - 5) / FLOAT(_cache.height),
      right = FLOAT(_cache.valid_rect.right + 5) / FLOAT(_cache.width),
      bottom = FLOAT(_cache.valid_rect.bottom + 5) / FLOAT(_cache.height);

   const Vertex data[6] = {
      {left * 2 - 1, 2 * (1 - bottom) - 1,      left + half_x, bottom + half_y},
      {left * 2 - 1, 2 * (1 - top) - 1,         left + half_x, top + half_y},
      {right * 2 - 1, 2 * (1 - bottom) - 1,     right + half_x, bottom + half_y},
      {right * 2 - 1, 2 * (1 - bottom) - 1,     right + half_x, bottom + half_y},
      {left * 2 - 1, 2 * (1 - top) - 1,         left + half_x, top + half_y},
      {right * 2 - 1, 2 * (1 - top) - 1,        right + half_x, top + half_y}};

   d3d->GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, data, sizeof(Vertex));

   //D3DImageCache::Current()->UpdateImageDataWithDirtyInfo(info, NULL, _cache.dirty.Data());
}

D3DObjectFont::Glyph *D3DObjectFont::GetGlyph(void *source)
{
   if (_image_id < 0)
   {
      Log("Font is not initialized");
      return NULL;
   }
   for (int i = 0; i < _glyphs.Length(); i++)
   {
      if (_glyphs[i]->Compare(source))
         return _glyphs[i];
   }
   return NULL;
}

D3DObjectFont::Glyph *D3DObjectFont::AddGlyph(D3DDevice *d3d, void *source,
   BYTE *data8, int width, int height, int pitch)
{
   if (_image_id < 0)
   {
      Log("Font is not initialized");
      return NULL;
   }
   for (int i = 0; i < _glyphs.Length(); i++)
   {
      if (_glyphs[i]->Compare(source))
         return _glyphs[i];
   }
   Ref<Glyph> glyph = new Glyph(source);
   glyph->_data.Allocate(width * height);
   glyph->_width = width;
   glyph->_height = height;

   for (int i = 0; i < height; i++)
      CopyMemory(&glyph->_data[i * width], &data8[i * pitch], width);

   Log("Glyph added (%p) (%dx%d)", source, width, height);
   _glyphs.Add(glyph);
   return _glyphs.Last()->Addr();
}

void D3DObjectFont::PushForDraw(Glyph *glyph, int x, int y)
{
   BeginCache(_image_id);

   // Uff, I'm not sure about multiple windows...

#define LERP(a, b, t1, t2)  (BYTE)(FLOAT(a) * (t1) + FLOAT(b) * (t2))

   Color dc, sc;
   FLOAT a;
   sc.color = _color;

   const FLOAT color_alpha = sc.Alpha();

   DWORD *data = _cache.data.Data();
   BYTE *gdata = glyph->_data.Data();
   const int glyph_height = glyph->_height;
   const int cache_height = _cache.height;
   const int glyph_width = glyph->_width;
   const int cache_width = _cache.width;

   for (int i = 0, yi = y; i < glyph_height && yi < cache_height; i++, yi++)
   {
      if (yi < 0)
         continue;
      DWORD *dst = data + ((yi) * cache_width + x);
      BYTE *src = gdata + (i * glyph_width);
      POINT *dirty_yi = &_cache.dirty[yi];

      if (_cache.valid_rect.top > yi)
         _cache.valid_rect.top = yi;
      if (_cache.valid_rect.bottom < yi)
         _cache.valid_rect.bottom = yi;

      for (int j = 0, xj = x; j < glyph_width && xj < cache_width; j++, xj++, dst++, src++)
      {
         if (xj < 0)
            continue;
         BYTE glyph_pix = *src;
         if (glyph_pix == 0)
            continue;

         if (dirty_yi->x >= 0 && dirty_yi->y < 0)
            dirty_yi->x = 0, dirty_yi->y = cache_width - 1;
         else
         {
            if (dirty_yi->x < 0 || dirty_yi->x > xj)
               dirty_yi->x = xj;
            if (dirty_yi->y < 0 || dirty_yi->y < xj)
               dirty_yi->y = xj;
         }

         if (_cache.valid_rect.left > xj)
            _cache.valid_rect.left = xj;
         if (_cache.valid_rect.right < xj)
            _cache.valid_rect.right = xj;

         if (glyph_pix == 0xff && sc.a == 0xff)
         {
            *dst = sc.color;
            continue;
         }
         a = FLOAT(glyph_pix) * color_alpha / 255.f;
         if (*dst == 0)
         {
            *dst = (BYTE(255.f * a) << 24) | (0x00ffffff & sc.color);
            continue;
         }

         dc.color = *dst;

         dc.r = LERP(dc.r, sc.r, 1 - a, a);
         dc.g = LERP(dc.g, sc.g, 1 - a, a);
         dc.b = LERP(dc.b, sc.b, 1 - a, a);
         dc.a = max(dc.a, BYTE(255.f * a));
         *dst = dc.color;
      }
   }
}
