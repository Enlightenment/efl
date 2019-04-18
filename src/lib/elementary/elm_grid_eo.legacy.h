#ifndef _ELM_GRID_EO_LEGACY_H_
#define _ELM_GRID_EO_LEGACY_H_

#ifndef _ELM_GRID_EO_CLASS_TYPE
#define _ELM_GRID_EO_CLASS_TYPE

typedef Eo Elm_Grid;

#endif

#ifndef _ELM_GRID_EO_TYPES
#define _ELM_GRID_EO_TYPES


#endif

/**
 * @brief Set the virtual size of the grid
 *
 * @param[in] obj The object.
 * @param[in] w The virtual width of the grid
 * @param[in] h The virtual height of the grid
 *
 * @ingroup Elm_Grid_Group
 */
EAPI void elm_grid_size_set(Elm_Grid *obj, int w, int h);

/**
 * @brief Get the virtual size of the grid
 *
 * @param[in] obj The object.
 * @param[out] w The virtual width of the grid
 * @param[out] h The virtual height of the grid
 *
 * @ingroup Elm_Grid_Group
 */
EAPI void elm_grid_size_get(const Elm_Grid *obj, int *w, int *h);

/**
 * @brief Get the list of the children for the grid.
 *
 * @note This is a duplicate of the list kept by the grid internally. It's up
 * to the user to destroy it when it no longer needs it. It's possible to
 * remove objects from the grid when walking this list, but these removals
 * won't be reflected on it.
 *
 * @param[in] obj The object.
 *
 * @return List of children
 *
 * @ingroup Elm_Grid_Group
 */
EAPI Eina_List *elm_grid_children_get(const Elm_Grid *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Faster way to remove all child objects from a grid object.
 *
 * @param[in] obj The object.
 * @param[in] clear If @c true, it will delete just removed children
 *
 * @ingroup Elm_Grid_Group
 */
EAPI void elm_grid_clear(Elm_Grid *obj, Eina_Bool clear);

/**
 * @brief Unpack a child from a grid object
 *
 * @param[in] obj The object.
 * @param[in] subobj The child to unpack
 *
 * @ingroup Elm_Grid_Group
 */
EAPI void elm_grid_unpack(Elm_Grid *obj, Efl_Canvas_Object *subobj);

/**
 * @brief Pack child at given position and size
 *
 * @param[in] obj The object.
 * @param[in] subobj The child to pack.
 * @param[in] x The virtual x coord at which to pack it.
 * @param[in] y The virtual y coord at which to pack it.
 * @param[in] w The virtual width at which to pack it.
 * @param[in] h The virtual height at which to pack it.
 *
 * @ingroup Elm_Grid_Group
 */
EAPI void elm_grid_pack(Elm_Grid *obj, Efl_Canvas_Object *subobj, int x, int y, int w, int h);

#endif
