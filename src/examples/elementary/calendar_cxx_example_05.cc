#ifdef HAVE_CONFIG_H
#include "config.h"
#include "elementary_config.h"
#endif

#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Calendar Getters Example");
   win.autohide_set(true);

   elm::Calendar cal(win);
   //cal.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(cal);
   cal.eo_cxx::efl::Gfx::size_set(135,135);

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
   cal.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(135,135);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
