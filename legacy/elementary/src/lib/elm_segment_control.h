/**
 * @defgroup SegmentControl SegmentControl
 * @ingroup Elementary
 *
 * @image html segment_control_inheritance_tree.png
 * @image latex segment_control_inheritance_tree.eps
 *
 * @image html img/widget/segment_control/preview-00.png
 * @image latex img/widget/segment_control/preview-00.eps width=\textwidth
 *
 * @image html img/segment_control.png
 * @image latex img/segment_control.eps width=\textwidth
 *
 * Segment control widget is a horizontal control made of multiple segment
 * items, each segment item functioning similar to discrete two state button.
 * A segment control groups the items together and provides compact
 * single button with multiple equal size segments.
 *
 * Segment item size is determined by base widget
 * size and the number of items added.
 * Only one segment item can be at selected state. A segment item can display
 * combination of Text and any Evas_Object like Images or other widget.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for segment control objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - When the user clicks on a segment item which is not
 *   previously selected and get selected. The event_info parameter is the
 *   segment item pointer.
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the segment control items that you can use for are:
 * @li "icon" - An icon in a segment control item
 *
 * Default text parts of the segment control items that you can use for are:
 * @li "default" - Title label in a segment control item
 *
 * Supported elm_object common APIs.
 * @li elm_object_disabled_set
 * @li elm_object_disabled_get
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 *
 * Here is an example on its usage:
 * @li @ref segment_control_example
 *
 */

/**
 * @addtogroup SegmentControl
 * @{
 */

#include "elm_segment_control_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_segment_control_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_segment_control_legacy.h"
#endif
/**
 * @}
 */
