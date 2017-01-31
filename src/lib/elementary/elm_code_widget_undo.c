#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#include "elm_code_widget_private.h"

Elm_Code_Widget_Change_Info *
_elm_code_widget_undo_info_copy(Elm_Code_Widget_Change_Info *info)
{
   Elm_Code_Widget_Change_Info *copy;

   copy = calloc(1, sizeof(*info));
   memcpy(copy, info, sizeof(*info));
   copy->content = malloc(sizeof(char) * (info->length + 1));
   strncpy(copy->content, info->content, info->length);

   return copy;
}

void
_elm_code_widget_undo_change_add(Evas_Object *widget,
                                 Elm_Code_Widget_Change_Info *info)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Change_Info *info_copy;

   info_copy = _elm_code_widget_undo_info_copy(info);
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   pd->undo_stack_ptr = eina_list_prepend(pd->undo_stack_ptr, info_copy);
   pd->undo_stack = pd->undo_stack_ptr;
}

static void
_elm_code_widget_undo_change(Evas_Object *widget,
                             Elm_Code_Widget_Change_Info *info)
{
   Elm_Code_Widget_Data *pd;
   unsigned int textlen, position, row, col;
   short nllen;
   Elm_Code_Line *line;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (info->insert)
     {
        elm_code_widget_selection_start(widget, info->start_line, info->start_col);
        elm_code_widget_selection_end(widget, info->end_line, info->end_col);
        elm_code_widget_selection_delete(widget);
     }
   else
     {
        elm_code_widget_selection_clear(widget);
        elm_code_widget_cursor_position_set(widget, info->start_line, info->start_col);
        unsigned int newrow = info->start_line;
        while (newrow <= info->end_line)
          {
             line = elm_code_file_line_get(pd->code->file, newrow);
             if (newrow != info->end_line)
               {
                  textlen = info->length;
                  textlen = elm_code_text_newlinenpos(info->content, info->length, &nllen);
                  info->length -= textlen + nllen;
                  _elm_code_widget_text_at_cursor_insert_no_undo(widget,
                                                                 info->content,
                                                                 textlen);
                  elm_obj_code_widget_cursor_position_get(widget, &row, &col);
                  position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
                  elm_code_line_split_at(line, position);
                  elm_code_widget_cursor_position_set(widget, newrow + 1, 1);
                  info->content += textlen + nllen;
               }
             else
               {
                  _elm_code_widget_text_at_cursor_insert_no_undo(widget,
                                                                 info->content,
                                                                 info->length);
               }
             newrow++;
          }
        if (info->end_col < 1)
          info->end_col = 1;
        elm_code_widget_cursor_position_set(widget, info->end_line, info->end_col);
     }
}

static void
_elm_code_widget_undo(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   Elm_Code_Widget_Change_Info *info;

   if (!pd->undo_stack_ptr)
     return;

   info = eina_list_data_get(pd->undo_stack_ptr);
   _elm_code_widget_undo_change(obj, info);

   if (eina_list_next(pd->undo_stack_ptr))
     pd->undo_stack_ptr = eina_list_next(pd->undo_stack_ptr);
   else
     pd->undo_stack_ptr = NULL;
}

