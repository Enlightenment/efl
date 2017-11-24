/**
 * @defgroup Elm_Combobox Combobox
 * @ingroup Elementary
 *
 * @image html combobox_inheritance_tree.png
 * @image latex combobox_inheritance_tree.eps
 *
 * @image html img/widget/combobox/preview-00.png
 * @image latex img/widget/combobox/preview-00.eps
 *
 * A combobox is a button displaying an entry that pops up a list of items
 * (automatically choosing the direction to display). It is a convenience
 * widget to avoid the need to do all the piecing together yourself. It is
 * intended for manipulating a large number of items in the combobox menu.
 *
 * This widget inherits from the @ref Button, @ref Genlist and @ref Entry
 * one, so that all the functions acting on it also work for combobox objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Button:
 * - @c "clicked" - the user clicked the combobox button and popped up
 *   the sel
 * - @c "selected" - an item in the combobox list is selected. event_info
 *   is the selected item
 * - @c "dismissed" - the hover is dismissed
 * - @c "expanded" - This is called on clicking combobox and elm_combobox_hover_begin().
 * - @c "language,changed" - the program's language changed.
 * - @c "item,pressed" - When the combobox item is pressed.
 * - @c "filter,done" - When the combobox completes the filter process.
 *
 * Default content parts of the combobox widget that you can use are the
 * the same that you use with the @ref Button
 *
 * Default text parts of the combobox widget that you can use are the
 * the same that you use with the @ref Entry
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
 * See @ref tutorial_combobox for an example.
 * @{
 */

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_combobox_legacy.h"
#endif
/**
 * @}
 */
