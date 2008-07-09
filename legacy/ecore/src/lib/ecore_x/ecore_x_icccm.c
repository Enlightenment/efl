/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * Various ICCCM related functions.
 * 
 * This is ALL the code involving anything ICCCM related. for both WM and
 * client.
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"


EAPI void
ecore_x_icccm_init(void)
{
}

EAPI void
ecore_x_icccm_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state)
{
   unsigned long       c[2];

   if (state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
      c[0] = WithdrawnState;
   else if (state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
      c[0] = NormalState;
   else if (state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
      c[0] = IconicState;
   c[1] = None;
   XChangeProperty(_ecore_x_disp, win, ECORE_X_ATOM_WM_STATE,
		   ECORE_X_ATOM_WM_STATE, 32, PropModeReplace,
		   (unsigned char *)c, 2);
}

EAPI Ecore_X_Window_State_Hint
ecore_x_icccm_state_get(Ecore_X_Window win)
{
   unsigned char *prop_ret = NULL;
   Atom           type_ret;
   unsigned long  bytes_after, num_ret;
   int            format_ret;
   Ecore_X_Window_State_Hint hint;

   hint = ECORE_X_WINDOW_STATE_HINT_NONE;
   XGetWindowProperty(_ecore_x_disp, win, ECORE_X_ATOM_WM_STATE,
		      0, 0x7fffffff, False, ECORE_X_ATOM_WM_STATE,
		      &type_ret, &format_ret, &num_ret, &bytes_after,
		      &prop_ret);
   if ((prop_ret) && (num_ret == 2))
     {
	if (prop_ret[0] == WithdrawnState)
	  hint = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
	else if (prop_ret[0] == NormalState)
	  hint = ECORE_X_WINDOW_STATE_HINT_NORMAL;
	else if (prop_ret[0] == IconicState)
	  hint = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }

   if (prop_ret)
     XFree(prop_ret);
   
   return hint;
}

EAPI void
ecore_x_icccm_delete_window_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
				 ECORE_X_EVENT_MASK_NONE,
				 ECORE_X_ATOM_WM_DELETE_WINDOW,
				 t, 0, 0, 0);
}

EAPI void
ecore_x_icccm_take_focus_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
				 ECORE_X_EVENT_MASK_NONE,
				 ECORE_X_ATOM_WM_TAKE_FOCUS,
				 t, 0, 0, 0);
}

EAPI void
ecore_x_icccm_save_yourself_send(Ecore_X_Window win, Ecore_X_Time t)
{
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
				 ECORE_X_EVENT_MASK_NONE,
				 ECORE_X_ATOM_WM_SAVE_YOURSELF,
				 t, 0, 0, 0);
}

EAPI void
ecore_x_icccm_move_resize_send(Ecore_X_Window win, int x, int y, int w, int h)
{
   XEvent              ev;

   ev.type = ConfigureNotify;
   ev.xconfigure.display = _ecore_x_disp;
   ev.xconfigure.event = win;
   ev.xconfigure.window = win;
   ev.xconfigure.x = x;
   ev.xconfigure.y = y;
   ev.xconfigure.width = w;
   ev.xconfigure.height = h;
   ev.xconfigure.border_width = 0;
   ev.xconfigure.above = None;
   ev.xconfigure.override_redirect = False;
   XSendEvent(_ecore_x_disp, win, False, StructureNotifyMask, &ev);
}

