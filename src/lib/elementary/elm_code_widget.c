#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"

#include "elm_code_private.h"
#include "elm_code_widget_private.h"

/* FIXME: hoversel is deprecated in EO APIs */
#include "elm_hoversel.eo.h"

#define MY_CLASS ELM_CODE_WIDGET_CLASS

typedef enum {
   ELM_CODE_WIDGET_COLOR_GUTTER_BG = ELM_CODE_TOKEN_TYPE_COUNT,
   ELM_CODE_WIDGET_COLOR_GUTTER_SCOPE_BG,
   ELM_CODE_WIDGET_COLOR_GUTTER_FG,
   ELM_CODE_WIDGET_COLOR_WHITESPACE,
   ELM_CODE_WIDGET_COLOR_SELECTION,

   ELM_CODE_WIDGET_COLOR_COUNT
} Elm_Code_Widget_Colors;

static Eina_Unicode status_icons[] = {
 ' ',
 ' ',
 ' ',
 ' ',
 '!',
 '!',
 '!',

 '+',
 '-',
 ' ',

 0x2713,
 0x2717,

 0x2691,

 0
};

#define EO_CONSTRUCTOR_CHECK_RETURN(obj) do { \
   Eina_Bool finalized; \
   \
   finalized = efl_finalized_get(obj); \
   if (finalized) \
     { \
        ERR("This function is only allowed during construction."); \
        return; \
     } \
} while (0)

static void _elm_code_widget_resize(Elm_Code_Widget *widget, Elm_Code_Line *newline);

#include "elm_code_widget_legacy.eo.h"

EAPI Evas_Object *
elm_code_widget_add(Evas_Object *parent, Elm_Code *code)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(ELM_CODE_WIDGET_LEGACY_CLASS, parent,
                         elm_obj_code_widget_code_set(efl_added, code));
}

#include "elm_code_widget_legacy.eo.c"

EOLIAN static Eo *
_elm_code_widget_efl_object_constructor(Eo *obj, Elm_Code_Widget_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ELM_CODE_WIDGET_CLASS));

   pd->cursor_line = 1;
   pd->cursor_col = 1;

   pd->tabstop = 8;

   return obj;
}

EOLIAN static Eo *
_elm_code_widget_efl_object_finalize(Eo *obj, Elm_Code_Widget_Data *pd)
{
   obj = efl_finalize(efl_super(obj, ELM_CODE_WIDGET_CLASS));

   if (pd->code)
     return obj;

   ERR("Elm_Code_Widget cannot finalize without calling elm_code_widget_code_set.");
   return NULL;
}

EOLIAN static void
_elm_code_widget_class_constructor(Efl_Class *klass EINA_UNUSED)
{

}

void
_elm_code_widget_cell_size_get(Elm_Code_Widget *widget, Evas_Coord *width, Evas_Coord *height)
{
   Elm_Code_Widget_Data *pd;
   Evas_Object *grid;
   Evas_Coord w = 0, h = 0;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   grid = eina_list_nth(pd->grids, 0);
   if (!grid)
     return;

   evas_object_textgrid_cell_size_get(grid, &w, &h);
   if (w == 0) w = 5;
   if (h == 0) h = 10;

   if (width) *width = w;
   if (height) *height = h;
}

static void
_elm_code_widget_scroll_by(Elm_Code_Widget *widget, int by_x, int by_y)
{
   Elm_Code_Widget_Data *pd;
   Evas_Coord x, y, w, h;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_scroller_region_get(pd->scroller, &x, &y, &w, &h);
   x += by_x;
   y += by_y;
   elm_scroller_region_show(pd->scroller, x, y, w, h);
}

static void
_elm_code_widget_fill_line_token(Evas_Textgrid_Cell *cells, int count, int start, int end, Elm_Code_Token_Type type)
{
   int x;

   for (x = start; x <= end && x < count; x++)
     {
        // TODO find a way to mark if a token is themes fg or bg
        if (type == ELM_CODE_TOKEN_TYPE_MATCH)
          cells[x - 1].bg = type;
        else
          cells[x - 1].fg = type;
     }
}

static unsigned int
_elm_code_widget_status_type_get(Elm_Code_Widget *widget, Elm_Code_Line *line, unsigned int col)
{
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (line->status != ELM_CODE_STATUS_TYPE_DEFAULT)
     return line->status;

   if (pd->editable && pd->focussed && pd->cursor_line == line->number)
     return ELM_CODE_STATUS_TYPE_CURRENT;

   if (pd->line_width_marker > 0 && pd->line_width_marker == col-1)
     return ELM_CODE_WIDGET_COLOR_GUTTER_BG;

   return ELM_CODE_STATUS_TYPE_DEFAULT;
}

static void
_elm_code_widget_fill_line_tokens(Elm_Code_Widget *widget, Evas_Textgrid_Cell *cells,
                                  unsigned int count, Elm_Code_Line *line)
{
   Eina_List *item;
   Elm_Code_Token *token;
   unsigned int end, length, offset;
   unsigned int token_start_col, token_end_col;

   offset = elm_obj_code_widget_text_left_gutter_width_get(widget);
   length = elm_code_widget_line_text_column_width_get(widget, line) + offset;

   EINA_LIST_FOREACH(line->tokens, item, token)
     {
        token_start_col = elm_code_widget_line_text_column_width_to_position(widget, line, token->start) + offset;
        token_end_col = elm_code_widget_line_text_column_width_to_position(widget, line, token->end) + offset;

        // TODO handle a token starting before the previous finishes
        end = token_end_col;
        if (token->continues)
          end = length + offset;
        _elm_code_widget_fill_line_token(cells, count, token_start_col, end, token->type);
     }
}

static Eina_Bool
_elm_code_widget_line_in_scope(Elm_Code_Line *line, Elm_Code_Line *fromline)
{
   Elm_Code_Line *midline;
   unsigned int number;

   if (!line || !fromline || line->scope == 0 || fromline->scope == 0)
     return EINA_FALSE;

   if (line->number == fromline->number)
     return EINA_TRUE;

   if (line->scope < fromline->scope)
     return EINA_FALSE;

   number = fromline->number;
   while (number != line->number)
   {
      midline = elm_code_file_line_get(line->file, number);

      if (midline->scope < fromline->scope)
        return EINA_FALSE;

      if (line->number < fromline->number)
        number--;
      else
        number++;
   }
   return EINA_TRUE;
}

static void
_elm_code_widget_fill_line_gutter(Elm_Code_Widget *widget, Evas_Textgrid_Cell *cells,
                                  int width, Elm_Code_Line *line)
{
   char *number = NULL;
   int gutter, g;
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *cursor_line;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_code_widget_text_left_gutter_width_get(widget);

   if (width < gutter)
     return;

   cells[gutter-1].codepoint = status_icons[line->status];
   cells[gutter-1].bold = 1;
   cells[gutter-1].fg = ELM_CODE_WIDGET_COLOR_GUTTER_FG;
   if (line->status == ELM_CODE_STATUS_TYPE_DEFAULT)
     {
        cursor_line = elm_code_file_line_get(line->file, pd->cursor_line);
        if (_elm_code_widget_line_in_scope(line, cursor_line))
          cells[gutter-1].bg = ELM_CODE_WIDGET_COLOR_GUTTER_SCOPE_BG;
        else
          cells[gutter-1].bg = ELM_CODE_WIDGET_COLOR_GUTTER_BG;
     }
   else
     {
        cells[gutter-1].bg = line->status;
     }

   if (pd->show_line_numbers)
     {
        if (line->number > 0)
          {
             number = malloc(sizeof(char) * gutter);
             snprintf(number, gutter, "%*d", gutter - 1, line->number);
          }
        for (g = 0; g < gutter - 1; g++)
          {
             if (number)
               cells[g].codepoint = *(number + g);
             else
               cells[g].codepoint = 0;

             cells[g].fg = ELM_CODE_WIDGET_COLOR_GUTTER_FG;
             cells[g].bg = ELM_CODE_WIDGET_COLOR_GUTTER_BG;
          }

        if (number)
          free(number);
     }
}

static void
_elm_code_widget_fill_whitespace(Elm_Code_Widget *widget, Eina_Unicode character, Evas_Textgrid_Cell *cell)
{
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!pd->show_whitespace)
     {
        if (character== '\t')
          cell->codepoint = 0;
        return;
     }

   switch (character)
     {
        case ' ':
          cell->codepoint = 0x00b7;
          break;
        case '\t':
          cell->codepoint = 0x2192;
          break;
        case '\n':
          cell->codepoint = 0x23ce;
          break;
        default:
          return;
     }

   cell->fg = ELM_CODE_WIDGET_COLOR_WHITESPACE;
}

