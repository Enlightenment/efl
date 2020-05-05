#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"

EFL_START_TEST(edje_test_text_cursor)
{
   Evas *evas;
   Evas_Object *obj;
   const char *buf = "ABC<br/>DEF";
   const char *txt;
   int i, old_pos, new_pos;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_text_cursor.edj"), "test_text_cursor"));
   edje_object_part_text_set(obj, "text", buf);
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, buf));

   edje_object_part_text_cursor_pos_set(obj, "text", EDJE_CURSOR_MAIN, 0);
   ck_assert_int_eq(edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN), 0);
   edje_object_part_text_cursor_pos_set(obj, "text", EDJE_CURSOR_MAIN, 1);
   ck_assert_int_eq(edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN), 1);

   /* Move cursor to the 0 pos from 1 pos */
   old_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert(edje_object_part_text_cursor_prev(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Move cursor to the -1 pos from 0 pos. It has to fail. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_prev(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Jump to 2nd line from 1st line.
    * It has to return EINA_TRUE which means success. */
   old_pos = new_pos;
   ck_assert(edje_object_part_text_cursor_down(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Try to jump to 3rd line from 2nd line. But, 3rd line does not exist.
    * So, it has to return EINA_FALSE which means failure. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_down(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Move cursor to the end of 2nd line. */
   for (i = 0; i < 3; i++)
     {
        old_pos = new_pos;
        ck_assert(edje_object_part_text_cursor_next(obj, "text", EDJE_CURSOR_MAIN));
        new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
        ck_assert_int_ne(old_pos, new_pos);
     }

   /* Move cursor to the next of the end of 2nd line which does not exist. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_next(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Jump to 1st line from 2nd line.
    * It has to return EINA_TRUE which means success. */
   old_pos = new_pos;
   ck_assert(edje_object_part_text_cursor_up(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Try to jump to the above of 1st line from 1st line. But, there is no such line.
    * So, it has to return EINA_FALSE which means failure. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_up(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

}
EFL_END_TEST


EFL_START_TEST(edje_test_textblock)
{
   Evas *evas;
   Evas_Object *obj;
   const char *buf = "Hello";
   const char *txt;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   ck_assert(edje_object_file_set(obj, test_layout_get("test_textblock.edj"), "test_textblock"));
   txt = edje_object_part_text_get(obj, "text");
   ck_assert_ptr_ne(txt, NULL);
   ck_assert_int_eq(strcmp(txt, "Bye"), 0);
   edje_object_part_text_set(obj, "text", buf);
   txt = edje_object_part_text_get(obj, "text");
   ck_assert_ptr_ne(txt, NULL);
   ck_assert_int_eq(strcmp(txt, buf), 0);

   Evas_Object *obj2 = edje_object_add(evas);
   ck_assert(edje_object_file_set(obj2, test_layout_get("test_textblock.edj"), "test_tc_textblock"));
   Evas_Object *tb = (Evas_Object*)edje_object_part_object_get(obj2, "tb");
   ck_assert_ptr_ne(tb, NULL);
   int w = 0, h = 0;
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   Evas_Textblock_Style *st = evas_object_textblock_style_get(tb);
   txt = evas_textblock_style_get(st);
   ck_assert_str_eq(txt, "DEFAULT='color=#ff0 font_size=18.0 font=Serif'");
   ck_assert_int_ne(w, 0);
   ck_assert_int_ne(h, 0);

   edje_object_text_class_set(obj2, "tc1", "Sans", 8);
   edje_object_calc_force(obj2);
   int w2 = 0, h2 = 0;
   evas_object_textblock_size_formatted_get(tb, &w2, &h2);
   ck_assert_int_ne(w2, 0);
   ck_assert_int_ne(h2, 0);
   ck_assert_int_ne(w, w2);
   ck_assert_int_ne(h, h2);

   ck_assert(edje_object_file_set(obj2, test_layout_get("test_textblock.edj"), "test_tc_textblock2"));
   tb = (Evas_Object*)edje_object_part_object_get(obj2, "tb2");
   ck_assert_ptr_ne(tb, NULL);
   st = evas_object_textblock_style_get(tb);
   txt = evas_textblock_style_get(st);
   ck_assert_str_eq(txt, "DEFAULT='color=#0ff'");
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, 0);
   ck_assert_int_eq(h, 0);

   edje_object_text_class_set(obj2, "tc2", "Sans", 15);
   edje_object_calc_force(obj2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_ne(w, 0);
   ck_assert_int_ne(h, 0);
   st = evas_object_textblock_style_get(tb);
   txt = evas_textblock_style_get(st);
   ck_assert_str_eq(txt, "DEFAULT='color=#0ff font_size=15.0 font=Sans'");
}
EFL_END_TEST

EFL_START_TEST(edje_test_text_ellipsis)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 40)));
   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test");
   ck_assert(!efl_file_load(layout));

   efl_text_ellipsis_set(efl_part(layout, "text"), 1.0);

}
EFL_END_TEST

EFL_START_TEST(edje_test_text_wrap)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 40)));
   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test");
   ck_assert(!efl_file_load(layout));

   efl_text_wrap_set(efl_part(layout, "text"), EFL_TEXT_FORMAT_WRAP_WORD);

}
EFL_END_TEST

