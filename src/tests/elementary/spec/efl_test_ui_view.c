#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.View",
       "test-widgets": ["Efl.Ui.Image", "Efl.Ui.Layout", "Efl.Ui.Button"]}
   spec-meta-end */

typedef struct _Efl_Ui_Image_Model_Change Efl_Ui_Image_Model_Change;
struct _Efl_Ui_Image_Model_Change
{
   Efl_Model *expected_current;

   Eina_Bool called;
   Eina_Bool previous;
   Eina_Bool current;
};

static void
_efl_ui_image_model_changed(void *data, const Efl_Event *event)
{
   Efl_Ui_Image_Model_Change *change = data;
   Efl_Model_Changed_Event *ev = event->info;

   ck_assert(!change->called);
   change->called = EINA_TRUE;
   change->previous = ev->previous ? EINA_TRUE : EINA_FALSE;
   ck_assert_ptr_eq(ev->current, change->expected_current);
   change->current = ev->current ? EINA_TRUE : EINA_FALSE;
}

EFL_START_TEST(model_change_event)
{
   Efl_Model *model = efl_add(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   Efl_Ui_Image_Model_Change test = { NULL, EINA_FALSE, EINA_FALSE, EINA_FALSE };

   efl_event_callback_add(widget, EFL_UI_VIEW_EVENT_MODEL_CHANGED,
                          _efl_ui_image_model_changed, &test);

   ck_assert_int_eq(test.called, EINA_FALSE);
   test.expected_current = model;
   efl_ui_view_model_set(widget, model);
   ck_assert_int_eq(test.called, EINA_TRUE);
   ck_assert_int_eq(test.previous, EINA_FALSE);
   ck_assert_int_eq(test.current, EINA_TRUE);

   test.called = EINA_FALSE;
   test.expected_current = NULL;
   efl_ui_view_model_set(widget, NULL);
   ck_assert_int_eq(test.called, EINA_TRUE);
   ck_assert_int_eq(test.previous, EINA_TRUE);
   ck_assert_int_eq(test.current, EINA_FALSE);
}
EFL_END_TEST

void
efl_ui_view_behavior_test(TCase *tc)
{
   tcase_add_test(tc, model_change_event);
}
