/**
 * @ingroup Gengrid
 *
 * @{
 */
#define ELM_OBJ_GENGRID_PAN_CLASS elm_obj_gengrid_pan_class_get()

const Eo_Class *elm_obj_gengrid_pan_class_get(void) EINA_CONST;

#define ELM_OBJ_GENGRID_CLASS elm_obj_gengrid_class_get()

const Eo_Class *elm_obj_gengrid_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_GENGRID_BASE_ID;

enum
{
   ELM_OBJ_GENGRID_SUB_ID_ITEM_SIZE_SET,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_SIZE_GET,
   ELM_OBJ_GENGRID_SUB_ID_GROUP_ITEM_SIZE_SET,
   ELM_OBJ_GENGRID_SUB_ID_GROUP_ITEM_SIZE_GET,
   ELM_OBJ_GENGRID_SUB_ID_ALIGN_SET,
   ELM_OBJ_GENGRID_SUB_ID_ALIGN_GET,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_APPEND,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_GENGRID_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_GENGRID_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_GENGRID_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_GENGRID_SUB_ID_CLEAR,
   ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_SET,
   ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_GET,
   ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_MODE_SET,
   ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_MODE_GET,
   ELM_OBJ_GENGRID_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_GENGRID_SUB_ID_SELECTED_ITEMS_GET,
   ELM_OBJ_GENGRID_SUB_ID_REALIZED_ITEMS_GET,
   ELM_OBJ_GENGRID_SUB_ID_REALIZED_ITEMS_UPDATE,
   ELM_OBJ_GENGRID_SUB_ID_REORDER_MODE_SET,
   ELM_OBJ_GENGRID_SUB_ID_REORDER_MODE_GET,
   ELM_OBJ_GENGRID_SUB_ID_PAGE_RELATIVE_SET,
   ELM_OBJ_GENGRID_SUB_ID_PAGE_RELATIVE_GET,
   ELM_OBJ_GENGRID_SUB_ID_PAGE_SIZE_SET,
   ELM_OBJ_GENGRID_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_GENGRID_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_GENGRID_SUB_ID_FILLED_SET,
   ELM_OBJ_GENGRID_SUB_ID_FILLED_GET,
   ELM_OBJ_GENGRID_SUB_ID_ITEMS_COUNT,
   ELM_OBJ_GENGRID_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_GENGRID_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_GENGRID_SUB_ID_HIGHLIGHT_MODE_SET,
   ELM_OBJ_GENGRID_SUB_ID_HIGHLIGHT_MODE_GET,
   ELM_OBJ_GENGRID_SUB_ID_AT_XY_ITEM_GET,
   ELM_OBJ_GENGRID_SUB_ID_LAST
};

#define ELM_OBJ_GENGRID_ID(sub_id) (ELM_OBJ_GENGRID_BASE_ID + sub_id)


/**
 * @def elm_obj_gengrid_item_size_set
 * @since 1.8
 *
 * Set the size for the items of a given gengrid widget
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_gengrid_item_size_set
 */
#define elm_obj_gengrid_item_size_set(w, h) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_obj_gengrid_item_size_get
 * @since 1.8
 *
 * Get the size set for the items of a given gengrid widget
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_gengrid_item_size_get
 */
#define elm_obj_gengrid_item_size_get(w, h) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_obj_gengrid_group_item_size_set
 * @since 1.8
 *
 * Set the size for the group items of a given gengrid widget
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_gengrid_group_item_size_set
 */
#define elm_obj_gengrid_group_item_size_set(w, h) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_GROUP_ITEM_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def elm_obj_gengrid_group_item_size_get
 * @since 1.8
 *
 * Get the size set for the group items of a given gengrid widget
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_gengrid_group_item_size_get
 */
#define elm_obj_gengrid_group_item_size_get(w, h) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_GROUP_ITEM_SIZE_GET), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @def elm_obj_gengrid_align_set
 * @since 1.8
 *
 * Set the items grid's alignment within a given gengrid widget
 *
 * @param[in] align_x
 * @param[in] align_y
 *
 * @see elm_gengrid_align_set
 */
