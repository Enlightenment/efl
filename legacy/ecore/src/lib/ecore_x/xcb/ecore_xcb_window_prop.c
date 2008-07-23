/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"
#include "Ecore_X_Atoms.h"


/*
 * Set CARD32 (array) property
 */
EAPI void
ecore_x_window_prop_card32_set(Ecore_X_Window win,
                               Ecore_X_Atom   atom,
			       unsigned int  *val,
                               unsigned int   num)
{
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                       atom, ECORE_X_ATOM_CARDINAL, 32, num, (const void *)val);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @param atom   The atom.
 */
EAPI void
ecore_x_window_prop_card32_get_prefetch(Ecore_X_Window window,
                                        Ecore_X_Atom atom)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window,
                                       atom,
                                       ECORE_X_ATOM_CARDINAL,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_card32_get_prefetch().
 */
EAPI void
ecore_x_window_prop_card32_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
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
ecore_x_window_prop_card32_get(Ecore_X_Window win __UNUSED__,
                               Ecore_X_Atom   atom __UNUSED__,
			       unsigned int  *val,
                               unsigned int   len)
{
   xcb_get_property_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply ||
       (reply->type != ECORE_X_ATOM_CARDINAL) ||
       (reply->format != 32))
      return -1;

   if (reply->value_len < len)
     len = xcb_get_property_value_length(reply);

   if (val)
      memcpy(val, xcb_get_property_value(reply), len);

   return (int)len;
}

/*
 * Get CARD32 (array) property of any length
 *
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
EAPI int
ecore_x_window_prop_card32_list_get(Ecore_X_Window win __UNUSED__,
                                    Ecore_X_Atom   atom __UNUSED__,
				    unsigned int **plist)
{
   xcb_get_property_reply_t *reply;
   int                       num = -1;

   if (plist)
      *plist = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return -1;

   if ((reply->type == XCB_NONE) ||
       (reply->value_len == 0))
      num = 0;
   else if ((reply->type == ECORE_X_ATOM_CARDINAL) &&
            (reply->format == 32))
     {
       uint32_t *val;

       num = xcb_get_property_value_length(reply);
       if (plist)
         {
            val = (uint32_t *)malloc (num);
            if (!val)
               goto error;

            memcpy(val, xcb_get_property_value(reply), num);
            *plist = val;
         }
     }

 error:

   return num;
}

/*
 * Set X ID (array) property
 */
EAPI void
ecore_x_window_prop_xid_set(Ecore_X_Window win,
                            Ecore_X_Atom   atom,
			    Ecore_X_Atom   type,
                            Ecore_X_ID    *xids,
			    unsigned int   num)
{
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                       atom, type, 32, num, xids);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @param atom   The atom.
 * @param type   The atom type.
 */
EAPI void
ecore_x_window_prop_xid_get_prefetch(Ecore_X_Window window,
                                     Ecore_X_Atom   atom,
                                     Ecore_X_Atom   type)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window,
                                       atom,
                                       type,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_xid_get_prefetch().
 */
