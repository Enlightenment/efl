#include "evas_common.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */

EAPI void
evas_object_focus_set(Evas_Object *eo_obj, Eina_Bool focus)
{
   eo_do(eo_obj, evas_obj_focus_set(focus));
}

void
_focus_set(Eo *eo_obj, void *_pd, va_list *list)
{
   Eina_Bool focus = va_arg(*list, int);

   int event_id = 0;
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   Evas_Object_Protected_Data *obj = _pd; 

   _evas_object_event_new();

   event_id = _evas_event_counter;
   if (focus)
     {
        if (obj->focused) goto end;
        if (obj->layer->evas->focused)
          evas_object_focus_set(obj->layer->evas->focused, 0);
        obj->focused = 1;
        obj->layer->evas->focused = eo_obj;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_IN, NULL, event_id);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, eo_obj);
     }
   else
     {
        if (!obj->focused) goto end;
        obj->focused = 0;
        obj->layer->evas->focused = NULL;
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_OUT, NULL, event_id);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, eo_obj);
     }
   end:
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
}

EAPI Eina_Bool
evas_object_focus_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool focus = 0;
   eo_do((Eo *)eo_obj, evas_obj_focus_get(&focus));
   return focus;
}

void
_focus_get(Eo *eo_obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *focus = va_arg(*list, Eina_Bool *);

   const Evas_Object_Protected_Data *obj = _pd;
   *focus = obj->focused;
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

