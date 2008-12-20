#include "evas_direct3d_image_cache.h"

#include "evas_direct3d_device.h"

#include <assert.h>

Ref<D3DImageCache> D3DImageCache::_this;

D3DImageCache::D3DImageCache()
{
   _max_width = 512;
   _max_height = 512;
   _margin = 0;
}

D3DImageCache::~D3DImageCache()
{
   Uninitialize();
}

D3DImageCache *D3DImageCache::Current()
{
   if (_this.IsNull())
      _this = new D3DImageCache();
   return _this;
}

void D3DImageCache::SetCurrent(D3DImageCache *obj)
{
   _this = obj;
}

void D3DImageCache::Uninitialize()
{
   for (int i = 0; i < _cache.Length(); i++)
   {
      // In normal case they all will be NULL
      if (_cache[i].texture != NULL)
         _cache[i].texture->Release();
   }
   _cache.Resize();
}

bool D3DImageCache::SelectImageToDevice(D3DDevice *d3d, int id)
{
   if (id < 0 || id >= _cache.Length())
      return false;
   assert(_cache[id].texture != NULL);
   return SUCCEEDED(d3d->SetTexture(_cache[id].stage, _cache[id].texture));
}

void D3DImageCache::RemoveImageUser(int id)
{
   if (id < 0 || id >= _cache.Length())
      return;
   assert(_cache[id].texture != NULL);
   _cache[id].users--;
   if (_cache[id].users == 0)
   {
      _cache[id].texture->Release();
      ZeroMemory(&_cache[id], sizeof(_cache[id]));
   }
}

void D3DImageCache::AddImageUser(int id)
{
   if (id < 0 || id >= _cache.Length())
      return;
   assert(_cache[id].texture != NULL);
   _cache[id].users++;
}

bool D3DImageCache::InsertImage(D3DDevice *d3d, DWORD *data, int w, int h, CacheEntryInfo &info)
{
   CacheEntry *ce = NULL;
   int id = -1;
   for (int i = 0; i < _cache.Length(); i++)
   {
      if (!_cache[i].locked && RequestInsert(_cache[i], w, h))
      {
         ce = &_cache[i];
         id = i;
         break;
      }
   }
   if (ce == NULL)
   {
      CacheEntry new_entry;
      if (!CreateEntry(d3d, new_entry, w, h))
         return false;
      for (id = 0; id < _cache.Length(); id++)
      {
         if (_cache[id].texture == NULL)
            break;
      }

      if (id < _cache.Length())
      {
         _cache[id] = new_entry;
         ce = &_cache[id];
      }
      else
      {
         _cache.Add(new_entry);
         ce = _cache.Last();
         id = _cache.Length() - 1;
      }
   }

   assert(ce != NULL && ce->texture != NULL);

   if (!InsertData(*ce, data, w, h))
      return false;

   info.id = id;
   info.u = FLOAT(ce->cur_x) / FLOAT(ce->width);
   info.v = FLOAT(ce->cur_y) / FLOAT(ce->height);
   info.du = FLOAT(w) / FLOAT(ce->width);
   info.dv = FLOAT(h) / FLOAT(ce->height);
   info.width = w;
   info.height = h;

   UpdateInsert(*ce, w, h);
   return true;
}

bool D3DImageCache::InsertImage(D3DDevice *d3d, int id, DWORD *data, int w, int h, CacheEntryInfo &info)
{
   if (id < 0 || id >= _cache.Length())
      return false;
   assert(_cache[id].texture != NULL);
   CacheEntry *ce = &_cache[id];
   if (!RequestInsert(*ce, w, h))
      return false;
   if (!InsertData(*ce, data, w, h))
      return false;

   info.id = id;
   info.u = FLOAT(ce->cur_x) / FLOAT(ce->width);
   info.v = FLOAT(ce->cur_y) / FLOAT(ce->height);
   info.du = FLOAT(w) / FLOAT(ce->width);
   info.dv = FLOAT(h) / FLOAT(ce->height);
   info.width = w;
   info.height = h;

   UpdateInsert(*ce, w, h);
   return true;
}

