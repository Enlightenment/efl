#ifndef _ELM_HOVER_EO_LEGACY_H_
#define _ELM_HOVER_EO_LEGACY_H_

#ifndef _ELM_HOVER_EO_CLASS_TYPE
#define _ELM_HOVER_EO_CLASS_TYPE

typedef Eo Elm_Hover;

#endif

#ifndef _ELM_HOVER_EO_TYPES
#define _ELM_HOVER_EO_TYPES

/** The orientation axis for the hover object
 *
 * @ingroup Elm_Hover
 */
typedef enum
{
  ELM_HOVER_AXIS_NONE = 0, /**< ELM_HOVER_AXIS_NONE -- no preferred orientation.
                            */
  ELM_HOVER_AXIS_HORIZONTAL, /**< ELM_HOVER_AXIS_HORIZONTAL -- horizontal. */
  ELM_HOVER_AXIS_VERTICAL, /**< ELM_HOVER_AXIS_VERTICAL -- vertical. */
  ELM_HOVER_AXIS_BOTH /**< ELM_HOVER_AXIS_BOTH -- both. */
} Elm_Hover_Axis;


#endif

/**
 * @brief Sets the target object for the hover.
 *
 * This function will cause the hover to be centered on the target object.
 *
 * @param[in] obj The object.
 * @param[in] target The target object.
 *
 * @ingroup Elm_Hover_Group
 */
EAPI void elm_hover_target_set(Elm_Hover *obj, Efl_Canvas_Object *target);

/**
 * @brief Get the target object for the hover.
 *
 * @param[in] obj The object.
 *
 * @return The target object.
 *
 * @ingroup Elm_Hover_Group
 */
EAPI Efl_Canvas_Object *elm_hover_target_get(const Elm_Hover *obj);

/**
 * @brief Returns the best swallow location for content in the hover.
 *
 * Best is defined here as the location at which there is the most available
 * space.
 *
 * @c pref_axis may be either #ELM_HOVER_AXIS_NONE (for no preferred
 * orientation), #ELM_HOVER_AXIS_HORIZONTAL, #ELM_HOVER_AXIS_VERTICAL or
 * #ELM_HOVER_AXIS_BOTH.
 *
 * If #ELM_HOVER_AXIS_HORIZONTAL is chosen the returned position will
 * necessarily be along the horizontal axis("left" or "right"). If
 * #ELM_HOVER_AXIS_VERTICAL is chosen the returned position will necessarily be
 * along the vertical axis("top" or "bottom"). Choosing #ELM_HOVER_AXIS_BOTH or
 * #ELM_HOVER_AXIS_NONE has the same effect and the returned position may be in
 * either axis.
 *
 * See also @ref elm_object_part_content_set.
 *
 * @param[in] obj The object.
 * @param[in] pref_axis The preferred orientation axis for the hover object to
 * use
 *
 * @return Swallow location
 *
 * @ingroup Elm_Hover_Group
 */
EAPI const char *elm_hover_best_content_location_get(const Elm_Hover *obj, Elm_Hover_Axis pref_axis);

/** Dismiss a hover object
 *
 * @ingroup Elm_Hover_Group
 */
EAPI void elm_hover_dismiss(Elm_Hover *obj);

#endif
