/**
 * @brief Add a new flip to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Flip
 */
EAPI Evas_Object *elm_flip_add(Evas_Object *parent);

/**
 * @brief Get flip front visibility state
 *
 * @param obj The flip object
 * @return EINA_TRUE if front front is showing, EINA_FALSE if the back is
 * showing.
 *
 * @ingroup Flip
 */
EAPI Eina_Bool elm_flip_front_visible_get(const Evas_Object *obj);

/**
 * @brief Set flip perspective
 *
 * @param obj The flip object
 * @param foc The coordinate to set the focus on
 * @param x The X coordinate
 * @param y The Y coordinate
 *
 * @warning This function currently does nothing.
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc, Evas_Coord x, Evas_Coord y);

/**
 * @brief Runs the flip animation
 *
 * @param obj The flip object
 * @param mode The mode type
 *
 * Flips the front and back contents using the @p mode animation. This
 * effectively hides the currently visible content and shows the hidden one.
 *
 * There a number of possible animations to use for the flipping:
 * @li ELM_FLIP_ROTATE_X_CENTER_AXIS - Rotate the currently visible content
 * around a horizontal axis in the middle of its height, the other content
 * is shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_Y_CENTER_AXIS - Rotate the currently visible content
 * around a vertical axis in the middle of its width, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_XZ_CENTER_AXIS - Rotate the currently visible content
 * around a diagonal axis in the middle of its width, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_YZ_CENTER_AXIS - Rotate the currently visible content
 * around a diagonal axis in the middle of its height, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_CUBE_LEFT - Rotate the currently visible content to the left
 * as if the flip was a cube, the other content is show as the right face of
 * the cube.
 * @li ELM_FLIP_CUBE_RIGHT - Rotate the currently visible content to the
 * right as if the flip was a cube, the other content is show as the left
 * face of the cube.
 * @li ELM_FLIP_CUBE_UP - Rotate the currently visible content up as if the
 * flip was a cube, the other content is show as the bottom face of the cube.
 * @li ELM_FLIP_CUBE_DOWN - Rotate the currently visible content down as if
 * the flip was a cube, the other content is show as the upper face of the
 * cube.
 * @li ELM_FLIP_PAGE_LEFT - Move the currently visible content to the left as
 * if the flip was a book, the other content is shown as the page below that.
 * @li ELM_FLIP_PAGE_RIGHT - Move the currently visible content to the right
 * as if the flip was a book, the other content is shown as the page below
 * that.
 * @li ELM_FLIP_PAGE_UP - Move the currently visible content up as if the
 * flip was a book, the other content is shown as the page below that.
 * @li ELM_FLIP_PAGE_DOWN - Move the currently visible content down as if the
 * flip was a book, the other content is shown as the page below that.
 *
 * @image html elm_flip.png
 * @image latex elm_flip.eps width=\textwidth
 *
 * @see elm_flip_go_to()
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_go(Evas_Object *obj, Elm_Flip_Mode mode);

/**
 * @brief Runs the flip animation to front or back.
 *
 * @param obj The flip object
 * @param front if @c EINA_TRUE, makes front visible, otherwise makes back.
 * @param mode The mode type
 *
 * Flips the front and back contents using the @p mode animation. This
 * effectively hides the currently visible content and shows the hidden one.
 *
 * There a number of possible animations to use for the flipping:
 * @li ELM_FLIP_ROTATE_X_CENTER_AXIS - Rotate the currently visible content
 * around a horizontal axis in the middle of its height, the other content
 * is shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_Y_CENTER_AXIS - Rotate the currently visible content
 * around a vertical axis in the middle of its width, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_XZ_CENTER_AXIS - Rotate the currently visible content
 * around a diagonal axis in the middle of its width, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_ROTATE_YZ_CENTER_AXIS - Rotate the currently visible content
 * around a diagonal axis in the middle of its height, the other content is
 * shown as the other side of the flip.
 * @li ELM_FLIP_CUBE_LEFT - Rotate the currently visible content to the left
 * as if the flip was a cube, the other content is show as the right face of
 * the cube.
 * @li ELM_FLIP_CUBE_RIGHT - Rotate the currently visible content to the
 * right as if the flip was a cube, the other content is show as the left
 * face of the cube.
 * @li ELM_FLIP_CUBE_UP - Rotate the currently visible content up as if the
 * flip was a cube, the other content is show as the bottom face of the cube.
 * @li ELM_FLIP_CUBE_DOWN - Rotate the currently visible content down as if
 * the flip was a cube, the other content is show as the upper face of the
 * cube.
 * @li ELM_FLIP_PAGE_LEFT - Move the currently visible content to the left as
 * if the flip was a book, the other content is shown as the page below that.
 * @li ELM_FLIP_PAGE_RIGHT - Move the currently visible content to the right
 * as if the flip was a book, the other content is shown as the page below
 * that.
 * @li ELM_FLIP_PAGE_UP - Move the currently visible content up as if the
 * flip was a book, the other content is shown as the page below that.
 * @li ELM_FLIP_PAGE_DOWN - Move the currently visible content down as if the
 * flip was a book, the other content is shown as the page below that.
 *
 * @image html elm_flip.png
 * @image latex elm_flip.eps width=\textwidth
 *
 * @since 1.7
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_go_to(Evas_Object *obj, Eina_Bool front, Elm_Flip_Mode mode);

/**
 * @brief Set the interactive flip mode
 *
 * @param obj The flip object
 * @param mode The interactive flip mode to use
 *
 * This sets if the flip should be interactive (allow user to click and
 * drag a side of the flip to reveal the back page and cause it to flip).
 * By default a flip is not interactive. You may also need to set which
 * sides of the flip are "active" for flipping and how much space they use
 * (a minimum of a finger size) with elm_flip_interaction_direction_enabled_set()
 * and elm_flip_interaction_direction_hitsize_set()
 *
 * The four available mode of interaction are:
 * @li ELM_FLIP_INTERACTION_NONE - No interaction is allowed
 * @li ELM_FLIP_INTERACTION_ROTATE - Interaction will cause rotate animation
 * @li ELM_FLIP_INTERACTION_CUBE - Interaction will cause cube animation
 * @li ELM_FLIP_INTERACTION_PAGE - Interaction will cause page animation
 *
 * @note ELM_FLIP_INTERACTION_ROTATE won't cause
 * ELM_FLIP_ROTATE_XZ_CENTER_AXIS or ELM_FLIP_ROTATE_YZ_CENTER_AXIS to
 * happen, those can only be achieved with elm_flip_go();
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_interaction_set(Evas_Object *obj, Elm_Flip_Interaction mode);

/**
 * @brief Get the interactive flip mode
 *
 * @param obj The flip object
 * @return The interactive flip mode
 *
 * Returns the interactive flip mode set by elm_flip_interaction_set()
 *
 * @ingroup Flip
 */
