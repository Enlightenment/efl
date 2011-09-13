#include "ecore_xcb_private.h"

/* local function prototypes */
static void _ecore_xcb_netwm_startup_info_free(void *data);
static Ecore_X_Atom _ecore_xcb_netwm_window_type_atom_get(Ecore_X_Window_Type type);
static Ecore_X_Window_Type _ecore_xcb_netwm_window_type_type_get(Ecore_X_Atom atom);
static Ecore_X_Atom _ecore_xcb_netwm_window_state_atom_get(Ecore_X_Window_State state);
static Ecore_X_Atom _ecore_xcb_netwm_action_atom_get(Ecore_X_Action action);

/* local variables */
static Eina_Hash *_startup_info = NULL;

/* local structures */
typedef struct _Ecore_Xcb_Startup_Info Ecore_Xcb_Startup_Info;
struct _Ecore_Xcb_Startup_Info 
{
   Ecore_X_Window win;
   int init, size;
   char *buffer;
   int length;

   /* sequence info fields */
   char *id, *name;
   int screen;
   char *bin, *icon;
   int desktop, timestamp;
   char *description, *wmclass;
   int silent;
};

EAPI void 
ecore_x_netwm_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _startup_info = 
     eina_hash_string_superfast_new(_ecore_xcb_netwm_startup_info_free);
}

EAPI void 
ecore_x_netwm_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_startup_info) eina_hash_free(_startup_info);
   _startup_info = NULL;
}

EAPI Eina_Bool 
ecore_x_netwm_pid_get(Ecore_X_Window win, int *pid) 
{
   uint32_t tmp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_PID, &tmp, 1))
     return EINA_FALSE;

   if (pid) *pid = tmp;

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_pid_set(Ecore_X_Window win, int pid) 
{
   unsigned int tmp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   tmp = pid;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_PID, &tmp, 1);
}

EAPI Eina_Bool 
ecore_x_netwm_window_type_get(Ecore_X_Window win, Ecore_X_Window_Type *type) 
{
   Ecore_X_Atom *atoms;
   int num = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (type) *type = ECORE_X_WINDOW_TYPE_NORMAL;

   num = 
     ecore_x_window_prop_atom_list_get(win, 
                                       ECORE_X_ATOM_NET_WM_WINDOW_TYPE, &atoms);
   if ((type) && (num >= 1) && (atoms))
     *type = _ecore_xcb_netwm_window_type_type_get(atoms[0]);

   if (atoms) free(atoms);

   if (num >= 1) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI void 
ecore_x_netwm_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type) 
{
   Ecore_X_Atom atom;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   atom = _ecore_xcb_netwm_window_type_atom_get(type);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE, &atom, 1);
}

EAPI int 
ecore_x_netwm_window_types_get(Ecore_X_Window win, Ecore_X_Window_Type **types) 
{
   int num = 0, i = 0;
   Ecore_X_Atom *atoms = NULL;
   Ecore_X_Window_Type *atoms2 = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (types) *types = NULL;
   num = 
     ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE, 
                                       &atoms);
   if ((num <= 0) || (!atoms)) 
     {
        if (atoms) free(atoms);
        return 0;
     }

   atoms2 = malloc(num * sizeof(Ecore_X_Window_Type));
   if (!atoms2) 
     {
        if (atoms) free(atoms);
        return 0;
     }

   for (i = 0; i < num; i++)
     atoms2[i] = _ecore_xcb_netwm_window_type_type_get(atoms[i]);
   if (atoms) free(atoms);

   if (types) 
     *types = atoms2;
   else
     free(atoms2);

   return num;
}

EAPI int 
ecore_x_netwm_name_get(Ecore_X_Window win, char **name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (name) 
     *name = ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_NAME);
   return 1;
}

EAPI void 
ecore_x_netwm_name_set(Ecore_X_Window win, const char *name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_NAME, name);
}

EAPI void 
ecore_x_netwm_opacity_set(Ecore_X_Window win, unsigned int opacity) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY, 
                                  &opacity, 1);
}

