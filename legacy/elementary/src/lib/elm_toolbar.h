/**
 * @defgroup Toolbar Toolbar
 * @ingroup Elementary
 *
 * @image html toolbar_inheritance_tree.png
 * @image latex toolbar_inheritance_tree.eps
 *
 * @image html img/widget/toolbar/preview-00.png
 * @image latex img/widget/toolbar/preview-00.eps width=\textwidth
 *
 * @image html img/toolbar.png
 * @image latex img/toolbar.eps width=\textwidth
 *
 * A toolbar is a widget that displays a list of items inside
 * a box. It can be scrollable, show a menu with items that don't fit
 * to toolbar size or even crop them.
 *
 * Only one item can be selected at a time.
 *
 * Items can have multiple states, or show menus when selected by the user.
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base
 * @ref Scroller widget also work for toolbars (@since 1.8)
 *
 * Smart callbacks one can listen to:
 * - @c "clicked" - when the user clicks on a toolbar item and becomes
 *                  selected.
 * - @c "longpressed" - when the toolbar is pressed for a certain
 *                      amount of time.
 * - @c "language,changed" - when the program language changes.
 * - @c "focused" - When the toolbar has received focus. (since 1.8)
 * - @c "unfocused" - When the toolbar has lost focus. (since 1.8)
 *
 * Available styles for it:
 * - @c "default"
 * - @c "transparent" - no background or shadow, just show the content
 *
 * Default text parts of the toolbar items that you can use for are:
 * @li "default" - label of the toolbar item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_part_content_unset
 *
 * List of examples:
 * @li @ref toolbar_example_01
 * @li @ref toolbar_example_02
 * @li @ref toolbar_example_03
 */

/**
 * @addtogroup Toolbar
 * @{
 */

#include <elm_toolbar_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_toolbar_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_toolbar_legacy.h>
#endif

/**
 * @}
 */
