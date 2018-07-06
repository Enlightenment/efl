
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

#define TEST_FONT_DIR TESTS_SRC_DIR "/fonts/"
#define TEST_FONT_NAME "DejaVuSans,UnDotum"
#define TEST_FONT_SOURCE TEST_FONT_DIR "TestFont.eet"
#define TEST_FONT_TTF_NAME "Evas Test Font"

#define START_TEXT_TEST() \
   Evas *evas; \
   Evas_Object *to; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   to = evas_object_text_add(evas); \
   evas_object_text_font_source_set(to, TEST_FONT_SOURCE); \
do \
{ \
} \
while (0)

#define END_TEXT_TEST() \
do \
{ \
   evas_object_del(to); \
   evas_free(evas); \
} \
while (0)

EFL_START_TEST(evas_text_simple)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   evas_object_text_text_set(to, buf);
   fail_if(strcmp(evas_object_text_text_get(to), buf));
   END_TEXT_TEST();
}
EFL_END_TEST

/* Various text related geometries */
EFL_START_TEST(evas_text_geometries)
{
   START_TEXT_TEST();
   const char *buf = "Tests";
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;
   Evas_Coord prev;
   int i;
   Evas_Coord x, y, w, h, px;

   evas_object_text_text_set(to, buf);

   /* All should be 0 without a font set */
   fail_if(evas_object_text_ascent_get(to) != 0);
   fail_if(evas_object_text_descent_get(to) != 0);
   fail_if(evas_object_text_max_ascent_get(to) != 0);
   fail_if(evas_object_text_max_descent_get(to) != 0);
   fail_if(evas_object_text_horiz_advance_get(to) != 0);
   fail_if(evas_object_text_vert_advance_get(to) != 0);

   evas_object_text_font_set(to, font, size);

   /* Check that they are bigger than 0. */
   fail_if(evas_object_text_ascent_get(to) <= 0);
   fail_if(evas_object_text_descent_get(to) <= 0);
   fail_if(evas_object_text_max_ascent_get(to) <= 0);
   fail_if(evas_object_text_max_descent_get(to) <= 0);
   fail_if(evas_object_text_horiz_advance_get(to) <= 0);
   fail_if(evas_object_text_vert_advance_get(to) <= 0);

   /* Check that expanding the text does what we expect it */
   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_ascent_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_ascent_get(to) != prev);

   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_descent_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_descent_get(to) != prev);

   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_max_ascent_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_max_ascent_get(to) != prev);

   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_max_descent_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_max_descent_get(to) != prev);

   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_horiz_advance_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_horiz_advance_get(to) <= prev);

   evas_object_text_text_set(to, "Test");
   prev = evas_object_text_vert_advance_get(to);
   evas_object_text_text_set(to, "Testing");
   fail_if(evas_object_text_vert_advance_get(to) != prev);

   /* Go through all the characters, making sure the geometries we get
    * are in a monotonically increasing order and that all sizes are
    * bigger than 0. */
   evas_object_text_text_set(to, "Testing...");
   x = 0;
   px = -100;
   for (i = 0 ; i < eina_unicode_utf8_get_len("Testing...") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x <= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 4),
                 y + (h / 2), NULL, NULL, NULL, NULL));
        /* Get back cursor position, if click on right half of char. */
        fail_if((i + 1) != evas_object_text_char_coords_get(to, x + ((3 * w) / 4),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* Last up to pos */
   Evas_Coord adv;
   int pos, prev_pos;
   evas_object_text_text_set(to, "Test - 유니코드");
   adv = evas_object_text_horiz_advance_get(to);
   pos = prev_pos = 0;
   for (x = 0 ; x <= (adv - 1) ; x++)
     {
        pos = evas_object_text_last_up_to_pos(to, x, 0);
        _ck_assert_int(pos, >=, prev_pos);
        prev_pos = pos;
     }
   pos = evas_object_text_last_up_to_pos(to, x, 0);
   ck_assert_int_eq(pos, -1);
   pos = evas_object_text_last_up_to_pos(to, -50, 0);
   ck_assert_int_eq(pos, -1);

   /* Obviously, adv > width case */
   evas_object_text_text_set(to, "こんにちは。");
   adv = evas_object_text_horiz_advance_get(to);
   evas_object_geometry_get(to, NULL, NULL, &w, NULL);
   ck_assert_int_lt(w, adv);

#ifdef HAVE_HARFBUZZ
   // FIXME: missing a font for Oriya script in our test suite
   ///* Obviously, adv < width case */
   //evas_object_text_text_set(to, "ନୂଁ");
   //adv = evas_object_text_horiz_advance_get(to);
   //evas_object_geometry_get(to, NULL, NULL, &w, NULL);
   //ck_assert_int_lt(adv, w);
#endif

   END_TEXT_TEST();
}
EFL_END_TEST

/* Various evas stuff, such as scale */
EFL_START_TEST(evas_text_evas)
{
   Evas_Coord w, h, bw, bh;
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;

   evas_object_text_font_set(to, font, size);
   evas_object_text_text_set(to, buf);
   evas_object_geometry_get(to, NULL, NULL, &bw, &bh);
   evas_object_scale_set(to, 3.0);
   evas_object_geometry_get(to, NULL, NULL, &w, &h);
   fail_if((w <= bw) || (h <= bh));

   evas_object_scale_set(to, 0.5);
   evas_object_geometry_get(to, NULL, NULL, &w, &h);
   fail_if((w >= bw) || (h >= bh));

   END_TEXT_TEST();
}
EFL_END_TEST

static void
_test_ellipsis(Evas_Object *to, const char *buf, const char *font, Evas_Font_Size size, double ellipsis)
{
   Evas_Coord text_width;
   Evas_Coord w, h;

   evas_object_text_ellipsis_set(to, ellipsis);
   evas_object_move(to, 0, 0);
   evas_object_text_font_set(to, font, size);
   evas_object_text_text_set(to, buf);
   /* Besure to give a enough width for text. */
   evas_object_resize(to, 500, 500);
   /* Because of the way text object behaves, this will actually force
    * a resize. */
   evas_object_geometry_get(to, NULL, NULL, &w, NULL);

   text_width = w;
   /* Force a resize. */
   evas_object_resize(to, text_width - 1, 500);
   evas_object_geometry_get(to, NULL, NULL, &w, NULL);
   ck_assert_int_ne(text_width, w);

   /* Make it smaller to force ellipsis and check the resulting size. */
   evas_object_geometry_get(to, NULL, NULL, NULL, &h);
   evas_object_resize(to, 140, h);

   /* Force a resize. */
   evas_object_geometry_get(to, NULL, NULL, &w, NULL);
   /* If it's gotten way too small, it means we have an issue. */
   fail_if(w < 100);

   w = evas_object_text_horiz_advance_get(to);
   fail_if(w < 100);
}

EFL_START_TEST(evas_text_ellipsis)
{
   START_TEXT_TEST();
   const char *buf = "נסיון בלה בלה בלה בלה";
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;

   /* Test various ellipsis types. */

   /* RTL */
   _test_ellipsis(to, buf, font, size, 0.0);
   _test_ellipsis(to, buf, font, size, 0.5);
   _test_ellipsis(to, buf, font, size, 1.0);

   /* BiDi */
   buf = "Test נסיון בלה בלה בלה";
   _test_ellipsis(to, buf, font, size, 0.0);
   _test_ellipsis(to, buf, font, size, 0.5);
   _test_ellipsis(to, buf, font, size, 1.0);

   /* LTR */
   buf = "Test test test test test";
   _test_ellipsis(to, buf, font, size, 0.0);
   _test_ellipsis(to, buf, font, size, 0.5);
   _test_ellipsis(to, buf, font, size, 1.0);

   /* Ligatures */
   buf = "Fffffffffffffffffffffffffffffffffff";
   _test_ellipsis(to, buf, font, size, 0.0);

   /* Obviously, adv > width case */
   buf = "This is a test for adv > width case. こんにちは。";
   _test_ellipsis(to, buf, font, size, 0.0);
   _test_ellipsis(to, buf, font, size, 0.5);
   _test_ellipsis(to, buf, font, size, 1.0);

   /* Obviously, adv < width case */
   buf = "This is a test for adv < width case. ନୂଁ";
   _test_ellipsis(to, buf, font, size, 0.0);
   _test_ellipsis(to, buf, font, size, 0.5);
   _test_ellipsis(to, buf, font, size, 1.0);

   /* Check ellipsis value with NULL */
   fail_if(evas_object_text_ellipsis_get(NULL) != -1.0);

   END_TEXT_TEST();
}
EFL_END_TEST

/* Tests for functions that are in evas_object_text.c but
 * don't really have anything to do with it. */
EFL_START_TEST(evas_text_unrelated)
{
   START_TEXT_TEST();
   const char *buf = "נסיון";
   int pos;
   int value;
   /* Actually, they are tested in eina, just doing it for completeness. */
   fail_if(evas_string_char_len_get(buf) != 5);

   pos = 0;
   fail_if(2 != evas_string_char_next_get(buf, pos, &value));
   fail_if(value != 1504);

   pos = 2;
   fail_if(0 != evas_string_char_prev_get(buf, pos, &value));
   fail_if(value != 1505);

   END_TEXT_TEST();
}
EFL_END_TEST

#define _CHECK_SET_GET(x) \
do \
{ \
   Evas_Coord r, g, b, a; \
   evas_object_text_##x##_set(to, 100, 150, 125, 12); \
   evas_object_text_##x##_get(to, &r, &g, &b, &a); \
   fail_if((r != 100) || (g != 150) || (b != 125) || (a != 12)); \
   /* Set to the same value */ \
   evas_object_text_##x##_set(to, 100, 150, 125, 12); \
   evas_object_text_##x##_get(to, &r, &g, &b, &a); \
   fail_if((r != 100) || (g != 150) || (b != 125) || (a != 12)); \
} \
while (0)

EFL_START_TEST(evas_text_set_get)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   /* Text */
   evas_object_text_text_set(to, buf);
   fail_if(strcmp(evas_object_text_text_get(to), buf));

   /* Colors */
   _CHECK_SET_GET(shadow_color);
   _CHECK_SET_GET(glow_color);
   _CHECK_SET_GET(glow2_color);
   _CHECK_SET_GET(outline_color);

   /* Font and size */
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;
   evas_object_text_font_set(to, font, size);
   font = NULL;
   size = 0;
   evas_object_text_font_get(to, &font, &size);
   fail_if(strcmp(font, TEST_FONT_NAME));
   fail_if(size != 14);
   evas_object_text_font_set(to, font, size);
   font = NULL;
   size = 0;
   evas_object_text_font_get(to, &font, &size);
   fail_if(strcmp(font, TEST_FONT_NAME));
   fail_if(size != 14);

   font = "NON-EXISTING-FONT";
   size = 14;
   evas_object_text_font_set(to, font, size);
   font = NULL;
   size = 0;
   evas_object_text_font_get(to, &font, &size);
   fail_if(strcmp(font, "NON-EXISTING-FONT"));

   font = "Serif";
   size = 2;
   evas_object_text_font_set(to, font, size);
   font = NULL;
   size = 0;
   evas_object_text_font_get(to, &font, &size);
   fail_if(strcmp(font, "Serif"));
   fail_if(size != 2);

   evas_object_text_font_source_set(to, "/usr/share/fonts/Sans.ttf");
   font = evas_object_text_font_source_get(to);
   fail_if(strcmp(font, "/usr/share/fonts/Sans.ttf"));
   evas_object_text_font_source_set(to, "/usr/share/fonts/Sans.ttf");
   font = evas_object_text_font_source_get(to);
   fail_if(strcmp(font, "/usr/share/fonts/Sans.ttf"));

   /* BiDi Delimiters */
   evas_object_text_bidi_delimiters_set(to, ",.|");
   fail_if(strcmp(evas_object_text_bidi_delimiters_get(to), ",.|"));
   evas_object_text_bidi_delimiters_set(to, ",|");
   fail_if(strcmp(evas_object_text_bidi_delimiters_get(to), ",|"));
   evas_object_text_bidi_delimiters_set(to, NULL);
   fail_if(evas_object_text_bidi_delimiters_get(to));
   evas_object_text_bidi_delimiters_set(to, ",|");
   fail_if(strcmp(evas_object_text_bidi_delimiters_get(to), ",|"));

   /* Style */
   evas_object_text_text_set(to, "");
   evas_object_text_style_set(to, EVAS_TEXT_STYLE_SHADOW);
   fail_if(evas_object_text_style_get(to) != EVAS_TEXT_STYLE_SHADOW);
   evas_object_text_style_set(to, EVAS_TEXT_STYLE_OUTLINE);
   fail_if(evas_object_text_style_get(to) != EVAS_TEXT_STYLE_OUTLINE);

   /* Rehinting */
   evas_object_text_text_set(to, "Bla");
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_NONE);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_BYTECODE);

   /* Direction of an empty text should be NEUTRAL */
   evas_object_text_text_set(to, "");
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_NEUTRAL);

   /* LTR paragraphs */
   evas_object_text_text_set(to, "Test נסיון");
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_LTR);

   /* RTL paragraphs */
   evas_object_text_text_set(to, "נסיון test");