static void
_elm_code_widget_cursor_update(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd)
{
   Evas_Coord cx, cy, cw, ch;

   elm_code_widget_geometry_for_position_get(widget, pd->cursor_line, pd->cursor_col, &cx, &cy, &cw, &ch);

   if (!pd->cursor_rect)
     {
        pd->cursor_rect = elm_layout_add(widget);

        if (!elm_layout_theme_set(pd->cursor_rect, "entry", "cursor", elm_widget_style_get(widget)))
          CRI("Failed to set layout!");
        elm_layout_signal_emit(pd->cursor_rect, "elm,action,focus", "elm");
     }

   evas_object_resize(pd->cursor_rect, cw/8, ch);
   evas_object_move(pd->cursor_rect, cx, cy);
   evas_object_show(pd->cursor_rect);
}

static void
_elm_code_widget_fill_cursor(Elm_Code_Widget *widget, unsigned int number, int gutter, int w)
{
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->visible && pd->editable && pd->focussed && pd->cursor_line == number)
     {
        if (pd->cursor_col + gutter - 1 >= (unsigned int) w)
          return;

        if (pd->selection && !pd->selection->in_progress)
          {
             Elm_Code_Line *line = elm_code_file_line_get(pd->code->file, number);
             if (!line)
               return;

             if (!elm_code_widget_line_visible_get(widget, line))
               return;
          }

        _elm_code_widget_cursor_update(widget, pd);
     }
}

static void
_elm_code_widget_fill_selection(Elm_Code_Widget *widget, Elm_Code_Line *line, Evas_Textgrid_Cell *cells,
                                int gutter, int w)
{
   Elm_Code_Widget_Data *pd;
   unsigned int x, start, end;
   Elm_Code_Widget_Selection_Data *selection;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!pd->selection)
     return;

   selection = elm_code_widget_selection_normalized_get(widget);
   if (selection->start_line > line->number || selection->end_line < line->number)
     {
        free(selection);
        return;
     }

   start = selection->start_col;
   if (selection->start_line < line->number)
     start = 1;
   end = selection->end_col;
   if (selection->end_line > line->number)
     end = w;
   free(selection);

   for (x = gutter + start - 1; x < gutter + end && x < (unsigned int) w; x++)
     cells[x].bg = ELM_CODE_WIDGET_COLOR_SELECTION;
}

static void
_elm_code_widget_fill_line(Elm_Code_Widget *widget, Elm_Code_Line *line)
{
   char *chr;
   Eina_Unicode unichr;
   unsigned int length, x, charwidth, i, w;
   int chrpos, gutter;
   Evas_Object *grid;
   Evas_Textgrid_Cell *cells;
   Elm_Code_Widget_Data *pd;

   EINA_SAFETY_ON_NULL_RETURN(line);

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);
   if (eina_list_count(pd->grids) < line->number)
     return;

   w = elm_code_widget_columns_get(widget);
   grid = eina_list_nth(pd->grids, line->number - 1);
   cells = evas_object_textgrid_cellrow_get(grid, 0);

   length = elm_code_widget_line_text_column_width_get(widget, line);
   chrpos = 0;
   chr = (char *)elm_code_line_text_get(line, NULL);

   for (x = gutter; x < (unsigned int) w && x < length + gutter; x+=charwidth)
     {
        unichr = eina_unicode_utf8_next_get(chr, &chrpos);

        cells[x].codepoint = unichr;
        cells[x].bold = 0;
        cells[x].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;
        cells[x].bg = _elm_code_widget_status_type_get(widget, line, x - gutter + 1);

        charwidth = 1;
        if (unichr == '\t')
          charwidth = elm_code_widget_text_tabwidth_at_column_get(widget, x - gutter + 1);
        for (i = x + 1; i < x + charwidth && i < (unsigned int) w; i++)
          {
             cells[i].codepoint = 0;
             cells[i].bg = _elm_code_widget_status_type_get(widget, line, i - gutter + 1);
          }

        _elm_code_widget_fill_whitespace(widget, unichr, &cells[x]);
     }
   for (; x < (unsigned int) w; x++)
     {
        cells[x].codepoint = 0;
        cells[x].bold = 0;
        cells[x].bg = _elm_code_widget_status_type_get(widget, line, x - gutter + 1);
     }

   _elm_code_widget_fill_line_gutter(widget, cells, w, line);
   _elm_code_widget_fill_line_tokens(widget, cells, w, line);

   _elm_code_widget_fill_selection(widget, line, cells, gutter, w);
   _elm_code_widget_fill_cursor(widget, line->number, gutter, w);
   if (line->number < elm_code_file_lines_get(line->file))
     _elm_code_widget_fill_whitespace(widget, '\n', &cells[length + gutter]);

   evas_object_textgrid_update_add(grid, 0, 0, w, 1);
}

static void
_elm_code_widget_cursor_selection_set(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd)
{
   unsigned int end_line, end_col;

   end_line = pd->selection->end_line;
   end_col = pd->selection->end_col;

   if (pd->selection->type == ELM_CODE_WIDGET_SELECTION_KEYBOARD)
     return;

   if ((pd->selection->start_line == pd->selection->end_line && pd->selection->end_col > pd->selection->start_col) ||
       (pd->selection->start_line < pd->selection->end_line))
     {
        end_col++;
     }

   elm_code_widget_cursor_position_set(widget, end_line, end_col);
}

static void
_elm_code_widget_fill_range(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd,
                            unsigned int first_row, unsigned int last_row,
                            Elm_Code_Line *newline)
{
   Elm_Code_Line *line;
   unsigned int y;

   if (newline)
     _elm_code_widget_fill_line(widget, newline);

   // if called from new line cb, no need to update whole range unless visible
   if (newline && !elm_obj_code_widget_line_visible_get(widget, newline))
     return;

   // cursor will be shown if it should be visible
   evas_object_hide(pd->cursor_rect);

   for (y = first_row; y <= last_row; y++)
     {
        line = elm_code_file_line_get(pd->code->file, y);

        if (line)
          _elm_code_widget_fill_line(widget, line);
     }

   if (pd->selection)
     _elm_code_widget_cursor_selection_set(widget, pd);
}

static void
_elm_code_widget_fill_update(Elm_Code_Widget *widget, unsigned int first_row, unsigned int last_row,
                             Elm_Code_Line *newline)
{
   Elm_Code_Widget_Data *pd;

   _elm_code_widget_resize(widget, newline);
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   _elm_code_widget_fill_range(widget, pd, first_row, last_row, newline);
}

static Eina_Bool
_elm_code_widget_viewport_get(Elm_Code_Widget *widget,
                              Elm_Code_Widget_Data *pd,
                              unsigned int *first_row,
                              unsigned int *last_row)
{
   Evas_Coord scroll_y, scroll_h, oy;

   evas_object_geometry_get(widget, NULL, &oy, NULL, NULL);
   elm_scroller_region_get(pd->scroller, NULL, &scroll_y, NULL, &scroll_h);
   if (scroll_h == 0)
     return EINA_FALSE;

   elm_code_widget_position_at_coordinates_get(widget, 0, oy, first_row, NULL);
   elm_code_widget_position_at_coordinates_get(widget, 0, oy + scroll_h, last_row, NULL);

   if (last_row && *last_row > elm_code_file_lines_get(pd->code->file))
     *last_row = elm_code_file_lines_get(pd->code->file);

   return EINA_TRUE;
}

static void
_elm_code_widget_refresh(Elm_Code_Widget *widget, Elm_Code_Line *line)
{
   unsigned int first_row, last_row;

   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!_elm_code_widget_viewport_get(widget, pd, &first_row, &last_row))
     return ;

   _elm_code_widget_fill_update(widget, first_row, last_row, line);
}

static void
_elm_code_widget_clear(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Evas_Object *grid;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   EINA_LIST_FREE(pd->grids, grid)
     {
        evas_object_del(grid);
     }
}

static void
_elm_code_widget_fill(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   unsigned int height;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   height = elm_code_widget_lines_visible_get(widget);
   if (height > elm_code_file_lines_get(pd->code->file))
     height = elm_code_file_lines_get(pd->code->file);

   _elm_code_widget_fill_update(widget, 1, height, NULL);
}

static void
_elm_code_widget_line_cb(void *data, const Efl_Event *event)
{
   Elm_Code_Line *line;
   Elm_Code_Widget *widget;

   line = (Elm_Code_Line *)event->info;
   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_refresh(widget, line);
}