EAPI Eina_Bool 
ecore_x_netwm_opacity_get(Ecore_X_Window win, unsigned int *opacity) 
{
   unsigned int tmp = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY, 
                                       &tmp, 1))
     return EINA_FALSE;

   if (opacity) *opacity = tmp;

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check, const char *wm_name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, 
                                  &check, 1);
   ecore_x_window_prop_window_set(check, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, 
                                  &check, 1);
   ecore_x_window_prop_string_set(check, ECORE_X_ATOM_NET_WM_NAME, wm_name);
   ecore_x_window_prop_string_set(root, ECORE_X_ATOM_NET_WM_NAME, wm_name);
}

EAPI void 
ecore_x_netwm_supported_set(Ecore_X_Window root, Ecore_X_Atom *supported, int num) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_atom_set(root, ECORE_X_ATOM_NET_SUPPORTED, 
                                supported, num);
}

EAPI Eina_Bool 
ecore_x_netwm_supported_get(Ecore_X_Window root, Ecore_X_Atom **supported, int *num) 
{
   int num_ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = 0;
   if (supported) *supported = NULL;

   num_ret = 
     ecore_x_window_prop_atom_list_get(root, ECORE_X_ATOM_NET_SUPPORTED, 
                                       supported);
   if (num_ret <= 0) return EINA_FALSE;
   if (num) *num = num_ret;

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_desk_count_set(Ecore_X_Window root, unsigned int n_desks) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS, 
                                  &n_desks, 1);
}

EAPI void 
ecore_x_netwm_desk_roots_set(Ecore_X_Window root, Ecore_X_Window *vroots, unsigned int n_desks) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_VIRTUAL_ROOTS, 
                                  vroots, n_desks);
}

EAPI void 
ecore_x_netwm_desk_names_set(Ecore_X_Window root, const char **names, unsigned int n_desks) 
{
   char ss[32], *buf = NULL, *t = NULL;
   const char *s;
   uint32_t len = 0, i, l;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   for (i = 0; i < n_desks; i++)
     {
        s = ((names) ? names[i] : NULL);
        if (!s)
          {
             /* Default to "Desk-<number>" */
             sprintf(ss, "Desk-%d", i);
             s = ss;
          }

        l = strlen(s) + 1;
        t = realloc(buf, len + 1);
        if (t) 
          {
             buf = t;
             memcpy(buf + len, s, l);
          }
        len += l;
     }

   xcb_change_property(_ecore_xcb_conn, XCB_PROP_MODE_REPLACE, root,
                       ECORE_X_ATOM_NET_DESKTOP_NAMES, 
                       ECORE_X_ATOM_UTF8_STRING, 8, len, (const void *)buf);
   ecore_x_flush();
   free(buf);
}

EAPI void 
ecore_x_netwm_desk_size_set(Ecore_X_Window root, unsigned int width, unsigned int height) 
{
   uint32_t size[2];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   size[0] = width;
   size[1] = height;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_GEOMETRY, 
                                  size, 2);
}

EAPI void 
ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, unsigned int *origins, unsigned int n_desks) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_VIEWPORT, 
                                  origins, (2 * n_desks));
}

EAPI void 
ecore_x_netwm_desk_layout_set(Ecore_X_Window root, int orientation, int columns, int rows, int starting_corner) 
{
   unsigned int layout[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   layout[0] = orientation;
   layout[1] = columns;
   layout[2] = rows;
   layout[3] = starting_corner;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_LAYOUT, 
                                  layout, 4);
}

EAPI void 
ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, unsigned int *areas, unsigned int n_desks) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_WORKAREA, areas, 
                                  4 * n_desks);
}

EAPI void 
ecore_x_netwm_desk_current_set(Ecore_X_Window root, unsigned int desk) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_CURRENT_DESKTOP, 
                                  &desk, 1);
}

EAPI void 
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, Eina_Bool on) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   val = ((on) ? 1 : 0);
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_SHOWING_DESKTOP, 
                                  &val, 1);
}

EAPI int 
ecore_x_netwm_startup_id_get(Ecore_X_Window win, char **id) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (id) 
     {
        *id = 
          ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_STARTUP_ID);
     }

   return 1;
}

EAPI void 
ecore_x_netwm_startup_id_set(Ecore_X_Window win, const char *id) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_STARTUP_ID, id);
}

