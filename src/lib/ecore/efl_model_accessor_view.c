#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Eo.h"

#include "efl_model_accessor_view_private.h"

typedef struct _Efl_Model_Accessor
{
   Eina_Accessor                          vtable;
   Eina_Accessor                          *real_accessor;
   void                                   *pdata;
   Efl_Model_Accessor_View_Constructor_Cb cb;
   Eina_Array                             *children;
} Efl_Model_Accessor;

static void
_efl_model_accessor_setup(Efl_Model_Accessor *acc,
                          Eina_Accessor* real_accessor,
                          Efl_Model_Accessor_View_Constructor_Cb ctor,
                          void* pdata);

static Eina_Bool
_efl_model_acessor_get_at(Efl_Model_Accessor *acc, unsigned int idx, void **data)
{
  void* eo;
  Eo *p;
  Eo *child;

  if(eina_accessor_data_get(acc->real_accessor, idx, &eo))
    {
      p = eo;
      child = acc->cb(acc->pdata, p);
      if(!acc->children)
        {
           acc->children = eina_array_new(32);
        }
      eina_array_push(acc->children, child);
      *data = child;
      return !!*data;
    }
  else
    return EINA_FALSE;
}

static void *
_efl_model_acessor_get_container(Efl_Model_Accessor *acc)
{
   return eina_accessor_container_get(acc->real_accessor);
}

static void
_efl_model_acessor_free(Efl_Model_Accessor *acc)
{
   if (acc->real_accessor)
     {
        eina_accessor_free(acc->real_accessor);
        acc->real_accessor = NULL;
     }

   if(acc->children)
     {
        unsigned i;
        Eina_Array_Iterator iterator;
        Eo* item;
       
        EINA_ARRAY_ITER_NEXT(acc->children, i, item, iterator)
          {
            efl_unref(item);
          }
        eina_array_free(acc->children);
        acc->children = NULL;
     }

   free(acc);
}

static Eina_Bool
_efl_model_acessor_lock(Efl_Model_Accessor *acc)
{
   return eina_accessor_lock(acc->real_accessor);
}

static Eina_Bool
_efl_model_acessor_unlock(Efl_Model_Accessor *acc)
{
   return eina_accessor_unlock(acc->real_accessor);
}

static Efl_Model_Accessor *
_efl_model_acessor_clone(Efl_Model_Accessor *acc EINA_UNUSED)
{
   Efl_Model_Accessor* accessor = calloc(1, sizeof(Efl_Model_Accessor));
   _efl_model_accessor_setup(accessor, eina_accessor_clone(acc->real_accessor),
                             acc->cb, acc->pdata);
   return accessor;
}

static void
_efl_model_accessor_setup(Efl_Model_Accessor *acc,
                                Eina_Accessor* real_accessor,
                                Efl_Model_Accessor_View_Constructor_Cb ctor,
                                void* pdata)
{
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(_efl_model_acessor_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_model_acessor_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(_efl_model_acessor_free);

   acc->vtable.lock = FUNC_ACCESSOR_LOCK(_efl_model_acessor_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(_efl_model_acessor_unlock);

   acc->vtable.clone = FUNC_ACCESSOR_CLONE(_efl_model_acessor_clone);

   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);

   acc->real_accessor = real_accessor;
   acc->cb = ctor;
   acc->pdata = pdata;
}

Eina_Accessor* efl_model_accessor_view_new(Eina_Accessor* accessor,
                                           Efl_Model_Accessor_View_Constructor_Cb ctor,
                                           void* data)
{
   Efl_Model_Accessor* acc = calloc(1, sizeof(Efl_Model_Accessor));
   _efl_model_accessor_setup(acc, accessor, ctor, data);
   return &acc->vtable;
}


