/*
 * TODO:
 * * Test different font loading mechanisms.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <locale.h>

#include <Eina.h>
#include <Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"
#include "evas_buffer_helper.h"

#define TESTS_DIC_DIR TESTS_SRC_DIR"/dicts"

/* Functions defined in evas_object_textblock.c */
EVAS_API Eina_Bool
_evas_textblock_check_item_node_link(Evas_Object *obj);
EVAS_API int
_evas_textblock_format_offset_get(const Evas_Object_Textblock_Node_Format *n);
/* end of functions defined in evas_object_textblock.c */

#define TEST_FONT "font=DejaVuSans,UnDotum,malayalam font_source=" TESTS_SRC_DIR "/fonts/TestFont.eet"
#define TEST_FONT_SOURCE TESTS_SRC_DIR "/fonts/TestFontSource.eet"

static const char *style_buf =
   "DEFAULT='" TEST_FONT " font_size=10 color=#000 text_class=entry'"
   "newline='br'"
   "b='+ font_weight=bold'";

#define START_TB_TEST() \
   Evas *evas; \
   Evas_Object *tb; \
   Evas_Textblock_Style *st; \
   Evas_Textblock_Cursor *cur; \
   Efl_Text_Cursor_Object       *cur_obj; \
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
   cur_obj =efl_canvas_textblock_cursor_create(tb);\
   (void) cur_obj;\
   (void) cur;\
do \
{ \
} \
while (0)

#define END_TB_TEST() \
do \
{ \
   evas_textblock_cursor_free(cur); \
   efl_del(cur_obj); \
   evas_object_del(tb); \
   evas_textblock_style_free(st); \
   evas_free(evas); \
} \
while (0)


typedef struct _Escape_Value Escape_Value;

struct _Escape_Value
{
   char *escape;
   char *value;
};

#define ESCAPE_VALUE(e,v) {e,v}

static const Escape_Value escape_strings[] = {
   ESCAPE_VALUE("&Aacute;", "\xc3\x81"),
   ESCAPE_VALUE("&Acirc;", "\xc3\x82"),
   ESCAPE_VALUE("&Aelig;", "\xc3\x86"),
   ESCAPE_VALUE("&Otilde;", "\xc3\x95"),
   ESCAPE_VALUE("&Ouml;", "\xc3\x96"),
   ESCAPE_VALUE("&Thorn;", "\xc3\x9e"),
   ESCAPE_VALUE("&Uacute;", "\xc3\x9a"),
   ESCAPE_VALUE("&Ucirc;", "\xc3\x9b"),
   ESCAPE_VALUE("&rArr;", "\xe2\x87\x92"),
   ESCAPE_VALUE("&reg;", "\xc2\xae"),
   ESCAPE_VALUE("&rho;", "\xce\xa1"),
   ESCAPE_VALUE("&sigma;", "\xce\xa3"),
   ESCAPE_VALUE("&sum;", "\xe2\x88\x91"),
   ESCAPE_VALUE("&sup1;", "\xc2\xb9"),
   ESCAPE_VALUE("&ucirc;", "\xc3\xbb"),
   ESCAPE_VALUE("&ugrave;", "\xc3\xb9"),
   ESCAPE_VALUE("&uml;", "\xc2\xa8"),
   ESCAPE_VALUE("&yen;", "\xc2\xa5"),
   ESCAPE_VALUE("&yuml;", "\xc3\xbf"),
   ESCAPE_VALUE("<tab/>", "\t"),
   ESCAPE_VALUE("<br/>", "\n"),
   ESCAPE_VALUE("<ps/>", "\xe2\x80\xa9"),    //0x2029
   ESCAPE_VALUE("&#xfffc;", "\xef\xbf\xbc"), //0xFFFC
};

EFL_START_TEST(evas_textblock_simple)
{
   START_TB_TEST();
   const char *buf = "Th<i>i</i>s is a <br/> te<b>s</b>t.";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));

   /* Set markup text(includes tag) without setting style */
   Evas_Object *tb2 = evas_object_textblock_add(evas);
   fail_if(!tb2);
   evas_object_textblock_text_markup_set(tb2, buf);
   ck_assert("Crash Not occurred");
   evas_object_del(tb2);

   END_TB_TEST();
}
EFL_END_TEST

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
EFL_START_TEST(evas_textblock_cursor)
{
   START_TB_TEST();
   Evas_Textblock_Cursor *cur2;
   Evas_Coord x, y, w, h;
   size_t i, j, len;
   Evas_Coord nw, nh;
#ifdef HAVE_FRIBIDI
   Evas_BiDi_Direction dir;
#endif
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
        ck_assert_int_eq(evas_textblock_cursor_pos_get(cur), (int) len);
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
   /* Check direction */
   evas_object_textblock_text_markup_set(tb, "test");
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "test"));
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   evas_object_textblock_text_markup_set(tb, "עוד פסקה");
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "עוד פסקה"));
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   evas_object_textblock_text_markup_set(tb, "123");
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "123"));
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   evas_object_textblock_text_markup_set(tb, "%^&amp;");
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "%^&amp;"));
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, NULL, NULL, NULL, NULL, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_BEFORE);
   fail_if(dir != EVAS_BIDI_DIRECTION_LTR);

   /* Check direction with evas_object_paragraph_direction_set API */
     {
        Evas_Coord xx, yy, ww, hh;

        /* LTR text case */
        evas_object_textblock_text_markup_set(tb, "test");
        fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "test"));

        /* EVAS_TEXTBLOCK_CURSOR_UNDER */
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        /* EVAS_TEXTBLOCK_CURSOR_BEFORE */
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        /* RTL text case */
        evas_object_textblock_text_markup_set(tb, "עוד פסקה");
        fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "עוד פסקה"));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x <= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x <= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        /* NEUTRAL(European Number) text case */
        /* It doesn't change characters sequence. */
        evas_object_textblock_text_markup_set(tb, "123");
        fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "123"));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        /* NEUTRAL(Other Neutrals) text case */
        /* It changes characters sequence. */
        evas_object_textblock_text_markup_set(tb, "%^&amp;");
        fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "%^&amp;"));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_UNDER);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_RTL);
        dir = EVAS_BIDI_DIRECTION_LTR;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
        fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_LTR);
        dir = EVAS_BIDI_DIRECTION_RTL;
        evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                           EVAS_TEXTBLOCK_CURSOR_BEFORE);
        fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
        fail_if((x != xx) || (y != yy) || (w != ww) || (h != hh));

        /* Reset paragraph direction */
        evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
     }

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
             fail_if((x < lx) ||
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
             fail_if((x < lx) ||
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

        evas_object_textblock_text_markup_set(tb, "123<br/>456<br/>789");
        evas_object_textblock_valign_set(tb, 0.0);
        evas_textblock_cursor_pos_set(cur, 6);
        ck_assert_int_eq(evas_textblock_cursor_line_coord_set(cur, 0), 0);
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

        /* Bug with 1 char word separators at paragraph start. */
        evas_object_textblock_text_markup_set(tb, "=test");
        evas_textblock_cursor_pos_set(cur, 4);
        evas_textblock_cursor_word_start(cur);
        fail_if(1 != evas_textblock_cursor_pos_get(cur));

        /* 1 char words separated by spaces. */
        evas_object_textblock_text_markup_set(tb, "a a a a");
        evas_textblock_cursor_paragraph_first(cur);

        evas_textblock_cursor_word_end(cur);
        ck_assert_int_eq(0, evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_word_start(cur);
        ck_assert_int_eq(0, evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_pos_set(cur, 2);
        evas_textblock_cursor_word_start(cur);
        ck_assert_int_eq(2, evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_word_end(cur);
        ck_assert_int_eq(2, evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_pos_set(cur, 3);
        evas_textblock_cursor_word_start(cur);
        ck_assert_int_eq(2, evas_textblock_cursor_pos_get(cur));
        evas_textblock_cursor_pos_set(cur, 3);
        evas_textblock_cursor_word_end(cur);
        ck_assert_int_eq(4, evas_textblock_cursor_pos_get(cur));

        /* Going back when ending with whites. */
        evas_object_textblock_text_markup_set(tb, "aa bla ");
        evas_textblock_cursor_paragraph_last(cur);

        evas_textblock_cursor_word_start(cur);
        ck_assert_int_eq(3, evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_word_end(cur);
        ck_assert_int_eq(5, evas_textblock_cursor_pos_get(cur));

        /* moving across paragraphs */
        evas_object_textblock_text_markup_set(tb,
                                              "test<ps/>"
                                              "  case");
        evas_textblock_cursor_pos_set(cur, 4);
        evas_textblock_cursor_word_end(cur);
        ck_assert_int_eq(10, evas_textblock_cursor_pos_get(cur));

        evas_textblock_cursor_pos_set(cur, 6);
        evas_textblock_cursor_word_start(cur);
        ck_assert_int_eq(0, evas_textblock_cursor_pos_get(cur));
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

   /* Line set with BiDi text */
     {
        size_t pos;

        evas_object_textblock_text_markup_set(tb,
              "שלום עולם hello world<ps>"
              "שלום עולם hello world<ps>"
              "hello world שלום עולם");
        evas_textblock_cursor_line_set(cur, 0);
        pos = evas_textblock_cursor_pos_get(cur);
        ck_assert_int_eq(pos, 0);
        evas_textblock_cursor_line_set(cur, 1);
        pos = evas_textblock_cursor_pos_get(cur);
        ck_assert_int_eq(pos, 22);
        evas_textblock_cursor_line_set(cur, 2);
        pos = evas_textblock_cursor_pos_get(cur);
        ck_assert_int_eq(pos, 44);
     }

     {
        /* Test multiple cursors:
         * Deleting characters should "pull" all the cursors positioned *after*
         * the deleted position, and "push" on insertion.
         * Testing with one additional cursor will suffice. */
        int j, pos;
        Evas_Textblock_Cursor *cur2;

        cur2 = evas_object_textblock_cursor_new(tb);
        evas_object_textblock_text_markup_set(tb, "Hello world");
        evas_textblock_cursor_pos_set(cur2, 0);
        evas_textblock_cursor_pos_set(cur, 5);
        for (j = 5; j >= 0; j--)
          {
             pos = evas_textblock_cursor_pos_get(cur);
             ck_assert_int_eq(pos, j);
             evas_textblock_cursor_char_delete(cur2);
          }
        evas_object_textblock_text_markup_set(tb, "Hello world");
        evas_textblock_cursor_pos_set(cur2, 0);
        evas_textblock_cursor_pos_set(cur, 5);
        for (j = 5; j <= 10; j++)
          {
             pos = evas_textblock_cursor_pos_get(cur);
             ck_assert_int_eq(pos, j);
             evas_textblock_cursor_text_append(cur2, "a");
          }
        evas_textblock_cursor_free(cur2);
     }

   /* Testing for grapheme cluster */
   cur2 = evas_object_textblock_cursor_new(tb);
   evas_object_textblock_text_markup_set(tb, "ഹലോ");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(cur2, 0);

   i = j = 0;
   while (evas_textblock_cursor_cluster_next(cur)) i++;
   ck_assert_int_eq(i, 2);

   while (evas_textblock_cursor_char_next(cur2)) j++;
   ck_assert_int_eq(j, 4);

   i = j = 0;
   while (evas_textblock_cursor_cluster_prev(cur)) i++;
   ck_assert_int_eq(i, 2);

   while (evas_textblock_cursor_char_prev(cur2)) j++;
   ck_assert_int_eq(j, 4);

   //make sure if we have cluster at line start we return to pos 0
   Evas_Coord x_coord, y_coord;
   int pos;

   cur2 = evas_object_textblock_cursor_new(tb);
   evas_object_textblock_text_markup_set(tb, "&#x262a;&#xfe0f;");

   evas_textblock_cursor_char_next(cur2);
   evas_textblock_cursor_pen_geometry_get(cur2, &x_coord, &y_coord, NULL, NULL);
   evas_textblock_cursor_cluster_coord_set(cur, x_coord, y_coord);
   pos = evas_textblock_cursor_pos_get(cur);
   
   ck_assert_int_eq(pos, 0);
   evas_object_textblock_text_markup_set(tb, "&#x1f3f3;&#xfe0f;&#x200d;&#x1f308;");
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_cluster_next(cur);
   ck_assert_int_eq(4, evas_textblock_cursor_pos_get(cur));

   END_TB_TEST();
}
EFL_END_TEST

#ifdef HAVE_FRIBIDI
EFL_START_TEST(evas_textblock_split_cursor)
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
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL, NULL,
            &cx2, NULL, NULL, NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE));
   evas_textblock_cursor_pos_set(cur, 18);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 20);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);

   /* Logical cursor before "a" */
   evas_textblock_cursor_pos_set(cur, 11);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL, NULL,
            &cx2, NULL, NULL, NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE));
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
EFL_END_TEST
#endif

