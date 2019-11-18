/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` radio_cxx_example_01.cc -o radio_cxx_example_01

#define EFL_CXXPERIMENTAL
#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#include <Efl_Ui.hh>
#include <iostream>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Radio Example");
   win.autohide_set(true);

   efl::ui::Radio_Box bx(instantiate, win);
   bx.orientation_set(EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL);
   win.content_set(bx);

   efl::ui::Radio radio(instantiate, win);
   efl::ui::Radio group = radio;
   radio.text_set("Radio 1");
   radio.state_value_set(1);

   efl::ui::Image ic(instantiate, win);
   ic.icon_set("home");
   radio.content_set(ic);
   bx.pack_end(radio);

   auto cb_val = std::bind([bx] (efl::ui::Radio &obj EINA_UNUSED)
   { std::cout << "val is now: " << bx.selected_value_get() << std::endl; },
         std::placeholders::_1);
   efl::eolian::event_add(efl::ui::Radio_Group::value_changed_event, radio, cb_val);

   efl::ui::Radio radio2(instantiate, win);
   radio2.text_set("Radio 2");
   radio2.state_value_set(2);

   efl::ui::Image ic2(instantiate, win);
   ic2.icon_set("file");
   radio2.content_set(ic2);
   bx.pack_end(radio2);
   efl::eolian::event_add(efl::ui::Radio_Group::value_changed_event, radio2, cb_val);

   efl::ui::Radio radio3(instantiate, win);
   radio3.text_set("Radio 3");
   radio3.state_value_set(3);
   bx.pack_end(radio3);
   efl::eolian::event_add(efl::ui::Radio_Group::value_changed_event, radio3, cb_val);

   efl::ui::Radio radio4(instantiate, win);
   radio4.text_set("Radio 4");
   radio4.state_value_set(4);
   bx.pack_end(radio4);
   efl::eolian::event_add(efl::ui::Radio_Group::value_changed_event, radio4, cb_val);
}
EFL_MAIN()
