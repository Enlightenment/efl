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
// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` button_cxx_example_00.cc -o button_cxx_example_00

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
   win.text_set("Button Example");
   win.autohide_set(true);

   efl::ui::Box box(instantiate, win);
   win.content_set(box);

   efl::ui::Button bt(instantiate, win);
   bt.text_set("Hello world!");
   box.pack(bt);

   auto wbt(bt._get_wref());
   auto cb(std::bind([wbt]() {
        std::cout << wbt->text_get() << std::endl;
     }));
   efl::eolian::event_add(efl::input::Clickable::clicked_event, bt, cb);

   efl::ui::Button bt2(instantiate, win);
   bt2.text_set("Click to quit");
   box.pack(bt2);

   auto wwin(win._get_wref());
   auto cb2(std::bind([wwin]() {
        ::efl_del(wwin->_eo_ptr()); // FIXME: No proper C++ API to delete win
     }));
   efl::eolian::event_add(efl::input::Clickable::clicked_event, bt2, cb2);

   win.size_set({320,160});
}
EFL_MAIN()
