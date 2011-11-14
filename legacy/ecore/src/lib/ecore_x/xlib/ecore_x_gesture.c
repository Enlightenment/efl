#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

static Eina_Bool _gesture_available = EINA_FALSE;

#ifdef ECORE_XGESTURE
static int _gesture_major, _gesture_minor, _gesture_patch;
int _gesture_version;
#endif /* ifdef ECORE_XGESTURE */

void
_ecore_x_gesture_init(void)
{
#ifdef ECORE_XGESTURE
   _gesture_major = 0;
   _gesture_minor = 0;
   _gesture_patch = 0;
   _gesture_version = 0;

   if (XGestureQueryVersion(_ecore_x_disp, &_gesture_major, &_gesture_minor, &_gesture_patch))
     {
        _gesture_version = (_gesture_major << 16) | _gesture_minor;
        _gesture_available = EINA_TRUE;
     }
   else
      _gesture_available = EINA_FALSE;
#else /* ifdef ECORE_XGESTURE */
   _gesture_available = EINA_FALSE;
#endif /* ifdef ECORE_XGESTURE */
}

/*
 * @brief query whether gesture is available or not
 * @return EINA_TRUE, if extension is available, else EINA_FALSE
 */
EAPI Eina_Bool
ecore_x_gesture_supported(void)
{
   return _gesture_available;
}

EAPI Eina_Bool
ecore_x_gesture_events_select(Ecore_X_Window win,
                              Ecore_X_Gesture_Event_Mask mask)
{
#ifdef ECORE_XGESTURE
   if (!_gesture_available)
   	return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGestureSelectEvents(_ecore_x_disp, win, mask);
   
   return EINA_TRUE;
#else /* ifdef ECORE_XGESTURE */
   return EINA_FALSE;
   win = 0;
   mask = 0;
#endif /* ifdef ECORE_XGESTURE */
}

EAPI Ecore_X_Gesture_Event_Mask
ecore_x_gesture_events_selected_get(Ecore_X_Window win)
{
#ifdef ECORE_XGESTURE
   Ecore_X_Gesture_Event_Mask mask;

   if (!_gesture_available)
   	return ECORE_X_GESTURE_EVENT_MASK_NONE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (GestureSuccess != XGestureGetSelectedEvents(_ecore_x_disp, win, &mask))
     {
        mask = ECORE_X_GESTURE_EVENT_MASK_NONE;
        return mask;
     }
   
   return mask;
#else /* ifdef ECORE_XGESTURE */
   return ECORE_X_GESTURE_EVENT_MASK_NONE;
   win = 0;
#endif /* ifdef ECORE_XGESTURE */
}

EAPI Eina_Bool
ecore_x_gesture_event_grab(Ecore_X_Window win,
                           Ecore_X_Gesture_Event_Type type,
                           int num_fingers)
{
#ifdef ECORE_XGESTURE
   if (!_gesture_available)
   	return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (GestureGrabSuccess != XGestureGrabEvent(_ecore_x_disp, win, type, num_fingers, CurrentTime))
     {
        return EINA_FALSE;
     }
   
   return EINA_TRUE;
#else /* ifdef ECORE_XGESTURE */
   return EINA_FALSE;
   win = 0;
   type = 0;
   num_fingers = 0;
#endif /* ifdef ECORE_XGESTURE */
}

EAPI Eina_Bool
ecore_x_gesture_event_ungrab(Ecore_X_Window win,
                             Ecore_X_Gesture_Event_Type type,
                             int num_fingers)
{
#ifdef ECORE_XGESTURE
   Ecore_X_Gesture_Event_Mask mask;

   if (!_gesture_available)
   	return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (GestureUngrabSuccess != XGestureUngrabEvent(_ecore_x_disp, win, type, num_fingers, CurrentTime))
     {
        return EINA_FALSE;
     }
   
   return EINA_TRUE;
#else /* ifdef ECORE_XGESTURE */
   return EINA_FALSE;
   win = 0;
   type = 0;
   num_fingers = 0;
#endif /* ifdef ECORE_XGESTURE */
}

