#include "ecore_xcb_private.h"
//#include "Ecore_X_Atoms.h"
#include <langinfo.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_event.h>
# ifdef ECORE_XCB_DAMAGE
#  include <xcb/damage.h>
# endif
# ifdef ECORE_XCB_RANDR
#  include <xcb/randr.h>
# endif
# ifdef ECORE_XCB_SCREENSAVER
#  include <xcb/screensaver.h>
# endif
# ifdef ECORE_XCB_SYNC
#  include <xcb/sync.h>
# endif
# ifdef ECORE_XCB_XFIXES
#  include <xcb/xfixes.h>
# endif

#ifndef CODESET
# define CODESET "INVALID"
#endif

typedef struct _Ecore_X_Mouse_Down_Info 
{
   EINA_INLIST;
   int dev;
   Ecore_X_Time last_time;
   Ecore_X_Time last_last_time;
   Ecore_X_Window last_win;
   Ecore_X_Window last_last_win;
   Ecore_X_Window last_event_win;
   Ecore_X_Window last_last_event_win;
   Eina_Bool did_double : 1;
   Eina_Bool did_triple : 1;
} Ecore_X_Mouse_Down_Info;

/* local function prototypes */
static void _ecore_xcb_event_handle_any_event(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_key_press(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_key_release(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_button_press(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_button_release(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_motion_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_enter_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_leave_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_keymap_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_focus_in(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_focus_out(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_expose(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_graphics_exposure(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_visibility_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_create_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_destroy_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_map_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_unmap_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_map_request(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_reparent_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_configure_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_configure_request(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_gravity_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_resize_request(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_circulate_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_circulate_request(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_property_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_selection_clear(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_selection_request(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_selection_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_colormap_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_client_message(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_mapping_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_damage_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_randr_change(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_randr_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_randr_crtc_change(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_randr_output_change(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_randr_output_property_change(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_screensaver_notify(xcb_generic_event_t *event);
#ifdef ECORE_XCB_SHAPE
static void _ecore_xcb_event_handle_shape_change(xcb_generic_event_t *event);
#endif
static void _ecore_xcb_event_handle_sync_counter(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_sync_alarm(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_xfixes_selection_notify(xcb_generic_event_t *event __UNUSED__);
static void _ecore_xcb_event_handle_xfixes_cursor_notify(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_generic_event(xcb_generic_event_t *event);
static void _ecore_xcb_event_handle_input_event(xcb_generic_event_t *event);

static void _ecore_xcb_event_key_press(xcb_generic_event_t *event);
static void _ecore_xcb_event_key_release(xcb_generic_event_t *event);
static void _ecore_xcb_event_mouse_move_free(void *data __UNUSED__, void *event);
static Ecore_X_Event_Mode _ecore_xcb_event_mode_get(uint8_t mode);
static Ecore_X_Event_Detail _ecore_xcb_event_detail_get(uint8_t detail);
static void _ecore_xcb_event_xdnd_enter_free(void *data __UNUSED__, void *event);
static void _ecore_xcb_event_selection_notify_free(void *data __UNUSED__, void *event);
static void _ecore_xcb_event_generic_event_free(void *data, void *event);
static void _ecore_xcb_event_mouse_down_info_clear(void);
static Ecore_X_Mouse_Down_Info *_ecore_xcb_event_mouse_down_info_get(int dev);

/* local variables */
static Eina_Bool _ecore_xcb_event_last_mouse_move = EINA_FALSE;
//static Ecore_Event *_ecore_xcb_event_last_mouse_move_event = NULL;
static Eina_Inlist *_ecore_xcb_mouse_down_info_list = NULL;
static Ecore_X_Time _ecore_xcb_event_last_time;
static Ecore_X_Window _ecore_xcb_event_last_window = 0;

/* public variables */
int16_t _ecore_xcb_event_last_root_x = 0;
int16_t _ecore_xcb_event_last_root_y = 0;

EAPI int ECORE_X_EVENT_ANY = 0;
EAPI int ECORE_X_EVENT_MOUSE_IN = 0;
EAPI int ECORE_X_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_X_EVENT_WINDOW_FOCUS_IN = 0;
EAPI int ECORE_X_EVENT_WINDOW_FOCUS_OUT = 0;
EAPI int ECORE_X_EVENT_WINDOW_KEYMAP = 0;
EAPI int ECORE_X_EVENT_WINDOW_DAMAGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_CREATE = 0;
EAPI int ECORE_X_EVENT_WINDOW_DESTROY = 0;
EAPI int ECORE_X_EVENT_WINDOW_HIDE = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHOW = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHOW_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_REPARENT = 0;
EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_GRAVITY = 0;
EAPI int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_STACK = 0;
EAPI int ECORE_X_EVENT_WINDOW_STACK_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_PROPERTY = 0;
EAPI int ECORE_X_EVENT_WINDOW_COLORMAP = 0;
EAPI int ECORE_X_EVENT_WINDOW_MAPPING = 0;
EAPI int ECORE_X_EVENT_MAPPING_CHANGE = 0;
EAPI int ECORE_X_EVENT_SELECTION_CLEAR = 0;
EAPI int ECORE_X_EVENT_SELECTION_REQUEST = 0;
EAPI int ECORE_X_EVENT_SELECTION_NOTIFY = 0;
EAPI int ECORE_X_EVENT_FIXES_SELECTION_NOTIFY = 0;
EAPI int ECORE_X_EVENT_CLIENT_MESSAGE = 0;
EAPI int ECORE_X_EVENT_WINDOW_SHAPE = 0;
EAPI int ECORE_X_EVENT_SCREENSAVER_NOTIFY = 0;
EAPI int ECORE_X_EVENT_SYNC_COUNTER = 0;
EAPI int ECORE_X_EVENT_SYNC_ALARM = 0;
EAPI int ECORE_X_EVENT_SCREEN_CHANGE = 0;
EAPI int ECORE_X_EVENT_DAMAGE_NOTIFY = 0;
EAPI int ECORE_X_EVENT_RANDR_CRTC_CHANGE = 0;
EAPI int ECORE_X_EVENT_RANDR_OUTPUT_CHANGE = 0;
EAPI int ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY = 0;
EAPI int ECORE_X_EVENT_WINDOW_DELETE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = 0;
EAPI int ECORE_X_EVENT_WINDOW_STATE_REQUEST = 0;
EAPI int ECORE_X_EVENT_FRAME_EXTENTS_REQUEST = 0;
EAPI int ECORE_X_EVENT_PING = 0;
EAPI int ECORE_X_EVENT_DESKTOP_CHANGE = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_NEW = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE = 0;
EAPI int ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE = 0;
EAPI int ECORE_X_EVENT_GENERIC = 0;

void 
_ecore_xcb_events_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ECORE_X_EVENT_ANY)
     {
        ECORE_X_EVENT_ANY = ecore_event_type_new();
        ECORE_X_EVENT_MOUSE_IN = ecore_event_type_new();
        ECORE_X_EVENT_MOUSE_OUT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_FOCUS_IN = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_FOCUS_OUT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_KEYMAP = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_DAMAGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CREATE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_DESTROY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_HIDE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHOW = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHOW_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_REPARENT = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_GRAVITY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STACK = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STACK_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_PROPERTY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_COLORMAP = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_MAPPING = ecore_event_type_new();
        ECORE_X_EVENT_MAPPING_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_CLEAR = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_SELECTION_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_FIXES_SELECTION_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_CLIENT_MESSAGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_SHAPE = ecore_event_type_new();
        ECORE_X_EVENT_SCREENSAVER_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_COUNTER = ecore_event_type_new();
        ECORE_X_EVENT_SYNC_ALARM = ecore_event_type_new();
        ECORE_X_EVENT_SCREEN_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_CRTC_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_OUTPUT_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_DAMAGE_NOTIFY = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_DESKTOP_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_STATE_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_FRAME_EXTENTS_REQUEST = ecore_event_type_new();
        ECORE_X_EVENT_PING = ecore_event_type_new();
        ECORE_X_EVENT_STARTUP_SEQUENCE_NEW = ecore_event_type_new();
        ECORE_X_EVENT_STARTUP_SEQUENCE_CHANGE = ecore_event_type_new();
        ECORE_X_EVENT_STARTUP_SEQUENCE_REMOVE = ecore_event_type_new();
        ECORE_X_EVENT_GENERIC = ecore_event_type_new();
     }
}

void 
_ecore_xcb_events_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_event_mouse_down_info_clear();

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
//   if (_ecore_xcb_event_last_mouse_move_event) 
//     {
//        ecore_event_del(_ecore_xcb_event_last_mouse_move_event);
//        _ecore_xcb_event_last_mouse_move_event = NULL;
//     }
}

void 
_ecore_xcb_events_handle(xcb_generic_event_t *ev) 
{
   uint8_t response = 0;

//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* strip highest bit (set if event is generated) */
   response = (ev->response_type & ~0x80);
   if (response == 0) 
     {
        xcb_generic_error_t *err;

        err = (xcb_generic_error_t *)ev;

        /* NB: There is no way to check access of destroyed windows, 
         * so trap those cases and ignore. We also ignore BadValue from 
         * xcb_grab/ungrab_button (happens when we are using any_mod) 
         * and a few others */
#ifdef OLD_XCB_VERSION
        if (err->error_code == XCB_EVENT_ERROR_BAD_WINDOW) return;
        else if (err->error_code == XCB_EVENT_ERROR_BAD_MATCH) 
          {
             if ((err->major_code == XCB_SET_INPUT_FOCUS) || 
                 (err->major_code == XCB_CONFIGURE_WINDOW))
               return;
          }
        else if (err->error_code == XCB_EVENT_ERROR_BAD_VALUE) 
          {
             if ((err->major_code == XCB_KILL_CLIENT) || 
                 (err->major_code == XCB_GRAB_BUTTON) || 
                 (err->major_code == XCB_UNGRAB_BUTTON))
               return;
          }
#else
        if (err->error_code == XCB_WINDOW) return;
        else if (err->error_code == XCB_MATCH) 
          {
             if ((err->major_code == XCB_SET_INPUT_FOCUS) || 
                 (err->major_code == XCB_CONFIGURE_WINDOW))
               return;
          }
        else if (err->error_code == XCB_VALUE) 
          {
             if ((err->major_code == XCB_KILL_CLIENT) || 
                 (err->major_code == XCB_GRAB_BUTTON) || 
                 (err->major_code == XCB_UNGRAB_BUTTON))
               return;
          }
#endif
        WRN("Got Event Error:");
        WRN("\tMajor Code: %d", err->major_code);
        WRN("\tMinor Code: %d", err->minor_code);
        WRN("\tRequest: %s", xcb_event_get_request_label(err->major_code));
        WRN("\tError: %s", xcb_event_get_error_label(err->error_code));
        if (err->error_code == XCB_EVENT_ERROR_BAD_VALUE)
          WRN("\tValue: %d", ((xcb_value_error_t *)err)->bad_value);
        else if (err->error_code == XCB_EVENT_ERROR_BAD_MATCH) 
          WRN("\tMatch: %d", ((xcb_match_error_t *)err)->bad_value);

        if (err->major_code == XCB_SEND_EVENT) 
          {
             WRN("\tSend Event Error");
             WRN("\t\tSeq: %d", ev->sequence);
             WRN("\t\tFull Seq: %d", ev->full_sequence);
             WRN("\t\tType: %d", ev->response_type);
          }
        /* if (err->major_code == 148)  */
        /*   { */
        /*      printf("GOT 148 Error\n"); */
        /*   } */
        return;
     }

   /* FIXME: Filter event for xim when xcb supports xim */

   _ecore_xcb_event_handle_any_event(ev);

   if (response == XCB_KEY_PRESS)
     _ecore_xcb_event_handle_key_press(ev);
   else if (response == XCB_KEY_RELEASE)
     _ecore_xcb_event_handle_key_release(ev);
   else if (response == XCB_BUTTON_PRESS) 
     _ecore_xcb_event_handle_button_press(ev);
   else if (response == XCB_BUTTON_RELEASE)
     _ecore_xcb_event_handle_button_release(ev);
   else if (response == XCB_MOTION_NOTIFY)
     _ecore_xcb_event_handle_motion_notify(ev);
   else if (response == XCB_ENTER_NOTIFY) 
     _ecore_xcb_event_handle_enter_notify(ev);
   else if (response == XCB_LEAVE_NOTIFY) 
     _ecore_xcb_event_handle_leave_notify(ev);
   else if (response == XCB_KEYMAP_NOTIFY)
     _ecore_xcb_event_handle_keymap_notify(ev);
   else if (response == XCB_FOCUS_IN) 
     _ecore_xcb_event_handle_focus_in(ev);
   else if (response == XCB_FOCUS_OUT) 
     _ecore_xcb_event_handle_focus_out(ev);
   else if (response == XCB_EXPOSE)
     _ecore_xcb_event_handle_expose(ev);
   else if (response == XCB_GRAPHICS_EXPOSURE)
     _ecore_xcb_event_handle_graphics_exposure(ev);
   else if (response == XCB_VISIBILITY_NOTIFY)
     _ecore_xcb_event_handle_visibility_notify(ev);
   else if (response == XCB_CREATE_NOTIFY)
     _ecore_xcb_event_handle_create_notify(ev);
   else if (response == XCB_DESTROY_NOTIFY)
     _ecore_xcb_event_handle_destroy_notify(ev);
   else if (response == XCB_MAP_NOTIFY) 
     _ecore_xcb_event_handle_map_notify(ev);
   else if (response == XCB_UNMAP_NOTIFY) 
     _ecore_xcb_event_handle_unmap_notify(ev);
   else if (response == XCB_MAP_REQUEST)
     _ecore_xcb_event_handle_map_request(ev);
   else if (response == XCB_REPARENT_NOTIFY)
     _ecore_xcb_event_handle_reparent_notify(ev);
   else if (response == XCB_CONFIGURE_NOTIFY) 
     _ecore_xcb_event_handle_configure_notify(ev);
   else if (response == XCB_CONFIGURE_REQUEST) 
     _ecore_xcb_event_handle_configure_request(ev);
   else if (response == XCB_GRAVITY_NOTIFY)
     _ecore_xcb_event_handle_gravity_notify(ev);
   else if (response == XCB_RESIZE_REQUEST)
     _ecore_xcb_event_handle_resize_request(ev);
   else if (response == XCB_CIRCULATE_NOTIFY)
     _ecore_xcb_event_handle_circulate_notify(ev);
   else if (response == XCB_CIRCULATE_REQUEST)
     _ecore_xcb_event_handle_circulate_request(ev);
   else if (response == XCB_PROPERTY_NOTIFY)
     _ecore_xcb_event_handle_property_notify(ev);
   else if (response == XCB_SELECTION_CLEAR)
     _ecore_xcb_event_handle_selection_clear(ev);
   else if (response == XCB_SELECTION_REQUEST)
     _ecore_xcb_event_handle_selection_request(ev);
   else if (response == XCB_SELECTION_NOTIFY)
     _ecore_xcb_event_handle_selection_notify(ev);
   else if (response == XCB_COLORMAP_NOTIFY)
     _ecore_xcb_event_handle_colormap_notify(ev);
   else if (response == XCB_CLIENT_MESSAGE) 
     _ecore_xcb_event_handle_client_message(ev);
   else if (response == XCB_MAPPING_NOTIFY) 
     _ecore_xcb_event_handle_mapping_notify(ev);
   else if (response == 35) /* GenericEvent == 35 */
     _ecore_xcb_event_handle_generic_event(ev);
#ifdef ECORE_XCB_DAMAGE
   else if ((_ecore_xcb_event_damage >= 0) && 
            (response == (_ecore_xcb_event_damage + XCB_DAMAGE_NOTIFY)))
     _ecore_xcb_event_handle_damage_notify(ev);
#endif
#ifdef ECORE_XCB_RANDR
   else if ((_ecore_xcb_event_randr >= 0) && 
            (response == 
                _ecore_xcb_event_randr + XCB_RANDR_SCREEN_CHANGE_NOTIFY))
     _ecore_xcb_event_handle_randr_change(ev);
   else if ((_ecore_xcb_event_randr >= 0) && 
            (response == (_ecore_xcb_event_randr + XCB_RANDR_NOTIFY)))
     _ecore_xcb_event_handle_randr_notify(ev);
#endif
#ifdef ECORE_XCB_SCREENSAVER
   else if ((_ecore_xcb_event_screensaver >= 0) && 
            (response == 
                _ecore_xcb_event_screensaver + XCB_SCREENSAVER_NOTIFY))
     _ecore_xcb_event_handle_screensaver_notify(ev);
#endif
#ifdef ECORE_XCB_SHAPE
   else if ((_ecore_xcb_event_shape >= 0) && 
            (response == (_ecore_xcb_event_shape + XCB_SHAPE_NOTIFY)))
     _ecore_xcb_event_handle_shape_change(ev);
#endif
#ifdef ECORE_XCB_SYNC
   else if ((_ecore_xcb_event_sync >= 0) && 
            (response == (_ecore_xcb_event_sync + XCB_SYNC_COUNTER_NOTIFY)))
     _ecore_xcb_event_handle_sync_counter(ev);
   else if ((_ecore_xcb_event_sync >= 0) && 
            (response == (_ecore_xcb_event_sync + XCB_SYNC_ALARM_NOTIFY)))
     _ecore_xcb_event_handle_sync_alarm(ev);
#endif
#ifdef ECORE_XCB_XFIXES
   else if ((_ecore_xcb_event_xfixes >= 0) && 
            (response == 
                _ecore_xcb_event_xfixes + XCB_XFIXES_SELECTION_NOTIFY))
     _ecore_xcb_event_handle_xfixes_selection_notify(ev);
   else if ((_ecore_xcb_event_xfixes >= 0) && 
            (response == (_ecore_xcb_event_xfixes + XCB_XFIXES_CURSOR_NOTIFY)))
     _ecore_xcb_event_handle_xfixes_cursor_notify(ev);
#endif
}

Ecore_X_Time 
_ecore_xcb_events_last_time_get(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   return _ecore_xcb_event_last_time;
}

EAPI void 
ecore_x_event_mask_set(Ecore_X_Window win, Ecore_X_Event_Mask mask) 
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;

   list = (mask | reply->your_event_mask);
   free(reply);
   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);
//   ecore_x_flush();
}

EAPI void 
ecore_x_event_mask_unset(Ecore_X_Window win, Ecore_X_Event_Mask mask) 
{
   xcb_get_window_attributes_cookie_t cookie;
   xcb_get_window_attributes_reply_t *reply;
   uint32_t list;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!win) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   cookie = xcb_get_window_attributes_unchecked(_ecore_xcb_conn, win);
   reply = xcb_get_window_attributes_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;

   list = (reply->your_event_mask & ~mask);
   free(reply);
   xcb_change_window_attributes(_ecore_xcb_conn, win, 
                                XCB_CW_EVENT_MASK, &list);
//   ecore_x_flush();
}

unsigned int 
_ecore_xcb_events_modifiers_get(unsigned int state) 
{
   unsigned int modifiers = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (state & ECORE_X_MODIFIER_SHIFT)
     modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (state & ECORE_X_MODIFIER_CTRL)
     modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (state & ECORE_X_MODIFIER_ALT)
     modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (state & ECORE_X_MODIFIER_WIN)
     modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (state & ECORE_X_LOCK_SCROLL)
     modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (state & ECORE_X_LOCK_CAPS)
     modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (state & ECORE_X_LOCK_NUM)
     modifiers |= ECORE_EVENT_LOCK_NUM;
   if (state & ECORE_X_LOCK_SHIFT)
     modifiers |= ECORE_EVENT_LOCK_SHIFT;

   return modifiers;
}

/* local functions */
static void 
_ecore_xcb_event_handle_any_event(xcb_generic_event_t *event) 
{
   xcb_generic_event_t *ev;

//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ev = malloc(sizeof(xcb_generic_event_t));
   if (!ev) return;

   memcpy(ev, event, sizeof(xcb_generic_event_t));
   ecore_event_add(ECORE_X_EVENT_ANY, ev, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_key_press(xcb_generic_event_t *event) 
{
   _ecore_xcb_event_key_press(event);
}

static void 
_ecore_xcb_event_handle_key_release(xcb_generic_event_t *event) 
{
   _ecore_xcb_event_key_release(event);
}

static void 
_ecore_xcb_event_handle_button_press(xcb_generic_event_t *event) 
{
   xcb_button_press_event_t *ev;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;

   ev = (xcb_button_press_event_t *)event;
   if ((ev->detail > 3) && (ev->detail < 8)) 
     {
        Ecore_Event_Mouse_Wheel *e;

        if (!(e = malloc(sizeof(Ecore_Event_Mouse_Wheel)))) return;

        e->timestamp = ev->time;
        e->modifiers = _ecore_xcb_events_modifiers_get(ev->state);
        switch (ev->detail) 
          {
           case 4:
             e->direction = 0;
             e->z = -1;
             break;
           case 5:
             e->direction = 0;
             e->z = 1;
             break;
           case 6:
             e->direction = 1;
             e->z = -1;
             break;
           case 7:
             e->direction = 1;
             e->z = 1;
             break;
           default:
             e->direction = 0;
             e->z = 0;
             break;
          }
        e->x = ev->event_x;
        e->y = ev->event_y;
        e->root.x = ev->root_x;
        e->root.y = ev->root_y;
        if (ev->child)
          e->window = ev->child;
        else
          e->window = ev->event;

        e->event_window = ev->event;
        e->same_screen = ev->same_screen;
        e->root_window = ev->root;

        _ecore_xcb_event_last_time = e->timestamp;
        _ecore_xcb_event_last_window = e->window;
        _ecore_xcb_event_last_root_x = e->root.x;
        _ecore_xcb_event_last_root_y = e->root.y;

        ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);

        _ecore_xcb_window_grab_allow_events(ev->event, ev->child, 
                                            ECORE_EVENT_MOUSE_WHEEL, 
                                            e, ev->time);
     }
   else 
     {
        Ecore_Event_Mouse_Button *e;
        unsigned int child_win = 0;

        child_win = (ev->child ? ev->child : ev->event);

        _ecore_xcb_event_mouse_move(ev->time, ev->state, 
                                    ev->event_x, ev->event_y, 
                                    ev->root_x, ev->root_y, 
                                    ev->event, child_win, 
                                    ev->root, ev->same_screen, 
                                    0, 1, 1, 1.0, 0.0, 
                                    ev->event_x, ev->event_y, 
                                    ev->root_x, ev->root_y);

        e = _ecore_xcb_event_mouse_button(ECORE_EVENT_MOUSE_BUTTON_DOWN, 
                                          ev->time, 
                                          ev->state, ev->detail, 
                                          ev->event_x, ev->event_y, 
                                          ev->root_x, ev->root_y, ev->event, 
                                          child_win, 
                                          ev->root, ev->same_screen, 
                                          0, 1, 1, 1.0, 0.0, 
                                          ev->event_x, ev->event_y, 
                                          ev->root_x, ev->root_y);
        if (e)
          _ecore_xcb_window_grab_allow_events(ev->event, ev->child, 
                                              ECORE_EVENT_MOUSE_BUTTON_DOWN, 
                                              e, ev->time);
     }
}

static void 
_ecore_xcb_event_handle_button_release(xcb_generic_event_t *event) 
{
   xcb_button_release_event_t *ev;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_button_release_event_t *)event;
   if ((ev->detail <= 3) || (ev->detail > 7)) 
     {
        _ecore_xcb_event_mouse_move(ev->time, ev->state, 
                                    ev->event_x, ev->event_y, 
                                    ev->root_x, ev->root_y, 
                                    ev->event, 
                                    (ev->child ? ev->child : ev->event), 
                                    ev->root, ev->same_screen, 
                                    0, 1, 1, 1.0, 0.0, 
                                    ev->event_x, ev->event_y, 
                                    ev->root_x, ev->root_y);

        _ecore_xcb_event_mouse_button(ECORE_EVENT_MOUSE_BUTTON_UP, ev->time, 
                                      ev->state, ev->detail, 
                                      ev->event_x, ev->event_y, ev->root_x, 
                                      ev->root_y, ev->event, 
                                      (ev->child ? ev->child : ev->event), 
                                      ev->root, ev->same_screen, 
                                      0, 1, 1, 1.0, 0.0, 
                                      ev->event_x, ev->event_y, 
                                      ev->root_x, ev->root_y);
     }
}

static void 
_ecore_xcb_event_handle_motion_notify(xcb_generic_event_t *event) 
{
   xcb_motion_notify_event_t *ev;

   ev = (xcb_motion_notify_event_t *)event;

   /* if (_ecore_xcb_event_last_mouse_move_event)  */
   /*   { */
   /*      ecore_event_del(_ecore_xcb_event_last_mouse_move_event); */
   /*      _ecore_xcb_event_last_mouse_move = EINA_FALSE; */
   /*      _ecore_xcb_event_last_mouse_move_event = NULL; */
   /*   } */

   _ecore_xcb_event_mouse_move(ev->time, ev->state, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y, 
                               ev->event, 
                               (ev->child ? ev->child : ev->event), 
                               ev->root, ev->same_screen, 
                               0, 1, 1, 1.0, 0.0, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y);
   _ecore_xcb_event_last_mouse_move = EINA_TRUE;

   _ecore_xcb_dnd_drag(ev->root, ev->root_x, ev->root_y);
}

static void 
_ecore_xcb_event_handle_enter_notify(xcb_generic_event_t *event) 
{
   xcb_enter_notify_event_t *ev;
   Ecore_X_Event_Mouse_In *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_enter_notify_event_t *)event;

   _ecore_xcb_event_mouse_move(ev->time, ev->state, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y, 
                               ev->event, 
                               (ev->child ? ev->child : ev->event), 
                               ev->root, ev->same_screen_focus, 
                               0, 1, 1, 1.0, 0.0, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y);

   if (!(e = calloc(1, sizeof(Ecore_X_Event_Mouse_In)))) return;

   e->modifiers = _ecore_xcb_events_modifiers_get(ev->state);
   e->x = ev->event_x;
   e->y = ev->event_y;
   e->root.x = ev->root_x;
   e->root.y = ev->root_y;
   if (ev->child)
     e->win = ev->child;
   else
     e->win = ev->event;
   e->event_win = ev->event;
   e->same_screen = ev->same_screen_focus;
   e->root_win = ev->root;
   e->mode = _ecore_xcb_event_mode_get(ev->mode);
   e->detail = _ecore_xcb_event_detail_get(ev->detail);
   e->time = ev->time;
   _ecore_xcb_event_last_time = e->time;

   ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_leave_notify(xcb_generic_event_t *event) 
{
   xcb_leave_notify_event_t *ev;
   Ecore_X_Event_Mouse_Out *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_enter_notify_event_t *)event;

   _ecore_xcb_event_mouse_move(ev->time, ev->state, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y, 
                               ev->event, 
                               (ev->child ? ev->child : ev->event), 
                               ev->root, ev->same_screen_focus, 
                               0, 1, 1, 1.0, 0.0, 
                               ev->event_x, ev->event_y, 
                               ev->root_x, ev->root_y);

   if (!(e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out)))) return;

   e->modifiers = _ecore_xcb_events_modifiers_get(ev->state);
   e->x = ev->event_x;
   e->y = ev->event_y;
   e->root.x = ev->root_x;
   e->root.y = ev->root_y;
   if (ev->child)
     e->win = ev->child;
   else
     e->win = ev->event;
   e->event_win = ev->event;
   e->same_screen = ev->same_screen_focus;
   e->root_win = ev->root;
   e->mode = _ecore_xcb_event_mode_get(ev->mode);
   e->detail = _ecore_xcb_event_detail_get(ev->detail);

   e->time = ev->time;
   _ecore_xcb_event_last_time = e->time;
   _ecore_xcb_event_last_window = e->win;
   _ecore_xcb_event_last_root_x = e->root.x;
   _ecore_xcb_event_last_root_y = e->root.y;

   ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_keymap_notify(xcb_generic_event_t *event __UNUSED__) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   // FIXME: handle this event type
   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
}

static void 
_ecore_xcb_event_handle_focus_in(xcb_generic_event_t *event) 
{
   xcb_focus_in_event_t *ev;
   Ecore_X_Event_Window_Focus_In *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_focus_in_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In)))) return;

   e->win = ev->event;
   e->mode = _ecore_xcb_event_mode_get(ev->mode);
   e->detail = _ecore_xcb_event_detail_get(ev->detail);

   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_focus_out(xcb_generic_event_t *event) 
{
   xcb_focus_out_event_t *ev;
   Ecore_X_Event_Window_Focus_Out *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_focus_out_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out)))) return;

   e->win = ev->event;
   e->mode = _ecore_xcb_event_mode_get(ev->mode);
   e->detail = _ecore_xcb_event_detail_get(ev->detail);

   e->time = _ecore_xcb_event_last_time;
   _ecore_xcb_event_last_time = e->time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_expose(xcb_generic_event_t *event) 
{
   xcb_expose_event_t *ev;
   Ecore_X_Event_Window_Damage *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_expose_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Damage)))) return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;

   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_graphics_exposure(xcb_generic_event_t *event) 
{
   xcb_graphics_exposure_event_t *ev;
   Ecore_X_Event_Window_Damage *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_graphics_exposure_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Damage)))) return;

   e->win = ev->drawable;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->count = ev->count;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_visibility_notify(xcb_generic_event_t *event) 
{
   xcb_visibility_notify_event_t *ev;
   Ecore_X_Event_Window_Visibility_Change *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_visibility_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Visibility_Change))))
     return;

   e->win = ev->window;
   e->time = _ecore_xcb_event_last_time;
   if (ev->state == XCB_VISIBILITY_FULLY_OBSCURED)
     e->fully_obscured = 1;
   else
     e->fully_obscured = 0;

   ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_create_notify(xcb_generic_event_t *event) 
{
   xcb_create_notify_event_t *ev;
   Ecore_X_Event_Window_Create *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_create_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Create)))) return;

   e->win = ev->window;
   e->parent = ev->parent;
   if (ev->override_redirect)
     e->override = 1;
   else
     e->override = 0;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->border = ev->border_width;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_destroy_notify(xcb_generic_event_t *event) 
{
   xcb_destroy_notify_event_t *ev;
   Ecore_X_Event_Window_Destroy *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_destroy_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   if (e->win == _ecore_xcb_event_last_window)
     _ecore_xcb_event_last_window = 0;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_map_notify(xcb_generic_event_t *event) 
{
   xcb_map_notify_event_t *ev;
   Ecore_X_Event_Window_Show *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_map_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Show)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_unmap_notify(xcb_generic_event_t *event) 
{
   xcb_unmap_notify_event_t *ev;
   Ecore_X_Event_Window_Hide *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_unmap_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Hide)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_map_request(xcb_generic_event_t *event) 
{
   xcb_map_request_event_t *ev;
   Ecore_X_Event_Window_Show_Request *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_map_request_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request)))) return;

   e->win = ev->window;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_reparent_notify(xcb_generic_event_t *event) 
{
   xcb_reparent_notify_event_t *ev;
   Ecore_X_Event_Window_Reparent *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_reparent_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   e->parent = ev->parent;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_configure_notify(xcb_generic_event_t *event) 
{
   xcb_configure_notify_event_t *ev;
   Ecore_X_Event_Window_Configure *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_configure_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Configure)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   e->abovewin = ev->above_sibling;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->border = ev->border_width;
   e->override = ev->override_redirect;
   /* send_event is bit 7 (0x80) of response_type */
   e->from_wm = ((ev->response_type & 0x80) ? 1 : 0);
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_configure_request(xcb_generic_event_t *event) 
{
   xcb_configure_request_event_t *ev;
   Ecore_X_Event_Window_Configure_Request *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_configure_request_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Configure_Request)))) 
     return;

   e->win = ev->window;
   e->parent_win = ev->parent;
   e->abovewin = ev->sibling;
   e->x = ev->x;
   e->y = ev->y;
   e->w = ev->width;
   e->h = ev->height;
   e->border = ev->border_width;
   e->value_mask = ev->value_mask;
   switch (ev->stack_mode) 
     {
      case XCB_STACK_MODE_ABOVE:
        e->detail = ECORE_X_WINDOW_STACK_ABOVE;
        break;
      case XCB_STACK_MODE_BELOW:
        e->detail = ECORE_X_WINDOW_STACK_BELOW;
        break;
      case XCB_STACK_MODE_TOP_IF:
        e->detail = ECORE_X_WINDOW_STACK_TOP_IF;
        break;
      case XCB_STACK_MODE_BOTTOM_IF:
        e->detail = ECORE_X_WINDOW_STACK_BOTTOM_IF;
        break;
      case XCB_STACK_MODE_OPPOSITE:
        e->detail = ECORE_X_WINDOW_STACK_OPPOSITE;
        break;
     }
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_gravity_notify(xcb_generic_event_t *event __UNUSED__) 
{
/*
   xcb_gravity_notify_event_t *ev;
   Ecore_X_Event_Window_Gravity *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_gravity_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Gravity)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_GRAVITY, e, NULL, NULL);
*/
}

