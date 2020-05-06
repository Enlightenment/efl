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
   win.text_set("Calendar Getters Example");
   win.autohide_set(true);

   efl::ui::Calendar cal(instantiate, win);
   win.content_set(cal);
   cal.size_set({135,135});

   // auto print_cal_info = std::bind([] (::elm::Calendar obj)
   //                       {
   //                          int year_min, year_max;
   //                          bool sel_enabled;
   //                          const char **wds;
   //                          struct tm sel_time;
   //                          double interval;

   //                          if (!obj.selected_time_get(&sel_time))
   //                            return;

   //                          interval = obj.interval_get();
   //                          obj.min_max_year_get(&year_min, &year_max);
   //                          sel_enabled = (obj.select_mode_get() != ELM_CALENDAR_SELECT_MODE_NONE);
   //                          wds = obj.weekdays_names_get();

   //                          std::cout << "Day: " << sel_time.tm_mday << ", Mon: " << sel_time.tm_mon << ", Year " <<
   //                          sel_time.tm_year + 1900 << ", WeekDay: " << sel_time.tm_wday << "<br>" << std::endl;

   //                          std::cout << "Interval: " << interval << ", Year_Min: " << year_min << ", Year_Max " <<
   //                          year_max << ", Sel Enabled: " << sel_enabled << "<br>" << std::endl;

   //                          std::cout << "Weekdays: " << wds[0] << ", " << wds[1] << ", " << wds[2] << ", " << wds[3] <<
   //                          ", " <<  wds[4] << ", " <<  wds[5] << ", " << wds[6] << "<br>" << std::endl << std::endl;
   //                       } , std::placeholders::_1 );

   // cal.callback_changed_add(print_cal_info);

   win.size_set({135,135});

   elm_run();
   return 0;
}
ELM_MAIN()
