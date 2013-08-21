/**
 * @brief Adds a hover object to @p parent
 *
 * @param parent The parent object
 * @return The hover object or NULL if one could not be created
 *
 * @ingroup Hover
 */
EAPI Evas_Object *elm_hover_add(Evas_Object *parent);

/**
 * @brief Sets the parent object for the hover.
 *
 * @param obj The hover object
 * @param parent The object to locate the hover over.
 *
 * This function will cause the hover to take up the entire space that the
 * parent object fills.
 *
 * @ingroup Hover
 */
EAPI void         elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the parent object for the hover.
 *
 * @param obj The hover object
 * @return The parent object to locate the hover over.
 *
 * @see elm_hover_parent_set()
 *
 * @ingroup Hover
 */
EAPI Evas_Object *elm_hover_parent_get(const Evas_Object *obj);

/**
 * @brief Sets the target object for the hover.
 *
 * @param obj The hover object
 * @param target The object to center the hover onto.
 *
 * This function will cause the hover to be centered on the target object.
 *
 * @ingroup Hover
 */
EAPI void         elm_hover_target_set(Evas_Object *obj, Evas_Object *target);

/**
 * @brief Get the target object for the hover.
 *
 * @param obj The hover object
 * @return The target object for the hover.
 *
 * @see elm_hover_target_set()
 *
 * @ingroup Hover
 */
EAPI Evas_Object *elm_hover_target_get(const Evas_Object *obj);

/**
 * @brief Returns the best swallow location for content in the hover.
 *
 * @param obj The hover object
 * @param pref_axis The preferred orientation axis for the hover object to use
 * @return The edje location to place content into the hover or @c
 *         NULL, on errors.
 *
 * Best is defined here as the location at which there is the most available
 * space.
 *
 * @p pref_axis may be one of
 * - @c ELM_HOVER_AXIS_NONE -- no preferred orientation
 * - @c ELM_HOVER_AXIS_HORIZONTAL -- horizontal
 * - @c ELM_HOVER_AXIS_VERTICAL -- vertical
 * - @c ELM_HOVER_AXIS_BOTH -- both
 *
 * If ELM_HOVER_AXIS_HORIZONTAL is chosen the returned position will
 * necessarily be along the horizontal axis("left" or "right"). If
 * ELM_HOVER_AXIS_VERTICAL is chosen the returned position will necessarily
 * be along the vertical axis("top" or "bottom"). Choosing
 * ELM_HOVER_AXIS_BOTH or ELM_HOVER_AXIS_NONE has the same effect and the
 * returned position may be in either axis.
 *
 * @see elm_object_part_content_set()
 *
 * @ingroup Hover
 */
EAPI const char  *elm_hover_best_content_location_get(const Evas_Object *obj, Elm_Hover_Axis pref_axis);

/**
 * @brief Dismiss a hover object
 *
 * @param obj The hover object
 * Use this function to simulate clicking outside the hover to dismiss it.
 * In this way, the hover will be hidden and the "clicked" signal will be emitted.
 *
 * @ingroup Hover
 */
EAPI void elm_hover_dismiss(Evas_Object *obj);
