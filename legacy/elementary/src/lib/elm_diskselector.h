/**
 * @defgroup Diskselector Diskselector
 * @ingroup Elementary
 *
 * @image html diskselector_inheritance_tree.png
 * @image latex diskselector_inheritance_tree.eps
 *
 * @image html img/widget/diskselector/preview-00.png
 * @image latex img/widget/diskselector/preview-00.eps
 *
 * A diskselector is a kind of list widget. It scrolls horizontally,
 * and can contain label and icon objects. Three items are displayed
 * with the selected one in the middle.
 *
 * It can act like a circular list with round mode and labels can be
 * reduced for a defined length for side items.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for diskselectors.
 *
 * Some calls on the diskselector's API are marked as @b deprecated,
 * as they just wrap the scrollable widgets counterpart functions. Use
 * the ones we point you to, for each case of deprecation here,
 * instead -- eventually the deprecated ones will be discarded (next
 * major release).
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "selected" - when item is selected, i.e. scroller stops.
 * @li @c "clicked" - This is called when a user clicks an item (since 1.8)
 * @li @c "scroll,anim,start" - scrolling animation has started
 * @li @c "scroll,anim,stop" - scrolling animation has stopped
 * @li @c "scroll,drag,start" - dragging the diskselector has started
 * @li @c "scroll,drag,stop" - dragging the diskselector has stopped
 * @li @c "focused" - When the diskselector has received focus. (since 1.8)
 * @li @c "unfocused" - When the diskselector has lost focus. (since 1.8)
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * @note The "scroll,anim,*" and "scroll,drag,*" signals are only emitted by
 * user intervention.
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the diskselector items that you can use for are:
 * @li "icon" - An icon in the diskselector item
 *
 * Default text parts of the diskselector items that you can use for are:
 * @li "default" - Label of the diskselector item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 *
 * List of examples:
 * @li @ref diskselector_example_01
 * @li @ref diskselector_example_02
 */

/**
 * @addtogroup Diskselector
 * @{
 */

#include "elm_diskselector_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_diskselector_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_diskselector_legacy.h"
#endif
/**
 * @}
 */
