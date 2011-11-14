#include "ecore_xcb_private.h"
#include <xcb/xcb_icccm.h>

EAPI void
ecore_x_icccm_init(void)
{
}

/**
 * Sets the WM_COMMAND property for @a win.
 *
 * @param win  The window.
 * @param argc Number of arguments.
 * @param argv Arguments.
 */
EAPI void
ecore_x_icccm_command_set(Ecore_X_Window win,
                          int            argc,
                          char         **argv)
{
   void *buf;
   char *b;
   int nbytes, i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   for (i = 0, nbytes = 0; i < argc; i++) 
     if (argv[i]) nbytes += strlen(argv[i]) + 1;

   buf = malloc(sizeof(char) * nbytes);
   if (!buf) return;

   b = (char *)buf;
   for (i = 0; i < argc; i++)
     {
        if (argv[i])
          {
             strcpy(b, argv[i]);
             b += strlen(argv[i]) + 1;
          }
        else
          *b++ = '\0';
     }
   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                       ECORE_X_ATOM_WM_COMMAND, ECORE_X_ATOM_STRING, 8,
                       nbytes, buf);
   free(buf);
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
ecore_x_icccm_command_get(Ecore_X_Window win,
                          int           *argc,
                          char        ***argv)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   int len = 0;
   char **v, *data, *cp, *start;
   int c = 0, i = 0, j = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (argc) *argc = 0;
   if (argv) *argv = NULL;

   cookie = xcb_get_property_unchecked(_ecore_xcb_conn, 0, win,
                                       ECORE_X_ATOM_WM_COMMAND,
                                       XCB_GET_PROPERTY_TYPE_ANY,
                                       0, 1000000L);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;

   if ((reply->type != ECORE_X_ATOM_STRING) || (reply->format != 8))
     {
        free(reply);
        return;
     }

   len = reply->value_len;
   if (len < 1)
     {
        free(reply);
        return;
     }

   data = (char *)xcb_get_property_value(reply);
   if (len && (data[len - 1] == '\0'))
     len--;

   c = 1;
   for (cp = (char *)data, i = len; i > 0; cp++, i--)
     if (*cp == '\0') c++;

   v = (char **)malloc((c + 1) * sizeof(char *));
   if (!v)
     {
        free(reply);
        return;
     }

   start = (char *)malloc((len + 1) * sizeof(char));
   if (!start)
     {
        free(reply);
        free(v);
        return;
     }

   memcpy(start, (char *)data, len);
   start[len] = '\0';
   for (cp = start, i = len + 1, j = 0; i > 0; cp++, i--)
     {
        if (*cp == '\0')
          {
             v[j] = start;
             start = (cp + 1);
             j++;
          }
     }

   if (c < 1)
     {
        free(reply);
        free(v);
        return;
     }

   if (argc) *argc = c;

   if (argv)
     {
        (*argv) = malloc(c * sizeof(char *));
        if (!*argv)
          {
             free(reply);
             free(v);
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

   free(reply);
   free(v);
}

EAPI char *
ecore_x_icccm_title_get(Ecore_X_Window win)
{
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_t prop;
#else
   xcb_icccm_get_text_property_reply_t prop;
#endif
   uint8_t ret = 0;
   char *title = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!win) return NULL;
#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_name_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_name_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#else
   cookie = xcb_icccm_get_wm_name_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_name_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#endif
   if (ret == 0) return NULL;
   if (prop.name_len < 1)
     {
#ifdef OLD_XCB_VERSION
        xcb_get_text_property_reply_wipe(&prop);
#else
        xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
        return NULL;
     }

   if (!(title = malloc((prop.name_len + 1) * sizeof(char *))))
     {
#ifdef OLD_XCB_VERSION
        xcb_get_text_property_reply_wipe(&prop);
#else
        xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
        return NULL;
     }
   memcpy(title, prop.name, sizeof(char *) * prop.name_len);
   title[prop.name_len] = '\0';

   if (prop.encoding != ECORE_X_ATOM_UTF8_STRING)
     {
        Ecore_Xcb_Textproperty tp;
        int count = 0;
        char **list = NULL;
        Eina_Bool ret = EINA_FALSE;

        tp.value = strdup(title);
        tp.nitems = prop.name_len;
        tp.encoding = prop.encoding;
#ifdef HAVE_ICONV
        ret = _ecore_xcb_utf8_textproperty_to_textlist(&tp, &list, &count);
#else
        ret = _ecore_xcb_mb_textproperty_to_textlist(&tp, &list, &count);
#endif
        if (ret)
          {
             if (count > 0)
               title = strdup(list[0]);

             if (list) free(list);
          }
     }

#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_wipe(&prop);
#else
   xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
   return title;
}

EAPI void
ecore_x_icccm_title_set(Ecore_X_Window win,
                        const char    *title)
{
   Ecore_Xcb_Textproperty prop;
   char *list[1];
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!title) return;

   prop.value = NULL;
   list[0] = strdup(title);

#ifdef HAVE_ICONV
   ret = _ecore_xcb_utf8_textlist_to_textproperty(list, 1, XcbUTF8StringStyle,
                                                  &prop);
#else
   ret = _ecore_xcb_mb_textlist_to_textproperty(list, 1, XcbStdICCTextStyle,
                                                &prop);
