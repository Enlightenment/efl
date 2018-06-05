#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include <Eina.h>
#include <Edje.h>

#include "edje_suite.h"
#include "edje_tests_helpers.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"

static const char *
test_layout_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_BUILD_DIR"/data/%s", name);

   return filename;
}

START_TEST(edje_test_text_ellipsis)
{
   Eo *evas = EDJE_TEST_INIT_EVAS();
   Eo *layout;

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_ellipsis_set(efl_part(layout, "text"), 1.0);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_text_wrap)
{
   Eo *evas = EDJE_TEST_INIT_EVAS();
   Eo *layout;

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_wrap_set(efl_part(layout, "text"), EFL_TEXT_FORMAT_WRAP_WORD);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_text_font)
{
   Eo *evas = EDJE_TEST_INIT_EVAS();
   Eo *layout;

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_font_set(efl_part(layout, "text"), "Sans", 14);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_text_color)
{
   Eo *evas = EDJE_TEST_INIT_EVAS();
   Eo *layout;

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));
   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));

   efl_text_normal_color_set(efl_part(layout, "text"), 255, 255, 255, 255);

   EDJE_TEST_FREE_EVAS();
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
   Eo *evas = EDJE_TEST_INIT_EVAS();
   Eo *layout;

   layout = efl_add(EFL_CANVAS_LAYOUT_CLASS, evas,
         efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 40)));

   efl_file_set(layout, test_layout_get("test_text.edj"), "test");
   fail_if(efl_file_load_error_get(layout));
   _basic_check(layout, EINA_TRUE);

   // Load again and check persistance
   efl_file_set(layout, test_layout_get("test_text.edj"), "test2");
   fail_if(efl_file_load_error_get(layout));
   _basic_check(layout, EINA_FALSE);

   EDJE_TEST_FREE_EVAS();
}
END_TEST
void edje_test_text(TCase *tc)
{
   tcase_add_test(tc, edje_test_text_ellipsis);
   tcase_add_test(tc, edje_test_text_wrap);
   tcase_add_test(tc, edje_test_text_font);
   tcase_add_test(tc, edje_test_text_color);
   tcase_add_test(tc, edje_test_text_part);
}
