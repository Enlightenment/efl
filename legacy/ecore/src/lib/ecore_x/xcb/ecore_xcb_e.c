#include "ecore_xcb_private.h"

/* local function prototypes */
static Ecore_X_Atom _ecore_xcb_e_vkbd_atom_get(Ecore_X_Virtual_Keyboard_State state);
static Ecore_X_Virtual_Keyboard_State _ecore_xcb_e_vkbd_state_get(Ecore_X_Atom atom);
static Ecore_X_Atom _ecore_xcb_e_quickpanel_atom_get(Ecore_X_Illume_Quickpanel_State state);
static Ecore_X_Illume_Quickpanel_State _ecore_xcb_e_quickpanel_state_get(Ecore_X_Atom atom);
static Ecore_X_Atom _ecore_xcb_e_illume_atom_get(Ecore_X_Illume_Mode mode);
static Ecore_X_Illume_Mode _ecore_xcb_e_illume_mode_get(Ecore_X_Atom atom);

EAPI void 
ecore_x_e_init(void) 
{

}

EAPI void 
ecore_x_e_comp_sync_draw_done_send(Ecore_X_Window root, Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root, 
                  (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_sync_draw_size_done_send(Ecore_X_Window root, Ecore_X_Window win, int w, int h) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 1;
   ev.data.data32[2] = w;
   ev.data.data32[3] = h;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, root, 
                  (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                      XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_sync_counter_set(Ecore_X_Window win, Ecore_X_Sync_Counter counter) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (counter)
     ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_SYNC_COUNTER, 
                                 ECORE_X_ATOM_CARDINAL, &counter, 1);
   else
     ecore_x_window_prop_property_del(win, ECORE_X_ATOM_E_COMP_SYNC_COUNTER);
}

EAPI Ecore_X_Sync_Counter 
ecore_x_e_comp_sync_counter_get(Ecore_X_Window win) 
{
   Ecore_X_Sync_Counter counter = 0;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = ecore_x_window_prop_xid_get(win, ECORE_X_ATOM_E_COMP_SYNC_COUNTER, 
                                     ECORE_X_ATOM_CARDINAL, &counter, 1);
   if (ret != 1) return 0;
   return counter;
}

EAPI Eina_Bool 
ecore_x_e_comp_sync_supported_get(Ecore_X_Window root) 
{
   Ecore_X_Window win, win2;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   ret = 
     ecore_x_window_prop_xid_get(root, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED, 
                                 ECORE_X_ATOM_WINDOW, &win, 1);
   if ((ret == 1) && (win)) 
     {
        ret = 
          ecore_x_window_prop_xid_get(win, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED, 
                                      ECORE_X_ATOM_WINDOW, &win2, 1);
        if ((ret == 1) && (win2 == win)) 
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI void 
ecore_x_e_comp_sync_supported_set(Ecore_X_Window root, Eina_Bool enabled) 
{
   Ecore_X_Window win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!root) root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   if (enabled) 
     {
        win = ecore_x_window_new(root, 1, 2, 3, 4);
        ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED, 
                                    ECORE_X_ATOM_WINDOW, &win, 1);
        ecore_x_window_prop_xid_set(root, ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED, 
                                    ECORE_X_ATOM_WINDOW, &win, 1);
     }
   else 
     {
        int ret = 0;

        ret = ecore_x_window_prop_xid_get(root, 
                                          ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED, 
                                          ECORE_X_ATOM_WINDOW, &win, 1);
        if ((ret == 1) && (win)) 
          {
             ecore_x_window_prop_property_del(root, 
                                              ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED);
             ecore_x_window_free(win);
          }
     }
}

EAPI void 
ecore_x_e_comp_sync_begin_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_SYNC_BEGIN;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_sync_end_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_SYNC_END;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_sync_cancel_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_SYNC_CANCEL;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_flush_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_FLUSH;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_dump_send(Ecore_X_Window win) 
{
   xcb_client_message_event_t ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   memset(&ev, 0, sizeof(xcb_client_message_event_t));

   ev.response_type = XCB_CLIENT_MESSAGE;
   ev.format = 32;
   ev.window = win;
   ev.type = ECORE_X_ATOM_E_COMP_DUMP;
   ev.data.data32[0] = win;
   ev.data.data32[1] = 0;
   ev.data.data32[2] = 0;
   ev.data.data32[3] = 0;
   ev.data.data32[4] = 0;

   xcb_send_event(_ecore_xcb_conn, 0, win, 
                  XCB_EVENT_MASK_NO_EVENT, (const char *)&ev);
//   ecore_x_flush();
}

EAPI void 
ecore_x_e_comp_pixmap_set(Ecore_X_Window win, Ecore_X_Pixmap pixmap) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (pixmap)
     ecore_x_window_prop_xid_set(win, ECORE_X_ATOM_E_COMP_PIXMAP, 
                                 ECORE_X_ATOM_PIXMAP, &pixmap, 1);
   else
     ecore_x_window_prop_property_del(win, pixmap);
}

