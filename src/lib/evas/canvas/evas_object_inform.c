#include "evas_common_private.h"
#include "evas_image_private.h"
#include "evas_private.h"

/* BEGIN: events to maintain compatibility with legacy */
EWAPI const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_SHOW =
   EFL_EVENT_DESCRIPTION("show");
EWAPI const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_HIDE =
   EFL_EVENT_DESCRIPTION("hide");
/* END: events to maintain compatibility with legacy */

/* local calls */

void
evas_object_inform_call_show(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();
   Eina_Bool vis = EINA_TRUE;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_SHOW, &vis, event_id, EFL_GFX_ENTITY_EVENT_SHOW);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();
   Eina_Bool vis = EINA_FALSE;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HIDE, &vis, event_id, EFL_GFX_ENTITY_EVENT_HIDE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_move(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Eina_Position2D pos;
   int event_id = _evas_object_event_new();

   pos = ((Eina_Rect) obj->cur->geometry).pos;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOVE, &pos, event_id, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_resize(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Eina_Size2D size;
   int event_id = _evas_object_event_new();

   size = ((Eina_Rect) obj->cur->geometry).size;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESIZE, &size, event_id, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_restack(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESTACK, NULL, event_id, EFL_GFX_ENTITY_EVENT_STACKING_CHANGED);
   if (obj->layer)
     _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_changed_size_hints(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, NULL, event_id, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_image_preloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int event_id;

   EINA_SAFETY_ON_NULL_RETURN(obj);

   unsigned char preload = _evas_object_image_preloading_get(eo_obj);

   //Even cancelled, obj needs to draw image.
   _evas_image_load_post_update(eo_obj, obj);

   if ((preload & EVAS_IMAGE_PRELOADING) ||
     /* Boom! This cancellation call stack is in the intermediate render sequence. Need better idea.
          So far, this cancellation is triggered by other non-preload image instances,
          which doesn't require preloading. So by mechasnim we cancel preload other instances as well.
          and mimic as it finished preloading done. */
       (preload & EVAS_IMAGE_PRELOAD_CANCEL))
     {
        event_id = _evas_object_event_new();
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, event_id, EFL_GFX_IMAGE_EVENT_IMAGE_PRELOAD);
        _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
     }
}

void
evas_object_inform_call_image_unloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_UNLOADED, NULL, event_id, EFL_GFX_IMAGE_EVENT_IMAGE_UNLOAD);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_image_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_RESIZE, NULL, event_id, EFL_GFX_IMAGE_EVENT_IMAGE_RESIZE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}