EFL_START_TEST(evas_textblock_format_removal)
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
EFL_END_TEST

/* Testing items */
EFL_START_TEST(evas_textblock_items)
{
   Evas_Coord x, y, w, h, w2, h2, nw, nh, ih;
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

   buf = "<ellipsis=1.0>a<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item></ellipsis>";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_resize(tb, 30, 30);
   evas_textblock_cursor_pos_set(cur, 1);
   if (evas_textblock_cursor_format_item_geometry_get(cur, NULL, NULL, &w, &h))
     fail_if((w != 64) || (h != 64));

   /* Test char coordinate for item at middle position of the item to decide cursor position,
    * it means when char coordinate exceeds the half width of the item then only
    * cursor position is changed. */
   buf = "<item size=100x100 vsize=full></>.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_format_item_geometry_get(cur, &x, &y, &w, NULL);
   evas_textblock_cursor_char_coord_set(cur, x + (w / 2) + 1, y);
   fail_if(evas_textblock_cursor_pos_get(cur) != 1);
   /* Test small increment in x and cursor position will be same */
   evas_textblock_cursor_char_coord_set(cur, x + 10, y);
   fail_if(evas_textblock_cursor_pos_get(cur) != 0);

   /* FIXME: Also verify x,y positions of the item. */

   /* FIXME We need some item tests that involve line wrapping that make the
    * items move between lines that are in different sizes.
    * Also, tests that involve wrapping positions with relsized items. We
    * want to make sure the item gets a relsize on the correct time (before
    * the wrapping, and then is updated after the wrapping) and that
    * all the lines have the correct sizes afterwards. */

   END_TB_TEST();
}
EFL_END_TEST

/* Wrapping tests */
EFL_START_TEST(evas_textblock_wrapping)
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
   ck_assert_int_eq(w, bw);
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
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

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

   /* Verify that no empty line is added */
   evas_object_textblock_text_markup_set(tb, "<wrap=word>Hello</wrap>");
   evas_object_textblock_size_native_get(tb, NULL, &nh);
   evas_object_resize(tb, 0, 1000);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_eq(nh, h);

   evas_object_textblock_text_markup_set(tb, "<wrap=char>a</wrap>");
   evas_object_textblock_size_native_get(tb, NULL, &nh);
   evas_object_resize(tb, 0, 1000);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_eq(nh, h);

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
   int ellip_w = 0;
   evas_object_textblock_text_markup_set(tb, "…");
   evas_object_textblock_size_native_get(tb, &ellip_w, NULL);

   evas_object_textblock_text_markup_set(tb, "aaaaaaaaaaaaaaaaaa<br/>b");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0 wrap=word");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, (nw / 2));

   evas_object_textblock_text_markup_set(tb, "a<b>b</b>a<b>b</b>a<b>b</b>");
   evas_textblock_cursor_format_prepend(cur, "+ font_size=50 ellipsis=1.0");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, (nw / 2));

   /* Vertical ellipsis when text includes "br" tags */
   evas_object_textblock_text_markup_set(tb, "AAAA<br/>BBBB<br/>CCCC<br/>DDDD<br/>EEEE<br/>FFFF<br/>");
   evas_textblock_cursor_format_prepend(cur, "+ font_size=20 ellipsis=1.0");
   evas_object_resize(tb, 500, 500);
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_object_resize(tb, bw * 10, bh / 2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(h, (bh / 2));

   evas_object_textblock_text_markup_set(tb, "<item absize=100x100 href=item1></item><item absize=100x100 href=item2></item>");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_resize(tb, 101, 100);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, 100);

   evas_object_textblock_text_markup_set(tb, "ab");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 2);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_le(w, ellip_w);

   /* Ellipsis test for multiple items in singleline. */
     {
        evas_object_resize(tb, 500, 500);
        evas_object_textblock_text_markup_set(tb, "ABC 한글한글 DEF");
        evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock_size_formatted_get(tb, &w, &h);

        /* Make the object's width smaller. */
        for (i = 1; (nw / 5) <= (nw - i); i++)
          {
             evas_object_resize(tb, nw - i, nh);
             evas_object_textblock_size_formatted_get(tb, &bw, &bh);
             ck_assert_int_le(bw, w);
          }

        /* Revert the object's width to native width. */
        for (; (nw - i) <= nw; i--)
          {
             evas_object_resize(tb, nw - i, nh);
             evas_object_textblock_size_formatted_get(tb, &bw, &bh);
             ck_assert_int_le(bw, w);
          }

        /* The object resized same as native size.
         * So, formatted width and native width should be same
         * just like our first check. */
        ck_assert_int_eq(bw, w);
     }

   {
      double ellip;
      for(ellip = 0.0; ellip <= 1.0; ellip = ellip + 0.1)
        {
           char buf[128];
           Evas_Coord w1, h1, w2, h2;

           sprintf(buf, "+ ellipsis=%f", ellip);
           evas_object_textblock_text_markup_set(tb, "aaaaaaaaaa");
           evas_textblock_cursor_format_prepend(cur, buf);
           evas_object_textblock_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock_size_formatted_get(tb, &w, &h);
           ck_assert_int_le(w, (nw / 2));
           ck_assert_int_eq(h, nh);

           evas_object_textblock_text_markup_set(tb, "aaaaaaaaaa");
           evas_textblock_cursor_format_prepend(cur, buf);
           evas_object_textblock_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw, nh);
           evas_object_textblock_size_formatted_get(tb, &w, &h);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock_size_formatted_get(tb, &w1, &h1);
           evas_object_resize(tb, nw, nh);
           evas_object_textblock_size_formatted_get(tb, &w2, &h2);
           ck_assert_int_eq(w, w2);
           ck_assert_int_eq(h, h2);

           sprintf(buf, "+ ellipsis=%f", ellip);
           evas_object_textblock_text_markup_set(tb,
                 "the<tab>quick brown fox"
                 "jumps<tab> over the<tab> lazy dog"
                 );
           evas_textblock_cursor_format_prepend(cur, buf);
           evas_object_textblock_size_native_get(tb, &nw, &nh);
           evas_object_resize(tb, nw / 2, nh);
           evas_object_textblock_size_formatted_get(tb, &w, &h);
           ck_assert_int_le(w, (nw / 2));
           ck_assert_int_eq(h, nh);
        }
   }

   evas_object_textblock_text_markup_set(tb, "aaaaaaaaaaaaaaaaaa");
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0 wrap=char");
   nw /= 3;
   nh *= 2;
   evas_object_resize(tb, nw, nh);

     {
        evas_object_textblock_line_number_geometry_get(tb, 1, NULL, NULL, &w, NULL);
        ck_assert_int_gt(w, ellip_w);
     }

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
   ck_assert_int_eq(w, 32);
   _ck_assert_int(h, >=, 25);

   evas_object_resize(tb, 400, 400);

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   _ck_assert_int(w, >=, 44);
   ck_assert_int_eq(h, 16);

   /* Complex compound clusters using Devanagari. */
   evas_object_resize(tb, 0, 0);

   evas_object_textblock_text_markup_set(tb, "<wrap=char> करेंकरेंकरेंकरेंकरेंकरें");
   evas_object_textblock_size_formatted_get(tb, &w, &h);

   fail_if(w > h); /* FIXME: Not the best test, should be more strict. */

   evas_object_textblock_text_markup_set(tb,
         "<wrap=none>aaa bbbbbbbbbbb ccccc</wrap><wrap=word>dddddd</wrap>");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw / 2, nh * 4);
   evas_object_textblock_size_formatted_get(tb, &w, NULL);
   ck_assert_int_le(w, nw);

     {
        int bret, ret;
        evas_object_textblock_text_markup_set(tb,
              "<ellipsis=1.0>aaa<ps>bbb</ellipsis>");
        evas_object_resize(tb, 1, 1);
        evas_object_textblock_size_formatted_get(tb, NULL, NULL);
        evas_textblock_cursor_line_set(cur, 1);
        bret = evas_textblock_cursor_pos_get(cur);

        evas_object_resize(tb, 500, 500);
        evas_object_textblock_size_formatted_get(tb, NULL, NULL);
        evas_object_resize(tb, 1, 1);
        evas_object_textblock_size_formatted_get(tb, NULL, NULL);
        evas_textblock_cursor_line_set(cur, 1);
        ret = evas_textblock_cursor_pos_get(cur);

        ck_assert_int_eq(bret, ret);
     }

   /* Check that line wrap produces the same height values as paragraph break */
   evas_object_resize(tb, 1, 100);
   evas_object_textblock_text_markup_set(tb, "<wrap=word>hello world");
   evas_object_textblock_size_formatted_get(tb, NULL, &bh);
   evas_object_textblock_text_markup_set(tb, "hello<ps>world");
   evas_object_textblock_size_formatted_get(tb, NULL, &h);

   ck_assert_int_eq(bh, h);

   /* Check that unnecessary ellipsis is not applied */
   evas_object_textblock_text_markup_set(tb, "This is test for ellipsis with formatted height.");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
   evas_object_resize(tb, 100, 100);
   evas_object_textblock_size_formatted_get(tb, NULL, &bh);
   evas_object_resize(tb, 100, bh);
   evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed ellipsis=1.0");
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_ge(h, bh);

   /* Check char-wrapping for small items */
   evas_object_textblock_text_markup_set(tb, "x");
   evas_object_textblock_size_native_get(tb, &bw, NULL);
   evas_object_textblock_text_markup_set(tb, "AxAx");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw - bw, nh);
   evas_object_textblock_size_formatted_get(tb, &bw, NULL);

   evas_object_textblock_text_markup_set(tb, "A<color=#f00>x</color>Ax");
   evas_textblock_cursor_format_prepend(cur, "+ wrap=char");
   evas_object_textblock_size_formatted_get(tb, &w, NULL);
   ck_assert_int_eq(bw, w);

