#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

#include "ecore_wl2_suite.h"
#include "ecore_wl2_tests_helpers.h"

EFL_START_TEST(wl2_input_seat_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        struct wl_seat *seat;

        seat = ecore_wl2_input_seat_get(input);
        ck_assert(seat != NULL);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_input_seat_id_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        int id;

        id = ecore_wl2_input_seat_id_get(input);
        ck_assert_int_ne(id, 0);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_input_display_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        ck_assert(ecore_wl2_input_display_get(input) != NULL);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_input_keymap_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        if (ecore_wl2_input_seat_capabilities_get(input) ==
            ECORE_WL2_SEAT_CAPABILITIES_KEYBOARD)
          ck_assert(ecore_wl2_input_keymap_get(input) != NULL);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_input_name_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        printf("Input: <%s>\n", ecore_wl2_input_name_get(input));
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_input_seat_capabilities)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Input *input;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, input)
     {
        Ecore_Wl2_Seat_Capabilities cap = ECORE_WL2_SEAT_CAPABILITIES_NONE;

        cap = ecore_wl2_input_seat_capabilities_get(input);
        ck_assert(cap != ECORE_WL2_SEAT_CAPABILITIES_NONE);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

void
ecore_wl2_test_input(TCase *tc)
{
   if (getenv("WAYLAND_DISPLAY"))
     {
        tcase_add_test(tc, wl2_input_seat_get);
        tcase_add_test(tc, wl2_input_seat_id_get);
        tcase_add_test(tc, wl2_input_display_get);
        tcase_add_test(tc, wl2_input_keymap_get);
        tcase_add_test(tc, wl2_input_name_get);
        tcase_add_test(tc, wl2_input_seat_capabilities);
     }
}