EAPI void 
ecore_x_netwm_state_request_send(Ecore_X_Window win, Ecore_X_Window root, Ecore_X_Window_State s1, Ecore_X_Window_State s2, Eina_Bool set) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;
   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_NET_WM_STATE;
   ev.data.data32[0] = !!set;
   ev.data.data32[1] = _ecore_xcb_netwm_window_state_atom_get(s1);
   ev.data.data32[2] = _ecore_xcb_netwm_window_state_atom_get(s2);
   /* 1 == normal client, if used in a pager this should be 2 */
   ev.data.data32[3] = 1;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root, 
                  (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), (const char *)&ev);
   ecore_x_flush();
}

EAPI void 
ecore_x_netwm_window_state_set(Ecore_X_Window win, Ecore_X_Window_State *state, unsigned int num) 
{
   Ecore_X_Atom *set;
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!num) 
     {
        ecore_x_window_prop_property_del(win, ECORE_X_ATOM_NET_WM_STATE);
        return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++) 
     set[i] = _ecore_xcb_netwm_window_state_atom_get(state[i]);

   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_STATE, set, num);
   free(set);
}

EAPI Eina_Bool 
ecore_x_netwm_window_state_get(Ecore_X_Window win, Ecore_X_Window_State **state, unsigned int *num) 
{
   Ecore_X_Atom *atoms;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = 0;
   if (state) *state = NULL;

   ret = 
     ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_STATE, &atoms);

   if (ret <= 0) return EINA_FALSE;

   if (state) 
     {
        *state = malloc(ret * sizeof(Ecore_X_Window_State));
        if (*state) 
          {
             int i = 0;

             for (i = 0; i < ret; i++)
               (*state)[i] = _ecore_xcb_netwm_window_state_get(atoms[i]);
             if (num) *num = ret;
          }
     }

   free(atoms);

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(root, 
                                  ECORE_X_ATOM_NET_ACTIVE_WINDOW, &win, 1);
}

EAPI void 
ecore_x_netwm_client_active_request(Ecore_X_Window root, Ecore_X_Window win, int type, Ecore_X_Window current_win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_NET_ACTIVE_WINDOW;
   ev.data.data32[0] = type;
   ev.data.data32[1] = XCB_CURRENT_TIME;
   ev.data.data32[2] = current_win;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root, 
                  (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), (const char *)&ev);
   ecore_x_flush();
}

EAPI void 
ecore_x_netwm_client_list_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_CLIENT_LIST, 
                                  p_clients, n_clients);
}

EAPI void 
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, Ecore_X_Window *p_clients, unsigned int n_clients) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(root, ECORE_X_ATOM_NET_CLIENT_LIST_STACKING, 
                                  p_clients, n_clients);
}

EAPI Eina_Bool 
ecore_x_screen_is_composited(int screen) 
{
   char buff[32];
   xcb_get_selection_owner_cookie_t ocookie;
   xcb_get_selection_owner_reply_t *oreply;
   Ecore_X_Window win;
   static Ecore_X_Atom atom = XCB_NONE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   snprintf(buff, sizeof(buff), "_NET_WM_CM_S%i", screen);

   if (atom == XCB_NONE) 
     {
        xcb_intern_atom_cookie_t acookie;
        xcb_intern_atom_reply_t *areply;

        acookie = 
          xcb_intern_atom_unchecked(_ecore_xcb_conn, 0, strlen(buff), buff);
        areply = xcb_intern_atom_reply(_ecore_xcb_conn, acookie, NULL);
        if (!areply) return EINA_FALSE;
        atom = areply->atom;
        free(areply);
     }
   if (atom == XCB_NONE) return EINA_FALSE;

   ocookie = xcb_get_selection_owner_unchecked(_ecore_xcb_conn, atom);
   oreply = xcb_get_selection_owner_reply(_ecore_xcb_conn, ocookie, NULL);
   if (!oreply) return EINA_FALSE;
   win = oreply->owner;
   free(oreply);

   return (win != XCB_NONE) ? EINA_TRUE : EINA_FALSE;
}

