#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_INPUT_POINTER_CLASS


/* Pointer Event
 *
 * This is a storage class only, should not require ANY knowledge about
 * Ecore, Evas or anything else. Note: locks & modifiers require evas. :(
 *
 * This is intended to replace Ecore and Evas structs for mouse events.
 *
 * Do not add any logic here.
 */

/* internal eo */
EOAPI Eo*
efl_input_pointer_instance_get(Efl_Object *owner, void **priv)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;

   evt = efl_input_event_instance_get(EFL_INPUT_POINTER_CLASS, owner);
   if (!evt) return NULL;

   ev = efl_data_scope_get(evt, EFL_INPUT_POINTER_CLASS);
   ev->fake = EINA_FALSE;
   if (priv) *priv = ev;

   return evt;
}

EOLIAN static void
_efl_input_pointer_class_destructor(Efl_Class *klass)
{
   efl_input_event_instance_clean(klass);
}

EAPI void
efl_input_pointer_finalize(Efl_Input_Pointer *obj)
{
   const Evas_Pointer_Data *pdata;
   Efl_Input_Pointer_Data *ev;
   Evas_Public_Data *evas;
   Evas *eo_evas;

   ev = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(ev);

   eo_evas = efl_provider_find(obj, EVAS_CANVAS_CLASS);
   evas = efl_data_scope_get(eo_evas, EVAS_CANVAS_CLASS);
   if (!evas) return;

   /* FIXME: modifiers & locks should be seat-based! */
   ev->modifiers = &evas->modifiers;
   ev->locks = &evas->locks;

   pdata = _evas_pointer_data_by_device_get(evas, ev->device);
   if (!pdata) return;

   ev->prev.x = pdata->seat->x;
   ev->prev.y = pdata->seat->y;
   ev->pressed_buttons = pdata->button;
}

EOLIAN static Efl_Object *
_efl_input_pointer_efl_object_constructor(Eo *obj, Efl_Input_Pointer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->fake = 1;
   efl_input_reset(obj);
   return obj;
}

static inline void
_efl_input_pointer_free(Efl_Input_Pointer_Data *pd)
{
   free(pd->legacy);
   efl_unref(pd->device);
}

EOLIAN static void
_efl_input_pointer_efl_object_destructor(Eo *obj, Efl_Input_Pointer_Data *pd)
{
   _efl_input_pointer_free(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_input_pointer_efl_input_event_reset(Eo *obj, Efl_Input_Pointer_Data *pd)
{
   Eina_Bool fake = pd->fake;
   _efl_input_pointer_free(pd);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
   pd->fake = fake;
}

EOLIAN static Efl_Input_Event *
_efl_input_pointer_efl_duplicate_duplicate(const Eo *obj, Efl_Input_Pointer_Data *pd)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Focus *evt;

   if (efl_invalidated_get(obj))
     {
        ERR("Object %s has already been invalidated and can't be duplicated.", efl_debug_name_get(obj));
        return NULL;
     }
   if (!efl_parent_get(obj))
     {
        ERR("Object %s has not parent during duplicate.", efl_debug_name_get(obj));
        return NULL;
     }
   evt = efl_add(MY_CLASS, efl_parent_get(obj),
                 efl_allow_parent_unref_set(efl_added, EINA_TRUE));
   ev = efl_data_scope_get(evt, MY_CLASS);
   if (!ev) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo = evt;
   ev->legacy = NULL;
   ev->evas_done = 0;
   ev->win_fed = 0;
   ev->fake = 1;
   ev->device = efl_ref(pd->device);

   return evt;
}

EOLIAN static void
_efl_input_pointer_action_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Pointer_Action act)
{
   pd->action = act;
}

EOLIAN static Efl_Pointer_Action
_efl_input_pointer_action_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->action;
}

EOLIAN static void
_efl_input_pointer_button_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, int but)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_BUTTON);
   pd->button = but;
}

EOLIAN static int
_efl_input_pointer_button_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->button;
}

