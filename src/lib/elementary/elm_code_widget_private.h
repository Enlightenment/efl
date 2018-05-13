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
   Eina_List *grids;
   unsigned int col_count;
   Evas_Object *scroller, *gridbox, *background;

   const char *font_name;
   Evas_Font_Size font_size;
   double gravity_x, gravity_y;

   unsigned int cursor_line, cursor_col;
   Evas_Object *cursor_rect;

   Eina_Bool visible, editable, focussed;
   Eina_Bool show_line_numbers;
   unsigned int line_width_marker, tabstop;
   Eina_Bool show_whitespace, tab_inserts_spaces;
   int alpha;

   Elm_Code_Widget_Selection_Data *selection;

   /* Undo stack */
   Eina_List *undo_stack;
   Eina_List *undo_stack_ptr;

   Evas_Object *hoversel;
} Elm_Code_Widget_Data;

typedef struct
{
   char *content;
   unsigned int length;
   unsigned int start_line, start_col, end_line, end_col;

   Eina_Bool insert : 1; /**< True if the change is an insertion */
} Elm_Code_Widget_Change_Info;

/* Private widget methods */

void _elm_code_widget_cell_size_get(Elm_Code_Widget *widget, Evas_Coord *width, Evas_Coord *height);

void _elm_code_widget_text_at_cursor_insert_no_undo(Elm_Code_Widget *widget, const char *text, unsigned int length);

void _elm_code_widget_newline(Elm_Code_Widget *widget);

void _elm_code_widget_backspace(Elm_Code_Widget *widget);

void _elm_code_widget_delete(Elm_Code_Widget *widget);

EAPI Elm_Code_Widget_Selection_Data *elm_code_widget_selection_normalized_get(Evas_Object *widget);

void _elm_code_widget_selection_delete_no_undo(Elm_Code_Widget *widget);

void _elm_code_widget_undo_change_add(Evas_Object *widget, Elm_Code_Widget_Change_Info *info);

void _elm_code_widget_change_selection_add(Evas_Object *widget);

#endif
