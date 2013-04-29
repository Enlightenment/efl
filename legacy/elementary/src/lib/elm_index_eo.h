/**
 * @ingroup Index
 *
 * @{
 */
#define ELM_OBJ_INDEX_CLASS elm_obj_index_class_get()

const Eo_Class *elm_obj_index_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_INDEX_BASE_ID;

enum
{
   ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET,
   ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET,
   ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT,
   ELM_OBJ_INDEX_SUB_ID_ITEM_FIND,
   ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR,
   ELM_OBJ_INDEX_SUB_ID_LEVEL_GO,
   ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET,
   ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET,
   ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET,
   ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET,
   ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_SET,
   ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_GET,
   ELM_OBJ_INDEX_SUB_ID_LAST
};

#define ELM_OBJ_INDEX_ID(sub_id) (ELM_OBJ_INDEX_BASE_ID + sub_id)


/**
 * @def elm_obj_index_autohide_disabled_set
 * @since 1.8
 *
 * Enable or disable auto hiding feature for a given index widget.
 *
 * @param[in] disabled
 *
 * @see elm_index_autohide_disabled_set
 */
#define elm_obj_index_autohide_disabled_set(disabled) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_index_autohide_disabled_get
 * @since 1.8
 *
 * Get whether auto hiding feature is enabled or not for a given index widget.
 *
 * @param[out] ret
 *
 * @see elm_index_autohide_disabled_get
 */
#define elm_obj_index_autohide_disabled_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_item_level_set
 * @since 1.8
 *
 * Set the items level for a given index widget.
 *
 * @param[in] level
 *
 * @see elm_index_item_level_set
 */
#define elm_obj_index_item_level_set(level) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET), EO_TYPECHECK(int, level)

/**
 * @def elm_obj_index_item_level_get
 * @since 1.8
 *
 * Get the items level set for a given index widget.
 *
 * @param[out] ret
 *
 * @see elm_index_item_level_get
 */
#define elm_obj_index_item_level_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_index_selected_item_get
 * @since 1.8
 *
 * Returns the last selected item, for a given index widget.
 *
 * @param[in] level
 * @param[out] ret
 *
 * @see elm_index_selected_item_get
 */
#define elm_obj_index_selected_item_get(level, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(int, level), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_append
 * @since 1.8
 *
 * Append a new item on a given index widget.
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_append
 */
#define elm_obj_index_item_append(letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_prepend
 * @since 1.8
 *
 * Prepend a new item on a given index widget.
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_prepend
 */
#define elm_obj_index_item_prepend(letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_insert_after
 * @since 1.8
 *
 * Insert a new item into the index object after item after.
 *
 * @param[in] after
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_insert_after
 */
#define elm_obj_index_item_insert_after(after, letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_insert_before
 * @since 1.8
 *
 * Insert a new item into the index object before item before.
 *
 * @param[in] before
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_insert_before
 */
#define elm_obj_index_item_insert_before(before, letter, func, data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_sorted_insert
 * @since 1.8
 *
 * Insert a new item into the given index widget, using cmp_func
 *
 * @param[in] letter
 * @param[in] func
 * @param[in] data
 * @param[in] cmp_func
 * @param[in] cmp_data_func
 * @param[out] ret
 *
 * @see elm_index_item_sorted_insert
 */
#define elm_obj_index_item_sorted_insert(letter, func, data, cmp_func, cmp_data_func, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT), EO_TYPECHECK(const char *, letter), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Eina_Compare_Cb, cmp_func), EO_TYPECHECK(Eina_Compare_Cb, cmp_data_func), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_find
 * @since 1.8
 *
 * Find a given index widget's item, <b>using item data</b>.
 *
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_index_item_find
 */
#define elm_obj_index_item_find(data, ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_FIND), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_index_item_clear
 * @since 1.8
 *
 * Removes all items from a given index widget.
 *
 *
 * @see elm_index_item_clear
 */
#define elm_obj_index_item_clear() ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR)

/**
 * @def elm_obj_index_level_go
 * @since 1.8
 *
 * Go to a given items level on a index widget
 *
 * @param[in] level
 *
 * @see elm_index_level_go
 */
#define elm_obj_index_level_go(level) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_LEVEL_GO), EO_TYPECHECK(int, level)

/**
 * @def elm_obj_index_indicator_disabled_set
 * @since 1.8
 *
 * Set the indicator as to be disabled.
 *
 * @param[in] disabled
 *
 * @see elm_index_indicator_disabled_set
 */
#define elm_obj_index_indicator_disabled_set(disabled) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_index_indicator_disabled_get
 * @since 1.8
 *
 * Get the value of indicator's disabled status.
 *
 * @param[out] ret
 *
 * @see elm_index_indicator_disabled_get
 */
#define elm_obj_index_indicator_disabled_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_horizontal_set
 * @since 1.8
 *
 * Enable or disable horizontal mode on the index object
 *
 * @param[in] horizontal
 *
 * @see elm_index_horizontal_set
 */
#define elm_obj_index_horizontal_set(horizontal) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_index_horizontal_get
 * @since 1.8
 *
 * Get a value whether horizontal mode is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_index_horizontal_get
 */
#define elm_obj_index_horizontal_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_index_delay_change_time_set
 * @since 1.8
 *
 * Change index delay change time.
 *
 * @param[in] ret
 *
 * @see elm_index_delay_change_time_get
 */
#define elm_obj_index_delay_change_time_set(dtime) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET), EO_TYPECHECK(double, dtime)

/**
 * @def elm_obj_index_delay_change_time_get
 * @since 1.8
 *
 * Get index delay change time.
 *
 * @param[out] ret
 *
 * @see elm_index_delay_change_time_set
 */
#define elm_obj_index_delay_change_time_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_index_omit_enabled_set
 * @since 1.8
 *
 * Enable or disable omit feature for a given index widget.
 *
 * @param[in] enabled
 *
 * @see elm_index_omit_enabled_set
 */
#define elm_obj_index_omit_enabled_set(enabled) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_index_omit_enabled_get
 * @since 1.8
 *
 * Get whether omit feature is enabled or not for a given index widget.
 *
 * @param[out] ret
 *
 * @see elm_index_omit_enabled_get
 */
#define elm_obj_index_omit_enabled_get(ret) ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @}
 */