#define elm_obj_gengrid_align_set(align_x, align_y) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, align_x), EO_TYPECHECK(double, align_y)

/**
 * @def elm_obj_gengrid_align_get
 * @since 1.8
 *
 * Get the items grid's alignment values within a given gengrid
 *
 * @param[out] align_x
 * @param[out] align_y
 *
 * @see elm_gengrid_align_get
 */
#define elm_obj_gengrid_align_get(align_x, align_y) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, align_x), EO_TYPECHECK(double *, align_y)

/**
 * @def elm_obj_gengrid_item_append
 * @since 1.8
 *
 * Append a new item in a given gengrid widget.
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_gengrid_item_append
 */
#define elm_obj_gengrid_item_append(itc, data, func, func_data, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const Elm_Gengrid_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_item_prepend
 * @since 1.8
 *
 * Prepend a new item in a given gengrid widget.
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_gengrid_item_prepend
 */
#define elm_obj_gengrid_item_prepend(itc, data, func, func_data, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const Elm_Gengrid_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_item_insert_before
 * @since 1.8
 *
 * Insert an item before another in a gengrid widget
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] relative
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_gengrid_item_insert_before
 */
#define elm_obj_gengrid_item_insert_before(itc, data, relative, func, func_data, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(const Elm_Gengrid_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, relative), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_item_insert_after
 * @since 1.8
 *
 * Insert an item after another in a gengrid widget
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] relative
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_gengrid_item_insert_after
 */
#define elm_obj_gengrid_item_insert_after(itc, data, relative, func, func_data, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(const Elm_Gengrid_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item *, relative), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_item_sorted_insert
 * @since 1.8
 *
 * Insert an item in a gengrid widget using a user-defined sort function.
 *
 * @param[in] itc
 * @param[in] data
 * @param[in] comp
 * @param[in] func
 * @param[in] func_data
 * @param[out] ret
 *
 * @see elm_gengrid_item_sorted_insert
 */
#define elm_obj_gengrid_item_sorted_insert(itc, data, comp, func, func_data, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const Elm_Gengrid_Item_Class *, itc), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Compare_Cb, comp), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, func_data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_horizontal_set
 * @since 1.8
 *
 * Set the direction in which a given gengrid widget will expand while
 * placing its items.
 *
 * @param[in] horizontal
 *
 * @see elm_gengrid_horizontal_set
 */
#define elm_obj_gengrid_horizontal_set(horizontal) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_gengrid_horizontal_get
 * @since 1.8
 *
 * Get for what direction a given gengrid widget will expand while
 * placing its items.
 *
 * @param[out] ret
 *
 * @see elm_gengrid_horizontal_get
 */
#define elm_obj_gengrid_horizontal_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gengrid_clear
 * @since 1.8
 *
 * Remove all items from a given gengrid widget
 *
 *
 * @see elm_gengrid_clear
 */
#define elm_obj_gengrid_clear() ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_CLEAR)

/**
 * @def elm_obj_gengrid_multi_select_set
 * @since 1.8
 *
 * Enable or disable multi-selection in a given gengrid widget
 *
 * @param[in] multi
 *
 * @see elm_gengrid_multi_select_set
 */
#define elm_obj_gengrid_multi_select_set(multi) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_SET), EO_TYPECHECK(Eina_Bool, multi)

/**
 * @def elm_obj_gengrid_multi_select_get
 * @since 1.8
 *
 * Get whether multi-selection is enabled or disabled for a given
 * gengrid widget
 *
 * @param[out] ret
 *
 * @see elm_gengrid_multi_select_get
 */
#define elm_obj_gengrid_multi_select_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gengrid_multi_select_mode_set
 * @since 1.8
 *
 * Set the gengrid multi select mode.
 *
 * @param[in] mode
 *
 * - ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT : select/unselect items whenever each
 *   item is clicked.
 * - ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL : Only one item will be selected
 *   although multi-selection is enabled, if clicked without pressing control
 *   key. This mode is only available with multi-selection.
 *
 * @see elm_gengrid_multi_select_set()
 * @see elm_gengrid_multi_select_mode_get()
 */