EFL_START_TEST(edje_test_text_font)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 40)));
   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test");
   ck_assert(!efl_file_load(layout));

   efl_text_font_family_set(efl_part(layout, "text"), "Sans");
   efl_text_font_size_set(efl_part(layout, "text"), 14);

}
EFL_END_TEST

EFL_START_TEST(edje_test_text_color)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 40)));
   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test");
   ck_assert(!efl_file_load(layout));

   efl_text_color_set(efl_part(layout, "text"), 255, 255, 255, 255);

}
EFL_END_TEST

static void
_basic_check(Eo *layout, Eina_Bool set)
{
   // Colors
     {
        unsigned char r, g, b, a;

        // Just normal_color is enough
        if (set)
          {
             efl_text_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_background_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_glow_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_secondary_glow_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_outline_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_shadow_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_strikethrough_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_underline_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_secondary_underline_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_underline_dashed_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
          }

        efl_text_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_background_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_glow_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_secondary_glow_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_outline_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_shadow_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_strikethrough_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_underline_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_secondary_underline_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_underline_dashed_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);
     }

   // Rest of api
     {
        Efl_Text_Format_Wrap wrap;
        double ellipsis;
        const char *font;
        Efl_Font_Size size;

        if (set)
          {
             efl_text_wrap_set(efl_part(layout, "text"), EFL_TEXT_FORMAT_WRAP_WORD);
             efl_text_ellipsis_set(efl_part(layout, "text"), 1.0);
             efl_text_font_family_set(efl_part(layout, "text"), "Sans");
             efl_text_font_size_set(efl_part(layout, "text"), 12);
          }

        wrap = efl_text_wrap_get(efl_part(layout, "text"));
        ck_assert_int_eq(wrap, EFL_TEXT_FORMAT_WRAP_WORD);

        ellipsis = efl_text_ellipsis_get(efl_part(layout, "text"));
        ck_assert(EINA_DBL_EQ(ellipsis, 1.0));

        font = efl_text_font_family_get(efl_part(layout, "text"));
        size = efl_text_font_size_get(efl_part(layout, "text"));
        ck_assert_str_eq(font, "Sans");
        ck_assert_int_eq(size, 12);
     }
}

EFL_START_TEST(edje_test_text_part)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 40)));

   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test");
   ck_assert(!efl_file_load(layout));
   _basic_check(layout, EINA_TRUE);

   // Load again and check persistance
   ck_assert(!efl_file_set(layout, test_layout_get("test_text.edj")));
   efl_file_key_set(layout, "test2");
   ck_assert(!efl_file_load(layout));
   _basic_check(layout, EINA_FALSE);

}
EFL_END_TEST

void edje_test_text(TCase *tc)
{
   tcase_add_test(tc, edje_test_text_cursor);
   tcase_add_test(tc, edje_test_textblock);
   tcase_add_test(tc, edje_test_text_ellipsis);
   tcase_add_test(tc, edje_test_text_wrap);
   tcase_add_test(tc, edje_test_text_font);
   tcase_add_test(tc, edje_test_text_color);
   tcase_add_test(tc, edje_test_text_part);
}
