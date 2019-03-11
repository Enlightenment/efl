#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#include "elm_code_widget_private.h"

static int
_elm_code_widget_text_line_number_width_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   int max;

   max = elm_code_file_lines_get(pd->code->file);

   // leave space for 2 digits minimum
   if (max < 10)
     max = 10;

   return floor(log10(max)) + 1;
}

static int
_elm_code_widget_text_left_gutter_width_get(Eo *obj, Elm_Code_Widget_Data *pd)
{
   Elm_Code_Widget *widget;
   int width = 1; // the status icon, for now

   widget = obj;
   if (!widget)
     return width;

   if (pd->show_line_numbers)
     width += _elm_code_widget_text_line_number_width_get(widget, pd);

   return width;
}

static char *
_elm_code_widget_text_multi_get(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd,
                                unsigned int start_line, unsigned int start_col,
                                unsigned int end_line, unsigned int end_col)
{
   Elm_Code_Line *line;
   char *first, *last, *ret, *ptr;
   const char *newline;
   short newline_len;
   int ret_len;
   unsigned int row, start, end;

   newline = elm_code_file_line_ending_chars_get(pd->code->file, &newline_len);

   line = elm_code_file_line_get(pd->code->file, start_line);
   start = elm_code_widget_line_text_position_for_column_get(widget, line, start_col);
   first = elm_code_line_text_substr(line, start, line->length - start);

   line = elm_code_file_line_get(pd->code->file, end_line);
   end = elm_code_widget_line_text_position_for_column_get(widget, line, end_col + 1);
   last = elm_code_line_text_substr(line, 0, end);

   ret_len = strlen(first) + strlen(last) + newline_len;

   for (row = start_line + 1; row < end_line; row++)
     {
        line = elm_code_file_line_get(pd->code->file, row);
        ret_len += line->length + newline_len;
     }

   ret = malloc(sizeof(char) * (ret_len + 1));
   if (!ret) goto end;

   snprintf(ret, strlen(first) + newline_len + 1, "%s%s", first, newline);

   ptr = ret;
   ptr += strlen(first) + newline_len;

   for (row = start_line + 1; row < end_line; row++)
     {
        line = elm_code_file_line_get(pd->code->file, row);
        if (line->length > 0)
          snprintf(ptr, line->length + 1, "%s", elm_code_line_text_get(line, NULL));

        snprintf(ptr + line->length, newline_len + 1, "%s", newline);
        ptr += line->length + newline_len;
     }
   snprintf(ptr, strlen(last) + 1, "%s", last);

end:
   free(first);
   free(last);
   return ret;
}

static char *
_elm_code_widget_text_single_get(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd,
                                           unsigned int start_line, unsigned int start_col,
                                           unsigned int end_col)
{
   Elm_Code_Line *line;
   unsigned int start, end;

   line = elm_code_file_line_get(pd->code->file, start_line);
   start = elm_code_widget_line_text_position_for_column_get(widget, line, start_col);
   end = elm_code_widget_line_text_position_for_column_get(widget, line, end_col + 1);

   return elm_code_line_text_substr(line, start, end - start);
}

static char *
_elm_code_widget_text_between_positions_get(Eo *widget, Elm_Code_Widget_Data *pd,
                                            unsigned int start_line, unsigned int start_col,
                                            unsigned int end_line, unsigned int end_col)
{
   if (start_line == end_line)
     return _elm_code_widget_text_single_get(widget, pd, start_line, start_col, end_col);
   else
     return _elm_code_widget_text_multi_get(widget, pd, start_line, start_col, end_line, end_col);
}

static unsigned int
_elm_code_widget_line_text_column_width_to_position(Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED, Elm_Code_Line *line, unsigned int position)
{
   Eina_Unicode unicode;
   unsigned int count = 1;
   int index = 0;
   const char *chars;

   if (line->length == 0)
     return 1;

   if (line->modified)
     chars = line->modified;
   else
     chars = line->content;
   if (position > line->length)
     position = line->length;

   while ((unsigned int) index < position)
     {
        unicode = eina_unicode_utf8_next_get(chars, &index);
        if (unicode == 0)
          break;

        if (unicode == '\t')
          count += elm_code_widget_text_tabwidth_at_column_get(obj, count);
        else
          count++;
     }

   return count;
}

static unsigned int
_elm_code_widget_line_text_column_width_get(Eo *obj, Elm_Code_Widget_Data *pd, Elm_Code_Line *line)
{
   if (!line)
     return 0;

   return _elm_code_widget_line_text_column_width_to_position(obj, pd, line, line->length) - 1;
}

