/**
 * @ingroup List
 *
 * @{
 */
#define ELM_OBJ_LIST_CLASS elm_obj_list_class_get()

 const Eo_Class *elm_obj_list_class_get(void) EINA_CONST;

 extern EAPI Eo_Op ELM_OBJ_LIST_BASE_ID;

 enum
{
   ELM_OBJ_LIST_SUB_ID_GO,
   ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_SET,
   ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_GET,
   ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_MODE_SET,
   ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_MODE_GET,
   ELM_OBJ_LIST_SUB_ID_MODE_SET,
   ELM_OBJ_LIST_SUB_ID_MODE_GET,
   ELM_OBJ_LIST_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_LIST_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_LIST_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_LIST_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_LIST_SUB_ID_CLEAR,
   ELM_OBJ_LIST_SUB_ID_ITEMS_GET,
   ELM_OBJ_LIST_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_LIST_SUB_ID_SELECTED_ITEMS_GET,
   ELM_OBJ_LIST_SUB_ID_ITEM_APPEND,
   ELM_OBJ_LIST_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_LIST_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_LIST_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_LIST_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_LIST_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_LIST_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_LIST_SUB_ID_AT_XY_ITEM_GET,
   ELM_OBJ_LIST_SUB_ID_FOCUS_ON_SELECTION_SET,
   ELM_OBJ_LIST_SUB_ID_FOCUS_ON_SELECTION_GET,
   ELM_OBJ_LIST_SUB_ID_LAST
};

#define ELM_OBJ_LIST_ID(sub_id) (ELM_OBJ_LIST_BASE_ID + sub_id)


/**
 * @def elm_obj_list_go
 * @since 1.8
 *
 * Starts the list.
 *
 *
 * @see elm_list_go
 */
#define elm_obj_list_go() ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_GO)

/**
 * @def elm_obj_list_multi_select_set
 * @since 1.8
 *
 * Enable or disable multiple items selection on the list object.
 *
 * @param[in] multi
 *
 * @see elm_list_multi_select_set
 */
#define elm_obj_list_multi_select_set(multi) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_SET), EO_TYPECHECK(Eina_Bool, multi)

/**
 * @def elm_obj_list_multi_select_get
 * @since 1.8
 *
 * Get a value whether multiple items selection is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_list_multi_select_get
 */
#define elm_obj_list_multi_select_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_list_multi_select_mode_set
 * @since 1.8
 *
 * Set the list multi select mode.
 *
 * @param[in] mode
 *
 * - ELM_OBJECT_MULTI_SELECT_MODE_DEFAULT : select/unselect items whenever each
 *   item is clicked.
 * - ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL : Only one item will be selected
 *   although multi-selection is enabled, if clicked without pressing control
 *   key. This mode is only available with multi-selection.
 *
 * @see elm_list_multi_select_set()
 * @see elm_list_multi_select_mode_get()
 */
#define elm_obj_list_multi_select_mode_set(mode) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode, mode)

/**
 * @def elm_obj_list_multi_select_mode_get
 * @since 1.8
 *
 * Get the list multi select mode.
 *
 * @param[out] ret
 *
 * (If getting mode is failed, it returns ELM_OBJECT_MULTI_SELECT_MODE_MAX)
 *
 * @see elm_list_multi_select_set()
 * @see elm_list_multi_select_mode_set()
 */
#define elm_obj_list_multi_select_mode_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MULTI_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Multi_Select_Mode *, ret)

/**
 * @def elm_obj_list_mode_set
 * @since 1.8
 *
 * Set which mode to use for the list object.
 *
 * @param[in] mode
 *
 * @see elm_list_mode_set
 */
#define elm_obj_list_mode_set(mode) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MODE_SET), EO_TYPECHECK(Elm_List_Mode, mode)

/**
 * @def elm_obj_list_mode_get
 * @since 1.8
 *
 * Get the mode the list is at.
 *
 * @param[out] ret
 *
 * @see elm_list_mode_get
 */
#define elm_obj_list_mode_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_MODE_GET), EO_TYPECHECK(Elm_List_Mode *, ret)

/**
 * @def elm_obj_list_horizontal_set
 * @since 1.8
 *
 * Enable or disable horizontal mode on the list object.
 *
 * @param[in] horizontal
 *
 * @see elm_list_horizontal_set
 */
#define elm_obj_list_horizontal_set(horizontal) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_list_horizontal_get
 * @since 1.8
 *
 * Get a value whether horizontal mode is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_list_horizontal_get
 */
#define elm_obj_list_horizontal_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_list_select_mode_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] mode
 *
 * @see elm_list_select_mode_set
 */
#define elm_obj_list_select_mode_set(mode) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Select_Mode, mode)

/**
 * @def elm_obj_list_select_mode_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_list_select_mode_get
 */
#define elm_obj_list_select_mode_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Select_Mode *, ret)

/**
 * @def elm_obj_list_bounce_set
 * @since 1.8
 *
 * Set bouncing behaviour when the scrolled content reaches an edge.
 *
 * @param[in] h_bounce
 * @param[in] v_bounce
 *
 * @see elm_list_bounce_set
 */
#define elm_obj_list_bounce_set(h_bounce, v_bounce) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_BOUNCE_SET), EO_TYPECHECK(Eina_Bool, h_bounce), EO_TYPECHECK(Eina_Bool, v_bounce)