EAPI void
ecore_x_icccm_hints_set(Ecore_X_Window win,
			int accepts_focus,
			Ecore_X_Window_State_Hint initial_state,
			Ecore_X_Pixmap icon_pixmap,
			Ecore_X_Pixmap icon_mask,
			Ecore_X_Window icon_window,
			Ecore_X_Window window_group, int is_urgent)
{
   XWMHints           *hints;

   hints = XAllocWMHints();
   if (!hints)
      return;

   hints->flags = InputHint | StateHint;
   hints->input = accepts_focus;
   if (initial_state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
      hints->initial_state = WithdrawnState;
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
      hints->initial_state = NormalState;
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
      hints->initial_state = IconicState;
   if (icon_pixmap != 0)
     {
	hints->icon_pixmap = icon_pixmap;
	hints->flags |= IconPixmapHint;
     }
   if (icon_mask != 0)
     {
	hints->icon_mask = icon_mask;
	hints->flags |= IconMaskHint;
     }
   if (icon_window != 0)
     {
	hints->icon_window = icon_window;
	hints->flags |= IconWindowHint;
     }
   if (window_group != 0)
     {
	hints->window_group = window_group;
	hints->flags |= WindowGroupHint;
     }
   if (is_urgent)
      hints->flags |= XUrgencyHint;
   XSetWMHints(_ecore_x_disp, win, hints);
   XFree(hints);
}

EAPI int
ecore_x_icccm_hints_get(Ecore_X_Window win,
			int *accepts_focus,
			Ecore_X_Window_State_Hint *initial_state,
			Ecore_X_Pixmap *icon_pixmap,
			Ecore_X_Pixmap *icon_mask,
			Ecore_X_Window *icon_window,
			Ecore_X_Window *window_group, int *is_urgent)
{
   XWMHints *hints;

   if (accepts_focus)
      *accepts_focus = 1;
   if (initial_state)
      *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
   if (icon_pixmap)
      *icon_pixmap = 0;
   if (icon_mask)
      *icon_mask = 0;
   if (icon_window)
      *icon_window = 0;
   if (window_group)
      *window_group = 0;
   if (is_urgent)
      *is_urgent = 0;
   hints = XGetWMHints(_ecore_x_disp, win);
   if (hints)
     {
	if ((hints->flags & InputHint) && (accepts_focus))
	  {
	     if (hints->input)
		*accepts_focus = 1;
	     else
		*accepts_focus = 0;
	  }
	if ((hints->flags & StateHint) && (initial_state))
	  {
	     if (hints->initial_state == WithdrawnState)
		*initial_state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
	     else if (hints->initial_state == NormalState)
		*initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
	     else if (hints->initial_state == IconicState)
		*initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
	  }
	if ((hints->flags & IconPixmapHint) && (icon_pixmap))
	  {
	     *icon_pixmap = hints->icon_pixmap;
	  }
	if ((hints->flags & IconMaskHint) && (icon_mask))
	  {
	     *icon_mask = hints->icon_mask;
	  }
	if ((hints->flags & IconWindowHint) && (icon_window))
	  {
	     *icon_window = hints->icon_window;
	  }
	if ((hints->flags & WindowGroupHint) && (window_group))
	  {
	     *window_group = hints->window_group;
	  }
	if ((hints->flags & XUrgencyHint) && (is_urgent))
	  {
	     *is_urgent = 1;
	  }
	XFree(hints);
	return 1;
     }
   return 0;
}

EAPI void
ecore_x_icccm_size_pos_hints_set(Ecore_X_Window win,
				 int request_pos,
				 Ecore_X_Gravity gravity,
				 int min_w, int min_h,
				 int max_w, int max_h,
				 int base_w, int base_h,
				 int step_x, int step_y,
				 double min_aspect, double max_aspect)
{
   XSizeHints  hint;
   long        mask;

   if (!XGetWMNormalHints(_ecore_x_disp, win, &hint, &mask))
     {
	memset(&hint, 0, sizeof(XSizeHints));
     }

   hint.flags = 0;
   if (request_pos)
     {
	hint.flags |= USPosition;
     }
   if (gravity != ECORE_X_GRAVITY_NW)
     {
	hint.flags |= PWinGravity;
	hint.win_gravity = gravity;
     }
   if ((min_w > 0) || (min_h > 0))
     {
	hint.flags |= PMinSize;
	hint.min_width = min_w;
	hint.min_height = min_h;
     }
   if ((max_w > 0) || (max_h > 0))
     {
	hint.flags |= PMaxSize;
	hint.max_width = max_w;
	hint.max_height = max_h;
     }
   if ((base_w > 0) || (base_h > 0))
     {
	hint.flags |= PBaseSize;
	hint.base_width = base_w;
	hint.base_height = base_h;
     }
   if ((step_x > 1) || (step_y > 1))
     {
	hint.flags |= PResizeInc;
	hint.width_inc = step_x;
	hint.height_inc = step_y;
     }
   if ((min_aspect > 0.0) || (max_aspect > 0.0))
     {
	hint.flags |= PAspect;
	hint.min_aspect.x = min_aspect * 10000;
	hint.min_aspect.y = 10000;
	hint.max_aspect.x = max_aspect * 10000;
	hint.max_aspect.y = 10000;
     }
   XSetWMNormalHints(_ecore_x_disp, win, &hint);
}

EAPI int
ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win,
				 int *request_pos,
				 Ecore_X_Gravity *gravity,
				 int *min_w, int *min_h,
				 int *max_w, int *max_h,
				 int *base_w, int *base_h,
				 int *step_x, int *step_y,
				 double *min_aspect, double *max_aspect)
{
   XSizeHints          hint;
   long                mask;

   int                 minw = 0, minh = 0;
   int                 maxw = 32767, maxh = 32767;
   int                 basew = -1, baseh = -1;
   int                 stepx = -1, stepy = -1;
   double              mina = 0.0, maxa = 0.0;

   if (!XGetWMNormalHints(_ecore_x_disp, win, &hint, &mask))
     return 0;

   if ((hint.flags & USPosition) || ((hint.flags & PPosition)))
     {
	if (request_pos)
	   *request_pos = 1;
     }
   else
     {
	if (request_pos)
	   *request_pos = 0;
     }
   if (hint.flags & PWinGravity)
     {
	if (gravity)
	   *gravity = hint.win_gravity;
     }
   else
     {
	if (gravity)
	   *gravity = ECORE_X_GRAVITY_NW;
     }
   if (hint.flags & PMinSize)
     {
	minw = hint.min_width;
	minh = hint.min_height;
     }
   if (hint.flags & PMaxSize)
     {
	maxw = hint.max_width;
	maxh = hint.max_height;
	if (maxw < minw)
	   maxw = minw;
	if (maxh < minh)
	   maxh = minh;
     }
   if (hint.flags & PBaseSize)
     {
	basew = hint.base_width;
	baseh = hint.base_height;
	if (basew > minw)
	   minw = basew;
	if (baseh > minh)
	   minh = baseh;
     }
   if (hint.flags & PResizeInc)
     {
	stepx = hint.width_inc;
	stepy = hint.height_inc;
	if (stepx < 1)
	   stepx = 1;
	if (stepy < 1)
	   stepy = 1;
     }
   if (hint.flags & PAspect)
     {
	if (hint.min_aspect.y > 0)
	   mina = ((double)hint.min_aspect.x) / ((double)hint.min_aspect.y);
	if (hint.max_aspect.y > 0)
	   maxa = ((double)hint.max_aspect.x) / ((double)hint.max_aspect.y);
     }
   if (min_w)
      *min_w = minw;
   if (min_h)
      *min_h = minh;
   if (max_w)
      *max_w = maxw;
   if (max_h)
      *max_h = maxh;
   if (base_w)
      *base_w = basew;
   if (base_h)
      *base_h = baseh;
   if (step_x)
      *step_x = stepx;
   if (step_y)
      *step_y = stepy;
   if (min_aspect)
      *min_aspect = mina;
   if (max_aspect)
      *max_aspect = maxa;
   return 1;
}

