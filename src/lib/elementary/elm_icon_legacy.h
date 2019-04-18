/** Lookup order used by elm_icon_standard_set(). Should look for icons in the
 * theme, FDO paths, or both?
 *
 * @ingroup Elm_Icon
 *
 * @deprecated Prefer using elm_config "icon_theme" instead.
 */
typedef enum
{
  ELM_ICON_LOOKUP_FDO_THEME = 0, /** Icon look up order: freedesktop, theme. */
  ELM_ICON_LOOKUP_THEME_FDO, /** Icon look up order: theme, freedesktop. */
  ELM_ICON_LOOKUP_FDO, /** Icon look up order: freedesktop. */
  ELM_ICON_LOOKUP_THEME /** Icon look up order: theme. */
} Elm_Icon_Lookup_Order;

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
/**
 * @brief Sets the icon lookup order used by elm_icon_standard_set().
 *
 * See also @ref elm_icon_order_lookup_get, @ref Elm_Icon_Lookup_Order.
 *
 * @param[in] order The icon lookup order (can be one of
 * ELM_ICON_LOOKUP_FDO_THEME, ELM_ICON_LOOKUP_THEME_FDO, ELM_ICON_LOOKUP_FDO or
 * ELM_ICON_LOOKUP_THEME)
 *
 * @ingroup Elm_Icon
 */

EAPI void elm_icon_order_lookup_set(Evas_Object *obj EINA_UNUSED, Elm_Icon_Lookup_Order order EINA_UNUSED) EINA_DEPRECATED;

/**
 * @brief Get the icon lookup order.
 *
 * See also @ref elm_icon_order_lookup_set, @ref Elm_Icon_Lookup_Order.
 *
 * @return The icon lookup order (can be one of ELM_ICON_LOOKUP_FDO_THEME,
 * ELM_ICON_LOOKUP_THEME_FDO, ELM_ICON_LOOKUP_FDO or ELM_ICON_LOOKUP_THEME)
 *
 * @ingroup Elm_Icon
 */
EAPI Elm_Icon_Lookup_Order elm_icon_order_lookup_get(const Evas_Object *obj EINA_UNUSED) EINA_DEPRECATED;

/**
 * @brief Set the icon by icon standards names.
 *
 * For example, freedesktop.org defines standard icon names such as "go-home",
 * "network-wired", etc. There can be different icon sets to match those icon keys.
 * The "name" given as parameter is one of these "keys", and will be used to
 * look in the freedesktop.org paths and elementary theme.
 *
 * If name is not found in any of the expected locations and it is the absolute
 * path of an image file, this image will be used.
 *
 * @note The icon image set by this function can be changed by
 * @ref Efl.File.file.set.
 *
 * @note This function does not accept relative icon path.
 *
 * See also @ref elm_icon_standard_get.
 *
 * @param[in] name The icon name
 *
 * @return true on success, false on error
 *
 * @ingroup Elm_Icon
 */
EAPI Eina_Bool elm_icon_standard_set(Evas_Object *obj, const char *name);

/**
 * @brief Get the icon name set by icon standard names.
 *
 * If the icon image was set using elm_image_file_set() instead of
 * @ref elm_icon_standard_set, then this function will return null.
 *
 * @return The icon name
 *
 * @ingroup Elm_Icon
 */
EAPI const char *elm_icon_standard_get(const Evas_Object *obj);

#include "elm_icon_eo.legacy.h"
