#ifndef __EVAS_DIRECT3D_OBJECT_LINE_H__
#define __EVAS_DIRECT3D_OBJECT_LINE_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

#include "evas_direct3d_object.h"

class D3DObjectLine : public D3DObject
{
public:
   D3DObjectLine();

   static void BeginCache();
   virtual void Draw(D3DDevice *d3d);
   static void EndCache(D3DDevice *d3d);

   void Setup(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, DWORD color);

private:
   FLOAT _x1, _y1, _x2, _y2;
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

#endif  // __EVAS_DIRECT3D_OBJECT_LINE_H__
