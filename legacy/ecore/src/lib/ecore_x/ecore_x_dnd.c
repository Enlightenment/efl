/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

Ecore_X_Atom     ECORE_X_ATOM_SELECTION_XDND = 0;
Ecore_X_Atom     ECORE_X_ATOM_SELECTION_PROP_XDND = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_AWARE = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ENTER = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_TYPE_LIST = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_POSITION = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_COPY = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_MOVE = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_LINK = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_PRIVATE = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_ASK = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_LIST = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_ACTION_DESCRIPTION = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_PROXY = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_STATUS = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_DROP = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_FINISHED = 0;
Ecore_X_Atom     ECORE_X_ATOM_XDND_LEAVE = 0;

/* Xdnd atoms that need to be exposed to the application interface */
Ecore_X_Atom  ECORE_X_DND_ACTION_COPY = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_MOVE = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_LINK = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_ASK = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_PRIVATE = 0;

int ECORE_X_EVENT_XDND_ENTER = 0;
int ECORE_X_EVENT_XDND_POSITION = 0;
int ECORE_X_EVENT_XDND_STATUS = 0;
int ECORE_X_EVENT_XDND_LEAVE = 0;
int ECORE_X_EVENT_XDND_DROP = 0;
int ECORE_X_EVENT_XDND_FINISHED = 0;

static Ecore_X_DND_Source *_source = NULL;
static Ecore_X_DND_Target *_target = NULL;
static int _ecore_x_dnd_init_count = 0;

void
_ecore_x_dnd_init(void)
{
   if (!_ecore_x_dnd_init_count)
     {
	_source = calloc(1, sizeof(Ecore_X_DND_Source));
	_source->version = ECORE_X_DND_VERSION;
	_source->win = None;
	_source->dest = None;
	_source->state = ECORE_X_DND_SOURCE_IDLE;

	_target = calloc(1, sizeof(Ecore_X_DND_Target));
	_target->win = None;
	_target->source = None;
	_target->state = ECORE_X_DND_TARGET_IDLE;
	
	ECORE_X_EVENT_XDND_ENTER              = ecore_event_type_new();
	ECORE_X_EVENT_XDND_POSITION           = ecore_event_type_new();
	ECORE_X_EVENT_XDND_STATUS             = ecore_event_type_new();
	ECORE_X_EVENT_XDND_LEAVE              = ecore_event_type_new();
	ECORE_X_EVENT_XDND_DROP               = ecore_event_type_new();
	ECORE_X_EVENT_XDND_FINISHED           = ecore_event_type_new();

	ECORE_X_ATOM_SELECTION_PROP_XDND      = XInternAtom(_ecore_x_disp, "JXSelectionWindowProperty", False);
	ECORE_X_ATOM_SELECTION_XDND           = XInternAtom(_ecore_x_disp, "XdndSelection", False);
	ECORE_X_ATOM_XDND_AWARE               = XInternAtom(_ecore_x_disp, "XdndAware", False);
	ECORE_X_ATOM_XDND_TYPE_LIST           = XInternAtom(_ecore_x_disp, "XdndTypeList", False);
	ECORE_X_ATOM_XDND_ENTER               = XInternAtom(_ecore_x_disp, "XdndEnter", False);
	ECORE_X_ATOM_XDND_POSITION            = XInternAtom(_ecore_x_disp, "XdndPosition", False);
	ECORE_X_ATOM_XDND_ACTION_COPY         = XInternAtom(_ecore_x_disp, "XdndActionCopy", False);
	ECORE_X_ATOM_XDND_ACTION_MOVE         = XInternAtom(_ecore_x_disp, "XdndActionMove", False);
	ECORE_X_ATOM_XDND_ACTION_PRIVATE      = XInternAtom(_ecore_x_disp, "XdndActionPrivate", False);
	ECORE_X_ATOM_XDND_ACTION_ASK          = XInternAtom(_ecore_x_disp, "XdndActionAsk", False);
	ECORE_X_ATOM_XDND_ACTION_LIST         = XInternAtom(_ecore_x_disp, "XdndActionList", False);
	ECORE_X_ATOM_XDND_ACTION_LINK         = XInternAtom(_ecore_x_disp, "XdndActionLink", False);
	ECORE_X_ATOM_XDND_ACTION_DESCRIPTION  = XInternAtom(_ecore_x_disp, "XdndActionDescription", False);
	ECORE_X_ATOM_XDND_PROXY               = XInternAtom(_ecore_x_disp, "XdndProxy", False);
	ECORE_X_ATOM_XDND_STATUS              = XInternAtom(_ecore_x_disp, "XdndStatus", False);
	ECORE_X_ATOM_XDND_LEAVE               = XInternAtom(_ecore_x_disp, "XdndLeave", False);
	ECORE_X_ATOM_XDND_DROP                = XInternAtom(_ecore_x_disp, "XdndDrop", False);
	ECORE_X_ATOM_XDND_FINISHED            = XInternAtom(_ecore_x_disp, "XdndFinished", False);

	/* Initialize the globally defined xdnd atoms */
	ECORE_X_DND_ACTION_COPY                = ECORE_X_ATOM_XDND_ACTION_COPY;
	ECORE_X_DND_ACTION_MOVE                = ECORE_X_ATOM_XDND_ACTION_MOVE;
	ECORE_X_DND_ACTION_LINK                = ECORE_X_ATOM_XDND_ACTION_LINK;
	ECORE_X_DND_ACTION_ASK                 = ECORE_X_ATOM_XDND_ACTION_ASK;
	ECORE_X_DND_ACTION_PRIVATE             = ECORE_X_ATOM_XDND_ACTION_PRIVATE;
     }

   _ecore_x_dnd_init_count++;
}