bool D3DImageCache::CreateImage(D3DDevice *d3d, int w, int h, bool locked, CacheEntryInfo &info)
{
   int id;
   CacheEntry new_entry;
   CacheEntry *ce = NULL;

   if (!CreateEntry(d3d, new_entry, w, h, true))
      return false;
   for (id = 0; id < _cache.Length(); id++)
   {
      if (_cache[id].texture == NULL)
         break;
   }

   if (id < _cache.Length())
   {
      _cache[id] = new_entry;
      ce = &_cache[id];
   }
   else
   {
      _cache.Add(new_entry);
      ce = _cache.Last();
      id = _cache.Length() - 1;
   }

   assert(ce != NULL && ce->texture != NULL);

   // Fill with zero
   if (!InsertData(*ce, NULL, w, h))
      return false;

   info.id = id;
   info.u = 0;
   info.v = 0;
   info.du = 1;
   info.dv = 1;
   info.width = w;
   info.height = h;

   UpdateInsert(*ce, 0, 0);
   ce->locked = locked;
   return true;
}

bool D3DImageCache::ResizeImage(D3DDevice *d3d, int nw, int nh, int id)
{
   if (id < 0 || id >= _cache.Length())
      return false;
   assert(_cache[id].texture != NULL);
   CacheEntry *ce = &_cache[id];

   if (ce->width == nw && ce->height == nh)
      return true;

   LPDIRECT3DTEXTURE9 tex = NULL;

   HRESULT hr;
   if (FAILED(hr = d3d->GetDevice()->CreateTexture(nw, nh, 0, 0, D3DFMT_A8R8G8B8,
      D3DPOOL_MANAGED, &tex, NULL)))
   {
      Log("Failed to create texture: %X", hr);
      return false;
   }
   assert(tex != NULL);

   ce->texture->Release();
   ce->texture = tex;
   ce->width = nw;
   ce->height = nh;
   return true;
}

bool D3DImageCache::RequestInsert(CacheEntry &entry, int w, int h)
{
   // If we already have large image entry
   if (entry.width > _max_width || entry.height > _max_height)
      return false;
   // If requested size does not fit into this entry at all
   if (entry.height - entry.cur_h < h + _margin * 2 || entry.width < w + _margin * 2)
      return false;

   // If requested size does not fit into the current line of the entry
   if (entry.width - entry.cur_x < w + _margin * 2)
   {
      entry.cur_y = entry.cur_h + _margin;
      entry.cur_x = _margin;
      return true;
   }
   entry.cur_x += _margin;

   return true;
}

bool D3DImageCache::CreateEntry(D3DDevice *d3d, CacheEntry &entry, int w, int h, bool exact_size)
{
   int width = exact_size ? w : max(_max_width, w);
   int height = exact_size ? h : max(_max_height, h);
   HRESULT hr;
   if (FAILED(hr = d3d->GetDevice()->CreateTexture(width, height, 0, 0, D3DFMT_A8R8G8B8,
      D3DPOOL_MANAGED, &entry.texture, NULL)))
   {
      Log("Failed to create texture: %X", hr);
      return false;
   }

   entry.cur_x = entry.cur_y = entry.cur_h = 0;
   entry.width = width;
   entry.height = height;
   entry.users = 0;
   entry.locked = false;
   entry.stage = 0;
   return true;
}

bool D3DImageCache::InsertData(CacheEntry &entry, DWORD *data, int w, int h)
{
   if (entry.texture == NULL)
      return false;

   RECT rc = {entry.cur_x, entry.cur_y, entry.cur_x + w, entry.cur_y + h};
   D3DLOCKED_RECT lr;
   if (FAILED(entry.texture->LockRect(0, &lr, &rc, 0)))
   {
      Log("Failed to lock texture");
      return false;
   }

   if (data != NULL)
   {
      for (int i = 0; i < h; i++)
         CopyMemory(((BYTE *)lr.pBits) + i * lr.Pitch, data + i * w, sizeof(DWORD) * w);
   }
   else
   {
      for (int i = 0; i < h; i++)
         ZeroMemory(((BYTE *)lr.pBits) + i * lr.Pitch, sizeof(DWORD) * w);
   }

   if (FAILED(entry.texture->UnlockRect(0)))
   {
      Log("Failed to unlock texture");
      return false;
   }
   return true;
}


bool D3DImageCache::RetrieveData(CacheEntry &entry, DWORD *data, int w, int h)
{
   if (entry.texture == NULL || data == NULL)
      return false;

   RECT rc = {entry.cur_x, entry.cur_y, entry.cur_x + w, entry.cur_y + h};
   D3DLOCKED_RECT lr;
   if (FAILED(entry.texture->LockRect(0, &lr, &rc, D3DLOCK_READONLY)))
   {
      Log("Failed to lock texture");
      return false;
   }

   for (int i = 0; i < h; i++)
      CopyMemory(data + i * w, ((BYTE *)lr.pBits) + i * lr.Pitch, sizeof(DWORD) * w);

   if (FAILED(entry.texture->UnlockRect(0)))
   {
      Log("Failed to unlock texture");
      return false;
   }
   return true;
}

