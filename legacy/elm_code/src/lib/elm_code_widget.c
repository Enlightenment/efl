#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include <Elm_Code.h>
#include "elm_code_private.h"

typedef enum {
   ELM_CODE_WIDGET_COLOR_GUTTER_BG = ELM_CODE_TOKEN_TYPE_COUNT,
   ELM_CODE_WIDGET_COLOR_GUTTER_FG,
   ELM_CODE_WIDGET_COLOR_WHITESPACE,
   ELM_CODE_WIDGET_COLOR_CURSOR,
   ELM_CODE_WIDGET_COLOR_SELECTION,

   ELM_CODE_WIDGET_COLOR_COUNT
} Elm_Code_Widget_Colors;

Eina_Unicode status_icons[] = {
 ' ',
 ' ',
 ' ',
 ' ',
 '!',
 '!',

 '+',
 '-',
 ' ',

 0x2713,
 0x2717,

 0
};

#define EO_CONSTRUCTOR_CHECK_RETURN(obj) do { \
   Eina_Bool finalized; \
   if (eo_do_ret(obj, finalized, eo_finalized_get())) \
     { \
        ERR("This function is only allowed during construction."); \
        return; \
     } \
} while (0)

EOLIAN static void
_elm_code_widget_eo_base_constructor(Eo *obj, Elm_Code_Widget_Data *pd)
{
   eo_do_super(obj, ELM_CODE_WIDGET_CLASS, eo_constructor());

   pd->cursor_line = 1;
   pd->cursor_col = 1;
}

EOLIAN static Eo *
_elm_code_widget_eo_base_finalize(Eo *obj, Elm_Code_Widget_Data *pd)
{
   eo_do_super(obj, ELM_CODE_WIDGET_CLASS, eo_finalize());

   if (pd->code)
     return obj;

   ERR("Elm_Code_Widget cannot finalize without calling elm_code_widget_code_set.");
   return NULL;
}

EOLIAN static void
_elm_code_widget_class_constructor(Eo_Class *klass EINA_UNUSED)
{

}

static void
_elm_code_widget_scroll_by(Elm_Code_Widget *widget, int by_x, int by_y)
{
   Elm_Code_Widget_Data *pd;
   Evas_Coord x, y, w, h;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_scroller_region_get(pd->scroller, &x, &y, &w, &h);
   x += by_x;
   y += by_y;
   elm_scroller_region_show(pd->scroller, x, y, w, h);
}

static void
_elm_code_widget_resize(Elm_Code_Widget *widget)
{
   Elm_Code_Line *line;
   Eina_List *item;
   Evas_Coord ww, wh, old_width, old_height;
   int w, h, cw, ch, gutter;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_code_widget_text_left_gutter_width_get(widget);

   if (!pd->code)
     return;

   evas_object_geometry_get(widget, NULL, NULL, &ww, &wh);
   evas_object_textgrid_cell_size_get(pd->grid, &cw, &ch);
   old_width = ww;
   old_height = wh;

   w = 0;
   h = elm_code_file_lines_get(pd->code->file);
   EINA_LIST_FOREACH(pd->code->file->lines, item, line)
     if ((int) line->unicode_length + gutter + 1 > w)
        w = (int) line->unicode_length + gutter + 1;

   if (w*cw > ww)
     ww = w*cw;
   if (h*ch > wh)
     wh = h*ch;

   evas_object_textgrid_size_set(pd->grid, ww/cw+1, wh/ch+1);
   evas_object_size_hint_min_set(pd->grid, w*cw, h*ch);

   if (pd->gravity_x == 1.0 || pd->gravity_y == 1.0)
     _elm_code_widget_scroll_by(widget, 
        (pd->gravity_x == 1.0 && ww > old_width) ? ww - old_width : 0,
        (pd->gravity_y == 1.0 && wh > old_height) ? wh - old_height : 0);
}

