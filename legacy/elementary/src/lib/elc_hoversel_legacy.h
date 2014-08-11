/**
 * @brief Add a new Hoversel object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Hoversel
 */
EAPI Evas_Object                 *elm_hoversel_add(Evas_Object *parent);

/**
 * @brief This sets the icon for the given hoversel item.
 *
 * @param it The item to set the icon
 * @param icon_file An image file path on disk to use for the icon or standard
 * icon name
 * @param icon_group The edje group to use if @p icon_file is an edje file. Set this
 * to NULL if the icon is not an edje file
 * @param icon_type The icon type
 *
 * The icon can be loaded from the standard set, from an image file, or from
 * an edje file.
 *
 * @see elm_hoversel_item_add()
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_item_icon_set(Elm_Object_Item *it, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type);

/**
 * @brief Get the icon object of the hoversel item
 *
 * @param it The item to get the icon from
 * @param icon_file The image file path on disk used for the icon or standard
 * icon name
 * @param icon_group The edje group used if @p icon_file is an edje file. NULL
 * if the icon is not an edje file
 * @param icon_type The icon type
 *
 * @see elm_hoversel_item_icon_set()
 * @see elm_hoversel_item_add()
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_item_icon_get(const Elm_Object_Item *it, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);

#include "elm_hoversel.eo.legacy.h"