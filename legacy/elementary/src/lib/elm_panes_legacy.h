/**
 * Add a new panes widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new panes widget handle or @c NULL, on errors.
 *
 * This function inserts a new panes widget on the canvas.
 *
 * @ingroup Panes
 */
EAPI Evas_Object                 *elm_panes_add(Evas_Object *parent);

/**
 * Get the size proportion of panes widget's left side.
 *
 * @param obj The panes object.
 * @return float value between 0.0 and 1.0 representing size proportion
 * of left side.
 *
 * @see elm_panes_content_left_size_set() for more details.
 *
 * @ingroup Panes
 */
EAPI double                       elm_panes_content_left_size_get(const Evas_Object *obj);

/**
 * Set the size proportion of panes widget's left side.
 *
 * @param obj The panes object.
 * @param size Value between 0.0 and 1.0 representing size proportion
 * of left side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function.
 * For example, if the left content should be displayed over
 * 75% of the panes size, @p size should be passed as @c 0.75.
 * This way, right content will be resized to 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and
 * right content at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @see elm_panes_content_left_size_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_content_left_size_set(Evas_Object *obj, double size);

/**
 * Get the size proportion of panes widget's right side.
 *
 * @param obj The panes object.
 * @return float value between 0.0 and 1.0 representing size proportion
 * of right side.
 *
 * @see elm_panes_content_right_size_set() for more details.
 *
 * @ingroup Panes
 */
EAPI double                       elm_panes_content_right_size_get(const Evas_Object *obj);

/**
 * Set the size proportion of panes widget's right side.
 *
 * @param obj The panes object.
 * @param size Value between 0.0 and 1.0 representing size proportion
 * of right side.
 *
 * By default it's homogeneous, i.e., both sides have the same size.
 *
 * If something different is required, it can be set with this function.
 * For example, if the right content should be displayed over
 * 75% of the panes size, @p size should be passed as @c 0.75.
 * This way, left content will be resized to 25% of panes size.
 *
 * If displayed vertically, left content is displayed at top, and
 * right content at bottom.
 *
 * @note This proportion will change when user drags the panes bar.
 *
 * @see elm_panes_content_right_size_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_content_right_size_set(Evas_Object *obj, double size);


/**
 * Set how to split and dispose each content.
 *
 * @param obj The panes object.
 * @param horizontal Use @c EINA_TRUE to make @p obj to split panes
 * horizontally ("top" and  "bottom" contents). @c EINA_FALSE to make it
 * vertically ("left" and "right" contents)
 *
 * Use this function to change how your panes is to be disposed:
 * vertically or horizontally.
 * Horizontal panes have "top" and "bottom" contents, vertical panes have
 * "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @see elm_panes_horizontal_get()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Retrieve the split direction of a given panes widget.
 *
 * @param obj The panes object.
 * @return @c EINA_TRUE, if @p obj is set to be @b horizontal,
 * @c EINA_FALSE if it's @b vertical (and on errors).
 *
 * @see elm_panes_horizontal_set() for more details.
 *
 * @ingroup Panes
 */
EAPI Eina_Bool                    elm_panes_horizontal_get(const Evas_Object *obj);

/**
 * Set whether the left and right panes can be resized by user interaction.
 *
 * @param obj The panes object.
 * @param fixed Use @c EINA_TRUE to fix the left and right panes sizes and make
 * them not to be resized by user interaction. Use @c EINA_FALSE to make them
 * resizable.
 *
 * By default panes' contents are resizable by user interaction.
 *
 * @see elm_panes_fixed_get()
 * @see elm_panes_content_left_size_set()
 * @see elm_panes_content_right_size_set()
 *
 * @ingroup Panes
 */
EAPI void                         elm_panes_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Retrieve the resize mode for the panes of a given panes widget.
 *
 * @param obj The panes object.
 * @return @c EINA_TRUE, if @p obj is set to be resizable by user interaction.
 *
 * @see elm_panes_fixed_set() for more details.
 * @see elm_panes_content_left_size_get()
 * @see elm_panes_content_right_size_get()
 *
 * @ingroup Panes
 */
EAPI Eina_Bool                    elm_panes_fixed_get(const Evas_Object *obj);
