/*
 * OLD E hints
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
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
ecore_x_e_frame_size_set(Ecore_X_Window win,
                         int fl,
                         int fr,
                         int ft,
                         int fb)
{
   unsigned int frames[4];

   LOGFN;
   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_FRAME_SIZE, frames, 4);
}

EAPI void
ecore_x_e_virtual_keyboard_set(Ecore_X_Window win,
                               unsigned int is_keyboard)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD,
                                  &is_keyboard, 1);
}

EAPI Eina_Bool
ecore_x_e_virtual_keyboard_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
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

      default: break;
     }
   return 0;
}

EAPI void
ecore_x_e_virtual_keyboard_state_set(Ecore_X_Window win,
                                     Ecore_X_Virtual_Keyboard_State state)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_vkbd_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
                                &atom, 1);
}

EAPI Ecore_X_Virtual_Keyboard_State
ecore_x_e_virtual_keyboard_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE,
                                     &atom, 1))
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;

   return _ecore_x_e_vkbd_state_get(atom);
}

EAPI void
ecore_x_e_virtual_keyboard_state_send(Ecore_X_Window win,
                                      Ecore_X_Virtual_Keyboard_State state)
{
   LOGFN;
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

      case ECORE_X_ILLUME_MODE_DUAL_TOP:
        return ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP;

      case ECORE_X_ILLUME_MODE_DUAL_LEFT:
        return ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT;

      default:
        break;
     }
   return ECORE_X_ILLUME_MODE_UNKNOWN;
}

static Ecore_X_Illume_Mode
_ecore_x_e_illume_mode_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_SINGLE)
     return ECORE_X_ILLUME_MODE_SINGLE;

   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP)
     return ECORE_X_ILLUME_MODE_DUAL_TOP;

   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT)
     return ECORE_X_ILLUME_MODE_DUAL_LEFT;

   return ECORE_X_ILLUME_MODE_UNKNOWN;
}

EAPI void
ecore_x_e_illume_zone_set(Ecore_X_Window win,
                          Ecore_X_Window zone)
{
   LOGFN;
   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_E_ILLUME_ZONE,
                                  &zone, 1);
}

EAPI Ecore_X_Window
ecore_x_e_illume_zone_get(Ecore_X_Window win)
{
   Ecore_X_Window zone = 0;
   int ret;

   LOGFN;
   ret = ecore_x_window_prop_window_get(win, ECORE_X_ATOM_E_ILLUME_ZONE,
                                        &zone, 1);
   if ((ret == 0) || (ret == -1))
     return 0;

   return zone;
}

EAPI void
ecore_x_e_illume_zone_list_set(Ecore_X_Window win,
                               Ecore_X_Window *zones,
                               unsigned int n_zones)
{
   LOGFN;
   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_E_ILLUME_ZONE_LIST,
                                  zones, n_zones);
}

EAPI void
ecore_x_e_illume_conformant_set(Ecore_X_Window win,
                                unsigned int is_conformant)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT,
                                  &is_conformant, 1);
}

EAPI Eina_Bool
ecore_x_e_illume_conformant_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_illume_mode_set(Ecore_X_Window win,
                          Ecore_X_Illume_Mode mode)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_illume_atom_get(mode);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_MODE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Mode
ecore_x_e_illume_mode_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_ILLUME_MODE, &atom, 1))
     return ECORE_X_ILLUME_MODE_UNKNOWN;

   return _ecore_x_e_illume_mode_get(atom);
}

EAPI void
ecore_x_e_illume_mode_send(Ecore_X_Window win,
                           Ecore_X_Illume_Mode mode)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_MODE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_illume_atom_get(mode),
                                 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_focus_back_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_BACK,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_focus_forward_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_FORWARD,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_focus_home_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_HOME,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_close_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_CLOSE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_home_new_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_HOME_NEW,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_home_del_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_HOME_DEL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_next_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_NEXT,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_prev_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_PREV,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_activate_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_over_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_OVER,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_read_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_read_next_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_read_prev_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_up_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_UP,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_access_action_down_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 win,
                                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_DOWN,
                                 0, 0, 0);
}

EAPI void
ecore_x_e_illume_drag_set(Ecore_X_Window win,
                          unsigned int drag)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG, &drag, 1);
}

EAPI Eina_Bool
ecore_x_e_illume_drag_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_illume_drag_locked_set(Ecore_X_Window win,
                                 unsigned int is_locked)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED,
                                  &is_locked, 1);
}

EAPI Eina_Bool
ecore_x_e_illume_drag_locked_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_illume_drag_start_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_START,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_drag_end_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_END,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_indicator_geometry_set(Ecore_X_Window win,
                                        int x,
                                        int y,
                                        int w,
                                        int h)
{
   unsigned int geom[4];

   LOGFN;
   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY,
                                  geom, 4);
}

EAPI Eina_Bool
ecore_x_e_illume_indicator_geometry_get(Ecore_X_Window win,
                                        int *x,
                                        int *y,
                                        int *w,
                                        int *h)
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN;
   ret =
     ecore_x_window_prop_card32_get(win,
                                    ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY,
                                    geom, 4);
   if (ret != 4)
     return EINA_FALSE;

   if (x)
     *x = geom[0];

   if (y)
     *y = geom[1];

   if (w)
     *w = geom[2];

   if (h)
     *h = geom[3];

   return EINA_TRUE;
}

EAPI void
ecore_x_e_illume_softkey_geometry_set(Ecore_X_Window win,
                                      int x,
                                      int y,
                                      int w,
                                      int h)
{
   unsigned int geom[4];

   LOGFN;
   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY,
                                  geom, 4);
}

EAPI Eina_Bool
ecore_x_e_illume_softkey_geometry_get(Ecore_X_Window win,
                                      int *x,
                                      int *y,
                                      int *w,
                                      int *h)
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN;
   ret =
     ecore_x_window_prop_card32_get(win,
                                    ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY,
                                    geom, 4);
   if (ret != 4)
     return EINA_FALSE;

   if (x)
     *x = geom[0];

   if (y)
     *y = geom[1];

   if (w)
     *w = geom[2];

   if (h)
     *h = geom[3];

   return EINA_TRUE;
}

EAPI void
ecore_x_e_illume_keyboard_geometry_set(Ecore_X_Window win,
                                       int x,
                                       int y,
                                       int w,
                                       int h)
{
   unsigned int geom[4];

   LOGFN;
   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY,
                                  geom, 4);
}

EAPI Eina_Bool
ecore_x_e_illume_keyboard_geometry_get(Ecore_X_Window win,
                                       int *x,
                                       int *y,
                                       int *w,
                                       int *h)
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN;
   ret =
     ecore_x_window_prop_card32_get(win,
                                    ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY,
                                    geom, 4);
   if (ret != 4)
     return EINA_FALSE;

   if (x)
     *x = geom[0];

   if (y)
     *y = geom[1];

   if (w)
     *w = geom[2];

   if (h)
     *h = geom[3];

   return EINA_TRUE;
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
        break;
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
ecore_x_e_illume_quickpanel_set(Ecore_X_Window win,
                                unsigned int is_quickpanel)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL,
                                  &is_quickpanel, 1);
}

EAPI Eina_Bool
ecore_x_e_illume_quickpanel_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_illume_quickpanel_state_set(Ecore_X_Window win,
                                      Ecore_X_Illume_Quickpanel_State state)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_quickpanel_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Quickpanel_State
ecore_x_e_illume_quickpanel_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
                                     &atom, 1))
     return ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN;

   return _ecore_x_e_quickpanel_state_get(atom);
}

EAPI void
ecore_x_e_illume_quickpanel_state_send(Ecore_X_Window win,
                                       Ecore_X_Illume_Quickpanel_State state)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_quickpanel_atom_get(state),
                                 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_quickpanel_state_toggle(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win,
                                 ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 0, 0, 0, 0, 0);
}

EAPI void
ecore_x_e_illume_quickpanel_priority_major_set(Ecore_X_Window win,
                                               unsigned int priority)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR,
                                  &priority, 1);
}

EAPI int
ecore_x_e_illume_quickpanel_priority_major_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR,
                                       &val, 1);
   if ((ret == 0) || (ret == -1))
     return EINA_FALSE;

   return val;
}

EAPI void
ecore_x_e_illume_quickpanel_priority_minor_set(Ecore_X_Window win,
                                               unsigned int priority)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR,
                                  &priority, 1);
}

EAPI int
ecore_x_e_illume_quickpanel_priority_minor_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win,
                                        ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return 0;

   return val;
}

EAPI void
ecore_x_e_illume_quickpanel_zone_set(Ecore_X_Window win,
                                     unsigned int zone)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ZONE,
                                  &zone, 1);
}

EAPI int
ecore_x_e_illume_quickpanel_zone_get(Ecore_X_Window win)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win,
                                        ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ZONE,
                                        &val, 1);
   if ((ret == 0) || (ret == -1))
     return 0;

   return val;
}

EAPI void
ecore_x_e_illume_quickpanel_position_update_send(Ecore_X_Window win)
{
   LOGFN;
   ecore_x_client_message32_send(win,
                                 ECORE_X_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 1, 0, 0, 0, 0);
}

static Ecore_X_Atom
_ecore_x_e_clipboard_atom_get(Ecore_X_Illume_Clipboard_State state)
{
   switch (state)
     {
      case ECORE_X_ILLUME_CLIPBOARD_STATE_ON:
        return ECORE_X_ATOM_E_ILLUME_CLIPBOARD_ON;
      case ECORE_X_ILLUME_CLIPBOARD_STATE_OFF:
        return ECORE_X_ATOM_E_ILLUME_CLIPBOARD_OFF;
      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Clipboard_State
_ecore_x_e_clipboard_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_ON)
     return ECORE_X_ILLUME_CLIPBOARD_STATE_ON;

   if (atom == ECORE_X_ATOM_E_ILLUME_CLIPBOARD_OFF)
     return ECORE_X_ILLUME_CLIPBOARD_STATE_OFF;

   return ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN;
}

EAPI void
ecore_x_e_illume_clipboard_state_set(Ecore_X_Window win,
                                     Ecore_X_Illume_Clipboard_State state)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_clipboard_atom_get(state);

   ecore_x_window_prop_atom_set(win,
                                ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Clipboard_State
ecore_x_e_illume_clipboard_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;

   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_CLIPBOARD_STATE,
                                     &atom, 1))
     return ECORE_X_ILLUME_CLIPBOARD_STATE_UNKNOWN;
   return _ecore_x_e_clipboard_state_get(atom);
}

EAPI void
ecore_x_e_illume_clipboard_geometry_set(Ecore_X_Window win,
                                        int x, int y, int w, int h)
{
   unsigned int geom[4];

   LOGFN;
   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY,
                                  geom, 4);
}

EAPI Eina_Bool
ecore_x_e_illume_clipboard_geometry_get(Ecore_X_Window win,
                                        int *x, int *y, int *w, int *h)
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN;
   ret =
      ecore_x_window_prop_card32_get(win,
                                     ECORE_X_ATOM_E_ILLUME_CLIPBOARD_GEOMETRY,
                                     geom, 4);
   if (ret != 4) return EINA_FALSE;

   if (x) *x = geom[0];
   if (y) *y = geom[1];
   if (w) *w = geom[2];
   if (h) *h = geom[3];

   return EINA_TRUE;
}

/* for sliding window */
EAPI void
ecore_x_e_illume_sliding_win_state_set(Ecore_X_Window win,
                                       unsigned int   is_visible)
{
   LOGFN;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_SLIDING_WIN_STATE,
                                  &is_visible, 1);
} /* ecore_x_e_illume_sliding_win_state_set */

