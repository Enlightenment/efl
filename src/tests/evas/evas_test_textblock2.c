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
END_TEST

/* Should handle all the text editing. */
START_TEST(evas_textblock2_editing)
{
   START_TB_TEST();
   const char *buf = "First par." _PS "Second par.";
   eo_do(tb, efl_text_set(buf));
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);

   /* Check deletion works */
   /* Try deleting after the end of the textblock2 */
     {
        char *content;
        evas_textblock2_cursor_paragraph_last(cur);
        content = strdup(eo_do(tb, efl_text_get()));
        evas_textblock2_cursor_char_delete(cur);
        fail_if(strcmp(content, eo_do(tb, efl_text_get())));
        free(content);
     }

   /* Delete the first char */
   evas_textblock2_cursor_paragraph_first(cur);
   evas_textblock2_cursor_char_delete(cur);
   fail_if(strcmp(eo_do(tb, efl_text_get()),
            "irst par." _PS "Second par."));

   /* Delete some arbitrary char */
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_delete(cur);
   fail_if(strcmp(eo_do(tb, efl_text_get()),
            "irs par." _PS "Second par."));

   /* Delete a range */
   evas_textblock2_cursor_pos_set(main_cur, 1);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(eo_do(tb, efl_text_get()),
            "ir." _PS "Second par."));
   evas_textblock2_cursor_paragraph_char_first(main_cur);
   evas_textblock2_cursor_paragraph_char_last(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(eo_do(tb, efl_text_get()),
            "Second par."));

   eo_do(tb, efl_text_set(buf));
   evas_textblock2_cursor_paragraph_last(main_cur);
   evas_textblock2_cursor_text_prepend(main_cur, "Test<b>bla</b>bla.");
   evas_textblock2_cursor_paragraph_last(cur);
   evas_textblock2_cursor_paragraph_char_first(main_cur);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(eo_do(tb, efl_text_get()),
            "First par." _PS ""));

   /* Merging paragraphs */
   eo_do(tb, efl_text_set(buf));
   evas_textblock2_cursor_paragraph_char_last(cur);
   evas_textblock2_cursor_copy(cur, main_cur);
   evas_textblock2_cursor_char_delete(cur);

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

   /* Split paragraphs */
   evas_textblock2_cursor_text_prepend(cur, _PS);

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

   /* Merge paragraphs using range deletion */
   eo_do(tb, efl_text_set(buf));
   evas_textblock2_cursor_paragraph_first(cur);
   evas_textblock2_cursor_paragraph_char_last(cur);
   evas_textblock2_cursor_copy(cur, main_cur);
   evas_textblock2_cursor_char_prev(cur);
   evas_textblock2_cursor_char_next(main_cur);

   evas_textblock2_cursor_range_delete(cur, main_cur);
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

     {
        /* Limit to 1000 iterations so we'll never get into an infinite loop,
         * even if broken */
        int limit = 1000;
        eo_do(tb, efl_text_set("this is a test eauoeuaou" _PS "this is a test1" _PS "this is a test 3"));
        evas_textblock2_cursor_paragraph_last(cur);
        while (evas_textblock2_cursor_pos_get(cur) > 0)
          {
             limit--;
             fail_if(limit <= 0);
             evas_textblock2_cursor_copy(cur, main_cur);
             evas_textblock2_cursor_char_prev(cur);
             evas_textblock2_cursor_word_start(cur);
             evas_textblock2_cursor_range_delete(cur, main_cur);
          }
     }


   /* Insert illegal characters inside the format. */
     {
        const char *content;
        eo_do(tb, efl_text_set("a\n"));
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        eo_do(tb, efl_text_set("a\t"));
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        eo_do(tb, efl_text_set("a\xEF\xBF\xBC"));
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        eo_do(tb, efl_text_set("a\xE2\x80\xA9"));
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);
        (void) content;
     }

   /* FIXME: Also add text appending/prepending */

   END_TB_TEST();
}
END_TEST

