/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"
#include <inttypes.h>
#include <limits.h>

static Ecore_X_Atom _ecore_x_window_prop_state_atom_get(Ecore_X_Window_State s);

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
Ecore_X_Atom
ecore_x_window_prop_any_type(void)
{
   return AnyPropertyType;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_property_set(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, void *data, int number)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (size != 32)
     XChangeProperty(_ecore_x_disp, win, property, type, size, PropModeReplace,
		     (unsigned char *)data, number);
   else
     {
	unsigned long *dat;
	int            i, *ptr;
	
	dat = malloc(sizeof(unsigned long) * number);
	if (dat)
	  {
	     for (ptr = (int *)data, i = 0; i < number; i++) dat[i] = ptr[i];
	     XChangeProperty(_ecore_x_disp, win, property, type, size, 
			     PropModeReplace, (unsigned char *)dat, number);
	     free(dat);
	  }
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
int
ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, unsigned char **data, int *num)
{
   Atom type_ret = 0;
   int ret, size_ret = 0;
   unsigned long num_ret = 0, bytes = 0, i;
   unsigned char *prop_ret = NULL;

   /* make sure these are initialized */
   if (num) *num = 0;

   if (data)
	  *data = NULL;
   else /* we can't store the retrieved data, so just return */
	  return 0;

   if (!win) win = DefaultRootWindow(_ecore_x_disp);

   ret = XGetWindowProperty(_ecore_x_disp, win, property, 0, LONG_MAX,
                            False, type, &type_ret, &size_ret,
                            &num_ret, &bytes, &prop_ret);

   if (ret != Success)
	return 0;

   if (size != size_ret || !num_ret) {
      XFree(prop_ret);
	  return 0;
   }

   if (!(*data = malloc(num_ret * size / 8))) {
	   XFree(prop_ret);
	   return 0;
   }

   for (i = 0; i < num_ret; i++)
     switch (size) {
	case 8:
	   (*data)[i] = prop_ret[i];
	   break;
	case 16:
	   ((uint16_t *) *data)[i] = ((uint16_t *) prop_ret)[i];
	   break;
	case 32:
	   ((uint32_t *) *data)[i] = ((uint32_t *) prop_ret)[i];
	   break;
     }

   XFree(prop_ret);

   if (num) *num = num_ret;
   return 1;
}

void
ecore_x_window_prop_property_del(Ecore_X_Window win, Ecore_X_Atom property)
{
   XDeleteProperty(_ecore_x_disp, win, property);
}

#if 0
/*
 * I see no purpose for a ecore_x_window_prop_property_notify().
 * Commenting out for now, suggest to remove it entirely.
 * /Kim
 */
/**
 * Send a property notify to a window.
 * @param win The window
 * @param type Type of notification
 * @param data The data
 *
 * Send a property notify to a window.
 */
void
ecore_x_window_prop_property_notify(Ecore_X_Window win, const char *type, long *data)
{
   Ecore_X_Atom        tmp;
   XClientMessageEvent xev;

   tmp = XInternAtom(_ecore_x_disp, type, False);

   xev.type = PropertyNotify;
   xev.display = _ecore_x_disp;
   xev.window = win;
   xev.message_type = tmp;
   xev.format = 32;
   xev.data.l[0] = data[0];
   xev.data.l[1] = data[1];
   xev.data.l[2] = data[2];
   xev.data.l[3] = data[3];
   xev.data.l[4] = data[4];

   XSendEvent(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), False,
	      (SubstructureNotifyMask | SubstructureRedirectMask),
	      (XEvent *)&xev);
}
#endif

/**
 * Set a window string property.
 * @param win The window
 * @param type The property
 * @param str The string
 * 
 * Set a window string property
 */
void
ecore_x_window_prop_string_set(Ecore_X_Window win, Ecore_X_Atom type, const char *str)
{
   XTextProperty       xtp;

   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   xtp.value = (unsigned char *)str;
   xtp.format = 8;
   xtp.encoding = ECORE_X_ATOM_UTF8_STRING;
   xtp.nitems = strlen(str);
   XSetTextProperty(_ecore_x_disp, win, &xtp, type);
}

