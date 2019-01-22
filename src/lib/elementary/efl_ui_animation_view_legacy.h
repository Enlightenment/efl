typedef Eo Elm_Animation_View;


typedef enum _Elm_Animation_View_State
{
  ELM_ANIMATION_VIEW_STATE_NOT_READY = 0, /**< Animation is not ready to
                                              * play. (Probably, it didn't file
                                              * set yet or failed to read file.
                                              *
                                              * @since 1.22 */
  ELM_ANIMATION_VIEW_STATE_PLAY, /**< Animation is on playing. see @elm_animation_view_play
                                     *
                                     * @since 1.22 */
  ELM_ANIMATION_VIEW_STATE_PLAY_BACK, /**< Animation is on playing back
                                          * (rewinding). see @elm_animation_view_play_back
                                          *
                                          * @since 1.22 */
  ELM_ANIMATION_VIEW_STATE_PAUSE, /**< Animation has been paused. To continue
                                      * animation, call @elm_animation_view_resume. see @elm_animation_view_pause
                                      *
                                      * @since 1.22 */
  ELM_ANIMATION_VIEW_STATE_STOP /**< Animation view successfully loaded a
                                    * file then readied for playing. Otherwise
                                    * after finished animation or stopped
                                    * forcely by request. see @elm_animation_view_stop
                                    *
                                    * @since 1.22 */
}Elm_Animation_View_State;

/**
 * Add a new animation view widget to the parent's canvas
 *
 * @param parent The parent object
 * @return The new animation view object or @c NULL if it failed to create.
 *
 * @ingroup Elm_Animation_View_Group
 *
 * @since 1.22
 */
EAPI Elm_Animation_View     *elm_animation_view_add(Evas_Object *parent);

/**
 *
 * Set the source file from where an vector object must fetch the real
 * vector data (it may be one of json, svg, eet files).
 *
 * If the file supports multiple data stored in it (as Eet files do),
 * you can specify the key to be used as the index of the vector in
 * this file.
 *
 * @param[in] file The vector file path.
 * @param[in] key The vector key in @p file (if its an Eet one), or @c
NULL, otherwise.
 *
 * @return @c EINA_TRUE if it's succeed to read file, @c EINA_FALSE otherwise.
 *
 * @ingroup Elm_Animation_View_Group
 *
 * @since 1.22
*/
EAPI Eina_Bool        elm_animation_view_file_set(Elm_Animation_View *obj, const char *file, const char *key);

/**
 * @brief Get current animation view state.
 *
 * @return Current animation view state
 *
 * @see Elm_Animation_View_State
 *
 * @ingroup Elm_Animation_View_Group
 *
 * @since 1.22
 */
EAPI Elm_Animation_View_State elm_animation_view_state_get(Elm_Animation_View *obj);

#include "efl_ui_animation_view.eo.legacy.h"
