#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#define EFL_GFX_FILTER_BETA
#define EFL_CANVAS_LAYOUT_BETA

#include "edje_suite.h"

#define EVAS_DATA_DIR TESTS_SRC_DIR "/../../lib/evas"


EFL_START_TEST(edje_test_text_cursor)
{
   Evas *evas;
   Evas_Object *obj;
   const char *buf = "ABC<br/>DEF";
   const char *txt;
   int i, old_pos, new_pos;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_text_cursor.edj"), "test_text_cursor"));
   edje_object_part_text_set(obj, "text", buf);
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, buf));

   edje_object_part_text_cursor_pos_set(obj, "text", EDJE_CURSOR_MAIN, 0);
   ck_assert_int_eq(edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN), 0);
   edje_object_part_text_cursor_pos_set(obj, "text", EDJE_CURSOR_MAIN, 1);
   ck_assert_int_eq(edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN), 1);

   /* Move cursor to the 0 pos from 1 pos */
   old_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert(edje_object_part_text_cursor_prev(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Move cursor to the -1 pos from 0 pos. It has to fail. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_prev(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Jump to 2nd line from 1st line.
    * It has to return EINA_TRUE which means success. */
   old_pos = new_pos;
   ck_assert(edje_object_part_text_cursor_down(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Try to jump to 3rd line from 2nd line. But, 3rd line does not exist.
    * So, it has to return EINA_FALSE which means failure. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_down(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Move cursor to the end of 2nd line. */
   for (i = 0; i < 3; i++)
     {
        old_pos = new_pos;
        ck_assert(edje_object_part_text_cursor_next(obj, "text", EDJE_CURSOR_MAIN));
        new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
        ck_assert_int_ne(old_pos, new_pos);
     }

   /* Move cursor to the next of the end of 2nd line which does not exist. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_next(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   /* Jump to 1st line from 2nd line.
    * It has to return EINA_TRUE which means success. */
   old_pos = new_pos;
   ck_assert(edje_object_part_text_cursor_up(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_ne(old_pos, new_pos);

   /* Try to jump to the above of 1st line from 1st line. But, there is no such line.
    * So, it has to return EINA_FALSE which means failure. */
   old_pos = new_pos;
   ck_assert(!edje_object_part_text_cursor_up(obj, "text", EDJE_CURSOR_MAIN));
   new_pos = edje_object_part_text_cursor_pos_get(obj, "text", EDJE_CURSOR_MAIN);
   ck_assert_int_eq(old_pos, new_pos);

   evas_free(evas);
}
EFL_END_TEST


EFL_START_TEST(edje_test_textblock)
{
   Evas *evas;
   Evas_Object *obj;
   const char *buf = "Hello";
   const char *txt;

   evas = _setup_evas();

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_textblock.edj"), "test_textblock"));
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, "Bye"));
   edje_object_part_text_set(obj, "text", buf);
   txt = edje_object_part_text_get(obj, "text");
   fail_if(!txt || strcmp(txt, buf));

   evas_free(evas);
}
EFL_END_TEST

void edje_test_text(TCase *tc)
{
   tcase_add_test(tc, edje_test_text_cursor);
   tcase_add_test(tc, edje_test_textblock);
}
