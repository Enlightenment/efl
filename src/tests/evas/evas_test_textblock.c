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

/* Functions defined in evas_object_textblock.c */
EAPI Eina_Bool
_evas_textblock_check_item_node_link(Evas_Object *obj);
EAPI int
_evas_textblock_format_offset_get(const Evas_Object_Textblock_Node_Format *n);
/* end of functions defined in evas_object_textblock.c */

#define TEST_FONT "font=DejaVuSans font_source=" TESTS_SRC_DIR "/TestFont.eet"

static const char *style_buf =
   "DEFAULT='" TEST_FONT " font_size=10 color=#000 text_class=entry'"
   "newline='br'"
   "b='+ font_weight=bold'";

#define START_TB_TEST() \
   Evas *evas; \
   Evas_Object *tb; \
   Evas_Textblock_Style *st; \
   Evas_Textblock_Cursor *cur; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   tb = evas_object_textblock_add(evas); \
   fail_if(!tb); \
   evas_object_textblock_legacy_newline_set(tb, EINA_FALSE); \
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
   const char *buf = "Th<i>i</i>s is a <br/> te<b>s</b>t.";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   END_TB_TEST();
}
END_TEST

#define _CHECK_CURSOR_COORDS() \
do \
{ \
        Evas_Coord cx, cy, cw, ch; \
        int ret; \
        ret = evas_textblock_cursor_geometry_get(cur, &cx, &cy, &cw, &ch, \
              NULL, EVAS_TEXTBLOCK_CURSOR_UNDER); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_geometry_get(cur, &cx, &cy, &cw, &ch, \
              NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_char_geometry_get(cur, \
              &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_pen_geometry_get(cur, &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
        ret = evas_textblock_cursor_line_geometry_get(cur, \
              &cx, &cy, &cw, &ch); \
        fail_if(ret == -1); \
} \
while (0)
START_TEST(evas_textblock_cursor)
{
   START_TB_TEST();
   Evas_Coord x, y, w, h;
   size_t i, len;
   Evas_Coord nw, nh;
   const char *buf = "This is a<br/> test.<ps/>Lets see if this works.<ps/>עוד פסקה.";

   /* Walk the textblock using cursor_char_next */
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
   len = eina_unicode_utf8_get_len(buf) - 12; /* 12 because len(<br/>) == 1 and len(<ps/>) == 1 */
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
            "This is a<br/> test."));
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "Lets see if this works."));
   evas_textblock_cursor_paragraph_next(cur);
   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "עוד פסקה."));

   /* Paragraph length get */
   evas_textblock_cursor_paragraph_first(cur);
   /* -4 because len(<br/>) == 1 */
   fail_if(evas_textblock_cursor_paragraph_text_length_get(cur) !=
            eina_unicode_utf8_get_len("This is a<br/> test.") - 4);
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
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "<br/>"));
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

   /* Format positions */
   const Evas_Object_Textblock_Node_Format *fnode;
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   evas_textblock_cursor_copy(cur, main_cur);
   fail_if(evas_textblock_cursor_pos_get(cur) != 9);
   fail_if(evas_textblock_cursor_format_get(cur) != fnode);

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_pos_get(cur) != 16);
   fail_if(evas_textblock_cursor_format_get(cur) != fnode);
   evas_textblock_cursor_format_next(main_cur);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_pos_get(cur) != 9);
   fail_if(evas_textblock_cursor_format_get(cur) != fnode);
   evas_textblock_cursor_format_prev(main_cur);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

   evas_textblock_cursor_char_next(main_cur);
   evas_textblock_cursor_format_prev(main_cur);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));


   evas_object_textblock_text_markup_set(tb, buf);

   /* Check that pen geometry and getting char at coord are in sync. */
   do
     {
        int cur_pos;

        /* Check if it's the last char, if it is, break, otherwise, go back
         * to the current char because our test advanced the cursor. */
        if (!evas_textblock_cursor_char_next(cur))
           break;
        else
           evas_textblock_cursor_char_prev(cur);

        cur_pos = evas_textblock_cursor_pos_get(cur);
        evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
        evas_textblock_cursor_char_coord_set(cur, x + (w / 2), y + (h / 2));
        fail_if(cur_pos != evas_textblock_cursor_pos_get(cur));
     }
   while (evas_textblock_cursor_char_next(cur));

   /* Try positions before the first paragraph, and after the last paragraph */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_textblock_cursor_pos_set(cur, 5);
   evas_textblock_cursor_char_coord_set(cur, nw / 2,
         -50);
   evas_textblock_cursor_paragraph_first(main_cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   evas_textblock_cursor_pos_set(cur, 5);
   evas_textblock_cursor_char_coord_set(cur, nw / 2,
         nh + 50);
   evas_textblock_cursor_paragraph_last(main_cur);
   fail_if(evas_textblock_cursor_compare(cur, main_cur));

   /* Try positions between the first paragraph and the first line. */
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(!evas_textblock_cursor_char_coord_set(cur, 5, 1));

   /* Try positions beyond the left/right limits of lines. */
   for (i = 0 ; i < 2 ; i++)
     {
        evas_textblock_cursor_line_set(cur, i);
        evas_textblock_cursor_line_geometry_get(cur, &x, &y, &w, &h);

        evas_textblock_cursor_pos_set(main_cur, 5);
        evas_textblock_cursor_char_coord_set(main_cur, x - 50, y);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));

        evas_textblock_cursor_line_char_last(cur);
        evas_textblock_cursor_pos_set(main_cur, 5);
        evas_textblock_cursor_char_coord_set(main_cur, x + w + 50, y);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));
     }

#ifdef HAVE_FRIBIDI
   evas_object_textblock_text_markup_set(tb,
         "testנסיוןtestנסיון<ps/>"
         "נסיוןtestנסיוןtest<ps/>"
         "testנסיוןtest<ps/>"
         "נסיוןtestנסיון<ps/>"
         "testנסיון<br/>נסיון<ps/>"
         "נסיוןtest<br/>test"
         );

   for (i = 0 ; i < 8 ; i++)
     {
        evas_textblock_cursor_line_set(cur, i);
        evas_textblock_cursor_line_geometry_get(cur, &x, &y, &w, &h);
        switch (i)
          {
           case 0:
           case 2:
           case 4:
           case 5:
              /* Ltr paragraph */
              evas_textblock_cursor_pos_set(main_cur, 7);
              evas_textblock_cursor_char_coord_set(main_cur, x - 50, y);
              fail_if(evas_textblock_cursor_compare(main_cur, cur));

              evas_textblock_cursor_line_char_last(cur);
              evas_textblock_cursor_pos_set(main_cur, 7);
              evas_textblock_cursor_char_coord_set(main_cur, x + w + 50, y);
              fail_if(evas_textblock_cursor_compare(main_cur, cur));
              break;
           case 1:
           case 3:
           case 6:
           case 7:
              /* Rtl paragraph */
              evas_textblock_cursor_line_char_last(cur);
              evas_textblock_cursor_pos_set(main_cur, 7);
              evas_textblock_cursor_char_coord_set(main_cur, x - 50, y);
              fail_if(evas_textblock_cursor_compare(main_cur, cur));

              evas_textblock_cursor_line_char_first(cur);
              evas_textblock_cursor_pos_set(main_cur, 7);
              evas_textblock_cursor_char_coord_set(main_cur, x + w + 50, y);
              fail_if(evas_textblock_cursor_compare(main_cur, cur));
              break;
          }
     }
