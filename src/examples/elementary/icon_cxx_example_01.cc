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
   //win.title_set("Icon");
   win.autohide_set(true);

   efl::ui::Image icon(win);
   icon.icon_set("home");

   efl::eina::string_view path, group;
   icon.file_get(path, group);
   std::cout << "path = " << path << ", group = "<< group;

   std::cout << ", name = " << icon.icon_get() << std::endl;

   icon.scale_type_set(EFL_UI_IMAGE_SCALE_TYPE_NONE);
   //icon.resizable_set(false, true);
   //icon.smooth_set(false);
   //icon.fill_outside_set(true);

   // icon.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //win.resize_object_add(icon);
   icon.eo_cxx::efl::Gfx::size_set(320,320);
   icon.visible_set(true);

   win.eo_cxx::efl::Gfx::size_set(320, 320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