/**
 * Get a window string property.
 * @param win The window
 * @param type The property
 * 
 * Return window string property of a window. String must be free'd when done.
 */
char *
ecore_x_window_prop_string_get(Ecore_X_Window win, Ecore_X_Atom type)
{
   XTextProperty       xtp;
   char               *str = NULL;

   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (XGetTextProperty(_ecore_x_disp, win, &xtp, type))
     {
	int      items;
	char   **list;
	Status   s;
	
	if (xtp.format == 8)
	  {
	     s = XmbTextPropertyToTextList(_ecore_x_disp, &xtp, &list, &items);
	     if ((s == Success) && (items > 0))
	       {
		  /* FIXME convert from xlib encoding to utf8  */
		  str = strdup(*list);
		  XFreeStringList(list);
	       }
	     else
	       /* FIXME convert from xlib encoding to utf8  */
	       str = strdup((char *)xtp.value);
	  }
	else
	  /* FIXME convert from xlib encoding to utf8  */
	  str = strdup((char *)xtp.value);
	XFree(xtp.value);
     }
   return str;
}

/**
 * Set a window title.
 * @param win The window
 * @param t The title string
 * 
 * Set a window title
 */
void
ecore_x_window_prop_title_set(Ecore_X_Window win, const char *t)
{
   char *list[1];
   XTextProperty xprop;
   
   list[0] = (char *) t;
   
   /* Xlib may not like the UTF8 String */
   /* FIXME convert utf8 to whatever encoding xlib prefers */
   /* ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_NAME, (char *)t); */
   if (XStringListToTextProperty(list, 1, &xprop))
     {
      XSetWMName(_ecore_x_disp, win, &xprop);
      XFree(xprop.value);
     }
            
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_NAME, (char *)t);
}

/**
 * Get a window title.
 * @param win The window
 * @return The windows title string
 * 
 * Return the title of a window. String must be free'd when done with.
 */
char *
ecore_x_window_prop_title_get(Ecore_X_Window win)
{
   char *title;

/*   title = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_NAME);*/
   title = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_NAME);
   return title;
}

/**
 * Sets the WM_COMMAND property for @a win.
 * 
 * @param win  The window.
 * @param argc Number of arguments.
 * @param argv Arguments.
 *
 * DEPRECATED. Please use ecore_x_icccm_command_set() instead.
 */
void
ecore_x_window_prop_command_set(Ecore_X_Window win, int argc, char **argv)
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
 *
 * DEPRECATED. Please use ecore_x_icccm_command_get() instead.
 */
void
ecore_x_window_prop_command_get(Ecore_X_Window win, int *argc, char ***argv)
{
   XGetCommand(_ecore_x_disp, win, argv, argc);
}



/**
 * Set a window visible title.
 * @param win The window
 * @param t The visible title string
 * 
 * Set a window visible title
 */
void
ecore_x_window_prop_visible_title_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_NAME,
				  (char *)t);
}

/**
 * Get a window visible title.
 * @param win The window
 * @return The windows visible title string
 * 
 * Return the visible title of a window. String must be free'd when done with.
 */
char *
ecore_x_window_prop_visible_title_get(Ecore_X_Window win)
{
   char *title;

   title = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
   return title;
}

/**
 * Set a window icon name.
 * @param win The window
 * @param t The icon name string
 * 
 * Set a window icon name
 * DEPRECATED. Please use ecore_x_icccm_icon_name_set() instead,
 * and ecore_x_netwm_icon_name_set() when it becomes available.
 */
void
ecore_x_window_prop_icon_name_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_ICON_NAME, (char *)t);
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_ICON_NAME,
				  (char *)t);
}

/**
 * Get a window icon name.
 * @param win The window
 * @return The windows icon name string
 * 
 * Return the icon name of a window. String must be free'd when done with.
 * DEPRECATED. Please use ecore_x_icccm_icon_name_get() instead.
 */
char *
ecore_x_window_prop_icon_name_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_ICON_NAME);
   if (!name) name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_ICON_NAME);
   return name;
}

/**
 * Set a window visible icon name.
 * @param win The window
 * @param t The visible icon name string
 * 
 * Set a window visible icon name
 */