EAPI void
ecore_x_icccm_title_set(Ecore_X_Window win, const char *t)
{
   char               *list[1];
   XTextProperty       xprop;
   int                 ret;

   xprop.value = NULL;
#ifdef X_HAVE_UTF8_STRING
   list[0] = strdup(t);
   ret =
      Xutf8TextListToTextProperty(_ecore_x_disp, list, 1, XUTF8StringStyle,
				  &xprop);
#else
   list[0] = strdup(t);
   ret =
      XmbTextListToTextProperty(_ecore_x_disp, list, 1, XStdICCTextStyle,
				&xprop);
#endif
   if (ret >= Success)
     {
	XSetWMName(_ecore_x_disp, win, &xprop);
	if (xprop.value) XFree(xprop.value);
     }
   else
     {
	if (XStringListToTextProperty(list, 1, &xprop) >= Success)
	  {
	     XSetWMName(_ecore_x_disp, win, &xprop);
	     if (xprop.value) XFree(xprop.value);
	  }
     }
   free(list[0]);
}

EAPI char               *
ecore_x_icccm_title_get(Ecore_X_Window win)
{
   XTextProperty       xprop;

   xprop.value = NULL;
   if (XGetWMName(_ecore_x_disp, win, &xprop) >= Success)
     {
	if (xprop.value)
	  {
	     char              **list = NULL;
	     char               *t = NULL;
	     int                 num = 0;
	     int                 ret;

	     if (xprop.encoding == ECORE_X_ATOM_UTF8_STRING)
	       {
		  t = strdup((char *)xprop.value);
	       }
	     else
	       {

		  /* convert to utf8 */
#ifdef X_HAVE_UTF8_STRING
		  ret = Xutf8TextPropertyToTextList(_ecore_x_disp, &xprop,
						    &list, &num);
#else
		  ret = XmbTextPropertyToTextList(_ecore_x_disp, &xprop,
						  &list, &num);
#endif

		  if ((ret == XLocaleNotSupported) ||
		      (ret == XNoMemory) || (ret == XConverterNotFound))
		    {
		       t = strdup((char *)xprop.value);
		    }
		  else if ((ret >= Success) && (num > 0))
		    {
		       t = strdup(list[0]);
		    }
		  if (list)
		    XFreeStringList(list);
	       }
	     
	     if (xprop.value) XFree(xprop.value);
	     return t;
	  }
     }
   return NULL;
}