static void
_elm_code_widget_file_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (elm_code_file_lines_get(pd->code->file))
     _elm_code_widget_fill(widget);
   else
     _elm_code_widget_clear(widget);
}

static void
_elm_code_widget_selection_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_refresh(widget, NULL);
}

static void
_elm_code_widget_selection_clear_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_fill(widget);
}

static void
_elm_code_widget_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_refresh(widget, NULL);
}

static void
_elm_code_widget_show_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget *widget = (Elm_Code_Widget *) data;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->visible = EINA_TRUE;
   if (pd->cursor_rect)
     evas_object_show(pd->cursor_rect);
}

static void
_elm_code_widget_hidden_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget *widget = (Elm_Code_Widget *) data;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->visible = EINA_FALSE;
   if (pd->cursor_rect)
     evas_object_hide(pd->cursor_rect);
}

static void
_elm_code_widget_cursor_ensure_visible(Elm_Code_Widget *widget)
{
   Evas_Coord viewx, viewy, vieww, viewh, cellw = 0, cellh = 0;
   Evas_Coord curx, cury, oy, rowy;
   Evas_Object *grid;
   Elm_Code_Widget_Data *pd;
   int gutter;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   evas_object_geometry_get(widget, NULL, &oy, NULL, NULL);
   elm_scroller_region_get(pd->scroller, &viewx, &viewy, &vieww, &viewh);
   _elm_code_widget_cell_size_get(widget, &cellw, &cellh);

   grid = eina_list_data_get(eina_list_nth_list(pd->grids, pd->cursor_line - 1));
   evas_object_geometry_get(grid, NULL, &rowy, NULL, NULL);

   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);
   curx = (pd->cursor_col + gutter - 1) * cellw;
   cury = rowy + viewy - oy;

   if (curx >= viewx && cury >= viewy && curx + cellw <= viewx + vieww && cury + cellh <= viewy + viewh)
     return;

   elm_scroller_region_show(pd->scroller, curx, cury, cellw, cellh);
}

static void
_elm_code_widget_cursor_move(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd, unsigned int col, unsigned int line)
{
   Elm_Code *code;
   Elm_Code_Line *line_obj;
   unsigned int oldrow, position, length;
   const char *text;

   oldrow = pd->cursor_line;
   pd->cursor_col = col;
   pd->cursor_line = line;

   code = pd->code;
   line_obj = elm_code_file_line_get(code->file, line);
   position = elm_code_widget_line_text_position_for_column_get(widget, line_obj, col);
   text = elm_code_line_text_get(line_obj, &length);
   if (position < length && text[position] == '\t')
     pd->cursor_col = elm_code_widget_line_text_column_width_to_position(widget, line_obj, position);

   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CURSOR_CHANGED, widget);
   if (!pd->selection || (pd->selection && pd->selection->in_progress))
     _elm_code_widget_cursor_ensure_visible(widget);

   if (oldrow != pd->cursor_line)
     _elm_code_widget_refresh(widget, line_obj);
   else
     _elm_code_widget_fill_line(widget, elm_code_file_line_get(pd->code->file, pd->cursor_line));
   elm_layout_signal_emit(pd->cursor_rect, "elm,action,show,cursor", "elm");
}


EOLIAN static Eina_Bool
_elm_code_widget_position_at_coordinates_get(Eo *obj, Elm_Code_Widget_Data *pd,
                                             Evas_Coord x, Evas_Coord y,
                                             unsigned int *row, int *col)
{
   Elm_Code_Widget *widget;
   Eina_List *item;
   Elm_Code_Line *line;
   Evas_Coord ox = 0, oy = 0, sx = 0, sy = 0, rowy = 0;
   Evas_Object *grid;
   int cw = 0, ch = 0, gutter, retcol;
   unsigned int guess = 1, number = 1;

   widget = (Elm_Code_Widget *)obj;
   evas_object_geometry_get(widget, &ox, &oy, NULL, NULL);
   elm_scroller_region_get(pd->scroller, &sx, &sy, NULL, NULL);
   x = x + sx - ox;
   y = y + sy - oy;

   _elm_code_widget_cell_size_get(widget, &cw, &ch);
   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);

   if (y >= 0 && ch > 0)
     guess = ((double) y / ch) + 1;
   if (guess > 1)
     {
        number = guess;

        // unfortunately EINA_LIST_REVERSE_FOREACH skips to the end of the list...
        for (item = eina_list_nth_list(pd->grids, number - 1), grid = eina_list_data_get(item);
             number > 1 && item;
             item = eina_list_prev(item), grid = eina_list_data_get(item))
          {
             evas_object_geometry_get(grid, NULL, &rowy, NULL, NULL);

             if (rowy + sy - oy - 1 <= y)
               break;

             number--;
          }
     }

   if (col)
     {
        if (cw == 0)
           retcol = 1;
        else
           retcol = ((double) x / cw) - gutter + 1;

        if (retcol <= 0)
           *col = 1;
        else
           *col = retcol;
     }
   if (row)
     *row = number;

   line = elm_code_file_line_get(pd->code->file, number);
   return !!line;
}

EOLIAN static Eina_Bool
_elm_code_widget_geometry_for_position_get(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd, unsigned int row, int col,
                                           Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Elm_Code_Line *line;
   Evas_Object *grid;
   Evas_Coord cellw = 0;
   unsigned int length = 0;
   int gutter;

   line = elm_code_file_line_get(pd->code->file, row);
   if (!line)
     return EINA_FALSE;

   elm_code_line_text_get(line, &length);
   _elm_code_widget_cell_size_get(widget, &cellw, h);
   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);

   grid = eina_list_nth(pd->grids, row - 1);
   evas_object_geometry_get(grid, x, y, NULL, NULL);

   if (x)
     *x += (col - 1 + gutter) * cellw;
   if (w)
     *w = cellw;

   return !!line && col <= (int) length;
}

EOLIAN static void
_elm_code_widget_line_status_toggle(Elm_Code_Widget *widget EINA_UNUSED, Elm_Code_Widget_Data *pd,
                               Elm_Code_Line *line)
{
   Evas_Object *status, *grid;
   const char *template = "<color=#8B8B8B>%s</color>";
   char *text;

   // add a status below the line if needed (and remove those no longer needed)
   grid = eina_list_nth(pd->grids, line->number - 1);
   status = evas_object_data_get(grid, "status");

   if (status)
     {
        elm_box_unpack(pd->gridbox, status);
        evas_object_hide(status);
        evas_object_data_set(grid, "status", NULL);
     }
   else
     {
        status = elm_label_add(pd->gridbox);
        evas_object_size_hint_weight_set(status, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(status, 0.05, EVAS_HINT_FILL);
        evas_object_show(status);

        elm_box_pack_after(pd->gridbox, status, grid);
        evas_object_data_set(grid, "status", status);

        text = malloc((strlen(template) + strlen(line->status_text) + 1) * sizeof(char));
        sprintf(text, template, line->status_text);
        elm_object_text_set(status, text);
        free(text);
     }
}

static void
_popup_menu_dismissed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->hoversel) evas_object_hide(pd->hoversel);
}

static void
_popup_menu_cut_cb(void *data,
        Evas_Object *obj EINA_UNUSED,
        void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_code_widget_selection_cut(widget);
   if (pd->hoversel) evas_object_hide(pd->hoversel);
}

static void
_popup_menu_copy_cb(void *data,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_code_widget_selection_copy(widget);
   if (pd->hoversel) evas_object_hide(pd->hoversel);
}

static void
_popup_menu_paste_cb(void *data,
          Evas_Object *obj EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_code_widget_selection_paste(widget);
   if (pd->hoversel) evas_object_hide(pd->hoversel);
}

static void
_popup_menu_cancel_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_code_widget_selection_clear(widget);
   if (pd->hoversel) evas_object_hide(pd->hoversel);
}

