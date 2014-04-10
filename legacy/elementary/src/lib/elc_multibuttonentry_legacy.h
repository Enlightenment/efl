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

/**
 * Set the selected state of an item
 *
 * @param it The item
 * @param selected if it's @c EINA_TRUE, select the item otherwise, unselect the item
 *
 * @ingroup Multibuttonentry
 */
EAPI void                       elm_multibuttonentry_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * Get the selected state of an item
 *
 * @param it The item
 * @return @c EINA_TRUE if the item is selected, @c EINA_FALSE otherwise.
 *
 * @ingroup Multibuttonentry
 */
EAPI Eina_Bool elm_multibuttonentry_item_selected_get(const Elm_Object_Item *it);

/**
 * Get the previous item in the multibuttonentry
 *
 * @param it The item
 * @return The item before the item @p it
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it);

/**
 * Get the next item in the multibuttonentry
 *
 * @param it The item
 * @return The item after the item @p it
 *
 * @ingroup Multibuttonentry
 */
EAPI Elm_Object_Item *elm_multibuttonentry_item_next_get(const Elm_Object_Item *it);

#include "elc_multibuttonentry.eo.legacy.h"