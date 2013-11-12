/**
 * @ingroup Layout
 *
 * @{
 */
#define ELM_OBJ_LAYOUT_CLASS elm_obj_layout_class_get()

 const Eo_Class *elm_obj_layout_class_get(void) EINA_CONST;

 extern EAPI Eo_Op ELM_OBJ_LAYOUT_BASE_ID;

 enum
{
   ELM_OBJ_LAYOUT_SUB_ID_FILE_SET,
   ELM_OBJ_LAYOUT_SUB_ID_THEME_SET,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD,
   ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE,
   ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK,
   ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET,
   ELM_OBJ_LAYOUT_SUB_ID_DATA_GET,
   ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET,
   ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET,
   ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET,
   ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET,
   ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET,
   ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE,
   ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE,
   ELM_OBJ_LAYOUT_SUB_ID_FREEZE,
   ELM_OBJ_LAYOUT_SUB_ID_THAW,
   ELM_OBJ_LAYOUT_SUB_ID_SIZING_RESTRICTED_EVAL,
   ELM_OBJ_LAYOUT_SUB_ID_LAST
};

#define ELM_OBJ_LAYOUT_ID(sub_id) (ELM_OBJ_LAYOUT_BASE_ID + sub_id)


/**
 * @def elm_obj_layout_file_set
 * @since 1.8
 *
 * Set the file that will be used as layout
 *
 * @param[in] file
 * @param[in] group
 * @param[out] ret
 *
 * @see elm_layout_file_set
 */
#define elm_obj_layout_file_set(file, group, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_theme_set
 * @since 1.8
 *
 * Set the edje group from the elementary theme that will be used as layout
 *
 * @param[in] klass
 * @param[in] group
 * @param[in] style
 * @param[out] ret
 *
 * @see elm_layout_theme_set
 */
#define elm_obj_layout_theme_set(klass, group, style, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_SET), EO_TYPECHECK(const char *, klass), EO_TYPECHECK(const char *, group), EO_TYPECHECK(const char *, style), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_signal_emit
 * @since 1.8
 *
 * Send a (Edje) signal to a given layout widget's underlying Edje
 *
 * @param[in] emission
 * @param[in] source
 *
 * @see elm_layout_signal_emit
 */
#define elm_obj_layout_signal_emit(emission, source) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_EMIT), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source)

/**
 * @def elm_obj_layout_signal_callback_add
 * @since 1.8
 *
 * Add a callback for a (Edje) signal emitted by a layout widget's
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 *
 * @see elm_layout_signal_callback_add
 */
#define elm_obj_layout_signal_callback_add(emission, source, func, data) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_ADD), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_layout_signal_callback_del
 * @since 1.8
 *
 * Remove a signal-triggered callback from a given layout widget.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[out] ret
 *
 * @see elm_layout_signal_callback_del
 */
#define elm_obj_layout_signal_callback_del(emission, source, func, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIGNAL_CALLBACK_DEL), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def elm_obj_layout_text_set
 * @since 1.8
 *
 * Set the text of the given part
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see elm_layout_text_set
 */
#define elm_obj_layout_text_set(part, text, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_text_get
 * @since 1.8
 *
 * Get the text set in the given part
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see elm_layout_text_get
 */
#define elm_obj_layout_text_get(part, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_box_append
 * @since 1.8
 *
 * Append child to layout box part.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_append
 */
#define elm_obj_layout_box_append(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_prepend
 * @since 1.8
 *
 * Prepend child to layout box part.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_prepend
 */
#define elm_obj_layout_box_prepend(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_PREPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_insert_before
 * @since 1.8
 *
 * Insert child to layout box part before a reference object.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] reference
 * @param[out] ret
 *
 * @see elm_layout_box_insert_before
 */
#define elm_obj_layout_box_insert_before(part, child, reference, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_BEFORE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_insert_at
 * @since 1.8
 *
 * Insert child to layout box part at a given position.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] pos
 * @param[out] ret
 *
 * @see elm_layout_box_insert_at
 */
#define elm_obj_layout_box_insert_at(part, child, pos, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_INSERT_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_box_remove
 * @since 1.8
 *
 * Remove a child of the given part box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_box_remove
 */
#define elm_obj_layout_box_remove(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_box_remove_all
 * @since 1.8
 *
 * Remove all children of the given part box.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see elm_layout_box_remove_all
 */
#define elm_obj_layout_box_remove_all(part, clear, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_BOX_REMOVE_ALL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_table_pack
 * @since 1.8
 *
 * Insert child to layout table part.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see elm_layout_table_pack
 */
#define elm_obj_layout_table_pack(part, child, col, row, colspan, rowspan, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_PACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_table_unpack
 * @since 1.8
 *
 * Unpack (remove) a child of the given part table.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see elm_layout_table_unpack
 */
#define elm_obj_layout_table_unpack(part, child, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_UNPACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_table_clear
 * @since 1.8
 *
 * Remove all the child objects of the given part table.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see elm_layout_table_clear
 */
#define elm_obj_layout_table_clear(part, clear, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TABLE_CLEAR), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_get
 * @since 1.8
 *
 * Get the edje layout
 *
 * @param[out] ret
 *
 * @see elm_layout_edje_get
 */