EAPI int
ecore_x_e_illume_sliding_win_state_get(Ecore_X_Window win)
{
   unsigned int is_visible = 0;
   int ret = 0;

   LOGFN;
   ret = ecore_x_window_prop_card32_get(win,
                                        ECORE_X_ATOM_E_ILLUME_SLIDING_WIN_STATE,
                                        &is_visible, 1);
   if ((ret == 0) || (ret == -1))
     return 0;

   return is_visible;
}

EAPI void
ecore_x_e_illume_sliding_win_geometry_set(Ecore_X_Window win,
                                          int            x,
                                          int            y,
                                          int            w,
                                          int            h)
{
   unsigned int geom[4];

   LOGFN;
   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_ILLUME_SLIDING_WIN_GEOMETRY,
                                  geom, 4);
} /* ecore_x_e_illume_sliding_win_geometry_set */

EAPI int
ecore_x_e_illume_sliding_win_geometry_get(Ecore_X_Window win,
                                          int           *x,
                                          int           *y,
                                          int           *w,
                                          int           *h)
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN;
   ret =
     ecore_x_window_prop_card32_get(win,
                                    ECORE_X_ATOM_E_ILLUME_SLIDING_WIN_GEOMETRY,
                                    geom, 4);
   if (ret != 4)
      return 0;

   if (x)
      *x = geom[0];

   if (y)
      *y = geom[1];

   if (w)
      *w = geom[2];

   if (h)
      *h = geom[3];

   return 1;
}/* ecore_x_e_illume_sliding_win_geometry_get */

