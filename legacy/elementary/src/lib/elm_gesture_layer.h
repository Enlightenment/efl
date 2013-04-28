/**
 * @defgroup Elm_Gesture_Layer Gesture Layer
 * @ingroup Elementary
 *
 * @image html gesture_layer_inheritance_tree.png
 * @image latex gesture_layer_inheritance_tree.eps
 *
 * Gesture Layer Usage:
 *
 * Use Gesture Layer to detect gestures.
 * The advantage is that you don't have to implement
 * gesture detection, just set callbacks of gesture state.
 * By using gesture layer we make standard interface.
 *
 * In order to use Gesture Layer you start with @ref elm_gesture_layer_add
 * with a parent object parameter.
 * Next 'activate' gesture layer with a @ref elm_gesture_layer_attach
 * call. Usually with same object as target (2nd parameter).
 *
 * Now you need to tell gesture layer what gestures you follow.
 * This is done with @ref elm_gesture_layer_cb_set call.
 * By setting the callback you actually saying to gesture layer:
 * I would like to know when the gesture @ref Elm_Gesture_Type
 * switches to state @ref Elm_Gesture_State.
 *
 * Next, you need to implement the actual action that follows the input
 * in your callback.
 *
 * Note that if you like to stop being reported about a gesture, just set
 * all callbacks referring this gesture to NULL.
 * (again with @ref elm_gesture_layer_cb_set)
 *
 * The information reported by gesture layer to your callback is depending
 * on @ref Elm_Gesture_Type :
 * @ref Elm_Gesture_Taps_Info is the info reported for tap gestures:
 * @ref ELM_GESTURE_N_TAPS, @ref ELM_GESTURE_N_LONG_TAPS,
 * @ref ELM_GESTURE_N_DOUBLE_TAPS, @ref ELM_GESTURE_N_TRIPLE_TAPS.
 *
 * @ref Elm_Gesture_Momentum_Info is info reported for momentum gestures:
 * @ref ELM_GESTURE_MOMENTUM.
 *
 * @ref Elm_Gesture_Line_Info is the info reported for line gestures:
 * (this also contains @ref Elm_Gesture_Momentum_Info internal structure)
 * @ref ELM_GESTURE_N_LINES, @ref ELM_GESTURE_N_FLICKS.
 * Note that we consider a flick as a line-gesture that should be completed
 * in flick-time-limit as defined in @ref Config.
 *
 * @ref Elm_Gesture_Zoom_Info is the info reported for @ref ELM_GESTURE_ZOOM gesture.
 *
 * @ref Elm_Gesture_Rotate_Info is the info reported for @ref ELM_GESTURE_ROTATE gesture.
 *
 *
 * Gesture Layer Tweaks:
 *
 * Note that line, flick, gestures can start without the need to remove fingers from surface.
 * When user fingers rests on same-spot gesture is ended and starts again when fingers moved.
 *
 * Setting glayer_continues_enable to false in @ref Config will change this behavior
 * so gesture starts when user touches (a *DOWN event) touch-surface
 * and ends when no fingers touches surface (a *UP event).
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * @{
 *
 */

#include "elm_gesture_layer_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_gesture_layer_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_gesture_layer_legacy.h"
#endif
/**
 * @}
 */
