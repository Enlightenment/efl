#include "evas_common_private.h"
#include "evas_private.h"

/* local calls */

void
evas_object_inform_call_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_SHOW, NULL, _evas_object_event_new(), EFL_GFX_EVENT_SHOW);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_hide(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HIDE, NULL, _evas_object_event_new(), EFL_GFX_EVENT_HIDE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_move(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOVE, NULL, _evas_object_event_new(), EFL_GFX_EVENT_MOVE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESIZE, NULL, _evas_object_event_new(), EFL_GFX_EVENT_RESIZE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_restack(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESTACK, NULL, _evas_object_event_new(), EFL_GFX_EVENT_RESTACK);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_changed_size_hints(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, NULL, _evas_object_event_new(), EFL_GFX_EVENT_CHANGE_SIZE_HINTS);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_preloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if (!_evas_object_image_preloading_get(eo_obj)) return;
   _evas_object_image_preloading_check(eo_obj);
   _evas_object_image_preloading_set(eo_obj, 0);

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, _evas_object_event_new(), EFL_IMAGE_EVENT_PRELOAD);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_unloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_UNLOADED, NULL, _evas_object_event_new(), EFL_IMAGE_EVENT_UNLOAD);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_RESIZE, NULL, _evas_object_event_new(), EFL_IMAGE_EVENT_RESIZE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}
