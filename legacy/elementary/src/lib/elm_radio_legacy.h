/**
 * @brief Add a new radio to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Radio
 */
EAPI Evas_Object                 *elm_radio_add(Evas_Object *parent);

/**
 * @brief Add this radio to a group of other radio objects
 *
 * @param obj The radio object
 * @param group Any object whose group the @p obj is to join.
 *
 * Radio objects work in groups. Each member should have a different integer
 * value assigned. In order to have them work as a group, they need to know
 * about each other. This adds the given radio object to the group of which
 * the group object indicated is a member.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_group_add(Evas_Object *obj, Evas_Object *group);

/**
 * @brief Set the integer value that this radio object represents
 *
 * @param obj The radio object
 * @param value The value to use if this radio object is selected
 *
 * This sets the value of the radio.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_state_value_set(Evas_Object *obj, int value);

/**
 * @brief Get the integer value that this radio object represents
 *
 * @param obj The radio object
 * @return The value used if this radio object is selected
 *
 * This gets the value of the radio.
 * @see elm_radio_value_set()
 *
 * @ingroup Radio
 */
EAPI int                          elm_radio_state_value_get(const Evas_Object *obj);

/**
 * @brief Set the value of the radio group.
 *
 * @param obj The radio object (any radio object of the group).
 * @param value The value to use for the group
 *
 * This sets the value of the radio group and will also set the value if
 * pointed to, to the value supplied, but will not call any callbacks.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_value_set(Evas_Object *obj, int value);

/**
 * @brief Get the value of the radio group
 *
 * @param obj The radio object (any radio object of the group).
 * @return The integer state
 *
 * @ingroup Radio
 */
EAPI int                          elm_radio_value_get(const Evas_Object *obj);

/**
 * @brief Set a convenience pointer to a integer to change when radio group
 * value changes.
 *
 * @param obj The radio object (any object of a group)
 * @param valuep Pointer to the integer to modify
 *
 * This sets a pointer to a integer, that, in addition to the radio objects
 * state will also be modified directly. To stop setting the object pointed
 * to simply use NULL as the @p valuep argument. If valuep is not NULL, then
 * when this is called, the radio objects state will also be modified to
 * reflect the value of the integer valuep points to, just like calling
 * elm_radio_value_set().
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_value_pointer_set(Evas_Object *obj, int *valuep);

/**
 * @brief Get the selected radio object.
 *
 * @param obj Any radio object (any object of a group)
 * @return The selected radio object
 *
 * @ingroup Radio
 */
EAPI Evas_Object                 *elm_radio_selected_object_get(Evas_Object *obj);
