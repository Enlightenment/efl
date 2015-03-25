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
   eo_do(widget,
         eo_event_callback_call(ELM_CODE_WIDGET_EVENT_SELECTION_CHANGED, widget),
         elm_code_widget_cursor_position_set(col, line));
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

static char *
_elm_code_widget_selection_text_single_get(Elm_Code_Widget_Data *pd)
{
   Elm_Code_Line *line;

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);

   return elm_code_line_text_substr(line, pd->selection->start_col - 1,
                                    pd->selection->end_col - pd->selection->start_col + 1);
}

static char *
_elm_code_widget_selection_text_multi_get(Elm_Code_Widget_Data *pd)
{
   Elm_Code_Line *line;
   char *first, *last, *ret;
   const char *newline;
   short newline_len;
   int ret_len;

   newline = elm_code_file_line_ending_chars_get(pd->code->file, &newline_len);

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);
   first = elm_code_line_text_substr(line, pd->selection->start_col - 1,
                                     line->length - pd->selection->start_col + 1);

   line = elm_code_file_line_get(pd->code->file, pd->selection->end_line);
   last = elm_code_line_text_substr(line, 0, pd->selection->end_col + 1);

   ret_len = strlen(first) + strlen(last) + newline_len;
   ret = malloc(sizeof(char) * (ret_len + 1));
   snprintf(ret, ret_len, "%s%s%s", first, newline, last);

   free(first);
   free(last);
   return ret;
}

EAPI char *
elm_code_widget_selection_text_get(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection || pd->selection->end_line < pd->selection->start_line)
     return strdup("");

   if (pd->selection->start_line == pd->selection->end_line)
     return _elm_code_widget_selection_text_single_get(pd);
   else
     return _elm_code_widget_selection_text_multi_get(pd);

   return strdup("TODO");
}
