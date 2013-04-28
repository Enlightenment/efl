#define ELM_OBJ_DISKSELECTOR_CLASS elm_obj_diskselector_class_get()

const Eo_Class *elm_obj_diskselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_DISKSELECTOR_BASE_ID;

enum
{
   ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_SET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_SET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_CLEAR,
   ELM_OBJ_DISKSELECTOR_SUB_ID_ITEMS_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_ITEM_APPEND,
   ELM_OBJ_DISKSELECTOR_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_SET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_GET,
   ELM_OBJ_DISKSELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_DISKSELECTOR_ID(sub_id) (ELM_OBJ_DISKSELECTOR_BASE_ID + sub_id)


/**
 * @def elm_obj_diskselector_round_enabled_get
 * @since 1.8
 *
 * Get a value whether round mode is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_round_enabled_get
 */
#define elm_obj_diskselector_round_enabled_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_diskselector_round_enabled_set
 * @since 1.8
 *
 * Enable or disable round mode.
 *
 * @param[in] enabled
 *
 * @see elm_diskselector_round_enabled_set
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_round_enabled_set(enabled) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_diskselector_side_text_max_length_get
 * @since 1.8
 *
 * Get the side labels max length.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_side_text_max_length_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_side_text_max_length_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_diskselector_side_text_max_length_set
 * @since 1.8
 *
 * Set the side labels max length.
 *
 * @param[in] len
 *
 * @see elm_diskselector_side_text_max_length_set
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_side_text_max_length_set(len) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_SET), EO_TYPECHECK(int, len)

/**
 * @def elm_obj_diskselector_clear
 * @since 1.8
 *
 * Remove all diskselector's items.
 *
 *
 * @see elm_diskselector_clear
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_clear() ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_CLEAR)

/**
 * @def elm_obj_diskselector_items_get
 * @since 1.8
 *
 * Get a list of all the diskselector items.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_items_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_items_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_diskselector_item_append
 * @since 1.8
 *
 * Appends a new item to the diskselector object.
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_diskselector_item_append
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_item_append(label, icon, func, data, ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_diskselector_selected_item_get
 * @since 1.8
 *
 * Get the selected item.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_selected_item_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_selected_item_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_diskselector_first_item_get
 * @since 1.8
 *
 * Get the first item of the diskselector.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_first_item_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_first_item_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_diskselector_last_item_get
 * @since 1.8
 *
 * Get the last item of the diskselector.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_last_item_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_last_item_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_diskselector_display_item_num_set
 * @since 1.8
 *
 * Set the number of items to be displayed.
 *
 * @param[in] num
 *
 * @see elm_diskselector_display_item_num_set
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_display_item_num_set(num) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_SET), EO_TYPECHECK(int, num)

/**
 * @def elm_obj_diskselector_display_item_num_get
 * @since 1.8
 *
 * Get the number of items in the diskselector object.
 *
 * @param[out] ret
 *
 * @see elm_diskselector_display_item_num_get
 *
 * @ingroup Diskselector
 */
#define elm_obj_diskselector_display_item_num_get(ret) ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_GET), EO_TYPECHECK(int *, ret)
