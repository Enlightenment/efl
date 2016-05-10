#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

Eina_Bool
efl_pointer_event_legacy_info_set(Efl_Pointer_Event *evt, const void *event_info, Evas_Callback_Type type)
{
   Efl_Pointer_Event_Data *ev = eo_data_scope_get(evt, EFL_POINTER_EVENT_CLASS);
   if (!ev || !event_info) return EINA_FALSE;

   /* FIXME/TODO:
    *
    * Some things that are not supported or should be eo-ified somehow
    *
    * 1. Lock
    * 2. Modifiers
    * 3. data
    *
    * Some events definitely don't have all the info they could have. Need
    * to get it from ecore!
    */

#warning Modifiers and locks not supported yet - very bad!

   switch (type)
     {
      //cse EVAS_CALLBACK_MOUSE_IN:
      //case EVAS_CALLBACK_MOUSE_OUT:
      case EVAS_CALLBACK_MOUSE_DOWN:
        {
           const Evas_Event_Mouse_Down *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_DOWN;
           ev->button = e->button;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           break;
        }

      case EVAS_CALLBACK_MOUSE_UP:
        {
           const Evas_Event_Mouse_Up *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_UP;
           ev->button = e->button;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           break;
        }

      case EVAS_CALLBACK_MOUSE_MOVE:
        {
           const Evas_Event_Mouse_Move *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_MOVE;
           ev->pressed_buttons = e->buttons;
           ev->cur.x = e->cur.canvas.x;
           ev->cur.y = e->cur.canvas.y;
           ev->cur.xsub = e->cur.canvas.x;
           ev->cur.ysub = e->cur.canvas.y;
           ev->prev.x = e->prev.canvas.x;
           ev->prev.y = e->prev.canvas.y;
           ev->prev.xsub = e->prev.canvas.x;
           ev->prev.ysub = e->prev.canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           ev->source = e->event_src;
           break;
        }

      case EVAS_CALLBACK_MOUSE_WHEEL:
        {
           const Evas_Event_Mouse_Wheel *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_WHEEL;
           ev->wheel.dir = (e->direction ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL);
           ev->wheel.z = e->z;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           break;
        }

      case EVAS_CALLBACK_MULTI_DOWN:
        {
           const Evas_Event_Multi_Down *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_DOWN; // TOUCH DOWN???
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.xsub;
           ev->cur.ysub = e->canvas.ysub;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           break;
        }

      case EVAS_CALLBACK_MULTI_UP:
        {
           const Evas_Event_Multi_Up *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_UP;
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.xsub;
           ev->cur.ysub = e->canvas.ysub;
           ev->data = e->data;
           ev->button_flags = e->flags;
           ev->timestamp = e->timestamp;
           ev->event_flags = e->event_flags;
           ev->device = e->dev;
           break;
        }

      case EVAS_CALLBACK_MULTI_MOVE:
        {
           const Evas_Event_Multi_Move *e = event_info;
           ev->action = EFL_POINTER_ACTION_MOUSE_MOVE;
           ev->finger = e->device;
           ev->radius = e->radius;
           ev->radius_x = e->radius_x;
           ev->radius_y = e->radius_y;
           ev->pressure = e->pressure;
           ev->angle = e->angle;
           ev->cur.x = e->cur.canvas.x;
           ev->cur.y = e->cur.canvas.y;
           ev->cur.xsub = e->cur.canvas.xsub;
           ev->cur.ysub = e->cur.canvas.ysub;
           ev->data = e->data;
           ev->timestamp = e->timestamp;
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

const void *
efl_pointer_event_legacy_info_get(const Efl_Pointer_Event *evt, Evas_Callback_Type *ptype, Eina_Bool multi)
{
   Efl_Pointer_Event_Data *ev = eo_data_scope_get(evt, EFL_POINTER_EVENT_CLASS);
   if (!ev) return NULL;

   switch (ev->action)
     {
      case EFL_POINTER_ACTION_MOUSE_DOWN:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_DOWN;
             ev->legacy = e;

             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             e->output.x = ev->cur.x;
             e->output.y = ev->cur.y;
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
          }
        else
          {
             Evas_Event_Multi_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_DOWN;
             ev->legacy = e;

             e->device = ev->finger;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             e->canvas.xsub = ev->cur.xsub;
             e->canvas.ysub = ev->cur.ysub;
             e->output.x = ev->cur.x;
             e->output.y = ev->cur.y;
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
          }
        break;

      case EFL_POINTER_ACTION_MOUSE_UP:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Up *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_UP;
             ev->legacy = e;

             e->button = ev->button;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             e->output.x = ev->cur.x;
             e->output.y = ev->cur.y;
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
          }
        else
          {
             Evas_Event_Multi_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_UP;
             ev->legacy = e;

             e->device = ev->finger;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->canvas.x = ev->cur.x;
             e->canvas.y = ev->cur.y;
             e->canvas.xsub = ev->cur.xsub;
             e->canvas.ysub = ev->cur.ysub;
             e->output.x = ev->cur.x;
             e->output.y = ev->cur.y;
             e->data = ev->data;
             e->flags = ev->button_flags;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             break;
          }
        break;

      case EFL_POINTER_ACTION_MOUSE_MOVE:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Move *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_MOVE;
             ev->legacy = e;

             e->buttons = ev->pressed_buttons;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             e->cur.output.x = ev->cur.x;
             e->cur.output.y = ev->cur.y;
             e->prev.canvas.x = ev->prev.x;
             e->prev.canvas.y = ev->prev.y;
             e->prev.output.x = ev->prev.x;
             e->prev.output.y = ev->prev.y;
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
             e->event_src = ev->source;
          }
        else
          {
             Evas_Event_Multi_Move *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_MOVE;
             ev->legacy = e;

             e->device = ev->finger;
             e->radius = ev->radius;
             e->radius_x = ev->radius_x;
             e->radius_y = ev->radius_y;
             e->pressure = ev->pressure;
             e->angle = ev->angle;
             e->cur.canvas.x = ev->cur.x;
             e->cur.canvas.y = ev->cur.y;
             e->cur.canvas.xsub = ev->cur.xsub;
             e->cur.canvas.ysub = ev->cur.ysub;
             e->cur.output.x = ev->cur.x;
             e->cur.output.y = ev->cur.y;
             e->data = ev->data;
             e->timestamp = ev->timestamp;
             e->event_flags = ev->event_flags;
             e->dev = ev->device;
          }
        break;

      case EFL_POINTER_ACTION_MOUSE_WHEEL:
        {
           Evas_Event_Mouse_Wheel *e = calloc(1, sizeof(*e));
           if (ptype) *ptype = EVAS_CALLBACK_MOUSE_WHEEL;
           ev->legacy = e;

           e->direction = (ev->wheel.dir == EFL_ORIENT_VERTICAL);
           e->z = ev->wheel.z;
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           e->output.x = ev->cur.x;
           e->output.y = ev->cur.y;
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = ev->event_flags;
           e->dev = ev->device;
           break;
        }

      default:
        ERR("invalid event type %d", ev->action);
        return NULL;
     }

   return ev->legacy;
}