#define elm_obj_gengrid_multi_select_mode_set(mode) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode, mode)

/**
 * @def elm_obj_gengrid_multi_select_mode_get
 * @since 1.8
 *
 * Get the gengrid multi select mode.
 *
 * @param[out] ret
 *
 * (If getting mode is failed, it returns ELM_OBJECT_MULTI_SELECT_MODE_MAX)
 *
 * @see elm_gengrid_multi_select_set()
 * @see elm_gengrid_multi_select_mode_set()
 */
#define elm_obj_gengrid_multi_select_mode_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_MULTI_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode *, ret)

/**
 * @def elm_obj_gengrid_selected_item_get
 * @since 1.8
 *
 * Get the selected item in a given gengrid widget
 *
 * @param[out] ret
 *
 * @see elm_gengrid_selected_item_get
 */
#define elm_obj_gengrid_selected_item_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_selected_items_get
 * @since 1.8
 *
 * Get <b>a list</b> of selected items in a given gengrid
 *
 * @param[out] ret
 *
 * @see elm_gengrid_selected_items_get
 */
#define elm_obj_gengrid_selected_items_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SELECTED_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_gengrid_realized_items_get
 * @since 1.8
 *
 * Get a list of realized items in gengrid
 *
 * @param[out] ret
 *
 * @see elm_gengrid_realized_items_get
 */
#define elm_obj_gengrid_realized_items_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_REALIZED_ITEMS_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def elm_obj_gengrid_realized_items_update
 * @since 1.8
 *
 * Update the contents of all realized items.
 *
 *
 * @see elm_gengrid_realized_items_update
 */
#define elm_obj_gengrid_realized_items_update() ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_REALIZED_ITEMS_UPDATE)

/**
 * @def elm_obj_gengrid_reorder_mode_set
 * @since 1.8
 *
 * Set whether a given gengrid widget is or not able have items
 *
 * @param[in] reorder_mode
 *
 * @see elm_gengrid_reorder_mode_set
 */
#define elm_obj_gengrid_reorder_mode_set(reorder_mode) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_REORDER_MODE_SET), EO_TYPECHECK(Eina_Bool, reorder_mode)

/**
 * @def elm_obj_gengrid_reorder_mode_get
 * @since 1.8
 *
 * Get whether a given gengrid widget is or not able have items
 *
 * @param[out] ret
 *
 * @see elm_gengrid_reorder_mode_get
 */
#define elm_obj_gengrid_reorder_mode_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_REORDER_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gengrid_bounce_set
 * @since 1.8
 *
 * Enable or disable bouncing effect for a given gengrid widget
 *
 * @param[in] h_bounce
 * @param[in] v_bounce
 *
 * @see elm_gengrid_bounce_set
 */
#define elm_obj_gengrid_bounce_set(h_bounce, v_bounce) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_BOUNCE_SET), EO_TYPECHECK(Eina_Bool, h_bounce), EO_TYPECHECK(Eina_Bool, v_bounce)

/**
 * @def elm_obj_gengrid_bounce_get
 * @since 1.8
 *
 * Get whether bouncing effects are enabled or disabled, for a
 * given gengrid widget, on each axis
 *
 * @param[out] h_bounce
 * @param[out] v_bounce
 *
 * @see elm_gengrid_bounce_get
 */
#define elm_obj_gengrid_bounce_get(h_bounce, v_bounce) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_BOUNCE_GET), EO_TYPECHECK(Eina_Bool *, h_bounce), EO_TYPECHECK(Eina_Bool *, v_bounce)

/**
 * @def elm_obj_gengrid_page_relative_set
 * @since 1.8
 *
 * Set a given gengrid widget's scrolling page size, relative to
 * its viewport size.
 *
 * @param[in] h_pagerel
 * @param[in] v_pagerel
 *
 * @see elm_gengrid_page_relative_set
 */
