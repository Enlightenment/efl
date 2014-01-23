/**
 * @defgroup Flipselector Flip Selector
 * @ingroup Elementary
 *
 * @image html flipselector_inheritance_tree.png
 * @image latex flipselector_inheritance_tree.eps
 *
 * @image html img/widget/flipselector/preview-00.png
 * @image latex img/widget/flipselector/preview-00.eps
 *
 * A flip selector is a widget to show a set of @b text items, one
 * at a time, with the same sheet switching style as the @ref Clock
 * "clock" widget, when one changes the current displaying sheet
 * (thus, the "flip" in the name).
 *
 * User clicks to flip sheets which are @b held for some time will
 * make the flip selector to flip continuously and automatically for
 * the user. The interval between flips will keep growing in time,
 * so that it helps the user to reach an item which is distant from
 * the current selection.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for flip selector objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "selected" - when the widget's selected text item is changed. The @c
 *   event_info parameter is the item that was selected.
 * - @c "overflowed" - when the widget's current selection is changed
 *   from the first item in its list to the last
 * - @c "underflowed" - when the widget's current selection is changed
 *   from the last item in its list to the first
 * - @c "focused" - When the flip selector has received focus. (since 1.8)
 * - @c "unfocused" - When the flip selector has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default text parts of the flipselector items that you can use for are:
 * @li "default" - label of the flipselector item
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_signal_emit
 *
 * Here is an example on its usage:
 * @li @ref flipselector_example
 */

/**
 * @addtogroup Flipselector
 * @{
 */

#include "elm_flipselector_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_flipselector_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_flipselector_legacy.h"
#endif
/**
 * @}
 */