static void
_elm_code_widget_fill_line_token(Evas_Textgrid_Cell *cells, int count, int start, int end, Elm_Code_Token_Type type)
{
   int x;

   for (x = start; x <= end && x < count; x++)
     {
        cells[x].fg = type;
     }
}

static unsigned int
_elm_code_widget_status_type_get(Elm_Code_Widget *widget, Elm_Code_Status_Type set_type, unsigned int col)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (set_type != ELM_CODE_STATUS_TYPE_DEFAULT)
     return set_type;

   if (pd->line_width_marker == col)
     return ELM_CODE_WIDGET_COLOR_GUTTER_BG;

   return ELM_CODE_STATUS_TYPE_DEFAULT;
}

static void
_elm_code_widget_fill_line_tokens(Elm_Code_Widget *widget, Evas_Textgrid_Cell *cells,
                                  unsigned int count, Elm_Code_Line *line)
{
   Eina_List *item;
   Elm_Code_Token *token;
   const char *content;
   unsigned int start, end, length, offset;
   unsigned int token_start_col, token_end_col;

   offset = elm_code_widget_text_left_gutter_width_get(widget);
   start = offset;
   content = elm_code_line_text_get(line, NULL);
   length = line->unicode_length + offset;

   EINA_LIST_FOREACH(line->tokens, item, token)
     {
        token_start_col = elm_code_text_unicode_strlen(content, token->start - 1) + offset;
        token_end_col = elm_code_text_unicode_strlen(content, token->end - 1) + offset;

        if (token_start_col > start)
          _elm_code_widget_fill_line_token(cells, count, start, token_start_col, ELM_CODE_TOKEN_TYPE_DEFAULT);

        // TODO handle a token starting before the previous finishes
        end = token_end_col;
        if (token->end_line > line->number)
          end = length + offset;
        _elm_code_widget_fill_line_token(cells, count, token_start_col, end, token->type);

        start = end + 1;
     }

   _elm_code_widget_fill_line_token(cells, count, start, length, ELM_CODE_TOKEN_TYPE_DEFAULT);
}

static void
_elm_code_widget_fill_gutter(Elm_Code_Widget *widget, Evas_Textgrid_Cell *cells,
                             Elm_Code_Status_Type status, int line)
{
   char *number = NULL;
   int w, gutter, g;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_code_widget_text_left_gutter_width_get(widget);

   evas_object_textgrid_size_get(pd->grid, &w, NULL);

   cells[gutter-1].codepoint = status_icons[status];
   cells[gutter-1].bold = 1;
   cells[gutter-1].fg = ELM_CODE_WIDGET_COLOR_GUTTER_FG;
   cells[gutter-1].bg = (status == ELM_CODE_STATUS_TYPE_DEFAULT) ? ELM_CODE_WIDGET_COLOR_GUTTER_BG : status;

   if (pd->show_line_numbers)
     {
        if (line > 0)
          {
             number = malloc(sizeof(char) * gutter);
             snprintf(number, gutter, "%*d", gutter - 1, line);
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

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!pd->show_whitespace)
     return;

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
_elm_code_widget_fill_cursor(Elm_Code_Widget *widget, Elm_Code_Line *line, Evas_Textgrid_Cell *cells,
                             int gutter, int w)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->editable && pd->focussed && pd->cursor_line == line->number)
     {
        if (pd->cursor_col + gutter - 1 < (unsigned int) w)
          cells[pd->cursor_col + gutter - 1].bg = ELM_CODE_WIDGET_COLOR_CURSOR;
     }
}

static void
_elm_code_widget_fill_selection(Elm_Code_Widget *widget, Elm_Code_Line *line, Evas_Textgrid_Cell *cells,
                                int gutter, int w)
{
   Elm_Code_Widget_Data *pd;
   unsigned int x, start, end;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection)
     return;

   if (pd->selection->start_line > line->number || pd->selection->end_line < line->number)
     return;

   start = pd->selection->start_col;
   if (pd->selection->start_line < line->number)
     start = 1;
   end = pd->selection->end_col;
   if (pd->selection->end_line > line->number)
     end = w;

   for (x = gutter + start - 1; x < gutter + end && x < (unsigned int) w; x++)
     cells[x].bg = ELM_CODE_WIDGET_COLOR_SELECTION;
}

