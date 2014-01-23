/**
 * @defgroup Progressbar Progress bar
 * @ingroup Elementary
 *
 * @image html progressbar_inheritance_tree.png
 * @image latex progressbar_inheritance_tree.eps
 *
 * The progress bar is a widget for visually representing the
 * progress status of a given job/task.
 *
 * A progress bar may be horizontal or vertical. It may display an
 * icon besides it, as well as primary and @b units labels. The
 * former is meant to label the widget as a whole, while the
 * latter, which is formatted with floating point values (and thus
 * accepts a <c>printf</c>-style format string, like <c>"%1.2f
 * units"</c>), is meant to label the widget's <b>progress
 * value</b>. Label, icon and unit strings/objects are @b optional
 * for progress bars.
 *
 * A progress bar may be @b inverted, in which case it gets its
 * values inverted, i.e., high values being on the left or top and
 * low values on the right or bottom, for horizontal and vertical modes
 * respectively.
 *
 * The @b span of the progress, as set by
 * elm_progressbar_span_size_set(), is its length (horizontally or
 * vertically), unless one puts size hints on the widget to expand
 * on desired directions, by any container. That length will be
 * scaled by the object or applications scaling factor.
 * Applications can query the progress bar for its value with
 * elm_progressbar_value_get().
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for progress bar objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "changed" - when the value is changed (since 1.7)
 * @li @c "focused" - When the progressbar has received focus. (since 1.8)
 * @li @c "unfocused" - When the progressbar has lost focus. (since 1.8)
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * This widget has the following styles:
 * - @c "default"
 * - @c "wheel" (simple style, no text, no progression, only
 *      "pulse" effect is available)
 *
 * Default text parts of the progressbar widget that you can use for are:
 * @li "default" - Label of the progressbar
 *
 * Default content parts of the progressbar widget that you can use for are:
 * @li "icon" - An icon of the progressbar
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Here is an example on its usage:
 * @li @ref progressbar_example
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_progressbar_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_progressbar_legacy.h"
#endif
/**
 * @}
 */