#endif

   if (ret)
     {
#ifdef OLD_XCB_VERSION
        xcb_set_wm_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                        strlen(prop.value), prop.value);
#else
        xcb_icccm_set_wm_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING, 8,
                              strlen(prop.value), prop.value);
#endif
        if (prop.value) free(prop.value);
     }
   else
#ifdef OLD_XCB_VERSION
     xcb_set_wm_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                     strlen(title), title);
#else
     xcb_icccm_set_wm_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING, 8,
                           strlen(title), title);
#endif
   free(list[0]);
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
ecore_x_icccm_name_class_get(Ecore_X_Window win,
                             char         **name,
                             char         **class)
{
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_wm_class_reply_t prop;
#else
   xcb_icccm_get_wm_class_reply_t prop;
#endif
   uint8_t ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (name) *name = NULL;
   if (class) *class = NULL;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_class_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_class_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#else
   cookie = xcb_icccm_get_wm_class_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_class_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#endif
   if (ret == 0) return;

   if (name) *name = strdup(prop.instance_name);
   if (class) *class = strdup(prop.class_name);

#ifdef OLD_XCB_VERSION
   xcb_get_wm_class_reply_wipe(&prop);
#else
   xcb_icccm_get_wm_class_reply_wipe(&prop);
#endif
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
ecore_x_icccm_name_class_set(Ecore_X_Window win,
                             const char    *name,
                             const char    *class)
{
   char *class_string, *s;
   int length_name, length_class;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   length_name = strlen(name);
   length_class = strlen(class);
   class_string =
     (char *)malloc(sizeof(char) * (length_name + length_class + 2));
   if (!class_string) return;

   s = class_string;
   if (length_name)
     {
        strcpy(s, name);
        s += length_name + 1;
     }
   else
     *s++ = '\0';

   if (length_class)
     strcpy(s, class);
   else
     *s = '\0';

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                       ECORE_X_ATOM_WM_CLASS, ECORE_X_ATOM_STRING, 8,
                       length_name + length_class + 2, (void *)class_string);
   free(class_string);
}

/**
 * Specify that a window is transient for another top-level window and should be handled accordingly.
 * @param win the transient window
 * @param forwin the toplevel window
 */
EAPI void
ecore_x_icccm_transient_for_set(Ecore_X_Window win,
                                Ecore_X_Window forwindow)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, win,
                       ECORE_X_ATOM_WM_TRANSIENT_FOR, ECORE_X_ATOM_WINDOW, 32,
                       1, (void *)&forwindow);
}

/**
 * Remove the transient_for setting from a window.
 * @param The window
 */
EAPI void
ecore_x_icccm_transient_for_unset(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_property_del(win, ECORE_X_ATOM_WM_TRANSIENT_FOR);
}

/**
 * Get the window this window is transient for, if any.
 * @param win The window to check
 * @return The window ID of the top-level window, or 0 if the property does not exist.
 */
EAPI Ecore_X_Window
ecore_x_icccm_transient_for_get(Ecore_X_Window win)
{
   Ecore_X_Window forwin = 0;
   xcb_get_property_cookie_t cookie;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_transient_for_unchecked(_ecore_xcb_conn, win);
   xcb_get_wm_transient_for_reply(_ecore_xcb_conn, cookie, &forwin, NULL);
#else
   cookie = xcb_icccm_get_wm_transient_for_unchecked(_ecore_xcb_conn, win);
   xcb_icccm_get_wm_transient_for_reply(_ecore_xcb_conn, cookie, &forwin, NULL);
#endif

   return forwin;
}

/**
 * Get the window role.
 * @param win The window
 * @return The window's role string.
 */
EAPI char *
ecore_x_icccm_window_role_get(Ecore_X_Window win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return ecore_x_window_prop_string_get(win, ECORE_X_ATOM_WM_WINDOW_ROLE);
}

/**
 * Set the window role hint.
 * @param win The window
 * @param role The role string
 */
EAPI void
ecore_x_icccm_window_role_set(Ecore_X_Window win,
                              const char    *role)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_WINDOW_ROLE, role);
}

/**
 * Get the window's client leader.
 * @param win The window
 * @return The window's client leader window, or 0 if unset
 */
EAPI Ecore_X_Window
ecore_x_icccm_client_leader_get(Ecore_X_Window win)
{
   Ecore_X_Window leader;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (ecore_x_window_prop_window_get(win, ECORE_X_ATOM_WM_CLIENT_LEADER,
                                      &leader, 1) > 0)
     return leader;

   return 0;
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
ecore_x_icccm_client_leader_set(Ecore_X_Window win,
                                Ecore_X_Window leader)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_WM_CLIENT_LEADER,
                                  &leader, 1);
}