#ifdef HAVE_FRIBIDI
   /* Check the ellipsis is placed at proper place
    * in RTL text with formats */
   evas_object_textblock_text_markup_set(tb, ")");
   evas_object_textblock_size_native_get(tb, &bw, NULL);
   bw++;

   /* Expect to see: "...)ي" */
   evas_object_textblock_text_markup_set(tb, "ي(ي)");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw - bw, nh);
   evas_object_textblock_size_formatted_get(tb, &bw, NULL);

   /* Expect to see: "...)ي"
    * But, Evas Textblock could put ellipsis item at wrong place: ")...ي"
    * Then, formatted size could be different from the case without format. */
   evas_object_textblock_text_markup_set(tb, "ي<color=#f00>(</color>ي)");
   evas_textblock_cursor_format_prepend(cur, "+ ellipsis=1.0");
   evas_object_textblock_size_formatted_get(tb, &w, NULL);
   ck_assert_int_eq(bw, w);
#endif

   END_TB_TEST();
}
EFL_END_TEST

/* Various textblock stuff */
EFL_START_TEST(evas_textblock_various)
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

   /* Check margins' position */
     {
        Evas_Coord nw, nh, lx, lw;

        evas_object_textblock_text_markup_set(tb, "This is a test");
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 0);
        ck_assert_int_eq(lx + lw, nw);

        evas_object_textblock_text_markup_set(tb, "<left_margin=10 right_margin=5>This is a test</>");
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 10);
        ck_assert_int_eq(lx + lw + 5, nw);

        evas_object_textblock_text_markup_set(tb, "עוד פסקה");
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 0);
        ck_assert_int_eq(lx + lw, nw);

        evas_object_textblock_text_markup_set(tb, "<left_margin=10 right_margin=5>עוד פסקה</>");
        evas_object_textblock_size_native_get(tb, &nw, &nh);
        evas_object_resize(tb, nw, nh);
        evas_object_textblock_line_number_geometry_get(tb, 0, &lx, NULL, &lw, NULL);
        ck_assert_int_eq(lx, 10);
        ck_assert_int_eq(lx + lw + 5, nw);
     }

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
EFL_END_TEST

/* Various geometries. e.g. range geometry. */
EFL_START_TEST(evas_textblock_geometries)
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
   evas_object_textblock_text_markup_set(tb, "עברית");
   evas_textblock_cursor_pos_set(main_cur, 4); // last character
   evas_textblock_cursor_pos_set(cur, 5); // after last character

   rects = evas_textblock_cursor_range_geometry_get(cur, main_cur);
   fail_if(!rects);
   EINA_LIST_FREE(rects, tr)
      free(tr);

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

   /* Range simple geometry */
   /* Single line range */
   Eina_Iterator *it, *it2;
   Evas_Textblock_Rectangle *tr3;
   Evas_Coord w = 200;
   evas_object_textblock_text_markup_set(tb, "This <br/> is a test.<br/>Another <br/>text.");
   evas_object_resize(tb, w, w / 2);
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 3);

   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock_cursor_range_simple_geometry_get(main_cur, cur);
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

   eina_iterator_free(it);
   eina_iterator_free(it2);

   /* Multiple range */
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 16);

   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock_cursor_range_simple_geometry_get(main_cur, cur);
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
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 31);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   it2 = evas_textblock_cursor_range_simple_geometry_get(main_cur, cur);
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

   eina_iterator_free(it);
   eina_iterator_free(it2);

   /* Check number of rectangles in case a of line wrapping */
   evas_object_textblock_text_markup_set(tb, "<wrap=word>abc def <color=#0ff>ghi");
   evas_object_resize(tb, w, w / 2);
   evas_textblock_cursor_pos_set(cur, 10);

     {
        Evas_Coord cx;
        evas_textblock_cursor_geometry_bidi_get(cur, &cx, NULL, NULL,
              NULL, NULL, NULL, NULL, NULL,
              EVAS_TEXTBLOCK_CURSOR_BEFORE);
        /* enforce wrapping of "ghi" to the next line */
        evas_object_resize(tb, cx, 400);
        /* Sanity, check there is actually a second line */
        fail_if (!evas_textblock_cursor_line_set(cur, 1));
        fail_if (evas_textblock_cursor_line_set(cur, 2));
     }

   evas_textblock_cursor_pos_set(cur, 7);
   evas_textblock_cursor_pos_set(main_cur, 9);

   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 2);

     {
        Evas_Coord x1, y1, x2, y2;
        void *tmp = tr;
        /* We have 2 rectangles */
        Eina_Iterator *itr = it;
        fail_if (!eina_iterator_next(itr, &tmp));
        tr = tmp;
        x1 = tr->x;
        y1 = tr->y;
        fail_if (!eina_iterator_next(itr, &tmp));
        tr = tmp;
        x2 = tr->x;
        y2 = tr->y;

        /* These rectangles must be placed without overlapping.
         * In this test case, we expect to see a rect for each line. */
        fail_if((x1 == x2) && (y1 == y2));
        ck_assert_int_ne(y1, y2);
        eina_iterator_free(it);
     }

   /* Alignment fills */

   /* LTR text */
   evas_object_resize(tb, 400, 400);
   evas_object_textblock_text_markup_set(tb,
         "<align=0.1>"
         "Hello World<ps>"
         "How are you<ps>");
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_line_set(main_cur, 1);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 3);
   evas_textblock_cursor_char_next(main_cur);
   eina_iterator_free(it);

   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 4);
   evas_textblock_cursor_char_next(main_cur);
   eina_iterator_free(it);

   evas_textblock_cursor_line_set(main_cur, 2);
   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 4);
   evas_textblock_cursor_char_next(main_cur);
   eina_iterator_free(it);

   /* RTL text aligned to the left */
   evas_object_textblock_text_markup_set(tb,
         "<align=left>"
         "שלום עולם<ps>"
         "מה שלומך");
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_line_set(main_cur, 1);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 2);
   evas_textblock_cursor_char_next(main_cur);
   eina_iterator_free(it);

   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 3);
   evas_textblock_cursor_char_next(main_cur);
   eina_iterator_free(it);

   /* RTL text aligned to the middle */
   evas_object_textblock_text_markup_set(tb,
         "<align=middle>"
         "שלום עולם<ps>"
         "מה שלומך");
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_line_set(main_cur, 1);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 3);
   eina_iterator_free(it);

   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 4);
   eina_iterator_free(it);

   evas_object_textblock_text_markup_set(tb,
         "<align=middle>"
         "שלום עולם<ps>"
         "מה שלומך");
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_line_set(main_cur, 1);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 3);
   eina_iterator_free(it);

   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 4);
   eina_iterator_free(it);

   /* Auto align RTL and LTR */
   evas_object_textblock_text_markup_set(tb,
         "Hello world<ps>"
         "מה שלומך");
   evas_textblock_cursor_line_set(cur, 0);
   evas_textblock_cursor_line_set(main_cur, 1);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 2);
   eina_iterator_free(it);

   evas_textblock_cursor_char_next(main_cur);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);
   ck_assert_int_eq(eina_list_count(rects), 3);
   eina_iterator_free(it);

   /* Same run different scripts */
   evas_object_textblock_text_markup_set(tb, "עבריתenglishрусскийעברית");

   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 7);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);

   fail_if(eina_list_count(rects) != 2);

   eina_iterator_free(it);

   /* Same run different styles */
   evas_object_textblock_text_markup_set(tb, "test<b>test2</b>test3");

   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 11);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);

   fail_if(eina_list_count(rects) != 3);

   eina_iterator_free(it);

   /* Bidi text with a few back and forth from bidi. */
   evas_object_textblock_text_markup_set(tb, "נגכדגךלח eountoheunth ךלחגדךכלח");

   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_pos_set(main_cur, 28);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   fail_if(!it);
   rects = eina_iterator_container_get(it);
   fail_if(!rects);

   ck_assert_int_eq(eina_list_count(rects), 3);

   eina_iterator_free(it);

   /* Check trivial case with format items */
   evas_object_textblock_text_markup_set(tb, "abc<item size=32x32>efg");
   evas_textblock_cursor_pos_set(cur, 3);
   evas_textblock_cursor_pos_set(main_cur, 3);
   it = evas_textblock_cursor_range_simple_geometry_get(cur, main_cur);
   rects = eina_iterator_container_get(it);
   ck_assert(!rects);
   eina_iterator_free(it);

   END_TB_TEST();
}
EFL_END_TEST

/* Should handle all the text editing. */
EFL_START_TEST(evas_textblock_editing)
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

   /* Test cursor range delete with <br/> tags when legacy newline is enabled.
    * After deleting first <br/> tag, the second <br/> tag shouldn't be changed to <ps/> */
   evas_object_textblock_legacy_newline_set(tb, EINA_TRUE);
   evas_object_textblock_text_markup_set(tb, "A<br/><br/>B");
   evas_textblock_cursor_paragraph_first(cur);
   evas_textblock_cursor_paragraph_first(main_cur);
   evas_textblock_cursor_pos_set(main_cur, 2);
   ck_assert_str_eq(evas_textblock_cursor_range_text_get(cur, main_cur, EVAS_TEXTBLOCK_TEXT_MARKUP), "A<br/>");

   evas_textblock_cursor_range_delete(cur, main_cur);
   ck_assert_str_eq(evas_object_textblock_text_markup_get(tb), "<br/>B");

   /* Restore legacy newline disabled setting */
   evas_object_textblock_legacy_newline_set(tb, EINA_FALSE);

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
EFL_END_TEST

/* Text getters */
EFL_START_TEST(evas_textblock_text_getters)
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
   /* If one of the given cursor is NULL, it returns NULL. */
   fail_if(evas_textblock_cursor_range_text_get(NULL, NULL,
            EVAS_TEXTBLOCK_TEXT_MARKUP));
   fail_if(evas_textblock_cursor_range_text_get(cur, NULL,
            EVAS_TEXTBLOCK_TEXT_MARKUP));
   fail_if(evas_textblock_cursor_range_text_get(NULL, cur,
            EVAS_TEXTBLOCK_TEXT_MARKUP));

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
        fail_if(strcmp(tmp2, "a&nbsp;"));
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
        fail_if(strcmp(tmp2, "a&nbsp;"));
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

        tmp = evas_textblock_text_markup_to_utf8(NULL, "&lt;align=center&gt;hello&lt;/align&gt;&amp;gt;&quot;&apos;");
        fail_if(strcmp(tmp, "<align=center>hello</align>&gt;\"\'"));
        tmp2 = evas_textblock_text_utf8_to_markup(NULL, tmp);
        fail_if(strcmp(tmp2, "&lt;align=center&gt;hello&lt;/align&gt;&amp;gt;&quot;&apos;"));
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
EFL_END_TEST