#define elm_obj_gengrid_page_relative_set(h_pagerel, v_pagerel) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_PAGE_RELATIVE_SET), EO_TYPECHECK(double, h_pagerel), EO_TYPECHECK(double, v_pagerel)

/**
 * @def elm_obj_gengrid_page_relative_get
 * @since 1.8
 *
 * Get a given gengrid widget's scrolling page size, relative to
 * its viewport size.
 *
 * @param[out] h_pagerel
 * @param[out] v_pagerel
 *
 * @see elm_gengrid_page_relative_get
 */
#define elm_obj_gengrid_page_relative_get(h_pagerel, v_pagerel) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_PAGE_RELATIVE_GET), EO_TYPECHECK(double *, h_pagerel), EO_TYPECHECK(double *, v_pagerel)

/**
 * @def elm_obj_gengrid_page_size_set
 * @since 1.8
 *
 * Set a given gengrid widget's scrolling page size
 *
 * @param[in] h_pagesize
 * @param[in] v_pagesize
 *
 * @see elm_gengrid_page_size_set
 */
#define elm_obj_gengrid_page_size_set(h_pagesize, v_pagesize) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_PAGE_SIZE_SET), EO_TYPECHECK(Evas_Coord, h_pagesize), EO_TYPECHECK(Evas_Coord, v_pagesize)

/**
 * @def elm_obj_gengrid_current_page_get
 * @since 1.8
 *
 * @brief Get gengrid current page number.
 *
 * @param[out] h_pagenumber
 * @param[out] v_pagenumber
 *
 * @see elm_gengrid_current_page_get
 */
#define elm_obj_gengrid_current_page_get(h_pagenumber, v_pagenumber) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_CURRENT_PAGE_GET), EO_TYPECHECK(int *, h_pagenumber), EO_TYPECHECK(int *, v_pagenumber)

/**
 * @def elm_obj_gengrid_last_page_get
 * @since 1.8
 *
 * @brief Get gengrid last page number.
 *
 * @param[out] h_pagenumber
 * @param[out] v_pagenumber
 *
 * @see elm_gengrid_last_page_get
 */
#define elm_obj_gengrid_last_page_get(h_pagenumber, v_pagenumber) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_LAST_PAGE_GET), EO_TYPECHECK(int *, h_pagenumber), EO_TYPECHECK(int *, v_pagenumber)

/**
 * @def elm_obj_gengrid_page_show
 * @since 1.8
 *
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param[in] h_pagenumber
 * @param[in] v_pagenumber
 *
 * @see elm_gengrid_page_show
 */
#define elm_obj_gengrid_page_show(h_pagenumber, v_pagenumber) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_PAGE_SHOW), EO_TYPECHECK(int, h_pagenumber), EO_TYPECHECK(int, v_pagenumber)

/**
 * @def elm_obj_gengrid_page_bring_in
 * @since 1.8
 *
 * Show a specific virtual region within the gengrid content object by page number.
 *
 * @param[in] h_pagenumber
 * @param[in] v_pagenumber
 *
 * @see elm_gengrid_page_bring_in
 */
#define elm_obj_gengrid_page_bring_in(h_pagenumber, v_pagenumber) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_PAGE_BRING_IN), EO_TYPECHECK(int, h_pagenumber), EO_TYPECHECK(int, v_pagenumber)

/**
 * @def elm_obj_gengrid_scroller_policy_set
 * @since 1.8
 *
 * Set the scrollbar policy
 *
 * @param[in] policy_h
 * @param[in] policy_v
 *
 * @see elm_gengrid_scroller_policy_set
 */
#define elm_obj_gengrid_scroller_policy_set(policy_h, policy_v) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SCROLLER_POLICY_SET), EO_TYPECHECK(Elm_Scroller_Policy, policy_h), EO_TYPECHECK(Elm_Scroller_Policy, policy_v)