EAPI void 
ecore_x_screen_is_composited_set(int screen, Ecore_X_Window win) 
{
   static Ecore_X_Atom atom = XCB_NONE;
   char buff[32];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   snprintf(buff, sizeof(buff), "_NET_WM_CM_S%i", screen);
   if (atom == XCB_NONE) 
     {
        xcb_intern_atom_cookie_t acookie;
        xcb_intern_atom_reply_t *areply;

        acookie = 
          xcb_intern_atom_unchecked(_ecore_xcb_conn, 0, strlen(buff), buff);
        areply = xcb_intern_atom_reply(_ecore_xcb_conn, acookie, NULL);
        if (!areply) return;
        atom = areply->atom;
        free(areply);
     }
   if (atom == XCB_NONE) return;
   xcb_set_selection_owner(_ecore_xcb_conn, win, atom, 
                           _ecore_xcb_events_last_time_get());
}

EAPI void 
ecore_x_netwm_ping_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_WM_PROTOCOLS;
   ev.data.data32[0] = ECORE_X_ATOM_NET_WM_PING;
   ev.data.data32[1] = ecore_x_current_time_get();
   ev.data.data32[2] = win;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
   ecore_x_flush();
}

EAPI void 
ecore_x_netwm_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb) 
{
   uint32_t frames[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, 
                                  frames, 4);
}

EAPI Eina_Bool 
ecore_x_netwm_frame_size_get(Ecore_X_Window win, int *fl, int *fr, int *ft, int *fb) 
{
   int ret = 0;
   unsigned int frames[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, 
                                        frames, 4);
   if (ret != 4) return EINA_FALSE;

   if (fl) *fl = frames[0];
   if (fr) *fr = frames[1];
   if (ft) *ft = frames[2];
   if (fb) *fb = frames[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_sync_request_send(Ecore_X_Window win, unsigned int serial) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) return;

   /* FIXME: Maybe need XSyncIntToValue ?? */
   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_WM_PROTOCOLS;
   ev.data.data32[0] = ECORE_X_ATOM_NET_WM_SYNC_REQUEST;
   ev.data.data32[1] = _ecore_xcb_events_last_time_get();
   ev.data.data32[2] = serial;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
   ecore_x_flush();
}

EAPI void 
ecore_x_netwm_desktop_set(Ecore_X_Window win, unsigned int desk) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_DESKTOP, &desk, 1);
}

EAPI Eina_Bool 
ecore_x_netwm_desktop_get(Ecore_X_Window win, unsigned int *desk) 
{
   unsigned int tmp = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_DESKTOP, 
                                       &tmp, 1))
     return EINA_FALSE;

   if (desk) *desk = tmp;

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_desktop_request_send(Ecore_X_Window win, Ecore_X_Window root, unsigned int desktop) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_NET_WM_DESKTOP;
   ev.data.data32[0] = desktop;

   xcb_send_event(_ecore_xcb_conn, 0, root, 
                  (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), (const char *)&ev);
   ecore_x_flush();
}

EAPI void 
ecore_x_netwm_handled_icons_set(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS, 
                                  NULL, 0);
}

EAPI Eina_Bool 
ecore_x_netwm_handled_icons_get(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS, 
                                       NULL, 0))
     return EINA_FALSE;

   return EINA_TRUE;
}

EAPI int 
ecore_x_netwm_icon_name_get(Ecore_X_Window win, char **name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (name) 
     {
        *name = 
          ecore_x_window_prop_string_get(win, ECORE_X_ATOM_NET_WM_ICON_NAME);
     }

   return 1;
}

EAPI void 
ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_ICON_NAME, name);
}

