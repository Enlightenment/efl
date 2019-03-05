#include "efl_ui_flip_eo.legacy.h"

typedef Eo Elm_Flip;

typedef Efl_Ui_Flip_Interaction       Elm_Flip_Interaction;

#define ELM_FLIP_INTERACTION_NONE     EFL_UI_FLIP_INTERACTION_NONE
#define ELM_FLIP_INTERACTION_ROTATE   EFL_UI_FLIP_INTERACTION_ROTATE
#define ELM_FLIP_INTERACTION_CUBE     EFL_UI_FLIP_INTERACTION_CUBE
#define ELM_FLIP_INTERACTION_PAGE     EFL_UI_FLIP_INTERACTION_PAGE


typedef Efl_Ui_Flip_Mode                  Elm_Flip_Mode;

#define ELM_FLIP_ROTATE_Y_CENTER_AXIS     EFL_UI_FLIP_MODE_ROTATE_Y_CENTER_AXIS
#define ELM_FLIP_ROTATE_X_CENTER_AXIS     EFL_UI_FLIP_MODE_ROTATE_X_CENTER_AXIS
#define ELM_FLIP_ROTATE_XZ_CENTER_AXIS    EFL_UI_FLIP_MODE_ROTATE_XZ_CENTER_AXIS
#define ELM_FLIP_ROTATE_YZ_CENTER_AXIS    EFL_UI_FLIP_MODE_ROTATE_YZ_CENTER_AXIS
#define ELM_FLIP_CUBE_LEFT                EFL_UI_FLIP_MODE_CUBE_LEFT
#define ELM_FLIP_CUBE_RIGHT               EFL_UI_FLIP_MODE_CUBE_RIGHT
#define ELM_FLIP_CUBE_UP                  EFL_UI_FLIP_MODE_CUBE_UP
#define ELM_FLIP_CUBE_DOWN                EFL_UI_FLIP_MODE_CUBE_DOWN
#define ELM_FLIP_PAGE_LEFT                EFL_UI_FLIP_MODE_PAGE_LEFT
#define ELM_FLIP_PAGE_RIGHT               EFL_UI_FLIP_MODE_PAGE_RIGHT
#define ELM_FLIP_PAGE_UP                  EFL_UI_FLIP_MODE_PAGE_UP
#define ELM_FLIP_PAGE_DOWN                EFL_UI_FLIP_MODE_PAGE_DOWN
#define ELM_FLIP_CROSS_FADE               EFL_UI_FLIP_MODE_CROSS_FADE

typedef enum
{
  ELM_FLIP_DIRECTION_UP = 0, /** Allows interaction with the top of the widget
                              */
  ELM_FLIP_DIRECTION_DOWN, /** Allows interaction with the bottom of the widget
                            */
  ELM_FLIP_DIRECTION_LEFT, /** Allows interaction with the left portion of the
                            * widget */
  ELM_FLIP_DIRECTION_RIGHT /** Allows interaction with the right portion of the
                            * widget */
} Elm_Flip_Direction;

/**
 * @brief Set the amount of the flip that is sensitive to interactive flip.
 *
 * Set the amount of the flip that is sensitive to interactive flip, with 0
 * representing no area in the flip and 1 representing the entire flip. There
 * is however a consideration to be made in that the area will never be smaller
 * than the finger size set (as set in your Elementary configuration), and
 * dragging must always start from the opposite half of the flip (eg. right
 * half of the flip when dragging to the left).
 *
 * @note The @c dir parameter is not actually related to the direction of the
 * drag, it only refers to the area in the flip where interaction can occur
 * (top, bottom, left, right).
 *
 * Negative values of @c hitsize will disable this hit area.
 *
 * See also @ref elm_flip_interaction_set.
 *
 * @param[in] hitsize The amount of that dimension (0.0 to 1.0) to use.
 *
 * @ingroup Efl_Ui_Flip
 */
EAPI void elm_flip_interaction_direction_hitsize_set(Efl_Ui_Flip *obj, Elm_Flip_Direction dir, double hitsize);

/**
 * @brief Get the amount of the flip that is sensitive to interactive flip.
 *
 * @param[in] dir The direction to check.
 *
 * @return The size set for that direction.
 *
 * @ingroup Efl_Ui_Flip
 */
EAPI double elm_flip_interaction_direction_hitsize_get(Efl_Ui_Flip *obj, Elm_Flip_Direction dir);

/**
 * @brief Set which directions of the flip respond to interactive flip
 *
 * By default all directions are disabled, so you may want to enable the
 * desired directions for flipping if you need interactive flipping. You must
 * call this function once for each direction that should be enabled.
 *
 * You can also set the appropriate hit area size by calling
 * @ref efl_ui_flip_interaction_direction_hitsize_set. By default, a minimum
 * hit area will be created on the opposite edge of the flip.
 *
 * @param[in] enabled If that direction is enabled or not.
 *
 * @ingroup Efl_Ui_Flip
 */
EAPI void elm_flip_interaction_direction_enabled_set(Efl_Ui_Flip *obj, Elm_Flip_Direction dir, Eina_Bool enabled);

/**
 * @brief Get the enabled state of that flip direction.
 *
 * @param[in] dir The direction to check.
 *
 * @return If that direction is enabled or not.
 *
 * @ingroup Efl_Ui_Flip
 */
EAPI Eina_Bool elm_flip_interaction_direction_enabled_get(Efl_Ui_Flip *obj, Elm_Flip_Direction dir);

/**
 * @brief Add a new flip to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Flip
 */
EAPI Evas_Object *elm_flip_add(Evas_Object *parent);

/**
 * @brief Set flip perspective
 *
 * @param obj The flip object
 * @param foc The coordinate to set the focus on
 * @param x The X coordinate
 * @param y The Y coordinate
 *
 * @warning This function currently does nothing.
 *
 * @ingroup Elm_Flip
 */
EAPI void                 elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc, Evas_Coord x, Evas_Coord y);
