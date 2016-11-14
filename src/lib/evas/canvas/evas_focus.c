#include "evas_common_private.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */


static Eina_Bool
_already_focused(Eina_List *seats, Efl_Input_Device *seat)
{
   Eina_List *l;
   const Efl_Input_Device *s;

   EINA_LIST_FOREACH(seats, l, s)
     {
        if (s == seat)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Efl_Input_Device *
_default_seat_get(Eo *evas_obj)
{
   Evas_Public_Data *edata;
   Evas *evas = evas_object_evas_get(evas_obj);

   edata = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);
   if (!edata) return NULL;
   return edata->default_seat;
}

static void
_evas_focus_set(Eo *evas_obj, Efl_Input_Device *key, Eina_Bool focus)
{
   Evas_Public_Data *edata;
   Evas *evas = evas_object_evas_get(evas_obj);

   edata = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);

   if (focus)
     eina_hash_add(edata->focused_objects, &key, evas_obj);
   else
     eina_hash_del_by_key(edata->focused_objects, &key);
}

static Eo *
_current_focus_get(Eo *evas_obj, Efl_Input_Device *key)
{
   Evas_Public_Data *edata;
   Evas *evas = evas_object_evas_get(evas_obj);

   edata = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);

   return eina_hash_find(edata->focused_objects, &key);
}

static void
_evas_object_unfocus(Evas_Object_Protected_Data *obj, Efl_Input_Device *seat)
{
   obj->focused_by_seats = eina_list_remove(obj->focused_by_seats, seat);
   _evas_focus_set(obj->object, seat, EINA_FALSE);

   //Legacy events...
   if (seat == obj->layer->evas->default_seat)
     {
        evas_object_event_callback_call(obj->object, obj,
                                        EVAS_CALLBACK_FOCUS_OUT,
                                        NULL, _evas_object_event_new(),
                                        EFL_CANVAS_OBJECT_EVENT_FOCUS_OUT);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
                                 obj->object);
     }
   efl_event_callback_call(obj->object,
                           EFL_CANVAS_OBJECT_EVENT_FOCUS_DEVICE_OUT,
                           seat);
   _evas_post_event_callback_call(obj->layer->evas->evas,
                                  obj->layer->evas);
}

void
_evas_focus_device_del_cb(void *data, const Efl_Event *ev)
{
   _evas_object_unfocus(data, ev->object);
}

EOLIAN Eina_Bool
_efl_canvas_object_seat_focus_del(Eo *eo_obj,
                                  Evas_Object_Protected_Data *obj,
                                  Efl_Input_Device *seat)
{
   Eina_List *l;
   Efl_Input_Device *dev;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (!seat) seat = _default_seat_get(eo_obj);

   EINA_LIST_FOREACH(obj->focused_by_seats, l, dev)
     {
        if (dev != seat)
          continue;
        if (_evas_object_intercept_call(eo_obj,
                                        EVAS_OBJECT_INTERCEPT_CB_FOCUS_SET,
                                        1, EINA_FALSE))
          {
             return EINA_FALSE;
          }

        efl_event_callback_del(dev, EFL_EVENT_DEL,
                               _evas_focus_device_del_cb, obj);
        _evas_object_unfocus(obj, dev);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_canvas_object_seat_focus_add(Eo *eo_obj,
                                  Evas_Object_Protected_Data *obj,
                                  Efl_Input_Device *seat)
{
   Eo *current_focus;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (!seat) seat = _default_seat_get(eo_obj);

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, EINA_FALSE);
   if (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_CLASS_SEAT)
     return EINA_FALSE;

   if (_already_focused(obj->focused_by_seats, seat))
     goto end;

   if (_evas_object_intercept_call(eo_obj, EVAS_OBJECT_INTERCEPT_CB_FOCUS_SET,
                                   1, EINA_TRUE))
     {
        return EINA_FALSE;
     }

   current_focus = _current_focus_get(eo_obj, seat);
   if (current_focus)
     efl_canvas_object_seat_focus_del(current_focus, seat);

   //In case intercept focus callback focused object we should return.
   if (_current_focus_get(eo_obj, seat)) goto end;

   efl_event_callback_add(seat, EFL_EVENT_DEL, _evas_focus_device_del_cb, obj);

   obj->focused_by_seats = eina_list_append(obj->focused_by_seats, seat);
   _evas_focus_set(eo_obj, seat, EINA_TRUE);

   //Legacy events...
   if (seat == _default_seat_get(eo_obj))
     {
        evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FOCUS_IN,
                                        NULL, _evas_object_event_new(),
                                        EFL_CANVAS_OBJECT_EVENT_FOCUS_IN);
        evas_event_callback_call(obj->layer->evas->evas,
                                 EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN, eo_obj);
     }

   efl_event_callback_call(eo_obj,
                           EFL_CANVAS_OBJECT_EVENT_FOCUS_DEVICE_IN, seat);
 end:
   _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas);
   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_canvas_object_seat_focus_check(Eo *eo_obj,
                                    Evas_Object_Protected_Data *obj,
                                    Efl_Input_Device *seat)
{
   Eina_List *l;
   Efl_Input_Device *s;

   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if (!seat) seat = _default_seat_get(eo_obj);

   EINA_LIST_FOREACH(obj->focused_by_seats, l, s)
     {
        if (s == seat)
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN void
_efl_canvas_object_key_focus_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool focus)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (focus)
     _efl_canvas_object_seat_focus_add(eo_obj, obj, NULL);
   else
     _efl_canvas_object_seat_focus_del(eo_obj, obj, NULL);
}

EOLIAN Eina_Bool
_efl_canvas_object_seat_focus_get(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return eina_list_count(obj->focused_by_seats) ? EINA_TRUE : EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_canvas_object_key_focus_get(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   return _efl_canvas_object_seat_focus_check(eo_obj, obj, NULL);
}

EOLIAN Evas_Object *
_evas_canvas_seat_focus_get(Eo *eo_obj EINA_UNUSED, Evas_Public_Data *e,
                            Efl_Input_Device *seat)
{
   if (!seat)
     seat = e->default_seat;

   return eina_hash_find(e->focused_objects, &seat);
}

EOLIAN Evas_Object*
_evas_canvas_focus_get(Eo *eo_obj EINA_UNUSED, Evas_Public_Data *e)
{
   return _evas_canvas_seat_focus_get(eo_obj, e, NULL);
}

EOLIAN Eina_Iterator *
_evas_canvas_focused_objects_get(Eo *eo_obj EINA_UNUSED, Evas_Public_Data *e)
{
   return eina_hash_iterator_tuple_new(e->focused_objects);
}