#define elm_obj_layout_edje_get(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_layout_data_get
 * @since 1.8
 *
 * Get the edje data from the given layout
 *
 * @param[in] key
 * @param[out] ret
 *
 * @see elm_layout_data_get
 */
#define elm_obj_layout_data_get(key, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_DATA_GET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_sizing_eval
 * @since 1.8
 *
 * Eval sizing
 *
 * @see elm_layout_sizing_eval
 */
#define elm_obj_layout_sizing_eval() ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL)

/**
 * @def elm_obj_layout_sizing_restricted_eval
 * @since 1.8
 *
 * Eval sizing, restricted to current width and/or height
 *
 * @param[in] w   Restrict minimum size to the current width
 * @param[in] h   Restrict minimum size ot the current height
 *
 * @see elm_layout_sizing_restricted_eval
 */
#define elm_obj_layout_sizing_restricted_eval(width, height) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_RESTRICTED_EVAL), EO_TYPECHECK(Eina_Bool, width), EO_TYPECHECK(Eina_Bool, height)

/**
 * @def elm_obj_layout_part_cursor_set
 * @since 1.8
 *
 * Sets a specific cursor for an edje part.
 *
 * @param[in] part_name
 * @param[in] cursor
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_set
 */
#define elm_obj_layout_part_cursor_set(part_name, cursor, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char *, cursor), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_get
 * @since 1.8
 *
 * Get the cursor to be shown when mouse is over an edje part
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_get
 */
#define elm_obj_layout_part_cursor_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_part_cursor_unset
 * @since 1.8
 *
 * Unsets a cursor previously set with elm_layout_part_cursor_set().
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_unset
 */
#define elm_obj_layout_part_cursor_unset(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_UNSET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_style_set
 * @since 1.8
 *
 * Sets a specific cursor style for an edje part.
 *
 * @param[in] part_name
 * @param[in] style
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_style_set
 */
#define elm_obj_layout_part_cursor_style_set(part_name, style, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char *, style), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_style_get
 * @since 1.8
 *
 * Get a specific cursor style for an edje part.
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_style_get
 */
#define elm_obj_layout_part_cursor_style_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_STYLE_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_layout_part_cursor_engine_only_set
 * @since 1.8
 *
 * Sets if the cursor set should be searched on the theme or should use
 *
 * @param[in] part_name
 * @param[in] engine_only
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_engine_only_set
 */
#define elm_obj_layout_part_cursor_engine_only_set(part_name, engine_only, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_SET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool, engine_only), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_part_cursor_engine_only_get
 * @since 1.8
 *
 * Get a specific cursor engine_only for an edje part.
 *
 * @param[in] part_name
 * @param[out] ret
 *
 * @see elm_layout_part_cursor_engine_only_get
 */
#define elm_obj_layout_part_cursor_engine_only_get(part_name, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_PART_CURSOR_ENGINE_ONLY_GET), EO_TYPECHECK(const char *, part_name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_object_can_access_set
 * @since 1.8
 *
 * Set accessibility to all texblock(text) parts in the layout object
 *
 * @param[in] can_access
 * @param[out] ret
 *
 * @see elm_layout_edje_object_can_access_set
 */
#define elm_obj_layout_edje_object_can_access_set(can_access, ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_SET), EO_TYPECHECK(Eina_Bool, can_access), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_edje_object_can_access_get
 * @since 1.8
 *
 * Get accessibility state of texblock(text) parts in the layout object
 *
 * @param[out] ret
 *
 * @see elm_layout_edje_object_can_access_get
 */
#define elm_obj_layout_edje_object_can_access_get(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_EDJE_OBJECT_CAN_ACCESS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_layout_smart_text_aliases_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] aliases
 */
#define elm_obj_layout_smart_text_aliases_get(aliases) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), EO_TYPECHECK(const Elm_Layout_Part_Alias_Description **, aliases)

/**
 * @def elm_obj_layout_smart_content_aliases_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[out] aliases
 */
#define elm_obj_layout_smart_content_aliases_get(aliases) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), EO_TYPECHECK(const Elm_Layout_Part_Alias_Description **, aliases)

/**
 * @def elm_obj_layout_sub_object_add_enable
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] enable
 *
 * @see elm_layout_sub_object_add_enable
 */
#define elm_obj_layout_sub_object_add_enable(enable) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SUB_OBJECT_ADD_ENABLE), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def elm_obj_layout_theme_enable
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] enable
 */
#define elm_obj_layout_theme_enable(enable) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THEME_ENABLE), EO_TYPECHECK(Eina_Bool *, enable)

/**
 * @def elm_obj_layout_freeze
 * @since 1.8
 *
 * @brief Freezes the Elementary layout object.
 *
 * @param[out] ret
 *
 * @see elm_layout_freeze
 */
#define elm_obj_layout_freeze(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_FREEZE), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_layout_thaw
 * @since 1.8
 *
 * @brief Thaws the Elementary object.
 *
 * @param[out] ret
 *
 * @see elm_layout_thaw
 */
#define elm_obj_layout_thaw(ret) ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_THAW), EO_TYPECHECK(int *, ret)

/**
 * @}
 */