EAPI void
ecore_x_e_comp_sync_counter_set(Ecore_X_Window win,
                                Ecore_X_Sync_Counter counter)
{
   LOGFN;
   if (counter)
     ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_SYNC_COUNTER,
                                 ECORE_X_ATOM_CARDINAL, &counter, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_COMP_SYNC_COUNTER);
}

EAPI Ecore_X_Sync_Counter
ecore_x_e_comp_sync_counter_get(Ecore_X_Window win)
{
   int ret = 0;
   Ecore_X_Sync_Counter counter = 0;

   LOGFN;
   ret =
     ecore_x_window_prop_xid_get(win,
                                 ECORE_X_ATOM_E_COMP_SYNC_COUNTER,
                                 ECORE_X_ATOM_CARDINAL,
                                 &counter, 1);
   if (ret != 1)
     return 0;

   return counter;
}

EAPI void
ecore_x_e_comp_sync_draw_done_send(Ecore_X_Window root,
                                   Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // version
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_sync_draw_size_done_send(Ecore_X_Window root,
                                        Ecore_X_Window win,
                                        int w,
                                        int h)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 1; // version
   xev.xclient.data.l[2] = w; // win width at draw time
   xev.xclient.data.l[3] = h; // win height at draw time
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

/*
 * @since 1.3
 * @deprecated use ecore_x_e_window_available_profiles_set
 */
EAPI void
ecore_x_e_window_profile_list_set(Ecore_X_Window  win,
                                  const char    **profiles,
                                  unsigned int    num_profiles)
{
   Ecore_X_Atom *atoms;

   LOGFN;
   if (!win)
     return;

   if ((!profiles) || (num_profiles == 0))
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST);
   else
     {
        atoms = alloca(num_profiles * sizeof(Ecore_X_Atom));
        ecore_x_atoms_get(profiles, num_profiles, atoms);
        ecore_x_window_prop_property_set(win,
                                         ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST,
                                         XA_ATOM, 32, (void *)atoms,
                                         num_profiles);
     }
}