#endif

   evas_object_textblock_text_markup_set(tb, buf);
   /* Testing line geometry.*/
     {
        Evas_Coord lx, ly, lw, lh;
        Evas_Coord plx, ply, plw, plh;
        evas_textblock_cursor_line_set(cur, 0);
        evas_textblock_cursor_copy(cur, main_cur);
        evas_textblock_cursor_line_char_last(main_cur);
        evas_textblock_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);

        while (evas_textblock_cursor_compare(cur, main_cur) <= 0)
          {
             evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
             fail_if(0 != evas_textblock_cursor_line_geometry_get(
                      cur, &lx, &ly, &lw, &lh));
             fail_if((x < lx) || (x + w > lx + lw) ||
                   (y < ly) || (y + h > ly + lh));
             fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));

             plx = lx;
             ply = ly;
             plw = lw;
             plh = lh;
             evas_textblock_cursor_char_next(cur);
          }

        evas_textblock_cursor_line_set(cur, 1);
        evas_textblock_cursor_copy(cur, main_cur);
        evas_textblock_cursor_line_char_last(main_cur);
        evas_textblock_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);

        while (evas_textblock_cursor_compare(cur, main_cur) <= 0)
          {
             evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
             fail_if(1 != evas_textblock_cursor_line_geometry_get(
                      cur, &lx, &ly, &lw, &lh));
             fail_if((x < lx) || (x + w > lx + lw) ||
                   (y < ly) || (y + h > ly + lh));
             fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));

             plx = lx;
             ply = ly;
             plw = lw;
             plh = lh;
             evas_textblock_cursor_char_next(cur);
          }

        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_line_set(cur, 0);
        evas_textblock_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);
        evas_object_textblock_line_number_geometry_get(tb, 0,
              &lx, &ly, &lw, &lh);
        fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));
        fail_if(0 != evas_textblock_cursor_line_coord_set(cur, ly + (lh / 2)));

        evas_textblock_cursor_line_set(cur, 1);
        evas_textblock_cursor_line_geometry_get(cur, &plx, &ply, &plw, &plh);
        evas_object_textblock_line_number_geometry_get(tb, 1,
              &lx, &ly, &lw, &lh);
        fail_if((lx != plx) || (ly != ply) || (lw != plw) || (lh != plh));
        fail_if(1 != evas_textblock_cursor_line_coord_set(cur, ly + (lh / 2)));

        /* Before the start of the textblock */
        fail_if(0 != evas_textblock_cursor_line_coord_set(cur, -50));
        fail_if(3 != evas_textblock_cursor_line_coord_set(cur, 100000));

        /* And now with a valigned textblock. */
        evas_object_textblock_text_markup_set(tb, buf);
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, 2 * nw, 2 * nh);

        evas_object_textblock_valign_set(tb, 0.5);
        evas_textblock_cursor_paragraph_first(cur);
        evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
        fail_if(y <= 0);

        evas_textblock_cursor_paragraph_last(main_cur);
        evas_textblock_cursor_char_coord_set(main_cur, x + w, y / 2);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));

        evas_textblock_cursor_paragraph_last(main_cur);
        evas_textblock_cursor_line_coord_set(main_cur, y / 2);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));

        /* Fail if they are equal, i.e if it for some reason thinks it should
         * go to the end. */
        evas_textblock_cursor_paragraph_first(main_cur);
        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_char_coord_set(main_cur, x + w, nh + 1);
        fail_if(!evas_textblock_cursor_compare(main_cur, cur));

        evas_textblock_cursor_paragraph_first(main_cur);
        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_line_coord_set(main_cur, nh + 1);
        fail_if(!evas_textblock_cursor_compare(main_cur, cur));

        /* Fail if it doesn't go to the end. */
        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_paragraph_first(main_cur);
        evas_textblock_cursor_char_coord_set(main_cur, x + w, (2 * nh) - 1);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));

        evas_textblock_cursor_paragraph_first(main_cur);
        evas_textblock_cursor_line_coord_set(main_cur, (2 * nh) - 1);
        fail_if(evas_textblock_cursor_compare(main_cur, cur));
     }

     {
        const char *buf_wb = "a This is_a t:e.s't a";
        evas_object_textblock_text_markup_set(tb, buf_wb);

        /* Word start/end */
        evas_textblock_cursor_pos_set(cur, 3);
        evas_textblock_cursor_word_start(cur);
        fail_if(2 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_end(cur);
        fail_if(5 != evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_pos_set(cur, 13);
        evas_textblock_cursor_word_end(cur);
        fail_if(18 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_start(cur);
        fail_if(12 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_start(cur);
        fail_if(12 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_start(cur);
        fail_if(12 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_end(cur);
        fail_if(18 != evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_end(cur);
        fail_if(21 != evas_textblock_cursor_pos_get(cur));

        /* Bug with 1 char word separators at paragraph start. */
        evas_object_textblock_text_markup_set(tb, "=test");
        evas_textblock_cursor_pos_set(cur, 4);
        evas_textblock_cursor_word_start(cur);
        fail_if(1 != evas_textblock_cursor_pos_get(cur));
     }

   /* Make sure coords are correct for ligatures */
     {
        evas_object_textblock_text_markup_set(tb, "fi<br/>fii");

#ifdef HAVE_HARFBUZZ
        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
             evas_textblock_cursor_char_next(cur);
          }

        evas_textblock_cursor_char_next(cur);

        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
             evas_textblock_cursor_char_next(cur);
          }
        evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 3);
#else
/* FIXME: Disabled because fails on jenkins */
#if 0
        evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 4);
        evas_textblock_cursor_char_next(cur);
        evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 3);

        evas_textblock_cursor_pos_set(cur, 3);
        evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
        ck_assert_int_eq(w, 4);

        for (i = 0 ; i < 2 ; i++)
          {
             evas_textblock_cursor_char_next(cur);
             evas_textblock_cursor_pen_geometry_get(cur, NULL, NULL, &w, NULL);
             ck_assert_int_eq(w, 3);
          }
#endif
#endif
     }

   END_TB_TEST();
}
END_TEST

#ifdef HAVE_FRIBIDI
START_TEST(evas_textblock_split_cursor)
{
   START_TB_TEST();
   Evas_Coord x, w, x2, w2;
   Evas_Coord nw, nh;
   Evas_Coord cx, cy, cx2, cy2;

   /* Split cursor in LTR paragraph.
    * Russian 't' in the beginnning to create additional item.*/
                                            /*01234    5 6789012345678  19  01234 */
   evas_object_textblock_text_markup_set(tb, "тest \u202bנסיוןabcנסיון\u202c bang");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   /* Logical cursor after "test " */
   evas_textblock_cursor_pos_set(cur, 6);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 18);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 20);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before "a" */
   evas_textblock_cursor_pos_set(cur, 11);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 10);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after "c" */
   evas_textblock_cursor_pos_set(cur, 14);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 10);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before " bang" */
   evas_textblock_cursor_pos_set(cur, 20);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 19);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor in the beginning */
   evas_textblock_cursor_line_char_first(cur);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Logical cursor in the end */
   evas_textblock_cursor_line_char_last(cur);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Logical cursor on the second pos */
   evas_textblock_cursor_pos_set(cur, 2);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Split cursor in RTL paragraph. */
                                           /*                1           2
                                              01234  5   67890123456789  0   123456 */
   evas_object_textblock_text_markup_set(tb, "שלום \u202atest עברית efl\u202c נסיון");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   /* Logical cursor before "test" */
   evas_textblock_cursor_pos_set(cur, 6);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 4);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after "test " */
   evas_textblock_cursor_pos_set(cur, 11);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 16);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 15);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before " efl" */
   evas_textblock_cursor_pos_set(cur, 16);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 15);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor after " efl" */
   evas_textblock_cursor_pos_set(cur, 21);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 4);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor in the beginning */
   evas_textblock_cursor_line_char_first(cur);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));

   /* Logical cursor in the end */
   evas_textblock_cursor_line_char_last(cur);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 26);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Corner cases for split cursor. */

   /* End of line in LTR paragraph with embedding*/
                                             /*              1
                                              01234   5  678901234567   */
   evas_object_textblock_text_markup_set(tb, "test \u202bנסיוןشسيبabc");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock_cursor_line_char_last(cur);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 15);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));
   ck_assert_int_eq(cx2, x2);

   /* End of line in RTL paragraph */
                                          /*                 1         2
                                              012345678   9  01234567890123 */
   evas_object_textblock_text_markup_set(tb, "נסיוןشسي \u202atestприветשלום");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock_cursor_line_char_last(cur);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, &cx2, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 8);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 10);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Cursor is between items of the same direction */
   evas_textblock_cursor_pos_set(cur, 14);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Cursor type is UNDER */
   evas_textblock_cursor_pos_set(cur, 0);
   fail_if(evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
                                                    NULL, NULL, NULL, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_UNDER));
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);

   /* Multiline */
   Evas_Coord ly;
   int i;
                                                      /* 012345678901234 */
   evas_object_textblock_text_markup_set(tb, "<wrap=char>testשלוםشسيبefl");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   nh = nh * 15;
   evas_object_resize(tb, nw, nh);

   for (i = 0; i < nw; i++)
     {
        evas_object_resize(tb, i, nh);

        evas_textblock_cursor_pos_set(cur, 12);
        fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                         NULL, &cx2, &cy2, NULL, NULL,
                                                         EVAS_TEXTBLOCK_CURSOR_BEFORE));
        evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy, ly);
        evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
        ck_assert_int_eq(cx, x);
        evas_textblock_cursor_pos_set(cur, 11);
        evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy2, ly);
        evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
        ck_assert_int_eq(cx2, x2);
     }
                                                      /* 01234567890123456789 */
   evas_object_textblock_text_markup_set(tb, "<wrap=char>נסיוןhelloприветשלום");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   nh = nh * 20;
   evas_object_resize(tb, nw, nh);

   for (i = 0; i < nw; i++)
     {
        evas_object_resize(tb, i, nh);
        evas_textblock_cursor_pos_set(cur, 16);
        fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                         NULL, &cx2, &cy2, NULL, NULL,
                                                         EVAS_TEXTBLOCK_CURSOR_BEFORE));
        evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy, ly);
        evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
        ck_assert_int_eq(cx, (x + w));
        evas_textblock_cursor_pos_set(cur, 15);
        evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
        ck_assert_int_eq(cy2, ly);
        evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, &w2, NULL);
        ck_assert_int_eq(cx2, (x2 + w2));
     }

   /* Testing multiline, when only RTL item is in the line. */
                                                      /* 012345678901234567890123 */
   evas_object_textblock_text_markup_set(tb, "<wrap=char>testtesttestтестשלוםشسيب");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock_cursor_pos_set(cur, 15);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   /* Resizing textblock, so RTL item will be on the next line.*/
   evas_object_resize(tb, x + w, nh);

   evas_textblock_cursor_pos_set(cur, 24);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                    NULL, &cx2, &cy2, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 16);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   ck_assert_int_eq(cx, (x + w));
   evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy, ly);

   evas_textblock_cursor_pos_set(cur, 23);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx2, x2);
   evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy2, ly);

   /* Testing multiline, when only LTR item is in the line. */
                                                      /* 012345678901234567890123 */
   evas_object_textblock_text_markup_set(tb, "<wrap=char>שלוםשלוםשלוםشسيبtestтест");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);

   evas_textblock_cursor_pos_set(cur, 15);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, &w, NULL);
   /* Resizing textblock, so LTR item will be on the next line.*/
   evas_object_resize(tb, nw - x, nh);

   evas_textblock_cursor_pos_set(cur, 24);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, &cy, NULL,
                                                    NULL, &cx2, &cy2, NULL, NULL,
                                                    EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 16);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy, ly);

   evas_textblock_cursor_line_char_last(cur);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx2, x2);
   evas_textblock_cursor_line_geometry_get(cur, NULL, &ly, NULL, NULL);
   ck_assert_int_eq(cy2, ly);

   END_TB_TEST();
}
END_TEST
#endif