EAPI void
ecore_x_window_prop_xid_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
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
ecore_x_window_prop_xid_get(Ecore_X_Window win __UNUSED__,
                            Ecore_X_Atom   atom __UNUSED__,
			    Ecore_X_Atom   type __UNUSED__,
                            Ecore_X_ID    *xids,
			    unsigned int   len)
{
   xcb_get_property_reply_t *reply;
   int                       num = len;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return -1;

   if (reply->type == XCB_NONE)
      num = 0;
   else if (reply->format == 32)
     {
        if (reply->value_len < len)
          num = xcb_get_property_value_length(reply);

        if (xids)
           memcpy(xids, xcb_get_property_value(reply), num);
     }

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
ecore_x_window_prop_xid_list_get(Ecore_X_Window win __UNUSED__,
                                 Ecore_X_Atom   atom __UNUSED__,
				 Ecore_X_Atom   type __UNUSED__,
                                 Ecore_X_ID   **pxids)
{
   xcb_get_property_reply_t *reply;
   int                       num = -1;

   if (pxids)
      *pxids = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return -1;

   if ((reply->type == XCB_NONE) ||
       (reply->value_len == 0))
      num = 0;
   else if ((reply->type == ECORE_X_ATOM_CARDINAL) &&
            (reply->format == 32))
     {
       uint32_t *val;

       num = xcb_get_property_value_length(reply);
       if (pxids)
         {
            val = (uint32_t *)malloc (num);
            if (!val)
               return -1;

            memcpy(val, xcb_get_property_value(reply), num);
            *pxids = val;
         }
     }

   return num;
}

/*
 * Remove/add/toggle X ID list item.
 */
EAPI void
ecore_x_window_prop_xid_list_change(Ecore_X_Window win,
                                    Ecore_X_Atom   atom,
                                    Ecore_X_Atom   type,
                                    Ecore_X_ID     item,
                                    int            op)
{
   Ecore_X_ID *lst;
   int         i;
   int         num;

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
ecore_x_window_prop_atom_set(Ecore_X_Window win,
                             Ecore_X_Atom   atom,
                             Ecore_X_Atom  *list,
                             unsigned int   num)
{
   ecore_x_window_prop_xid_set(win, atom, ECORE_X_ATOM_ATOM, list, num);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @param atom   Property atom.
 */
EAPI void
ecore_x_window_prop_atom_get_prefetch(Ecore_X_Window window,
                                      Ecore_X_Atom   atom)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window,
                                       atom,
                                       ECORE_X_ATOM_ATOM,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_atom_get_prefetch().
 */
EAPI void
ecore_x_window_prop_atom_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
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
                             Ecore_X_Atom   atom,
                             Ecore_X_Atom  *list,
                             unsigned int len)
{
   return ecore_x_window_prop_xid_get(win, atom, ECORE_X_ATOM_ATOM, list, len);
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
                                  Ecore_X_Atom   atom,
                                  Ecore_X_Atom **plist)
{
   return ecore_x_window_prop_xid_list_get(win, atom, ECORE_X_ATOM_ATOM, plist);
}

/*
 * Remove/add/toggle atom list item.
 */
EAPI void
ecore_x_window_prop_atom_list_change(Ecore_X_Window win,
                                     Ecore_X_Atom   atom,
                                     Ecore_X_Atom   item,
                                     int            op)
{
   ecore_x_window_prop_xid_list_change(win, atom, ECORE_X_ATOM_ATOM, item, op);
}

/*
 * Set Window (array) property
 */
EAPI void
ecore_x_window_prop_window_set(Ecore_X_Window  win,
                               Ecore_X_Atom    atom,
                               Ecore_X_Window *list,
                               unsigned int    num)
{
   ecore_x_window_prop_xid_set(win, atom, ECORE_X_ATOM_WINDOW, list, num);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @param atom   The atom.
 */
EAPI void
ecore_x_window_prop_window_get_prefetch(Ecore_X_Window window,
                                        Ecore_X_Atom   atom)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window,
                                       atom,
                                       ECORE_X_ATOM_WINDOW,
                                       0, 0x7fffffff);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_window_get_prefetch().
 */
EAPI void
ecore_x_window_prop_window_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
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
ecore_x_window_prop_window_get(Ecore_X_Window  win,
                               Ecore_X_Atom    atom,
                               Ecore_X_Window *list,
                               unsigned int    len)
{
   return ecore_x_window_prop_xid_get(win, atom, ECORE_X_ATOM_WINDOW, list, len);
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
ecore_x_window_prop_window_list_get(Ecore_X_Window   win,
                                    Ecore_X_Atom     atom,
                                    Ecore_X_Window **plist)
{
   return ecore_x_window_prop_xid_list_get(win, atom, ECORE_X_ATOM_WINDOW, plist);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_X_Atom
ecore_x_window_prop_any_type(void)
{
   return XCB_GET_PROPERTY_TYPE_ANY;
}

/**
 * To be documented.
 * @param window   The window.
 * @param property The property atom.
 * @param type     The type atom.
 * @param size     The size.
 * @param data     The data.
 * @param number   The size of the data.
 *
 * FIXME: To be fixed.
 */
EAPI void
ecore_x_window_prop_property_set(Ecore_X_Window window,
                                 Ecore_X_Atom   property,
                                 Ecore_X_Atom   type,
                                 int            size,
                                 void          *data,
                                 int            number)
{
   if (window == 0) window = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, window,
                       property, type,
                       size, number, data);
}

/**
 * Sends the GetProperty request.
 * @param window   Window whose properties are requested.
 * @param property Property atom.
 * @param type     Type atom.
 */
EAPI void
ecore_x_window_prop_property_get_prefetch(Ecore_X_Window window,
                                          Ecore_X_Atom   property,
                                          Ecore_X_Atom   type)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       property, type, 0, LONG_MAX);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_property_get_prefetch().
 */
EAPI void
ecore_x_window_prop_property_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * To be documented.
 * @param window   The window (Unused).
 * @param property The property atom (Unused).
 * @param type     The type atom (Unused).
 * @param size     The size (Unused).
 * @param data     The returned data.
 * @param num      The size of the data.
 * @return         1 on success, 0 otherwise.
 *
 * FIXME: To be fixed.
 */
EAPI int
ecore_x_window_prop_property_get(Ecore_X_Window  window __UNUSED__,
                                 Ecore_X_Atom    property __UNUSED__,
                                 Ecore_X_Atom    type __UNUSED__,
                                 int             size __UNUSED__,
                                 unsigned char **data,
                                 int            *num)
{
   xcb_get_property_reply_t *reply;

   /* make sure these are initialized */
   if (num) *num = 0L;

   if (data)
     *data = NULL;
   else /* we can't store the retrieved data, so just return */
     return 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return 0;

   if ((reply->format != size) ||
       (reply->value_len == 0))
      return 0;

   *data = malloc(reply->value_len);
   if (!*data)
      return 0;

   memcpy(*data, xcb_get_property_value(reply), 
          xcb_get_property_value_length(reply));

   if (num)
      *num = reply->value_len;

   return reply->format;
}

EAPI void
ecore_x_window_prop_property_del(Ecore_X_Window window,
                                 Ecore_X_Atom   property)
{
   xcb_delete_property(_ecore_xcb_conn, window, property);
}

/**
 * Sends the ListProperties request.
 * @param window Window whose properties are requested.
 */
EAPI void
ecore_x_window_prop_list_prefetch(Ecore_X_Window window)
{
   xcb_list_properties_cookie_t cookie;

   cookie = xcb_list_properties_unchecked(_ecore_xcb_conn, window);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the ListProperties request sent by ecore_x_window_prop_list_prefetch().
 */
EAPI void
ecore_x_window_prop_list_fetch(void)
{
   xcb_list_properties_cookie_t cookie;
   xcb_list_properties_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_list_properties_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}


/**
 * To be documented.
 * @param window  The window (Unused).
 * @param num_ret The number of atoms.
 * @return        The returned atoms.
 *
 * FIXME: To be fixed.
 */
EAPI Ecore_X_Atom *
ecore_x_window_prop_list(Ecore_X_Window window __UNUSED__,
                         int           *num_ret)
{
   xcb_list_properties_reply_t *reply;
   Ecore_X_Atom                *atoms;

   if (num_ret) *num_ret = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return NULL;

   atoms = (Ecore_X_Atom *)malloc(reply->atoms_len * sizeof(Ecore_X_Atom));
   if (!atoms)
      return NULL;
   memcpy(atoms,
          xcb_list_properties_atoms(reply),
          reply->atoms_len * sizeof(Ecore_X_Atom));
   if(num_ret)
     *num_ret = reply->atoms_len;

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
                               Ecore_X_Atom   type,
                               const char    *str)
{
   if (win == 0) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                     type, ECORE_X_ATOM_UTF8_STRING,
                     8, strlen(str), str);
}

/**
 * Sends the GetProperty request.
 * @param window Window whose properties are requested.
 * @param type   The atom.
 */
EAPI void
ecore_x_window_prop_string_get_prefetch(Ecore_X_Window window,
                                        Ecore_X_Atom   type)
{
   xcb_get_property_cookie_t cookie;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                       window ? window : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                       type, XCB_GET_PROPERTY_TYPE_ANY, 0L, 1000000L);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetProperty request sent by ecore_x_window_prop_string_get_prefetch().
 */
EAPI void
ecore_x_window_prop_string_get_fetch(void)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get a window string property.
 * @param window The window
 * @param type The property
 *
 * Return window string property of a window. String must be free'd when done.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_window_prop_string_get_prefetch(), which sends the GetProperty request,
 * then ecore_x_window_prop_string_get_fetch(), which gets the reply.
 */
EAPI char *
ecore_x_window_prop_string_get(Ecore_X_Window window __UNUSED__,
                               Ecore_X_Atom   type __UNUSED__)
{
   xcb_get_property_reply_t *reply;
   char                     *str = NULL;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return NULL;

   if (reply->type == ECORE_X_ATOM_UTF8_STRING)
     {
        int length;

        length = reply->value_len;
        str = (char *)malloc(length + 1);
        memcpy(str,
               xcb_get_property_value(reply),
               length);
        str[length] = '\0';
     }
   else
     {
       /* FIXME: to be done... */

/* #ifdef X_HAVE_UTF8_STRING */
/*         s = Xutf8TextPropertyToTextList(_ecore_xcb_conn, &xtp, */
/*                                         &list, &items); */
/* #else */
/*         s = XmbTextPropertyToTextList(_ecore_xcb_conn, &xtp, */
/*                                       &list, &items); */
/* #endif */
/*         if ((s == XLocaleNotSupported) || */
/*             (s == XNoMemory) || (s == XConverterNotFound)) */
/*           { */
/*              str = strdup((char *)xtp.value); */
/*           } */
/*         else if ((s >= Success) && (items > 0)) */
/*           { */
/*              str = strdup(list[0]); */
/*           } */
/*         if (list) */
/*            XFreeStringList(list); */
     }

   return str;
}

/* FIXME : round trips because of GetWMProtocols */
/*         should we rewrite its code ? */
EAPI int
ecore_x_window_prop_protocol_isset(Ecore_X_Window      window,
                                   Ecore_X_WM_Protocol protocol)
{
   Ecore_X_Atom *protos;
   Ecore_X_Atom  proto;
   uint32_t      protos_count;
   uint32_t      i;
   uint8_t       ret = 0;

   /* check for invalid values */
   if (protocol >= ECORE_X_WM_PROTOCOL_NUM)
	return ret;

   proto = _ecore_xcb_atoms_wm_protocols[protocol];

   if (!xcb_get_wm_protocols(_ecore_xcb_conn, window, &protos_count, &protos))
	return ret;

   for (i = 0; i < protos_count; i++)
	if (protos[i] == proto)
	  {
	     ret = 1;
	     break;
	  }

   free(protos);

   return ret;
}

/**
 * To be documented.
 * @param window  The window.
 * @param num_ret The number of WM protocols.
 * @return        The returned WM protocols.
 *
 * FIXME: To be fixed.
 */

/* FIXME : round trips because of get_wm_protocols */
/*         should we rewrite its code ? */

EAPI Ecore_X_WM_Protocol *
ecore_x_window_prop_protocol_list_get(Ecore_X_Window window,
                                      int           *num_ret)
{
   Ecore_X_WM_Protocol *prot_ret = NULL;
   Ecore_X_Atom        *protos;
   uint32_t             protos_count;
   uint32_t             i;

   if (!xcb_get_wm_protocols(_ecore_xcb_conn, window, &protos_count, &protos))
     return NULL;

   if ((!protos) || (protos_count <= 0)) return NULL;

   prot_ret = calloc(1, protos_count * sizeof(Ecore_X_WM_Protocol));
   if (!prot_ret)
     {
	free(protos);
	return NULL;
     }
   for (i = 0; i < protos_count; i++)
     {
	Ecore_X_WM_Protocol j;

	prot_ret[i] = -1;
	for (j = 0; j < ECORE_X_WM_PROTOCOL_NUM; j++)
	  {
	     if (_ecore_xcb_atoms_wm_protocols[j] == protos[i])
	       prot_ret[i] = j;
	  }
     }
   free(protos);
   *num_ret = protos_count;

   return prot_ret;
}