/*
 * @since 1.3
 * @deprecated use ecore_x_e_window_available_profiles_get
 */
EAPI Eina_Bool
ecore_x_e_window_profile_list_get(Ecore_X_Window   win,
                                  const char    ***profiles,
                                  int             *ret_num)
{
   unsigned char *data;
   Ecore_X_Atom *atoms;
   int num, i;

   LOGFN;

   if (ret_num)
     *ret_num = 0;

   if (profiles)
     *profiles = NULL;

   if (!win)
     return EINA_FALSE;

   if (!ecore_x_window_prop_property_get(win,
                                         ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST,
                                         XA_ATOM, 32, &data, &num))
     {
        if (data) free(data);
        return EINA_FALSE;
     }

   if (ret_num)
     *ret_num = num;

   if (profiles)
     {
        (*profiles) = calloc(num, sizeof(char *));
        if (!(*profiles))
          {
             if (ret_num)
               *ret_num = 0;

             if (data)
               free(data);

             return EINA_FALSE;
          }

        atoms = (Ecore_X_Atom *)data;
        for (i = 0; i < num; i++)
           (*profiles)[i] = ecore_x_atom_name_get(atoms[i]);
     }

   if (data) free(data);

   return EINA_TRUE;
}

/*
 * @since 1.3
 */
EAPI void
ecore_x_e_window_profile_set(Ecore_X_Window win,
                             const char    *profile)
{
   Ecore_X_Atom atom;

   LOGFN;
   if (!win)
     return;

   if (!profile)
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_WINDOW_PROFILE);
   else
     {
        atom = ecore_x_atom_get(profile);
        ecore_x_window_prop_property_set(win, ECORE_X_ATOM_E_WINDOW_PROFILE,
                                         XA_ATOM, 32, (void *)&atom, 1);
     }
}

/*
 * @since 1.3
 */
EAPI char *
ecore_x_e_window_profile_get(Ecore_X_Window win)
{
   Ecore_X_Atom *atom = NULL;
   unsigned char *data;
   char *profile = NULL;
   int num;

   LOGFN;
   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_E_WINDOW_PROFILE,
                                         XA_ATOM, 32, &data, &num))
     {
        if (data) free(data);
        return NULL;
     }

   if (data)
     atom = (Ecore_X_Atom *)data;

   if (atom)
     profile = ecore_x_atom_name_get(atom[0]);

   if (data) free(data);

   return profile;
}

