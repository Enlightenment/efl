#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

static Elm_Code_Widget_Selection_Data *
_elm_code_widget_selection_new()
{
   Elm_Code_Widget_Selection_Data *data;

   data = calloc(1, sizeof(Elm_Code_Widget_Selection_Data));

   return data;
}

static void
_elm_code_widget_selection_limit(Evas_Object *widget EINA_UNUSED, Elm_Code_Widget_Data *pd,
                                 unsigned int *row, unsigned int *col)
{
   Elm_Code_Line *line;
   Elm_Code_File *file;

   file = pd->code->file;

   if (*row > elm_code_file_lines_get(file))
     *row = elm_code_file_lines_get(file);

   line = elm_code_file_line_get(file, *row);

   if (*col > line->unicode_length + 1)
     *col = line->unicode_length + 1;
   if (*col < 1)
     *col = 1;
}

EAPI void
elm_code_widget_selection_start(Evas_Object *widget,
                                unsigned int line, unsigned int col)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Selection_Data *selection;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   _elm_code_widget_selection_limit(widget, pd, &line, &col);
   if (!pd->selection)
     {
        selection = _elm_code_widget_selection_new();

        selection->end_line = line;
        selection->end_col = col;

        pd->selection = selection;
     }

   pd->selection->start_line = line;
   pd->selection->start_col = col;
   eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET_EVENT_SELECTION_CHANGED, widget));
}

EAPI void
elm_code_widget_selection_end(Evas_Object *widget,
                              unsigned int line, unsigned int col)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Selection_Data *selection;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   _elm_code_widget_selection_limit(widget, pd, &line, &col);
   if (!pd->selection)
     {
        selection = _elm_code_widget_selection_new();

        selection->start_line = line;
        selection->start_col = col;

        pd->selection = selection;
     }

   pd->selection->end_line = line;
   pd->selection->end_col = col;
   eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET_EVENT_SELECTION_CHANGED, widget));
}

EAPI void
elm_code_widget_selection_clear(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->selection)
     free(pd->selection);

   pd->selection = NULL;
   eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET_EVENT_SELECTION_CLEARED, widget));
}

EAPI const char *
elm_code_widget_selection_text_get(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection)
     return "";

   return "TODO";
}