static void
_elm_code_widget_fill_line(Elm_Code_Widget *widget, Elm_Code_Line *line)
{
   char *chr;
   Eina_Unicode unichr;
   unsigned int length, x;
   int w, chrpos, gutter;
   Evas_Textgrid_Cell *cells;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_code_widget_text_left_gutter_width_get(widget);

   evas_object_textgrid_size_get(pd->grid, &w, NULL);
   cells = evas_object_textgrid_cellrow_get(pd->grid, line->number - 1);

   _elm_code_widget_fill_gutter(widget, cells, line->status, line->number);
   _elm_code_widget_fill_line_tokens(widget, cells, w, line);

   length = elm_code_line_utf8_length_get(line);
   chrpos = 0;
   chr = (char *)elm_code_line_text_get(line, NULL);

   for (x = gutter; x < (unsigned int) w && x < length + gutter; x++)
     {
        unichr = eina_unicode_utf8_next_get(chr, &chrpos);

        cells[x].codepoint = unichr;
        cells[x].bg = _elm_code_widget_status_type_get(widget, line->status, x - gutter + 1);

        _elm_code_widget_fill_whitespace(widget, unichr, &cells[x]);
     }
   for (; x < (unsigned int) w; x++)
     {
        cells[x].codepoint = 0;
        cells[x].bg = _elm_code_widget_status_type_get(widget, line->status, x - gutter + 1);
     }

   _elm_code_widget_fill_cursor(widget, line, cells, gutter, w);
   _elm_code_widget_fill_selection(widget, line, cells, gutter, w);
   if (line->number < elm_code_file_lines_get(line->file))
     _elm_code_widget_fill_whitespace(widget, '\n', &cells[length + gutter]);

   evas_object_textgrid_update_add(pd->grid, 0, line->number - 1, w, 1);
}

static void
_elm_code_widget_empty_line(Elm_Code_Widget *widget, unsigned int number)
{
   unsigned int x;
   int w, gutter;
   Evas_Textgrid_Cell *cells;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   gutter = elm_code_widget_text_left_gutter_width_get(widget);

   evas_object_textgrid_size_get(pd->grid, &w, NULL);
   cells = evas_object_textgrid_cellrow_get(pd->grid, number - 1);
   _elm_code_widget_fill_gutter(widget, cells, ELM_CODE_STATUS_TYPE_DEFAULT, 0);

   for (x = gutter; x < (unsigned int) w; x++)
     {
        cells[x].codepoint = 0;
        cells[x].bg = _elm_code_widget_status_type_get(widget, ELM_CODE_STATUS_TYPE_DEFAULT, x - gutter + 1);
     }

   evas_object_textgrid_update_add(pd->grid, 0, number - 1, w, 1);
}

static void
_elm_code_widget_fill(Elm_Code_Widget *widget)
{
   Elm_Code_Line *line;
   int w, h;
   unsigned int y;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   _elm_code_widget_resize(widget);
   evas_object_textgrid_size_get(pd->grid, &w, &h);

   for (y = 1; y <= (unsigned int) h && y <= elm_code_file_lines_get(pd->code->file); y++)
     {
        line = elm_code_file_line_get(pd->code->file, y);

        _elm_code_widget_fill_line(widget, line);
     }
   for (; y <= (unsigned int) h; y++)
     {
        _elm_code_widget_empty_line(widget, y);
     }
}

static Eina_Bool
_elm_code_widget_line_cb(void *data, Eo *obj EINA_UNUSED,
                         const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   // FIXME refresh just the row unless we have resized (by being the result of a row append)
   _elm_code_widget_fill(widget);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_elm_code_widget_file_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_fill(widget);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_elm_code_widget_selection_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_fill(widget);
   return EO_CALLBACK_CONTINUE;
}

