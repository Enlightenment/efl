#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#define ELM_INTERNAL_API_ARGESFSDFEFC
#include <elm_widget.h>

#include <Elm_Code.h>
#include "elm_code_widget2.eo.h"

typedef struct
{
   Elm_Code *code;
   Evas_Object *grid;

   Evas_Font_Size font_size;
   unsigned int cursor_line, cursor_col;
   Eina_Bool editable, focussed;
} Elm_Code_Widget2_Data;

Eina_Unicode status_icons2[] = {
 ' ',
 '!',

 '+',
 '-',
 ' ',

 0x2713,
 0x2717,

 0
};


EOLIAN static void
_elm_code_widget2_eo_base_constructor(Eo *obj, Elm_Code_Widget2_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, ELM_CODE_WIDGET2_CLASS, eo_constructor());

   pd->cursor_line = 1;
   pd->cursor_col = 1;
}

EOLIAN static void
_elm_code_widget2_class_constructor(Eo_Class *klass EINA_UNUSED)
{

}

static Eina_Bool
_elm_code_widget_resize(Evas_Object *o)
{
   int w, h, cw, ch;

   evas_object_geometry_get(o, NULL, NULL, &w, &h);
   evas_object_textgrid_cell_size_get(o, &cw, &ch);

   evas_object_textgrid_size_set(o, ceil(((double) w) / cw),
         ceil(((double) h) / ch));

   return h > 0 && w > 0;
}

static void
_elm_code_widget_fill_line_token(Evas_Textgrid_Cell *cells, int count, int start, int end, Elm_Code_Token_Type type)
{
   int x;

   for (x = start; x <= end && x < count; x++)
     {
        cells[x].fg = type;
     }
}

static void
_elm_code_widget_fill_line_tokens(Evas_Textgrid_Cell *cells, unsigned int count, Elm_Code_Line *line)
{
   Eina_List *item;
   Elm_Code_Token *token;
   int start, length;

   start = 1;
   length = line->length;

   EINA_LIST_FOREACH(line->tokens, item, token)
     {

        _elm_code_widget_fill_line_token(cells, count, start, token->start, ELM_CODE_TOKEN_TYPE_DEFAULT);

        // TODO handle a token starting before the previous finishes
        _elm_code_widget_fill_line_token(cells, count, token->start, token->end, token->type);

        start = token->end + 1;
     }

   _elm_code_widget_fill_line_token(cells, count, start, length, ELM_CODE_TOKEN_TYPE_DEFAULT);
}

static void
_elm_code_widget_fill_line(Elm_Code_Widget2_Data *pd, Evas_Textgrid_Cell *cells, Elm_Code_Line *line)
{
   char *chr;
   unsigned int length, x;
   int w;

   if (!_elm_code_widget_resize(pd->grid))
     return;

   length = line->length;
   evas_object_textgrid_size_get(pd->grid, &w, NULL);

   cells[0].codepoint = status_icons2[line->status];
   cells[0].bold = 1;
   cells[0].fg = ELM_CODE_TOKEN_TYPE_DEFAULT;
   cells[0].bg = line->status;

   if (line->modified)
      chr = line->modified;
   else
      chr = (char *)line->content;
   for (x = 1; x < (unsigned int) w && x <= length; x++)
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

   _elm_code_widget_fill_line_tokens(cells, w, line);
   if (pd->editable && pd->focussed && pd->cursor_line == line->number)
     {
        if (pd->cursor_col < (unsigned int) w)
          cells[pd->cursor_col].bg = ELM_CODE_TOKEN_TYPE_CURSOR;
     }

   evas_object_textgrid_update_add(pd->grid, 0, line->number - 1, w, 1);
}

static void
_elm_code_widget_fill(Elm_Code_Widget2_Data *pd)
{
   Elm_Code_Line *line;
   Evas_Textgrid_Cell *cells;
   int w, h;
   unsigned int y;

   if (!_elm_code_widget_resize(pd->grid))
     return;
   evas_object_textgrid_size_get(pd->grid, &w, &h);

   for (y = 1; y <= (unsigned int) h && y <= elm_code_file_lines_get(pd->code->file); y++)
     {
        line = elm_code_file_line_get(pd->code->file, y);

        cells = evas_object_textgrid_cellrow_get(pd->grid, y - 1);
        _elm_code_widget_fill_line(pd, cells, line);
     }
}

static Eina_Bool
_elm_code_widget2_line_cb(void *data, Eo *obj EINA_UNUSED,
                         const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Code_Widget2_Data *widget;
   Elm_Code_Line *line;
   int h;

   Evas_Textgrid_Cell *cells;

   widget = (Elm_Code_Widget2_Data *)data;
   line = (Elm_Code_Line *)event_info;

   evas_object_textgrid_size_get(widget->grid, NULL, &h);

   if (line->number > (unsigned int) h)
     return EINA_TRUE;

   cells = evas_object_textgrid_cellrow_get(widget->grid, line->number - 1);
   _elm_code_widget_fill_line(widget, cells, line);

   return EINA_TRUE;
}