EAPI void
ecore_x_e_window_profile_supported_set(Ecore_X_Window root,
                                       Eina_Bool      enabled)
{
   Ecore_X_Window win;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   if (enabled)
     {
        win = ecore_x_window_new(root, 1, 2, 3, 4);
        ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
        ecore_x_window_prop_xid_set(root, ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
     }
   else
     {
        int ret;

        ret =
          ecore_x_window_prop_xid_get(root,
                                      ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win, 1);
        if ((ret == 1) && (win))
          {
             ecore_x_window_prop_property_del(
               root,
               ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED);
             ecore_x_window_free(win);
          }
     }
}

EAPI Eina_Bool
ecore_x_e_window_profile_supported_get(Ecore_X_Window root)
{
   Ecore_X_Window win, win2;
   int ret;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   ret =
     ecore_x_window_prop_xid_get(root,
                                 ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
                                 ECORE_X_ATOM_WINDOW,
                                 &win, 1);
   if ((ret == 1) && (win))
     {
        ret =
          ecore_x_window_prop_xid_get(win,
                                      ECORE_X_ATOM_E_WINDOW_PROFILE_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win2, 1);
        if ((ret == 1) && (win2 == win))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void
ecore_x_e_window_available_profiles_set(Ecore_X_Window  win,
                                        const char    **profiles,
                                        unsigned int    count)
{
   Ecore_X_Atom *atoms;

   LOGFN;

   if (!win)
     return;

   if ((!profiles) || (count == 0))
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST);
   else
     {
        atoms = alloca(count * sizeof(Ecore_X_Atom));
        ecore_x_atoms_get(profiles, count, atoms);
        ecore_x_window_prop_property_set(win,
                                         ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST,
                                         XA_ATOM, 32, (void *)atoms,
                                         count);
     }
}

EAPI Eina_Bool
ecore_x_e_window_available_profiles_get(Ecore_X_Window   win,
                                        const char    ***profiles,
                                        int             *count)
{
   unsigned char *data;
   Ecore_X_Atom *atoms;
   int num, i;

   LOGFN;

   if (count)
     *count = 0;

   if (profiles)
     *profiles = NULL;

   if (!win)
     return EINA_FALSE;

   if (!ecore_x_window_prop_property_get(win,
                                         ECORE_X_ATOM_E_WINDOW_PROFILE_AVAILABLE_LIST,
                                         XA_ATOM, 32, &data, &num))
   {
     if (data)
       free(data);
     return EINA_FALSE;
   }

   if (count)
     *count = num;

   if (profiles)
     {
        (*profiles) = calloc(num, sizeof(char *));
        if (!(*profiles))
          {
             if (count)
               *count = 0;

             if (data)
               free(data);

             return EINA_FALSE;
          }

        atoms = (Ecore_X_Atom *)data;
        for (i = 0; i < num; i++)
           (*profiles)[i] = ecore_x_atom_name_get(atoms[i]);
     }

   if (data)
     XFree(data);

   return EINA_TRUE;
}

EAPI void
ecore_x_e_window_profile_change_send(Ecore_X_Window  root,
                                     Ecore_X_Window  win,
                                     const char     *profile)
{
   XEvent xev = { 0 };
   Ecore_X_Atom atom;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   if (!win)
     return;

   atom = ecore_x_atom_get(profile);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_WINDOW_PROFILE_CHANGE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = atom;
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_window_profile_change_request_send(Ecore_X_Window win,
                                             const char    *profile)
{
   XEvent xev = { 0 };
   Ecore_X_Atom atom;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!win)
     return;

   atom = ecore_x_atom_get(profile);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_WINDOW_PROFILE_CHANGE_REQUEST;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = atom;
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}


EAPI void
ecore_x_e_window_profile_change_done_send(Ecore_X_Window root,
                                          Ecore_X_Window win,
                                          const char    *profile)
{
   XEvent xev = { 0 };
   Ecore_X_Atom atom;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   atom = ecore_x_atom_get(profile);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_WINDOW_PROFILE_CHANGE_DONE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = atom;
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_sync_supported_set(Ecore_X_Window root,
                                  Eina_Bool enabled)
{
   Ecore_X_Window win;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   LOGFN;
   if (enabled)
     {
        win = ecore_x_window_new(root, 1, 2, 3, 4);
        ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
        ecore_x_window_prop_xid_set(root, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
     }
   else
     {
        int ret;

        ret =
          ecore_x_window_prop_xid_get(root,
                                      ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win, 1);
        if ((ret == 1) && (win))
          {
             ecore_x_window_prop_property_del(
               root,
               ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED);
             ecore_x_window_free(win);
          }
     }
}

EAPI Eina_Bool
ecore_x_e_comp_sync_supported_get(Ecore_X_Window root)
{
   Ecore_X_Window win, win2;
   int ret;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   ret =
     ecore_x_window_prop_xid_get(root,
                                 ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED,
                                 ECORE_X_ATOM_WINDOW,
                                 &win, 1);
   if ((ret == 1) && (win))
     {
        ret =
          ecore_x_window_prop_xid_get(win,
                                      ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win2, 1);
        if ((ret == 1) && (win2 == win))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void
ecore_x_e_comp_sync_begin_send(Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_SYNC_BEGIN;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // later
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False,
              NoEventMask, //SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_sync_end_send(Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_SYNC_END;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // later
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False,
              NoEventMask, //SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_sync_cancel_send(Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_SYNC_CANCEL;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // later
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False,
              NoEventMask, //SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_flush_send(Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_FLUSH;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // later
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False,
              NoEventMask, //SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_dump_send(Ecore_X_Window win)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_COMP_DUMP;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = 0; // later
   xev.xclient.data.l[2] = 0; // later
   xev.xclient.data.l[3] = 0; // later
   xev.xclient.data.l[4] = 0; // later

   XSendEvent(_ecore_x_disp, win, False,
              NoEventMask, //SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI void
ecore_x_e_comp_pixmap_set(Ecore_X_Window win,
                          Ecore_X_Pixmap pixmap)
{
   LOGFN;
   if (pixmap)
     ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_PIXMAP,
                                 ECORE_X_ATOM_PIXMAP, &pixmap, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_COMP_PIXMAP);
}

EAPI Ecore_X_Pixmap
ecore_x_e_comp_pixmap_get(Ecore_X_Window win)
{
   int ret = 0;
   Ecore_X_Pixmap pixmap = 0;

   LOGFN;
   ret =
     ecore_x_window_prop_xid_get(win,
                                 ECORE_X_ATOM_E_COMP_PIXMAP,
                                 ECORE_X_ATOM_PIXMAP,
                                 &pixmap, 1);
   if (ret != 1)
     return 0;

   return pixmap;
}

static Ecore_X_Atom
_ecore_x_e_indicator_atom_get(Ecore_X_Illume_Indicator_State state)
{
   switch (state)
     {
      case ECORE_X_ILLUME_INDICATOR_STATE_ON:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_ON;

      case ECORE_X_ILLUME_INDICATOR_STATE_OFF:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_OFF;

      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Indicator_State
_ecore_x_e_indicator_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_ON)
     return ECORE_X_ILLUME_INDICATOR_STATE_ON;

   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_OFF)
     return ECORE_X_ILLUME_INDICATOR_STATE_OFF;

   return ECORE_X_ILLUME_INDICATOR_STATE_UNKNOWN;
}

EAPI void
ecore_x_e_illume_indicator_state_set(Ecore_X_Window win,
                                     Ecore_X_Illume_Indicator_State state)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_indicator_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Indicator_State
ecore_x_e_illume_indicator_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                     &atom, 1))
     return ECORE_X_ILLUME_INDICATOR_STATE_UNKNOWN;

   return _ecore_x_e_indicator_state_get(atom);
}

EAPI void
ecore_x_e_illume_indicator_state_send(Ecore_X_Window win,
                                      Ecore_X_Illume_Indicator_State state)
{
   LOGFN;
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_indicator_atom_get(state),
                                 0, 0, 0, 0);
}

static Ecore_X_Atom
_ecore_x_e_indicator_opacity_atom_get(Ecore_X_Illume_Indicator_Opacity_Mode mode)
{
   switch (mode)
     {
      case ECORE_X_ILLUME_INDICATOR_OPAQUE:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_OPAQUE;

      case ECORE_X_ILLUME_INDICATOR_TRANSLUCENT:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_TRANSLUCENT;

      case ECORE_X_ILLUME_INDICATOR_TRANSPARENT:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_TRANSPARENT;

      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Indicator_Opacity_Mode
_ecore_x_e_indicator_opacity_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_OPAQUE)
     return ECORE_X_ILLUME_INDICATOR_OPAQUE;

   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_TRANSLUCENT)
     return ECORE_X_ILLUME_INDICATOR_TRANSLUCENT;

   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_TRANSPARENT)
     return ECORE_X_ILLUME_INDICATOR_TRANSPARENT;

   return ECORE_X_ILLUME_INDICATOR_OPACITY_UNKNOWN;
}

