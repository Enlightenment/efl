#include <Elementary.hh>

#include <iostream>
#include <sstream>

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   elm_need_ethumb();

   elm_app_info_set(reinterpret_cast<void*>(elm_main), "elementary", "images/plant_01.jpg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::win::Standard win;
   //win.title_set("Thumbnailer");
   win.autohide_set(true);

   elm::Thumb thumb(win);

   auto generation_started = std::bind([] { std::cout << "thumbnail generation started." << std::endl; });

   auto generation_finished = std::bind([] { std::cout << "thumbnail generation finished." << std::endl; });

   auto generation_error = std::bind([] { std::cout << "thumbnail generation error." << std::endl; });

   efl::eolian::event_add(thumb.generate_start_event, thumb, generation_started);
   efl::eolian::event_add(thumb.generate_stop_event, thumb, generation_finished);
   efl::eolian::event_add(thumb.generate_error_event, thumb, generation_error);

   std::stringstream ss;
   ss << elm_app_data_dir_get() << "/images/plant_01.jpg";
   thumb.file_set(ss.str(), "image");
   thumb.eo_cxx::efl::Gfx::size_set(160, 160);
   //thumb.reload();

   // thumb.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(thumb);
   thumb.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(200, 200);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

