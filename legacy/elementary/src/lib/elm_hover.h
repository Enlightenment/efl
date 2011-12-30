/**
 * @defgroup Hover Hover
 *
 * @image html img/widget/hover/preview-00.png
 * @image latex img/widget/hover/preview-00.eps
 *
 * A Hover object will hover over its @p parent object at the @p target
 * location. Anything in the background will be given a darker coloring to
 * indicate that the hover object is on top (at the default theme). When the
 * hover is clicked it is dismissed(hidden), if the contents of the hover are
 * clicked that @b doesn't cause the hover to be dismissed.
 *
 * A Hover object has two parents. One parent that owns it during creation
 * and the other parent being the one over which the hover object spans.
 *
 *
 * @note The hover object will take up the entire space of @p target
 * object.
 *
 * Elementary has the following styles for the hover widget:
 * @li default
 * @li popout
 * @li menu
 * @li hoversel_vertical
 *
 * The following are the available position for content:
 * @li left
 * @li top-left
 * @li top
 * @li top-right
 * @li right
 * @li bottom-right
 * @li bottom
 * @li bottom-left
 * @li middle
 * @li smart
 *
 * Signals that you can add callbacks for are:
 * @li "clicked" - the user clicked the empty space in the hover to dismiss
 * @li "smart,changed" - a content object placed under the "smart"
 *                   policy was replaced to a new slot direction.
 *
 * See @ref tutorial_hover for more information.
 *
 * @{
 */
typedef enum _Elm_Hover_Axis
{
   ELM_HOVER_AXIS_NONE, /**< ELM_HOVER_AXIS_NONE -- no prefered orientation */
   ELM_HOVER_AXIS_HORIZONTAL, /**< ELM_HOVER_AXIS_HORIZONTAL -- horizontal */
   ELM_HOVER_AXIS_VERTICAL, /**< ELM_HOVER_AXIS_VERTICAL -- vertical */
   ELM_HOVER_AXIS_BOTH /**< ELM_HOVER_AXIS_BOTH -- both */
} Elm_Hover_Axis;

/**
 * @brief Adds a hover object to @p parent
 *
 * @param parent The parent object
 * @return The hover object or NULL if one could not be created
 */
EAPI Evas_Object *
                  elm_hover_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * @brief Sets the target object for the hover.
 *
 * @param obj The hover object
 * @param target The object to center the hover onto.
 *
 * This function will cause the hover to be centered on the target object.
 */
EAPI void         elm_hover_target_set(Evas_Object *obj, Evas_Object *target) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the target object for the hover.
 *
 * @param obj The hover object
 * @return The target object for the hover.
 *
 * @see elm_hover_target_set()
 */
EAPI Evas_Object *elm_hover_target_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the parent object for the hover.
 *
 * @param obj The hover object
 * @param parent The object to locate the hover over.
 *
 * This function will cause the hover to take up the entire space that the
 * parent object fills.
 */
EAPI void         elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the parent object for the hover.
 *
 * @param obj The hover object
 * @return The parent object to locate the hover over.
 *
 * @see elm_hover_parent_set()
 */
EAPI Evas_Object *elm_hover_parent_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Sets the content of the hover object and the direction in which it
 * will pop out.
 *
 * @param obj The hover object
 * @param swallow The direction that the object will be displayed
 * at. Accepted values are "left", "top-left", "top", "top-right",
 * "right", "bottom-right", "bottom", "bottom-left", "middle" and
 * "smart".
 * @param content The content to place at @p swallow
 *
 * Once the content object is set for a given direction, a previously
 * set one (on the same direction) will be deleted. If you want to
 * keep that old content object, use the elm_hover_content_unset()
 * function.
 *
 * All directions may have contents at the same time, except for
 * "smart". This is a special placement hint and its use case
 * independs of the calculations coming from
 * elm_hover_best_content_location_get(). Its use is for cases when
 * one desires only one hover content, but with a dynamic special
 * placement within the hover area. The content's geometry, whenever
 * it changes, will be used to decide on a best location, not
 * extrapolating the hover's parent object view to show it in (still
 * being the hover's target determinant of its medium part -- move and
 * resize it to simulate finger sizes, for example). If one of the
 * directions other than "smart" are used, a previously content set
 * using it will be deleted, and vice-versa.
 */
EAPI void         elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content) EINA_ARG_NONNULL(1);

/**
 * @brief Get the content of the hover object, in a given direction.
 *
 * Return the content object which was set for this widget in the
 * @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used
 *
 * @see elm_hover_content_set()
 */
EAPI Evas_Object *elm_hover_content_get(const Evas_Object *obj, const char *swallow) EINA_ARG_NONNULL(1);

/**
 * @brief Unset the content of the hover object, in a given direction.
 *
 * Unparent and return the content object set at @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used.
 *
 * @see elm_hover_content_set()
 */
EAPI Evas_Object *elm_hover_content_unset(Evas_Object *obj, const char *swallow) EINA_ARG_NONNULL(1);

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
 * - @c ELM_HOVER_AXIS_NONE -- no prefered orientation
 * - @c ELM_HOVER_AXIS_HORIZONTAL -- horizontal
 * - @c ELM_HOVER_AXIS_VERTICAL -- vertical
 * - @c ELM_HOVER_AXIS_BOTH -- both
 *
 * If ELM_HOVER_AXIS_HORIZONTAL is choosen the returned position will
 * nescessarily be along the horizontal axis("left" or "right"). If
 * ELM_HOVER_AXIS_VERTICAL is choosen the returned position will nescessarily
 * be along the vertical axis("top" or "bottom"). Chossing
 * ELM_HOVER_AXIS_BOTH or ELM_HOVER_AXIS_NONE has the same effect and the
 * returned position may be in either axis.
 *
 * @see elm_hover_content_set()
 */
EAPI const char  *elm_hover_best_content_location_get(const Evas_Object *obj, Elm_Hover_Axis pref_axis) EINA_ARG_NONNULL(1);

/**
 * @}
 */
