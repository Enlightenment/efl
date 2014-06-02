#include "evas_common_private.h"
#include "evas_private.h"

/* local calls */

void
evas_object_inform_call_show(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_SHOW, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_hide(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HIDE, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_move(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOVE, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESIZE, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_restack(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESTACK, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_changed_size_hints(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_preloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if (!_evas_object_image_preloading_get(eo_obj)) return;
   _evas_object_image_preloading_check(eo_obj);
   _evas_object_image_preloading_set(eo_obj, 0);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_unloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_UNLOADED, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

void
evas_object_inform_call_image_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_event_new();
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_RESIZE, NULL, _evas_event_counter);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}
