#include "evas_common_private.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */

EOLIAN void
_evas_object_focus_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool focus)
{

   int event_id = 0;
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   _evas_object_event_new();

   event_id = _evas_event_counter;
   if (obj->focused == focus) goto end;
   if (evas_object_intercept_call_focus_set(eo_obj, obj, focus)) goto end;
   if (focus)
     {
        if (obj->layer->evas->focused)
	  eo_do(obj->layer->evas->focused, evas_obj_focus_set(0));
	
        if (obj->layer->evas->focused) goto end;
        obj->focused = 1;
        obj->layer->evas->focused = eo_obj;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_IN, NULL, event_id);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, eo_obj);
     }
   else
     {
        obj->focused = 0;
        obj->layer->evas->focused = NULL;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_OUT, NULL, event_id);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, eo_obj);
     }

 end:
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

EOLIAN Eina_Bool
_evas_object_focus_get(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->focused;
}

EAPI Evas_Object *
evas_focus_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *ret = NULL;
   eo_do((Eo *)e, evas_canvas_focus_get(&ret));
   return ret;
}

void
_canvas_focus_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   const Evas_Public_Data *e = _pd;
   *ret = e->focused;
}

