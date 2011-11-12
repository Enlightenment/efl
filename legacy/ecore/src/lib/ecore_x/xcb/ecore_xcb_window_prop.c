#include "ecore_xcb_private.h"
#include <xcb/xcb_icccm.h>

EAPI int
ecore_x_window_prop_card32_get(Ecore_X_Window win,
                               Ecore_X_Atom   atom,
                               unsigned int  *val,
                               unsigned int   len)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, win, atom,
                                       ECORE_X_ATOM_CARDINAL, 0, 0x7fffffff);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return -1;

   if ((reply->type != ECORE_X_ATOM_CARDINAL) || (reply->format != 32))
     num = -1;
   else if (reply->value_len == 0)
     num = 0;
   else
     {
        if (reply->value_len < len)
          len = reply->value_len;

        if (val)
          {
             unsigned int i = 0;
             unsigned char *v;

             v = xcb_get_property_value(reply);
             for (i = 0; i < len; i++)
               val[i] = ((unsigned long *)v)[i];
             num = len;
          }
     }

   if (reply) free(reply);
   return num;
}

EAPI void
ecore_x_window_prop_card32_set(Ecore_X_Window win,
                               Ecore_X_Atom   atom,
                               unsigned int  *val,
                               unsigned int   num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#if SIZEOF_INT == SIZEOF_LONG
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, atom,
                       ECORE_X_ATOM_CARDINAL, 32, num, (unsigned char *)val);
//   ecore_x_flush();
#else
   long *v2;
   unsigned int i;

   v2 = malloc(num * sizeof(long));
   if (!v2) return;
   for (i = 0; i < num; i++)
     v2[i] = val[i];

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, atom,
                       ECORE_X_ATOM_CARDINAL, 32, num, (unsigned char *)v2);
   free(v2);
//   ecore_x_flush();
#endif
}

EAPI int
ecore_x_window_prop_card32_list_get(Ecore_X_Window win,
                                    Ecore_X_Atom   atom,
                                    unsigned int **list)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int num = -1;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (list) *list = NULL;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, win, atom,
                                       XCB_ATOM_CARDINAL, 0, 0x7fffffff);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return -1;

   if ((reply->type != XCB_ATOM_CARDINAL) || (reply->format != 32))
     num = -1;
   else if ((reply->value_len == 0) || (!xcb_get_property_value(reply)))
     num = 0;
   else
     {
        num = reply->value_len;
        if (list)
          {
             unsigned int *val;
             void *data;
             int i = 0;

             val = malloc(num * sizeof(unsigned int));
             if (!val)
               {
                  free(reply);
                  return -1;
               }
             data = xcb_get_property_value(reply);
             for (i = 0; i < num; i++)
               val[i] = ((unsigned long *)data)[i];
             *list = val;
          }
     }

   free(reply);
   return num;
}

EAPI int
ecore_x_window_prop_atom_get(Ecore_X_Window win,
                             Ecore_X_Atom   atom,
                             Ecore_X_Atom  *list,
                             unsigned int   len)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_window_prop_xid_get(win, atom, ECORE_X_ATOM_ATOM, list, len);
}

EAPI void
ecore_x_window_prop_atom_set(Ecore_X_Window win,
                             Ecore_X_Atom   atom,
                             Ecore_X_Atom  *list,
                             unsigned int   num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, atom,  */
   /*                     ECORE_X_ATOM_ATOM, 32, num, list); */
   ecore_x_window_prop_xid_set(win, atom, ECORE_X_ATOM_ATOM, list, num);
}

EAPI void
ecore_x_window_prop_xid_set(Ecore_X_Window win,
                            Ecore_X_Atom   atom,
                            Ecore_X_Atom   type,
                            Ecore_X_ID    *xids,
                            unsigned int   num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#if SIZEOF_INT == SIZEOF_LONG
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, atom,
                       type, 32, num, (unsigned char *)xids);
//   ecore_x_flush();
#else
   long *v2;
   unsigned int i;

   v2 = malloc(num * sizeof(long));
   if (!v2) return;
   for (i = 0; i < num; i++)
     v2[i] = xids[i];

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, atom,
                       type, 32, num, (unsigned char *)v2);
   free(v2);
//   ecore_x_flush();
#endif
}

