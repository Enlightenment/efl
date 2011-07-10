
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

#define START_TEXT_TEST() \
   Evas *evas; \
   Evas_Object *to; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   to = evas_object_text_add(evas); \
do \
{ \
} \
while (0)

#define END_TEXT_TEST() \
do \
{ \
   evas_object_del(to); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

START_TEST(evas_text_simple)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   evas_object_text_text_set(to, buf);
   fail_if(strcmp(evas_object_text_text_get(to), buf));
   END_TEXT_TEST();
}
END_TEST

#define _CHECK_SET_GET(x) \
do \
{ \
   Evas_Coord r, g, b, a; \
   evas_object_text_##x##_set(to, 100, 150, 125, 12); \
   evas_object_text_##x##_get(to, &r, &g, &b, &a); \
   fail_if((r != 100) || (g != 150) || (b != 125) || (a != 12)); \
} \
while (0)

START_TEST(evas_text_set_get)
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
   const char *font = "Sans";
   Evas_Font_Size size = 14;
   evas_object_text_font_set(to, font, size);
   font = NULL;
   size = 0;
   evas_object_text_font_get(to, &font, &size);
   fail_if(strcmp(font, "Sans"));
   fail_if(size != 14);

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

   /* BiDi Delimiters */
   const char *delim;
   evas_object_text_bidi_delimiters_set(to, ",.|");
   delim = evas_object_text_bidi_delimiters_get(to);
   fail_if(strcmp(delim, ",.|"));
   END_TEXT_TEST();
}
END_TEST

START_TEST(evas_text_style)
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
END_TEST

#ifdef HAVE_FRIBIDI
START_TEST(evas_text_bidi)
{
   START_TEXT_TEST();
   const char *buf = "Test - בדיקה";
   evas_object_text_text_set(to, buf);
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_LTR);
   evas_object_text_text_set(to, "בדיקה");
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);
   END_TEXT_TEST();
}
END_TEST
#endif

void evas_test_text(TCase *tc)
{
   tcase_add_test(tc, evas_text_simple);
   tcase_add_test(tc, evas_text_style);
   tcase_add_test(tc, evas_text_set_get);
#ifdef HAVE_FRIBIDI
   tcase_add_test(tc, evas_text_bidi);
#endif
}
