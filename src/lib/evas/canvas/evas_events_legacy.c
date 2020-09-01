#include "evas_common_private.h"
#include "evas_private.h"

typedef union {
   Evas_Event_Mouse_Down down;
   Evas_Event_Mouse_Up up;
   Evas_Event_Mouse_In in;
   Evas_Event_Mouse_Out out;
   Evas_Event_Mouse_Move move;
   Evas_Event_Mouse_Wheel wheel;
   Evas_Event_Multi_Down mdown;
   Evas_Event_Multi_Up mup;
   Evas_Event_Multi_Move mmove;
   Evas_Event_Key_Down kdown;
   Evas_Event_Key_Up kup;
   Evas_Event_Hold hold;
   Evas_Event_Axis_Update axis;
} Evas_Event_Any;

#define EV_SIZE sizeof(Evas_Event_Any)

static inline void *
_event_alloc(void *old)
{
   if (old)
     memset(old, 0, EV_SIZE);
   else
     old = calloc(1, EV_SIZE);
   return old;
}

void *
efl_input_pointer_legacy_info_fill(Evas *eo_evas, Efl_Input_Key *eo_ev, Evas_Callback_Type type, Evas_Event_Flags **pflags)
{
   Efl_Input_Pointer_Data *ev = efl_data_scope_get(eo_ev, EFL_INPUT_POINTER_CLASS);
   Evas_Public_Data *evas;
   Evas_Pointer_Data *pdata;

   if (!ev) return NULL;
   if (!eo_evas) eo_evas = efl_provider_find(eo_ev, EVAS_CANVAS_CLASS);
   evas = efl_data_scope_get(eo_evas, EVAS_CANVAS_CLASS);
   if (!evas) return NULL;
   pdata = _evas_pointer_data_by_device_get(evas, ev->device);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, NULL);

#define COORD_DUP(e) do { (e)->output.x = pdata->seat->x; (e)->output.y = pdata->seat->y; } while (0)
#define COORD_DUP_CUR(e) do { (e)->cur.output.x = pdata->seat->x; (e)->cur.output.y = pdata->seat->y; } while (0)
#define COORD_DUP_PREV(e) do { (e)->prev.output.x = pdata->seat->prev.x; (e)->prev.output.y = pdata->seat->prev.y; } while (0)
#define TYPE_CHK(typ) do { if ((type != EVAS_CALLBACK_LAST) && (type != EVAS_CALLBACK_ ## typ)) return NULL; } while (0)

   switch (ev->action)
     {
      case EFL_POINTER_ACTION_IN:
        TYPE_CHK(MOUSE_IN);
        {
           Evas_Event_Mouse_In *e = _event_alloc(ev->legacy);
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           COORD_DUP(e);
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = (Evas_Event_Flags)ev->event_flags;
           e->dev = ev->device;
           e->event_src = ev->source;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           if (pflags) *pflags = &e->event_flags;
           ev->legacy = e;
           return e;
        }

      case EFL_POINTER_ACTION_OUT:
        TYPE_CHK(MOUSE_OUT);
        {
           Evas_Event_Mouse_Out *e = _event_alloc(ev->legacy);
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           COORD_DUP(e);
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = (Evas_Event_Flags)ev->event_flags;
           e->dev = ev->device;
           e->event_src = ev->source;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           if (pflags) *pflags = &e->event_flags;
           ev->legacy = e;
           return e;
        }

      case EFL_POINTER_ACTION_DOWN:
        if (ev->touch_id == 0)
          {
             // filter out MULTI with finger 0, valid for eo, invalid for legacy
             if (type == EVAS_CALLBACK_MULTI_DOWN)
               return NULL;
             TYPE_CHK(MOUSE_DOWN);
             Evas_Event_Mouse_Down *e = _event_alloc(ev->legacy);
             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = (Evas_Button_Flags)ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }
        else
          {
             TYPE_CHK(MULTI_DOWN);
             Evas_Event_Multi_Down *e = _event_alloc(ev->legacy);
             e->device = ev->touch_id;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->canvas.xsub = ev->cur.x;
             e->canvas.ysub = ev->cur.y;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = (Evas_Button_Flags)ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }

      case EFL_POINTER_ACTION_UP:
        if (ev->touch_id == 0)
          {
             // filter out MULTI with finger 0, valid for eo, invalid for legacy
             if (type == EVAS_CALLBACK_MULTI_UP)
               return NULL;
             TYPE_CHK(MOUSE_UP);
             Evas_Event_Mouse_Up *e = _event_alloc(ev->legacy);
             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = (Evas_Button_Flags)ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }
        else
          {
             TYPE_CHK(MULTI_UP);
             Evas_Event_Multi_Up *e = _event_alloc(ev->legacy);
             e->device = ev->touch_id;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->canvas.xsub = ev->cur.x;
             e->canvas.ysub = ev->cur.y;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = (Evas_Button_Flags)ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }

      case EFL_POINTER_ACTION_MOVE:
        if (ev->touch_id == 0)
          {
             // filter out MULTI with finger 0, valid for eo, invalid for legacy
             if (type == EVAS_CALLBACK_MULTI_MOVE)
               return NULL;
             TYPE_CHK(MOUSE_MOVE);
             Evas_Event_Mouse_Move *e = _event_alloc(ev->legacy);
             e->buttons = ev->pressed_buttons;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             e->prev.canvas.x = ev->prev.x;
             e->prev.canvas.y = ev->prev.y;
             COORD_DUP_CUR(e);
             COORD_DUP_PREV(e);
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }
        else
          {
             TYPE_CHK(MULTI_MOVE);
             Evas_Event_Multi_Move *e = _event_alloc(ev->legacy);
             e->device = ev->touch_id;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->cur.canvas.xsub = ev->cur.x;
             e->cur.canvas.ysub = ev->cur.y;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             COORD_DUP_CUR(e);
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = (Evas_Event_Flags)ev->event_flags;
             e->dev = ev->device;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }

      case EFL_POINTER_ACTION_WHEEL:
        {
           TYPE_CHK(MOUSE_WHEEL);
           Evas_Event_Mouse_Wheel *e = _event_alloc(ev->legacy);
           e->direction = ev->wheel.horizontal;
           e->z = ev->wheel.z;
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           COORD_DUP(e);
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = (Evas_Event_Flags)ev->event_flags;
           e->dev = ev->device;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           if (pflags) *pflags = &e->event_flags;
           ev->legacy = e;
           return e;
        }

      case EFL_POINTER_ACTION_AXIS:
        {
           TYPE_CHK(AXIS_UPDATE);
           Evas_Event_Axis_Update *e = ev->legacy;
           Evas_Axis *tmp_axis;
           if (e && e->axis) free(e->axis);
           e = _event_alloc(ev->legacy);
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->dev = ev->device;
           /* FIXME: Get device id from above device object. 0 for now. */
           e->device = 0;
           e->toolid = ev->touch_id;
           e->axis = malloc(sizeof(Evas_Axis) * 11);
           e->axis[e->naxis].label = EVAS_AXIS_LABEL_WINDOW_X;
           e->axis[e->naxis].value = ev->cur.x;
           e->naxis++;
           e->axis[e->naxis].label = EVAS_AXIS_LABEL_WINDOW_Y;
           e->axis[e->naxis].value = ev->cur.y;
           e->naxis++;
           if (ev->has_raw)
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_X;
                e->axis[e->naxis].value = ev->raw.x;
                e->naxis++;
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_Y;
                e->axis[e->naxis].value = ev->raw.y;
                e->naxis++;
             }
           if (ev->has_norm)
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_NORMAL_X;
                e->axis[e->naxis].value = ev->norm.x;
                e->naxis++;
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_NORMAL_Y;
                e->axis[e->naxis].value = ev->norm.y;
                e->naxis++;
             }
           if (_efl_input_value_has(ev, EFL_INPUT_VALUE_PRESSURE))
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_PRESSURE;
                e->axis[e->naxis].value = ev->pressure;
                e->naxis++;
             }
           if (_efl_input_value_has(ev, EFL_INPUT_VALUE_DISTANCE))
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_DISTANCE;
                e->axis[e->naxis].value = ev->distance;
                e->naxis++;
             }
           if (_efl_input_value_has(ev, EFL_INPUT_VALUE_AZIMUTH))
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_AZIMUTH;
                e->axis[e->naxis].value = ev->azimuth;
                e->naxis++;
             }
           if (_efl_input_value_has(ev, EFL_INPUT_VALUE_TILT))
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_TILT;
                e->axis[e->naxis].value = ev->tilt;
                e->naxis++;
             }
           if (_efl_input_value_has(ev, EFL_INPUT_VALUE_TWIST))
             {
                e->axis[e->naxis].label = EVAS_AXIS_LABEL_TWIST;
                e->axis[e->naxis].value = ev->twist;
                e->naxis++;
             }
           tmp_axis = realloc(e->axis, e->naxis * sizeof(Evas_Axis));
           if (tmp_axis) e->axis = tmp_axis;
           if (pflags) *pflags = NULL;
           ev->legacy = e;
           return e;
        }

      default:
        return NULL;
     }
}