static void 
_ecore_xcb_event_handle_resize_request(xcb_generic_event_t *event) 
{
   xcb_resize_request_event_t *ev;
   Ecore_X_Event_Window_Resize_Request *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_resize_request_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Resize_Request)))) return;

   e->win = ev->window;
   e->w = ev->width;
   e->h = ev->height;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_RESIZE_REQUEST, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_circulate_notify(xcb_generic_event_t *event) 
{
   xcb_circulate_notify_event_t *ev;
   Ecore_X_Event_Window_Stack *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_circulate_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Stack)))) return;

   e->win = ev->window;
   e->event_win = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_circulate_request(xcb_generic_event_t *event) 
{
   xcb_circulate_request_event_t *ev;
   Ecore_X_Event_Window_Stack_Request *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_circulate_request_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Stack_Request)))) return;

   e->win = ev->window;
   e->parent = ev->event;
   if (ev->place == XCB_PLACE_ON_TOP)
     e->detail = ECORE_X_WINDOW_STACK_ABOVE;
   else
     e->detail = ECORE_X_WINDOW_STACK_BELOW;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_STACK_REQUEST, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_property_notify(xcb_generic_event_t *event) 
{
   xcb_property_notify_event_t *ev;
   Ecore_X_Event_Window_Property *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_property_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Property)))) return;

   e->win = ev->window;
   e->atom = ev->atom;
   e->time = ev->time;
   _ecore_xcb_event_last_time = e->time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_selection_clear(xcb_generic_event_t *event) 
{
   xcb_selection_clear_event_t *ev;
   Ecore_X_Event_Selection_Clear *e;
   Ecore_X_Atom sel;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_selection_clear_event_t *)event;
   if (!(e = malloc(sizeof(Ecore_X_Event_Selection_Clear)))) return;

   e->win = ev->owner;
   e->atom = sel = ev->selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (sel == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_X_SELECTION_OTHER;
   e->time = ev->time;

   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_selection_request(xcb_generic_event_t *event) 
{
   xcb_selection_request_event_t *ev;
   Ecore_X_Event_Selection_Request *e;
   Ecore_X_Selection_Intern *sd;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_selection_request_event_t *)event;
   if (!(e = malloc(sizeof(Ecore_X_Event_Selection_Request)))) return;

   e->owner = ev->owner;
   e->requestor = ev->requestor;
   e->selection = ev->selection;
   e->target = ev->target;
   e->property = ev->property;
   e->time = ev->time;

   ecore_event_add(ECORE_X_EVENT_SELECTION_REQUEST, e, NULL, NULL);

   if ((sd = _ecore_xcb_selection_get(ev->selection)) && 
       (sd->win == ev->owner)) 
     {
        Ecore_X_Selection_Intern *si;

        si = _ecore_xcb_selection_get(ev->selection);
        if (si->data) 
          {
             Ecore_X_Atom property = XCB_NONE, type;
             void *data = NULL;
             int len = 0, typesize = 0;

             type = ev->target;
             typesize = 8;
             len = sd->length;

             if (!ecore_x_selection_convert(ev->selection, ev->target, 
                                            &data, &len, &type, &typesize))
               property = XCB_NONE;
             else if (data)
               {
                  ecore_x_window_prop_property_set(ev->requestor, ev->property, 
                                                   type, typesize, data, len);
                  property = ev->property;
                  free(data);
               }
             ecore_x_selection_notify_send(ev->requestor, ev->selection, 
                                           ev->target, property, ev->time);
          }
     }
}