#define _CHECK_CURSOR_COORDS() \
do \
{ \
        Evas_Coord cx, cy, cw, ch; \
        int ret; \
        ret = evas_textblock2_cursor_geometry_get(cur, &cx, &cy, &cw, &ch, \
              NULL, EVAS_TEXTBLOCK2_CURSOR_UNDER); \
        fail_if(ret == -1); \
        ret = evas_textblock2_cursor_geometry_get(cur, &cx, &cy, &cw, &ch, \
              NULL, EVAS_TEXTBLOCK2_CURSOR_BEFORE); \
        fail_if(ret == -1); \
        ret = evas_textblock2_cursor_char_geometry_get(cur, \
              &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
        ret = evas_textblock2_cursor_pen_geometry_get(cur, &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
        ret = evas_textblock2_cursor_line_geometry_get(cur, \
              &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
} \
while (0)
START_TEST(evas_textblock2_cursor)
{
   START_TB_TEST();
   Evas_Coord x, y, w, h;
   size_t i, len;
   Evas_Coord nw, nh;
   Evas_BiDi_Direction dir;
   const char *buf = "This is a\n test." _PS "Lets see if this works." _PS "עוד פסקה.";

   /* Walk the textblock2 using cursor_char_next */
   eo_do(tb, efl_text_set(buf));
   ck_assert_str_eq(eo_do(tb, efl_text_get()), buf);
   len = eina_unicode_utf8_get_len(buf);
   for (i = 0 ; i < len ; i++)
     {
        _CHECK_CURSOR_COORDS();

        fail_if(evas_textblock2_cursor_pos_get(cur) != (int) i);

        fail_if(!evas_textblock2_cursor_char_next(cur) && (i < len - 1));
     }
   fail_if(evas_textblock2_cursor_char_next(cur));

   /* Jump to positions all aronud the textblock2 */
   evas_textblock2_cursor_pos_set(cur, -1);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 0);

   evas_textblock2_cursor_pos_set(cur, len + 5);
   fail_if(evas_textblock2_cursor_pos_get(cur) != (int) len);

   for (i = 0 ; i < len ; i++)
     {
        evas_textblock2_cursor_pos_set(cur, i);

        _CHECK_CURSOR_COORDS();

        fail_if(evas_textblock2_cursor_pos_get(cur) != (int) i);
     }

   /* Create another cursor and insert text, making sure everything
    * is in sync. */
   eo_do(tb, efl_text_set(""));
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);
   evas_textblock2_cursor_copy(main_cur, cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) !=
         evas_textblock2_cursor_pos_get(main_cur));

   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));
   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));

   /* Insert text to a non-empty textblock2 */
   eo_do(tb, efl_text_set(""));
   eo_do(tb, efl_text_set(buf));
   evas_textblock2_cursor_copy(main_cur, cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) !=
         evas_textblock2_cursor_pos_get(main_cur));

   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));
   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));

   evas_textblock2_cursor_text_prepend(main_cur, "a");

   /* Cursor comparison */
   evas_textblock2_cursor_pos_set(cur, 1);
   evas_textblock2_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur) != -1);

   evas_textblock2_cursor_pos_set(cur, 2);
   evas_textblock2_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur) != 0);

   evas_textblock2_cursor_pos_set(cur, 3);
   evas_textblock2_cursor_pos_set(main_cur, 2);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur) != 1);

   /* Paragraph first */
   eo_do(tb, efl_text_set(buf));
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock2_cursor_pos_set(cur, i);

        evas_textblock2_cursor_paragraph_first(cur);
        fail_if(evas_textblock2_cursor_pos_get(cur) != 0);
     }

   /* Paragraph last */
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock2_cursor_pos_set(cur, i);

        evas_textblock2_cursor_paragraph_last(cur);
        fail_if(evas_textblock2_cursor_pos_get(cur) != (int) len);
     }

   /* Paragraph next */
   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));

   /* Cher next */
   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(evas_textblock2_cursor_char_next(cur));

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));

   /* Cher prev */
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(evas_textblock2_cursor_char_prev(cur));

   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(!evas_textblock2_cursor_char_prev(cur));
   fail_if(!evas_textblock2_cursor_char_prev(cur));

   /* Paragraph char first */
   evas_textblock2_cursor_paragraph_first(main_cur);
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_char_next(cur));
   evas_textblock2_cursor_paragraph_char_first(cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   /* Paragraph char last */
   evas_textblock2_cursor_paragraph_last(main_cur);
   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(!evas_textblock2_cursor_char_prev(cur));
   evas_textblock2_cursor_paragraph_char_last(cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   /* Line char first */
   evas_textblock2_cursor_paragraph_first(main_cur);
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_char_next(cur));
   evas_textblock2_cursor_line_char_first(cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   evas_textblock2_cursor_pos_set(cur, 12);
   evas_textblock2_cursor_line_char_first(cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 10);

   /* Line char first */
   evas_textblock2_cursor_paragraph_last(main_cur);
   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(!evas_textblock2_cursor_char_prev(cur));
   evas_textblock2_cursor_line_char_last(cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   evas_textblock2_cursor_pos_set(cur, 12);
   evas_textblock2_cursor_line_char_last(cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 16);

   /* Line set */
   evas_textblock2_cursor_paragraph_first(main_cur);
   evas_textblock2_cursor_paragraph_last(cur);

   fail_if(!evas_textblock2_cursor_line_set(cur, 0));
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));
   fail_if(!evas_textblock2_cursor_line_set(cur, 1));
   fail_if(!evas_textblock2_cursor_line_set(cur, 2));
   fail_if(!evas_textblock2_cursor_line_set(cur, 3));

   fail_if(evas_textblock2_cursor_line_set(cur, -1));
   fail_if(evas_textblock2_cursor_line_set(cur, 99));

   /* Cursor content get */
   evas_textblock2_cursor_pos_set(cur, 0);
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "T"));
   evas_textblock2_cursor_pos_set(cur, 9);
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "\n"));
   evas_textblock2_cursor_pos_set(cur, 43);
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "ד"));

   /* Eol get */
   for (i = 0 ; i < len ; i++)
     {
        evas_textblock2_cursor_pos_set(cur, i);
        evas_textblock2_cursor_copy(cur, main_cur);
        evas_textblock2_cursor_line_char_last(main_cur);

        if (!evas_textblock2_cursor_compare(cur, main_cur))
          {
             fail_if(!evas_textblock2_cursor_eol_get(cur));
          }
        else
          {
             fail_if(evas_textblock2_cursor_eol_get(cur));
          }
     }

