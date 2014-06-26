#include "evas_common_private.h"
#include "evas_private.h"

EAPI Evas_Device *
evas_device_add(Evas *eo_e)
{
   Evas_Device *dev;
   
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   dev = calloc(1, sizeof(Evas_Device));
   if (!dev) return NULL;
   dev->magic = MAGIC_DEV;
   dev->evas = eo_e;
   dev->ref = 1;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   e->devices = eina_list_append(e->devices, dev);
   evas_event_callback_call(eo_e, EVAS_CALLBACK_DEVICE_CHANGED, dev);
   return dev;
}

EAPI void
evas_device_del(Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   if (dev->ref == 1)
     {
        Evas_Device *dev2;
        
        EINA_LIST_FREE(dev->children, dev2)
          {
             dev2->parent = NULL;
             evas_device_del(dev2);
          }
        if (dev->src)
          {
             _evas_device_unref(dev->src);
             dev->src = NULL;
          }
        dev->parent = NULL;
     }
   _evas_device_unref(dev);
}

EAPI void
evas_device_push(Evas *eo_e, Evas_Device *dev)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e->cur_device)
     {
        e->cur_device = eina_array_new(4);
        if (!e->cur_device) return;
     }
   dev->ref++;
   eina_array_push(e->cur_device, dev);
}

EAPI void
evas_device_pop(Evas *eo_e)
{
   Evas_Device *dev;
   
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   dev = eina_array_pop(e->cur_device);
   if (dev) _evas_device_unref(dev);
}

EAPI const Eina_List *
evas_device_list(Evas *eo_e, const Evas_Device *dev)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (dev)
     {
        MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
        return NULL;
        MAGIC_CHECK_END();
     }
   if (dev) return dev->children;
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   return e->devices;
}

EAPI void
evas_device_name_set(Evas_Device *dev, const char *name)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   eina_stringshare_replace(&(dev->name), name);
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_name_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return NULL;
   MAGIC_CHECK_END();
   return dev->name;
}

EAPI void
evas_device_description_set(Evas_Device *dev, const char *desc)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   eina_stringshare_replace(&(dev->desc), desc);
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_description_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return NULL;
   MAGIC_CHECK_END();
   return dev->desc;
}

EAPI void
evas_device_parent_set(Evas_Device *dev, Evas_Device *parent)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   Evas_Public_Data *e = eo_data_scope_get(dev->evas, EVAS_CANVAS_CLASS);
   if (parent)
     {
        MAGIC_CHECK(parent, Evas_Device, MAGIC_DEV);
        return;
        MAGIC_CHECK_END();
     }
   if (dev->parent == parent) return;
   if (dev->parent)
     dev->parent->children = eina_list_remove(dev->parent->children, dev);
   else if (parent)
     e->devices = eina_list_remove(e->devices, dev);
   dev->parent = parent;
   if (parent)
     parent->children = eina_list_append(parent->children, dev);
   else
     e->devices = eina_list_append(e->devices, dev);
   
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_parent_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return NULL;
   MAGIC_CHECK_END();
   return dev->parent;
}

EAPI void
evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   dev->clas = clas;
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Class
evas_device_class_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return EVAS_DEVICE_CLASS_NONE;
   MAGIC_CHECK_END();
   return dev->clas;
}

EAPI void
evas_device_subclass_set(Evas_Device *dev, Evas_Device_Subclass clas)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   dev->subclas = clas;
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Subclass
evas_device_subclass_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return EVAS_DEVICE_SUBCLASS_NONE;
   MAGIC_CHECK_END();
   return dev->subclas;
}

EAPI void
evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return;
   MAGIC_CHECK_END();
   if (src)
     {
        MAGIC_CHECK(src, Evas_Device, MAGIC_DEV);
        return;
        MAGIC_CHECK_END();
     }
   if (dev->src == src) return;
   if (dev->src) _evas_device_unref(dev->src);
   dev->src = src;
   if (dev->src) dev->src->ref++;
   evas_event_callback_call(dev->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_emulation_source_get(const Evas_Device *dev)
{
   MAGIC_CHECK(dev, Evas_Device, MAGIC_DEV);
   return NULL;
   MAGIC_CHECK_END();
   return dev->src;
}

void
_evas_device_cleanup(Evas *eo_e)
{
   Evas_Device *dev;
   
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (e->cur_device)
     {
        while ((dev = eina_array_pop(e->cur_device)))
          _evas_device_unref(dev);
        eina_array_free(e->cur_device);
        e->cur_device = NULL;
     }
   EINA_LIST_FREE(e->devices, dev)
     {
        evas_device_del(dev);
     }
}

Evas_Device *
_evas_device_top_get(const Evas *eo_e)
{
   int num;
   
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e->cur_device) return NULL;
   num = eina_array_count(e->cur_device);
   if (num < 1) return NULL;
   return eina_array_data_get(e->cur_device, num - 1);
}

void
_evas_device_ref(Evas_Device *dev)
{
   dev->ref++;
}

void
_evas_device_unref(Evas_Device *dev)
{
   dev->ref--;
   if (dev->ref > 0) return;
   if (dev->name) eina_stringshare_del(dev->name);
   if (dev->desc) eina_stringshare_del(dev->desc);
   dev->magic = 0;
   free(dev);
}

