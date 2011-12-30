/**
 * @defgroup Grid Grid
 *
 * The grid is a grid layout widget that lays out a series of children as a
 * fixed "grid" of widgets using a given percentage of the grid width and
 * height each using the child object.
 *
 * The Grid uses a "Virtual resolution" that is stretched to fill the grid
 * widgets size itself. The default is 100 x 100, so that means the
 * position and sizes of children will effectively be percentages (0 to 100)
 * of the width or height of the grid widget
 *
 * @{
 */

/**
 * Add a new grid to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Grid
 */
EAPI Evas_Object *elm_grid_add(Evas_Object *parent);

/**
 * Set the virtual size of the grid
 *
 * @param obj The grid object
 * @param w The virtual width of the grid
 * @param h The virtual height of the grid
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_size_set(Evas_Object *obj, int w, int h);

/**
 * Get the virtual size of the grid
 *
 * @param obj The grid object
 * @param w Pointer to integer to store the virtual width of the grid
 * @param h Pointer to integer to store the virtual height of the grid
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_size_get(Evas_Object *obj, int *w, int *h);

/**
 * Pack child at given position and size
 *
 * @param obj The grid object
 * @param subobj The child to pack
 * @param x The virtual x coord at which to pack it
 * @param y The virtual y coord at which to pack it
 * @param w The virtual width at which to pack it
 * @param h The virtual height at which to pack it
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h);

/**
 * Unpack a child from a grid object
 *
 * @param obj The grid object
 * @param subobj The child to unpack
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_unpack(Evas_Object *obj, Evas_Object *subobj);

/**
 * Faster way to remove all child objects from a grid object.
 *
 * @param obj The grid object
 * @param clear If true, it will delete just removed children
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_clear(Evas_Object *obj, Eina_Bool clear);

/**
 * Set packing of an existing child at to position and size
 *
 * @param subobj The child to set packing of
 * @param x The virtual x coord at which to pack it
 * @param y The virtual y coord at which to pack it
 * @param w The virtual width at which to pack it
 * @param h The virtual height at which to pack it
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_pack_set(Evas_Object *subobj, int x, int y, int w, int h);

/**
 * get packing of a child
 *
 * @param subobj The child to query
 * @param x Pointer to integer to store the virtual x coord
 * @param y Pointer to integer to store the virtual y coord
 * @param w Pointer to integer to store the virtual width
 * @param h Pointer to integer to store the virtual height
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_pack_get(Evas_Object *subobj, int *x, int *y, int *w, int *h);

/**
 * @}
 */
