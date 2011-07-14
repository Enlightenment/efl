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

   /* Try positions beyond the left/right limits of lines. */
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
   evas_textblock_cursor_char_coord_set(main_cur, x - 50, y);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

   evas_textblock_cursor_line_char_last(cur);
   evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
   evas_textblock_cursor_char_coord_set(main_cur, x + 50, y);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

   evas_textblock_cursor_line_set(cur, 1);
   evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
   evas_textblock_cursor_char_coord_set(main_cur, x - 50, y);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

   evas_textblock_cursor_line_char_last(cur);
   evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
   evas_textblock_cursor_char_coord_set(main_cur, x + 50, y);
   fail_if(evas_textblock_cursor_compare(main_cur, cur));

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


   /* FIXME: Add tests that check positions left of/right of rtl lines. */

   END_TB_TEST();
}
END_TEST

/* Testing items */
START_TEST(evas_textblock_items)
{
   Evas_Coord w, h, ih;
   START_TB_TEST();
   const char *buf = "This is an <item absize=93x152></>.";

   /* Absolute item size */
   buf = "This is an <item absize=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h != 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   buf = "This is an <item absize=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h <= 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   /* Size is the same as abssize, unless there's scaling applied. */
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h != 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < 93) || (h <= 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != 93) || (h != 152));

   evas_object_scale_set(tb, 2.0);
   buf = "This is an <item size=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h != (2 * 152)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   buf = "This is an <item size=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w < (2 * 93)) || (h <= (2 * 152)));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h);
   fail_if((w != (2 * 93)) || (h != (2 * 152)));

   evas_object_scale_set(tb, 1.0);

   /* Relsize */
   /* relsize means it should adjust itself to the size of the line */
   buf = "This is an <item relsize=93x152 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 90) || (h != ih));

   buf = "This is an <item relize=93x152 vsize=ascent></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w >= 93) || (h >= 152));
   evas_textblock_cursor_pos_set(cur, 11);
   evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &ih);
   fail_if((w > 90) || (h <= ih));

   /* FIXME: Also verify x,y positions of the item. */

   /* FIXME We need some item tests that involve line wrapping that make the
    * items move between lines that are in different sizes. */

   END_TB_TEST();
}
END_TEST

/* Wrapping tests */
START_TEST(evas_textblock_wrapping)
{
   Evas_Coord bw, bh, w, h, nw, nh;
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
#if 0
   evas_object_textblock_text_markup_set(tb, "aaaaaaa");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, bw, bh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   /* Wrap to minimum */
   fail_if(w != bw);
   fail_if(h <= bh);
#endif

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
         "aaaa aaaa aaa aa aaa<ps>"
         "aaaa aaa aaa aaa aaa<ps>"
         "a aaaaa aaaaaaaaaaaaaa<br>aaaaa<ps>"
         "aaaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   Evas_Coord iw;
   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if((w < bw) || (h < bh));
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Word wrap */
   evas_object_textblock_text_markup_set(tb, "aaaaaa");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa<ps>"
         "aaaa aaa aaa aaa aaa<ps>"
         "a aaaaa aaaaaa<br>aaaaa<ps>"
         "aaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=word");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if((w < bw) || (h < bh));
        fail_if(w > iw);
     }
   fail_if(w != bw);

   /* Mixed wrap */
#if 0
   evas_object_textblock_text_markup_set(tb, "a");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_textblock_text_markup_set(tb,
         "aaaa aaaa aaa aa aaa<ps>"
         "aaaa aaa aaa aaa aaa<ps>"
         "a aaaaa aaaaaa<br>aaaaa<ps>"
         "aaaaa"
         );
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        fail_if((w < bw) || (h < bh));
        fail_if(w > iw);
     }
   fail_if(w != bw);
#endif

   /* Ellipsis */
   evas_object_textblock_text_markup_set(tb, "aaaaaaaaaa");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   fail_if((w > (nw / 2)) || (h != nh));

   END_TB_TEST();
}
END_TEST

/* Various textblock stuff */
START_TEST(evas_textblock_various)
{
   Evas_Coord w, h, bw, bh;
   START_TB_TEST();
   const char *buf = "This<ps>textblock<ps>has<ps>a<ps>lot<ps>of<ps>lines<ps>.";
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
   END_TB_TEST();
}
END_TEST

