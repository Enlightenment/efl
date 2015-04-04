#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_widget_private.h"

EAPI int
elm_code_widget_text_line_number_width_get(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   int max;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   max = elm_code_file_lines_get(pd->code->file);
   if (max < 1)
     max = 1;

   return floor(log10(max)) + 1;
}

EAPI int
elm_code_widget_text_left_gutter_width_get(Elm_Code_Widget *widget)
{
   Elm_Code_Widget_Data *pd;
   int width = 1; // the status icon, for now

   if (!widget)
     return width;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->show_line_numbers)
     width += elm_code_widget_text_line_number_width_get(widget);

   return width;
}
