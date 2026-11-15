#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

EFL_START_TEST(efl_ui_flip_test_two_faces)
{
   Eo *win, *flip, *front, *back;

   win = win_add(NULL, "flip", EFL_UI_WIN_TYPE_BASIC);
   flip = efl_add(EFL_UI_FLIP_CLASS, win);
   ck_assert(flip != NULL);

   front = efl_add(EFL_UI_BUTTON_CLASS, flip);
   back = efl_add(EFL_UI_BUTTON_CLASS, flip);

   efl_pack_end(flip, front);
   efl_pack_end(flip, back);
   ck_assert_int_eq(efl_content_count(flip), 2);

   /* The front is what shows to begin with. Which face is up after a flip is
      not asserted here: going over is an animation, and a test that waits a
      fixed number of loop iterations for one is a test that fails on a slow
      machine. What is asserted is that go_to does nothing when it is already
      showing the side it was asked for, which is its documented difference
      from go and needs no animation to observe. */
   ck_assert(efl_ui_flip_front_visible_get(flip));
   efl_ui_flip_go_to(flip, EINA_TRUE, EFL_UI_FLIP_MODE_ROTATE_Y_CENTER_AXIS);
   get_me_to_those_events(win);
   ck_assert(efl_ui_flip_front_visible_get(flip));
}
EFL_END_TEST

/* unpack_all takes the contents out and leaves them alive; pack_clear takes
 * them out and deletes them.
 */
EFL_START_TEST(efl_ui_flip_test_unpack_all_keeps_contents)
{
   Eo *win, *flip, *front, *back;

   win = win_add(NULL, "flip", EFL_UI_WIN_TYPE_BASIC);
   flip = efl_add(EFL_UI_FLIP_CLASS, win);

   front = efl_add(EFL_UI_BUTTON_CLASS, flip);
   back = efl_add(EFL_UI_BUTTON_CLASS, flip);
   efl_pack_end(flip, front);
   efl_pack_end(flip, back);
   ck_assert_int_eq(efl_content_count(flip), 2);

   efl_ref(front);
   efl_ref(back);

   ck_assert(efl_pack_unpack_all(flip));
   ck_assert_int_eq(efl_content_count(flip), 0);
   ck_assert_ptr_eq(efl_content_get(efl_part(flip, "front")), NULL);
   ck_assert_ptr_eq(efl_content_get(efl_part(flip, "back")), NULL);

   /* still alive, which is the whole difference from pack_clear */
   ck_assert(efl_isa(front, EFL_UI_BUTTON_CLASS));
   ck_assert(efl_isa(back, EFL_UI_BUTTON_CLASS));

   efl_unref(front);
   efl_unref(back);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_flip_test_pack_clear_deletes_contents)
{
   Eo *win, *flip, *front;

   win = win_add(NULL, "flip", EFL_UI_WIN_TYPE_BASIC);
   flip = efl_add(EFL_UI_FLIP_CLASS, win);

   front = efl_add(EFL_UI_BUTTON_CLASS, flip);
   efl_pack_end(flip, front);
   ck_assert_int_eq(efl_content_count(flip), 1);

   /* An extra reference outlives the delete, so the object is still there to
      be asked whether it was invalidated -- which is how "deleted" is visible
      from here without reading freed memory. */
   efl_ref(front);
   ck_assert(!efl_invalidated_get(front));

   ck_assert(efl_pack_clear(flip));
   ck_assert_int_eq(efl_content_count(flip), 0);
   ck_assert_ptr_eq(efl_content_get(efl_part(flip, "front")), NULL);
   ck_assert(efl_invalidated_get(front));
   efl_unref(front);
}
EFL_END_TEST

void efl_ui_test_flip(TCase *tc)
{
   tcase_add_test(tc, efl_ui_flip_test_two_faces);
   tcase_add_test(tc, efl_ui_flip_test_unpack_all_keeps_contents);
   tcase_add_test(tc, efl_ui_flip_test_pack_clear_deletes_contents);
}
