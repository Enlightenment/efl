#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_widget_private.h"

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
   unsigned int width;

   file = pd->code->file;

   if (*row > elm_code_file_lines_get(file))
     *row = elm_code_file_lines_get(file);

   line = elm_code_file_line_get(file, *row);
   width = elm_code_line_text_column_width(line, pd->tabstop);

   if (*col > width + 1)
     *col = width + 1;
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

static void
_elm_code_widget_selection_delete_single(Elm_Code_Widget_Data *pd)
{
   Elm_Code_Line *line;
   const char *old;
   unsigned int old_length, length;
   char *content;

   if (pd->selection->end_col < pd->selection->start_col)
     return;

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);
   old = elm_code_line_text_get(line, &old_length);
   length = line->length - (pd->selection->end_col - pd->selection->start_col + 1);
   content = malloc(sizeof(char) * length);

   strncpy(content, old, pd->selection->start_col - 1);
   strncpy(content + pd->selection->start_col - 1, old + pd->selection->end_col,
           old_length - pd->selection->end_col);
   elm_code_line_text_set(line, content, length);
   free(content);
}

static void
_elm_code_widget_selection_delete_multi(Elm_Code_Widget_Data *pd)
{
   Elm_Code_Line *line;
   const char *first, *last;
   unsigned int last_length, length, i;
   char *content;

   if (pd->selection->end_line <= pd->selection->start_line)
     return;

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);
   first = elm_code_line_text_get(line, NULL);
   line = elm_code_file_line_get(pd->code->file, pd->selection->end_line);
   last = elm_code_line_text_get(line, &last_length);
   length = pd->selection->start_col + last_length - pd->selection->end_col + 1;
   content = malloc(sizeof(char) * length);
   strncpy(content, first, pd->selection->start_col - 1);
   strncpy(content + pd->selection->start_col - 1, last + pd->selection->end_col,
           last_length - pd->selection->end_col);

   for (i = line->number; i > pd->selection->start_line; i--)
     elm_code_file_line_remove(pd->code->file, i);

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);
   elm_code_line_text_set(line, content, length);
   free(content);
}

EAPI void
elm_code_widget_selection_delete(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;

   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (!pd->selection)
     return;

   if (pd->selection->start_line == pd->selection->end_line)
     _elm_code_widget_selection_delete_single(pd);
   else
     _elm_code_widget_selection_delete_multi(pd);

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
   char *first, *last, *ret, *ptr;
   const char *newline;
   short newline_len;
   int ret_len;
   unsigned int row;

   newline = elm_code_file_line_ending_chars_get(pd->code->file, &newline_len);

   line = elm_code_file_line_get(pd->code->file, pd->selection->start_line);
   first = elm_code_line_text_substr(line, pd->selection->start_col - 1,
                                     line->length - pd->selection->start_col + 1);

   line = elm_code_file_line_get(pd->code->file, pd->selection->end_line);
   last = elm_code_line_text_substr(line, 0, pd->selection->end_col);

   ret_len = strlen(first) + strlen(last) + newline_len;

   for (row = pd->selection->start_line + 1; row < pd->selection->end_line; row++)
     {
        line = elm_code_file_line_get(pd->code->file, row);
        ret_len += line->length + newline_len;
     }

   ret = malloc(sizeof(char) * (ret_len + 1));
   snprintf(ret, strlen(first) + newline_len + 1, "%s%s", first, newline);

   ptr = ret;
   ptr += strlen(first) + newline_len;

   for (row = pd->selection->start_line + 1; row < pd->selection->end_line; row++)
     {
        line = elm_code_file_line_get(pd->code->file, row);
        if (line->modified)
          snprintf(ptr, line->length + 1, "%s", line->modified);
        else
          snprintf(ptr, line->length + 1, "%s", line->content);

        snprintf(ptr + line->length, newline_len + 1, "%s", newline);
        ptr += line->length + newline_len;
     }
   snprintf(ptr, strlen(last) + 1, "%s", last);

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
}

