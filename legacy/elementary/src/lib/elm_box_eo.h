#define ELM_OBJ_BOX_CLASS elm_obj_box_class_get()

const Eo_Class *elm_obj_box_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_BOX_BASE_ID;

enum
{
   ELM_OBJ_BOX_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_BOX_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_BOX_SUB_ID_HOMOGENEOUS_SET,
   ELM_OBJ_BOX_SUB_ID_HOMOGENEOUS_GET,
   ELM_OBJ_BOX_SUB_ID_PACK_START,
   ELM_OBJ_BOX_SUB_ID_PACK_END,
   ELM_OBJ_BOX_SUB_ID_PACK_BEFORE,
   ELM_OBJ_BOX_SUB_ID_PACK_AFTER,
   ELM_OBJ_BOX_SUB_ID_CLEAR,
   ELM_OBJ_BOX_SUB_ID_UNPACK,
   ELM_OBJ_BOX_SUB_ID_UNPACK_ALL,
   ELM_OBJ_BOX_SUB_ID_LAYOUT_SET,
   ELM_OBJ_BOX_SUB_ID_CHILDREN_GET,
   ELM_OBJ_BOX_SUB_ID_PADDING_SET,
   ELM_OBJ_BOX_SUB_ID_PADDING_GET,
   ELM_OBJ_BOX_SUB_ID_ALIGN_SET,
   ELM_OBJ_BOX_SUB_ID_ALIGN_GET,
   ELM_OBJ_BOX_SUB_ID_RECALCULATE,
   ELM_OBJ_BOX_SUB_ID_LAST
};

#define ELM_OBJ_BOX_ID(sub_id) (ELM_OBJ_BOX_BASE_ID + sub_id)


/**
 * @def elm_obj_box_horizontal_set
 * @since 1.8
 *
 * Set the horizontal orientation
 *
 * @param[in] horizontal
 *
 * @see elm_box_horizontal_set
 *
 * @ingroup Box
 */
#define elm_obj_box_horizontal_set(horizontal) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_box_horizontal_get
 * @since 1.8
 *
 * Get the horizontal orientation
 *
 * @param[out] ret
 *
 * @see elm_box_horizontal_get
 *
 * @ingroup Box
 */
#define elm_obj_box_horizontal_get(ret) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_box_homogeneous_set
 * @since 1.8
 *
 * Set the box to arrange its children homogeneously
 *
 * @param[in] homogeneous
 *
 * @see elm_box_homogeneous_set
 *
 * @ingroup Box
 */
#define elm_obj_box_homogeneous_set(homogeneous) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Eina_Bool, homogeneous)

/**
 * @def elm_obj_box_homogeneous_get
 * @since 1.8
 *
 * Get whether the box is using homogeneous mode or not
 *
 * @param[out] ret
 *
 * @see elm_box_homogeneous_get
 *
 * @ingroup Box
 */
#define elm_obj_box_homogeneous_get(ret) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_box_pack_start
 * @since 1.8
 *
 * Add an object to the beginning of the pack list
 *
 * @param[in] subobj
 *
 * @see elm_box_pack_start
 *
 * @ingroup Box
 */
#define elm_obj_box_pack_start(subobj) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PACK_START), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_box_pack_end
 * @since 1.8
 *
 * Add an object at the end of the pack list
 *
 * @param[in] subobj
 *
 * @see elm_box_pack_end
 *
 * @ingroup Box
 */
#define elm_obj_box_pack_end(subobj) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PACK_END), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_box_pack_before
 * @since 1.8
 *
 * Adds an object to the box before the indicated object
 *
 * @param[in] subobj
 * @param[in] before
 *
 * @see elm_box_pack_before
 *
 * @ingroup Box
 */
#define elm_obj_box_pack_before(subobj, before) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PACK_BEFORE), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(Evas_Object *, before)

/**
 * @def elm_obj_box_pack_after
 * @since 1.8
 *
 * Adds an object to the box after the indicated object
 *
 * @param[in] subobj
 * @param[in] after
 *
 * @see elm_box_pack_after
 *
 * @ingroup Box
 */
#define elm_obj_box_pack_after(subobj, after) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PACK_AFTER), EO_TYPECHECK(Evas_Object *, subobj), EO_TYPECHECK(Evas_Object *, after)

/**
 * @def elm_obj_box_clear
 * @since 1.8
 *
 * Clear the box of all children
 *
 *
 * @see elm_box_clear
 *
 * @ingroup Box
 */
#define elm_obj_box_clear() ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_CLEAR)

/**
 * @def elm_obj_box_unpack
 * @since 1.8
 *
 * Unpack a box item
 *
 * @param[in] subobj
 *
 * @see elm_box_unpack
 *
 * @ingroup Box
 */
#define elm_obj_box_unpack(subobj) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_UNPACK), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_box_unpack_all
 * @since 1.8
 *
 * Remove all items from the box, without deleting them
 *
 *
 * @see elm_box_unpack_all
 *
 * @ingroup Box
 */
#define elm_obj_box_unpack_all() ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_UNPACK_ALL)

/**
 * @def elm_obj_box_layout_set
 * @since 1.8
 *
 * Set the layout defining function to be used by the box
 *
 * @param[in] cb
 * @param[in] data
 * @param[in] free_data
 *
 * @see elm_box_layout_set
 *
 * @ingroup Box
 */
#define elm_obj_box_layout_set(cb, data, free_data) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_LAYOUT_SET), EO_TYPECHECK(Evas_Object_Box_Layout, cb), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Ecore_Cb, free_data)

/**
 * @def elm_obj_box_children_get
 * @since 1.8
 *
 * Retrieve a list of the objects packed into the box
 *
 * @param[out] ret
 *
 * @see elm_box_children_get
 *
 * @ingroup Box
 */
#define elm_obj_box_children_get(ret) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_CHILDREN_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def elm_obj_box_padding_set
 * @since 1.8
 *
 * Set the space (padding) between the box's elements.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see elm_box_padding_set
 *
 * @ingroup Box
 */
#define elm_obj_box_padding_set(horizontal, vertical) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PADDING_SET), EO_TYPECHECK(Evas_Coord, horizontal), EO_TYPECHECK(Evas_Coord, vertical)

/**
 * @def elm_obj_box_padding_get
 * @since 1.8
 *
 * Get the space (padding) between the box's elements.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see elm_box_padding_get
 *
 * @ingroup Box
 */
#define elm_obj_box_padding_get(horizontal, vertical) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_PADDING_GET), EO_TYPECHECK(Evas_Coord *, horizontal), EO_TYPECHECK(Evas_Coord *, vertical)

/**
 * @def elm_obj_box_align_set
 * @since 1.8
 *
 * Set the alignment of the whole bounding box of contents.
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see elm_box_align_set
 *
 * @ingroup Box
 */
#define elm_obj_box_align_set(horizontal, vertical) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def elm_obj_box_align_get
 * @since 1.8
 *
 * Get the alignment of the whole bounding box of contents.
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see elm_box_align_get
 *
 * @ingroup Box
 */
#define elm_obj_box_align_get(horizontal, vertical) ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)

/**
 * @def elm_obj_box_recalculate
 * @since 1.8
 *
 * Force the box to recalculate its children packing.
 *
 *
 * @see elm_box_recalculate
 *
 * @ingroup Box
 */
#define elm_obj_box_recalculate() ELM_OBJ_BOX_ID(ELM_OBJ_BOX_SUB_ID_RECALCULATE)
