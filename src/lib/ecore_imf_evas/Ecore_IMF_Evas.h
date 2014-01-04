#ifndef _ECORE_IMF_EVAS_H
#define _ECORE_IMF_EVAS_H

#include <Ecore_IMF.h>
#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_IMF_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_IMF_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/**
 * @defgroup Ecore_IMF_Evas_Group Ecore Input Method Context Evas Helper Functions
 * @ingroup Ecore_IMF_Lib_Group
 *
 * Helper functions to make it easy to use Evas with Ecore_IMF.
 * Converts each event from Evas to the corresponding event of Ecore_IMF.
 *
 * An example of usage of these functions can be found at:
 * @li @ref ecore_imf_example_c
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts a "mouse_in" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_in_wrap(Evas_Event_Mouse_In *evas_event, Ecore_IMF_Event_Mouse_In *imf_event);

/**
 * Converts a "mouse_out" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_out_wrap(Evas_Event_Mouse_Out *evas_event, Ecore_IMF_Event_Mouse_Out *imf_event);

/**
 * Converts a "mouse_move" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_move_wrap(Evas_Event_Mouse_Move *evas_event, Ecore_IMF_Event_Mouse_Move *imf_event);

/**
 * Converts a "mouse_down" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_down_wrap(Evas_Event_Mouse_Down *evas_event, Ecore_IMF_Event_Mouse_Down *imf_event);

/**
 * Converts a "mouse_up" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_up_wrap(Evas_Event_Mouse_Up *evas_event, Ecore_IMF_Event_Mouse_Up *imf_event);

/**
 * Converts a "mouse_wheel" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 */
EAPI void ecore_imf_evas_event_mouse_wheel_wrap(Evas_Event_Mouse_Wheel *evas_event, Ecore_IMF_Event_Mouse_Wheel *imf_event);

/**
 * Converts a "key_down" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 *
 * Example
 * @code
 * static void
 * _key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
 * {
 *    Evas_Event_Key_Down *ev = event_info;
 *    if (!ev->key) return;
 *
 *    if (imf_context)
 *      {
 *         Ecore_IMF_Event_Key_Down ecore_ev;
 *         ecore_imf_evas_event_key_down_wrap(ev, &ecore_ev);
 *         if (ecore_imf_context_filter_event(imf_context,
 *                                            ECORE_IMF_EVENT_KEY_DOWN,
 *                                            (Ecore_IMF_Event *)&ecore_ev))
 *           return;
 *      }
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, data);
 * @endcode
 */
EAPI void ecore_imf_evas_event_key_down_wrap(Evas_Event_Key_Down *evas_event, Ecore_IMF_Event_Key_Down *imf_event);

/**
 * Converts a "key_up" event from Evas to the corresponding event of Ecore_IMF.
 *
 * @param evas_event The received Evas event.
 * @param imf_event The location to store the converted Ecore_IMF event.
 * @ingroup Ecore_IMF_Evas_Group
 *
 * Example
 * @code
 * static void
 * _key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
 * {
 *    Evas_Event_Key_Up *ev = event_info;
 *    if (!ev->key) return;
 *
 *    if (imf_context)
 *      {
 *         Ecore_IMF_Event_Key_Up ecore_ev;
 *         ecore_imf_evas_event_key_up_wrap(ev, &ecore_ev);
 *         if (ecore_imf_context_filter_event(imf_context,
 *                                            ECORE_IMF_EVENT_KEY_UP,
 *                                            (Ecore_IMF_Event *)&ecore_ev))
 *           return;
 *      }
 * }
 *
 * evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_UP, _key_up_cb, data);
 * @endcode
 */
EAPI void ecore_imf_evas_event_key_up_wrap(Evas_Event_Key_Up *evas_event, Ecore_IMF_Event_Key_Up *imf_event);

#ifdef __cplusplus
}
#endif

#endif
