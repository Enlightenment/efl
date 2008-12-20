#ifndef __EVAS_DIRECT3D_IMAGE_CACHE_H__
#define __EVAS_DIRECT3D_IMAGE_CACHE_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

class D3DDevice;

class D3DImageCache : virtual public Referenc
{
public:
   struct CacheEntryInfo
   {
      int id;
      int width, height;
      FLOAT u, v;
      FLOAT du, dv;
   };

public:
   ~D3DImageCache();

   static D3DImageCache *Current();
   static void SetCurrent(D3DImageCache *obj);

   inline void SetMaxSize(int w, int h);
   inline void SetMargin(int margin);

   bool InsertImage(D3DDevice *d3d, DWORD *data, int w, int h, CacheEntryInfo &info);
   bool InsertImage(D3DDevice *d3d, int id, DWORD *data, int w, int h, CacheEntryInfo &info);
   bool CreateImage(D3DDevice *d3d, int w, int h, bool locked, CacheEntryInfo &info);
   bool ResizeImage(D3DDevice *d3d, int nw, int nh, int id);
   bool SelectImageToDevice(D3DDevice *d3d, int id);
   void RemoveImageUser(int id);
   void AddImageUser(int id);
   bool UpdateImageData(CacheEntryInfo &info, DWORD *data);
   bool UpdateImageDataWithDirtyInfo(CacheEntryInfo &info, DWORD *data, POINT *dirty);
   bool UpdateImageDataDiscard(CacheEntryInfo &info, DWORD *data);
   bool GetImageData(CacheEntryInfo &info, TArray<DWORD> &data);
   void Uninitialize();

   inline int GetImageWidth(int image_id);
   inline int GetImageHeight(int image_id);

   inline void SetImageStage(int image_id, int stage);

private:
   struct CacheEntry
   {
      LPDIRECT3DTEXTURE9 texture;
      int width;
      int height;
      int cur_x;
      int cur_y;
      int cur_h;
      int users;
      bool locked;
      int stage;
   };

private:
   D3DImageCache();

   bool RequestInsert(CacheEntry &entry, int w, int h);
   bool CreateEntry(D3DDevice *d3d, CacheEntry &entry, int w, int h, bool exact_size = false);
   bool InsertData(CacheEntry &entry, DWORD *data, int w, int h);
   bool RetrieveData(CacheEntry &entry, DWORD *data, int w, int h);
   void UpdateInsert(CacheEntry &entry, int w, int h);

private:
   TArray<CacheEntry> _cache;
   int _max_width;
   int _max_height;

   int _margin;

   static Ref<D3DImageCache> _this;
};

void D3DImageCache::SetMaxSize(int w, int h)
{
   _max_width = w;
   _max_height = h;
}

void D3DImageCache::SetMargin(int margin)
{
   _margin = margin;
}

int D3DImageCache::GetImageWidth(int image_id)
{
   return _cache[image_id].width;
}

int D3DImageCache::GetImageHeight(int image_id)
{
   return _cache[image_id].height;
}

void D3DImageCache::SetImageStage(int image_id, int stage)
{
   _cache[image_id].stage = stage;
}

#endif  // __EVAS_DIRECT3D_IMAGE_CACHE_H__
