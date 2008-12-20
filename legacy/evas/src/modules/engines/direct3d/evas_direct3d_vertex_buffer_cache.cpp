
//#define ENABLE_LOG_PRINTF

#include "evas_direct3d_vertex_buffer_cache.h"
#include "evas_direct3d_device.h"

#include <assert.h>


Ref<D3DVertexBufferCache> D3DVertexBufferCache::_this;

D3DVertexBufferCache::D3DVertexBufferCache()
{
   size_border_low = 0.6;  // We can reuse buffer on 60%
   size_border_high = 0.2;  // We can reallocate the buffer on 20%
}

D3DVertexBufferCache::~D3DVertexBufferCache()
{
   Uninitialize();
}

D3DVertexBufferCache *D3DVertexBufferCache::Current()
{
   if (_this.IsNull())
      _this = new D3DVertexBufferCache();
   return _this;
}

void D3DVertexBufferCache::SetCurrent(D3DVertexBufferCache *obj)
{
   _this = obj;
}

void D3DVertexBufferCache::Uninitialize()
{
   for (int i = 0; i < _cache.Length(); i++)
   {
      assert(_cache[i].vb != NULL);
      _cache[i].vb->Release();
   }
   _cache.Resize();
}

bool D3DVertexBufferCache::InitBuffer(D3DDevice *d3d, BYTE *data, int size, CacheEntryInfo &info)
{
   assert(d3d != NULL);
   assert(data != NULL);
   assert(size > 0);

   int best = FindBestEntry(size);
   CacheEntry *ce = NULL;

   // Reallocate
   if (best >= 0 && _cache[best].size < size)
   {
      DeleteEntry(best);
      best = -1;
   }

   // New
   if (best < 0)
   {
      CacheEntry new_entry;
      if (!CreateEntry(d3d, new_entry, size))
      {
         Log("Failed to create new vbcache entry");
         return false;
      }
      _cache.Add(new_entry);
      info.id = _cache.Length() - 1;
      ce = _cache.Last();
   }
   else
   {
      info.id = best;
      ce = &_cache[best];
   }

   assert(ce != NULL);
   if (!InsertData(*ce, data, size))
   {
      Log("Failed to insert vbcache data");
      return false;
   }
   return true;
}

bool D3DVertexBufferCache::SelectBufferToDevice(D3DDevice *device, int id, int vertex_size)
{
   if (id < 0 || id >= _cache.Length())
      return false;
   return SUCCEEDED(device->GetDevice()->SetStreamSource(0, _cache[id].vb, 0, vertex_size));
}

int D3DVertexBufferCache::FindBestEntry(int size)
{
   // Search for buffer that fits in borders
   for (int i = 0; i < _cache.Length(); i++)
   {
      const int vs = _cache[i].size;
      if (size >= (vs - FLOAT(vs) * size_border_low) && size <= vs)
         return i;
   }
   bool less_than_all = true;
   for (int i = 0; i < _cache.Length(); i++)
   {
      const int vs = _cache[i].size;
      if (size >= (vs - FLOAT(vs) * size_border_low))
         less_than_all = false;
   }
   // Requested size is too small to reuse in any buffer
   if (less_than_all)
      return -1;
   // Search for buffer that can be reallocated
   for (int i = 0; i < _cache.Length(); i++)
   {
      const int vs = _cache[i].size;
      if (size <= (vs + FLOAT(vs) * size_border_high))
         return i;
   }
   // No buffer can be reused or reallocated, create a new one
   return -1;
}

bool D3DVertexBufferCache::CreateEntry(D3DDevice *d3d, CacheEntry &entry, int size)
{
   assert(d3d != NULL);
   if (FAILED(d3d->GetDevice()->CreateVertexBuffer(size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
      0, D3DPOOL_DEFAULT, &entry.vb, NULL)))
   {
      return false;
   }
   entry.size = size;
   return true;
}

void D3DVertexBufferCache::DeleteEntry(int id)
{
   if (id < 0 || id >= _cache.Length())
      return;
   assert(_cache[id].vb != NULL);
   _cache[id].vb->Release();
   _cache.Replace(id);
}

bool D3DVertexBufferCache::InsertData(CacheEntry &entry, BYTE *data, int size)
{
   BYTE *ptr = NULL;
   if (FAILED(entry.vb->Lock(0, size, (void **)&ptr, D3DLOCK_DISCARD)))
      return false;
   CopyMemory(ptr, data, size);
   return SUCCEEDED(entry.vb->Unlock());
}
