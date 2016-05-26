#include <Elementary.hh>

#include <sstream>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win;
   //win.title_set("Bg Image");
   win.autohide_set(true);

   //elm_app_compile_data_dir_set("/usr/share/elementary");
  //elm_app_info_set(reinterpret_cast<void*>(elm_main), "elementary", "images/plant_01.jpg");

   elm::Bg bg(win);
   //bg.load_size_set(320,320);
   //bg.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   bg.option_set(ELM_BG_OPTION_SCALE);
   //std::stringstream ss;
   //ss << elm_app_data_dir_get() << "/images/plant_01.jpg";
   bg.file_set("/home/luciana/Pictures/surprised_cat.jpeg", nullptr);
   //win.resize_object_add(bg);
   bg.eo_cxx::efl::Gfx::size_set(640,400);
   bg.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(640, 400);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