EAPI void
ecore_x_e_illume_indicator_opacity_set(Ecore_X_Window win,
                                     Ecore_X_Illume_Indicator_Opacity_Mode mode)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_indicator_opacity_atom_get(mode);
   ecore_x_window_prop_atom_set(win,
                                ECORE_X_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Indicator_Opacity_Mode
ecore_x_e_illume_indicator_opacity_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE,
                                     &atom, 1))
     return ECORE_X_ILLUME_INDICATOR_OPACITY_UNKNOWN;

   return _ecore_x_e_indicator_opacity_get(atom);
}

EAPI void
ecore_x_e_illume_indicator_opacity_send(Ecore_X_Window win,
                                      Ecore_X_Illume_Indicator_Opacity_Mode mode)
{
   LOGFN;
   ecore_x_client_message32_send(win,
                                 ECORE_X_ATOM_E_ILLUME_INDICATOR_OPACITY_MODE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_indicator_opacity_atom_get(mode),
                                 0, 0, 0, 0);
}

static Ecore_X_Atom
_ecore_x_e_indicator_type_atom_get(Ecore_X_Illume_Indicator_Type_Mode mode)
{
   switch (mode)
     {
      case ECORE_X_ILLUME_INDICATOR_TYPE_1:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_1;

      case ECORE_X_ILLUME_INDICATOR_TYPE_2:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_2;

      case ECORE_X_ILLUME_INDICATOR_TYPE_3:
        return ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_3;

      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Indicator_Type_Mode
_ecore_x_e_indicator_type_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_1)
     return ECORE_X_ILLUME_INDICATOR_TYPE_1;

   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_2)
     return ECORE_X_ILLUME_INDICATOR_TYPE_2;

   if (atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_3)
     return ECORE_X_ILLUME_INDICATOR_TYPE_3;

   return ECORE_X_ILLUME_INDICATOR_TYPE_UNKNOWN;
}

EAPI void
ecore_x_e_illume_indicator_type_set(Ecore_X_Window win,
                                     Ecore_X_Illume_Indicator_Type_Mode mode)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_indicator_type_atom_get(mode);
   ecore_x_window_prop_atom_set(win,
                                ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_MODE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Indicator_Type_Mode
ecore_x_e_illume_indicator_type_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_MODE,
                                     &atom, 1))
     return ECORE_X_ILLUME_INDICATOR_TYPE_UNKNOWN;

   return _ecore_x_e_indicator_type_get(atom);
}

EAPI void
ecore_x_e_illume_indicator_type_send(Ecore_X_Window win,
                                      Ecore_X_Illume_Indicator_Type_Mode mode)
{
   LOGFN;
   ecore_x_client_message32_send(win,
                                 ECORE_X_ATOM_E_ILLUME_INDICATOR_TYPE_MODE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_indicator_type_atom_get(mode),
                                 0, 0, 0, 0);
}

static Ecore_X_Atom
_ecore_x_e_illume_window_state_atom_get(Ecore_X_Illume_Window_State state)
{
   switch (state)
     {
      case ECORE_X_ILLUME_WINDOW_STATE_NORMAL:
        return ECORE_X_ATOM_E_ILLUME_WINDOW_STATE_NORMAL;

      case ECORE_X_ILLUME_WINDOW_STATE_FLOATING:
        return ECORE_X_ATOM_E_ILLUME_WINDOW_STATE_FLOATING;

      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Window_State
_ecore_x_e_illume_window_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_WINDOW_STATE_NORMAL)
     return ECORE_X_ILLUME_WINDOW_STATE_NORMAL;

   if (atom == ECORE_X_ATOM_E_ILLUME_WINDOW_STATE_FLOATING)
     return ECORE_X_ILLUME_WINDOW_STATE_FLOATING;

   return ECORE_X_ILLUME_WINDOW_STATE_NORMAL;
}

