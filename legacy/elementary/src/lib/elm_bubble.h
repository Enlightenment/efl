/**
 * @defgroup Bubble Bubble
 * @ingroup Elementary
 *
 * @image html bubble_inheritance_tree.png
 * @image latex bubble_inheritance_tree.eps
 *
 * @image html img/widget/bubble/preview-00.png
 * @image latex img/widget/bubble/preview-00.eps
 * @image html img/widget/bubble/preview-01.png
 * @image latex img/widget/bubble/preview-01.eps
 * @image html img/widget/bubble/preview-02.png
 * @image latex img/widget/bubble/preview-02.eps
 *
 * @brief The Bubble is a widget to show text similar to how speech is
 * represented in comics.
 *
 * The bubble widget contains 5 important visual elements:
 * @li The frame is a rectangle with rounded edjes and an "arrow".
 * @li The @p icon is an image to which the frame's arrow points to.
 * @li The @p label is a text which appears to the right of the icon if the
 * corner is "top_left" or "bottom_left" and is right aligned to the frame
 * otherwise.
 * @li The @p info is a text which appears to the right of the label. Info's
 * font is of a lighter color than label.
 * @li The @p content is an evas object that is shown inside the frame.
 *
 * The position of the arrow, icon, label and info depends on which corner is
 * selected. The four available corners are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for bubble objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "clicked" - This is called when a user has clicked the bubble.
 * @li @c "focused" - When the bubble has received focus. (since 1.8)
 * @li @c "unfocused" - When the bubble has lost focus. (since 1.8)
 *
 * Default content parts of the bubble that you can use for are:
 * @li "default" - A content of the bubble
 * @li "icon" - An icon of the bubble
 *
 * Default text parts of the button widget that you can use for are:
 * @li "default" - Label of the bubble
 * @li "info" - info of the bubble
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * For an example of using a bubble see @ref bubble_01_example_page "this".
 *
 * @{
 */

#include "elm_bubble_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_bubble_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_bubble_legacy.h"
#endif
/**
 * @}
 */
