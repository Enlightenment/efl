/**
 * Add a new map widget to the given parent Elementary (container) object.
 *
 * @param parent The parent object.
 * @return a new map widget handle or @c NULL, on errors.
 *
 * This function inserts a new map widget on the canvas.
 *
 * @ingroup Map
 */
EAPI Evas_Object          *elm_map_add(Evas_Object *parent);

/**
 * @internal
 *
 * @brief Requests a list of addresses corresponding to a given name.
 *
 * @since 1.8
 *
 * @remarks This is used if you want to search the address from a name.
 *
 * @param obj The map object
 * @param address The address
 * @param name_cb The callback function
 * @param data The user callback data
 *
 * @ingroup Map
 */
EAPI void                  elm_map_name_search(const Evas_Object *obj, const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

#include "elm_map.eo.legacy.h"