EAPI Eina_Bool 
ecore_x_netwm_icons_get(Ecore_X_Window win, Ecore_X_Icon **icon, int *num) 
{
   int num_ret = 0;
   unsigned int i = 0, len = 0, icons = 0;
   unsigned int *data, *p, *src;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = 0;
   if (icon) *icon = NULL;

   num_ret = 
     ecore_x_window_prop_card32_list_get(win, ECORE_X_ATOM_NET_WM_ICON, &data);

   if ((num_ret <= 0) || (!data)) 
     {
        if (data) free(data);
        return EINA_FALSE;
     }
   if (num_ret < 2) 
     {
        if (data) free(data);
        return EINA_FALSE;
     }

   icons = 0;
   p = data;
   while (p) 
     {
        len = (p[0] * p[1]);
        p += (len + 2);
        if ((p - data) > num_ret) 
          {
             if (data) free(data);
             return EINA_FALSE;
          }
        icons++;
        if ((p - data) == num_ret) p = NULL;
     }
   if (num) *num = icons;
   if (!icon) 
     {
        if (data) free(data);
        return EINA_TRUE;
     }

   *icon = malloc(icons * sizeof(Ecore_X_Icon));
   if (!(*icon)) 
     {
        if (data) free(data);
        return EINA_FALSE;
     }

   /* Fetch the icons */
   p = data;
   for (i = 0; i < icons; i++) 
     {
        unsigned int *ps, *pd, *pe;

        len = p[0] * p[1];
        ((*icon)[i]).width = p[0];
        ((*icon)[i]).height = p[1];
        src = &(p[2]);
        ((*icon)[i]).data = malloc(len * sizeof(unsigned int));
        if (!((*icon)[i]).data)
          {
             while (i)
               free(((*icon)[--i]).data);
             free(*icon);
             free(data);
             return EINA_FALSE;
          }

        pd = ((*icon)[i]).data;
        ps = src;
        pe = ps + len;
        for (; ps < pe; ps++)
          {
             unsigned int r, g, b, a;

             a = (*ps >> 24) & 0xff;
             r = (((*ps >> 16) & 0xff) * a) / 255;
             g = (((*ps >> 8) & 0xff) * a) / 255;
             b = (((*ps) & 0xff) * a) / 255;
             *pd = (a << 24) | (r << 16) | (g << 8) | (b);
             pd++;
          }
        p += (len + 2);
     }

   if (data) free(data);
   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_icon_geometry_set(Ecore_X_Window win, int x, int y, int w, int h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, 
                                  geom, 4);
}

EAPI Eina_Bool 
ecore_x_netwm_icon_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h) 
{
   int ret = 0;
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = 
     ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, 
                                    geom, 4);
   if (ret != 4) return EINA_FALSE;
   if (x) *x = geom[0];
   if (y) *y = geom[1];
   if (w) *w = geom[2];
   if (h) *h = geom[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_strut_set(Ecore_X_Window win, int l, int r, int t, int b) 
{
   unsigned int strut[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   strut[0] = l;
   strut[1] = r;
   strut[2] = t;
   strut[3] = b;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
}

EAPI Eina_Bool 
ecore_x_netwm_strut_get(Ecore_X_Window win, int *l, int *r, int *t, int *b) 
{
   unsigned int strut[4];
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = 
     ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
   if (ret != 4) return EINA_FALSE;

   if (l) *l = strut[0];
   if (r) *r = strut[1];
   if (t) *t = strut[2];
   if (b) *b = strut[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_strut_partial_set(Ecore_X_Window win, int left, int right, int top, int bottom, int left_start_y, int left_end_y, int right_start_y, int right_end_y, int top_start_x, int top_end_x, int bottom_start_x, int bottom_end_x)
{
   unsigned int strut[12];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   strut[4] = left_start_y;
   strut[5] = left_end_y;
   strut[6] = right_start_y;
   strut[7] = right_end_y;
   strut[8] = top_start_x;
   strut[9] = top_end_x;
   strut[10] = bottom_start_x;
   strut[11] = bottom_end_x;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL,
                                  strut, 12);
}

EAPI Eina_Bool 
ecore_x_netwm_strut_partial_get(Ecore_X_Window win, int *left, int *right, int *top, int *bottom, int *left_start_y, int *left_end_y, int *right_start_y, int *right_end_y, int *top_start_x, int *top_end_x, int *bottom_start_x, int *bottom_end_x) 
{
   unsigned int strut[12];
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = 
     ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, 
                                    strut, 12);
   if (ret != 12) return EINA_FALSE;

   if (left) *left = strut[0];
   if (right) *right = strut[1];
   if (top) *top = strut[2];
   if (bottom) *bottom = strut[3];
   if (left_start_y) *left_start_y = strut[4];
   if (left_end_y) *left_end_y = strut[5];
   if (right_start_y) *right_start_y = strut[6];
   if (right_end_y) *right_end_y = strut[7];
   if (top_start_x) *top_start_x = strut[8];
   if (top_end_x) *top_end_x = strut[9];
   if (bottom_start_x) *bottom_start_x = strut[10];
   if (bottom_end_x) *bottom_end_x = strut[11];

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_user_time_set(Ecore_X_Window win, unsigned int t) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_USER_TIME, &t, 1);
}

EAPI Eina_Bool 
ecore_x_netwm_user_time_get(Ecore_X_Window win, unsigned int *t) 
{
   unsigned int tmp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_USER_TIME, 
                                        &tmp, 1))
     return EINA_FALSE;

   if (t) *t = tmp;

   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_NAME, 
                                  name);
}

