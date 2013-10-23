/**
 * @defgroup Spinner Spinner
 * @ingroup Elementary
 *
 * @image html spinner_inheritance_tree.png
 * @image latex spinner_inheritance_tree.eps
 *
 * @image html img/widget/spinner/preview-00.png
 * @image latex img/widget/spinner/preview-00.eps
 *
 * A spinner is a widget which allows the user to increase or decrease
 * numeric values using arrow buttons, or edit values directly, clicking
 * over it and typing the new value.
 *
 * By default the spinner will not wrap and has a label
 * of "%.0f" (just showing the integer value of the double).
 *
 * A spinner has a label that is formatted with floating
 * point values and thus accepts a printf-style format string, like
 * “%1.2f units”.
 *
 * It also allows specific values to be replaced by pre-defined labels.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for spinner objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - Whenever the spinner value is changed.
 * - @c "delay,changed" - A short time after the value is changed by
 *    the user.  This will be called only when the user stops dragging
 *    for a very short period or when they release their finger/mouse,
 *    so it avoids possibly expensive reactions to the value change.
 * - @c "language,changed" - the program's language changed
 * - @c "focused" - When the spinner has received focus. (since 1.8)
 * - @c "unfocused" - When the spinner has lost focus. (since 1.8)
 * - @c "spinner,drag,start" - When dragging has started. (since 1.8)
 * - @c "spinner,drag,stop" - When dragging has stopped. (since 1.8)
 *
 * Available styles for it:
 * - @c "default";
 * - @c "vertical": up/down buttons at the right side and text left aligned.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * Here is an example on its usage:
 * @ref spinner_example
 */

/**
 * @addtogroup Spinner
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_spinner_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_spinner_legacy.h"
#endif
/**
 * @}
 */