static void
_elm_code_widget_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;

   _elm_code_widget_fill(widget);
}

static Eina_Bool
_elm_code_widget_cursor_key_will_move(Elm_Code_Widget *widget, const char *key)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   line = elm_code_file_line_get(pd->code->file, pd->cursor_line);

   if (!line)
     return EINA_FALSE;

   if (!strcmp(key, "Up"))
     return pd->cursor_line > 1;
   else if (!strcmp(key, "Down"))
     return pd->cursor_line < elm_code_file_lines_get(pd->code->file);
   else if (!strcmp(key, "Left"))
     return pd->cursor_col > 1;
   else if (!strcmp(key, "Right"))
     return pd->cursor_col < (unsigned int) line->unicode_length + 1;
   else
     return EINA_FALSE;
}

static void
_elm_code_widget_update_focus_directions(Elm_Code_Widget *obj)
{
   if (_elm_code_widget_cursor_key_will_move(obj, "Up"))
     elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_UP);
   else
     elm_widget_focus_next_object_set(obj, NULL, ELM_FOCUS_UP);

   if (_elm_code_widget_cursor_key_will_move(obj, "Down"))
     elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_DOWN);
   else
     elm_widget_focus_next_object_set(obj, NULL, ELM_FOCUS_DOWN);

   if (_elm_code_widget_cursor_key_will_move(obj, "Left"))
     elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_LEFT);
   else
     elm_widget_focus_next_object_set(obj, NULL, ELM_FOCUS_LEFT);

   if (_elm_code_widget_cursor_key_will_move(obj, "Right"))
     elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_RIGHT);
   else
     elm_widget_focus_next_object_set(obj, NULL, ELM_FOCUS_RIGHT);

   elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_PREVIOUS);
   elm_widget_focus_next_object_set(obj, obj, ELM_FOCUS_NEXT);
}

static void
_elm_code_widget_cursor_ensure_visible(Elm_Code_Widget *widget)
{
   Evas_Coord viewx, viewy, vieww, viewh, cellw, cellh;
   Evas_Coord curx, cury;
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   elm_scroller_region_get(pd->scroller, &viewx, &viewy, &vieww, &viewh);
   evas_object_textgrid_cell_size_get(pd->grid, &cellw, &cellh);

   curx = (pd->cursor_col + elm_code_widget_text_left_gutter_width_get(widget) - 1) * cellw;
   cury = (pd->cursor_line - 1) * cellh;

   if (curx >= viewx && cury >= viewy && curx + cellw <= viewx + vieww && cury + cellh <= viewy + viewh)
     return;

   elm_scroller_region_show(pd->scroller, curx, cury, cellw, cellh);
}

static void
_elm_code_widget_cursor_move(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd, unsigned int col, unsigned int line,
                             Eina_Bool was_key)
{
   Elm_Code *code;
   unsigned int oldrow;

   oldrow = pd->cursor_line;
   pd->cursor_col = col;
   pd->cursor_line = line;

   if (!was_key)
     _elm_code_widget_update_focus_directions(widget);

   eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET_EVENT_CURSOR_CHANGED, widget));
   _elm_code_widget_cursor_ensure_visible(widget);

   if (oldrow != pd->cursor_line)
     {
        code = pd->code;
        if (oldrow <= elm_code_file_lines_get(code->file))
          _elm_code_widget_fill_line(widget, elm_code_file_line_get(pd->code->file, oldrow));
        else
          _elm_code_widget_empty_line(widget, oldrow);
     }
   _elm_code_widget_fill_line(widget, elm_code_file_line_get(pd->code->file, pd->cursor_line));
}

