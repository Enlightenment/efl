/* Unit test cases for the filters submodule.
 * For now, limited to Evas_Object_Text, but I prefer to separate these
 * functions from the core text object.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"
#include "../../lib/evas/include/evas_filter.h"

#if !defined(EFL_EO_API_SUPPORT) || !defined(EFL_BETA_API_SUPPORT)
# define BUILD_FILTER_TESTS 0
#else
# define BUILD_FILTER_TESTS 1
#endif

#if BUILD_FILTER_TESTS

/* These are the same as in evas_test_text.c */

#define TEST_FONT_NAME "DejaVuSans,UnDotum"
#define TEST_FONT_SOURCE TESTS_SRC_DIR "/TestFont.eet"

#define START_FILTER_TEST() \
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

#define END_FILTER_TEST() \
do \
{ \
   evas_object_del(to); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)


START_TEST(evas_filter_parser)
{
   Evas_Filter_Program *pgm;

#define CHECK_FILTER(_a, _v) do { \
   pgm = evas_filter_program_new("evas_suite"); \
   fail_if(evas_filter_program_parse(pgm, _a) != _v); \
   evas_filter_program_del(pgm); \
   } while (0)
#define CHKGOOD(_a) CHECK_FILTER(_a, EINA_TRUE)
#define CHKBAAD(_a) CHECK_FILTER(_a, EINA_FALSE)

   // Basic syntax errors
   fail_if(evas_filter_program_parse(NULL, "blend();"));
   CHKBAAD(NULL);
   CHKBAAD("");
   CHKBAAD("       \t    \n    ");
   CHKBAAD("blend");
   CHKBAAD("blend()");
   CHKBAAD("blend;");

   // Known & unknown instructions + some syntax errors
   CHKGOOD("blend();");
   CHKBAAD("blend()\nblend();");
   CHKBAAD("blend(blend());");
   CHKGOOD("blend(); blend();");
   CHKGOOD("buffer:a;blend();");
   CHKGOOD("buffer:a();blend();");
   CHKGOOD("buffer:a(alpha);blend();");
   CHKGOOD("buffer:a(rgba);blend();");
   CHKBAAD("buffer:a(BAAD);blend();");
   CHKGOOD("buffer:a(src=partname);blend();");
   CHKBAAD("buffer a(alpha);blend();");
   CHKGOOD("blend();blur();fill();");
   CHKGOOD("grow(10);");
   CHKGOOD("curve(0:0 - 255:255);");
   CHKGOOD("buffer:a(alpha);mask(a);");
   CHKGOOD("buffer:a(rgba);mask(a);");
   CHKGOOD("buffer:a(rgba);bump(a);");
   CHKGOOD("buffer:a(rgba);displace(a);");
   CHKGOOD("transform(output);");
   CHKBAAD("unknown_command();");
   CHKBAAD("blend(unknown_buffer);");
   CHKGOOD("//comment\nblend();");
   CHKBAAD("blend(); /* unterminated comment section");
   CHKGOOD("blend(/* we want yellow */ color = yellow);");
   CHKGOOD("/* blend ();\n this is still a comment\n*/\n blend();");
   CHKBAAD("blend(ox = 1/2);");
   CHKBAAD("blend();!@#$%^&*");
   CHKBAAD("blend(invalid=hello);");
   CHKBAAD("buffer:a(alpha);buffer:a(rgba);blend();");
   CHKBAAD("buffer:a(alpha,src=partname);");

   // Case sensitivity
   CHKGOOD("BLEND();");
   CHKGOOD("Blend();");
   CHKGOOD("bLeNd();");
   CHKGOOD("buffer : lowercase; blend (lowercase);");
   CHKGOOD("buffer : UPPERCASE; blend (UPPERCASE);");
   CHKBAAD("buffer : CamelCase; blend (cAMELcASE);");

   // Full sequential arguments (default values)
   CHKGOOD("blend(input, output, 0, 0, color = white, fillmode = none);");
   CHKGOOD("blur(3, -1, default, 0, 0, color = white, src = input, dst = output);");
   CHKGOOD("buffer : m (alpha); "
           "bump(m, 135.0, 45.0, 8.0, 0.0, color = white, compensate = true, "
           "src = input, dst = output, black = black, white = white, fillmode = repeat);");
   CHKGOOD("curve(0:0 - 255:255, linear, rgb, src = input, dst = output);");
   CHKGOOD("buffer : m (rgba); "
           "displace(m, 10, default, src = input, dst = output, fillmode = repeat);");
   CHKGOOD("fill(output, transparent, 0, 0, 0, 0);");
   CHKGOOD("grow(0, smooth = yes, src = input, dst = output);");
   CHKGOOD("buffer : m (alpha); "
           "mask(m, src = input, dst = output, color = white, fillmode = none);");
   CHKGOOD("buffer : m (alpha); "
           "transform(m, op = vflip, src = input, oy = 0);");

   // All colors
   static const char *colors [] = {
      "white",
      "black",
      "red",
      "green",
      "blue",
      "darkblue",
      "yellow",
      "magenta",
      "cyan",
      "orange",
      "purple",
      "brown",
      "maroon",
      "lime",
      "gray",
      "grey",
      "silver",
      "olive",
      "invisible",
      "transparent",
      "#ABC",
      "#ABCF",
      "#AABBCC",
      "#AABBCCFF",
      "WHITE",
      "White"
   };

   static const char *colors_bad[] = {
      "newcolor",
      "ABC",
      "#ZZZ",
      "#-10"
   };

   for (size_t c = 0; c < sizeof(colors) / sizeof(colors[0]); c++)
     {
        char buf[64];
        sprintf(buf, "blend(color = %s);", colors[c]);
        CHKGOOD(buf);
     }

   for (size_t c = 0; c < sizeof(colors_bad) / sizeof(colors_bad[0]); c++)
     {
        char buf[64];
        sprintf(buf, "blend(color = %s);", colors_bad[c]);
        CHKBAAD(buf);
     }

   // All booleans
   static const char *booleans[] = {
      "1", "0",
      "yes", "no",
      "on", "off",
      "enable", "disable",
      "enabled", "disabled",
      "true", "false",
      "YES", "Yes"
   };

   static const char *booleans_bad[] = {
      "o", "oui", "10"
   };

   for (size_t c = 0; c < sizeof(booleans) / sizeof(booleans[0]); c++)
     {
        char buf[64];
        sprintf(buf, "grow(10, smooth = %s);", booleans[c]);
        CHKGOOD(buf);
     }

   for (size_t c = 0; c < sizeof(booleans_bad) / sizeof(booleans_bad[0]); c++)
     {
        char buf[64];
        sprintf(buf, "grow(10, smooth = %s);", booleans_bad[c]);
        CHKBAAD(buf);
     }

   // fillmodes are parsed when converting from instructions to commands
}
END_TEST