static void
_popup_menu_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Object *top;

   widget = (Elm_Code_Widget *)obj;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->hoversel) evas_object_del(pd->hoversel);

   pd->hoversel = elm_hoversel_add(obj);
   elm_widget_sub_object_add(obj, pd->hoversel);
   top = elm_widget_top_get(obj);

   if (top) elm_hoversel_hover_parent_set(pd->hoversel, top);

   efl_event_callback_add
     (pd->hoversel, ELM_HOVERSEL_EVENT_DISMISSED, _popup_menu_dismissed_cb, obj);
   if (pd->selection)
     {
        if (pd->editable)
          {
             elm_hoversel_item_add
                (pd->hoversel, "Cut", NULL, ELM_ICON_NONE,
                 _popup_menu_cut_cb, obj);
          }
        elm_hoversel_item_add
           (pd->hoversel, "Copy", NULL, ELM_ICON_NONE,
            _popup_menu_copy_cb, obj);
        if (pd->editable)
          {
             elm_hoversel_item_add
                (pd->hoversel, "Paste", NULL, ELM_ICON_NONE,
                 _popup_menu_paste_cb, obj);
          }
        elm_hoversel_item_add
          (pd->hoversel, "Cancel", NULL, ELM_ICON_NONE,
           _popup_menu_cancel_cb, obj);
     }
   else
     {
        if (pd->editable)
          {
             elm_hoversel_item_add
               (pd->hoversel, "Paste", NULL, ELM_ICON_NONE,
                _popup_menu_paste_cb, obj);

          }
        else
          elm_hoversel_item_add
             (pd->hoversel, "Cancel", NULL, ELM_ICON_NONE,
              _popup_menu_cancel_cb, obj);
     }

   if (pd->hoversel)
     {
        evas_object_move(pd->hoversel, x, y);
        evas_object_show(pd->hoversel);
        elm_hoversel_hover_begin(pd->hoversel);
     }
}

static void
_elm_code_widget_clicked_gutter_cb(Elm_Code_Widget *widget, unsigned int row)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   line = elm_code_file_line_get(pd->code->file, row);
   if (!line)
     return;

   if (line->status_text)
     {
       elm_code_widget_line_status_toggle(widget, line);
       return;
     }

   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_LINE_GUTTER_CLICKED, line);
}

static void
_elm_code_widget_clicked_editable_cb(Elm_Code_Widget *widget, unsigned int row, unsigned int col)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int column_width;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   line = elm_code_file_line_get(pd->code->file, row);
   if (!line)
     return;
   column_width = elm_code_widget_line_text_column_width_get(widget, line);

   if (col > column_width + 1)
     col = column_width + 1;
   else if (col <= 0)
     col = 1;

   _elm_code_widget_cursor_move(widget, pd, col, row);
}

static void
_elm_code_widget_clicked_readonly_cb(Elm_Code_Widget *widget, unsigned int row)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   line = elm_code_file_line_get(pd->code->file, row);
   if (!line)
     return;

   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_LINE_CLICKED, line);
}

static void
_mouse_selection_paste_at_position(Elm_Code_Widget *widget,
                                   unsigned int row, unsigned int col)
{
   char *text;

   if (elm_code_widget_selection_is_empty(widget))
     return;

   text = elm_code_widget_selection_text_get(widget);
   elm_code_widget_selection_clear(widget);

   elm_code_widget_cursor_position_set(widget, row, col);
   elm_code_widget_text_at_cursor_insert(widget, text);

   free(text);
}

static Evas_Object *
_elm_code_widget_tooltip_cb(void *data, Evas_Object *obj EINA_UNUSED, Evas_Object *tooltip)
{
   Evas_Object *label;

   if (!data)
     return NULL;

   label = elm_label_add(tooltip);
   elm_object_text_set(label, (Eina_Stringshare *)data);
   return label;
}

static void
_elm_code_widget_tooltip_del_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                void *event_info EINA_UNUSED)
{
   if (!data)
     return;

   eina_stringshare_del((Eina_Stringshare *)data);
}

static void
_elm_code_widget_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                               void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Down *event;
   Eina_Bool ctrl, shift;
   unsigned int row;
   int col;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Down *)event_info;
   _elm_code_widget_position_at_coordinates_get(widget, pd, event->canvas.x, event->canvas.y, &row, &col);

   ctrl = evas_key_modifier_is_set(event->modifiers, "Control");
   shift = evas_key_modifier_is_set(event->modifiers, "Shift");
   if (!ctrl)
     {
        if (event->button == 3)
          {
             _popup_menu_show(widget, event->canvas.x, event->canvas.y);
             return;
          }
	else if (event->button == 2)
          {
             _mouse_selection_paste_at_position(widget, row, col);
             return;
          }
     }

   if (!shift)
     elm_code_widget_selection_clear(widget);

   if (event->flags & EVAS_BUTTON_TRIPLE_CLICK)
     {
        elm_code_widget_selection_select_line(widget, row);
        return;
     }
   else if (event->flags & EVAS_BUTTON_DOUBLE_CLICK)
     {
        elm_code_widget_selection_select_word(widget, row, col);
        return;
     }

   if (pd->editable)
     {
        if (shift && !pd->selection)
          elm_code_widget_selection_start(widget, pd->cursor_line, pd->cursor_col);
        _elm_code_widget_clicked_editable_cb(widget, row, (unsigned int) col);
        if (shift)
          elm_code_widget_selection_end(widget, pd->cursor_line, pd->cursor_col);
     }
}

static void
_elm_code_widget_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                            void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Move *event;
   unsigned int row;
   int col;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Move *)event_info;

   _elm_code_widget_position_at_coordinates_get(widget, pd, event->cur.canvas.x, event->cur.canvas.y, &row, &col);

   if (!pd->editable || !event->buttons)
     {
        Elm_Code_Line *line;
        Eina_Stringshare *text = NULL;

        line = elm_code_file_line_get(elm_code_widget_code_get(widget)->file, row);
        if (line && line->status_text)
          text = eina_stringshare_add(line->status_text);
        elm_object_tooltip_content_cb_set(pd->gridbox, _elm_code_widget_tooltip_cb,
                                          text, _elm_code_widget_tooltip_del_cb);

        return;
     }

   if (!pd->selection)
     if (col > 0 && row <= elm_code_file_lines_get(pd->code->file))
       elm_code_widget_selection_start(widget, row, col);

   _elm_code_widget_selection_type_set(widget, ELM_CODE_WIDGET_SELECTION_MOUSE);
   _elm_code_widget_selection_in_progress_set(widget, EINA_TRUE);


   elm_code_widget_selection_end(widget, row, col);
}

static void
_elm_code_widget_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                            void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Up *event;
   Evas_Coord x, y;
   unsigned int row;
   int col;
   Eina_Bool hasline;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Up *)event_info;

   if (pd->selection)
     {
        _elm_code_widget_selection_in_progress_set(widget, EINA_FALSE);

        if (pd->selection->start_line == pd->selection->end_line &&
            pd->selection->start_col == pd->selection->end_col)
          elm_code_widget_selection_clear(widget);
        else
          return;
     }

   x = event->canvas.x;
   y = event->canvas.y;
   hasline = _elm_code_widget_position_at_coordinates_get(widget, pd, x, y, &row, &col);
   if (!hasline)
     return;

   if (col <= 0)
     _elm_code_widget_clicked_gutter_cb(widget, row);
   else if (pd->editable)
     _elm_code_widget_clicked_editable_cb(widget, row, (unsigned int) col);
   else
     _elm_code_widget_clicked_readonly_cb(widget, row);
}

static void
_elm_code_widget_scroller_clicked_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                                     void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Down *event;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Down *)event_info;

   if (_elm_code_widget_position_at_coordinates_get(widget, pd,
         event->canvas.x, event->canvas.y, NULL, NULL))
     return;

   elm_code_widget_selection_clear(widget);
}

static void
_elm_code_widget_cursor_move_home(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->cursor_col <= 1)
     return;

   _elm_code_widget_cursor_move(widget, pd, 1, pd->cursor_line);
}

static void
_elm_code_widget_cursor_move_end(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int lastcol;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   line = elm_code_file_line_get(pd->code->file, pd->cursor_line);
   lastcol = elm_code_widget_line_text_column_width_get(widget, line);
   if (pd->cursor_col > lastcol + 1)
     return;

   _elm_code_widget_cursor_move(widget, pd, lastcol + 1, pd->cursor_line);
}

static void
_elm_code_widget_cursor_move_up(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col, column_width;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line <= 1)
     return;

   row--;
   line = elm_code_file_line_get(pd->code->file, row);
   column_width = elm_code_widget_line_text_column_width_get(widget, line);
   if (col > column_width + 1)
     col = column_width + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row);
}

static void
_elm_code_widget_cursor_move_down(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col, column_width;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line >= elm_code_file_lines_get(pd->code->file))
     return;

   row++;
   line = elm_code_file_line_get(pd->code->file, row);
   column_width = elm_code_widget_line_text_column_width_get(widget, line);
   if (col > column_width + 1)
     col = column_width + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row);
}

