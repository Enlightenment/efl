/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"
#include <inttypes.h>
#include <limits.h>

#define _ATOM_SET_CARD32(win, atom, p_val, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_CARDINAL, 32, PropModeReplace, \
                   (unsigned char *)p_val, cnt)

/*
 * Set CARD32 (array) property
 */
EAPI void
ecore_x_window_prop_card32_set(Ecore_X_Window win, Ecore_X_Atom atom,
			       unsigned int *val, unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   _ATOM_SET_CARD32(win, atom, val, num);
#else
   long               *v2;
   unsigned int        i;

   v2 = malloc(num * sizeof(long));
   if (!v2)
      return;
   for (i = 0; i < num; i++)
      v2[i] = val[i];
   _ATOM_SET_CARD32(win, atom, v2, num);
   free(v2);
#endif
}

/*
 * Get CARD32 (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_card32_get(Ecore_X_Window win, Ecore_X_Atom atom,
			       unsigned int *val, unsigned int len)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   unsigned int        i;
   int                 num;

   prop_ret = NULL;
   XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
		      XA_CARDINAL, &type_ret, &format_ret, &num_ret,
		      &bytes_after, &prop_ret);
   if (prop_ret && type_ret == XA_CARDINAL && format_ret == 32)
     {
	if (num_ret < len)
	   len = num_ret;

	for (i = 0; i < len; i++)
	   val[i] = ((unsigned long*)prop_ret)[i];
	
	num = len;
     }
   else
     {
	num = -1;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num;
}

/*
 * Get CARD32 (array) property of any length
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_card32_list_get(Ecore_X_Window win, Ecore_X_Atom atom,
				    unsigned int **plst)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   unsigned int        i, *val;
   int                 num;

   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
			  XA_CARDINAL, &type_ret, &format_ret, &num_ret,
			  &bytes_after, &prop_ret) != Success)
      return -1;

   if (type_ret == None || num_ret == 0)
     {
	num = 0;
	*plst = NULL;
     }
   else if (prop_ret && type_ret == XA_CARDINAL && format_ret == 32)
     {
	val = malloc(num_ret * sizeof(unsigned int));
	for (i = 0; i < num_ret; i++)
	   val[i] = ((unsigned long *)prop_ret)[i];
	num = num_ret;
	*plst = val;
     }
   else
     {
	num = -1;
	*plst = NULL;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num;
}

/*
 * Set X ID (array) property
 */
EAPI void
ecore_x_window_prop_xid_set(Ecore_X_Window win, Ecore_X_Atom atom,
			    Ecore_X_Atom type, Ecore_X_ID * lst,
			    unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
		   (unsigned char *)lst, num);
#else
   unsigned long      *pl;
   unsigned int        i;

   pl = malloc(num * sizeof(long));
   if (!pl)
      return;
   for (i = 0; i < num; i++)
      pl[i] = lst[i];
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
		   (unsigned char *)pl, num);
   free(pl);
#endif
}

/*
 * Get X ID (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_xid_get(Ecore_X_Window win, Ecore_X_Atom atom,
			    Ecore_X_Atom type, Ecore_X_ID * lst,
			    unsigned int len)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   int                 num;
   unsigned            i;

   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
			  type, &type_ret, &format_ret, &num_ret,
			  &bytes_after, &prop_ret) != Success)
      return -1;

   if (type_ret == None)
     {
	num = 0;
     }
   else if (prop_ret && type_ret == type && format_ret == 32)
     {
	if (num_ret < len)
	   len = num_ret;
	for (i = 0; i < len; i++)
	   lst[i] = ((unsigned long *)prop_ret)[i];
	num = len;
     }
   else
     {
	num = -1;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num;
}

/*
 * Get X ID (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_xid_list_get(Ecore_X_Window win, Ecore_X_Atom atom,
				 Ecore_X_Atom type, Ecore_X_ID ** val)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   Ecore_X_Atom       *alst;
   int                 num;
   unsigned            i;

   *val = NULL;
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
			  type, &type_ret, &format_ret, &num_ret,
			  &bytes_after, &prop_ret) != Success)
      return -1;

   if (type_ret == None || num_ret == 0)
     {
	num = 0;
     }
   else if (prop_ret && type_ret == type && format_ret == 32)
     {
	alst = malloc(num_ret * sizeof(Ecore_X_ID));
	for (i = 0; i < num_ret; i++)
	   alst[i] = ((unsigned long *)prop_ret)[i];
	*val = alst;
	num = num_ret;
     }
   else
     {
	num = -1;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num;
}

/*
 * Remove/add/toggle X ID list item.
 */
EAPI void
ecore_x_window_prop_xid_list_change(Ecore_X_Window win, Ecore_X_Atom atom,
				    Ecore_X_Atom type, Ecore_X_ID item, int op)
{
   Ecore_X_ID         *lst;
   int                 i, num;

   num = ecore_x_window_prop_xid_list_get(win, atom, type, &lst);
   if (num < 0)
      return;			/* Error - assuming invalid window */

   /* Is it there? */
   for (i = 0; i < num; i++)
     {
	if (lst[i] == item)
	   break;
     }

   if (i < num)
     {
	/* Was in list */
	if (op == ECORE_X_PROP_LIST_ADD)
	   goto done;
	/* Remove it */
	num--;
	for (; i < num; i++)
	   lst[i] = lst[i + 1];
     }
   else
     {
	/* Was not in list */
	if (op == ECORE_X_PROP_LIST_REMOVE)
	   goto done;
	/* Add it */
	num++;
	lst = realloc(lst, num * sizeof(Ecore_X_ID));
	lst[i] = item;
     }

   ecore_x_window_prop_xid_set(win, atom, type, lst, num);

 done:
   if (lst)
      free(lst);
}

