#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>

#include "evas_suite.h"
#include "Evas.h"

#include "evas_tests_helpers.h"

static const char *style_buf =
   "DEFAULT='font=Sans font_size=10 color=#000 text_class=entry'"
   "br='\n'"
   "ps='ps'"
   "tab='\t'";

#define START_TB_TEST() \
   Evas *evas; \
   Evas_Object *tb; \
   Evas_Textblock_Style *st; \
   Evas_Textblock_Cursor *cur; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   tb = evas_object_textblock_add(evas); \
   fail_if(!tb); \
   st = evas_textblock_style_new(); \
   fail_if(!st); \
   evas_textblock_style_set(st, style_buf); \
   fail_if(strcmp(style_buf, evas_textblock_style_get(st))); \
   evas_object_textblock_style_set(tb, st); \
   cur = evas_object_textblock_cursor_new(tb); \
do \
{ \
} \
while (0)

#define END_TB_TEST() \
do \
{ \
   evas_textblock_cursor_free(cur); \
   evas_object_del(tb); \
   evas_textblock_style_free(st); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

START_TEST(evas_textblock_simple)
{
   START_TB_TEST();
   const char *buf = "This is a <br> test.";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   END_TB_TEST();
}
END_TEST

#define _CHECK_CURSOR_COORDS() \
do \
{ \
        Evas_Coord x, y, w, h; \
        int ret; \
        ret = evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, NULL, \
              EVAS_TEXTBLOCK_CURSOR_UNDER); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_char_geometry_get(cur, &x, &y, &w, &h); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h); \
        fail_if(ret == -1); \
} \
while (0)
START_TEST(evas_textblock_cursor)
{
   START_TB_TEST();
   size_t i, len;
   evas_object_textblock_newline_mode_set(tb, EINA_FALSE);
   const char *buf = "This is a<br> test.<ps>Lets see if this works.<ps>עוד פסקה.";

   /* Walk the textblock using cursor_char_next */
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   len = eina_unicode_utf8_get_len(buf) - 9; /* 9 because len(<br>) == 1 and len(<ps>) == 1 */
   for (i = 0 ; i < len ; i++)
     {
        _CHECK_CURSOR_COORDS();

        fail_if(evas_textblock_cursor_pos_get(cur) != (int) i);

        fail_if(!evas_textblock_cursor_char_next(cur) && (i < len - 1));
     }
   fail_if(evas_textblock_cursor_char_next(cur));

   /* Jump to positions all aronud the textblock */
   evas_textblock_cursor_pos_set(cur, -1);
   fail_if(evas_textblock_cursor_pos_get(cur) != 0);

   evas_textblock_cursor_pos_set(cur, len + 5);
   fail_if(evas_textblock_cursor_pos_get(cur) != (int) len);

   for (i = 0 ; i < len ; i++)
     {
        evas_textblock_cursor_pos_set(cur, i);

        _CHECK_CURSOR_COORDS();

        fail_if(evas_textblock_cursor_pos_get(cur) != (int) i);
     }

   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
END_TEST

START_TEST(evas_textblock_size)
{
   START_TB_TEST();
   Evas_Coord w, h, h2, nw, nh;
   const char *buf = "This is a <br> test.<br>גם בעברית";
   /* When wrapping is off, native size should be the same as formatted
    * size */

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h != nh));
   fail_if(w != 0);

   evas_object_textblock_text_markup_set(tb, "<br>");
   evas_object_textblock_size_formatted_get(tb, &w, &h2);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h2 != nh));

   /* Two lines == double the height */
   fail_if((w != 0) || (h * 2 != h2));

   evas_object_textblock_text_markup_set(tb, buf);

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h != nh));
   fail_if(w <= 0);

   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
END_TEST

void evas_test_textblock(TCase *tc)
{
   tcase_add_test(tc, evas_textblock_simple);
   tcase_add_test(tc, evas_textblock_cursor);
   tcase_add_test(tc, evas_textblock_size);
}