EAPI Ecore_X_Window_State_Hint
ecore_x_icccm_state_get(Ecore_X_Window win)
{
   xcb_get_property_cookie_t cookie;
   xcb_get_property_reply_t *reply;
   Ecore_X_Window_State_Hint hint = ECORE_X_WINDOW_STATE_HINT_NONE;
   uint8_t *prop;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   cookie =
     xcb_get_property_unchecked(_ecore_xcb_conn, 0, win,
                                ECORE_X_ATOM_WM_STATE, ECORE_X_ATOM_WM_STATE,
                                0L, 0x7fffffff);
   reply = xcb_get_property_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return hint;
   if ((reply->type == 0) || (reply->format != 8) || (reply->value_len != 2))
     {
        free(reply);
        return hint;
     }

   prop = (uint8_t *)xcb_get_property_value(reply);
#ifdef OLD_XCB_VERSION
   switch (prop[0])
     {
      case XCB_WM_STATE_WITHDRAWN:
        hint = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
        break;

      case XCB_WM_STATE_NORMAL:
        hint = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        break;

      case XCB_WM_STATE_ICONIC:
        hint = ECORE_X_WINDOW_STATE_HINT_ICONIC;
        break;

      default:
        break;
     }
#else
   switch (prop[0])
     {
      case XCB_ICCCM_WM_STATE_WITHDRAWN:
        hint = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
        break;

      case XCB_ICCCM_WM_STATE_NORMAL:
        hint = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        break;

      case XCB_ICCCM_WM_STATE_ICONIC:
        hint = ECORE_X_WINDOW_STATE_HINT_ICONIC;
        break;

      default:
        break;
     }
#endif

   free(reply);
   return hint;
}

