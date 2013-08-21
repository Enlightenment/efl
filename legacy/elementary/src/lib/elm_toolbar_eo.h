/**
 * @ingroup Toolbar
 *
 * @{
 */
#define ELM_OBJ_TOOLBAR_CLASS elm_obj_toolbar_class_get()

const Eo_Class *elm_obj_toolbar_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_TOOLBAR_BASE_ID;

enum
{
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT,
   ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_TOOLBAR_SUB_ID_LAST
};

#define ELM_OBJ_TOOLBAR_ID(sub_id) (ELM_OBJ_TOOLBAR_BASE_ID + sub_id)


/**
 * @def elm_obj_toolbar_icon_size_set
 * @since 1.8
 *
 * Set the icon size, in pixels, to be used by toolbar items.
 *
 * @param[in] icon_size
 *
 * @see elm_toolbar_icon_size_set
 */
#define elm_obj_toolbar_icon_size_set(icon_size) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET), EO_TYPECHECK(int, icon_size)

/**
 * @def elm_obj_toolbar_icon_size_get
 * @since 1.8
 *
 * Get the icon size, in pixels, to be used by toolbar items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_icon_size_get
 */
#define elm_obj_toolbar_icon_size_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_toolbar_item_append
 * @since 1.8
 *
 * Append item to the toolbar.
 *
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_append
 */
#define elm_obj_toolbar_item_append(icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_prepend
 * @since 1.8
 *
 * Prepend item to the toolbar.
 *
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_prepend
 */
#define elm_obj_toolbar_item_prepend(icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_insert_before
 * @since 1.8
 *
 * Insert a new item into the toolbar object before item before.
 *
 * @param[in] before
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_insert_before
 */
#define elm_obj_toolbar_item_insert_before(before, icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_insert_after
 * @since 1.8
 *
 * Insert a new item into the toolbar object after item after.
 *
 * @param[in] after
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_toolbar_item_insert_after
 */
#define elm_obj_toolbar_item_insert_after(after, icon, label, func, data, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_first_item_get
 * @since 1.8
 *
 * Get the first item in the given toolbar widget's list of
 * items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_first_item_get
 */
#define elm_obj_toolbar_first_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_last_item_get
 * @since 1.8
 *
 * Get the last item in the given toolbar widget's list of
 * items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_last_item_get
 */
#define elm_obj_toolbar_last_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_item_find_by_label
 * @since 1.8
 *
 * Returns a pointer to a toolbar item by its label.
 *
 * @param[in] label
 * @param[out] ret
 *
 * @see elm_toolbar_item_find_by_label
 */
#define elm_obj_toolbar_item_find_by_label(label, ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_selected_item_get
 * @since 1.8
 *
 * Get the selected item.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_selected_item_get
 */
#define elm_obj_toolbar_selected_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_more_item_get
 * @since 1.8
 *
 * Get the more item.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_more_item_get
 */
#define elm_obj_toolbar_more_item_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_toolbar_shrink_mode_set
 * @since 1.8
 *
 * Set the item displaying mode of a given toolbar widget obj.
 *
 * @param[in] shrink_mode
 *
 * @see elm_toolbar_shrink_mode_set
 */
#define elm_obj_toolbar_shrink_mode_set(shrink_mode) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET), EO_TYPECHECK(Elm_Toolbar_Shrink_Mode, shrink_mode)

/**
 * @def elm_obj_toolbar_shrink_mode_get
 * @since 1.8
 *
 * Get the shrink mode of toolbar obj.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_shrink_mode_get
 */
#define elm_obj_toolbar_shrink_mode_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET), EO_TYPECHECK(Elm_Toolbar_Shrink_Mode *, ret)

/**
 * @def elm_obj_toolbar_homogeneous_set
 * @since 1.8
 *
 * Enable/disable homogeneous mode.
 *
 * @param[in] homogeneous
 *
 * @see elm_toolbar_homogeneous_set
 */
#define elm_obj_toolbar_homogeneous_set(homogeneous) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET), EO_TYPECHECK(Eina_Bool, homogeneous)

/**
 * @def elm_obj_toolbar_homogeneous_get
 * @since 1.8
 *
 * Get whether the homogeneous mode is enabled.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_homogeneous_get
 */
#define elm_obj_toolbar_homogeneous_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_toolbar_menu_parent_set
 * @since 1.8
 *
 * Set the parent object of the toolbar items' menus.
 *
 * @param[in] parent
 *
 * @see elm_toolbar_menu_parent_set
 */
#define elm_obj_toolbar_menu_parent_set(parent) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_toolbar_menu_parent_get
 * @since 1.8
 *
 * Get the parent object of the toolbar items' menus.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_menu_parent_get
 */
#define elm_obj_toolbar_menu_parent_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_toolbar_align_set
 * @since 1.8
 *
 * Set the alignment of the items.
 *
 * @param[in] align
 *
 * @see elm_toolbar_align_set
 */
#define elm_obj_toolbar_align_set(align) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, align)

/**
 * @def elm_obj_toolbar_align_get
 * @since 1.8
 *
 * Get the alignment of the items.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_align_get
 */
#define elm_obj_toolbar_align_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_toolbar_icon_order_lookup_set
 * @since 1.8
 *
 * Sets icon lookup order, for toolbar items' icons.
 *
 * @param[in] order
 *
 * @see elm_toolbar_icon_order_lookup_set
 */
#define elm_obj_toolbar_icon_order_lookup_set(order) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET), EO_TYPECHECK(Elm_Icon_Lookup_Order, order)

/**
 * @def elm_obj_toolbar_icon_order_lookup_get
 * @since 1.8
 *
 * Get the icon lookup order.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_icon_order_lookup_get
 */
#define elm_obj_toolbar_icon_order_lookup_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET), EO_TYPECHECK(Elm_Icon_Lookup_Order *, ret)

/**
 * @def elm_obj_toolbar_horizontal_set
 * @since 1.8
 *
 * Change a toolbar's orientation
 *
 * @param[in] horizontal
 *
 * @see elm_toolbar_horizontal_set
 */
#define elm_obj_toolbar_horizontal_set(horizontal) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_toolbar_horizontal_get
 * @since 1.8
 *
 * Get a toolbar's orientation
 *
 * @param[out] ret
 *
 * @see elm_toolbar_horizontal_get
 */
#define elm_obj_toolbar_horizontal_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_toolbar_items_count
 * @since 1.8
 *
 * Get the number of items in a toolbar
 *
 * @param[out] ret
 *
 * @see elm_toolbar_items_count
 */
#define elm_obj_toolbar_items_count(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_toolbar_standard_priority_set
 * @since 1.8
 *
 * Set the standard priority of visible items in a toolbar
 *
 * @param[in] priority
 *
 * @see elm_toolbar_standard_priority_set
 */
#define elm_obj_toolbar_standard_priority_set(priority) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET), EO_TYPECHECK(int, priority)

/**
 * @def elm_obj_toolbar_standard_priority_get
 * @since 1.8
 *
 * Get the standard_priority of visible items in a toolbar
 *
 * @param[out] ret
 *
 * @see elm_toolbar_standard_priority_get
 */
#define elm_obj_toolbar_standard_priority_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_toolbar_select_mode_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] mode
 *
 * @see elm_toolbar_select_mode_set
 */
#define elm_obj_toolbar_select_mode_set(mode) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Object_Select_Mode, mode)

/**
 * @def elm_obj_toolbar_select_mode_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret
 *
 * @see elm_toolbar_select_mode_get
 */
#define elm_obj_toolbar_select_mode_get(ret) ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Object_Select_Mode *, ret)

/**
 * @}
 */

