#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_TEXT_BETA
#define EFL_ACCESS_TEXT_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_entry_legacy_type_check)
{
   Evas_Object *win, *entry;
   const char *type;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   type = elm_object_widget_type_get(entry);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Entry"));

   type = evas_object_type_get(entry);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_entry"));

}
EFL_END_TEST

EFL_START_TEST(elm_entry_del)
{
   Evas_Object *win, *entry;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, "TEST");

}
EFL_END_TEST

void
_dummy_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED,
          const char *sig EINA_UNUSED, const char *src EINA_UNUSED)
{
}

EFL_START_TEST(elm_entry_signal_callback)
{
   Evas_Object *win, *entry;
   void *data;
   int k;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   for (k = 1; k < 10; k++)
     {
        int *val = malloc(sizeof(int));
        *val = 42 * k;
        elm_layout_signal_callback_add(entry, "sig", "src", _dummy_cb, val);
     }

   data = elm_layout_signal_callback_del(entry, "notsig", "notsrc", _dummy_cb);
   fail_if(data);

   // this test verifies that the legacy wrapper returns the proper data
   // the eo API requires the data as input to "del()"
   for (k = 1; k < 10; k++)
     {
        int *val;
        data = elm_layout_signal_callback_del(entry, "sig", "src", _dummy_cb);
        fail_if(!data);
        val = (int *)data;
        ck_assert_int_ne(*val, 0);
        ck_assert_int_eq((*val) % 42, 0);
        free(data);
     }

   data = elm_layout_signal_callback_del(entry, "sig", "src", _dummy_cb);
   fail_if(data);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_char_get)
{
   Evas_Object *win, *entry;
   Eina_Unicode *expected;
   Eina_Unicode val;

   const char *txt = "ĄA11Y Ł TEST";
   const char *mtxt = "<b>ĄA11Y</b> <title>Ł</> TEST";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   expected = eina_unicode_utf8_to_unicode(txt, NULL);

   val = efl_access_text_character_get(entry, -1);
   ck_assert(val == 0);

   val = efl_access_text_character_get(entry, 0);
   ck_assert(val == expected[0]);

   val = efl_access_text_character_get(entry, 1);
   ck_assert(val == expected[1]);

   val = efl_access_text_character_get(entry, 2);
   ck_assert(val == expected[2]);

   val = efl_access_text_character_get(entry, 6);
   ck_assert(val == expected[6]);

   val = efl_access_text_character_get(entry, 26);
   ck_assert(val == 0);

   free(expected);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_char_count)
{
   Evas_Object *win, *entry;
   int val;

   const char *mtxt = "<b>AĄ11Y</b> <title>Ł</> TEST";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, mtxt);

   val = efl_access_text_character_count_get(entry);
   ck_assert(val == 12);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_char)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end, &val);
   ck_assert_str_eq(val, "o");
   ck_assert(start == 1);
   ck_assert(end == 2);
   if (val) free(val);

   start = 8;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end, &val);
   ck_assert_str_eq(val, "ś");
   ck_assert(start == 8);
   ck_assert(end == 9);
   if (val) free(val);

   start = 11;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end, &val);
   ck_assert_str_eq(val, " ");
   ck_assert(start == 11);
   ck_assert(end == 12);
   if (val) free(val);

   start = 111;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_CHAR, &start, &end, &val);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_word)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end, &val);
   ck_assert_str_eq(val, "Lorem");
   ck_assert(start == 0);
   ck_assert(end == 5);
   if (val) free(val);

   start = 6;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end, &val);
   ck_assert_str_eq(val, "ipśum");
   ck_assert(start == 6);
   ck_assert(end == 11);
   if (val) free(val);

   start = 19;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end, &val);
   ck_assert_str_eq(val, "dolor");
   ck_assert(start == 14);
   ck_assert(end == 19);
   if (val) free(val);

   start = 111;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end, &val);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_paragraph)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit<br> amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   start = 1;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end, &val);
   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 20;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end, &val);
   ck_assert_str_eq(val, "   dolor sit");
   ck_assert(start == 12);
   ck_assert(end == 24);
   if (val) free(val);

   start = 25;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH, &start, &end, &val);
   ck_assert_str_eq(val, " amęt");
   ck_assert(start == 25);
   ck_assert(end == 30);
   if (val) free(val);

   start = 111;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_WORD, &start, &end, &val);
   ck_assert(start == -1);
   ck_assert(end == -1);
   ck_assert(val == NULL);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_string_get_line)
{
   Evas_Object *win, *entry;
   char *val;
   int start, end;

   const char *txt = "Lorem ipśum<br>   dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   evas_object_resize(entry, 500, 500);

   elm_object_text_set(entry, txt);

   start = 1;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_LINE, &start, &end, &val);

   ck_assert_str_eq(val, "Lorem ipśum");
   ck_assert(start == 0);
   ck_assert(end == 11);
   if (val) free(val);

   start = 13;
   efl_access_text_string_get(entry, EFL_ACCESS_TEXT_GRANULARITY_LINE, &start, &end, &val);
   ck_assert_str_eq(val, "   dolor sit amęt");
   ck_assert(start == 12);
   ck_assert(end == 29);
   if (val) free(val);

}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_text_get)
{
   Evas_Object *win, *entry;
   char *val;
   const char *txt = "Lorem Xpśum dolor sit amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   // invalid ranges
   val = efl_access_text_get(entry, 6, 100);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, -6, 10);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, -6, -10);
   ck_assert(val == NULL);
   val = efl_access_text_get(entry, 60, 100);
   ck_assert(val == NULL);

   // proper range
   val = efl_access_text_get(entry, 6, 17);
   ck_assert_str_eq(val, "Xpśum dolor");

   if (val) free(val);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_atspi_text_selections)
{
   Evas_Object *win, *entry;
   int val, start, end;
   const char *str;
   Eina_Bool ret;
   const char *txt = "Lorem ipśum<br>   dolor sit\n amęt";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, txt);

   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 0);

   elm_entry_select_region_set(entry, 2, 4);
   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 1);
   efl_access_text_access_selection_get(entry, 0, &start, &end);
   ck_assert(start == 2);
   ck_assert(end == 4);

   elm_entry_select_region_set(entry, 6, 10);
   val = efl_access_text_selections_count_get(entry);
   ck_assert(val == 1);
   efl_access_text_access_selection_get(entry, 0, &start, &end);
   ck_assert(start == 6);
   ck_assert(end == 10);

   elm_entry_select_none(entry);
   ret = efl_access_text_selection_add(entry, 2, 5);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert_str_eq(str, "rem");

   ret = efl_access_text_selection_remove(entry, 0);
   ck_assert(ret == EINA_TRUE);
   str = elm_entry_selection_get(entry);
   ck_assert(str == NULL);

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *entry;
   Efl_Access_Role role;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   role = efl_access_object_role_get(entry);

   ck_assert(role == EFL_ACCESS_ROLE_ENTRY);

}
EFL_END_TEST

