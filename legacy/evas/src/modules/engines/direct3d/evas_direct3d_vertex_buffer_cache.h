#ifndef __EVAS_DIRECT3D_VERTEX_BUFFER_CACHE_H__
#define __EVAS_DIRECT3D_VERTEX_BUFFER_CACHE_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

class D3DDevice;

class D3DVertexBufferCache : virtual public Referenc
{
public:
   struct CacheEntryInfo
   {
      int id;
   };

public:
   ~D3DVertexBufferCache();

   static D3DVertexBufferCache *Current();
   static void SetCurrent(D3DVertexBufferCache *obj);

   inline void SetSizeBorders(FLOAT low, FLOAT high);

   bool InitBuffer(D3DDevice *d3d, BYTE *data, int size, CacheEntryInfo &info);
   bool SelectBufferToDevice(D3DDevice *device, int id, int vertex_size);
   void Uninitialize();

private:
   struct CacheEntry
   {
      LPDIRECT3DVERTEXBUFFER9 vb;
      int size;
   };

private:
   D3DVertexBufferCache();

   int FindBestEntry(int size);
   bool CreateEntry(D3DDevice *d3d, CacheEntry &entry, int size);
   void DeleteEntry(int id);
   bool InsertData(CacheEntry &entry, BYTE *data, int size);

private:
   TArray<CacheEntry> _cache;
   FLOAT size_border_low;
   FLOAT size_border_high;

   static Ref<D3DVertexBufferCache> _this;
};

void D3DVertexBufferCache::SetSizeBorders(FLOAT low, FLOAT high)
{
   size_border_low = low;
   size_border_high = high;
}

#endif  // __EVAS_DIRECT3D_VERTEX_BUFFER_CACHE_H__
