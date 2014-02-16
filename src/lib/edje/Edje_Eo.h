#define EDJE_OBJ_CLASS edje_object_class_get()

const Eo_Class *edje_object_class_get(void) EINA_CONST;

extern EAPI Eo_Op EDJE_OBJ_BASE_ID;

enum
{
   EDJE_OBJ_SUB_ID_SCALE_SET,
   EDJE_OBJ_SUB_ID_SCALE_GET,
   EDJE_OBJ_SUB_ID_MIRRORED_GET,
   EDJE_OBJ_SUB_ID_MIRRORED_SET,
   EDJE_OBJ_SUB_ID_DATA_GET,
   EDJE_OBJ_SUB_ID_FREEZE,
   EDJE_OBJ_SUB_ID_THAW,
   EDJE_OBJ_SUB_ID_COLOR_CLASS_SET,
   EDJE_OBJ_SUB_ID_COLOR_CLASS_GET,
   EDJE_OBJ_SUB_ID_TEXT_CLASS_SET,
   EDJE_OBJ_SUB_ID_PART_EXISTS,
   EDJE_OBJ_SUB_ID_PART_OBJECT_GET,
   EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET,
   EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP,
   EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK,
   EDJE_OBJ_SUB_ID_PART_TEXT_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL,
   EDJE_OBJ_SUB_ID_PART_TEXT_INSERT,
   EDJE_OBJ_SUB_ID_PART_TEXT_APPEND,
   EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN,
   EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND,
   EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_SET,
   EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_GET,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL,
   EDJE_OBJ_SUB_ID_PART_SWALLOW,
   EDJE_OBJ_SUB_ID_PART_UNSWALLOW,
   EDJE_OBJ_SUB_ID_PART_SWALLOW_GET,
   EDJE_OBJ_SUB_ID_SIZE_MIN_GET,
   EDJE_OBJ_SUB_ID_SIZE_MAX_GET,
   EDJE_OBJ_SUB_ID_CALC_FORCE,
   EDJE_OBJ_SUB_ID_SIZE_MIN_CALC,
   EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC,
   EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC,
   EDJE_OBJ_SUB_ID_PART_STATE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET,
   EDJE_OBJ_SUB_ID_PART_DRAG_STEP,
   EDJE_OBJ_SUB_ID_PART_DRAG_PAGE,
   EDJE_OBJ_SUB_ID_PART_BOX_APPEND,
   EDJE_OBJ_SUB_ID_PART_BOX_PREPEND,
   EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE,
   EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT,
   EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL,
   EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_PACK,
   EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK,
   EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET,
   EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR,
   EDJE_OBJ_SUB_ID_PERSPECTIVE_SET,
   EDJE_OBJ_SUB_ID_PERSPECTIVE_GET,
   EDJE_OBJ_SUB_ID_PRELOAD,
   EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET,
   EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET,

   EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET,
   EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET,

   EDJE_OBJ_SUB_ID_FILE_SET,
   EDJE_OBJ_SUB_ID_MMAP_SET,
   EDJE_OBJ_SUB_ID_FILE_GET,
   EDJE_OBJ_SUB_ID_LOAD_ERROR_GET,

   EDJE_OBJ_SUB_ID_MESSAGE_SEND,
   EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET,
   EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS,

   EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD,
   EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL,
   EDJE_OBJ_SUB_ID_SIGNAL_EMIT,
   EDJE_OBJ_SUB_ID_PLAY_SET,
   EDJE_OBJ_SUB_ID_PLAY_GET,
   EDJE_OBJ_SUB_ID_ANIMATION_SET,
   EDJE_OBJ_SUB_ID_ANIMATION_GET,

   EDJE_OBJ_SUB_ID_LAST
};

#define EDJE_OBJ_ID(sub_id) (EDJE_OBJ_BASE_ID + sub_id)

/**
 * @ingroup Edje_Object_Scale
 *
 * @{
 */

/**
 * @def edje_obj_scale_set
 * @since 1.8
 *
 * @brief Set the scaling factor for a given Edje object.
 *
 * @param[in] scale
 * @param[out] ret
 *
 * @see edje_object_scale_set
 */
#define edje_obj_scale_set(scale, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_SET), EO_TYPECHECK(double, scale), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_scale_get
 * @since 1.8
 *
 * @brief Get a given Edje object's scaling factor.
 *
 * @param[out] ret
 *
 * @see edje_object_scale_get
 */