EOLIAN static void
_efl_input_pointer_button_pressed_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, int button, Eina_Bool pressed)
{
   if (button < 0) return;
   if (button > 31) return;
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_BUTTONS_PRESSED);
   if (pressed)
     pd->button_flags |= (1 << button);
   else
     pd->button_flags &= ~(1 << button);
}

EOLIAN static Eina_Bool
_efl_input_pointer_button_pressed_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, int button)
{
   if (button < 0) return EINA_FALSE;
   if (button > 31) return EINA_FALSE;
   return (pd->button_flags & (1 << button)) != 0;
}

EOLIAN static void
_efl_input_pointer_position_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Position2D pos)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_X);
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_Y);
   pd->cur.x = (double) pos.x;
   pd->cur.y = (double) pos.y;
}

EOLIAN static Eina_Position2D
_efl_input_pointer_position_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return EINA_POSITION2D((int) pd->cur.x, (int) pd->cur.y);
}

EOLIAN static void
_efl_input_pointer_precise_position_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Vector2 pos)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_X);
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_Y);
   pd->cur = pos;
}

EOLIAN static Eina_Vector2
_efl_input_pointer_precise_position_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->cur;
}

EOLIAN static void
_efl_input_pointer_previous_position_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Position2D pos)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_PREVIOUS_X);
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_PREVIOUS_Y);
   pd->prev.x = (double) pos.x;
   pd->prev.y = (double) pos.y;
}

EOLIAN static Eina_Position2D
_efl_input_pointer_previous_position_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return EINA_POSITION2D((int) pd->prev.x, (int) pd->prev.y);
}

EOLIAN static void
_efl_input_pointer_efl_input_event_device_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Input_Device *dev)
{
   efl_replace(&pd->device, dev);
}

EOLIAN static Efl_Input_Device *
_efl_input_pointer_efl_input_event_device_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->device;
}

EOLIAN static void
_efl_input_pointer_source_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Gfx_Entity *src)
{
   /* ref? */
   pd->source = src;
}

EOLIAN static Efl_Gfx_Entity *
_efl_input_pointer_source_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_input_pointer_button_flags_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Pointer_Flags flags)
{
   pd->button_flags = flags;
}

EOLIAN static Efl_Pointer_Flags
_efl_input_pointer_button_flags_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->button_flags;
}

EOLIAN static void
_efl_input_pointer_efl_input_event_event_flags_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Input_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Input_Flags
_efl_input_pointer_efl_input_event_event_flags_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static void
_efl_input_pointer_efl_input_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, double ms)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_TIMESTAMP);
   pd->timestamp = (unsigned int) ms;
}

EOLIAN static double
_efl_input_pointer_efl_input_event_timestamp_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return (double) pd->timestamp;
}

EOLIAN static void
_efl_input_pointer_wheel_horizontal_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Bool dir)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_WHEEL_HORIZONTAL);
   pd->wheel.horizontal = !!dir;
}

EOLIAN static Eina_Bool
_efl_input_pointer_wheel_horizontal_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->wheel.horizontal;
}

EOLIAN static void
_efl_input_pointer_wheel_delta_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, int dist)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_WHEEL_DELTA);
   pd->wheel.z = dist;
}

EOLIAN static int
_efl_input_pointer_wheel_delta_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->wheel.z;
}

EOLIAN static int
_efl_input_pointer_tool_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return pd->tool;
}

EOLIAN static void
_efl_input_pointer_tool_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, int id)
{
   _efl_input_value_mark(pd, EFL_INPUT_VALUE_TOOL);
   pd->tool = id;
}

EOLIAN static Eina_Bool
_efl_input_pointer_efl_input_state_modifier_enabled_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd,
                                                        Efl_Input_Modifier mod, const Efl_Input_Device *seat)
{
   const char *name;

   if (!pd->modifiers) return EINA_FALSE;
   if (!seat)
     {
        seat = efl_input_device_seat_get(pd->device);
        if (!seat) return EINA_FALSE;
     }
   name = _efl_input_modifier_to_string(mod);
   if (!name) return EINA_FALSE;
   return evas_seat_key_modifier_is_set(pd->modifiers, name, seat);
}

