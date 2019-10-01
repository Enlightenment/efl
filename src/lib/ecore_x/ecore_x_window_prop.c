#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"
#include <inttypes.h>
#include <limits.h>

#define _ATOM_SET_CARD32(win, atom, p_val, cnt)                               \
  XChangeProperty(_ecore_x_disp, win, atom, XA_CARDINAL, 32, PropModeReplace, \
                  (unsigned char *)p_val, cnt)

/*
 * Set CARD32 (array) property
 */
EAPI void
ecore_x_window_prop_card32_set(Ecore_X_Window win,
                               Ecore_X_Atom atom,
                               unsigned int *val,
                               unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   _ATOM_SET_CARD32(win, atom, val, num);
#else /* if SIZEOF_INT == SIZEOF_LONG */
   long *v2;
   unsigned int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   v2 = malloc(num * sizeof(long));
   if (!v2)
     return;

   for (i = 0; i < num; i++)
     v2[i] = val[i];
   _ATOM_SET_CARD32(win, atom, v2, num);
   free(v2);
#endif /* if SIZEOF_INT == SIZEOF_LONG */
   if (_ecore_xlib_sync) ecore_x_sync();
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
ecore_x_window_prop_card32_get(Ecore_X_Window win,
                               Ecore_X_Atom atom,
                               unsigned int *val,
                               unsigned int len)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   unsigned int i;
   int num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          XA_CARDINAL, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if (type_ret != XA_CARDINAL || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else
     {
        if (num_ret < len)
          len = num_ret;

        if (val)
          for (i = 0; i < len; i++)
            val[i] = ((unsigned long *)prop_ret)[i];
        num = len;
     }

   if (_ecore_xlib_sync) ecore_x_sync();
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
ecore_x_window_prop_card32_list_get(Ecore_X_Window win,
                                    Ecore_X_Atom atom,
                                    unsigned int **plst)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   unsigned int i, *val;
   int num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (plst) *plst = NULL;
   prop_ret = NULL;
   if (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          XA_CARDINAL, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) != Success)
     return -1;

   if ((type_ret != XA_CARDINAL) || (format_ret != 32))
     num = -1;
   else if ((num_ret == 0) || (!prop_ret))
     num = 0;
   else if (plst)
     {
        val = malloc(num_ret * sizeof(unsigned int));
        if (!val)
          {
             if (prop_ret) XFree(prop_ret);
             return -1;
          }
        for (i = 0; i < num_ret; i++)
          val[i] = ((unsigned long *)prop_ret)[i];
        num = num_ret;
        *plst = val;
     }
   else
     num = num_ret;

   if (_ecore_xlib_sync) ecore_x_sync();
   if (prop_ret)
     XFree(prop_ret);
   return num;
}

/*
 * Set X ID (array) property
 */
EAPI void
ecore_x_window_prop_xid_set(Ecore_X_Window win,
                            Ecore_X_Atom atom,
                            Ecore_X_Atom type,
                            Ecore_X_ID *lst,
                            unsigned int num)
{
#if SIZEOF_INT == SIZEOF_LONG
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
                   (unsigned char *)lst, num);
#else /* if SIZEOF_INT == SIZEOF_LONG */
   unsigned long *pl;
   unsigned int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   pl = malloc(num * sizeof(unsigned long));
   if (!pl)
     return;

   for (i = 0; i < num; i++)
     pl[i] = lst[i];
   XChangeProperty(_ecore_x_disp, win, atom, type, 32, PropModeReplace,
                   (unsigned char *)pl, num);
   free(pl);
