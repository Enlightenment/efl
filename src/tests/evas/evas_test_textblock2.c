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
EAPI int
_evas_textblock2_format_offset_get(const Evas_Object_Textblock2_Node_Format *n);
/* end of functions defined in evas_object_textblock2.c */

#define TEST_FONT "font=DejaVuSans font_source=" TESTS_SRC_DIR "/TestFont.eet"

#define _PS "\xE2\x80\xA9"

static const char *style_buf =
   "DEFAULT='" TEST_FONT " font_size=10 color=#000 text_class=entry'"
   "newline='br'"
   "b='+ font_weight=bold'";

#define START_TB_TEST() \
   Evas *evas; \
   Evas_Object *tb; \
   Evas_Textblock2_Style *st; \
   Evas_Textblock2_Cursor *cur; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   tb = evas_object_textblock2_add(evas); \
   fail_if(!tb); \
   evas_object_textblock2_legacy_newline_set(tb, EINA_FALSE); \
   st = evas_textblock2_style_new(); \
   fail_if(!st); \
   evas_textblock2_style_set(st, style_buf); \
   fail_if(strcmp(style_buf, evas_textblock2_style_get(st))); \
   evas_object_textblock2_style_set(tb, st); \
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
   evas_textblock2_style_free(st); \
   evas_free(evas); \
   evas_shutdown(); \
} \
while (0)

START_TEST(evas_textblock2_simple)
{
   START_TB_TEST();
   const char *buf = "Th<i>i</i>s is a \n te<b>s</b>t.";
   evas_object_textblock2_text_markup_set(tb, buf);
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), buf);
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
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), buf));
   len = eina_unicode_utf8_get_len(buf) - 12; /* 12 because len(<br/>) == 1 and len(<ps/>) == 1 */
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
   evas_object_textblock2_clear(tb);
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
   evas_object_textblock2_clear(tb);
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_copy(main_cur, cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) !=
         evas_textblock2_cursor_pos_get(main_cur));

   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));
   evas_textblock2_cursor_text_prepend(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) ==
         evas_textblock2_cursor_pos_get(main_cur));

   /* Make sure append works */
   evas_textblock2_cursor_copy(main_cur, cur);
   fail_if(evas_textblock2_cursor_pos_get(cur) !=
         evas_textblock2_cursor_pos_get(main_cur));
   evas_textblock2_cursor_text_append(main_cur, "a");
   fail_if(evas_textblock2_cursor_pos_get(cur) !=
         evas_textblock2_cursor_pos_get(main_cur));

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
   evas_object_textblock2_text_markup_set(tb, buf);
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

   /* Paragraph prev */
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(evas_textblock2_cursor_paragraph_prev(cur));

   evas_textblock2_cursor_paragraph_last(cur);
   fail_if(!evas_textblock2_cursor_paragraph_prev(cur));
   fail_if(!evas_textblock2_cursor_paragraph_prev(cur));

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
   fail_if(!evas_textblock2_cursor_paragraph_prev(cur));
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

   /* Paragraph text get */
   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "This is a\n test."));
   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "Lets see if this works."));
   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "עוד פסקה."));

   /* Paragraph length get */
   evas_textblock2_cursor_paragraph_first(cur);
   /* -4 because len(<br/>) == 1 */
   fail_if(evas_textblock2_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("This is a\n test.") - 4);
   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(evas_textblock2_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("Lets see if this works."));
   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(evas_textblock2_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("עוד פסקה."));

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

   /* Format positions */
   const Evas_Object_Textblock2_Node_Format *fnode;
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


   evas_object_textblock2_text_markup_set(tb, buf);

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
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
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
   evas_object_textblock2_text_markup_set(tb, buf);
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
   evas_object_textblock2_text_markup_set(tb, "test");
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), "test"));
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK2_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK2_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   evas_object_textblock2_text_markup_set(tb, "עוד פסקה");
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), "עוד פסקה"));
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK2_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock2_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK2_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);

#ifdef HAVE_FRIBIDI
   evas_object_textblock2_text_markup_set(tb,
         "testנסיוןtestנסיון" _PS ""
         "נסיוןtestנסיוןtest" _PS ""
         "testנסיוןtest" _PS ""
         "נסיוןtestנסיון" _PS ""
         "testנסיון\nנסיון" _PS ""
         "נסיוןtest\ntest"
         );

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

              evas_textblock2_cursor_line_char_first(cur);
              evas_textblock2_cursor_pos_set(main_cur, 7);
              evas_textblock2_cursor_char_coord_set(main_cur, x + w + 50, y);
              fail_if(evas_textblock2_cursor_compare(main_cur, cur));
              break;
          }
     }
#endif

   evas_object_textblock2_text_markup_set(tb, buf);
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
        evas_object_textblock2_text_markup_set(tb, buf);
        evas_object_textblock2_size_native_get(tb, &nw, &nh);
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
        evas_object_textblock2_text_markup_set(tb, buf_wb);

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
        evas_object_textblock2_text_markup_set(tb, "=test");
        evas_textblock2_cursor_pos_set(cur, 4);
        evas_textblock2_cursor_word_start(cur);
        fail_if(1 != evas_textblock2_cursor_pos_get(cur));

        /* 1 char words separated by spaces. */
        evas_object_textblock2_text_markup_set(tb, "a a a a");
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
        evas_object_textblock2_text_markup_set(tb, "aa bla ");
        evas_textblock2_cursor_paragraph_last(cur);

        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(3, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(5, evas_textblock2_cursor_pos_get(cur));

        /* moving across paragraphs */
        evas_object_textblock2_text_markup_set(tb,
                                              "test" _PS ""
                                              "  case");
        evas_textblock2_cursor_pos_set(cur, 4);
        evas_textblock2_cursor_word_end(cur);
        ck_assert_int_eq(10, evas_textblock2_cursor_pos_get(cur));

        evas_textblock2_cursor_pos_set(cur, 6);
        evas_textblock2_cursor_word_start(cur);
        ck_assert_int_eq(0, evas_textblock2_cursor_pos_get(cur));
     }

   /* Make sure coords are correct for ligatures */
     {
        evas_object_textblock2_text_markup_set(tb, "fi\nfii");

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

#ifdef HAVE_FRIBIDI
START_TEST(evas_textblock2_split_cursor)
{
   START_TB_TEST();
   Evas_Coord x, w, x2, w2;
   Evas_Coord nw, nh;
   Evas_Coord cx, cy, cx2, cy2;

   /* Split cursor in LTR paragraph.
    * Russian 't' in the beginnning to create additional item.*/
                                            /*01234    5 6789012345678  19  01234 */
   evas_object_textblock2_text_markup_set(tb, "тest \u202bנסיוןabcנסיון\u202c bang");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   /* Logical cursor after "test " */
   evas_textblock2_cursor_pos_set(cur, 6);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 18);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 20);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before "a" */
   evas_textblock2_cursor_pos_set(cur, 11);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 10);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after "c" */
   evas_textblock2_cursor_pos_set(cur, 14);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 10);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before " bang" */
   evas_textblock2_cursor_pos_set(cur, 20);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 19);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor in the beginning */
   evas_textblock2_cursor_line_char_first(cur);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Logical cursor in the end */
   evas_textblock2_cursor_line_char_last(cur);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Logical cursor on the second pos */
   evas_textblock2_cursor_pos_set(cur, 2);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Split cursor in RTL paragraph. */
                                           /*                1           2
                                              01234  5   67890123456789  0   123456 */
   evas_object_textblock2_text_markup_set(tb, "שלום \u202atest עברית efl\u202c נסיון");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   /* Logical cursor before "test" */
   evas_textblock2_cursor_pos_set(cur, 6);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 4);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after "test " */
   evas_textblock2_cursor_pos_set(cur, 11);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 16);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 15);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before " efl" */
   evas_textblock2_cursor_pos_set(cur, 16);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 15);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after " efl" */
   evas_textblock2_cursor_pos_set(cur, 21);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 4);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor in the beginning */
   evas_textblock2_cursor_line_char_first(cur);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));

   /* Logical cursor in the end */
   evas_textblock2_cursor_line_char_last(cur);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 26);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Corner cases for split cursor. */

   /* End of line in LTR paragraph with embedding*/
                                             /*              1
                                              01234   5  678901234567   */
   evas_object_textblock2_text_markup_set(tb, "test \u202bנסיוןشسيبabc");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock2_cursor_line_char_last(cur);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 15);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));
   ck_assert_int_eq(cx2, x2);

   /* End of line in RTL paragraph */
                                          /*                 1         2
                                              012345678   9  01234567890123 */
   evas_object_textblock2_text_markup_set(tb, "נסיוןشسي \u202atestприветשלום");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock2_cursor_line_char_last(cur);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 8);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock2_cursor_pos_set(cur, 10);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Cursor is between items of the same direction */
   evas_textblock2_cursor_pos_set(cur, 14);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Cursor type is UNDER */
   evas_textblock2_cursor_pos_set(cur, 0);
   fail_if(evas_textblock2_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_UNDER));
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Multiline */
   Evas_Coord ly;
   int i;
                                                      /* 012345678901234 */
   evas_object_textblock2_text_markup_set(tb, "<wrap=char>testשלוםشسيبefl");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   nh = nh * 15;
   evas_object_resize(tb, nw, nh);

   for (i = 0; i < nw; i++)
     {
        evas_object_resize(tb, i, nh);

        evas_textblock2_cursor_pos_set(cur, 12);
        fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                         NULL, &cx2, &cy2, NULL, NULL,
                                                         EVAS_TEXTBLOCK2_CURSOR_BEFORE));
        evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy, ly);
        evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
        ck_assert_int_eq(cx, x);
        evas_textblock2_cursor_pos_set(cur, 11);
        evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy2, ly);
        evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
        ck_assert_int_eq(cx2, x2);
     }
                                                      /* 01234567890123456789 */
   evas_object_textblock2_text_markup_set(tb, "<wrap=char>נסיוןhelloприветשלום");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   nh = nh * 20;
   evas_object_resize(tb, nw, nh);

   for (i = 0; i < nw; i++)
     {
        evas_object_resize(tb, i, nh);
        evas_textblock2_cursor_pos_set(cur, 16);
        fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                         NULL, &cx2, &cy2, NULL, NULL,
                                                         EVAS_TEXTBLOCK2_CURSOR_BEFORE));
        evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy, ly);
        evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
        ck_assert_int_eq(cx, (x + w));
        evas_textblock2_cursor_pos_set(cur, 15);
        evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy2, ly);
        evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, &w2, NULL);
        ck_assert_int_eq(cx2, (x2 + w2));
     }

   /* Testing multiline, when only RTL item is in the line. */
                                                      /* 012345678901234567890123 */
   evas_object_textblock2_text_markup_set(tb, "<wrap=char>testtesttestтестשלוםشسيب");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock2_cursor_pos_set(cur, 15);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   /* Resizing textblock2, so RTL item will be on the next line.*/
   evas_object_resize(tb, x + w, nh);

   evas_textblock2_cursor_pos_set(cur, 24);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                    NULL, &cx2, &cy2, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 16);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));
   evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy, ly);

   evas_textblock2_cursor_pos_set(cur, 23);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx2, x2);
   evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy2, ly);

   /* Testing multiline, when only LTR item is in the line. */
                                                      /* 012345678901234567890123 */
   evas_object_textblock2_text_markup_set(tb, "<wrap=char>שלוםשלוםשלוםشسيبtestтест");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock2_cursor_pos_set(cur, 15);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   /* Resizing textblock2, so LTR item will be on the next line.*/
   evas_object_resize(tb, nw - x, nh);

   evas_textblock2_cursor_pos_set(cur, 24);
   fail_if(!evas_textblock2_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                    NULL, &cx2, &cy2, NULL, NULL,
                                                    EVAS_TEXTBLOCK2_CURSOR_BEFORE));
   evas_textblock2_cursor_pos_set(cur, 16);
   evas_textblock2_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy, ly);

   evas_textblock2_cursor_line_char_last(cur);
   evas_textblock2_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx2, x2);
   evas_textblock2_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy2, ly);

   END_TB_TEST();
}
END_TEST
#endif

