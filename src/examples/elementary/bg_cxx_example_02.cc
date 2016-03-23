#include <Elementary.hh>

#include <sstream>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win_standard win;
   win.title_set("Bg Image");
   win.autohide_set(true);

   elm_app_compile_data_dir_set("/usr/share/elementary");
   elm_app_info_set(reinterpret_cast<void*>(elm_main), "elementary", "images/plant_01.jpg");

   ::elm::bg bg(efl::eo::parent = win);
   bg.load_size_set(20,20);
   bg.option_set(ELM_BG_OPTION_CENTER);
   std::stringstream ss;
   ss << elm_app_data_dir_get() << "/images/plant_01.jpg";
   bg.file_set(ss.str(), nullptr);
   bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(bg);
   bg.visible_set(true);

   win.size_set(320, 320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
