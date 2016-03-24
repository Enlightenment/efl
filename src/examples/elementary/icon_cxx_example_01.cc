#include <Elementary.hh>

EAPI_MAIN int
elm_main (int argc, char *argv[])
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   ::elm::win win(elm_win_util_standard_add("icon", "Icon"));
   win.autohide_set(true);

   ::elm::icon icon(efl::eo::parent = win);
   icon.order_lookup_set(ELM_ICON_LOOKUP_THEME_FDO);
   icon.standard_set("home");

   const char *path, *group;
   icon.file_get(&path, &group);
   std::cout << "path = " << path << ", group = "<< group;

   efl::eina::optional<std::string> name;
   name = icon.standard_get();
   std::cout << ", name = " << *name << std::endl;

   icon.no_scale_set(true);
   icon.resizable_set(false, true);
   icon.smooth_set(false);
   icon.fill_outside_set(true);

   icon.size_hint_weight_set(EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   win.resize_object_add(icon);
   icon.visible_set(true);

   win.size_set(320, 320);
   win.visible_set(true);

   elm_run();
   return 0;
}
ELM_MAIN()
