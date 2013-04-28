/**
 * Add a new bubble to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function adds a text bubble to the given parent evas object.
 *
 * @ingroup Bubble
 */
EAPI Evas_Object                 *elm_bubble_add(Evas_Object *parent);

/**
 * Set the corner of the bubble
 *
 * @param obj The bubble object.
 * @param pos The given corner for the bubble.
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown.
 *
 *
 * @ingroup Bubble
 */
EAPI void  elm_bubble_pos_set(Evas_Object *obj, Elm_Bubble_Pos pos);

/**
 * Get the corner of the bubble
 *
 * @param obj The bubble object.
 * @return The given corner for the bubble.
 *
 * This function gets the selected corner of the bubble.
 *
 * @ingroup Bubble
 */
EAPI Elm_Bubble_Pos elm_bubble_pos_get(const Evas_Object *obj);