static void 
_ecore_xcb_event_handle_selection_notify(xcb_generic_event_t *event) 
{
   xcb_selection_notify_event_t *ev;
   Ecore_X_Event_Selection_Notify *e;
   unsigned char *data = NULL;
   Ecore_X_Atom selection;
   int num = 0, format = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_selection_notify_event_t *)event;
   selection = ev->selection;
   if (ev->target == ECORE_X_ATOM_SELECTION_TARGETS) 
     {
        format = 
          ecore_x_window_prop_property_get(ev->requestor, ev->property, 
                                           XCB_ATOM_ATOM, 32, &data, &num);
        if (!format) return;
     }
   else 
     {
        format = 
          ecore_x_window_prop_property_get(ev->requestor, ev->property, 
                                           XCB_GET_PROPERTY_TYPE_ANY, 8, 
                                           &data, &num);
        if (!format) return;
     }

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   if (!e) return;
   e->win = ev->requestor;
   e->time = ev->time;
   e->atom = selection;
   e->target = _ecore_xcb_selection_target_get(ev->target);

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     e->selection = ECORE_X_SELECTION_XDND;
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_X_SELECTION_OTHER;

   e->data = _ecore_xcb_selection_parse(e->target, data, num, format);

   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e, 
                   _ecore_xcb_event_selection_notify_free, NULL);
}