START_TEST(evas_textblock_format_removal)
{
   START_TB_TEST();
   int i;
   const char *buf = "Th<b>is a<a>tes</a>st</b>.";
   const Evas_Object_Textblock_Node_Format *fnode;
   Evas_Textblock_Cursor *main_cur = evas_object_textblock_cursor_get(tb);
   evas_object_textblock_text_markup_set(tb, buf);

   /* Remove the "b" pair. */
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ a"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- a"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Now also remove the a pair */
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Remove the "a" pair. */
   evas_object_textblock_text_markup_set(tb, buf);

   fnode = evas_textblock_node_format_first_get(tb);
   fnode = evas_textblock_node_format_next_get(fnode);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Now also remove the b pair */
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Now remove formats by removing text */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   /* Only b formats should remain */
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* No formats should remain. */
   evas_textblock_cursor_pos_set(cur, 2);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Try to remove the formats in a way that shouldn't remove them */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_pos_set(cur, 7);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   evas_textblock_cursor_char_delete(cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ a"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- a"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Try range deletion to delete a */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_pos_set(main_cur, 9);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Range deletion to delete both */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_pos_set(cur, 2);
   evas_textblock_cursor_pos_set(main_cur, 11);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Range deletion across paragraphs */
   evas_object_textblock_text_markup_set(tb,
         "Th<b>is a<a>te<ps/>"
         "s</a>st</b>.");
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_pos_set(main_cur, 10);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Range deletion across paragraph - a bug found in elm. */
   evas_object_textblock_text_markup_set(tb,
         "This is an entry widget in this window that<ps/>"
         "uses markup <b>like this</> for styling and<ps/>"
         "formatting <em>like this</>, as well as<ps/>"
         "<a href=X><link>links in the text</></a>, so enter text<ps/>"
         "in here to edit it. By the way, links are<ps/>"
         "called <a href=anc-02>Anchors</a> so you will need<ps/>"
         "to refer to them this way.<ps/>"
         "<ps/>"

         "Also you can stick in items with (relsize + ascent): "
         "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
         " (full) "
         "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
         " (to the left)<ps/>"

         "Also (size + ascent): "
         "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
         " (full) "
         "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
         " (before this)<ps/>"

         "And as well (absize + ascent): "
         "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
         " (full) "
         "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
         " or even paths to image files on disk too like: "
         "<item absize=96x128 vsize=full href=file://bla/images/sky_01.jpg></item>"
         " ... end.");
   evas_textblock_cursor_paragraph_first(cur);
   evas_textblock_cursor_paragraph_last(main_cur);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(fnode);

   /* The first one below used to crash: empty value.
    * Test some invalid stuff doesn't segv. We force relayout as this will
    * trigger the parser. */
   evas_object_textblock_text_markup_set(tb, "A<a=>");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock_text_markup_set(tb, "A<=b>");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock_text_markup_set(tb, "A<=>");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock_text_markup_set(tb, "A<a='>");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);
   evas_object_textblock_text_markup_set(tb, "A<a='");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);

   /* Deleting a range with just one char and surrounded by formats, that
    * deletes a paragraph. */
   evas_object_textblock_text_markup_set(tb, "A<ps/><b>B</b>");
   evas_textblock_cursor_pos_set(cur, 2);
   evas_textblock_cursor_pos_set(main_cur, 3);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Two formats in the same place. */
   evas_object_textblock_text_markup_set(tb, "a<b><a>b</a></b>b");
   evas_textblock_cursor_pos_set(cur, 1);
   evas_textblock_cursor_char_delete(cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Two formats across different paragraphs with notihng in between. */
   evas_object_textblock_text_markup_set(tb, "<b><ps/></b>");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_char_delete(cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Try with range */
   evas_object_textblock_text_markup_set(tb, "<b><ps/></b>");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 1);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Range delete with empty paragraphs. */
   evas_object_textblock_text_markup_set(tb, "<ps/><ps/><ps/><ps/><ps/>");
   evas_textblock_cursor_pos_set(cur, 2);
   evas_textblock_cursor_pos_set(main_cur, 3);
   evas_textblock_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "<ps/><ps/><ps/><ps/>");

   /* Range delete with item formats, TEST_CASE#1 */
   evas_object_textblock_text_markup_set(tb, "The <b>Multiline</b><item size=50x50 href=abc></item> text!");
   evas_textblock_cursor_pos_set(cur, 4);
   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "The  text!");

   /* Range delete with item formats, TEST_CASE#2 */
   evas_object_textblock_text_markup_set(tb, "The <b>Multiline</b><item size=50x50 href=abc></item> text! it is lon<item size=40x40 href=move></item>g text for test.");
   evas_textblock_cursor_pos_set(cur, 14);
   evas_textblock_cursor_pos_set(main_cur, 15);
   evas_textblock_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "The <b>Multiline</b><item size=50x50 href=abc></item>text! it is lon<item size=40x40 href=move></item>g text for test.");

   /* Verify fmt position and REP_CHAR positions are the same */
   evas_object_textblock_text_markup_set(tb,
         "This is<ps/>an <item absize=93x152 vsize=ascent></>a.");
   evas_textblock_cursor_pos_set(cur, 7);
   evas_textblock_cursor_char_delete(cur);
   fnode = evas_textblock_node_format_first_get(tb);
   /* FIXME:  to fix in Evas.h */