START_TEST(evas_textblock2_format_removal)
{
   START_TB_TEST();
   int i;
   const char *buf = "Th<b>is a<a>tes</a>st</b>.";
   const Evas_Object_Textblock2_Node_Format *fnode;
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);
   evas_object_textblock2_text_markup_set(tb, buf);

   /* Remove the "b" pair. */
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ a"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- a"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Now also remove the a pair */
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Remove the "a" pair. */
   evas_object_textblock2_text_markup_set(tb, buf);

   fnode = evas_textblock2_node_format_first_get(tb);
   fnode = evas_textblock2_node_format_next_get(fnode);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Now also remove the b pair */
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Now remove formats by removing text */
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   /* Only b formats should remain */
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* No formats should remain. */
   evas_textblock2_cursor_pos_set(cur, 2);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Try to remove the formats in a way that shouldn't remove them */
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_pos_set(cur, 7);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   evas_textblock2_cursor_char_delete(cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ a"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- a"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Try range deletion to delete a */
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_pos_set(main_cur, 9);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Range deletion to delete both */
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_pos_set(cur, 2);
   evas_textblock2_cursor_pos_set(main_cur, 11);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Range deletion across paragraphs */
   evas_object_textblock2_text_markup_set(tb,
         "Th<b>is a<a>te" _PS ""
         "s</a>st</b>.");
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_pos_set(main_cur, 10);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Range deletion across paragraph - a bug found in elm. */
   evas_object_textblock2_text_markup_set(tb,
         "This is an entry widget in this window that" _PS ""
         "uses markup <b>like this</> for styling and" _PS ""
         "formatting <em>like this</>, as well as" _PS ""
         "<a href=X><link>links in the text</></a>, so enter text" _PS ""
         "in here to edit it. By the way, links are" _PS ""
         "called <a href=anc-02>Anchors</a> so you will need" _PS ""
         "to refer to them this way." _PS ""
         "" _PS ""

         "Also you can stick in items with (relsize + ascent): "
         "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
         " (full) "
         "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
         " (to the left)" _PS ""

         "Also (size + ascent): "
         "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
         " (full) "
         "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
         " (before this)" _PS ""

         "And as well (absize + ascent): "
         "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
         " (full) "
         "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
         " or even paths to image files on disk too like: "
         "<item absize=96x128 vsize=full href=file://bla/images/sky_01.jpg></item>"
         " ... end.");
   evas_textblock2_cursor_paragraph_first(cur);
   evas_textblock2_cursor_paragraph_last(main_cur);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(fnode);

   /* The first one below used to crash: empty value.
    * Test some invalid stuff doesn't segv. We force relayout as this will
    * trigger the parser. */
   evas_object_textblock2_text_markup_set(tb, "A<a=>");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock2_text_markup_set(tb, "A<=b>");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock2_text_markup_set(tb, "A<=>");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock2_text_markup_set(tb, "A<a='>");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock2_text_markup_set(tb, "A<a='");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);

   /* Deleting a range with just one char and surrounded by formats, that
    * deletes a paragraph. */
   evas_object_textblock2_text_markup_set(tb, "A" _PS "<b>B</b>");
   evas_textblock2_cursor_pos_set(cur, 2);
   evas_textblock2_cursor_pos_set(main_cur, 3);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Two formats in the same place. */
   evas_object_textblock2_text_markup_set(tb, "a<b><a>b</a></b>b");
   evas_textblock2_cursor_pos_set(cur, 1);
   evas_textblock2_cursor_char_delete(cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Two formats across different paragraphs with notihng in between. */
   evas_object_textblock2_text_markup_set(tb, "<b>" _PS "</b>");
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_char_delete(cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Try with range */
   evas_object_textblock2_text_markup_set(tb, "<b>" _PS "</b>");
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 1);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Range delete with empty paragraphs. */
   evas_object_textblock2_text_markup_set(tb, "" _PS "" _PS "" _PS "" _PS "" _PS "");
   evas_textblock2_cursor_pos_set(cur, 2);
   evas_textblock2_cursor_pos_set(main_cur, 3);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), "" _PS "" _PS "" _PS "" _PS "");

   /* Range delete with item formats, TEST_CASE#1 */
   evas_object_textblock2_text_markup_set(tb, "The <b>Multiline</b><item size=50x50 href=abc></item> text!");
   evas_textblock2_cursor_pos_set(cur, 4);
   evas_textblock2_cursor_pos_set(main_cur, 14);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), "The  text!");

   /* Range delete with item formats, TEST_CASE#2 */
   evas_object_textblock2_text_markup_set(tb, "The <b>Multiline</b><item size=50x50 href=abc></item> text! it is lon<item size=40x40 href=move></item>g text for test.");
   evas_textblock2_cursor_pos_set(cur, 14);
   evas_textblock2_cursor_pos_set(main_cur, 15);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), "The <b>Multiline</b><item size=50x50 href=abc></item>text! it is lon<item size=40x40 href=move></item>g text for test.");

   /* Verify fmt position and REP_CHAR positions are the same */
   evas_object_textblock2_text_markup_set(tb,
         "This is" _PS "an <item absize=93x152 vsize=ascent></>a.");
   evas_textblock2_cursor_pos_set(cur, 7);
   evas_textblock2_cursor_char_delete(cur);
   fnode = evas_textblock2_node_format_first_get(tb);
   /* FIXME:  to fix in Evas.h */