/**
 * @def elm_obj_gengrid_scroller_policy_get
 * @since 1.8
 *
 * Get the scrollbar policy
 *
 * @param[out] policy_h
 * @param[out] policy_v
 *
 * @see elm_gengrid_scroller_policy_get
 */
#define elm_obj_gengrid_scroller_policy_get(policy_h, policy_v) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SCROLLER_POLICY_GET), EO_TYPECHECK(Elm_Scroller_Policy *, policy_h), EO_TYPECHECK(Elm_Scroller_Policy *, policy_v)

/**
 * @def elm_obj_gengrid_first_item_get
 * @since 1.8
 *
 * Get the first item in a given gengrid widget
 *
 * @param[out] ret
 *
 * @see elm_gengrid_first_item_get
 */
#define elm_obj_gengrid_first_item_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_last_item_get
 * @since 1.8
 *
 * Get the last item in a given gengrid widget
 *
 * @param[out] ret
 *
 * @see elm_gengrid_last_item_get
 */
#define elm_obj_gengrid_last_item_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_gengrid_filled_set
 * @since 1.8
 *
 * Set how the items grid's filled within a given gengrid widget
 *
 * @param[in] fill
 *
 * @see elm_gengrid_filled_set
 */
#define elm_obj_gengrid_filled_set(fill) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_FILLED_SET), EO_TYPECHECK(Eina_Bool, fill)

/**
 * @def elm_obj_gengrid_filled_get
 * @since 1.8
 *
 * Get how the items grid's filled within a given gengrid widget
 *
 * @param[out] ret
 *
 * @see elm_gengrid_filled_get
 */
#define elm_obj_gengrid_filled_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_FILLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gengrid_items_count
 * @since 1.8
 *
 * Return how many items are currently in a list
 *
 * @param[out] ret
 *
 * @see elm_gengrid_items_count
 */
#define elm_obj_gengrid_items_count(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_ITEMS_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_gengrid_select_mode_set
 * @since 1.8
 *
 * Set the gengrid select mode.
 *
 * @param[in] mode
 *
 * @see elm_gengrid_select_mode_set
 */
#define elm_obj_gengrid_select_mode_set(mode) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Select_Mode, mode)

/**
 * @def elm_obj_gengrid_select_mode_get
 * @since 1.8
 *
 * Get the gengrid select mode.
 *
 * @param[out] ret
 *
 * @see elm_gengrid_select_mode_get
 */
#define elm_obj_gengrid_select_mode_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Select_Mode *, ret)

/**
 * @def elm_obj_gengrid_highlight_mode_set
 * @since 1.8
 *
 * Set whether the gengrid items' should be highlighted when item selected.
 *
 * @param[in] highlight
 *
 * @see elm_gengrid_highlight_mode_set
 */
#define elm_obj_gengrid_highlight_mode_set(highlight) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_HIGHLIGHT_MODE_SET), EO_TYPECHECK(Eina_Bool, highlight)

/**
 * @def elm_obj_gengrid_highlight_mode_get
 * @since 1.8
 *
 * Get whether the gengrid items' should be highlighted when item selected.
 *
 * @param[out] ret
 *
 * @see elm_gengrid_highlight_mode_get
 */
#define elm_obj_gengrid_highlight_mode_get(ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_HIGHLIGHT_MODE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_gengrid_at_xy_item_get
 * @since 1.8
 *
 * Get the item that is at the x, y canvas coords.
 *
 * @param[in] x
 * @param[in] y
 * @param[out] xposret
 * @param[out] yposret
 * @param[out] ret
 *
 * @see elm_gengrid_at_xy_item_get
 */
#define elm_obj_gengrid_at_xy_item_get(x, y, xposret, yposret, ret) ELM_OBJ_GENGRID_ID(ELM_OBJ_GENGRID_SUB_ID_AT_XY_ITEM_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, xposret), EO_TYPECHECK(int *, yposret), EO_TYPECHECK(Elm_Object_Item **, ret)


/**
 * @}
 */

