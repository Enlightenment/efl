
#ifdef HAVE_CONFIG_H
# include "config.h"
# include "elementary_config.h"
#endif

#include <Elementary.hh>

#include <iostream>
#include <sstream>

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   elm_need_ethumb();

   elm_app_info_set(reinterpret_cast<void*>(elm_main), "elementary", "images/plant_01.jpg");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win::Standard win;
   win.title_set("Thumbnailer");
   win.autohide_set(true);

   ::elm::Thumb thumb(efl::eo::parent = win);

   auto generation_started = std::bind([] { std::cout << "thumbnail generation started." << std::endl; });

   auto generation_finished = std::bind([] { std::cout << "thumbnail generation finished." << std::endl; });

   auto generation_error = std::bind([] { std::cout << "thumbnail generation error." << std::endl; });

   thumb.callback_generate_start_add( generation_started );
   thumb.callback_generate_stop_add( generation_finished );
   thumb.callback_generate_error_add( generation_error );

   thumb.size_set(160, 160);
   std::stringstream ss;
   ss << elm_app_data_dir_get() << "/images/plant_01.jpg";
   thumb.file_set(ss.str(), "image");
   thumb.reload();

   thumb.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(thumb);
   thumb.visible_set(true);

   win.size_set(320, 320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()