/*    fail_if(_evas_textblock2_format_offset_get(fnode) != 10); */

   /* Out of order <b><i></b></i> mixes. */
   evas_object_textblock2_text_markup_set(tb, "a<b>b<i>c</b>d</i>e");
   evas_textblock2_cursor_pos_set(cur, 2);

   for (i = 0 ; i < 2 ; i++)
     {
        fnode = evas_textblock2_node_format_first_get(tb);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ i"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- b"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- i"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (fnode);

        evas_textblock2_cursor_char_delete(cur);
     }
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* This time with a generic closer */
   evas_object_textblock2_text_markup_set(tb, "a<b>b<i>c</b>d</>e");
   evas_textblock2_cursor_pos_set(cur, 2);

   for (i = 0 ; i < 2 ; i++)
     {
        fnode = evas_textblock2_node_format_first_get(tb);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ i"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- b"));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

        fnode = evas_textblock2_node_format_next_get(fnode);
        fail_if (fnode);

        evas_textblock2_cursor_char_delete(cur);
     }
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* And now with remove pair. */
   evas_object_textblock2_text_markup_set(tb, "a<b>b<i>c</b>d</i>e");
   evas_textblock2_cursor_pos_set(cur, 2);
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ i"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Remove the other pair */
   evas_object_textblock2_text_markup_set(tb, "a<b>b<i>c</>d</i>e");
   evas_textblock2_cursor_pos_set(cur, 2);
   fnode = evas_textblock2_node_format_first_get(tb);
   fnode = evas_textblock2_node_format_next_get(fnode);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   /* Remove two pairs with the same name and same positions. */
   evas_object_textblock2_text_markup_set(tb, "<a><a>A</a></a>");
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_char_delete(cur);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (fnode);

   /* Try to remove a format that doesn't have a pair (with a bad mkup) */
   evas_object_textblock2_text_markup_set(tb, "a<b>b<i>c</>d</i>e");
   evas_textblock2_cursor_pos_set(cur, 2);
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb,
         (Evas_Object_Textblock2_Node_Format *) fnode);

   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ i"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if (fnode);

   END_TB_TEST();
}
END_TEST

/* Testing items */
START_TEST(evas_textblock2_items)
{
   Evas_Coord w, h, w2, h2, nw, nh, ih;
   START_TB_TEST();
   const char *buf = "This is an <item absize=93x152></>.";

   /* Absolute item size */
   buf = "This is an <item absize=93x152 vsize=full></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 93);
   ck_assert_int_eq(h, 152);

   buf = "This is an <item absize=93x152 vsize=ascent></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 93);
   ck_assert_int_eq(h, 152);

   /* Size is the same as abssize, unless there's scaling applied. */
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h <= 153));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   evas_object_scale_set(tb, 2.0);
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h != (2 * 154)));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h <= (2 * 154)));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   evas_object_scale_set(tb, 1.0);

   /* Relsize */
   /* relsize means it should adjust itself to the size of the line */
   buf = "This is an <item relsize=93x152 vsize=full></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 153));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 108) || (h <= ih));

   buf = "This is an <item relize=93x152 vsize=ascent></>.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 152));
   evas_textblock2_cursor_pos_set(cur, 11);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 108) || (h <= ih));

   /* Relsize and abs size in the same line, all should be the same size */
   buf = "<item relsize=64x64 vsize=ascent href=emoticon/knowing-grin></item><item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item><item relsize=64x64 vsize=ascent href=emoticon/knowing-grin></item>";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   fail_if((nw != w) || (nh != h));
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w2, &h2);
   fail_if((w != w2) || (h != h2));
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w2, &h2);
   fail_if((w != w2) || (h != h2));

   buf = "<ellipsis=1.0>a<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item></ellipsis>";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_resize(tb, 30, 30);
   evas_textblock2_cursor_pos_set(cur, 1);
   if (evas_textblock2_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h))
     fail_if((w != 64) || (h != 64));

   /* FIXME: Also verify x,y positions of the item. */

   /* FIXME We need some item tests that involve line wrapping that make the
    * items move between lines that are in different sizes.
    * Also, tests that involve wrapping positions with relsized items. We
    * want to make sure the item gets a relsize on the correct time (before
    * the wrapping, and then is updated after the wrapping) and that
    * all the lines have the correct sizes afterwards. */

   END_TB_TEST();
}
END_TEST

/* Wrapping tests */
START_TEST(evas_textblock2_wrapping)
{
   Evas_Coord bw, bh, w, h, nw, nh;
   int i;
   START_TB_TEST();
   evas_object_textblock2_text_markup_set(tb, "a");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);

   /* Char wrap */
   evas_object_textblock2_text_markup_set(tb, "aaaaaaa");
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Mixed - fallback to char wrap */
   evas_object_textblock2_text_markup_set(tb, "aaaaaaa");
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Basic Word wrap */
   evas_object_textblock2_text_markup_set(tb, "aaaa");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);

   evas_object_textblock2_text_markup_set(tb, "aaaa aa");
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Mixed - fallback to word wrap */
   evas_object_textblock2_text_markup_set(tb, "aaaa aa");
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, bw + 1, bh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   ck_assert_int_eq(w, bw);
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Wrap and then expand again. */
   evas_object_textblock2_text_markup_set(tb, "aaaa aa");
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   /* Reduce size until reaching the minimum, making sure we don't
    * get something wrong along the way */
   /* Char wrap */
   evas_object_textblock2_text_markup_set(tb, "a");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock2_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa" _PS ""
         "aaaa aaa aaa aaa aaa" _PS ""
         "a aaaaa aaaaaaaaaaaaaa\naaaaa" _PS ""
         "aaaaaa"
         );
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);

   Evas_Coord iw;
   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Verify that no empty line is added */
   evas_object_textblock2_text_markup_set(tb, "<wrap=word>Hello</wrap>");
   evas_object_textblock2_size_native_get(tb, NULL, &nh);
   evas_object_resize(tb, 0, 1000);
   evas_object_textblock2_size_formatted_get(tb, NULL, &h);
   ck_assert_int_eq(nh, h);

   evas_object_textblock2_text_markup_set(tb, "<wrap=char>a</wrap>");
   evas_object_textblock2_size_native_get(tb, NULL, &nh);
   evas_object_resize(tb, 0, 1000);
   evas_object_textblock2_size_formatted_get(tb, NULL, &h);
   ck_assert_int_eq(nh, h);

   /* Word wrap */
   evas_object_textblock2_text_markup_set(tb, "aaaaaa");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock2_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa" _PS ""
         "aaaa aaa aaa aaa aaa" _PS ""
         "a aaaaa aaaaaa\naaaaa" _PS ""
         "aaaaa"
         );
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Mixed wrap */
   evas_object_textblock2_text_markup_set(tb, "a");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock2_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa" _PS ""
         "aaaa aaa aaa aaa aaa" _PS ""
         "a aaaaa aaaaaa\naaaaa" _PS ""
         "aaaaa"
         );
   evas_textblock2_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Resize, making sure we keep going down in the minimum size. */
   char *wrap_style[] = { "+ wrap=word", "+ wrap=char", "+ wrap=mixed" };
   int wrap_items = sizeof(wrap_style) / sizeof(*wrap_style);

   evas_object_textblock2_text_markup_set(tb,
         "This is an entry widget in this window that\n"
         "uses markup <b>like this</> for styling and\n"
         "formatting <em>like this</>, as well as\n"
         "<a href=X><link>links in the text</></a>, so enter text\n"
         "in here to edit it. By the way, links are\n"
         "called <a href=anc-02>Anchors</a> so you will need\n"
         "to refer to them this way.\n"
         "\n"

         "Also you can stick in items with (relsize + ascent): "
         "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
         " (full) "
         "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
         " (to the left)\n"

         "Also (size + ascent): "
         "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
         " (full) "
         "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
         " (before this)\n"

         "And as well (absize + ascent): "
         "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
         " (full) "
         "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
         " or even paths to image files on disk too like: "
         "<item absize=96x128 vsize=full href=file://%s/images/sky_01.jpg></item>"
         " ... end."
         );

   /* Get minimum size */
   evas_object_textblock2_size_native_get(tb, &nw, &nh);

   for (i = 0 ; i < wrap_items ; i++)
     {
        evas_textblock2_cursor_format_prepend(cur, wrap_style[i]);
        evas_object_resize(tb, 0, 0);
        evas_object_textblock2_size_formatted_get(tb, &bw, &bh);

        for (iw = nw ; iw >= bw ; iw--)
          {
             evas_object_resize(tb, iw, 1000);
             evas_object_textblock2_size_formatted_get(tb, &w, &h);
             fail_if(w < bw);
             fail_if(w > iw);
          }
        fail_if(w != bw);
     }


   /* Ellipsis */
   int ellip_w = 0;
   evas_object_textblock2_text_markup_set(tb, "…");
   evas_object_textblock2_size_native_get(tb, &ellip_w, NULL);

   evas_object_textblock2_text_markup_set(tb, "aaaaaaaaaaaaaaaaaa\nb");
   evas_textblock2_cursor_format_prepend(cur, "+ ellipsis=1.0 wrap=word");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, (nw / 2));

   evas_object_textblock2_text_markup_set(tb, "a<b>b</b>a<b>b</b>a<b>b</b>");
   evas_textblock2_cursor_format_prepend(cur, "+ font_size=50 ellipsis=1.0");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, (nw / 2));

   evas_object_textblock2_text_markup_set(tb, "<item absize=100x100 href=item1></item><item absize=100x100 href=item2></item>");
   evas_textblock2_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_resize(tb, 101, 100);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, 100);

   evas_object_textblock2_text_markup_set(tb, "ab");
   evas_textblock2_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, ellip_w);

   {
      double ellip;
      for(ellip = 0.0; ellip <= 1.0; ellip = ellip + 0.1)
        {
           char buf[128];
           Evas_Coord w1, h1, w2, h2;

           sprintf(buf, "+ ellipsis=%f", ellip);
           evas_object_textblock2_text_markup_set(tb, "aaaaaaaaaa");
           evas_textblock2_cursor_format_prepend(cur, buf);
           evas_object_textblock2_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock2_size_formatted_get(tb, &w, &h);
           ck_assert_int_le(w, (nw / 2));
           ck_assert_int_eq(h, nh);

           evas_object_textblock2_text_markup_set(tb, "aaaaaaaaaa");
           evas_textblock2_cursor_format_prepend(cur, buf);
           evas_object_textblock2_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw, nh);
           evas_object_textblock2_size_formatted_get(tb, &w, &h);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock2_size_formatted_get(tb, &w1, &h1);
           evas_object_resize(tb, nw, nh);
           evas_object_textblock2_size_formatted_get(tb, &w2, &h2);
           ck_assert_int_eq(w, w2);
           ck_assert_int_eq(h, h2);

           sprintf(buf, "+ ellipsis=%f", ellip);
           evas_object_textblock2_text_markup_set(tb,
                 "the<tab>quick brown fox"
                 "jumps<tab> over the<tab> lazy dog"
                 );
           evas_textblock2_cursor_format_prepend(cur, buf);
           evas_object_textblock2_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock2_size_formatted_get(tb, &w, &h);
           ck_assert_int_le(w, (nw / 2));
           ck_assert_int_eq(h, nh);
        }
   }

   evas_object_textblock2_text_markup_set(tb, "aaaaaaaaaaaaaaaaaa");
   evas_object_textblock2_size_native_get(tb, &nw, &nh);

   evas_textblock2_cursor_format_prepend(cur, "+ ellipsis=1.0 wrap=char");
   nw /= 3;
   nh *= 2;
   evas_object_resize(tb, nw, nh);

     {
        evas_object_textblock2_line_number_geometry_get(tb, 1, NULL, NULL, &w, NULL);
        ck_assert_int_gt(w, ellip_w);
     }

   /* Word wrap ending with whites. */
   evas_object_resize(tb, 322, 400);
   evas_object_textblock2_text_markup_set(tb, "<wrap=word>This is an example text that should break at the end aaa     ");

   evas_textblock2_cursor_paragraph_last(cur);
   Evas_Coord cx, cy, cw, ch;
   evas_textblock2_cursor_text_prepend(cur, " ");
   fail_if(-1 == evas_textblock2_cursor_geometry_get(cur, &cx, &cy, &cw, &ch,
            NULL, EVAS_TEXTBLOCK2_CURSOR_BEFORE));

   /* Getting whites back after wrapping. */
   evas_object_resize(tb, 1, 1);
   evas_object_textblock2_text_markup_set(tb, "<wrap=word><keyword>return</keyword> <number>0</number>;</wrap>");

   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, 32);
   _ck_assert_int(h, >=, 25);

   evas_object_resize(tb, 400, 400);

   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 44);
   ck_assert_int_eq(h, 16);

   /* Complex compound clusters using Devanagari. */
   evas_object_resize(tb, 0, 0);

   evas_object_textblock2_text_markup_set(tb, "<wrap=char> करेंकरेंकरेंकरेंकरेंकरें");
   evas_object_textblock2_size_formatted_get(tb, &w, &h);

   fail_if(w > h); /* FIXME: Not the best test, should be more strict. */

   END_TB_TEST();
}
END_TEST

