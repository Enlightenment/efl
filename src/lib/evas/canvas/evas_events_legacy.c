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
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
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
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
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
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
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
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
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
           ev->cur.xsub = e->canvas.x;
           ev->cur.ysub = e->canvas.y;
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
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.xsub;
           ev->cur.ysub = e->canvas.ysub;
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
           ev->cur.x = e->canvas.x;
           ev->cur.y = e->canvas.y;
           ev->cur.xsub = e->canvas.xsub;
           ev->cur.ysub = e->canvas.ysub;
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
           ev->cur.x = e->cur.canvas.x;
           ev->cur.y = e->cur.canvas.y;
           ev->cur.xsub = e->cur.canvas.xsub;
           ev->cur.ysub = e->cur.canvas.ysub;
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

const void *
efl_pointer_event_legacy_info_get(const Efl_Pointer_Event *evt, Evas_Callback_Type *ptype, Eina_Bool multi)
{
   Efl_Pointer_Event_Data *ev = eo_data_scope_get(evt, EFL_POINTER_EVENT_CLASS);
   if (!ev) return NULL;

   switch (ev->action)
     {
      case EFL_POINTER_ACTION_IN:
        {
           Evas_Event_Mouse_In *e = calloc(1, sizeof(*e));
           if (ptype) *ptype = EVAS_CALLBACK_MOUSE_IN;
           ev->legacy = e;
           e->reserved = ev->eo;

           e->buttons = ev->pressed_buttons;
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           e->output.x = ev->cur.x;
           e->output.y = ev->cur.y;
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = ev->event_flags;
           e->dev = ev->device;
           e->event_src = ev->source;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
        }
        break;

      case EFL_POINTER_ACTION_OUT:
        {
           Evas_Event_Mouse_Out *e = calloc(1, sizeof(*e));
           if (ptype) *ptype = EVAS_CALLBACK_MOUSE_OUT;
           ev->legacy = e;
           e->reserved = ev->eo;

           e->buttons = ev->pressed_buttons;
           e->canvas.x = ev->cur.x;
           e->canvas.y = ev->cur.y;
           e->output.x = ev->cur.x;
           e->output.y = ev->cur.y;
           e->data = ev->data;
           e->timestamp = ev->timestamp;
           e->event_flags = ev->event_flags;
           e->dev = ev->device;
           e->event_src = ev->source;
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
        }
        break;

      case EFL_POINTER_ACTION_DOWN:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_DOWN;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
          }
        else
          {
             Evas_Event_Multi_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_DOWN;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
          }
        break;

      case EFL_POINTER_ACTION_UP:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Up *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_UP;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
          }
        else
          {
             Evas_Event_Multi_Down *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_UP;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
             break;
          }
        break;

      case EFL_POINTER_ACTION_MOVE:
        if (!ev->finger || !multi)
          {
             Evas_Event_Mouse_Move *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MOUSE_MOVE;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
          }
        else
          {
             Evas_Event_Multi_Move *e = calloc(1, sizeof(*e));
             if (ptype) *ptype = EVAS_CALLBACK_MULTI_MOVE;
             ev->legacy = e;
             e->reserved = ev->eo;

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
             e->modifiers = ev->modifiers;
             e->locks = ev->locks;
          }
        break;

      case EFL_POINTER_ACTION_WHEEL:
        {
           Evas_Event_Mouse_Wheel *e = calloc(1, sizeof(*e));
           if (ptype) *ptype = EVAS_CALLBACK_MOUSE_WHEEL;
           ev->legacy = e;
           e->reserved = ev->eo;

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
           e->modifiers = ev->modifiers;
           e->locks = ev->locks;
           break;
        }

      default:
        ERR("invalid event type %d", ev->action);
        return NULL;
     }

   return ev->legacy;
}
