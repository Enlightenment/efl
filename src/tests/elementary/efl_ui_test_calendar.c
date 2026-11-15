#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_NOLEGACY_API_SUPPORT

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_calendar_item.eo.h"

// A day that resolves to no part of the calendar's theme is refused, and the
// item stays usable without one.
EFL_START_TEST(efl_ui_calendar_item_without_part)
{
   Eo *win, *calendar, *item;
   Eina_Rect geometry;

   win = win_add(NULL, "calendar", EFL_UI_WIN_TYPE_BASIC);
   calendar = efl_add(EFL_UI_CALENDAR_CLASS, win);
   item = efl_add(EFL_UI_CALENDAR_ITEM_CLASS, calendar);

   EXPECT_ERROR_START;
   efl_ui_calendar_item_day_number_set(item, 9999);
   EXPECT_ERROR_END;

   geometry = efl_ui_focus_object_focus_geometry_get(item);
   ck_assert_int_eq(geometry.w, 0);
   ck_assert_int_eq(geometry.h, 0);
   efl_ui_focus_object_focus_set(item, EINA_TRUE);
}
EFL_END_TEST

void efl_ui_test_calendar(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_test(tc, efl_ui_calendar_item_without_part);
}
