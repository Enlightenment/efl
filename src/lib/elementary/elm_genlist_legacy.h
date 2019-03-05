/**
 * Add a new genlist widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new genlist widget handle or @c NULL, on errors
 *
 * This function inserts a new genlist widget on the canvas.
 *
 * @see elm_genlist_item_append()
 * @see elm_object_item_del()
 * @see elm_genlist_clear()
 *
 * @ingroup Elm_Genlist
 */
EAPI Evas_Object                  *elm_genlist_add(Evas_Object *parent);

/**
 * Get the nth item, in a given genlist widget, placed at position @p nth, in
 * its internal items list
 *
 * @param obj The genlist object
 * @param nth The number of the item to grab (0 being the first)
 *
 * @return The item stored in @p obj at position @p nth or @c NULL, if there's
 * no item with that index (and on errors)
 *
 * @ingroup Genilst
 * @since 1.8
 */
EAPI Elm_Object_Item *
elm_genlist_nth_item_get(const Evas_Object *obj, unsigned int nth);

#include "elm_genlist_item_eo.legacy.h"
#include "elm_genlist_eo.legacy.h"