static void
_selection_loss_cb(void *data, Elm_Sel_Type selection EINA_UNUSED)
{
   Elm_Code_Widget *widget;

   widget = (Elm_Code_Widget *)data;
// TODO we need to know whih selection we are clearing!
//   elm_code_widget_selection_clear(widget);
}

EAPI void
elm_code_widget_selection_cut(Evas_Object *widget)
{
   char *text;

   text = elm_code_widget_selection_text_get(widget);
   elm_cnp_selection_set(widget, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, text, strlen(text));
   elm_cnp_selection_loss_callback_set(widget, ELM_SEL_TYPE_CLIPBOARD, _selection_loss_cb, widget);
   free(text);

   elm_code_widget_selection_delete(widget);
}

EAPI void
elm_code_widget_selection_copy(Evas_Object *widget)
{
   char *text;

   text = elm_code_widget_selection_text_get(widget);
   elm_cnp_selection_set(widget, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, text, strlen(text));
   elm_cnp_selection_loss_callback_set(widget, ELM_SEL_TYPE_CLIPBOARD, _selection_loss_cb, widget);
   free(text);
}

static void
_selection_paste_single(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd, Elm_Code *code,
                        unsigned int col, unsigned int row, const char *text, unsigned int len)
{
   Elm_Code_Line *line;
   unsigned int position, newcol;

   line = elm_code_file_line_get(code->file, row);
   position = elm_code_line_text_position_for_column_get(line, col - 1, pd->tabstop);
   elm_code_line_text_insert(line, position + 1, text, len);

   newcol = elm_code_line_text_column_width_to_position(line, position + len, pd->tabstop);
   eo_do(widget,
         elm_code_widget_cursor_position_set(newcol + 1, row));
}

static void
_selection_paste_multi(Elm_Code_Widget *widget, Elm_Code_Widget_Data *pd, Elm_Code *code,
                       unsigned int col, unsigned int row, const char *text, unsigned int len)
{
   Elm_Code_Line *line;
   unsigned int position, newrow, remain;
   int nlpos;
   short nllen;
   char *ptr;

   line = elm_code_file_line_get(code->file, row);
   position = elm_code_line_text_position_for_column_get(line, col - 1, pd->tabstop);
   elm_code_line_split_at(line, position);

   newrow = row;
   ptr = (char *)text;
   remain = len;
   while ((nlpos = elm_code_text_newlinenpos(ptr, remain, &nllen)) != ELM_CODE_TEXT_NOT_FOUND)
     {
        if (newrow == row)
          _selection_paste_single(widget, pd, code, col, row, text, nlpos);
        else
          elm_code_file_line_insert(code->file, newrow, ptr, nlpos, NULL);

        remain -= nlpos + nllen;
        ptr += nlpos + nllen;
        newrow++;
     }

   _selection_paste_single(widget, pd, code, 1, newrow, ptr, len - (ptr - text));
}

static Eina_Bool
_selection_paste_cb(void *data, Evas_Object *obj EINA_UNUSED, Elm_Selection_Data *ev)
{
   Elm_Code *code;
   Elm_Code_Widget *widget;
   Elm_Code_Widget_Data *pd;
   unsigned int row, col;

   widget = (Elm_Code_Widget *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (ev->format != ELM_SEL_FORMAT_TEXT)
     return EINA_TRUE;

   if (ev->len <= 0)
     return EINA_TRUE;

   eo_do(widget,
         code = elm_code_widget_code_get(),
         elm_code_widget_cursor_position_get(&col, &row));

   if (elm_code_text_newlinenpos(ev->data, ev->len, NULL) == ELM_CODE_TEXT_NOT_FOUND)
     _selection_paste_single(widget, pd, code, col, row, ev->data, ev->len - 1);
   else
     _selection_paste_multi(widget, pd, code, col, row, ev->data, ev->len - 1);

   return EINA_TRUE;
}

EAPI void
elm_code_widget_selection_paste(Evas_Object *widget)
{
   elm_code_widget_selection_delete(widget);

   elm_cnp_selection_get(widget, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, _selection_paste_cb, widget);
}
