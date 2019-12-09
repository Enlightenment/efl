#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

/*
 * @brief Query whether gesture is available or not.
 *
 * @return @c EINA_TRUE, if extension is available, @c EINA_FALSE otherwise.
 *
 * @deprecated
 */
EAPI Eina_Bool
ecore_x_gesture_supported(void)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_gesture_events_select(Ecore_X_Window win,
                              Ecore_X_Gesture_Event_Mask mask)
{
   return EINA_FALSE;
}

EAPI Ecore_X_Gesture_Event_Mask
ecore_x_gesture_events_selected_get(Ecore_X_Window win)
{
   return ECORE_X_GESTURE_EVENT_MASK_NONE;
}

EAPI Eina_Bool
ecore_x_gesture_event_grab(Ecore_X_Window win,
                           Ecore_X_Gesture_Event_Type type,
                           int num_fingers)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_gesture_event_ungrab(Ecore_X_Window win,
                             Ecore_X_Gesture_Event_Type type,
                             int num_fingers)
{
   return EINA_FALSE;
}