EAPI int 
ecore_x_netwm_visible_name_get(Ecore_X_Window win, char **name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (name) 
     *name = ecore_x_window_prop_string_get(win, 
                                            ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
   return 1;
}

EAPI void 
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_string_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME, 
                                  name);
}

EAPI int 
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win, char **name) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (name) 
     {
        *name = 
          ecore_x_window_prop_string_get(win, 
                                         ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
     }

   return 1;
}

EAPI Eina_Bool 
ecore_x_netwm_sync_counter_get(Ecore_X_Window win, Ecore_X_Sync_Counter *counter) 
{
   unsigned int tmp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, 
                                    ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER, 
                                    &tmp, 1))
     return EINA_FALSE;

   if (counter) *counter = tmp;

   return EINA_TRUE;
}

EAPI Eina_Bool 
ecore_x_netwm_allowed_action_isset(Ecore_X_Window win, Ecore_X_Action action) 
{
   int num = 0, i = 0;
   Ecore_X_Atom *atoms, atom;
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   num = 
     ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE, 
                                       &atoms);
   if (num <= 0) return EINA_FALSE;

   atom = _ecore_xcb_netwm_action_atom_get(action);
   for (i = 0; i < num; i++) 
     {
        if (atoms[i] == atom) 
          {
             ret = EINA_TRUE;
             break;
          }
     }

   if (atoms) free(atoms);
   return ret;
}

EAPI Eina_Bool 
ecore_x_netwm_allowed_action_get(Ecore_X_Window win, Ecore_X_Action **action, unsigned int *num) 
{
   Ecore_X_Atom *atoms;
   int num_ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (num) *num = 0;
   if (action) *action = NULL;

   num_ret = 
     ecore_x_window_prop_atom_list_get(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, 
                                       &atoms);
   if (num_ret <= 0) return EINA_FALSE;
   if (action) 
     {
        *action = malloc(num_ret * sizeof(Ecore_X_Action));
        if (*action) 
          {
             int i = 0;

             for (i = 0; i < num_ret; i++)
               (*action)[i] = _ecore_xcb_netwm_action_atom_get(atoms[i]);
          }
        if (num) *num = num_ret;
     }
   free(atoms);
   return EINA_TRUE;
}

EAPI void 
ecore_x_netwm_allowed_action_set(Ecore_X_Window win, Ecore_X_Action *action, unsigned int num) 
{
   Ecore_X_Atom *set;
   unsigned int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!num) 
     {
        ecore_x_window_prop_property_del(win, 
                                         ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS);
        return;
     }

   set = malloc(num * sizeof(Ecore_X_Atom));
   if (!set) return;

   for (i = 0; i < num; i++)
     set[i] = _ecore_xcb_netwm_action_atom_get(action[i]);

   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS, 
                                set, num);
   free(set);
}

/* local functions */
int 
_ecore_xcb_netwm_startup_info_begin(Ecore_X_Window win __UNUSED__, uint8_t data __UNUSED__) 
{
   // TODO: TBD
   return 1;
}

int 
_ecore_xcb_netwm_startup_info(Ecore_X_Window win __UNUSED__, uint8_t data __UNUSED__) 
{
   // TODO: TBD
   return 1;
}

static void 
_ecore_xcb_netwm_startup_info_free(void *data) 
{
   Ecore_Xcb_Startup_Info *info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(info = data)) return;
   if (info->buffer) free(info->buffer);
   if (info->id) free(info->id);
   if (info->name) free(info->name);
   if (info->bin) free(info->bin);
   if (info->icon) free(info->icon);
   if (info->description) free(info->description);
   if (info->wmclass) free(info->wmclass);
   free(info);
}

static Ecore_X_Atom 
_ecore_xcb_netwm_window_type_atom_get(Ecore_X_Window_Type type) 
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
      case ECORE_X_WINDOW_TYPE_DROPDOWN_MENU:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
      case ECORE_X_WINDOW_TYPE_POPUP_MENU:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU;
      case ECORE_X_WINDOW_TYPE_TOOLTIP:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP;
      case ECORE_X_WINDOW_TYPE_NOTIFICATION:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION;
      case ECORE_X_WINDOW_TYPE_COMBO:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_COMBO;
      case ECORE_X_WINDOW_TYPE_DND:
        return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DND;
      default:
        return 0;
     }
}