EAPI Ecore_X_Pixmap 
ecore_x_e_comp_pixmap_get(Ecore_X_Window win) 
{
   Ecore_X_Pixmap pixmap = 0;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ret = ecore_x_window_prop_xid_get(win, ECORE_X_ATOM_E_COMP_PIXMAP, 
                                     ECORE_X_ATOM_PIXMAP, &pixmap, 1);
   if (ret != 1) return 0;
   return pixmap;
}

EAPI void 
ecore_x_e_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb) 
{
   uint32_t frames[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_FRAME_SIZE, frames, 4);
}

EAPI Ecore_X_Virtual_Keyboard_State 
ecore_x_e_virtual_keyboard_state_get(Ecore_X_Window win) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE, 
                                     &atom, 1))
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;

   return _ecore_xcb_e_vkbd_state_get(atom);
}

EAPI void 
ecore_x_e_virtual_keyboard_state_set(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   atom = _ecore_xcb_e_vkbd_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE, 
                                &atom, 1);
}

EAPI void 
ecore_x_e_virtual_keyboard_state_send(Ecore_X_Window win, Ecore_X_Virtual_Keyboard_State state) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 _ecore_xcb_e_vkbd_atom_get(state), 
                                 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_virtual_keyboard_set(Ecore_X_Window win, unsigned int is_keyboard) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD, 
                                  &is_keyboard, 1);
}

EAPI Eina_Bool 
ecore_x_e_virtual_keyboard_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_VIRTUAL_KEYBOARD, 
                                       &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI int 
ecore_x_e_illume_quickpanel_priority_major_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, 
                                       ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR, 
                                       &val, 1))
     return 0;

   return val;
}

EAPI void 
ecore_x_e_illume_quickpanel_priority_major_set(Ecore_X_Window win, unsigned int priority) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, 
                                  ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR, 
                                  &priority, 1);
}

EAPI int 
ecore_x_e_illume_quickpanel_priority_minor_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, 
                                       ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR, 
                                       &val, 1))
     return 0;

   return val;
}

EAPI void 
ecore_x_e_illume_quickpanel_priority_minor_set(Ecore_X_Window win, unsigned int priority) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, 
                                  ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR, 
                                  &priority, 1);
}

EAPI void 
ecore_x_e_illume_quickpanel_zone_set(Ecore_X_Window win, unsigned int zone) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ZONE, 
                                  &zone, 1);
}

EAPI int 
ecore_x_e_illume_quickpanel_zone_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, 
                                       ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ZONE, 
                                       &val, 1))
     return 0;

   return val;
}

EAPI void 
ecore_x_e_illume_quickpanel_position_update_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, 
                                 ECORE_X_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI Eina_Bool 
ecore_x_e_illume_conformant_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT, 
                                       &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void 
ecore_x_e_illume_conformant_set(Ecore_X_Window win, unsigned int is_conformant) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_CONFORMANT, 
                                  &is_conformant, 1);
}

EAPI void 
ecore_x_e_illume_softkey_geometry_set(Ecore_X_Window win, int x, int y, int w, int h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY, 
                                  geom, 4);
}

