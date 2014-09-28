/**
 * @brief Add a new multibuttonentry to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 *
 * @ingroup Multibuttonentry
 */
EAPI Evas_Object               *elm_multibuttonentry_add(Evas_Object *parent);

EAPI void                       elm_multibuttonentry_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

EAPI Eina_Bool elm_multibuttonentry_item_selected_get(const Elm_Object_Item *it);

EAPI Elm_Object_Item *elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it);

EAPI Elm_Object_Item *elm_multibuttonentry_item_next_get(const Elm_Object_Item *it);

#include "elm_multibuttonentry.eo.legacy.h"
