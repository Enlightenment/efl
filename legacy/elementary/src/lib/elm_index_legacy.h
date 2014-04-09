/**
 * Add a new index widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new index widget handle or @c NULL, on errors
 *
 * This function inserts a new index widget on the canvas.
 *
 * @ingroup Index
 */
EAPI Evas_Object          *elm_index_add(Evas_Object *parent);

/**
 * Get the letter (string) set on a given index widget item.
 *
 * @param item The index item handle
 * @return The letter string set on @p it
 *
 * @ingroup Index
 */
EAPI const char           *elm_index_item_letter_get(const Elm_Object_Item *item);

#include "elm_index.eo.legacy.h"