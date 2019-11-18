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
#define EFL_CXXPERIMENTAL // for background part
#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#include <Efl_Ui.hh>

using efl::eo::instantiate;

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   efl::ui::Win win(instantiate);
   win.text_set("Window Background");
   win.autohide_set(true);
   win.size_set({320,320});
   //win.background().color_set(139, 69, 19, 255);

   // Clean exit
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   efl::eolian::event_add(efl::ui::Win::delete_request_event, win,
                          std::bind([&](){ win = nullptr; }));

   elm_run();
   return 0;
}
ELM_MAIN()