#define edje_obj_scale_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SCALE_GET), EO_TYPECHECK(double *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Entry
 *
 * @{
 */

/**
 * @def edje_obj_mirrored_get
 * @since 1.8
 *
 * @brief Get the RTL orientation for this object.
 *
 * @param[out] ret
 *
 * @see edje_object_mirrored_get
 */
#define edje_obj_mirrored_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_mirrored_set
 * @since 1.8
 *
 * @brief Set the RTL orientation for this object.
 *
 * @param[in] rtl
 *
 * @see edje_object_mirrored_set
 */
#define edje_obj_mirrored_set(rtl) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MIRRORED_SET), EO_TYPECHECK(Eina_Bool, rtl)

/**
 * @def edje_obj_item_provider_set
 * @since 1.8
 *
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_item_provider_set
 */
#define edje_obj_item_provider_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ITEM_PROVIDER_SET), EO_TYPECHECK(Edje_Item_Provider_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_part_text_imf_context_reset
 * @since 1.8
 *
 * @brief Reset the input method context if needed.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_imf_context_reset
 */
#define edje_obj_part_text_imf_context_reset(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_RESET), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_imf_context_get
 * @since 1.8
 *
 * @brief Get the input method context in entry.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_imf_context_get
 */
#define edje_obj_part_text_imf_context_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_IMF_CONTEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_part_text_input_panel_layout_set
 * @since 1.8
 *
 * @brief Set the layout of the input panel.
 *
 * @param[in] part
 * @param[in] layout
 *
 * @see edje_object_part_text_input_panel_layout_set
 */
#define edje_obj_part_text_input_panel_layout_set(part, layout) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Layout, layout)

/**
 * @def edje_obj_part_text_input_panel_layout_get
 * @since 1.8
 *
 * @brief Get the layout of the input panel.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_layout_get
 */
#define edje_obj_part_text_input_panel_layout_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Layout *, ret)

/**
 * @def edje_obj_part_text_input_panel_variation_get
 * @since 1.8
 *
 * @brief Get the layout variation of the input panel.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_variation_get
 */
#define edje_obj_part_text_input_panel_variation_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_part_text_input_panel_variation_set
 * @since 1.8
 *
 * @brief Set the layout variation of the input panel.
 *
 * @param[in] part
 * @param[in] variation
 *
 * @see edje_object_part_text_input_panel_variation_get
 */
#define edje_obj_part_text_input_panel_variation_set(part, variation) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LAYOUT_VARIATION_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(int, variation)

/**
 * @def edje_obj_part_text_autocapital_type_set
 * @since 1.8
 *
 * @brief Set the autocapitalization type on the immodule.
 *
 * @param[in] part
 * @param[in] autocapital_type
 *
 * @see edje_object_part_text_autocapital_type_set
 */
#define edje_obj_part_text_autocapital_type_set(part, autocapital_type) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Autocapital_Type, autocapital_type)

/**
 * @def edje_obj_part_text_autocapital_type_get
 * @since 1.8
 *
 * @brief Retrieves the autocapitalization type
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_autocapital_type_get
 */
#define edje_obj_part_text_autocapital_type_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_AUTOCAPITAL_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Autocapital_Type *, ret)

/**
 * @def edje_obj_part_text_prediction_allow_set
 * @since 1.8
 *
 * @brief Set whether the prediction is allowed or not.
 *
 * @param[in] part
 * @param[in] prediction
 *
 * @see edje_object_part_text_prediction_allow_set
 */
#define edje_obj_part_text_prediction_allow_set(part, prediction) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, prediction)

/**
 * @def edje_obj_part_text_prediction_allow_get
 * @since 1.8
 *
 * @brief Get whether the prediction is allowed or not.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_prediction_allow_get
 */
#define edje_obj_part_text_prediction_allow_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_PREDICTION_ALLOW_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_input_panel_enabled_set
 * @since 1.8
 *
 * @brief Sets the attribute to show the input panel automatically.
 *
 * @param[in] part
 * @param[in] enabled
 *
 * @see edje_object_part_text_input_panel_enabled_set
 */
#define edje_obj_part_text_input_panel_enabled_set(part, enabled) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def edje_obj_part_text_input_panel_enabled_get
 * @since 1.8
 *
 * @brief Retrieve the attribute to show the input panel automatically.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_enabled_get
 */
#define edje_obj_part_text_input_panel_enabled_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_ENABLED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_input_panel_show
 * @since 1.8
 *
 * @brief Show the input panel (virtual keyboard) based on the input panel property such as layout, autocapital types, and so on.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_input_panel_show
 */
#define edje_obj_part_text_input_panel_show(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_input_panel_hide
 * @since 1.8
 *
 * @brief Hide the input panel (virtual keyboard).
 *
 * @param[in] part
 *
 * @see edje_object_part_text_input_panel_hide
 */
#define edje_obj_part_text_input_panel_hide(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_HIDE), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_input_panel_language_set
 * @since 1.8
 *
 * Set the language mode of the input panel.
 *
 * @param[in] part
 * @param[in] lang
 *
 * @see edje_object_part_text_input_panel_language_set
 */
#define edje_obj_part_text_input_panel_language_set(part, lang) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Lang, lang)

/**
 * @def edje_obj_part_text_input_panel_language_get
 * @since 1.8
 *
 * Get the language mode of the input panel.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_language_get
 */
#define edje_obj_part_text_input_panel_language_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_LANGUAGE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Lang *, ret)

/**
 * @def edje_obj_part_text_input_panel_imdata_set
 * @since 1.8
 *
 * Set the input panel-specific data to deliver to the input panel.
 *
 * @param[in] part
 * @param[in] data
 * @param[in] len
 *
 * @see edje_object_part_text_input_panel_imdata_set
 */
#define edje_obj_part_text_input_panel_imdata_set(part, data, len) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const void *, data), EO_TYPECHECK(int, len)

/**
 * @def edje_obj_part_text_input_panel_imdata_get
 * @since 1.8
 *
 * Get the specific data of the current active input panel.
 *
 * @param[in] part
 * @param[in] data
 * @param[in] len
 *
 * @see edje_object_part_text_input_panel_imdata_get
 */
#define edje_obj_part_text_input_panel_imdata_get(part, data, len) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_IMDATA_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(void *, data), EO_TYPECHECK(int *, len)

/**
 * @def edje_obj_part_text_input_panel_return_key_type_set
 * @since 1.8
 *
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * @param[in] part
 * @param[out] return_key_type
 *
 * @see edje_object_part_text_input_panel_return_key_type_set
 */
#define edje_obj_part_text_input_panel_return_key_type_set(part, return_key_type) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Return_Key_Type, return_key_type)

/**
 * @def edje_obj_part_text_input_panel_return_key_type_get
 * @since 1.8
 *
 * Get the "return" key type.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_return_key_type_get
 */
#define edje_obj_part_text_input_panel_return_key_type_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Input_Panel_Return_Key_Type *, ret)

/**
 * @def edje_obj_part_text_input_panel_return_key_disabled_set
 * @since 1.8
 *
 * Set the return key on the input panel to be disabled.
 *
 * @param[in] part
 * @param[in] disabled
 *
 * @see edje_object_part_text_input_panel_return_key_disabled_set
 */
#define edje_obj_part_text_input_panel_return_key_disabled_set(part, disabled) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def edje_obj_part_text_input_panel_return_key_disabled_get
 * @since 1.8
 *
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_return_key_disabled_get
 */
#define edje_obj_part_text_input_panel_return_key_disabled_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_RETURN_KEY_DISABLED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_input_panel_show_on_demand_set
 * @since 1.9
 *
 * @brief Set the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param[in] part
 * @param[in] ondemand
 *
 * @see edje_object_part_text_input_panel_show_on_demand_set
 */
#define edje_obj_part_text_input_panel_show_on_demand_set(part, ondemand) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, ondemand)

/**
 * @def edje_obj_part_text_input_panel_show_on_demand_get
 * @since 1.9
 *
 * @brief Get the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_input_panel_show_on_demand_get
 */
#define edje_obj_part_text_input_panel_show_on_demand_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INPUT_PANEL_SHOW_ON_DEMAND_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_text_insert_filter_callback_add
 * @since 1.8
 *
 * Add a filter function for newly inserted text.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_insert_filter_callback_add
 */
#define edje_obj_text_insert_filter_callback_add(part, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_ADD), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_text_insert_filter_callback_del
 * @since 1.8
 *
 * Delete a function from the filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[out] ret
 *
 * @see edje_object_text_insert_filter_callback_del
 */
#define edje_obj_text_insert_filter_callback_del(part, func, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_insert_filter_callback_del_full
 * @since 1.8
 *
 * Delete a function and matching user data from the filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_text_insert_filter_callback_del_full
 */
#define edje_obj_text_insert_filter_callback_del_full(part, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_INSERT_FILTER_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Text_Filter_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_markup_filter_callback_add
 * @since 1.8
 *
 * Add a markup filter function for newly inserted text.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_markup_filter_callback_add
 */
#define edje_obj_text_markup_filter_callback_add(part, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_ADD), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_text_markup_filter_callback_del
 * @since 1.8
 *
 * Delete a function from the markup filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[out] ret
 *
 * @see edje_object_text_markup_filter_callback_del
 */
#define edje_obj_text_markup_filter_callback_del(part, func, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_text_markup_filter_callback_del_full
 * @since 1.8
 *
 * Delete a function and matching user data from the markup filter list.
 *
 * @param[in] part
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_text_markup_filter_callback_del_full
 */
#define edje_obj_text_markup_filter_callback_del_full(part, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_MARKUP_FILTER_CALLBACK_DEL_FULL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Markup_Filter_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_File
 *
 * @{
 */

/**
 * @def edje_obj_data_get
 * @since 1.8
 *
 * @brief Retrieve an <b>EDC data field's value</b> from a given Edje.
 *
 * @param[in] key
 * @param[out] ret
 *
 * @see edje_object_data_get
 */
#define edje_obj_data_get(key, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_DATA_GET), EO_TYPECHECK(const char *, key), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_file_set
 * @since 1.8
 *
 * @param[in] file in
 * @param[in] group in
 * @param[out] ret out
 *
 * @see edje_object_file_set
 */
#define edje_obj_file_set(file, group, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_SET), EO_TYPECHECK(const char*, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_mmap_set
 * @since 1.8
 *
 * @param[in] file in
 * @param[in] group in
 * @param[out] ret out
 *
 * @see edje_object_file_set
 */
#define edje_obj_mmap_set(file, group, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MMAP_SET), EO_TYPECHECK(const Eina_File*, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_file_get
 * @since 1.8
 *
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * @param[out] file
 * @param[out] group
 *
 * @see edje_object_file_get
 */
#define edje_obj_file_get(file, group) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, group)

/**
 * @def edje_obj_load_error_get
 * @since 1.8
 *
 * @brief Gets the (last) file loading error for a given Edje object
 *
 * @param[out] ret
 *
 * @see edje_object_load_error_get
 */
#define edje_obj_load_error_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_LOAD_ERROR_GET), EO_TYPECHECK(Edje_Load_Error *, ret)
/**
 * @}
 */

/**
 * @ingroup Edje_Object_Animation
 *
 * @{
 */

/**
 * @def edje_obj_play_set
 * @since 1.8
 *
 * @brief Set the Edje object to playing or paused states.
 *
 * @param[in] play
 *
 * @see edje_object_play_set
 */
#define edje_obj_play_set(play) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_SET), EO_TYPECHECK(Eina_Bool, play)

/**
 * @def edje_obj_play_get
 * @since 1.8
 *
 * @brief Get the Edje object's state.
 *
 * @param[out] ret
 *
 * @see edje_object_play_get
 */
#define edje_obj_play_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PLAY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_animation_set
 * @since 1.8
 *
 * @brief Set the object's animation state.
 *
 * @param[in] on
 *
 * @see edje_object_animation_set
 */
#define edje_obj_animation_set(on) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_SET), EO_TYPECHECK(Eina_Bool, on)

/**
 * @def edje_obj_animation_get
 * @since 1.8
 *
 * @brief Get the Edje object's animation state.
 *
 * @param[out] ret
 *
 * @see edje_object_animation_get
 */
#define edje_obj_animation_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ANIMATION_GET), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @def edje_obj_freeze
 * @since 1.8
 *
 * @brief Freezes the Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_freeze
 */
#define edje_obj_freeze(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_FREEZE), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_thaw
 * @since 1.8
 *
 * @brief Thaws the Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_thaw
 */
#define edje_obj_thaw(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_THAW), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_part_state_get
 * @since 1.8
 *
 * @brief Returns the state of the Edje part.
 *
 * @param[in] part
 * @param[out] val_ret
 * @param[out] ret
 *
 * @see edje_object_part_state_get
 */
#define edje_obj_part_state_get(part, val_ret, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_STATE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, val_ret), EO_TYPECHECK(const char **, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Color_Class
 *
 * @{
 */

/**
 * @def edje_obj_color_class_set
 * @since 1.8
 *
 * @brief Sets the object color class.
 *
 * @param[in] color_class
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 * @param[in] r2
 * @param[in] g2
 * @param[in] b2
 * @param[in] a2
 * @param[in] r3
 * @param[in] g3
 * @param[in] b3
 * @param[in] a3
 * @param[out] ret
 *
 * @see edje_object_color_class_set
 */
#define edje_obj_color_class_set(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_SET), EO_TYPECHECK(const char *, color_class), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a), EO_TYPECHECK(int, r2), EO_TYPECHECK(int, g2), EO_TYPECHECK(int, b2), EO_TYPECHECK(int, a2), EO_TYPECHECK(int, r3), EO_TYPECHECK(int, g3), EO_TYPECHECK(int, b3), EO_TYPECHECK(int, a3), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_color_class_get
 * @since 1.8
 *
 * @brief Gets the object color class.
 *
 * @param[in] color_class
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 * @param[out] r2
 * @param[out] g2
 * @param[out] b2
 * @param[out] a2
 * @param[out] r3
 * @param[out] g3
 * @param[out] b3
 * @param[out] a3
 * @param[out] ret
 *
 * @see edje_object_color_class_get
 */
#define edje_obj_color_class_get(color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_COLOR_CLASS_GET), EO_TYPECHECK(const char *, color_class), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a), EO_TYPECHECK(int *, r2), EO_TYPECHECK(int *, g2), EO_TYPECHECK(int *, b2), EO_TYPECHECK(int *, a2), EO_TYPECHECK(int *, r3), EO_TYPECHECK(int *, g3), EO_TYPECHECK(int *, b3), EO_TYPECHECK(int *, a3), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Text_Class
 *
 * @{
 */

/**
 * @def edje_obj_text_class_set
 * @since 1.8
 *
 * @brief Sets Edje text class.
 *
 * @param[in] text_class
 * @param[in] font
 * @param[in] size
 * @param[out] ret
 *
 * @see edje_object_text_class_set
 */
#define edje_obj_text_class_set(text_class, font, size, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CLASS_SET), EO_TYPECHECK(const char *, text_class), EO_TYPECHECK(const char *, font), EO_TYPECHECK(Evas_Font_Size, size), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Part
 *
 * @{
 */

/**
 * @def edje_obj_part_exists
 * @since 1.8
 *
 * @brief Check if an Edje part exists in a given Edje object's group
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_exists
 */
#define edje_obj_part_exists(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXISTS), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_object_get
 * @since 1.8
 *
 * @brief Get a handle to the Evas object implementing a given Edje
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_object_get
 */
#define edje_obj_part_object_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_OBJECT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Evas_Object **, ret)

/**
 * @def edje_obj_part_geometry_get
 * @since 1.8
 *
 * @brief Retrieve the geometry of a given Edje part, in a given Edje
 *
 * @param[in] part
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see edje_object_part_geometry_get
 */
#define edje_obj_part_geometry_get(part, x, y, w, h, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_access_part_list_get
 * @since 1.8
 *
 * @brief Retrieve a list all accessibility part names
 *
 * @param[out] ret
 *
 * @see edje_object_access_part_list_get
 */
#define edje_obj_access_part_list_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_ACCESS_PART_LIST_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Text
 *
 * @{
 */

/**
 * @def edje_obj_text_change_cb_set
 * @since 1.8
 *
 * @brief Set the object text callback.
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_text_change_cb_set
 */
#define edje_obj_text_change_cb_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_TEXT_CHANGE_CB_SET), EO_TYPECHECK(Edje_Text_Change_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_part_text_style_user_push
 * @since 1.8
 *
 * @brief Set the style of the part.
 *
 * @param[in] part
 * @param[in] style
 *
 * @see edje_object_part_text_style_user_push
 */
#define edje_obj_part_text_style_user_push(part, style) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PUSH), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, style)

/**
 * @def edje_obj_part_text_style_user_pop
 * @since 1.8
 *
 * @brief Delete the top style form the user style stack.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_style_user_pop
 */
#define edje_obj_part_text_style_user_pop(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_POP), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_style_user_peek
 * @since 1.8
 *
 * @brief Return the text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_style_user_peek
 */
#define edje_obj_part_text_style_user_peek(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_STYLE_USER_PEEK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_set
 * @since 1.8
 *
 * @brief Sets the text for an object part
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see edje_object_part_text_set
 */
#define edje_obj_part_text_set(part, text, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_get
 * @since 1.8
 *
 * @brief Return the text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_get
 */
#define edje_obj_part_text_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_escaped_set
 * @since 1.8
 *
 * @brief Sets the text for an object part, but converts HTML escapes to UTF8
 *
 * @param[in] part
 * @param[in] text
 * @param[out] ret
 *
 * @see edje_object_part_text_escaped_set
 */
#define edje_obj_part_text_escaped_set(part, text, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ESCAPED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_unescaped_set
 * @since 1.8
 *
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param[in] part
 * @param[in] text_to_escape
 * @param[out] ret
 *
 * @see edje_object_part_text_unescaped_set
 */
#define edje_obj_part_text_unescaped_set(part, text_to_escape, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text_to_escape), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_unescaped_get
 * @since 1.8
 *
 * @brief Returns the text of the object part, without escaping.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_unescaped_get
 */
#define edje_obj_part_text_unescaped_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_UNESCAPED_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(char **, ret)

/**
 * @def edje_obj_part_text_insert
 * @since 1.8
 *
 * @brief Insert text for an object part.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_insert
 */
#define edje_obj_part_text_insert(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_INSERT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @def edje_obj_part_text_append
 * @since 1.8
 *
 * @brief Insert text for an object part.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_append
 */
#define edje_obj_part_text_append(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @def edje_obj_part_text_anchor_list_get
 * @since 1.8
 *
 * @brief Return a list of char anchor names.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_anchor_list_get
 */
#define edje_obj_part_text_anchor_list_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_LIST_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_anchor_geometry_get
 * @since 1.8
 *
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param[in] part
 * @param[in] anchor
 * @param[out] ret
 *
 * @see edje_object_part_text_anchor_geometry_get
 */
#define edje_obj_part_text_anchor_geometry_get(part, anchor, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ANCHOR_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, anchor), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_item_list_get
 * @since 1.8
 *
 * @brief Return a list of char item names.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_item_list_get
 */
#define edje_obj_part_text_item_list_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_LIST_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def edje_obj_part_text_item_geometry_get
 * @since 1.8
 *
 * @brief Return item geometry.
 *
 * @param[in] part
 * @param[in] item
 * @param[out] cx
 * @param[out] cy
 * @param[out] cw
 * @param[out] ch
 * @param[out] ret
 *
 * @see edje_object_part_text_item_geometry_get
 */
#define edje_obj_part_text_item_geometry_get(part, item, cx, cy, cw, ch, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_ITEM_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, item), EO_TYPECHECK(Evas_Coord *, cx), EO_TYPECHECK(Evas_Coord *, cy), EO_TYPECHECK(Evas_Coord *, cw), EO_TYPECHECK(Evas_Coord *, ch), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_user_insert
 * @since 1.8
 *
 * @brief This function inserts text as if the user has inserted it.
 *
 * @param[in] part
 * @param[in] text
 *
 * @see edje_object_part_text_user_insert
 */
#define edje_obj_part_text_user_insert(part, text) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_USER_INSERT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, text)

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Selection
 *
 * @{
 */

/**
 * @def edje_obj_part_text_selection_get
 * @since 1.8
 *
 * @brief Return the selection text of the object part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_text_selection_get
 */
#define edje_obj_part_text_selection_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECTION_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char **, ret)

/**
 * @def edje_obj_part_text_select_none
 * @since 1.8
 *
 * @brief Set the selection to be none.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_none
 */
#define edje_obj_part_text_select_none(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_NONE), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_all
 * @since 1.8
 *
 * @brief Set the selection to be everything.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_all
 */
#define edje_obj_part_text_select_all(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALL), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_allow_set
 * @since 1.8
 *
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 *
 * @param[in] part
 * @param[in] allow
 *
 * @see edje_object_part_text_select_allow_set
 */
#define edje_obj_part_text_select_allow_set(part, allow) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ALLOW_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, allow)

/**
 * @def edje_obj_part_text_select_abort
 * @since 1.8
 *
 * @brief Aborts any selection action on a part.
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_abort
 */
#define edje_obj_part_text_select_abort(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_ABORT), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_begin
 * @since 1.8
 *
 * @brief Starts selecting at current cursor position
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_begin
 */
#define edje_obj_part_text_select_begin(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_BEGIN), EO_TYPECHECK(const char *, part)

/**
 * @def edje_obj_part_text_select_extend
 * @since 1.8
 *
 * @brief Extends the current selection to the current cursor position
 *
 * @param[in] part
 *
 * @see edje_object_part_text_select_extend
 */
#define edje_obj_part_text_select_extend(part) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_SELECT_EXTEND), EO_TYPECHECK(const char *, part)

/**
 * @}
 */

/**
 * @ingroup Edje_Text_Cursor
 *
 * @{
 */

/**
 * @def edje_obj_part_text_cursor_next
 * @since 1.8
 *
 * @brief Advances the cursor to the next cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_next
 */
#define edje_obj_part_text_cursor_next(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_NEXT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_prev
 * @since 1.8
 *
 * @brief Moves the cursor to the previous char
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_prev
 */
#define edje_obj_part_text_cursor_prev(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_PREV), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_up
 * @since 1.8
 *
 * @brief Move the cursor to the char above the current cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_up
 */
#define edje_obj_part_text_cursor_up(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_UP), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_down
 * @since 1.8
 *
 * @brief Moves the cursor to the char below the current cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_down
 */
#define edje_obj_part_text_cursor_down(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_DOWN), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_begin_set
 * @since 1.8
 *
 * @brief Moves the cursor to the beginning of the text part
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_begin_set
 */
#define edje_obj_part_text_cursor_begin_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_BEGIN_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_end_set
 * @since 1.8
 *
 * @brief Moves the cursor to the end of the text part.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_end_set
 */
#define edje_obj_part_text_cursor_end_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_END_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_copy
 * @since 1.8
 *
 * @brief Copy the cursor to another cursor.
 *
 * @param[in] part
 * @param[in] src
 * @param[in] dst
 *
 * @see edje_object_part_text_cursor_copy
 */
#define edje_obj_part_text_cursor_copy(part, src, dst) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COPY), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, src), EO_TYPECHECK(Edje_Cursor, dst)

/**
 * @def edje_obj_part_text_cursor_line_begin_set
 * @since 1.8
 *
 * @brief Move the cursor to the beginning of the line.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_line_begin_set
 */
#define edje_obj_part_text_cursor_line_begin_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_BEGIN_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_line_end_set
 * @since 1.8
 *
 * @brief Move the cursor to the end of the line.
 *
 * @param[in] part
 * @param[in] cur
 *
 * @see edje_object_part_text_cursor_line_end_set
 */
#define edje_obj_part_text_cursor_line_end_set(part, cur) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_LINE_END_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur)

/**
 * @def edje_obj_part_text_cursor_coord_set
 * @since 1.8
 *
 * Position the given cursor to a X,Y position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[in] x
 * @param[in] y
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_coord_set
 */
#define edje_obj_part_text_cursor_coord_set(part, cur, x, y, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_COORD_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_is_format_get
 * @since 1.8
 *
 * @brief Returns whether the cursor points to a format.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_is_format_get
 */
#define edje_obj_part_text_cursor_is_format_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_FORMAT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_is_visible_format_get
 * @since 1.8
 *
 * @brief Return true if the cursor points to a visible format
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_is_visible_format_get
 */
#define edje_obj_part_text_cursor_is_visible_format_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_IS_VISIBLE_FORMAT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_text_cursor_content_get
 * @since 1.8
 *
 * @brief Returns the content (char) at the cursor position.
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_content_get
 */
#define edje_obj_part_text_cursor_content_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_CONTENT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(char **, ret)

/**
 * @def edje_obj_part_text_cursor_pos_set
 * @since 1.8
 *
 * @brief Sets the cursor position to the given value
 *
 * @param[in] part
 * @param[in] cur
 * @param[in] pos
 *
 * @see edje_object_part_text_cursor_pos_set
 */
#define edje_obj_part_text_cursor_pos_set(part, cur, pos) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(int, pos)

/**
 * @def edje_obj_part_text_cursor_pos_get
 * @since 1.8
 *
 * @brief Retrieves the current position of the cursor
 *
 * @param[in] part
 * @param[in] cur
 * @param[out] ret
 *
 * @see edje_object_part_text_cursor_pos_get
 */
#define edje_obj_part_text_cursor_pos_get(part, cur, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_POS_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Cursor, cur), EO_TYPECHECK(int *, ret)

/**
 * @def edje_obj_part_text_cursor_geometry_get
 * @since 1.8
 *
 * @brief Returns the cursor geometry of the part relative to the edje
 *
 * @param[in] part
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see edje_object_part_text_cursor_geometry_get
 */
#define edje_obj_part_text_cursor_geometry_get(part, x, y, w, h) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TEXT_CURSOR_GEOMETRY_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h)

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Swallow
 *
 * @{
 */

/**
 * @def edje_obj_part_swallow
 * @since 1.8
 *
 * @brief "Swallows" an object into one of the Edje object SWALLOW
 *
 * @param[in] part
 * @param[in] obj_swallow
 * @param[out] ret
 *
 * @see edje_object_part_swallow
 */
#define edje_obj_part_swallow(part, obj_swallow, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, obj_swallow), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_unswallow
 * @since 1.8
 *
 * @brief Unswallow an object.
 *
 * @param[in] obj_swallow
 *
 * @see edje_object_part_unswallow
 */
#define edje_obj_part_unswallow(obj_swallow) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_UNSWALLOW), EO_TYPECHECK(Evas_Object *, obj_swallow)

/**
 * @def edje_obj_part_swallow_get
 * @since 1.8
 *
 * @brief Get the object currently swallowed by a part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_swallow_get
 */
#define edje_obj_part_swallow_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_SWALLOW_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Geometry_Group
 *
 * @{
 */

/**
 * @def edje_obj_size_min_get
 * @since 1.8
 *
 * @brief Get the minimum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param[out] minw
 * @param[out] minh
 *
 * @see edje_object_size_min_get
 */
#define edje_obj_size_min_get(minw, minh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_GET), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh)

/**
 * @def edje_obj_update_hints_set
 * @since 1.8
 *
 * @brief Edje will automatically update the size hints on itself.
 *
 * @param[in] update
 *
 * @see edje_object_update_hints_set
 */
#define edje_obj_update_hints_set(update) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_SET), EO_TYPECHECK(Eina_Bool, update)

/**
 * @def edje_obj_update_hints_get
 * @since 1.8
 *
 * @brief Wether or not Edje will update size hints on itself.
 *
 * @param[out] ret
 *
 * @see edje_object_update_hints_get
 */
#define edje_obj_update_hints_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_UPDATE_HINTS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_size_max_get
 * @since 1.8
 *
 * @brief Get the maximum size specified -- as an EDC property -- for a
 * given Edje object
 *
 * @param[out] maxw
 * @param[out] maxh
 *
 * @see edje_object_size_max_get
 */
#define edje_obj_size_max_get(maxw, maxh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MAX_GET), EO_TYPECHECK(Evas_Coord *, maxw), EO_TYPECHECK(Evas_Coord *, maxh)

/**
 * @def edje_obj_calc_force
 * @since 1.8
 *
 * @brief Force a Size/Geometry calculation.
 *
 *
 * @see edje_object_calc_force
 */
#define edje_obj_calc_force() EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_CALC_FORCE)

/**
 * @def edje_obj_size_min_calc
 * @since 1.8
 *
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param[out] minw
 * @param[out] minh
 *
 * @see edje_object_size_min_calc
 */
#define edje_obj_size_min_calc(minw, minh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_CALC), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh)

/**
 * @def edje_obj_parts_extends_calc
 * @since 1.8
 *
 * Calculate the geometry of the region, relative to a given Edje
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see edje_object_parts_extends_calc
 */
#define edje_obj_parts_extends_calc(x, y, w, h, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PARTS_EXTENDS_CALC), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_size_min_restricted_calc
 * @since 1.8
 *
 * @brief Calculate the minimum required size for a given Edje object.
 *
 * @param[out] minw
 * @param[out] minh
 * @param[in] restrictedw
 * @param[in] restrictedh
 *
 * @see edje_object_size_min_restricted_calc
 */
#define edje_obj_size_min_restricted_calc(minw, minh, restrictedw, restrictedh) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIZE_MIN_RESTRICTED_CALC), EO_TYPECHECK(Evas_Coord *, minw), EO_TYPECHECK(Evas_Coord *, minh), EO_TYPECHECK(Evas_Coord, restrictedw), EO_TYPECHECK(Evas_Coord, restrictedh)

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Drag
 *
 * @{
 */

/**
 * @def edje_obj_part_drag_dir_get
 * @since 1.8
 *
 * @brief Determine dragable directions.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_drag_dir_get
 */
#define edje_obj_part_drag_dir_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_DIR_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_Drag_Dir *, ret)

/**
 * @def edje_obj_part_drag_value_set
 * @since 1.8
 *
 * @brief Set the dragable object location.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_value_set
 */
#define edje_obj_part_drag_value_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_value_get
 * @since 1.8
 *
 * @brief Get the dragable object location.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_value_get
 */
#define edje_obj_part_drag_value_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_VALUE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_size_set
 * @since 1.8
 *
 * @brief Set the dragable object size.
 *
 * @param[in] part
 * @param[in] dw
 * @param[in] dh
 * @param[out] ret
 *
 * @see edje_object_part_drag_size_set
 */
#define edje_obj_part_drag_size_set(part, dw, dh, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dw), EO_TYPECHECK(double, dh), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_size_get
 * @since 1.8
 *
 * @brief Get the dragable object size.
 *
 * @param[in] part
 * @param[out] dw
 * @param[out] dh
 * @param[out] ret
 *
 * @see edje_object_part_drag_size_get
 */
#define edje_obj_part_drag_size_get(part, dw, dh, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_SIZE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dw), EO_TYPECHECK(double *, dh), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step_set
 * @since 1.8
 *
 * @brief Sets the drag step increment.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step_set
 */
#define edje_obj_part_drag_step_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step_get
 * @since 1.8
 *
 * @brief Gets the drag step increment values.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step_get
 */
#define edje_obj_part_drag_step_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page_set
 * @since 1.8
 *
 * @brief Sets the page step increments.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page_set
 */
#define edje_obj_part_drag_page_set(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page_get
 * @since 1.8
 *
 * @brief Gets the page step increments.
 *
 * @param[in] part
 * @param[out] dx
 * @param[out] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page_get
 */
#define edje_obj_part_drag_page_get(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double *, dx), EO_TYPECHECK(double *, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_step
 * @since 1.8
 *
 * @brief Steps the dragable x,y steps.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_step
 */
#define edje_obj_part_drag_step(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_STEP), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_drag_page
 * @since 1.8
 *
 * @brief Pages x,y steps.
 *
 * @param[in] part
 * @param[in] dx
 * @param[in] dy
 * @param[out] ret
 *
 * @see edje_object_part_drag_page
 */
#define edje_obj_part_drag_page(part, dx, dy, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_DRAG_PAGE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(double, dx), EO_TYPECHECK(double, dy), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Box
 *
 * @{
 */

/**
 * @def edje_obj_part_box_append
 * @since 1.8
 *
 * @brief Appends an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_append
 */
#define edje_obj_part_box_append(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_APPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_prepend
 * @since 1.8
 *
 * @brief Prepends an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_prepend
 */
#define edje_obj_part_box_prepend(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_PREPEND), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_insert_before
 * @since 1.8
 *
 * @brief Adds an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] reference
 * @param[out] ret
 *
 * @see edje_object_part_box_insert_before
 */
#define edje_obj_part_box_insert_before(part, child, reference, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_BEFORE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(const Evas_Object *, reference), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_insert_at
 * @since 1.8
 *
 * @brief Inserts an object to the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[in] pos
 * @param[out] ret
 *
 * @see edje_object_part_box_insert_at
 */
#define edje_obj_part_box_insert_at(part, child, pos, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_INSERT_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_box_remove
 * @since 1.8
 *
 * @brief Removes an object from the box.
 *
 * @param[in] part
 * @param[in] child
 * @param[out] ret
 *
 * @see edje_object_part_box_remove
 */
#define edje_obj_part_box_remove(part, child, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_box_remove_at
 * @since 1.8
 *
 * @brief Removes an object from the box.
 *
 * @param[in] part
 * @param[in] pos
 * @param[out] ret
 *
 * @see edje_object_part_box_remove_at
 */
#define edje_obj_part_box_remove_at(part, pos, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_AT), EO_TYPECHECK(const char *, part), EO_TYPECHECK(unsigned int, pos), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_box_remove_all
 * @since 1.8
 *
 * @brief Removes all elements from the box.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see edje_object_part_box_remove_all
 */
#define edje_obj_part_box_remove_all(part, clear, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_BOX_REMOVE_ALL), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Part_Table
 *
 * @{
 */

/**
 * @def edje_obj_part_table_child_get
 * @since 1.8
 *
 * @brief Retrieve a child from a table
 *
 * @param[in] part
 * @param[in] col
 * @param[in] row
 * @param[out] ret
 *
 * @see edje_object_part_table_child_get
 */
#define edje_obj_part_table_child_get(part, col, row, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CHILD_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(unsigned int, col), EO_TYPECHECK(unsigned int, row), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_table_pack
 * @since 1.8
 *
 * @brief Packs an object into the table.
 *
 * @param[in] part
 * @param[in] child_obj
 * @param[in] col
 * @param[in] row
 * @param[in] colspan
 * @param[in] rowspan
 * @param[out] ret
 *
 * @see edje_object_part_table_pack
 */
#define edje_obj_part_table_pack(part, child_obj, col, row, colspan, rowspan, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_PACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child_obj), EO_TYPECHECK(unsigned short, col), EO_TYPECHECK(unsigned short, row), EO_TYPECHECK(unsigned short, colspan), EO_TYPECHECK(unsigned short, rowspan), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_unpack
 * @since 1.8
 *
 * @brief Removes an object from the table.
 *
 * @param[in] part
 * @param[in] child_obj
 * @param[out] ret
 *
 * @see edje_object_part_table_unpack
 */
#define edje_obj_part_table_unpack(part, child_obj, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_UNPACK), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object *, child_obj), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_col_row_size_get
 * @since 1.8
 *
 * @brief Gets the number of columns and rows the table has.
 *
 * @param[in] part
 * @param[out] cols
 * @param[out] rows
 * @param[out] ret
 *
 * @see edje_object_part_table_col_row_size_get
 */
#define edje_obj_part_table_col_row_size_get(part, cols, rows, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_COL_ROW_SIZE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(int *, cols), EO_TYPECHECK(int *, rows), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_table_clear
 * @since 1.8
 *
 * @brief Removes all object from the table.
 *
 * @param[in] part
 * @param[in] clear
 * @param[out] ret
 *
 * @see edje_object_part_table_clear
 */
#define edje_obj_part_table_clear(part, clear, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_TABLE_CLEAR), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Eina_Bool, clear), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Perspective
 *
 * @{
 */

/**
 * @def edje_obj_perspective_set
 * @since 1.8
 *
 * Set the given perspective object on this Edje object.
 *
 * @param[in] ps
 *
 * @see edje_object_perspective_set
 */
#define edje_obj_perspective_set(ps) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_SET), EO_TYPECHECK(Edje_Perspective *, ps)

/**
 * @def edje_obj_perspective_get
 * @since 1.8
 *
 * Get the current perspective used on this Edje object.
 *
 * @param[out] ret
 *
 * @see edje_object_perspective_get
 */
#define edje_obj_perspective_get(ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PERSPECTIVE_GET), EO_TYPECHECK(const Edje_Perspective **, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Group
 *
 * @{
 */

/**
 * @def edje_obj_preload
 * @since 1.8
 *
 * @brief Preload the images on the Edje Object in the background.
 *
 * @param[in] cancel
 * @param[out] ret
 *
 * @see edje_object_preload
 */
#define edje_obj_preload(cancel, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PRELOAD), EO_TYPECHECK(Eina_Bool, cancel), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_External_Group
 *
 * @{
 */

/**
 * @def edje_obj_part_external_object_get
 * @since 1.8
 *
 * @brief Get the object created by this external part.
 *
 * @param[in] part
 * @param[out] ret
 *
 * @see edje_object_part_external_object_get
 */
#define edje_obj_part_external_object_get(part, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_OBJECT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_external_param_set
 * @since 1.8
 *
 * @brief Set the parameter for the external part.
 *
 * @param[in] part
 * @param[in] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_set
 */
#define edje_obj_part_external_param_set(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_SET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const Edje_External_Param *, param), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_external_param_get
 * @since 1.8
 *
 * @brief Get the parameter for the external part.
 *
 * @param[in] part
 * @param[out] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_get
 */
#define edje_obj_part_external_param_get(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(Edje_External_Param *, param), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def edje_obj_part_external_content_get
 * @since 1.8
 *
 * @brief Get an object contained in an part of type EXTERNAL
 *
 * @param[in] part
 * @param[out] content
 * @param[out] ret
 *
 * @see edje_object_part_external_content_get
 */
#define edje_obj_part_external_content_get(part, content, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_CONTENT_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, content), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def edje_obj_part_external_param_type_get
 * @since 1.8
 *
 * Facility to query the type of the given parameter of the given part.
 *
 * @param[in] part
 * @param[out] param
 * @param[out] ret
 *
 * @see edje_object_part_external_param_type_get
 */
#define edje_obj_part_external_param_type_get(part, param, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_PART_EXTERNAL_PARAM_TYPE_GET), EO_TYPECHECK(const char *, part), EO_TYPECHECK(const char *, param), EO_TYPECHECK(Edje_External_Param_Type *, ret)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Communication_Interface_Message
 *
 * @{
 */

/**
 * @def edje_obj_message_send
 * @since 1.8
 *
 * @brief Send an (Edje) message to a given Edje object
 *
 * @param[in] type
 * @param[in] id
 * @param[in] msg
 *
 * @see edje_object_message_send
 */
#define edje_obj_message_send(type, id, msg) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SEND), EO_TYPECHECK(Edje_Message_Type, type), EO_TYPECHECK(int, id), EO_TYPECHECK(void *, msg)

/**
 * @def edje_obj_message_handler_set
 * @since 1.8
 *
 * @brief Set an Edje message handler function for a given Edje object.
 *
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_message_handler_set
 */
#define edje_obj_message_handler_set(func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_HANDLER_SET), EO_TYPECHECK(Edje_Message_Handler_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_message_signal_process
 * @since 1.8
 *
 * @brief Process an object's message queue.
 *
 *
 * @see edje_object_message_signal_process
 */
#define edje_obj_message_signal_process() EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_MESSAGE_SIGNAL_PROCESS)

/**
 * @}
 */

/**
 * @ingroup Edje_Object_Communication_Interface_Signal
 *
 * @{
 */

/**
 * @def edje_obj_signal_callback_add
 * @since 1.8
 *
 * @brief Add a callback for an arriving Edje signal, emitted by
 * a given Ejde object.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 *
 * @see edje_object_signal_callback_add
 */
#define edje_obj_signal_callback_add(emission, source, func, data) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_ADD), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def edje_obj_signal_callback_del
 * @since 1.8
 *
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param[in] emission
 * @param[in] source
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see edje_object_signal_callback_del
 */
#define edje_obj_signal_callback_del(emission, source, func, data, ret) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_CALLBACK_DEL), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source), EO_TYPECHECK(Edje_Signal_Cb, func), EO_TYPECHECK(void *, data), EO_TYPECHECK(void **, ret)

/**
 * @def edje_obj_signal_emit
 * @since 1.8
 *
 * @brief Send/emit an Edje signal to a given Edje object
 *
 * @param[in] emission
 * @param[in] source
 *
 * @see edje_object_signal_emit
 */
#define edje_obj_signal_emit(emission, source) EDJE_OBJ_ID(EDJE_OBJ_SUB_ID_SIGNAL_EMIT), EO_TYPECHECK(const char *, emission), EO_TYPECHECK(const char *, source)

/**
 * @}
 */

#define EDJE_EDIT_CLASS edje_edit_class_get()

const Eo_Class *edje_edit_class_get(void) EINA_CONST;
