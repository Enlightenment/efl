#define ELM_OBJ_SEGMENT_CONTROL_CLASS elm_obj_segment_control_class_get()

const Eo_Class *elm_obj_segment_control_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SEGMENT_CONTROL_BASE_ID;

enum
{
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ADD,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_INSERT_AT,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_DEL_AT,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_LABEL_GET,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ICON_GET,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_COUNT_GET,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_SELECTED_GET,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_GET,
   ELM_OBJ_SEGMENT_CONTROL_SUB_ID_LAST
};

#define ELM_OBJ_SEGMENT_CONTROL_ID(sub_id) (ELM_OBJ_SEGMENT_CONTROL_BASE_ID + sub_id)


/**
 * @def elm_obj_segment_control_item_add
 * @since 1.8
 *
 * Append a new item to the segment control object.
 *
 * @param[in] icon
 * @param[in] label
 * @param[out] ret
 *
 * @see elm_segment_control_item_add
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_add(icon, label, ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ADD), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_segment_control_item_insert_at
 * @since 1.8
 *
 * Insert a new item to the segment control object at specified position.
 *
 * @param[in] icon
 * @param[in] label
 * @param[in] idx
 * @param[out] ret
 *
 * @see elm_segment_control_item_insert_at
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_insert_at(icon, label, idx, ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_INSERT_AT), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(int, idx), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_segment_control_item_del_at
 * @since 1.8
 *
 * Remove a segment control item at given index from its parent,
 *
 * @param[in] idx
 *
 * @see elm_segment_control_item_del_at
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_del_at(idx) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_DEL_AT), EO_TYPECHECK(int, idx)

/**
 * @def elm_obj_segment_control_item_label_get
 * @since 1.8
 *
 * Get the label of item.
 *
 * @param[in] idx
 * @param[out] ret
 *
 * @see elm_segment_control_item_label_get
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_label_get(idx, ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_LABEL_GET), EO_TYPECHECK(int, idx), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_segment_control_item_icon_get
 * @since 1.8
 *
 * Get the icon associated to the item.
 *
 * @param[in] idx
 * @param[out] ret
 *
 * @see elm_segment_control_item_icon_get
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_icon_get(idx, ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ICON_GET), EO_TYPECHECK(int, idx), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_segment_control_item_count_get
 * @since 1.8
 *
 * Get the Segment items count from segment control.
 *
 * @param[out] ret
 *
 * @see elm_segment_control_item_count_get
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_count_get(ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_COUNT_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_segment_control_item_selected_get
 * @since 1.8
 *
 * Get the selected item.
 *
 * @param[out] ret
 *
 * @see elm_segment_control_item_selected_get
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_selected_get(ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_SELECTED_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_segment_control_item_get
 * @since 1.8
 *
 * Get the item placed at specified index.
 *
 * @param[in] idx
 * @param[out] ret
 *
 * @see elm_segment_control_item_get
 *
 * @ingroup SegmentControl
 */
#define elm_obj_segment_control_item_get(idx, ret) ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_GET), EO_TYPECHECK(int, idx), EO_TYPECHECK(Elm_Object_Item **, ret)