EAPI int
ecore_x_window_prop_xid_get(Ecore_X_Window win,
                            Ecore_X_Atom   atom,
                            Ecore_X_Atom   type,
                            Ecore_X_ID    *xids,
                            unsigned int   len)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   num = len;
   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, win, atom, type,
                                       0, 0x7fffffff);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return -1;

   if ((reply->type != type) || (reply->format != 32))
     num = -1;
   else if (reply->value_len == 0)
     num = 0;
   else
     {
        unsigned int i = 0;
        unsigned char *v;

        if (reply->value_len < len)
          len = reply->value_len;

        v = xcb_get_property_value(reply);
        for (i = 0; i < len; i++)
          xids[i] = ((unsigned long *)v)[i];

        num = len;
     }

   if (reply) free(reply);
   return num;
}

EAPI void
ecore_x_window_prop_string_set(Ecore_X_Window win,
                               Ecore_X_Atom   type,
                               const char    *str)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win, type,
                       ECORE_X_ATOM_UTF8_STRING, 8, strlen(str), str);
//   ecore_x_flush();
}

EAPI char *
ecore_x_window_prop_string_get(Ecore_X_Window win,
                               Ecore_X_Atom   type)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   char *str = NULL;
   int len = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie =
     xcb_get_property_unchecked(_ecore_xcb_conn, 0,
                                win ? win : ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                type, XCB_GET_PROPERTY_TYPE_ANY, 0, 1000000L);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;

   len = ((reply->value_len * reply->format) / 8);
   str = (char *)malloc((len + 1) * sizeof(char));
   memcpy(str, xcb_get_property_value(reply), len);
   str[len] = '\0';

   if (reply->type != ECORE_X_ATOM_UTF8_STRING)
     {
        Ecore_Xcb_Textproperty prop;
        int count = 0;
        char **list = NULL;
        Eina_Bool ret = EINA_FALSE;

        prop.value = strdup(str);
        prop.nitems = len;
        prop.encoding = reply->type;

#ifdef HAVE_ICONV
        ret = _ecore_xcb_utf8_textproperty_to_textlist(&prop, &list, &count);
#else
        ret = _ecore_xcb_mb_textproperty_to_textlist(&prop, &list, &count);
#endif
        if (ret)
          {
             if (count > 0)
               str = strdup(list[0]);
             else
               str = strdup((char *)prop.value);

             if (list) free(list);
          }
        else
          str = strdup((char *)prop.value);
     }

   free(reply);
   return str;
}

EAPI int
ecore_x_window_prop_window_get(Ecore_X_Window  win,
                               Ecore_X_Atom    atom,
                               Ecore_X_Window *list,
                               unsigned int    len)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_window_prop_xid_get(win, atom, ECORE_X_ATOM_WINDOW, list, len);
}

EAPI void
ecore_x_window_prop_window_set(Ecore_X_Window  win,
                               Ecore_X_Atom    atom,
                               Ecore_X_Window *list,
                               unsigned int    num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_xid_set(win, atom, ECORE_X_ATOM_WINDOW, list, num);
}

EAPI int
ecore_x_window_prop_window_list_get(Ecore_X_Window   win,
                                    Ecore_X_Atom     atom,
                                    Ecore_X_Window **plst)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_window_prop_xid_list_get(win, atom, ECORE_X_ATOM_WINDOW, plst);
}

EAPI Ecore_X_Atom
ecore_x_window_prop_any_type(void)
{
   return XCB_ATOM_ANY;
}

EAPI void
ecore_x_window_prop_property_del(Ecore_X_Window win,
                                 Ecore_X_Atom   property)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_delete_property(_ecore_xcb_conn, win, property);
}

EAPI void
ecore_x_window_prop_property_set(Ecore_X_Window win,
                                 Ecore_X_Atom   property,
                                 Ecore_X_Atom   type,
                                 int            size,
                                 void          *data,
                                 int            num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (win == 0)
     win = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   if (size != 32)
     {
        xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                            property, type, size, num, (unsigned char *)data);
//        ecore_x_flush();
     }
   else
     {
        unsigned long *dat;
        int i = 0, *ptr;

        dat = malloc(sizeof(unsigned long) * num);
        if (dat)
          {
             for (ptr = (int *)data, i = 0; i < num; i++)
               dat[i] = ptr[i];
             xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                                 property, type, size, num,
                                 (unsigned char *)dat);
             free(dat);
//             ecore_x_flush();
          }
     }
}