/* Various textblock2 stuff */
START_TEST(evas_textblock2_various)
{
   Evas_Coord w, h, bw, bh;
   START_TB_TEST();
   const char *buf = "This" _PS "textblock2" _PS "has" _PS "a" _PS "lot" _PS "of" _PS "lines" _PS ".";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   /* Move outside of the screen so it'll have to search for the correct
    * paragraph and etc. */
   evas_object_move(tb, -(w / 2), -(h / 2));

   /* Replacement char */
   evas_object_textblock2_text_markup_set(tb, "*");
   evas_object_textblock2_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock2_replace_char_set(tb, "*");
   evas_object_textblock2_text_markup_set(tb, "|");
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   fail_if((w != bw) || (h != bh));

   /* Items have correct text node information */
   /* FIXME:  to fix in Evas.h */
   evas_object_textblock2_text_markup_set(tb, "");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */
   evas_object_textblock2_text_markup_set(tb, "" _PS "");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */
   evas_object_textblock2_text_markup_set(tb, "a" _PS "");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */
   evas_object_textblock2_text_markup_set(tb, "a" _PS "a");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */
   evas_object_textblock2_text_markup_set(tb, "a" _PS "a" _PS "");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */
   evas_object_textblock2_text_markup_set(tb, "a" _PS "a" _PS "a");
/*    fail_if(!_evas_textblock2_check_item_node_link(tb)); */

   /* These shouldn't crash (although the desired outcome is not yet defined) */
   evas_object_textblock2_text_markup_set(tb, "&#xfffc;");
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_char_delete(cur);

   evas_object_textblock2_text_markup_set(tb, "\xEF\xBF\xBC");
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_char_delete(cur);

   /* Check margins' position */
     {
        Evas_Coord nw, nh, lx, lw;

        evas_object_textblock2_text_markup_set(tb, "This is a test");
        evas_object_textblock2_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock2_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 0);
        ck_assert_int_eq(lx + lw, nw);

        evas_object_textblock2_text_markup_set(tb, "<left_margin=10 right_margin=5>This is a test</>");
        evas_object_textblock2_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock2_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 10);
        ck_assert_int_eq(lx + lw + 5, nw);

        evas_object_textblock2_text_markup_set(tb, "עוד פסקה");
        evas_object_textblock2_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock2_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 0);
        ck_assert_int_eq(lx + lw, nw);

        evas_object_textblock2_text_markup_set(tb, "<left_margin=10 right_margin=5>עוד פסקה</>");
        evas_object_textblock2_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock2_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 10);
        ck_assert_int_eq(lx + lw + 5, nw);
     }

   /* Super big one line item. */
     {
#define CNT 10000
        char str[(CNT * 6) + 128], *d;
        const char substr[] = "x";
        Evas_Textblock2_Style *stt;
        int i, l;

        l = strlen(substr);
        d = str;
        for (i = 0; i < CNT; i++)
          {
             memcpy(d, substr, l);
             d += l;
          }
        *d = 0;

        stt = evas_textblock2_style_new();
        evas_textblock2_style_set(stt,
              "DEFAULT='" TEST_FONT " font_size=10 align=left color=#000000 wrap=char'");
        evas_object_textblock2_style_set(tb, stt);
        evas_textblock2_style_free(stt);

        evas_object_textblock2_text_markup_set(tb, substr);
        Evas_Textblock2_Cursor *cr;

        cr = evas_object_textblock2_cursor_get(tb);
        evas_textblock2_cursor_text_append(cr, str);
        evas_object_resize(tb, 480, 800);

        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        fail_if(w == 0);
     }

   END_TB_TEST();
}
END_TEST