#if 0
   /* FIXME: Add back once we actually implement this. */

   /* Format positions */
   const Evas_Object_Textblock_Node_Format *fnode;
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   evas_textblock2_cursor_copy(cur, main_cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 9);
   fail_if(evas_textblock2_cursor_format_get(cur) != fnode);

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 16);
   fail_if(evas_textblock2_cursor_format_get(cur) != fnode);
   evas_textblock2_cursor_format_next(main_cur);
   fail_if(evas_textblock2_cursor_compare(main_cur, cur));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_pos_get(cur) != 9);
   fail_if(evas_textblock2_cursor_format_get(cur) != fnode);
   evas_textblock2_cursor_format_prev(main_cur);
   fail_if(evas_textblock2_cursor_compare(main_cur, cur));

   evas_textblock2_cursor_char_next(main_cur);
   evas_textblock2_cursor_format_prev(main_cur);
   fail_if(evas_textblock2_cursor_compare(main_cur, cur));
#endif

   eo_do(tb, efl_text_set(buf));

   /* Check that pen geometry and getting char at coord are in sync. */
   do
     {
        int cur_pos;

        /* Check if it's the last char, if it is, break, otherwise, go back
         * to the current char because our test advanced the cursor. */
        if (!evas_textblock2_cursor_char_next(cur))
           break;
        else
           evas_textblock2_cursor_char_prev(cur);

        cur_pos = evas_textblock2_cursor_pos_get(cur);
        evas_textblock2_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
        evas_textblock2_cursor_char_coord_set(cur, x + (w / 2), y + (h / 2));
        fail_if(cur_pos != evas_textblock2_cursor_pos_get(cur));
     }
   while (evas_textblock2_cursor_char_next(cur));

   /* Try positions before the first paragraph, and after the last paragraph */
   eo_do(tb, efl_text_set(buf));
   evas_object_resize(tb, 10000, 10000);
   evas_object_textblock2_size_formatted_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_textblock2_cursor_pos_set(cur, 5);
   evas_textblock2_cursor_char_coord_set(cur, nw / 2,
         -50);
   evas_textblock2_cursor_paragraph_first(main_cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   evas_textblock2_cursor_pos_set(cur, 5);
   evas_textblock2_cursor_char_coord_set(cur, nw / 2,
         nh + 50);
   evas_textblock2_cursor_paragraph_last(main_cur);
   fail_if(evas_textblock2_cursor_compare(cur, main_cur));

   /* Try positions between the first paragraph and the first line. */
   eo_do(tb, efl_text_set(buf));
   fail_if(!evas_textblock2_cursor_char_coord_set(cur, 5, 1));

   /* Try positions beyond the left/right limits of lines. */
   for (i = 0 ; i < 2 ; i++)
     {
        evas_textblock2_cursor_line_set(cur, i);
        evas_textblock2_cursor_line_geometry_get(cur, &x, &y, &w, &h);

        evas_textblock2_cursor_pos_set(main_cur, 5);
        evas_textblock2_cursor_char_coord_set(main_cur, x - 50, y);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));

        evas_textblock2_cursor_line_char_last(cur);
        evas_textblock2_cursor_pos_set(main_cur, 5);
        evas_textblock2_cursor_char_coord_set(main_cur, x + w + 50, y);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));
     }

   /* Check direction */
   eo_do(tb, efl_text_set("test"));
   fail_if(strcmp(eo_do(tb, efl_text_get()), "test"));
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   eo_do(tb, efl_text_set("עוד פסקה"));
   fail_if(strcmp(eo_do(tb, efl_text_get()), "עוד פסקה"));
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);