static Eina_Bool
_elm_code_widget_position_at_coordinates_get(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd,
                                             Evas_Coord x, Evas_Coord y,
                                             unsigned int *row, unsigned int *col)
{
   Elm_Code_Line *line;
   Evas_Coord ox, oy, sx, sy;
   int cw, ch;

   evas_object_geometry_get(widget, &ox, &oy, NULL, NULL);
   elm_scroller_region_get(pd->scroller, &sx, &sy, NULL, NULL);
   x = x + sx - ox;
   y = y + sy - oy;

   evas_object_textgrid_cell_size_get(pd->grid, &cw, &ch);
   if (col)
     *col = ((double) x / cw) - elm_code_widget_text_left_gutter_width_get(widget) + 1;
   if (row)
     *row = ((double) y / ch) + 1;

   line = elm_code_file_line_get(pd->code->file, *row);
   return !!line;
}

static void
_elm_code_widget_clicked_editable_cb(Elm_Code_Widget *widget, Evas_Coord x, Evas_Coord y)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col;
   Eina_Bool hasline;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   hasline = _elm_code_widget_position_at_coordinates_get(widget, pd, x, y, &row, &col);
   if (!hasline)
     return;

   line = elm_code_file_line_get(pd->code->file, row);
   if (col > (unsigned int) line->unicode_length + 1)
     col = line->unicode_length + 1;
   else if (col == 0)
     col = 1;

   _elm_code_widget_cursor_move(widget, pd, col, row, EINA_FALSE);
}

static void
_elm_code_widget_clicked_readonly_cb(Elm_Code_Widget *widget, Evas_Coord x, Evas_Coord y)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row;
   Eina_Bool hasline;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   hasline = _elm_code_widget_position_at_coordinates_get(widget, pd, x, y, &row, NULL);
   if (!hasline)
     return;

   line = elm_code_file_line_get(pd->code->file, row);
   eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET_EVENT_LINE_CLICKED, line));
}

static void
_elm_code_widget_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                            void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Down *event;
   unsigned int row, col;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Down *)event_info;

   elm_code_widget_selection_clear(widget);
   if (!pd->editable)
     return;

   _elm_code_widget_position_at_coordinates_get(widget, pd, event->canvas.x, event->canvas.y, &row, &col);
   elm_code_widget_selection_start(widget, row, col);
}

static void
_elm_code_widget_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                            void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   Evas_Event_Mouse_Move *event;
   unsigned int row, col;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Move *)event_info;

   if (!pd->editable || !pd->selection || !event->buttons)
     return;

   _elm_code_widget_position_at_coordinates_get(widget, pd, event->cur.canvas.x, event->cur.canvas.y, &row, &col);

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

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   event = (Evas_Event_Mouse_Up *)event_info;

   if (pd->selection)
     {
        if (pd->selection->start_line == pd->selection->end_line &&
            pd->selection->start_col == pd->selection->end_col)
          elm_code_widget_selection_clear(widget);
        else
          return;
     }

   x = event->canvas.x;
   y = event->canvas.y;
   if (pd->editable)
     _elm_code_widget_clicked_editable_cb(widget, x, y);
   else
     _elm_code_widget_clicked_readonly_cb(widget, x, y);
}

static void
_elm_code_widget_cursor_move_up(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line <= 1)
     return;

   row--;
   line = elm_code_file_line_get(pd->code->file, row);
   if (col > (unsigned int) line->unicode_length + 1)
     col = line->unicode_length + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row, EINA_TRUE);
}

static void
_elm_code_widget_cursor_move_down(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int row, col;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   row = pd->cursor_line;
   col = pd->cursor_col;

   if (pd->cursor_line >= elm_code_file_lines_get(pd->code->file))
     return;

   row++;
   line = elm_code_file_line_get(pd->code->file, row);
   if (col > (unsigned int) line->unicode_length + 1)
     col = line->unicode_length + 1;

   _elm_code_widget_cursor_move(widget, pd, col, row, EINA_TRUE);
}

static void
_elm_code_widget_cursor_move_left(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->cursor_col <= 1)
     return;

   _elm_code_widget_cursor_move(widget, pd, pd->cursor_col-1, pd->cursor_line, EINA_TRUE);
}