static Eina_Bool
_elm_code_widget2_file_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Elm_Code_Widget2_Data *widget;

   widget = (Elm_Code_Widget2_Data *)data;

   _elm_code_widget_fill(widget);
   return EINA_TRUE;
}

static void
_elm_code_widget2_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Code_Widget2_Data *widget;

   widget = (Elm_Code_Widget2_Data *)data;

   _elm_code_widget_fill(widget);
}

static void
_elm_code_widget2_clicked_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                            void *event_info)
{
   Elm_Code_Widget2 *widget;
   Elm_Code_Widget2_Data *pd;
   Elm_Code_Line *line;
   Evas_Event_Mouse_Up *event;
   Evas_Coord y;
   int ch;
   unsigned int row;

   widget = (Elm_Code_Widget2 *)data;
   pd = eo_data_scope_get(widget, ELM_CODE_WIDGET2_CLASS);

   if (pd->editable && !pd->focussed)
     return;

   event = (Evas_Event_Mouse_Up *)event_info;
   y = event->canvas.y;

   evas_object_textgrid_cell_size_get(pd->grid, NULL, &ch);
   row = ((double) y / ch) + 1;

   line = elm_code_file_line_get(pd->code->file, row);
   if (line)
     eo_do(widget, eo_event_callback_call(ELM_CODE_WIDGET2_EVENT_LINE_CLICKED, line));
}

EOLIAN static Eina_Bool
_elm_code_widget2_elm_widget_on_focus(Eo *obj, Elm_Code_Widget2_Data *pd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, ELM_CODE_WIDGET2_CLASS, int_ret = elm_obj_widget_on_focus());
   if (!int_ret) return EINA_TRUE;

   pd->focussed = elm_widget_focus_get(obj);

   _elm_code_widget_fill(pd);
   return EINA_TRUE;
}

EOLIAN static void
_elm_code_widget2_elm_interface_scrollable_content_pos_set(Eo *obj EINA_UNUSED,
                                                           Elm_Code_Widget2_Data *pd EINA_UNUSED,
                                                           Evas_Coord x, Evas_Coord y,
                                                           Eina_Bool sig EINA_UNUSED)
{
   printf("scroll to %d, %d\n", x, y);
}

EOLIAN static void
_elm_code_widget2_font_size_set(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd, Evas_Font_Size font_size)
{
   evas_object_textgrid_font_set(pd->grid, "Mono", font_size * elm_config_scale_get());
   pd->font_size = font_size;
}

EOLIAN static Evas_Font_Size
_elm_code_widget2_font_size_get(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd)
{
   return pd->font_size;
}

EOLIAN static void
_elm_code_widget2_code_set(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd, Elm_Code *code)
{
   pd->code = code;

   code->widgets = eina_list_append(code->widgets, pd);
}

EOLIAN static Elm_Code *
_elm_code_widget2_code_get(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd)
{
   return pd->code;
}

EOLIAN static void
_elm_code_widget2_editable_set(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd, Eina_Bool editable)
{
   pd->editable = editable;
}

EOLIAN static Eina_Bool
_elm_code_widget2_editable_get(Eo *obj EINA_UNUSED, Elm_Code_Widget2_Data *pd)
{
   return pd->editable;
}

static void
_elm_code_widget2_setup_palette(Evas_Object *o)
{
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

   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_PASSED,
                                    54, 96, 54, 255);
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_STATUS_TYPE_FAILED,
                                    96, 54, 54, 255);

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

   // the style for a cursor - this is a special token and will be applied to the background
   evas_object_textgrid_palette_set(o, EVAS_TEXTGRID_PALETTE_STANDARD, ELM_CODE_TOKEN_TYPE_CURSOR,
                                    205, 205, 54, 255);
}

EOLIAN static void
_elm_code_widget2_evas_object_smart_add(Eo *obj, Elm_Code_Widget2_Data *pd)
{
   Evas_Object *grid;

   eo_do_super(obj, ELM_CODE_WIDGET2_CLASS, evas_obj_smart_add());
   elm_widget_can_focus_set(obj, EINA_TRUE);

   grid = evas_object_textgrid_add(obj);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(grid, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(grid);
   elm_box_pack_end(obj, grid);
   pd->grid = grid;
   _elm_code_widget2_setup_palette(grid);

   evas_object_event_callback_add(grid, EVAS_CALLBACK_RESIZE, _elm_code_widget2_resize_cb, pd);
   evas_object_event_callback_add(grid, EVAS_CALLBACK_MOUSE_UP, _elm_code_widget2_clicked_cb, obj);

   eo_do(obj,
         eo_event_callback_add(&ELM_CODE_EVENT_LINE_SET_DONE, _elm_code_widget2_line_cb, pd);
         eo_event_callback_add(&ELM_CODE_EVENT_FILE_LOAD_DONE, _elm_code_widget2_file_cb, pd));

   _elm_code_widget2_font_size_set(obj, pd, 10);
}

#include "elm_code_widget2.eo.c"
