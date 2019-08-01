#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

#include "ecore_wl2_suite.h"

static Ecore_Wl2_Display *
_display_connect(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_connect(NULL);
   return disp;
}

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

void
ecore_wl2_test_input(TCase *tc)
{
   if (getenv("WAYLAND_DISPLAY"))
     {
        tcase_add_test(tc, wl2_input_seat_get);
     }
}