#endif /* if SIZEOF_INT == SIZEOF_LONG */
   if (_ecore_xlib_sync) ecore_x_sync();
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
ecore_x_window_prop_xid_get(Ecore_X_Window win,
                            Ecore_X_Atom atom,
                            Ecore_X_Atom type,
                            Ecore_X_ID *lst,
                            unsigned int len)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   int num;
   unsigned i;
   Eina_Bool success;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   prop_ret = NULL;
   success = (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          type, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) == Success);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!success) return -1;

   if (type_ret != type || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else
     {
        if (num_ret < len)
          len = num_ret;

        if (lst)
          for (i = 0; i < len; i++)
            lst[i] = ((unsigned long *)prop_ret)[i];
        num = len;
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
ecore_x_window_prop_xid_list_get(Ecore_X_Window win,
                                 Ecore_X_Atom atom,
                                 Ecore_X_Atom type,
                                 Ecore_X_ID **val)
{
   unsigned char *prop_ret;
   Atom type_ret;
   unsigned long bytes_after, num_ret;
   int format_ret;
   Ecore_X_Atom *alst;
   int num;
   unsigned i;
   Eina_Bool success;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (val) *val = NULL;
   prop_ret = NULL;
   success = (XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
                          type, &type_ret, &format_ret, &num_ret,
                          &bytes_after, &prop_ret) == Success);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!success) return -1;

   if (type_ret != type || format_ret != 32)
     num = -1;
   else if (num_ret == 0 || !prop_ret)
     num = 0;
   else if (val)
     {
        alst = malloc(num_ret * sizeof(Ecore_X_ID));
        for (i = 0; i < num_ret; i++)
          alst[i] = ((unsigned long *)prop_ret)[i];
        num = num_ret;
        *val = alst;
     }
   else
     num = num_ret;

   if (prop_ret)
     XFree(prop_ret);
   return num;
}

/*
 * Remove/add/toggle X ID list item.
 */
