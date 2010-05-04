#include "evas_common.h"
#include "evas_private.h"

/* local calls */

void
evas_object_inform_call_show(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_SHOW, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_hide(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_HIDE, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_move(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_MOVE, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_resize(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_RESIZE, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_restack(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_RESTACK, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_changed_size_hints(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}

void
evas_object_inform_call_image_preloaded(Evas_Object *obj)
{
   _evas_object_event_new();

   evas_object_event_callback_call(obj, EVAS_CALLBACK_IMAGE_PRELOADED, NULL);
   _evas_post_event_callback_call(obj->layer->evas);
}
