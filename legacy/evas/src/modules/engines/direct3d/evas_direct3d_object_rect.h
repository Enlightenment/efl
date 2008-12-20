#ifndef __EVAS_DIRECT3D_OBJECT_RECT_H__
#define __EVAS_DIRECT3D_OBJECT_RECT_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

#include "evas_direct3d_object.h"

class D3DObjectRect : public D3DObject
{
public:
   D3DObjectRect();

   static void BeginCache();
   virtual void Draw(D3DDevice *d3d);
   static void EndCache(D3DDevice *d3d);

   void Setup(FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD color);

private:
   FLOAT _x, _y, _w, _h;
   DWORD _color;

private:
   struct Vertex
   {
      FLOAT x, y;
      DWORD color;
   };

   static TArray<Vertex> _cache;
   static bool _cache_enabled;
};

#endif  // __EVAS_DIRECT3D_OBJECT_RECT_H__
