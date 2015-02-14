#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

#define _ELM_CODE_DIFF_WIDGET_LEFT "diffwidgetleft"
#define _ELM_CODE_DIFF_WIDGET_RIGHT "diffwidgetright"

#define _ELM_CODE_DIFF_WIDGET_TYPE_ADDED "added"
#define _ELM_CODE_DIFF_WIDGET_TYPE_REMOVED "removed"
#define _ELM_CODE_DIFF_WIDGET_TYPE_CHANGED "changed"

static void
_elm_code_diff_widget_parse_diff_line(Elm_Code_Line *line, Elm_Code_File *left, Elm_Code_File *right)
{
   if (line->length < 1)
     {
        elm_code_file_line_append(left, "", 0, NULL);
        elm_code_file_line_append(right, "", 0, NULL);
     }

   if (line->content[0] == '+')
     {
        elm_code_file_line_append(left, "", 0, NULL);
        elm_code_file_line_append(right, line->content+1, line->length-1, _ELM_CODE_DIFF_WIDGET_TYPE_ADDED);
     }
   else if (line->content[0] == '-')
     {
        elm_code_file_line_append(left, line->content+1, line->length-1, _ELM_CODE_DIFF_WIDGET_TYPE_REMOVED);
        elm_code_file_line_append(right, "", 0, NULL);
     }
   else
     {
        elm_code_file_line_append(left, line->content+1, line->length-1, NULL);
        elm_code_file_line_append(right, line->content+1, line->length-1, NULL);
     }
}

static void
_elm_code_diff_widget_parse_diff(Elm_Code_File *diff, Elm_Code_File *left, Elm_Code_File *right)
{
   Eina_List *item;
   Elm_Code_Line *line;
   int offset;

   offset = 0;
   EINA_LIST_FOREACH(diff->lines, item, line)
     {
        if (line->length > 0 && (line->content[0] == 'd' || line->content[0] == 'i' || line->content[0] == 'n'))
          {
             offset = 0;
             continue;
          }

        if (offset == 0)
          elm_code_file_line_append(left, line->content+4, line->length-4, _ELM_CODE_DIFF_WIDGET_TYPE_CHANGED);
        else if (offset == 1)
          elm_code_file_line_append(right, line->content+4, line->length-4, _ELM_CODE_DIFF_WIDGET_TYPE_CHANGED);
        else
          _elm_code_diff_widget_parse_diff_line(line, left, right);

        offset++;
     }
}

static Eina_Bool
_elm_code_diff_widget_line_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
                              const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Code_Line *line;

   line = (Elm_Code_Line *)event_info;

   if (!line->data)
     return EO_CALLBACK_CONTINUE;

   if (!strcmp(_ELM_CODE_DIFF_WIDGET_TYPE_ADDED, (char *)line->data))
     line->status = ELM_CODE_STATUS_TYPE_ADDED;
   else if (!strcmp(_ELM_CODE_DIFF_WIDGET_TYPE_REMOVED, (char *)line->data))
     line->status = ELM_CODE_STATUS_TYPE_REMOVED;
   else if (!strcmp(_ELM_CODE_DIFF_WIDGET_TYPE_CHANGED, (char *)line->data))
     line->status = ELM_CODE_STATUS_TYPE_CHANGED;

   return EO_CALLBACK_CONTINUE;
}

EAPI Evas_Object *
elm_code_diff_widget_add(Evas_Object *parent, Elm_Code *code)
{
   Elm_Code *wcode1, *wcode2;
   Elm_Code_Widget *widget_left, *widget_right;
   Evas_Object *hbox;

   hbox = elm_panes_add(parent);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_panes_horizontal_set(hbox, EINA_FALSE);
   evas_object_show(hbox);

   // left side of diff
   wcode1 = elm_code_create();
   widget_left = eo_add(ELM_CODE_WIDGET_CLASS, parent);
   eo_do(widget_left,
         elm_code_widget_code_set(wcode1),
         eo_event_callback_add(&ELM_CODE_EVENT_LINE_LOAD_DONE, _elm_code_diff_widget_line_cb, NULL));

   evas_object_size_hint_weight_set(widget_left, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget_left, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget_left);
   evas_object_data_set(hbox, _ELM_CODE_DIFF_WIDGET_LEFT, widget_left);
   elm_object_part_content_set(hbox, "left", widget_left);

   // right side of diff
   wcode2 = elm_code_create();
   widget_right = eo_add(ELM_CODE_WIDGET_CLASS, parent);
   eo_do(widget_right,
         elm_code_widget_code_set(wcode2),
         eo_event_callback_add(&ELM_CODE_EVENT_LINE_LOAD_DONE, _elm_code_diff_widget_line_cb, NULL));

   evas_object_size_hint_weight_set(widget_right, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget_right, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget_right);
   evas_object_data_set(hbox, _ELM_CODE_DIFF_WIDGET_RIGHT, widget_right);
   elm_object_part_content_set(hbox, "right", widget_right);

   _elm_code_diff_widget_parse_diff(code->file, wcode1->file, wcode2->file);
   return hbox;
}

EAPI void
elm_code_diff_widget_font_size_set(Evas_Object *widget, int size)
{
   Elm_Code_Widget *child;

   child = (Elm_Code_Widget *) evas_object_data_get(widget, _ELM_CODE_DIFF_WIDGET_LEFT);
   eo_do(child, elm_code_widget_font_size_set(size));
   child = (Elm_Code_Widget *) evas_object_data_get(widget, _ELM_CODE_DIFF_WIDGET_RIGHT);
   eo_do(child, elm_code_widget_font_size_set(size));
}