void
_ecore_x_dnd_shutdown(void)
{
   _ecore_x_dnd_init_count--;
   if (_ecore_x_dnd_init_count > 0)
     return;

   if (_source)
     free(_source);
   _source = NULL;

   if (_target)
     free(_target);
   _target = NULL;

   _ecore_x_dnd_init_count = 0;
}

void
ecore_x_dnd_aware_set(Ecore_X_Window win, int on)
{
   Ecore_X_Atom prop_data = ECORE_X_DND_VERSION;

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
	newset = calloc(num - 1, sizeof(Ecore_X_Atom));
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

void
ecore_x_dnd_types_set(Ecore_X_Window win, char **types, unsigned int num_types)
{
   Ecore_X_Atom      *newset = NULL;
   int               i;
   unsigned char     *data = NULL;

   if (!num_types)
     {
	ecore_x_window_prop_property_del(win, ECORE_X_ATOM_XDND_TYPE_LIST);
     }
   else
     {
	newset = calloc(num_types, sizeof(Ecore_X_Atom));
	if (!newset) return;
	data = (unsigned char *)newset;
	for (i = 0; i < num_types; i++)
	  newset[i] = ecore_x_atom_get(types[i]);
	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_XDND_TYPE_LIST,
					 XA_ATOM, 32, data, num_types);
	free(newset);
     }
}

Ecore_X_DND_Source *
_ecore_x_dnd_source_get(void)
{
   return _source;
}

Ecore_X_DND_Target *
_ecore_x_dnd_target_get(void)
{
   return _target;
}

int
ecore_x_dnd_begin(Ecore_X_Window source, unsigned char *data, int size)
{

   if (!ecore_x_dnd_version_get(source))
     return 0;

   /* Take ownership of XdndSelection */
   if (!ecore_x_selection_xdnd_set(source, data, size))
     return 0;

   _source->win = source;
   ecore_x_window_ignore_set(_source->win, 1);
   _source->state = ECORE_X_DND_SOURCE_DRAGGING;
   _source->time = _ecore_x_event_last_time;

   /* Default Accepted Action: ask */
   _source->action = ECORE_X_ATOM_XDND_ACTION_COPY;
   _source->accepted_action = None;
   return 1;
}

int
ecore_x_dnd_drop(void)
{
   XEvent xev;
   int status = 0;

   if (_source->dest)
     {
	xev.xany.type = ClientMessage;
	xev.xany.display = _ecore_x_disp;
	xev.xclient.format = 32;
	xev.xclient.window = _source->dest;

	if (_source->will_accept)
	  {
	     xev.xclient.message_type = ECORE_X_ATOM_XDND_DROP;
	     xev.xclient.data.l[0] = _source->win;
	     xev.xclient.data.l[1] = 0;
	     xev.xclient.data.l[2] = _source->time;
	     XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
	     _source->state = ECORE_X_DND_SOURCE_DROPPED;
	     status = 1;
	  }
	else
	  {
	     xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
	     xev.xclient.data.l[0] = _source->win;
	     xev.xclient.data.l[1] = 0;
	     XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
	     _source->state = ECORE_X_DND_SOURCE_IDLE;
	  }
     }
   else
     {
	/* Dropping on nothing */
	ecore_x_selection_xdnd_clear();
	_source->state = ECORE_X_DND_SOURCE_IDLE;
     }
   ecore_x_window_ignore_set(_source->win, 0);

   _source->dest = None; 


   return status;
}

void
ecore_x_dnd_send_status(int will_accept, int suppress, Ecore_X_Rectangle rectangle, Ecore_X_Atom action)
{
   XEvent xev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
     return;

   memset(&xev, 0, sizeof(XEvent));

   _target->will_accept = will_accept;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.message_type = ECORE_X_ATOM_XDND_STATUS;
   xev.xclient.format = 32;
   xev.xclient.window = _target->source;

   xev.xclient.data.l[0] = _target->win;
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
	_target->accepted_action = action;
     }
   else
     {
	xev.xclient.data.l[4] = None;
	_target->accepted_action = action;
     }

   XSendEvent(_ecore_x_disp, _target->source, False, 0, &xev);
}