/*    fail_if(_evas_textblock_format_offset_get(fnode) != 10); */

   /* Out of order <b><i></b></i> mixes. */
   evas_object_textblock_text_markup_set(tb, "a<b>b<i>c</b>d</i>e");
   evas_textblock_cursor_pos_set(cur, 2);

   for (i = 0 ; i < 2 ; i++)
     {
        fnode = evas_textblock_node_format_first_get(tb);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ i"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- b"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- i"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (fnode);

        evas_textblock_cursor_char_delete(cur);
     }
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* This time with a generic closer */
   evas_object_textblock_text_markup_set(tb, "a<b>b<i>c</b>d</>e");
   evas_textblock_cursor_pos_set(cur, 2);

   for (i = 0 ; i < 2 ; i++)
     {
        fnode = evas_textblock_node_format_first_get(tb);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ i"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- b"));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (!fnode);
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

        fnode = evas_textblock_node_format_next_get(fnode);
        fail_if (fnode);

        evas_textblock_cursor_char_delete(cur);
     }
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* And now with remove pair. */
   evas_object_textblock_text_markup_set(tb, "a<b>b<i>c</b>d</i>e");
   evas_textblock_cursor_pos_set(cur, 2);
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ i"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Remove the other pair */
   evas_object_textblock_text_markup_set(tb, "a<b>b<i>c</>d</i>e");
   evas_textblock_cursor_pos_set(cur, 2);
   fnode = evas_textblock_node_format_first_get(tb);
   fnode = evas_textblock_node_format_next_get(fnode);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   /* Remove two pairs with the same name and same positions. */
   evas_object_textblock_text_markup_set(tb, "<a><a>A</a></a>");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_char_delete(cur);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (fnode);

   /* Try to remove a format that doesn't have a pair (with a bad mkup) */
   evas_object_textblock_text_markup_set(tb, "a<b>b<i>c</>d</i>e");
   evas_textblock_cursor_pos_set(cur, 2);
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb,
         (Evas_Object_Textblock_Node_Format *) fnode);

   fnode = evas_textblock_node_format_first_get(tb);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ i"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- i"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if (fnode);

   END_TB_TEST();
}
END_TEST

/* Testing items */
START_TEST(evas_textblock_items)
{
   Evas_Coord w, h, w2, h2, nw, nh, ih;
   START_TB_TEST();
   const char *buf = "This is an <item absize=93x152></>.";

   /* Absolute item size */
   buf = "This is an <item absize=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 93);
   ck_assert_int_eq(h, 152);

   buf = "This is an <item absize=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   ck_assert_int_eq(w, 93);
   ck_assert_int_eq(h, 152);

   /* Size is the same as abssize, unless there's scaling applied. */
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 93);
   _ck_assert_int(h, >=, 153);
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h <= 153));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   evas_object_scale_set(tb, 2.0);
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h != (2 * 154)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h <= (2 * 154)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   evas_object_scale_set(tb, 1.0);

   /* Relsize */
   /* relsize means it should adjust itself to the size of the line */
   buf = "This is an <item relsize=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 153));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 108) || (h <= ih));

   buf = "This is an <item relize=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 108) || (h <= ih));

   /* Relsize and abs size in the same line, all should be the same size */
   buf = "<item relsize=64x64 vsize=ascent href=emoticon/knowing-grin></item><item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item><item relsize=64x64 vsize=ascent href=emoticon/knowing-grin></item>";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((nw != w) || (nh != h));
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w2, &h2);
   fail_if((w != w2) || (h != h2));
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w2, &h2);
   fail_if((w != w2) || (h != h2));

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
START_TEST(evas_textblock_wrapping)
{
   Evas_Coord bw, bh, w, h, nw, nh;
   int i;
   START_TB_TEST();
   evas_object_textblock_text_markup_set(tb, "a");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);

   /* Char wrap */
   evas_object_textblock_text_markup_set(tb, "aaaaaaa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Mixed - fallback to char wrap */
   evas_object_textblock_text_markup_set(tb, "aaaaaaa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Basic Word wrap */
   evas_object_textblock_text_markup_set(tb, "aaaa");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);

   evas_object_textblock_text_markup_set(tb, "aaaa aa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Mixed - fallback to word wrap */
   evas_object_textblock_text_markup_set(tb, "aaaa aa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, bw + 1, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);

   /* Wrap and then expand again. */
   evas_object_textblock_text_markup_set(tb, "aaaa aa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w != nw) || (h != nh));

   /* Reduce size until reaching the minimum, making sure we don't
    * get something wrong along the way */
   /* Char wrap */
   evas_object_textblock_text_markup_set(tb, "a");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa<ps/>"
         "aaaa aaa aaa aaa aaa<ps/>"
         "a aaaaa aaaaaaaaaaaaaa<br/>aaaaa<ps/>"
         "aaaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   Evas_Coord iw;
   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Word wrap */
   evas_object_textblock_text_markup_set(tb, "aaaaaa");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa<ps/>"
         "aaaa aaa aaa aaa aaa<ps/>"
         "a aaaaa aaaaaa<br/>aaaaa<ps/>"
         "aaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Mixed wrap */
   evas_object_textblock_text_markup_set(tb, "a");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa<ps/>"
         "aaaa aaa aaa aaa aaa<ps/>"
         "a aaaaa aaaaaa<br/>aaaaa<ps/>"
         "aaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if(w < bw);
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Resize, making sure we keep going down in the minimum size. */
   char *wrap_style[] = { "+ wrap=word", "+ wrap=char", "+ wrap=mixed" };
   int wrap_items = sizeof(wrap_style) / sizeof(*wrap_style);

   evas_object_textblock_text_markup_set(tb,
         "This is an entry widget in this window that<br/>"
         "uses markup <b>like this</> for styling and<br/>"
         "formatting <em>like this</>, as well as<br/>"
         "<a href=X><link>links in the text</></a>, so enter text<br/>"
         "in here to edit it. By the way, links are<br/>"
         "called <a href=anc-02>Anchors</a> so you will need<br/>"
         "to refer to them this way.<br/>"
         "<br/>"

         "Also you can stick in items with (relsize + ascent): "
         "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
         " (full) "
         "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
         " (to the left)<br/>"

         "Also (size + ascent): "
         "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
         " (full) "
         "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
         " (before this)<br/>"

         "And as well (absize + ascent): "
         "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
         " (full) "
         "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
         " or even paths to image files on disk too like: "
         "<item absize=96x128 vsize=full href=file://%s/images/sky_01.jpg></item>"
         " ... end."
         );

   /* Get minimum size */
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   for (i = 0 ; i < wrap_items ; i++)
     {
        evas_textblock_cursor_format_prepend(cur, wrap_style[i]);
        evas_object_resize(tb, 0, 0);
        evas_object_textblock_size_formatted_get(tb, &bw, &bh);

        for (iw = nw ; iw >= bw ; iw--)
          {
             evas_object_resize(tb, iw, 1000);
             evas_object_textblock_size_formatted_get(tb, &w, &h);
             fail_if(w < bw);
             fail_if(w > iw);
          }
        fail_if(w != bw);
     }


   /* Ellipsis */
   evas_object_textblock_text_markup_set(tb, "aaaaaaaaaa");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w > (nw / 2)) || (h != nh));

   evas_object_textblock_text_markup_set(tb, "aaaaaaaaaaaaaaaaaa<br/>b");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0 wrap=word");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if(w > (nw / 2));

   /* Word wrap ending with whites. */
   evas_object_resize(tb, 322, 400);
   evas_object_textblock_text_markup_set(tb, "<wrap=word>This is an example text that should break at the end aaa     ");

   evas_textblock_cursor_paragraph_last(cur);
   Evas_Coord cx, cy, cw, ch;
   evas_textblock_cursor_text_prepend(cur, " ");
   fail_if(-1 == evas_textblock_cursor_geometry_get(cur, &cx, &cy, &cw, &ch,
            NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE));

   /* Getting whites back after wrapping. */
   evas_object_resize(tb, 1, 1);
   evas_object_textblock_text_markup_set(tb, "<wrap=word><keyword>return</keyword> <number>0</number>;</wrap>");

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, 33);
   ck_assert_int_eq(h, 25);

   evas_object_resize(tb, 400, 400);

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, 45);
   ck_assert_int_eq(h, 16);

   /* Complex compound clusters using Devanagari. */
   evas_object_resize(tb, 0, 0);

   evas_object_textblock_text_markup_set(tb, "<wrap=char> करेंकरेंकरेंकरेंकरेंकरें");
   evas_object_textblock_size_formatted_get(tb, &w, &h);

   fail_if(w > h); /* FIXME: Not the best test, should be more strict. */

   END_TB_TEST();
}
END_TEST