static void 
_ecore_xcb_event_handle_colormap_notify(xcb_generic_event_t *event) 
{
   xcb_colormap_notify_event_t *ev;
   Ecore_X_Event_Window_Colormap *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_colormap_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Colormap)))) return;

   e->win = ev->window;
   e->cmap = ev->colormap;
   if (ev->state == XCB_COLORMAP_STATE_INSTALLED)
     e->installed = 1;
   else
     e->installed = 0;
   e->time = _ecore_xcb_event_last_time;

   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_client_message(xcb_generic_event_t *event) 
{
   xcb_client_message_event_t *ev;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_client_message_event_t *)event;

   /* Special client message event handling here. need to put LOTS of if */
   /* checks here and generate synthetic events per special message known */
   /* otherwise generate generic client message event. this would handle*/
   /* netwm, ICCCM, gnomewm, old kde and mwm hint client message protocols */

   if ((ev->type == ECORE_X_ATOM_WM_PROTOCOLS) && (ev->format == 32) && 
       (ev->data.data32[0] == ECORE_X_ATOM_WM_DELETE_WINDOW)) 
     {
        Ecore_X_Event_Window_Delete_Request *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Delete_Request))))
          return;
        e->win = ev->window;
        e->time = _ecore_xcb_event_last_time;
        ecore_event_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_WM_MOVERESIZE) && 
            (ev->format == 32) && (ev->data.data32[2] < 9)) 
     {
        Ecore_X_Event_Window_Move_Resize_Request *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Move_Resize_Request))))
          return;
        e->win = ev->window;
        e->x = ev->data.data32[0];
        e->y = ev->data.data32[1];
        e->direction = ev->data.data32[2];
        e->button = ev->data.data32[3];
        e->source = ev->data.data32[4];
        ecore_event_add(ECORE_X_EVENT_WINDOW_MOVE_RESIZE_REQUEST, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_ENTER) 
     {
        Ecore_X_Event_Xdnd_Enter *e;
        Ecore_X_DND_Target *target;

        DBG("Got Xdnd Enter Event");
        if (!(e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter)))) return;
        target = _ecore_xcb_dnd_target_get();
        target->state = ECORE_X_DND_TARGET_ENTERED;
        target->source = ev->data.data32[0];
        target->win = ev->window;
        target->version = (int)(ev->data.data32[1] >> 24);
        if (target->version > ECORE_X_DND_VERSION) 
          {
             WRN("DND: Requested version %d but we only support up to %d", 
                 target->version, ECORE_X_DND_VERSION);
             free(e);
             return;
          }
        if (ev->data.data32[1] & 0x1UL) 
          {
             unsigned char *data;
             Ecore_X_Atom *types;
             int num_ret = 0;

             if (!ecore_x_window_prop_property_get(target->source, 
                                                   ECORE_X_ATOM_XDND_TYPE_LIST, 
                                                   ECORE_X_ATOM_ATOM, 32, 
                                                   &data, &num_ret)) 
               {
                  WRN("DND: Could not fetch data type list from source window");
                  free(e);
                  return;
               }
             types = (Ecore_X_Atom *)data;
             e->types = calloc(num_ret, sizeof(char *));
             if (e->types) 
               {
                  int i = 0;

                  for (i = 0; i < num_ret; i++)
                    e->types[i] = ecore_x_atom_name_get(types[i]);
               }
             e->num_types = num_ret;
          }
        else 
          {
             int i = 0;

             e->types = calloc(3, sizeof(char *));
             if (e->types) 
               {
                  while ((i < 3) && (ev->data.data32[i + 2])) 
                    {
                       e->types[i] = 
                         ecore_x_atom_name_get(ev->data.data32[i + 2]);
                       i++;
                    }
               }
             e->num_types = i;
          }

        e->win = target->win;
        e->source = target->source;
        ecore_event_add(ECORE_X_EVENT_XDND_ENTER, e, 
                        _ecore_xcb_event_xdnd_enter_free, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_POSITION) 
     {
        Ecore_X_Event_Xdnd_Position *e;
        Ecore_X_DND_Target *target;

        DBG("Got Xdnd Position Event");
        target = _ecore_xcb_dnd_target_get();
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) || 
            (target->win != ev->window)) return;
        target->pos.x = ev->data.data32[2] >> 16;
        target->pos.y = ev->data.data32[2] & 0xFFFFUL;
        target->action = ev->data.data32[4];
        target->time = (target->version >= 1) ? 
          (Ecore_X_Time)ev->data.data32[3] : XCB_CURRENT_TIME;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Position));
        if (!e) return;
        e->win = target->win;
        e->source = target->source;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        e->action = target->action;
        ecore_event_add(ECORE_X_EVENT_XDND_POSITION, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_STATUS) 
     {
        Ecore_X_Event_Xdnd_Status *e;
        Ecore_X_DND_Source *source;

        DBG("Got Xdnd Status Event");
        source = _ecore_xcb_dnd_source_get();
        if ((source->win != ev->window) || 
            (source->dest != (Ecore_X_Window)ev->data.data32[0]))
          return;

        source->await_status = 0;
        source->will_accept = ev->data.data32[1] & 0x1UL;
        source->suppress = (ev->data.data32[1] & 0x2UL) ? 0 : 1;
        source->rectangle.x = ev->data.data32[2] >> 16;
        source->rectangle.y = ev->data.data32[2] & 0xFFFFUL;
        source->rectangle.width = ev->data.data32[3] >> 16;
        source->rectangle.height = ev->data.data32[3] & 0xFFFFUL;
        source->accepted_action = ev->data.data32[4];

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Status));
        if (!e) return;
        e->win = source->win;
        e->target = source->dest;
        e->will_accept = source->will_accept;
        e->rectangle.x = source->rectangle.x;
        e->rectangle.y = source->rectangle.y;
        e->rectangle.width = source->rectangle.width;
        e->rectangle.height = source->rectangle.height;
        e->action = source->accepted_action;

        ecore_event_add(ECORE_X_EVENT_XDND_STATUS, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_LEAVE) 
     {
        Ecore_X_Event_Xdnd_Leave *e;
        Ecore_X_DND_Target *target;

        DBG("Got Xdnd Leave Event");
        target = _ecore_xcb_dnd_target_get();
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) || 
            (target->win != ev->window)) 
          return;
        target->state = ECORE_X_DND_TARGET_IDLE;
        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Leave));
        if (!e) return;
        e->win = ev->window;
        e->source = (Ecore_X_Window)ev->data.data32[0];
        ecore_event_add(ECORE_X_EVENT_XDND_LEAVE, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_DROP) 
     {
        Ecore_X_Event_Xdnd_Drop *e;
        Ecore_X_DND_Target *target;

        DBG("Got Xdnd Drop Event");
        target = _ecore_xcb_dnd_target_get();
        if ((target->source != (Ecore_X_Window)ev->data.data32[0]) || 
            (target->win != ev->window))
          return;
        target->time = (target->version >= 1) ? 
          (Ecore_X_Time)ev->data.data32[2] : _ecore_xcb_event_last_time;

        e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Drop));
        if (!e) return;
        e->win = target->win;
        e->source = target->source;
        e->action = target->action;
        e->position.x = target->pos.x;
        e->position.y = target->pos.y;
        ecore_event_add(ECORE_X_EVENT_XDND_DROP, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_XDND_FINISHED) 
     {
        Ecore_X_Event_Xdnd_Finished *e;
        Ecore_X_DND_Source *source;
        Eina_Bool completed = EINA_TRUE;

        DBG("Got Xdnd Finished Event");
        source = _ecore_xcb_dnd_source_get();
        if ((source->win != ev->window) || 
            (source->dest != (Ecore_X_Window)ev->data.data32[0]))
          return;
        if ((source->version < 5) || (ev->data.data32[1] & 0x1UL)) 
          {
             ecore_x_selection_xdnd_clear();
             source->state = ECORE_X_DND_SOURCE_IDLE;
          }
        else if (source->version >= 5) 
          {
             completed = EINA_FALSE;
             source->state = ECORE_X_DND_SOURCE_CONVERTING;
             /* FIXME: Probably need to add a timer to switch back to idle 
              * and discard the selection data */
          }

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Finished))))
          return;
        e->win = source->win;
        e->target = source->dest;
        e->completed = completed;
        if (source->version >= 5) 
          {
             source->accepted_action = ev->data.data32[2];
             e->action = source->accepted_action;
          }
        else 
          {
             source->accepted_action = 0;
             e->action = source->action;
          }
        ecore_event_add(ECORE_X_EVENT_XDND_FINISHED, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_NET_WM_STATE) 
     {
        Ecore_X_Event_Window_State_Request *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request))))
          return;
        e->win = ev->window;
        if (ev->data.data32[0] == 0)
          e->action = ECORE_X_WINDOW_STATE_ACTION_REMOVE;
        else if (ev->data.data32[0] == 1)
          e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        else if (ev->data.data32[0] == 2)
          e->action = ECORE_X_WINDOW_STATE_ACTION_TOGGLE;
        else 
          {
             free(e);
             return;
          }
        e->state[0] = _ecore_xcb_netwm_window_state_get(ev->data.data32[1]);
        if (e->state[0] == ECORE_X_WINDOW_STATE_UNKNOWN) 
          {
             /* FIXME */
          }
        e->state[1] = _ecore_xcb_netwm_window_state_get(ev->data.data32[2]);
        if (e->state[1] == ECORE_X_WINDOW_STATE_UNKNOWN) 
          {
             /* FIXME */
          }
        e->source = ev->data.data32[3];
        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
