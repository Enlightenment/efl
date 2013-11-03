/**
 * @ingroup Genlist
 *
 * @{
 */
#define ELM_OBJ_GENLIST_PAN_CLASS elm_obj_genlist_pan_class_get()

const Eo_Class *elm_obj_genlist_pan_class_get(void) EINA_CONST;

#define ELM_OBJ_GENLIST_CLASS elm_obj_genlist_class_get()

const Eo_Class *elm_obj_genlist_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_GENLIST_BASE_ID;

enum
{
   ELM_OBJ_GENLIST_SUB_ID_ITEMS_COUNT,
   ELM_OBJ_GENLIST_SUB_ID_ITEM_APPEND,
   ELM_OBJ_GENLIST_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_GENLIST_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_GENLIST_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_GENLIST_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_GENLIST_SUB_ID_CLEAR,
   ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_SET,
   ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_GET,
   ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_GENLIST_SUB_ID_SELECTED_ITEMS_GET,
   ELM_OBJ_GENLIST_SUB_ID_REALIZED_ITEMS_GET,
   ELM_OBJ_GENLIST_SUB_ID_AT_XY_ITEM_GET,
   ELM_OBJ_GENLIST_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_GENLIST_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_GENLIST_SUB_ID_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_HOMOGENEOUS_SET,
   ELM_OBJ_GENLIST_SUB_ID_HOMOGENEOUS_GET,
   ELM_OBJ_GENLIST_SUB_ID_BLOCK_COUNT_SET,
   ELM_OBJ_GENLIST_SUB_ID_BLOCK_COUNT_GET,
   ELM_OBJ_GENLIST_SUB_ID_LONGPRESS_TIMEOUT_SET,
   ELM_OBJ_GENLIST_SUB_ID_LONGPRESS_TIMEOUT_GET,
   ELM_OBJ_GENLIST_SUB_ID_REALIZED_ITEMS_UPDATE,
   ELM_OBJ_GENLIST_SUB_ID_DECORATED_ITEM_GET,
   ELM_OBJ_GENLIST_SUB_ID_DECORATE_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_DECORATE_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_REORDER_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_REORDER_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_HIGHLIGHT_MODE_SET,
   ELM_OBJ_GENLIST_SUB_ID_HIGHLIGHT_MODE_GET,
   ELM_OBJ_GENLIST_SUB_ID_TREE_EFFECT_ENABLED_SET,
   ELM_OBJ_GENLIST_SUB_ID_TREE_EFFECT_ENABLED_GET,
   ELM_OBJ_GENLIST_SUB_ID_FOCUS_ON_SELECTION_SET,
   ELM_OBJ_GENLIST_SUB_ID_FOCUS_ON_SELECTION_GET,
   ELM_OBJ_GENLIST_SUB_ID_LAST
};

#define ELM_OBJ_GENLIST_ID(sub_id) (ELM_OBJ_GENLIST_BASE_ID + sub_id)


/**
 * @def elm_obj_genlist_items_count
 * @since 1.8
 *
 * Return how many items are currently in a list
 *
 * @param[out] ret
 *
 * @see elm_genlist_items_count
 */
#define elm_obj_genlist_items_count(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEMS_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_genlist_item_append
 * @since 1.8
 *
 * Append a new item in a given genlist widget.
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] parent
 * @param[in] type
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_genlist_item_append
 */
#define elm_obj_genlist_item_append(itc, data, parent, type, func, func_data, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const Elm_Genlist_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Genlist_Item_Type, type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_item_prepend
 * @since 1.8
 *
 * Prepend a new item in a given genlist widget.
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] parent
 * @param[in] type
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_genlist_item_prepend
 */
#define elm_obj_genlist_item_prepend(itc, data, parent, type, func, func_data, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const Elm_Genlist_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Genlist_Item_Type, type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_item_insert_after
 * @since 1.8
 *
 * Insert an item after another in a genlist widget
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] parent
 * @param[in] after_it
 * @param[in] type
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_genlist_item_insert_after
 */
