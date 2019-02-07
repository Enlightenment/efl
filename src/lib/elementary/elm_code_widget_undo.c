#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#include "elm_code_widget_private.h"

static void
_elm_code_widget_undo_prev_clear(Evas_Object *widget)
{
   Elm_Code_Widget_Data *pd;
   Elm_Code_Widget_Change_Info *info;
   Eina_List *list;

   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);
   if (!pd->undo_stack_ptr)
     return;

   for (list = eina_list_prev(pd->undo_stack_ptr); list; list = eina_list_prev(list))
     {
        info = eina_list_data_get(list);
        free(info->content);
        free(info);
     }
}

Elm_Code_Widget_Change_Info *
_elm_code_widget_undo_info_copy(Elm_Code_Widget_Change_Info *info)
{
   Elm_Code_Widget_Change_Info *copy;

   copy = calloc(1, sizeof(*info));
   if (!copy) return NULL;
   memcpy(copy, info, sizeof(*info));
   copy->content = strndup(info->content, info->length);

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

   _elm_code_widget_undo_prev_clear(widget);

   pd->undo_stack_ptr = eina_list_prepend(pd->undo_stack_ptr, info_copy);
   pd->undo_stack = pd->undo_stack_ptr;
}

static void
_elm_code_widget_undo_change(Evas_Object *widget,
                             Elm_Code_Widget_Change_Info *info)
{
   Elm_Code_Widget_Data *pd;
   unsigned int textlen, position, row, col, newrow, remainlen;
   short nllen;
   char *content;
   Elm_Code_Line *line;
   pd = efl_data_scope_get(widget, ELM_CODE_WIDGET_CLASS);

   if (info->insert)
     {
        elm_code_widget_selection_start(widget, info->start_line, info->start_col);
        elm_code_widget_selection_end(widget, info->end_line, info->end_col);
        _elm_code_widget_selection_delete_no_undo(widget);
     }
   else
     {
        newrow = info->start_line;
        content = info->content;
        remainlen = info->length;
        elm_code_widget_selection_clear(widget);
        elm_code_widget_cursor_position_set(widget, info->start_line,
                                            info->start_col);
        while (newrow <= info->end_line)
          {
             line = elm_code_file_line_get(pd->code->file, newrow);
             if (newrow != info->end_line)
               {
                  textlen = elm_code_text_newlinenpos(content, remainlen,
                                                      &nllen);
                  remainlen -= textlen + nllen;
                  _elm_code_widget_text_at_cursor_insert_no_undo(widget,
                                                                 content,
                                                                 textlen);
                  elm_obj_code_widget_cursor_position_get(widget, &row, &col);
                  position = elm_code_widget_line_text_position_for_column_get(widget, line, col);
                  elm_code_line_split_at(line, position);
                  elm_code_widget_cursor_position_set(widget, newrow + 1, 1);
                  content += textlen + nllen;
               }
             else
               {
                  _elm_code_widget_text_at_cursor_insert_no_undo(widget,
                                                                 content,
                                                                 strlen(content));
               }
             newrow++;
          }
        elm_code_widget_cursor_position_set(widget, info->end_line,
                                            info->end_col + 1);
     }

   efl_event_callback_legacy_call(widget, ELM_OBJ_CODE_WIDGET_EVENT_CHANGED_USER, NULL);
}

static Eina_Bool
_elm_code_widget_can_undo_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   return !!pd->undo_stack_ptr;
}

static void
_elm_code_widget_undo(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   Elm_Code_Widget_Change_Info *info;

   if (!pd->undo_stack_ptr)
     return;

   info = eina_list_data_get(pd->undo_stack_ptr);
   _elm_code_widget_undo_change(obj, info);

   pd->undo_stack_ptr = eina_list_next(pd->undo_stack_ptr);
}

static Eina_Bool
_elm_code_widget_can_redo_get(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   if (pd->undo_stack_ptr)
     return !!eina_list_prev(pd->undo_stack_ptr);

   return !!eina_list_last(pd->undo_stack);
}

static void
_elm_code_widget_redo(Eo *obj EINA_UNUSED, Elm_Code_Widget_Data *pd)
{
   Elm_Code_Widget_Change_Info *info, *redo_info;
   Eina_List *redo_ptr;

   if (pd->undo_stack_ptr)
     redo_ptr = eina_list_prev(pd->undo_stack_ptr);
   else
     redo_ptr = eina_list_last(pd->undo_stack);

   if (!redo_ptr)
     return;

   info = eina_list_data_get(redo_ptr);
   redo_info = _elm_code_widget_undo_info_copy(info);
   redo_info->insert = redo_info->insert ? EINA_FALSE : EINA_TRUE;
   _elm_code_widget_undo_change(obj, redo_info);

   pd->undo_stack_ptr = redo_ptr;

   free(redo_info->content);
   free(redo_info);
}