#ifdef OLD_XCB_VERSION
   else if ((ev->type == ECORE_X_ATOM_WM_CHANGE_STATE) && 
            (ev->format == 32) && (ev->data.data32[0] == XCB_WM_STATE_ICONIC)) 
#else
   else if ((ev->type == ECORE_X_ATOM_WM_CHANGE_STATE) && (ev->format == 32) && 
            (ev->data.data32[0] == XCB_ICCCM_WM_STATE_ICONIC)) 
#endif
     {
        Ecore_X_Event_Window_State_Request *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_State_Request))))
          return;
        e->win = ev->window;
        e->action = ECORE_X_WINDOW_STATE_ACTION_ADD;
        e->state[0] = ECORE_X_WINDOW_STATE_ICONIFIED;
        ecore_event_add(ECORE_X_EVENT_WINDOW_STATE_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_WM_DESKTOP) && (ev->format == 32)) 
     {
        Ecore_X_Event_Desktop_Change *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Desktop_Change))))
          return;
        e->win = ev->window;
        e->desk = ev->data.data32[0];
        e->source = ev->data.data32[1];
        ecore_event_add(ECORE_X_EVENT_DESKTOP_CHANGE, e, NULL, NULL);
     }
   else if (ev->type == ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS) 
     {
        Ecore_X_Event_Frame_Extents_Request *e;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Frame_Extents_Request))))
          return;
        e->win = ev->window;
        ecore_event_add(ECORE_X_EVENT_FRAME_EXTENTS_REQUEST, e, NULL, NULL);
     }
   else if ((ev->type == ECORE_X_ATOM_WM_PROTOCOLS) && 
            ((Ecore_X_Atom)ev->data.data32[0] == ECORE_X_ATOM_NET_WM_PING) && 
            (ev->format == 32)) 
     {
        Ecore_X_Event_Ping *e;
        Ecore_X_Window root = 0;
        int count = 0;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Ping)))) return;
        e->win = ev->window;
        e->time = ev->data.data32[1];
        e->event_win = ev->data.data32[2];
        ecore_event_add(ECORE_X_EVENT_PING, e, NULL, NULL);

        count = xcb_setup_roots_length(xcb_get_setup(_ecore_xcb_conn));
        if (count > 1) 
          root = ecore_x_window_root_get(e->win);
        else 
          root = ((xcb_screen_t *)_ecore_xcb_screen)->root;

        if (ev->window != root) 
          {
             ev->window = root;
             xcb_send_event(_ecore_xcb_conn, 0, root, 
                            (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
                                XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY), 
                            (const char *)&ev);
//             ecore_x_flush();
          }
     }
   else if ((ev->type == ECORE_X_ATOM_NET_STARTUP_INFO_BEGIN) && 
            (ev->format == 8)) 
     {
        _ecore_xcb_netwm_startup_info_begin(ev->window, ev->data.data8[0]);
     }
   else if ((ev->type == ECORE_X_ATOM_NET_STARTUP_INFO) && (ev->format == 8)) 
     {
        _ecore_xcb_netwm_startup_info(ev->window, ev->data.data8[0]);
     }
   else if ((ev->type == 27777) && (ev->data.data32[0] == 0x7162534) && 
            (ev->format == 32)) // && (ev->window = _private_window)) 
     {
        if (ev->data.data32[1] == 0x10000001)
          _ecore_xcb_window_button_grab_remove(ev->data.data32[2]);
        else if (ev->data.data32[1] == 0x10000002) 
          _ecore_xcb_window_key_grab_remove(ev->data.data32[2]);
     }
   else 
     {
        Ecore_X_Event_Client_Message *e;
        int i = 0;

        if (!(e = calloc(1, sizeof(Ecore_X_Event_Client_Message))))
          return;

        e->win = ev->window;
        e->message_type = ev->type;
        e->format = ev->format;
        for (i = 0; i < 5; i++)
          e->data.l[i] = ev->data.data32[i];
        ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}

