/**
 * A position for indicators, magnets, and enabled items.
 *
 * @ingroup Actionslider
 */
typedef enum
{
   ELM_ACTIONSLIDER_NONE = 0, /**< no position is set */
   ELM_ACTIONSLIDER_LEFT = 1 << 0, /**< left position */
   ELM_ACTIONSLIDER_CENTER = 1 << 1, /**< center position */
   ELM_ACTIONSLIDER_RIGHT = 1 << 2, /**< right position */
   ELM_ACTIONSLIDER_ALL = (1 << 3) - 1 /**< all positions for left/center/right */
} Elm_Actionslider_Pos;