EAPI void
ecore_x_icccm_state_set(Ecore_X_Window            win,
                        Ecore_X_Window_State_Hint state)
{
#ifdef OLD_XCB_VERSION
   xcb_wm_hints_t hints;
#else
   xcb_icccm_wm_hints_t hints;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef OLD_XCB_VERSION
   xcb_wm_hints_set_none(&hints);

   hints.flags = XCB_WM_HINT_STATE;

   if (state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     xcb_wm_hints_set_withdrawn(&hints);
   else if (state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     xcb_wm_hints_set_normal(&hints);
   else if (state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     xcb_wm_hints_set_iconic(&hints);

   xcb_set_wm_hints(_ecore_xcb_conn, win, &hints);
#else
   xcb_icccm_wm_hints_set_none(&hints);

   hints.flags = XCB_ICCCM_WM_HINT_STATE;

   if (state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     xcb_icccm_wm_hints_set_withdrawn(&hints);
   else if (state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     xcb_icccm_wm_hints_set_normal(&hints);
   else if (state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     xcb_icccm_wm_hints_set_iconic(&hints);

   xcb_icccm_set_wm_hints(_ecore_xcb_conn, win, &hints);
#endif
}

EAPI void
ecore_x_icccm_delete_window_send(Ecore_X_Window win,
                                 Ecore_X_Time   t)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
                                 ECORE_X_EVENT_MASK_NONE,
                                 ECORE_X_ATOM_WM_DELETE_WINDOW, t, 0, 0, 0);
}

EAPI void
ecore_x_icccm_hints_set(Ecore_X_Window            win,
                        Eina_Bool                 accepts_focus,
                        Ecore_X_Window_State_Hint initial_state,
                        Ecore_X_Pixmap            icon_pixmap,
                        Ecore_X_Pixmap            icon_mask,
                        Ecore_X_Window            icon_window,
                        Ecore_X_Window            window_group,
                        Eina_Bool                 is_urgent)
{
#ifdef OLD_XCB_VERSION
   xcb_wm_hints_t hints;
#else
   xcb_icccm_wm_hints_t hints;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef OLD_XCB_VERSION
   xcb_wm_hints_set_none(&hints);
   xcb_wm_hints_set_input(&hints, accepts_focus);

   if (initial_state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     xcb_wm_hints_set_withdrawn(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     xcb_wm_hints_set_normal(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     xcb_wm_hints_set_iconic(&hints);

   if (icon_pixmap != 0) xcb_wm_hints_set_icon_pixmap(&hints, icon_pixmap);
   if (icon_mask != 0) xcb_wm_hints_set_icon_mask(&hints, icon_mask);
   if (icon_window != 0) xcb_wm_hints_set_icon_window(&hints, icon_window);
   if (window_group != 0) xcb_wm_hints_set_window_group(&hints, window_group);
   if (is_urgent) xcb_wm_hints_set_urgency(&hints);

   xcb_set_wm_hints(_ecore_xcb_conn, win, &hints);
#else
   xcb_icccm_wm_hints_set_none(&hints);
   xcb_icccm_wm_hints_set_input(&hints, accepts_focus);

   if (initial_state == ECORE_X_WINDOW_STATE_HINT_WITHDRAWN)
     xcb_icccm_wm_hints_set_withdrawn(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_NORMAL)
     xcb_icccm_wm_hints_set_normal(&hints);
   else if (initial_state == ECORE_X_WINDOW_STATE_HINT_ICONIC)
     xcb_icccm_wm_hints_set_iconic(&hints);

   if (icon_pixmap != 0)
     xcb_icccm_wm_hints_set_icon_pixmap(&hints, icon_pixmap);
   if (icon_mask != 0)
     xcb_icccm_wm_hints_set_icon_mask(&hints, icon_mask);
   if (icon_window != 0)
     xcb_icccm_wm_hints_set_icon_window(&hints, icon_window);
   if (window_group != 0)
     xcb_icccm_wm_hints_set_window_group(&hints, window_group);
   if (is_urgent)
     xcb_icccm_wm_hints_set_urgency(&hints);

   xcb_icccm_set_wm_hints(_ecore_xcb_conn, win, &hints);
#endif
}

EAPI Eina_Bool
ecore_x_icccm_hints_get(Ecore_X_Window             win,
                        Eina_Bool                 *accepts_focus,
                        Ecore_X_Window_State_Hint *initial_state,
                        Ecore_X_Pixmap            *icon_pixmap,
                        Ecore_X_Pixmap            *icon_mask,
                        Ecore_X_Window            *icon_window,
                        Ecore_X_Window            *window_group,
                        Eina_Bool                 *is_urgent)
{
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_wm_hints_t hints;
#else
   xcb_icccm_wm_hints_t hints;
#endif
   uint8_t ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (accepts_focus) *accepts_focus = EINA_TRUE;
   if (initial_state) *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
   if (icon_pixmap) *icon_pixmap = 0;
   if (icon_mask) *icon_mask = 0;
   if (icon_window) *icon_window = 0;
   if (window_group) *window_group = 0;
   if (is_urgent) *is_urgent = EINA_FALSE;

#ifdef OLD_XCB_VERSION
   xcb_wm_hints_set_none(&hints);
   cookie = xcb_get_wm_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_hints_reply(_ecore_xcb_conn, cookie, &hints, NULL);
#else
   xcb_icccm_wm_hints_set_none(&hints);
   cookie = xcb_icccm_get_wm_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_hints_reply(_ecore_xcb_conn, cookie, &hints, NULL);
#endif
   if (!ret) return EINA_FALSE;

#ifdef OLD_XCB_VERSION
   if ((hints.flags & XCB_WM_HINT_INPUT) && (accepts_focus))
#else
   if ((hints.flags & XCB_ICCCM_WM_HINT_INPUT) && (accepts_focus))
#endif
     {
        if (hints.input)
          *accepts_focus = EINA_TRUE;
        else
          *accepts_focus = EINA_FALSE;
     }

#ifdef OLD_XCB_VERSION
   if ((hints.flags & XCB_WM_HINT_STATE) && (initial_state))
     {
        if (hints.initial_state == XCB_WM_STATE_WITHDRAWN)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
        else if (hints.initial_state == XCB_WM_STATE_NORMAL)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        else if (hints.initial_state == XCB_WM_STATE_ICONIC)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }

   if ((hints.flags & XCB_WM_HINT_ICON_PIXMAP) && (icon_pixmap))
     *icon_pixmap = hints.icon_pixmap;

   if ((hints.flags & XCB_WM_HINT_ICON_MASK) && (icon_mask))
     *icon_mask = hints.icon_mask;

   if ((hints.flags & XCB_WM_HINT_ICON_WINDOW) && (icon_window))
     *icon_window = hints.icon_window;

   if ((hints.flags & XCB_WM_HINT_WINDOW_GROUP) && (window_group))
     *window_group = hints.window_group;

   if ((hints.flags & XCB_WM_HINT_X_URGENCY) && (is_urgent))
     *is_urgent = EINA_TRUE;
#else
   if ((hints.flags & XCB_ICCCM_WM_HINT_STATE) && (initial_state))
     {
        if (hints.initial_state == XCB_ICCCM_WM_STATE_WITHDRAWN)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_WITHDRAWN;
        else if (hints.initial_state == XCB_ICCCM_WM_STATE_NORMAL)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_NORMAL;
        else if (hints.initial_state == XCB_ICCCM_WM_STATE_ICONIC)
          *initial_state = ECORE_X_WINDOW_STATE_HINT_ICONIC;
     }

   if ((hints.flags & XCB_ICCCM_WM_HINT_ICON_PIXMAP) && (icon_pixmap))
     *icon_pixmap = hints.icon_pixmap;

   if ((hints.flags & XCB_ICCCM_WM_HINT_ICON_MASK) && (icon_mask))
     *icon_mask = hints.icon_mask;

   if ((hints.flags & XCB_ICCCM_WM_HINT_ICON_WINDOW) && (icon_window))
     *icon_window = hints.icon_window;

   if ((hints.flags & XCB_ICCCM_WM_HINT_WINDOW_GROUP) && (window_group))
     *window_group = hints.window_group;

   if ((hints.flags & XCB_ICCCM_WM_HINT_X_URGENCY) && (is_urgent))
     *is_urgent = EINA_TRUE;
#endif

   return EINA_TRUE;
}

/**
 * Get a window icon name.
 * @param win The window
 * @return The windows icon name string
 *
 * Return the icon name of a window. String must be free'd when done with.
 */
EAPI char *
ecore_x_icccm_icon_name_get(Ecore_X_Window win)
{
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_t prop;
#else
   xcb_icccm_get_text_property_reply_t prop;
#endif
   uint8_t ret = 0;
   char *tmp = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!win) return NULL;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_icon_name_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_icon_name_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#else
   cookie = xcb_icccm_get_wm_icon_name_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_icon_name_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#endif
   if (ret == 0) return NULL;

   if (prop.name_len < 1)
     {
#ifdef OLD_XCB_VERSION
        xcb_get_text_property_reply_wipe(&prop);
#else
        xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
        return NULL;
     }

   if (!(tmp = malloc((prop.name_len + 1) * sizeof(char *))))
     {
#ifdef OLD_XCB_VERSION
        xcb_get_text_property_reply_wipe(&prop);
#else
        xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
        return NULL;
     }
   memcpy(tmp, prop.name, sizeof(char *) * prop.name_len);
   tmp[prop.name_len] = '\0';

   if (prop.encoding != ECORE_X_ATOM_UTF8_STRING)
     {
        Ecore_Xcb_Textproperty tp;
        int count = 0;
        char **list = NULL;
        Eina_Bool ret = EINA_FALSE;

        tp.value = strdup(tmp);
        tp.nitems = prop.name_len;
        tp.encoding = prop.encoding;
#ifdef HAVE_ICONV
        ret = _ecore_xcb_utf8_textproperty_to_textlist(&tp, &list, &count);
#else
        ret = _ecore_xcb_mb_textproperty_to_textlist(&tp, &list, &count);
#endif
        if (ret)
          {
             if (count > 0)
               tmp = strdup(list[0]);

             if (list) free(list);
          }
     }

#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_wipe(&prop);
#else
   xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
   return tmp;
}

/**
 * Set a window icon name.
 * @param win The window
 * @param t The icon name string
 *
 * Set a window icon name
 */
EAPI void
ecore_x_icccm_icon_name_set(Ecore_X_Window win,
                            const char    *name)
{
   Ecore_Xcb_Textproperty prop;
   char *list[1];
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if ((!win) || (!name)) return;

   prop.value = NULL;
   list[0] = strdup(name);

#ifdef HAVE_ICONV
   ret = _ecore_xcb_utf8_textlist_to_textproperty(list, 1, XcbUTF8StringStyle,
                                                  &prop);
#else
   ret = _ecore_xcb_mb_textlist_to_textproperty(list, 1, XcbStdICCTextStyle,
                                                &prop);
#endif

   if (ret)
     {
#ifdef OLD_XCB_VERSION
        xcb_set_wm_icon_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                             strlen(prop.value), prop.value);
#else
        xcb_icccm_set_wm_icon_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                                   8, strlen(prop.value), prop.value);
#endif
        if (prop.value) free(prop.value);
     }
   else
#ifdef OLD_XCB_VERSION
     xcb_set_wm_icon_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                          strlen(name), name);
#else
     xcb_icccm_set_wm_icon_name(_ecore_xcb_conn, win, ECORE_X_ATOM_STRING,
                                8, strlen(name), name);
#endif

   free(list[0]);
}

EAPI void
ecore_x_icccm_iconic_request_send(Ecore_X_Window win,
                                  Ecore_X_Window root)
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!win) return;
   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_WM_CHANGE_STATE;
#ifdef OLD_XCB_VERSION
   ev.data.data32[0] = XCB_WM_STATE_ICONIC;
#else
   ev.data.data32[0] = XCB_ICCCM_WM_STATE_ICONIC;
#endif

   xcb_send_event(_ecore_xcb_conn, 0, root,
                  (XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT),
                  (const char *)&ev);
//   ecore_x_flush();
}

/**
 * Set or unset a wm protocol property.
 * @param win The Window
 * @param protocol The protocol to enable/disable
 * @param on On/Off
 */
EAPI void
ecore_x_icccm_protocol_set(Ecore_X_Window      win,
                           Ecore_X_WM_Protocol protocol,
                           Eina_Bool           on)
{
   Ecore_X_Atom proto;
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_t protos;
#else
   xcb_icccm_get_wm_protocols_reply_t protos;
#endif
   int i = 0, count = 0, set = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (protocol >= ECORE_X_WM_PROTOCOL_NUM) return;
   proto = _ecore_xcb_atoms_wm_protocol[protocol];
#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_protocols_unchecked(_ecore_xcb_conn, win, proto);
   if (!xcb_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &protos, NULL))
#else
   cookie = xcb_icccm_get_wm_protocols_unchecked(_ecore_xcb_conn, win, proto);
   if (!xcb_icccm_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &protos, NULL))
