#include <Elementary.hh>

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

   efl::eina::string_view path, group;
   icon.file_get(path, group);
   std::cout << "path = " << path << ", group = "<< group;

   std::cout << ", name = " << icon.icon_get() << std::endl;

   icon.scale_type_set(EFL_GFX_IMAGE_SCALE_TYPE_NONE);
   //icon.resizable_set(false, true);
   //icon.smooth_set(false);
   //icon.fill_outside_set(true);

   win.content_set(icon);
   win.size_set({320, 320});

   elm_run();
   return 0;
}
ELM_MAIN()
