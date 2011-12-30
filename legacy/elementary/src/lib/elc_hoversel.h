/**
 * @defgroup Hoversel Hoversel
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
 * Signals that you can add callbacks for are:
 * "clicked" - the user clicked the hoversel button and popped up the sel
 * "selected" - an item in the hoversel list is selected. event_info is the item
 * "dismissed" - the hover is dismissed
 *
 * Default contents parts of the hoversel widget that you can use for are:
 * @li "icon" - An icon of the hoversel
 *
 * Default text parts of the hoversel widget that you can use for are:
 * @li "default" - Label of the hoversel
 *
 * Supported elm_object common APIs.
 * @li elm_object_disabled_set
 * @li elm_object_text_set
 * @li elm_object_part_text_set
 * @li elm_object_text_get
 * @li elm_object_part_text_get
 * @li elm_object_content_set
 * @li elm_object_part_content_set
 * @li elm_object_content_unset
 * @li elm_object_part_content_unset
 *
 * Supported elm_object_item common APIs.
 * @li elm_object_item_text_get
 * @li elm_object_item_part_text_get
 *
 * See @ref tutorial_hoversel for an example.
 * @{
 */

/**
 * @brief Add a new Hoversel object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 */
EAPI Evas_Object *
                                  elm_hoversel_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * @brief This sets the hoversel to expand horizontally.
 *
 * @param obj The hoversel object
 * @param horizontal If true, the hover will expand horizontally to the
 * right.
 *
 * @note The initial button will display horizontally regardless of this
 * setting.
 */
EAPI void                         elm_hoversel_horizontal_set(Evas_Object *obj, Eina_Bool horizontal) EINA_ARG_NONNULL(1);

/**
 * @brief This returns whether the hoversel is set to expand horizontally.
 *
 * @param obj The hoversel object
 * @return If true, the hover will expand horizontally to the right.
 *
 * @see elm_hoversel_horizontal_set()
 */
EAPI Eina_Bool                    elm_hoversel_horizontal_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the Hover parent
 *
 * @param obj The hoversel object
 * @param parent The parent to use
 *
 * Sets the hover parent object, the area that will be darkened when the
 * hoversel is clicked. Should probably be the window that the hoversel is
 * in. See @ref Hover objects for more information.
 */
EAPI void                         elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent) EINA_ARG_NONNULL(1);
/**
 * @brief Get the Hover parent
 *
 * @param obj The hoversel object
 * @return The used parent
 *
 * Gets the hover parent object.
 *
 * @see elm_hoversel_hover_parent_set()
 */
EAPI Evas_Object                 *elm_hoversel_hover_parent_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the hoversel button label
 *
 * @param obj The hoversel object
 * @param label The label text.
 *
 * This sets the label of the button that is always visible (before it is
 * clicked and expanded).
 *
 * @deprecated elm_object_text_set()
 */
EINA_DEPRECATED EAPI void         elm_hoversel_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

/**
 * @brief Get the hoversel button label
 *
 * @param obj The hoversel object
 * @return The label text.
 *
 * @deprecated elm_object_text_get()
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @param icon The icon object
 *
 * Sets the icon of the button that is always visible (before it is clicked
 * and expanded).  Once the icon object is set, a previously set one will be
 * deleted, if you want to keep that old content object, use the
 * elm_hoversel_icon_unset() function.
 *
 * @see elm_object_content_set() for the button widget
 * @deprecated Use elm_object_item_part_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1);

/**
 * @brief Get the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object
 *
 * Get the icon of the button that is always visible (before it is clicked
 * and expanded). Also see elm_object_content_get() for the button widget.
 *
 * @see elm_hoversel_icon_set()
 * @deprecated Use elm_object_item_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Get and unparent the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object that was being used
 *
 * Unparent and return the icon of the button that is always visible
 * (before it is clicked and expanded).
 *
 * @see elm_hoversel_icon_set()
 * @see elm_object_content_unset() for the button widget
 * @deprecated Use elm_object_item_part_content_unset() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief This triggers the hoversel popup from code, the same as if the user
 * had clicked the button.
 *
 * @param obj The hoversel object
 */
EAPI void                         elm_hoversel_hover_begin(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief This dismisses the hoversel popup as if the user had clicked
 * outside the hover.
 *
 * @param obj The hoversel object
 */
EAPI void                         elm_hoversel_hover_end(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Returns whether the hoversel is expanded.
 *
 * @param obj The hoversel object
 * @return  This will return EINA_TRUE if the hoversel is expanded or
 * EINA_FALSE if it is not expanded.
 */
EAPI Eina_Bool                    elm_hoversel_expanded_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief This will remove all the children items from the hoversel.
 *
 * @param obj The hoversel object
 *
 * @warning Should @b not be called while the hoversel is active; use
 * elm_hoversel_expanded_get() to check first.
 *
 * @see elm_hoversel_item_del_cb_set()
 * @see elm_hoversel_item_del()
 */
EAPI void                         elm_hoversel_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Get the list of items within the given hoversel.
 *
 * @param obj The hoversel object
 * @return Returns a list of Elm_Object_Item*
 *
 * @see elm_hoversel_item_add()
 */
EAPI const Eina_List             *elm_hoversel_items_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Add an item to the hoversel button
 *
 * @param obj The hoversel object
 * @param label The text label to use for the item (NULL if not desired)
 * @param icon_file An image file path on disk to use for the icon or standard
 * icon name (NULL if not desired)
 * @param icon_type The icon type if relevant
 * @param func Convenience function to call when this item is selected
 * @param data Data to pass to item-related functions
 * @return A handle to the item added.
 *
 * This adds an item to the hoversel to show when it is clicked. Note: if you
 * need to use an icon from an edje file then use
 * elm_hoversel_item_icon_set() right after the this function, and set
 * icon_file to NULL here.
 *
 * For more information on what @p icon_file and @p icon_type are see the
 * @ref Icon "icon documentation".
 */
EAPI Elm_Object_Item             *elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Delete an item from the hoversel
 *
 * @param it The item to delete
 *
 * This deletes the item from the hoversel (should not be called while the
 * hoversel is active; use elm_hoversel_expanded_get() to check first).
 *
 * @see elm_hoversel_item_add()
 * @see elm_hoversel_item_del_cb_set()
 */
EAPI void                         elm_hoversel_item_del(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

/**
 * @brief Set the function to be called when an item from the hoversel is
 * freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * That function will receive these parameters:
 * @li void * item data
 * @li Evas_Object * hoversel object
 * @li Elm_Object_Item * hoversel item
 *
 * @see elm_hoversel_item_add()
 */
EAPI void                         elm_hoversel_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func) EINA_ARG_NONNULL(1);

/**
 * @brief This returns the data pointer supplied with elm_hoversel_item_add()
 * that will be passed to associated function callbacks.
 *
 * @param it The item to get the data from
 * @return The data pointer set with elm_hoversel_item_add()
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void        *elm_hoversel_item_data_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

/**
 * @brief This returns the label text of the given hoversel item.
 *
 * @param it The item to get the label
 * @return The label text of the hoversel item
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_text_get() instead
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_item_label_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

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
 */
EAPI void                         elm_hoversel_item_icon_set(Elm_Object_Item *it, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type) EINA_ARG_NONNULL(1);

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
 */
EAPI void                         elm_hoversel_item_icon_get(const Elm_Object_Item *it, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type) EINA_ARG_NONNULL(1);

/**
 * @}
 */