#endif
     count = 0;
   else
     count = protos.atoms_len;

   for (i = 0; i < count; i++)
     {
        if (protos.atoms[i] == proto)
          {
             set = 1;
             break;
          }
     }

   if (on)
     {
        if (!set)
          {
             Ecore_X_Atom *atoms = NULL;

             atoms = malloc((count + 1) * sizeof(Ecore_X_Atom));
             if (atoms)
               {
                  for (i = 0; i < count; i++)
                    atoms[i] = protos.atoms[i];
                  atoms[count] = proto;
#ifdef OLD_XCB_VERSION
                  xcb_set_wm_protocols(_ecore_xcb_conn,
                                       ECORE_X_ATOM_WM_PROTOCOLS,
                                       win, count, atoms);
#else
                  xcb_icccm_set_wm_protocols(_ecore_xcb_conn, win, 
                                             ECORE_X_ATOM_WM_PROTOCOLS,
                                             count, atoms);
#endif
                  free(atoms);
               }
          }
     }
   else
     {
        if (set)
          {
             for (i = 0; i < count; i++)
               {
                  if (protos.atoms[i] == proto)
                    {
                       int j = 0;

                       for (j = (i + 1); j < count; j++)
                         protos.atoms[j - 1] = protos.atoms[j];
                       if (count > 1)
#ifdef OLD_XCB_VERSION
                         xcb_set_wm_protocols(_ecore_xcb_conn,
                                              ECORE_X_ATOM_WM_PROTOCOLS,
                                              win, count - 1, protos.atoms);
#else
                         xcb_icccm_set_wm_protocols(_ecore_xcb_conn, win, 
                                                    ECORE_X_ATOM_WM_PROTOCOLS,
                                                    count - 1, protos.atoms);
#endif
                       else
                         ecore_x_window_prop_property_del(win,
                                                          ECORE_X_ATOM_WM_PROTOCOLS);
                       break;
                    }
               }
          }
     }

