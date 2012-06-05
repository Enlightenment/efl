/**
 * @defgroup Hover Hover
 * @ingroup Elementary
 *
 * @image html hover_inheritance_tree.png
 * @image latex hover_inheritance_tree.eps
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
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for hover objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "clicked" - the user clicked the empty space in the hover to dismiss
 * @li @c "smart,changed" - a content object placed under the "smart"
 *                   policy was replaced to a new slot direction.
 *
 * Default content parts of the hover widget that you can use for are:
 * @li @c "left"
 * @li @c "top-left"
 * @li @c "top"
 * @li @c "top-right"
 * @li @c "right"
 * @li @c "bottom-right"
 * @li @c "bottom"
 * @li @c "bottom-left"
 * @li @c "middle"
 * @li @c "smart"
 *
 * @note These content parts indicates the direction that the content will be
 * displayed
 *
 * All directions may have contents at the same time, except for
 * "smart". This is a special placement hint and its use case
 * depends of the calculations coming from
 * elm_hover_best_content_location_get(). Its use is for cases when
 * one desires only one hover content, but with a dynamic special
 * placement within the hover area. The content's geometry, whenever
 * it changes, will be used to decide on a best location, not
 * extrapolating the hover's parent object view to show it in (still
 * being the hover's target determinant of its medium part -- move and
 * resize it to simulate finger sizes, for example). If one of the
 * directions other than "smart" are used, a previously content set
 * using it will be deleted, and vice-versa.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * See @ref tutorial_hover for more information.
 *
 * @{
 */

/**
 * @typedef Elm_Hover_Axis
 *
 * The orientation axis for the hover object
 */
typedef enum
{
   ELM_HOVER_AXIS_NONE, /**< ELM_HOVER_AXIS_NONE -- no preferred orientation */
   ELM_HOVER_AXIS_HORIZONTAL, /**< ELM_HOVER_AXIS_HORIZONTAL -- horizontal */
   ELM_HOVER_AXIS_VERTICAL, /**< ELM_HOVER_AXIS_VERTICAL -- vertical */
   ELM_HOVER_AXIS_BOTH /**< ELM_HOVER_AXIS_BOTH -- both */
} Elm_Hover_Axis;

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
 * @brief Gets the target object for the hover.
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
 * @brief Gets the parent object for the hover.
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
/**
 * @}
 */