static void 
_ecore_xcb_event_handle_mapping_notify(xcb_generic_event_t *event) 
{
   xcb_mapping_notify_event_t *ev;
   Ecore_X_Event_Mapping_Change *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;

   ev = (xcb_mapping_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Mapping_Change)))) return;

   _ecore_xcb_keymap_refresh(ev);

   switch (ev->request) 
     {
      case XCB_MAPPING_MODIFIER:
        e->type = ECORE_X_MAPPING_MODIFIER;
        break;
      case XCB_MAPPING_KEYBOARD:
        e->type = ECORE_X_MAPPING_KEYBOARD;
        break;
      case XCB_MAPPING_POINTER:
      default:
        e->type = ECORE_X_MAPPING_MOUSE;
        break;
     }
   e->keycode = ev->first_keycode;
   e->num = ev->count;

   ecore_event_add(ECORE_X_EVENT_MAPPING_CHANGE, e, NULL, NULL);
}

static void 
_ecore_xcb_event_handle_damage_notify(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_DAMAGE
   xcb_damage_notify_event_t *ev;
   Ecore_X_Event_Damage *e;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_DAMAGE
   ev = (xcb_damage_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Damage)))) return;

   e->level = ev->level;
   e->drawable = ev->drawable;
   e->damage = ev->damage;
   e->time = ev->timestamp;
   e->area.x = ev->area.x;
   e->area.y = ev->area.y;
   e->area.width = ev->area.width;
   e->area.height = ev->area.height;
   e->geometry.x = ev->geometry.x;
   e->geometry.y = ev->geometry.y;
   e->geometry.width = ev->geometry.width;
   e->geometry.height = ev->geometry.height;

   ecore_event_add(ECORE_X_EVENT_DAMAGE_NOTIFY, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_randr_change(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_screen_change_notify_event_t *ev;
   Ecore_X_Event_Screen_Change *e;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_RANDR
   ev = (xcb_randr_screen_change_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Screen_Change)))) return;

   e->win = ev->request_window;
   e->root = ev->root;
   e->size.width = ev->width;
   e->size.height = ev->height;
   e->time = ev->timestamp;
   e->config_time = ev->config_timestamp;
   e->size.width_mm = ev->mwidth;
   e->size.height_mm = ev->mheight;
   e->orientation = ev->rotation;
   e->subpixel_order = ev->subpixel_order;

   ecore_event_add(ECORE_X_EVENT_SCREEN_CHANGE, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_randr_notify(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_notify_event_t *ev;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_RANDR
   ev = (xcb_randr_notify_event_t *)event;
   switch (ev->subCode) 
     {
      case XCB_RANDR_NOTIFY_CRTC_CHANGE:
        _ecore_xcb_event_handle_randr_crtc_change(event);
        break;
      case XCB_RANDR_NOTIFY_OUTPUT_CHANGE:
        _ecore_xcb_event_handle_randr_output_change(event);
        break;
      case XCB_RANDR_NOTIFY_OUTPUT_PROPERTY:
        _ecore_xcb_event_handle_randr_output_property_change(event);
        break;
      default:
        break;
     }
#endif
}

static void 
_ecore_xcb_event_handle_randr_crtc_change(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_notify_event_t *ev;
   Ecore_X_Event_Randr_Crtc_Change *e;
#endif

#ifdef ECORE_XCB_RANDR
   ev = (xcb_randr_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Randr_Crtc_Change))))
     return;

   e->win = ev->u.cc.window;
   e->crtc = ev->u.cc.crtc;
   e->mode = ev->u.cc.mode;
   e->orientation = ev->u.cc.rotation;
   e->geo.x = ev->u.cc.x;
   e->geo.y = ev->u.cc.y;
   e->geo.w = ev->u.cc.width;
   e->geo.h = ev->u.cc.height;

   ecore_event_add(ECORE_X_EVENT_RANDR_CRTC_CHANGE, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_randr_output_change(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_notify_event_t *ev;
   Ecore_X_Event_Randr_Output_Change *e;
#endif

#ifdef ECORE_XCB_RANDR
   ev = (xcb_randr_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Randr_Crtc_Change))))
     return;

   e->win = ev->u.oc.window;
   e->output = ev->u.oc.output;
   e->crtc = ev->u.oc.crtc;
   e->mode = ev->u.oc.mode;
   e->orientation = ev->u.oc.rotation;
   e->connection = ev->u.oc.connection;
   e->subpixel_order = ev->u.oc.subpixel_order;

   ecore_event_add(ECORE_X_EVENT_RANDR_OUTPUT_CHANGE, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_randr_output_property_change(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_notify_event_t *ev;
   Ecore_X_Event_Randr_Output_Property_Notify *e;
#endif

#ifdef ECORE_XCB_RANDR
   ev = (xcb_randr_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Randr_Output_Property_Notify))))
     return;

   e->win = ev->u.op.window;
   e->output = ev->u.op.output;
   e->property = ev->u.op.atom;
   e->time = ev->u.op.timestamp;
   if (ev->u.op.status == XCB_PROPERTY_NEW_VALUE)
     e->state = ECORE_X_RANDR_PROPERTY_CHANGE_ADD;
   else
     e->state = ECORE_X_RANDR_PROPERTY_CHANGE_DEL;

   ecore_event_add(ECORE_X_EVENT_RANDR_OUTPUT_PROPERTY_NOTIFY, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_screensaver_notify(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_notify_event_t *ev;
   Ecore_X_Event_Screensaver_Notify *e;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_SCREENSAVER
   ev = (xcb_screensaver_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Screensaver_Notify)))) return;

   e->win = ev->window;
   e->on = EINA_FALSE;
   if (ev->state == XCB_SCREENSAVER_STATE_ON) e->on = EINA_TRUE;
   e->time = ev->time;

   ecore_event_add(ECORE_X_EVENT_SCREENSAVER_NOTIFY, e, NULL, NULL);
