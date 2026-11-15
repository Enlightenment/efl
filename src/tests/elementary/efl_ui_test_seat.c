#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

/* A buffer canvas has exactly one seat, the default one, so what is checked
 * here is that the per-seat API answers coherently about it -- not that two
 * seats behave independently, which needs a display that has two.
 */

EFL_START_TEST(efl_ui_seat_test_canvas_has_a_default_seat)
{
   Eo *win;
   Efl_Input_Device *seat;
   Eina_Iterator *seats;
   Efl_Input_Device *dev;
   int found = 0;

   win = win_add(NULL, "seat", EFL_UI_WIN_TYPE_BASIC);

   seat = efl_canvas_scene_seat_default_get(win);
   ck_assert(seat != NULL);
   ck_assert_int_eq(efl_input_device_type_get(seat), EFL_INPUT_DEVICE_TYPE_SEAT);

   /* the window forwards the seats to its canvas */
   seats = efl_canvas_scene_seats(win);
   ck_assert(seats != NULL);
   EINA_ITERATOR_FOREACH(seats, dev)
     {
        if (dev == seat) found++;
     }
   eina_iterator_free(seats);
   ck_assert_int_eq(found, 1);

   /* and the default seat is reachable by its id */
   ck_assert(efl_canvas_scene_seat_get(win, efl_input_device_seat_id_get(seat)) == seat);
}
EFL_END_TEST

void efl_ui_test_seat(TCase *tc)
{
   tcase_add_test(tc, efl_ui_seat_test_canvas_has_a_default_seat);
}
