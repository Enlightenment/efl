#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include <inttypes.h>
#include <limits.h>

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, unsigned char **data, int *num)
{
   Atom type_ret = 0;
   int ret, size_ret = 0;
   unsigned long num_ret = 0, bytes = 0, i;
   unsigned char *prop_ret = NULL;
   
   if (!win) win = DefaultRootWindow(_ecore_x_disp);

   ret = XGetWindowProperty(_ecore_x_disp, win, property, 0, LONG_MAX,
                            False, type, &type_ret, &size_ret,
                            &num_ret, &bytes, &prop_ret);

   if (ret != Success)
     {
	*data = NULL;
	return 0;
     }

   if (size != size_ret || !num_ret) {
      XFree(prop_ret);
	  *data = NULL;
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

   *num = num_ret;
   return 1;
}

/**
 * Send a property notify to a window.
 * @param win The window
 * @param type Type of notification
 * @param data The data
 *
 * Send a property notify to a window.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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

/**
 * Set a window string property.
 * @param win The window
 * @param type The property
 * @param str The string
 * 
 * Set a window string property
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_string_set(Ecore_X_Window win, Ecore_X_Atom type, char *str)
{
   XTextProperty       xtp;

   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   xtp.value = str;
   xtp.format = 8;
   xtp.encoding = _ecore_x_atom_utf8_string;
   xtp.nitems = strlen(str);
   XSetTextProperty(_ecore_x_disp, win, &xtp, type);
}

/**
 * Get a window string property.
 * @param win The window
 * @param type The property
 * 
 * Return window string property of a window. String must be free'd when done.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
		  str = strdup(*list);
		  XFreeStringList(list);
	       }
	     else
	       str = strdup((char *)xtp.value);
	  }
	else
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_title_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, _ecore_x_atom_wm_name, (char *)t);
   ecore_x_window_prop_string_set(win, _ecore_x_atom_net_wm_name, (char *)t);
}

/**
 * Get a window title.
 * @param win The window
 * @return The windows title string
 * 
 * Return the title of a window. String must be free'd when done with.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
char *
ecore_x_window_prop_title_get(Ecore_X_Window win)
{
   char *title;

   title = ecore_x_window_prop_string_get(win, _ecore_x_atom_net_wm_name);
   if (!title) title = ecore_x_window_prop_string_get(win, _ecore_x_atom_wm_name);
   return title;
}

/**
 * Set a window visible title.
 * @param win The window
 * @param t The visible title string
 * 
 * Set a window visible title
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_visible_title_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, _ecore_x_atom_net_wm_visible_name,
				  (char *)t);
}

/**
 * Get a window visible title.
 * @param win The window
 * @return The windows visible title string
 * 
 * Return the visible title of a window. String must be free'd when done with.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
char *
ecore_x_window_prop_visible_title_get(Ecore_X_Window win)
{
   char *title;

   title = ecore_x_window_prop_string_get(win, _ecore_x_atom_net_wm_visible_name);
   return title;
}

/**
 * Set a window icon name.
 * @param win The window
 * @param t The icon name string
 * 
 * Set a window icon name
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_icon_name_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, _ecore_x_atom_wm_icon_name, (char *)t);
   ecore_x_window_prop_string_set(win, _ecore_x_atom_net_wm_icon_name,
				  (char *)t);
}

/**
 * Get a window icon name.
 * @param win The window
 * @return The windows icon name string
 * 
 * Return the icon name of a window. String must be free'd when done with.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
char *
ecore_x_window_prop_icon_name_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, _ecore_x_atom_net_wm_icon_name);
   if (!name) name = ecore_x_window_prop_string_get(win, _ecore_x_atom_wm_icon_name);
   return name;
}

/**
 * Set a window visible icon name.
 * @param win The window
 * @param t The visible icon name string
 * 
 * Set a window visible icon name
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_visible_icon_name_set(Ecore_X_Window win, const char *t)
{
   ecore_x_window_prop_string_set(win, _ecore_x_atom_net_wm_visible_icon_name,
				  (char *)t);
}

/**
 * Get a window visible icon name.
 * @param win The window
 * @return The windows visible icon name string
 * 
 * Return the visible icon name of a window. String must be free'd when done with.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
char *
ecore_x_window_prop_visible_icon_name_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, _ecore_x_atom_net_wm_visible_icon_name);
   return name;
}

/**
 * Get a window client machine string.
 * @param win The window
 * @return The windows client machine string
 * 
 * Return the client machine of a window. String must be free'd when done with.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
char *
ecore_x_window_prop_client_machine_get(Ecore_X_Window win)
{
   char *name;

   name = ecore_x_window_prop_string_get(win, _ecore_x_atom_wm_client_machine);
   return name;
}

/**
 * Get a windows process id
 * @param win The window
 * @return The windows process id
 * 
 * Return the process id of a window.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
pid_t
ecore_x_window_prop_pid_get(Ecore_X_Window win)
{
   int            num = 0;
   pid_t          pid = 0;
   unsigned char *tmp = NULL;

   ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_pid, XA_CARDINAL,
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
				    _ecore_x_atom_wm_protocols);
		  goto leave;
	       }
	  }
     }
   leave:
   if (protos) XFree(protos);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
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
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

/**
 * Sets the sticky state for @win
 * @param win The window
 * @param on Boolean representing the sticky state
 */
void
ecore_x_window_prop_sticky_set(Ecore_X_Window win, int on)
{
   unsigned long val = 0xffffffff;
   int ret, num = 0;
   unsigned char *data = NULL;

   if (on) {
      ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_desktop,
                                       XA_CARDINAL, 32, &val, 1);
      return;
   }
   
   ret = ecore_x_window_prop_property_get(0, _ecore_x_atom_net_current_desktop,
                                         XA_CARDINAL, 32, &data, &num);
   if (!ret || !num)
	   return;

   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_desktop,
                                    XA_CARDINAL, 32, data, 1);
   free(data);
}