#ifdef HAVE_FRIBIDI
   eo_do(tb, efl_text_set(
         "testנסיוןtestנסיון" _PS
         "נסיוןtestנסיוןtest" _PS
         "testנסיוןtest" _PS
         "נסיוןtestנסיון" _PS
         "testנסיון\nנסיון" _PS
         "נסיוןtest\ntest"
         ));

   for (i = 0 ; i < 8 ; i++)
     {
        evas_textblock2_cursor_line_set(cur, i);
        evas_textblock2_cursor_line_geometry_get(cur, &x, &y, &w, &h);
        switch (i)
          {
           case 0:
           case 2:
           case 4:
           case 5:
              /* Ltr paragraph */
              evas_textblock2_cursor_pos_set(main_cur, 7);
              evas_textblock2_cursor_char_coord_set(main_cur, x - 50, y);
              fail_if(evas_textblock2_cursor_compare(main_cur, cur));

              evas_textblock2_cursor_line_char_last(cur);
              evas_textblock2_cursor_pos_set(main_cur, 7);
              evas_textblock2_cursor_char_coord_set(main_cur, x + w + 50, y);
              fail_if(evas_textblock2_cursor_compare(main_cur, cur));
              break;
           case 1:
           case 3:
           case 6:
           case 7:
              /* Rtl paragraph */
              evas_textblock2_cursor_line_char_last(cur);
              evas_textblock2_cursor_pos_set(main_cur, 7);
              evas_textblock2_cursor_char_coord_set(main_cur, x - 50, y);
              fail_if(evas_textblock2_cursor_compare(main_cur, cur));
              printf("TOM2 %d %d\n",
                    evas_textblock2_cursor_pos_get(main_cur),
                    evas_textblock2_cursor_pos_get(cur)
                    );

              evas_textblock2_cursor_line_char_first(cur);
              evas_textblock2_cursor_pos_set(main_cur, 7);
              evas_textblock2_cursor_char_coord_set(main_cur, x + w + 50, y);
              printf("TOM3 %d %d\n",
                    evas_textblock2_cursor_pos_get(main_cur),
                    evas_textblock2_cursor_pos_get(cur)
                    );
              printf("***\n");
              fail_if(evas_textblock2_cursor_compare(main_cur, cur));
              break;
          }
     }
