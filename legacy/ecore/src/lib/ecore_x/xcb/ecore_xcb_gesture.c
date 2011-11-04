#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_XGESTURE
# include <xcb/gesture.h>
# include <xcb/xcb_event.h>
#endif

/* local variables */
static Eina_Bool _gesture_available = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_gesture = -1;

void 
_ecore_xcb_gesture_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XGESTURE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_gesture_id);
#endif
}

void 
_ecore_xcb_gesture_finalize(void) 
{
#ifdef ECORE_XCB_XGESTURE
   xcb_gesture_query_version_cookie_t cookie;
   xcb_gesture_query_version_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XGESTURE
   cookie = 
     xcb_gesture_query_version_unchecked(_ecore_xcb_conn);
   reply = 
     xcb_gesture_query_version_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply) 
     {
        _gesture_available = EINA_TRUE;
        free(reply);
     }

   if (_gesture_available) 
     {
        const xcb_query_extension_reply_t *ext_reply;

        ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_gesture_id);
        if (ext_reply) 
          _ecore_xcb_event_gesture = ext_reply->first_event;
     }
#endif
}

void 
_ecore_xcb_gesture_shutdown(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

EAPI Eina_Bool
ecore_x_gesture_supported(void)
{
   return _gesture_available;
}

EAPI Eina_Bool
ecore_x_gesture_events_select(Ecore_X_Window win,
                              Ecore_X_Gesture_Event_Mask mask)
{
#ifdef ECORE_XCB_XGESTURE
   if (!_gesture_available) return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN

   xcb_gesture_select_events(_ecore_xcb_conn, win, mask);

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI Ecore_X_Gesture_Event_Mask
ecore_x_gesture_events_selected_get(Ecore_X_Window win)
{
#ifdef ECORE_XCB_XGESTURE
   xcb_gesture_get_selected_events_cookie_t ecookie;
   xcb_gesture_get_selected_events_reply_t *ereply;
   Ecore_X_Gesture_Event_Mask mask = ECORE_X_GESTURE_EVENT_MASK_NONE;

   if (!_gesture_available) return mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN

   ecookie = xcb_gesture_get_selected_events(_ecore_xcb_conn, win);
   ereply = 
     xcb_gesture_get_selected_events_reply(_ecore_xcb_conn, ecookie, NULL);
   if (ereply)
     {
        mask = ereply->mask;
	 free(ereply);
     }

   return mask;
#else
   return ECORE_X_GESTURE_EVENT_MASK_NONE;
#endif
}

EAPI Eina_Bool
ecore_x_gesture_event_grab(Ecore_X_Window win,
                           Ecore_X_Gesture_Event_Type type,
                           int num_fingers)
{
#ifdef ECORE_XCB_XGESTURE
   Eina_Bool status = EINA_TRUE;
   xcb_gesture_grab_event_cookie_t ecookie;
   xcb_gesture_grab_event_reply_t *ereply;

   if (!_gesture_available) return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN

   ecookie = 
     xcb_gesture_grab_event(_ecore_xcb_conn, win, type, num_fingers, 0L);
   ereply = xcb_gesture_grab_event_reply(_ecore_xcb_conn, ecookie, NULL);

   if (ereply)
     {
        if (ereply->status) status = EINA_FALSE;
        free(ereply);
     }
   else
     status = EINA_FALSE;

   return status;
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
ecore_x_gesture_event_ungrab(Ecore_X_Window win,
                             Ecore_X_Gesture_Event_Type type,
                             int num_fingers)
{
#ifdef ECORE_XCB_XGESTURE
   Eina_Bool status = EINA_TRUE;
   xcb_gesture_ungrab_event_cookie_t ecookie;
   xcb_gesture_ungrab_event_reply_t *ereply;

   if (!_gesture_available) return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN

  ecookie = 
     xcb_gesture_ungrab_event(_ecore_xcb_conn, win, type, num_fingers, 0L);
  ereply = xcb_gesture_ungrab_event_reply(_ecore_xcb_conn, ecookie, NULL);

   if (ereply)
     {
        if (ereply->status) status = EINA_FALSE;
        free(ereply);
     }
   else
     status = EINA_FALSE;

   return status;
#else
   return EINA_FALSE;
#endif
}
