#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include <Eo.h>
#include <Elementary.h>

#include "elm_code_widget.h"

#include "elm_code_private.h"

static Eina_Bool _elm_code_widget_resize(Evas_Object *o)
{
   int w, h, cw, ch;

   evas_object_geometry_get(o, NULL, NULL, &w, &h);
   evas_object_textgrid_cell_size_get(o, &cw, &ch);
   evas_object_textgrid_size_set(o, ceil(((double) w) / cw),
         ceil(((double) h) / ch));

   return h > 0 && w > 0;
}

static void _elm_code_widget_fill_line_token(Evas_Textgrid_Cell *cells, int count, int start, int end, Elm_Code_Token_Type type)
{
   int x;

   for (x = start - 1; x < end && x < count; x++)
     {
        cells[x].fg = type;
     }
}

EAPI void elm_code_widget_fill_line_tokens(Evas_Textgrid_Cell *cells, int count, Elm_Code_Line *line)
{
   Eina_List *item;
   Elm_Code_Token *token;
   int start, length;

   start = 1;
   length = strlen(line->content);

   EINA_LIST_FOREACH(line->tokens, item, token)
     {

        _elm_code_widget_fill_line_token(cells, count, start, token->start, ELM_CODE_TOKEN_TYPE_DEFAULT);

        // TODO handle a token starting before the previous finishes
        _elm_code_widget_fill_line_token(cells, count, token->start, token->end, token->type);

        start = token->end + 1;
     }

   _elm_code_widget_fill_line_token(cells, count, start, length, ELM_CODE_TOKEN_TYPE_DEFAULT);
}

static void _elm_code_widget_fill_line(Evas_Object *o, Evas_Textgrid_Cell *cells, Elm_Code_Line *line)
{
   char *chr;
   unsigned int length, x;
   int w;

   if (!_elm_code_widget_resize(o))
     return;

   length = strlen(line->content);
   evas_object_textgrid_size_get(o, &w, NULL);

   chr = (char *)line->content;
   for (x = 0; x < (unsigned int) w && x < length; x++)
     {
        cells[x].codepoint = *chr;
        cells[x].bg = line->status;

        chr++;
     }
   for (; x < (unsigned int) w; x++)
     {
        cells[x].codepoint = 0;
        cells[x].bg = line->status;
     }

   elm_code_widget_fill_line_tokens(cells, w, line);

   evas_object_textgrid_update_add(o, 0, line->number - 1, w, 1);
}

EAPI void elm_code_widget_fill(Evas_Object *o, Elm_Code *code)
{
   Elm_Code_Line *line;
   Evas_Textgrid_Cell *cells;
   int w, h;
   unsigned int y;

   if (!_elm_code_widget_resize(o))
     return;
   evas_object_textgrid_size_get(o, &w, &h);

   for (y = 1; y <= (unsigned int) h && y <= elm_code_file_lines_get(code->file); y++)
     {
        line = elm_code_file_line_get(code->file, y);

        cells = evas_object_textgrid_cellrow_get(o, y - 1);
        _elm_code_widget_fill_line(o, cells, line);
     }
}

static Eina_Bool
_elm_code_widget_line_cb(void *data EINA_UNUSED, Eo *obj,
                         const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Code_Line *line;
   Evas_Object *o;

   Evas_Textgrid_Cell *cells;

   line = (Elm_Code_Line *)event_info;
   o = (Evas_Object *)obj;

   cells = evas_object_textgrid_cellrow_get(o, line->number - 1);
   _elm_code_widget_fill_line(o, cells, line);

   return EINA_TRUE;
}


static Eina_Bool
_elm_code_widget_file_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Evas_Object *o;
   Elm_Code *code;

   code = (Elm_Code *)data;
   o = (Evas_Object *)obj;

   elm_code_widget_fill(o, code);
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
                                    36, 36, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ERROR,
                                    205, 54, 54, 255);

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_ADDED,
                                    36, 96, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_REMOVED,
                                    96, 36, 36, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_CHANGED,
                                    36, 36, 96, 255);

   // setup token colors
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_DEFAULT,
                                    205, 205, 205, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_COMMENT,
                                    54, 205, 255, 255);

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_ADDED,
                                    54, 255, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_REMOVED,
                                    255, 54, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_CHANGED,
                                    54, 54, 255, 255);

   evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE, _elm_code_widget_resize_cb, code);

   eo_do(o,eo_event_callback_add(&ELM_CODE_EVENT_LINE_SET_DONE, _elm_code_widget_line_cb, code));
   eo_do(o,eo_event_callback_add(&ELM_CODE_EVENT_FILE_LOAD_DONE, _elm_code_widget_file_cb, code));

   code->widgets = eina_list_append(code->widgets, o);
   return o;
}