/* Formats */
EFL_START_TEST(evas_textblock_formats)
{
   START_TB_TEST();
   const char *buf = "Th<b>i<font_size=15 wrap=none>s i</font_size=13>s</> a <br/> te<ps/>st<item></>.";
   const Evas_Object_Textblock_Node_Format *fnode;
   Evas_Coord w, h, nw, nh;
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
         "<underline=single underline_color=#F2D006 underline_height=5.5>underline=single underline_color=#F2D006 underline_height=5.5</><ps/>"
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

   /* Ligatures cut by formats */
   evas_object_textblock_text_markup_set(tb, "f<color=#f00>i</color>f");
   evas_object_textblock_size_formatted_get(tb, NULL, NULL);

   /* Scaling Line size */
   evas_object_scale_set(tb, 1.0);
   evas_object_textblock_text_markup_set(tb, "<linesize=100>Line size 100</linesize>");
   evas_object_resize(tb, 400, 400);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_ge(h, 100);

   evas_object_scale_set(tb, 2.0);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_ge(h, 200);

   /* Scaling Line gap */
   evas_object_scale_set(tb, 1.0);
   evas_object_textblock_text_markup_set(tb, "<linegap=100>Line gap 100</linegap>");
   evas_object_resize(tb, 50, 400);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_ge(h, 100);

   evas_object_scale_set(tb, 2.0);
   evas_object_textblock_size_formatted_get(tb, NULL, &h);
   ck_assert_int_ge(h, 200);

   /* Restore scale */
   evas_object_scale_set(tb, 1.0);

   /* Line gap and multi language */
   {
      Evas_Textblock_Style *newst;
      buf = "This is a test suite for line gap - ഈ ലൈൻ "
                  "വിടവ് ടെസ്റ്റ് ടെസ്റ്റ് ടെസ്റ്റ് ടെസ്റ്റ് ഒരു പരീക്ഷണ വെയര് ";
      newst = evas_textblock_style_new();
      fail_if(!newst);
      evas_textblock_style_set(newst,
            "DEFAULT='" TEST_FONT " font_size=10 color=#000 wrap=word linegap=50'"
            "br='\n'"
            "ps='ps'"
            "tab='\t'");
      evas_object_textblock_style_set(tb, newst);
      evas_object_textblock_text_markup_set(tb, buf);
      fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));
      evas_object_resize(tb, 400, 400);
      evas_object_textblock_size_formatted_get(tb, NULL, &h);
      fail_if(h > 150);
   }

   END_TB_TEST();
}
EFL_END_TEST

/* Different text styles, for example, shadow. */
EFL_START_TEST(evas_textblock_style)
{
   Evas_Coord w, h, nw, nh;
   Evas_Coord l, r, t, b;
   Evas_Coord bw;
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

   /* Style tag test */
   buf = "Test <br><br/><ps><ps/><tab><tab/>";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(buf, evas_object_textblock_text_markup_get(tb)));

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

   /* Size with style padding */
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);

   /* It is non-sense if the following condition is true. */
   fail_if((w + l + r == nw) && (h == nh + t + b));

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
      ck_assert_int_eq(w2[0], w2[1]);
      fail_if((x[0] != x[1]) || ((y[0] + h2[0]) != y[1]) || (w2[0] != w2[1]) || (h2[0] != h2[1]));

      evas_object_textblock_text_markup_set(tb, "Test<br/><style=glow>Test</><br/>Test");
      evas_object_textblock_style_insets_get(tb, &l, &r, &t, &b);
      evas_object_textblock_line_number_geometry_get(tb, 0, &x[2], &y[2], &w2[2], &h2[2]);
      evas_object_textblock_line_number_geometry_get(tb, 1, &x[3], &y[3], &w2[3], &h2[3]);
      evas_object_textblock_line_number_geometry_get(tb, 2, &x[4], &y[4], &w2[4], &h2[4]);

      // check line 1 geometry relatively to line 0
      fail_if((x[2] != x[3]) || ((y[2] + h2[2]) != y[3]) || (w2[2] > w2[3]) || (h2[2] != h2[3]));

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

   evas_textblock_style_set(newst,
         "DEFAULT='" TEST_FONT " font_size=10 color=#000"
         "  style=glow text_class=entry'");
   evas_object_textblock_style_set(tb, newst);
   evas_object_textblock_text_markup_set(tb, "Hello");
   evas_object_textblock_size_formatted_get(tb, &w, &h);

   evas_textblock_style_set(newst,
         "DEFAULT='" TEST_FONT " font_size=10 color=#000 text_class=entry'");
   evas_object_textblock_style_set(tb, newst);
   evas_object_textblock_text_markup_set(tb,
         "<style=glow>Hello");
   evas_object_textblock_size_formatted_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   // Ellipsis style padding
   // Should be consistent if style_pad is added
   evas_object_textblock_text_markup_set(tb, "hello");
   evas_object_textblock_size_native_get(tb, &w, &h);
   evas_object_resize(tb, w - 1, 200);
   evas_object_textblock_text_markup_set(tb,
         "<ellipsis=1.0>hello");
   evas_object_textblock_size_formatted_get(tb, &bw, NULL);
   evas_object_textblock_text_markup_set(tb,
         "<ellipsis=1.0 style=glow>hello</style>");
   evas_object_textblock_style_insets_get(tb, &l, &r, NULL, NULL);
   // Add padding to compensate for the style
   evas_object_resize(tb, w - 1 + l + r, 200);
   evas_object_textblock_size_formatted_get(tb, &nw, &nh);
   ck_assert_int_eq(nw, bw);

   evas_textblock_style_set(newst, "DEFAULT='font=Sans font_size=30'small_size='+ font_size=10'");
   evas_object_textblock_text_markup_set(tb, "Test <small_size>SMALL</small_size>");
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_gt(w, 0);
   ck_assert_int_gt(h, 0);

   evas_textblock_style_set(newst, "DEFAULT='font=Sans font_size=30'small_size='+font_size=50'");
   evas_object_textblock_size_formatted_get(tb, &nw, &nh);
   ck_assert_int_gt(nw, w);
   ck_assert_int_gt(nh, h);

   END_TB_TEST();
}
EFL_END_TEST

/* Textblock functionality without style. */
EFL_START_TEST(evas_textblock_style_empty)
{
   Evas *evas;
   Evas_Object *txt;
   Evas_Coord w, h;
   evas = EVAS_TEST_INIT_EVAS();
   txt = evas_object_textblock_add(evas);
   evas_object_textblock_text_markup_set(txt, "<b></b>");
   evas_object_textblock_size_formatted_get(txt, &w, &h);
   ck_assert_int_eq(w, 0);
   ck_assert_int_eq(h, 0);
   evas_free(evas);
}
EFL_END_TEST

/* Basic test for style user push/peek/pop. */
EFL_START_TEST(evas_textblock_style_user)
{
   Evas_Textblock_Style *user_st;

   START_TB_TEST();

   user_st = evas_textblock_style_new();
   fail_if(!user_st);
   evas_textblock_style_set(user_st, "DEFAULT='" TEST_FONT " font_size=50 color=#000'");

   evas_object_textblock_style_user_push(tb, user_st);
   fail_if(evas_object_textblock_style_user_peek(tb) != user_st);

   evas_object_textblock_style_user_pop(tb);
   fail_if(evas_object_textblock_style_user_peek(tb) != NULL);

   /* new/free should be handled from outside of Evas Textblock. */
   evas_textblock_style_free(user_st);

   END_TB_TEST();
}
EFL_END_TEST

/* Various setters and getters */
EFL_START_TEST(evas_textblock_set_get)
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
   fail_if(EINA_DBL_NONZERO(evas_object_textblock_valign_get(tb)));
   evas_object_textblock_valign_set(tb, 0.0);
   fail_if(EINA_DBL_NONZERO(evas_object_textblock_valign_get(tb)));
   evas_object_textblock_valign_set(tb, 0.432);
   fail_if(!EINA_DBL_EQ(evas_object_textblock_valign_get(tb), 0.432));
   evas_object_textblock_valign_set(tb, 1.0);
   fail_if(!EINA_DBL_EQ(evas_object_textblock_valign_get(tb), 1.0));
   evas_object_textblock_valign_set(tb, 1.5);
   fail_if(!EINA_DBL_EQ(evas_object_textblock_valign_get(tb), 1.0));

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
EFL_END_TEST

/* Aux evas stuff, such as scale. */
EFL_START_TEST(evas_textblock_evas)
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
EFL_END_TEST

/* All the string escaping stuff */
EFL_START_TEST(evas_textblock_escaping)
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

   buf = "This &nbsp; is";
   evas_object_textblock_text_markup_set(tb, buf);
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), buf));

   END_TB_TEST();
}
EFL_END_TEST

EFL_START_TEST(evas_textblock_size)
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

   evas_object_textblock_text_markup_set(tb, "/u200eאאא AAA");
   evas_object_resize(tb, 1, 1); //force wrapping
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, nw, nh);
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   /* Two lines != double the height */
   fail_if(h * 2 == h2);

   evas_object_textblock_text_markup_set(tb, buf);

   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   fail_if((w != nw) || (h != nh));
   fail_if(w <= 0);

   evas_object_textblock_text_markup_set(tb, "i<b>。</b>");
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

   evas_object_textblock_text_markup_set(tb, "。<b>i</b>");
   evas_object_textblock_size_formatted_get(tb, &w, &h);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(w, nw);
   ck_assert_int_eq(h, nh);

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

   evas_object_resize(tb, 1000, 1000);
   evas_object_textblock_text_markup_set(tb, "\u200fHello שלום");
   evas_object_textblock_size_formatted_get(tb, &w, NULL);
   evas_object_textblock_size_native_get(tb, &nw, NULL);
   ck_assert_int_eq(nw, w);

     {
        /* Check correct recalculation of sizes when some of the paragraphs
         * are not dirty */
        Evas_Coord bw, bh;
        evas_object_textblock_text_markup_set(tb,
              "XXXXXXXXXXXX<ps>"
              "YYY");
        evas_object_textblock_size_formatted_get(tb, &bw, &bh);
        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_text_append(cur, "Y");
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        ck_assert_int_eq(bw, w);

        evas_object_textblock_text_markup_set(tb,
              "XXXXXXXXXXXX<br>"
              "X<ps>"
              "YYY<br>");
        evas_object_textblock_size_formatted_get(tb, &bw, &bh);
        evas_textblock_cursor_paragraph_last(cur);
        evas_textblock_cursor_text_append(cur, "Y");
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        ck_assert_int_eq(bw, w);
     }

   /* FIXME: There is a lot more to be done. */
   END_TB_TEST();
}
EFL_END_TEST

EFL_START_TEST(evas_textblock_delete)
{
   START_TB_TEST();
   const Evas_Object_Textblock_Node_Format *fmt;

   /* The first and the second set of commands should result in the same
    * conditions for format and text nodes of the textblock object.
    * Essentially, it creates the markup 'a<ps>b' */
   evas_object_textblock_text_markup_set(tb, "ab");
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if(fmt);
   evas_textblock_cursor_pos_set(cur, 1);
   evas_object_textblock_text_markup_prepend(cur, "<ps/>");
   evas_textblock_cursor_pos_set(cur, 1);
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if (!fmt);
   evas_textblock_cursor_char_delete(cur);
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if(fmt);

   evas_object_textblock_text_markup_set(tb, "ab");
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if(fmt);
   evas_textblock_cursor_pos_set(cur, 1);
   evas_object_textblock_text_markup_prepend(cur, "<ps>");
   evas_textblock_cursor_pos_set(cur, 1);
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if (!fmt);
   evas_textblock_cursor_char_delete(cur);
   fmt =  evas_textblock_cursor_format_get(cur);
   fail_if(fmt);

   END_TB_TEST();
}
EFL_END_TEST;

