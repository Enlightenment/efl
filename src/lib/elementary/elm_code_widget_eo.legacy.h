#ifndef _ELM_CODE_WIDGET_EO_LEGACY_H_
#define _ELM_CODE_WIDGET_EO_LEGACY_H_

#ifndef _ELM_CODE_WIDGET_EO_CLASS_TYPE
#define _ELM_CODE_WIDGET_EO_CLASS_TYPE

typedef Eo Elm_Code_Widget;

#endif

#ifndef _ELM_CODE_WIDGET_EO_TYPES
#define _ELM_CODE_WIDGET_EO_TYPES


#endif

/**
 * @brief Set the underlying code object that this widget renders. This can
 * only be set during construction, once the widget is created the backing code
 * object cannot be changed.
 *
 * @param[in] obj The object.
 * @param[in] code Our underlying Elm.Code object
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_code_set(Elm_Code_Widget *obj, Elm_Code *code);

/**
 * @brief Get the underlying code object we are rendering
 *
 * @param[in] obj The object.
 *
 * @return Our underlying Elm.Code object
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Elm_Code *elm_code_widget_code_get(const Elm_Code_Widget *obj);

/**
 * @brief Set the font that this widget uses, the font should be a monospaced
 * scalable font. Passing NULL will load the default system monospaced font.
 *
 * @param[in] obj The object.
 * @param[in] name The name of the font to load
 * @param[in] size The font size for the widget
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_font_set(Elm_Code_Widget *obj, const char *name, int size);

/**
 * @brief Get the font currently in use. The font name is a copy ad should be
 * freed once it is no longer needed
 *
 * @param[in] obj The object.
 * @param[out] name The name of the font to load
 * @param[out] size The font size for the widget
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_font_get(const Elm_Code_Widget *obj, const char **name, int *size);

/**
 * @brief Get the number of columns in the widget currently. This will be the
 * max of the number of columns to represent the longest line and the minimum
 * required to fill the visible widget width.
 *
 * @param[in] obj The object.
 *
 * @return The number of columns required to render the widget
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_columns_get(const Elm_Code_Widget *obj);

/**
 * @brief Set how this widget's scroller should respond to new lines being
 * added.
 *
 * An x value of 0.0 will maintain the distance from the left edge, 1.0 will
 * ensure the rightmost edge (of the longest line) is respected With 0.0 for y
 * the view will keep it's position relative to the top whereas 1.0 will scroll
 * downward as lines are added.
 *
 * @param[in] obj The object.
 * @param[in] x The horizontal value of the scroller gravity - valid values are
 * 0.0 and 1.0
 * @param[in] y The vertical gravity of the widget's scroller - valid values
 * are 0.0 and 1.0
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_gravity_set(Elm_Code_Widget *obj, double x, double y);

/**
 * @brief Get the current x and y gravity of the widget's scroller
 *
 * @param[in] obj The object.
 * @param[out] x The horizontal value of the scroller gravity - valid values
 * are 0.0 and 1.0
 * @param[out] y The vertical gravity of the widget's scroller - valid values
 * are 0.0 and 1.0
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_gravity_get(const Elm_Code_Widget *obj, double *x, double *y);

/**
 * @brief Set the policy for scrollbar visibility.
 *
 * @param[in] obj The object.
 * @param[in] policy_h The horizontal scrollbar visibility policy
 * @param[in] policy_v The vertical scrollbar visibility policy
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_policy_set(Elm_Code_Widget *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * @brief Get the widget's policy for scrollbar visibility.
 *
 * @param[in] obj The object.
 * @param[out] policy_h The horizontal scrollbar visibility policy
 * @param[out] policy_v The vertical scrollbar visibility policy
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_policy_get(const Elm_Code_Widget *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * @brief Set the width of a tab stop, used purely for visual layout of tab
 * characters.
 *
 * Recommended value is between 2 and 8.
 *
 * @param[in] obj The object.
 * @param[in] tabstop Maximum width of a tab character
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_tabstop_set(Elm_Code_Widget *obj, unsigned int tabstop);

/**
 * @brief Get the current width of a tab stop. This is used to determine where
 * characters after a tab should appear in the line.
 *
 * @param[in] obj The object.
 *
 * @return Maximum width of a tab character
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_tabstop_get(const Elm_Code_Widget *obj);

/**
 * @brief Set whether this widget allows editing
 *
 * If editable then the widget will allow user input to manipulate the
 * underlying Elm.Code_File of this Elm.Code instance. Any other
 * Elm.Code_Widget's connected to this Elm.Code will update to reflect the
 * changes.
 *
 * @param[in] obj The object.
 * @param[in] editable The editable state of the widget
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_editable_set(Elm_Code_Widget *obj, Eina_Bool editable);

/**
 * @brief Get the current editable state of this widget
 *
 * returns EINA_TRUE if the widget is editable, EINA_FALSE otherwise. If this
 * widget is not editable the underlying Elm.Code_File could still be
 * manipulated by a different widget or the filesystem.
 *
 * @param[in] obj The object.
 *
 * @return The editable state of the widget
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_editable_get(const Elm_Code_Widget *obj);

/**
 * @brief Set whether line numbers should be displayed in the left gutter.
 *
 * Passing EINA_TRUE will reserve a space for showing line numbers, EINA_FALSE
 * will turn this off.
 *
 * @param[in] obj The object.
 * @param[in] line_numbers Whether or not line numbers (or their placeholder)
 * should be shown
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_line_numbers_set(Elm_Code_Widget *obj, Eina_Bool line_numbers);

/**
 * @brief Get the status of line number display for this widget.
 *
 * @param[in] obj The object.
 *
 * @return Whether or not line numbers (or their placeholder) should be shown
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_line_numbers_get(const Elm_Code_Widget *obj);

/**
 * @brief Set where the line width market should be shown.
 *
 * Passing a non-zero value will set which line width to mark with a vertical
 * line. Passing 0 will hide this marker.
 *
 * @param[in] obj The object.
 * @param[in] line_width_marker Where to display a line width marker, if at all
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_line_width_marker_set(Elm_Code_Widget *obj, unsigned int line_width_marker);

/**
 * @brief Get the position of the line width marker, any positive return
 * indicates where the marker appears.
 *
 * @param[in] obj The object.
 *
 * @return Where to display a line width marker, if at all
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_line_width_marker_get(const Elm_Code_Widget *obj);

/**
 * @brief Set whether white space should be shown.
 *
 * @param[in] obj The object.
 * @param[in] show_whitespace Whether or not we show whitespace characters
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_show_whitespace_set(Elm_Code_Widget *obj, Eina_Bool show_whitespace);

/**
 * @brief Get whether or not white space will be visible.
 *
 * @param[in] obj The object.
 *
 * @return Whether or not we show whitespace characters
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_show_whitespace_get(const Elm_Code_Widget *obj);

/**
 * @brief Set an alpha color value for this widget (0 - 255) default is fully
 * opaque (255).
 *
 * @param[in] obj The object.
 * @param[in] alpha Alpha value
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_alpha_set(Elm_Code_Widget *obj, int alpha);

/**
 * @brief Set an alpha color value for this widget (0 - 255) default is fully
 * opaque (255).
 *
 * @param[in] obj The object.
 *
 * @return Alpha value
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI int elm_code_widget_alpha_get(const Elm_Code_Widget *obj);

/**
 * @brief Set whether syntax highlighting should be use for this widget.
 *
 * @param[in] obj The object.
 * @param[in] syntax_enabled Whether or not to enable syntax highlighting
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_syntax_enabled_set(Elm_Code_Widget *obj, Eina_Bool syntax_enabled);

/**
 * @brief Get this widget's enabled state for syntax highlighting.
 *
 * @param[in] obj The object.
 *
 * @return Whether or not to enable syntax highlighting
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_syntax_enabled_get(const Elm_Code_Widget *obj);

/**
 * @brief Set whether space characters should be inserted instead of tabs.
 *
 * @param[in] obj The object.
 * @param[in] tab_inserts_spaces @c true if we should insert space characters
 * instead of a tab when the Tab key is pressed
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_tab_inserts_spaces_set(Elm_Code_Widget *obj, Eina_Bool tab_inserts_spaces);

/**
 * @brief Get whether or not space characters will be inserted instead of tabs.
 *
 * @param[in] obj The object.
 *
 * @return @c true if we should insert space characters instead of a tab when
 * the Tab key is pressed
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_tab_inserts_spaces_get(const Elm_Code_Widget *obj);

/**
 * @brief Set the current location of the text cursor.
 *
 * @param[in] obj The object.
 * @param[in] row The vertical position ov the cursur - the fist row is 1
 * @param[in] col The horizontal position of the cursor, starting from column 1
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_cursor_position_set(Elm_Code_Widget *obj, unsigned int row, unsigned int col);

/**
 * @brief Get the current x and y position of the widget's cursor.
 *
 * @param[in] obj The object.
 * @param[out] row The vertical position ov the cursur - the fist row is 1
 * @param[out] col The horizontal position of the cursor, starting from column
 * 1
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_cursor_position_get(const Elm_Code_Widget *obj, unsigned int *row, unsigned int *col);

/** Update and refresh theme for widget.
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_theme_refresh(Elm_Code_Widget *obj);

/**
 * @brief Refresh code line in widget
 *
 * @param[in] obj The object.
 * @param[in] line The line to refresh.
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_line_refresh(Elm_Code_Widget *obj, Elm_Code_Line *line);

/**
 * @brief Check if the code line is currently visible
 *
 * @param[in] obj The object.
 * @param[in] line The line to test for visibility.
 *
 * @return @c true if the line specified is currently visible within the scroll
 * region.
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_line_visible_get(Elm_Code_Widget *obj, Elm_Code_Line *line);

/**
 * @brief Get the number of code lines currently visible in the widget
 *
 * @param[in] obj The object.
 *
 * @return the number of lines currently visible in the widget.
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_lines_visible_get(Elm_Code_Widget *obj);

/**
 * @brief Get the row, col position for a given coordinate on the widget.
 *
 * @param[in] obj The object.
 * @param[in] x The x coordinate in the widget
 * @param[in] y The y coordinate in the widget
 * @param[in] row The row for the coordinates
 * @param[in] col The column for the coordinates
 *
 * @return @c true if a line exists at these coordinates
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_position_at_coordinates_get(Elm_Code_Widget *obj, int x, int y, unsigned int *row, int *col);

/**
 * @brief Get the geometry for the cell at the specified position.
 *
 * @param[in] obj The object.
 * @param[in] row The row for the requested position
 * @param[in] col The column for the requested position
 * @param[in] x The x coordinate of the cell at specified position
 * @param[in] y The y coordinate of the cell at specified position
 * @param[in] w The width of the cell at specified position
 * @param[in] h The height of the cell at specified position
 *
 * @return @c true if a cell exists at the specified position
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_geometry_for_position_get(Elm_Code_Widget *obj, unsigned int row, int col, int *x, int *y, int *w, int *h);

/**
 * @brief Get the column width of the gutter
 *
 * @param[in] obj The object.
 *
 * @return The current column width of the gutter for the widget.
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI int elm_code_widget_text_left_gutter_width_get(Elm_Code_Widget *obj);

/**
 * @brief Get text between given positions
 *
 * @param[in] obj The object.
 * @param[in] start_line The line of the first character to get
 * @param[in] start_col The widget column of the first character to get
 * @param[in] end_line The line of the last character to get
 * @param[in] end_col The widget column of the last character to get
 *
 * @return The text content between start and end positions
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI char *elm_code_widget_text_between_positions_get(Elm_Code_Widget *obj, unsigned int start_line, unsigned int start_col, unsigned int end_line, unsigned int end_col);

/**
 * @brief Insert the provided text at the cursor position. This will add the
 * operation to the widget's undo list.
 *
 * @param[in] obj The object.
 * @param[in] text The text to insert
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_text_at_cursor_insert(Elm_Code_Widget *obj, const char *text);

/**
 * @brief Get text column width at given position
 *
 * @param[in] obj The object.
 * @param[in] line Code line
 * @param[in] position Code position
 *
 * @return Text column width
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_line_text_column_width_to_position(Elm_Code_Widget *obj, Elm_Code_Line *line, unsigned int position);

/**
 * @brief Get text column width for code line
 *
 * @param[in] obj The object.
 * @param[in] line Code line
 *
 * @return Text column width
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_line_text_column_width_get(Elm_Code_Widget *obj, Elm_Code_Line *line);

/**
 * @brief Get position from column
 *
 * @param[in] obj The object.
 * @param[in] line Code line
 * @param[in] column Column
 *
 * @return Position
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_line_text_position_for_column_get(Elm_Code_Widget *obj, Elm_Code_Line *line, unsigned int column);

/**
 * @brief Get tabwidth for column
 *
 * @param[in] obj The object.
 * @param[in] column Column
 *
 * @return Tabwidth
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI unsigned int elm_code_widget_text_tabwidth_at_column_get(Elm_Code_Widget *obj, unsigned int column);

/**
 * @brief Toggle the display of the line status widget
 *
 * @param[in] obj The object.
 * @param[in] line Code line
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_line_status_toggle(Elm_Code_Widget *obj, Elm_Code_Line *line);

/** Undo last action
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_undo(Elm_Code_Widget *obj);

/**
 * @brief Determine if there are any available undo operations
 *
 * @param[in] obj The object.
 *
 * @return @c true if there are undo operations
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_can_undo_get(Elm_Code_Widget *obj);

/** Redo last action
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI void elm_code_widget_redo(Elm_Code_Widget *obj);

/**
 * @brief Determine if there are any available redo operations
 *
 * @param[in] obj The object.
 *
 * @return @c true if there are redo operations
 *
 * @ingroup Elm_Code_Widget_Group
 */
EAPI Eina_Bool elm_code_widget_can_redo_get(Elm_Code_Widget *obj);

#endif