/*
 * Set Atom (array) property
 */
EAPI void
ecore_x_window_prop_atom_set(Ecore_X_Window win, Ecore_X_Atom atom,
			     Ecore_X_Atom * lst, unsigned int num)
{
   ecore_x_window_prop_xid_set(win, atom, XA_ATOM, lst, num);
}

/*
 * Get Atom (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_atom_get(Ecore_X_Window win, Ecore_X_Atom atom,
			     Ecore_X_Atom * lst, unsigned int len)
{
   return ecore_x_window_prop_xid_get(win, atom, XA_ATOM, lst, len);
}

/*
 * Get Atom (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_atom_list_get(Ecore_X_Window win, Ecore_X_Atom atom,
				  Ecore_X_Atom ** plst)
{
   return ecore_x_window_prop_xid_list_get(win, atom, XA_ATOM, plst);
}

/*
 * Remove/add/toggle atom list item.
 */
EAPI void
ecore_x_window_prop_atom_list_change(Ecore_X_Window win, Ecore_X_Atom atom,
				     Ecore_X_Atom item, int op)
{
   ecore_x_window_prop_xid_list_change(win, atom, XA_ATOM, item, op);
}

/*
 * Set Window (array) property
 */
EAPI void
ecore_x_window_prop_window_set(Ecore_X_Window win, Ecore_X_Atom atom,
			       Ecore_X_Window * lst, unsigned int num)
{
   ecore_x_window_prop_xid_set(win, atom, XA_WINDOW, lst, num);
}

/*
 * Get Window (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_window_get(Ecore_X_Window win, Ecore_X_Atom atom,
			       Ecore_X_Window * lst, unsigned int len)
{
   return ecore_x_window_prop_xid_get(win, atom, XA_WINDOW, lst, len);
}

/*
 * Get Window (array) property
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * The returned array must be freed with free().
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_window_list_get(Ecore_X_Window win, Ecore_X_Atom atom,
				    Ecore_X_Window ** plst)
{
   return ecore_x_window_prop_xid_list_get(win, atom, XA_WINDOW, plst);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_X_Atom
ecore_x_window_prop_any_type(void)
{
   return AnyPropertyType;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI void
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
EAPI int
ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size __UNUSED__, unsigned char **data, int *num)
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

   if (!num_ret) {
      XFree(prop_ret);
      return 0;
   }
   
   if (!(*data = malloc(num_ret * size_ret / 8))) {
      XFree(prop_ret);
      return 0;
   }
   
   switch (size_ret) {
      case 8:
	 for (i = 0; i < num_ret; i++)
	   (*data)[i] = prop_ret[i];
	 break;
      case 16:
	 for (i = 0; i < num_ret; i++)
	   ((unsigned short *) *data)[i] = ((unsigned short *) prop_ret)[i];
	 break;
      case 32:
	 for (i = 0; i < num_ret; i++)
	   ((unsigned int *) *data)[i] = ((unsigned long *) prop_ret)[i];
	 break;
   }

   XFree(prop_ret);

   if (num) *num = num_ret;
   return size_ret;
}

EAPI void
ecore_x_window_prop_property_del(Ecore_X_Window win, Ecore_X_Atom property)
{
   XDeleteProperty(_ecore_x_disp, win, property);
}

EAPI Ecore_X_Atom *
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

/**
 * Set a window string property.
 * @param win The window
 * @param type The property
 * @param str The string
 * 
 * Set a window string property
 */
EAPI void
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
EAPI char *
ecore_x_window_prop_string_get(Ecore_X_Window win, Ecore_X_Atom type)
{
   XTextProperty       xtp;
   char               *str = NULL;

   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (XGetTextProperty(_ecore_x_disp, win, &xtp, type))
     {
	int      items;
	char   **list = NULL;
	Status   s;
	
	if (xtp.encoding == ECORE_X_ATOM_UTF8_STRING)
	  {
	     str = strdup((char *)xtp.value);
	  }
	else
	  {
#ifdef X_HAVE_UTF8_STRING
	     s = Xutf8TextPropertyToTextList(_ecore_x_disp, &xtp,
					     &list, &items);
#else
	     s = XmbTextPropertyToTextList(_ecore_x_disp, &xtp,
					   &list, &items);
#endif
	     if ((s == XLocaleNotSupported) ||
		 (s == XNoMemory) || (s == XConverterNotFound))
	       {
		  str = strdup((char *)xtp.value);
	       }
	     else if ((s >= Success) && (items > 0))
	       {
		  str = strdup(list[0]);
	       }
	     if (list)
	       XFreeStringList(list);
	  }
	XFree(xtp.value);
     }
   return str;
}

EAPI int
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
EAPI Ecore_X_WM_Protocol *
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