#define elm_obj_genlist_item_insert_after(itc, data, parent, after_it, type, func, func_data, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(const Elm_Genlist_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Object_Item *, after_it), EO_TYPECHECK(Elm_Genlist_Item_Type, type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_item_insert_before
 * @since 1.8
 *
 * Insert an item before another in a genlist widget
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] parent
 * @param[in] before_it
 * @param[in] type
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_genlist_item_insert_before
 */
#define elm_obj_genlist_item_insert_before(itc, data, parent, before_it, type, func, func_data, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(const Elm_Genlist_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Object_Item *, before_it), EO_TYPECHECK(Elm_Genlist_Item_Type, type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_item_sorted_insert
 * @since 1.8
 *
 * Insert a new item into the sorted genlist object
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] parent
 * @param[in] type
 * @param[in] comp
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_genlist_item_sorted_insert
 */
#define elm_obj_genlist_item_sorted_insert(itc, data, parent, type, comp, func, func_data, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const Elm_Genlist_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Genlist_Item_Type, type), EO_TYPECHECK(Eina_Compare_Cb, comp), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_clear
 * @since 1.8
 *
 * Remove all items from a given genlist widget.
 *
 *
 * @see elm_genlist_clear
 */
#define elm_obj_genlist_clear() ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_CLEAR)

/**
 * @def elm_obj_genlist_multi_select_set
 * @since 1.8
 *
 * Enable or disable multi-selection in the genlist
 *
 * @param[in] multi
 *
 * @see elm_genlist_multi_select_set
 */
#define elm_obj_genlist_multi_select_set(multi) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_SET), EO_TYPECHECK(Eina_Bool, multi)

/**
 * @def elm_obj_genlist_multi_select_get
 * @since 1.8
 *
 * Get if multi-selection in genlist is enabled or disabled.
 *
 * @param[out] ret
 *
 * @see elm_genlist_multi_select_get
 */
#define elm_obj_genlist_multi_select_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_multi_select_mode_set
 * @since 1.8
 *
 * Set the genlist multi select mode.
 *
 * @param[in] mode
 *
 * - ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT : select/unselect items whenever each
 *   item is clicked.
 * - ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL : Only one item will be selected
 *   although multi-selection is enabled, if clicked without pressing control
 *   key. This mode is only available with multi-selection.
 *
 * @see elm_genlist_multi_select_set()
 * @see elm_genlist_multi_select_mode_get()
 */
#define elm_obj_genlist_multi_select_mode_set(mode) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode, mode)

/**
 * @def elm_obj_genlist_multi_select_mode_get
 * @since 1.8
 *
 * Get the genlist multi select mode.
 *
 * @param[out] ret
 *
 * (If getting mode is failed, it returns ELM_OBJECT_MULTI_SELECT_MODE_MAX)
 *
 * @see elm_genlist_multi_select_set()
 * @see elm_genlist_multi_select_mode_set()
 */
#define elm_obj_genlist_multi_select_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MULTI_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode *, ret)

/**
 * @def elm_obj_genlist_selected_item_get
 * @since 1.8
 *
 * Get the selected item in the genlist.
 *
 * @param[out] ret
 *
 * @see elm_genlist_selected_item_get
 */
#define elm_obj_genlist_selected_item_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_selected_items_get
 * @since 1.8
 *
 * Get a list of selected items in the genlist.
 *
 * @param[out] ret
 *
 * @see elm_genlist_selected_items_get
 */
#define elm_obj_genlist_selected_items_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_SELECTED_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_genlist_realized_items_get
 * @since 1.8
 *
 * Get a list of realized items in genlist
 *
 * @param[out] ret
 *
 * @see elm_genlist_realized_items_get
 */
#define elm_obj_genlist_realized_items_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_REALIZED_ITEMS_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def elm_obj_genlist_at_xy_item_get
 * @since 1.8
 *
 * Get the item that is at the x, y canvas coords.
 *
 * @param[in] x
 * @param[in] y
 * @param[out] posret
 * @param[out] ret
 *
 * @see elm_genlist_at_xy_item_get
 */