/**
 * @def elm_obj_list_bounce_get
 * @since 1.8
 *
 * Get the bouncing behaviour of the internal scroller.
 *
 * @param[out] h_bounce
 * @param[out] v_bounce
 *
 * @see elm_list_bounce_get
 */
#define elm_obj_list_bounce_get(h_bounce, v_bounce) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_BOUNCE_GET), EO_TYPECHECK(Eina_Bool *, h_bounce), EO_TYPECHECK(Eina_Bool *, v_bounce)

/**
 * @def elm_obj_list_scroller_policy_set
 * @since 1.8
 *
 * Set the scrollbar policy.
 *
 * @param[in] policy_h
 * @param[in] policy_v
 *
 * @see elm_list_scroller_policy_set
 */
#define elm_obj_list_scroller_policy_set(policy_h, policy_v) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SCROLLER_POLICY_SET), EO_TYPECHECK(Elm_Scroller_Policy, policy_h), EO_TYPECHECK(Elm_Scroller_Policy, policy_v)

/**
 * @def elm_obj_list_scroller_policy_get
 * @since 1.8
 *
 * Get the scrollbar policy.
 *
 * @param[out] policy_h
 * @param[out] policy_v
 *
 * @see elm_list_scroller_policy_get
 */
#define elm_obj_list_scroller_policy_get(policy_h, policy_v) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SCROLLER_POLICY_GET), EO_TYPECHECK(Elm_Scroller_Policy *, policy_h), EO_TYPECHECK(Elm_Scroller_Policy *, policy_v)

/**
 * @def elm_obj_list_clear
 * @since 1.8
 *
 * Remove all list's items.
 *
 *
 * @see elm_list_clear
 */
#define elm_obj_list_clear() ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_CLEAR)

/**
 * @def elm_obj_list_items_get
 * @since 1.8
 *
 * Get a list of all the list items.
 *
 * @param[out] ret
 *
 * @see elm_list_items_get
 */
#define elm_obj_list_items_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_list_selected_item_get
 * @since 1.8
 *
 * Get the selected item.
 *
 * @param[out] ret
 *
 * @see elm_list_selected_item_get
 */
#define elm_obj_list_selected_item_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_selected_items_get
 * @since 1.8
 *
 * Return a list of the currently selected list items.
 *
 * @param[out] ret
 *
 * @see elm_list_selected_items_get
 */
#define elm_obj_list_selected_items_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_SELECTED_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_list_item_append
 * @since 1.8
 *
 * Append a new item to the list object.
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] end
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_list_item_append
 */
#define elm_obj_list_item_append(label, icon, end, func, data, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Object *, end), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_item_prepend
 * @since 1.8
 *
 * Prepend a new item to the list object.
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] end
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_list_item_prepend
 */
#define elm_obj_list_item_prepend(label, icon, end, func, data, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Object *, end), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_item_insert_before
 * @since 1.8
 *
 * Insert a new item into the list object before item before.
 *
 * @param[in] before
 * @param[in] label
 * @param[in] icon
 * @param[in] end
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_list_item_insert_before
 */
#define elm_obj_list_item_insert_before(before, label, icon, end, func, data, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Object *, end), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_item_insert_after
 * @since 1.8
 *
 * Insert a new item into the list object after item after.
 *
 * @param[in] after
 * @param[in] label
 * @param[in] icon
 * @param[in] end
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_list_item_insert_after
 */
#define elm_obj_list_item_insert_after(after, label, icon, end, func, data, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Object *, end), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_item_sorted_insert
 * @since 1.8
 *
 * Insert a new item into the sorted list object.
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] end
 * @param[in] func
 * @param[in] data
 * @param[in] cmp_func
 * @param[out] ret
 *
 * @see elm_list_item_sorted_insert
 */
#define elm_obj_list_item_sorted_insert(label, icon, end, func, data, cmp_func, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Object *, end), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Compare_Cb, cmp_func), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_first_item_get
 * @since 1.8
 *
 * Get the first item in the list
 *
 * @param[out] ret
 *
 * @see elm_list_first_item_get
 */
#define elm_obj_list_first_item_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_last_item_get
 * @since 1.8
 *
 * Get the last item in the list
 *
 * @param[out] ret
 *
 * @see elm_list_last_item_get
 */
#define elm_obj_list_last_item_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_at_xy_item_get
 * @since 1.8
 *
 * Get the item that is at the x, y canvas coords.
 *
 * @param[in] x
 * @param[in] y
 * @param[out] posret
 * @param[out] ret
 *
 * @see elm_list_at_xy_item_get
 */
#define elm_obj_list_at_xy_item_get(x, y, posret, ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_AT_XY_ITEM_GET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(int *, posret), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_list_focus_on_selection_set
 * @since 1.8
 *
 * Set focus to a first from left focusable widget upon item selection.
 *
 * @param[in] enabled
 *
 * @see elm_obj_list_focus_on_selection_get
 */
#define elm_obj_list_focus_on_selection_set(enabled) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_FOCUS_ON_SELECTION_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_list_focus_on_selection_get
 * @since 1.8
 *
 * Get whether the focus will be set to a widget on an item upon it's selection.
 *
 * @param[out] ret
 *
 * @see elm_obj_list_focus_on_selection_set
 */
#define elm_obj_list_focus_on_selection_get(ret) ELM_OBJ_LIST_ID(ELM_OBJ_LIST_SUB_ID_FOCUS_ON_SELECTION_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */
