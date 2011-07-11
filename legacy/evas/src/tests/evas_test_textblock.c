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
   evas_object_textblock_newline_mode_set(tb, EINA_FALSE); \
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
        ret = evas_textblock_cursor_line_geometry_get(cur, &x, &y, &w, &h); \
        fail_if(ret == -1); \
} \
while (0)
START_TEST(evas_textblock_cursor)
{
   START_TB_TEST();
   size_t i, len;
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

   /* Create another cursor and insert text, making sure everything
    * is in sync. */
   evas_object_textblock_clear(tb);
   Evas_Textblock_Cursor *main_cur = evas_object_textblock_cursor_get(tb);
   evas_textblock_cursor_copy(main_cur, cur);
   fail_if(evas_textblock_cursor_pos_get(cur) !=
         evas_textblock_cursor_pos_get(main_cur));

   evas_textblock_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock_cursor_pos_get(cur) ==
         evas_textblock_cursor_pos_get(main_cur));
   evas_textblock_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock_cursor_pos_get(cur) ==
         evas_textblock_cursor_pos_get(main_cur));

   /* Insert text to a non-empty textblock */
   evas_object_textblock_clear(tb);
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_copy(main_cur, cur);
   fail_if(evas_textblock_cursor_pos_get(cur) !=
         evas_textblock_cursor_pos_get(main_cur));

   evas_textblock_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock_cursor_pos_get(cur) ==
         evas_textblock_cursor_pos_get(main_cur));
   evas_textblock_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock_cursor_pos_get(cur) ==
         evas_textblock_cursor_pos_get(main_cur));

   /* Make sure append works */
   evas_textblock_cursor_copy(main_cur, cur);
   fail_if(evas_textblock_cursor_pos_get(cur) !=
         evas_textblock_cursor_pos_get(main_cur));
   evas_textblock_cursor_text_append(main_cur, "a");
   fail_if(evas_textblock_cursor_pos_get(cur) !=
         evas_textblock_cursor_pos_get(main_cur));

   /* Cursor comparison */
   evas_textblock_cursor_pos_set(cur, 1);
   evas_textblock_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock_cursor_compare(cur, main_cur) != -1);

   evas_textblock_cursor_pos_set(cur, 2);
   evas_textblock_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock_cursor_compare(cur, main_cur) != 0);

   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock_cursor_compare(cur, main_cur) != 1);

   /* Paragraph first */
   evas_object_textblock_text_markup_set(tb, buf);
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock_cursor_pos_set(cur, i);

        evas_textblock_cursor_paragraph_first(cur);
        fail_if(evas_textblock_cursor_pos_get(cur) != 0);
     }

   /* Paragraph last */
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock_cursor_pos_set(cur, i);

        evas_textblock_cursor_paragraph_last(cur);
        fail_if(evas_textblock_cursor_pos_get(cur) != (int) len);
     }

   /* Paragraph next */
   evas_textblock_cursor_paragraph_last(cur);
   fail_if(evas_textblock_cursor_paragraph_next(cur));

   evas_textblock_cursor_paragraph_first(cur);
   fail_if(!evas_textblock_cursor_paragraph_next(cur));
   fail_if(!evas_textblock_cursor_paragraph_next(cur));

   /* Paragraph prev */
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(evas_textblock_cursor_paragraph_prev(cur));

   evas_textblock_cursor_paragraph_last(cur);
   fail_if(!evas_textblock_cursor_paragraph_prev(cur));
   fail_if(!evas_textblock_cursor_paragraph_prev(cur));

   /* Cher next */
   evas_textblock_cursor_paragraph_last(cur);
   fail_if(evas_textblock_cursor_char_next(cur));

   evas_textblock_cursor_paragraph_first(cur);
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(!evas_textblock_cursor_paragraph_next(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(!evas_textblock_cursor_paragraph_next(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));

   /* Cher prev */
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(evas_textblock_cursor_char_prev(cur));

   evas_textblock_cursor_paragraph_last(cur);
   fail_if(!evas_textblock_cursor_char_prev(cur));
   fail_if(!evas_textblock_cursor_paragraph_prev(cur));
   fail_if(!evas_textblock_cursor_char_prev(cur));

   /* Paragraph char first */
   evas_textblock_cursor_paragraph_first(main_cur);
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(!evas_textblock_cursor_char_next(cur));
   evas_textblock_cursor_paragraph_char_first(cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   /* Paragraph char last */
   evas_textblock_cursor_paragraph_last(main_cur);
   evas_textblock_cursor_paragraph_last(cur);
   fail_if(!evas_textblock_cursor_char_prev(cur));
   evas_textblock_cursor_paragraph_char_last(cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   /* Line char first */
   evas_textblock_cursor_paragraph_first(main_cur);
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(!evas_textblock_cursor_char_next(cur));
   evas_textblock_cursor_line_char_first(cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   evas_textblock_cursor_pos_set(cur, 12);
   evas_textblock_cursor_line_char_first(cur);
   fail_if(evas_textblock_cursor_pos_get(cur) != 10);

   /* Line char first */
   evas_textblock_cursor_paragraph_last(main_cur);
   evas_textblock_cursor_paragraph_last(cur);
   fail_if(!evas_textblock_cursor_char_prev(cur));
   evas_textblock_cursor_line_char_last(cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   evas_textblock_cursor_pos_set(cur, 12);
   evas_textblock_cursor_line_char_last(cur);
   fail_if(evas_textblock_cursor_pos_get(cur) != 16);

   /* Line set */
   evas_textblock_cursor_paragraph_first(main_cur);
   evas_textblock_cursor_paragraph_last(cur);

   fail_if(!evas_textblock_cursor_line_set(cur, 0));
   fail_if(evas_textblock_cursor_compare(cur, main_cur));
   fail_if(!evas_textblock_cursor_line_set(cur, 1));
   fail_if(!evas_textblock_cursor_line_set(cur, 2));
   fail_if(!evas_textblock_cursor_line_set(cur, 3));

   fail_if(evas_textblock_cursor_line_set(cur, -1));
   fail_if(evas_textblock_cursor_line_set(cur, 99));

   /* Paragraph text get */
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "This is a<br> test."));
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "Lets see if this works."));
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "עוד פסקה."));

   /* Paragraph length get */
   evas_textblock_cursor_paragraph_first(cur);
   /* -3 because len(<br>) == 1 */
   fail_if(evas_textblock_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("This is a<br> test.") - 3);
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(evas_textblock_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("Lets see if this works."));
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(evas_textblock_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("עוד פסקה."));

   /* Cursor content get */
   evas_textblock_cursor_pos_set(cur, 0);
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "T"));
   evas_textblock_cursor_pos_set(cur, 9);
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "\n"));
   evas_textblock_cursor_pos_set(cur, 43);
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "ד"));

   /* Eol get */
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock_cursor_pos_set(cur, i);
        evas_textblock_cursor_copy(cur, main_cur);
        evas_textblock_cursor_line_char_last(main_cur);

        if (!evas_textblock_cursor_compare(cur, main_cur))
          {
             fail_if(!evas_textblock_cursor_eol_get(cur));
          }
        else
          {
             fail_if(evas_textblock_cursor_eol_get(cur));
          }
     }


   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
