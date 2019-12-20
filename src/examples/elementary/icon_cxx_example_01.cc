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
#include <Efl_Ui.hh>

EAPI_MAIN int
elm_main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   using efl::eo::instantiate;

   efl::ui::Win win(instantiate);
   //win.title_set("Icon");
   win.autohide_set(true);

   efl::ui::Image icon(instantiate, win);
   icon.icon_set("home");

   efl::eina::string_view path = icon.file_get();
   efl::eina::string_view group = icon.key_get();
   std::cout << "path = " << path << ", group = "<< group;

   std::cout << ", name = " << icon.icon_get() << std::endl;

   icon.scale_method_set(EFL_GFX_IMAGE_SCALE_METHOD_NONE);
   //icon.resizable_set(false, true);
   //icon.smooth_set(false);
   //icon.fill_outside_set(true);

   win.content_set(icon);
   win.size_set({320, 320});

   elm_run();
   return 0;
}
ELM_MAIN()
