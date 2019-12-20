/*
+ * Copyright 2019 by its authors. See AUTHORS.
+ *
+ * Licensed under the Apache License, Version 2.0 (the "License");
+ * you may not use this file except in compliance with the License.
+ * You may obtain a copy of the License at
+ *
+ *     http://www.apache.org/licenses/LICENSE-2.0
+ *
+ * Unless required by applicable law or agreed to in writing, software
+ * distributed under the License is distributed on an "AS IS" BASIS,
+ * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
+ * See the License for the specific language governing permissions and
+ * limitations under the License.
+ */
// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` spinner_cxx_example.cc -o spinner_cxx_example

#define EFL_CXXPERIMENTAL
#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#include <Efl_Ui.hh>
#include <iostream>

using namespace std::placeholders;
using efl::eo::instantiate;

static efl::ui::Win win;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl::ui::Win win(instantiate);
   win.text_set("Spinner Example");
   win.autohide_set(true);


   efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

   efl::ui::Spin_Button sp(instantiate, win);
   sp.hint_fill_set(true, false);
   sp.range_step_set(1.0);
   sp.range_limits_set(0.0, 10.0);
   sp.wraparound_set(true);
   bx.pack_end(sp);

   efl::ui::Spin_Button sp2(instantiate, win);
   sp2.format_string_set("Percentage %%%1.2f something", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
   sp2.hint_fill_set(true, false);
   bx.pack_end(sp2);

   efl::ui::Spin_Button sp3(instantiate, win);
   sp3.format_string_set("%1.1f units", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
   sp3.range_step_set(1.5);
   sp3.range_limits_set(-50.0, 250.0);
   sp3.hint_fill_set(true, false);
   bx.pack_end(sp3);

   efl::ui::Spin_Button sp4(instantiate, win);
   //FIXME setting the style is propetected...
   //sp4.style_set("vertical");
   sp4.range_step_set(0.2);
   sp4.hint_fill_set(true, false);
   bx.pack_end(sp4);

   efl::ui::Spin_Button sp5(instantiate, win);
   sp5.direct_text_input_set(false);
   sp5.hint_fill_set(true, false);
   bx.pack_end(sp5);

   Efl_Ui_Format_Value values[] = {
     {1, "January"},
     {2, "February"},
     {3, "March"},
     {4, "April"},
     {5, "May"},
     {6, "June"},
     {7, "July"},
     {8, "August"},
     {9, "September"},
     {10, "October"},
     {11, "November"},
     {12, "December"}
   };
   efl::eina::accessor<Efl_Ui_Format_Value> values_acc(EINA_C_ARRAY_ACCESSOR_NEW(values));
   efl::ui::Spin_Button sp6(instantiate, win);
   sp6.direct_text_input_set(false);
   sp6.range_limits_set(1, 12);
   //sp6.format_values_set(values_acc);
   sp6.hint_fill_set(true, false);
   bx.pack_end(sp6);

   efl::ui::Spin_Button sp7(instantiate, win);
   sp7.hint_fill_set(true, false);
   bx.pack_end(sp7);
   sp7.direct_text_input_set(true);

   auto changed = std::bind ( [] (efl::ui::Range_Display obj)
   { std::cout << "Changed to " << obj.range_value_get() << std::endl; }
         , std::placeholders::_1);
   efl::eo::downcast<efl::ui::Range_Display>(sp7).changed_event_cb_add(changed);

   auto steady =  std::bind ( [] (efl::ui::Range_Interactive obj)
   { std::cout << "Steady to " << obj.range_value_get() << std::endl; }
         , std::placeholders::_1);
   efl::eo::downcast<efl::ui::Range_Interactive>(sp7).steady_event_cb_add(steady);

}
EFL_MAIN()
