#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_property_set(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Atom format, int size, void *data, int number)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (size != 32)
     XChangeProperty(_ecore_x_disp, win, type, format, size, PropModeReplace,
		     (unsigned char *)data, number);
   else
     {
	unsigned long *dat;
	int            i, *ptr;
	
	dat = malloc(sizeof(unsigned long) * number);
	if (dat)
	  {
	     for (ptr = (int *)data, i = 0; i < number; i++) dat[i] = ptr[i];
	     XChangeProperty(_ecore_x_disp, win, type, format, size, 
			     PropModeReplace, (unsigned char *)dat, number);
	     free(dat);
	  }
     }
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
   XStoreName(_ecore_x_disp, win, t);
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
   XTextProperty       xtp;
   
   if (XGetWMName(_ecore_x_disp, win, &xtp))
     {
	int      items;
	char   **list;
	Status   s;
	char    *title = NULL;
	
	if (xtp.format == 8)
	  {
	     s = XmbTextPropertyToTextList(_ecore_x_disp, &xtp, &list, &items);
	     if ((s == Success) && (items > 0))
	       {
		  title = strdup(*list);
		  XFreeStringList(list);
	       }
	     else
	       title = strdup((char *)xtp.value);
	  }
	else
	  title = strdup((char *)xtp.value);
	XFree(xtp.value);
	return title;
     }
   return NULL;
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
 * Set a window property to get message for close.
 * @param win The window
 * @param on On/Off
 * 
 * Set a window porperty to let a window manager send a delete message instead
 * of just closing (destroying) the window.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_delete_request_set(Ecore_X_Window win, int on)
{
   Atom *protos = NULL;
   int   protos_count = 0;
   int   already_set = 0;
   int   i;
   
   if (!XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count))
     {
	protos = NULL;
	protos_count = 0;
     }
   for (i = 0; i < protos_count; i++)
     {
	if (protos[i] == _ecore_x_atom_wm_delete_window)
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
	new_protos[protos_count] = _ecore_x_atom_wm_delete_window;
	XSetWMProtocols(_ecore_x_disp, win, new_protos, protos_count + 1);
	free(new_protos);
     }
   else
     {
	if (!already_set) goto leave;
	for (i = 0; i < protos_count; i++)
	  {
	     if (protos[i] == _ecore_x_atom_wm_delete_window)
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
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_borderless_set(Ecore_X_Window win, int borderless)
{
   if (!borderless)
     XDeleteProperty(_ecore_x_disp, win, _ecore_x_atom_motif_wm_hints);
   else
     {
	const unsigned int data[5] =
	  { 0x3, 0x0, 0x0, 0x2ada27b0, 0x2aabd6b0};
	
	ecore_x_window_prop_property_set(win, 
					 _ecore_x_atom_motif_wm_hints,
					 _ecore_x_atom_motif_wm_hints,
					 32, (void *)data, 5);
     }
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_window_prop_layer_set(Ecore_X_Window win, int layer)
{
   ecore_x_window_prop_property_set(win, _ecore_x_atom_win_layer, 
				    XA_CARDINAL, 32, &layer, 1);
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
   XGetWMNormalHints(_ecore_x_disp, win, &hints, &ret);
   
   if (!withdrawn)
      hints.initial_state &= ~WithdrawnState;
   else
      hints.initial_state |= WithdrawnState;
   
   hints.flags = WindowGroupHint | StateHint;
   XSetWMHints(_ecore_x_disp, win, &hints);
   XSetWMNormalHints(_ecore_x_disp, win, &hints);
}

