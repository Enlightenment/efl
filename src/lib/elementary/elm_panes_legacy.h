/**
 * Add a new panes widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new panes widget handle or @c NULL, on errors.
 *
 * This function inserts a new panes widget on the canvas.
 *
 * @ingroup Elm_Panes
 */
EAPI Evas_Object                 *elm_panes_add(Evas_Object *parent);

/**
 * @brief Set how to split and dispose each content.
 *
 * Use this function to change how your panes is to be disposed: vertically or
 * horizontally. Horizontal panes have "top" and "bottom" contents, vertical
 * panes have "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @param[in] horizontal Use @c true to make @c obj to split panes horizontally
 * ("top" and "bottom" contents). @c false to make it vertically ("left" and
 * "right" contents).
 *
 * @ingroup Elm_Panes
 */
EAPI void elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Set how to split and dispose each content.
 *
 * Use this function to change how your panes is to be disposed: vertically or
 * horizontally. Horizontal panes have "top" and "bottom" contents, vertical
 * panes have "left" and "right" contents.
 *
 * By default panes is in a vertical mode.
 *
 * @return Use @c true to make @c obj to split panes horizontally ("top" and
 * "bottom" contents). @c false to make it vertically ("left" and "right"
 * contents).
 *
 * @ingroup Elm_Panes
 */
EAPI Eina_Bool elm_panes_horizontal_get(const Evas_Object *obj);

#include "efl_ui_panes.eo.legacy.h"
