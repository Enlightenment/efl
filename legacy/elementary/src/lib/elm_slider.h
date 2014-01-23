/**
 * @defgroup Slider Slider
 * @ingroup Elementary
 *
 * @image html slider_inheritance_tree.png
 * @image latex slider_inheritance_tree.eps
 *
 * @image html img/widget/slider/preview-00.png
 * @image latex img/widget/slider/preview-00.eps width=\textwidth
 *
 * The slider adds a draggable “slider” widget for selecting the value of
 * something within a range.
 *
 * A slider can be horizontal or vertical. It can contain an Icon and has a
 * primary label as well as a units label (that is formatted with floating
 * point values and thus accepts a printf-style format string, like
 * “%1.2f units”. There is also an indicator string that may be somewhere
 * else (like on the slider itself) that also accepts a format string like
 * units. Label, Icon Unit and Indicator strings/objects are optional.
 *
 * A slider may be inverted which means values invert, with high vales being
 * on the left or top and low values on the right or bottom (as opposed to
 * normally being low on the left or top and high on the bottom and right).
 *
 * The slider should have its minimum and maximum values set by the
 * application with  elm_slider_min_max_set() and value should also be set by
 * the application before use with  elm_slider_value_set(). The span of the
 * slider is its length (horizontally or vertically). This will be scaled by
 * the object or applications scaling factor. At any point code can query the
 * slider for its value with elm_slider_value_get().
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for slider objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - Whenever the slider value is changed by the user.
 * - @c "slider,drag,start" - dragging the slider indicator around has started.
 * - @c "slider,drag,stop" - dragging the slider indicator around has stopped.
 * - @c "delay,changed" - A short time after the value is changed by the user.
 * This will be called only when the user stops dragging for
 * a very short period or when they release their
 * finger/mouse, so it avoids possibly expensive reactions to
 * the value change.
 * - @c "focused" - When the slider has received focus. (since 1.8)
 * - @c "unfocused" - When the slider has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the slider widget that you can use for are:
 * @li "icon" - An icon of the slider
 * @li "end" - A end part content of the slider
 *
 * Default text parts of the slider widget that you can use for are:
 * @li "default" - Label of the slider
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Here is an example on its usage:
 * @li @ref slider_example
 */

/**
 * @addtogroup Slider
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_slider_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_slider_legacy.h"
#endif
/**
 * @}
 */