EAPI int
ecore_x_window_prop_property_get(Ecore_X_Window  win,
                                 Ecore_X_Atom    property,
                                 Ecore_X_Atom    type,
                                 int             size,
                                 unsigned char **data,
                                 int            *num)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int format = 0;
   unsigned int i = 0;
   void *value;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num) *num = 0;

   if (data)
     *data = NULL;
   else
     return 0;

   if (win == 0)
     win = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   cookie =
     xcb_get_property_unchecked(_ecore_xcb_conn, 0, win,
                                property, type, 0, UINT_MAX);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   if ((reply->format != size) || (reply->value_len == 0))
     {
        free(reply);
        return 0;
     }

   if (!(*data = malloc(reply->value_len * reply->format / 8)))
     {
        free(reply);
        return 0;
     }

   value = xcb_get_property_value(reply);
   switch (reply->format)
     {
      case 8:
        for (i = 0; i < reply->value_len; i++)
          (*data)[i] = ((unsigned char *)value)[i];
        break;

      case 16:
        for (i = 0; i < reply->value_len; i++)
          ((unsigned short *)*data)[i] = ((unsigned short *)value)[i];
        break;

      case 32:
        for (i = 0; i < reply->value_len; i++)
          ((unsigned int *)*data)[i] = ((unsigned long *)value)[i];
        break;
     }

   if (num) *num = reply->value_len;
   format = reply->format;
   free(reply);
   return format;
}

EAPI int
ecore_x_window_prop_atom_list_get(Ecore_X_Window win,
                                  Ecore_X_Atom   atom,
                                  Ecore_X_Atom **list)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_window_prop_xid_list_get(win, atom, ECORE_X_ATOM_ATOM, list);
}

EAPI void
ecore_x_window_prop_atom_list_change(Ecore_X_Window win,
                                     Ecore_X_Atom   atom,
                                     Ecore_X_Atom   item,
                                     int            op)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_window_prop_xid_list_change(win, atom, ECORE_X_ATOM_ATOM, item, op);
}

EAPI int
ecore_x_window_prop_xid_list_get(Ecore_X_Window win,
                                 Ecore_X_Atom   atom,
                                 Ecore_X_Atom   type,
                                 Ecore_X_ID   **xids)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int num = -1;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (xids) *xids = NULL;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, win, atom, type,
                                       0, 0x7fffffff);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return -1;

   if ((reply->type != type) || (reply->format != 32))
     num = -1;
   else if ((reply->value_len == 0) || (!xcb_get_property_value(reply)))
     num = 0;
   else
     {
        Ecore_X_Atom *alst;
        void *val;

        num = xcb_get_property_value_length(reply);
        val = xcb_get_property_value(reply);
        alst = malloc(num * sizeof(Ecore_X_ID));
        if (alst)
          {
             int i = 0;

             for (i = 0; i < num; i++)
               alst[i] = ((unsigned long *)val)[i];
             *xids = alst;
          }
     }

   free(reply);
   return num;
}

EAPI void
ecore_x_window_prop_xid_list_change(Ecore_X_Window win,
                                    Ecore_X_Atom   atom,
                                    Ecore_X_Atom   type,
                                    Ecore_X_ID     item,
                                    int            op)
{
   Ecore_X_ID *lst;
   int i = 0, num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   num = ecore_x_window_prop_xid_list_get(win, atom, type, &lst);
   if (num < 0) return;

   for (i = 0; i < num; i++)
     {
        if (lst[i] == item) break;
     }

   if (i < num)
     {
        if (op == ECORE_X_PROP_LIST_ADD)
          goto done;
        num--;
        for (; i < num; i++)
          lst[i] = lst[i + 1];
     }
   else
     {
        if (op == ECORE_X_PROP_LIST_REMOVE)
          goto done;
        num++;
        lst = realloc(lst, num * sizeof(Ecore_X_ID));
        lst[i] = item;
     }
   ecore_x_window_prop_xid_set(win, atom, type, lst, num);

done:
   if (lst) free(lst);
}