static void
_elm_code_widget_cursor_move_left(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->cursor_col <= 1)
     {
        if (pd->cursor_line > 1)
          {
             _elm_code_widget_cursor_move_up(widget);
             _elm_code_widget_cursor_move_end(widget);
          }
        return;
     }

   _elm_code_widget_cursor_move(widget, pd, pd->cursor_col-1, pd->cursor_line);
}

static void
_elm_code_widget_cursor_move_right(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int position, next_col;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   line = elm_code_file_line_get(pd->code->file, pd->cursor_line);
   if (pd->cursor_col > elm_code_widget_line_text_column_width_get(widget, line))
     {
        if (pd->cursor_line < elm_code_file_lines_get(pd->code->file))
          {
             _elm_code_widget_cursor_move_down(widget);
             _elm_code_widget_cursor_move_home(widget);
          }
        return;
     }

   next_col = pd->cursor_col + 1;
   position = elm_code_widget_line_text_position_for_column_get(widget, line, pd->cursor_col);
   if (elm_code_line_text_get(line, NULL)[position] == '\t')
     next_col = pd->cursor_col + pd->tabstop;

   _elm_code_widget_cursor_move(widget, pd, next_col, pd->cursor_line);
}

static unsigned int
_elm_code_widget_cursor_move_page_height_get(Elm_Code_Widget *widget)
{
   unsigned int lines;

   lines = elm_obj_code_widget_lines_visible_get(widget);
   return lines * 0.85;
}

static void
_elm_code_widget_cursor_move_pageup(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col, column_width;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line <= 1)
     return;

   if (row > _elm_code_widget_cursor_move_page_height_get(widget))
     row -= _elm_code_widget_cursor_move_page_height_get(widget);
   else
     row = 1;

   line = elm_code_file_line_get(pd->code->file, row);
   column_width = elm_code_widget_line_text_column_width_get(widget, line);
   if (col > column_width + 1)
     col = column_width + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row);
}

static void
_elm_code_widget_cursor_move_pagedown(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col, column_width;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line >= elm_code_file_lines_get(pd->code->file))
     return;

   row += _elm_code_widget_cursor_move_page_height_get(widget);
   if (row > elm_code_file_lines_get(pd->code->file))
     row = elm_code_file_lines_get(pd->code->file);

   line = elm_code_file_line_get(pd->code->file, row);
   column_width = elm_code_widget_line_text_column_width_get(widget, line);
   if (col > column_width + 1)
     col = column_width + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row);
}

static Elm_Code_Widget_Change_Info *
_elm_code_widget_change_create(unsigned int start_col, unsigned int start_line,
                               unsigned int end_col, unsigned int end_line,
                               const char *text, unsigned int length, Eina_Bool insert)
{
   Elm_Code_Widget_Change_Info *info;

   info = calloc(1, sizeof(*info));
   info->insert = insert;

   info->start_col = start_col;
   info->start_line = start_line;
   info->end_col = end_col;
   info->end_line = end_line;

   info->content = strndup(text, length);
   info->length = length;

   return info;
}

static void
_elm_code_widget_change_free(Elm_Code_Widget_Change_Info *info)
{
   free((char *)info->content);
   free(info);
}

void
_elm_code_widget_change_selection_add(Evas_Object *widget)
{
   Elm_Code_Widget_Change_Info *change;
   Elm_Code_Widget_Selection_Data *selection;
   char *selection_text;

   if (elm_code_widget_selection_is_empty(widget))
     return;

   selection_text = elm_code_widget_selection_text_get(widget);
   selection = elm_code_widget_selection_normalized_get(widget);

   change = _elm_code_widget_change_create(selection->start_col,
                                           selection->start_line,
                                           selection->end_col,
                                           selection->end_line,
                                           selection_text,
                                           strlen(selection_text),
                                           EINA_FALSE);
   _elm_code_widget_undo_change_add(widget, change);
   _elm_code_widget_change_free(change);
   free(selection_text);
   free(selection);
}

static void
_elm_code_widget_tab_at_cursor_insert(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   unsigned int col, row, rem;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!pd->tab_inserts_spaces)
     {
        elm_code_widget_text_at_cursor_insert(widget, "\t");
        return;
     }

   elm_obj_code_widget_cursor_position_get(widget, &row, &col);
   rem = (col - 1) % pd->tabstop;

   while (rem < pd->tabstop)
     {
        elm_code_widget_text_at_cursor_insert(widget, " ");
        rem++;
     }
}

void
_elm_code_widget_newline(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   Elm_Code_Widget_Change_Info *change;
   unsigned int row, col, position, oldlen, width, indent, textlen;
   char *oldtext, *leading, *text;

   if (!elm_code_widget_selection_is_empty(widget))
     elm_code_widget_selection_delete(widget);

   code = elm_obj_code_widget_code_get(widget);
   elm_obj_code_widget_cursor_position_get(widget, &row, &col);
   line = elm_code_file_line_get(code->file, row);
   if (line == NULL)
     {
        elm_code_file_line_append(code->file, "", 0, NULL);
        row = elm_code_file_lines_get(code->file);
        line = elm_code_file_line_get(code->file, row);
     }
   oldtext = (char *) elm_code_line_text_get(line, &oldlen);
   oldtext = strndup(oldtext, oldlen);

   position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
   elm_code_line_split_at(line, position);
   width = elm_code_widget_line_text_column_width_get(widget, line);

   line = elm_code_file_line_get(code->file, row + 1);
   leading = elm_code_line_indent_get(line);
   elm_code_line_text_leading_whitespace_strip(line);
   elm_code_widget_cursor_position_set(widget, row + 1, 1);
   elm_code_widget_text_at_cursor_insert(widget, leading);
   free(oldtext);

   indent = elm_obj_code_widget_line_text_column_width_to_position(widget, line,
     strlen(leading));
   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER, NULL);

   textlen = strlen(leading) + 2;
   text = malloc(sizeof(char) * textlen);
   snprintf(text, textlen, "\n%s", leading);
   free(leading);

   change = _elm_code_widget_change_create(width + 1, row, indent - 1, row + 1, text, strlen(text), EINA_TRUE);
   _elm_code_widget_undo_change_add(widget, change);
   _elm_code_widget_change_free(change);
   free(text);
}

static void
_elm_code_widget_backspaceline(Elm_Code_Widget *widget, Eina_Bool nextline)
{
   Elm_Code *code;
   Elm_Code_Line *line, *oldline;
   unsigned int row, col, oldlength, position;

   code = elm_obj_code_widget_code_get(widget);
   elm_obj_code_widget_cursor_position_get(widget, &row, &col);
   line = elm_code_file_line_get(code->file, row);

   if (nextline)
     {
        elm_code_widget_selection_start(widget, row, col);
        elm_code_widget_selection_end(widget, row + 1, 0);
        _elm_code_widget_change_selection_add(widget);

        elm_code_line_merge_down(line);
     }
   else
     {
        oldline = elm_code_file_line_get(code->file, row - 1);
        elm_code_line_text_get(oldline, &oldlength);

        position = elm_code_widget_line_text_column_width_to_position(widget, oldline, oldlength);
        elm_code_widget_selection_start(widget, row - 1, position);
        elm_code_widget_selection_end(widget, row, 0);
        _elm_code_widget_change_selection_add(widget);

        elm_code_line_merge_up(line);
     }
   elm_code_widget_selection_clear(widget);
// TODO construct and pass a change object
   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER, NULL);
}

void
_elm_code_widget_backspace(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   Elm_Code_Widget_Change_Info *change;
   unsigned int row, col, position, start_col, end_col, char_width;
   const char *text;

   if (!elm_code_widget_selection_is_empty(widget))
     {
        elm_code_widget_selection_delete(widget);
        return;
     }

   code = elm_obj_code_widget_code_get(widget);
   elm_obj_code_widget_cursor_position_get(widget, &row, &col);

   if (col <= 1)
     {
        if (row == 1)
          return;

        _elm_code_widget_backspaceline(widget, EINA_FALSE);
        return;
     }

   line = elm_code_file_line_get(code->file, row);

   position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
   end_col = elm_code_widget_line_text_column_width_to_position(widget, line, position);
   start_col = elm_code_widget_line_text_column_width_to_position(widget, line,
      elm_code_widget_line_text_position_for_column_get(widget, line, col - 1));
   char_width = position - elm_code_widget_line_text_position_for_column_get(widget, line, start_col);

   text = elm_code_widget_text_between_positions_get(widget, row, start_col, row, end_col);
   elm_code_line_text_remove(line, position - char_width, char_width);
   elm_obj_code_widget_cursor_position_set(widget, row, start_col);

   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER, NULL);

   change = _elm_code_widget_change_create(start_col, row, end_col - 1, row, text, char_width, EINA_FALSE);
   _elm_code_widget_undo_change_add(widget, change);
   _elm_code_widget_change_free(change);
}