#define elm_obj_genlist_at_xy_item_get(x, y, posret, ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_AT_XY_ITEM_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, posret), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_first_item_get
 * @since 1.8
 *
 * Get the first item in the genlist
 *
 * @param[out] ret
 *
 * @see elm_genlist_first_item_get
 */
#define elm_obj_genlist_first_item_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_last_item_get
 * @since 1.8
 *
 * Get the last item in the genlist
 *
 * @param[out] ret
 *
 * @see elm_genlist_last_item_get
 */
#define elm_obj_genlist_last_item_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_mode_set
 * @since 1.8
 *
 * This sets the horizontal stretching mode.
 *
 * @param[in] mode
 *
 * @see elm_genlist_mode_set
 */
#define elm_obj_genlist_mode_set(mode) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MODE_SET), EO_TYPECHECK(Elm_List_Mode, mode)

/**
 * @def elm_obj_genlist_mode_get
 * @since 1.8
 *
 * Get the horizontal stretching mode.
 *
 * @param[out] ret
 *
 * @see elm_genlist_mode_get
 */
#define elm_obj_genlist_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_MODE_GET), EO_TYPECHECK(Elm_List_Mode *, ret)

/**
 * @def elm_obj_genlist_homogeneous_set
 * @since 1.8
 *
 * Enable/disable homogeneous mode.
 *
 * @param[in] homogeneous
 *
 * @see elm_genlist_homogeneous_set
 */
#define elm_obj_genlist_homogeneous_set(homogeneous) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Eina_Bool, homogeneous)

/**
 * @def elm_obj_genlist_homogeneous_get
 * @since 1.8
 *
 * Get whether the homogeneous mode is enabled.
 *
 * @param[out] ret
 *
 * @see elm_genlist_homogeneous_get
 */
#define elm_obj_genlist_homogeneous_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_block_count_set
 * @since 1.8
 *
 * Set the maximum number of items within an item block
 *
 * @param[in] count
 *
 * @see elm_genlist_block_count_set
 */
#define elm_obj_genlist_block_count_set(count) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_BLOCK_COUNT_SET), EO_TYPECHECK(int, count)

/**
 * @def elm_obj_genlist_block_count_get
 * @since 1.8
 *
 * Get the maximum number of items within an item block
 *
 * @param[out] ret
 *
 * @see elm_genlist_block_count_get
 */
#define elm_obj_genlist_block_count_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_BLOCK_COUNT_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_genlist_longpress_timeout_set
 * @since 1.8
 *
 * Set the timeout in seconds for the longpress event.
 *
 * @param[in] timeout
 *
 * @see elm_genlist_longpress_timeout_set
 */
#define elm_obj_genlist_longpress_timeout_set(timeout) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_LONGPRESS_TIMEOUT_SET), EO_TYPECHECK(double, timeout)

/**
 * @def elm_obj_genlist_longpress_timeout_get
 * @since 1.8
 *
 * Get the timeout in seconds for the longpress event.
 *
 * @param[out] ret
 *
 * @see elm_genlist_longpress_timeout_get
 */
#define elm_obj_genlist_longpress_timeout_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_LONGPRESS_TIMEOUT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_genlist_realized_items_update
 * @since 1.8
 *
 * Update the contents of all realized items.
 *
 *
 * @see elm_genlist_realized_items_update
 */
#define elm_obj_genlist_realized_items_update() ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_REALIZED_ITEMS_UPDATE)

/**
 * @def elm_obj_genlist_decorated_item_get
 * @since 1.8
 *
 * Get active genlist mode item
 *
 * @param[out] ret
 *
 * @see elm_genlist_decorated_item_get
 */
#define elm_obj_genlist_decorated_item_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_DECORATED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_genlist_decorate_mode_get
 * @since 1.8
 *
 * Get Genlist decorate mode
 *
 * @param[out] ret
 *
 * @see elm_genlist_decorate_mode_get
 */
