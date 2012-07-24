/**
 * @defgroup Flip Flip
 * @ingroup Elementary
 *
 * @image html flip_inheritance_tree.png
 * @image latex flip_inheritance_tree.eps
 *
 * @image html img/widget/flip/preview-00.png
 * @image latex img/widget/flip/preview-00.eps
 *
 * This widget holds 2 content objects(Evas_Object): one on the front and one
 * on the back. It allows you to flip from front to back and vice-versa using
 * various animations.
 *
 * If either the front or back contents are not set the flip will treat that
 * as transparent. So if you wore to set the front content but not the back,
 * and then call elm_flip_go() you would see whatever is below the flip.
 *
 * For a list of supported animations see elm_flip_go().
 *
 * Signals that you can add callbacks for are:
 * "animate,begin" - when a flip animation was started
 * "animate,done" - when a flip animation is finished
 *
 * Default content parts of the flip widget that you can use for are:
 * @li "front" - A front content of the flip
 * @li "back" - A back content of the flip
 *
 * This widget inherits from @ref elm-container-class, so that the
 * functions meant to act on it will work for mapbuf objects:
 *
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * @ref tutorial_flip show how to use most of the API.
 *
 * @{
 */
typedef enum
{
   ELM_FLIP_ROTATE_Y_CENTER_AXIS,
   ELM_FLIP_ROTATE_X_CENTER_AXIS,
   ELM_FLIP_ROTATE_XZ_CENTER_AXIS,
   ELM_FLIP_ROTATE_YZ_CENTER_AXIS,
   ELM_FLIP_CUBE_LEFT,
   ELM_FLIP_CUBE_RIGHT,
   ELM_FLIP_CUBE_UP,
   ELM_FLIP_CUBE_DOWN,
   ELM_FLIP_PAGE_LEFT,
   ELM_FLIP_PAGE_RIGHT,
   ELM_FLIP_PAGE_UP,
   ELM_FLIP_PAGE_DOWN
} Elm_Flip_Mode;

typedef enum
{
   ELM_FLIP_INTERACTION_NONE,
   ELM_FLIP_INTERACTION_ROTATE,
   ELM_FLIP_INTERACTION_CUBE,
   ELM_FLIP_INTERACTION_PAGE
} Elm_Flip_Interaction;

typedef enum
{
   ELM_FLIP_DIRECTION_UP, /**< Allows interaction with the top of the widget */
   ELM_FLIP_DIRECTION_DOWN, /**< Allows interaction with the bottom of the widget */
   ELM_FLIP_DIRECTION_LEFT, /**< Allows interaction with the left portion of the widget */
   ELM_FLIP_DIRECTION_RIGHT /**< Allows interaction with the right portion of the widget */
} Elm_Flip_Direction;

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
Eina_Bool elm_flip_front_visible_get(const Evas_Object *obj);

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
 * Gets the enabled state set by elm_flip_interaction_direction_enabled_set()
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
 * @param dir The direction to modify
 * @param hitsize The amount of that dimension (0.0 to 1.0) to use
 *
 * Set the amount of the flip that is sensitive to interactive flip, with 0
 * representing no area in the flip and 1 representing the entire flip. There
 * is however a consideration to be made in that the area will never be
 * smaller than the finger size set(as set in your Elementary configuration).
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

/**
 * @}
 */
