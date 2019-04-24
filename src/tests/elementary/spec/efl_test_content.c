#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Content",
       "test-widgets": ["Efl.Ui.Button", "Efl.Ui.Frame", "Efl.Ui.Grid_Default_Item",
                   "Efl.Ui.List_Default_Item", "Efl.Ui.List_Empty_Item",
                   "Efl.Ui.Navigation_Layout", "Efl.Ui.Panel", "Efl.Ui.Progressbar",
                   "Efl.Ui.Radio", "Efl.Ui.Popup", "Efl.Ui.Tab_Page", "Efl.Ui.Scroller"]}

   spec-meta-end */

EFL_START_TEST(content_set_get)
{
   Efl_Ui_Widget *w = create_test_widget();

   ck_assert_int_eq(efl_content_set(widget, w), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w), widget);
   ck_assert_ptr_eq(efl_content_get(widget), w);

   EXPECT_ERROR_START;
   ck_assert_int_eq(efl_content_set(widget, (void*)0x1), EINA_FALSE);
   EXPECT_ERROR_END;
   ck_assert_ptr_eq(efl_content_get(widget), NULL);
}
EFL_END_TEST

EFL_START_TEST(content_set_get2)
{
   Efl_Ui_Widget *w = create_test_widget();

   ck_assert_int_eq(efl_content_set(widget, w), EINA_TRUE);
   ck_assert_ptr_eq(efl_ui_widget_parent_get(w), widget);
   ck_assert_ptr_eq(efl_content_get(widget), w);

   ck_assert_int_eq(efl_content_set(widget, NULL), EINA_TRUE);
   ck_assert_ptr_eq(efl_content_get(widget), NULL);
}
EFL_END_TEST


EFL_START_TEST(content_unset)
{
   Efl_Ui_Widget *w = create_test_widget();

   ck_assert_int_eq(efl_content_set(widget, w), EINA_TRUE);
   efl_content_unset(widget);
   ck_assert_ptr_eq(efl_content_get(widget), NULL);
}
EFL_END_TEST

EFL_START_TEST(content_lifetime1)
{
   Efl_Ui_Widget *w1 = create_test_widget();

   efl_wref_add(w1, &w1);
   ck_assert_int_eq(efl_content_set(widget, w1), EINA_TRUE);
   efl_del(widget);
   ck_assert_ptr_eq(w1, NULL);
}
EFL_END_TEST

EFL_START_TEST(content_lifetime2)
{
   Efl_Ui_Widget *w1 = create_test_widget(), *w2 = create_test_widget();

   efl_wref_add(w2, &w2);
   efl_wref_add(w1, &w1);

   ck_assert_int_eq(efl_content_set(widget, w1), EINA_TRUE);
   ck_assert_int_eq(efl_content_set(widget, w2), EINA_TRUE);
   ck_assert_ptr_eq(w1, NULL);
   efl_del(widget);
   ck_assert_ptr_eq(w2, NULL);
}
EFL_END_TEST

EFL_START_TEST(content_lifetime3)
{
   Efl_Ui_Widget *w1 = create_test_widget();
   Eina_Bool content_changed_event = EINA_FALSE;

   ck_assert_int_eq(efl_content_set(widget, w1), EINA_TRUE);
   efl_test_container_expect_evt_content_added(widget, EFL_CONTENT_EVENT_CONTENT_CHANGED, &content_changed_event, NULL);
   efl_del(w1);
   ck_assert_int_eq(content_changed_event, EINA_TRUE);
   ck_assert_ptr_eq(efl_content_get(widget), NULL);
}
EFL_END_TEST

EFL_START_TEST(content_event)
{
   Efl_Ui_Widget *w1 = create_test_widget(), *w2 = create_test_widget();
   Eina_Bool content_changed_event;

   content_changed_event = EINA_FALSE;
   efl_test_container_expect_evt_content_added(widget, EFL_CONTENT_EVENT_CONTENT_CHANGED, &content_changed_event, w1);
   ck_assert_int_eq(efl_content_set(widget, w1), EINA_TRUE);
   ck_assert_int_eq(content_changed_event, EINA_TRUE);

   content_changed_event = EINA_FALSE;
   efl_test_container_expect_evt_content_added(widget, EFL_CONTENT_EVENT_CONTENT_CHANGED, &content_changed_event, w2);
   ck_assert_int_eq(efl_content_set(widget, w2), EINA_TRUE);
   ck_assert_int_eq(content_changed_event, EINA_TRUE);
}
EFL_END_TEST

void
efl_content_behavior_test(TCase *tc)
{
   tcase_add_test(tc, content_set_get);
   tcase_add_test(tc, content_set_get2);
   tcase_add_test(tc, content_lifetime1);
   tcase_add_test(tc, content_lifetime2);
   tcase_add_test(tc, content_lifetime3);
   tcase_add_test(tc, content_unset);
   tcase_add_test(tc, content_event);
}