/* Various textblock stuff */
START_TEST(evas_textblock_various)
{
   Evas_Coord w, h, bw, bh;
   START_TB_TEST();
   const char *buf = "This<ps/>textblock<ps/>has<ps/>a<ps/>lot<ps/>of<ps/>lines<ps/>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Move outside of the screen so it'll have to search for the correct
    * paragraph and etc. */
   evas_object_move(tb, -(w / 2), -(h / 2));

   /* Replacement char */
   evas_object_textblock_text_markup_set(tb, "*");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_replace_char_set(tb, "*");
   evas_object_textblock_text_markup_set(tb, "|");
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w != bw) || (h != bh));

   /* Items have correct text node information */
   /* FIXME:  to fix in Evas.h */
   evas_object_textblock_text_markup_set(tb, "");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */
   evas_object_textblock_text_markup_set(tb, "<ps/>");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */
   evas_object_textblock_text_markup_set(tb, "a<ps/>");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */
   evas_object_textblock_text_markup_set(tb, "a<ps/>a");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */
   evas_object_textblock_text_markup_set(tb, "a<ps/>a<ps/>");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */
   evas_object_textblock_text_markup_set(tb, "a<ps/>a<ps/>a");
/*    fail_if(!_evas_textblock_check_item_node_link(tb)); */

   /* These shouldn't crash (although the desired outcome is not yet defined) */
   evas_object_textblock_text_markup_set(tb, "&#xfffc;");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_char_delete(cur);

   evas_object_textblock_text_markup_set(tb, "\xEF\xBF\xBC");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_char_delete(cur);

   /* Super big one line item. */
     {
#define CNT 10000
        char str[(CNT * 6) + 128], *d;
        const char substr[] = "x";
        Evas_Textblock_Style *stt;
        int i, l;

        l = strlen(substr);
        d = str;
        for (i = 0; i < CNT; i++)
          {
             memcpy(d, substr, l);
             d += l;
          }
        *d = 0;

        stt = evas_textblock_style_new();
        evas_textblock_style_set(stt,
              "DEFAULT='" TEST_FONT " font_size=10 align=left color=#000000 wrap=char'");
        evas_object_textblock_style_set(tb, stt);
        evas_textblock_style_free(stt);

        evas_object_textblock_text_markup_set(tb, substr);
        Evas_Textblock_Cursor *cr;

        cr = evas_object_textblock_cursor_get(tb);
        evas_textblock_cursor_text_append(cr, str);
        evas_object_resize(tb, 480, 800);

        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if(w == 0);
     }

   END_TB_TEST();
}
END_TEST

/* Various geometries. e.g. range geometry. */
START_TEST(evas_textblock_geometries)
{
   START_TB_TEST();
   const char *buf = "This is a <br/> test.";
   evas_object_textblock_text_markup_set(tb, buf);

   /* Single line range */
   Evas_Textblock_Cursor *main_cur = evas_object_textblock_cursor_get(tb);
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 6);

   Eina_List *rects, *rects2;
   Evas_Textblock_Rectangle *tr, *tr2;
   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);
   fail_if(!rects);
   rects2 = evas_textblock_cursor_range_geometry_get(main_cur, cur);
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
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 14);

   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);
   fail_if(!rects);
   rects2 = evas_textblock_cursor_range_geometry_get(main_cur, cur);
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
   evas_object_textblock_text_markup_set(tb, "עבריתenglishрусскийעברית");

   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 7);
   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);

   fail_if(eina_list_count(rects) != 2);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   /* Same run different styles */
   evas_object_textblock_text_markup_set(tb, "test<b>test2</b>test3");

   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 11);
   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);

   fail_if(eina_list_count(rects) != 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   /* Bidi text with a few back and forth from bidi. */
   evas_object_textblock_text_markup_set(tb, "נגכדגךלח eountoheunth ךלחגדךכלח");

   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 28);
   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);

   ck_assert_int_eq(eina_list_count(rects), 3);

   EINA_LIST_FREE(rects, tr)
      free(tr);

   END_TB_TEST();
}
END_TEST

/* Should handle all the text editing. */
START_TEST(evas_textblock_editing)
{
   START_TB_TEST();
   const char *buf = "First par.<ps/>Second par.";
   evas_object_textblock_text_markup_set(tb, buf);
   Evas_Textblock_Cursor *main_cur = evas_object_textblock_cursor_get(tb);

   /* Check deletion works */
   /* Try deleting after the end of the textblock */
     {
        char *content;
        evas_textblock_cursor_paragraph_last(cur);
        content = strdup(evas_object_textblock_text_markup_get(tb));
        evas_textblock_cursor_char_delete(cur);
        fail_if(strcmp(content, evas_object_textblock_text_markup_get(tb)));
        free(content);
     }

   /* Delete the first char */
   evas_textblock_cursor_paragraph_first(cur);
   evas_textblock_cursor_char_delete(cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "irst par.<ps/>Second par."));

   /* Delete some arbitrary char */
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_delete(cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "irs par.<ps/>Second par."));

   /* Delete a range */
   evas_textblock_cursor_pos_set(main_cur, 1);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "ir.<ps/>Second par."));
   evas_textblock_cursor_paragraph_char_first(main_cur);
   evas_textblock_cursor_paragraph_char_last(cur);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "Second par."));

   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_paragraph_last(main_cur);
   evas_object_textblock_text_markup_prepend(main_cur, "Test<b>bla</b>bla.");
   evas_textblock_cursor_paragraph_last(cur);
   evas_textblock_cursor_paragraph_char_first(main_cur);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "First par.<ps/>"));

   /* Merging paragraphs */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_paragraph_char_last(cur);
   evas_textblock_cursor_copy(cur, main_cur);
   evas_textblock_cursor_char_delete(cur);

   evas_textblock_cursor_paragraph_first(cur);
   fail_if(evas_textblock_cursor_paragraph_next(cur));

   /* Split paragraphs */
   evas_textblock_cursor_format_prepend(cur, "ps");

   evas_textblock_cursor_paragraph_first(cur);
   fail_if(!evas_textblock_cursor_paragraph_next(cur));
   fail_if(evas_textblock_cursor_paragraph_next(cur));

   /* Merge paragraphs using range deletion */
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_paragraph_first(cur);
   evas_textblock_cursor_paragraph_char_last(cur);
   evas_textblock_cursor_copy(cur, main_cur);
   evas_textblock_cursor_char_prev(cur);
   evas_textblock_cursor_char_next(main_cur);

   evas_textblock_cursor_range_delete(cur, main_cur);
   evas_textblock_cursor_paragraph_first(cur);
   fail_if(evas_textblock_cursor_paragraph_next(cur));

     {
        /* Limit to 1000 iterations so we'll never get into an infinite loop,
         * even if broken */
        int limit = 1000;
        evas_object_textblock_text_markup_set(tb, "this is a test eauoeuaou<ps/>this is a test1<ps/>this is a test 3");
        evas_textblock_cursor_paragraph_last(cur);
        while (evas_textblock_cursor_pos_get(cur) > 0)
          {
             limit--;
             fail_if(limit <= 0);
             evas_textblock_cursor_copy(cur, main_cur);
             evas_textblock_cursor_char_prev(cur);
             evas_textblock_cursor_word_start(cur);
             evas_textblock_cursor_range_delete(cur, main_cur);
          }
     }


   /* Insert illegal characters inside the format. */
     {
        const char *content;
        evas_object_textblock_text_markup_set(tb, "a\n");
        evas_textblock_cursor_pos_set(cur, 1);
        content = evas_textblock_cursor_content_get(cur);

        evas_object_textblock_text_markup_set(tb, "a\t");
        evas_textblock_cursor_pos_set(cur, 1);
        content = evas_textblock_cursor_content_get(cur);

        evas_object_textblock_text_markup_set(tb, "a\xEF\xBF\xBC");
        evas_textblock_cursor_pos_set(cur, 1);
        content = evas_textblock_cursor_content_get(cur);

        evas_object_textblock_text_markup_set(tb, "a\xE2\x80\xA9");
        evas_textblock_cursor_pos_set(cur, 1);
        content = evas_textblock_cursor_content_get(cur);
        (void) content;
     }

   /* FIXME: Also add text appending/prepending */

   END_TB_TEST();
}
END_TEST