/* Runs x,y in [from,to] range */
static void
_obstacle_run(Evas_Object *tb, Evas_Object *obj,
      Evas_Coord from_x, Evas_Coord to_x,
      Evas_Coord from_y, Evas_Coord to_y,
      Evas_Coord bh)
{
   Evas_Coord fw, fh;
   Evas_Coord x, y;
   for (y = from_y; y <= to_y; y += 5)
     {
        for (x = from_x; x <= to_x; x += 5)
          {
             evas_object_move(obj, x, y);
             evas_object_textblock_obstacles_update(tb);
             evas_object_textblock_size_formatted_get(tb, &fw, &fh);
             /* the obstacle size is large enough to assume that adding it
              * will at least make the formatted height value bigger */
             ck_assert_int_ge(fh, bh);
          }
     }
}

EFL_START_TEST(evas_textblock_obstacle)
{
   START_TB_TEST();
   Evas_Coord fw, fh;
   Evas_Object *rect, *rect2, *rect3;
   const char *buf =
      "This is an example text to demonstrate the textblock object"
      " with obstacle objects support."
      " Any evas object <item size=72x16></item>can register itself as an obstacle to the textblock"
      " object. Upon registering, it affects the layout of the text in"
      " certain situations. Usually, when the obstacle shows above the text"
      " area, it will cause the layout of the text to split and move"
      " parts of it, so that all text area is apparent.";

   rect = evas_object_rectangle_add(evas);
   rect2 = evas_object_rectangle_add(evas);
   rect3 = evas_object_rectangle_add(evas);
   evas_object_resize(rect, 50, 50);
   evas_object_resize(rect2, 50, 50);
   evas_object_resize(rect3, 50, 50);
   evas_object_resize(tb, 300, 400);

   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_format_prepend(cur, "<wrap=word>");
   evas_object_textblock_size_formatted_get(tb, &fw, &fh);

   ck_assert(!evas_object_textblock_obstacle_del(tb, rect));

   ck_assert(evas_object_textblock_obstacle_add(tb, rect));
   ck_assert(!evas_object_textblock_obstacle_add(tb, rect));

   ck_assert(evas_object_textblock_obstacle_add(tb, rect2));
   ck_assert(evas_object_textblock_obstacle_add(tb, rect3));

   evas_object_show(rect);
   evas_object_show(rect2);
   evas_object_show(rect3);

   /*check if first character after the obstacle*/
   Evas_Coord cx;
   evas_textblock_cursor_char_geometry_get(cur, &cx, NULL, NULL, NULL);
   ck_assert_int_eq(cx, 50);

   /* Compare formatted size with and without obstacle */
   _obstacle_run(tb, rect, 0, fw, fh / 2, fh / 2, fh);
   /* Now, with bigger obstacles */
   evas_object_resize(rect, 150, 150);
   evas_object_resize(rect3, 300, 300);
   evas_object_hide(rect);
   evas_object_textblock_obstacles_update(tb);
   _obstacle_run(tb, rect, 0, fw, fh / 2, fh / 2, fh);

   evas_object_textblock_obstacle_del(tb, rect);
   /* running with rect, now that it's not observed */
   evas_textblock_cursor_format_prepend(cur, "<wrap=mixed>");
   _obstacle_run(tb, rect, 0, fw, fh / 2, fh / 2, fh);

   evas_object_del(rect2);
   /* running with rect again, since rect2 is deleted */
   evas_textblock_cursor_format_prepend(cur, "<wrap=char>");
   _obstacle_run(tb, rect, 0, fw, fh / 2, fh / 2, fh);

   evas_object_del(rect);
   _obstacle_run(tb, rect3, 0, fw, 0, 0, fh);
   END_TB_TEST();
}
EFL_END_TEST;

EFL_START_TEST(evas_textblock_fit)
{
   START_TB_TEST();
   Evas_Coord fw, fh,fw_new, fh_new;
   int n_ret;
   const char *buf =
      "This is an example text to demonstrate the textblock object"
      " with content fit feature.";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_resize(tb, 300, 300);
   evas_object_textblock_size_formatted_get(tb, &fw, &fh);
   n_ret = evas_textblock_fit_options_set(tb,TEXTBLOCK_FIT_MODE_ALL);
   fail_if(n_ret != EVAS_ERROR_SUCCESS);
   n_ret = evas_textblock_fit_size_range_set(tb,1,50);
   fail_if(n_ret != EVAS_ERROR_SUCCESS);
   evas_object_textblock_size_formatted_get(tb, &fw_new, &fh_new);
   fail_if(fw_new == fw && fh_new == fh);
   unsigned int size_array[3] = {150,200,250};
   n_ret = evas_textblock_fit_size_array_set(tb,size_array,3);
   fail_if(n_ret != EVAS_ERROR_SUCCESS);
   evas_object_textblock_size_formatted_get(tb, &fw, &fh);
   fail_if(fw_new == fw && fh_new == fh);

   const char *buf1 = "<font_size=20>AAAA</font_size>";
   evas_object_textblock_text_markup_set(tb, buf1);
   evas_object_resize(tb, 300, 4);
   n_ret = evas_textblock_fit_options_set(tb,TEXTBLOCK_FIT_MODE_ALL);
   fail_if(n_ret != EVAS_ERROR_SUCCESS);
   n_ret = evas_textblock_fit_size_range_set(tb,1,255);
   fail_if(n_ret != EVAS_ERROR_SUCCESS);
   evas_object_textblock_size_formatted_get(tb, &fw, &fh);


   const char *buf2 = "AAAAA<font_size=20>AAAA</font_size>";
   evas_object_textblock_text_markup_set(tb, buf2);
   evas_object_resize(tb, 300, 4);
   evas_object_textblock_size_formatted_get(tb, &fw_new, &fh_new);
   fail_if(fw_new < (fw * 2));

   END_TB_TEST();
}
EFL_END_TEST;

EFL_START_TEST(evas_textblock_textrun_font)
{
   START_TB_TEST();
   int w1, h1, w2, h2;

   evas_object_resize(tb, 300, 300);
   evas_object_textblock_text_markup_set(tb, "가123A321");
   evas_object_textblock_size_native_get(tb, &w1, &h1);
   evas_object_textblock_text_markup_set(tb, "A321가123");
   evas_object_textblock_size_native_get(tb, &w2, &h2);
   ck_assert(w1==w2 && h1==h2);
   evas_object_textblock_text_markup_set(tb, "123가A321");
   evas_object_textblock_size_native_get(tb, &w2, &h2);
   ck_assert(w1==w2 && h1==h2);
   evas_object_textblock_text_markup_set(tb, "A가123321");
   evas_object_textblock_size_native_get(tb, &w2, &h2);
   ck_assert(w1==w2 && h1==h2);
   END_TB_TEST();
}
EFL_END_TEST;

#ifdef HAVE_HYPHEN
static void
_hyphenation_width_stress(Evas_Object *tb, Evas_Textblock_Cursor *cur)
{
   Evas_Coord bw, bh, iw, nw, nh, w, h;

   evas_object_resize(tb, 100000, 1000);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   evas_object_resize(tb, 1, 1000);
   evas_textblock_cursor_format_prepend(cur, "<wrap=mixed>");
   evas_object_textblock_size_formatted_get(tb, &bw, &bh);
   evas_textblock_cursor_format_prepend(cur, "<wrap=hyphenation>");
   for (iw = nw ; iw >= bw ; iw--)
     {
        evas_object_resize(tb, iw, 1000);
        evas_object_textblock_size_formatted_get(tb, &w, &h);
        ck_assert_int_ge(w, bw);
        ck_assert_int_le(w, iw);
     }
   ck_assert_int_eq(w, bw);
}

EFL_START_TEST(evas_textblock_hyphenation)
{
   START_TB_TEST();
   Evas_Coord w, fw;

   const char *buf = "Automati-";

   setenv("EVAS_DICTS_HYPHEN_DIR", TESTS_DIC_DIR, 1);

   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, NULL);
   evas_object_resize(tb, w, 100);

   setlocale(LC_MESSAGES, "en_US.UTF-8");
   /* Language should be reinitialized after calling setlocale(). */
   evas_language_reinit();

   buf = "Automatically";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_format_prepend(cur, "<wrap=hyphenation>");
   evas_object_textblock_size_formatted_get(tb, &fw, NULL);
   ck_assert_int_eq(w, fw);

   /* Restore locale */
   setlocale(LC_MESSAGES, "C");
   evas_language_reinit();

   /* SHY-HYPHEN (&shy;) */
   /* Note: placing &shy; in a ligature is errornuos, so for the sake
    * of this test, it was removed from the "officia" word */
   buf =
      "Lorem ipsum dolor sit amet, cons&shy;ectetur adipisicing elit,"
      " sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
      " Ut enim ad minim veniam, quis nostrud exer&shy;citation ullamco"
      " laboris nisi ut aliquip ex ea com&shy;modo consequat. Duis aute"
      " irure dolor in repre&shy;henderit in voluptate velit esse cillum"
      " dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
      " non proident, sunt in culpa qui oficia deserunt mollit anim"
      " id est lab&shy;orum.";

   evas_object_textblock_text_markup_set(tb, buf);

   /* Dictionary + locale fallback (en_US) */
   setlocale(LC_MESSAGES, "en_US.UTF-8");
   evas_language_reinit();

   /* Mixture of Dictionary with SHY-HYPHEN */
   _hyphenation_width_stress(tb, cur);

   /* Just dictionary */
   buf =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit,"
      " sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
      " Ut enim ad minim veniam, quis nostrud exercitation ullamco"
      " laboris nisi ut aliquip ex ea commodo consequat. Duis aute"
      " irure dolor in reprehenderit in voluptate velit esse cillum"
      " dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat"
      " non proident, sunt in culpa qui oficia deserunt mollit anim"
      " id est laborum.";

   evas_object_textblock_text_markup_set(tb, buf);
   _hyphenation_width_stress(tb, cur);

   buf = "europäi-";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_object_textblock_size_formatted_get(tb, &w, NULL);

   buf = "europäischen";
   evas_object_textblock_text_markup_set(tb, buf);
   evas_textblock_cursor_format_prepend(cur, "<wrap=hyphenation lang=de_DE>");
   evas_object_resize(tb, w, 100);
   evas_object_textblock_size_formatted_get(tb, &fw, NULL);
   ck_assert_int_eq(w, fw);

   unsetenv("EVAS_DICTS_HYPHEN_DIR");

   END_TB_TEST();
}
EFL_END_TEST;
#endif

