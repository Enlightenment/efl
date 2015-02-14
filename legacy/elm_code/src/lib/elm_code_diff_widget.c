#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

#define ELM_CODE_DIFF_WIDGET_LEFT "diffwidgetleft"
#define ELM_CODE_DIFF_WIDGET_RIGHT "diffwidgetright"

static void _elm_code_diff_widget_parse_diff_line(Elm_Code_Line *line, int number, Elm_Code_File *left, Elm_Code_File *right)
{
   if (line->length < 1)
     {
        elm_code_file_line_append(left, "", 0);
        elm_code_file_line_append(right, "", 0);
     }

   if (line->content[0] == '+')
     {
        elm_code_file_line_append(left, "", 0);
        elm_code_file_line_append(right, line->content+1, line->length-1);
        elm_code_file_line_status_set(right, number, ELM_CODE_STATUS_TYPE_ADDED);
     }
   else if (line->content[0] == '-')
     {
        elm_code_file_line_append(left, line->content+1, line->length-1);
        elm_code_file_line_append(right, "", 0);
        elm_code_file_line_status_set(left, number, ELM_CODE_STATUS_TYPE_REMOVED);
     }
   else
     {
        elm_code_file_line_append(left, line->content+1, line->length-1);
        elm_code_file_line_append(right, line->content+1, line->length-1);
     }
}

static void _elm_code_diff_widget_parse_diff(Elm_Code_File *diff, Elm_Code_File *left, Elm_Code_File *right)
{
   Eina_List *item;
   Elm_Code_Line *line;
   int number, offset;

   number = 0;
   offset = 0;
   EINA_LIST_FOREACH(diff->lines, item, line)
     {

        if (line->length > 0)
          {
             if (line->content[0] == 'd' || line->content[0] == 'i' || line->content[0] == 'n')
               {
                  offset = 0;
                  continue;
               }
          }

        number++;
        if (offset == 0)
          {
             elm_code_file_line_append(left, line->content+4, line->length-4);
             elm_code_file_line_status_set(left, number, ELM_CODE_STATUS_TYPE_CHANGED);
          }
        else if (offset == 1)
          {
             number--;
             elm_code_file_line_append(right, line->content+4, line->length-4);
             elm_code_file_line_status_set(right, number, ELM_CODE_STATUS_TYPE_CHANGED);
          }
        else
          {
             _elm_code_diff_widget_parse_diff_line(line, number, left, right);
          }

        offset++;
     }
}

EAPI Evas_Object *elm_code_diff_widget_add(Evas_Object *parent, Elm_Code *code)
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
         elm_code_widget_code_set(wcode1));

   evas_object_size_hint_weight_set(widget_left, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget_left, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget_left);
   evas_object_data_set(hbox, ELM_CODE_DIFF_WIDGET_LEFT, widget_left);
   elm_object_part_content_set(hbox, "left", widget_left);

   // right side of diff
   wcode2 = elm_code_create();
   widget_right = eo_add(ELM_CODE_WIDGET_CLASS, parent);
   eo_do(widget_right,
         elm_code_widget_code_set(wcode2));

   evas_object_size_hint_weight_set(widget_right, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(widget_right, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(widget_right);
   evas_object_data_set(hbox, ELM_CODE_DIFF_WIDGET_RIGHT, widget_right);
   elm_object_part_content_set(hbox, "right", widget_right);

   _elm_code_diff_widget_parse_diff(code->file, wcode1->file, wcode2->file);
   return hbox;
}

EAPI void elm_code_diff_widget_font_size_set(Evas_Object *widget, int size)
{
   Elm_Code_Widget *child;

   child = (Elm_Code_Widget *) evas_object_data_get(widget, ELM_CODE_DIFF_WIDGET_LEFT);
   eo_do(child, elm_code_widget_font_size_set(size));
   child = (Elm_Code_Widget *) evas_object_data_get(widget, ELM_CODE_DIFF_WIDGET_RIGHT);
   eo_do(child, elm_code_widget_font_size_set(size));
}