#ifdef HAVE_FRIBIDI
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);
#else
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_LTR);
#endif

#ifdef HAVE_FRIBIDI
   /* Check direction with evas_object_paragraph_direction_set API */
   evas_object_text_text_set(to, "12345");
   fail_if(evas_object_text_direction_get(to) == EVAS_BIDI_DIRECTION_RTL);
   evas_object_paragraph_direction_set(to, EVAS_BIDI_DIRECTION_RTL);
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);
   evas_object_paragraph_direction_set(to, EVAS_BIDI_DIRECTION_NEUTRAL);
#endif

   END_TEXT_TEST();
}
EFL_END_TEST

EFL_START_TEST(evas_text_style)
{
   Evas_Coord l, r, t, b;
   START_TEXT_TEST();
   const char *buf = "Test";
   evas_object_text_text_set(to, buf);
   evas_object_text_style_set(to, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 0) || (t != 0) || (b != 0));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 1) || (t != 0) || (b != 1));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_OUTLINE);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 1) || (t != 1) || (b != 1));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_SOFT_OUTLINE);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_GLOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_OUTLINE_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_FAR_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_SOFT_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_text_style_set(to, EVAS_TEXT_STYLE_FAR_SOFT_SHADOW);
   evas_object_text_style_pad_get(to, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 4) || (t != 0) || (b != 4));

   fail_if(strcmp(evas_object_text_text_get(to), buf));
   END_TEXT_TEST();
}
EFL_END_TEST