#define elm_obj_genlist_decorate_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_DECORATE_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_decorate_mode_set
 * @since 1.8
 *
 * Set Genlist decorate mode
 *
 * @param[in] decorated
 *
 * @see elm_genlist_decorate_mode_set
 */
#define elm_obj_genlist_decorate_mode_set(decorated) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_DECORATE_MODE_SET), EO_TYPECHECK(Eina_Bool, decorated)

/**
 * @def elm_obj_genlist_reorder_mode_set
 * @since 1.8
 *
 * Set reorder mode
 *
 * @param[in] reorder_mode
 *
 * @see elm_genlist_reorder_mode_set
 */
#define elm_obj_genlist_reorder_mode_set(reorder_mode) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_REORDER_MODE_SET), EO_TYPECHECK(Eina_Bool, reorder_mode)

/**
 * @def elm_obj_genlist_reorder_mode_get
 * @since 1.8
 *
 * Get the reorder mode
 *
 * @param[out] ret
 *
 * @see elm_genlist_reorder_mode_get
 */
#define elm_obj_genlist_reorder_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_REORDER_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_select_mode_set
 * @since 1.8
 *
 * Set the genlist select mode.
 *
 * @param[in] mode
 *
 * @see elm_genlist_select_mode_set
 */
#define elm_obj_genlist_select_mode_set(mode) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Select_Mode, mode)

/**
 * @def elm_obj_genlist_select_mode_get
 * @since 1.8
 *
 * Get the genlist select mode.
 *
 * @param[out] ret
 *
 * @see elm_genlist_select_mode_get
 */
#define elm_obj_genlist_select_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Select_Mode *, ret)

/**
 * @def elm_obj_genlist_highlight_mode_set
 * @since 1.8
 *
 * Set whether the genlist items' should be highlighted when item selected.
 *
 * @param[in] highlight
 *
 * @see elm_genlist_highlight_mode_set
 */
#define elm_obj_genlist_highlight_mode_set(highlight) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_HIGHLIGHT_MODE_SET), EO_TYPECHECK(Eina_Bool, highlight)

/**
 * @def elm_obj_genlist_highlight_mode_get
 * @since 1.8
 *
 * Get whether the genlist items' should be highlighted when item selected.
 *
 * @param[out] ret
 *
 * @see elm_genlist_highlight_mode_get
 */
#define elm_obj_genlist_highlight_mode_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_HIGHLIGHT_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_tree_effect_enabled_set
 * @since 1.8
 *
 * Set Genlist tree effect
 *
 * @param[in] enabled
 *
 * @see elm_genlist_tree_effect_enabled_set
 */
#define elm_obj_genlist_tree_effect_enabled_set(enabled) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_TREE_EFFECT_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_genlist_tree_effect_enabled_get
 * @since 1.8
 *
 * Get Genlist tree effect
 *
 * @param[out] ret
 *
 * @see elm_genlist_tree_effect_enabled_get
 */
#define elm_obj_genlist_tree_effect_enabled_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_TREE_EFFECT_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_genlist_focus_on_selection_set
 * @since 1.8
 *
 * Set focus to a first from left focusable widget upon item selection.
 *
 * @param[in] enabled
 *
 * @see elm_obj_genlist_focus_on_selection_get
 */
#define elm_obj_genlist_focus_on_selection_set(enabled) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_FOCUS_ON_SELECTION_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_genlist_focus_on_selection_get
 * @since 1.8
 *
 * Get whether the focus will be set to a widget on an item upon it's selection.
 *
 * @param[out] ret
 *
 * @see elm_obj_genlist_focus_on_selection_set
 */
#define elm_obj_genlist_focus_on_selection_get(ret) ELM_OBJ_GENLIST_ID(ELM_OBJ_GENLIST_SUB_ID_FOCUS_ON_SELECTION_GET), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @}
 */

