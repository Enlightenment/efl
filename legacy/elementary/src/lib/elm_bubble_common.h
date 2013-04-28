/**
 * Defines the corner values for a bubble.
 *
 * The corner will be used to determine where the arrow of the
 * bubble points to.
 */
typedef enum
{
  ELM_BUBBLE_POS_INVALID = -1, /**< invalid corner */
  ELM_BUBBLE_POS_TOP_LEFT, /**< the arrow of the bubble points to the top left corner. */
  ELM_BUBBLE_POS_TOP_RIGHT, /**< the arrow of the bubble points to the top right corner. */
  ELM_BUBBLE_POS_BOTTOM_LEFT, /**< the arrow of the bubble points to the bottom left corner. */
  ELM_BUBBLE_POS_BOTTOM_RIGHT, /**< the arrow of the bubble points to the bottom right corner. */
} Elm_Bubble_Pos;

