/**
 * @brief Add a new Ctxpopup object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_add(Evas_Object *parent);

/**
 * @brief Set the Ctxpopup's parent
 *
 * @param obj The ctxpopup object
 * @param parent The parent to use
 *
 * Set the parent object.
 *
 * @note elm_ctxpopup_add() will automatically call this function
 * with its @c parent argument.
 *
 * @see elm_ctxpopup_add()
 * @see elm_hover_parent_set()
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the Ctxpopup's parent
 *
 * @param obj The ctxpopup object
 *
 * @see elm_ctxpopup_hover_parent_set() for more information
 *
 * @ingroup Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_hover_parent_get(const Evas_Object *obj);

/**
 * @brief Clear all items in the given ctxpopup object.
 *
 * @param obj Ctxpopup object
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_clear(Evas_Object *obj);

/**
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param obj Ctxpopup object
 * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param obj Ctxpopup object
 * @return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical mode (or errors)
 *
 * @see elm_ctxpopup_horizontal_set()
 *
 * @ingroup Ctxpopup
 */
EAPI Eina_Bool                    elm_ctxpopup_horizontal_get(const Evas_Object *obj);

/**
 * @brief Add a new item to a ctxpopup object.
 *
 * @param obj Ctxpopup object
 * @param icon Icon to be set on new item
 * @param label The Label of the new item
 * @param func Convenience function called when item selected
 * @param data Data passed to @p func
 * @return A handle to the item added or @c NULL, on errors
 *
 * @warning Ctxpopup can't hold both an item list and a content at the same
 * time. When an item is added, any previous content will be removed.
 *
 * @see elm_object_content_set()
 *
 * @ingroup Ctxpopup
 */
EAPI Elm_Object_Item             *elm_ctxpopup_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data);

/**
 * @brief Set the direction priority of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param first 1st priority of direction
 * @param second 2nd priority of direction
 * @param third 3th priority of direction
 * @param fourth 4th priority of direction
 *
 * This functions gives a chance to user to set the priority of ctxpopup
 * showing direction. This doesn't guarantee the ctxpopup will appear in the
 * requested direction.
 *
 * @see Elm_Ctxpopup_Direction
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_direction_priority_set(Evas_Object *obj, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth);

/**
 * @brief Get the direction priority of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param first 1st priority of direction to be returned
 * @param second 2nd priority of direction to be returned
 * @param third 3th priority of direction to be returned
 * @param fourth 4th priority of direction to be returned
 *
 * @see elm_ctxpopup_direction_priority_set() for more information.
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_direction_priority_get(Evas_Object *obj, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth);

/**
 * @brief Get the current direction of a ctxpopup.
 *
 * @param obj Ctxpopup object
 * @return current direction of a ctxpopup
 *
 * @warning Once the ctxpopup showed up, the direction would be determined
 *
 * @ingroup Ctxpopup
 */
EAPI Elm_Ctxpopup_Direction       elm_ctxpopup_direction_get(const Evas_Object *obj);

/**
 * @brief Dismiss a ctxpopup object
 *
 * @param obj The ctxpopup object
 * Use this function to simulate clicking outside of the ctxpopup to dismiss it.
 * In this way, the ctxpopup will be hidden and the "clicked" signal will be
 * emitted.
 */
EAPI void                         elm_ctxpopup_dismiss(Evas_Object *obj);

/**
 * @brief Set ctxpopup auto hide mode triggered by ctxpopup policy.
 * @since 1.9
 *
 * @param obj The ctxpopup object
 * @param disabled auto hide enable/disable.
 *
 * Use this function when user wants ctxpopup not to hide automatically.
 * By default, ctxpopup is dismissed whenever mouse clicked its background area, language is changed,
 * and its parent geometry is updated(changed).
 * Not to hide ctxpopup automatically, disable auto hide function by calling this API,
 * then ctxpopup won't be dismissed in those scenarios.
 *
 * Default value of disabled is @c EINA_FALSE.
 *
 * @see elm_ctxpopup_auto_hide_disabled_get()
 *
 * @ingroup Ctxpopup
 */
EAPI void                         elm_ctxpopup_auto_hide_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * @brief Get ctxpopup auto hide mode triggered by ctxpopup policy.
 * @since 1.9
 *
 * @param obj The ctxpopup object
 * @return auto hide mode's state of a ctxpopup
 *
 * @see elm_ctxpopup_auto_hide_disabled_set() for more information.
 *
 * @ingroup Ctxpopup
 */
EAPI Eina_Bool                         elm_ctxpopup_auto_hide_disabled_get(const Evas_Object *obj);