EAPI void
ecore_x_e_illume_window_state_set(Ecore_X_Window win,
                                  Ecore_X_Illume_Window_State state)
{
   Ecore_X_Atom atom = 0;

   LOGFN;
   atom = _ecore_x_e_illume_window_state_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_WINDOW_STATE,
                                &atom, 1);
}

EAPI Ecore_X_Illume_Window_State
ecore_x_e_illume_window_state_get(Ecore_X_Window win)
{
   Ecore_X_Atom atom;

   LOGFN;
   if (!ecore_x_window_prop_atom_get(win,
                                     ECORE_X_ATOM_E_ILLUME_WINDOW_STATE,
                                     &atom, 1))
     return ECORE_X_ILLUME_WINDOW_STATE_NORMAL;

   return _ecore_x_e_illume_window_state_get(atom);
}

EAPI void
ecore_x_e_illume_window_state_send(Ecore_X_Window win,
                                   Ecore_X_Illume_Window_State state)
{
   LOGFN;
   ecore_x_client_message32_send(win,
                                 ECORE_X_ATOM_E_ILLUME_WINDOW_STATE,
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE,
                                 _ecore_x_e_illume_window_state_atom_get(state),
                                 0, 0, 0, 0);
}

EAPI void
ecore_x_e_window_rotation_supported_set(Ecore_X_Window root,
                                        Eina_Bool      enabled)
{
   Ecore_X_Window win;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   if (enabled)
     {
        win = ecore_x_window_new(root, 1, 2, 3, 4);
        ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
        ecore_x_window_prop_xid_set(root, ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED,
                                    ECORE_X_ATOM_WINDOW, &win, 1);
     }
   else
     {
        int ret;

        ret =
          ecore_x_window_prop_xid_get(root,
                                      ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win, 1);
        if ((ret == 1) && (win))
          {
             ecore_x_window_prop_property_del
               (root, ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED);
             ecore_x_window_free(win);
          }
     }
}

