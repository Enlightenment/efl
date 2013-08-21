/**
 * Add a new icon object to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI Evas_Object          *elm_icon_add(Evas_Object *parent);

/**
 * Set the file that will be used, but use a generated thumbnail.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon image
 * @param group The group that the icon belongs to an edje file
 *
 * This functions like elm_image_file_set() but requires the Ethumb library
 * support to be enabled successfully with elm_need_ethumb(). When set
 * the file indicated has a thumbnail generated and cached on disk for
 * future use or will directly use an existing cached thumbnail if it
 * is valid.
 *
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI void                  elm_icon_thumb_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Set the icon by icon standards names.
 *
 * @param obj The icon object
 * @param name The icon name
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * For example, freedesktop.org defines standard icon names such as "home",
 * "network", etc. There can be different icon sets to match those icon
 * keys. The @p name given as parameter is one of these "keys", and will be
 * used to look in the freedesktop.org paths and elementary theme. One can
 * change the lookup order with elm_icon_order_lookup_set().
 *
 * If name is not found in any of the expected locations and it is the
 * absolute path of an image file, this image will be used.
 *
 * @note The icon image set by this function can be changed by
 * elm_image_file_set().
 *
 * @note This function does not accept relative icon path.
 *
 * @see elm_icon_standard_get()
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI Eina_Bool             elm_icon_standard_set(Evas_Object *obj, const char *name);

/**
 * Get the icon name set by icon standard names.
 *
 * @param obj The icon object
 * @return The icon name
 *
 * If the icon image was set using elm_image_file_set() instead of
 * elm_icon_standard_set(), then this function will return @c NULL.
 *
 * @see elm_icon_standard_set()
 *
 * @ingroup Icon
 */
EAPI const char           *elm_icon_standard_get(const Evas_Object *obj);

/**
 * Sets the icon lookup order used by elm_icon_standard_set().
 *
 * @param obj The icon object
 * @param order The icon lookup order (can be one of
 * ELM_ICON_LOOKUP_FDO_THEME, ELM_ICON_LOOKUP_THEME_FDO, ELM_ICON_LOOKUP_FDO
 * or ELM_ICON_LOOKUP_THEME)
 *
 * @see elm_icon_order_lookup_get()
 * @see Elm_Icon_Lookup_Order
 *
 * @ingroup Icon
 */
EAPI void                  elm_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order);

/**
 * Get the icon lookup order.
 *
 * @param obj The icon object
 * @return The icon lookup order
 *
 * @see elm_icon_order_lookup_set()
 * @see Elm_Icon_Lookup_Order
 *
 * @ingroup Icon
 */
EAPI Elm_Icon_Lookup_Order elm_icon_order_lookup_get(const Evas_Object *obj);
