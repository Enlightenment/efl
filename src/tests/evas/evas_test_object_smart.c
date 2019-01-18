/*
 * TODO:
 * * Test different font lodaing mechanisms.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>
#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

#define TEST_FONT_SOURCE TESTS_SRC_DIR "/fonts/TestFont.eet"
#define TEST_TEXTBLOCK_FONT "font=DejaVuSans font_source=" TEST_FONT_SOURCE
#define TEST_TEXTBLOCK_FONT_SIZE "14"
#define TEST_TEXT_FONT "DejaVuSans,UnDotum"
#define TEST_TEXT_FONT_SIZE 14

static const char *style_buf =
   "DEFAULT='" TEST_TEXTBLOCK_FONT " font_size="TEST_TEXTBLOCK_FONT_SIZE" color=#000 text_class=entry'"
   "newline='br'"
   "b='+ font_weight=bold'";

#define START_EVAS_OBJECT_SMART_TEST() \
   Evas *evas; \
   Evas_Object *smart_obj; \
   evas = EVAS_TEST_INIT_EVAS(); \
   smart_obj = evas_object_box_add(evas); \
   fail_if(!smart_obj); \
do \
{ \
} \
while (0)

#define END_EVAS_OBJECT_SMART_TEST() \
do \
{ \
   evas_object_del(smart_obj); \
   evas_free(evas); \
} \
while (0)

#define ADD_EVAS_TEXTBLOCK() \
   Evas_Object *tb; \
   Evas_Textblock_Style *st; \
   Evas_Textblock_Cursor *cur; \
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

#define ADD_EVAS_TEXT() \
   Evas_Object *to; \
   evas_font_hinting_set(evas, EVAS_FONT_HINTING_AUTO); \
   to = evas_object_text_add(evas); \
   fail_if(!to); \
   evas_object_text_font_source_set(to, TEST_FONT_SOURCE); \
   evas_object_text_font_set(to, TEST_TEXT_FONT, TEST_TEXT_FONT_SIZE); \
do \
{ \
} \
while (0)

#define DELETE_EVAS_TEXTBLOCK() \
do \
{ \
   evas_textblock_cursor_free(cur); \
   evas_object_del(tb); \
   evas_textblock_style_free(st); \
} \
while (0)

#define DELETE_EVAS_TEXT() \
do \
{ \
   evas_object_del(to); \
} \
while (0)

EFL_START_TEST(evas_object_smart_paragraph_direction)
{
   START_EVAS_OBJECT_SMART_TEST();

   ADD_EVAS_TEXTBLOCK();
   evas_object_resize(tb, 500, 500);
   evas_object_textblock_text_markup_set(tb, "%^&amp;");
   fail_if(strcmp(evas_object_textblock_text_markup_get(tb), "%^&amp;"));
   evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_INHERIT);

   ADD_EVAS_TEXT();
   evas_object_text_text_set(to, "%^&amp;");
   fail_if(strcmp(evas_object_text_text_get(to), "%^&amp;"));
   evas_object_paragraph_direction_set(to, EVAS_BIDI_DIRECTION_INHERIT);

   /* Test evas_object_paragraph_direction_set API with smart objects. */
   Evas_Object *smart_child = evas_object_box_add(evas);
   Evas_BiDi_Direction dir;
   Evas_Coord x, y, w, h;
   Evas_Coord xx, yy, ww, hh;

   evas_object_smart_member_add(smart_child, smart_obj);
   evas_object_smart_member_add(tb, smart_child);
   evas_object_smart_member_add(to, smart_child);

   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
   fail_if(evas_object_text_direction_get(to) == EVAS_BIDI_DIRECTION_RTL);

   /* Change paragraph direction of smart parent object */
   evas_object_paragraph_direction_set(smart_obj, EVAS_BIDI_DIRECTION_RTL);
   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);

   /* The paragraph direction of smart member object has to be reset
      if smart member object is removed from smart parent. */
   evas_object_smart_member_del(smart_child);

   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
   fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
   fail_if(evas_object_text_direction_get(to) == EVAS_BIDI_DIRECTION_RTL);

   /* The paragraph direction of smart member object has to be changed
      if smart member object is appended to smart parent. */
   evas_object_smart_member_add(smart_child, smart_obj);

   dir = EVAS_BIDI_DIRECTION_LTR;
   evas_textblock_cursor_geometry_get(cur, &xx, &yy, &ww, &hh, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir != EVAS_BIDI_DIRECTION_RTL);
   fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
   fail_if(evas_object_text_direction_get(to) != EVAS_BIDI_DIRECTION_RTL);

   /* Ignore smart parent's paragraph direction */
   evas_object_paragraph_direction_set(tb, EVAS_BIDI_DIRECTION_NEUTRAL);
   evas_object_paragraph_direction_set(to, EVAS_BIDI_DIRECTION_NEUTRAL);
   dir = EVAS_BIDI_DIRECTION_RTL;
   evas_textblock_cursor_geometry_get(cur, &x, &y, &w, &h, &dir,
                                      EVAS_TEXTBLOCK_CURSOR_UNDER);
   fail_if(dir == EVAS_BIDI_DIRECTION_RTL);
   fail_if((x >= xx) || (y != yy) || (w != ww) || (h != hh));
   fail_if(evas_object_text_direction_get(to) == EVAS_BIDI_DIRECTION_RTL);

   evas_object_smart_member_del(tb);
   evas_object_smart_member_del(to);
   evas_object_del(smart_child);

   DELETE_EVAS_TEXTBLOCK();
   DELETE_EVAS_TEXT();
   END_EVAS_OBJECT_SMART_TEST();
}
EFL_END_TEST


void
_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   ck_abort_msg("This function should be not called");
}

EFL_START_TEST(evas_object_smart_clipped_smart_move)
{
   Evas *evas;
   Evas_Smart *smart;
   Evas_Object *smart_obj, *smart_child;

   evas = _setup_evas();

   Evas_Smart_Class sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("MyClass");
   evas_object_smart_clipped_smart_set(&sc);
   sc.move = NULL;

   smart = evas_smart_class_new(&sc);
   fail_if(!smart);

   smart_obj = evas_object_smart_add(evas, smart);
   fail_if(!smart_obj);

   smart_child = evas_object_box_add(evas);
   evas_object_smart_member_add(smart_child, smart_obj);

   evas_object_event_callback_add(smart_child, EVAS_CALLBACK_MOVE, _move_cb, NULL);
   evas_object_move(smart_obj, 100, 100);

   evas_object_smart_member_del(smart_child);
   evas_object_del(smart_child);
   evas_object_del(smart_obj);
   evas_free(evas);
}
EFL_END_TEST

void evas_test_object_smart(TCase *tc)
{
   tcase_add_test(tc, evas_object_smart_paragraph_direction);
   tcase_add_test(tc, evas_object_smart_clipped_smart_move);
}
