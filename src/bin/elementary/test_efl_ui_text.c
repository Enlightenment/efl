#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Elementary.h>

// 1. Label-equivalent setup

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

// 2. Entry-equivalent setup

typedef struct
{
   const char *wrap_mode[4];
   size_t cur_wrap;
} Test_Data;

static void
my_efl_ui_text_anchor_hover_opened(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *bt;
   Eo *en = data;

   Efl_Ui_Text_Anchor_Hover_Info *ei = event->info;

   bt = efl_add(ELM_BUTTON_CLASS, en);
   elm_object_text_set(bt, ei->anchor_info->name);
   evas_object_show(bt);
   elm_object_part_content_set(ei->hover, "middle", bt);

   printf("anchor hover\n");
}

static void
my_efl_ui_text_bt_3(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *sel_start, *sel_end;

   efl_ui_text_interactive_selection_cursors_get(data, &sel_start, &sel_end);
   const char *s = efl_canvas_text_range_text_get(data, sel_start, sel_end);

   printf("SELECTION REGION: %d - %d\n",
         efl_canvas_text_cursor_position_get(sel_start),
         efl_canvas_text_cursor_position_get(sel_end));
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
}

static void
my_efl_ui_text_bt_4(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   efl_canvas_text_cursor_object_item_insert(efl_canvas_text_cursor_get(en),
         "size=32x32 href=emoticon");
}

const char *_wrap_modes[4] = { "none", "char", "word", "mixed" };

static void
my_efl_ui_text_bt_6(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char style[128];
   Evas_Object *en = data;
   size_t idx = (size_t) efl_key_data_get(en, "wrap_idx");
   idx = (idx + 1) % 4;
   efl_key_data_set(en, "wrap_idx", (void *)idx);

   sprintf(style, "DEFAULT='font=Sans font_size=12 color=#fff wrap=%s'",
         _wrap_modes[idx]);
   efl_canvas_text_style_set(en, NULL, style);
   printf("wrap mode changed to '%s'\n", _wrap_modes[idx]);

}

static void
my_efl_ui_text_bt_5(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   efl_ui_text_scrollable_set(en, !efl_ui_text_scrollable_get(en));
}

void
test_efl_ui_text(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *bx3, *bt, *en;
   Efl_Canvas_Text_Cursor *main_cur, *cur;

   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = efl_add(EFL_UI_TEXT_CLASS, win);

   printf("Added Efl.Ui.Text object\n");
   efl_key_data_set(en, "wrap_idx", 0);
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");

   main_cur = efl_canvas_text_cursor_get(en);
   cur = efl_ui_text_cursor_new(en);

   efl_canvas_text_cursor_position_set(cur, 2);
   efl_canvas_text_cursor_object_item_insert(cur, "size=32x32 href=emoticon");
   efl_canvas_text_cursor_position_set(cur, 50);
   efl_canvas_text_cursor_object_item_insert(cur, "size=32x32 href=emoticon");

   efl_canvas_text_cursor_position_set(main_cur, 5);
   efl_canvas_text_cursor_position_set(cur, 20);

   efl_canvas_text_annotation_insert(en, main_cur, cur, "a href=#hello");

   efl_ui_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, en);
   evas_object_show(en);
   elm_object_focus_set(en, EINA_TRUE);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Sel");
   evas_object_smart_callback_add(bt, "clicked", my_efl_ui_text_bt_3, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Ins");
   evas_object_smart_callback_add(bt, "clicked", my_efl_ui_text_bt_4, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Scrl");
   evas_object_smart_callback_add(bt, "clicked", my_efl_ui_text_bt_5, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wr");
   evas_object_smart_callback_add(bt, "clicked", my_efl_ui_text_bt_6, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

#if 0
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Edit");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_7, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);
#endif

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(bx, bx3);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx3);
   evas_object_show(bx2);

   efl_event_callback_add(en, EFL_UI_TEXT_EVENT_ANCHOR_HOVER_OPENED, my_efl_ui_text_anchor_hover_opened, en);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);
}
