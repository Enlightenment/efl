#include "evas_image_private.h"
#include "efl_canvas_proxy.eo.h"

#define MY_CLASS EFL_CANVAS_PROXY_CLASS

EOLIAN static Eina_Bool
_efl_canvas_proxy_source_set(Eo *eo_obj, void *pd EINA_UNUSED, Evas_Object *eo_src)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   if (obj->delete_me && eo_src)
     {
        WRN("Setting deleted object %p as image source %p", eo_src, eo_obj);
        return EINA_FALSE;
     }
   if (eo_src)
     {
        Evas_Object_Protected_Data *src = eo_data_scope_get(eo_src, EVAS_OBJECT_CLASS);
        if (src->delete_me)
          {
             WRN("Setting object %p to deleted image source %p", eo_src, eo_obj);
             return EINA_FALSE;
          }
        if (!src->layer)
          {
             CRI("No evas surface associated with source object (%p)", eo_src);
             return EINA_FALSE;
          }
        if (!obj->layer)
          {
             CRI("No evas surface associated with destination object (%p)", eo_obj);
             return EINA_FALSE;
          }
        if ((obj->layer && src->layer) &&
            (obj->layer->evas != src->layer->evas))
          {
             CRI("Setting object %p from Evas (%p) from another Evas (%p)", eo_src, src->layer->evas, obj->layer->evas);
             return EINA_FALSE;
          }
        if (eo_src == eo_obj)
          {
             CRI("Setting object %p as a source for itself", obj);
             return EINA_FALSE;
          }
     }
   if (o->cur->source == eo_src) return EINA_TRUE;
   evas_object_async_block(obj);
   _evas_object_image_cleanup(eo_obj, obj, o);
   /* Kill the image if any */
   if (o->cur->u.file || o->cur->key)
     evas_object_image_file_set(eo_obj, NULL, NULL);

   if (eo_src) _proxy_set(eo_obj, eo_src);
   else _proxy_unset(eo_obj, obj, o);

   return EINA_TRUE;
}

EOLIAN static Evas_Object *
_efl_canvas_proxy_source_get(Eo *eo_obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   return o->cur->source;
}

EOLIAN static void
_efl_canvas_proxy_source_clip_set(Eo *eo_obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool source_clip)
{
   Evas_Object_Protected_Data *src_obj;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   source_clip = !!source_clip;
   if (o->proxy_src_clip == source_clip) return;
   evas_object_async_block(obj);
   o->proxy_src_clip = source_clip;

   if (!o->cur->source) return;

   src_obj = eo_data_scope_get(o->cur->source, EVAS_OBJECT_CLASS);
   evas_object_change(o->cur->source, src_obj);
}

EOLIAN static Eina_Bool
_efl_canvas_proxy_source_clip_get(Eo *eo_obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);
   return o->proxy_src_clip;
}

EOLIAN static void
_efl_canvas_proxy_source_events_set(Eo *eo_obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool source_events)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Image_Data *o = eo_data_scope_get(eo_obj, EVAS_IMAGE_CLASS);

   source_events = !!source_events;
   if (obj->proxy->src_events == source_events) return;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy, Evas_Object_Proxy_Data, proxy_write)
     proxy_write->src_events = source_events;
   EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write);

   if (!o->cur->source) return;
   if ((obj->proxy->src_invisible) || (!source_events)) return;
   //FIXME: Feed mouse events here.
}

EOLIAN static Eina_Bool
_efl_canvas_proxy_source_events_get(Eo *eo_obj, void *pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   return obj->proxy->src_events;
}

#include "efl_canvas_proxy.eo.c"
