/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static Ecore_X_DND_Protocol *_xdnd = NULL;
static int _ecore_x_dnd_init_count = 0;

void
_ecore_x_dnd_init(void)
{
   if (!_ecore_x_dnd_init_count)
     {
	_xdnd = calloc(1, sizeof(Ecore_X_DND_Protocol));

	_xdnd->version = ECORE_X_DND_VERSION;
	_xdnd->source = None;
	_xdnd->dest = None;
	_xdnd->state = ECORE_X_DND_IDLE;
     }

   _ecore_x_dnd_init_count++;
}

void
_ecore_x_dnd_shutdown(void)
{
   _ecore_x_dnd_init_count--;
   if (_ecore_x_dnd_init_count > 0)
     return;

   if (_xdnd)
     free(_xdnd);
   _xdnd = NULL;
   _ecore_x_dnd_init_count = 0;
}

void
ecore_x_dnd_aware_set(Ecore_X_Window win, int on)
{
   Atom prop_data = ECORE_X_DND_VERSION;

   if (on)
     ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_AWARE,
				      XA_ATOM, 32, &prop_data, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_AWARE);
}

int
ecore_x_dnd_version_get(Ecore_X_Window win)
{
   unsigned char *prop_data;
   int num;

   if (ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_AWARE,
                                        XA_ATOM, 32, &prop_data, &num))
     {
	int version = (int) *prop_data;
	free(prop_data);
	return version;
     }
   else
     return 0;
}

int
ecore_x_dnd_type_isset(Ecore_X_Window win, const char *type)
{
   int                  num, i, ret = 0;
   unsigned char       *data;
   Ecore_X_Atom        *atoms, atom;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST,
					 XA_ATOM, 32, &data, &num))
     return ret;

   atom = ecore_x_atom_get(type);
   atoms = (Ecore_X_Atom *)data;

   for (i = 0; i < num; ++i)
     {
	if (atom == atoms[i])
	  {
	     ret = 1;
	     break;
	  }
     }

   XFree(data);
   return ret;
}

void
ecore_x_dnd_type_set(Ecore_X_Window win, const char *type, int on)
{
   Ecore_X_Atom      atom;
   Ecore_X_Atom      *oldset = NULL, *newset = NULL;
   int               i, j = 0, num = 0;
   unsigned char     *data = NULL;
   unsigned char     *old_data = NULL;

   atom = ecore_x_atom_get(type);
   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_XDND_TYPE_LIST,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *)old_data;

   if (on)
     {
	if (ecore_x_dnd_type_isset(win, type))
	  {
	     XFree(old_data);
	     return;
	  }
	newset = calloc(num + 1, sizeof(Ecore_X_Atom));
	if (!newset) return;
	data = (unsigned char *)newset;

	for (i = 0; i < num; i++)
	  newset[i + 1] = oldset[i];
	/* prepend the new type */
	newset[0] = atom;

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
					 XA_ATOM, 32, data, num + 1);
     }
   else
     {
	if (!ecore_x_dnd_type_isset(win, type))
	  {
	     XFree(old_data);
	     return;
	  }
	newset = calloc(num - 1, sizeof(Atom));
	if (!newset)
	  {
	     XFree(old_data);
	     return;
	  }
	data = (unsigned char *)newset;
	for (i = 0; i < num; i++)
	  if (oldset[i] != atom)
	    newset[j++] = oldset[i];

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
					 XA_ATOM, 32, data, num - 1);
     }
   XFree(oldset);
   free(newset);
}

Ecore_X_DND_Protocol *
_ecore_x_dnd_protocol_get(void)
{
   return _xdnd;
}

int
ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size)
{

   if (!ecore_x_dnd_version_get(source))
     return 0;

   /* Take ownership of XdndSelection */
   if (!ecore_x_selection_xdnd_set(source, data, size))
     return 0;

   _xdnd->source = source;
   _xdnd->state = ECORE_X_DND_DRAGGING;
   _xdnd->time = _ecore_x_event_last_time;

   /* Default Accepted Action: ask */
   _xdnd->action = ECORE_X_ATOM_XDND_ACTION_ASK;
   _xdnd->accepted_action = None;
   return 1;
}

