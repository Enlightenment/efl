#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

EFL_START_TEST(efl_ui_test_caching_factory_del_before_pause)
{
   Eo *win, *factory, *app = efl_app_main_get();
   unsigned int before;

   win = win_add(NULL, "factory", EFL_UI_WIN_TYPE_BASIC);
   before = efl_event_callback_count(app, EFL_APP_EVENT_PAUSE);

   /* A factory empties its cache when the application is paused, so it
      listens to the application -- which outlives it. Going away without
      letting go left the application calling into a freed factory the next
      time it paused, which closing the last window does. */
   factory = efl_add(EFL_UI_CACHING_FACTORY_CLASS, win,
                     efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   ck_assert_int_eq(efl_event_callback_count(app, EFL_APP_EVENT_PAUSE), before + 1);

   efl_del(factory);
   ck_assert_int_eq(efl_event_callback_count(app, EFL_APP_EVENT_PAUSE), before);
}
EFL_END_TEST

void efl_ui_test_caching_factory(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_caching_factory_del_before_pause);
}
