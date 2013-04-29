/**
 * @brief Type that controls when scrollbars should appear.
 *
 * @see elm_scroller_policy_set()
 */
typedef enum
{
   ELM_SCROLLER_POLICY_AUTO = 0, /**< Show scrollbars as needed */
   ELM_SCROLLER_POLICY_ON, /**< Always show scrollbars */
   ELM_SCROLLER_POLICY_OFF, /**< Never show scrollbars */
   ELM_SCROLLER_POLICY_LAST
} Elm_Scroller_Policy;

/**
 * @brief Type that controls how the content is scrolled.
 *
 * @see elm_scroller_single_direction_set()
 */
typedef enum
{
   ELM_SCROLLER_SINGLE_DIRECTION_NONE = 0, /**< Scroll every direction */
   ELM_SCROLLER_SINGLE_DIRECTION_SOFT, /**< Scroll single direction if the direction is certain*/
   ELM_SCROLLER_SINGLE_DIRECTION_HARD, /**< Scroll only single direction */
   ELM_SCROLLER_SINGLE_DIRECTION_LAST
} Elm_Scroller_Single_Direction;