/* Text getters */
START_TEST(evas_textblock_text_getters)
{
   START_TB_TEST();
   const char *buf = "This is a <br/> test.<ps/>"
      "טקסט בעברית<ps/>and now in english.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_paragraph_first(cur);

   fail_if(strcmp(evas_textblock_cursor_paragraph_text_get(cur),
            "This is a <br/> test."));

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
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is a <br/> te"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps/>טק"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps/>טקסט בעברית<ps/>an"));

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
            EVAS_TEXTBLOCK_TEXT_MARKUP), "is a <br/> te"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 20);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps/>טק"));

   evas_textblock_cursor_pos_set(main_cur, 14);
   evas_textblock_cursor_pos_set(cur, 32);
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "st.<ps/>טקסט בעברית<ps/>an"));

   /* Uninit cursors and other weird cases */
   evas_object_textblock_clear(tb);
   evas_textblock_cursor_copy(main_cur, cur);
   evas_textblock_cursor_text_prepend(main_cur, "aaa");
   fail_if(strcmp(evas_textblock_cursor_range_text_get(cur, main_cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP), "aaa"));

   /* Markup to plain and vice versa */
     {
        char *tmp, *tmp2;

        /* Real textblock object */
        tmp = evas_textblock_text_markup_to_utf8(tb, "<br/>aa<\n/>bb<\t/>");
        fail_if(strcmp(tmp, "\naa\nbb\t"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "<br/>aa<br/>bb<tab/>"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a<item></item>");
        fail_if(strcmp(tmp, "a\xEF\xBF\xBC"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&#xfffc;"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a&nbsp;");
        fail_if(strcmp(tmp, "a\xC2\xA0"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a\xC2\xA0"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a<b>b</b><more></>a");
        fail_if(strcmp(tmp, "aba"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "aba"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a&amp;a");
        fail_if(strcmp(tmp, "a&a"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&amp;a"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a<newline/>a");
        fail_if(strcmp(tmp, "a\na"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a<br/>a"));
        free(tmp2);
        free(tmp);

        /* NULL textblock object */
        tmp = evas_textblock_text_markup_to_utf8(NULL, "<br/>aa<\n/>bb<\t/>");
        fail_if(strcmp(tmp, "\naa\nbb\t"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "<br/>aa<br/>bb<tab/>"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(NULL, "a<item></item>");
        fail_if(strcmp(tmp, "a\xEF\xBF\xBC"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "a&#xfffc;"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(NULL, "a&nbsp;");
        fail_if(strcmp(tmp, "a\xC2\xA0"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "a\xC2\xA0"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(NULL, "a<b>b</b><more></>a");
        fail_if(strcmp(tmp, "aba"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "aba"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(tb, "a&amp;a");
        fail_if(strcmp(tmp, "a&a"));
        tmp2 = evas_textblock_text_utf8_to_markup(tb, tmp);
        fail_if(strcmp(tmp2, "a&amp;a"));
        free(tmp2);
        free(tmp);

        tmp = evas_textblock_text_markup_to_utf8(NULL, "a<newline/>a");
        fail_if(strcmp(tmp, "aa"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "aa"));
        free(tmp2);
        free(tmp);
     }

   /* complex markup set/get */
     {
        const char *text =
           "This is an entry widget in this window that<ps/>"
           "uses markup <b>like this</> for styling and<ps/>"
           "formatting <em>like this</>, as well as<ps/>"
           "<a href=X><link>links in the text</></a>, so enter text<ps/>"
           "in here to edit it. By the way, links are<ps/>"
           "called <a href=anc-02>Anchors</a> so you will need<ps/>"
           "to refer to them this way.<ps/>"
           "<ps/>"

           "Also you can stick in items with (relsize + ascent): "
           "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
           " (full) "
           "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
           " (to the left)<ps/>"

           "Also (size + ascent): "
           "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
           " (full) "
           "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
           " (before this)<ps/>"

           "And as well (absize + ascent): "
           "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
           " (full) "
           "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
           " or even paths to image files on disk too like: "
           "<item absize=96x128 vsize=full href=file://bla/images/sky_01.jpg></item>"
           " ... end.";
        evas_object_textblock_text_markup_set(tb, text);
        ck_assert_str_eq(text, evas_object_textblock_text_markup_get(tb));
     }

   /* complex markup range get */
     {
        const char *text = "Break tag tes<item size=40x40 href=a></item>t <br/>Next<br/> line with it<item size=40x40 href=i></item>em tag";
        evas_object_textblock_text_markup_set(tb, text);
        evas_textblock_cursor_pos_set(main_cur, 14);
        evas_textblock_cursor_pos_set(cur, 37);
        fail_if(strcmp(evas_textblock_cursor_range_text_get(main_cur, cur,
                 EVAS_TEXTBLOCK_TEXT_MARKUP), "</item>t <br/>Next<br/> line with it<item size=40x40 href=i></item>e"));
     }

   END_TB_TEST();
}
END_TEST

/* Formats */
START_TEST(evas_textblock_formats)
{
   START_TB_TEST();
   const char *buf = "Th<b>i<font_size=15 wrap=none>s i</font_size=13>s</> a <br/> te<ps/>st<item></>.";
   const Evas_Object_Textblock_Node_Format *fnode;
   evas_object_textblock_text_markup_set(tb, buf);

   /* Walk from the start */
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ font_size=15 wrap=none"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- font_size=13"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "br"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "ps"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ item"));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(fnode);

   /* Walk backwards */
   fnode = evas_textblock_node_format_last_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ item"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "ps"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "br"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- "));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "- font_size=13"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode),
            "+ font_size=15 wrap=none"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));

   fnode = evas_textblock_node_format_prev_get(fnode);
   fail_if(fnode);

   /* Cursor and format detection */
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_format_is_visible_get(cur));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_format_is_visible_get(cur));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_format_is_visible_get(cur));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(evas_textblock_cursor_format_is_visible_get(cur));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));

   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   evas_textblock_cursor_at_format_set(cur, fnode);
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));

   size_t i = 0;
   evas_textblock_cursor_paragraph_first(cur);
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
              fail_if(!evas_textblock_cursor_is_format(cur));
              break;
           default:
              fail_if(evas_textblock_cursor_is_format(cur));
              fail_if(evas_textblock_cursor_format_is_visible_get(cur));
              break;
          }
        i++;
     }
   while (evas_textblock_cursor_char_next(cur));

   /* Format text nodes invalidation */
     {
        Evas_Coord w, h, nw, nh;
        evas_object_textblock_text_markup_set(tb, "Test");
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        evas_textblock_cursor_paragraph_first(cur);
        evas_textblock_cursor_format_prepend(cur, "+ font_size=40");
        evas_object_textblock_size_formatted_get(tb, &nw, &nh);
        fail_if((w >= nw) || (h >= nh));
     }
   /* FIXME: Should extend invalidation tests. */

   /* Various formats, just verify there's no seg, we can't really
    * verify them visually, well, we can some of them. Possibly in the
    * future we will */
   evas_object_textblock_text_markup_set(tb,
         "<font_size=40>font_size=40</><ps/>"
         "<color=#F210B3FF>color=#F210B3FF</><ps/>"
         "<underline=single underline_color=#A2B3C4>underline=single underline_color=#A2B3C4</><ps/>"
         "<underline=double underline_color=#F00 underline2_color=#00F>underline=double underline_color=#F00 underline2_color=#00F</><ps/>"
         "<underline=dashed underline_dash_color=#0F0 underline_dash_width=2 underline_dash_gap=1>underline=dashed underline_dash_color=#0F0 underline_dash_width=2 underline_dash_gap=1</><ps/>"
         "<style=outline outline_color=#F0FA>style=outline outline_color=#F0FA</><ps/>"
         "<style=shadow shadow_color=#F0F>style=shadow shadow_color=#F0F</><ps/>"
         "<style=glow glow_color=#BBB>style=glow glow_color=#BBB</><ps/>"
         "<style=glow glow2_color=#0F0>style=glow glow2_color=#0F0</><ps/>"
         "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214>style=glow color=#fff glow2_color=#fe87 glow_color=#f214</><ps/>"
         "<backing=on backing_color=#00F>backing=on backing_color=#00F</><ps/>"
         "<strikethrough=on strikethrough_color=#FF0>strikethrough=on strikethrough_color=#FF0</><ps/>"
         "<align=right>align=right</><ps/>"
         "<backing=on backing_color=#F008 valign=0.0>valign=0.0</><ps/>"
         "<backing=on backing_color=#0F08 tabstops=50>tabstops=<\\t></>50</><ps/>"
         "<backing=on backing_color=#00F8 linesize=40>linesize=40</><ps/>"
         "<backing=on backing_color=#F0F8 linerelsize=200%>linerelsize=200%</><ps/>"
         "<backing=on backing_color=#0FF8 linegap=20>linegap=20</><ps/>"
         "<backing=on backing_color=#FF08 linerelgap=100%>linerelgap=100%</><ps/>");

   /* Force a relayout */
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);

   /* Removing paired formats. */
   evas_object_textblock_text_markup_set(tb, "<a>aa<b>bb</b>cc</a>");
   fnode = evas_textblock_node_format_first_get(tb);
   evas_textblock_node_format_remove_pair(tb, (Evas_Object_Textblock_Node_Format *) fnode);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ b"));
   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- b"));

   evas_object_textblock_text_markup_set(tb, "<a>aa<b>bb</b>cc</a>");
   fnode = evas_textblock_node_format_first_get(tb);
   fnode = evas_textblock_node_format_next_get(fnode);
   evas_textblock_node_format_remove_pair(tb, (Evas_Object_Textblock_Node_Format *) fnode);
   fnode = evas_textblock_node_format_first_get(tb);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ a"));
   fnode = evas_textblock_node_format_next_get(fnode);
   fail_if(!fnode);
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "- a"));

   /* Format list get */
   evas_object_textblock_text_markup_set(tb, "<a>a</>a<item>b</>"
         "b<item>b</>c<a>c</>");
   const Eina_List *flist = evas_textblock_node_format_list_get(tb, "a");
   const Eina_List *itr;
   EINA_LIST_FOREACH(flist, itr, fnode)
     {
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ a"));
     }

   flist = evas_textblock_node_format_list_get(tb, "item");
   EINA_LIST_FOREACH(flist, itr, fnode)
     {
        fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "+ item"));
     }

   /* Make sure we get all the types of visible formats correctly. */
   evas_object_textblock_text_markup_set(tb, "<ps/>a<br/>a<tab/>a<item></>");
   fail_if(strcmp(evas_textblock_node_format_text_get(
               evas_textblock_cursor_format_get(cur)), "ps"));
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "<ps/>"));
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock_node_format_text_get(
               evas_textblock_cursor_format_get(cur)), "br"));
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "<br/>"));
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock_node_format_text_get(
               evas_textblock_cursor_format_get(cur)), "tab"));
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "<tab/>"));
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(!evas_textblock_cursor_char_next(cur));
   fail_if(strcmp(evas_textblock_node_format_text_get(
               evas_textblock_cursor_format_get(cur)), "+ item"));
   fail_if(strcmp(evas_textblock_cursor_content_get(cur), "<item>"));
   fail_if(!evas_textblock_cursor_format_is_visible_get(cur));

   evas_object_textblock_text_markup_set(tb, "abc<br/>def");
   evas_textblock_cursor_pos_set(cur, 3);
   evas_object_textblock_text_markup_prepend(cur, "<b></b>");
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "abc<b></b><br/>def");
   evas_object_textblock_text_markup_set(tb, "abc<br/>def");
   evas_textblock_cursor_pos_set(cur, 2);
   evas_object_textblock_text_markup_prepend(cur, "<b></b>");
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "ab<b></b>c<br/>def");


   END_TB_TEST();
}
END_TEST

