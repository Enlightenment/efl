#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private calls */

/* local calls */

/* public calls */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_object_focus_set(Evas_Object *obj, Evas_Bool focus)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   
   if (focus)
     {
	if (obj->focused) return;
	if (obj->layer->evas->focused)
	  evas_object_focus_set(obj->layer->evas->focused, 0);
	obj->focused = 1;
	obj->layer->evas->focused = obj;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_IN, NULL);
     }
   else
     {
	if (!obj->focused) return;
	obj->focused = 0;
	obj->layer->evas->focused = NULL;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_OUT, NULL);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Bool
evas_object_focus_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();   
   return obj->focused;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Object *
evas_focus_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->focused;
}