EAPI Elm_Flip_Interaction elm_flip_interaction_get(const Evas_Object *obj);

/**
 * @brief Set which directions of the flip respond to interactive flip
 *
 * @param obj The flip object
 * @param dir The direction to change
 * @param enabled If that direction is enabled or not
 *
 * By default all directions are disabled, so you may want to enable the
 * desired directions for flipping if you need interactive flipping. You must
 * call this function once for each direction that should be enabled.
 *
 * You can also set the appropriate hit area size by calling
 * @c elm_flip_interaction_direction_hitsize_set(). By default, a minimum
 * hit area will be created on the opposite edge of the flip.
 *
 * @see elm_flip_interaction_set()
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_interaction_direction_enabled_set(Evas_Object *obj, Elm_Flip_Direction dir, Eina_Bool enabled);

/**
 * @brief Get the enabled state of that flip direction
 *
 * @param obj The flip object
 * @param dir The direction to check
 * @return If that direction is enabled or not
 *
 * Get the enabled state set by elm_flip_interaction_direction_enabled_set()
 *
 * @see elm_flip_interaction_set()
 *
 * @ingroup Flip
 */
EAPI Eina_Bool            elm_flip_interaction_direction_enabled_get(Evas_Object *obj, Elm_Flip_Direction dir);

/**
 * @brief Set the amount of the flip that is sensitive to interactive flip
 *
 * @param obj The flip object
 * @param dir The hit area to set
 * @param hitsize The amount of that dimension (0.0 to 1.0) to use
 *
 * Set the amount of the flip that is sensitive to interactive flip, with 0
 * representing no area in the flip and 1 representing the entire flip. There
 * is however a consideration to be made in that the area will never be
 * smaller than the finger size set (as set in your Elementary configuration),
 * and dragging must always start from the opposite half of the flip (eg. right
 * half of the flip when dragging to the left).
 *
 * Note that the @c dir parameter is not actually related to the direction of
 * the drag, it only refers to the area in the flip where interaction can
 * occur (top, bottom, left, right).
 *
 * Negative values of @c hitsize will disable this hit area.
 *
 * @see elm_flip_interaction_set()
 *
 * @ingroup Flip
 */
EAPI void                 elm_flip_interaction_direction_hitsize_set(Evas_Object *obj, Elm_Flip_Direction dir, double hitsize);

/**
 * @brief Get the amount of the flip that is sensitive to interactive flip
 *
 * @param obj The flip object
 * @param dir The direction to check
 * @return The size set for that direction
 *
 * Returns the amount of sensitive area set by
 * elm_flip_interaction_direction_hitsize_set().
 *
 * @ingroup Flip
 */
EAPI double               elm_flip_interaction_direction_hitsize_get(Evas_Object *obj, Elm_Flip_Direction dir);