/* Different text styles, for example, shadow. */
START_TEST(evas_textblock_style)
{
   Evas_Coord w, h, nw, nh;
   Evas_Coord l, r, t, b;
   START_TB_TEST();
   Evas_Textblock_Style *newst;
   const char *buf = "Test<ps/>Test2<ps/>נסיון";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   newst = evas_textblock_style_new();
   fail_if(!newst);
   evas_textblock_style_set(newst,
         "DEFAULT='" TEST_FONT " font_size=50 color=#000 text_class=entry'"
         "br='\n'"
         "ps='ps'"
         "tab='\t'");
   evas_object_textblock_style_set(tb, newst);
   evas_object_textblock_size_formatted_get(tb, &nw, &nh);
   fail_if((w >= nw) || (h >= nh));

   /* Style padding. */
   evas_object_textblock_text_markup_set(tb, "Test");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 0) || (t != 0) || (b != 0));

   evas_object_textblock_text_markup_set(tb, "<style=shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 1) || (t != 0) || (b != 1));

   evas_object_textblock_text_markup_set(tb, "<style=outline>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 1) || (t != 1) || (b != 1));

   evas_object_textblock_text_markup_set(tb, "<style=soft_outline>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_textblock_text_markup_set(tb, "<style=glow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 2) || (r != 2) || (t != 2) || (b != 2));

   evas_object_textblock_text_markup_set(tb, "<style=outline_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_textblock_text_markup_set(tb, "<style=far_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 2) || (t != 1) || (b != 2));

   evas_object_textblock_text_markup_set(tb, "<style=outline_soft_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_textblock_text_markup_set(tb, "<style=soft_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 3) || (t != 1) || (b != 3));

   evas_object_textblock_text_markup_set(tb, "<style=far_soft_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 0) || (r != 4) || (t != 0) || (b != 4));

   /* Mixed style padding */
   evas_object_textblock_text_markup_set(tb,
         "<style=far_shadow>Test</><style=far_soft_shadow>Test</>");
   evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
   fail_if((l != 1) || (r != 4) || (t != 1) || (b != 4));

   /* Multi-line padding */
   {
      Evas_Coord x[5] = {0}, y[5] = {0}, w2[5] = {0}, h2[5] = {0};

      // w2, h2 should not change between test 1 and 2
      // insets and x, y should increase by 2
      // line 1 in test 2 should have same geometry as in test 1 (despite style)

      evas_object_textblock_text_markup_set(tb, "Test<br/>Test");
      evas_object_textblock_line_number_geometry_get(tb, 0, &x[0], &y[0], &w2[0], &h2[0]);
      evas_object_textblock_line_number_geometry_get(tb, 1, &x[1], &y[1], &w2[1], &h2[1]);

      // check line 1 geometry relatively to line 0
      fail_if((x[0] != x[1]) || ((y[0] + h2[0]) != y[1]) || (w2[0] != w2[1]) || (h2[0] != h2[1]));

      evas_object_textblock_text_markup_set(tb, "Test<br/><style=glow>Test</><br/>Test");
      evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
      evas_object_textblock_line_number_geometry_get(tb, 0, &x[2], &y[2], &w2[2], &h2[2]);
      evas_object_textblock_line_number_geometry_get(tb, 1, &x[3], &y[3], &w2[3], &h2[3]);
      evas_object_textblock_line_number_geometry_get(tb, 2, &x[4], &y[4], &w2[4], &h2[4]);

      // check line 1 geometry relatively to line 0
      fail_if((x[2] != x[3]) || ((y[2] + h2[2]) != y[3]) || (w2[2] != w2[3]) || (h2[2] != h2[3]));

      // check padding is correct
      fail_if((x[2] != (x[0] + l)) || (y[2] != (y[0] + t)));

      // line 2 should not suffer from padding in line 1, as it is globally applied
      fail_if((x[4] != x[2]) || ((y[2] + h2[2] + h2[3]) != y[4]));
   }

   /* No font */
   evas_textblock_style_set(newst, "DEFAULT=''");
   evas_object_textblock_text_markup_set(tb, "Test");
   evas_object_textblock_size_formatted_get(tb, &nw, &nh);
   ck_assert_int_eq(nw, 0);
   ck_assert_int_eq(nw, nh);

   END_TB_TEST();
}
END_TEST

