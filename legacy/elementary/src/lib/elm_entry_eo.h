#define ELM_OBJ_ENTRY_CLASS elm_obj_entry_class_get()

const Eo_Class *elm_obj_entry_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ENTRY_BASE_ID;

enum
{
   ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_PUSH,
   ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_POP,
   ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_PEEK,
   ELM_OBJ_ENTRY_SUB_ID_SINGLE_LINE_SET,
   ELM_OBJ_ENTRY_SUB_ID_SINGLE_LINE_GET,
   ELM_OBJ_ENTRY_SUB_ID_PASSWORD_SET,
   ELM_OBJ_ENTRY_SUB_ID_PASSWORD_GET,
   ELM_OBJ_ENTRY_SUB_ID_ENTRY_APPEND,
   ELM_OBJ_ENTRY_SUB_ID_IS_EMPTY,
   ELM_OBJ_ENTRY_SUB_ID_TEXTBLOCK_GET,
   ELM_OBJ_ENTRY_SUB_ID_CALC_FORCE,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_GET,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_HANDLER_DISABLED_SET,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_HANDLER_DISABLED_GET,
   ELM_OBJ_ENTRY_SUB_ID_ENTRY_INSERT,
   ELM_OBJ_ENTRY_SUB_ID_LINE_WRAP_SET,
   ELM_OBJ_ENTRY_SUB_ID_LINE_WRAP_GET,
   ELM_OBJ_ENTRY_SUB_ID_EDITABLE_SET,
   ELM_OBJ_ENTRY_SUB_ID_EDITABLE_GET,
   ELM_OBJ_ENTRY_SUB_ID_SELECT_NONE,
   ELM_OBJ_ENTRY_SUB_ID_SELECT_ALL,
   ELM_OBJ_ENTRY_SUB_ID_SELECT_REGION_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_GEOMETRY_GET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_NEXT,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_PREV,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_UP,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_DOWN,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_BEGIN_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_END_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_LINE_BEGIN_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_LINE_END_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_SELECTION_BEGIN,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_SELECTION_END,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_IS_FORMAT_GET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_IS_VISIBLE_FORMAT_GET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_CONTENT_GET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_POS_SET,
   ELM_OBJ_ENTRY_SUB_ID_CURSOR_POS_GET,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_CUT,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_COPY,
   ELM_OBJ_ENTRY_SUB_ID_SELECTION_PASTE,
   ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_CLEAR,
   ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_ITEM_ADD,
   ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_DISABLED_SET,
   ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_DISABLED_GET,
   ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_APPEND,
   ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_PREPEND,
   ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_REMOVE,
   ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_APPEND,
   ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_PREPEND,
   ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_REMOVE,
   ELM_OBJ_ENTRY_SUB_ID_FILE_SET,
   ELM_OBJ_ENTRY_SUB_ID_FILE_GET,
   ELM_OBJ_ENTRY_SUB_ID_FILE_SAVE,
   ELM_OBJ_ENTRY_SUB_ID_AUTOSAVE_SET,
   ELM_OBJ_ENTRY_SUB_ID_AUTOSAVE_GET,
   ELM_OBJ_ENTRY_SUB_ID_CNP_MODE_SET,
   ELM_OBJ_ENTRY_SUB_ID_CNP_MODE_GET,
   ELM_OBJ_ENTRY_SUB_ID_SCROLLABLE_SET,
   ELM_OBJ_ENTRY_SUB_ID_SCROLLABLE_GET,
   ELM_OBJ_ENTRY_SUB_ID_ICON_VISIBLE_SET,
   ELM_OBJ_ENTRY_SUB_ID_END_VISIBLE_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_GET,
   ELM_OBJ_ENTRY_SUB_ID_AUTOCAPITAL_TYPE_SET,
   ELM_OBJ_ENTRY_SUB_ID_AUTOCAPITAL_TYPE_GET,
   ELM_OBJ_ENTRY_SUB_ID_PREDICTION_ALLOW_SET,
   ELM_OBJ_ENTRY_SUB_ID_PREDICTION_ALLOW_GET,
   ELM_OBJ_ENTRY_SUB_ID_IMF_CONTEXT_RESET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_ENABLED_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_ENABLED_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_HIDE,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LANGUAGE_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LANGUAGE_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_IMDATA_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_IMDATA_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_TYPE_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_TYPE_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_DISABLED_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_DISABLED_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_AUTOENABLED_SET,
   ELM_OBJ_ENTRY_SUB_ID_IMF_CONTEXT_GET,
   ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_PARENT_SET,
   ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_PARENT_GET,
   ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_STYLE_SET,
   ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_STYLE_GET,
   ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_END,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_VARIATION_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_VARIATION_GET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW_ON_DEMAND_SET,
   ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW_ON_DEMAND_GET,
   ELM_OBJ_ENTRY_SUB_ID_LAST
};

