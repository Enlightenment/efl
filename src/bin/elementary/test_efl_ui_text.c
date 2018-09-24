#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EO_BETA_API
#include <Elementary.h>

// 1. Label-equivalent setup

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
   elm_box_pack_end(bx, en);
   return en;
}

void
test_efl_ui_text_label(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx;
   Eo *en;
   char *markup;

   win = elm_win_util_standard_add("label", "Label");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = _create_label(win, bx);
   efl_text_set(en, "This is a\t small label");
   //                012345678901234567890
   _apply_style(en, 0, 21, "font_size=12 font_weight=bold");
   efl_text_halign_set(en, 0.5);
   efl_text_font_weight_set(en, EFL_TEXT_FONT_WEIGHT_BOLD);

   en = _create_label(win, bx);
   efl_text_halign_set(en, 0.5);
   efl_text_set(en, "This is a text. Is also has\n"
         "newlines. There are several styles applied.");
   _apply_style(en, 40, 45, "font_weight=bold color=#ff0");
   _apply_style(en, 52, 58, "font_weight=italic color=#f00");
   efl_text_multiline_set(en, EINA_TRUE);

   en = _create_label(win, bx);
   efl_text_halign_set(en, 0.5);
   efl_text_set(en, "By default 'multiline' is disabled.\n"
         "So, \\n would only work if you enable it.");

   en = _create_label(win, bx);
   efl_text_set(en, "You can input text here.");
   efl_text_font_set(en, "Sans", 14);
   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   en = _create_label(win, bx);
   efl_text_set(en, "Input multiline here.");
   efl_text_font_set(en, "Sans", 14);
   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   en = _create_label(win, bx);
   efl_text_markup_set(en, "You can also <b>ENTER</b> markup!");
   efl_text_font_set(en, "Sans", 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_text_to_markup(EFL_TEXT_MARKUP_UTIL_CLASS,
         "You can use the text -> markup helper\nto make coding easier.");
   efl_text_markup_set(en, markup);
   free(markup);
   efl_text_font_set(en, "Sans", 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   en = _create_label(win, bx);
   markup = efl_text_markup_util_markup_to_text(EFL_TEXT_MARKUP_UTIL_CLASS,
         "You can use markup -&gt; text helpers<ps>to make coding easier.");
   efl_text_set(en, markup);
   free(markup);
   efl_text_font_set(en, "Sans", 14);
   efl_text_wrap_set(en, EFL_TEXT_FORMAT_WRAP_WORD);
   efl_text_multiline_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.5);

   evas_object_resize(win, 480, 480);
   evas_object_show(win);
}

// 2. Entry-equivalent setup

typedef struct
{
   const char *wrap_mode[4];
   size_t cur_wrap;
} Test_Data;

static void
my_efl_ui_text_bt_3(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Text_Cursor_Cursor *sel_start, *sel_end;

   efl_text_interactive_selection_cursors_get(data, &sel_start, &sel_end);
   const char *s = efl_canvas_text_range_text_get(data, sel_start, sel_end);

   printf("SELECTION REGION: %d - %d\n",
         efl_text_cursor_position_get(obj, sel_start),
         efl_text_cursor_position_get(obj, sel_end));
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
}

static void
my_efl_ui_text_bt_4(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   efl_text_cursor_item_insert(en, efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_MAIN),
         "emoticon/evil-laugh", "size=32x32");
}

static void
my_efl_ui_text_bt_6(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
   Efl_Text_Cursor_Cursor *main_cur, *cur;
   char buf[128];

   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = efl_add(EFL_UI_TEXT_CLASS, win,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   printf("Added Efl.Ui.Text object\n");
   efl_key_data_set(en, "wrap_idx", 0);
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");
   efl_text_font_set(en, "Sans", 14);
   efl_text_font_weight_set(en, EFL_TEXT_FONT_WEIGHT_BOLD);
   efl_text_font_slant_set(en, EFL_TEXT_FONT_SLANT_ITALIC);
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_normal_color_set(en, 255, 255, 255, 255);

   main_cur = efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_MAIN);
   cur = efl_text_cursor_new(en);

   efl_text_cursor_position_set(en, cur, 2);
   efl_text_cursor_item_insert(en, cur, "emoticon/happy", "size=32x32");
   efl_text_cursor_position_set(en, cur, 50);
   snprintf(buf, sizeof(buf), "file://%s/images/sky_01.jpg", elm_app_data_dir_get());
   efl_text_cursor_item_insert(en, cur, buf, "size=32x32");

   efl_text_cursor_position_set(en, main_cur, 5);
   efl_text_cursor_position_set(en, cur, 20);

   efl_text_annotation_insert(en, main_cur, cur, "a href=#hello");

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   elm_box_pack_end(bx, en);
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

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(bx, bx3);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx3);
   evas_object_show(bx2);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);
}