static void
_elm_code_widget_cursor_move_right(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   line = elm_code_file_line_get(pd->code->file, pd->cursor_line);
   if (pd->cursor_col > (unsigned int) line->unicode_length)
     return;

   _elm_code_widget_cursor_move(widget, pd, pd->cursor_col+1, pd->cursor_line, EINA_TRUE);
}

static void
_elm_code_widget_text_at_cursor_insert(Elm_Code_Widget *widget, const char *text, int length)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   unsigned int row, col;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));
   line = elm_code_file_line_get(code->file, row);

   elm_code_line_text_insert(line, col, text, length);
   eo_do(widget,
         elm_code_widget_cursor_position_set(col + length, row));
}

static void
_elm_code_widget_newline(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line, *newline;
   unsigned int row, col, length;
   char *content;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));
   line = elm_code_file_line_get(code->file, row);

   content = (char *) elm_code_line_text_get(line, &length);
   content = strndup(content, length);
   elm_code_file_line_insert(code->file, line->number + 1, "", 0, NULL);
   newline = elm_code_file_line_get(code->file, line->number + 1);
// TODO we need to split tokens from these lines (move this to elm_code_line?)
   elm_code_line_text_set(newline, content + col - 1, length - col + 1);
   elm_code_line_text_set(line, content, col - 1);

   free(content);
   eo_do(widget,
         elm_code_widget_cursor_position_set(1, row + 1));
}

static void
_elm_code_widget_backspaceline(Elm_Code_Widget *widget, Eina_Bool nextline)
{
   Elm_Code *code;
   Elm_Code_Line *line, *otherline;
   unsigned int row, col;

   const char *text1, *text2;
   char *newtext;
   unsigned int length1, length2;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));
   line = elm_code_file_line_get(code->file, row);

   if (nextline)
     {
        otherline = elm_code_file_line_get(code->file, row + 1);
        text1 = elm_code_line_text_get(line, &length1);
        text2 = elm_code_line_text_get(otherline, &length2);
     }
   else
     {
        otherline = elm_code_file_line_get(code->file, row - 1);
        text1 = elm_code_line_text_get(otherline, &length1);
        text2 = elm_code_line_text_get(line, &length2);
     }

   newtext = malloc(sizeof(char) * (length1 + length2 + 1));
   snprintf(newtext, length1 + 1, "%s", text1);
   snprintf(newtext + length1, length2 + 1, "%s", text2);

// TODO we need to merge tokens from these lines (move this to elm_code_line?)
   elm_code_file_line_remove(code->file, otherline->number);
   elm_code_line_text_set(line, newtext, length1 + length2);

   free(newtext);
   if (!nextline)
     eo_do(widget,
           elm_code_widget_cursor_position_set(length1 + 1, row - 1));
}

static void
_elm_code_widget_backspace(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   unsigned int row, col;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));

   if (col <= 1)
     {
        if (row == 1)
          return;

        _elm_code_widget_backspaceline(widget, EINA_FALSE);
        line = elm_code_file_line_get(code->file, row - 1);

        return;
     }

   line = elm_code_file_line_get(code->file, row);

   elm_code_line_text_remove(line, col - 1, 1);
   eo_do(widget,
         elm_code_widget_cursor_position_set(col - 1, row));
}

static void
_elm_code_widget_delete(Elm_Code_Widget *widget)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   unsigned int row, col;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));
   line = elm_code_file_line_get(code->file, row);
   if (col > line->unicode_length)
     {
        if (row == elm_code_file_lines_get(code->file))
          return;

        _elm_code_widget_backspaceline(widget, EINA_TRUE);
        return;
     }

   elm_code_line_text_remove(line, col, 1);
   eo_do(widget,
         elm_code_widget_cursor_position_set(col, row));
}