static unsigned int
_elm_code_widget_line_text_position_for_column_get(Eo *obj, Elm_Code_Widget_Data *pd EINA_UNUSED, Elm_Code_Line *line, unsigned int column)
{
   Eina_Unicode unicode;
   unsigned int count = 1, position = 0;
   int index = 0;
   const char *chars;

   if (!line || line->length == 0 || column == 1)
     return 0;

   if (line->modified)
     chars = line->modified;
   else
     chars = line->content;

   while ((unsigned int) count <= column && index <= (int) line->length)
     {
        position = (unsigned int) index;
        if (index < (int) line->length)
          unicode = eina_unicode_utf8_next_get(chars, &index);
        else return line->length;

        if (unicode == 0)
          return line->length;
        else if (unicode == '\t')
          count += elm_code_widget_text_tabwidth_at_column_get(obj, count);
        else
          count++;
     }

   return position;
}

static unsigned int
_elm_code_widget_text_tabwidth_at_column_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd, unsigned int column)
{
   return pd->tabstop - ((column - 1) % pd->tabstop);
}

static void
_elm_code_widget_text_insert_single(Elm_Code_Widget *widget, Elm_Code *code,
                                    unsigned int col, unsigned int row, const char *text, unsigned int len)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Line *line;
   unsigned int position, newcol;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   line = elm_code_file_line_get(code->file, row);
   position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
   elm_code_line_text_insert(line, position, text, len);

   newcol = elm_code_widget_line_text_column_width_to_position(widget, line, position + len);

   // if we are making a line longer than before then we need to resize
   if (newcol > pd->col_count)
     _elm_code_widget_resize(widget, line);

   efl_ui_code_widget_cursor_position_set(widget, row, newcol);
}

static void
_elm_code_widget_text_insert_multi(Elm_Code_Widget *widget, Elm_Code *code,
                                   unsigned int col, unsigned int row, const char *text, unsigned int len)
{
   Elm_Code_Line *line;
   unsigned int position, newrow, remain;
   int nlpos;
   short nllen;
   char *ptr;

   line = elm_code_file_line_get(code->file, row);
   position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
   elm_code_line_split_at(line, position);

   newrow = row;
   ptr = (char *)text;
   remain = len;
   while ((nlpos = elm_code_text_newlinenpos(ptr, remain, &nllen)) != ELM_CODE_TEXT_NOT_FOUND)
     {
        if (newrow == row)
          _elm_code_widget_text_insert_single(widget, code, col, row, text, nlpos);
        else
          elm_code_file_line_insert(code->file, newrow, ptr, nlpos, NULL);

        remain -= nlpos + nllen;
        ptr += nlpos + nllen;
        newrow++;
     }

   _elm_code_widget_text_insert_single(widget, code, 1, newrow, ptr, len - (ptr - text));
}

void
_elm_code_widget_text_at_cursor_insert_do(Elm_Code_Widget *widget, const char *text, int length, Eina_Bool undo)
{
   Elm_Code *code;
   Elm_Code_Line *line;
   Elm_Code_Widget_Change_Info *change;
   unsigned int row, col, end_row, end_col, curlen, indent;
   const char *curtext, *indent_text;

   if (undo)
     elm_code_widget_selection_delete(widget);

   code = efl_ui_code_widget_code_get(widget);
   efl_ui_code_widget_cursor_position_get(widget, &row, &col);
   line = elm_code_file_line_get(code->file, row);
   if (line == NULL)
     {
        elm_code_file_line_append(code->file, "", 0, NULL);
        row = elm_code_file_lines_get(code->file);
        line = elm_code_file_line_get(code->file, row);
     }
   if (text[0] == '}')
     {
        curtext = elm_code_line_text_get(line, &curlen);

        if (elm_code_text_is_whitespace(curtext, line->length))
          {
             indent_text = elm_code_line_indent_matching_braces_get(line, &indent);
             elm_code_line_text_leading_whitespace_strip(line);

             if (indent > 0)
               elm_code_line_text_insert(line, 0, indent_text, indent);

             col = elm_code_widget_line_text_column_width_to_position(widget, line, indent + 1);
             efl_ui_code_widget_cursor_position_set(widget, row, col);
          }
     }

   if (elm_code_text_newlinenpos(text, length, NULL) == ELM_CODE_TEXT_NOT_FOUND)
     _elm_code_widget_text_insert_single(widget, code, col, row, text, length);
   else
     _elm_code_widget_text_insert_multi(widget, code, col, row, text, length);
   efl_ui_code_widget_cursor_position_get(widget, &end_row, &end_col);

   efl_event_callback_legacy_call(widget, EFL_UI_CODE_WIDGET_EVENT_CHANGED_USER, NULL);

   if (undo)
     {
        change = _elm_code_widget_change_create(col, row, end_col - 1, end_row, text, length, EINA_TRUE);
        _elm_code_widget_undo_change_add(widget, change);
        _elm_code_widget_change_free(change);
     }
}

EOLIAN void
_elm_code_widget_text_at_cursor_insert(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd EINA_UNUSED, const char *text)
{
   _elm_code_widget_text_at_cursor_insert_do(widget, text, strlen(text), EINA_TRUE);
}

void
_elm_code_widget_text_at_cursor_insert_no_undo(Elm_Code_Widget *widget, const char *text, unsigned int length)
{
   _elm_code_widget_text_at_cursor_insert_do(widget, text, length, EINA_FALSE);
}