EAPI Eina_Bool
ecore_x_window_prop_protocol_isset(Ecore_X_Window      win,
                                   Ecore_X_WM_Protocol protocol)
{
   Eina_Bool ret = EINA_FALSE;
   Ecore_X_Atom proto;
#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_t protos;
#else
   xcb_icccm_get_wm_protocols_reply_t protos;
#endif
   xcb_get_property_cookie_t cookie;
   uint8_t reply;
   uint32_t count = 0, i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (protocol >= ECORE_X_WM_PROTOCOL_NUM) return EINA_FALSE;

   proto = _ecore_xcb_atoms_wm_protocol[protocol];
#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_protocols_unchecked(_ecore_xcb_conn, win,
                                           ECORE_X_ATOM_WM_PROTOCOLS);
   reply = xcb_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &protos, NULL);
#else
   cookie = xcb_icccm_get_wm_protocols_unchecked(_ecore_xcb_conn, win,
                                                 ECORE_X_ATOM_WM_PROTOCOLS);
   reply = xcb_icccm_get_wm_protocols_reply(_ecore_xcb_conn, cookie,
                                            &protos, NULL);
#endif
   if (!reply) return EINA_FALSE;

   count = protos.atoms_len;
   for (i = 0; i < count; i++)
     {
        if (protos.atoms[i] == proto)
          {
             ret = EINA_TRUE;
             break;
          }
     }

#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_wipe(&protos);
#else
   xcb_icccm_get_wm_protocols_reply_wipe(&protos);
#endif
   return ret;
}

EAPI Ecore_X_WM_Protocol *
ecore_x_window_prop_protocol_list_get(Ecore_X_Window win,
                                      int           *num_ret)
{
#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_t protos;
#else
   xcb_icccm_get_wm_protocols_reply_t protos;
#endif
   xcb_get_property_cookie_t cookie;
   uint8_t reply;
   uint32_t count = 0, i = 0;
   Ecore_X_WM_Protocol *prot_ret = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!num_ret) return NULL;

   *num_ret = 0;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_protocols_unchecked(_ecore_xcb_conn, win,
                                           ECORE_X_ATOM_WM_PROTOCOLS);
   reply = xcb_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &protos, NULL);
#else
   cookie = xcb_icccm_get_wm_protocols_unchecked(_ecore_xcb_conn, win,
                                                 ECORE_X_ATOM_WM_PROTOCOLS);
   reply = xcb_icccm_get_wm_protocols_reply(_ecore_xcb_conn, cookie,
                                            &protos, NULL);
#endif
   if (!reply) return NULL;

   count = protos.atoms_len;
   if (count <= 0)
     {
#ifdef OLD_XCB_VERSION
        xcb_get_wm_protocols_reply_wipe(&protos);
#else
        xcb_icccm_get_wm_protocols_reply_wipe(&protos);
#endif
        return NULL;
     }

   prot_ret = calloc(1, count * sizeof(Ecore_X_WM_Protocol));
   if (!prot_ret)
     {
#ifdef OLD_XCB_VERSION
        xcb_get_wm_protocols_reply_wipe(&protos);
#else
        xcb_icccm_get_wm_protocols_reply_wipe(&protos);
#endif
        return NULL;
     }

   for (i = 0; i < count; i++)
     {
        Ecore_X_WM_Protocol j;

        prot_ret[i] = -1;
        for (j = 0; j < ECORE_X_WM_PROTOCOL_NUM; j++)
          {
             if (_ecore_xcb_atoms_wm_protocol[j] == protos.atoms[i])
               prot_ret[i] = j;
          }
     }

   if (num_ret) *num_ret = count;

#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_wipe(&protos);
#else
   xcb_icccm_get_wm_protocols_reply_wipe(&protos);
#endif
   return prot_ret;
}

EAPI Ecore_X_Atom *
ecore_x_window_prop_list(Ecore_X_Window win,
                         int           *num)
{
   xcb_list_properties_cookie_t cookie;
   xcb_list_properties_reply_t *reply;
   xcb_atom_t *atm;
   Ecore_X_Atom *atoms;
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num) *num = 0;

   cookie = xcb_list_properties_unchecked(_ecore_xcb_conn, win);
   reply = xcb_list_properties_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;

   atoms = (Ecore_X_Atom *)malloc(reply->atoms_len * sizeof(Ecore_X_Atom));
   if (!atoms)
     {
        free(reply);
        return NULL;
     }

   atm = xcb_list_properties_atoms(reply);
   for (i = 0; i < reply->atoms_len; i++)
     atoms[i] = atm[i];

   if (num) *num = reply->atoms_len;
   free(reply);

   return atoms;
}

