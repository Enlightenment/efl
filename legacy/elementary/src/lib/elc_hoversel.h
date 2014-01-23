/**
 * @defgroup Hoversel Hoversel
 * @ingroup Elementary
 *
 * @image html hoversel_inheritance_tree.png
 * @image latex hoversel_inheritance_tree.eps
 *
 * @image html img/widget/hoversel/preview-00.png
 * @image latex img/widget/hoversel/preview-00.eps
 *
 * A hoversel is a button that pops up a list of items (automatically
 * choosing the direction to display) that have a label and, optionally, an
 * icon to select from. It is a convenience widget to avoid the need to do
 * all the piecing together yourself. It is intended for a small number of
 * items in the hoversel menu (no more than 8), though is capable of many
 * more.
 *
 * This widget inherits from the @ref Button one, so that all the
 * functions acting on it also work for hoversel objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Button:
 * - @c "clicked" - the user clicked the hoversel button and popped up
 *   the sel
 * - @c "selected" - an item in the hoversel list is selected. event_info
 *   is the selected item
 * - @c "dismissed" - the hover is dismissed
 * - @c "expanded" - This is called on clicking hoversel and elm_hoversel_hover_begin().
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Default content parts of the hoversel widget that you can use for are:
 * @li "icon" - An icon of the hoversel
 *
 * Default text parts of the hoversel widget that you can use for are:
 * @li "default" - Label of the hoversel
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_unset
 *
 * Supported elm_object_item common APIs.
 * @li elm_object_item_del
 * @li elm_object_item_part_text_get
 * @li elm_object_item_signal_emit - this works only when the item is created.
 * @li elm_object_item_style_set - this works only when the item is created.
 * @li elm_object_item_style_get - this works only when the item is created.
 *
 * See @ref tutorial_hoversel for an example.
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elc_hoversel_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_hoversel_legacy.h"
#endif
/**
 * @}
 */