START_TEST(evas_filter_text_padding_test)
{
   START_FILTER_TEST();
   const char *buf = "Tests";
   const char *font = TEST_FONT_NAME;
   Evas_Font_Size size = 14;
   Evas_Coord x, y, w, h, W, H;
   int l, r, t, b;

   evas_object_move(to, 0, 0);
   evas_object_text_text_set(to, buf);
   evas_object_text_font_set(to, font, size);
   evas_object_geometry_get(to, &x, &y, &w, &h);
   printf("Geometry: %dx%d+%d,%d\n", w, h, x, y);

#define CHKPAD(_l, _r, _t, _b, _code) \
   l = r = t = b = 0; \
   eo_do(to, evas_obj_text_filter_program_set(_code)); \
   evas_object_text_style_pad_get(to, &l, &r, &t, &b); \
   evas_object_geometry_get(to, NULL, NULL, &W, &H); \
   printf("Line %d: %dx%d for padding %d,%d,%d,%d\n", __LINE__, W, H, l, r, t, b); \
   fail_if((l != _l) || (r != _r) || (t != _t) || (b != _b)); \
   fail_if((W != (_l + _r + w)) || (H != (_t + _b + h)));

   // Single filters
   // In some scripts, a first blend is used to make sure all buffers are valid
   CHKPAD(0, 0, 0, 0, "blend();");
   CHKPAD(7, 0, 11, 0, "blend(ox = -7, oy = -11);");
   CHKPAD(0, 7, 0, 11, "blend(ox = 7, oy = 11);");
   CHKPAD(5, 5, 7, 7, "blur(rx = 5, ry = 7);");
   CHKPAD(0, 0, 5, 5, "blur(rx = 0, ry = 5);");
   CHKPAD(5, 5, 0, 0, "blur(rx = 5, ry = 0);");
   CHKPAD(0, 15, 7, 0, "blur(rx = 5, ry = 0, ox = 10, oy = -7);");
   CHKPAD(5, 5, 5, 5, "grow(5);");
   CHKPAD(0, 0, 0, 0, "buffer:a(alpha);blend(dst=a);curve(0:0-255:255,src=a,dst=a);");
   CHKPAD(0, 0, 0, 0, "fill();");
   CHKPAD(0, 0, 0, 0, "buffer:a(rgba);blend(dst=a);mask(a);");
   CHKPAD(0, 0, 0, 0, "buffer:a(alpha);blend(dst=a);bump(a);");
   CHKPAD(7, 7, 7, 7, "buffer:a(rgba);blend(dst=a);displace(a,7);");
   CHKPAD(0, 0, 0, 40, "buffer:a(alpha);blend(dst=a);transform(a,vflip,oy=20);blend(src=a);");

   // Filter combos. TODO: Add some more tricky cases :)
   CHKPAD(3, 5, 7, 11, "blend(ox = -3, oy = 11); blend(ox = 5, oy = -7);");
   CHKPAD(15, 15, 15, 15, "buffer:a(rgba);grow(10,dst=a);blur(5,src=a);");
   CHKPAD(10, 15, 10, 17, "buffer:a(alpha);blend(dst=a,ox=5,oy=7);blur(10,src=a);blend(ox=-6,oy=-9);");
   CHKPAD(5, 5, 5, 5, "buffer:a(alpha);blur(5,dst=a);bump(a,azimuth=45.0,color=yellow);");

   END_FILTER_TEST();
}
END_TEST

#endif // BUILD_FILTER_TESTS

void evas_test_filters(TCase *tc)
{
#if BUILD_FILTER_TESTS
   tcase_add_test(tc, evas_filter_parser);
   tcase_add_test(tc, evas_filter_text_padding_test);
#else
   (void) tc;
#endif
}
