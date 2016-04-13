/**
 * @brief Add a new Ctxpopup object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Evas_Object                 *elm_ctxpopup_add(Evas_Object *parent);

/**
 * @brief Change the ctxpopup's orientation to horizontal or vertical.
 *
 * @param obj Elm Ctxpopup object
 * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical.
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI void                         elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the value of current ctxpopup object's orientation.
 *
 * @param obj Elm Ctxpopup object
 * return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical.
 * See also @ref elm_ctxpopup_horizontal_set
 *
 * @ingroup Elm_Ctxpopup
 */
EAPI Eina_Bool                    elm_ctxpopup_horizontal_get(const Evas_Object *obj);

#include "elm_ctxpopup_item.eo.legacy.h"
#include "elm_ctxpopup.eo.legacy.h"