static Eina_Bool
end_test()
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

EFL_START_TEST(elm_entry_magnifier)
{
   Evas_Object *win, *entry;
   Evas *e;
   char buf[4096];

   win = win_add_focused(NULL, "entry", ELM_WIN_BASIC);
   e = evas_object_evas_get(win);
   evas_object_size_hint_weight_set(win, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   entry = elm_entry_add(win);
   elm_entry_line_wrap_set(entry, ELM_WRAP_NONE);
   snprintf(buf, sizeof(buf),
            "This is an entry widget in this window that<br/>"
            "uses markup <b>like this</> for styling and<br/>"
            "formatting <em>like this</>, as well as<br/>"
            "<a href=X><link>links in the text</></a>, so enter text<br/>"
            "in here to edit it. By the way, links are<br/>"
            "called <a href=anc-02>Anchors</a> so you will need<br/>"
            "to refer to them this way.<br/>"
            "<br/>"

            "Also you can stick in items with (relsize + ascent): "
            "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
            " (full) "
            "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
            " (to the left)<br/>"

            "Also (size + ascent): "
            "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
            " (full) "
            "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
            " (before this)<br/>"

            "And as well (absize + ascent): "
            "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
            " (full) "
            "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
            " or even paths to image files on disk too like: "
            "<item absize=96x128 vsize=full href=file://%s/images/sky_01.jpg></item>"
            " ... end."
            , elm_app_data_dir_get()
            );
   elm_object_text_set(entry, buf);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(entry);
   elm_win_resize_object_add(win, entry);
   evas_object_show(win);
   evas_object_resize(entry, 600, 400);
   evas_object_resize(win, 600, 400);
   get_me_to_those_events(e);

   evas_event_feed_mouse_out(e, 0, NULL);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_event_feed_mouse_move(e, 200, 100, 0, NULL);
   evas_event_feed_mouse_down(e, 1, 0, 0, NULL);
   real_timer_add(elm_config_longpress_timeout_get() + 0.1, end_test, NULL);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_text_set)
{
   Evas_Object *win, *entry;
   const char *entry_text = "hello world";
   const char *entry_text2 = "scrollable";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   ck_assert(elm_layout_text_set(entry, NULL, entry_text));
   ck_assert_str_eq(elm_object_text_get(entry), entry_text);

   elm_entry_scrollable_set(entry, EINA_TRUE);
   ck_assert(elm_layout_text_set(entry, NULL, entry_text2));
   ck_assert_str_eq(elm_object_text_get(entry), entry_text2);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_guide_text_set)
{
   Evas_Object *win, *entry;
   const char *markup_text = "<font_size=32 color=#FFF>markup_test</font_size>";
   const char *markup_text2 = "<font_size=24 color=#FFF>test_markup</font_size>";

   win = win_add(NULL, "entry", ELM_WIN_BASIC);

   entry = elm_entry_add(win);

   elm_object_part_text_set(entry, "elm.guide", markup_text);
   elm_entry_editable_set(entry, EINA_FALSE);
   ck_assert_str_eq(elm_object_part_text_get(entry, "elm.guide"), markup_text);

   elm_object_part_text_set(entry, "elm.guide", markup_text2);
   elm_entry_single_line_set(entry, EINA_FALSE);
   ck_assert_str_eq(elm_object_part_text_get(entry, "elm.guide"), markup_text2);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_file_get_set)
{
   Evas_Object *win, *entry;
   const char *file_path = NULL;
   Elm_Text_Format format = ELM_TEXT_FORMAT_PLAIN_UTF8;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   entry = elm_entry_add(win);

   ck_assert(elm_entry_file_set(entry, TESTS_SRC_DIR"/testfile_entry.txt", ELM_TEXT_FORMAT_PLAIN_UTF8));
   elm_entry_file_get(entry, &file_path, &format);
   fprintf(stderr, "elm_entry_file_get_set1 %s, %s, %d\n", elm_object_text_get(entry), file_path, format);

   ck_assert_str_eq(elm_object_text_get(entry), "hello world<br/>");
   ck_assert_str_eq(file_path, TESTS_SRC_DIR"/testfile_entry.txt");
   ck_assert(format == ELM_TEXT_FORMAT_PLAIN_UTF8);

   ck_assert(elm_entry_file_set(entry, TESTS_SRC_DIR"/testfile_entry2.txt", ELM_TEXT_FORMAT_PLAIN_UTF8));
   elm_entry_file_get(entry, &file_path, &format);
   fprintf(stderr, "elm_entry_file_get_set2 %s, %s, %d\n", elm_object_text_get(entry), file_path, format);

   ck_assert_str_eq(elm_object_text_get(entry), "hello elementary<br/>hello entry<br/>");
   ck_assert_str_eq(file_path, TESTS_SRC_DIR"/testfile_entry2.txt");
   ck_assert(format == ELM_TEXT_FORMAT_PLAIN_UTF8);

   ck_assert(elm_entry_file_set(entry, NULL, ELM_TEXT_FORMAT_PLAIN_UTF8));
   elm_entry_file_get(entry, &file_path, &format);
   fprintf(stderr, "elm_entry_file_get_set3 %s, %s, %d\n", elm_object_text_get(entry), file_path, format);

   ck_assert_str_eq(elm_object_text_get(entry), "");
   ck_assert(file_path == NULL);
   ck_assert(format == ELM_TEXT_FORMAT_PLAIN_UTF8);

   fprintf(stderr, "elm_entry_file_get_set4\n");

}
EFL_END_TEST

EFL_START_TEST(elm_entry_test_text_class)
{
   Evas_Object *win, *entry1, *entry2, *entry3, *entry4;
   const char *filename = NULL;
   int w1 = 0, h1 = 0, w2 = 0, h2 = 0, w3 = 0, h3 = 0;
   const char *font;
   int font_size;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   entry1 = elm_entry_add(win);
   entry2 = elm_entry_add(win);
   entry3 = elm_entry_add(win);

   elm_object_text_set(entry1, "hello");
   elm_object_text_set(entry2, "hello");
   elm_object_text_set(entry3, "hello");

   edje_object_file_get(elm_layout_edje_get(entry1), &filename, NULL);
   ck_assert(filename != NULL);

   ck_assert(edje_file_text_class_set(filename, "entry_text", "Serif:Style=Bold", 24));

   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry1), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 24);
   ck_assert_str_eq(font, "Serif:Style=Bold");
   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry2), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 24);
   ck_assert_str_eq(font, "Serif:Style=Bold");
   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry3), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 24);
   ck_assert_str_eq(font, "Serif:Style=Bold");

   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry1), &w1, &h1);
   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry2), &w2, &h2);
   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry3), &w3, &h3);

   ck_assert_int_eq(w1, w2);
   ck_assert_int_eq(h1, h2);
   ck_assert_int_eq(w2, w3);
   ck_assert_int_eq(h2, h3);

   ck_assert(edje_object_text_class_set(elm_layout_edje_get(entry1), "entry_text", "Sans", 50));
   ck_assert(edje_object_text_class_set(elm_layout_edje_get(entry2), "entry_text", "Serif", 20));

   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry1), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 50);
   ck_assert_str_eq(font, "Sans");
   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry2), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 20);
   ck_assert_str_eq(font, "Serif");
   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry3), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 24);
   ck_assert_str_eq(font, "Serif:Style=Bold");

   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry1), &w1, &h1);
   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry2), &w2, &h2);
   evas_object_textblock_size_formatted_get(elm_entry_textblock_get(entry3), &w3, &h3);

   ck_assert_int_ne(w1, w2);
   ck_assert_int_ne(h1, h2);
   ck_assert_int_ne(w2, w3);
   ck_assert_int_ne(h2, h3);

   entry4 = elm_entry_add(win);

   elm_object_text_set(entry4, "hello");
   ck_assert(edje_object_text_class_get(elm_layout_edje_get(entry4), "entry_text", &font, &font_size));
   ck_assert_int_eq(font_size, 24);
   ck_assert_str_eq(font, "Serif:Style=Bold");
}
EFL_END_TEST