void
_elm_code_widget_delete(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   Elm_Code_Widget_Change_Info *change;
   unsigned int row, col, position, char_width, start_col, end_col;
   const char *text;

   if (!elm_code_widget_selection_is_empty(widget))
     {
        elm_code_widget_selection_delete(widget);
        return;
     }

   code = elm_obj_code_widget_code_get(widget);
   elm_obj_code_widget_cursor_position_get(widget, &row, &col);
   line = elm_code_file_line_get(code->file, row);
   if (col > elm_code_widget_line_text_column_width_get(widget, line))
     {
        if (row == elm_code_file_lines_get(code->file))
          return;

        _elm_code_widget_backspaceline(widget, EINA_TRUE);
        return;
     }

   position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
   char_width = elm_code_widget_line_text_position_for_column_get(widget, line, col + 1) - position;
   if (char_width == 0) // a partial tab
     char_width = 1;
   start_col = elm_code_widget_line_text_column_width_to_position(widget, line, position);
   end_col = elm_code_widget_line_text_column_width_to_position(widget, line, position + char_width);

   text = elm_code_widget_text_between_positions_get(widget, row, start_col, row, end_col);
   elm_code_line_text_remove(line, position, char_width);
   elm_obj_code_widget_cursor_position_set(widget, row, start_col);
   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER, NULL);

   change = _elm_code_widget_change_create(start_col, row, col - 1, row, text, char_width, EINA_FALSE);
   _elm_code_widget_undo_change_add(widget, change);
   _elm_code_widget_change_free(change);
}

static void
_elm_code_widget_control_key_down_cb(Elm_Code_Widget *widget, const char *key)
{
   if (!key)
     return;

   if (!strcmp("c", key))
     {
        elm_code_widget_selection_copy(widget);
        return;
     }

   if (!strcmp("v", key))
     elm_code_widget_selection_paste(widget);
   else if (!strcmp("x", key))
     elm_code_widget_selection_cut(widget);
   else if (!strcmp("y", key))
     elm_code_widget_redo(widget);
   else if (!strcmp("z", key))
     elm_code_widget_undo(widget);
   else if (!strcmp("a", key))
     elm_code_widget_selection_select_all(widget);
}