/**
 * Set or unset a wm protocol property.
 * @param win The Window
 * @param protocol The protocol to enable/disable
 * @param on On/Off
 */
EAPI void
ecore_x_icccm_protocol_set(Ecore_X_Window win,
			   Ecore_X_WM_Protocol protocol, int on)
{
   Atom               *protos = NULL;
   Atom                proto;
   int                 protos_count = 0;
   int                 already_set = 0;
   int                 i;

   /* Check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
      return;

   proto = _ecore_x_atoms_wm_protocols[protocol];

   if (!XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count))
     {
	protos = NULL;
	protos_count = 0;
     }

   for (i = 0; i < protos_count; i++)
     {
	if (protos[i] == proto)
	  {
	     already_set = 1;
	     break;
	  }
     }

   if (on)
     {
	Atom *new_protos = NULL;

	if (already_set)
	   goto leave;
	new_protos = malloc((protos_count + 1) * sizeof(Atom));
	if (!new_protos)
	   goto leave;
	for (i = 0; i < protos_count; i++)
	   new_protos[i] = protos[i];
	new_protos[protos_count] = proto;
	XSetWMProtocols(_ecore_x_disp, win, new_protos, protos_count + 1);
	free(new_protos);
     }
   else
     {
	if (!already_set)
	   goto leave;
	for (i = 0; i < protos_count; i++)
	  {
	     if (protos[i] == proto)
	       {
		  int                 j;

		  for (j = i + 1; j < protos_count; j++)
		     protos[j - 1] = protos[j];
		  if (protos_count > 1)
		     XSetWMProtocols(_ecore_x_disp, win, protos,
				     protos_count - 1);
		  else
		     XDeleteProperty(_ecore_x_disp, win,
				     ECORE_X_ATOM_WM_PROTOCOLS);
		  goto leave;
	       }
	  }
     }

 leave:
   if (protos)
     XFree(protos);

}

/**
 * Determines whether a protocol is set for a window.
 * @param win The Window
 * @param protocol The protocol to query
 * @return 1 if the protocol is set, else 0.
 */
EAPI int
ecore_x_icccm_protocol_isset(Ecore_X_Window win, Ecore_X_WM_Protocol protocol)
{
   Atom                proto, *protos = NULL;
   int                 i, ret = 0, protos_count = 0;

   /* check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
      return 0;

   proto = _ecore_x_atoms_wm_protocols[protocol];

   if (!XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count))
      return 0;

   for (i = 0; i < protos_count; i++)
      if (protos[i] == proto)
	{
	   ret = 1;
	   break;
	}

   if (protos) XFree(protos);
   return ret;

}

/**
 * Set a window name & class.
 * @param win The window
 * @param n The name string
 * @param c The class string
 * 
 * Set a window name * class
 */
EAPI void
ecore_x_icccm_name_class_set(Ecore_X_Window win, const char *n, const char *c)
{
   XClassHint         *xch;

   xch = XAllocClassHint();
   if (!xch)
      return;
   xch->res_name = (char *)n;
   xch->res_class = (char *)c;
   XSetClassHint(_ecore_x_disp, win, xch);
   XFree(xch);
}

/**
 * Get a window name & class.
 * @param win The window
 * @param n The name string
 * @param c The class string
 * 
 * Get a window name * class
 */
EAPI void
ecore_x_icccm_name_class_get(Ecore_X_Window win, char **n, char **c)
{
   XClassHint          xch;
   
   if (n) *n = NULL;
   if (c) *c = NULL;
   xch.res_name = NULL;
   xch.res_class = NULL;
   if (XGetClassHint(_ecore_x_disp, win, &xch))
     {
	if (n)
	  {
	     if (xch.res_name) *n = strdup(xch.res_name);
	  }
	if (c)
	  {
	     if (xch.res_class) *c = strdup(xch.res_class);
	  }
	XFree(xch.res_name);
	XFree(xch.res_class);
     }
}

/**
 * Get a window client machine string.
 * @param win The window
 * @return The windows client machine string
 * 
 * Return the client machine of a window. String must be free'd when done with.
 */
EAPI char               *
ecore_x_icccm_client_machine_get(Ecore_X_Window win)
{
   char               *name;

   name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_CLIENT_MACHINE);
   return name;
}

/**
 * Sets the WM_COMMAND property for @a win.
 * 
 * @param win  The window.
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
EAPI void
ecore_x_icccm_command_set(Ecore_X_Window win, int argc, char **argv)
{
   XSetCommand(_ecore_x_disp, win, argv, argc);
}

/**
 * Get the WM_COMMAND property for @a win.
 *
 * Return the command of a window. String must be free'd when done with.
 *
 * @param win  The window.
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
EAPI void
ecore_x_icccm_command_get(Ecore_X_Window win, int *argc, char ***argv)
{
   int i, c;
   char **v;

   if (argc) *argc = 0;
   if (argv) *argv = NULL;

   if (!XGetCommand(_ecore_x_disp, win, &v, &c))
     return;
   if (c < 1)
     {
	if (v)
	  XFreeStringList(v);
       	return;
     }

   if (argc) *argc = c;
   if (argv)
     {
	(*argv) = malloc(c * sizeof(char *));
	if (!*argv)
	  { 
	     XFreeStringList(v);
	     if (argc) *argc = 0;
	     return;
	  }
	for (i = 0; i < c; i++)
	  {
	     if (v[i])
	       (*argv)[i] = strdup(v[i]);
	     else
	       (*argv)[i] = strdup("");
	  }
     }
   XFreeStringList(v);
}

/**
 * Set a window icon name.
 * @param win The window
 * @param t The icon name string
 * 
 * Set a window icon name
 */
EAPI void
ecore_x_icccm_icon_name_set(Ecore_X_Window win, const char *t)
{
   char               *list[1];
   XTextProperty       xprop;
   int                 ret;

   xprop.value = NULL;
#ifdef X_HAVE_UTF8_STRING
   list[0] = strdup(t);
   ret = Xutf8TextListToTextProperty(_ecore_x_disp, list, 1,
				     XUTF8StringStyle, &xprop);
#else
   list[0] = strdup(t);
   ret = XmbTextListToTextProperty(_ecore_x_disp, list, 1,
				   XStdICCTextStyle, &xprop);
#endif
   if (ret >= Success)
     {
	XSetWMIconName(_ecore_x_disp, win, &xprop);
	if (xprop.value) XFree(xprop.value);
     }
   else
     {
	if (XStringListToTextProperty(list, 1, &xprop) >= Success)
	  {
	     XSetWMIconName(_ecore_x_disp, win, &xprop);
	     if (xprop.value) XFree(xprop.value);
	  }
     }
   free(list[0]);
}

/**
 * Get a window icon name.
 * @param win The window
 * @return The windows icon name string
 * 
 * Return the icon name of a window. String must be free'd when done with.
 */
EAPI char               *
ecore_x_icccm_icon_name_get(Ecore_X_Window win)
{
   XTextProperty       xprop;

   xprop.value = NULL;
   if (XGetWMIconName(_ecore_x_disp, win, &xprop) >= Success)
     {
	if (xprop.value)
	  {
	     char              **list = NULL;
	     char               *t = NULL;
	     int                 num = 0;
	     int                 ret;

	     if (xprop.encoding == ECORE_X_ATOM_UTF8_STRING)
	       {
		  t = strdup((char *)xprop.value);
	       }
	     else
	       {

		  /* convert to utf8 */
#ifdef X_HAVE_UTF8_STRING
		  ret = Xutf8TextPropertyToTextList(_ecore_x_disp, &xprop,
						    &list, &num);
#else
		  ret = XmbTextPropertyToTextList(_ecore_x_disp, &xprop,
						  &list, &num);
#endif

		  if ((ret == XLocaleNotSupported) ||
		      (ret == XNoMemory) || (ret == XConverterNotFound))
		    {
		       t = strdup((char *)xprop.value);
		    }
		  else if (ret >= Success)
		    {
		       if ((num >= 1) && (list))
			 {
			    t = strdup(list[0]);
			 }
		       if (list)
			 XFreeStringList(list);
		    }
	       }
	     
	     if (xprop.value) XFree(xprop.value);
	     return t;
	  }
     }
   return NULL;
}

/**
 * Add a subwindow to the list of windows that need a different colormap installed.
 * @param win The toplevel window
 * @param subwin The subwindow to be added to the colormap windows list
 */
EAPI void
ecore_x_icccm_colormap_window_set(Ecore_X_Window win, Ecore_X_Window subwin)
{
   int                 num = 0, i;
   unsigned char      *old_data = NULL;
   unsigned char      *data = NULL;
   Window             *oldset = NULL;
   Window             *newset = NULL;

   if (!ecore_x_window_prop_property_get(win,
					 ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
					 XA_WINDOW, 32, &old_data, &num))
     {
	newset = calloc(1, sizeof(Window));
	if (!newset)
	   return;
	newset[0] = subwin;
	num = 1;
	data = (unsigned char *)newset;
     }
   else
     {
	newset = calloc(num + 1, sizeof(Window));
	oldset = (Window *) old_data;
	if (!newset)
	   return;
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == subwin)
	       {
		  if (old_data) XFree(old_data);
		  old_data = NULL;
		  free(newset);
		  return;
	       }

	     newset[i] = oldset[i];
	  }

	newset[num++] = subwin;
	if (old_data) XFree(old_data);
	data = (unsigned char *)newset;
     }

   ecore_x_window_prop_property_set(win,
				    ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
				    XA_WINDOW, 32, data, num);
   free(newset);
}

