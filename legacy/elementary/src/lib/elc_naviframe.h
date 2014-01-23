/**
 * @defgroup Naviframe Naviframe
 * @ingroup Elementary
 *
 * @image html naviframe_inheritance_tree.png
 * @image latex naviframe_inheritance_tree.eps
 *
 * @brief Naviframe stands for navigation frame. It's a views manager
 * for applications.
 *
 * A naviframe holds views (or pages) as its items. Those items are
 * organized in a stack, so that new items get pushed on top of the
 * old, and only the topmost view is displayed at one time. Due to the
 * characteristics of a stack, even though you push a new item, previous item
 * is not deleted. Previous item will be shown when you pop new item. The
 * transition between views is animated, depending on the theme applied to the
 * widget.
 *
 * Naviframe views hold spaces to various elements, which are:
 * - back button, used to navigate to previous views,
 * - next button, used to navigate to next views in the stack,
 * - title label,
 * - sub-title label,
 * - title icon and
 * - content area.
 *
 * One can use @ref elm_object_item_part_content_set,
 * @ref elm_object_item_part_content_get,
 * @ref elm_object_item_part_content_unset functions to handle the contents.
 * The swallow part name should be one of these:
 * @li @c "default" - The main content of the current page
 * @li @c "icon" - An icon in the title area of the current page
 * @li @c "prev_btn" - A button of the current page to go to the
 *                     previous page
 * @li @c "next_btn" - A button of the current page to go to the next
 *                     page
 *
 * One can use @ref elm_object_item_part_text_set,
 * @ref elm_object_item_part_text_get to handle the text parts.
 * The swallow part name should be one of these:
 * @li @c "default" - Title label in the title area of the current
 *                    page
 * @li @c "subtitle" - Sub-title label in the title area of the
 *                     current page
 *
 * Most of those content objects can be passed at the time of an item
 * creation (see elm_naviframe_item_push()).
 *
 * Naviframe items can have different styles, which affect the
 * transition between views, for example. On the default theme, two of
 * them are supported:
 * - @c "basic"   - views are switched sliding horizontally, one after
 *                  the other
 * - @c "overlap" - like the previous one, but the previous view stays
 *                  at its place and is overlapped by the new
 *
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "transition,finished" - When the transition is finished in
 *                                changing the item
 * @li @c "title,transition,finished" - When the title area's transition
 *                                      is finished in changing the state
 *                                      of the title
 * @li @c "title,clicked" - User clicked title area
 * @li @c "focused" - When the naviframe has received focus. (since 1.8)
 * @li @c "unfocused" - When the naviframe has lost focus. (since 1.8)
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * All the parts, for content and text, described here will also be
 * reachable by naviframe @b items direct calls:
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_part_content_unset
 * @li @ref elm_object_item_signal_emit
 *
 * What happens is that the topmost item of a naviframe will be the
 * widget's target layout, when accessed directly. Items lying below
 * the top one can be interacted with this way.
 *
 * Here is an example on its usage:
 * @li @ref naviframe_example
 */

/**
 * @addtogroup Naviframe
 * @{
 */

#include "elc_naviframe_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elc_naviframe_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_naviframe_legacy.h"
#endif

/**
 * @}
 */