void
ecore_x_window_prop_visible_icon_name_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME,
				  (char *)t);
}

/**
 * Get a window visible icon name.
 * @param win The window
 * @return The windows visible icon name string
 * 
 * Return the visible icon name of a window. String must be free'd when done with.
 */
char *
ecore_x_window_prop_visible_icon_name_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
   return name;
}

/**
 * Get a window client machine string.
 * @param win The window
 * @return The windows client machine string
 * 
 * Return the client machine of a window. String must be free'd when done with.
 * DEPRECATED. Please use ecore_x_icccm_client_machine_get() instead.
 */
char *
ecore_x_window_prop_client_machine_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_CLIENT_MACHINE);
   return name;
}

/**
 * Get a windows process id
 * @param win The window
 * @return The windows process id
 * 
 * Return the process id of a window.
 */
pid_t
ecore_x_window_prop_pid_get(Ecore_X_Window win)
{
   int            num = 0;
   pid_t          pid = 0;
   unsigned char *tmp = NULL;

   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_PID, XA_CARDINAL,
		                    32, &tmp, &num);
   if ((num) && (tmp))
     {
	pid = (pid_t)(*tmp);
	free(tmp);
     }
   return pid;
}

/**
 * Set a window name & class.
 * @param win The window
 * @param n The name string
 * @param c The class string
 * 
 * Set a window name * class
 * DEPRECATED. Please use ecore_x_icccm_name_class_set() instead.
 */
void
ecore_x_window_prop_name_class_set(Ecore_X_Window win, const char *n, const char *c)
{
   XClassHint *xch;
   
   xch = XAllocClassHint();
   if (!xch) return;
   xch->res_name = (char *)n;
   xch->res_class = (char *)c;
   XSetClassHint(_ecore_x_disp, win, xch);
   XFree(xch);   
}

/**
 * Get a window name & class.
 * @param win The window
 * @param n Name string
 * @param c Class string
 * 
 * Get a windows name and class property. strings must be free'd when done 
 * with.
 * DEPRECATED. Please use ecore_x_icccm_name_class_get() instead.
 */
