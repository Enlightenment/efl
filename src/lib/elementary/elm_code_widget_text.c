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
   if (max < 1)
     max = 1;

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
                                unsigned int start_col, unsigned int start_line,
                                unsigned int end_col, unsigned int end_line)
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

   snprintf(ret, strlen(first) + newline_len + 1, "%s%s", first, newline);

   ptr = ret;
   ptr += strlen(first) + newline_len;

   for (row = start_line + 1; row < end_line; row++)
     {
        line = elm_code_file_line_get(pd->code->file, row);
        snprintf(ptr, line->length + 1, "%s",
                 elm_code_line_text_get(line, NULL));

        snprintf(ptr + line->length, newline_len + 1, "%s", newline);
        ptr += line->length + newline_len;
     }
   snprintf(ptr, strlen(last) + 1, "%s", last);

   free(first);
   free(last);
   return ret;
}

static char *
_elm_code_widget_text_single_get(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd,
                                           unsigned int start_col, unsigned int start_line,
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
                                            unsigned int start_col, unsigned int start_line,
                                            unsigned int end_col, unsigned int end_line)
{
   if (start_line == end_line)
     return _elm_code_widget_text_single_get(widget, pd, start_col, start_line, end_col);
   else
     return _elm_code_widget_text_multi_get(widget, pd, start_col, start_line, end_col, end_line);
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
        unicode = eina_unicode_utf8_next_get(chars, &index);

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

