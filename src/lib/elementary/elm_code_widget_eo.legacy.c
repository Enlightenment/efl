
EAPI void
elm_code_widget_code_set(Elm_Code_Widget *obj, Elm_Code *code)
{
   efl_ui_code_widget_code_set(obj, code);
}

EAPI Elm_Code *
elm_code_widget_code_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_code_get(obj);
}

EAPI void
elm_code_widget_font_set(Elm_Code_Widget *obj, const char *name, int size)
{
   efl_ui_code_widget_font_set(obj, name, size);
}

EAPI void
elm_code_widget_font_get(const Elm_Code_Widget *obj, const char **name, int *size)
{
   efl_ui_code_widget_font_get(obj, name, size);
}

EAPI unsigned int
elm_code_widget_columns_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_columns_get(obj);
}

EAPI void
elm_code_widget_gravity_set(Elm_Code_Widget *obj, double x, double y)
{
   efl_ui_code_widget_gravity_set(obj, x, y);
}

EAPI void
elm_code_widget_gravity_get(const Elm_Code_Widget *obj, double *x, double *y)
{
   efl_ui_code_widget_gravity_get(obj, x, y);
}

EAPI void
elm_code_widget_policy_set(Elm_Code_Widget *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   efl_ui_code_widget_policy_set(obj, (Elm_Code_Widget_Scroller_Policy)policy_h, (Elm_Code_Widget_Scroller_Policy)policy_v);
}

EAPI void
elm_code_widget_policy_get(const Elm_Code_Widget *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   efl_ui_code_widget_policy_get(obj, (Elm_Code_Widget_Scroller_Policy*)policy_h, (Elm_Code_Widget_Scroller_Policy*)policy_v);
}

EAPI void
elm_code_widget_tabstop_set(Elm_Code_Widget *obj, unsigned int tabstop)
{
   efl_ui_code_widget_tabstop_set(obj, tabstop);
}

EAPI unsigned int
elm_code_widget_tabstop_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_tabstop_get(obj);
}

EAPI void
elm_code_widget_editable_set(Elm_Code_Widget *obj, Eina_Bool editable)
{
   efl_ui_code_widget_editable_set(obj, editable);
}

EAPI Eina_Bool
elm_code_widget_editable_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_editable_get(obj);
}

EAPI void
elm_code_widget_line_numbers_set(Elm_Code_Widget *obj, Eina_Bool line_numbers)
{
   efl_ui_code_widget_line_numbers_set(obj, line_numbers);
}

EAPI Eina_Bool
elm_code_widget_line_numbers_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_line_numbers_get(obj);
}

EAPI void
elm_code_widget_line_width_marker_set(Elm_Code_Widget *obj, unsigned int line_width_marker)
{
   efl_ui_code_widget_line_width_marker_set(obj, line_width_marker);
}

EAPI unsigned int
elm_code_widget_line_width_marker_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_line_width_marker_get(obj);
}

EAPI void
elm_code_widget_show_whitespace_set(Elm_Code_Widget *obj, Eina_Bool show_whitespace)
{
   efl_ui_code_widget_show_whitespace_set(obj, show_whitespace);
}

EAPI Eina_Bool
elm_code_widget_show_whitespace_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_show_whitespace_get(obj);
}

EAPI void
elm_code_widget_alpha_set(Elm_Code_Widget *obj, int alpha)
{
   efl_ui_code_widget_alpha_set(obj, alpha);
}

EAPI int
elm_code_widget_alpha_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_alpha_get(obj);
}

EAPI void
elm_code_widget_syntax_enabled_set(Elm_Code_Widget *obj, Eina_Bool syntax_enabled)
{
   efl_ui_code_widget_syntax_enabled_set(obj, syntax_enabled);
}

EAPI Eina_Bool
elm_code_widget_syntax_enabled_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_syntax_enabled_get(obj);
}

EAPI void
elm_code_widget_tab_inserts_spaces_set(Elm_Code_Widget *obj, Eina_Bool tab_inserts_spaces)
{
   efl_ui_code_widget_tab_inserts_spaces_set(obj, tab_inserts_spaces);
}

