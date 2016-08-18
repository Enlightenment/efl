#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

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

Eina_Bool
efl_event_pointer_legacy_info_set(Efl_Event_Pointer *evt, const void *event_info, Evas_Callback_Type type)
{
   Efl_Event_Pointer_Data *ev = efl_data_scope_get(evt, EFL_EVENT_POINTER_CLASS);
   if (!ev || !event_info) return EINA_FALSE;

   /* FIXME/TODO:
    * Some events definitely don't have all the info they could have. Need
    * to get it from ecore!
    */

#if defined(DEBUG)
# define CHKACT(a) do { if (evdata->action != EFL_POINTER_ACTION_ ## a) abort(); } while (0)
#else
# define CHKACT(a) do {} while(0)
#endif

   switch (type)
     {
      case EVAS_CALLBACK_MOUSE_IN:
        CHKACT(IN);
        {
           const Evas_Event_Mouse_In *e = event_info;
           ev->action = EFL_POINTER_ACTION_IN;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MOUSE_OUT:
        CHKACT(OUT);
        {
           const Evas_Event_Mouse_Out *e = event_info;
           ev->action = EFL_POINTER_ACTION_OUT;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MOUSE_DOWN:
        CHKACT(DOWN);
        {
           const Evas_Event_Mouse_Down *e = event_info;
           ev->action = EFL_POINTER_ACTION_DOWN;
           ev->button = e->button;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MOUSE_UP:
        CHKACT(UP);
        {
           const Evas_Event_Mouse_Up *e = event_info;
           ev->action = EFL_POINTER_ACTION_UP;
           ev->button = e->button;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MOUSE_MOVE:
        CHKACT(MOVE);
        {
           const Evas_Event_Mouse_Move *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOVE;
           ev->pressed_buttons = e->buttons;
           ev->cur.x = e->cur.canvas.x;
           ev->cur.y = e->cur.canvas.y;
           ev->prev.x = e->prev.canvas.x;
           ev->prev.y = e->prev.canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MOUSE_WHEEL:
        CHKACT(WHEEL);
        {
           const Evas_Event_Mouse_Wheel *e = event_info;
           ev->action = EFL_POINTER_ACTION_WHEEL;
           ev->wheel.dir = (e->direction ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL);
           ev->wheel.z = e->z;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MULTI_DOWN:
        CHKACT(DOWN);
        {
           const Evas_Event_Multi_Down *e = event_info;
           ev->action = EFL_POINTER_ACTION_DOWN;
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->canvas.xsub;
           ev->cur.y = e->canvas.ysub;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MULTI_UP:
        CHKACT(UP);
        {
           const Evas_Event_Multi_Up *e = event_info;
           ev->action = EFL_POINTER_ACTION_UP;
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->canvas.xsub;
           ev->cur.y = e->canvas.ysub;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      case EVAS_CALLBACK_MULTI_MOVE:
        CHKACT(MOVE);
        {
           const Evas_Event_Multi_Move *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOVE;
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->cur.canvas.xsub;
           ev->cur.y = e->cur.canvas.ysub;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           break;
        }

      default:
        ERR("invalid event type %d", type);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void *
efl_event_pointer_legacy_info_fill(Efl_Event_Key *evt, Evas_Callback_Type type, Evas_Event_Flags **pflags)
{
   Efl_Event_Pointer_Data *ev = efl_data_scope_get(evt, EFL_EVENT_POINTER_CLASS);
   if (!ev) return NULL;

#define COORD_DUP(e) do { (e)->output.x = (e)->canvas.x; (e)->output.y = (e)->canvas.y; } while (0)
#define TYPE_CHK(typ) do { if (type != EVAS_CALLBACK_ ## typ) return NULL; } while (0)

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
           e->event_flags = ev->event_flags;
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
           e->event_flags = ev->event_flags;
           e->dev = ev->device;
           e->event_src = ev->source;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           if (pflags) *pflags = &e->event_flags;
           ev->legacy = e;
           return e;
        }

      case EFL_POINTER_ACTION_DOWN:
        if (ev->finger == 0)
          {
             TYPE_CHK(MOUSE_DOWN);
             Evas_Event_Mouse_Down *e = _event_alloc(ev->legacy);
             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
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
             e->device = ev->finger;
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
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }

      case EFL_POINTER_ACTION_UP:
        if (ev->finger == 0)
          {
             TYPE_CHK(MOUSE_UP);
             Evas_Event_Mouse_Up *e = _event_alloc(ev->legacy);
             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             COORD_DUP(e);
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
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
             e->device = ev->finger;
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
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             if (pflags) *pflags = &e->event_flags;
             ev->legacy = e;
             return e;
          }

      case EFL_POINTER_ACTION_MOVE:
        if (ev->finger == 0)
          {
             TYPE_CHK(MOUSE_MOVE);
             Evas_Event_Mouse_Move *e = _event_alloc(ev->legacy);
             e->buttons = ev->pressed_buttons;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             COORD_DUP(&e->cur);
             e->prev.canvas.x = ev->prev.x;
             e->prev.canvas.y = ev->prev.y;
             COORD_DUP(&e->prev);
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
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
             e->device = ev->finger;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->cur.canvas.xsub = ev->cur.x;
             e->cur.canvas.ysub = ev->cur.y;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             COORD_DUP(&e->cur);
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
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
           e->direction =  (ev->wheel.dir == EFL_ORIENT_HORIZONTAL) ? 1 : 0;
           e->z = ev->wheel.z;
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           COORD_DUP(e);
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = ev->event_flags;
           e->dev = ev->device;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           if (pflags) *pflags = &e->event_flags;
           ev->legacy = e;
           return e;
        }

      default:
        return NULL;
     }
}

Eina_Bool
efl_event_key_legacy_info_set(Efl_Event_Key *evt, const void *event_info, Evas_Callback_Type type)
{
   Efl_Event_Key_Data *ev = efl_data_scope_get(evt, EFL_EVENT_KEY_CLASS);
   if (!ev || !event_info) return EINA_FALSE;

   switch (type)
     {
      case EVAS_CALLBACK_KEY_DOWN:
        {
           const Evas_Event_Key_Down *e = event_info;
           ev->timestamp = e->timestamp;
           ev->pressed = EINA_TRUE;
           eina_stringshare_replace(&ev->keyname, e->keyname);
           eina_stringshare_replace(&ev->key, e->key);
           eina_stringshare_replace(&ev->string, e->string);
           eina_stringshare_replace(&ev->compose, e->compose);
           ev->keycode = e->keycode;
           ev->data = e->data;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           break;
        }

      case EVAS_CALLBACK_KEY_UP:
        {
           const Evas_Event_Key_Up *e = event_info;
           ev->timestamp = e->timestamp;
           ev->pressed = EINA_FALSE;
           eina_stringshare_replace(&ev->keyname, e->keyname);
           eina_stringshare_replace(&ev->key, e->key);
           eina_stringshare_replace(&ev->string, e->string);
           eina_stringshare_replace(&ev->compose, e->compose);
           ev->keycode = e->keycode;
           ev->data = e->data;
           ev->modifiers = e->modifiers;
           ev->locks = e->locks;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           break;
        }

      default:
        ERR("invalid event type %d", type);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void *
efl_event_key_legacy_info_fill(Efl_Event_Key *evt, Evas_Event_Flags **pflags)
{
   Efl_Event_Key_Data *ev;

   ev = efl_data_scope_get(evt, EFL_EVENT_KEY_CLASS);
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
        e->event_flags = ev->event_flags;
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
        e->event_flags = ev->event_flags;
        e->dev = ev->device;
        if (pflags) *pflags = &e->event_flags;
        ev->legacy = e;
        return e;
     }
}

Eina_Bool
efl_event_hold_legacy_info_set(Efl_Event_Hold *evt, const void *event_info, Evas_Callback_Type type)
{
   Efl_Event_Hold_Data *ev = efl_data_scope_get(evt, EFL_EVENT_HOLD_CLASS);
   const Evas_Event_Hold *e = event_info;

   if (!ev || !event_info) return EINA_FALSE;

   if (type != EVAS_CALLBACK_HOLD)
     {
        ERR("invalid event type %d", type);
        return EINA_FALSE;
     }

   ev->timestamp = e->timestamp;
   ev->device = e->dev;
   ev->hold = !!e->hold;
   ev->event_flags = e->event_flags;
   ev->data = e->data;

   return EINA_TRUE;
}

void *
efl_event_hold_legacy_info_fill(Efl_Event_Hold *evt, Evas_Event_Flags **pflags)
{
   Efl_Event_Hold_Data *ev = efl_data_scope_get(evt, EFL_EVENT_HOLD_CLASS);
   Evas_Event_Hold *e;

   if (!ev) return NULL;

   e = _event_alloc(ev->legacy);
   e->timestamp = ev->timestamp;
   e->dev = ev->device;
   e->hold = ev->hold;
   e->event_flags = ev->event_flags;
   if (pflags) *pflags = &e->event_flags;
   ev->legacy = e;

   return e;
}