EFL_START_TEST(elm_entry_test_burmese)
{
   Evas_Object *win, *textblock;
   Evas_Textblock_Style *style;
   int w;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   textblock = evas_object_textblock_add(win);

   style = evas_textblock_style_new();
   evas_textblock_style_set(style, "DEFAULT='font=arial font_size=20 color=red'");
   evas_object_textblock_style_set(textblock, style);
   evas_textblock_style_free(style);
   style = NULL;

   evas_object_textblock_text_markup_set(textblock, "\u1006\u200C\u1031\u200C\u1031\u200C\u1031");
   evas_object_textblock_size_native_get(textblock, &w, NULL);
   ck_assert_int_ne(w, 0);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_keycode)
{
   Evas_Object *win, *entry;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   entry = elm_entry_add(win);
   evas_object_show(entry);

   elm_entry_entry_set(entry, "Hello World");
   Evas *e = evas_object_evas_get(entry);
   elm_object_focus_set(entry, EINA_TRUE);
   evas_key_modifier_on(e, "Control");
   evas_event_feed_key_down_with_keycode(e, "", "a", "", "", time(NULL), NULL, 38);
   ecore_main_loop_iterate();
   evas_event_feed_key_down(e, "BackSpace", "BackSpace", "\b", "\b", time(NULL), NULL);
   ck_assert_str_eq(elm_object_text_get(entry), "");

   evas_object_del(entry);
   evas_object_del(win);
}
EFL_END_TEST