void D3DImageCache::UpdateInsert(CacheEntry &entry, int w, int h)
{
   entry.cur_h = max(entry.cur_h, entry.cur_y + h + _margin);
   entry.cur_x += w + _margin;
   entry.users++;
}

bool D3DImageCache::UpdateImageData(CacheEntryInfo &info, DWORD *data)
{
   assert(data != NULL);
   if (info.id < 0 || info.id >= _cache.Length())
      return false;
   CacheEntry ce_copy = _cache[info.id];
   ce_copy.cur_x = int(info.u * FLOAT(ce_copy.width));
   ce_copy.cur_y = int(info.v * FLOAT(ce_copy.height));
   return InsertData(ce_copy, data, info.width, info.height);
}

bool D3DImageCache::UpdateImageDataWithDirtyInfo(CacheEntryInfo &info, DWORD *data, POINT *dirty)
{
   if (info.id < 0 || info.id >= _cache.Length())
      return false;
   CacheEntry &entry = _cache[info.id];
   if (entry.texture == NULL)
      return false;

   RECT rc = {0, 0, entry.width, entry.height};
   D3DLOCKED_RECT lr;
   if (FAILED(entry.texture->LockRect(0, &lr, &rc, 0)))
   {
      Log("Failed to lock texture");
      return false;
   }

   if (data != NULL)
   {
      for (int i = 0; i < rc.bottom; i++)
      {
         if (dirty[i].x < 0 && dirty[i].y < 0)
            continue;
         if (dirty[i].x >= 0 && dirty[i].y >= 0)
         {
            CopyMemory(((BYTE *)lr.pBits) + i * lr.Pitch + dirty[i].x * 4,
               data + i * rc.right + dirty[i].x, sizeof(DWORD) * (dirty[i].y - dirty[i].x + 1));
            dirty[i].y = -dirty[i].y;
         }
         else if (dirty[i].x >= 0 && dirty[i].y < 0)
         {
            ZeroMemory(((BYTE *)lr.pBits) + i * lr.Pitch + dirty[i].x * 4,
               sizeof(DWORD) * (-dirty[i].y - dirty[i].x + 1));
            dirty[i].x = -dirty[i].x;
         }
      }
   }
   else
   {
      for (int i = 0; i < rc.bottom; i++)
      {
         if (dirty[i].x < 0 || dirty[i].y < 0)
            continue;
         ZeroMemory(((BYTE *)lr.pBits) + i * lr.Pitch + dirty[i].x * 4,
            sizeof(DWORD) * (dirty[i].y - dirty[i].x + 1));
      }
   }

   if (FAILED(entry.texture->UnlockRect(0)))
   {
      Log("Failed to unlock texture");
      return false;
   }
   return true;
}

bool D3DImageCache::UpdateImageDataDiscard(CacheEntryInfo &info, DWORD *data)
{
   assert(data != NULL);
   if (info.id < 0 || info.id >= _cache.Length())
      return false;
   CacheEntry &entry = _cache[info.id];
   if (entry.texture == NULL)
      return false;

   RECT rc = {0, 0, entry.width, entry.height};
   D3DLOCKED_RECT lr;
   if (FAILED(entry.texture->LockRect(0, &lr, &rc, 0)))
   {
      Log("Failed to lock texture");
      return false;
   }

   for (int i = 0; i < rc.bottom; i++)
   {
      CopyMemory(((BYTE *)lr.pBits) + i * lr.Pitch,
         data + i * rc.right, sizeof(DWORD) * rc.right);
   }

   if (FAILED(entry.texture->UnlockRect(0)))
   {
      Log("Failed to unlock texture");
      return false;
   }
   return true;
}

bool D3DImageCache::GetImageData(CacheEntryInfo &info, TArray<DWORD> &data)
{
   if (info.id < 0 || info.id >= _cache.Length())
      return false;
   CacheEntry ce_copy = _cache[info.id];
   ce_copy.cur_x = int(info.u * FLOAT(ce_copy.width));
   ce_copy.cur_y = int(info.v * FLOAT(ce_copy.height));
   data.Allocate(info.width * info.height);

   return RetrieveData(ce_copy, data.Data(), info.width, info.height);
}