/**
 * Sets the input mode for @win
 * @param win The Window
 * @param mode The input mode. See the description of
 *             @Ecore_X_Window_Input_Mode for details
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
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_borderless_set(Ecore_X_Window win, int borderless)
{
   unsigned int data[5] = {0};

   data[0] = 2; /* just set the decorations hint! */
   data[2] = !borderless;
   
   ecore_x_window_prop_property_set(win, 
					 _ecore_x_atom_motif_wm_hints,
					 _ecore_x_atom_motif_wm_hints,
					 32, (void *)data, 5);
}

/**
 * Puts @win in the desired layer. This currently works with
 * windowmanagers that are Gnome-compliant or support NetWM.
 * 
 * @param win
 * @param layer If < 0, @win will be put below all other windows.
 *              If > 0, @win will be "always-on-top"
 *              If = 0, @win will be put in the default layer.
 * @return 1 if the state could be set else 0
	 *
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_window_prop_layer_set(Ecore_X_Window win, int layer)
{
   Ecore_X_Atom atom = 0;
   unsigned char *data = NULL;
   int i, val = 4, num = 0; /* normal layer */

   if (layer < 0) /* below */
     {
      atom = _ecore_x_atom_net_wm_state_below;
	  val = 2;
     }
   else if (layer > 0) /* above */
     {
	   atom = _ecore_x_atom_net_wm_state_above;
	   val = 6;
     }
  
   /* set the NetWM atoms
	* get the atoms that are already set
	*/
   if (ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_state,
                                        XA_ATOM, 32, &data, &num))
     {
      /* and set the ones we're interested in */
        for (i = 0; i < num; i++)
	  {
	     if (data[i] == _ecore_x_atom_net_wm_state_below)
	       data[i] = (layer < 0);
	     else if (data[i] == _ecore_x_atom_net_wm_state_above)
	       data[i] = (layer > 0);
	  }

	ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_state,
			                 XA_ATOM, 32, data, num);
	free(data);
     }
   else
      ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_state,
                                       XA_ATOM, 32, &atom, 1);

   /* set the gnome atom */	  
   ecore_x_window_prop_property_set(win, _ecore_x_atom_win_layer,
				    XA_CARDINAL, 32, &val, 1);

   return 1;
}