/**
 * Remove a window from the list of colormap windows.
 * @param win The toplevel window
 * @param subwin The window to be removed from the colormap window list.
 */
EAPI void
ecore_x_icccm_colormap_window_unset(Ecore_X_Window win, Ecore_X_Window subwin)
{
   int                 num = 0, i, j, k = 0;
   unsigned char      *old_data = NULL;
   unsigned char      *data = NULL;
   Window             *oldset = NULL;
   Window             *newset = NULL;

   if (!ecore_x_window_prop_property_get(win,
					 ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
					 XA_WINDOW, 32, &old_data, &num))
      return;

   oldset = (Window *) old_data;
   for (i = 0; i < num; i++)
     {
	if (oldset[i] == subwin)
	  {
	     if (num == 1)
	       {
		  XDeleteProperty(_ecore_x_disp,
				  win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS);
		  if (old_data) XFree(old_data);
		  old_data = NULL;
		  return;
	       }
	     else
	       {
		  newset = calloc(num - 1, sizeof(Window));
		  data = (unsigned char *)newset;
		  for (j = 0; j < num; ++j)
		     if (oldset[j] != subwin)
			newset[k++] = oldset[j];
		  ecore_x_window_prop_property_set(win,
						   ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
						   XA_WINDOW, 32, data, k);
		  if (old_data) XFree(old_data);
		  old_data = NULL;
		  free(newset);
		  return;
	       }
	  }
     }

   if (old_data) XFree(old_data);
}

