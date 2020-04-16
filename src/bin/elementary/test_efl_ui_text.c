#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_apply_style(Eo *obj, size_t start_pos, size_t end_pos, const char *style)
{
   Efl_Text_Cursor_Object *start, *end;

   start = efl_ui_textbox_cursor_create(obj);
   end = efl_ui_textbox_cursor_create(obj);

   efl_text_cursor_object_position_set(start, start_pos);
   efl_text_cursor_object_position_set(end, end_pos);

   efl_text_formatter_attribute_insert(start, end, style);

   efl_del(start);
   efl_del(end);
}

static Eo *
_create_label(Eo *win, Eo *bx)
{
   Eo *en;
   en = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   printf("Added Efl.Ui.Textbox object\n");
   efl_text_interactive_editable_set(en, EINA_FALSE);
   efl_pack(bx, en);
   return en;
}


void
test_efl_ui_text_label(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx;
   Eo *en;
   char *markup;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                  efl_text_set(efl_added, "Efl Canvas_Layout"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, bx);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a\t small label");
   //                012345678901234567890
   _apply_style(en, 0, 21, "font_size=12 font_weight=bold");
   efl_text_font_weight_set(en, EFL_TEXT_FONT_WEIGHT_BOLD);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a text. It is longer but its size is taken as the"
         " min size so that it shows in whole");
   _apply_style(en, 40, 45, "font_weight=bold color=#ff0");
   _apply_style(en, 52, 58, "font_weight=italic color=#f00");

   en = _create_label(win, bx);
   efl_text_set(en, "By default 'multiline' is disabled.\n"
         "So, \\n would only work if you enable it.");

   en = _create_label(win, bx);
   efl_text_markup_set(en, "You can also <b>ENTER</b> markup!");
   efl_text_font_family_set(en, "Sans");
   efl_text_font_size_set(en, 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_text_to_markup(
         "You can use the text -> markup helper\nto make coding easier.");
   efl_text_markup_set(en, markup);
   free(markup);
   efl_text_font_family_set(en, "Sans");
   efl_text_font_size_set(en, 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_markup_to_text(
         "You can use markup -&gt; text helpers<ps>to make coding easier.");
   efl_text_set(en, markup);
   free(markup);
   efl_text_font_family_set(en, "Sans");
   efl_text_font_size_set(en, 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
}

typedef struct
{
   const char *wrap_mode[4];
   size_t cur_wrap;
} Test_Data;

static void
_on_bt3_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Text_Cursor_Object *sel_start, *sel_end;
   Eo *en = data;

   efl_text_interactive_selection_cursors_get(en, &sel_start, &sel_end);
   const char *s = efl_text_cursor_object_range_text_get(sel_start, sel_end);

   printf("SELECTION REGION: %d - %d\n",
         efl_text_cursor_object_position_get( sel_start),
         efl_text_cursor_object_position_get(sel_end));
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
}

static void
_on_bt6_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *text_obj = data;

   Efl_Text_Format_Wrap wrap = efl_text_wrap_get(text_obj);

   switch(wrap)
     {
      case EFL_TEXT_FORMAT_WRAP_NONE:
         wrap = EFL_TEXT_FORMAT_WRAP_CHAR;
         break;
      case EFL_TEXT_FORMAT_WRAP_CHAR:
         wrap = EFL_TEXT_FORMAT_WRAP_WORD;
         break;
      case EFL_TEXT_FORMAT_WRAP_WORD:
         wrap = EFL_TEXT_FORMAT_WRAP_MIXED;
         break;
      case EFL_TEXT_FORMAT_WRAP_MIXED:
         wrap = EFL_TEXT_FORMAT_WRAP_NONE;
         break;
      default:
         break;
     }
   efl_text_wrap_set(text_obj, wrap);
}

void
test_efl_ui_text(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *bx2, *en;
   Eo *bt;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                  efl_text_set(efl_added, "Efl Ui Text"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(bx, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_content_set(win, bx);

   en = efl_add(EFL_UI_TEXTBOX_CLASS, bx,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   printf("Added Efl.Ui.Textbox object\n");

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_textbox_scrollable_set(en, EINA_TRUE);

   efl_text_font_family_set(en, "Sans");
   efl_text_font_size_set(en, 12);
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_color_set(en, 255, 255, 255, 255);

   efl_text_set(en,
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod\n"
      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim\n"
      "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea\n"
      "commodo consequat.  Duis aute irure dolor in reprehenderit in voluptate\n"
      "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint\n"
      "occaecat cupidatat non proident, sunt in culpa qui officia deserunt\n"
      "mollit anim id est laborum");

   efl_gfx_hint_size_min_set(en, EINA_SIZE2D(300, 100));
   efl_pack(bx, en);

   bx2 = efl_add(EFL_UI_BOX_CLASS, bx);
   efl_gfx_hint_weight_set(bx2, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_ui_layout_orientation_set(bx2, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Sel");
   efl_event_callback_add(bt, EFL_INPUT_EVENT_CLICKED, _on_bt3_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Wr");
   efl_event_callback_add(bt, EFL_INPUT_EVENT_CLICKED, _on_bt6_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   efl_pack(bx, bx2);
}

void
test_efl_ui_text_inputfield(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *en;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                  efl_text_set(efl_added, "Efl Ui Text Input Field"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(bx, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_content_set(win, bx);

   en = efl_add(EFL_UI_TEXTBOX_CLASS, bx,
         efl_text_multiline_set(efl_added, EINA_FALSE));

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_textbox_scrollable_set(en, EINA_TRUE);

   printf("Added Efl.Ui.Textbox object\n");
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_set(efl_part(en, "efl.text_guide"), "Enter you name here");

   efl_pack(bx, en);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a multiline input.\n"
         "Enter multiline here");
   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   efl_ui_textbox_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 200));

}