#endif
}

#ifdef ECORE_XCB_SHAPE
static void 
_ecore_xcb_event_handle_shape_change(xcb_generic_event_t *event) 
{
   xcb_shape_notify_event_t *ev;
   Ecore_X_Event_Window_Shape *e;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
   ev = (xcb_shape_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Window_Shape)))) return;

   e->win = ev->affected_window;
   e->time = ev->server_time;
   switch (ev->shape_kind) 
     {
      case XCB_SHAPE_SK_BOUNDING:
        e->type = ECORE_X_SHAPE_BOUNDING;
        break;
      case XCB_SHAPE_SK_CLIP:
        e->type = ECORE_X_SHAPE_CLIP;
        break;
      case XCB_SHAPE_SK_INPUT:
        e->type = ECORE_X_SHAPE_INPUT;
        break;
      default:
        break;
     }
   e->x = ev->extents_x;
   e->y = ev->extents_y;
   e->w = ev->extents_width;
   e->h = ev->extents_height;
   e->shaped = ev->shaped;

   ecore_event_add(ECORE_X_EVENT_WINDOW_SHAPE, e, NULL, NULL);
}
#endif

static void 
_ecore_xcb_event_handle_sync_counter(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_counter_notify_event_t *ev;
   Ecore_X_Event_Sync_Counter *e;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;

#ifdef ECORE_XCB_SYNC
   ev = (xcb_sync_counter_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Sync_Counter)))) return;

   e->time = ev->timestamp;

   ecore_event_add(ECORE_X_EVENT_SYNC_COUNTER, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_sync_alarm(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_SYNC
   xcb_sync_alarm_notify_event_t *ev;
   Ecore_X_Event_Sync_Alarm *e;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_SYNC
   ev = (xcb_sync_alarm_notify_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Sync_Alarm)))) return;

   e->time = ev->timestamp;
   e->alarm = ev->alarm;

   ecore_event_add(ECORE_X_EVENT_SYNC_ALARM, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_xfixes_selection_notify(xcb_generic_event_t *event) 
{
#ifdef ECORE_XCB_XFIXES
   Ecore_X_Event_Fixes_Selection_Notify *e;
   Ecore_X_Atom sel;
   xcb_xfixes_selection_notify_event_t *ev;
#endif

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;
#ifdef ECORE_XCB_XFIXES
   ev = (xcb_xfixes_selection_notify_event_t *)event;

   if (!(e = calloc(1, sizeof(*e)))) return;

   e->win = ev->window;
   e->owner = ev->owner;
   e->time = ev->timestamp;
   e->selection_time = ev->selection_timestamp;
   e->atom = sel = ev->selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (sel == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     e->selection = ECORE_X_SELECTION_OTHER;
   e->reason = ev->subtype;

   ecore_event_add(ECORE_X_EVENT_FIXES_SELECTION_NOTIFY, e, NULL, NULL);
#endif
}

static void 
_ecore_xcb_event_handle_xfixes_cursor_notify(xcb_generic_event_t *event __UNUSED__) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
//  FIXME: TBD
}

static void 
_ecore_xcb_event_handle_generic_event(xcb_generic_event_t *event) 
{
   xcb_ge_event_t *ev;
   Ecore_X_Event_Generic *e;

   ev = (xcb_ge_event_t *)event;
   if (!(e = calloc(1, sizeof(Ecore_X_Event_Generic))))
     return;

   DBG("Handle Generic Event: %d", ev->event_type);

   e->cookie = ev->sequence;
   /* NB: These are bugs in xcb ge_event structure. The struct should have a 
    * field for extension & data, but does not. 
    * 
    * XCB people have been notified of this issue */
   e->extension = ev->pad0;
   /* e->data = ev->pad1; */

   e->evtype = ev->event_type;

   if (e->extension == _ecore_xcb_event_input)
     _ecore_xcb_event_handle_input_event(event);

   ecore_event_add(ECORE_X_EVENT_GENERIC, e, 
                   _ecore_xcb_event_generic_event_free, event);
}

static void 
_ecore_xcb_event_handle_input_event(xcb_generic_event_t *event) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_xcb_input_handle_event(event);
}

static void 
_ecore_xcb_event_key_press(xcb_generic_event_t *event) 
{
   Ecore_Event_Key *e;
   xcb_keysym_t sym = XCB_NO_SYMBOL;
   xcb_keycode_t keycode = 0;
   xcb_key_press_event_t *xevent;
   char *keyname = NULL, *key = NULL;
   char *compose = NULL;
   char compose_buffer[256];
   int val = 0;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;

   xevent = (xcb_key_press_event_t *)event;
   keycode = xevent->detail;

   sym = _ecore_xcb_keymap_keycode_to_keysym(keycode, xevent->state);
   keyname = _ecore_xcb_keymap_keysym_to_string(sym);
   if (!keyname) 
     {
        char buff[256];

        snprintf(buff, sizeof(buff), "Keycode-%i", keycode);
        keyname = buff;
     }

   val = 
     _ecore_xcb_keymap_lookup_string(keycode, xevent->state, compose_buffer, 
                                     sizeof(compose_buffer), &sym);
   if (val > 0) 
     {
        compose_buffer[val] = 0;
        compose = 
          eina_str_convert(nl_langinfo(CODESET), "UTF-8", compose_buffer);
//        tmp = compose;
     }

   key = _ecore_xcb_keymap_keysym_to_string(sym);
   if (!key) key = keyname;

   e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + 
              (compose ? strlen(compose) : 0) + 3);
   if (e) 
     {
        e->keyname = (char *)(e + 1);
        e->key = e->keyname + strlen(keyname) + 1;

        e->compose = NULL;
        if (compose) e->compose = (e->key + strlen(key) + 1);
        e->string = e->compose;

        strcpy((char *)e->keyname, keyname);
        strcpy((char *)e->key, key);
        if (compose) strcpy((char *)e->compose, compose);

        e->modifiers = _ecore_xcb_events_modifiers_get(xevent->state);
        e->timestamp = xevent->time;
        e->window = xevent->child ? xevent->child : xevent->event;
        e->event_window = xevent->event;
        e->same_screen = xevent->same_screen;
        e->root_window = xevent->root;

        DBG("Sending Key Down Event: %s", e->keyname);
        ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
     }
   _ecore_xcb_event_last_time = xevent->time;
}

static void 
_ecore_xcb_event_key_release(xcb_generic_event_t *event) 
{
   Ecore_Event_Key *e;
   xcb_keysym_t sym = XCB_NO_SYMBOL;
   xcb_keycode_t keycode = 0;
   xcb_key_release_event_t *xevent;
   char *keyname = NULL, *key = NULL;
   char *compose = NULL;
   char compose_buffer[256];
   int val = 0;

   _ecore_xcb_event_last_mouse_move = EINA_FALSE;

   xevent = (xcb_key_release_event_t *)event;
   keycode = xevent->detail;

   sym = _ecore_xcb_keymap_keycode_to_keysym(keycode, xevent->state);
   keyname = _ecore_xcb_keymap_keysym_to_string(sym);
   if (!keyname) 
     {
        char buff[256];

        snprintf(buff, sizeof(buff), "Keycode-%i", keycode);
        keyname = buff;
     }

   val = 
     _ecore_xcb_keymap_lookup_string(keycode, xevent->state, compose_buffer, 
                                     sizeof(compose_buffer), &sym);
   if (val > 0) 
     {
        compose_buffer[val] = 0;
        compose = 
          eina_str_convert(nl_langinfo(CODESET), "UTF-8", compose_buffer);
//        tmp = compose;
     }

   key = _ecore_xcb_keymap_keysym_to_string(sym);
   if (!key) key = keyname;

   e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + 
              (compose ? strlen(compose) : 0) + 3);
   if (e) 
     {
        e->keyname = (char *)(e + 1);
        e->key = e->keyname + strlen(keyname) + 1;

        e->compose = NULL;
        if (compose) e->compose = (e->key + strlen(key) + 1);
        e->string = e->compose;

        strcpy((char *)e->keyname, keyname);
        strcpy((char *)e->key, key);
        if (compose) strcpy((char *)e->compose, compose);

        e->modifiers = _ecore_xcb_events_modifiers_get(xevent->state);
        e->timestamp = xevent->time;
        e->window = xevent->child ? xevent->child : xevent->event;
        e->event_window = xevent->event;
        e->same_screen = xevent->same_screen;
        e->root_window = xevent->root;

        ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);
     }
   _ecore_xcb_event_last_time = xevent->time;
}