void
ecore_x_dnd_send_finished(void)
{
   XEvent   xev;

   if (_target->state == ECORE_X_DND_TARGET_IDLE)
     return;

   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.message_type = ECORE_X_ATOM_XDND_FINISHED;
   xev.xclient.format = 32;
   xev.xclient.window = _target->source;

   xev.xclient.data.l[0] = _target->win;
   xev.xclient.data.l[1] = 0;
   xev.xclient.data.l[2] = 0;
   if (_target->will_accept)
     {
	xev.xclient.data.l[1] |= 0x1UL;
	xev.xclient.data.l[2] = _target->accepted_action;
     }
   XSendEvent(_ecore_x_disp, _target->source, False, 0, &xev);

   _target->state = ECORE_X_DND_TARGET_IDLE;
}

void
_ecore_x_dnd_drag(int x, int y)
{
   XEvent          xev;
   Ecore_X_Window  win;
   Ecore_X_Window *skip;
   int             num;

   if (_source->state != ECORE_X_DND_SOURCE_DRAGGING)
     return;

   /* Preinitialize XEvent struct */
   memset(&xev, 0, sizeof(XEvent));
   xev.xany.type = ClientMessage;
   xev.xany.display = _ecore_x_disp;
   xev.xclient.format = 32;

   /* Attempt to find a DND-capable window under the cursor */
   skip = ecore_x_window_ignore_list(&num);
   win = ecore_x_window_at_xy_with_skip_get(x, y, skip, num);
   while ((win) && !(ecore_x_dnd_version_get(win)))
     win = ecore_x_window_parent_get(win);

   /* Send XdndLeave to current destination window if we have left it */
   if ((_source->dest) && (win != _source->dest))
     {
	xev.xclient.window = _source->dest;
	xev.xclient.message_type = ECORE_X_ATOM_XDND_LEAVE;
	xev.xclient.data.l[0] = _source->win;
	xev.xclient.data.l[1] = 0;

	XSendEvent(_ecore_x_disp, _source->dest, False, 0, &xev);
	_source->suppress = 0;
     }

   if (win)
     {
	int x1, x2, y1, y2;

	_source->version = MIN(ECORE_X_DND_VERSION,
			     ecore_x_dnd_version_get(win));
	if (win != _source->dest)
	  {
	     int i, num;
	     unsigned char *data;
	     Ecore_X_Atom *types;

	     ecore_x_window_prop_property_get(_source->win, ECORE_X_ATOM_XDND_TYPE_LIST,
					      XA_ATOM, 32, &data, &num);
	     types = (Ecore_X_Atom *)data;

	     /* Entered new window, send XdndEnter */
	     xev.xclient.window = win;
	     xev.xclient.message_type = ECORE_X_ATOM_XDND_ENTER;
	     xev.xclient.data.l[0] = _source->win;
	     xev.xclient.data.l[1] = 0;
	     if (num > 3)
	       xev.xclient.data.l[1] |= 0x1UL;
	     else
	       xev.xclient.data.l[1] &= 0xfffffffeUL;
	     xev.xclient.data.l[1] |= ((unsigned long) _source->version) << 24;

	     for (i = 2; i < 5; i++)
	       xev.xclient.data.l[i] = 0;
	     for (i = 0; i < MIN(num, 3); ++i)
	       xev.xclient.data.l[i + 2] = types[i];
	     XFree(data);
	     XSendEvent(_ecore_x_disp, win, False, 0, &xev);
	     _source->await_status = 0;
	     _source->will_accept = 0;
	  }

	/* Determine if we're still in the rectangle from the last status */
	x1 = _source->rectangle.x;
	x2 = _source->rectangle.x + _source->rectangle.width;
	y1 = _source->rectangle.y;
	y2 = _source->rectangle.y + _source->rectangle.height;

	if ((!_source->await_status) ||
	    (!_source->suppress) ||
	    ((x < x1) || (x > x2) || (y < y1) || (y > y2)))
	  {
	     xev.xclient.window = win;
	     xev.xclient.message_type = ECORE_X_ATOM_XDND_POSITION;
	     xev.xclient.data.l[0] = _source->win;
	     xev.xclient.data.l[1] = 0; /* Reserved */
	     xev.xclient.data.l[2] = ((x << 16) & 0xffff0000) | (y & 0xffff);
	     xev.xclient.data.l[3] = _source->time; /* Version 1 */
	     xev.xclient.data.l[4] = _source->action; /* Version 2, Needs to be pre-set */
	     XSendEvent(_ecore_x_disp, win, False, 0, &xev);

	     _source->await_status = 1;
	  }
     }

   _source->dest = win;
}

