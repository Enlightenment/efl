/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * OLD E hints
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

EAPI void
ecore_x_e_init(void)
{
}

EAPI void
ecore_x_e_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb)
{
   unsigned int frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_FRAME_SIZE, frames, 4);
}

EAPI void
ecore_x_e_virtual_keyboard_set(Ecore_X_Window win, unsigned int is_keyboard)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD,
				  &is_keyboard, 1);
}

EAPI int
ecore_x_e_virtual_keyboard_get(Ecore_X_Window win)
{
   unsigned int val;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD, &val, 1))
     return 0;
   return val;
}

static Ecore_X_Virtual_Keyboard_State
_ecore_x_e_vkbd_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ON) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_ON;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ALPHA) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PIN) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HEX) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_IP) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_IP;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HOST) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_HOST;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_FILE) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_FILE;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_URL) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_URL;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_KEYPAD;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_J2ME) 
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_J2ME;
   return ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_x_e_vkbd_atom_get(Ecore_X_Virtual_Keyboard_State state)
{
   switch (state)
     {
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_ON: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ON;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ALPHA;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PIN;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HEX;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_IP: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_IP;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_HOST: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HOST;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_FILE: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_FILE;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_URL: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_URL;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_KEYPAD: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_J2ME: 
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_J2ME;
      default: return 0;
     }
   return 0;
}

EAPI void
ecore_x_e_virtual_keyboard_state_set(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state)
{
   Ecore_X_Atom atom = 0;

   atom = _ecore_x_e_vkbd_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
				&atom, 1);
}

EAPI Ecore_X_Virtual_Keyboard_State
ecore_x_e_virtual_keyboard_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom;

   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
				     &atom, 1))
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;
   return _ecore_x_e_vkbd_state_get(atom);
}

EAPI void
ecore_x_e_virtual_keyboard_state_send(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state)
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 _ecore_x_e_vkbd_atom_get(state),
				 0, 0, 0, 0);
}

static Ecore_X_Atom 
_ecore_x_e_illume_atom_get(Ecore_X_Illume_Mode mode) 
{
   switch (mode) 
     {
      case ECORE_X_ILLUME_MODE_SINGLE:
        return ECORE_X_ATOM_E_ILLUME_MODE_SINGLE;
      case ECORE_X_ILLUME_MODE_DUAL:
        return ECORE_X_ATOM_E_ILLUME_MODE_DUAL;
      default:
        return 0;
     }
   return 0;
}

static Ecore_X_Illume_Mode 
_ecore_x_e_illume_mode_get(Ecore_X_Atom atom) 
{
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_SINGLE)
     return ECORE_X_ILLUME_MODE_SINGLE;
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_DUAL)
     return ECORE_X_ILLUME_MODE_DUAL;
   return ECORE_X_ILLUME_MODE_UNKNOWN;
}

EAPI void 
ecore_x_e_illume_conformant_set(Ecore_X_Window win, unsigned int is_conformant) 
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT,
				  &is_conformant, 1);
}

EAPI int 
ecore_x_e_illume_conformant_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT, 
                                       &val, 1))
     return 0;
   return val;
}

EAPI void 
ecore_x_e_illume_mode_set(Ecore_X_Window win, Ecore_X_Illume_Mode mode) 
{
   Ecore_X_Atom atom = 0;

   atom = _ecore_x_e_illume_atom_get(mode);
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_MODE,
				  &atom, 1);
}

EAPI Ecore_X_Illume_Mode 
ecore_x_e_illume_mode_get(Ecore_X_Window win) 
{
   Ecore_X_Atom atom = 0;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_MODE, &atom, 1))
     return ECORE_X_ILLUME_MODE_UNKNOWN;
   return _ecore_x_e_illume_mode_get(atom);
}

EAPI void 
ecore_x_e_illume_mode_send(Ecore_X_Window win, Ecore_X_Illume_Mode mode) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_MODE,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 _ecore_x_e_illume_atom_get(mode),
				 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_back_send(Ecore_X_Window win) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_BACK,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_close_send(Ecore_X_Window win) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_CLOSE,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
				 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_drag_set(Ecore_X_Window win, unsigned int drag) 
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG, &drag, 1);
}

EAPI int 
ecore_x_e_illume_drag_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG, &val, 1))
     return 0;
   return val;
}

EAPI void 
ecore_x_e_illume_drag_locked_set(Ecore_X_Window win, unsigned int is_locked) 
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED, 
                                  &is_locked, 1);
}

EAPI int 
ecore_x_e_illume_drag_locked_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED, 
                                       &val, 1))
     return 0;
   return val;
}

EAPI void 
ecore_x_e_illume_drag_start_send(Ecore_X_Window win) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_START,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_drag_end_send(Ecore_X_Window win) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_END,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 1, 0, 0, 0, 0);
}

static Ecore_X_Atom
_ecore_x_e_quickpanel_atom_get(Ecore_X_Illume_Quickpanel_State state)
{
   switch (state) 
     {
      case ECORE_X_ILLUME_QUICKPANEL_STATE_ON:
        return ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON;
      case ECORE_X_ILLUME_QUICKPANEL_STATE_OFF:
        return ECORE_X_ATOM_E_ILLUME_QUICKPANEL_OFF;
      default:
        return 0;
     }
   return 0;
}

static Ecore_X_Illume_Quickpanel_State
_ecore_x_e_quickpanel_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON) 
     return ECORE_X_ILLUME_QUICKPANEL_STATE_ON;
   if (atom == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_OFF)
     return ECORE_X_ILLUME_QUICKPANEL_STATE_OFF;
   return ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN;
}

EAPI void 
ecore_x_e_illume_quickpanel_set(Ecore_X_Window win, unsigned int is_quickpanel) 
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL,
				  &is_quickpanel, 1);
}

EAPI int 
ecore_x_e_illume_quickpanel_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL, 
                                       &val, 1))
     return 0;
   return val;
}

EAPI void 
ecore_x_e_illume_quickpanel_state_set(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state) 
{
   Ecore_X_Atom atom = 0;

   atom = _ecore_x_e_quickpanel_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
				&atom, 1);
}

EAPI Ecore_X_Illume_Quickpanel_State 
ecore_x_e_illume_quickpanel_state_get(Ecore_X_Window win) 
{
   Ecore_X_Atom atom;

   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
				     &atom, 1))
     return ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN;
   return _ecore_x_e_quickpanel_state_get(atom);
}

EAPI void 
ecore_x_e_illume_quickpanel_state_send(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state) 
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
				 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
				 _ecore_x_e_quickpanel_atom_get(state),
				 0, 0, 0, 0);
}
