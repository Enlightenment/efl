/**
 * @brief Add a new frame to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Frame
 */
EAPI Evas_Object                 *elm_frame_add(Evas_Object *parent);

/**
 * @brief Toggle autocollapsing of a frame
 * @param obj The frame
 * @param autocollapse Whether to enable autocollapse
 *
 * When @p enable is EINA_TRUE, clicking a frame's label will collapse the frame
 * vertically, shrinking it to the height of the label.
 * By default, this is DISABLED.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_autocollapse_set(Evas_Object *obj, Eina_Bool autocollapse);

/**
 * @brief Determine autocollapsing of a frame
 * @param obj The frame
 * @return Whether autocollapse is enabled
 *
 * When this returns EINA_TRUE, clicking a frame's label will collapse the frame
 * vertically, shrinking it to the height of the label.
 * By default, this is DISABLED.
 *
 * @ingroup Frame
 */
EAPI Eina_Bool elm_frame_autocollapse_get(const Evas_Object *obj);

/**
 * @brief Manually collapse a frame without animations
 * @param obj The frame
 * @param collapse true to collapse, false to expand
 *
 * Use this to toggle the collapsed state of a frame, bypassing animations.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_collapse_set(Evas_Object *obj, Eina_Bool collapse);

/**
 * @brief Manually collapse a frame with animations
 * @param obj The frame
 * @param collapse true to collapse, false to expand
 *
 * Use this to toggle the collapsed state of a frame, triggering animations.
 *
 * @ingroup Frame
 */
EAPI void elm_frame_collapse_go(Evas_Object *obj, Eina_Bool collapse);

/**
 * @brief Determine the collapse state of a frame
 * @param obj The frame
 * @return true if collapsed, false otherwise
 *
 * Use this to determine the collapse state of a frame.
 *
 * @ingroup Frame
 */
EAPI Eina_Bool elm_frame_collapse_get(const Evas_Object *obj);
