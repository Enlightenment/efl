#include "evas_common.h"
#include "evas_private.h"

/* local calls */

void
evas_object_inform_call_show(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_SHOW, NULL);
}

void
evas_object_inform_call_hide(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_HIDE, NULL);
}

void
evas_object_inform_call_move(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_MOVE, NULL);
}

void
evas_object_inform_call_resize(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_RESIZE, NULL);
}

void
evas_object_inform_call_restack(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_RESTACK, NULL);
}

void
evas_object_inform_call_changed_size_hints(Evas_Object *obj)
{
   evas_object_event_callback_call(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, NULL);
}
