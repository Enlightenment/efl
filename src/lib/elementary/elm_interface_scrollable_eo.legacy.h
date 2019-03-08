#ifndef _ELM_INTERFACE_SCROLLABLE_EO_LEGACY_H_
#define _ELM_INTERFACE_SCROLLABLE_EO_LEGACY_H_

#ifndef _ELM_INTERFACE_SCROLLABLE_EO_CLASS_TYPE
#define _ELM_INTERFACE_SCROLLABLE_EO_CLASS_TYPE

typedef Eo Elm_Interface_Scrollable;

#endif

#ifndef _ELM_INTERFACE_SCROLLABLE_EO_TYPES
#define _ELM_INTERFACE_SCROLLABLE_EO_TYPES

/**
 * @brief Type that controls when scrollbars should appear.
 *
 * See also @ref Elm.Interface_Scrollable.policy.set.
 *
 * @ingroup Elm_Scroller
 */
typedef enum
{
  ELM_SCROLLER_POLICY_AUTO = 0, /**< Show scrollbars as needed */
  ELM_SCROLLER_POLICY_ON, /**< Always show scrollbars */
  ELM_SCROLLER_POLICY_OFF, /**< Never show scrollbars */
  ELM_SCROLLER_POLICY_LAST /**< Sentinel value to indicate last enum field
                            * during iteration */
} Elm_Scroller_Policy;

/**
 * @brief Type that controls how the content is scrolled.
 *
 * See also @ref Elm.Interface_Scrollable.single_direction.set.
 *
 * @ingroup Elm_Scroller
 */
typedef enum
{
  ELM_SCROLLER_SINGLE_DIRECTION_NONE = 0, /**< Scroll every direction */
  ELM_SCROLLER_SINGLE_DIRECTION_SOFT, /**< Scroll single direction if the
                                       * direction is certain */
  ELM_SCROLLER_SINGLE_DIRECTION_HARD, /**< Scroll only single direction */
  ELM_SCROLLER_SINGLE_DIRECTION_LAST /**< Sentinel value to indicate last enum
                                      * field during iteration */
} Elm_Scroller_Single_Direction;


#endif

#endif
