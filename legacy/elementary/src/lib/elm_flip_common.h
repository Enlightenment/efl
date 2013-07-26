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
   ELM_FLIP_DIRECTION_UP = 0, /**< Allows interaction with the top of the widget */
   ELM_FLIP_DIRECTION_DOWN, /**< Allows interaction with the bottom of the widget */
   ELM_FLIP_DIRECTION_LEFT, /**< Allows interaction with the left portion of the widget */
   ELM_FLIP_DIRECTION_RIGHT /**< Allows interaction with the right portion of the widget */
} Elm_Flip_Direction;