static Ecore_X_Window_Type 
_ecore_xcb_netwm_window_type_type_get(Ecore_X_Atom atom) 
{
   if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP)
     return ECORE_X_WINDOW_TYPE_DESKTOP;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK)
     return ECORE_X_WINDOW_TYPE_DOCK;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR)
     return ECORE_X_WINDOW_TYPE_TOOLBAR;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU)
     return ECORE_X_WINDOW_TYPE_MENU;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY)
     return ECORE_X_WINDOW_TYPE_UTILITY;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH)
     return ECORE_X_WINDOW_TYPE_SPLASH;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG)
     return ECORE_X_WINDOW_TYPE_DIALOG;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL)
     return ECORE_X_WINDOW_TYPE_NORMAL;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU)
      return ECORE_X_WINDOW_TYPE_DROPDOWN_MENU;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU)
      return ECORE_X_WINDOW_TYPE_POPUP_MENU;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP)
      return ECORE_X_WINDOW_TYPE_TOOLTIP;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION)
      return ECORE_X_WINDOW_TYPE_NOTIFICATION;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_COMBO)
      return ECORE_X_WINDOW_TYPE_COMBO;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DND)
      return ECORE_X_WINDOW_TYPE_DND;
   else
     return ECORE_X_WINDOW_TYPE_UNKNOWN;
}

static Ecore_X_Atom 
_ecore_xcb_netwm_window_state_atom_get(Ecore_X_Window_State state) 
{
   switch (state) 
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
        return ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
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

Ecore_X_Window_State 
_ecore_xcb_netwm_window_state_get(Ecore_X_Atom atom) 
{
   if (atom == ECORE_X_ATOM_NET_WM_STATE_MODAL)
      return ECORE_X_WINDOW_STATE_MODAL;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_STICKY)
      return ECORE_X_WINDOW_STATE_STICKY;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
      return ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
      return ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_SHADED)
      return ECORE_X_WINDOW_STATE_SHADED;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR)
      return ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER)
      return ECORE_X_WINDOW_STATE_SKIP_PAGER;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_HIDDEN)
      return ECORE_X_WINDOW_STATE_HIDDEN;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN)
      return ECORE_X_WINDOW_STATE_FULLSCREEN;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_ABOVE)
      return ECORE_X_WINDOW_STATE_ABOVE;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_BELOW)
      return ECORE_X_WINDOW_STATE_BELOW;
   else if (atom == ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION)
      return ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION;
   else
      return ECORE_X_WINDOW_STATE_UNKNOWN;
}

static Ecore_X_Atom 
_ecore_xcb_netwm_action_atom_get(Ecore_X_Action action) 
{
   switch (action)
     {
      case ECORE_X_ACTION_MOVE:
        return ECORE_X_ATOM_NET_WM_ACTION_MOVE;
      case ECORE_X_ACTION_RESIZE:
        return ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
      case ECORE_X_ACTION_MINIMIZE:
        return ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
      case ECORE_X_ACTION_SHADE:
        return ECORE_X_ATOM_NET_WM_ACTION_SHADE;
      case ECORE_X_ACTION_STICK:
        return ECORE_X_ATOM_NET_WM_ACTION_STICK;
      case ECORE_X_ACTION_MAXIMIZE_HORZ:
        return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
      case ECORE_X_ACTION_MAXIMIZE_VERT:
        return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
      case ECORE_X_ACTION_FULLSCREEN:
        return ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
      case ECORE_X_ACTION_CHANGE_DESKTOP:
        return ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
      case ECORE_X_ACTION_CLOSE:
        return ECORE_X_ATOM_NET_WM_ACTION_CLOSE;
      case ECORE_X_ACTION_ABOVE:
        return ECORE_X_ATOM_NET_WM_ACTION_ABOVE;
      case ECORE_X_ACTION_BELOW:
        return ECORE_X_ATOM_NET_WM_ACTION_BELOW;
      default:
        return 0;
     }
}
