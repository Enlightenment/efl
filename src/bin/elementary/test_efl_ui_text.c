#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Elementary.h>

static void
_apply_style(Eo *obj, size_t start_pos, size_t end_pos, const char *style)
{
   Efl_Canvas_Text_Cursor *start, *end;

   start = efl_canvas_text_cursor_get(obj);
   end = efl_ui_text_cursor_new(obj);

   efl_canvas_text_cursor_position_set(start, start_pos);
   efl_canvas_text_cursor_position_set(end, end_pos);

   efl_canvas_text_annotation_insert(obj, start, end, style);
}

static Eo *
_create_label(Eo *win, Eo *bx)
{
   Eo *en;
   en = efl_add(EFL_UI_TEXT_CLASS, win);
   printf("Added Efl.Ui.Text object\n");
   efl_ui_text_interactive_editable_set(en, EINA_FALSE);
   efl_canvas_text_style_set(en, NULL, "DEFAULT='align=center font=Sans font_size=10 color=#fff wrap=word'");

   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
   return en;
}

void
test_efl_ui_text_label(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx;
   Eo *en;

   win = elm_win_util_standard_add("label", "Label");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a small label");
   //                012345678901234567890
   _apply_style(en, 0, 21, "font_size=12 font_weight=bold");

   en = _create_label(win, bx);
   efl_text_set(en, "This is a text. Is also has\n"
         "newlines. There are several styles applied.");
   _apply_style(en, 40, 45, "font_weight=bold color=#ff0");
   _apply_style(en, 52, 58, "font_weight=italic color=#f00");
   efl_canvas_text_style_set(en, NULL, "DEFAULT='align=center font=Sans font_size=12 color=#fff wrap=word'");

//   elm_object_focus_set(en, EINA_TRUE);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);
}