void *
efl_input_key_legacy_info_fill(Efl_Input_Key *evt, Evas_Event_Flags **pflags)
{
   Efl_Input_Key_Data *ev;

   ev = efl_data_scope_get(evt, EFL_INPUT_KEY_CLASS);
   if (!ev) return NULL;

   if (ev->pressed)
     {
        Evas_Event_Key_Down *e = _event_alloc(ev->legacy);
        e->timestamp = ev->timestamp;
        e->keyname = (char *) ev->keyname;
        e->key = ev->key;
        e->string = ev->string;
        e->compose = ev->compose;
        e->keycode = ev->keycode;
        e->data = ev->data;
        e->modifiers = ev->modifiers;
        e->locks = ev->locks;
        e->event_flags = (Evas_Event_Flags)ev->event_flags;
        e->dev = ev->device;
        if (pflags) *pflags = &e->event_flags;
        ev->legacy = e;
        return e;
     }
   else
     {
        Evas_Event_Key_Up *e = _event_alloc(ev->legacy);
        e->timestamp = ev->timestamp;
        e->keyname = (char *) ev->keyname;
        e->key = ev->key;
        e->string = ev->string;
        e->compose = ev->compose;
        e->keycode = ev->keycode;
        e->data = ev->data;
        e->modifiers = ev->modifiers;
        e->locks = ev->locks;
        e->event_flags = (Evas_Event_Flags)ev->event_flags;
        e->dev = ev->device;
        if (pflags) *pflags = &e->event_flags;
        ev->legacy = e;
        return e;
     }
}

void *
efl_input_hold_legacy_info_fill(Efl_Input_Hold *evt, Evas_Event_Flags **pflags)
{
   Efl_Input_Hold_Data *ev = efl_data_scope_get(evt, EFL_INPUT_HOLD_CLASS);
   Evas_Event_Hold *e;

   if (!ev) return NULL;

   e = _event_alloc(ev->legacy);
   e->timestamp = ev->timestamp;
   e->dev = ev->device;
   e->hold = ev->hold;
   e->event_flags = (Evas_Event_Flags)ev->event_flags;
   if (pflags) *pflags = &e->event_flags;
   ev->legacy = e;

   return e;
}