void
ecore_x_window_prop_name_class_get(Ecore_X_Window win, char **n, char **c)
{
   XClassHint          xch;
   
   if (n) *n = NULL;
   if (c) *c = NULL;
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
 * Set or unset a wm protocol property.
 * @param win The Window
 * @param protocol The protocol to enable/disable
 * @param on On/Off
 *
 * DEPRECATED. Please use ecore_x_icccm_protocol_set() instead.
 */
void
ecore_x_window_prop_protocol_set(Ecore_X_Window win,
                                 Ecore_X_WM_Protocol protocol, int on)
{
   Atom *protos = NULL;
   Atom  proto;
   int   protos_count = 0;
   int   already_set = 0;
   int   i;

   /* check for invalid values */
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
	
	if (already_set) goto leave;
	new_protos = malloc((protos_count + 1) * sizeof(Atom));
	if (!new_protos) goto leave;
	for (i = 0; i < protos_count; i++)
	  new_protos[i] = protos[i];
	new_protos[protos_count] = proto;
	XSetWMProtocols(_ecore_x_disp, win, new_protos, protos_count + 1);
	free(new_protos);
     }
   else
     {
	if (!already_set) goto leave;
	for (i = 0; i < protos_count; i++)
	  {
	     if (protos[i] == proto)
	       {
		  int j;
		  
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
   if (protos) XFree(protos);
}

/**
 * Determines whether a protocol is set for a window.
 * @param win The Window
 * @param protocol The protocol to query
 * @return 1 if the protocol is set, else 0.
 *
 * DEPRECATED. Please use ecore_x_icccm_protocol_isset() instead.
 */
int
ecore_x_window_prop_protocol_isset(Ecore_X_Window win,
                                   Ecore_X_WM_Protocol protocol)
{
   Atom proto, *protos = NULL;
   int i, ret = 0, protos_count = 0;

   /* check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
	return 0;

   proto = _ecore_x_atoms_wm_protocols[protocol];

   if (!XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count))
	return ret;

   for (i = 0; i < protos_count; i++)
	if (protos[i] == proto)
	  {
	     ret = 1;
	     break;
	  }

   XFree(protos);

   return ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
Ecore_X_WM_Protocol *
ecore_x_window_prop_protocol_list_get(Ecore_X_Window win, int *num_ret)
{
   Atom *protos = NULL;
   int i, protos_count = 0;
   Ecore_X_WM_Protocol *prot_ret = NULL;
   
   if (!XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count))
     return NULL;

   if ((!protos) || (protos_count <= 0)) return NULL;
   prot_ret = calloc(1, protos_count * sizeof(Ecore_X_WM_Protocol));
   if (!prot_ret)
     {
	XFree(protos);
	return NULL;
     }
   for (i = 0; i < protos_count; i++)
     {
	Ecore_X_WM_Protocol j;
	
	prot_ret[i] = -1;
	for (j = 0; j < ECORE_X_WM_PROTOCOL_NUM; j++)
	  {
	     if (_ecore_x_atoms_wm_protocols[j] == protos[i])
	       prot_ret[i] = j;
	  }
     }
   XFree(protos);
   *num_ret = protos_count;
   return prot_ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_min_size_set(Ecore_X_Window win, int w, int h)
{
   XSizeHints          hints;
   long                ret;
   
   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   hints.flags |= PMinSize | PSize;
   hints.min_width = w;
   hints.min_height = h;
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_max_size_set(Ecore_X_Window win, int w, int h)
{
   XSizeHints          hints;
   long                ret;
   
   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   hints.flags |= PMaxSize | PSize;
   hints.max_width = w;
   hints.max_height = h;
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_base_size_set(Ecore_X_Window win, int w, int h)
{
   XSizeHints          hints;
   long                ret;
   
   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   hints.flags |= PBaseSize | PSize;
   hints.base_width = w;
   hints.base_height = h;
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_step_size_set(Ecore_X_Window win, int x, int y)
{
   XSizeHints          hints;
   long                ret;
   
   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   hints.flags |= PResizeInc;
   hints.width_inc = x;
   hints.height_inc = y;
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_xy_set(Ecore_X_Window win, int x, int y)
{
   XSizeHints          hints;
   long                ret;
   
   memset(&hints, 0, sizeof(XSizeHints));
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   hints.flags |= PPosition | USPosition;
   hints.x = x;
   hints.y = y;
   XMoveWindow(_ecore_x_disp, win, x, y);
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * Sets the sticky state for @a win.
 * @param win The window
 * @param on  Boolean representing the sticky state
 */
void
ecore_x_window_prop_sticky_set(Ecore_X_Window win, int on)
{
   unsigned long val = 0xffffffff;
   int ret, num = 0;
   unsigned char *data = NULL;

   if (on) {
      ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_DESKTOP,
                                       XA_CARDINAL, 32, &val, 1);
      ecore_x_window_prop_state_set(win, ECORE_X_WINDOW_STATE_STICKY);
      return;
   }
   
   ecore_x_window_prop_state_unset(win, ECORE_X_WINDOW_STATE_STICKY);
   ret = ecore_x_window_prop_property_get(0, ECORE_X_ATOM_NET_CURRENT_DESKTOP,
                                         XA_CARDINAL, 32, &data, &num);
   if (!ret || !num)
	   return;

   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_DESKTOP,
                                    XA_CARDINAL, 32, data, 1);
   free(data);
}

/**
 * Sets the input mode for @a win
 * @param win The Window
 * @param mode The input mode. See the description of
 *             #_Ecore_X_Window_Input_Mode for details.
 * @return 1 if the input mode could be set, else 0
 */
int
ecore_x_window_prop_input_mode_set(Ecore_X_Window win, Ecore_X_Window_Input_Mode mode)
{
   XWMHints *hints;

   if (!(hints = XGetWMHints(_ecore_x_disp, win)))
      if (!(hints = XAllocWMHints()))
         return 0;
	
   hints->flags |= InputHint;
   hints->input = (mode == ECORE_X_WINDOW_INPUT_MODE_PASSIVE
                   || mode == ECORE_X_WINDOW_INPUT_MODE_ACTIVE_LOCAL);
   XSetWMHints(_ecore_x_disp, win, hints);
   XFree(hints);

   ecore_x_window_prop_protocol_set(win, ECORE_X_WM_PROTOCOL_TAKE_FOCUS,
                 (mode == ECORE_X_WINDOW_INPUT_MODE_ACTIVE_LOCAL
                  || mode == ECORE_X_WINDOW_INPUT_MODE_ACTIVE_GLOBAL));

   return 1;
}

/**
 * Set the initial state of an Ecore_X_Window.
 *
 * @param win The window whose initial state is set.
 * @param withdrawn The window's new initial state.
 *
 * @return 1 if the input mode could be set, else 0
 *
 */
int
ecore_x_window_prop_initial_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state)
{
   XWMHints *hints;

   if (!(hints = XGetWMHints(_ecore_x_disp, win)))
      if (!(hints = XAllocWMHints()))
         return 0;

   switch (state) {
   case ECORE_X_WINDOW_STATE_HINT_NONE:
      hints->flags &= ~StateHint;
      break;
   case ECORE_X_WINDOW_STATE_HINT_WITHDRAWN:
      hints->initial_state = WithdrawnState;
      hints->flags |= StateHint;
      break;
   case ECORE_X_WINDOW_STATE_HINT_NORMAL:
      hints->initial_state = NormalState;
      hints->flags |= StateHint;
      break;
   case ECORE_X_WINDOW_STATE_HINT_ICONIC:
      hints->initial_state = IconicState;
      hints->flags |= StateHint;
      break;
   }

   XSetWMHints(_ecore_x_disp, win, hints);
   XFree(hints);

   return 1;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_prop_borderless_set(Ecore_X_Window win, int borderless)
{
   unsigned int data[5] = {0, 0, 0, 0, 0};

   data[0] = 2; /* just set the decorations hint! */
   data[2] = !borderless;
   
   ecore_x_window_prop_property_set(win, 
				    ECORE_X_ATOM_MOTIF_WM_HINTS,
				    ECORE_X_ATOM_MOTIF_WM_HINTS,
				    32, (void *)data, 5);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
int
ecore_x_window_prop_borderless_get(Ecore_X_Window win)
{
   unsigned char *data;
   int num;
   int borderless = 0;

   ecore_x_window_prop_property_get(win,
                                    ECORE_X_ATOM_MOTIF_WM_HINTS,
                                    ECORE_X_ATOM_MOTIF_WM_HINTS,
                                    32, &data, &num);

   /* check for valid data. only read the borderless flag if the
    * decorations data has been set.
    */
   if (data)
     {
	borderless = ((num > 2) && (data[0] & 2)) ? !data[2] : 0;
	free(data);
     }
   return borderless;
}

/**
 * Puts @a win in the desired layer. This currently works with
 * windowmanagers that are Gnome-compliant or support NetWM.
 * 
 * Normally you will use this function with one of the predefined
 * layer constants:
 * ECORE_X_WINDOW_LAYER_BELOW  - for windows to be stacked below
 * ECORE_X_WINDOW_LAYER_ABOVE  - for windows to be stacked on top
 * ECORE_X_WINDOW_LAYER_NORMAL - for windows to be put in the default layer

 * @param win
 * @param layer If < 3, @a win will be put below all other windows.
 *              If > 5, @a win will be "always-on-top"
 *              If = 4, @a win will be put in the default layer.
 *              Acceptable values range from 1 to 255 (0 reserved for
 *              desktop windows)
 * @return 1 if the state could be set else 0
 */
int
ecore_x_window_prop_layer_set(Ecore_X_Window win, int layer)
{
   if (layer <= 0 || layer > 255)
      return 0;
   
   if (layer < 3) /* below */
      ecore_x_window_prop_state_set(win, ECORE_X_WINDOW_STATE_BELOW);
   else if (layer > 5) /* above */
      ecore_x_window_prop_state_set(win, ECORE_X_WINDOW_STATE_ABOVE);
   else if (layer == 4)
   {
      ecore_x_window_prop_state_unset(win, ECORE_X_WINDOW_STATE_BELOW);
      ecore_x_window_prop_state_unset(win, ECORE_X_WINDOW_STATE_ABOVE);
   }

   /* set the gnome atom */	  
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_WIN_LAYER,
				    XA_CARDINAL, 32, &layer, 1);

   return 1;
}

/**
 * Set the withdrawn state of an Ecore_X_Window.
 * @param win The window whose withdrawn state is set.
 * @param withdrawn The window's new withdrawn state.
 */
void
ecore_x_window_prop_withdrawn_set(Ecore_X_Window win, int withdrawn)
{
#if 0
   XWMHints hints;
   long     ret;
   
   memset(&hints, 0, sizeof(XWMHints));
   XGetWMNormalHints(_ecore_x_disp, win, (XSizeHints *) &hints, &ret);
   
   if (!withdrawn)
      hints.initial_state &= ~WithdrawnState;
   else
      hints.initial_state |= WithdrawnState;
   
   hints.flags = WindowGroupHint | StateHint;
   XSetWMHints(_ecore_x_disp, win, &hints);
   XSetWMNormalHints(_ecore_x_disp, win, (XSizeHints *) &hints);
#else
   if (withdrawn)
      ecore_x_window_prop_initial_state_set(win, ECORE_X_WINDOW_STATE_HINT_WITHDRAWN);
   else
      ecore_x_window_prop_initial_state_set(win, ECORE_X_WINDOW_STATE_HINT_NONE);
#endif
}

/**
 * Request the window manager to change this windows desktop.
 * @param win The Window
 * @param desktop The desktop number.
 */
void
ecore_x_window_prop_desktop_request(Ecore_X_Window win, long desktop)
{
   XEvent xev;

   memset(&xev, 0, sizeof(XEvent));

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_NET_WM_DESKTOP;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = desktop;

   XSendEvent(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), False,
	      SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

/**
 * Request the window manager to change this window's state.
 *
 * Use this function to request the window manager to change the
 * specified window's state after it has been displayed (mapped).
 *
 * @param win     The Window
 * @param state   The requested state
 * @param action  The action to perform: 0 - unset, 1 - set, 2 - toggle
 */
void
ecore_x_window_prop_state_request(Ecore_X_Window win, Ecore_X_Window_State state, int action)
{
   XEvent xev;

   if (action < 0 || action > 2)
      return;

   xev.xclient.type = ClientMessage;
   xev.xclient.serial = 0;
   xev.xclient.send_event = True;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.format = 32;

   switch (state) {
   case ECORE_X_WINDOW_STATE_ICONIFIED:
      if (action == 0)
      {
	 XMapWindow(_ecore_x_disp, win);
	 return;
      }
      if (action != 1)
	 return;
      xev.xclient.message_type = ECORE_X_ATOM_WM_CHANGE_STATE;
      xev.xclient.data.l[0] = IconicState;
      break;
   default: /* The _NET_WM_STATE_... hints */
      xev.xclient.message_type = ECORE_X_ATOM_NET_WM_STATE;
      xev.xclient.data.l[0] = action;
      xev.xclient.data.l[1] = _ecore_x_window_prop_state_atom_get(state);
      break;
   }

   XSendEvent(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), False,
              SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}
   

/**
 * Used by the window manager, or client prior mapping, to set window desktop.
 * @param win The Window
 * @param desktop The desktop number.
 *
 * Used by the window manager, or client prior mapping, to set window desktop.
 */
void
ecore_x_window_prop_desktop_set(Ecore_X_Window win, long desktop)
{
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_DESKTOP, 
                    XA_CARDINAL, 32, &desktop, 1);
}

/**
 * Get the current desktop of a window
 * @param win The Window
 *
 * Get the current desktop of a window
 */
long
ecore_x_window_prop_desktop_get(Ecore_X_Window win)
{
   int            num;
   unsigned char *tmp;
   long           desktop = -1;

   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_DESKTOP, 
                                    XA_CARDINAL, 32, &tmp, &num);
   if ((tmp) && (num))
     {
	desktop = *(long *)tmp;
	free(tmp);
     }

   return desktop;
}


static Ecore_X_Atom 
_ecore_x_window_prop_type_atom_get(Ecore_X_Window_Type type)
{
   switch (type)
   {
      case ECORE_X_WINDOW_TYPE_DESKTOP:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
      case ECORE_X_WINDOW_TYPE_DOCK:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
      case ECORE_X_WINDOW_TYPE_TOOLBAR:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
      case ECORE_X_WINDOW_TYPE_MENU:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
      case ECORE_X_WINDOW_TYPE_UTILITY:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
      case ECORE_X_WINDOW_TYPE_SPLASH:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
      case ECORE_X_WINDOW_TYPE_DIALOG:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
      case ECORE_X_WINDOW_TYPE_NORMAL:
         return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;
      default:
         return 0;
   }
}

/**
 * Set a window's type.
 * @param win The Window
 * @param type The Type
 *
 * Set a windows type.
 */
void
ecore_x_window_prop_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type)
{
   Atom a;

   a = _ecore_x_window_prop_type_atom_get(type);
   if (a)
      ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
				       XA_ATOM, 32, (unsigned char*)&a, 1);
}


