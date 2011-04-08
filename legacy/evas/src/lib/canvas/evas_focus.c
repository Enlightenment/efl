#include "evas_common.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */

EAPI void
evas_object_focus_set(Evas_Object *obj, Eina_Bool focus)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   _evas_object_event_new();

   if (focus)
     {
	if (obj->focused) goto end;
	if (obj->layer->evas->focused)
	  evas_object_focus_set(obj->layer->evas->focused, 0);
	obj->focused = 1;
	obj->layer->evas->focused = obj;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_IN, NULL);
        evas_event_callback_call(obj->layer->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, obj);
     }
   else
     {
	if (!obj->focused) goto end;
	obj->focused = 0;
	obj->layer->evas->focused = NULL;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_OUT, NULL);
        evas_event_callback_call(obj->layer->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT, obj);
     }
   end:
   _evas_post_event_callback_call(obj->layer->evas);
}

EAPI Eina_Bool
evas_object_focus_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->focused;
}

EAPI Evas_Object *
evas_focus_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->focused;
}