#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_wipe(&protos);
#else
   xcb_icccm_get_wm_protocols_reply_wipe(&protos);
#endif
}

/**
 * Determines whether a protocol is set for a window.
 * @param win The Window
 * @param protocol The protocol to query
 * @return 1 if the protocol is set, else 0.
 */
EAPI Eina_Bool
ecore_x_icccm_protocol_isset(Ecore_X_Window      win,
                             Ecore_X_WM_Protocol protocol)
{
   Ecore_X_Atom proto;
   Eina_Bool ret = EINA_FALSE;
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_t reply;
#else
   xcb_icccm_get_wm_protocols_reply_t reply;
#endif
   uint8_t val = 0;
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (protocol >= ECORE_X_WM_PROTOCOL_NUM) return EINA_FALSE;

   proto = _ecore_xcb_atoms_wm_protocol[protocol];
#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_protocols_unchecked(_ecore_xcb_conn, win, proto);
   val = xcb_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &reply, NULL);
#else
   cookie = xcb_icccm_get_wm_protocols_unchecked(_ecore_xcb_conn, win, proto);
   val = xcb_icccm_get_wm_protocols_reply(_ecore_xcb_conn, cookie, &reply, NULL);
#endif
   if (!val) return EINA_FALSE;

   for (i = 0; i < reply.atoms_len; i++)
     if (reply.atoms[i] == proto)
       {
          ret = EINA_TRUE;
          break;
       }

#ifdef OLD_XCB_VERSION
   xcb_get_wm_protocols_reply_wipe(&reply);
#else
   xcb_icccm_get_wm_protocols_reply_wipe(&reply);
#endif

   return ret;
}

/**
 * Set protocol atoms explicitly
 * @param win The Window
 * @param protos An array of protocol atoms
 * @param num the number of members of the array
 */
EAPI void
ecore_x_icccm_protocol_atoms_set(Ecore_X_Window win,
                                 Ecore_X_Atom  *protos,
                                 int            num)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num > 0)
#ifdef OLD_XCB_VERSION
     xcb_set_wm_protocols(_ecore_xcb_conn, ECORE_X_ATOM_WM_PROTOCOLS,
                          win, num, protos);
#else
     xcb_icccm_set_wm_protocols(_ecore_xcb_conn, win, 
                                ECORE_X_ATOM_WM_PROTOCOLS, num, protos);
#endif
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_WM_PROTOCOLS);
}

EAPI Eina_Bool
ecore_x_icccm_size_pos_hints_get(Ecore_X_Window   win,
                                 Eina_Bool       *request_pos,
                                 Ecore_X_Gravity *gravity,
                                 int             *min_w,
                                 int             *min_h,
                                 int             *max_w,
                                 int             *max_h,
                                 int             *base_w,
                                 int             *base_h,
                                 int             *step_x,
                                 int             *step_y,
                                 double          *min_aspect,
                                 double          *max_aspect)
{
   xcb_size_hints_t hints;
   xcb_get_property_cookie_t cookie;
   uint8_t ret = 0;
   int32_t minw = 0, minh = 0;
   int32_t maxw = 32767, maxh = 32767;
   int32_t basew = -1, baseh = -1;
   int32_t stepx = -1, stepy = -1;
   double mina = 0.0, maxa = 0.0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (request_pos) *request_pos = EINA_FALSE;
   if (gravity) *gravity = ECORE_X_GRAVITY_NW;
   if (min_w) *min_w = minw;
   if (min_h) *min_h = minh;
   if (max_w) *max_w = maxw;
   if (max_h) *max_h = maxh;
   if (base_w) *base_w = basew;
   if (base_h) *base_h = baseh;
   if (step_x) *step_x = stepx;
   if (step_y) *step_y = stepy;
   if (min_aspect) *min_aspect = mina;
   if (max_aspect) *max_aspect = maxa;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_normal_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_normal_hints_reply(_ecore_xcb_conn, cookie, &hints, NULL);
#else
   cookie = xcb_icccm_get_wm_normal_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_normal_hints_reply(_ecore_xcb_conn, cookie,
                                             &hints, NULL);
#endif
   if (!ret) return EINA_FALSE;

#ifdef OLD_XCB_VERSION
   if ((hints.flags & XCB_SIZE_HINT_US_POSITION) ||
       (hints.flags & XCB_SIZE_HINT_P_POSITION))
#else
   if ((hints.flags & XCB_ICCCM_SIZE_HINT_US_POSITION) ||
       (hints.flags & XCB_ICCCM_SIZE_HINT_P_POSITION))
#endif
     {
        if (request_pos) *request_pos = EINA_TRUE;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_P_WIN_GRAVITY)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_P_WIN_GRAVITY)
#endif
     {
        if (gravity) *gravity = hints.win_gravity;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_P_MIN_SIZE)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MIN_SIZE)
#endif
     {
        minw = hints.min_width;
        minh = hints.min_height;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_P_MAX_SIZE)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_P_MAX_SIZE)
