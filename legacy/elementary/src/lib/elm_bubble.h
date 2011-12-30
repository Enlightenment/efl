/**
 * @defgroup Bubble Bubble
 *
 * @image html img/widget/bubble/preview-00.png
 * @image latex img/widget/bubble/preview-00.eps
 * @image html img/widget/bubble/preview-01.png
 * @image latex img/widget/bubble/preview-01.eps
 * @image html img/widget/bubble/preview-02.png
 * @image latex img/widget/bubble/preview-02.eps
 *
 * @brief The Bubble is a widget to show text similar to how speech is
 * represented in comics.
 *
 * The bubble widget contains 5 important visual elements:
 * @li The frame is a rectangle with rounded edjes and an "arrow".
 * @li The @p icon is an image to which the frame's arrow points to.
 * @li The @p label is a text which appears to the right of the icon if the
 * corner is "top_left" or "bottom_left" and is right aligned to the frame
 * otherwise.
 * @li The @p info is a text which appears to the right of the label. Info's
 * font is of a ligther color than label.
 * @li The @p content is an evas object that is shown inside the frame.
 *
 * The position of the arrow, icon, label and info depends on which corner is
 * selected. The four available corners are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 *
 * Signals that you can add callbacks for are:
 * @li "clicked" - This is called when a user has clicked the bubble.
 *
 * Default contents parts of the bubble that you can use for are:
 * @li "default" - A content of the bubble
 * @li "icon" - An icon of the bubble
 *
 * Default text parts of the button widget that you can use for are:
 * @li NULL - Label of the bubble
 *
 * For an example of using a buble see @ref bubble_01_example_page "this".
 *
 * @{
 */

/**
 * Add a new bubble to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function adds a text bubble to the given parent evas object.
 */
EAPI Evas_Object *
                                  elm_bubble_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * Set the label of the bubble
 *
 * @param obj The bubble object
 * @param label The string to set in the label
 *
 * This function sets the title of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_bubble_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

/**
 * Get the label of the bubble
 *
 * @param obj The bubble object
 * @return The string of set in the label
 *
 * This function gets the title of the bubble.
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the info of the bubble
 *
 * @param obj The bubble object
 * @param info The given info about the bubble
 *
 * This function sets the info of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_part_text_set() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI void         elm_bubble_info_set(Evas_Object *obj, const char *info) EINA_ARG_NONNULL(1);

/**
 * Get the info of the bubble
 *
 * @param obj The bubble object
 *
 * @return The "info" string of the bubble
 *
 * This function gets the info text.
 * @deprecated use elm_object_part_text_get() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_info_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the content to be shown in the bubble
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_bubble_content_unset() function.
 *
 * @param obj The bubble object
 * @param content The given content of the bubble
 *
 * This function sets the content shown on the middle of the bubble.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_content_set(Evas_Object *obj, Evas_Object *content) EINA_ARG_NONNULL(1);

/**
 * Get the content shown in the bubble
 *
 * Return the content object which is set for this widget.
 *
 * @param obj The bubble object
 * @return The content that is being used
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Unset the content shown in the bubble
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The content that was being used
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the icon of the bubble
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_icon_content_unset() function.
 *
 * @param obj The bubble object
 * @param icon The given icon for the bubble
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1);

/**
 * Get the icon of the bubble
 *
 * @param obj The bubble object
 * @return The icon for the bubble
 *
 * This function gets the icon shown on the top left of bubble.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Unset the icon of the bubble
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The icon that was being used
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param corner The given corner for the bubble.
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown.
 *
 * Possible values for corner are:
 * @li "top_left" - Default
 * @li "top_right"
 * @li "bottom_left"
 * @li "bottom_right"
 */
EAPI void                         elm_bubble_corner_set(Evas_Object *obj, const char *corner) EINA_ARG_NONNULL(1, 2);

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * This function gets the selected corner of the bubble.
 */
EAPI const char                  *elm_bubble_corner_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @}
 */