#ifdef HAVE_FRIBIDI
EFL_START_TEST(evas_text_bidi)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   int i;
   Evas_Coord x, y, w, h, px;
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;

   evas_object_text_font_set(to, font, size);

   evas_object_text_text_set(to, buf);
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_LTR);
   evas_object_text_text_set(to, "בדיקה");
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);

   /* With RTL text coords should be monotontically decreasing. */
   evas_object_text_text_set(to, "נסיון...");
   x = 0;
   px = 200;
   for (i = 0 ; i < eina_unicode_utf8_get_len("נסיון...") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x >= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + ((3 * w) / 4),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* Get back cursor position, if click on left half of char.  */
   evas_object_text_text_set(to, "שלום...");
   x = 0;
   px = 200;
   for (i = 0 ; i < eina_unicode_utf8_get_len("שלום...") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x >= px);
        px = x;
        fail_if((i + 1) != evas_object_text_char_coords_get(to, x + (w / 4),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* Bidi text is a bit more complex */
   evas_object_text_text_set(to, "Test - נסיון...");
   x = 0;
   px = -100;
   for (i = 0 ; i < eina_unicode_utf8_get_len("Test - ") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x <= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 4),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* First rtl char requires more specific handling */
   fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
   fail_if(x <= px);
   px = x;
   fail_if(i != evas_object_text_char_coords_get(to, x + ((3 * w) / 4),
            y + (h / 2), &x, &y, &w, &h));
   i++;
   for ( ; i < eina_unicode_utf8_get_len("Test - נסיון") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x >= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 2),
                 y + (h / 2), &x, &y, &w, &h));
     }
   /* First ltr char requires more specific handling */
   fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
   fail_if(x <= px);
   px = x;
   i++;
   for ( ; i < eina_unicode_utf8_get_len("Test - נסיון...") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x <= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 2),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* And with an rtl text */
   evas_object_text_text_set(to, "נסיון - test...");
   x = 0;
   px = 100;
   for (i = 0 ; i < eina_unicode_utf8_get_len("נסיון - ") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x >= px);
        px = x;
        /* Get back the coords */
        if (w == 0)
          {
             int cx;
             fail_if(!evas_object_text_char_pos_get(to, i - 1, &cx, NULL, NULL, NULL));
             w = cx - x;
          }
        fail_if(i != evas_object_text_char_coords_get(to, x + (3 * w /4),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* First ltr char requires more specific handling */
   fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
   fail_if(x >= px);
   px = x;
   fail_if(i != evas_object_text_char_coords_get(to, x + (w / 2),
            y + (h / 2), &x, &y, &w, &h));
   i++;
   for ( ; i < eina_unicode_utf8_get_len("נסיון - test") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x <= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 2),
                 y + (h / 2), &x, &y, &w, &h));
     }
   /* First rtl char requires more specific handling */
   fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
   fail_if(x >= px);
   px = x;
   i++;
   for ( ; i < eina_unicode_utf8_get_len("נסיון - test...") ; i++)
     {
        fail_if(!evas_object_text_char_pos_get(to, i, &x, &y, &w, &h));
        fail_if(x >= px);
        px = x;
        /* Get back the coords */
        fail_if(i != evas_object_text_char_coords_get(to, x + (w / 2),
                 y + (h / 2), &x, &y, &w, &h));
     }

   /* And some last up to pos tests */
   Evas_Coord adv;
   int pos, prev_pos;
   evas_object_text_text_set(to, "Test - נסיון...");
   adv = evas_object_text_horiz_advance_get(to);
   pos = prev_pos = 0;
   for (x = 0 ; x <= (adv - 1) ; x++)
     {
        pos = evas_object_text_last_up_to_pos(to, x, 0);
        _ck_assert_int(pos, >=, prev_pos);
        prev_pos = pos;
     }
   pos = evas_object_text_last_up_to_pos(to, x, 0);
   fail_if(pos != -1);
   pos = evas_object_text_last_up_to_pos(to, -50, 0);
   fail_if(pos != -1);

   END_TEXT_TEST();
}
EFL_END_TEST
#endif