#if 0
/**
 * Change a window's type.
 * @param win The Window
 * @param type The Type
 *
 * Change a windows type.
 */
void
ecore_x_window_prop_window_type_set(Ecore_X_Window win, Ecore_X_Atom type)
{
	int            num;
	unsigned char *data = NULL;

	if (ecore_x_window_prop_property_get(win,
					     ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					     XA_ATOM, 32, &data, &num))
	   XFree(data);

	data = malloc(sizeof(Ecore_X_Atom));
	if (data)
	  {
	     ((Atom *)data)[0] = type;
	     ecore_x_window_prop_property_set(win,
					      ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					      XA_ATOM, 32, data, 1);
	  }
	free(data);
}
#endif

static Ecore_X_Atom 
_ecore_x_window_prop_state_atom_get(Ecore_X_Window_State s)
{
   switch(s)
   {
      case ECORE_X_WINDOW_STATE_MODAL:
         return ECORE_X_ATOM_NET_WM_STATE_MODAL;
      case ECORE_X_WINDOW_STATE_STICKY:
         return ECORE_X_ATOM_NET_WM_STATE_STICKY;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
         return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
         return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
      case ECORE_X_WINDOW_STATE_SHADED:
         return ECORE_X_ATOM_NET_WM_STATE_SHADED;
      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
         return ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
         return ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
      case ECORE_X_WINDOW_STATE_HIDDEN:
         return ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
      case ECORE_X_WINDOW_STATE_FULLSCREEN:
         return ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
      case ECORE_X_WINDOW_STATE_ABOVE:
         return ECORE_X_ATOM_NET_WM_STATE_ABOVE;
      case ECORE_X_WINDOW_STATE_BELOW:
         return ECORE_X_ATOM_NET_WM_STATE_BELOW;
      case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
         return ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;
      default:
         return 0;
   }
}