#define ELM_OBJ_ENTRY_ID(sub_id) (ELM_OBJ_ENTRY_BASE_ID + sub_id)


/**
 * @def elm_obj_entry_text_style_user_push
 * @since 1.8
 *
 * Push the style to the top of user style stack.
 *
 * @param[in] style
 *
 * @see elm_entry_text_style_user_push
 *
 * @ingroup Entry
 */
#define elm_obj_entry_text_style_user_push(style) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_PUSH), EO_TYPECHECK(const char *, style)

/**
 * @def elm_obj_entry_text_style_user_pop
 * @since 1.8
 *
 * Remove the style in the top of user style stack.
 *
 *
 * @see elm_entry_text_style_user_pop
 *
 * @ingroup Entry
 */
#define elm_obj_entry_text_style_user_pop() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_POP)

/**
 * @def elm_obj_entry_text_style_user_peek
 * @since 1.8
 *
 * Retrieve the style on the top of user style stack.
 *
 * @param[out] ret
 *
 * @see elm_entry_text_style_user_peek
 *
 * @ingroup Entry
 */
#define elm_obj_entry_text_style_user_peek(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_TEXT_STYLE_USER_PEEK), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_entry_single_line_set
 * @since 1.8
 *
 * Sets the entry to single line mode.
 *
 * @param[in] single_line
 *
 * @see elm_entry_single_line_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_single_line_set(single_line) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SINGLE_LINE_SET), EO_TYPECHECK(Eina_Bool, single_line)

/**
 * @def elm_obj_entry_single_line_get
 * @since 1.8
 *
 * Get whether the entry is set to be single line.
 *
 * @param[out] ret
 *
 * @see elm_entry_single_line_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_single_line_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SINGLE_LINE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_password_set
 * @since 1.8
 *
 * Sets the entry to password mode.
 *
 * @param[in] password
 *
 * @see elm_entry_password_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_password_set(password) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_PASSWORD_SET), EO_TYPECHECK(Eina_Bool, password)

/**
 * @def elm_obj_entry_password_get
 * @since 1.8
 *
 * Get whether the entry is set to password mode.
 *
 * @param[out] ret
 *
 * @see elm_entry_password_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_password_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_PASSWORD_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_entry_append
 * @since 1.8
 *
 * Appends entry to the text of the entry.
 *
 * @param[in] entry
 *
 * @see elm_entry_entry_append
 *
 * @ingroup Entry
 */
#define elm_obj_entry_entry_append(entry) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ENTRY_APPEND), EO_TYPECHECK(const char *, entry)

/**
 * @def elm_obj_entry_is_empty
 * @since 1.8
 *
 * Get whether the entry is empty.
 *
 * @param[out] ret
 *
 * @see elm_entry_is_empty
 *
 * @ingroup Entry
 */
#define elm_obj_entry_is_empty(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_IS_EMPTY), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_textblock_get
 * @since 1.8
 *
 * Returns the actual textblock object of the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_textblock_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_textblock_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_TEXTBLOCK_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_entry_calc_force
 * @since 1.8
 *
 * Forces calculation of the entry size and text layouting.
 *
 *
 * @see elm_entry_calc_force
 *
 * @ingroup Entry
 */
