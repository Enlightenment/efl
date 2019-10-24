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

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Clock Example");
   win.autohide_set(true);

   efl::ui::Box bx(instantiate, win);
   win.content_set(bx);

   efl::ui::Clock ck(instantiate, win);
   bx.pack_end(ck);

   efl::ui::Clock ck2(instantiate, win);
   ck2.format_set("%I:%M %p");
   bx.pack_end(ck2);

   Efl_Time time;
   time.tm_hour = 12;
   time.tm_min = 42;
   time.tm_sec = 59;

   efl::ui::Clock ck3(instantiate, win);
   ck2.format_set("%H:%M:%S");
   ck3.time_set(time);
   bx.pack_end(ck3);

   efl::ui::Clock ck5(instantiate, win);
   int digedit = ELM_CLOCK_EDIT_HOUR_UNIT | ELM_CLOCK_EDIT_MIN_UNIT | ELM_CLOCK_EDIT_SEC_UNIT;
   ck5.edit_mode_set(static_cast<Elm_Clock_Edit_Mode>(digedit));
   bx.pack_end(ck5);

   win.size_set({500,500});
}
EFL_MAIN()