/**
 * Set a state for a window
 * @param win The Window whose properties will be changed
 * @param s The state to be set for this window
 *
 * Adds the state to the window's properties if not already included.
 */
void
ecore_x_window_prop_state_set(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num = 0, i;
   unsigned char  *old_data = NULL;
   unsigned char  *data = NULL;
   Atom           *oldset = NULL;
   Atom           *newset = NULL;
   Ecore_X_Atom   state;
   
   state = _ecore_x_window_prop_state_atom_get(s);

   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Atom *) old_data;
   newset = calloc(num + 1, sizeof(Atom));
   if (!newset) return;
   data = (unsigned char *) newset;
   
   for (i = 0; i < num; ++i)
   {
      if (oldset[i] == state)
      {
         XFree(old_data);
         free(data);
         return;
      }
      
      newset[i] = oldset[i];
   }
   
   newset[num] = state;
   
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_STATE,
                                    XA_ATOM, 32, data, num + 1);
   XFree(old_data);
   free(data);
}

/**
 * Check if a state is set for a window.
 * @param win The window to be checked
 * @param s The state whose state will be checked
 * @return 1 if the state has been set for this window, 0 otherwise.
 *
 * This function will look up the window's properties to determine
 * if a particular state is set for that window.
 */
int
ecore_x_window_prop_state_isset(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num, i, ret = 0;
   unsigned char  *data;
   Atom           *states;
   Ecore_X_Atom   state;

   state = _ecore_x_window_prop_state_atom_get(s);
   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
                                         XA_ATOM, 32, &data, &num))
      return ret;

   states = (Atom *) data;

   for (i = 0; i < num; ++i)
   {
      if(states[i] == state)
      {
         ret = 1;
         break;
      }
   }

   XFree(data);
   return ret;
}