static void
_elm_code_widget_key_down_cb(void *data, Evas *evas EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   Evas_Event_Key_Down *ev = event_info;

   if (!pd->editable)
     return;

   _elm_code_widget_update_focus_directions((Elm_Code_Widget *)obj);

   if (!strcmp(ev->key, "Up"))
     _elm_code_widget_cursor_move_up(widget);
   else if (!strcmp(ev->key, "Down"))
     _elm_code_widget_cursor_move_down(widget);
   else if (!strcmp(ev->key, "Left"))
     _elm_code_widget_cursor_move_left(widget);
   else if (!strcmp(ev->key, "Right"))
     _elm_code_widget_cursor_move_right(widget);

   else if (!strcmp(ev->key, "KP_Enter") || !strcmp(ev->key, "Return"))
     _elm_code_widget_newline(widget);
   else if (!strcmp(ev->key, "BackSpace"))
     _elm_code_widget_backspace(widget);
   else if (!strcmp(ev->key, "Delete"))
     _elm_code_widget_delete(widget);

   else if (ev->string && strlen(ev->string) == 1)
     _elm_code_widget_text_at_cursor_insert(widget, ev->string, 1);
   else
     INF("Unhandled key %s (%s) (%s)", ev->key, ev->keyname, ev->string);
}

static void
_elm_code_widget_focused_event_cb(void *data, Evas_Object *obj,
                                  void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->focussed = EINA_TRUE;

   _elm_code_widget_update_focus_directions(widget);
   _elm_code_widget_fill(obj);
}

static void
_elm_code_widget_unfocused_event_cb(void *data, Evas_Object *obj,
                                    void *event_info EINA_UNUSED)
{
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->focussed = EINA_FALSE;
   _elm_code_widget_fill(obj);
}

EOLIAN static Eina_Bool
_elm_code_widget_elm_widget_event(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd EINA_UNUSED,
                                  Evas_Object *src EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;

   if (!strcmp(ev->key, "BackSpace"))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_code_widget_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED,
                                                  Elm_Code_Widget_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_code_widget_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED,
                                                       Elm_Code_Widget_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOAPI void
_elm_code_widget_line_refresh(Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED, Elm_Code_Line *line)
{
   _elm_code_widget_fill_line(obj, line);
}

EOLIAN static void
_elm_code_widget_font_size_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, Evas_Font_Size font_size)
{
   evas_object_textgrid_font_set(pd->grid, "Mono", font_size * elm_config_scale_get());
   pd->font_size = font_size;
}

EOLIAN static Evas_Font_Size
_elm_code_widget_font_size_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->font_size;
}

EOLIAN static void
_elm_code_widget_code_set(Eo *obj, Elm_Code_Widget_Data *pd, Elm_Code *code)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   pd->code = code;

   code->widgets = eina_list_append(code->widgets, obj);
}

EOLIAN static Elm_Code *
_elm_code_widget_code_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
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
_elm_code_widget_gravity_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, double *x, double *y)
{
   *x = pd->gravity_x;
   *y = pd->gravity_y;
}

EOLIAN static void
_elm_code_widget_editable_set(Eo *obj, Elm_Code_Widget_Data *pd, Eina_Bool editable)
{
   pd->editable = editable;
   elm_object_focus_allow_set(obj, editable);
}

EOLIAN static Eina_Bool
_elm_code_widget_editable_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->editable;
}

EOLIAN static void
_elm_code_widget_line_numbers_set(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, Eina_Bool line_numbers)
{
   pd->show_line_numbers = line_numbers;
}

EOLIAN static Eina_Bool
_elm_code_widget_line_numbers_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
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
_elm_code_widget_line_width_marker_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
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
_elm_code_widget_show_whitespace_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return pd->show_whitespace;
}

EOLIAN static void
_elm_code_widget_cursor_position_set(Eo *obj, Elm_Code_Widget_Data *pd, unsigned int col, unsigned int line)
{
   _elm_code_widget_cursor_move(obj, pd, col, line, EINA_FALSE);
}

EOLIAN static void
_elm_code_widget_cursor_position_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, unsigned int *col, unsigned int *line)
{
   *col = pd->cursor_col;
   *line = pd->cursor_line;
}

