/**
 * @brief Add a separator object to @p parent
 *
 * @param parent The parent object
 *
 * @return The separator object, or NULL upon failure
 *
 * @ingroup Elm_Separator
 */
EAPI Evas_Object *elm_separator_add(Evas_Object *parent);

/**
 * @brief Set the horizontal mode of a separator object
 *
 * @param[in] horizontal If true, the separator is horizontal
 *
 * @ingroup Elm_Separator
 */
EAPI void elm_separator_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the horizontal mode of a separator object
 *
 * @return If true, the separator is horizontal
 *
 * @ingroup Elm_Separator
 */
EAPI Eina_Bool elm_separator_horizontal_get(const Evas_Object *obj);

#include "elm_separator.eo.legacy.h"
