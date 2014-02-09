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
 * @brief This sets the hoversel to expand horizontally.
 *
 * @param obj The hoversel object
 * @param horizontal If true, the hover will expand horizontally to the
 * right.
 *
 * @note The initial button will display horizontally regardless of this
 * setting.
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief This returns whether the hoversel is set to expand horizontally.
 *
 * @param obj The hoversel object
 * @return If true, the hover will expand horizontally to the right.
 *
 * @see elm_hoversel_horizontal_set()
 *
 * @ingroup Hoversel
 */
EAPI Eina_Bool                    elm_hoversel_horizontal_get(const Evas_Object *obj);

/**
 * @brief Set the Hover parent
 *
 * @param obj The hoversel object
 * @param parent The parent to use
 *
 * Sets the hover parent object, the area that will be darkened when the
 * hoversel is clicked. Should probably be the window that the hoversel is
 * in. See @ref Hover objects for more information.
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the Hover parent
 *
 * @param obj The hoversel object
 * @return The used parent
 *
 * Get the hover parent object.
 *
 * @see elm_hoversel_hover_parent_set()
 *
 * @ingroup Hoversel
 */
EAPI Evas_Object                 *elm_hoversel_hover_parent_get(const Evas_Object *obj);

/**
 * @brief This triggers the hoversel popup from code, the same as if the user
 * had clicked the button.
 *
 * @param obj The hoversel object
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_hover_begin(Evas_Object *obj);

/**
 * @brief This dismisses the hoversel popup as if the user had clicked
 * outside the hover.
 *
 * @param obj The hoversel object
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_hover_end(Evas_Object *obj);

/**
 * @brief Returns whether the hoversel is expanded.
 *
 * @param obj The hoversel object
 * @return  This will return @c EINA_TRUE if the hoversel is expanded or
 * @c EINA_FALSE if it is not expanded.
 *
 * @ingroup Hoversel
 */
EAPI Eina_Bool                    elm_hoversel_expanded_get(const Evas_Object *obj);

/**
 * @brief This will remove all the children items from the hoversel.
 *
 * @param obj The hoversel object
 *
 * @warning Should @b not be called while the hoversel is active; use
 * elm_hoversel_expanded_get() to check first.
 *
 * @see elm_object_item_del()
 *
 * @ingroup Hoversel
 */
EAPI void                         elm_hoversel_clear(Evas_Object *obj);

/**
 * @brief Get the list of items within the given hoversel.
 *
 * @param obj The hoversel object
 * @return Returns a list of Elm_Object_Item*
 *
 * @see elm_hoversel_item_add()
 *
 * @ingroup Hoversel
 */
EAPI const Eina_List             *elm_hoversel_items_get(const Evas_Object *obj);

/**
 * @brief Add an item to the hoversel button
 *
 * @param obj The hoversel object
 * @param label The text label to use for the item (NULL if not desired)
 * @param icon_file An image file path on disk to use for the icon or standard
 * icon name (NULL if not desired)
 * @param icon_type The icon type if relevant
 * @param func Convenience function to call when this item is selected. The last
 * parameter @p event_info of @c func is the selected item pointer.
 * @param data Data to pass to item-related functions
 * @return A handle to the item added.
 *
 * This adds an item to the hoversel to show when it is clicked. Note: if you
 * need to use an icon from an edje file then use
 * elm_hoversel_item_icon_set() right after this function, and set
 * icon_file to NULL here.
 *
 * For more information on what @p icon_file and @p icon_type are, see the
 * @ref Icon "icon documentation".
 *
 * @ingroup Hoversel
 */
EAPI Elm_Object_Item             *elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

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
