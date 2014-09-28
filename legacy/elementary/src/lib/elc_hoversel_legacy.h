/**
 * @brief Add a new Hoversel object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Hoversel
 */
EAPI Evas_Object                 *elm_hoversel_add(Evas_Object *parent);

EAPI void                         elm_hoversel_item_icon_set(Elm_Object_Item *it, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type);

EAPI void                         elm_hoversel_item_icon_get(const Elm_Object_Item *it, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type);

#include "elm_hoversel.eo.legacy.h"