static Eina_Bool
_elm_code_widget_key_cursor_is(const char *key)
{
   if (!strcmp(key, "Up") || !strcmp(key, "Down"))
     return EINA_TRUE;
   if (!strcmp(key, "Left") || !strcmp(key, "Right"))
     return EINA_TRUE;
   if (!strcmp(key, "Home") || !strcmp(key, "End"))
     return EINA_TRUE;
   if (!strcmp(key, "Prior") || !strcmp(key, "Next"))
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_elm_code_widget_key_down_cb(void *data, Evas *evas EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Eina_Bool shift, adjust, backwards = EINA_FALSE;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   Evas_Event_Key_Down *ev = event_info;

   if (!pd->editable || (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     return;

#if defined(__APPLE__) && defined(__MACH__)
   if (evas_key_modifier_is_set(ev->modifiers, "Super"))
#else
   if (evas_key_modifier_is_set(ev->modifiers, "Control"))
#endif
     {
        _elm_code_widget_control_key_down_cb(widget, ev->key);
        return;
     }

   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   if (_elm_code_widget_key_cursor_is(ev->key))
     {
        if (shift)
          {
             backwards = !strcmp(ev->key, "Up") || !strcmp(ev->key, "Left") ||
                         !strcmp(ev->key, "Home") || !strcmp(ev->key, "Prior");

             if (!pd->selection)
               elm_code_widget_selection_start(widget, pd->cursor_line, pd->cursor_col - (backwards?1:0));

             _elm_code_widget_selection_type_set(widget, ELM_CODE_WIDGET_SELECTION_KEYBOARD);
             _elm_code_widget_selection_in_progress_set(widget, EINA_TRUE);

             if (pd->selection && pd->selection->start_line == pd->selection->end_line)
               {
                  if ((pd->selection->end_col == pd->selection->start_col && !backwards) ||
                      (pd->selection->end_col > pd->selection->start_col))
                    elm_code_widget_cursor_position_set(widget, pd->selection->end_line, pd->selection->end_col+1);
               }
             else if (pd->selection && pd->selection->end_line > pd->selection->start_line)
               {
                    elm_code_widget_cursor_position_set(widget, pd->selection->end_line, pd->selection->end_col+1);
               }
          }
        else
          elm_code_widget_selection_clear(widget);

        if (!strcmp(ev->key, "Up"))
          _elm_code_widget_cursor_move_up(widget);
        else if (!strcmp(ev->key, "Down"))
          _elm_code_widget_cursor_move_down(widget);
        else if (!strcmp(ev->key, "Left"))
          _elm_code_widget_cursor_move_left(widget);
        else if (!strcmp(ev->key, "Right"))
          _elm_code_widget_cursor_move_right(widget);
        else if (!strcmp(ev->key, "Home"))
          _elm_code_widget_cursor_move_home(widget);
        else if (!strcmp(ev->key, "End"))
          _elm_code_widget_cursor_move_end(widget);
        else if (!strcmp(ev->key, "Prior"))
          _elm_code_widget_cursor_move_pageup(widget);
        else if (!strcmp(ev->key, "Next"))
          _elm_code_widget_cursor_move_pagedown(widget);

        if (shift && pd->selection)
          {
             if (pd->selection->start_line == pd->selection->end_line)
               adjust = (pd->selection->end_col > pd->selection->start_col) ||
                         (!backwards && (pd->selection->end_col == pd->selection->start_col));
             else
               adjust = (pd->selection->end_line > pd->selection->start_line);

             elm_code_widget_selection_end(widget, pd->cursor_line, pd->cursor_col - (adjust?1:0));
             _elm_code_widget_selection_in_progress_set(widget, EINA_FALSE);
          }
     }

   else if (!strcmp(ev->key, "KP_Enter") || !strcmp(ev->key, "Return"))
     _elm_code_widget_newline(widget);
   else if (!strcmp(ev->key, "BackSpace"))
     _elm_code_widget_backspace(widget);
   else if (!strcmp(ev->key, "Delete"))
     _elm_code_widget_delete(widget);
   else if (!strcmp(ev->key, "Tab"))
     _elm_code_widget_tab_at_cursor_insert(widget);

   else if (!strcmp(ev->key, "Escape"))
     elm_code_widget_selection_clear(widget);

   else if (ev->string)
     elm_code_widget_text_at_cursor_insert(widget, ev->string);
   else
     INF("Unhandled key %s (%s)", ev->key, ev->keyname);
}

static void
_elm_code_widget_focused_event_cb(void *data, Evas_Object *obj,
                                  void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->focussed = EINA_TRUE;
   elm_layout_signal_emit(pd->cursor_rect, "elm,action,focus", "elm");

   _elm_code_widget_refresh(obj, NULL);
}

static void
_elm_code_widget_unfocused_event_cb(void *data, Evas_Object *obj,
                                    void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->focussed = EINA_FALSE;
   elm_layout_signal_emit(pd->cursor_rect, "elm,action,unfocus", "elm");

  _elm_code_widget_refresh(obj, NULL);
}

static void
_elm_code_widget_scroll_event_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                 void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_refresh(widget, NULL);
}

EOLIAN static Eina_Bool
_elm_code_widget_efl_ui_widget_widget_event(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd EINA_UNUSED, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
{
   Eo *ev = eo_event->info;

   if (efl_input_processed_get(ev)) return EINA_FALSE;
   if (eo_event->desc != EFL_EVENT_KEY_DOWN) return EINA_FALSE;

   // FIXME: This should use key bindings and the standard implementation!
   if (eina_streq(efl_input_key_get(ev), "BackSpace"))
     {
        efl_input_processed_set(ev, EINA_TRUE);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

// load a named colour class from the theme and apply it to the grid's specified palette
static void
_elm_code_widget_setup_palette_item(Evas_Object *grid, int type, const char *name,
                                    double fade, Evas_Object *edje)
{
   int r, g, b, a;

   if (!edje_object_color_class_get(edje, name, &r, &g, &b, &a,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
     return;

   evas_object_textgrid_palette_set(grid, EVAS_TEXTGRID_PALETTE_STANDARD, type,
                                    r * fade, g * fade, b * fade, a * fade);
}

static void
_elm_code_widget_setup_palette(Evas_Object *o, Evas_Object *layout, float fade)
{
   Evas_Object *edje;

   edje = elm_layout_edje_get(layout);

   // setup status colors
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_DEFAULT, "elm/code/status/default", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_CURRENT, "elm/code/status/current", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_IGNORED, "elm/code/status/ignored", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_NOTE, "elm/code/status/note", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_WARNING, "elm/code/status/warning", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_ERROR, "elm/code/status/error", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_FATAL, "elm/code/status/fatal", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_ADDED, "elm/code/status/added", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_REMOVED, "elm/code/status/removed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_CHANGED, "elm/code/status/changed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_PASSED, "elm/code/status/passed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_FAILED, "elm/code/status/failed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_STATUS_TYPE_TODO, "elm/code/status/todo", fade, edje);

   // setup token colors
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_DEFAULT, "elm/code/token/default", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_COMMENT, "elm/code/token/comment", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_STRING, "elm/code/token/string", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_NUMBER, "elm/code/token/number", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_BRACE, "elm/code/token/brace", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_TYPE, "elm/code/token/type", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_CLASS, "elm/code/token/class", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_FUNCTION, "elm/code/token/function", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_PARAM, "elm/code/token/param", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_KEYWORD, "elm/code/token/keyword", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_PREPROCESSOR, "elm/code/token/preprocessor", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_ADDED, "elm/code/token/added", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_REMOVED, "elm/code/token/removed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_CHANGED, "elm/code/token/changed", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_TOKEN_TYPE_MATCH, "elm/code/token/match", fade, edje);

   // other styles that the widget uses
   _elm_code_widget_setup_palette_item(o, ELM_CODE_WIDGET_COLOR_SELECTION, "elm/code/widget/color/selection", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_WIDGET_COLOR_GUTTER_BG, "elm/code/widget/color/gutter/bg", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_WIDGET_COLOR_GUTTER_FG, "elm/code/widget/color/gutter/fg", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_WIDGET_COLOR_GUTTER_SCOPE_BG, "elm/code/widget/color/scope/bg", fade, edje);
   _elm_code_widget_setup_palette_item(o, ELM_CODE_WIDGET_COLOR_WHITESPACE, "elm/code/widget/color/whitespace", fade, edje);
}

static void
_elm_code_widget_ensure_n_grid_rows(Elm_Code_Widget *widget, int rows)
{
   Evas_Object *grid;
   int existing, i;
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   existing = eina_list_count(pd->grids);

   // trim unneeded rows in our rendering
   if (rows < existing)
     {
        for (i = existing - rows; i > 0; i--)
          {
             grid = eina_list_data_get(eina_list_last(pd->grids));
             evas_object_del(grid);
             elm_box_unpack(pd->gridbox, grid);
             pd->grids = eina_list_remove_list(pd->grids, eina_list_last(pd->grids));
          }
        rows = existing;
     }

   if (rows == existing)
     return;

   for (i = existing; i < rows; i++)
     {
        grid = evas_object_textgrid_add(pd->gridbox);
        evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, 0.0);
        evas_object_show(grid);
        _elm_code_widget_setup_palette(grid, widget, (double) pd->alpha / 255);

        elm_box_pack_end(pd->gridbox, grid);
        pd->grids = eina_list_append(pd->grids, grid);

        evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_DOWN, _elm_code_widget_mouse_down_cb, widget);
        evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_MOVE, _elm_code_widget_mouse_move_cb, widget);
        evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_UP, _elm_code_widget_mouse_up_cb, widget);

        evas_object_textgrid_font_set(grid, pd->font_name, pd->font_size * elm_config_scale_get());
     }

   elm_box_recalculate(pd->gridbox);
}

static void
_elm_code_widget_resize(Elm_Code_Widget *widget, Elm_Code_Line *newline)
{
   Elm_Code_Line *line;
   Eina_List *item;
   Evas_Object *grid;
   Evas_Coord ww, wh, old_width, old_height;
   int w, h, cw = 0, ch = 0, gutter;
   unsigned int line_width;
   Elm_Code_Widget_Data *pd;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_obj_code_widget_text_left_gutter_width_get(widget);

   if (!pd->code)
     return;

   if (!pd->code->file->lines)
     return;

   evas_object_geometry_get(widget, NULL, NULL, &ww, &wh);

   old_width = ww;
   old_height = wh;
   w = 0;
   h = elm_code_file_lines_get(pd->code->file);

   EINA_LIST_FOREACH(pd->code->file->lines, item, line)
     {
        line_width = elm_code_widget_line_text_column_width_get(widget, line);
        if ((int) line_width + gutter + 1 > w)
          w = (int) line_width + gutter + 1;
     }

   _elm_code_widget_ensure_n_grid_rows(widget, h);
   _elm_code_widget_cell_size_get(widget, &cw, &ch);
   if (w*cw > ww)
     ww = w*cw;
   if (h*ch > wh)
     wh = h*ch;

   if (cw > 0 && ww/cw > w)
     pd->col_count = ww/cw;
   else
     pd->col_count = w;

   EINA_LIST_FOREACH(pd->grids, item, grid)
     {
        evas_object_textgrid_size_set(grid, pd->col_count, 1);
        evas_object_size_hint_min_set(grid, ww, ch);
     }

   if (!newline)
     {
        unsigned int first_row, last_row;

        if (!_elm_code_widget_viewport_get(widget, pd, &first_row, &last_row))
          return ;

        _elm_code_widget_fill_range(widget, pd, first_row, last_row, NULL);

        return;
     }

   if (pd->gravity_x == 1.0 || pd->gravity_y == 1.0)
     _elm_code_widget_scroll_by(widget,
        (pd->gravity_x == 1.0 && ww > old_width) ? ww - old_width : 0,
        (pd->gravity_y == 1.0 && wh > old_height) ? wh - old_height : 0);
   elm_box_recalculate(pd->gridbox);
}

EOAPI void
_elm_code_widget_line_refresh(Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED, Elm_Code_Line *line)
{
   _elm_code_widget_fill_line(obj, line);
}

EOAPI Eina_Bool
_elm_code_widget_line_visible_get(Eo *obj, Elm_Code_Widget_Data *pd, Elm_Code_Line *line)
{
   Evas_Coord cellh = 0, viewy = 0, viewh = 0;

   elm_scroller_region_get(pd->scroller, NULL, &viewy, NULL, &viewh);
   _elm_code_widget_cell_size_get(obj, NULL, &cellh);

   if (((int)line->number - 1) * cellh > viewy + viewh || (int)line->number * cellh < viewy)
     return EINA_FALSE;

   return EINA_TRUE;;
}

EOAPI unsigned int
_elm_code_widget_lines_visible_get(Eo *obj, Elm_Code_Widget_Data *pd)
{
   Evas_Coord cellh = 0, viewh = 0;

   elm_scroller_region_get(pd->scroller, NULL, NULL, NULL, &viewh);
   _elm_code_widget_cell_size_get(obj, NULL, &cellh);

   if (cellh == 0)
     return 0;
   return viewh / cellh + 1;
}

EOLIAN static void
_elm_code_widget_font_set(Eo *obj, Elm_Code_Widget_Data *pd,
                          const char *name, Evas_Font_Size size)
{
   Eina_List *item;
   Evas_Object *grid;

   const char *face = name;
   if (!face)
     face = "Mono";

   if (size == pd->font_size && !strcmp(face, pd->font_name))
     return;

   EINA_LIST_FOREACH(pd->grids, item, grid)
     {
        evas_object_textgrid_font_set(grid, face, size * elm_config_scale_get());
     }

   if (pd->cursor_rect && (eina_list_count(pd->grids) >= pd->cursor_line))
     _elm_code_widget_cursor_update(obj, pd);

   if (pd->font_name)
     eina_stringshare_del((char *)pd->font_name);
   pd->font_name = eina_stringshare_add(face);
   pd->font_size = size;
}

EOLIAN static void
_elm_code_widget_font_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd,
                          const char **name, Evas_Font_Size *size)
{
   if (name)
     *name = strdup((const char *)pd->font_name);
   if (size)
     *size = pd->font_size;
}

EOLIAN static unsigned int
_elm_code_widget_columns_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->col_count;
}