/* Various geometries. e.g. range geometry. */
START_TEST(evas_textblock2_geometries)
{
   START_TB_TEST();
   const char *buf = "This is a \n test.";
   evas_object_textblock2_text_markup_set(tb, buf);

   /* Single line range */
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 6);

   Eina_List *rects, *rects2;
   Evas_Textblock2_Rectangle *tr, *tr2;
   rects = evas_textblock2_cursor_range_geometry_get(cur, main_cur);
   fail_if(!rects);
   rects2 = evas_textblock2_cursor_range_geometry_get(main_cur, cur);
   fail_if(!rects2);

   fail_if(eina_list_count(rects) != 1);
   fail_if(eina_list_count(rects2) != 1);

   tr = eina_list_data_get(rects);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(rects2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
         (tr->h != tr2->h));

   EINA_LIST_FREE(rects, tr)
      free(tr);
   EINA_LIST_FREE(rects2, tr2)
      free(tr2);

   /* Multiline range */
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 14);

   rects = evas_textblock2_cursor_range_geometry_get(cur, main_cur);
   fail_if(!rects);
   rects2 = evas_textblock2_cursor_range_geometry_get(main_cur, cur);
   fail_if(!rects2);

   fail_if(eina_list_count(rects) != 2);
   fail_if(eina_list_count(rects2) != 2);

   tr = eina_list_data_get(rects);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(rects2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
         (tr->h != tr2->h));

   tr = eina_list_data_get(eina_list_next(rects));
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(eina_list_next(rects2));
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
         (tr->h != tr2->h));

   /* Check that the second line is positioned below the first */
   tr = eina_list_data_get(rects);
   tr2 = eina_list_data_get(eina_list_next(rects));
   fail_if(tr->y >= tr2->y);

   EINA_LIST_FREE(rects, tr)
      free(tr);
   EINA_LIST_FREE(rects2, tr2)
      free(tr2);

   /* Same run different scripts */
   evas_object_textblock2_text_markup_set(tb, "עבריתenglishрусскийעברית");

   evas_textblock2_cursor_pos_set(cur, 3);
   evas_textblock2_cursor_pos_set(main_cur, 7);
   rects = evas_textblock2_cursor_range_geometry_get(cur, main_cur);

   fail_if(eina_list_count(rects) != 2);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   /* Same run different styles */
   evas_object_textblock2_text_markup_set(tb, "test<b>test2</b>test3");

   evas_textblock2_cursor_pos_set(cur, 3);
   evas_textblock2_cursor_pos_set(main_cur, 11);
   rects = evas_textblock2_cursor_range_geometry_get(cur, main_cur);

   fail_if(eina_list_count(rects) != 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   /* Bidi text with a few back and forth from bidi. */
   evas_object_textblock2_text_markup_set(tb, "נגכדגךלח eountoheunth ךלחגדךכלח");

   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 28);
   rects = evas_textblock2_cursor_range_geometry_get(cur, main_cur);

   ck_assert_int_eq(eina_list_count(rects), 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   /* Range simple geometry */
   /* Single line range */
   Eina_Iterator *it, *it2;
   Evas_Textblock2_Rectangle *tr3;
   Evas_Coord w = 200;
   evas_object_textblock2_text_markup_set(tb, "This \n is a test.\nAnother \ntext.");
   evas_object_resize(tb, w, w / 2);
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 3);

   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock2_cursor_range_simple_geometry_get(main_cur, cur);
   fail_if(!it2);
   rects2 = eina_iterator_container_get(it2);
   fail_if(!rects2);

   fail_if(eina_list_count(rects) != 1);
   fail_if(eina_list_count(rects2) != 1);

   tr = eina_list_data_get(rects);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(rects2);
   fail_if((tr->h <= 0) || (tr->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   EINA_LIST_FREE(rects, tr)
      free(tr);
   EINA_LIST_FREE(rects2, tr2)
      free(tr2);
   eina_iterator_free(it);
   eina_iterator_free(it2);

   /* Multiple range */
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 16);

   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock2_cursor_range_simple_geometry_get(main_cur, cur);
   fail_if(!it2);
   rects2 = eina_iterator_container_get(it2);
   fail_if(!rects2);

   fail_if(eina_list_count(rects) != 3);
   fail_if(eina_list_count(rects2) != 3);

   tr = eina_list_data_get(rects);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(rects2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   tr = eina_list_nth(rects, 1);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(eina_list_next(rects2));
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   tr = eina_list_nth(rects, 2);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_nth(rects2, 2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));

   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   /* Check that the second line is positioned below the first */
   tr = eina_list_data_get(rects);
   tr2 = eina_list_nth(rects, 1);
   tr3 = eina_list_nth(rects, 2);
   fail_if((tr->y >= tr3->y) || (tr2->y >= tr3->y));
   fail_if(tr2->x + tr2->w != w);

   /* Have middle rectangle */
   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 31);
   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock2_cursor_range_simple_geometry_get(main_cur, cur);
   fail_if(!it2);
   rects2 = eina_iterator_container_get(it2);
   fail_if(!rects2);

   fail_if(eina_list_count(rects) != 4);
   fail_if(eina_list_count(rects) != 4);

   tr = eina_list_data_get(rects);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_data_get(rects2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));
   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
      (tr->h != tr2->h));

   tr = eina_list_nth(rects, 1);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_nth(rects2, 1);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));
   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   tr = eina_list_nth(rects, 2);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_nth(rects2, 2);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));
   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   tr = eina_list_nth(rects, 3);
   fail_if((tr->h <= 0) || (tr->w <= 0));
   tr2 = eina_list_nth(rects2, 3);
   fail_if((tr2->h <= 0) || (tr2->w <= 0));
   fail_if((tr->x != tr2->x) || (tr->y != tr2->y) || (tr->w != tr2->w) ||
           (tr->h != tr2->h));

   /* Check that the middle rectanlge is between first and last rectangles */
   tr = eina_list_data_get(rects);
   tr2 = eina_list_nth(rects, 2);
   tr3 = eina_list_nth(rects, 3);
   fail_if((tr2->y < tr->y + tr->h) || (tr2->y + tr2->h > tr3->y));

   /* Check that the middle rectangle has proper width */
   tr = eina_list_data_get(rects);
   tr2 = eina_list_nth(rects, 1);
   fail_if((tr->y != tr2->y) || (tr->h != tr2->h));
   tr3 = eina_list_nth(rects, 2);
   fail_if((tr2->x + tr2->w != w) || (tr2->x + tr2->w != tr3->x + tr3->w));
   tr2 = eina_list_nth(rects, 2);
   tr3 = eina_list_nth(rects, 3);
   fail_if((tr2->x != tr3->x));

   EINA_LIST_FREE(rects, tr)
      free(tr);
   EINA_LIST_FREE(rects2, tr2)
      free(tr2);
   eina_iterator_free(it);
   eina_iterator_free(it2);

   /* Same run different scripts */
   evas_object_textblock2_text_markup_set(tb, "עבריתenglishрусскийעברית");

   evas_textblock2_cursor_pos_set(cur, 3);
   evas_textblock2_cursor_pos_set(main_cur, 7);
   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);

   fail_if(eina_list_count(rects) != 2);

   EINA_LIST_FREE(rects, tr)
      free(tr);
   eina_iterator_free(it);

   /* Same run different styles */
   evas_object_textblock2_text_markup_set(tb, "test<b>test2</b>test3");

   evas_textblock2_cursor_pos_set(cur, 3);
   evas_textblock2_cursor_pos_set(main_cur, 11);
   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);

   fail_if(eina_list_count(rects) != 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);
   eina_iterator_free(it);

   /* Bidi text with a few back and forth from bidi. */
   evas_object_textblock2_text_markup_set(tb, "נגכדגךלח eountoheunth ךלחגדךכלח");

   evas_textblock2_cursor_pos_set(cur, 0);
   evas_textblock2_cursor_pos_set(main_cur, 28);
   it = evas_textblock2_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);

   ck_assert_int_eq(eina_list_count(rects), 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);
   eina_iterator_free(it);

   END_TB_TEST();
}
END_TEST

/* Should handle all the text editing. */
START_TEST(evas_textblock2_editing)
{
   START_TB_TEST();
   const char *buf = "First par." _PS "Second par.";
   evas_object_textblock2_text_markup_set(tb, buf);
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);

   /* Check deletion works */
   /* Try deleting after the end of the textblock2 */
     {
        char *content;
        evas_textblock2_cursor_paragraph_last(cur);
        content = strdup(evas_object_textblock2_text_markup_get(tb));
        evas_textblock2_cursor_char_delete(cur);
        fail_if(strcmp(content, evas_object_textblock2_text_markup_get(tb)));
        free(content);
     }

   /* Delete the first char */
   evas_textblock2_cursor_paragraph_first(cur);
   evas_textblock2_cursor_char_delete(cur);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb),
            "irst par." _PS "Second par."));

   /* Delete some arbitrary char */
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_char_delete(cur);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb),
            "irs par." _PS "Second par."));

   /* Delete a range */
   evas_textblock2_cursor_pos_set(main_cur, 1);
   evas_textblock2_cursor_pos_set(cur, 6);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb),
            "ir." _PS "Second par."));
   evas_textblock2_cursor_paragraph_char_first(main_cur);
   evas_textblock2_cursor_paragraph_char_last(cur);
   evas_textblock2_cursor_char_next(cur);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb),
            "Second par."));

   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_paragraph_last(main_cur);
   evas_object_textblock2_text_markup_prepend(main_cur, "Test<b>bla</b>bla.");
   evas_textblock2_cursor_paragraph_last(cur);
   evas_textblock2_cursor_paragraph_char_first(main_cur);
   evas_textblock2_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb),
            "First par." _PS ""));

   /* Merging paragraphs */
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_paragraph_char_last(cur);
   evas_textblock2_cursor_copy(cur, main_cur);
   evas_textblock2_cursor_char_delete(cur);

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

   /* Split paragraphs */
   evas_textblock2_cursor_format_prepend(cur, "ps");

   evas_textblock2_cursor_paragraph_first(cur);
   fail_if(!evas_textblock2_cursor_paragraph_next(cur));
   fail_if(evas_textblock2_cursor_paragraph_next(cur));

   /* Merge paragraphs using range deletion */
   evas_object_textblock2_text_markup_set(tb, buf);
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
        evas_object_textblock2_text_markup_set(tb, "this is a test eauoeuaou" _PS "this is a test1" _PS "this is a test 3");
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
        evas_object_textblock2_text_markup_set(tb, "a\n");
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        evas_object_textblock2_text_markup_set(tb, "a\t");
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        evas_object_textblock2_text_markup_set(tb, "a\xEF\xBF\xBC");
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);

        evas_object_textblock2_text_markup_set(tb, "a\xE2\x80\xA9");
        evas_textblock2_cursor_pos_set(cur, 1);
        content = evas_textblock2_cursor_content_get(cur);
        (void) content;
     }

   /* FIXME: Also add text appending/prepending */

   END_TB_TEST();
}
END_TEST