EFL_START_TEST(evas_text_render)
{
   Ecore_Evas *ee = ecore_evas_buffer_new(500, 500);
   Evas *evas = ecore_evas_get(ee);
   Evas_Object *to;
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;
   int w;

   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO);
   to = evas_object_text_add(evas);
   evas_object_text_font_source_set(to, TEST_FONT_SOURCE);
   evas_object_show(to);

   /* Check the changing ellipsis is updated properly. */
   evas_object_text_font_set(to, font, size);
   evas_object_text_text_set(to, "Dynamically changing ellipsis!");
   evas_object_text_ellipsis_set(to, 0.0);
   evas_object_resize(to, 50, 100);
   w = evas_object_text_horiz_advance_get(to);
   evas_object_text_ellipsis_set(to, -1.0);
   evas_render(evas);
   ck_assert_int_gt(evas_object_text_horiz_advance_get(to), w);

   ecore_evas_free(ee);
}
EFL_END_TEST

EFL_START_TEST(evas_text_font_load)
{
   Ecore_Evas *ee = ecore_evas_buffer_new(500, 500);
   Evas *evas = ecore_evas_get(ee);
   const char *font, *match;
   Eina_List *list, *li;
   int found = 0;

   evas_font_path_global_append(TEST_FONT_DIR);
   match = eina_stringshare_printf("%s:style=Regular", TEST_FONT_TTF_NAME);
   list = evas_font_available_list(evas);
   EINA_LIST_FOREACH(list, li, font)
     if (eina_streq(font, match))
       {
          found = 1;
          break;
       }
   evas_font_available_list_free(evas, list);
   eina_stringshare_del(match);

   ck_assert_int_eq(found, 1);

   ecore_evas_free(ee);
}
EFL_END_TEST

void evas_test_text(TCase *tc)
{
   tcase_add_test(tc, evas_text_simple);
   tcase_add_test(tc, evas_text_style);
   tcase_add_test(tc, evas_text_set_get);
   tcase_add_test(tc, evas_text_geometries);
   tcase_add_test(tc, evas_text_evas);
   tcase_add_test(tc, evas_text_ellipsis);
#ifdef HAVE_FRIBIDI
   tcase_add_test(tc, evas_text_bidi);
#endif

   tcase_add_test(tc, evas_text_unrelated);
   tcase_add_test(tc, evas_text_render);
   tcase_add_test(tc, evas_text_font_load);
}
