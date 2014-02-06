/**
 * @brief Adds a new Popup to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Popup
 */
EAPI Evas_Object *elm_popup_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the wrapping type of content text packed in content
 * area of popup object.
 *
 * @param obj The Popup object
 * @param wrap wrapping type of type Elm_Wrap_Type
 *
 * @ingroup Popup
 * @see elm_popup_content_text_wrap_type_get()
 */
EAPI void elm_popup_content_text_wrap_type_set(Evas_Object *obj, Elm_Wrap_Type wrap) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the wrapping type of content text packed in content area of
 * popup object.
 *
 * @param obj The Popup object
 * @return wrap type of the content text
 *
 * @ingroup Popup
 * @see elm_popup_content_text_wrap_type_set
 */
EAPI Elm_Wrap_Type elm_popup_content_text_wrap_type_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the orientation of the popup in the parent region
 *
 * @param obj The popup object
 * @param orient  the orientation of the popup
 *
 * Sets the position in which popup will appear in its parent. By default,
 * #ELM_POPUP_ORIENT_CENTER is set.
 *
 * @ingroup Popup
 * @see @ref Elm_Popup_Orient for possible values.
 */
EAPI void elm_popup_orient_set(Evas_Object *obj, Elm_Popup_Orient orient) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the orientation of Popup
 *
 * @param obj The popup object
 * @return the orientation of the popup
 *
 * @ingroup Popup
 * @see elm_popup_orient_set()
 * @see Elm_Popup_Orient
 */
EAPI Elm_Popup_Orient elm_popup_orient_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets a timeout to hide popup automatically
 *
 * @param obj The popup object
 * @param timeout The timeout in seconds
 *
 * This function sets a timeout and starts the timer controlling when the
 * popup is hidden. Since calling evas_object_show() on a popup restarts
 * the timer controlling when it is hidden, setting this before the
 * popup is shown will in effect mean starting the timer when the popup is
 * shown. Smart signal "timeout" is called afterwards which can be handled
 * if needed.
 *
 * @note Set a value <= 0.0 to disable a running timer.
 *
 * @note If the value > 0.0 and the popup is previously visible, the
 * timer will be started with this value, canceling any running timer.
 * @ingroup Popup
 */
EAPI void elm_popup_timeout_set(Evas_Object *obj, double timeout) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the timeout value set to the popup (in seconds)
 *
 * @param obj The popup object
 * @return the timeout value
 *
 * @ingroup Popup
 * @see elm_popup_timeout_set()
 */
EAPI double elm_popup_timeout_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets whether events should be passed to by a click outside.
 *
 * @param obj The popup object
 * @param allow @c EINA_TRUE Events are passed to lower objects, else not
 *
 * Enabling allow event will remove the Blocked event area and events will
 * pass to the lower layer objects otherwise they are blocked.
 *
 * @ingroup Popup
 * @see elm_popup_allow_events_get()
 * @note The default value is EINA_FALSE.
 */
EAPI void elm_popup_allow_events_set(Evas_Object *obj, Eina_Bool allow);

/**
 * @brief Returns value indicating whether allow event is enabled or not
 *
 * @param obj The popup object
 * @return @c EINA_FALSE if Blocked event area is present else @c EINA_TRUE
 *
 * @ingroup Popup
 * @see elm_popup_allow_events_set()
 * @note By default the Blocked event area is present
 */
EAPI Eina_Bool elm_popup_allow_events_get(const Evas_Object *obj);

/**
 * @brief Add a new item to a Popup object
 *
 * Both an item list and a content could not be set at the same time!
 * once you add an item, the previous content will be removed.
 *
 * @param obj popup object
 * @param icon Icon to be set on new item
 * @param label The Label of the new item
 * @param func Convenience function called when item selected
 * @param data Data passed to @p func above
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Popup
 * @warning When the first item is appended to popup object, any previous content
 * of the content area is deleted. At a time, only one of content, content-text
 * and item(s) can be there in a popup content area.
 */
EAPI Elm_Object_Item *elm_popup_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

/**
 * @brief Set the alignment of the popup object
 * @param obj popup object
 * @param horizontal The horizontal alignment of the popup
 * @param vertical The vertical alignment of the popup
 *
 * Sets the alignment in which the popup will appear in its parent.
 *
 * @see elm_popup_align_get()
 *
 * @since 1.9
 * @ingroup Popup
 */
EAPI void elm_popup_align_set(Evas_Object *obj, double horizontal, double vertical);

/**
 * @brief Get the alignment of the popup object
 * @param obj The popup object
 * @param horizontal The horizontal alignment of the popup
 * @param vertical The vertical alignment of the popup
 *
 * @see elm_popup_align_set()
 *
 * @since 1.9
 * @ingroup Popup
 */
EAPI void elm_popup_align_get(const Evas_Object *obj, double *horizontal, double *vertical);