EOLIAN static Eina_Bool
_efl_input_pointer_efl_input_state_lock_enabled_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd,
                                                    Efl_Input_Lock lock, const Efl_Input_Device *seat)
{
   const char *name;

   if (!pd->locks) return EINA_FALSE;
   if (!seat)
     {
        seat = efl_input_device_seat_get(pd->device);
        if (!seat) return EINA_FALSE;
     }
   name = _efl_input_lock_to_string(lock);
   if (!name) return EINA_FALSE;
   return evas_seat_key_lock_is_set(pd->locks, name, seat);
}

EOLIAN static void
_efl_input_pointer_double_click_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->button_flags |= EFL_POINTER_FLAGS_DOUBLE_CLICK;
   else
     pd->button_flags &= ~EFL_POINTER_FLAGS_DOUBLE_CLICK;
}

EOLIAN static Eina_Bool
_efl_input_pointer_double_click_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return !!(pd->button_flags & EFL_POINTER_FLAGS_DOUBLE_CLICK);
}

EOLIAN static void
_efl_input_pointer_triple_click_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->button_flags |= EFL_POINTER_FLAGS_TRIPLE_CLICK;
   else
     pd->button_flags &= ~EFL_POINTER_FLAGS_TRIPLE_CLICK;
}

EOLIAN static Eina_Bool
_efl_input_pointer_triple_click_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   return !!(pd->button_flags & EFL_POINTER_FLAGS_TRIPLE_CLICK);
}

EOLIAN static Eina_Bool
_efl_input_pointer_efl_input_event_fake_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd)
{
   // read-only
   return pd->fake;
}

EOLIAN static Eina_Bool
_efl_input_pointer_value_has_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Input_Value key)
{
   if (!pd || (key <= EFL_INPUT_VALUE_NONE) || (key > EFL_INPUT_VALUE_SLIDER))
     return EINA_FALSE;
   if (key == EFL_INPUT_VALUE_DX)
     {
        return _efl_input_value_has(pd, EFL_INPUT_VALUE_X) &&
              _efl_input_value_has(pd, EFL_INPUT_VALUE_PREVIOUS_X);
     }
   if (key == EFL_INPUT_VALUE_DY)
     {
        return _efl_input_value_has(pd, EFL_INPUT_VALUE_Y) &&
              _efl_input_value_has(pd, EFL_INPUT_VALUE_PREVIOUS_Y);
     }
   return _efl_input_value_has(pd, key);
}

EOLIAN static Eina_Bool
_efl_input_pointer_value_set(Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Input_Value key, double val)
{
   if ((key <= EFL_INPUT_VALUE_NONE) || (key > EFL_INPUT_VALUE_SLIDER))
     return EINA_FALSE;

   // note: not a fan of setting ints based on a double...
   switch (key)
     {
      case EFL_INPUT_VALUE_TIMESTAMP:
        pd->timestamp = (unsigned int) (val * 1000.0);
        break;

      case EFL_INPUT_VALUE_BUTTON:
        pd->button = (int) val;
        break;

      case EFL_INPUT_VALUE_BUTTONS_PRESSED:
        pd->pressed_buttons = (int) val;
        break;

      case EFL_INPUT_VALUE_TOOL:
        pd->tool = (int) val;
        break;

      case EFL_INPUT_VALUE_X:
        pd->cur.x = val;
        break;

      case EFL_INPUT_VALUE_Y:
        pd->cur.y = val;
        break;

      case EFL_INPUT_VALUE_DX:
      case EFL_INPUT_VALUE_DY:
        return EINA_FALSE;

      case EFL_INPUT_VALUE_PREVIOUS_X:
        pd->prev.x = val;
        break;

      case EFL_INPUT_VALUE_PREVIOUS_Y:
        pd->prev.y = val;
        break;

      case EFL_INPUT_VALUE_RADIUS:
        pd->radius = val;
        break;

      case EFL_INPUT_VALUE_RADIUS_X:
        pd->radius_x = val;
        break;

      case EFL_INPUT_VALUE_RADIUS_Y:
        pd->radius_y = val;
        break;

      case EFL_INPUT_VALUE_PRESSURE:
        return pd->pressure;

      case EFL_INPUT_VALUE_DISTANCE:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_AZIMUTH:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_TILT:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_TILT_X:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_TILT_Y:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_TWIST:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_WHEEL_DELTA:
        pd->wheel.z = (int) val;
        break;

      case EFL_INPUT_VALUE_WHEEL_ANGLE:
        return EINA_FALSE; // TODO

      case EFL_INPUT_VALUE_WHEEL_HORIZONTAL:
        pd->wheel.horizontal = (((int) val) == 1);
        break;

      case EFL_INPUT_VALUE_SLIDER:
        return EINA_FALSE; // TODO

      default:
        return EINA_FALSE;
     }

   _efl_input_value_mark(pd, key);
   return EINA_TRUE;
}

