#ifndef ELM_WIN_EO_H
# define ELM_WIN_EO_H

/**
 * @ingroup Elm_Win
 *
 * @{
 */

#include "efl_ui_win.eo.h"

EAPI void elm_win_autodel_set(Evas_Object *obj, Eina_Bool autodel);
EAPI Eina_Bool elm_win_autodel_get(const Evas_Object *obj);
#if defined (EFL_EO_API_SUPPORT) && defined (EFL_BETA_API_SUPPORT)
/**
 * @brief Set the window's autodel state.
 *
 * When closing the window in any way outside of the program control, like
 * pressing the X button in the titlebar or using a command from the Window
 * Manager, a "delete,request" signal is emitted to indicate that this event
 * occurred and the developer can take any action, which may include, or not,
 * destroying the window object.
 *
 * When the @c autodel parameter is set, the window will be automatically
 * destroyed when this event occurs, after the signal is emitted. If @c autodel
 * is @c false, then the window will not be destroyed and is up to the program
 * to do so when it's required.
 *
 * @param[in] obj The object.
 * @param[in] autodel If @c true, the window will automatically delete itself
 * when closed.
 *
 * Note: This function is only available in C.
 *
 * @ingroup Efl_Ui_Win
 */
static inline void
efl_ui_win_autodel_set(Efl_Ui_Win *obj, Eina_Bool autodel)
{
   elm_win_autodel_set(obj, autodel);
}

/**
 * @brief Get the window's autodel state.
 *
 * @param[in] obj The object.
 *
 * @return If @c true, the window will automatically delete itself when closed.
 *
 * Note: This function is only available in C.
 *
 * @ingroup Efl_Ui_Win
 */
static inline Eina_Bool
efl_ui_win_autodel_get(const Efl_Ui_Win *obj)
{
   return elm_win_autodel_get(obj);
}

/**
 * @}
 */
#endif

#endif