EFL_START_TEST(evas_textblock_text_iface)
{
   START_TB_TEST();
   Evas_Coord nw, nh;
   Evas_Coord bw, bh;
   Evas_Coord w, h;
   const char *utf8;

   /* Set text */
   efl_text_set(tb, "hello world");
   evas_object_textblock_size_native_get(tb, &bw, &bh);
   efl_text_set(tb, "hello\nworld");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_gt(nh, bh);
   ck_assert_int_gt(bw, nw);
   efl_text_set(tb, "hello\nworld\ngoodbye\nworld");
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_ge(nh, bh * 3);

   /* Delete text */
   efl_text_set(tb, "a");
   evas_object_textblock_size_native_get(tb, &bw, &bh);
   efl_text_set(tb, "a\nb");
   evas_textblock_cursor_pos_set(cur, 1);
   evas_textblock_cursor_char_delete(cur);
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(nh, bh);

   /* Paragraph checks */
   efl_text_set(tb, "d");
   evas_object_textblock_size_native_get(tb, &w, &h);
   efl_text_set(tb, "aa\nb\nc\nd");
   evas_object_textblock_size_native_get(tb, &bw, &bh);
   evas_textblock_cursor_pos_set(cur, 0);
   evas_textblock_cursor_char_delete(cur); // delete 'a'
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(nh, bh);
   evas_textblock_cursor_char_delete(cur); // delete 'a'
   evas_textblock_cursor_char_delete(cur); // delete '\n'
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_lt(nh, bh);
   /* "b\nc\nd" is left */
   evas_textblock_cursor_char_delete(cur); // b
   evas_textblock_cursor_char_delete(cur); // \n
   evas_textblock_cursor_char_delete(cur); // c
   evas_textblock_cursor_char_delete(cur); // \n
   /* expecting "d" only */
   evas_object_textblock_size_native_get(tb, &nw, &nh);
   ck_assert_int_eq(nh, h);
   ck_assert_int_eq(nw, w);

   /* Text get */
   utf8 = "a";
   efl_text_set(tb, utf8);
   ck_assert_str_eq(utf8, efl_text_get(tb));
   utf8 = "a\nb";
   efl_text_set(tb, utf8);
   ck_assert_str_eq(utf8, efl_text_get(tb));
   utf8 = "a\u2029b";
   efl_text_set(tb, utf8);
   ck_assert_str_eq(utf8, efl_text_get(tb));
   utf8 = "a\u2029bc\ndef\n\u2029";
   efl_text_set(tb, utf8);
   ck_assert_str_eq(utf8, efl_text_get(tb));
   utf8 = "\u2029\n\n\n\n\u2029\n\u2029\n\n\n";
   efl_text_set(tb, utf8);
   ck_assert_str_eq(utf8, efl_text_get(tb));

   END_TB_TEST();
}
EFL_END_TEST;


#define _COMP_STR(...) ((const char *[]) { __VA_ARGS__ })
#define _CREATE_PARAMS(X) (sizeof(X) / sizeof(X[0])), (X)
#define _COMP_PARAMS(...) _CREATE_PARAMS(_COMP_STR(__VA_ARGS__))

EFL_START_TEST(evas_textblock_annotation)
{
   START_TB_TEST();
   Efl_Text_Cursor_Object *start, *end;

   start = efl_canvas_textblock_cursor_create(tb);
   end   = efl_canvas_textblock_cursor_create(tb);

   const char *buf =
      "This text will check annotation."
      " By \"annotating\" the text, we can apply formatting simply by"
      " specifying a range (start, end) in the text, and the format we want"
      " for it."
      ;

   efl_text_set(tb, buf);

   /* Check some trivial cases */

   efl_text_cursor_object_position_set(start, 0);
   efl_text_cursor_object_position_set(end, 3);
   efl_text_formatter_attribute_insert(start, end, "font_size=80");
   efl_text_cursor_object_position_set(start, 1);
   efl_text_cursor_object_position_set(end, 2);
   efl_text_formatter_attribute_insert(start, end, "font=arial");
   efl_text_cursor_object_position_set(start, 2);
   efl_text_cursor_object_position_set(end, 3);
   efl_text_formatter_attribute_insert(start, end, "color=#fff");

   efl_text_cursor_object_position_set(start, 0);
   efl_text_cursor_object_position_set(end, 3);
   unsigned int count = efl_text_formatter_attribute_clear(start, end);
   fail_if(count != 3);
}
EFL_END_TEST;

static const char *efl_style_buf =
   "" TEST_FONT " font_size=10 color=#000 text_class=entry";

#define START_EFL_CANVAS_TEXTBLOCK_TEST() \
   Evas *evas; \
   Eo *txt; \
   Efl_Text_Cursor_Handle *cur; \
   Efl_Text_Cursor_Object *cur_obj; \
   evas = EVAS_TEST_INIT_EVAS(); \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   txt = efl_add(EFL_CANVAS_TEXTBLOCK_CLASS, evas); \
   fail_if(!txt); \
   efl_canvas_textblock_newline_as_paragraph_separator_set(txt, EINA_FALSE); \
   efl_canvas_textblock_style_apply(txt, efl_style_buf); \
   fail_if(!efl_canvas_textblock_all_styles_get(txt)); \
   cur_obj = efl_canvas_textblock_cursor_create(txt);\
   cur = evas_object_textblock_cursor_new(txt); \
   fail_if(!cur); \
do \
{ \
} \
while (0)

#define END_EFL_CANVAS_TEXTBLOCK_TEST() \
do \
{ \
   efl_del(cur_obj); \
   evas_textblock_cursor_free(cur); \
   efl_del(txt); \
   evas_free(evas); \
} \
while (0)

EFL_START_TEST(evas_textblock_utf8_to_markup)
{
   size_t len = sizeof(escape_strings) / sizeof(Escape_Value);
   char * mkup_txt;
   for(size_t i = 0 ; i < len ; i++)
     {
       mkup_txt = evas_textblock_text_utf8_to_markup(NULL, escape_strings[i].value);
       fail_if(strcmp(escape_strings[i].escape, mkup_txt));
       free(mkup_txt);
     }
}
EFL_END_TEST

EFL_START_TEST(efl_canvas_textblock_simple)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();

   /* It is simple test for Efl_Canvas_Textblock.
    * The main object is "txt". */
   const char *buf = "Th<i>i</i>s is a <br/> te<b>s</b>t.";
   efl_text_set(txt, buf);
   fail_if(strcmp(efl_text_get(txt), buf));

   /* Check if password is false by default*/
   Eina_Bool password = efl_text_password_get(txt);
   fail_if(password);
   efl_text_password_set(txt, EINA_TRUE);
   password = efl_text_password_get(txt);
   fail_if(!password);
   efl_text_password_set(txt, EINA_FALSE);
   password = efl_text_password_get(txt);
   fail_if(password);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_text)
{
   START_TB_TEST();

#ifdef HAVE_FRIBIDI
   Evas_Coord x, x2;
   Evas_Coord nw, nh;
   Evas_Coord cx, cx2;
   Eina_Size2D size;

   /* Split cursor in LTR paragraph.
    * Russian 't' in the beginnning to create additional item.*/
                   /*01234    5 6789012345678  19  01234 */
   efl_text_set(tb, "тest \u202bנסיוןabcנסיון\u202c bang");
   size = efl_canvas_textblock_size_native_get(tb);
   nw = size.w;
   nh = size.h;

   efl_gfx_entity_size_set(tb, EINA_SIZE2D(nw,  nh));

   /* Logical cursor after "test " */
   evas_textblock_cursor_pos_set(cur, 6);
   fail_if(!evas_textblock_cursor_geometry_bidi_get(cur,
            &cx, NULL, NULL, NULL, &cx2,
            NULL, NULL, NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE));

   evas_textblock_cursor_pos_set(cur, 18);
   evas_textblock_cursor_pen_geometry_get(cur, &x, NULL, NULL, NULL);
   evas_textblock_cursor_pos_set(cur, 20);
   evas_textblock_cursor_pen_geometry_get(cur, &x2, NULL, NULL, NULL);
   ck_assert_int_eq(cx, x);
   ck_assert_int_eq(cx2, x2);
#endif

   END_TB_TEST();
}
EFL_END_TEST

static void
_increment_int_changed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   int *value = data;
   (*value)++;
}

EFL_START_TEST(efl_canvas_textblock_cursor)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();
   int pos;

   int changed_emit = 0;
   efl_event_callback_add(txt, EFL_CANVAS_TEXTBLOCK_EVENT_CHANGED, _increment_int_changed, &changed_emit);
   const char *buf = "abcdefghij";
   efl_text_set(txt, buf);
   ck_assert_int_eq(strcmp(efl_text_get(txt), buf), 0);

   pos = efl_text_cursor_object_position_get(cur_obj);
   ck_assert_int_eq(pos, 0);
   ck_assert(!efl_text_cursor_object_line_jump_by(cur_obj, -1));
   pos = efl_text_cursor_object_position_get(cur_obj);
   ck_assert_int_eq(pos, 0);
   ck_assert(efl_text_cursor_object_line_jump_by(cur_obj, 1));
   pos = efl_text_cursor_object_position_get(cur_obj);
   ck_assert_int_eq(pos, 10);

   efl_text_markup_set(txt, "Hello World<ps/>This is EFL<br/>Enlightenment");
   efl_text_cursor_object_position_set(cur_obj, 0);
   ck_assert_int_eq(efl_text_cursor_object_line_number_get(cur_obj), 0);
   ck_assert(efl_text_cursor_object_line_jump_by(cur_obj, 2));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 24);
   ck_assert_int_eq(efl_text_cursor_object_line_number_get(cur_obj), 2);
   ck_assert(efl_text_cursor_object_line_jump_by(cur_obj, -2));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   ck_assert_int_eq(efl_text_cursor_object_line_number_get(cur_obj), 0);

   ck_assert(efl_text_cursor_object_line_jump_by(cur_obj, 2));
   efl_text_cursor_object_line_number_set(cur_obj, 2);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 24);
   efl_text_cursor_object_line_number_set(cur_obj, 0);
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);

   Eo * cursor_temp = efl_duplicate(cur_obj);
   ck_assert_ptr_ne(cursor_temp, NULL);
   efl_del(cursor_temp);
   cursor_temp = NULL;

   Eo * cursor1 = efl_canvas_textblock_cursor_create(txt);
   efl_text_cursor_object_position_set(cursor1, 1);
   pos = efl_text_cursor_object_position_get(cursor1);
   ck_assert_int_eq(pos, 1);

   efl_text_set(txt, "");
   efl_text_set(txt, "");
   efl_text_cursor_object_text_insert(cursor1, "aa");
   ck_assert_int_eq(changed_emit, 4);

   efl_text_markup_set(txt, "Hello<br/>Word");
   efl_text_markup_set(txt, "Hello<br/>Word");
   efl_text_cursor_object_markup_insert(cursor1, "aa");
   ck_assert_int_eq(changed_emit, 6);

   efl_text_set(txt, "");
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_PREVIOUS));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_START));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_PREVIOUS));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST));
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LAST));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);

   ck_assert_int_eq(changed_emit, 7);

   efl_text_markup_set(txt, "Hello World<ps/>This is EFL<br/>Enlightenment");
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 1);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 1);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_PREVIOUS));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);

   efl_text_cursor_object_position_set(cur_obj, 0);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 4);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 4);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);

   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 5);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 10);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END));
   ck_assert_int_ne(efl_text_cursor_object_position_get(cur_obj), 10);

   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 23);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 23);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 12);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 12);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS));
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);

