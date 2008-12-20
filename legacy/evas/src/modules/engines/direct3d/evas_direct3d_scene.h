#ifndef __EVAS_DIRECT3D_SCENE_H__
#define __EVAS_DIRECT3D_SCENE_H__

#include "evas_engine.h"

#include <assert.h>
#include <typeinfo>

#include "ref.h"
#include "array.h"

#include "evas_direct3d_object.h"

class D3DDevice;

class D3DScene : virtual public Referenc
{
public:
   D3DScene();

   void FreeObjects();

   inline void AddObject(D3DObject *object);
   inline int GetObjectCount();
   void DeleteObject(D3DObject *object);

   void DrawAll(D3DDevice *d3d);

   template <class T> T *GetFreeObject()
   {
      for (int i = 0; i < _objects.Length(); i++)
      {
         if (typeid(T) == typeid(*_objects[i].Addr()) && _objects[i]->IsFree())
            return (T *)_objects[i].Addr();
      }
      return NULL;
   }

   template <class T> void GetObjectsOfType(TArray<T *> &res)
   {
      for (int i = 0; i < _objects.Length(); i++)
      {
         if (typeid(T) == typeid(*_objects[i].Addr()))
            res.Add((T *)_objects[i].Addr());
      }
   }

private:
   TArray<Ref<D3DObject> > _objects;
};

void D3DScene::AddObject(D3DObject *object)
{
   assert(object != NULL);
   _objects.Add(object);
}

int D3DScene::GetObjectCount()
{
   return _objects.Length();
}

#endif  // __EVAS_DIRECT3D_SCENE_H__
