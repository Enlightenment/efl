/**
 * @defgroup Check Check
 * @ingroup Elementary
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
 * "changed" - This is called whenever the user changes the state of the check
 *             objects(event_info is NULL).
 *
 * Default content parts of the check widget that you can use for are:
 * @li "icon" - An icon of the check
 *
 * Default text parts of the check widget that you can use for are:
 * @li "default" - A label of the check
 * @li "on" - On state label of the check
 * @li "off" - Off state label of the check
 *
 * Supported elm_object common APIs.
 * @li elm_object_disabled_set
 * @li elm_object_disabled_get
 * @li elm_object_part_text_set
 * @li elm_object_part_text_get
 * @li elm_object_part_content_set
 * @li elm_object_part_content_get
 * @li elm_object_part_content_unset
 * @li elm_object_signal_emit
 * @li elm_object_signal_callback_add
 * @li elm_object_signal_callback_del
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
 *
 * @ingroup Check
 */
EAPI Evas_Object *                elm_check_add(Evas_Object *parent);

/**
 * @brief Set the on/off state of the check object
 *
 * @param obj The check object
 * @param state The state to use (1 == on, 0 == off)
 *
 * This sets the state of the check. If set with elm_check_state_pointer_set()
 * the state of that variable is also changed. Calling this @b doesn't cause
 * the "changed" signal to be emitted.
 *
 * @ingroup Check
 */
EAPI void                         elm_check_state_set(Evas_Object *obj, Eina_Bool state);

/**
 * @brief Get the state of the check object
 *
 * @param obj The check object
 * @return The boolean state
 *
 * @ingroup Check
 */
EAPI Eina_Bool                    elm_check_state_get(const Evas_Object *obj);

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
 *
 * @ingroup Check
 */
EAPI void                         elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

/**
 * @}
 */