#if defined(HAVE_FRIBIDI) && defined(HAVE_HARFBUZZ)
   efl_text_markup_set(txt, "الْبَرْمَجةُ<ps/>مَرْحبَاً");
   efl_text_cursor_object_cluster_coord_set(cur_obj, EINA_POSITION2D(0, 0));
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 1);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 3);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 5);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 4);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 5);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 6);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_CLUSTER_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 7);

   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 13);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 13);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 22);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 22);
   ck_assert(!efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LAST));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 22);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_START));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 13);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_END));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 22);
   ck_assert(efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
#endif

   Eo *nCur = efl_canvas_textblock_cursor_create(txt);
   Eo *nCur2 = efl_canvas_textblock_cursor_create(txt);
   Eo *nCur3 = efl_canvas_textblock_cursor_create(txt);
   efl_text_markup_set(txt, "Hello World<ps/>This is EFL<br/>Enlightenment");
   efl_text_cursor_object_position_set(cur_obj, 0);
   ck_assert_ptr_ne(nCur, NULL);
   ck_assert_ptr_ne(nCur2, NULL);
   ck_assert_ptr_ne(nCur3, NULL);

   ck_assert(efl_text_cursor_object_equal(cur_obj, nCur));
   ck_assert(efl_text_cursor_object_equal(cur_obj, nCur2));
   ck_assert(efl_text_cursor_object_equal(cur_obj, nCur3));
   ck_assert(efl_text_cursor_object_equal(nCur2, nCur3));

   ck_assert_int_eq(efl_text_cursor_object_compare(cur_obj, nCur3), 0);
   ck_assert_int_eq(efl_text_cursor_object_compare(nCur2, nCur3), 0);
   ck_assert_int_eq(efl_text_cursor_object_compare(cur_obj, nCur), 0);

   ck_assert(efl_text_cursor_object_move(nCur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_NEXT));
   ck_assert_int_lt(efl_text_cursor_object_compare(cur_obj, nCur), 0);
   ck_assert_int_gt(efl_text_cursor_object_compare(nCur, cur_obj), 0);
   efl_text_cursor_object_position_set(nCur2, efl_text_cursor_object_position_get(nCur));
   ck_assert_int_lt(efl_text_cursor_object_compare(cur_obj, nCur2), 0);
   ck_assert_int_gt(efl_text_cursor_object_compare(nCur2, cur_obj), 0);
   ck_assert(!efl_text_cursor_object_equal(nCur2, nCur3));

   efl_text_set(txt, "");
   efl_text_cursor_object_text_insert(cur_obj, "Hello World");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 11);
   efl_text_cursor_object_text_insert(cur_obj, "Hello World");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 22);
   ck_assert_str_eq(efl_text_get(txt), "Hello WorldHello World");

   efl_text_set(txt, "");
   efl_text_cursor_object_markup_insert(cur_obj, "Hello World<ps/>Hello World");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 23);

   efl_text_cursor_object_char_coord_set(cur_obj, EINA_POSITION2D(0, 0));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   efl_text_cursor_object_char_coord_set(cur_obj, EINA_POSITION2D(500, 500));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 23);
   efl_text_cursor_object_cluster_coord_set(cur_obj, EINA_POSITION2D(0, 0));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   efl_text_cursor_object_cluster_coord_set(cur_obj, EINA_POSITION2D(500, 500));
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 23);

   efl_text_cursor_object_position_set(nCur, 0);
   efl_text_cursor_object_position_set(cur_obj, 5);

   ck_assert_str_eq(efl_text_cursor_object_range_text_get(cur_obj, nCur), "Hello");
   ck_assert_str_eq(efl_text_cursor_object_range_text_get(nCur, cur_obj), "Hello");

   efl_text_cursor_object_position_set(nCur, 0);
   efl_text_cursor_object_position_set(cur_obj, 17);
   
   Eina_Iterator *iter = efl_text_cursor_object_range_geometry_get(cur_obj, nCur);
   Eina_Rect *geoRect;
   int x = 0;

   EINA_ITERATOR_FOREACH(iter, geoRect)
     {
        x ++;
        ck_assert_int_ne(geoRect->w, 0);
        ck_assert_int_ne(geoRect->h, 0);
     }

   ck_assert_int_eq(x, 3);

   iter = efl_text_cursor_object_range_precise_geometry_get(cur_obj, nCur);
   x = 0;

   EINA_ITERATOR_FOREACH(iter, geoRect)
     {
        x ++;
        ck_assert_int_ne(geoRect->w, 0);
        ck_assert_int_ne(geoRect->h, 0);
     }

   ck_assert_int_eq(x, 2);

   ck_assert_str_eq(efl_text_cursor_object_range_markup_get(cur_obj, nCur), "Hello World<ps/>Hello");
   ck_assert_str_eq(efl_text_cursor_object_range_markup_get(nCur, cur_obj), "Hello World<ps/>Hello");

   efl_text_cursor_object_position_set(nCur, 0);
   efl_text_cursor_object_position_set(cur_obj, 5);

   efl_text_cursor_object_range_delete(nCur, cur_obj);

   ck_assert_str_eq(efl_text_markup_get(txt), " World<ps/>Hello World");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur_obj), 0);
   ck_assert_int_eq(efl_text_cursor_object_position_get(nCur), 0);

   efl_text_markup_set(txt, "Hello World<ps/>Hello World");
   efl_text_cursor_object_position_set(cur_obj, 6);
   Eina_Unicode str[2] = {0};
   str[0] = efl_text_cursor_object_content_get(cur_obj);
   ck_assert_str_eq(eina_unicode_unicode_to_utf8(str, NULL), "W");

   efl_text_cursor_object_char_delete(cur_obj);

   str[0] = efl_text_cursor_object_content_get(cur_obj);
   ck_assert_str_eq(eina_unicode_unicode_to_utf8(str, NULL), "o");

   Eo *cur_txt = efl_text_cursor_object_text_object_get(cur_obj);
   Eo *cur_txt2 = efl_text_cursor_object_text_object_get(nCur);
   ck_assert_ptr_eq(cur_txt, txt);
   ck_assert_ptr_eq(cur_txt2, txt);

   efl_text_cursor_object_position_set(cur_obj, 1);
   Eina_Rect rect = efl_text_cursor_object_content_geometry_get(cur_obj);
   ck_assert_int_ne(rect.w, 0);
   ck_assert_int_ne(rect.h, 0);
   ck_assert_int_ne(rect.x, 0);

   rect = efl_text_cursor_object_cursor_geometry_get(cur_obj, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert_int_eq(rect.w, 0);
   ck_assert_int_ne(rect.h, 0);
   ck_assert_int_ne(rect.x, 0);

   Eina_Rect rect2 = efl_text_cursor_object_cursor_geometry_get(cur_obj, EFL_TEXT_CURSOR_TYPE_UNDER);
   ck_assert_int_ne(rect2.w, 0);
   ck_assert_int_ne(rect2.h, 0);
   ck_assert_int_ne(rect2.x, 0);

   ck_assert_int_ne(rect2.w, rect.w);
   ck_assert_int_eq(rect2.h, rect.h);
   ck_assert_int_eq(rect2.x, rect.x);
   ck_assert_int_eq(rect2.y, rect.y);

   efl_text_markup_set(txt, "Hello World");
   efl_text_cursor_object_position_set(cur_obj, 11);
   ck_assert(!efl_text_cursor_object_lower_cursor_geometry_get(cur_obj, &rect2));
#ifdef HAVE_FRIBIDI
   efl_text_cursor_object_text_insert(cur_obj, "مرحباً");
   rect = efl_text_cursor_object_cursor_geometry_get(cur_obj, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ck_assert(efl_text_cursor_object_lower_cursor_geometry_get(cur_obj, &rect2));
   ck_assert_int_eq(rect2.w, 0);
   ck_assert_int_ne(rect2.h, 0);
   ck_assert_int_ne(rect2.x, 0);

   ck_assert_int_eq(rect2.w, rect.w);
   ck_assert_int_eq(rect2.h, rect.h);
   ck_assert_int_ne(rect2.x, rect.x);
   ck_assert_int_eq(rect2.y, rect.y);
#endif

   //Efl able to deal with br tab without closing tag "/"
   efl_text_markup_set(txt, "a<br>a<tab>a");
   efl_text_cursor_object_move(nCur, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   efl_text_cursor_object_move(cur_obj, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   efl_text_cursor_object_range_delete(nCur, cur_obj);
   ck_assert_str_eq(efl_text_markup_get(txt), "");

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_canvas_textblock_cursor_change)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();
   (void) cur_obj;
   int changed_emit1 = 0;
   int changed_emit2 = 0;
   Efl_Object *cur1, *cur2;
   cur1 = efl_canvas_textblock_cursor_create(txt);
   cur2 = efl_canvas_textblock_cursor_create(txt);
   efl_text_set(txt, "Hello World");
   efl_text_cursor_object_position_set(cur1, 0);
   efl_text_cursor_object_position_set(cur2, 1);
   efl_event_callback_add(cur1, EFL_TEXT_CURSOR_OBJECT_EVENT_CHANGED, _increment_int_changed, &changed_emit1);
   efl_event_callback_add(cur2, EFL_TEXT_CURSOR_OBJECT_EVENT_CHANGED, _increment_int_changed, &changed_emit2);
   efl_text_set(txt, "");
   ck_assert_int_eq(changed_emit1, 0);
   ck_assert_int_eq(changed_emit2, 1);
}
EFL_END_TEST


EFL_START_TEST(efl_canvas_textblock_markup)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();
   Efl_Text_Cursor_Object *start, *end;
   char *res;

   start = efl_canvas_textblock_cursor_create(txt);
   end = efl_canvas_textblock_cursor_create(txt);

   efl_text_set(txt, "\n\n\n");

   efl_text_cursor_object_position_set(start, 1);
   efl_text_cursor_object_position_set(end, 2);
   res = efl_text_cursor_object_range_markup_get(start, end);
   ck_assert_str_eq(res, "<br>");
   free(res);

   efl_text_set(txt, "a\u2029bc\ndef\n\u2029");
   efl_text_cursor_object_position_set(start, 2);
   efl_text_cursor_object_position_set(end, 5);
   res = efl_text_cursor_object_range_markup_get(start, end);
   ck_assert_str_eq(res, "bc<br>");
   free(res);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_canvas_textblock_markup_invalid_escape)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();

   char * text1 = "Hello";
   char * text2 = "Hello&123";
   char * text3 = "Hello&123&456";
   Eina_Size2D fw1, fw2, fw3;

   efl_text_markup_set(txt,text1);
   fw1 = efl_canvas_textblock_size_native_get(txt);
   efl_text_markup_set(txt,text2);
   fw2 = efl_canvas_textblock_size_native_get(txt);
   fail_if(fw2.w <= fw1.w);
   efl_text_markup_set(txt,text3);
   fw3 = efl_canvas_textblock_size_native_get(txt);
   fail_if(fw3.w <= fw2.w);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST


EFL_START_TEST(efl_text_font)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();

   efl_text_set(txt, "\n\n\n");

   const char * font;
   int font_size;
   efl_text_font_family_set(txt, "Sans");
   efl_text_font_size_set(txt, 20);
   efl_text_font_family_set(txt, NULL);
   efl_text_font_size_set(txt, 0);

   font = efl_text_font_family_get(txt);
   font_size = efl_text_font_size_get(txt);
   fail_if(20 != font_size);
   fail_if(strcmp(font,"Sans"));

   efl_text_font_family_set(txt, NULL);
   efl_text_font_size_set(txt, 30);
   font = efl_text_font_family_get(txt);
   font_size = efl_text_font_size_get(txt);
   fail_if(30 != font_size);
   fail_if(strcmp(font,"Sans"));

   efl_text_font_family_set(txt, "arial");
   efl_text_font_size_set(txt, 0);
   font = efl_text_font_family_get(txt);
   font_size = efl_text_font_size_get(txt);
   fail_if(30 != font_size);
   fail_if(strcmp(font,"arial"));

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_canvas_textblock_style)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();
   unsigned char r, g, b, a;
   const char *style;
   Eina_Size2D size1, size2;

   int changed_emit = 0;
   efl_event_callback_add(txt, EFL_CANVAS_TEXTBLOCK_EVENT_CHANGED, _increment_int_changed, &changed_emit);

   efl_text_password_set(txt, EINA_FALSE);
   efl_text_underline_type_set(txt, EFL_TEXT_STYLE_UNDERLINE_TYPE_DOUBLE);
   efl_text_font_weight_set(txt, EFL_TEXT_FONT_WEIGHT_EXTRABOLD);
   efl_text_font_slant_set(txt, EFL_TEXT_FONT_SLANT_OBLIQUE);
   efl_text_tab_stops_set(txt, 20);

   ck_assert_int_eq(changed_emit, 4);

   efl_canvas_textblock_style_apply(txt, "color=#90E135");

   style = efl_canvas_textblock_all_styles_get(txt);

   // from efl_style_buf
   fail_if(!strstr(style, "font=DejaVuSans,UnDotum,malayalam"));
   // default value
   fail_if(!strstr(style, "font_width=normal"));
   fail_if(!strstr(style, "wrap=none"));

   // from functions
   fail_if(!strstr(style, "font_weight=extrabold"));
   fail_if(!strstr(style, "tab_stops=20"));
   fail_if(!strstr(style, "color=rgba(144,225,53,255)"));
   fail_if(!strstr(style, "password=off"));
   efl_text_password_set(txt, EINA_TRUE);
   style = efl_canvas_textblock_all_styles_get(txt);
   fail_if(!strstr(style, "password=on"));

   changed_emit = 0;
   efl_canvas_textblock_style_apply(txt, "font_width=ultracondensed");
   ck_assert_int_eq(efl_text_font_width_get(txt), EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);

   efl_canvas_textblock_style_apply(txt, "wrap=word");
   ck_assert_int_eq(efl_text_wrap_get(txt), EFL_TEXT_FORMAT_WRAP_WORD);

   efl_canvas_textblock_style_apply(txt, "wrap=none");
   ck_assert_int_eq(efl_text_wrap_get(txt), EFL_TEXT_FORMAT_WRAP_NONE);

   efl_canvas_textblock_style_apply(txt, "background_type=solid");
   ck_assert_int_eq(efl_text_background_type_get(txt), EFL_TEXT_STYLE_BACKGROUND_TYPE_SOLID_COLOR);

   efl_canvas_textblock_style_apply(txt, "background_color=red");
   efl_text_background_color_get(txt, &r, &g, &b, &a);
   ck_assert_int_eq(r, 0xFF);
   ck_assert_int_eq(g, 0x00);
   ck_assert_int_eq(b, 0x00);
   ck_assert_int_eq(a, 0xFF);

   efl_canvas_textblock_style_apply(txt, "effect_type=far_soft_shadow");
   ck_assert_int_eq(efl_text_effect_type_get(txt), EFL_TEXT_STYLE_EFFECT_TYPE_FAR_SOFT_SHADOW);

   efl_canvas_textblock_style_apply(txt, "shadow_direction=top_right");
   ck_assert_int_eq(efl_text_shadow_direction_get(txt), EFL_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT);

   efl_canvas_textblock_style_apply(txt, "shadow_direction=top");
   ck_assert_int_eq(efl_text_shadow_direction_get(txt),  EFL_TEXT_STYLE_SHADOW_DIRECTION_TOP);

   efl_canvas_textblock_style_apply(txt, "effect_type=soft_outline");
   ck_assert_int_eq(efl_text_effect_type_get(txt), EFL_TEXT_STYLE_EFFECT_TYPE_SOFT_OUTLINE);

   efl_canvas_textblock_style_apply(txt, "underline_type=none");
   ck_assert_int_eq(efl_text_underline_type_get(txt),  EFL_TEXT_STYLE_UNDERLINE_TYPE_NONE);

   efl_canvas_textblock_style_apply(txt, "strikethrough_type=single");
   ck_assert_int_eq(efl_text_strikethrough_type_get(txt),  EFL_TEXT_STYLE_STRIKETHROUGH_TYPE_SINGLE);

   efl_canvas_textblock_style_apply(txt, "color=#EF596C");
   efl_text_color_get(txt, &r, &g, &b, &a);
   ck_assert_int_eq(r, 0xEF);
   ck_assert_int_eq(g, 0x59);
   ck_assert_int_eq(b, 0x6C);
   ck_assert_int_eq(a, 0xFF);

   ck_assert_int_eq(changed_emit, 12);

   //check if multiple attribute set, called only once
   changed_emit = 0;
   efl_canvas_textblock_style_apply(txt, style);
   ck_assert_int_eq(changed_emit, 1);

   // Style Apply taking 
   efl_text_set(txt,"A");
   efl_canvas_textblock_style_apply(txt,"font_size=2");
   size1 = efl_canvas_textblock_size_native_get(txt);
   efl_canvas_textblock_style_apply(txt,"font_size=20");
   size2 = efl_canvas_textblock_size_native_get(txt);
   ck_assert(size1.w < size2.w);
   ck_assert(size1.h < size2.h);

   efl_text_gfx_filter_set(txt, "code");
   ck_assert_str_eq(efl_text_gfx_filter_get(txt), "code");

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_text_style)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();
   Eina_Size2D size1, size2;

   int changed_emit = 0;
   efl_event_callback_add(txt, EFL_CANVAS_TEXTBLOCK_EVENT_CHANGED, _increment_int_changed, &changed_emit);
   efl_text_set(txt, "Hello");
   ck_assert_int_eq(changed_emit, 1);
   efl_text_set(txt, "");
   ck_assert_int_eq(changed_emit, 2);
   efl_text_set(txt, "");
   ck_assert_int_eq(changed_emit, 2);

   changed_emit  = 0;
   efl_text_markup_set(txt, "&quot;Hello&quot;");
   ck_assert_int_eq(changed_emit, 1);
   efl_text_markup_set(txt, "");
   ck_assert_int_eq(changed_emit, 2);
   efl_text_markup_set(txt, "");
   ck_assert_int_eq(changed_emit, 2);

   // Style Applying
   efl_text_set(txt,"A");
   efl_canvas_textblock_style_apply(txt,"\tfont_size=2\t");
   size1 = efl_canvas_textblock_size_native_get(txt);
   efl_canvas_textblock_style_apply(txt,"\nfont_size=20\n");
   size2 = efl_canvas_textblock_size_native_get(txt);
   ck_assert(size1.w < size2.w);
   ck_assert(size1.h < size2.h);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_text_markup)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();

   efl_text_underline_type_set(txt, EFL_TEXT_STYLE_UNDERLINE_TYPE_NONE);
   ck_assert_int_eq(efl_text_underline_type_get(txt), EFL_TEXT_STYLE_UNDERLINE_TYPE_NONE);
   efl_text_underline_type_set(txt, EFL_TEXT_STYLE_UNDERLINE_TYPE_SINGLE);
   ck_assert_int_eq(efl_text_underline_type_get(txt), EFL_TEXT_STYLE_UNDERLINE_TYPE_SINGLE);
   efl_text_underline_type_set(txt, EFL_TEXT_STYLE_UNDERLINE_TYPE_DOUBLE);
   ck_assert_int_eq(efl_text_underline_type_get(txt), EFL_TEXT_STYLE_UNDERLINE_TYPE_DOUBLE);
   efl_text_underline_type_set(txt, EFL_TEXT_STYLE_UNDERLINE_TYPE_DASHED);
   ck_assert_int_eq(efl_text_underline_type_get(txt), EFL_TEXT_STYLE_UNDERLINE_TYPE_DASHED);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_text_font_source)
{
   START_EFL_CANVAS_TEXTBLOCK_TEST();

   Eina_Size2D size1, size2;

   efl_canvas_textblock_style_apply(txt,"\tfont_size=30\t");
   efl_text_markup_set(txt, "Hello, This Text Use The Font : Does_Not_Exists_Font");

   efl_text_font_family_set(txt, "Does_Not_Exists_Font_1");
   size1 = efl_canvas_textblock_size_native_get(txt);

   efl_text_font_source_set(txt, TEST_FONT_SOURCE);
   efl_text_font_family_set(txt, "Does_Not_Exists_Font_1");
   size2 = efl_canvas_textblock_size_native_get(txt);
   ck_assert_int_ne(size1.w, size2.w);

   efl_text_font_source_set(txt, "");
   efl_text_font_family_set(txt, "Does_Not_Exists_Font_2");
   size1 = efl_canvas_textblock_size_native_get(txt);

   efl_text_font_source_set(txt, TEST_FONT_SOURCE);
   efl_text_font_family_set(txt, "Does_Not_Exists_Font_2");
   size2 = efl_canvas_textblock_size_native_get(txt);
   ck_assert_int_ne(size1.w, size2.w);

   END_EFL_CANVAS_TEXTBLOCK_TEST();
}
EFL_END_TEST

