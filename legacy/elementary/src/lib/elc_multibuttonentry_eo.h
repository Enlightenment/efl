#define ELM_OBJ_MULTIBUTTONENTRY_CLASS elm_obj_multibuttonentry_class_get()

const Eo_Class *elm_obj_multibuttonentry_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_MULTIBUTTONENTRY_BASE_ID;

enum
{
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ENTRY_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EXPANDED_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EXPANDED_SET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EDITABLE_SET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EDITABLE_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_APPEND,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEMS_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_CLEAR,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_APPEND,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_PREPEND,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_REMOVE,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_FORMAT_FUNCTION_SET,
   ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_LAST
};

#define ELM_OBJ_MULTIBUTTONENTRY_ID(sub_id) (ELM_OBJ_MULTIBUTTONENTRY_BASE_ID + sub_id)


/**
 * @def elm_obj_multibuttonentry_entry_get
 * @since 1.8
 *
 * Get the entry of the multibuttonentry object
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_entry_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_entry_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ENTRY_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_multibuttonentry_expanded_get
 * @since 1.8
 *
 * Get the value of expanded state.
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_expanded_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_expanded_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EXPANDED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_multibuttonentry_expanded_set
 * @since 1.8
 *
 * Set/Unset the multibuttonentry to expanded state.
 *
 * @param[in] expanded
 *
 * @see elm_multibuttonentry_expanded_set
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_expanded_set(expanded) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EXPANDED_SET), EO_TYPECHECK(Eina_Bool, expanded)

/**
 * @def elm_obj_multibuttonentry_editable_set
 * @since 1.8
 *
 * Sets if the multibuttonentry is to be editable or not.
 *
 * @param[in] editable
 *
 * @see elm_multibuttonentry_editable_set
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_editable_set(editable) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, editable)

/**
 * @def elm_obj_multibuttonentry_editable_get
 * @since 1.8
 *
 * Get whether the multibuttonentry is editable or not.
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_editable_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_editable_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_EDITABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_multibuttonentry_item_prepend
 * @since 1.8
 *
 * Prepend a new item to the multibuttonentry
 *
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_multibuttonentry_item_prepend
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_prepend(label, func, data, ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_item_append
 * @since 1.8
 *
 * Append a new item to the multibuttonentry
 *
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_multibuttonentry_item_append
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_append(label, func, data, ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_item_insert_before
 * @since 1.8
 *
 * Add a new item to the multibuttonentry before the indicated object
 *
 * @param[in] before
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_multibuttonentry_item_insert_before
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_insert_before(before, label, func, data, ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_item_insert_after
 * @since 1.8
 *
 * Add a new item to the multibuttonentry after the indicated object
 *
 * @param[in] after
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_multibuttonentry_item_insert_after
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_insert_after(after, label, func, data, ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_items_get
 * @since 1.8
 *
 * Get a list of items in the multibuttonentry
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_items_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_items_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_multibuttonentry_first_item_get
 * @since 1.8
 *
 * Get the first item in the multibuttonentry
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_first_item_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_first_item_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_last_item_get
 * @since 1.8
 *
 * Get the last item in the multibuttonentry
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_last_item_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_last_item_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_selected_item_get
 * @since 1.8
 *
 * Get the selected item in the multibuttonentry
 *
 * @param[out] ret
 *
 * @see elm_multibuttonentry_selected_item_get
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_selected_item_get(ret) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_multibuttonentry_clear
 * @since 1.8
 *
 * Remove all items in the multibuttonentry.
 *
 *
 * @see elm_multibuttonentry_clear
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_clear() ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_CLEAR)

/**
 * @def elm_obj_multibuttonentry_item_filter_append
 * @since 1.8
 *
 * Append an item filter function for text inserted in the Multibuttonentry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_multibuttonentry_item_filter_append
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_filter_append(func, data) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_APPEND), EO_TYPECHECK(Elm_Multibuttonentry_Item_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_multibuttonentry_item_filter_prepend
 * @since 1.8
 *
 * Prepend a filter function for text inserted in the Multibuttonentry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_multibuttonentry_item_filter_prepend
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_filter_prepend(func, data) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_PREPEND), EO_TYPECHECK(Elm_Multibuttonentry_Item_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_multibuttonentry_item_filter_remove
 * @since 1.8
 *
 * Remove a filter from the list
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_multibuttonentry_item_filter_remove
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_item_filter_remove(func, data) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_ITEM_FILTER_REMOVE), EO_TYPECHECK(Elm_Multibuttonentry_Item_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_multibuttonentry_format_function_set
 * @since 1.9
 *
 * Set a function to format the string that will be used to display
 * the hidden items counter.
 *
 * @param[in] format_function The actual format function
 * @param[in] data User data to passed to @a format_function
 *
 * If @a format_function is @c NULL, the default format will be used,
 * which is @c "... + %d".
 *
 * @see elm_multibuttonentry_format_function_set
 *
 * @ingroup Multibuttonentry
 */
#define elm_obj_multibuttonentry_format_function_set(format_function, data) ELM_OBJ_MULTIBUTTONENTRY_ID(ELM_OBJ_MULTIBUTTONENTRY_SUB_ID_FORMAT_FUNCTION_SET), EO_TYPECHECK(Elm_Multibuttonentry_Format_Cb, format_function), EO_TYPECHECK(const void *, data)
