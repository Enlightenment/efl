#ifndef _EVAS_GRID_EO_LEGACY_H_
#define _EVAS_GRID_EO_LEGACY_H_

#ifndef _EVAS_GRID_EO_CLASS_TYPE
#define _EVAS_GRID_EO_CLASS_TYPE

typedef Eo Evas_Grid;

#endif

#ifndef _EVAS_GRID_EO_TYPES
#define _EVAS_GRID_EO_TYPES


#endif

/**
 * @brief Set the virtual resolution for the grid
 *
 * @param[in] obj The object.
 * @param[in] w The virtual horizontal size (resolution) in integer units.
 * @param[in] h The virtual vertical size (resolution) in integer units.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI void evas_object_grid_size_set(Evas_Grid *obj, int w, int h);

/**
 * @brief Get the current virtual resolution
 *
 * See also @ref evas_object_grid_size_set
 *
 * @param[in] obj The object.
 * @param[out] w The virtual horizontal size (resolution) in integer units.
 * @param[out] h The virtual vertical size (resolution) in integer units.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI void evas_object_grid_size_get(const Evas_Grid *obj, int *w, int *h);

/**
 * @brief Get the list of children for the grid.
 *
 * @note This is a duplicate of the list kept by the grid internally. It's up
 * to the user to destroy it when it no longer needs it. It's possible to
 * remove objects from the grid when walking this list, but these removals
 * won't be reflected on it.
 *
 * @param[in] obj The object.
 *
 * @return Iterator to grid children
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_List *evas_object_grid_children_get(const Evas_Grid *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get an accessor to get random access to the list of children for the
 * grid.
 *
 * @note Do not remove or delete objects while walking the list.
 *
 * @param[in] obj The object.
 *
 * @return Accessor to children list
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_Accessor *evas_object_grid_accessor_new(const Evas_Grid *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Faster way to remove all child objects from a grid object.
 *
 * @param[in] obj The object.
 * @param[in] clear If @c true, it will delete just removed children.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI void evas_object_grid_clear(Evas_Grid *obj, Eina_Bool clear);

/**
 * @brief Get an iterator to walk the list of children for the grid.
 *
 * @note Do not remove or delete objects while walking the list.
 *
 * @param[in] obj The object.
 *
 * @return Iterator
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_Iterator *evas_object_grid_iterator_new(const Evas_Grid *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Create a grid that is child of a given element parent.
 *
 * @ref evas_object_grid_add()
 *
 * @param[in] obj The object.
 *
 * @return New child object
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Efl_Canvas_Object *evas_object_grid_add_to(Evas_Grid *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Remove child from grid.
 *
 * @note removing a child will immediately call a walk over children in order
 * to recalculate numbers of columns and rows. If you plan to remove all
 * children, use evas_object_grid_clear() instead.
 *
 * @param[in] obj The object.
 * @param[in] child Child to be removed
 *
 * @return 1 on success, 0 on failure.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_Bool evas_object_grid_unpack(Evas_Grid *obj, Efl_Canvas_Object *child) EINA_ARG_NONNULL(2);

/**
 * @brief Get the pack options for a grid child
 *
 * Get the pack x, y, width and height in virtual coordinates set by
 * @ref evas_object_grid_pack.
 *
 * @param[in] obj The object.
 * @param[in] child The grid child to query for coordinates.
 * @param[out] x The pointer to where the x coordinate will be returned.
 * @param[out] y The pointer to where the y coordinate will be returned.
 * @param[out] w The pointer to where the width will be returned.
 * @param[out] h The pointer to where the height will be returned.
 *
 * @return 1 on success, 0 on failure.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_Bool evas_object_grid_pack_get(const Evas_Grid *obj, Efl_Canvas_Object *child, int *x, int *y, int *w, int *h);

/**
 * @brief Add a new child to a grid object.
 *
 * @param[in] obj The object.
 * @param[in] child The child object to add.
 * @param[in] x The virtual x coordinate of the child.
 * @param[in] y The virtual y coordinate of the child.
 * @param[in] w The virtual width of the child.
 * @param[in] h The virtual height of the child.
 *
 * @return 1 on success, 0 on failure.
 *
 * @since 1.1
 *
 * @ingroup Evas_Object_Grid_Group
 */
EAPI Eina_Bool evas_object_grid_pack(Evas_Grid *obj, Efl_Canvas_Object *child, int x, int y, int w, int h) EINA_ARG_NONNULL(2);

#endif