/**
 * Specify that a window is transient for another top-level window and should be handled accordingly.
 * @param win the transient window
 * @param forwin the toplevel window
 */
EAPI void
ecore_x_icccm_transient_for_set(Ecore_X_Window win, Ecore_X_Window forwin)
{
   XSetTransientForHint(_ecore_x_disp, win, forwin);
}

/**
 * Remove the transient_for setting from a window.
 * @param The window
 */
EAPI void
ecore_x_icccm_transient_for_unset(Ecore_X_Window win)
{
   XDeleteProperty(_ecore_x_disp, win, ECORE_X_ATOM_WM_TRANSIENT_FOR);
}

/**
 * Get the window this window is transient for, if any.
 * @param win The window to check
 * @return The window ID of the top-level window, or 0 if the property does not exist.
 */
EAPI Ecore_X_Window
ecore_x_icccm_transient_for_get(Ecore_X_Window win)
{
   Window              forwin;

   if (XGetTransientForHint(_ecore_x_disp, win, &forwin))
      return (Ecore_X_Window) forwin;
   else
      return 0;

}

/**
 * Set the window role hint.
 * @param win The window
 * @param role The role string
 */
EAPI void
ecore_x_icccm_window_role_set(Ecore_X_Window win, const char *role)
{
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_WINDOW_ROLE,
				  (char *)role);
}