EAPI void
ecore_x_window_prop_xid_list_change(Ecore_X_Window win,
                                    Ecore_X_Atom atom,
                                    Ecore_X_Atom type,
                                    Ecore_X_ID item,
                                    int op)
{
   Ecore_X_ID *lst, *temp;
   int i, num;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   num = ecore_x_window_prop_xid_list_get(win, atom, type, &lst);
   if (num < 0)
     {
        return; /* Error - assuming invalid window */
     }

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
          goto done;  /* Remove it */

        num--;
        for (; i < num; i++)
          lst[i] = lst[i + 1];
     }
   else
     {
        /* Was not in list */
        if (op == ECORE_X_PROP_LIST_REMOVE)
          goto done;  /* Add it */

        num++;

        temp = lst;
        lst = realloc(lst, num * sizeof(Ecore_X_ID));
        if (lst)
          {
             lst[i] = item;
          }
        else
          {
             lst = temp;
             num--;
          }
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
ecore_x_window_prop_atom_set(Ecore_X_Window win,
                             Ecore_X_Atom atom,
                             Ecore_X_Atom *lst,
                             unsigned int num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
ecore_x_window_prop_atom_get(Ecore_X_Window win,
                             Ecore_X_Atom atom,
                             Ecore_X_Atom *lst,
                             unsigned int len)
{
   int ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = ecore_x_window_prop_xid_get(win, atom, XA_ATOM, lst, len);
   return ret;
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
ecore_x_window_prop_atom_list_get(Ecore_X_Window win,
                                  Ecore_X_Atom atom,
                                  Ecore_X_Atom **plst)
{
   int ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = ecore_x_window_prop_xid_list_get(win, atom, XA_ATOM, plst);
   return ret;
}

/*
 * Remove/add/toggle atom list item.
 */
EAPI void
ecore_x_window_prop_atom_list_change(Ecore_X_Window win,
                                     Ecore_X_Atom atom,
                                     Ecore_X_Atom item,
                                     int op)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_xid_list_change(win, atom, XA_ATOM, item, op);
}

/*
 * Set Window (array) property
 */
EAPI void
ecore_x_window_prop_window_set(Ecore_X_Window win,
                               Ecore_X_Atom atom,
                               Ecore_X_Window *lst,
                               unsigned int num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
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
ecore_x_window_prop_window_get(Ecore_X_Window win,
                               Ecore_X_Atom atom,
                               Ecore_X_Window *lst,
                               unsigned int len)
{
   int ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = ecore_x_window_prop_xid_get(win, atom, XA_WINDOW, lst, len);
   return ret;
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
ecore_x_window_prop_window_list_get(Ecore_X_Window win,
                                    Ecore_X_Atom atom,
                                    Ecore_X_Window **plst)
{
   int ret;
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ret = ecore_x_window_prop_xid_list_get(win, atom, XA_WINDOW, plst);
   return ret;
}

EAPI Ecore_X_Atom
ecore_x_window_prop_any_type(void)
{
   return AnyPropertyType;
}

/**
 * @brief Set a property of Ecore_X_Window.
 * @param win The window for which the property will be set.
 * @param property The property of the window to be set.
 * @param type The type of the property that will be set.
 * @param size The size of the property that will be set.
 * @param data The data of the property that will be set.
 * @param number The size of data.
 */
EAPI void
ecore_x_window_prop_property_set(Ecore_X_Window win,
                                 Ecore_X_Atom property,
                                 Ecore_X_Atom type,
                                 int size,
                                 void *data,
                                 int number)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (win == 0)
     win = DefaultRootWindow(_ecore_x_disp);

   if (size != 32)
     XChangeProperty(_ecore_x_disp,
                     win,
                     property,
                     type,
                     size,
                     PropModeReplace,
                     (unsigned char *)data,
                     number);
   else
     {
        unsigned long *dat;
        int i, *ptr;

        dat = malloc(sizeof(unsigned long) * number);
        if (dat)
          {
             for (ptr = (int *)data, i = 0; i < number; i++)
               dat[i] = ptr[i];
             XChangeProperty(_ecore_x_disp, win, property, type, size,
                             PropModeReplace, (unsigned char *)dat, number);
             free(dat);
          }
     }
   if (_ecore_xlib_sync) ecore_x_sync();
}

/**
 * @brief Get a property of Ecore_X_Window.
 * @note If there aren't any data to be got the function return NULL.
 *       If the function can't allocate the memory then 0 is returned.
 * @param win The window for which the property will be got.
 * @param property The property of the window that will be gotten.
 * @param type The type of the property that will be gotten.
 * @param size This parameter isn't in use.
 * @param data The data of the property that will be gotten.
 * @param num The size of property.
 * @return size_ret The size of array that contains the property.
 */
EAPI int
ecore_x_window_prop_property_get(Ecore_X_Window win,
                                 Ecore_X_Atom property,
                                 Ecore_X_Atom type,
                                 int size EINA_UNUSED,
                                 unsigned char **data,
                                 int *num)
{
   Atom type_ret = 0;
   int ret, size_ret = 0;
   unsigned long num_ret = 0, bytes = 0, i;
   unsigned char *prop_ret = NULL;

   /* make sure these are initialized */
   if (num)
     *num = 0;

   if (data)
     *data = NULL;
   else /* we can't store the retrieved data, so just return */
     return 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!win)
     win = DefaultRootWindow(_ecore_x_disp);

   ret = XGetWindowProperty(_ecore_x_disp, win, property, 0, LONG_MAX,
                            False, type, &type_ret, &size_ret,
                            &num_ret, &bytes, &prop_ret);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (ret != Success)
     return 0;
   if ((!num_ret) || (size_ret <= 0))
     {
        XFree(prop_ret);
        return 0;
     }

   if (!(*data = malloc(num_ret * size_ret / 8)))
     {
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
          ((unsigned short *)*data)[i] = ((unsigned short *)prop_ret)[i];
        break;

      case 32:
        for (i = 0; i < num_ret; i++)
          ((unsigned int *)*data)[i] = ((unsigned long *)prop_ret)[i];
        break;
     }

   XFree(prop_ret);

   if (num)
     *num = num_ret;

   return size_ret;
}

EAPI void
ecore_x_window_prop_property_del(Ecore_X_Window win,
                                 Ecore_X_Atom property)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XDeleteProperty(_ecore_x_disp, win, property);
   if (_ecore_xlib_sync) ecore_x_sync();
}

EAPI Ecore_X_Atom *
ecore_x_window_prop_list(Ecore_X_Window win,
                         int *num_ret)
{
   Ecore_X_Atom *atoms;
   Atom *atom_ret;
   int num = 0, i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num_ret)
     *num_ret = 0;

   atom_ret = XListProperties(_ecore_x_disp, win, &num);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!atom_ret)
     return NULL;

   atoms = malloc(num * sizeof(Ecore_X_Atom));
   if (atoms)
     {
        for (i = 0; i < num; i++)
          atoms[i] = atom_ret[i];
        if (num_ret)
          *num_ret = num;
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
ecore_x_window_prop_string_set(Ecore_X_Window win,
                               Ecore_X_Atom type,
                               const char *str)
{
   XTextProperty xtp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (win == 0)
     win = DefaultRootWindow(_ecore_x_disp);

   xtp.value = (unsigned char *)str;
   xtp.format = 8;
   xtp.encoding = ECORE_X_ATOM_UTF8_STRING;
   xtp.nitems = strlen(str);
   XSetTextProperty(_ecore_x_disp, win, &xtp, type);
   if (_ecore_xlib_sync) ecore_x_sync();
}

/**
 * Get a window string property.
 * @param win The window
 * @param type The property
 * @return Window string property of a window. String must be free'd when done.
 */
EAPI char *
ecore_x_window_prop_string_get(Ecore_X_Window win,
                               Ecore_X_Atom type)
{
   XTextProperty xtp;
   char *str = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (win == 0)
     win = DefaultRootWindow(_ecore_x_disp);

   if (XGetTextProperty(_ecore_x_disp, win, &xtp, type))
     {
        int items;
        char **list = NULL;
        Status s;

        if (_ecore_xlib_sync) ecore_x_sync();
        if (xtp.encoding == ECORE_X_ATOM_UTF8_STRING)
          str = strdup((char *)xtp.value);
        else
          {
#ifdef X_HAVE_UTF8_STRING
             s = Xutf8TextPropertyToTextList(_ecore_x_disp, &xtp,
                                             &list, &items);
#else /* ifdef X_HAVE_UTF8_STRING */
             s = XmbTextPropertyToTextList(_ecore_x_disp, &xtp,
                                           &list, &items);
#endif /* ifdef X_HAVE_UTF8_STRING */
             if (_ecore_xlib_sync) ecore_x_sync();
             if ((s == XLocaleNotSupported) ||
                 (s == XNoMemory) || (s == XConverterNotFound))
               str = strdup((char *)xtp.value);
             else if ((s >= Success) && (items > 0))
               str = strdup(list[0]);

             if (list)
               XFreeStringList(list);
          }

        XFree(xtp.value);
     }
   return str;
}

EAPI Eina_Bool
ecore_x_window_prop_protocol_isset(Ecore_X_Window win,
                                   Ecore_X_WM_Protocol protocol)
{
   Atom proto, *protos = NULL;
   int i, protos_count = 0;
   Eina_Bool ret = EINA_FALSE;

   /* check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   proto = _ecore_x_atoms_wm_protocols[protocol];

   ret = XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!ret)
     return ret;

   for (i = 0; i < protos_count; i++)
     if (protos[i] == proto)
       {
          ret = EINA_TRUE;
          break;
       }

   XFree(protos);
   return ret;
}

/**
 * @brief Get a array containing the protocols of @a win
 * @note If there aren't any properties to be counted or any protocols to get
 *       then the function returns NULL.
 * @param win The window for which protocol list will be got.
 * @param num_ret Contains the number of elements of the array to be returned.
 * @return The array that contains the protocols.
 */
EAPI Ecore_X_WM_Protocol *
ecore_x_window_prop_protocol_list_get(Ecore_X_Window win,
                                      int *num_ret)
{
   Atom *protos = NULL;
   int i, protos_count = 0;
   Ecore_X_WM_Protocol *prot_ret = NULL;
   Eina_Bool success;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   success = XGetWMProtocols(_ecore_x_disp, win, &protos, &protos_count);
   if (_ecore_xlib_sync) ecore_x_sync();
   if (!success)
     return NULL;

   if ((!protos) || (protos_count <= 0))
     return NULL;

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

