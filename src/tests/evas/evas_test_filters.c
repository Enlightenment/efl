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
   CHKGOOD("buffer:a(alpha);blend();");
   CHKGOOD("buffer:a(rgba);blend();");
   CHKBAAD("buffer:a(BAAD);blend();");
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

#endif // BUILD_FILTER_TESTS

void evas_test_filters(TCase *tc)
{
#if BUILD_FILTER_TESTS
   tcase_add_test(tc, evas_filter_parser);
#else
   (void) tc;
#endif
}
