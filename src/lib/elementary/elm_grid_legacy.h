/**
 * Add a new grid to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Grid
 */
EAPI Evas_Object *elm_grid_add(Evas_Object *parent);

/**
 * Set packing of an existing child at to position and size
 *
 * @param subobj The child to set packing of
 * @param x The virtual x coord at which to pack it
 * @param y The virtual y coord at which to pack it
 * @param w The virtual width at which to pack it
 * @param h The virtual height at which to pack it
 *
 * @ingroup Elm_Grid
 */
EAPI void         elm_grid_pack_set(Evas_Object *subobj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * Get packing of a child
 *
 * @param subobj The child to query
 * @param x Pointer to integer to store the virtual x coord
 * @param y Pointer to integer to store the virtual y coord
 * @param w Pointer to integer to store the virtual width
 * @param h Pointer to integer to store the virtual height
 *
 * @ingroup Elm_Grid
 */
EAPI void         elm_grid_pack_get(Evas_Object *subobj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

#include "elm_grid_eo.legacy.h"