/* Text getters */
START_TEST(evas_textblock2_text_getters)
{
   START_TB_TEST();
   const char *buf = "This is a \n test." _PS ""
      "טקסט בעברית" _PS "and now in english.";
   evas_object_textblock2_text_markup_set(tb, buf);
   evas_textblock2_cursor_paragraph_first(cur);

   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "This is a \n test."));

   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "טקסט בעברית"));

   evas_textblock2_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock2_cursor_paragraph_text_get(cur),
            "and now in english."));

   /* Range get */
   Evas_Textblock2_Cursor *main_cur = evas_object_textblock2_cursor_get(tb);
   evas_textblock2_cursor_pos_set(main_cur, 2);
   evas_textblock2_cursor_pos_set(cur, 2);
   fail_if(*evas_textblock2_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP));

   evas_textblock2_cursor_pos_set(main_cur, 2);
   evas_textblock2_cursor_pos_set(cur, 6);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "is i"));

   evas_textblock2_cursor_pos_set(main_cur, 5);
   evas_textblock2_cursor_pos_set(cur, 14);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "is a \n te"));

   evas_textblock2_cursor_pos_set(main_cur, 14);
   evas_textblock2_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "st." _PS "טק"));

   evas_textblock2_cursor_pos_set(main_cur, 14);
   evas_textblock2_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "st." _PS "טקסט בעברית" _PS "an"));

   /* Backward range get */
   evas_textblock2_cursor_pos_set(main_cur, 2);
   evas_textblock2_cursor_pos_set(cur, 2);
   fail_if(*evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP));

   evas_textblock2_cursor_pos_set(main_cur, 2);
   evas_textblock2_cursor_pos_set(cur, 6);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "is i"));

   evas_textblock2_cursor_pos_set(main_cur, 5);
   evas_textblock2_cursor_pos_set(cur, 14);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "is a \n te"));

   evas_textblock2_cursor_pos_set(main_cur, 14);
   evas_textblock2_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "st." _PS "טק"));

   evas_textblock2_cursor_pos_set(main_cur, 14);
   evas_textblock2_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "st." _PS "טקסט בעברית" _PS "an"));

   /* Uninit cursors and other weird cases */
   evas_object_textblock2_clear(tb);
   evas_textblock2_cursor_copy(main_cur, cur);
   evas_textblock2_cursor_text_prepend(main_cur, "aaa");
   fail_if(strcmp(evas_textblock2_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK2_TEXT_MARKUP), "aaa"));

   /* Markup to plain and vice versa */
     {
        char *tmp, *tmp2;

        /* Real textblock2 object */
        tmp = evas_textblock2_text_markup_to_utf8(tb, "\naa<\n/>bb<\t/>");
        fail_if(strcmp(tmp, "\naa\nbb\t"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "\naa\nbb<tab/>"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a<item></item>");
        fail_if(strcmp(tmp, "a\xEF\xBF\xBC"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&#xfffc;"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a&nbsp;");
        fail_if(strcmp(tmp, "a\xC2\xA0"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a\xC2\xA0"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a<b>b</b><more></>a");
        fail_if(strcmp(tmp, "aba"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "aba"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a&amp;a");
        fail_if(strcmp(tmp, "a&a"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&amp;a"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a<newline/>a");
        fail_if(strcmp(tmp, "a\na"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a\na"));
        free(tmp2);
        free(tmp);

        /* NULL textblock2 object */
        tmp = evas_textblock2_text_markup_to_utf8(NULL, "\naa<\n/>bb<\t/>");
        fail_if(strcmp(tmp, "\naa\nbb\t"));
        tmp2 = evas_textblock2_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "\naa\nbb<tab/>"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(NULL, "a<item></item>");
        fail_if(strcmp(tmp, "a\xEF\xBF\xBC"));
        tmp2 = evas_textblock2_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "a&#xfffc;"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(NULL, "a&nbsp;");
        fail_if(strcmp(tmp, "a\xC2\xA0"));
        tmp2 = evas_textblock2_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "a\xC2\xA0"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(NULL, "a<b>b</b><more></>a");
        fail_if(strcmp(tmp, "aba"));
        tmp2 = evas_textblock2_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "aba"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(tb, "a&amp;a");
        fail_if(strcmp(tmp, "a&a"));
        tmp2 = evas_textblock2_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&amp;a"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock2_text_markup_to_utf8(NULL, "a<newline/>a");
        fail_if(strcmp(tmp, "aa"));
        tmp2 = evas_textblock2_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "aa"));
        free(tmp2);
        free(tmp);
     }

   /* complex markup set/get */
     {
        const char *text =
           "This is an entry widget in this window that" _PS ""
           "uses markup <b>like this</> for styling and" _PS ""
           "formatting <em>like this</>, as well as" _PS ""
           "<a href=X><link>links in the text</></a>, so enter text" _PS ""
           "in here to edit it. By the way, links are" _PS ""
           "called <a href=anc-02>Anchors</a> so you will need" _PS ""
           "to refer to them this way." _PS ""
           "" _PS ""

           "Also you can stick in items with (relsize + ascent): "
           "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
           " (full) "
           "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
           " (to the left)" _PS ""

           "Also (size + ascent): "
           "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
           " (full) "
           "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
           " (before this)" _PS ""

           "And as well (absize + ascent): "
           "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
           " (full) "
           "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
           " or even paths to image files on disk too like: "
           "<item absize=96x128 vsize=full href=file://bla/images/sky_01.jpg></item>"
           " ... end.";
        evas_object_textblock2_text_markup_set(tb, text);
        ck_assert_str_eq(text, evas_object_textblock2_text_markup_get(tb));
     }

   /* complex markup range get */
     {
        const char *text = "Break tag tes<item size=40x40 href=a></item>t \nNext\n line with it<item size=40x40 href=i></item>em tag";
        evas_object_textblock2_text_markup_set(tb, text);
        evas_textblock2_cursor_pos_set(main_cur, 14);
        evas_textblock2_cursor_pos_set(cur, 37);
        fail_if(strcmp(evas_textblock2_cursor_range_text_get(main_cur, cur,
                 EVAS_TEXTBLOCK2_TEXT_MARKUP), "</item>t \nNext\n line with it<item size=40x40 href=i></item>e"));
     }

   END_TB_TEST();
}
END_TEST

/* Formats */
START_TEST(evas_textblock2_formats)
{
   START_TB_TEST();
   const char *buf = "Th<b>i<font_size=15 wrap=none>s i</font_size=13>s</> a \n te" _PS "st<item></>.";
   const Evas_Object_Textblock2_Node_Format *fnode;
   evas_object_textblock2_text_markup_set(tb, buf);

   /* Walk from the start */
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ font_size=15 wrap=none"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- font_size=13"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "br"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "ps"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ item"));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(fnode);

   /* Walk backwards */
   fnode = evas_textblock2_node_format_last_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ item"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "ps"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "br"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- "));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "- font_size=13"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode),
            "+ font_size=15 wrap=none"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock2_node_format_prev_get(fnode);
   fail_if(fnode);

   /* Cursor and format detection */
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_format_is_visible_get(cur));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_format_is_visible_get(cur));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_format_is_visible_get(cur));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock2_cursor_format_is_visible_get(cur));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));

   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock2_cursor_at_format_set(cur, fnode);
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));

   size_t i = 0;
   evas_textblock2_cursor_paragraph_first(cur);
   do
     {
        switch (i)
          {
           case 2:
           case 3:
           case 6:
           case 7:
           case 10:
           case 14:
           case 17:
           case 18:
              fail_if(!evas_textblock2_cursor_is_format(cur));
              break;
           default:
              fail_if(evas_textblock2_cursor_is_format(cur));
              fail_if(evas_textblock2_cursor_format_is_visible_get(cur));
              break;
          }
        i++;
     }
   while (evas_textblock2_cursor_char_next(cur));

   /* Format text nodes invalidation */
     {
        Evas_Coord w, h, nw, nh;
        evas_object_textblock2_text_markup_set(tb, "Test");
        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        evas_textblock2_cursor_paragraph_first(cur);
        evas_textblock2_cursor_format_prepend(cur, "+ font_size=40");
        evas_object_textblock2_size_formatted_get(tb, &nw, &nh);
        fail_if((w >= nw) || (h >= nh));
     }
   /* FIXME: Should extend invalidation tests. */

   /* Various formats, just verify there's no seg, we can't really
    * verify them visually, well, we can some of them. Possibly in the
    * future we will */
   evas_object_textblock2_text_markup_set(tb,
         "<font_size=40>font_size=40</>" _PS ""
         "<color=#F210B3FF>color=#F210B3FF</>" _PS ""
         "<underline=single underline_color=#A2B3C4>underline=single underline_color=#A2B3C4</>" _PS ""
         "<underline=double underline_color=#F00 underline2_color=#00F>underline=double underline_color=#F00 underline2_color=#00F</>" _PS ""
         "<underline=dashed underline_dash_color=#0F0 underline_dash_width=2 underline_dash_gap=1>underline=dashed underline_dash_color=#0F0 underline_dash_width=2 underline_dash_gap=1</>" _PS ""
         "<style=outline outline_color=#F0FA>style=outline outline_color=#F0FA</>" _PS ""
         "<style=shadow shadow_color=#F0F>style=shadow shadow_color=#F0F</>" _PS ""
         "<style=glow glow_color=#BBB>style=glow glow_color=#BBB</>" _PS ""
         "<style=glow glow2_color=#0F0>style=glow glow2_color=#0F0</>" _PS ""
         "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214>style=glow color=#fff glow2_color=#fe87 glow_color=#f214</>" _PS ""
         "<backing=on backing_color=#00F>backing=on backing_color=#00F</>" _PS ""
         "<strikethrough=on strikethrough_color=#FF0>strikethrough=on strikethrough_color=#FF0</>" _PS ""
         "<align=right>align=right</>" _PS ""
         "<backing=on backing_color=#F008 valign=0.0>valign=0.0</>" _PS ""
         "<backing=on backing_color=#0F08 tabstops=50>tabstops=<\\t></>50</>" _PS ""
         "<backing=on backing_color=#00F8 linesize=40>linesize=40</>" _PS ""
         "<backing=on backing_color=#F0F8 linerelsize=200%>linerelsize=200%</>" _PS ""
         "<backing=on backing_color=#0FF8 linegap=20>linegap=20</>" _PS ""
         "<backing=on backing_color=#FF08 linerelgap=100%>linerelgap=100%</>" _PS "");

   /* Force a relayout */
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);

   /* Removing paired formats. */
   evas_object_textblock2_text_markup_set(tb, "<a>aa<b>bb</b>cc</a>");
   fnode = evas_textblock2_node_format_first_get(tb);
   evas_textblock2_node_format_remove_pair(tb, (Evas_Object_Textblock2_Node_Format *) fnode);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ b"));
   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- b"));

   evas_object_textblock2_text_markup_set(tb, "<a>aa<b>bb</b>cc</a>");
   fnode = evas_textblock2_node_format_first_get(tb);
   fnode = evas_textblock2_node_format_next_get(fnode);
   evas_textblock2_node_format_remove_pair(tb, (Evas_Object_Textblock2_Node_Format *) fnode);
   fnode = evas_textblock2_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ a"));
   fnode = evas_textblock2_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "- a"));

   /* Format list get */
   evas_object_textblock2_text_markup_set(tb, "<a>a</>a<item>b</>"
         "b<item>b</>c<a>c</>");
   const Eina_List *flist = evas_textblock2_node_format_list_get(tb, "a");
   const Eina_List *itr;
   EINA_LIST_FOREACH(flist, itr, fnode)
     {
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ a"));
     }

   flist = evas_textblock2_node_format_list_get(tb, "item");
   EINA_LIST_FOREACH(flist, itr, fnode)
     {
        fail_if(strcmp(evas_textblock2_node_format_text_get(fnode), "+ item"));
     }

   /* Make sure we get all the types of visible formats correctly. */
   evas_object_textblock2_text_markup_set(tb, "" _PS "a\na<tab/>a<item></>");
   fail_if(strcmp(evas_textblock2_node_format_text_get(
               evas_textblock2_cursor_format_get(cur)), "ps"));
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "" _PS ""));
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock2_node_format_text_get(
               evas_textblock2_cursor_format_get(cur)), "br"));
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "\n"));
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock2_node_format_text_get(
               evas_textblock2_cursor_format_get(cur)), "tab"));
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "<tab/>"));
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(!evas_textblock2_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock2_node_format_text_get(
               evas_textblock2_cursor_format_get(cur)), "+ item"));
   fail_if(strcmp(evas_textblock2_cursor_content_get(cur), "<item>"));
   fail_if(!evas_textblock2_cursor_format_is_visible_get(cur));

   evas_object_textblock2_text_markup_set(tb, "abc\ndef");
   evas_textblock2_cursor_pos_set(cur, 3);
   evas_object_textblock2_text_markup_prepend(cur, "<b></b>");
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), "abc<b></b>\ndef");
   evas_object_textblock2_text_markup_set(tb, "abc\ndef");
   evas_textblock2_cursor_pos_set(cur, 2);
   evas_object_textblock2_text_markup_prepend(cur, "<b></b>");
   ck_assert_str_eq(evas_object_textblock2_text_markup_get(tb), "ab<b></b>c\ndef");

   /* Ligatures cut by formats */
   evas_object_textblock2_text_markup_set(tb, "f<color=#f00>i</color>f");
   evas_object_textblock2_size_formatted_get(tb, NULL, NULL);

   END_TB_TEST();
}
END_TEST

