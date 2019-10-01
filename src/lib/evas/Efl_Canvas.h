#ifndef _EFL_CANVAS_H
#define _EFL_CANVAS_H

#include <Efl_Config.h>

#include <Eina.h>

#include <Eo.h>
/* This include has been added to support Eo in Evas */
#include <Efl.h>
#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <interfaces/efl_gfx_types.eot.h>
#include <interfaces/efl_gfx_path.eo.h>
#include <interfaces/efl_input_types.eot.h>
#include <interfaces/efl_text_types.eot.h>
#include <canvas/efl_canvas_animation_types.eot.h>
#include <gesture/efl_canvas_gesture_types.eot.h>

#include <gesture/efl_canvas_gesture.eo.h>
#include <gesture/efl_canvas_gesture_touch.eo.h>
#include <gesture/efl_canvas_gesture_recognizer.eo.h>
#include <gesture/efl_canvas_gesture_manager.eo.h>
#include <canvas/efl_canvas_object.eo.h>

#include <canvas/efl_canvas_animation_alpha.eo.h>
#include <canvas/efl_canvas_animation.eo.h>
#include <canvas/efl_canvas_animation_group.eo.h>
#include <canvas/efl_canvas_animation_group_parallel.eo.h>
#include <canvas/efl_canvas_animation_group_sequential.eo.h>
#include <canvas/efl_canvas_animation_player.eo.h>
#include <canvas/efl_canvas_animation_rotate.eo.h>
#include <canvas/efl_canvas_animation_scale.eo.h>
#include <canvas/efl_canvas_animation_translate.eo.h>
#include <canvas/efl_canvas_event_grabber.eo.h>
#include <canvas/efl_canvas_group.eo.h>
#include <canvas/efl_canvas_image.eo.h>
#include <canvas/efl_canvas_image_internal.eo.h>
#include <canvas/efl_canvas_polygon.eo.h>
#include <canvas/efl_canvas_proxy.eo.h>
#include <canvas/efl_canvas_rectangle.eo.h>
/* FIXME: this uses EVAS types in its API and is broken.
#include <canvas/efl_canvas_scene3d.eo.h>
*/
#include <canvas/efl_canvas_snapshot.eo.h>
#include <canvas/efl_canvas_text.eo.h>
#include <canvas/efl_canvas_text_factory.eo.h>
#include <canvas/efl_canvas_vg_node.eo.h>
#include <canvas/efl_canvas_vg_container.eo.h>
#include <canvas/efl_canvas_vg_gradient.eo.h>
#include <canvas/efl_canvas_vg_gradient_linear.eo.h>
#include <canvas/efl_canvas_vg_gradient_radial.eo.h>
#include <canvas/efl_canvas_vg_image.eo.h>
#include <canvas/efl_canvas_vg_object.eo.h>
#include <canvas/efl_canvas_vg_shape.eo.h>
#include <canvas/efl_gfx_mapping.eo.h>

#include <canvas/efl_input_clickable.eo.h>
#include <canvas/efl_input_event.eo.h>
#include <canvas/efl_input_focus.eo.h>
#include <canvas/efl_input_hold.eo.h>
#include <canvas/efl_input_interface.eo.h>
#include <canvas/efl_input_key.eo.h>
#include <canvas/efl_input_pointer.eo.h>
#include <canvas/efl_input_state.eo.h>

#include "canvas/efl2_text_cursor.eo.h"
#include "canvas/efl2_text_attribute_factory.eo.h"
#include "canvas/efl2_text_item_factory.eo.h"
#include "canvas/efl2_canvas_text.eo.h"

#include <gesture/efl_canvas_gesture_double_tap.eo.h>
#include <gesture/efl_canvas_gesture_flick.eo.h>
#include <gesture/efl_canvas_gesture_long_tap.eo.h>
#include <gesture/efl_canvas_gesture_momentum.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_double_tap.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_flick.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_long_tap.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_momentum.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_tap.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_triple_tap.eo.h>
#include <gesture/efl_canvas_gesture_recognizer_zoom.eo.h>
#include <gesture/efl_canvas_gesture_tap.eo.h>
#include <gesture/efl_canvas_gesture_triple_tap.eo.h>
#include <gesture/efl_canvas_gesture_zoom.eo.h>
#include <gesture/efl_gesture_events.eo.h>


#ifdef __cplusplus
}
#endif
#undef EAPI
#endif