/**
 * Remove the specified state from a window.
 * @param win The window whose properties will be changed
 * @param s The state to be deleted from the window's properties
 *
 * Checks if the specified state is set for the window, and if so, deletes
 * that state from the window's properties.
 */
void
ecore_x_window_prop_state_unset(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num = 0, i, j = 0;
   unsigned char  *old_data = NULL;
   unsigned char  *data = NULL;
   Atom           *oldset = NULL;
   Atom           *newset = NULL;
   Ecore_X_Atom   state ;
   
   state = _ecore_x_window_prop_state_atom_get(s);

   if (!ecore_x_window_prop_state_isset(win, s)) {
      return;
   }
   
   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Atom *) old_data;
   newset = calloc(num - 1, sizeof(Atom));
   data = (unsigned char *) newset;
   for (i = 0; i < num; ++i)
      if (oldset[i] != state)
         newset[j++] = oldset[i];

   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_STATE,
                                    XA_ATOM, 32, data, j);
   XFree(oldset);
   free(newset);
}

#if 0
void
ecore_x_window_prop_window_type_utility_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY);
}

/**
 * Set a window as a splash type.
 * @param win The Window
 *
 * Set a window as a splash type.
 */
void
ecore_x_window_prop_window_type_splash_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH);
}

/**
 * Set a window as a dialog type.
 * @param win The Window
 *
 * Set a window as a dialog type.
 */