#define elm_obj_entry_calc_force() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CALC_FORCE)

/**
 * @def elm_obj_entry_selection_get
 * @since 1.8
 *
 * Get any selected text within the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_selection_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_entry_selection_handler_disabled_set
 * @since 1.8
 *
 * This disables entry's selection handlers.
 *
 * @param[in] disabled
 *
 * @see elm_entry_selection_handler_disabled_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_handler_disabled_set(disbaled) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_HANDLER_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_entry_selection_handler_disabled_get
 * @since 1.8
 *
 * This returns whether the entry's selection handlers are disabled.
 *
 * @param[out] ret
 *
 * @see elm_entry_selection_handler_disabled_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_handler_disabled_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_HANDLER_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @def elm_obj_entry_entry_insert
 * @since 1.8
 *
 * Inserts the given text into the entry at the current cursor position.
 *
 * @param[in] entry
 *
 * @see elm_entry_entry_insert
 *
 * @ingroup Entry
 */
#define elm_obj_entry_entry_insert(entry) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ENTRY_INSERT), EO_TYPECHECK(const char *, entry)

/**
 * @def elm_obj_entry_line_wrap_set
 * @since 1.8
 *
 * Set the line wrap type to use on multi-line entries.
 *
 * @param[in] wrap
 *
 * @see elm_entry_line_wrap_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_line_wrap_set(wrap) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_LINE_WRAP_SET), EO_TYPECHECK(Elm_Wrap_Type, wrap)

/**
 * @def elm_obj_entry_line_wrap_get
 * @since 1.8
 *
 * Get the wrap mode the entry was set to use.
 *
 * @param[out] ret
 *
 * @see elm_entry_line_wrap_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_line_wrap_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_LINE_WRAP_GET), EO_TYPECHECK(Elm_Wrap_Type *, ret)

/**
 * @def elm_obj_entry_editable_set
 * @since 1.8
 *
 * Sets if the entry is to be editable or not.
 *
 * @param[in] editable
 *
 * @see elm_entry_editable_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_editable_set(editable) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_EDITABLE_SET), EO_TYPECHECK(Eina_Bool, editable)

/**
 * @def elm_obj_entry_editable_get
 * @since 1.8
 *
 * Get whether the entry is editable or not.
 *
 * @param[out] ret
 *
 * @see elm_entry_editable_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_editable_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_EDITABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_select_none
 * @since 1.8
 *
 * This drops any existing text selection within the entry.
 *
 *
 * @see elm_entry_select_none
 *
 * @ingroup Entry
 */
#define elm_obj_entry_select_none() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECT_NONE)

/**
 * @def elm_obj_entry_select_all
 * @since 1.8
 *
 * This selects all text within the entry.
 *
 *
 * @see elm_entry_select_all
 *
 * @ingroup Entry
 */
#define elm_obj_entry_select_all() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECT_ALL)