/**
 * Get the window role.
 * @param win The window
 * @return The window's role string.
 */
EAPI char               *
ecore_x_icccm_window_role_get(Ecore_X_Window win)
{
   return ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_WINDOW_ROLE);
}

/**
 * Set the window's client leader.
 * @param win The window
 * @param l The client leader window
 *
 * All non-transient top-level windows created by an app other than
 * the main window must have this property set to the app's main window.
 */
EAPI void
ecore_x_icccm_client_leader_set(Ecore_X_Window win, Ecore_X_Window l)
{
   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_WM_CLIENT_LEADER,
				  &l, 1);
}

/**
 * Get the window's client leader.
 * @param win The window
 * @return The window's client leader window, or 0 if unset */
EAPI Ecore_X_Window
ecore_x_icccm_client_leader_get(Ecore_X_Window win)
{
   Ecore_X_Window      l;

   if (ecore_x_window_prop_window_get(win, ECORE_X_ATOM_WM_CLIENT_LEADER,
				      &l, 1) > 0)
      return l;
   return 0;
}

EAPI void
ecore_x_icccm_iconic_request_send(Ecore_X_Window win, Ecore_X_Window root)
{
   XEvent xev;

   if (!win) return;
   if (!root) root = DefaultRootWindow(_ecore_x_disp);

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.format = 32;
   xev.xclient.message_type = ECORE_X_ATOM_WM_CHANGE_STATE;
   xev.xclient.data.l[0] = IconicState;

   XSendEvent(_ecore_x_disp, root, False,
              SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

/* FIXME: there are older E hints, gnome hints and mwm hints and new netwm */
/*        hints. each should go in their own file/section so we know which */
/*        is which. also older kde hints too. we should try support as much */
/*        as makese sense to support */
