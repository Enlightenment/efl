#ifndef __EVAS_DIRECT3D_CONTEXT_H__
#define __EVAS_DIRECT3D_CONTEXT_H__

#include "evas_engine.h"

#include "ref.h"
#include "evas_direct3d_object.h"

class D3DContext : virtual public Referenc
{
public:
   D3DContext();

public:
   DWORD color;
   DWORD color_mul;

   Ref<D3DObject> font;

};

#endif  // __EVAS_DIRECT3D_CONTEXT_H__
