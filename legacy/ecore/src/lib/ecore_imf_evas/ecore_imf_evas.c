/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_IMF_Evas.h"

/**
 * @defgroup Ecore_IMF_Evas_Group Ecore Input Method Context Evas Helper Functions
 *
 * Helper functions to make it easy to use Evas with Ecore_IMF.
 */

static char *_ecore_imf_evas_event_empty = "";

/* Converts the Evas modifiers to Ecore_IMF keyboard modifiers */
static void
_ecore_imf_evas_event_modifiers_wrap(Evas_Modifier *evas_modifiers,
				     Ecore_IMF_Keyboard_Modifiers *imf_keyboard_modifiers)
{
   if (!evas_modifiers || !imf_keyboard_modifiers)
     return;

   *imf_keyboard_modifiers = ECORE_IMF_KEYBOARD_MODIFIER_NONE;
   if (evas_key_modifier_is_set(evas_modifiers, "Control"))
     *imf_keyboard_modifiers |= ECORE_IMF_KEYBOARD_MODIFIER_CTRL;
   if (evas_key_modifier_is_set(evas_modifiers, "Alt"))
     *imf_keyboard_modifiers |= ECORE_IMF_KEYBOARD_MODIFIER_ALT;
   if (evas_key_modifier_is_set(evas_modifiers, "Shift"))
     *imf_keyboard_modifiers |= ECORE_IMF_KEYBOARD_MODIFIER_SHIFT;
   if (evas_key_modifier_is_set(evas_modifiers, "Super") || evas_key_modifier_is_set(evas_modifiers, "Hyper"))
     *imf_keyboard_modifiers |= ECORE_IMF_KEYBOARD_MODIFIER_WIN;
}

/* Converts the Evas locks to Ecore_IMF keyboard locks */
static void
_ecore_imf_evas_event_locks_wrap(Evas_Lock *evas_locks,
				 Ecore_IMF_Keyboard_Locks *imf_keyboard_locks)
{
   if (!evas_locks || !imf_keyboard_locks)
     return;

   *imf_keyboard_locks = ECORE_IMF_KEYBOARD_LOCK_NONE;
   if (evas_key_lock_is_set(evas_locks, "Num_Lock"))
     *imf_keyboard_locks |= ECORE_IMF_KEYBOARD_LOCK_NUM;
   if (evas_key_lock_is_set(evas_locks, "Caps_Lock"))
     *imf_keyboard_locks |= ECORE_IMF_KEYBOARD_LOCK_CAPS;
   if (evas_key_lock_is_set(evas_locks, "Scroll_Lock"))
     *imf_keyboard_locks |= ECORE_IMF_KEYBOARD_LOCK_SCROLL;
}

/* Converts the Evas mouse flags to Ecore_IMF mouse flags */
static void
_ecore_imf_evas_event_mouse_flags_wrap(Evas_Button_Flags evas_flags,
				       Ecore_IMF_Mouse_Flags *imf_flags)
{
   if (!imf_flags)
     return;

   *imf_flags = ECORE_IMF_MOUSE_NONE;
   if (evas_flags & EVAS_BUTTON_DOUBLE_CLICK)
     *imf_flags |= ECORE_IMF_MOUSE_DOUBLE_CLICK;
   if (evas_flags & EVAS_BUTTON_TRIPLE_CLICK)
     *imf_flags |= ECORE_IMF_MOUSE_TRIPLE_CLICK;
}