/**
 * @def elm_obj_entry_select_region_set
 * @since 1.9
 *
 * This selects a region of text within the entry.
 *
 * @param[in] start
 * @param[in] end
 *
 * @see elm_entry_select_region_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_select_region_set(start, end) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECT_REGION_SET), EO_TYPECHECK(int, start), EO_TYPECHECK(int, end)

/**
 * @def elm_obj_entry_cursor_geometry_get
 * @since 1.8
 *
 * This function returns the geometry of the cursor.
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] ret
 *
 * @see elm_entry_cursor_geometry_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_geometry_get(x, y, w, h, ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_GEOMETRY_GET), EO_TYPECHECK(Evas_Coord *, x), EO_TYPECHECK(Evas_Coord *, y), EO_TYPECHECK(Evas_Coord *, w), EO_TYPECHECK(Evas_Coord *, h), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_next
 * @since 1.8
 *
 * This moves the cursor one place to the right within the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_next
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_next(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_NEXT), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_prev
 * @since 1.8
 *
 * This moves the cursor one place to the left within the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_prev
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_prev(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_PREV), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_up
 * @since 1.8
 *
 * This moves the cursor one line up within the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_up
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_up(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_UP), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_down
 * @since 1.8
 *
 * This moves the cursor one line down within the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_down
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_down(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_DOWN), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_begin_set
 * @since 1.8
 *
 * This moves the cursor to the beginning of the entry.
 *
 *
 * @see elm_entry_cursor_begin_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_begin_set() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_BEGIN_SET)

/**
 * @def elm_obj_entry_cursor_end_set
 * @since 1.8
 *
 * This moves the cursor to the end of the entry.
 *
 *
 * @see elm_entry_cursor_end_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_end_set() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_END_SET)

/**
 * @def elm_obj_entry_cursor_line_begin_set
 * @since 1.8
 *
 * This moves the cursor to the beginning of the current line.
 *
 *
 * @see elm_entry_cursor_line_begin_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_line_begin_set() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_LINE_BEGIN_SET)

/**
 * @def elm_obj_entry_cursor_line_end_set
 * @since 1.8
 *
 * This moves the cursor to the end of the current line.
 *
 *
 * @see elm_entry_cursor_line_end_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_line_end_set() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_LINE_END_SET)

/**
 * @def elm_obj_entry_cursor_selection_begin
 * @since 1.8
 *
 * This begins a selection within the entry as though
 *
 *
 * @see elm_entry_cursor_selection_begin
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_selection_begin() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_SELECTION_BEGIN)

/**
 * @def elm_obj_entry_cursor_selection_end
 * @since 1.8
 *
 * This ends a selection within the entry as though
 *
 *
 * @see elm_entry_cursor_selection_end
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_selection_end() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_SELECTION_END)

/**
 * @def elm_obj_entry_cursor_is_format_get
 * @since 1.8
 *
 * Get whether a format node exists at the current cursor position.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_is_format_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_is_format_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_IS_FORMAT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_is_visible_format_get
 * @since 1.8
 *
 * Get if the current cursor position holds a visible format node.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_is_visible_format_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_is_visible_format_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_IS_VISIBLE_FORMAT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cursor_content_get
 * @since 1.8
 *
 * Get the character pointed by the cursor at its current position.
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_content_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_content_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_CONTENT_GET), EO_TYPECHECK(char **, ret)

/**
 * @def elm_obj_entry_cursor_pos_set
 * @since 1.8
 *
 * Sets the cursor position in the entry to the given value
 *
 * @param[in] pos
 *
 * @see elm_entry_cursor_pos_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_pos_set(pos) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_POS_SET), EO_TYPECHECK(int, pos)

/**
 * @def elm_obj_entry_cursor_pos_get
 * @since 1.8
 *
 * Retrieves the current position of the cursor in the entry
 *
 * @param[out] ret
 *
 * @see elm_entry_cursor_pos_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cursor_pos_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CURSOR_POS_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_entry_selection_cut
 * @since 1.8
 *
 * This executes a "cut" action on the selected text in the entry.
 *
 *
 * @see elm_entry_selection_cut
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_cut() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_CUT)

/**
 * @def elm_obj_entry_selection_copy
 * @since 1.8
 *
 * This executes a "copy" action on the selected text in the entry.
 *
 *
 * @see elm_entry_selection_copy
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_copy() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_COPY)

/**
 * @def elm_obj_entry_selection_paste
 * @since 1.8
 *
 * This executes a "paste" action in the entry.
 *
 *
 * @see elm_entry_selection_paste
 *
 * @ingroup Entry
 */
#define elm_obj_entry_selection_paste() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SELECTION_PASTE)

/**
 * @def elm_obj_entry_context_menu_clear
 * @since 1.8
 *
 * This clears and frees the items in a entry's contextual (longpress)
 *
 *
 * @see elm_entry_context_menu_clear
 *
 * @ingroup Entry
 */
#define elm_obj_entry_context_menu_clear() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_CLEAR)

/**
 * @def elm_obj_entry_context_menu_item_add
 * @since 1.8
 *
 * This adds an item to the entry's contextual menu.
 *
 * @param[in] label
 * @param[in] icon_file
 * @param[in] icon_type
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_context_menu_item_add
 *
 * @ingroup Entry
 */