EFL_START_TEST(elm_entry_textnodes_with_no_format)
{
   Evas_Object *win, *entry;

   win = win_add(NULL, "entry", ELM_WIN_BASIC);
   entry = elm_entry_add(win);
   evas_object_show(entry);

   elm_entry_entry_set(entry, "<br/><br/>");
   Evas_Object *tb = elm_entry_textblock_get(entry);
   Evas_Textblock_Cursor *c1 = evas_object_textblock_cursor_new(tb);
   Evas_Textblock_Cursor *c2 = evas_object_textblock_cursor_new(tb);
   evas_textblock_cursor_char_next(c2);
   evas_textblock_cursor_range_delete(c1, c2);
   elm_entry_cursor_pos_set(entry, 0);
   ck_assert(elm_entry_cursor_down(entry));

   evas_object_del(entry);
   evas_object_del(win);
}
EFL_END_TEST

void elm_test_entry(TCase *tc)
{
   tcase_add_test(tc, elm_entry_legacy_type_check);
   tcase_add_test(tc, elm_entry_del);
   tcase_add_test(tc, elm_entry_signal_callback);
   tcase_add_test(tc, elm_entry_atspi_text_char_get);
   tcase_add_test(tc, elm_entry_atspi_text_char_count);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_char);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_word);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_paragraph);
   tcase_add_test(tc, elm_entry_atspi_text_string_get_line);
   tcase_add_test(tc, elm_entry_atspi_text_text_get);
   tcase_add_test(tc, elm_entry_atspi_text_selections);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_entry_text_set);
   tcase_add_test(tc, elm_entry_guide_text_set);
   tcase_add_test(tc, elm_entry_magnifier);
   tcase_add_test(tc, elm_entry_file_get_set);
   tcase_add_test(tc, elm_entry_test_text_class);
   tcase_add_test(tc, elm_entry_test_burmese);
   tcase_add_test(tc, elm_entry_keycode);
   tcase_add_test(tc, elm_entry_textnodes_with_no_format);
}
