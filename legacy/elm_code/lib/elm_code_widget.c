#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include <Eo.h>
#include <Elementary.h>

#include "elm_code_widget.h"

#include "elm_code_private.h"

EAPI void elm_code_widget_fill(Evas_Object *o, Elm_Code *code)
{
   Elm_Code_Line *line;
   Evas_Textgrid_Cell *cells;
   const char *content;
   char *chr;
   unsigned int length;
   int w, h, cw, ch;
   unsigned int x, y;

   evas_object_geometry_get(o, NULL, NULL, &w, &h);
   evas_object_textgrid_cell_size_get(o, &cw, &ch);
   evas_object_textgrid_size_set(o, ceil(((double) w) / cw),
         ceil(((double) h) / ch));
   evas_object_textgrid_size_get(o, &w, &h);

   for (y = 1; y <= (unsigned int) h && y <= elm_code_file_lines_get(code->file); y++)
     {
        line = elm_code_file_line_get(code->file, y);
        content = elm_code_file_line_content_get(code->file, y);
        chr = (char *)content;

        cells = evas_object_textgrid_cellrow_get(o, y - 1);
        length = strlen(content);

        for (x = 0; x < (unsigned int) w && x < length; x++)
          {
             cells[x].codepoint = *chr;
             cells[x].bg = line->status;
             cells[x].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;

             chr++;
          }
        for (; x < (unsigned int) w; x++)
          {
             cells[x].codepoint = 0;
             cells[x].bg = line->status;
             cells[x].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;
          }
     }

   evas_object_textgrid_update_add(o, 0, 0, w, h);
}

static Eina_Bool
_elm_code_widget_line_cb(void *data EINA_UNUSED, Eo *obj,
                         const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Code_Line *line;
   Evas_Object *o;

   Evas_Textgrid_Cell *cells;
   char *chr;
   unsigned int length, x;
   int w;

   line = (Elm_Code_Line *)event_info;
   o = (Evas_Object *)obj;

   cells = evas_object_textgrid_cellrow_get(o, line->number - 1);
   length = strlen(line->content);
   evas_object_textgrid_size_get(o, &w, NULL);

   chr = (char *)line->content;
   for (x = 0; x < (unsigned int) w && x < length; x++)
     {
        cells[x].codepoint = *chr;
        cells[x].bg = line->status;
        cells[x].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;

        chr++;
     }
   for (; x < (unsigned int) w; x++)
     {
        cells[x].codepoint = 0;
        cells[x].bg = line->status;
        cells[x].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;
     }

   evas_object_textgrid_update_add(o, 0, line->number - 1, w, 1);
   return EINA_TRUE;
}

static void
_elm_code_widget_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
                           void *event_info EINA_UNUSED)
{
   Elm_Code *code;

   code = (Elm_Code *)data;
   elm_code_widget_fill(obj, code);
}

EAPI Evas_Object *elm_code_widget_add(Evas_Object *parent, Elm_Code *code)
{
   Evas_Object *o;

   o = evas_object_textgrid_add(parent);

   evas_object_textgrid_font_set(o, "Mono", 10 * elm_config_scale_get());

   // setup status colors
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_DEFAULT,
                                    54, 54, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ERROR,
                                    205, 54, 54, 255);

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ADDED,
                                    54, 125, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_REMOVED,
                                    125, 54, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_CHANGED,
                                    54, 54, 125, 255);

   // setup token colors
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_DEFAULT,
                                    205, 205, 205, 255);

   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _elm_code_widget_resize_cb, code);

   eo_do(o,eo_event_callback_add(&ELM_CODE_EVENT_LINE_SET_DONE, _elm_code_widget_line_cb, code));

   code->widgets = eina_list_append(code->widgets, o);
   return o;
}

