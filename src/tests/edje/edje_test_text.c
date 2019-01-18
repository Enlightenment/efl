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

   evas_free(evas);
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
   fail_unless(edje_object_file_set(obj, test_layout_get("test_textblock.edj"), "test_textblock"));
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, "Bye"));
   edje_object_part_text_set(obj, "text", buf);
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, buf));

   evas_free(evas);
}
EFL_END_TEST

START_TEST(edje_test_text_ellipsis)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_ellipsis_set(efl_part(layout, "text"), 1.0);

   evas_free(evas);
}
END_TEST

START_TEST(edje_test_text_wrap)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_wrap_set(efl_part(layout, "text"), EFL_TEXT_FORMAT_WRAP_WORD);

   evas_free(evas);
}
END_TEST

START_TEST(edje_test_text_font)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_font_set(efl_part(layout, "text"), "Sans", 14);

   evas_free(evas);
}
END_TEST

START_TEST(edje_test_text_color)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_normal_color_set(efl_part(layout, "text"), 255, 255, 255, 255);

   evas_free(evas);
}
END_TEST

static void
_basic_check(Eo *layout, Eina_Bool set)
{
   // Colors
     {
        unsigned char r, g, b, a;

        // Just normal_color is enough
        if (set)
          {
             efl_text_normal_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_backing_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_glow_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_glow2_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_outline_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_shadow_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_strikethrough_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_underline_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_underline2_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
             efl_text_underline_dashed_color_set(efl_part(layout, "text"),
                   255, 255, 255, 255);
          }

        efl_text_normal_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_backing_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_glow_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
        ck_assert_int_eq(r, 255);
        ck_assert_int_eq(g, 255);
        ck_assert_int_eq(b, 255);
        ck_assert_int_eq(a, 255);

        efl_text_glow2_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
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

        efl_text_underline2_color_get(efl_part(layout, "text"), &r, &g, &b, &a);
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
             efl_text_font_set(efl_part(layout, "text"), "Sans", 12);
          }

        wrap = efl_text_wrap_get(efl_part(layout, "text"));
        ck_assert_int_eq(wrap, EFL_TEXT_FORMAT_WRAP_WORD);

        ellipsis = efl_text_ellipsis_get(efl_part(layout, "text"));
        ck_assert(EINA_DBL_EQ(ellipsis, 1.0));

        efl_text_font_get(efl_part(layout, "text"), &font, &size);
        ck_assert_str_eq(font, "Sans");
        ck_assert_int_eq(size, 12);
     }
}

START_TEST(edje_test_text_part)
{
   Eo *evas;
   Eo *layout;

   evas = _setup_evas();

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));

   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));
   _basic_check(layout, EINA_TRUE);

   // Load again and check persistance
   efl_file_set(layout, test_layout_get("test_text.edj"), "test2");
   fail_if(efl_file_load_error_get(layout));
   _basic_check(layout, EINA_FALSE);

   evas_free(evas);
}
END_TEST

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
