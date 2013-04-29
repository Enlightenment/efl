/**
 * @ingroup Grid
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
 * @}
 */