static void
_elm_code_widget_setup_palette(Evas_Object *o)
{
   double feint = 0.5;

   // setup status colors
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_DEFAULT,
                                    36, 36, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_IGNORED,
                                    36, 36, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_NOTE,
                                    255, 153, 0, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_WARNING,
                                    255, 153, 0, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ERROR,
                                    205, 54, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_FATAL,
                                    205, 54, 54, 255);

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ADDED,
                                    36, 96, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_REMOVED,
                                    96, 36, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_CHANGED,
                                    36, 36, 96, 255);

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_PASSED,
                                    54, 96, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_FAILED,
                                    96, 54, 54, 255);

   // setup token colors
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_DEFAULT,
                                    205, 205, 205, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_COMMENT,
                                    51, 153, 255, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_STRING,
                                    255, 90, 53, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_NUMBER,
                                    212, 212, 42, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_BRACE,
                                    101, 101, 101, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_TYPE,
                                    51, 153, 255, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_CLASS,
                                    114, 170, 212, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_FUNCTION,
                                    114, 170, 212, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_PARAM,
                                    255, 255, 255, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_KEYWORD,
                                    255, 153, 0, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_PREPROCESSOR,
                                    0, 176, 0, 255);


   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_ADDED,
                                    54, 255, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_REMOVED,
                                    255, 54, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_CHANGED,
                                    54, 54, 255, 255);

   // other styles that the widget uses
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_WIDGET_COLOR_CURSOR,
                                    205, 205, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_WIDGET_COLOR_SELECTION,
                                    51, 153, 255, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_WIDGET_COLOR_GUTTER_BG,
                                    75, 75, 75, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_WIDGET_COLOR_GUTTER_FG,
                                    139, 139, 139, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_WIDGET_COLOR_WHITESPACE,
                                    101 * feint, 101 * feint, 101 * feint, 255 * feint);
}

EOLIAN static void
_elm_code_widget_evas_object_smart_add(Eo *obj, Elm_Code_Widget_Data *pd)
{
   Evas_Object *grid, *scroller;

   eo_do_super(obj, ELM_CODE_WIDGET_CLASS, evas_obj_smart_add());
   elm_object_focus_allow_set(obj, EINA_TRUE);

   elm_layout_file_set(obj, PACKAGE_DATA_DIR "/themes/elm_code.edj", "elm_code/layout/default");

   scroller = elm_scroller_add(obj);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(scroller);
   elm_layout_content_set(obj, "elm.swallow.content", scroller);
   elm_object_focus_allow_set(scroller, EINA_FALSE);
   pd->scroller = scroller;

   grid = evas_object_textgrid_add(obj); 
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(grid);
   elm_object_content_set(scroller, grid);
   pd->grid = grid;
   _elm_code_widget_setup_palette(grid);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _elm_code_widget_resize_cb, obj);
   evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_DOWN, _elm_code_widget_mouse_down_cb, obj);
   evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_MOVE, _elm_code_widget_mouse_move_cb, obj);
   evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_UP, _elm_code_widget_mouse_up_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _elm_code_widget_key_down_cb, obj);

   evas_object_smart_callback_add(obj, "focused", _elm_code_widget_focused_event_cb, obj);
   evas_object_smart_callback_add(obj, "unfocused", _elm_code_widget_unfocused_event_cb, obj);

   eo_do(obj,
         eo_event_callback_add(&ELM_CODE_EVENT_LINE_LOAD_DONE, _elm_code_widget_line_cb, obj),
         eo_event_callback_add(&ELM_CODE_EVENT_FILE_LOAD_DONE, _elm_code_widget_file_cb, obj),
         eo_event_callback_add(ELM_CODE_WIDGET_EVENT_SELECTION_CHANGED, _elm_code_widget_selection_cb, obj),
         eo_event_callback_add(ELM_CODE_WIDGET_EVENT_SELECTION_CLEARED, _elm_code_widget_selection_cb, obj));

   _elm_code_widget_font_size_set(obj, pd, 10);
}

#include "elm_code_widget.eo.c"
