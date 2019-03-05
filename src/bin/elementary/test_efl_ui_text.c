#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Elementary.h>

static void
_apply_style(Eo *obj, size_t start_pos, size_t end_pos, const char *style)
{
   Efl_Text_Cursor_Cursor *start, *end;

   start = efl_text_cursor_new(obj);
   end = efl_text_cursor_new(obj);

   efl_text_cursor_position_set(obj, start, start_pos);
   efl_text_cursor_position_set(obj, end, end_pos);

   efl_text_annotation_insert(obj, start, end, style);

   efl_text_cursor_free(obj, start);
   efl_text_cursor_free(obj, end);
}

static Eo *
_create_label(Eo *win, Eo *bx)
{
   Eo *en;
   en = efl_add(EFL_UI_TEXT_CLASS, win);
   printf("Added Efl.Ui.Text object\n");
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

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
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
   efl_text_font_set(en, "Sans", 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_text_to_markup(
         "You can use the text -> markup helper\nto make coding easier.");
   efl_text_markup_set(en, markup);
   free(markup);
   efl_text_font_set(en, "Sans", 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_markup_to_text(
         "You can use markup -&gt; text helpers<ps>to make coding easier.");
   efl_text_set(en, markup);
   free(markup);
   efl_text_font_set(en, "Sans", 14);
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
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;
   Eo *en = data;

   efl_text_interactive_selection_cursors_get(data, &sel_start, &sel_end);
   const char *s = efl_canvas_text_range_text_get(data, sel_start, sel_end);

   printf("SELECTION REGION: %d - %d\n",
         efl_text_cursor_position_get(en, sel_start),
         efl_text_cursor_position_get(en, sel_end));
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
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
         efl_text_set(efl_added, "Efl Ui Text"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(bx, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_content_set(win, bx);

   en = efl_add(EFL_UI_TEXT_CLASS, bx,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   printf("Added Efl.Ui.Text object\n");

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);

   efl_text_font_set(en, "Sans", 12);
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_normal_color_set(en, 255, 255, 255, 255);

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
   efl_ui_direction_set(bx2, EFL_UI_DIR_HORIZONTAL);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Sel");
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _on_bt3_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Wr");
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _on_bt6_clicked, en);
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
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
         efl_text_set(efl_added, "Efl Ui Text Input Field"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(bx, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_content_set(win, bx);

   en = efl_add(EFL_UI_TEXT_CLASS, bx,
         efl_text_multiline_set(efl_added, EINA_FALSE));

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);

   printf("Added Efl.Ui.Text object\n");
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_set(efl_part(en, "efl.text_guide"), "Enter you name here");

   efl_pack(bx, en);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a multiline input.\n"
         "Enter multiline here");
   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);
   efl_pack(bx, en);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 200));

}

#define IMAGES_SZ 5

static const char *images[IMAGES_SZ] = {
     "sky", "logo", "dog", "eet_rock", "eet_plant" };

static void
_on_factory_bt_image_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *en = data;
   static int image_idx = 0;

   image_idx = (image_idx + 1) % IMAGES_SZ;
   efl_text_cursor_item_insert(en,
         efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_TYPE_MAIN),
         images[image_idx], "size=32x32");
   printf("Inserted image: key = %s\n", images[image_idx]);
}

static void
_on_factory_bt_emoticon_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *en = data;
   efl_text_cursor_item_insert(en, efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_TYPE_MAIN),
         "emoticon/evil-laugh", "size=32x32");
}

static struct
{
   const char *name;
   Eo *item_factory;
} factories[3];

static void
_on_factory_bt_factory_clicked(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *en = data;
   static int item_factory_idx = 0;

   item_factory_idx = (item_factory_idx + 1) % 3;
   efl_ui_text_item_factory_set(en, factories[item_factory_idx].item_factory);
   printf("Factory set to: %s\n", factories[item_factory_idx].name);
}

#define FACTORY_NONE     0
#define FACTORY_IMAGE    1
#define FACTORY_EMOTICON 2

void
test_ui_text_item_factory(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *bt, *en;
   Efl_Text_Cursor_Cursor *main_cur, *cur;
   char buf[128];
   Eina_File *f;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
         efl_text_set(efl_added, "Efl Ui Text Item Factory"),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, bx);

   en = efl_add(EFL_UI_TEXT_CLASS, bx,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   factories[FACTORY_NONE].name = "None (Fallback)";
   factories[FACTORY_NONE].item_factory = NULL;

   factories[FACTORY_IMAGE].name = "Image Factory";
   factories[FACTORY_IMAGE].item_factory =
      efl_add(EFL_UI_TEXT_FACTORY_IMAGES_CLASS, en);

   factories[FACTORY_EMOTICON].name = "Emoticon Factory";
   factories[FACTORY_EMOTICON].item_factory =
      efl_add(EFL_UI_TEXT_FACTORY_EMOTICONS_CLASS, en);

   // Test assigning file path source
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   efl_ui_text_factory_images_matches_add(factories[FACTORY_IMAGE].item_factory,
         images[0], buf, NULL);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   efl_ui_text_factory_images_matches_add(factories[FACTORY_IMAGE].item_factory,
         images[1], buf, NULL);
   snprintf(buf, sizeof(buf), "%s/images/mystrale.jpg", elm_app_data_dir_get());
   efl_ui_text_factory_images_matches_add(factories[FACTORY_IMAGE].item_factory,
         images[2], buf, NULL);

   // Open EET source w/ key
   snprintf(buf, sizeof(buf), "%s/images/image_items.eet", elm_app_data_dir_get());
   f = eina_file_open(buf, EINA_FALSE);
   if (f)
     {
        efl_ui_text_factory_images_matches_mmap_add(
              factories[FACTORY_IMAGE].item_factory,
              "eet_rock", f, "rock");
        efl_ui_text_factory_images_matches_mmap_add(
              factories[FACTORY_IMAGE].item_factory,
              "eet_plant", f, "plant");
        eina_file_close(f);
     }
   else
     {
        printf("Error loading test file. Please review test.");
     }


   printf("Added Efl.Ui.Text object\n");
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");
   efl_text_font_set(en, "Sans", 14);
   efl_text_normal_color_set(en, 255, 255, 255, 255);

   main_cur = efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_TYPE_MAIN);
   cur = efl_text_cursor_new(en);

   efl_text_cursor_position_set(en, cur, 2);
   efl_text_cursor_item_insert(en, cur, "emoticon/happy", "size=32x32");
   efl_text_cursor_position_set(en, cur, 50);

   snprintf(buf, sizeof(buf), "file://%s/images/sky_01.jpg", elm_app_data_dir_get());
   efl_text_cursor_item_insert(en, cur, buf, "size=32x32");
   efl_text_cursor_position_set(en, main_cur, 5);

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   efl_pack(bx, en);
   elm_object_focus_set(en, EINA_TRUE);

   bx2 = efl_add(EFL_UI_BOX_CLASS, bx);
   efl_gfx_hint_weight_set(bx2, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_ui_direction_set(bx2, EFL_UI_DIR_HORIZONTAL);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Image");
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _on_factory_bt_image_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Emoticon");
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _on_factory_bt_emoticon_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   bt = efl_add(EFL_UI_BUTTON_CLASS, bx2);
   efl_text_set(bt, "Factory");
   efl_event_callback_add(bt, EFL_UI_EVENT_CLICKED, _on_factory_bt_factory_clicked, en);
   efl_gfx_hint_weight_set(bt, EFL_GFX_HINT_EXPAND, 0.0);
   efl_pack(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);

   efl_pack(bx, bx2);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(480, 320));
}
