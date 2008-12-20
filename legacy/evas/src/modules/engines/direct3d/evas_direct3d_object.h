#ifndef __EVAS_DIRECT3D_OBJECT_H__
#define __EVAS_DIRECT3D_OBJECT_H__

#include "evas_engine.h"

#include "ref.h"

class D3DDevice;

class D3DObject : virtual public Referenc
{
public:
   D3DObject();
   virtual ~D3DObject();

   virtual void Draw(D3DDevice *d3d) = 0;

   inline bool IsFree();
   inline void SetFree(bool state);

private:
   bool _free;
};

bool D3DObject::IsFree()
{
   return _free;
}

void D3DObject::SetFree(bool state)
{
   _free = state;
}

#endif  // __EVAS_DIRECT3D_OBJECT_H__
