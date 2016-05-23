
#ifdef HAVE_CONFIG_H
# include "config.h"
# include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win::Standard win(elm_win_util_standard_add("spinner", "Spinner Example"));
   win.autohide_set(true);

   ::elm::Box bx(efl::eo::parent = win);
   bx.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bx);
   bx.visible_set(true);

   ::elm::Spinner sp(efl::eo::parent = win);
   sp.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp);
   sp.visible_set(true);

   ::elm::Spinner sp2(efl::eo::parent = win);
   sp2.label_format_set("Percentage %%%1.2f something");
   sp2.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp2.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp2);
   sp2.visible_set(true);

   ::elm::Spinner sp3(efl::eo::parent = win);
   sp3.label_format_set("%1.1f units");
   sp3.step_set(1.5);
   sp3.wrap_set(true);
   sp3.min_max_set(-50.0, 250.0);
   sp3.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp3.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp3);
   sp3.visible_set(true);

   ::elm::Spinner sp4(efl::eo::parent = win);
   sp4.style_set("vertical");
   sp4.interval_set(0.2);
   sp4.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp4.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp4);
   sp4.visible_set(true);

   ::elm::Spinner sp5(efl::eo::parent = win);
   sp5.editable_set(false);
   sp5.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp5.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp5);
   sp5.visible_set(true);

   ::elm::Spinner sp6(efl::eo::parent = win);
   sp6.editable_set(false);
   sp6.min_max_set(1, 12);
   sp6.special_value_add(1, "January");
   sp6.special_value_add(2, "February");
   sp6.special_value_add(3, "March");
   sp6.special_value_add(4, "April");
   sp6.special_value_add(5, "May");
   sp6.special_value_add(6, "June");
   sp6.special_value_add(7, "July");
   sp6.special_value_add(8, "August");
   sp6.special_value_add(9, "September");
   sp6.special_value_add(10, "October");
   sp6.special_value_add(11, "November");
   sp6.special_value_add(12, "December");
   sp6.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp6.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp6);
   sp6.visible_set(true);

   ::elm::Spinner sp7(efl::eo::parent = win);
   sp7.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   sp7.size_hint_align_set(EVAS_HINT_FILL, 0.5);
   bx.pack_end(sp7);
   sp7.visible_set(true);
   sp7.editable_set(true);

   auto changed = std::bind([&] () { std::cout << "Value changed to " << sp7.value_get() << std::endl; });

   sp7.callback_changed_add( changed );

   auto delay = std::bind([&] () {std::cout << "Value delay changed to " << sp7.value_get() << std::endl;});

   sp7.callback_delay_changed_add( delay );

   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

