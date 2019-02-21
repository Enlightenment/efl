#include "evas_common_private.h"
#include "evas_image_private.h"
#include "evas_private.h"

/* BEGIN: events to maintain compatibility with legacy */
EWAPI const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_MOVE =
   EFL_EVENT_DESCRIPTION("move");
EWAPI const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_RESIZE =
   EFL_EVENT_DESCRIPTION("resize");
EWAPI extern const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_MOVE;
#define EFL_GFX_ENTITY_EVENT_MOVE (&(_EFL_GFX_ENTITY_EVENT_MOVE))
EWAPI extern const Efl_Event_Description _EFL_GFX_ENTITY_EVENT_RESIZE;
#define EFL_GFX_ENTITY_EVENT_RESIZE (&(_EFL_GFX_ENTITY_EVENT_RESIZE))
/* END: events to maintain compatibility with legacy */

/* local calls */

void
evas_object_inform_call_show(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_SHOW, NULL, event_id, EFL_GFX_ENTITY_EVENT_SHOW);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_HIDE, NULL, event_id, EFL_GFX_ENTITY_EVENT_HIDE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_move(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Eina_Position2D pos;
   int event_id = _evas_object_event_new();

   pos = ((Eina_Rect) obj->cur->geometry).pos;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOVE, NULL, event_id, EFL_GFX_ENTITY_EVENT_MOVE);
   efl_event_callback_call(eo_obj, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, &pos);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_resize(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Eina_Size2D size;
   int event_id = _evas_object_event_new();

   size = ((Eina_Rect) obj->cur->geometry).size;

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESIZE, NULL, event_id, EFL_GFX_ENTITY_EVENT_RESIZE);
   efl_event_callback_call(eo_obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, &size);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_restack(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_RESTACK, NULL, event_id, EFL_GFX_ENTITY_EVENT_STACK_CHANGED);
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
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_PRELOADED, NULL, event_id, EFL_GFX_IMAGE_EVENT_PRELOAD);
        _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
     }
}

void
evas_object_inform_call_image_unloaded(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_UNLOADED, NULL, event_id, EFL_GFX_IMAGE_EVENT_UNLOAD);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}

void
evas_object_inform_call_image_resize(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int event_id = _evas_object_event_new();

   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_IMAGE_RESIZE, NULL, event_id, EFL_GFX_IMAGE_EVENT_RESIZE);
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
}