END_TEST

/* Should handle all the text editing. */
START_TEST(evas_textblock_editing)
{
   START_TB_TEST();
   const char *buf = "This is a <br> test.";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   END_TB_TEST();
}
END_TEST

/* Text getters */
START_TEST(evas_textblock_text_getters)
{
   START_TB_TEST();
   const char *buf = "This is a <br> test.<ps>"
      "טקסט בעברית<ps>and now in english.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_paragraph_first(cur);

   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "This is a <br> test."));

   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "טקסט בעברית"));

   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "and now in english."));

   /* Range get */
   Evas_Textblock_Cursor *main_cur = evas_object_textblock_cursor_get(tb);
   evas_textblock_cursor_pos_set(main_cur, 2);
   evas_textblock_cursor_pos_set(cur, 2);
   fail_if(*evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP));

   evas_textblock_cursor_pos_set(main_cur, 2);
   evas_textblock_cursor_pos_set(cur, 6);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is i"));

   evas_textblock_cursor_pos_set(main_cur, 5);
   evas_textblock_cursor_pos_set(cur, 14);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is a <br> te"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps>טק"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps>טקסט בעברית<ps>an"));

   /* Backward range get */
   evas_textblock_cursor_pos_set(main_cur, 2);
   evas_textblock_cursor_pos_set(cur, 2);
   fail_if(*evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP));

   evas_textblock_cursor_pos_set(main_cur, 2);
   evas_textblock_cursor_pos_set(cur, 6);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is i"));

   evas_textblock_cursor_pos_set(main_cur, 5);
   evas_textblock_cursor_pos_set(cur, 14);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is a <br> te"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps>טק"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps>טקסט בעברית<ps>an"));

   /* Uninit cursors and other weird cases */
   evas_object_textblock_clear(tb);
   evas_textblock_cursor_copy(main_cur, cur);
   evas_textblock_cursor_text_prepend(main_cur, "aaa");
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "aaa"));

   END_TB_TEST();
}
END_TEST

/* Different text styles, for example, shadow. */
START_TEST(evas_textblock_style)
{
   START_TB_TEST();
   const char *buf = "";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   END_TB_TEST();
}
END_TEST

/* Aux evas stuff, such as scale. */
START_TEST(evas_textblock_aux)
{
   Evas_Coord w, h, sw, sh;
   START_TB_TEST();
   const char *buf = "Test";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_scale_set(tb, 3.0);
   evas_object_textblock_size_formatted_get(tb, &sw, &sh);
   fail_if((sw <= w) || (sh <= h));

   evas_object_scale_set(tb, 0.5);
   evas_object_textblock_size_formatted_get(tb, &sw, &sh);
   fail_if((sw >= w) || (sh >= h));

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

   evas_object_textblock_text_markup_set(tb, "a<br>a");
   evas_object_textblock_size_formatted_get(tb, &w, &h2);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h2 != nh));

   /* Two lines == double the height */
   fail_if(h * 2 != h2);

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
   tcase_add_test(tc, evas_textblock_editing);
   tcase_add_test(tc, evas_textblock_style);
   tcase_add_test(tc, evas_textblock_aux);
   tcase_add_test(tc, evas_textblock_text_getters);
}