#define elm_obj_entry_context_menu_item_add(label, icon_file, icon_type, func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_ITEM_ADD), EO_TYPECHECK(const char *, label), EO_TYPECHECK(const char *, icon_file), EO_TYPECHECK(Elm_Icon_Type, icon_type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data)

/**
 * @def elm_obj_entry_context_menu_disabled_set
 * @since 1.8
 *
 * This disables the entry's contextual (longpress) menu.
 *
 * @param[in] disabled
 *
 * @see elm_entry_context_menu_disabled_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_context_menu_disabled_set(disabled) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_entry_context_menu_disabled_get
 * @since 1.8
 *
 * This returns whether the entry's contextual (longpress) menu is
 * disabled.
 *
 * @param[out] ret
 *
 * @see elm_entry_context_menu_disabled_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_context_menu_disabled_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CONTEXT_MENU_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_item_provider_append
 * @since 1.8
 *
 * This appends a custom item provider to the list for that entry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_item_provider_append
 *
 * @ingroup Entry
 */
#define elm_obj_entry_item_provider_append(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_APPEND), EO_TYPECHECK(Elm_Entry_Item_Provider_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_item_provider_prepend
 * @since 1.8
 *
 * This prepends a custom item provider to the list for that entry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_item_provider_prepend
 *
 * @ingroup Entry
 */
#define elm_obj_entry_item_provider_prepend(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_PREPEND), EO_TYPECHECK(Elm_Entry_Item_Provider_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_item_provider_remove
 * @since 1.8
 *
 * This removes a custom item provider to the list for that entry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_item_provider_remove
 *
 * @ingroup Entry
 */
#define elm_obj_entry_item_provider_remove(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ITEM_PROVIDER_REMOVE), EO_TYPECHECK(Elm_Entry_Item_Provider_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_markup_filter_append
 * @since 1.8
 *
 * Append a markup filter function for text inserted in the entry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_markup_filter_append
 *
 * @ingroup Entry
 */
#define elm_obj_entry_markup_filter_append(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_APPEND), EO_TYPECHECK(Elm_Entry_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_markup_filter_prepend
 * @since 1.8
 *
 * Prepend a markup filter function for text inserted in the entry
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_markup_filter_prepend
 *
 * @ingroup Entry
 */
#define elm_obj_entry_markup_filter_prepend(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_PREPEND), EO_TYPECHECK(Elm_Entry_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_markup_filter_remove
 * @since 1.8
 *
 * Remove a markup filter from the list
 *
 * @param[in] func
 * @param[in] data
 *
 * @see elm_entry_markup_filter_remove
 *
 * @ingroup Entry
 */
#define elm_obj_entry_markup_filter_remove(func, data) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_MARKUP_FILTER_REMOVE), EO_TYPECHECK(Elm_Entry_Filter_Cb, func), EO_TYPECHECK(void *, data)

/**
 * @def elm_obj_entry_file_set
 * @since 1.8
 *
 * This sets the file (and implicitly loads it) for the text to display and
 * then edit. All changes are written back to the file after a short delay if
 * the entry object is set to autosave (which is the default).
 *
 * @param[in] file
 * @param[in] format
 * @param[out] ret
 *
 * @see elm_entry_file_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_file_set(file, format, ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(Elm_Text_Format, format), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_file_get
 * @since 1.8
 *
 * Get the file being edited by the entry.
 *
 * @param[out] file
 * @param[out] format
 *
 * @see elm_entry_file_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_file_get(file, format) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(Elm_Text_Format *, format)

/**
 * @def elm_obj_entry_file_save
 * @since 1.8
 *
 * This function writes any changes made to the file set with
 * elm_entry_file_set()
 *
 *
 * @see elm_entry_file_save
 *
 * @ingroup Entry
 */
#define elm_obj_entry_file_save() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_FILE_SAVE)

