#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_LAYOUT_CALC_PROTECTED
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

EFL_START_TEST(check_all_times)
{
   Eo *win = win_add();
   Eo *timepicker = efl_add(EFL_UI_TIMEPICKER_CLASS, win);

   for (int min = 0; min < 60; ++min)
     {
        for (int hour = 0; hour < 24; ++hour)
          {
             efl_ui_timepicker_time_set(timepicker, hour, min);
          }
     }
}
EFL_END_TEST

EFL_START_TEST(check_all_times_24_mode)
{
   Eo *win = win_add();
   Eo *timepicker = efl_add(EFL_UI_TIMEPICKER_CLASS, win,
                            efl_ui_timepicker_is_24hour_set(efl_added, EINA_TRUE));

   for (int min = 0; min < 60; ++min)
     {
        for (int hour = 0; hour < 24; ++hour)
          {
             efl_ui_timepicker_time_set(timepicker, hour, min);
          }
     }
}
EFL_END_TEST

void efl_ui_test_timepicker(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_test(tc, check_all_times);
   tcase_add_test(tc, check_all_times_24_mode);
}