EAPI Eina_Bool
ecore_x_e_window_rotation_supported_get(Ecore_X_Window root)
{
   Ecore_X_Window win, win2;
   int ret;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   ret =
     ecore_x_window_prop_xid_get(root,
                                 ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED,
                                 ECORE_X_ATOM_WINDOW,
                                 &win, 1);
   if ((ret == 1) && (win))
     {
        ret =
          ecore_x_window_prop_xid_get(win,
                                      ECORE_X_ATOM_E_WINDOW_ROTATION_SUPPORTED,
                                      ECORE_X_ATOM_WINDOW,
                                      &win2, 1);
        if ((ret == 1) && (win2 == win))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void
ecore_x_e_window_rotation_app_set(Ecore_X_Window win,
                                  Eina_Bool      set)
{
   unsigned int val = 0;

   LOGFN;

   if (set) val = 1;

   ecore_x_window_prop_card32_set(win,
                                  ECORE_X_ATOM_E_WINDOW_ROTATION_APP_SUPPORTED,
                                  &val, 1);
}

EAPI Eina_Bool
ecore_x_e_window_rotation_app_get(Ecore_X_Window win)
{
   unsigned int val = 0;

   LOGFN;
   if (!ecore_x_window_prop_card32_get(win,
                                       ECORE_X_ATOM_E_WINDOW_ROTATION_APP_SUPPORTED,
                                       &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_window_rotation_preferred_rotation_set(Ecore_X_Window win,
                                                 int            rot)
{
   unsigned int val = 0;

   LOGFN;

   if (rot != -1)
     {
        val = (unsigned int)rot;
        ecore_x_window_prop_card32_set(win,
                                       ECORE_X_ATOM_E_WINDOW_ROTATION_PREFERRED_ROTATION,
                                       &val, 1);
     }
   else
     {
        ecore_x_window_prop_property_del(win,
                                         ECORE_X_ATOM_E_WINDOW_ROTATION_PREFERRED_ROTATION);
     }
}

EAPI Eina_Bool
ecore_x_e_window_rotation_preferred_rotation_get(Ecore_X_Window win,
                                                 int           *rot)
{
   unsigned int val = 0;
   int ret = 0;

   LOGFN;

   ret = ecore_x_window_prop_card32_get(win,
                                        ECORE_X_ATOM_E_WINDOW_ROTATION_PREFERRED_ROTATION,
                                        &val, 1);
   if (ret == 1)
     {
        if (rot) *rot = (int)val;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI void
ecore_x_e_window_rotation_available_rotations_set(Ecore_X_Window win,
                                                  const int     *rots,
                                                  unsigned int   count)
{
   LOGFN;

   if (!win) return;

   if ((rots) && (count > 0))
     ecore_x_window_prop_card32_set(win,
                                    ECORE_X_ATOM_E_WINDOW_ROTATION_AVAILABLE_LIST,
                                    (unsigned int *)rots, count);
   else
     ecore_x_window_prop_property_del(win,
                                      ECORE_X_ATOM_E_WINDOW_ROTATION_AVAILABLE_LIST);
}

EAPI Eina_Bool
ecore_x_e_window_rotation_available_rotations_get(Ecore_X_Window  win,
                                                  int           **rots,
                                                  unsigned int   *count)
{
   unsigned char *data = NULL;
   int num, i;
   int *val = NULL;

   LOGFN;

   if ((!win) || (!rots) || (!count))
     return EINA_FALSE;

   *rots = NULL;
   *count = 0;

   if (!ecore_x_window_prop_property_get(win,
                                         ECORE_X_ATOM_E_WINDOW_ROTATION_AVAILABLE_LIST,
                                         XA_CARDINAL, 32, &data, &num))
     return EINA_FALSE;

   *count = num;

   if ((num >= 1) && (data))
     {
        val = calloc(num, sizeof(int));
        if (!val)
          {
             if (data) XFree(data);
             return EINA_FALSE;
          }
        for (i = 0; i < num; i++)
          val[i] = ((int *)data)[i];
        if (data) XFree(data);
        *rots = val;
        return EINA_TRUE;
     }
   if (data) XFree(data);
   return EINA_FALSE;
}

EAPI void
ecore_x_e_window_rotation_change_prepare_send(Ecore_X_Window win,
                                              int            rot,
                                              Eina_Bool      resize,
                                              int            w,
                                              int            h)
{
   LOGFN;
   ecore_x_client_message32_send
     (win, ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_PREPARE,
     ECORE_X_EVENT_MASK_NONE,
     win, rot, resize, w, h);
}

EAPI void
ecore_x_e_window_rotation_change_prepare_done_send(Ecore_X_Window root,
                                                   Ecore_X_Window win,
                                                   int            rot)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_PREPARE_DONE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = rot;
   xev.xclient.data.l[2] = 0;
   xev.xclient.data.l[3] = 0;
   xev.xclient.data.l[4] = 0;

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
}

EAPI void
ecore_x_e_window_rotation_change_request_send(Ecore_X_Window win,
                                              int            rot)
{
   LOGFN;
   ecore_x_client_message32_send
     (win, ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_REQUEST,
     ECORE_X_EVENT_MASK_NONE,
     win, rot, 0, 0, 0);
}

EAPI void
ecore_x_e_window_rotation_change_done_send(Ecore_X_Window root,
                                           Ecore_X_Window win,
                                           int            rot,
                                           int            w,
                                           int            h)
{
   XEvent xev = { 0 };

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (!root)
     root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_E_WINDOW_ROTATION_CHANGE_DONE;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = win;
   xev.xclient.data.l[1] = rot;
   xev.xclient.data.l[2] = w;
   xev.xclient.data.l[3] = h;
   xev.xclient.data.l[4] = 0;

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureRedirectMask | SubstructureNotifyMask,
              &xev);
}

/*
 * Does keyrouter exist?
 */
EAPI void
ecore_x_e_keyrouter_set(Ecore_X_Window win EINA_UNUSED,
                        Eina_Bool on)
{
   //key router call this api when it start running
   unsigned int val;
   Ecore_X_Window root;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   root = DefaultRootWindow(_ecore_x_disp);

   val = (on) ? 1 : 0;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_E_KEYROUTER_SUPPORTED,
                                  &val, 1);
}

EAPI Eina_Bool
ecore_x_e_keyrouter_get(Ecore_X_Window win EINA_UNUSED)
{
   //check the existance of keyrouter
   int ret;
   unsigned int val;
   Ecore_X_Window root;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

   root = DefaultRootWindow(_ecore_x_disp);

   ret = ecore_x_window_prop_card32_get(root, ECORE_X_ATOM_E_KEYROUTER_SUPPORTED,
                                        &val, 1);
   if (ret != 1) return EINA_FALSE;

   return val == 1 ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_x_e_stack_type_set(Ecore_X_Window win,
                         Ecore_X_Stack_Type stack_type)
{
   unsigned int val = stack_type;
   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (stack_type == ECORE_X_STACK_NONE)
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_STACK_TYPE);
   else
     ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_STACK_TYPE,
                                    &val, 1);
}

EAPI Ecore_X_Stack_Type
ecore_x_e_stack_type_get(Ecore_X_Window win)
{
   int ret;
   unsigned int val;
   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, ECORE_X_STACK_NONE);

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_STACK_TYPE,
                                        &val, 1);
   if (ret != 1) return ECORE_X_STACK_NONE;
   if (val >= ECORE_X_STACK_LAST) val = ECORE_X_STACK_STANDARD;
   return (Ecore_X_Stack_Type)val;
}

EAPI void
ecore_x_e_stack_position_set(Ecore_X_Window win,
                             Ecore_X_Stack_Position stack_position)
{
   unsigned int val = stack_position;
   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (stack_position == ECORE_X_STACK_POSITION_NONE)
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_STACK_POSITION);
   else
     ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_STACK_POSITION,
                                    &val, 1);
}

EAPI Ecore_X_Stack_Position
ecore_x_e_stack_position_get(Ecore_X_Window win)
{
   int ret;
   unsigned int val;
   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, ECORE_X_STACK_POSITION_NONE);

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_STACK_POSITION,
                                        &val, 1);
   if (ret != 1) return ECORE_X_STACK_POSITION_NONE;
   if (val >= ECORE_X_STACK_POSITION_LAST) val = ECORE_X_STACK_POSITION_BOTTOM;
   return (Ecore_X_Stack_Position)val;
}