EFL_START_TEST(efl_text_default_format)
{
   Evas *evas;
   Eo *txt;
   evas = EVAS_TEST_INIT_EVAS();
   txt = efl_add(EFL_CANVAS_TEXTBLOCK_CLASS, evas);

   Eina_Size2D size;

   efl_text_markup_set(txt, "<font=Sans>Hello</font>");

   efl_text_font_size_set(txt, 80);
   efl_text_color_set(txt, 255, 255, 255, 255);
   size = efl_canvas_textblock_size_native_get(txt);

   ck_assert_int_gt(size.h, 20);
   efl_del(txt);
   evas_free(evas);
}
EFL_END_TEST

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
   tcase_add_test(tc, evas_textblock_style_empty);
   tcase_add_test(tc, evas_textblock_style_user);
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
   tcase_add_test(tc, evas_textblock_delete);
   tcase_add_test(tc, evas_textblock_obstacle);
   tcase_add_test(tc, evas_textblock_fit);
   tcase_add_test(tc, evas_textblock_textrun_font);
#ifdef HAVE_HYPHEN
   tcase_add_test(tc, evas_textblock_hyphenation);
#endif
   tcase_add_test(tc, evas_textblock_text_iface);
   tcase_add_test(tc, evas_textblock_annotation);
   tcase_add_test(tc, evas_textblock_utf8_to_markup);
   tcase_add_test(tc, efl_canvas_textblock_simple);
   tcase_add_test(tc, efl_text);
   tcase_add_test(tc, efl_canvas_textblock_cursor);
   tcase_add_test(tc, efl_canvas_textblock_cursor_change);
   tcase_add_test(tc, efl_canvas_textblock_markup);
   tcase_add_test(tc, efl_canvas_textblock_markup_invalid_escape);
   tcase_add_test(tc, efl_text_font);
   tcase_add_test(tc, efl_canvas_textblock_style);
   tcase_add_test(tc, efl_text_style);
   tcase_add_test(tc, efl_text_markup);
   tcase_add_test(tc, efl_text_font_source);
   tcase_add_test(tc, efl_text_default_format);
}

