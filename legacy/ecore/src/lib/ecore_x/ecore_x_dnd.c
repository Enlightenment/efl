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
   Atom _type_text_plain; /* FIXME: API-ize this stuff */

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
   _xdnd->time = _ecore_x_event_last_time;

   /* TODO: Set supported data types in API */
   _type_text_plain = ecore_x_atom_get("text/plain");
   _xdnd->num_types = 1;
   _xdnd->types = (Atom *) calloc(1, sizeof(Atom));
   _xdnd->types[0] = _type_text_plain;
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

void
_ecore_x_dnd_drag(int x, int y)
{
   XEvent         xev;
   Ecore_X_Window win;
   
   /* Preinitialize XEvent struct */
   memset(&xev, 0, sizeof(XEvent));
   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.format = 32;

   /* Attempt to find a DND-capable window under the cursor */
   win = ecore_x_window_at_xy_get(x, y);
   while ((win) && !(ecore_x_dnd_version_get(win)))
      win = ecore_x_window_parent_get(win);

   /* Send XdndLeave to current destination window if we have left it */
   if ((win != _xdnd->dest) && (_xdnd->dest))
   {
      xev.xclient.window = _xdnd->dest;
      xev.xclient.message_type = _ecore_x_atom_xdnd_leave;
      xev.xclient.data.l[0] = _xdnd->source;

      XSendEvent(_ecore_x_disp, _xdnd->dest, False, 0, &xev);
   }

   if (win)
   {
      _xdnd->version = ECORE_MIN(ECORE_X_DND_VERSION, 
                                 ecore_x_dnd_version_get(win));
      if (win != _xdnd->dest)
      {
         int i;
         
         /* Entered new window, send XdndEnter */
         xev.xclient.window = win;
         xev.xclient.message_type = _ecore_x_atom_xdnd_enter;
         xev.xclient.data.l[0] = _xdnd->source;
         if(_xdnd->num_types > 3)
            xev.xclient.data.l[1] |= 0x1UL;
         else
            xev.xclient.data.l[1] &= 0xfffffffeUL;
         xev.xclient.data.l[1] |= ((unsigned long) _xdnd->version) << 24;
         
         for (i = 0; i < ECORE_MIN(_xdnd->num_types, 3); ++i)
            xev.xclient.data.l[i + 2] = _xdnd->types[i];
         XSendEvent(_ecore_x_disp, win, False, 0, &xev);
         _xdnd->await_status = 0;
      }

      /* FIXME: Handle box information */
      if (!_xdnd->await_status)
      {
         xev.xclient.window = win;
         xev.xclient.message_type = _ecore_x_atom_xdnd_position;
         xev.xclient.data.l[0] = _xdnd->source;
         xev.xclient.data.l[1] = 0; /* Reserved */
         xev.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y * 0xffff);
         xev.xclient.data.l[3] = CurrentTime; /* Version 1 */
         xev.xclient.data.l[4] = _xdnd->action; /* Version 2, Needs to be pre-set */
         XSendEvent(_ecore_x_disp, win, False, 0, &xev);
         _xdnd->await_status = 1;
      }
   }

   if (_xdnd->state == ECORE_X_DND_DROPPED)
   {
      if (win)
      {
         if (_xdnd->will_accept)
         {
            xev.xclient.window = win;
            xev.xclient.message_type = _ecore_x_atom_xdnd_drop;
            xev.xclient.data.l[0] = _xdnd->source;
            xev.xclient.data.l[1] = 0;
            xev.xclient.data.l[2] = CurrentTime;
            xev.xclient.data.l[3] = 0;
            xev.xclient.data.l[4] = 0;
            XSendEvent(_ecore_x_disp, win, False, 0, &xev);
         }
         else
         {
            xev.xclient.window = win;
            xev.xclient.message_type = _ecore_x_atom_xdnd_leave;
            xev.xclient.data.l[0] = _xdnd->source;
            memset(xev.xclient.data.l + 1, 0, sizeof(long) * 3); /* Evil */
            XSendEvent(_ecore_x_disp, win, False, 0, &xev);
         }
      }

      _xdnd->will_accept = 0;
   }
   _xdnd->dest = win;
}


