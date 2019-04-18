/**
 * @brief Adds a hover object to @p parent
 *
 * @param parent The parent object
 * @return The hover object or NULL if one could not be created
 *
 * @ingroup Elm_Hover
 */
EAPI Evas_Object *elm_hover_add(Evas_Object *parent);

/**
 * @brief Sets the parent object for the hover.
 *
 * @param obj The hover object
 * @param parent The object to locate the hover over.
 *
 * This function will cause the hover to take up the entire space that the
 * parent object fills.
 *
 * @ingroup Elm_Hover
 */
EAPI void         elm_hover_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the parent object for the hover.
 *
 * @param obj The hover object
 * @return The parent object to locate the hover over.
 *
 * @see elm_hover_parent_set()
 *
 * @ingroup Elm_Hover
 */
EAPI Evas_Object *elm_hover_parent_get(const Evas_Object *obj);

#include "elm_hover_eo.legacy.h"