EOLIAN static double
_efl_input_pointer_value_get(const Eo *obj EINA_UNUSED, Efl_Input_Pointer_Data *pd, Efl_Input_Value key)
{
   switch (key)
     {
      case EFL_INPUT_VALUE_TIMESTAMP:
        return (double) pd->timestamp / 1000.0;

      case EFL_INPUT_VALUE_BUTTON:
        return (double) pd->button;

      case EFL_INPUT_VALUE_BUTTONS_PRESSED:
        return (double) pd->pressed_buttons;

      case EFL_INPUT_VALUE_TOOL:
        return (double) pd->tool;

      case EFL_INPUT_VALUE_X:
        return pd->cur.x;

      case EFL_INPUT_VALUE_Y:
        return pd->cur.y;

      case EFL_INPUT_VALUE_DX:
        return pd->cur.x - pd->prev.x;

      case EFL_INPUT_VALUE_DY:
        return pd->cur.y - pd->prev.y;

      case EFL_INPUT_VALUE_PREVIOUS_X:
        return pd->prev.x;

      case EFL_INPUT_VALUE_PREVIOUS_Y:
        return pd->prev.y;

      case EFL_INPUT_VALUE_RAW_X:
        if (!_efl_input_value_has(pd, EFL_INPUT_VALUE_RAW_X))
          return pd->cur.x;
        return pd->raw.x;

      case EFL_INPUT_VALUE_RAW_Y:
        if (!_efl_input_value_has(pd, EFL_INPUT_VALUE_RAW_Y))
          return pd->cur.y;
        return pd->raw.y;

      case EFL_INPUT_VALUE_RADIUS:
        return pd->radius;

      case EFL_INPUT_VALUE_RADIUS_X:
        return pd->radius_x;

      case EFL_INPUT_VALUE_RADIUS_Y:
        return pd->radius_y;

      case EFL_INPUT_VALUE_PRESSURE:
        return pd->pressure;

      case EFL_INPUT_VALUE_DISTANCE:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_AZIMUTH:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_TILT:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_TILT_X:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_TILT_Y:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_TWIST:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_WHEEL_DELTA:
        return (double) pd->wheel.z;

      case EFL_INPUT_VALUE_WHEEL_ANGLE:
        return 0.0; // TODO (emulate??)

      case EFL_INPUT_VALUE_WHEEL_HORIZONTAL:
        return (double) pd->wheel.horizontal;

      case EFL_INPUT_VALUE_SLIDER:
        return 0.0; // TODO

      case EFL_INPUT_VALUE_NONE:
      default:
        return 0;
     }
}

EOLIAN static void *
_efl_input_pointer_efl_input_event_legacy_info_get(Eo *obj, Efl_Input_Pointer_Data *pd)
{
   if (pd->legacy) return pd->legacy;
   return efl_input_pointer_legacy_info_fill(NULL, obj, EVAS_CALLBACK_LAST, NULL);
}

/* Internal EO APIs */

#define EFL_INPUT_POINTER_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_input_legacy_info_get, _efl_input_pointer_efl_input_event_legacy_info_get)

#include "efl_input_pointer.eo.c"