/**
 * @def elm_obj_entry_autosave_set
 * @since 1.8
 *
 * This sets the entry object to 'autosave' the loaded text file or not.
 *
 * @param[in] auto_save
 *
 * @see elm_entry_autosave_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_autosave_set(auto_save) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_AUTOSAVE_SET), EO_TYPECHECK(Eina_Bool, auto_save)

/**
 * @def elm_obj_entry_autosave_get
 * @since 1.8
 *
 * This gets the entry object's 'autosave' status.
 *
 * @param[out] ret
 *
 * @see elm_entry_autosave_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_autosave_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_AUTOSAVE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_cnp_mode_set
 * @since 1.8
 *
 * Control pasting of text and images for the widget.
 *
 * @param[in] cnp_mode
 *
 * @see elm_entry_cnp_mode_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cnp_mode_set(cnp_mode) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CNP_MODE_SET), EO_TYPECHECK(Elm_Cnp_Mode, cnp_mode)

/**
 * @def elm_obj_entry_cnp_mode_get
 * @since 1.8
 *
 * Getting elm_entry text paste/drop mode.
 *
 * @param[out] ret
 *
 * @see elm_entry_cnp_mode_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_cnp_mode_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_CNP_MODE_GET), EO_TYPECHECK(Elm_Cnp_Mode *, ret)

/**
 * @def elm_obj_entry_scrollable_set
 * @since 1.8
 *
 * Enable or disable scrolling in entry
 *
 * @param[in] scroll
 *
 * @see elm_entry_scrollable_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_scrollable_set(scroll) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SCROLLABLE_SET), EO_TYPECHECK(Eina_Bool, scroll)

/**
 * @def elm_obj_entry_scrollable_get
 * @since 1.8
 *
 * Get the scrollable state of the entry
 *
 * @param[out] ret
 *
 * @see elm_entry_scrollable_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_scrollable_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_SCROLLABLE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_icon_visible_set
 * @since 1.8
 *
 * Sets the visibility of the left-side widget of the entry,
 * set by elm_object_part_content_set().
 *
 * @param[in] setting
 *
 * @see elm_entry_icon_visible_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_icon_visible_set(setting) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ICON_VISIBLE_SET), EO_TYPECHECK(Eina_Bool, setting)

/**
 * @def elm_obj_entry_end_visible_set
 * @since 1.8
 *
 * Sets the visibility of the end widget of the entry, set by
 *
 * @param[in] setting
 *
 * @see elm_entry_end_visible_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_end_visible_set(setting) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_END_VISIBLE_SET), EO_TYPECHECK(Eina_Bool, setting)

/**
 * @def elm_obj_entry_input_panel_layout_set
 * @since 1.8
 *
 * Set the input panel layout of the entry
 *
 * @param[in] layout
 *
 * @see elm_entry_input_panel_layout_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_layout_set(layout) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_SET), EO_TYPECHECK(Elm_Input_Panel_Layout, layout)

/**
 * @def elm_obj_entry_input_panel_layout_get
 * @since 1.8
 *
 * Get the input panel layout of the entry
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_layout_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_layout_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_GET), EO_TYPECHECK(Elm_Input_Panel_Layout *, ret)

/**
 * @def elm_obj_entry_input_panel_layout_variation_set
 * @since 1.8
 *
 * Set the input panel layout variation of the entry
 *
 * @param[in] layout variation
 *
 * @see elm_entry_input_panel_layout_variation_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_layout_variation_set(variation) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_VARIATION_SET), EO_TYPECHECK(int, variation)

/**
 * @def elm_obj_entry_input_panel_layout_variation_get
 * @since 1.8
 *
 * Get the input panel layout variation of the entry
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_layout_variation_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_layout_variation_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LAYOUT_VARIATION_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_entry_autocapital_type_set
 * @since 1.8
 *
 * Set the autocapitalization type on the immodule.
 *
 * @param[in] autocapital_type
 *
 * @see elm_entry_autocapital_type_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_autocapital_type_set(autocapital_type) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_AUTOCAPITAL_TYPE_SET), EO_TYPECHECK(Elm_Autocapital_Type, autocapital_type)

/**
 * @def elm_obj_entry_autocapital_type_get
 * @since 1.8
 *
 * Retrieve the autocapitalization type on the immodule.
 *
 * @param[out] ret
 *
 * @see elm_entry_autocapital_type_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_autocapital_type_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_AUTOCAPITAL_TYPE_GET), EO_TYPECHECK(Elm_Autocapital_Type *, ret)

/**
 * @def elm_obj_entry_prediction_allow_set
 * @since 1.8
 *
 * Set whether the entry should allow to use the text prediction.
 *
 * @param[in] prediction
 *
 * @see elm_entry_prediction_allow_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_prediction_allow_set(prediction) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_PREDICTION_ALLOW_SET), EO_TYPECHECK(Eina_Bool, prediction)

/**
 * @def elm_obj_entry_prediction_allow_get
 * @since 1.8
 *
 * Get whether the entry should allow to use the text prediction.
 *
 * @param[out] ret
 *
 * @see elm_entry_prediction_allow_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_prediction_allow_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_PREDICTION_ALLOW_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_imf_context_reset
 * @since 1.8
 *
 * Reset the input method context of the entry if needed.
 *
 *
 * @see elm_entry_imf_context_reset
 *
 * @ingroup Entry
 */