/* Various geometries. e.g. range geometry. */
START_TEST(evas_textblock_geometries)
{
   START_TB_TEST();
   const char *buf = "This is a <br> test.";
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

   END_TB_TEST();
}
END_TEST

/* Should handle all the text editing. */
START_TEST(evas_textblock_editing)
{
   START_TB_TEST();
   const char *buf = "First par.<ps>Second par.";
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
            "irst par.<ps>Second par."));

   /* Delete some arbitrary char */
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_next(cur);
   evas_textblock_cursor_char_delete(cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "irs par.<ps>Second par."));

   /* Delete a range */
   evas_textblock_cursor_pos_set(main_cur, 1);
   evas_textblock_cursor_pos_set(cur, 6);
   evas_textblock_cursor_range_delete(cur, main_cur);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb),
            "ir.<ps>Second par."));
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
            "First par.<ps>"));

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

   /* FIXME: Also add text appending/prepending */

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

/* Formats */
START_TEST(evas_textblock_formats)
{
   START_TB_TEST();
   const char *buf = "Th<b>i<font_size=15 wrap=none>s i</font_size=13>s</> a <br> te<ps>st<item></>.";
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
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "\n"));

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
   fail_if(strcmp(evas_textblock_node_format_text_get(fnode), "\n"));

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

   /* Various formats, just verify there's no seg, we can't really
    * verify them visually, well, we can some of them. Possibly in the
    * future we will */
   evas_object_textblock_text_markup_set(tb,
         "<font_size=40>font_size=40</><ps>"
         "<color=#F210B3FF>color=#F210B3FF</><ps>"
         "<underline=single underline_color=#A2B3C4>underline=single underline_color=#A2B3C4</><ps>"
         "<underline=double underline_color=#F00 underline2_color=#00F>underline=double underline_color=#F00 underline2_color=#00F</><ps>"
         "<style=outline outline_color=#F0FA>style=outline outline_color=#F0FA</><ps>"
         "<style=shadow shadow_color=#F0F>style=shadow shadow_color=#F0F</><ps>"
         "<style=glow glow_color=#BBB>style=glow glow_color=#BBB</><ps>"
         "<style=glow glow2_color=#0F0>style=glow glow2_color=#0F0</><ps>"
         "<style=glow color=#fff glow2_color=#fe87 glow_color=#f214>style=glow color=#fff glow2_color=#fe87 glow_color=#f214</><ps>"
         "<backing=on backing_color=#00F>backing=on backing_color=#00F</><ps>"
         "<strikethrough=on strikethrough_color=#FF0>strikethrough=on strikethrough_color=#FF0</><ps>"
         "<align=right>align=right</><ps>"
         "<backing=on backing_color=#F008 valign=0.0>valign=0.0</><ps>"
         "<backing=on backing_color=#0F08 tabstops=50>tabstops=<\\t></>50</><ps>"
         "<backing=on backing_color=#00F8 linesize=40>linesize=40</><ps>"
         "<backing=on backing_color=#F0F8 linerelsize=200%>linerelsize=200%</><ps>"
         "<backing=on backing_color=#0FF8 linegap=20>linegap=20</><ps>"
         "<backing=on backing_color=#FF08 linerelgap=100%>linerelgap=100%</><ps>");

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
   const char *buf = "Test<ps>Test2<ps>נסיון";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   newst = evas_textblock_style_new();
   fail_if(!newst);
   evas_textblock_style_set(newst,
         "DEFAULT='font=Sans font_size=20 color=#000 text_class=entry'"
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
   evas_object_textblock_text_markup_set(tb, "This is<ps>a test<br>bla");
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
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));

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
   tcase_add_test(tc, evas_textblock_evas);
   tcase_add_test(tc, evas_textblock_text_getters);
   tcase_add_test(tc, evas_textblock_formats);
   tcase_add_test(tc, evas_textblock_escaping);
   tcase_add_test(tc, evas_textblock_set_get);
   tcase_add_test(tc, evas_textblock_geometries);
   tcase_add_test(tc, evas_textblock_various);
   tcase_add_test(tc, evas_textblock_wrapping);
   tcase_add_test(tc, evas_textblock_items);
}

