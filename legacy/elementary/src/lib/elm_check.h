/**
 * @defgroup Check Check
 *
 * @image html img/widget/check/preview-00.png
 * @image latex img/widget/check/preview-00.eps
 * @image html img/widget/check/preview-01.png
 * @image latex img/widget/check/preview-01.eps
 * @image html img/widget/check/preview-02.png
 * @image latex img/widget/check/preview-02.eps
 *
 * @brief The check widget allows for toggling a value between true and
 * false.
 *
 * Check objects are a lot like radio objects in layout and functionality
 * except they do not work as a group, but independently and only toggle the
 * value of a boolean from false to true (0 or 1). elm_check_state_set() sets
 * the boolean state (1 for true, 0 for false), and elm_check_state_get()
 * returns the current state. For convenience, like the radio objects, you
 * can set a pointer to a boolean directly with elm_check_state_pointer_set()
 * for it to modify.
 *
 * Signals that you can add callbacks for are:
 * "changed" - This is called whenever the user changes the state of one of
 *             the check object(event_info is NULL).
 *
 * Default contents parts of the check widget that you can use for are:
 * @li "icon" - An icon of the check
 *
 * Default text parts of the check widget that you can use for are:
 * @li "elm.text" - Label of the check
 *
 * @ref tutorial_check should give you a firm grasp of how to use this widget.
 * 
 * @{
 */

/**
 * @brief Add a new Check object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 */
EAPI Evas_Object *                elm_check_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * @brief Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_check_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

/**
 * @brief Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_check_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the icon object of the check object
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1);

/**
 * @brief Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Unset the icon used for the check object
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the on/off state of the check object
 *
 * @param obj The check object
 * @param state The state to use (1 == on, 0 == off)
 *
 * This sets the state of the check. If set
 * with elm_check_state_pointer_set() the state of that variable is also
 * changed. Calling this @b doesn't cause the "changed" signal to be emited.
 */
EAPI void                         elm_check_state_set(Evas_Object *obj, Eina_Bool state) EINA_ARG_NONNULL(1);

/**
 * @brief Get the state of the check object
 *
 * @param obj The check object
 * @return The boolean state
 */
EAPI Eina_Bool                    elm_check_state_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set a convenience pointer to a boolean to change
 *
 * @param obj The check object
 * @param statep Pointer to the boolean to modify
 *
 * This sets a pointer to a boolean, that, in addition to the check objects
 * state will also be modified directly. To stop setting the object pointed
 * to simply use NULL as the @p statep parameter. If @p statep is not NULL,
 * then when this is called, the check objects state will also be modified to
 * reflect the value of the boolean @p statep points to, just like calling
 * elm_check_state_set().
 */
EAPI void                         elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep) EINA_ARG_NONNULL(1);
EINA_DEPRECATED EAPI void         elm_check_states_labels_set(Evas_Object *obj, const char *ontext, const char *offtext) EINA_ARG_NONNULL(1, 2, 3);
EINA_DEPRECATED EAPI void         elm_check_states_labels_get(const Evas_Object *obj, const char **ontext, const char **offtext) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @}
 */