EAPI Eina_Bool
elm_code_widget_tab_inserts_spaces_get(const Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_tab_inserts_spaces_get(obj);
}

EAPI void
elm_code_widget_cursor_position_set(Elm_Code_Widget *obj, unsigned int row, unsigned int col)
{
   efl_ui_code_widget_cursor_position_set(obj, row, col);
}

EAPI void
elm_code_widget_cursor_position_get(const Elm_Code_Widget *obj, unsigned int *row, unsigned int *col)
{
   efl_ui_code_widget_cursor_position_get(obj, row, col);
}

EAPI void
elm_code_widget_theme_refresh(Elm_Code_Widget *obj)
{
   efl_ui_code_widget_theme_refresh(obj);
}

EAPI void
elm_code_widget_line_refresh(Elm_Code_Widget *obj, Elm_Code_Line *line)
{
   efl_ui_code_widget_line_refresh(obj, line);
}

EAPI Eina_Bool
elm_code_widget_line_visible_get(Elm_Code_Widget *obj, Elm_Code_Line *line)
{
   return efl_ui_code_widget_line_visible_get(obj, line);
}

EAPI unsigned int
elm_code_widget_lines_visible_get(Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_lines_visible_get(obj);
}

EAPI Eina_Bool
elm_code_widget_position_at_coordinates_get(Elm_Code_Widget *obj, int x, int y, unsigned int *row, int *col)
{
   return efl_ui_code_widget_position_at_coordinates_get(obj, x, y, row, col);
}

EAPI Eina_Bool
elm_code_widget_geometry_for_position_get(Elm_Code_Widget *obj, unsigned int row, int col, int *x, int *y, int *w, int *h)
{
   return efl_ui_code_widget_geometry_for_position_get(obj, row, col, x, y, w, h);
}

EAPI int
elm_code_widget_text_left_gutter_width_get(Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_text_left_gutter_width_get(obj);
}

EAPI char *
elm_code_widget_text_between_positions_get(Elm_Code_Widget *obj, unsigned int start_line, unsigned int start_col, unsigned int end_line, unsigned int end_col)
{
   return efl_ui_code_widget_text_between_positions_get(obj, start_line, start_col, end_line, end_col);
}

EAPI void
elm_code_widget_text_at_cursor_insert(Elm_Code_Widget *obj, const char *text)
{
   efl_ui_code_widget_text_at_cursor_insert(obj, text);
}

EAPI unsigned int
elm_code_widget_line_text_column_width_to_position(Elm_Code_Widget *obj, Elm_Code_Line *line, unsigned int position)
{
   return efl_ui_code_widget_line_text_column_width_to_position(obj, line, position);
}

EAPI unsigned int
elm_code_widget_line_text_column_width_get(Elm_Code_Widget *obj, Elm_Code_Line *line)
{
   return efl_ui_code_widget_line_text_column_width_get(obj, line);
}

EAPI unsigned int
elm_code_widget_line_text_position_for_column_get(Elm_Code_Widget *obj, Elm_Code_Line *line, unsigned int column)
{
   return efl_ui_code_widget_line_text_position_for_column_get(obj, line, column);
}

EAPI unsigned int
elm_code_widget_text_tabwidth_at_column_get(Elm_Code_Widget *obj, unsigned int column)
{
   return efl_ui_code_widget_text_tabwidth_at_column_get(obj, column);
}

EAPI void
elm_code_widget_line_status_toggle(Elm_Code_Widget *obj, Elm_Code_Line *line)
{
   efl_ui_code_widget_line_status_toggle(obj, line);
}

EAPI void
elm_code_widget_undo(Elm_Code_Widget *obj)
{
   efl_ui_code_widget_undo(obj);
}

EAPI Eina_Bool
elm_code_widget_can_undo_get(Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_can_undo_get(obj);
}

EAPI void
elm_code_widget_redo(Elm_Code_Widget *obj)
{
   efl_ui_code_widget_redo(obj);
}

EAPI Eina_Bool
elm_code_widget_can_redo_get(Elm_Code_Widget *obj)
{
   return efl_ui_code_widget_can_redo_get(obj);
}