/**
 * Converts a "mouse_in" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_in_wrap(Evas_Event_Mouse_In *evas_event,
				   Ecore_IMF_Event_Mouse_In *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->buttons = evas_event->buttons;
   imf_event->output.x = evas_event->output.x;
   imf_event->output.y = evas_event->output.y;
   imf_event->canvas.x = evas_event->canvas.x;
   imf_event->canvas.y = evas_event->canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
}

/**
 * Converts a "mouse_out" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_out_wrap(Evas_Event_Mouse_Out *evas_event,
				    Ecore_IMF_Event_Mouse_Out *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->buttons = evas_event->buttons;
   imf_event->output.x = evas_event->output.x;
   imf_event->output.y = evas_event->output.y;
   imf_event->canvas.x = evas_event->canvas.x;
   imf_event->canvas.y = evas_event->canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
}

/**
 * Converts a "mouse_move" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_move_wrap(Evas_Event_Mouse_Move *evas_event,
				     Ecore_IMF_Event_Mouse_Move *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->buttons = evas_event->buttons;
   imf_event->cur.output.x = evas_event->cur.output.x;
   imf_event->cur.output.y = evas_event->cur.output.y;
   imf_event->prev.output.x = evas_event->prev.output.x;
   imf_event->prev.output.y = evas_event->prev.output.y;
   imf_event->cur.canvas.x = evas_event->cur.canvas.x;
   imf_event->cur.canvas.y = evas_event->cur.canvas.y;
   imf_event->prev.canvas.x = evas_event->prev.canvas.x;
   imf_event->prev.canvas.y = evas_event->prev.canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
}

/**
 * Converts a "mouse_down" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_down_wrap(Evas_Event_Mouse_Down *evas_event,
				     Ecore_IMF_Event_Mouse_Down *imf_event)
{
   if (!evas_event || !imf_event)
      return;

   imf_event->button = evas_event->button;
   imf_event->output.x = evas_event->output.x;
   imf_event->output.y = evas_event->output.y;
   imf_event->canvas.x = evas_event->canvas.x;
   imf_event->canvas.y = evas_event->canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
   _ecore_imf_evas_event_mouse_flags_wrap(evas_event->flags, &imf_event->flags);
}

/**
 * Converts a "mouse_up" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_up_wrap(Evas_Event_Mouse_Up *evas_event,
				   Ecore_IMF_Event_Mouse_Up *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->button = evas_event->button;
   imf_event->output.x = evas_event->output.x;
   imf_event->output.y = evas_event->output.y;
   imf_event->canvas.x = evas_event->canvas.x;
   imf_event->canvas.y = evas_event->canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
   _ecore_imf_evas_event_mouse_flags_wrap(evas_event->flags, &imf_event->flags);
}

/**
 * Converts a "mouse_wheel" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_mouse_wheel_wrap(Evas_Event_Mouse_Wheel *evas_event,
				      Ecore_IMF_Event_Mouse_Wheel *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->direction = evas_event->direction;
   imf_event->z = evas_event->z;
   imf_event->output.x = evas_event->output.x;
   imf_event->output.y = evas_event->output.y;
   imf_event->canvas.x = evas_event->canvas.x;
   imf_event->canvas.y = evas_event->canvas.y;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
   imf_event->timestamp = evas_event->timestamp;
}

/**
 * Converts a "key_down" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_key_down_wrap(Evas_Event_Key_Down *evas_event,
				   Ecore_IMF_Event_Key_Down *imf_event)
{
   if (!evas_event || !imf_event)
     return;

   imf_event->keyname = evas_event->keyname ? evas_event->keyname : _ecore_imf_evas_event_empty;
   imf_event->key = evas_event->key ? evas_event->key : _ecore_imf_evas_event_empty;
   imf_event->string = evas_event->string ? evas_event->string : _ecore_imf_evas_event_empty;
   imf_event->compose = evas_event->compose ? evas_event->compose : _ecore_imf_evas_event_empty;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
}

/**
 * Converts a "key_up" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void
ecore_imf_evas_event_key_up_wrap(Evas_Event_Key_Up *evas_event,
				 Ecore_IMF_Event_Key_Up *imf_event)
{
   imf_event->keyname = evas_event->keyname ? evas_event->keyname : _ecore_imf_evas_event_empty;
   imf_event->key = evas_event->key ? evas_event->key : _ecore_imf_evas_event_empty;
   imf_event->string = evas_event->string ? evas_event->string : _ecore_imf_evas_event_empty;
   imf_event->compose = evas_event->compose ? evas_event->compose : _ecore_imf_evas_event_empty;
   imf_event->timestamp = evas_event->timestamp;
   _ecore_imf_evas_event_modifiers_wrap(evas_event->modifiers, &imf_event->modifiers);
   _ecore_imf_evas_event_locks_wrap(evas_event->locks, &imf_event->locks);
}
