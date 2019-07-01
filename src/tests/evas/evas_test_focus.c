#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"

EFL_START_TEST(evas_focus)
{
   Evas *evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   evas_focus_in(evas);
   ck_assert_int_eq(evas_focus_state_get(evas), EINA_TRUE);
   evas_focus_out(evas);
   ck_assert_int_eq(evas_focus_state_get(evas), EINA_FALSE);
   evas_focus_out(evas);
   ck_assert_int_eq(evas_focus_state_get(evas), EINA_FALSE);
}
EFL_END_TEST

void evas_test_focus(TCase *tc)
{
   tcase_add_test(tc, evas_focus);
}
