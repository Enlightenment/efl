#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

/* WARNING: This API is not used across EFL, hard to test! */

#ifdef DEBUG_UNTESTED_
// booh
#define SAFETY_CHECK(obj, klass, ...) \
   do { MAGIC_CHECK(dev, Evas_Device, 1); \
        return __VA_ARGS__; \
        MAGIC_CHECK_END(); \
   } while (0)

#else
#define SAFETY_CHECK(obj, klass, ...) \
   do { if (!obj) return __VA_ARGS__; } while (0)
#endif

/* FIXME: Ideally no work besides calling the Efl_Input_Device API
 * should be done here. But unfortunately, some knowledge of Evas is required
 * here (callbacks and canvas private data).
 */

static Eina_Bool
_del_cb(void *data, const Eo_Event *ev)
{
   Evas_Public_Data *e = data;

   // can not be done in std destructor
   e->devices = eina_list_remove(e->devices, ev->object);

   return EO_CALLBACK_CONTINUE;
}

EAPI Evas_Device *
evas_device_add(Evas *eo_e)
{
   Efl_Input_Device_Data *d;
   Evas_Public_Data *e;
   Evas_Device *dev;

   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS, NULL);

   dev = eo_add(EFL_INPUT_DEVICE_CLASS, eo_e);

   d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   d->evas = eo_e;

   e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   e->devices = eina_list_append(e->devices, dev);
   eo_event_callback_add(dev, EO_EVENT_DEL, _del_cb, e);

   evas_event_callback_call(eo_e, EVAS_CALLBACK_DEVICE_CHANGED, dev);

   return dev;
}

EAPI void
evas_device_del(Evas_Device *dev)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   eo_unref(dev);
}

EAPI void
evas_device_push(Evas *eo_e, Evas_Device *dev)
{
   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS);
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (!e->cur_device)
     {
        e->cur_device = eina_array_new(4);
        if (!e->cur_device) return;
     }
   eo_ref(dev);
   eina_array_push(e->cur_device, dev);
}

EAPI void
evas_device_pop(Evas *eo_e)
{
   Evas_Device *dev;

   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS);

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   dev = eina_array_pop(e->cur_device);
   if (dev) eo_unref(dev);
}

EAPI const Eina_List *
evas_device_list(Evas *eo_e, const Evas_Device *dev)
{
   SAFETY_CHECK(eo_e, EVAS_CANVAS_CLASS, NULL);

   if (dev)
     {
        SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS, NULL);

        Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
        return d->children;
     }

   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   return e->devices;
}

EAPI void
evas_device_name_set(Evas_Device *dev, const char *name)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_name_set(dev, name);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_name_get(const Evas_Device *dev)
{
   return efl_input_device_name_get(dev);
}

EAPI void
evas_device_description_set(Evas_Device *dev, const char *desc)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_description_set(dev, desc);

   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const char *
evas_device_description_get(const Evas_Device *dev)
{
   return efl_input_device_description_get(dev);
}

EAPI void
evas_device_parent_set(Evas_Device *dev, Evas_Device *parent)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);
   Evas_Public_Data *e = eo_data_scope_get(d->evas, EVAS_CANVAS_CLASS);
   if (parent)
     {
        SAFETY_CHECK(parent, EFL_INPUT_DEVICE_CLASS);
     }

   /* FIXME: move this to Efl.Input.Device */
   if (d->parent == parent) return;
   if (d->parent)
     {
        Efl_Input_Device_Data *p = eo_data_scope_get(d->parent, EFL_INPUT_DEVICE_CLASS);
        p->children = eina_list_remove(p->children, dev);
     }
   else if (parent)
     e->devices = eina_list_remove(e->devices, dev);
   d->parent = parent;
   if (parent)
     {
        Efl_Input_Device_Data *p = eo_data_scope_get(parent, EFL_INPUT_DEVICE_CLASS);
        p->children = eina_list_append(p->children, dev);
     }
   else
     e->devices = eina_list_append(e->devices, dev);
   
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_parent_get(const Evas_Device *dev)
{
   return efl_input_device_parent_get(dev);
}

EAPI void
evas_device_class_set(Evas_Device *dev, Evas_Device_Class clas)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);

   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_type_set(dev, clas);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Class
evas_device_class_get(const Evas_Device *dev)
{
   return efl_input_device_type_get(dev);
}

EAPI void
evas_device_subclass_set(Evas_Device *dev, Evas_Device_Subclass clas)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_subtype_set(dev, clas);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI Evas_Device_Subclass
evas_device_subclass_get(const Evas_Device *dev)
{
   return efl_input_device_subtype_get(dev);
}

EAPI void
evas_device_emulation_source_set(Evas_Device *dev, Evas_Device *src)
{
   SAFETY_CHECK(dev, EFL_INPUT_DEVICE_CLASS);
   Efl_Input_Device_Data *d = eo_data_scope_get(dev, EFL_INPUT_DEVICE_CLASS);

   efl_input_device_source_set(dev, src);
   evas_event_callback_call(d->evas, EVAS_CALLBACK_DEVICE_CHANGED, dev);
}

EAPI const Evas_Device *
evas_device_emulation_source_get(const Evas_Device *dev)
{
   return efl_input_device_source_get(dev);
}

void
_evas_device_cleanup(Evas *eo_e)
{
   Evas_Device *dev;
   
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   if (e->cur_device)
     {
        while ((dev = eina_array_pop(e->cur_device)))
          eo_unref(dev);
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