EAPI Eina_Bool 
ecore_x_e_illume_softkey_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;

   if (ecore_x_window_prop_card32_get(win, 
                                      ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY, 
                                      geom, 4) != 4)
     return EINA_FALSE;

   if (x) *x = geom[0];
   if (y) *y = geom[1];
   if (w) *w = geom[2];
   if (h) *h = geom[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_e_illume_indicator_geometry_set(Ecore_X_Window win, int x, int y, int w, int h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, 
                                  ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY, 
                                  geom, 4);
}

EAPI Eina_Bool 
ecore_x_e_illume_indicator_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;

   if (ecore_x_window_prop_card32_get(win, 
                                      ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY, 
                                      geom, 4) != 4)
     return EINA_FALSE;

   if (x) *x = geom[0];
   if (y) *y = geom[1];
   if (w) *w = geom[2];
   if (h) *h = geom[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_e_illume_keyboard_geometry_set(Ecore_X_Window win, int x, int y, int w, int h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   geom[0] = x;
   geom[1] = y;
   geom[2] = w;
   geom[3] = h;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY, 
                                  geom, 4);
}

EAPI Eina_Bool 
ecore_x_e_illume_keyboard_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h) 
{
   unsigned int geom[4];

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;

   if (ecore_x_window_prop_card32_get(win, 
                                      ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY, 
                                      geom, 4) != 4)
     return EINA_FALSE;

   if (x) *x = geom[0];
   if (y) *y = geom[1];
   if (w) *w = geom[2];
   if (h) *h = geom[3];

   return EINA_TRUE;
}

EAPI void 
ecore_x_e_illume_quickpanel_set(Ecore_X_Window win, unsigned int is_quickpanel) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL, 
                                  &is_quickpanel, 1);
}

EAPI Eina_Bool 
ecore_x_e_illume_quickpanel_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL, 
                                       &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void 
ecore_x_e_illume_quickpanel_state_set(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   atom = _ecore_xcb_e_quickpanel_atom_get(state);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE, 
                                &atom, 1);
}

EAPI Ecore_X_Illume_Quickpanel_State 
ecore_x_e_illume_quickpanel_state_get(Ecore_X_Window win) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_atom_get(win, 
                                     ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE, 
                                     &atom, 1))
     return ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN;

   return _ecore_xcb_e_quickpanel_state_get(atom);
}

EAPI void 
ecore_x_e_illume_quickpanel_state_send(Ecore_X_Window win, Ecore_X_Illume_Quickpanel_State state) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 _ecore_xcb_e_quickpanel_atom_get(state), 
                                 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_quickpanel_state_toggle(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, 
                                 ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 0, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_mode_set(Ecore_X_Window win, Ecore_X_Illume_Mode mode) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   atom = _ecore_xcb_e_illume_atom_get(mode);
   ecore_x_window_prop_atom_set(win, ECORE_X_ATOM_E_ILLUME_MODE, &atom, 1);
}

EAPI Ecore_X_Illume_Mode 
ecore_x_e_illume_mode_get(Ecore_X_Window win) 
{
   Ecore_X_Atom atom = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_atom_get(win, ECORE_X_ATOM_E_ILLUME_MODE, &atom, 1))
     return ECORE_X_ILLUME_MODE_UNKNOWN;

   return _ecore_xcb_e_illume_mode_get(atom);
}

EAPI void 
ecore_x_e_illume_mode_send(Ecore_X_Window win, Ecore_X_Illume_Mode mode) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_MODE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 _ecore_xcb_e_illume_atom_get(mode), 
                                 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_focus_back_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_BACK, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_focus_forward_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_FORWARD, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_focus_home_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_FOCUS_HOME, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_close_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_CLOSE, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_home_new_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_HOME_NEW, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_home_del_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_HOME_DEL, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_drag_set(Ecore_X_Window win, unsigned int drag) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG, &drag, 1);
}

EAPI void 
ecore_x_e_illume_drag_locked_set(Ecore_X_Window win, unsigned int is_locked) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED, 
                                  &is_locked, 1);
}

EAPI Eina_Bool 
ecore_x_e_illume_drag_locked_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED, 
                                       &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_e_illume_drag_get(Ecore_X_Window win) 
{
   unsigned int val = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_E_ILLUME_DRAG, &val, 1))
     return EINA_FALSE;

   return val ? EINA_TRUE : EINA_FALSE;
}