EOLIAN static void
_elm_code_widget_code_set(Eo *obj, Elm_Code_Widget_Data *pd, Elm_Code *code)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   pd->code = code;

   if (code)
     code->widgets = eina_list_append(code->widgets, obj);
}

EOLIAN static Elm_Code *
_elm_code_widget_code_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->code;
}

EOLIAN static void
_elm_code_widget_gravity_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, double x, double y)
{
   pd->gravity_x = x;
   pd->gravity_y = y;
}

EOLIAN static void
_elm_code_widget_gravity_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, double *x, double *y)
{
   *x = pd->gravity_x;
   *y = pd->gravity_y;
}

EOLIAN static void
_elm_code_widget_policy_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   elm_scroller_policy_set(pd->scroller, policy_h, policy_v);
}

EOLIAN static void
_elm_code_widget_policy_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   elm_scroller_policy_get(pd->scroller, policy_h, policy_v);
}

EOLIAN static void
_elm_code_widget_tabstop_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, unsigned int tabstop)
{
   pd->tabstop = tabstop;
   _elm_code_widget_fill(obj);
}

EOLIAN static unsigned int
_elm_code_widget_tabstop_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->tabstop;
}

EOLIAN static void
_elm_code_widget_editable_set(Eo *obj, Elm_Code_Widget_Data *pd, Eina_Bool editable)
{
   pd->editable = editable;
   elm_object_focus_allow_set(obj, editable);
}

EOLIAN static Eina_Bool
_elm_code_widget_editable_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->editable;
}

EOLIAN static void
_elm_code_widget_line_numbers_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, Eina_Bool line_numbers)
{
   pd->show_line_numbers = line_numbers;
}

EOLIAN static Eina_Bool
_elm_code_widget_line_numbers_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->show_line_numbers;
}

EOLIAN static void
_elm_code_widget_line_width_marker_set(Eo *obj, Elm_Code_Widget_Data *pd, unsigned int col)
{
   pd->line_width_marker = col;
   _elm_code_widget_fill(obj);
}

EOLIAN static unsigned int
_elm_code_widget_line_width_marker_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->line_width_marker;
}

EOLIAN static void
_elm_code_widget_show_whitespace_set(Eo *obj, Elm_Code_Widget_Data *pd, Eina_Bool show)
{
   pd->show_whitespace = show;
   _elm_code_widget_fill(obj);
}

EOLIAN static Eina_Bool
_elm_code_widget_show_whitespace_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->show_whitespace;
}

EOLIAN static void
_elm_code_widget_syntax_enabled_set(Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED,
                                    Eina_Bool enabled)
{
   Elm_Code_Widget *widget = obj;
   Elm_Code *code;

   code = elm_code_widget_code_get(widget);
   if (enabled)
     elm_code_parser_standard_add(code, ELM_CODE_PARSER_STANDARD_SYNTAX);
   else
     code->parsers = eina_list_remove(code->parsers, ELM_CODE_PARSER_STANDARD_SYNTAX);

   _elm_code_parse_reset_file(code, code->file);
   _elm_code_widget_fill(obj);
}

EOLIAN static Eina_Bool
_elm_code_widget_syntax_enabled_get(const Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED)
{
   const Elm_Code_Widget *widget = obj;
   Elm_Code *code;

   code = elm_code_widget_code_get(widget);
   return !!eina_list_data_find(code->parsers, ELM_CODE_PARSER_STANDARD_SYNTAX);
}

EOLIAN static void
_elm_code_widget_tab_inserts_spaces_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd,
                                        Eina_Bool spaces)
{
   pd->tab_inserts_spaces = spaces;
   if (!spaces)
     elm_code_widget_code_get(obj)->config.indent_style_efl = EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_code_widget_tab_inserts_spaces_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->tab_inserts_spaces;
}

EOLIAN static void
_elm_code_widget_cursor_position_set(Eo *obj, Elm_Code_Widget_Data *pd, unsigned int row, unsigned int col)
{
   _elm_code_widget_cursor_move(obj, pd, col, row);
}

EOLIAN static void
_elm_code_widget_cursor_position_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, unsigned int *row, unsigned int *col)
{
   *row = pd->cursor_line;
   *col = pd->cursor_col;
}

EOLIAN static void
_elm_code_widget_theme_refresh(Eo *obj, Elm_Code_Widget_Data *pd)
{
   Eo *edje;
   int r, g, b, a;
   unsigned int i;
   double fade;
   Evas_Object *grid;

   edje = elm_layout_edje_get(obj);
   edje_object_color_class_get(edje, "elm/code/status/default", &r, &g, &b, &a,
                               NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

   fade = (double) pd->alpha / 255;
   evas_object_color_set(pd->background, r * fade, g * fade, b * fade, a * fade);

   for (i = 0; i < eina_list_count(pd->grids); i++)
     {
        grid = eina_list_nth(pd->grids, i);
        _elm_code_widget_setup_palette(grid, obj, fade);
     }
}

EOLIAN static Efl_Ui_Theme_Apply
_elm_code_widget_efl_ui_widget_theme_apply(Eo *obj, Elm_Code_Widget_Data *pd)
{
   if (!efl_ui_widget_theme_apply(efl_cast(obj, EFL_UI_WIDGET_CLASS)))
     return EFL_UI_THEME_APPLY_FAILED;

   _elm_code_widget_theme_refresh(obj, pd);

   return EFL_UI_THEME_APPLY_SUCCESS;
}

EOLIAN static int
_elm_code_widget_alpha_get(const Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->alpha;
}

EOLIAN static void
_elm_code_widget_alpha_set(Eo *obj, Elm_Code_Widget_Data *pd, int alpha)
{
   pd->alpha = alpha;

   _elm_code_widget_efl_ui_widget_theme_apply(obj, pd);
}

EOLIAN static void
_elm_code_widget_efl_canvas_group_group_add(Eo *obj, Elm_Code_Widget_Data *pd)
{
   Evas_Object *gridrows, *scroller, *background;
   const char *fontname, *fontsize;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, ELM_CODE_WIDGET_CLASS));
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "code");
   elm_widget_theme_element_set(obj, "layout");

   elm_object_focus_allow_set(obj, EINA_TRUE);
   pd->alpha = 255;

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   scroller = elm_scroller_add(obj);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(scroller);
   elm_layout_content_set(obj, "elm.swallow.content", scroller);
   elm_object_focus_allow_set(scroller, EINA_FALSE);
   pd->scroller = scroller;
   evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_DOWN,
                                  _elm_code_widget_scroller_clicked_cb, obj);

   background = evas_object_rectangle_add(scroller);
   evas_object_size_hint_weight_set(background, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(background, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(background);
   elm_object_part_content_set(scroller, "elm.swallow.background", background);
   pd->background = background;

   fontname = edje_object_data_get(elm_layout_edje_get(obj), "font.name");
   fontsize = edje_object_data_get(elm_layout_edje_get(obj), "font.size");
   if (fontname && fontsize)
     _elm_code_widget_font_set(obj, pd, fontname, atoi(fontsize));

   gridrows = elm_box_add(scroller);
   evas_object_size_hint_weight_set(gridrows, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(gridrows, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(scroller, gridrows);
   pd->gridbox = gridrows;

   _elm_code_widget_efl_ui_widget_theme_apply(obj, pd);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _elm_code_widget_resize_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _elm_code_widget_key_down_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _elm_code_widget_hidden_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _elm_code_widget_show_cb, obj);

   evas_object_smart_callback_add(obj, "focused", _elm_code_widget_focused_event_cb, obj);
   evas_object_smart_callback_add(obj, "unfocused", _elm_code_widget_unfocused_event_cb, obj);
   evas_object_smart_callback_add(scroller, "scroll", _elm_code_widget_scroll_event_cb, obj);

   efl_event_callback_add(obj, &ELM_CODE_EVENT_LINE_LOAD_DONE, _elm_code_widget_line_cb, obj);
   efl_event_callback_add(obj, &ELM_CODE_EVENT_FILE_LOAD_DONE, _elm_code_widget_file_cb, obj);
   efl_event_callback_add(obj, ELM_OBJ_CODE_WIDGET_EVENT_SELECTION_CHANGED, _elm_code_widget_selection_cb, obj);
   efl_event_callback_add(obj, ELM_OBJ_CODE_WIDGET_EVENT_SELECTION_CLEARED, _elm_code_widget_selection_clear_cb, obj);
}

/* Internal EO APIs and hidden overrides */

#define ELM_CODE_WIDGET_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_code_widget)

#include "elm_code_widget_text.c"
#include "elm_code_widget_undo.c"
#include "elm_code_widget.eo.c"
