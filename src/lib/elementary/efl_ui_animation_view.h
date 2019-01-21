/**
 * @defgroup Elm_Animation_View Animation_View
 * @ingroup Elementary
 *
 * Animation_View widget is designed to show and play animation of
 * vector graphics based content. It hides all efl_canvas_vg details
 * but just open an API to read vector data from file. Also, it implements
 * details of animation control methods of Vector.
 *
 * Vector data could contain static or animatable vector elements including
 * animation infomation. Currently approved vector data file format is svg, json and eet.
 * Only json(known for Lottie file as well) and eet could contains animation infomation,
 * currently Animation_View is supporting.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Animation_View:
 * @li "play,start": animation is just started.
 * @li "play,repeat": animation is just repeated.
 * @li "play,done": animation is just finished.
 * @li "play,pause": animation is just paused.
 * @li "play,resume": animation is just resumed.
 * @li "play,stop": animation is just stopped.
 * @li "play,update": animation is updated to the next frame.
 *
 */

/**
 * @ingroup Elm_Animation_View
 */

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "efl_ui_animation_view_legacy.h"
#endif
/**
 * @}
 */