#endif
     {
        maxw = hints.max_width;
        maxh = hints.max_height;
        if (maxw < minw) maxw = minw;
        if (maxh < minh) maxh = minh;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_BASE_SIZE)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_BASE_SIZE)
#endif
     {
        basew = hints.base_width;
        baseh = hints.base_height;
        if (basew > minw) minw = basew;
        if (baseh > minh) minh = baseh;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_P_RESIZE_INC)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_P_RESIZE_INC)
#endif
     {
        stepx = hints.width_inc;
        stepy = hints.height_inc;
        if (stepx < 1) stepx = 1;
        if (stepy < 1) stepy = 1;
     }

#ifdef OLD_XCB_VERSION
   if (hints.flags & XCB_SIZE_HINT_P_ASPECT)
#else
   if (hints.flags & XCB_ICCCM_SIZE_HINT_P_ASPECT)
#endif
     {
        if (hints.min_aspect_den > 0)
          mina = ((double)hints.min_aspect_num) / ((double)hints.min_aspect_den);

        if (hints.max_aspect_den > 0)
          maxa = ((double)hints.max_aspect_num) / ((double)hints.max_aspect_den);
     }

   if (min_w) *min_w = minw;
   if (min_h) *min_h = minh;
   if (max_w) *max_w = maxw;
   if (max_h) *max_h = maxh;
   if (base_w) *base_w = basew;
   if (base_h) *base_h = baseh;
   if (step_x) *step_x = stepx;
   if (step_y) *step_y = stepy;
   if (min_aspect) *min_aspect = mina;
   if (max_aspect) *max_aspect = maxa;

   return EINA_TRUE;
}

EAPI void
ecore_x_icccm_size_pos_hints_set(Ecore_X_Window  win,
                                 Eina_Bool       request_pos,
                                 Ecore_X_Gravity gravity,
                                 int             min_w,
                                 int             min_h,
                                 int             max_w,
                                 int             max_h,
                                 int             base_w,
                                 int             base_h,
                                 int             step_x,
                                 int             step_y,
                                 double          min_aspect,
                                 double          max_aspect)
{
   xcb_get_property_cookie_t cookie;
   xcb_size_hints_t hints;
   uint8_t ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_normal_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_normal_hints_reply(_ecore_xcb_conn, cookie, &hints, NULL);
#else
   cookie = xcb_icccm_get_wm_normal_hints_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_normal_hints_reply(_ecore_xcb_conn, cookie,
                                             &hints, NULL);
#endif
   if (!ret) memset(&hints, 0, sizeof(xcb_size_hints_t));

   hints.flags = 0;

#ifdef OLD_XCB_VERSION
   if (request_pos)
     hints.flags |= XCB_SIZE_HINT_US_POSITION;

   if (gravity != ECORE_X_GRAVITY_NW)
     xcb_size_hints_set_win_gravity(&hints, gravity);
   if ((min_w > 0) || (min_h > 0))
     xcb_size_hints_set_min_size(&hints, min_w, min_h);
   if ((max_w > 0) || (max_h > 0))
     xcb_size_hints_set_max_size(&hints, max_w, max_h);
   if ((base_w > 0) || (base_h > 0))
     xcb_size_hints_set_base_size(&hints, base_w, base_h);
   if ((step_x > 1) || (step_y > 1))
     xcb_size_hints_set_resize_inc(&hints, step_x, step_y);
   if ((min_aspect > 0.0) || (max_aspect > 0.0))
     xcb_size_hints_set_aspect(&hints,
                               (int32_t)(min_aspect * 10000), 10000,
                               (int32_t)(max_aspect * 10000), 10000);

   xcb_set_wm_normal_hints(_ecore_xcb_conn, win, &hints);
#else
   if (request_pos)
     hints.flags |= XCB_ICCCM_SIZE_HINT_US_POSITION;

   if (gravity != ECORE_X_GRAVITY_NW)
     xcb_icccm_size_hints_set_win_gravity(&hints, gravity);
   if ((min_w > 0) || (min_h > 0))
     xcb_icccm_size_hints_set_min_size(&hints, min_w, min_h);
   if ((max_w > 0) || (max_h > 0))
     xcb_icccm_size_hints_set_max_size(&hints, max_w, max_h);
   if ((base_w > 0) || (base_h > 0))
     xcb_icccm_size_hints_set_base_size(&hints, base_w, base_h);
   if ((step_x > 1) || (step_y > 1))
     xcb_icccm_size_hints_set_resize_inc(&hints, step_x, step_y);
   if ((min_aspect > 0.0) || (max_aspect > 0.0))
     xcb_icccm_size_hints_set_aspect(&hints,
                                     (int32_t)(min_aspect * 10000), 10000,
                                     (int32_t)(max_aspect * 10000), 10000);

   xcb_icccm_set_wm_normal_hints(_ecore_xcb_conn, win, &hints);
#endif
}

EAPI void
ecore_x_icccm_move_resize_send(Ecore_X_Window win,
                               int            x,
                               int            y,
                               int            w,
                               int            h)
{
   xcb_configure_notify_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!win) return;

   memset(&ev, 0, sizeof(xcb_configure_notify_event_t));

   ev.response_type = XCB_CONFIGURE_NOTIFY;
   ev.event = win;
   ev.window = win;
   ev.above_sibling = XCB_NONE;
   ev.x = x;
   ev.y = y;
   ev.width = w;
   ev.height = h;
   ev.border_width = 0;
   ev.override_redirect = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win,
                  XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char *)&ev);