/**
 * Set the withdrawn state of an Ecore_X_Window.
 * @param win The window whose withdrawn state is set.
 * @param withdrawn The window's new withdrawn state.
 *
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_withdrawn_set(Ecore_X_Window win, int withdrawn)
{
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
}

/**
 * Request the window manager to change this windows desktop.
 * @param win The Window
 * @param desktop The desktop number.
 *
 * Request the window manager to change this windows desktop.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_desktop_request(Ecore_X_Window win, long desktop)
{
   long data[5];

   data[0] = desktop;
   data[1] = 0;
   data[2] = 0;
   data[3] = 0;
   data[4] = 0;
   ecore_x_window_prop_property_notify(win, "_NET_WM_DESKTOP", data);
}

/**
 * Used by the window manager, or client prior mapping, to set window desktop.
 * @param win The Window
 * @param desktop The desktop number.
 *
 * Used by the window manager, or client prior mapping, to set window desktop.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_desktop_set(Ecore_X_Window win, long desktop)
{
   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_desktop, 
                    XA_CARDINAL, 32, &desktop, 1);
}

/**
 * Get the current desktop of a window
 * @param win The Window
 *
 * Get the current desktop of a window
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
long
ecore_x_window_prop_desktop_get(Ecore_X_Window win)
{
   int            num;
   unsigned char *tmp;
   long           desktop = -1;

   ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_desktop, 
                                    XA_CARDINAL, 32, &tmp, &num);
   if ((tmp) && (num))
     {
	desktop = *(long *)tmp;
	free(tmp);
     }

   return desktop;
}

/**
 * Change a window's type.
 * @param win The Window
 * @param type The Type
 *
 * Change a windows type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_set(Ecore_X_Window win, Ecore_X_Atom type)
{
	int            num;
	unsigned char *data = NULL;

	if (ecore_x_window_prop_property_get(win,
					     _ecore_x_atom_net_wm_window_type,
					     XA_ATOM, 32, &data, &num))
	   XFree(data);

	data = malloc(sizeof(Ecore_X_Atom));
	if (data)
	  {
	     ((Atom *)data)[0] = type;
	     ecore_x_window_prop_property_set(win,
					      _ecore_x_atom_net_wm_window_type,
					      XA_ATOM, 32, data, 1);
	  }
	free(data);
}

static Ecore_X_Atom 
_ecore_x_window_prop_state_atom_get(Ecore_X_Window_State s)
{
   switch(s)
   {
      case ECORE_X_WINDOW_STATE_MODAL:
         return _ecore_x_atom_net_wm_state_modal;
      case ECORE_X_WINDOW_STATE_STICKY:
         return _ecore_x_atom_net_wm_state_sticky;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
         return _ecore_x_atom_net_wm_state_maximized_vert;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
         return _ecore_x_atom_net_wm_state_maximized_horz;
      case ECORE_X_WINDOW_STATE_SHADED:
         return _ecore_x_atom_net_wm_state_shaded;
      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
         return _ecore_x_atom_net_wm_state_skip_taskbar;
      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
         return _ecore_x_atom_net_wm_state_skip_pager;
      case ECORE_X_WINDOW_STATE_HIDDEN:
         return _ecore_x_atom_net_wm_state_skip_pager;
      case ECORE_X_WINDOW_STATE_FULLSCREEN:
         return _ecore_x_atom_net_wm_state_fullscreen;
      case ECORE_X_WINDOW_STATE_ABOVE:
         return _ecore_x_atom_net_wm_state_above;
      case ECORE_X_WINDOW_STATE_BELOW:
         return _ecore_x_atom_net_wm_state_below;
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
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_state_set(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num = 0, i;
   unsigned char  *oldset = NULL;
   unsigned char  *newset = NULL;
   Ecore_X_Atom   state = _ecore_x_window_prop_state_atom_get(s);

   ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_state,
                                    XA_ATOM, 32, &oldset, &num);
   
   newset = malloc(sizeof(Ecore_X_Atom) * (num + 1));
   
   for (i = 0; i < num; ++i)
   {
      if (oldset[i] == state)
      {
         XFree(oldset);
         free(newset);
         return;
      }
      
      newset[i] = oldset[i];
   }
   
   newset[i] = state;
   
   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_state,
                                    XA_ATOM, 32, newset, num + 1);
   XFree(oldset);
   free(newset);
}

/**
 * Check if a state is set for a window.
 * @param win The window to be checked
 * @param s The state whose state will be checked
 * @return 1 if the state has been set for this window, 0 otherwise.
 *
 * This function will look up the window's properties to determine
 * if a particular state is set for that window.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_x_window_prop_state_isset(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num, i;
   unsigned char  *data;
   Ecore_X_Atom   state = _ecore_x_window_prop_state_atom_get(s);

   if (!ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_state,
                                         XA_ATOM, 32, &data, &num))
   {
      XFree(data);
      return 0;
   }

   for (i = 0; i < num; ++i)
   {
      if(data[i] == state)
      {
         XFree(data);
         return 1;
      }
   }

   return 0;
}

/**
 * Remove the specified state from a window.
 * @param win The window whose properties will be changed
 * @param s The state to be deleted from the window's properties
 *
 * Checks if the specified state is set for the window, and if so, deletes
 * that state from the window's properties.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_state_unset(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num = 0, i, j = 0;
   unsigned char  *oldset = NULL;
   unsigned char  *newset = NULL;
   Ecore_X_Atom   state = _ecore_x_window_prop_state_atom_get(s);

   if (!ecore_x_window_prop_state_isset(win, s)) {
      return;
   }
   
   ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_state,
                                    XA_ATOM, 32, &oldset, &num);
   if(num > 1)
   {
      newset = calloc(sizeof(Ecore_X_Atom), num - 1);
      for (i = 0; i < num; ++i)
         if (oldset[i] != state)
            newset[j++] = oldset[i];
   }

   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_state,
                                    XA_ATOM, 32, newset, j);
   XFree(oldset);
   free(newset);
}

/**
 * Set a window as a desktop type.
 * @param win The Window
 *
 * Set a window as a desktop type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_desktop_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_desktop);
}

/**
 * Set a window as a desktop type.
 * @param win The Window
 *
 * Set a window as a desktop type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_dock_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_dock);
}

/**
 * Set a window as a toolbar type.
 * @param win The Window
 *
 * Set a window as a toolbar type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_toolbar_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_toolbar);
}

/**
 * Set a window as a menu type.
 * @param win The Window
 *
 * Set a window as a menu type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_menu_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_menu);
}

/**
 * Set a window as a utility type.
 * @param win The Window
 *
 * Set a window as a utility type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_utility_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_utility);
}

/**
 * Set a window as a splash type.
 * @param win The Window
 *
 * Set a window as a splash type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_splash_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_splash);
}

/**
 * Set a window as a dialog type.
 * @param win The Window
 *
 * Set a window as a dialog type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_dialog_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_dialog);
}

/**
 * Set a window as a normal type.
 * @param win The Window
 *
 * Set a window as a normal type.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_window_type_normal_set(Ecore_X_Window win)
{
   ecore_x_window_prop_window_type_set(win, _ecore_x_atom_net_wm_window_type_normal);
}

/**
 * Set the requested opacity of the window
 * @param win The window whose opacity will be set
 * @param opacity The opacity value to be applied to the window
 * 
 * This only has an effect if the Composite extension is present and
 * a compositing manager is running. This hint is still pending approval
 * as part of the EWMH specification. The value supplied should be an
 * integer between 0 and 255, with 255 representing full opacity.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void ecore_x_window_prop_window_opacity_set(Ecore_X_Window win, int opacity)
{
   unsigned long o_val;
   double tmp;

   if (opacity < 0)
      opacity = 0;
   else if (opacity > 255)
      opacity = 255;

   tmp = (double) opacity/255. * 4294967295.;
   o_val = (unsigned long) tmp;
   ecore_x_window_prop_property_set(win, _ecore_x_atom_net_wm_window_opacity,
                                    XA_CARDINAL, 32, &o_val, 1);
}

/**
 * Get the current opacity value of the window
 * @param win The window whose opacity is being requested
 * @return An int between 0 and 255 representing the window's opacity value,
 * or -1 if the property is not found.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int ecore_x_window_prop_window_opacity_get(Ecore_X_Window win)
{
   unsigned char  *data = NULL;
   unsigned long  lval;
   int            ret_val = -1;
   int            num;

   if(ecore_x_window_prop_property_get(win, _ecore_x_atom_net_wm_window_opacity,
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