#endif

   eo_do(tb, efl_text_set(buf));
   /* Testing line geometry.*/
     {
        Evas_Coord lx, ly, lw, lh;
        Evas_Coord plx, ply, plw, plh;
        evas_textblock2_cursor_line_set(cur, 0);
        evas_textblock2_cursor_copy(cur, main_cur);
        evas_textblock2_cursor_line_char_last(main_cur);
        evas_textblock2_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);

        while (evas_textblock2_cursor_compare(cur, main_cur) <= 0)
          {
             evas_textblock2_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
             fail_if(0 != evas_textblock2_cursor_line_geometry_get(
                      cur, &lx, &ly, &lw, &lh));
             fail_if((x < lx) ||
                   (y < ly) || (y + h > ly + lh));
             fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));

             plx = lx;
             ply = ly;
             plw = lw;
             plh = lh;
             evas_textblock2_cursor_char_next(cur);
          }

        evas_textblock2_cursor_line_set(cur, 1);
        evas_textblock2_cursor_copy(cur, main_cur);
        evas_textblock2_cursor_line_char_last(main_cur);
        evas_textblock2_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);

        while (evas_textblock2_cursor_compare(cur, main_cur) <= 0)
          {
             evas_textblock2_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
             fail_if(1 != evas_textblock2_cursor_line_geometry_get(
                      cur, &lx, &ly, &lw, &lh));
             fail_if((x < lx) ||
                   (y < ly) || (y + h > ly + lh));
             fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));

             plx = lx;
             ply = ly;
             plw = lw;
             plh = lh;
             evas_textblock2_cursor_char_next(cur);
          }

        evas_textblock2_cursor_paragraph_last(cur);
        evas_textblock2_cursor_line_set(cur, 0);
        evas_textblock2_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);
        evas_object_textblock2_line_number_geometry_get(tb, 0,
              &lx, &ly, &lw, &lh);
        fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));
        fail_if(0 != evas_textblock2_cursor_line_coord_set(cur, ly + (lh / 2)));

        evas_textblock2_cursor_line_set(cur, 1);
        evas_textblock2_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);
        evas_object_textblock2_line_number_geometry_get(tb, 1,
              &lx, &ly, &lw, &lh);
        fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));
        fail_if(1 != evas_textblock2_cursor_line_coord_set(cur, ly + (lh / 2)));

        /* Before the start of the textblock2 */
        fail_if(0 != evas_textblock2_cursor_line_coord_set(cur, -50));
        fail_if(3 != evas_textblock2_cursor_line_coord_set(cur, 100000));

        /* And now with a valigned textblock2. */
        eo_do(tb, efl_text_set(buf));
        evas_object_resize(tb, 10000, 10000);
        evas_object_textblock2_size_formatted_get(tb, &nw, &nh);
        evas_object_resize(tb, 2 * nw, 2 * nh);

        evas_object_textblock2_valign_set(tb, 0.5);
        evas_textblock2_cursor_paragraph_first(cur);
        evas_textblock2_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
        fail_if(y <= 0);

        evas_textblock2_cursor_paragraph_last(main_cur);
        evas_textblock2_cursor_char_coord_set(main_cur, x + w, y / 2);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));

        evas_textblock2_cursor_paragraph_last(main_cur);
        evas_textblock2_cursor_line_coord_set(main_cur, y / 2);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));

        /* Fail if they are equal, i.e if it for some reason thinks it should
         * go to the end. */
        evas_textblock2_cursor_paragraph_first(main_cur);
        evas_textblock2_cursor_paragraph_last(cur);
        evas_textblock2_cursor_char_coord_set(main_cur, x + w, nh + 1);
        fail_if(!evas_textblock2_cursor_compare(main_cur, cur));

        evas_textblock2_cursor_paragraph_first(main_cur);
        evas_textblock2_cursor_paragraph_last(cur);
        evas_textblock2_cursor_line_coord_set(main_cur, nh + 1);
        fail_if(!evas_textblock2_cursor_compare(main_cur, cur));

        /* Fail if it doesn't go to the end. */
        evas_textblock2_cursor_paragraph_last(cur);
        evas_textblock2_cursor_paragraph_first(main_cur);
        evas_textblock2_cursor_char_coord_set(main_cur, x + w, (2 * nh) - 1);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));

        evas_textblock2_cursor_paragraph_first(main_cur);
        evas_textblock2_cursor_line_coord_set(main_cur, (2 * nh) - 1);
        fail_if(evas_textblock2_cursor_compare(main_cur, cur));
     }

     {
        const char *buf_wb = "a This is_a t:e.s't a";
        eo_do(tb, efl_text_set(buf_wb));

        /* Word start/end */
        evas_textblock2_cursor_pos_set(cur, 3);
        evas_textblock2_cursor_word_start(cur);
        fail_if(2 != evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_end(cur);
        fail_if(5 != evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_pos_set(cur, 13);
        evas_textblock2_cursor_word_end(cur);
        fail_if(18 != evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_start(cur);
        fail_if(12 != evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_start(cur);
        fail_if(12 != evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_start(cur);
        fail_if(12 != evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_end(cur);
        fail_if(18 != evas_textblock2_cursor_pos_get(cur));

        /* Bug with 1 char word separators at paragraph start. */
        eo_do(tb, efl_text_set("=test"));
        evas_textblock2_cursor_pos_set(cur, 4);
        evas_textblock2_cursor_word_start(cur);
        fail_if(1 != evas_textblock2_cursor_pos_get(cur));

        /* 1 char words separated by spaces. */
        eo_do(tb, efl_text_set("a a a a"));
        evas_textblock2_cursor_paragraph_first(cur);

        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(0, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(0, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_pos_set(cur, 2);
        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(2, evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(2, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_pos_set(cur, 3);
        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(2, evas_textblock2_cursor_pos_get(cur));
        evas_textblock2_cursor_pos_set(cur, 3);
        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(4, evas_textblock2_cursor_pos_get(cur));

        /* Going back when ending with whites. */
        eo_do(tb, efl_text_set("aa bla "));
        evas_textblock2_cursor_paragraph_last(cur);

        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(3, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(5, evas_textblock2_cursor_pos_get(cur));

        /* moving across paragraphs */
        eo_do(tb, efl_text_set("test" _PS "  case"));
        evas_textblock2_cursor_pos_set(cur, 4);
        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(10, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_pos_set(cur, 6);
        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(0, evas_textblock2_cursor_pos_get(cur));
     }

   /* Make sure coords are correct for ligatures */
     {
        eo_do(tb, efl_text_set("fi\nfii"));

#ifdef HAVE_HARFBUZZ
        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
             evas_textblock2_cursor_char_next(cur);
          }

        evas_textblock2_cursor_char_next(cur);

        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
             evas_textblock2_cursor_char_next(cur);
          }
        evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 3);
#else
/* FIXME: Disabled because fails on jenkins */
#if 0
        evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 4);
        evas_textblock2_cursor_char_next(cur);
        evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 3);

        evas_textblock2_cursor_pos_set(cur, 3);
        evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 4);

        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock2_cursor_char_next(cur);
             evas_textblock2_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
          }
#endif
#endif
     }

   END_TB_TEST();
}
END_TEST

void evas_test_textblock2(TCase *tc)
{
   tcase_add_test(tc, evas_textblock2_simple);
   tcase_add_test(tc, evas_textblock2_cursor);
#ifdef HAVE_FRIBIDI
   //tcase_add_test(tc, evas_textblock2_split_cursor);
#endif
   //tcase_add_test(tc, evas_textblock2_size);
   tcase_add_test(tc, evas_textblock2_editing);
   //tcase_add_test(tc, evas_textblock2_style);
   tcase_add_test(tc, evas_textblock2_evas);
   //tcase_add_test(tc, evas_textblock2_text_getters);
   //tcase_add_test(tc, evas_textblock2_formats);
   //case_add_test(tc, evas_textblock2_format_removal);
   tcase_add_test(tc, evas_textblock2_set_get);
   //tcase_add_test(tc, evas_textblock2_geometries);
   //tcase_add_test(tc, evas_textblock2_various);
   //tcase_add_test(tc, evas_textblock2_wrapping);
   //tcase_add_test(tc, evas_textblock2_items);
   tcase_add_test(tc, evas_textblock2_delete);
}

