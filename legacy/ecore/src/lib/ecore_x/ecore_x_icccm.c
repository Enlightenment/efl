/*
 * Various ICCCM related functions.
 * These are normally called only by window managers.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

static void
_ecore_x_icccm_window_state_set(Ecore_X_Window win, unsigned int state)
{
   unsigned long       c[2];

   c[0] = state;
   c[1] = 0;
   XChangeProperty(_ecore_x_disp, win, _ecore_x_atom_wm_state,
		   _ecore_x_atom_wm_state, 32, PropModeReplace,
		   (unsigned char *)c, 2);
}

void
ecore_x_icccm_window_state_set_iconic(Ecore_X_Window win)
{
   _ecore_x_icccm_window_state_set(win, IconicState);
}

void
ecore_x_icccm_window_state_set_normal(Ecore_X_Window win)
{
   _ecore_x_icccm_window_state_set(win, NormalState);
}

void
ecore_x_icccm_window_state_set_withdrawn(Ecore_X_Window win)
{
   _ecore_x_icccm_window_state_set(win, WithdrawnState);
}

static void
_ecore_x_icccm_client_message_send(Ecore_X_Window win,
				  Ecore_X_Atom atom, Ecore_X_Time ts)
{
   ecore_x_client_message32_send(win, _ecore_x_atom_wm_protocols, atom, ts,
				 0, 0, 0);
}

void
ecore_x_icccm_send_delete_window(Ecore_X_Window win)
{
   _ecore_x_icccm_client_message_send(win, _ecore_x_atom_wm_delete_window,
				      CurrentTime);
}

void
ecore_x_icccm_send_take_focus(Ecore_X_Window win)
{
   _ecore_x_icccm_client_message_send(win, _ecore_x_atom_wm_take_focus,
				      CurrentTime);
}

#if 0
void
ecore_x_icccm_send_save_yourself(Ecore_X_Window win)
{
   _ecore_x_icccm_client_message_send(win, _ecore_x_atom_wm_save_yourself,
				      CurrentTime);
}
#endif