/* Different text styles, for example, shadow. */
START_TEST(evas_textblock2_style)
{
   Evas_Coord w, h, nw, nh;
   Evas_Coord l, r, t, b;
   START_TB_TEST();
   Evas_Textblock2_Style *newst;
   const char *buf = "Test" _PS "Test2" _PS "נסיון";
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), buf));

   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   newst = evas_textblock2_style_new();
   fail_if(!newst);
   evas_textblock2_style_set(newst,
         "DEFAULT='" TEST_FONT " font_size=50 color=#000 text_class=entry'"
         "br='\n'"
         "ps='ps'"
         "tab='\t'");
   evas_object_textblock2_style_set(tb, newst);
   evas_object_textblock2_size_formatted_get(tb, &nw, &nh);
   fail_if((w >= nw) || (h >= nh));

   /* Style tag test */
   buf = "Test <br>\n<ps>" _PS "<tab><tab/>";
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(buf, evas_object_textblock2_text_markup_get(tb)));

   /* Style padding. */
   evas_object_textblock2_text_markup_set(tb, "Test");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 0) || (t != 0) || (b != 0));

   evas_object_textblock2_text_markup_set(tb, "<style=shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 1) || (t != 0) || (b != 1));

   evas_object_textblock2_text_markup_set(tb, "<style=outline>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 1) || (t != 1) || (b != 1));

   evas_object_textblock2_text_markup_set(tb, "<style=soft_outline>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_textblock2_text_markup_set(tb, "<style=glow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_textblock2_text_markup_set(tb, "<style=outline_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_textblock2_text_markup_set(tb, "<style=far_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_textblock2_text_markup_set(tb, "<style=outline_soft_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_textblock2_text_markup_set(tb, "<style=soft_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_textblock2_text_markup_set(tb, "<style=far_soft_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 4) || (t != 0) || (b != 4));

   /* Mixed style padding */
   evas_object_textblock2_text_markup_set(tb,
         "<style=far_shadow>Test</><style=far_soft_shadow>Test</>");
   evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 4) || (t != 1) || (b != 4));

   /* Multi-line padding */
   {
      Evas_Coord x[5] = {0}, y[5] = {0}, w2[5] = {0}, h2[5] = {0};

      // w2, h2 should not change between test 1 and 2
      // insets and x, y should increase by 2
      // line 1 in test 2 should have same geometry as in test 1 (despite style)

      evas_object_textblock2_text_markup_set(tb, "Test\nTest");
      evas_object_textblock2_line_number_geometry_get(tb, 0, &x[0], &y[0], &w2[0], &h2[0]);
      evas_object_textblock2_line_number_geometry_get(tb, 1, &x[1], &y[1], &w2[1], &h2[1]);

      // check line 1 geometry relatively to line 0
      ck_assert_int_eq(w2[0], w2[1]);
      fail_if((x[0] != x[1]) || ((y[0] + h2[0]) != y[1]) || (w2[0] != w2[1]) || (h2[0] != h2[1]));

      evas_object_textblock2_text_markup_set(tb, "Test\n<style=glow>Test</>\nTest");
      evas_object_textblock2_style_insets_get(tb, &l, &r, &t, &b);
      evas_object_textblock2_line_number_geometry_get(tb, 0, &x[2], &y[2], &w2[2], &h2[2]);
      evas_object_textblock2_line_number_geometry_get(tb, 1, &x[3], &y[3], &w2[3], &h2[3]);
      evas_object_textblock2_line_number_geometry_get(tb, 2, &x[4], &y[4], &w2[4], &h2[4]);

      // check line 1 geometry relatively to line 0
      fail_if((x[2] != x[3]) || ((y[2] + h2[2]) != y[3]) || (w2[2] > w2[3]) || (h2[2] != h2[3]));

      // check padding is correct
      fail_if((x[2] != (x[0] + l)) || (y[2] != (y[0] + t)));

      // line 2 should not suffer from padding in line 1, as it is globally applied
      fail_if((x[4] != x[2]) || ((y[2] + h2[2] + h2[3]) != y[4]));
   }

   /* No font */
   evas_textblock2_style_set(newst, "DEFAULT=''");
   evas_object_textblock2_text_markup_set(tb, "Test");
   evas_object_textblock2_size_formatted_get(tb, &nw, &nh);
   ck_assert_int_eq(nw, 0);
   ck_assert_int_eq(nw, nh);

   END_TB_TEST();
}
END_TEST

/* Various setters and getters */
START_TEST(evas_textblock2_set_get)
{
   START_TB_TEST();
   const char *buf = "";
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(evas_textblock2_style_get(st), style_buf));
   fail_if(evas_object_textblock2_style_get(tb) != st);
   evas_object_textblock2_replace_char_set(tb, "|");
   fail_if(strcmp(evas_object_textblock2_replace_char_get(tb), "|"));
   evas_object_textblock2_replace_char_set(tb, "ש");
   fail_if(strcmp(evas_object_textblock2_replace_char_get(tb), "ש"));

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
   evas_object_textblock2_text_markup_set(tb, "This is" _PS "a test\nbla");
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
   evas_object_textblock2_text_markup_set(tb, buf);
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

/* All the string escaping stuff */
START_TEST(evas_textblock2_escaping)
{
   int len;
   START_TB_TEST();
   fail_if(strcmp(evas_textblock2_escape_string_get("&amp;"), "&"));
   fail_if(strcmp(evas_textblock2_string_escape_get("&", &len), "&amp;"));
   fail_if(len != 1);

   fail_if(strcmp(evas_textblock2_escape_string_get("&middot;"), "\xc2\xb7"));
   fail_if(strcmp(evas_textblock2_string_escape_get("\xc2\xb7", &len),
            "&middot;"));
   fail_if(len != 2);

   fail_if(strcmp(evas_textblock2_escape_string_get("&#x1f459;"),
            "\xF0\x9F\x91\x99"));
   fail_if(strcmp(evas_textblock2_escape_string_get("&#128089;"),
            "\xF0\x9F\x91\x99"));

   fail_if(evas_textblock2_escape_string_get("&middot;aa"));
   const char *tmp = "&middot;aa";
   fail_if(strcmp(evas_textblock2_escape_string_range_get(tmp, tmp + 8),
            "\xc2\xb7"));
   fail_if(evas_textblock2_escape_string_range_get(tmp, tmp + 9));
   fail_if(evas_textblock2_escape_string_range_get(tmp, tmp + 7));
   fail_if(evas_textblock2_escape_string_range_get(tmp, tmp + 5));

   const char *buf = "This &middot; is";
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), "This \xc2\xb7 is"));

   buf = "This &nbsp; is";
   evas_object_textblock2_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock2_text_markup_get(tb), "This \xc2\xa0 is"));

   END_TB_TEST();
}
END_TEST