/* Various setters and getters */
START_TEST(evas_textblock_set_get)
{
   START_TB_TEST();
   const char *buf = "";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_textblock_style_get(st), style_buf));
   fail_if(evas_object_textblock_style_get(tb) != st);
   evas_object_textblock_replace_char_set(tb, "|");
   fail_if(strcmp(evas_object_textblock_replace_char_get(tb), "|"));
   evas_object_textblock_replace_char_set(tb, "ש");
   fail_if(strcmp(evas_object_textblock_replace_char_get(tb), "ש"));

   evas_object_textblock_valign_set(tb, -1.0);
   fail_if(evas_object_textblock_valign_get(tb) != 0.0);
   evas_object_textblock_valign_set(tb, 0.0);
   fail_if(evas_object_textblock_valign_get(tb) != 0.0);
   evas_object_textblock_valign_set(tb, 0.432);
   fail_if(evas_object_textblock_valign_get(tb) != 0.432);
   evas_object_textblock_valign_set(tb, 1.0);
   fail_if(evas_object_textblock_valign_get(tb) != 1.0);
   evas_object_textblock_valign_set(tb, 1.5);
   fail_if(evas_object_textblock_valign_get(tb) != 1.0);

   evas_object_textblock_bidi_delimiters_set(tb, ",.|");
   fail_if(strcmp(evas_object_textblock_bidi_delimiters_get(tb), ",.|"));
   evas_object_textblock_bidi_delimiters_set(tb, ",|");
   fail_if(strcmp(evas_object_textblock_bidi_delimiters_get(tb), ",|"));
   evas_object_textblock_bidi_delimiters_set(tb, NULL);
   fail_if(evas_object_textblock_bidi_delimiters_get(tb));
   evas_object_textblock_bidi_delimiters_set(tb, ",|");
   fail_if(strcmp(evas_object_textblock_bidi_delimiters_get(tb), ",|"));

   /* Hinting */
   evas_object_textblock_text_markup_set(tb, "This is<ps/>a test<br/>bla");
   /* Force relayout */
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_NONE);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO);
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_BYTECODE);
   END_TB_TEST();
}
END_TEST

/* Aux evas stuff, such as scale. */
START_TEST(evas_textblock_evas)
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

/* All the string escaping stuff */
START_TEST(evas_textblock_escaping)
{
   int len;
   START_TB_TEST();
   fail_if(strcmp(evas_textblock_escape_string_get("&amp;"), "&"));
   fail_if(strcmp(evas_textblock_string_escape_get("&", &len), "&amp;"));
   fail_if(len != 1);

   fail_if(strcmp(evas_textblock_escape_string_get("&middot;"), "\xc2\xb7"));
   fail_if(strcmp(evas_textblock_string_escape_get("\xc2\xb7", &len),
            "&middot;"));
   fail_if(len != 2);

   fail_if(strcmp(evas_textblock_escape_string_get("&#x1f459;"),
            "\xF0\x9F\x91\x99"));
   fail_if(strcmp(evas_textblock_escape_string_get("&#128089;"),
            "\xF0\x9F\x91\x99"));

   fail_if(evas_textblock_escape_string_get("&middot;aa"));
   const char *tmp = "&middot;aa";
   fail_if(strcmp(evas_textblock_escape_string_range_get(tmp, tmp + 8),
            "\xc2\xb7"));
   fail_if(evas_textblock_escape_string_range_get(tmp, tmp + 9));
   fail_if(evas_textblock_escape_string_range_get(tmp, tmp + 7));
   fail_if(evas_textblock_escape_string_range_get(tmp, tmp + 5));

   const char *buf = "This &middot; is";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "This \xc2\xb7 is"));

   buf = "This &nbsp; is";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "This \xc2\xa0 is"));

   END_TB_TEST();
}
END_TEST

START_TEST(evas_textblock_size)
{
   START_TB_TEST();
   Evas_Coord w, h, h2, nw, nh;
   const char *buf = "This is a <br/> test.<br/>גם בעברית";

   /* Empty textblock */
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);
   fail_if(w != 0);

   /* When wrapping is off, native size should be the same as formatted
    * size */

   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   evas_object_textblock_text_markup_set(tb, "a<br/>a");
   evas_object_textblock_size_formatted_get(tb, &w, &h2);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h2, nh);

   /* Two lines != double the height */
   fail_if(h * 2 == h2);

   evas_object_textblock_text_markup_set(tb, buf);

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h != nh));
   fail_if(w <= 0);

   /* This time with margins. */
     {
        Evas_Textblock_Style *newst;
        Evas_Coord oldw, oldh, oldnw, oldnh;

        evas_object_textblock_text_markup_set(tb, buf);
        evas_object_textblock_size_formatted_get(tb, &oldw, &oldh);
        evas_object_textblock_size_native_get(tb, &oldnw, &oldnh);


        newst = evas_textblock_style_new();
        fail_if(!newst);
        evas_textblock_style_set(newst,
              "DEFAULT='left_margin=4 right_margin=4'");
        evas_object_textblock_style_user_push(tb, newst);

        evas_object_textblock_size_formatted_get(tb, &w, &h);
        evas_object_textblock_size_native_get(tb, &nw, &nh);

        fail_if((w != oldw + 8) || (h != oldh) ||
              (nw != oldnw + 8) || (nh != oldnh));
     }

   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
END_TEST

void evas_test_textblock(TCase *tc)
{
   tcase_add_test(tc, evas_textblock_simple);
   tcase_add_test(tc, evas_textblock_cursor);
#ifdef HAVE_FRIBIDI
   tcase_add_test(tc, evas_textblock_split_cursor);
#endif
   tcase_add_test(tc, evas_textblock_size);
   tcase_add_test(tc, evas_textblock_editing);
   tcase_add_test(tc, evas_textblock_style);
   tcase_add_test(tc, evas_textblock_evas);
   tcase_add_test(tc, evas_textblock_text_getters);
   tcase_add_test(tc, evas_textblock_formats);
   tcase_add_test(tc, evas_textblock_format_removal);
   tcase_add_test(tc, evas_textblock_escaping);
   tcase_add_test(tc, evas_textblock_set_get);
   tcase_add_test(tc, evas_textblock_geometries);
   tcase_add_test(tc, evas_textblock_various);
   tcase_add_test(tc, evas_textblock_wrapping);
   tcase_add_test(tc, evas_textblock_items);
}