#define elm_obj_entry_imf_context_reset() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_IMF_CONTEXT_RESET)

/**
 * @def elm_obj_entry_input_panel_enabled_set
 * @since 1.8
 *
 * Sets the attribute to show the input panel automatically.
 *
 * @param[in] enabled
 *
 * @see elm_entry_input_panel_enabled_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_enabled_set(enabled) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_entry_input_panel_enabled_get
 * @since 1.8
 *
 * Retrieve the attribute to show the input panel automatically.
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_enabled_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_enabled_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_input_panel_show
 * @since 1.8
 *
 * Show the input panel (virtual keyboard) based on the input panel property of entry such as layout, autocapital types, and so on.
 *
 *
 * @see elm_entry_input_panel_show
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_show() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW)

/**
 * @def elm_obj_entry_input_panel_hide
 * @since 1.8
 *
 * Hide the input panel (virtual keyboard).
 *
 *
 * @see elm_entry_input_panel_hide
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_hide() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_HIDE)

/**
 * @def elm_obj_entry_input_panel_language_set
 * @since 1.8
 *
 * Set the language mode of the input panel.
 *
 * @param[in] lang
 *
 * @see elm_entry_input_panel_language_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_language_set(lang) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LANGUAGE_SET), EO_TYPECHECK(Elm_Input_Panel_Lang, lang)

/**
 * @def elm_obj_entry_input_panel_language_get
 * @since 1.8
 *
 * Get the language mode of the input panel.
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_language_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_language_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_LANGUAGE_GET), EO_TYPECHECK(Elm_Input_Panel_Lang *, ret)

/**
 * @def elm_obj_entry_input_panel_imdata_set
 * @since 1.8
 *
 * Set the input panel-specific data to deliver to the input panel.
 *
 * @param[in] data
 * @param[in] len
 *
 * @see elm_entry_input_panel_imdata_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_imdata_set(data, len) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_IMDATA_SET), EO_TYPECHECK(const void *, data), EO_TYPECHECK(int, len)

/**
 * @def elm_obj_entry_input_panel_imdata_get
 * @since 1.8
 *
 * Get the specific data of the current input panel.
 *
 * @param[out] data
 * @param[out] len
 *
 * @see elm_entry_input_panel_imdata_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_imdata_get(data, len) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_IMDATA_GET), EO_TYPECHECK(void *, data), EO_TYPECHECK(int *, len)

/**
 * @def elm_obj_entry_input_panel_return_key_type_set
 * @since 1.8
 *
 * Set the "return" key type. This type is used to set string or icon on the "return" key of the input panel.
 *
 * @param[in] return_key_type
 *
 * @see elm_entry_input_panel_return_key_type_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_return_key_type_set(return_key_type) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_TYPE_SET), EO_TYPECHECK(Elm_Input_Panel_Return_Key_Type, return_key_type)

/**
 * @def elm_obj_entry_input_panel_return_key_type_get
 * @since 1.8
 *
 * Get the "return" key type.
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_return_key_type_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_return_key_type_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_TYPE_GET), EO_TYPECHECK(Elm_Input_Panel_Return_Key_Type *, ret)

/**
 * @def elm_obj_entry_input_panel_return_key_disabled_set
 * @since 1.8
 *
 * Set the return key on the input panel to be disabled.
 *
 * @param[in] disabled
 *
 * @see elm_entry_input_panel_return_key_disabled_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_return_key_disabled_set(disabled) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_DISABLED_SET), EO_TYPECHECK(Eina_Bool, disabled)

/**
 * @def elm_obj_entry_input_panel_return_key_disabled_get
 * @since 1.8
 *
 * Get whether the return key on the input panel should be disabled or not.
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_return_key_disabled_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_return_key_disabled_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_DISABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_input_panel_return_key_autoenabled_set
 * @since 1.8
 *
 * Set whether the return key on the input panel is disabled automatically when entry has no text.
 *
 * @param[in] enabled
 *
 * @see elm_entry_input_panel_return_key_autoenabled_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_return_key_autoenabled_set(enabled) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_RETURN_KEY_AUTOENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_entry_input_panel_show_on_demand_set
 * @since 1.9
 *
 * Set the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param[in] ondemand
 *
 * @see elm_entry_input_panel_show_on_demand_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_show_on_demand_set(ondemand) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW_ON_DEMAND_SET), EO_TYPECHECK(Eina_Bool, ondemand)

/**
 * @def elm_obj_entry_input_panel_show_on_demand_get
 * @since 1.9
 *
 * Get the attribute to show the input panel in case of only an user's explicit Mouse Up event.
 *
 * @param[out] ret
 *
 * @see elm_entry_input_panel_show_on_demand_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_input_panel_show_on_demand_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_INPUT_PANEL_SHOW_ON_DEMAND_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_entry_imf_context_get
 * @since 1.8
 *
 * Returns the input method context of the entry.
 *
 * @param[out] ret
 *
 * @see elm_entry_imf_context_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_imf_context_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_IMF_CONTEXT_GET), EO_TYPECHECK(void **, ret)

/**
 * @def elm_obj_entry_anchor_hover_parent_set
 * @since 1.8
 *
 * Set the parent of the hover popup
 *
 * @param[in] parent
 *
 * @see elm_entry_anchor_hover_parent_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_anchor_hover_parent_set(parent) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_entry_anchor_hover_parent_get
 * @since 1.8
 *
 * Get the parent of the hover popup
 *
 * @param[out] ret
 *
 * @see elm_entry_anchor_hover_parent_get
 *
 * @ingroup Entry
 */
#define elm_obj_entry_anchor_hover_parent_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_entry_anchor_hover_style_set
 * @since 1.8
 *
 * Set the style that the hover should use
 *
 * @param[in] style
 *
 * @see elm_entry_anchor_hover_style_set
 *
 * @ingroup Entry
 */
#define elm_obj_entry_anchor_hover_style_set(style) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_STYLE_SET), EO_TYPECHECK(const char *, style)

/**
 * @def elm_obj_entry_anchor_hover_style_get
 * @since 1.8
 *
 * Get the style that the hover should use
 *
 * @param[out] ret
 *
 * @see elm_entry_anchor_hover_style_get
 */
#define elm_obj_entry_anchor_hover_style_get(ret) ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_STYLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_entry_anchor_hover_end
 * @since 1.8
 *
 * Ends the hover popup in the entry
 *
 *
 * @see elm_entry_anchor_hover_end
 *
 * @ingroup Entry
 */
#define elm_obj_entry_anchor_hover_end() ELM_OBJ_ENTRY_ID(ELM_OBJ_ENTRY_SUB_ID_ANCHOR_HOVER_END)
