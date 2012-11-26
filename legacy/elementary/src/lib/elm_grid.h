/**
 * @defgroup Grid Grid
 * @ingroup Elementary
 *
 * @image html grid_inheritance_tree.png
 * @image latex grid_inheritance_tree.eps
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


#define ELM_OBJ_GRID_CLASS elm_obj_grid_class_get()

const Eo_Class *elm_obj_grid_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_GRID_BASE_ID;

enum
{

   ELM_OBJ_GRID_SUB_ID_SIZE_SET,
   ELM_OBJ_GRID_SUB_ID_SIZE_GET,
   ELM_OBJ_GRID_SUB_ID_PACK,
   ELM_OBJ_GRID_SUB_ID_UNPACK,
   ELM_OBJ_GRID_SUB_ID_CLEAR,
   ELM_OBJ_GRID_SUB_ID_CHILDREN_GET,
   ELM_OBJ_GRID_SUB_ID_LAST
};

#define ELM_OBJ_GRID_ID(sub_id) (ELM_OBJ_GRID_BASE_ID + sub_id)


/**
 * @def elm_obj_grid_size_set
 * @since 1.8
 *
 * Set the virtual size of the grid
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_grid_size_set
 */
#define elm_obj_grid_size_set(w, h) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_obj_grid_size_get
 * @since 1.8
 *
 * Get the virtual size of the grid
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_grid_size_get
 */
#define elm_obj_grid_size_get(w, h) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_obj_grid_pack
 * @since 1.8
 *
 * Pack child at given position and size
 *
 * @param[in] subobj
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 *
 * @see elm_grid_pack
 */
#define elm_obj_grid_pack(subobj, x, y, w, h) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_PACK), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_obj_grid_unpack
 * @since 1.8
 *
 * Unpack a child from a grid object
 *
 * @param[in] subobj
 *
 * @see elm_grid_unpack
 */
#define elm_obj_grid_unpack(subobj) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_grid_clear
 * @since 1.8
 *
 * Faster way to remove all child objects from a grid object.
 *
 * @param[in] clear
 *
 * @see elm_grid_clear
 */
#define elm_obj_grid_clear(clear) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_CLEAR), EO_TYPECHECK(Eina_Bool, clear)

/**
 * @def elm_obj_grid_children_get
 * @since 1.8
 *
 * Get the list of the children for the grid.
 *
 * @param[out] ret
 *
 * @see elm_grid_children_get
 */
#define elm_obj_grid_children_get(ret) ELM_OBJ_GRID_ID(ELM_OBJ_GRID_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

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
         It's up to the user to destroy it when it no longer needs it.
			It's possible to remove objects from the grid when walking this
			list, but these removals won't be reflected on it.
 *
 * @ingroup Grid
 */
EAPI Eina_List *elm_grid_children_get(const Evas_Object *obj);

/**
 * @}
 */
