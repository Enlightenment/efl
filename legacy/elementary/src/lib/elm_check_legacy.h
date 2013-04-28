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
