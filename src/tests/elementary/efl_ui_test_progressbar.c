#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win, *pb;

static void
check_setup()
{
   win = win_add();

   pb = efl_add(EFL_UI_PROGRESSBAR_CLASS, win);
}

EFL_START_TEST(pb_text)
{
   efl_text_set(pb, "Test the Rest");
   ck_assert_str_eq(efl_text_get(pb), "Test the Rest");
}
EFL_END_TEST

EFL_START_TEST(pb_progress_label)
{
   efl_ui_progressbar_show_progress_label_set(pb, EINA_TRUE);
   ck_assert_int_eq(efl_ui_progressbar_show_progress_label_get(pb), EINA_TRUE);
   efl_ui_progressbar_show_progress_label_set(pb, EINA_FALSE);
   ck_assert_int_eq(efl_ui_progressbar_show_progress_label_get(pb), EINA_FALSE);
}
EFL_END_TEST

void efl_ui_test_progressbar(TCase *tc)
{
   tcase_add_checked_fixture(tc, check_setup, NULL);
   tcase_add_test(tc, pb_text);
   tcase_add_test(tc, pb_progress_label);
}
