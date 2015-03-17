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

EAPI void
elm_code_widget_selection_start(Evas_Object *widget,
                                unsigned int line, unsigned int col)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Selection_Data *selection;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection)
     {
        selection = _elm_code_widget_selection_new();

        selection->end_line = line;
        selection->end_col = col;

        pd->selection = selection;
     }

   pd->selection->start_line = line;
   pd->selection->start_col = col;
}

EAPI void
elm_code_widget_selection_end(Evas_Object *widget,
                              unsigned int line, unsigned int col)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Selection_Data *selection;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection)
     {
        selection = _elm_code_widget_selection_new();

        selection->start_line = line;
        selection->start_col = col;

        pd->selection = selection;
     }

   pd->selection->end_line = line;
   pd->selection->end_col = col;
}

EAPI void
elm_code_widget_selection_clear(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (pd->selection)
     free(pd->selection);

   pd->selection = NULL;
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
