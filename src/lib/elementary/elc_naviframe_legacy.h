/**
 * @brief Set an item style.
 *
 * @param[in] obj The object.
 * @param[in] style The current item style name. @c null would be default
 *
 * @ingroup Elm_Naviframe_Item
 */
EAPI void elm_naviframe_item_style_set(Elm_Object_Item *obj, const char *style);

/**
 * @brief Get an item style.
 *
 * @param[in] obj The object.
 *
 * @return The current item style name. @c null would be default
 *
 * @ingroup Elm_Naviframe_Item
 */
EAPI const char *elm_naviframe_item_style_get(const Elm_Object_Item *obj);

#include "elm_naviframe_item_eo.legacy.h"
#include "elm_naviframe_eo.legacy.h"