START_TEST(evas_textblock2_size)
{
   START_TB_TEST();
   Evas_Coord w, h, h2, nw, nh;
   const char *buf = "This is a \n test.\nגם בעברית";

   /* Empty textblock2 */
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);
   fail_if(w != 0);

   /* When wrapping is off, native size should be the same as formatted
    * size */

   evas_object_textblock2_text_markup_set(tb, buf);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   evas_object_textblock2_text_markup_set(tb, "a\na");
   evas_object_textblock2_size_formatted_get(tb, &w, &h2);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h2, nh);

   evas_object_textblock2_text_markup_set(tb, "/u200eאאא AAA");
   evas_object_resize(tb, 1, 1); //force wrapping
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   /* Two lines != double the height */
   fail_if(h * 2 == h2);

   evas_object_textblock2_text_markup_set(tb, buf);

   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h != nh));
   fail_if(w <= 0);

   evas_object_textblock2_text_markup_set(tb, "i<b>。</b>");
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   evas_object_textblock2_text_markup_set(tb, "。<b>i</b>");
   evas_object_textblock2_size_formatted_get(tb, &w, &h);
   evas_object_textblock2_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   /* This time with margins. */
     {
        Evas_Textblock2_Style *newst;
        Evas_Coord oldw, oldh, oldnw, oldnh;

        evas_object_textblock2_text_markup_set(tb, buf);
        evas_object_textblock2_size_formatted_get(tb, &oldw, &oldh);
        evas_object_textblock2_size_native_get(tb, &oldnw, &oldnh);


        newst = evas_textblock2_style_new();
        fail_if(!newst);
        evas_textblock2_style_set(newst,
              "DEFAULT='left_margin=4 right_margin=4'");
        evas_object_textblock2_style_user_push(tb, newst);

        evas_object_textblock2_size_formatted_get(tb, &w, &h);
        evas_object_textblock2_size_native_get(tb, &nw, &nh);

        fail_if((w != oldw + 8) || (h != oldh) ||
              (nw != oldnw + 8) || (nh != oldnh));
     }

   evas_object_resize(tb, 1000, 1000);
   evas_object_textblock2_text_markup_set(tb, "\u200fHello שלום");
   evas_object_textblock2_size_formatted_get(tb, &w, NULL);
   evas_object_textblock2_size_native_get(tb, &nw, NULL);
   ck_assert_int_eq(nw, w);

   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
END_TEST

START_TEST(evas_textblock2_delete)
{
   START_TB_TEST();
   const Evas_Object_Textblock2_Node_Format *fmt;

   /* The first and the second set of commands should result in the same
    * conditions for format and text nodes of the textblock2 object.
    * Essentially, it creates the markup 'a<ps>b' */
   evas_object_textblock2_text_markup_set(tb, "ab");
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if(fmt);
   evas_textblock2_cursor_pos_set(cur, 1);
   evas_object_textblock2_text_markup_prepend(cur, "" _PS "");
   evas_textblock2_cursor_pos_set(cur, 1);
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if (!fmt);
   evas_textblock2_cursor_char_delete(cur);
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if(fmt);

   evas_object_textblock2_text_markup_set(tb, "ab");
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if(fmt);
   evas_textblock2_cursor_pos_set(cur, 1);
   evas_object_textblock2_text_markup_prepend(cur, "<ps>");
   evas_textblock2_cursor_pos_set(cur, 1);
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if (!fmt);
   evas_textblock2_cursor_char_delete(cur);
   fmt =  evas_textblock2_cursor_format_get(cur);
   fail_if(fmt);

   END_TB_TEST();
}
END_TEST;

void evas_test_textblock2(TCase *tc)
{
   tcase_add_test(tc, evas_textblock2_simple);
   tcase_add_test(tc, evas_textblock2_cursor);
#ifdef HAVE_FRIBIDI
   tcase_add_test(tc, evas_textblock2_split_cursor);
#endif
   tcase_add_test(tc, evas_textblock2_size);
   tcase_add_test(tc, evas_textblock2_editing);
   tcase_add_test(tc, evas_textblock2_style);
   tcase_add_test(tc, evas_textblock2_evas);
   tcase_add_test(tc, evas_textblock2_text_getters);
   tcase_add_test(tc, evas_textblock2_formats);
   tcase_add_test(tc, evas_textblock2_format_removal);
   tcase_add_test(tc, evas_textblock2_escaping);
   tcase_add_test(tc, evas_textblock2_set_get);
   tcase_add_test(tc, evas_textblock2_geometries);
   tcase_add_test(tc, evas_textblock2_various);
   tcase_add_test(tc, evas_textblock2_wrapping);
   tcase_add_test(tc, evas_textblock2_items);
   tcase_add_test(tc, evas_textblock2_delete);
}

