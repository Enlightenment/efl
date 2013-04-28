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
EAPI void         elm_grid_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * Get the virtual size of the grid
 *
 * @param obj The grid object
 * @param w Pointer to integer to store the virtual width of the grid
 * @param h Pointer to integer to store the virtual height of the grid
 *
 * @ingroup Grid
 */
EAPI void         elm_grid_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

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
EAPI void         elm_grid_pack(Evas_Object *obj, Evas_Object *subobj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

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
 * @ingroup Grid
 */
EAPI void         elm_grid_pack_get(Evas_Object *subobj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * Get the list of the children for the grid.
 *
 * @param obj The grid object
 *
 * @note This is a duplicate of the list kept by the grid internally.
 * It's up to the user to destroy it when it no longer needs it.
 * It's possible to remove objects from the grid when walking this
 * list, but these removals won't be reflected on it.
 *
 * @ingroup Grid
 */
EAPI Eina_List *elm_grid_children_get(const Evas_Object *obj);