//   ecore_x_flush();
}

/**
 * Get a window client machine string.
 * @param win The window
 * @return The windows client machine string
 *
 * Return the client machine of a window. String must be free'd when done with.
 */
EAPI char *
ecore_x_icccm_client_machine_get(Ecore_X_Window win)
{
   xcb_get_property_cookie_t cookie;
#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_t prop;
#else
   xcb_icccm_get_text_property_reply_t prop;
#endif
   uint8_t ret = 0;
   char *tmp = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef OLD_XCB_VERSION
   cookie = xcb_get_wm_client_machine_unchecked(_ecore_xcb_conn, win);
   ret = xcb_get_wm_client_machine_reply(_ecore_xcb_conn, cookie, &prop, NULL);
#else
   cookie = xcb_icccm_get_wm_client_machine_unchecked(_ecore_xcb_conn, win);
   ret = xcb_icccm_get_wm_client_machine_reply(_ecore_xcb_conn, cookie,
                                               &prop, NULL);
#endif
   if (ret == 0) return NULL;

   tmp = malloc((prop.name_len + 1) * sizeof(char *));
   if (!tmp)
     {
#ifdef OLD_XCB_VERSION
        xcb_get_text_property_reply_wipe(&prop);
#else
        xcb_icccm_get_text_property_reply_wipe(&prop);
#endif
        return NULL;
     }
   memcpy(tmp, prop.name, sizeof(char *) * prop.name_len);
   tmp[prop.name_len] = '\0';

#ifdef OLD_XCB_VERSION
   xcb_get_text_property_reply_wipe(&prop);
#else
   xcb_icccm_get_text_property_reply_wipe(&prop);
#endif

   return tmp;
}

EAPI void
ecore_x_icccm_take_focus_send(Ecore_X_Window win,
                              Ecore_X_Time   t)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
                                 XCB_EVENT_MASK_NO_EVENT,
                                 ECORE_X_ATOM_WM_TAKE_FOCUS, t, 0, 0, 0);
}

EAPI void
ecore_x_icccm_save_yourself_send(Ecore_X_Window win,
                                 Ecore_X_Time   t)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, ECORE_X_ATOM_WM_PROTOCOLS,
                                 XCB_EVENT_MASK_NO_EVENT,
                                 ECORE_X_ATOM_WM_SAVE_YOURSELF, t, 0, 0, 0);
}

/**
 * Add a subwindow to the list of windows that need a different colormap installed.
 * @param win The toplevel window
 * @param subwin The subwindow to be added to the colormap windows list
 */
EAPI void
ecore_x_icccm_colormap_window_set(Ecore_X_Window win,
                                  Ecore_X_Window subwin)
{
   int num = 0, i = 0;
   unsigned char *odata = NULL, *data = NULL;
   Ecore_X_Window *newset = NULL, *oldset = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                         ECORE_X_ATOM_WINDOW, 32, &odata, &num))
     {
        if (!(newset = calloc(1, sizeof(Ecore_X_Window)))) return;
        newset[0] = subwin;
        num = 1;
        data = (unsigned char *)newset;
     }
   else
     {
        if (!(newset = calloc(num + 1, sizeof(Ecore_X_Window)))) return;
        oldset = (Ecore_X_Window *)odata;
        for (i = 0; i < num; i++)
          {
             if (oldset[i] == subwin)
               {
                  if (odata) free(odata);
                  odata = NULL;
                  free(newset);
                  return;
               }
             newset[i] = oldset[i];
          }
        newset[num++] = subwin;
        if (odata) free(odata);
        data = (unsigned char *)newset;
     }
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                    ECORE_X_ATOM_WINDOW, 32, data, num);
   free(newset);
}

/**
 * Remove a window from the list of colormap windows.
 * @param win The toplevel window
 * @param subwin The window to be removed from the colormap window list.
 */
EAPI void
ecore_x_icccm_colormap_window_unset(Ecore_X_Window win,
                                    Ecore_X_Window subwin)
{
   int num = 0, i = 0, j = 0, k = 0;
   unsigned char *odata = NULL, *data = NULL;
   Ecore_X_Window *newset = NULL, *oldset = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                         ECORE_X_ATOM_WINDOW, 32, &odata, &num))
     return;

   oldset = (Ecore_X_Window *)odata;
   for (i = 0; i < num; i++)
     {
        if (oldset[i] == subwin)
          {
             if (num == 1)
               {
                  ecore_x_window_prop_property_del(win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS);
                  if (odata) free(odata);
                  odata = NULL;
                  return;
               }
             else
               {
                  newset = calloc(num - 1, sizeof(Ecore_X_Window));
                  data = (unsigned char *)newset;
                  for (j = 0; j < num; ++j)
                    if (oldset[j] != subwin)
                      newset[k++] = oldset[j];

                  ecore_x_window_prop_property_set(win, ECORE_X_ATOM_WM_COLORMAP_WINDOWS,
                                                   ECORE_X_ATOM_WINDOW, 32, data, k);
                  if (odata) free(odata);
                  odata = NULL;
                  free(newset);
                  return;
               }
          }
     }
   if (odata) free(odata);
}

