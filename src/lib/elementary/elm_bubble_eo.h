#ifndef _ELM_BUBBLE_EO_H_
#define _ELM_BUBBLE_EO_H_

#ifndef _ELM_BUBBLE_EO_CLASS_TYPE
#define _ELM_BUBBLE_EO_CLASS_TYPE

typedef Eo Elm_Bubble;

#endif

#ifndef _ELM_BUBBLE_EO_TYPES
#define _ELM_BUBBLE_EO_TYPES

/**
 * @brief Defines the corner values for a bubble.
 *
 * The corner will be used to determine where the arrow of the bubble points
 * to.
 *
 * @ingroup Elm_Bubble
 */
typedef enum
{
  ELM_BUBBLE_POS_INVALID = -1 /* +1 */, /**< Invalid corner. */
  ELM_BUBBLE_POS_TOP_LEFT, /**< The arrow of the bubble points to the top left
                            * corner. */
  ELM_BUBBLE_POS_TOP_RIGHT, /**< The arrow of the bubble points to the top right
                             * corner. */
  ELM_BUBBLE_POS_BOTTOM_LEFT, /**< The arrow of the bubble points to the bottom
                               * left corner. */
  ELM_BUBBLE_POS_BOTTOM_RIGHT /**< The arrow of the bubble points to the bottom
                               * right corner. */
} Elm_Bubble_Pos;


#endif
/** Speech bubble widget used in messaging applications
 *
 * @ingroup Elm_Bubble
 */
#define ELM_BUBBLE_CLASS elm_bubble_class_get()

EWAPI const Efl_Class *elm_bubble_class_get(void);

/**
 * @brief The corner of the bubble
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown. See @ref Elm_Bubble_Pos
 *
 * @param[in] obj The object.
 * @param[in] pos The given corner for the bubble.
 *
 * @ingroup Elm_Bubble
 */
EOAPI void elm_obj_bubble_pos_set(Eo *obj, Elm_Bubble_Pos pos);

/**
 * @brief The corner of the bubble
 *
 * This function sets the corner of the bubble. The corner will be used to
 * determine where the arrow in the frame points to and where label, icon and
 * info are shown. See @ref Elm_Bubble_Pos
 *
 * @param[in] obj The object.
 *
 * @return The given corner for the bubble.
 *
 * @ingroup Elm_Bubble
 */
EOAPI Elm_Bubble_Pos elm_obj_bubble_pos_get(const Eo *obj);

#endif
