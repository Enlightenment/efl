#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

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

   if (line->length == 0 || column == 1)
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

