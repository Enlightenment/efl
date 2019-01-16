// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` spinner_cxx_example.cc -o spinner_cxx_example

#define ELM_WIDGET_PROTECTED

#include <Elementary.hh>
#include <iostream>

#warning FIXME: This example requires proper EO API usage (not legacy spinner)

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
#if 0

   efl::ui::Win win(instantiate);
   win.text_set("Spinner Example");
   win.autohide_set(true);


   efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

   elm::Spinner sp(instantiate, win);
   sp.hint_fill_set(true, false);
   bx.pack_end(sp);

   elm::Spinner sp2(instantiate, win);
   sp2.label_format_set("Percentage %%%1.2f something");
   sp2.hint_fill_set(true, false);
   bx.pack_end(sp2);

   elm::Spinner sp3(instantiate, win);
   sp3.label_format_set("%1.1f units");
   sp3.step_set(1.5);
   sp3.wrap_set(true);
   sp3.min_max_set(-50.0, 250.0);
   sp3.hint_fill_set(true, false);
   bx.pack_end(sp3);

   elm::Spinner sp4(instantiate, win);
   sp4.style_set("vertical");
   sp4.interval_set(0.2);
   sp4.hint_fill_set(true, false);
   bx.pack_end(sp4);

   elm::Spinner sp5(instantiate, win);
   sp5.editable_set(false);
   sp5.hint_fill_set(true, false);
   bx.pack_end(sp5);

   elm::Spinner sp6(instantiate, win);
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
   sp6.hint_fill_set(true, false);
   bx.pack_end(sp6);

   elm::Spinner sp7(instantiate, win);
   sp7.hint_fill_set(true, false);
   bx.pack_end(sp7);
   sp7.editable_set(true);

   auto changed = std::bind([] (elm::Spinner &spinner)
   { std::cout << "Value changed to " << spinner.value_get() << std::endl; },
         std::placeholders::_1);
   efl::eolian::event_add(elm::Spinner::changed_event, sp7, changed);

   auto delay = std::bind([] (elm::Spinner &spinner)
   { std::cout << "Value changed to " << spinner.value_get() << "" << std::endl; },
         std::placeholders::_1);
   efl::eolian::event_add(elm::Spinner::delay_changed_event, sp7, delay);

#endif
}
EFL_MAIN()
