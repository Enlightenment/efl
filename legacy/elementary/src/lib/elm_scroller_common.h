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

/**
 * @brief Type that blocks the scroll movement in one or more direction.
 *
 * @see elm_scroller_movement_block()
 *
 * @since 1.8
 */
typedef enum
{
    ELM_SCROLLER_MOVEMENT_NO_BLOCK = 1 << 0, /**< Do not block movements */
    ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL = 1 << 1, /**< Block vertical movements */
    ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL = 1 << 2 /**< Block horizontal movements */
} Elm_Scroller_Movement_Block;