EAPI void 
ecore_x_e_illume_drag_start_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_START, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_drag_end_send(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   ecore_x_client_message32_send(win, ECORE_X_ATOM_E_ILLUME_DRAG_END, 
                                 ECORE_X_EVENT_MASK_WINDOW_CONFIGURE, 
                                 1, 0, 0, 0, 0);
}

EAPI void 
ecore_x_e_illume_zone_set(Ecore_X_Window win, Ecore_X_Window zone) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_E_ILLUME_ZONE, &zone, 1);
}

EAPI Ecore_X_Window 
ecore_x_e_illume_zone_get(Ecore_X_Window win) 
{
   Ecore_X_Window zone;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ecore_x_window_prop_window_get(win, ECORE_X_ATOM_E_ILLUME_ZONE, 
                                       &zone, 1))
     return 0;

   return zone;
}

EAPI void 
ecore_x_e_illume_zone_list_set(Ecore_X_Window win, Ecore_X_Window *zones, unsigned int num) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_window_prop_window_set(win, ECORE_X_ATOM_E_ILLUME_ZONE_LIST, 
                                  zones, num);
}

/* local functions */
static Ecore_X_Atom
_ecore_xcb_e_vkbd_atom_get(Ecore_X_Virtual_Keyboard_State state)
{
   switch (state)
     {
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_ON:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ON;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ALPHA;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PIN;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HEX;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_IP:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_IP;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_HOST:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HOST;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_FILE:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_FILE;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_URL:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_URL;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_KEYPAD:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD;
      case ECORE_X_VIRTUAL_KEYBOARD_STATE_J2ME:
        return ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_J2ME;
      default: 
        break;
     }
   return 0;
}

static Ecore_X_Virtual_Keyboard_State
_ecore_xcb_e_vkbd_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ON)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_ON;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ALPHA)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PIN)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PHONE_NUMBER;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HEX)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_HEX;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_TERMINAL;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_PASSWORD;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_IP)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_IP;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HOST)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_HOST;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_FILE)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_FILE;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_URL)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_URL;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_KEYPAD;
   if (atom == ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_J2ME)
     return ECORE_X_VIRTUAL_KEYBOARD_STATE_J2ME;

   return ECORE_X_VIRTUAL_KEYBOARD_STATE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_xcb_e_quickpanel_atom_get(Ecore_X_Illume_Quickpanel_State state)
{
   switch (state)
     {
      case ECORE_X_ILLUME_QUICKPANEL_STATE_ON:
        return ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON;
      case ECORE_X_ILLUME_QUICKPANEL_STATE_OFF:
        return ECORE_X_ATOM_E_ILLUME_QUICKPANEL_OFF;
      default:
        break;
     }
   return 0;
}

static Ecore_X_Illume_Quickpanel_State
_ecore_xcb_e_quickpanel_state_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON)
     return ECORE_X_ILLUME_QUICKPANEL_STATE_ON;
   if (atom == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_OFF)
     return ECORE_X_ILLUME_QUICKPANEL_STATE_OFF;

   return ECORE_X_ILLUME_QUICKPANEL_STATE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_xcb_e_illume_atom_get(Ecore_X_Illume_Mode mode)
{
   switch (mode)
     {
      case ECORE_X_ILLUME_MODE_SINGLE:
        return ECORE_X_ATOM_E_ILLUME_MODE_SINGLE;
      case ECORE_X_ILLUME_MODE_DUAL_TOP:
        return ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP;
      case ECORE_X_ILLUME_MODE_DUAL_LEFT:
        return ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT;
      default:
        break;
     }
   return ECORE_X_ILLUME_MODE_UNKNOWN;
}

static Ecore_X_Illume_Mode
_ecore_xcb_e_illume_mode_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_SINGLE)
     return ECORE_X_ILLUME_MODE_SINGLE;
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP)
     return ECORE_X_ILLUME_MODE_DUAL_TOP;
   if (atom == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT)
     return ECORE_X_ILLUME_MODE_DUAL_LEFT;

   return ECORE_X_ILLUME_MODE_UNKNOWN;
}
