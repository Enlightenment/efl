#include <Ecore.h>
#include "ecore_x_private.h"
#include <Ecore_X.h>

static Ecore_X_Selection_Data _xdnd_selection;
static Ecore_X_DND_Protocol *_xdnd;

void
_ecore_x_dnd_init (void)
{
   _xdnd = calloc(1, sizeof(Ecore_X_DND_Protocol));
   
   _xdnd->version = ECORE_X_DND_VERSION;
   _xdnd->source = None;
   _xdnd->dest = None;
   _xdnd->state = ECORE_X_DND_IDLE;
}

void
ecore_x_dnd_aware_set (Ecore_X_Window win, int on)
{
   Atom prop_data = ECORE_X_DND_VERSION;
   
   if (on)
      ecore_x_window_prop_property_set(win, _ecore_x_atom_xdnd_aware, 
                                       XA_ATOM, 32, &prop_data, 1);
   else
      ecore_x_window_prop_property_del(win, _ecore_x_atom_xdnd_aware);
   /* TODO: Add dnd typelist to window properties */
}

int
ecore_x_dnd_version_get (Ecore_X_Window win)
{
   unsigned char *prop_data;
   int num;
   
   if (ecore_x_window_prop_property_get(win, _ecore_x_atom_xdnd_aware,
                                        XA_ATOM, 32, &prop_data, &num))
   {
      int version = (int) *prop_data;
      free(prop_data);
      return version;
   }
   else
      return 0;
} 

Ecore_X_DND_Protocol *
_ecore_x_dnd_protocol_get (void)
{
   return _xdnd;
}

int 
ecore_x_dnd_begin (Ecore_X_Window source, unsigned char *data, int size)
{
   unsigned char *buf;

   if (!ecore_x_dnd_version_get(source))
      return 0;
   
   /* Take ownership of XdndSelection */
   XSetSelectionOwner(_ecore_x_disp, _ecore_x_atom_selection_xdnd, source,
                      _ecore_x_event_last_time);
   if (XGetSelectionOwner(_ecore_x_disp, _ecore_x_atom_selection_xdnd) != source)
      return 0;
   
   /* Initialize Selection Data Struct */
   _xdnd_selection.win = source;
   _xdnd_selection.selection = _ecore_x_atom_selection_xdnd;
   _xdnd_selection.length = size;
   _xdnd_selection.time = _ecore_x_event_last_time;
   
   buf = malloc(size);
   memcpy(buf, data, size);
   _xdnd_selection.data = buf;

   _xdnd->source = source;
   _xdnd->state = ECORE_X_DND_DRAGGING;

   return 1;
}

void 
ecore_x_dnd_send_status(int will_accept, int suppress, Ecore_X_Rectangle rectangle, Ecore_X_Atom action)
{
   XEvent xev;

   if (_xdnd->state == ECORE_X_DND_IDLE || 
       _xdnd->state == ECORE_X_DND_FINISHED)
      return;

   memset(&xev, 0, sizeof(XEvent));

   _xdnd->rectangle.x = rectangle.x;
   _xdnd->rectangle.y = rectangle.y;
   _xdnd->rectangle.width = rectangle.width;
   _xdnd->rectangle.height = rectangle.height;
   _xdnd->will_accept = will_accept;
   _xdnd->suppress = suppress;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.message_type = _ecore_x_atom_xdnd_status;
   xev.xclient.format = 32;
   xev.xclient.window = _xdnd->source;

   xev.xclient.data.l[0] = _xdnd->dest;
   if (will_accept)
      xev.xclient.data.l[1] |= 0x1UL;
   if (!suppress)
      xev.xclient.data.l[1] |= 0x2UL;
   
   /* Set rectangle information */
   xev.xclient.data.l[2] = rectangle.x;
   xev.xclient.data.l[2] <<= 16;
   xev.xclient.data.l[2] |= rectangle.y;
   xev.xclient.data.l[3] = rectangle.width;
   xev.xclient.data.l[3] <<= 16;
   xev.xclient.data.l[3] |= rectangle.height;

   if (will_accept)
   {
      xev.xclient.data.l[4] = action;
      _xdnd->accepted_action = action;
   }
   else
   {
      xev.xclient.data.l[4] = None;
      _xdnd->accepted_action = action;
   }

   XSendEvent(_ecore_x_disp, _xdnd->source, False, 0, &xev);
}
