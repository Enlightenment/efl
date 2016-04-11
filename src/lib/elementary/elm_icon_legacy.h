/**
 * Add a new icon object to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Elm_Icon
 */
EAPI Evas_Object          *elm_icon_add(Evas_Object *parent);

/**
 * @brief Set the file that will be used, but use a generated thumbnail.
 *
 * This functions like elm_image_file_set() but requires the Ethumb library
 * support to be enabled successfully with @c elm_need_ethumb. When set the
 * file indicated has a thumbnail generated and cached on disk for future use
 * or will directly use an existing cached thumbnail if it is valid.
 *
 * @param[in] file The path to file that will be used as icon image
 * @param[in] group The group that the icon belongs to an edje file
 *
 * @ingroup Elm_Icon
 */
EAPI void elm_icon_thumb_set(Evas_Object *obj, const char *file, const char *group);

#include "elm_icon.eo.legacy.h"