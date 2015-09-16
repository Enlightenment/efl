#ifndef ELM_CODE_WIDGET_PRIVATE_H
# define ELM_CODE_WIDGET_PRIVATE_H

/**
 * Structure holding the info about a selected region.
 */
typedef struct
{
   unsigned int start_line, end_line;
   unsigned int start_col, end_col;
} Elm_Code_Widget_Selection_Data;

typedef struct
{
   Elm_Code *code;
   Evas_Object *grid, *scroller;

   const char *font_name;
   Evas_Font_Size font_size;
   double gravity_x, gravity_y;

   unsigned int cursor_line, cursor_col;
   Eina_Bool editable, focussed;
   Eina_Bool show_line_numbers;
   unsigned int line_width_marker, tabstop;
   Eina_Bool show_whitespace;

   Elm_Code_Widget_Selection_Data *selection;
   Evas_Object *tooltip;
} Elm_Code_Widget_Data;

/* Private widget methods */

void _elm_code_widget_tooltip_text_set(Evas_Object *widget, const char *text);

void _elm_code_widget_tooltip_add(Evas_Object *widget);

EAPI Elm_Code_Widget_Selection_Data *elm_code_widget_selection_normalized_get(Evas_Object *widget);

#endif
