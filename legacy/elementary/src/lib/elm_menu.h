/**
 * @defgroup Menu Menu
 * @ingroup Elementary
 *
 * @image html menu_inheritance_tree.png
 * @image latex menu_inheritance_tree.eps
 *
 * @image html img/widget/menu/preview-00.png
 * @image latex img/widget/menu/preview-00.eps
 *
 * A menu is a list of items displayed above its parent. When the menu is
 * showing its parent is darkened. Each item can have a sub-menu. The menu
 * object can be used to display a menu on a right click event, in a toolbar,
 * anywhere.
 *
 * Signals that you can add callbacks for are:
 * @li @c "clicked" - the user clicked the empty space in the menu to dismiss.
 * @li @c "dismissed" - the user clicked the empty space in the menu to dismiss (since 1.8)
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * Default content parts of the menu items that you can use for are:
 * @li @c "default" - A main content of the menu item
 *
 * Default text parts of the menu items that you can use for are:
 * @li @c "default" - label in the menu item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 * @li @ref elm_object_item_part_content_set
 * @li @ref elm_object_item_part_content_get
 * @li @ref elm_object_item_disabled_set
 * @li @ref elm_object_item_disabled_get
 *
 * @see @ref tutorial_menu
 * @{
 */

#include "elm_menu_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_menu_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_menu_legacy.h"
#endif
/**
 * @}
 */