void
ecore_x_window_prop_window_type_dialog_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG);
}

/**
 * Set a window as a normal type.
 * @param win The Window
 *
 * Set a window as a normal type.
 */
void
ecore_x_window_prop_window_type_normal_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL);
}

#endif

/**
 * Set the requested opacity of the window
 * @param win The window whose opacity will be set
 * @param opacity The opacity value to be applied to the window
 * 
 * This only has an effect if the Composite extension is present and
 * a compositing manager is running. This hint is still pending approval
 * as part of the EWMH specification. The value supplied should be an
 * integer between 0 and 255, with 255 representing full opacity.
 */
void
ecore_x_window_prop_window_opacity_set(Ecore_X_Window win, int opacity)
{
   unsigned long o_val;
   double tmp;

   if (opacity < 0)
      opacity = 0;
   else if (opacity > 255)
      opacity = 255;

   tmp = (double) opacity/255. * 4294967295.;
   o_val = (unsigned long) tmp;
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
                                    XA_CARDINAL, 32, &o_val, 1);
}

/**
 * Get the current opacity value of the window
 * @param win The window whose opacity is being requested
 * @return An int between 0 and 255 representing the window's opacity value,
 * or -1 if the property is not found.
 */
int
ecore_x_window_prop_window_opacity_get(Ecore_X_Window win)
{
   unsigned char  *data = NULL;
   unsigned long  lval;
   int            ret_val = -1;
   int            num;

   if(ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
                                       XA_CARDINAL, 32, &data, &num))
   {
      if (data && num)
      {
         lval = *(unsigned long *) data;
         ret_val = (int) ((double) lval / 4294967295. * 255.);
      }
   }
   
   return ret_val;
}

Ecore_X_Atom *
ecore_x_window_prop_list(Ecore_X_Window win, int *num_ret)
{
   Ecore_X_Atom *atoms;
   Atom *atom_ret;
   int num = 0, i;
	
   if (num_ret) *num_ret = 0;

   atom_ret = XListProperties(_ecore_x_disp, win, &num);
   if (!atom_ret) return NULL;

   atoms = malloc(num * sizeof(Ecore_X_Atom));
   if (atoms)
     {
	for (i = 0; i < num; i++) atoms[i] = atom_ret[i];
	if (num_ret) *num_ret = num;
     }
   XFree(atom_ret);
   return atoms;
}