void
ecore_x_dnd_drop(void)
{
   XEvent xev;

   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.format = 32;
   xev.xclient.window = _xdnd->dest;

   if (_xdnd->will_accept)
     {
	xev.xclient.message_type = ECORE_X_ATOM_XDND_DROP;
	xev.xclient.data.l[0] = _xdnd->source;
	xev.xclient.data.l[1] = 0;
	xev.xclient.data.l[2] = _xdnd->time;
	XSendEvent(_ecore_x_disp, _xdnd->dest, False, 0, &xev);
     }
   else
     {
	xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
	xev.xclient.data.l[0] = _xdnd->source;
	xev.xclient.data.l[1] = 0;
	XSendEvent(_ecore_x_disp, _xdnd->dest, False, 0, &xev);
     }
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
   xev.xclient.message_type = ECORE_X_ATOM_XDND_STATUS;
   xev.xclient.format = 32;
   xev.xclient.window = _xdnd->source;

   xev.xclient.data.l[0] = _xdnd->dest;
   xev.xclient.data.l[1] = 0;
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
ecore_x_dnd_send_finished(void)
{
   XEvent   xev;

   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.message_type = ECORE_X_ATOM_XDND_FINISHED;
   xev.xclient.format = 32;
   xev.xclient.window = _xdnd->source;

   xev.xclient.data.l[0] = _xdnd->dest;
   xev.xclient.data.l[1] = 0;
   xev.xclient.data.l[2] = 0;
   if (_xdnd->will_accept)
     {
	xev.xclient.data.l[1] |= 0x1UL;
	xev.xclient.data.l[2] = _xdnd->accepted_action;
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
   if ((_xdnd->dest) && (win != _xdnd->dest))
     {
	xev.xclient.window = _xdnd->dest;
	xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
	xev.xclient.data.l[0] = _xdnd->source;
	xev.xclient.data.l[1] = 0;

	XSendEvent(_ecore_x_disp, _xdnd->dest, False, 0, &xev);
     }

   if (win)
     {
	_xdnd->version = MIN(ECORE_X_DND_VERSION,
			     ecore_x_dnd_version_get(win));
	if (win != _xdnd->dest)
	  {
	     int i, num;
	     unsigned char *data;
	     Ecore_X_Atom *types;

	     ecore_x_window_prop_property_get(_xdnd->source, ECORE_X_ATOM_XDND_TYPE_LIST,
					      XA_ATOM, 32, &data, &num);
	     types = (Ecore_X_Atom *)data;

	     /* Entered new window, send XdndEnter */
	     xev.xclient.window = win;
	     xev.xclient.message_type = ECORE_X_ATOM_XDND_ENTER;
	     xev.xclient.data.l[0] = _xdnd->source;
	     xev.xclient.data.l[1] = 0;
	     if (num > 3)
	       xev.xclient.data.l[1] |= 0x1UL;
	     else
	       xev.xclient.data.l[1] &= 0xfffffffeUL;
	     xev.xclient.data.l[1] |= ((unsigned long) _xdnd->version) << 24;

	     for (i = 2; i < 5; i++)
	       xev.xclient.data.l[i] = 0;
	     for (i = 0; i < MIN(num, 3); ++i)
	       xev.xclient.data.l[i + 2] = types[i];
	     XFree(data);
	     XSendEvent(_ecore_x_disp, win, False, 0, &xev);
	     _xdnd->await_status = 0;
	  }

	/* FIXME: Handle box information */
	/*if (!_xdnd->await_status)
	  {*/
	xev.xclient.window = win;
	xev.xclient.message_type = ECORE_X_ATOM_XDND_POSITION;
	xev.xclient.data.l[0] = _xdnd->source;
	xev.xclient.data.l[1] = 0; /* Reserved */
	xev.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
	xev.xclient.data.l[3] = _xdnd->time; /* Version 1 */
	xev.xclient.data.l[4] = _xdnd->action; /* Version 2, Needs to be pre-set */
	XSendEvent(_ecore_x_disp, win, False, 0, &xev);
	_xdnd->await_status = 1;
	/*}*/
     }

   if (_xdnd->state == ECORE_X_DND_DROPPED)
     {
	if (win)
	  {
	     if (_xdnd->will_accept)
	       {
		  xev.xclient.window = win;
		  xev.xclient.message_type = ECORE_X_ATOM_XDND_DROP;
		  xev.xclient.data.l[0] = _xdnd->source;
		  xev.xclient.data.l[1] = 0;
		  xev.xclient.data.l[2] = _xdnd->time;
		  XSendEvent(_ecore_x_disp, win, False, 0, &xev);
	       }
	     else
	       {
		  xev.xclient.window = win;
		  xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
		  xev.xclient.data.l[0] = _xdnd->source;
		  xev.xclient.data.l[1] = 0;
		  XSendEvent(_ecore_x_disp, win, False, 0, &xev);
	       }
	  }

	_xdnd->will_accept = 0;
     }
   _xdnd->dest = win;
}