void
test_efl_ui_text_async(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *bx3, *en;

   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = efl_add(EFL_UI_TEXT_CLASS, win,
         efl_text_wrap_set(efl_added, EFL_TEXT_FORMAT_WRAP_WORD),
         efl_text_multiline_set(efl_added, EINA_TRUE)
         );

   printf("Added Efl.Ui.Text object\n");
   efl_key_data_set(en, "wrap_idx", 0);
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");
   efl_text_font_set(en, "Sans", 10);
   efl_text_normal_color_set(en, 255, 255, 255, 255);

   elm_box_pack_end(bx, en);
   elm_object_focus_set(en, EINA_TRUE);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(bx, bx3);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx3);
   evas_object_show(bx2);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);
}

#define IMAGES_SZ 5

static const char *images[IMAGES_SZ] = {
     "sky", "logo", "dog", "eet_rock", "eet_plant" };

static void
my_efl_ui_text_item_factory_bt_image(void *data, Evas_Object *obj EINA_UNUSED,
      void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   static int image_idx = 0;

   image_idx = (image_idx + 1) % IMAGES_SZ;
   efl_text_cursor_item_insert(en,
         efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_MAIN),
         images[image_idx], "size=32x32");
   printf("Inserted image: key = %s\n", images[image_idx]);
}

static void
my_efl_ui_text_item_factory_bt_emoticon(void *data, Evas_Object *obj EINA_UNUSED,
      void *event_info EINA_UNUSED)
{
   Evas_Object *en = data;
   efl_text_cursor_item_insert(en, efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_MAIN),
         "emoticon/evil-laugh", "size=32x32");
}

static struct
{
   const char *name;
   Eo *item_factory;
} factories[3];

static void
my_efl_ui_text_item_factory_bt_change(void *data, Evas_Object *obj EINA_UNUSED,
      void *event_info EINA_UNUSED)
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
   Evas_Object *win, *bx, *bx2, *bx3, *bt, *en;
   Efl_Text_Cursor_Cursor *main_cur, *cur;
   char buf[128];
   Eina_File *f;

   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = efl_add(EFL_UI_TEXT_CLASS, win,
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

   main_cur = efl_text_cursor_get(en, EFL_TEXT_CURSOR_GET_MAIN);
   cur = efl_text_cursor_new(en);

   efl_text_cursor_position_set(en, cur, 2);
   efl_text_cursor_item_insert(en, cur, "emoticon/happy", "size=32x32");
   efl_text_cursor_position_set(en, cur, 50);

   snprintf(buf, sizeof(buf), "file://%s/images/sky_01.jpg", elm_app_data_dir_get());
   efl_text_cursor_item_insert(en, cur, buf, "size=32x32");
   efl_text_cursor_position_set(en, main_cur, 5);

   efl_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   elm_box_pack_end(bx, en);
   elm_object_focus_set(en, EINA_TRUE);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Image");
   evas_object_smart_callback_add(bt, "clicked",
         my_efl_ui_text_item_factory_bt_image, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Emoticon");
   evas_object_smart_callback_add(bt, "clicked",
         my_efl_ui_text_item_factory_bt_emoticon, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Factory");
   evas_object_smart_callback_add(bt, "clicked",
         my_efl_ui_text_item_factory_bt_change, en);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   elm_object_focus_allow_set(bt, EINA_FALSE);
   evas_object_show(bt);

   bx3 = elm_box_add(win);
   elm_box_horizontal_set(bx3, EINA_TRUE);
   evas_object_size_hint_weight_set(bx3, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx3, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(bx, bx3);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx3);
   evas_object_show(bx2);

   evas_object_resize(win, 480, 320);
   evas_object_show(win);
}