void 
_ecore_xcb_event_mouse_move(uint16_t timestamp, uint16_t modifiers, int16_t x, int16_t y, int16_t root_x, int16_t root_y, xcb_window_t event_win, xcb_window_t win, xcb_window_t root_win, uint8_t same_screen, int dev, double radx, double rady, double pressure, double angle, int16_t mx, int16_t my, int16_t mrx, int16_t mry) 
{
   Ecore_Event_Mouse_Move *e;
   Ecore_Event *event;

   if (!(e = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   e->window = win;
   e->root_window = root_win;
   e->timestamp = timestamp;
   e->same_screen = same_screen;
   e->event_window = event_win;
   e->modifiers = _ecore_xcb_events_modifiers_get(modifiers);
   e->x = x;
   e->y = y;
   e->root.x = root_x;
   e->root.y = root_y;
   e->multi.device = dev;
   e->multi.radius = ((radx + rady) / 2);
   e->multi.radius_x = radx;
   e->multi.radius_y = rady;
   e->multi.pressure = pressure;
   e->multi.angle = angle;
   e->multi.x = mx;
   e->multi.y = my;
   e->multi.root.x = mrx;
   e->multi.root.y = mry;

   event = ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, 
                           _ecore_xcb_event_mouse_move_free, NULL);

   _ecore_xcb_event_last_time = e->timestamp;
   _ecore_xcb_event_last_window = e->window;
   _ecore_xcb_event_last_root_x = root_x;
   _ecore_xcb_event_last_root_y = root_y;
//   _ecore_xcb_event_last_mouse_move_event = event;
}

static void 
_ecore_xcb_event_mouse_move_free(void *data __UNUSED__, void *event) 
{
   Ecore_Event_Mouse_Move *ev;

   ev = event;
//   if (_ecore_xcb_event_last_mouse_move_event) 
//     {
//        _ecore_xcb_event_last_mouse_move = EINA_FALSE;
//        _ecore_xcb_event_last_mouse_move_event = NULL;
//     }
   if (ev) free(ev);
}

Ecore_Event_Mouse_Button *
_ecore_xcb_event_mouse_button(int event, uint16_t timestamp, uint16_t modifiers, xcb_button_t buttons, int16_t x, int16_t y, int16_t root_x, int16_t root_y, xcb_window_t event_win, xcb_window_t win, xcb_window_t root_win, uint8_t same_screen, int dev, double radx, double rady, double pressure, double angle, int16_t mx, int16_t my, int16_t mrx, int16_t mry) 
{
   Ecore_Event_Mouse_Button *e;
   Ecore_X_Mouse_Down_Info *info = NULL;

   if (!(e = malloc(sizeof(Ecore_Event_Mouse_Button)))) return NULL;

   e->window = win;
   e->root_window = root_win;
   e->timestamp = timestamp;
   e->same_screen = same_screen;
   e->event_window = event_win;
   e->buttons = buttons;
   e->modifiers = _ecore_xcb_events_modifiers_get(modifiers);
   e->double_click = 0;
   e->triple_click = 0;
   e->x = x;
   e->y = y;
   e->root.x = root_x;
   e->root.y = root_y;

   if ((info = _ecore_xcb_event_mouse_down_info_get(dev))) 
     {
        if ((event == ECORE_EVENT_MOUSE_BUTTON_DOWN) && 
            (info->did_triple))
          {
             info->last_win = 0;
             info->last_last_win = 0;
             info->last_event_win = 0;
             info->last_time = 0;
             info->last_last_time = 0;
          }
        if (event_win == win) 
          {
             if (event == ECORE_EVENT_MOUSE_BUTTON_DOWN) 
               {
                  if (((int)(timestamp - info->last_time) <= 
                       (int)(1000 * _ecore_xcb_double_click_time)) && 
                      (win == info->last_win) && 
                      (event_win == info->last_event_win)) 
                    {
                       e->double_click = 1;
                       info->did_double = EINA_TRUE;
                    }
                  else 
                    {
                       info->did_double = EINA_FALSE;
                       info->did_triple = EINA_FALSE;
                    }
                  if (((int)(timestamp - info->last_last_time) <= 
                       (int)(2 * 1000 * _ecore_xcb_double_click_time)) && 
                      (win == info->last_win) && 
                      (win == info->last_last_win) && 
                      (event_win == info->last_event_win) && 
                      (event_win == info->last_last_event_win))
                    {
                       e->triple_click = 1;
                       info->did_triple = EINA_TRUE;
                    }
                  else 
                    info->did_triple = EINA_FALSE;
               }
             else 
               {
                  if (info->did_double) e->double_click = 1;
                  if (info->did_triple) e->triple_click = 1;
               }
          }
     }

   /* NB: Comment out right now because _ecore_xcb_mouse_up_count is
    * only used here...nowhere else in the code */

   /* if ((event == ECORE_EVENT_MOUSE_BUTTON_DOWN) &&  */
   /*     (!e->double_click) && (!e->triple_click)) */
   /*   _ecore_xcb_mouse_up_count = 0; */

   e->multi.device = dev;
   e->multi.radius = ((radx + rady) / 2);
   e->multi.radius_x = radx;
   e->multi.radius_y = rady;
   e->multi.pressure = pressure;
   e->multi.angle = angle;
   e->multi.x = mx;
   e->multi.y = my;
   e->multi.root.x = mrx;
   e->multi.root.y = mry;

   _ecore_xcb_event_last_time = e->timestamp;
   _ecore_xcb_event_last_window = e->window;
   _ecore_xcb_event_last_root_x = root_x;
   _ecore_xcb_event_last_root_y = root_y;

   ecore_event_add(event, e, NULL, NULL);

   if ((info) && (event == ECORE_EVENT_MOUSE_BUTTON_DOWN) && 
       (win == event_win) && (!info->did_triple)) 
     {
        info->last_last_win = info->last_win;
        info->last_win = win;
        info->last_last_event_win = info->last_event_win;
        info->last_event_win = event_win;
        info->last_last_time = info->last_time;
        info->last_time = timestamp;
     }

   return e;
}

static Ecore_X_Event_Mode 
_ecore_xcb_event_mode_get(uint8_t mode) 
{
   switch (mode) 
     {
      case XCB_NOTIFY_MODE_NORMAL:
        return ECORE_X_EVENT_MODE_NORMAL;
      case XCB_NOTIFY_MODE_WHILE_GRABBED:
        return ECORE_X_EVENT_MODE_WHILE_GRABBED;
      case XCB_NOTIFY_MODE_GRAB:
        return ECORE_X_EVENT_MODE_GRAB;
      case XCB_NOTIFY_MODE_UNGRAB:
        return ECORE_X_EVENT_MODE_UNGRAB;
      default:
        return ECORE_X_EVENT_MODE_NORMAL;
     }
}

static Ecore_X_Event_Detail 
_ecore_xcb_event_detail_get(uint8_t detail) 
{
   switch (detail) 
     {
      case XCB_NOTIFY_DETAIL_ANCESTOR:
        return ECORE_X_EVENT_DETAIL_ANCESTOR;
      case XCB_NOTIFY_DETAIL_VIRTUAL:
        return ECORE_X_EVENT_DETAIL_VIRTUAL;
      case XCB_NOTIFY_DETAIL_INFERIOR:
        return ECORE_X_EVENT_DETAIL_INFERIOR;
      case XCB_NOTIFY_DETAIL_NONLINEAR:
        return ECORE_X_EVENT_DETAIL_NON_LINEAR;
      case XCB_NOTIFY_DETAIL_NONLINEAR_VIRTUAL:
        return ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
      case XCB_NOTIFY_DETAIL_POINTER:
        return ECORE_X_EVENT_DETAIL_POINTER;
      case XCB_NOTIFY_DETAIL_POINTER_ROOT:
        return ECORE_X_EVENT_DETAIL_POINTER_ROOT;
      case XCB_NOTIFY_DETAIL_NONE:
      default:
        return ECORE_X_EVENT_DETAIL_ANCESTOR;
     }
}

static void 
_ecore_xcb_event_xdnd_enter_free(void *data __UNUSED__, void *event) 
{
   Ecore_X_Event_Xdnd_Enter *e;
   int i = 0;

   e = event;
   for (i = 0; i < e->num_types; i++)
     free(e->types[i]);
   free(e->types);
   free(e);
}

static void 
_ecore_xcb_event_selection_notify_free(void *data __UNUSED__, void *event) 
{
   Ecore_X_Event_Selection_Notify *e;
   Ecore_X_Selection_Data *sel;

   e = event;
   if (!(sel = e->data)) return;
   if (sel->free) sel->free(sel);
   free(e->target);
   free(e);
}

static void 
_ecore_xcb_event_generic_event_free(void *data, void *event) 
{
   Ecore_X_Event_Generic *e;

   e = (Ecore_X_Event_Generic *)event;
   if (e->data) free(data);
   free(e);
}

static void 
_ecore_xcb_event_mouse_down_info_clear(void) 
{
   Eina_Inlist *l;
   Ecore_X_Mouse_Down_Info *info = NULL;

   l = _ecore_xcb_mouse_down_info_list;
   while (l) 
     {
        info = EINA_INLIST_CONTAINER_GET(l, Ecore_X_Mouse_Down_Info);
        l = eina_inlist_remove(l, l);
        free(info);
     }
   _ecore_xcb_mouse_down_info_list = NULL;
}

static Ecore_X_Mouse_Down_Info *
_ecore_xcb_event_mouse_down_info_get(int dev) 
{
   Eina_Inlist *l;
   Ecore_X_Mouse_Down_Info *info = NULL;

   l = _ecore_xcb_mouse_down_info_list;
   EINA_INLIST_FOREACH(l, info)
     if (info->dev == dev) return info;

   if (!(info = calloc(1, sizeof(Ecore_X_Mouse_Down_Info)))) return NULL;

   info->dev = dev;
   l = eina_inlist_append(l, (Eina_Inlist *)info);
   _ecore_xcb_mouse_down_info_list = l;

   return info;
}
