/*
 * TODO:
 * * Test different font lodaing mechanisms.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>

#include "evas_suite.h"
#include "Evas.h"

#include "evas_tests_helpers.h"

/* Functions defined in evas_object_textblock2.c */
EAPI Eina_Bool
_evas_textblock2_check_item_node_link(Evas_Object *obj);
/* end of functions defined in evas_object_textblock2.c */

#define TEST_FONT "font=DejaVuSans font_source=" TESTS_SRC_DIR "/TestFont.eet"

#define _PS "\xE2\x80\xA9"

static const char *style_buf = TEST_FONT " font_size=10 color=#000 text_class=entry";

#define START_TB_TEST() \
   Evas *evas; \
   Evas_Object *tb; \
   Evas_Textblock2_Cursor *cur; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   tb = evas_object_textblock2_add(evas); \
   fail_if(!tb); \
   evas_object_textblock2_style_set(tb, style_buf); \
   cur = evas_object_textblock2_cursor_new(tb); \
do \
{ \
} \
while (0)

#define END_TB_TEST() \
do \
{ \
   evas_textblock2_cursor_free(cur); \
   evas_object_del(tb); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

START_TEST(evas_textblock2_simple)
{
   START_TB_TEST();
   const char *buf = "This " _PS _PS " is a \n test.";
   eo_do(tb, efl_text_set(buf));
   ck_assert_str_eq(buf, eo_do(tb, efl_text_get()));
   END_TB_TEST();
}
END_TEST

/* Various setters and getters */
START_TEST(evas_textblock2_set_get)
{
   START_TB_TEST();
   const char *buf = "";
   eo_do(tb, efl_text_set(buf));
   fail_if(strcmp(evas_object_textblock2_style_get(tb), style_buf));

   evas_object_textblock2_valign_set(tb, -1.0);
   fail_if(evas_object_textblock2_valign_get(tb) != 0.0);
   evas_object_textblock2_valign_set(tb, 0.0);
   fail_if(evas_object_textblock2_valign_get(tb) != 0.0);
   evas_object_textblock2_valign_set(tb, 0.432);
   fail_if(evas_object_textblock2_valign_get(tb) != 0.432);
   evas_object_textblock2_valign_set(tb, 1.0);
   fail_if(evas_object_textblock2_valign_get(tb) != 1.0);
   evas_object_textblock2_valign_set(tb, 1.5);
   fail_if(evas_object_textblock2_valign_get(tb) != 1.0);

   evas_object_textblock2_bidi_delimiters_set(tb, ",.|");
   fail_if(strcmp(evas_object_textblock2_bidi_delimiters_get(tb), ",.|"));
   evas_object_textblock2_bidi_delimiters_set(tb, ",|");
   fail_if(strcmp(evas_object_textblock2_bidi_delimiters_get(tb), ",|"));
   evas_object_textblock2_bidi_delimiters_set(tb, NULL);
   fail_if(evas_object_textblock2_bidi_delimiters_get(tb));
   evas_object_textblock2_bidi_delimiters_set(tb, ",|");
   fail_if(strcmp(evas_object_textblock2_bidi_delimiters_get(tb), ",|"));

   /* Hinting */
   eo_do(tb, efl_text_set("This is" _PS "a test\nbla"));
   /* Force relayout */
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_NONE);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_BYTECODE);
   END_TB_TEST();
}
END_TEST

/* Aux evas stuff, such as scale. */
START_TEST(evas_textblock2_evas)
{
   Evas_Coord w, h, sw, sh;
   START_TB_TEST();
   const char *buf = "Test";
   eo_do(tb, efl_text_set(buf));
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_scale_set(tb, 3.0);
   evas_object_textblock2_size_formatted_get(tb, &sw, &sh);
   fail_if((sw <= w) || (sh <= h));

   evas_object_scale_set(tb, 0.5);
   evas_object_textblock2_size_formatted_get(tb, &sw, &sh);
   fail_if((sw >= w) || (sh >= h));

   END_TB_TEST();
}
END_TEST

START_TEST(evas_textblock2_delete)
{
   START_TB_TEST();

   eo_do(tb, efl_text_set("a" _PS "b"));

   evas_textblock2_cursor_pos_set(cur, 1);
   evas_textblock2_cursor_char_delete(cur);
   ck_assert_str_eq("ab", eo_do(tb, efl_text_get()));

   evas_textblock2_cursor_char_delete(cur);
   ck_assert_str_eq("a", eo_do(tb, efl_text_get()));

   evas_textblock2_cursor_char_prev(cur);
   evas_textblock2_cursor_char_delete(cur);
   ck_assert_str_eq("", eo_do(tb, efl_text_get()));

   END_TB_TEST();
}
END_TEST;

void evas_test_textblock2(TCase *tc)
{
   tcase_add_test(tc, evas_textblock2_simple);
   tcase_add_test(tc, evas_textblock2_set_get);
   tcase_add_test(tc, evas_textblock2_evas);
   tcase_add_test(tc, evas_textblock2_delete);
}

