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
